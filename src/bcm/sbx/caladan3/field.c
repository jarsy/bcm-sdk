/*
 * $Id: field.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Field Processor APIs
 *
 * Purpose:
 *     'Field Processor' (FP) API for BCM88200 (SBX FE-2000 + Guadalupe-2000)
 *     This is the outer module that covers G2kP2 and G2kP3.
 */

#define _SBX_CALADAN3_FIELD_H_NEEDED_ TRUE

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/field.h>

#include <bcm_int/sbx/caladan3/field.h>
#include <bcm_int/sbx/caladan3/wb_db_field.h>

/*
 *  A note about locking:
 *
 *  This module uses one lock per unit (plus there is a lock used by the list
 *  manager when managing 'ranges', plus one lock that is only used to protect
 *  resources global to all units). This unit lock prevents multiple concurrent
 *  transactions against a specific unit. However, this granularity is possibly
 *  suboptimal in some cases (such as those cases where only a group or entry
 *  or range is being manipulated), though not in all (locks for range, entry,
 *  and group would have to be taken when committing changes to the hardware;
 *  for group and entry when inserting/removing entries; &c). It is possible
 *  that a finer granularity could be used, but this means more complicated
 *  locking process and additional resources per unit.  The lock per unit
 *  method was chosen primarily to control complexity, but it also has the
 *  advantage of not needing as many resources.
 */

/******************************************************************************
 *
 *  Local functions and data
 */

/* Global lock, for protecting unit init and detach functions */
static volatile sal_mutex_t _sbx_caladan3_field_glob_lock = NULL;

const _sbx_caladan3_field_unit_info_t _sbx_caladan3_field_unit_initValue =
    { NULL, SOC_SBX_UCODE_TYPE_NONE, NULL};
_sbx_caladan3_field_unit_info_t _sbx_caladan3_field[BCM_MAX_NUM_UNITS];

/*
 *  These locals are needed in all of the functions; just type it once.
 */
#define _CALADAN3_FIELD_COMMON_LOCALS \
    sal_mutex_t          lock;              /* unit lock */ \
    soc_sbx_ucode_type_t microcode;         /* unit microcode version */ \
    void                 *unitData;         /* unit private data */ \
    int                  result             /* working result */

/*
 *  This table provides the values the RCE needs to locate the field
 * data.
 */
_sbx_caladan3_field_header_info_t caladan3_rce_field_info[] = {
    {bcmFieldQualifySrcMac, socC3RceDataHeaderEther, SOC_C3_RCE_FRAME_BIT( 6, 7), 48, socC3RCEQualType_prefix},
    {bcmFieldQualifyDstMac, socC3RceDataHeaderEther, SOC_C3_RCE_FRAME_BIT( 0, 7), 48, socC3RCEQualType_prefix},
    {bcmFieldQualifySrcIp, socC3RceDataHeaderIpv4, SOC_C3_RCE_FRAME_BIT(12, 7), 32, socC3RCEQualType_prefix},
    {bcmFieldQualifyDstIp, socC3RceDataHeaderIpv4, SOC_C3_RCE_FRAME_BIT(16, 7), 32, socC3RCEQualType_prefix},
    {bcmFieldQualifyInPort, socC3RceDataMetadata, socC3RceMetadataInPortNum, 7, socC3RCEQualType_masked},
    {bcmFieldQualifyOuterVlan, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 7), 16, socC3RCEQualType_prefix},
    {bcmFieldQualifyOuterVlanId, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 3), 12, socC3RCEQualType_prefix},
    {bcmFieldQualifyOuterVlanPri, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 7), 3, socC3RCEQualType_prefix},
    {bcmFieldQualifyOuterVlanCfi, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 4), 1, socC3RCEQualType_prefix},
    {bcmFieldQualifyL4SrcPort, socC3RceDataHeaderTcpUdp, SOC_C3_RCE_FRAME_BIT(0, 7), 16, socC3RCEQualType_prefix},
    {bcmFieldQualifyL4DstPort, socC3RceDataHeaderTcpUdp, SOC_C3_RCE_FRAME_BIT(2, 7), 16, socC3RCEQualType_prefix},
    {bcmFieldQualifyEtherType, socC3RceDataHeaderEther, SOC_C3_RCE_FRAME_BIT(12, 7), 16, socC3RCEQualType_masked},
    {bcmFieldQualifyIpProtocol, socC3RceDataHeaderIpv4, SOC_C3_RCE_FRAME_BIT(9, 7), 8, socC3RCEQualType_prefix},
    {bcmFieldQualifyTos, socC3RceDataHeaderIpv4, SOC_C3_RCE_FRAME_BIT(1, 7), 8, socC3RCEQualType_masked},
    {bcmFieldQualifyTcpControl, socC3RceDataHeaderTcpUdp, SOC_C3_RCE_FRAME_BIT(13, 5), 6, socC3RCEQualType_prefix},
    {bcmFieldQualifyOutPort, socC3RceDataMetadata, socC3RceMetadataOutPortNum, 7, socC3RCEQualType_masked},
    {bcmFieldQualifyCount, 0, 0, 0, 0}
};

_sbx_caladan3_field_header_info_t caladan3_ip6_rce_field_info[] = {
    {bcmFieldQualifySrcIp6, socC3RceDataHeaderIpv6, SOC_C3_RCE_FRAME_BIT(8, 7), 128, socC3RCEQualType_prefix},
    {bcmFieldQualifyDstIp6, socC3RceDataHeaderIpv6, SOC_C3_RCE_FRAME_BIT(24, 7), 128, socC3RCEQualType_prefix},
    {bcmFieldQualifySrcMac, socC3RceDataHeaderEther, SOC_C3_RCE_FRAME_BIT( 6, 7), 48, socC3RCEQualType_prefix},
    {bcmFieldQualifyDstMac, socC3RceDataHeaderEther, SOC_C3_RCE_FRAME_BIT( 0, 7), 48, socC3RCEQualType_prefix},
    {bcmFieldQualifyInPort, socC3RceDataMetadata, socC3RceMetadataInPortNum, 7, socC3RCEQualType_masked},
    {bcmFieldQualifyOuterVlan, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 7), 16, socC3RCEQualType_prefix},
    {bcmFieldQualifyOuterVlanId, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 3), 12, socC3RCEQualType_prefix},
    {bcmFieldQualifyOuterVlanPri, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 7), 3, socC3RCEQualType_prefix},
    {bcmFieldQualifyOuterVlanCfi, socC3RceDataHeaderVlan, SOC_C3_RCE_FRAME_BIT(2, 4), 1, socC3RCEQualType_prefix},
    {bcmFieldQualifyEtherType, socC3RceDataHeaderEther, SOC_C3_RCE_FRAME_BIT(12, 7), 16, socC3RCEQualType_masked},
    {bcmFieldQualifyIp6NextHeader, socC3RceDataHeaderIpv6, SOC_C3_RCE_FRAME_BIT(6, 7), 8, socC3RCEQualType_prefix},
    {bcmFieldQualifyIp6TrafficClass, socC3RceDataHeaderIpv6, SOC_C3_RCE_FRAME_BIT(0, 3), 8, socC3RCEQualType_masked},
    {bcmFieldQualifyOutPort, socC3RceDataMetadata, socC3RceMetadataOutPortNum, 7, socC3RCEQualType_masked},
    {bcmFieldQualifyCount, 0, 0, 0, 0}
};

/*
 *  Name
 *    _bcm_caladan3_field_unit_intro
 *  Purpose
 *    Perform initial unit validation and claim unit lock if good
 *  Arguments
 *    (in) int unit = the unit on which we're entering field context
 *    (out) sal_mutex_t *lock = where to put the unit's lock handle
 *    (out) soc_sbx_ucode_type_t *microcode = where to put microcode version
 *    (out) void **unitData = where to put unit's data pointer
 *  Returns
 *    bcm_error_t cast as int
 *       BCM_E_NONE if successful
 *       BCM_E_UNIT if the unit number is invalid
 *       BCM_E_INIT if the unit is not initialised
 *       BCM_E_INTERNAL if there are lock problems
 *       BCM_E_CONFIG if the unit microcode has changed
 *  Notes
 *    If lock is non-NULL on return, it must be released.
 */
