/*
 * $Id: mpls.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _BCM_INT_SBX_CALADAN3_MPLS_H_
#define _BCM_INT_SBX_CALADAN3_MPLS_H_
#if 0
#include <bcm_int/sbx/fe2000/l3.h>
#include <bcm_int/sbx/fe2000/qos.h>
#endif
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/common/trunk.h>
#include <bcm/mpls.h>
#include <bcm/trunk.h>
#include <soc/sbx/sbDq.h>

/* Caladan3 g3p1 supports 16bits of mpls label for vsi matching.
 * However, the MSb signals to the PPE that the label is a PWE3 label
 */
#define _CALADAN3_MPLS_LABEL(_lbl) ((_lbl) & _CALADAN3_L3_MPLS_LBL_MASK)
#define _CALADAN3_MPLSTP_LABEL(_lbl) ((_lbl) & _CALADAN3_L3_MPLSTP_LBL_MASK)

#define _CALADAN3_MPLS_PORT_FTE_VALID(unit, fte)             \
      (((fte) >= SBX_GLOBAL_GPORT_FTE_BASE(unit) &&      \
        (fte) <= SBX_GLOBAL_GPORT_FTE_END(unit))     ||  \
       ((fte) >= SBX_LOCAL_GPORT_FTE_BASE(unit)  &&      \
        (fte) <= SBX_LOCAL_GPORT_FTE_END(unit))      ||  \
       ((SOC_SBX_CFG(unit)->mplstp_ena) &&               \
        (fte >= SBX_VPWS_UNI_FTE_BASE(unit)) &&          \
        (fte <= SBX_VPWS_UNI_FTE_END(unit)))         ||  \
       ((SOC_SBX_CFG(unit)->mplstp_ena) &&               \
        (fte >= SBX_DYNAMIC_FTE_BASE(unit)) &&           \
        (fte <= SBX_DYNAMIC_FTE_END(unit)))          ||  \
       ((fte) >= SBX_DYNAMIC_VSI_FTE_BASE(unit) &&       \
        (fte) <= SBX_DYNAMIC_VSI_FTE_END(unit))      ||  \
       ((SOC_SBX_CFG(unit)->mplstp_ena) &&               \
        (fte >= SBX_PW_FO_FTE_BASE(unit)) &&             \
        (fte <= SBX_PW_FO_FTE_END(unit))))
       
/*
 *  Unhappily, while the create API for ExpMap has a parameter that says the
 *  map is ingress or egress, the delete API does not have such a thing.  So,
 *  we need to add some additional data to the map ID that's passed back so we
 *  can tell the difference.  Of course, we'll need to mask it off when writing
 *  it to the proper places, as well.  Much work for little omission.
 */
/*
#define _MPLS_EXPMAP_HANDLE_TYPE_MASK 0xFFF00000
#define _MPLS_EXPMAP_HANDLE_TYPE_INGR 0x1E300000
#define _MPLS_EXPMAP_HANDLE_TYPE_EGR  0xEE300000
#define _MPLS_EXPMAP_HANDLE_TYPE(h) (_MPLS_EXPMAP_HANDLE_TYPE_MASK & (h))
*/
#define _MPLS_EXPMAP_HANDLE_DATA(h) \
    ((_MPLS_EXPMAP_HANDLE_IS_INGR(h)) ? (h) : ((h) - QOS_MAP_ID_EGRESS_OFFSET))

#define _MPLS_EXPMAP_HANDLE_IS_INGR(h) ((h) < QOS_MAP_ID_EGRESS_OFFSET)
#define _MPLS_EXPMAP_HANDLE_IS_EGR(h) ((h) >= QOS_MAP_ID_EGRESS_OFFSET)

#define _MPLS_EXPMAP_HANDLE_IS_VALID(h) (_MPLS_EXPMAP_HANDLE_IS_EGR(h) || \
                                         _MPLS_EXPMAP_HANDLE_IS_INGR(h))
#define _MPLS_EXPMAP_HANDLE_MAKE_INGR(h) \
              (_bcm_caladan3_qos_hw_id_to_map_id(BCM_QOS_MAP_INGRESS, h))
#define _MPLS_EXPMAP_HANDLE_MAKE_EGR(h) \
              (_bcm_caladan3_qos_hw_id_to_map_id(BCM_QOS_MAP_EGRESS, h))


#define _BCM_CALADAN3_IS_PWE3_TUNNEL(flag) (((flag) & BCM_MPLS_PORT_EGRESS_TUNNEL) || \
                                          ((flag) & BCM_MPLS_PORT_NO_EGRESS_TUNNEL_ENCAP))

typedef enum {
    _BCM_CALADAN3_LABEL_OPCODE_PWE,
    _BCM_CALADAN3_LABEL_OPCODE_CES,        
    _BCM_CALADAN3_LABEL_OPCODE_LER,
    _BCM_CALADAN3_LABEL_OPCODE_LSR,
    _BCM_CALADAN3_LABEL_OPCODE_MAX /* must stay last */
} _bcm_caladan3_mplstp_opcode_t;

typedef struct {
    dq_t plist;
    bcm_trunk_add_info_t add_info;
} bcm_caladan3_mpls_trunk_association_t;

#define MAX_LBL_OPCODE (_BCM_CALADAN3_LABEL_OPCODE_MAX)

extern uint32 _sbx_mplstp_lbl_opcode[BCM_MAX_NUM_UNITS][MAX_LBL_OPCODE];

typedef enum {
    _CALADAN3_MPLS_PORT_MATCH_ADD,
    _CALADAN3_MPLS_PORT_MATCH_UPDATE,
    _CALADAN3_MPLS_PORT_MATCH_DELETE,
    _CALADAN3_MPLS_PORT_MATCH_MAX
} _caladan3_mpls_port_action_type;

/* Maximum number of PRIMARY Service Access points
 * valid on a MPLS VPWS private line service */
#define _BCM_CALADAN3_VPWS_MAX_SAP (2)

/* 
 * Routines and macros shared by ucode impls
 */
#define _BCM_MPLS_VPN_TYPE(_flags)                \
    ((_flags) & (BCM_MPLS_VPN_L3   |              \
                 BCM_MPLS_VPN_VPLS |              \
                 BCM_MPLS_VPN_VPWS))

/* make sure only ONE type of vpn is set in flags */
#define _BCM_MPLS_VPN_VALID_TYPE(_flags)                                \
     (_BCM_MPLS_VPN_TYPE(_flags) &&                                      \
      ((_BCM_MPLS_VPN_TYPE(_flags) & (_BCM_MPLS_VPN_TYPE(_flags) - 1)) == 0))

#define _BCM_MPLS_VPN_VALID_USER_HANDLE(unit, _vpn_id) \
    (((_vpn_id) > 0 && (_vpn_id) < BCM_VLAN_MAX ) || \
     ((_vpn_id) >= SBX_DYNAMIC_VSI_BASE(unit) &&        \
      (_vpn_id) <= (SBX_DYNAMIC_VSI_END(unit) * 2)))



extern bcm_caladan3_mpls_trunk_association_t 
mpls_trunk_assoc_info[BCM_MAX_NUM_UNITS][SBX_MAX_TRUNKS];

#define _BCM_CALADAN3_LABEL_LER(unit) \
    _sbx_mplstp_lbl_opcode[unit][_BCM_CALADAN3_LABEL_OPCODE_LER]
#define _BCM_CALADAN3_LABEL_LSR(unit) \
    _sbx_mplstp_lbl_opcode[unit][_BCM_CALADAN3_LABEL_OPCODE_LSR]
#define _BCM_CALADAN3_LABEL_PWE(unit) \
    _sbx_mplstp_lbl_opcode[unit][_BCM_CALADAN3_LABEL_OPCODE_PWE]
#define _BCM_CALADAN3_LABEL_CES(unit) \
    _sbx_mplstp_lbl_opcode[unit][_BCM_CALADAN3_LABEL_OPCODE_CES]

#define _BCM_CALADAN3_IS_LABEL_LER(unit, opcode) \
    ((_sbx_mplstp_lbl_opcode[unit][_BCM_CALADAN3_LABEL_OPCODE_LER] == (opcode)) ? 1:0)

#define BCM_MPLS_PORT_ALL_FAILOVERS             \
                 (BCM_MPLS_PORT_FAILOVER | BCM_MPLS_PORT_PW_FAILOVER)

typedef enum {
    _BCM_CALADAN3_MPLS_VPWS_UNI_PORT,
    _BCM_CALADAN3_MPLS_VPWS_UNI_PORT_FO,
    _BCM_CALADAN3_MPLS_VPWS_NW_PORT,
    _BCM_CALADAN3_MPLS_VPWS_NW_PORT_FO,
    _BCM_CALADAN3_MPLS_VPWS_NW_PORT_PW_FO,
    _BCM_CALADAN3_MPLS_VPWS_NW_PORT_PW_FO2 /* LSP failover port for PW FO port */
} _bcm_caladan3_mpls_vpws_port_type_t;

extern int
bcm_caladan3_mpls_vpn_id_get(int                    unit,
                             bcm_vpn_t              vpn,
                             bcm_mpls_vpn_config_t *info);

 extern int
_bcm_caladan3_mpls_vpn_stp_set(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        stp_state);
extern int
_bcm_caladan3_mpls_vpn_stp_get(int        unit,
                             bcm_vpn_t  vpn,
                             bcm_port_t port,
                             int        *stp_state);

int
_bcm_caladan3_mpls_alloc_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t  *l3_fe,
                                            _caladan3_vpn_sap_t         *vpn_sap,
                                            bcm_mpls_port_t         *mpls_port);
int
_bcm_caladan3_mpls_free_vpn_sap_hw_resources(_caladan3_l3_fe_instance_t  *l3_fe,
                                           _caladan3_vpn_sap_t        *vpn_sap);
#if 0
extern int
_bcm_caladan3_fill_mpls_label_array_from_ete_idx(_caladan3_l3_fe_instance_t *l3_fe,
                                               uint32                   ete_idx,
                                               int                      label_max,
                                               bcm_mpls_egress_label_t *label_array,
                                               int                     *label_count);
#endif

extern int
_bcm_caladan3_mpls_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int
_bcm_caladan3_mpls_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int
_bcm_caladan3_mpls_port_gport_attr_get(int unit,
                                     bcm_gport_t gport,
                                     bcm_port_t *phy_port,
                                     bcm_vlan_t *match_vlan,
                                     bcm_vlan_t *vsi);

extern int
_bcm_caladan3_mpls_port_info_get(int unit, int port, int vid,
                               bcm_vlan_t vsi, uint8 vpwsuni, 
                               int *keepUntagged);

extern int
_bcm_caladan3_mpls_port_vlan_vector_internal(int unit,
                                           bcm_gport_t gport,
                                           bcm_port_t *phy_port,
                                           bcm_vlan_t *match_vlan,
                                           bcm_vlan_t *vsi,
                                           uint32   *logicalPort,
                                           bcm_mpls_port_t *mpls_port,
                                           uint8    *vpwsuni,
                                           uint16   *num_ports);

extern int
_bcm_caladan3_find_mpls_vpncb_by_id(int                      unit,
                                  _caladan3_l3_fe_instance_t  *l3_fe,
                                  bcm_vpn_t                vpn_id,
                                  _caladan3_vpn_control_t    **vpnc);

extern int
bcm_caladan3_mpls_port_qosmap_set(int unit,
                                  bcm_gport_t gport,
                                  int ingrMap,
                                  int egrMap,
                                  uint32 ingFlags, 
                                  uint32 egrFlags);

extern int
bcm_caladan3_mpls_port_qosmap_get(int unit, bcm_gport_t gport, 
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags);

/*
 *   Function
 *      bcm_caladan3_g3p1_mpls_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided MPLS GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = MPLS GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
extern int
bcm_caladan3_g3p1_mpls_port_get_lpid(int unit,
                                   bcm_gport_t gport,
                                   uint32 *lpid,
                                   bcm_port_t *pport);

/*
 *  Function:
 *    bcm_caladan3_mpls_gport_get
 *  Description:
 *    Get the specified MPLS gport information
 *  Parameters:
 *    in int unit - unit to access
 *    in const bcm_gport_t gport - MPLS GPort to translate
 *    _caladan3_vpn_sap_t * 
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */
extern int
bcm_caladan3_mpls_gport_get(int unit,
                          bcm_gport_t gport,
                          _caladan3_vpn_sap_t **vpn_sap);



int
_bcm_caladan3_mpls_gport_get_mod_port(int            unit,
                                    bcm_gport_t    gport,
                                    bcm_module_t  *modid,
                                    bcm_port_t    *port);


#define _BCM_CALADAN3_VPWS_UNI_OFFSET (16*1024)

extern int
_bcm_caladan3_find_vpn_sap_by_id(_caladan3_l3_fe_instance_t *l3_fe,
                                 _caladan3_vpn_control_t   *vpnc,
                                 bcm_gport_t            mpls_port_id,
                                 _caladan3_vpn_sap_t      **vpn_sap);

extern int 
_bcm_caladan3_g3p1_mplstp_vpws_ft_lp_offset(int         unit,
                                             bcm_gport_t gport,
                                             uint32   *lport,
                                             uint32   *fte);

extern int
_bcm_caladan3_mpls_vpws_fte_connect(_caladan3_l3_fe_instance_t *l3_fe,
                                    _caladan3_vpn_sap_t* vpnSaps[_BCM_CALADAN3_VPWS_MAX_SAP],
                                    _caladan3_mpls_port_action_type action,
                                    uint32 connect);

extern int
_bcm_caladan3_mpls_label_commit_enable_set(int unit, uint32 enable);

extern int
_bcm_caladan3_mpls_label_commit_enable_get(int unit, uint32 *enable);

extern int
_bcm_caladan3_mpls_label_commit(int unit);

extern int
_bcm_caladan3_mpls_vpws_port_sap_get(_caladan3_vpn_control_t *vpnc, 
                                     _bcm_caladan3_mpls_vpws_port_type_t port_type,
                                     _caladan3_vpn_sap_t **vpn_sap);

#endif /* _BCM_INT_SBX_CALADAN3_MPLS_H_ */
