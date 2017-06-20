
#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id: arad_pp_init.c,v 1.55 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#include <shared/bsl.h>

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_ac.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_filter.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_mirror.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_qos.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_vlan_edit.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_bmact.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_fec.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ilm.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv6.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_trill.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lag.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_cos.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_ing_vlan_edit.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lif_table.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_cos.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_filter.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_mirror.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_parse.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_sa_auth.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_trap.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_vid_assign.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_metering.h>
#include <soc/dpp/JER/JER_PP/jer_pp_metering.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mpls_term.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_mymac.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_port.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_rif.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_isem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_trap_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_vsi.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_key.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_l3_src_bind.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_slb.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_vtt.h>
#include <soc/dpp/JER/jer_init.h>

#include <soc/dpp/drv.h>

#include <shared/swstate/access/sw_state_access.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* default setting for forwarding enable:
 * - AC P2P to AC - no lookup
 * - AC P2P to PWE - no lookup
 * - AC P2P to PBB - no lookup
 * - AC MP - lookup
 * - ISID P2P - no lookup
 * - ISID MP - lookup
 * - TRILL, IP-Tunnel, Label-PWE-MP, Label-VRF, Label-LSP -lookup
 * - lABEL-PWE-P2P - no lookup
 */


/*
 * VSI for p2p, used for
 * - AC P2P to AC - no lookup
 * - AC P2P to PWE - no lookup
 * - AC P2P to PBB - no lookup
 *  - needed as all P2P services has the same VSI value
 *  - has no usage, and
 *  - VSI (1) can still be used for MP
 */

/* 3 Msb of destination encoding for system port in ARAD */
#define ARAD_PP_DESTINATION_SYSTEM_PORT_PREFIX  0x4
/* } */
/*************
 *  MACROS   *
 *************/
/* { */



#define ARAD_PP_INIT_PRINT_ADVANCE(str, phase_id)			\
          do                                                                              \
          {                                                                               \
            if (!silent)                                                                  \
            {                                                                             \
              LOG_VERBOSE(BSL_LS_SOC_INIT, \
                          (BSL_META_U(unit, \
                                      "    + Phase %u, %.2u: %s\n\r"), phase_id, ++stage_id, str)); \
            }                                                                             \
          }while(0)

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

