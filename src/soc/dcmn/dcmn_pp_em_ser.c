#include <soc/mcm/memregs.h>
/*
 * $Id: jer_pp_em_ser.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

/*************
 * INCLUDES  *
 *************/

#ifdef BCM_DPP_SUPPORT
#include <soc/dcmn/error.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>
#include <soc/dpp/SAND/Utils/sand_os_interface.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_mact_mgmt.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_lem_access.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_frwrd_ipv4.h>
#include <soc/dpp/drv.h>
#include <soc/mem.h>
#include <sal/core/dpc.h>
#include <soc/dpp/ARAD/arad_sim_em.h>
#include <soc/dcmn/dcmn_pp_em_ser.h>
#include <soc/dcmn/dcmn_intr_corr_act_func.h>
/*************
 * DEFINES   *
 *************/

#define JER_CHIP_SIM_LEM_KEY                  ARAD_CHIP_SIM_LEM_KEY
#define JER_CHIP_SIM_LEM_PAYLOAD              ARAD_CHIP_SIM_LEM_PAYLOAD
#define JER_CHIP_SIM_LEM_BASE                 ARAD_CHIP_SIM_LEM_BASE
#define JER_CHIP_SIM_LEM_TABLE_SIZE           ARAD_CHIP_SIM_LEM_TABLE_SIZE

#define JER_CHIP_SIM_ISEM_KEY                 ARAD_CHIP_SIM_ISEM_A_KEY
#define JER_CHIP_SIM_ISEM_PAYLOAD             ARAD_CHIP_SIM_ISEM_A_PAYLOAD
#define JER_CHIP_SIM_ISEM_BASE                ARAD_CHIP_SIM_ISEM_A_BASE
#define JER_CHIP_SIM_ISEM_TABLE_SIZE          ARAD_CHIP_SIM_ISEM_A_TABLE_SIZE

#define JER_CHIP_SIM_GLEM_KEY                 ARAD_CHIP_SIM_GLEM_KEY
#define JER_CHIP_SIM_GLEM_PAYLOAD             ARAD_CHIP_SIM_GLEM_PAYLOAD
#define JER_CHIP_SIM_GLEM_BASE                ARAD_CHIP_SIM_GLEM_BASE
#define JER_CHIP_SIM_GLEM_TABLE_SIZE          ARAD_CHIP_SIM_GLEM_TABLE_SIZE

#define JER_CHIP_SIM_ESEM_KEY                 ARAD_CHIP_SIM_ESEM_KEY
#define JER_CHIP_SIM_ESEM_PAYLOAD             ARAD_CHIP_SIM_ESEM_PAYLOAD
#define JER_CHIP_SIM_ESEM_BASE                ARAD_CHIP_SIM_ESEM_BASE
#define JER_CHIP_SIM_ESEM_TABLE_SIZE          ARAD_CHIP_SIM_ESEM_TABLE_SIZE

#define JER_CHIP_SIM_RMAPEM_KEY               ARAD_CHIP_SIM_RMAPEM_KEY
#define JER_CHIP_SIM_RMAPEM_PAYLOAD           ARAD_CHIP_SIM_RMAPEM_PAYLOAD
#define JER_CHIP_SIM_RMAPEM_BASE              ARAD_CHIP_SIM_RMAPEM_BASE
#define JER_CHIP_SIM_RMAPEM_TABLE_SIZE        ARAD_CHIP_SIM_RMAPEM_TABLE_SIZE

#define JER_CHIP_SIM_OEMA_KEY                 ARAD_CHIP_SIM_OEMA_KEY
#define JER_CHIP_SIM_OEMA_PAYLOAD             ARAD_CHIP_SIM_OEMA_PAYLOAD
#define JER_CHIP_SIM_OEMA_BASE                ARAD_CHIP_SIM_OEMA_BASE
#define JER_CHIP_SIM_OEMA_TABLE_SIZE          ARAD_CHIP_SIM_OEMA_TABLE_SIZE

#define JER_CHIP_SIM_OEMB_KEY                 ARAD_CHIP_SIM_OEMB_KEY
#define JER_CHIP_SIM_OEMB_PAYLOAD             ARAD_CHIP_SIM_OEMB_PAYLOAD
#define JER_CHIP_SIM_OEMB_BASE                ARAD_CHIP_SIM_OEMB_BASE
#define JER_CHIP_SIM_OEMB_TABLE_SIZE          ARAD_CHIP_SIM_OEMB_TABLE_SIZE

#define JER_PP_EM_SER_LEM_KEYT_BASE_ADDR       (0xF600000)
#define JER_PP_EM_SER_LEM_NOF_LINES            (2048*1024)

#define JER_PP_EM_SER_ISEM_KEYT_BASE_ADDR      (0x1A00000)
#define JER_PP_EM_SER_ISEM_NOF_LINES           (64*1024)

#define JER_PP_EM_SER_GLEM_KEYT_BASE_ADDR      (0x2C20000)
#define JER_PP_EM_SER_GLEM_NOF_LINES           (64*1024)

#define JER_PP_EM_SER_ESEM_KEYT_BASE_ADDR      (0x520000)
#define JER_PP_EM_SER_ESEM_NOF_LINES           (64*1024)

#define JER_PP_EM_SER_OEMA_KEYT_BASE_ADDR      (0x12000000)
#define JER_PP_EM_SER_OEMA_NOF_LINES           (32*1024)

#define JER_PP_EM_SER_OEMB_KEYT_BASE_ADDR      (0x15000000)
#define JER_PP_EM_SER_OEMB_NOF_LINES           (16*1024)

#define JER_PP_EM_SER_RMAPEM_KEYT_BASE_ADDR    (0x100000)
#define JER_PP_EM_SER_RMAPEM_NOF_LINES         (32*1024)

#define JER_PP_EM_SER_NOF_HASH_RESULTS             8
#define JER_PP_EM_SER_NOF_HASH6_RESULTS            6
#define JER_PP_EM_SER_EM_AUX_CAM_SIZE              32

#define JER_PP_EM_SER_LEM_VERIFIER_LEN             59
#define JER_PP_EM_SER_LEM_VERIFIER_LEN_IN_UINT32   (JER_PP_EM_SER_LEM_VERIFIER_LEN / SOC_SAND_NOF_BITS_IN_UINT32 + 1)

#define JER_PP_EM_SER_LEM_NOF_FIDS                 (32 * 1024)

#define LEM_ERR_MEM_MASK_REGISTER       PPDB_B_ECC_ERR_2B_MONITOR_MEM_MASKr
#define ISEM_ERR_MEM_MASK_REGISTER      IHB_ECC_ERR_2B_MONITOR_MEM_MASKr
#define GLEM_ERR_MEM_MASK_REGISTER      EDB_ECC_ERR_2B_MONITOR_MEM_MASKr
#define ESEM_ERR_MEM_MASK_REGISTER      EDB_ECC_ERR_2B_MONITOR_MEM_MASKr
#define OEMA_ERR_MEM_MASK_REGISTER      PPDB_A_ECC_ERR_2B_MONITOR_MEM_MASKr
#define OEMB_ERR_MEM_MASK_REGISTER      PPDB_A_ECC_ERR_2B_MONITOR_MEM_MASKr
#define RMAPEM_ERR_MEM_MASK_REGISTER    OAMP_ECC_ERR_2B_MONITOR_MEM_MASKr

#define JER_PP_EM_SER_ISEM_REQUEST_KEY_START_BIT       11
#define JER_PP_EM_SER_GLEM_REQUEST_KEY_START_BIT       11
#define JER_PP_EM_SER_ESEM_REQUEST_KEY_START_BIT       11
#define JER_PP_EM_SER_OEMA_REQUEST_KEY_START_BIT       11
#define JER_PP_EM_SER_OEMB_REQUEST_KEY_START_BIT       11
#define JER_PP_EM_SER_RMAPEM_REQUEST_KEY_START_BIT     11

#define JER_PP_EM_SER_ISEM_REQUEST_PAYLOAD_START_BIT       61
#define JER_PP_EM_SER_GLEM_REQUEST_PAYLOAD_START_BIT       29
#define JER_PP_EM_SER_ESEM_REQUEST_PAYLOAD_START_BIT       51
#define JER_PP_EM_SER_OEMA_REQUEST_PAYLOAD_START_BIT       31
#define JER_PP_EM_SER_OEMB_REQUEST_PAYLOAD_START_BIT       34
#define JER_PP_EM_SER_RMAPEM_REQUEST_PAYLOAD_START_BIT     38

#define JER_PP_EM_SER_ISEM_REQUEST_KEY_SIZE       50
#define JER_PP_EM_SER_GLEM_REQUEST_KEY_SIZE       18
#define JER_PP_EM_SER_ESEM_REQUEST_KEY_SIZE       40
#define JER_PP_EM_SER_OEMA_REQUEST_KEY_SIZE       20
#define JER_PP_EM_SER_OEMB_REQUEST_KEY_SIZE       23
#define JER_PP_EM_SER_RMAPEM_REQUEST_KEY_SIZE     27

#define JER_PP_EM_SER_ISEM_REQUEST_PAYLOAD_SIZE       17
#define JER_PP_EM_SER_GLEM_REQUEST_PAYLOAD_SIZE       18
#define JER_PP_EM_SER_ESEM_REQUEST_PAYLOAD_SIZE       17
#define JER_PP_EM_SER_OEMA_REQUEST_PAYLOAD_SIZE       35
#define JER_PP_EM_SER_OEMB_REQUEST_PAYLOAD_SIZE       18
#define JER_PP_EM_SER_RMAPEM_REQUEST_PAYLOAD_SIZE     15

#define JER_PP_EM_SER_REQUEST_TYPE_START_BIT  0
#define JER_PP_EM_SER_REQUEST_TYPE_SIZE       3
#define JER_PP_EM_SER_REQUEST_TYPE_INSERT_VAL 1
#define JER_PP_EM_SER_REQUEST_TYPE_DELETE_VAL 0

/*
 * when traversing the MACT, to perform action on each entry how many
 * entries to return in each iteration
 */
#define JER_PP_EM_SER_LEM_TRVRS_ITER_BLK_SIZE      (130)

/*************
 * MACROS    *
 *************/

#define MANAGEMENT_UNIT_CONFIGURATION_REGISTER(block,db)    block##_##db##_MANAGEMENT_UNIT_CONFIGURATION_REGISTERr
#define MNGMNT_UNIT_ENABLE_FIELD(db)                        db##_MNGMNT_UNIT_ENABLEf

#define ECC_2B_ERR_CNT_REGISTER(block)                      block##_ECC_2B_ERR_CNTr
#define MANAGEMENT_REQUEST_MEM(block, db)                   block##_##db##_MANAGEMENT_REQUESTm
#define KEYT_PLDT_MEM(block, db)                            block##_##db##_KEYT_PLDT_Hm
#define KEYT_AUX_MEM(block, db)                             block##_##db##_KEYT_AUXm
#define PLDT_AUX_MEM(block, db)                             block##_##db##_PLDT_AUXm
#define ENTRIES_COUNTER_REGISTER(block, db)                 block##_##db##_ENTRIES_COUNTERr
#define DIAGNOSTICS_REGISTER(block, db)                     block##_##db##_DIAGNOSTICSr
#define DIAGNOSTICS_INDEX_REGISTER(block, db)               block##_##db##_DIAGNOSTICS_INDEXr
#define DIAGNOSTICS_KEY_REGISTER(block, db)                 block##_##db##_DIAGNOSTICS_KEYr
#define DIAGNOSTICS_READ_RESULT_REGISTER(block, db)         block##_##db##_DIAGNOSTICS_READ_RESULTr
#define DIAGNOSTICS_LOOKUP_RESULT_REGISTER(block, db)       block##_##db##_DIAGNOSTICS_LOOKUP_RESULTr
#define DIAGNOSTICS_READ_FIELD(db)                          db##_DIAGNOSTICS_READf
#define DIAGNOSTICS_LOOKUP_FIELD(db)                        db##_DIAGNOSTICS_LOOKUPf

