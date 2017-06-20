/*
 * $Id: xmac_drv.c,v 1.23 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC XMAC DRV
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT

#include <shared/bsl.h>
#include <soc/dcmn/error.h>
#include <soc/cm.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/ARAD/NIF/ports_manager.h>
#include <soc/dpp/ARAD/NIF/common_drv.h>
#include <soc/mcm/memregs.h>
#include <soc/cmic.h>
#include <soc/drv.h>
#include <soc/dpp/drv.h>
#include <soc/phyctrl.h>

#define NIF_MAL_NUM_LOGICAL_4_FIFOS_MODE_VAL 0x0
#define NIF_MAL_NUM_LOGICAL_2_FIFOS_MODE_VAL 0x1
#define NIF_MAL_NUM_LOGICAL_1_FIFOS_MODE_VAL 0x2

static soc_field_t port_enable_field[] = {PORT_0f, PORT_1f, PORT_2f, PORT_3f, PORT_4f, PORT_5f, 
        PORT_6f, PORT_7f, PORT_8f, PORT_9f, PORT_10f, PORT_11f};

/*
 * Function:
 *      soc_xmac_drv_init
 * Purpose:
 *      init XMAC port
 * Parameters:
 *      unit     - (IN) Unit number.
 *      port     - (IN) Port number.
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
STATIC soc_error_t 
soc_xmac_drv_init(int unit, soc_port_t port)
{
    soc_reg_t xmac_rx_ctrl, 
                xmac_tx_ctrl, 
                xmac_pause_ctrl,
                xmac_pfc_ctrl,
                xmac_llfc_ctrl,
                xmac_rx_lss_ctrl,
                xmac_rx_max_size,
                xmac_tx_fifo_credits,
                xmac_mode,
                xmac_ctrl;

    uint32 reg_val32[1], 
            flags, 
            quad, 
            is_hg, 
            phy_port, 
            in_block_index, 
            nof_logical_fifos,
            speed_mode,
            fld_val[1],
            core_port_mode,
            cmac_id,
            runt_pad,
            lane;
    int speed;
    uint64 reg_val64;
    soc_pbmp_t quads_in_use, phy_pbmp;
    soc_port_if_t interface_type;
    soc_reg_above_64_val_t reg_val_a64, field_val_a64;
    SOCDNX_INIT_FUNC_DEFS;

    /*don't touch NBI in stat interface*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit,port,&flags));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phy_pbmp));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_in_block_index_get(unit, port, &in_block_index));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_hg_get(unit, port, &is_hg));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_get(unit, port, &speed));
    SOCDNX_IF_ERR_EXIT(soc_common_drv_core_port_mode_get(unit, port, &core_port_mode));

    SOCDNX_IF_ERR_EXIT(soc_pm_serdes_quads_in_use_get(unit,port,&quads_in_use));
    SOC_PBMP_ITER(quads_in_use, quad) {break;};

    /* WrapCore in non-Bypass mode*/
    if(!SOC_PORT_IS_STAT_INTERFACE(flags)) {
        SOCDNX_IF_ERR_EXIT(soc_common_drv_bypass_mode_set(unit,port,0));
    }

    /*FIFOs mode*/
    if(!SOC_PORT_IS_STAT_INTERFACE(flags)) {
        switch (core_port_mode) {
            case 0: nof_logical_fifos = NIF_MAL_NUM_LOGICAL_1_FIFOS_MODE_VAL; break;
            case 1: nof_logical_fifos = NIF_MAL_NUM_LOGICAL_2_FIFOS_MODE_VAL; break;
            case 2: nof_logical_fifos = NIF_MAL_NUM_LOGICAL_4_FIFOS_MODE_VAL; break;
            default:
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Invalid core_port_mode %d"), core_port_mode));
                break;

        }

        SOCDNX_IF_ERR_EXIT(READ_NBI_RX_MLF_CONFIGr(unit, quad, reg_val32));
        soc_reg_field_set(unit, NBI_RX_MLF_CONFIGr, reg_val32, RX_N_NUM_LOGICAL_FIFOS_MODEf, nof_logical_fifos);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_RX_MLF_CONFIGr(unit, quad, *reg_val32));

        SOCDNX_IF_ERR_EXIT(READ_NBI_TX_MLF_CONFIGr(unit, quad, reg_val32));
        soc_reg_field_set(unit, NBI_TX_MLF_CONFIGr, reg_val32, TX_N_NUM_LOGICAL_FIFOS_MODEf, nof_logical_fifos);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_TX_MLF_CONFIGr(unit, quad, *reg_val32));
    }

    if(SOC_PORT_IS_STAT_INTERFACE(flags)) {
        SOCDNX_IF_ERR_EXIT(READ_XLP_XMAC_RX_LSS_CTRLr(unit, port, reg_val32));
        soc_reg_field_set(unit, XLP_XMAC_RX_LSS_CTRLr, reg_val32, LOCAL_FAULT_DISABLEf, 1);
        soc_reg_field_set(unit, XLP_XMAC_RX_LSS_CTRLr, reg_val32, REMOTE_FAULT_DISABLEf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_XLP_XMAC_RX_LSS_CTRLr(unit, port, *reg_val32));
    }

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_rx_ctrl = CLP_XMAC_RX_CTRLr;
        xmac_tx_ctrl = CLP_XMAC_TX_CTRLr;
        xmac_pause_ctrl = CLP_XMAC_PAUSE_CTRLr;
        xmac_pfc_ctrl = CLP_XMAC_PFC_CTRLr;
        xmac_llfc_ctrl = CLP_XMAC_LLFC_CTRLr;
        xmac_rx_lss_ctrl = CLP_XMAC_RX_LSS_CTRLr;
        xmac_rx_max_size = CLP_XMAC_RX_MAX_SIZEr;
        xmac_tx_fifo_credits = CLP_XMAC_TX_FIFO_CREDITSr;
        xmac_mode = CLP_XMAC_MODEr;
        xmac_ctrl = CLP_XMAC_CTRLr;
    } else {
        xmac_rx_ctrl = XLP_XMAC_RX_CTRLr;
        xmac_tx_ctrl = XLP_XMAC_TX_CTRLr;
        xmac_pause_ctrl = XLP_XMAC_PAUSE_CTRLr;
        xmac_pfc_ctrl = XLP_XMAC_PFC_CTRLr;
        xmac_llfc_ctrl = XLP_XMAC_LLFC_CTRLr;
        xmac_rx_lss_ctrl = XLP_XMAC_RX_LSS_CTRLr;
        xmac_rx_max_size = XLP_XMAC_RX_MAX_SIZEr;
        xmac_tx_fifo_credits = XLP_XMAC_TX_FIFO_CREDITSr;
        xmac_mode = XLP_XMAC_MODEr;
        xmac_ctrl = XLP_XMAC_CTRLr;
    }

    /*XAMC RX control*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_ctrl, port, 0, reg_val32));
    soc_reg_field_set(unit, xmac_rx_ctrl, reg_val32, STRIP_CRCf, 1);
    soc_reg_field_set(unit, xmac_rx_ctrl, reg_val32, PROCESS_VARIABLE_PREAMBLEf, is_hg ? 0 : 1);
    soc_reg_field_set(unit, xmac_rx_ctrl, reg_val32, STRICT_PREAMBLEf, is_hg ? 0 : 1);
    soc_reg_field_set(unit, xmac_rx_ctrl, reg_val32, RUNT_THRESHOLDf, 64);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_rx_ctrl, port, 0, *reg_val32));

    /*XAMC TX control*/
    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_tx_ctrl, port, 0, &reg_val64));
    soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, CRC_MODEf, 0);
    soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, AVERAGE_IPGf, is_hg ? NIF_AVERAGE_IPG_HIGIG : NIF_AVERAGE_IPG_DEFAULT);
    soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, TX_PREAMBLE_LENGTHf, NIF_TX_PREAMBLE_LENGTH_DEFAULT);
    soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, TX_64_BYTE_BUFFER_ENf, 1);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_runt_pad_get(unit, port, &runt_pad));
    soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, PAD_ENf, runt_pad ? 1 : 0);
    if(runt_pad) {
        soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, PAD_THRESHOLDf, runt_pad);
    }

    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_tx_ctrl, port, 0, reg_val64));

    /*Pause control*/
    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pause_ctrl, port, 0, &reg_val64));
    soc_reg64_field32_set(unit, xmac_pause_ctrl, &reg_val64, RX_PAUSE_ENf, 1);
    soc_reg64_field32_set(unit, xmac_pause_ctrl, &reg_val64, TX_PAUSE_ENf, 0);
    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_pause_ctrl, port, 0, reg_val64));

    /*PFC control*/
    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
    soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, RX_PFC_ENf, 1);
    soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, TX_PFC_ENf, 0);
    soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, PFC_STATS_ENf, 1);
    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_pfc_ctrl, port, 0, reg_val64));

    SOCDNX_IF_ERR_EXIT(READ_PORT_CONFIGr(unit, port, reg_val32));
    soc_reg_field_set(unit, PORT_CONFIGr, reg_val32, PFC_ENABLEf, 1);
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_CONFIGr(unit, port, *reg_val32));

    /*stop TX signals are not working for ARAD (known issue of timing violation)*/
    SOCDNX_IF_ERR_EXIT(READ_NBI_FC_LLFC_STOP_TX_FROM_CFC_MASKr(unit, reg_val32));
    SHR_BITCLR(reg_val32, (phy_port-1));
    SOCDNX_IF_ERR_EXIT(WRITE_NBI_FC_LLFC_STOP_TX_FROM_CFC_MASKr(unit, *reg_val32));

    SOC_REG_ABOVE_64_CLEAR(field_val_a64);
    SOCDNX_IF_ERR_EXIT(READ_NBI_FC_PFC_RX_BITMAP_TYPE_Ar(unit, reg_val_a64));
    soc_reg_above_64_field_set(unit, NBI_FC_PFC_RX_BITMAP_TYPE_Ar, reg_val_a64, FC_LLFC_STOP_TX_FROM_PFC_TX_BITMAP_TYPE_Af, field_val_a64);
    SOCDNX_IF_ERR_EXIT(WRITE_NBI_FC_PFC_RX_BITMAP_TYPE_Ar(unit, reg_val_a64));

    SOCDNX_IF_ERR_EXIT(READ_NBI_FC_PFC_RX_BITMAP_TYPE_Br(unit, reg_val_a64));
    soc_reg_above_64_field_set(unit, NBI_FC_PFC_RX_BITMAP_TYPE_Br, reg_val_a64, FC_LLFC_STOP_TX_FROM_PFC_TX_BITMAP_TYPE_Bf, field_val_a64);
    SOCDNX_IF_ERR_EXIT(WRITE_NBI_FC_PFC_RX_BITMAP_TYPE_Br(unit, reg_val_a64));

    /*LLFC*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_llfc_ctrl, port, 0, reg_val32));
    soc_reg_field_set(unit, xmac_llfc_ctrl, reg_val32, RX_LLFC_ENf, 0);
    soc_reg_field_set(unit, xmac_llfc_ctrl, reg_val32, TX_LLFC_ENf, 0);
    soc_reg_field_set(unit, xmac_llfc_ctrl, reg_val32, LLFC_IN_IPG_ONLYf, 0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_llfc_ctrl, port, 0, *reg_val32));

    SOCDNX_IF_ERR_EXIT(READ_PORT_CONFIGr(unit, port, reg_val32));
    soc_reg_field_set(unit, PORT_CONFIGr, reg_val32, LLFC_ENf, 0);
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_CONFIGr(unit, port, *reg_val32));

    /*LSS*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_ctrl, port, 0, reg_val32));
    soc_reg_field_set(unit, xmac_rx_lss_ctrl, reg_val32, DROP_TX_DATA_ON_LOCAL_FAULTf, 1);
    soc_reg_field_set(unit, xmac_rx_lss_ctrl, reg_val32, DROP_TX_DATA_ON_REMOTE_FAULTf, 1);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_rx_lss_ctrl, port, 0, *reg_val32));

    /*RX MAX size*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_max_size, port, 0, reg_val32));
    soc_reg_field_set(unit, xmac_rx_max_size, reg_val32, RX_MAX_SIZEf, NIF_PACKET_MAX_SIZE);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_rx_max_size, port, 0, *reg_val32));

    SOCDNX_IF_ERR_EXIT(READ_PORT_CNTMAXSIZEr(unit, port, reg_val32));
    soc_reg_field_set(unit, PORT_CNTMAXSIZEr, reg_val32, CNTMAXSIZEf, soc_property_get(unit, spn_BCM_STAT_JUMBO, 1518));
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_CNTMAXSIZEr(unit, port, *reg_val32));

    /*TX FIFO credits*/
    if(SOC_PORT_IF_SGMII == interface_type) {
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_tx_fifo_credits, port, 0, reg_val32));
        soc_reg_field_set(unit, xmac_tx_fifo_credits, reg_val32, QUAD_PORT_TX_CREDITSf, 6);
        soc_reg_field_set(unit, xmac_tx_fifo_credits, reg_val32, DUAL_PORT_TX_CREDITSf, 10);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_tx_fifo_credits, port, 0, *reg_val32));
    }

    /*MAC mode*/
    if(SOC_PORT_IF_SGMII == interface_type) {
        switch(speed) {
            case 10:
                speed_mode = NIF_XMAC_SPEED_10;
                break;
            case 100:
                speed_mode = NIF_XMAC_SPEED_100;
                break;
            case 1000:
                speed_mode = NIF_XMAC_SPEED_1000;
                break;
            default:
                speed_mode = NIF_XMAC_SPEED_2500;
                break;
        }
    } else {
        speed_mode = NIF_XMAC_SPEED_10000;
    }
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_mode, port, 0, reg_val32));
    soc_reg_field_set(unit, xmac_mode, reg_val32, SPEED_MODEf, speed_mode);
    SOCDNX_IF_ERR_EXIT(soc_common_drv_mac_mode_reg_hdr_set(unit, port, xmac_mode, reg_val32));
    soc_reg_field_set(unit, xmac_mode, reg_val32, NO_SOP_FOR_CRC_HGf, 0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_mode, port, 0, *reg_val32));

    /* configure_err_mask_in_mac */
    /* RUNT detected = bit 17, Truncation/Frame Length Out of Range = bit 6, CRC_ERR = bit 4, RECEIVE TERMINATE ERROR = bit 3 */
    SOCDNX_IF_ERR_EXIT(WRITE_MAC_RSV_MASKr(unit, port, 0x20058));

    /*MAC control*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_ctrl, port, 0, reg_val32));
    soc_reg_field_set(unit, xmac_ctrl, reg_val32, TX_ENf, 1);
    soc_reg_field_set(unit, xmac_ctrl, reg_val32, RX_ENf, 1);
    soc_reg_field_set(unit, xmac_ctrl, reg_val32, SOFT_RESETf, 0);
    soc_reg_field_set(unit, xmac_ctrl, reg_val32, XLGMII_ALIGN_ENBf, (SOC_PORT_IF_XLAUI == interface_type && !is_hg) ? 1 : 0);
    soc_reg_field_set(unit, xmac_ctrl, reg_val32, XGMII_IPG_CHECK_DISABLEf, 1);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_ctrl, port, 0, *reg_val32));

    /*fault report*/
    SOCDNX_IF_ERR_EXIT(READ_PORT_FAULT_LINK_STATUSr(unit, port, reg_val32));
    soc_reg_field_set(unit, PORT_FAULT_LINK_STATUSr, reg_val32, LOCAL_FAULTf , 1);
    soc_reg_field_set(unit, PORT_FAULT_LINK_STATUSr, reg_val32, REMOTE_FAULTf , 1);
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_FAULT_LINK_STATUSr(unit, port, *reg_val32));

    /*Fast port enable*/
    SOCDNX_IF_ERR_EXIT(soc_common_drv_fast_port_set(unit, port, 0, 0));
    
    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        /*turn off HRF*/
        cmac_id = (phy_port > 12 ? 1 : 0);

        if(in_block_index < 8) { /*in ILKN 8 first lanes always associate with HRF*/
            SOCDNX_IF_ERR_EXIT(READ_NBI_ENABLE_INTERLAKENr(unit, reg_val32));
            soc_reg_field_set(unit, NBI_ENABLE_INTERLAKENr, reg_val32, (0 == cmac_id) ? ENABLE_HRF_0f : ENABLE_HRF_1f, 0);
            soc_reg_field_set(unit, NBI_ENABLE_INTERLAKENr, reg_val32, (0 == cmac_id) ? USE_HRF_FOR_ILKN_0f : USE_HRF_FOR_ILKN_1f, 0);
            SOCDNX_IF_ERR_EXIT(WRITE_NBI_ENABLE_INTERLAKENr(unit, *reg_val32));

            SOCDNX_IF_ERR_EXIT(READ_NBI_ILKN_RESETr(unit, reg_val32));
            soc_reg_field_set(unit, NBI_ILKN_RESETr, reg_val32, (0 == cmac_id) ? HRF_RX_0_CONTROLLER_RSTNf : HRF_RX_1_CONTROLLER_RSTNf, 0);
            soc_reg_field_set(unit, NBI_ILKN_RESETr, reg_val32, (0 == cmac_id) ? HRF_TX_0_CONTROLLER_RSTNf : HRF_TX_1_CONTROLLER_RSTNf, 0);
            SOCDNX_IF_ERR_EXIT(WRITE_NBI_ILKN_RESETr(unit, *reg_val32));
        }

        /*NBI FC*/
        SOCDNX_IF_ERR_EXIT(READ_NBI_FC_ILKNr(unit, reg_val32));
        *fld_val = soc_reg_field_get(unit, NBI_FC_ILKNr, *reg_val32, FC_CMAL_MODEf);
        *fld_val &= ~(1<<cmac_id);
        soc_reg_field_set(unit, NBI_FC_ILKNr, reg_val32, FC_CMAL_MODEf, *fld_val);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_FC_ILKNr(unit, *reg_val32));
    }

    /*MLF*/
    if(!SOC_PORT_IS_STAT_INTERFACE(flags)) {

        SOCDNX_IF_ERR_EXIT(READ_NBI_RX_MLF_RESET_PORTSr(unit, reg_val32));
        *fld_val = soc_reg_field_get(unit, NBI_RX_MLF_RESET_PORTSr, *reg_val32, RX_PORTS_SRSTNf);

        /* reset rx all lanes */
        SOC_PBMP_ITER(phy_pbmp, lane) {
            if (lane > 0) {
                SHR_BITCLR(fld_val, (lane-1));
            }
        }
        soc_reg_field_set(unit, NBI_RX_MLF_RESET_PORTSr, reg_val32, RX_PORTS_SRSTNf, *fld_val);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_RX_MLF_RESET_PORTSr(unit, *reg_val32));

        /* set rx first phy_port */
        SHR_BITSET(fld_val, (phy_port-1));
        soc_reg_field_set(unit, NBI_RX_MLF_RESET_PORTSr, reg_val32, RX_PORTS_SRSTNf, *fld_val);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_RX_MLF_RESET_PORTSr(unit, *reg_val32));

        SOCDNX_IF_ERR_EXIT(READ_NBI_TX_MLF_RESET_PORTSr(unit, reg_val32));
        *fld_val = soc_reg_field_get(unit, NBI_TX_MLF_RESET_PORTSr, *reg_val32, TX_PORTS_SRSTNf);

        /* reset tx all lanes */
        SOC_PBMP_ITER(phy_pbmp, lane) {
            if (lane > 0) {
                SHR_BITCLR(fld_val, (lane-1));
            }
        }
        soc_reg_field_set(unit, NBI_TX_MLF_RESET_PORTSr, reg_val32, TX_PORTS_SRSTNf, *fld_val);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_TX_MLF_RESET_PORTSr(unit, *reg_val32));

        /* set tx first phy_port */
        SHR_BITSET(fld_val, (phy_port-1));
        soc_reg_field_set(unit, NBI_TX_MLF_RESET_PORTSr, reg_val32, TX_PORTS_SRSTNf, *fld_val);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_TX_MLF_RESET_PORTSr(unit, *reg_val32));
    }

    /*port enable*/
    SOCDNX_IF_ERR_EXIT(READ_PORT_ENABLE_REGr(unit, port, reg_val32));
    soc_reg_field_set(unit, PORT_ENABLE_REGr, reg_val32, port_enable_field[in_block_index] , 1);
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_ENABLE_REGr(unit, port, *reg_val32));

    /*port meter*/
    SOCDNX_IF_ERR_EXIT(WRITE_NBI_PORT_METER_RSTNr(unit, 0));

    /*Buffer enable*/
    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_tx_ctrl, port, 0, &reg_val64));
    soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, TX_64_BYTE_BUFFER_ENf, 1);
    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_tx_ctrl, port, 0, reg_val64));

    /*Reset counters*/
    *reg_val32=0;
    SHR_BITSET(reg_val32, in_block_index);
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_MIB_RESETr(unit, port, *reg_val32));
    *reg_val32=0;
    SOCDNX_IF_ERR_EXIT(WRITE_PORT_MIB_RESETr(unit, port, *reg_val32));

    /*WA for Arad A0: no parity ERROR protection for MLFs*/
    if (SOC_IS_ARAD_A0(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NBI_MASK_ECC_PARITY_ERROR_FROM_DATA_PATHr(unit, reg_val32));
        soc_reg_field_set(unit, NBI_MASK_ECC_PARITY_ERROR_FROM_DATA_PATHr, reg_val32, RX_MASK_PAR_ERR_DATA_MEMf, 0);
        SOCDNX_IF_ERR_EXIT(WRITE_NBI_MASK_ECC_PARITY_ERROR_FROM_DATA_PATHr(unit, *reg_val32));
    }

    /* ARAD+ 1588 48b stamping */
#ifdef BCM_88660_A0
    if(SOC_IS_ARADPLUS(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_PORT_MODE_REGr(unit, port, reg_val32));
        soc_reg_field_set(unit, PORT_MODE_REGr, reg_val32, EGR_1588_TIMESTAMPING_MODEf, (SOC_DPP_CONFIG(unit))->pp.ptp_48b_stamping_enable ? 1 : 0);
        SOCDNX_IF_ERR_EXIT(WRITE_PORT_MODE_REGr(unit, port, *reg_val32));
     }
#endif /* BCM_88660_A0 */

exit:
    SOCDNX_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_xmac_drv_enable_set
 * Purpose:
 *      Enable or disable MAC
 * Parameters:
 *      unit - unit number.
 *      port - Port number.
 *      enable - TRUE to enable, FALSE to disable
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_enable_set(int unit, soc_port_t port, int enable)
{
    uint32 ctrl, octrl;
    uint32 reg_val, first_phy_port, flags;
    soc_reg_t xmac_ctrl;
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy_port));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit,port,&flags));

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_ctrl = CLP_XMAC_CTRLr;
    } else {
        xmac_ctrl = XLP_XMAC_CTRLr;
    }
    if (!enable) {  
         SOCDNX_IF_ERR_EXIT(READ_PORT_TXFIFO_PKT_DROP_CTLr(unit, port, &reg_val));
         soc_reg_field_set(unit, PORT_TXFIFO_PKT_DROP_CTLr, &reg_val, DROP_ENf, 1);
         SOCDNX_IF_ERR_EXIT(WRITE_PORT_TXFIFO_PKT_DROP_CTLr(unit, port, reg_val));
    }

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_ctrl, port, 0, &ctrl));
    octrl = ctrl;
    /* Don't disable TX since it stops egress and hangs if CPU sends */
    soc_reg_field_set(unit, xmac_ctrl, &ctrl, TX_ENf, 1);
    soc_reg_field_set(unit, xmac_ctrl, &ctrl, RX_ENf, enable ? 1 : 0);
    /* Put the XMAC in a soft reset state when disabled to allow packets to drain from the TX FIFO */ 
    soc_reg_field_set(unit, xmac_ctrl, &ctrl, SOFT_RESETf, enable ? 0 : 1); 
    if (ctrl != octrl) {
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_ctrl, port, 0, ctrl));
    }

    if (enable) {  
         SOCDNX_IF_ERR_EXIT(READ_PORT_TXFIFO_PKT_DROP_CTLr(unit, port, &reg_val));
         soc_reg_field_set(unit, PORT_TXFIFO_PKT_DROP_CTLr, &reg_val, DROP_ENf, 0);
         SOCDNX_IF_ERR_EXIT(WRITE_PORT_TXFIFO_PKT_DROP_CTLr(unit, port, reg_val));
    }

exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_enable_get
 * Purpose:
 *      Get current XMAC enable setting.
 * Parameters:
 *      unit - unit number.
 *      port - Port number.
 *      enable - (OUT) TRUE is enabled, FALSE is disabled
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_enable_get(int unit, soc_port_t port, int *enable)
{
    uint32 reg_val;
    int is_soft_reset;
    soc_reg_t ctrl;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        ctrl = CLP_XMAC_CTRLr;
    } else {
        ctrl = XLP_XMAC_CTRLr;
    }

    /* mac is disabled when soft reset is on */
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, ctrl, port, 0, &reg_val));
    is_soft_reset = soc_reg_field_get(unit, ctrl, reg_val, SOFT_RESETf);
    *enable = !is_soft_reset;

exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_speed_set
 * Purpose:
 *      Set XMAC in the specified speed.
 * Parameters:
 *      unit - unit number.
 *      port - port number.
 *      speed - speed to set
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_speed_set(int unit, soc_port_t port, int speed)
{
    soc_port_if_t interface_type;
    uint32 reg_val32, core_port_mode, phy_port_mode, num_of_lanes,
        internal_quad, speed_mode, in_block_index, prev_reg_32;
    soc_reg_t xmac_mode, xmac_tx_fifo_credits;
    int is_mac_enabled, is_need_to_enable_mac = 0;
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    SOCDNX_IF_ERR_EXIT(soc_common_drv_core_port_mode_get(unit,port,&core_port_mode));
    SOCDNX_IF_ERR_EXIT(soc_common_drv_phy_port_mode_get(unit,port,&phy_port_mode));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_of_lanes));

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_mode = CLP_XMAC_MODEr;
        xmac_tx_fifo_credits = CLP_XMAC_TX_FIFO_CREDITSr;
    } else {
        xmac_mode = XLP_XMAC_MODEr;
        xmac_tx_fifo_credits = XLP_XMAC_TX_FIFO_CREDITSr;
    }

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_in_block_index_get(unit, port, &in_block_index));
    internal_quad = in_block_index/LANES_IN_QUAD;

    SOCDNX_IF_ERR_EXIT(soc_xmac_drv_enable_get(unit, port, &is_mac_enabled));

    /*Port mode*/
    SOCDNX_IF_ERR_EXIT(READ_PORT_MODE_REGr(unit, port, &reg_val32));
    prev_reg_32 = reg_val32;
    soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, MAC_MODEf, MAC_MODE_INDEP);
    switch (internal_quad) {
        case 0:
            soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, XPORT_0_PHY_PORT_MODEf, phy_port_mode);
            soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, XPORT_0_CORE_PORT_MODEf, core_port_mode);
            break;
        case 1:
            soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, XPORT_1_PHY_PORT_MODEf, phy_port_mode);
            soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, XPORT_1_CORE_PORT_MODEf, core_port_mode);
            break;
        case 2:
            soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, XPORT_2_PHY_PORT_MODEf, phy_port_mode);
            soc_reg_field_set(unit, PORT_MODE_REGr, &reg_val32, XPORT_2_CORE_PORT_MODEf, core_port_mode);
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Wrong internal quad id %d"), internal_quad));
            break;
    }
    if(reg_val32 != prev_reg_32) {
        /* Disable MAC when it is not set correctly */
        if (is_mac_enabled) {
            SOCDNX_IF_ERR_EXIT(soc_xmac_drv_enable_set(unit, port, FALSE));
            is_mac_enabled = 0;
            is_need_to_enable_mac = 1;
        }

        SOCDNX_IF_ERR_EXIT(WRITE_PORT_MODE_REGr(unit, port, reg_val32));
    }

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_tx_fifo_credits, port, 0, &reg_val32));
    prev_reg_32 = reg_val32;
    if(1 == num_of_lanes && 10000 > speed) {
        soc_reg_field_set(unit, xmac_tx_fifo_credits, &reg_val32, QUAD_PORT_TX_CREDITSf, 6);
        soc_reg_field_set(unit, xmac_tx_fifo_credits, &reg_val32, DUAL_PORT_TX_CREDITSf, 10);
        
    } else {
        soc_reg_field_set(unit, xmac_tx_fifo_credits, &reg_val32, QUAD_PORT_TX_CREDITSf, 8);
        soc_reg_field_set(unit, xmac_tx_fifo_credits, &reg_val32, DUAL_PORT_TX_CREDITSf, 12);
    }
    if(reg_val32 != prev_reg_32) {
        /* Disable MAC when it is not set correctly */
        if (is_mac_enabled) {
            SOCDNX_IF_ERR_EXIT(soc_xmac_drv_enable_set(unit, port, FALSE));
            is_mac_enabled = 0;
            is_need_to_enable_mac = 1;
        }

        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_tx_fifo_credits, port, 0, reg_val32));
    }

    /*speed mode*/
    if(1 == num_of_lanes) {
        switch(speed) {
            case 10:
                speed_mode = NIF_XMAC_SPEED_10;
                break;
            case 100:
                speed_mode = NIF_XMAC_SPEED_100;
                break;
            case 1000:
                speed_mode = NIF_XMAC_SPEED_1000;
                break;
            case 2500:
                speed_mode = NIF_XMAC_SPEED_2500;
                break;
            default:
                speed_mode = NIF_XMAC_SPEED_10000;
                break;
        }
    } else {
        speed_mode = NIF_XMAC_SPEED_10000;
    }
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_mode, port, 0, &reg_val32));
    prev_reg_32 = reg_val32;
    soc_reg_field_set(unit, xmac_mode, &reg_val32, SPEED_MODEf, speed_mode);
    SOCDNX_IF_ERR_EXIT(soc_common_drv_mac_mode_reg_hdr_set(unit, port, xmac_mode, &reg_val32));
    soc_reg_field_set(unit, xmac_mode, &reg_val32, NO_SOP_FOR_CRC_HGf, 0);
    if(reg_val32 != prev_reg_32) {
        /* Disable MAC when it is not set correctly */
        if (is_mac_enabled) {
            SOCDNX_IF_ERR_EXIT(soc_xmac_drv_enable_set(unit, port, FALSE));
            is_mac_enabled = 0;
            is_need_to_enable_mac = 1;
        }

        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_mode, port, 0, reg_val32));
    }


    /* 
    * Notify internal PHY driver of speed change in case it is being 
    * used as pass-through to an external PHY. 
    */ 
    if (! PHY_REPEATER(unit, port)) { 
        SOC_IF_ERROR_RETURN(soc_phyctrl_notify(unit, port, phyEventSpeed, speed)); 
    } 

    /* Enable MAC back if was disabled */
    if (is_need_to_enable_mac) {
        SOCDNX_IF_ERR_EXIT(soc_xmac_drv_enable_set(unit, port, TRUE));
    }