CONST STATIC SOC_PROCEDURE_DESC_ELEMENT
  Arad_pp_procedure_desc_element_init[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_INIT_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_INIT_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */

  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_FUNCTIONAL_INIT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_HW_SET_DEFAULTS),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2_VERIFY),

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_init[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_ERR_DESC_ELEMENT_DEF_LAST
};

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/* Fill FTMH header data in sw db for parsing in interrupt context */
STATIC uint32
  arad_pp_mgmt_sw_dbs_ftmh_header_set(
    SOC_SAND_IN  int                 unit
  )
{
  soc_error_t
    rv = SOC_E_NONE;
  uint32 reg_val;
  uint8 ftmh_lb_key_enable, ftmh_stacking_ext_mode, system_headers_mode;
  soc_reg_t
    global_f = SOC_IS_JERICHO(unit)? ECI_GLOBAL_SYS_HEADER_CFGr: ECI_GLOBALFr;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* LB mode */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(rv,  10,  exit, ARAD_REG_ACCESS_ERR, soc_reg32_get(unit, global_f, REG_PORT_ANY, 0, &reg_val));
  ARAD_FLD_FROM_REG(global_f, FTMH_LB_KEY_EXT_ENf, ftmh_lb_key_enable, reg_val, 20, exit);
  rv = sw_state_access[unit].dpp.soc.arad.pp.header_data.ftmh_lb_key_ext_en.set(unit, ftmh_lb_key_enable);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(rv, 30, exit);

  /* Stacking mode */
  ARAD_FLD_FROM_REG(global_f, FTMH_STACKING_EXT_ENABLEf, ftmh_stacking_ext_mode, reg_val, 50, exit);
  rv = sw_state_access[unit].dpp.soc.arad.pp.header_data.ftmh_stacking_ext_enable.set(unit, ftmh_stacking_ext_mode);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(rv, 60, exit);

  /* System headers mode */
  ARAD_FLD_FROM_REG(global_f, SYSTEM_HEADERS_MODEf, system_headers_mode, reg_val, 80, exit);
  rv = sw_state_access[unit].dpp.soc.arad.pp.header_data.system_headers_mode.set(unit, system_headers_mode);
  SOC_SAND_CHECK_FUNC_RESULT(rv, 90, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_sw_dbs_ftmh_header_set()",0,0);
}

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
uint32 
    arad_pp_init_dynamic_kbp_result(int unit)
{
    int res_sizes[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS] = {0};
    int res_sizes_in_byte[ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS] = {0};
    ARAD_PMF_CE_IRPP_QUALIFIER_ATTRIBUTES kbp_qualifiers_values;
    int i, total_nof_bits_in_elk_res = 0;
    int found_result_zero;
	int res1_pad=0, res3_pad=0;
	int total_bits_in_lsb = 0, total_bits_in_msb = 0, aling_to_120b_index = -1;
    ARAD_PMF_CE_IRPP_QUALIFIER_INFO irpp_qual_info;
    uint8 found;
    ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA flp_lookups_tbl;

    SOCDNX_INIT_FUNC_DEFS;

    /* default result size in Jericho: 48 32 24 16, when result 0 can be used for FWD, and result 2 for RPF */
    res_sizes[0] = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "0", 48);/* FWD/ACL0 	*/
    res_sizes[1] = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "1", 32);/* ACL1		*/
    res_sizes[2] = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "2", 24);/* RPF/ACL2	*/
    res_sizes[3] = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "3", 16);/* ACL3		*/
    res_sizes[4] = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "4", 32);/* ACL4   	*/
    res_sizes[5] = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "5", 32);/* ACL5		*/

    /*if double capacity enabled  allresults are constant */
    if (ARAD_KBP_ENABLE_IPV4_DC){
        res1_pad = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "res1_pad", 0);
        res3_pad = soc_property_suffix_num_get(unit, 0, spn_EXT_TCAM_RESULT_SIZE_SEGMENT, "res3_pad", 0);
        if(ARAD_KBP_IPV4DC_24BIT_FWD) {
            if ((res_sizes[0] != 24) || (res_sizes[1] != 24) || (res_sizes[2] != 24) || (res1_pad != 24) || (res3_pad != 24)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res when using DC feature with 24bit forwarding. Should be res0 = res1 = res2 = res1_pad = res3_pad = 24 bits\n")));
            }
            if ((res_sizes[3] + res_sizes[4] + res_sizes[5] > 104)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res when using DC feature. Should be res3 + res4 + res5 <= 104 bits\n")));
            }
        } else {
            if ((res_sizes[0] != 48) || (res_sizes[2] != 48) || (res3_pad != 48 )) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res when using DC feature. Should be res0 = res2 = res3_pad = 48 bits\n")));
            }
            if (res_sizes[1] > 24) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res when using DC feature. Should be res1 <= 24 bits\n")));
            }
        }
        aling_to_120b_index = 2;

		for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
			total_nof_bits_in_elk_res += res_sizes[i];
		}
        total_nof_bits_in_elk_res += res1_pad;
        total_nof_bits_in_elk_res += res3_pad;

    } else {

    	/*if fwd is enable in one of the tables the first result has to be 48b */
        if (ARAD_KBP_ENABLE_IPV4_UC ||  ARAD_KBP_ENABLE_IPV4_MC || ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_MC 
            || ARAD_KBP_ENABLE_TRILL_UC || ARAD_KBP_ENABLE_TRILL_MC || ARAD_KBP_ENABLE_MPLS){
            if (res_sizes[0] != ARAD_PP_FLP_DEFAULT_FWD_RES_SIZE) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res0 , when we have fwd in external tcam res0 has to be %d \n"), ARAD_PP_FLP_DEFAULT_FWD_RES_SIZE));
            }
        }

    	/*if rpf is enable in one of the tables its result has to: (1)be 24b, (2)be located on the MSB (first 120 bits) */
        if (ARAD_KBP_ENABLE_IPV4_RPF ||  ARAD_KBP_ENABLE_IPV6_RPF ){
            if ((res_sizes[0]+res_sizes[1]+res_sizes[2]) > ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res 0-2, when we have fwd_rpf in external tcam res0-2 have to be <= %d \n"), ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS));
            }
            if (res_sizes[2] < ARAD_PP_FLP_DEFAULT_RPF_RES_SIZE) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("illegal value for external res2 , when we have rpf in external tcam res 2 has to be >= %d \n"), ARAD_PP_FLP_DEFAULT_RPF_RES_SIZE));
            }
        }

        /* check which result is align to 120 bit. if none, return an error*/
		total_nof_bits_in_elk_res = 0;
		for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
			total_nof_bits_in_elk_res += res_sizes[i];
			if (total_nof_bits_in_elk_res == ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS) {
				aling_to_120b_index = i;
			}
		}

		/* results have to be align 120b */
		if (aling_to_120b_index == -1) {
			SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("results length have to be 120b alignment, total separation between LSB and MSB buffers\n"
																  "res0 = %d, res1 = %d, res2 = %d, res3 = %d, res4 = %d, res5 = %d\n"),res_sizes[0],res_sizes[1],res_sizes[2],res_sizes[3],res_sizes[4],res_sizes[5]));
		}
	}

    /* total results size is limit to payload buffer size */
    if (total_nof_bits_in_elk_res > (ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS + ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_SIZE_IN_BITS - ARAD_PP_FLP_KBP_ROP_HEADERS_SIZE_IN_BITS)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("total external result size has to be %d, total = %d, "
                                                              "update ext_tcam_ressize_segment_# according\n"), (ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS + ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_SIZE_IN_BITS - ARAD_PP_FLP_KBP_ROP_HEADERS_SIZE_IN_BITS), total_nof_bits_in_elk_res));
    }

    /* check that there is no holes in the results buffer */
    found_result_zero = 0;
    for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
        if (found_result_zero == 1) {
            if (res_sizes[i] > 0) {
              SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Found hole in KBP result size configuration. No holes are allowed")));
            }
        } else {
            if (res_sizes[i] == 0) {
                found_result_zero = 1;
            }
        }
    }


    for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
        res_sizes_in_byte[i] = (res_sizes[i]+7)/8;
    }

    /* configure the FLP - for all FLP programs that uses KBP for forwarding*/
    for(i = 0; i < SOC_DPP_DEFS_GET(unit, nof_flp_programs); i++){
        uint32 ipv6rpf_prog_id = 0;
        arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, i, &flp_lookups_tbl);
        arad_pp_sw_db_flp_prog_app_to_index_get(unit, PROG_FLP_IPV6UC_RPF, &ipv6rpf_prog_id);

		if (flp_lookups_tbl.elk_lkp_valid) {

            ARAD_KBP_FRWRD_IP_DB_TYPE   db_type;
            uint32                      ltr_id;
            SOC_SAND_SUCCESS_FAILURE    success;

            int rpf_result_start_offset  = 0;
            int fec_res_found_bit_offset = 0;
            int is_rpf_lookup_needed     = 0;

            /* add RPF configuration */
            if((i == PROG_FLP_IPV4UC_RPF)||(i == ipv6rpf_prog_id)||(i == PROG_FLP_IPV6MC)||(i == PROG_FLP_IPV4COMPMC_WITH_RPF)||(i == PROG_FLP_IPV4_DC)){
                rpf_result_start_offset = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS-(res_sizes[0]+res_sizes[1]+res_sizes[2]+res1_pad);
                fec_res_found_bit_offset = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS+5;
                is_rpf_lookup_needed = 1;
				if(i == PROG_FLP_IPV4_DC && !ARAD_KBP_IPV4DC_24BIT_FWD){
					is_rpf_lookup_needed = 2; /* in order to get 48 bit result length, instead of 24*/
				}
            }

            /* configures the FLP */            
            arad_pp_flp_elk_result_configure(unit, i, res_sizes[0]+res1_pad, ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS+7, is_rpf_lookup_needed*24, rpf_result_start_offset, fec_res_found_bit_offset);
                            
            SOCDNX_SAND_IF_ERR_EXIT(arad_kbp_opcode_to_db_type(unit, flp_lookups_tbl.elk_opcode, &db_type, &ltr_id, &success));
            SOCDNX_SAND_IF_ERR_EXIT(arad_kbp_result_sizes_configurations_init(unit, db_type, res_sizes_in_byte));
        }        
    }    

    /* configure the action size */
    SOCDNX_SAND_IF_ERR_EXIT(arad_pmf_fem_action_elk_result_size_update(unit, res_sizes));
    
    total_bits_in_msb = 0;
    if (ARAD_KBP_ENABLE_IPV4_DC) {
        total_bits_in_lsb = res_sizes[0]+res_sizes[1]+res_sizes[2]+res1_pad+res3_pad - ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS;
        res_sizes[0] += res1_pad;
    } else {
        total_bits_in_lsb = 0;
    }

    for (i = 0; i < ARAD_PP_FLP_KBP_MAX_NUMBER_OF_RESULTS; i++) {
        SOCDNX_SAND_IF_ERR_EXIT(arad_pmf_ce_internal_field_info_find(unit, SOC_PPC_FP_QUAL_ELK_LOOKUP_RESULT_0+i, 
                                                                     SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, 0, &found, &irpp_qual_info));

        kbp_qualifiers_values.irpp_field    = irpp_qual_info.info.irpp_field;
        kbp_qualifiers_values.is_msb        = irpp_qual_info.info.is_msb;
		kbp_qualifiers_values.is_lsb        = irpp_qual_info.info.is_lsb;
		kbp_qualifiers_values.qual_nof_bits = res_sizes[i];

		if (i <= aling_to_120b_index) {
			total_bits_in_msb += res_sizes[i];
			kbp_qualifiers_values.buffer_lsb = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_BIT_IN_BUFFER + ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_SIZE_IN_BITS - total_bits_in_msb;
		}else{
			total_bits_in_lsb += res_sizes[i];
			kbp_qualifiers_values.buffer_lsb = ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_BIT_IN_BUFFER + ARAD_PP_FLP_KBP_MAX_ROP_PAYLOAD_LSB_SIZE_IN_BITS - total_bits_in_lsb;
		}
        sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_pmf_ce_dynamic_kbp_qualifiers_values.set(unit, i, &kbp_qualifiers_values); 

        LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"kbp_qualifiers_values.irpp_field    = %d\n"),kbp_qualifiers_values.irpp_field));
        LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"kbp_qualifiers_values.qual_nof_bits = %d\n"),kbp_qualifiers_values.qual_nof_bits));
        LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"kbp_qualifiers_values.buffer_lsb    = %d\n"),kbp_qualifiers_values.buffer_lsb));
        LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"kbp_qualifiers_values.is_lsb        = %d\n"),kbp_qualifiers_values.is_lsb));
        LOG_DEBUG(BSL_LS_SOC_FP,(BSL_META_U(unit,"kbp_qualifiers_values.is_msb        = %d\n"),kbp_qualifiers_values.is_msb));
    }

    /* configure the PMF */
    sw_state_access[unit].dpp.soc.arad.tm.kbp_info.Arad_pmf_ce_dynamic_kbp_qualifiers_enabled.set(unit, 1); 

exit:
    SOCDNX_FUNC_RETURN; 
}

#endif

/* For QAX: set L2_LIF per outlif type 
 *  outlif type are an internal type (ees action type.)
 */
#define EPNI_ETPP_EES_ACTION_TYPE_DATA        (0)
#define EPNI_ETPP_EES_ACTION_TYPE_MPLS        (1)
#define EPNI_ETPP_EES_ACTION_TYPE_IP_TUNNEL   (2)
#define EPNI_ETPP_EES_ACTION_TYPE_ISID        (3)
#define EPNI_ETPP_EES_ACTION_TYPE_OUT_RIF     (4)
#define EPNI_ETPP_EES_ACTION_TYPE_TRILL       (5)
#define EPNI_ETPP_EES_ACTION_TYPE_LL_ARP      (6)
#define EPNI_ETPP_EES_ACTION_TYPE_AC          (7)
#define EPNI_ETPP_EES_ACTION_TYPE_EEP         (8)
#define EPNI_ETPP_EES_ACTION_TYPE_NULL        (9)


/*********************************************************************
*     Initialize roo application
*********************************************************************/
uint32 arad_mgmt_roo_init(
    SOC_SAND_IN  int                                unit)
{
    uint32
        reg_val, 
        fld_val; 
    soc_reg_above_64_val_t reg_val_above_64; 

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_val_above_64);

    
    if (!SOC_IS_ARDON(unit) && SOC_IS_ARADPLUS(unit)) 
    {
        /* roo: skip ethernet feature: enable skip ethernet feature.
         * In roo, arad+, only way to terminate the inner ethernet
         * Only for Arad+.
         * For Jericho, use second my-mac termination */
        if (SOC_IS_ARADPLUS_A0(unit)) {
            SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_0r, SOC_CORE_ALL, 0, FIELD_31_31f, 1)); /* EnableIpSkipEthernet is an internal field. */
        }

        /* roo: used to check that the LL is of type data-entry.
           In this field, save the expected data-entry type of the LL */
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_CFG_DC_OVERLAYr, SOC_CORE_ALL, 0, CFG_DATA_ENTRY_TYPE_IS_LL_ETHERNETf,  ARAD_PRGE_DATA_ENTRY_LSBS_ROO_VXLAN));
        if SOC_IS_JERICHO_PLUS_A0(unit) {
            SOCDNX_IF_ERR_EXIT(READ_IHP_VTT_GENERAL_CONFIGS_1r(unit, REG_PORT_ANY, reg_val_above_64));
            soc_reg_above_64_field32_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, reg_val_above_64, SKIP_ETHERNET_UPDATE_IN_RIFf, 1);
            soc_reg_above_64_field32_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, reg_val_above_64, ENABLE_TUNNEL_TERMINATION_CODE_UPGRADESf, 1);
            SOCDNX_IF_ERR_EXIT(WRITE_IHP_VTT_GENERAL_CONFIGS_1r(unit, REG_PORT_ANY, reg_val_above_64));
        }
        else {
            SOCDNX_IF_ERR_EXIT(READ_IHP_VTT_GENERAL_CONFIGS_1r_REG32(unit, REG_PORT_ANY, &reg_val));
            soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, &reg_val, SKIP_ETHERNET_UPDATE_IN_RIFf, 1);
            soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_1r, &reg_val, ENABLE_TUNNEL_TERMINATION_CODE_UPGRADESf, 1);
            SOCDNX_IF_ERR_EXIT(WRITE_IHP_VTT_GENERAL_CONFIGS_1r_REG32(unit, REG_PORT_ANY, reg_val));
        }

        /* roo: enable chicken bits. LBP
         * Only for Arad+.
         * For Jericho, stamping is determined according to PPH-VSI-Source table. 
         */
        if (SOC_IS_ARADPLUS_A0(unit)) {
            SOCDNX_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IHB_LBP_GENERAL_CONFIG_0r, SOC_CORE_ALL, 0, ENABLE_NATIVE_VSI_STAMPINGf, 1));
        }

        /* roo: enable chiken bits. Forwarding lookup */
        SOCDNX_IF_ERR_EXIT(READ_IHP_FLP_GENERAL_CFGr(unit, SOC_CORE_ALL, reg_val_above_64)); 


        fld_val = 0x1;
        soc_reg_above_64_field32_set(unit, IHP_FLP_GENERAL_CFGr, reg_val_above_64, ENABLE_MACT_FORMAT_3f, fld_val);
        soc_reg_above_64_field32_set(unit, IHP_FLP_GENERAL_CFGr, reg_val_above_64, ENABLE_INNER_COMPATIBLE_MCf, fld_val);

        /* for jericho */
        if (SOC_IS_JERICHO(unit)) 
        {
            /* forwarding registers */

            /* eei filling:
             * use register to fill eei. (eei to build the native ethernet header).
             * native-arp: Truncated entry from the host table (format 3b).  15b instead of 18b.
             *             Full entry: MactFormat3EeiBits[2:1] Native-ARP(15) MactFormat3EeiBits[0]
             * native arp pointer lsb is always 0 (to get full entry in eedb): MactFormat3EeiBits[0] 
             */
              
            /* in arad+ eei format: 1111 HI(4) Native-ARP(15) 0.
             * in jericho eei format: 11 MactFormat3EeiBits[2:1] HI(4) Native-ARP(15) MactFormat3EeiBits[0]
             * In case we are in Jericho device with arad system header, then eei format must be 1111, meaning we need to configure
             *  MactFormat3EeiBits[2:1] to be 11.
             * We make sure we don't have user misconfiguration
             */
            if (SOC_IS_ROO_ENABLE(unit) 
                && (soc_property_get(unit, spn_SYSTEM_IS_ARAD_IN_SYSTEM, 0))
                && (SOC_ROO_HOST_ARP_MSBS(unit) != 3)){
                SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG(
                                                        "For ROO application with Arad system headers, soc property spn_BCM886XX_ROO_HOST_ARP_MSBS must be configured with 3 instead of %d "), 
                                                            SOC_ROO_HOST_ARP_MSBS(unit)));

            }


            /* get 2 msbs for the arp pointer from soc property.*/
            fld_val = SOC_ROO_HOST_ARP_MSBS(unit); 
            /* arp pointer 2 msbs are saved in MactFormat3EeiBits[2:1].
             * MactFormat3EeiBits[0] = 0 */
            fld_val = (fld_val << 1); 
            soc_reg_above_64_field32_set(unit, IHP_FLP_GENERAL_CFGr, reg_val_above_64, MACT_FORMAT_3_EEI_BITSf, fld_val);

            /* fec filling:
             * use register to fill fec.
             * Fec: Truncated entry from the host table (format 3b).
             *      Full entry: MactFormat3FecMsbBits(5) Fec(12)   .
             * Requirement: MactFormat3FecMsbBits = 0 */
            fld_val =  0; 
            soc_reg_above_64_field32_set(unit, IHP_FLP_GENERAL_CFGr, reg_val_above_64, MACT_FORMAT_3_FEC_MSB_BITSf, fld_val);
        }
        SOCDNX_IF_ERR_EXIT(WRITE_IHP_FLP_GENERAL_CFGr(unit, SOC_CORE_ALL, reg_val_above_64));


        /* QAX
         * - enable routing for ROO, to build native link layer
         * - set L2_LIF per outlif type (outlif prefix)
         * - set is L2_LIF per outlif profile 
         */
         
        if (SOC_IS_JERICHO_PLUS(unit)) {
            uint64 fld_val64;
            uint32 outlif_profile; 
            SHR_BITDCL mask = 0; 
            COMPILER_64_SET(fld_val64, 0, 0);

            /* enable routing if forwarding is IPVx_xC */
            fld_val = 0; 
            fld_val = ((1 << ARAD_PP_FWD_CODE_IPV4_MC) | (1 << ARAD_PP_FWD_CODE_IPV4_UC) 
                       | (1 << ARAD_PP_FWD_CODE_IPV6_MC) | (1 << ARAD_PP_FWD_CODE_IPV6_UC)); 
            SOCDNX_IF_ERR_EXIT(WRITE_EPNI_CFG_ROUTING_PER_HEADER_CODE_ENABLEr(unit, fld_val));

            /* set L2_LIF per outlif type:
               IP, DATA, MPLS, TRILL, MIM may be L2_LIF */
            fld_val = 0; 

            fld_val = (1 << EPNI_ETPP_EES_ACTION_TYPE_DATA) | (1 << EPNI_ETPP_EES_ACTION_TYPE_MPLS) | (1 << EPNI_ETPP_EES_ACTION_TYPE_IP_TUNNEL)
                    | (1 << EPNI_ETPP_EES_ACTION_TYPE_ISID) | (1 << EPNI_ETPP_EES_ACTION_TYPE_TRILL) | (1 << EPNI_ETPP_EES_ACTION_TYPE_LL_ARP);

            SOCDNX_IF_ERR_EXIT(WRITE_EPNI_CFG_ENTRY_TYPE_L_2_LIFr(unit, fld_val));


            /* set L2_LIF per outlif profile */

            /* get is L2_LIF bit allocated by occ manager */
            SOCDNX_IF_ERR_EXIT(
               arad_pp_occ_mgmt_get_app_mask(unit, SOC_OCC_MGMT_TYPE_OUTLIF, SOC_OCC_MGMT_OUTLIF_APP_ROO_IS_L2_LIF, &mask)); 

            /* set outlif profiles containing the masked bit.
               Ex: mask: 001000 
                   For xx1xxx outlif profile values, we need to set L2_LIF */
            for (outlif_profile=0; outlif_profile < (1 << SOC_DPP_IMP_DEFS_GET(unit, outlif_profile_nof_bits)); outlif_profile++) {
                if (outlif_profile & mask) {
                    COMPILER_64_BITSET(fld_val64, outlif_profile);
                }
            }
            SOCDNX_IF_ERR_EXIT(WRITE_EPNI_CFG_OUTLIF_PROFILE_L_2_LIFr(unit, fld_val64));
        }

    }

exit:
    SOCDNX_FUNC_RETURN; 
}

STATIC uint32
  arad_pp_mgmt_functional_init_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *conf,
    SOC_SAND_IN  uint8                 silent
  )
{
  uint32
    res = SOC_SAND_OK;
    int core;
  soc_error_t new_res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_FUNCTIONAL_INIT);


  res = arad_pp_dbal_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_tbl_access_em_step_tbls_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 365, exit);

  res = arad_pp_lem_access_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  
  res = arad_pp_eg_ac_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  res = arad_pp_eg_encap_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = arad_pp_eg_filter_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  res = arad_pp_eg_qos_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

  res = arad_pp_eg_vlan_edit_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

  res = arad_pp_frwrd_ilm_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);


#if defined(INCLUDE_L3)
    {
      uint8 is_routing_enabled;

      res = arad_pp_sw_db_ipv4_is_routing_enabled(unit, &is_routing_enabled);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 105, exit);
      if(is_routing_enabled)
      {
          res = arad_pp_frwrd_fec_init_unsafe(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

          res = arad_pp_frwrd_ipv4_init_unsafe(unit);
          SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
      }
    }
#endif  /* INCLUDE_L3 */

  res = arad_pp_frwrd_mact_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);

  res = arad_pp_frwrd_mact_mgmt_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);

  res = arad_pp_lag_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);

  res = arad_pp_lif_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

  new_res = arad_pp_lif_cos_init_unsafe(unit);
  if (new_res != SOC_E_NONE) {
    SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 210, exit);
  }

  if (SOC_DPP_CONFIG(unit)->trill.mode) {
    res = arad_pp_frwrd_trill_init_unsafe(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);
  }

  res = arad_pp_lif_ing_vlan_edit_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit);

  res = arad_pp_lif_table_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 230, exit);

    res = arad_pp_llp_cos_init_unsafe(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 240, exit);

  res = arad_pp_llp_filter_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);

  res = arad_pp_llp_parse_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 270, exit);

  res = arad_pp_llp_sa_auth_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 280, exit);

  res = arad_pp_llp_trap_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 290, exit);

  res = arad_pp_llp_vid_assign_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 300, exit);

  res = arad_pp_mpls_term_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 320, exit);

  res = arad_pp_port_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);

  res = arad_pp_rif_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 360, exit);

  if (SOC_IS_JERICHO(unit)) {
	  res = jer_pp_metering_init(unit);
  }
  else{
	  res = arad_pp_metering_init_unsafe(unit);
  }
  SOC_SAND_CHECK_FUNC_RESULT(res, 310, exit);

  res = arad_pp_mymac_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 330, exit);

    if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable == TRUE) {
        /* Already called if TM */
        res = arad_pp_vtt_init(unit);
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
    }

  if (SOC_DPP_CONFIG(unit)->arad->init.pp_enable == TRUE) {
    /* Already called if TM */
    res = arad_pp_trap_mgmt_init_unsafe(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 400, exit);
  }

  BCM_DPP_CORES_ITER(BCM_CORE_ALL, core) {
    res = arad_pp_diag_init_unsafe(unit, core);
    SOC_SAND_CHECK_FUNC_RESULT(res, 405, exit);
  }
  res = arad_pp_vsi_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 410, exit);

  res = arad_pp_fp_init_unsafe(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 420, exit);

  res = arad_pp_diag_sample_enable_set_unsafe(unit, TRUE);
  SOC_SAND_CHECK_FUNC_RESULT(res, 430, exit);

  res = arad_pp_mgmt_sw_dbs_ftmh_header_set(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 440, exit);

  if (SOC_DPP_CONFIG(unit)->pp.l3_source_bind_mode != SOC_DPP_L3_SOURCE_BIND_MODE_DISABLE)
  {
    res = arad_pp_src_bind_init_unsafe(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 450, exit);
  }

#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
    uint8 enable_slb = soc_property_get(unit, spn_RESILIENT_HASH_ENABLE, 0) == 1 ? TRUE : FALSE;
    if (SOC_IS_JERICHO(unit) && enable_slb) {
        SOC_SAND_SET_ERROR_MSG((_BSL_SOCDNX_SAND_MSG(" Error: resilient_hash_enable=1 is not supported on this device\n")));
    }
    res = arad_pp_slb_init_unsafe(unit, enable_slb);
    SOC_SAND_CHECK_FUNC_RESULT(res, 460, exit);
  }
#endif /* BCM_88660_A0 */

#ifdef BCM_88675
  /* Perform module initialization that is specific for Jericho and above */
  if (SOC_IS_JERICHO(unit)) {
      res = soc_jer_pp_mgmt_functional_init(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 470, exit);
  }
#endif /* BCM_88675 */  
  if (SOC_DPP_CONFIG(unit)->pp.compression_ip6_enable)
  {
    res = arad_pp_ip6_compression_tcam_init_unsafe(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 480, exit);
  }

  /* ROO init configuration */
  res = arad_mgmt_roo_init(unit);
  SOC_SAND_CHECK_FUNC_RESULT(res, 490, exit);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (SOC_IS_JERICHO(unit)) {
      res = arad_pp_init_dynamic_kbp_result(unit);
      SOC_SAND_CHECK_FUNC_RESULT(res, 500, exit);
  }
