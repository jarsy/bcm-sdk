/*
 * $Id: qe2000_mvt.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Multicast Vector Table manipulation and management
 *
 */

#include <shared/bsl.h>

#include <sal/core/time.h>

#include <soc/debug.h>
#include <soc/error.h>
#include <soc/cm.h>

#ifdef BCM_QE2000_SUPPORT

#include <soc/sbx/qe2000_mvt.h>
#include <soc/sbx/qe2000_util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbx_util.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/module.h>
#include <shared/idxres_fl.h>
#include <shared/warmboot.h>


/*
 *  The first 4096 MVT entries are reserved for VLAN use by the microcode.
 *
 *  The next 4096 MVT entries are reserved for L2MC use by the L2MC driver
 *  (this is queued for change eventually to full dynamic, at which time this
 *  set will be placed in the available MVT entries, but this change is not
 *  currently at high priority).
 */
#define MVT_NUM_STATIC         (8 * 1024)
#define MVT_NUM_TOTAL          (48 * 1024)

typedef struct sbx_mvt_state_s {
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_scache_handle_t wbStateHandle;
    SHR_BITDCL *mvtUse;
#endif /* def BCM_WARM_BOOT_SUPPORT */
    shr_idxres_list_handle_t freeList;
} sbx_mvt_state_t;

/*
 *  There is only one MVT entry allocator and that is global to the local...
 *  ...uh...  ...node?  ...block?  ...cluster?
 *
 *  We only want (and need!) one of these allocators because mcGroups (thence
 *  MVT entry IDs) are global to the system, to allow all FEs to send to the
 *  same mcGroup ID for all members of that group rather than having to do
 *  something more blade-centric.
 *
 *  It's not locked because... well, the allocator functions have their own
 *  lock, we're not keeping local state outside of the allocator, and hopefully
 *  all of the PCI (or whatever transport) functions we use are atomic or do
 *  their own locking. Only the third assumption is in question, so if that's
 *  wrong, maybe we should have a unit-based lock (at least to keep I/O
 *  transactions separate).
 *
 *  If we ever move the MVT management away from the FE code and up to the
 *  application (where it probably belongs), this model will have to change...
 *  ...again!
 */
static sbx_mvt_state_t *g_mvtState = NULL;


/*
 * The following structure is used to maintain MVT information per unit.
 * The application API calling sequence ensures that this information is
 * consistent across all units in the system.
 *
 * The MVT space is managed as follows
 *  - 4K (0 - (4K-1)) McGroups reserved for VLANS. There is 1:1 association between a VLAN
 *    and McGroup number. The MVT entries are initialized when the VLAN module is initialized.
 *    MVT entry port membership is configured as ports are added/removed from VLAN. Alternatively
 *    the application could manage these MVTs via the APIs for managing the Global MVT space.
 *    However the two mechanisms should not be inter-mixed.
 *
 * - (4K - (spn_MC_GROUP_LOCAL_START_INDEX - 1)) McGroups are Global. This space is consistent
 *   across all QEs. multicast.h lists the APIs for managing this space.
 *
 * - (spn_MC_GROUP_LOCAL_START_INDEX  - (max device limit)) McGroups are Local. This space
 *   need not be consistent across all QEs. multicast.h lists the APIs for managing this space.
 *   Local Multicast indexes are hidden from the user application.
 *
 */
typedef struct sbx_mvt_state_info_s {
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_scache_handle_t wbStateHandle;
    SHR_BITDCL *vlanUse;
    SHR_BITDCL *globalUse;
    SHR_BITDCL *localUse;
    int vlanBase;
    int globalBase;
    int localBase;
#endif /* def BCM_WARM_BOOT_SUPPORT */
    uint32 mcgroup_local_start_index;
    shr_idxres_list_handle_t VlanFreeList;
    shr_idxres_list_handle_t GlobalFreeList;
    shr_idxres_list_handle_t LocalFreeList;
} sbx_mvt_state_info_t;

static sbx_mvt_state_info_t *mvtState_p[BCM_MAX_NUM_UNITS];

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 *  Need to handle the warm boot state header.
 *
 *  For MODE 0, the bitmap copy of the alloc state for all MVRs follows the
 *  header as a single contiguous bitmap.
 *
 *  For MODE 1, the bitmap copy of the alloc state is split into three
 *  segments, one each for VLANs, global space, and local space.  Each bitmap
 *  covers specifically that range of MVR space.
 *
 *  Note that we keep these updated in the scache cell as we go along, so there
 *  is no 'pack' function.  The init functions will unpack the scache cell as
 *  appropriate if they are invoked when in warm boot mode.  Since it is kept
 *  in sync with the hardware table, there is no sync (it can piggyback on
 *  other unitwise sync functions).
 *
 *  During operations, resources are claimed as gathered, and freed as
 *  released.  This means that we might leak resources in the case where there
 *  is a crash during an update that gathers or frees (or both, as several do)
 *  if there is a commit after the crash.  However, it seems unlikely that the
 *  scache would be committed in such a state, since this kind of state only
 *  occurs during operations rather than between them.  As long as commits are
 *  made between completed operations, this should never happen.
 */
typedef struct _sbx_qe2000_mvt_warm_boot_m0_hdr_s {
    uint8 mvtMode;
    uint8 hdrVersion;
    uint16 unused; /* align to 32b for performance */
} _sbx_qe2000_mvt_warm_boot_m0_hdr_t;
typedef struct _sbx_qe2000_mvt_warm_boot_m1_hdr_s {
    uint8 mvtMode;
    uint8 hdrVersion;
    uint16 unused; /* align to 32b for performance */
    uint16 vlanLow;
    uint16 vlanHigh;
    uint16 globalLow;
    uint16 globalHigh;
    uint16 localLow;
    uint16 localHigh;
} _sbx_qe2000_mvt_warm_boot_m1_hdr_t;
#endif /* def BCM_WARM_BOOT_SUPPORT */

#define SBX_MVT_MODE_VER0     0
#define SBX_MVT_MODE_VER1     1
static int sbx_mvt_mode = -1;

static soc_error_t
soc_qe2000_mvt_consistency_check(const int qeUnit)
{
    soc_error_t rv = SOC_E_NONE;


    if (qeUnit >= BCM_MAX_NUM_UNITS) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, Invalid unit(%d)\n"), FUNCTION_NAME(), qeUnit));
        return(SOC_E_INIT);
    }
    if (mvtState_p[qeUnit] == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, MVT (Ver1) Not Initialized, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
        return(SOC_E_INIT);
    }

    if (sbx_mvt_mode == SBX_MVT_MODE_VER0) {
        if (!g_mvtState) {
            /* something isn't initialised */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, MVT (Ver0) Not Initialized, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
            return(SOC_E_INIT);
        }
    }
    else if (sbx_mvt_mode == SBX_MVT_MODE_VER1)  {
        if (mvtState_p[qeUnit] == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, MVT (Ver1) Not Initialized, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
            return(SOC_E_INIT);
        }
    }
    else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, MVT (Ver Unknown), unit(%d)\n"), FUNCTION_NAME(), qeUnit));
        return(SOC_E_INIT);
    }

    return(rv);
}

/*
 * Function:
 *     soc_qe2000_mvt_entry_alloc
 * Description:
 *     Allocate 1 or more MVT entries in the QE.  When more than one entry
 *     is requested, the block is considered 'chained' and the 'next' pointers
 *     are set accordingly.  If num entries fail to allocate, the function will
 *     de-allocate the previous entries up to the failure.
 *     The application is responsible for managing freeing chains, and
 *     maintaining the chain when entries are removed from the middle.
 * Parameters:
 *     qeunit      - BCM device number
 *     num         - number of MVT entries to allocate.
 *     pEntryIds[] - (OUT) caller defined array of at least num elements.
 *                   Num allocated IDs will be stored in this array
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 */
soc_error_t
soc_qe2000_mvt_entry_alloc(const int qeunit,
                            const int num,
                            sbx_mvt_id_t pEntryIds[])
{
    int rv = SOC_E_NONE;
    int nCount;
    int aCount;
    sbx_qe2000_mvt_entry_t mvtEntry;
    shr_idxres_element_t idxResElt;

    /* check initialisation */
    if (!g_mvtState) {
        /* something isn't initialised */
        return SOC_E_INIT;
    }

    for (nCount = 0, aCount = 0;
         (nCount < num) && BCM_SUCCESS(rv);
         nCount++, aCount++) {
        rv = shr_idxres_list_alloc(g_mvtState->freeList, &idxResElt);
        pEntryIds[nCount] = (sbx_mvt_id_t)idxResElt;
#ifdef BCM_WARM_BOOT_SUPPORT
        SHR_BITSET(g_mvtState->mvtUse, idxResElt - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    }

    if (BCM_SUCCESS(rv)) {
        /* link them together if succesful and more than one allocated */
        for (nCount = 1; nCount < num; nCount++) {
            rv = soc_qe2000_mvt_entry_get(qeunit,
					  pEntryIds[nCount - 1],
					  &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
            mvtEntry.next = pEntryIds[nCount];

            LOG_INFO(BSL_LS_SOC_COMMON,
                     (BSL_META("Linking entry[%d]=%d to entry[%d]=%d\n"),
                      nCount-1, pEntryIds[nCount-1],
                      nCount, pEntryIds[nCount]));

            rv = soc_qe2000_mvt_entry_set(qeunit,
					  pEntryIds[nCount - 1],
					  &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
        }
    }

    if (BCM_FAILURE(rv)) {
        /* free the tried entries on failure */
        for (nCount = 0; nCount <= aCount; nCount++) {
            shr_idxres_list_free(g_mvtState->freeList, pEntryIds[nCount]);
#ifdef BCM_WARM_BOOT_SUPPORT
            SHR_BITCLR(g_mvtState->mvtUse, pEntryIds[nCount] - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */
            pEntryIds[nCount] = SBX_MVT_ID_NULL;
        }
    }

    return rv;
}


/*
 * Function:
 *     soc_qe2000_mvt_entry_reserve
 * Description:
 *     Reserve 1 or more MVT entries in the QE.  When more than one entry
 *     is requested, the block is considered 'chained' and the 'next' pointers
 *     are set accordingly.  If num entries fail to allocate, the function will
 *     de-allocate the previous entries up to the failure.
 *     The application is responsible for managing freeing chains, and
 *     maintaining the chain when entries are removed from the middle.
 * Parameters:
 *     qeunit      - BCM device number
 *     num         - number of MVT entries to allocate.
 *     pEntryIds[] - caller defined array of at least num elements to reserve
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 */
int
soc_qe2000_mvt_entry_reserve(const int qeunit,
                              const int num,
                              const sbx_mvt_id_t pEntryIds[])
{
    int rv = SOC_E_NONE;
    int nCount;
    int aCount;
    sbx_qe2000_mvt_entry_t mvtEntry;

    /* check initialisation */
    if (!g_mvtState) {
        /* something isn't initialised */
        return SOC_E_INIT;
    }

    for (nCount = 0, aCount = 0;
         (nCount < num) && BCM_SUCCESS(rv);
         nCount++, aCount++) {
        rv = shr_idxres_list_reserve(g_mvtState->freeList,
                                     pEntryIds[nCount],
                                     pEntryIds[nCount]);
#ifdef BCM_WARM_BOOT_SUPPORT
        SHR_BITSET(g_mvtState->mvtUse, pEntryIds[nCount] - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    }

    if (BCM_SUCCESS(rv)) {
        /* link them together if succesful and more than one allocated */
        for (nCount = 1; nCount < num; nCount++) {
            rv = soc_qe2000_mvt_entry_get(qeunit,
					  pEntryIds[nCount - 1],
					  &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
            mvtEntry.next = pEntryIds[nCount];

            LOG_INFO(BSL_LS_SOC_COMMON,
                     (BSL_META("Linking entry[%d]=%d to entry[%d]=%d\n"),
                      nCount-1, pEntryIds[nCount-1],
                      nCount, pEntryIds[nCount]));

            rv = soc_qe2000_mvt_entry_set(qeunit,
					  pEntryIds[nCount - 1],
					  &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
        }
    }

    if (BCM_FAILURE(rv)) {
        /* free the tried entries on failure */
        for (nCount = 0; nCount <= aCount; nCount++) {
            shr_idxres_list_free(g_mvtState->freeList, pEntryIds[nCount]);
#ifdef BCM_WARM_BOOT_SUPPORT
            SHR_BITCLR(g_mvtState->mvtUse, pEntryIds[nCount] - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */
        }
    }

    return rv;
}


/*
 * Function:
 *     soc_qe2000_mvt_entry_free
 * Description:
 *     Return a single MVT entry id back to the pool of free resources.
 * Parameters:
 *     qeunit    - BCM device number
 *     entryId   - Entry ID to free
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 */
soc_error_t
soc_qe2000_mvt_entry_free(const int qeunit,
                           const sbx_mvt_id_t entryId)
{
    int rv;

    /* check initialisation */
    if (!g_mvtState) {
        /* something isn't initialised */
        return SOC_E_INIT;
    }

    rv = shr_idxres_list_free(g_mvtState->freeList, entryId);
#ifdef BCM_WARM_BOOT_SUPPORT
    SHR_BITCLR(g_mvtState->mvtUse, entryId - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    return rv;
}


/*
 * Function:
 *     soc_qe2000_mvt_entry_check
 * Description:
 *     Verify whether an MVT entry is valid.  Works like get, but does not try
 *     to read the hardware.
 * Parameters:
 *     qeunit    - BCM device number
 *     entryId   - Entry ID to retrieve
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 */
soc_error_t
soc_qe2000_mvt_entry_check(const int qeUnit,
                           const sbx_mvt_id_t entryId)
{
    int rv = SOC_E_NONE;

    /* check initialisation */
    if (!g_mvtState) {
        /* something isn't initialised */
        return SOC_E_INIT;
    }

    /* caller must allocate the entry before using it */
    rv = shr_idxres_list_elem_state(g_mvtState->freeList, entryId);
    if (rv == BCM_E_EXISTS ) {
        rv = SOC_E_NONE;
    }
    return rv;
}


/*
 * Function:
 *     soc_qe2000_mvt_entry_get
 * Description:
 *     Get the MVT entry data structure at an MVT entry ID.  Returns QE
 *     based fabric ports in the MVT data structure
 * Parameters:
 *     qeunit    - BCM device number
 *     entryId   - Entry ID to retrieve
 *     pEntry    - (OUT) Caller allocated storage location for MVT entry.
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 */
soc_error_t
soc_qe2000_mvt_entry_get(const int qeUnit,
			 const sbx_mvt_id_t entryId,
			 sbx_qe2000_mvt_entry_t *pEntry)
{
    int rv = SOC_E_NONE;
    int sbxRv;
    int nPort;
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;

    /* check initialisation */
    if (!g_mvtState) {
        /* something isn't initialised */
        return SOC_E_INIT;
    }

    /* caller must allocate the entry before using it */
    rv = shr_idxres_list_elem_state(g_mvtState->freeList, entryId);
    if (rv != BCM_E_EXISTS ) {
        return rv;    /* could be some other idxres error... */
    }
    rv = SOC_E_NONE;

    sal_memset(&sbMvtEntry, 0, sizeof(HW_QE2000_MVT_ENTRY_ST));

    /* semaphore params not used */
    sbxRv = hwQe2000MVTGet((sbhandle)qeUnit, &sbMvtEntry, entryId,
                           NULL, NULL, 0, 0, NULL);
    if( sbxRv ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("MVTGet Failed:(%d)\n"), sbxRv ));
        return SOC_E_FAIL;
    }

    /* Convert from HW_QE2000_MVT_ENTRY_ST to sbx_mvt_id_t */
    pEntry->egress_data_a   = sbMvtEntry.ulMvtdA;
    pEntry->egress_data_b   = sbMvtEntry.ulMvtdB;
    pEntry->source_knockout = sbMvtEntry.bSourceKnockout;
    pEntry->next            = sbMvtEntry.ulNext;

    SOC_PBMP_CLEAR(pEntry->ports);
    for (nPort=0; nPort < SBX_MAX_PORTS; nPort++) {
        if (sbMvtEntry.bPortEnable[nPort]) {
            SOC_PBMP_PORT_ADD(pEntry->ports, nPort);
        }
    }

    return rv;
}


/*
 * Function:
 *     soc_qe2000_mvt_entry_set
 * Description:
 *     Set the MVT entry data structure at an MVT entry ID.  Sets the
 *     QE's view of the fabric ports in MVT data structure on HW.
 *     Implemented as a complete write in that it will completely overwrite
 *     all ports.  The caller must take care in maintaining existing port
 *     bitmaps set by peer FEs, or other clients.
 * Parameters:
 *     qeunit    - BCM device number
 *     entryId   - Entry ID to set
 *     pEntry    - MVT data structure to set, ports are QE based.
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 */
soc_error_t
soc_qe2000_mvt_entry_set(const int qeUnit,
                          const sbx_mvt_id_t entryId,
                          const sbx_qe2000_mvt_entry_t *pEntry)
{
    int rv = SOC_E_NONE;
    int sbxRv, nPort;
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;

    /* check initialisation */
    if (!g_mvtState) {
        /* something isn't initialised */
        return SOC_E_INIT;
    }

    /* caller must allocate the entry before using it */
    rv = shr_idxres_list_elem_state(g_mvtState->freeList, entryId);
    if( rv != BCM_E_EXISTS ) {
        return rv;  /* could be some other idxres error... */
    }
    rv = SOC_E_NONE;

    sal_memset(&sbMvtEntry, 0, sizeof(HW_QE2000_MVT_ENTRY_ST));

    /* Convert from sbx_mvt_id_t to HW_QE2000_MVT_ENTRY_ST */
    sbMvtEntry.ulMvtdA = pEntry->egress_data_a;
    sbMvtEntry.ulMvtdB = pEntry->egress_data_b;
    sbMvtEntry.bSourceKnockout = pEntry->source_knockout;
    sbMvtEntry.ulNext  = pEntry->next;

    for (nPort = 0; nPort < HW_QE2000_MAX_NUM_PORTS_K; nPort++) {
        sbMvtEntry.bPortEnable[nPort] =
            SOC_PBMP_MEMBER(pEntry->ports, nPort)?1:0;
    }

    /* Now set it, semaphore params not used */
    sbxRv = hwQe2000MVTSet((sbhandle)qeUnit, sbMvtEntry, entryId,
                           NULL, NULL, 0, 0, NULL);
    if( sbxRv ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("MVTSet Failed:(%d)\n"), sbxRv ));
        return SOC_E_FAIL;
    }

    return rv;
}

/*
 * Function:
 *     soc_qe2000_mvt_init
 * Description:
 *     Initialize the MVT table to NULL (0 for all entries, all ones for next
 *     pointer), and internal data structures.
 * Parameters:
 *     qeunit    - BCM device number
 * Returns:
 *     SOC_E_NONE    - Success
 *     SOC_E_XXX     - Failure, other errors
 * Notes:
 *     There is no reinit.  This must be called at least once on all QE units,
 *     but calling it multiple times does not reset the allocator list; it only
 *     clears the MVT entries on that unit.
 */
soc_error_t
soc_qe2000_mvt_init(const int qeUnit)
{
    int rv = SOC_E_NONE, sbRv;
    int i;
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;
    sal_usecs_t startTime;
    int nNumInitMtv;
    sbx_mvt_state_t *tempState = NULL;
    shr_idxres_list_handle_t oldList;
    int xrv;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 wbSize = 0;
    uint32 wbOldSize = 0;
    uint8 *wbPtr = NULL;
    _sbx_qe2000_mvt_warm_boot_m0_hdr_t *hdrPtr = NULL;
    int j, k;
#endif /* def BCM_WARM_BOOT_SUPPORT */

    startTime = sal_time_usecs();

    if (sbx_mvt_mode == SBX_MVT_MODE_VER1) {
        return(SOC_E_INTERNAL);
    }

    /* see if we're just starting out */
    if (!g_mvtState) {
        /* starting; allocate global structures */
        tempState = sal_alloc(sizeof(sbx_mvt_state_t),"global MVT state");
        if (tempState) {
            /* managed to allocate the needed space */
            sal_memset(tempState, 0x00, sizeof(sbx_mvt_state_t));
        } else {
            /* failed to allocate the needed space */
            return SOC_E_MEMORY;
        }
#ifdef BCM_WARM_BOOT_SUPPORT
        /* since the handle is fixed, always just fill in the computed value */
        SOC_SCACHE_HANDLE_SET(tempState->wbStateHandle,
                              0 /* global, so unit zero */,
                              BCM_MODULE_MULTICAST,
                              0 /* sequence */);
#endif /* def BCM_WARM_BOOT_SUPPORT */
        rv = shr_idxres_list_create(&(tempState->freeList),
                                    MVT_NUM_STATIC,
                                    MVT_NUM_TOTAL-1,
                                    0,
                                    MVT_NUM_TOTAL-1,
                                    "mvtFreeList");
        if(BCM_FAILURE(rv)) {
            /* get rid of the working structure */
            sal_free(tempState);
            return rv;
        }
        /* if we're here, it all went well */
        g_mvtState = tempState;
    } else {
        /* restarting; mark all resources as free */
        
        oldList = g_mvtState->freeList;
        rv = shr_idxres_list_create(&(g_mvtState->freeList),
                                    MVT_NUM_STATIC,
                                    MVT_NUM_TOTAL-1,
                                    0,
                                    MVT_NUM_TOTAL-1,
                                    "mvtFreeList");
        if (!BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META("Discard unit %d prior state %08X; reinit at %08X\n"),
                         qeUnit,
                         (uint32)oldList,
                         (uint32)g_mvtState->freeList));
            if (oldList) {
                xrv = shr_idxres_list_destroy(oldList);
                COMPILER_REFERENCE(xrv);
            }
        } else {
            g_mvtState->freeList = oldList;
        }
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += _SHR_BITDCLSIZE(MVT_NUM_TOTAL - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += 1; /* allow for rounding error to shift */
    wbSize *= sizeof(SHR_BITDCL); /* scale BITDCL count to bytes */
    wbSize += sizeof(_sbx_qe2000_mvt_warm_boot_m0_hdr_t); /* include header */
    if (!SOC_WARM_BOOT(qeUnit)) {
#endif /* def BCM_WARM_BOOT_SUPPORT */
        /* Init the entire MVT table for this unit */
        sal_memset(&sbMvtEntry, 0, sizeof(HW_QE2000_MVT_ENTRY_ST) );
        sbMvtEntry.ulNext = SBX_MVT_ID_NULL;

        nNumInitMtv = MVT_NUM_TOTAL;

        for(i=0; i<nNumInitMtv; i++)
        {
            sbRv = hwQe2000MVTSet((sbhandle)qeUnit, sbMvtEntry, i,
                                  NULL, NULL, 0, 0, NULL /* semaphore params not used */);
            if( sbRv ) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("Error on entry=%d  %d\n"), i, sbRv));
            }
        }
        /* diagnostic */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("Initialized %d MVT entries, in %d uSecs\n"),
                     nNumInitMtv, sal_time_usecs() - startTime));
#ifdef BCM_WARM_BOOT_SUPPORT
        /* set up warm boot recovery space */
        rv = soc_scache_ptr_get(qeUnit,
                                g_mvtState->wbStateHandle,
                                &wbPtr,
                                &wbOldSize);
        if ((SOC_E_NOT_FOUND != rv) && (SOC_E_NONE != rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error(%s) checking scache handle on"
                                " unit:%d\n"),
                       FUNCTION_NAME(),
                       soc_errmsg(rv),
                       qeUnit));
        }
        if (SOC_E_NOT_FOUND == rv) {
            rv = soc_scache_alloc(qeUnit, g_mvtState->wbStateHandle, wbSize);
            if (SOC_E_NONE != rv) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Error(%s) allocating WB scache handle"
                                    " on unit:%d \n"),
                           FUNCTION_NAME(),
                           soc_errmsg(rv),
                           qeUnit));
            }
            if (SOC_E_NONE == rv) {
                rv = soc_scache_ptr_get(qeUnit,
                                        g_mvtState->wbStateHandle,
                                        &wbPtr,
                                        &wbOldSize);
                if (SOC_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: Error(%s) checking scache handle"
                                        " on unit:%d\n"),
                               FUNCTION_NAME(),
                               soc_errmsg(rv),
                               qeUnit));
                }
            }
        } /* if (SOC_E_NOT_FOUND == rc) */
        if ((SOC_E_NONE == rv) && (wbOldSize < wbSize)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d scache block is too small (%d;"
                                " needed %d)\n"),
                       FUNCTION_NAME(),
                       qeUnit,
                       wbOldSize,
                       wbSize));
            rv = BCM_E_RESOURCE;
        }
        if (SOC_E_NONE == rv) {
            sal_memset((void*)(wbPtr +
                               sizeof(_sbx_qe2000_mvt_warm_boot_m0_hdr_t)),
                       0x00,
                       wbSize - sizeof(_sbx_qe2000_mvt_warm_boot_m0_hdr_t));
            hdrPtr = (_sbx_qe2000_mvt_warm_boot_m0_hdr_t*)wbPtr;
            hdrPtr->mvtMode = SBX_MVT_MODE_VER0;
            hdrPtr->hdrVersion = 0;
            hdrPtr->unused = 0;
            g_mvtState->mvtUse = (SHR_BITDCL*)(&((hdrPtr[1])));
        }
#endif /* def BCM_WARM_BOOT_SUPPORT */

        sbx_mvt_mode = SBX_MVT_MODE_VER0;
#ifdef BCM_WARM_BOOT_SUPPORT
    } else { /* if (!SOC_WARM_BOOT(unit)) */
        rv = soc_scache_ptr_get(qeUnit,
                                g_mvtState->wbStateHandle,
                                &wbPtr,
                                &wbOldSize);
        if (SOC_E_NONE != rv) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error(%s) getting scache handle on"
                                " unit:%d\n"),
                       FUNCTION_NAME(),
                       soc_errmsg(rv),
                       qeUnit));
        }
        if ((SOC_E_NONE == rv) && (wbOldSize < wbSize)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d scache block is too small (%d;"
                                " needed %d)\n"),
                       FUNCTION_NAME(),
                       qeUnit,
                       wbOldSize,
                       wbSize));
            rv = BCM_E_RESOURCE;
        }
        if (SOC_E_NONE == rv) {
            hdrPtr = (_sbx_qe2000_mvt_warm_boot_m0_hdr_t*)wbPtr;
            /* sanity check */
            if (0 != hdrPtr->hdrVersion) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Unknown state version %d\n"),
                           FUNCTION_NAME(),
                           hdrPtr->hdrVersion));
                rv = BCM_E_CONFIG;
            }
            if (SBX_MVT_MODE_VER0 != hdrPtr->mvtMode) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: unit %d configuration has changed\n"),
                           FUNCTION_NAME(),
                           qeUnit));
                rv = BCM_E_CONFIG;
            }
        }
        if (SOC_E_NONE == rv) {
            g_mvtState->mvtUse = (SHR_BITDCL*)(&((hdrPtr[1])));
            /* set up global use */
            k = MVT_NUM_TOTAL - MVT_NUM_STATIC;
            for (i = 0; i < k; i++) {
                if (SHR_BITGET(g_mvtState->mvtUse, i)) {
                    /* found one in use; collect as many contiguous as we can */
                    j = i + 1;
                    while ((j < k) && SHR_BITGET(g_mvtState->mvtUse, j)) {
                        j++;
                    }
                    j--;
                    /* reserve elements [i..j] */
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META("Reserve unit %d MVT entries %08X..%08X\n"),
                                 qeUnit,
                                 i + MVT_NUM_STATIC,
                                 j + MVT_NUM_STATIC));
                    rv = shr_idxres_list_reserve(g_mvtState->freeList,
                                                 i + MVT_NUM_STATIC,
                                                 j + MVT_NUM_STATIC);
                    if (BCM_E_NONE != rv) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("%s: unable to reserve unit %d MVT"
                                            " elements %d..%d: %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   qeUnit,
                                   i + MVT_NUM_STATIC,
                                   j + MVT_NUM_STATIC,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                        break;
                    }
                    /* resume search at next element */
                    i = j + 1;
                }
            }
        }
    } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* def BCM_WARM_BOOT_SUPPORT */
    return rv;
}

