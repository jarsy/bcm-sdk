#ifndef _SB_FE_ILIB_STATUS_H_
#define _SB_FE_ILIB_STATUS_H_
/* --------------------------------------------------------------------------
**
** $Id: sbFeIlibStatus.h,v 1.5 Broadcom SDK $
**
** $Copyright: (c) 2016 Broadcom.
** Broadcom Proprietary and Confidential. All rights reserved.$
** 
** sbStatus.h: Sandburst driver status codes
**
** --------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{

  SB_OK,
  SB_IN_PROGRESS = SB_BUILD_ERR_CODE(SB_VENDOR_ID_SANDBURST, SB_MODULE_ID_FE_ILIB, 0x1),
  SB_MORE,

  SB_BAD_ARGUMENT_ERR_CODE = SB_BUILD_ERR_CODE(SB_VENDOR_ID_SANDBURST, SB_MODULE_ID_FE_ILIB, 0x500),
  SB_BUSY_ERR_CODE,			/* 0x501 */
  SB_UNIMPLEMENTED_FUNCTION_ERR_CODE,	/* 0x502 */
  SB_FE_MEM_ACC_TIMEOUT_ERR_CODE,		/* 0x503 */
  SB_MAC_COL,				/* 0x504 */
  SB_MAC_DUP,				/* 0x505 */
  SB_MAC_FULL,				/* 0x506 */
  SB_MAC_NO_MEM,				/* 0x507 */
  SB_MAC_NOT_FOUND,			/* 0x508 */
  SB_MAC_NOT_COMMITTED,			/* 0x509 */
  SB_MAC_TOO_MANY,                      /* 0x50a */
  SB_MALLOC_FAILED,			/* 0x50b */
  SB_MAC_PRESENT,   /* For internal L2 use */
  SB_CLS_TOO_MANY_IPV4_RULES,		/* 0x50d */
  SB_CLS_TOO_MANY_L2_RULES,		/* 0x50e */
  SB_CLS_RAB_SWITCH_TIMEOUT,		/* 0x50f */

  /* ---- iuParams.ipv4TableSize1 param is bad */
  SB_IPV4_BAD_SLICE1_SIZE_PARAM_ERR_CODE,	/* 0x510 */

  /* ---- iuParams.ipv6TableSize1 param is bad */
  SB_IPV6_BAD_SLICE1_SIZE_PARAM_ERR_CODE,	/* 0x511 */


  /* ---- Bad params: cParams.pktfMem{A,B}Size,
   *                  iuParams.ipv4{Src,Dest}TableStart1,
   *                  iuParams.ipv4{Src,Dest}TableStart2,
   *                  iuParams.ipv4{Src,Dest}TableSize2
   */
  SB_IPV4_BAD_MEM_AREA_PARAMS_ERR_CODE,	/* 0x512 */

  /* ---- HW init() or API call before module initialization */
  SB_IPV4_MODULE_NOT_INITIALIZED_ERR_CODE,/* 0x513 */

  /* ---- API call after module init but before module hardware init */
  SB_IPV4_MODULE_HARDWARE_NOT_INITIALIZED_ERR_CODE,/* 0x514 */

  /* ---- Duplicate call to HWinit() */
  SB_IPV4_MODULE_ALREADY_INITIALIZED_ERR_CODE,	/* 0x515 */

  /* ---- No such key, in call to SetIpv4{Sa,Da}Default(),
   *      UpdateIpv4{Sa,Da}(), RemIpv4{Sa,Da}()
   */
  SB_IPV4_NO_SUCH_KEY_ERR_CODE,		/* 0x516 */

  /* ---- Key already exists in AddIpv4{Sa,Da}() */
  SB_IPV4_ADD_KEY_COLLISION_ERR_CODE,	/* 0x517 */

  /* ---- Node memory exhausted during sbSetIpv4ClassifierIds() */
  SB_IPV4_SET_CLSID_MEMORY_FULL_ERR_CODE,	/* 0x518 */

  /* ---- Node memory exhausted during sbAddIpv4{Sa,Da}() */
  SB_IPV4_ADD_ADDR_MEMORY_FULL_ERR_CODE,	/* 0x519 */

  /* ---- Too many sbAddIpv4{Sa,Da}() calls without a commit */
  SB_IPV4_TOO_MANY_ADDS_WITHOUT_COMMIT_ERR_CODE,	/* 0x51a */

  /* ---- Commit: not enough memory, too many routes */
  SB_IPV4_COMMIT_TOO_MANY_ROUTES_ERR_CODE,/* 0x51b */

  /* ---------------------------------------------------------------- */

  SB_LPM_OUT_OF_HOST_MEMORY,		/* 0x51c */
  SB_LPM_OUT_OF_DEVICE_MEMORY,		/* 0x51d */
  SB_LPM_CALL_UPDATE_INSTEAD,             /* 0x51e */
  SB_LPM_ADDRESS_NOT_FOUND,		/* 0x51f */
  SB_LPM_DUPLICATE_ADDRESS,		/* 0x520 */
  SB_LPM_DROP_MASK_ERROR,                 /* 0x521 */
  SB_LPM_INV_DROP_PORT,                   /* 0x522 */

  /* ---------------------------------------------------------------- */

  SB_IPV6_INVALID_PREFIX_LEN_ERR_CODE,	/* 0x523 */
  SB_IPV6_128_NEEDS_64_PREFIX,		/* 0x524 */

  SB_SYNC_INIT_RETURNED_IN_PROGRESS_ERR_CODE,/* 0x525 */

  /* Signal that the UCODE image is Bad, or for other version of chip */
  SB_UCODE_BAD_IMAGE_ERR_CODE,		/* 0x526 */
  SB_UCODE_INCOMPATIBLE_ERR_CODE,		/* 0x527 */
  SB_UCODE_SYMBOL_NOT_FOUND,		/* 0x528 */
  SB_CLS_RAB_SWITCH_TIMEOUT_ERR_CODE,	/* 0x529 */
  SB_TIMEOUT_ERR_CODE,			/* 0x52a */
  SB_OTHER_ERR_CODE,			/* 0x52b */
  SB_BAD_CHIP_IDENT_ERR_CODE,		/* 0x52c */
  SB_MAC_NO_PAYLOAD_DESC,			/* 0x52d */
  SB_ACE_TOO_MANY_ACES,                 /* 0x52e */
  SB_ACE_INV_ACE_HANDLE,                /* 0x52f */
  SB_ACE_HANDLE_NOT_COMMITTED,          /* 0x530 */
  SB_ACE_END_OF_LIST,                   /* 0x531 */
  SB_ACE_DUPLICATE_PATTERN,             /* 0x532 */

  SB_INVALID_HANDLE,                    /* 0x533 */

  SB_SVID_TOO_MANY_ENTRIES,		/* 0x534 */
  SB_SVID_COL,				/* 0x535 */
  SB_SVID_DUP,                          /* 0x536 */
  SB_SVID_KEY_NOT_FOUND,		/* 0x537 */
  SB_SVID_INT_ERROR,                    /* 0x538 */
  SB_IPV6_INVALID_STATS_ID,             /* 0x539 */
  SB_NOT_FOUND,                         /* 0x53a */
  SB_INVALID_LIST,                      /* 0x53b */
  SB_NO_UCODE_FOUND,                    /* 0x53c */
  SB_INVALID_RANGE,                     /* 0x53d */
  SB_EGR_MC_PORT_ALREADY_DEFINED,       /* 0x53e */
  SB_EGR_MC_OUT_OF_DEV_MEM,             /* 0x53f */
  SB_EGR_MC_PORT_NOT_DEFINED,           /* 0x540 */
  SB_PM_TOO_MANY_POLICERS,              /* 0x541 */
  SB_PM_TOO_MANY_TIMERS,                /* 0x542 */
  SB_PM_TOO_MANY_SEQUENCEGENERATORS,    /* 0x543 */

  /* fill in new error codes before SB_LAST_ERR_CODE
   * must be last */
  SB_LAST_ERR_CODE
} sbIlibStatus_t, *sbIlibStatus_pt;

char * sbGetFeIlibStatusString(sbStatus_t status);

#ifdef __cplusplus
}
#endif
#endif
