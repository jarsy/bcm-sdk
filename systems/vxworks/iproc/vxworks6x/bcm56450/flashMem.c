/* flashMem.c - flash memory device driver */

/*
 * Copyright (c) 2010 Wind River Systems, Inc.
 *
 * The right to copy, distribute, modify or otherwise make use
 * of this software may be licensed only pursuant to the terms
 * of an applicable Wind River license agreement.
 */
 
/*
modification history
--------------------
01a,17may10,my_ created from arm_pbxa9 rev 01a
*/

/*
DESCRIPTION
This library contains routines to manipulate flash memory. Read and write
routines are included.
*/

#define CONFIRM_SET_LOCK_BIT    0x00010001
#define PROGRAM_WORD            0x00100010
#define SETUP_BLOCK_ERASE       0x00200020
#define PROGRAM                 0x00400040
#define CLEAR_STATUS            0x00500050
#define SET_LOCK_BIT            0x00600060
#define CLEAR_LOCK_BIT          0x00600060
#define READ_STATUS             0x00700070
#define CONFIRM_ERASE           0x00d000d0
#define RESUME_ERASE            0x00d000d0
#define READ_ARRAY              0x00ff00ff

/* status register bits */

#define WSM_ERROR               0x003a003a
#define SR_ERASE_ERROR          0x00200020
#define SR_READY                0x00800080

/* disable debugging */

#undef CFI_DEBUG

#ifdef FLASH_DEBUG
#   define DEBUG_PRINT          printf
#else
#   undef  DEBUG_PRINT
#endif

/*******************************************************************************
*
* sysFlashBlkErase - erase routine for flash driver
*
* This routine erases the specified sector.
*
* RETURNS: OK or ERROR if erase fail.
*
* ERRNO: N/A
*/

LOCAL STATUS sysFlashBlkErase
    (
    int     eraseBlock
    )
    {    
    volatile UINT32* flashPtr;
    UINT32 writeTimeoutThreshold = 5000000;
    UINT32 writeTimeoutCount = 0;
    STATUS sts = OK;

    flashPtr = (UINT32 *)(ALT_FLASH_ADRS + FLASH_SEGMENT_SIZE * eraseBlock);

    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CLEAR_LOCK_BIT));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CONFIRM_ERASE));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(READ_ARRAY));
    
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(SETUP_BLOCK_ERASE));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CONFIRM_ERASE));

    while ((((ARMA9CTX_REGISTER_READ(flashPtr)) & SR_READY) != SR_READY) && 
            (++writeTimeoutCount < writeTimeoutThreshold));

    if(writeTimeoutCount == writeTimeoutThreshold)
        {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("Debug: timeout error in flash block erase.\n");
#endif
        ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(READ_ARRAY));
        sts = ERROR;
        }
    else
        {
        if ((ARMA9CTX_REGISTER_READ(flashPtr)) & WSM_ERROR)
            {
#ifdef DEBUG_PRINT
            DEBUG_PRINT("Debug: flash erase error on block %d.\n", eraseBlock);
#endif
            sts = ERROR;
            ARMA9CTX_REGISTER_WRITE(flashPtr, CLEAR_STATUS);
            } 
        }

    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)CLEAR_LOCK_BIT);
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)CONFIRM_SET_LOCK_BIT);
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)READ_ARRAY);

    return sts;
    }

/*******************************************************************************
*
* sysFlashWordProgram - write routine for flash
*
* This routine writes a word into flash.
*
* RETURNS: OK or ERROR if program fail.
*
* ERRNO: N/A
*/

LOCAL STATUS sysFlashWordProgram
    (
    volatile UINT32 * address,
    volatile UINT32   data
    )
    {
    STATUS    sts = OK;
    UINT32 *  flashPtr;
    UINT32 writeTimeoutThreshold = 1000000;
    UINT32 writeTimeoutCount = 0;
        
    flashPtr = (UINT32 *)address;

    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(PROGRAM_WORD));

    *(volatile UINT32 *)flashPtr =  data;

    writeTimeoutCount = 0;

    while ((((ARMA9CTX_REGISTER_READ(flashPtr)) & SR_READY) != SR_READY) && 
           (++writeTimeoutCount < writeTimeoutThreshold));

    if(writeTimeoutCount == writeTimeoutThreshold)
        {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("Debug: timeout error in flash write.\n");
#endif
        ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(READ_ARRAY));
        sts = ERROR;
        }
    else
        {
        if ((ARMA9CTX_REGISTER_READ(flashPtr)) & WSM_ERROR)
            {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("Debug: error in flash write.\n");
#endif                
            sts = ERROR;
            ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CLEAR_STATUS));
            }
        }

    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(READ_ARRAY));

    return sts;
    }

/*******************************************************************************
*
* sysFlashWrite - write to flash
*
* This routine writes a word into flash.
*
* RETURNS: OK or ERROR if program fail.
*
* ERRNO: N/A
*/