#define MASK_SER_INTERRUPTS_32B_SET(db, mask_register, reg, val) \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_0_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_1_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_2_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_3_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_4_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_5_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_6_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_7_ECC_2B_ERR_MASKf, val)

#define MASK_SER_INTERRUPTS_6A_32B_SET(db, mask_register, reg, val) \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_0_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_1_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_2_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_3_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_4_ECC_2B_ERR_MASKf, val);   \
    soc_reg_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_5_ECC_2B_ERR_MASKf, val)

#define MASK_SER_INTERRUPTS_64B_SET(db, mask_register, reg, val) \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_0_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_1_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_2_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_3_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_4_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_5_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_6_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_7_ECC_2B_ERR_MASKf, val)

#define MASK_SER_INTERRUPTS_6A_64B_SET(db, mask_register, reg, val) \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_0_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_1_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_2_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_3_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_4_ECC_2B_ERR_MASKf, val);   \
    soc_reg64_field_set(unit, mask_register, reg, db##_KEYT_PLDT_H_5_ECC_2B_ERR_MASKf, val)


/*************
 * TYPE DEFS *
 *************/

typedef enum
{
    JER_PP_EM_SER_DB_TYPE_LEM,
    JER_PP_EM_SER_DB_TYPE_ISEM,
    JER_PP_EM_SER_DB_TYPE_GLEM,
    JER_PP_EM_SER_DB_TYPE_ESEM,
    JER_PP_EM_SER_DB_TYPE_OEMA,
    JER_PP_EM_SER_DB_TYPE_OEMB,
    JER_PP_EM_SER_DB_TYPE_RMAPEM,
    JER_PP_EM_SER_DB_TYPE_NOF_TYPES
}JER_PP_EM_SER_DB_TYPE;

typedef struct
{
    JER_PP_EM_SER_DB_TYPE db_type;

    /* request table info */
    soc_mem_t req_mem;
    uint32 key_start_bit;
    uint32 key_size;
    uint32 payload_start_bit;
    uint32 payload_size;

    /* shadow table info */
    uint32 shadow_key_size;
    uint32 shadow_payload_size;
    uint32 shadow_base;

    /* diagnostics info */
    soc_reg_t diagnostics_reg;
    soc_reg_t diagnostics_key_reg;
    soc_reg_t diagnostics_lookup_result_reg;
    soc_reg_t diagnostics_read_result_reg;
    soc_reg_t diagnostics_index_reg;
    soc_field_t diagnostics_lookup_field;
    soc_field_t diagnostics_read_field;

    soc_mem_t keyt_pldt_mem;
    soc_mem_t keyt_aux_mem;
    soc_mem_t pldt_aux_mem;
    soc_reg_t counter_reg;
    uint32 db_nof_lines;

}JER_PP_SER_EM_TYPE_INFO;

/*************
 * GLOBALS   *
 *************/
/* variables to control global and fid/lif counters handling */
static int fix_global_counters=1;
static int fix_fid_counters=1;

static uint32 p1, p2, p3, p4;
static SOC_PPC_FRWRD_MACT_ENTRY_KEY    l2_traverse_mact_keys[JER_PP_EM_SER_LEM_TRVRS_ITER_BLK_SIZE];
static SOC_PPC_FRWRD_MACT_ENTRY_VALUE  l2_traverse_mact_vals[JER_PP_EM_SER_LEM_TRVRS_ITER_BLK_SIZE];
static uint16 lem_fid_lif_counters[JER_PP_EM_SER_LEM_NOF_FIDS]={0};

/*
 * Possible locations in the different EM databases for an entry with key=all_ones.
 */
uint32 jer_entry_index[JER_PP_EM_SER_DB_TYPE_NOF_TYPES][JER_PP_EM_SER_NOF_HASH_RESULTS] =
            {{16375, 5903, 22433, 27896, 9645, 5150, 26058, 16115}, /* LEM */
             {1957, 1828, 50, 887, 3229, 70, 2892, 603},            /* ISEM */
             {1957, 1828, 50, 887, 3229, 70, 2892, 603},            /* GLEM */
             {1451, 487, 1382, 1807, 1605, 179, 1995, 1228},        /* ESEM */
             {1852, 875, 646, 39, 334, 6, 1808, 1753},              /* OEMA */
             {902, 35, 610, 708, 197, 518, 722, 315},               /* OEMB */
             {124, 652, 406, 97, 1140, 1467, 587, 1481}             /* RMAPEM */
            };

/*************
 * FUNCTIONS *
 *************/

uint32 jer_pp_ser_em_type_info_get(int unit, JER_PP_EM_SER_TYPE em_ser_type, JER_PP_SER_EM_TYPE_INFO *ser_info)
{

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    switch (em_ser_type) {
        case JER_PP_EM_SER_TYPE_ISEM_KEYT_PLDT:
            ser_info->db_type = JER_PP_EM_SER_DB_TYPE_ISEM;
            ser_info->req_mem = MANAGEMENT_REQUEST_MEM(IHB, ISEM);
            ser_info->key_start_bit = JER_PP_EM_SER_ISEM_REQUEST_KEY_START_BIT;
            ser_info->key_size = JER_PP_EM_SER_ISEM_REQUEST_KEY_SIZE;
            ser_info->payload_start_bit = JER_PP_EM_SER_ISEM_REQUEST_PAYLOAD_START_BIT;
            ser_info->payload_size = JER_PP_EM_SER_ISEM_REQUEST_PAYLOAD_SIZE;
            ser_info->shadow_key_size = JER_CHIP_SIM_ISEM_KEY;
            ser_info->shadow_payload_size = JER_CHIP_SIM_ISEM_PAYLOAD;
            ser_info->shadow_base = JER_CHIP_SIM_ISEM_BASE;
            ser_info->db_nof_lines = JER_PP_EM_SER_ISEM_NOF_LINES;
            ser_info->diagnostics_reg = DIAGNOSTICS_REGISTER(IHB, ISEM);
            ser_info->diagnostics_key_reg = DIAGNOSTICS_KEY_REGISTER(IHB, ISEM);
            ser_info->diagnostics_lookup_result_reg = DIAGNOSTICS_LOOKUP_RESULT_REGISTER(IHB, ISEM);
            ser_info->diagnostics_lookup_field = DIAGNOSTICS_LOOKUP_FIELD(ISEM);
            ser_info->diagnostics_read_result_reg = DIAGNOSTICS_READ_RESULT_REGISTER(IHB, ISEM);
            ser_info->diagnostics_read_field = DIAGNOSTICS_READ_FIELD(ISEM);
            ser_info->diagnostics_index_reg = DIAGNOSTICS_INDEX_REGISTER(IHB, ISEM);
            ser_info->keyt_aux_mem = KEYT_AUX_MEM(IHB, ISEM);
            ser_info->pldt_aux_mem = PLDT_AUX_MEM(IHB, ISEM);
            ser_info->keyt_pldt_mem = KEYT_PLDT_MEM(IHB, ISEM);
            ser_info->counter_reg = ENTRIES_COUNTER_REGISTER(IHB, ISEM);
            break;

        case JER_PP_EM_SER_TYPE_GLEM_KEYT_PLDT:
            ser_info->db_type = JER_PP_EM_SER_DB_TYPE_GLEM;
            ser_info->req_mem = MANAGEMENT_REQUEST_MEM(EDB, GLEM);
            ser_info->key_start_bit = JER_PP_EM_SER_GLEM_REQUEST_KEY_START_BIT;
            ser_info->key_size = JER_PP_EM_SER_GLEM_REQUEST_KEY_SIZE;
            ser_info->payload_start_bit = JER_PP_EM_SER_GLEM_REQUEST_PAYLOAD_START_BIT;
            ser_info->payload_size = JER_PP_EM_SER_GLEM_REQUEST_PAYLOAD_SIZE;
            ser_info->shadow_key_size = JER_CHIP_SIM_GLEM_KEY;
            ser_info->shadow_payload_size = JER_CHIP_SIM_GLEM_PAYLOAD;
            ser_info->shadow_base = JER_CHIP_SIM_GLEM_BASE;
            ser_info->db_nof_lines = JER_PP_EM_SER_GLEM_NOF_LINES;
            ser_info->diagnostics_reg = DIAGNOSTICS_REGISTER(EDB, GLEM);
            ser_info->diagnostics_key_reg = DIAGNOSTICS_KEY_REGISTER(EDB, GLEM);
            ser_info->diagnostics_lookup_result_reg = DIAGNOSTICS_LOOKUP_RESULT_REGISTER(EDB, GLEM);
            ser_info->diagnostics_lookup_field = DIAGNOSTICS_LOOKUP_FIELD(GLEM);
            ser_info->diagnostics_read_result_reg = DIAGNOSTICS_READ_RESULT_REGISTER(EDB, GLEM);
            ser_info->diagnostics_read_field = DIAGNOSTICS_READ_FIELD(GLEM);
            ser_info->diagnostics_index_reg = DIAGNOSTICS_INDEX_REGISTER(EDB, GLEM);
            ser_info->keyt_aux_mem = KEYT_AUX_MEM(EDB, GLEM);
            ser_info->pldt_aux_mem = PLDT_AUX_MEM(EDB, GLEM);
            ser_info->keyt_pldt_mem = KEYT_PLDT_MEM(EDB, GLEM);
            ser_info->counter_reg = ENTRIES_COUNTER_REGISTER(EDB, GLEM);
            break;

        case JER_PP_EM_SER_TYPE_ESEM_KEYT_PLDT:
            ser_info->db_type = JER_PP_EM_SER_DB_TYPE_ESEM;
            ser_info->req_mem = MANAGEMENT_REQUEST_MEM(EDB, ESEM);
            ser_info->key_start_bit = JER_PP_EM_SER_ESEM_REQUEST_KEY_START_BIT;
            ser_info->key_size = JER_PP_EM_SER_ESEM_REQUEST_KEY_SIZE;
            ser_info->payload_start_bit = JER_PP_EM_SER_ESEM_REQUEST_PAYLOAD_START_BIT;
            ser_info->payload_size = JER_PP_EM_SER_ESEM_REQUEST_PAYLOAD_SIZE;
            ser_info->shadow_key_size = JER_CHIP_SIM_ESEM_KEY;
            ser_info->shadow_payload_size = JER_CHIP_SIM_ESEM_PAYLOAD;
            ser_info->shadow_base = JER_CHIP_SIM_ESEM_BASE;
            ser_info->db_nof_lines = JER_PP_EM_SER_ESEM_NOF_LINES;
            ser_info->diagnostics_reg = DIAGNOSTICS_REGISTER(EDB, ESEM);
            ser_info->diagnostics_key_reg = DIAGNOSTICS_KEY_REGISTER(EDB, ESEM);
            ser_info->diagnostics_lookup_result_reg = DIAGNOSTICS_LOOKUP_RESULT_REGISTER(EDB, ESEM);
            ser_info->diagnostics_lookup_field = DIAGNOSTICS_LOOKUP_FIELD(ESEM);
            ser_info->diagnostics_read_result_reg = DIAGNOSTICS_READ_RESULT_REGISTER(EDB, ESEM);
            ser_info->diagnostics_read_field = DIAGNOSTICS_READ_FIELD(ESEM);
            ser_info->diagnostics_index_reg = DIAGNOSTICS_INDEX_REGISTER(EDB, ESEM);
            ser_info->keyt_aux_mem = KEYT_AUX_MEM(EDB, ESEM);
            ser_info->pldt_aux_mem = PLDT_AUX_MEM(EDB, ESEM);
            ser_info->keyt_pldt_mem = KEYT_PLDT_MEM(EDB, ESEM);
            ser_info->counter_reg = ENTRIES_COUNTER_REGISTER(EDB, ESEM);
            break;

        case JER_PP_EM_SER_TYPE_OEMA_KEYT_PLDT:
            ser_info->db_type = JER_PP_EM_SER_DB_TYPE_OEMA;
            ser_info->req_mem = MANAGEMENT_REQUEST_MEM(PPDB_A, OEMA);
            ser_info->key_start_bit = JER_PP_EM_SER_OEMA_REQUEST_KEY_START_BIT;
            ser_info->key_size = JER_PP_EM_SER_OEMA_REQUEST_KEY_SIZE;
            ser_info->payload_start_bit = JER_PP_EM_SER_OEMA_REQUEST_PAYLOAD_START_BIT;
            ser_info->payload_size = JER_PP_EM_SER_OEMA_REQUEST_PAYLOAD_SIZE;
            ser_info->shadow_key_size = JER_CHIP_SIM_OEMA_KEY;
            ser_info->shadow_payload_size = JER_CHIP_SIM_OEMA_PAYLOAD;
            ser_info->shadow_base = JER_CHIP_SIM_OEMA_BASE;
            ser_info->db_nof_lines = JER_PP_EM_SER_OEMA_NOF_LINES;
            ser_info->diagnostics_reg = DIAGNOSTICS_REGISTER(PPDB_A, OEMA);
            ser_info->diagnostics_key_reg = DIAGNOSTICS_KEY_REGISTER(PPDB_A, OEMA);
            ser_info->diagnostics_lookup_result_reg = DIAGNOSTICS_LOOKUP_RESULT_REGISTER(PPDB_A, OEMA);
            ser_info->diagnostics_lookup_field = DIAGNOSTICS_LOOKUP_FIELD(OEMA);
            ser_info->diagnostics_read_result_reg = DIAGNOSTICS_READ_RESULT_REGISTER(PPDB_A, OEMA);
            ser_info->diagnostics_read_field = DIAGNOSTICS_READ_FIELD(OEMA);
            ser_info->diagnostics_index_reg = DIAGNOSTICS_INDEX_REGISTER(PPDB_A, OEMA);
            ser_info->keyt_aux_mem = KEYT_AUX_MEM(PPDB_A, OEMA);
            ser_info->pldt_aux_mem = PLDT_AUX_MEM(PPDB_A, OEMA);
            ser_info->keyt_pldt_mem = KEYT_PLDT_MEM(PPDB_A, OEMA);
            ser_info->counter_reg = ENTRIES_COUNTER_REGISTER(PPDB_A, OEMA);
            break;

        case JER_PP_EM_SER_TYPE_OEMB_KEYT_PLDT:
            ser_info->db_type = JER_PP_EM_SER_DB_TYPE_OEMB;
            ser_info->req_mem = MANAGEMENT_REQUEST_MEM(PPDB_A, OEMB);
            ser_info->key_start_bit = JER_PP_EM_SER_OEMB_REQUEST_KEY_START_BIT;
            ser_info->key_size = JER_PP_EM_SER_OEMB_REQUEST_KEY_SIZE;
            ser_info->payload_start_bit = JER_PP_EM_SER_OEMB_REQUEST_PAYLOAD_START_BIT;
            ser_info->payload_size = JER_PP_EM_SER_OEMB_REQUEST_PAYLOAD_SIZE;
            ser_info->shadow_key_size = JER_CHIP_SIM_OEMB_KEY;
            ser_info->shadow_payload_size = JER_CHIP_SIM_OEMB_PAYLOAD;
            ser_info->shadow_base = JER_CHIP_SIM_OEMB_BASE;
            ser_info->db_nof_lines = JER_PP_EM_SER_OEMB_NOF_LINES;
            ser_info->diagnostics_reg = DIAGNOSTICS_REGISTER(PPDB_A, OEMB);
            ser_info->diagnostics_key_reg = DIAGNOSTICS_KEY_REGISTER(PPDB_A, OEMB);
            ser_info->diagnostics_lookup_result_reg = DIAGNOSTICS_LOOKUP_RESULT_REGISTER(PPDB_A, OEMB);
            ser_info->diagnostics_lookup_field = DIAGNOSTICS_LOOKUP_FIELD(OEMB);
            ser_info->diagnostics_read_result_reg = DIAGNOSTICS_READ_RESULT_REGISTER(PPDB_A, OEMB);
            ser_info->diagnostics_read_field = DIAGNOSTICS_READ_FIELD(OEMB);
            ser_info->diagnostics_index_reg = DIAGNOSTICS_INDEX_REGISTER(PPDB_A, OEMB);
            ser_info->keyt_aux_mem = KEYT_AUX_MEM(PPDB_A, OEMB);
            ser_info->pldt_aux_mem = PLDT_AUX_MEM(PPDB_A, OEMB);
            ser_info->keyt_pldt_mem = KEYT_PLDT_MEM(PPDB_A, OEMB);
            ser_info->counter_reg = ENTRIES_COUNTER_REGISTER(PPDB_A, OEMB);
            break;

        case JER_PP_EM_SER_TYPE_RMAPEM_KEYT_PLDT:
            ser_info->db_type = JER_PP_EM_SER_DB_TYPE_RMAPEM;
            ser_info->req_mem = MANAGEMENT_REQUEST_MEM(OAMP, RMAPEM);
            ser_info->key_start_bit = JER_PP_EM_SER_RMAPEM_REQUEST_KEY_START_BIT;
            ser_info->key_size = JER_PP_EM_SER_RMAPEM_REQUEST_KEY_SIZE;
            ser_info->payload_start_bit = JER_PP_EM_SER_RMAPEM_REQUEST_PAYLOAD_START_BIT;
            ser_info->payload_size = JER_PP_EM_SER_RMAPEM_REQUEST_PAYLOAD_SIZE;
            ser_info->shadow_key_size = JER_CHIP_SIM_RMAPEM_KEY;
            ser_info->shadow_payload_size = JER_CHIP_SIM_RMAPEM_PAYLOAD;
            ser_info->shadow_base = JER_CHIP_SIM_RMAPEM_BASE;
            ser_info->db_nof_lines = JER_PP_EM_SER_RMAPEM_NOF_LINES;
            ser_info->diagnostics_reg = DIAGNOSTICS_REGISTER(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->diagnostics_key_reg = DIAGNOSTICS_KEY_REGISTER(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->diagnostics_lookup_result_reg = DIAGNOSTICS_LOOKUP_RESULT_REGISTER(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->diagnostics_lookup_field = DIAGNOSTICS_LOOKUP_FIELD(REMOTE_MEP_EXACT_MATCH);
            ser_info->diagnostics_read_result_reg = DIAGNOSTICS_READ_RESULT_REGISTER(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->diagnostics_read_field = DIAGNOSTICS_READ_FIELD(REMOTE_MEP_EXACT_MATCH);
            ser_info->diagnostics_index_reg = DIAGNOSTICS_INDEX_REGISTER(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->keyt_aux_mem = KEYT_AUX_MEM(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->pldt_aux_mem = PLDT_AUX_MEM(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->keyt_pldt_mem = KEYT_PLDT_MEM(OAMP, REMOTE_MEP_EXACT_MATCH);
            ser_info->counter_reg = ENTRIES_COUNTER_REGISTER(OAMP, REMOTE_MEP_EXACT_MATCH);
            break;

        default:
            break;
    }

    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_mask_interrupt_set(int unit,
                                         JER_PP_EM_SER_TYPE em_ser_type,
                                         uint32 val)
{
    uint32 res = SOC_SAND_OK;
    soc_reg_above_64_val_t reg_above_64_val, mask_val_above_64;
    uint64 reg_64_val, mask_val_64;
    uint32 reg_32_val, mask_val_32;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOC_REG_ABOVE_64_CLEAR(mask_val_above_64);
    mask_val_32 = val;
    COMPILER_64_SET(mask_val_64,0,val);
    mask_val_above_64[0] = val;

    switch (em_ser_type) {
        case JER_PP_EM_SER_TYPE_LEM_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_PPDB_B_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, &reg_32_val));
            MASK_SER_INTERRUPTS_6A_32B_SET(MACT, LEM_ERR_MEM_MASK_REGISTER, &reg_32_val, mask_val_32);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_PPDB_B_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, reg_32_val));
            break;

        case JER_PP_EM_SER_TYPE_ISEM_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_IHB_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, 0, &reg_32_val));
            MASK_SER_INTERRUPTS_32B_SET(ISEM, ISEM_ERR_MEM_MASK_REGISTER, &reg_32_val, mask_val_32);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_IHB_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, 0, reg_32_val));
            break;

        case JER_PP_EM_SER_TYPE_GLEM_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, READ_EDB_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, &reg_64_val));
            MASK_SER_INTERRUPTS_6A_64B_SET(GLEM, GLEM_ERR_MEM_MASK_REGISTER, &reg_64_val, mask_val_64);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 40, exit, WRITE_EDB_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, reg_64_val));
            break;

        case JER_PP_EM_SER_TYPE_ESEM_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 50, exit, READ_EDB_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, &reg_64_val));
            MASK_SER_INTERRUPTS_64B_SET(ESEM, ESEM_ERR_MEM_MASK_REGISTER, &reg_64_val, mask_val_64);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, WRITE_EDB_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, reg_64_val));
            break;

        case JER_PP_EM_SER_TYPE_OEMA_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 70, exit, READ_PPDB_A_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, &reg_64_val));
            MASK_SER_INTERRUPTS_64B_SET(OEMA, OEMA_ERR_MEM_MASK_REGISTER, &reg_64_val, mask_val_64);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 80, exit, WRITE_PPDB_A_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, reg_64_val));
            break;

        case JER_PP_EM_SER_TYPE_OEMB_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 90, exit, READ_PPDB_A_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, &reg_64_val));
            MASK_SER_INTERRUPTS_64B_SET(OEMB, OEMB_ERR_MEM_MASK_REGISTER, &reg_64_val, mask_val_64);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 100, exit, WRITE_PPDB_A_ECC_ERR_2B_MONITOR_MEM_MASKr(unit, reg_64_val));
            break;

        case JER_PP_EM_SER_TYPE_RMAPEM_KEYT_PLDT:
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 110, exit, READ_OAMP_ECC_ERR_2B_MONITOR_MEM_MASKr_REG32(unit, &reg_32_val));
            MASK_SER_INTERRUPTS_64B_SET(RMAPEM, RMAPEM_ERR_MEM_MASK_REGISTER, &reg_64_val, mask_val_64);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 120, exit, WRITE_OAMP_ECC_ERR_2B_MONITOR_MEM_MASKr_REG32(unit, reg_32_val));
            break;

        default:
            res = SOC_SAND_ERR;
            break;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_management_enable_set(int unit,
                                            JER_PP_EM_SER_DB_TYPE db_type,
                                            int val)
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    switch (db_type) {
       case JER_PP_EM_SER_DB_TYPE_LEM:
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(PPDB_B,LARGE_EM), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(LARGE_EM),  val));
            break;

       case JER_PP_EM_SER_DB_TYPE_ISEM:
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(IHB,ISEM), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(ISEM),  val));
            break;

       case JER_PP_EM_SER_DB_TYPE_GLEM:
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(EDB,GLEM), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(GLEM),  val));
            break;
       case JER_PP_EM_SER_DB_TYPE_ESEM:
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(EDB,ESEM), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(ESEM),  val));
            break;

       case JER_PP_EM_SER_DB_TYPE_OEMA:
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(PPDB_A,OEMA), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(OEMA),  val));
            break;

       case JER_PP_EM_SER_DB_TYPE_OEMB:
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(PPDB_A,OEMB), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(OEMB),  val));
            break;

       case JER_PP_EM_SER_DB_TYPE_RMAPEM:
           SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  70,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MANAGEMENT_UNIT_CONFIGURATION_REGISTER(OAMP,REMOTE_MEP_EXACT_MATCH), REG_PORT_ANY, 0, MNGMNT_UNIT_ENABLE_FIELD(REMOTE_MEP_EXACT_MATCH),  val));
            break;

        default:
            res = SOC_SAND_ERR;
            break;
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}
int jer_pp_em_ser_ecc_error_info_get(int unit,
                                         JER_PP_EM_SER_BLOCK block,
                                         uint32 *address,
                                         uint32 *address_valid,
                                         uint32 *counter,
                                         uint32 *counter_overflow)
{
    int res;
    uint64 reg_val;
    soc_reg_t ecc_err_reg;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_SAND_CHECK_NULL_INPUT(address);
    SOC_SAND_CHECK_NULL_INPUT(address_valid);
    SOC_SAND_CHECK_NULL_INPUT(counter);
    SOC_SAND_CHECK_NULL_INPUT(counter_overflow);

    switch (block) {
       case JER_PP_EM_SER_BLOCK_IHB:
           ecc_err_reg = ECC_2B_ERR_CNT_REGISTER(IHB);
           break;

       case JER_PP_EM_SER_BLOCK_PPDB_A:
           ecc_err_reg = ECC_2B_ERR_CNT_REGISTER(PPDB_A);
           break;

       case JER_PP_EM_SER_BLOCK_PPDB_B:
           ecc_err_reg = ECC_2B_ERR_CNT_REGISTER(PPDB_B);
           break;

       case JER_PP_EM_SER_BLOCK_EDB:
           ecc_err_reg = ECC_2B_ERR_CNT_REGISTER(EDB);
           break;

       case JER_PP_EM_SER_BLOCK_OAMP:
           ecc_err_reg = ECC_2B_ERR_CNT_REGISTER(OAMP);
           break;

       default:
           res = SOC_SAND_ERR;
           goto exit;
    }
    res = soc_reg_get(unit, ecc_err_reg, REG_PORT_ANY, 0, &reg_val);
    SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 10, exit);

    /* get address validity bit */
    *address_valid = soc_reg64_field32_get(unit, ecc_err_reg, reg_val, ECC_2B_ERR_ADDR_VALIDf);

    /* get memory address bit */
    *address = soc_reg64_field32_get(unit, ecc_err_reg, reg_val, ECC_2B_ERR_ADDRf);

     /* get counter overflow indication  */
    *counter_overflow = soc_reg64_field32_get(unit, ecc_err_reg, reg_val, ECC_2B_ERR_CNT_OVERFLOWf);

    /* get counter value of 2 bit error */
    *counter = soc_reg64_field32_get(unit, ecc_err_reg, reg_val, ECC_2B_ERR_CNTf);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/*
 * Get SER address
 */
