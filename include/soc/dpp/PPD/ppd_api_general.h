/* $Id: ppd_api_general.h,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __SOC_PPD_API_GENERAL_INCLUDED__
/* { */
#define __SOC_PPD_API_GENERAL_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPD/ppd_api_framework.h>
#include <soc/dpp/PPC/ppc_api_general.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     ignore given value     */
#define  SOC_PPD_IGNORE_VAL 0xFFFFFFFF

#define SOC_PPD_NOF_BITS_MPLS_LABEL                (20)
/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define SOC_PPD_DO_NOTHING_AND_EXIT                    \
  do                                              \
  {                                               \
    SOC_SAND_IGNORE_UNUSED_VAR(res);                  \
    goto exit;                                    \
  } while(0)


#define ARAD_MACRO(macro,params, res) res = SOC_SAND_OK;\
    ARAD_PP_##macro params

#define ARAD_MACRO_FUNC(func,params) ARAD_PP_##func params


#define SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, macro, params, res)    \
  switch (SOC_SAND_DEVICE_TYPE_GET(unit))        \
  {                                                   \
  case SOC_SAND_DEV_ARAD:                             \
      ARAD_MACRO(macro, params, res);                 \
	  break;                                          \
    default:                                          \
      res =  SOC_PPD_INVALID_DEVICE_TYPE_ERR;         \
  }



/* Used when macro returns a value */
#define SOC_PPD_ARAD_TMP_DEVICE_MACRO_VAL_CALL(unit, func, params)    \
	((SOC_SAND_DEVICE_TYPE_GET(unit) == SOC_SAND_DEV_ARAD) ? ARAD_MACRO_FUNC(func, params) : -1 )


/************************************************************************/
/* forwarding decision Macros                                           */
/************************************************************************/

/* $Id: ppd_api_general.h,v 1.23 Broadcom SDK $
 *  Destination is Drop. Set 'fwd_decision' to drop
 *  destination. Packet forwarded according to this
 *  'fwd_decision' is dropped.
 */
#define SOC_PPD_FRWRD_DECISION_DROP_SET(unit, fwd_decision, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_DROP_SET, (unit, fwd_decision), res);
/*
 *  Destination is the local CPU. Set 'fwd_decision' to local
 *  CPU (i.e. local port 0). Packet forwarded according to
 *  this 'fwd_decision' is forwarded to CPU (not trapped,
 *  i.e., with no trap-code attached to it)
 */
#define SOC_PPD_FRWRD_DECISION_LOCAL_CPU_SET(unit, fwd_decision, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_LOCAL_CPU_SET, (unit, fwd_decision), res);
/*
 *  Destination is a physical system port. Set the
 *  'fwd_decision' to include the destination physical
 *  system port (0 to 4K-1).
 */
#define SOC_PPD_FRWRD_DECISION_PHY_SYS_PORT_SET(unit, fwd_decision, phy_port, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_PHY_SYS_PORT_SET, (unit, fwd_decision, phy_port), res);
/*
 *  Destination is a LAG. Set the 'fwd_decision' to include
 *  the LAG ID.
 */
#define SOC_PPD_FRWRD_DECISION_LAG_SET(unit, fwd_decision, lag_id, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_LAG_SET, (unit, fwd_decision, lag_id), res);
/*
 *  Destination is a multicast group. Set the 'fwd_decision'
 *  to include MC-group ID
 */
#define SOC_PPD_FRWRD_DECISION_MC_GROUP_SET(unit, fwd_decision, mc_id, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_MC_GROUP_SET, (unit, fwd_decision, mc_id), res);
/*
 *  Destination is a FEC-entry. Set the 'fwd_decision' to
 *  include a pointer to the FEC table
 */
#define SOC_PPD_FRWRD_DECISION_FEC_SET(unit, fwd_decision, fec_id, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_FEC_SET, (unit, fwd_decision, fec_id), res);
/*
 *  Destination with COS (i.e., explicit TM flow). Set the
 *  'fwd_decision' to include the explicit TM flow_id
 */
#define SOC_PPD_FRWRD_DECISION_EXPL_FLOW_SET(unit, fwd_decision, flow_id, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_EXPL_FLOW_SET, (unit, fwd_decision, flow_id), res);
/*
 *  Trap packet. Set the 'fwd_decision' to Trap the packet
 *  using the following attributes: - trap_code : 0-255;
 *  identifies the trap/snoop actions to be applied if the
 *  assigned strength is higher than the previously assigned
 *  strength.- fwd_strength: 0-7- snp_strenght: 0-3
 */
#define SOC_PPD_FRWRD_DECISION_TRAP_SET(unit, fwd_decision, _trap_code, frwrd_strength, snp_strength, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_TRAP_SET, (unit, fwd_decision, _trap_code, frwrd_strength,snp_strength), res);
/*
 *  Forward to an Out-AC Logical Interface. Set the
 *  'fwd_decision' to include the destination system-port
 *  and the Out-AC ID. Notes 1. The system-port can either be
 *  a LAG port or a system physical port.2. Packets
 *  forwarded according to this 'fwd_decision' are forwarded
 *  to the given sys_port3. The outgoing VLAN editing
 *  information is configured according to the associated
 *  Out-AC.4. This forwarding decision can be dynamically
 *  learned, by setting it in the In-AC's Learn-Record (see
 *  SOC_PPC_L2_LIF_AC_INFO).
 */
#define SOC_PPD_FRWRD_DECISION_AC_SET(unit, fwd_decision, ac_id, is_lag, sys_port_id, res)  \
  SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_AC_SET, (unit, fwd_decision, ac_id, is_lag, sys_port_id), res);
/*
 *  Forward to an Out-AC Logical Interface, using an
 *  explicit TM flow ID. Set the 'fwd_decision' to include
 *  the destination flow-id and the Out-AC ID. Notes 1.
 *  Packets forwarded according to this 'fwd_decision' are
 *  forwarded according the given TM flow-id2. The outgoing
 *  VLAN editing information is configured according to the
 *  associated Out-AC. 3. This forwarding decision can be
 *  dynamically learned, by setting it in the In-AC's
 *  Learn-Record (see SOC_PPC_L2_LIF_AC_INFO).
 */
#define SOC_PPD_FRWRD_DECISION_AC_WITH_COSQ_SET(unit, fwd_decision, ac_id,flow_id, res)  \
  SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_AC_WITH_COSQ_SET, (unit, fwd_decision, ac_id, flow_id), res);
/*
 *  Forward to access associated with AC-id with
 *  protection. Set the 'fwd_decision' to include AC-id with
 *  FEC-index. Packet forwarded according to this
 *  'fwd_decision' is forwarded according the FEC entry
 *  setting associated with the given (out) AC-id. This
 *  forwarding decision can be learned.
 */
#define SOC_PPD_FRWRD_DECISION_PROTECTED_AC_SET(unit, fwd_decision, ac_id, fec_index, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_PROTECTED_AC_SET, (unit, fwd_decision, ac_id, fec_index), res);
/*
 *  VPLS access to core with no protection. Set the
 *  'fwd_decision' to include pwe-id and system-port. Packet
 *  forwarded according to this 'fwd_decision' is forwarded
 *  to sys_port encapsulated according to pwe_id setting.
 *  This forwarding decision can be learned as well.
 */
#define SOC_PPD_FRWRD_DECISION_PWE_SET(unit, fwd_decision, pwe_id, is_lag, sys_port_id, res)  \
  SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_PWE_SET, (unit, fwd_decision, pwe_id, is_lag, sys_port_id), res);
/*
 *
 */
#define SOC_PPD_FRWRD_DECISION_TRILL_SET(unit, fwd_decision, nick, is_multi, fec_or_mc_id, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_TRILL_SET, (unit, fwd_decision, nick, is_multi, fec_or_mc_id), res);
/*
 *  VPLS access to core with no protection using an explicit TM flow ID.
 *  Set the fwd_decision' to include the destination flow-id and the PWE-ID.
 *  Notes 1. Packets forwarded according to this 'fwd_decision' are
 *  forwarded according the given TM flow-id 2. This forwarding decision can be
 *  dynamically learned, by setting it in the In-PWE's
 *  Learn-Record (see SOC_PPC_L2_LIF_PWE_INFO).
 */
#define SOC_PPD_FRWRD_DECISION_PWE_WITH_COSQ_SET(unit, fwd_decision, pwe_id, flow_id, res)  \
  SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_PWE_WITH_COSQ_SET, (unit, fwd_decision, pwe_id, flow_id), res);

/*
 *  VPLS access to core with protection on tunnel only. Set
 *  the 'fwd_decision' to include fec-index VC-label. Packet
 *  forwarded according to this 'fwd_decision' is
 *  encapsulated with 'vc_label'. EXP,TTL is set according
 *  to 'push_profile' definition see. This forwarding
 *  decision can be learned as well.
 */
#define SOC_PPD_FRWRD_DECISION_PWE_PROTECTED_TUNNEL_SET(unit, fwd_decision, vc_label, push_profile, fec_index, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_PWE_PROTECTED_TUNNEL_SET, (unit, fwd_decision, vc_label, push_profile, fec_index), res);

/*
 *  VPLS access to core with protection on tunnel only. Set
 *  the 'fwd_decision' to include fec-index and VPLS outlif. Packet
 *  forwarded according to this 'fwd_decision' is
 *  encapsulated as follows: PWE label is accoring to pwe_outlif entry, 
 *  tunnel labels are according to fec resolution. This forwarding
 *  decision can be learned as well.
 */
#define SOC_PPD_FRWRD_DECISION_PWE_PROTECTED_TUNNEL_WITH_OUTLIF_SET(unit, fwd_decision, fec_index, pwe_outlif, res) \
    ARAD_MACRO(FRWRD_DECISION_PWE_PROTECTED_TUNNEL_WITH_OUTLIF_SET, (unit, fwd_decision, fec_index, pwe_outlif), res); 

/*
 *  VPLS access to core with protection on PWE. Set the
 *  'fwd_decision' to include fec-index. Packet forwarded
 *  according to this 'fwd_decision' is forwarded according
 *  to FEC entry setting. By this setting the PWE can be
 *  protected. This forwarding decision can be learned as
 *  well.
 */
#define SOC_PPD_FRWRD_DECISION_PROTECTED_PWE_SET(unit, fwd_decision, fec_index, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_PROTECTED_PWE_SET, (unit, fwd_decision, fec_index), res);
/*
 *  ILM entry. Set the 'fwd_decision' to include swap-label
 *  and fec-index label. For Packets forwarded according to
 *  this 'fwd_decision' MPLS label is swappedAnd forwarded
 *  according to FEC entry setting
 */
#define SOC_PPD_FRWRD_DECISION_ILM_SWAP_SET(unit, fwd_decision, swap_label, fec_index, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_ILM_SWAP_SET, (unit, fwd_decision, swap_label, fec_index), res);
/*
 *  ILM Push entry. Set the 'fwd_decision' to include label
 *  and fec-index label. For Packets forwarded according to
 *  this 'fwd_decision' MPLS label is pushed And forwarded
 *  according to FEC entry setting
 */
#define SOC_PPD_FRWRD_DECISION_ILM_PUSH_SET(unit, fwd_decision,label, push_profile, fec_index, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_ILM_PUSH_SET, (unit, fwd_decision,label, push_profile, fec_index), res);
/*
 *  Mac in mac entry. Destination is a FEC-entry. Set the 'fwd_decision' to
 *  include a pointer to the FEC table. EEI is the isid_id.
 */
#define SOC_PPD_FRWRD_DECISION_MAC_IN_MAC_SET(unit, fwd_decision, isid_id, fec_id, res)  \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_CALL(unit, FRWRD_DECISION_MAC_IN_MAC_SET, (unit, fwd_decision, isid_id, fec_id), res);

/*
 *  Forward to an Out-AC Logical Interface. Set the
 *  'fwd_decision' to include the destination system-port
 *  and the Out-AC ID. Notes 1. The system-port can either be
 *  a LAG port or a system physical port.2. Packets
 *  forwarded according to this 'fwd_decision' are forwarded
 *  to the given sys_port3. The outgoing VLAN editing
 *  information is configured according to the associated
 *  Out-AC.4. This forwarding decision can be dynamically
 *  learned, by setting it in the In-AC's Learn-Record (see
 *  SOC_PPC_L2_LIF_AC_INFO).
 */

/************************************************************************/
/* CUD macros                                                                     */
/************************************************************************/

/*
 * Returns CUD that includes EEP with value 'eep_ndx'
 */
#define SOC_PPD_CUD_EEP_GET(unit, eep_ndx)   \
    SOC_PPD_ARAD_TMP_DEVICE_MACRO_VAL_CALL(unit, CUD_EEP_GET, (unit, eep_ndx));

/*
 * Returns CUD that includes VSI with value 'vsi_ndx'
 */

#define SOC_PPD_CUD_VSI_GET(unit, vsi_ndx)   \
    SOC_PPD_ARAD_TMP_DEVICE_MACRO_VAL_CALL(unit, CUD_VSI_GET, (unit, vsi_ndx));
/*
 * Returns CUD that includes AC with value 'ac_ndx'
 */
#define SOC_PPD_CUD_AC_GET(unit, ac_ndx)   \
	SOC_PPD_ARAD_TMP_DEVICE_MACRO_VAL_CALL(unit, CUD_AC_GET, (unit, ac_ndx));


#define SOC_PPD_DEST_TRAP_VAL_SET(__dest_val, __trap_code,__fwd_strenght, __snoop_strength)   \
            __dest_val = (((__snoop_strength)<<19) | (__fwd_strenght)<<16)|((__trap_code))

#define SOC_PPD_DEST_TRAP_VAL_GET_TRAP_CODE(__dest_val)   \
            ((__dest_val) & 0xFFFF)

#define SOC_PPD_DEST_TRAP_VAL_GET_FWD_STRENGTH(__dest_val)   \
            (((__dest_val) >> 16) & 0x7)

#define SOC_PPD_DEST_TRAP_VAL_GET_SNP_STRENGTH(__dest_val)   \
            (((__dest_val) >> 19) & 0x3)

/* 
 * "identifier" encodes the action type of the mpls command.
 * identifier 0-7: push action. In this case identifier is also the push profile.
 * identifier 8: pop action.
 * identifier 9: swap action.
 */
#define SOC_PPC_EEI_ENCODING_MPLS_COMMAND(identifier,mpls_label)           ((identifier << SOC_PPD_NOF_BITS_MPLS_LABEL) | mpls_label)
#define SOC_PPD_MPLS_LABEL_FROM_EEI_COMMAND_ENCODING(eei)                  (eei & 0xfffff)
#define SOC_PPD_MPLS_IDENTIFIER_FROM_EEI_COMMAND_ENCODING(eei)           (eei >> SOC_PPD_NOF_BITS_MPLS_LABEL)

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PPD_GENERAL_GET_PROCS_PTR = SOC_PPD_PROC_DESC_BASE_GENERAL_FIRST,
  /*
   * } Auto generated. Do not edit previous section.
   */

   SOC_PPD_FWD_DECISION_TO_SAND_DEST,
   SOC_PPD_SAND_DEST_TO_FWD_DECISION,



  /*
   * Last element. Do no touch.
   */
  SOC_PPD_GENERAL_PROCEDURE_DESC_LAST
} SOC_PPD_GENERAL_PROCEDURE_DESC;


/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
uint32
  soc_ppd_fwd_decision_to_sand_dest(
    SOC_SAND_IN int              unit,
    SOC_SAND_IN SOC_PPC_FRWRD_DECISION_INFO *fwd_decision,
    SOC_SAND_OUT SOC_SAND_PP_DESTINATION_ID *dest_id
  );

uint32
  soc_ppd_sand_dest_to_fwd_decision(
    SOC_SAND_IN int              unit,
    SOC_SAND_IN SOC_SAND_PP_DESTINATION_ID *dest_id,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO *fwd_decision
  );

/*********************************************************************
* NAME:
 *   soc_ppd_l2_next_prtcl_type_allocate_test
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Checks whether l2_next_prtcl_type can be successfully allocated.
 * INPUT:
 *   SOC_SAND_IN  int                                               unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                                               l2_next_prtcl_type -
 *     Ethernet Type.
 *   SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                             *success -
 *     Whether the set operation succeeded. Operation may fail
 *     if there are no available resources to support the given
 *     (new) Ethernet Type.
 * REMARKS:
 *   Arad-only.
 * RETURNS:
 *   OK or ERROR indication.
*********************************************************************/
uint32
  soc_ppd_l2_next_prtcl_type_allocate_test(
    SOC_SAND_IN  int                               unit,
    SOC_SAND_IN  uint32                               l2_next_prtcl_type,
    SOC_SAND_OUT  SOC_SAND_SUCCESS_FAILURE            *success
  );

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

/* } __SOC_PPD_API_GENERAL_INCLUDED__*/
#endif
