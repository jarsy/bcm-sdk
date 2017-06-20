/*
 * $Id: nlmcmarena.c,v 1.1.6.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */



#include <nlmcmarena.h>

/*align should be a power of 2. Given a value, the nearest highest power of 2 is returned*/
#define NLM_ALIGN_UP(value, align) (((value) + ((align) - 1)) & ~((align) - 1))

typedef struct NlmCmArena
{
    char *m_arenaStart_p;           /*Starting address of the arena*/
    char *m_arenaEnd_p;             /*Ending address of the arena*/
    nlm_u32 m_numBytesAvailable;    /*Number of free bytes available in the arena*/
    nlm_u32 m_numAllocedChunks;     /*Number of chunks allocated in the arena */
    nlm_u32 m_curOffset;            /*Offset of the first free byte in the arena */
    struct NlmCmArena *m_next_p;    /*Pointer to the next arena*/
    NlmCmAllocator *m_alloc_p;      /*Pointer to the Memory Allocator*/
} NlmCmArena;

/*Pointer to the first arena */
static NlmCmArena *g_arenaHead_p;

/*If arena is active then g_allowAllocFromArena is set to 1 and memory will be allocated from the arena
If arena is inactive then g_allowAllocFromArena is set to 0 and memory will NOT be allocated from the arena*/
static nlm_u32 g_allowAllocFromArena;

/*Total number of allocated chunks in all Arenas currently present */
static nlm_u32 g_numAllocedChunksFromAllArenas;

/*Number of completely free arena completely present */
static nlm_u32 g_numCompletelyFreeArena;

/*Size of the arena in bytes. This value should NOT be changed once FibTblMgr Configuration is Locked*/
static nlm_u32 g_arenaSizeInBytes = NLM_DEFAULT_ARENA_SIZE_IN_BYTES;


/*Internal function that allocates space for a new Arena and adds it to the linked list*/
NlmErrNum_t NlmCmArena__pvt_AddArena(NlmCmAllocator *alloc_p, NlmReasonCode *o_reason_p)
{
    NlmCmArena *arena_p = NULL;

    /*Allocate space for the linked list node from the system memory*/
    arena_p = alloc_p->m_vtbl.m_calloc(alloc_p, 1, sizeof(NlmCmArena));
    
    if(!arena_p)
    {
        if(o_reason_p)
            *o_reason_p = NLMRSC_LOW_MEMORY;
        return NLMERR_FAIL;
    }

    /*Allocate space for the arena from system memory*/
    arena_p->m_arenaStart_p = alloc_p->m_vtbl.m_calloc(alloc_p, 1, g_arenaSizeInBytes);

    if(!arena_p->m_arenaStart_p) 
    {
            alloc_p->m_vtbl.m_free(alloc_p, arena_p);
            if(o_reason_p)
                *o_reason_p = NLMRSC_LOW_MEMORY;
            return NLMERR_FAIL;
    }

    arena_p->m_arenaEnd_p = arena_p->m_arenaStart_p + g_arenaSizeInBytes - 1;
    arena_p->m_numBytesAvailable = g_arenaSizeInBytes;
    arena_p->m_alloc_p = alloc_p;

    if(!g_arenaHead_p)
    {
        g_arenaHead_p = arena_p;
    }
    else 
    {
        NlmCmArena *curArena_p = g_arenaHead_p;
        while(curArena_p->m_next_p)
            curArena_p = curArena_p->m_next_p;

        /*Place the new arena at the end of the list */
        curArena_p->m_next_p = arena_p;
    }

    g_numCompletelyFreeArena++;
    
    return NLMERR_OK;
}

NlmErrNum_t NlmCmArena__Init(NlmCmAllocator *alloc_p, nlm_u32 arenaSizeInBytes, NlmReasonCode *o_reason_p)
{
    NlmErrNum_t errNum = NLMERR_OK;

    if(arenaSizeInBytes > g_arenaSizeInBytes)
        g_arenaSizeInBytes = arenaSizeInBytes;

    errNum = NlmCmArena__pvt_AddArena(alloc_p, o_reason_p);

    return errNum;
}


NlmErrNum_t NlmCmArena__ActivateArena(NlmCmAllocator *alloc_p, NlmReasonCode *o_reason_p)
{
    NlmErrNum_t errNum = NLMERR_OK;

    if(g_numCompletelyFreeArena)
    {
        /*Allow memory allocations to happen from the Arena*/
        g_allowAllocFromArena = 1;
        return NLMERR_OK;
    }

    /*If there are no completely free Arenas, then try to allocate a completely new Arena*/
    errNum = NlmCmArena__pvt_AddArena(alloc_p, o_reason_p);

    /*If AddArena is successful, then we will have atleast one completely free Arena. In this case
    we can allow memory allocations to take place from the Arena.
    If AddArena fails then return the failure that AddArena reported*/
    if(g_numCompletelyFreeArena)
        g_allowAllocFromArena = 1;

    return errNum;
}

NlmErrNum_t NlmCmArena__DeactivateArena(void)
{
    /*Don't allow any memory allocations to take place from the Arena until Activate is called again*/
    g_allowAllocFromArena = 0;

    /*If we have 0 or 1 completely free Arenas then return*/
    if(g_numCompletelyFreeArena <= 1)
        return NLMERR_OK;
    
    {
        NlmCmArena *curArena_p = g_arenaHead_p;
        NlmCmArena *prevArena_p = NULL;
        NlmCmArena *temp;
        NlmCmAllocator *alloc_p;
        NlmBool isFirstFullyFreeArenaFound = NlmFalse;
    
        /*If there are 2 or more completely free Arenas, then free up the extra free Arenas to the system memory 
        so that we are left with only one completely free Arena*/
        g_arenaHead_p = NULL;
    
        while(curArena_p)
        {
            temp = curArena_p->m_next_p;
        
            if(curArena_p->m_numBytesAvailable == g_arenaSizeInBytes && isFirstFullyFreeArenaFound)
            {
                if(prevArena_p)
                    prevArena_p->m_next_p = curArena_p->m_next_p;
            
                alloc_p = curArena_p->m_alloc_p;

                alloc_p->m_vtbl.m_free(alloc_p, curArena_p->m_arenaStart_p);
                alloc_p->m_vtbl.m_free(alloc_p, curArena_p);

                --g_numCompletelyFreeArena;
                
            }
            else 
            {
                if(curArena_p->m_numBytesAvailable == g_arenaSizeInBytes)
                    isFirstFullyFreeArenaFound = 1;

                if(!g_arenaHead_p)
                    g_arenaHead_p = curArena_p;
                
                prevArena_p = curArena_p;   
            }
        
            curArena_p = temp;
        }
    
    }

    return NLMERR_OK;
}



void* NlmCmArena__Allocate(nlm_u32 size)
{
    /*If the Arena is not active or the memory size to be requested is 0, then return NULL*/
    if(!g_allowAllocFromArena || size == 0)
        return NULL;
    
    {
        nlm_u32 actual_size = NLM_ALIGN_UP(size, sizeof(char *));   /*Ensure that the address returned from Arena is word aligned*/
        NlmCmArena *curArena_p = g_arenaHead_p;
        void *result = NULL;

        /*Iterate through the Arenas and allocate the requested memory from the first Arena
        that can satisfy the request*/
        while(curArena_p)
        {
            if(curArena_p->m_numBytesAvailable >= actual_size)
            {
                result = &curArena_p->m_arenaStart_p[curArena_p->m_curOffset];
                
                if(curArena_p->m_numBytesAvailable == g_arenaSizeInBytes)
                    g_numCompletelyFreeArena--;
                curArena_p->m_numBytesAvailable -= actual_size;
                curArena_p->m_curOffset += actual_size;
                curArena_p->m_numAllocedChunks++;

                g_numAllocedChunksFromAllArenas++;

                break;
            }
            curArena_p = curArena_p->m_next_p;
        }

        return result;
    }

}


NlmBool NlmCmArena__FreeIfArenaMemory(void *ptr)
{
    /*If there are no chunks allocated from any of the Arenas or ptr is NULL, then return NlmFalse
    indicating that the memory is not Arena memory*/
    if(!g_numAllocedChunksFromAllArenas || !ptr)
        return NlmFalse;

    {
        NlmCmArena *curArena_p = g_arenaHead_p;
        NlmBool result = NlmFalse;

        while(curArena_p)
        {
            if(ptr >= (void*)curArena_p->m_arenaStart_p &&
                ptr <= (void*)curArena_p->m_arenaEnd_p)
            {
                /*The memory has been allocated from the Arena. So return it back to the arena*/
                result = NlmTrue;
                
                if(curArena_p->m_numAllocedChunks)
                    curArena_p->m_numAllocedChunks--;

                if(g_numAllocedChunksFromAllArenas)
                    g_numAllocedChunksFromAllArenas--;

                if(curArena_p->m_numAllocedChunks == 0)
                {
                    /*The Arena is completely free*/
                    curArena_p->m_curOffset = 0;
                    curArena_p->m_numBytesAvailable = g_arenaSizeInBytes;
                    g_numCompletelyFreeArena++;
                }
                break;
            }
            curArena_p = curArena_p->m_next_p;
        }

        return result;
    }
}


void NlmCmArena__Destroy(void)
{
    NlmCmArena *curArena_p = g_arenaHead_p;
    NlmCmArena *temp;
    NlmCmAllocator *alloc_p;

    /*Iterate through all the Arenas. Release the memory for the linked list node and the actual Arena*/
    while(curArena_p)
    {
        temp = curArena_p->m_next_p;
        alloc_p = curArena_p->m_alloc_p;

        alloc_p->m_vtbl.m_free(alloc_p, curArena_p->m_arenaStart_p);
        alloc_p->m_vtbl.m_free(alloc_p, curArena_p);

        curArena_p = temp;
    }
    g_arenaHead_p = NULL;

    g_allowAllocFromArena = 0;
    g_numAllocedChunksFromAllArenas = 0;
    g_numCompletelyFreeArena = 0;
}

