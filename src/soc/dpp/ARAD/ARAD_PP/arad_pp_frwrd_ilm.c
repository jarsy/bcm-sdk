#include <soc/mcm/memregs.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/hwstate/hw_log.h>
#include <soc/dcmn/dcmn_crash_recovery.h>
#endif /* CRASH_RECOVERY_SUPPORT */

#if defined(BCM_88650_A0)
/* $Id: arad_pp_frwrd_ilm.c,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FORWARD

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/PPD/ppd_api_frwrd_ilm.h>

#include <soc/dpp/ARAD/arad_tbl_access.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_general.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ilm.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#endif
#include <soc/dpp/drv.h>


#ifdef PLISIM
  #include <sim/dpp/ChipSim/chip_sim_em.h>
#else
  #include <soc/dpp/ARAD/arad_sim_em.h>
#endif


/* } */
/*************
 * DEFINES   *
 *************/
/* { */
/* EXP nof bits */
#define ARAD_PP_FRWRD_ILM_EXP_NOF_BITS      (3)

#define ARAD_PP_IHB_MPLS_EXP_REG_NOF_FLDS   (8)


/* } */
/*************
 * MACROS    *
 *************/
/* { */

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
  Arad_pp_procedure_desc_element_frwrd_ilm[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_BLOCK),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_BLOCK_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_BLOCK_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_BLOCK_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_TABLE_CLEAR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_TABLE_CLEAR_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_TABLE_CLEAR_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_TABLE_CLEAR_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FRWRD_ILM_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */

  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC SOC_ERROR_DESC_ELEMENT
  Arad_pp_error_desc_element_frwrd_ilm[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_FRWRD_ILM_SUCCESS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FRWRD_ILM_SUCCESS_OUT_OF_RANGE_ERR",
    "The parameter 'success' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_NOF_SUCCESS_FAILURES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */

  {
    SOC_PPC_FRWRD_ILM_KEY_INPORT_NOT_MASKED_ERR,
    "SOC_PPC_FRWRD_ILM_KEY_INPORT_NOT_MASKED_ERR",
    "If by the global setting soc_ppd_frwrd_ilm_glbl_info_set, the port is masked, \n\r"
    "then the inport field of ILM key has to be zero \n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FRWRD_ILM_KEY_INRIF_NOT_MASKED_ERR,
    "SOC_PPC_FRWRD_ILM_KEY_INRIF_NOT_MASKED_ERR",
    "If by the global setting soc_ppd_frwrd_ilm_glbl_info_set, the port is masked, \n\r"
    "then the inrif field of ILM key has to be zero \n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FRWRD_ILM_EEI_NOT_MPLS_ERR,
    "ARAD_PP_FRWRD_ILM_EEI_NOT_MPLS_ERR",
    "EEI in ILM add command must be of type MPLS. \n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FRWRD_ILM_KEY_MAPPED_EXP_NOT_ZERO_ERR,
    "SOC_PPC_FRWRD_ILM_KEY_MAPPED_EXP_NOT_ZERO_ERR",
    "Mapped_exp ILM key field should be zero for labels not in the ELSP range . \n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FRWRD_ILM_KEY_MASK_NOT_SUPPORTED_ERR,
    "SOC_PPC_FRWRD_ILM_KEY_MASK_NOT_SUPPORTED_ERR",
    "In ILM key, Inrif and Port are always masked. This setting can not be changed in this SW release. \n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },

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

uint32
  arad_pp_frwrd_ilm_init_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  SOC_PPC_FRWRD_ILM_GLBL_INFO
    glbl_info;
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_PPC_FRWRD_ILM_GLBL_INFO_clear(&glbl_info);

 /*
  * default set lookup to be according to label only.
  */
  glbl_info.key_info.mask_inrif = TRUE;
  glbl_info.key_info.mask_port = TRUE;

  res = arad_pp_frwrd_ilm_glbl_info_set_unsafe(
          unit,
          &glbl_info
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_init_unsafe()", 0, 0);
}

STATIC
  uint32
    arad_pp_frwrd_ilm_key_verify(
      SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FRWRD_ILM_GLBL_INFO
    glbl_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = SOC_PPC_FRWRD_ILM_KEY_verify(unit, ilm_key);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  
  res = soc_ppd_frwrd_ilm_glbl_info_get(
          unit,
          &glbl_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if((glbl_info.key_info.mask_port) && (ilm_key->in_local_port != 0))
  {
    SOC_SAND_SET_ERROR_CODE(SOC_PPC_FRWRD_ILM_KEY_INPORT_NOT_MASKED_ERR, 40, exit);
  }
  if((glbl_info.key_info.mask_inrif) && (ilm_key->inrif != 0))
  {
    SOC_SAND_SET_ERROR_CODE(SOC_PPC_FRWRD_ILM_KEY_INRIF_NOT_MASKED_ERR, 50, exit);
  }
  
  if((ilm_key->in_label < glbl_info.elsp_info.labels_range.start) ||
     (ilm_key->in_label > glbl_info.elsp_info.labels_range.end))
  {
    if(ilm_key->mapped_exp != 0)
    {
      SOC_SAND_SET_ERROR_CODE(SOC_PPC_FRWRD_ILM_KEY_MAPPED_EXP_NOT_ZERO_ERR, 50, exit);
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_key_verify()", 0, 0);
}

/* build lem access key for ILM host address */
void
    arad_pp_frwrd_ilm_lem_key_build(
       SOC_SAND_IN  int                                 unit,
      SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY  *ilm_key,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_KEY *key
    )
{
  uint32 num_bits;

  ARAD_PP_LEM_ACCESS_KEY_clear(key);
  key->nof_params = ARAD_PP_LEM_ACCESS_KEY_NOF_PARAMS_FOR_ILM;
  key->type = ARAD_PP_LEM_ACCESS_KEY_TYPE_ILM;

  num_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM0_IN_BITS_FOR_ILM;
  key->param[0].nof_bits = (uint8)num_bits;
  key->param[0].value[0] = ilm_key->in_label;
  
  num_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM1_IN_BITS_FOR_ILM;
  key->param[1].nof_bits = (uint8)num_bits;
  key->param[1].value[0] = ilm_key->mapped_exp;

  key->param[2].value[0] = ilm_key->in_local_port;
  if (SOC_IS_JERICHO(unit)) {
      /* in Jericho the port is 9 bits, 1bit Core-Bit and 8bits In-PP-Port */
      key->param[2].value[0] |= ilm_key->in_core << ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM;
      key->param[2].nof_bits = (uint8)(ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM_JERICHO);
  } else {
      key->param[2].nof_bits = (uint8)ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM;
  }

  num_bits = ARAD_PP_LEM_ACCESS_KEY_PARAM3_IN_BITS_FOR_ILM;
  key->param[3].nof_bits = (uint8)num_bits;
  key->param[3].value[0] = ilm_key->inrif;

  key->prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_ILM;
  key->prefix.value = ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_ILM;
}

/* parse lem access key for IpV4 host address
*/
  void
    arad_pp_frwrd_ilm_lem_key_parse(
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_KEY *key,
      SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
    )
{
  ilm_key->in_label = key->param[0].value[0];
  ilm_key->mapped_exp = (SOC_SAND_PP_MPLS_EXP)key->param[1].value[0];
  ilm_key->in_local_port = key->param[2].value[0] & ((1 << ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM) -1);
  ilm_key->in_core = (key->param[2].value[0]  >> ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM);
  ilm_key->inrif = key->param[3].value[0];
}

/* build lem access payload for IpV4 host address */
STATIC
  uint32
    arad_pp_frwrd_ilm_lem_payload_build(
      SOC_SAND_IN int                                      unit,
      SOC_SAND_IN SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val,
      SOC_SAND_OUT ARAD_PP_LEM_ACCESS_PAYLOAD                 *payload
    )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(payload);

  /*
  *	Get the encoded destination and ASD
  */
  res = arad_pp_fwd_decision_in_buffer_build(
          unit,
          ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE_ILM,
          ilm_val,
          &payload->dest,
          &payload->asd
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  payload->age = 0;
  payload->is_dynamic = FALSE;

  payload->flags = ARAD_PP_FWD_DECISION_PARSE_DEST;

  if(ilm_val->additional_info.eei.type != SOC_PPC_EEI_TYPE_EMPTY) {
    payload->flags |= ARAD_PP_FWD_DECISION_PARSE_EEI;
  }
  else if(ilm_val->additional_info.outlif.type != SOC_PPC_OUTLIF_ENCODE_TYPE_NONE) {
    payload->flags |= ARAD_PP_FWD_DECISION_PARSE_OUTLIF;
  }
  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_lem_payload_build()", 0, 0);
}

/* parse lem access payload for IpV4 host address
*/
  void
    arad_pp_frwrd_ilm_lem_payload_parse(
      SOC_SAND_IN int                                      unit,
      SOC_SAND_IN ARAD_PP_LEM_ACCESS_PAYLOAD                  *payload,
      SOC_SAND_OUT  SOC_PPC_FRWRD_DECISION_INFO               *ilm_val
    )
{
  arad_pp_fwd_decision_in_buffer_parse(
    unit,    
    payload->dest,
    payload->asd,
    payload->flags,
    ilm_val    
  );
}

/*********************************************************************
*     Setting global information of the ILM (ingress label
 *     mapping) (including ELSP and key building information)
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_glbl_info_set_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK,
    fld_val = 0,
    fld_ndx,
    data32;
  uint64
    data64;
    
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  COMPILER_64_ZERO(data64);

  /* ELSP range { */
  fld_val = glbl_info->elsp_info.labels_range.start;
  soc_reg64_field32_set(unit, IHP_LSR_ELSP_RANGEr, &data64, ELSP_RANGE_MINf, fld_val);

  fld_val = glbl_info->elsp_info.labels_range.end;
  soc_reg64_field32_set(unit, IHP_LSR_ELSP_RANGEr, &data64, ELSP_RANGE_MAXf, fld_val);

  SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHP_LSR_ELSP_RANGEr(unit, SOC_CORE_ALL, data64));
  /* ELSP range } */

  /* ELSP EXP MAP { */
  data32 = 0;
  for(fld_ndx = 0; fld_ndx < ARAD_PP_IHB_MPLS_EXP_REG_NOF_FLDS; ++fld_ndx) 
  {
    fld_val = glbl_info->elsp_info.exp_map_tbl[fld_ndx];
    SHR_BITCOPY_RANGE(&data32,fld_ndx*ARAD_PP_FRWRD_ILM_EXP_NOF_BITS,&fld_val,0,ARAD_PP_FRWRD_ILM_EXP_NOF_BITS);
  }  
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IHP_MPLS_EXP_MAPr(unit, SOC_CORE_ALL, data32));
  /* ELSP EXP MAP } */

  res = sw_state_access[unit].dpp.soc.arad.pp.ilm_info.mask_inrif.set(unit, glbl_info->key_info.mask_inrif);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = sw_state_access[unit].dpp.soc.arad.pp.ilm_info.mask_port.set(unit, glbl_info->key_info.mask_port);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

#ifdef BCM_88660
  if (SOC_IS_ARADPLUS(unit)) {
     
    /* Global pipe mode */
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHP_VTT_GENERAL_CONFIGS_1r, SOC_CORE_ALL, 0, ENABLE_MPLS_PIPEf,  glbl_info->short_pipe_enable));
  }
 
#endif /* BCM_88660 */

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_glbl_info_set_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_glbl_info_set_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FRWRD_ILM_GLBL_INFO_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FRWRD_ILM_GLBL_INFO, glbl_info, 10, exit);  
   
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_glbl_info_set_verify()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_glbl_info_get_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_glbl_info_get_verify()", 0, 0);
}

