/*
 * $Id: port.h,v 1.37 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains PORT definitions internal to the BCM library.
 */

#ifndef _BCM_INT_PORT_H
#define _BCM_INT_PORT_H

#include <bcm/port.h>
#include <bcm/vlan.h>

/* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT is for SW workaround for all GE PHY's 
 *  link problem with Inter's NIC and Marvel's PHY when the PHY is at 
 *  Auto-PowerDown(AUTO_PD) mode.
 *  - This WAR is designed for the GE PHY at Auto-PD mode and the link 
 *      is not established condition.
 *  - The Link problem only occurred when GE PHY's Auto-PD enabled and the
 *      the link partner is Inter-NIC or Marvel_PHY.
 *  - Auto-PD is not a IEEE Spec. The interoperability with other vendors 
 *      is not guaranteed.
 */
#define BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT    1
#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT
#include <sal/core/time.h>
#include <sal/core/sync.h>

/* SW database for port based GE PHY's Auto-PowerDown(AUTO_PD) workaround */
typedef struct bcm_robo_port_phy_auto_pd_s
{
    int             war_enable;     /* TRUE means AUTO_PD is enabled */
    int             current_link;
    int             current_auto_pd_mode;
    sal_time_t      last_soc_time; /* seconds */ 
    sal_mutex_t     auto_pd_war_lock;
} bcm_robo_port_phy_auto_pd_t;

/* Time frame for PHY Auto-PowerDown(AUTO_PD) workaround :
 *  - AUTO_PD will be enabled or disabled in every time frame when the link 
 *      is not established.
 *  - Two time frame defined for the wait time for link at Auto-PD power 
 *      and Full-Power condition
 */
#define BCM_ROBO_PHY_AUTO_PD_WAR_FULL_TIME_FRAME    10  /* seconds */
#define BCM_ROBO_PHY_AUTO_PD_WAR_PD_TIME_FRAME      10  /* seconds */

extern int _bcm_robo_port_phy_auto_pd_war(int unit, 
                bcm_port_t port, int link);
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */

/*
 * For MAC low power mode, it takes 1.5 second from phy energy detect to link up.
 * So we define a waiting time to make sure all ports are link-down. 
 * It should be large than 1.5 seconds.
 * When all ports are link-down, it need to wait until the time is expired.
 * Then, enable the MAC low power mode.
 */
#define BCM_ROBO_MAC_LOW_POWER_WAITING_TIME 2 /* seconds */

#define BCM_ROBO_MAC_ULTRA_LOW_POWER_WAITING_TIME 600 /* seconds */


#if defined(BCM_53101) || defined(BCM_POLAR_SUPPORT)
/* SW database to save the phy configuration. */
typedef struct bcm_robo_phy_cfg_s
{
    int autoneg; 
    int speed; 
    int duplex; 
    bcm_port_abil_t local_advert;
} bcm_robo_phy_cfg_t;

extern int _bcm_robo53101_phy_cfg_save(int unit);

#endif /* BCM_53101 || BCM_POLAR_SUPPORT */

/*
 * Internal port configuration enum.
 * Used internally by sdk to set port configuration from 
 */
typedef enum _bcm_port_config_e {
   _bcmPortUseInnerPri,         /* Trust the inner vlan tag PRI bits  */
   _bcmPortUseOuterPri,         /* Trust the outer vlan tag PRI bits  */
   _bcmPortCount                /* Always last max value of the enum. */
} _bcm_port_config_t;

/*
 * Port configuration structure.
 * An abstraction of data in the PTABLE (robo) and other places.
 */

