/*
 * $Id: vswitch.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Generic Virtual Switching
 */



#include <shared/bsl.h>

#include <soc/sbx/sbx_drv.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/vswitch.h>
#include <bcm_int/sbx/caladan3/wb_db_vswitch.h>

#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/caladan3/ocm.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/vlan.h>

/*
 *  Debugging output...
 */
#define VSWITCH_EXCESS_VERBOSITY 0
#if VSWITCH_EXCESS_VERBOSITY
#define VSWITCH_EVERB(stuff)        LOG_DEBUG(BSL_LS_BCM_VLAN, stuff)
#else /* VLAN_EXCESS_VERBOSITY */
#define VSWITCH_EVERB(stuff)
#endif /* VLAN_EXCESS_VERBOSITY */
#if defined(__GNUC__) && !defined(__PEDANTIC__)
#define VSWITCH_FUNCMSG(string) \
    "%s[%d]%s" string, __FILE__, __LINE__, FUNCTION_NAME()
#define VSWITCH_FUNCMSG1(string) \
    "%s[%d]%s: " string, __FILE__, __LINE__, FUNCTION_NAME()
#else /* defined(__GNUC__) && !defined(__PEDANTIC__) */
#define VSWITCH_FUNCMSG(string)    "%s[%d]" string, __FILE__, __LINE__
#define VSWITCH_FUNCMSG1(string)   "%s[%d]: " string, __FILE__, __LINE__
#endif /* defined(__GNUC__) && !defined(__PEDANTIC__) */

#define VSWITCH_UNIT_VALID_CHECK \
    if (!((unit) >= 0 && ((unit) < (BCM_MAX_NUM_UNITS)))) { \
        LOG_ERROR(BSL_LS_BCM_VLAN, \
                  (BSL_META("unit %d is not valid\n"), \
                   unit)); \
        return BCM_E_UNIT; \
    }
#define VSWITCH_UNIT_INIT_CHECK \
    if ((!(_primary_lock)) || (!(_vswitch_state[unit]))) { \
        LOG_ERROR(BSL_LS_BCM_VLAN, \
                  (BSL_META("unit %d not initialised\n"), \
                   unit)); \
        return BCM_E_INIT; \
    }

#if 0 /* Super Mutex  */
extern void _vlan_lock_take();
extern void _vlan_lock_release();
#define VSWITCH_LOCK_TAKE _vlan_lock_take(unit)
#define VSWITCH_LOCK_RELEASE _vlan_lock_release(unit);
#else
#define VSWITCH_LOCK_TAKE \
    if (sal_mutex_take(_vswitch_state[unit]->lock, sal_mutex_FOREVER)) { \
        /* Cound not obtain unit lock  */ \
        LOG_ERROR(BSL_LS_BCM_VLAN, \
                  (BSL_META("unable to obtain vswitch lock" \
                            " on unit %d\n"),               \
                   unit)); \
        return BCM_E_INTERNAL; \
    }
#define VSWITCH_LOCK_RELEASE \
    if (sal_mutex_give(_vswitch_state[unit]->lock)) { \
        /* Could not release unit lock */ \
        LOG_ERROR(BSL_LS_BCM_VLAN, \
                  (BSL_META("unable to release vswitch lock" \
                            " on unit %d\n"),                \
                   unit)); \
        return BCM_E_INTERNAL; \
    }
#endif


#define LPORT_IDX(gport) \
    ((~(_SHR_GPORT_TYPE_MASK << _SHR_GPORT_TYPE_SHIFT)) & gport)

volatile static sal_mutex_t _primary_lock = NULL;
static _bcm_caladan3_vswitch_state_t *_vswitch_state[BCM_MAX_NUM_UNITS];

#ifdef DEBUG_MACLOG
typedef struct maclog_s {
  
  bcm_mac_t       mac;
  uint32          fn;
  int             flag;
  int             unit;
  int             port;
  int             modid;
  int             vid;
  int             fte;
} maclog_t;

extern sal_mutex_t      _l2_log_lock[BCM_LOCAL_UNITS_MAX];

extern int log_index;
extern maclog_t maclog[];
            

#define L2_LOG_LOCK(unit) sal_mutex_take(_l2_log_lock[unit], sal_mutex_FOREVER)
#define L2_LOG_UNLOCK(unit)  sal_mutex_give(_l2_log_lock[unit])
#endif /* DEBUG_MACLOG */


#ifdef BCM_WARM_BOOT_SUPPORT
_bcm_caladan3_vswitch_state_t *bcm_sbx_caladan3_vswitch_cntl_ptr_get(int unit)
{
    return _vswitch_state[unit];
}

#endif /* BCM_WARM_BOOT_SUPPORT */


#ifdef BCM_CALADAN3_G3P1_SUPPORT

/*
 * Function
 *  _bcm_caladan3_is_vswitch_inuse
 * Purpose
 *    To test if l2 gports exist on the vsi
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t vsi = vsi to check
 *  Returns
 *    BCM_E_EXISTS, if gports do exists
 *    BCM_E_NOT_FOUND, if no gports exist
 */
int 
_bcm_caladan3_is_vswitch_inuse(int unit,
                             bcm_vlan_t vsi) 
{
   if (_vswitch_state[unit]->portList[vsi]) {
       return BCM_E_EXISTS;
   } else {
       return BCM_E_NOT_FOUND;
   }
}

/*
 *  Function
 *    _bcm_caladan3_g3p1_vswitch_create_id
 *  Purpose
 *    Allocate and prepare a specific VSI for use on g3p1
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t defVid = default VID for this unit
 *    bcm_vlan_t *vsi = where to put the created VSI ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise
 *  Notes
 *    Must be called with vswitch lock taken
 *    Limited error checking
 */