LOCAL STATUS sysFlashWrite
    (
    volatile UINT32 * address,
    volatile UINT32   data
    )
    {
    STATUS sts = OK;
    UINT32 * flashPtr;

    flashPtr = (UINT32 *)address;

    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CLEAR_LOCK_BIT));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CONFIRM_ERASE));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(READ_ARRAY));
    
    sts = sysFlashWordProgram((UINT32 *)(address), data);

    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CLEAR_LOCK_BIT));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(CONFIRM_SET_LOCK_BIT));
    ARMA9CTX_REGISTER_WRITE(flashPtr, (UINT32)(READ_ARRAY));

    if (sts != OK)
        {
#ifdef DEBUG_PRINT
        DEBUG_PRINT("[0x%x]word write status NOT ok\n", (UINT32)flashPtr);  
#endif
        }
        
    return sts;
    }

/*******************************************************************************
*
* sysFlashGet - get the contents of flash memory
*
* This routine copies the contents of flash memory into a specified
* string.
*
* RETURNS: OK, or ERROR if access is outside the flash memory range.
*
* SEE ALSO: sysFlashSet()
*
* INTERNAL
* If multiple tasks are calling sysFlashSet() and sysFlashGet(),
* they should use a semaphore to ensure mutually exclusive access.
*/

STATUS sysFlashGet
    (
    char *  string,     /* where to copy flash memory      */
    int     strLen,     /* maximum number of bytes to copy */
    int     offset      /* byte offset into flash memory   */
    )
    {
    if ((offset < 0) || (strLen < 0) || ((offset + strLen) > FLASH_MEM_SIZE))
        return (ERROR);
    
    bcopy ((char *) (ALT_FLASH_ADRS + offset), string, strLen);
   
    return (OK);
    }

/*******************************************************************************
*
* sysFlashSet - write to flash memory
*
* This routine copies a specified string into flash memory.
*
* If the specified string must be overlaid on the contents of flash memory,
* define FLASH_OVERLAY in config.h.
*
* RETURNS: OK, or ERROR if the write fails or the input parameters are
* out of range.
*
* SEE ALSO: sysFlashGet()
*
* INTERNAL
* If multiple tasks are calling sysFlashSet() and sysFlashGet(),
* they should use a semaphore to ensure mutually exclusive access to flash
* memory.
*/

STATUS sysFlashSet
    (
    char *  string,     /* string to be copied into flash memory */
    int     byteLen,    /* maximum number of bytes to copy       */
    int     offset      /* byte offset into flash memory         */
    )
    {

    UINT32 i, startSec;
    char * tempBuf;
#ifndef FLASH_OVERLAY    
    UINT32 flashStartAddr, tempBufStartAddr;
#endif /* FLASH_OVERLAY */

    /* limited to one sector */

    if ((offset < 0) || (byteLen < 0) || 
        (((offset % FLASH_SEGMENT_SIZE) + byteLen) > FLASH_SEGMENT_SIZE))
        return (ERROR);

    /* see if contents are actually changing */

    if (bcmp ((char *) (ALT_FLASH_ADRS + offset), string, byteLen) == 0)
        return (OK);

    if ((tempBuf = memalign(4, FLASH_SEGMENT_SIZE)) == NULL)
        return (ERROR);

    startSec = (offset) / FLASH_SEGMENT_SIZE;

#ifdef FLASH_OVERLAY

    /* first save the current data in this sector */

    bcopy((char *)(ALT_FLASH_ADRS + startSec * FLASH_SEGMENT_SIZE),
               tempBuf, FLASH_SEGMENT_SIZE);       
    bcopy(string, tempBuf + (offset % FLASH_SEGMENT_SIZE), byteLen);
#else
    bcopy(string, tempBuf, byteLen);
#endif  /* FLASH_OVERLAY */

    if(sysFlashBlkErase(startSec) == ERROR)
        return (ERROR);

#ifdef FLASH_OVERLAY                /* program device */
    for(i = 0; i < FLASH_SEGMENT_SIZE; i += 4)
        {
        if(*(UINT32 *)(tempBuf + i) == 0xffffffff)
            continue;
        
        if (sysFlashWrite((UINT32 *)(ALT_FLASH_ADRS +
                        startSec * FLASH_SEGMENT_SIZE + i),
                       *(UINT32 *)(tempBuf + i)) != OK)
            {
            free (tempBuf);
            return (ERROR);
            }
        }
#else   /* FLASH_OVERLAY */
    for(i = 0; i < ((byteLen + 3) / 4); i++)
        {
        flashStartAddr = ALT_FLASH_ADRS + startSec * FLASH_SEGMENT_SIZE + 
                         (offset % FLASH_SEGMENT_SIZE) + i * 4;
        tempBufStartAddr = tempBuf + i * 4;
        
        /* 
         * Note if FLASH_OVERLAY is not defined:
         * Here we assume that the specified start offset in a sector will 
         * always be aligned with 4 bytes. This routines should be updated if
         * we find that some cases conflict with this assumption.
         */
        
        if (sysFlashWrite(flashStartAddr, *(UINT32 *)tempBufStartAddr) != OK)
            {
            free (tempBuf);
            return (ERROR);
            }
        }
#endif  /* FLASH_OVERLAY */

        free (tempBuf);
        return (OK);
    }