static int
_bcm_caladan3_field_unit_intro(int unit,
                             sal_mutex_t *lock,
                             soc_sbx_ucode_type_t *microcode,
                             void **unitData)
{
    FIELD_EVERB((BSL_META("unit %d intro check\n"), unit));
    /* check unit valid */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unit %d invalid\n"),
                   unit));
        *lock = NULL;
        return BCM_E_UNIT;
    }
    /* check unit intialised */
    if ((!(_sbx_caladan3_field_glob_lock)) ||
        (!(*lock = _sbx_caladan3_field[unit].lock))) {
        /* unit not initialised */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unit %d not initialised\n"),
                   unit));
        *lock = NULL;
        return BCM_E_INIT;
    }
    /* claim unit lock */
    FIELD_EVERB((BSL_META("take unit %d lock\n"), unit));
    if (sal_mutex_take(*lock, sal_mutex_FOREVER)) {
        /* failed to claim the lock */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "failed to take unit %d lock\n"),
                   unit));
        *lock = NULL;
        return BCM_E_INTERNAL;
    }
    /* make sure microcode type is as expected */
    *microcode = _sbx_caladan3_field[unit].microcode;
    if (*microcode != SOC_SBX_CONTROL(unit)->ucodetype) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unit %d microcode type changed %d -> %d\n"),
                   unit,
                   _sbx_caladan3_field[unit].microcode,
                   SOC_SBX_CONTROL(unit)->ucodetype));

        return BCM_E_CONFIG;
    }
    /* also get the unit's private data */
    *unitData = _sbx_caladan3_field[unit].data;
    return BCM_E_NONE;
}

/*
 *  Name
 *    _bcm_caladan3_field_unit_outro
 *  Purpose
 *    Perform cleanup and release unit lock
 *  Arguments
 *    (in) int unit = the unit on which we're entering field context
 *    (out) sal_mutex_t lock = lock being held
 *  Returns
 *     int (implied cast from bcm_error_t)
 *       BCM_E_NONE if successful
 *       BCM_E_* appropriately otherwise
 *  Notes
 *    Only releases the lock if it's not NULL.
 */
static int
_bcm_caladan3_field_unit_outro(int resultSoFar, int unit, sal_mutex_t lock)
{
    /* release unit lock */
    FIELD_EVERB((BSL_META("release unit %d lock\n"), unit));
    if (lock && sal_mutex_give(lock)) {
        /* failed to release the lock */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "failed to release unit %d lock\n"),
                   unit));
        return BCM_E_INTERNAL;
    }
    return resultSoFar;
}


/******************************************************************************
 *
 *  Implementation exported functions and data
 */


const char *_sbx_caladan3_field_qual_name[bcmFieldQualifyCount] = BCM_FIELD_QUALIFY_STRINGS;
const char *_sbx_caladan3_field_action_name[bcmFieldActionCount] = BCM_FIELD_ACTION_STRINGS;

#ifdef BROADCOM_DEBUG
/*
 *   Function
 *      _bcm_caladan3_bcm_debug
 *   Purpose
 *      Shell for LOG_CLI in certain places of the code where compiler
 *      issues have forced it to be handled in a different manner.
 *   Parameters
 *      (in) const int flags = BSL_LS_xxx print flags
 *      (in) const char *message = log message (but already processed)
 *   Notes
 *      In some compilers, the nesting of macros past a certain point causes
 *      them to get 'out of phase' with themselves, and suddenly the rest of
 *      the file doesn't make sense.  Where that has been discovered, we
 *      replace the LOG_CLI macro with a call to this function.  Unhappily,
 *      it seems we can't macro it into oblivion when debugging is disabled,
 *      since the nesting still remains.
 */
void
_bcm_caladan3_bcm_debug(const int flags,
                      const char *message)
{
    /* display the message */
    if (LOG_CHECK(flags)) {
        LOG_CLI((message));
    }
}

/*
 *   Function
 *      _bcm_caladan3_field_mac_expand
 *   Purpose
 *      Parse a MAC address from SBX rule format into something displayable.
 *   Parameters
 *      (out) char *result = where to put the MAC address string
 *      (in) int length = size of buffer for MAC address string
 *      (in) uint64 macAddr = MAC address (lower 48b; upper 16b ignored)
 *   Returns
 *      Pointer to the input buffer space.
 *   Notes
 *      The compiler mangling, er, handling of 64b numbers is sreiousyl borkne
 *      if the build is made on x86/32, but seems fine if on x86/64 and BE.
 */
char *
_bcm_caladan3_field_mac_expand(char *result,
                             unsigned int length,
                             uint64 macAddr) {
    static const uint64 guessFormat = COMPILER_64_INIT(0x12345678,0x9ABCDEF0);
                           /* 64bLE =   123456789ABCDEF0 -- looks BE here! */
                           /* 32bLE =   9ABCDEF012345678 -- Some NPs, others? */
                           /* 16bLE =   DEF09ABC56781234 -- Old DEC, others? */
                           /*  8bLE =   F0DEBC9A78563412 -- x86, others */
                           /*  4bLE =   0FEDCBA987654321 -- forget it! */
    const uint8 peekChar = *((const uint8*)(&guessFormat));
    uint8 *peekHere = (uint8*)(&macAddr);
    uint8 macParsed[8];
    unsigned int toggle;
    unsigned int index;

    /*
     *  Since the compiler can't deal with simple arithmetic and 64bit numbers,
     *  we shall just parse it from bytes, but we need to guess the direction
     *  and grain size.
     */
    switch (peekChar) {
    case 0xF0:
        toggle = 0x07;
        break;
    case 0xDE:
        toggle = 0x06;
        break;
    case 0x9A:
        toggle = 0x04;
        break;
    case 0x12:
        toggle = 0x00;
        break;
    default:
        toggle = 0x0F;
    }
    if (toggle < 0x08) {
        /* toggle distance < sizeof item, so valid */
        for (index = 0; index < 8; index++) {
            macParsed[index] = peekHere[index ^ toggle];
        }
        sal_snprintf(result,
                     length,
                     "%02X:%02X:%02X:%02X:%02X:%02X",
                     macParsed[2],
                     macParsed[3],
                     macParsed[4],
                     macParsed[5],
                     macParsed[6],
                     macParsed[7]);
    } else {
        /* don't know this one or don't want to bother with it */
        sal_snprintf(result,
                     length,
                     "%s",
                     "??:??:??:??:??:??");
        return result;
    }
    return result;
}

/*
 *  Function
 *    _bcm_caladan3_field_qset_dump
 *  Purpose
 *    Dump qualifier list from a qset
 *  Parameters
 *    (in) bcm_field_qset_t qset = the qset to dump
 *    (in) char *prefix = line prefix
 *  Returns
 *    int (implied cast from bcm_error_t)
 *                  BCM_E_NONE if successful
 *                  BCM_E_* appropriately if not
 *  Notes
 *    No error checking or locking is done here.
 */
int
_bcm_caladan3_field_qset_dump(const bcm_field_qset_t qset,
                            const char *prefix)
{
    bcm_field_qualify_t qualifier;
    unsigned int column = 0;

    /* for each qualifier potentially in the qset  */
    for (qualifier = 0; qualifier < bcmFieldQualifyCount; qualifier++) {
        /* if that qualifier actually is in the qset */
        if (BCM_FIELD_QSET_TEST(qset, qualifier)) {
            /* display the qualifier */
            if (0 == column) {
                /* just starting out */
                FIELD_PRINT(("%s%s",
                             prefix,
                             _sbx_caladan3_field_qual_name[qualifier]));
                column = (sal_strlen(prefix) +
                          sal_strlen(_sbx_caladan3_field_qual_name[qualifier]));
            } else if ((3 + column +
                       sal_strlen(_sbx_caladan3_field_qual_name[qualifier])) >=
                       _SBX_CALADAN3_FIELD_PAGE_WIDTH) {
                /* this qualifier would wrap */
                FIELD_PRINT((",\n%s%s",
                             prefix,
                             _sbx_caladan3_field_qual_name[qualifier]));
                column = (sal_strlen(prefix) +
                          sal_strlen(_sbx_caladan3_field_qual_name[qualifier]));
            } else {
                /* this qualifier fits on the line */
                FIELD_PRINT((", %s",
                             _sbx_caladan3_field_qual_name[qualifier]));
                column += (2 +
                           sal_strlen(_sbx_caladan3_field_qual_name[qualifier]));
            }
        } /* if (BCM_FIELD_QSET_TEST(thisGroup->qset, qualifier)) */
    } /* for (qualifier = 0; qualifier < bcmFieldQualifyCount; qualifier++) */
    if (0 < column) {
        FIELD_PRINT(("\n"));
    } else {
        FIELD_PRINT(("%s(none)\n", prefix));
    }
    return BCM_E_NONE;
}
#endif /* def BROADCOM_DEBUG */

