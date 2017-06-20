#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
/* $Id: jer2_arad_flow_control.c,v 1.59 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FLOWCONTROL

/*************
 * INCLUDES  *
 *************/
#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>
/* { */

#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>
#include <soc/dnx/legacy/SAND/Utils/sand_os_interface.h>
#include <soc/dnx/legacy/ARAD/arad_api_framework.h>
#include <soc/dnx/legacy/ARAD/arad_flow_control.h>
#include <soc/dnx/legacy/JER/jer_flow_control.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/TMC/tmc_api_egr_queuing.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/portmod/portmod.h>
#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
#include <soc/mcm/allenum.h>


/* } */
/*************
 * DEFINES   *
 *************/
/* { */
#define JER2_JER_FC_REC_CAL_DEST_ARR_SIZE   7
#define JER2_JER_FC_GEN_CAL_SRC_ARR_SIZE    8


#define NUM_OF_LANES_IN_PM                4


#define GLB_RSC_MAP_TBL_NIF_PFC_OFFSET    0
#define GLB_RSC_MAP_TBL_RCY_PFC_OFFSET    8
#define GLB_RSC_MAP_TBL_HCFC_OFFSET       16

/* (2^10 - 1) */
#define JER2_JER_FC_RX_MLF_MAX           ((1 << soc_reg_field_length(unit, \
                                                                 SOC_IS_QUX(unit) ? NIF_RX_MLF_LLFC_THRESHOLDS_CONFIGr : \
                                                                 NBIH_RX_MLF_LLFC_THRESHOLDS_CONFIGr, RX_LLFC_THRESHOLD_SET_QMLF_Nf)) - 1)
/* (2^11 - 1) */
#define JER2_JER_FC_RX_MLF_HRF_MAX       ((1 << soc_reg_field_length(unit, NBIH_HRF_RX_CONFIG_HRFr, HRF_RX_LLFC_THRESHOLD_SET_HRF_Nf)) - 1)

/* JER2_JER: (2^4  - 1) ,  JER2_QAX (2^3  - 1) */
#define NIF_PFC_STATUS_SEL_MAX       ((1 << soc_reg_field_length(unit, CFC_NIF_PFC_STATUS_SELr, NIF_PFC_STATUS_SELf)) - 1)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

#define SET_FC_ENABLE(__FC_ENA, __FIELD) {                                            \
    if (SHR_BITGET(cfc_enables->bmp, __FC_ENA)) {                                     \
        if (soc_reg_field_valid(unit, reg, __FIELD)) {                                \
            field_value = (SHR_BITGET(ena_info->bmp, __FC_ENA))? 1: 0;                \
            soc_reg_above_64_field32_set(unit, reg, reg_data, __FIELD, field_value);  \
        }                                                                             \
    }                                                                                 \
}

#define GET_FC_ENABLE(__FC_ENA, __FIELD) {                                            \
    if (SHR_BITGET(cfc_enables->bmp, __FC_ENA)) {                                     \
        if (soc_reg_field_valid(unit, reg, __FIELD)) {                                \
            field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, __FIELD); \
            if (field_value) {                                                        \
                SHR_BITSET(ena_info->bmp, __FC_ENA);                                  \
            }                                                                         \
        }                                                                             \
        else {                                                                        \
            SHR_BITCLR(ena_info->bmp, __FC_ENA);                                      \
        }                                                                             \
    }                                                                                 \
}

#define FC_ENABLE_NOT_SUPPORT_JER2_JER {     \
    DNX_TMC_FC_ILKN_RX_TO_RET_REQ_EN,   \
    DNX_TMC_FC_SPI_OOB_RX_TO_RET_REQ_EN \
}

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

static DNX_TMC_FC_GEN_CAL_SRC 
  jer2_jer_fc_gen_cal_src_arr[JER2_JER_FC_GEN_CAL_SRC_ARR_SIZE] = 
    { DNX_TMC_FC_GEN_CAL_SRC_STE, 
      DNX_TMC_FC_GEN_CAL_SRC_STTSTCS_TAG,
      DNX_TMC_FC_GEN_CAL_SRC_LLFC_VSQ,
      DNX_TMC_FC_GEN_CAL_SRC_PFC_VSQ,
      DNX_TMC_FC_GEN_CAL_SRC_GLB_RCS,
      DNX_TMC_FC_GEN_CAL_SRC_HCFC,
      DNX_TMC_FC_GEN_CAL_SRC_LLFC,
      DNX_TMC_FC_GEN_CAL_SRC_CONST
    };

static DNX_TMC_FC_REC_CAL_DEST 
  jer2_jer_fc_rec_cal_dest_arr[JER2_JER_FC_REC_CAL_DEST_ARR_SIZE] = 
    { DNX_TMC_FC_REC_CAL_DEST_PFC, 
      DNX_TMC_FC_REC_CAL_DEST_NIF_LL,
      DNX_TMC_FC_REC_CAL_DEST_PORT_2_PRIORITY,
      DNX_TMC_FC_REC_CAL_DEST_PORT_8_PRIORITY,
      DNX_TMC_FC_REC_CAL_DEST_GENERIC_PFC,
      DNX_TMC_FC_REC_CAL_DEST_RETRANSMIT,
      DNX_TMC_FC_REC_CAL_DEST_NONE
    };

static soc_field_t ilkn_inb_tx_ena_flds[] = {
        ILKN_INB_TX_0_ENf, ILKN_INB_TX_1_ENf, ILKN_INB_TX_2_ENf,
        ILKN_INB_TX_3_ENf, ILKN_INB_TX_4_ENf, ILKN_INB_TX_5_ENf};
static soc_field_t ilkn_inb_tx_cal_len_flds[] = {
        ILKN_INB_TX_0_CAL_LENf, ILKN_INB_TX_1_CAL_LENf, ILKN_INB_TX_2_CAL_LENf,
        ILKN_INB_TX_3_CAL_LENf, ILKN_INB_TX_4_CAL_LENf, ILKN_INB_TX_5_CAL_LENf};
static uint32 calendar_regs[] = {
        CFC_ILKN_0_MUB_TX_CALr, CFC_ILKN_1_MUB_TX_CALr, CFC_ILKN_2_MUB_TX_CALr, 
        CFC_ILKN_3_MUB_TX_CALr, CFC_ILKN_4_MUB_TX_CALr, CFC_ILKN_5_MUB_TX_CALr};
static uint32
        fields_0[] = {
        ILKN_0_MUB_TX_MAP_0f, ILKN_0_MUB_TX_MAP_1f, ILKN_0_MUB_TX_MAP_2f, ILKN_0_MUB_TX_MAP_3f,
        ILKN_0_MUB_TX_MAP_4f, ILKN_0_MUB_TX_MAP_5f, ILKN_0_MUB_TX_MAP_6f, ILKN_0_MUB_TX_MAP_7f};
static uint32
        fields_1[] = {
        ILKN_1_MUB_TX_MAP_0f, ILKN_1_MUB_TX_MAP_1f, ILKN_1_MUB_TX_MAP_2f, ILKN_1_MUB_TX_MAP_3f,
        ILKN_1_MUB_TX_MAP_4f, ILKN_1_MUB_TX_MAP_5f, ILKN_1_MUB_TX_MAP_6f, ILKN_1_MUB_TX_MAP_7f};
static uint32
        fields_2[] = {
        ILKN_2_MUB_TX_MAP_0f, ILKN_2_MUB_TX_MAP_1f, ILKN_2_MUB_TX_MAP_2f, ILKN_2_MUB_TX_MAP_3f,
        ILKN_2_MUB_TX_MAP_4f, ILKN_2_MUB_TX_MAP_5f, ILKN_2_MUB_TX_MAP_6f, ILKN_2_MUB_TX_MAP_7f};
static uint32
        fields_3[] = {
        ILKN_3_MUB_TX_MAP_0f, ILKN_3_MUB_TX_MAP_1f, ILKN_3_MUB_TX_MAP_2f, ILKN_3_MUB_TX_MAP_3f,
        ILKN_3_MUB_TX_MAP_4f, ILKN_3_MUB_TX_MAP_5f, ILKN_3_MUB_TX_MAP_6f, ILKN_3_MUB_TX_MAP_7f};
static uint32
        fields_4[] = {
        ILKN_4_MUB_TX_MAP_0f, ILKN_4_MUB_TX_MAP_1f, ILKN_4_MUB_TX_MAP_2f, ILKN_4_MUB_TX_MAP_3f,
        ILKN_4_MUB_TX_MAP_4f, ILKN_4_MUB_TX_MAP_5f, ILKN_4_MUB_TX_MAP_6f, ILKN_4_MUB_TX_MAP_7f};
static uint32
        fields_5[] = {
        ILKN_5_MUB_TX_MAP_0f, ILKN_5_MUB_TX_MAP_1f, ILKN_5_MUB_TX_MAP_2f, ILKN_5_MUB_TX_MAP_3f,
        ILKN_5_MUB_TX_MAP_4f, ILKN_5_MUB_TX_MAP_5f, ILKN_5_MUB_TX_MAP_6f, ILKN_5_MUB_TX_MAP_7f};
static soc_field_t rx_flds[] = {
        ILKN_0_MUB_RX_ENAf,
        ILKN_1_MUB_RX_ENAf, 
        ILKN_2_MUB_RX_ENAf,
        ILKN_3_MUB_RX_ENAf,
        ILKN_4_MUB_RX_ENAf,
        ILKN_5_MUB_RX_ENAf};
static soc_field_t tx_flds[] = {
        ILKN_0_MUB_TX_ENAf,
        ILKN_1_MUB_TX_ENAf,
        ILKN_2_MUB_TX_ENAf,
        ILKN_3_MUB_TX_ENAf,
        ILKN_4_MUB_TX_ENAf,
        ILKN_5_MUB_TX_ENAf};
static soc_mem_t cal_inb_rx_tables[] = {
        CFC_ILKN_INB_RX_0_CALm, CFC_ILKN_INB_RX_1_CALm, CFC_ILKN_INB_RX_2_CALm,
        CFC_ILKN_INB_RX_3_CALm, CFC_ILKN_INB_RX_4_CALm, CFC_ILKN_INB_RX_5_CALm};
static soc_mem_t cal_inb_tx_tables[] = {
        CFC_ILKN_INB_TX_0_CALm, CFC_ILKN_INB_TX_1_CALm, CFC_ILKN_INB_TX_2_CALm,
        CFC_ILKN_INB_TX_3_CALm, CFC_ILKN_INB_TX_4_CALm, CFC_ILKN_INB_TX_5_CALm};
static uint32 index_field[] = {
        INDEX_0f, INDEX_1f, INDEX_2f, INDEX_3f, INDEX_4f, INDEX_5f, INDEX_6f, INDEX_7f};
static uint32 select_field[] = { 
        SELECT_0f, SELECT_1f, SELECT_2f, SELECT_3f, SELECT_4f, SELECT_5f, SELECT_6f, SELECT_7f};
static uint32 valid_field[] = {
        VALID_0f, VALID_1f, VALID_2f, VALID_3f, VALID_4f, VALID_5f, VALID_6f, VALID_7f};
static soc_field_t inb_sel_fld[] = {
        ILKN_INB_RX_0_SELf, ILKN_INB_RX_1_SELf, ILKN_INB_RX_2_SELf,
        ILKN_INB_RX_3_SELf, ILKN_INB_RX_4_SELf, ILKN_INB_RX_5_SELf};
static soc_field_t ilkn_inb_rx_ena_flds[] = {
        ILKN_INB_RX_0_ENf, ILKN_INB_RX_1_ENf, ILKN_INB_RX_2_ENf,
        ILKN_INB_RX_3_ENf, ILKN_INB_RX_4_ENf, ILKN_INB_RX_5_ENf};
static soc_field_t rx_mlf_pfc_fld[] = {
        FC_TX_N_MLF_0_GEN_PFC_FROM_QMLF_MASKf,
        FC_TX_N_MLF_1_GEN_PFC_FROM_QMLF_MASKf, 
        FC_TX_N_MLF_2_GEN_PFC_FROM_QMLF_MASKf, 
        FC_TX_N_MLF_3_GEN_PFC_FROM_QMLF_MASKf};
static soc_field_t ilkn_inb_rx_cal_len_flds[] = {
        FC_ILKN_RX_0_CHFC_CAL_LENf, FC_ILKN_RX_1_CHFC_CAL_LENf, FC_ILKN_RX_2_CHFC_CAL_LENf,
        FC_ILKN_RX_3_CHFC_CAL_LENf, FC_ILKN_RX_4_CHFC_CAL_LENf, FC_ILKN_RX_5_CHFC_CAL_LENf};
static soc_mem_t GENERIC_BITMAP_NIF_MAP[] = {
        CFC_GENERIC_BITMAP_NIF_0_MAPm,
        CFC_GENERIC_BITMAP_NIF_1_MAPm,
        CFC_GENERIC_BITMAP_NIF_2_MAPm,
        CFC_GENERIC_BITMAP_NIF_3_MAPm,
        CFC_GENERIC_BITMAP_NIF_4_MAPm,
        CFC_GENERIC_BITMAP_NIF_5_MAPm,
        CFC_GENERIC_BITMAP_NIF_6_MAPm,
        CFC_GENERIC_BITMAP_NIF_7_MAPm};  
/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */


static uint32
jer2_jer_fc_cal_tbl_get(
    DNX_SAND_IN int  unit,
    DNX_SAND_IN DNX_TMC_FC_CAL_MODE           cal_mode,
    DNX_SAND_IN JER2_ARAD_FC_CAL_TYPE              cal_type,
    DNX_SAND_IN DNX_TMC_FC_CAL_IF_ID          if_ndx,
    DNX_SAND_OUT soc_mem_t                    *cal_table
  )
{
    soc_mem_t
        cal_table_tmp = INVALIDm;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(cal_table);

    if (cal_mode == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        if (if_ndx > SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ilkn ndx is out of range")));
        }
    }
    else {
        if (if_ndx > SOC_DNX_CONFIG(unit)->tm.max_oob_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("oob ndx is out of range")));
        }        
    }

    if (cal_type == JER2_ARAD_FC_CAL_TYPE_RX){
        if(cal_mode == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            cal_table_tmp = cal_inb_rx_tables[if_ndx];
        }
        else {
            cal_table_tmp = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_OOB_RX_0_CALm : CFC_OOB_RX_1_CALm);
        }
    }
    else {
        if(cal_mode == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            cal_table_tmp = cal_inb_tx_tables[if_ndx];
        }
        else {
            cal_table_tmp = (if_ndx == DNX_TMC_FC_OOB_ID_A ? CFC_OOB_TX_0_CALm : CFC_OOB_TX_1_CALm);
        }
    }
	
	(*cal_table) = cal_table_tmp;

exit:
    DNXC_FUNC_RETURN;    
}


/*********************************************************************
* NAME:
*     jer2_jer_flow_control_regs_init
* FUNCTION:
*   Initialization of the Arad blocks configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
static uint32
  jer2_jer_flow_control_regs_init(
    DNX_SAND_IN  int                 unit
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    DNX_TMC_FC_ILKN_MUB_GEN_CAL
        ilkn_cal;
    uint32
        i = 0,
        ilkn_ndx = 0,
        oob_ndx = 0,
        interface_num;
    DNX_TMC_FC_ENABLE_BITMAP
        cfc_enables; 
    soc_dnx_tm_config_t  *tm;
    uint32
        fld_val = 0,
        fld = 0;
    uint32
        cal_length = 0;
    soc_reg_t
        fc_reset_reg;

    DNXC_INIT_FUNC_DEFS;

    tm = &(SOC_DNX_CONFIG(unit)->tm);

    /* CFC Enablers */
    SHR_BITCLR_RANGE(cfc_enables.bmp, 0, DNX_TMC_FC_NOF_ENABLEs);

    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_EGQ_TO_SCH_DEVICE_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_EGQ_TO_SCH_ERP_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_EGQ_TO_SCH_ERP_TC_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_EGQ_TO_SCH_IF_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_EGQ_TO_SCH_PFC_EN);

    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_EGQ_RCL_PFC_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_HCFC_HP_CFG);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_HCFC_LP_CFG);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_HP_CFG);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_LP_CFG);

    rv = jer2_jer_fc_enables_set(unit, &cfc_enables, &cfc_enables);
    DNXC_IF_ERR_EXIT(rv);

    if (!SOC_IS_QUX(unit)) {
    /* Clear Calendars  - ILKN */
    for (ilkn_ndx = 0; ilkn_ndx < SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports; ilkn_ndx++) {
        rv = jer2_jer_fc_clear_calendar_unsafe(unit, DNX_TMC_FC_CAL_MODE_ILKN_INBND, JER2_ARAD_FC_CAL_TYPE_RX, ilkn_ndx);
        DNXC_IF_ERR_EXIT(rv);
        rv = jer2_jer_fc_clear_calendar_unsafe(unit, DNX_TMC_FC_CAL_MODE_ILKN_INBND, JER2_ARAD_FC_CAL_TYPE_TX, ilkn_ndx);
        DNXC_IF_ERR_EXIT(rv);

        fld = ilkn_inb_tx_cal_len_flds[ilkn_ndx]; 
        cal_length = SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_length[ilkn_ndx][DNX_TMC_CONNECTION_DIRECTION_TX] * 
                        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_rep_count[ilkn_ndx][DNX_TMC_CONNECTION_DIRECTION_TX];
        rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld, cal_length-1);
        DNXC_IF_ERR_EXIT(rv);
    }
    }

    /* Clear Calendars  - SPI OOB */
    for (oob_ndx = 0; oob_ndx < SOC_DNX_CONFIG(unit)->tm.max_oob_ports; oob_ndx++) {
        rv = jer2_jer_fc_clear_calendar_unsafe(unit, DNX_TMC_FC_CAL_MODE_SPI_OOB, JER2_ARAD_FC_CAL_TYPE_RX, oob_ndx);
        DNXC_IF_ERR_EXIT(rv);
        rv = jer2_jer_fc_clear_calendar_unsafe(unit, DNX_TMC_FC_CAL_MODE_SPI_OOB, JER2_ARAD_FC_CAL_TYPE_TX, oob_ndx);
        DNXC_IF_ERR_EXIT(rv);
    }

    /*  Configure polarity for SPI FC indications (both rx and tx) */
    for(oob_ndx = 0; oob_ndx < SOC_DNX_CONFIG(unit)->tm.max_oob_ports; oob_ndx++)
    {
        if(tm->fc_oob_type[oob_ndx] == DNX_TMC_FC_CAL_TYPE_SPI || 
           tm->fc_oob_type[oob_ndx] == DNX_TMC_FC_CAL_TYPE_HCFC) {
            fld_val = (SOC_DNX_CONFIG(unit)->tm.fc_oob_spi_indication_invert[oob_ndx]) ? 1 : 0;
            fld = (oob_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_0_OOB_POLARITYf : SPI_1_OOB_POLARITYf);
            rv = soc_reg_field32_modify(unit, CFC_SPI_OOB_CONFIGURATIONr, REG_PORT_ANY, fld, fld_val);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    /* Init the ILKN MUB TX calendar */
    if (!SOC_IS_QUX(unit)) {
        for (ilkn_ndx = 0; ilkn_ndx < SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports; ilkn_ndx++) {
            for(i = 0; i < DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN; i++) {
                ilkn_cal.entries[i].source = DNX_TMC_FC_GEN_CAL_SRC_CONST;
                ilkn_cal.entries[i].id = 0;
            }
            rv = jer2_jer_fc_ilkn_mub_gen_cal_set(unit, ilkn_ndx, &ilkn_cal);
            DNXC_IF_ERR_EXIT(rv);
        }
    }
    /* Configure the ILKN FC swap bit */
    for (interface_num = 0; interface_num < DNX_TMC_FC_NOF_OOB_IDS; interface_num++) {
        if((tm->fc_oob_type[interface_num] == DNX_TMC_FC_CAL_TYPE_ILKN) &&
           (tm->fc_oob_ilkn_pad_sync_pin == 1)) {
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr,
                                                 REG_PORT_ANY, 0, SWAP_ILKN_IFf, 1);
            DNXC_IF_ERR_EXIT(rv);
        }
    }
    fc_reset_reg = SOC_IS_QUX(unit) ? NIF_FC_RESETr : NBIH_FC_RESETr;
    /* Enable Flow Control Interface processes indications from CFC, MALs and ILKN */
    rv = soc_reg_field32_modify(unit, fc_reset_reg, REG_PORT_ANY, FC_RESETf, 0);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_init_pfc_mapping(
      DNX_SAND_IN int  unit
    )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        nif_pfc_data[3],
        *data_select = NULL,
        entry_offset = 0,
        field_offset = 0,
        base_queue_pair = 0,
        nof_priorities = 0,
        i = 0,
        interface_i = 0,
        valid = 0, 
        queue_pair_index = 0;
    soc_port_if_t
        interface_type = SOC_PORT_IF_NULL;
    uint32
        phy_port = 0;
    soc_pbmp_t
        pbmp;
    soc_port_t
        logical_port = 0;
    uint32
        tm_port = 0,
        flags = 0;
    int 
        core = 0;

    DNXC_INIT_FUNC_DEFS;

    /* Default PFC/TC mapping */
    rv = dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp);
    DNXC_IF_ERR_EXIT(rv);

    SOC_PBMP_ITER(pbmp, logical_port)
    {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, logical_port, &flags)); 
        if (DNX_PORT_IS_STAT_INTERFACE(flags))
        {
            continue;
        }
        if (DNX_PORT_IS_ELK_INTERFACE(flags))
        {
            continue;
        }
        if (DNX_PORT_IS_LB_MODEM(flags))
        {
            continue;
        }

        rv = dnx_port_sw_db_local_to_tm_port_get(unit, logical_port, &tm_port, &core);
        DNXC_IF_ERR_EXIT(rv);

        rv = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_queue_pair);
        DNXC_IF_ERR_EXIT(rv);

        rv = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
        DNXC_IF_ERR_EXIT(rv);

        rv = dnx_port_sw_db_interface_type_get(unit, logical_port, &interface_type);        
        DNXC_IF_ERR_EXIT(rv);

        if (interface_type == SOC_PORT_IF_NULL || interface_type == SOC_PORT_IF_CPU ||
            interface_type == SOC_PORT_IF_OLP || interface_type == SOC_PORT_IF_RCY ||
            interface_type == SOC_PORT_IF_SAT || interface_type == SOC_PORT_IF_IPSEC ||
            interface_type == SOC_PORT_IF_ERP || interface_type == SOC_PORT_IF_OAMP) {
            continue;
        }

        rv = dnx_port_sw_db_first_phy_port_get(unit, logical_port, &phy_port);
        DNXC_IF_ERR_EXIT(rv);

        rv = MBCM_DNX_DRIVER_CALL(unit,
                mbcm_dnx_qsgmii_offsets_remove, (unit, phy_port, &phy_port));
        DNXC_IF_ERR_EXIT(rv);

        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        if (phy_port >= SOC_JER2_JER_NIF_FIRST_FABRIC_PHY_PORT(unit))
        {
            continue;
        }
#endif 

        interface_i = phy_port - 1;

        valid = 0x1;

        entry_offset = interface_i;
        
        if (SOC_IS_QUX(unit)) {
    	      entry_offset /= 3;
        }
        if(entry_offset >= SOC_MEM_SIZE(unit, CFC_NIF_PFC_MAPm)) {
            /* Protection against miss-configured interfaces */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("entry offset is out of range[0, %d]"), (SOC_MEM_SIZE(unit, CFC_NIF_PFC_MAPm)-1)));
        }
            
        rv = soc_mem_read(unit, CFC_NIF_PFC_MAPm, MEM_BLOCK_ANY, entry_offset, nif_pfc_data);
        DNXC_IF_ERR_EXIT(rv);

        for(i = 0; i < nof_priorities; i++) {
            field_offset = i;
            data_select = nif_pfc_data; 
            queue_pair_index = base_queue_pair + i;
            queue_pair_index += ((core == 1) ? JER2_ARAD_EGR_NOF_BASE_Q_PAIRS : 0);
      
            soc_mem_field32_set(unit, CFC_NIF_PFC_MAPm, data_select, index_field[field_offset], queue_pair_index);
            soc_mem_field32_set(unit, CFC_NIF_PFC_MAPm, data_select, select_field[field_offset], 0);
            soc_mem_field32_set(unit, CFC_NIF_PFC_MAPm, data_select, valid_field[field_offset], valid);
        }

        rv = soc_mem_write(unit, CFC_NIF_PFC_MAPm, MEM_BLOCK_ANY, entry_offset, nif_pfc_data);
        DNXC_IF_ERR_EXIT(rv);
    }
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_flow_control_init_pp_rx_calendar(
    DNX_SAND_IN  int                 unit
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        interface_number = 0,
        fld_val = 0,
        reg32_data = 0;
    soc_reg_t
        reg = INVALIDr;
    soc_field_t
        fld = INVALIDf;
    uint32
        pp_intf_id = 0;
    int 
        is_pp_enable[DNX_TMC_FC_NOF_PP_INTF] = {0},
        is_coe_enable[DNX_TMC_FC_NOF_PP_INTF] = {0},
        calendar_len[DNX_TMC_FC_NOF_PP_INTF] = {0};
    int
        coe_tick = 0;
    soc_dnx_tm_config_t  
        *tm;
    soc_reg_above_64_val_t 
        reg_val_64;
    soc_reg_above_64_val_t 
        fld_val_64;
    int 
        i = 0, offset = 0;

    DNXC_INIT_FUNC_DEFS;

    tm = &(SOC_DNX_CONFIG(unit)->tm);

    /* Only one fc calendar interface is needed to map for PP calendar.
     * The connection between fc calendar interface and PP RX calendar is fixed. 
     *   PP0:  OOB0, InB0, InB2, InB4
     *   PP1:  OOB1, InB1, InB3, InB5
     */

    /* 1. Select PP as ILKN OOB source { */
    for(interface_number = 0; interface_number < tm->max_oob_ports; interface_number++){
        if((tm->fc_oob_type[interface_number] != DNX_TMC_FC_CAL_TYPE_COE) &&
           (tm->fc_oob_type[interface_number] != DNX_TMC_FC_CAL_TYPE_E2E))
            continue;

        pp_intf_id = interface_number % 2;
        is_pp_enable[pp_intf_id] = 1;
        calendar_len[pp_intf_id] = tm->fc_oob_calender_length[interface_number][DNX_TMC_CONNECTION_DIRECTION_RX];
        if (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE) {
            is_coe_enable[pp_intf_id] = 1;
        }

        /* Put the OOB interfaces into Reset */
        fld_val = 0x0;
        fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_RX_0_RSTNf : OOB_RX_1_RSTNf);
        rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);

        /* Disable the FC before configuring it */
        fld_val = 0x0;
        fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_ENf : ILKN_OOB_RX_1_ENf); 
        rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);

        /* Select PP as ILKN OOB source { */
        fld_val = 1;
        fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_SELf : ILKN_OOB_RX_1_SELf);
        rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);
        /* Select PP as ILKN OOB source } */

        /* Take the OOB/ILKN interfaces out of Reset */
        fld_val = 0x1;
        fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_RX_0_RSTNf : OOB_RX_1_RSTNf);
        rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);

        /* Enable the FC after configuring it */
        fld_val = 0x1;
        fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_ENf : ILKN_OOB_RX_1_ENf); 
        rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    /* 1. Select PP as ILKN OOB source } */

    /* 2. Select PP as ILKN InB source { */
    for(interface_number = 0; interface_number < tm->max_interlaken_ports; interface_number++){
        if(tm->fc_inband_intlkn_type[interface_number] != DNX_TMC_FC_CAL_INB_TYPE_COE &&
           tm->fc_inband_intlkn_type[interface_number] != DNX_TMC_FC_CAL_INB_TYPE_E2E) {
            continue;
        }

        pp_intf_id = interface_number % 2;
        is_pp_enable[pp_intf_id] = 1;
        calendar_len[pp_intf_id] = tm->fc_inband_intlkn_calender_length[interface_number][DNX_TMC_CONNECTION_DIRECTION_RX];
        if (tm->fc_inband_intlkn_type[interface_number] == DNX_TMC_FC_CAL_INB_TYPE_COE) {
            is_coe_enable[pp_intf_id] = 1;
        }

        /* Disable the FC before configuring it */
        fld_val = 0x0;
        fld = ilkn_inb_rx_ena_flds[interface_number];
        rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);

        /* Select PP as ILKN INB source { */
        if(!SOC_IS_QAX(unit)) {
            fld_val = 1;
            fld = inb_sel_fld[interface_number];
            rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);
        }
        /* Select PP as ILKN INB source { */

        /* Enable the FC after configuring it */
        fld_val = 0x1;
        fld = ilkn_inb_rx_ena_flds[interface_number];
        rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
        DNXC_IF_ERR_EXIT(rv);

    }
    /* 2. Select PP as ILKN InB source } */

    /* 3. Configure PP Rx calendar parameters { */
    for (pp_intf_id = 0; pp_intf_id < DNX_TMC_FC_NOF_PP_INTF; pp_intf_id++) {
        if (is_pp_enable[pp_intf_id]) {
            reg = (pp_intf_id == 0) ? CFC_IHB_0_PP_INBANDr : CFC_IHB_1_PP_INBANDr;

            /* Enable PP inband FC(both modes: COE and E2EFC)*/
            fld_val = 1;
            fld = (pp_intf_id == 0) ? IHB_0_PP_INB_ENBf : IHB_1_PP_INB_ENBf;
            soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);

            /* Setting PP inband Rx calendar length */
            fld_val = calendar_len[pp_intf_id] - 1;
            fld = (pp_intf_id == 0) ? IHB_0_PP_INB_CAL_LENf : IHB_1_PP_INB_CAL_LENf;
            soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);

            /* Setting PP inband calendar mode */
            fld_val = is_coe_enable[pp_intf_id] ? 0 : 1;
            fld = (pp_intf_id == 0) ? IHB_0_PP_MODEf : IHB_1_PP_MODEf;
            soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);

            if (is_coe_enable[pp_intf_id]) {
                /* Setting PP COE sub-mode: 0: Pause, 1: PFC */
                fld_val = tm->fc_calendar_coe_mode;
                fld = (pp_intf_id == 0) ? IHB_0_PP_COE_MODEf : IHB_1_PP_COE_MODEf;
                soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);

                /* Setting PP COE Pause counter rate - Configuration (per block) number of clocks 
                   between two successive decrements of each counter */
                coe_tick = tm->fc_calendar_pause_resolution * SOC_INFO(unit).frequency;
                fld_val = coe_tick - 1;
                fld = (pp_intf_id == 0) ? IHB_0_PP_COE_TICKf : IHB_1_PP_COE_TICKf;
                soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);
            }
            else {
                /* Setting PP E2E FC status size */
                fld_val = tm->fc_calendar_e2e_status_of_entries;
                fld = (pp_intf_id == 0) ? IHB_0_PP_E_2_EFC_SIZEf : IHB_1_PP_E_2_EFC_SIZEf;
                soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);

                /* Setting PP E2E FC polarity */
                fld_val = tm->fc_calendar_indication_invert;
                fld = (pp_intf_id == 0) ? IHB_0_PP_E_2_EFC_POLf : IHB_1_PP_E_2_EFC_POLf;
                soc_reg_field_set(unit, reg, &reg32_data, fld, fld_val);                
            }

            rv = soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg32_data);
            DNXC_IF_ERR_EXIT(rv);
        }        
    }
    /* 3. Configure PP Rx calendar parameters } */

    /* 4. Update FC packet parameters { */
    if (tm->fc_coe_data_offset > ((1 << soc_reg_field_length(unit, IHB_COE_CFGSr, COE_DATA_OFFSETf)) - 1)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("coe data offset(%d) is out of range[0, %d]"), 
            tm->fc_coe_data_offset, ((1 << soc_reg_field_length(unit, IHB_COE_CFGSr, COE_DATA_OFFSETf)) - 1)));
    }

    reg = IHB_COE_CFGSr;
    for (pp_intf_id = 0; pp_intf_id < (SOC_IS_QAX(unit) ? 1 : DNX_TMC_FC_NOF_PP_INTF); pp_intf_id++) {
        SOC_REG_ABOVE_64_CLEAR(reg_val_64);
        SOC_REG_ABOVE_64_CLEAR(fld_val_64);
        rv = soc_reg_above_64_get(unit, reg, pp_intf_id, 0, reg_val_64);
        DNXC_IF_ERR_EXIT(rv);

        /* Enable COE FC */
        fld_val = 1;
        fld = COE_ENABLEf;
        soc_reg_above_64_field32_set(unit, reg, reg_val_64, fld, fld_val);

        /* Setting COE CONTROL OPCODE */
        fld_val = (tm->fc_calendar_coe_mode == DNX_TMC_FC_COE_MODE_PAUSE) ? 0x10 : 0x101;
        fld = COE_CONTROL_OPCODEf;
        soc_reg_above_64_field32_set(unit, reg, reg_val_64, fld, fld_val);

        /* Setting COE MAC ADDRESS */
        offset = 0;
        for (i = 0; i < 6; i++) {
            fld_val = tm->fc_coe_mac_address[5-i];
            SHR_BITCOPY_RANGE(fld_val_64, offset, &fld_val, 0, 8);
            offset += 8;
        }
        fld = COE_MAC_ADDRESSf;
        soc_reg_above_64_field_set(unit, reg, reg_val_64, fld, fld_val_64);

        /* Setting COE ETHER TYPE */
        fld_val = tm->fc_coe_ethertype;
        fld = COE_ETHER_TYPEf;
        soc_reg_above_64_field32_set(unit, reg, reg_val_64, fld, fld_val);

        /* Setting offset in bytes from after Ethernet Frame for COE FC data */
        fld_val = tm->fc_coe_data_offset;
        fld = COE_DATA_OFFSETf;
        soc_reg_above_64_field32_set(unit, reg, reg_val_64, fld, fld_val);

        /* Setting channel id to In PP PORT */
        fld_val = 1;
        fld = COE_USE_IN_PP_PORTf;
        soc_reg_above_64_field32_set(unit, reg, reg_val_64, fld, fld_val);

        rv = soc_reg_above_64_set(unit, reg, pp_intf_id, 0, reg_val_64);
        DNXC_IF_ERR_EXIT(rv);
    }
    /* 4. Update FC packet parameters } */

