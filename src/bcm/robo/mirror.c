/*
 * $Id: mirror.c,v 1.44 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Mirror - Broadcom RoboSwitch Mirror API.
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/debug.h>

#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/port.h>
#include <bcm/stack.h>
#include <bcm/switch.h>

#include <bcm_int/robo/port.h>
#include <bcm_int/robo/mirror.h>

#include <bcm_int/robo_dispatch.h>

typedef struct {
    int init;           /* TRUE if Mirror module has been inited */
    int mode;       /* mirroring mode */
    bcm_pbmp_t mirror_mtp_bmp;       /* single or multiple mirror-to ports bitmap to set */
    bcm_pbmp_t fp_mtp_bmp;       /* mirror-to ports bitmap to set from FP */
    uint16 fp_ref_count[SOC_MAX_NUM_PORTS];  /* mirror-to ports reference count from FP */
    bcm_port_t fp_mtp_any;
    int modid;          /* module id of mirror-to port */
    uint32 flags;
} mirror_info_t;

static mirror_info_t    _bcm_robo_mirror_info[BCM_MAX_NUM_UNITS];

/* The mirror control for this unit */
#define MIRROR_INFO(unit)   _bcm_robo_mirror_info[unit]

/* return BCM_E_INIT if mirroring is not initialized */
#define MIRROR_INIT(unit) \
    if (!MIRROR_INFO(unit).init) {return BCM_E_INIT;}

/*
 * Function:
 *      _bcm_robo_mirror_gport_construct
 * Description:
 *      Constructs gport for mirror module
 * Parameters:
 *      unit        - (IN) BCM device number
 *      port_tgid   - (IN) port or trunk id to construct into a gprot
 *      modid       - (IN) module id to construct into a gport
 *      flags       - (IN) Mirror trunk flag
 *      gport       - (OUT)- gport to to construct
 * Returns:
 *      BCM_E_XXX
 */
STATIC int 
_bcm_robo_mirror_gport_construct(int unit, int port_tgid, int modid, uint32 flags, 
                            bcm_gport_t *gport)
{
    _bcm_gport_dest_t  dest;
    bcm_module_t  mymodid;
    int  rv;

    sal_memset(&dest, 0, sizeof (_bcm_gport_dest_t));
    if (flags & BCM_MIRROR_PORT_DEST_TRUNK) {
        dest.tgid = port_tgid;
        dest.gport_type = _SHR_GPORT_TYPE_TRUNK;
    } else {
        dest.port = port_tgid;
        if (IS_ST_PORT(unit, port_tgid)) {
            rv = bcm_robo_stk_my_modid_get(unit, &mymodid);
            if (BCM_E_UNAVAIL == rv) {
                dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
            } else {
                dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
                dest.modid = modid;
            }
        } else {
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
            dest.modid = modid;
        }
    }
    BCM_IF_ERROR_RETURN(
        _bcm_robo_gport_construct(unit, &dest, gport));

    return BCM_E_NONE;
}

int
_bcm_robo_mirror_fp_dest_add(int unit, int flags, int port) 
{
    int rv;
    bcm_port_t any;

    MIRROR_INIT(unit);
    /* At least one packet direction must be specified. */
    if (!(flags & BCM_MIRROR_PORT_INGRESS)) {
        return (BCM_E_PARAM);
    }
    
    if (MIRROR_INFO(unit).mode == BCM_MIRROR_DISABLE) {
        MIRROR_INFO(unit).mode = BCM_MIRROR_L2;
    }

    if (port == -1) {
        if (MIRROR_INFO(unit).fp_mtp_any == -1) {
            /* choose a mirror_to port */
            rv = bcm_robo_mirror_to_get(unit, &any);
            BCM_IF_ERROR_RETURN(rv);
            if (any == -1) {
                MIRROR_INFO(unit).fp_mtp_any = 0;
            } else {
                MIRROR_INFO(unit).fp_mtp_any = any;
            }
        }
        port = MIRROR_INFO(unit).fp_mtp_any;
    }

    if (!BCM_PBMP_MEMBER(MIRROR_INFO(unit).fp_mtp_bmp, port)) {
        BCM_PBMP_PORT_ADD(MIRROR_INFO(unit).fp_mtp_bmp, port);
    }

    MIRROR_INFO(unit).fp_ref_count[port]++;

    MIRROR_INFO(unit).flags |= _BCM_MIRROR_INGRESS_FROM_FP;
    rv = bcm_robo_mirror_mode_set(unit, MIRROR_INFO(unit).mode);
    BCM_IF_ERROR_RETURN(rv);
        
    return BCM_E_NONE;
}