exit:
    SOCDNX_FUNC_RETURN
}


/*
 * Function:
 *      soc_xmac_drv_loopback_set
 * Purpose:
 *      Set a XMAC into/out-of loopback mode
 * Parameters:
 *      unit - unit number.
 *      port - port number.
 *      loopback - Boolean: true -> loopback mode, false -> normal operation
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_loopback_set(int unit, soc_port_t port, int lb)
{
    uint32 reg_val;
    soc_reg_t ctrl;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        ctrl = CLP_XMAC_CTRLr;
    } else {
        ctrl = XLP_XMAC_CTRLr;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, ctrl, port, 0, &reg_val));
    soc_reg_field_set(unit, ctrl, &reg_val, LINE_LOCAL_LPBKf, lb ? 1 : 0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, ctrl, port, 0, reg_val));

exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_loopback_get
 * Purpose:
 *      Get current XMAC loopback mode setting.
 * Parameters:
 *      unit - unit number.
 *      port - port number.
 *      loopback - (OUT) Boolean: true = loopback, false = normal
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_loopback_get(int unit, soc_port_t port, int *lb)
{
    uint32 reg_val;
    soc_reg_t ctrl;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        ctrl = CLP_XMAC_CTRLr;
    } else {
        ctrl = XLP_XMAC_CTRLr;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, ctrl, port, 0, &reg_val));
    *lb = soc_reg_field_get(unit, ctrl, reg_val, LINE_LOCAL_LPBKf);

exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_frame_max_set
 * Description:
 *      Set the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
STATIC int
soc_xmac_drv_frame_max_set(int unit, soc_port_t port, int size)
{
    soc_reg_t xmac_rx_max_size;
    uint32 reg_val32;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_rx_max_size = CLP_XMAC_RX_MAX_SIZEr;
    } else {
        xmac_rx_max_size = XLP_XMAC_RX_MAX_SIZEr;
    }

    /*RX MAX size*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_max_size, port, 0, &reg_val32));
    soc_reg_field_set(unit, xmac_rx_max_size, &reg_val32, RX_MAX_SIZEf, size);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_rx_max_size, port, 0, reg_val32));

exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_frame_max_get
 * Description:
 *      Get the maximum receive frame size for the port
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      size - Maximum frame size in bytes
 * Return Value:
 *      BCM_E_XXX
 */
STATIC int
soc_xmac_drv_frame_max_get(int unit, soc_port_t port, int *size)
{
    soc_reg_t xmac_rx_max_size;
    uint32 reg_val32;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_rx_max_size = CLP_XMAC_RX_MAX_SIZEr;
    } else {
        xmac_rx_max_size = XLP_XMAC_RX_MAX_SIZEr;
    }

    /*RX MAX size*/
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_max_size, port, 0, &reg_val32));
    (*size) = soc_reg_field_get(unit, xmac_rx_max_size, reg_val32, RX_MAX_SIZEf);

exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_control_set
 * Purpose:
 *      To set MAC control setting.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      type - MAC control property to set.
 *      int  - New setting for MAC control.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_control_set(int unit, soc_port_t port, soc_mac_control_t type,
                  int value)
{
    soc_reg_t xmac_clear_rx_lss_status, xmac_tx_ctrl, xmac_pfc_ctrl, xmac_eee_ctrl, xmac_eee_timers, xmac_eee_1_sec_link_status_timer, xmac_llfc_ctrl,
        xmac_rx_lss_ctrl;
    uint32 rval;
    uint64 reg_val64;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_clear_rx_lss_status = CLP_XMAC_CLEAR_RX_LSS_STATUSr;
        xmac_tx_ctrl = CLP_XMAC_TX_CTRLr;
        xmac_pfc_ctrl = CLP_XMAC_PFC_CTRLr;
        xmac_llfc_ctrl = CLP_XMAC_LLFC_CTRLr;
        xmac_eee_ctrl = CLP_XMAC_EEE_CTRLr;
        xmac_eee_timers = CLP_XMAC_EEE_TIMERSr;
        xmac_eee_1_sec_link_status_timer = CLP_XMAC_EEE_1_SEC_LINK_STATUS_TIMERr;
        xmac_rx_lss_ctrl = CLP_XMAC_RX_LSS_CTRLr;
    } else {
        xmac_clear_rx_lss_status = XLP_XMAC_CLEAR_RX_LSS_STATUSr;
        xmac_tx_ctrl = XLP_XMAC_TX_CTRLr;
        xmac_pfc_ctrl = XLP_XMAC_PFC_CTRLr;
        xmac_llfc_ctrl = XLP_XMAC_LLFC_CTRLr;
        xmac_eee_ctrl = XLP_XMAC_EEE_CTRLr;
        xmac_eee_timers = XLP_XMAC_EEE_TIMERSr;
        xmac_eee_1_sec_link_status_timer = XLP_XMAC_EEE_1_SEC_LINK_STATUS_TIMERr;
        xmac_rx_lss_ctrl = XLP_XMAC_RX_LSS_CTRLr;
    }

    switch (type) {
        case SOC_MAC_CONTROL_FAULT_LOCAL_STATUS:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_clear_rx_lss_status, port, 0 , &rval));
            soc_reg_field_set(unit, xmac_clear_rx_lss_status, &rval, CLEAR_LOCAL_FAULT_STATUSf, 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_clear_rx_lss_status, port, 0 , rval));
            soc_reg_field_set(unit, xmac_clear_rx_lss_status, &rval, CLEAR_LOCAL_FAULT_STATUSf, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_clear_rx_lss_status, port, 0 , rval));
            break;

        case SOC_MAC_CONTROL_FAULT_REMOTE_STATUS:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_clear_rx_lss_status, port, 0 , &rval));
            soc_reg_field_set(unit, xmac_clear_rx_lss_status, &rval, CLEAR_REMOTE_FAULT_STATUSf, 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_clear_rx_lss_status, port, 0 , rval));
            soc_reg_field_set(unit, xmac_clear_rx_lss_status, &rval, CLEAR_REMOTE_FAULT_STATUSf, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_clear_rx_lss_status, port, 0 , rval));
            break;

        case SOC_MAC_CONTROL_FAULT_LOCAL_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_ctrl, port, 0 , &rval));
            soc_reg_field_set(unit, xmac_rx_lss_ctrl, &rval, LOCAL_FAULT_DISABLEf, value ? 0 : 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_rx_lss_ctrl, port, 0 , rval));
            break;

        case SOC_MAC_CONTROL_FAULT_REMOTE_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_ctrl, port, 0 , &rval));
            soc_reg_field_set(unit, xmac_rx_lss_ctrl, &rval, REMOTE_FAULT_DISABLEf, value ? 0 : 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_rx_lss_ctrl, port, 0 , rval));
            break;

        case SOC_MAC_CONTROL_PAD_SIZE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_tx_ctrl, port, 0, &reg_val64));
            soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, PAD_ENf, value ? 1 : 0);
            if(value) {
                soc_reg64_field32_set(unit, xmac_tx_ctrl, &reg_val64, PAD_THRESHOLDf, value);
            }
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_tx_ctrl, port, 0, reg_val64));
            break;

        case SOC_MAC_CONTROL_PFC_REFRESH_TIME:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
            soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, PFC_REFRESH_ENf, value ? 1 : 0);
            if(value) {
                soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, PFC_REFRESH_TIMERf, value);
            }
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_pfc_ctrl, port, 0, reg_val64));
            break;

        case SOC_MAC_CONTROL_PFC_RX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
            soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, RX_PFC_ENf, value ? 1 : 0);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_pfc_ctrl, port, 0, reg_val64));
            break;

        case SOC_MAC_CONTROL_PFC_TX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
            soc_reg64_field32_set(unit, xmac_pfc_ctrl, &reg_val64, TX_PFC_ENf, value ? 1 : 0);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_pfc_ctrl, port, 0, reg_val64));
            break;

        case SOC_MAC_CONTROL_LLFC_RX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_llfc_ctrl, port, 0, &reg_val64));
            soc_reg64_field32_set(unit, xmac_llfc_ctrl, &reg_val64, RX_LLFC_ENf, value ? 1 : 0);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_llfc_ctrl, port, 0, reg_val64));
            break;

        case SOC_MAC_CONTROL_LLFC_TX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_llfc_ctrl, port, 0, &reg_val64));
            soc_reg64_field32_set(unit, xmac_llfc_ctrl, &reg_val64, TX_LLFC_ENf, value ? 1 : 0);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_llfc_ctrl, port, 0, reg_val64));
            break;

        case SOC_MAC_CONTROL_EEE_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_eee_ctrl, port, 0 , &rval));
            soc_reg_field_set(unit, xmac_eee_ctrl, &rval, EEE_ENf, value ? 1 : 0);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_eee_ctrl, port, 0 , rval));
            break;

        case SOC_MAC_CONTROL_EEE_TX_IDLE_TIME:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_eee_timers, port, 0 , &reg_val64));
            soc_reg64_field32_set(unit, xmac_eee_timers, &reg_val64, EEE_DELAY_ENTRY_TIMERf, value);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_eee_timers, port, 0 , reg_val64));
            break;

        case SOC_MAC_CONTROL_EEE_TX_WAKE_TIME:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_eee_timers, port, 0 , &reg_val64));
            soc_reg64_field32_set(unit, xmac_eee_timers, &reg_val64, EEE_WAKE_TIMERf, value);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, xmac_eee_timers, port, 0 , reg_val64));
            break;

        case SOC_MAC_CONTROL_EEE_LINK_ACTIVE_DURATION:
            if (value >= (1 << soc_reg_field_length(unit, xmac_eee_1_sec_link_status_timer, ONE_SECOND_TIMERf))) {
                return BCM_E_PARAM;
            }

            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_eee_1_sec_link_status_timer, port, 0 , &rval));
            soc_reg_field_set(unit, xmac_eee_1_sec_link_status_timer, &rval, ONE_SECOND_TIMERf, value);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, xmac_eee_1_sec_link_status_timer, port, 0 , rval));
            break;

        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Unsupported control %d"), type));
    }