typedef struct bcm_port_cfg_s 
{
    int     pc_frame_type;
    int     pc_ether_type;

    int     pc_stp_state;     /* spanning tree state of port */
    int     pc_cpu;       /* CPU learning */
    int     pc_disc;      /* discard state */
    int     pc_bpdu_disable;  /* Where is this in Draco? */
    int     pc_trunk;     /* trunking on for this port */
    int     pc_tgid;      /* trunk group id */
    int     pc_mirror_ing;    /* mirror on ingress */
    int     pc_mirror_eng;    /* mirror on engress */
    int     pc_ptype;     /* port type */
    int     pc_jumbo;
    int     pc_cml;       /* CML bits */

    bcm_pbmp_t  pc_pbm;       /* port bitmaps for port based vlan */
    bcm_pbmp_t  pc_ut_pbm;
    bcm_vlan_t  pc_vlan;      /* port based vlan tag */
    bcm_vlan_t	pc_ivlan;	  /* port based inner-tag vlan tag */

    int     pc_new_pri;   /* new priority */
    int     pc_remap_pri_en;  /* remap priority enable */
    
    int     pc_dse_mode;      /* DSE mode ? */
    int     pc_dscp;      /* diffserv code point */

#if BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT
    bcm_robo_port_phy_auto_pd_t *ge_phy_auto_pd_war_info;
#endif  /* BCM_ROBO_PHY_AUTO_PD_WAR_SUPPORT */

    uint32 pc_dt_mode;
    uint32 pc_dt_tpid;

#if defined(BCM_53101) || defined(BCM_POLAR_SUPPORT)
    bcm_robo_phy_cfg_t phy_cfg;   
#endif /* BCM_53101 || BCM_POLAR_SUPPORT */
} bcm_port_cfg_t;

typedef struct _bcm_gport_dest_s {
    bcm_port_t      port;
    bcm_module_t    modid;
    bcm_trunk_t     tgid;
    int             mpls_id;
    int             mim_id;
    int             subport_id;
    int             scheduker_id;
    uint32          gport_type;
} _bcm_gport_dest_t;

extern int _bcm_robo_port_software_init(int unit);
extern int _bcm_robo_port_link_get(int unit, 
                bcm_port_t port, int hw, int *link);
extern int _bcm_robo_port_untagged_data_update(int unit, bcm_pbmp_t pbmp);
extern int _bcm_robo_port_untagged_vlan_get(int unit, bcm_port_t port,
                       bcm_vlan_t *vid_ptr);
extern int _bcm_robo_port_config_set(int unit, bcm_port_t port, 
                                    _bcm_port_config_t type, int value);
extern int _bcm_robo_port_config_get(int unit, bcm_port_t port, 
                                    _bcm_port_config_t type, int *value);
extern void _bcm_robo_gport_dest_t_init(_bcm_gport_dest_t *gport_dest);
extern int _bcm_robo_gport_construct(int unit, _bcm_gport_dest_t *gport_dest, 
                                    bcm_gport_t *gport);
extern int _bcm_robo_port_gport_validate(int unit, bcm_port_t port_in, 
                                    bcm_port_t *port_out);
extern int _bcm_robo_gport_resolve(int unit, bcm_gport_t gport,
                                  bcm_module_t *modid, bcm_port_t *port, 
                                  bcm_trunk_t *trunk_id, int *id);
extern int _bcm_robo_gport_modport_hw2api_map(int, bcm_module_t, bcm_port_t,
                                         bcm_module_t *, bcm_port_t *);
extern int _bcm_robo_gport_modport_api2hw_map(int, bcm_module_t, bcm_port_t,
                                         bcm_module_t *, bcm_port_t *);
extern int _bcm_robo_modid_is_local(int unit, 
                                    bcm_module_t modid, int *result);

extern int _bcm_robo_vlan_port_protocol_action_add(int unit,
                                      bcm_port_t port,
                                      bcm_port_frametype_t frame,
                                      bcm_port_ethertype_t ether,
                                      bcm_vlan_action_set_t *action);
extern int _bcm_robo_port_deinit(int unit);
                                      
#define TOS_DIFFSERV_EN_ADDR    soc_reg_addr(unit, QOS_TOSDIF_ENr, 0, 0)
#define TOS_DIFFSERV_CTRL_ADDR  soc_reg_addr(unit, QOS_TOSDIF_CTRLr, 0, 0)
#define DSCP_PRIORITY1_ADDR soc_reg_addr(unit, QOS_DIFF_DSCP1r, 0, 0)
#define DSCP_PRIORITY2_ADDR soc_reg_addr(unit, QOS_DIFF_DSCP2r, 0, 0)

#define TTOS_PRIORITY_ADDR  soc_reg_addr(unit, QOS_TTOS_THRSHr, 0, 0)
#define DTOS_PRIORITY_ADDR  soc_reg_addr(unit, QOS_DTOS_THRSHr, 0, 0)
#define MTOS_PRIORITY_ADDR  soc_reg_addr(unit, QOS_MTOS_THRSHr, 0, 0)
#define RTOS_PRIORITY_ADDR  soc_reg_addr(unit, QOS_RTOS_THRSHr, 0, 0)

#define _BCM_DSCP_CODE_POINT_VALID(dscp) ((dscp) >= 0 && (dscp < 64))

/* egress port priority color Macro : for bcm53115/TB so far */
#define _BCM_ROBO_COLOR_NONE    0 /*treat as inband for BCM53115 */

/* bcm53115 color encoding rule
 *  None bcm53115 : _BCM_ROBO_COLOR_NONE
 *  Green : inband (0)
 *  Yellow : outband (1)
 *  Red : outband (1)
 */
#define _COLOR_ENCODING(color) \
        (((color) == bcmColorGreen) ?  0:\
        ((color) == bcmColorYellow) ?  1:\
        ((color) == bcmColorRed) ? 1 : _BCM_ROBO_COLOR_NONE)

        
/* TB color encoding rule 
 *  Green : (DP1)
 *  Yellow :  (DP2)
 *  Red : (DP3)
 *  Preserve : (DP0)
 */
#define _TB_COLOR_ENCODING(color) \
        (((color) == bcmColorPreserve) ? 0 :\
        ((color) == bcmColorGreen) ?  1:\
        ((color) == bcmColorYellow) ?  2:\
        ((color) == bcmColorRed) ? 3 : 1)

#define _TB_COLOR_DECODING(hw_color) \
        ((hw_color) == 0 ? bcmColorPreserve :\
        (hw_color) == 1 ? bcmColorGreen :\
        (hw_color) == 2 ? bcmColorYellow :\
        (hw_color) == 3 ? bcmColorRed : bcmColorGreen)

/* NorthstarPlus encoding rule
 * Green : 0
 * Yellow : 1
 * Red : 2
 */

#define _NSP_COLOR_ENCODING(color) \
        (((color) == bcmColorGreen) ?  0:\
        ((color) == bcmColorYellow) ?  1:\
        ((color) == bcmColorRed) ? 2 : 0)

#define _NSP_COLOR_DECODING(hw_color) \
        ((hw_color) == 0 ? bcmColorGreen :\
        (hw_color) == 1 ? bcmColorYellow :\
        (hw_color) == 2 ? bcmColorRed : bcmColorGreen)



#define _BCM_COLOR_ENCODING(unit, color) \
        ((SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || \
          SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit)) ? \
            _COLOR_ENCODING(color) : SOC_IS_TBX(unit) ?\
            _TB_COLOR_ENCODING(color) : (SOC_IS_NORTHSTARPLUS(unit) || \
            SOC_IS_STARFIGHTER3(unit)) ?\
            _NSP_COLOR_ENCODING(color) : _BCM_ROBO_COLOR_NONE)


#define _BCM_COLOR_DECODING(unit, hw_color) \
        (SOC_IS_TBX(unit) ? \
        _TB_COLOR_DECODING(hw_color) : (SOC_IS_NORTHSTARPLUS(unit) || \
        SOC_IS_STARFIGHTER3(unit)) ?\
        _NSP_COLOR_ENCODING(hw_color) : _BCM_ROBO_COLOR_NONE)

extern void
_bcm_robo_dtag_mode_sw_update(int unit, bcm_port_t port, int is_dtag);

extern int
_bcm_robo_pbmp_check(int unit, bcm_pbmp_t pbmp);

#endif  /* !_BCM_INT_PORT_H */