int
_bcm_robo_mirror_fp_dest_delete(int unit, int flags, int port) 
{
    uint32  menable = 0;
    bcm_pbmp_t igr_pbmp, egr_pbmp, mtp_bmp;
    int rv = BCM_E_NONE;

    MIRROR_INIT(unit);
    /* At least one packet direction must be specified. */
    if (!(flags & BCM_MIRROR_PORT_INGRESS)) {
        return (BCM_E_PARAM);
    }
    if (port == -1) {
        if (MIRROR_INFO(unit).fp_mtp_any == -1) {
            port = 0;
        } else {
            port = MIRROR_INFO(unit).fp_mtp_any;
        }
    }

    BCM_PBMP_CLEAR(mtp_bmp);
    BCM_PBMP_CLEAR(igr_pbmp);
    BCM_PBMP_CLEAR(egr_pbmp);

   if (MIRROR_INFO(unit).flags & _BCM_MIRROR_INGRESS_FROM_FP) {
        if (MIRROR_INFO(unit).fp_ref_count[port] == 0) {
            return BCM_E_PARAM;
        }
        MIRROR_INFO(unit).fp_ref_count[port]--;

        if (MIRROR_INFO(unit).fp_ref_count[port] == 0) {
            BCM_PBMP_PORT_REMOVE(MIRROR_INFO(unit).fp_mtp_bmp, port);

            if (BCM_PBMP_IS_NULL(MIRROR_INFO(unit).fp_mtp_bmp)) {
                MIRROR_INFO(unit).flags &= ~(_BCM_MIRROR_INGRESS_FROM_FP);

                BCM_IF_ERROR_RETURN(DRV_MIRROR_GET
                    (unit, &menable, &mtp_bmp, &igr_pbmp, &egr_pbmp));
    
                if (BCM_PBMP_IS_NULL(igr_pbmp) && BCM_PBMP_IS_NULL(egr_pbmp)) {
                    rv = bcm_robo_mirror_mode_set(unit, BCM_MIRROR_DISABLE);
                    if (BCM_SUCCESS(rv)) {
                        MIRROR_INFO(unit).fp_mtp_any = -1;
                    }
                    return (rv);
                } 
            }
        }
        rv = bcm_robo_mirror_mode_set(unit, MIRROR_INFO(unit).mode);
        BCM_IF_ERROR_RETURN(rv);
   }

   return BCM_E_NONE;
}

/*
 * Function:
 *	  bcm_robo_mirror_deinit
 * Purpose:
 *	  Deinitialize mirror software module.
 * Parameters:
 *    unit - (IN) BCM device number.
 * Returns:
 *	  BCM_E_XXX
 */