#endif
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_functional_init_unsafe()",0,0);
}



STATIC uint32
  arad_pp_mgmt_hw_set_defaults(
    SOC_SAND_IN  int                 unit
  )
{
 uint32
   fld_val,
   nof_vrfs,
   vrf_bits,
   bits_per_depth,
   dpth_indx,
   res,
   sa_prefix_for_key_construction,
   reg_val;
 uint8
   is_routing_enabled,
   mac_in_mac_enabled;
 ARAD_PP_IPV4_LPM_MNGR_INFO
   lpm_mngr;
 ARAD_PP_MGMT_OPERATION_MODE
   *oper_mode = NULL;
 soc_reg_above_64_val_t
     fld_value,
     reg_above64_val;
 soc_reg_t
     disable_pp_reg;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_HW_SET_DEFAULTS);

  SOC_REG_ABOVE_64_CLEAR(reg_above64_val);
  SOC_REG_ABOVE_64_CLEAR(fld_value);

  /* set pp as enabled */
  disable_pp_reg = SOC_IS_JERICHO(unit)? ECI_GLOBAL_PP_7r: ECI_GLOBAL_1r;
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 5, exit, soc_reg32_get(unit, disable_pp_reg, REG_PORT_ANY,  0, &reg_val));
  fld_val = 0;
  soc_reg_field_set(unit, disable_pp_reg, &reg_val, DISABLE_PPf, fld_val);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg32_set(unit, disable_pp_reg, REG_PORT_ANY,  0, reg_val));

  if(SOC_IS_ARADPLUS_AND_BELOW(unit)){
      res = arad_pp_sw_db_ipv4_is_routing_enabled(unit, &is_routing_enabled);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
      if(is_routing_enabled) {
          /* In Jericho, do not use the VRF logic */
          res = sw_state_access[unit].dpp.soc.arad.pp.ipv4_info.nof_vrfs.get(unit,&nof_vrfs);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      }
  }
  else{
      nof_vrfs = 0;
  }

  ARAD_ALLOC(oper_mode, ARAD_PP_MGMT_OPERATION_MODE, 1, "arad_pp_mgmt_hw_set_defaults.oper_mode");
  res = arad_pp_mgmt_operation_mode_get_unsafe(
          unit,
          oper_mode
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  reg_val = 0;
  if (SOC_IS_JERICHO(unit)) {
    /* fix destination encoding for learned system ports to be as in ARAD */
    soc_reg_field_set(unit, ECI_GLOBAL_PP_8r, &reg_val, SRC_SYSTEM_PORT_ID_PREFIXf, ARAD_PP_DESTINATION_SYSTEM_PORT_PREFIX);
  }
  /* set MIM VSI*/
  soc_reg_field_set(unit, ECI_GLOBAL_PP_8r, &reg_val, MAC_IN_MAC_VSIf, oper_mode->p2p_info.mim_vsi);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_ECI_GLOBAL_PP_8r(unit,reg_val));
  /* WRITE_EGQ_REG_00DAr(unit, REG_PORT_ANY,fld_val);
  WRITE_EPNI_REG_00DAr(unit, REG_PORT_ANY,fld_val); */

  /* set ether type  for MPLS*/
  /* for ingress parsing*/
  reg_val = 0;
  soc_reg_field_set(unit, ECI_GLOBAL_PP_6r, &reg_val, ETHERTYPE_MPLS_0f, oper_mode->mpls_info.mpls_ether_types[0]);
  soc_reg_field_set(unit, ECI_GLOBAL_PP_6r, &reg_val, ETHERTYPE_MPLS_1f, oper_mode->mpls_info.mpls_ether_types[1]);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, WRITE_ECI_GLOBAL_PP_6r(unit,reg_val));

  
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, READ_EGQ_ETHERNET_TYPESr(unit, REG_PORT_ANY, reg_above64_val));
  /* Set the EGQ MPLS EtherType */
  fld_value[0] = oper_mode->mpls_info.mpls_ether_types[0];
  ARAD_FLD_TO_REG_ABOVE_64(EGQ_ETHERNET_TYPESr, ETH_TYPE_0f, fld_value, reg_above64_val, 70, exit);
  fld_value[0] = oper_mode->mpls_info.mpls_ether_types[1];
  ARAD_FLD_TO_REG_ABOVE_64(EGQ_ETHERNET_TYPESr, ETH_TYPE_1f, fld_value, reg_above64_val, 80, exit);
  /* Set CNM default EtherType */
  fld_value[0] = 0x22E7;
  ARAD_FLD_TO_REG_ABOVE_64(EGQ_ETHERNET_TYPESr, ETH_TYPE_2f, fld_value, reg_above64_val, 85, exit);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 90, exit, WRITE_EGQ_ETHERNET_TYPESr(unit, REG_PORT_ANY, reg_above64_val));



  /* Set Service-type For PWE P2P and MP */
  soc_reg_field_set(unit, IHP_SERVICE_TYPE_VALUESr, &reg_val, SERVICE_TYPE_LABEL_PWE_P2Pf, 0x7);
  soc_reg_field_set(unit, IHP_SERVICE_TYPE_VALUESr, &reg_val, SERVICE_TYPE_LABEL_PWE_MPf, 0x3);
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, WRITE_IHP_SERVICE_TYPE_VALUESr(unit, REG_PORT_ANY,reg_val));

  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) { /* no such fields in Jericho */
      /* Set Parser leaf context action with no update in case of upper protocol over MPLS */
      reg_val = 0;
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit,READ_IHP_VTT_GENERAL_CONFIGS_0r(unit, REG_PORT_ANY, &reg_val));
      soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_0r, &reg_val, IPV4_OVER_MPLS_PARSER_LEAF_CONTEXT_MASKf, 0xF);
      soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_0r, &reg_val, IPV6_OVER_MPLS_PARSER_LEAF_CONTEXT_MASKf, 0xF);
      soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_0r, &reg_val, ETH_OVER_PWE_PARSER_LEAF_CONTEXT_MASKf , 0xF);
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 120, exit,WRITE_IHP_VTT_GENERAL_CONFIGS_0r(unit, REG_PORT_ANY,reg_val));
  }


  /* set decrement TTL 
    0x0 - ACTION_TYPE_DATA
    0x1 - ACTION_TYPE_MPLS
    0x2 - ACTION_TYPE_IP_TUNNEL
    0x3 - ACTION_TYPE_ISID
    0x4 - ACTION_TYPE_OUT_RIF
    0x5 - ACTION_TYPE_TRILL
    0x6 - ACTION_TYPE_LL_ARP
    0x7 - ACTION_TYPE_AC
  */
  fld_val = 0x56; /* out-rif, LL, IP, MPLS*/
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 130, exit, WRITE_EPNI_TTL_DECREMENT_ENABLEr(unit, REG_PORT_ANY,fld_val));

  /* ipv4 setting */
  /* init number of VRFs for LPM lookup*/
  if(nof_vrfs > 0)
  {
    vrf_bits = soc_sand_log2_round_up(nof_vrfs);
    reg_val = 0;

    res = arad_pp_sw_db_ipv4_lpm_mngr_get(
            unit,
            &lpm_mngr
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);


    for (dpth_indx = 0 ; dpth_indx < 4; ++dpth_indx)
    {
      bits_per_depth = lpm_mngr.init_info.nof_bits_per_bank[dpth_indx+1];
      reg_val = reg_val | (bits_per_depth<<(dpth_indx*3));
    }
    soc_reg_field_set(unit,IHB_LPM_QUERY_CONFIGURATIONr,&reg_val, VRF_DEPTHf, vrf_bits);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 150, exit, WRITE_IHB_LPM_QUERY_CONFIGURATIONr(unit,reg_val));
  }

  sa_prefix_for_key_construction = ARAD_PP_LEM_ACCESS_KEY_TYPE_PREFIX_SA_AUTH;
  if (SOC_IS_JERICHO(unit)) {
      /* SA authentication has a 10 bit prefix. Push the 4 bits value to the MSBs of the 10 bits value */
      sa_prefix_for_key_construction = sa_prefix_for_key_construction << 6;
  }

  /* Set sa-based prefix */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  160,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_LINK_LAYER_LOOKUP_CFGr, SOC_CORE_ALL, 0, SA_BASED_VID_PREFIXf,  sa_prefix_for_key_construction));
  
  /* if mac in mac is enabled, set bit 7 */  
  res = arad_pp_is_mac_in_mac_enabled(
      unit,
      &mac_in_mac_enabled
      );
  SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);

  /*
  * set AUX mode at egress
  */
  if (oper_mode->split_horizon_filter_enable)
  {
    fld_val = 1; /* Split Horizon */
  }
  else if (mac_in_mac_enabled)
  {
    fld_val = 2; /* Mac in Mac (Provider Backbone Bridging) */
  }
  else
  {
    fld_val = 0; /* Private VLAN */
  } 
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  180,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EGQ_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUXILIARY_DATA_TABLE_MODEf,  fld_val));
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  190,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, EPNI_AUXILIARY_DATA_TABLEr, SOC_CORE_ALL, 0, AUXILIARY_DATA_TABLE_MODEf,  fld_val));
  if (!mac_in_mac_enabled) {
      /* Change the default MY BMAC MC to 0 in case MIM is disabled */
      fld_val = 0;
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  195,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_MY_B_MAC_MCr, SOC_CORE_ALL, 0, MY_B_MAC_MCf,  fld_val));
  }