exit:
    DNXC_FUNC_RETURN;
}



soc_error_t
  jer2_jer_flow_control_init_oob_rx(
    DNX_SAND_IN  int                 unit
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        interface_number = 0,
        fld_val = 0,
        fld = 0;
    uint32
        per_rep_idx = 0,
        rep_idx = 0,
        entry_idx = 0,
        cal_length = 0,
        cal_reps = 0;
    uint64
        field64 = COMPILER_64_INIT(0, 0);
    soc_mem_t
        cal_table = INVALIDm;
    soc_reg_above_64_val_t
        cal_table_data;
    DNX_TMC_FC_CAL_MODE
        cal_mode_ndx = DNX_TMC_FC_NOF_CAL_MODES;
    soc_dnx_tm_config_t  *tm;

    DNXC_INIT_FUNC_DEFS;
 
    SOC_REG_ABOVE_64_CLEAR(cal_table_data);
    tm = &(SOC_DNX_CONFIG(unit)->tm);

    for(interface_number = 0; interface_number < DNX_TMC_FC_NOF_OOB_IDS; interface_number++)
    {
        if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_NONE) {
            continue;
        }

        /* Handles RX */
        if(tm->fc_oob_mode[interface_number] & SOC_DNX_FC_CAL_MODE_RX_ENABLE)
        {
            /* Put the OOB interfaces into Reset */
            fld_val = 0x0;
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_RX_0_RSTNf : OOB_RX_1_RSTNf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            /* Disable the FC before configuring it */
            fld_val = 0x0;
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_ENf : ILKN_OOB_RX_1_ENf); 
            rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_ENf : SPI_OOB_RX_1_ENf);
            COMPILER_64_SET(field64,0,fld_val);
            rv = soc_reg_above_64_field64_modify(unit, CFC_SPI_OOB_RX_CONFIGURATION_0r, REG_PORT_ANY, 0, fld,  field64);
            DNXC_IF_ERR_EXIT(rv);

            /* Configuration start { */

            /*Disable flow control on port fault*/

           /*  Configure polarity for ILKN FC indications */
            if (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN || 
                tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE || 
                tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E) {
                fld_val = (tm->fc_oob_ilkn_indication_invert[interface_number][DNX_TMC_CONNECTION_DIRECTION_RX]) ? 1 : 0;
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_FC_POLARITYf : ILKN_OOB_RX_1_FC_POLARITYf);
                rv = soc_reg_field32_modify(unit, CFC_ILKN_OOB_POLARITY_CFGr, REG_PORT_ANY, fld, fld_val);
                DNXC_IF_ERR_EXIT(rv);
            }

            /*  Configure OOB Type: ILKN/SPI/HCFC */
            fld_val = ((tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ||
                        tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE ||
                        tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E) ? 1 : 0);
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_RX_0_MODEf : OOB_RX_1_MODEf);                            
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_SPI || tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_HCFC) {
                fld_val = (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_HCFC ? 1 : 0);
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_0_OOB_MODEf : SPI_1_OOB_MODEf);                                            
                rv = soc_reg_above_64_field32_modify(unit, CFC_SPI_OOB_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
                DNXC_IF_ERR_EXIT(rv);
            }

            /* Calendar length (single repetition) */
            cal_length = tm->fc_oob_calender_length[interface_number][DNX_TMC_CONNECTION_DIRECTION_RX];
            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ||
               tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE ||
               tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E)
            {
                fld = (interface_number == JER2_ARAD_FC_ILKN_ID_A ? ILKN_OOB_RX_0_CAL_LENf : ILKN_OOB_RX_1_CAL_LENf);
                rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  cal_length - 1);
                DNXC_IF_ERR_EXIT(rv);
            }
            else
            {
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_CAL_LENf : SPI_OOB_RX_1_CAL_LENf);
                COMPILER_64_SET(field64, 0, cal_length);
                rv = soc_reg_above_64_field64_modify(unit, CFC_SPI_OOB_RX_CONFIGURATION_0r, REG_PORT_ANY, 0, fld,  field64);
                DNXC_IF_ERR_EXIT(rv);
            } 

            /*  Calendar number of repetitions - SPI/OOB Only */
            cal_reps = (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ||
                        tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE ||
                        tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E) ? 1 : 
                       tm->fc_oob_calender_rep_count[interface_number][DNX_TMC_CONNECTION_DIRECTION_RX];
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_CAL_Mf : SPI_OOB_RX_1_CAL_Mf);
            COMPILER_64_SET(field64,0,cal_reps);
            rv = soc_reg_above_64_field64_modify(unit, CFC_SPI_OOB_RX_CONFIGURATION_0r, REG_PORT_ANY, 0, fld,  field64);
            DNXC_IF_ERR_EXIT(rv);

            /* Clear Calendar entry */
            cal_mode_ndx = (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ||
                            tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE ||
                            tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E) ? 
                            DNX_TMC_FC_CAL_MODE_ILKN_OOB : DNX_TMC_FC_CAL_MODE_SPI_OOB;
            rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_RX, interface_number, &cal_table);
            DNXC_IF_ERR_EXIT(rv);
            for (per_rep_idx = 0; per_rep_idx < cal_length; per_rep_idx++) {
                soc_mem_field32_set(unit, cal_table, cal_table_data, FC_INDEXf, 0);
                soc_mem_field32_set(unit, cal_table, cal_table_data, FC_DST_SELf, jer2_jer_fc_rec_cal_dest_type_to_val_internal(DNX_TMC_FC_REC_CAL_DEST_NONE));

                for (rep_idx = 0; rep_idx < cal_reps; rep_idx++) {
                    entry_idx = (rep_idx * cal_length) + per_rep_idx;
                    rv = soc_mem_write(unit, cal_table, MEM_BLOCK_ANY, entry_idx, cal_table_data);
                    DNXC_IF_ERR_EXIT(rv);
                }
            }

            /* Configuration end } */

            /* Take the OOB/ILKN interfaces out of Reset */
            fld_val = 0x1;
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_RX_0_RSTNf : OOB_RX_1_RSTNf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            /* Enable the FC after configuring it */
            fld_val = 0x1;
            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ||
               tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE ||
               tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E) {
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_ENf : ILKN_OOB_RX_1_ENf); 
                rv = soc_reg_above_64_field32_modify(unit, CFC_ILKN_RX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
                DNXC_IF_ERR_EXIT(rv);
            }
            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_SPI || tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_HCFC) {
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_ENf : SPI_OOB_RX_1_ENf);
                COMPILER_64_SET(field64,0,fld_val);
                rv = soc_reg_above_64_field64_modify(unit, CFC_SPI_OOB_RX_CONFIGURATION_0r, REG_PORT_ANY, 0, fld,  field64);
                DNXC_IF_ERR_EXIT(rv);
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_flow_control_init_oob_tx(
    DNX_SAND_IN  int                 unit
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        interface_number = 0,
        fld_val = 0,
        fld = 0;
    uint32
        per_rep_idx = 0,
        rep_idx = 0,
        entry_idx = 0,
        cal_length = 0,
        cal_reps = 0;
    uint64
        field64 = COMPILER_64_INIT(0, 0);
    soc_mem_t
        cal_table = INVALIDm;
    soc_reg_above_64_val_t
        cal_table_data;
    DNX_TMC_FC_CAL_MODE
        cal_mode_ndx = DNX_TMC_FC_NOF_CAL_MODES;
    soc_dnx_tm_config_t  *tm;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(cal_table_data);
    tm = &(SOC_DNX_CONFIG(unit)->tm);
  
    for(interface_number = 0; interface_number < DNX_TMC_FC_NOF_OOB_IDS; interface_number++)
    {
        if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_NONE) {
            continue;
        }

        /* OOB TX CLK Speed */
        fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_TX_0_OUT_SPEEDf : OOB_TX_1_OUT_SPEEDf);
        rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld, tm->fc_oob_tx_speed[interface_number]);
        DNXC_IF_ERR_EXIT(rv);

        /* Handles TX */
        if(tm->fc_oob_mode[interface_number] & SOC_DNX_FC_CAL_MODE_TX_ENABLE)
        {
            /* Put the OOB interfaces into Reset */
            fld_val = 0x0;
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_TX_0_RSTNf : OOB_TX_1_RSTNf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_TX_0_OUT_PHASEf : OOB_TX_1_OUT_PHASEf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld, 3);
            DNXC_IF_ERR_EXIT(rv);

            /* Disable the FC before configuring it */
            fld_val = 0x0;
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_TX_0_ENf : ILKN_OOB_TX_1_ENf); 
            COMPILER_64_SET(field64,0,fld_val);
            rv = soc_reg_above_64_field64_modify(unit, CFC_ILKN_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  field64);
            DNXC_IF_ERR_EXIT(rv);
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_ENf : SPI_OOB_TX_1_ENf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_SPI_OOB_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            /* Configuration start */

            /*  Configure polarity for ILKN FC indications */
            if (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN || 
                tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_COE || 
                tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_E2E) {
                fld_val = (SOC_DNX_CONFIG(unit)->tm.fc_oob_ilkn_indication_invert[interface_number][DNX_TMC_CONNECTION_DIRECTION_TX]) ? 1 : 0;
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_TX_0_FC_POLARITYf : ILKN_OOB_TX_1_FC_POLARITYf);
                rv = soc_reg_field32_modify(unit, CFC_ILKN_OOB_POLARITY_CFGr, REG_PORT_ANY, fld, fld_val);
                DNXC_IF_ERR_EXIT(rv);
            }

            /* Configure OOB Type: ILKN/SPI/HCFC */
            fld_val = (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ? 1 : 0);
            COMPILER_64_SET(field64,0,fld_val);
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_TX_0_MODEf : OOB_TX_1_MODEf);        
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            /* Calendar length (single repetition) */
            cal_length = tm->fc_oob_calender_length[interface_number][DNX_TMC_CONNECTION_DIRECTION_TX];
            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN) {
                fld = (interface_number == JER2_ARAD_FC_ILKN_ID_A ? ILKN_OOB_TX_0_CAL_LENf : ILKN_OOB_TX_1_CAL_LENf);
                COMPILER_64_SET(field64,0,cal_length - 1);
                rv = soc_reg_above_64_field64_modify(unit, CFC_ILKN_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  field64);
                DNXC_IF_ERR_EXIT(rv);
            }
            else
            { 
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_CAL_LENf : SPI_OOB_TX_1_CAL_LENf);
                rv = soc_reg_above_64_field32_modify(unit, CFC_SPI_OOB_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  cal_length);
                DNXC_IF_ERR_EXIT(rv);
            } 

            /*  Calendar number of repetitions - SPI/OOB Only */
            cal_reps = tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN ? 
                           1 : 
                           tm->fc_oob_calender_rep_count[interface_number][DNX_TMC_CONNECTION_DIRECTION_TX];
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_CAL_Mf : SPI_OOB_TX_1_CAL_Mf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_SPI_OOB_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  cal_reps);
            DNXC_IF_ERR_EXIT(rv);

            /* Clear Calendar entry */
            cal_mode_ndx = (tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN) ? DNX_TMC_FC_CAL_MODE_ILKN_OOB : DNX_TMC_FC_CAL_MODE_SPI_OOB;
            rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_TX, interface_number, &cal_table);
            DNXC_IF_ERR_EXIT(rv);
            for (per_rep_idx = 0; per_rep_idx < cal_length; per_rep_idx++) {
                soc_mem_field32_set(unit, cal_table, cal_table_data, FC_INDEXf, 0);
                soc_mem_field32_set(unit, cal_table, cal_table_data, FC_SRC_SELf, jer2_jer_fc_gen_cal_src_type_to_val_internal(DNX_TMC_FC_GEN_CAL_SRC_CONST));

                for (rep_idx = 0; rep_idx < cal_reps; rep_idx++) {
                    entry_idx = (rep_idx * cal_length) + per_rep_idx;

                    rv = soc_mem_write(unit, cal_table, MEM_BLOCK_ANY, entry_idx, cal_table_data);
                    DNXC_IF_ERR_EXIT(rv);
                }
            }

            /* Configuration end */

            /* Take the OOB interface out of Reset */
            fld_val = 0x1;
            fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? OOB_TX_0_RSTNf : OOB_TX_1_RSTNf);
            rv = soc_reg_above_64_field32_modify(unit, CFC_OOB_PAD_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
            DNXC_IF_ERR_EXIT(rv);

            /* Enable the FC after configuring it */
            fld_val = 0x1;
            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_ILKN) {
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_TX_0_ENf : ILKN_OOB_TX_1_ENf); 
                COMPILER_64_SET(field64,0,fld_val);
                rv = soc_reg_above_64_field64_modify(unit, CFC_ILKN_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  field64);
                DNXC_IF_ERR_EXIT(rv);
            }
            if(tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_SPI || tm->fc_oob_type[interface_number] == DNX_TMC_FC_CAL_TYPE_HCFC) {
                fld = (interface_number == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_ENf : SPI_OOB_TX_1_ENf);
                rv = soc_reg_above_64_field32_modify(unit, CFC_SPI_OOB_TX_CONFIGURATIONr, REG_PORT_ANY, 0, fld,  fld_val);
                DNXC_IF_ERR_EXIT(rv);
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     jer2_arad_flow_control_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
soc_error_t
  jer2_jer_flow_control_init(
    DNX_SAND_IN  int                 unit
  )
{
    soc_error_t
        rv = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;
    DNXC_PCID_LITE_SKIP(unit);

    rv = jer2_jer_flow_control_regs_init(unit);
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_jer_flow_control_init_oob_rx(unit);
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_jer_flow_control_init_oob_tx(unit);
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_jer_flow_control_init_pp_rx_calendar(unit);
    DNXC_IF_ERR_EXIT(rv);

    if(!SOC_IS_QAX(unit)) {
        rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.alloc(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_enables_verify(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *ena_info
    )
{
    int i = 0;
    DNX_TMC_FC_ENABLE fc_enable_not_support[] = FC_ENABLE_NOT_SUPPORT_JER2_JER;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(cfc_enables);
    DNXC_NULL_CHECK(ena_info);

    for (i = 0; i < sizeof(fc_enable_not_support)/sizeof(DNX_TMC_FC_ENABLE); i++) {
        if (SHR_BITGET(cfc_enables->bmp, fc_enable_not_support[i])) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("cfc_enables[%d] is not supported by this chip"), i));
        }
    }  

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
   jer2_jer_fc_enables_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *ena_info
    )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_reg_above_64_val_t
        reg_data;
    soc_reg_t 
        reg = CFC_CFC_ENABLERSr;
    uint32 
        field_value = 0;

    DNXC_INIT_FUNC_DEFS;

    rv = jer2_jer_fc_enables_verify(unit, cfc_enables, ena_info);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(reg_data);
    rv = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(rv);

    SET_FC_ENABLE(DNX_TMC_FC_EN, CFC_ENf);
    if(!SOC_IS_QAX(unit)) {
        SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_NIF_CNM_LLFC_EN, EGQ_TO_NIF_CNM_LLFC_ENf);
        SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_NIF_CNM_PFC_EN, EGQ_TO_NIF_CNM_PFC_ENf);
        SET_FC_ENABLE(DNX_TMC_FC_NIF_TO_GEN_PFC_EN, NIF_TO_GEN_PFC_ENf);
    }
    SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_DEVICE_EN, EGQ_TO_SCH_DEVICE_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_ERP_EN, EGQ_TO_SCH_ERP_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_ERP_TC_EN, EGQ_TO_SCH_ERP_TC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_IF_EN, EGQ_TO_SCH_IF_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_PFC_EN, EGQ_TO_SCH_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_EGQ_RCL_PFC_EN, GLB_RSC_TO_EGQ_RCL_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_HCFC_HP_CFG, GLB_RSC_TO_HCFC_HP_CFGf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_HCFC_LP_CFG, GLB_RSC_TO_HCFC_LP_CFGf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_NIF_LLFC_EN, GLB_RSC_TO_NIF_LLFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_NIF_PFC_EN, GLB_RSC_TO_NIF_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_HP_CFG, GLB_RSC_TO_RCL_PFC_HP_CFGf);
    SET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_LP_CFG, GLB_RSC_TO_RCL_PFC_LP_CFGf);
    SET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_EGQ_PFC_EN, ILKN_RX_TO_EGQ_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_EGQ_PORT_EN, ILKN_RX_TO_EGQ_PORT_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_GEN_PFC_EN, ILKN_RX_TO_GEN_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_NIF_FAST_LLFC_EN, ILKN_RX_TO_NIF_FAST_LLFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_LLFC_VSQ_TO_NIF_EN, LLFC_VSQ_TO_NIF_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_PFC_VSQ_TO_NIF_EN, PFC_VSQ_TO_NIF_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_SCH_OOB_RX_EN, SCH_OOB_RX_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PFC_EN, SPI_OOB_RX_TO_EGQ_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PORT_EN, SPI_OOB_RX_TO_EGQ_PORT_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_GEN_PFC_EN, SPI_OOB_RX_TO_GEN_PFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_NIF_FAST_LLFC_EN, SPI_OOB_RX_TO_NIF_FAST_LLFC_ENf);
    SET_FC_ENABLE(DNX_TMC_FC_STAT_VSQ_TO_HCFC_EN, STAT_VSQ_TO_HCFC_ENf);
    if(SOC_IS_QAX(unit)) {
        SET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_LAG_EN, EGQ_TO_SCH_LAG_ENf);
    }

    rv = soc_reg_above_64_set(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(rv);
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
   jer2_jer_fc_enables_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN DNX_TMC_FC_ENABLE_BITMAP      *cfc_enables,
      DNX_SAND_OUT DNX_TMC_FC_ENABLE_BITMAP     *ena_info
    )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_reg_above_64_val_t 
        reg_data;
    soc_reg_t 
        reg = CFC_CFC_ENABLERSr;
    uint32 
        field_value = 0;

    DNXC_INIT_FUNC_DEFS;

    rv = jer2_jer_fc_enables_verify(unit, cfc_enables, ena_info);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(reg_data);
    rv = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(rv);

    GET_FC_ENABLE(DNX_TMC_FC_EN, CFC_ENf);
    if(!SOC_IS_QAX(unit)) {
        GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_NIF_CNM_LLFC_EN, EGQ_TO_NIF_CNM_LLFC_ENf);
        GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_NIF_CNM_PFC_EN, EGQ_TO_NIF_CNM_PFC_ENf);
        GET_FC_ENABLE(DNX_TMC_FC_NIF_TO_GEN_PFC_EN, NIF_TO_GEN_PFC_ENf);
    }
    GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_DEVICE_EN, EGQ_TO_SCH_DEVICE_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_ERP_EN, EGQ_TO_SCH_ERP_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_ERP_TC_EN, EGQ_TO_SCH_ERP_TC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_IF_EN, EGQ_TO_SCH_IF_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_PFC_EN, EGQ_TO_SCH_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_EGQ_RCL_PFC_EN, GLB_RSC_TO_EGQ_RCL_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_HCFC_HP_CFG, GLB_RSC_TO_HCFC_HP_CFGf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_HCFC_LP_CFG, GLB_RSC_TO_HCFC_LP_CFGf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_NIF_LLFC_EN, GLB_RSC_TO_NIF_LLFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_NIF_PFC_EN, GLB_RSC_TO_NIF_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_HP_CFG, GLB_RSC_TO_RCL_PFC_HP_CFGf);
    GET_FC_ENABLE(DNX_TMC_FC_GLB_RSC_TO_RCL_PFC_LP_CFG, GLB_RSC_TO_RCL_PFC_LP_CFGf);
    GET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_EGQ_PFC_EN, ILKN_RX_TO_EGQ_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_EGQ_PORT_EN, ILKN_RX_TO_EGQ_PORT_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_GEN_PFC_EN, ILKN_RX_TO_GEN_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_ILKN_RX_TO_NIF_FAST_LLFC_EN, ILKN_RX_TO_NIF_FAST_LLFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_LLFC_VSQ_TO_NIF_EN, LLFC_VSQ_TO_NIF_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_PFC_VSQ_TO_NIF_EN, PFC_VSQ_TO_NIF_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_SCH_OOB_RX_EN, SCH_OOB_RX_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PFC_EN, SPI_OOB_RX_TO_EGQ_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PORT_EN, SPI_OOB_RX_TO_EGQ_PORT_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_GEN_PFC_EN, SPI_OOB_RX_TO_GEN_PFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_SPI_OOB_RX_TO_NIF_FAST_LLFC_EN, SPI_OOB_RX_TO_NIF_FAST_LLFC_ENf);
    GET_FC_ENABLE(DNX_TMC_FC_STAT_VSQ_TO_HCFC_EN, STAT_VSQ_TO_HCFC_ENf);
    if(SOC_IS_QAX(unit)) {
        GET_FC_ENABLE(DNX_TMC_FC_EGQ_TO_SCH_LAG_EN, EGQ_TO_SCH_LAG_ENf);
    }
  
exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  jer2_jer_fc_gen_inbnd_pfc_set_unsafe(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  soc_port_t                    port,
    DNX_SAND_IN  DNX_TMC_FC_GEN_INBND_PFC      *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        first_phy = 0, 
        first_lane = 0;
    uint32
        reg_offset = 0,
        bit_offset = 0, 
        fld_offset = 0;
    uint32
        base_q_pair = 0;
    JER2_ARAD_EGQ_PPCT_TBL_DATA
        data;
    int
        core = 0;
    uint32
        pfc_map_rx_mlf_reg = 0;
    soc_mem_t 
        mem = INVALIDm;
    soc_reg_above_64_val_t
        map_data;
    uint32
        entry_offset = 0; 
    soc_reg_above_64_val_t 
        cat2tc_map_ena;
    soc_reg_t 
        fc_pfc_qmlf_mask_reg = INVALIDr;
    soc_reg_t
        cat2tc_map_ena_reg = INVALIDr;

    DNXC_INIT_FUNC_DEFS;

    rv = dnx_port_sw_db_core_get(unit, port, &core);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy /*one based*/);
    DNXC_IF_ERR_EXIT(rv);

    rv = MBCM_DNX_DRIVER_CALL(unit,
            mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane));
    DNXC_IF_ERR_EXIT(rv);
    
    first_lane--; /* first_lane returned is one-based */

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_egq_ppct_tbl_get_unsafe(
              unit,
              core,
              base_q_pair,
              &data
              );
    DNXC_IF_ERR_EXIT(rv);

    data.cnm_intrcpt_fc_en = DNX_SAND_BOOL2NUM(info->cnm_intercept_enable);
    data.cnm_intrcpt_fc_vec_pfc = info->cnm_pfc_channel;
    rv = jer2_arad_egq_ppct_tbl_set_unsafe(
          unit,
          core,
          base_q_pair,
          &data
          );
    DNXC_IF_ERR_EXIT(rv);


    /* Set the bit in the GLB RSC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) { */
    if ((info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_LOW) || (info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH)) {
        mem = (core == 0) ? CFC_GLB_RSC_IQM_0_MAPm : CFC_GLB_RSC_IQM_1_MAPm;
        entry_offset = ((info->is_ocb_only == 1)?(4):(0)) + 
                       ((info->glbl_rcs_pool == 1)?(2):(0)) + 
                       ((info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH)?(1):(0));
        entry_offset += GLB_RSC_MAP_TBL_NIF_PFC_OFFSET;

        /* set mapping from CAT2+TC to NIF PFC */
        rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
        DNXC_IF_ERR_EXIT(rv);

        if (info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH) {
            SHR_BITCOPY_RANGE(map_data, (first_lane*8), &(info->glbl_rcs_high), 0, 8);
        }
        else {
            SHR_BITCOPY_RANGE(map_data, (first_lane*8), &(info->glbl_rcs_low), 0, 8);
        }

        rv = soc_mem_write(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
        DNXC_IF_ERR_EXIT(rv);
    }
    /* Set the bit in the GLB RSC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) } */

    /* Set the bit in the MLF PFC bitmap { */
    if (info->gen_src == DNX_TMC_FC_GEN_SRC_NIF) {
        reg_offset = first_lane / NUM_OF_LANES_IN_PM;
        fld_offset = first_lane % NUM_OF_LANES_IN_PM;
        fc_pfc_qmlf_mask_reg = SOC_IS_QUX(unit) ? NIF_FC_PFC_QMLF_MASKr : NBIH_FC_PFC_QMLF_MASKr;

        rv = soc_reg32_get(unit, fc_pfc_qmlf_mask_reg, REG_PORT_ANY, reg_offset, &pfc_map_rx_mlf_reg);
        DNXC_IF_ERR_EXIT(rv);

        soc_reg_field_set(unit, fc_pfc_qmlf_mask_reg, &pfc_map_rx_mlf_reg, rx_mlf_pfc_fld[fld_offset], info->nif_cls_bitmap);

        rv = soc_reg32_set(unit, fc_pfc_qmlf_mask_reg, REG_PORT_ANY, reg_offset, pfc_map_rx_mlf_reg);
        DNXC_IF_ERR_EXIT(rv);
    }
    /* Set the bit in the MLF PFC bitmap } */

    /* Set the bit in the CAT2+TC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) { */
    if (info->gen_src == DNX_TMC_FC_GEN_SRC_VSQ_CAT2TC) {
        mem = (core == 0) ? CFC_CAT_2_TC_IQM_0_MAP_NIFm : CFC_CAT_2_TC_IQM_1_MAP_NIFm;
        entry_offset = info->cat2_tc + ((info->is_ocb_only == 1)?(8):(0));
        
        cat2tc_map_ena_reg = CFC_CAT_2_TC_MAP_NIF_ENAr;
        bit_offset = info->cat2_tc + ((info->is_ocb_only == 1)?(8):(0));

        SOC_REG_ABOVE_64_CLEAR(map_data);
        /* set mapping from CAT2+TC to NIF PFC */
        rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
        DNXC_IF_ERR_EXIT(rv);

        SHR_BITCOPY_RANGE(map_data, (first_lane*8), &(info->cat2_tc_bitmap), 0, 8);

        rv = soc_mem_write(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
        DNXC_IF_ERR_EXIT(rv);

        /* enable map from CAT2+TC to NIF PFC */
        SOC_REG_ABOVE_64_CLEAR(cat2tc_map_ena);
        rv = soc_reg_above_64_get(unit, cat2tc_map_ena_reg, REG_PORT_ANY, 0, cat2tc_map_ena);
        DNXC_IF_ERR_EXIT(rv);
        
        SHR_BITSET(cat2tc_map_ena, bit_offset);
        
        rv = soc_reg_above_64_set(unit, cat2tc_map_ena_reg, REG_PORT_ANY, 0, cat2tc_map_ena);
        DNXC_IF_ERR_EXIT(rv);
    }
    /* Set the bit in the CAT2+TC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) } */

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  jer2_jer_fc_gen_inbnd_ll_set_unsafe(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  soc_port_t                    port,
    DNX_SAND_IN  DNX_TMC_FC_GEN_INBND_LL       *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        first_phy = 0, 
        first_lane = 0;
    uint32
        base_q_pair = 0;
    JER2_ARAD_EGQ_PPCT_TBL_DATA
        data;
    int
        core = 0;
    soc_reg_above_64_val_t 
        llfc_map;
    soc_reg_t 
        llfc_map_reg = INVALIDr;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(llfc_map);

    rv = dnx_port_sw_db_core_get(unit, port, &core);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy);
    DNXC_IF_ERR_EXIT(rv);
    rv = MBCM_DNX_DRIVER_CALL(unit,
            mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane));
    DNXC_IF_ERR_EXIT(rv);
    if (first_phy > 0) {
        first_phy--;  /* first_phy returned is one-based */
    }
    if (first_lane > 0) {
        first_lane--; /* first_lane returned is one-based */
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);

    /* Get PPCT data and update it */
    rv = jer2_arad_egq_ppct_tbl_get_unsafe(
              unit,
              core,
              base_q_pair,
              &data
              );
    DNXC_IF_ERR_EXIT(rv);

    data.cnm_intrcpt_fc_en = DNX_SAND_BOOL2NUM(info->cnm_enable);
    /* Map OTM-Port to the LLFC Channel (internal nif id) */
    data.cnm_intrcpt_fc_vec_llfc = first_phy; 

    rv = jer2_arad_egq_ppct_tbl_set_unsafe(
              unit,
              core,
              base_q_pair,
              &data
              );
    DNXC_IF_ERR_EXIT(rv);

    /* set mapping from global resource to LLFC { */
    if (info->gen_src ==  DNX_TMC_FC_GEN_SRC_GLB_HIGH) {
        if (info->is_ocb_only) {
            if (info->glbl_rcs_pool == 0) {
                llfc_map_reg = (core == 0) ? CFC_GLB_RSC_OCB_POOL_0_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_OCB_POOL_0_IQM_1_TO_NIF_LLFC_HP_MAPr;                    
            }
            else {
                llfc_map_reg = (core == 0) ? CFC_GLB_RSC_OCB_POOL_1_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_OCB_POOL_1_IQM_1_TO_NIF_LLFC_HP_MAPr;
            }
        }
        else {
            if(!SOC_IS_QAX(unit)) {
                if (info->glbl_rcs_pool == 0) {
                    llfc_map_reg = (core == 0) ? CFC_GLB_RSC_MIX_POOL_0_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_MIX_POOL_0_IQM_1_TO_NIF_LLFC_HP_MAPr;            
                }
                else {
                    llfc_map_reg = (core == 0) ? CFC_GLB_RSC_MIX_POOL_1_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_MIX_POOL_1_IQM_1_TO_NIF_LLFC_HP_MAPr;
                }
            }
        }

        if (llfc_map_reg != INVALIDr) {
            rv = soc_reg_above_64_get(unit, llfc_map_reg, REG_PORT_ANY, 0, llfc_map);
            DNXC_IF_ERR_EXIT(rv);
            if (info->glbl_rcs_enable) {
                SHR_BITSET(llfc_map, first_phy);
            }
            else {
                SHR_BITCLR(llfc_map, first_phy);
            }
            rv = soc_reg_above_64_set(unit, llfc_map_reg, REG_PORT_ANY, 0, llfc_map);
            DNXC_IF_ERR_EXIT(rv);
        }
    }
    /* set mapping from global resource to LLFC } */

    /* Set the bit in the MLF LLFC bitmap { */
    if (info->gen_src == DNX_TMC_FC_GEN_SRC_NIF) {

        llfc_map_reg = SOC_IS_QUX(unit) ? NIF_FC_TX_GEN_LLFC_FROM_QMLF_MASKr : NBIH_FC_TX_GEN_LLFC_FROM_QMLF_MASKr;

        rv = soc_reg_above_64_get(unit, llfc_map_reg, REG_PORT_ANY, 0, llfc_map);
        DNXC_IF_ERR_EXIT(rv);
        if (info->nif_enable) {
            SHR_BITSET(llfc_map, first_lane);
        }
        else {
            SHR_BITCLR(llfc_map, first_lane);
        }

        rv = soc_reg_above_64_set(unit, llfc_map_reg, REG_PORT_ANY, 0, llfc_map);
        DNXC_IF_ERR_EXIT(rv);

    }
    /* Set the bit in the MLF LLFC bitmap } */

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Inband Flow Control Generation Configuration, based on
 *     Arad Ingress state indications.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_inbnd_set(
    DNX_SAND_IN  int                                unit,
    DNX_SAND_IN  soc_port_t                         port,
    DNX_SAND_IN  DNX_TMC_FC_GEN_INBND_INFO          *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    DNX_TMC_FC_GEN_INBND_LL
        ll_disabled;
    DNX_TMC_FC_GEN_INBND_PFC
        pfc_disabled;
    DNX_TMC_FC_ENABLE_BITMAP
        cfc_enables;
  
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);


    DNX_TMC_FC_GEN_INBND_LL_clear(&ll_disabled);
    DNX_TMC_FC_GEN_INBND_PFC_clear(&pfc_disabled);
    pfc_disabled.inherit = DNX_TMC_FC_INBND_PFC_INHERIT_DISABLED;

    /* setting CFC enables - device level { */
    SHR_BITCLR_RANGE(cfc_enables.bmp, 0, DNX_TMC_FC_NOF_ENABLEs);
    
    switch (info->mode) {
    case DNX_TMC_FC_INBND_MODE_LL:
        if (info->ll.gen_src == DNX_TMC_FC_GEN_SRC_VSQ_LLFC) {
            SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_LLFC_VSQ_TO_NIF_EN);
        }
        else if (info->ll.gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH) {
            SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_NIF_LLFC_EN);
        }
        break;
    case DNX_TMC_FC_INBND_MODE_PFC:
    case DNX_TMC_FC_INBND_MODE_SAFC:
        if (info->pfc.gen_src == DNX_TMC_FC_GEN_SRC_VSQ_PFC) {
            SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_PFC_VSQ_TO_NIF_EN);
        }
        else if ((info->pfc.gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH) || (info->pfc.gen_src <= DNX_TMC_FC_GEN_SRC_GLB_LOW)) {
            SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_NIF_PFC_EN);
        }
        break;
    case DNX_TMC_FC_INBND_MODE_DEVICE_DISABLED:
        SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_LLFC_VSQ_TO_NIF_EN);
        SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_PFC_VSQ_TO_NIF_EN);
        SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_NIF_LLFC_EN); 
        SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_GLB_RSC_TO_NIF_PFC_EN);
        break;
    default:
    case DNX_TMC_FC_INBND_MODE_DISABLED:
        break;
    }

    rv = jer2_jer_fc_enables_set(unit, &cfc_enables, &cfc_enables);
    DNXC_IF_ERR_EXIT(rv);
    /* setting CFC enables - device level } */

    /* setting MAC registers - port level { */
    rv = jer2_jer_fc_inbnd_mode_set(unit, port, 1, info->mode);
    DNXC_IF_ERR_EXIT(rv);
    /* setting MAC registers - port level } */

    /* setting registers in CFC, NBI blocks { */
    switch (info->mode)
    {
    case DNX_TMC_FC_INBND_MODE_LL:
        /* Configure LL */
        rv = jer2_jer_fc_gen_inbnd_ll_set_unsafe(
            unit,
            port,
            &info->ll
          );
        DNXC_IF_ERR_EXIT(rv);
        break;
    case DNX_TMC_FC_INBND_MODE_PFC:
    case DNX_TMC_FC_INBND_MODE_SAFC:
        /* Configure PFC */
        rv = jer2_jer_fc_gen_inbnd_pfc_set_unsafe(
            unit,
            port,
            &info->pfc
          );
        DNXC_IF_ERR_EXIT(rv);
        break;
    case DNX_TMC_FC_INBND_MODE_DISABLED:
    default:
        break;
    }
    /* setting registers in CFC, NBI blocks } */

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  jer2_jer_fc_gen_inbnd_pfc_get_unsafe(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  soc_port_t                    port,
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_PFC      *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        first_phy = 0, 
        first_lane = 0;
    uint32
        reg_offset = 0,
        bit_offset = 0,
        fld_offset = 0;
    uint32
        base_q_pair = 0;
    JER2_ARAD_EGQ_PPCT_TBL_DATA
        data;
    uint32
        pfc_map_rx_mlf_reg = 0;
    int
        core = 0;
    soc_mem_t 
        mem= INVALIDm;
    soc_reg_above_64_val_t
        map_data;
    uint32
        entry_offset = 0; 
    soc_reg_above_64_val_t 
        cat2tc_map_ena_data;
    soc_reg_t 
        cat2tc_map_ena_reg = INVALIDr;
    int 
        ca2tc_map_ena = 0;
    soc_reg_t
        fc_pfc_qmlf_mask_reg = INVALIDr;

    DNXC_INIT_FUNC_DEFS;

    rv = dnx_port_sw_db_core_get(unit, port, &core);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy /*one based*/);
    DNXC_IF_ERR_EXIT(rv);

    rv = MBCM_DNX_DRIVER_CALL(unit,
            mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane));
    DNXC_IF_ERR_EXIT(rv);

    first_lane--;

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_egq_ppct_tbl_get_unsafe(
                    unit,
                    core,
                    base_q_pair,
                    &data
                    );
    DNXC_IF_ERR_EXIT(rv);
    info->cnm_intercept_enable = DNX_SAND_NUM2BOOL(data.cnm_intrcpt_fc_en);


    /* Get the bit in the GLB RSC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) { */
    if ((info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_LOW) || (info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH)) { 
        mem = (core == 0) ? CFC_GLB_RSC_IQM_0_MAPm : CFC_GLB_RSC_IQM_1_MAPm;
        entry_offset = ((info->is_ocb_only == 1)?(4):(0)) + 
                       ((info->glbl_rcs_pool == 1)?(2):(0)) + 
                       ((info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH)?(1):(0));
        entry_offset += GLB_RSC_MAP_TBL_NIF_PFC_OFFSET;

        /* get mapping from CAT2+TC to NIF PFC */
        rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
        DNXC_IF_ERR_EXIT(rv);

        if (info->gen_src == DNX_TMC_FC_GEN_SRC_GLB_HIGH) {
            SHR_BITCOPY_RANGE(&(info->glbl_rcs_high), 0, map_data, (first_lane*8), 8);
        }
        else {
            SHR_BITCOPY_RANGE(&(info->glbl_rcs_low), 0, map_data, (first_lane*8), 8);
        }
    }
    /* Get the bit in the GLB RSC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) } */

    /* Get the bit in the MLF PFC bitmap { */
    if (info->gen_src == DNX_TMC_FC_GEN_SRC_NIF) {
        reg_offset = first_lane / NUM_OF_LANES_IN_PM;
        fld_offset = first_lane % NUM_OF_LANES_IN_PM;

        fc_pfc_qmlf_mask_reg = SOC_IS_QUX(unit) ? NIF_FC_PFC_QMLF_MASKr : NBIH_FC_PFC_QMLF_MASKr;
        rv = soc_reg32_get(unit, fc_pfc_qmlf_mask_reg, REG_PORT_ANY, reg_offset, &pfc_map_rx_mlf_reg);
        DNXC_IF_ERR_EXIT(rv);

        info->nif_cls_bitmap = soc_reg_field_get(unit, fc_pfc_qmlf_mask_reg, pfc_map_rx_mlf_reg, rx_mlf_pfc_fld[fld_offset]);
    }
    /* Get the bit in the MLF PFC bitmap } */

    /* Get the bit in the CAT2+TC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) { */
    if (info->gen_src == DNX_TMC_FC_GEN_SRC_VSQ_CAT2TC) {
        mem = (core == 0) ? CFC_CAT_2_TC_IQM_0_MAP_NIFm : CFC_CAT_2_TC_IQM_1_MAP_NIFm;
        entry_offset = info->cat2_tc + ((info->is_ocb_only == 1)?(8):(0));

        cat2tc_map_ena_reg = CFC_CAT_2_TC_MAP_NIF_ENAr;
        bit_offset = info->cat2_tc + ((info->is_ocb_only == 1)?(8):(0));

        /* check if map from CAT2+TC to NIF PFC is enable */
        SOC_REG_ABOVE_64_CLEAR(cat2tc_map_ena_data);
        rv = soc_reg_above_64_get(unit, cat2tc_map_ena_reg, REG_PORT_ANY, 0, cat2tc_map_ena_data);
        DNXC_IF_ERR_EXIT(rv);
        
        ca2tc_map_ena = SHR_BITGET(cat2tc_map_ena_data, bit_offset);

        /* get mapping from CAT2+TC to NIF PFC */
        if (ca2tc_map_ena) {
            rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
            DNXC_IF_ERR_EXIT(rv);

            SHR_BITCOPY_RANGE(&(info->cat2_tc_bitmap), 0, map_data, (first_lane*8), 8);
        }
        else {
            info->cat2_tc_bitmap = 0;
        }
    }
    /* Get the bit in the CAT2+TC PFC bitmap that will be sent to the NIF (NIF_PORT x TC) } */

    info->inherit = DNX_TMC_FC_INBND_PFC_INHERIT_DISABLED;

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  jer2_jer_fc_gen_inbnd_ll_get_unsafe(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  soc_port_t                    port,
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_LL       *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        first_phy = 0, 
        first_lane = 0;    
    uint32
        base_q_pair = 0;
    JER2_ARAD_EGQ_PPCT_TBL_DATA
        data;
    int
        core = 0;
    soc_reg_above_64_val_t llfc_map;
    soc_reg_t llfc_map_reg = INVALIDr;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(llfc_map);    

    rv = dnx_port_sw_db_core_get(unit, port, &core);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy);
    DNXC_IF_ERR_EXIT(rv);
    rv = MBCM_DNX_DRIVER_CALL(unit,
            mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane));
    DNXC_IF_ERR_EXIT(rv);
    if (first_phy > 0) {
        first_phy--;  /* first_phy returned is one-based */
    }
    if (first_lane > 0) {
        first_lane--; /* first_lane returned is one-based */
    }

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_egq_ppct_tbl_get_unsafe(
                    unit,
                    core,
                    base_q_pair,
                    &data
                  );
    DNXC_IF_ERR_EXIT(rv);
    info->cnm_enable = DNX_SAND_NUM2BOOL(data.cnm_intrcpt_fc_en);

    /* check if mapping from global resource to LLFC is enable { */
    if (info->gen_src ==  DNX_TMC_FC_GEN_SRC_GLB_HIGH) {
        if (info->is_ocb_only) {
            if (info->glbl_rcs_pool == 0) {
                llfc_map_reg = (core == 0) ? CFC_GLB_RSC_OCB_POOL_0_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_OCB_POOL_0_IQM_1_TO_NIF_LLFC_HP_MAPr;                    
            }
            else {
                llfc_map_reg = (core == 0) ? CFC_GLB_RSC_OCB_POOL_1_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_OCB_POOL_1_IQM_1_TO_NIF_LLFC_HP_MAPr;
            }
        }
        else {
            if(!SOC_IS_QAX(unit)) {
                if (info->glbl_rcs_pool == 0) {
                    llfc_map_reg = (core == 0) ? CFC_GLB_RSC_MIX_POOL_0_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_MIX_POOL_0_IQM_1_TO_NIF_LLFC_HP_MAPr;            
                }
                else {
                    llfc_map_reg = (core == 0) ? CFC_GLB_RSC_MIX_POOL_1_IQM_0_TO_NIF_LLFC_HP_MAPr : CFC_GLB_RSC_MIX_POOL_1_IQM_1_TO_NIF_LLFC_HP_MAPr;
                }
            }
        }

        if (llfc_map_reg != INVALIDr) {
            rv = soc_reg_above_64_get(unit, llfc_map_reg, REG_PORT_ANY, 0, llfc_map);
            DNXC_IF_ERR_EXIT(rv);

            info->glbl_rcs_enable = SHR_BITGET(llfc_map, first_phy);
        }
    }
    /* check if mapping from global resource to LLFC is enable } */

    /* check if mapping from MLF to LLFC is enable { */
    if (info->gen_src == DNX_TMC_FC_GEN_SRC_NIF) {

        llfc_map_reg = SOC_IS_QUX(unit) ? NIF_FC_TX_GEN_LLFC_FROM_QMLF_MASKr : NBIH_FC_TX_GEN_LLFC_FROM_QMLF_MASKr;
        rv = soc_reg_above_64_get(unit, llfc_map_reg, REG_PORT_ANY, 0, llfc_map);

        DNXC_IF_ERR_EXIT(rv);

        info->nif_enable = SHR_BITGET(llfc_map, first_lane);
    }
    /* check if mapping from MLF to LLFC is enable } */

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Inband Flow Control Generation Configuration, based on
 *     Arad Ingress state indications.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_inbnd_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  soc_port_t                 port,
    DNX_SAND_OUT DNX_TMC_FC_GEN_INBND_INFO  *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;

    DNXC_INIT_FUNC_DEFS;

    /* Check which mode is enabled */
    rv = jer2_jer_fc_inbnd_mode_get(unit, port, 1, &(info->mode));
    DNXC_IF_ERR_EXIT(rv);
  
    /* Get the LL configuration */
    if (info->mode == DNX_TMC_FC_INBND_MODE_LL)
    {
        rv = jer2_jer_fc_gen_inbnd_ll_get_unsafe(
                unit,
                port,
                &info->ll
              );
        DNXC_IF_ERR_EXIT(rv);
    }

    /* Get the PFC configuration */
    if (info->mode == DNX_TMC_FC_INBND_MODE_PFC || info->mode == DNX_TMC_FC_INBND_MODE_SAFC)
    {
        rv = jer2_jer_fc_gen_inbnd_pfc_get_unsafe(
                unit,
                port,
                &info->pfc
              );
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Inband Flow Control Reception Configuration
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_rec_inbnd_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_INTERFACE_ID             nif_ndx,
    DNX_SAND_IN  DNX_TMC_FC_REC_INBND_INFO           *info
  )
{
    
    DNXC_INIT_FUNC_DEFS;

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    res = jer2_arad_nif_id_verify(
          nif_ndx
        );
    DNXC_SAND_IF_ERR_EXIT(res);

    if (JER2_ARAD_NIF_IS_TYPE_ID(ILKN, nif_ndx))
    {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
#endif 

    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_rec_inbnd_get_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_INTERFACE_ID             nif_ndx
  )
{

    DNXC_INIT_FUNC_DEFS;

    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    res = jer2_arad_nif_id_verify(
          nif_ndx
        );
    DNXC_SAND_IF_ERR_EXIT(res);

    if (JER2_ARAD_NIF_IS_TYPE_ID(ILKN, nif_ndx))
    {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }
#endif 

    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configures Calendar-based interface calendar for Flow
 *     Control Generation (OOB/ILKN-Inband TX).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_cal_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_IN  DNX_TMC_FC_GEN_CALENDAR             *cal_buff
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (cal_mode_ndx >= DNX_TMC_FC_NOF_CAL_MODES) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }
    else {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_oob_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_gen_cal_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_IN  DNX_TMC_FC_GEN_CALENDAR             *cal_buff
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        source_val = 0;
    uint32
        rep_idx = 0,      /* Calendar repetition index */
        per_rep_idx = 0,  /* Calendar entry index, per repetition */
        entry_idx = 0;    /* Calendar entry index, global */
    JER2_ARAD_CFC_CALTX_TBL_DATA
        tx_cal;
    uint64
        field64;
    uint32
        field32;
    soc_mem_t
        cal_table = INVALIDm;
    soc_reg_above_64_val_t 
        cal_table_data;
    soc_reg_t
        reg = INVALIDr;
    soc_field_t 
        fld = INVALIDf;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(cal_conf);
    DNXC_NULL_CHECK(cal_buff);

    rv = jer2_jer_fc_gen_cal_set_verify(unit, cal_mode_ndx, if_ndx, cal_conf, cal_buff);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(cal_table_data);

    rv = jer2_jer_fc_clear_calendar_unsafe(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_TX, if_ndx);
    DNXC_IF_ERR_EXIT(rv);

    /* Calendar entries { */
    for (per_rep_idx = 0; per_rep_idx < cal_conf->cal_len; per_rep_idx++)
    {
        /* Convert source enum to HW value */
        source_val = jer2_jer_fc_gen_cal_src_type_to_val_internal(cal_buff[per_rep_idx].source);

        if (source_val == -1)
        {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
        else
        {
            tx_cal.fc_source_sel = source_val;
            tx_cal.fc_index = cal_buff[per_rep_idx].id;
        }

        rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_TX, if_ndx, &cal_table);
        DNXC_IF_ERR_EXIT(rv);
        for (rep_idx = 0; rep_idx < cal_conf->cal_reps; rep_idx++)
        {
            entry_idx = (rep_idx * cal_conf->cal_len) + per_rep_idx;

            soc_mem_field32_set(unit, cal_table, cal_table_data, FC_INDEXf, tx_cal.fc_index);
            soc_mem_field32_set(unit, cal_table, cal_table_data, FC_SRC_SELf, tx_cal.fc_source_sel);
            
            rv = soc_mem_write(unit, cal_table, MEM_BLOCK_ANY, entry_idx, cal_table_data);
            DNXC_IF_ERR_EXIT(rv);
        }
    } /* Calendar entries } */

    /* Enable the FC after configuring it { */
    
    /* Select OOB Type 1'b1 - ILKN; 1'b0 - SPI4 / HCFC */
    if ((cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB) || (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB)) {
        reg = CFC_OOB_PAD_CONFIGURATIONr;
        fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? OOB_TX_0_MODEf : OOB_TX_1_MODEf);
        field32 = DNX_SAND_BOOL2NUM(cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB);       
        rv = soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, fld,  field32);
        DNXC_IF_ERR_EXIT(rv);
    }

    /* Enalbe SPI OOB */
    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB) {
        reg = CFC_SPI_OOB_TX_CONFIGURATIONr;        
        fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_ENf : SPI_OOB_TX_1_ENf);
        field32 = cal_conf->enable;
        rv = soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, fld , field32);
        DNXC_IF_ERR_EXIT(rv);
    }
    else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB || cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        reg = CFC_ILKN_TX_CONFIGURATIONr; 
		
        if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            /* Enable ILKN InBand */
            fld = ilkn_inb_tx_ena_flds[if_ndx];	
        }
        else {
            /* Enable ILKN OOB */
            fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_TX_0_ENf : ILKN_OOB_TX_1_ENf);    
        }
        field32 = cal_conf->enable; 
        rv = soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, fld,  field32);
        DNXC_IF_ERR_EXIT(rv);
    }
    else {
        DNXC_IF_ERR_EXIT(SOC_E_NONE);
    }

    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        fld = BLOCKS_SOFT_INIT_59f;
    } else {
        fld = BLOCKS_SOFT_INIT_55f;
    }
    COMPILER_64_SET(field64,0,0x1);
    rv = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);    /* Init block for CFC */
    DNXC_IF_ERR_EXIT(rv);
    COMPILER_64_SET(field64,0,0x0);
    rv = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);
    DNXC_IF_ERR_EXIT(rv);
    /* Enable the FC after configuring it } */

exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
  jer2_jer_fc_gen_cal_get_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (cal_mode_ndx >= DNX_TMC_FC_NOF_CAL_MODES) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }
    else {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_oob_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Configures Calendar-based interface calendar for Flow
 *     Control Generation (OOB/ILKN-Inband TX).
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_gen_cal_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_OUT DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_OUT DNX_TMC_FC_GEN_CALENDAR             *cal_buff
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        entry_idx = 0;    /* Calendar entry index, global */
    JER2_ARAD_CFC_CALTX_TBL_DATA
        tx_cal;
    soc_mem_t
        cal_table = INVALIDm;
    soc_reg_above_64_val_t
        cal_table_data;
    soc_reg_t
        en_reg = INVALIDr,
        cal_len_reg = INVALIDr,
        cal_rep_reg = INVALIDr;
    soc_field_t 
        en_fld = INVALIDf, 
        cal_len_fld = INVALIDf, 
        cal_rep_fld = INVALIDf;
    uint32
        en_fld_val = 0,
        cal_len_val = 0,
        cal_rep_val = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(cal_conf);
    DNXC_NULL_CHECK(cal_buff);

    rv = jer2_jer_fc_gen_cal_get_verify(unit, cal_mode_ndx, if_ndx);
    DNXC_IF_ERR_EXIT(rv);

    DNX_TMC_FC_CAL_IF_INFO_clear(cal_conf);
    DNX_TMC_FC_GEN_CALENDAR_clear(cal_buff);
    SOC_REG_ABOVE_64_CLEAR(cal_table_data);

    /* Calendar parameters { */
    /* Calendar Enable/disable { */
    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB){
        en_reg = CFC_SPI_OOB_TX_CONFIGURATIONr;
        en_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A) ? SPI_OOB_TX_0_ENf : SPI_OOB_TX_1_ENf;
        rv = soc_reg_above_64_field32_read(unit, en_reg, REG_PORT_ANY, 0, en_fld, &en_fld_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    else { /* ILKN */
        en_reg = CFC_ILKN_TX_CONFIGURATIONr;
        if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            en_fld = ilkn_inb_tx_ena_flds[if_ndx];
        }
        else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB) {
            en_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A) ? ILKN_OOB_TX_0_ENf : ILKN_OOB_TX_1_ENf;
        }
        rv = soc_reg_above_64_field32_read(unit, en_reg, REG_PORT_ANY, 0, en_fld, &en_fld_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    cal_conf->enable = DNX_SAND_NUM2BOOL(en_fld_val);
    /* Calendar Enable/disable } */

    /* Calendar length (single repetition) { */
    /* The len register should be set to LEN-1 */
    if(cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB)
    {
        cal_len_reg = CFC_SPI_OOB_TX_CONFIGURATIONr;
        cal_len_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_CAL_LENf : SPI_OOB_TX_1_CAL_LENf);
        rv = soc_reg_above_64_field32_read(unit, cal_len_reg, REG_PORT_ANY, 0, cal_len_fld, &cal_len_val);
        DNXC_IF_ERR_EXIT(rv);
        cal_conf->cal_len = cal_len_val;
    }
    else
    {
        cal_len_reg = CFC_ILKN_TX_CONFIGURATIONr;
        if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            cal_len_fld = ilkn_inb_tx_cal_len_flds[if_ndx];
        }
        else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB) {
            cal_len_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A) ? ILKN_OOB_TX_0_CAL_LENf : ILKN_OOB_TX_1_CAL_LENf;
        }

        rv = soc_reg_above_64_field32_read(unit, cal_len_reg, REG_PORT_ANY, 0, cal_len_fld, &cal_len_val);
        DNXC_IF_ERR_EXIT(rv);
        cal_conf->cal_len = cal_len_val + 1;
    }
    /* Calendar length (single repetition) } */

    /* Calendar number of repetitions - SPI/OOB Only { */
    if(cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB)
    {
        cal_rep_reg = CFC_SPI_OOB_TX_CONFIGURATIONr;
        cal_rep_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_TX_0_CAL_Mf : SPI_OOB_TX_1_CAL_Mf);

        rv = soc_reg_above_64_field32_read(unit, cal_rep_reg, REG_PORT_ANY, 0, cal_rep_fld, &cal_rep_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    else /* ILKN */
    {
        cal_rep_val = 1;
    }
    cal_conf->cal_reps = cal_rep_val;
    /* Calendar number of repetitions - SPI/OOB Only } */

    /* Calendar parameters } */

    /* Calendar entries { */

    rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_TX, if_ndx, &cal_table);
    DNXC_IF_ERR_EXIT(rv);
    
    for (entry_idx = 0; entry_idx < cal_conf->cal_len; entry_idx++)
    {
        rv = soc_mem_read(unit, cal_table, MEM_BLOCK_ANY, entry_idx, cal_table_data);
        DNXC_IF_ERR_EXIT(rv);

        tx_cal.fc_index = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_INDEXf);
        tx_cal.fc_source_sel = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_SRC_SELf);

        if(tx_cal.fc_source_sel >= JER2_JER_FC_GEN_CAL_SRC_ARR_SIZE)
        {
            cal_buff[entry_idx].source = DNX_TMC_FC_GEN_CAL_SRC_NONE;
            cal_buff[entry_idx].id = 0;
        }
        else
        {
            cal_buff[entry_idx].source = jer2_jer_fc_gen_cal_src_arr[tx_cal.fc_source_sel];
            cal_buff[entry_idx].id = tx_cal.fc_index;
        }
    }
    /* Calendar entries } */

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_clear_calendar_unsafe(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE              cal_mode_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_CAL_TYPE                 cal_type,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             if_ndx
  )
{
    uint32
        res = DNX_SAND_OK;
    soc_error_t
        rv = SOC_E_NONE;
    uint32 
        cal_rx_entry[128] = {0};
    uint32 
        cal_tx_entry[128] = {0};
    soc_mem_t
        cal_table = INVALIDm;
  
    DNXC_INIT_FUNC_DEFS;

    rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, cal_type, if_ndx, &cal_table);
    DNXC_IF_ERR_EXIT(rv);

    if(cal_type == JER2_ARAD_FC_CAL_TYPE_RX)
    {
        soc_mem_field32_set(unit, cal_table, cal_rx_entry, FC_DST_SELf, jer2_jer_fc_rec_cal_dest_type_to_val_internal(DNX_TMC_FC_REC_CAL_DEST_NONE));
        soc_mem_field32_set(unit, cal_table, cal_rx_entry, FC_INDEXf, 0x0);

        res = jer2_arad_fill_table_with_entry(unit, cal_table, MEM_BLOCK_ANY, cal_rx_entry); 
        DNXC_SAND_IF_ERR_EXIT(res);
    }
    
    if(cal_type == JER2_ARAD_FC_CAL_TYPE_TX)
    {
        soc_mem_field32_set(unit, cal_table, cal_tx_entry, FC_SRC_SELf, jer2_jer_fc_gen_cal_src_type_to_val_internal(DNX_TMC_FC_GEN_CAL_SRC_CONST));
        soc_mem_field32_set(unit, cal_table, cal_tx_entry, FC_INDEXf, 0x0);

        res = jer2_arad_fill_table_with_entry(unit, cal_table, MEM_BLOCK_ANY, cal_tx_entry); 
        DNXC_SAND_IF_ERR_EXIT(res);
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configures Calendar-based interface calendar for Flow
 *     Control Reception.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_rec_cal_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_IN  DNX_TMC_FC_REC_CALENDAR             *cal_buff
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (cal_mode_ndx >= DNX_TMC_FC_NOF_CAL_MODES) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }
    else {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_oob_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }
 
exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
  jer2_jer_fc_rec_cal_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_IN  DNX_TMC_FC_REC_CALENDAR             *cal_buff
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        dest_sel = 0;
    uint32
        rep_idx = 0,      /* Calendar repetition index */
        per_rep_idx = 0,  /* Calendar entry index, per repetition */
        entry_idx = 0;    /* Calendar entry index, global */
    JER2_ARAD_CFC_CALRX_TBL_DATA
        rx_cal;
    soc_mem_t
        cal_table = INVALIDm;
    soc_reg_above_64_val_t 
        cal_table_data;
    uint64
        field64;
    DNX_TMC_FC_ENABLE_BITMAP
        cfc_enables; 
    uint32
        field32;
    soc_reg_t
        reg = INVALIDr;
    soc_field_t 
        fld = INVALIDf;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(cal_conf);
    DNXC_NULL_CHECK(cal_buff);

    rv = jer2_jer_fc_rec_cal_set_verify(unit, cal_mode_ndx, if_ndx, cal_conf, cal_buff);
    DNXC_IF_ERR_EXIT(rv);

    /* Clear calendar */
    rv = jer2_jer_fc_clear_calendar_unsafe(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_RX, if_ndx);
    DNXC_IF_ERR_EXIT(rv);

    /* Calendar entries { */
    for (per_rep_idx = 0; per_rep_idx < cal_conf->cal_len; per_rep_idx++)
    {
        /* Convert destination enum to HW value */
        dest_sel = jer2_jer_fc_rec_cal_dest_type_to_val_internal(cal_buff[per_rep_idx].destination);

        if(dest_sel == -1)
        {
            DNXC_IF_ERR_EXIT(SOC_E_PARAM);
        }
        else if (cal_buff[per_rep_idx].destination == DNX_TMC_FC_REC_CAL_DEST_NONE)
        {
            rx_cal.fc_dest_sel = dest_sel;
            rx_cal.fc_index = 0;
        }
        else
        {
            rx_cal.fc_dest_sel = dest_sel;
            rx_cal.fc_index = cal_buff[per_rep_idx].id;
        }

        rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_RX, if_ndx, &cal_table);
        DNXC_IF_ERR_EXIT(rv);
        for (rep_idx = 0; rep_idx < cal_conf->cal_reps; rep_idx++)
        {
            entry_idx = (rep_idx * cal_conf->cal_len) + per_rep_idx;

            soc_mem_field32_set(unit, cal_table, cal_table_data, FC_INDEXf, rx_cal.fc_index);
            soc_mem_field32_set(unit, cal_table, cal_table_data, FC_DST_SELf, rx_cal.fc_dest_sel);
            
            rv = soc_mem_write(unit, cal_table, MEM_BLOCK_ANY, entry_idx, cal_table_data);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

    /* Enable the FC after configuring it { */

    /* Enalbe SPI OOB */
    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB) {
        reg = CFC_SPI_OOB_RX_CONFIGURATION_0r;
        fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_ENf : SPI_OOB_RX_1_ENf);
        field32 = cal_conf->enable;
        rv = soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, fld , field32);
        DNXC_IF_ERR_EXIT(rv);
    }
    else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB || cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        reg = CFC_ILKN_RX_CONFIGURATIONr; 
		
        if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            /* Enable ILKN InBand */
            fld = ilkn_inb_rx_ena_flds[if_ndx];	
        }
        else {
            /* Enable ILKN OOB */
            fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_ENf : ILKN_OOB_RX_1_ENf);    
        }
        field32 = cal_conf->enable; 
        rv = soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, fld,  field32);
        DNXC_IF_ERR_EXIT(rv);
	}
    else {
        DNXC_IF_ERR_EXIT(SOC_E_NONE);
    }

    /* Set CFC Enablers { */
    SHR_BITCLR_RANGE(((cfc_enables).bmp), 0, DNX_TMC_FC_NOF_ENABLEs);

    /* Always enable the CFC Enablers for ILKN , it's device-based */
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_ILKN_RX_TO_EGQ_PFC_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_ILKN_RX_TO_EGQ_PORT_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_ILKN_RX_TO_GEN_PFC_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_ILKN_RX_TO_NIF_FAST_LLFC_EN);

    /* Always enable the CFC Enablers for OOB , it's device-based */
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PFC_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_SPI_OOB_RX_TO_EGQ_PORT_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_SPI_OOB_RX_TO_GEN_PFC_EN);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_SPI_OOB_RX_TO_NIF_FAST_LLFC_EN);
    
    rv = jer2_jer_fc_enables_set(unit, &cfc_enables, &cfc_enables);
    DNXC_IF_ERR_EXIT(rv);
    /* Set CFC Enablers } */

    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        fld = BLOCKS_SOFT_INIT_59f;
    } else {
        fld = BLOCKS_SOFT_INIT_55f;
    }
    COMPILER_64_SET(field64,0, 0x1);
    rv = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);  /* Init block for CFC */
    DNXC_IF_ERR_EXIT(rv);
    COMPILER_64_SET(field64,0, 0x0);
    rv = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);
    DNXC_IF_ERR_EXIT(rv);

    /* Enable the FC after configuring it } */
exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
  jer2_jer_fc_rec_cal_get_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID                if_ndx
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (cal_mode_ndx >= DNX_TMC_FC_NOF_CAL_MODES) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }
    else {
        if (if_ndx >= SOC_DNX_CONFIG(unit)->tm.max_oob_ports) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("if_ndx is out of range")));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configures Calendar-based interface calendar for Flow
 *     Control Reception.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_rec_cal_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_MODE                 cal_mode_ndx,
    DNX_SAND_IN  DNX_TMC_FC_OOB_ID                   if_ndx,
    DNX_SAND_OUT DNX_TMC_FC_CAL_IF_INFO              *cal_conf,
    DNX_SAND_OUT DNX_TMC_FC_REC_CALENDAR             *cal_buff
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        entry_idx = 0;    /* Calendar entry index, global */
    JER2_ARAD_CFC_CALRX_TBL_DATA
        rx_cal;
    soc_mem_t
        cal_table = INVALIDm;
    soc_reg_above_64_val_t
        cal_table_data;
    soc_reg_t
        en_reg = INVALIDr,
        cal_len_reg = INVALIDr,
        cal_rep_reg = INVALIDr;        
    soc_field_t 
        en_fld = INVALIDf, 
        cal_len_fld = INVALIDf, 
        cal_rep_fld = INVALIDf;
    uint32
        en_fld_val = 0,
        cal_len_val = 0,
        cal_rep_val = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(cal_conf);
    DNXC_NULL_CHECK(cal_buff);

    rv = jer2_jer_fc_rec_cal_get_verify(unit, cal_mode_ndx, if_ndx);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(cal_table_data);

    /* Calendar parameters { */
    /* Calendar Enable/disable { */
    if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB){
        en_reg = CFC_SPI_OOB_RX_CONFIGURATION_0r;
        en_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A) ? SPI_OOB_RX_0_ENf : SPI_OOB_RX_1_ENf;
        rv = soc_reg_above_64_field32_read(unit, en_reg, REG_PORT_ANY, 0, en_fld, &en_fld_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    else { /* ILKN */
        en_reg = CFC_ILKN_RX_CONFIGURATIONr;
        if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND) {
            en_fld = ilkn_inb_rx_ena_flds[if_ndx];
        }
        else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB) {
            en_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A) ? ILKN_OOB_RX_0_ENf : ILKN_OOB_RX_1_ENf;
        }
        rv = soc_reg_above_64_field32_read(unit, en_reg, REG_PORT_ANY, 0, en_fld, &en_fld_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    cal_conf->enable = DNX_SAND_NUM2BOOL(en_fld_val);
    /* Calendar Enable/disable } */

    /* Calendar length (single repetition) { */
    /* The len register should be set to LEN-1 */
    if(cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB)
    {
        cal_len_reg = CFC_SPI_OOB_RX_CONFIGURATION_0r;
        cal_len_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_CAL_LENf : SPI_OOB_RX_1_CAL_LENf);
        rv = soc_reg_above_64_field32_read(unit, cal_len_reg, REG_PORT_ANY, 0, cal_len_fld, &cal_len_val);
        DNXC_IF_ERR_EXIT(rv);
        cal_conf->cal_len = cal_len_val;
    }
    else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_OOB)
    {
        cal_len_reg = CFC_ILKN_RX_CONFIGURATIONr;
        cal_len_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? ILKN_OOB_RX_0_CAL_LENf : ILKN_OOB_RX_1_CAL_LENf);
        rv = soc_reg_above_64_field32_read(unit, cal_len_reg, REG_PORT_ANY, 0, cal_len_fld, &cal_len_val);
        DNXC_IF_ERR_EXIT(rv);
        cal_conf->cal_len = cal_len_val+1;
    }
    else if (cal_mode_ndx == DNX_TMC_FC_CAL_MODE_ILKN_INBND)
    {
        cal_len_reg = NBIH_FC_ILKN_RX_CHFC_CAL_LENr;
        cal_len_fld = ilkn_inb_rx_cal_len_flds[if_ndx];
        rv = soc_reg_above_64_field32_read(unit, cal_len_reg, REG_PORT_ANY, 0, cal_len_fld, &cal_len_val);
        DNXC_IF_ERR_EXIT(rv);
        cal_conf->cal_len = cal_len_val;
    }
    /* Calendar length (single repetition) } */

    /* Calendar number of repetitions - SPI/OOB Only { */
    if(cal_mode_ndx == DNX_TMC_FC_CAL_MODE_SPI_OOB)
    {
        cal_rep_reg = CFC_SPI_OOB_RX_CONFIGURATION_0r;
        cal_rep_fld = (if_ndx == DNX_TMC_FC_OOB_ID_A ? SPI_OOB_RX_0_CAL_Mf : SPI_OOB_RX_1_CAL_Mf);

        rv = soc_reg_above_64_field32_read(unit, cal_rep_reg, REG_PORT_ANY, 0, cal_rep_fld, &cal_rep_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    else /* ILKN */
    {
        cal_rep_val = 1;
    }
    cal_conf->cal_reps = cal_rep_val;
    /* Calendar number of repetitions - SPI/OOB Only } */
  
    /* Calendar parameters } */

    /* Calendar entries { */
    rv = jer2_jer_fc_cal_tbl_get(unit, cal_mode_ndx, JER2_ARAD_FC_CAL_TYPE_RX, if_ndx, &cal_table);
    DNXC_IF_ERR_EXIT(rv);
    for (entry_idx = 0; entry_idx < cal_conf->cal_len; entry_idx++)
    {
        rv = soc_mem_read(unit, cal_table, MEM_BLOCK_ANY, entry_idx, cal_table_data);
        DNXC_IF_ERR_EXIT(rv);

        rx_cal.fc_index = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_INDEXf);
        rx_cal.fc_dest_sel = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_DST_SELf);

        if(rx_cal.fc_dest_sel >= JER2_JER_FC_REC_CAL_DEST_ARR_SIZE)
        {
            cal_buff[entry_idx].destination = DNX_TMC_FC_REC_CAL_DEST_NONE;
            cal_buff[entry_idx].id = 0;
        }
        else
        {
            cal_buff[entry_idx].destination = jer2_jer_fc_rec_cal_dest_arr[rx_cal.fc_dest_sel];
            cal_buff[entry_idx].id = rx_cal.fc_index;
        }
    }
    /* Calendar entries } */

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Defines if and how LLFC can be received/generated using
 *     Interlaken NIF.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_ilkn_llfc_set_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID          ilkn_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_DIRECTION             direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_ILKN_LLFC_INFO           *info
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (ilkn_ndx >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ilkn_ndx is out of range")));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_llfc_set(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID          ilkn_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_DIRECTION             direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FC_ILKN_LLFC_INFO        *info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        ilkn_bit_offset;
    soc_reg_t
        ilkn_llfc_reg = INVALIDr,
        multiple_use_bits_reg = INVALIDr;
    soc_field_t 
        on_ch0_fld = INVALIDf, 
        every_16_chs_fld = INVALIDf,
        multiple_use_bits_fld = INVALIDf;
    uint32
        buffer;
    uint64
        field64;
    int reg_arr_index;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);
    rv = jer2_jer_fc_ilkn_llfc_set_verify(unit, ilkn_ndx, direction_ndx, info);
    DNXC_IF_ERR_EXIT(rv);

    ilkn_llfc_reg = NBIH_FC_ILKNr;
    multiple_use_bits_reg = NBIH_ILKN_MULTIPLE_USE_BITSr;
    if (JER2_ARAD_IS_DIRECTION_REC(direction_ndx)) {
        multiple_use_bits_fld = ILKN_RX_N_LLFC_STOP_TX_FROM_MULTIPLE_USE_BITS_MASKf;
    }
    else {
        multiple_use_bits_fld = ILKN_TX_N_MULTIPLE_USE_BITS_FROM_LLFC_MASKf;
    }
    reg_arr_index = ilkn_ndx;

    if (JER2_ARAD_IS_DIRECTION_REC(direction_ndx)) {
        on_ch0_fld                 = FC_ILKN_RX_LLFC_ON_CH_0f;
        every_16_chs_fld           = FC_ILKN_RX_LLFC_EVERY_16_CHSf;
    }
    else{
        on_ch0_fld                 = FC_ILKN_TX_GEN_LLFC_ON_CH_0f;
        every_16_chs_fld           = FC_ILKN_TX_GEN_LLFC_EVERY_16_CHSf;
    }

    ilkn_bit_offset = ilkn_ndx;
    rv = soc_reg_above_64_field32_read(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, on_ch0_fld, &buffer);
    DNXC_IF_ERR_EXIT(rv);
    buffer &= DNX_SAND_RBIT(ilkn_bit_offset);
    buffer |= DNX_SAND_SET_FLD_IN_PLACE(
                  info->cal_channel == DNX_TMC_FC_ILKN_CAL_LLFC_CH_0 ? 1 : 0,
                  ilkn_bit_offset,
                  DNX_SAND_BIT(ilkn_bit_offset)
                );
    rv = soc_reg_above_64_field32_modify(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, on_ch0_fld,  buffer);
    DNXC_IF_ERR_EXIT(rv);

    rv = soc_reg_above_64_field32_read(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, every_16_chs_fld, &buffer);
    DNXC_IF_ERR_EXIT(rv);
    buffer &= DNX_SAND_RBIT(ilkn_bit_offset);
    buffer |= DNX_SAND_SET_FLD_IN_PLACE(
                  info->cal_channel == DNX_TMC_FC_ILKN_CAL_LLFC_CH_16N ? 1 : 0,
                  ilkn_bit_offset,
                  DNX_SAND_BIT(ilkn_bit_offset)
                );
    rv = soc_reg_above_64_field32_modify(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, every_16_chs_fld,  buffer);
    DNXC_IF_ERR_EXIT(rv);
    
    buffer = info->multi_use_mask;
    COMPILER_64_SET(field64,0, buffer);
    rv = soc_reg_above_64_field64_modify(unit, multiple_use_bits_reg, REG_PORT_ANY, reg_arr_index, multiple_use_bits_fld,  field64);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_llfc_get_verify(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (ilkn_ndx >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ilkn_ndx is out of range")));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Defines if and how LLFC can be received/generated using
 *     Interlaken NIF.
 *     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
  jer2_jer_fc_ilkn_llfc_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_LLFC_INFO           *rec_info,
    DNX_SAND_OUT JER2_ARAD_FC_ILKN_LLFC_INFO           *gen_info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        ilkn_bit_offset;
    uint32
        buffer;
    soc_reg_t
        ilkn_llfc_reg = INVALIDr,
        multiple_use_bits_reg = INVALIDr;
    soc_field_t 
        rx_on_ch0_fld = INVALIDf, 
        rx_every_16_chs_fld = INVALIDf,
        rx_multiple_use_bits_fld = INVALIDf,
        tx_on_ch0_fld = INVALIDf, 
        tx_every_16_chs_fld = INVALIDf,
        tx_multiple_use_bits_fld = INVALIDf;
    int reg_arr_index;

    DNXC_INIT_FUNC_DEFS;
    
    DNXC_NULL_CHECK(rec_info);
    DNXC_NULL_CHECK(gen_info);

    rv = jer2_jer_fc_ilkn_llfc_get_verify(unit, ilkn_ndx);
    DNXC_IF_ERR_EXIT(rv);

    DNX_TMC_FC_ILKN_LLFC_INFO_clear(rec_info);
    DNX_TMC_FC_ILKN_LLFC_INFO_clear(gen_info);

    ilkn_llfc_reg = NBIH_FC_ILKNr;
    multiple_use_bits_reg = NBIH_ILKN_MULTIPLE_USE_BITSr;
    rx_multiple_use_bits_fld = ILKN_RX_N_LLFC_STOP_TX_FROM_MULTIPLE_USE_BITS_MASKf;
    tx_multiple_use_bits_fld = ILKN_TX_N_MULTIPLE_USE_BITS_FROM_LLFC_MASKf;
    
    rx_on_ch0_fld                 = FC_ILKN_RX_LLFC_ON_CH_0f;
    rx_every_16_chs_fld           = FC_ILKN_RX_LLFC_EVERY_16_CHSf;
    tx_on_ch0_fld                 = FC_ILKN_TX_GEN_LLFC_ON_CH_0f;
    tx_every_16_chs_fld           = FC_ILKN_TX_GEN_LLFC_EVERY_16_CHSf;

    reg_arr_index = ilkn_ndx;
    ilkn_bit_offset = ilkn_ndx;

    /*
     *  Determine Rx type
     */
    rv = soc_reg_above_64_field32_read(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, rx_on_ch0_fld, &buffer);
    DNXC_IF_ERR_EXIT(rv);
    if (DNX_SAND_GET_FLD_FROM_PLACE(buffer, ilkn_bit_offset, DNX_SAND_BIT(ilkn_bit_offset)) == 1)
    {
        rec_info->cal_channel = DNX_TMC_FC_ILKN_CAL_LLFC_CH_0;
    }
    else
    {
        rv = soc_reg_above_64_field32_read(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, rx_every_16_chs_fld, &buffer);
        DNXC_IF_ERR_EXIT(rv);
        if (DNX_SAND_GET_FLD_FROM_PLACE(buffer, ilkn_bit_offset, DNX_SAND_BIT(ilkn_bit_offset)) == 1)
        {
            rec_info->cal_channel = DNX_TMC_FC_ILKN_CAL_LLFC_CH_16N;
        }
        else
        {
            rec_info->cal_channel = DNX_TMC_FC_ILKN_CAL_LLFC_NONE;
        }
    }

    /*
     *  Determine Tx type
     */
    rv = soc_reg_above_64_field32_read(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, tx_on_ch0_fld, &buffer);
    DNXC_IF_ERR_EXIT(rv);
    if (DNX_SAND_GET_FLD_FROM_PLACE(buffer, ilkn_bit_offset, DNX_SAND_BIT(ilkn_bit_offset)) == 1)
    {
        gen_info->cal_channel = DNX_TMC_FC_ILKN_CAL_LLFC_CH_0;
    }
    else
    {
        rv = soc_reg_above_64_field32_read(unit, ilkn_llfc_reg, REG_PORT_ANY, 0, tx_every_16_chs_fld, &buffer);
        DNXC_IF_ERR_EXIT(rv);
        if (DNX_SAND_GET_FLD_FROM_PLACE(buffer, ilkn_bit_offset, DNX_SAND_BIT(ilkn_bit_offset)) == 1)
        {
            gen_info->cal_channel = DNX_TMC_FC_ILKN_CAL_LLFC_CH_16N;
        }
        else
        {
            gen_info->cal_channel = DNX_TMC_FC_ILKN_CAL_LLFC_NONE;
        }
    }

    rv = soc_reg_above_64_field32_read(unit, multiple_use_bits_reg, REG_PORT_ANY, reg_arr_index, rx_multiple_use_bits_fld, &buffer);
    DNXC_IF_ERR_EXIT(rv);
    rec_info->multi_use_mask = (uint8) buffer;
    rv = soc_reg_above_64_field32_read(unit, multiple_use_bits_reg, REG_PORT_ANY, reg_arr_index, tx_multiple_use_bits_fld, &buffer);
    DNXC_IF_ERR_EXIT(rv);
    gen_info->multi_use_mask = (uint8) buffer;

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_mub_channel_verify(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID           ilkn_ndx
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (ilkn_ndx >  SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ilkn_ndx is out of range")));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_mub_channel_set(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID           ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_DIRECTION           direction_ndx,
    DNX_SAND_OUT uint8                          bitmap
  )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_reg_t
        reg = CFC_ILKN_MUB_ENABLEr;
    soc_field_t fld;
    uint32 reg_data[1];

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_ilkn_mub_channel_verify(unit, ilkn_ndx);
    DNXC_IF_ERR_EXIT(res);

    /* Enable mapping to/from the ILKN Multi-USE-Bits */
    fld = (direction_ndx == DNX_TMC_FC_DIRECTION_GEN) ? tx_flds[ilkn_ndx] : rx_flds[ilkn_ndx];

    res = soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, fld,  bitmap);
    DNXC_IF_ERR_EXIT(res);

    /* Enable/disable Multi-USE-Bits to use GenericBitmapsNif */
    if(!SOC_IS_QAX(unit)) {
        if (direction_ndx == DNX_TMC_FC_DIRECTION_REC) {
            reg = CFC_ILKN_MUB_RX_IF_ENAr;
            res = soc_reg32_get(unit, reg, REG_PORT_ANY, 0, reg_data);
            DNXC_IF_ERR_EXIT(res);

            if (bitmap != 0x0) {
                SHR_BITSET(reg_data, ilkn_ndx);
            } else {
               SHR_BITCLR(reg_data, ilkn_ndx);
            }

            res = soc_reg32_set(unit, reg, REG_PORT_ANY, 0, *reg_data);
            DNXC_IF_ERR_EXIT(res);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_mub_channel_get(
    DNX_SAND_IN  int                            unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID           ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_DIRECTION           direction_ndx,
    DNX_SAND_OUT uint8                          *bitmap
  )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_reg_t 
        reg = CFC_ILKN_MUB_ENABLEr;
    soc_field_t fld;
    soc_reg_above_64_val_t reg_data_64;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(bitmap);

    res = jer2_jer_fc_ilkn_mub_channel_verify(unit, ilkn_ndx);
    DNXC_IF_ERR_EXIT(res);

    SOC_REG_ABOVE_64_CLEAR(reg_data_64);
    /* get mapping to/from the ILKN Multi-USE-Bits */
    fld = (direction_ndx == DNX_TMC_FC_DIRECTION_GEN) ? tx_flds[ilkn_ndx] : rx_flds[ilkn_ndx];

    res = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data_64);
    DNXC_IF_ERR_EXIT(res);

    (*bitmap) = soc_reg_above_64_field32_get(unit, reg, reg_data_64, fld);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_mub_gen_cal_verify(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (ilkn_ndx > SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ilkn_ndx is out of range")));
    }

    DNXC_NULL_CHECK(cal_info);
 
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_mub_gen_cal_set (
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_IN  DNX_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_reg_above_64_val_t  
        cal_data, entry_data;
    uint32
        entry_idx;
    uint32
        ilkn_id_offset = ilkn_ndx;
    uint32
        source_fld_offset = 0;
    uint64
        field64;
    soc_field_t
        fld = INVALIDf;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_ilkn_mub_gen_cal_verify(unit, ilkn_ndx, cal_info);
    DNXC_IF_ERR_EXIT(res);
    
    SOC_REG_ABOVE_64_CLEAR(cal_data);

    source_fld_offset = soc_mem_field_length(unit, CFC_OOB_TX_0_CALm, FC_INDEXf);
    if (source_fld_offset == 0) {
        DNXC_IF_ERR_EXIT(SOC_E_UNAVAIL);
    }

    /* Set calendar data */
    for(entry_idx = 0; entry_idx < DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN; entry_idx++)
    {
        /* init */
        SOC_REG_ABOVE_64_CLEAR(entry_data);
        /* set source (bits 11:9) */
        entry_data[0] = jer2_jer_fc_gen_cal_src_type_to_val_internal(cal_info->entries[entry_idx].source) << source_fld_offset;
        if(source_fld_offset > 0) {
            /* set index (bits 8:0) */
            entry_data[0] |= (cal_info->entries[entry_idx].id & DNX_SAND_BITS_MASK(source_fld_offset - 1, 0));
        }

        /* update the calendar data with the entry*/
        if(ilkn_ndx == 0) {
            soc_reg_above_64_field_set(unit, calendar_regs[ilkn_id_offset], cal_data, fields_0[entry_idx], entry_data);
        } else if(ilkn_ndx == 1){
            soc_reg_above_64_field_set(unit, calendar_regs[ilkn_id_offset], cal_data, fields_1[entry_idx], entry_data);
        } else if(ilkn_ndx == 2){
            soc_reg_above_64_field_set(unit, calendar_regs[ilkn_id_offset], cal_data, fields_2[entry_idx], entry_data);
        } else if(ilkn_ndx == 3){
            soc_reg_above_64_field_set(unit, calendar_regs[ilkn_id_offset], cal_data, fields_3[entry_idx], entry_data);
        } else if(ilkn_ndx == 4){
            soc_reg_above_64_field_set(unit, calendar_regs[ilkn_id_offset], cal_data, fields_4[entry_idx], entry_data);
        } else if(ilkn_ndx == 5){
            soc_reg_above_64_field_set(unit, calendar_regs[ilkn_id_offset], cal_data, fields_5[entry_idx], entry_data);
        }
    }
  
    /* Write the calendar */
    res = soc_reg_above_64_set(unit, calendar_regs[ilkn_id_offset], REG_PORT_ANY, 0, cal_data);
    DNXC_IF_ERR_EXIT(res);

    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        fld = BLOCKS_SOFT_INIT_59f;
    } else {
        fld = BLOCKS_SOFT_INIT_55f;
    }
    COMPILER_64_SET(field64,0, 0x1);
    res = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);  /* Init block for CFC */
    DNXC_IF_ERR_EXIT(res);
    COMPILER_64_SET(field64,0, 0x0);
    res = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_ilkn_mub_gen_cal_get(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  DNX_TMC_FC_CAL_IF_ID             ilkn_ndx,
    DNX_SAND_OUT DNX_TMC_FC_ILKN_MUB_GEN_CAL      *cal_info
  )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_reg_above_64_val_t  
        cal_data, entry_data;
    uint32
        entry_idx;
    uint32
        ilkn_id_offset = ilkn_ndx;
    uint32
        source_fld_offset = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_ilkn_mub_gen_cal_verify(unit, ilkn_ndx, cal_info);
    DNXC_IF_ERR_EXIT(res);

    SOC_REG_ABOVE_64_CLEAR(cal_data);

    source_fld_offset = soc_mem_field_length(unit, CFC_OOB_TX_0_CALm, FC_INDEXf);
    if (source_fld_offset == 0) {
        DNXC_IF_ERR_EXIT(SOC_E_UNAVAIL);
    }

    /* Read the calendar */
    res = soc_reg_above_64_get(unit, calendar_regs[ilkn_id_offset], REG_PORT_ANY, 0, cal_data);
    DNXC_IF_ERR_EXIT(res);

    /* Get calendar data */
    for(entry_idx = 0; entry_idx < DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN; entry_idx++)
    {
        /* init */
        SOC_REG_ABOVE_64_CLEAR(entry_data);
        if(ilkn_ndx == 0) {
            soc_reg_above_64_field_get(unit, calendar_regs[ilkn_id_offset], cal_data, fields_0[entry_idx], entry_data);
        } else if(ilkn_ndx == 1){
            soc_reg_above_64_field_get(unit, calendar_regs[ilkn_id_offset], cal_data, fields_1[entry_idx], entry_data);
        } else if(ilkn_ndx == 2){
            soc_reg_above_64_field_get(unit, calendar_regs[ilkn_id_offset], cal_data, fields_2[entry_idx], entry_data);
        } else if(ilkn_ndx == 3){
            soc_reg_above_64_field_get(unit, calendar_regs[ilkn_id_offset], cal_data, fields_3[entry_idx], entry_data);
        } else if(ilkn_ndx == 4){
            soc_reg_above_64_field_get(unit, calendar_regs[ilkn_id_offset], cal_data, fields_4[entry_idx], entry_data);
        } else if(ilkn_ndx == 5){
            soc_reg_above_64_field_get(unit, calendar_regs[ilkn_id_offset], cal_data, fields_5[entry_idx], entry_data);
        }

        /* get source (bits 11:9) */
        cal_info->entries[entry_idx].source = jer2_jer_fc_gen_cal_src_arr[entry_data[0] >> source_fld_offset];

        /* get index (bits 8:0) */
        cal_info->entries[entry_idx].id = (entry_data[0] & DNX_SAND_BITS_MASK(source_fld_offset-1, 0));
    }
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_pfc_generic_bitmap_verify(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index
  )
{
    DNXC_INIT_FUNC_DEFS;

    if ((priority < 0) || (priority > DNX_TMC_EGR_NOF_Q_PRIO_JER2_ARAD - 1)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if(pfc_bitmap_index >= SOC_DNX_DEFS_GET(unit, nof_fc_pfc_generic_bitmaps)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_pfc_generic_bitmap_set(
    DNX_SAND_IN   int                              unit,
    DNX_SAND_IN   int                              priority,
    DNX_SAND_IN   uint32                           pfc_bitmap_index,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_reg_above_64_val_t  
        pfc_map_data;
    uint32 
        entry_offset = 0;
    uint64
        field64 = COMPILER_64_INIT(0, 0);
    soc_field_t
        fld = INVALIDf;

    DNXC_INIT_FUNC_DEFS;

    rv = jer2_jer_fc_pfc_generic_bitmap_verify(unit, priority, pfc_bitmap_index);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(pfc_map_data);

    entry_offset = pfc_bitmap_index;

    /* set Generic Bitmap */
    rv = soc_mem_read(unit, GENERIC_BITMAP_NIF_MAP[priority], MEM_BLOCK_ANY, entry_offset, pfc_map_data);
    DNXC_IF_ERR_EXIT(rv);

    /* Update the Generic Bitmap */
    SHR_BITCOPY_RANGE(pfc_map_data, 0, pfc_bitmap->bitmap, 0, DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE);
    if (pfc_bitmap->core) {
        SHR_BITSET(pfc_map_data, DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE);
    }
    else {
        SHR_BITCLR(pfc_map_data, DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE);
    }

    rv = soc_mem_write(unit, GENERIC_BITMAP_NIF_MAP[priority], MEM_BLOCK_ANY, entry_offset, pfc_map_data);
    DNXC_IF_ERR_EXIT(rv);

    /* Init block for CFC */
    if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        fld = BLOCKS_SOFT_INIT_59f;
    } else {
        fld = BLOCKS_SOFT_INIT_55f;
    }
    COMPILER_64_SET(field64, 0, 0x1);
    rv = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);  
    DNXC_IF_ERR_EXIT(rv);
    COMPILER_64_SET(field64, 0, 0x0);
    rv = soc_reg_above_64_field64_modify(unit, ECI_BLOCKS_SOFT_INITr, REG_PORT_ANY, 0, fld,  field64);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_pfc_generic_bitmap_get(
    DNX_SAND_IN   int   unit,
    DNX_SAND_IN   int       priority,
    DNX_SAND_IN   uint32    pfc_bitmap_index,
    DNX_SAND_OUT   DNX_TMC_FC_PFC_GENERIC_BITMAP    *pfc_bitmap
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_reg_above_64_val_t  
        pfc_map_data;
    uint32 entry_offset = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(pfc_bitmap);
    rv = jer2_jer_fc_pfc_generic_bitmap_verify(unit, priority, pfc_bitmap_index);
    DNXC_IF_ERR_EXIT(rv);

    SOC_REG_ABOVE_64_CLEAR(pfc_map_data);

    entry_offset = pfc_bitmap_index;

    /* Read Generic Bitmap */
    rv = soc_mem_read(unit, GENERIC_BITMAP_NIF_MAP[priority], MEM_BLOCK_ANY, entry_offset, pfc_map_data);
    DNXC_IF_ERR_EXIT(rv);

    /* Get the Generic Bitmap */
    SHR_BITCOPY_RANGE(pfc_bitmap->bitmap, 0, pfc_map_data, 0, DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE);
    pfc_bitmap->core = SHR_BITGET(pfc_map_data, DNX_TMC_FC_PFC_GENERIC_BITMAP_SIZE);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
jer2_jer_fc_pfc_generic_bitmap_valid_update(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GEN_BMP_SRC_TYPE   src_type,
    DNX_SAND_IN   int                               priority,
    DNX_SAND_OUT  uint32                           *is_valid)
{
    soc_error_t
        rv = SOC_E_NONE;
    DNX_TMC_FC_PFC_GEN_BMP_INFO
        pfc_gen_bmp_info;
    int
        i = 0,
        is_nif_valid = 0,
        is_cal_valid = 0,
        is_nif_to_gen_pfc_ena = 0;
    DNX_TMC_FC_ENABLE_BITMAP
        cfc_enables,
        cfc_enables_info;
    uint32
        reg_data[1];

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(is_valid);

    rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.get(unit, &pfc_gen_bmp_info);
    DNXC_IF_ERR_EXIT(rv);

    /*
     * For Jericho, there are 8 GenericBitmapsNif[0..7] Maps.
     * 1. When NifToGenPfcEn is set, then GenericBitmapsNif[0..7]Map are used by NIF Rx PFC flow controls.
     * 2. When NifToGenPfcEn is 0, then the GenericBitmapsNif[0..7]Map are used by calendar Rx flow controls.
     *    2.1 GenericBitmapsNif[6..7]Map memories are used by the ILKN Out-of-band flow control interfaces.
     *    2.2 If IlknMubRxIfEna bit N[0..5] enabled, Multi-use bits is using GenericBitmapsNif[N]map,
     *        else the ILKN In-band is using this memory.
     */

    switch(src_type)
    {
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_NIF:
        is_nif_valid = TRUE;
        for (i = 0; i < 7; i++) {
            if (pfc_gen_bmp_info.cal_pfc_gen_bmp_used[i] != 0) {
                is_nif_valid = FALSE;
                break;
            }
        }
        if (is_nif_valid == TRUE) {
            is_nif_to_gen_pfc_ena = TRUE;
        }
        *is_valid = is_nif_valid;
        break;
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_OOB:
        is_cal_valid = TRUE;
        for (i = 0; i < 7; i++) {
            if (pfc_gen_bmp_info.nif_pfc_gen_bmp_used[i] != 0) {
                is_cal_valid = FALSE;
                break;
            }
        }
        if (is_cal_valid == TRUE) {
            is_nif_to_gen_pfc_ena = FALSE;
        }
        *is_valid = is_cal_valid;
        break;
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_ILKN_INB:
        is_cal_valid = TRUE;
        for (i = 0; i < 7; i++) {
            if (pfc_gen_bmp_info.nif_pfc_gen_bmp_used[i] != 0) {
                is_cal_valid = FALSE;
                break;
            }
        }
        if (is_cal_valid == TRUE) {
            is_nif_to_gen_pfc_ena = FALSE;

            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, CFC_ILKN_MUB_RX_IF_ENAr, REG_PORT_ANY, 0, reg_data));
            if (pfc_gen_bmp_info.mub_pfc_gen_bmp_used[priority] != 0) {
                *is_valid = FALSE;
                SHR_BITSET(reg_data, priority);
            }
            else {
                *is_valid = TRUE;
                SHR_BITCLR(reg_data, priority);
            }
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, CFC_ILKN_MUB_RX_IF_ENAr, REG_PORT_ANY, 0, *reg_data));
        }
        break;
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_MUB:
        is_cal_valid = TRUE;
        for (i = 0; i < 7; i++) {
            if (pfc_gen_bmp_info.nif_pfc_gen_bmp_used[i] != 0) {
                is_cal_valid = FALSE;
                break;
            }
        }
        if (is_cal_valid == TRUE) {
            is_nif_to_gen_pfc_ena = FALSE;

            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, CFC_ILKN_MUB_RX_IF_ENAr, REG_PORT_ANY, 0, reg_data));
            if (pfc_gen_bmp_info.ilkn_inb_pfc_gen_bmp_used[priority] != 0) {
                *is_valid = FALSE;
                SHR_BITCLR(reg_data, priority);
            }
            else {
                *is_valid = TRUE;
                SHR_BITSET(reg_data, priority);
            }
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, CFC_ILKN_MUB_RX_IF_ENAr, REG_PORT_ANY, 0, *reg_data));
        }
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support src_type(%d)\n"), src_type));
        break;
    }

    SHR_BITCLR_RANGE(((cfc_enables).bmp), 0, DNX_TMC_FC_NOF_ENABLEs);
    SHR_BITCLR_RANGE(((cfc_enables_info).bmp), 0, DNX_TMC_FC_NOF_ENABLEs);
    SHR_BITSET(cfc_enables.bmp, DNX_TMC_FC_NIF_TO_GEN_PFC_EN);
    if (is_nif_to_gen_pfc_ena) {
        SHR_BITSET(cfc_enables_info.bmp, DNX_TMC_FC_NIF_TO_GEN_PFC_EN);
    }
    rv = jer2_jer_fc_enables_set(unit, &cfc_enables, &cfc_enables_info);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