soc_error_t
soc_qe2000_bd_mvt_init(const int qeUnit)
{
    int rv = SOC_E_NONE;
    int xrv;
    sbx_mvt_state_t *tempState;
    shr_idxres_list_handle_t oldList;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 wbSize = 0;
    uint32 wbOldSize = 0;
    uint8 *wbPtr = NULL;
    _sbx_qe2000_mvt_warm_boot_m0_hdr_t *hdrPtr;
    int i, j, k;
#endif /* def BCM_WARM_BOOT_SUPPORT */

    if (SBX_MVT_ID_BD_SIZE == 0) {
#if 0
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, no initialization required\n"), FUNCTION_NAME()));
#endif
        return(rv);
    }

    if (!g_mvtState) {
        /* starting; allocate global structures */
        tempState = sal_alloc(sizeof(sbx_mvt_state_t),"global bd MVT state");
        if (tempState) {
            /* managed to allocate the needed space */
            sal_memset(tempState, 0x00, sizeof(sbx_mvt_state_t));
        } else {
            /* failed to allocate the needed space */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, memory allocation failed\n"), FUNCTION_NAME()));
            return(SOC_E_MEMORY);
        }
#ifdef BCM_WARM_BOOT_SUPPORT
        /* since the handle is fixed, always just fill in the computed value */
        SOC_SCACHE_HANDLE_SET(tempState->wbStateHandle,
                              0 /* global, so unit zero */,
                              BCM_MODULE_MULTICAST,
                              0 /* sequence */);
#endif /* def BCM_WARM_BOOT_SUPPORT */

        /* allocate Back Door McGroup list */
        rv = shr_idxres_list_create(&(tempState->freeList),
                                    (SBX_MVT_ID_DYNAMIC_END - SBX_MVT_ID_BD_SIZE + 1),
                                    SBX_MVT_ID_DYNAMIC_END,
                                    (SBX_MVT_ID_DYNAMIC_END - SBX_MVT_ID_BD_SIZE + 1),
                                    SBX_MVT_ID_DYNAMIC_END,
                                    "mvtBackDoorFreeLIst");
        if(BCM_FAILURE(rv)) {
            /* get rid of the working structure */
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, shr_idxres_list_create, failed, Err: 0x%x\n"), FUNCTION_NAME(), rv));
            sal_free(tempState);
            return(rv);
        }
        /* if we're here, it all went well */
        g_mvtState = tempState;
    } else {
        /* restarting; mark all resources as free */
        
        oldList = g_mvtState->freeList;
        rv = shr_idxres_list_create(&(g_mvtState->freeList),
                                    (SBX_MVT_ID_DYNAMIC_END - SBX_MVT_ID_BD_SIZE + 1),
                                    SBX_MVT_ID_DYNAMIC_END,
                                    (SBX_MVT_ID_DYNAMIC_END - SBX_MVT_ID_BD_SIZE + 1),
                                    SBX_MVT_ID_DYNAMIC_END,
                                    "mvtBackDoorFreeLIst");
        if (!BCM_FAILURE(rv)) {
            LOG_VERBOSE(BSL_LS_SOC_COMMON,
                        (BSL_META("Discard unit %d prior BD state %08X; reinit at %08X\n"),
                         qeUnit,
                         (uint32)oldList,
                         (uint32)g_mvtState->freeList));
            if (oldList) {
                xrv = shr_idxres_list_destroy(oldList);
                COMPILER_REFERENCE(xrv);
            }
        } else {
            g_mvtState->freeList = oldList;
        }
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += _SHR_BITDCLSIZE(MVT_NUM_TOTAL - MVT_NUM_STATIC);
#endif /* def BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += 1; /* allow for rounding error to shift */
    wbSize *= sizeof(SHR_BITDCL); /* scale BITDCL count to bytes */
    wbSize += sizeof(_sbx_qe2000_mvt_warm_boot_m0_hdr_t); /* include header */
    if (!SOC_WARM_BOOT(qeUnit)) {
        /* set up warm boot recovery space */
        rv = soc_scache_ptr_get(qeUnit,
                                g_mvtState->wbStateHandle,
                                &wbPtr,
                                &wbOldSize);
        if ((SOC_E_NOT_FOUND != rv) && (SOC_E_NONE != rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error(%s) checking scache handle on"
                                " unit:%d\n"),
                       FUNCTION_NAME(),
                       soc_errmsg(rv),
                       qeUnit));
        }
        if (SOC_E_NOT_FOUND == rv) {
            rv = soc_scache_alloc(qeUnit, g_mvtState->wbStateHandle, wbSize);
            if (SOC_E_NONE != rv) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Error(%s) allocating WB scache handle"
                                    " on unit:%d \n"),
                           FUNCTION_NAME(),
                           soc_errmsg(rv),
                           qeUnit));
            }
            if (SOC_E_NONE == rv) {
                rv = soc_scache_ptr_get(qeUnit,
                                        g_mvtState->wbStateHandle,
                                        &wbPtr,
                                        &wbOldSize);
                if (SOC_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: Error(%s) checking scache handle"
                                        " on unit:%d\n"),
                               FUNCTION_NAME(),
                               soc_errmsg(rv),
                               qeUnit));
                }
            }
        } /* if (SOC_E_NOT_FOUND == rc) */
        if ((SOC_E_NONE == rv) && (wbOldSize < wbSize)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d scache block is too small (%d;"
                                " needed %d)\n"),
                       FUNCTION_NAME(),
                       qeUnit,
                       wbOldSize,
                       wbSize));
            rv = BCM_E_RESOURCE;
        }
        if (SOC_E_NONE == rv) {
            sal_memset((void*)(wbPtr +
                               sizeof(_sbx_qe2000_mvt_warm_boot_m0_hdr_t)),
                       0x00,
                       wbSize - sizeof(_sbx_qe2000_mvt_warm_boot_m0_hdr_t));
            hdrPtr = (_sbx_qe2000_mvt_warm_boot_m0_hdr_t*)wbPtr;
            hdrPtr->mvtMode = SBX_MVT_MODE_VER0;
            hdrPtr->hdrVersion = 0;
            hdrPtr->unused = 0;
            g_mvtState->mvtUse = (SHR_BITDCL*)(&((hdrPtr[1])));
        }
    } else { /* if (!SOC_WARM_BOOT(unit)) */
        rv = soc_scache_ptr_get(qeUnit,
                                tempState->wbStateHandle,
                                &wbPtr,
                                &wbOldSize);
        if (SOC_E_NONE != rv) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error(%s) getting scache handle on"
                                " unit:%d\n"),
                       FUNCTION_NAME(),
                       soc_errmsg(rv),
                       qeUnit));
        }
        if ((SOC_E_NONE == rv) && (wbOldSize < wbSize)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d scache block is too small (%d;"
                                " needed %d)\n"),
                       FUNCTION_NAME(),
                       qeUnit,
                       wbOldSize,
                       wbSize));
            rv = BCM_E_RESOURCE;
        }
        if (SOC_E_NONE == rv) {
            hdrPtr = (_sbx_qe2000_mvt_warm_boot_m0_hdr_t*)wbPtr;
            /* sanity check */
            if (0 != hdrPtr->hdrVersion) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Unknown state version %d\n"),
                           FUNCTION_NAME(),
                           hdrPtr->hdrVersion));
                rv = BCM_E_CONFIG;
            }
            if (SBX_MVT_MODE_VER0 != hdrPtr->mvtMode) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: unit %d configuration has changed\n"),
                           FUNCTION_NAME(),
                           qeUnit));
                rv = BCM_E_CONFIG;
            }
        }
        if (SOC_E_NONE == rv) {
            tempState->mvtUse = (SHR_BITDCL*)(&((hdrPtr[1])));
            /* set up global use */
            k = MVT_NUM_TOTAL - MVT_NUM_STATIC;
            for (i = 0; i < k; i++) {
                if (SHR_BITGET(tempState->mvtUse, i)) {
                    /* found one in use; collect as many contiguous as we can */
                    j = i + 1;
                    while ((j < k) && SHR_BITGET(tempState->mvtUse, j)) {
                        j++;
                    }
                    j--;
                    /* reserve elements [i..j] */
                    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                                (BSL_META("Reserve unit %d MVT entries %08X..%08X\n"),
                                 qeUnit,
                                 i + MVT_NUM_STATIC,
                                 j + MVT_NUM_STATIC));
                    rv = shr_idxres_list_reserve(tempState->freeList,
                                                 i + MVT_NUM_STATIC,
                                                 j + MVT_NUM_STATIC);
                    if (BCM_E_NONE != rv) {
                        LOG_ERROR(BSL_LS_SOC_COMMON,
                                  (BSL_META("%s: unable to reserve unit %d MVT"
                                            " elements %d..%d: %d (%s)\n"),
                                   FUNCTION_NAME(),
                                   qeUnit,
                                   i + MVT_NUM_STATIC,
                                   j + MVT_NUM_STATIC,
                                   rv,
                                   _SHR_ERRMSG(rv)));
                        break;
                    }
                    /* resume search at next element */
                    i = j + 1;
                }
            }
        }
    } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* def BCM_WARM_BOOT_SUPPORT */

    return(rv);
}

int
_sbx_qe2000_mvt_entry_reserve(const int qeunit,
                              const int num,
                              const sbx_mvt_id_t pEntryIds[])
{
    return soc_qe2000_mvt_entry_reserve(qeunit, num, pEntryIds);
}

int
_sbx_qe2000_mvt_entry_alloc(const int qeunit,
			    const int num,
			    sbx_mvt_id_t entryIds[])
{
    return soc_qe2000_mvt_entry_alloc(qeunit, num, entryIds);
}

int
_sbx_qe2000_mvt_entry_free(const int qeunit,
			   const sbx_mvt_id_t entryId)
{
    return soc_qe2000_mvt_entry_free(qeunit, entryId);
}

int
_sbx_qe2000_mvt_entry_get(const int qeunit,
			  const sbx_mvt_id_t entryId,
			  sbx_qe2000_mvt_entry_t *entry)
{
    return soc_qe2000_mvt_entry_get(qeunit, entryId, entry);
}

int
_sbx_qe2000_mvt_entry_set(const int qeunit,
			  const sbx_mvt_id_t entryId,
			  const sbx_qe2000_mvt_entry_t *entry)
{
    return soc_qe2000_mvt_entry_set(qeunit, entryId, entry);
}

int
_sbx_qe2000_mvt_init(const int qeUnit)
{
    return soc_qe2000_mvt_init(qeUnit);
}

#ifdef BCM_WARM_BOOT_SUPPORT
STATIC INLINE void
soc_qe2000_get_wb_free_data_frm_block(const int qeUnit,
                                      int mvt_id_block,
                                      SHR_BITDCL **bits,
                                      int *base)
{
    switch (mvt_id_block) {
    case SBX_MVT_ID_GLOBAL:
        *bits = mvtState_p[qeUnit]->globalUse;
        *base = mvtState_p[qeUnit]->globalBase;
        break;
    case SBX_MVT_ID_LOCAL:
        *bits = mvtState_p[qeUnit]->localUse;
        *base = mvtState_p[qeUnit]->localBase;
        break;
    case SBX_MVT_ID_VLAN:
        *bits = mvtState_p[qeUnit]->vlanUse;
        *base = mvtState_p[qeUnit]->vlanBase;
        break;
    default:
        *bits = NULL;
        *base = 0;
    }
}

STATIC INLINE void
soc_qe2000_get_wb_free_data_frm_id(const int qeUnit,
                                   int mvt_id,
                                   SHR_BITDCL **bits,
                                   int *base)
{
    if (SBX_MVT_MODE_VER0 == sbx_mvt_mode) {
        *bits = g_mvtState->mvtUse;
        *base = MVT_NUM_STATIC;
    } else if (SBX_MVT_MODE_VER1 == sbx_mvt_mode)  {
        if ((mvt_id >= SBX_MVT_ID_VSI_BASE) &&
            (mvt_id <= SBX_MVT_ID_VSI_END)) {
            *bits = mvtState_p[qeUnit]->vlanUse;
            *base = mvtState_p[qeUnit]->vlanBase;
        } else if ((mvt_id >= SBX_MVT_ID_DYNAMIC_BASE) &&
                   (mvt_id <= (mvtState_p[qeUnit]->mcgroup_local_start_index - 1))) {
            *bits = mvtState_p[qeUnit]->globalUse;
            *base = mvtState_p[qeUnit]->globalBase;
        } else if ((mvt_id >= mvtState_p[qeUnit]->mcgroup_local_start_index) &&
                   (mvt_id <= SBX_MVT_ID_DYNAMIC_END)) {
            *bits = mvtState_p[qeUnit]->localUse;
            *base = mvtState_p[qeUnit]->localBase;
        } else {
            *bits = NULL;
            *base = 0;
        }
    }
}
#endif /* def BCM_WARM_BOOT_SUPPORT */