#ifdef BCM_JERICHO_SUPPORT 
  if (SOC_IS_JERICHO(unit))  {
    /* The port default VLAN translation Advanced mode is enabed by default. */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 200, exit,READ_IHP_VTT_GENERAL_CONFIGS_0r(unit, REG_PORT_ANY, &reg_val));
    soc_reg_field_set(unit, IHP_VTT_GENERAL_CONFIGS_0r, &reg_val, FORCE_VEC_FROM_LIFf, 0x1);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 210, exit,WRITE_IHP_VTT_GENERAL_CONFIGS_0r(unit, REG_PORT_ANY,reg_val));

    /* Configure number of cycles between lookup to reply in KAPS */
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  220,  exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IHP_FLP_GENERAL_CFGr, REG_PORT_ANY, 0, FLP_KAPS_DELAY_IS_EVENf,  SOC_IS_JERICHO_PLUS(unit) ? (SOC_IS_QUX(unit) ? 1 : 0) : 1));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  220,  exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IHB_PMF_GENERALr, REG_PORT_ANY, 0, PMF_KAPS_DELAY_IS_EVENf,  SOC_IS_JERICHO_PLUS(unit) ? (SOC_IS_QUX(unit) ? 1 : 0) : 1));

    if (SOC_IS_JERICHO_PLUS(unit))
    {
        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  220,  exit, ARAD_REG_ACCESS_ERR, soc_reg_above_64_field32_modify(unit, IHB_PMF_GENERALr, REG_PORT_ANY, 0, ENABLE_DESTINATION_ACTION_TRAP_ENCODINGf, 1));
    }
  }