jer2_jer_fc_pfc_generic_bitmap_used_update(
    DNX_SAND_IN   int                               unit,
    DNX_SAND_IN   DNX_TMC_FC_PFC_GEN_BMP_SRC_TYPE   src_type,
    DNX_SAND_IN   int                               priority,
    DNX_SAND_IN   int                               pfc_bitmap_index,
    DNX_SAND_OUT  uint32                            is_set)
{
    soc_error_t
        rv = SOC_E_NONE;
    DNX_TMC_FC_PFC_GEN_BMP_INFO
        pfc_gen_bmp_info;
    uint32
        tmp_used_1 = 0,
        tmp_used_2 = 0;

    DNXC_INIT_FUNC_DEFS;

    rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.get(unit, &pfc_gen_bmp_info);
    DNXC_IF_ERR_EXIT(rv);

    switch(src_type)
    {
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_NIF:
        tmp_used_1 = pfc_gen_bmp_info.nif_pfc_gen_bmp_used[priority];
        if (is_set) {
            tmp_used_1 |= (1<<pfc_bitmap_index);
        }
        else {
            tmp_used_1 &= ~(1<<pfc_bitmap_index);
        }

        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.nif_pfc_gen_bmp_used.set(unit, priority, tmp_used_1);
        DNXC_IF_ERR_EXIT(rv);
#endif 
        break;
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_OOB:
        tmp_used_1 = pfc_gen_bmp_info.cal_pfc_gen_bmp_used[priority];
        if (is_set) {
            tmp_used_1 |= (1<<pfc_bitmap_index);
        }
        else {
            tmp_used_1 &= ~(1<<pfc_bitmap_index);
        }

        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.cal_pfc_gen_bmp_used.set(unit, priority, tmp_used_1);
        DNXC_IF_ERR_EXIT(rv);
#endif 
        break;
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_ILKN_INB:
        tmp_used_1 = pfc_gen_bmp_info.cal_pfc_gen_bmp_used[priority];
        tmp_used_2 = pfc_gen_bmp_info.ilkn_inb_pfc_gen_bmp_used[priority];
        if (is_set) {
            tmp_used_1 |= (1<<pfc_bitmap_index);
            tmp_used_2 |= (1<<pfc_bitmap_index);
        }
        else {
            tmp_used_1 &= ~(1<<pfc_bitmap_index);
            tmp_used_2 &= ~(1<<pfc_bitmap_index);
        }

        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.cal_pfc_gen_bmp_used.set(unit, priority, tmp_used_1);
        DNXC_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.ilkn_inb_pfc_gen_bmp_used.set(unit, priority, tmp_used_2);
        DNXC_IF_ERR_EXIT(rv);
#endif 
        break;
    case DNX_TMC_FC_PFC_GEN_BMP_SRC_MUB:
        tmp_used_1 = pfc_gen_bmp_info.cal_pfc_gen_bmp_used[priority];
        tmp_used_2 = pfc_gen_bmp_info.mub_pfc_gen_bmp_used[priority];
        if (is_set) {
            tmp_used_1 |= (1<<pfc_bitmap_index);
            tmp_used_2 |= (1<<pfc_bitmap_index);
        }
        else {
            tmp_used_1 &= ~(1<<pfc_bitmap_index);
            tmp_used_2 &= ~(1<<pfc_bitmap_index);
        }

        

        /*rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.cal_pfc_gen_bmp_used.set(unit, priority, tmp_used_1);
        DNXC_IF_ERR_EXIT(rv);
        rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.mub_pfc_gen_bmp_used.set(unit, priority, tmp_used_2);
        DNXC_IF_ERR_EXIT(rv);*/
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support src_type(%d)\n"), src_type));
        break;
    }

   
    rv = sw_state_access[unit].dnx.soc.jer2_jericho.tm.pfc_gen_bmp.get(unit, &pfc_gen_bmp_info);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_cat_2_tc_hcfc_bitmap_verify(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           tc,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (core > MAX_NUM_OF_CORES) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("core is out of range")));
    }

    if ((tc < 0) || (tc > DNX_TMC_EGR_NOF_Q_PRIO_JER2_ARAD - 1)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("tc is out of range")));
    }

    DNXC_NULL_CHECK(hcfc_bitmap);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_cat_2_tc_hcfc_bitmap_set(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           tc,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  )
{
    soc_error_t 
        rv = SOC_E_NONE;
    soc_mem_t
        mem = INVALIDm;          
    soc_reg_above_64_val_t
        map_data;
    uint32 
        entry_offset = 0,
        entry_length = 0;
    soc_reg_above_64_val_t 
        cat2tc_map_ena;
    soc_reg_t 
        cat2tc_map_ena_reg = INVALIDr;
    uint32
        bit_offset;

    DNXC_INIT_FUNC_DEFS;

    rv = jer2_jer_fc_cat_2_tc_hcfc_bitmap_verify(unit, core, tc, is_ocb_only, hcfc_bitmap);
    DNXC_IF_ERR_EXIT(rv);

    mem = (core == 0) ? CFC_CAT_2_TC_IQM_0_MAP_HCFCm : CFC_CAT_2_TC_IQM_1_MAP_HCFCm;
    entry_offset = tc + ((is_ocb_only == 1)?(8):(0));    

    cat2tc_map_ena_reg = CFC_CAT_2_TC_MAP_HCFC_ENAr;
    bit_offset = tc + ((is_ocb_only == 1)?(8):(0));

    entry_length = soc_mem_field_length(unit, mem, BITMAPf);

    SOC_REG_ABOVE_64_CLEAR(map_data);
    /* set mapping from CAT2+TC to HCFC vector */
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
    DNXC_IF_ERR_EXIT(rv);

    SHR_BITCOPY_RANGE(map_data, 0, hcfc_bitmap->bitmap, 0, entry_length);

    rv = soc_mem_write(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
    DNXC_IF_ERR_EXIT(rv);

    /* enable map from CAT2+TC to NIF PFC */
    SOC_REG_ABOVE_64_CLEAR(cat2tc_map_ena);
    rv = soc_reg_above_64_get(unit, cat2tc_map_ena_reg, REG_PORT_ANY, 0, cat2tc_map_ena);
    DNXC_IF_ERR_EXIT(rv);
        
    SHR_BITSET(cat2tc_map_ena, bit_offset);
        
    rv = soc_reg_above_64_set(unit, cat2tc_map_ena_reg, REG_PORT_ANY, 0, cat2tc_map_ena);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_cat_2_tc_hcfc_bitmap_get(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           tc,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_mem_t
        mem = INVALIDm;          
    soc_reg_above_64_val_t 
        map_data;
    uint32 
        entry_offset = 0,
        entry_length = 0;

    DNXC_INIT_FUNC_DEFS;

    rv = jer2_jer_fc_cat_2_tc_hcfc_bitmap_verify(unit, core, tc, is_ocb_only, hcfc_bitmap);
    DNXC_IF_ERR_EXIT(rv);

    mem = (core == 0) ? CFC_CAT_2_TC_IQM_0_MAP_HCFCm : CFC_CAT_2_TC_IQM_1_MAP_HCFCm;
    entry_offset = tc + ((is_ocb_only == 1)?(8):(0));
    entry_length = soc_mem_field_length(unit, mem, BITMAPf);

    SOC_REG_ABOVE_64_CLEAR(map_data);
    /* get mapping from CAT2+TC to HCFC vector */
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
    DNXC_IF_ERR_EXIT(rv);

    SHR_BITCOPY_RANGE(hcfc_bitmap->bitmap, 0, map_data, 0, entry_length);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_glb_hcfc_bitmap_verify(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           is_high_prio,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_IN   int                           pool_id,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  )
{
    DNXC_INIT_FUNC_DEFS;

    if (core > MAX_NUM_OF_CORES) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("core is out of range")));
    }

    if ((pool_id != 0) && (pool_id != 1)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("pool_id is out of range")));
    }

    DNXC_NULL_CHECK(hcfc_bitmap);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_glb_hcfc_bitmap_set(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           is_high_prio,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_IN   int                           pool_id,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_mem_t 
        mem = INVALIDm;          
    soc_reg_above_64_val_t
        map_data;
    uint32 
        entry_offset = 0,
        entry_length = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_glb_hcfc_bitmap_verify(unit, core, is_high_prio, is_ocb_only, pool_id, hcfc_bitmap);
    DNXC_IF_ERR_EXIT(res);

    mem = (core == 0) ? CFC_GLB_RSC_IQM_0_MAPm : CFC_GLB_RSC_IQM_1_MAPm;
    entry_offset = ((is_ocb_only == 1)?(4):(0)) + 
                   ((pool_id == 1)?(2):(0)) + 
                   ((is_high_prio)?(1):(0));
    entry_offset += GLB_RSC_MAP_TBL_HCFC_OFFSET;
    entry_length = soc_mem_field_length(unit, mem, BITMAPf);
        
    /* set mapping from CAT2+TC to HCFC vector */
    res = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
    DNXC_IF_ERR_EXIT(res);

    SHR_BITCOPY_RANGE(map_data, 0, hcfc_bitmap->bitmap, 0, entry_length);

    res = soc_mem_write(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_glb_hcfc_bitmap_get(
    DNX_SAND_IN   int                           unit,
    DNX_SAND_IN   int                           core,
    DNX_SAND_IN   int                           is_high_prio,
    DNX_SAND_IN   int                           is_ocb_only,
    DNX_SAND_IN   int                           pool_id,
    DNX_SAND_OUT  DNX_TMC_FC_HCFC_BITMAP        *hcfc_bitmap
  )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_mem_t
        mem = INVALIDm;
    soc_reg_above_64_val_t
        map_data;
    uint32
        entry_offset = 0,
        entry_length = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_glb_hcfc_bitmap_verify(unit, core, is_high_prio, is_ocb_only, pool_id, hcfc_bitmap);
    DNXC_IF_ERR_EXIT(res);

    mem = (core == 0) ? CFC_GLB_RSC_IQM_0_MAPm : CFC_GLB_RSC_IQM_1_MAPm;
    entry_offset = ((is_ocb_only == 1)?(4):(0)) + 
                   ((pool_id == 1)?(2):(0)) + 
                   ((is_high_prio)?(1):(0));
    entry_offset += GLB_RSC_MAP_TBL_HCFC_OFFSET;
    entry_length = soc_mem_field_length(unit, mem, BITMAPf);
        
    /* set mapping from CAT2+TC to HCFC vector */
    res = soc_mem_read(unit, mem, MEM_BLOCK_ANY, entry_offset, map_data);
    DNXC_IF_ERR_EXIT(res);

    SHR_BITCOPY_RANGE(hcfc_bitmap->bitmap, 0, map_data, 0, entry_length);

exit:
    DNXC_FUNC_RETURN;
}


uint32
jer2_jer_fc_rec_cal_dest_type_to_val_internal(
    DNX_SAND_IN DNX_TMC_FC_REC_CAL_DEST dest_type
  )
{
    uint32 ret = 0;

    for(ret = 0; ret < JER2_JER_FC_REC_CAL_DEST_ARR_SIZE; ret++)
    {
        if(jer2_jer_fc_rec_cal_dest_arr[ret] == dest_type)
        {
            return ret;
        }
    }

    return -1;
}

uint32
jer2_jer_fc_gen_cal_src_type_to_val_internal(
    DNX_SAND_IN  DNX_TMC_FC_GEN_CAL_SRC     src_type
  )
{
    uint32 ret = 0;

    for(ret = 0; ret < JER2_JER_FC_GEN_CAL_SRC_ARR_SIZE; ret++) {
        if(jer2_jer_fc_gen_cal_src_arr[ret] == src_type) {
            return ret;
        }
    }

    return -1;
}


soc_error_t
  jer2_jer_fc_pfc_mapping_verify(
      DNX_SAND_IN int  unit,
      DNX_SAND_IN uint32                       nif_id,
      DNX_SAND_IN uint32                       src_pfc_ndx,
      DNX_SAND_IN DNX_TMC_FC_PFC_MAP           *pfc_map
  )
{
    DNXC_INIT_FUNC_DEFS;

    if(nif_id >= SOC_MEM_SIZE(unit, CFC_NIF_PFC_MAPm)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("nif_id is out of range[0, %d]"), (SOC_MEM_SIZE(unit, CFC_NIF_PFC_MAPm)-1)));
    }

    if (src_pfc_ndx > (DNX_TMC_EGR_NOF_Q_PRIO_JER2_ARAD - 1)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("src pfc index is out of range")));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_pfc_mapping_set(
      DNX_SAND_IN int  unit,
      DNX_SAND_IN uint32                        nif_id,
      DNX_SAND_IN uint32                        src_pfc_ndx,
      DNX_SAND_IN DNX_TMC_FC_PFC_MAP            *pfc_map
    )
{
    soc_error_t
        res = SOC_E_NONE;
    uint32
        nif_pfc_data[3] = {0};
    uint32
        entry_offset = 0, 
        field_offset = 0;
    uint32 
        select_data = 0,
        valid = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_pfc_mapping_verify(unit, nif_id, src_pfc_ndx, pfc_map);
    DNXC_IF_ERR_EXIT(res);

    entry_offset = nif_id;
    field_offset = src_pfc_ndx;

    if (pfc_map->mode == DNX_TMC_FC_PFC_MAP_EGQ) {
        select_data = 0;
    }
    else {
        select_data = 1;
    }
    valid = pfc_map->valid;

    res = soc_mem_read(unit, CFC_NIF_PFC_MAPm, MEM_BLOCK_ANY, entry_offset, nif_pfc_data);
    DNXC_IF_ERR_EXIT(res);

    soc_mem_field32_set(unit, CFC_NIF_PFC_MAPm, nif_pfc_data, index_field[field_offset], pfc_map->index);
    soc_mem_field32_set(unit, CFC_NIF_PFC_MAPm, nif_pfc_data, select_field[field_offset], select_data);
    soc_mem_field32_set(unit, CFC_NIF_PFC_MAPm, nif_pfc_data, valid_field[field_offset], valid);

    res = soc_mem_write(unit, CFC_NIF_PFC_MAPm, MEM_BLOCK_ANY, entry_offset, nif_pfc_data);
    DNXC_IF_ERR_EXIT(res);
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_pfc_mapping_get(
      DNX_SAND_IN int  unit,
      DNX_SAND_IN uint32                        nif_id,
      DNX_SAND_IN uint32                        src_pfc_ndx,
      DNX_SAND_OUT DNX_TMC_FC_PFC_MAP           *pfc_map
    )
{
    soc_error_t
        res = SOC_E_NONE;
    soc_reg_above_64_val_t 
        nif_pfc_data;
    uint32 
        entry_offset = 0, 
        field_offset = 0;
    uint32 select_data = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_pfc_mapping_verify(unit, nif_id, src_pfc_ndx, pfc_map);
    DNXC_IF_ERR_EXIT(res);

    entry_offset = nif_id;
    field_offset = src_pfc_ndx;

    SOC_REG_ABOVE_64_CLEAR(nif_pfc_data);
    res = soc_mem_read(unit, CFC_NIF_PFC_MAPm, MEM_BLOCK_ANY, entry_offset, nif_pfc_data);
    DNXC_IF_ERR_EXIT(res);

    pfc_map->index = soc_mem_field32_get(unit, CFC_NIF_PFC_MAPm, nif_pfc_data, index_field[field_offset]);
    select_data = soc_mem_field32_get(unit, CFC_NIF_PFC_MAPm, nif_pfc_data, select_field[field_offset]);
    pfc_map->valid = soc_mem_field32_get(unit, CFC_NIF_PFC_MAPm, nif_pfc_data, valid_field[field_offset]);

    if (select_data == 0) {
        pfc_map->mode = DNX_TMC_FC_PFC_MAP_EGQ;
    }
    else {
        pfc_map->mode = DNX_TMC_FC_PFC_MAP_GENERIC_BITMAP;
    }
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_inbnd_mode_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_IN int                           is_generate,
      DNX_SAND_IN DNX_TMC_FC_INBND_MODE         mode
      )
{
    soc_error_t
        rv = SOC_E_NONE;    
    portmod_pause_control_t 
        pause_control;
    portmod_llfc_control_t  
        llfc_control;
    portmod_pfc_control_t   
        pfc_control;

    DNXC_INIT_FUNC_DEFS;

    /* 1. setting MAC registers - port level { */
    /* Enable / Disable - LL */
    rv = portmod_port_pause_control_get(unit, port, &pause_control);
    DNXC_IF_ERR_EXIT(rv);
    if (is_generate) {
        pause_control.tx_enable = DNX_SAND_BOOL2NUM(mode == DNX_TMC_FC_INBND_MODE_LL);
    }
    else {
        pause_control.rx_enable = DNX_SAND_BOOL2NUM(mode == DNX_TMC_FC_INBND_MODE_LL);
    }
    rv = portmod_port_pause_control_set(unit, port, &pause_control);
    DNXC_IF_ERR_EXIT(rv);

    /* Enable / Disable - PFC */  
    rv = portmod_port_pfc_control_get(unit, port, &pfc_control);
    DNXC_IF_ERR_EXIT(rv);
    if (is_generate) {
        pfc_control.tx_enable = DNX_SAND_BOOL2NUM(mode == DNX_TMC_FC_INBND_MODE_PFC);
    }
    else {
        pfc_control.rx_enable = DNX_SAND_BOOL2NUM(mode == DNX_TMC_FC_INBND_MODE_PFC);
    }
    pfc_control.stats_en = TRUE;  /* always enable stats for PFC reception */
    rv = portmod_port_pfc_control_set(unit, port, &pfc_control);
    DNXC_IF_ERR_EXIT(rv);

    /* Enable / Disable - SAFC */
    rv = portmod_port_llfc_control_get(unit, port, &llfc_control);
    DNXC_IF_ERR_EXIT(rv);
    if (is_generate) {
        llfc_control.tx_enable = DNX_SAND_BOOL2NUM(mode == DNX_TMC_FC_INBND_MODE_SAFC);
    }
    else {
        llfc_control.rx_enable = DNX_SAND_BOOL2NUM(mode == DNX_TMC_FC_INBND_MODE_SAFC);
    }
    rv = portmod_port_llfc_control_set(unit, port, &llfc_control);
    DNXC_IF_ERR_EXIT(rv);    
    /* 1. setting MAC registers - port level } */

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_inbnd_mode_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_IN int                           is_generate,
      DNX_SAND_OUT DNX_TMC_FC_INBND_MODE        *mode
    )
{
    soc_error_t 
        rv = SOC_E_NONE;
    uint32 
        ll_enable = 0, 
        pfc_enable = 0, 
        safc_enable = 0;
    portmod_pause_control_t
        pause_control;
    portmod_llfc_control_t
        llfc_control;
    portmod_pfc_control_t
        pfc_control;

    DNXC_INIT_FUNC_DEFS;

    /* 1. Check which mode is enabled { */
    /* LL */
    rv = portmod_port_pause_control_get(unit, port, &pause_control);
    DNXC_IF_ERR_EXIT(rv);
    ll_enable = (is_generate) ? pause_control.tx_enable : pause_control.rx_enable;

    /* PFC */  
    rv = portmod_port_pfc_control_get(unit, port, &pfc_control);
    DNXC_IF_ERR_EXIT(rv);
    pfc_enable = (is_generate) ? pfc_control.tx_enable : pfc_control.rx_enable;

    /* SAFC */
    rv = portmod_port_llfc_control_get(unit, port, &llfc_control);
    DNXC_IF_ERR_EXIT(rv);
    safc_enable = (is_generate) ? llfc_control.tx_enable : llfc_control.rx_enable;
    /* 1. Check which mode is enabled } */

    /* 2. set the Inband Mode according to the register values { */
    if (ll_enable)
    {
        (*mode) = DNX_TMC_FC_INBND_MODE_LL;
    }
    else if(pfc_enable)
    {
        (*mode) = DNX_TMC_FC_INBND_MODE_PFC;
    }
    else if(safc_enable)
    {
        (*mode) = DNX_TMC_FC_INBND_MODE_SAFC;
    }
    else
    {
        (*mode) = DNX_TMC_FC_INBND_MODE_DISABLED;
    }
    /* 2. set the Inband Mode according to the register values } */
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_glb_rcs_mask_verify(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst
  )
{
    DNXC_INIT_FUNC_DEFS;

    /* Place your code here */
    if (core > MAX_NUM_OF_CORES) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("core is out of range")));
    }
    
    if (glb_res_dst < DNX_TMC_FC_GLB_RES_TYPE_MIX_P0 || glb_res_dst > DNX_TMC_FC_GLB_RES_TYPE_OCB_P1) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("global resouce dest is error")));
    }


exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_glb_rcs_mask_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_IN uint32                        glb_res_src_bitmap
    )
{
    soc_error_t res = DNX_SAND_OK;
    soc_reg_above_64_val_t reg_data;
    soc_reg_t reg = CFC_GLB_RSC_IQM_MASKr;
    soc_field_t field = INVALIDf;
    uint32 field_value = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_glb_rcs_mask_verify(unit, core, is_high_prio, glb_res_dst);
    DNXC_IF_ERR_EXIT(res);

    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_BDB)) {
        field_value |= 0x1;
    }
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_MINI_MC_DB)) {
        field_value |= 0x2;            
    }
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_MC_DB)) {
        field_value |= 0x4;            
    }
    if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB_DB)) {
        field_value |= 0x8;            
    } 

    switch (glb_res_dst) {
    case DNX_TMC_FC_GLB_RES_TYPE_MIX_P0:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_MIX_POOL_0_IQM_0_HP_MASKf : GLB_RSC_MIX_POOL_0_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_MIX_POOL_0_IQM_0_LP_MASKf : GLB_RSC_MIX_POOL_0_IQM_1_LP_MASKf;            
        }       
        if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_MIX_P0)) {
            field_value |= 0x10;
        }
        break;
    case DNX_TMC_FC_GLB_RES_TYPE_MIX_P1:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_MIX_POOL_1_IQM_0_HP_MASKf : GLB_RSC_MIX_POOL_1_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_MIX_POOL_1_IQM_0_LP_MASKf : GLB_RSC_MIX_POOL_1_IQM_1_LP_MASKf;            
        }       
        if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_MIX_P1)) {
            field_value |= 0x10;
        }
        break;
    case DNX_TMC_FC_GLB_RES_TYPE_OCB_P0:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_OCB_POOL_0_IQM_0_HP_MASKf : GLB_RSC_OCB_POOL_0_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_OCB_POOL_0_IQM_0_LP_MASKf : GLB_RSC_OCB_POOL_0_IQM_1_LP_MASKf;            
        }       
        if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB_P0)) {
            field_value |= 0x10;
        }
        break;
    case DNX_TMC_FC_GLB_RES_TYPE_OCB_P1:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_OCB_POOL_1_IQM_0_HP_MASKf: GLB_RSC_OCB_POOL_1_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_OCB_POOL_1_IQM_0_LP_MASKf: GLB_RSC_OCB_POOL_1_IQM_1_LP_MASKf;            
        }       
        if (glb_res_src_bitmap & (1<<DNX_TMC_FC_GLB_RES_TYPE_OCB_P1)) {
            field_value |= 0x10;
        }
        break;
    default:
        break;
    }

    res = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(res);
        
    soc_reg_above_64_field32_set(unit, reg, reg_data, field, field_value);    
        
    res = soc_reg_above_64_set(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(res);
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_glb_rcs_mask_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN int                           core,
      DNX_SAND_IN int                           is_high_prio,
      DNX_SAND_IN DNX_TMC_FC_GLB_RES_TYPE       glb_res_dst,
      DNX_SAND_OUT uint32                       *glb_res_src_bitmap
    )
{
    soc_error_t res = DNX_SAND_OK;
    soc_reg_above_64_val_t reg_data;
    soc_reg_t reg = CFC_GLB_RSC_IQM_MASKr;
    soc_field_t field = INVALIDf;
    uint32 field_value = 0;

    DNXC_INIT_FUNC_DEFS;

    res = jer2_jer_fc_glb_rcs_mask_verify(unit, core, is_high_prio, glb_res_dst);
    DNXC_IF_ERR_EXIT(res);
    
    DNXC_NULL_CHECK(glb_res_src_bitmap);

    switch (glb_res_dst) {
    case DNX_TMC_FC_GLB_RES_TYPE_MIX_P0:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_MIX_POOL_0_IQM_0_HP_MASKf : GLB_RSC_MIX_POOL_0_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_MIX_POOL_0_IQM_0_LP_MASKf : GLB_RSC_MIX_POOL_0_IQM_1_LP_MASKf;            
        }
        break;
    case DNX_TMC_FC_GLB_RES_TYPE_MIX_P1:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_MIX_POOL_1_IQM_0_HP_MASKf : GLB_RSC_MIX_POOL_1_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_MIX_POOL_1_IQM_0_LP_MASKf : GLB_RSC_MIX_POOL_1_IQM_1_LP_MASKf;            
        }
        break;
    case DNX_TMC_FC_GLB_RES_TYPE_OCB_P0:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_OCB_POOL_0_IQM_0_HP_MASKf : GLB_RSC_OCB_POOL_0_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_OCB_POOL_0_IQM_0_LP_MASKf : GLB_RSC_OCB_POOL_0_IQM_1_LP_MASKf;            
        }
        break;
    case DNX_TMC_FC_GLB_RES_TYPE_OCB_P1:
        if (is_high_prio) {
            field = (core == 0) ? GLB_RSC_OCB_POOL_1_IQM_0_HP_MASKf: GLB_RSC_OCB_POOL_1_IQM_1_HP_MASKf;
        }
        else {
            field = (core == 0) ? GLB_RSC_OCB_POOL_1_IQM_0_LP_MASKf: GLB_RSC_OCB_POOL_1_IQM_1_LP_MASKf;            
        }
        break;
    default:
        break;
    }

    res = soc_reg_above_64_get(unit, reg, REG_PORT_ANY, 0, reg_data);
    DNXC_IF_ERR_EXIT(res);
        
    field_value = soc_reg_above_64_field32_get(unit, reg, reg_data, field);

    if (field_value & 0x1) {
        (*glb_res_src_bitmap) |= DNX_TMC_FC_GLB_RES_TYPE_BDB;
    }
    if (field_value & 0x2) {
        (*glb_res_src_bitmap) |= DNX_TMC_FC_GLB_RES_TYPE_MINI_MC_DB;
    }
    if (field_value & 0x4) {
        (*glb_res_src_bitmap) |= DNX_TMC_FC_GLB_RES_TYPE_MC_DB;
    }
    if (field_value & 0x8) {
        (*glb_res_src_bitmap) |= DNX_TMC_FC_GLB_RES_TYPE_OCB_DB;
    } 
    if (field_value & 0x10) {
        (*glb_res_src_bitmap) |= glb_res_dst;
    }
  
exit:
    DNXC_FUNC_RETURN;
}

static int
  jer2_jer_fc_port_fifo_threshold_verify(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  soc_port_if_t              if_type,
    DNX_SAND_IN  DNX_TMC_FC_PORT_FIFO_TH    *info
  )
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    if (if_type == SOC_PORT_IF_ILKN) {  
        /* LLFC */
        if (info->llfc.clear > JER2_JER_FC_RX_MLF_HRF_MAX) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("llfc.clear is out of range")));
        }

        if (info->llfc.set > JER2_JER_FC_RX_MLF_HRF_MAX) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("llfc.set is out of range")));
        }

        /* PFC */
        if (info->pfc.clear != 0) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("setting pfc.clear is not supported by this chip")));
        }

        if (info->pfc.set != 0) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("setting pfc.set is not supported by this chip")));
        }
    }
    else {
        /* LLFC */
        if (info->llfc.clear > JER2_JER_FC_RX_MLF_MAX) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("llfc.clear is out of range")));
        }

        if (info->llfc.set > JER2_JER_FC_RX_MLF_MAX) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("llfc.set is out of range")));
        }

        /* PFC */
        if (info->pfc.clear > JER2_JER_FC_RX_MLF_MAX) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("pfc.clear is out of range")));
        }

        if (info->pfc.set > JER2_JER_FC_RX_MLF_MAX) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("pfc.set is out of range")));
        }
    }  

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_jer_fc_port_fifo_threshold_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  soc_port_t                 port,
    DNX_SAND_IN  DNX_TMC_FC_PORT_FIFO_TH    *info
  )
{
    soc_port_if_t interface_type = SOC_PORT_IF_NULL;
    uint32
        first_phy = 0, 
        first_lane = 0;
    uint32 ilkn_offset = 0;

    soc_reg_t 
        reg = INVALIDr,
        llfc_reg = INVALIDr,
        pfc_reg = INVALIDr;
    uint32 reg_val32 = 0;
    uint64 reg_val64 = COMPILER_64_INIT(0, 0);
    int reg_port = 0, 
        reg_offset = 0;
    uint32 num_ports_nbih = 0,
           num_lanes_per_nbi = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));

    DNXC_IF_ERR_EXIT(jer2_jer_fc_port_fifo_threshold_verify(unit, interface_type, info));

    if (interface_type == SOC_PORT_IF_ILKN) {

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_protocol_offset_get(unit, port, 0, &ilkn_offset));
        reg = (ilkn_offset < 2) ? NBIH_HRF_RX_CONFIG_HRFr : NBIL_HRF_RX_CONFIG_HRFr;
        reg_port = (ilkn_offset < 2) ? REG_PORT_ANY : (ilkn_offset / 4);

        /* set llfc threshold for non-TDM traffic */
        reg_offset = (ilkn_offset & 1) ? 1 : 0;
        DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, reg_offset, &reg_val64));
        soc_reg64_field32_set(unit, reg, &reg_val64, HRF_RX_LLFC_THRESHOLD_SET_HRF_Nf, info->llfc.set);
        soc_reg64_field32_set(unit, reg, &reg_val64, HRF_RX_LLFC_THRESHOLD_CLR_HRF_Nf, info->llfc.clear);
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, reg_offset, reg_val64));

        /* set llfc threshold for TDM traffic */
        reg_offset = (ilkn_offset & 1) ? 3 : 2;
        DNXC_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, reg_offset, reg_val64));
    }
    else {

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy /*one based*/));
        DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,
                mbcm_dnx_qsgmii_offsets_remove,
                (unit, first_phy, &first_lane)));

        num_ports_nbih = SOC_DNX_DEFS_GET(unit, nof_ports_nbih);
        num_lanes_per_nbi = SOC_DNX_DEFS_GET(unit, nof_lanes_per_nbi);
        if (SOC_IS_QUX(unit)) {
            llfc_reg = NIF_RX_MLF_LLFC_THRESHOLDS_CONFIGr;
            pfc_reg = NIF_RX_MLF_PFC_THRESHOLDS_CONFIGr;
            reg_port = REG_PORT_ANY;
        } else {
            llfc_reg = (first_lane < num_ports_nbih) ?  NBIH_RX_MLF_LLFC_THRESHOLDS_CONFIGr : NBIL_RX_MLF_LLFC_THRESHOLDS_CONFIGr;
            pfc_reg = (first_lane < num_ports_nbih) ?  NBIH_RX_MLF_PFC_THRESHOLDS_CONFIGr : NBIL_RX_MLF_PFC_THRESHOLDS_CONFIGr;
            reg_port = (first_lane < num_ports_nbih) ? REG_PORT_ANY : first_lane > (2 * num_lanes_per_nbi);
        }
        reg_offset = ((first_lane - 1) % num_lanes_per_nbi) / NUM_OF_LANES_IN_PM;

        /* set llfc threshold */
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, llfc_reg, reg_port, reg_offset, &reg_val32));
        soc_reg_field_set(unit, llfc_reg, &reg_val32, RX_LLFC_THRESHOLD_SET_QMLF_Nf, info->llfc.set);
        soc_reg_field_set(unit, llfc_reg, &reg_val32, RX_LLFC_THRESHOLD_CLR_QMLF_Nf, info->llfc.clear);
        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, llfc_reg, reg_port, reg_offset, reg_val32));

        /* set pfc threshold */
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, pfc_reg, reg_port, reg_offset, &reg_val32));
        soc_reg_field_set(unit, pfc_reg, &reg_val32, RX_PFC_THRESHOLD_SET_QMLF_Nf, info->pfc.set);
        soc_reg_field_set(unit, pfc_reg, &reg_val32, RX_PFC_THRESHOLD_CLR_QMLF_Nf, info->pfc.clear);
        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, pfc_reg, reg_port, reg_offset, reg_val32));
    }

exit:
    DNXC_FUNC_RETURN;
}

int
  jer2_jer_fc_port_fifo_threshold_get(
    DNX_SAND_IN  int                      unit,
    DNX_SAND_IN  soc_port_t               port,
    DNX_SAND_OUT DNX_TMC_FC_PORT_FIFO_TH  *info
  )
{
    soc_port_if_t interface_type = SOC_PORT_IF_NULL;
    uint32
        first_phy = 0, 
        first_lane = 0;
    uint32 ilkn_offset = 0;

    soc_reg_t reg = INVALIDr,
              llfc_reg = INVALIDr,
              pfc_reg = INVALIDr;
    uint32 reg_val32 = 0;
    uint64 reg_val64 = COMPILER_64_INIT(0, 0);
    int reg_port = 0,
        reg_offset = 0;
    uint32 num_ports_nbih = 0,
           num_lanes_per_nbi = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(info);

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));

    if (interface_type == SOC_PORT_IF_ILKN) {

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_protocol_offset_get(unit, port, 0, &ilkn_offset));
        reg = (ilkn_offset < 2) ? NBIH_HRF_RX_CONFIG_HRFr : NBIL_HRF_RX_CONFIG_HRFr;
        reg_port = (ilkn_offset < 2) ? REG_PORT_ANY : (ilkn_offset / 4);

        /* get llfc threshold for non-TDM traffic */
        reg_offset = (ilkn_offset & 1) ? 1 : 0;
        DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, reg_offset, &reg_val64));
        info->llfc.set = soc_reg64_field32_get(unit, reg, reg_val64, HRF_RX_LLFC_THRESHOLD_SET_HRF_Nf);
        info->llfc.clear = soc_reg64_field32_get(unit, reg, reg_val64, HRF_RX_LLFC_THRESHOLD_CLR_HRF_Nf);

        info->pfc.set = 0;
        info->pfc.clear = 0;
    }
    else {

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy /*one based*/));
        DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,
                mbcm_dnx_qsgmii_offsets_remove,
                (unit, first_phy, &first_lane)));

        num_ports_nbih = SOC_DNX_DEFS_GET(unit, nof_ports_nbih);
        num_lanes_per_nbi = SOC_DNX_DEFS_GET(unit, nof_lanes_per_nbi);
        if (SOC_IS_QUX(unit)) {
            llfc_reg = NIF_RX_MLF_LLFC_THRESHOLDS_CONFIGr;
            pfc_reg = NIF_RX_MLF_PFC_THRESHOLDS_CONFIGr;
            reg_port = REG_PORT_ANY;
        } else {
            llfc_reg = (first_lane < num_ports_nbih) ?  NBIH_RX_MLF_LLFC_THRESHOLDS_CONFIGr : NBIL_RX_MLF_LLFC_THRESHOLDS_CONFIGr;
            pfc_reg = (first_lane < num_ports_nbih) ?  NBIH_RX_MLF_PFC_THRESHOLDS_CONFIGr : NBIL_RX_MLF_PFC_THRESHOLDS_CONFIGr;
            reg_port = (first_lane < num_ports_nbih) ? REG_PORT_ANY : first_lane > (2 * num_lanes_per_nbi);
        }
        reg_offset = ((first_lane - 1) % num_lanes_per_nbi) / NUM_OF_LANES_IN_PM;

        /* get llfc threshold */
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, llfc_reg, reg_port, reg_offset, &reg_val32));
        info->llfc.set = soc_reg_field_get(unit, llfc_reg, reg_val32, RX_LLFC_THRESHOLD_SET_QMLF_Nf);
        info->llfc.clear = soc_reg_field_get(unit, llfc_reg, reg_val32, RX_LLFC_THRESHOLD_CLR_QMLF_Nf);

        /* get pfc threshold */
        DNXC_IF_ERR_EXIT(soc_reg32_get(unit, pfc_reg, reg_port, reg_offset, &reg_val32));
        info->pfc.set = soc_reg_field_get(unit, pfc_reg, reg_val32, RX_PFC_THRESHOLD_SET_QMLF_Nf);
        info->pfc.clear = soc_reg_field_get(unit, pfc_reg, reg_val32, RX_PFC_THRESHOLD_CLR_QMLF_Nf);
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_vsq_index_group2global(
    DNX_SAND_IN int                    unit,
    DNX_SAND_IN  DNX_TMC_ITM_VSQ_GROUP vsq_group,
    DNX_SAND_IN  uint32                vsq_in_group_ndx,
    DNX_SAND_IN  int                   cosq,
    DNX_SAND_IN  uint8                 is_ocb_only,
    DNX_SAND_IN  uint32                src_port,
    DNX_SAND_OUT uint32                *vsq_fc_ndx
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    DNX_TMC_ITM_VSQ_NDX
        vsq_ndx_tmp = 0;
    uint32
        first_phy = 0, 
        first_lane = 0,
        num_llfc = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(vsq_fc_ndx);

    if (vsq_group == JER2_ARAD_ITM_VSQ_GROUP_PFC || vsq_group == JER2_ARAD_ITM_VSQ_GROUP_LLFC) {
        rv = dnx_port_sw_db_first_phy_port_get(unit, src_port, &first_phy);
        DNXC_IF_ERR_EXIT(rv);
        rv = MBCM_DNX_DRIVER_CALL(unit,
                mbcm_dnx_qsgmii_offsets_remove,
                (unit, first_phy, &first_lane));
        DNXC_IF_ERR_EXIT(rv);
        first_phy--;  /* first_phy returned is one-based */
        first_lane--; /* first_lane returned is one-based */
    }

    switch(vsq_group)
    {
    case JER2_ARAD_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS:
        vsq_ndx_tmp += (JER2_ARAD_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS_SIZE + JER2_ARAD_ITM_VSQ_GROUP_CTGRY_SIZE)*2;
        vsq_ndx_tmp += (is_ocb_only?JER2_ARAD_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS_SIZE:0);
        vsq_ndx_tmp += vsq_in_group_ndx;
        break;
    case JER2_ARAD_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS:
        vsq_ndx_tmp += JER2_ARAD_ITM_VSQ_GROUP_CTGRY_SIZE*2;
        vsq_ndx_tmp += (is_ocb_only?JER2_ARAD_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS_SIZE:0);
        vsq_ndx_tmp += vsq_in_group_ndx;
        break;
    case JER2_ARAD_ITM_VSQ_GROUP_STTSTCS_TAG:
        vsq_ndx_tmp += (is_ocb_only?JER2_ARAD_ITM_VSQ_GROUP_STTSTCS_TAG_SIZE:0);
        vsq_ndx_tmp += vsq_in_group_ndx;
        break;
    case JER2_ARAD_ITM_VSQ_GROUP_CTGRY:
        vsq_ndx_tmp += (is_ocb_only?JER2_ARAD_ITM_VSQ_GROUP_CTGRY_SIZE:0);
        vsq_ndx_tmp += vsq_in_group_ndx;
        break;
    case JER2_ARAD_ITM_VSQ_GROUP_PFC:
        vsq_ndx_tmp = first_lane*8 + cosq;
        break;
    case JER2_ARAD_ITM_VSQ_GROUP_LLFC:
        num_llfc = SOC_IS_QAX(unit) ? 128 : 148;
        vsq_ndx_tmp = first_phy + (is_ocb_only ? num_llfc : 0);
        break;
    default:
        break;
    }
    *vsq_fc_ndx = vsq_ndx_tmp;

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_cmic_rx_set(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_IN uint32                        priority_bmp,
      DNX_SAND_IN int                           is_ena
  )
{
    soc_error_t 
        rv = SOC_E_NONE;    
    uint32 
        tm_port = 0,
        base_queue_pair = 0,
        nof_priorities = 0, 
        val32_arr[2] = {0},
        queue_pair[1] = {0};
    int
        core = 0, 
        cosq = 0, 
        i = 0, 
        j = 0;
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0);
    soc_reg_above_64_val_t 
        reg_abv64;
    uint32
        priority_bmp_lcl[1];
    

    DNXC_INIT_FUNC_DEFS;

    /* verify port, priority_bmp */
    if(!IS_CPU_PORT(unit, port)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("port is not CPU port!\n")));
    }

    /* get core, base_queue, nof_priorities */
    rv = dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_queue_pair);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
    DNXC_IF_ERR_EXIT(rv);

    /* set CFC_CMIC_RX_FC_MASK, CFC_CMIC_RX_FC_MAP */
    DNXC_IF_ERR_EXIT(soc_reg64_get(unit, CFC_CMIC_RX_FC_MASKr, REG_PORT_ANY, 0, &reg_val64));
    val32_arr[1] = COMPILER_64_HI(reg_val64);
    val32_arr[0] = COMPILER_64_LO(reg_val64);

    SOC_REG_ABOVE_64_CLEAR(reg_abv64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, CFC_CMIC_RX_FC_MAPr, REG_PORT_ANY, 0, reg_abv64));

    *priority_bmp_lcl =  priority_bmp;
    for (i = 0; i < nof_priorities; i++) 
    {
        if (SHR_BITGET(priority_bmp_lcl, i)) 
        {
            /* update CFC_CMIC_RX_FC_MASK */
            
            DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
            cosq = base_queue_pair - JER2_ARAD_PS_CPU_FIRST_VALID_QPAIR + i;
#endif 
            if (cosq >= 32) {
                (is_ena) ? SHR_BITSET(&val32_arr[1], cosq - 32) : SHR_BITCLR(&val32_arr[1], cosq - 32);
            }
            else {
                (is_ena) ? SHR_BITSET(&val32_arr[0], cosq) : SHR_BITCLR(&val32_arr[0], cosq);
            }

            /* update CFC_CMIC_RX_FC_MAP */
            *queue_pair = (core == 1) ? (JER2_ARAD_EGR_NOF_Q_PAIRS + base_queue_pair + i) : (base_queue_pair + i);

            /* It takes 9 bits to represent 512 EGQ PFCs */
            for (j = 0; j < 9; j++) {
                if (SHR_BITGET(queue_pair, j)) {
                    (is_ena) ? SHR_BITSET(reg_abv64, cosq*9 + j) : SHR_BITCLR(reg_abv64, cosq*9 + j);
                }
            }
        }
    }

    COMPILER_64_SET(reg_val64, val32_arr[1], val32_arr[0]);        
    DNXC_IF_ERR_EXIT(soc_reg64_set(unit, CFC_CMIC_RX_FC_MASKr, REG_PORT_ANY, 0, reg_val64));
    
    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, CFC_CMIC_RX_FC_MAPr, REG_PORT_ANY, 0, reg_abv64));

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_cmic_rx_get(
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    port,
      DNX_SAND_OUT uint32                       *priority_bmp,
      DNX_SAND_OUT int                          *is_ena
  )
{
    soc_error_t 
        rv = SOC_E_NONE;    
    uint32 
        tm_port = 0,
        base_queue_pair = 0,
        nof_priorities = 0, 
        val32_arr[2] = {0};
    int
        core = 0, 
        cosq = 0, 
        i = 0, 
        cosq_ena = 0;
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0);  

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(priority_bmp);
    DNXC_NULL_CHECK(is_ena);

    /* verify port, priority_bmp */
    if(!IS_CPU_PORT(unit, port)) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("port is not CPU port!\n")));
    }

    *priority_bmp = 0;
    *is_ena = 0;

    /* get core, base_queue, nof_priorities */
    rv = dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, tm_port, &base_queue_pair);
    DNXC_IF_ERR_EXIT(rv);

    rv = dnx_port_sw_db_tm_port_to_out_port_priority_get(unit, core, tm_port, &nof_priorities);
    DNXC_IF_ERR_EXIT(rv);

    /* set CFC_CMIC_RX_FC_MASK, CFC_CMIC_RX_FC_MAP */
    DNXC_IF_ERR_EXIT(soc_reg64_get(unit, CFC_CMIC_RX_FC_MASKr, REG_PORT_ANY, 0, &reg_val64));
    val32_arr[1] = COMPILER_64_HI(reg_val64);
    val32_arr[0] = COMPILER_64_LO(reg_val64);

    for (i = 0; i < nof_priorities; i++) 
    {
        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        cosq = base_queue_pair - JER2_ARAD_PS_CPU_FIRST_VALID_QPAIR + i;
#endif 
        if (cosq >= 32) {
            cosq_ena = SHR_BITGET(&val32_arr[1], cosq - 32);
        }
        else {
            cosq_ena = SHR_BITGET(&val32_arr[0], cosq);
        }

        if (cosq_ena != 0) {
            SHR_BITSET(priority_bmp, i);
            *is_ena = 1;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
   jer2_jer_fc_find_oob_inf_for_ilkn_inf (
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    ilkn_port,
      DNX_SAND_OUT DNX_TMC_FC_CAL_IF_ID         *oob_if
    )
{
    uint32
        oob_if_tmp = 0;
    soc_port_t
        ilkn_port_tmp = 0;
    soc_dnx_tm_config_t  *tm;
    uint32
        nof_oob_ena = 0, 
        ilkn_cnt = 0;
    uint32
        is_master = 0;
    uint32
        oob_id[DNX_TMC_FC_NOF_OOB_IDS] = {0, 1};

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(oob_if);

    *oob_if = -1;

    tm = &(SOC_DNX_CONFIG(unit)->tm);

    /* get how many OOB interface is enabled for ILKN */
    for(oob_if_tmp = 0; oob_if_tmp < DNX_TMC_FC_NOF_OOB_IDS; oob_if_tmp++)
    {
        if(tm->fc_oob_type[oob_if_tmp] != DNX_TMC_FC_CAL_TYPE_ILKN) {
            continue;
        }

        /* Handles RX */
        if(tm->fc_oob_mode[oob_if_tmp] & SOC_DNX_FC_CAL_MODE_RX_ENABLE || 
           tm->fc_oob_mode[oob_if_tmp] & SOC_DNX_FC_CAL_MODE_TX_ENABLE)
        {
            nof_oob_ena++;
        }
    }

    /* the first ilkn_if is connected to OOB 0, the second ilkn_if is connected OOB 1 */
    if (nof_oob_ena > 0) {
        PBMP_IL_ITER(unit, ilkn_port_tmp) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_master_get(unit, ilkn_port_tmp, &is_master));
            if (!is_master) {
                continue;
            }

            if (ilkn_cnt < nof_oob_ena) {
                if (ilkn_port_tmp == ilkn_port) {
                    *oob_if = oob_id[ilkn_cnt];
                    break;
                }
            }
            else {
                break;
            }
            
            ilkn_cnt++;
        }
    }
  
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
   jer2_jer_fc_connect_ilkn_inf_to_oob_inf (
      DNX_SAND_IN int                           unit,
      DNX_SAND_IN soc_port_t                    ilkn_if
    )
{
    DNX_TMC_FC_CAL_IF_ID oob_if;
    soc_reg_t reg;
    soc_field_t fld, 
        nbiH_fc_oob_cfg_flds[DNX_TMC_FC_NOF_OOB_IDS][3] = {
            {ILKN_OOB_0_TO_ILKN_PMHf, ILKN_OOB_0_TO_ILKN_PML_0f, ILKN_OOB_0_TO_ILKN_PML_1f},
            {ILKN_OOB_1_TO_ILKN_PMHf, ILKN_OOB_1_TO_ILKN_PML_0f, ILKN_OOB_1_TO_ILKN_PML_1f}},
        ilkn_fc_oob_cfg_flds[DNX_TMC_FC_NOF_OOB_IDS][2] = {
            {ILKN_OOB_0_TO_ILKN_0f, ILKN_OOB_0_TO_ILKN_1f},
            {ILKN_OOB_1_TO_ILKN_0f, ILKN_OOB_1_TO_ILKN_1f}},
        oob_clk_flds[DNX_TMC_FC_NOF_OOB_IDS] = {
            FC_OOB_0_CLK_SELECTf, FC_OOB_1_CLK_SELECTf},
        oob_nif_rstn_flds[2] = {
            FC_OOB_0_NIF_RSTNf, FC_OOB_1_NIF_RSTNf},
        ilkn_oob_nif_rstn_flds[2] = {
            FC_ILKN_0_OOB_NIF_RSTNf, FC_ILKN_1_OOB_NIF_RSTNf},
        oob_nif_2_pad_rstn_flds[2] = {
            FC_OOB_0_NIF_2_PAD_RSTNf, FC_OOB_1_NIF_2_PAD_RSTNf};
    uint32 reg_val;
    int pm_id = 0, reg_port = 0, ilkn_core_id = 0;
    uint32 ilkn_offset;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_jer_fc_find_oob_inf_for_ilkn_inf(unit, ilkn_if, &oob_if));
    if (oob_if == -1) {
        SOC_EXIT;
    }

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_protocol_offset_get(unit, ilkn_if, 0, &ilkn_offset));

    /* ILKN0/1 -> PMH, ILKN2/3 ->PML0, ILKN4/5 ->PML1 */
    pm_id = ilkn_offset/2;
    ilkn_core_id = (ilkn_offset & 1);

    /* ILKN0/1 -> port=REG_PORT_ANY, ILKN2/3 -> port=0, ILKN4/5 -> port=1 */
    reg_port = (ilkn_offset < 2) ? REG_PORT_ANY : (ilkn_offset / 4); 

    /* set NBIH_ILKN_FC_OOB_CONF */
    reg = NBIH_ILKN_FC_OOB_CONFr;
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val));
    fld = nbiH_fc_oob_cfg_flds[oob_if][pm_id];
    soc_reg_field_set(unit, reg, &reg_val, fld, 1);
    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val));

    /* set ILKN_PMH_ILKN_FC_OOB_CONF ILKN_PML_ILKN_FC_OOB_CONF(ILKN_PML0,ILKN_PML1) */
    reg = (ilkn_offset < 2 ) ? ILKN_PMH_ILKN_FC_OOB_CONFr : ILKN_PML_ILKN_FC_OOB_CONFr;
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
    fld = ilkn_fc_oob_cfg_flds[oob_if][ilkn_core_id];
    soc_reg_field_set(unit, reg, &reg_val, fld, 1);
    fld = oob_clk_flds[oob_if];
    soc_reg_field_set(unit, reg, &reg_val, fld, SOC_DNX_CONFIG(unit)->tm.fc_oob_tx_speed[oob_if]);
    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));

    /* set ILKN_PMH_ILKN_RESET ILKN_PML_ILKN_RESET(ILKN_PML0,ILKN_PML1)*/ 
    reg = (ilkn_offset < 2 ) ? ILKN_PMH_ILKN_RESETr : ILKN_PML_ILKN_RESETr;
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
    fld = oob_nif_2_pad_rstn_flds[oob_if];
    soc_reg_field_set(unit, reg, &reg_val, fld, 1);    

    fld = oob_nif_rstn_flds[oob_if];
    soc_reg_field_set(unit, reg, &reg_val, fld, 1);
	
    fld = ilkn_oob_nif_rstn_flds[ilkn_core_id];
    soc_reg_field_set(unit, reg, &reg_val, fld, 1);
    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_parse_tx_cal_tbl(
    int                      unit,
    uint32                   intf_type,
    uint32                   is_oob,
    DNX_TMC_FC_DIRECTION     direction,
    int                      port, 
    int                      calendar_id,
    uint32                   *cal_tx_src_type,
    DNX_TMC_ITM_VSQ_GROUP    *vsq_group,
    uint32                   *fc_index
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_mem_t 
        cal_table = INVALIDm;
    soc_reg_above_64_val_t
        cal_table_data;
    DNX_TMC_FC_CAL_MODE
        cal_mode = DNX_TMC_FC_NOF_CAL_MODES;
    JER2_ARAD_FC_CAL_TYPE
        cal_type = JER2_ARAD_FC_CAL_TYPES;
    JER2_ARAD_CFC_CALTX_TBL_DATA
        tx_cal;
    uint8
        is_ocb_only = 0;
    DNX_TMC_FC_GEN_CAL_SRC
        fc_src = DNX_TMC_FC_NOF_GEN_CAL_SRCS;

    DNXC_INIT_FUNC_DEFS;

    if (intf_type == DNX_TMC_FC_INTF_TYPE_ILKN) {
        cal_mode = is_oob ? DNX_TMC_FC_CAL_MODE_ILKN_OOB : DNX_TMC_FC_CAL_MODE_ILKN_INBND;
    }
    else {
        cal_mode = DNX_TMC_FC_CAL_MODE_SPI_OOB;
    }
    cal_type = ((direction == DNX_TMC_FC_DIRECTION_GEN) ? JER2_ARAD_FC_CAL_TYPE_TX : JER2_ARAD_FC_CAL_TYPE_RX);
    DNXC_IF_ERR_EXIT(jer2_jer_fc_cal_tbl_get(unit, cal_mode, cal_type, port, &cal_table));

    rv = soc_mem_read(unit, cal_table, MEM_BLOCK_ANY, calendar_id, cal_table_data);
    DNXC_IF_ERR_EXIT(rv);
    tx_cal.fc_index = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_INDEXf);
    tx_cal.fc_source_sel = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_SRC_SELf);

    fc_src = jer2_jer_fc_gen_cal_src_arr[tx_cal.fc_source_sel];
    *fc_index = tx_cal.fc_index;
 
    if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_STE) {
        /* update vsq_group and fc_index */
        if (DNX_SAND_FAILURE(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_itm_vsq_index_global2group,(unit, tx_cal.fc_index, vsq_group, fc_index, &is_ocb_only)))) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unit %d, error in vsq id retrieve %d"), unit, tx_cal.fc_index));
        }
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
    }
    else if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_STTSTCS_TAG) {
        *vsq_group = DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG; /* Sttstcs VSQ */
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
    }
    else if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_LLFC_VSQ) {
        *vsq_group = DNX_TMC_ITM_VSQ_GROUP_SRC_PORT;    /* Src Port VSQ */
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
    }
    else if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_PFC_VSQ) {
        *vsq_group = DNX_TMC_ITM_VSQ_GROUP_PG;          /* PG VSQ */
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
    }
    else if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_GLB_RCS) {
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_GLB;     /* GLB */
    }
    else if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_LLFC) {
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_NIF;     /* NIF */
    }
    else if (fc_src == DNX_TMC_FC_GEN_CAL_SRC_HCFC) {
        *cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_HCFC;    /* HCFC */
    }    
    else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support fc_src(%d) fc_source_sel(%d)\n"), fc_src, tx_cal.fc_source_sel));
    }

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_parse_rx_cal_tbl(
    int                      unit,
    uint32                   intf_type,
    uint32                   is_oob,
    DNX_TMC_FC_DIRECTION     direction,
    int                      port, 
    int                      calendar_id,
    uint32                   *cal_rx_dst_type,
    uint32                   *fc_index
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    soc_mem_t 
        cal_table = INVALIDm;
    soc_reg_above_64_val_t
        cal_table_data;
    DNX_TMC_FC_CAL_MODE
        cal_mode;
    JER2_ARAD_FC_CAL_TYPE
        cal_type;
    JER2_ARAD_CFC_CALRX_TBL_DATA
        rx_cal;
    uint32
        fc_dst_sel_2_dst_type[] = {    /* 0:PFC, 1:NIF, 2:2P port, 3:8P port, 4:Generic PFC bitmap, 5: retransmit, 6: NULL */
            DNX_TMC_FC_REC_CAL_DEST_PFC, 
            DNX_TMC_FC_REC_CAL_DEST_NIF_LL,
            DNX_TMC_FC_REC_CAL_DEST_PORT_2_PRIORITY,
            DNX_TMC_FC_REC_CAL_DEST_PORT_8_PRIORITY,
            DNX_TMC_FC_REC_CAL_DEST_GENERIC_PFC,
            DNX_TMC_FC_REC_CAL_DEST_RETRANSMIT,
            DNX_TMC_FC_REC_CAL_DEST_NONE};

    DNXC_INIT_FUNC_DEFS;

    if (intf_type == DNX_TMC_FC_INTF_TYPE_ILKN) {
        cal_mode = is_oob ? DNX_TMC_FC_CAL_MODE_ILKN_OOB : DNX_TMC_FC_CAL_MODE_ILKN_INBND;
    }
    else {
        cal_mode = DNX_TMC_FC_CAL_MODE_SPI_OOB;
    }
    cal_type = ((direction == DNX_TMC_FC_DIRECTION_GEN) ? JER2_ARAD_FC_CAL_TYPE_TX : JER2_ARAD_FC_CAL_TYPE_RX);
    DNXC_IF_ERR_EXIT(jer2_jer_fc_cal_tbl_get(unit, cal_mode, cal_type, port, &cal_table));

    rv = soc_mem_read(unit, cal_table, MEM_BLOCK_ANY, calendar_id, cal_table_data);
    DNXC_IF_ERR_EXIT(rv);
    rx_cal.fc_index = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_INDEXf);
    rx_cal.fc_dest_sel = soc_mem_field32_get(unit, cal_table, cal_table_data, FC_DST_SELf);

    *cal_rx_dst_type = fc_dst_sel_2_dst_type[rx_cal.fc_dest_sel];
    *fc_index = rx_cal.fc_index;

exit:
    DNXC_FUNC_RETURN;
}


static soc_error_t
  _jer2_jer_fc_trigger_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr; 
    uint32
        reg_val32 = 0, 
        fld_val32 = 0, 
        val32_arr[2] = {0};
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0);
    soc_reg_above_64_val_t
        reg_val_abv64;
    uint32
        start_bit = 0;
    uint32
        first_phy = 0, 
        first_lane = 0;
    int
        core = 0, 
        i = 0,
        res_type = 0,
        vsq_type = 0,
        src_id = 0,
        llfc_src_id = 0, 
        nif_af_id = 0,
        num_llfc = 0;
    
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_key);
    DNXC_NULL_CHECK(fc_status_info);

    if (fc_status_key->intf_type == DNX_TMC_FC_INTF_TYPE_NIF) {
        res_type = fc_status_key->src_type;
        vsq_type = fc_status_key->vsq_type;
        src_id = fc_status_key->vsq_id;

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, fc_status_key->port, &first_phy /*one based*/));
        DNXC_IF_ERR_EXIT
            (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane)));
        if (first_phy > 0) {
            first_phy--;
        }
        if (first_lane > 0) {
            first_lane--; /* first_lane returned is one-based */
        }

        if (fc_status_key->src_type == DNX_TMC_FC_SRC_TYPE_NIF){
            src_id = (fc_status_key->nif_fc_type == DNX_TMC_FC_NIF_TYPE_LLFC) ? first_phy : first_lane;
        }
        else if (fc_status_key->src_type == DNX_TMC_FC_SRC_TYPE_VSQ){
            if (fc_status_key->vsq_type == DNX_TMC_ITM_VSQ_GROUP_SRC_PORT) {
                src_id = first_phy;
                fc_status_info->nif_tx_src_id = src_id;
            }
            else if (fc_status_key->vsq_type == DNX_TMC_ITM_VSQ_GROUP_PG) {
                src_id = first_lane*8;
                fc_status_info->nif_tx_src_id = src_id;
            }
        }
    }
    else {
        res_type = fc_status_info->cal_tx_src_type;
        vsq_type = fc_status_info->cal_tx_src_vsq_type;
        src_id = fc_status_info->cal_tx_src_id;
    }

    switch(res_type) {
    case DNX_TMC_FC_SRC_TYPE_GLB:  /* Global */
        /* 1.1 IQM block*/
        if(!SOC_IS_QAX(unit)) {
            reg = IQM_GLOBAL_FLOW_CONTROL_STATEr;
            for (core = 0; core < SOC_DNX_DEFS_GET(unit, nof_cores); core++) {
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, core, 0, &reg_val32));

                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, GLBL_FC_FR_DB_MNMCf);
                fc_status_info->iqm_glb_mnmc_db_hi_fc_state[core] = SHR_BITGET(&fld_val32, 1) ? 1 : 0;
                fc_status_info->iqm_glb_mnmc_db_lo_fc_state[core] = SHR_BITGET(&fld_val32, 0) ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, GLBL_FC_FR_DB_FLMCf);
                fc_status_info->iqm_glb_flmc_db_hi_fc_state[core] = SHR_BITGET(&fld_val32, 1) ? 1 : 0;
                fc_status_info->iqm_glb_flmc_db_lo_fc_state[core] = SHR_BITGET(&fld_val32, 0) ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, GLBL_FC_BDBf);
                fc_status_info->iqm_glb_bdb_hi_fc_state[core] = SHR_BITGET(&fld_val32, 1) ? 1 : 0;
                fc_status_info->iqm_glb_bdb_lo_fc_state[core] = SHR_BITGET(&fld_val32, 0) ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, GLBL_FC_FR_OCBf);
                fc_status_info->iqm_glb_ocb_hi_fc_state[core] = SHR_BITGET(&fld_val32, 1) ? 1 : 0;
                fc_status_info->iqm_glb_ocb_lo_fc_state[core] = SHR_BITGET(&fld_val32, 0) ? 1 : 0;
            }
        }

        /* 1.2 CFC block*/
        for (core = 0; core < SOC_DNX_DEFS_GET(unit, nof_cores); core++) {
            if(!SOC_IS_QAX(unit)) {
                reg = CFC_IQM_SELr;
                fld_val32 = core;
                soc_reg_field_set(unit, reg, &reg_val32, IQM_SELf, fld_val32);
                DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32));

                reg = CFC_IQM_GLBL_FC_STATUSr;
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, 0, 0, &reg_val32)); 

                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_UC_DB_FC_HPf);
                fc_status_info->cfc_iqm_glb_mnmc_db_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_UC_DB_FC_LPf);
                fc_status_info->cfc_iqm_glb_mnmc_db_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_FL_MC_DB_FC_HPf);
                fc_status_info->cfc_iqm_glb_flmc_db_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_FL_MC_DB_FC_LPf);
                fc_status_info->cfc_iqm_glb_flmc_db_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_BDB_FC_HPf);
                fc_status_info->cfc_iqm_glb_bdb_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_BDB_FC_LPf);
                fc_status_info->cfc_iqm_glb_bdb_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_OCB_DB_FC_HPf);
                fc_status_info->cfc_iqm_glb_ocb_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_OCB_DB_FC_LPf);
                fc_status_info->cfc_iqm_glb_ocb_lo_fc_state[core] = fld_val32 ? 1 : 0;
            } else {
                reg = CFC_CGM_GLBL_FC_STATUSr;
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, 0, 0, &reg_val32));

                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_PDB_FC_HPf);
                fc_status_info->cfc_iqm_glb_mnmc_db_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_PDB_FC_LPf);
                fc_status_info->cfc_iqm_glb_mnmc_db_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_SRAM_FC_HPf);
                fc_status_info->cfc_iqm_glb_flmc_db_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_SRAM_FC_LPf);
                fc_status_info->cfc_iqm_glb_flmc_db_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_BDB_FC_HPf);
                fc_status_info->cfc_iqm_glb_bdb_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_BDB_FC_LPf);
                fc_status_info->cfc_iqm_glb_bdb_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_POOL_0_FC_HPf);
                fc_status_info->cfc_cgm_pool_0_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_POOL_0_FC_LPf);
                fc_status_info->cfc_cgm_pool_0_lo_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_POOL_1_FC_HPf);
                fc_status_info->cfc_cgm_pool_1_hi_fc_state[core] = fld_val32 ? 1 : 0;
                fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FR_POOL_1_FC_LPf);
                fc_status_info->cfc_cgm_pool_1_lo_fc_state[core] = fld_val32 ? 1 : 0;
				
            }
        }
        break;
    case DNX_TMC_FC_SRC_TYPE_VSQ:  /* VSQ A/B/C/D/E/F */
        /* A/B/C/D */
        if ((vsq_type >= DNX_TMC_ITM_VSQ_GROUP_CTGRY) && (vsq_type <= DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG)) {
            if (vsq_type == DNX_TMC_ITM_VSQ_GROUP_CTGRY) {
                fld_val32 = 0;
            }
            else if (vsq_type == DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS) {
                fld_val32 = 1;
            }
            else if (vsq_type == DNX_TMC_ITM_VSQ_GROUP_CTGRY_2_3_CNCTN_CLS) {
                fld_val32 = 2 + src_id / 64;
            }
            else if (vsq_type == DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG) {
                fld_val32 = 4 + src_id/64;
            }
            reg = CFC_IQM_VSQ_FC_STATUS_SELr;
            soc_reg_field_set(unit, reg, &reg_val32, IQM_VSQ_GRPS_ABCD_FC_STATUS_SELf, fld_val32);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 

            for (core = 0; core < SOC_DNX_DEFS_GET(unit, nof_cores); core++) {
                if(!SOC_IS_QAX(unit)) {
                    reg = CFC_IQM_SELr;
                    fld_val32 = core;
                    soc_reg_field_set(unit, reg, &reg_val32, IQM_SELf, fld_val32);
                    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 
                }

                reg = CFC_IQM_VSQ_GRPS_ABCD_FC_STATUSr;
                DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                val32_arr[1] = COMPILER_64_HI(reg_val64);
                val32_arr[0] = COMPILER_64_LO(reg_val64);
                if ((vsq_type == DNX_TMC_ITM_VSQ_GROUP_CTGRY) || (vsq_type == DNX_TMC_ITM_VSQ_GROUP_CTGRY_TRAFFIC_CLS)) {
                    fc_status_info->cfc_iqm_vsq_fc_state[core] = SHR_BITGET(val32_arr, src_id) ? 1 : 0;
                }
                else {
                    fc_status_info->cfc_iqm_vsq_fc_state[core] = SHR_BITGET(val32_arr, src_id%64) ? 1 : 0;
                }
            }
        }
        else if (vsq_type == DNX_TMC_ITM_VSQ_GROUP_LLFC) {
            for (i = 0; i < 2; i++) {
                num_llfc = SOC_IS_QAX(unit) ? 0 : 148;
                llfc_src_id = (i == 0) ? src_id : src_id + num_llfc;

                fld_val32 = llfc_src_id/64;
                reg = CFC_IQM_VSQ_FC_STATUS_SELr;
                soc_reg_field_set(unit, reg, &reg_val32, IQM_VSQ_LLFC_STATUS_SELf, fld_val32);
                DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 

                for (core = 0; core < SOC_DNX_DEFS_GET(unit, nof_cores); core++) {
                    if(!SOC_IS_QAX(unit)) {
                        reg = CFC_IQM_SELr;
                        fld_val32 = core;
                        soc_reg_field_set(unit, reg, &reg_val32, IQM_SELf, fld_val32);
                        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 
                    } 

                    reg = CFC_IQM_VSQ_LLFC_STATUSr;
                    DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                    val32_arr[1] = COMPILER_64_HI(reg_val64);
                    val32_arr[0] = COMPILER_64_LO(reg_val64);
                    if (i == 0) {
                        fc_status_info->cfc_iqm_vsq_fc_state[core] = SHR_BITGET(val32_arr, llfc_src_id%64) ? 1 : 0;
                    }
                    else {
                        fc_status_info->cfc_iqm_vsq_fc_state_2[core] = SHR_BITGET(val32_arr, llfc_src_id%64) ? 1 : 0;
                    }
                }
            }
        }
        else if (vsq_type == DNX_TMC_ITM_VSQ_GROUP_PFC) {
            reg = CFC_IQM_VSQ_FC_STATUS_SELr;
            fld_val32 = src_id/64;
            soc_reg_field_set(unit, reg, &reg_val32, IQM_VSQ_PFC_STATUS_SELf, fld_val32);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 

            for (core = 0; core < SOC_DNX_DEFS_GET(unit, nof_cores); core++) {
                if(!SOC_IS_QAX(unit)) {
                    reg = CFC_IQM_SELr;
                    fld_val32 = core;
                    soc_reg_field_set(unit, reg, &reg_val32, IQM_SELf, fld_val32);
                    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 
                } 

                reg = CFC_IQM_VSQ_PFC_STATUSr;
                DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                val32_arr[1] = COMPILER_64_HI(reg_val64);
                val32_arr[0] = COMPILER_64_LO(reg_val64);
                start_bit = src_id%64;
                SHR_BITCOPY_RANGE(&(fc_status_info->cfc_iqm_vsq_fc_state[core]), 0, val32_arr, start_bit, 8);
            }
        }
        break;
    case DNX_TMC_FC_SRC_TYPE_NIF:  /* Nif */
        nif_af_id = (fc_status_key->intf_type == DNX_TMC_FC_INTF_TYPE_NIF) ? first_phy : src_id;
        reg = CFC_NIF_AF_FC_STATUSr;
        SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
        DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
        fc_status_info->cfc_nif_af_fc_status = SHR_BITGET(reg_val_abv64, nif_af_id) ? 1 : 0;

        if (fc_status_key->nif_fc_type == DNX_TMC_FC_NIF_TYPE_LLFC) {
            reg = SOC_IS_QUX(unit) ? NIF_FC_RX_GEN_LLFC_FROM_QMLFr : NBIH_FC_RX_GEN_LLFC_FROM_QMLFr;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
            fc_status_info->nbi_llfc_status_from_mlf = SHR_BITGET(reg_val_abv64, src_id) ? 1 : 0;
        }
        else {
            reg = SOC_IS_QUX(unit) ? NIF_FC_RX_GEN_PFC_FROM_QMLFr : NBIH_FC_RX_GEN_PFC_FROM_QMLFr;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
            fc_status_info->nbi_pfc_status_from_mlf = SHR_BITGET(reg_val_abv64, src_id) ? 1 : 0;
        }
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support")));
        break;
    }

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_llfc_stop_tx_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN uint32                   first_phy,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO  *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr;
    soc_reg_above_64_val_t
        reg_val_abv64;
    uint32
        first_lane = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_info);
    DNXC_IF_ERR_EXIT
        (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane)));
    first_lane--; /* first_lane returned is one-based */

    /* 1. From cfc */
    reg = SOC_IS_QUX(unit) ? NIF_FC_TX_LLFC_STOP_TX_FROM_CFCr : NBIH_FC_TX_LLFC_STOP_TX_FROM_CFCr;
    SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
    fc_status_info->nbi_llfc_stop_tx_from_cfc = SHR_BITGET(reg_val_abv64, first_phy) ? 1 : 0;

    /* 2. To mac */
    reg = SOC_IS_QUX(unit) ? NIF_FC_TX_LLFC_STOP_TX_TO_PMr : NBIH_FC_TX_LLFC_STOP_TX_TO_PMr;
    SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
    fc_status_info->nbi_llfc_stop_tx_to_mac = SHR_BITGET(reg_val_abv64, first_lane) ? 1 : 0;

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_queue_pair_to_egr_if_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN int                      core,
    DNX_SAND_IN uint32                   queue_pair,
    DNX_SAND_OUT uint32                  *egr_if
  )
{
    soc_pbmp_t pbmp;
    soc_port_t port_i = 0;
    uint32     base_q_pair_i = 0;
    uint32     tm_port = 0;
    int        core_i = 0;
    int        is_found = 0, nof_priorities = 0;
    soc_error_t rv = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp));
    SOC_PBMP_ITER(pbmp, port_i) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port_i, &base_q_pair_i);
        DNXC_IF_ERR_EXIT(rv);

        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.priority_mode.get(unit, port_i, &nof_priorities);
        DNXC_IF_ERR_EXIT(rv);

        if ((queue_pair >= base_q_pair_i) && (queue_pair < (base_q_pair_i + nof_priorities))) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core_i));
            if (core_i == core) {
                is_found = 1;
                DNXC_SAND_IF_ERR_EXIT(soc_jer2_jer_egr_port2egress_offset(unit, core, tm_port, egr_if));
                break;            
            }
        }
    }

    if (!is_found) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Egress interface is not found for queue_pair(%d)\n"), queue_pair));
    }
    
exit:
    DNXC_FUNC_RETURN;
}


static soc_error_t
  _jer2_jer_fc_first_phy_to_egr_if_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN uint32                   first_phy,
    DNX_SAND_OUT uint32                  *egr_if
  )
{
    soc_pbmp_t pbmp;
    soc_port_t port_i;
    uint32     first_phy_i;
    uint32     tm_port;
    int        core_i;
    int        is_found = 0;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, 0, &pbmp));
    SOC_PBMP_ITER(pbmp, port_i) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_first_phy_port_get(unit, port_i, &first_phy_i /*one based*/));
        first_phy_i--;

        if (first_phy == first_phy_i) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core_i));
            is_found = 1;
            DNXC_SAND_IF_ERR_EXIT(soc_jer2_jer_egr_port2egress_offset(unit, core_i, tm_port, egr_if));
            break;            
        }
    }

    if (!is_found) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Egress interface is not found for first_phy(%d)\n"), first_phy));
    }
    
exit:
    DNXC_FUNC_RETURN;
}


static soc_error_t
  _jer2_jer_fc_egq_pfc_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN int                      is_full,
    DNX_SAND_IN int                      core,
    DNX_SAND_IN uint32                   base_queue_pair,
    DNX_SAND_IN int                      nof_priorities,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO  *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr;
    uint32
        reg_val32 = 0, 
        fld_val32 = 0, 
        reg_val32_hi = 0,
        reg_val32_lo = 0;
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0), 
        fld_val64 = COMPILER_64_INIT(0, 0); 
    soc_reg_above_64_val_t
        reg_val_abv64;
    int
        i = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_info);
    if (core > SOC_DNX_DEFS_GET(unit, nof_cores)) {
        DNXC_IF_ERR_EXIT(SOC_E_PARAM);
    }

    if(!SOC_IS_QAX(unit)) {
        reg = CFC_EGQ_SELr;
        fld_val32 = core;
        soc_reg_field_set(unit, reg, &reg_val32, EGQ_SELf, fld_val32);
        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 
    }

    if (is_full) {
        /* 1. Status in CFC */
        for (i = 0; i < 256; i+=64) {
            reg = CFC_EGQ_STATUS_SELr;
            fld_val32 = i/64;
            soc_reg_field_set(unit, reg, &reg_val32, EGQ_PFC_STATUS_SELf, fld_val32);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 

            reg = CFC_EGQ_PFC_STATUSr;
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
            fld_val64 = soc_reg64_field_get(unit, reg, reg_val64, EGQ_PFC_STATUSf);
            reg_val32_hi = COMPILER_64_HI(fld_val64);
            reg_val32_lo = COMPILER_64_LO(fld_val64);
            SHR_BITCOPY_RANGE(fc_status_info->cfc_egq_pfc_status_full[0], i+32, &reg_val32_hi, 0, 32);
            SHR_BITCOPY_RANGE(fc_status_info->cfc_egq_pfc_status_full[0], i, &reg_val32_lo, 0, 32);
         }

        /* 2. Status in EGQ */
        reg = EGQ_CFC_FLOW_CONTROLr;
        SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
        DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, core, 0, reg_val_abv64));
        SHR_BITCOPY_RANGE(fc_status_info->egq_cfc_fc_status_full[0], 0, reg_val_abv64, 0, 256);
    }
    else {
        /* 1. Status in CFC */
        reg = CFC_EGQ_STATUS_SELr;
        fld_val32 = base_queue_pair/64;
        soc_reg_field_set(unit, reg, &reg_val32, EGQ_PFC_STATUS_SELf, fld_val32);
        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 

        reg = CFC_EGQ_PFC_STATUSr;
        DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
        fld_val64 = soc_reg64_field_get(unit, reg, reg_val64, EGQ_PFC_STATUSf);
        reg_val32_hi = COMPILER_64_HI(fld_val64);
        reg_val32_lo = COMPILER_64_LO(fld_val64);
        if (base_queue_pair%64 >= 32){
            SHR_BITCOPY_RANGE(&fld_val32, 0, &reg_val32_hi, base_queue_pair%32, nof_priorities);
        }
        else {
            SHR_BITCOPY_RANGE(&fld_val32, 0, &reg_val32_lo, base_queue_pair%32, nof_priorities);
        }
        fc_status_info->cfc_egq_pfc_status[0] = fld_val32;

        /* 2. Status in EGQ */
        reg = EGQ_CFC_FLOW_CONTROLr;
        SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
        DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, core, 0, reg_val_abv64));
        SHR_BITCOPY_RANGE(&fld_val32, 0, reg_val_abv64, base_queue_pair, nof_priorities);
        fc_status_info->egq_cfc_fc_status[0] = fld_val32;

        /* 3. Status in SCH */
        for (i = 0; i < 8; i++) {
            reg = SCH_DBG_DVS_FC_COUNTERS_CONFIGURATIONr;
            fld_val32 = base_queue_pair + i;
            soc_reg_field_set(unit, reg, &reg_val32, FC_CNT_PORTf, fld_val32);
            soc_reg_field_set(unit, reg, &reg_val32, CNT_PORT_FCf, 1);
            soc_reg_field_set(unit, reg, &reg_val32, CNT_INTERFACE_FCf, 0);
            soc_reg_field_set(unit, reg, &reg_val32, CNT_DEVICE_FCf, 0);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, core, 0, reg_val32)); 

            reg = SCH_DBG_DVS_FLOW_CONTROL_COUNTERr;
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, core, 0, &reg_val32));
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, core, 0, &reg_val32));
            fc_status_info->sch_fc_port_cnt[i] = reg_val32;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_egq_inf_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN int                      core,
    DNX_SAND_IN uint32                   egr_if,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO  *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr;
    uint32
        reg_val32 = 0, 
        fld_val32 = 0; 
    soc_reg_above_64_val_t
        reg_val_abv64;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_info);

    /* 1. Status in CFC */
    if(!SOC_IS_QAX(unit)) {
        reg = CFC_EGQ_SELr;
        fld_val32 = core;
        soc_reg_field_set(unit, reg, &reg_val32, EGQ_SELf, fld_val32);
        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32));
    }

    reg = CFC_EGQ_IF_FC_STATUSr;
    SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
    fc_status_info->cfc_egq_inf_fc = SHR_BITGET(reg_val_abv64, egr_if) ? 1 : 0;

    /* 2. Status in SCH */
    /* Not supportted */

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_egq_dev_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN int                      core,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO  *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr;
    uint32
        reg_val32 = 0, 
        fld_val32 = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_info);

    /* 1. Status in CFC */
    if(!SOC_IS_QAX(unit)) {
        reg = CFC_EGQ_SELr;
        fld_val32 = core;
        soc_reg_field_set(unit, reg, &reg_val32, EGQ_SELf, fld_val32);
        DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32));
    }
    reg = CFC_EGQ_FC_STATUSr;
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32));
    fld_val32 = soc_reg_field_get(unit, reg, reg_val32, EGQ_DEV_FCf);
    fc_status_info->cfc_egq_dev_fc = fld_val32; 

    /* 2. Status in SCH */
    reg = SCH_DBG_DVS_FC_COUNTERS_CONFIGURATIONr;
    soc_reg_field_set(unit, reg, &reg_val32, CNT_PORT_FCf, 0);
    soc_reg_field_set(unit, reg, &reg_val32, CNT_INTERFACE_FCf, 0);
    soc_reg_field_set(unit, reg, &reg_val32, CNT_DEVICE_FCf, 1);
    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, core, 0, reg_val32)); 

    reg = SCH_DBG_DVS_FLOW_CONTROL_COUNTERr;
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, core, 0, &reg_val32));
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, core, 0, &reg_val32));
    fc_status_info->sch_fc_dev_cnt = reg_val32;

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_nif_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  )
{
    soc_error_t
        rv = SOC_E_NONE;
    uint32
        first_phy = 0, 
        first_lane = 0;
    soc_reg_t
        reg = INVALIDr, 
        reg_tx_pfc_arr[] = {TPFC0r, TPFC1r, TPFC2r, TPFC3r, TPFC4r, TPFC5r, TPFC6r, TPFC7r},
        reg_rx_pfc_arr[] = {RPFC0r, RPFC1r, RPFC2r, RPFC3r, RPFC4r, RPFC5r, RPFC6r, RPFC7r};
    uint32
        reg_val32 = 0, 
        fld_val32 = 0, 
        fld_val32_arr[2] = {0};
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0),
        fld_val64 = COMPILER_64_INIT(0, 0);
    soc_reg_above_64_val_t
        reg_val_abv64;
    uint32
        port = 0,
        tm_port = 0,
        base_q_pair = 0,
        egr_if = 0;
    int 
        i = 0, 
        nof_priorities = 0,
        core = 0, 
        is_generic_map_used = 0;
    DNX_TMC_FC_PFC_MAP
        pfc_map;
    
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_key);
    DNXC_NULL_CHECK(fc_status_info);

    port = fc_status_key->port;
    rv = dnx_port_sw_db_first_phy_port_get(unit, port, &first_phy /*one based*/);
    DNXC_IF_ERR_EXIT(rv);
    DNXC_IF_ERR_EXIT
       (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_qsgmii_offsets_remove, (unit, first_phy, &first_lane)));
    if (first_phy > 0) {
        first_phy--;
    }
    if (first_lane > 0) {
        first_lane--; /* first_lane returned is one-based */
    }

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.base_q_pair.get(unit, port, &base_q_pair);
    DNXC_IF_ERR_EXIT(rv);

    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.logical_ports_info.priority_mode.get(unit, port, &nof_priorities);
    DNXC_IF_ERR_EXIT(rv);

    DNXC_SAND_IF_ERR_EXIT(soc_jer2_jer_egr_port2egress_offset(unit, core, tm_port, &egr_if));

    if (fc_status_key->direction == DNX_TMC_FC_DIRECTION_GEN) {
        /* IQM & CFC block */
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_trigger_status_info_get(unit, fc_status_key, fc_status_info));

        /* NIB & MIB block */
        switch(fc_status_key->nif_fc_type) /* 0:llfc, 1:pfc 2:safc */
        {
        case DNX_TMC_FC_NIF_TYPE_LLFC:
            /* NBI block */
            reg = SOC_IS_QUX(unit) ? NIF_FC_TX_GEN_LLFC_FROM_CFCr : NBIH_FC_TX_GEN_LLFC_FROM_CFCr;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
            fc_status_info->nbi_tx_llfc_from_cfc = SHR_BITGET(reg_val_abv64, first_phy) ? 1 : 0;

            reg = SOC_IS_QUX(unit) ? NIF_FC_TX_GEN_LLFC_TO_PMr : NBIH_FC_TX_GEN_LLFC_TO_PMr;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
            fc_status_info->nbi_tx_llfc_to_mac = SHR_BITGET(reg_val_abv64, first_phy) ? 1 : 0;  

            /* MIB block */
            reg = TXPFr;
            DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
            COMPILER_64_ZERO(fc_status_info->mib_tx_pause_cnt);
            COMPILER_64_ADD_64(fc_status_info->mib_tx_pause_cnt, reg_val64);

            break;
        case DNX_TMC_FC_NIF_TYPE_PFC:
        case DNX_TMC_FC_NIF_TYPE_SAFC:
            /* NBI block */
            reg = SOC_IS_QUX(unit) ? NIF_FC_CHOOSE_PORT_FOR_PFCr : NBIH_FC_CHOOSE_PORT_FOR_PFCr;
            fld_val32 = first_lane;
            soc_reg_field_set(unit, reg, &reg_val32, FC_CHOOSE_PORT_FOR_PFCf, fld_val32);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32));

            reg = SOC_IS_QUX(unit) ? NIF_FC_PFC_DEBUG_INDICATIONSr : NBIH_FC_PFC_DEBUG_INDICATIONSr;
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32));
            fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FC_TX_GEN_PFC_FROM_CFCf);
            fc_status_info->nbi_tx_pfc_from_cfc = fld_val32; 

            fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FC_TX_GEN_PFC_TO_PMf);
            fc_status_info->nbi_tx_pfc_to_mac = fld_val32; 

            /* MIB block */
            if (fc_status_key->nif_fc_type == DNX_TMC_FC_NIF_TYPE_PFC) {
                reg = TXPPr;
                DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                COMPILER_64_ZERO(fc_status_info->mib_tx_pfc_cnt);
                COMPILER_64_ADD_64(fc_status_info->mib_tx_pfc_cnt, reg_val64);

                for (i = 0; i < 8; i++) {
                    reg = reg_tx_pfc_arr[i];
                    DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                    COMPILER_64_ZERO(fc_status_info->mib_tx_pfc_x_cnt[i]);
                    COMPILER_64_ADD_64(fc_status_info->mib_tx_pfc_x_cnt[i], reg_val64);
                }
            }
            else {
                reg = TX_LLFC_LOG_COUNTERr;
                DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                COMPILER_64_ZERO(fc_status_info->mib_tx_safc_log_cnt);
                COMPILER_64_ADD_64(fc_status_info->mib_tx_safc_log_cnt, reg_val64);
            }
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support nif_fc_type(%d)\n"), fc_status_key->nif_fc_type));
            break;
        }
    }
    else {
        switch(fc_status_key->nif_fc_type) /* 0:llfc, 1:pfc 2:safc */
        {
        case DNX_TMC_FC_NIF_TYPE_LLFC:
            /* MIB block */
            reg = RXPFr;
            DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
            COMPILER_64_ZERO(fc_status_info->mib_rx_pause_cnt);
            COMPILER_64_ADD_64(fc_status_info->mib_rx_pause_cnt, reg_val64);

            /* EGQ block */
            reg = EGQ_NIF_FLOW_CONTROLr;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, core, 0, reg_val_abv64));
            fc_status_info->egq_nif_fc_status[0] = SHR_BITGET(reg_val_abv64, egr_if) ? 1 : 0;
            break;
        case DNX_TMC_FC_NIF_TYPE_PFC:
        case DNX_TMC_FC_NIF_TYPE_SAFC:
            /* MIB block */
            if (fc_status_key->nif_fc_type == DNX_TMC_FC_NIF_TYPE_PFC) {
                reg = RXPPr;
                DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                COMPILER_64_ZERO(fc_status_info->mib_rx_pfc_cnt);
                COMPILER_64_ADD_64(fc_status_info->mib_rx_pfc_cnt, reg_val64);

                for (i = 0; i < 8; i++) {
                    reg = reg_rx_pfc_arr[i];
                    DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                    COMPILER_64_ZERO(fc_status_info->mib_rx_pfc_x_cnt[i]);
                    COMPILER_64_ADD_64(fc_status_info->mib_rx_pfc_x_cnt[i], reg_val64);
                }
            }
            else {
                reg = RX_LLFC_LOG_COUNTERr;
                DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                COMPILER_64_ZERO(fc_status_info->mib_rx_safc_log_cnt);
                COMPILER_64_ADD_64(fc_status_info->mib_rx_safc_log_cnt, reg_val64);

                reg = RX_LLFC_PHY_COUNTERr;
                DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                COMPILER_64_ZERO(fc_status_info->mib_rx_safc_phy_cnt);
                COMPILER_64_ADD_64(fc_status_info->mib_rx_safc_phy_cnt, reg_val64);

                reg = RX_LLFC_CRC_COUNTERr;
                DNXC_IF_ERR_EXIT(soc_reg_get(unit, reg, port, 0, &reg_val64));
                COMPILER_64_ZERO(fc_status_info->mib_rx_safc_crc_cnt);
                COMPILER_64_ADD_64(fc_status_info->mib_rx_safc_crc_cnt, reg_val64);
            }

            /* NIF block */
            reg = SOC_IS_QUX(unit) ? NIF_FC_CHOOSE_PORT_FOR_PFCr : NBIH_FC_CHOOSE_PORT_FOR_PFCr;
            fld_val32 = first_lane;
            soc_reg_field_set(unit, reg, &reg_val32, FC_CHOOSE_PORT_FOR_PFCf, fld_val32);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32));

            reg = SOC_IS_QUX(unit) ? NIF_FC_PFC_DEBUG_INDICATIONSr : NBIH_FC_PFC_DEBUG_INDICATIONSr;
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32));
            fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FC_RX_PFC_FROM_PMf);
            fc_status_info->nbi_rx_pfc_from_mac = fld_val32; 

            fld_val32 = soc_reg_field_get(unit, reg, reg_val32, FC_RX_PFC_TO_CFCf);
            fc_status_info->nbi_rx_pfc_to_cfc = fld_val32; 

            /* CFC Nif */
            reg = CFC_NIF_PFC_STATUS_SELr;
            fld_val32 = (first_lane*8)/64;
            if (fld_val32 > NIF_PFC_STATUS_SEL_MAX) {
                DNXC_EXIT_WITH_ERR
                   (SOC_E_PARAM, (_BSL_DNXC_MSG("NIF_PFC_STATUS_SELf is out of range")));
            }
            soc_reg_field_set(unit, reg, &reg_val32, NIF_PFC_STATUS_SELf, fld_val32);
            DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32)); 

            reg = CFC_NIF_PFC_STATUSr;
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
            fld_val64 = soc_reg64_field_get(unit, reg, reg_val64, NIF_PFC_STATUSf);
            fld_val32_arr[1] = COMPILER_64_HI(fld_val64);
            fld_val32_arr[0] = COMPILER_64_LO(fld_val64); 
            SHR_BITCOPY_RANGE(&fld_val32, 0, fld_val32_arr, (first_lane*8)%64, 8);
            fc_status_info->cfc_nif_pfc_status = fld_val32;

            /* CFC & EGQ & SCH block */
            for (i = 0; i < 8; i++) {
                DNXC_IF_ERR_EXIT(jer2_jer_fc_pfc_mapping_get(unit, first_lane, i, &pfc_map));

                if (pfc_map.valid == 0) {
                    fc_status_info->nif_rx_dst_type[i] = DNX_TMC_FC_NOF_PFC_MAP_MODE;
                    continue;
                }
                fc_status_info->nif_rx_dst_type[i] = pfc_map.mode;
                if (pfc_map.mode == DNX_TMC_FC_PFC_MAP_EGQ) {
                    fc_status_info->nif_rx_dst_id[i] = pfc_map.index % JER2_ARAD_EGR_NOF_BASE_Q_PAIRS;
                }
                else {
                    fc_status_info->nif_rx_dst_id[i] = pfc_map.index;
                    is_generic_map_used = 1;
                }
            }
            fc_status_info->core_id = core;

            if (is_generic_map_used) {
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, TRUE, core, 0, 0, fc_status_info));
            }
            else {
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, FALSE, core, base_q_pair, nof_priorities, fc_status_info));
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_inf_status_info_get(unit, core, egr_if, fc_status_info));
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_dev_status_info_get(unit, core, fc_status_info));
            }
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support nif_fc_type(%d)\n"), fc_status_key->nif_fc_type));
            break;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_ilkn_inb_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr, 
        ilkn_inb_status_regs[] = {
            CFC_ILKN_INB_RX_0_FC_STATUSr, CFC_ILKN_INB_RX_1_FC_STATUSr, CFC_ILKN_INB_RX_2_FC_STATUSr, 
            CFC_ILKN_INB_RX_3_FC_STATUSr, CFC_ILKN_INB_RX_4_FC_STATUSr, CFC_ILKN_INB_RX_5_FC_STATUSr};
    uint32
        reg_val32 = 0,
        reg_val32_hi[1] = {0},
        reg_val32_lo = 0;
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0);
    soc_reg_above_64_val_t
        reg_val_abv64;
    uint32
        calendar_id = 0,
        port = 0, 
        intf_type = 0;
    DNX_TMC_FC_DIRECTION
        direction = DNX_TMC_FC_NOF_DIRECTIONS;
    uint32
        fc_index = 0,
        q_pair = 0,
        egr_if = 0;
    int 
        core = 0,
        nof_priorities = 0;
    DNX_TMC_FC_PFC_GENERIC_BITMAP
        pfc_bitmap;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_key);
    DNXC_NULL_CHECK(fc_status_info);

    /* Verify paramters */
    if (fc_status_key->port >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("port(%d) is out of range [0, %d]"), fc_status_key->port, (SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports-1)));
    }

    if (fc_status_key->calendar_id >= SOC_DNX_DEFS_GET(unit, fc_inb_cal_len_max)){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("calendar_id(%d) is out of range [0, %d]"), fc_status_key->calendar_id, (SOC_DNX_DEFS_GET(unit, fc_inb_cal_len_max)-1)));
    }

    calendar_id = fc_status_key->calendar_id;
    port = fc_status_key->port;
    intf_type = fc_status_key->intf_type;
    direction = fc_status_key->direction;

    if (fc_status_key->direction == DNX_TMC_FC_DIRECTION_GEN) {
        /* Parse tx calendar entry */
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_parse_tx_cal_tbl(unit, intf_type, FALSE, direction, port, calendar_id,
            &fc_status_info->cal_tx_src_type,
            &fc_status_info->cal_tx_src_vsq_type,
            &fc_status_info->cal_tx_src_id));

        /* IQM & CFC block */
        if (fc_status_info->cal_tx_src_type != DNX_TMC_FC_SRC_TYPE_HCFC) {
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_trigger_status_info_get(unit, fc_status_key, fc_status_info));
        }

        /* NBI block*/
        reg = NBIH_FC_ILKN_RX_IB_CHFC_FROM_CFC_RAWr;
        DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, port, &reg_val64));
        *reg_val32_hi = COMPILER_64_HI(reg_val64);
        reg_val32_lo = COMPILER_64_LO(reg_val64);
        fc_status_info->nbi_ilkn_rx_chfc_from_cfc_raw = (calendar_id >= 32) ? 
            (SHR_BITGET(reg_val32_hi, calendar_id-32) ? 1 : 0) : 
            (SHR_BITGET(&reg_val32_lo, calendar_id) ? 1 : 0);

        reg = NBIH_FC_ILKN_TX_GEN_IB_CHFC_ROCr;
        DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, port, &reg_val64));
        *reg_val32_hi = COMPILER_64_HI(reg_val64);
        reg_val32_lo = COMPILER_64_LO(reg_val64);
        fc_status_info->nbi_ilkn_tx_chfc_roc = (calendar_id >= 32) ? 
            (SHR_BITGET(reg_val32_hi, calendar_id-32) ? 1 : 0) : 
            (SHR_BITGET(&reg_val32_lo, calendar_id) ? 1 : 0);
    }
    else
    {
        if (fc_status_key->calendar_fc_type == DNX_TMC_FC_CALENDAR_TYPE_LLFC) /* LLFC */
        {
            if(SOC_IS_JERICHO_PLUS_ONLY(unit)) { /* JER2_JERICHO PLUS is 64 bit */
                reg = NBIH_ILKN_RX_LLFC_FROM_RX_CNTr;
                DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, port, &reg_val64));
                fc_status_info->nbi_ilkn_rx_llfc_cnt = reg_val64;

                reg = NBIH_ILKN_TX_LLFC_STOP_TX_CNTr;
                DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, port, &reg_val64));
                fc_status_info->nbi_ilkn_llfc_stop_tx_cnt = reg_val64; 
            } else { /* JER2_JERICHO, QMX, JER2_QAX,QUX  are 32 bit */
                reg = NBIH_ILKN_RX_LLFC_FROM_RX_CNTr;
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, port, &reg_val32));
                COMPILER_64_SET(fc_status_info->nbi_ilkn_rx_llfc_cnt, 0, reg_val32);

                reg = NBIH_ILKN_TX_LLFC_STOP_TX_CNTr;
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, port, &reg_val32));
                COMPILER_64_SET(fc_status_info->nbi_ilkn_llfc_stop_tx_cnt, 0, reg_val32);
            }
        }
        else if (fc_status_key->calendar_fc_type == DNX_TMC_FC_CALENDAR_TYPE_CHFC) /* CHFC */
        {
            /* Parse tx calendar entry */
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_parse_rx_cal_tbl(unit, intf_type, FALSE, direction, port, calendar_id,
                &fc_status_info->cal_rx_dst_type,
                &fc_status_info->cal_rx_dst_id));
            fc_index = fc_status_info->cal_rx_dst_id;

            /* NBI block*/
            reg = NBIH_FC_ILKN_RX_IB_CHFC_FROM_PORT_RAWr;
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, port, &reg_val64));
            *reg_val32_hi = COMPILER_64_HI(reg_val64);
            reg_val32_lo = COMPILER_64_LO(reg_val64);
            fc_status_info->nbi_ilkn_rx_chfc_from_port_raw = (calendar_id >= 32) ? 
                (SHR_BITGET(reg_val32_hi, calendar_id-32) ? 1 : 0) : 
                (SHR_BITGET(&reg_val32_lo, calendar_id) ? 1 : 0); 

            reg = NBIH_FC_ILKN_RX_IB_CHFC_FROM_PORT_ROCr;
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, port, &reg_val64));
            *reg_val32_hi = COMPILER_64_HI(reg_val64);
            reg_val32_lo = COMPILER_64_LO(reg_val64);
            fc_status_info->nbi_ilkn_rx_chfc_from_port_roc = (calendar_id >= 32) ?
                (SHR_BITGET(reg_val32_hi, calendar_id-32) ? 1 : 0) :
                (SHR_BITGET(&reg_val32_lo, calendar_id) ? 1 : 0);

            /* CFC block */
            reg = ilkn_inb_status_regs[port];;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
            fc_status_info->cfc_ilkn_fc_status = SHR_BITGET(reg_val_abv64, calendar_id) ? 1 : 0;

            switch(fc_status_info->cal_rx_dst_type)
            {
            case DNX_TMC_FC_REC_CAL_DEST_NIF_LL:
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_first_phy_to_egr_if_get(unit, fc_index, &egr_if));

                reg = EGQ_NIF_FLOW_CONTROLr;
                SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
                DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, core, 0, reg_val_abv64));
                fc_status_info->egq_nif_fc_status[0] = SHR_BITGET(reg_val_abv64, egr_if) ? 1 : 0;
            
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_llfc_stop_tx_status_info_get(unit, fc_index, fc_status_info));
                break;
            case DNX_TMC_FC_REC_CAL_DEST_PFC:
                if (nof_priorities == 0) nof_priorities = 1;
                /* Intentional fall through */
                /* coverity[unterminated_case] */
            case DNX_TMC_FC_REC_CAL_DEST_PORT_2_PRIORITY:
                if (nof_priorities == 0) nof_priorities = 2;
                /* Intentional fall through */
                /* coverity[unterminated_case] */
            case DNX_TMC_FC_REC_CAL_DEST_PORT_8_PRIORITY:
                if (nof_priorities == 0) nof_priorities = 8;

                core = (fc_index >= 256) ? 1 : 0;
                q_pair = fc_index % 256;
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_queue_pair_to_egr_if_get(unit, core, q_pair, &egr_if));

                /* CFC & EGQ & SCH block */
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, FALSE, core, q_pair, nof_priorities, fc_status_info));
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_inf_status_info_get(unit, core, egr_if, fc_status_info));
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_dev_status_info_get(unit, core, fc_status_info));
                break;
            case DNX_TMC_FC_REC_CAL_DEST_GENERIC_PFC:
                DNXC_IF_ERR_EXIT
                    (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_fc_pfc_generic_bitmap_get, (unit, port, fc_index, &pfc_bitmap)));
                DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, TRUE, pfc_bitmap.core, 0, 0, fc_status_info));
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support")));
                break;
            }
        }
        else {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support calendar_fc_type(%d)\n"), fc_status_key->calendar_fc_type));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_mub_inb_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr;
    soc_field_t
        fld = INVALIDf, 
        tx_mub_from_cfc_flds[] = {
            FC_TX_0_MUBITS_FROM_CFCf, FC_TX_1_MUBITS_FROM_CFCf, FC_TX_2_MUBITS_FROM_CFCf,
            FC_TX_3_MUBITS_FROM_CFCf, FC_TX_4_MUBITS_FROM_CFCf, FC_TX_5_MUBITS_FROM_CFCf}, 
        rx_llfc_stop_tx_from_mub_flds[] = {
            FC_RX_0_LLFC_STOP_TX_FROM_MUBITSf, FC_RX_1_LLFC_STOP_TX_FROM_MUBITSf, FC_RX_2_LLFC_STOP_TX_FROM_MUBITSf, 
            FC_RX_3_LLFC_STOP_TX_FROM_MUBITSf, FC_RX_4_LLFC_STOP_TX_FROM_MUBITSf, FC_RX_5_LLFC_STOP_TX_FROM_MUBITSf}, 
        rx_mub_to_cfc_flds[] = {
            FC_RX_0_MUBITS_TO_CFCf, FC_RX_1_MUBITS_TO_CFCf, FC_RX_2_MUBITS_TO_CFCf, 
            FC_RX_3_MUBITS_TO_CFCf, FC_RX_4_MUBITS_TO_CFCf, FC_RX_5_MUBITS_TO_CFCf};
    uint32
        reg_val32 = 0,
        fld_val32[1] = {0}, 
        reg_val32_hi[1] = {0},
        reg_val32_lo[1] = {0};
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0);
    uint32
        calendar_id = 0, 
        port = 0;
    uint32
        vsq_group = DNX_TMC_ITM_VSQ_GROUP_CTGRY,
        vsq_id = 0,
        fc_index = 0;
    uint8
        is_ocb_only = 0;
    DNX_TMC_FC_ILKN_MUB_GEN_CAL
        cal_info;
    DNX_TMC_FC_PFC_GENERIC_BITMAP
        pfc_bitmap;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_key);
    DNXC_NULL_CHECK(fc_status_info);

    /* Verify paramters */
    if (fc_status_key->port >= SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("port(%d) is out of range [0, %d]"), fc_status_key->port, (SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports-1)));
    }

    if (fc_status_key->calendar_id >= DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("calendar_id(%d) is out of range [0, %d]"), fc_status_key->calendar_id, (DNX_TMC_FC_ILKN_MUB_GEN_CAL_LEN-1)));
    }

    calendar_id = fc_status_key->calendar_id;
    port = fc_status_key->port;

    if (fc_status_key->direction == DNX_TMC_FC_DIRECTION_GEN) {
        /* Parse tx calendar entry */
        DNXC_IF_ERR_EXIT(jer2_jer_fc_ilkn_mub_gen_cal_get(unit, port, &cal_info));

        fc_index = cal_info.entries[calendar_id].id;

        if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_STE) {
            if (DNX_SAND_FAILURE(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_itm_vsq_index_global2group,(unit, fc_index, &vsq_group, &vsq_id, &is_ocb_only)))) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unit %d, error in vsq id retrieve %d"), unit, fc_index));
            }
            if(unit >= SOC_MAX_NUM_DEVICES) {
                DNXC_EXIT_WITH_ERR(SOC_E_UNIT, (_BSL_DNXC_MSG("ERROR: invalid unit")));
            }
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
            fc_index = vsq_id;
        }
        else if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_STTSTCS_TAG) {
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
            vsq_group = DNX_TMC_ITM_VSQ_GROUP_STTSTCS_TAG;   /* STA VSQ */
        }
        else if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_LLFC_VSQ) {
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
            vsq_group = DNX_TMC_ITM_VSQ_GROUP_LLFC;          /* LLFC VSQ */
        }
        else if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_PFC_VSQ) {
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_VSQ;
            vsq_group = DNX_TMC_ITM_VSQ_GROUP_PFC;           /* PFC VSQ */
        }
        else if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_GLB_RCS) {
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_GLB;
        }
        else if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_LLFC) {
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_NIF;
        }
        else if (cal_info.entries[calendar_id].source == DNX_TMC_FC_GEN_CAL_SRC_HCFC) {
            fc_status_info->cal_tx_src_type = DNX_TMC_FC_SRC_TYPE_HCFC;
        }
        else {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support fc_src(%d)\n"), cal_info.entries[calendar_id].source));
        }

        fc_status_info->cal_tx_src_vsq_type = vsq_group;
        fc_status_info->cal_tx_src_id = fc_index;

        /* IQM & CFC block */
        if (fc_status_info->cal_tx_src_type != DNX_TMC_FC_SRC_TYPE_HCFC) {
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_trigger_status_info_get(unit, fc_status_key, fc_status_info));
        }

        /* 1.3 NBI block*/
        reg = NBIH_FC_TX_MUBITS_FROM_CFCr;
        fld = tx_mub_from_cfc_flds[port];
        if(!SOC_IS_QAX(unit)) {
        DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
        *fld_val32 = soc_reg64_field32_get(unit, reg, reg_val64, fld);
        } else {
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32));
            *fld_val32 = soc_reg_field_get(unit, reg, reg_val32, fld);
        }
        fc_status_info->nbi_mub_tx_from_cfc = SHR_BITGET(fld_val32, calendar_id) ? 1 : 0;
    }
    else {
        if (fc_status_key->calendar_fc_type == DNX_TMC_FC_CALENDAR_TYPE_LLFC) /* LLFC */
        {
            /* 2.2 NBI block */
            reg = NBIH_FC_RX_LLFC_STOP_TX_FROM_MUBITSr;
            fld = rx_llfc_stop_tx_from_mub_flds[port];
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
            *fld_val32 = soc_reg_field_get(unit, reg, reg_val32, fld);
            fc_status_info->nbi_mub_llfc_stop_tx_from_mub = *fld_val32;
        }
        else if (fc_status_key->calendar_fc_type == DNX_TMC_FC_CALENDAR_TYPE_CHFC) /* CHFC */
        {
            fc_status_info->cal_rx_dst_id = port;

            /* 2.2 NBI block */
            reg = NBIH_FC_RX_MUBITS_TO_CFCr;
            fld = rx_mub_to_cfc_flds[port];
            if(!SOC_IS_QAX(unit)) {
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
            *fld_val32 = soc_reg64_field32_get(unit, reg, reg_val64, fld);
            } else {
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32));
                *fld_val32 = soc_reg_field_get(unit, reg, reg_val32, fld);
            }
            fc_status_info->nbi_mub_rx_to_cfc = SHR_BITGET(fld_val32, calendar_id) ? 1 : 0;

            /* 2.3 CFC block */
            reg = CFC_NIF_MUB_STATUSr;
            if(!SOC_IS_QAX(unit)) {
            DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
            *reg_val32_hi = COMPILER_64_HI(reg_val64);
            *reg_val32_lo = COMPILER_64_LO(reg_val64);
            } else {
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32));
                *reg_val32_lo = reg_val32;
            }
            if (port <= 3) {
                fc_status_info->cfc_mub_fc_status = SHR_BITGET(reg_val32_lo, (calendar_id+port*8)) ? 1 : 0;
            }
            else {
                fc_status_info->cfc_mub_fc_status = SHR_BITGET(reg_val32_hi, (calendar_id+port*8-32)) ? 1 : 0;
            }

            /* 2.4 EGQ block */
            DNXC_IF_ERR_EXIT
                (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_fc_pfc_generic_bitmap_get, (unit, port, calendar_id, &pfc_bitmap)));
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, TRUE, pfc_bitmap.core, 0, 0, fc_status_info));
        }
        else {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support calendar_fc_type(%d)\n"), fc_status_key->calendar_fc_type));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

static soc_error_t
  _jer2_jer_fc_oob_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  )
{
    soc_reg_t
        reg = INVALIDr;
    soc_field_t
        fld = INVALIDf, 
        fld_hcfc_crc_err_arr[DNX_TMC_FC_NOF_OOB_IDS][DNX_TMC_FC_HCFC_CHANNEL_NUM] = 
            {{HCFC_OOB_RX_0_CRC_ERR_CTR_0f, 
              HCFC_OOB_RX_0_CRC_ERR_CTR_1f, 
              HCFC_OOB_RX_0_CRC_ERR_CTR_2f,
              HCFC_OOB_RX_0_CRC_ERR_CTR_3f,
              HCFC_OOB_RX_0_CRC_ERR_CTR_4f}, 
             {HCFC_OOB_RX_1_CRC_ERR_CTR_0f, 
              HCFC_OOB_RX_1_CRC_ERR_CTR_1f, 
              HCFC_OOB_RX_1_CRC_ERR_CTR_2f,
              HCFC_OOB_RX_1_CRC_ERR_CTR_3f,
              HCFC_OOB_RX_1_CRC_ERR_CTR_4f}};
    uint32
        reg_val32 = 0,
        fld_val32 = 0,
        reg_val32_hi = 0,
        reg_val32_lo = 0;
    uint64
        reg_val64 = COMPILER_64_INIT(0, 0);
    soc_reg_above_64_val_t
        reg_val_abv64;
    uint32
        calendar_id = 0, 
        port = 0, 
        intf_type = 0;
    DNX_TMC_FC_DIRECTION
        direction = DNX_TMC_FC_DIRECTION_GEN;
    int 
        i = 0,
        num_hcfc_channel = 0;
    uint32
        fc_index = 0,
        q_pair = 0,
        egr_if = 0;
    int 
        core = 0,
        nof_priorities = 0;
    DNX_TMC_FC_PFC_GENERIC_BITMAP
        pfc_bitmap;

    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_key);
    DNXC_NULL_CHECK(fc_status_info);

    /* Verify paramters */
    if (fc_status_key->port >= SOC_DNX_CONFIG(unit)->tm.max_oob_ports) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("port(%d) is out of range [0, %d]"), fc_status_key->port, (SOC_DNX_CONFIG(unit)->tm.max_oob_ports-1)));
    }

    if (fc_status_key->calendar_id >= DNX_TMC_FC_OOB_CAL_LEN_MAX) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("calendar_id(%d) is out of range [0, %d]"), fc_status_key->calendar_id, (DNX_TMC_FC_OOB_CAL_LEN_MAX-1)));
    }

    calendar_id = fc_status_key->calendar_id;
    port = fc_status_key->port;
    intf_type = fc_status_key->intf_type;
    direction = fc_status_key->direction;

    if (fc_status_key->direction == DNX_TMC_FC_DIRECTION_GEN) {
        /* Parse tx calendar entry */
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_parse_tx_cal_tbl(unit, intf_type, TRUE, direction, port, calendar_id,
            &fc_status_info->cal_tx_src_type,
            &fc_status_info->cal_tx_src_vsq_type,
            &fc_status_info->cal_tx_src_id));

        /* IQM & CFC block */
        if (fc_status_info->cal_tx_src_type != DNX_TMC_FC_SRC_TYPE_HCFC) {
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_trigger_status_info_get(unit, fc_status_key, fc_status_info));
        }
    }
    else {
        /* Parse tx calendar entry */
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_parse_rx_cal_tbl(unit, intf_type, TRUE, direction, port, calendar_id,
            &fc_status_info->cal_rx_dst_type,
            &fc_status_info->cal_rx_dst_id));
        fc_index = fc_status_info->cal_rx_dst_id;

        /* CFC & EGQ block */
        if (intf_type == DNX_TMC_FC_INTF_TYPE_ILKN) {
            reg = ((port == 0) ? CFC_ILKN_OOB_RX_0_FC_STATUSr : CFC_ILKN_OOB_RX_1_FC_STATUSr);
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
            fc_status_info->cfc_ilkn_fc_status = SHR_BITGET(reg_val_abv64, calendar_id) ? 1 : 0;
        }

        switch(fc_status_info->cal_rx_dst_type)
        {
        case DNX_TMC_FC_REC_CAL_DEST_NIF_LL:
            if (intf_type == DNX_TMC_FC_INTF_TYPE_SPI) {
                reg = ((port == 0) ?  CFC_SPI_OOB_RX_0_LLFC_STATUSr : CFC_SPI_OOB_RX_1_LLFC_STATUSr);
                SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
                DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
                fc_status_info->cfc_spi_rx_llfc_status = SHR_BITGET(reg_val_abv64, fc_index) ? 1 : 0;
            }

            DNXC_IF_ERR_EXIT(_jer2_jer_fc_first_phy_to_egr_if_get(unit, fc_index, &egr_if));
            reg = EGQ_NIF_FLOW_CONTROLr;
            SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
            DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, core, 0, reg_val_abv64));
            fc_status_info->egq_nif_fc_status[0] = SHR_BITGET(reg_val_abv64, egr_if) ? 1 : 0;

            DNXC_IF_ERR_EXIT(_jer2_jer_fc_llfc_stop_tx_status_info_get(unit, fc_index, fc_status_info));
            break;
        case DNX_TMC_FC_REC_CAL_DEST_PFC:
            if (nof_priorities == 0) nof_priorities = 1;
            /* Intentional fall through */
            /* coverity[unterminated_case] */
        case DNX_TMC_FC_REC_CAL_DEST_PORT_2_PRIORITY:
            if (nof_priorities == 0) nof_priorities = 2;
            /* Intentional fall through */
            /* coverity[unterminated_case] */
        case DNX_TMC_FC_REC_CAL_DEST_PORT_8_PRIORITY:
            if (nof_priorities == 0) nof_priorities = 8;

            if (intf_type == DNX_TMC_FC_INTF_TYPE_SPI) {
                reg = CFC_SPI_OOB_RX_PFC_SELr;
                fld_val32 = (fc_status_info->cal_rx_dst_type == DNX_TMC_FC_REC_CAL_DEST_PFC) ? 0 : 1;
                fld = ((port == 0) ? SPI_OOB_RX_0_PFC_SRC_SELf : SPI_OOB_RX_1_PFC_SRC_SELf);
                soc_reg_field_set(unit, reg, &reg_val32, fld, fld_val32);
                fld_val32 = fc_index/64;
                fld = ((port == 0) ? SPI_OOB_RX_0_PFC_SELf : SPI_OOB_RX_1_PFC_SELf);
                soc_reg_field_set(unit, reg, &reg_val32, fld, fld_val32);
                DNXC_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, 0, reg_val32));

                reg = ((port == 0) ?  CFC_SPI_OOB_RX_0_PFC_STATUSr : CFC_SPI_OOB_RX_1_PFC_STATUSr);
                DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                reg_val32_hi = COMPILER_64_HI(reg_val64);
                reg_val32_lo = COMPILER_64_LO(reg_val64);
                if (fc_index%64 >= 32){
                    SHR_BITCOPY_RANGE(&fc_status_info->cfc_spi_rx_pfc_status, 0, &reg_val32_hi, fc_index%32, nof_priorities);
                }
                else {
                    SHR_BITCOPY_RANGE(&fc_status_info->cfc_spi_rx_pfc_status, 0, &reg_val32_lo, fc_index%32, nof_priorities);
                }
            }

            core = (fc_index >= 256) ? 1 : 0;
            q_pair = fc_index % 256;
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_queue_pair_to_egr_if_get(unit, core, q_pair, &egr_if));

            /* CFC & EGQ & SCH block */
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, FALSE, core, q_pair, nof_priorities, fc_status_info));
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_inf_status_info_get(unit, core, egr_if, fc_status_info));
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_dev_status_info_get(unit, core, fc_status_info));
            break;
        case DNX_TMC_FC_REC_CAL_DEST_GENERIC_PFC:
            if (intf_type == DNX_TMC_FC_INTF_TYPE_SPI) {
                reg = ((port == 0) ?   CFC_SPI_OOB_RX_0_GEN_PFC_STATUSr : CFC_SPI_OOB_RX_1_GEN_PFC_STATUSr);
                SOC_REG_ABOVE_64_CLEAR(reg_val_abv64);
                DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, 0, 0, reg_val_abv64));
                fc_status_info->cfc_spi_rx_gen_pfc_status = SHR_BITGET(reg_val_abv64, calendar_id) ? 1 : 0;
            }

            DNXC_IF_ERR_EXIT
                (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_fc_pfc_generic_bitmap_get, (unit, port + 6, fc_index, &pfc_bitmap)));
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_egq_pfc_status_info_get(unit, TRUE, pfc_bitmap.core, 0, 0, fc_status_info));
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support cal_rx_dst_type(%d)\n"), fc_status_info->cal_rx_dst_type));
            break;
        }

        /* CFC Block - Error status */
        switch(intf_type)
        {
        case DNX_TMC_FC_INTF_TYPE_ILKN: /* ILKN OOB */
            if(!SOC_IS_QAX(unit)) {
                reg = CFC_OOB_RX_ERRr;
                fld = ((port == 0) ? ILKN_OOB_RX_0_ERRf : ILKN_OOB_RX_1_ERRf);
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
                fc_status_info->cfc_ilkn_rx_crc_err_status = soc_reg_field_get(unit, reg, reg_val32, fld);

                reg = ((port == 0) ? CFC_ILKN_0_OOB_RX_LANES_STATUSr : CFC_ILKN_1_OOB_RX_LANES_STATUSr);
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
                fc_status_info->cfc_oob_rx_lanes_status = reg_val32;

                reg = ((port == 0) ? CFC_ILKN_0_OOB_RX_CRC_ERR_CNTr : CFC_ILKN_1_OOB_RX_CRC_ERR_CNTr);
                fld = ((port == 0) ? ILKN_0_OOB_RX_CRC_ERR_CNTRf : ILKN_1_OOB_RX_CRC_ERR_CNTRf);
                DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
                fc_status_info->cfc_ilkn_rx_crc_err_cnt = soc_reg_field_get(unit, reg, reg_val32, fld);
            }
            break;
        case DNX_TMC_FC_INTF_TYPE_SPI: /* SPI OOB */
            reg = CFC_OOB_RX_ERRr;
            fld = ((port == 0) ? SPI_OOB_RX_0_ERRf : SPI_OOB_RX_1_ERRf);
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
            fc_status_info->cfc_spi_rx_crc_err_status = soc_reg_field_get(unit, reg, reg_val32, fld);

            reg = ((port == 0) ? CFC_SPI_OOB_RX_0_ERROR_COUNTERr : CFC_SPI_OOB_RX_1_ERROR_COUNTERr);
            fld = ((port == 0) ? SPI_OOB_RX_0_FRM_ERR_CNTf : SPI_OOB_RX_1_FRM_ERR_CNTf);
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
            fc_status_info->cfc_spi_rx_frame_err_cnt = soc_reg_field_get(unit, reg, reg_val32, fld);

            reg = ((port == 0) ? CFC_SPI_OOB_RX_0_ERROR_COUNTERr : CFC_SPI_OOB_RX_1_ERROR_COUNTERr);
            fld = ((port == 0) ? SPI_OOB_RX_0_DIP_2_ERR_CNTf : SPI_OOB_RX_1_DIP_2_ERR_CNTf);
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
            fc_status_info->cfc_spi_rx_dip_2_err_cnt = soc_reg_field_get(unit, reg, reg_val32, fld);
            break;
        case DNX_TMC_FC_INTF_TYPE_HCFC: /* HCFC OOB */
            reg = CFC_OOB_RX_ERRr;
            fld = ((port == 0) ? HCFC_OOB_RX_0_ERRf : HCFC_OOB_RX_1_ERRf);
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
            fc_status_info->cfc_hcfc_rx_crc_err_status = soc_reg_field_get(unit, reg, reg_val32, fld);

            reg = ((port == 0) ? CFC_HCFC_OOB_RX_0_CRC_ERR_CTRr : CFC_HCFC_OOB_RX_1_CRC_ERR_CTRr);
            num_hcfc_channel = SOC_IS_QAX(unit) ? 4 : 5;
            for (i = 0; i < num_hcfc_channel; i++) {
                DNXC_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                fc_status_info->cfc_hcfc_rx_crc_err_cnt[i] = soc_reg64_field32_get(unit, reg, reg_val64, fld_hcfc_crc_err_arr[port][i]);
            }

            reg = CFC_HCFC_OOB_RX_WD_ERR_STATUSr;
            fld = ((port == 0) ? HCFC_OOB_RX_0_WD_ERR_STATUSf : HCFC_OOB_RX_1_WD_ERR_STATUSf);
            DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, 0, &reg_val32)); 
            fc_status_info->cfc_hcfc_rx_wd_err_status = soc_reg_field_get(unit, reg, reg_val32, fld);
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support")));
            break;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
  jer2_jer_fc_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info
  )
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_NULL_CHECK(fc_status_key);
    DNXC_NULL_CHECK(fc_status_info);

    switch(fc_status_key->intf_type)
    {
    case DNX_TMC_FC_INTF_TYPE_NIF:
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_nif_status_info_get(unit, fc_status_key, fc_status_info));
        break;
    case DNX_TMC_FC_INTF_TYPE_ILKN:
        if (fc_status_key->is_oob) {
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_oob_status_info_get(unit, fc_status_key, fc_status_info));
        }
        else {
            DNXC_IF_ERR_EXIT(_jer2_jer_fc_ilkn_inb_status_info_get(unit, fc_status_key, fc_status_info));
        }
        break;
    case DNX_TMC_FC_INTF_TYPE_MUB:
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_mub_inb_status_info_get(unit, fc_status_key, fc_status_info));
        break;
    case DNX_TMC_FC_INTF_TYPE_SPI:
    case DNX_TMC_FC_INTF_TYPE_HCFC:
        DNXC_IF_ERR_EXIT(_jer2_jer_fc_oob_status_info_get(unit, fc_status_key, fc_status_info));
        break;
    default:
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Not support intf_type(%d)\n"), fc_status_key->intf_type));
        break;
    }

exit:
    DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