int
bcm_robo_mirror_deinit(int unit)
{
    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_mirror_init
 * Purpose:
 *  Initialize mirror software system.
 * Parameters:
 *  unit - RoboSwitch unit #.
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_mirror_init(int unit)
{
    bcm_port_t  port = 0;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_init()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    MIRROR_INFO(unit).init = TRUE;
    MIRROR_INFO(unit).mode = -1;
    MIRROR_INFO(unit).fp_mtp_any = -1;
    BCM_PBMP_CLEAR(MIRROR_INFO(unit).mirror_mtp_bmp);
    BCM_PBMP_CLEAR(MIRROR_INFO(unit).fp_mtp_bmp);
    PBMP_ALL_ITER(unit, port) {
        MIRROR_INFO(unit).fp_ref_count[port] = 0;
    }
    MIRROR_INFO(unit).flags= 0;
    return bcm_robo_mirror_mode_set(unit, BCM_MIRROR_DISABLE);
}

/*
 * Function:
 *  bcm_robo_mirror_mode_set
 * Description:
 *  Enable or disable mirroring.  Will wait for bcm_mirror_to_set to be
 *  called to actually do the enable if needed.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  mode - One of BCM_MIRROR_(DISABLE|L2|L2_L3)
 * Returns:
 *  BCM_E_XXX
 */

int bcm_robo_mirror_mode_set(int unit, int mode)
{
    uint32      menable, t_mode;
    bcm_pbmp_t  mtp_bmp, t_mtp_bmp;
    bcm_pbmp_t  igr_pbmp, egr_pbmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_mode_set()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }
        
    /* Get the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_GET
        (unit, &menable, &mtp_bmp, &igr_pbmp, &egr_pbmp));

    /* If mode is Mirror-L2, enable both Mirror-ingress and Mirror-egress, */
    /* else set Mirror-disable */
    if (mode == BCM_MIRROR_L2) {
        t_mode = TRUE;
    } else if (mode == BCM_MIRROR_L2_L3){
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "No L3 mirror mode for Robo!\n")));
        t_mode = TRUE;
        return BCM_E_UNAVAIL;
    } else if (mode == BCM_MIRROR_DISABLE) {
        t_mode = FALSE;
    } else {
        return BCM_E_PARAM;
    }

    /* t_mtp_bmp=MIRROR_INFO(unit).mirror_mtp_bmp | MIRROR_INFO(unit).fp_mtp_bmp */
    BCM_PBMP_ASSIGN(t_mtp_bmp, MIRROR_INFO(unit).mirror_mtp_bmp);
    BCM_PBMP_OR(t_mtp_bmp, MIRROR_INFO(unit).fp_mtp_bmp);

    if ((t_mode == menable) && (BCM_PBMP_EQ(t_mtp_bmp, mtp_bmp))) {
        MIRROR_INFO(unit).mode = mode;
        return BCM_E_NONE;
    }
    
    if (t_mode == FALSE) {
        BCM_PBMP_CLEAR(igr_pbmp);
        BCM_PBMP_CLEAR(egr_pbmp);
        if (MIRROR_INFO(unit).flags & _BCM_MIRROR_INGRESS_FROM_FP) {
            /* check if any mirror-to port set from FP */
            LOG_WARN(BSL_LS_BCM_COMMON,
                     (BSL_META_U(unit,
                                 "bcmFieldActionMirrorIngress need to enable mirror mode\n")));
            BCM_PBMP_ASSIGN(t_mtp_bmp, MIRROR_INFO(unit).fp_mtp_bmp);
            if (DRV_MIRROR_SET(unit, TRUE, t_mtp_bmp, igr_pbmp, egr_pbmp) < 0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Fail to set ingress/egress pbm to 0\n")));
            }
            return BCM_E_RESOURCE;
        }
        BCM_PBMP_CLEAR(t_mtp_bmp);    /* clear port when disabling */
    } else {
        if (BCM_PBMP_IS_NULL(t_mtp_bmp)) {          /* wait for mirror_to_set() */
            MIRROR_INFO(unit).mode = mode;
            return BCM_E_NONE;
        }
    }

    /* Set the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_SET
        (unit, t_mode, t_mtp_bmp, igr_pbmp, egr_pbmp));
    MIRROR_INFO(unit).mode = mode;

    return BCM_E_NONE;

}   

/*
 * Function:
 *  bcm_robo_mirror_mode_get
 * Description:
 *  Get mirror mode.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  mode - (OUT) One of BCM_MIRROR_(DISABLE|L2|L2_L3)
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_mirror_mode_get(int unit, int *mode)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_mode_get()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (MIRROR_INFO(unit).mode < 0) {
        *mode = BCM_MIRROR_DISABLE;
    } else {
        *mode = MIRROR_INFO(unit).mode;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_mirror_to_set
 * Description:
 *  Set the mirror-to port for all mirroring, enabling mirroring
 *  if a mode has previously been set.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to mirror all ingress/egress selections to
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_mirror_to_set(int unit, bcm_port_t port)
{
    bcm_pbmp_t  mtp_bmp, t_mtp_bmp;
    int  rv = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_to_set()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    /* Initialization check. */
    MIRROR_INIT(unit);
    BCM_PBMP_PORT_SET(mtp_bmp, port);

    /* backup the original mirror-to port bitmap status and need to recover back if return fails */
    BCM_PBMP_ASSIGN(t_mtp_bmp, MIRROR_INFO(unit).mirror_mtp_bmp);

    BCM_PBMP_ASSIGN(MIRROR_INFO(unit).mirror_mtp_bmp, mtp_bmp);

    rv = bcm_mirror_mode_set(unit, MIRROR_INFO(unit).mode);
    if (rv < 0) {
        /* recover back to  the original mirror-to port bitmap status */
        BCM_PBMP_ASSIGN(MIRROR_INFO(unit).mirror_mtp_bmp, t_mtp_bmp);
    }

    return rv;
}