STATIC INLINE shr_idxres_list_handle_t
soc_qe2000_get_mvt_free_list_frm_block(const int qeUnit, int mvt_id_block)
{
    shr_idxres_list_handle_t free_list;


    if (mvtState_p[qeUnit] == NULL) {
        return(NULL);
    }
    if ((mvt_id_block != SBX_MVT_ID_GLOBAL) && (mvt_id_block != SBX_MVT_ID_LOCAL) &&
                        (mvt_id_block != SBX_MVT_ID_VLAN) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, Invalid mvt_id_block Unit(%d), MvtBlock(%d)\n"), FUNCTION_NAME(), qeUnit, mvt_id_block));
        return(NULL);
    }

    free_list = (mvt_id_block == SBX_MVT_ID_GLOBAL) ?  mvtState_p[qeUnit]->GlobalFreeList :
                     ((mvt_id_block == SBX_MVT_ID_LOCAL) ? mvtState_p[qeUnit]->LocalFreeList :
                         mvtState_p[qeUnit]->VlanFreeList);

    if (free_list == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, NULL Free list, Unit(%d)\n"), FUNCTION_NAME(), qeUnit));
    }

    return(free_list);
}


STATIC INLINE shr_idxres_list_handle_t
soc_qe2000_get_mvt_free_list_frm_id(const int qeUnit, int mvt_id)
{
    shr_idxres_list_handle_t free_list;


    if (sbx_mvt_mode == SBX_MVT_MODE_VER0) {
        if (!g_mvtState) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, MVT(Ver0) Not Initialized, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
            return(NULL);
        }

        free_list = g_mvtState->freeList;
    }
    else if (sbx_mvt_mode == SBX_MVT_MODE_VER1)  {
        if (mvtState_p[qeUnit] == NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, MVT(Ver1) Not Initialized, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
            return(NULL);
        }

        if ( (mvt_id >= SBX_MVT_ID_VSI_BASE) && (mvt_id <= SBX_MVT_ID_VSI_END) ) {
            free_list = mvtState_p[qeUnit]->VlanFreeList;
        }
        else if ( (mvt_id >= SBX_MVT_ID_DYNAMIC_BASE) &&
                     (mvt_id <= (mvtState_p[qeUnit]->mcgroup_local_start_index - 1)) ) {
            free_list = mvtState_p[qeUnit]->GlobalFreeList;
        }
        else if ( (mvt_id >= mvtState_p[qeUnit]->mcgroup_local_start_index) &&
                     (mvt_id <= SBX_MVT_ID_DYNAMIC_END) ) {
            free_list = mvtState_p[qeUnit]->LocalFreeList;
        }
        else {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s, Inconsistency, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
            free_list = NULL;
        }
    }
    else {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, MVT(Ver Unknown), unit(%d)\n"), FUNCTION_NAME(), qeUnit));
        free_list = NULL;
    }

    return(free_list);
}

soc_error_t
soc_qe2000_mvt_initialization(const int qeUnit)
{
    int rv = SOC_E_NONE, rc;
    int xrv;
    sbx_mvt_state_info_t *tempState;
    sbx_mvt_state_info_t *orgState;
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;
    sal_usecs_t startTime;
    int nNumInitMtv;
    int i;
    int is_global = TRUE, is_local = TRUE;
    int dynamic_end;
    int local_start = 0, local_end = 0, global_start = 0, global_end = 0;
#ifdef BCM_WARM_BOOT_SUPPORT
    uint32 wbSize = 0;
    uint32 wbOldSize = 0;
    uint8 *wbPtr = NULL;
    _sbx_qe2000_mvt_warm_boot_m1_hdr_t *hdrPtr;
    int j, k;
#endif /* def BCM_WARM_BOOT_SUPPORT */
#if SBX_MVT_ID_BD_SIZE
    int adj_dynamic_end, dynamic_available;
#endif /* SBX_MVT_ID_BD_SIZE */


    COMPILER_REFERENCE(is_global);
    COMPILER_REFERENCE(is_local);

    /* consistency checks */
    if (qeUnit >= BCM_MAX_NUM_UNITS) {
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META("%s, Inconsistency, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
        return(SOC_E_INIT);
    }
    orgState = mvtState_p[qeUnit];
#if 0 
    /* To test easy reload, the state may be already initialized - we will need to reallocate it */
#ifdef BCM_EASY_RELOAD_SUPPORT
    if (!SOC_IS_RELOADING(qeUnit)) {
#endif
	if (mvtState_p[qeUnit] != NULL) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s mvtState_p not null, unit(%d)\n"), FUNCTION_NAME(), qeUnit));
	    return(SOC_E_INTERNAL);
	}
#ifdef BCM_EASY_RELOAD_SUPPORT
    }
#endif
#endif 
    if (sbx_mvt_mode == SBX_MVT_MODE_VER0) {
      LOG_ERROR(BSL_LS_SOC_COMMON,
                (BSL_META("%s, Inconsistency, unit(%d) sbx_mvt_mode(%d)\n"), FUNCTION_NAME(), qeUnit, sbx_mvt_mode));
        return(SOC_E_INTERNAL);
    }

    startTime = sal_time_usecs();

    /* allocate global structures */
    tempState = sal_alloc(sizeof(sbx_mvt_state_info_t), "global MVT state");
    if (tempState == NULL) {
        return SOC_E_MEMORY;
    }
    sal_memset(tempState, 0x00, sizeof(sbx_mvt_state_info_t));
    tempState->mcgroup_local_start_index = SOC_SBX_CFG(qeUnit)->mcgroup_local_start_index;

#ifdef BCM_WARM_BOOT_SUPPORT
    /* since the handle is fixed, always just fill in the computed value */
    SOC_SCACHE_HANDLE_SET(tempState->wbStateHandle,
                          qeUnit,
                          BCM_MODULE_MULTICAST,
                          1 /* sequence */);
#endif /* def BCM_WARM_BOOT_SUPPORT */

    /* calculate dynamic range end */
    switch (SOC_SBX_CFG_QE2000(qeUnit)->uEgMVTSize) {
        case 0:
            dynamic_end = SBX_MVT_ID_12K_DYNAMIC_END;
            break;

        case 1:
            dynamic_end = SBX_MVT_ID_24K_DYNAMIC_END;
            break;

        case 2:
        default:
            dynamic_end = SBX_MVT_ID_DYNAMIC_END;
            break;
    }

#if SBX_MVT_ID_BD_SIZE
    /* SBX_MVT_ID_BD_SIZE is defined as 0, check here to make coverity happy */
    if (SBX_MVT_ID_BD_SIZE > 0) {
        adj_dynamic_end = dynamic_end - SBX_MVT_ID_BD_SIZE;

        /* calculate the ammount of space available for Global/local space */
        dynamic_available = (adj_dynamic_end > SBX_MVT_ID_DYNAMIC_BASE) ?
                                      (adj_dynamic_end - SBX_MVT_ID_VSI_END): 0;

        if (dynamic_available == 0) {
            is_global = FALSE;
            is_local = FALSE;
        }
        else {
            /* determine if local needs to be allocated */
            if ( (tempState->mcgroup_local_start_index >= adj_dynamic_end) ||
                    (tempState->mcgroup_local_start_index <= SBX_MVT_ID_DYNAMIC_BASE) ) {
                is_local = FALSE;
            }
            else {
                local_start = tempState->mcgroup_local_start_index;
                local_end = adj_dynamic_end;
            }

            /* determine if global needs to be allocated */
            global_start = SBX_MVT_ID_DYNAMIC_BASE;
            global_end = (is_local == FALSE) ?
                                     adj_dynamic_end : tempState->mcgroup_local_start_index - 1;
        }
    }
    else {
        is_global = TRUE;
        is_local = TRUE;
        global_start = SBX_MVT_ID_DYNAMIC_BASE;
        global_end = tempState->mcgroup_local_start_index - 1;
        local_start = tempState->mcgroup_local_start_index;
        local_end = dynamic_end;
    }

    if (SBX_MVT_ID_BD_SIZE > 0) {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("%s, is_local: %s, localStart: 0x%x localEnd; 0x%x\n"),
                     FUNCTION_NAME(), ((is_local == TRUE) ? "TRUE" : "FALSE"), local_start, local_end));
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("%s, is_global: %s, globalStart: 0x%x globalEnd; 0x%x\n"),
                     FUNCTION_NAME(), ((is_global == TRUE) ? "TRUE" : "FALSE"), global_start, global_end));

	if (is_local == FALSE) {
            tempState->mcgroup_local_start_index = global_end + 1;
	}
	if (is_global == FALSE) {
            tempState->mcgroup_local_start_index = SBX_MVT_ID_VSI_END;
	}
    } else {
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("%s, is_local: %s, localStart: 0x%x localEnd; 0x%x\n"),
                     FUNCTION_NAME(), "TRUE", local_start, local_end));
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("%s, is_global: %s, globalStart: 0x%x globalEnd; 0x%x\n"),
                     FUNCTION_NAME(), "TRUE", global_start, global_end));
    }

    /* allocate VLANs McGroup List */
    rv = shr_idxres_list_create(&(tempState->VlanFreeList),
                                    SBX_MVT_ID_VSI_BASE,
                                    SBX_MVT_ID_VSI_END,
                                    SBX_MVT_ID_VSI_BASE,
                                    SBX_MVT_ID_VSI_END,
                                    "mvtVlanFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += _SHR_BITDCLSIZE(SBX_MVT_ID_VSI_END - SBX_MVT_ID_VSI_BASE + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    if (BCM_FAILURE(rv)) {
        goto err;
    }

    if (SBX_MVT_ID_BD_SIZE > 0) {
        if (is_global == TRUE) {
	    /* allocate Global McGroup list */
	    rv = shr_idxres_list_create(&(tempState->GlobalFreeList),
					global_start,
					global_end,
					global_start,
					global_end,
					"mvtGlobalFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
            wbSize += _SHR_BITDCLSIZE(global_end - global_start + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
	    if (BCM_FAILURE(rv)) {
	        goto err;
	    }
	}

	if (is_local == TRUE) {
	    /* allocate Local McGroup list */
            rv = shr_idxres_list_create(&(tempState->LocalFreeList),
					local_start,
					local_end,
					local_start,
					local_end,
					"mvtLocalFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
            wbSize += _SHR_BITDCLSIZE(local_end - local_start + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
	    if (BCM_FAILURE(rv)) {
                goto err;
	    }
	}
    } else {
        /* allocate Global McGroup list */
        rv = shr_idxres_list_create(&(tempState->GlobalFreeList),
				    global_start,
				    global_end,
				    global_start,
				    global_end,
				    "mvtGlobalFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
            wbSize += _SHR_BITDCLSIZE(global_end - global_start + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
	if (BCM_FAILURE(rv)) {
	    goto err;
	}

	/* allocate Local McGroup list */
	rv = shr_idxres_list_create(&(tempState->LocalFreeList),
				    local_start,
				    local_end,
				    local_start,
				    local_end,
				    "mvtLocalFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
            wbSize += _SHR_BITDCLSIZE(local_end - local_start + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
	if (BCM_FAILURE(rv)) {
	    goto err;
	}
    }
#else
    is_global = TRUE;
    is_local = TRUE;
    global_start = SBX_MVT_ID_DYNAMIC_BASE;
    global_end = tempState->mcgroup_local_start_index - 1;
    local_start = tempState->mcgroup_local_start_index;
    local_end = dynamic_end;

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("%s, is_local: %s, localStart: 0x%x localEnd; 0x%x\n"),
                 FUNCTION_NAME(), "TRUE", local_start, local_end));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META("%s, is_global: %s, globalStart: 0x%x globalEnd; 0x%x\n"),
                 FUNCTION_NAME(), "TRUE", global_start, global_end));

    /* allocate VLANs McGroup List */
    rv = shr_idxres_list_create(&(tempState->VlanFreeList),
                                    SBX_MVT_ID_VSI_BASE,
                                    SBX_MVT_ID_VSI_END,
                                    SBX_MVT_ID_VSI_BASE,
                                    SBX_MVT_ID_VSI_END,
                                    "mvtVlanFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += _SHR_BITDCLSIZE(SBX_MVT_ID_VSI_END - SBX_MVT_ID_VSI_BASE + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    if (BCM_FAILURE(rv)) {
        goto err;
    }

    /* allocate Global McGroup list */
    rv = shr_idxres_list_create(&(tempState->GlobalFreeList),
				global_start,
				global_end,
				global_start,
				global_end,
				"mvtGlobalFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += _SHR_BITDCLSIZE(global_end - global_start + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    if (BCM_FAILURE(rv)) {
        goto err;
    }

    if (local_start <= local_end) {
        /* allocate Local McGroup list */
        rv = shr_idxres_list_create(&(tempState->LocalFreeList),
                                    local_start,
                                    local_end,
                                    local_start,
                                    local_end,
                                    "mvtLocalFreeLIst");
#ifdef BCM_WARM_BOOT_SUPPORT
        wbSize += _SHR_BITDCLSIZE(local_end - local_start + 1);
#endif /* def BCM_WARM_BOOT_SUPPORT */
        if (BCM_FAILURE(rv)) {
            goto err;
        }
    } else {
        tempState->LocalFreeList = NULL;
    }
#endif /* SBX_MVT_ID_BD_SIZE */

#ifdef BCM_WARM_BOOT_SUPPORT
    wbSize += 3; /* allow for rounding error to shift */
    wbSize *= sizeof(SHR_BITDCL); /* scale BITDCL count to bytes */
    wbSize += sizeof(_sbx_qe2000_mvt_warm_boot_m1_hdr_t); /* include header */
    if (!SOC_WARM_BOOT(qeUnit)) {
#endif /* def BCM_WARM_BOOT_SUPPORT */
        /* Init the entire MVT table for this unit */
        
        sal_memset(&sbMvtEntry, 0, sizeof(HW_QE2000_MVT_ENTRY_ST) );
        sbMvtEntry.ulNext = SBX_MVT_ID_NULL;

        nNumInitMtv = dynamic_end + 1;

        for (i = 0; i < nNumInitMtv; i++) {
            rc = hwQe2000MVTSet((sbhandle)qeUnit, sbMvtEntry, i, NULL, NULL, 0, 0, NULL);
            if (rc) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("Error on entry=%d  %d\n"), i, rc));
            }
        }

        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("Initialized %d MVT entries, in %d uSecs\n"),
                     nNumInitMtv, sal_time_usecs() - startTime));

