/*
 * $Id: error.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Error translation
 */

#include <bcm/error.h>

#include "sbWrappers.h"
#include <bcm_int/sbx/error.h>
#include <soc/sbx/sbx_drv.h>

/*
 *   Function
 *      translate_sbx_result
 *   Purpose
 *      Translate a Sandburst result code into a Broadcom result code
 *   Parameters
 *      (in) sbStatus_t result = Sandburst result to translate
 *   Returns
 *      bcm_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it just guesses and returns
 *      BCM_E_FAIL instead of something more specific.  It is possible that
 *      this result is wrong if there's a 'pending' or similar result that is
 *      not included in the switch statement in this function.  Therefore,
 *      this must be updated whenever BCM_E_* is expanded or SB_* results are
 *      expanded, in order to ensure consistency to the intended behaviour.
 *      The numeric comments to the right are just to make it easier to keep
 *      this sorted and faster to look up for humans.
 */
int
translate_sbx_result(const sbStatus_t result)
{
    switch(result) {
    /* SBX results indicate successful completion */
    case SB_OK:                                                  /* xxxx0000 */
        return BCM_E_NONE;

    /* SBX results indicate an internal error */
    case SB_UCODE_BAD_IMAGE_ERR_CODE:                            /* xxxx0526 */
    case SB_UCODE_INCOMPATIBLE_ERR_CODE:                         /* xxxx0527 */
    case SB_UCODE_SYMBOL_NOT_FOUND:                              /* xxxx0528 */
    case SB_BAD_CHIP_IDENT_ERR_CODE:                             /* xxxx052C */
    case SB_SVID_INT_ERROR:                                      /* xxxx0538 */
         return BCM_E_INTERNAL;

    /* SBX results indicate a memory is full */
    case SB_MAC_NO_MEM:                                          /* xxxx0507 */
    case SB_MALLOC_FAILED:                                       /* xxxx050B */
    case SB_LPM_OUT_OF_HOST_MEMORY:                              /* xxxx051C */
    case SB_LPM_OUT_OF_DEVICE_MEMORY:                            /* xxxx051D */
    case SB_EGR_MC_OUT_OF_DEV_MEM:                               /* xxxx051C */
        return BCM_E_MEMORY;

    /* SBX results indicate invalid device handles */
    case SB_INVALID_HANDLE:                                      /* xxxx0533 */
        return BCM_E_UNIT;

    /* SBX results indicate invalid parameters */
    case SB_BAD_ARGUMENT_ERR_CODE:                               /* xxxx0500 */
    case SB_IPV4_BAD_SLICE1_SIZE_PARAM_ERR_CODE:                 /* xxxx0510 */
    case SB_IPV6_BAD_SLICE1_SIZE_PARAM_ERR_CODE:                 /* xxxx0511 */
    case SB_IPV4_BAD_MEM_AREA_PARAMS_ERR_CODE:                   /* xxxx0512 */
    case SB_LPM_DROP_MASK_ERROR:                                 /* xxxx0521 */
    case SB_IPV6_INVALID_PREFIX_LEN_ERR_CODE:                    /* xxxx0523 */
    case SB_IPV6_128_NEEDS_64_PREFIX:                            /* xxxx0524 */
    case SB_MAC_NO_PAYLOAD_DESC:                                 /* xxxx052D */
    case SB_INVALID_LIST:         /* xxxx053B */
        return BCM_E_PARAM;

    /* SBX results indicate something is empty */
    /* case <none> */
    /*     return BCM_E_EMPTY; */

    /* SBX results indicate something is full */
    case SB_MAC_COL:                                             /* xxxx0504 */
    case SB_MAC_FULL:                                            /* xxxx0506 */
    case SB_MAC_TOO_MANY:                                        /* xxxx050A */
    case SB_CLS_TOO_MANY_IPV4_RULES:                             /* xxxx050D */
    case SB_CLS_TOO_MANY_L2_RULES:                               /* xxxx050E */
    case SB_IPV4_ADD_KEY_COLLISION_ERR_CODE:                     /* xxxx0517 */
    case SB_IPV4_SET_CLSID_MEMORY_FULL_ERR_CODE:                 /* xxxx0518 */
    case SB_IPV4_ADD_ADDR_MEMORY_FULL_ERR_CODE:                  /* xxxx0519 */
    case SB_ACE_TOO_MANY_ACES:                                   /* xxxx052E */
    case SB_SVID_TOO_MANY_ENTRIES:                               /* xxxx0534 */
    case SB_SVID_COL:                                            /* xxxx0535 */
        return BCM_E_FULL;

    /* SBX results indicate something was not found */
    case SB_MAC_NOT_FOUND:                                       /* xxxx0508 */
    case SB_IPV4_NO_SUCH_KEY_ERR_CODE:                           /* xxxx0516 */
    case SB_LPM_ADDRESS_NOT_FOUND:                               /* xxxx051F */
    case SB_ACE_END_OF_LIST:                                     /* xxxx0531 */
    case SB_SVID_KEY_NOT_FOUND:                                  /* xxxx0537 */
    case SB_NOT_FOUND:                                           /* xxxx053A */
    case SB_EGR_MC_PORT_NOT_DEFINED:                             /* xxxx053D */
        return BCM_E_NOT_FOUND;

    /* SBX results indicate an item already exists */
    case SB_MAC_DUP:                                             /* xxxx0505 */
    case SB_MAC_PRESENT:                                         /* xxxx050C */
    case SB_LPM_CALL_UPDATE_INSTEAD:                             /* xxxx051E */
    case SB_LPM_DUPLICATE_ADDRESS:                               /* xxxx0520 */
    case SB_ACE_DUPLICATE_PATTERN:                               /* xxxx0532 */
    case SB_SVID_DUP:                                            /* xxxx0536 */
    case SB_EGR_MC_PORT_ALREADY_DEFINED:                         /* xxxx053E */
        return BCM_E_EXISTS;

    /* SBX results indicate an operation has timed out */
    case SB_FE_MEM_ACC_TIMEOUT_ERR_CODE:                         /* xxxx0503 */
    case SB_CLS_RAB_SWITCH_TIMEOUT:                              /* xxxx050F */
    case SB_CLS_RAB_SWITCH_TIMEOUT_ERR_CODE:                     /* xxxx0529 */
    case SB_TIMEOUT_ERR_CODE:                                    /* xxxx052A */
        return BCM_E_TIMEOUT;

    /* SBX results indicate an operation is pending */
    case SB_IN_PROGRESS:                                         /* xxxx0001 */
    case SB_MORE:                                                /* xxxx0002 */
    case SB_BUSY_ERR_CODE:                                       /* xxxx0501 */
    case SB_SYNC_INIT_RETURNED_IN_PROGRESS_ERR_CODE:             /* xxxx0525 */
        return BCM_E_BUSY;

    /* SBX results indicate a failed operation */
    
    case SB_MAC_NOT_COMMITTED:                                   /* xxxx0509 */
    case SB_IPV4_MODULE_ALREADY_INITIALIZED_ERR_CODE:            /* xxxx0515 */
    case SB_OTHER_ERR_CODE:                                      /* xxxx052B */
    case SB_ACE_HANDLE_NOT_COMMITTED:                            /* xxxx0530 */
        return BCM_E_FAIL;

    /* SBX retults indicate a disabled operation */
    /* case <none> */
    /*     return BCM_E_DISABLED; */

    /* SBX retults indicate an invalid ID */
    case SB_ACE_INV_ACE_HANDLE:                                  /* xxxx052F */
    case SB_IPV6_INVALID_STATS_ID:                               /* xxxx0539 */
        return BCM_E_BADID;

    /* SBX results indicates a resource is exhausted */
    case SB_IPV4_TOO_MANY_ADDS_WITHOUT_COMMIT_ERR_CODE:          /* xxxx051A */
    case SB_IPV4_COMMIT_TOO_MANY_ROUTES_ERR_CODE:                /* xxxx051B */
        /* the error indicates a resource is exhausted */
        return BCM_E_RESOURCE;

    /* SBX results indicate an invalid configuration*/
    /* case <none> */
    /*     return BCM_E_CONFIG; */

    /* SBX results indicate unimplemented/unavailable functions */
    case SB_UNIMPLEMENTED_FUNCTION_ERR_CODE:                     /* xxxx0502 */
        return BCM_E_UNAVAIL;

    /* SBX results indicate something is uninitialised */
    case SB_IPV4_MODULE_NOT_INITIALIZED_ERR_CODE:                /* xxxx0513 */
    case SB_IPV4_MODULE_HARDWARE_NOT_INITIALIZED_ERR_CODE:       /* xxxx0514 */
        return BCM_E_INIT;

    /* SBX results indicate an invalid port */
    case SB_LPM_INV_DROP_PORT:                                   /* xxxx0522 */
        return BCM_E_PORT;

    /* SBX results indicate a port has not finished autonegotiate */
    /* case <none> */
    /*     return BCM_E_AUTONEG_INCOMPLETE; */

    /* SBX results indicate a port has failed autonegotiate */
    /* case <none> */
    /*     return BCM_E_AUTONEG_FAIL; */

    /* SBX results indicate a MAC is not initialised */
    /* case <none> */
    /*     return BCM_E_MAC_INIT; */

    /* Anything else is just guessing... */
    default:
        return BCM_E_FAIL;
    } /* switch(result) */
}

/*
 *   Function
 *      translate_sbx_elib_result
 *   Purpose
 *      Translate a Sandburst ELib result code into a Broadcom result code
 *   Parameters
 *      (in) sbElibStatus_et result = Sandburst result to translate
 *   Returns
 *      bcm_error_t = Closest reasonable Broadcom result
 *   Notes
 *      If there is no reasonably close error, it just guesses and returns
 *      BCM_E_FAIL instead of something more specific.  It is possible that
 *      this result is wrong if there's a 'pending' or similar result that is
 *      not included in the switch statement in this function.  Therefore,
 *      this must be updated whenever BCM_E_* is expanded or SB_* results are
 *      expanded, in order to ensure consistency to the intended behaviour.
 *      The numeric comments to the right are just to make it easier to keep
 *      this sorted and faster to look up for humans.
 */

int
translate_sbx_elib_result(const sbElibStatus_et result)
{
    switch(result) {
    /* SBX results indicate successful completion */
    case SB_ELIB_OK:                                             /* xxxx0000 */
        return BCM_E_NONE;

    /* SBX results indicate an internal error */
    case SB_ELIB_VLAN_MEM_FREE_FAIL:                             /* xxxx0017 */
         return BCM_E_INTERNAL;

    /* SBX results indicate a memory is full */
    case SB_ELIB_MEM_ALLOC_FAIL:                                 /* xxxx000E */
    case SB_ELIB_VLAN_MEM_ALLOC_FAIL:                            /* xxxx0016 */
        return BCM_E_MEMORY;

    /* SBX results indicate invalid device handles */
        return BCM_E_UNIT;

    /* SBX results indicate invalid parameters */
    case SB_ELIB_BAD_ARGS:                                       /* xxxx0001 */
    case SB_ELIB_BAD_IDX:                                        /* xxxx0002 */
    case SB_ELIB_BAD_VID:                                        /* xxxx0018 */
        return BCM_E_PARAM;

    /* SBX results indicate something is empty */
    /* case <none> */
    /*     return BCM_E_EMPTY; */

    /* SBX results indicate something is full */
    /* case <none> */
    /*    return BCM_E_FULL; */

    /* SBX results indicate something was not found */
    /* case <none> */
    /*    return BCM_E_NOT_FOUND;  */

    /* SBX results indicate an item already exists */
    /* case <none> */
    /*    return BCM_E_EXISTS;  */

    /* SBX results indicate an operation has timed out */
    case SB_ELIB_IND_MEM_TIMEOUT:                                /* xxxx0004 */
        return BCM_E_TIMEOUT;

    /* SBX results indicate an operation is pending */
    /* case <none> */
    /*    return BCM_E_BUSY;  */

    /* SBX results indicate a failed operation */
    
    case SB_ELIB_INIT_FAIL:                                      /* xxxx0003 */
    case SB_ELIB_INIT_CIT_FAIL:                                  /* xxxx0005 */
    case SB_ELIB_INIT_CRT_FAIL:                                  /* xxxx0006 */
    case SB_ELIB_GENERAL_FAIL:                                   /* xxxx0007 */
    case SB_ELIB_PORT_ST_GET_FAIL:                               /* xxxx0008 */
    case SB_ELIB_PORT_ST_SET_FAIL:                               /* xxxx0009 */
    case SB_ELIB_VIT_SET_FAIL:                                   /* xxxx000A */
    case SB_ELIB_VIT_GET_FAIL:                                   /* xxxx000B */
    case SB_ELIB_VRT_SET_FAIL:                                   /* xxxx000C */
    case SB_ELIB_VRT_GET_FAIL:                                   /* xxxx000D */
    case SB_ELIB_PORT_CFG_GET_FAIL:                              /* xxxx000F */
    case SB_ELIB_PORT_CFG_SET_FAIL:                              /* xxxx0010 */
    case SB_ELIB_BIST_FAIL:                                      /* xxxx0011 */
    case SB_ELIB_SEM_GET_FAIL:                                   /* xxxx0012 */
    case SB_ELIB_SEM_GIVE_FAIL:                                  /* xxxx0013 */
    case SB_ELIB_DMA_FAIL:                                       /* xxxx0014 */
    case SB_ELIB_COUNT_FAIL:                                     /* xxxx0015 */
        return BCM_E_FAIL;

    /* SBX retults indicate a disabled operation */
    /* case <none> */
    /*     return BCM_E_DISABLED; */

    /* SBX retults indicate an invalid ID */
    /* case <none> */
    /*    return BCM_E_BADID;  */

    /* SBX results indicates a resource is exhausted */
        return BCM_E_RESOURCE;

    /* SBX results indicate an invalid configuration*/
    /* case <none> */
    /*     return BCM_E_CONFIG; */

    /* SBX results indicate unimplemented/unavailable functions */
    /* case <none> */
    /*    return BCM_E_UNAVAIL; */

    /* SBX results indicate something is uninitialised */
    /* case <none> */
    /*    return BCM_E_INIT; */

    /* SBX results indicate an invalid port */
    /* case <none> */
    /*    return BCM_E_PORT; */

    /* SBX results indicate a port has not finished autonegotiate */
    /* case <none> */
    /*     return BCM_E_AUTONEG_INCOMPLETE; */

    /* SBX results indicate a port has failed autonegotiate */
    /* case <none> */
    /*     return BCM_E_AUTONEG_FAIL; */

    /* SBX results indicate a MAC is not initialised */
    /* case <none> */
    /*     return BCM_E_MAC_INIT; */

    /* Anything else is just guessing... */
    default:
        return BCM_E_FAIL;
    } /* switch(result) */
}

/*
 *   Function
 *      sbx_result_string
 *   Purpose
 *      Get the text representation of a Sandburst result code.
 *   Parameters
 *      (in) sbStatus_t sbxResult = Sandburst result to translate
 *   Returns
 *      const char * = Text String explaining the error.
 *   Notes
 *      Straight ripoff of the (unavailable) sbGetFeIlibStatusString()
 */
const char *
sbx_result_string(sbStatus_t status)
{
    switch (status) {
    case SB_OK:
        return "ok";
    case SB_IN_PROGRESS:
        return "in progress";
    case SB_MORE:
        return "more to do";
    case SB_BAD_ARGUMENT_ERR_CODE:
        return "bad argument";
    case SB_BUSY_ERR_CODE:
        return "unexpectedly busy";
    case SB_UNIMPLEMENTED_FUNCTION_ERR_CODE:
        return "unimplemented function";
    case SB_FE_MEM_ACC_TIMEOUT_ERR_CODE:
        return "indirect memory timeout";
    case SB_MAC_COL:
        return "MAC address collision";
    case SB_MAC_DUP:
        return "duplicate MAC address";
    case SB_MAC_FULL:
        return "MAC table is full";
    case SB_MAC_NO_MEM:
        return "MAC FE memory exhausted";
    case SB_MAC_NOT_FOUND:
        return "MAC address not found";
    case SB_MAC_NOT_COMMITTED:
        return "MAC address not yet committed";
    case SB_MALLOC_FAILED:
        return "malloc failed";
    case SB_MAC_PRESENT:
        return "MAC address already present";
    case SB_CLS_TOO_MANY_IPV4_RULES:
        return "too many IPv4 rules";
    case SB_CLS_TOO_MANY_L2_RULES:
        return "too many L2 rules";
    case SB_IPV4_BAD_SLICE1_SIZE_PARAM_ERR_CODE:
        return "invalid IPv4 slice 1 size";
    case SB_IPV4_BAD_MEM_AREA_PARAMS_ERR_CODE:
        return "invalid IPv4 memory area parameter";
    case SB_IPV4_MODULE_NOT_INITIALIZED_ERR_CODE:
        return "IPv4 not initialized";
    case SB_IPV4_MODULE_HARDWARE_NOT_INITIALIZED_ERR_CODE:
        return "IPv4 hardware not initialized";
    case SB_IPV4_MODULE_ALREADY_INITIALIZED_ERR_CODE:
        return "IPv4 already initialized";
    case SB_IPV4_NO_SUCH_KEY_ERR_CODE:
        return "IPv4 address not found";
    case SB_IPV4_ADD_KEY_COLLISION_ERR_CODE:
        return "IPv4 address already present";
    case SB_IPV4_SET_CLSID_MEMORY_FULL_ERR_CODE:
        return "Node memory not available for sbSetIpv4ClassifierIds()";
    case SB_IPV4_ADD_ADDR_MEMORY_FULL_ERR_CODE:
        return "Node/param memory not available for sbAddIpv4{Sa,Da}()";
    case SB_IPV4_TOO_MANY_ADDS_WITHOUT_COMMIT_ERR_CODE:
        return "Too many sbAddIpv4{Sa,Da}() calls without a commit";
    case SB_IPV4_COMMIT_TOO_MANY_ROUTES_ERR_CODE:
        return "Too many routes for sbIpv4Commit()";
    case SB_LPM_OUT_OF_HOST_MEMORY:
        return "out of host memory";
    case SB_LPM_OUT_OF_DEVICE_MEMORY:
        return "out of device memory";
    case SB_LPM_DUPLICATE_ADDRESS:
        return "duplicate address";
    case SB_LPM_ADDRESS_NOT_FOUND:
        return "address not found";
    case SB_SYNC_INIT_RETURNED_IN_PROGRESS_ERR_CODE:
        return "synchronous init returned in progress";
    case SB_UCODE_BAD_IMAGE_ERR_CODE:
        return "bad ucode image";
    case SB_UCODE_INCOMPATIBLE_ERR_CODE:
        return "incompatible ucode image";
    case SB_UCODE_SYMBOL_NOT_FOUND:
        return "ucode symbol not found";
    case SB_CLS_RAB_SWITCH_TIMEOUT_ERR_CODE:
        return "classifier bank switch timeout";
    case SB_TIMEOUT_ERR_CODE:
        return "timeout";
    case SB_OTHER_ERR_CODE:
        return "unspecified error";
    case SB_BAD_CHIP_IDENT_ERR_CODE:
        return "bad chip id";
    case SB_MAC_NO_PAYLOAD_DESC:
        return "no MAC payload desc";
    case SB_ACE_TOO_MANY_ACES:
        return "too many aces";
    case SB_ACE_INV_ACE_HANDLE:
        return "invalid ace handle";
    case SB_ACE_HANDLE_NOT_COMMITTED:
        return "ace handle not committed";
    case SB_ACE_END_OF_LIST:
        return "ace end of list";
    case SB_ACE_DUPLICATE_PATTERN:
        return "ace duplicate pattern";
    case SB_INVALID_HANDLE:
        return "invalid handle";
    case SB_SVID_TOO_MANY_ENTRIES:
        return "svid too many entries";
    case SB_SVID_COL:
        return "svid collision";
    case SB_SVID_DUP:
        return "svid duplicate";
    case SB_SVID_KEY_NOT_FOUND:
        return "svid key not found";
    case SB_SVID_INT_ERROR:
        return "svid internal error";
    case SB_IPV6_INVALID_STATS_ID:
        return "invalid IPv6 stats ID";
    case SB_NOT_FOUND:
        return "not found";
    case SB_INVALID_LIST:
        return "invalid list";
    case SB_EGR_MC_OUT_OF_DEV_MEM:
        return "egress multicast out of device memory";
    case SB_EGR_MC_PORT_NOT_DEFINED:
        return "egress multicast port not defined";
    case SB_EGR_MC_PORT_ALREADY_DEFINED:
        return "egress multicast port already defined";
    default:
        return "unknown error code";
    }
}