uint32 jer_pp_em_ser_type_get(int unit, soc_mem_t mem, JER_PP_EM_SER_TYPE *em_ser_type)
{
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    switch(mem)
    {
        case KEYT_PLDT_MEM(PPDB_B, LARGE_EM):
            *em_ser_type = JER_PP_EM_SER_TYPE_LEM_KEYT_PLDT;
            break;

        case KEYT_PLDT_MEM(IHB, ISEM):
            *em_ser_type = JER_PP_EM_SER_TYPE_ISEM_KEYT_PLDT;
            break;

        case KEYT_PLDT_MEM(EDB, GLEM):
            *em_ser_type = JER_PP_EM_SER_TYPE_GLEM_KEYT_PLDT;
            break;

        case KEYT_PLDT_MEM(EDB, ESEM):
            *em_ser_type = JER_PP_EM_SER_TYPE_ESEM_KEYT_PLDT;
            break;

        case KEYT_PLDT_MEM(PPDB_A, OEMA):
            *em_ser_type = JER_PP_EM_SER_TYPE_OEMA_KEYT_PLDT;
            break;

        case KEYT_PLDT_MEM(PPDB_A, OEMB):
            *em_ser_type = JER_PP_EM_SER_TYPE_OEMB_KEYT_PLDT;
            break;

        case KEYT_PLDT_MEM(OAMP, REMOTE_MEP_EXACT_MATCH):
            *em_ser_type = JER_PP_EM_SER_TYPE_RMAPEM_KEYT_PLDT;
            break;

        default:
            break;
    }

    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Increment the LEM global counter */
uint32 jer_pp_em_lem_counter_increment(int unit, int inc_val)
{
    uint32 i, j, is_found=0;
    uint32 key_data[SOC_DPP_DEFS_MAX_LEM_WIDTH_IN_UINT32S];
    uint32 verifier_data[JER_PP_EM_SER_LEM_VERIFIER_LEN_IN_UINT32] = {0};
    soc_reg_above_64_val_t reg_above_64_val, request_val, fld_value, tmp, dummy_key_value;
    uint32 *entry_index, entry_location;
    uint32 res = SOC_SAND_OK;
    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOC_REG_ABOVE_64_CLEAR(fld_value);
    SOC_REG_ABOVE_64_CLEAR(dummy_key_value);
    SOC_REG_ABOVE_64_CLEAR(tmp);
    SOC_REG_ABOVE_64_CLEAR(request_val);

    entry_index = jer_entry_index[JER_PP_EM_SER_DB_TYPE_LEM];

    sal_memset(tmp, 0xff, sizeof(soc_reg_above_64_val_t));
    res = soc_sand_bitstream_set_any_field(
          tmp,
          0,
          SOC_DPP_DEFS_MAX(LEM_WIDTH),
          dummy_key_value
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (i = 0; i < inc_val; i++) {
        /* Front door insertion to LEM (via management) of dummy entry */
        fld_value[0] = 1;
        ARAD_FLD_TO_REG_ABOVE_64(PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr, LARGE_EM_REQ_MFF_IS_KEYf, fld_value, request_val,  15, exit);
        ARAD_FLD_TO_REG_ABOVE_64(PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr, LARGE_EM_REQ_MFF_KEYf, dummy_key_value, request_val,  15, exit);
        ARAD_FLD_TO_REG_ABOVE_64(PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr, LARGE_EM_REQ_COMMANDf, fld_value, request_val,  15, exit);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr(unit, request_val));

        /* Backdoor removal of the entry */
        for (j = 0; j < JER_PP_EM_SER_NOF_HASH6_RESULTS; j++) {
            /*
             * Check in which of the 8 possible locations in the LEM, the dummy entry was inserted.
             * When found, delete it directly (not via management).
             */
            entry_location = entry_index[j] + (JER_PP_EM_SER_LEM_NOF_LINES/JER_PP_EM_SER_NOF_HASH6_RESULTS) * j;
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 30, exit, WRITE_PPDB_B_LARGE_EM_DIAGNOSTICS_INDEXr(unit, entry_location));
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR, soc_reg_field32_modify(unit, PPDB_B_LARGE_EM_DIAGNOSTICSr, REG_PORT_ANY, LARGE_EM_DIAGNOSTICS_READf, 1));

            /* Poll on the trigger bit before getting the result */
                res = arad_polling(
                      unit,
                      ARAD_TIMEOUT,
                      ARAD_MIN_POLLS,
                      PPDB_B_LARGE_EM_DIAGNOSTICSr,
                      REG_PORT_ANY,
                      0,
                      LARGE_EM_DIAGNOSTICS_READf,
                      0
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 60, exit, READ_PPDB_B_LARGE_EM_DIAGNOSTICS_READ_RESULTr(unit, reg_above_64_val));

            key_data[0] = 0;
            key_data[1] = 0;
            key_data[2] = 0;

            /* Get the key data */
            res = soc_sand_bitstream_get_any_field(
                  reg_above_64_val,
                  1, /* start bit */
                  74, /* nof bits */
                  key_data
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);


            if ((key_data[0] == dummy_key_value[0]) &&
                (key_data[1] == dummy_key_value[1]) &&
                (key_data[2] == dummy_key_value[2])) {
                /* found the dummy entry */
                is_found = 1;

                /* Delete the entry directly by setting the key to all zeros (including is-valid bit) */
                res = soc_mem_write(unit,
                                KEYT_PLDT_MEM(PPDB_B, LARGE_EM),
                                MEM_BLOCK_ANY,
                                entry_location,
                                verifier_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

                break;
            }
        }
        if (!is_found) {
            /*
             * Dummy entry was not found in any of the possible entries in the LEM.
             * search the CAM.
             */
            for (j=0; j < JER_PP_EM_SER_EM_AUX_CAM_SIZE; j++) {
                res = soc_mem_read(unit,
                                   KEYT_AUX_MEM(PPDB_B, LARGE_EM),
                                   MEM_BLOCK_ANY,
                                   j,
                                   reg_above_64_val);
                SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

                if ((reg_above_64_val[0] == dummy_key_value[0]) &&
                    (reg_above_64_val[1] == dummy_key_value[1]) &&
                    (reg_above_64_val[2] == dummy_key_value[2])) {
                    /* found the dummy entry */
                    is_found = 1;
                    key_data[0] = 0;
                    key_data[1] = 0;
                    key_data[2] = 0;

                    /* Delete the entry directly by setting the key to all zeros (including is-valid bit) */
                    res = soc_mem_write(unit,
                                    KEYT_AUX_MEM(PPDB_B, LARGE_EM),
                                    MEM_BLOCK_ANY,
                                    j,
                                    key_data);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);
                    break;
                }
            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Increment the EM global counter */
uint32 jer_pp_em_em_counter_increment(int unit, JER_PP_SER_EM_TYPE_INFO *ser_info, int inc_val)
{
    uint32 i, j, is_found=0;
    uint32 key_data[2] = {0};
    uint32 verifier_data[2] = {0};
    soc_reg_above_64_val_t reg_above_64_val, tmp, dummy_key_value, read_result;
    uint32 *entry_index, entry_location, temp, buffer[3] = {0};
    uint32 res = SOC_SAND_OK;
	uint32 hash;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if( ser_info->db_type == JER_PP_EM_SER_DB_TYPE_GLEM) {
	    hash = JER_PP_EM_SER_NOF_HASH6_RESULTS;
    }
    else {
	    hash = JER_PP_EM_SER_NOF_HASH_RESULTS;
    }


    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOC_REG_ABOVE_64_CLEAR(dummy_key_value);
    SOC_REG_ABOVE_64_CLEAR(tmp);
    SOC_REG_ABOVE_64_CLEAR(read_result);

    /* Get the possible locations for the dummy entry */
    entry_index = jer_entry_index[ser_info->db_type];

    /* Set dummy key */
    sal_memset(tmp, 0xff, sizeof(soc_reg_above_64_val_t));
    res = soc_sand_bitstream_set_any_field(
          tmp,
          0,
          ser_info->key_size,
          dummy_key_value
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (i = 0; i < inc_val; i++) {
        /* Front door insertion to EM (via management) of dummy entry */

        /* Set type field */
        temp = JER_PP_EM_SER_REQUEST_TYPE_INSERT_VAL;
        res = soc_sand_bitstream_set_any_field(
              &temp,
              JER_PP_EM_SER_REQUEST_TYPE_START_BIT,
              JER_PP_EM_SER_REQUEST_TYPE_SIZE,
              buffer
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        /* Set key field */
        res = soc_sand_bitstream_set_any_field(
              dummy_key_value,
              ser_info->key_start_bit,
              ser_info->key_size,
              buffer
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        /* Write the insertion request */
        res = soc_mem_write(unit, ser_info->req_mem, MEM_BLOCK_ALL, 0, buffer);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

        /* Backdoor removal of the entry */
        for (j = 0; j < hash; j++) {
            /*
             * Check in which of the 8 possible locations in the EM, the dummy entry was inserted.
             * When found, delete it directly (not via management).
             */
            entry_location = entry_index[j] + ((ser_info->db_nof_lines)/hash) * j;
            res = soc_reg32_set(unit, ser_info->diagnostics_index_reg, REG_PORT_ANY, 0, entry_location);
            SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ser_info->diagnostics_reg, REG_PORT_ANY, 0, ser_info->diagnostics_read_field, 1));

            /* Poll on the trigger bit before getting the result */
                res = arad_polling(
                      unit,
                      ARAD_TIMEOUT,
                      ARAD_MIN_POLLS,
                      ser_info->diagnostics_reg,
                      REG_PORT_ANY,
                      0,
                      ser_info->diagnostics_read_field,
                      0
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);
            res = soc_reg_above_64_get(unit, ser_info->diagnostics_read_result_reg, REG_PORT_ANY, 0, read_result);
            SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

            /* Get the key data */
            res = soc_sand_bitstream_get_any_field(
                  read_result,
                  1, /* start bit */
                  ser_info->key_size, /* nof bits */
                  key_data
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);


            if ((key_data[0] == dummy_key_value[0]) && (key_data[1] == dummy_key_value[1])) {
                is_found = 1;

                /* Delete the entry directly by setting the key to all zeros (including is-valid bit) */
                res = soc_mem_write(unit,
                                ser_info->keyt_pldt_mem,
                                MEM_BLOCK_ANY,
                                entry_location,
                                verifier_data);
                SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

                break;
            }
        }
        if (!is_found) {
            /*
             * Dummy entry was not found in any of the possible entries in the LEM.
             * search the CAM.
             */
            for (j=0; j < JER_PP_EM_SER_EM_AUX_CAM_SIZE; j++) {
                res = soc_mem_read(unit,
                                   ser_info->keyt_aux_mem,
                                   MEM_BLOCK_ANY,
                                   j,
                                   reg_above_64_val);
                SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

                if ((reg_above_64_val[0] == dummy_key_value[0]) &&
                    (reg_above_64_val[1] == dummy_key_value[1])) {
                    /* found the dummy entry */
                    is_found = 1;
                    key_data[0] = 0;
                    key_data[1] = 0;

                    /* Delete the entry directly by setting the key to all zeros (including is-valid bit) */
                    res = soc_mem_write(unit,
                                    ser_info->keyt_aux_mem,
                                    MEM_BLOCK_ANY,
                                    j,
                                    key_data);
                    SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
                    break;
                }
            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Decrement the LEM global counter */
uint32 jer_pp_em_lem_counter_decrement(int unit, int dec_val)
{
    uint32 res = SOC_SAND_OK;
    uint32 is_valid;
    int i,j;
    soc_reg_above_64_val_t reg_above_64_val, dummy_key_value, request_val, fld_value, tmp;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOC_REG_ABOVE_64_CLEAR(dummy_key_value);
    SOC_REG_ABOVE_64_CLEAR(request_val);
    SOC_REG_ABOVE_64_CLEAR(fld_value);
    SOC_REG_ABOVE_64_CLEAR(tmp);

    /* Init dummy entry */
    sal_memset(tmp, 0xff, sizeof(soc_reg_above_64_val_t));
    res = soc_sand_bitstream_set_any_field(
          tmp,
          0,
          SOC_DPP_DEFS_LEM_WIDTH_IN_UINT32S(unit) + 1, /* adding valid bit */
          dummy_key_value
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (i = 0; i < dec_val; i++) {
        /* Search for a free location in the auxiliary CAM */
        for (j=0; j < JER_PP_EM_SER_EM_AUX_CAM_SIZE; j++) {
            res = soc_mem_read(unit,
                               KEYT_AUX_MEM(PPDB_B, LARGE_EM),
                               MEM_BLOCK_ANY,
                               j,
                               reg_above_64_val);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            res = soc_sand_bitstream_get_any_field(
                  reg_above_64_val,
                  0, /* start bit */
                  1, /* nof bits */
                  &is_valid);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            if (!is_valid) {
                /* found a free location in index 'j' */
                break;
            }
        }

        if (j == JER_PP_EM_SER_EM_AUX_CAM_SIZE) {
            /* Free location in the auxiliary CAM was not found */
            res = SOC_SAND_ERR;
            goto exit;;
        }

        /* Backdoor insertion of entry to CAM */
        res = soc_mem_write(unit,
                            KEYT_AUX_MEM(PPDB_B, LARGE_EM),
                            MEM_BLOCK_ANY,
                            j, /* index of the CAM */
                            dummy_key_value);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

        SOC_REG_ABOVE_64_CLEAR(dummy_key_value);
        res = soc_sand_bitstream_set_any_field(
              tmp,
              0,
              SOC_DPP_DEFS_LEM_WIDTH_IN_UINT32S(unit),
              dummy_key_value
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

        /* Front door removal of the entry */
        fld_value[0] = 1;
        ARAD_FLD_TO_REG_ABOVE_64(PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr, LARGE_EM_REQ_MFF_IS_KEYf, fld_value, request_val,  60, exit);
        ARAD_FLD_TO_REG_ABOVE_64(PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr, LARGE_EM_REQ_MFF_KEYf, dummy_key_value, request_val,  70, exit);
        fld_value[0] = 0;
        ARAD_FLD_TO_REG_ABOVE_64(PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr, LARGE_EM_REQ_COMMANDf, fld_value, request_val,  80, exit);
        SOC_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, WRITE_PPDB_B_LARGE_EM_CPU_REQUEST_REQUESTr(unit, request_val));
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Decrement the EM global counter */
uint32 jer_pp_em_em_counter_decrement(int unit, JER_PP_SER_EM_TYPE_INFO *ser_info, int dec_val)
{
    uint32 res = SOC_SAND_OK;
    uint32 is_valid, temp, buffer[3] = {0};
    int i,j;
    soc_reg_above_64_val_t reg_above_64_val, dummy_key_value, request_val, tmp;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);
    SOC_REG_ABOVE_64_CLEAR(dummy_key_value);
    SOC_REG_ABOVE_64_CLEAR(request_val);
    SOC_REG_ABOVE_64_CLEAR(tmp);

    /* Init dummy entry */
    sal_memset(tmp, 0xff, sizeof(soc_reg_above_64_val_t));
    res = soc_sand_bitstream_set_any_field(
          tmp,
          0,
          ser_info->key_size + 1, /* adding valid bit */
          dummy_key_value
        );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    for (i = 0; i < dec_val; i++) {
        /* Search for a free location in the auxiliary CAM */
        for (j=0; j < JER_PP_EM_SER_EM_AUX_CAM_SIZE; j++) {
            res = soc_mem_read(unit,
                               ser_info->keyt_aux_mem,
                               MEM_BLOCK_ANY,
                               j,
                               reg_above_64_val);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            res = soc_sand_bitstream_get_any_field(
                  reg_above_64_val,
                  0, /* start bit */
                  1, /* nof bits */
                  &is_valid);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            if (!is_valid) {
                /* found a free location in index 'j' */
                break;
            }
        }

        if (j == JER_PP_EM_SER_EM_AUX_CAM_SIZE) {
            /* Free location in the auxiliary CAM was not found */
            res = SOC_SAND_ERR;
            goto exit;
        }

        /* Backdoor insertion of entry to CAM */
        res = soc_mem_write(unit,
                            ser_info->keyt_aux_mem,
                            MEM_BLOCK_ANY,
                            j, /* index of the CAM */
                            dummy_key_value);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

        SOC_REG_ABOVE_64_CLEAR(dummy_key_value);
        res = soc_sand_bitstream_set_any_field(
              tmp,
              0,
              ser_info->key_size,
              dummy_key_value
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

        /* Front door removal of the entry */
        /* Set type field */
        temp = JER_PP_EM_SER_REQUEST_TYPE_DELETE_VAL;
        res = soc_sand_bitstream_set_any_field(
              &temp,
              JER_PP_EM_SER_REQUEST_TYPE_START_BIT,
              JER_PP_EM_SER_REQUEST_TYPE_SIZE,
              buffer
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

        /* Set key field */
        res = soc_sand_bitstream_set_any_field(
              dummy_key_value,
              ser_info->key_start_bit,
              ser_info->key_size,
              buffer
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

        /* Write the insertion request */
        res = soc_mem_write(unit, ser_info->req_mem, MEM_BLOCK_ALL, 0, buffer);
        SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
    }
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_mact_counter_fix(int unit)
{
    uint32 res = SOC_SAND_OK;
    ARAD_PP_LEM_ACCESS_KEY                          rule_key, rule_key_mask;
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE    rule_val;
    SOC_SAND_TABLE_BLOCK_RANGE                      block_range;
    SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION              action;
    uint32 nof_matched_mact_entries;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_PP_LEM_ACCESS_KEY_clear(&rule_key);
    ARAD_PP_LEM_ACCESS_KEY_clear(&rule_key_mask);
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE_clear(&rule_val);
    SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_clear(&action);
    soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);

    block_range.entries_to_scan = SOC_SAND_TBL_ITER_SCAN_ALL;
    rule_key.type = ARAD_PP_LEM_ACCESS_KEY_TYPE_MAC;
    rule_key.prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_MAC;
    rule_key.prefix.value = ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_MAC(unit);

    rule_key_mask.type = ARAD_PP_LEM_ACCESS_KEY_TYPE_MAC;
    rule_key_mask.prefix.nof_bits = ARAD_PP_LEM_ACCESS_KEY_PREFIX_SIZE_IN_BITS_FOR_MAC;
    rule_key_mask.prefix.value = 0xf;

    action.type = SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_NONE; /* count only */

    /* activate traverse for MACT */
    res = arad_pp_frwrd_lem_traverse_internal_unsafe(
            unit,
            &rule_key,
            &rule_key_mask,
            &rule_val,
            &block_range,
            &action,
            TRUE, /* wait_till_finish,*/
            &nof_matched_mact_entries
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Write matched number of entries to MACT counter */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_PPDB_B_LARGE_EM_COUNTER_LIMIT_LARGE_EM_DB_ENTRIES_COUNTr(unit, nof_matched_mact_entries));

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Get diff between number of entries in LEM (via flush machine) and LEM counter */
uint32 jer_pp_em_ser_lem_counter_diff_get(int unit, int *lem_counter_diff)
{
    uint32 res = SOC_SAND_OK;
    uint32 nof_matched_lem_entries, nof_lem_entries;
    ARAD_PP_LEM_ACCESS_KEY                          rule_key, rule_key_mask;
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE    rule_val;
    SOC_SAND_TABLE_BLOCK_RANGE                      block_range;
    SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION              action;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_PP_LEM_ACCESS_KEY_clear(&rule_key);
    ARAD_PP_LEM_ACCESS_KEY_clear(&rule_key_mask);
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_VALUE_RULE_clear(&rule_val);
    SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_clear(&action);
    soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);

    /* Get LEM counter value */
    SOC_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_PPDB_B_LARGE_EM_ENTRIES_COUNTERr(unit, &nof_lem_entries));

    block_range.entries_to_scan = SOC_SAND_TBL_ITER_SCAN_ALL;
    rule_key.prefix.value = ARAD_PP_LEM_ACCESS_KEY_PREFIX_FOR_MAC(unit);
    action.type = SOC_PPC_FRWRD_MACT_TRAVERSE_ACTION_TYPE_NONE; /* count only */

    /* activate traverse for LEM */
    res = arad_pp_frwrd_lem_traverse_internal_unsafe(
            unit,
            &rule_key,
            &rule_key_mask,
            &rule_val,
            &block_range,
            &action,
            TRUE, /* wait_till_finish,*/
            &nof_matched_lem_entries
          );
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    *lem_counter_diff = nof_matched_lem_entries - nof_lem_entries;

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Get diff between number of entries in EM and EM counter */
uint32 jer_pp_em_ser_em_counter_diff_get(int unit, JER_PP_SER_EM_TYPE_INFO *ser_info, int *em_counter_diff)
{
    uint32 res = SOC_SAND_OK;
    uint32 nof_valid_entries=0, nof_em_entries, is_valid, entry_index, array_index;
    uint32 key_data[SOC_DPP_DEFS_MAX_LEM_WIDTH_IN_UINT32S] = {0};
	uint32 hash;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if( ser_info->db_type == JER_PP_EM_SER_DB_TYPE_GLEM) {
	    hash = JER_PP_EM_SER_NOF_HASH6_RESULTS;
    }
    else {
	    hash = JER_PP_EM_SER_NOF_HASH_RESULTS;
    }

    /* Get EM counter value */
    res = soc_reg32_get(unit, ser_info->counter_reg, REG_PORT_ANY, 0, &nof_em_entries);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    for (array_index = 0; array_index < hash; array_index++) {
        for (entry_index=0; entry_index < (ser_info->db_nof_lines/hash); entry_index++) {
            is_valid = 0;

            res = soc_mem_array_read(unit,
                                     ser_info->keyt_pldt_mem,
                                     array_index,
                                     MEM_BLOCK_ANY,
                                     entry_index,
                                     &key_data);
            SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

            /* Get is-valid bit */
            res = soc_sand_bitstream_get_any_field(
                  key_data,
                  0, /* start bit */
                  1, /* nof bits */
                  &is_valid
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

            if (is_valid) {
                nof_valid_entries++;
            }
        }
    }

    *em_counter_diff = nof_valid_entries - nof_em_entries;
exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}


/* Update LEM counters per FID */
uint32 jer_pp_em_ser_lem_fid_counters_update(int unit)
{
    uint32 res = SOC_SAND_OK;
    uint32 nof_entries, indx, fid;
    uint32 tbl_data[1];
    SOC_SAND_TABLE_BLOCK_RANGE                      block_range;
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE          rule;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE_clear(&rule);

    sal_memset(lem_fid_lif_counters, 0, sizeof(uint16)*JER_PP_EM_SER_LEM_NOF_FIDS);

    /*
     * Count entries per FID via flush machine
     */
    SOC_SAND_TBL_ITER_SET_BEGIN(&(block_range.iter));
    block_range.entries_to_act = JER_PP_EM_SER_LEM_TRVRS_ITER_BLK_SIZE;
    block_range.entries_to_scan = SOC_SAND_TBL_ITER_SCAN_ALL;

    rule.key_type = SOC_PPC_FRWRD_MACT_KEY_TYPE_MAC_ADDR;

    while(!SOC_SAND_TBL_ITER_IS_END(&(block_range.iter))) {
        /* activate traverse for MACT */
        res = soc_ppd_frwrd_mact_get_block(unit,
                                           &rule,
                                           SOC_PPC_FRWRD_MACT_TABLE_SW_HW,
                                           &block_range,
                                           l2_traverse_mact_keys,
                                           l2_traverse_mact_vals,
                                           &nof_entries);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(nof_entries == 0) {
            break;
        }
        for (indx = 0; indx < nof_entries; ++indx) {
            fid = l2_traverse_mact_keys[indx].key_val.mac.fid;
            /* increment relevant FID counter */
            lem_fid_lif_counters[fid]++;
        }
    }

    /* Write all FID counters */
    for (indx = 0; indx < JER_PP_EM_SER_LEM_NOF_FIDS; ++indx) {
        tbl_data[0] = lem_fid_lif_counters[indx];
        res = soc_mem_write(unit,
                            PPDB_B_LARGE_EM_FID_COUNTER_DBm,
                            MEM_BLOCK_ANY,
                            indx,
                            tbl_data);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/* Update LEM counters per LIF */
uint32 jer_pp_em_ser_lem_lif_counters_update(int unit)
{
    uint32 res = SOC_SAND_OK;
    SOC_SAND_TABLE_BLOCK_RANGE                      block_range;
    uint32 nof_entries, indx, lif_indx, lif, range0_min, range0_max, range1_min, range1_max;
    uint32 tbl_data[1];
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE          rule;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE_clear(&rule);

    /* Get ranges of LIFs for count */
    range0_min = SOC_PPC_FRWRD_MACT_LEARN_LIF_RANGE_BASE(0);
    range0_max = range0_min + ARAD_PP_FRWRD_MACT_LIMIT_RANGE_MAP_SIZE;
    range1_min = SOC_PPC_FRWRD_MACT_LEARN_LIF_RANGE_BASE(1);
    range1_max = range1_min + ARAD_PP_FRWRD_MACT_LIMIT_RANGE_MAP_SIZE;

    sal_memset(lem_fid_lif_counters, 0, sizeof(uint16)*JER_PP_EM_SER_LEM_NOF_FIDS);

    /*
     * Count entries per LIF via flush machine
     */
    SOC_SAND_TBL_ITER_SET_BEGIN(&(block_range.iter));
    block_range.entries_to_act = JER_PP_EM_SER_LEM_TRVRS_ITER_BLK_SIZE;
    block_range.entries_to_scan = SOC_SAND_TBL_ITER_SCAN_ALL;

    rule.key_type = SOC_PPC_FRWRD_MACT_KEY_TYPE_MAC_ADDR;

    while(!SOC_SAND_TBL_ITER_IS_END(&(block_range.iter))) {
        /* activate traverse for MACT */
        res = soc_ppd_frwrd_mact_get_block(unit,
                                           &rule,
                                           SOC_PPC_FRWRD_MACT_TABLE_SW_HW,
                                           &block_range,
                                           l2_traverse_mact_keys,
                                           l2_traverse_mact_vals,
                                           &nof_entries);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        if(nof_entries == 0) {
            break;
        }
        for (indx = 0; indx < nof_entries; ++indx) {
            lif_indx = JER_PP_EM_SER_LEM_NOF_FIDS;
            if (l2_traverse_mact_vals[indx].frwrd_info.forward_decision.additional_info.outlif.type != SOC_PPC_OUTLIF_ENCODE_TYPE_NONE) {
                /* destination is out-LIF */
                lif = l2_traverse_mact_vals[indx].frwrd_info.forward_decision.additional_info.outlif.val;

                /* Get counter index for LIF according to range */
                if ((lif >= range0_min) && (lif < range0_max)) {
                    lif_indx = lif;
                }
                else if ((lif >= range1_min) && (lif < range1_max)) {
                    lif_indx = lif + ARAD_PP_FRWRD_MACT_LIMIT_RANGE_MAP_SIZE;
                }
            }
            /* update counter if LIF is in counting range */
            if (lif_indx < JER_PP_EM_SER_LEM_NOF_FIDS) {
                lem_fid_lif_counters[lif_indx]++;
            }
        }
    }

    /* Write all LIF counters */
    for (indx = 0; indx < JER_PP_EM_SER_LEM_NOF_FIDS; ++indx) {
        tbl_data[0] = lem_fid_lif_counters[indx];
        res = soc_mem_write(unit,
                            PPDB_B_LARGE_EM_FID_COUNTER_DBm,
                            MEM_BLOCK_ANY,
                            indx,
                            tbl_data);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_fix_lem_fid_lif_counters(int unit, JER_PP_EM_SER_TYPE em_ser_type)
{
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (SOC_PPC_FRWRD_MACT_LEARN_LIMIT_MODE == SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE_VSI) {
        res = jer_pp_em_ser_lem_fid_counters_update(unit);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }
    else if (SOC_PPC_FRWRD_MACT_LEARN_LIMIT_MODE == SOC_PPC_FRWRD_MACT_LEARN_LIMIT_TYPE_LIF) {
        res = jer_pp_em_ser_lem_lif_counters_update(unit);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);

}

uint32 jer_pp_em_ser_lem_fix_counters(int unit, JER_PP_EM_SER_TYPE em_ser_type)
{
    uint32 res = SOC_SAND_OK;
    int lem_counter_diff = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (fix_global_counters) {
        /* check global counters mismatch */
        res = jer_pp_em_ser_lem_counter_diff_get(unit, &lem_counter_diff);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
        /* Fix MACT counter */
        res = jer_pp_em_ser_mact_counter_fix(unit);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }

    if (fix_fid_counters) {
        /* Fix FID/LIF counters */
        res = jer_pp_em_ser_fix_lem_fid_lif_counters(unit, em_ser_type);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    }

    /* Enable the management unit */
    res = jer_pp_em_ser_management_enable_set(unit, JER_PP_EM_SER_DB_TYPE_LEM, 1);
    SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);

    /* Fix the global LEM counter. Can be executed only after management is enabled. */
    if (lem_counter_diff > 0) {
        res = jer_pp_em_lem_counter_increment(unit, lem_counter_diff);
        SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);
    }
    else if (lem_counter_diff < 0) {
        res = jer_pp_em_lem_counter_decrement(unit, lem_counter_diff*(-1));
        SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_em_fix_counters(int unit, JER_PP_EM_SER_TYPE em_ser_type, JER_PP_SER_EM_TYPE_INFO* ser_info)
{
    uint32 res = SOC_SAND_OK;
    int em_counter_diff = 0;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    if (fix_global_counters) {
        /* check global counters mismatch */
        res = jer_pp_em_ser_em_counter_diff_get(unit, ser_info, &em_counter_diff);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    }

    /* Enable the management unit */
    res = jer_pp_em_ser_management_enable_set(unit, ser_info->db_type, 1);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* Fix the global EM counter. Can be executed only after management is enabled. */
    if (em_counter_diff > 0) {
        res = jer_pp_em_em_counter_increment(unit, ser_info, em_counter_diff);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    }
    else if (em_counter_diff < 0) {
        res = jer_pp_em_em_counter_decrement(unit, ser_info, em_counter_diff*(-1));
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_align_shadow_and_hw_lem(int unit)
{
    uint32 res = SOC_SAND_OK;
    uint32 key_out[SOC_DPP_DEFS_MAX_LEM_WIDTH_IN_UINT32S],
           data_out[ARAD_PP_LEM_ACCESS_PAYLOAD_IN_UINT32S];
    uint8  is_valid, is_found;
    uint32 entry_index, indx, nof_entries;
    uint64 fld_val64, reg_val1;
    ARAD_PP_LEM_ACCESS_PAYLOAD payload;
    ARAD_PP_LEM_ACCESS_KEY     key;
    ARAD_PP_LEM_ACCESS_REQUEST request;
    ARAD_PP_LEM_ACCESS_KEY_ENCODED key_in_buffer;
    ARAD_PP_LEM_ACCESS_ACK_STATUS ack;
    SOC_SAND_TABLE_BLOCK_RANGE                      block_range;
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE          rule;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    ARAD_PP_LEM_ACCESS_PAYLOAD_clear(&payload);
    ARAD_PP_LEM_ACCESS_KEY_clear(&key);
    ARAD_PP_LEM_ACCESS_REQUEST_clear(&request);
    ARAD_PP_LEM_ACCESS_ACK_STATUS_clear(&ack);
    ARAD_PP_LEM_ACCESS_KEY_ENCODED_clear(&key_in_buffer);
    soc_sand_SAND_TABLE_BLOCK_RANGE_clear(&block_range);
    SOC_PPC_FRWRD_MACT_TRAVERSE_MATCH_RULE_clear(&rule);

    /*
     * Traverse shadow LEM and write missing entries to HW LEM
     */
    for(entry_index = 0; entry_index < JER_CHIP_SIM_LEM_TABLE_SIZE; entry_index++) {
        soc_sand_os_memset(key_out, 0x0, JER_CHIP_SIM_LEM_KEY) ;
        soc_sand_os_memset(data_out, 0x0, JER_CHIP_SIM_LEM_PAYLOAD) ;

        res = soc_sand_exact_match_entry_get_by_index_unsafe(
              unit,
              JER_CHIP_SIM_LEM_BASE,
              entry_index,
              key_out,
              JER_CHIP_SIM_LEM_KEY,
              data_out,
              JER_CHIP_SIM_LEM_PAYLOAD,
              &is_valid
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if (is_valid) {
            /* Try to get the same entry from HW */
            is_found = 0;
            SOC_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_PPDB_B_LARGE_EM_DIAGNOSTICS_KEYr(unit, key_out));

            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, PPDB_B_LARGE_EM_DIAGNOSTICSr, REG_PORT_ANY, 0, LARGE_EM_DIAGNOSTICS_LOOKUPf, 1));

            /* Poll on the trigger bit before getting the result */
                res = arad_polling(
                      unit,
                      ARAD_TIMEOUT,
                      ARAD_MIN_POLLS,
                      PPDB_B_LARGE_EM_DIAGNOSTICSr,
                      REG_PORT_ANY,
                      0,
                      LARGE_EM_DIAGNOSTICS_LOOKUPf,
                      0
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
            /* Get the lookup result */
            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, ARAD_REG_ACCESS_ERR,READ_PPDB_B_LARGE_EM_DIAGNOSTICS_LOOKUP_RESULTr(unit, &reg_val1));

            ARAD_FLD_FROM_REG64(PPDB_B_LARGE_EM_DIAGNOSTICS_LOOKUP_RESULTr, LARGE_EM_ENTRY_FOUNDf, fld_val64, reg_val1, 50, exit);
            is_found = SOC_SAND_NUM2BOOL(COMPILER_64_LO(fld_val64));

            if ((is_found) == FALSE) {
                /* Write missing entry to HW via management */
                request.command = ARAD_PP_LEM_ACCESS_CMD_INSERT;
                key_in_buffer.buffer[0] = key_out[0];
                key_in_buffer.buffer[1] = key_out[1];
                key_in_buffer.buffer[2] = key_out[2];

                res = arad_pp_lem_key_encoded_parse(
                  unit,
                  &key_in_buffer,
                  &(request.key)
                  );
                SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

                res = arad_pp_lem_access_payload_parse(unit,
                                                       data_out,
                                                       request.key.type,
                                                       &payload);
                SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

                res = arad_pp_lem_access_entry_add_unsafe(
                        unit,
                        &request,
                        &payload,
                        &ack);
                SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);
            }
        }
    }

    /*
     * Traverse HW LEM and remove entries that are missing from shadow LEM
     */

    SOC_SAND_TBL_ITER_SET_BEGIN(&(block_range.iter));
    block_range.entries_to_act = JER_PP_EM_SER_LEM_TRVRS_ITER_BLK_SIZE;
    block_range.entries_to_scan = SOC_SAND_TBL_ITER_SCAN_ALL;

    rule.key_type = SOC_PPC_FRWRD_MACT_KEY_TYPE_MAC_ADDR;

    while(!SOC_SAND_TBL_ITER_IS_END(&(block_range.iter))) {
        /* activate traverse for MACT */
        res = soc_ppd_frwrd_mact_get_block(unit,
                                           &rule,
                                           SOC_PPC_FRWRD_MACT_TABLE_SW_HW,
                                           &block_range,
                                           l2_traverse_mact_keys,
                                           l2_traverse_mact_vals,
                                           &nof_entries);
        SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
        if(nof_entries == 0) {
            break;
        }
        for (indx = 0; indx < nof_entries; ++indx) {

            res = arad_pp_frwrd_mact_key_convert(
                  unit,
                  &l2_traverse_mact_keys[indx],
                  &key
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

            res = arad_pp_lem_key_encoded_build(
                   unit,
                   &(key),
                   0,
                   &key_in_buffer
            );

            /* Get entry from shadow LEM */
            is_found = 0;
            res = chip_sim_exact_match_entry_get_unsafe(
                  unit,
                  JER_CHIP_SIM_LEM_BASE,
                  key_in_buffer.buffer,
                  JER_CHIP_SIM_LEM_KEY,
                  data_out,
                  JER_CHIP_SIM_LEM_PAYLOAD,
                  &is_found
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);

            if ((is_found) == FALSE) {
                /* Remove missing entry from HW via management */

                request.command = ARAD_PP_LEM_ACCESS_CMD_DELETE;

                res = arad_pp_lem_key_encoded_parse(
                  unit,
                  &key_in_buffer,
                  &(request.key)
                  );
                SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);

                res = arad_pp_lem_access_entry_remove_unsafe(
                      unit,
                      &request,
                      &ack);
                SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);
            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

uint32 jer_pp_em_ser_align_shadow_and_hw_em(int unit, JER_PP_SER_EM_TYPE_INFO* ser_info)
{
    uint32 res = SOC_SAND_OK;
    uint8  is_valid, is_found;
    uint32 entry_index, temp, entry_is_valid;
    soc_reg_above_64_val_t reg_val, fld_val, read_result, key_data, key_out, data_out, buffer;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(buffer);
    SOC_REG_ABOVE_64_CLEAR(reg_val);
    SOC_REG_ABOVE_64_CLEAR(fld_val);
    SOC_REG_ABOVE_64_CLEAR(read_result);
    SOC_REG_ABOVE_64_CLEAR(key_data);
    SOC_REG_ABOVE_64_CLEAR(key_out);
    SOC_REG_ABOVE_64_CLEAR(data_out);
    /*
     * Traverse shadow EM and write missing entries to HW EM
     */
    for(entry_index = 0; entry_index < ser_info->db_nof_lines; entry_index++) {

        res = soc_sand_exact_match_entry_get_by_index_unsafe(
              unit,
              ser_info->shadow_base,
              entry_index,
              key_out,
              ser_info->shadow_key_size,
              data_out,
              ser_info->shadow_payload_size,
              &is_valid
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        if (is_valid) {
            /* Try to get the same entry from HW */
            is_found = 0;
            res = soc_reg_above_64_set(unit, ser_info->diagnostics_key_reg, REG_PORT_ANY, 0, key_out);
            SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

            SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ser_info->diagnostics_reg, REG_PORT_ANY, 0, ser_info->diagnostics_lookup_field, 1));

            /* Poll on the trigger bit before getting the result */
                res = arad_polling(
                      unit,
                      ARAD_TIMEOUT,
                      ARAD_MIN_POLLS,
                      ser_info->diagnostics_reg,
                      REG_PORT_ANY,
                      0,
                      ser_info->diagnostics_lookup_field,
                      0
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
            /* Get the lookup result */
            res= soc_reg_above_64_get(unit, ser_info->diagnostics_lookup_result_reg, REG_PORT_ANY, 0, reg_val);
            SOC_SAND_CHECK_FUNC_RESULT(res, 50, exit);

            /* is_found is always bit 0 in lookup result register. */
            is_found = reg_val[0] & 1;

            if ((is_found) == FALSE) {
                /* Write missing entry to HW via management */
                /* Set type field */
                temp = JER_PP_EM_SER_REQUEST_TYPE_INSERT_VAL;
                res = soc_sand_bitstream_set_any_field(
                      &temp,
                      JER_PP_EM_SER_REQUEST_TYPE_START_BIT,
                      JER_PP_EM_SER_REQUEST_TYPE_SIZE,
                      buffer
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 60, exit);

                /* Set key field */
                res = soc_sand_bitstream_set_any_field(
                      key_out,
                      ser_info->key_start_bit,
                      ser_info->key_size,
                      buffer
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 70, exit);

                /* Set payload field */
                res = soc_sand_bitstream_set_any_field(
                      data_out,
                      ser_info->payload_start_bit,
                      ser_info->payload_size,
                      buffer
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 80, exit);

                /* Write the insertion request */
                res = soc_mem_write(unit, ser_info->req_mem, MEM_BLOCK_ALL, 0, buffer);
                SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);
            }
        }
    }

    /*
     * Traverse HW EM and remove entries that are missing from shadow EM
     */

    for (entry_index = 0; entry_index < ser_info->db_nof_lines; entry_index++) {
        /* Get entry from HW */
        res = soc_reg32_set(unit, ser_info->diagnostics_index_reg, REG_PORT_ANY, 0, entry_index);
        SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);

        SOC_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  110,  exit, ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ser_info->diagnostics_reg, REG_PORT_ANY, 0, ser_info->diagnostics_read_field, 1));

        /* Poll on the trigger bit before getting the result */
        res = arad_polling(
              unit,
              ARAD_TIMEOUT,
              ARAD_MIN_POLLS,
              ser_info->diagnostics_reg,
              REG_PORT_ANY,
              0,
              ser_info->diagnostics_read_field,
              0
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 120, exit);
        res = soc_reg_above_64_get(unit, ser_info->diagnostics_read_result_reg, REG_PORT_ANY, 0, read_result);
        SOC_SAND_CHECK_FUNC_RESULT(res, 130, exit);

        /* Get the key data */
        res = soc_sand_bitstream_get_any_field(
              read_result,
              0, /* start bit */
              1, /* nof bits */
              &entry_is_valid
            );
        SOC_SAND_CHECK_FUNC_RESULT(res, 140, exit);

        if (entry_is_valid) {
            /* Get the key data */
            res = soc_sand_bitstream_get_any_field(
                  read_result,
                  1, /* start bit */
                  ser_info->key_size, /* nof bits */
                  key_data
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 150, exit);

            /* Get entry from shadow LEM */
            is_found = 0;
            res = chip_sim_exact_match_entry_get_unsafe(
                  unit,
                  ser_info->shadow_base,
                  key_data,
                  ser_info->shadow_key_size,
                  data_out,
                  ser_info->shadow_payload_size,
                  &is_found
                );
            SOC_SAND_CHECK_FUNC_RESULT(res, 160, exit);

            if ((is_found) == FALSE) {
                /* Remove missing entry from HW via management */

                /* Set type field */
                temp = JER_PP_EM_SER_REQUEST_TYPE_DELETE_VAL;
                res = soc_sand_bitstream_set_any_field(
                      &temp,
                      JER_PP_EM_SER_REQUEST_TYPE_START_BIT,
                      JER_PP_EM_SER_REQUEST_TYPE_SIZE,
                      buffer
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 170, exit);

                /* Set key field */
                res = soc_sand_bitstream_set_any_field(
                      key_data,
                      ser_info->key_start_bit,
                      ser_info->key_size,
                      buffer
                    );
                SOC_SAND_CHECK_FUNC_RESULT(res, 180, exit);

                /* Write the insertion request */
                res = soc_mem_write(unit, ser_info->req_mem, MEM_BLOCK_ALL, 0, buffer);
                SOC_SAND_CHECK_FUNC_RESULT(res, 190, exit);

            }
        }
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}


void jer_pp_em_ser_lem_recover(void *p1, void *p2, void *p3, void *p4, void *p5)
{
    uint32 key_pld_data[4] = {0};
    soc_reg_above_64_val_t reg_above_64_val;
    int unit = *(int*)p1;
    JER_PP_EM_SER_TYPE em_ser_type = JER_PP_EM_SER_TYPE_LEM_KEYT_PLDT;
    uint32 is_ser_exist = *(uint32*)p2;
    uint32 array_index = *(uint32*)p3;
    uint32 entry_offset = *(uint32*)p4;
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    SOC_REG_ABOVE_64_CLEAR(reg_above_64_val);

    if (!is_ser_exist) {
        /* fix counters */
        res = jer_pp_em_ser_lem_fix_counters(unit, em_ser_type);
        SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

        /* align shadow and HW static entries */
        res = jer_pp_em_ser_align_shadow_and_hw_lem(unit);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
    else {


        if (em_ser_type == JER_PP_EM_SER_TYPE_LEM_KEYT_PLDT) {
            /* Delete entry directly by setting the key to all zeros (including is-valid bit) */
            res = soc_mem_array_write(unit,
                                      KEYT_PLDT_MEM(PPDB_B, LARGE_EM),
                                      array_index,
                                      MEM_BLOCK_ANY,
                                      entry_offset,
                                      key_pld_data);
            SOC_SAND_CHECK_FUNC_RESULT(res, 90, exit);

            /* fix counters */
            res = jer_pp_em_ser_lem_fix_counters(unit, em_ser_type);
            SOC_SAND_CHECK_FUNC_RESULT(res, 100, exit);


            /* align shadow and HW static entries */
            res = jer_pp_em_ser_align_shadow_and_hw_lem(unit);
            SOC_SAND_CHECK_FUNC_RESULT(res, 110, exit);
        }
    }

exit:
    /* Unmask LEM interrupts */
    jer_pp_em_ser_mask_interrupt_set(unit, em_ser_type, 1);
}

void jer_pp_em_ser_em_recover(void *p1, void *p2, void *p3, void *p4, void *p5)
{
    int unit = *(int*)p1;
    JER_PP_EM_SER_TYPE em_ser_type = *(JER_PP_EM_SER_TYPE*)p2;
    uint32 array_index = *(uint32*)p3;
    uint32 entry_offset = *(uint32*)p4;
    soc_reg_above_64_val_t key_pld_data;
    JER_PP_SER_EM_TYPE_INFO ser_info;
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);


    SOC_REG_ABOVE_64_CLEAR(key_pld_data);
    sal_memset(&ser_info, 0, sizeof(JER_PP_SER_EM_TYPE_INFO));

    res = jer_pp_ser_em_type_info_get(unit, em_ser_type, &ser_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);


    if ((em_ser_type == JER_PP_EM_SER_TYPE_ISEM_KEYT_PLDT) ||
        (em_ser_type == JER_PP_EM_SER_TYPE_GLEM_KEYT_PLDT) ||
        (em_ser_type == JER_PP_EM_SER_TYPE_ESEM_KEYT_PLDT) ||
        (em_ser_type == JER_PP_EM_SER_TYPE_OEMA_KEYT_PLDT) ||
        (em_ser_type == JER_PP_EM_SER_TYPE_OEMB_KEYT_PLDT) ||
        (em_ser_type == JER_PP_EM_SER_TYPE_RMAPEM_KEYT_PLDT))
    {
        /* Delete entry directly by setting the key to all zeros (including is-valid bit) */
        res = soc_mem_array_write(unit,
                                  ser_info.keyt_pldt_mem,
                                  array_index,
                                  MEM_BLOCK_ANY,
                                  entry_offset,
                                  key_pld_data);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

        /* Fix counters */
        res = jer_pp_em_ser_em_fix_counters(unit, em_ser_type, &ser_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);

        /* Align shadow and HW entries */
        res = jer_pp_em_ser_align_shadow_and_hw_em(unit, &ser_info);
        SOC_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }
exit:
    /* Unmask EM interrupts */
    jer_pp_em_ser_mask_interrupt_set(unit, em_ser_type, 1);
}
/*
 * Handle corruption in LEM
 */
int jer_pp_em_ser_lem(int unit, soc_mem_t mem, uint32 array_index, uint32 entry_offset, JER_PP_EM_SER_TYPE em_ser_type)
{
    uint32 counter, counter_overflow, address_valid, address, addr;
    uint32 data[4];
    uint32 res = SOC_SAND_OK;
    uint32 is_ser_exist;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    /* Disable the management unit */
    res = jer_pp_em_ser_management_enable_set(unit, JER_PP_EM_SER_DB_TYPE_LEM, 0);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);
    addr = JER_PP_EM_SER_LEM_KEYT_BASE_ADDR + (JER_PP_EM_SER_LEM_NOF_LINES/JER_PP_EM_SER_NOF_HASH6_RESULTS) * array_index + entry_offset;
    res = soc_mem_array_read(unit,
                             mem,
                             array_index,
                             MEM_BLOCK_ANY,
                             entry_offset,
                             data);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* Read SER indication */
	res = jer_pp_em_ser_ecc_error_info_get(unit, JER_PP_EM_SER_BLOCK_PPDB_B, &address, &address_valid, &counter, &counter_overflow);
 	SOC_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

    if ((counter > 0) && (address == addr)) {
        /* SER indication still exists */
        is_ser_exist = 1;
    }
    else {
        /* SER indication doesn't exist. Corrupted entry moved.  */
        is_ser_exist = 0;
    }

    /* Set global variables used for DPC*/
    p1 = unit;
    p2 = is_ser_exist;
    p3 = array_index;
    p4 = entry_offset;

    /*
     * LEM recovery process may be very long. Proceed process in another context,
     * using the deferred procedure call mechanism.
     */
    sal_dpc(jer_pp_em_ser_lem_recover, (void*)&p1, (void*)&p2, (void*)&p3, (void*)&p4, NULL);

    /*
     * Mask LEM interrupts
     */
    jer_pp_em_ser_mask_interrupt_set(unit, em_ser_type, 0);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

/*
 * Handle corruption in non-LEM
 */
int jer_pp_em_ser_em(int unit, soc_mem_t mem, uint32 array_index, uint32 entry_offset, JER_PP_EM_SER_TYPE em_ser_type)
{
    JER_PP_SER_EM_TYPE_INFO ser_info;
    uint32 res = SOC_SAND_OK;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = jer_pp_ser_em_type_info_get(unit, em_ser_type, &ser_info);
    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    /* Disable the management unit */
    res = jer_pp_em_ser_management_enable_set(unit, ser_info.db_type, 0);
    SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);

    /* Set global variables used for DPC*/
    p1 = unit;
    p2 = em_ser_type;
    p3 = array_index;
    p4 = entry_offset;

    /*
     * EM recovery process may be very long. Proceed process in another context,
     * using the deferred procedure call mechanism.
     */
    sal_dpc(jer_pp_em_ser_em_recover, (void*)&p1, (void*)&p2, (void*)&p3, (void*)&p4, NULL);

    /*
     * Mask EM interrupts
     */
    jer_pp_em_ser_mask_interrupt_set(unit, em_ser_type, 0);

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR("error in jer_pp_em_ser_em()", 0, 0);
}

/*
 * Handle SER indication in exact match DB
 */

int dcmn_pp_em_ser(int unit, soc_mem_t mem, unsigned array_index, int copyno, int index)
{
    uint32 res = SOC_SAND_OK;
    JER_PP_EM_SER_TYPE em_ser_type;

    SOC_SAND_INIT_ERROR_DEFINITIONS(0);

    res = jer_pp_em_ser_type_get(unit, mem, &em_ser_type);

    SOC_SAND_CHECK_FUNC_RESULT(res, 10, exit);

    if (em_ser_type == JER_PP_EM_SER_TYPE_LEM_KEYT_PLDT) {
        /* Corruption in LEM */
        res = jer_pp_em_ser_lem(unit, mem, array_index, index, em_ser_type);
        SOC_SAND_CHECK_FUNC_RESULT(res, 20, exit);
    }
    else if (em_ser_type != JER_PP_EM_SER_TYPE_LAST) {
        /* Corruption in a non-LEM database */
        res = jer_pp_em_ser_em(unit, mem, array_index, index, em_ser_type);
        SOC_SAND_CHECK_FUNC_RESULT(res, 30, exit);
    }

exit:
    SOC_SAND_EXIT_AND_SEND_ERROR(0, 0, 0);
}

int
dcmn_interrupt_handles_corrective_action_for_em_ecc_1b(
    int unit,
    int block_instance,
    uint32 interrupt_id,
    dcmn_interrupt_mem_err_info* ecc_1b_correct_info_p,
    char* msg)
{
    int rc;
    JER_PP_EM_SER_TYPE em_ser_type;
    JER_PP_SER_EM_TYPE_INFO ser_info;
    soc_mem_t mem;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(ecc_1b_correct_info_p);

    mem = ecc_1b_correct_info_p->mem;
    rc = jer_pp_em_ser_type_get(unit, mem, &em_ser_type);
    SOCDNX_IF_ERR_EXIT(rc);

    rc = jer_pp_ser_em_type_info_get(unit, em_ser_type, &ser_info);
    SOCDNX_IF_ERR_EXIT(rc);

    /* Disable the management unit */
    rc = jer_pp_em_ser_management_enable_set(unit, ser_info.db_type, 0);
    SOCDNX_IF_ERR_EXIT(rc);

    if (SOC_MEM_TYPE(unit,mem) == SOC_MEM_TYPE_XOR) {
        rc = dcmn_interrupt_handles_corrective_action_for_xor(unit, block_instance, interrupt_id, ecc_1b_correct_info_p, msg);
        SOCDNX_IF_ERR_EXIT(rc);

    } else {
        rc = dcmn_interrupt_handles_corrective_action_for_ecc_1b(unit, block_instance, interrupt_id, ecc_1b_correct_info_p, msg);
        SOCDNX_IF_ERR_EXIT(rc);
    }

    /* Enable the management unit */
    rc = jer_pp_em_ser_management_enable_set(unit, ser_info.db_type, 1);
    SOCDNX_IF_ERR_EXIT(rc);


exit:
    SOCDNX_FUNC_RETURN;
}
#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif

