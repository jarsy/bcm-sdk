/*
 * $Id: gport_mgmt.h,v 1.71 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        gport_mgmt.h
 * Purpose:     GPORT Management internal definitions to the BCM library.
 */

#ifndef  INCLUDE_DNX_GPORT_MGMT_H
#define  INCLUDE_DNX_GPORT_MGMT_H

#include <bcm/types.h>
/*#include <bcm/vlan.h>*/
#include <bcm/mpls.h>
#include <bcm/mim.h>
#include <bcm/trill.h>
#include <bcm/tunnel.h>
#include <bcm/extender.h>

#include <soc/dnx/legacy/TMC/tmc_api_general.h>

#include <shared/hash_tbl.h>
#include <shared/swstate/sw_state.h>
#include <shared/swstate/sw_state_hash_tbl.h>


/* Number of LIFs as gports */
#define _BCM_DNX_NOF_GPORTS(unit) ((SOC_DNX_CONFIG(unit))->l2.nof_lifs)

/* 
 * Gport resolution type information,
 * Used to retreive Forwarding Information of a given GPORT
 * Note: Not all logical gports have forwarding information 
 * (for example when GPORT is not expected to be located in Forwarding databases) 
 */ 

#define _BCM_DNX_GPORT_RESOLVE_TYPE_IS_FEC(resolve_type) ((resolve_type==_bcmDnxGportResolveTypeProtectedFec) || (resolve_type==_bcmDnxGportResolveTypeForwardFec))

typedef enum _bcm_dnx_gport_resolve_type_s {
    _bcmDnxGportResolveTypeInvalid=0, /* invalid gport type or not initialized */
    _bcmDnxGportResolveTypeProtectedFec=1, /* gport-id includes FEC only, FEC usually includes: port + EEP */
    _bcmDnxGportResolveTypeAC=2, /* gport is AC-OutLIF: phy = physical port + encap-id = AC */
    _bcmDnxGportResolveTypeMC=3, /* gport is MC, usually for 1+1 protection */
    _bcmDnxGportResolveTypePhy=4, /* gport is only physical port, encap_id is not relevant */
    _bcmDnxGportResolveTypeFecVC=5, /* gport is FEC+VC (For PWE application): phy = FEC + encap-id = VC */
    _bcmDnxGportResolveTypePhyEEP=6, /* gport is Port+OutLIF (EEP) for PWE: phy = physical port + encap-id = EEP */
    _bcmDnxGportResolveTypeFecEEP=7, /* gport is FEC+OutLIF (EEP) for PWE: phy = FEC + encap-id = EEP */
    _bcmDnxGportResolveTypeMimMP=8, /* gport is FEC for MIM MP: phy = B-SA LSB + encap-id = {B-SA MSB,B-VID} */
    _bcmDnxGportResolveTypeTrillMC=9, /* gport is MC for trill Multicast */
    _bcmDnxGportResolveTypeMimP2P=10, /* gport is FEC for MIM P2P: phy = LIF + encap-id = B-VID */
    _bcmDnxGportResolveTypeIPTunnel=11, /* gport is out-lif/EEP for ip-tunnel: phy = physical port + encap-id = EEP */
    _bcmDnxGportResolveTypeL2Gre=12, /* l2gre gport: gport is in-lif for ip-tunnel: phy = physical port + encap-id = out-LIF */
    _bcmDnxGportResolveTypeL2GreEgFec=13, /* l2gre gport: gport is in-lif for ip-tunnel: phy = FEC + encap-id = out-LIF */
    _bcmDnxGportResolveTypeVxlan=14, /* vxlan gport: gport is in-lif for ip-tunnel: phy = physical port + encap-id = out-LIF */
    _bcmDnxGportResolveTypeVxlanEgFec=15, /* vxlan gport: gport is in-lif for ip-tunnel: phy = FEC + encap-id = out-LIF */
    _bcmDnxGportResolveTypeRing=16, /* ring gport for AC: phy = FEC + encap-id */
    _bcmDnxGportResolveTypeForwardFec=17 /* gport-id includes FEC only, not used for protection */
} _bcm_dnx_gport_resolve_type_e;

/* 
 * Struct present the forwarding information of a given GPORT 
 * Including: 
 * - _bcm_dnx_gport_resolve_type_e - The forwarding information type 
 * - phy_gport - The physical information of a gport. May be for example destination-port,trunk or FEC. 
 *    According to _bcm_dnx_gport_resolve_type_e.
 * - encap_id - The encapsulation information of a gport. May be for example OutLIF, EEI or additional information required by a specific application. 
 *    According to _bcm_dnx_gport_resolve_type_e.
 */
typedef struct {
    _bcm_dnx_gport_resolve_type_e type;
    int phy_gport;/* physical information, according to resolve type */
    int encap_id;/* encapsulation information or additional info , according to resolve type */
} DNX_BCM_GPORT_PHY_PORT_INFO;

/* per LIF what is match key */

/* Flags for _bcm_dnx_sw_resources_inlif_t */
#define _BCM_DNX_INFLIF_MATCH_INFO_MPLS_PORT_NO_OUTLIF      (0x1)
#define _BCM_DNX_INFLIF_MATCH_INFO_P2P                      (0x2)

/* Flags for _bcm_dnx_sw_resources_outlif_t */
#define _BCM_DNX_OUTLIF_MATCH_INFO_MPLS_EXTENDED_LABEL     (0x1)
#define _BCM_DNX_OUTLIF_IP_TUNNEL_MATCH_INFO_GRE_WITH_LB   (0x2)

#define GPORT_HASH_VLAN_NOT_FOUND(phy_port,rv)                    BCM_FAILURE(rv)

#define _BCM_DNX_LIF_TO_GPORT_INGRESS       0x02
#define _BCM_DNX_LIF_TO_GPORT_EGRESS        0x04
#define _BCM_DNX_LIF_TO_GPORT_GLOBAL_LIF    0x08
#define _BCM_DNX_LIF_TO_GPORT_LOCAL_LIF     0x10

typedef struct {
    int             dummy;
}bcm_dnx_gport_mgmt_info_t;
#define _BCM_GPORT_ENCAP_ID_LIF_INVALID  (-1)

/* 
 * MACROS
 */

/* 
 * Functions 
 */

/*
 * Function:
 *      _bcm_dnx_gport_mgmt_init
 * Purpose:
 *      init Gport module
 * Parameters:
 * Note:
 * Returns:
 *      BCM_E_XXX
 */

int
_bcm_dnx_gport_mgmt_init(int                     unit);

extern int
_bcm_dnx_gport_mgmt_sw_state_cleanup(int                     unit);


/* 
 * Gport compare type information,
 * Used to specify the criteria for gport compare
 */ 
typedef enum _bcm_dnx_gport_compare_type_s {
    _bcmDnxGportCompareTypeSystemPort=0, /* compare system port */
    _bcmDnxGportCompareTypeInLif=1, /* compare Global In Lif */
    _bcmDnxGportCompareTypeOutLif=2 /* compare Global Out Lif */
} _bcm_dnx_gport_compare_type_e;

/*
 * Function:
 *    _bcm_dnx_gport_compare
 * Description:
 *    compare two gports
 * Parameters:
 *    unit -          [IN] DNX device unit number (driver internal).
 *  gport1 -          [IN] general port 1
 *  gport2 -          [IN] general port 2 
 *  is_equal -         [OUT] result of comparison
 * Returns:
 *    BCM_E_XXX
 */
int 
_bcm_dnx_gport_compare(int unit, bcm_gport_t gport1, bcm_gport_t gport2, _bcm_dnx_gport_compare_type_e type, uint8 * is_equal);

/*
 * given vpn and iter return next gport-value in vpn 
 * caller still need to call  
 */
int _bcm_dnx_vpn_get_next_gport(
    int                  unit,
    bcm_vpn_t            vpn, /* if -1 then get next gport belongs to any vsi */
    int                  *iter,
    bcm_gport_t          *port_val,
    int                  *cur_vsi /* the vsi of the return LIF*/
 );

/* Gport parse type */
typedef enum _bcm_dnx_gport_parse_type_s {
    _bcmDnxGportParseTypeProtectedFec=0, /* Fec used for bundle protection */       
    _bcmDnxGportParseTypeEncap, /* LIF ID */    
    _bcmDnxGportParseTypeMC, /* Multicast-ID */      
    _bcmDnxGportParseTypeSimple, /* Simple , physical */
    _bcmDnxGportParseTypeForwardFec, /* Fec used for forwarding and not protected */
    _bcmDnxGportParseTypePushProfile /* Push-profile ID, for PWE application*/
} _bcm_dnx_gport_parse_type_e;

/* Additional information on parsing */
#define _BCM_DNX_GPORT_PARSE_PRIMARY      (1 << 0) /* May be valid only when type is MC */
#define _BCM_DNX_GPORT_PARSE_SECONDARY    (1 << 1) /* May be valid only when type is MC */
#define _BCM_DNX_GPORT_PARSE_INGRESS_ONLY (1 << 2) /* May be valid only when type is ENCAP */
#define _BCM_DNX_GPORT_PARSE_EGRESS_ONLY  (1 << 3) /* May be valid only when type is ENCAP */

typedef struct _bcm_dnx_gport_parse_info_s {
    _bcm_dnx_gport_parse_type_e type; /* type of the object */
    int val; /* Object ID */
    uint32     flags; /* Additional information on object: _BCM_DNX_GPORT_PARSE_XXX */    
} _bcm_dnx_gport_parse_info_t;
/* 
 *  Function:
 *    _bcm_dnx_gport_parse
 * Description:
 *  Parse gport-id encoding to indicate the type of the object and its value.
 *  Done only according to gport-id encoding (without any SW DB).
 *  Input: gport 
 *  Output: Structure gport_parse includes:
 *  Type: _bcmDnxGportParseTypeProtectedFec, Val: FEC-ID, Flags: None
 *  Type: _bcmDnxGportParseTypeForwardFec,   Val: FEC-ID, Flags: None
 *  Type: _bcmDnxGportParseTypeMC,           Val: MC-ID,  Flags: Primary/Secondary
 *  Type: _bcmDnxGportParseTypeEncap,        Val: LIF-ID, Flags: Ingress/Egress Only
 *  Type: _bcmDnxGportParseTypeSimple        Val: physical-port
 *  Type: _bcmDnxGportParseTypePushProfile   Val: Push-profile ID
 *  Note:
 *    For more encoding information of gport see drv.h
 */
int _bcm_dnx_gport_parse(int unit, bcm_gport_t port, _bcm_dnx_gport_parse_info_t *gport_parse_info);

/* if set then don't check if gport already exist, should be in construction */
#define _BCM_DNX_GPORT_RESOLVE_FLAG_NO_CHECK (0x1)

typedef struct _bcm_dnx_gport_info_s {
    bcm_pbmp_t pbmp_local_ports;
    bcm_port_t local_port; /* one of the ports in pbmp_local_ports*/
    uint32     sys_port;
    uint32     flags;
    int        lane;
    int        phyn;
} _bcm_dnx_gport_info_t;

#define _BCM_DNX_GPORT_INFO_F_IS_LOCAL_PORT 0x01
#define _BCM_DNX_GPORT_INFO_F_IS_LAG        0x02
#define _BCM_DNX_GPORT_INFO_F_IS_SYS_SIDE   0x08
#define _BCM_DNX_GPORT_INFO_F_IS_BLACK_HOLE 0x10

#define _BCM_DNX_GPORT_INFO_IS_LOCAL_PORT(gport_info) (gport_info.flags & _BCM_DNX_GPORT_INFO_F_IS_LOCAL_PORT ? 1: 0)
#define _BCM_DNX_GPORT_INFO_IS_LAG(gport_info) (gport_info.flags & _BCM_DNX_GPORT_INFO_F_IS_LAG ? 1: 0)
#define _BCM_DNX_GPORT_INFO_IS_SYS_SIDE(gport_info) (gport_info.flags & _BCM_DNX_GPORT_INFO_F_IS_SYS_SIDE ? 1: 0)

/*
 * Function:
 *    _bcm_dnx_gport_to_phy_port
 * Description:
 *    map gport to PPD local and system port
 * Parameters:
 *    unit  -       [IN] DNX device unit number (driver internal).
 *    gport -       [IN] general port
 *    operations -  [IN] see _BCM_DNX_GPORT_TO_PHY_OP_...
 *    gport_info -  [OUT] Retrive information
 * Returns:
 *    BCM_E_XXX
 */

#define _BCM_DNX_GPORT_TO_PHY_OP_RETRIVE_SYS_PORT    0x1
#define _BCM_DNX_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY  0x2
int 
_bcm_dnx_gport_to_phy_port(int unit, bcm_gport_t gport, uint32 operations, _bcm_dnx_gport_info_t* gport_info);

/* given gport, return if it's local in this unit */
int 
_bcm_dnx_gport_is_local(int unit, bcm_gport_t port,  int *is_local);


/*
 * FORWARDING INFORMATION FUNCTIONS
 */

/*
 * Function:
 *       _bcm_dnx_gport_to_port_encap
 * Description:
 *       map gport to forwarding info <phy-port (can be FEC) and encap that store LIF or EEI)
 * Parameters:
 *       unit -           [IN] DNX device unit number (driver internal).
 *  gport -          [IN] general port
 *  dest_port -      [OUT] dest_port is physical TM port or FEC. i.e. pp-destination
 *  encap_id -       [OUT] encap-info includes eei or outlif
 * Returns:
 *       BCM_E_XXX
 */int 
_bcm_dnx_gport_to_port_encap(int unit, bcm_gport_t gport, int *dest_port, int *encap_id);

/*
 * Function:
 *    _bcm_dnx_gport_to_fwd_decision
 * Description:
 *    convert gport to forwardubg decision (destination + editting information)
 * Parameters:
 *    unit -           [IN] DNX device unit number (driver internal).
 *  gport -          [IN] general port
 *  encap_id -       [IN] encap_id considered only if gport is logical port
 *  fwd_decsion -    [OUT] PPD forwarding decision
 * Returns:
 *    BCM_E_XXX
 */
int 
_bcm_dnx_gport_to_tm_dest_info(int unit, bcm_gport_t gport, DNX_TMC_DEST_INFO  *soc_dnx_dest_info);

/*
 * Function:
 *    _bcm_dnx_gport_to_tm_dest_info
 * Description:
 *    convert gport from TM dest information
 * Parameters:
 *  unit -           [IN] DNX device unit number (driver internal).
 *  gport -          [OUT] general port
 *  soc_dnx_dest_info - [OUT] Soc_dnx destination info
 * Returns:
 *    BCM_E_XXX
 */
int 
_bcm_dnx_gport_from_tm_dest_info(int unit, bcm_gport_t *gport, DNX_TMC_DEST_INFO  *soc_dnx_dest_info);


#endif /* INCLUDE_DNX_GPORT_MGMT_H */

