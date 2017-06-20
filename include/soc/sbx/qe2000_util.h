/*
 * $Id: qe2000_util.h,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _QE2000_UTIL_H
#define _QE2000_UTIL_H

/*#include "sbTypes.h" */
#include "glue.h"

#include "fabric/sbZfHwQe2000QsPriLutEntry.hx"


#define HW_QE2000_MAX_NUM_PORTS_K 50

/*
 * Egress Buffer Memory
 */
#define HW_QE2000_EB_MEM_MAX_OFFSET          0x7FFF
#define HW_QE2000_EB_MVT_ROW_V               (12)

#define HW_QE2000_EB_MVT_ROW_GET(_index, _mcgrpsz)    ( HW_QE2000_EB_MEM_MAX_OFFSET - \
                                                    ( _index & (( 1 << ( HW_QE2000_EB_MVT_ROW_V + _mcgrpsz ))-1)) )

#define HW_QE2000_EB_MVT_COL_GET(_index, _mcgrpsz)    ( (_index >> ( HW_QE2000_EB_MVT_ROW_V + _mcgrpsz )) & 0x3)

#define HW_QE2000_EB_MVT_MIN(_mcgrpsz)                ( (1 << (HW_QE2000_EB_MVT_ROW_V + _mcgrpsz)) * 3)

/**
 * Multicast Vector Table (MVT) Entry
 *
 *
 */
typedef struct hw_qe2000_mvt_entry_s {
    /* 22087: note: field below should be sbBool_t.  Changed to unit32_t for socket transport */
    uint32 bPortEnable[ HW_QE2000_MAX_NUM_PORTS_K ]; /**< Port Enable array - TRUE means multicast to this port. */
    uint32 ulMvtdA;       /**< MVTD_A value - This is a 14-bit cookie that is available to the EP. */
    uint32 ulMvtdB;       /**< MVTD_B value - this is a 4-bit cookie that is available to the EP. */
    /* 22087: note: field below should be sbBool_t.  Changed to unit32_t for socket transport */
    uint32 bSourceKnockout; /**< Enable Source Knockout for this entry */
    uint32 ulNext;        /**< 16-bit Offset to the next MVT 0xFFFF terminates list */
} HW_QE2000_MVT_ENTRY_ST, *HW_QE2000_MVT_ENTRY_PST;

/**
 * semaphore try & wait function pointer
 *
 * Function pointer to user supplied semaphore try & wait function.
 * Takes semaphore ID (int) and timeout in seconds.
 * Returns 0 on OK, some other value for failure.
 */
typedef int (*HW_QE2000_SEM_TRYWAIT_PF)(int nSemId, void* pUserSemData, int nTimeOut);

/**
 * semaphore give function pointer
 *
 * Function pointer to user supplied semaphore give function.
 * Takes semaphore ID (int).
 * Returns 0 on OK, some other valure for failure.
 */
typedef int (*HW_QE2000_SEM_GIVE_PF)(int nSemId, void* pUserSemData);

/**
 * Get an Multicast Vector Table Entry
 *
 *
 * @param Handle       A valid handle
 * @param pEntry       The MVT entry stored at the given index.  Upon success, the MVT
 *                     entry will be placed at this location.
 * @param uIndex       The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param uTimeOut     Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 incates success.  Non-zero indicates failure.
 */
int hwQe2000MVTGet( sbhandle                 userDeviceHandle,
		    HW_QE2000_MVT_ENTRY_PST  pEntry,
		    uint32                 uIndex,
		    HW_QE2000_SEM_TRYWAIT_PF pfSemTryWait,
		    HW_QE2000_SEM_GIVE_PF    pfSemGive,
		    int                      nSemId,
		    uint32                 uTimeOut,
		    void                     *pUserData);

/**
 * Set an Multicast Vector Table Entry
 *
 *
 * @param Handle     A valid device handle.
 *
 * @param Entry        The MVT entry to be placed in the table.
 * @param uIndex       The MVT entry index.
 * @param pfSemTryWait Pointer to the Semaphore Get function
 * @param pfSemGive    Pointer to the Semaphore Give function
 * @param nSemId       The semaphore ID
 * @param uTimeOut     Time out value passed to the sem get
 * @param pUserData    User specified data to be passed to the sem get/give functions
 *
 * @return           Status.  0 indicates success.  Non-zero indicates failure.
 */
int hwQe2000MVTSet( sbhandle                 userDeviceHandle,
			HW_QE2000_MVT_ENTRY_ST   Entry,
			uint32                 uIndex,
			HW_QE2000_SEM_TRYWAIT_PF pfSemTryWait,
			HW_QE2000_SEM_GIVE_PF    pfSemGive,
			int                      nSemId,
			uint32                 uTimeOut,
			void                     *pUserData);


#endif /* _QE2000_UTIL_H */