int
_bcm_caladan3_g3p1_vswitch_create_id(int unit,
                                 bcm_vlan_t defVid,
                                 bcm_vlan_t vsi)
{
    int igmpSnoop;
    int rv;
    soc_sbx_g3p1_ft_t      p3ft;
    soc_sbx_g3p1_v2e_t     p3vlan2Etc;
    uint32 ftidx = 0;

    /* get IGMP snoop initial state from default VID */
    /* note this is atomic read so not bothering with VLAN lock */
    rv = soc_sbx_g3p1_v2e_get(unit, defVid, &p3vlan2Etc);
    if (BCM_E_NONE == rv) {
        igmpSnoop = (0 != p3vlan2Etc.igmp);
    } else {
        igmpSnoop = FALSE;
    }
    /* otherwise use mostly system defaults for the VSI */
    soc_sbx_g3p1_v2e_t_init(&p3vlan2Etc);
    p3vlan2Etc.dontlearn = FALSE;
    p3vlan2Etc.igmp = igmpSnoop;
    rv = soc_sbx_g3p1_v2e_set(unit, vsi, &p3vlan2Etc);

    

    if (BCM_E_NONE == rv) {
        soc_sbx_g3p1_ft_t_init(&p3ft);
        p3ft.excidx = 0;
        p3ft.oi = vsi;
        p3ft.qid = SBX_MC_QID_BASE;
        p3ft.mc = 1;
        BCM_IF_ERROR_RETURN
           (soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));
        ftidx += vsi;
        rv = soc_sbx_g3p1_ft_set(unit, ftidx, &p3ft);
    }

    return rv;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 *  Function
 *    bcm_caladan3_vswitch_create
 *  Purpose
 *    Allocate and prepare a VSI for use
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t *vsi = where to put the created VSI ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise
 *  Notes
 */
int
bcm_caladan3_vswitch_create(int unit,
                          bcm_vlan_t *vsi)
{
    int    rv = BCM_E_UNAVAIL;

#ifdef BCM_CALADAN3_G3P1_SUPPORT

    uint32     alloc_vsi = ~0;
    bcm_vlan_t defVid;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,*) enter\n"),
                 unit));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if (!vsi) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    /* get default VID; assume 1 if error */
    defVid = 1;
    rv = bcm_caladan3_vlan_default_get(unit, &defVid);

    VSWITCH_LOCK_TAKE;

    rv = _sbx_caladan3_resource_alloc(unit,
                                 SBX_CALADAN3_USR_RES_VSI,
                                 1,
                                 &alloc_vsi,
                                 0);
    if (rv == BCM_E_NONE) {
        *vsi = alloc_vsi & 0xffff;
        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            rv = _bcm_caladan3_g3p1_vswitch_create_id(unit, defVid, *vsi);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            rv = BCM_E_INTERNAL;
            _sbx_caladan3_resource_free(unit,
                                   SBX_CALADAN3_USR_RES_VSI,
                                   1,
                                   &alloc_vsi,
                                   0);
        }
    }

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,&(%04X)) return %d (%s)\n"),
                 unit,
                 *vsi,
                 rv,
                 _SHR_ERRMSG(rv)));

#endif /* BCM_CALADAN3_G3P1_SUPPORT */

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_create_with_id
 *  Purpose
 *    Allocate and prepare a specific VSI for use
 *  Arguments
 *    int unit = unit on which the VSI is to be allocated
 *    bcm_vlan_t *vsi = where to put the created VSI ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise
 *  Notes
 */
int
bcm_caladan3_vswitch_create_with_id(int unit,
                                  bcm_vlan_t vsi)
{
    int    rv = BCM_E_UNAVAIL;

#ifdef BCM_CALADAN3_G3P1_SUPPORT

    uint32 alloc_vsi = (vsi & 0xFFFF); /* stupid signed extension workaround */
    bcm_vlan_t defVid;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X) enter\n"),
                 unit, vsi));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if ((vsi < SBX_DYNAMIC_VSI_BASE(unit))
        || (vsi > SBX_DYNAMIC_VSI_END(unit))
       ) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "invalid VSI %04X for unit %d\n"),
                   vsi,
                   unit));
        return BCM_E_PARAM;
    }

    /* get default VID; assume 1 if error */
    defVid = 1;
    rv = bcm_caladan3_vlan_default_get(unit, &defVid);

    VSWITCH_LOCK_TAKE;

    /* try to allocate the specified VSI */
    rv = _sbx_caladan3_resource_alloc(unit,
                                 SBX_CALADAN3_USR_RES_VSI,
                                 1,
                                 &alloc_vsi,
                                 _SBX_CALADAN3_RES_FLAGS_RESERVE);
    if (BCM_E_BUSY == rv) {
        /* for some reason, we get 'busy' if it already exists */
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "VSI %04X already exists on unit %d\n"),
                   vsi,
                   unit));
        rv = BCM_E_EXISTS;
    }

    if (BCM_E_NONE == rv) {

        switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
        case SOC_SBX_UCODE_TYPE_G3P1:
            rv = _bcm_caladan3_g3p1_vswitch_create_id(unit, defVid, vsi);
            break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
        default:
            SBX_UNKNOWN_UCODE_WARN(unit);
            rv = BCM_E_INTERNAL;
            _sbx_caladan3_resource_free(unit,
                                   SBX_CALADAN3_USR_RES_VSI,
                                   1,
                                   &alloc_vsi,
                                   0);
        }
    }

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X) return %d (%s)\n"),
                 unit,
                 vsi,
                 rv,
                 _SHR_ERRMSG(rv)));

#endif /* BCM_CALADAN3_G3P1_SUPPORT */

    return rv;
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
/*
 *  Function
 *    _bcm_caladan3_g3p1_vswitch_destroy
 *  Purpose
 *    Destroy an existing VSI on g3p1
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t = the VSI to destroy
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 */
int
_bcm_caladan3_g3p1_vswitch_destroy(int unit,
                               bcm_vlan_t vsi)
{
    int rv;
    soc_sbx_g3p1_ft_t      p3ft;
    soc_sbx_g3p1_v2e_t     p3vlan2Etc;
    uint32 ftidx = 0;

    soc_sbx_g3p1_ft_t_init(&p3ft);
    p3ft.excidx = VLAN_INV_FTE_EXC;



        soc_sbx_g3p1_v2e_t_init(&p3vlan2Etc);
        p3vlan2Etc.dontlearn = TRUE;
        BCM_IF_ERROR_RETURN
           (soc_sbx_g3p1_vlan_ft_base_get(unit, &ftidx));
        ftidx += vsi;
        rv = soc_sbx_g3p1_ft_set(unit, ftidx, &p3ft);

    return rv;
}
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */

/*
 *  Function
 *    _bcm_caladan3_vswitch_destroy
 *  Purpose
 *    Destroy an existing VSI
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t = the VSI to destroy
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 */
int
_bcm_caladan3_vswitch_destroy(int unit,
                            bcm_vlan_t vsi)
{
    int rv = BCM_E_UNAVAIL;

    /* destroy the VSI */
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_vswitch_destroy(unit, vsi);
        break;
#endif /* def BCM_CALADAN3_G3P1_SUPPORT */
   default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_destroy
 *  Purpose
 *    Destroy an existing VSI
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t = the VSI to destroy
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 */
int
bcm_caladan3_vswitch_destroy(int unit,
                           bcm_vlan_t vsi)
{
    int rv;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X) enter\n"),
                 unit, vsi));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /* make sure the VSI is allocated */
    rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_VSI, vsi);
    if (BCM_E_EXISTS == rv) {
        rv = BCM_E_NONE;
    } else {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "access unit %d VSI %04X: %d (%s)\n"),
                   unit,
                   vsi,
                   rv,
                   _SHR_ERRMSG(rv)));
    }

    /* make sure VSI has no attached ports */
    if (BCM_E_NONE == rv) {
        if (_vswitch_state[unit]->portList[tempVsi]) {
            rv = BCM_E_BUSY;
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unit %d VSI %04X still has"
                                   " ports attached\n"),
                       unit,
                       vsi));
        }
    }

    /* destroy the VSI */
    if (BCM_E_NONE == rv) {
        rv = _bcm_caladan3_vswitch_destroy(unit, vsi);
    }

    /* release the VSI for reuse */
    if (BCM_E_NONE == rv) {
        rv = _sbx_caladan3_resource_free(unit,
                                    SBX_CALADAN3_USR_RES_VSI,
                                    1,
                                    &tempVsi,
                                    0);
    }

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X) return %d (%s)\n"),
                 unit,
                 vsi,
                 rv,
                 _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_caladan3_vswitch_port_get
 *  Purpose
 *    Get the vswitch of which the port is a member
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_gport_t port = the gport to check
 *    bcm_vlan_t *vsi = where to put the VSI of which the port is a member
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the check for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 *    This is used internally because we need a way to get the VSI from a given
 *    gport.  If 0 == VSI, we assume it has no assigned VSI.
 */
static int
_bcm_caladan3_vswitch_port_get(int unit,
                            bcm_gport_t gport,
                            bcm_vlan_t *vsi)
{
    int rv = BCM_E_UNAVAIL;
    int gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;

#ifdef DEBUG_MACLOG
    int my_log_id;

    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x53;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=gport;
    maclog[my_log_id].modid=-1;

    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */


    /* get the VSI from the port */
    switch (gpType) {
#if 0 
    case _SHR_GPORT_TYPE_MODPORT:
        /* fallthrough intentional -- port should do this also */
    case _SHR_GPORT_TYPE_LOCAL:
        /* need to have port do the work for this type */
        /*
         *  To work as I'd expect, this function would have to put the port
         *  into 'raw' mode, so that VID tags are considered part of the
         *  frame payload instead of something for the forwarder to parse,
         *  except that it probably needs to strip the tags so the proper
         *  tags can be put in the expected place on egress.  This would,
         *  as does VLAN, have to specify the egress encapsulation, though
         *  I'm not sure what kind of examples make real sense here.
         */
        rv = _bcm_caladan3_port_vsi_gport_get(unit, vsi, gport);
        break;
    case _SHR_GPORT_TYPE_MPLS_PORT:
        /* need to have MPLS do the work for this type */
        /*
         *  So this would probably set up that anything received with a
         *  specific MPLS label or label stack would have the labels tossed
         *  and the payload inserted to the VSI for forwarding.
         */
        rv = _bcm_caladan3_mpls_vsi_gport_get(unit, vsi, gport);
        break;
    /* Probably will also be other types that make sense here */
#endif 
    case BCM_GPORT_VLAN_PORT:
        /* need to have VLAN do the work for this type */
        rv = _bcm_caladan3_vlan_vsi_gport_get(unit, gport, vsi);
#ifdef DEBUG_MACLOG
	maclog[my_log_id].vid=*vsi;
#endif /* DEBUG_MACLOG */
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "can't query unacceptable gport %08X"
                               " type %d on unit %d\n"),
                   gport,
                   gpType,
                   unit));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_port_add
 *  Purpose
 *    Add a gport to a vswitch
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t vsi = the VSI to which the port is to be added
 *    bcm_gport_t port = the gport to add
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 */
int
bcm_caladan3_vswitch_port_add(int unit,
                            bcm_vlan_t vsi,
                            bcm_gport_t gport)
{
    int rv = BCM_E_UNAVAIL;
    int valid = FALSE;
    int gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;
    _bcm_caladan3_vswitch_port_t *listEntry = NULL;
    bcm_vlan_t currVsi = 0;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

#ifdef DEBUG_MACLOG
    int my_log_id;

    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x50;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=gport;
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vsi;
    maclog[my_log_id].fte=0;
    
#endif /* DEBUG_MACLOG */

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X,%08X) enter\n"),
                 unit,
                 vsi,
                 gport));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /* check the VSI to see if it's valid */
    if (BCM_VLAN_MAX < vsi) {
        /* not TB; make sure the VSI has been allocated */
        rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_VSI, vsi);
        if (BCM_E_EXISTS == rv) {
            rv = BCM_E_NONE;
        } else {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "access unit %d VSI %04X: %d (%s)\n"),
                       unit,
                       vsi,
                       rv,
                       _SHR_ERRMSG(rv)));
        }
    } else { /* if (BCM_VLAN_MAX < vsi) */
        /* Traditional bridging; ask VLAN if the VSI has been allocated */
        rv = _bcm_caladan3_vlan_check_exists(unit, vsi, &valid);
        if ((BCM_E_NONE == rv) && !valid) {
            /* VSI successfully found to not exist, so still an error here */
            rv = BCM_E_NOT_FOUND;
        }
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "access unit %d VLAN %03X: %d (%s)\n"),
                       unit,
                       vsi,
                       rv,
                       _SHR_ERRMSG(rv)));
        }
    } /* if (BCM_VLAN_MAX < vsi) */

    /* allocate the list entry for this port */
    if (BCM_E_NONE == rv) {
        listEntry = sal_alloc(sizeof(_bcm_caladan3_vswitch_port_t),
                              "_bcm_caladan3_vswitch_port_list_entry");
        if (listEntry) {
            listEntry->gport = gport;
            listEntry->next = _vswitch_state[unit]->portList[tempVsi];
            listEntry->prev = NULL;
        } else {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unable to create VSI"
                                   " gport list entry on unit %d\n"),
                       unit));
            rv = BCM_E_MEMORY;
        }
    }

    /* since we're keeping a list, must move by delete then add */
    if (BCM_E_NONE == rv) {
        rv = _bcm_caladan3_vswitch_port_get(unit, gport, &currVsi);
        if (BCM_E_NONE == rv) {
            if (currVsi && (currVsi != vsi)) {
                /* moving the gport to another VSI without removing it first */
                LOG_ERROR(BSL_LS_BCM_VLAN,
                          (BSL_META_U(unit,
                                      "trying to move unit %d gport"
                                       " %08X to VSI %04X without"
                                       " removing it from VSI %04X"
                                       " first\n"),
                           unit,
                           gport,
                           vsi,
                           currVsi));
                rv = BCM_E_CONFIG;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unable to read current VSI for "
                                   "unit %d gport %08X\n"),
                       unit,
                       gport));
        }
    }

    /* add the port to the VSI */
    if (BCM_E_NONE == rv) {
        switch (gpType) {
#if 0 
        case _SHR_GPORT_TYPE_MODPORT:
            /* fallthrough intentional -- port should do this also */
        case _SHR_GPORT_TYPE_LOCAL:
            /* need to have port do the work for this type */
            /*
             *  To work as I'd expect, this function would have to put the port
             *  into 'raw' mode, so that VID tags are considered part of the
             *  frame payload instead of something for the forwarder to parse,
             *  except that it probably needs to strip the tags so the proper
             *  tags can be put in the expected place on egress.  This would,
             *  as does VLAN, have to specify the egress encapsulation, though
             *  I'm not sure what kind of examples make real sense here.
             */
            rv = _bcm_caladan3_port_vsi_gport_add(unit, vsi, gport);
            break;
        case _SHR_GPORT_TYPE_MPLS_PORT:
            /* need to have MPLS do the work for this type */
            /*
             *  So this would probably set up that anything received with a
             *  specific MPLS label or label stack would have the labels tossed
             *  and the payload inserted to the VSI for forwarding.
             */
            rv = _bcm_caladan3_mpls_vsi_gport_add(unit, vsi, gport);
            break;
        /* Probably will also be other types that make sense here */
#endif 
        case BCM_GPORT_VLAN_PORT:
            /* need to have VLAN do the work for this type */
            rv = _bcm_caladan3_vlan_vsi_gport_add(unit, vsi, gport);
            break;
        default:
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "can't add unacceptable gport %08X"
                                   " type %d to unit %d VSI %04X\n"),
                       gport,
                       gpType,
                       unit,
                       vsi));
            rv = BCM_E_PARAM;
        }
    }

    if (BCM_E_NONE == rv) {
        /* add this port to those in the VSI */
        _vswitch_state[unit]->portList[tempVsi] = listEntry;
        if (listEntry->next) {
            listEntry->next->prev = listEntry;
        }
    } else {
        /* don't track this port */
        if (listEntry) {
            sal_free(listEntry);
        }
    }

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X,%08X) return %d (%s)\n"),
                 unit,
                 vsi,
                 gport,
                 rv,
                 _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_caladan3_vswitch_port_delete
 *  Purpose
 *    Remove a gport from a vswitch
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t vsi = the VSI from which the port is to be removed
 *    bcm_gport_t port = the gport to remove
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 *    This is called from delete and delete_all.
 */
static int
_bcm_caladan3_vswitch_port_delete(int unit,
                               bcm_vlan_t vsi,
                               bcm_gport_t gport)
{
    int rv;
    int gpType = (gport >> _SHR_GPORT_TYPE_SHIFT) & _SHR_GPORT_TYPE_MASK;


#ifdef DEBUG_MACLOG
    int my_log_id;

    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x51;
    maclog[my_log_id].mac[0]=0;
    maclog[my_log_id].mac[1]=0;
    maclog[my_log_id].mac[2]=0;
    maclog[my_log_id].mac[3]=0;
    maclog[my_log_id].mac[4]=0;
    maclog[my_log_id].mac[5]=0;
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=gport;
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vsi;
    maclog[my_log_id].fte=0;

#endif /* DEBUG_MACLOG */

    /* remove the gport from the VSI */
    switch (gpType) {
#if 0 
    case _SHR_GPORT_TYPE_MODPORT:
        /* fallthrough intentional -- port should do this also */
    case _SHR_GPORT_TYPE_LOCAL:
        /* need to have port do the work for this type */
        /*
         *  To work as I'd expect, this function would have to put the port
         *  into 'raw' mode, so that VID tags are considered part of the
         *  frame payload instead of something for the forwarder to parse,
         *  except that it probably needs to strip the tags so the proper
         *  tags can be put in the expected place on egress.  This would,
         *  as does VLAN, have to specify the egress encapsulation, though
         *  I'm not sure what kind of examples make real sense here.
         */
        rv = _bcm_caladan3_port_vsi_gport_delete(unit, vsi, gport);
        break;
    case _SHR_GPORT_TYPE_MPLS_PORT:
        /* need to have MPLS do the work for this type */
        /*
         *  So this would probably set up that anything received with a
         *  specific MPLS label or label stack would have the labels tossed
         *  and the payload inserted to the VSI for forwarding.  Of course,
         *  it also has to specify egress encapsulation (labels &c).
         */
        rv = _bcm_caladan3_mpls_vsi_gport_delete(unit, vsi, gport);
        break;
    /* Probably will also be other types that make sense here */
#endif 
    case BCM_GPORT_VLAN_PORT:
        /* need to have VLAN do the work for this type */
        rv = _bcm_caladan3_vlan_vsi_gport_delete(unit, vsi, gport);
        break;
    default:
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "can't remove unacceptable gport %08X"
                               " type %d from unit %d VSI %04X\n"),
                   gport,
                   gpType,
                   unit,
                   vsi));
        rv = BCM_E_PARAM;
    }

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_port_delete
 *  Purpose
 *    Remove a gport from a vswitch
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_vlan_t vsi = the VSI from which the port is to be removed
 *    bcm_gport_t port = the gport to remove
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 */
int
bcm_caladan3_vswitch_port_delete(int unit,
                               bcm_vlan_t vsi,
                               bcm_gport_t gport)
{
    int rv = BCM_E_NONE;
    _bcm_caladan3_vswitch_port_t *listCurr = NULL;
    bcm_vlan_t currVsi = 0;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X,%08X) enter\n"),
                 unit,
                 vsi,
                 gport));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /*
     *  Due to problems trying to coordinate TB VSIs with VSWITCH VSIs, we
     *  don't bother seeing if the VSI is valid here; all we do is make sure
     *  the VSI has some port in it (if not, we know we can return not found
     *  since the specified port isn't in the VSI).  Otherwise we have to check
     *  the GPORT to see if it's a member of the claimed VSI.
     */
    if (!(_vswitch_state[unit]->portList[tempVsi])) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "unit %d VSI %04X has no member GPORT\n"),
                   unit,
                   vsi));
        rv = BCM_E_NOT_FOUND;
    }

    /*
     *  Make sure the GPORT is actually a member of the claimed VSI, so we
     *  don't do something stupid like try to remove something that isn't in a
     *  list from that list.
     */
    if (BCM_E_NONE == rv) {
        rv = _bcm_caladan3_vswitch_port_get(unit, gport, &currVsi);
        if (BCM_E_NONE == rv) {
            if (currVsi && (currVsi != vsi)) {
                /* deleting the gport from the wrong VSI */
                LOG_ERROR(BSL_LS_BCM_VLAN,
                          (BSL_META_U(unit,
                                      "trying to remove unit %d"
                                       " gport %08X from VSI %04X but"
                                       " it belongs to VSI %04X\n"),
                           unit,
                           gport,
                           vsi,
                           currVsi));
                rv = BCM_E_CONFIG;
            }
        } else {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unable to read current VSI for "
                                   "unit %d gport %08X\n"),
                       unit,
                       gport));
        }
    }

    /* remove the gport from the VSI */
    if (BCM_E_NONE == rv) {
        rv = _bcm_caladan3_vswitch_port_delete(unit, vsi, gport);
    }

    /* remove the gport from the list of ports on this VSI */
    if (BCM_E_NONE == rv) {
        /* look for the gport to delete from the list */
        listCurr = _vswitch_state[unit]->portList[tempVsi];
        while (listCurr && (listCurr->gport != gport)) {
            /* still in the list and not found it yet */
            listCurr = listCurr->next;
        }
        if (listCurr) {
            /* found the gport to delete; delete it */
            if (listCurr->prev) {
                listCurr->prev->next = listCurr->next;
            } else {
                _vswitch_state[unit]->portList[tempVsi] = listCurr->next;
                if (_vswitch_state[unit]->portList[tempVsi]) {
                    _vswitch_state[unit]->portList[tempVsi]->prev = NULL;
                }
            }
            if (listCurr->next) {
                listCurr->next->prev = listCurr->prev;
            }

            sal_free(listCurr);
        }
        /*
         *  Going to take a lazy attitude here about not finding the list
         *  element to be deleted -- since it's already not in the list, we
         *  don't need to delete it, even though this situation should not
         *  happen unless due to corruption.
         */
    }

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X,%08X) return %d (%s)\n"),
                 unit,
                 vsi,
                 gport,
                 rv,
                 _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_caladan3_vswitch_port_delete_all
 *  Purpose
 *    Remove all ports from a VSI
 *  Arguments
 *    int unit = the unit on which to operate
 *    bcm_vlan_t vsi = the VSI whose ports are to be removed
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Walks the VSI's gport member list, deleting each one.
 *    THIS WILL NOT REMOVE NON-VSWITCH PORTS FROM THE VSI!
 */