/*
 *   Function
 *      _bcm_caladan3_compare_entry_priority
 *   Purpose
 *      Compare two entry priorities.  This returns -1 if the first is less
 *      than the second, 0 if the first is equal to the second (or if either
 *      priority is not valid), 1 if the first is greater than the second.
 *   Parameters
 *      (in) int pri1 = first priority to compare
 *      (in) int pri2 = second priority to compare
 *   Returns
 *      signed int = 0 if priorities are equal
 *                   >0 if pri1 is higher priority than pri2
 *                   <0 if pri1 is lower priority than pri2
 *   Notes
 *      Valid priorities are: 0..MAXINT.
 *
 *      The actual mapping of the priorities is (from highest priority to
 *      lowest priority): MAXINT down to 0.  Negative priorities are invalid
 *      for the SBX platform.
 *
 *      If a priority is invalid and the other is valid, the valid one is
 *      considered greater; if they're both invalid, they're considered equal.
 *
 *      Unhappily, this gets called a lot and it involves a lot of branches and
 *      obligatorily sequential decisions as written. It'd be better if this
 *      could be written in assembly, using inline, or made somehow otherwise
 *      more optimal; maybe later?  It does early returns and boolean
 *      short-circuiting where reasonable; this should help some.
 */
signed int
_bcm_caladan3_compare_entry_priority(int pri1,
                                   int pri2)
{
    /* validate parameters */
    /* note that we don't allow *any* negative priorities on this platform */
    if (0 > pri1) {
        /* pri1 is not valid */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("pri1 (%d) invalid\n"),
                   pri1));
        if (0 > pri2) {
            /* pri2 is not valid either, so equal   */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META("pri2 (%d) invalid\n"),
                       pri1));
            return 0;
        }
        /* pri2 is valid, so it's greater */
        return -1;
    }
    if (0 > pri2) {
        /* pri2 is not valid, so pri1 is greater (since it's valid) */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META("pri2 (%d) invalid\n"),
                   pri1));
        return 1;
    }
    /*
     *  All other cases degenerated into this, with the change from the special
     *  values being negative to them being just really wide-flung positive
     *  values, and the folding of the negative side into TCAM addresses; since
     *  we don't have a TCAM, that went away also.
     */
    return pri1 - pri2;
}

/*
 *   Function
 *      _bcm_caladan3_qset_subset
 *   Purpose
 *      Check whether qset2 is a subset of qset1.
 *   Parameters
 *      (in) bcm_field_qset_t qset1 = qualifier set 1
 *      (in) bcm_field_qset_t qset2 = qualifier set 2
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if qset2 is subset of qset1
 *                    BCM_E_FAIL if qset2 is not subset of qset1
 *   Notes
 *      This checks whether qset2 is a subset, either proper and improper, of
 *      qset1, and returns BCM_E_NONE if qset2 is a subset of qset1, but
 *      returns BCM_E_FAIL if qset2 is not a subset of qset1.  Other errors may
 *      be returned under appropriate conditions.
 */
int
_bcm_caladan3_qset_subset(bcm_field_qset_t qset1,
                        bcm_field_qset_t qset2)
{
    unsigned int index;                 /* working index for loops */
    int result;                         /* result for caller */

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META("(*,*)\n")));

    /* be optimistic */
    result = BCM_E_NONE;

    /* check all qualifiers */
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META("scanning qualifiers\n")));
    for (index = 0;
         (index < bcmFieldQualifyCount) && (BCM_E_NONE == result);
         index++) {
        if (BCM_FIELD_QSET_TEST(qset2,index)) {
            if (!BCM_FIELD_QSET_TEST(qset1,index)) {
                result = BCM_E_FAIL;
                LOG_VERBOSE(BSL_LS_BCM_FP,
                            (BSL_META("qualifier %d (%s) in qset 2"
                             " but not in qset 1\n"),
                             index,
                             _sbx_caladan3_field_qual_name[index]));
            }
        } /* if (BCM_FIELD_QSET_TEST(qset2,index)) */
    } /* for (index = 0; index < bcmFieldQualifyCount; index++) */

    /* then tell the caller the result */
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META("(*,*) = %d (%s)\n"),
               result,
               _SHR_ERRMSG(result)));
    return result;
}



/******************************************************************************
 *
 *  API exported functions and data
 */

/*
 *   Function
 *      _bcm_caladan3_field_detach
 *   Purpose
 *      Shut down the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
static int
_bcm_caladan3_field_detach(int unit,
                         sal_mutex_t lock,
                         soc_sbx_ucode_type_t microcode,
                         void *unitData)
{
    int result = BCM_E_NONE;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "(%d,%08X,%08X,%08X)\n"),
               unit,
               (uint32)lock,
               (uint32)microcode,
               (uint32)unitData));

    if (BCM_E_NONE == result) {
        /* detach the unit so nobody else will access it */
        FIELD_EVERB((BSL_META("detach unit %d\n"), unit));
        _sbx_caladan3_field[unit].lock = NULL;
        _sbx_caladan3_field[unit].data = NULL;
        _sbx_caladan3_field[unit].microcode = SOC_SBX_UCODE_TYPE_NONE;

        /* now tell the microcode specific implementation to detach */
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_detach(unitData);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
        FIELD_EVERB((BSL_META("destroy unit %d lock\n"), unit));
        sal_mutex_destroy(lock);
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 (uint32)lock,
                 (uint32)microcode,
                 (uint32)unitData,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

int bcm_caladan3_field_detach(int unit);

/*
 *  Function
 *     bcm_caladan3_field_init
 *  Purpose
 *     Initialise the field APIs.
 *  Parameters
 *     (in) int unit = the unit number
 *  Returns
 *     int (implied cast from bcm_error_t)
 *       BCM_E_NONE if successful
 *       BCM_E_* appropriately if not
 *  Notes
 */