/*********************************************************************
*     Setting global information of the ILM (ingress label
 *     mapping) (including ELSP and key building information)
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_glbl_info_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_GLBL_INFO                 *glbl_info
  )
{
  uint32
    res = SOC_SAND_OK,
    fld_val = 0,
    fld_ndx,
    data32;
  uint64
    data64;
   
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FRWRD_ILM_GLBL_INFO_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(glbl_info);

  SOC_PPC_FRWRD_ILM_GLBL_INFO_clear(glbl_info);

  COMPILER_64_ZERO(data64);

  /* ELSP range { */
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, READ_IHP_LSR_ELSP_RANGEr(unit, 0, &data64));

  fld_val = soc_reg64_field32_get(unit, IHP_LSR_ELSP_RANGEr, data64, ELSP_RANGE_MINf);
  glbl_info->elsp_info.labels_range.start = fld_val;

  fld_val = soc_reg64_field32_get(unit, IHP_LSR_ELSP_RANGEr, data64, ELSP_RANGE_MAXf);
  glbl_info->elsp_info.labels_range.end = fld_val;
  /* ELSP range } */

  /* ELSP EXP MAP { */
  data32 = 0;  
  SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, READ_IHP_MPLS_EXP_MAPr(unit, 0, &data32));
  for(fld_ndx = 0; fld_ndx < ARAD_PP_IHB_MPLS_EXP_REG_NOF_FLDS; ++fld_ndx) 
  {
    fld_val = 0;
    SHR_BITCOPY_RANGE(&fld_val,0,&data32,fld_ndx*ARAD_PP_FRWRD_ILM_EXP_NOF_BITS,ARAD_PP_FRWRD_ILM_EXP_NOF_BITS);
    glbl_info->elsp_info.exp_map_tbl[fld_ndx] = fld_val;    
  }    
  /* ELSP EXP MAP } */

  res = sw_state_access[unit].dpp.soc.arad.pp.ilm_info.mask_inrif.get(unit, &glbl_info->key_info.mask_inrif);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

  res = sw_state_access[unit].dpp.soc.arad.pp.ilm_info.mask_port.get(unit, &glbl_info->key_info.mask_port);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_glbl_info_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Add mapping from incoming label to destination and MPLS
 *     command.
 *     Details: in the H file. (search for prototype)
*********************************************************************/

/* Fills qual_vals according the key elemenets from ilm_key */
void
    arad_pp_frwrd_ilm_quals_fill(int unit, SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY *ilm_key, SOC_PPC_FP_QUAL_VAL *qual_vals)
{
    int i = 0;
    char *propval;
    
    SOCDNX_INIT_FUNC_DEFS;

    DBAL_QUAL_VALS_CLEAR(qual_vals);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    /* Check if Exteral Lookup device is used */
    if((ARAD_KBP_ENABLE_MPLS)) {
        qual_vals[0].type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
        qual_vals[0].val.arr[0] = ilm_key->in_label;
        qual_vals[0].is_valid.arr[0] = 0x000FFFFF; /* 20b mask */
    }
    else
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    {
        propval = soc_property_get_str(unit, spn_MPLS_CONTEXT);  
        if (propval && sal_strcmp(propval, "port") == 0) {
            /* in_port defined */
            qual_vals[i].val.arr[0] = ilm_key->in_local_port;
            if (SOC_IS_JERICHO(unit)) {
                qual_vals[i].val.arr[0] |= ilm_key->in_core << ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM;
            }
            qual_vals[i++].type = SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT;
        } else if (propval && sal_strcmp(propval, "interface") == 0){
            /* in rif defined */
            qual_vals[i].val.arr[0] = ilm_key->inrif;
            qual_vals[i++].type = SOC_PPC_FP_QUAL_IRPP_IN_RIF;
        } else if (propval && sal_strcmp(propval, "vrf") == 0) {
            /* vrf defined */
            qual_vals[i].val.arr[0] = ilm_key->vrf;
            qual_vals[i++].type = SOC_PPC_FP_QUAL_IRPP_VRF;
        }
        qual_vals[i].val.arr[0] = ilm_key->in_label;
        qual_vals[i++].type = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
        qual_vals[i].val.arr[0] = ilm_key->mapped_exp;
        qual_vals[i++].type = SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_MPLS_EXP;
    }

    SOCDNX_FUNC_RETURN_VOID;
}

/* Fills ilm_key according the key elemenets from qual_vals */
void
    arad_pp_frwrd_quals_ilm_fill(int unit, SOC_SAND_IN SOC_PPC_FP_QUAL_VAL *qual_vals, SOC_PPC_FRWRD_ILM_KEY *ilm_key)
{
    int i = 0;
    
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PPC_FRWRD_ILM_KEY_clear(ilm_key);

    for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
        switch (qual_vals[i].type)
        {
            case SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT:
                ilm_key->in_local_port = qual_vals[i].val.arr[0];
                ilm_key->in_core = (qual_vals[i].val.arr[0]  >> ARAD_PP_LEM_ACCESS_KEY_PARAM2_IN_BITS_FOR_ILM);
                break;
            case SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD:
                ilm_key->in_label = qual_vals[i].val.arr[0];
                break;
            case SOC_PPC_FP_QUAL_IPR2DSP_6EQ7_MPLS_EXP:
                ilm_key->mapped_exp = qual_vals[i].val.arr[0];
                break;
            case SOC_PPC_FP_QUAL_IRPP_IN_RIF:
                ilm_key->inrif = qual_vals[i].val.arr[0];
                break;
            case SOC_PPC_FP_QUAL_IRPP_VRF:
                ilm_key->vrf = qual_vals[i].val.arr[0];
                break;
            default:
                break;
        }
    }


    SOCDNX_FUNC_RETURN_VOID;
}


uint32
  arad_pp_frwrd_ilm_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY               *ilm_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO         *ilm_val,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE            *success
  )
{
  uint32 res = SOC_SAND_OK;
  ARAD_PP_LEM_ACCESS_PAYLOAD payload;
  SOC_DPP_DBAL_SW_TABLE_IDS table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM;
  SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(ilm_key);
  SOC_SAND_CHECK_NULL_INPUT(ilm_val);
  SOC_SAND_CHECK_NULL_INPUT(success);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* Check if Exteral Lookup device is used */
  if(ARAD_KBP_ENABLE_MPLS) {
      table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP;
  }
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);

  arad_pp_frwrd_ilm_quals_fill(unit, ilm_key, qual_vals);

  arad_pp_frwrd_ilm_lem_payload_build(unit, ilm_val, &payload);

  res = arad_pp_dbal_entry_add(unit, table_id, qual_vals, 0/*priority*/, &payload, success);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_add_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key,
    SOC_SAND_IN  SOC_PPC_FRWRD_DECISION_INFO                 *ilm_val
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_ADD_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FRWRD_DECISION_INFO, ilm_val, 20, exit);

  res = arad_pp_frwrd_ilm_key_verify(  
          unit,        
          ilm_key
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_add_verify()", 0, 0);
}

/*********************************************************************
*     Gets the value (destination and MPLS command) the
 *     incoming label key is associated with.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY               *ilm_key,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO         *ilm_val,
    SOC_SAND_OUT uint8                               *found
  )
{
  uint32 res = SOC_SAND_OK;
  uint32 priority = 0;
  ARAD_PP_LEM_ACCESS_PAYLOAD payload;
  SOC_DPP_DBAL_SW_TABLE_IDS table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM;
  SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(ilm_key);
  SOC_SAND_CHECK_NULL_INPUT(ilm_val);
  SOC_SAND_CHECK_NULL_INPUT(found);

  SOC_PPC_FRWRD_DECISION_INFO_clear(ilm_val);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* Check if Exteral Lookup device is used */
  if(ARAD_KBP_ENABLE_MPLS) {
      table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP;
  }
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
  
  arad_pp_frwrd_ilm_quals_fill(unit, ilm_key, qual_vals);

  res = arad_pp_dbal_entry_get(unit, table_id, qual_vals, &payload, &priority, NULL/*hit_bit*/, found);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if(*found)
  {
    arad_pp_frwrd_ilm_lem_payload_parse(unit, &payload, ilm_val);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_get_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY                       *ilm_key
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_GET_VERIFY);

  res = arad_pp_frwrd_ilm_key_verify(
          unit,
          ilm_key);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_get_verify()", 0, 0);
}

/*********************************************************************
*     Gets the block of entries from the ILM DB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE        *block_range,
    SOC_SAND_OUT SOC_PPC_FRWRD_ILM_KEY               *ilm_keys,
    SOC_SAND_OUT SOC_PPC_FRWRD_DECISION_INFO         *ilm_vals,
    SOC_SAND_OUT uint32                              *nof_entries
  )
{
    uint32 res = SOC_SAND_OK, read_index = 0, tmp_nof_entries = 0;
    SOC_PPC_FRWRD_ILM_KEY ilm_key;
    ARAD_PP_LEM_ACCESS_KEY key, key_mask;
    ARAD_PP_LEM_ACCESS_KEY *read_keys = NULL;
    ARAD_PP_LEM_ACCESS_PAYLOAD *read_vals = NULL;
#if defined (BCM_ARAD_SUPPORT) || (PLISIM)
    uint32 nof_total_entries;
#endif
    SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
    SOC_DPP_DBAL_SW_TABLE_IDS table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM;
    SOC_DPP_DBAL_PHYSICAL_DB_TYPES physical_db_type;
    uint8 found;
    ARAD_PP_LEM_ACCESS_PAYLOAD payload;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_GET_BLOCK_UNSAFE);

    SOC_SAND_CHECK_NULL_INPUT(block_range);
    SOC_SAND_CHECK_NULL_INPUT(nof_entries);
    SOC_SAND_CHECK_NULL_INPUT(ilm_keys);
    SOC_SAND_CHECK_NULL_INPUT(ilm_vals);  

    *nof_entries = 0;

    SOC_PPC_FRWRD_ILM_KEY_clear(&ilm_key);
    ARAD_PP_LEM_ACCESS_KEY_clear(&key);
    ARAD_PP_LEM_ACCESS_KEY_clear(&key_mask);
  

#if defined (BCM_ARAD_SUPPORT) || (PLISIM)
  if ((SAL_BOOT_PLISIM )
#ifdef CRASH_RECOVERY_SUPPORT
       || ((SOC_IS_DONE_INIT(unit)) && BCM_UNIT_DO_HW_READ_WRITE(unit))
#endif /* CRASH_RECOVERY_SUPPORT */
     )
  {
      int i;

      read_vals = (ARAD_PP_LEM_ACCESS_PAYLOAD*)sal_alloc(sizeof(ARAD_PP_LEM_ACCESS_PAYLOAD) * (block_range->entries_to_act), "arad_pp_lem_block_get");
      if (read_vals == NULL) {
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 10, exit);
      }
      read_keys = (ARAD_PP_LEM_ACCESS_KEY*)sal_alloc(sizeof(ARAD_PP_LEM_ACCESS_KEY) * (block_range->entries_to_act), "arad_pp_lem_block_get");
      if (read_keys == NULL) {
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_MALLOC_FAIL, 15, exit);
      }

      arad_pp_lem_block_get(unit, NULL, block_range, read_keys, read_vals, &nof_total_entries);
      SOC_SAND_TBL_ITER_SET_END(&(block_range->iter));

      *nof_entries =0;
      for (i=0; i < nof_total_entries; i++) {

        if (read_keys[i].type == ARAD_PP_LEM_ACCESS_KEY_TYPE_ILM) 
        {
          arad_pp_frwrd_ilm_lem_key_parse(
            &read_keys[i],
            &ilm_keys[*nof_entries]
          );

          SOC_PPC_FRWRD_DECISION_INFO_clear(&ilm_vals[*nof_entries]);
          arad_pp_frwrd_ilm_lem_payload_parse(
            unit,
            &read_vals[i],
            &ilm_vals[*nof_entries]
          );   
          (*nof_entries)++;   
        }
      }
      SOC_SAND_EXIT_NO_ERROR;
  }
#endif

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if(ARAD_KBP_ENABLE_MPLS) {
        table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP;
    }
#endif
    res = arad_pp_dbal_iterator_init(unit, table_id, &physical_db_type);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    while ((read_index < block_range->entries_to_act) && (read_index < block_range->entries_to_scan)) {
        ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
        DBAL_QUAL_VALS_CLEAR(qual_vals);
        res = arad_pp_dbal_iterator_get_next(unit, table_id, 0/*flags*/, qual_vals, &payload, NULL/*priority*/, NULL/*hit_bit*/, &found);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(found==TRUE) {
            arad_pp_frwrd_quals_ilm_fill(unit, qual_vals, &ilm_keys[read_index]);
            SOC_PPC_FRWRD_DECISION_INFO_clear(&ilm_vals[read_index]);
            arad_pp_frwrd_ilm_lem_payload_parse(unit, &payload, &ilm_vals[read_index]);      
            read_index++;
        } else {
            break;
        }
    }

    res = arad_pp_dbal_iterator_deinit(unit, table_id);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    SOC_SAND_TBL_ITER_SET_END(&(block_range->iter));
    tmp_nof_entries = read_index;

    *nof_entries += tmp_nof_entries;


exit:
  if (read_keys) {
    soc_sand_os_free_any_size(read_keys);
  }
  if (read_vals) {
    soc_sand_os_free_any_size(read_vals);
  }
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_get_block_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE        *block_range
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_GET_BLOCK_VERIFY);

  

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_get_block_verify()", 0, 0);
}

/*********************************************************************
*     Remove incoming label key from the ILM DB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY               *ilm_key
  )
{
  uint32 res = SOC_SAND_OK;
  SOC_DPP_DBAL_SW_TABLE_IDS table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM;
  SOC_PPC_FP_QUAL_VAL qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  SOC_SAND_SUCCESS_FAILURE success;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_REMOVE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(ilm_key);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* Check if Exteral Lookup device is used */
  if(ARAD_KBP_ENABLE_MPLS) {
      table_id = SOC_DPP_DBAL_SW_TABLE_ID_LSR_KBP;
  }
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  arad_pp_frwrd_ilm_quals_fill(unit, ilm_key, qual_vals);

  res = arad_pp_dbal_entry_delete(unit, table_id, qual_vals, &success);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_remove_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY               *ilm_key
  )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_REMOVE_VERIFY);

  /* remove it if exists*/

  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_remove_verify()", 0, 0);
}

/*********************************************************************
*     Remove all keys from the ILM DB.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_frwrd_ilm_table_clear_unsafe(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32 res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_TABLE_CLEAR_UNSAFE);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* Check if External Lookup device is used */
  if(ARAD_KBP_ENABLE_MPLS) 
  {
      res = arad_pp_tcam_kbp_table_clear(unit, ARAD_KBP_FRWRD_TBL_ID_LSR);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }
  else
#endif
  { 
    /* delete ILM entries by DBAL (also handle shadow */    
    res = arad_pp_dbal_table_clear(unit, SOC_DPP_DBAL_SW_TABLE_ID_LSR_LEM);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_table_clear_unsafe()", 0, 0);
}

uint32
  arad_pp_frwrd_ilm_table_clear_verify(
    SOC_SAND_IN  int                                 unit
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FRWRD_ILM_TABLE_CLEAR_VERIFY);

  ARAD_PP_DO_NOTHING_AND_EXIT;
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_frwrd_ilm_table_clear_verify()", 0, 0);
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_frwrd_ilm module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_frwrd_ilm_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_frwrd_ilm;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_frwrd_ilm module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_frwrd_ilm_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_frwrd_ilm;
}

uint32
  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO_verify()",0,0);
}

uint32
  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO *info
  )
{
  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  
  for (ind = 0; ind < SOC_SAND_PP_NOF_BITS_IN_EXP; ++ind)
  {
    SOC_SAND_ERR_IF_ABOVE_MAX(info->exp_map_tbl[ind], SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 11, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO_verify()",0,0);
}

uint32
  SOC_PPC_FRWRD_ILM_KEY_verify(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_KEY *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->in_label, SOC_SAND_PP_MPLS_LABEL_MAX, SOC_SAND_PP_MPLS_LABEL_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->mapped_exp, SOC_SAND_PP_MPLS_EXP_MAX, SOC_SAND_PP_MPLS_EXP_OUT_OF_RANGE_ERR, 11, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->in_local_port, ARAD_PP_PORT_MAX, SOC_PPC_PORT_OUT_OF_RANGE_ERR, 12, exit);
  SOC_SAND_ERR_IF_ABOVE_NOF(info->inrif, SOC_DPP_CONFIG(unit)->l3.nof_rifs, SOC_PPC_RIF_ID_OUT_OF_RANGE_ERR, 13, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FRWRD_ILM_KEY_verify()",0,0);
}

uint32
  SOC_PPC_FRWRD_ILM_GLBL_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FRWRD_ILM_GLBL_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FRWRD_ILM_GLBL_KEY_INFO, &(info->key_info), 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FRWRD_ILM_GLBL_ELSP_INFO, &(info->elsp_info), 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FRWRD_ILM_GLBL_INFO_verify()",0,0);
}

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


#endif /* of #if defined(BCM_88650_A0) */