static int
_bcm_caladan3_vswitch_port_delete_all(int unit,
                                    bcm_vlan_t vsi,
                                    _bcm_caladan3_vswitch_state_t *tempUnit)
{
    int rv = BCM_E_NONE;
    _bcm_caladan3_vswitch_port_t *listCurr = NULL;
    uint32 tempVsi = (vsi & 0xFFFF); /* stupid signed extension workaround */

    /* for all gports in the list for this VSI, delete the gport */
    do {
        listCurr = tempUnit->portList[tempVsi];
        if (listCurr) {
            rv = _bcm_caladan3_vswitch_port_delete(unit, vsi, listCurr->gport);
            if (BCM_E_NONE == rv) {
                /* deleted the port; remove it from the list */
                tempUnit->portList[tempVsi] = listCurr->next;
                sal_free(listCurr);
            }
        }
    } while (listCurr && (BCM_E_NONE == rv));

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_port_delete_all
 *  Purpose
 *    Remove all ports from a VSI
 *  Arguments
 *    int unit = the unit on which to operate
 *    bcm_vlan_t vsi = the VSI whose ports are to be removed
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Walks the VSI's gport member list, deleting each one.
 *    THIS WILL NOT REMOVE NON-VSWITCH PORTS FROM THE VSI!
 */

int
bcm_caladan3_vswitch_port_delete_all(int unit,
                                   bcm_vlan_t vsi)
{
    int rv = BCM_E_UNAVAIL;
    int valid = FALSE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X) enter\n"),
                 unit,
                 vsi));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    VSWITCH_LOCK_TAKE;

    /* check the VSI to see if it's valid */
    if (BCM_VLAN_MAX < vsi) {
        /* not TB; make sure the VSI has been allocated */
        rv = _sbx_caladan3_resource_test(unit, SBX_CALADAN3_USR_RES_VSI, vsi);
        if (BCM_E_EXISTS == rv) {
            rv = BCM_E_NONE;
        } else {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "access unit %d VSI %04X: %d (%s)\n"),
                       unit,
                       vsi,
                       rv,
                       _SHR_ERRMSG(rv)));
        }
    } else { /* if (BCM_VLAN_MAX < vsi) */
        /* Traditional bridging; ask VLAN if the VSI has been allocated */
        rv = _bcm_caladan3_vlan_check_exists(unit, vsi, &valid);
        if ((BCM_E_NONE == rv) && !valid) {
            /*
             *  Now here's where this gets interesting -- it's possible to
             *  delete a TB VSI without getting rid of its GPORTs.  Because of
             *  this, we need to check whether there are GPORTs in an invalid
             *  TB VSI and allow delete_all to work even in that case (but it
             *  should return an error if there are no member GPORTs *and* the
             *  VSI does not exist).
             */
            if (!(_vswitch_state[unit]->portList[vsi])) {
                rv = BCM_E_NOT_FOUND;
            }
        }
        if (BCM_E_NONE != rv) {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "access unit %d VLAN %03X: %d (%s)\n"),
                       unit,
                       vsi,
                       rv,
                       _SHR_ERRMSG(rv)));
        }
    } /* if (BCM_VLAN_MAX < vsi) */

    /* for all gports in the list for this VSI, delete the gport */
    if (BCM_E_NONE == rv) {
        rv = _bcm_caladan3_vswitch_port_delete_all(unit,
                                                 vsi,
                                                 _vswitch_state[unit]);
    }

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%04X) return %d (%s)\n"),
                 unit,
                 vsi,
                 rv,
                 _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_port_get
 *  Purpose
 *    Get the vswitch of which the port is a member
 *  Arguments
 *    int unit = on which unit to destroy the VSI
 *    bcm_gport_t port = the gport to check
 *    bcm_vlan_t *vsi = where to put the VSI of which the port is a member
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* otherwise
 *  Notes
 *    Defers to the appropriate module to make the changes for its own gport
 *    type rather than trying to include that knowledge (and access to the
 *    appropriate locks) here.
 */
int
bcm_caladan3_vswitch_port_get(int unit,
                            bcm_gport_t gport,
                            bcm_vlan_t *vsi)
{
    int rv = BCM_E_UNAVAIL;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%08X,*) enter\n"),
                 unit,
                 gport));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if (!vsi) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "NULL pointer for outbound argument\n")));
        return BCM_E_PARAM;
    }

    VSWITCH_LOCK_TAKE;

    rv = _bcm_caladan3_vswitch_port_get(unit, gport, vsi);

    VSWITCH_LOCK_RELEASE;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,%08X,&(%04X)) return %d (%s)\n"),
                 unit,
                 gport,
                 *vsi,
                 rv,
                 _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    _bcm_caladan3_vswitch_detach
 *  Purpose
 *    Shut down the vswitch functionality
 *  Arguments
 *    int unit = the unit whose vswitch function is to be initialised
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 */
static int
_bcm_caladan3_vswitch_detach(int unit)
{
    int rv = BCM_E_NONE;
    _bcm_caladan3_vswitch_state_t *tempUnit = _vswitch_state[unit];

    VSWITCH_EVERB((BSL_META_U(unit,
                              "(%d) enter\n"),
                   unit));

    /* disconnect the unit information (keep others from seeing unit) */
    VSWITCH_EVERB((BSL_META_U(unit,
                              "unlink unit %d data\n"),
                   unit));
    _vswitch_state[unit] = NULL;

    /* destroy the lock (this should wait until no other owners) */
    VSWITCH_EVERB((BSL_META_U(unit,
                              "destroy unit %d lock\n"),
                   unit));
    sal_mutex_destroy(tempUnit->lock);

    /* get rid of the unit state information */
    VSWITCH_EVERB((BSL_META_U(unit,
                              "release unit %d state structure\n"),
                   unit));
    sal_free(tempUnit->portList);
    sal_free(tempUnit);

    VSWITCH_EVERB((BSL_META_U(unit,
                              "(%d) return %d (%s)\n"),
                   unit,
                   rv,
                   _SHR_ERRMSG(rv)));

    return rv;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_detach
 *  Purpose
 *    Shut down the vswitch functionality
 *  Arguments
 *    int unit = the unit whose vswitch function is to be initialised
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 */
int
bcm_caladan3_vswitch_detach(int unit)
{
    int result;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    VSWITCH_UNIT_VALID_CHECK;
    VSWITCH_UNIT_INIT_CHECK;

    if (sal_mutex_take(_primary_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "unable to take primary lock\n")));
        return BCM_E_INTERNAL;
    }

    result = _bcm_caladan3_vswitch_detach(unit);

    if (sal_mutex_give(_primary_lock)) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "unable to release primary lock\n")));
        return BCM_E_INTERNAL;
    }

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d) return %d (%s)\n"),
                 unit,
                 result,
                 _SHR_ERRMSG(result)));

    return result;
}

/*
 *  Function
 *    bcm_caladan3_vswitch_init
 *  Purpose
 *    Initialise the vswitch functionality
 *  Arguments
 *    int unit = the unit whose vswitch function is to be initialised
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 */
int
bcm_caladan3_vswitch_init(int unit)
{
    sal_mutex_t tempLock = NULL;
    _bcm_caladan3_vswitch_state_t *tempUnit = NULL;
    bcm_vlan_t vIndex;
    int didDetach = FALSE;
    int result;
    uint32             fti;
    soc_sbx_g3p1_ft_t  fte;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d) enter\n"),
                 unit));

    VSWITCH_UNIT_VALID_CHECK;

    /* configure the primary lock */
    if (!_primary_lock) {
        tempLock = sal_mutex_create("_bcm_caladan3_vswitch_primary_lock");
        if (NULL == tempLock) {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "unable to create temp"
                                   "primary lock\n")));
            return BCM_E_RESOURCE;
        }
        _primary_lock = tempLock;
    }

    if (sal_mutex_take(_primary_lock, sal_mutex_FOREVER)) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "unable to take primary lock\n")));
        return BCM_E_INTERNAL;
    }

    if (tempLock && (_primary_lock != tempLock)) {
        LOG_WARN(BSL_LS_BCM_VLAN,
                 (BSL_META_U(unit,
                             "concurrent init; discarding spare"
                              " primary lock instance\n")));
        sal_mutex_destroy(tempLock);
    }

    /* hope for the best */
    result = BCM_E_NONE;

    /* do reinit cleanly (detach first) */
    if (_vswitch_state[unit]) {
        /* already initialised; do detach first */
        LOG_DEBUG(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "detach unit %d first\n"),
                   unit));
        result = _bcm_caladan3_vswitch_detach(unit);
        didDetach = TRUE;
    }

    /* create new unit state information */
    if (BCM_E_NONE == result) {
        LOG_DEBUG(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "allocate %d bytes unit %d state\n"),
                   sizeof(_bcm_caladan3_vswitch_state_t),
                   unit));
        tempUnit = sal_alloc(sizeof(_bcm_caladan3_vswitch_state_t),
                                    "_bcm_caladan3_vswitch_unit_state");
        if (tempUnit) {
            /* success; zero the memory cell */
            sal_memset(tempUnit, 0, sizeof(_bcm_caladan3_vswitch_state_t));
            /* create the unit lock */
            tempUnit->lock = sal_mutex_create("_bcm_caladan3_vswitch_unit_lock");
            if (!tempUnit->lock) {
                /* failed to create the unit lock */
                result = BCM_E_RESOURCE;
            }
            tempUnit->portList = sal_alloc((sizeof(_bcm_caladan3_vswitch_port_t *)*SBX_DYNAMIC_VSI_END(unit)), "_bcm_caladan3_vswitch_port_t");
            if(tempUnit->portList)
            {
                sal_memset(tempUnit->portList, 0, (sizeof(_bcm_caladan3_vswitch_port_t *)*SBX_DYNAMIC_VSI_END(unit)));
            }
            else
            {
                /* failed to create the working unit descriptor */
                result = BCM_E_MEMORY;
            }
        } else { /* if (tempUnit) */
            /* failed to create the working unit descriptor */
            result = BCM_E_MEMORY;
        } /* if (tempUnit) */
    } /* if (BCM_E_NONE == result) */

    if ((BCM_E_NONE == result) && !didDetach) {
        /* initialise appropriate tables */
        LOG_DEBUG(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "initialise unit %d VSI tables\n"),
                   unit));
        for (vIndex = SBX_DYNAMIC_VSI_BASE(unit);
             vIndex >= SBX_DYNAMIC_VSI_END(unit);
             vIndex--) {
            _bcm_caladan3_vswitch_destroy(unit, vIndex);
        }
    }

    LOG_DEBUG(BSL_LS_BCM_VLAN,
              (BSL_META_U(unit,
                          "commit unit %d changes\n"),
               unit));
    if (BCM_E_NONE == result) {
        /* all unit preparation went well; attach unit structure */
        _vswitch_state[unit] = tempUnit;
    } else { /* if (BCM_E_NONE == result) */
        /* something went wrong; undo any remaining unit allocation */
        if (tempUnit) {
            if (tempUnit->lock) {
                sal_mutex_destroy(tempUnit->lock);
            }
            sal_free(tempUnit->portList);
            sal_free(tempUnit);
        } /* if (tempUnit) */
        /*
         *  Note that we don't try to destroy the global lock; it's shared
         *  between all units and should not be destroyed once it exists.
         */
    } /* if (BCM_E_NONE == result) */


    if (SOC_WARM_BOOT(unit)) {
        uint32 vsi, base_fti;

        result = soc_sbx_g3p1_vlan_ft_base_get(unit, &base_fti);
        if (BCM_FAILURE(result)) {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "Failed to get base ft: %s\n"),
                       bcm_errmsg(result)));

            if (sal_mutex_give(_primary_lock)) {
                LOG_ERROR(BSL_LS_BCM_VLAN,
                          (BSL_META_U(unit,
                                      "unable to release primary lock\n")));
            }

            return result;
        }

        for (fti = SBX_DYNAMIC_VSI_FTE_BASE(unit);
             fti <= SBX_DYNAMIC_VSI_FTE_END(unit);
             fti++) 
        {

            result = soc_sbx_g3p1_ft_get(unit, fti, &fte);

            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_VLAN,
                          (BSL_META_U(unit,
                                      "Failed to read fte[0x%x]: %s\n"),
                           fti, bcm_errmsg(result)));

                if (sal_mutex_give(_primary_lock)) {
                    LOG_ERROR(BSL_LS_BCM_VLAN,
                              (BSL_META_U(unit,
                                          "unable to release primary lock\n")));
                }
                
                return result;
            }

            if (fte.mc && fte.excidx != VLAN_INV_FTE_EXC) {
                vsi = fti - base_fti;
                result = _sbx_caladan3_resource_alloc(unit,
                                                 SBX_CALADAN3_USR_RES_VSI,
                                                 1, &vsi, 
                                                 _SBX_CALADAN3_RES_FLAGS_RESERVE);
                LOG_VERBOSE(BSL_LS_BCM_VLAN,
                            (BSL_META_U(unit,
                                        "Recover VSI FTE[0x%04x]: %d\n"), 
                             fti, result));
                             
                if (BCM_FAILURE(result)) {
                    LOG_ERROR(BSL_LS_BCM_VLAN,
                              (BSL_META_U(unit,
                                          "Error reserving VSI: %s\n"),
                               bcm_errmsg(result)));
                }
            }
        }

    } else {

        soc_sbx_g3p1_ft_t_init(&fte);
        fte.excidx = VLAN_INV_FTE_EXC;
        
        for (fti = SBX_DYNAMIC_VSI_FTE_BASE(unit);
             fti <= SBX_DYNAMIC_VSI_FTE_END(unit);
             fti++) 
        {
            result = soc_sbx_g3p1_ft_set(unit, fti, &fte);

            if (BCM_FAILURE(result)) {
                LOG_ERROR(BSL_LS_BCM_VLAN,
                          (BSL_META_U(unit,
                                      "Failed to write fte[0x%x]: %s\n"),
                           fti, bcm_errmsg(result)));

                if (sal_mutex_give(_primary_lock)) {
                    LOG_ERROR(BSL_LS_BCM_VLAN,
                              (BSL_META_U(unit,
                                          "unable to release primary lock\n")));
                }
                
                return result;
            }        
        }
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    result = bcm_caladan3_wb_vswitch_state_init(unit);
    if (SOC_WARM_BOOT(unit) ) {
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_VLAN,
                      (BSL_META_U(unit,
                                  "vswitch warm boot init failed, error %d(%s)\n"),
                       result, bcm_errmsg(result)));
        }
    }
#endif /* BCM_WARM_BOOT_SUPPORT */

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d) return %d (%s)\n"),
                 unit, result, _SHR_ERRMSG(result)));

    if (sal_mutex_give(_primary_lock)) {
        LOG_ERROR(BSL_LS_BCM_VLAN,
                  (BSL_META_U(unit,
                              "unable to release primary lock\n")));
    }
    
    return result;
}



/*
 *  Function
 *    _bcm_caladan3_vswitch_port_info_get
 *  Purpose
 *    Obtain tagged/untagged information with regard to double tagging for
 *    ports on a vsi
 *  Arguments
 *    unit    - bcm device number
 *    port    - local target port
 *    vsi     - vsi to scan
 *    keepUntagged - storage location for behavior setting
 */
int
_bcm_caladan3_vswitch_port_info_get(int unit, bcm_port_t port,
                                  bcm_vlan_t vsi, int *keepUntagged)
{
    int rv = BCM_E_NOT_FOUND;
    _bcm_caladan3_vswitch_port_t *pCur = NULL;
    bcm_vlan_port_t vlanPort;
    bcm_module_t    localMod, vpMod;
    bcm_port_t      vpPort;

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,0x%X,0x%04X) enter\n"), 
                 unit, port, vsi));

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &localMod));


    VSWITCH_LOCK_TAKE;

    /* iterate all gports in the VSI -
     *   for each vlan gport -
     *     vlan_port from vlan with bcm_vlan_port_find
     *     set keepUntagged if port/mods match and not set to ADD a vid
     */

    pCur = _vswitch_state[unit]->portList[vsi];
    while (pCur && (rv == BCM_E_NOT_FOUND)) {
        if (BCM_GPORT_IS_VLAN_PORT(pCur->gport)) {
            int tmpRv;

            sal_memset(&vlanPort, 0, sizeof(bcm_vlan_port_t));
            vlanPort.vlan_port_id = pCur->gport;
            
            tmpRv = bcm_vlan_port_find(unit, &vlanPort);
            
            if (BCM_SUCCESS(tmpRv)) {
                if (!BCM_GPORT_IS_SET(vlanPort.port)) {
                    BCM_GPORT_MODPORT_SET(vlanPort.port, localMod, vlanPort.port);
                }
                vpMod  = BCM_GPORT_MODPORT_MODID_GET(vlanPort.port);
                vpPort = BCM_GPORT_MODPORT_PORT_GET(vlanPort.port);
                
                LOG_VERBOSE(BSL_LS_BCM_VLAN,
                            (BSL_META_U(unit,
                                        " gp=0x%08x modport=0x%08x flags=0x%08x "
                                         "port=%d mod=%d\n"),
                             vlanPort.vlan_port_id, vlanPort.port, vlanPort.flags,
                             vpPort, vpMod));
                
                /* not adding a vlan, keep the packet untagged when egress the
                 * provider port (ie. don't add the nativevid)
                 */
                if (vpMod == localMod && vpPort == port &&
                    (!(vlanPort.flags & BCM_VLAN_PORT_INNER_VLAN_PRESERVE) ||
                     !(vlanPort.flags & BCM_VLAN_PORT_INNER_VLAN_ADD)))
                {
                    *keepUntagged = 1;
                    rv = BCM_E_NONE;
                }
            }
        }

        pCur = pCur->next;
    }

    VSWITCH_LOCK_RELEASE;

    return rv;
}

/*
 *  Function
 *    _bcm_caladan3_vswitch_port_gport_get
 *  Purpose
 *    Gets a matching gport from a port,VSI
 *  Arguments
 *    int unit = the unit on which to operate
 *    bcm_port_t port = the matching port
 *    bcm_vlan_t vsi = the matching VSI
 *    bcm_gport_t gport = the return gport
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    only finds the first matching gport.
 *    there could be more than one match.
 */
int
_bcm_caladan3_vswitch_port_gport_get(int unit,
                                   bcm_port_t  port,
                                   bcm_vlan_t  vsi,
                                   bcm_gport_t *gport)
{
    int rv = BCM_E_NOT_FOUND;

    int             tmpRv;
    bcm_vlan_port_t vlanPort;
    bcm_module_t    localMod;
    bcm_module_t    vpMod;
    bcm_port_t      vpPort;
    uint32          tempVsi = (vsi & 0xFFFF);

    _bcm_caladan3_vswitch_port_t *listCurr = NULL;

    *gport = 0; 

    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                (BSL_META_U(unit,
                            "(%d,0x%X,0x%04X) enter\n"),
                 unit, port, vsi));

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &localMod));

    VSWITCH_LOCK_TAKE;
    listCurr = _vswitch_state[unit]->portList[tempVsi];

    /* for all gports in the list for this VSI, find a matching port */
    do {
        if (listCurr) {
            if (BCM_GPORT_IS_VLAN_PORT(listCurr->gport)) {
                sal_memset(&vlanPort, 0, sizeof(bcm_vlan_port_t));
                vlanPort.vlan_port_id = listCurr->gport;
                tmpRv = bcm_vlan_port_find(unit, &vlanPort);

                if (BCM_SUCCESS(tmpRv)) {
                    if (!BCM_GPORT_IS_SET(vlanPort.port)) {
                        BCM_GPORT_MODPORT_SET(vlanPort.port, localMod, vlanPort.port);
                    }
                    vpMod  = BCM_GPORT_MODPORT_MODID_GET(vlanPort.port);
                    vpPort = BCM_GPORT_MODPORT_PORT_GET(vlanPort.port);

                    LOG_VERBOSE(BSL_LS_BCM_VLAN,
                                (BSL_META_U(unit,
                                            " gp=0x%08x modport=0x%08x flags=0x%08x "
                                             "port=%d mod=%d\n"),
                                 vlanPort.vlan_port_id, vlanPort.port, vlanPort.flags,
                                 vpPort, vpMod));

                    if ((port == vpPort) && (localMod == vpMod)) {
                        *gport = listCurr->gport;
                        rv = BCM_E_NONE;
                    }
                }
            listCurr = listCurr->next;
            }
        }
    } while (listCurr && (BCM_E_NOT_FOUND == rv));

    VSWITCH_LOCK_RELEASE;

    return rv;
}