int
bcm_caladan3_field_init(int unit)
{
    sal_mutex_t          lock;              /* unit lock */ 
    soc_sbx_ucode_type_t microcode;         /* unit microcode version */ 
    void                 *unitData;         /* unit private data */ 
    sal_mutex_t tempLock;
    int result = BCM_E_NONE;
    int index;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    /* check unit valid */
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unit %d invalid\n"),
                   unit));
        return BCM_E_UNIT;
    }

    if (_sbx_caladan3_field[unit].lock) {
        result = bcm_caladan3_field_detach(unit);
        BCM_IF_ERROR_RETURN(result);
    }

    /* Make sure the global lock exists */
    if (!_sbx_caladan3_field_glob_lock) {
        /* Global lock does not exist */
        FIELD_EVERB((BSL_META("create global lock\n")));
        tempLock = sal_mutex_create("caladan3_field_global_lock");
        if (!tempLock) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unable to create global lock\n")));
            return BCM_E_RESOURCE;
        }
        /* Claim the global lock before exposing it */
        FIELD_EVERB((BSL_META("claim global lock\n")));
        if (sal_mutex_take(tempLock, sal_mutex_FOREVER)) {
            /* something went wrong claiming the lock */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unable to claim global lock in create\n")));
            /* free the working lock even though it's defective */
            sal_mutex_destroy(tempLock);
            return BCM_E_INTERNAL;
        }
        /* Set the global lock to the one we now own, if it's still none */
        if (!_sbx_caladan3_field_glob_lock) {
            FIELD_EVERB((BSL_META("set global lock\n")));
            _sbx_caladan3_field_glob_lock = tempLock;
        }
        /* Let everybody else catch up */
        FIELD_EVERB((BSL_META("give up timeslice\n")));
        sal_thread_yield();
        /* Check for race condition and compensate if needed */
        if (_sbx_caladan3_field_glob_lock != tempLock) {
            /* somebody came along during the race hole; yield to them */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "detected race condition on field init for"
                                   " unit %d: trying to compensate\n"),
                       unit));
            /*
             *  We encountered a race condition during the hole beteen testing
             *  whether there was a global lock and setting it.  This hole
             *  seems to be impossible to avoid using C (and may be impossible
             *  to avoid in assembly on any platform that does not support a
             *  move-if-destination-is-zero type of instruction).
             *
             *  We compensate for this condition by destroying the losing copy
             *  of the primary lock (ours, since the other thread stomped ours)
             *  and going on.  We'll pick up the winning copy later (before
             *  filling in the unit data), so there should not be an error here
             *  (though the diagnostic may be useful and we definitely do need
             *  to free our lock in order to avoid leaking it).
             *
             *  If another thread is already waiting on our lock instead of the
             *  winner, it should receive an error during its next timeslice at
             *  the point of the wait, and that will propagate back to the
             *  caller as BCM_E_INTERNAL (there doesn't seem to be anything
             *  better for it).
             *
             *  This is a Bad Thing, but it is not, in itself, unrecoverable.
             *  We therefore continue without any error indication other than
             *  the diagnostic message (and any thread that was waiting on our
             *  lock can retry and should be okay, since it will also pick up
             *  the new global lock).
             *
             *  Unhappily, the race condition check is itself susceptible to
             *  the same condition for which it checks, so there's still a
             *  possibility of leaks and contention here.  Hopefully nobody's
             *  doing parallelised inits of the same subsystem on true parallel
             *  hardware or preemptive timesharing systems...
             */
            sal_mutex_destroy(tempLock);
        } else { /* if (_sbx_caladan3_field_glob_lock != tempLock) */
            /* no obvious race condition */
            /* Clear the global resources */
            FIELD_EVERB((BSL_META("clear global unit information\n")));
            for (index = 0; index < BCM_MAX_NUM_UNITS; index++) {
                _sbx_caladan3_field[index] = _sbx_caladan3_field_unit_initValue;
            }
            /* release the global lock */
            FIELD_EVERB((BSL_META("release global lock\n")));
            if (sal_mutex_give(tempLock)) {
                /* something went wrong */
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "unable to release global lock"
                                       " in create\n")));
                return BCM_E_INTERNAL;
            }
        } /* if (_sbx_caladan3_field_glob_lock != tempLock) */
    } /* if (!_sbx_caladan3_field_glob_lock) */

    /* Take the global lock */
    FIELD_EVERB((BSL_META("take global lock\n")));
    if (sal_mutex_take(_sbx_caladan3_field_glob_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unable to take global lock\n")));
        return BCM_E_INTERNAL;
    }

    /* Make sure the unit is not already inited */
    if (_sbx_caladan3_field[unit].lock) {
        /* this unit is already initialised; detach it first */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "unit %d already inited; detaching first\n"),
                     unit));
        /* check validity of unit and claim unit lock */
        result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
        if (BCM_E_NONE == result) {
            result = _bcm_caladan3_field_detach(unit, lock, microcode, unitData);
        }
        if (BCM_E_NONE != result) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "implied detach unit %d failed: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
        }
    }

    if (BCM_E_NONE == result) {
        /* Create the unit lock */
        FIELD_EVERB((BSL_META("create unit %d lock\n"), unit));
        tempLock = sal_mutex_create("caladan3_field_unit_lock");

        /* Initialise the unit */
        _sbx_caladan3_field[unit].microcode = SOC_SBX_CONTROL(unit)->ucodetype;
        switch (_sbx_caladan3_field[unit].microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_init(unit,
                                                &(_sbx_caladan3_field[unit].data));
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }

        if (BCM_E_NONE == result) {
            /* init success; attach the lock to the unit */
            FIELD_EVERB((BSL_META("set unit %d lock\n"), unit));
            _sbx_caladan3_field[unit].lock = tempLock;
        } else {
            /* init fail; destroy the lock & make sure unit info clear */
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "init unit %d failed: %d (%s)\n"),
                       unit,
                       result,
                       _SHR_ERRMSG(result)));
            FIELD_EVERB((BSL_META("clear and release unit %d resources\n"),
                         unit));
            _sbx_caladan3_field[unit].lock = NULL;
            _sbx_caladan3_field[unit].data = NULL;
            _sbx_caladan3_field[unit].microcode = SOC_SBX_UCODE_TYPE_NONE;
            sal_mutex_destroy(tempLock);
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    if (BCM_E_NONE == result) {
        bcm_caladan3_wb_field_state_init(unit);
    }
#endif

    /* Release the global lock */
    FIELD_EVERB((BSL_META("release global lock\n")));
    if (sal_mutex_give(_sbx_caladan3_field_glob_lock)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unable to release global lock\n")));
        return BCM_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d) return %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_detach
 *   Purpose
 *      Shut down the field APIs.
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_detach(int unit)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d)\n"),
                 unit));

    /* check the global lock */
    if (!_sbx_caladan3_field_glob_lock) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "no CALADAN3 field units initialised\n")));
        return BCM_E_INIT;
    }

    /* claim the global lock */
    FIELD_EVERB((BSL_META("claim global lock\n")));
    if (sal_mutex_take(_sbx_caladan3_field_glob_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "unable to take global lock\n")));
        return BCM_E_INTERNAL;
    }

    /* check validity of unit and claim unit lock */
    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        result = _bcm_caladan3_field_detach(unit, lock, microcode, unitData);
    }

    /* release globals lock */
    FIELD_EVERB((BSL_META("release global lock\n")));
    if (sal_mutex_give(_sbx_caladan3_field_glob_lock)) {
        /* failed to release the lock */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "failed to release global lock\n")));
        return BCM_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d) = %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_create
 *   Purpose
 *      Create a new group that has the specified qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (out) _field_group_index *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 */
int
bcm_caladan3_field_group_create(int unit,
                              bcm_field_qset_t qset,
                              int pri,
                              bcm_field_group_t *group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,*,%d,*)\n"),
                 unit, pri));

    /* check group pointer */
    if (!group) {
        /* NULL pointer for group output */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for group\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_create(unitData,
                                                        qset,
                                                        pri,
                                                        group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,*,%d,&(%08X)) = %d (%s)\n"),
                 unit,
                 pri,
                 *group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_create_id
 *   Purpose
 *      Create a new group with the specified ID that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 */
int
bcm_caladan3_field_group_create_id(int unit,
                                 bcm_field_qset_t qset,
                                 int pri,
                                 bcm_field_group_t group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,*,%d,%08X)\n"),
                 unit, pri, group));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_create_id(unitData,
                                                           qset,
                                                           pri,
                                                           group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,*,%d,%08X) = %d (%s)\n"),
                 unit,
                 pri,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 *   Function
 *      bcm_caladan3_field_group_install
 *   Purpose
 *      Insert all of a group's entries from the hardware.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This inserts and updates all of the groups entries to the hardware as
 *      appropriate.  No error is asserted for entries already in hardware,
 *      even if the entire group is already in hardware.
 */
int
bcm_caladan3_field_group_install(int unit,
                               bcm_field_group_t group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit, group));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_install(unitData, group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_remove
 *   Purpose
 *      Remove all of a group's entries from the hardware, but do not remove
 *      the entries from the software table.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This removes the group's entries from the hardware, marking them so,
 *      and commits the changes to the hardware.
 */
int
bcm_caladan3_field_group_remove(int unit,
                              bcm_field_group_t group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit, group));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_remove(unitData, group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_flush
 *   Purpose
 *      Remove all of a group's entries from the hardware, remove the group
 *      from the hardware, remove the group's entries from the software, and
 *      remove the group from the software.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This will destroy the field group and all its entries.  No mention is
 *      made that it affects ranges, so they aren't destroyed.  This also
 *      destroys the field group and its entries in hardware.
 */
int
bcm_caladan3_field_group_flush(int unit,
                             bcm_field_group_t group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit, group));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            /* coverity[stack_use_overflow] */
            result = bcm_caladan3_g3p1_field_group_flush(unitData, group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_set
 *   Purpose
 *      This changes the group's qualifier set so it is the specified set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) bcm_field_qset_t qset = new qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If there are any entries, all of them must be representable using the
 *      new qualifier set (if not, this fails), plus the new qualifier set can
 *      not change the required pattern type or stage (it will also fail in
 *      these cases).
 *      Updates are always permitted if there are no entries present.
 */
int
bcm_caladan3_field_group_set(int unit,
                           bcm_field_group_t group,
                           bcm_field_qset_t qset)
{
    /* Not currently supported for Caladan3 */
    return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_caladan3_field_group_get
 *   Purpose
 *      Gets the group's qualifier set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_qset_t *qset = where to put the current qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_group_get(int unit,
                           bcm_field_group_t group,
                           bcm_field_qset_t *qset)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*)\n"),
                 unit, group));

    if (!qset) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for qset\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_get(unitData, group, qset);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*) = %d (%s)\n"),
                 unit,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_destroy
 *   Purpose
 *      Destroys a group.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      There must be no entries in this group when calling this function.
 */
int
bcm_caladan3_field_group_destroy(int unit,
                               bcm_field_group_t group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit, group));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_destroy(unitData, group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_status_get
 *   Purpose
 *      Gets the group's status.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_group_status_t *status = where to put the status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_group_status_get(int unit,
                                  bcm_field_group_t group,
                                  bcm_field_group_status_t *status)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*)\n"),
                 unit, group));

    if (!status) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for status\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_status_get(unitData,
                                                            group,
                                                            status);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*) = %d (%s)\n"),
                 unit,
                 group,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_range_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_caladan3_field_range_create(int unit,
                              bcm_field_range_t *range,
                              uint32 flags,
                              bcm_l4_port_t min,
                              bcm_l4_port_t max)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,*,%08X,%04X,%04X)\n"),
                 unit,
                 flags,
                 min,
                 max));

    if (!range) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for range\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_range_create(unitData,
                                                        range,
                                                        flags,
                                                        min,
                                                        max);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,&(%08X),%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 *range,
                 flags,
                 min,
                 max,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_range_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_caladan3_field_range_create_id(int unit,
                                 bcm_field_range_t range,
                                 uint32 flags,
                                 bcm_l4_port_t min,
                                 bcm_l4_port_t max)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%04X,%04X)\n"),
                 unit,
                 range,
                 flags,
                 min,
                 max));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_range_create_id(unitData,
                                                           range,
                                                           flags,
                                                           min,
                                                           max);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 range,
                 flags,
                 min,
                 max,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_range_get
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 *flags = where to put the flags for the range
 *      (in) bcm_l4_port_t *min = where to put range's low port number
 *      (in) bcm_l4_port_t *max = where to put range's high port number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_caladan3_field_range_get(int unit,
                           bcm_field_range_t range,
                           uint32 *flags,
                           bcm_l4_port_t *min,
                           bcm_l4_port_t *max)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*,*,*)\n"),
                 unit,
                 range));

    if ((!flags) || (!min) || (!max)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_range_get(unitData,
                                                     range,
                                                     flags,
                                                     min,
                                                     max);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%08X),&(%04X),&(%04X)) = %d (%s)\n"),
                 unit,
                 range,
                 *flags,
                 *min,
                 *max,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_range_destroy
 *   Purpose
 *      Destroy a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_caladan3_field_range_destroy(int unit,
                               bcm_field_range_t range)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit, range));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_range_destroy(unitData, range);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 range,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_create
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = the group ID to use
 *      (out) bcm_field_entry_t *entry = where to put the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function can
 *      only insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 */
int
bcm_caladan3_field_entry_create(int unit,
                              bcm_field_group_t group,
                              bcm_field_entry_t *entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*)\n"),
                 unit, group));

    if (!entry) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for entry\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_create(unitData,
                                                        group,
                                                        entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%08X)) = %d (%s)\n"),
                 unit,
                 group,
                 *entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_create_id
 *   Purpose
 *      Create an empty field entry based upon the specified grup
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = the group ID to use
 *      (in) bcm_field_entry_t entry = the entry ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function can
 *      only insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 */
int
bcm_caladan3_field_entry_create_id(int unit,
                                 bcm_field_group_t group,
                                 bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X)\n"),
                 unit, group, entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_create_id(unitData,
                                                           group,
                                                           entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X) = %d (%s)\n"),
                 unit,
                 group,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 *   Function
 *      bcm_caladan3_field_entry_destroy
 *   Purpose
 *      Destroy a field entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not remove any associated entries from the hardware.[!?]
 */
int
bcm_caladan3_field_entry_destroy(int unit,
                               bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            /* coverity[stack_use_overflow] */
            result = bcm_caladan3_g3p1_field_entry_destroy(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_destroy_all
 *   Purpose
 *      Destroy all field entries
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not remove any associated entries from the hardware.[!?]
 */
int
bcm_caladan3_field_entry_destroy_all(int unit)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d)\n"),
                 unit));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            /* coverity[stack_use_overflow] */
            result = bcm_caladan3_g3p1_field_entry_destroy_all(unitData);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d) = %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_copy
 *   Purpose
 *      Copy an existing field entry to another one
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (out) bcm_field_entry_t *dst_entry = where to put the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function will
 *      insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 *
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the original entry's priority.  If the
 *      original entry is participating in counter sharing, so is the copy; if
 *      not, neither is the copy (but if the original had a counter allocated,
 *      so will the copy, though it will be a *different* counter).
 */
int
bcm_caladan3_field_entry_copy(int unit,
                            bcm_field_entry_t src_entry,
                            bcm_field_entry_t *dst_entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*)\n"),
                 unit, src_entry));

    if (!dst_entry) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for destination entry\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_copy(unitData,
                                                      src_entry,
                                                      dst_entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%08X)) = %d (%s)\n"),
                 unit,
                 src_entry,
                 *dst_entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_copy_id
 *   Purpose
 *      Copy an existing field entry to a specific one
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t src_entry = the original entry ID
 *      (in) bcm_field_entry_t dst_entry = the copy entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      'The field entry identifier is the priority of the entry in the group.'
 *      Yeah, right.  Where's that deed to the oceanfront property within easy
 *      walking distance of Des Moines?
 *
 *      Actually, the field entry identifier has nothing whatsoever to do with
 *      the priority, which is set separately.  Annoyingly, this function will
 *      insert the entry into the group based upon a priority setting of
 *      BCM_FIELD_ENTRY_PRIO_DEFAULT, and it will be moved later if the user
 *      actually bothers to set the priority.
 *
 *      This can only copy the entry within its group, and the copy will be
 *      inserted as the last entry of the priority.  If the original entry is
 *      participating in counter sharing, so is the copy; if not, neither is
 *      the copy (but if the original had a counter allocated, so will the
 *      copy, though it will be a different counter).
 */
int
bcm_caladan3_field_entry_copy_id(int unit,
                               bcm_field_entry_t src_entry,
                               bcm_field_entry_t dst_entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X)\n"),
                 unit, src_entry, dst_entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_copy_id(unitData,
                                                         src_entry,
                                                         dst_entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X) = %d (%s)\n"),
                 unit,
                 src_entry,
                 dst_entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_install
 *   Purpose
 *      Install a field entry to the hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This causes an error if the entry is already in hardware.
 *      This will commit the appropriate database to the hardware.
 */
int
bcm_caladan3_field_entry_install(int unit,
                               bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_install(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_reinstall
 *   Purpose
 *      Reinstall a field entry to the hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Apprarently, despite the API doc indicating this can only be used to
 *      refresh an entry that is already in hardware, the regression tests
 *      require that this work to install an entry that is not in hardware.
 */
int
bcm_caladan3_field_entry_reinstall(int unit,
                                 bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_reinstall(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_remove
 *   Purpose
 *      Remove a field entry from the hardware
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The API doc indicates that this can only be used against an entry that
 *      is already in hardware, but the regression tests require that it work
 *      even if the entry isn't in hardware.
 */
int
bcm_caladan3_field_entry_remove(int unit,
                              bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_remove(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_prio_get
 *   Purpose
 *      Get the priority of a specific entry (within its group)
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) int *prio = where to put the entry's priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 *      Priority is signed; nonnegative numbers are priority order; negative
 *      numbers have special meanings.
 *      Overall sort is:
 *          highest >= numbered >= dontcare >= lowest
 */
int
bcm_caladan3_field_entry_prio_get(int unit,
                                bcm_field_entry_t entry,
                                int *prio)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*)\n"),
                 unit,
                 entry));

    if (!prio) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for priority\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_prio_get(unitData,
                                                          entry,
                                                          prio);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%d)) = %d (%s)\n"),
                 unit,
                 entry,
                 *prio,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_prio_set
 *   Purpose
 *      Set the priority of a specific entry (within its group)
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int prio = the entry's new priority
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The field entry identifier is NOT the priority of the entry in the
 *      group on in the system.
 *      Priority is signed; nonnegative numbers are priority order; negative
 *      numbers have special meanings. Overall sort is:
 *          highest >= numbered >= dontcare >= lowest
 *
 *      At this time the priority of an entry must be changed after it is
 *      created and before any of the qualifiers or actions are added. This
 *      restriction will be lifted at some point once additional soc layer
 *      support is ready.
 */
int
bcm_caladan3_field_entry_prio_set(int unit,
                                bcm_field_entry_t entry,
                                int prio)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d)\n"),
                 unit, entry, prio));

    /* check parameters */
    if ( ((prio >= _SBX_CALADAN3_FIELD_ENTRY_PRIO_HIGHEST) &&
         (prio != BCM_FIELD_ENTRY_PRIO_HIGHEST)) ||
         (prio < 0) ) {
        /* invalid priority */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "invalid priority (%d)\n"),
                   prio));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_prio_set(unitData,
                                                          entry,
                                                          prio);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d) = %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_clear
 *   Purpose
 *      Clear all qualifiers for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_qualify_clear(int unit,
                               bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_clear(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_InPort
 *   Purpose
 *      Set allowed ingress port for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = allowed port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      All-or-nothing mask only on SBX.
 *      Supports GPORTs of various types and will map back to phys port.
 */
int
bcm_caladan3_field_qualify_InPort(int unit,
                                bcm_field_entry_t entry,
                                bcm_port_t data,
                                bcm_port_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_InPort(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_OutPort
 *   Purpose
 *      Set allowed egress port for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = allowed port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      All-or-nothing mask only on SBX.
 *      Supports GPORTs of various types and will map back to phys port.
 */
int
bcm_caladan3_field_qualify_OutPort(int unit,
                                 bcm_field_entry_t entry,
                                 bcm_port_t data,
                                 bcm_port_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OutPort(unitData,
                                                           entry,
                                                           data,
                                                           mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_InPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_InPort_get(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_port_t *data,
                                    bcm_port_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*,*)\n"),
                 unit, entry));

    /* make sure return pointers are valid */
    if ((!data) || (!mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_InPort_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%08X),&(%08X)) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_OutPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOutPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_OutPort_get(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_port_t *data,
                                     bcm_port_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,*,*)\n"),
                 unit, entry));

    /* make sure return pointers are valid */
    if ((!data) || (!mask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OutPort_get(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%08X),&(%08X)) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_OuterVlanId
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VID (12 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_OuterVlanId(int unit,
                                     bcm_field_entry_t entry,
                                     bcm_vlan_t data,
                                     bcm_vlan_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%03X,%03X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlanId(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%03X,%03X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 *   Function
 *      bcm_caladan3_field_qualify_OuterVlanPri
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which Pri (3 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_OuterVlanPri(int unit,
                                      bcm_field_entry_t entry,
                                      uint8 data,
                                      uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlanPri(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_caladan3_field_qualify_OuterVlanCfi
 * Purpose:
 *       Set match criteria for bcmFieildQualifyOuterVlanCfi
 *                       qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (IN) Qualifier match data.
 *      mask - (IN) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_OuterVlanCfi(
    int unit,
    bcm_field_entry_t entry,
    uint8 data,
    uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlanCfi(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}


/*
 * Function:
 *      bcm_caladan3_field_qualify_OuterVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_OuterVlanId_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t *data,
    bcm_vlan_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%03X,%03X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlanId_get(unitData,
                                                               entry,
                                                               data,
                                                               mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%03X,%03X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_caladan3_field_qualify_OuterVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_OuterVlanPri_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlanPri_get(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_OuterVlanCfi_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanCfi_get
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_OuterVlanCfi_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlanCfi_get(unitData,
                                                                entry,
                                                                data,
                                                                mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%01X,%01X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_OuterVlan
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VLAN tag (16 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be all zeroes or ones for supported subfields.  While we
 *      accept a nonzero mask for CFI, it is ignored with a warning.
 *
 *      BCM API docs neglect to metion that this function should apply to the
 *      entire tag, rather than just the VID.
 */
int
bcm_caladan3_field_qualify_OuterVlan(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_vlan_t data,
                                   bcm_vlan_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlan(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_EtherType
 *   Purpose
 *      Set expected ethernet type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_EtherType(int unit,
                                   bcm_field_entry_t entry,
                                   uint16 data,
                                   uint16 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_EtherType(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_IpProtocol
 *   Purpose
 *      Set expected IPv4 protocol type type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_IpProtocol(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_IpProtocol(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_SrcIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_field_qualify_SrcIp(int unit,
                               bcm_field_entry_t entry,
                               bcm_ip_t data,
                               bcm_ip_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_SrcIp(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_DstIp
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_caladan3_field_qualify_DstIp(int unit,
                               bcm_field_entry_t entry,
                               bcm_ip_t data,
                               bcm_ip_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DstIp(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

int
bcm_caladan3_field_qualify_L4SrcPort(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_l4_port_t data,
                                   bcm_l4_port_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_L4SrcPort(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

int
bcm_caladan3_field_qualify_L4DstPort(int unit,
                                   bcm_field_entry_t entry,
                                   bcm_l4_port_t data,
                                   bcm_l4_port_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_L4DstPort(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_L4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_L4SrcPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_l4_port_t *data,
    bcm_l4_port_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_L4SrcPort_get(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_L4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_L4DstPort_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_l4_port_t *data,
    bcm_l4_port_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_L4DstPort_get(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_DSCP
 *   Purpose
 *      Set expected IPv4 DSCP for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which DSCP
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_qualify_DSCP(int unit,
                              bcm_field_entry_t entry,
                              uint8 data,
                              uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DSCP(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_Tos
 *   Purpose
 *      Set expected IPv4 ToS for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ToS
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      API internals say this is same as DSCP above.
 */
int
bcm_caladan3_field_qualify_Tos(int unit,
                             bcm_field_entry_t entry,
                             uint8 data,
                             uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            
            result = bcm_caladan3_g3p1_field_qualify_DSCP(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_TcpControl
 *   Purpose
 *      Set expected TCP control flags for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which TCP control bits
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Also implies TCP as protocol.
 */
int
bcm_caladan3_field_qualify_TcpControl(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_TcpControl(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_RangeCheck
 *   Purpose
 *      Set expected TCP/UDP port range for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_range_t range = which ethertype
 *      (in) int invert = whether the range match is to be inverted
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The range that is specified is read only during this call; it will not
 *      be read later if that range changes; another call to this function will
 *      be required should the range change and the update need to apply.
 *      The invert flag is not supported.
 *      This can't use the helper functions because it is setting a more
 *      complex set of fields under a more complex set of conditions.
 */
int
bcm_caladan3_field_qualify_RangeCheck(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_field_range_t range,
                                    int invert)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%s)\n"),
                 unit,
                 entry,
                 range,
                 invert?"TRUE":"FALSE"));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_RangeCheck(unitData,
                                                              entry,
                                                              range,
                                                              invert);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%s) = %d (%s)\n"),
                 unit,
                 entry,
                 range,
                 invert?"TRUE":"FALSE",
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_SrcMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_SrcMac(int unit,
                                bcm_field_entry_t entry,
                                bcm_mac_t data,
                                bcm_mac_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_SrcMac(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_DstMac
 *   Purpose
 *      Set expected destination MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which destination MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the destination MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_DstMac(int unit,
                                bcm_field_entry_t entry,
                                bcm_mac_t data,
                                bcm_mac_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DstMac(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(data),
                 FIELD_MACA_SHOW(mask),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_Llc
 *   Purpose
 *      Set expected LLC header information for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_llc_header_t data = which LLC header information
 *      (in) bcm_field_llc_header_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Masks for data.dsap and data.ssap must be either zero or all ones, and
 *      mask for data.control must be all zeroes, else BCM_E_PARAM, since SBX
 *      doesn't allow other than all-or-nothing for dsap and ssap and doesn't
 *      appear to provide control field.
 */
int
bcm_caladan3_field_qualify_Llc(int unit,
                             bcm_field_entry_t entry,
                             bcm_field_llc_header_t data,
                             bcm_field_llc_header_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_LLC_F0RMAT "," FIELD_LLC_F0RMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_LLC_SHOW(data),
                 FIELD_LLC_SHOW(mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_LLC_F0RMAT "," FIELD_LLC_F0RMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_LLC_SHOW(data),
                 FIELD_LLC_SHOW(mask),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_action_add
 *   Purpose
 *      Add a specific action to a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to add
 *      (in) uint32 param0 = action parameter 0 (some actions)
 *      (in) uint32 param1 = action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_field_action_add(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_action_t action,
                            uint32 param0,
                            uint32 param1)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%08X,%08X)\n"),
                 unit,
                 entry,
                 action,
                 param0,
                 param1));

    if ((0 > action) || (bcmFieldActionCount <= action)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "invalid action %d\n"),
                   action));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_action_add(unitData,
                                                      entry,
                                                      action,
                                                      param0,
                                                      param1);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d(%s),%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 action,
                 _sbx_caladan3_field_action_name[action],
                 param0,
                 param1,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_action_get
 *   Purpose
 *      Get a specific action from a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to get
 *      (out) uint32 *param0 = action parameter 0 (some actions)
 *      (out) uint32 *param1 = action parameter 1 (some actions)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_field_action_get(int unit,
                            bcm_field_entry_t entry,
                            bcm_field_action_t action,
                            uint32 *param0,
                            uint32 *param1)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,*,*)\n"),
                 unit,
                 entry,
                 action));

    if ((0 > action) || (bcmFieldActionCount <= action)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "invalid action %d\n"),
                   action));
        return BCM_E_PARAM;
    }
    if ((!param0) || (!param1)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for param0 or param1\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_action_get(unitData,
                                                      entry,
                                                      action,
                                                      param0,
                                                      param1);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d(%s),&(%08X),&(%08X)) = %d (%s)\n"),
                 unit,
                 entry,
                 action,
                 _sbx_caladan3_field_action_name[action],
                 *param0,
                 *param1,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_action_remove
 *   Purpose
 *      Remove a specific action from a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_action_t action = the action to remove
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_field_action_remove(int unit,
                               bcm_field_entry_t entry,
                               bcm_field_action_t action)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d)\n"),
                 unit,
                 entry,
                 action));

    if ((0 > action) || (bcmFieldActionCount <= action)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "invalid action %d\n"),
                   action));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_action_remove(unitData,
                                                         entry,
                                                         action);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d(%s)) = %d (%s)\n"),
                 unit,
                 entry,
                 action,
                 _sbx_caladan3_field_action_name[action],
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_action_remove_all
 *   Purpose
 *      Remove all actions from a specific entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      No hardware changes; only software state.
 */
int
bcm_caladan3_field_action_remove_all(int unit,
                                   bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_action_remove_all(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_field_counter_create
 *   Purpose
 *      'Create' a counter for the specified entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
_bcm_caladan3_field_counter_create(int unit,
                                bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_counter_create(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_field_counter_destroy
 *   Purpose
 *      Remove the entry's counter
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If an entry is not sharing a counter, this merely turns off the counter
 *      for that entry.  If the entry is sharing a counter, it removes that
 *      entry from the sharing list and then disables the counter for that
 *      entry (so the end result is the specified entry has no counter but any
 *      other entries that shared with it are left alone).
 */
int
_bcm_caladan3_field_counter_destroy(int unit,
                                 bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_counter_destroy(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_field_counter_set
 *   Purpose
 *      Set the specified counter to a value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint64 val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
_bcm_caladan3_field_counter_set(int unit,
                             bcm_field_entry_t entry,
                             int counter_num,
                             uint64 val)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%08X%08X)\n"),
                 unit,
                 entry,
                 counter_num,
                 COMPILER_64_HI(val), COMPILER_64_LO(val)));

    if ((0 > counter_num) || (1 < counter_num)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "counter number %d is not supported\n"),
                   counter_num));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_counter_set(unitData,
                                                       entry,
                                                       counter_num,
                                                       val);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%08X%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 counter_num,
                 COMPILER_64_HI(val), COMPILER_64_LO(val),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_field_counter_set32
 *   Purpose
 *      Set the specified counter to a 32 bit value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (in) uint32 val = new value for counter
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
_bcm_caladan3_field_counter_set32(int unit,
                               bcm_field_entry_t entry,
                               int counter_num,
                               uint32 val)
{ uint64 val64;
    /* just let the compiler coerce the number and call the 64b version */
    COMPILER_64_SET(val64, 0, val);
    return _bcm_caladan3_field_counter_set(unit, entry, counter_num, val64);
}

/*
 *   Function
 *      _bcm_caladan3_field_counter_get
 *   Purpose
 *      Get the specified counter's value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (out) uint64 *val = where to put the counter's value
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Overflow when accumulating values is handled by keeping the maximum
 *      possible value.
 */
int
_bcm_caladan3_field_counter_get(int unit,
                             bcm_field_entry_t entry,
                             int counter_num,
                             uint64 *valp)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,*)\n"),
                 unit,
                 entry,
                 counter_num));

    if (!valp) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer for value\n")));
        return BCM_E_PARAM;
    }
    if ((0 > counter_num) || (1 < counter_num)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "counter number %d is not supported\n"),
                   counter_num));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_counter_get(unitData,
                                                       entry,
                                                       counter_num,
                                                       valp);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,&(%08X%08X)) = %d (%s)\n"),
                 unit,
                 entry,
                 counter_num,
                 COMPILER_64_HI(*valp), COMPILER_64_LO(*valp),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      _bcm_caladan3_field_counter_get32
 *   Purpose
 *      Get the specified counter's low 32 bits value
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int counter_num = which counter (perhaps frame or byte?)
 *      (out) uint32 *val = where to put the counter's value
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Overflow when accumulating values is handled by keeping the maximum
 *      possible value.
 */
int
_bcm_caladan3_field_counter_get32(int unit,
                               bcm_field_entry_t entry,
                               int counter_num,
                               uint32 *valp)
{
    int                     result;             /* result for caller */
    uint64                  temp = COMPILER_64_INIT(0,0); /* accumulated frames */

    /* get the function above to do the work */
    result = _bcm_caladan3_field_counter_get(unit, entry, counter_num, &temp);

    /* now strip the entry down to what we can return from here */
    if (BCM_E_NONE == result) {
        if (COMPILER_64_HI(temp) == 0) {
            /* the amount to return fits */
            *valp = COMPILER_64_LO(temp);
        } else { /* if (temp <= 0xFFFFFFFFul) */
            /* the amount to return does not fit */
            LOG_WARN(BSL_LS_BCM_FP,
                     (BSL_META_U(unit,
                                 "returned value saturated 32b number\n")));
            *valp = 0xFFFFFFFF;
        } /* if (temp <= 0xFFFFFFFFul) */
    } /* if (BCM_E_NONE == result) */

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,&(%08X)) = %d (%s)\n"),
                 unit,
                 entry,
                 counter_num,
                 *valp,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_policer_attach
 *   Purpose
 *      Attach a policer to a specified entry, at the given heirarchical level
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *      (in) bcm_policer_t policer = which policer to attach to the entry
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes policer 0 = no policer and does not allow it to be set.
 *      Policers must be managed by caller.
 */
int
bcm_caladan3_field_entry_policer_attach(int unit,
                                      bcm_field_entry_t entry_id,
                                      int level,
                                      bcm_policer_t policer_id)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%08X)\n"),
                 unit,
                 entry_id,
                 level,
                 policer_id));

    if (!policer_id) {
        /* can't set policer zero (use clear instead) */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "can't set policer 0 (no policer)\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_policer_attach(unitData,
                                                                entry_id,
                                                                level,
                                                                policer_id);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,%08X) = %d (%s)\n"),
                 unit,
                 entry_id,
                 level,
                 policer_id,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_policer_detach
 *   Purpose
 *      Remove the policer used by the specified entry at the given level
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) int level = level (for heirarchical policing)
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes policer 0 = no policer and calls that 'empty'.
 */
int
bcm_caladan3_field_entry_policer_detach(int unit,
                                      bcm_field_entry_t entry_id,
                                      int level)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d)\n"),
                 unit,
                 entry_id,
                 level));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_policer_detach(unitData,
                                                                entry_id,
                                                                level);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d) = %d (%s)\n"),
                 unit,
                 entry_id,
                 level,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_field_entry_policer_detach_all
 *   Purpose
 *      Remove all policers used by the specified entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Does not return an error if a level has no policer.
 *      Policers must be managed by caller.
 */
int
bcm_caladan3_field_entry_policer_detach_all(int unit,
                                          bcm_field_entry_t entry_id)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X)\n"),
                 unit,
                 entry_id));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_policer_detach_all(unitData,
                                                                    entry_id);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X) = %d (%s)\n"),
                 unit,
                 entry_id,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_policer_get
 *   Purpose
 *      Get the policer used by the specified entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (out) bcm_policer_t *policer = where to put the policer ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes policer 0 = no policer and calls that 'empty'.
 *      Policers must be managed by caller.
 */