exit:
    SOCDNX_FUNC_RETURN
}

/*
 * Function:
 *      soc_xmac_drv_control_get
 * Purpose:
 *      To get current MAC control setting.
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      type - MAC control property to set.
 *      int  - setting for MAC control.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_drv_control_get(int unit, soc_port_t port, soc_mac_control_t type,
                  int *value)
{
    soc_reg_t xmac_rx_lss_status, xmac_pfc_ctrl, xmac_eee_ctrl, xmac_eee_timers, xmac_eee_1_sec_link_status_timer, xmac_llfc_ctrl,
              xmac_rx_lss_ctrl;
    uint32 rval;
    uint64 reg_val64;
    SOCDNX_INIT_FUNC_DEFS

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        xmac_rx_lss_status = CLP_XMAC_RX_LSS_STATUSr;
        xmac_pfc_ctrl = CLP_XMAC_PFC_CTRLr;
        xmac_llfc_ctrl = CLP_XMAC_LLFC_CTRLr;
        xmac_eee_ctrl = CLP_XMAC_EEE_CTRLr;
        xmac_eee_timers = CLP_XMAC_EEE_TIMERSr;
        xmac_eee_1_sec_link_status_timer = CLP_XMAC_EEE_1_SEC_LINK_STATUS_TIMERr;
        xmac_rx_lss_ctrl = CLP_XMAC_RX_LSS_CTRLr;
    } else {
        xmac_rx_lss_status = XLP_XMAC_RX_LSS_STATUSr;
        xmac_pfc_ctrl = XLP_XMAC_PFC_CTRLr;
        xmac_llfc_ctrl = XLP_XMAC_LLFC_CTRLr;
        xmac_eee_ctrl = XLP_XMAC_EEE_CTRLr;
        xmac_eee_timers = XLP_XMAC_EEE_TIMERSr;
        xmac_eee_1_sec_link_status_timer = XLP_XMAC_EEE_1_SEC_LINK_STATUS_TIMERr;
        xmac_rx_lss_ctrl = XLP_XMAC_RX_LSS_CTRLr;
    }

    switch (type) {

        case SOC_MAC_CONTROL_FAULT_LOCAL_STATUS:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_status, port, 0 , &rval));
            *value = soc_reg_field_get(unit, xmac_rx_lss_status, rval, LOCAL_FAULT_STATUSf);
            break;

        case SOC_MAC_CONTROL_FAULT_REMOTE_STATUS:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_status, port, 0 , &rval));
            *value = soc_reg_field_get(unit, xmac_rx_lss_status, rval, REMOTE_FAULT_STATUSf);
            break;

        case SOC_MAC_CONTROL_FAULT_LOCAL_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_ctrl, port, 0 , &rval));
            *value = soc_reg_field_get(unit, xmac_rx_lss_ctrl, rval, LOCAL_FAULT_DISABLEf);
            *value = !(*value);
            break;

        case SOC_MAC_CONTROL_FAULT_REMOTE_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_rx_lss_ctrl, port, 0 , &rval));
            *value = soc_reg_field_get(unit, xmac_rx_lss_ctrl, rval, REMOTE_FAULT_DISABLEf);
            *value = !(*value);
            break;

         case SOC_MAC_CONTROL_PFC_REFRESH_TIME:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
            *value = soc_reg64_field32_get(unit, xmac_pfc_ctrl, reg_val64, PFC_REFRESH_ENf);
            if(*value) { /* if PFC refresh is enbaled - read the actual value */
                *value = soc_reg64_field32_get(unit, xmac_pfc_ctrl, reg_val64, PFC_REFRESH_TIMERf);
            }
            break;

        case SOC_MAC_CONTROL_PFC_RX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
            *value = soc_reg64_field32_get(unit, xmac_pfc_ctrl, reg_val64, RX_PFC_ENf);
            break;

        case SOC_MAC_CONTROL_PFC_TX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_pfc_ctrl, port, 0, &reg_val64));
            *value = soc_reg64_field32_get(unit, xmac_pfc_ctrl, reg_val64, TX_PFC_ENf);
            break;

        case SOC_MAC_CONTROL_LLFC_RX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_llfc_ctrl, port, 0, &reg_val64));
            *value = soc_reg64_field32_get(unit, xmac_llfc_ctrl, reg_val64, RX_LLFC_ENf);
            break;

        case SOC_MAC_CONTROL_LLFC_TX_ENABLE:
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_llfc_ctrl, port, 0, &reg_val64));
            *value = soc_reg64_field32_get(unit, xmac_llfc_ctrl, reg_val64, TX_LLFC_ENf);
            break;

     case SOC_MAC_CONTROL_EEE_ENABLE:
         SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_eee_ctrl, port, 0 , &rval));
         *value = soc_reg_field_get(unit, xmac_eee_ctrl, rval, EEE_ENf);
         break;

     case SOC_MAC_CONTROL_EEE_TX_IDLE_TIME:
         SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_eee_timers, port, 0 , &reg_val64));
         *value = soc_reg64_field32_get(unit, xmac_eee_timers, reg_val64, EEE_DELAY_ENTRY_TIMERf);
         break;

     case SOC_MAC_CONTROL_EEE_TX_WAKE_TIME:
         SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, xmac_eee_timers, port, 0 , &reg_val64));
         *value = soc_reg64_field32_get(unit, xmac_eee_timers, reg_val64, EEE_WAKE_TIMERf);
         break;

    case SOC_MAC_CONTROL_EEE_LINK_ACTIVE_DURATION:
         SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, xmac_eee_1_sec_link_status_timer, port, 0 , &rval));
         *value = soc_reg_field_get(unit, xmac_eee_1_sec_link_status_timer, rval, ONE_SECOND_TIMERf);
         break;
         
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Unsupported control %d"), type));
    }


exit:
    SOCDNX_FUNC_RETURN
}


/*
 * Function:
 *      mac_x_ability_local_get
 * Purpose:
 *      Return the abilities of XMAC
 * Parameters:
 *      unit - XGS unit #.
 *      port - XGS port # on unit.
 *      mode - (OUT) Supported operating modes as a mask of abilities.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int
soc_xmac_ability_local_get(int unit, soc_port_t port,
                          soc_port_ability_t *ability)
{
    int port_speed_max;
    SOCDNX_INIT_FUNC_DEFS

    SOCDNX_NULL_CHECK(ability);

    ability->speed_half_duplex  = SOC_PA_ABILITY_NONE;
    ability->pause     = SOC_PA_PAUSE | SOC_PA_PAUSE_ASYMM;
    ability->interface = SOC_PA_INTF_MII | SOC_PA_INTF_XGMII;
    ability->medium    = SOC_PA_MEDIUM_COPPER | SOC_PA_MEDIUM_FIBER;
    ability->loopback  = SOC_PA_LB_MAC | SOC_PA_LB_PHY;
    ability->flags     = SOC_PA_ABILITY_NONE;
    ability->encap = SOC_PA_ENCAP_IEEE | SOC_PA_ENCAP_HIGIG |
        SOC_PA_ENCAP_HIGIG2;

    port_speed_max = SOC_INFO(unit).port_speed_max[port];
    if((port_speed_max >= 41500) && IS_HG_PORT(unit , port)){
        ability->speed_full_duplex |= SOC_PA_SPEED_42GB;
    }
    if(port_speed_max >= 39500){
        ability->speed_full_duplex |= SOC_PA_SPEED_48GB; /* 48G  is not officialy supported*/
        ability->speed_full_duplex |= SOC_PA_SPEED_40GB;
    }
    if(port_speed_max >= 22500){
        ability->speed_full_duplex |= SOC_PA_SPEED_23GB;
    }
    if((port_speed_max >= 20500) && IS_HG_PORT(unit , port)){
        ability->speed_full_duplex |= SOC_PA_SPEED_21GB;
    }
    if(port_speed_max >= 19500){
        ability->speed_full_duplex |= SOC_PA_SPEED_20GB;
    }
    if(port_speed_max >= 12500){
        ability->speed_full_duplex |= SOC_PA_SPEED_12P5GB;
    }
    if(port_speed_max >= 11500){
        ability->speed_full_duplex |= SOC_PA_SPEED_12GB;
    }
    if(port_speed_max >= 9500){
        ability->speed_full_duplex |= SOC_PA_SPEED_10GB;
    }
    if(port_speed_max >= 2500){
        ability->speed_full_duplex |= SOC_PA_SPEED_2500MB;
    }
    if(port_speed_max >= 1000){
        ability->speed_full_duplex |= SOC_PA_SPEED_1000MB;
    }
    if(port_speed_max >= 100){
        ability->speed_full_duplex |= SOC_PA_SPEED_100MB;
    }
    if(port_speed_max >= 10){
        ability->speed_full_duplex |= SOC_PA_SPEED_10MB;
    }
exit:
    SOCDNX_FUNC_RETURN
}

STATIC int
soc_xmac_pause_addr_set(int unit, soc_port_t port, sal_mac_addr_t m)
{
    soc_reg_t tx_sa_reg, rx_sa_reg;
    uint64 r, tmp;
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        tx_sa_reg = CLP_XMAC_TX_MAC_SAr;
        rx_sa_reg = CLP_XMAC_RX_MAC_SAr;
    } else {
        tx_sa_reg = XLP_XMAC_TX_MAC_SAr;
        rx_sa_reg = XLP_XMAC_RX_MAC_SAr;
    }

    COMPILER_64_ZERO(r);
    for (i = 0; i< 6; i++) {
        COMPILER_64_SET(tmp, 0, m[i]);
        COMPILER_64_SHL(r, 8);
        COMPILER_64_OR(r, tmp);
    }

    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit , tx_sa_reg, port, 0, r));
    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit , rx_sa_reg, port, 0, r));

exit:
    SOCDNX_FUNC_RETURN
}

STATIC int
soc_xmac_pause_addr_get(int unit, soc_port_t port, sal_mac_addr_t m)
{
    soc_reg_t rx_sa_reg;
    uint64 reg;
    uint32 msw;
    uint32 lsw;
    SOCDNX_INIT_FUNC_DEFS;

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        rx_sa_reg = CLP_XMAC_RX_MAC_SAr;
    } else {
        rx_sa_reg = XLP_XMAC_RX_MAC_SAr;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit , rx_sa_reg, port, 0, &reg));

    COMPILER_64_TO_32_HI(msw, reg);
    COMPILER_64_TO_32_LO(lsw, reg);

    m[0] = (uint8) ( ( msw & 0x0000ff00 ) >> 8 );
    m[1] = (uint8) ( msw & 0x000000ff );

    m[2] = (uint8)  ( ( lsw & 0xff000000) >> 24 );
    m[3] = (uint8)  ( ( lsw & 0x00ff0000) >> 16 );
    m[4] = (uint8)  ( ( lsw & 0x0000ff00) >> 8 );
    m[5] = (uint8)  ( lsw & 0x000000ff );

exit:
    SOCDNX_FUNC_RETURN
}

STATIC int
soc_xmac_pause_set(int unit, soc_port_t port, int pause_tx, int pause_rx)
{
    uint64 reg_val64;
    soc_reg_t pause_ctrl;
    SOCDNX_INIT_FUNC_DEFS;

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        pause_ctrl = CLP_XMAC_PAUSE_CTRLr;
    } else {
        pause_ctrl = XLP_XMAC_PAUSE_CTRLr;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit , pause_ctrl, port, 0, &reg_val64));
    soc_reg64_field32_set(unit, pause_ctrl, &reg_val64, TX_PAUSE_ENf, pause_tx);
    soc_reg64_field32_set(unit, pause_ctrl, &reg_val64, RX_PAUSE_ENf, pause_rx);
    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit , pause_ctrl, port, 0, reg_val64));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_xmac_pause_get(int unit, soc_port_t port, int *pause_tx, int *pause_rx)
{
    uint64 reg_val64;
    soc_reg_t pause_ctrl;
    SOCDNX_INIT_FUNC_DEFS;

    if(SOC_BLK_CLP == SOC_PORT_TYPE(unit,port)) {
        pause_ctrl = CLP_XMAC_PAUSE_CTRLr;
    } else {
        pause_ctrl = XLP_XMAC_PAUSE_CTRLr;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit , pause_ctrl, port, 0, &reg_val64));
    *pause_tx = soc_reg64_field32_get(unit, pause_ctrl, reg_val64, TX_PAUSE_ENf);
    *pause_rx = soc_reg64_field32_get(unit, pause_ctrl, reg_val64, RX_PAUSE_ENf);

exit:
    SOCDNX_FUNC_RETURN;
}

CONST mac_driver_t soc_xmac_driver = {
    "DPP XMAC Driver",              /* drv_name */
    soc_xmac_drv_init,              /* md_init  */
    soc_xmac_drv_enable_set,        /* md_enable_set */
    soc_xmac_drv_enable_get,        /* md_enable_get */
    NULL,                           /* md_duplex_set */
    NULL,                           /* md_duplex_get */
    soc_xmac_drv_speed_set,         /* md_speed_set */
    NULL,                           /* md_speed_get */
    soc_xmac_pause_set,             /* md_pause_set */
    soc_xmac_pause_get,             /* md_pause_get */
    soc_xmac_pause_addr_set,
    soc_xmac_pause_addr_get,
    soc_xmac_drv_loopback_set,      /* md_lb_set */
    soc_xmac_drv_loopback_get,      /* md_lb_get */
    NULL,                           /* md_interface_set */
    NULL,                           /* md_interface_get */
    NULL,                           /* md_ability_get - Deprecated */
    soc_xmac_drv_frame_max_set,     /* md_frame_max_set */
    soc_xmac_drv_frame_max_get,     /* md_frame_max_get */
    NULL,                           /* md_ifg_set */
    NULL,                           /* md_ifg_get */
    NULL,                           /* md_encap_set */
    NULL,                           /* md_encap_get */
    soc_xmac_drv_control_set,       /* md_control_set */
    soc_xmac_drv_control_get,       /* md_control_get */
    soc_xmac_ability_local_get      /* md_ability_local_get */
 };

#undef _ERR_MSG_MODULE_NAME