#endif
exit:
  ARAD_FREE(oper_mode);
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_hw_set_defaults()",0,0);
}
/*********************************************************************
*     Initialize the device.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase1_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *conf,
    SOC_SAND_IN  uint8                 silent
  )
{
  uint32
    res;
  uint32
    stage_id = 0;
  soc_error_t
      new_rv;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(conf);

  if (!silent)
  {
    LOG_VERBOSE(BSL_LS_SOC_INIT,
                (BSL_META_U(unit,
                            "   Phase 1 : PP initialization: device %u\n\r"), unit));
  }

  /* To be removed once TM init is set in the emulation */
  if (SOC_IS_JERICHO(unit)) {
      new_rv = arad_tbl_access_init_unsafe(unit);
      if (new_rv != SOC_E_NONE) {
        SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 55555, exit);
      }
  }

  /************************************************************************/
  /* 1. Per functional module, perform initializations                    */
  /* covered by module's functionality.                                   */
  /************************************************************************/
  res = arad_pp_mgmt_functional_init_unsafe(
          unit,
          conf,
          silent
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  ARAD_PP_INIT_PRINT_ADVANCE("arad_pp_mgmt_functional_init", 1);

  /************************************************************************/
  /* 2. Set registers not covered in any functional module,               */
  /* with default values different from hardware defaults                 */
  /************************************************************************/
  res = arad_pp_mgmt_hw_set_defaults(
          unit
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  ARAD_PP_INIT_PRINT_ADVANCE("arad_pp_mgmt_hw_set_defaults", 1);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_init_sequence_phase1_unsafe()", 0, 0);
}

uint32
  arad_pp_mgmt_init_sequence_phase1_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *conf
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1_VERIFY);

  SOC_SAND_CHECK_NULL_INPUT(conf);
  SOC_SAND_MAGIC_NUM_VERIFY(conf);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_init_sequence_phase1_verify()", 0, 0);
}

/*********************************************************************
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase2_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *conf,
    SOC_SAND_IN  uint8                 silent
  )
{
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2_UNSAFE);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_init_sequence_phase2_unsafe()", 0, 0);
}

uint32
  arad_pp_mgmt_init_sequence_phase2_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *conf
  )
{
  uint32
    res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR( "error in arad_pp_mgmt_init_sequence_phase2_verify()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
*     arad_pp_api_init module.
*     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_init_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_init;
}

/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_init module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_init_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_init;
}


/*********************************************************************
*     Initialize the device.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase1(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE1_CONF    *conf,
    SOC_SAND_IN  uint8                 silent
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE1);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(conf);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_init_sequence_phase1_unsafe(
          unit,
          conf,
          silent
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_init_sequence_phase1()", 0, 0);
}

/*********************************************************************
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_mgmt_init_sequence_phase2(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  ARAD_PP_INIT_PHASE2_CONF    *conf,
    SOC_SAND_IN  uint8                 silent
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_MGMT_INIT_SEQUENCE_PHASE2);

  SOC_SAND_CHECK_DRIVER_AND_DEVICE;

  SOC_SAND_CHECK_NULL_INPUT(conf);

  SOC_SAND_TAKE_DEVICE_SEMAPHORE;

  res = arad_pp_mgmt_init_sequence_phase2_unsafe(
          unit,
          conf,
          silent
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit_semaphore);

exit_semaphore:
  SOC_SAND_GIVE_DEVICE_SEMAPHORE;
  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_mgmt_init_sequence_phase2()", 0, 0);
}

void
  ARAD_PP_INIT_PHASE1_CONF_clear(
    SOC_SAND_OUT ARAD_PP_INIT_PHASE1_CONF *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_PP_INIT_PHASE1_CONF));
  info->cpu_sys_port = 0;
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}
  
void
  ARAD_PP_INIT_PHASE2_CONF_clear(
    SOC_SAND_OUT ARAD_PP_INIT_PHASE2_CONF *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  soc_sand_os_memset(info, 0x0, sizeof(ARAD_PP_INIT_PHASE2_CONF));
  SOC_SAND_MAGIC_NUM_SET;
exit:
  SOC_SAND_VOID_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */


