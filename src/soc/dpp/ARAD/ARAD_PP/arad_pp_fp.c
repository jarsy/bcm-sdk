#include <soc/mcm/memregs.h>
#if defined(BCM_88650_A0)
/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
*/

/*************
 * INCLUDES  *
 *************/
/* { */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_FP

#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dcmn/error.h>
#include <soc/mem.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>

#include <soc/dpp/ARAD/ARAD_PP/arad_pp_framework.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_key.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_fp_fem.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ip_tcam.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_dbal.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap_access.h>

#include <soc/dpp/ARAD/arad_pmf_low_level_ce.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_db.h>
#include <soc/dpp/ARAD/arad_pmf_low_level.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_pgm.h>
#include <soc/dpp/ARAD/arad_pmf_prog_select.h>
#include <soc/dpp/ARAD/arad_pmf_low_level_fem_tag.h>
#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_tcam.h>
#include <soc/dpp/ARAD/arad_tcam_mgmt.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/drv.h>
#include <soc/shared/sand_signals.h>

/* calling BCM functions from SOC layer is unusual
 * and happens in the specific case where data should 
 * be restored from BCM sw database */ 
#include <bcm_int/dpp/field_int.h>

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
#include <soc/dpp/ARAD/arad_kbp.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps.h>
#include <soc/dpp/JER/JER_PP/jer_pp_kaps_entry_mgmt.h>
#endif

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_PP_FP_PFG_NDX_MAX                                   (SOC_PPC_FP_NOF_PFGS_ARAD-1)
#define ARAD_PP_FP_DB_ID_NDX_MAX                                 (SOC_PPC_FP_NOF_DBS-1)
#define ARAD_PP_FP_ENTRY_ID_NDX_MAX                              (SOC_SAND_UINT_MAX-1)
#define SOC_PPC_FP_FWD_TYPE_NDX_MAX                              (SOC_PPC_NOF_FP_FWD_TYPES-1)
#define ARAD_PP_FP_PORT_PROFILE_NDX_MAX                          (3)
#define SOC_PPC_FP_QUAL_TYPES_MAX                                (SOC_PPC_NOF_FP_QUAL_TYPES-1)
#define SOC_PPC_FP_ACTION_TYPES_MAX                              (SOC_PPC_NOF_FP_ACTION_TYPES_ARAD-1)
#define SOC_PPC_FP_DATABASE_TYPE_MAX                             (SOC_PPC_NOF_FP_DATABASE_TYPES-1)
#define ARAD_PP_FP_STRENGTH_MAX                                  (1023)
#define ARAD_PP_FP_VAL_MAX                                       (SOC_SAND_U32_MAX)
#define ARAD_PP_FP_PRIORITY_MAX                                  (SOC_SAND_U32_MAX)
#define ARAD_PP_FP_FLD_LSB_MAX                                   (SOC_SAND_U32_MAX)
#define ARAD_PP_FP_CST_VAL_MAX                                   (SOC_SAND_U32_MAX)
#define ARAD_PP_FP_NOF_FIELDS_MAX                                (SOC_PPC_FP_DIR_EXTR_MAX_NOF_FIELDS)
#define ARAD_PP_FP_DE_ENTRY_ID_NDX_MAX                           (63)
#define ARAD_PP_FP_DB_ID_MAX                                     (SOC_PPC_FP_NOF_DBS-1)
#define SOC_PPC_FP_CONTROL_TYPE_MAX                              (SOC_PPC_NOF_FP_CONTROL_TYPES-1)

/*
 * Control set API defines
 */
#define ARAD_PP_FP_NOF_L4OPS_RANGES                              (24)
#define ARAD_PP_FP_NOF_PKT_SZ_RANGES                             (3)
#define ARAD_PP_FP_NOF_OUT_LIF_RANGES                            (3)
#define ARAD_PP_FP_NOF_ACE_POINTERS                              (4096)
#define ARAD_PP_FP_ETHERTYPE_MAX                                 (0xFFFF)
#define ARAD_PP_FP_NEXT_PROTOCOL_IP_MAX                          (255)
#define ARAD_PP_FP_ING_TM_PORT_DATA_MAX                          (SOC_SAND_U16_MAX) /* in 16b */
#define ARAD_PP_FP_EGR_IPV4_NEXT_PROTOCOL_MIN                    (1)
#define ARAD_PP_FP_EGR_IPV4_NEXT_PROTOCOL_MAX                    (15)
#define ARAD_PP_FP_PP_PORT_PROFILE_MAX                           (3)
#define ARAD_PP_FP_PP_PORT_IN_BITS                               (8)
#define ARAD_PP_FP_OUT_LIF_IN_BITS                               SOC_DPP_DEFS_GET(unit, out_lif_nof_bits)
#define ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE                      (4)
#define ARAD_PP_FP_PRGE_VAR_IN_BITS                              (10)
#define ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR             (2)
#define ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_OUT_LIF              (2)
#define ARAD_PP_UD_HEADER_OFFSET_MAX                             (508)
#define ARAD_PP_FP_UDP_NOF_BITS_MIN                              (1)
#define ARAD_PP_FP_UDP_NOF_BITS_MAX                              (32)



#define SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_SUB_HEADER_NDX      (0)
#define SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX          (1)
#define SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX        (2)
#define SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX    (3)
#define SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_STAGE_NDX           (4)

#define ARAD_PP_FP_ACTION_TOTAL_LENGTH_IN_BITS_TCAM(unit)        (SOC_DPP_DEFS_GET(unit, tcam_action_width) * 4)
#define ARAD_PP_FP_BASE_VAL_MAX(unit)                            ((1<<SOC_DPP_DEFS_GET(unit, fem_max_action_size_nof_bits))-1)

#define SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_DIRECT_EXTRACTION          (32)
#define SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_FLP                        (160) /* Assumption of using only key-c*/
#define SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_SLB                        (8*32) 
#define SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_TCAM                       (320)
#define SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_DIRECT_TABLE_SMALL_BANK    (7)

#define ARAD_PP_FP_DIRECT_TABLE_KAPS_KEY_LENGTH                           SOC_DPP_DEFS_GET(unit, field_large_direct_lu_key_length)
#define ARAD_PP_FP_DIRECT_TABLE_KAPS_NOF_LINES                            (1 << ARAD_PP_FP_DIRECT_TABLE_KAPS_KEY_LENGTH)
#define ARAD_PP_FP_DIRECT_TABLE_KAPS_ACTION_MAX_LENGTH                    (56)




/* Number of Egress CAM program selection parameters */
#define ARAD_PP_FP_EGRESS_NOF_CAM_PARAMS    (7)


/* Max length of a KeyGebVar table */
#define ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS  (20)

/* Key Part - LSB or MSB */
#define ARAD_PP_FP_KEY_BIT_TYPE_LSB   SOC_PPC_FP_KEY_BIT_TYPE_LSB
typedef SOC_PPC_FP_KEY_BIT_TYPE_LSB_MSB   ARAD_PP_FP_KEY_BIT_TYPE_LSB_MSB;

/*  Number of Port Profiles */
#define  ARAD_PP_FP_NOF_PORT_PROFILE_NDX_MAX  	(7)

#define ARAD_TCAM_KEY_LENGTH_DIRECT_TABLE_PER_BANK(unit, bank_id)   \
  ((bank_id < SOC_DPP_DEFS_GET(unit, nof_tcam_big_banks))? SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits):SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_DIRECT_TABLE_SMALL_BANK)

#define ARAD_PP_FP_NOF_FLP2EGW_KEY_SIGNALS (5)

#define ARAD_PP_FP_NOF_PROFILE_OBJECTS(unit, control_type) ((control_type == SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE)? SOC_DPP_DEFS_GET(unit, nof_flp_programs): _BCM_DPP_NOF_PORTS_PER_CORE(unit))

#define ARAD_PP_FP_PMF_SEL_LINE_VALID  (0x4)

#define ARAD_PP_FP_IRE_PACKET_NOF_BYTES_MAX_PER_WRITE   (SOC_IS_JERICHO(unit)? 256: 32)
#define ARAD_PP_FP_IRE_PACKET_NOF_LONGS_COPY_REG        (SOC_IS_JERICHO(unit)? 16: 8)

#define ARAD_PP_FP_SUBNET_LENGTH_IPV4_MAX                        (32)
#define ARAD_PP_FP_SUBNET_LENGTH_IPV6_MAX                        (64)


/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define ARAD_PP_FP_DIAG_FLD_FILL(prm_fld, _array)  \
    ARAD_PP_DIAG_FLD_FILL(prm_fld, _array[2], _array[3], _array[0], _array[1])  

#define ARAD_PP_FP_DIAG_FLD_FILL_UNIT(prm_fld, _array_jericho, _array_arad)  \
    if (SOC_IS_JERICHO(unit)) { ARAD_PP_FP_DIAG_FLD_FILL(prm_fld, _array_jericho) } else { ARAD_PP_FP_DIAG_FLD_FILL(prm_fld, _array_arad) };

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */
/* 
 * If set, assume all the packets are identical: 
 * no need to have a packet stream in the diagnostic
 */
#define ARAD_PP_FP_DIAG_LAST_PACKET_ALL_IDENTICALS_NO_STREAM    (1)
/* 
 * If set, reset the debug registers in HW
 */
#define ARAD_PP_FP_DIAG_LAST_PACKET_RESET_DBG_PMF               (0)


typedef struct
{
  SOC_PPC_FP_PREDEFINED_ACL_KEY predefined_key;
  SOC_PPC_FP_QUAL_TYPE qual_type;
  uint32 lsb;
  uint32 length;
} ARAD_PP_FP_PREDEFINED_KEY_INFO;

typedef struct
{
  uint8 exists;
  uint32 priority;
  uint32 bank_id;
  uint32 entry_id;
  uint32 line_idx;

} ARAD_PP_FP_RESOURCE_DIAG_DB_PRIORITY;

typedef struct
{
  SOC_PPC_FP_RESOURCE_KEY key_qual[SOC_PPC_FP_NOF_CES_PER_DB_MAX];
  uint32 nof_ces;
  
} ARAD_PP_FP_RESOURCE_DIAG_DB_KEY;

typedef struct
{
  uint8 complete;
  uint32 program_id;
  uint32 cycle_id;
  uint32 lsb_msb;
  
} ARAD_PP_FP_RESOURCE_DIAG_DB_KEY_ARGS;

typedef struct
{
  uint32 new_prio;
  uint32 last_prio;
  uint32 entry_strength;
  uint32 db_id;
  uint32 pgm_id;
  uint8  is_fes;
  uint32 fes_fem_id;
  
} ARAD_PP_FP_RESOURCE_DIAG_ACTION_PRIORITY;

CONST STATIC
  SOC_PROCEDURE_DESC_ELEMENT
    Arad_pp_procedure_desc_element_fp[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_FORMAT_GROUP_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_CREATE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_CREATE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_CREATE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_CREATE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_DESTROY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_DESTROY_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_DESTROY_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_DESTROY_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_ENTRIES_GET_BLOCK),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_ENTRIES_GET_BLOCK_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_ENTRIES_GET_BLOCK_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DATABASE_ENTRIES_GET_BLOCK_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_ADD),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_ADD_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_ADD_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_ADD_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_REMOVE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_REMOVE_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_REMOVE_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_REMOVE_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CONTROL_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_SET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_SET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_SET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_SET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_EGR_DB_MAP_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_DIAG_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_DIAG_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_DIAG_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_PACKET_DIAG_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_RESOURCE_DIAG_GET),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_RESOURCE_DIAG_GET_PRINT),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_RESOURCE_DIAG_GET_UNSAFE),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_RESOURCE_DIAG_GET_VERIFY),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_GET_PROCS_PTR),
  SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_GET_ERRS_PTR),
  /*
   * } Auto generated. Do not edit previous section.
   */
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_TYPES_VERIFY_FIRST_STAGE),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_ACTION_TYPES_VERIFY_FIRST_STAGE),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_ACTION_TYPE_TO_CE_COPY),
    SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DB_STAGE_GET),
    SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DB_CASCADED_CYCLE_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_TCAM_CALLBACK),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_ACTION_TYPE_TO_PMF_CONVERT),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_ACTION_TYPE_FROM_PMF_CONVERT),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_ACTION_TYPE_MAX_SIZE_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_DB_TYPES_VERIFY_FIRST_STAGE),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_INIT_UNSAFE),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_FEM_SET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_FEM_DB_FIND),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_TYPE_PREDEFINED_KEY_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_PREDEFINED_KEY_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_TYPE_TO_TCAM_TYPE_CONVERT),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_TYPE_TO_KEY_FLD_CONVERT),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_PREDEFINED_KEY_SIZE_AND_TYPE_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_VALIDITY_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_VAL_ENCODE),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_TYPE_PRESET1),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ACTION_LSB_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_QUAL_LSB_AND_LENGTH_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CE_KEY_LENGTH_MINIMAL_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_KEY_INPUT_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_ENTRY_NDX_DIRECT_TABLE_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(SOC_PPC_FP_QUAL_TYPE_AND_LOCAL_LSB_GET),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_INGRESS_QUAL_VERIFY),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_IPV4_HOST_EXTEND_ENABLE),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_CYCLE_FOR_DB_INFO_VERIFY),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_IRE_TRAFFIC_SEND_VERIFY),
   SOC_PROCEDURE_DESC_ELEMENT_DEF(ARAD_PP_FP_IRE_TRAFFIC_SEND_UNSAFE),



  /*
   * Last element. Do no touch.
   */
  SOC_PROCEDURE_DESC_ELEMENT_DEF_LAST
};

CONST STATIC
SOC_ERROR_DESC_ELEMENT
    Arad_pp_error_desc_element_fp[] =
{
  /*
   * Auto generated. Do not edit following section {
   */
  {
    ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'pfg_ndx' is out of range. \n\r "
    "The range is: 0 - 4.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'db_id_ndx' is out of range. \n\r "
    "The range is: 0 - 127.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_SUCCESS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_SUCCESS_OUT_OF_RANGE_ERR",
    "The parameter 'success' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_NOF_SUCCESS_FAILURES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'entry_id_ndx' is out of range. \n\r "
    "The range is: 0 - 4*1024-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_IS_FOUND_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_IS_FOUND_OUT_OF_RANGE_ERR",
    "The parameter 'is_found' is out of range. \n\r "
    "The range is: 0 - 4*1024-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_NOF_ENTRIES_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_NOF_ENTRIES_OUT_OF_RANGE_ERR",
    "The parameter 'nof_entries' is out of range. \n\r "
    "The range is: 0 - 127.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_FWD_TYPE_NDX_OUT_OF_RANGE_ERR,
    "SOC_PPC_FP_FWD_TYPE_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'fwd_type_ndx' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_FP_FWD_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'port_profile_ndx' is out of range. \n\r "
    "The range is: 0 - 3.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_ID_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_DB_ID_OUT_OF_RANGE_ERR",
    "The parameter 'db_id' is out of range. \n\r "
    "The range is: 0 - 63.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'type' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_FP_QUAL_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_HDR_FORMAT_BMP_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_HDR_FORMAT_BMP_OUT_OF_RANGE_ERR",
    "The parameter 'hdr_format_bmp' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_STAGE_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_STAGE_OUT_OF_RANGE_ERR",
    "The parameter 'stage' is out of range. \n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_VLAN_TAG_STRUCTURE_BMP_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_VLAN_TAG_STRUCTURE_BMP_OUT_OF_RANGE_ERR",
    "The parameter 'vlan_tag_structure_bmp' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_TYPE_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_DB_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'db_type' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_FP_DATABASE_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_SUPPORTED_PFGS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_SUPPORTED_PFGS_OUT_OF_RANGE_ERR",
    "The parameter 'supported_pfgs' is out of range. \n\r "
    "The range is: 1 (at least one PFG mnust be set) - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR,
    "SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR",
    "The parameter 'qual_types' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_FP_QUAL_TYPES-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_DATABASE_TYPE_OUT_OF_RANGE_ERR,
    "SOC_PPC_FP_DATABASE_TYPE_OUT_OF_RANGE_ERR",
    "The parameter 'db_type' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_FP_DATABASE_TYPES.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_ACTION_TYPES_OUT_OF_RANGE_ERR,
    "SOC_PPC_FP_ACTION_TYPES_OUT_OF_RANGE_ERR",
    "The parameter 'action_types' is out of range. \n\r "
    "The range is: 0 - SOC_PPC_NOF_FP_ACTION_TYPES_ARAD-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_STRENGTH_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_STRENGTH_OUT_OF_RANGE_ERR",
    "The parameter 'strength' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR",
    "The parameter 'val' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_PRIORITY_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_PRIORITY_OUT_OF_RANGE_ERR",
    "The parameter 'priority' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_FLD_LSB_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_FLD_LSB_OUT_OF_RANGE_ERR",
    "The parameter 'fld_lsb' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_CST_VAL_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_CST_VAL_OUT_OF_RANGE_ERR",
    "The parameter 'cst_val' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_NOF_BITS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_NOF_BITS_OUT_OF_RANGE_ERR",
    "The parameter 'nof_bits' is out of range. \n\r "
    "The range is: 0 - 17.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_NOF_FIELDS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_NOF_FIELDS_OUT_OF_RANGE_ERR",
    "The parameter 'nof_fields' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_BASE_VAL_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_BASE_VAL_OUT_OF_RANGE_ERR",
    "The parameter 'base_val' is out of range. \n\r "
    "The range is: 0 - 16*1024-1.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR",
    "The parameter 'val_ndx' is out of range. \n\r "
    "The range is: 0 - SOC_SAND_U32_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  /*
   * } Auto generated. Do not edit previous section.
   */
  {
    ARAD_PP_FP_DB_ID_ALREADY_EXIST_ERR,
    "ARAD_PP_FP_DB_ID_ALREADY_EXIST_ERR",
    "The database 'db_id_ndx' is already created. \n\r "
    "Delete it to create another database with the same index.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_ID_NOT_DIRECT_EXTRACTION_ERR,
    "ARAD_PP_FP_DB_ID_NOT_DIRECT_EXTRACTION_ERR",
    "The entry cannot be added to a database which is not \n\r "
    "of type direct extraction.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_ID_NOT_LOOKUP_ERR,
    "ARAD_PP_FP_DB_ID_NOT_LOOKUP_ERR",
    "The entry cannot be added to a database which is not \n\r "
    "of type lookup (Direct Table, Egress or TCAM).\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_ID_DIRECT_TABLE_ALREADY_EXIST_ERR,
    "ARAD_PP_FP_DB_ID_DIRECT_TABLE_ALREADY_EXIST_ERR",
    "The database cannot be inserted since a direct table database \n\r "
    "already exists or the VSQ pointer feature for Flow Control is enabled.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_ENTRY_QUAL_TYPE_NOT_IN_DB_ERR,
    "ARAD_PP_FP_ENTRY_QUAL_TYPE_NOT_IN_DB_ERR",
    "On of the entry qualifiers is not present \n\r "
    "in the entry database qualifier list.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_ENTRY_ACTION_TYPE_NOT_IN_DB_ERR,
    "ARAD_PP_FP_ENTRY_ACTION_TYPE_NOT_IN_DB_ERR",
    "One of the entry action types is not present \n\r "
    "in the entry database action type list.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_ENTRY_ALREADY_EXIST_ERR,
    "ARAD_PP_FP_ENTRY_ALREADY_EXIST_ERR",
    "The Database contains already an entry \n\r "
    "with the same index.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_TAG_ACTION_ALREADY_EXIST_ERR,
    "ARAD_PP_FP_TAG_ACTION_ALREADY_EXIST_ERR",
    "The Database contains a TAG action that is already \n\r "
    "present for a supported PFG.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_QUAL_TYPES_NOT_PREDEFINED_ACL_KEY_ERR,
    "SOC_PPC_FP_QUAL_TYPES_NOT_PREDEFINED_ACL_KEY_ERR",
    "The Database has a TCAM Database type but \n\r "
    "its qualifier types does not correspond to a predefined ACL key.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_DB_CREATION_FOR_IPV4_HOST_EXTEND_ERR,
    "ARAD_PP_FP_DB_CREATION_FOR_IPV4_HOST_EXTEND_ERR",
    "The Database creation for the IPv4 Host Extended \n\r "
    "configuration has failed during the initialization.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_EGR_DATABASE_NOT_ALREADY_ADDED_ERR,
    "ARAD_PP_FP_EGR_DATABASE_NOT_ALREADY_ADDED_ERR",
    "The mapping between (port profile, packet forwarding type) \n\r "
    "and the egress Database can be done only for egress DB alreay created.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_QUAL_TYPES_END_OF_LIST_ERR,
    "SOC_PPC_FP_QUAL_TYPES_END_OF_LIST_ERR",
    "The parameter 'qual_types' has reached end of list \n\r "
    "but a non SOC_PPC_NOF_FP_QUAL_TYPES is inserted after that.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_ACTION_TYPES_END_OF_LIST_ERR,
    "SOC_PPC_FP_ACTION_TYPES_END_OF_LIST_ERR",
    "The parameter 'action_types' has reached end of list \n\r "
    "but a non SOC_PPC_NOF_FP_ACTION_TYPES_ARAD is inserted after that.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_ACTION_LENGTHS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_ACTION_LENGTHS_OUT_OF_RANGE_ERR",
    "The sum of all the action lengths is higher than  \n\r "
    "the HW admissible length (32b for TCAM, 20b for Direct Table).\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR",
    "The sum of all the qualifiers lengths is higher than  \n\r "
    "the HW admissible length (288b for TCAM, 10b for Direct Table, 32b for DE).\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_EGRESS_QUAL_USED_FOR_INGRESS_DB_ERR,
    "ARAD_PP_FP_EGRESS_QUAL_USED_FOR_INGRESS_DB_ERR",
    "One of the qualifiers is dedicated to egress  \n\r "
    "databases (ERPP) and is used by error for an ingress DB.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_QUAL_NOT_EGRESS_ERR,
    "ARAD_PP_FP_QUAL_NOT_EGRESS_ERR",
    "The qualifiers do not correspond to a predefined \n\r "
    "egress Key although the Database type is Egress.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_ACTIONS_MIXED_WITH_TAG_ERR,
    "ARAD_PP_FP_ACTIONS_MIXED_WITH_TAG_ERR",
    "The actions of a database must be all of type TAG \n\r "
    "or no one must be of type TAG.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_PACKET_SIZE_OUT_OF_RANGE_ERR,
    "SOC_PPC_FP_PACKET_SIZE_OUT_OF_RANGE_ERR",
    "The size of the sent packet is out of range. \n\r "
    "The range is: 64 - 200.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    SOC_PPC_FP_NOF_DBS_PER_BANK_OUT_OF_RANGE_ERR,
    "SOC_PPC_FP_NOF_DBS_PER_BANK_OUT_OF_RANGE_ERR",
    "The number of databases which can be related to a TCAM bank\n\r "
    "The range is 0 - SOC_PPC_FP_MAX_NOF_DBS_PER_BANK.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_NOF_QUALS_PER_DB_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_NOF_QUALS_PER_DB_OUT_OF_RANGE_ERR",
    "The number of qualifiers which can be related to one database\n\r "
    "The range is 0 - SOC_PPC_FP_NOF_QUALS_PER_DB_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_NOF_ACTIONS_PER_DB_OUT_OF_RANGE_ERR,
    "ARAD_PP_FP_NOF_ACTIONS_PER_DB_OUT_OF_RANGE_ERR",
    "The number of qualifiers which can be related to one database\n\r "
    "The range is 0 - SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX.\n\r ",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  {
    ARAD_PP_FP_PORT_PROFILE_ALREADY_EXIST_ERR,
    "ARAD_PP_FP_PORT_PROFILE_ALREADY_EXIST_ERR",
    "The requested port already has a profile ID.\n\r",
    SOC_SAND_SVR_ERR,
    FALSE
  },
  
  /*
   * Last element. Do no touch.
   */
SOC_ERR_DESC_ELEMENT_DEF_LAST
};


/* this struct used for DIP SIP sharing. there are two sharing clusters: IPv4 and IPv6
   qualfiers sharing can be only inside the cluster */
ARAD_PP_FP_DIP_SIP_CLUSTER_INFO dip_sip_sharing_cluster[ARAD_PP_FP_NOF_DIP_SIP_CLUSTERS ] = 
  {{"IPv4",
    ARAD_PP_FP_DIP_SIP_CLUSTERS_IPV4,
    {PROG_FLP_IPV4UC, PROG_FLP_IPV4UC_RPF, PROG_FLP_IPV4COMPMC_WITH_RPF, PROG_FLP_IPV4_DC, -1},
    {{"DIP", 4, SOC_PPC_FP_QUAL_HDR_IPV4_DIP},{"SIP", 4, SOC_PPC_FP_QUAL_HDR_IPV4_SIP},{"NA", 0, BCM_FIELD_ENTRY_INVALID},{"NA", 0, BCM_FIELD_ENTRY_INVALID}},
    2},

   {"IPv6",
     ARAD_PP_FP_DIP_SIP_CLUSTERS_IPV6,
    {PROG_FLP_IPV6UC, PROG_FLP_IPV6UC_RPF, PROG_FLP_IPV6MC, PROG_FLP_IPV6UC_PUBLIC, PROG_FLP_IPV6UC_PUBLIC_RPF},
    {{"DIP", 8, SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH},{"DIP", 8, SOC_PPC_FP_QUAL_HDR_IPV6_DIP_LOW},{"SIP", 8, SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH},{"SIP", 8, SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW}},
    4}};


void
  ARAD_PP_FP_SHARED_QUAL_INFO_clear(ARAD_PP_FP_SHARED_QUAL_INFO* shard_qualifiers_info)
{
  shard_qualifiers_info->qual_type = 0;
  shard_qualifiers_info->size = 0;
}

/* according to program returns the relevant cluster */
void
  arad_pp_fp_dip_sip_sharing_cluster_get(int unit, int prog_id, int* pos){

  int i,j;
  uint8 prog_index;
  int nof_dynamic_programs = 0;
  int dynamic_program[3];

  dynamic_program[nof_dynamic_programs++] = PROG_FLP_IPV6UC_RPF;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  /* Add search for PROG_FLP_IPV6UC_PUBLIC and PROG_FLP_IPV6UC_PUBLIC_RPF
   * when the KBP is not used for IPv6 forwarding.
   * If KBP is used for IPv6 forwarding, these programs are not in use.
   * This will prevent unwanted error prints from arad_pp_flp_app_to_prog_index_get
   */
  if(!ARAD_KBP_ENABLE_IPV6_UC) {
    dynamic_program[nof_dynamic_programs++] = PROG_FLP_IPV6UC_PUBLIC;
  }
  if(!ARAD_KBP_ENABLE_IPV6_RPF) {
    dynamic_program[nof_dynamic_programs++] = PROG_FLP_IPV6UC_PUBLIC_RPF;
  }
#endif

  for(i = 0; i < nof_dynamic_programs; i++) {
    arad_pp_flp_app_to_prog_index_get((unit), dynamic_program[i], &prog_index);
    if(prog_index == prog_id) {
      prog_id = dynamic_program[i];
      break;
    }
  }
  (*pos) = -1;

  for (i = 0; i < ARAD_PP_FP_NOF_DIP_SIP_CLUSTERS; i++) {
    for (j = 0; j < ARAD_PP_FP_MAX_NOF_PROGRAMS_IN_DIP_SIP_CLUSTERS; j++) {
      if (dip_sip_sharing_cluster[i].prog_id[j] == prog_id) {
        (*pos) = i;
        break;
      }
    }
  }
}

/*  this function receive shared qualifiers array and arange them according to the order in the cluster */
void
  dip_sip_sharing_sharde_qualifiers_arrange(int unit, int prog_id, ARAD_PP_FP_SHARED_QUAL_INFO* shard_qualifiers_info)
{
  int i, pos = -1, j, nof_shared_qualifiers = 0;
  ARAD_PP_FP_SHARED_QUAL_INFO shard_qualifiers_info_lcl[MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM];

  sal_memcpy(shard_qualifiers_info_lcl, shard_qualifiers_info, sizeof(ARAD_PP_FP_SHARED_QUAL_INFO)*MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM);

  arad_pp_fp_dip_sip_sharing_cluster_get(unit, prog_id, &pos);

  for (i = 0; i < dip_sip_sharing_cluster[pos].nof_qualifiers; i++) {
    for (j = 0; j < MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM; j++) {

      if (dip_sip_sharing_cluster[pos].shared_quals[i].qual_type == shard_qualifiers_info_lcl[j].qual_type) {

        sal_memcpy(&shard_qualifiers_info[nof_shared_qualifiers], &shard_qualifiers_info_lcl[j], sizeof(ARAD_PP_FP_SHARED_QUAL_INFO));
        nof_shared_qualifiers++;
        break;
      }
    }

  }
}


/*  returns information about shard qualifier */
void
  dip_sip_sharing_qual_info_get(int unit, int prog_id, SOC_PPC_FP_QUAL_TYPE qual_type, int* is_exists, ARAD_PP_FP_SHARED_QUAL_INFO* shared_qual_info){

  int i, pos = -1;
  (*is_exists) = 0;

  arad_pp_fp_dip_sip_sharing_cluster_get(unit, prog_id, &pos);

  if (pos == -1) {
    return;
  }

  for (i = 0; i < dip_sip_sharing_cluster[pos].nof_qualifiers; i++) {

      if (dip_sip_sharing_cluster[pos].shared_quals[i].qual_type == qual_type) {
        (*is_exists) = 1;
        if (shared_qual_info) {
          sal_memcpy(shared_qual_info->name, dip_sip_sharing_cluster[pos].shared_quals[i].name, 20);
          shared_qual_info->qual_type = dip_sip_sharing_cluster[pos].shared_quals[i].qual_type;
          shared_qual_info->size = dip_sip_sharing_cluster[pos].shared_quals[i].size;
        }        
        break;
      }
  }

}


/* in order that db can share qualifier, all the programs in the DB has to be in the same cluster*/
uint32
  arad_pp_fp_dip_sip_sharing_is_sharing_enabled_for_db(int unit, SOC_PPC_FP_DATABASE_STAGE stage, uint32 db_id_ndx, uint32* is_enabled, int* in_cluster_id){

  int curr_cluster = -1;
  uint32 res;
  uint32 min_prog_indx, max_prog_indx, program_id, exist_progs;
  SOC_PPC_FP_DATABASE_INFO fp_database_info;
  int cluster_id = -1;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
  int is_mc_program_exists = 0;
  ARAD_INIT_ELK* elk = &SOC_DPP_CONFIG(unit)->arad->init.elk;
#endif

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  (*is_enabled) = 1;
  (*in_cluster_id) = -1;

  res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  min_prog_indx = 0;
  max_prog_indx = ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX;

  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(unit, stage, db_id_ndx, 0, &exist_progs);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
  
  for (program_id = min_prog_indx; program_id < max_prog_indx; ++program_id){
    if (SOC_SAND_GET_BIT(exist_progs, program_id) != 0x1){
        /* Program not used */
        continue;
    }
    cluster_id = -1;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    if ((program_id == PROG_FLP_IPV6MC) || (program_id == PROG_FLP_IPV4COMPMC_WITH_RPF)) {
      is_mc_program_exists = 1;
    }    
#endif
    arad_pp_fp_dip_sip_sharing_cluster_get(unit, program_id, &cluster_id);

    if (cluster_id == -1) {
      (*is_enabled) = 0;
      break;
    }

    if (curr_cluster != -1) {
      if (cluster_id != curr_cluster) {
        (*is_enabled) = 0;
      }
    }else{
      curr_cluster = cluster_id;
    }       
  }

  /*  we need to validate that for all the programs external FWD lookups is enables */

  *in_cluster_id = cluster_id;

  if ((*is_enabled)) {
    switch (cluster_id) {
  #if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
    case ARAD_PP_FP_DIP_SIP_CLUSTERS_IPV4:
      if (!ARAD_KBP_ENABLE_IPV4_UC) {
        (*is_enabled) = 0;
      }
      if (is_mc_program_exists) {
        if (!ARAD_KBP_ENABLE_IPV4_MC) {
          (*is_enabled) = 0;
        }
      }
      break;

    case ARAD_PP_FP_DIP_SIP_CLUSTERS_IPV6:
      if(elk->kbp_no_fwd_ipv6_dip_sip_sharing_disable == 1) {
        if (!ARAD_KBP_ENABLE_IPV6_UC) {
          (*is_enabled) = 0;
        }
        if (is_mc_program_exists) {
          if (!ARAD_KBP_ENABLE_IPV6_MC) {
            (*is_enabled) = 0;
          }
        }
      }
      break;
  #endif
    default:
      (*is_enabled) = 0;
    }
  }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_dip_sip_sharing_is_sharing_enabled_for_db()", 0, 0);
}



/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

/*
 * Callback for the TCAM management
 */

uint32
  arad_pp_fp_key_length_get_unsafe(
    SOC_SAND_IN    int              unit,
    SOC_SAND_IN     SOC_PPC_FP_DATABASE_STAGE    stage,
    SOC_SAND_IN    SOC_PPC_FP_QUAL_TYPE     qual_type,
    SOC_SAND_IN    uint8              with_padding,
    SOC_SAND_OUT   uint32              *length
  )
{
    uint32
        res,
        length_logical,
        length_padded_best_case,
        length_padded_worst_case,
      qual_info_found;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_KEY_LENGTH_GET_UNSAFE);

    SOC_SAND_CHECK_NULL_INPUT(length);

    /* Get qualifier length */
    res = arad_pp_fp_qual_length_get(
                unit,
                stage,
                qual_type,
                &qual_info_found,
                &length_padded_best_case,
                &length_padded_worst_case,
                &length_logical
              );
    SOC_SAND_CHECK_FUNC_RESULT(res, 18, exit);


    if (!qual_info_found)
    {
    LOG_ERROR(BSL_LS_SOC_FP,
              (BSL_META_U(unit,
                          "Unit: %d, Stage: %s, Qual Type: %s Fail in trying to get the length of a qualifier.\n\r"),
               unit, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), SOC_PPC_FP_QUAL_TYPE_to_string(qual_type)));
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_KEY_UNKNOWN_QUAL_ERR, 20, exit);
    }

    if (!with_padding)
    {
      *length = length_logical;
    }
    else
    {
      *length = length_padded_best_case;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_key_length_get_unsafe()", 0, 0);
}

uint32
    arad_pp_fp_db_stage_info_get(
          SOC_SAND_IN int unit,
          SOC_SAND_IN SOC_PPC_FP_DATABASE_INFO  *fp_database_info,
          SOC_SAND_OUT SOC_PPC_FP_DATABASE_STAGE  *stage
    )
{
  SOC_PPC_FP_DATABASE_STAGE
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DB_STAGE_GET);

  switch (fp_database_info->db_type) {
  case SOC_PPC_FP_DB_TYPE_EGRESS:
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_EGRESS;
      break;
  case SOC_PPC_FP_DB_TYPE_FLP:
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;
      break;
  case SOC_PPC_FP_DB_TYPE_SLB:
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB;
      break;
  case SOC_PPC_FP_DB_TYPE_VT:
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT;
      break;
  case SOC_PPC_FP_DB_TYPE_TT:
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT;
      break;
  default:
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;
      break;
  }

  *stage = stage_lcl;

  ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_db_stage_info_get()", fp_database_info->db_type, 0);
}


uint32
    arad_pp_fp_db_stage_get(
          SOC_SAND_IN int unit,
          SOC_SAND_IN uint32  db_id_ndx,
          SOC_SAND_OUT SOC_PPC_FP_DATABASE_STAGE  *stage
    )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_STAGE
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info; 


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DB_STAGE_GET);

  /* 
   * Take any stage since the DB info is set for all stages during the create API
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
          unit,
          stage_lcl,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_fp_db_stage_info_get(
            unit,
            &fp_database_info,
            stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_db_stage_get()", db_id_ndx, 0);
}

uint32
    arad_pp_fp_db_cascaded_cycle_get(
          SOC_SAND_IN int unit,
          SOC_SAND_IN uint32  db_id_ndx,
          SOC_SAND_OUT uint8  *is_cascaded,
          SOC_SAND_OUT uint8  *lookup_id /* 1st or 2nd cycle */
    )
{
  uint32
      qual_ndx,
      action_indx,
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_STAGE
      stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info; 


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DB_CASCADED_CYCLE_GET);

  /* 
   * Take any stage since the DB info is set for all stages during the create API
   */
  /* Get the correct stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage_lcl
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
          unit,
          stage_lcl,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  *is_cascaded = FALSE;
  *lookup_id = 0;
  /* Check if there is a key-change action -> cycle 0 only */
  for(action_indx = 0;  (action_indx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX) && (fp_database_info.action_types[action_indx] != SOC_PPC_FP_ACTION_TYPE_INVALID); ++action_indx) {
      if (fp_database_info.action_types[action_indx] == SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY) {
          *is_cascaded = TRUE;
          *lookup_id = 0;
      }
  }

  /* Check if Key-changed qualifier -> cycle 1 only */
  for (qual_ndx = 0; qual_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; qual_ndx++)
  {
    if (fp_database_info.qual_types[qual_ndx] == SOC_PPC_FP_QUAL_IRPP_KEY_CHANGED)
    {
        if (*lookup_id == 1) {
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "   Error in cascaded Database get: "
                                "For database %d, stage %s, no success in Cycle computation: forbidden both cascaded actions and qualifiers for cascaded group \n\r"),
                     db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage_lcl)));
            SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 78, exit); 
        }
        *is_cascaded = TRUE;
        *lookup_id = 1;
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_db_cascaded_cycle_get()", db_id_ndx, 0);
}

uint32
    arad_pp_fp_db_is_equal_place_get(
          SOC_SAND_IN int    unit,
          SOC_SAND_IN uint32    db_id_ndx,
          SOC_SAND_OUT uint8    *is_equal,
          SOC_SAND_OUT uint32   *place /* LSB or MSB */
    )
{
    uint32 
        res = SOC_SAND_OK;

    SOC_PPC_FP_DATABASE_STAGE 
        stage_lcl = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

    SOC_PPC_FP_DATABASE_INFO 
        fp_database_info; 

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DB_IS_EQUAL_PLACE_GET);

    /* Get the correct stage */
    res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage_lcl
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
          unit,
          stage_lcl,
          db_id_ndx,
          &fp_database_info
        );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

    *is_equal = FALSE;
    *place    = 0;

    if(fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_LSB)
    {
        *is_equal = TRUE;
        *place    = ARAD_PP_FP_KEY_CE_LOW;
    }
    else if(fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_MSB)
    {
        *is_equal = TRUE;
        *place    = ARAD_PP_FP_KEY_CE_HIGH;
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_db_is_equal_place_get()", db_id_ndx, 0);
}

uint32
    arad_pp_fp_tcam_callback(
      SOC_SAND_IN int unit,
      SOC_SAND_IN uint32  user_data
    )
{

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_TCAM_CALLBACK);

  /* callback data is db-id*/

  /* Nothing to do for the moment */
  ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_tcam_callback()", user_data, 0);
}


uint32
  arad_pp_fp_init_unsafe(
    SOC_SAND_IN  int      unit
  )
{
  uint32 res;
  SOC_SAND_OCC_BM_INIT_INFO
    occ_kaps_db_bm_init_info;
  SOC_SAND_OCC_BM_PTR
    kaps_db_bm;
  int i,j;
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
  ARAD_INIT_ELK* elk = &SOC_DPP_CONFIG(unit)->arad->init.elk;
#endif

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_INIT_UNSAFE);

  if(SOC_IS_ARADPLUS_AND_BELOW(unit)) { /* DIP SIP only supported for Jericho */
    for (i = 0; i < ARAD_PP_FP_NOF_DIP_SIP_CLUSTERS; i++) {
      for (j = 0; j < ARAD_PP_FP_MAX_NOF_PROGRAMS_IN_DIP_SIP_CLUSTERS; j++) {
        dip_sip_sharing_cluster[i].prog_id[j] = -1;
      }
    }
  }
#if defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030)
  else {
    if(ARAD_KBP_ENABLE_IPV6_UC || ARAD_KBP_ENABLE_IPV6_RPF || ARAD_KBP_ENABLE_IPV6_MC) {
        /* KBP is used for IPv6 forwarding
         * disable IPv6 DIP SIP sharing for IPv6 programs that don't use the KBP for forwarding
         */
        elk->kbp_no_fwd_ipv6_dip_sip_sharing_disable = 1;
    }
  }
#endif /* defined(BCM_88675_A0) && defined(INCLUDE_KBP) && !defined(BCM_88030) */

#ifdef ARAD_TCAM_EGQ_DUMMY_ACCESS_PROFILE_WA_ENABLE
  if(!soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "egress_pmf_lookups_always_valid_disable", 0))
  {
      uint32
          access_profile_id,
          prog_ndx,
          key_ndx;
      soc_reg_above_64_val_t
              reg_above_64;
      soc_reg_t
          egq_key_data_base_profile[2] = {EGQ_KEYA_DATA_BASE_PROFILEr, EGQ_KEYB_DATA_BASE_PROFILEr};

      access_profile_id = ARAD_TCAM_EGQ_DUMMY_ACCESS_PROFILE_NO_LOOKUP;
      /* Initialize this access profile not to lookup */
      res = arad_tcam_access_profile_destroy_unsafe(unit, access_profile_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      /* Allocate the last access profile in SW state */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.profiles.valid.set(unit, access_profile_id, TRUE); /* Never freed */
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);

      /* Initialize the Egress PMF Programs to have this access profile */
      for (key_ndx = 0; key_ndx < SOC_DPP_DEFS_GET(unit, nof_egress_pmf_keys); key_ndx++) {
          SOC_SAND_SOC_IF_ERROR_RETURN(res, 12, exit, soc_reg_above_64_get(unit, egq_key_data_base_profile[key_ndx], REG_PORT_ANY, 0, reg_above_64));
          for (prog_ndx = 0; prog_ndx < SOC_DPP_DEFS_GET(unit, nof_egress_pmf_programs); prog_ndx++) {
              SHR_BITCOPY_RANGE(reg_above_64, (ARAD_PMF_KEY_TCAM_DB_PROFILE_NOF_BITS * prog_ndx), &access_profile_id, 0, ARAD_PMF_KEY_TCAM_DB_PROFILE_NOF_BITS);
          }
          SOC_SAND_SOC_IF_ERROR_RETURN(res, 14, exit, soc_reg_above_64_set(unit, egq_key_data_base_profile[key_ndx], REG_PORT_ANY, 0, reg_above_64));
      }
  }
#endif /* ARAD_TCAM_EGQ_DUMMY_ACCESS_PROFILE_WA_ENABLE */


    soc_sand_SAND_OCC_BM_INIT_INFO_clear(&occ_kaps_db_bm_init_info);
    occ_kaps_db_bm_init_info.size     = 16;
    occ_kaps_db_bm_init_info.init_val = FALSE;

    res = soc_sand_occ_bm_create(
            unit,
            &occ_kaps_db_bm_init_info,
            &kaps_db_bm
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.kaps_db_used.set(unit,0, kaps_db_bm);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_init_unsafe()", 0, 0);
}



/*
 * Verify the validity of the entries
 */
STATIC
  uint32
    arad_pp_fp_entry_validity_get(
      SOC_SAND_IN  int                     unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO        *fp_database_info,
      SOC_SAND_IN  SOC_PPC_FP_QUAL_VAL             qual_vals[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX],
      SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE          action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX]
    )
{
  uint32
    res;
  uint32
    action_type_ndx,
    action_type_ndx2,
    qual_type_ndx,
    qual_type_ndx2;
  uint8
    is_type_found;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_VALIDITY_GET);

  for (qual_type_ndx = 0; qual_type_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++qual_type_ndx)
  {
    if ((qual_vals[qual_type_ndx].type != SOC_PPC_NOF_FP_QUAL_TYPES) && (qual_vals[qual_type_ndx].type != BCM_FIELD_ENTRY_INVALID))
    {
      is_type_found = FALSE;
      for (qual_type_ndx2 = 0; (qual_type_ndx2 < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX) && (is_type_found == FALSE); ++qual_type_ndx2)
      {
        if ( (qual_vals[qual_type_ndx].type == fp_database_info->qual_types[qual_type_ndx2]) || 
            ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "vt_tst2", 0)) && (qual_vals[qual_type_ndx].type == SOC_PPC_FP_QUAL_HDR_VLAN_TAG_ID) && (fp_database_info->qual_types[qual_type_ndx2] == SOC_PPC_FP_QUAL_INITIAL_VID) ) )
        {
          is_type_found = TRUE;
        }
      }
      if (is_type_found == FALSE)
      {
        LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Failed in entry validity get. Can\'t find adequate type for all the qualifiers.\n\r"))); 
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ENTRY_QUAL_TYPE_NOT_IN_DB_ERR, 50, exit);
      }
    }
  }

  /*
   *	Verify the action types and qualifiers types exist in the Database
   */
  for (action_type_ndx = 0; action_type_ndx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++action_type_ndx)
  {
    if (action_types[action_type_ndx] != SOC_PPC_FP_ACTION_TYPE_INVALID)
    {
      is_type_found = FALSE;
      for (action_type_ndx2 = 0; (action_type_ndx2 < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX) && (is_type_found == FALSE); ++action_type_ndx2)
      {
        if (action_types[action_type_ndx] == fp_database_info->action_types[action_type_ndx2])
        {
          is_type_found = TRUE;
        }
      }
      if (is_type_found == FALSE)
      {
        LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Failed in entry validity get. Can\'t find adequate type for all the actions.\n\r"))); 
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ENTRY_ACTION_TYPE_NOT_IN_DB_ERR, 70, exit);
      }
    }
  }

  ARAD_PP_DO_NOTHING_AND_EXIT;
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_validity_get()", 0, 0);
}

/*
 * Indicate if the PFG is of type TM 
 * Mandatory to have only Forwarding = TM 
 */
STATIC
  uint32
    arad_pp_fp_pfg_is_tm_get(
      SOC_SAND_IN  int              unit,
      SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO *info,
      SOC_SAND_OUT uint8               *is_pfg_tm,       
      SOC_SAND_OUT uint8               *is_pfg_default_tm       
    )
{
  uint32
      nof_qual_types,
      qual_ndx,
    res;
  uint8
    is_pfg_default_tm_lcl,
    is_pfg_tm_local;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Special TM PFG only at ingress due to the default static PMF-Programs */
  if (info->stage != SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
      *is_pfg_tm = FALSE;
      ARAD_DO_NOTHING_AND_EXIT;
  }

  is_pfg_tm_local = FALSE;
  is_pfg_default_tm_lcl = TRUE;
  nof_qual_types = 0;
  for (qual_ndx = 0; qual_ndx < SOC_PPC_FP_NOF_QUALS_PER_PFG_MAX; qual_ndx++) {
      if ((info->qual_vals[qual_ndx].type == SOC_PPC_FP_QUAL_IRPP_INVALID)
          || (info->qual_vals[qual_ndx].type == SOC_PPC_FP_QUAL_FORWARDING_OFFSET_EXTENSION) /* bypass for Forwarding Type where only the 1st qualifier is taken */
          || (info->qual_vals[qual_ndx].type == SOC_PPC_NOF_FP_QUAL_TYPES)
          || (info->qual_vals[qual_ndx].type == BCM_FIELD_ENTRY_INVALID)) {
          continue;
      }

      nof_qual_types ++;
      /* Verify if forwarding is in the list */
      if (info->qual_vals[qual_ndx].type == SOC_PPC_FP_QUAL_IRPP_FWD_TYPE) {
          /**/
          if ((info->qual_vals[qual_ndx].is_valid.arr[0] == 0xF) 
              && (info->qual_vals[qual_ndx].val.arr[0] == SOC_TMC_PKT_FRWRD_TYPE_TM)) {
              is_pfg_tm_local = TRUE;
          }
      }

      /* Skip the port-profile qualifier */
      if (info->qual_vals[qual_ndx].type == SOC_PPC_FP_QUAL_IRPP_IN_PORT_KEY_GEN_VAR_PS) {
          /* Do not take this qualifier in account */
          nof_qual_types --;
          is_pfg_default_tm_lcl = FALSE;
      }
  }

  /* Return error if there was not a single qualifier (Forwarding = TM) */
  if (is_pfg_tm_local && (nof_qual_types > 1)) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 15, exit);
  }

  *is_pfg_tm = is_pfg_tm_local;
  *is_pfg_default_tm = is_pfg_default_tm_lcl;

  ARAD_PP_DO_NOTHING_AND_EXIT;
    
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_pfg_is_tm_get()", 0, 0);
}


/*********************************************************************
*     Set a Packet Format Group (PFG). The packet format group
 *     defines the supported Packet formats. The user must
 *     indicate for each Database which Packet format(s) are
 *     associated with this Database. E.g.: A Packet Format
 *     Group including only IPv6 packets can be defined to use
 *     Databases with IPv6 Destination-IP qualifiers.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_packet_format_group_set_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    pfg_ndx,
    SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO            *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success
  )
{
    ARAD_PMF_PSL
      psl,
      *psl_ptr;
    uint8
        is_pfg_tm,
        is_pfg_default_tm,
        is_for_delete,
        psl_alloc_success;
    SOC_PPC_PMF_PFG_INFO
        pfg_info_current;
    uint32
        pfg_tm_bmp[1],
        res,
        flags = 0,
        line_indx,
        min_indx, max_indx, 
        flag_advanced_mode = 0;
    ARAD_PMF_PSL_LEVEL_INFO
        *curr_level = NULL;
    SOC_PPC_FP_DATABASE_STAGE
        stage; 

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_PACKET_FORMAT_GROUP_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  *success = SOC_SAND_SUCCESS;

  ARAD_ALLOC(curr_level, ARAD_PMF_PSL_LEVEL_INFO, 1, "arad_pp_fp_packet_format_group_set_unsafe.curr_level");

  res = arad_pp_fp_pfg_is_tm_get(unit, info, &is_pfg_tm, &is_pfg_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 152, exit);

  /*only Eth-programs in advanced preselcetion management mode*/
  if (soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE) && !is_pfg_tm ) {
      flag_advanced_mode = ARAD_PMF_SEL_ALLOC_ADVANCED_MODE;
      flags |= flag_advanced_mode;

      if (info->is_for_hw_commit) {

          /*get Eth presel section borders*/
          SOC_SAND_CHECK_FUNC_RESULT(arad_pmf_prog_select_line_borders_get(unit,info->stage, ARAD_PMF_PSL_TYPE_ETH, &min_indx,&max_indx), 25, exit);

          /*map presel id to line in hw*/
          line_indx = max_indx - pfg_ndx;

          if (info->is_array_qualifier) {

              /* get first level info */
              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.levels_info.get(unit, info->stage, ARAD_PMF_PSL_TYPE_ETH, 1, curr_level);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 27, exit);
              res = arad_pmf_sel_line_hw_write(
                        unit,
                        info->stage,
                        line_indx,
                        &curr_level->lines[pfg_ndx],
                        curr_level->lines[pfg_ndx].prog_id,
                        TRUE
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
          }
          else /*presel line for deletion*/
          {
                /* get first level info */
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.levels_info.get(unit, info->stage, ARAD_PMF_PSL_TYPE_ETH, 1, curr_level);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 27, exit);
                curr_level->lines[pfg_ndx].flags &= (~ARAD_PP_FP_PMF_SEL_LINE_VALID);
                res = arad_pmf_sel_line_valid_set(unit, info->stage, line_indx, TRUE /* is_valid_field */, FALSE /*valid value */);
                SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
                psl_alloc_success = TRUE;
                goto exit_tm;
          }
          ARAD_DO_NOTHING_AND_EXIT;
      }
  }

  /* 
   * Verify if the PFG should be removed (iff is_array_qualifier = 0)
   */
  is_for_delete = FALSE;
  if (!info->is_array_qualifier) {

      /* 
       * Special case for FLP: save it to SW DB, but always success: 
       * - in case of create / destroy, no configuration done in HW 
       * - the SW DB is used in database-create to retrieve the PFG info 
       */
      if (info->stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
          psl_alloc_success = TRUE;
          goto exit_tm;
      }
      /* Remove this PFG */
      is_for_delete = TRUE;
      pfg_info_current.stage = info->stage;
      /* Find if it exists */
      res = arad_pp_fp_packet_format_group_get_unsafe(
                unit,
                pfg_ndx,
                &pfg_info_current
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
      if (!pfg_info_current.is_array_qualifier) {
          /* The PFG is not defined, nothing to do */
          ARAD_DO_NOTHING_AND_EXIT;
      }
      stage = pfg_info_current.stage;

      /* 
       * Special case: see if it is a TM PFG
       */
      res = arad_pp_fp_pfg_is_tm_get(unit, &pfg_info_current, &is_pfg_tm, &is_pfg_default_tm);
      SOC_SAND_CHECK_FUNC_RESULT(res, 132, exit);
      if (is_pfg_tm) {
          /* Clear it in the SW DB */
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_tm_bmp.get(unit, stage, pfg_ndx/(sizeof(uint32)*8), pfg_tm_bmp);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 134, exit);
          SHR_BITCLR(pfg_tm_bmp, pfg_ndx % (sizeof(uint32)*8));
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_tm_bmp.set(unit, stage, pfg_ndx/(sizeof(uint32)*8), *pfg_tm_bmp);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 136, exit);

          /* Check if default TM or with per-port logic */
          if (is_pfg_default_tm)
          {
            /* Bypass the Ethernet PFG logic */
            psl_alloc_success = TRUE;
            goto exit_tm;
          }
          else {
            flags |= ARAD_PMF_SEL_ALLOC_TM;
          }
      }

      psl_ptr = NULL;
  }
  else { /* Addition */
      /* 
       * Translate the PFG info to the table
       */
      stage = info->stage;

      /* 
       * Special case for FLP: save it to SW DB, but always success: 
       * - in case of create / destroy, no configuration done in HW 
       * - the SW DB is used in database-create to retrieve the PFG info 
       */
      if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
          psl_alloc_success = TRUE;
          goto exit_tm;
      }

      /* 
       * Special case: see if it is a TM PFG
       */
      res = arad_pp_fp_pfg_is_tm_get(unit, info, &is_pfg_tm, &is_pfg_default_tm);
      SOC_SAND_CHECK_FUNC_RESULT(res, 152, exit);
      if (is_pfg_tm) {
          /* Save it in the SW DB */
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_tm_bmp.get(unit, stage, pfg_ndx/(sizeof(uint32)*8), pfg_tm_bmp);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 154, exit);
          SHR_BITSET(pfg_tm_bmp, pfg_ndx % (sizeof(uint32)*8));
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_tm_bmp.set(unit, stage, pfg_ndx/(sizeof(uint32)*8), *pfg_tm_bmp);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 156, exit);

          /* Check if default TM or with per-port logic */
          if (is_pfg_default_tm)
          {
            /* Bypass the Ethernet PFG logic */
            psl_alloc_success = TRUE;
            goto exit_tm;
          }
          else {
            flags |= ARAD_PMF_SEL_ALLOC_TM;
          }
      }

      ARAD_PMF_PSL_clear(unit, stage, &psl, 1 + is_pfg_tm, TRUE);
      res = arad_pmf_psl_map(
                unit,
                info,
                is_pfg_tm,
                &psl
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 18, exit);
      psl_ptr = &psl;
  }

  /* allocate PSL for this Ethernet-based program - just a try */
  flags |= ARAD_PMF_SEL_ALLOC_CHECK_ONLY | ( (info->is_staggered) ? ARAD_PMF_SEL_ALLOC_SECOND_PASS : 0);;
  if (is_for_delete) {
      /* Remove in the same way than adding PSLs */
      flags |= ARAD_PMF_SEL_ALLOC_FOR_DELETE;
  }
  /* psl_ptr can be null only if is_for_delete is set to TRUE before. In this case the flag ARAD_PMF_SEL_ALLOC_FOR_DELETE will be set, 
   * This is also verified in arad_pmf_add_group_at_level
   */
  /* coverity[var_deref_model : FALSE] */
  res = arad_pmf_psl_add(
          unit,
          stage,
          pfg_ndx,
          flags,
          psl_ptr,
          &psl_alloc_success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  /*
   * The SW DB indicates the current PFG
   */
  if (psl_alloc_success) {
      /* Set for this time also the SW DB for good */
      flags = flag_advanced_mode | ( (info->is_staggered) ? ARAD_PMF_SEL_ALLOC_SECOND_PASS : 0);
      if (is_for_delete) {
          flags |= ARAD_PMF_SEL_ALLOC_FOR_DELETE;
      }
      if (is_pfg_tm) {
          flags |= ARAD_PMF_SEL_ALLOC_TM;
      }

      /* psl_ptr can be null only if is_for_delete is set to TRUE before. In this case the flag ARAD_PMF_SEL_ALLOC_FOR_DELETE will be set, 
       * This is also verified in arad_pmf_add_group_at_level
       */
      /* coverity[var_deref_model : FALSE] */
      res = arad_pmf_psl_add(
              unit,
              stage,
              pfg_ndx,
              flags, /* flags - not check only */
              psl_ptr,
              &psl_alloc_success
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

      if (!psl_alloc_success) {
          /* inconsistent with above */
        LOG_ERROR(BSL_LS_SOC_FP,
                  (BSL_META_U(unit,
                              "Unit %d Pfg index %d - The insertion with check only succeeded, however, the real insertion failed.\n\r"),
                   unit, pfg_ndx));
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_ALREADY_EXIST_ERR, 19, exit);
      }

      /*Don't commit to HW in advanced mode*/
      if (! (flags & ARAD_PMF_SEL_ALLOC_ADVANCED_MODE)) {
        /*
        * map new lines to programs 
        * for each affect DB, map db to new joined progs  
        */
        res = arad_pmf_psl_hw_commit(unit, stage, flags);
        SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
      }

  }

exit_tm:
  if (psl_alloc_success) {
      /* Save in the SW DB: */
      if (soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) 
      {
          stage = info->stage;
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.pfg_info.set(
              unit,
              stage,
              pfg_ndx,
              info
          );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);
      }
      else
      {
          /* Save in the SW DB: for the get function for ex. for all stages */
          for (stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage ++) {
              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.pfg_info.set(
                  unit,
                  stage,
                  pfg_ndx,
                  info
              );
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);
          }
      }

  }

  *success = psl_alloc_success? SOC_SAND_SUCCESS: SOC_SAND_FAILURE_OUT_OF_RESOURCES;

exit:
  ARAD_FREE(curr_level);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_format_group_set_unsafe()", pfg_ndx, 0);
}


uint32
  arad_pp_fp_packet_format_group_set_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    pfg_ndx,
    SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO            *info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_PMF_PFG_INFO
      pfg_info_current;
  uint32
      db_bmp_id,
      pfg_db_pmb[ARAD_PMF_LOW_LEVEL_NOF_DBS_BMP];
  SOC_PPC_FP_DATABASE_STAGE
      stage = info->stage; 

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_PACKET_FORMAT_GROUP_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(pfg_ndx, ARAD_PP_FP_PFG_NDX_MAX, ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 10, exit);

  if (soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE) ==0 ) 
  {
      res = SOC_PPC_PMF_PFG_INFO_verify(unit, info);
      SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);

      pfg_info_current.stage = stage;
      /* Verify the PFG does not already exist -- needed in advanced mode? */
      res = arad_pp_fp_packet_format_group_get_unsafe(
                unit,
                pfg_ndx,
                &pfg_info_current
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
      if (pfg_info_current.is_array_qualifier && info->is_array_qualifier) {
        LOG_ERROR(BSL_LS_SOC_FP,
                  (BSL_META_U(unit,
                              "Unit %d pfg_ndx %d: Failed to set packet format group.\n\r"),unit, pfg_ndx));
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_ALREADY_EXIST_ERR, 30, exit);
      }
  

      /* In case of destroy, verify that no existing Database is attached to this PFG */
      if (!info->is_array_qualifier) {

          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_db_pmb.bit_range_read(unit, stage, pfg_ndx, 0, 0, ARAD_PMF_NOF_DBS, pfg_db_pmb);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

          for (db_bmp_id = 0; db_bmp_id < ARAD_PMF_LOW_LEVEL_NOF_DBS_BMP; db_bmp_id++) {
              if (pfg_db_pmb[db_bmp_id]) {
                LOG_ERROR(BSL_LS_SOC_FP,
                          (BSL_META_U(unit,
                                      "   Error in PFG %d destroy: Not all the Databases are deestroyed, BMP[%d] = 0x%x\n\r"), 
                           pfg_ndx, db_bmp_id, pfg_db_pmb[db_bmp_id]));
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_ALREADY_EXIST_ERR, 22, exit);
              }
          }
      }
  }



exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_format_group_set_verify()", pfg_ndx, 0);
}

uint32
  arad_pp_fp_packet_format_group_get_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    pfg_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_PACKET_FORMAT_GROUP_GET_VERIFY);

  /* Don't know the stage here, cannot decide the exact limit */
  SOC_SAND_ERR_IF_ABOVE_MAX(pfg_ndx, SOC_PPC_FP_NOF_PFGS_ARAD, ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_format_group_get_verify()", pfg_ndx, 0);
}


/*********************************************************************
*     Set a Packet Format Group (PFG). The packet format group
 *     defines the supported Packet formats. The user must
 *     indicate for each Database which Packet format(s) are
 *     associated with this Database. E.g.: A Packet Format
 *     Group including only IPv6 packets can be defined to use
 *     Databases with IPv6 Destination-IP qualifiers.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_packet_format_group_get_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  uint32                    pfg_ndx,
    SOC_SAND_INOUT SOC_PPC_PMF_PFG_INFO            *info
  )
{
    soc_error_t
        rv;
    SOC_PPC_FP_DATABASE_STAGE
        stage;


  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_PACKET_FORMAT_GROUP_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);

  stage = (soc_property_get(unit, spn_FIELD_PRESEL_MGMT_ADVANCED_MODE, FALSE)) ?  info->stage : SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;

  SOC_PPC_PMF_PFG_INFO_clear(info);

      
  /*
   * The SW DB indicates the current PFG
   */
  rv = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.pfg_info.get(
    unit,
    stage,
    pfg_ndx,
    info
  );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(rv, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_format_group_get_unsafe()", pfg_ndx, 0);
}


uint32
  arad_pp_fp_ce_key_length_minimal_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO       *info,
    SOC_SAND_OUT uint32                     *length_min
  )
{
  uint32
    total_length,
    res;
  uint32
    qual_length,
    qual_ndx;
  SOC_PPC_FP_DATABASE_STAGE
      stage;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_CE_KEY_LENGTH_MINIMAL_GET);

  SOC_SAND_CHECK_NULL_INPUT(info);

  total_length = 0;

  /* Get the local stage */
  res = arad_pp_fp_db_stage_info_get(
            unit,
            info,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  if (total_length == 0)
  {
    for (qual_ndx = 0; qual_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; qual_ndx++)
    {
      if ((info->qual_types[qual_ndx] == BCM_FIELD_ENTRY_INVALID) || ((info->qual_types[qual_ndx] == SOC_PPC_NOF_FP_QUAL_TYPES)))
      {
        continue;
      }

      res = arad_pp_fp_key_length_get_unsafe(
              unit,
              stage,            
              info->qual_types[qual_ndx],
              TRUE, /* with padding */
              &qual_length
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      total_length += qual_length;

      if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
        if ((info->qual_types[qual_ndx] == SOC_PPC_FP_QUAL_HDR_IPV4_SIP) || 
            (info->qual_types[qual_ndx] == SOC_PPC_FP_QUAL_HDR_IPV4_DIP) ||
            (info->qual_types[qual_ndx] == SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH) ||
            (info->qual_types[qual_ndx] == SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH) ||
            (info->qual_types[qual_ndx] == SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH) ||
            (info->qual_types[qual_ndx] == SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH))
        {          
		  if(SOC_IS_JERICHO(unit)){
            total_length -= qual_length;         
		  }
        }
      }
    }
  }

  *length_min = total_length;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_ce_key_length_minimal_get()", 0, 0);
}

/* Verify the qualifier exists in the current stage */
STATIC 
 uint32
  arad_pp_fp_qual_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO       *info
  )
{
  uint32
      found,
      length_padded_best_case,
      length_padded_worst_case,
      length_logical,
    res;
  uint32
    qual_ndx;
  SOC_PPC_FP_DATABASE_STAGE
      stage;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_INGRESS_QUAL_VERIFY);

  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Get the correct stage */
  res = arad_pp_fp_db_stage_info_get(
            unit,
            info,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  for (qual_ndx = 0; qual_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; qual_ndx++)
  {
      if ((info->qual_types[qual_ndx] == BCM_FIELD_ENTRY_INVALID) || ((info->qual_types[qual_ndx] == SOC_PPC_NOF_FP_QUAL_TYPES)))
      {
        continue;
      }

      /* Get qualifier length */
      res = arad_pp_fp_qual_length_get(
                unit,
                stage,
                info->qual_types[qual_ndx],
                &found,
                &length_padded_best_case,
                &length_padded_worst_case,
                &length_logical
             );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (!found)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 30 + qual_ndx, exit);
    }
  }

  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_verify()", 0, 0);
}

uint32 
  arad_fp_sw_db_info_commit(
      SOC_SAND_IN  int                  unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN  uint32                   db_id_ndx,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO *db_info,
      SOC_SAND_IN  uint8                    is_default_db, /* Relevant for all lines */
      SOC_SAND_IN  SOC_SAND_SUCCESS_FAILURE success
  )
{
    ARAD_PMF_DB_INFO
       pmf_db_info;
    uint32
        pfg_ndx,
        res;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    SOC_SAND_OCC_BM_PTR
        kaps_db_bm;
    uint32 
        kaps_db_id;
    uint8 found;
#endif 
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Add to the PFG Database information */
    if (is_default_db
        && (db_info->db_type != SOC_PPC_FP_DB_TYPE_FLP)
        && (db_info->db_type != SOC_PPC_FP_DB_TYPE_SLB)
        && (db_info->db_type != SOC_PPC_FP_DB_TYPE_VT)
        && (db_info->db_type != SOC_PPC_FP_DB_TYPE_TT)
        && (db_info->db_type != SOC_PPC_FP_DB_TYPE_DIRECT_TABLE)
        && (db_info->db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION)) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.default_db_pmb.bit_set(unit, stage, db_id_ndx);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
    }
    else {
        for (pfg_ndx = 0; pfg_ndx < SOC_PPC_FP_NOF_PFGS_ARAD; ++pfg_ndx) {
            if (SHR_BITGET(db_info->supported_pfgs_arad, pfg_ndx)) {
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_db_pmb.bit_set(unit, stage, pfg_ndx, db_id_ndx);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
            }
        }
    }

    /* new DB info */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
            unit,
            stage,
            db_id_ndx,
            &pmf_db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);

    pmf_db_info.prio = db_info->strength;
    pmf_db_info.valid = 1;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)

    if (db_info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE && db_info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) 
    {
        soc_dpp_config_jer_pp_t *jer_pp_config = &(SOC_DPP_JER_CONFIG(unit)->pp);

        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.kaps_db_used.get(unit,0, &kaps_db_bm);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);

        /* Index is used, meaning: Some other channelize interface spot reserved place, try to find a new place and replace */
        res = soc_sand_occ_bm_get_next(
              unit,
              kaps_db_bm,
              &kaps_db_id,
              &found
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

        if ((!found) || (kaps_db_id >= (jer_pp_config->kaps_large_db_size / JER_KAPS_DMA_DB_NOF_ENTRIES )) )
        {
            /* No reserved kaps_db_ids */
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 0, exit);
        }

        res = soc_sand_occ_bm_alloc_next(
              unit,
              kaps_db_bm,
              &kaps_db_id,
              &found
           );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

        pmf_db_info.kaps_db_id = kaps_db_id; /*need to check how to allocate*/
    }
#endif

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.set(
            unit,
            stage,
            db_id_ndx,
            &pmf_db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);


exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fp_sw_db_info_commit()",0,0);
}

STATIC
uint32 
  arad_fp_sw_db_info_clear(
      SOC_SAND_IN  int                  unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_IN  uint32                   db_id_ndx,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO *db_info
  )
{
    ARAD_PMF_DB_INFO
       pmf_db_info;
    SOC_PPC_FP_DATABASE_STAGE   
      stage_lcl;
    ARAD_FP_ENTRY
      fp_entry;
    SOC_PPC_FP_DATABASE_INFO
      fp_database_info; 
    uint8
        is_default_db;
    uint32
        pfg_ndx,
        res;


    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Add to the PFG Database information */
    is_default_db = (db_info->supported_pfgs_arad[0] || db_info->supported_pfgs_arad[1])? FALSE : TRUE;
    if (is_default_db) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.default_db_pmb.bit_clear(unit, stage, db_id_ndx);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
    }
    else {
        for (pfg_ndx = 0; pfg_ndx < SOC_PPC_FP_NOF_PFGS_ARAD; ++pfg_ndx) {
            if (SHR_BITGET(db_info->supported_pfgs_arad, pfg_ndx)) {
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_db_pmb.bit_clear(unit, stage, pfg_ndx, db_id_ndx);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);
            }
        }
    }


    /* Clear DB info */
    ARAD_CLEAR(&pmf_db_info, ARAD_PMF_DB_INFO, 1);
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.set(
            unit,
            stage,
            db_id_ndx,
            &pmf_db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);

    for (stage_lcl = 0; stage_lcl < SOC_PPC_NOF_FP_DATABASE_STAGES; stage_lcl ++) {
      SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.set(
              unit,
              stage_lcl,
              db_id_ndx,
              &fp_database_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

      fp_entry.nof_db_entries = 0;
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.set(
              unit,
              stage,
              db_id_ndx,
              &fp_entry
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);

      res = arad_sw_db_fp_db_entry_bitmap_clear(unit, stage_lcl);
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_fp_sw_db_info_clear()",0,0);
}

/* Verify the qualifier is UDF with base-header = post-forwarding */
STATIC uint32
  arad_pp_fp_qual_tm_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE     stage,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO   *info,
    SOC_SAND_OUT SOC_PPC_FP_ACTION_TYPE     *action_type,
    SOC_SAND_OUT uint32                     *offset_lsb
  )
{
  uint32
    res,
      fes_tm_lsb,
      action_size,
      action_lsb_egress,
      nof_actions,
    qual_ndx;
  uint8
      is_qual_found;
  ARAD_PMF_CE_QUAL_INFO
    qual_info;
  int32
      action_indx = 0;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Get the correct stage */
  is_qual_found = FALSE;

  for (qual_ndx = 0; qual_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; qual_ndx++)
  {
      if ((info->qual_types[qual_ndx] == BCM_FIELD_ENTRY_INVALID) || ((info->qual_types[qual_ndx] == SOC_PPC_NOF_FP_QUAL_TYPES)))
      {
        continue;
      }

      if (is_qual_found == TRUE) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 30 + qual_ndx, exit);
      }

      is_qual_found = TRUE;

      /* Verify it is User-Defined qualifier */
      if (!(SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(info->qual_types[qual_ndx]))) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 60, exit);
      }

      /* Verify the base-header */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.udf.get(
              unit,
              stage,
              (info->qual_types[qual_ndx] - SOC_PPC_FP_QUAL_HDR_USER_DEF_0),
              &qual_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 62, exit);
      if (qual_info.header_qual_info.header_ndx_0 != SOC_TMC_PMF_CE_SUB_HEADER_FWD_POST) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 70, exit);
      }

      /* Compute the offset-lsb and verify it is under 160b */
      *offset_lsb = qual_info.header_qual_info.lsb;
      if (*offset_lsb > ARAD_PP_FP_KEY_LENGTH_TM_IN_BITS) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 80, exit);
      }
  }

  if (is_qual_found == FALSE) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 100 + qual_ndx, exit);
  }

  nof_actions = 0;
  for(action_indx = 0;  (action_indx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX) && (info->action_types[action_indx] != SOC_PPC_FP_ACTION_TYPE_INVALID); ++action_indx) {
      nof_actions ++;
      *action_type = info->action_types[action_indx];
  }
  if (nof_actions != 1) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 200, exit);
  }


  /* 
   * Verify that the offset-lsb + action length is in the same 
   * 32b input: [31-0], [47-16], [63-32], [79-48], [111-80], [127-96], 
   * [143-112], [159-128]
   */
  res = arad_pmf_db_fes_action_size_get_unsafe(
          unit,
          (*action_type),
          stage,
          &action_size,
          &action_lsb_egress
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 210, exit);
  fes_tm_lsb = ARAD_PP_FP_KEY_LENGTH_TM_IN_BITS - (*offset_lsb) - 1;
  if ((ARAD_PP_FP_FEM_ACTION_LSB_TO_KEY_LSB(fes_tm_lsb) + 32) < (fes_tm_lsb + action_size)) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 220, exit);
  }

  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_qual_tm_verify()", 0, 0);
}

uint32
  arad_pp_fp_database_is_tm_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO               *info,
    SOC_SAND_OUT uint8                                  *is_for_tm,
    SOC_SAND_OUT uint8                                  *is_default_tm
  )
{
  uint32
      pfg_tm_bmp[1],
      pfg_ndx,
      pfg_long_ndx,
      pfg_bit_ndx,
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_STAGE
      stage;
  uint8
      is_default_tm_lcl,
      is_pfg_tm,
      is_pfg_default_tm,
      is_for_tm_lcl;
  SOC_PPC_PMF_PFG_INFO
      pfg_info_current;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Get the correct stage */
  res = arad_pp_fp_db_stage_info_get(
            unit,
            info,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  /* TM PFGs only at ingress PMF */
  if (stage != SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
      *is_for_tm = FALSE;
      ARAD_DO_NOTHING_AND_EXIT;
  }

  /* 
   * Verify the PFGs: 
   * - if all of them are TMs
   */
  is_for_tm_lcl = FALSE;
  is_default_tm_lcl = FALSE;
  for (pfg_long_ndx = 0; pfg_long_ndx < SOC_PPC_FP_NOF_PFGS_IN_LONGS_ARAD; pfg_long_ndx++) {
      for (pfg_bit_ndx = 0; pfg_bit_ndx < 32; pfg_bit_ndx++) {
          if (SHR_BITGET(&(info->supported_pfgs_arad[pfg_long_ndx]), pfg_bit_ndx)) {
              pfg_ndx = pfg_bit_ndx + (pfg_long_ndx * 32);
              if (pfg_ndx > ARAD_PP_FP_PFG_NDX_MAX) {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 38, exit);
              }

              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_tm_bmp.get(unit, stage, pfg_bit_ndx/(sizeof(uint32)*8), pfg_tm_bmp);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 154, exit);
              /* Check if it is TM */
              if (SHR_BITGET(pfg_tm_bmp, pfg_bit_ndx % (sizeof(uint32)*8))) {
                  is_for_tm_lcl = TRUE;
              }
              else if (is_for_tm_lcl) {
                  /* Error: 2 PFGs, once TM and the other not */
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 40, exit);
              }

              pfg_info_current.stage = stage;

              /* Check if it is default TM: enough one PFG default TM */
              res = arad_pp_fp_packet_format_group_get_unsafe(
                        unit,
                        pfg_ndx,
                        &pfg_info_current
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 53, exit);
              res = arad_pp_fp_pfg_is_tm_get(unit, &pfg_info_current, &is_pfg_tm, &is_pfg_default_tm);
              SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
              if (is_pfg_default_tm) {
                  is_default_tm_lcl = TRUE;
              }
          }
      }
  }

  *is_for_tm = is_for_tm_lcl;
  *is_default_tm = is_default_tm_lcl;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_is_tm_get()", 0, 0);
}

STATIC uint32
  arad_pp_fp_database_is_large_direct_extraction_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO               *info,
    SOC_SAND_OUT uint8                                  *is_large_direct_extraction
  )
{
    uint32
        action_ndx,
        action_lengths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
        action_lsbs[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
        nof_actions,
      res = SOC_SAND_OK;
    SOC_PPC_FP_DATABASE_STAGE
        stage;
    uint8
        has_action_for_fem,
        has_action_for_fes = FALSE;
    ARAD_TCAM_ACTION_SIZE
        action_size_needed;
    SOC_SAND_SUCCESS_FAILURE  
        success;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Get the correct stage */
  res = arad_pp_fp_db_stage_info_get(
            unit,
            info,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
  /* TM PFGs only at ingress PMF */
  if (stage != SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
      *is_large_direct_extraction = FALSE;
      ARAD_DO_NOTHING_AND_EXIT;
  }

    /* get from action vals lsbs */
    res = arad_pp_fp_action_to_lsbs(
            unit,
            stage,
            ((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? TRUE : FALSE ),
            info->action_types,
            info->action_widths,
            action_lsbs,
            action_lengths,
            &action_size_needed,
            &nof_actions,
            &success
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    if (success != SOC_SAND_SUCCESS) {
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d Invalid action composition.\n\r"),
                 unit));
        SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 101, exit);
    }

    /* 
     * Special case of Direct extraction with action size higher than 20b 
     * If so, no mix of actions with less and more than 20b: some use FES, 
     * some use FEM 
     */
    if (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) {
        has_action_for_fem = FALSE;
        has_action_for_fes = FALSE;
        for (action_ndx = 0; action_ndx < nof_actions; action_ndx ++)
        {
            if (action_lengths[action_ndx] > (SOC_DPP_DEFS_GET(unit, fem_max_action_size_nof_bits) + 1 /* Skip the TCAM valid bit */)) {
                has_action_for_fes = TRUE;
            }
            else {
                has_action_for_fem = TRUE;
            }
        }
        if (has_action_for_fes && has_action_for_fem) {
            /* Not valid */
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ACTION_LENGTHS_OUT_OF_RANGE_ERR, 90, exit);
        }
    }


  *is_large_direct_extraction = has_action_for_fes;

  /* if the user set the flag to use FES so this direct extraction will be treated as legre direct extraction*/
  if (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_ALLOCATE_FES) {
      *is_large_direct_extraction =1;
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_is_large_direct_extraction_get()", 0, 0);
}


STATIC uint32
  arad_pp_fp_direct_extraction_use_fes_get(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO     *info,
    SOC_SAND_OUT uint8                              *use_fes
  )
{
  uint32
      res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *use_fes = FALSE;

  /*check if there is more then 1 action - if yes, use FEM*/
  if ( info->actions[1].type == SOC_PPC_FP_ACTION_TYPE_INVALID )
  {
      /*There is only 1 action : find if it's 1:1 mapping and no filtering is imposed*/
      if ( info->actions->nof_fields == 1) 
      {
            if  (  (soc_sand_u64_is_zero(&info->qual_vals->is_valid)) && (info->actions->base_val == 0) && (info->actions->fld_ext->cst_val == 0)  )     
            {
                *use_fes = TRUE;
            }
      }
  }

  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_use_fes_get()", 0, 0);
}

uint32
  arad_pp_fp_dbal_table_create(int unit, uint32 db_id_ndx)
{
  uint32 min_prog_indx, max_prog_indx, program_id, pgm_bmp_used[1];
  uint32 res = SOC_SAND_OK;
  int nof_qualifiers = 0, i;
  char table_name[50];
  SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, stage_indx;
  SOC_DPP_DBAL_SW_TABLE_IDS table_id;
  SOC_DPP_DBAL_KEY_TO_TABLE keys_to_table_id[SOC_DPP_DBAL_PROGRAM_NOF_KEYS] = {{0}};
  SOC_DPP_DBAL_QUAL_INFO  qual_info[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  SOC_PPC_FP_DATABASE_INFO fp_database_info;  
  uint32 presel_bmp_update[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES_LEN], pmf_pgm_bmp_remain[1];
  uint8  psl_alloc_success;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  DBAL_QUAL_INFO_CLEAR(&qual_info, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
  
  res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  
  sprintf(table_name, "FLP Dynamic Table %d %s", db_id_ndx, arad_pp_dbal_physical_db_to_string(fp_database_info.physicalDB));

  /*if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
    sprintf(table_name, "FLP Dynamic Table %d %s", db_id_ndx, arad_pp_dbal_physical_db_to_string(fp_database_info.physicalDB));
  }else{  
    sprintf(table_name, "VTT Dynamic Table %d %s", db_id_ndx, arad_pp_dbal_physical_db_to_string(fp_database_info.physicalDB));
  }*/

  nof_qualifiers = 0;
  for(i = 0; ((i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX) && (fp_database_info.qual_types[i]!= SOC_PPC_NOF_FP_QUAL_TYPES) && (fp_database_info.qual_types[i]!= BCM_FIELD_ENTRY_INVALID) && (nof_qualifiers < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX - 1)); i++){
    uint8 qual_full_size = 0, is_in_hdr = 0, remain_bits = 0;

    res = arad_pp_dbal_qualifier_full_size_get(unit, stage, fp_database_info.qual_types[i], &qual_full_size, &is_in_hdr);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (qual_full_size > 32) {
      
      qual_info[nof_qualifiers].qual_type = fp_database_info.qual_types[i];
      qual_info[nof_qualifiers].qual_nof_bits = 32;
      qual_info[nof_qualifiers].qual_offset = 32;
      nof_qualifiers++;
      remain_bits =  qual_full_size - 32;
      if (remain_bits > 16) {/* in this case we will use 32 bit CE */
        qual_info[nof_qualifiers].qual_offset = 32 - remain_bits;
      } else{/* in this case we will use 16 bit CE */
        qual_info[nof_qualifiers].qual_offset = 16 - remain_bits;
      }
      qual_info[nof_qualifiers].qual_type = fp_database_info.qual_types[i];
      qual_info[nof_qualifiers].qual_nof_bits = remain_bits;
      nof_qualifiers++;     
    }else{      
      qual_info[nof_qualifiers].qual_type = fp_database_info.qual_types[i];
      nof_qualifiers++;
    }    
  }
  
  res = arad_pp_dbal_dynamic_table_create(unit, fp_database_info.physicalDB, nof_qualifiers, qual_info, table_name, &table_id);
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  res = arad_pp_dbal_table_actions_set(unit, table_id, fp_database_info.action_types);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  fp_database_info.internal_table_id = table_id;
  for (stage_indx = 0; stage_indx < SOC_PPC_NOF_FP_DATABASE_STAGES; stage_indx++) {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.set(
              unit,
              stage_indx,
              db_id_ndx,
              &fp_database_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit); 
  }  

  keys_to_table_id[0].sw_table_id = table_id;
  min_prog_indx = 0;
  max_prog_indx = ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX;

  res = arad_pmf_psl_pmf_pgms_get(unit, stage, &fp_database_info, 0, 0, pgm_bmp_used,
            pmf_pgm_bmp_remain, presel_bmp_update, &psl_alloc_success);
  
  for (program_id = min_prog_indx; program_id < max_prog_indx; ++program_id){
    if (SOC_SAND_GET_BIT(*pgm_bmp_used, program_id) != 0x1){
        /* Program not used */
        continue;
    }

    keys_to_table_id[0].key_id = SOC_DPP_DBAL_PROGRAM_NOF_KEYS; /* use SOC_DPP_DBAL_PROGRAM_NOF_KEYS to dynamic allocation */

    

    keys_to_table_id[0].lookup_number = 0xFF; /* use 0xFF to dynamic allocation */
    
    res = arad_pp_dbal_program_to_tables_associate(unit, program_id, /* stage */SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP, keys_to_table_id, NULL, 1/*num of lookups*/);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);   
            
  }
  
  LOG_CLI((BSL_META("\nDynamic Table created\n")));
  arad_pp_dbal_table_info_dump(unit, table_id); 

  

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_dbal_table_create()", db_id_ndx, 0);
}


/*********************************************************************
*     Create a database. Each database specifies the action
 *     types to perform and the qualifier fields for this
 *     Database. Entries in the database specify the specific
 *     actions to be taken upon specific values of the
 *     packet. E.g.: Policy Based Routing database update the
 *     FEC value according to DSCP DIP and In-RIF. An entry in
 *     the database may set the FEC of a packet with DIP
 *     1.2.2.3, DSCP value 7 and In-RIF 3 to be 9.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_database_create_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT  SOC_PPC_FP_DATABASE_INFO               *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE               *success
  )
{
  uint32
      res = SOC_SAND_OK;
  ARAD_PMF_DB_INFO
      fp_database_info; 
  uint32
      db_id_other_ndx,
      offset_lsb,
      forbidden_db_bmp[ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS)],
      presel_bmp_update[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES_LEN],
      action_lengths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
      action_lsbs[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
      total_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES] = {0},
      pmf_pgm_bmp_db,
      pmf_pgm_bmp_used,
      pmf_pgm_bmp_new,
      pmf_pgm_bmp_remain[1],
      pmf_pgm_ndx_min,
      pmf_pgm_ndx_max,
      pmf_pgm_ndx,
      pmf_pgm_ndx_duplicate,
      alloc_flags = 0,
      total_ce_indx = 0,
      selected_cycle[ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES],
    cycle_cons, 
    place_start,
      is_equal_place_cons,
    place_cons = 0, 
      nof_actions,
      nof_20s,
      table_entry[20] = {0},
      zone_idx,
      expected_size_in_bits,
      fes_info_idx,
      max_key_size;
  uint8
      is_default_db,
      is_large_direct_extraction = FALSE,
      is_for_tm,
      is_default_tm,
      switch_key_egq,
      is_cascaded,
      lookup_id = 0,
      is_equal = 0,
      fes_found = 0,
      pmf_pgm_alloced = 0,
      key_alloced = 0,
      action_alloced = 0,
      is_slb_hash_in_quals = FALSE;
  ARAD_PP_FEM_ACTIONS_CONSTRAINT
    action_const;
  ARAD_TCAM_ACCESS_INFO 
      tcam_info;
  uint8
      psl_alloc_success;
  ARAD_PP_FP_KEY_ALLOC_INFO    
      *alloc_info = NULL;
  SOC_PPC_FP_DATABASE_INFO
      other_db_info,
      fp_pp_database_info; 
  ARAD_TCAM_ACTION_SIZE  
      action_size_constraint = ARAD_TCAM_NOF_ACTION_SIZES,
      action_size_needed = 0xffffffff;
  uint32
      prefix_nof_bits_max = 0;
  ARAD_PP_FP_CE_CONSTRAINT     
      *ce_const = NULL;
  ARAD_TCAM_BANK_ENTRY_SIZE           
      key_size;
  ARAD_PMF_SEL_INIT_INFO  
      init_info;
  SOC_PPC_FP_DATABASE_STAGE
      stage;
  ARAD_PMF_FES
      *fes_info = NULL;
  SOC_PPC_FP_ACTION_TYPE
      action_type;
  SOC_SAND_SUCCESS_FAILURE  
      success_action;
  SOC_SAND_SUCCESS_FAILURE
      success_tcam;
  uint8 is_skip_hw = FALSE; 
  soc_error_t soc_res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_CREATE_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);
  SOC_SAND_CHECK_NULL_INPUT(success);
  *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;


  if ((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) || (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL) ) {
      is_skip_hw = TRUE;
  }

  sal_memset(selected_cycle, 0x0, sizeof(uint32) * ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES);
  sal_memset(forbidden_db_bmp, 0x0, sizeof(uint32) * ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS));
  
  /*
   *    Verify the Database does not already exist in any stage and set its type
   */
  for (stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage ++) {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
              unit,
              stage,
              db_id_ndx,
              &fp_database_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

      /* info->supported_pfgs*/
      if (fp_database_info.valid)
      {
        LOG_DEBUG(BSL_LS_SOC_FP,
                  (BSL_META_U(unit,
                              "   Error in database create: Database %d exists already\n\r"), db_id_ndx));
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_ALREADY_EXIST_ERR, 20, exit);
      }
  }

  /*
   *    Set the SW DB in any stage
   */
  for (stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage ++) {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.set(
              unit,
              stage,
              db_id_ndx,
              info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit); 
  }

  if ( is_skip_hw)
  {
      if(info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL) {
        if (info->internal_table_id == 0){
          /* only in this case we need to allocate new tables */
          res = arad_pp_fp_dbal_table_create(unit, db_id_ndx);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);
        }
      }
      *success = SOC_SAND_SUCCESS;
      ARAD_DO_NOTHING_AND_EXIT;
  }


  /* Get the correct stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  /* 
   * Special treatment for TM DBs
   */
  res = arad_pp_fp_database_is_tm_get(unit, info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  /* 
   * Special treatment for large Direct Extraction DBs
   */
  res = arad_pp_fp_database_is_large_direct_extraction_get(
          unit,
          info,
          &is_large_direct_extraction
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);

  /*
   * Program selection stage: 
   * - retrieve the relevant PMF-Programs and set it in the SW DB 
   * (arad_sw_db_pmf_db_prog_set) 
   * - compute the number of needed PMF-Programs to replicate and 
   * see if acceptable (considering removing the unused)
   */
  /* Detect if there is some preselector */
  is_default_db = (info->supported_pfgs_arad[0] || info->supported_pfgs_arad[1])? FALSE : TRUE;
  if (is_for_tm) {
      is_default_db = is_default_tm;
  }
  res = arad_pmf_psl_pmf_pgms_get(
            unit,
            stage,
            info,
            is_default_db, /* Relevant for all lines */
            is_for_tm,
            &pmf_pgm_bmp_used,
            pmf_pgm_bmp_remain,
            presel_bmp_update,
            &psl_alloc_success);
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  if(!psl_alloc_success) {
    LOG_ERROR(BSL_LS_SOC_FP,
              (BSL_META_U(unit,
                          "   Error in database create: For database %d, stage %s, no success for program selection computation\n\r"), 
               db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
      
      goto exit_failure;
  }

  /* Update the SW DB of the PMF-Programs to update */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.set(
            unit,
            stage,
            db_id_ndx,
            0,
            pmf_pgm_bmp_used
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  if ( is_for_tm && (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) ==0 )  &&
   ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0))==0)) {
    /* Skip the allocation verifications */
    goto exit_tm;
  }

  /* 
   * Compute all the constraints first
   */
  cycle_cons = (ARAD_PMF_LOW_LEVEL_NOF_CYCLES == 1)? /* Always first cycle at egress - single lookup at this stage */
                  ARAD_PP_FP_KEY_CYCLE_EVEN:ARAD_PP_FP_KEY_CYCLE_ANY;
  place_cons = ARAD_PP_FP_KEY_CE_PLACE_ANY;
  place_start = 0; 

  /* Insert a min key constraint only on the current Database */
 /* Check if there is a key-change action or qualifier */
  res = arad_pp_fp_db_cascaded_cycle_get(
          unit,
          db_id_ndx,
          &is_cascaded,
          &lookup_id /* 1st or 2nd cycle */
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
  
  if (SOC_IS_ARADPLUS(unit))
  {
      /* Check if a compare operation should be performed on this database,
       * according to database flags. 
       */
      res = arad_pp_fp_db_is_equal_place_get(
                unit,
                db_id_ndx,
                &is_equal,
                &is_equal_place_cons /* LSB or MSB */
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 75, exit);

      /* Check whether there is a hash value qualifier (data from SLB 74b LEM key). */
      soc_res = arad_pp_fp_is_qual_in_qual_arr(unit, 
                                               info->qual_types, 
                                               ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS, 
                                               SOC_PPC_FP_QUAL_KEY_AFTER_HASHING, 
                                               &is_slb_hash_in_quals);

      if (soc_res != SOC_E_NONE) {
          SOC_SAND_SET_ERROR_CODE(SOC_SAND_GEN_ERR, 11, exit);
      }
  }

  if (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) {
      
      place_start = ( soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "optimized_de_allocation", 0) )  ? ARAD_PP_FP_KEY_CE_LOW : ARAD_PP_FP_KEY_CE_HIGH;
      place_cons =  ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE;
      cycle_cons = ARAD_PP_FP_KEY_CYCLE_ODD;
      if (is_cascaded) {
          /* Cascaded database has a constraint of using the lsb part of the key
           * only, since the engine copies the bits from the action buffer to the
           * lower part (lsb) of the 2nd key.
           * * An additional constraint to be validated: Make sure in case of   *
           * * Multi-Databases sharing the same key, that the cascaded database *
           * * sits in the 20lsb bits of the lower (lsb) key.                   *
           */
          place_start = ARAD_PP_FP_KEY_CE_LOW;
          place_cons = ARAD_PP_FP_KEY_CE_LOW;
      }
  }
  else if((info->db_type == SOC_PPC_FP_DB_TYPE_FLP) || (info->db_type == SOC_PPC_FP_DB_TYPE_VT) || (info->db_type == SOC_PPC_FP_DB_TYPE_TT)) {
    /* 
     * No constraints on zone for FLP databases. 
     */
    place_start = ARAD_PP_FP_KEY_CE_LOW;
    place_cons = SOC_IS_JERICHO(unit)? ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE : ARAD_PP_FP_KEY_CE_LOW;
    cycle_cons = ARAD_PP_FP_KEY_CYCLE_EVEN;
  }
  else if(info->db_type == SOC_PPC_FP_DB_TYPE_SLB) {
    /* 
     * Constraints on SLB: always 1st (single) cycle, 
     * LSB or MSB according to the action 
     */
    place_cons = (info->action_types[0] == SOC_PPC_FP_ACTION_TYPE_SLB_HASH_VALUE)? ARAD_PP_FP_KEY_CE_LOW: ARAD_PP_FP_KEY_CE_HIGH;
    place_start = place_cons;
    cycle_cons = ARAD_PP_FP_KEY_CYCLE_EVEN;
  }
  else {
     /* get from action vals lsbs */
     res = arad_pp_fp_action_to_lsbs(
             unit,
             stage,
             ((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? TRUE : FALSE ),
             info->action_types,
             info->action_widths,
             action_lsbs,
             action_lengths,
             &action_size_needed,
             &nof_actions,
             &success_action
           );
     SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
     if (success_action != SOC_SAND_SUCCESS) {
       LOG_ERROR(BSL_LS_SOC_FP,
                 (BSL_META_U(unit,
                             "Unit %d DB-Id %d, Invalid action composition.\n\r"),
                  unit, db_id_ndx));
         SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 101, exit);
     }

     if(info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) {
         /* 
          * Always starts and ends in LSB
          */
         place_cons = ARAD_PP_FP_KEY_CE_LOW;
         place_start = ARAD_PP_FP_KEY_CE_LOW;
     }
     else {
             /* Get the min key size according to the action length - see Arad_pp_fp_place_const_convert for mapping */
             place_start = (action_size_needed >= ARAD_TCAM_ACTION_SIZE_THIRD_20_BITS)? ARAD_PP_FP_KEY_CE_PLACE_ANY: 
                                           (action_size_needed >= ARAD_TCAM_ACTION_SIZE_SECOND_20_BITS)? ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE:
                                           /*80b in LSB */ ARAD_PP_FP_KEY_CE_LOW;
     }
  }
  /* Indicate the cycle in case of cascaded */
  if (is_cascaded) {
      if (lookup_id == 0) {
          if (cycle_cons == ARAD_PP_FP_KEY_CYCLE_ODD) {
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "   Error in database create: "
                                  "For database %d creation, stage %s, no success in Cycle computation: cycle odd not allowed for 1st cycle cascaded group \n\r"),
                       db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 90, exit); 
          }
          cycle_cons = ARAD_PP_FP_KEY_CYCLE_EVEN;
      }
      else { /* lookup_id == 1 */
          if (cycle_cons == ARAD_PP_FP_KEY_CYCLE_EVEN) {
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "   Error in database create: "
                                  "For database %d creation, stage %s, no success in Cycle computation: cycle even not allowed for 2nd cycle cascaded group \n\r"),
                       db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
              SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 100, exit); /* need more bits in TCAM action as expected */
          }
          if (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) {
              place_start = place_start; /* Take Low instead of high */
          }
          else {
              place_start = SOC_SAND_MAX(ARAD_PP_FP_KEY_CE_LOW, place_start);
          }
          cycle_cons = ARAD_PP_FP_KEY_CYCLE_ODD;
      }
  }

  if (SOC_IS_ARADPLUS(unit))
  {
      if (is_equal) {
          place_cons  = is_equal_place_cons;
          place_start = is_equal_place_cons;
          cycle_cons  = ARAD_PP_FP_KEY_CYCLE_ODD;
      }
  }

  /* 
   *  Update the content of the PMF-Programs and of the TCAM:
   *  - Perform 2 steps, where:
   *  1. In the 1st step, compute the minimum size for the action, then key
   *  In the same step, create the TCAM DB. Destroy it if fail to allocate
   *  
   *  2. In the 2nd step, we have 2 rounds:
   *    a. Perform computations for key and action,
   *       Check that the allocation of the new data-base can be done for all programs
   *       (but with flags 'don't commit', neither in SW nor in HW)
   *    b. Perform all the round again, this time commit to HW & SW.
   */
    /* Just check in the 1st step */
    alloc_flags = ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY;
    if(info->db_type == SOC_PPC_FP_DB_TYPE_FLP) 
    {
        alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT;
    }
    if (is_large_direct_extraction 
        || (info->db_type == SOC_PPC_FP_DB_TYPE_SLB)) 
    {
        alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER;
    }
    if (SOC_IS_ARADPLUS(unit))
    {
        if (is_equal) 
        {
            alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER;
        }
    }

    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
        if (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SINGLE_BANK) 
        {
            alloc_flags |= ARAD_PP_FP_KEY_ALLOC_USE_KEY_A;
        }
    }
    
    ARAD_ALLOC(ce_const, ARAD_PP_FP_CE_CONSTRAINT, ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS, "arad_pp_fp_database_create_unsafe.ce_const");
    /*
     * allocate key resources for exist DB, that need to be duplicated in new programs
     */

    res = arad_pp_fp_key_alloc_constrain_calc(
              unit,
              stage,
              db_id_ndx,
              alloc_flags,
              info->qual_types,
              cycle_cons,
              place_cons,
              (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION),/* is_for_direct_extraction */
              ce_const,
              &total_ce_indx,
              &place_start,
              selected_cycle,
              total_bits_in_zone,
              &key_alloced
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
    if(!key_alloced) {
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "    "
                            "Key: fail to allocate Stage %s Database %d with flags %d, selected_cycle %d, "
                            "place_cons %d, total_ce_indx %d, place_start %d"  "\n\r"),
                 SOC_PPC_FP_DATABASE_STAGE_to_string(stage), db_id_ndx, alloc_flags,
                 cycle_cons, place_cons, total_ce_indx, place_start));
        *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
      
      goto exit_failure;
    }

   /* For Direct Table, verify key is not larger than 10 bits  */
    /*for KAPS should not be more than kaps key length (depends on number of entries)*/
   if(info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) {
       max_key_size = (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? ARAD_PP_FP_DIRECT_TABLE_KAPS_KEY_LENGTH : SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits);
       for(zone_idx = 0; zone_idx < ARAD_PP_FP_KEY_NOF_ZONES; zone_idx++) {
           if((zone_idx == 0 && total_bits_in_zone[zone_idx] > max_key_size) 
              || (zone_idx != 0 && total_bits_in_zone[zone_idx] != 0))
           {
             LOG_DEBUG(BSL_LS_SOC_FP,
                       (BSL_META_U(unit,
                                   "   Error in database create: "
                                   "For database %d creation, stage %s, key size is %d bits in zone %d,"
                                   " (max for Direct Table is 10 bits, zone 0 only)\n\r"),
                        db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), total_bits_in_zone[zone_idx], zone_idx));
               SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_ENTRY_SIZE_OUT_OF_RANGE_ERR, 120, exit);
           }
       }
   }

   if (SOC_IS_ARADPLUS(unit))
   {
       /* For Compare databases, verify key is not larger
        * than 80 bits (one key-zone is allowed)
        */
       if(is_equal) 
       {
           if(info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION){
               expected_size_in_bits = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE;
           }
           else{
               expected_size_in_bits = 0;
           }

           if(((total_bits_in_zone[0] != expected_size_in_bits) && (total_bits_in_zone[1] != expected_size_in_bits))
              || (total_bits_in_zone[2] != expected_size_in_bits) 
              || (total_bits_in_zone[3] != expected_size_in_bits))
           {
             LOG_DEBUG(BSL_LS_SOC_FP,
                       (BSL_META_U(unit,
                                   "   Error in database create: "
                                   "For Compare database %d creation, stage %s, "
                                   "only one half-key can be used\n\r"),
                        db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
               SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_ENTRY_SIZE_OUT_OF_RANGE_ERR, 120, exit);
           }
       }

       /* Make sure that there are at least 80 bits in either zone 1 or zone 3. */
       
       if (is_slb_hash_in_quals) {
           uint32 length_found, length_best_unused, length_worst_unused, nof_bits_for_slb_hash;

           arad_pp_fp_qual_length_get(unit, 
                                      SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, 
                                      SOC_PPC_FP_QUAL_KEY_AFTER_HASHING, 
                                      &length_found,
                                      &length_best_unused,
                                      &length_worst_unused,
                                      &nof_bits_for_slb_hash);
         
           /* The SLB hash key must be in the 80 MSBs of any of the two keys. */
           /* SOCDNX_VERIFY(found && ((total_bits_in_zone[1] >= nof_bits_for_slb_hash) || (total_bits_in_zone[3] >= nof_bits_for_slb_hash))); */
           if (!(length_found && ((total_bits_in_zone[1] >= nof_bits_for_slb_hash) || (total_bits_in_zone[3] >= nof_bits_for_slb_hash)))) {
              SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_ENTRY_SIZE_OUT_OF_RANGE_ERR, 120, exit); 
           }
           
       }
   }

    /*
    * for new DB, after check success, allocate TCAM DB profile 
    */

    /* Allocate TCAM Database */
    if((info->db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) 
       && (info->db_type != SOC_PPC_FP_DB_TYPE_FLP)
       && (info->db_type != SOC_PPC_FP_DB_TYPE_VT)
       && (info->db_type != SOC_PPC_FP_DB_TYPE_TT)
       && (info->db_type != SOC_PPC_FP_DB_TYPE_SLB)
       && !(info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS))
    {
        /* Check if action size is valid */
        
        action_size_constraint = (place_start == ARAD_PP_FP_KEY_CE_PLACE_ANY)?ARAD_TCAM_ACTION_SIZE_FORTH_20_BITS:
                                 ((place_start == ARAD_PP_FP_KEY_CE_LOW)?ARAD_TCAM_ACTION_SIZE_FIRST_20_BITS:
                                                          ARAD_TCAM_ACTION_SIZE_SECOND_20_BITS); 

        if(info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE){ /* not relevent for KAPS*/
            key_size = ARAD_TCAM_NOF_BANK_ENTRY_SIZES;
        }
        else {
            key_size = (place_start == ARAD_PP_FP_KEY_CE_PLACE_ANY)?ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS:
                ((place_start == ARAD_PP_FP_KEY_CE_LOW)?ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS:
                 ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS); 
        }

        prefix_nof_bits_max = ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE -
                                 ((key_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS)? total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_MSB_1]:
                                 ((key_size == ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS)? total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_MSB_0]:
                                                          total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_LSB_0]));
        /* check doesn't exceed Tcam action size */
        if(action_size_needed > action_size_constraint) {
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "   Error in database create: "
                                "For database %d creation, stage %s, no success in Action computation: action_size_needed %d and action_size_constraint %d \n\r"),
                     db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), action_size_needed, action_size_constraint));
            SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 130, exit_failure); /* need more bits in TCAM action as expected */
        }

        /* 
         * Small banks were indicated as the only banks for this DB, however,
         * it's a Direct Table with too large a key to fit in small bank
         */
        if((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SMALL_BANKS) 
           && ((info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) 
               && (total_bits_in_zone[0] > ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_DIRECT_TABLE_SMALL)))
        {
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "   Error in database create: "
                                "For database %d creation, stage %s, Direct Table with too large a key to fit in small bank (key size %d)\n\r"),
                     db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), total_bits_in_zone[0]));
            SOC_SAND_SET_ERROR_CODE(ARAD_TCAM_BANK_ENTRY_SIZE_OUT_OF_RANGE_ERR, 140, exit_failure); /* need more bits in TCAM action as expected */
        }


        /* create TCAM DB */
        arad_ARAD_TCAM_ACCESS_INFO_clear(&tcam_info);
        tcam_info.min_banks = 0; /* 0 - no allocation, 1 - guarantee a place for entries. Also 1 for 320b entries */
        tcam_info.prefix_size = (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) ? 0 : SOC_SAND_MIN(prefix_nof_bits_max, ARAD_TCAM_PREFIX_SIZE_MAX); 
        tcam_info.entry_size = key_size;
        tcam_info.callback = arad_pp_fp_tcam_callback;
        tcam_info.user_data = db_id_ndx;
        tcam_info.is_direct = (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) ? TRUE : FALSE;

        /* If the DB is Direct Lookup and key size is larger than 7 bits, then forbid the use of small banks */
        tcam_info.use_small_banks = 
            ((info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) 
             && (total_bits_in_zone[0] > ARAD_TCAM_BANK_KEY_SIZE_IN_BITS_DIRECT_TABLE_SMALL))? ARAD_TCAM_SMALL_BANKS_FORBID : 
                ((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SMALL_BANKS) ? ARAD_TCAM_SMALL_BANKS_FORCE : ARAD_TCAM_SMALL_BANKS_ALLOW);
        tcam_info.no_insertion_priority_order = (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_NO_INSERTION_PRIORITY_ORDER)? 1: 0;
        tcam_info.sparse_priorities = (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_SPARSE_PRIORITIES)? 1: 0;

        /* Set the bank owner according to the selected cycle
         * (TCAM bank can only be shared by DBs of the same cycle)
         */
        pmf_pgm_ndx = 0;
        ARAD_PP_FP_KEY_FIRST_SET_BIT(&pmf_pgm_bmp_used,pmf_pgm_ndx,ARAD_PMF_LOW_LEVEL_NOF_PROGS,ARAD_PMF_LOW_LEVEL_NOF_PROGS,FALSE,res);
        if(res != 0) {
             tcam_info.bank_owner = (stage == SOC_PPC_FP_DATABASE_STAGE_EGRESS) ? /* Different bank owner per stage */
               ARAD_TCAM_BANK_OWNER_EGRESS_ACL : 
                 ((selected_cycle[pmf_pgm_ndx] == 0) ? /* Different logical bank owner per cycle */
                  ARAD_TCAM_BANK_OWNER_PMF_0 : ARAD_TCAM_BANK_OWNER_PMF_1);
             res = SOC_SAND_OK;
        }

        /* Compute the action bitmap */
        tcam_info.action_bitmap_ndx = 0;
        switch (action_size_needed) {
        case ARAD_TCAM_ACTION_SIZE_FIRST_20_BITS:
            nof_20s = 1;  
            break;
        case ARAD_TCAM_ACTION_SIZE_SECOND_20_BITS:
            nof_20s = 2;  
            break;
        case ARAD_TCAM_ACTION_SIZE_THIRD_20_BITS:
            nof_20s = 3;  
            break;
        case ARAD_TCAM_ACTION_SIZE_FORTH_20_BITS:
            nof_20s = 4;  
            break;
        default:
          LOG_DEBUG(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "    "
                                "Error in action computation: "
                                "For stage %s, total action size needed: %d\n\r"),
                     SOC_PPC_FP_DATABASE_STAGE_to_string(stage), action_size_needed));
          LOG_DEBUG(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "\n\r")));
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 148, exit_failure);

        }
        SHR_BITSET_RANGE(&tcam_info.action_bitmap_ndx, 0, nof_20s); 
        /* Set the forbidden DBs: aggregation of the Databases with which some PMF-Program is shared */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.default_db_pmb.bit_range_read(unit, stage, 0, 0, ARAD_PMF_NOF_DBS, forbidden_db_bmp);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 150, exit);
        for (db_id_other_ndx = 0; db_id_other_ndx < SOC_PPC_FP_NOF_DBS; ++db_id_other_ndx)
        {
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
                    unit,
                    stage,
                    db_id_other_ndx,
                    &other_db_info
                  );
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 160, exit_failure);

            /* 
             * Forbidden DBs doesn't apply to Direct-Table and
             * Direct-Extraction DBs
             */
            if(other_db_info.db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION)
            {
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(
                          unit,
                          stage,
                          db_id_other_ndx,
                          0,
                          &pmf_pgm_bmp_db
                        );
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 170, exit_failure);

                if (pmf_pgm_bmp_db & pmf_pgm_bmp_used) {
                    SHR_BITSET(forbidden_db_bmp, db_id_other_ndx);
                }
            }
        }
        /* Clear this Database */
        SHR_BITCLR(forbidden_db_bmp, db_id_ndx);
        SHR_BITCOPY_RANGE(tcam_info.forbidden_dbs, ARAD_PP_FP_DB_ID_TO_TCAM_DB_SHIFT, forbidden_db_bmp, 0, SOC_PPC_FP_NOF_DBS);

        res = arad_tcam_access_create_unsafe(
                  unit,
                  ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), 
                  &tcam_info,
                  &success_tcam
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit_failure);

        if(success_tcam != SOC_SAND_SUCCESS) {
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "   Error in database create: "
                                "For database %d creation, stage %s,  no success for TCAM profile for database %d \n\r"), 
                     db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage), db_id_ndx));
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "   Parameters: Prefix size %d, entry size %s, bank owner (0-IngPMF, 3-EgrPMF) %d, Action bitmap %d \n\r"), tcam_info.prefix_size, 
                     SOC_TMC_TCAM_BANK_ENTRY_SIZE_to_string(tcam_info.entry_size), tcam_info.bank_owner, tcam_info.action_bitmap_ndx));
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "\n\r")));
            *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
            
            goto exit_failure;
        }
    }

exit_tm:
  *success = SOC_SAND_SUCCESS;

  /* 
   * 2nd part: at this stage, we have validated the basic configuration.
   *
   * Now, per PMF-Program to update: 
   * - Find an empty PMF-Program 
   * - Duplicate the PMF-Program at SW DB and HW 
   * - Insert the new Key 
   * - Insert the action macros 
   * - Update the Program selection 
   * - If not used anymore, suppress the current PMF-Program 
   *  
   * In case the stage is FLP (Use FLP programs and External TCAM) 
   * then there will be no need to perform the program duplication 
   * in order to write it, since the ACL configuration is performed 
   * during init while there is no traffic. Instead, there is only 
   * a need to configure relevant FLP programs.
   */

    res = arad_pmf_prog_select_pmf_pgm_borders_get(
              unit,
              stage,
              is_for_tm, 
              &pmf_pgm_ndx_min,
              &pmf_pgm_ndx_max
            );
    SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit_failure);

    ARAD_ALLOC(alloc_info, ARAD_PP_FP_KEY_ALLOC_INFO, 1, "arad_pp_fp_database_create_unsafe.alloc_info");

    if(info->db_type == SOC_PPC_FP_DB_TYPE_FLP)
    {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      /* In case the database is of FLP type, a KBP database will be
       * used rather than a TCAM DB. 
      */
      int cluster_id = -1;

      uint32 
        config_phase,
        pmf_pgm_bmp_new_sw,
        key_size_in_bytes = (total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_LSB_0] + 7) / 8,
        key_size_in_bytes_msb = 0,
        dip_sip_sharing_enabled;

      if(SOC_IS_JERICHO(unit)){
		  key_size_in_bytes_msb = (total_bits_in_zone[ARAD_PP_FP_KEY_ZONE_MSB_0] + 7) / 8;
	  }

      for (config_phase = 0; config_phase < 2; config_phase++) 
      {
        SOC_PPC_FP_QUAL_TYPE new_qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
        int i,j, nof_shared_quals = 0;
        int is_exists = 0;
        ARAD_PP_FP_SHARED_QUAL_INFO shard_qualifiers_info[MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM];

        for (i = 0; i < MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM; i++) {
          ARAD_PP_FP_SHARED_QUAL_INFO_clear(&shard_qualifiers_info[i]);
        }        

        sal_memcpy(new_qual_types, info->qual_types, sizeof(SOC_PPC_FP_QUAL_TYPE) * SOC_PPC_FP_NOF_QUALS_PER_DB_MAX);
        /* Allocate instructions in relevant FLP programs */
        alloc_flags = (config_phase == 0) ? ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY : 0;
        alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT;

        pmf_pgm_bmp_new = 0;

        arad_pp_fp_dip_sip_sharing_is_sharing_enabled_for_db(unit, stage, db_id_ndx, &dip_sip_sharing_enabled, &cluster_id);        

        for (pmf_pgm_ndx = pmf_pgm_ndx_min; pmf_pgm_ndx < pmf_pgm_ndx_max; ++pmf_pgm_ndx)
        {
            if (SOC_SAND_GET_BIT(pmf_pgm_bmp_used, pmf_pgm_ndx) != 0x1){
                /* PMF Program not used */
                continue;
            }                
            if (dip_sip_sharing_enabled) {            
              for (i = 0; i < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; i++) {
                if((info->qual_types[i] == SOC_PPC_NOF_FP_QUAL_TYPES) || (info->qual_types[i] == BCM_FIELD_ENTRY_INVALID)){
                  break;
                }                                
                dip_sip_sharing_qual_info_get(unit, pmf_pgm_ndx, new_qual_types[i], &is_exists, &(shard_qualifiers_info[nof_shared_quals]));
                if (is_exists) {                                    
                  nof_shared_quals++;

                  for (j = i; j < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX - nof_shared_quals; j++) {
                    new_qual_types[j] = info->qual_types[j+nof_shared_quals];
                  }
                  i = i -1;
                }
              }

              dip_sip_sharing_sharde_qualifiers_arrange(unit, pmf_pgm_ndx, shard_qualifiers_info);
            }            
            
            res = arad_pp_fp_elk_key_alloc_in_prog(
                      unit,
                      stage,
                      pmf_pgm_ndx,
                      db_id_ndx,
                      alloc_flags | ARAD_PP_FP_KEY_ALLOC_USE_CE_CONS, /* Exact CE repartition */
                      new_qual_types,
                      ce_const,
                      total_ce_indx,/* for each Qual which CE is allocated */
                      alloc_info,
                      &key_alloced
                  );
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 193, exit_failure);

            if(!key_alloced && (nof_shared_quals == 0)) {
              LOG_ERROR(BSL_LS_SOC_FP,
                        (BSL_META_U(unit,
                                    "    "
                                    "FLP Key: fail to allocate Database %d in program %d, with flags %d, selected_cycle %d, "
                                    "place_cons %d, ce_rsrc_bmp_glbl %d, total_ce_indx %d, place_start %d"  "\n\r"), 
                         db_id_ndx, pmf_pgm_ndx, alloc_flags, selected_cycle[pmf_pgm_ndx], place_cons, 0, 
                         total_ce_indx, place_start));
                *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                goto exit_failure; /* done with FAIL*/
            }
            else{
                SHR_BITSET(&pmf_pgm_bmp_new, pmf_pgm_ndx);
            }

            if (config_phase != 0) 
            {
                uint8 prog_id=0;

                res = arad_pp_prog_index_to_flp_app_get(unit,pmf_pgm_ndx,&prog_id);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 194, exit_failure);

                res = arad_pp_flp_elk_prog_config(unit,
                                                  pmf_pgm_ndx,
                                                  prog_id,
                                                  key_size_in_bytes_msb,
                                                  key_size_in_bytes);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 194, exit_failure);
            }
        }

        /* The next step is the KBP table and instruction configuration */
        
        pmf_pgm_bmp_new_sw = 0;
        res = arad_kbp_add_acl(
                unit,
                (ARAD_KBP_ACL_TABLE_ID_OFFSET + db_id_ndx), /* table_id */
                (info->action_types[0] - SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_0), /* search_id */
                pmf_pgm_bmp_used,
                pmf_pgm_ndx_min,
                pmf_pgm_ndx_max,
                key_size_in_bytes + key_size_in_bytes_msb,
                alloc_flags,
                info->min_priority,
                nof_shared_quals,
                shard_qualifiers_info,
                &pmf_pgm_bmp_new_sw,                
                success
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 195, exit_failure);

        /* Verify that pmf_pgm_bmp_new_sw and pmf_pgm_bmp_new are the same */
        if (pmf_pgm_bmp_new_sw != pmf_pgm_bmp_new) {
          LOG_ERROR(BSL_LS_SOC_FP,
                    (BSL_META_U(unit,
                                "    "
                                "FLP Key: fail to allocate Database %d with two different program bitmaps: pmf_pgm_bmp_new_sw %d and pmf_pgm_bmp_new %d \n\r"), 
                     db_id_ndx, pmf_pgm_bmp_new_sw, pmf_pgm_bmp_new));
          *success = SOC_SAND_FAILURE_INTERNAL_ERR;
        }

        if(*success != SOC_SAND_SUCCESS) {
          goto exit_failure;
        }
      }

#else
      *success = SOC_SAND_FAILURE_INTERNAL_ERR;
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_STAGE_OUT_OF_RANGE_ERR, 196, exit_failure);
#endif
  }
  else 
  {
    int round;

    ARAD_ALLOC(fes_info, ARAD_PMF_FES, ARAD_PMF_LOW_LEVEL_NOF_FESS, "arad_pp_fp_database_create_unsafe.fes_info");
    /* In order to check recourses allocation for databases that may be configured for multiple programs we will do 2 rounds:
     * Round 0: resource allocation test: verify that we have enough resources in every program in for this DB.
     *          We skip dataBase duplication because resource availability is not depended on the program number.
     *          Also, we skip the program allocation and free, from the same reason.
     * Round 1: Allocate and configure the DB. */

    for (round = 0; round < 2 ; round++)
    {
        if (round == 0) /* Test Loop */
            alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY;
        else
            alloc_flags = 0;

        if(info->db_type == SOC_PPC_FP_DB_TYPE_FLP)
        {
            alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_FLP_NO_SPLIT;
        }
        if (is_large_direct_extraction || (info->db_type == SOC_PPC_FP_DB_TYPE_SLB)) {
            alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER;
        }
        if (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_HEADER_SELECTION)
        {
            alloc_flags |= ARAD_PP_FP_KEY_ALLOC_CE_USE_HEADER_SELECTION; /*using second header index for inner mac qualifiers*/
        }
        if (SOC_IS_ARADPLUS_AND_BELOW(unit)) {
            if (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SINGLE_BANK)
            {
                alloc_flags |= ARAD_PP_FP_KEY_ALLOC_USE_KEY_A;
            }
        }
		pmf_pgm_bmp_new = 0;
				
        for (pmf_pgm_ndx = pmf_pgm_ndx_min; pmf_pgm_ndx < pmf_pgm_ndx_max; ++pmf_pgm_ndx)
        {
            if (SOC_SAND_GET_BIT(pmf_pgm_bmp_used, pmf_pgm_ndx) != 0x1){
                /* PMF Program not used */
                continue;
            }
            if (round == 0 ) /* Allocation Check - no need to allocate new program. *
                          *  we can check resources allocation on existing program*/
            {
                pmf_pgm_ndx_duplicate = pmf_pgm_ndx;
                pmf_pgm_alloced = 1; 
            }
            else
            {
                /* Allocate a PMF-Program (update SW DB inside) */
                res = arad_pmf_sel_prog_alloc(unit, stage, is_for_tm, &pmf_pgm_ndx_duplicate, &pmf_pgm_alloced);
                SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit_failure);
            }
            if (!pmf_pgm_alloced) {
                 /*Should always success, previous check already*/
              LOG_ERROR(BSL_LS_SOC_FP,
                        (BSL_META_U(unit,
                                    "    "
                                    "PMF-Program: fail to allocate Database %d in program %d\n\r"),
                         db_id_ndx, pmf_pgm_bmp_used));
                *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                goto exit_failure;  /*done with FAIL*/
            }

            if(round != 0)  /* Skip hw configuration and program duplication in alloc check */
            {
                /* For egress, look if a switch in the key is necessary */
                res = arad_pmf_psl_swicth_key_egq_get(unit, stage, pmf_pgm_ndx, db_id_ndx, info->strength, &switch_key_egq);
                SOC_SAND_CHECK_FUNC_RESULT(res, 210, exit_failure);

                /* Duplicate the SW DB and HW of PMF-Programs */
                res = arad_pmf_psl_pmf_pgm_duplicate(unit, stage, pmf_pgm_ndx, pmf_pgm_ndx_duplicate, switch_key_egq);
                SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit_failure);
                /* For SLB, set this program valid - lookup enable */
                if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB) {
                    SOC_SAND_SOC_IF_ERROR_RETURN(res, 222, exit, READ_IHB_CONSISTENT_HASHING_PROGRAM_VARIABLESm(unit, MEM_BLOCK_ANY, pmf_pgm_ndx, table_entry));
                    soc_IHB_CONSISTENT_HASHING_PROGRAM_VARIABLESm_field32_set(unit, table_entry, ENABLE_LOOKUPf, 0x1 /* valid */);
                    SOC_SAND_SOC_IF_ERROR_RETURN(res, 224, exit, WRITE_IHB_CONSISTENT_HASHING_PROGRAM_VARIABLESm(unit, MEM_BLOCK_ANY, pmf_pgm_ndx, table_entry));
                }

                /* Set that this DB is using this Program. */
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_db_pmb.bit_set(unit, stage, pmf_pgm_ndx_duplicate, db_id_ndx);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit_failure);
            }

            /* Different allocations if TM or Ethernet */
            if ( is_for_tm && (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) ==0 )  &&
                 ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0))==0)) {
                /*
                 * Get the action type and the offset-lsb
                 * for the FES configuration
                 */
                res = arad_pp_fp_qual_tm_verify(
                          unit,
                          stage,
                          info,
                          &action_type,
                          &offset_lsb
                        );
                SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit_failure);

                for(fes_info_idx = 0; fes_info_idx < ARAD_PMF_LOW_LEVEL_NOF_FESS; fes_info_idx++) {
                    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.get(
                            unit,
                            stage,
                            pmf_pgm_ndx_duplicate,
                            fes_info_idx,
                            &fes_info[fes_info_idx]
                        );
                    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 47, exit_failure);
                }
                /*
                 * Find a free FES and configure it
                 */
                res = arad_pp_fp_action_alloc_fes(
                          unit,
                          db_id_ndx,
                          pmf_pgm_ndx_duplicate,
                          -1,
                          action_type,
                          ARAD_PP_FP_FEM_ALLOC_FES_TM | ((round == 0) ? ARAD_PP_FP_FEM_ALLOC_FES_CHECK_ONLY:0),
                          0,
                          NULL, /* *constraint */
                          offset_lsb /* action_lsb */,
                          0 /* action_len */,
                          -1, /* Ignore this input */
                          fes_info,
                          &fes_found
                        );
                SOC_SAND_CHECK_FUNC_RESULT(res, 49, exit_failure);
                if (!fes_found)
                {
                    LOG_ERROR(BSL_LS_SOC_FP,
                              (BSL_META_U(unit,
                                          "    "
                                          "FES: fail to allocate Database %d in program %d, with action_type %d, offset_lsb %d \n\r"),
                               db_id_ndx, pmf_pgm_ndx_duplicate, action_type, offset_lsb));
                      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                      goto exit_failure; 
                }
            }
            /* Do not allocate action and key for large DE DBs, inserted on entry insertion */
            else if (!is_large_direct_extraction) {
                /* Allocate the Key */
                res = arad_pp_fp_key_alloc_in_prog(
                          unit,
                          stage,
                          pmf_pgm_ndx_duplicate,
                          db_id_ndx,
                          alloc_flags | ARAD_PP_FP_KEY_ALLOC_USE_CE_CONS, /* Exact CE repartition */
                          info->qual_types,
                          selected_cycle[pmf_pgm_ndx],
                          place_cons,
                          place_start,
                          ce_const,
                          total_ce_indx,/* for each Qual which CE is allocated */
                          alloc_info,
                          &key_alloced
                       );
                SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit_failure);
                if(!key_alloced) {
                  LOG_ERROR(BSL_LS_SOC_FP,
                            (BSL_META_U(unit,
                                        "    "
                                        "Key: fail to allocate pgm_idx %d Database %d in program %d, pmf_pgm_alloced %d with flags %d, selected_cycle %d, "
                                        "place_cons %d, ce_rsrc_bmp_glbl %d, total_ce_indx %d, place_start %d"  "\n\r"),
                                    pmf_pgm_ndx, db_id_ndx, pmf_pgm_ndx_duplicate, pmf_pgm_alloced, alloc_flags, selected_cycle[pmf_pgm_ndx], place_cons, 0,
                             total_ce_indx, place_start));
                    *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                    goto exit_failure; /* done with FAIL*/
                }

                /* Do not allocate FES / FEM at egress  - there is none */
                /* For Direct Extraction, allocate at the entry insertion */
                if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) &&  (info->db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION)) {
                    ARAD_PP_FEM_ACTIONS_CONSTRAINT_clear(&action_const);
                    action_const.action_size = ARAD_TCAM_ACTION_SIZE_FORTH_20_BITS;

                    /* Allocate the Action */
                    action_const.tcam_res_id[0] = alloc_info->key_id[0];
                    action_const.tcam_res_id[1] = alloc_info->key_id[1];

                    action_const.cycle = selected_cycle[pmf_pgm_ndx];

                    res = arad_pp_fp_action_alloc_in_prog(
                              unit,
                              db_id_ndx,
                              pmf_pgm_ndx_duplicate,
                              alloc_flags,
                              info->action_types,
                              info->strength,
                              &action_const,
                              &action_alloced
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 260, exit_failure);

                    if(!action_alloced) {
                      LOG_ERROR(BSL_LS_SOC_FP,
                                (BSL_META_U(unit,
                                            "    "
                                            "FES: fail to allocate for DB %d, program %d, flags %d, priority %d, constraint action size %d \n\r"),
                                 db_id_ndx, pmf_pgm_ndx_duplicate, alloc_flags,  info->strength, action_const.action_size));
                        *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                        goto exit_failure; /* done with FAIL*/
                    }
                }
            }

            if (round != 0 ) /* Skip for testing round. */
            {
            /* Update the Resource bitmap and the used PMF-Programs for this DB */
            SHR_BITSET(&pmf_pgm_bmp_new, pmf_pgm_ndx_duplicate);

            /* Update the Preselector line's PMF program at SW DB and HW */
            res = arad_pmf_psl_pmf_pgm_update(unit, stage, is_for_tm, presel_bmp_update, FALSE /* is_for_all_lines */, pmf_pgm_ndx, pmf_pgm_ndx_duplicate);
            SOC_SAND_CHECK_FUNC_RESULT(res, 270, exit_failure);
                /* See if this PMF-Program is still in use */
                if (!is_default_db) {
                    /* The default Ethernet program should be removed only for Default DBs */
                    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.init_info.get(unit, stage, &init_info);
                    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 280, exit_failure);
                    SHR_BITSET(pmf_pgm_bmp_remain, init_info.pmf_pgm_default[ARAD_PMF_PSL_TYPE_ETH]);
                }
                if (!SHR_BITGET(pmf_pgm_bmp_remain, pmf_pgm_ndx)) {
                    /* Suppress this program */
            res = arad_pmf_sel_prog_free(unit, stage, is_for_tm, pmf_pgm_ndx, pmf_pgm_ndx_duplicate);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 290, exit_failure);

            res = arad_pmf_psl_second_pass_update(unit, stage, info->db_staggered_info, pmf_pgm_ndx, pmf_pgm_ndx_duplicate );
                }
            }
        }
    }
  }

  /* Update the PMF-Programs of this Database (for destroy for ex.) */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.set(
            unit,
            stage,
            db_id_ndx,
            0,
            pmf_pgm_bmp_new
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 300, exit_failure);

  /*
   * Set the SW DB to void
   */
  res = arad_fp_sw_db_info_commit(
          unit,
          stage,
          db_id_ndx,
          info,
          is_default_db,
          *success
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 310, exit_failure);

goto exit;

exit_failure:
  /* Restore a free Database in case of failure */
  if (*success != SOC_SAND_SUCCESS) {
     SOC_PPC_FP_DATABASE_INFO_clear(&fp_pp_database_info);
     for (stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage ++) {
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.set(
                  unit,
                  stage,
                  db_id_ndx,
                  &fp_pp_database_info
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 320, exit); 

          /* Clear the SW DB of the PMF-Programs */
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.set(
                    unit,
                    stage,
                    db_id_ndx,
                    0,
                    0
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 330, exit);
      }
  }
    res = arad_tcam_access_destroy_unsafe(
          unit,
          ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx)
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 42, exit);

exit:
  ARAD_FREE(alloc_info);
  ARAD_FREE(ce_const);
  ARAD_FREE(fes_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_create_unsafe()", db_id_ndx, 0);
}


uint32
  arad_pp_fp_database_create_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO                    *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  )
{
  uint32
      pfg_ndx,
      pfg_long_ndx,
      pfg_bit_ndx,
      action_lengths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
      action_lsbs[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
    db_key_length_min,
    key_max_length,
      offset_lsb,
      nof_actions,
    res = SOC_SAND_OK;
  ARAD_TCAM_ACTION_SIZE
      action_length_max,
      action_size_needed;
  SOC_PPC_PMF_PFG_INFO
      pfg_info_current;
  SOC_PPC_FP_DATABASE_STAGE
      stage;
  uint8
      is_default_tm,
      is_large_direct_extraction,
      is_for_tm;
  SOC_PPC_FP_ACTION_TYPE     
      action_type;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_CREATE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FP_DATABASE_INFO, info, 20, exit);

  /* Get the correct stage */
  res = arad_pp_fp_db_stage_info_get(
            unit,
            info,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  /* 
   * Verify the PFGs: 
   * - in an acceptable range (e.g. 0-47) 
   * - that they exist and defined
   */
  for (pfg_long_ndx = 0; pfg_long_ndx < SOC_PPC_FP_NOF_PFGS_IN_LONGS_ARAD; pfg_long_ndx++) {
      for (pfg_bit_ndx = 0; pfg_bit_ndx < 32; pfg_bit_ndx++) {
          if (SHR_BITGET(&(info->supported_pfgs_arad[pfg_long_ndx]), pfg_bit_ndx)) {
              pfg_ndx = pfg_bit_ndx + (pfg_long_ndx * 32);
              if (pfg_ndx > ARAD_PP_FP_PFG_NDX_MAX) {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 38, exit);
              }

              pfg_info_current.stage = stage;

              /* Find if it exists */
              res = arad_pp_fp_packet_format_group_get_unsafe(
                        unit,
                        pfg_ndx,
                        &pfg_info_current
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
              if (!pfg_info_current.is_array_qualifier) {
                  /* The PFG is not defined, error */
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 39, exit);
              }

              if (pfg_info_current.stage != stage) {
                  /* The PFG is defined for another stage, error */
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PFG_NDX_OUT_OF_RANGE_ERR, 42, exit);
              }
          }
      }
  }

  /* 
   * Verify the Database content if it is for TM: all the PFGs are TM
   */
  res = arad_pp_fp_database_is_tm_get(unit, info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);
  if ( is_for_tm && (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) ==0 )  &&
   ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0))==0)) {
      /* 
       * Verify the TM DB content: 
       * 1. There must be only one qualifier - UDF starting at 
       * base-header  = Forwarding + 1 
       * 2. There must be only one action 
       */ 
      res = arad_pp_fp_qual_tm_verify(
                unit,
                stage,
                info,
                &action_type,
                &offset_lsb
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);
  }



  /*
   * Verify the qualifier lengths can be inserted in an admissible key
   */
    res = arad_pp_fp_qual_verify(
            unit,
            info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 51, exit);

    res = arad_pp_fp_ce_key_length_minimal_get(
            unit,
            info,
            &db_key_length_min
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 52, exit);

    switch (info->db_type)
    {
    case SOC_PPC_FP_DB_TYPE_FLP:
    case SOC_PPC_FP_DB_TYPE_VT:
    case SOC_PPC_FP_DB_TYPE_TT:
      key_max_length = SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_FLP;
      break;
    case SOC_PPC_FP_DB_TYPE_SLB:
      key_max_length = SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_SLB;
      break;
    case SOC_PPC_FP_DB_TYPE_TCAM:
    case SOC_PPC_FP_DB_TYPE_EGRESS:
      key_max_length = SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_TCAM;
      break;
    case SOC_PPC_FP_DB_TYPE_DIRECT_TABLE:
      key_max_length =  (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? ARAD_PP_FP_DIRECT_TABLE_KAPS_KEY_LENGTH  : SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits);
      break;
    case SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION:
      key_max_length = SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_DIRECT_EXTRACTION;
      break;
    default:
      key_max_length = 0;
    }

    if (SOC_IS_ARADPLUS(unit))
    {
        uint8 is_slb_hash_in_quals = FALSE;
        soc_error_t soc_res;

        /* Check whether there is a hash value qualifier (data from SLB 74b LEM key). */
        soc_res = arad_pp_fp_is_qual_in_qual_arr(unit, 
                                                 info->qual_types, 
                                                 SOC_PPC_FP_NOF_QUALS_PER_DB_MAX, 
                                                 SOC_PPC_FP_QUAL_KEY_AFTER_HASHING, 
                                                 &is_slb_hash_in_quals);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(soc_res, 151, exit);

        if((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_LSB)
           || (info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_IS_EQUAL_MSB)
           || (is_slb_hash_in_quals))
        {
            key_max_length = ARAD_PP_FP_ACTION_TOTAL_LENGTH_IN_BITS_TCAM(unit);
        }
    }

    if (db_key_length_min > key_max_length)
    {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_QUALS_LENGTHS_OUT_OF_RANGE_ERR, 60, exit);
    }

    /*
     * Verify the action lengths can be inserted in
     * an admissible length for DT and TCAM
     */
        /* get from action vals lsbs */
        res = arad_pp_fp_action_to_lsbs(
                unit,
                stage,
                ((info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? TRUE : FALSE ),
                info->action_types,
                info->action_widths,
                action_lsbs,
                action_lengths,
                &action_size_needed,
                &nof_actions,
                success
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

        if (*success != SOC_SAND_SUCCESS) {
            ARAD_DO_NOTHING_AND_EXIT;
        }

        if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) || (stage == SOC_PPC_FP_DATABASE_STAGE_EGRESS)) { /* do not check for FLP */
           if ((info->db_type == SOC_PPC_FP_DB_TYPE_TCAM) || (info->db_type == SOC_PPC_FP_DB_TYPE_EGRESS))
            {
              action_length_max = ARAD_TCAM_ACTION_SIZE_FORTH_20_BITS;
            }
            else if (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE)
            {
              action_length_max = ( info->flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) ?  ARAD_PP_FP_DIRECT_TABLE_KAPS_ACTION_MAX_LENGTH : ARAD_TCAM_ACTION_SIZE_SECOND_20_BITS;
            }
            else
            {
              action_length_max = ARAD_TCAM_NOF_ACTION_SIZES;
            }

            if (action_size_needed > action_length_max)
            {
              SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ACTION_LENGTHS_OUT_OF_RANGE_ERR, 80, exit);
            }
        }

    if (info->db_type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION) {
        /* Verify the action composition */
        res = arad_pp_fp_database_is_large_direct_extraction_get(
                unit,
                info,
                &is_large_direct_extraction
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
    }

    if (info->db_type == SOC_PPC_FP_DB_TYPE_SLB) {
        if (SOC_IS_ARAD_B1_AND_BELOW(unit)) {
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ACTION_LENGTHS_OUT_OF_RANGE_ERR, 100, exit);
        }
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_create_verify()", db_id_ndx, 0);
}


/*********************************************************************
*     Get the database parameters.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_database_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_OUT SOC_PPC_FP_DATABASE_INFO                    *info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_STAGE
      stage; 
  uint8
      is_for_tm,
      is_default_tm;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Get the correct stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);



  SOC_PPC_FP_DATABASE_INFO_clear(info);
  /*
   * Get the DB type and verify if it exists
   */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
          unit,
          stage,
          db_id_ndx,
          info
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 25, exit);

  /* In case of TM and IN_LIF, do not take it in account */
  res = arad_pp_fp_database_is_tm_get(unit, info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_get_unsafe()", db_id_ndx, 0);
}

uint32
  arad_pp_fp_database_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_get_verify()", db_id_ndx, 0);
}

/*********************************************************************
*     Destroy the database: all its entries are suppressed and
 *     the Database-ID is freed.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_database_destroy_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx
  )
{
  uint32
    tcam_db_id,
      prog_rsrc,
      pmf_pgm_ndx_used,
      pmf_pgm_ndx_new,
      pmf_pgm_bmp_used,
      pmf_pgm_ndx_min,
      pmf_pgm_ndx_max,
      res = SOC_SAND_OK,
      default_db_pmb_used[ARAD_BIT_TO_U32(ARAD_PMF_NOF_DBS)];
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info;
  ARAD_FP_ENTRY
    fp_entry;
  SOC_PPC_FP_DATABASE_STAGE
      stage; 
  uint8
      is_default_tm,
      is_large_direct_extraction,
      is_for_tm;
  uint8
      is_equal;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_DESTROY_UNSAFE);

  /*
   * 1. Verify the Database exists, otherwise exit
   */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  if ((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL)){
      res = arad_pp_dbal_table_destroy(unit, fp_database_info.internal_table_id);
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      res = arad_pp_fp_db_stage_get(unit, db_id_ndx, &stage );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      res = arad_fp_sw_db_info_clear(unit, stage, db_id_ndx, &fp_database_info);
      SOC_SAND_CHECK_FUNC_RESULT(res, 190, exit);
      ARAD_PP_DO_NOTHING_AND_EXIT;
  }

  if ((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) || (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL))
  {
     goto exit_without_field_hardware_update;
  }

  if (fp_database_info.db_type == SOC_PPC_NOF_FP_DATABASE_TYPES)
  {
    LOG_WARN(BSL_LS_SOC_FP,
             (BSL_META_U(unit,
                         "Unit %d, DB id index %d, The data base to destroy doesn\'t exist.\n\r"), unit, db_id_ndx));
    ARAD_PP_DO_NOTHING_AND_EXIT;
  }

  /* Get the correct stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_fp_database_is_large_direct_extraction_get(
          unit,
          &fp_database_info,
          &is_large_direct_extraction
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

  /* 
   * Verify no entry if not TCAM, otherwise return error
   */
  if ((fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_TCAM)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_EGRESS))
  {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
              unit,
              stage,
              db_id_ndx,
              &fp_entry
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
      if (fp_entry.nof_db_entries)
      {
      LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d DB id index %d - Can\'t destroy the data base. The data base has entries which haven\'t been destroied.\n\r"),
                 unit, db_id_ndx));
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_NOF_ENTRIES_OUT_OF_RANGE_ERR, 40, exit);
      }
  }

  /* 
   * Special for TM DBs
   */
  res = arad_pp_fp_database_is_tm_get(unit, &fp_database_info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  /* 
   * Get the list of PMF-Programs of this Database 
   * In case it is identical to some other existing PMF-Program 
   * (i.e. the list of DBs supported is identical), then 
   * redirect the presel line to this program to minimize the 
   * number of PMF-Programs used. 
   */
  /* Get the SW DB of the PMF-Programs used */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(unit, stage, db_id_ndx, 0, &pmf_pgm_bmp_used);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.progs.get(unit, stage, 0, &prog_rsrc);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

  res = arad_pmf_prog_select_pmf_pgm_borders_get(
            unit,
            stage,
            is_for_tm, 
            &pmf_pgm_ndx_min,
            &pmf_pgm_ndx_max
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);

  for (pmf_pgm_ndx_used = pmf_pgm_ndx_min; pmf_pgm_ndx_used < pmf_pgm_ndx_max; ++pmf_pgm_ndx_used)
  {
      if (SOC_SAND_GET_BIT(pmf_pgm_bmp_used, pmf_pgm_ndx_used) != 0x1){
          /* PMF Program not used */
          continue;
      }

      /* Remove this DB from the used program. */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_db_pmb.bit_clear(unit, stage, pmf_pgm_ndx_used, db_id_ndx);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

      /* Find a PMF-Program identical */
      for (pmf_pgm_ndx_new = pmf_pgm_ndx_min; pmf_pgm_ndx_new < pmf_pgm_ndx_max; ++pmf_pgm_ndx_new)
      {
          /*
           * There cannot be 2 PMF-Programs identical with this Database 
           * so skip the used PMF-Programs when looking for an identical DB 
           * without the current DB 
           */ 
          if (SOC_SAND_GET_BIT(pmf_pgm_bmp_used, pmf_pgm_ndx_new) == 0x1) {
              continue;
          }
          if (SOC_SAND_GET_BIT(prog_rsrc, pmf_pgm_ndx_new) != 0x1) {
              /* PMF-Program not in used */
              continue;
          }

          /* Compare the DB list of both */
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_db_pmb.bit_range_read(unit, stage, pmf_pgm_ndx_used, 0, 0, ARAD_PMF_NOF_DBS, default_db_pmb_used);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 105, exit);
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_db_pmb.bit_range_eq(unit, stage, pmf_pgm_ndx_new, default_db_pmb_used, 0, ARAD_PMF_NOF_DBS, &is_equal);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 110, exit);
          if(is_equal) {
              /* Update the Preselector line's PMF program at SW DB and HW */
              res = arad_pmf_psl_pmf_pgm_update(unit, stage, is_for_tm, NULL, TRUE, pmf_pgm_ndx_used, pmf_pgm_ndx_new);
              SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);

             /* Suppress this program */
              res = arad_pmf_sel_prog_free(unit, stage, is_for_tm, pmf_pgm_ndx_used, pmf_pgm_ndx_new);
              SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
          }
      }

  }


  /* 
   * Different processing for TM
   */
  if (is_for_tm) {
      res = arad_pp_fp_action_dealloc(
              unit,
              db_id_ndx,
              fp_database_info.action_types
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);
  }
  else if (!is_large_direct_extraction) {
    /*
     * Remove the FES configuration - only for TCAM  
     */
    if ((fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_TCAM) 
        || (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE ))
    {
        res = arad_pp_fp_action_dealloc(
                unit,
                db_id_ndx,
                fp_database_info.action_types
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);
    }

    /*
     * Remove CE configuration - must be before the TCAM destroy 
     * due to dependences 
     */
    res = arad_pp_fp_key_dealloc(
            unit,
            stage,
            db_id_ndx,
            fp_database_info.qual_types
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);

exit_without_field_hardware_update:
    /*
     * If TCAM, remove the Database
     */
    if ((fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_TCAM) 
        || ((fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) && !( fp_database_info.flags &  SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ))
        || (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_EGRESS))
    {
      tcam_db_id = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx);
      res = arad_tcam_access_destroy_unsafe(
              unit,
              tcam_db_id
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);
    }
    if((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) || (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL)) {
      ARAD_DO_NOTHING_AND_EXIT;
    }
  }


  /*
   * Update SW DB
   */
  res = arad_fp_sw_db_info_clear(
          unit,
          stage,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 190, exit);


  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_destroy_unsafe()", db_id_ndx, 0);
}

uint32
  arad_pp_fp_database_destroy_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_DESTROY_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_destroy_verify()", db_id_ndx, 0);
}

/* Set entry_dt_typical and bit_meaningful_mask to indicate
 * which entries should be configured for Direct Table
 */
STATIC
uint32
  arad_pp_fp_entry_ndx_direct_table_get(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 db_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO  *info,
    SOC_SAND_OUT uint32                 *bank_id,
    SOC_SAND_OUT uint32                 *entry_dt_typical,
    SOC_SAND_OUT uint32                 *bit_meaningful_mask
  )
{
  uint32
    res = SOC_SAND_OK,
    qual_lsb,
    qual_length_no_padding,
    entry_dt_first,
    is_dt_bit_meaningful = 0,
    bit_ndx,
    dt_bit_max,
    qual_type_ndx,
    bank_ndx = 0;
  uint8
    is_used, is_tcam_dt;
  ARAD_PMF_DB_INFO /* update db info */
        db_info; 
  SOC_PPC_FP_DATABASE_INFO  fp_database_info;    


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_NDX_DIRECT_TABLE_GET);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);    

  /* Get the database Info */
  res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  is_tcam_dt = !( fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) ? TRUE : FALSE ;

  /* Find which bank this database uses (in case of KAPS - which BB)*/

  if (!is_tcam_dt) {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
              unit,
              SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
              db_id_ndx,
              &db_info
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 180, exit);

      *bank_id = db_info.kaps_db_id ; 
  }

  else {

      for(bank_ndx = 0; bank_ndx < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_ndx++)
      {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(
                unit, 
                ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), 
                bank_ndx,
                &is_used
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

        if(is_used)
        {
          break;
        }
      }

      /* In case no bank is used for this DB, allocate a bank */
      if(bank_ndx >= SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit)) {
        SOC_SAND_SET_ERROR_CODE(SOC_PPC_FP_NOF_DBS_PER_BANK_OUT_OF_RANGE_ERR, 40, exit);
      }

      *bank_id = bank_ndx;
  }

  entry_dt_first = 0;
  for (qual_type_ndx = 0; qual_type_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX ; ++qual_type_ndx)
  {
    if ((info->qual_vals[qual_type_ndx].type == SOC_PPC_NOF_FP_QUAL_TYPES) || ((info->qual_vals[qual_type_ndx].type == BCM_FIELD_ENTRY_INVALID))){
      break;
    }

    res = arad_pp_fp_qual_lsb_and_length_get(
            unit,
            db_id_ndx,
            info->qual_vals[qual_type_ndx].type,
            &qual_lsb,
            &qual_length_no_padding
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

    dt_bit_max = (is_tcam_dt) ? ARAD_TCAM_KEY_LENGTH_DIRECT_TABLE_PER_BANK(unit, bank_ndx) : ARAD_PP_FP_DIRECT_TABLE_KAPS_KEY_LENGTH ;

    for (bit_ndx = 0; bit_ndx < dt_bit_max; ++bit_ndx)
    {
        if ((bit_ndx >= qual_lsb)
            && (bit_ndx < qual_lsb + qual_length_no_padding)
           )
        {
          if ((info->qual_vals[qual_type_ndx].is_valid.arr[0] & (1 << (bit_ndx - qual_lsb))) != 0)
          {
            SOC_SAND_SET_BIT(is_dt_bit_meaningful, 0x1, bit_ndx);
          }
        }
    }

    entry_dt_first |= SOC_SAND_SET_BITS_RANGE(info->qual_vals[qual_type_ndx].val.arr[0], qual_lsb + qual_length_no_padding - 1, qual_lsb);
  }

  *entry_dt_typical = entry_dt_first;
  *bit_meaningful_mask = is_dt_bit_meaningful;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_ndx_direct_table_get()", 0, 0);
}




uint32
  arad_pp_fp_direct_table_entry_add_unsafe(
     SOC_SAND_IN  int                        unit,
     SOC_SAND_IN  uint32                     db_id_ndx,
     SOC_SAND_IN  uint32                     entry_id_ndx,
     SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO      *info,
     SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE     stage,
     SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success)
{
    uint32  res = SOC_SAND_OK,
            entry_ndx_dt,
            entry_ndx_dt_max,
            entry_dt_first = 0,
            is_dt_bit_meaningful = 0,
            phase_ndx,
            buffer_size,
            bank_id = 0;
    uint8   is_used, is_tcam_dt;
    ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx;
    ARAD_PP_FP_LOCATION_HASH  dt_key;
    ARAD_FP_ENTRY             fp_entry;
    ARAD_TCAM_ACTION          action;
    SOC_PPC_FP_DATABASE_INFO  fp_database_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);    

    /* Get the database Info */
    res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    is_tcam_dt = !( fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) ? TRUE : FALSE ;

    ARAD_TCAM_ACTION_clear(&action);

  /* map action to tcam action buffer */
    res = arad_pp_fp_action_value_to_buffer(unit,info->actions,db_id_ndx,action.value, &buffer_size);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if ( is_tcam_dt ) 
    {
        /* If number of entries is 0, then a bank should be allocated */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
                unit,
                stage,
                db_id_ndx,
                &fp_entry
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);

        if(0 == fp_entry.nof_db_entries)
        {
            /* allocate bank */
            res = arad_tcam_managed_db_direct_table_bank_add(
                    unit, 
                    ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
                    success
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 36, exit);

            /* if bank allocation failed */
            if(SOC_SAND_SUCCESS != *success)
            {
                ARAD_DO_NOTHING_AND_EXIT;
            }
        }

        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
                unit,
                ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
                &action_bitmap_ndx
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
    }
    /* Entry insertion in Direct Table DBs is in location as
     * specified by 10/7 bit key (depending on bank type -
     * large or small). 
     * Replicate the entry as long as needed
     */
    res = arad_pp_fp_entry_ndx_direct_table_get(
            unit,
            db_id_ndx,
            info,
            &bank_id,
            &entry_dt_first,
            &is_dt_bit_meaningful
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    /*
     *  Add the new entry to the entry_id -> location hash table
     */
    if ( is_tcam_dt ) 
    {
        dt_key.entry_dt_key.value = entry_dt_first;
        dt_key.entry_dt_key.mask = is_dt_bit_meaningful;
        res = arad_tcam_db_entry_id_to_location_entry_add(
                unit,
                ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
                entry_id_ndx,
                &(dt_key.location)
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);

        *success = SOC_SAND_SUCCESS;
    }

    entry_ndx_dt_max = ( is_tcam_dt ) ? ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_id) : ARAD_PP_FP_DIRECT_TABLE_KAPS_NOF_LINES ;
    for (phase_ndx = 0; phase_ndx < 2; ++phase_ndx) 
    {
        for (entry_ndx_dt = 0; entry_ndx_dt < entry_ndx_dt_max; ++entry_ndx_dt)
        {
          if ((entry_ndx_dt & is_dt_bit_meaningful) == (entry_dt_first & is_dt_bit_meaningful))
          {
              /* In phase 0 - only check if entry is already
               * used - this is done to avoid collision
               */
            if (is_tcam_dt) {
              if (phase_ndx == 0) {
                  res = arad_sw_db_fp_db_entry_bitmap_get(
                          unit, 
                          stage, 
                          bank_id, 
                          entry_ndx_dt,
                          &is_used
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 47, exit);        
                  /* If one entry is used - break */
                  if(is_used) 
                  {
                      *success = SOC_SAND_FAILURE_REMOVE_ENTRY_FIRST;
                      phase_ndx = 1;
                      break;
                  }
              }
              else {

                    res = arad_tcam_db_direct_tbl_entry_set_unsafe(
                            unit,
                            ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
                            bank_id, 
                            entry_ndx_dt,
                            TRUE, /* valid entry */
                            action_bitmap_ndx,          
                            &action
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
                }
            }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
                else if(SOC_IS_JERICHO(unit))
                {
                    uint8   data_bytes[JER_KAPS_DMA_BUFFER_NOF_BYTES];
                    ARAD_PMF_DB_INFO pmf_db_info;
                    uint32 dma_offset;

                    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
                            unit,
                            stage,
                            db_id_ndx,
                            &pmf_db_info
                          );
                    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit);

                    dma_offset =  ( pmf_db_info.kaps_db_id << ARAD_PP_FP_DIRECT_TABLE_KAPS_KEY_LENGTH) + entry_ndx_dt ; 
                    res = jer_pp_kaps_dma_buffer_to_kaps_payload_buffer_encode(unit, action.value, buffer_size, data_bytes);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 32, exit);

                    res = jer_pp_kaps_dma_entry_add( unit,
                                                     dma_offset,
                                                     data_bytes,
                                                     buffer_size,
                                                     success );

                    SOC_SAND_CHECK_FUNC_RESULT(res, 36, exit);

                    /* if bank allocation failed */
                    if(SOC_SAND_SUCCESS != *success)
                    {
                        ARAD_DO_NOTHING_AND_EXIT;
                    }
                }
#endif /* BCM_88675_SUPPURT */
            if (is_tcam_dt) {
                /* update entry is used in bitmap */
                res = arad_sw_db_fp_db_entry_bitmap_set(
                   unit, 
                   stage, 
                   bank_id, 
                   entry_ndx_dt,
                   TRUE);
                SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
            }
              
          }
        }
    }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_table_entry_add_unsafe()", db_id_ndx, entry_id_ndx);
}


uint32 
    arad_pp_fp_ivl_entry_add(int unit, SOC_PPC_FP_DATABASE_STAGE stage, uint32 db_id_ndx, uint32 entry_id_ndx, const  SOC_PPC_FP_ENTRY_INFO *info, uint32 result[ARAD_TCAM_ACTION_MAX_LEN], SOC_SAND_SUCCESS_FAILURE *success)
{
    uint32 res = 0;
    ARAD_PP_LEM_ACCESS_PAYLOAD lem_payload;
    uint32 field_val = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&lem_payload);
        
    /* this is a W.A for the IVL */
    lem_payload.dest = result[0] & 0x7ffff; /*19 bit of the destination */
    lem_payload.asd = result[1] & 0x1fffff; /* 24 bit of the asd */
    /* getting the is_dynamic and sa_drop fields, the 19 bit offset change is because we are counting only from the asd,
       the 19 bit of the destination is in param0*/
    soc_sand_bitstream_get_any_field(&(result[1]), SOC_DPP_DEFS_GET(unit, lem_is_dynamic_lsb) - 19, 1, &field_val);
    lem_payload.is_dynamic = field_val;
    soc_sand_bitstream_get_any_field(&(result[1]), SOC_DPP_DEFS_GET(unit, lem_sa_drop_lsb) - 19, 1, &field_val);
    lem_payload.sa_drop = field_val;

    lem_payload.has_cw = (result[1] >> 21) & 0x1;
    lem_payload.tpid_profile = (result[1] >> 20) & 0x1;
    lem_payload.is_learn_data = 0x0;      /* No. The meaning is the learned payload is from the in-lif table of the in- lif*/
    lem_payload.flags = ARAD_PP_FWD_DECISION_PARSE_OUTLIF;
    lem_payload.age = 0;
    res = arad_pp_dbal_entry_add(unit, db_id_ndx, (SOC_PPC_FP_QUAL_VAL*)(info->qual_vals), info->priority, (void *)(&lem_payload), success);
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_dbal_entry_add()", db_id_ndx, entry_id_ndx);
}


#if defined(INCLUDE_KBP) && !defined(BCM_88030)
/* this is a temparary function that add entries direct to the KBP, when those databases will be implemented in the DBAL this function can be deleted. */
uint32 
    arad_pp_fp_extended_database_entry_add(int unit, SOC_PPC_FP_DATABASE_STAGE stage, uint32 db_id_ndx, uint32 entry_id_ndx, const  SOC_PPC_FP_ENTRY_INFO *info, uint32 result[ARAD_TCAM_ACTION_MAX_LEN], SOC_SAND_SUCCESS_FAILURE *success)
{
    uint32 res = 0;
    uint8   elk_data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
    uint8   elk_mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
    uint8   elk_ad_value[SOC_DPP_TCAM_ACTION_ELK_KBP_MAX_LEN_BYTES] = {0};
    uint32 logical_entry_size_in_bytes, table_payload_in_bytes;
    ARAD_KBP_FRWRD_IP_TBL_ID   frwrd_table_id;
    uint32 value[ARAD_TCAM_ENTRY_MAX_LEN] = {0};
    uint32 mask[ARAD_TCAM_ENTRY_MAX_LEN] = {0};
    SOC_PPC_FP_DATABASE_INFO  fp_database_info;    

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);    

    /* Get the database Info */
    res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    frwrd_table_id = fp_database_info.internal_table_id; /* the table ID is updated in the BCM layer according to the table in the KBP*/
    
    /* map key qualifiers to tcam buffer */
    res = arad_pp_fp_key_value_to_buffer(unit, stage, info->qual_vals, db_id_ndx, value, mask);
    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);    

    res = arad_kbp_table_size_get(unit, frwrd_table_id, &logical_entry_size_in_bytes, &table_payload_in_bytes); /* For ACL, entry-size = table-size */
    SOC_SAND_CHECK_FUNC_RESULT(res,  71, exit);

    res = arad_pp_tcam_route_buffer_to_kbp_buffer_encode(
            unit,
            logical_entry_size_in_bytes,
            value,
            mask,
            elk_data,
            elk_mask
          );
    SOC_SAND_CHECK_FUNC_RESULT(res,  73, exit);

    res = arad_pp_tcam_route_kbp_payload_buffer_encode(
            unit,
            table_payload_in_bytes,
            result, /* limited to 96b */
            elk_ad_value);
    SOC_SAND_CHECK_FUNC_RESULT(res,  76, exit);


    if ((frwrd_table_id == ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6) || (frwrd_table_id == ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P) ||
		(frwrd_table_id == ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING) ) { /* add entry to kbp LPM database */
          uint32 prefix_len = 0;

          res = arad_pp_frwrd_ip_tcam_lpm_prefix_len_get(elk_mask, logical_entry_size_in_bytes, &prefix_len);
          SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

          res = arad_pp_tcam_kbp_lpm_route_add(
                  unit,
                  frwrd_table_id,
                  entry_id_ndx,
                  prefix_len,
                  elk_data,
                  elk_ad_value,
                  success
                );

          SOC_SAND_CHECK_FUNC_RESULT(res, 00, exit);
    } else {/* add entry to kbp TCAM database */

        res = arad_pp_tcam_kbp_route_add(
                unit,
                frwrd_table_id,
                entry_id_ndx,
                info->is_for_update,
                info->priority,
                elk_data,
                elk_mask,
                elk_ad_value,
                success
              );
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_extended_database_entry_add()", db_id_ndx, entry_id_ndx);
}


/* 
   this function used for ACL entry add to KBP.
   if we used DIP SIP sharing we need to add the shared qualifiers value to the begining of the entry according to the cluster order
*/
uint32
  arad_pp_fp_dip_sip_sharing_handle(
     SOC_SAND_IN int unit, 
     SOC_SAND_IN uint32 db_id_ndx, 
     SOC_SAND_IN SOC_PPC_FP_ENTRY_INFO* info, 
     uint8*   elk_data, 
     uint8*   elk_mask) 
{

  uint32 res = SOC_SAND_OK, fld_val;
  SOC_PPC_FP_DATABASE_INFO  fp_database_info;
  uint32 is_enabled, exist_progs[1], prog_result = 0;
  uint8   data[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
  uint8   mask[ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES] = {0};
  uint32  buffer_data[1] = {0};
  uint32  buffer_mask[1] = {0};
  uint32  buffer_data_part2[1] = {0};
  uint32  buffer_mask_part2[1] = {0};


  int i,j, offset = 0, num_of_shared_quals = 0, byte_ndx, cluster_id, handle_64_bit_data = 0;
  int qual_pos = -1;
  ARAD_PP_FP_SHARED_QUAL_INFO shard_qualifiers_in_qset[MAX_NOF_SHARED_QUALIFIERS_PER_PROGRAM];

  SOC_PPC_FP_DATABASE_STAGE stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* if this DB can use DIP DIP sharing (all is programs are part of the same cluster) */
  arad_pp_fp_dip_sip_sharing_is_sharing_enabled_for_db(unit, stage, db_id_ndx, &is_enabled, &cluster_id);

  if (!is_enabled) {
    goto exit;
  }

  /* get relevant programs for the DB */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(unit,stage,db_id_ndx,0,&exist_progs[0]);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  ARAD_PP_FP_KEY_FIRST_SET_BIT(exist_progs, prog_result, ARAD_PMF_LOW_LEVEL_NOF_PROGS, ARAD_PMF_LOW_LEVEL_NOF_PROGS, FALSE, res);
    
  for (i = 0; i < dip_sip_sharing_cluster[cluster_id].nof_qualifiers; i++) {     
   for (j = 0; j < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; j++) {       
     if (fp_database_info.qual_types[j] == dip_sip_sharing_cluster[cluster_id].shared_quals[i].qual_type) {
       shard_qualifiers_in_qset[num_of_shared_quals].qual_type = dip_sip_sharing_cluster[cluster_id].shared_quals[i].qual_type;
       shard_qualifiers_in_qset[num_of_shared_quals].size = dip_sip_sharing_cluster[cluster_id].shared_quals[i].size;
       num_of_shared_quals++;
     }
     if (fp_database_info.qual_types[j] == BCM_FIELD_ENTRY_INVALID) {
       break;
     }
   }
  }

  if (!num_of_shared_quals) {
   goto exit;
  }

  for (i = 0; i < num_of_shared_quals; i++) {      
    for (j = 0; j < ARAD_TCAM_ENTRY_MAX_LEN; j++) {     
      if (info->qual_vals[j].type == shard_qualifiers_in_qset[i].qual_type) {
        qual_pos = j;          
        break;
      }
    }

    if(shard_qualifiers_in_qset[i].size > 4){
      handle_64_bit_data = 1;
    }

    if (qual_pos != -1) {

      if (handle_64_bit_data) {
        buffer_data[0] = info->qual_vals[qual_pos].val.arr[1];        
        buffer_mask[0] = info->qual_vals[qual_pos].is_valid.arr[1];
        buffer_data_part2[0] = info->qual_vals[qual_pos].val.arr[0];
        buffer_mask_part2[0] = info->qual_vals[qual_pos].is_valid.arr[0];

      }else{
        buffer_data[0] = info->qual_vals[qual_pos].val.arr[0];        
        buffer_mask[0] = info->qual_vals[qual_pos].is_valid.arr[0];
        buffer_data_part2[0] = info->qual_vals[qual_pos].val.arr[1];/* not used */
        buffer_mask_part2[0] = info->qual_vals[qual_pos].is_valid.arr[1];/* not used */
      }
    } else { /* qualifier not found in qset, we need to add zeros.. */
      buffer_data[0] = 0;
      buffer_mask[0] = 0;
      buffer_data_part2[0] = 0;
      buffer_mask_part2[0] = 0;
    }
    for (byte_ndx = 0; byte_ndx < 4; byte_ndx++)
    {        
        /* Build data */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, buffer_data, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        data[(3 - byte_ndx) + offset] = (uint8) (fld_val & 0xFF);
        /* Build mask */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, buffer_mask, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        fld_val = (~fld_val); /* Inverse of PPD convention here for mask: 0 - care, 1 - don't care */
        mask[(3 - byte_ndx) + offset] = (uint8) (fld_val & 0xFF);
    }

    if (handle_64_bit_data) {

      for (byte_ndx = 0; byte_ndx < 4; byte_ndx++)
      {        
        /* Build data */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, buffer_data_part2, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        data[(3 - byte_ndx) + offset + 4] = (uint8) (fld_val & 0xFF);
        /* Build mask */
        fld_val = 0;
        SHR_BITCOPY_RANGE(&fld_val, 0, buffer_mask_part2, (SOC_SAND_NOF_BITS_IN_BYTE * byte_ndx), SOC_SAND_NOF_BITS_IN_BYTE);
        fld_val = (~fld_val); /* Inverse of PPD convention here for mask: 0 - care, 1 - don't care */
        mask[(3 - byte_ndx) + offset + 4] = (uint8) (fld_val & 0xFF);
      }
    }

    offset += shard_qualifiers_in_qset[i].size; 
    handle_64_bit_data = 0;
    qual_pos = -1;
  }
           
  for (i= 0; i < (ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES - (offset)); i++) {

    data[(offset) + i] = elk_data[i];
    mask[(offset) + i] = elk_mask[i];
  }

  for (i = 0; i < ARAD_PP_FRWRD_IP_ELK_FWD_MAX_KEY_LENGTH_IN_BYTES; i++) {

    elk_data[i] = data[i];
    elk_mask[i] = mask[i];
  }
   
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_dip_sip_sharing_handle()", db_id_ndx, 0);

}
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

/*********************************************************************
*     Add an entry to the Database. The database entry is
 *     selected if the entire relevant packet field values are
 *     matched to the database entry qualifiers values. When
 *     the packet is qualified to several entries, the entry
 *     with the strongest priority is chosen.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_entry_add_unsafe(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     db_id_ndx,
    SOC_SAND_IN  uint32                     entry_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO      *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE   *success
  )
{
    uint32 res = SOC_SAND_OK, tcam_db_id;
    ARAD_FP_ENTRY             fp_entry;
    ARAD_TCAM_ENTRY           entry;
    ARAD_TCAM_ACTION          action;
    SOC_PPC_FP_DATABASE_INFO  fp_database_info;
    SOC_PPC_FP_DATABASE_STAGE    stage;

    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_ADD_UNSAFE);

    SOC_SAND_CHECK_NULL_INPUT(info);
    SOC_SAND_CHECK_NULL_INPUT(success);

    ARAD_TCAM_ENTRY_clear(&entry);
    ARAD_TCAM_ACTION_clear(&action);
    SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);

    /* Get the database Info */
    res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Get the entry stage */
    res = arad_pp_fp_db_stage_info_get(unit, &fp_database_info, &stage);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    if (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) {
        res = arad_pp_fp_direct_table_entry_add_unsafe(unit, db_id_ndx, entry_id_ndx, info, stage, success);
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
    } else {
        /* map action to tcam action buffer */
        res = arad_pp_fp_action_value_to_buffer(unit, info->actions, db_id_ndx, action.value, NULL);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        
        if ((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES)) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
            res = arad_pp_fp_extended_database_entry_add(unit, stage, db_id_ndx, entry_id_ndx, info, action.value, success);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
#endif
        } else {
            if ((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL)) {
               if ( (fp_database_info.internal_table_id == SOC_DPP_DBAL_SW_TABLE_ID_IVL_LEARN_LEM) ||
                   (fp_database_info.internal_table_id == SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_LEM) || 
                   (fp_database_info.internal_table_id == SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_OUTER_LEARN_LEM) ||
                   (fp_database_info.internal_table_id == SOC_DPP_DBAL_SW_TABLE_ID_IVL_FWD_OUTER_FWD_LEM) ||
                   (fp_database_info.internal_table_id == SOC_DPP_DBAL_SW_TABLE_ID_IVL_INNER_LEARN_LEM) ||
                   (fp_database_info.internal_table_id == SOC_DPP_DBAL_SW_TABLE_ID_IVL_INNER_FWD_LEM)) { 
                      res = arad_pp_fp_ivl_entry_add(unit, stage, fp_database_info.internal_table_id, entry_id_ndx, info, action.value, success);
                      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
              }else{
                if ((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_HANDLE_ENTRIES_BY_KEY)) {			
                      res = arad_pp_dbal_entry_add(unit, fp_database_info.internal_table_id, (SOC_PPC_FP_QUAL_VAL*)(info->qual_vals), info->priority, (void *)(action.value), success);                    
                      SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);
                } else {
                    res = arad_pp_dbal_entry_add_id(unit, fp_database_info.internal_table_id, entry_id_ndx, (SOC_PPC_FP_QUAL_VAL*)(info->qual_vals), info->priority, &action, info->is_for_update, success);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
                }
              }
            } else { /* ACL datbase that was created by FP API on the fly */

                /* map key qualifiers to tcam buffer */
                res = arad_pp_fp_key_value_to_buffer(unit, stage, info->qual_vals, db_id_ndx, entry.value, entry.mask);
                SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

                if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)                    
                    res = arad_pp_tcam_kbp_tcam_entry_add(unit, (ARAD_KBP_ACL_TABLE_ID_OFFSET + db_id_ndx), entry_id_ndx, info->is_for_update,
                                                                   info->priority, entry.value, entry.mask, action.value, info,  success);                    
                    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
#else /* INCLUDE_KBP */      

                    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NOT_LOOKUP_ERR, 45, exit); /* No use for FLP stage with ACL database without KBP */
#endif
                } else { 

                  /* Add entry to internal TCAM (ACL) */                    

                    entry.valid = (info->is_invalid == TRUE) ? FALSE : TRUE;                    
                    entry.is_for_update = info->is_for_update;
                    entry.is_inserted_top = info->is_inserted_top;/* Indicate if it is for top of bank or middle */

                    tcam_db_id = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx);

                    res = arad_tcam_managed_db_entry_add_unsafe(
                       unit,
                       tcam_db_id,
                       entry_id_ndx,
                       (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SINGLE_BANK)? 1:0, 
                       info->priority,
                       &entry,
                       &action,
                       success
                       );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
                }
            }
        }
    }

    if ((*success == SOC_SAND_SUCCESS) && (!entry.is_for_update)) {
        /* Update the nof-entries per Database if not TCAM. In TCAM, a verification is done in lower-level */
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(unit, stage, db_id_ndx, &fp_entry);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 100, exit);

        fp_entry.nof_db_entries++;

        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.set(unit, stage, db_id_ndx, &fp_entry);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 110, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_add_unsafe()", db_id_ndx, entry_id_ndx);
}

uint32
  arad_pp_fp_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO                       *info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info;
  SOC_PPC_FP_ACTION_TYPE
    fp_action_type[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  uint32
    action_type_ndx;
  uint8
    is_found;
  SOC_PPC_FP_ENTRY_INFO
    entry_info;
  SOC_PPC_FP_DATABASE_STAGE
      stage;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_ADD_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id_ndx, ARAD_PP_FP_ENTRY_ID_NDX_MAX, ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR, 20, exit);
  res = SOC_PPC_FP_ENTRY_INFO_verify(unit, info, stage);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  /*
   *  Verify the Database exists and is of type TCAM or direct table
   */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  if ((fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_TCAM)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_FLP)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_DIRECT_TABLE)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_EGRESS))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NOT_LOOKUP_ERR, 45, exit);
  }

  /*
   * Verify the entry validity (vs the Database)
   */
  for (action_type_ndx = 0; action_type_ndx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++action_type_ndx)
  {
    fp_action_type[action_type_ndx] = info->actions[action_type_ndx].type;
  }
  res = arad_pp_fp_entry_validity_get(
          unit,
          &fp_database_info,
          info->qual_vals,
          fp_action_type
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

  /*
   * Verify the entry does not already exist 
   * Take from info for direct table
   */

  if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
      if ((fp_database_info.internal_table_id == ARAD_KBP_FRWRD_TBL_ID_EXTENDED_IPV6) || 
          (fp_database_info.internal_table_id == ARAD_KBP_FRWRD_TBL_ID_EXTENDED_P2P) ||
          (fp_database_info.internal_table_id == ARAD_KBP_FRWRD_TBL_ID_INRIF_MAPPING) ) {
          /* in this case we don't need to search for the entry based on the entry ID only by key.
             key verification is done in the KBP level */
          goto exit;
      }
#endif
  }

  sal_memcpy(&entry_info, info, sizeof(SOC_PPC_FP_ENTRY_INFO)); 
  res = arad_pp_fp_entry_get_unsafe(
          unit,
          db_id_ndx,
          entry_id_ndx,
          &is_found,
          &entry_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

  if ((is_found && (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_HANDLE_ENTRIES_BY_KEY))) {
      /* EXACT match databases support entry update action by default */
      return SOC_SAND_OK;
  } else if ((is_found && !(info->is_for_update))
             || ((!is_found) && info->is_for_update)) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ENTRY_ALREADY_EXIST_ERR, 65, exit);
  }

  /* 
   * No native support of invalid entry in KBP 
   */ 
  if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
      if (info->is_invalid == TRUE) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ENTRY_ALREADY_EXIST_ERR, 75, exit);
      }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_add_verify()", db_id_ndx, entry_id_ndx);
}


STATIC 
 uint32
  arad_pp_fp_entry_db_id_key_type_get(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE                 stage,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_OUT uint32                                 *tcam_db_id,
    SOC_SAND_OUT ARAD_IP_TCAM_ENTRY_TYPE                *tcam_key_type
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  *tcam_key_type = ARAD_IP_NOF_TCAM_ENTRY_TYPES;
  *tcam_db_id = ARAD_TCAM_MAX_NOF_LISTS;

    if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
        *tcam_key_type = ARAD_IP_TCAM_ENTRY_TYPE_KBP_FIRST_ACL + db_id_ndx; /* Use NOF for the ACL table id */
        *tcam_db_id = ARAD_TCAM_MAX_NOF_LISTS-1;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
    }
    else 
    {
        *tcam_db_id = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx);
        *tcam_key_type = ARAD_IP_NOF_TCAM_ENTRY_TYPES;
    }

    ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_db_id_key_type_get()", stage, 0);
}

STATIC uint32
  arad_pp_fp_direct_table_entry_get(int unit, SOC_PPC_FP_DATABASE_STAGE stage, uint32 db_id_ndx, uint32 entry_id_ndx, SOC_PPC_FP_ENTRY_INFO* info, uint8* is_found, uint32 value_in[ARAD_PP_FP_TCAM_ENTRY_SIZE], uint32 mask_in[ARAD_PP_FP_TCAM_ENTRY_SIZE], ARAD_TCAM_ACTION* action)
{
  uint32
     res = SOC_SAND_OK,
     entry_ndx_dt,
     is_dt_bit_meaningful = 0,
     entry_dt_first,
     bank_id,
     bank_ndx;

  uint8
     is_used;

  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx;
  ARAD_FP_ENTRY             fp_entry;
  ARAD_PP_FP_LOCATION_HASH  dt_key;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  /* Special case: no entry in DB  - stop here */
  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(unit, stage, db_id_ndx, &fp_entry);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 13, exit);
  if (fp_entry.nof_db_entries == 0) {
    *is_found = FALSE;
    ARAD_DO_NOTHING_AND_EXIT;
  }

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), &action_bitmap_ndx);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);

  /* Get the entry's key value and mask from the entry_id -> location hash table */
  res = arad_tcam_db_entry_id_to_location_entry_get(
     unit,
     ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
     entry_id_ndx,
     FALSE,
     &(dt_key.location),
     is_found
     );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (*is_found == FALSE) {
    ARAD_DO_NOTHING_AND_EXIT;
  }

  entry_dt_first = dt_key.entry_dt_key.value;
  is_dt_bit_meaningful = dt_key.entry_dt_key.mask;

  SHR_BITCOPY_RANGE(value_in, 0, &entry_dt_first, 0, SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits));
  SHR_BITCOPY_RANGE(mask_in, 0, &is_dt_bit_meaningful, 0, SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits));

  /* Find which bank this database uses */
  for (bank_ndx = 0; bank_ndx < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_ndx++) {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), bank_ndx, &is_used);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
    if (is_used) {
      break;
    }
  }

  if (bank_ndx >= SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit)) {
    SOC_SAND_SET_ERROR_CODE(SOC_PPC_FP_NOF_DBS_PER_BANK_OUT_OF_RANGE_ERR, 25, exit);
  }

  bank_id = bank_ndx;

  for (entry_ndx_dt = 0; entry_ndx_dt < ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_id); ++entry_ndx_dt) {
    if ((entry_ndx_dt & is_dt_bit_meaningful) == (entry_dt_first & is_dt_bit_meaningful)) {
      res = arad_tcam_db_direct_tbl_entry_get_unsafe(
         unit,
         ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
         bank_id,
         entry_ndx_dt,
         action_bitmap_ndx,
         action,
         is_found
         );
      SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

      if (*is_found == TRUE) {
        /* at least one entry was found */
        break;
      }
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_table_entry_get()", db_id_ndx, entry_id_ndx);
}

/*********************************************************************
*     Get an entry from the Database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_entry_get_unsafe(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  uint32                   db_id_ndx,
    SOC_SAND_IN  uint32                   entry_id_ndx,
    SOC_SAND_OUT uint8                    *is_found,
    SOC_SAND_INOUT SOC_PPC_FP_ENTRY_INFO  *info
  )
{
  uint32
     res = SOC_SAND_OK,
     tcam_db_id,
     priority;

  uint8
     hit_bit,
     found;

  uint32
     value_in[ARAD_PP_FP_TCAM_ENTRY_SIZE],
     mask_in[ARAD_PP_FP_TCAM_ENTRY_SIZE];

  SOC_PPC_FP_DATABASE_INFO fp_database_info;
  ARAD_TCAM_ENTRY  entry;
  ARAD_TCAM_ACTION action;
  SOC_PPC_FP_DATABASE_STAGE stage;
  ARAD_IP_TCAM_ENTRY_TYPE tcam_key_type;
  
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  uint8 is_fp_db_extended_fwrd = FALSE;
#endif  

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(is_found);
  SOC_SAND_CHECK_NULL_INPUT(info);

  ARAD_TCAM_ACTION_clear(&action);
  sal_memset(value_in, 0, ARAD_PP_FP_TCAM_ENTRY_SIZE * sizeof(uint32));
  sal_memset(mask_in, 0, ARAD_PP_FP_TCAM_ENTRY_SIZE * sizeof(uint32));  

  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);

  *is_found = FALSE;

  res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) {
    is_fp_db_extended_fwrd = TRUE;
  }
#endif

  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get(unit, db_id_ndx, &stage );
  SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);

  /* Different behavior for Direct Table database */
  if (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) {

    res = arad_pp_fp_direct_table_entry_get(unit, stage, db_id_ndx, entry_id_ndx, info, is_found, value_in, mask_in, &action);
    SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);

  } else if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL) {

    if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_HANDLE_ENTRIES_BY_KEY) {      
      res = arad_pp_dbal_entry_get(unit, fp_database_info.internal_table_id, (info->qual_vals), (void *)(action.value), &(info->priority), &hit_bit, is_found);      
    } else{      
      res = arad_pp_dbal_entry_get_id(unit, fp_database_info.internal_table_id, entry_id_ndx, (info->qual_vals), (void *)(&action), &(info->priority), &hit_bit, is_found);
    }
    SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);    

  }else {
    SOC_PPC_FP_ENTRY_INFO_clear(info);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
    if ((is_fp_db_extended_fwrd) ||(stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP)) {

      if (is_fp_db_extended_fwrd)
      {
          tcam_db_id = fp_database_info.internal_table_id; /* the table ID is updated in the BCM layer according to the table in the KBP*/
          tcam_key_type = tcam_db_id;
      } 
      else
      {
        tcam_key_type = ARAD_KBP_ACL_TABLE_ID_OFFSET + db_id_ndx;
      }
	  
	  res = arad_pp_tcam_kbp_tcam_entry_get(
         unit, 
         tcam_key_type, 
         entry_id_ndx,
         entry.value,
         entry.mask,
         action.value,
         &priority,
		 &found,
		 &hit_bit
         );
      SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    } else
#endif
	{
		res = arad_pp_fp_entry_db_id_key_type_get(
		   unit,
		   stage,
		   db_id_ndx,
		   &tcam_db_id,
		   &tcam_key_type
		   );
		SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);

		res = arad_tcam_db_entry_get_unsafe(
		   unit,
		   tcam_db_id,
		   entry_id_ndx,
		   TRUE, /* hit_bit_clear */
		   &priority,
		   &entry,
		   &action,
		   &found,
		   &hit_bit
		   );
		SOC_SAND_CHECK_FUNC_RESULT(res, 12, exit);
	}

    if (!found) {
      *is_found = 0;
      goto exit;
    }
    *is_found = 1;

    sal_memcpy(value_in, entry.value, ARAD_PP_FP_TCAM_ENTRY_SIZE * sizeof(uint32));
    sal_memcpy(mask_in, entry.mask, ARAD_PP_FP_TCAM_ENTRY_SIZE * sizeof(uint32));
    /* 
     * Retrieve the entry info  
     */
    /* Entry priority */
    info->priority = priority;

    /* Entry validity */
    info->is_invalid = (entry.valid) ? FALSE : TRUE;
  }

  /* Key: map from tcam buffer to key qualifiers */
    if (!(fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL)) {
      res = arad_pp_fp_key_buffer_to_value(unit, stage, db_id_ndx, value_in, mask_in, info->qual_vals);
      SOC_SAND_CHECK_FUNC_RESULT(res, 29, exit);
    }

  /* Entry actions */
  res = arad_pp_fp_action_buffer_to_value(unit, db_id_ndx, action.value, info->actions);
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_get_unsafe()", db_id_ndx, entry_id_ndx);
}

uint32
  arad_pp_fp_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx
  )
{
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info;
  uint32
    res = SOC_SAND_OK;
 
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id_ndx, ARAD_PP_FP_ENTRY_ID_NDX_MAX, ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR, 20, exit);

  /*
   * Verify the Database is of type TCAM or direct table
   */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

  if ((fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_TCAM)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_FLP)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_DIRECT_TABLE)
        && (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_EGRESS))
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NOT_LOOKUP_ERR, 40, exit);
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_get_verify()", db_id_ndx, entry_id_ndx);
}


STATIC uint32
  arad_pp_fp_direct_table_entry_remove(int unit, SOC_PPC_FP_DATABASE_STAGE stage, uint32 db_id_ndx, uint32 entry_id_ndx, SOC_PPC_FP_ENTRY_INFO* entry_info, uint8* found)
{
  uint32
     res = SOC_SAND_OK,
     entry_ndx_dt,
     is_dt_bit_meaningful = 0,
     entry_dt_first,entry_ndx_dt_max,
     bank_id,
     buffer_size;
  uint8 is_tcam_dt;
  ARAD_TCAM_ACTION          action;
  ARAD_TCAM_ACTION_SIZE     action_bitmap_ndx;
  SOC_PPC_FP_DATABASE_INFO  fp_database_info;    

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);    

  /* Get the database Info */
  res = arad_pp_fp_database_get_unsafe(unit, db_id_ndx, &fp_database_info);
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  is_tcam_dt = !( fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS ) ? TRUE : FALSE ;

  ARAD_TCAM_ACTION_clear(&action);

  res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), &action_bitmap_ndx);
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_fp_action_value_to_buffer(unit, entry_info->actions, db_id_ndx, action.value, &buffer_size);
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_fp_entry_ndx_direct_table_get(
     unit,
     db_id_ndx,
     entry_info,
     &bank_id,
     &entry_dt_first,
     &is_dt_bit_meaningful
     );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

  if ( is_tcam_dt ) {
      /*
       *  Remove the entry from the entry_id -> location hash table
       */
      res = arad_tcam_db_entry_id_to_location_entry_remove(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), entry_id_ndx );
      SOC_SAND_CHECK_FUNC_RESULT(res, 46, exit);
  }

  entry_ndx_dt_max = ( is_tcam_dt ) ? ARAD_TCAM_NOF_LINES_PER_BANK(unit, bank_id) : ARAD_PP_FP_DIRECT_TABLE_KAPS_NOF_LINES ;
  for (entry_ndx_dt = 0; entry_ndx_dt < entry_ndx_dt_max; ++entry_ndx_dt) {
    if ((entry_ndx_dt & is_dt_bit_meaningful) == (entry_dt_first & is_dt_bit_meaningful)) {
        if ( is_tcam_dt ) {
          res = arad_tcam_db_direct_tbl_entry_set_unsafe(
             unit,
             ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx),
             bank_id,
             entry_ndx_dt,
             FALSE, /* invalidate entry */
             action_bitmap_ndx,
             &action
             );
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

          /* Clear entry bit in bitmap */
          res = arad_sw_db_fp_db_entry_bitmap_set(unit, stage, bank_id, entry_ndx_dt, FALSE);
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
        }
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
        else if(SOC_IS_JERICHO(unit))
        {
                    uint8   data_bytes[JER_KAPS_AD_BUFFER_NOF_BYTES];
                    uint32 dma_offset =  entry_ndx_dt ; 
                    SOC_SAND_SUCCESS_FAILURE   success;
                    sal_memset(&data_bytes, 0x0, sizeof(uint8) * 4);

                    res = jer_pp_kaps_dma_entry_add( unit,
                                                     dma_offset,
                                                     data_bytes,
                                                     buffer_size,
                                                     &success );

                    SOC_SAND_CHECK_FUNC_RESULT(res, 36, exit);

                    /* if bank allocation failed */
                    if(SOC_SAND_SUCCESS != success)
                    {
                        ARAD_DO_NOTHING_AND_EXIT;
                    }

        }
#endif /* BCM_88675_SUPPURT */
    }
  }

  (*found) = TRUE;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_table_entry_remove()", db_id_ndx, entry_id_ndx);
}
/*********************************************************************
*     Remove an entry from the Database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_entry_remove_unsafe(
    SOC_SAND_IN  int    unit,
    SOC_SAND_IN  uint32 db_id_ndx,
    SOC_SAND_IN  uint32 entry_id_ndx,
    SOC_SAND_IN  uint32 is_sw_remove_only
  )
{
  uint32
     res = SOC_SAND_OK,
     tcam_db_id;

  uint8 found = FALSE;

  ARAD_FP_ENTRY             fp_entry;
  ARAD_TCAM_ENTRY           entry;
  SOC_PPC_FP_DATABASE_INFO  fp_database_info;
  SOC_SAND_SUCCESS_FAILURE  success;
  SOC_PPC_FP_DATABASE_STAGE    stage;
  SOC_PPC_FP_ENTRY_INFO     entry_info;
  ARAD_IP_TCAM_ENTRY_TYPE   tcam_key_type = 0;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_REMOVE_UNSAFE);
  
  ARAD_TCAM_ENTRY_clear(&entry);

  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get( unit, db_id_ndx, &stage );
  SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);

  /* Get FP database in order to get the DB type */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);

  res = arad_pp_fp_database_get_unsafe( unit, db_id_ndx, &fp_database_info );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_EXTENDED_DATABASES) {
    ARAD_KBP_FRWRD_IP_TBL_ID frwrd_table_id;
    frwrd_table_id = fp_database_info.internal_table_id; /* the table ID is updated in the BCM layer according to the table in the KBP*/
    tcam_key_type = ARAD_IP_TCAM_FROM_KBP_FRWRD_IP_TBL_ID(frwrd_table_id);
  }

  if (tcam_key_type) {
    if (ARAD_IP_TCAM_ENTRY_TYPE_IS_KBP(tcam_key_type)) {
        if (is_sw_remove_only) {
            ARAD_SW_KBP_HANDLE location;
             sal_memset(&location, 0x0, sizeof(ARAD_SW_KBP_HANDLE));
             res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.location_tbl.set(unit, entry_id_ndx, &location);             
        }else{
            res = arad_pp_tcam_kbp_route_remove(
                    unit,
                    ARAD_IP_TCAM_TO_KBP_FRWRD_IP_TBL_ID(tcam_key_type),
                    entry_id_ndx
                  );
          SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
        }
      found = TRUE;
    }        
  } else
#endif
  {
    if (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL) {
          res = arad_pp_dbal_entry_delete_id(unit, fp_database_info.internal_table_id, entry_id_ndx, &success);
          SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
          found = TRUE;
    } else {
      /* Get the entry info */
      SOC_PPC_FP_ENTRY_INFO_clear(&entry_info);
      res = arad_pp_fp_entry_get_unsafe( unit, db_id_ndx, entry_id_ndx, &found, &entry_info );
      SOC_SAND_CHECK_FUNC_RESULT(res, 45, exit);

      if (found) {
        if (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) {
          res = arad_pp_fp_direct_table_entry_remove(unit, stage, db_id_ndx, entry_id_ndx, &entry_info, &found);
          SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        } else {
            
              res = arad_pp_fp_entry_db_id_key_type_get(unit, stage, db_id_ndx, &tcam_db_id, &tcam_key_type );
              SOC_SAND_CHECK_FUNC_RESULT(res, 13, exit);
            
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
            if (ARAD_IP_TCAM_ENTRY_TYPE_IS_KBP(tcam_key_type)) {
                res = arad_pp_tcam_kbp_route_remove(
                          unit,
                          ARAD_IP_TCAM_TO_KBP_FRWRD_IP_TBL_ID(tcam_key_type),
                          entry_id_ndx
                        );
                SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);
            }
            else 
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
            {
                res = arad_tcam_managed_db_entry_remove_unsafe(
                        unit,
                        tcam_db_id,
                        entry_id_ndx
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 35, exit);
            }
        }
      }
    }
  }
  if (!found) {
    LOG_VERBOSE(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit: %d, Database id: %d, Entry id: %d - The entry to remove wasn't found\n\r"),
                 unit, db_id_ndx, entry_id_ndx));
    goto exit;
  } else {
    /*
     * Update the nof-entries per Database if not TCAM
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(unit, stage, db_id_ndx, &fp_entry);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);

    fp_entry.nof_db_entries--;

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.set(unit, stage, db_id_ndx, &fp_entry);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 70, exit);
  }

  /* In case of Direct Table remove bank if no entries */
  if ((fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE) && !( fp_database_info.flags &  SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS )) {
    if (0 == fp_entry.nof_db_entries) {
      /* allocate bank */
      res = arad_tcam_managed_db_direct_table_bank_remove(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx), &success );
      SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

      /* if bank allocation failed */
      if (SOC_SAND_SUCCESS != success) {
        SOC_SAND_SET_ERROR_CODE(SOC_PPC_FP_NOF_DBS_PER_BANK_OUT_OF_RANGE_ERR, 90, exit);
      }
    }
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_remove_unsafe()", db_id_ndx, entry_id_ndx);
}

uint32
  arad_pp_fp_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_ENTRY_REMOVE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id_ndx, ARAD_PP_FP_ENTRY_ID_NDX_MAX, ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_remove_verify()", db_id_ndx, entry_id_ndx);
}



uint32
  arad_pp_fp_entry_remove_by_key_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT SOC_PPC_FP_ENTRY_INFO                *info
  )
{
uint32
     res = SOC_SAND_OK;

  SOC_PPC_FP_DATABASE_INFO  fp_database_info;
  SOC_SAND_SUCCESS_FAILURE  success = SOC_SAND_FAILURE_UNKNOWN_ERR;
  SOC_PPC_FP_DATABASE_STAGE    stage;


  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  
  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get( unit, db_id_ndx, &stage );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* Get FP database in order to get the DB type */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);

  res = arad_pp_fp_database_get_unsafe( unit, db_id_ndx, &fp_database_info );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if ((fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_DBAL) && (fp_database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_HANDLE_ENTRIES_BY_KEY)) {    
        res = arad_pp_dbal_entry_delete(unit, fp_database_info.internal_table_id, info->qual_vals, &success);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    } else {
        res = -1;
    }

    if ((success != SOC_SAND_SUCCESS)) {
      res = -1;
    }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_remove_by_key_unsafe()", db_id_ndx, 0);
}

uint32
  arad_pp_fp_entry_remove_by_key_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT SOC_PPC_FP_ENTRY_INFO                *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);  

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_entry_remove_by_key_verify()", db_id_ndx, 0);
}



/*********************************************************************
*     Get the Database entries. The function returns list of
 *     entries that were added to a database with database ID
 *     'db_id_ndx'.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_database_entries_get_block_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range,
    SOC_SAND_OUT SOC_PPC_FP_ENTRY_INFO                       *entries,
    SOC_SAND_OUT uint32                                  *nof_entries
  )
{
  uint32
    nof_valid_entries = 0,
    res = SOC_SAND_OK;
  uint32
    entry_ndx;
  ARAD_FP_ENTRY
    fp_entry;
  uint8
    is_found;
  SOC_PPC_FP_DATABASE_STAGE
      stage;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_ENTRIES_GET_BLOCK_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(block_range);
  SOC_SAND_CHECK_NULL_INPUT(entries);
  SOC_SAND_CHECK_NULL_INPUT(nof_entries);

  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
          unit,
          stage,
          db_id_ndx,
          &fp_entry
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    for (entry_ndx = block_range->iter; entry_ndx < block_range->iter + block_range->entries_to_scan; ++entry_ndx)
    {
      if (nof_valid_entries >= block_range->entries_to_act)
      {
        /*
         *	No need to go further, maximal number of entries have been found
         */
        break;
      }
      SOC_PPC_FP_ENTRY_INFO_clear(&entries[nof_valid_entries]);
      res = arad_pp_fp_entry_get_unsafe(
              unit,
              db_id_ndx,
              entry_ndx,
              &is_found,
              &(entries[nof_valid_entries])
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      if (is_found == TRUE)
      {
        nof_valid_entries ++;
      }
    }

  *nof_entries = nof_valid_entries;
  block_range->iter = entry_ndx;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_entries_get_block_unsafe()", db_id_ndx, 0);
}

uint32
  arad_pp_fp_database_entries_get_block_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                    *block_range
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DATABASE_ENTRIES_GET_BLOCK_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_entries_get_block_verify()", db_id_ndx, 0);
}

/*********************************************************************
*     Add an entry to the Database. The database entry is
 *     selected if all the Packet Qualifier field values are in
 *     the Database entry range.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_direct_extraction_entry_add_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE                      *success
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_FEM_ENTRY
    fem_entry;
  SOC_PPC_FP_FEM_CYCLE
    fem_cycle;
  uint8
      is_large_direct_extraction,
      is_for_tm,
      is_default_tm,
      use_fes = FALSE,
      key_alloced,
    action_alloced,
    fem_pgm_id,
    fem_pgm_bmp;
  SOC_PPC_FP_DATABASE_INFO
    database_info;
  uint32
      fem_id_ndx,
      total_bits_in_zone[ARAD_PP_FP_KEY_NOF_ZONES],
      pmf_pgm_ndx,
      alloc_flags,
      key_alloc_flag,
      ce_index,
      exist_progs,
      pmf_pgm_ndx_min,
      pmf_pgm_ndx_max,
      total_ce_indx = 0,
      selected_cycle[ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES],
      place_start,
      qual_type_ndx,
    action_ndx;
  ARAD_FP_ENTRY
    fp_entry;
  SOC_PPC_FP_DATABASE_STAGE
      stage; 
  SOC_PPC_FP_QUAL_TYPE         
      qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX];
  ARAD_PP_FP_CE_CONSTRAINT     
      *ce_const = NULL;
  SOC_PPC_FP_ACTION_TYPE
    fp_action_type[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  uint32
    action_type_ndx;
  ARAD_PP_FEM_ACTIONS_CONSTRAINT
    action_const;
  ARAD_PP_FP_KEY_ALLOC_INFO    
      *alloc_info = NULL;
  ARAD_PMF_DB_INFO /* update db info */
        db_info; 
  ARAD_PMF_CE
        sw_db_ce;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_ADD_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);
  SOC_SAND_CHECK_NULL_INPUT(success);
  sal_memset(selected_cycle, 0x0, sizeof(uint32) * ARAD_PMF_LOW_LEVEL_NOF_PROGS_ALL_STAGES);

  SOC_PPC_FP_DATABASE_INFO_clear(&database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_fp_database_is_large_direct_extraction_get(
          unit,
          &database_info,
          &is_large_direct_extraction
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

  res = arad_pp_fp_database_is_tm_get(unit, &database_info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 43, exit);

  res = arad_pp_fp_direct_extraction_use_fes_get(
          unit,
          info,
          &use_fes
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
 
  alloc_flags = (is_for_tm) ? ARAD_PP_FP_FEM_ALLOC_FES_TM : 0 ;

  /* 
   * Complete different sequence if large DB or not: 
   * - for thin DBs, alloc a FEM and set it 
   * - for large DBs, alloc the key in this exact order, 
   * alloc a FES and set it (indicate if filter or not)
   */
  if ( (!is_large_direct_extraction) && (!use_fes)) {

      /*
       *    Find if there is a free FEM
       */
      SOC_PPC_FP_FEM_ENTRY_clear(&fem_entry);
      SOC_PPC_FP_FEM_CYCLE_clear(&fem_cycle);
      fem_entry.is_for_entry = TRUE;
      fem_entry.db_id = db_id_ndx;
      fem_entry.db_strength = database_info.strength;
      fem_entry.entry_id = entry_id_ndx;
      fem_entry.entry_strength = info->priority;
      for (action_ndx = 0; action_ndx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; action_ndx ++)
      {
        fem_entry.action_type[action_ndx] = info->actions[action_ndx].type;
        fem_entry.is_base_positive[action_ndx] = SOC_SAND_NUM2BOOL(info->actions[action_ndx].base_val);
      }

      fem_cycle.is_cycle_fixed = FALSE;

      /*
       * Set it
       */

      res = arad_pp_fp_fem_pgm_id_bmp_get(unit,db_id_ndx,&fem_pgm_bmp);
      SOC_SAND_CHECK_FUNC_RESULT(res, 17, exit);  

      for(fem_pgm_id = 0 ; fem_pgm_id < ARAD_PMF_NOF_FEM_PGMS; fem_pgm_id++)
      {
        if(SOC_SAND_GET_BIT(fem_pgm_bmp,fem_pgm_id) == TRUE)
        {
            res = arad_pp_fp_fem_insert_unsafe(
              unit,
              &fem_entry,
              &fem_cycle,
              alloc_flags,
              info,
              NULL,
              fem_pgm_id,
              &fem_id_ndx,
              success
            );
            SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        }
      }
  }
  else { /* Large Direct extraction */
      *success = SOC_SAND_SUCCESS;

      /* Set the qual type array */
      for (qual_type_ndx = 0; qual_type_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; qual_type_ndx++) {
          qual_types[qual_type_ndx] = (qual_type_ndx < SOC_PPC_FP_DIR_EXTR_MAX_NOF_FIELDS)? 
              info->actions[0].fld_ext[qual_type_ndx].type: BCM_FIELD_ENTRY_INVALID;
      }

      ARAD_ALLOC(ce_const, ARAD_PP_FP_CE_CONSTRAINT, ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_MAX_ALL_LEVELS, "arad_pp_fp_direct_extraction_entry_add_unsafe.ce_const");
      /* 
       * Verify the alloc the Key in the exact order
       */ 
      place_start = ARAD_PP_FP_KEY_CE_HIGH;
      key_alloc_flag = ( is_large_direct_extraction) ? ARAD_PP_FP_KEY_ALLOC_CE_USE_QUAL_ORDER : ARAD_PP_FP_KEY_ALLOC_CE_SINGLE_MAPPING ; 
      key_alloc_flag |= ( database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SINGLE_BANK) ? ARAD_PP_FP_KEY_ALLOC_USE_KEY_A : 0 ; 
      res = arad_pp_fp_key_alloc_constrain_calc(
                unit,
                stage,
                db_id_ndx,
                ARAD_PP_FP_KEY_ALLOC_CHECK_ONLY | key_alloc_flag,
                qual_types,
                ARAD_PP_FP_KEY_CYCLE_ODD,
                ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE,
                TRUE,/* is_for_direct_extraction */
                ce_const,
                &total_ce_indx,
                &place_start,
                selected_cycle,
                total_bits_in_zone,
                &key_alloced
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
      if(!key_alloced) {
        LOG_ERROR(BSL_LS_SOC_FP,
                  (BSL_META_U(unit,
                              "    "
                              "Key: fail to allocate Database %d with total_ce_indx %d, place_start %d\n\r"), 
                   db_id_ndx, total_ce_indx, place_start));
          *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
          ARAD_DO_NOTHING_AND_EXIT;
      }

      /* 
       * Allocate a FES
       */
      for (action_type_ndx = 0; action_type_ndx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++action_type_ndx)
      {
        fp_action_type[action_type_ndx] = info->actions[action_type_ndx].type;
      }
      ARAD_PP_FEM_ACTIONS_CONSTRAINT_clear(&action_const);
      action_const.action_size = ARAD_TCAM_ACTION_SIZE_SECOND_20_BITS; 

      res = arad_pp_fp_action_alloc(
              unit,
              db_id_ndx,
              ARAD_PP_FP_FEM_ALLOC_FES_CHECK_ONLY | alloc_flags,
              fp_action_type,
              database_info.strength,
              selected_cycle,
              &action_const,
              &action_alloced
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
      if(!action_alloced)
      {
        LOG_ERROR(BSL_LS_SOC_FP,
                  (BSL_META_U(unit,
                              "   Error in DE entry add: db-id: %d, stage %s, no success for Action allocation \n\r"),
                   db_id_ndx, SOC_PPC_FP_DATABASE_STAGE_to_string(stage)));
          *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
          ARAD_DO_NOTHING_AND_EXIT;
      }

      /* At this point, guarantee it can be inserted */
      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                unit,
                stage,
                is_for_tm /* is_for_tm */, 
                &pmf_pgm_ndx_min,
                &pmf_pgm_ndx_max
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);

      /* get relevant programs for the DB */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(unit, stage, db_id_ndx, 0, &exist_progs);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 64, exit);

      key_alloc_flag = ( database_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_SINGLE_BANK) ? ARAD_PP_FP_KEY_ALLOC_USE_KEY_A : 0 ; 
      ARAD_ALLOC(alloc_info, ARAD_PP_FP_KEY_ALLOC_INFO, 1, "arad_pp_fp_database_create_unsafe.alloc_info");
      for (pmf_pgm_ndx = pmf_pgm_ndx_min; pmf_pgm_ndx < pmf_pgm_ndx_max; ++pmf_pgm_ndx)
      {
          if (SOC_SAND_GET_BIT(exist_progs, pmf_pgm_ndx) != 0x1){
              /* PMF Program not used */
              continue;
          }
          if (is_large_direct_extraction) {
              /* Allocate the Key */
              res = arad_pp_fp_key_alloc_in_prog(
                        unit,
                        stage,
                        pmf_pgm_ndx,
                        db_id_ndx,
                        key_alloc_flag | ARAD_PP_FP_KEY_ALLOC_USE_CE_CONS, /* Exact CE repartition */
                        qual_types,
                        selected_cycle[pmf_pgm_ndx],
                        ARAD_PP_FP_KEY_CE_PLACE_ANY_NOT_DOUBLE,
                        place_start,
                        ce_const,
                        total_ce_indx,/* for each Qual which CE is allocated */
                        alloc_info,
                        &key_alloced
                     );
              SOC_SAND_CHECK_FUNC_RESULT(res, 250, exit);
              if(!key_alloced) {
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "    "
                  "Key: fail to allocate Database %d in program %d\n\r"), 
                       db_id_ndx, pmf_pgm_ndx));
                  *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                  ARAD_DO_NOTHING_AND_EXIT;
              }
              for(ce_index = 0; ce_index  < total_ce_indx; ++ce_index) {
                  if (ce_const[ce_index].size_cons != ARAD_PP_FP_KEY_CE_SIZE_16) {
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "    "
                      "Key: fail to find only 16b instructions for Database %d in program %d\n\r"), 
                           db_id_ndx, pmf_pgm_ndx));
                      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                      ARAD_DO_NOTHING_AND_EXIT;
                  }
                  if (ce_index && (ce_const[ce_index].lost_bits != 0)) {
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "    "
                      "Key: lost bits for instructions for Database %d in program %d\n\r"), 
                           db_id_ndx, pmf_pgm_ndx));
                      *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
                      ARAD_DO_NOTHING_AND_EXIT;
                  }
              }

              /* Allocate the Action */
              action_const.tcam_res_id[0] = alloc_info->key_id[0]; /* Which key */
              action_const.tcam_res_id[1] = (alloc_info->alloc_place == ARAD_PP_FP_KEY_CE_HIGH)? 80: 0; /* Which LSB bit */
              action_const.tcam_res_id[1] += alloc_info->act_ce_const[0].lost_bits; 
          } else {
              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
                      unit,
                      stage,
                      db_id_ndx,
                      &db_info
                    );
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 180, exit);

             res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(
                unit,
                stage,
                pmf_pgm_ndx,
                selected_cycle[pmf_pgm_ndx],
                0,
                &sw_db_ce
              );
             SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 200, exit);

              /* Allocate the Action */
              action_const.tcam_res_id[0] = db_info.used_key[pmf_pgm_ndx][0];
              action_const.tcam_res_id[1] = (db_info.alloc_place == ARAD_PP_FP_KEY_CE_HIGH)? 80: 0; /* Which LSB bit */
              action_const.tcam_res_id[1] += info->actions[0].fld_ext[0].fld_lsb + ce_const[0].lost_bits; /*Add the number of lost bits to action offset*/
          }

          action_const.cycle = selected_cycle[pmf_pgm_ndx];
          alloc_flags |= ARAD_PP_FP_FEM_ALLOC_FES_FROM_KEY;
          if (is_for_tm)  
          {
            alloc_flags |= ARAD_PP_FP_FEM_ALLOC_FES_TM;
          }
          if ((info->qual_vals[0].type != SOC_PPC_NOF_FP_QUAL_TYPES) && (info->qual_vals[0].type != BCM_FIELD_ENTRY_INVALID)) {
              alloc_flags |= ARAD_PP_FP_FEM_ALLOC_FES_KEY_IS_CONDITIONAL_VALID;
          }
          res = arad_pp_fp_action_alloc_in_prog_with_entry(
                    unit,
                    db_id_ndx,
                    pmf_pgm_ndx,
                    entry_id_ndx,
                    alloc_flags,
                    fp_action_type,
                    database_info.strength,
                    &action_const,
                    &action_alloced
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 260, exit);

          if(!action_alloced) {
            LOG_ERROR(BSL_LS_SOC_FP,
                      (BSL_META_U(unit,
                                  "    "
                                  "FES: fail to allocate for DB %d, program %d\n\r"),
                       db_id_ndx, pmf_pgm_ndx));
              *success = SOC_SAND_FAILURE_OUT_OF_RESOURCES;
              ARAD_DO_NOTHING_AND_EXIT;
          }
          
      } /* for (pmf_pgm_ndx = pmf_pgm_ndx_min; pmf_pgm_ndx < pmf_pgm_ndx_max; ++pmf_pgm_ndx) */
  } /* if (is_large_direct_extraction) */

  if (*success == SOC_SAND_SUCCESS) {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
          unit,
          stage,
          db_id_ndx,
          &fp_entry
        );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

      fp_entry.nof_db_entries ++;

      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.set(
          unit,
          stage,
          db_id_ndx,
          &fp_entry
        );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);
  }
exit:
  ARAD_FREE(alloc_info);
  ARAD_FREE(ce_const);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_entry_add_unsafe()", db_id_ndx, entry_id_ndx);
}

uint32
  arad_pp_fp_direct_extraction_entry_add_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx,
    SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info
  )
{
  uint32
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info;
  SOC_PPC_FP_ACTION_TYPE
    fp_action_type[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  uint32
    action_type_ndx;
  uint8
    is_large_direct_extraction,
    is_for_tm,
    is_default_tm,
    is_fes,
    is_found;
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO
    *dir_extr_entry_info = NULL;
  ARAD_FP_ENTRY
    fp_entry;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_ADD_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  /*
   * Verify the Database exists and is of type DIrect Extraction
   */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  if (fp_database_info.db_type != SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION)
  {
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NOT_DIRECT_EXTRACTION_ERR, 45, exit);
  }

  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id_ndx, ARAD_PP_FP_DE_ENTRY_ID_NDX_MAX, ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR, 20, exit);

  res = arad_pp_fp_database_is_large_direct_extraction_get(
          unit,
          &fp_database_info,
          &is_large_direct_extraction
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
  res = SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_verify(unit, info, SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, is_large_direct_extraction);
  SOC_SAND_CHECK_FUNC_RESULT(res, 24, exit);

  /* 
   * No need to add entries for TM DBs
   */ 
  res = arad_pp_fp_database_is_tm_get(unit, &fp_database_info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 47, exit);
  if ( is_for_tm && (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE) ==0 )  &&
   ((soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0))==0)) {
      SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NOT_DIRECT_EXTRACTION_ERR, 49, exit);
  }


  /*
   * Verify the action types and qualifiers types exist in the Database
   */
  for (action_type_ndx = 0; action_type_ndx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++action_type_ndx)
  {
    fp_action_type[action_type_ndx] = info->actions[action_type_ndx].type;
  }
  res = arad_pp_fp_entry_validity_get(
          unit,
          &fp_database_info,
          info->qual_vals,
          fp_action_type
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 52, exit);

  /*
   *    Verify it is the entry already exists
   */
  if (!is_large_direct_extraction) {
      ARAD_ALLOC(dir_extr_entry_info, SOC_PPC_FP_DIR_EXTR_ENTRY_INFO, 1, "arad_pp_fp_direct_extraction_entry_add_verify.dir_extr_entry_info");
      SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_clear(dir_extr_entry_info);
      res = arad_pp_fp_direct_extraction_entry_get_unsafe(
              unit,
              db_id_ndx,
              entry_id_ndx,
              &is_found,
              &is_fes,
              dir_extr_entry_info
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      if (is_found == TRUE)
      {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_ENTRY_ALREADY_EXIST_ERR, 65, exit);
      }
  }
  else {
      /* Single entry maximum allowed for these large DBs */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
              unit,
              SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
              db_id_ndx,
              &fp_entry
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
      if (fp_entry.nof_db_entries) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 70, exit);
      }
  }

exit:
  ARAD_FREE(dir_extr_entry_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_entry_add_verify()", db_id_ndx, entry_id_ndx);
}

/*********************************************************************
*     Get an entry from the Database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_direct_extraction_entry_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx,
    SOC_SAND_OUT uint8                                 *is_found,
    SOC_SAND_OUT uint8                                 *is_fes,
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *info
  )
{
  uint32
    qual_type_ndx,
    res = SOC_SAND_OK;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info;
  uint8
    is_large_direct_extraction,
      is_for_tm,
      is_default_tm,
    fem_is_found,
    fes_is_found,
    fem_pgm_id;
  uint32
    action_ndx,
    fem_id_ndx,
    fes_group_idx,
    pgm_idx,
    pgm_idx_min, 
    pgm_idx_max,
    fes_idx,
    cycle_ndx;
  SOC_PPC_FP_FEM_ENTRY
    fem_entry;
  ARAD_PMF_FES            
    fes_info;
  SOC_PPC_FP_DATABASE_STAGE
      stage; 

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(is_found);
  SOC_SAND_CHECK_NULL_INPUT(info);

  *is_found = FALSE;
  *is_fes = FALSE;

  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_clear(info);

  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  res = arad_pp_fp_database_is_tm_get(unit, &fp_database_info, &is_for_tm, &is_default_tm);
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);

  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  res = arad_pp_fp_database_is_large_direct_extraction_get(
          unit,
          &fp_database_info,
          &is_large_direct_extraction
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);

  /* 
   * Complete different sequence if large DB or not: 
   * - for thin DBs, alloc a FEM and set it 
   * - for large DBs, alloc the key in this exact order, 
   * alloc a FES and set it (indicate if filter or not)
   */
 
    /*
      * Find if there is a FEM for this entry
      */
      fem_is_found = FALSE;
 
    action_ndx = 0;
    for (cycle_ndx = 0; (cycle_ndx < ARAD_PMF_NOF_CYCLES) && (fem_is_found == FALSE); ++cycle_ndx)
    {
      for (fem_id_ndx = 0; (fem_id_ndx < ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP) && (fem_is_found == FALSE); ++fem_id_ndx)
      {
        for(fem_pgm_id = 0 ; (fem_pgm_id < ARAD_PMF_NOF_FEM_PGMS) && (fem_is_found == FALSE); ++fem_pgm_id)
        {
            SOC_PPC_FP_FEM_ENTRY_clear(&fem_entry);
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fem_entry.get(
                    unit,
                    stage,
                    fem_pgm_id,
                    fem_id_ndx + (cycle_ndx * ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP),
                    &fem_entry
                  );
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);

            if ((fem_entry.db_id == db_id_ndx) && (fem_entry.entry_id == entry_id_ndx) && (fem_entry.is_for_entry == TRUE))
            {
               res = arad_pp_fp_fem_configuration_de_get(
                      unit,
                      fem_id_ndx,
                      cycle_ndx,
                      fem_pgm_id,
                      &fem_entry,
                      &(info->actions[action_ndx]),
                      &(info->qual_vals[0])
                    );
               SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
               info->priority = fem_entry.entry_strength;
               action_ndx ++;
               fem_is_found = TRUE;
            }
        }
      }
    }
    if ( fem_is_found  ) {
        *is_found = TRUE;
        ARAD_DO_NOTHING_AND_EXIT;
    }

    /*
     * Find if there is a FES for this entry . 
     */

    fes_is_found = FALSE;
    /* using the macro SOC_DPP_DEFS_MAX can compare definitions of the same value */
    /* coverity[same_on_both_sides] */
    for (cycle_ndx = 0; (cycle_ndx < ARAD_PMF_NOF_CYCLES) && (fes_is_found == FALSE); ++cycle_ndx)
    {
      /* Each cycle starts with 16 FESs */
      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                unit,
                stage,
                is_for_tm, 
                &pgm_idx_min,
                &pgm_idx_max
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);
      for(pgm_idx = pgm_idx_min; (pgm_idx < pgm_idx_max) && (fes_is_found == FALSE);  pgm_idx++)
      {
        for(fes_group_idx = 0; fes_group_idx < ARAD_PMF_LOW_LEVEL_NOF_FESS_PER_GROUP; fes_group_idx++)
        {
          fes_idx = (cycle_ndx * ARAD_PMF_LOW_LEVEL_NOF_FESS_PER_GROUP) + fes_group_idx;

          sal_memset(&fes_info, 0x0, sizeof(ARAD_PMF_FES));
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.get(
                  unit, 
                  stage, 
                  pgm_idx, 
                  fes_idx, 
                  &fes_info
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 170, exit); 

          if ((fes_info.db_id == db_id_ndx) && (fes_info.entry_id == entry_id_ndx) && (fes_info.is_used == TRUE))
          {
           fes_is_found = TRUE;
           break;
          }/*if */
        }/*for(fes_group_idx)*/
      }/*for(pgm_idx*/
    }/*for (cycle_ndx*/

     if (fes_is_found) {
       *is_found = TRUE;
       *is_fes = TRUE;
        /* 
         * The Key is in the exact order of the DB qualifiers
         */ 
        /* Set the qual type array */
        for (qual_type_ndx = 0; qual_type_ndx < SOC_PPC_FP_DIR_EXTR_MAX_NOF_FIELDS; qual_type_ndx++) {
          info->actions[0].fld_ext[qual_type_ndx].type = fp_database_info.qual_types[qual_type_ndx];
          if ((fp_database_info.qual_types[qual_type_ndx] == SOC_PPC_NOF_FP_QUAL_TYPES)
		       || (fp_database_info.qual_types[qual_type_ndx] == BCM_FIELD_ENTRY_INVALID)
               || (fp_database_info.qual_types[qual_type_ndx] == SOC_PPC_FP_QUAL_IRPP_INVALID))
          {
            break;
          }
          /* Get the qualifier size */
          res = arad_pp_fp_key_length_get_unsafe(
                  unit,
                  stage,
                  fp_database_info.qual_types[qual_type_ndx],
                  FALSE /* not supposed to have padding */,
                  &(info->actions[0].fld_ext[qual_type_ndx].nof_bits)
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 34, exit);

          info->actions[0].nof_fields++;
          if (!is_large_direct_extraction) 
          {
            break;
          }
        }

        /* Retrieve the action type */
        info->actions[0].type = fes_info.action_type; 
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_entry_get_unsafe()", db_id_ndx, entry_id_ndx);
}

uint32
  arad_pp_fp_direct_extraction_entry_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id_ndx, ARAD_PP_FP_DE_ENTRY_ID_NDX_MAX, ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_entry_get_verify()", db_id_ndx, entry_id_ndx);
}

/*********************************************************************
*     Remove an entry from the Database.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_direct_extraction_entry_remove_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx
  )
{
  uint32
    res = SOC_SAND_OK;
  ARAD_FP_ENTRY
    fp_entry;
  uint8
      is_large_direct_extraction,
    is_fes,
    fem_is_found;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info;
  uint32
    action_ndx;
  SOC_PPC_FP_FEM_ENTRY
    fem_entry;
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO
    *dir_extr_entry_info = NULL;
  SOC_PPC_FP_DATABASE_STAGE
      stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF; /* Single stage with FEMs */


  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_REMOVE_UNSAFE);


  /*
   * Find the FEM
   */
  SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
  res = arad_pp_fp_database_get_unsafe(
          unit,
          db_id_ndx,
          &fp_database_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
  fem_is_found = FALSE;

  res = arad_pp_fp_database_is_large_direct_extraction_get(
          unit,
          &fp_database_info,
          &is_large_direct_extraction
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 15, exit);


  ARAD_ALLOC(dir_extr_entry_info, SOC_PPC_FP_DIR_EXTR_ENTRY_INFO, 1, "arad_pp_fp_direct_extraction_entry_remove_unsafe.dir_extr_entry_info");
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_clear(dir_extr_entry_info);
  res = arad_pp_fp_direct_extraction_entry_get_unsafe(
          unit,
          db_id_ndx,
          entry_id_ndx,
          &fem_is_found,
          &is_fes,
          dir_extr_entry_info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if ( (!is_large_direct_extraction) && (fem_is_found) && (!is_fes)) {

      /*
       * Build the entry struct
       */
      SOC_PPC_FP_FEM_ENTRY_clear(&fem_entry);
      fem_entry.is_for_entry = TRUE;
      fem_entry.db_id = db_id_ndx;
      fem_entry.entry_id = entry_id_ndx;
      for (action_ndx = 0; action_ndx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; action_ndx ++)
      {
        fem_entry.action_type[action_ndx] = dir_extr_entry_info->actions[action_ndx].type;
      }

      /*
       * Remove from the HW (and reorganize the FEMs)
       */
      res = arad_pp_fp_fem_remove(
              unit,
              &fem_entry
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }
  else { /* is_large_direct_extraction or use FES */
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
              unit,
              stage,
              db_id_ndx,
              &fp_entry
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);
      if (fp_entry.nof_db_entries) {
          fem_is_found = TRUE;
          /* 
           * De-allocate the FES 
           * De-allocate the Key 
           */
          res = arad_pp_fp_action_dealloc(
                  unit,
                  db_id_ndx,
                  fp_database_info.action_types
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

          res = arad_pp_fp_key_dealloc(
                  unit,
                  stage,
                  db_id_ndx,
                  fp_database_info.qual_types
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      }
  }

  /*
   * Reset the FEM as in the DB create API
   */
  if (fem_is_found != FALSE)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
            unit,
            stage,
            db_id_ndx,
            &fp_entry
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 45, exit);

    /*
     *	Update the SW DB
     */
    fp_entry.nof_db_entries --;

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.set(
            unit,
            stage,
            db_id_ndx,
            &fp_entry
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
  }

exit:
  ARAD_FREE(dir_extr_entry_info);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_entry_remove_unsafe()", db_id_ndx, entry_id_ndx);
}

uint32
  arad_pp_fp_direct_extraction_entry_remove_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_IN  uint32                                 entry_id_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_ENTRY_REMOVE_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(entry_id_ndx, ARAD_PP_FP_DE_ENTRY_ID_NDX_MAX, ARAD_PP_FP_ENTRY_ID_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_entry_remove_verify()", db_id_ndx, entry_id_ndx);
}

/*********************************************************************
*     Get the Database entries.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_direct_extraction_db_entries_get_unsafe(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                  *block_range,
    SOC_SAND_OUT SOC_PPC_FP_DIR_EXTR_ENTRY_INFO              *entries,
    SOC_SAND_OUT uint32                                  *nof_entries
  )
{
  uint32
    nof_valid_entries = 0,
    res = SOC_SAND_OK;
  uint32
    entry_ndx;
  ARAD_FP_ENTRY
    fp_entry;
  uint8
    is_fes = FALSE,
    is_found = FALSE;
  SOC_PPC_FP_DATABASE_STAGE
      stage;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(block_range);
  SOC_SAND_CHECK_NULL_INPUT(entries);
  SOC_SAND_CHECK_NULL_INPUT(nof_entries);

  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_clear(entries);

  /* Get the entry stage */
  res = arad_pp_fp_db_stage_get(
            unit,
            db_id_ndx,
            &stage
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 5, exit);

  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
          unit,
          stage,
          db_id_ndx,
          &fp_entry
        );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

  if (fp_entry.nof_db_entries != 0)
  {
    for (entry_ndx = block_range->iter; entry_ndx < block_range->iter + block_range->entries_to_scan; ++entry_ndx)
    {
      if (nof_valid_entries >= block_range->entries_to_act)
      {
        /*
         *	No need to go further, maximal number of entries have been found
         */
        break;
      }
      res = arad_pp_fp_direct_extraction_entry_get_unsafe(
              unit,
              db_id_ndx,
              entry_ndx,
              &is_found,
              &is_fes,
              &(entries[nof_valid_entries])
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

      if (is_found == TRUE)
      {
        nof_valid_entries ++;
      }
    }
  }
  else
  {
    entry_ndx = block_range->iter + block_range->entries_to_scan;
    nof_valid_entries = 0;
  }

  *nof_entries = nof_valid_entries;
  block_range->iter = entry_ndx;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_db_entries_get_unsafe()", db_id_ndx, 0);
}

uint32
  arad_pp_fp_direct_extraction_db_entries_get_verify(
    SOC_SAND_IN  int                                 unit,
    SOC_SAND_IN  uint32                                 db_id_ndx,
    SOC_SAND_INOUT SOC_SAND_TABLE_BLOCK_RANGE                 *block_range
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_DIRECT_EXTRACTION_DB_ENTRIES_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(db_id_ndx, ARAD_PP_FP_DB_ID_NDX_MAX, ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 10, exit);
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_direct_extraction_db_entries_get_verify()", db_id_ndx, 0);
}

/* Get the correct Port key gen var table */
STATIC
soc_mem_t
  arad_pp_fp_port_table_get(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  int                       core,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_TYPE   control_type,
    SOC_SAND_OUT int                       *block
  )
{
    soc_mem_t
        port_table = 0;
    *block = MEM_BLOCK_ANY;

    switch (control_type) {
    case SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA:
        port_table = EGQ_PP_PPCTm;
        if(SOC_IS_JERICHO(unit)) {
            *block = EGQ_BLOCK(unit, core);
        }
        break;
    case SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA:
        port_table = EGQ_PPCTm;
        if(SOC_IS_JERICHO(unit)) {
            *block = EGQ_BLOCK(unit, core);
        }
        break;
    case SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA:
        if(SOC_IS_JERICHO(unit)) {
            port_table = IHP_PINFO_FLP_0m;
            *block = IHP_BLOCK(unit, core);
        } else {
            port_table = IHB_PINFO_FLPm;
            *block = IHB_BLOCK(unit, core);
        }
        break;
    case SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA:
        if(SOC_IS_JERICHO(unit)) {
            port_table = IHB_PINFO_PMFm;
            *block = IHB_BLOCK(unit, core);
        } else {
            port_table = IHB_IN_PORT_KEY_GEN_VARm;
        }
        break;
    case SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA:
        port_table = IHB_PTC_KEY_GEN_VARm;
        *block = IHB_BLOCK(unit, core);
        break;
    default:
        break;
    }

    return port_table;
}

/* Get the correct Port key gen var table */
STATIC
soc_field_t
  arad_pp_fp_port_field_get(
    SOC_SAND_IN  int                       unit,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_TYPE   control_type
  )
{
    soc_field_t
        port_field = 0;

    switch (control_type) {
    case SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA:
        port_field = PMF_DATAf;
        break;
    case SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA:
        port_field = PMF_DATAf;
        break;
    case SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA:
        port_field = KEY_GEN_VARf;
        break;
    case SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA:
        if(SOC_IS_JERICHO(unit)) {
            port_field = KEY_GEN_VARf;
        } else {
            port_field = IN_PORT_KEY_GEN_VARf;
        }
        break;
    case SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA:
        port_field = PTC_KEY_GEN_VARf;
        break;
    default:
        break;
    }

    return port_field;
}



/*********************************************************************
*     Set one of the control options.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_control_set_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX       *control_ndx,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INFO        *info,
    SOC_SAND_OUT SOC_SAND_SUCCESS_FAILURE         *success
  )
{
  uint32
    profile_object_idx, /* Port or FLP-Program */
      profile_id,
      temp_indx,
      fld_data[ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS],
      tbl_data[ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS],
    res = SOC_SAND_OK;
  ARAD_PMF_CE_SUB_HEADER
    base_header_type;
  ARAD_PMF_CE_QUAL_INFO
    qual_info;
  soc_reg_above_64_val_t
      l4_ops_tbl_data;
  SOC_PPC_FP_DATABASE_STAGE
      stage;
  ARAD_IHB_PINFO_PMF_TBL_DATA 
    pinfo_tbl_data;
   ARAD_IHB_PTC_INFO_PMF_TBL_DATA   
    ptc_info_tbl_data; 
  ARAD_PP_EGQ_PP_PPCT_TBL_DATA
    pp_ppct_tbl_data;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  soc_mem_t
      port_table;
  int block;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_CONTROL_SET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(control_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);
  SOC_SAND_CHECK_NULL_INPUT(success);

  
  *success = SOC_SAND_SUCCESS;

  switch (control_ndx->type)
  {
  case SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE:
    res = READ_IHB_L_4_OPSm(unit, MEM_BLOCK_ANY, control_ndx->val_ndx, l4_ops_tbl_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);

    soc_IHB_L_4_OPSm_field32_set(unit, l4_ops_tbl_data, SOURCE_PORT_MINf, info->val[0]);
    soc_IHB_L_4_OPSm_field32_set(unit, l4_ops_tbl_data, SOURCE_PORT_MAXf, info->val[1]);
    soc_IHB_L_4_OPSm_field32_set(unit, l4_ops_tbl_data, DESTINATION_PORT_MINf, info->val[2]);
    soc_IHB_L_4_OPSm_field32_set(unit, l4_ops_tbl_data, DESTINATION_PORT_MAXf, info->val[3]);

    res = WRITE_IHB_L_4_OPSm(unit, MEM_BLOCK_ANY, control_ndx->val_ndx, l4_ops_tbl_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 15, exit);

    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA:
      sal_memset(tbl_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      sal_memset(fld_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      /* Set / get the table field */
      port_table = arad_pp_fp_port_table_get(unit, core_id, control_ndx->type, &block);
      if (control_ndx->type == SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA)
      {
          res = soc_port_sw_db_pp_port_to_base_q_pair_get(unit, core_id, control_ndx->val_ndx, &temp_indx);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 16, exit);
      } else {
           temp_indx = control_ndx->val_ndx;
      }
      res = soc_mem_read(unit, port_table, block, temp_indx, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);
      fld_data[0] = info->val[0];
      soc_mem_field_set(unit, port_table, tbl_data, arad_pp_fp_port_field_get(unit, control_ndx->type), fld_data);
      res = soc_mem_write(unit, port_table, block, temp_indx, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 18, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_PP_PORT:
      sal_memset(tbl_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      /* Set / get the ACE table field */
      res = soc_mem_read(unit, EPNI_ACE_TO_OUT_PP_PORTm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE, tbl_data);
      /* Copy the DSP-pointer in bit 8*i+7:8*i */
      SHR_BITCOPY_RANGE(tbl_data, 
                        ARAD_PP_FP_PP_PORT_IN_BITS * (control_ndx->val_ndx % ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE), 
                        &info->val[0],
                        0, 
                        ARAD_PP_FP_PP_PORT_IN_BITS);
      res = soc_mem_write(unit, EPNI_ACE_TO_OUT_PP_PORTm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 31, exit);

      /* Set the ACE-Pointer PRGE Var from info->val[1] */
      res = soc_mem_read(unit, EPNI_ACE_TABLEm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 32, exit);
      SHR_BITCOPY_RANGE(tbl_data, 
                        ARAD_PP_FP_PRGE_VAR_IN_BITS * (control_ndx->val_ndx % ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR), 
                        &info->val[1], 
                        0, 
                        ARAD_PP_FP_PRGE_VAR_IN_BITS);
      res = soc_mem_write(unit, EPNI_ACE_TABLEm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_ONLY:
      /* Set the ACE-Pointer PRGE Var from info->val[0] */
      res = soc_mem_read(unit, EPNI_ACE_TABLEm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 21, exit);
      SHR_BITCOPY_RANGE(tbl_data, 
                        ARAD_PP_FP_PRGE_VAR_IN_BITS * (control_ndx->val_ndx % ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR), 
                        &info->val[0], 
                        0, 
                        ARAD_PP_FP_PRGE_VAR_IN_BITS);
      res = soc_mem_write(unit, EPNI_ACE_TABLEm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 23, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_OUT_LIF:
      sal_memset(tbl_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      /* Set / get the table field */
      res = soc_mem_read(unit, EPNI_ACE_TO_OUT_LIFm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_OUT_LIF, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      /*
       * For Arad   : Copy the Out-lif in bit range 16*i:16*i+15, where I is either 0 or 1
       * For Jericho: Copy the Out-lif in bit range 18*i:18*i+17, where I is either 0 or 1
       */
      SHR_BITCOPY_RANGE(tbl_data, 
                        ARAD_PP_FP_OUT_LIF_IN_BITS * (control_ndx->val_ndx % ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_OUT_LIF), 
                        &info->val[0], 
                        0, 
                        ARAD_PP_FP_OUT_LIF_IN_BITS);
      res = soc_mem_write(unit, EPNI_ACE_TO_OUT_LIFm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_OUT_LIF, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

  break;

  case SOC_PPC_FP_CONTROL_TYPE_PACKET_SIZE_RANGE:
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_PACKET_HEADER_SIZE_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, PKT_HDR_SIZE_RANGE_LOW_Nf,  info->val[0] - 1));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  42,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_PACKET_HEADER_SIZE_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, PKT_HDR_SIZE_RANGE_HIGH_Nf,  info->val[1] - 1));
    break;

  case SOC_PPC_FP_CONTROL_TYPE_OUT_LIF_RANGE:
  {
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
        res,  45,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_OUT_LIF_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, OUT_LIF_RANGE_MIN_Nf,  info->val[0]));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
        res,  47,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IHB_OUT_LIF_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, OUT_LIF_RANGE_MAX_Bf,  info->val[1]));
    break;
  }
  case SOC_PPC_FP_CONTROL_TYPE_INNER_ETH_NOF_VLAN_TAGS:
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.inner_eth_nof_tags.set(
      unit,
      SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
      control_ndx->val_ndx,
      info->val[0]
    );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 48, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_KEY_CHANGE_SIZE:
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.key_change_size.set(
      unit,
      SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
      (uint8)   info->val[0]
      );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 49, exit);
    break;
  case SOC_PPC_FP_CONTROL_TYPE_EGRESS_DP:
    break;
  case SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF:
    soc_sand_os_memset(&qual_info, 0x0, sizeof(ARAD_PMF_CE_QUAL_INFO));

    /*
     * Build the instruction
     */
    if (control_ndx->db_id == 0) {
        SOC_PPC_FP_DATABASE_STAGE stage = info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_STAGE_NDX];
        /* 
         * instruction is built according to offset+size from packet headers
         */
        switch (info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_SUB_HEADER_NDX])
        {
        case SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_0:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_0;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_1:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_1;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_2:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_2;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_3:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_3;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_4:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_4;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_5:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_5;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_FWD:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_FWD;
          break;
        case SOC_PPC_FP_BASE_HEADER_TYPE_FWD_POST:
          base_header_type = ARAD_PMF_CE_SUB_HEADER_FWD_POST;
          break;
        default:
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 82, exit);
        }

        qual_info.is_header_qual = 1;
        qual_info.header_qual_info.qual_type = SOC_PPC_FP_QUAL_HDR_USER_DEF_0 + control_ndx->val_ndx;
        qual_info.header_qual_info.header_ndx_0 = base_header_type;
        if(info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] & SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_FLAG_NEGATIVE) {
            qual_info.header_qual_info.msb =  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] & (~SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_FLAG_NEGATIVE);/* gives the positive value*/
            qual_info.header_qual_info.msb *= -1;
        } else {
            qual_info.header_qual_info.msb =  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX];
        }
        qual_info.header_qual_info.lsb = qual_info.header_qual_info.msb
                                   + info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] - 1;
        qual_info.stage = stage;
    } else {
        /* 
         * instruction is built by taking some of the bits of a defined qualifier
         */
        SOC_PPC_FP_QUAL_TYPE     qual_type = info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX];
        if ((qual_type >= SOC_PPC_NOF_FP_QUAL_TYPES) || (qual_type == BCM_FIELD_ENTRY_INVALID)) {
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 86, exit);
        } else if (SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(qual_type)) {
            /* 
             * user-defining a qualifier from a user-defined qualifier doesn't make much sense...
             */
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 87, exit);
        } else {
            uint8 is_found;
            uint8 is_found_lsb;
            int index;
            SOC_PPC_FP_DATABASE_STAGE stage = info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_STAGE_NDX];
            
            res = arad_pmf_ce_header_info_find(unit,qual_type,stage,&is_found,&(qual_info.header_qual_info));
            SOC_SAND_CHECK_FUNC_RESULT(res, 88, exit);
            if (is_found) {
                int lsb, msb;
                msb = qual_info.header_qual_info.msb;
                lsb = qual_info.header_qual_info.lsb;
                qual_info.is_header_qual = 1;
                qual_info.header_qual_info.lsb -= info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX];
                qual_info.header_qual_info.msb = qual_info.header_qual_info.lsb - info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] + 1;
                if (qual_info.header_qual_info.msb > qual_info.header_qual_info.lsb || 
                qual_info.header_qual_info.msb < msb || qual_info.header_qual_info.msb > lsb ||
                qual_info.header_qual_info.lsb < msb || qual_info.header_qual_info.lsb > lsb) {
                    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 88, exit);
                }
            } else {
                res = arad_pmf_ce_internal_field_info_find(unit,qual_type,stage,1,&is_found,&(qual_info.irpp_qual_info[1]));
                SOC_SAND_CHECK_FUNC_RESULT(res, 89, exit);
                res = arad_pmf_ce_internal_field_info_find(unit,qual_type,stage,0,&is_found_lsb,&(qual_info.irpp_qual_info[0]));
                SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

                if (!(is_found | is_found_lsb)) {
                    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 91, exit);
                }
                for (index=0; index < 2; ++index) {
                    if (qual_info.irpp_qual_info[index].info.qual_nof_bits) {
                        qual_info.irpp_qual_info[index].info.buffer_lsb += info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX];
                        if (info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] + info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] >
                            qual_info.irpp_qual_info[index].info.qual_nof_bits) {
                            SOC_SAND_CHECK_FUNC_RESULT(res, 92, exit);
                        }
                        qual_info.irpp_qual_info[index].info.qual_nof_bits = info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX];
                    }
                }

            }
            qual_info.stage = stage;
        }
    }

    /* 
     * Set it at any stage
     */
    for (stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage ++) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.udf.set(
                unit,
                stage,
                control_ndx->val_ndx,
                &qual_info
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 90, exit);
    }
    break;
  case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
    for(profile_object_idx = 0; profile_object_idx < ARAD_PP_FP_NOF_PROFILE_OBJECTS(unit, control_ndx->type); profile_object_idx++)
    {
      /* For each port check if it was requested or it should be cleared */
      if(control_ndx->clear_val || (SHR_BITGET(info->val, profile_object_idx)))
      {
        /* Set the Profile in control_ndx->val_ndx to specified ports,
         * unless they are already configured.
         */
          switch (control_ndx->type) {
          case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
            res = arad_ihb_pinfo_pmf_tbl_get_unsafe(unit, core_id, profile_object_idx, &pinfo_tbl_data);
            SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
            profile_id = pinfo_tbl_data.port_pmf_profile;
            break;
          case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE:  
            res = arad_ihb_ptc_info_pmf_tbl_get_unsafe(unit, core_id, profile_object_idx, &ptc_info_tbl_data);  
            SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);  
            profile_id = ptc_info_tbl_data.interface_port_pmf_profile;  
            break; 
          case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
              res = arad_pp_egq_pp_ppct_tbl_get_unsafe(unit, core_id, profile_object_idx, &pp_ppct_tbl_data);
              SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
              profile_id = pp_ppct_tbl_data.pmf_profile;
              break;
          case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
              res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, profile_object_idx, &flp_process_tbl);
              SOC_SAND_CHECK_FUNC_RESULT(res, 133, exit);
              profile_id = flp_process_tbl.fwd_processing_profile;
              break;
          default:
              SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PORT_PROFILE_ALREADY_EXIST_ERR, 82, exit);
          }

        /* Make sure port doesn't already have a profile */
        if(!control_ndx->clear_val 
           && (profile_id != 0) 
           && (profile_id != control_ndx->val_ndx)) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PORT_PROFILE_ALREADY_EXIST_ERR, 115, exit);
        }

        /* Set port profile only if it was asked for OR if it should be cleared */
        if(!control_ndx->clear_val 
           || (profile_id == control_ndx->val_ndx))
        {
            profile_id = control_ndx->clear_val ? 0 : control_ndx->val_ndx;
            switch (control_ndx->type) {
            case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
                pinfo_tbl_data.port_pmf_profile = profile_id;
                res = arad_ihb_pinfo_pmf_tbl_set_unsafe(unit, core_id, profile_object_idx, &pinfo_tbl_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
              break;
            case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE:  
                ptc_info_tbl_data.interface_port_pmf_profile = profile_id;  
                res = arad_ihb_ptc_info_pmf_tbl_set_unsafe(unit, core_id, profile_object_idx, &ptc_info_tbl_data);  
                SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);  
                break; 
            case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
                pp_ppct_tbl_data.pmf_profile = profile_id;
                res = arad_pp_egq_pp_ppct_tbl_set_unsafe(unit, core_id, profile_object_idx, &pp_ppct_tbl_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);
                break;
            case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
                flp_process_tbl.fwd_processing_profile = profile_id;
                res = arad_pp_ihb_flp_process_tbl_set_unsafe(unit, profile_object_idx, &flp_process_tbl);
                SOC_SAND_CHECK_FUNC_RESULT(res, 141, exit);
                break;
            default:
                SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PORT_PROFILE_ALREADY_EXIST_ERR, 82, exit);
            }
        }
      }
    }
    break;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  case SOC_PPC_FP_CONTROL_TYPE_KBP_CACHE:
    /* set KBP cache value */
    res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_cache_mode.set(unit, info->val[0]);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 150, exit);
    break;
  case SOC_PPC_FP_CONTROL_TYPE_KBP_COMMIT:
    /* commit KBP cached configuration */
    res = arad_kbp_db_commit(unit);
    SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);
    break;
#endif
  default:
    break;
    
    /* SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 1000, exit);*/
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_control_set_unsafe()", 0, 0);
return 0;
}

uint32
  arad_pp_fp_control_set_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX       *control_ndx,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INFO        *info
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_CONTROL_SET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FP_CONTROL_INDEX, control_ndx, 10, exit);
  res = SOC_PPC_FP_CONTROL_INFO_verify(
          unit,
          control_ndx->type,
          info
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_control_set_verify()", 0, 0);
}

uint32
  arad_pp_fp_control_get_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX       *control_ndx
  )
{
  uint32
    res = SOC_SAND_OK;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_CONTROL_GET_VERIFY);

  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FP_CONTROL_INDEX, control_ndx, 10, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_control_get_verify()", 0, 0);
}

/*********************************************************************
*     Set one of the control options.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_control_get_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX       *control_ndx,
    SOC_SAND_OUT SOC_PPC_FP_CONTROL_INFO        *info
  )
{
  uint32
    profile_object_idx,
      profile_id,
      fld_data[ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS],
      tbl_data[ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS],
    res = SOC_SAND_OK;
  SOC_PPC_FP_BASE_HEADER_TYPE
    base_header_type;
  ARAD_PMF_CE_QUAL_INFO
    qual_info;
  soc_reg_above_64_val_t
      l4_ops_tbl_data;
  ARAD_IHB_PINFO_PMF_TBL_DATA 
    pinfo_tbl_data; 
  ARAD_IHB_PTC_INFO_PMF_TBL_DATA   
    ptc_info_tbl_data; 
  ARAD_PP_EGQ_PP_PPCT_TBL_DATA
    pp_ppct_tbl_data;
  ARAD_PP_IHB_FLP_PROCESS_TBL_DATA
    flp_process_tbl;
  soc_mem_t
      port_table;
  uint8
      key_change_size;
  int block;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_CONTROL_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(control_ndx);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_PPC_FP_CONTROL_INFO_clear(info);

  

  switch (control_ndx->type)
  {
  case SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE:
    res = READ_IHB_L_4_OPSm(unit, MEM_BLOCK_ANY, control_ndx->val_ndx, l4_ops_tbl_data);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 12, exit);

    info->val[0] = soc_IHB_L_4_OPSm_field32_get(unit, l4_ops_tbl_data, SOURCE_PORT_MINf);
    info->val[1] = soc_IHB_L_4_OPSm_field32_get(unit, l4_ops_tbl_data, SOURCE_PORT_MAXf);
    info->val[2] = soc_IHB_L_4_OPSm_field32_get(unit, l4_ops_tbl_data, DESTINATION_PORT_MINf);
    info->val[3] = soc_IHB_L_4_OPSm_field32_get(unit, l4_ops_tbl_data, DESTINATION_PORT_MAXf);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA:
      sal_memset(tbl_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      sal_memset(fld_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      /* Set / get the table field */
      port_table = arad_pp_fp_port_table_get(unit, core_id, control_ndx->type, &block);
      res = soc_mem_read(unit, port_table, block, control_ndx->val_ndx, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 17, exit);
      soc_mem_field_get(unit, port_table, tbl_data, arad_pp_fp_port_field_get(unit, control_ndx->type), fld_data);
      info->val[0] = fld_data[0];
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_PP_PORT:
      sal_memset(tbl_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      /* Set / get the table field */
      res = soc_mem_read(unit, EPNI_ACE_TO_OUT_PP_PORTm, EPNI_BLOCK(unit, core_id), control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
      /* Copy the PP-Port in bit 8*i+7:8*i */
      SHR_BITCOPY_RANGE(&info->val[0], 
                        0, 
                        tbl_data, 
                        ARAD_PP_FP_PP_PORT_IN_BITS * (control_ndx->val_ndx % ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE), 
                        ARAD_PP_FP_PP_PORT_IN_BITS);

      sal_memset(tbl_data, 0x0, ARAD_PP_FP_KEY_GEN_VAR_MAX_IN_LONGS * sizeof(uint32));
      /* Set / get the table field */
      res = soc_mem_read(unit, EPNI_DSP_PTR_MAPm, EPNI_BLOCK(unit, core_id), info->val[0], tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 24, exit);
      /* Copy the TM-Port in bit 7:0 of DSP-Ptr-map[out-PP-port]*/
      SHR_BITCOPY_RANGE(&info->val[2],
                        0, 
                        tbl_data, 
                        0, 
                        ARAD_PP_FP_PP_PORT_IN_BITS);
      /* Set / get the table field */
      res = soc_mem_read(unit, EPNI_ACE_TABLEm, MEM_BLOCK_ANY, control_ndx->val_ndx / ARAD_PP_FP_NOF_ACE_POINTERS_IN_LINE_PRGE_VAR, tbl_data);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 22, exit);
      break;

  case SOC_PPC_FP_CONTROL_TYPE_PACKET_SIZE_RANGE:
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
      res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHB_PACKET_HEADER_SIZE_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, PKT_HDR_SIZE_RANGE_LOW_Nf, &info->val[0]));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
      res,  42,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHB_PACKET_HEADER_SIZE_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, PKT_HDR_SIZE_RANGE_HIGH_Nf, &info->val[1]));
    info->val[0] ++;
    info->val[1] ++;
    break;

  case SOC_PPC_FP_CONTROL_TYPE_OUT_LIF_RANGE:
  {
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
      res,  45,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHB_OUT_LIF_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, OUT_LIF_RANGE_MIN_Nf, &info->val[0]));
    SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(
      res,  47,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IHB_OUT_LIF_RANGEr, SOC_CORE_ALL,  control_ndx->val_ndx, OUT_LIF_RANGE_MAX_Bf, &info->val[1]));
    break;
  }
  case SOC_PPC_FP_CONTROL_TYPE_INNER_ETH_NOF_VLAN_TAGS:
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.inner_eth_nof_tags.get(
            unit,
            SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF,
            control_ndx->val_ndx,
            &(info->val[0])
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 84, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_KEY_CHANGE_SIZE:
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.key_change_size.get(unit, SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, &key_change_size);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 85, exit);
    info->val[0] = key_change_size;
    break;
  case SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF:
    /*
     * Build the instruction
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.udf.get(
            unit,
            SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF, /* Take any stage since all are set similarly */
            control_ndx->val_ndx,
            &qual_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 86, exit);

      if (SOC_PPC_FP_IS_QUAL_TYPE_USER_DEFINED(qual_info.header_qual_info.qual_type)) {
          /* User-defined qualifier built from packet headers */
          switch (qual_info.header_qual_info.header_ndx_0)
          {
          case ARAD_PMF_CE_SUB_HEADER_0:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_0;
            break;
          case ARAD_PMF_CE_SUB_HEADER_1:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_1;
            break;
          case ARAD_PMF_CE_SUB_HEADER_2:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_2;
            break;
          case ARAD_PMF_CE_SUB_HEADER_3:
            base_header_type = ARAD_PMF_CE_SUB_HEADER_3;
            break;
          case ARAD_PMF_CE_SUB_HEADER_4:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_4;
            break;
          case ARAD_PMF_CE_SUB_HEADER_5:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_HEADER_5;
            break;
          case ARAD_PMF_CE_SUB_HEADER_FWD:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_FWD;
            break;
          case ARAD_PMF_CE_SUB_HEADER_FWD_POST:
            base_header_type = SOC_PPC_FP_BASE_HEADER_TYPE_FWD_POST;
            break;
          default:
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 82, exit);
          }
          info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_SUB_HEADER_NDX] = base_header_type;
          if(qual_info.header_qual_info.msb >= 0) {
              info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] = qual_info.header_qual_info.msb;
          } else {
              info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] = (-qual_info.header_qual_info.msb) | SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_FLAG_NEGATIVE;
          }
          info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] = (uint32)(qual_info.header_qual_info.lsb - qual_info.header_qual_info.msb) + 1;
      } else {
          uint8 is_found;
          uint8 is_found_lsb;
          SOC_PPC_FP_DATABASE_STAGE stage = qual_info.stage;
          if (qual_info.is_header_qual) {
              ARAD_PMF_CE_HEADER_QUAL_INFO header_qual_info;
              res = arad_pmf_ce_header_info_find(unit,qual_info.header_qual_info.qual_type,stage,&is_found,&header_qual_info);
              SOC_SAND_CHECK_FUNC_RESULT(res, 86, exit);
              if (!is_found) {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 87, exit);
              } else {
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX] = qual_info.header_qual_info.qual_type;
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] = header_qual_info.lsb - qual_info.header_qual_info.lsb;
                /* Here we are always with positive offset. */
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] = qual_info.header_qual_info.lsb - qual_info.header_qual_info.msb + 1;
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_SUB_HEADER_NDX] = ARAD_NOF_PMF_CE_SUB_HEADERS;
              }
          } else {
              ARAD_PMF_CE_IRPP_QUALIFIER_INFO irpp_qual_info;
              res = arad_pmf_ce_internal_field_info_find(unit,qual_info.irpp_qual_info[1].info.irpp_field,stage,1,&is_found,&irpp_qual_info);
              SOC_SAND_CHECK_FUNC_RESULT(res, 89, exit);
              if (!is_found) {
                  res = arad_pmf_ce_internal_field_info_find(unit,qual_info.irpp_qual_info[0].info.irpp_field,stage,0,&is_found_lsb,&irpp_qual_info);
                  SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
              }
              if (!(is_found || is_found_lsb)) {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 91, exit);
              }
              if (is_found) {
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX] = qual_info.irpp_qual_info[1].info.irpp_field;
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] = qual_info.irpp_qual_info[1].info.buffer_lsb - irpp_qual_info.info.buffer_lsb;
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] = qual_info.irpp_qual_info[1].info.qual_nof_bits;
              }
              else {
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_PARTIAL_QUAL_NDX] = qual_info.irpp_qual_info[0].info.irpp_field;
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_OFFSET_NDX] = qual_info.irpp_qual_info[0].info.buffer_lsb - irpp_qual_info.info.buffer_lsb;
                  info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_NOF_BITS_NDX] = qual_info.irpp_qual_info[0].info.qual_nof_bits;
              }
              info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_SUB_HEADER_NDX] = ARAD_NOF_PMF_CE_SUB_HEADERS;
          }
      }
      info->val[SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_STAGE_NDX] = qual_info.stage;
    break;
  case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
    for(profile_object_idx = 0; profile_object_idx < ARAD_PP_FP_NOF_PROFILE_OBJECTS(unit, control_ndx->type); profile_object_idx++)
    {
        switch (control_ndx->type) {
        case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
          res = arad_ihb_pinfo_pmf_tbl_get_unsafe(unit, core_id, profile_object_idx, &pinfo_tbl_data);
          SOC_SAND_CHECK_FUNC_RESULT(res, 105, exit);
          profile_id = pinfo_tbl_data.port_pmf_profile;
          break;
        case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE:  
            res = arad_ihb_ptc_info_pmf_tbl_get_unsafe(unit, core_id, profile_object_idx, &ptc_info_tbl_data);  
            SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);  
            profile_id = ptc_info_tbl_data.interface_port_pmf_profile;  
            break; 
        case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
            res = arad_pp_egq_pp_ppct_tbl_get_unsafe(unit,core_id, profile_object_idx, &pp_ppct_tbl_data);
            SOC_SAND_CHECK_FUNC_RESULT(res, 125, exit);
            profile_id = pp_ppct_tbl_data.pmf_profile;
            break;
        case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
            res = arad_pp_ihb_flp_process_tbl_get_unsafe(unit, profile_object_idx, &flp_process_tbl);
            SOC_SAND_CHECK_FUNC_RESULT(res, 133, exit);
            profile_id = flp_process_tbl.fwd_processing_profile;
            break;
        default:
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_PORT_PROFILE_ALREADY_EXIST_ERR, 81, exit);
        }

      /* Check if port is related to specified profile */
      if(profile_id == control_ndx->val_ndx) {
        SHR_BITSET(info->val, profile_object_idx);
      }
    }
    break;
#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  case SOC_PPC_FP_CONTROL_TYPE_KBP_CACHE:
    {
        uint8 val;
        /* set KBP cache value */
        res = sw_state_access[unit].dpp.soc.arad.pp.frwrd_ip.kbp_cache_mode.get(unit, &val);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 140, exit);
        info->val[0] = val;
    }
    break;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */
  default:
    
    break;
    /*SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 60, exit);*/
  }

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_control_get_unsafe()", 0, 0);
}



/*********************************************************************
*     Set the mapping between the Packet forward type and the
 *     Port profile to the Database-ID.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_egr_db_map_set_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx,
    SOC_SAND_IN  uint32                    port_profile_ndx,
    SOC_SAND_IN  uint32                     db_id
  )
{
return 0;
}

uint32
  arad_pp_fp_egr_db_map_set_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx,
    SOC_SAND_IN  uint32                    port_profile_ndx,
    SOC_SAND_IN  uint32                     db_id
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_EGR_DB_MAP_SET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(fwd_type_ndx, SOC_PPC_FP_FWD_TYPE_NDX_MAX, SOC_PPC_FP_FWD_TYPE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(port_profile_ndx, ARAD_PP_FP_PORT_PROFILE_NDX_MAX, ARAD_PP_FP_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR, 20, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(db_id, ARAD_PP_FP_DB_ID_MAX, ARAD_PP_FP_DB_ID_OUT_OF_RANGE_ERR, 30, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_egr_db_map_set_verify()", 0, port_profile_ndx);
}

uint32
  arad_pp_fp_egr_db_map_get_verify(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx,
    SOC_SAND_IN  uint32                    port_profile_ndx
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_EGR_DB_MAP_GET_VERIFY);

  SOC_SAND_ERR_IF_ABOVE_MAX(fwd_type_ndx, SOC_PPC_FP_FWD_TYPE_NDX_MAX, SOC_PPC_FP_FWD_TYPE_NDX_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(port_profile_ndx, ARAD_PP_FP_PORT_PROFILE_NDX_MAX, ARAD_PP_FP_PORT_PROFILE_NDX_OUT_OF_RANGE_ERR, 20, exit);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_egr_db_map_get_verify()", 0, port_profile_ndx);
}

/*********************************************************************
*     Set the mapping between the Packet forward type and the
 *     Port profile to the Database-ID.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_egr_db_map_get_unsafe(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_FWD_TYPE            fwd_type_ndx,
    SOC_SAND_IN  uint32                    port_profile_ndx,
    SOC_SAND_OUT uint32                     *db_id
  )
{
return 0;
}

/*********************************************************************
*     Compress a TCAM Database: compress the entries to minimum
*     number of banks.
*********************************************************************/
uint32
  arad_pp_fp_database_compress_unsafe(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32                  db_id_ndx
  )
{
    uint32
        res = SOC_SAND_OK;
    SOC_PPC_FP_DATABASE_INFO
        fp_database_info;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
    res = arad_pp_fp_database_get_unsafe(
            unit,
            db_id_ndx,
            &fp_database_info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Different behavior for Direct Table database */
    if ((fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_TCAM) || (fp_database_info.db_type == SOC_PPC_FP_DB_TYPE_EGRESS))
    {
        res = arad_tcam_managed_db_compress_unsafe(unit, ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_id_ndx));
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_database_compress_unsafe()", db_id_ndx, 0);
}

/* Get the program and program selection bitmap */
uint32
  arad_pp_fp_packet_diag_pgm_bmp_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE stage,
    SOC_SAND_IN  int                    core_id,
    SOC_SAND_OUT  uint32                *pmf_progs_bmp_final,
    SOC_SAND_OUT  uint32                pfg_bmp[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES]
  )
{
  uint32
    pmf_progs_bmp,
      nof_selected_cam_line_regs,
      fld_val,
      fld_len,
      selected_cam_line_ndx,
    res = SOC_SAND_OK;
  soc_reg_t
      selected_pgm,
      selected_cam_line[2] = {0};
  soc_field_t
      selected_pgm_fld,
      selected_cam_line_fld[2] = {0};

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  pfg_bmp[0] = 0;
  pfg_bmp[1] = 0;

  if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
      selected_pgm = IHB_DBG_PMF_SELECTED_PROGRAMr;
      selected_pgm_fld = DBG_PMF_SELECTED_PROGRAMf;
      selected_cam_line[0] = IHB_DBG_PMF_SELECTED_CAM_LINE_0r;
      selected_cam_line[1] = IHB_DBG_PMF_SELECTED_CAM_LINE_1r;
      selected_cam_line_fld[0] = DBG_PMF_SELECTED_CAM_LINE_0f;
      selected_cam_line_fld[1] = DBG_PMF_SELECTED_CAM_LINE_1f;
      nof_selected_cam_line_regs = 2;
  }
#ifdef BCM_88660_A0
  else if ((SOC_IS_ARADPLUS(unit)) && (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)) {
      selected_pgm = IHP_DBG_FLP_CONSISTENT_HASHING_PROGRAMr;
      selected_pgm_fld = DBG_FLP_CONSISTENT_HASHING_PROGRAM_NUMf;
      selected_cam_line[0] = IHP_DBG_FLP_CONSISTENT_HASHING_PROGRAMr;
      selected_cam_line_fld[0] = DBG_FLP_CONSISTENT_HASHING_PROGRAM_TCAM_LINEf;
      nof_selected_cam_line_regs = 1;
  }
#endif /* BCM_88660_A0 */
  else /* SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP */ {
      selected_pgm = IHP_DBG_FLP_SELECTED_PROGRAMr;
      selected_pgm_fld = DBG_FLP_SELECTED_PROGRAMf;
      selected_cam_line[0] = IHP_DBG_FLP_PROGRAM_SELECTION_CAM_LINEr;
      selected_cam_line_fld[0] = DBG_FLP_PROGRAM_SELECTION_CAM_LINEf;
      nof_selected_cam_line_regs = 1;
  }
#if ARAD_PP_FP_DIAG_LAST_PACKET_ALL_IDENTICALS_NO_STREAM
#else
  /* Get the PMF-Program index, assumption of a single program */
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, selected_pgm, core_id, 0, selected_pgm_fld, &pmf_progs_bmp));
      for (selected_cam_line_ndx = 0; selected_cam_line_ndx < nof_selected_cam_line_regs; selected_cam_line_ndx++) {
          SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  51 + selected_cam_line_ndx,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field_read(unit, selected_cam_line[selected_cam_line_ndx], core_id, 0, selected_cam_line_fld[selected_cam_line_ndx], &pfg_bmp[selected_cam_line_ndx]));
      }
  }

  /* Wait more than 1 sedcond to be sure a packet was sent meanwhile */
  sal_msleep(1100);
#endif

  /* Reset the registers after called */
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  53,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, selected_pgm, core_id, 0, selected_pgm_fld, &pmf_progs_bmp));
  fld_len = soc_reg_field_length(unit, selected_pgm, selected_pgm_fld);
  fld_val = (fld_len == 32) ? 0xffffffff : ((1 << fld_len) - 1); /* field all ones to invalidate it */
#ifdef ARAD_PP_FP_DIAG_LAST_PACKET_RESET_DBG_PMF
  SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  54,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, selected_pgm, core_id, 0, selected_pgm_fld,  fld_val)); 
#endif /* ARAD_PP_FP_DIAG_LAST_PACKET_RESET_DBG_PMF */
  for (selected_cam_line_ndx = 0; selected_cam_line_ndx < nof_selected_cam_line_regs; selected_cam_line_ndx++) {
      SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  55 + selected_cam_line_ndx,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field_read(unit, selected_cam_line[selected_cam_line_ndx], core_id, 0, selected_cam_line_fld[selected_cam_line_ndx], &pfg_bmp[selected_cam_line_ndx]));
  }

  *pmf_progs_bmp_final = pmf_progs_bmp;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_diag_pgm_bmp_get_unsafe()", 0, 0);
}

/* Get the program selection line and the program id according to the register bitmap outputs */
uint32
  arad_pp_fp_packet_diag_pgm_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint32                 pmf_progs_bmp,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE stage,
    SOC_SAND_IN  uint32                 pfg_bmp[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES],
    SOC_SAND_OUT uint32                 *pfg_ndx_final,
    SOC_SAND_OUT uint32                 *pmf_pgm_ndx_final
  )
{
  uint32
    pfg_count,
    pfg_ndx,
    pmf_pgm_count,
    pmf_pgm_ndx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  pfg_count = 0;
  pmf_pgm_count = 0;
  *pmf_pgm_ndx_final = ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 2;
  *pfg_ndx_final = ARAD_PMF_NOF_LINES;

  for (pmf_pgm_ndx = 0; pmf_pgm_ndx <= ARAD_PMF_LOW_LEVEL_PMF_PGM_NDX_MAX + 1; ++pmf_pgm_ndx)
  {
    if (SOC_SAND_GET_BIT(pmf_progs_bmp, pmf_pgm_ndx) == 0x1)
    {
      /* PMF Program found */
      if(!pmf_pgm_count) {
        /* Take the first found program */
        *pmf_pgm_ndx_final = pmf_pgm_ndx;
      }
      pmf_pgm_count ++;
    }
  }

  for (pfg_ndx = 0; pfg_ndx < ARAD_PMF_NOF_LINES; ++pfg_ndx)
  {
    if (((pfg_ndx < 32) && (SOC_SAND_GET_BIT(pfg_bmp[0], pfg_ndx) == 0x1))
        || ((pfg_ndx >= 32) && (SOC_SAND_GET_BIT(pfg_bmp[1], (pfg_ndx-32)) == 0x1)))
    {
      /* PFG found */
      if(!pfg_count) {
        /* Take the first found group */
        *pfg_ndx_final = pfg_ndx;
      }
      pfg_count ++;
    }
  }

  if (((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) ||
      (stage == SOC_PPC_FP_DATABASE_STAGE_EGRESS))
      && (pmf_pgm_count > 1)) {
    /* More than one program in the PMF bitmap, corresponds to more than 1 packet type*/
    LOG_WARN(BSL_LS_SOC_FP,
             (BSL_META_U(unit,
                         "[Unit %d, stage %d] PMF: more than one program selected is present.\n"
                         "WARNING: The below results may be misleading and not represent the last packet.\n"
                         "         For correct diagnostic results, please run a single type of packet.\n"
                         "         Restrict the case from potential learning packets.\n\r"), unit, stage));

  }
  ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_diag_pgm_get_unsafe()", 0, 0);
}

uint32
  arad_pp_fp_packet_diag_buffer_to_value_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE stage,
    SOC_SAND_IN  uint32                 db_id_curr,
    SOC_SAND_IN  uint32                 db_id_ndx,
    SOC_SAND_IN  uint32                 data_in[ARAD_PP_FP_TCAM_ENTRY_SIZE],
    SOC_SAND_IN  uint8                  is_320b,
    SOC_SAND_INOUT SOC_PPC_FP_PACKET_DIAG_INFO *info
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
      qual_index, 
      arr_ndx, 
      mask_in[ARAD_PP_FP_TCAM_ENTRY_SIZE];
  ARAD_PP_IHB_TCAM_BANK_TBL_DATA
      tcam_bank_tbl_data, 
      tcam_bank_tbl_data2;
  SOC_PPC_FP_QUAL_VAL      
      *qual_vals_out = NULL; 

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(&tcam_bank_tbl_data,0x0, sizeof(ARAD_PP_IHB_TCAM_BANK_TBL_DATA));
  sal_memset(&tcam_bank_tbl_data2,0x0, sizeof(ARAD_PP_IHB_TCAM_BANK_TBL_DATA));

  /* Key: map from tcam buffer to key qualifiers */
  SHR_BITCOPY_RANGE(tcam_bank_tbl_data.value, 0, data_in, 0, 160); 
  if (is_320b) { 
      SHR_BITCOPY_RANGE(tcam_bank_tbl_data2.value, 0, data_in, 160, 160); 
      tcam_bank_tbl_data2.valid = 1;
  }

  /* Print the key looked-up: in case the entries field match but no hit: entry not built correctly */
  tcam_bank_tbl_data.valid = 1;
  arad_tbl_access_tcam_print(&tcam_bank_tbl_data);
  if (tcam_bank_tbl_data2.valid) {
      /* Print 320b also */
      arad_tbl_access_tcam_print(&tcam_bank_tbl_data2);
  }

  info->key.db_id_quals[db_id_curr].db_id = db_id_ndx;
  info->key.db_id_quals[db_id_curr].stage = stage;
  for(arr_ndx = 0; arr_ndx < ARAD_PP_FP_TCAM_ENTRY_SIZE; ++arr_ndx) {
      mask_in[arr_ndx] = SOC_SAND_U32_MAX;
  }

  ARAD_ALLOC(qual_vals_out, SOC_PPC_FP_QUAL_VAL, SOC_PPC_FP_NOF_QUALS_PER_DB_MAX, "arad_pp_fp_packet_diag_get_unsafe.qual_vals_out");
  res = arad_pp_fp_key_buffer_to_value(unit, stage, db_id_ndx, data_in, mask_in, qual_vals_out);
  SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);
  for(qual_index = 0; qual_index < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX ; ++qual_index) {
      info->key.db_id_quals[db_id_curr].qual[qual_index].type = qual_vals_out[qual_index].type;
      info->key.db_id_quals[db_id_curr].qual[qual_index].val[0] = qual_vals_out[qual_index].val.arr[0];
      info->key.db_id_quals[db_id_curr].qual[qual_index].val[1] = qual_vals_out[qual_index].val.arr[1];
  }

  ARAD_DO_NOTHING_AND_EXIT;

exit:
  ARAD_FREE(qual_vals_out);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_diag_buffer_to_value_get_unsafe()", 0, 0);
}

/*
* This function reads a value from signal given action type
*/
STATIC uint32 arad_pp_fp_packet_diag_action_to_signal_value_get(
              SOC_SAND_IN  int                            unit,
              SOC_SAND_IN  int                            core,
              SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE         action_type,
              SOC_SAND_OUT  uint32                        *value)
{
    uint32 res = SOC_SAND_OK;
    uint32 size = 0;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;

    if(value == NULL)
    {
        res = SOC_SAND_ERR;
        ARAD_DO_NOTHING_AND_EXIT;
    }
    *value = 0;
    res = arad_pmf_fem_action_width_default_get(unit,action_type,&size);
    switch(action_type)
    {
        case SOC_PPC_FP_ACTION_TYPE_DEST:
        case SOC_PPC_FP_ACTION_TYPE_DEST_DROP:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Dst",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_TC:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","TC",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_DP:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","DP",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_EEI:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","EEI",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_OUTLIF:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Out_LIF",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_TRAP:
            {
                uint32 trap_strength = 0;
                uint32 trap_code = 0;
                uint32 trap_qualifier = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Strength",&trap_strength,3);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","CPU_Trap_Code",&trap_code,8);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","CPU_Trap_Qualifier",&trap_qualifier,16);
                *value = (trap_qualifier << 11) + (trap_strength << 8) + trap_code;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_SNP:
            {
                uint32 snoop_strength = 0;
                uint32 snoop_code = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Snoop_Strength",&snoop_strength,2);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Snoop_Code",&snoop_code,8);
                *value = (snoop_strength<<8) + snoop_code;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_PORT:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_PP_Port",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_STAT:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Statistics_Tag",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VSQ_PTR:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","St_VSQ_Ptr",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_MIRROR:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Mirror_Action",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_MIR_DIS:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Out_Mirror_Disable",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_EXC_SRC:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Exclude_Src_Action",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IS:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Ingress_Shaping_Dst",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_METER_A:
            {
                uint32 update = 0;
                uint32 meter_ptr = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Meter_A_Update",&update,1);
                if(update)
                {
                    res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Meter_A_Ptr",&meter_ptr,17);
                    *value = (update << 17)+meter_ptr;
                }
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_METER_B:
            {
                uint32 update = 0;
                uint32 meter_ptr = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Meter_B_Update",&update,1);
                if(update)
                {
                    res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Meter_B_Ptr",&meter_ptr,17);
                    *value = (update << 17)+meter_ptr;
                }
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_COUNTER_A:
            {
                uint32 update = 0;
                uint32 counter_ptr = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Counter_A_Update",&update,1);
                if(update)
                {
                    res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Counter_A_Ptr",&counter_ptr,21);
                    *value = (update << 21)+counter_ptr;
                }
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_COUNTER_B:
            {
                uint32 update = 0;
                uint32 counter_ptr = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Counter_B_Update",&update,1);
                if(update)
                {
                    res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Counter_B_Ptr",&counter_ptr,21);
                    *value = (update << 21)+counter_ptr;
                }
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_DP_METER_COMMAND:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","DP_Meter_Cmd",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_SRC_SYST_PORT:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Src_System_Port",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_FWD_CODE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Fwd_Code",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_FWD_OFFSET:
            {
                uint32 offset_index;
                uint32 offset_fix;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Fwd_Offset_Index",&offset_index,3);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Fwd_Offset_Fix",&offset_fix,6);
                *value = (offset_index << 6) + offset_fix;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_BYTES_TO_REMOVE:
            {
                uint32 remove_index;
                uint32 remove_fix;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Bytes_to_Remove_Index",&remove_index,2);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","Bytes_to_Remove_Fix",&remove_fix,6);
                *value = (remove_index << 6) + remove_fix;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_SYSTEM_HEADER_PROFILE_ID:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","System_Header_Profile_Index",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VSI:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","VSI_ACL",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_ORIENTATION_IS_HUB:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Orientation_is_Hub",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_COMMAND:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Cmd",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_1:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","VID_1",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_VID_2:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","VID_2",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VLAN_EDIT_PCP_DEI:
            {
                uint32 dei = 0;
                uint32 pcp = 0;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","DEI",&dei,1);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","PCP",&pcp,3);
                *value = (pcp << 3) + dei;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_RIF:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_RIF",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_VRF:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","VRF",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_TTL:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_TTL",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_DSCP_EXP:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_DSCP_EXP",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION_VALID:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","RPF_Dst_Valid",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_RPF_DESTINATION:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","RPF_Dst",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_INGRESS_LEARN_ENABLE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Ingress_Learn_Enable",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_EGRESS_LEARN_ENABLE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Egress_Learn_Enable",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_LEARN_FID:
            {
                uint32 key[2] = {0};
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Learn_Key",key,40);
                *value = ((key[1] & 0xFFFF0000) >> 16);
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_LEARN_SA_0_TO_15:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_LEARN_SA_0_TO_15",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_LEARN_SA_16_TO_47:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_LEARN_SA_16_TO_47",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_0_TO_15:
            {
                uint32 data[2] = {0};
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Learn_Data",data,40);
                *value = data[0] & 0xFFFF;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_LEARN_DATA_16_TO_39:
            {
                uint32 data[2] = {0};
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Learn_Data",data,40);
                *value = ((data[0] & 0xFFFF0000) >> 16) + (data[1] << 16);
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_LEARN_OR_TRANSPLANT:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Learn_or_Transplant",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_LIF:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_LIF",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_ECMP_LB:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","ECMP_LB_Key_Packet_Data",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_LAG_LB:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","LAG_LB_Key_Packet_Data",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_STACK_RT_HIST:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Stacking_Route_History_Bitmap",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IGNORE_CP:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Ignore_CP",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_PPH_TYPE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","PPH_Type",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_PACKET_IS_BOOTP_DHCP:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Packet_is_BOOTP_DHCP",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_UNKNOWN_ADDR:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Unknown_Addr",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_FWD_HDR_ENCAPSULATION:
            if(SOC_IS_JERICHO(unit) == TRUE)
            {
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Fwd_Header_Enc",value,size);
            }
            else
            {
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Fwd_Header_Encapsulation",value,size);
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_IEEE_1588:
            if(SOC_IS_JERICHO(unit) == TRUE)
            {
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","IEEE1588_Enc",value,size);
            }
            else
            {
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","IEEE1588_Encapsulation",value,size);
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_OAM:
            {
                uint32 up_mep = 0;
                uint32 sub_type = 0;
                uint32 stamp_offset = 0;
                uint32 offset;
                res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","OAM_Up_MEP",&up_mep,1);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","OAM_Sub_Type",&sub_type,3);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","OAM_Stamp_Offset",&stamp_offset,7);
                res |= dpp_dsig_read(unit,core,"IRPP","PMF","FER","OAM_Offset",&offset,7);
                *value = (offset << 11) + (stamp_offset << 4) + (sub_type << 1) + up_mep;
            }
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","User_Header_1",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","User_Header_2",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_3:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","User_Header_3",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_4:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","User_Header_4",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_1_TYPE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_USER_HEADER_1_TYPE",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_2_TYPE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_USER_HEADER_2_TYPE",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_3_TYPE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_USER_HEADER_3_TYPE",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_USER_HEADER_4_TYPE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_USER_HEADER_4_TYPE",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_NATIVE_VSI:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_NATIVE_VSI",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_LIF_PROFILE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_LIF_Profile",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_IN_RIF_PROFILE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","In_RIF_Profile",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_ITPP_DELTA:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","ITPP_Delta",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_STATISTICS_POINTER_0:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Statistics_Ptr_0",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_STATISTICS_POINTER_1:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Statistics_Ptr_1",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_LATENCY_FLOW_ID:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Latency_Flow",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_ADMIT_PROFILE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Admt_Profile",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_PEM_CONTEXT:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_PEM_CONTEXT",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_PEM_GENERAL_DATA_0:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_PEM_GENERAL_DATA_0",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_PEM_GENERAL_DATA_1:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_PEM_GENERAL_DATA_1",value,1);
            break;
        case SOC_PPC_FP_ACTION_TYPE_PPH_RESERVE_VALUE:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","PPH_Reserved",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_TRAP_REDUCED",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_INVALID_NEXT",value,size);
            break;
        case SOC_PPC_FP_ACTION_TYPE_NOP:
            res = dpp_dsig_read(unit,core,"IRPP","PMF","FER","Invalid signal ACTION_TYPE_NOP",value,size);
            break;
        /*Invalid Actions*/
        case SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER:
        case SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP:
        case SOC_PPC_FP_ACTION_TYPE_CHANGE_KEY:
        case SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_0:
        case SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_1:
        case SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_2:
        case SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_3:
        case SOC_PPC_FP_ACTION_TYPE_STAGGERED_PRESEL_RESULT_KAPS:
        case SOC_PPC_FP_ACTION_TYPE_USER_PRIORITY:
        case SOC_PPC_FP_ACTION_TYPE_SEQUENCE_NUMBER_TAG:
        default:
            res = SOC_SAND_ERR;
        break;
    }
    
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        
    ARAD_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_diag_action_to_signal_value_get()", 0, 0);
}

/*********************************************************************
*     Get the Field Processing of the last packets.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_packet_diag_get_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  int                 core_id,
    SOC_SAND_OUT SOC_PPC_FP_PACKET_DIAG_INFO *info
  )
{
  uint32
      key_ndx,
      signal_ndx,
      signal_id,
      key_id,
      tbl_data[2],
      access_profile_id,
    db_id_curr = 0,
    pmf_pgm_ndx,
      action_type_hw,
      pfg_bmp[ARAD_PMF_NOF_LINES_MAX_ALL_STAGES],
      pfg_ndx,
    pmf_progs_bmp = 0,
    val[ARAD_PP_DIAG_DBG_VAL_LEN],
    fem_pgm_id,
    res = SOC_SAND_OK;
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO
    rcvd_pkt_info;
  ARAD_PP_DIAG_REG_FIELD
    reg_fld;
  uint32
      is_val,
      stage_ndx, 
      nof_stages,
      qual_index,
      qual_length_no_padding,
      qual_lsb,
      val_lsb,
      pp_port_ndx,
      action_len,
      action_lsb_egress,
      nof_bits_lsb,
      mask_in[ARAD_PP_FP_TCAM_ENTRY_SIZE],
      data_in[ARAD_PP_FP_TCAM_ENTRY_SIZE],
      elk_master_key[ARAD_PP_FP_TCAM_ENTRY_SIZE], /* ELK Master-Key. Assumption not using more than 320b */
      exist_progs,
    fem_macro_ndx = 0,
    cycle_ndx,
    bank_ndx = 0,
    db_id_ndx,
    ind,
      pd_ndx,
      fes_ndx,
      action_ndx,
      action_curr;
    const static uint32 reg_fld_key_sent_arad[ARAD_PP_FP_NOF_PDS][2][4] = 
      {
          /* MSB  LSB  Base */
          {{41,    0, 17, 5},{255, 138, 17, 4}},
          {{131,   0, 17, 4},{255, 228, 17, 3}},
          {{221,  62, 17, 3},{0, 0, 0, 0}},
          {{ 55,   0, 17, 3},{255, 152, 17, 2}},
          {{145,   0, 17, 2},{255, 242, 17, 1}},
          {{235,  76, 17, 1},{0, 0, 0, 0}},
          {{69,    0, 17, 1},{255, 166, 17, 0}},
          {{159,   0, 17, 0},{0, 0, 0, 0}},
      },
   reg_fld_key_sent_jericho[ARAD_PP_FP_NOF_PDS][2][4] = 
      {
          /* MSB  LSB  Base */
          {{41,    0, 5, 5},{255, 138, 5, 4}},
          {{131,   0, 5, 4},{255, 228,  5, 3}},
          {{221,  62, 5, 3},{0, 0, 0, 0}},
          {{ 55,   0, 5, 3},{255, 152, 5, 2}},
          {{145,   0, 5, 2},{255, 242, 5, 1}},
          {{235,  76, 5, 1},{0, 0, 0, 0}},
          {{69,    0, 5, 1},{255, 166, 5, 0}},
          {{159,   0, 5, 0},{0, 0, 0, 0}},
      },
    reg_fld_key_sent_qax[ARAD_PP_FP_NOF_PDS][2][4] =
      {
          /* MSB  LSB  Base */
          {{41,    0, 6, 5},{255, 138, 6, 4}},
          {{131,   0, 6, 4},{255, 228,  6, 3}},
          {{221,  62, 6, 3},{0, 0, 0, 0}},
          {{ 55,   0, 6, 3},{255, 152, 6, 2}},
          {{145,   0, 6, 2},{255, 242, 6, 1}},
          {{235,  76, 6, 1},{0, 0, 0, 0}},
          {{69,    0, 6, 1},{255, 166, 6, 0}},
          {{159,   0, 6, 0},{0, 0, 0, 0}},
      },
    reg_fld_db_profile_arad[ARAD_PP_FP_NOF_PDS][4] = 
      {
          /* MSB  LSB  Base */
          {47, 42, 17, 5},
          {137, 132, 17, 4},
          {227, 222, 17, 3},
          {61,   56, 17, 3},
          {151, 146, 17, 2},
          {241, 236, 17, 1},
          {75,   70, 17, 1},
          {165, 160, 17, 0},
      },
    reg_fld_db_profile_jericho[ARAD_PP_FP_NOF_PDS][4] = 
        {
            /* MSB  LSB  Base */
            {47, 42, 5, 5},
            {137, 132, 5, 4},
            {227, 222, 5, 3},
            {61,   56, 5, 3},
            {151, 146, 5, 2},
            {241, 236, 5, 1},
            {75,   70, 5, 1},
            {165, 160, 5, 0},
        },
    reg_fld_db_profile_qax[ARAD_PP_FP_NOF_PDS][4] =
    {
        /* MSB  LSB  Base */
        {47, 42, 6, 5},
        {137, 132, 6, 4},
        {227, 222, 6, 3},
        {61,   56, 6, 3},
        {151, 146, 6, 2},
        {241, 236, 6, 1},
        {75,   70, 6, 1},
        {165, 160, 6, 0},
    },
    reg_fld_action_match_arad[ARAD_PP_FP_NOF_PDS][4] = 
        {
            /* MSB  LSB  Base1 Base2 */
            {79, 79, 18, 1},
            {37, 37, 18, 1},
            {251, 251,  18, 0},
            {209, 209,  18, 0},
            {167, 167,  18, 0},
            {125, 125,  18, 0},
            {83,   83,  18, 0},
            {41,   41,  18, 0},
        },
    reg_fld_action_match_jericho[ARAD_PP_FP_NOF_PDS][4] = 
        {
            /* MSB  LSB  Base1 Base2 */
            {143, 143, 6, 1},
            { 93,  93, 6, 1},
            { 43,  43, 6, 1},
            {249, 249, 6, 0},
            {199, 199, 6, 0},
            {149, 149, 6, 0},
            { 99,  99, 6, 0},
            { 49,  49, 6, 0},
        },
    reg_fld_action_match_qax[ARAD_PP_FP_NOF_PDS][4] =
    {
        /* MSB  LSB  Base1 Base2 */
        {143, 143, 7, 1},
        { 93,  93, 7, 1},
        { 43,  43, 7, 1},
        {249, 249, 7, 0},
        {199, 199, 7, 0},
        {149, 149, 7, 0},
        { 99,  99, 7, 0},
        { 49,  49, 7, 0},
    },
    reg_fld_action_arad[ARAD_PP_FP_NOF_PDS][2][4] = 
        {
            /* MSB  LSB  Base1 Base2 */
            {{ 78,  39, 18, 1},{0, 0, 0, 0}},
            {{ 36,   0, 18, 1},{255, 253, 18, 0}}, 
            {{250, 211, 18, 0},{0, 0, 0, 0}},
            {{208, 169, 18, 0},{0, 0, 0, 0}},
            {{166, 127, 18, 0},{0, 0, 0, 0}},
            {{124,  85, 18, 0},{0, 0, 0, 0}},
            {{82,   43, 18, 0},{0, 0, 0, 0}},
            {{40,    1, 18, 0},{0, 0, 0, 0}},
        },
    reg_fld_action_jericho[ARAD_PP_FP_NOF_PDS][2][4] = 
        {
            /* MSB  LSB  Base1 Base2 */
            {{142, 95, 6, 1},{0, 0, 0, 0}},
            {{ 92, 45, 6, 1},{0, 0, 0, 0}},
            {{ 42, 0,  6, 1},{255,251,6,0}},
            {{248,201, 6, 0},{0, 0, 0, 0}},
            {{198,151, 6, 0},{0, 0, 0, 0}},
            {{148,101, 6, 0},{0, 0, 0, 0}},
            {{ 98, 51, 6, 0},{0, 0, 0, 0}},
            {{ 48,  1, 6, 0},{0, 0, 0, 0}},
        },
      reg_fld_action_qax[ARAD_PP_FP_NOF_PDS][2][4] = 
      {
          /* MSB  LSB  Base1 Base2 */
          {{142, 95, 7, 1},{0, 0, 0, 0}},
          {{ 92, 45, 7, 1},{0, 0, 0, 0}},
          {{ 42, 0,  7, 1},{255,251,7,0}},
          {{248,201, 7, 0},{0, 0, 0, 0}},
          {{198,151, 7, 0},{0, 0, 0, 0}},
          {{148,101, 7, 0},{0, 0, 0, 0}},
          {{ 98, 51, 7, 0},{0, 0, 0, 0}},
          {{ 48,  1, 7, 0},{0, 0, 0, 0}},
      },
    reg_fld_pgm_selection_egress_arad[ARAD_PP_FP_EGRESS_NOF_CAM_PARAMS][4] = 
          {
              /* MSB  LSB  Base1 Base 2 */
              {48,   41, 1, 0}, /* 0: Out-PP-Port */
              {84,   81, 1, 0}, /* 1: Header-Code */
              {98,   94, 1, 0}, /* 2: Ethernet-Tag-Format */
              {0,   0, 0, 0}, /* 3: Format-Code */
              {0,   0, 0, 0}, /* 4: Value1 */
              {0,   0, 0, 0}, /* 5: Value2 */
              {0,   0, 0, 0}, /* 6: Qualifier */
          },
      reg_fld_pgm_selection_egress_jericho[ARAD_PP_FP_EGRESS_NOF_CAM_PARAMS][4] = 
            {
                /* MSB  LSB  Base1 Base 2 */
                {50,  43, 1, 0}, /* 0: Out-PP-Port */
                {94,  91, 1, 0}, /* 1: Header-Code */
                {108,104, 1, 0}, /* 2: Ethernet-Tag-Format */
                {0,   0, 0, 0}, /* 3: Format-Code */
                {0,   0, 0, 0}, /* 4: Value1 */
                {0,   0, 0, 0}, /* 5: Value2 */
                {0,   0, 0, 0}, /* 6: Qualifier */
            },
      reg_fld_pgm_selection_egress_qax[ARAD_PP_FP_EGRESS_NOF_CAM_PARAMS][4] =
      {
          /* MSB  LSB  Base1 Base 2 */
          {50,  43, 1, 0}, /* 0: Out-PP-Port */
          {94,  91, 1, 0}, /* 1: Header-Code */
          {108,104, 1, 0}, /* 2: Ethernet-Tag-Format */
          {0,   0, 0, 0}, /* 3: Format-Code */
          {0,   0, 0, 0}, /* 4: Value1 */
          {0,   0, 0, 0}, /* 5: Value2 */
          {0,   0, 0, 0}, /* 6: Qualifier */
      },
      reg_fld_flp2egw_key_arad[ARAD_PP_FP_NOF_FLP2EGW_KEY_SIGNALS][4] = 
          {
              /* MSB  LSB  Base1 Base2 */
              {255, 8, 3, 0}, /* 7:0 */
              {255, 0, 3, 1}, 
              {255, 0, 3, 2},
              {255, 0, 3, 3},
              {  7, 0, 3, 4}, /* 1023:1016 */
          },
      reg_fld_flp2egw_key_jericho[ARAD_PP_FP_NOF_FLP2EGW_KEY_SIGNALS][4] = 
          {
              /* MSB  LSB  Base1 Base2 */
              {255, 8, 32, 0}, /* 7:0 */
              {255, 0, 32, 1}, 
              {255, 0, 32, 2},
              {255, 0, 32, 3},
              {  7, 0, 32, 4}, /* 1023:1016 */
          },
      reg_fld_flp2egw_key_qax[ARAD_PP_FP_NOF_FLP2EGW_KEY_SIGNALS][4] =
          {
              /* MSB  LSB  Base1 Base2 */
              {255, 8, 34, 0}, /* 7:0 */
              {255, 0, 34, 1},
              {255, 0, 34, 2},
              {255, 0, 34, 3},
              {  7, 0, 34, 4}, /* 1023:1016 */
          },      
      reg_fld_egw2flp_result_arad[4] = 
          {
             /* MSB  LSB    Base1 Base2  */
                128, 1, 4, 0
          },
       reg_fld_egw2flp_result_jericho[4] = 
          {
                166, 39, 4, 9
          },
      reg_fld_egw2flp_result_qax[4] = 
          {
                231, 104, 4, 9
          },
      reg_fld_pmf2fer_consistent_hashing_lem_key_result_arad[4] = 
          {
             /* MSB  LSB  Base */
                141+47/*hash-flow*/+16/* fec */-1, 141, 9, 11
          },
      reg_fld_pmf2fer_consistent_hashing_lem_key_result_jericho[4] = 
          {
             /* MSB  LSB  Base */
                182+47/*hash-flow*/+18/* fec */-1, 182, 0, 11
          },
      reg_fld_pmf2fer_consistent_hashing_lem_key_result_qax[4] = 
          {
             /* MSB  LSB  Base */
                124+47/*hash-flow*/+18/* fec */-1, 124, 0, 14
          };
  uint32
      action_found[ARAD_PP_FP_NOF_PDS][9]; /* 0 - is found, 1 - access profile, 2/3/4 - action value, 5-8 after shift of 40b */
  ARAD_PP_EGQ_PMF_PROGRAM_SELECTION_CAM_TBL_DATA 
      egress_pmf_cam;
  uint32
    egress_pmf_cam_data[SOC_DPP_IMP_DEFS_MAX(IHB_PMF_PROGRAM_SELECTION_CAM_NOF_LONGS)];
  ARAD_PP_EGQ_PP_PPCT_TBL_DATA
    egq_pp_ppct_tbl;
  ARAD_EGQ_PPCT_TBL_DATA
    egq_ppct_tbl;
  uint8
    cycle_db_id,
      is_320b,
    is_found;
  ARAD_PMF_FEM_NDX
    fem_ndx;
  SOC_PPC_FP_FEM_ENTRY
    fem_entry;
  ARAD_PMF_FEM_INPUT_INFO
    fem_input_info;
  SOC_PPC_FP_DIR_EXTR_ACTION_VAL
    *dir_extr_action_val = NULL;
  SOC_PPC_FP_QUAL_VAL
    qual_val;
  soc_reg_above_64_val_t
      last_fes,
      key_built[SOC_PPC_FP_NOF_CYCLES][ARAD_PMF_LOW_LEVEL_PMF_KEY_MAX_ALL_STAGES + 1];
  const static soc_mem_t
    ihb_pmf_pass_lookup[SOC_PPC_FP_NOF_CYCLES] = {IHB_PMF_PASS_1_LOOKUPm, IHB_PMF_PASS_2_LOOKUPm};
  const static soc_field_t
      fields[ARAD_PMF_LOW_LEVEL_PMF_KEY_MAX_ALL_STAGES + 1 /* Ingress PMF */] = 
        {TCAM_DB_PROFILE_KEY_Af,TCAM_DB_PROFILE_KEY_Bf,TCAM_DB_PROFILE_KEY_Cf,TCAM_DB_PROFILE_KEY_Df};
  const static soc_field_t
      fields_qax[ARAD_PMF_LOW_LEVEL_PMF_KEY_MAX_ALL_STAGES + 1 /* Ingress PMF */] = 
        {TCAM_DB_PROFILE_KEY_0f,TCAM_DB_PROFILE_KEY_1f,TCAM_DB_PROFILE_KEY_2f,TCAM_DB_PROFILE_KEY_3f};
  ARAD_PMF_DB_INFO
     db_info;
  SOC_PPC_FP_DATABASE_INFO
    fp_database_info; 
  ARAD_PMF_CE_IRPP_QUALIFIER_INFO     
      qual_info, qual_info_tmp;
  ARAD_PMF_FEM_ACTION_EGRESS_SIGNAL 
      action_egress_info;
  ARAD_PMF_FES
      fes_info;
  ARAD_PP_IHB_TCAM_BANK_TBL_DATA
      tcam_bank_tbl_data,
      tcam_bank_tbl_data2; /* for 320b entries */
  SOC_PPC_FP_DATABASE_STAGE
        stage; 
  ARAD_PP_IHB_FLP_LOOKUPS_TBL_DATA
    flp_lookups_tbl;
  const static
      SOC_PPC_FP_ACTION_TYPE
        flp_action_types[SOC_PPC_FP_NOF_ELK_ACTIONS] = {
          SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_0, SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_1,
          SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_2, SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_3,
		  SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_4, SOC_PPC_FP_ACTION_TYPE_FLP_ACTION_5};
		
  uint32    ret_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_PACKET_DIAG_GET_UNSAFE);

  SOC_SAND_CHECK_NULL_INPUT(info);

  sal_memset(action_found, 0x0, sizeof(action_found));
  sal_memset(elk_master_key, 0x0, sizeof(elk_master_key));
  sal_memset(&tcam_bank_tbl_data,0x0, sizeof(ARAD_PP_IHB_TCAM_BANK_TBL_DATA));
  sal_memset(&tcam_bank_tbl_data2,0x0, sizeof(ARAD_PP_IHB_TCAM_BANK_TBL_DATA));
  sal_memset(&tbl_data,0x0, sizeof(tbl_data));


  SOC_PPC_FP_PACKET_DIAG_INFO_clear(info);

  /* 
   * FLP: ELK ACLs
   */
  nof_stages = 1; /* FLP */
  if (SOC_IS_ARADPLUS(unit)) {
      nof_stages = 2; /* Also SLB */
  }
  for (stage_ndx = 0; stage_ndx < nof_stages; stage_ndx++) {
      sal_memset(data_in, 0x0, sizeof(data_in));
      db_id_curr = 0;
      stage = (stage_ndx == 0)? SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP: SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB; 
      res = arad_pp_fp_packet_diag_pgm_bmp_get_unsafe(
                unit,
                stage,
                core_id,
                &pmf_progs_bmp,
                pfg_bmp
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 29, exit);

      res = arad_pp_fp_packet_diag_pgm_get_unsafe(
                unit,
                pmf_progs_bmp,
                stage,
                pfg_bmp,
                &pfg_ndx,
                &pmf_pgm_ndx
              );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 31, exit);

      /* Get the PMF Program Selection */
      info->pgm.pfg_id[stage] = pfg_ndx; 
      info->pgm.pgm_id[stage] = pmf_pgm_ndx; 

      /* Go to ingress-PMF if FLP-Program not found */
      if (!pmf_progs_bmp) {
          goto exit_ingress_pmf;
      }

        /* 
       *  Loop on all the Databases
       *  Retrieve the qualifiers values
       */
      for (db_id_ndx = 0; db_id_ndx < SOC_PPC_FP_NOF_DBS; ++db_id_ndx)
      {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(
                unit,
                stage,
                db_id_ndx,
                0,
                &exist_progs
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 160, exit);

        if ((exist_progs & (1 << pmf_pgm_ndx)) == 0)
        {
          continue;
        }

        if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
            /* 
             * Build the data-in, of this database specifically 
             * Assumption here: single database in Key-C 
             * In future (multiple DBs), need to retrieve the KBP configuration 
             * to select the specific Database key 
             *  
             * Order to retrieve the database key sent: 
             * 1. Rebuild the 1K master key (flp2egw_key) 
             * 2. Retrieve the size of A and B sent on this key 
             * 3. Take only the Key-C 
             */
            qual_lsb = ARAD_PP_FP_TCAM_ENTRY_SIZE * 32;
            for (signal_ndx = 0; (signal_ndx < ARAD_PP_FP_NOF_FLP2EGW_KEY_SIGNALS) && (qual_lsb != 0); ++signal_ndx)
            {
                signal_id = ARAD_PP_FP_NOF_FLP2EGW_KEY_SIGNALS - signal_ndx - 1;
                /* Get pmf2tcam_db_profile_index value */
                sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
                if (SOC_IS_JERICHO_PLUS(unit)) {
                    ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_flp2egw_key_qax[signal_id]);
                } else {
                    ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_flp2egw_key_jericho[signal_id], reg_fld_flp2egw_key_arad[signal_id]); 
                }
                res = arad_pp_diag_dbg_val_get_unsafe(
                        unit,
                        core_id,
                        SOC_IS_JERICHO(unit)? ARAD_IHP_ID: ARAD_IHB_ID,
                        &reg_fld,
                        val
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 15 + signal_ndx, exit);
                nof_bits_lsb = reg_fld.msb - reg_fld.lsb + 1;
                /* Verify if not end of buffer */
                nof_bits_lsb = (qual_lsb > nof_bits_lsb)? nof_bits_lsb: qual_lsb;
                val_lsb = (qual_lsb > nof_bits_lsb)? 0: (nof_bits_lsb - qual_lsb);
                qual_lsb = (qual_lsb > nof_bits_lsb)? (qual_lsb - nof_bits_lsb): 0;
                SHR_BITCOPY_RANGE(elk_master_key, qual_lsb, val, val_lsb, nof_bits_lsb);
            }

            /* Extract Key-C */
            res = arad_pp_ihb_flp_lookups_tbl_get_unsafe(unit, pmf_pgm_ndx, &flp_lookups_tbl);
            SOC_SAND_CHECK_FUNC_RESULT(res, 26, exit);
            nof_bits_lsb = flp_lookups_tbl.elk_key_c_valid_bytes * 8 /* in bits */;
            val_lsb = (ARAD_PP_FP_TCAM_ENTRY_SIZE * 32) - (8 * 
                         (flp_lookups_tbl.elk_key_a_valid_bytes + flp_lookups_tbl.elk_key_b_valid_bytes + flp_lookups_tbl.elk_key_c_valid_bytes));
            SHR_BITCOPY_RANGE(data_in, 0, elk_master_key, val_lsb, nof_bits_lsb);
        }
        else { /* SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB */
            /* Extract the final key - no debug mode for the initial value */
            SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
            res = arad_pp_fp_database_get_unsafe(
                    unit,
                    db_id_ndx,
                    &fp_database_info
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 55, exit);
            if (fp_database_info.action_types[0] == SOC_PPC_FP_ACTION_TYPE_SLB_HASH_VALUE) {
                /* Initial stage, continue */
                continue;
            }
            sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
            if (SOC_IS_JERICHO_PLUS(unit)) {
                ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_pmf2fer_consistent_hashing_lem_key_result_qax);
            } else {            
                ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_pmf2fer_consistent_hashing_lem_key_result_jericho, reg_fld_pmf2fer_consistent_hashing_lem_key_result_arad); 
            }
            res = arad_pp_diag_dbg_val_get_unsafe(
                    unit,
                    core_id,
                    ARAD_IHB_ID,
                    &reg_fld,
                    val
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 56, exit);
            SHR_BITCOPY_RANGE(data_in, 0, val, 0, reg_fld.msb - reg_fld.lsb + 1);
        }

        res = arad_pp_fp_packet_diag_buffer_to_value_get_unsafe(
              unit,
              stage,
              db_id_curr,
              db_id_ndx,
              data_in,
              FALSE /* is_320b */, 
              info
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 57, exit);
        info->valid_stage[stage] = TRUE;
        db_id_curr++;
      }

      if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB) {
          /* No action */
          continue;
      }
      /* 
       * Extract action values: 
       * 1. Get the ELK lookup result 
       * 2. Translate it into hit bits and action values 
       * Format: 8b lookup hits, then 120b results. 
       */
      sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
      if (SOC_IS_JERICHO_PLUS(unit)) {
          ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_egw2flp_result_qax);
      } else {
          ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_egw2flp_result_jericho, reg_fld_egw2flp_result_arad)
      }     
      res = arad_pp_diag_dbg_val_get_unsafe(
              unit,
              core_id,
              SOC_IS_JERICHO(unit)? ARAD_IHP_ID: ARAD_IHB_ID,
              &reg_fld,
              val
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 59, exit);

      action_curr = 0;
      for (action_ndx = 0; action_ndx < SOC_PPC_FP_NOF_ELK_ACTIONS; action_ndx++) {
          /* Get the value */
          res = arad_pmf_db_fes_action_size_get_unsafe(
                    unit,
                    flp_action_types[action_ndx],
                    stage,
                    &action_len,
                    &action_lsb_egress
                  );
          SOC_SAND_CHECK_FUNC_RESULT(res, 61, exit);

          info->elk_action[action_curr].action.type = flp_action_types[action_ndx]; 
          SHR_BITCOPY_RANGE(&(info->elk_action[action_curr].action.val), 0, val, action_lsb_egress, action_len); 
          SHR_BITCOPY_RANGE(&(info->elk_action[action_curr].hit), 0, val, (127 - action_ndx), 1); 

          action_curr++;
      }
  }

exit_ingress_pmf:
  /*
   * 1. Parser
   */
  SOC_PPC_DIAG_RECEIVED_PACKET_INFO_clear(&rcvd_pkt_info);
  res = arad_pp_diag_received_packet_info_get_unsafe(
          unit,
          core_id,
          &rcvd_pkt_info,
          &ret_val
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  info->parser.tm_port = rcvd_pkt_info.in_tm_port;
  info->parser.pp_port = rcvd_pkt_info.in_pp_port;
  info->parser.header_type = rcvd_pkt_info.pp_context;

  /* Get the Packet format code */
  sal_memset(val,0x0,ARAD_PP_DIAG_DBG_VAL_LEN * 4);
  reg_fld.msb = 66;
  reg_fld.lsb = 61;
  reg_fld.base = 65538;
  res = arad_pp_diag_dbg_val_get_unsafe(
          unit,
          core_id,
          ARAD_IHP_ID,
          &reg_fld,
          val
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  info->parser.pfc_hw = val[0];

  /* Get the VLAN Structure */
  sal_memset(val,0x0,ARAD_PP_DIAG_DBG_VAL_LEN * 4);
#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS(unit)) {
      reg_fld.msb = 42;
      reg_fld.lsb = 39;
      reg_fld.base = 7;
  }
  else
#endif /* BCM_88660_A0 */
  {
      reg_fld.msb = 149;
      reg_fld.lsb = 146;
      reg_fld.base = 6;
  }
  res = arad_pp_diag_dbg_val_get_unsafe(
          unit,
          core_id,
          ARAD_IHB_ID,
          &reg_fld,
          val
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
  info->parser.vlan_tag_structure = val[0];

  /*
   * 2. PMF-Program
   */
  stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF; 
  res = arad_pp_fp_packet_diag_pgm_bmp_get_unsafe(
            unit,
            stage,
            core_id,
            &pmf_progs_bmp,
            pfg_bmp
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);


  res = arad_pp_fp_packet_diag_pgm_get_unsafe(
            unit,
            pmf_progs_bmp,
            stage,
            pfg_bmp,
            &pfg_ndx,
            &pmf_pgm_ndx
          );
  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 31, exit);

  /* Get the PMF Program Selection */
  info->pgm.pfg_id[stage] = pfg_ndx; 
  info->pgm.pgm_id[stage] = pmf_pgm_ndx; 

  /* Go to egress if PMF-Program not found */
  if (!pmf_progs_bmp) {
      goto exit_egress;
  }

  /*
   * 3. CE
   */
  /* 
   * Get the raw keys: no simple access to the 1st cycle keys, but these 
   * keys cannot access directly to the FES: only TCAM and direct 
   * lookup databases - take the relevant key from the signals in 
   * case the TCAM DB Profile is set 
   */
  /* Zero the key */
  for (key_ndx = 0; key_ndx < ARAD_PMF_LOW_LEVEL_NOF_KEYS; ++key_ndx)
  {
      for (cycle_ndx = 0; cycle_ndx < SOC_PPC_FP_NOF_CYCLES; ++cycle_ndx)
      {
          SOC_REG_ABOVE_64_CLEAR(key_built[cycle_ndx][key_ndx]);
      }
  }
  for (cycle_ndx = 0; cycle_ndx < SOC_PPC_FP_NOF_CYCLES; ++cycle_ndx)
  {
    /* 1st cycle, only when TCAM keys are set */
    res = soc_mem_read(
            unit,
            ihb_pmf_pass_lookup[cycle_ndx],
            MEM_BLOCK_ANY,
            pmf_pgm_ndx, /* line */
            &tbl_data
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 59, exit);

    for (key_ndx = 0; key_ndx < ARAD_PMF_LOW_LEVEL_NOF_KEYS; ++key_ndx)
    {
        access_profile_id = soc_mem_field32_get(unit, ihb_pmf_pass_lookup[cycle_ndx], &tbl_data, (SOC_IS_JERICHO_PLUS(unit) ? fields_qax[key_ndx]:fields[key_ndx]));
        /* See if one of the keys in the signals have this DB-Profile */
        pd_ndx = key_ndx + (cycle_ndx * ARAD_PMF_LOW_LEVEL_NOF_KEYS);
        /* for (pd_ndx = 0; pd_ndx < ARAD_PP_FP_NOF_PDS; ++pd_ndx) */
        {
            /* Get pmf2tcam_db_profile_index value */
            sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
            if (SOC_IS_QAX(unit)) {
                ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_db_profile_qax[pd_ndx]);
            } else {
                ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_db_profile_jericho[pd_ndx], reg_fld_db_profile_arad[pd_ndx]); 
            }
            res = arad_pp_diag_dbg_val_get_unsafe(
                    unit,
                    core_id,
                    ARAD_IHB_ID,
                    &reg_fld,
                    val
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
            if (access_profile_id == val[0]) {
                /* Lookup found */
                /* Copy the LSB key */
                nof_bits_lsb = 0;
                if (SOC_IS_QAX(unit)) {
                    is_val = (reg_fld_key_sent_qax[pd_ndx][1][0] != 0);
                } else if (SOC_IS_JERICHO(unit)) {
                    is_val = (reg_fld_key_sent_jericho[pd_ndx][1][0] != 0);
                } else {
                    is_val = (reg_fld_key_sent_arad[pd_ndx][1][0] != 0);
                }
                if (is_val) {
                    /* LSB signal exists */
                    sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
                    if (SOC_IS_QAX(unit)) {
                        ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_key_sent_qax[pd_ndx][1]);
                    } else {
                        ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_key_sent_jericho[pd_ndx][1], reg_fld_key_sent_arad[pd_ndx][1]); 
                    }
                    res = arad_pp_diag_dbg_val_get_unsafe(
                            unit,
                            core_id,
                            ARAD_IHB_ID,
                            &reg_fld,
                            val
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
                    nof_bits_lsb = reg_fld.msb - reg_fld.lsb + 1;
                  
                    SHR_BITCOPY_RANGE(key_built[cycle_ndx][key_ndx], 0, val, 0, nof_bits_lsb);
                }
                /* Copy the MSB key */
                sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
                if (SOC_IS_QAX(unit)) {
                    ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_key_sent_qax[pd_ndx][0]);
                } else {
                    ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_key_sent_jericho[pd_ndx][0], reg_fld_key_sent_arad[pd_ndx][0]); 
                }
                res = arad_pp_diag_dbg_val_get_unsafe(
                        unit,
                        core_id,
                        ARAD_IHB_ID,
                        &reg_fld,
                        val
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

                SHR_BITCOPY_RANGE(key_built[cycle_ndx][key_ndx], nof_bits_lsb, val, 0, (reg_fld.msb - reg_fld.lsb + 1));

                /* Action: see if match and the action value */
                sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
                if (SOC_IS_QAX(unit)) {
                    ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_action_match_qax[pd_ndx]);
                } else {
                    ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_action_match_jericho[pd_ndx], reg_fld_action_match_arad[pd_ndx]); 
                }
                res = arad_pp_diag_dbg_val_get_unsafe(
                        unit,
                        core_id,
                        ARAD_IHB_ID,
                        &reg_fld,
                        val
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
                SHR_BITCOPY_RANGE(&(action_found[pd_ndx][0]), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1));
                if (action_found[pd_ndx][0] != 0) { 
                    /* Entry matched */
                    nof_bits_lsb = 0;
                    if (SOC_IS_QAX(unit)) {
                        is_val = (reg_fld_action_qax[pd_ndx][1][0] != 0);
                    } else if (SOC_IS_JERICHO(unit)) {
                        is_val = (reg_fld_action_jericho[pd_ndx][1][0] != 0);
                    } else {
                        is_val = (reg_fld_action_arad[pd_ndx][1][0] != 0);
                    }
                    if (is_val) {
                        /* LSB signal exists */
                        sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
                        if (SOC_IS_JERICHO_PLUS(unit)) {
                            ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_action_qax[pd_ndx][1]);
                        } else {
                            ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_action_jericho[pd_ndx][1], reg_fld_action_arad[pd_ndx][1]); 
                        }
                        res = arad_pp_diag_dbg_val_get_unsafe(
                                unit,
                                core_id,
                                ARAD_IHB_ID,
                                &reg_fld,
                                val
                              );
                        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
                        nof_bits_lsb = reg_fld.msb - reg_fld.lsb + 1;

                        SHR_BITCOPY_RANGE(&(action_found[pd_ndx][2]), 0, val, 0, nof_bits_lsb);
                        SHR_BITCOPY_RANGE(&(action_found[pd_ndx][5]), (SOC_DPP_DEFS_GET(unit, tcam_action_width) * 2), val, 0, nof_bits_lsb);
                    }

                    sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
                    if (SOC_IS_QAX(unit)) {
                        ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_action_qax[pd_ndx][0]);
                    } else {
                        ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_action_jericho[pd_ndx][0], reg_fld_action_arad[pd_ndx][0]); 
                    }                    
                    res = arad_pp_diag_dbg_val_get_unsafe(
                            unit,
                            core_id,
                            ARAD_IHB_ID,
                            &reg_fld,
                            val
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
                    action_found[pd_ndx][1] = access_profile_id;
                    SHR_BITCOPY_RANGE(&(action_found[pd_ndx][2]), nof_bits_lsb, val, 0, (reg_fld.msb - reg_fld.lsb + 1));
                    SHR_BITCOPY_RANGE(&(action_found[pd_ndx][5]), (SOC_DPP_DEFS_GET(unit, tcam_action_width) * 2) + nof_bits_lsb, val, 0, (reg_fld.msb - reg_fld.lsb + 1));
                }
            }
        }
    }
  }

  /* 2nd cycle */
  if (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 120, exit, READ_IHB_DBG_LAST_KEY_Ar(unit, REG_PORT_ANY, key_built[1][0]));
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 130, exit, READ_IHB_DBG_LAST_KEY_Br(unit, REG_PORT_ANY, key_built[1][1]));
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 140, exit, READ_IHB_DBG_LAST_KEY_Cr(unit, REG_PORT_ANY, key_built[1][2]));
      SOC_SAND_SOC_IF_ERROR_RETURN(res, 150, exit, READ_IHB_DBG_LAST_KEY_Dr(unit, REG_PORT_ANY, key_built[1][3]));
  }

  /* 
   *  Loop on all the Databases
   *  Retrieve the qualifiers values
   */
  for (db_id_ndx = 0; db_id_ndx < SOC_PPC_FP_NOF_DBS; ++db_id_ndx)
  {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(
                unit,
                stage,
                db_id_ndx,
                0,
                &exist_progs
              );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 160, exit);

    if ((exist_progs & (1 << pmf_pgm_ndx)) == 0)
    {
      continue;
    }

    /*
     * Get the Database key and cycle through SW DB
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
            unit,
            stage,
            db_id_ndx,
            &db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 170, exit);

    cycle_db_id = SHR_BITGET(&db_info.prog_used_cycle_bmp, pmf_pgm_ndx) ? 1 : 0;
    key_ndx = db_info.used_key[pmf_pgm_ndx][0];

    /* Key: map from tcam buffer to key qualifiers */
    SHR_BITCOPY_RANGE(data_in, 0, key_built[cycle_db_id][key_ndx], 0, 160); 
    is_320b = (db_info.used_key[pmf_pgm_ndx][1] == db_info.used_key[pmf_pgm_ndx][0] + 1)? 1: 0; 
    if (is_320b) { 
        key_ndx = db_info.used_key[pmf_pgm_ndx][1];
        SHR_BITCOPY_RANGE(data_in, 160, key_built[cycle_db_id][key_ndx], 0, 160); 
    }

    res = arad_pp_fp_packet_diag_buffer_to_value_get_unsafe(
            unit,
            stage,
            db_id_curr,
            db_id_ndx,
            data_in,
            is_320b, 
            info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);

     
    /*
     * 4. TCAM
     */
    res = soc_mem_read(
            unit,
            ihb_pmf_pass_lookup[cycle_db_id],
            MEM_BLOCK_ANY,
            pmf_pgm_ndx, /* line */
            &tbl_data
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 190, exit);

    for (key_id = 0; key_id < 2; key_id++) {
        if (key_id == 1) {
            if (!is_320b) { 
                /* Not a double-key Database */
                continue;
            }
        }
        key_ndx = db_info.used_key[pmf_pgm_ndx][key_id];
        access_profile_id = soc_mem_field32_get(unit, ihb_pmf_pass_lookup[cycle_db_id], &tbl_data, (SOC_IS_JERICHO_PLUS(unit) ? fields_qax[key_ndx]:fields[key_ndx]));
        for (pd_ndx = 0; pd_ndx < ARAD_PP_FP_NOF_PDS; ++pd_ndx)
        {
            if (action_found[pd_ndx][1] == access_profile_id) {
                /* TCAM Action found */
                if (action_found[pd_ndx][0]) {
                    /* Simple mapping between PD 0..7 and cycle-bank to conserve the PetraB defintion (same total) */
                    cycle_ndx = pd_ndx / SOC_TMC_TCAM_NOF_BANKS;
                    bank_ndx = pd_ndx % SOC_TMC_TCAM_NOF_BANKS;
                    /* Parse the action buffer */
                    info->tcam[cycle_ndx][bank_ndx].is_match = action_found[pd_ndx][0]; /* Is-Match */
                    info->tcam[cycle_ndx][bank_ndx].db_id = db_id_ndx; 
                    res = arad_pp_fp_action_buffer_to_value(
                            unit, 
                            db_id_ndx, 
                            (key_id == 0)? &(action_found[pd_ndx][2]) : &(action_found[pd_ndx][5]), 
                            info->tcam[cycle_ndx][bank_ndx].actions
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit);

                    /* Remove any Change-Key action from the results - parsing problem since there is no valid bit */
                    if (key_id)
                    {
                      for (ind = 0; ind < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++ind) 
                      {
                            if (ARAD_PP_FP_FEM_IS_ACTION_NOT_REQUIRE_FEM(info->tcam[cycle_ndx][bank_ndx].actions[ind].type))
                            {
                              SOC_PPC_FP_ACTION_VAL_clear(&info->tcam[cycle_ndx][bank_ndx].actions[ind]);
                            }
                      }
                    }
                }
                break;
            }
        }
    }

    db_id_curr ++;
  }
  /*In J+/QAX the LAST FEM/FES is corrupted so we read the action types from signals (tcam) */
  if(SOC_IS_JERICHO_PLUS(unit) == TRUE) {
      for (cycle_ndx = 0; cycle_ndx < SOC_TMC_TCAM_NOF_CYCLES; cycle_ndx++) {
          fem_macro_ndx = 0;
          for (bank_ndx = 0; bank_ndx < SOC_TMC_TCAM_NOF_BANKS; bank_ndx++) {
              for (ind = 0; ind < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++ind) {
                  if((info->tcam[cycle_ndx][bank_ndx].actions[ind].type != SOC_PPC_FP_ACTION_TYPE_INVALID) &&
                      (!ARAD_PP_FP_FEM_IS_ACTION_NOT_REQUIRE_FEM(info->tcam[cycle_ndx][bank_ndx].actions[ind].type))) {
                      info->macro_simple[cycle_ndx][fem_macro_ndx].action.type =
                        info->tcam[cycle_ndx][bank_ndx].actions[ind].type;
                      fem_macro_ndx++;
                      if(fem_macro_ndx == SOC_PPC_FP_NOF_MACRO_SIMPLES) {
                          SOC_SAND_CHECK_FUNC_RESULT(SOC_SAND_ERR, 10, exit); 
                      }
                  }
              }
          }
      }
  }

  /*
   * 6. FEM - FES
   */

  /* Loop on all the FESes */
  /*On J+/QAX Debug FEM/FES does not work, so the instruction type is read while Phase 4 TCAM and the value will be read from signals
        all actions will be held in info->macro_simple*/
  if(SOC_IS_JERICHO_PLUS(unit) == TRUE) {
       for (cycle_ndx = 0; cycle_ndx < ARAD_PMF_NOF_CYCLES; ++cycle_ndx)
       {
            for (fem_macro_ndx = 0; fem_macro_ndx < SOC_PPC_FP_NOF_MACRO_SIMPLES; ++fem_macro_ndx)
            {
                if(info->macro_simple[cycle_ndx][fem_macro_ndx].action.type != SOC_PPC_FP_ACTION_TYPE_INVALID)
                {
                    res = arad_pp_fp_packet_diag_action_to_signal_value_get(unit,0,
                                    info->macro_simple[cycle_ndx][fem_macro_ndx].action.type,
                                   &(info->macro_simple[cycle_ndx][fem_macro_ndx].action.val));
                    SOC_SAND_CHECK_FUNC_RESULT(res, 310, exit); 
                }
            }
       }
       
  }
  else
  {
  for (cycle_ndx = 0; cycle_ndx < ARAD_PMF_NOF_CYCLES; ++cycle_ndx)
  {
      for (fem_macro_ndx = 0; fem_macro_ndx < SOC_PPC_FP_NOF_MACRO_SIMPLES; ++fem_macro_ndx)
      {
          fes_ndx = fem_macro_ndx + (cycle_ndx * SOC_PPC_FP_NOF_MACRO_SIMPLES);
          /* 
           * Get the last FES: extract from the tables
           */
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.get(
                    unit,
                    stage,
                    pmf_pgm_ndx,
                    fes_ndx,
                    &fes_info
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 290, exit);

          /* See if this FES is configured to perform some action */
          if (fes_info.is_used == TRUE) {
              info->macro_simple[cycle_ndx][fem_macro_ndx].db_id = fes_info.db_id;
              /* Read the table and see which action has been done */
              res = READ_IHB_DBG_LAST_FESm(unit, IHB_BLOCK(unit, core_id), fes_ndx, last_fes);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 300, exit);
              action_type_hw = soc_IHB_DBG_LAST_FESm_field32_get(unit, last_fes, OUT_ACTION_TYPEf);
              
              res = arad_pmf_db_action_type_get_unsafe(
                        unit,
                        action_type_hw,
                        &is_found,
                        &(info->macro_simple[cycle_ndx][fem_macro_ndx].action.type)
                      );
              SOC_SAND_CHECK_FUNC_RESULT(res, 310, exit);

              /* Get the action value after masking by the action size */
              if (info->macro_simple[cycle_ndx][fem_macro_ndx].action.type != SOC_PPC_FP_ACTION_TYPE_NOP) {
                  res = arad_pmf_db_fes_action_size_get_unsafe(
                          unit,
                          info->macro_simple[cycle_ndx][fem_macro_ndx].action.type,
                          stage,
                          &action_len,
                          &action_lsb_egress
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 320, exit);

                  if (action_len != 0) { /* Only the invalid-next can be zero */
                      info->macro_simple[cycle_ndx][fem_macro_ndx].action.val =  
                          soc_IHB_DBG_LAST_FESm_field32_get(unit, last_fes, OUT_ACTION_VALUEf)
                          & ((1 << action_len) - 1);
                  }
              }
          }
      }
  }

  ARAD_ALLOC(dir_extr_action_val, SOC_PPC_FP_DIR_EXTR_ACTION_VAL, 1, "arad_pp_fp_packet_diag_get_unsafe.dir_extr_action_val")
  /* using the macro SOC_DPP_DEFS_MAX can compare definitions of the same value */
  /* coverity[same_on_both_sides] */
  for (cycle_ndx = 0; cycle_ndx < ARAD_PMF_NOF_CYCLES; ++cycle_ndx)
  {
    for (fem_macro_ndx = 0; fem_macro_ndx < SOC_PPC_FP_NOF_MACROS; ++fem_macro_ndx)
    {
      /*
       * Get the source
       */
      ARAD_PMF_FEM_INPUT_INFO_clear(&fem_input_info);
      fes_ndx = fem_macro_ndx + (cycle_ndx * SOC_PPC_FP_NOF_MACROS);
      res = arad_pmf_db_fem_input_get_unsafe(
              unit,
              pmf_pgm_ndx,
              FALSE, /* is_fes */
              fes_ndx,
              &fem_input_info
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 330, exit);

       /* Read the table and see which action has been done */
      res = READ_IHB_DBG_LAST_FEMm(unit, IHB_BLOCK(unit, core_id), fes_ndx, last_fes);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 340, exit);
      action_type_hw = soc_IHB_DBG_LAST_FEMm_field32_get(unit, last_fes, OUT_ACTION_TYPEf);

      fem_pgm_id = soc_IHB_DBG_LAST_FEMm_field32_get(unit, last_fes, IN_PROGRAMf);

      /* Get the DB-Id [and Entry-Id] */
      ARAD_PMF_FEM_NDX_clear(&fem_ndx);
      fem_ndx.cycle_ndx = cycle_ndx;
      fem_ndx.id = fes_ndx;
      SOC_PPC_FP_FEM_ENTRY_clear(&fem_entry);
      SOC_PPC_FP_DIR_EXTR_ACTION_VAL_clear(dir_extr_action_val);
      SOC_PPC_FP_QUAL_VAL_clear(&qual_val);
      res = arad_pp_fp_fem_configuration_de_get(
              unit,
              fem_macro_ndx,
              cycle_ndx,
              fem_pgm_id,
              &fem_entry,
              dir_extr_action_val,
              &qual_val
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit);

      info->macro[cycle_ndx][fem_macro_ndx].db_id = fem_entry.db_id;
      SOC_PPC_FP_DATABASE_INFO_clear(&fp_database_info);
      res = arad_pp_fp_database_get_unsafe(
              unit,
              fem_entry.db_id,
              &fp_database_info
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 360, exit);

      if (fem_entry.is_for_entry == TRUE)
      {
        info->macro[cycle_ndx][fem_macro_ndx].entry_id = fem_entry.entry_id;
        /* Get the Qualifier mask in this case*/
        info->macro[cycle_ndx][fem_macro_ndx].qual_mask.type = qual_val.type;
        if ((qual_val.type != SOC_PPC_NOF_FP_QUAL_TYPES) && ((qual_val.type != BCM_FIELD_ENTRY_INVALID))) {
            /* Extract the value from the input */
            res = arad_pp_fp_qual_lsb_and_length_get(
                    unit,
                    fem_entry.db_id,
                    qual_val.type,
                    &qual_lsb,
                    &qual_length_no_padding
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);
            info->macro[cycle_ndx][fem_macro_ndx].qual_mask.val[0] = 
                SOC_SAND_GET_BITS_RANGE(soc_IHB_DBG_LAST_FEMm_field32_get(unit, last_fes, IN_KEYf), 
                                        qual_lsb + qual_length_no_padding - 1, 
                                        qual_lsb);
        }
      }

      /* Get the action */
      
      res = arad_pmf_db_action_type_get_unsafe(
                unit,
                action_type_hw,
                &is_found,
                &(info->macro[cycle_ndx][fem_macro_ndx].action.type)
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 380, exit);
      if (!is_found)
      {
          /* Invalid action */
        info->macro[cycle_ndx][fem_macro_ndx].action.type = SOC_PPC_FP_ACTION_TYPE_NOP;
        info->macro[cycle_ndx][fem_macro_ndx].action.val = 0;
      }
      else {
          info->macro[cycle_ndx][fem_macro_ndx].action.val = soc_IHB_DBG_LAST_FEMm_field32_get(unit, last_fes, OUT_ACTION_VALUEf);
      }
    }
  }
  }

exit_egress:
  /* 
   *  7. Egress - Egress Program selection
   *  No signal, so need to simulate the HW
   */
  stage = SOC_PPC_FP_DATABASE_STAGE_EGRESS;
  /* Collect all the signals to get the inputs to the Program selection CAM */
  ARAD_CLEAR(&egress_pmf_cam, ARAD_PP_EGQ_PMF_PROGRAM_SELECTION_CAM_TBL_DATA, 1);
  for (qual_index = 0; qual_index < ARAD_PP_FP_EGRESS_NOF_CAM_PARAMS; qual_index++) {
      /* Get the signal */
      sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
      if (SOC_IS_JERICHO_PLUS(unit)) {
          ARAD_PP_FP_DIAG_FLD_FILL(&reg_fld, reg_fld_pgm_selection_egress_qax[qual_index]);
      } else {
          ARAD_PP_FP_DIAG_FLD_FILL_UNIT(&reg_fld, reg_fld_pgm_selection_egress_jericho[qual_index], reg_fld_pgm_selection_egress_arad[qual_index]); 
      }      
      if (reg_fld.base != 0) {
          res = arad_pp_diag_dbg_val_get_unsafe(
                  unit,
                  core_id,
                  ARAD_EGQ_ID,
                  &reg_fld,
                  val
                );
          SOC_SAND_CHECK_FUNC_RESULT(res, 390, exit);
          /* Copy the value */
          switch (qual_index) {
          case 0:
              /* In-PP-Port - get its profile */
              pp_port_ndx = 0;
              SHR_BITCOPY_RANGE(&pp_port_ndx, 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1)); /* Get the PP-Port */
              res = arad_pp_egq_pp_ppct_tbl_get_unsafe(unit, core_id, pp_port_ndx, &egq_pp_ppct_tbl);
              SOC_SAND_CHECK_FUNC_RESULT(res, 400, exit);
              egress_pmf_cam.egress_pmf_profile = egq_pp_ppct_tbl.pmf_profile;
              break;
          case 1:
              /* Header code */
              SHR_BITCOPY_RANGE(&(egress_pmf_cam.header_code), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1)); 
              break;
          case 2:
              /* Ethernet-Tag-Format */
              SHR_BITCOPY_RANGE(&(egress_pmf_cam.ethernet_tag_format), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1)); 
              break;
          default:
              break;
          }
      }
  }

  /* Transform the table to a buffer */
  res = arad_pp_egq_pmf_program_selection_cam_tbl_data_set_unsafe(
            unit,
            &egress_pmf_cam,
            egress_pmf_cam_data
          );
  SOC_SAND_CHECK_FUNC_RESULT(res, 410, exit);
  
  /* Scan all the tabe lines and find the first hit */
  is_found = FALSE;
  for (pfg_ndx = 0; pfg_ndx < ARAD_PMF_NOF_LINES; pfg_ndx++) {
      /* See if match */
      sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
      sal_memset(data_in, 0x0, ARAD_PP_FP_TCAM_ENTRY_SIZE * 4);
      sal_memset(mask_in, 0x0, ARAD_PP_FP_TCAM_ENTRY_SIZE * 4);
      res = READ_EGQ_PMF_PROGRAM_SELECTION_CAMm(unit, EGQ_BLOCK(unit, core_id), pfg_ndx, val);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 420, exit);
      SHR_BITCOPY_RANGE(data_in, 0, val, 0, ARAD_PMF_SEL_HW_DATA_NOF_BITS); 
      SHR_BITCOPY_RANGE(mask_in, 0, val, ARAD_PMF_SEL_HW_DATA_NOF_BITS, ARAD_PMF_SEL_HW_DATA_NOF_BITS); 

      if (soc_mem_field32_get(unit, EGQ_PMF_PROGRAM_SELECTION_CAMm, val, VALIDf) == 0) {
          /* Line not valid, continue the scan */
          continue;
      }

      /* Use not-mask since 1 means do not care */
      if (((data_in[0] & (~mask_in[0])) ==  (egress_pmf_cam_data[0] & (~mask_in[0])))
          && ((data_in[1] & (~mask_in[1])) ==  (egress_pmf_cam_data[1] & (~mask_in[1])))) {
          is_found = TRUE;
          pmf_pgm_ndx = soc_mem_field32_get(unit, EGQ_PMF_PROGRAM_SELECTION_CAMm, val, PROGRAMf);
          break;
      }
  }
  if (!is_found) {
      pmf_pgm_ndx = 0; /* Default PMF-Program */
  }
  info->pgm.pfg_id[stage] = pfg_ndx; 
  info->pgm.pgm_id[stage] = pmf_pgm_ndx; 

  /* Get the Key value by reading the correct signals */
  /* 
   *  Loop on all the Databases
   *  Retrieve the qualifiers values
   */
  for (db_id_ndx = 0; db_id_ndx < SOC_PPC_FP_NOF_DBS; ++db_id_ndx)
  {
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.progs.get(
                unit,
                stage,
                db_id_ndx,
                0,
                &exist_progs
              );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 430, exit);
    if ((exist_progs & (1 << pmf_pgm_ndx)) == 0)
    {
      continue;
    }

    /*
     * Get the Database key and cycle through SW DB
     */
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
            unit,
            stage,
            db_id_ndx,
            &fp_database_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 450, exit); 
    info->key.db_id_quals[db_id_curr].db_id = db_id_ndx;
    info->key.db_id_quals[db_id_curr].stage = stage;

    for(qual_index = 0; qual_index < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX ; ++qual_index) {
        info->key.db_id_quals[db_id_curr].qual[qual_index].type = fp_database_info.qual_types[qual_index];
        res = arad_pmf_ce_internal_field_info_find(
                unit,
                fp_database_info.qual_types[qual_index],
                stage,
                0, /* is_msb not relevant */    
                &is_found,
                &qual_info
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 460, exit); 
        if (is_found) {
            res = arad_pmf_ce_internal_field_info_find(
                    unit,
                    qual_info.info.irpp_field,
                    stage,
                    0, /* is_msb not relevant */    
                    &is_found,
                    &qual_info_tmp
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 465, exit); 

            /* Get the signal */
            sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
            reg_fld.msb = qual_info.signal.msb;
            reg_fld.lsb = qual_info.signal.lsb;
            reg_fld.base = (qual_info.signal.base0 << 16) + qual_info.signal.base1;
            if (reg_fld.base != 0) {
                res = arad_pp_diag_dbg_val_get_unsafe(
                        unit,
                        core_id,
                        ARAD_EGQ_ID,
                        &reg_fld,
                        val
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 470, exit);

                /* Special cases */
                if (qual_info.info.irpp_field == SOC_PPC_FP_QUAL_ERPP_OUT_PP_PORT_PMF_DATA) {
                    res = arad_pp_egq_pp_ppct_tbl_get_unsafe(unit, core_id, val[0], &egq_pp_ppct_tbl);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 480, exit);
                    val[0] = egq_pp_ppct_tbl.pmf_data;
                }
                else if (qual_info.info.irpp_field == SOC_PPC_FP_QUAL_ERPP_OUT_TM_PORT_PMF_DATA) {
                    res = arad_egq_ppct_tbl_get_unsafe(unit, core_id, val[0], &egq_ppct_tbl);
                    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 490, exit);
                    val[0] = egq_ppct_tbl.pmf_data;
                }

                SHR_BITCOPY_RANGE(info->key.db_id_quals[db_id_curr].qual[qual_index].val, 
                                  0, 
                                  val, 
                                  (qual_info.info.buffer_lsb - qual_info_tmp.info.buffer_lsb), 
                                  qual_info.info.qual_nof_bits
                                  ); 
            }
        }
    }


      /* Get the Action value by reading the correct signals */
      action_curr = 0;
      is_found = FALSE;
      for (action_ndx = 0; action_ndx < SOC_DPP_DEFS_GET(unit, nof_egress_pmf_actions); action_ndx++) {
          /* Get the value */
          res = arad_pmf_fem_action_egress_info_get(
                    unit,
                    fp_database_info.action_types[action_ndx],
                    &is_found,
                    &action_egress_info
                  );
          SOC_SAND_CHECK_FUNC_RESULT(res, 500, exit); 

          info->egress_action[action_curr].type = SOC_PPC_FP_ACTION_TYPE_INVALID; /* Default unless set */
          if (!is_found) {
              continue;
          }


          /* Get the signal */
          sal_memset(val, 0x0, ARAD_PP_DIAG_DBG_VAL_LEN * 4);
          reg_fld.msb = action_egress_info.msb;
          reg_fld.lsb = action_egress_info.lsb;
          reg_fld.base = (action_egress_info.base0 << 16) + action_egress_info.base1;
          if (reg_fld.base != 0) {
              res = arad_pp_diag_dbg_val_get_unsafe(
                      unit,
                      core_id,
                      ARAD_EGQ_ID,
                      &reg_fld,
                      val
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 510, exit);
              switch (action_egress_info.action_type) {
              case SOC_PPC_FP_ACTION_TYPE_EGR_OFP:
                if(SOC_IS_JERICHO(unit)) {
                  info->egress_action[action_curr].type = action_egress_info.action_type;
                  SHR_BITCOPY_RANGE(&(info->egress_action[action_curr].val), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1) - 1); 
                  break;
                }
                /* else - continue and handle like EGR_TRAP */
              case SOC_PPC_FP_ACTION_TYPE_EGR_TRAP:
                  if (val[0] & 0x1) { /* Valid bit in LSB */
                      info->egress_action[action_curr].type = action_egress_info.action_type;
                      SHR_BITCOPY_RANGE(&(info->egress_action[action_curr].val), 0, val, 1, (reg_fld.msb - reg_fld.lsb + 1) - 1); 
                  }
                  break;
              case SOC_PPC_FP_ACTION_TYPE_MIRROR:
                  if (val[0]) { /* Valid if not 0 */
                      info->egress_action[action_curr].type = action_egress_info.action_type;
                      SHR_BITCOPY_RANGE(&(info->egress_action[action_curr].val), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1)); 
                  }
                  break;
              case SOC_PPC_FP_ACTION_TYPE_ACE_POINTER:
                  if (val[0] != 0xFFF) { /* Valid if not 0xFFF */
                      info->egress_action[action_curr].type = action_egress_info.action_type;
                      SHR_BITCOPY_RANGE(&(info->egress_action[action_curr].val), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1)); 
                  }
                  break;
              default:
                  info->egress_action[action_curr].type = action_egress_info.action_type;
                  SHR_BITCOPY_RANGE(&(info->egress_action[action_curr].val), 0, val, 0, (reg_fld.msb - reg_fld.lsb + 1)); 
                  break;
              }
          }

          action_curr++;
      }
      db_id_curr++;
  }

exit:
  ARAD_FREE(dir_extr_action_val);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_packet_diag_get_unsafe()", 0, 0);
}

uint32
  arad_pp_fp_resource_diag_get_verify(
    SOC_SAND_IN  int                 		    unit,
    SOC_SAND_IN  SOC_PPC_FP_RESOURCE_DIAG_MODE	mode,
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DIAG_INFO	*info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_RESOURCE_DIAG_GET_VERIFY);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_PPC_FP_RESOURCE_DIAG_INFO_clear(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_resource_diag_get_verify()", 0, 0);
}

/*********************************************************************
 * NAME:
 *   arad_pp_fp_is_action_on_any_db
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Find out whether input 'in_action_type' is in use on any DB and
 *   specify the first using DB.
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type -
 *     Action to search for on all valid DBs.
 *   SOC_SAND_OUT uint32                  *db_identifier_p -
 *     If input action is NOT found then return *db_identifier_p set to -1.
 *     Otherwise, set *db_identifier_p to the index of containing DB. Note
 *     that this is true even when return value indicates 'error'
 * REMARKS:
 *    To get 'stage' out of db_identifier:
 *      arad_pp_fp_db_stage_get(unit,db_identifier,&stage) ;
 *    To get priority out of db_identifier:
 *      sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(unit,stage,db_idx,&db_priority) ;
 *    To get TCAM identifier out of db_identifier:
 *      tcam_db_idx = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_idx);
 * SEE:
 *   arad_pp_fp_get_dbs_for_action()
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_pp_fp_is_action_on_any_db(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE      in_action_type,
    SOC_SAND_OUT uint32                      *db_identifier_p
  )
{
  uint32
    res, 
    action_idx,
    quit,
    db_idx ;
  ARAD_PMF_DB_INFO        
    pmf_db_info ;
  SOC_PPC_FP_DATABASE_INFO      
    fp_db_info ;
  SOC_PPC_FP_ACTION_TYPE
    action_type ;
  SOC_PPC_FP_DATABASE_STAGE        
    stage ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;

  SOC_SAND_CHECK_NULL_INPUT(db_identifier_p) ;
  *db_identifier_p = -1 ;
  quit = 0 ;
  /* 
   * Retrieve Resource DB Information (actions) 
   */
  for (db_idx = 0 ; db_idx < SOC_PPC_FP_NOF_DBS ; db_idx++)
  {
    sal_memset(&pmf_db_info, 0x0, sizeof(pmf_db_info)) ;
    res = arad_pp_fp_db_stage_get(unit,db_idx,&stage) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(unit, stage, db_idx, &pmf_db_info) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ; 
    if (pmf_db_info.valid)
    {
      /* Get the DB type (TCAM, DT, DE) */
      sal_memset(&fp_db_info, 0x0, sizeof(SOC_PPC_FP_DATABASE_INFO)) ;
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(unit, stage, db_idx, &fp_db_info) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ; 
      if (fp_db_info.db_type >= SOC_PPC_NOF_FP_DATABASE_TYPES)
      {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_TYPE_OUT_OF_RANGE_ERR, 40, exit) ;
      }
      for (action_idx = 0 ; action_idx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX ; action_idx++) 
      {
        action_type = fp_db_info.action_types[action_idx] ;
        if (action_type == SOC_PPC_FP_ACTION_TYPE_INVALID) 
        {
          break ;
        }
        if (action_type == in_action_type) 
        {
          /*
           * Found input action on 'db_idx'
           */
          *db_identifier_p = db_idx ;
          quit = 1 ;
          break ;
        }
      }
      if (quit)
      {
        break ;
      }
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_is_action_on_any_db()", 0, 0); 
}
/*********************************************************************
 * NAME:
 *   arad_pp_fp_get_dbs_for_action
 * TYPE:
 *   PROC
 * FUNCTION:
 *   Get array of all DBs on which input 'in_action_type' is in use.
 * INPUT:
 *   SOC_SAND_IN  int                     unit -
 *     Identifier of the device to access.
 *   SOC_SAND_IN  uint32                  flags -
 *     Some extra controls for this procedure, Currently not in use.
 *   SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE  in_action_type -
 *     Action to search for on all valid DBs.
 *   SOC_SAND_IN  uint32                  db_identifier_size -
 *     Maximal number of array elements in '*db_identifier_p', as allowed
 *     by the caller.
 *   SOC_SAND_OUT uint32                  *db_identifier_p -
 *     Load *db_identifier_p by identifiers of containing DBs.
 *   SOC_SAND_OUT uint32                  *db_identifier_size_actual -
 *     To be loaded by this procedure by the actual number of
 *     valid elements loaded into '*db_identifier_p'. If 'db_identifier_size'
 *     is too small to load all relevant data bases, then
 *     '*data_base_size_actual' is loaded by '-1'.
 * REMARKS:
 *    To get 'stage' out of db_identifier:
 *      arad_pp_fp_db_stage_get(unit,db_identifier,&stage) ;
 *    To get priority out of db_identifier:
 *      sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(unit,stage,db_idx,&db_priority) ;
 *    To get TCAM identifier out of db_identifier:
 *      tcam_db_idx = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_idx);
 * SEE:
 *   arad_pp_fp_is_action_on_any_db(), arad_pp_fp_action_info_show_unsafe(),
 *   arad_pp_fp_dbs_action_info_show_unsafe()
 * RETURNS:
 *   OK or ERROR indication.
 *********************************************************************/
uint32
  arad_pp_fp_get_dbs_for_action(
    SOC_SAND_IN  int                         unit,
    SOC_SAND_IN  uint32                      flags,
    SOC_SAND_IN  SOC_PPC_FP_ACTION_TYPE      in_action_type,
    SOC_SAND_IN  uint32                      db_identifier_size,
    SOC_SAND_OUT uint32                      *db_identifier_p,
    SOC_SAND_OUT uint32                      *db_identifier_size_actual
  )
{
  uint32
    res,
    no_resource,
    action_idx,
    db_idx ;
  ARAD_PMF_DB_INFO        
    pmf_db_info ;
  SOC_PPC_FP_DATABASE_INFO      
    fp_db_info ;
  SOC_PPC_FP_ACTION_TYPE
    action_type ;
  SOC_PPC_FP_DATABASE_STAGE        
    stage ;
  uint32
    *loc_db_identifier_p ;
  uint32
    actual_num_elements ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;

  SOC_SAND_CHECK_NULL_INPUT(db_identifier_p) ;
  SOC_SAND_CHECK_NULL_INPUT(db_identifier_size_actual) ;
  *db_identifier_size_actual = -1 ;
  actual_num_elements = 0 ;
  loc_db_identifier_p = db_identifier_p ;
  no_resource = 0 ;
  /* 
   * Retrieve Resource DB Information (actions) 
   */
  for (db_idx = 0 ; db_idx < SOC_PPC_FP_NOF_DBS ; db_idx++)
  {
    sal_memset(&pmf_db_info, 0x0, sizeof(pmf_db_info)) ;
    res = arad_pp_fp_db_stage_get(unit,db_idx,&stage) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(unit, stage, db_idx, &pmf_db_info) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ; 
    if (pmf_db_info.valid)
    {
      /* Get the DB type (TCAM, DT, DE) */
      sal_memset(&fp_db_info, 0x0, sizeof(SOC_PPC_FP_DATABASE_INFO)) ;
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(unit, stage, db_idx, &fp_db_info) ;
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit) ; 
      if (fp_db_info.db_type >= SOC_PPC_NOF_FP_DATABASE_TYPES)
      {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_TYPE_OUT_OF_RANGE_ERR, 40, exit) ;
      }
      for (action_idx = 0 ; action_idx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX ; action_idx++)
      {
        action_type = fp_db_info.action_types[action_idx] ;
        if (action_type == SOC_PPC_FP_ACTION_TYPE_INVALID) 
        {
          break ;
        }
        if (action_type == in_action_type) 
        {
          /*
           * Found input action on 'db_idx'. Go to next DB.
           */
          if (actual_num_elements >= db_identifier_size)
          {
            /*
             * There is not enough space on input array: db_identifier_p[]
             * Quit leaving: *db_identifier_size_actual = -1 ;
             */
            res = SOC_E_RESOURCE ;
            FUNC_RESULT_SOC_PRINT(res) ;
            no_resource = 1 ;
            break ;
          }
          *loc_db_identifier_p++ = db_idx ;
          actual_num_elements++ ;
          break ;
        }
      }
      if (no_resource)
      {
        break ;
      }
    }
  }
  if (!no_resource)
  {
    *db_identifier_size_actual = actual_num_elements ;
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_get_dbs_for_action()", 0, 0); 
}

/*********************************************************************
*   Display correspondence between low level (HW level) internal action
*   and data bases which use it.
*   Details: in the H file. (search for prototype)
*   See:
*     SOC_PPC_FP_ACTION_TYPE_to_string()
*     See _bcm_dpp_field_action_name[bcm_action]
*     See _bcm_dpp_field_stage_name[bcm_stage]
*     SOC_PPC_FP_ACTION_TYPE
*     arad_pp_fp_get_dbs_for_action
*     SOC_PPC_FP_DATABASE_TYPE_to_string
*     SOC_PPC_FP_DATABASE_STAGE_to_string
*     ARAD_PP_FP_DB_ID_TO_TCAM_DB()
*********************************************************************/
uint32
  arad_pp_fp_dbs_action_info_show_unsafe(
    SOC_SAND_IN  int unit
  )
{
  char
    *action_name, *db_type_name, *device_name ;
  int
    internal_action, res, ii ;
  uint32
    flags, db_identifier_size, data_bases[40],
    db_identifier_size_actual, db_idx, width,
    default_width, hw_id ;
  SOC_PPC_FP_DATABASE_INFO      
    fp_db_info ;
  char
    ii_buff[5], db_buff[5],
    width_name[5],default_width_name[5],hw_id_name[5] ;
  SOC_PPC_FP_DATABASE_STAGE        
    stage ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  #define SHOW_FORMAT_DBS  "%8s %25s %5s %5s %7s  %2s %10s %20s\r\n"
  if (SOC_IS_JERICHO_PLUS(unit)) {
      device_name = "QAX" ;
  } else  if (SOC_IS_JERICHO(unit)) {
      device_name = "JERICHO" ;
  } else if (SOC_IS_ARADPLUS(unit) && (!(SOC_IS_ARDON(unit)))) {
      device_name = "ARADPLUS" ;
  } else {
      device_name = "ARAD" ;
  }
  LOG_CLI(
    (BSL_META_U(unit,
    "\r\n"
    "******** HW Actions and corresponding data bases ****** %s ********"
    "\r\n\n"),device_name
  )) ;
  /*
   * We assume the first internal action is '0'.
   * Currently, this is SOC_PPC_FP_ACTION_TYPE_DEST.
   */
  db_identifier_size = sizeof(data_bases) / sizeof(data_bases[0]) ;
  LOG_CLI((BSL_META_U(unit,SHOW_FORMAT_DBS),"ORDINAL","INTERNAL ACTION","HW ID","WIDTH","DEFAULT","I","DATA BASE","TYPE")) ;
  for (internal_action = 0 ; internal_action < SOC_PPC_NOF_FP_ACTION_TYPES_ARAD ; internal_action++) {
    action_name = (char *)SOC_PPC_FP_ACTION_TYPE_to_string(internal_action) ;
    flags = 0 ;
    res = arad_pmf_fem_action_width_get_unsafe(unit, internal_action, &width, &hw_id) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit) ;
    if ((int)width == NOT_ON_RUNTIME_ARRAY)
    {
      sal_strcpy(width_name,"-") ;
      sal_strcpy(hw_id_name,"-") ;
    }
    else
    {
      sal_itoa(width_name,width,10,0,2) ;
      /*
       * A negative 'hw_id' indicates it is not valid (on this device)
       */
      if ((int)hw_id < 0) {
        sal_strcpy(hw_id_name,"-") ;
      } else {
        sal_itoa(hw_id_name,hw_id,10,0,2) ;
      }
    }
    res = arad_pmf_fem_action_width_default_get(unit, internal_action, &default_width) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit) ;

    if ((int)default_width < 0)
    {
      sal_strcpy(default_width_name,"-") ;
    }
    else
    {
      sal_itoa(default_width_name,default_width,10,0,2) ;
    }
    res =
      arad_pp_fp_get_dbs_for_action(
        unit,flags,internal_action,db_identifier_size,data_bases,&db_identifier_size_actual) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;
    LOG_CLI((BSL_META_U(unit,"%8d %25s %5s %5s %7s\r\n"),internal_action,action_name,hw_id_name,width_name,default_width_name)) ;
    if (db_identifier_size_actual) {
      for (ii = 0 ; ii < db_identifier_size_actual ; ii++)
      {
        sal_itoa(ii_buff,ii+1,10,0,2) ;
        db_idx = data_bases[ii] ;
        sal_itoa(db_buff,db_idx,10,0,3) ;
        res = arad_pp_fp_db_stage_get(unit,db_idx,&stage) ;
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
        /*
         * Get the DB type (TCAM, DT, DE)
         */
        sal_memset(&fp_db_info, 0x0, sizeof(SOC_PPC_FP_DATABASE_INFO)) ;
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(unit, stage, db_idx, &fp_db_info) ;
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit) ; 
        db_type_name = (char *)SOC_PPC_FP_DATABASE_TYPE_to_string(fp_db_info.db_type) ;
        LOG_CLI((BSL_META_U(unit,SHOW_FORMAT_DBS),"","","","","",ii_buff,db_buff,db_type_name)) ;
      }
    } else {
      LOG_CLI((BSL_META_U(unit,SHOW_FORMAT_DBS),"","","","","","-","-","-")) ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_dbs_action_info_show_unsafe()", 0, 0); 
}
/*********************************************************************
*   Display correspondence between low level (HW level) internal action
*   and BCM level action + stage.
*   Details: in the H file. (search for prototype)
*   See:
*     SOC_PPC_FP_ACTION_TYPE_to_string()
*     See _bcm_dpp_field_action_name[bcm_action]
*     See _bcm_dpp_field_stage_name[bcm_stage]
*     SOC_PPC_FP_ACTION_TYPE
*     bcm_petra_field_internal_to_bcm_action_map
*********************************************************************/
uint32
  arad_pp_fp_action_info_show_unsafe(
    SOC_SAND_IN  int unit
  )
{
  char *action_name ;
  char *bcm_action_name ;
  char *bcm_stage_name ;
  int internal_action ;
  uint32 flags ;
  uint32 bcm_action_size ;
  bcm_field_internal_to_bcm_map_t bcm_action_stage[40] ;
  uint32 bcm_action_size_actual ;
  int res, ii ;
  char buff[5] ;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);

  #define SHOW_FORMAT_AC  "%8s %25s   %2s %30s %10s\r\n"
  LOG_CLI(
    (BSL_META_U(unit,
    "\r\n"
    "******************** HW Actions and corresponding BCM info (action, stage, ...) **************"
    "\r\n\n")
  )) ;
  /*
   * We assume the first internal action is '0'.
   * Currently, this is SOC_PPC_FP_ACTION_TYPE_DEST.
   */
  flags = 0 ;
  bcm_action_size = sizeof(bcm_action_stage) / sizeof(bcm_action_stage[0]) ;
  LOG_CLI((BSL_META_U(unit,SHOW_FORMAT_AC),"ORDINAL","INTERNAL ACTION","I","BCM ACTION","BCM STAGE")) ;
  for (internal_action = 0 ; internal_action < SOC_PPC_NOF_FP_ACTION_TYPES_ARAD ; internal_action++) {
    action_name = (char *)SOC_PPC_FP_ACTION_TYPE_to_string(internal_action) ;
    res =
      bcm_field_internal_to_bcm_action_map(
        unit,flags,internal_action,bcm_action_size,bcm_action_stage,&bcm_action_size_actual) ;
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit) ;
    LOG_CLI((BSL_META_U(unit,"%8d %25s\r\n"),internal_action,action_name)) ;
    if (bcm_action_size_actual) {
      for (ii = 0 ; ii < bcm_action_size_actual ; ii++)
      {
        bcm_action_name = (char *)_bcm_dpp_field_action_name[bcm_action_stage[ii].bcm_action] ;
        bcm_stage_name = (char *)_bcm_dpp_field_stage_name[bcm_action_stage[ii].bcm_stage] ;
        sal_itoa(buff,ii+1,10,0,2) ;
        LOG_CLI((BSL_META_U(unit,SHOW_FORMAT_AC),"","",buff,bcm_action_name,bcm_stage_name)) ;
      }
    } else {
      LOG_CLI((BSL_META_U(unit,SHOW_FORMAT_AC),"","","-","-","-")) ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_action_info_show_unsafe()", 0, 0); 
}
/*
 * Debug only utilities for display of some views of of FEMs and FESs
 * {
 */
/*
 * Print all FEMs and corresponding DBs for specified stage,
 * for TM or non-TM (FTMH header)
 * Details: in the H file. (search for prototype)
 */
uint32
  soc_ppd_fp_print_all_fems_for_stage(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN   SOC_PPC_FP_DATABASE_STAGE stage,
    SOC_SAND_IN  uint8                   is_for_tm
  )
{
  uint32
    width,
    ii,
    res,
    db_strength,
    hw_id,
    fem_idx ,
    fem_pgm_id;
  int
    internal_action ;
  SOC_PPC_FP_FEM_ENTRY
    fem_entry ;
  char
    width_name[5], fem_idx_name[5], db_id_name[5], db_strength_name[5] ;

  #define SHOW_FORMAT_FEMS_FOR_STAGE  "%7s %5s %11s %25s %5s\r\n"
  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  res = SOC_SAND_OK ;
  if (is_for_tm >= ARAD_PMF_NOF_FEM_PGMS) {
    LOG_CLI(
      (BSL_META_U(unit,
        "%s(): Parameter is_for_tm (%d) is out of range. Max is %d\r\n"),
        __FUNCTION__,is_for_tm,ARAD_PMF_NOF_FEM_PGMS-1
    )) ;
    res = SOC_E_PARAM ;
    FUNC_RESULT_SOC_PRINT(res) ;
    goto exit ;
  }
  if (stage >= SOC_PPC_NOF_FP_DATABASE_STAGES) {
    LOG_CLI(
      (BSL_META_U(unit,
        "%s(): Parameter stage (%d) is out of range. Max is %d\r\n"),
        __FUNCTION__,stage,SOC_PPC_NOF_FP_DATABASE_STAGES-1
    )) ;
    res = SOC_E_PARAM ;
    FUNC_RESULT_SOC_PRINT(res) ;
    goto exit ;
  }
  LOG_CLI(
    (BSL_META_U(unit,
      "%s(): Display FEMs -- for TM processing? %d, stage %d (%s)\r\n"),
      __FUNCTION__,is_for_tm,stage,SOC_PPC_FP_DATABASE_STAGE_to_string(stage)
  )) ;
  LOG_CLI(
    (BSL_META_U(unit,
      SHOW_FORMAT_FEMS_FOR_STAGE),
      "FEM_IDX","DB_ID","DB STRENGTH","ACTION NAME","WIDTH"
  )) ;
  for (fem_idx = 0; fem_idx < ARAD_PMF_LOW_LEVEL_NOF_FEMS; fem_idx++) {
   for(fem_pgm_id = 0 ; (fem_pgm_id < ARAD_PMF_NOF_FEM_PGMS); ++fem_pgm_id) {
    res =
      sw_state_access[unit].dpp.soc.arad.tm.pmf.fem_entry.get(
        unit,
        stage,
        fem_pgm_id,
        fem_idx,
        &fem_entry
      ) ;
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
    if (fem_entry.action_type[0] != SOC_PPC_FP_ACTION_TYPE_INVALID) {
      res =
        sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(unit, stage, fem_entry.db_id, &db_strength);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit) ;
      sal_itoa(fem_idx_name,fem_idx,10,0,2) ;
      sal_itoa(db_id_name,fem_entry.db_id,10,0,2) ;
      sal_itoa(db_strength_name,db_strength,10,0,2) ;
      LOG_CLI(
        (BSL_META_U(unit,
          SHOW_FORMAT_FEMS_FOR_STAGE),
          fem_idx_name,db_id_name,db_strength_name,"",""
      )) ;
      for (ii = 0 ; ii < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX ; ii++) {
        if (fem_entry.action_type[ii] == SOC_PPC_FP_ACTION_TYPE_INVALID) {
          break ;
        }
        internal_action = fem_entry.action_type[ii] ;
        res = arad_pmf_fem_action_width_get_unsafe(unit, internal_action, &width, &hw_id) ;
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit) ;
        if ((int)width == NOT_ON_RUNTIME_ARRAY)
        {
          sal_strcpy(width_name,"-") ;
        } else {
          sal_itoa(width_name,width,10,0,2) ;
        }
        LOG_CLI(
          (BSL_META_U(unit,
            SHOW_FORMAT_FEMS_FOR_STAGE),
            "-","-","-",
            (char *)SOC_PPC_FP_ACTION_TYPE_to_string(internal_action),width_name
        )) ;
      }
    } else {
      sal_itoa(fem_idx_name,fem_idx,10,0,2) ;
      LOG_CLI(
        (BSL_META_U(unit,
          SHOW_FORMAT_FEMS_FOR_STAGE),
          fem_idx_name,"-","-","EMPTY",""
      )) ;
    }
  }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_fp_print_all_fems_for_stage()", 0, 0);
}
/*
 * Print FES info for specified stage and program.
 * Show corresponding DBs with priorities as well.
 * Details: in the H file. (search for prototype)
 */
uint32
  soc_ppd_fp_print_fes_info_for_stage(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE stage,
    SOC_SAND_IN  uint32                 pmf_pgm_ndx
  )
{
  uint32
    res,
    db_strength,
    fes_idx ;
  int
    internal_action ;
  uint8
    db_id ;
  ARAD_PMF_FES         
    fes_info_lcl[ARAD_PMF_LOW_LEVEL_NOF_FESS] ;
  char
    db_id_name[5],db_strength_name[5],fes_idx_name[5] ;

  #define SHOW_FORMAT_FESS_FOR_STAGE  "%8s %25s %5s %11s\r\n"
  SOC_SAND_INIT_ERROR_DEFINITIONS(0) ;
  res = SOC_SAND_OK ;
  if (pmf_pgm_ndx >= ARAD_PMF_NOF_PROGS) {
    LOG_CLI(
      (BSL_META_U(unit,
        "%s(): Parameter pmf_pgm_ndx (%d) is out of range. Max is %d\r\n"),
        __FUNCTION__,pmf_pgm_ndx,ARAD_PMF_NOF_PROGS-1
    )) ;
    res = SOC_E_PARAM ;
    FUNC_RESULT_SOC_PRINT(res) ;
    goto exit ;
  }
  if (stage >= SOC_PPC_NOF_FP_DATABASE_STAGES) {
    LOG_CLI(
      (BSL_META_U(unit,
        "%s(): Parameter stage (%d) is out of range. Max is %d\r\n"),
        __FUNCTION__,stage,SOC_PPC_NOF_FP_DATABASE_STAGES-1
    )) ;
    res = SOC_E_PARAM ;
    FUNC_RESULT_SOC_PRINT(res) ;
    goto exit ;
  }
  LOG_CLI(
    (BSL_META_U(unit,
      "%s(): Display FESs for -- program %d, stage %d (%s)\r\n"),
      __FUNCTION__,pmf_pgm_ndx,stage,SOC_PPC_FP_DATABASE_STAGE_to_string(stage)
  )) ;
  LOG_CLI(
    (BSL_META_U(unit,
      SHOW_FORMAT_FESS_FOR_STAGE),
      "FES_IDX","ACTION NAME","DB_ID","DB STRENGTH"
  )) ;
  for (fes_idx = 0; fes_idx < ARAD_PMF_LOW_LEVEL_NOF_FESS; ++fes_idx) {
    res =
      sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.get(
        unit,
        stage,
        pmf_pgm_ndx,
        fes_idx,
        &fes_info_lcl[fes_idx]
    );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);
  }
  for (fes_idx = 0 ; fes_idx < ARAD_PMF_LOW_LEVEL_NOF_FESS ; fes_idx++) {
    if (fes_info_lcl[fes_idx].is_used) {
      internal_action = fes_info_lcl[fes_idx].action_type ;
      db_id = fes_info_lcl[fes_idx].db_id ;
      if (db_id < ARAD_PMF_NOF_DBS) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(unit, stage, db_id, &db_strength);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
        sal_itoa(db_id_name,db_id,10,0,2) ;
        sal_itoa(db_strength_name,db_strength,10,0,2) ;
      } else {
        /*
         * This FES is not assigned to any DB. See arad_pmf_low_level_fes_action_construction_unsafe()
         */
        db_id_name[0] = '-' ;
        db_id_name[1] = 0 ;
        db_strength_name[0] = '-' ;
        db_strength_name[1] = 0 ;
      }
      sal_itoa(fes_idx_name,fes_idx,10,0,2) ;
      LOG_CLI(
        (BSL_META_U(unit,
           SHOW_FORMAT_FESS_FOR_STAGE),
           fes_idx_name, SOC_PPC_FP_ACTION_TYPE_to_string(internal_action),
           db_id_name,db_strength_name
      )) ;
    }
  }
exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_fp_print_fes_info_for_stage()", 0, 0);
}
/*
 * }
 */

/*********************************************************************
*   Get the full resources usage.
*   Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  arad_pp_fp_resource_diag_get_unsafe(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  SOC_PPC_FP_RESOURCE_DIAG_MODE  mode,
    SOC_SAND_OUT SOC_PPC_FP_RESOURCE_DIAG_INFO  *info
  )
{
  uint32
    res, 
    valid,
    db_idx,
    bank_id,
    msb_loc,
    tcam_db_idx,
    key_idx = 0,
    entry_idx,
    entry_id,
    nof_keys,
    pgm_idx_min,
    pgm_idx_max,
    pgm_idx = 0,
    cycle_idx,
    ce_idx,
    ce_id,
    db_priority,
    fes_idx,
    fes_group_idx,
    fem_group_idx,
    fem_pgm_idx,
    fem_id,
    line_idx,
    profile_idx,
    nof_actions,
    action_idx,
    ce_loc,
    lsb_msb,
    param_idx,
    value_idx,
    profile_id,
    prefix_id,
    bank_nof_lines,
    qual_lsb, 
    nof_bits,
    qual_idx,
    ce_rsrc,
    key_rsrc,
    bank_used,
    bank_used_1,
    hw_action_bmp,
    resolution_ce_in_bits,
    nof_lost_lsb_bits,
    priority,
    pfg_idx,
    *db_used_program = NULL,
    diag_prog_bitmap[SOC_PPC_NOF_FP_DATABASE_STAGES],
    bank_nof_entries_hw[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS],
    bank_profile_id[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS],
    db_profile_id[ARAD_TCAM_NOF_ACCESS_PROFILE_IDS],
    /* bank_db_prefix will be 2 dimensions array of [SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS][ARAD_TCAM_NOF_PREFIXES].
       however, we will allocate it as one dimension array and bank_db_prefix[i][j] will be cur_alloc_key[i*ARAD_TCAM_NOF_PREFIXES + j] */
    *bank_db_prefix = NULL,
    db_nof_entries[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS],
    action_lsbs[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
    action_lengths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
    /* key_cur_loc will be 2 dimensions array of [SOC_PPC_FP_NOF_DBS][ARAD_PP_FP_KEY_NOF_ZONES].
       however, we will allocate it as one dimension array and key_cur_loc[i][j] will be cur_alloc_key[i*ARAD_PP_FP_KEY_NOF_ZONES + j] */
    *key_cur_loc = NULL,
    diag_ce_bitmap[SOC_PPC_NOF_FP_DATABASE_STAGES][ARAD_PMF_NOF_PROGS][ARAD_PMF_NOF_CYCLES],
    diag_key_bitmap[SOC_PPC_NOF_FP_DATABASE_STAGES][ARAD_PMF_NOF_PROGS][ARAD_PMF_NOF_CYCLES],
    diag_fes_bitmap[SOC_PPC_NOF_FP_DATABASE_STAGES][ARAD_PMF_NOF_PROGS],
    nof_entries;

  uint8
    is_32b_ce,
    found,
    not_valid,
    is_valid,
    is_used,
    is_direct,
    is_tm,
    bank_db_prefix_used[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS][ARAD_TCAM_NOF_PREFIXES];

  ARAD_TCAM_ACTION_SIZE 
    action_bmp;
  ARAD_TCAM_ENTRY 
      tcam_entry;
  SOC_PPC_FP_ACTION_TYPE        
    action_types[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX];
  ARAD_TCAM_ACTION_SIZE   
    fem_act_size;
  SOC_PPC_FP_DATABASE_INFO      
    fp_db_info; 
  ARAD_PMF_DB_INFO        
    pmf_db_info;
  ARAD_PMF_CE             
    pgm_ce;
  ARAD_TCAM_PREFIX
    prefix,
      prefix_hw;
  ARAD_PMF_FES            
    fes_info;
  SOC_PPC_FP_FEM_ENTRY          
    fem_entry;
  ARAD_PMF_PSL                  
    psl_tbl_data;
  ARAD_PP_IHB_TCAM_ACCESS_PROFILE_TBL_DATA 
    profile_tbl_data;
  ARAD_PP_IHB_TCAM_PD_PROFILE_TBL_DATA
    pd_profile_tbl_data;
  ARAD_TCAM_LOCATION 
    location;
  ARAD_TCAM_BANK_ENTRY_SIZE 
    entry_size,
    bank_entry_size[SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS];
  SOC_PPC_FP_RESOURCE_DIAG_ERROR_PARAM 
    *error_params;
  ARAD_PP_FP_RESOURCE_DIAG_DB_PRIORITY
    *diag_priority = NULL;
  ARAD_PP_FP_RESOURCE_DIAG_DB_KEY
    *diag_db_key = NULL;
  ARAD_PP_FP_RESOURCE_DIAG_DB_KEY_ARGS
     *diag_db_key_args = NULL;
  ARAD_PMF_CE_HEADER_QUAL_INFO
    header_qual_info;
  ARAD_PMF_CE_PACKET_HEADER_INFO      
    header_info;
  SOC_PPC_FP_QUAL_TYPE 
    irpp_field; 
  ARAD_PMF_CE_IRPP_QUALIFIER_INFO 
    irpp_qual_info;
  ARAD_PMF_FES_INPUT_INFO 
    fes_input_info;
  SOC_PPC_FP_DIR_EXTR_ACTION_VAL 
    fem_info;
  SOC_PPC_FP_QUAL_VAL 
    qual_info;
  ARAD_PP_FP_RESOURCE_DIAG_ACTION_PRIORITY
    *fes_prio = NULL,
    *action_prio = NULL;
  SOC_PPC_PMF_PFG_INFO
    *pfg_info = NULL;
  SOC_SAND_OCC_BM_PTR
    occ_bm;
  ARAD_FP_ENTRY
    fp_entry;
  SOC_PPC_FP_DATABASE_STAGE        
    stage;
  SOC_SAND_SUCCESS_FAILURE  
      success;
  SOC_PPC_FP_CONTROL_INDEX      
      control_ndx;
  SOC_PPC_FP_CONTROL_INFO        
      control_info;

  SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_RESOURCE_DIAG_GET_UNSAFE);
  arad_pp_fp_resource_diag_get_verify(unit, mode, info);
  sal_memset(db_nof_entries, 0x0, sizeof(uint32) * SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS); 
  
  ARAD_ALLOC(bank_db_prefix, uint32, SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS * ARAD_TCAM_NOF_PREFIXES, "arad_pp_fp_resource_diag_get_unsafe.bank_db_prefix");
  sal_memset(bank_db_prefix, 0x0, sizeof(uint32) * SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS * ARAD_TCAM_NOF_PREFIXES); 
  sal_memset(bank_db_prefix_used, 0x0, sizeof(uint8) * SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS * ARAD_TCAM_NOF_PREFIXES); 
  sal_memset(diag_key_bitmap, 0x0, sizeof(uint32) * SOC_PPC_NOF_FP_DATABASE_STAGES * ARAD_PMF_NOF_PROGS * ARAD_PMF_NOF_CYCLES);
  value_idx = 0;

  /* Prefix is relevant only if DB type is TCAM */
  sal_memset(&prefix, 0x0, sizeof(ARAD_TCAM_PREFIX));


  /* 
   * Retrieve Resource DB Information (e.g. type, stage, entries, qualifiers, actions) 
   * The order is important in some cases since a part of the data will later be used 
   * for retrieving per CE/FES information.
   */
  for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; db_idx++)
  {
    res = arad_pp_fp_db_stage_get(unit,db_idx,&stage);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    info->db[db_idx].stage = stage;

    sal_memset(&pmf_db_info, 0x0, sizeof(ARAD_PMF_DB_INFO));
    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(unit, info->db[db_idx].stage, db_idx, &pmf_db_info);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit); 

    info->db[db_idx].valid = pmf_db_info.valid;

    if(info->db[db_idx].valid)
    {
      /* Get the DB type (TCAM, DT, DE) */
      sal_memset(&fp_db_info, 0x0, sizeof(SOC_PPC_FP_DATABASE_INFO));
      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(unit, info->db[db_idx].stage, db_idx, &fp_db_info);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit); 

      if(fp_db_info.db_type >= SOC_PPC_NOF_FP_DATABASE_TYPES)
      {
        SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_TYPE_OUT_OF_RANGE_ERR, 40, exit);
      }
      info->db[db_idx].type = fp_db_info.db_type;

      res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(
              unit, 
              stage, 
              db_idx, 
              &db_priority
            );
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

      info->db[db_idx].db_priority = db_priority;

      tcam_db_idx = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_idx);

      if(info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_TCAM 
         || ( ( info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE ) && !(fp_db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS))
         || info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_EGRESS)
      {
          /* 
           * bank information per tcam db - calculate used entried from sorted list 
           * from each element retrieve tcam db id and bank id 
           */
          res = arad_tcam_resource_db_entries_find(unit, tcam_db_idx, db_nof_entries);
          SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
      
          /* Prefix is relevant only if DB type is TCAM */
          sal_memset(&prefix, 0x0, sizeof(ARAD_TCAM_PREFIX));

          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix.get(
              unit, 
              tcam_db_idx, 
              &prefix
              );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

          info->db[db_idx].db_tcam.prefix_val = prefix.bits;
          info->db[db_idx].db_tcam.prefix_len = prefix.length;
      }

      /* 
       * For each bank ID relevant to this TCAM DB get bank info
       * (e.g. used prefix in bank, bank owner, bank infomation per TCAM DB)
       */
      for(bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_id++)
      {
        if(! (((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) || (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)) 
             && (bank_id == 0)) ) {
            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_idx, bank_id, &is_used);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 20, exit);
        }

        if((((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) || (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)) 
             && (bank_id == 0)) /* Use arbitrarily bank 0 to print KBP DBs */ 
           || is_used)
        {
          info->db[db_idx].db_tcam.bank[bank_id].valid = 1;

          /* Prefix is relevant only if DB type is TCAM */
          if(info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_TCAM 
             || ( ( info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE ) && !(fp_db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS))
             || info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_EGRESS)
          {
            /* Get profile index for this TCAM DB, then
             * get bank owner according to profile from tcam db
             */
            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit,tcam_db_idx, &entry_size);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 25, exit);
            info->db[db_idx].db_tcam.nof_keys_per_db = 
              (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ?
              ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX : 
              ARAD_PP_FP_KEY_NOF_KEYS_PER_DB_MAX - 1;

            for(key_idx = 0; key_idx < info->db[db_idx].db_tcam.nof_keys_per_db; key_idx++)
            {
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_idx, key_idx, &profile_idx);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
              info->db[db_idx].db_tcam.access_profile_id[key_idx] = profile_idx;
            }
          }

          /* bank information per tcam db - differenece between TCAM and DT */
          if((info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE)
             || (info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_FLP))
          {
                /* There is no use of actual entries in case of DT */
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_entries.get(
                          unit,
                          stage,
                          db_idx,
                          &fp_entry
                      );
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);

                info->db[db_idx].db_tcam.bank[bank_id].entries_used = fp_entry.nof_db_entries;
                info->db[db_idx].db_tcam.bank[bank_id].entries_free = 0;
          }
          else
          {
                /* bank information per tcam db - only for TCAM */
                res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(
                        unit,
                        bank_id,
                        &(info->db[db_idx].db_tcam.bank[bank_id].entries_free)
                      );
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);
                info->db[db_idx].db_tcam.bank[bank_id].entries_used = db_nof_entries[bank_id];
          }

          /* bank information per tcam db - also for Direct Table*/
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(
                  unit,
                  tcam_db_idx,
                  &(info->db[db_idx].db_tcam.bank[bank_id].action_tbl_bmp)
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
          
          /* For each bank, add prefix and increase nof DBs */
          if(info->bank[bank_id].nof_dbs >= SOC_PPC_FP_MAX_NOF_DBS_PER_BANK)
          {
            SOC_SAND_SET_ERROR_CODE(SOC_PPC_FP_NOF_DBS_PER_BANK_OUT_OF_RANGE_ERR, 70, exit);
          }
          info->bank[bank_id].db[info->bank[bank_id].nof_dbs].db_id = db_idx;
          info->bank[bank_id].db[info->bank[bank_id].nof_dbs].prefix.val = prefix.bits;
          info->bank[bank_id].db[info->bank[bank_id].nof_dbs].prefix.nof_bits = prefix.length;
          info->bank[bank_id].db[info->bank[bank_id].nof_dbs].nof_entries = db_nof_entries[bank_id];
          info->bank[bank_id].nof_dbs++;
        }
      }

      /* Key size varies according to DB type */
      if(info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_TCAM 
         || info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_EGRESS)
      {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit,tcam_db_idx, &entry_size);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 60, exit);
        
        info->db[db_idx].key_size = ARAD_SW_DB_ENTRY_SIZE_ID_TO_BITS(entry_size);
      }
      else if(info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_DIRECT_TABLE)
      {
        info->db[db_idx].key_size = (fp_db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? 19 : SOC_DPP_DEFS_GET(unit, tcam_big_bank_key_nof_bits);
      }
      else if(info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION)
      {
        info->db[db_idx].key_size = SOC_PPC_FP_DB_TYPE_TCAM_KEY_MAX_LENGTH_DIRECT_EXTRACTION;
      }
      for (is_tm = FALSE; is_tm <= (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF); is_tm++) {
        res = arad_pmf_prog_select_pmf_pgm_borders_get(
                  unit,
                  stage,
                  is_tm, 
                  &pgm_idx_min,
                  &pgm_idx_max
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);

        /* Key Bitmap - per program */
        for(pgm_idx = pgm_idx_min; pgm_idx < pgm_idx_max; pgm_idx++)
        {
          /* if program is used by this DB update which keys are used */
          if(SHR_BITGET(pmf_db_info.progs, pgm_idx) > 0)
          {
            /* These 2 lines are not really needed. Just to avoid coverity defect */
            uint32 prog_used_cycle_bmp_lcl[1];
            prog_used_cycle_bmp_lcl[0] = pmf_db_info.prog_used_cycle_bmp;

            /* cycle for this program */
            cycle_idx = SHR_BITGET(prog_used_cycle_bmp_lcl, pgm_idx) ? 1 : 0;

            /* Key 0 (Key A cycle 0) is always used */
            diag_key_bitmap[stage][pgm_idx][cycle_idx] |= (cycle_idx == 0) ? 0x3 : 0;

            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit,tcam_db_idx, &entry_size);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 62, exit);
            /* which zones are used according to DB size */
            switch(entry_size)
            {
            case ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS:
              diag_key_bitmap[stage][pgm_idx][cycle_idx] |= (0x3 << (pmf_db_info.used_key[pgm_idx][0] * 2));
              diag_key_bitmap[stage][pgm_idx][cycle_idx] |= (0x3 << (pmf_db_info.used_key[pgm_idx][1] * 2));
              break;
            case ARAD_TCAM_BANK_ENTRY_SIZE_160_BITS:
              diag_key_bitmap[stage][pgm_idx][cycle_idx] |= (0x3 << (pmf_db_info.used_key[pgm_idx][0] * 2));
              break;
            case ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS:
            default:
              
              diag_key_bitmap[stage][pgm_idx][cycle_idx] |= (0x1 << (pmf_db_info.used_key[pgm_idx][0] * 2));
              break;
            }
          }
        }
      } /*is_tm*/
      /* Check if cascaded action is used (it doesn't appear in the FESs).
       * If so, set the first action in the action set to be change key.
       */
      for (action_idx = 0; action_idx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; action_idx++) 
      {
          if(ARAD_PP_FP_FEM_IS_ACTION_NOT_REQUIRE_FEM(fp_db_info.action_types[action_idx])) 
          {
              info->db[db_idx].db_tcam.action[action_idx].valid = 1;
              info->db[db_idx].db_tcam.action[action_idx].action_type = fp_db_info.action_types[action_idx];
              info->db[db_idx].db_tcam.nof_actions++;

              break;
          }
      }
    }
  } /* for(db_idx)*/

  /* 
   *  Per CE get key information for all databases 
   * (e.g. qualifier type, location)
   */
  ARAD_ALLOC(diag_db_key, ARAD_PP_FP_RESOURCE_DIAG_DB_KEY, SOC_PPC_FP_NOF_DBS, "");
  ARAD_ALLOC(diag_db_key_args, ARAD_PP_FP_RESOURCE_DIAG_DB_KEY_ARGS, SOC_PPC_FP_NOF_DBS, "arad_pp_fp_resource_diag_get_unsafe.diag_db_key_args");
  sal_memset(diag_db_key, 0x0, sizeof(ARAD_PP_FP_RESOURCE_DIAG_DB_KEY) * SOC_PPC_FP_NOF_DBS);
  sal_memset(diag_db_key_args, 0x0, sizeof(ARAD_PP_FP_RESOURCE_DIAG_DB_KEY_ARGS) * SOC_PPC_FP_NOF_DBS);
  sal_memset(diag_ce_bitmap, 0x0, sizeof(uint32) * SOC_PPC_NOF_FP_DATABASE_STAGES * ARAD_PMF_NOF_PROGS * ARAD_PMF_NOF_CYCLES);
  sal_memset(diag_prog_bitmap, 0x0, sizeof(uint32) * SOC_PPC_NOF_FP_DATABASE_STAGES);

  ARAD_ALLOC(key_cur_loc, uint32, SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES * SOC_PPC_NOF_FP_DATABASE_STAGES, "arad_pp_fp_resource_diag_get_unsafe.key_cur_loc");
  sal_memset(key_cur_loc, 0x0, sizeof(uint32) * SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES * SOC_PPC_NOF_FP_DATABASE_STAGES);

  ARAD_ALLOC(db_used_program, uint32, ARAD_PMF_NOF_DBS, "arad_pp_fp_resource_diag_get_unsafe.db_used_program");
  for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; db_idx++)
  {
      db_used_program[db_idx] = ARAD_PMF_NOF_PROGS;

      for(stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage++)
      {
        /* in general, the MSB zone is at the end of LSB. Special case in LSB: it is actually the second key */
        msb_loc = (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB)? 0: ARAD_PMF_LOW_LEVEL_ZONE_SIZE_PER_STAGE;
        key_cur_loc[(stage * SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES) + (db_idx * ARAD_PP_FP_KEY_NOF_ZONES) + ARAD_PP_FP_KEY_ZONE_MSB_0] = msb_loc;
        key_cur_loc[(stage * SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES) + (db_idx * ARAD_PP_FP_KEY_NOF_ZONES) + ARAD_PP_FP_KEY_ZONE_MSB_1] = msb_loc;
      }
  }

  for(stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage++)
  {
    for (is_tm = FALSE; is_tm <= (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF); is_tm++) {
      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                unit,
                stage,
                is_tm, 
                &pgm_idx_min,
                &pgm_idx_max
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);
     for(pgm_idx = pgm_idx_min; pgm_idx < pgm_idx_max; pgm_idx++)
      {
        for(cycle_idx = 0; cycle_idx < ARAD_PMF_LOW_LEVEL_NOF_CYCLES; cycle_idx++)
        {
          /*
           * Calulate CE from 15 to 0 and then from 31 to 16 
           * ----------------------------------------------- 
           * | 16      MSB       31 | 0        LSB       15|   <--
           * ----------------------------------------------- 
           */
          ce_id = 0;
          for(lsb_msb = 0; lsb_msb < ARAD_PMF_LOW_LEVEL_NOF_LSB_MSB; lsb_msb++)
          {
            for(ce_idx = 0; ce_idx < ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB; ce_idx++)
            {
              ce_id = 
                (ARAD_PP_FP_KEY_BIT_TYPE_LSB == lsb_msb) ? 
                (ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB - ce_idx - 1) : 
                (ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB * 2 - ce_idx - 1);

              sal_memset(&pgm_ce, 0x0, sizeof(ARAD_PMF_CE));
              res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_ce.get(
                      unit, 
                      stage, 
                      pgm_idx, 
                      cycle_idx, 
                      ce_id, 
                      &pgm_ce
                    );
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 80, exit); 

              /* if CE is used then retrieve qualifier */
              if(pgm_ce.is_used)
              {
                  /* Get PMF info per DB - for key IDs */
                  sal_memset(&pmf_db_info, 0x0, sizeof(ARAD_PMF_DB_INFO));
                  res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.get(
                            unit, 
                            stage, 
                            pgm_ce.db_id, 
                            &pmf_db_info
                        );
                  SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 110, exit); 

                  if(mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES) {
                    info->available.is_used = TRUE;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].is_used = TRUE;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].is_msb = pgm_ce.is_msb;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].is_second_key = pgm_ce.is_second_key;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].lsb = pgm_ce.lsb;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].msb = pgm_ce.msb;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].qual_lsb = pgm_ce.qual_lsb;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].qual_type = pgm_ce.qual_type;

                    key_idx = pmf_db_info.is_320b ? (pgm_ce.is_second_key ? 1 : 0) : 0;
                    info->available.pgm_ce[stage][pgm_idx][pgm_ce.db_id][cycle_idx][ce_id].key_id = pmf_db_info.used_key[pgm_idx][key_idx];

                    /* Per stage - Per program - Per Cycle - Get KEY resource bitmap from SW DB and compare to HW */
                    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(unit, stage, pgm_idx, cycle_idx, &key_rsrc);
                    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 410, exit);

                    info->available.key[stage][pgm_idx][cycle_idx][pmf_db_info.used_key[pgm_idx][key_idx]].is_used = TRUE;
                    info->available.key[stage][pgm_idx][cycle_idx][pmf_db_info.used_key[pgm_idx][key_idx]].is_lsb_db =
                          info->db[pgm_ce.db_id].type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION ? ( (key_rsrc >> (key_idx *2)) & 0x1 ) : TRUE;
                    info->available.key[stage][pgm_idx][cycle_idx][pmf_db_info.used_key[pgm_idx][key_idx]].is_msb_db =
                          info->db[pgm_ce.db_id].type == SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION ? ( (key_rsrc >> (key_idx *2)) & 0x2 ) : pgm_ce.is_msb;
                }

                if(pgm_ce.db_id >= SOC_PPC_FP_NOF_DBS)
                {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 90, exit);
                }

                if(info->db[pgm_ce.db_id].nof_ces >= SOC_PPC_FP_NOF_QUALS_PER_DB_MAX)
                {
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_NOF_QUALS_PER_DB_OUT_OF_RANGE_ERR, 100, exit);
                }

                /* Set CE and Key bitmap for this program and cycle, for later validation */
                key_idx = pgm_ce.is_second_key ? 1 : 0;
                diag_ce_bitmap[stage][pgm_idx][cycle_idx] |= (1 << ce_id);
                
                diag_prog_bitmap[stage] |= (1 << pgm_idx);

                if(db_used_program[pgm_ce.db_id] != ARAD_PMF_NOF_PROGS 
                       && db_used_program[pgm_ce.db_id] != pgm_idx) {
                        continue;
                }

                db_used_program[pgm_ce.db_id] = pgm_idx;

                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].valid = 1;
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].qual_type = pgm_ce.qual_type;
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].is_second_key = pgm_ce.is_second_key;
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].is_msb = pgm_ce.is_msb;
                
                /* The location of the Qualifier LSB in the key starts at 0 (key_cur_loc) and is increased
                 * by the size of the CE (pgm_ce.msb + 1) every time this calculation is done. 
                 * The MSB of the Qualifier in the key is the MSB of the CE respective to the LSB location 
                 * inside the key. 
                 */ 
                key_idx = pgm_ce.is_second_key ? 2 : 0;
                key_idx += pgm_ce.is_msb ? 1 : 0;
                
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].key_loc.lsb = key_cur_loc[(stage * SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES) + (pgm_ce.db_id * ARAD_PP_FP_KEY_NOF_ZONES) + key_idx]; 
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].key_loc.msb = key_cur_loc[(stage * SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES) + (pgm_ce.db_id * ARAD_PP_FP_KEY_NOF_ZONES) + key_idx] + pgm_ce.msb; 
                key_cur_loc[(stage * SOC_PPC_FP_NOF_DBS * ARAD_PP_FP_KEY_NOF_ZONES) + (pgm_ce.db_id * ARAD_PP_FP_KEY_NOF_ZONES) + key_idx] += pgm_ce.msb + 1;
                
                /* The location of the qualifier LSB inside the CE is the value of pgm_ce.qual_lsb (which is
                 * the number of Lost Bits after pgm_ce.lsb).
                 * The MSB of the qualifier should be relative to its LSB and thus the qualifier LSB is
                 * incremented by the length of the qualifier minus 1.
                 */ 
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].qual_loc.lsb = pgm_ce.qual_lsb;
                diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces].qual_loc.msb = pgm_ce.qual_lsb + (pgm_ce.msb - pgm_ce.lsb);

                diag_db_key[pgm_ce.db_id].nof_ces++;


              /* Determine if CE is 32 bit according to CE ID */
              is_32b_ce = arad_pmf_low_level_ce_is_32b_ce(unit, stage, ce_id);

              /* If key has been already constructed by a different program -
               * validate for the same DB it's the same key
               */
              key_idx = pgm_ce.is_second_key ? 1 : 0;
              if(diag_db_key_args[pgm_ce.db_id].complete)
              {
                /* If diagnostics mode requires validation as well */
                if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG)
                {
                  /* Check for each CE if it is identical */
                  qual_idx = diag_db_key[pgm_ce.db_id].nof_ces - 1;
                  if( sal_memcmp( &(diag_db_key[pgm_ce.db_id].key_qual[qual_idx]), 
                                  &(info->db[pgm_ce.db_id].key_qual[qual_idx]), 
                                  sizeof(SOC_PPC_FP_RESOURCE_KEY) != 0 ))
                  {
                    error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY].params;
                    param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_COHERENCY;
                    if(!error_params[param_idx].is_error)
                    {
                      value_idx = 0;
                      error_params[param_idx].is_error = TRUE;
                      error_params[param_idx].value[value_idx++] = diag_db_key_args[pgm_ce.db_id].program_id;
                      error_params[param_idx].value[value_idx++] = pgm_idx;
                      error_params[param_idx].value[value_idx++] = pmf_db_info.used_key[diag_db_key_args[pgm_ce.db_id].program_id][key_idx];
                      error_params[param_idx].value[value_idx++] = pmf_db_info.used_key[pgm_idx][key_idx];
                      error_params[param_idx].value[value_idx++] = diag_db_key_args[pgm_ce.db_id].cycle_id;
                      error_params[param_idx].value[value_idx++] = cycle_idx;
                      error_params[param_idx].value[value_idx++] = qual_idx;
                      error_params[param_idx].value[value_idx++] = ce_id;
                      error_params[param_idx].value[value_idx++] = pgm_ce.db_id;
                      error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(pgm_ce.db_id);
                    }
                  }
                } /* if Diagnostics mode */
              }
              else
              {
                sal_memcpy(&(info->db[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces - 1]),
                           &(diag_db_key[pgm_ce.db_id].key_qual[diag_db_key[pgm_ce.db_id].nof_ces-1]),
                           sizeof(SOC_PPC_FP_RESOURCE_KEY));
                info->db[pgm_ce.db_id].nof_ces = diag_db_key[pgm_ce.db_id].nof_ces;

                /* save for later local validation indication */
                diag_db_key_args[pgm_ce.db_id].cycle_id = cycle_idx;
                diag_db_key_args[pgm_ce.db_id].program_id = pgm_idx;
                diag_db_key_args[pgm_ce.db_id].lsb_msb = lsb_msb;
              }

              /* If diagnostics mode requires validation as well */
              if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG)
              {
                /* First get CE type (internal/signal or header) - then read CE from HW accordingly */
                found = FALSE;
                res = arad_pmf_ce_header_info_find(
                        unit,
                        pgm_ce.qual_type,
                        stage,
                        &found,
                        &header_qual_info
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);

                /* found is an indication to the qualifier being a header type (vs. signal/internal) */
                if(found)
                {
                  /* Get qualifier information - which bits to take and how many */
                  res = arad_pmf_ce_packet_header_entry_get_unsafe(
                          unit, 
                          stage, 
                          pgm_idx, 
                          pmf_db_info.used_key[pgm_idx][key_idx], 
                          (ce_id % ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB), 
                          lsb_msb, 
                          cycle_idx, 
                          &header_info
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);

                  /* The lost bits are only in the LSBs, up to the resolution */
                  resolution_ce_in_bits = is_32b_ce ? 8 : 4;
                  nof_lost_lsb_bits = (resolution_ce_in_bits - ((header_qual_info.lsb + 1) % resolution_ce_in_bits)) % resolution_ce_in_bits;
                  
                  /* Verify that the CE is the same (qualifier,
                   * offset and length) in both SW and HW
                   */
                  if((header_info.nof_bits != (pgm_ce.msb + 1)) 
                     || (pgm_ce.lsb != nof_lost_lsb_bits))
                  {               
                    error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY].params;
                    param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_SW_HW;
                    if(!error_params[param_idx].is_error)
                    {
                      value_idx = 0;
                      error_params[param_idx].is_error = TRUE;
                      error_params[param_idx].value[value_idx++] = pgm_idx;
                      error_params[param_idx].value[value_idx++] = header_qual_info.qual_type;
                      error_params[param_idx].value[value_idx++] = pmf_db_info.used_key[pgm_idx][key_idx];
                      error_params[param_idx].value[value_idx++] = cycle_idx;
                      error_params[param_idx].value[value_idx++] = ce_id;
                      error_params[param_idx].value[value_idx++] = pgm_ce.lsb;
                      error_params[param_idx].value[value_idx++] = nof_lost_lsb_bits;
                      error_params[param_idx].value[value_idx++] = pgm_ce.msb + 1;
                      error_params[param_idx].value[value_idx++] = header_info.nof_bits;
                    }
                  }
                }
                
                /* if not header type, check if internal */
                if(!found)
                {
                  /* Check if the qualifier is internal/signal */
                  res = arad_pmf_ce_internal_field_info_find(
                          unit, 
                          pgm_ce.qual_type, 
                          stage, 
                          lsb_msb, 
                          &found, 
                          &irpp_qual_info
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);

                  /* found is an indication to the qualifier being an internal type (PP) */
                  if(found)
                  {
                    /* Get internal qualifier information - which bits and how many */
                    res = arad_pmf_ce_internal_info_entry_get_unsafe(
                            unit, 
                            stage, 
                            pgm_idx, 
                            pmf_db_info.used_key[pgm_idx][key_idx], 
                            (ce_id % ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB), 
                            lsb_msb, 
                            cycle_idx, 
                            irpp_qual_info.info.irpp_field,
                            &qual_lsb, 
                            &nof_bits, 
                            &irpp_field
                          );
                    SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);
                    if(irpp_qual_info.info.irpp_field != irpp_field)
                    {
                        /* If Qual type was not found in the second search - report error */
                        SOC_SAND_SET_ERROR_CODE(SOC_PPC_FP_QUAL_TYPES_END_OF_LIST_ERR, 155, exit);
                    }

                    /* Verify that the CE is the same (qualifier,
                     * offset and length) in both SW and HW */
                    else
                    {
                        if((pgm_ce.qual_lsb != qual_lsb) 
                           || ((pgm_ce.msb - pgm_ce.lsb + 1) != nof_bits))
                        {
                          error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY].params;
                          param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_SW_HW;
                          if(!error_params[param_idx].is_error)
                          {
                            value_idx = 0;
                            error_params[param_idx].is_error = TRUE;
                            error_params[param_idx].value[value_idx++] = pgm_idx;
                            error_params[param_idx].value[value_idx++] = irpp_field;
                            error_params[param_idx].value[value_idx++] = pmf_db_info.used_key[pgm_idx][key_idx];
                            error_params[param_idx].value[value_idx++] = cycle_idx;
                            error_params[param_idx].value[value_idx++] = ce_id;
                            error_params[param_idx].value[value_idx++] = pgm_ce.qual_lsb;
                            error_params[param_idx].value[value_idx++] = qual_lsb;
                            error_params[param_idx].value[value_idx++] = (pgm_ce.msb - pgm_ce.lsb + 1);
                            error_params[param_idx].value[value_idx++] = nof_bits;
                          }
                        }
                    }
                  }
                }

                /* if not header and not internal - check if not used */
                if(!found)
                {
                  res = arad_pmf_ce_nop_entry_get_unsafe(
                          unit, 
                          stage, 
                          pgm_idx, 
                          pmf_db_info.used_key[pgm_idx][key_idx], 
                          (ce_id % ARAD_PMF_LOW_LEVEL_NOF_CE_IN_PROG_LSB), 
                          lsb_msb, 
                          pgm_ce.is_second_key, 
                          &not_valid, 
                          &ce_loc
                        );
                  SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);

                  /* if not_valid - means the CE is not used despite the
                   * SW indication that it is - Report Error */
                  if(not_valid)
                  {
                    error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY].params;
                    param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_CE_USED;
                    if(!error_params[param_idx].is_error)
                    {
                      value_idx = 0;
                      error_params[param_idx].is_error = TRUE;
                      error_params[param_idx].value[value_idx++] = pgm_idx;
                      error_params[param_idx].value[value_idx++] = pmf_db_info.used_key[pgm_idx][key_idx];
                      error_params[param_idx].value[value_idx++] = cycle_idx;
                      error_params[param_idx].value[value_idx++] = ce_id;
                      error_params[param_idx].value[value_idx++] = pgm_ce.is_used ? 1 : 0;
                      error_params[param_idx].value[value_idx++] = 0;
                    }
                  }
                }
              } /* If diagnostics mode */

              } /* if CE is used */
            } /* for(ce_idx) */
          } /* for(lsb_msb) */
        } /* for(cycle_idx) */

        for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; db_idx++)
        {
          diag_db_key_args[db_idx].complete = (diag_db_key[db_idx].nof_ces > 0) ? TRUE : FALSE;
          sal_memset(&(diag_db_key[db_idx]), 0x0, sizeof(ARAD_PP_FP_RESOURCE_DIAG_DB_KEY));
        }
      } /* for(pgm_idx) */
    } /* for(is_tm) */
  } /* for(stage) */
 
  /* 
   * From each FEM get action information per DB 
   * FEMs are only relevant for the Ingress 
   */
  ARAD_ALLOC(fes_prio, ARAD_PP_FP_RESOURCE_DIAG_ACTION_PRIORITY, SOC_PPC_NOF_FP_ACTION_TYPES, "arad_pp_fp_resource_diag_get_unsafe.fes_prio");
  ARAD_ALLOC(action_prio, ARAD_PP_FP_RESOURCE_DIAG_ACTION_PRIORITY, SOC_PPC_NOF_FP_ACTION_TYPES, "arad_pp_fp_resource_diag_get_unsafe.action_prio");

  sal_memset(action_prio, 0x0, sizeof(ARAD_PP_FP_RESOURCE_DIAG_ACTION_PRIORITY) * SOC_PPC_NOF_FP_ACTION_TYPES);
  sal_memset(diag_fes_bitmap, 0x0, sizeof(uint32) * SOC_PPC_NOF_FP_DATABASE_STAGES * ARAD_PMF_NOF_PROGS);
  for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; db_idx++)
  {
      db_used_program[db_idx] = ARAD_PMF_NOF_PROGS;
  }
  
  stage = SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF;
  for(cycle_idx = 0; cycle_idx < ARAD_PMF_LOW_LEVEL_NOF_CYCLES; cycle_idx++)
  {
    /* 
     * Order of actions is :
     * FESs 0 - 15, FEMs 0-7, FESs 16-31, FEMs 8-15
     */
    for (is_tm = FALSE; is_tm <= (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF && 
    (soc_property_get(unit, spn_ITMH_PROGRAMMABLE_MODE_ENABLE, FALSE)|| (soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "support_petra_itmh", 0)))); is_tm++) {
    /* Each cycle starts with 16 FESs */
      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                unit,
                stage,
                is_tm, 
                &pgm_idx_min,
                &pgm_idx_max
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);
      for(pgm_idx = pgm_idx_min; pgm_idx < pgm_idx_max; pgm_idx++)
      {
        if(stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF && mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES) {
          info->available.is_used = TRUE;

          /* The number of fess is in both of the cycles */
          info->available.fes_free[pgm_idx][cycle_idx].fes_free = ARAD_PMF_LOW_LEVEL_NOF_FESS /ARAD_PMF_NOF_CYCLES;
          
          /* latter we will valid the relevanric ones */
          info->available.fes_free[pgm_idx][cycle_idx].is_used = FALSE;
        }
        
        /* FES priority - used for diagnostics - validated per program */
        sal_memset(fes_prio, 0x0, sizeof(ARAD_PP_FP_RESOURCE_DIAG_ACTION_PRIORITY) * SOC_PPC_NOF_FP_ACTION_TYPES);

        /* 
         * Get the list of actions per TCAM DB: for each FES in the PMF, get the DB ID 
         * and get the action info for that DB.
         */
        for(fes_group_idx = 0; fes_group_idx < ARAD_PMF_LOW_LEVEL_NOF_FESS_PER_GROUP; fes_group_idx++)
        {
          fes_idx = (cycle_idx * ARAD_PMF_LOW_LEVEL_NOF_FESS_PER_GROUP) + fes_group_idx;

          sal_memset(&fes_info, 0x0, sizeof(ARAD_PMF_FES));
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_fes.get(
                  unit, 
                  stage, 
                  pgm_idx, 
                  fes_idx, 
                  &fes_info
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 170, exit); 

          if(fes_info.is_used)
          {
            /* Set FES bitmap for this program, for later validation */
            diag_fes_bitmap[stage][pgm_idx] |= (1 << fes_idx);
            
            if(fes_info.db_id >= SOC_PPC_FP_NOF_DBS)
            {
              SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 180, exit);
            }

            if(db_used_program[fes_info.db_id] == ARAD_PMF_LOW_LEVEL_NOF_PROGS 
               || db_used_program[fes_info.db_id] == pgm_idx)
            {
                /* Several programs can use the same DB, and thus if FESs for a
                 * certain DB were collected from some program, no need to do it 
                 * again for another program. 
                 */
                db_used_program[fes_info.db_id] = pgm_idx;
                if(info->db[fes_info.db_id].db_tcam.nof_actions >= SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX)
                {
                  /* Here the actions count in DB will be incremented. 
                   * If the number of actions in DB is already the maximum,
                   * it should not increase above this limit, thus error is given.
                   */
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_NOF_ACTIONS_PER_DB_OUT_OF_RANGE_ERR, 190, exit);
                }
                info->db[fes_info.db_id].db_tcam.action[info->db[fes_info.db_id].db_tcam.nof_actions].valid = 1;
                info->db[fes_info.db_id].db_tcam.action[info->db[fes_info.db_id].db_tcam.nof_actions].action_type = fes_info.action_type;
                
                info->db[fes_info.db_id].db_tcam.nof_actions++;
            }
          }
          
          if(fes_info.is_used && mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES)
          {
            /* Per FES index verify that it conforms with the data in the HW */
            sal_memset(&fes_input_info, 0x0, sizeof(ARAD_PMF_FES_INPUT_INFO));
            res = arad_pmf_db_fes_get_unsafe(
                    unit, 
                    pgm_idx,
                    fes_idx,
                    &fes_input_info
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 200, exit); 

            /* retrieve information for the verification of the action LSB */
            sal_memset(&fp_db_info, 0x0, sizeof(SOC_PPC_FP_DATABASE_INFO));

            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
                    unit,
                    stage,
                    fes_info.db_id,
                    &fp_db_info
                  );
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 210, exit); 

            /*arad_pp_fp_action_to_lsbs*/
            sal_memset(action_lsbs, 0x0, sizeof(uint32) * (SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX + 1));
            sal_memset(action_lengths, 0x0, sizeof(uint32) * (SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX + 1));

            res = arad_pp_fp_action_to_lsbs(
                    unit, 
                    stage, 
                    ((fp_db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? TRUE : FALSE ),
                    fp_db_info.action_types, 
                    fp_db_info.action_widths,
                    action_lsbs,
                    action_lengths,
                    &fem_act_size,
                    &nof_actions,
                    &success
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 220, exit); 
            if (success != SOC_SAND_SUCCESS) {
              LOG_ERROR(BSL_LS_SOC_FP,
                        (BSL_META_U(unit,
                                    "Unit %d Invalid action composition.\n\r"),
                         unit));
                SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 193, exit);
            }

            for(action_idx = 0; action_idx < nof_actions; action_idx++)
            {
                if(fes_input_info.action_type == fp_db_info.action_types[action_idx])
                {
                    break;
                }
            }

            --(info->available.fes_free[pgm_idx][cycle_idx].fes_free);
            info->available.fes[pgm_idx][cycle_idx][fes_info.db_id][fes_idx].is_used = TRUE;
            info->available.fes[pgm_idx][cycle_idx][fes_info.db_id][fes_idx].action_type = fes_info.action_type;
            info->available.fes[pgm_idx][cycle_idx][fes_info.db_id][fes_idx].action_lsb = action_lsbs[action_idx];
            if(fes_info.is_used && mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG) {

                /* if match it means the valid indication in the HW and SW DON'T match or
                 * the action-type/action-LSBs are not the same
                 */
                if(action_idx < nof_actions 
                   && (fes_input_info.is_action_always_valid 
                       || fes_input_info.action_type != fes_info.action_type 
                       || ARAD_PP_FP_FEM_ACTION_LSB_TO_SHIFT(action_lsbs[action_idx]) != fes_input_info.shift))
                {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_ACTION].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_FES;
                  if(!error_params[param_idx].is_error)
                  {
                    value_idx = 0;
                    error_params[param_idx].is_error = TRUE;
                    error_params[param_idx].value[value_idx++] = pgm_idx;
                    error_params[param_idx].value[value_idx++] = fes_idx;
                    error_params[param_idx].value[value_idx++] = fes_info.db_id;
                    error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(fes_info.db_id);
                    error_params[param_idx].value[value_idx++] = fes_input_info.action_type;
                    error_params[param_idx].value[value_idx++] = fes_info.action_type;
                    error_params[param_idx].value[value_idx++] = fes_input_info.shift;
                    error_params[param_idx].value[value_idx++] = action_lsbs[action_idx];
                  }
                }

                /* retrieve information for the validation of FESs priority per program
                 * (should be increasing)
                 */
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.db_info.prio.get(
                        unit, 
                        stage, 
                        fes_info.db_id, 
                        &(fes_prio[fes_info.action_type].new_prio)
                      );
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 230, exit);

                if(fes_prio[fes_info.action_type].last_prio > fes_prio[fes_info.action_type].new_prio)
                {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_ACTION].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_PRIORITY;
                  if(!error_params[param_idx].is_error)
                  {
                    value_idx = 0;
                    error_params[param_idx].is_error = TRUE;
                    error_params[param_idx].value[value_idx++] = pgm_idx;
                    error_params[param_idx].value[value_idx++] = fes_info.action_type;
                    error_params[param_idx].value[value_idx++] = fes_prio[fes_info.action_type].db_id;
                    error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(fes_prio[fes_info.action_type].db_id);
                    error_params[param_idx].value[value_idx++] = fes_info.db_id;
                    error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(fes_info.db_id);
                    error_params[param_idx].value[value_idx++] = fes_prio[fes_info.action_type].is_fes;
                    error_params[param_idx].value[value_idx++] = TRUE;
                    error_params[param_idx].value[value_idx++] = fes_prio[fes_info.action_type].fes_fem_id;
                    error_params[param_idx].value[value_idx++] = fes_idx;
                    error_params[param_idx].value[value_idx++] = fes_prio[fes_info.action_type].last_prio;
                    error_params[param_idx].value[value_idx++] = fes_prio[fes_info.action_type].new_prio;
                  }
                }
                else
                {
                  /* Save the last FES */
                  fes_prio[fes_info.action_type].last_prio = fes_prio[fes_info.action_type].new_prio;
                  fes_prio[fes_info.action_type].is_fes = TRUE;
                  fes_prio[fes_info.action_type].fes_fem_id = fes_idx;
                  fes_prio[fes_info.action_type].db_id = fes_info.db_id;
                  fes_prio[fes_info.action_type].pgm_id = pgm_idx;
                }

                if(fes_prio[fes_info.action_type].new_prio > action_prio[fes_info.action_type].last_prio)
                {
                  action_prio[fes_info.action_type].last_prio = fes_prio[fes_info.action_type].new_prio;
                  action_prio[fes_info.action_type].is_fes = TRUE;
                  action_prio[fes_info.action_type].fes_fem_id = fes_idx;
                  action_prio[fes_info.action_type].db_id = fes_info.db_id;
                  action_prio[fes_info.action_type].pgm_id = pgm_idx;
                  action_prio[fes_info.action_type].entry_strength = 0;
                }
            }/* If Diagnostics mode */
          } /* If with available mode */

        } /* fes_idx */
      } /* pgm_idx */
    }
    /* Continue with 8 FEMs */
    for(fem_group_idx = 0; fem_group_idx < SOC_PPC_FP_NOF_MACROS; fem_group_idx++)
    {
       /*Scan all FEM programs, not includeing TM*/
      for(fem_pgm_idx = 0; fem_pgm_idx < (ARAD_PMF_NOF_FEM_PGMS-1); fem_pgm_idx++ )
      {
          fem_id = fem_group_idx + (cycle_idx * ARAD_PMF_LOW_LEVEL_NOF_FEMS_PER_GROUP);
          
          SOC_PPC_FP_FEM_ENTRY_clear(&fem_entry);
          res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fem_entry.get(
                  unit,
                  stage, 
                  fem_pgm_idx,
                  fem_id, 
                  &fem_entry
                );
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 240, exit); 

          if(fem_entry.db_id >= SOC_PPC_FP_NOF_DBS)
          {
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NDX_OUT_OF_RANGE_ERR, 250, exit);
          }
          if(mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES &&
              fem_entry.action_type[0] != SOC_PPC_FP_ACTION_TYPE_INVALID) {
            info->available.is_used = TRUE;
            info->available.fem[cycle_idx][fem_entry.db_id][fem_group_idx].is_used = TRUE;
            info->available.fem[cycle_idx][fem_entry.db_id][fem_group_idx].db_strength = fem_entry.db_strength;
            info->available.fem[cycle_idx][fem_entry.db_id][fem_group_idx].entry_strength = fem_entry.entry_strength;
            info->available.fem[cycle_idx][fem_entry.db_id][fem_group_idx].entry_id = fem_entry.entry_id;
            info->available.fem[cycle_idx][fem_entry.db_id][fem_group_idx].action_type = fem_entry.action_type[0];
          }

          /* In case of Direct Extraction */
          if(fem_entry.is_for_entry)
          {
            if(SOC_PPC_FP_DB_TYPE_DIRECT_EXTRACTION != info->db[fem_entry.db_id].type)
            {
              SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_ID_NOT_DIRECT_EXTRACTION_ERR, 260, exit);
            }

            res = arad_pp_fp_fem_configuration_de_get(
                    unit,
                    fem_group_idx, 
                    cycle_idx, 
                    fem_pgm_idx,
                    &fem_entry, 
                    &(info->db[fem_entry.db_id].db_de.de_entry[cycle_idx][fem_group_idx].actions[0]), 
                    &(info->db[fem_entry.db_id].db_de.de_entry[cycle_idx][fem_group_idx].qual_vals[0]) 
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 270, exit);

            info->db[fem_entry.db_id].db_de.valid[cycle_idx][fem_group_idx] = 1;
            info->db[fem_entry.db_id].db_de.fem_entry_id[cycle_idx][fem_group_idx] = fem_id;
            info->db[fem_entry.db_id].db_de.de_entry_id[cycle_idx][fem_group_idx] = fem_entry.entry_id;
            info->db[fem_entry.db_id].db_de.de_entry[cycle_idx][fem_group_idx].priority = fem_entry.entry_strength;
          }
          else
          {
            if(fem_entry.action_type[0] != SOC_PPC_FP_ACTION_TYPE_INVALID)
            {
              if(info->db[fem_entry.db_id].db_tcam.nof_actions >= SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX)
              {
                SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_NOF_ACTIONS_PER_DB_OUT_OF_RANGE_ERR, 280, exit);
              }

              /* Only the first action is relevant */
              info->db[fem_entry.db_id].db_tcam.action[info->db[fem_entry.db_id].db_tcam.nof_actions].valid = 1;
              info->db[fem_entry.db_id].db_tcam.action[info->db[fem_entry.db_id].db_tcam.nof_actions].action_type = fem_entry.action_type[0];
              
              info->db[fem_entry.db_id].db_tcam.nof_actions++;
            }
          }

          /* If diagnostics mode requires validation as well */
          if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG && 
             fem_entry.action_type[0] != SOC_PPC_FP_ACTION_TYPE_INVALID)
          {
                /* Validate FEM configuration in the HW matches the SW */
                SOC_PPC_FP_DIR_EXTR_ACTION_VAL_clear(&fem_info);
                res = arad_pp_fp_fem_configuration_de_get(
                        unit,
                        fem_id,
                        cycle_idx,
                        fem_pgm_idx,
                        &fem_entry,
                        &fem_info,
                        &qual_info
                      );
                SOC_SAND_CHECK_FUNC_RESULT(res, 290, exit);

                if(fem_info.type != fem_entry.action_type[0])
                {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_ACTION].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_FEM;
                  if(!error_params[param_idx].is_error)
                  {
                    value_idx = 0;
                    error_params[param_idx].is_error = TRUE;
                    error_params[param_idx].value[value_idx++] = pgm_idx;
                    error_params[param_idx].value[value_idx++] = fem_id;
                    error_params[param_idx].value[value_idx++] = fem_entry.db_id;
                    error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(fem_entry.db_id);
                    error_params[param_idx].value[value_idx++] = fem_entry.action_type[0];
                    error_params[param_idx].value[value_idx++] = fem_info.type;
                  }
                }
                else 
                {
                    fes_prio[fem_info.type].new_prio = fem_entry.db_strength;
                    if((action_prio[fem_info.type].last_prio > fes_prio[fem_info.type].new_prio) ||
                       (action_prio[fem_info.type].last_prio == fes_prio[fem_info.type].new_prio && action_prio[fem_info.type].entry_strength > fem_entry.entry_strength) )
                    {
                      error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_ACTION].params;
                      param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_ACTION_PRIORITY;
                      if(!error_params[param_idx].is_error)
                      {
                        value_idx = 0;
                        error_params[param_idx].is_error = TRUE;
                        error_params[param_idx].value[value_idx++] = action_prio[fem_info.type].pgm_id;
                        error_params[param_idx].value[value_idx++] = fem_entry.action_type[0];
                        error_params[param_idx].value[value_idx++] = action_prio[fem_info.type].db_id;
                        error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(action_prio[fem_info.type].db_id);
                        error_params[param_idx].value[value_idx++] = fem_entry.entry_id;
                        error_params[param_idx].value[value_idx++] = ARAD_PP_FP_DB_ID_TO_TCAM_DB(fem_entry.entry_id);
                        error_params[param_idx].value[value_idx++] = action_prio[fem_info.type].is_fes;
                        error_params[param_idx].value[value_idx++] = action_prio[fem_info.type].fes_fem_id;
                        error_params[param_idx].value[value_idx++] = FALSE;
                        error_params[param_idx].value[value_idx++] = fem_id;
                        error_params[param_idx].value[value_idx++] = action_prio[fem_info.type].last_prio;
                        error_params[param_idx].value[value_idx++] = fes_prio[fem_info.type].new_prio;
                        error_params[param_idx].value[value_idx++] = fes_prio[fem_info.type].entry_strength;
                        error_params[param_idx].value[value_idx++] = fem_entry.entry_strength;
                      }
                    }
                    else
                    {
                      action_prio[fem_info.type].last_prio = action_prio[fem_info.type].new_prio;
                      action_prio[fem_info.type].is_fes = FALSE;
                      action_prio[fem_info.type].fes_fem_id = fem_id;
                      action_prio[fem_info.type].db_id = fem_entry.db_id;
                      action_prio[fem_info.type].pgm_id = ARAD_PMF_LOW_LEVEL_NOF_PROGS;
                      action_prio[fem_info.type].entry_strength = fem_entry.entry_strength;
                    }
                }

          } /* If diagnostics */
        }
    } /* for(fem_group_idx) */
  } /* for(cycle_idx */

  /* 
   * Per DB get the action location in TCAM according to lsbs 
   * This block should be performed here after getting the action types 
   * from the FES info array 
   */
  for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; db_idx++)
  {
    if(SOC_PPC_FP_DB_TYPE_TCAM != info->db[db_idx].type &&
       SOC_PPC_FP_DB_TYPE_FLP != info->db[db_idx].type && 
       SOC_PPC_FP_DB_TYPE_EGRESS != info->db[db_idx].type && 
       SOC_PPC_FP_DB_TYPE_DIRECT_TABLE != info->db[db_idx].type)
    {
      continue;
    }

    sal_memset(action_types, 0x0, sizeof(SOC_PPC_FP_ACTION_TYPE) * SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX);
    sal_memset(action_lsbs, 0x0, sizeof(uint32) * (SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX + 1));
    sal_memset(action_lengths, 0x0, sizeof(uint32) * (SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX + 1));
    sal_memset(&fp_db_info, 0x0, sizeof(SOC_PPC_FP_DATABASE_INFO));

    res = sw_state_access[unit].dpp.soc.arad.tm.pmf.fp_info.db_info.get(
            unit,
            info->db[db_idx].stage,
            db_idx,
            &fp_db_info
          );
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 210, exit); 

    for(action_idx = 0; action_idx < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; action_idx++)
    {
      if(action_idx >= info->db[db_idx].db_tcam.nof_actions)
      {
          info->db[db_idx].db_tcam.action[action_idx].action_type = SOC_PPC_FP_ACTION_TYPE_INVALID;
      }

      /* Get the action type from each action we got earlier */
      action_types[action_idx] = info->db[db_idx].db_tcam.action[action_idx].action_type;
    }

    /* Special cases for FLP and Egress */
    if (info->db[db_idx].stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) {
        info->db[db_idx].db_tcam.nof_actions = 1; /* always one action set per DB */
        for(action_idx = 0; action_idx < info->db[db_idx].db_tcam.nof_actions; action_idx++)
        {
            action_types[action_idx] = fp_db_info.action_types[action_idx]; 
            info->db[db_idx].db_tcam.action[action_idx].valid = 1;
            info->db[db_idx].db_tcam.action[action_idx].action_type = action_types[action_idx];
        }
    } else if(info->db[db_idx].stage == SOC_PPC_FP_DATABASE_STAGE_EGRESS) {
        int i;
        info->db[db_idx].db_tcam.nof_actions = 0;
        action_idx = 0;
        for (i = 0; i < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; i++) {
            if (fp_db_info.action_types[i] != SOC_PPC_FP_ACTION_TYPE_INVALID) {
                action_types[action_idx] = fp_db_info.action_types[i];
                info->db[db_idx].db_tcam.action[action_idx].action_type = action_types[action_idx];
                info->db[db_idx].db_tcam.action[action_idx].valid = 1;
                action_idx++;
            }
        }
        info->db[db_idx].db_tcam.nof_actions = action_idx;
    }

    fem_act_size = ARAD_TCAM_NOF_ACTION_SIZES;
    nof_actions = 0;
    /*
     * Get LSBs of actions as placed per their order within the data base. This is not
     * necessarily the same order as on fes table (because same actions are forced
     * the be ordered by priority and, therefore, are moved from their original placement.
     * See arad_pp_fp_fem_insert_unsafe()
     */
    res = arad_pp_fp_action_to_lsbs(
              unit, 
              info->db[db_idx].stage, 
              ((fp_db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ? TRUE : FALSE ),
              fp_db_info.action_types, 
              fp_db_info.action_widths,
              action_lsbs,
              action_lengths,
              &fem_act_size,
              &nof_actions,
              &success
            );
    SOC_SAND_CHECK_FUNC_RESULT(res, 300, exit); 
    if (success != SOC_SAND_SUCCESS) {
        LOG_ERROR(BSL_LS_SOC_FP,
                (BSL_META_U(unit,
                            "Unit %d, Invalid action composition.\n\r"),
                 unit));
        SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 195, exit);
    }
    {
      /*
       * For each action on the data base ('fp_db_info.action_types'), find the corresponding action
       * on FES table and match: 'action_lsbs' and 'action_lengths'.
       *
       */
      uint32
        found,
        action_idx_2,
        loc_action_lsbs[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1],
        loc_action_lengths[SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX+1] ;
      sal_memset(loc_action_lsbs, 0x0, sizeof(loc_action_lsbs)) ;
      sal_memset(loc_action_lengths, 0x0, sizeof(loc_action_lengths)) ;
      for (action_idx = 0; action_idx < info->db[db_idx].db_tcam.nof_actions ; action_idx++)
      {
        found = FALSE ;
        for (action_idx_2 = 0 ; action_idx_2 < nof_actions ; action_idx_2++)
        {
            /* Double actions dump */
          if ((fp_db_info.action_types[action_idx_2] == SOC_PPC_FP_ACTION_TYPE_COUNTER_AND_METER &&
                  (action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_COUNTER      ||
                   action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_METER        ||
                   action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT)) ||
              (fp_db_info.action_types[action_idx_2] == SOC_PPC_FP_ACTION_TYPE_SNOOP_AND_TRAP &&
                  (action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_SNP          ||
                   action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_TRAP_REDUCED ||
                   action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT)))
          {
            found = TRUE;
            loc_action_lsbs[action_idx] = action_lsbs[action_idx_2];
            loc_action_lengths[action_idx] = action_lengths[action_idx_2] - 1;
            if (action_types[action_idx] == SOC_PPC_FP_ACTION_TYPE_INVALID_NEXT) {
                /* For invalid next, length is zero */
                loc_action_lengths[action_idx] = 1;
            } else {
                /* For non-invalid next commands, starting lsb is double_action_lsb + 1 */
                loc_action_lsbs[action_idx] += 1;
            }
            break;
          }
          if (action_types[action_idx] == fp_db_info.action_types[action_idx_2])
          {
            found = TRUE ;
            loc_action_lsbs[action_idx] = action_lsbs[action_idx_2] ;
            loc_action_lengths[action_idx] = action_lengths[action_idx_2] ;
            break ;
          }
        }
        if (!found)
        {
          LOG_ERROR(
            BSL_LS_SOC_FP,
            (BSL_META_U(unit,"Unit %d, Could not find action %d (%s).\n\r"),
            unit,action_types[action_idx],SOC_PPC_FP_ACTION_TYPE_to_string(action_types[action_idx]))) ;
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 197, exit) ;
        }
      }
      sal_memcpy(action_lsbs, loc_action_lsbs, sizeof(action_lsbs)) ;
      sal_memcpy(action_lengths, loc_action_lengths, sizeof(action_lengths)) ;
    }
    if( (SOC_PPC_FP_DB_TYPE_FLP != info->db[db_idx].type) && !(fp_db_info.flags & SOC_PPC_FP_DATABASE_INFO_FLAGS_USE_KAPS) ) /* internal TCAM specific verification */
    {
        tcam_db_idx = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_idx);

        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(unit, tcam_db_idx, &action_bmp);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 305, exit);

        /* Check that the action size does not exceed the expected one */
        if(action_bmp < fem_act_size)
        {
          SOC_SAND_SET_ERROR_CODE(ARAD_PMF_LOW_LEVEL_ID_OUT_OF_RANGE_ERR, 310, exit);
        }
    }

    for(action_idx = 0; action_idx < info->db[db_idx].db_tcam.nof_actions; action_idx++)
    {
      /* The LSB of the action in the TCAM action table starts at 0 and
       * is increased by the length of each action. 
       * In the Ingress the first LSB bit indicates if the action is valid (if there 
       * is an action value following that bit), and so the action LSB is incremented 
       * by 1.
       * In the Egress there is no valid bit and the action LSB remains the same. 
       */
      info->db[db_idx].db_tcam.action[action_idx].action_loc.lsb = action_lsbs[action_idx];
      info->db[db_idx].db_tcam.action[action_idx].action_loc.lsb +=
        (info->db[db_idx].stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) ? 1 : 0;
      info->db[db_idx].db_tcam.action[action_idx].action_loc.msb  = 
        (action_lsbs[action_idx] + action_lengths[action_idx] - 1);
    }
  } /* for(db_idx) */

  /* Retrieve the rest of the information on all TCAM banks */
  for(bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_id++)
  {
    res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, bank_id, &(info->bank[bank_id].is_used));
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 313, exit);
    if(info->bank[bank_id].is_used)
    {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(unit, bank_id, &(info->bank[bank_id].entries_free));
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 315, exit);
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, bank_id, &(info->bank[bank_id].entry_size));
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 318, exit);

      res = arad_tcam_bank_owner_get_unsafe(
              unit, 
              bank_id, 
              &(info->bank[bank_id].owner)
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 320, exit);
    }
  }

  ARAD_ALLOC(pfg_info, SOC_PPC_PMF_PFG_INFO, 1, "arad_pp_fp_resource_diag_get_unsafe.pfg_info");
  /* Retrieve information on all Pre-Selectors */
  for(stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage++)
  {
    /* Preslectors resources */
    if(mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES) {
      info->available.is_used = TRUE;
      for(pfg_idx = 0; pfg_idx < SOC_PPC_FP_NOF_PFGS_ARAD; ++pfg_idx) {
        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.psl_info.pfgs_db_pmb.bit_range_read(
                unit,
                stage,
                pfg_idx,
                0,
                0,
                ARAD_PMF_NOF_DBS,
                info->available.pfgs_db_pmb[stage][pfg_idx]
               );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 325, exit);

        pfg_info->stage = stage;
        res = arad_pp_fp_packet_format_group_get_unsafe(
                unit,
                pfg_idx,
                pfg_info
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 327, exit);
        
        if(pfg_info->is_array_qualifier) {
          for(qual_idx = 0; qual_idx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++qual_idx) {
            if(!soc_sand_u64_is_zero(&pfg_info->qual_vals[qual_idx].is_valid)) {
              sal_memcpy(&info->available.pfgs_qualifiers[stage][pfg_idx][qual_idx], &pfg_info->qual_vals[qual_idx], sizeof(SOC_PPC_FP_QUAL_VAL));

              /* Special case for FLP: retrieve here the intersting FLP programs */
              if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) 
                  && (pfg_info->qual_vals[qual_idx].type == SOC_PPC_FP_QUAL_FWD_PRCESSING_PROFILE)) {
                      /* Get the list of FLP programs */
                      SOC_PPC_FP_CONTROL_INDEX_clear(&control_ndx);
                      control_ndx.type = SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE;
                      control_ndx.val_ndx = pfg_info->qual_vals[qual_idx].val.arr[0];
                      res = arad_pp_fp_control_get_unsafe(unit, SOC_CORE_INVALID, &control_ndx, &control_info);
                      SOC_SAND_CHECK_FUNC_RESULT(res, 14, exit);
                      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                                unit,
                                stage,
                                FALSE, 
                                &pgm_idx_min,
                                &pgm_idx_max
                              );
                      SOC_SAND_CHECK_FUNC_RESULT(res, 323, exit);
                      for(pgm_idx = pgm_idx_min; pgm_idx < pgm_idx_max; pgm_idx++)
                      {
                          if (SHR_BITGET(control_info.val, pgm_idx)) {
                              info->available.free_instructions[stage][pgm_idx][0].is_used = TRUE;
                          }
                      }
              }
            }
          }
        }
      }
    }
  }
  for(stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage++)
  {
      if ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) 
          || (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_VT) 
          || (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_TT)
          || ((stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_SLB) && SOC_IS_ARAD_B1_AND_BELOW(unit))) {
          /* Not relevant */
          continue;
      }
      res = arad_pmf_prog_select_pmf_pgm_borders_get(
                unit,
                stage,
                FALSE, 
                &pgm_idx_min,
                &pgm_idx_max
              );
      SOC_SAND_CHECK_FUNC_RESULT(res, 323, exit);

    for(line_idx = 0 ; line_idx < ARAD_PMF_NOF_LINES; line_idx++) 
    {
      sal_memset(&psl_tbl_data, 0x0, sizeof(ARAD_PMF_PSL));

      /* Read pre-selector table lines from the HW */
      res = arad_pmf_sel_table_get(
              unit, 
              line_idx, 
              stage, 
              &valid, 
              &(info->presel[stage][line_idx].pmf_pgm), 
              &psl_tbl_data
            ); 
      SOC_SAND_CHECK_FUNC_RESULT(res, 330, exit);

      if(valid)
      {
        info->presel[stage][line_idx].is_valid = 1;

        res = sw_state_access[unit].dpp.soc.arad.tm.pmf.pgm_db_pmb.bit_range_read(
                unit,
                stage,
                info->presel[stage][line_idx].pmf_pgm,
                0,
                0,
                ARAD_PMF_NOF_DBS,
                info->presel[stage][line_idx].db_bmp
              );
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 340, exit); 

        if(mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES) {
          /* first programs are static, and hence they can't be used anaway */ 
          if(info->presel[stage][line_idx].pmf_pgm >= pgm_idx_min) {
            for(cycle_idx = 0; cycle_idx < ARAD_PMF_LOW_LEVEL_NOF_CYCLES; cycle_idx++) {
              if(stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF) {
                info->available.fes_free[info->presel[stage][line_idx].pmf_pgm][cycle_idx].is_used = TRUE;
              }
              info->available.free_instructions[stage][info->presel[stage][line_idx].pmf_pgm][cycle_idx].is_used = TRUE;
            }
          }
          
          for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; ++db_idx) {
            if(SHR_BITGET(info->presel[stage][line_idx].db_bmp, db_idx)) {
              for(pfg_idx = 0; pfg_idx < SOC_PPC_FP_NOF_PFGS_ARAD; ++pfg_idx) {
                if(SHR_BITGET(info->available.pfgs_db_pmb[stage][pfg_idx], db_idx)) {
                  SHR_BITSET(info->available.pfgs[stage][line_idx][info->presel[stage][line_idx].pmf_pgm], pfg_idx);
                  for(qual_idx = 0; qual_idx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++qual_idx) {
                    if((info->available.pfgs_qualifiers[stage][pfg_idx][qual_idx].type != SOC_PPC_NOF_FP_QUAL_TYPES) &&
                       (info->available.pfgs_qualifiers[stage][pfg_idx][qual_idx].type != BCM_FIELD_ENTRY_INVALID)){
                      SHR_BITSET(info->available.quals[stage][line_idx][info->presel[stage][line_idx].pmf_pgm],
                        info->available.pfgs_qualifiers[stage][pfg_idx][qual_idx].type);
                    }
                  }
                }
              }
            }
          }          
        }
      }

      
      info->presel[stage][line_idx].presel_bmp[0] = 0;
    }
  }

  /* If diagnostics mode requires validation as well */
  if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG)
  {
    /* Initialize debug arrays */
    for(bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_id++)
    {
      bank_entry_size[bank_id] = ARAD_TCAM_NOF_BANK_ENTRY_SIZES;
      bank_profile_id[bank_id] = ARAD_TCAM_NOF_ACCESS_PROFILE_IDS;
    }
    for(profile_idx = 0; profile_idx < ARAD_TCAM_NOF_ACCESS_PROFILE_IDS; profile_idx++)
    {
      db_profile_id[profile_idx] = ARAD_TCAM_MAX_NOF_LISTS;
    }
    
    /* Perform Validation per tcam-db - per profile */
    for(db_idx = 0; db_idx < SOC_PPC_FP_NOF_DBS; db_idx++)
    {
      tcam_db_idx = ARAD_PP_FP_DB_ID_TO_TCAM_DB(db_idx);
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.valid.get(unit, tcam_db_idx, &is_valid);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 345, exit);
      if(!is_valid)
      {
        continue;
      }

      /* If key size is 320 check for both profile IDs */
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.entry_size.get(unit, tcam_db_idx, &entry_size);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 345, exit);
      nof_keys = (entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? 2 : 1;
      for(key_idx = 0; key_idx < nof_keys; key_idx++)
      {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_idx, key_idx, &profile_id);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 348, exit);

        sal_memset(&profile_tbl_data, 0x0, sizeof(ARAD_PP_IHB_TCAM_ACCESS_PROFILE_TBL_DATA));
        res = arad_pp_ihb_tcam_access_profile_tbl_read_unsafe(
                unit, 
                profile_id,
                &profile_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 350, exit); 
        
        /* Validate each profile is unique to a tcam DB */
        if(db_profile_id[profile_id] == ARAD_TCAM_MAX_NOF_LISTS)
        {
          db_profile_id[profile_id] = tcam_db_idx;
        }
        else if(db_profile_id[profile_id] != tcam_db_idx)
        {
          error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
          param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_UNIQUE_PROFILE;
          if(!error_params[param_idx].is_error)
          {
            value_idx = 0;
            error_params[param_idx].is_error = TRUE;
            error_params[param_idx].value[value_idx++] = profile_id;
            error_params[param_idx].value[value_idx++] = db_profile_id[profile_id];
            error_params[param_idx].value[value_idx++] = ARAD_PP_FP_TCAM_DB_TO_FP_ID(db_profile_id[profile_id]);
            error_params[param_idx].value[value_idx++] = tcam_db_idx;
            error_params[param_idx].value[value_idx++] = db_idx;
          }
        }

        /* Verify that the prefix in SW anh HW match */
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.prefix.get(unit, tcam_db_idx, &prefix);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 352, exit);
        if((entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) && (key_idx == 0))
        {
            prefix_hw.length = profile_tbl_data.prefix_and;
            prefix_hw.bits = profile_tbl_data.prefix_or;
        }
        else
        {
            prefix_hw.length = ARAD_SW_PREFIX_AND_TO_PREFIX_LENGTH(profile_tbl_data.prefix_and);
            prefix_hw.bits = profile_tbl_data.prefix_or >> (ARAD_TCAM_PREFIX_SIZE_MAX - prefix_hw.length);
        }
        
        not_valid = FALSE;
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.is_direct.get(unit, tcam_db_idx, &is_direct);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 355, exit);
        if (is_direct && (prefix_hw.bits != 0 || prefix_hw.length != 0)) 
        {
            not_valid = TRUE;
        }
        else if(entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS && key_idx == 0
                && (prefix_hw.bits != 0 || prefix_hw.length != 0xf)) {
            not_valid = TRUE;
        }
        else if(prefix_hw.bits != prefix.bits || prefix_hw.length != prefix.length) 
        {
            not_valid = TRUE;
        }
        
        if(not_valid)
        {
          /* Report Error in Diag */
          error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
          param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_PREFIX;
          if(!error_params[param_idx].is_error)
          {
            value_idx = 0;
            error_params[param_idx].is_error = TRUE;
            error_params[param_idx].value[value_idx++] = tcam_db_idx;
            error_params[param_idx].value[value_idx++] = db_idx;
            error_params[param_idx].value[value_idx++] = profile_id;
            error_params[param_idx].value[value_idx++] = prefix.bits;
            error_params[param_idx].value[value_idx++] = prefix.length;
            error_params[param_idx].value[value_idx++] = prefix_hw.bits;
            error_params[param_idx].value[value_idx++] = prefix_hw.length;
          }
        }

        /* Compare Key Size, relevant only if DB type is TCAM, must be identical*/
        if((info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_TCAM || 
            info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_EGRESS)
           && ARAD_SW_HW_ENTRY_SIZE_ID_TO_SW_ENTRY_SIZE(profile_tbl_data.key_size) != entry_size)
        {
          error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
          param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_ENTRY_SIZE;
          if(!error_params[param_idx].is_error)
          {
            value_idx = 0;
            error_params[param_idx].is_error = TRUE;
            error_params[param_idx].value[value_idx++] = tcam_db_idx;
            error_params[param_idx].value[value_idx++] = db_idx;
            error_params[param_idx].value[value_idx++] = profile_id;
            error_params[param_idx].value[value_idx++] = entry_size;
            error_params[param_idx].value[value_idx++] = ARAD_SW_HW_ENTRY_SIZE_ID_TO_SW_ENTRY_SIZE(profile_tbl_data.key_size);
          }
        }

        /* Read per profile which banks are being used by it */
        sal_memset(&pd_profile_tbl_data, 0x0, sizeof(ARAD_PP_IHB_TCAM_PD_PROFILE_TBL_DATA));
        res = arad_pp_ihb_tcam_pd_profile_tbl_read_unsafe(
                unit, 
                profile_id, 
                &pd_profile_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 360, exit);

        /* Per bank, check if it is used by the selected profile */
        for(bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_id++)
        {
          if(key_idx == 0 && !(bank_id %2))
          {
            bank_used = ((1 << bank_id) & pd_profile_tbl_data.bitmap);
            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_idx, bank_id, &is_used);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 280, exit);
            /* if bank is indicated as used by HW, verify if conforms with the SW */
            if((bank_used && !is_used) ||
               (!bank_used && is_used))
            {
              error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
              param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_BANKS;
              if(!error_params[param_idx].is_error)
              {
                value_idx = 0;
                error_params[param_idx].is_error = TRUE;
                error_params[param_idx].value[value_idx++] = tcam_db_idx;
                error_params[param_idx].value[value_idx++] = db_idx;
                error_params[param_idx].value[value_idx++] = bank_id;
                error_params[param_idx].value[value_idx++] = profile_id;
                error_params[param_idx].value[value_idx++] = is_used;
                error_params[param_idx].value[value_idx++] = bank_used;
              }
            }
          }

          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_idx, bank_id, &is_used);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 290, exit);
          if(is_used)
          {
            /* if the same bank is used for different entry sizes report Error */
            if(ARAD_TCAM_NOF_BANK_ENTRY_SIZES != bank_entry_size[bank_id])
            {
                if (bank_entry_size[bank_id] != entry_size) 
                {
                    error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_BANK].params;
                    param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_ENTRY_SIZE;
                    if(!error_params[param_idx].is_error)
                    {
                        value_idx = 0;
                        error_params[param_idx].is_error = TRUE;
                        error_params[param_idx].value[value_idx++] = bank_id;
                        error_params[param_idx].value[value_idx++] = bank_profile_id[bank_id];
                        error_params[param_idx].value[value_idx++] = bank_entry_size[bank_id];
                        error_params[param_idx].value[value_idx++] = profile_id;
                        error_params[param_idx].value[value_idx++] = entry_size;
                    }
                }
            }
            else
            {
                bank_entry_size[bank_id] = entry_size;
                bank_profile_id[bank_id] = profile_id;
            }
            
            /* Check for Bank Repartition (repeating prefix per bank) */
            prefix_id = prefix.bits;
            if(info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_TCAM 
               || info->db[db_idx].type == SOC_PPC_FP_DB_TYPE_EGRESS)
            {
              if(bank_db_prefix_used[bank_id][prefix_id])
              {
                if(tcam_db_idx != bank_db_prefix[bank_id * ARAD_TCAM_NOF_PREFIXES + prefix_id])
                {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_BANK].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_PREFIX_DB;
                  if(!error_params[param_idx].is_error)
                  {
                    value_idx = 0;
                    error_params[param_idx].is_error = TRUE;
                    error_params[param_idx].value[value_idx++] = bank_id;
                    error_params[param_idx].value[value_idx++] = bank_db_prefix[bank_id * ARAD_TCAM_NOF_PREFIXES + prefix_id];
                    error_params[param_idx].value[value_idx++] = ARAD_PP_FP_TCAM_DB_TO_FP_ID(bank_db_prefix[bank_id * ARAD_TCAM_NOF_PREFIXES + prefix_id]);
                    error_params[param_idx].value[value_idx++] = prefix_id;
                    error_params[param_idx].value[value_idx++] = tcam_db_idx;
                    error_params[param_idx].value[value_idx++] = db_idx;
                    error_params[param_idx].value[value_idx++] = 
                      (profile_tbl_data.prefix_or >> (ARAD_TCAM_PREFIX_SIZE_MAX - ARAD_SW_PREFIX_AND_TO_PREFIX_LENGTH(profile_tbl_data.prefix_and)));
                  }
                }
              }
              else
              {
                bank_db_prefix_used[bank_id][prefix_id] = TRUE;
                bank_db_prefix[bank_id * ARAD_TCAM_NOF_PREFIXES + prefix_id] = tcam_db_idx;
              }
            }

            /* for first key idx - if bank id is even, take first and second bits from
             * action bmp, else (bank_id id odd), take the third and forth bits.
             * compare the two bits with the action bitmap in the HW ( profile_tbl_data.action_bitmap ) 
             * if SW entry size is 80b and the action bitmap is 0x1 or 0x2, action bitmap in HW is 
             * expected to be 0x3, since two actions are expected to be in each line, as it is 
             * with entries in the TCAM (two 80b entries in line). 
             */
            hw_action_bmp = ((profile_tbl_data.action_bitmap >> (bank_id*2)) & 0x3);
            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.action_bitmap_ndx.get(unit, tcam_db_idx, &action_bmp);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 300, exit);

            if(entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS) 
            {
                if(action_bmp == 0x1 || action_bmp == 0x2) 
                {
                    action_bmp = 0x3;
                }
            }
             
            if( ( (key_idx == 0) && (nof_keys == 1 || !(bank_id % 2)) && ((action_bmp & 0x3) != hw_action_bmp) ) || 
                ( (key_idx == 1) &&  (bank_id % 2) && ((action_bmp >> 2) != hw_action_bmp) ) )
            {
              error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
              param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_ACTION_BITMAP;
              if(!error_params[param_idx].is_error)
              {
                value_idx = 0;
                error_params[param_idx].is_error = TRUE;
                error_params[param_idx].value[value_idx++] = tcam_db_idx;
                error_params[param_idx].value[value_idx++] = db_idx;
                error_params[param_idx].value[value_idx++] = profile_id;
                error_params[param_idx].value[value_idx++] = bank_id;
                error_params[param_idx].value[value_idx++] = !(bank_id % 2) ? (action_bmp & 0x3) : ((action_bmp >> 2) & 0x3);
                error_params[param_idx].value[value_idx++] = hw_action_bmp;
              }
            }
          } /* if bank used */

        } /* for bank_id */ 
        
      } /* for key_idx */
    } /* for db_idx */

    /* Collect Validation Diagnostics per Bank */
    sal_memset(bank_nof_entries_hw, 0x0, sizeof(uint32) * SOC_DPP_DEFS_MAX_TCAM_NOF_BANKS);
    ARAD_ALLOC(diag_priority, ARAD_PP_FP_RESOURCE_DIAG_DB_PRIORITY, ARAD_TCAM_MAX_NOF_LISTS, "arad_pp_fp_resource_diag_get_unsafe.diag_priority");
    sal_memset(diag_priority, 0x0, sizeof(ARAD_PP_FP_RESOURCE_DIAG_DB_PRIORITY) * ARAD_TCAM_MAX_NOF_LISTS);
    for(bank_id = 0; bank_id < SOC_DPP_DEFS_GET_TCAM_NOF_BANKS(unit); bank_id++)
    {
      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, bank_id, &is_valid);
      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 310, exit);
      if(!is_valid)
      {
        continue;
      }

      res = arad_tcam_bank_owner_get_unsafe(
              unit, 
              bank_id, 
              &(info->bank[bank_id].owner)
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 320, exit);

      /* If bank owner is not PMF - no need for entries information*/
      if (( (info->bank[bank_id].owner) != ARAD_TCAM_BANK_OWNER_PMF_0 )
          && ( (info->bank[bank_id].owner) != ARAD_TCAM_BANK_OWNER_PMF_1 ))
      {
        continue;
      }

      found = FALSE;

      /* Search for a profile ID which points to this bank */
      for(tcam_db_idx = 0; tcam_db_idx < ARAD_TCAM_MAX_NOF_LISTS; tcam_db_idx++)
      {
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.bank_used.get(unit, tcam_db_idx, bank_id, &is_used);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 350, exit);

        if(!is_used)
        {
          continue;
        }

        key_idx = (info->bank[bank_id].entry_size == ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS) ? (bank_id % 2) : 0;

        sal_memset(&profile_tbl_data, 0x0, sizeof(ARAD_PP_IHB_TCAM_ACCESS_PROFILE_TBL_DATA));
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_idx, key_idx, &profile_idx);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 360, exit);

        res = arad_pp_ihb_tcam_access_profile_tbl_read_unsafe(
                unit, 
                profile_idx, 
                &profile_tbl_data
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 370, exit);

        /* if profile points to this tcam bank, exit loop */
        bank_used = (1 << (bank_id*2)) & profile_tbl_data.action_bitmap;
        bank_used_1 = (1 << (bank_id*2 + 1)) & profile_tbl_data.action_bitmap;
        if(bank_used || bank_used_1)
        {
          found = TRUE;
          break;
        }
      }

      /* if no profile pointed to this bank - error */
      if((key_idx == 0) && !found) 
      {
        error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_BANK].params;
        param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_VALID;
        if(!error_params[param_idx].is_error)
        {
          value_idx = 0;
          error_params[param_idx].is_error = TRUE;
          error_params[param_idx].value[value_idx++] = bank_id;
          res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.valid.get(unit, bank_id, &is_valid);
          SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 375, exit);
          error_params[param_idx].value[value_idx++] = is_valid;
        }
      }

      if(SOC_PPC_FP_RESOURCE_MODE_ALL == mode)
      {
        /* Verify TCAM entries */
        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, bank_id, &entry_size);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 377, exit);
        /* if entry size is 320, do validation only for even bank IDs */
        if( (ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS != entry_size) ||
           ((ARAD_TCAM_BANK_ENTRY_SIZE_320_BITS == entry_size) && !(bank_id % 2)))
        {
          bank_nof_lines = arad_tcam_bank_entry_size_to_entry_count_get(unit, entry_size, bank_id);
          for(entry_idx = 0; entry_idx < bank_nof_lines; entry_idx++)
          {
            location.bank_id = bank_id;
            location.entry = entry_idx;
            
            res = arad_tcam_resource_db_entries_validate(
                    unit,
                    bank_id,
                    entry_size,
                    entry_idx,
                    &tcam_entry
                  );
            SOC_SAND_CHECK_FUNC_RESULT(res, 380, exit);

            /* IMPORTANT NOTICE : currently there is no validation that this specific entry
             * exists in the SW lists, only comparison of number of entries used by SW. 
             * The reason is: 
             * 1- currently there is no ability to avoid the complexity of O(n^2) reads 
             * (for each entry we read from HW, we would need to look for the entry in the 
             * SW (according to DB), and vise versa) 
             * 2- the other option would cost the complexity of keeping a bitmap for all 
             * possible entries, which would be time-consuming programming-wise 
             */
            if(tcam_entry.valid)
            {
              /* count valid entries per bank */
              bank_nof_entries_hw[bank_id]++;

              /* get tcam db id from prefix in bits 79:76 or 159:156 in value */
              prefix_id = (ARAD_TCAM_BANK_ENTRY_SIZE_80_BITS == entry_size) ? 
                          ((tcam_entry.value[2] >> 12) & 0xf) : 
                          ((tcam_entry.value[4] >> 28) & 0xf); 

              res = sw_state_access[unit].dpp.soc.arad.tm.tcam_mgmt.banks.prefix_db.get(unit, bank_id, prefix_id, &tcam_db_idx);
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 383, exit);

              /* Entires read from HW must be TCAM DBs */
              if (info->db[ARAD_PP_FP_TCAM_DB_TO_FP_ID(tcam_db_idx)].type != SOC_PPC_FP_DB_TYPE_TCAM){
                  SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_DB_TYPE_OUT_OF_RANGE_ERR, 385, exit);
              }

              /* Validate this entry is in Occupation Bitmap */
              found = FALSE;
              res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_bank_entries_used.get(
                         unit,
                         bank_id,
                         0,
                         &occ_bm
                       );
              SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);
              res = soc_sand_occ_bm_is_occupied(
                      unit,
                      occ_bm,
                      entry_idx,
                      &found
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 390, exit);

              if(!found)
              {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_ENTRY_VALID;
                  if(!error_params[param_idx].is_error)
                  {
                      value_idx = 0;
                      error_params[param_idx].is_error = TRUE;
                      error_params[param_idx].value[value_idx++] = tcam_db_idx;
                      error_params[param_idx].value[value_idx++] = ARAD_PP_FP_TCAM_DB_TO_FP_ID(tcam_db_idx);
                      res = sw_state_access[unit].dpp.soc.arad.tm.tcam.tcam_db.access_profile_id.get(unit, tcam_db_idx, bank_id%2, &(error_params[param_idx].value[value_idx++]));
                      SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 393, exit);
                      error_params[param_idx].value[value_idx++] = bank_id;
                      error_params[param_idx].value[value_idx++] = entry_idx;
                      error_params[param_idx].value[value_idx++] = tcam_entry.valid;
                      error_params[param_idx].value[value_idx++] = found;
                  }
              }

              res = arad_tcam_resource_db_entries_priority_validate(
                      unit,
                      tcam_db_idx,
                      &location,
                      &entry_id,
                      &priority
                    );
              SOC_SAND_CHECK_FUNC_RESULT(res, 395, exit);

              /* per DB for each entry validate its priority is not better than the one before it */
              if(diag_priority[tcam_db_idx].exists && 
                 diag_priority[tcam_db_idx].priority > priority)
              {
                error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_DB].params;
                param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_TCAM_DB_PRIORITY;
                if(!error_params[param_idx].is_error)
                {
                  value_idx = 0;
                  error_params[param_idx].is_error = TRUE;
                  error_params[param_idx].value[value_idx++] = tcam_db_idx;
                  error_params[param_idx].value[value_idx++] = ARAD_PP_FP_TCAM_DB_TO_FP_ID(tcam_db_idx);
                  error_params[param_idx].value[value_idx++] = diag_priority[tcam_db_idx].entry_id;
                  error_params[param_idx].value[value_idx++] = entry_id;
                  error_params[param_idx].value[value_idx++] = diag_priority[tcam_db_idx].bank_id;
                  error_params[param_idx].value[value_idx++] = bank_id;
                  error_params[param_idx].value[value_idx++] = diag_priority[tcam_db_idx].line_idx;
                  error_params[param_idx].value[value_idx++] = entry_idx;
                  error_params[param_idx].value[value_idx++] = diag_priority[tcam_db_idx].priority;
                  error_params[param_idx].value[value_idx++] = priority;
                }
              }
              else
              {
                diag_priority[tcam_db_idx].exists = TRUE;
                diag_priority[tcam_db_idx].bank_id = bank_id;
                diag_priority[tcam_db_idx].entry_id = entry_id;
                diag_priority[tcam_db_idx].priority = priority;
                diag_priority[tcam_db_idx].line_idx = entry_idx;
              }
            }
          } /* for(entry_idx) */

          /* compare count of valid entries per bank to SW DB */
          if((bank_nof_lines != 0))
          {
            res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.nof_entries_free.get(unit, bank_id, &nof_entries);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 61, exit);
            if(bank_nof_entries_hw[bank_id] != (bank_nof_lines - nof_entries)) {
                error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_TCAM_BANK].params;
                param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_BANK_NOF_ENTRIES;
                if(!error_params[param_idx].is_error)
                {
                  value_idx = 0;
                  error_params[param_idx].is_error = TRUE;
                  error_params[param_idx].value[value_idx++] = bank_id;
                  error_params[param_idx].value[value_idx++] = bank_nof_lines - nof_entries;
                  error_params[param_idx].value[value_idx++] = bank_nof_entries_hw[bank_id];
                }
            }
          }
        } /* filter banks according to entry size */
      }
    } /* for bank_id */
  }/* if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG) */
  
  if(mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES) {
    /* Validate Resource Bitmaps */
    for(stage = 0; stage < SOC_PPC_NOF_FP_DATABASE_STAGES; stage++)
    {
      for (is_tm = FALSE; is_tm <= (stage == SOC_PPC_FP_DATABASE_STAGE_INGRESS_PMF); is_tm++) {
        res = arad_pmf_prog_select_pmf_pgm_borders_get(
                  unit,
                  stage,
                  is_tm, 
                  &pgm_idx_min,
                  &pgm_idx_max
                );
        SOC_SAND_CHECK_FUNC_RESULT(res, 62, exit);
        for(pgm_idx = pgm_idx_min; pgm_idx < pgm_idx_max; pgm_idx++)
        {
          for(cycle_idx = 0; cycle_idx < ARAD_PMF_LOW_LEVEL_NOF_CYCLES; cycle_idx++)
          {
            /* Per stage - Per program - Per Cycle - Get CE resource bitmap from SW DB and compare to HW */
            res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.ce.get(unit, stage, pgm_idx, cycle_idx, &ce_rsrc);
            SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 400, exit);
             if(info->available.free_instructions[stage][pgm_idx][cycle_idx].is_used) {
              info->available.free_instructions[stage][pgm_idx][cycle_idx].lsb_16b = 
                arad_pp_fp_key_nof_free_instr_get(unit,stage, ce_rsrc, FALSE, FALSE, FALSE) + arad_pp_fp_key_nof_free_instr_get(unit, stage, ce_rsrc, FALSE, FALSE, TRUE);
              info->available.free_instructions[stage][pgm_idx][cycle_idx].lsb_32b = 
                arad_pp_fp_key_nof_free_instr_get(unit,stage, ce_rsrc, TRUE, FALSE, FALSE) + arad_pp_fp_key_nof_free_instr_get(unit, stage, ce_rsrc, TRUE, FALSE, TRUE);
              info->available.free_instructions[stage][pgm_idx][cycle_idx].msb_16b = 
                arad_pp_fp_key_nof_free_instr_get(unit,stage, ce_rsrc, FALSE, TRUE, FALSE) + arad_pp_fp_key_nof_free_instr_get(unit, stage, ce_rsrc, FALSE, TRUE, TRUE);
              info->available.free_instructions[stage][pgm_idx][cycle_idx].msb_32b = 
                arad_pp_fp_key_nof_free_instr_get(unit,stage, ce_rsrc, TRUE, TRUE, FALSE) + arad_pp_fp_key_nof_free_instr_get(unit, stage, ce_rsrc, TRUE, TRUE, TRUE);
            }
            if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG) {
                if(ce_rsrc != diag_ce_bitmap[stage][pgm_idx][cycle_idx])
                {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_CE_BITMAP;
                  if(!error_params[param_idx].is_error)
                  {
                    value_idx = 0;
                    error_params[param_idx].is_error = TRUE;
                    error_params[param_idx].value[value_idx++] = stage;
                    error_params[param_idx].value[value_idx++] = pgm_idx;
                    error_params[param_idx].value[value_idx++] = cycle_idx;
                    error_params[param_idx].value[value_idx++] = ce_rsrc;
                    error_params[param_idx].value[value_idx++] = diag_ce_bitmap[stage][pgm_idx][cycle_idx];
                  }
                }

                /* Per stage - Per program - Per Cycle - Get KEY resource bitmap from SW DB and compare to HW */
                res = sw_state_access[unit].dpp.soc.arad.tm.pmf.rsources.key.get(unit, stage, pgm_idx, cycle_idx, &key_rsrc);
                SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 410, exit);
                if(key_rsrc != diag_key_bitmap[stage][pgm_idx][cycle_idx])
                {
                  error_params = info->diag[SOC_PPC_FP_RESOURCE_DIAG_ERROR_TYPE_KEY].params;
                  param_idx = SOC_PPC_FP_RESOURCE_DIAG_PARAM_TYPE_KEY_KEY_BITMAP;
                  if(!error_params[param_idx].is_error)
                  {
                    value_idx = 0;
                    error_params[param_idx].is_error = TRUE;
                    error_params[param_idx].value[value_idx++] = stage;
                    error_params[param_idx].value[value_idx++] = pgm_idx;
                    error_params[param_idx].value[value_idx++] = cycle_idx;
                    error_params[param_idx].value[value_idx++] = key_rsrc;
                    error_params[param_idx].value[value_idx++] = diag_key_bitmap[stage][pgm_idx][cycle_idx];
                  }
                }
            }
          }

          if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG) {
          }
        }/* pgm_idx */

        if(mode >= SOC_PPC_FP_RESOURCE_MODE_DIAG) {
        }
      }/*is_tm */
    }/* stage */
  } /* if(mode >= SOC_PPC_FP_RESOURCE_MODE_WITH_AVAILABLE_RESOURCES) */

exit:
  ARAD_FREE(diag_db_key);
  ARAD_FREE(diag_db_key_args);
  ARAD_FREE(fes_prio);
  ARAD_FREE(action_prio);
  ARAD_FREE(diag_priority);
  ARAD_FREE(key_cur_loc);
  ARAD_FREE(pfg_info);
  ARAD_FREE(bank_db_prefix);
  ARAD_FREE(db_used_program);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_resource_diag_get_unsafe()", 0, 0); 
}

/*********************************************************************
*     Get the pointer to the list of procedures of the
 *     arad_pp_api_fp module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_PROCEDURE_DESC_ELEMENT*
  arad_pp_fp_get_procs_ptr(void)
{
  return Arad_pp_procedure_desc_element_fp;
}
/*********************************************************************
*     Get the pointer to the list of errors of the
 *     arad_pp_api_fp module.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
CONST SOC_ERROR_DESC_ELEMENT*
  arad_pp_fp_get_errs_ptr(void)
{
  return Arad_pp_error_desc_element_fp;
}

uint32
  SOC_PPC_FP_QUAL_VAL_verify(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_QUAL_VAL       *info,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE    stage
  )
{
    uint32
        res,
        qual_length,
        max_val;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->type, SOC_PPC_FP_QUAL_TYPES_MAX, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 10, exit);

  /* Verify the value and mask is 0 after the qualifier size */
  res = arad_pp_fp_key_length_get_unsafe(
          unit,
          stage,            
          info->type,
          FALSE, /* without padding */
          &qual_length
        );
  SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

  if (qual_length <= 32) {
      SOC_SAND_MAX_VAL_FOR_BIT_LEN(max_val, qual_length);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->val.arr[0], max_val, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 30, exit);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->is_valid.arr[0], max_val, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 32, exit);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->val.arr[1], 0, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 34, exit);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->is_valid.arr[1], 0, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 36, exit);
  }
  else {
      SOC_SAND_MAX_VAL_FOR_BIT_LEN(max_val, (qual_length - 32));     
      SOC_SAND_ERR_IF_ABOVE_MAX(info->val.arr[1], max_val, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 40, exit);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->is_valid.arr[1], max_val, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 42, exit);
  }


  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_QUAL_VAL_verify()",0,0);
}

uint32
  SOC_PPC_PMF_PFG_INFO_verify(
      SOC_SAND_IN  int               unit,
      SOC_SAND_IN  SOC_PPC_PMF_PFG_INFO *info
  )
{
    ARAD_PMF_PSL         
        psl;
    uint32
        res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  /* Support only array qualifiers */
  if (info->is_array_qualifier) {
      /* If 0, means to be removed */

      /* 
       * Verify the stage only when setting a PFG. 
       * No need for stage validation when deleting./ 
       */
      SOC_SAND_ERR_IF_ABOVE_MAX(info->stage, SOC_PPC_NOF_FP_DATABASE_STAGES - 1, ARAD_PP_FP_STAGE_OUT_OF_RANGE_ERR, 10, exit);

      /* 
       * Verify all the qualifiers are supported according to the stage 
       * by mapping the qualifier to the tables 
       */
      if (info->stage != SOC_PPC_FP_DATABASE_STAGE_INGRESS_FLP) { /* Do not check for FLP stage */
          res = arad_pmf_psl_map(
                    unit,
                    info,
                    0, /* is_pfg_tm */
                    &psl
                  );
          SOC_SAND_CHECK_FUNC_RESULT(res, 18, exit);
      }
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_PMF_PFG_INFO_verify()",0,0);
}

uint32
  SOC_PPC_FP_DATABASE_INFO_verify(
    SOC_SAND_IN  SOC_PPC_FP_DATABASE_INFO *info
  )
{
  uint32
    ind;
  uint8
    end_of_list;

  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->db_type, SOC_PPC_FP_DATABASE_TYPE_MAX, SOC_PPC_FP_DATABASE_TYPE_OUT_OF_RANGE_ERR, 10, exit);

  /* Parameter not supported in Arad */
  SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(info->supported_pfgs, 0, ARAD_PP_FP_SUPPORTED_PFGS_OUT_OF_RANGE_ERR, 11, exit);

  end_of_list = FALSE;
  for (ind = 0; ind < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++ind)
  {
      if ((info->qual_types[ind] == SOC_PPC_NOF_FP_QUAL_TYPES) || (info->qual_types[ind] == BCM_FIELD_ENTRY_INVALID))
      {
        end_of_list = TRUE;
      }

    if (end_of_list == FALSE)
    {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->qual_types[ind], SOC_PPC_NOF_FP_QUAL_TYPES, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 110, exit);
    }
    else /* end_of_list == TRUE */
    {
      SOC_SAND_ERR_IF_NOT_EQUALS_VALUE(info->qual_types[ind], BCM_FIELD_ENTRY_INVALID, SOC_PPC_FP_QUAL_TYPES_END_OF_LIST_ERR, 111, exit);
    }
  }


  end_of_list = FALSE;
  for (ind = 0; ind < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++ind)
  {
      if (info->action_types[ind] == SOC_PPC_FP_ACTION_TYPE_INVALID)
      {
        end_of_list = TRUE;
      }

    if (end_of_list == FALSE)
    {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->action_types[ind], SOC_PPC_NOF_FP_ACTION_TYPES_ARAD, SOC_PPC_FP_ACTION_TYPES_OUT_OF_RANGE_ERR, 112, exit);
    }
    else /* end_of_list == TRUE */
    {
      SOC_SAND_ERR_IF_OUT_OF_RANGE(info->action_types[ind], SOC_PPC_FP_ACTION_TYPE_INVALID, SOC_PPC_FP_ACTION_TYPE_INVALID, SOC_PPC_FP_ACTION_TYPES_END_OF_LIST_ERR, 121, exit);
    }
  }

  SOC_SAND_ERR_IF_ABOVE_MAX(info->strength, ARAD_PP_FP_STRENGTH_MAX, ARAD_PP_FP_STRENGTH_OUT_OF_RANGE_ERR, 130, exit);


  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_DATABASE_INFO_verify()",0,0);
}

uint32
  SOC_PPC_FP_ACTION_VAL_verify(
    SOC_SAND_IN  SOC_PPC_FP_ACTION_VAL *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->type, SOC_PPC_NOF_FP_ACTION_TYPES_ARAD, SOC_PPC_FP_ACTION_TYPES_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->val, ARAD_PP_FP_VAL_MAX, ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 11, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_ACTION_VAL_verify()",0,0);
}

uint32
  SOC_PPC_FP_ENTRY_INFO_verify(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  SOC_PPC_FP_ENTRY_INFO *info,
      SOC_SAND_IN     SOC_PPC_FP_DATABASE_STAGE    stage
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++ind)
  {
      if ((info->qual_vals[ind].type != SOC_PPC_NOF_FP_QUAL_TYPES) && (info->qual_vals[ind].type != BCM_FIELD_ENTRY_INVALID))
      {
          res = SOC_PPC_FP_QUAL_VAL_verify(unit, &(info->qual_vals[ind]), stage);
          SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
      }
  }
  for (ind = 0; ind < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++ind)
  {
	  if (info->actions[ind].type != BCM_FIELD_ENTRY_INVALID )
		  ARAD_PP_STRUCT_VERIFY(SOC_PPC_FP_ACTION_VAL, &(info->actions[ind]), 11, exit);
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->priority, ARAD_PP_FP_PRIORITY_MAX, ARAD_PP_FP_PRIORITY_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_ENTRY_INFO_verify()",0,0);
}

uint32
  SOC_PPC_FP_DIR_EXTR_ACTION_LOC_verify(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ACTION_LOC *info,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE    stage,
      SOC_SAND_IN  uint8                     is_large_direct_extraction
  )
{
    uint32
        qual_length,
        res;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  if ((info->type == SOC_PPC_NOF_FP_QUAL_TYPES) || (info->type == BCM_FIELD_ENTRY_INVALID))
  {
      /* Use NOF for constant values */
      if (!is_large_direct_extraction) {
          SOC_SAND_ERR_IF_ABOVE_MAX(info->cst_val, ARAD_PP_FP_CST_VAL_MAX, ARAD_PP_FP_CST_VAL_OUT_OF_RANGE_ERR, 10, exit);
      }
      else {
          /* Do not accept constant non-zero values, prefer Data qualifier ConstantZero/One based defined */
        if ((info->cst_val >> info->fld_lsb) & ((1 << info->nof_bits) - 1))
        {
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 12, exit);
        }
        ARAD_DO_NOTHING_AND_EXIT; /* skip it */
      }
  }
  else {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->type, SOC_PPC_FP_QUAL_TYPES_MAX, SOC_PPC_FP_QUAL_TYPES_OUT_OF_RANGE_ERR, 20, exit);
      SOC_SAND_ERR_IF_ABOVE_MAX(info->cst_val, 0, ARAD_PP_FP_CST_VAL_OUT_OF_RANGE_ERR, 30, exit);
  }
  /* Do not allow lost or LSB bits in large DE */
  if (info->type != SOC_PPC_FP_QUAL_KEY_AFTER_HASHING) {
    SOC_SAND_ERR_IF_ABOVE_MAX(info->fld_lsb, (is_large_direct_extraction ? 0 : ARAD_PP_FP_FLD_LSB_MAX), ARAD_PP_FP_FLD_LSB_OUT_OF_RANGE_ERR, 40, exit); 
  }

  if (!is_large_direct_extraction) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->nof_bits, SOC_DPP_DEFS_GET(unit, fem_max_action_size_nof_bits), ARAD_PP_FP_NOF_BITS_OUT_OF_RANGE_ERR, 50, exit);
  }
  else {
      /* The qualifier size must be equal to what is extracted: extract only the whole qualifier */
      res = arad_pp_fp_key_length_get_unsafe(
              unit,
              stage,            
              info->type,
              FALSE, /* logical size */
              &qual_length
            );
      SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

      if (info->type != SOC_PPC_FP_QUAL_KEY_AFTER_HASHING && info->nof_bits != qual_length) {
          SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 70, exit);
      }
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_DIR_EXTR_ACTION_LOC_verify()",0,0);
}

uint32
  SOC_PPC_FP_DIR_EXTR_ACTION_VAL_verify(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ACTION_VAL *info,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE    stage,
      SOC_SAND_IN  uint8                           is_large_direct_extraction
  )
{
  uint32
    res = SOC_SAND_OK;

  uint32
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->type, SOC_PPC_FP_ACTION_TYPES_MAX, SOC_PPC_FP_ACTION_TYPES_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->nof_fields, ARAD_PP_FP_NOF_FIELDS_MAX, ARAD_PP_FP_NOF_FIELDS_OUT_OF_RANGE_ERR, 12, exit);
  for (ind = 0; ind < info->nof_fields; ++ind)
  {
      res = SOC_PPC_FP_DIR_EXTR_ACTION_LOC_verify(unit, &(info->fld_ext[ind]), stage, is_large_direct_extraction);
      SOC_SAND_CHECK_FUNC_RESULT(res, 22, exit);
  }
  /* No base val in large DE DB since it is FES-based */
  SOC_SAND_ERR_IF_ABOVE_MAX(info->base_val, (is_large_direct_extraction? 0: ARAD_PP_FP_BASE_VAL_MAX(unit)), ARAD_PP_FP_BASE_VAL_OUT_OF_RANGE_ERR, 13, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_DIR_EXTR_ACTION_VAL_verify()",0,0);
}

uint32
  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_verify(
      SOC_SAND_IN  int                    unit,
      SOC_SAND_IN  SOC_PPC_FP_DIR_EXTR_ENTRY_INFO *info,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE    stage,
      SOC_SAND_IN  uint8                           is_large_direct_extraction
  )
{
  uint32
    res = SOC_SAND_OK,
    ind;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  for (ind = 0; ind < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++ind)
  {
    if ((info->qual_vals[ind].type != SOC_PPC_NOF_FP_QUAL_TYPES) && (info->qual_vals[ind].type != BCM_FIELD_ENTRY_INVALID))
    {
        res = SOC_PPC_FP_QUAL_VAL_verify(unit, &(info->qual_vals[ind]), stage);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
  }

  for (ind = 0; ind < SOC_PPC_FP_NOF_ACTIONS_PER_DB_MAX; ++ind)
  {
    if (info->actions[ind].type != SOC_PPC_FP_ACTION_TYPE_INVALID)
    {
        if (ind && is_large_direct_extraction) {
            /* For large DE, only one action is allowed */
            SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 11, exit);
        }

        res = SOC_PPC_FP_DIR_EXTR_ACTION_VAL_verify(unit, &(info->actions[ind]), stage, is_large_direct_extraction);
        SOC_SAND_CHECK_FUNC_RESULT(res, 11, exit);
    }
  }
  SOC_SAND_ERR_IF_ABOVE_MAX(info->priority, ARAD_PP_FP_PRIORITY_MAX, ARAD_PP_FP_PRIORITY_OUT_OF_RANGE_ERR, 12, exit);

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_DIR_EXTR_ENTRY_INFO_verify()",0,0);
}

uint32
  SOC_PPC_FP_CONTROL_INDEX_verify(
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INDEX *info
  )
{
  SOC_SAND_INIT_ERROR_DEFINITIONS_NO_DEVID(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  SOC_SAND_ERR_IF_ABOVE_MAX(info->db_id, ARAD_PP_FP_DB_ID_MAX, ARAD_PP_FP_DB_ID_OUT_OF_RANGE_ERR, 10, exit);
  SOC_SAND_ERR_IF_ABOVE_MAX(info->type, SOC_PPC_FP_CONTROL_TYPE_MAX, ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 20, exit);

  switch (info->type)
  {
  case SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NOF_L4OPS_RANGES, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 30, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_PACKET_SIZE_RANGE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NOF_PKT_SZ_RANGES-1, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 40, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_OUT_LIF_RANGE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NOF_OUT_LIF_RANGES-1, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 50, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_PP_PORT:
  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_OUT_LIF:
  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_ONLY:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NOF_ACE_POINTERS-1, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 60, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ETHERTYPE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_ETHERTYPE_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 70, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_NEXT_PROTOCOL_IP:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NEXT_PROTOCOL_IP_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 80, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_PP_PORT_PROFILE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_NOF_PORTS, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 90, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_L2_ETHERTYPES:
    SOC_SAND_ERR_IF_OUT_OF_RANGE(info->val_ndx, 8, 15, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 100, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_IPV4_NEXT_PROTOCOL:
    SOC_SAND_ERR_IF_OUT_OF_RANGE(info->val_ndx, ARAD_PP_FP_EGR_IPV4_NEXT_PROTOCOL_MIN, ARAD_PP_FP_EGR_IPV4_NEXT_PROTOCOL_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 110, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_L2_L3_KEY_IN_LIF_ENABLE:
  case SOC_PPC_FP_CONTROL_TYPE_L3_IPV6_TCP_CTL_ENABLE:
  case SOC_PPC_FP_CONTROL_TYPE_KEY_CHANGE_SIZE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, 0, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 120, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NOF_HDR_USER_DEFS, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 130, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGRESS_DP:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, 1, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 140, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_INNER_ETH_NOF_VLAN_TAGS:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, SOC_PPC_FP_NOF_PFGS_ARAD, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 150, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE: 
  case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, ARAD_PP_FP_NOF_PORT_PROFILE_NDX_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 160, exit);
    break;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  case SOC_PPC_FP_CONTROL_TYPE_KBP_CACHE:
  case SOC_PPC_FP_CONTROL_TYPE_KBP_COMMIT:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val_ndx, 0, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 160, exit);
    break;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 180, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_CONTROL_INDEX_verify()",0,0);
}

uint32
  SOC_PPC_FP_CONTROL_INFO_verify(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_TYPE type,
    SOC_SAND_IN  SOC_PPC_FP_CONTROL_INFO *info
  )
{
  uint32
    val_ndx;

  SOC_SAND_INIT_ERROR_DEFINITIONS(0);
  SOC_SAND_CHECK_NULL_INPUT(info);

  switch (type)
  {
  case SOC_PPC_FP_CONTROL_TYPE_L4OPS_RANGE:
    for (val_ndx = 0; val_ndx < 4; val_ndx++)
    {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->val[val_ndx], ((1<<16)-1), ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 10, exit);
    }
    break;

  case SOC_PPC_FP_CONTROL_TYPE_PACKET_SIZE_RANGE:
    for (val_ndx = 0; val_ndx < 2; val_ndx++)
    {
      SOC_SAND_ERR_IF_OUT_OF_RANGE(info->val[val_ndx], 1, ((1<<7)-1), ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 20, exit);
    }
    break;

  case SOC_PPC_FP_CONTROL_TYPE_OUT_LIF_RANGE:
  {
    /*
     * Only the first two items on info->val[] array are tested because they are the
     * only ones that are passed down to HW. This is custom tailored.
     * See _bcm_dpp_field_range_reset()
     * See arad_pp_fp_control_set_unsafe(), arad_pp_fp_control_get_unsafe()
     */
    for (val_ndx = 0; val_ndx < 2 ; val_ndx++)
    {
      ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, info->val[val_ndx], ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 30, exit);
      SOC_SAND_ERR_IF_BELOW_MIN(info->val[val_ndx], 1, ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 31, exit);
    }
    break;
  }
  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_PP_PORT:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], ARAD_PP_PORT_MAX, ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 31, exit);
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[1], ((1<< ARAD_PP_FP_PRGE_VAR_IN_BITS)- 1), ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 311, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_ONLY:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], ((1<< ARAD_PP_FP_PRGE_VAR_IN_BITS)- 1), ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 311, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ACE_POINTER_OUT_LIF:
    ARAD_PP_EG_ENCAP_CHECK_OUTLIF_ID(unit, info->val[0], ARAD_PP_FP_VAL_OUT_OF_RANGE_ERR, 31, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_ETHERTYPE:
  case SOC_PPC_FP_CONTROL_TYPE_NEXT_PROTOCOL_IP:
  case SOC_PPC_FP_CONTROL_TYPE_L2_L3_KEY_IN_LIF_ENABLE:
  case SOC_PPC_FP_CONTROL_TYPE_L3_IPV6_TCP_CTL_ENABLE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], 1, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 32, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_EGR_TM_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_PP_PORT_DATA:
      /* No need to verify (32b) */
      break;
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PP_PORT_DATA:
  case SOC_PPC_FP_CONTROL_TYPE_ING_TM_PORT_DATA:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], ARAD_PP_FP_ING_TM_PORT_DATA_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 34, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_L2_ETHERTYPES:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], (1 << 16) - 1, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 35, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGR_IPV4_NEXT_PROTOCOL:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], ARAD_PP_FP_NEXT_PROTOCOL_IP_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 36, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_PP_PORT_PROFILE:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], ARAD_PP_FP_PP_PORT_PROFILE_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 38, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], SOC_PPC_NOF_FP_BASE_HEADER_TYPES, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 40, exit);
    if(info->val[1] & SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_FLAG_NEGATIVE) {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->val[1] & (~SOC_PPC_FP_CONTROL_TYPE_HDR_USER_DEF_FLAG_NEGATIVE), ARAD_PP_UD_HEADER_OFFSET_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 41, exit);
    } else {
      SOC_SAND_ERR_IF_ABOVE_MAX(info->val[1], ARAD_PP_UD_HEADER_OFFSET_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 42, exit);
    }
    SOC_SAND_ERR_IF_OUT_OF_RANGE(info->val[2], ARAD_PP_FP_UDP_NOF_BITS_MIN, ARAD_PP_FP_UDP_NOF_BITS_MAX, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 44, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_EGRESS_DP:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], ARAD_NOF_DROP_PRECEDENCE, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 46, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_INNER_ETH_NOF_VLAN_TAGS:
    SOC_SAND_ERR_IF_ABOVE_MAX(info->val[0], 3, ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 48, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_KEY_CHANGE_SIZE:
    SOC_SAND_ERR_IF_OUT_OF_RANGE(info->val[0], 1, (SOC_DPP_DEFS_GET(unit, tcam_action_width) - 4), ARAD_PP_FP_VAL_NDX_OUT_OF_RANGE_ERR, 50, exit);
    break;

  case SOC_PPC_FP_CONTROL_TYPE_IN_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_IN_TM_PORT_PROFILE: 
  case SOC_PPC_FP_CONTROL_TYPE_OUT_PORT_PROFILE:
  case SOC_PPC_FP_CONTROL_TYPE_FLP_PGM_PROFILE:
    break;

#if defined(INCLUDE_KBP) && !defined(BCM_88030)
  case SOC_PPC_FP_CONTROL_TYPE_KBP_CACHE:
  case SOC_PPC_FP_CONTROL_TYPE_KBP_COMMIT:
    break;
#endif /* defined(INCLUDE_KBP) && !defined(BCM_88030) */

  default:
    SOC_SAND_SET_ERROR_CODE(ARAD_PP_FP_TYPE_OUT_OF_RANGE_ERR, 60, exit);
  }

  SOC_SAND_MAGIC_NUM_VERIFY(info);

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in SOC_PPC_FP_CONTROL_INFO_verify()",0,0);
}


soc_error_t
  arad_pp_fp_presel_max_id_get(
      SOC_SAND_IN  int                      unit,
      SOC_SAND_IN  SOC_PPC_FP_DATABASE_STAGE   stage,
      SOC_SAND_OUT int                   *presel_max_id 
  )
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(arad_pmf_prog_select_eth_section_nof_lines_get(unit,stage, (uint32*)presel_max_id));

exit:
    SOCDNX_FUNC_RETURN;
}

uint32
  arad_pp_fp_pp_test1(
      SOC_SAND_IN  int                           unit
  )
{
    uint32                       
        db_id = 2;
    uint32
      res = SOC_SAND_OK;
    SOC_PPC_FP_DATABASE_INFO
        db_info;
    uint32
        entry_id_ndx=100;
    SOC_PPC_FP_ENTRY_INFO                       
        *entry = NULL;
    SOC_SAND_SUCCESS_FAILURE               
        success;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_ALLOC(entry, SOC_PPC_FP_ENTRY_INFO, 1, "arad_pp_fp_pp_test1.entry");
    SOC_PPC_FP_ENTRY_INFO_clear(entry);
    entry->priority = 100;
    entry->actions[0].type = SOC_PPC_FP_ACTION_TYPE_DEST;
    entry->actions[0].val = 20;
    entry->qual_vals[0].is_valid.arr[0] = 0xFFFFFFFF;
    entry->qual_vals[0].type = SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI;
    entry->qual_vals[0].val.arr[0] = 0x123;

    SOC_PPC_FP_DATABASE_INFO_clear(&db_info);

    /* type */
    db_info.db_type = SOC_PPC_FP_DB_TYPE_TCAM;
    /* actions */
    db_info.action_types[0] = SOC_PPC_FP_ACTION_TYPE_DEST;
    db_info.action_types[1] = SOC_PPC_FP_ACTION_TYPE_OUTLIF;
    /* qual */
    db_info.qual_types[0] = SOC_PPC_FP_QUAL_IRPP_STP_STATE;
    db_info.qual_types[1] = SOC_PPC_FP_QUAL_IRPP_SYSTEM_VSI;
    db_info.strength = 3;
    db_info.supported_pfgs = 0;

    res = arad_pp_fp_database_create_unsafe(
          unit,
          db_id,
          &db_info,
          &success
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    res = arad_pp_fp_entry_add_unsafe(
            unit,
            db_id,
            entry_id_ndx,
            entry,
            &success);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
  ARAD_FREE(entry);
  SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_pp_test1()", 0, 0);
}

uint32
    arad_pp_fp_ire_traffic_send_verify(
        SOC_SAND_IN int          unit,
        SOC_SAND_IN SOC_PPC_FP_PACKET  *packet
    )
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_IRE_TRAFFIC_SEND_VERIFY);
    SOC_SAND_CHECK_NULL_INPUT(packet);

    SOC_SAND_ERR_IF_OUT_OF_RANGE(packet->size_bytes, 64, 200, SOC_PPC_FP_PACKET_SIZE_OUT_OF_RANGE_ERR, 10, exit);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_fp_ire_traffic_send_verify()", 0, 0);
}

uint32
    arad_pp_fp_ire_traffic_send_unsafe(
        SOC_SAND_IN int          unit,
        SOC_SAND_IN SOC_PPC_FP_PACKET  *packet,
        SOC_SAND_IN int          core
    )
{
    uint64
        reg_val_64;
    uint32
        i,
        j,
        reg_ndx,
        nof_regs,
        reg_val,
        res,
        tm_port_number;
    int core_tmp;
    uint8
        sop,
        eop,
        be;
    soc_reg_above_64_val_t
        reg_above_64;
    soc_reg_t
        ire_reg_fap_port_configuration_reg = SOC_IS_JERICHO(unit)? IRE_REG_REASSEMBLY_CONTEXT_CONFIGURATIONr: IRE_REG_FAP_PORT_CONFIGURATIONr,
        ire_regi_pkt_data_reg,
        ire_regi_pkt_data_jericho[4] = {IRE_REGISTER_INTERFACE_PACKET_DATA_BITS_2047_1536r,
            IRE_REGISTER_INTERFACE_PACKET_DATA_BITS_1535_1024r, 
            IRE_REGISTER_INTERFACE_PACKET_DATA_BITS_1023_512r,
            IRE_REGISTER_INTERFACE_PACKET_DATA_BITS_511_0r},
        ire_regi_pkt_data_qax[2] = {IRE_REGISTER_INTERFACE_PACKET_DATA_BITS_1023_512r,
            IRE_REGISTER_INTERFACE_PACKET_DATA_BITS_511_0r};


    SOC_SAND_INIT_ERROR_DEFINITIONS(ARAD_PP_FP_IRE_TRAFFIC_SEND_UNSAFE);

    if ( soc_port_sw_db_local_to_tm_port_get(unit,packet->local_port_src, &tm_port_number,&core_tmp) != SOC_E_NONE ) {
        goto exit; 
    }
    /* Configure the source port */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, soc_reg32_get(unit, ire_reg_fap_port_configuration_reg, REG_PORT_ANY, 0, &reg_val));
    soc_reg_field_set(unit, ire_reg_fap_port_configuration_reg, &reg_val, REG_REASSEMBLY_CONTEXTf, packet->local_port_src);
    soc_reg_field_set(unit, ire_reg_fap_port_configuration_reg, &reg_val, REG_PORT_TERMINATION_CONTEXTf, tm_port_number);
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, soc_reg32_set(unit, ire_reg_fap_port_configuration_reg, REG_PORT_ANY, 0, reg_val));

    /* Verify the data trigger is down */
    res = arad_polling(
              unit,
              ARAD_TIMEOUT,
              ARAD_MIN_POLLS,
              IRE_REGISTER_INTERFACE_PACKET_CONTROLr,
              REG_PORT_ANY,
              0,
              REGI_PKT_SEND_DATAf,
              0
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

    /* NOTE:
     * We assume that the data stored in the buffer as it should be sent,
     * i.e. buffer[0] should be sent first etc.
     * and hence, the 32 msb of the packet is stored in buffer[0],
     * the next in buffer[1] etc.
     */

    for(i = 0; i < packet->size_bytes; i += ARAD_PP_FP_IRE_PACKET_NOF_BYTES_MAX_PER_WRITE) {
        /* data */
        /* Copy the buffer to multiple registers in Jericho */
        nof_regs = SOC_IS_JERICHO(unit)? SOC_SAND_DIV_ROUND_UP(packet->size_bytes, 64): 1;
        for(reg_ndx = 0; reg_ndx < nof_regs; reg_ndx++) {
		/*QAX IRE interface is different from Jericho, Jericho+ and above*/
            ire_regi_pkt_data_reg = SOC_IS_JERICHO(unit)? (SOC_IS_QAX(unit)? ire_regi_pkt_data_qax[reg_ndx] : ire_regi_pkt_data_jericho[reg_ndx] ) : IRE_REGI_PKT_DATAr;
            SOC_REG_ABOVE_64_CLEAR(reg_above_64);
            for(j = 0; j <ARAD_PP_FP_IRE_PACKET_NOF_LONGS_COPY_REG ; ++j) {
                SHR_BITCOPY_RANGE(reg_above_64, 32*j, packet->buffer, 
                                  (32 * ARAD_PP_FP_IRE_PACKET_NOF_LONGS_COPY_REG * reg_ndx) + 
                                  (ARAD_PP_FP_IRE_PACKET_NOF_LONGS_COPY_REG * i) +
                                  32 * ( ARAD_PP_FP_IRE_PACKET_NOF_LONGS_COPY_REG - 1 - j), 32);
            }
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, soc_reg_above_64_set(unit, ire_regi_pkt_data_reg, REG_PORT_ANY, 0, reg_above_64));
        }
        /* control */
        sop = (i == 0) ? TRUE : FALSE;
        eop = (i + ARAD_PP_FP_IRE_PACKET_NOF_BYTES_MAX_PER_WRITE >= packet->size_bytes) ? TRUE : FALSE;
        be = eop ? (packet->size_bytes - i - 1): ((ARAD_PP_FP_IRE_PACKET_NOF_LONGS_COPY_REG * 4) - 1);

        if (SOC_IS_JERICHO(unit)) {
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, soc_reg_get(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, &reg_val_64));
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_SEND_DATAf, 0x0);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_SEND_MODEf, 0x0);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_IS_TDMf, 0x0);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_TDM_CONTEXTf, 0x0);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_ERRf, 0x0);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_SOPf, sop);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_EOPf, eop);
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_BEf, be);
            if (!SOC_IS_QAX(unit)) {
                soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_PIPEf, core);
            }
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, soc_reg_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, reg_val_64));

            SOC_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, soc_reg_get(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, &reg_val_64));
            soc_reg64_field32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val_64, REGI_PKT_SEND_DATAf, 0x1);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, soc_reg_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, reg_val_64));

        } else {
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 90, exit, soc_reg32_get(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, &reg_val));
            soc_reg_field_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val, REGI_PKT_SEND_DATAf, 0x0);
            soc_reg_field_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val, REGI_PKT_ERRf, 0x0);
            soc_reg_field_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val, REGI_PKT_SOPf, sop);
            soc_reg_field_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val, REGI_PKT_EOPf, eop);
            soc_reg_field_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val, REGI_PKT_BEf, be);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, soc_reg32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, reg_val));

            SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, soc_reg32_get(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, &reg_val));
            soc_reg_field_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, &reg_val, REGI_PKT_SEND_DATAf, 0x1);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 120, exit, soc_reg32_set(unit, IRE_REGISTER_INTERFACE_PACKET_CONTROLr, REG_PORT_ANY, 0, reg_val));
        }


        res = arad_polling(
                  unit,
                  ARAD_TIMEOUT,
                  ARAD_MIN_POLLS,
                  IRE_REGISTER_INTERFACE_PACKET_CONTROLr,
                  REG_PORT_ANY,
                  0,
                  REGI_PKT_SEND_DATAf,
                  0
              );
        SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_fp_ire_traffic_send()", 0, 0);
}

/*
 * given a location in tcam bank, search for the entry in SW database. 
 * If exists remove the entry and then rewrite it (using BCM APIs), 
 * otherwise, write an empty entry. 
 * the argument global_location is only valid if entry_exists is true
 */
uint32
  arad_pp_fp_rewrite_entry(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint8                      entry_exists,
    SOC_SAND_IN  ARAD_TCAM_GLOBAL_LOCATION  *global_location,
    SOC_SAND_IN  ARAD_TCAM_LOCATION         *location
  )
{
    uint32
        res = SOC_SAND_OK;
    uint8 
        found;
    ARAD_TCAM_ENTRY
        entry;
    ARAD_TCAM_BANK_ENTRY_SIZE
        entry_size;
    bcm_field_entry_t
        bcm_entry;
    
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(location);
    SOC_SAND_CHECK_NULL_INPUT(global_location);

    if (entry_exists)
    {
        found = FALSE;

        /* get BCM entry ID from PPD entry ID */
        res = _bcm_dpp_field_entry_ppd_to_bcm(unit, global_location->entry_id, &bcm_entry, &found);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if (found)
        {
            /* use BCM API to remove entry from HW and rewrite it.
             * It is unusual to call a BCM API is from SOC layer, 
             * however, it is done only in this specific case where 
             * data should be restored from BCM software databases.
             */

            /* remove entry from HW only (invalidates entry) */
            res = bcm_field_entry_remove(unit, bcm_entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            /* write entry to HW and validate it */
            res = bcm_field_entry_install(unit, bcm_entry);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
        }
    }
    else
    {
        ARAD_TCAM_ENTRY_clear(&entry);

        res = sw_state_access[unit].dpp.soc.arad.tm.tcam.bank.entry_size.get(unit, location->bank_id, &entry_size);
        SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 40, exit);

        /* write entry to specific location in HW without SW management */
        res = arad_tcam_entry_rewrite(unit, FALSE, 0, location, entry_size, &entry);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in arad_pp_ip_tcam_rewrite_entry()", 0, 0);
}


uint32
  arad_pp_fp_qual_type_preset(
    SOC_SAND_IN  int                   unit,
    SOC_SAND_IN  SOC_PPC_FP_PREDEFINED_ACL_KEY predefined_key,
    SOC_SAND_OUT SOC_PPC_FP_QUAL_TYPE          qual_types[SOC_PPC_FP_NOF_QUALS_PER_DB_MAX]
  )
{
  uint32
    res = SOC_SAND_OK;
  uint32
    qual_type_ndx;
  SOC_PPC_FP_CONTROL_INFO
    control_info;
  SOC_PPC_FP_CONTROL_INDEX
    control_index;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_QUAL_TYPE_PRESET1);

  for (qual_type_ndx = 0; qual_type_ndx < SOC_PPC_FP_NOF_QUALS_PER_DB_MAX; ++qual_type_ndx)
  {
    qual_types[qual_type_ndx] = BCM_FIELD_ENTRY_INVALID;
  }

  qual_type_ndx = 0;
  switch (predefined_key)
  {
    /*
     * L2
     */
  case SOC_PPC_FP_PREDEFINED_ACL_KEY_L2:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_VLAN_FORMAT;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_2ND_VLAN_TAG;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_ETHERTYPE;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT;
  	break;

    /*
     * L3 IPv4
     */
  case SOC_PPC_FP_PREDEFINED_ACL_KEY_IPV4:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV4_L4OPS_HI;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_NEXT_PRTCL;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DF;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_MF;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SRC_PORT;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DEST_PORT;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_TOS;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_TCP_CTL;

    SOC_PPC_FP_CONTROL_INDEX_clear(&control_index);
    SOC_PPC_FP_CONTROL_INFO_clear(&control_info);
    control_index.type = SOC_PPC_FP_CONTROL_TYPE_L2_L3_KEY_IN_LIF_ENABLE;
    res = arad_pp_fp_control_get_unsafe(
            unit,
            SOC_CORE_INVALID,
            &control_index,
            &control_info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (control_info.val[0] == 0)
    {
      qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT;
      qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_IN_VID;
    }
    else
    {
      qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_IRPP_IN_LIF;
    }
    break;

    /*
     * L3 IPv6
     */
  case SOC_PPC_FP_PREDEFINED_ACL_KEY_IPV6:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_L4OPS;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_LOW;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_DIP_HIGH;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_DIP_LOW;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_NEXT_PRTCL;

    SOC_PPC_FP_CONTROL_INDEX_clear(&control_index);
    SOC_PPC_FP_CONTROL_INFO_clear(&control_info);
    control_index.type = SOC_PPC_FP_CONTROL_TYPE_L3_IPV6_TCP_CTL_ENABLE;
    res = arad_pp_fp_control_get_unsafe(
            unit,
            SOC_CORE_INVALID,
            &control_index,
            &control_info
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    if (control_info.val[0] == 0)
    {
      qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_IRPP_SRC_PP_PORT;
    }
    else
    {
      qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_IPV6_TCP_CTL;
    }

    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_IRPP_IN_LIF;
    break;
 
  case SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_ETH:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_ETHERTYPE;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_IRPP_ETH_TAG_FORMAT;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_2ND_VLAN_TAG;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_SA;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_DA;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_PP_PORT_DATA;
    break;

  case SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_IPV4:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_TOS;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_IPV4_NEXT_PROTOCOL;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_SIP;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_IPV4_DIP;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_FWD_VLAN_TAG;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_PP_PORT_DATA;
    break;

  case SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_MPLS:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_FTMH;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_PAYLOAD;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_MPLS_LABEL_ID_FWD;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_MPLS_EXP_FWD;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_HDR_MPLS_TTL_FWD;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_PP_PORT_DATA;
    break;

  case SOC_PPC_FP_PREDEFINED_ACL_KEY_EGR_TM:
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_FTMH;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_PAYLOAD;
    qual_types[qual_type_ndx++] = SOC_PPC_FP_QUAL_ERPP_PP_PORT_DATA;
    break;

  default:
    break;
 }

  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_fp_qual_type_preset()", 0, 0);
}

uint32
  arad_pp_fp_qual_val_encode(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  SOC_PPC_FP_QUAL_VAL_ENCODE_INFO *qual_val_encode,
    SOC_SAND_OUT SOC_PPC_FP_QUAL_VAL             *qual_val
  )
{
  uint32
    mac_add_long[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S],
    mac_add_long_valid[SOC_SAND_PP_MAC_ADDRESS_NOF_UINT32S],
    dest_buffer,
    asd_buffer,
    subnet_length,
    res;
  uint32
    ipv6_first_long,
    arr_ndx;
  ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE
    appl_type=ARAD_PP_FRWRD_DECISION_APPLICATION_TYPE_DFLT;

  SOC_SAND_INIT_ERROR_DEFINITIONS(SOC_PPC_FP_QUAL_VAL_ENCODE);

  SOC_PPC_FP_QUAL_VAL_clear(qual_val);

  switch (qual_val_encode->type)
  {
  case SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_FWD_DECISION:
    qual_val->type = SOC_PPC_FP_QUAL_IRPP_FWD_DEC_DEST;

    /*
     * Transform to 17b encoding
     */
    res = arad_pp_fwd_decision_in_buffer_build(
            unit,
            appl_type,
            &(qual_val_encode->val.fd.fwd_dec),
            &dest_buffer,
            &asd_buffer
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    qual_val->val.arr[0] = dest_buffer;
    qual_val->val.arr[1] = asd_buffer;
  	break;

  case SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_MAC_ADDRESS:
    qual_val->type = SOC_PPC_FP_QUAL_HDR_SA;
  /* The function soc_sand_pp_mac_address_struct_to_long writes to indecies 0 and 1 of the second parameter only */
  /* coverity[overrun-buffer-val : FALSE] */   
    res = soc_sand_pp_mac_address_struct_to_long(
            &(qual_val_encode->val.mac.mac),
            mac_add_long
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

/*
 * COVERITY
 *
 * There is no overrun of static array "mac_add_long_valid" .
 */
/* coverity[overrun-buffer-val] */
    res = soc_sand_pp_mac_address_struct_to_long(
            &(qual_val_encode->val.mac.is_valid),
            mac_add_long_valid
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    for (arr_ndx = 0; arr_ndx < SOC_SAND_U64_NOF_UINT32S; ++arr_ndx)
    {
      qual_val->val.arr[arr_ndx] = mac_add_long[arr_ndx];
      qual_val->is_valid.arr[arr_ndx] = mac_add_long_valid[arr_ndx];
    }
  	break;

  case SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_IPV4_SUBNET:
    qual_val->type = SOC_PPC_FP_QUAL_HDR_IPV4_SIP;
    qual_val->val.arr[0] = qual_val_encode->val.ipv4.ip.address[0];
    subnet_length = qual_val_encode->val.ipv4.subnet_length;
    /*
     * In case of no subnet length, no valid bits
     */
    if (subnet_length != 0)
    {
      if (subnet_length > ARAD_PP_FP_SUBNET_LENGTH_IPV4_MAX)
      {
        subnet_length = ARAD_PP_FP_SUBNET_LENGTH_IPV4_MAX;
      }
      qual_val->is_valid.arr[0] = SOC_SAND_BITS_MASK(31, 31 - (subnet_length - 1));
    }
  	break;

  case SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_IPV6_SUBNET:
    qual_val->type = SOC_PPC_FP_QUAL_HDR_IPV6_SIP_HIGH;

    /*
     * Set Data
     */
    if (qual_val_encode->val.ipv6.is_low == TRUE)
    {
      ipv6_first_long = 0; /*Take bits 63:0*/
    }
    else
    {
      ipv6_first_long = 2; /*Take bits 127:64*/
    }
    for (arr_ndx = 0; arr_ndx < SOC_SAND_U64_NOF_UINT32S; ++arr_ndx)
    {
      qual_val->val.arr[arr_ndx] = qual_val_encode->val.ipv6.ip.address[arr_ndx + ipv6_first_long];
    }

    /*
     * Set mask
     */
    subnet_length = qual_val_encode->val.ipv6.subnet_length;

    /*
     * In case of no subnet length, no valid bits
     */
    if (subnet_length != 0)
    {
      if (subnet_length > ARAD_PP_FP_SUBNET_LENGTH_IPV6_MAX)
      {
        subnet_length = ARAD_PP_FP_SUBNET_LENGTH_IPV6_MAX;
      }
      if (subnet_length > 32)
      {
        qual_val->is_valid.arr[1] = SOC_SAND_BITS_MASK(31, 0);
        qual_val->is_valid.arr[0] = SOC_SAND_BITS_MASK(31, 31 - ((subnet_length - 32) - 1));
      }
      else
      {
        qual_val->is_valid.arr[1] = SOC_SAND_BITS_MASK(31, 31 - (subnet_length - 1));
      }
    }
  	break;

  case SOC_PPC_FP_QUAL_VAL_ENCODE_INFO_TYPE_ETH_TAG_FORMAT:
    qual_val->type = SOC_PPC_FP_QUAL_IRPP_ETH_TAG_FORMAT;
    qual_val->is_valid.arr[0] = SOC_SAND_BITS_MASK(4, 0);
    qual_val->val.arr[0] = (qual_val_encode->val.etf.tag_format.tag_outer << 3) +
      (qual_val_encode->val.etf.tag_format.is_priority << 2) +
      qual_val_encode->val.etf.tag_format.tag_inner;
    break;


  default:
    break;
  }

  ARAD_PP_DO_NOTHING_AND_EXIT;

exit:
  SOC_SAND_EXIT_AND_SEND_ERROR("error in soc_ppd_fp_qual_val_encode()", 0, 0);
}


/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>

#undef _ERR_MSG_MODULE_NAME



#endif /* of #if defined(BCM_88650_A0) */