/*
 * Function:
 *  bcm_robo_mirror_to_get
 * Description:
 *  Get the mirror-to port for all mirroring
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - (OUT) The port to mirror all ingress/egress selections to
 * Returns:
 *  BCM_E_XXX
 */
int
bcm_robo_mirror_to_get(int unit, bcm_port_t *port)
{
    bcm_pbmp_t  mtp_bmp;
    bcm_port_t mport = -1;
    int  isGport;    /* Indicator on which format to return port */
    bcm_module_t  mymodid;    /* module id to construct a gport */
    int  mod_out, port_out; /* To do a modmap mapping */
    uint32  flags;      /* Mirror destination flags. */

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_to_get()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Initialization check. */
    MIRROR_INIT(unit);

    BCM_PBMP_CLEAR(mtp_bmp);
    /* Return mirror software info. for mirror-to port bitmap */
    BCM_PBMP_ASSIGN(mtp_bmp, MIRROR_INFO(unit).mirror_mtp_bmp);

    /* If chips support multiple mirror-to ports, return the first one mport */
    if (BCM_PBMP_IS_NULL(mtp_bmp)) {
        mport = -1;
    } else {
        BCM_PBMP_ITER(mtp_bmp, mport) {
            break;
        }
    }

    *port = mport;

    flags = 0;
    BCM_IF_ERROR_RETURN
        (bcm_robo_stk_my_modid_get(unit, &mymodid));

    BCM_IF_ERROR_RETURN
        (bcm_switch_control_get(unit, bcmSwitchUseGport, &isGport));
    if (isGport) {
        BCM_IF_ERROR_RETURN
            (bcm_robo_stk_modmap_map(unit, BCM_STK_MODMAP_GET, mymodid, *port, 
            &mod_out, &port_out));
        BCM_IF_ERROR_RETURN
            (_bcm_robo_mirror_gport_construct(unit, port_out, mod_out, flags, port)); 
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_robo_mirror_ingress_set
 * Description:
 *     Enable or disable mirroring per ingress
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to affect
 *  val - Boolean value for on/off
 * Returns:
 *     BCM_E_xxxx
 * Notes:
 *     Mirroring must also be globally enabled.
 */
int 
bcm_robo_mirror_ingress_set(int unit, bcm_port_t port, int val)
{
    uint32  menable;
    bcm_pbmp_t  t_pbmp;
    bcm_pbmp_t igr_pbmp, egr_pbmp, mtp_bmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_ingress_set()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    MIRROR_INIT(unit);

    /* Get the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_GET
        (unit, &menable, &mtp_bmp, &igr_pbmp, &egr_pbmp));
    BCM_PBMP_CLEAR(t_pbmp);
    BCM_PBMP_ASSIGN(t_pbmp, igr_pbmp);

    /* BCM5345 allowed one Ingress Mirrored port only */
    if (val) {
        if (BCM_PBMP_MEMBER(t_pbmp, port)){
            return BCM_E_NONE;
        }
        BCM_PBMP_PORT_ADD(t_pbmp, port);
    } else {
        if (!BCM_PBMP_MEMBER(t_pbmp, port)){
            return BCM_E_NONE;
        }
        BCM_PBMP_PORT_REMOVE(t_pbmp, port);
    }
    
    BCM_PBMP_ASSIGN(igr_pbmp, t_pbmp);

    /* Set the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_SET
        (unit, menable, mtp_bmp, igr_pbmp, egr_pbmp));
    
    return BCM_E_NONE;
}   

/*
 * Function:
 *     bcm_robo_mirror_ingress_get
 * Description:
 *     Get the mirroring per ingress enabled/disabled status
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to check
 *  val - Place to store boolean return value for on/off
 * Returns:
 *     BCM_E_xxxx
 */
int 
bcm_robo_mirror_ingress_get(int unit, bcm_port_t port, int *val)
{
    uint32  menable;
    bcm_pbmp_t  igr_pbmp, egr_pbmp, mtp_bmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_ingress_get()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    MIRROR_INIT(unit);

     /* Get the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_GET
        (unit, &menable, &mtp_bmp, &igr_pbmp, &egr_pbmp));
    
    *val = (BCM_PBMP_MEMBER(igr_pbmp, port)) ? 1 : 0;
    
    return BCM_E_NONE;    
}   

/*
 * Function:
 *     bcm_robo_mirror_egress_set
 * Description:
 *     Enable or disable mirroring per egress
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to affect
 *  val - Boolean value for on/off
 * Returns:
 *     BCM_E_xxx
 * Notes:
 *     Mirroring must also be globally enabled.
 */
int 
bcm_robo_mirror_egress_set(int unit, bcm_port_t port, int val)
{
    uint32  menable;
    bcm_pbmp_t  t_pbmp;
    bcm_pbmp_t  igr_pbmp, egr_pbmp, mtp_bmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_egress_set()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    MIRROR_INIT(unit);

     /* Get the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_GET
        (unit, &menable, &mtp_bmp, &igr_pbmp, &egr_pbmp));
    BCM_PBMP_CLEAR(t_pbmp);
    BCM_PBMP_ASSIGN(t_pbmp, egr_pbmp);

    if (val) {
        if (BCM_PBMP_MEMBER(t_pbmp, port)){
            return BCM_E_NONE;
        }
        BCM_PBMP_PORT_ADD(t_pbmp, port);
    } else {
        if (!BCM_PBMP_MEMBER(t_pbmp, port)){
            return BCM_E_NONE;
        }
        BCM_PBMP_PORT_REMOVE(t_pbmp, port);
    }
    
    BCM_PBMP_ASSIGN(egr_pbmp, t_pbmp);

    /* Set the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_SET
        (unit, menable, mtp_bmp, igr_pbmp, egr_pbmp));
    
    return BCM_E_NONE;
}   

/*
 * Function:
 *     bcm_robo_mirror_egress_get
 * Description:
 *     Get the mirroring per egress enabled/disabled status
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to check
 *  val - Place to store boolean return value for on/off
 * Returns:
 *     BCM_E_xxxx
 */
int 
bcm_robo_mirror_egress_get(int unit, bcm_port_t port, int *val)
{
    uint32  menable;
    bcm_pbmp_t  igr_pbmp, egr_pbmp, mtp_bmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_egress_get()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    MIRROR_INIT(unit);

     /* Get the current Mirror setting */    
    BCM_IF_ERROR_RETURN(DRV_MIRROR_GET
        (unit, &menable, &mtp_bmp, &igr_pbmp, &egr_pbmp));

    *val = (BCM_PBMP_MEMBER(egr_pbmp, port)) ? 1 : 0;
    
    return BCM_E_NONE;    
}
    
/*
 * Function:
 *  bcm_robo_mirror_pfmt_set
 * Description:
 *  Set the mirroring preserve format field
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  val - value for preserve format on/off
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_mirror_pfmt_set(int unit, int val)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_pfmt_set()..unavailable\n")));
    return BCM_E_UNAVAIL;
}   

/*
 * Function:
 *  bcm_robo_mirror_pfmt_get
 * Description:
 *  Set the mirroring preserve format field
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  val - (OUT) Value for preserve format on/off
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_mirror_pfmt_get(int unit, int *val)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_pfmt_get()..unavailable\n")));
    return BCM_E_UNAVAIL;
}   

/*
 * Function:
 *  bcm_robo_mirror_to_pbmp_set
 * Description:
 *  Set the mirror-to port bitmap for mirroring on a given port.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to affect (unused for ROBO chips)
 *  pbmp - The port bitmap of mirrored to ports for this port.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  This API interface is only supported on XGS fabric chips.
 */
int 
bcm_robo_mirror_to_pbmp_set(int unit, bcm_port_t port, bcm_pbmp_t pbmp)
{
    bcm_pbmp_t  t_mtp_bmp;
    bcm_port_t  p = 0;
    int  rv = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_to_pbmp_set()..\n")));

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* port is unused for ROBO, means all ingress/egress selections are mirrored to pbmp */
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }
    BCM_PBMP_ITER(pbmp, p) {
        if (!BCM_PBMP_MEMBER(PBMP_ALL(unit), p)) {
            return BCM_E_PARAM;
        }
    }

    /* Initialization check. */
    MIRROR_INIT(unit);

    /* backup the original mirror-to port bitmap status and need to recover back if return fails */
    BCM_PBMP_ASSIGN(t_mtp_bmp, MIRROR_INFO(unit).mirror_mtp_bmp);

    BCM_PBMP_ASSIGN(MIRROR_INFO(unit).mirror_mtp_bmp, pbmp);

    rv = bcm_robo_mirror_mode_set(unit, MIRROR_INFO(unit).mode);
    if (rv < 0) {
        /* recover back to  the original mirror-to port bitmap status */
        BCM_PBMP_ASSIGN(MIRROR_INFO(unit).mirror_mtp_bmp, t_mtp_bmp);
    }

    return rv;

}   

/*
 * Function:
 *  bcm_robo_mirror_to_pbmp_get
 * Description:
 *  Get the mirror-to port bitmap for mirroring on the
 *  specified port.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - The port to mirror all ingress/egress selections to (unused for ROBO chips)
 *  pbmp - (OUT) The port bitmap of mirror-to ports for this port.
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_robo_mirror_to_pbmp_get(int unit, bcm_port_t port, bcm_pbmp_t *pbmp)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_to_pbmp_get()..\n")));

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    /* Initialization check. */
    MIRROR_INIT(unit);

    /* port is unused for ROBO, means all ingress/egress selections are mirrored to pbmp */
    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port) || !IS_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    /* Return mirror software info. for mirror-to port bitmap */
    BCM_PBMP_ASSIGN(*pbmp, MIRROR_INFO(unit).mirror_mtp_bmp);

    return BCM_E_NONE;
}   


/*
 * Function:
 *      bcm_mirror_port_set
 * Description:
 *      Set mirroring configuration for a port
 * Parameters:
 *      unit      - BCM device number
 *      port      - port to configure
 *      dest_mod  - module id of mirror-to port
 *                  (-1 for local port)
 *      dest_port - mirror-to port
 *      flags     - BCM_MIRROR_PORT_* flags
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Setting BCM_MIRROR_PORT_ENABLE without setting _INGRESS or
 *      _EGRESS allows the port to participate in bcm_l2_cache matches
 *      with the BCM_L2_CACHE_MIRROR bit set, and to participate in
 *      bcm_field lookups with the mirror action set.
 *
 *      If bcmSwitchDirectedMirroring is disabled for the unit and
 *      dest_mod is non-negative, then the dest_mod path is looked
 *      up using bcm_topo_port_get.
 *      If bcmSwitchDirectedMirroring is enabled for the unit and
 *      dest_mod is negative, then the local unit's modid is used
 *      as the dest_mod.
 */
int
bcm_robo_mirror_port_set(int unit, bcm_port_t port,
                    bcm_module_t dest_mod, bcm_port_t dest_port,
                    uint32 flags)
{
    bcm_module_t  modid;
    bcm_trunk_t  tgid;
    int  id; 
    int rv = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_port_set()..\n")));

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port) || IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    MIRROR_INIT(unit);

    /* standalone */
    /* dest_mod = -1 for local port */
    if (dest_mod == -1) {
        MIRROR_INFO(unit).modid = 0;
    } else {
        MIRROR_INFO(unit).modid = dest_mod;
    }

    if (flags & (BCM_MIRROR_PORT_DEST_TRUNK | 
        BCM_MIRROR_PORT_EGRESS_TRUE)) {
        return BCM_E_PARAM;
    }
    
    /* Disable mirroring if flags is zero */
    if ((flags & (BCM_MIRROR_PORT_INGRESS | 
                  BCM_MIRROR_PORT_EGRESS | 
                  BCM_MIRROR_PORT_ENABLE)) == 0) {
        BCM_IF_ERROR_RETURN
                 (bcm_mirror_ingress_set(unit, port, 0));
        BCM_IF_ERROR_RETURN
                 (bcm_mirror_egress_set(unit, port, 0));
        return BCM_E_NONE;
    }

    /* Gport resolution */
    if (BCM_GPORT_IS_SET(dest_port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, dest_port, &modid, &dest_port, &tgid, &id));
        if ((-1 != tgid) || (-1 != id)){
            return BCM_E_PARAM;
        }
    } else {
        if (!SOC_PORT_VALID(unit, dest_port)) { 
            return BCM_E_PORT; 
        }
    }

    BCM_IF_ERROR_RETURN
             (bcm_mirror_to_set(unit, dest_port));

    BCM_IF_ERROR_RETURN
             (bcm_mirror_ingress_set(unit, port, 
                         flags & BCM_MIRROR_PORT_INGRESS));

    BCM_IF_ERROR_RETURN
            (bcm_mirror_egress_set(unit, port, 
                        flags & BCM_MIRROR_PORT_EGRESS));
    
    return rv;
    
}