int
bcm_caladan3_field_entry_policer_get(int unit,
                                   bcm_field_entry_t entry_id,
                                   int level,
                                   bcm_policer_t *policer_id)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,*)\n"),
                 unit,
                 entry_id,
                 level));

    if (!policer_id) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "NULL pointer to policer ID\n")));
        return BCM_E_PARAM;
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_policer_get(unitData,
                                                             entry_id,
                                                             level,
                                                             policer_id);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%d,&(%08X)) = %d (%s)\n"),
                 unit,
                 entry_id,
                 level,
                 *policer_id,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_show
 *   Purpose
 *      Dump all field information for the unit
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_show(int unit,
                      const char *pfx)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    FIELD_EVERB((BSL_META("(%d,*)\n"), unit));

    if (!pfx) {
        pfx = "";
    }

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_show(unitData, pfx);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    FIELD_EVERB((BSL_META("(%d,\"%s\") = %d (%s)\n"),
                unit,
                pfx,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_entry_dump
 *   Purpose
 *      Dump information about the specified entry to debug output
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_entry_dump(int unit,
                            bcm_field_entry_t entry)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    FIELD_EVERB((BSL_META("(%d,%08X)\n"), unit, entry));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_entry_dump(unitData, entry);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    FIELD_EVERB((BSL_META("(%d,%08X) = %d (%s)\n"),
                unit,
                entry,
                result,
                _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_group_dump
 *   Purpose
 *      Dump information about the specified group to debug output
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_caladan3_field_group_dump(int unit,
                            bcm_field_group_t group)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    FIELD_EVERB((BSL_META("(%d,%08X)\n"), unit, group));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_group_dump(unitData, group);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    FIELD_EVERB((BSL_META("(%d,%08X) = %d (%s)\n"),
                unit,
                group,
                result,
                _SHR_ERRMSG(result)));
    return result;
}


int
bcm_caladan3_field_qualify_SrcIp6(int unit,
                                bcm_field_entry_t entry,
                                bcm_ip6_t data,
                                bcm_ip6_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    if (!SOC_IS_SBX_CALADAN3(unit)){
        return BCM_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_SrcIp6(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask),
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_SrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_SrcIp6_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    if (!SOC_IS_SBX_CALADAN3(unit)){
        return BCM_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(*data),
                 FIELD_IPV6A_SHOW(*mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_SrcIp6_get(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(*data),
                 FIELD_IPV6A_SHOW(*mask),
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

int
bcm_caladan3_field_qualify_DstIp6(int unit,
                                bcm_field_entry_t entry,
                                bcm_ip6_t data,
                                bcm_ip6_t mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    if (!SOC_IS_SBX_CALADAN3(unit)){
        return BCM_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DstIp6(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(data),
                 FIELD_IPV6A_SHOW(mask),
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_DstIp6_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    if (!SOC_IS_SBX_CALADAN3(unit)){
        return BCM_E_UNAVAIL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(*data),
                 FIELD_IPV6A_SHOW(*mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DstIp6_get(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_IPV6A_FORMAT "," FIELD_IPV6A_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_IPV6A_SHOW(*data),
                 FIELD_IPV6A_SHOW(*mask),
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_Ip6NextHeader
 *   Purpose
 *      Set expected IPv6 Next Header type type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_Ip6NextHeader(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_Ip6NextHeader(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

int
bcm_caladan3_field_qualify_Ip6NextHeader_get(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 *data,
                                    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_Ip6NextHeader_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 *   Function
 *      bcm_caladan3_field_qualify_Ip6TrafficClass
 *   Purpose
 *      Set expected IPv6 Next Header type type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which ethertype
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_caladan3_field_qualify_Ip6TrafficClass(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 data,
                                    uint8 mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 data,
                 mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 data,
                 mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

int
bcm_caladan3_field_qualify_Ip6TrafficClass_get(int unit,
                                    bcm_field_entry_t entry,
                                    uint8 *data,
                                    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_Ip6TrafficClass_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

int
bcm_caladan3_field_qualify_SrcIp6High(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_ip6_t data,
                                    bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}

int
bcm_caladan3_field_qualify_DstIp6High(int unit,
                                    bcm_field_entry_t entry,
                                    bcm_ip6_t data,
                                    bcm_ip6_t mask)
{
    return BCM_E_UNAVAIL;
}



int
bcm_caladan3_field_stat_set(int unit,
                          int stat_id,
                          bcm_field_stat_t stat,
                          uint64 value)
{
    return _bcm_caladan3_field_counter_set(unit, stat_id, stat, value);
}

int
bcm_caladan3_field_stat_set32(int unit,
                            int stat_id,
                            bcm_field_stat_t stat,
                            uint32 value)
{
    return _bcm_caladan3_field_counter_set32(unit, stat_id, stat, value);
}

int
bcm_caladan3_field_stat_get(int unit,
                          int stat_id,
                          bcm_field_stat_t stat,
                          uint64 *value)
{
    return _bcm_caladan3_field_counter_get(unit, stat_id, stat, value);
}
                            
int
bcm_caladan3_field_stat_get32(int unit,
                            int stat_id,
                            bcm_field_stat_t stat,
                            uint32 *value)
{
    return _bcm_caladan3_field_counter_get32(unit, stat_id, stat, value);
}


int
bcm_caladan3_field_entry_stat_attach(int unit,
                                   bcm_field_entry_t entry,
                                   int stat_id)
{
    if (stat_id != 0) {
        /* stat id not supported */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "Stat Id Not Supported\n")));
        return BCM_E_PARAM; 
    }
    return _bcm_caladan3_field_counter_create(unit, entry);
}

int
bcm_caladan3_field_entry_stat_detach(int unit,
                                   bcm_field_entry_t entry,
                                   int stat_id)
{
    if (stat_id != 0) {
        /* stat id not supported */
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "Stat Id Not Supported\n")));
        return BCM_E_PARAM; 
    }
    return _bcm_caladan3_field_counter_destroy(unit, entry);
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_OuterVlan_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyOuterVlan
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_OuterVlan_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_vlan_t *data,
    bcm_vlan_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_OuterVlan_get(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_EtherType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_EtherType_get(
    int unit,
    bcm_field_entry_t entry,
    uint16 *data,
    uint16 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_EtherType_get(unitData,
                                                             entry,
                                                             data,
                                                             mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%04X,%04X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_caladan3_field_qualify_IpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_IpProtocol_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_IpProtocol_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_caladan3_field_qualify_SrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_SrcIp_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip_t *data,
    bcm_ip_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_SrcIp_get(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_DstIp_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip_t *data,
    bcm_ip_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DstIp_get(unitData,
                                                         entry,
                                                         data,
                                                         mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_DSCP_get(int unit,
                              bcm_field_entry_t entry,
                              uint8 *data,
                              uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DSCP_get(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_Tos_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTos
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_Tos_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DSCP_get(unitData,
                                                        entry,
                                                        data,
                                                        mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_TcpControl_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyTcpControl
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_TcpControl_get(
    int unit,
    bcm_field_entry_t entry,
    uint8 *data,
    uint8 *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X)\n"),
                 unit,
                 entry,
                 *data,
                 *mask));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_TcpControl_get(unitData,
                                                              entry,
                                                              data,
                                                              mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%02X,%02X) = %d (%s)\n"),
                 unit,
                 entry,
                 *data,
                 *mask,
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}


/*
 * Function:
 *      bcm_caladan3_field_qualify_RangeCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyRangeCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      max_count - (IN) Max entries to fill.
 *      range - (OUT) Range checkers array.
 *      invert - (OUT) Range checkers invert array.
 *      count - (OUT) Number of filled range checkers.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_RangeCheck_get(
    int unit,
    bcm_field_entry_t entry,
    int max_count,
    bcm_field_range_t *range,
    int *invert,
    int *count)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%08X,%s,%08X)\n"),
                 unit,
                 entry,
                 max_count,
                 *range,
                 *invert?"TRUE":"FALSE",
                 *count));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_RangeCheck_get(unitData,
                                                              entry,
                                                              max_count,
                                                              range,
                                                              invert,
                                                              count);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X,%08X,%s) = %d (%s)\n"),
                 unit,
                 entry,
                 *range,
                 *invert?"TRUE":"FALSE",
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_SrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_SrcIp6High_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_DstIp6High_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_ip6_t *data,
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_SrcMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_SrcMac_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_mac_t *data,
    bcm_mac_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(*data),
                 FIELD_MACA_SHOW(*mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_SrcMac_get(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(*data),
                 FIELD_MACA_SHOW(*mask),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DstMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_DstMac_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_mac_t *data,
    bcm_mac_t *mask)
{
    _CALADAN3_FIELD_COMMON_LOCALS;

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ")\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(*data),
                 FIELD_MACA_SHOW(*mask)));

    result = _bcm_caladan3_field_unit_intro(unit, &lock, &microcode, &unitData);
    if (BCM_E_NONE == result) {
        switch (microcode) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            result = bcm_caladan3_g3p1_field_qualify_DstMac_get(unitData,
                                                          entry,
                                                          data,
                                                          mask);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "unsupported microcode on unit %d\n"),
                       unit));
            result = _SBX_CALADAN3_FIELD_UNKNOWN_MICROCODE_ERR;
        }
    }
    result = _bcm_caladan3_field_unit_outro(result, unit, lock);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "(%d,%08X," FIELD_MACA_FORMAT "," FIELD_MACA_FORMAT
                             ") = %d (%s)\n"),
                 unit,
                 entry,
                 FIELD_MACA_SHOW(*data),
                 FIELD_MACA_SHOW(*mask),
                 result,
                 _SHR_ERRMSG(result)));
    return result;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_Llc_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyLlc
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_Llc_get(
    int unit,
    bcm_field_entry_t entry,
    bcm_field_llc_header_t *data,
    bcm_field_llc_header_t *mask)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_SrcIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_SrcIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DstIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_caladan3_field_qualify_DstIp6Low(int unit, bcm_field_entry_t entry,
                                 bcm_ip6_t data, bcm_ip6_t mask)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_SrcIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_caladan3_field_qualify_SrcIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}

/*
 * Function:
 *      bcm_caladan3_field_qualify_DstIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int 
bcm_caladan3_field_qualify_DstIp6Low_get(
    int unit, 
    bcm_field_entry_t entry, 
    bcm_ip6_t *data, 
    bcm_ip6_t *mask)
{
    return BCM_E_UNAVAIL; 
}