#ifdef BCM_WARM_BOOT_SUPPORT
        /* set up warm boot recovery space */
        /* don't reallocate if it already exists */
        rv = soc_scache_ptr_get(qeUnit,
                                tempState->wbStateHandle,
                                &wbPtr,
                                &wbOldSize);
        if ((SOC_E_NOT_FOUND != rv) && (SOC_E_NONE != rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error(%s) checking scache handle on"
                                " unit:%d\n"),
                       FUNCTION_NAME(),
                       soc_errmsg(rv),
                       qeUnit));
            goto err;
        }
        if (SOC_E_NOT_FOUND == rv) {
            rv = soc_scache_alloc(qeUnit, tempState->wbStateHandle, wbSize);
            if (SOC_E_NONE != rv) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Error(%s) allocating WB scache handle"
                                    " on unit:%d \n"),
                           FUNCTION_NAME(),
                           soc_errmsg(rv),
                           qeUnit));
                goto err;
            }
            rv = soc_scache_ptr_get(qeUnit,
                                    tempState->wbStateHandle,
                                    &wbPtr,
                                    &wbOldSize);
            if (SOC_E_NONE != rv) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META("%s: Error(%s) checking scache handle on"
                                    " unit:%d\n"),
                           FUNCTION_NAME(),
                           soc_errmsg(rv),
                           qeUnit));
                goto err;
            }
        } /* if (SOC_E_NOT_FOUND == rc) */
        if (wbOldSize < wbSize) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d scache block is too small (%d;"
                                " needed %d)\n"),
                       FUNCTION_NAME(),
                       qeUnit,
                       wbOldSize,
                       wbSize));
            rv = BCM_E_RESOURCE;
            goto err;
        }
        sal_memset((void*)(wbPtr + sizeof(_sbx_qe2000_mvt_warm_boot_m1_hdr_t)),
                   0x00,
                   wbSize - sizeof(_sbx_qe2000_mvt_warm_boot_m1_hdr_t));
        hdrPtr = (_sbx_qe2000_mvt_warm_boot_m1_hdr_t*)wbPtr;
        hdrPtr->mvtMode = SBX_MVT_MODE_VER1;
        hdrPtr->hdrVersion = 0;
        hdrPtr->unused = 0;
        hdrPtr->vlanLow = SBX_MVT_ID_VSI_BASE;
        hdrPtr->vlanHigh = SBX_MVT_ID_VSI_END;
        hdrPtr->globalLow = global_start;
        hdrPtr->globalHigh = global_end;
        hdrPtr->localLow = local_start;
        hdrPtr->localHigh = local_end;
        tempState->vlanUse = (SHR_BITDCL*)(&((hdrPtr[1])));
        tempState->vlanBase = SBX_MVT_ID_VSI_BASE;
        tempState->globalUse = &(tempState->vlanUse[_SHR_BITDCLSIZE(hdrPtr->vlanHigh - hdrPtr->vlanLow + 1)]);
        tempState->globalBase = global_start;
        tempState->localUse = &(tempState->globalUse[_SHR_BITDCLSIZE(hdrPtr->globalHigh - hdrPtr->globalLow + 1)]);
        tempState->localBase = local_start;
#endif /* def BCM_WARM_BOOT_SUPPORT */

        mvtState_p[qeUnit] = tempState;
        sbx_mvt_mode = SBX_MVT_MODE_VER1;
#ifdef BCM_WARM_BOOT_SUPPORT
    } else { /* if (!SOC_WARM_BOOT(unit)) */
        rv = soc_scache_ptr_get(qeUnit,
                                tempState->wbStateHandle,
                                &wbPtr,
                                &wbOldSize);
        if (SOC_E_NONE != rv) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Error(%s) getting scache handle on"
                                " unit:%d\n"),
                       FUNCTION_NAME(),
                       soc_errmsg(rv),
                       qeUnit));
            goto err;
        }
        if (wbOldSize < wbSize) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d scache block is too small (%d;"
                                " needed %d)\n"),
                       FUNCTION_NAME(),
                       qeUnit,
                       wbOldSize,
                       wbSize));
            rv = BCM_E_RESOURCE;
            goto err;
        }
        hdrPtr = (_sbx_qe2000_mvt_warm_boot_m1_hdr_t*)wbPtr;
        /* sanity check */
        if (0 != hdrPtr->hdrVersion) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: Unknown state version %d\n"),
                       FUNCTION_NAME(),
                       hdrPtr->hdrVersion));
            rv = BCM_E_CONFIG;
            goto err;
        }
        if ((SBX_MVT_MODE_VER1 != hdrPtr->mvtMode) ||
            (SBX_MVT_ID_VSI_BASE != hdrPtr->vlanLow) ||
            (SBX_MVT_ID_VSI_END != hdrPtr->vlanHigh) ||
            (global_start != hdrPtr->globalLow) ||
            (global_end != hdrPtr->globalHigh) ||
            (local_start != hdrPtr->localLow) ||
            (local_end != hdrPtr->localHigh)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: unit %d configuration has changed\n"),
                       FUNCTION_NAME(),
                       qeUnit));
            rv = BCM_E_CONFIG;
            goto err;
        }
        tempState->vlanUse = (SHR_BITDCL*)(&((hdrPtr[1])));
        tempState->globalUse = &(tempState->vlanUse[_SHR_BITDCLSIZE(hdrPtr->vlanHigh - hdrPtr->vlanLow + 1)]);
        tempState->localUse = &(tempState->globalUse[_SHR_BITDCLSIZE(hdrPtr->globalHigh - hdrPtr->globalLow + 1)]);
        /* set up VLAN use */
        k = hdrPtr->vlanHigh - hdrPtr->vlanLow + 1;
        for (i = 0; i < k; i++) {
            if (SHR_BITGET(tempState->vlanUse, i)) {
                /* found one in use; collect as many contiguous as we can */
                j = i + 1;
                while ((j < k) && SHR_BITGET(tempState->vlanUse, j)) {
                    j++;
                }
                j--;
                /* reserve elements [i..j] */
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META("Reserve unit %d VLAN entries %08X..%08X\n"),
                             qeUnit,
                             i + hdrPtr->vlanLow,
                             j + hdrPtr->vlanLow));
                rv = shr_idxres_list_reserve(tempState->VlanFreeList,
                                             i + hdrPtr->vlanLow,
                                             j + hdrPtr->vlanLow);
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unable to reserve unit %d VLAN"
                                        " elements %d..%d: %d (%s)\n"),
                               FUNCTION_NAME(),
                               qeUnit,
                               i + hdrPtr->vlanLow,
                               j + hdrPtr->vlanLow,
                               rv,
                               _SHR_ERRMSG(rv)));
                    goto err;
                }
                /* resume search at next element */
                i = j + 1;
            }
        }
        /* set up global use */
        k = hdrPtr->globalHigh - hdrPtr->globalLow + 1;
        for (i = 0; i < k; i++) {
            if (SHR_BITGET(tempState->globalUse, i)) {
                /* found one in use; collect as many contiguous as we can */
                j = i + 1;
                while ((j < k) && SHR_BITGET(tempState->globalUse, j)) {
                    j++;
                }
                j--;
                /* reserve elements [i..j] */
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META("Reserve unit %d global entries %08X..%08X\n"),
                             qeUnit,
                             i + hdrPtr->globalLow,
                             j + hdrPtr->globalLow));
                rv = shr_idxres_list_reserve(tempState->GlobalFreeList,
                                             i + hdrPtr->globalLow,
                                             j + hdrPtr->globalLow);
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unable to reserve unit %d global"
                                        " elements %d..%d: %d (%s)\n"),
                               FUNCTION_NAME(),
                               qeUnit,
                               i + hdrPtr->globalLow,
                               j + hdrPtr->globalLow,
                               rv,
                               _SHR_ERRMSG(rv)));
                    goto err;
                }
                /* resume search at next element */
                i = j + 1;
            }
        }
        /* set up local use */
        k = hdrPtr->localHigh - hdrPtr->localLow;
        for (i = 0; i < k; i++) {
            if (SHR_BITGET(tempState->localUse, i)) {
                /* found one in use; collect as many contiguous as we can */
                j = i + 1;
                while ((j < k) && SHR_BITGET(tempState->localUse, j)) {
                    j++;
                }
                j--;
                /* reserve elements [i..j] */
                LOG_VERBOSE(BSL_LS_SOC_COMMON,
                            (BSL_META("Reserve unit %d local entries %08X..%08X\n"),
                             qeUnit,
                             i + hdrPtr->localLow,
                             j + hdrPtr->localLow));
                rv = shr_idxres_list_reserve(tempState->LocalFreeList,
                                             i + hdrPtr->localLow,
                                             j + hdrPtr->localLow);
                if (BCM_E_NONE != rv) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: unable to reserve unit %d local"
                                        " elements %d..%d: %d (%s)\n"),
                               FUNCTION_NAME(),
                               qeUnit,
                               i + hdrPtr->localLow,
                               j + hdrPtr->localLow,
                               rv,
                               _SHR_ERRMSG(rv)));
                    goto err;
                }
                /* resume search at next element */
                i = j + 1;
            }
        }
        mvtState_p[qeUnit] = tempState;
        sbx_mvt_mode = SBX_MVT_MODE_VER1;
    } /* if (!SOC_WARM_BOOT(unit)) */
#endif /* def BCM_WARM_BOOT_SUPPORT */
    if (orgState) {
        /* was reiniting; get rid of old information */
        LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META("Discard unit %d prior state %08X; reinit at %08X\n"),
                     qeUnit,
                     (uint32)orgState,
                     (uint32)tempState));
        xrv = shr_idxres_list_destroy(orgState->GlobalFreeList);
        xrv = shr_idxres_list_destroy(orgState->LocalFreeList);
        xrv = shr_idxres_list_destroy(orgState->VlanFreeList);
        COMPILER_REFERENCE(xrv);
        sal_free(orgState);
    }

    return(rv);

err:
    if (tempState != NULL) {
        if (tempState->VlanFreeList != NULL) {
            shr_idxres_list_destroy(tempState->VlanFreeList);
        }
        if (tempState->GlobalFreeList != NULL) {
            shr_idxres_list_destroy(tempState->GlobalFreeList);
        }
        if (tempState->LocalFreeList != NULL) {
            shr_idxres_list_destroy(tempState->LocalFreeList);
        }
        sal_free(tempState);
    }

    return(rv);
}

soc_error_t
soc_qe2000_mvt_entry_allocate(const int qeUnit,
                              const int num,
                              sbx_mvt_id_t pEntryIds[],
                              int mvt_id_block)
{
    int rv = SOC_E_NONE;
    int nCount;
    int aCount;
    sbx_qe2000_mvt_entry_t mvtEntry;
    shr_idxres_element_t idxResElt;
    shr_idxres_list_handle_t FreeList;
#ifdef BCM_WARM_BOOT_SUPPORT
    SHR_BITDCL *freeBits = NULL;
    int freeBase = 0;
#endif /* def BCM_WARM_BOOT_SUPPORT */

    /* check initialisation */
    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }
    if ((mvt_id_block != SBX_MVT_ID_GLOBAL)
        && (mvt_id_block != SBX_MVT_ID_LOCAL)
        && (mvt_id_block != SBX_MVT_ID_VLAN) ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, Invalid mvt_id_block Unit(%d), MvtBlock(%d)\n"), FUNCTION_NAME(), qeUnit, mvt_id_block));
        return(SOC_E_INIT);
    }

    FreeList = soc_qe2000_get_mvt_free_list_frm_block(qeUnit, mvt_id_block);
    if (FreeList == NULL) {
        return(SOC_E_INIT);
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_qe2000_get_wb_free_data_frm_block(qeUnit,
                                          mvt_id_block,
                                          &freeBits,
                                          &freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */

    for (nCount = 0, aCount = 0; (nCount < num) && BCM_SUCCESS(rv); nCount++, aCount++) {
        rv = shr_idxres_list_alloc(FreeList, &idxResElt);
        if (BCM_FAILURE(rv)) {
          LOG_ERROR(BSL_LS_SOC_COMMON,
                    (BSL_META("%s, shr_idxres_list_alloc Unit(%d), nCount(%d), aCount(%d)\n"), FUNCTION_NAME(), qeUnit, nCount, aCount));
        }

        pEntryIds[nCount] = (sbx_mvt_id_t)idxResElt;
#ifdef BCM_WARM_BOOT_SUPPORT
        SHR_BITSET(freeBits, idxResElt - freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */
    }

    if (BCM_SUCCESS(rv)) {
        /* link them together if succesful and more than one allocated */
        for (nCount = 1; nCount < num; nCount++) {
            rv = soc_qe2000_mvt_entry_get_frm_id(qeUnit,
						 pEntryIds[nCount - 1],
						 &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
            mvtEntry.next = pEntryIds[nCount];

            LOG_INFO(BSL_LS_SOC_COMMON,
                     (BSL_META("Linking entry[%d]=%d to entry[%d]=%d\n"),
                      nCount-1, pEntryIds[nCount-1],
                      nCount, pEntryIds[nCount]));

            rv = soc_qe2000_mvt_entry_set_frm_id(qeUnit,
						 pEntryIds[nCount - 1],
						 &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
        }
    }

    if (BCM_FAILURE(rv)) {
        /* free the tried entries on failure */
        for (nCount = 0; nCount < aCount; nCount++) {
            shr_idxres_list_free(FreeList, pEntryIds[nCount]);
#ifdef BCM_WARM_BOOT_SUPPORT
            SHR_BITCLR(freeBits, pEntryIds[nCount] - freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */
            pEntryIds[nCount] = SBX_MVT_ID_NULL;
        }
    }

    return(rv);
}


int
soc_qe2000_mvt_entry_reserve_id(const int qeUnit,
				const int num,
				const sbx_mvt_id_t pEntryIds[])
{
    int rv = SOC_E_NONE;
    int nCount;
    int aCount;
    sbx_qe2000_mvt_entry_t mvtEntry;
    shr_idxres_list_handle_t FreeList;
#ifdef BCM_WARM_BOOT_SUPPORT
    SHR_BITDCL *freeBits = NULL;
    int freeBase = 0;
#endif /* def BCM_WARM_BOOT_SUPPORT */

    /* check initialisation */
    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    for (nCount = 0, aCount = 0; (nCount < num) && BCM_SUCCESS(rv); nCount++, aCount++) {
        FreeList = soc_qe2000_get_mvt_free_list_frm_id(qeUnit, pEntryIds[nCount]);
        if (FreeList == NULL) {
            rv = SOC_E_PARAM;
            continue;
        }
#ifdef BCM_WARM_BOOT_SUPPORT
        soc_qe2000_get_wb_free_data_frm_id(qeUnit,
                                           pEntryIds[nCount],
                                           &freeBits,
                                           &freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */

        rv = shr_idxres_list_elem_state(FreeList, pEntryIds[nCount]);
        if (rv == BCM_E_NOT_FOUND) {
            rv = shr_idxres_list_reserve(FreeList,
                                         pEntryIds[nCount],
                                         pEntryIds[nCount]);
#ifdef BCM_WARM_BOOT_SUPPORT
            if (BCM_E_NONE == rv) {
                SHR_BITSET(freeBits, pEntryIds[nCount] - freeBase);
            }
#endif /* def BCM_WARM_BOOT_SUPPORT */
        }

    }

    if (BCM_SUCCESS(rv)) {
        /* link them together if succesful and more than one allocated */
        for (nCount = 1; nCount < num; nCount++) {
            rv = soc_qe2000_mvt_entry_get(qeUnit,
                                           pEntryIds[nCount - 1],
                                           &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
            mvtEntry.next = pEntryIds[nCount];

            LOG_INFO(BSL_LS_SOC_COMMON,
                     (BSL_META("Linking entry[%d]=%d to entry[%d]=%d\n"),
                      nCount-1, pEntryIds[nCount-1],
                      nCount, pEntryIds[nCount]));

            rv = soc_qe2000_mvt_entry_set(qeUnit,
                                           pEntryIds[nCount - 1],
                                           &mvtEntry);
            if( SOC_FAILURE(rv) ) {
                break;
            }
        }
    }

    if (BCM_FAILURE(rv)) {

        /* free the tried entries on failure, but don't free the last one
         * if it was found to exist already
         */
        if (rv == BCM_E_EXISTS) {
            aCount--;
        }

        for (nCount = 0; nCount < aCount; nCount++) {
            FreeList = soc_qe2000_get_mvt_free_list_frm_id(qeUnit, pEntryIds[nCount]);
            if (FreeList == NULL) {
                rv = SOC_E_PARAM;
                continue;
            }
#ifdef BCM_WARM_BOOT_SUPPORT
            soc_qe2000_get_wb_free_data_frm_id(qeUnit,
                                               pEntryIds[nCount],
                                               &freeBits,
                                               &freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */

            shr_idxres_list_free(FreeList, pEntryIds[nCount]);
#ifdef BCM_WARM_BOOT_SUPPORT
            SHR_BITCLR(freeBits, pEntryIds[nCount] - freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */
        }
    }

    return(rv);
}

soc_error_t
soc_qe2000_mvt_entry_free_frm_id(const int qeUnit,
				 const sbx_mvt_id_t entryId)
{
    int rv = SOC_E_NONE;
    shr_idxres_list_handle_t FreeList;
#ifdef BCM_WARM_BOOT_SUPPORT
    SHR_BITDCL *freeBits = NULL;
    int freeBase = 0;
#endif /* def BCM_WARM_BOOT_SUPPORT */


    /* check initialisation */
    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    FreeList = soc_qe2000_get_mvt_free_list_frm_id(qeUnit,entryId);
    if (FreeList == NULL) {
        return(SOC_E_PARAM);
    }
#ifdef BCM_WARM_BOOT_SUPPORT
    soc_qe2000_get_wb_free_data_frm_id(qeUnit,
                                       entryId,
                                       &freeBits,
                                       &freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */

    rv = shr_idxres_list_free(FreeList, entryId);
#ifdef BCM_WARM_BOOT_SUPPORT
    SHR_BITCLR(freeBits, entryId - freeBase);
#endif /* def BCM_WARM_BOOT_SUPPORT */

    return rv;
}

soc_error_t
soc_qe2000_mvt_entry_chk_frm_id(const int qeUnit,
				const sbx_mvt_id_t entryId)
{
    int rv = SOC_E_NONE;
    shr_idxres_list_handle_t FreeList;

    /* check initialization */
    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return rv ;
    }

    FreeList = soc_qe2000_get_mvt_free_list_frm_id(qeUnit,entryId);
    if (FreeList == NULL) {
        return SOC_E_PARAM ;
    }

    /* caller must allocate the entry before using it */
    if (!(entryId <= SBX_MVT_ID_VSI_END)) {
        rv = shr_idxres_list_elem_state(FreeList, entryId);
        if (BCM_E_EXISTS == rv) {
            rv = SOC_E_NONE;
        }
    }
    return rv;
}

soc_error_t
soc_qe2000_mvt_entry_get_frm_id(const int qeUnit,
				const sbx_mvt_id_t entryId,
				sbx_qe2000_mvt_entry_t *pEntry)
{
    int rv = SOC_E_NONE;
    int sbxRv;
    int nPort;
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;
    shr_idxres_list_handle_t FreeList;


    /* check initialization */
    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    FreeList = soc_qe2000_get_mvt_free_list_frm_id(qeUnit,entryId);
    if (FreeList == NULL) {
        return(SOC_E_PARAM);
    }

    /* If we are reloading, read the entry from the hardware, it will be allocated later if it is set up */
#ifdef BCM_EASY_RELOAD_SUPPORT
	if (SOC_IS_RELOADING(qeUnit) == 0) {
#endif
	    /* caller must allocate the entry before using it */
	    if (!(entryId <= SBX_MVT_ID_VSI_END)) {
		rv = shr_idxres_list_elem_state(FreeList, entryId);
		if (rv != BCM_E_EXISTS ) {
		    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s, Element not allocated Unit(%d), Element(0x%x)\n"), FUNCTION_NAME(), qeUnit, entryId));
		    return(rv);    /* could be some other idxres error... */
		}
		rv = SOC_E_NONE;
	    }
#ifdef BCM_EASY_RELOAD_SUPPORT
	}
#endif

    sal_memset(&sbMvtEntry, 0, sizeof(HW_QE2000_MVT_ENTRY_ST));

    /* semaphore params not used */
    sbxRv = hwQe2000MVTGet((sbhandle)qeUnit, &sbMvtEntry, entryId,
                           NULL, NULL, 0, 0, NULL);
    if( sbxRv ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("MVTGet Failed:(%d)\n"), sbxRv ));
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s, hwQe2000MVTGet, unit(%d), Err: 0x%x\n"), FUNCTION_NAME(), qeUnit, sbxRv));
        return(SOC_E_FAIL);
    }

    /* Convert from HW_QE2000_MVT_ENTRY_ST to sbx_mvt_id_t */
    pEntry->egress_data_a   = sbMvtEntry.ulMvtdA;
    pEntry->egress_data_b   = sbMvtEntry.ulMvtdB;
    pEntry->source_knockout = sbMvtEntry.bSourceKnockout;
    pEntry->next            = sbMvtEntry.ulNext;

    SOC_PBMP_CLEAR(pEntry->ports);
    for (nPort=0; nPort < SBX_MAX_PORTS; nPort++) {
        if (sbMvtEntry.bPortEnable[nPort]) {
            SOC_PBMP_PORT_ADD(pEntry->ports, nPort);
        }
    }
    return(rv);
}

soc_error_t
soc_qe2000_mvt_entry_set_frm_id(const int qeUnit,
				const sbx_mvt_id_t entryId,
				const sbx_qe2000_mvt_entry_t *pEntry)
{
    int rv = SOC_E_NONE;
    int sbxRv, nPort;
    HW_QE2000_MVT_ENTRY_ST sbMvtEntry;
    shr_idxres_list_handle_t FreeList;


    /* check initialisation */
    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return(rv);
    }

    FreeList = soc_qe2000_get_mvt_free_list_frm_id(qeUnit,entryId);
    if (FreeList == NULL) {
        return(SOC_E_PARAM);
    }

    if (!(entryId <= SBX_MVT_ID_VSI_END)) {
        /* caller must allocate the entry before using it */
        rv = shr_idxres_list_elem_state(FreeList, entryId);
        if( rv != BCM_E_EXISTS ) {
            return(rv);  /* could be some other idxres error... */
        }
        rv = SOC_E_NONE;
    }

    sal_memset(&sbMvtEntry, 0, sizeof(HW_QE2000_MVT_ENTRY_ST));

    /* Convert from sbx_mvt_id_t to HW_QE2000_MVT_ENTRY_ST */
    sbMvtEntry.ulMvtdA = pEntry->egress_data_a;
    sbMvtEntry.ulMvtdB = pEntry->egress_data_b;
    sbMvtEntry.bSourceKnockout = pEntry->source_knockout;
    sbMvtEntry.ulNext  = pEntry->next;

    for (nPort = 0; nPort < HW_QE2000_MAX_NUM_PORTS_K; nPort++) {
        sbMvtEntry.bPortEnable[nPort] =
            SOC_PBMP_MEMBER(pEntry->ports, nPort)?1:0;
    }

    /* Now set it, semaphore params not used */
    sbxRv = hwQe2000MVTSet((sbhandle)qeUnit, sbMvtEntry, entryId,
                           NULL, NULL, 0, 0, NULL);
    if( sbxRv ) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("MVTSet Failed:(%d)\n"), sbxRv ));
        return(SOC_E_FAIL);
    }

    return(rv);
}

int
soc_qe2000_mvt_entry_id_valid(const int qeUnit,
                          const sbx_mvt_id_t entryId)
{
    int rv = SOC_E_NONE;


    rv = soc_qe2000_mvt_consistency_check(qeUnit);
    if (BCM_FAILURE(rv)) {
        return(FALSE);
    }

    if (entryId >= mvtState_p[qeUnit]->mcgroup_local_start_index) {
        return(FALSE);
    }

    return(TRUE);
}

int
soc_qe2000_mvt_state_get(int unit, char *pbuf)
{

    sbx_mvt_id_t  mvtId;
    int rv = BCM_E_NONE;
    shr_idxres_list_handle_t FreeList;
    char *pbuf_current = pbuf;
    int vlanCount = 0;

    /* check initialization */
    rv = soc_qe2000_mvt_consistency_check(unit);
    if (BCM_FAILURE(rv)) {
	LOG_CLI((BSL_META_U(unit,
                            "consistency check error\n")));
        return(rv);
    }

    for (mvtId = 0; mvtId < MVT_NUM_TOTAL; mvtId++) {

	FreeList = soc_qe2000_get_mvt_free_list_frm_id(unit, mvtId);
	if (FreeList == NULL) {
	    LOG_CLI((BSL_META_U(unit,
                                "mvt list not found for mvtId(%d)\n"), (int32)mvtId));
	    continue;
	}

	/* caller must allocate the entry before using it */
	if (!(mvtId <= SBX_MVT_ID_VSI_END)) {
	    rv = shr_idxres_list_elem_state(FreeList, mvtId);
	    if (rv != BCM_E_EXISTS ) {
		continue;
	    }
	}
	/* Only register the non-VLAN free list since the VLAN */
	/* list is always pre-allocated.                       */
	if (FreeList != mvtState_p[unit]->VlanFreeList) {

	    rv = sal_sprintf(pbuf_current, " mvtId(%d) in use\n",
			     mvtId);

	    if (rv < 0) {
		LOG_CLI((BSL_META_U(unit,
                                    "rv = %d\n"), rv));
		return BCM_E_RESOURCE;
	    }
	    pbuf_current += rv;
	}
	if (FreeList == mvtState_p[unit]->VlanFreeList) {
	    vlanCount++;
	}
    }
    rv = sal_sprintf(pbuf_current, " %d vlans currently allocated\n",
		     vlanCount);
    return(BCM_E_NONE);
}

#endif /* BCM_QE2000_SUPPORT */