/*
 * Function:
 *      bcm_mirror_port_get
 * Description:
 *      Get mirroring configuration for a port
 * Parameters:
 *      unit      - BCM device number
 *      port      - port to get configuration for
 *      dest_mod  - (OUT) module id of mirror-to port
 *      dest_port - (OUT) mirror-to port
 *      flags     - (OUT) BCM_MIRROR_PORT_* flags
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mirror_port_get(int unit, bcm_port_t port,
                    bcm_module_t *dest_mod, bcm_port_t *dest_port,
                    uint32 *flags)
{
    int enable;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_mirror_port_get()..\n")));

    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(bcm_port_local_get(unit, port, &port));
    }

    /* Input parameters check. */
    if (!SOC_PORT_VALID(unit, port) || IS_CPU_PORT(unit, port)) {
        return BCM_E_PORT;
    }

    MIRROR_INIT(unit);

    /* standalone */
    *dest_mod = MIRROR_INFO(unit).modid;

    BCM_IF_ERROR_RETURN
          (bcm_mirror_to_get(unit, dest_port));

    *flags = 0;
    if (bcm_mirror_ingress_get(unit, port, &enable) == BCM_E_NONE && enable) {
        *flags |= BCM_MIRROR_PORT_INGRESS;
    }
    if (bcm_mirror_egress_get(unit, port, &enable) == BCM_E_NONE && enable) {
        *flags |= BCM_MIRROR_PORT_EGRESS;
    } 

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_mirror_vlan_set
 * Description:
 *      Set VLAN for egressing mirrored packets on a port (RSPAN)
 * Parameters:
 *      unit    - device number
 *      port    - mirror-to port to set (-1 for all ports)
 *      tpid    - tag protocol id (0 to disable)
 *      vlan    - virtual lan number (0 to disable)
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mirror_vlan_set(int unit, bcm_port_t port,
                    uint16 tpid, uint16 vlan)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_mirror_vlan_get
 * Description:
 *      Get VLAN for egressing mirrored packets on a port (RSPAN)
 * Parameters:
 *      unit    - device number
 *      port    - mirror-to port for which to get tag info
 *      tpid    - (OUT) tag protocol id
 *      vlan    - (OUT) virtual lan number
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_robo_mirror_vlan_get(int unit, bcm_port_t port,
                    uint16 *tpid, uint16 *vlan)
{
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *     bcm_robo_mirror_port_dest_add 
 * Purpose:
 *      Add mirroring destination to a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_robo_mirror_port_dest_add(int unit, bcm_port_t port, 
                              uint32 flags, bcm_gport_t mirror_dest) 
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_mirror_port_dest_delete
 * Purpose:
 *      Remove mirroring destination from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 *      mirror_dest  -  (IN) Mirroring destination gport. 
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_robo_mirror_port_dest_delete(int unit, bcm_port_t port, 
                                uint32 flags, bcm_gport_t mirror_dest) 
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_mirror_port_dest_delete_all
 * Purpose:
 *      Remove all mirroring destinations from a port. 
 * Parameters:
 *      unit         -  (IN) BCM device number. 
 *      port         -  (IN) Port mirrored port.
 *      flags        -  (IN) BCM_MIRROR_PORT_XXX flags.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_robo_mirror_port_dest_delete_all(int unit, bcm_port_t port, uint32 flags) 
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_mirror_port_dest_get
 * Purpose:
 *     Get port mirroring destinations.   
 * Parameters:
 *     unit             - (IN) BCM device number. 
 *     port             - (IN) Port mirrored port.
 *     flags            - (IN) BCM_MIRROR_PORT_XXX flags.
 *     mirror_dest_size - (IN) Preallocated mirror_dest array size.
 *     mirror_dest      - (OUT)Filled array of port mirroring destinations
 *     mirror_dest_count - (OUT)Actual number of mirroring destinations filled.
 * Returns:
 *      BCM_X_XXX
 */
int
bcm_robo_mirror_port_dest_get(int unit, bcm_port_t port, uint32 flags, 
                              int mirror_dest_size, bcm_gport_t *mirror_dest,
                              int *mirror_dest_count)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_mirror_destination_create
 * Purpose:
 *     Create mirror destination description.
 * Parameters:
 *      unit         - (IN) BCM device number. 
 *      mirror_dest  - (IN) Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_robo_mirror_destination_create(int unit, bcm_mirror_destination_t *mirror_dest) 
{
    return (BCM_E_UNAVAIL);
}


/*
 * Function:
 *     bcm_robo_mirror_destination_destroy
 * Purpose:
 *     Destroy mirror destination description.
 * Parameters:
 *      unit            - (IN) BCM device number. 
 *      mirror_dest_id  - (IN) Mirror destination id.
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_robo_mirror_destination_destroy(int unit, bcm_gport_t mirror_dest_id) 
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_mirror_destination_get
 * Purpose:
 *     Get mirror destination description.
 * Parameters:
 *      unit            - (IN) BCM device number. 
 *      mirror_dest_id  - (IN) Mirror destination id.
 *      mirror_dest     - (OUT)Mirror destination description.
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_robo_mirror_destination_get(int unit, bcm_gport_t mirror_dest_id, 
                                   bcm_mirror_destination_t *mirror_dest)
{
    return (BCM_E_UNAVAIL);
}

/*
 * Function:
 *     bcm_robo_mirror_destination_traverse
 * Purpose:
 *     Traverse installed mirror destinations
 * Parameters:
 *      unit      - (IN) BCM device number. 
 *      cb        - (IN) Mirror destination traverse callback.         
 *      user_data - (IN) User cookie
 * Returns:
 *      BCM_X_XXX
 */
int 
bcm_robo_mirror_destination_traverse(int unit, bcm_mirror_destination_traverse_cb cb, 
                                    void *user_data) 
{
    return (BCM_E_UNAVAIL);
}
