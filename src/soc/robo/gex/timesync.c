/*
 * $Id: timesync.c,v 1.1.2.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Field Processor related CLI commands
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/drv_if.h>
#include <soc/drv.h>
#include <soc/debug.h>

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)

void 
_drv_gex_timesync_encode_egress_message_mode(
     soc_port_phy_timesync_event_message_egress_mode_t mode, uint32 *value)
{
    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE:
        *value = 0x0;
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD:
        *value = 0x1;
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN:
        *value = 0x2;
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP:
        *value = 0x3;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_encode_ingress_message_mode(
     soc_port_phy_timesync_event_message_ingress_mode_t mode, uint32 *value)
{
    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE:
        *value = 0x0;
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD:
        *value = 0x1;
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP:
        *value = 0x2;
        break;
    case SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME:
        *value = 0x3;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_decode_egress_message_mode(
     uint32 value, soc_port_phy_timesync_event_message_egress_mode_t *mode)
{
    switch (value) {
    case 0x0:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_NONE;
        break;
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_UPDATE_CORRECTIONFIELD;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_REPLACE_CORRECTIONFIELD_ORIGIN;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_EGRESS_MODE_CAPTURE_TIMESTAMP;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_decode_ingress_message_mode( 
     uint32 value, soc_port_phy_timesync_event_message_ingress_mode_t *mode)
{
    switch (value) {
    case 0x0:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_NONE;
        break;
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_UPDATE_CORRECTIONFIELD;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_TIMESTAMP;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_EVENT_MESSAGE_INGRESS_MODE_INSERT_DELAYTIME;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_encode_gmode(
     soc_port_phy_timesync_global_mode_t mode, uint32 *value)
{
    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_MODE_FREE:
        *value = 0x1;
        break;
    case SOC_PORT_PHY_TIMESYNC_MODE_SYNCIN:
        *value = 0x2;
        break;
    case SOC_PORT_PHY_TIMESYNC_MODE_CPU:
        *value = 0x3;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_decode_gmode(
     uint32 value, soc_port_phy_timesync_global_mode_t *mode)
{
    switch (value & 0x3) {
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_MODE_FREE;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_MODE_SYNCIN;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_MODE_CPU;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_encode_framesync_mode(
     soc_port_phy_timesync_framesync_mode_t mode, uint32 *value)
{
    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN0:
        *value = 1U << 0;
        break;
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN1:
        *value = 1U << 1;
        break;
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCOUT:
        *value = 1U << 2;
        break;
    case SOC_PORT_PHY_TIMESYNC_FRAMESYNC_CPU:
        *value = 1U << 3;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_decode_framesync_mode(
     uint32 value, soc_port_phy_timesync_framesync_mode_t *mode)
{
    switch (value & 0xf) {
    case 1U << 0:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN0;
        break;
    case 1U << 1:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCIN1;
        break;
    case 1U << 2:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_SYNCOUT;
        break;
    case 1U << 3:
        *mode = SOC_PORT_PHY_TIMESYNC_FRAMESYNC_CPU;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_encode_syncout_mode(
     soc_port_phy_timesync_syncout_mode_t mode, uint32 *value)
{
    switch (mode) {
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_DISABLE:
        *value = 0x0;
        break;
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_ONE_TIME:
        *value = 0x1;
        break;
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN:
        *value = 0x2;
        break;
    case SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN_WITH_SYNC:
        *value = 0x3;
        break;
    default:
        break;
    }
}

void 
_drv_gex_timesync_decode_syncout_mode(
     uint32 value, soc_port_phy_timesync_syncout_mode_t *mode)
{
    switch (value & 0x3) {
    case 0x0:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_DISABLE;
        break;
    case 0x1:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_ONE_TIME;
        break;
    case 0x2:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN;
        break;
    case 0x3:
        *mode = SOC_PORT_PHY_TIMESYNC_SYNCOUT_PULSE_TRAIN_WITH_SYNC;
        break;
    }
}

int 
drv_gex_timesync_config_set(
     int unit, uint32 port, soc_port_phy_timesync_config_t *conf)
{
    uint32 reg_value = 0, reg_value1 = 0, temp = 0, temp1 = 0;
    uint32 gmode = 0, framesync_mode = 0, syncout_mode = 0;
    int index = 0;
    uint32 additional_soc_port_num = 0;

    index = (int)port;
    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &additional_soc_port_num));

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FLAGS) {
        SOC_IF_ERROR_RETURN(REG_READ_PORT_ENABLEr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_PORT_ENABLEr_field_get
                           (unit, &reg_value, TX_PORT_1588_ENf, &temp));
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_ENABLE) {
            temp |= (1U << port);
        } else {
            temp &= ~(1U << port);
        }
        SOC_IF_ERROR_RETURN(soc_PORT_ENABLEr_field_set
                           (unit, &reg_value, TX_PORT_1588_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_PORT_ENABLEr_field_set
                           (unit, &reg_value, RX_PORT_1588_ENf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_PORT_ENABLEr(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_TX_TS_CAPr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(REG_READ_RX_TS_CAPr(unit, &reg_value1));
        SOC_IF_ERROR_RETURN(soc_TX_TS_CAPr_field_get
                               (unit, &reg_value, TX_TS_CAPf, &temp));
        SOC_IF_ERROR_RETURN(soc_RX_TS_CAPr_field_get
                               (unit, &reg_value1, RX_TS_CAPf, &temp1));        
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE) {
            temp |= (1U << port);
            temp1 |= (1U << port);
        } else {
            temp &= ~(1U << port);
            temp1 &= ~(1U << port);
        }
        SOC_IF_ERROR_RETURN(soc_TX_TS_CAPr_field_set
                               (unit, &reg_value, TX_TS_CAPf, &temp));
        SOC_IF_ERROR_RETURN(soc_RX_TS_CAPr_field_set
                               (unit, &reg_value1, RX_TS_CAPf, &temp1));
        SOC_IF_ERROR_RETURN(REG_WRITE_TX_TS_CAPr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(REG_WRITE_RX_TS_CAPr(unit, &reg_value1));
        
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_6r(unit, &reg_value));
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE) {
            temp = 0x1;
        } else {
            temp &= 0x0;
        }
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                               (unit, &reg_value, TS_CAPTUREf, &temp));
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_ENABLE) {
            temp = 0x1;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                               (unit, &reg_value, NSE_INITf, &temp));
        }
        
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_6r(unit, &reg_value));
        
        SOC_IF_ERROR_RETURN(REG_READ_RX_TX_CTLr(unit, &reg_value));
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_RX_CRC_ENABLE) {
            temp = 0x1;
        } else {
            temp = 0x0;
        }
        SOC_IF_ERROR_RETURN(soc_RX_TX_CTLr_field_set
                               (unit, &reg_value, RX_CRC_ENf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_RX_TX_CTLr(unit, &reg_value));
        
        SOC_IF_ERROR_RETURN(REG_READ_RX_CTLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(REG_READ_TX_CTLr(unit, &reg_value1));
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_8021AS_ENABLE) {
            temp = 0x1;
        } else {
            temp = 0x0;
        }
        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_set
                               (unit, &reg_value, RX_AS_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_TX_CTLr_field_set
                               (unit, &reg_value1, TX_AS_ENf, &temp));
        
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_L2_ENABLE) {
            temp = 0x1;
        } else {
            temp = 0x0;
        }
        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_set
                               (unit, &reg_value, RX_L2_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_TX_CTLr_field_set
                               (unit, &reg_value1, TX_L2_ENf, &temp));
        
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_IP4_ENABLE) {
            temp = 0x1;
        } else {
            temp = 0x0;
        }
        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_set
                               (unit, &reg_value, RX_IPV4_UDP_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_TX_CTLr_field_set
                               (unit, &reg_value1, TX_IPV4_UDP_ENf, &temp));
        
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_IP6_ENABLE) {
            temp = 0x1;
        } else {
            temp = 0x0;
        }
        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_set
                               (unit, &reg_value, RX_IPV6_UDP_ENf, &temp));
        SOC_IF_ERROR_RETURN(soc_TX_CTLr_field_set
                               (unit, &reg_value1, TX_IPV6_UDP_ENf, &temp));
        
        SOC_IF_ERROR_RETURN(REG_WRITE_RX_CTLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(REG_WRITE_TX_CTLr(unit, &reg_value1));
        
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_2_Nr(unit, 2, &reg_value));
        if (conf->flags & SOC_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT) {
            reg_value &= ~(1U << 14);
        } else {
            reg_value |= (1U << 14);
        }
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_2_Nr(unit, 2, &reg_value));
    }
    
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ITPID) {
        /* VLAN TAG register */
        SOC_IF_ERROR_RETURN(REG_READ_VLAN_ITPIDr(unit, &reg_value));
        temp = conf->itpid;
        SOC_IF_ERROR_RETURN(soc_VLAN_ITPIDr_field_set
                               (unit, &reg_value, ITPIDf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_ITPIDr(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID) {
        /* OUTER VLAN TAG register */
        SOC_IF_ERROR_RETURN(REG_READ_VLAN_OTPIDr(unit, &reg_value));
        temp = conf->otpid;
        SOC_IF_ERROR_RETURN(soc_VLAN_OTPIDr_field_set
                               (unit, &reg_value, OTPIDf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_VLAN_OTPIDr(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID2) {
        /* INNER VLAN TAG register */
        SOC_IF_ERROR_RETURN(REG_READ_OTHER_OTPIDr(unit, &reg_value));
        temp = conf->otpid2;
        SOC_IF_ERROR_RETURN(soc_OTHER_OTPIDr_field_set
                               (unit, &reg_value, OTPID_2f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_OTHER_OTPIDr(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TS_DIVIDER) {
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_4r(unit, &reg_value));
        temp = conf->ts_divider & 0xfff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_4r_field_set
                               (unit, &reg_value, NSE_REG_TS_DIVIDERf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_4r(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_LINK_DELAY) {
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_LINK_DELAY_MSBr(unit, &reg_value1));
        } else {
            return SOC_E_PARAM;
        }

        temp = conf->rx_link_delay & 0xffff;
        SOC_IF_ERROR_RETURN(soc_RX_PORT_0_LINK_DELAY_LSBr_field_set
                               (unit, &reg_value, RX_LINK_DELAY_LSBf, &temp));
        temp = (conf->rx_link_delay >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_RX_PORT_0_LINK_DELAY_MSBr_field_set
                               (unit, &reg_value1, RX_LINK_DELAY_MSBf, &temp));
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_0_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_0_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_1_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_1_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_2_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_2_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_3_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_3_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_4_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_4_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_5_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_5_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_7_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_7_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_8_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_8_LINK_DELAY_MSBr(unit, &reg_value1));
        }
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ORIGINAL_TIMECODE) {
        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 0, &reg_value));
        temp = conf->original_timecode.nanoseconds & 0xffff;
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_set
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TIME_CODE_Nr(unit, 0, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 1, &reg_value));
        temp = (conf->original_timecode.nanoseconds >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_set
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TIME_CODE_Nr(unit, 1, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 2, &reg_value));
        temp = COMPILER_64_LO(conf->original_timecode.seconds) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_set
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TIME_CODE_Nr(unit, 2, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 3, &reg_value));
        temp = (COMPILER_64_LO(conf->original_timecode.seconds) >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_set
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TIME_CODE_Nr(unit, 3, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 4, &reg_value));
        temp = COMPILER_64_HI(conf->original_timecode.seconds) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_set
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_TIME_CODE_Nr(unit, 4, &reg_value));
    }

    SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_6r(unit, &reg_value1));    
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_GMODE) {
        _drv_gex_timesync_encode_gmode(conf->gmode,&gmode);
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                               (unit, &reg_value1, GMODEf, &gmode));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FRAMESYNC_MODE) {
        _drv_gex_timesync_encode_framesync_mode(conf->framesync.mode, &framesync_mode);
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                               (unit, &reg_value1, FRAMESYN_MODEf, &framesync_mode));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_7_0r(unit, &reg_value));
        temp = conf->framesync.length_threshold;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_7_0r_field_set
                               (unit, &reg_value, LENGTH_THRESHOLDf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_7_0r(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_7_1r(unit, &reg_value));
        temp = conf->framesync.event_offset;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_7_1r_field_set
                               (unit, &reg_value, EVENT_OFFSETf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_7_1r(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_SYNCOUT_MODE) {
        _drv_gex_timesync_encode_syncout_mode(conf->syncout.mode, &syncout_mode);
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                               (unit, &reg_value1, SYNOUT_MODEf, &syncout_mode));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_3_0r(unit, &reg_value));
        temp = conf->syncout.interval & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_0r_field_set
                               (unit, &reg_value, INTERVAL_LENGTH_0f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_3_0r(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_3_1r(unit, &reg_value));
        temp = (conf->syncout.interval >> 16) & 0x3fff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_1r_field_set
                               (unit, &reg_value, INTERVAL_LENGTH_1f, &temp));
        temp = conf->syncout.pulse_1_length & 0x3;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_1r_field_set
                               (unit, &reg_value, PULSE_TRAIN_LENGTH_0f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_3_1r(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_3_2r(unit, &reg_value));
        temp = (conf->syncout.pulse_1_length >> 2) & 0x7f;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_2r_field_set
                               (unit, &reg_value, PULSE_TRAIN_LENGTH_1f, &temp));
        temp = conf->syncout.pulse_2_length & 0x1ff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_2r_field_set
                               (unit, &reg_value, FRMSYNC_PULSE_LENGTHf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_3_2r(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_5_0r(unit, &reg_value));
        temp = COMPILER_64_LO(conf->syncout.syncout_ts) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_5_0r_field_set
                               (unit, &reg_value, SYNOUT_TS_REG_0f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_5_0r(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_5_1r(unit, &reg_value));
        temp = (COMPILER_64_LO(conf->syncout.syncout_ts) >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_5_1r_field_set
                               (unit, &reg_value, SYNOUT_TS_REG_1f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_5_1r(unit, &reg_value));

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_5_2r(unit, &reg_value));
        temp = COMPILER_64_HI(conf->syncout.syncout_ts) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_5_2r_field_set
                               (unit, &reg_value, SYNOUT_TS_REG_2f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_5_2r(unit, &reg_value));
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_6r(unit, &reg_value1));
    
    /* tx_timestamp_offset */
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_TIMESTAMP_OFFSET) {
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
        } else {
            return SOC_E_PARAM;
        }

        temp = conf->tx_timestamp_offset & 0xffff;
        SOC_IF_ERROR_RETURN(soc_TX_PORT_0_TS_OFFSET_LSBr_field_set
                               (unit, &reg_value, TS_OFFSET_TX_LSBf, &temp));
        temp = (conf->tx_timestamp_offset >> 16) & 0xf;
        SOC_IF_ERROR_RETURN(soc_TX_PORT_0_TS_OFFSET_MSBr_field_set
                               (unit, &reg_value1, TS_OFFSET_TX_MSBf, &temp));
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
        }
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_TIMESTAMP_OFFSET) {
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
        } else {
            return SOC_E_PARAM;
        }

        temp = conf->rx_timestamp_offset & 0xffff;
        SOC_IF_ERROR_RETURN(soc_RX_PORT_0_TS_OFFSET_LSBr_field_set
                               (unit, &reg_value, TS_OFFSET_RX_LSBf, &temp));
        temp = (conf->rx_timestamp_offset >> 16) & 0xf;
        SOC_IF_ERROR_RETURN(soc_RX_PORT_0_TS_OFFSET_MSBr_field_set
                               (unit, &reg_value1, TS_OFFSET_RX_MSBf, &temp));
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
        }
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_TX_MODE_PORT_IMPr(unit, &reg_value));
    } else if (port == additional_soc_port_num) {
        SOC_IF_ERROR_RETURN(REG_READ_TX_MODE_PORT_P7r(unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_TX_MODE_PORTr(unit, index, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_SYNC_MODE) {
        _drv_gex_timesync_encode_egress_message_mode(conf->tx_sync_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_set
                               (unit, &reg_value, TX_MODE1_M0f, &temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_DELAY_REQUEST_MODE) {
        _drv_gex_timesync_encode_egress_message_mode(conf->tx_delay_request_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_set
                               (unit, &reg_value, TX_MODE1_M1f, &temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_REQUEST_MODE) {
        _drv_gex_timesync_encode_egress_message_mode(conf->tx_pdelay_request_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_set
                               (unit, &reg_value, TX_MODE1_M2f, &temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_RESPONSE_MODE) {
        _drv_gex_timesync_encode_egress_message_mode(conf->tx_pdelay_response_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_set
                               (unit, &reg_value, TX_MODE1_M3f, &temp));
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_WRITE_TX_MODE_PORT_IMPr(unit, &reg_value));
    } else if (port == additional_soc_port_num) {
        SOC_IF_ERROR_RETURN(REG_WRITE_TX_MODE_PORT_P7r(unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_WRITE_TX_MODE_PORTr(unit, index, &reg_value));
    }
                                           
    if (IS_CPU_PORT(unit, port)) {
       SOC_IF_ERROR_RETURN(REG_READ_RX_MODE_PORT_IMPr(unit, &reg_value));
    } else if (port == additional_soc_port_num) {
       SOC_IF_ERROR_RETURN(REG_READ_RX_MODE_PORT_P7r(unit, &reg_value));
    } else {
       SOC_IF_ERROR_RETURN(REG_READ_RX_MODE_PORTr(unit, index, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_SYNC_MODE) {
        _drv_gex_timesync_encode_ingress_message_mode(conf->rx_sync_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_set
                               (unit, &reg_value, RX_MODE1_M0f, &temp));
    }
    
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_DELAY_REQUEST_MODE) {
        _drv_gex_timesync_encode_ingress_message_mode(conf->rx_delay_request_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_set
                               (unit, &reg_value, RX_MODE1_M1f, &temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_REQUEST_MODE) {
        _drv_gex_timesync_encode_ingress_message_mode(conf->rx_pdelay_request_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_set
                               (unit, &reg_value, RX_MODE1_M2f, &temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_RESPONSE_MODE) {
        _drv_gex_timesync_encode_ingress_message_mode(conf->rx_pdelay_response_mode, &temp);
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_set
                               (unit, &reg_value, RX_MODE1_M3f, &temp));
    }

    if (IS_CPU_PORT(unit, port)) {
       SOC_IF_ERROR_RETURN(REG_WRITE_RX_MODE_PORT_IMPr(unit, &reg_value));
    } else if (port == additional_soc_port_num) {
       SOC_IF_ERROR_RETURN(REG_WRITE_RX_MODE_PORT_P7r(unit, &reg_value));
    } else {
       SOC_IF_ERROR_RETURN(REG_WRITE_RX_MODE_PORTr(unit, index, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE) {
        /* Initial ref phase [15:0] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_2_Nr(unit, 0, &reg_value));
        temp = COMPILER_64_LO(conf->phy_1588_dpll_ref_phase) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_2_Nr_field_set
                               (unit, &reg_value, REF_PHASE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_2_Nr(unit, 0, &reg_value));

        /* Initial ref phase [31:16]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_2_Nr(unit, 1, &reg_value));
        temp = (COMPILER_64_LO(conf->phy_1588_dpll_ref_phase) >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_2_Nr_field_set
                               (unit, &reg_value, REF_PHASE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_2_Nr(unit, 1, &reg_value));

        /*  Initial ref phase [47:32] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_2_Nr(unit, 2, &reg_value));
        temp = COMPILER_64_HI(conf->phy_1588_dpll_ref_phase) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_2_Nr_field_set
                               (unit, &reg_value, REF_PHASE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_2_Nr(unit, 2, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE_DELTA) {
        /* Ref phase delta [15:0] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_3_Nr(unit, 0, &reg_value));
        temp = conf->phy_1588_dpll_ref_phase_delta & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_3_Nr_field_set
                               (unit, &reg_value, REF_PHASE_DELTA_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_3_Nr(unit, 0, &reg_value));

        /* Ref phase delta [31:16]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_3_Nr(unit, 1, &reg_value));
        temp = (conf->phy_1588_dpll_ref_phase_delta >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_3_Nr_field_set
                               (unit, &reg_value, REF_PHASE_DELTA_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_3_Nr(unit, 1, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K1) {
        /* DPLL K1 */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_4r(unit, &reg_value));
        temp = conf->phy_1588_dpll_k1 & 0xff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_4r_field_set
                               (unit, &reg_value, DPLL_K1f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_4r(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K2) {
        /* DPLL K2 */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_5r(unit, &reg_value));
        temp = conf->phy_1588_dpll_k2 & 0xff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_5r_field_set
                               (unit, &reg_value, DPLL_K2f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_5r(unit, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K3) {
        /* DPLL K3 */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_6r(unit, &reg_value));
        temp = conf->phy_1588_dpll_k3 & 0xff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_6r_field_set
                               (unit, &reg_value, DPLL_K3f, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_6r(unit, &reg_value));
    }


    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_LOOP_FILTER) {
        /* Initial loop filter[15:0] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 0, &reg_value));
        temp = COMPILER_64_LO(conf->phy_1588_dpll_loop_filter) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_set
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_7_Nr(unit, 0, &reg_value));

        /* Initial loop filter[31:16]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 1, &reg_value));
        temp = (COMPILER_64_LO(conf->phy_1588_dpll_loop_filter) >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_set
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_7_Nr(unit, 1, &reg_value));

        /*  Initial loop filter[47:32] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 2, &reg_value));
        temp = COMPILER_64_HI(conf->phy_1588_dpll_loop_filter) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_set
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_7_Nr(unit, 2, &reg_value));

        /* Initial loop filter[63:48]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 3, &reg_value));
        temp = (COMPILER_64_HI(conf->phy_1588_dpll_loop_filter) >> 16) & 0xffff;
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_set
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_NSE_DPLL_7_Nr(unit, 3, &reg_value));
    }

    return SOC_E_NONE;
}

int 
drv_gex_timesync_config_get(
     int unit, uint32 port, soc_port_phy_timesync_config_t *conf)
{
    uint32 reg_value = 0, reg_value1 = 0, reg_value2 = 0, nse_nco_6_value = 0;
    uint32 temp = 0, temp1 = 0, temp2 = 0, temp3;
    uint32 gmode = 0, framesync_mode = 0, syncout_mode = 0;
    int index = 0;
    uint32 additional_soc_port_num = 0;

    index = (int)port;
    SOC_IF_ERROR_RETURN((DRV_SERVICES(unit)->dev_prop_get)
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, 
                &additional_soc_port_num));
    conf->flags = 0;

    SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_6r(unit, &nse_nco_6_value));

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FLAGS) {
        SOC_IF_ERROR_RETURN(REG_READ_PORT_ENABLEr(unit, &reg_value));
        if (reg_value & (1U << port)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_ENABLE;
        }

        SOC_IF_ERROR_RETURN(REG_READ_TX_TS_CAPr(unit, &reg_value));
        if (reg_value & (1U << port)) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_CAPTURE_TS_ENABLE;
        }

        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_get
                                  (unit, &nse_nco_6_value, TS_CAPTUREf, &temp));
        if (temp) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_HEARTBEAT_TS_ENABLE;
        }

        SOC_IF_ERROR_RETURN(REG_READ_RX_TX_CTLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_RX_TX_CTLr_field_get
                               (unit, &reg_value, RX_CRC_ENf, &temp));
        if (temp) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_RX_CRC_ENABLE;
        }

        SOC_IF_ERROR_RETURN(REG_READ_RX_CTLr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_get
                               (unit, &reg_value, RX_AS_ENf, &temp));
        if (temp) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_8021AS_ENABLE;
        }

        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_get
                               (unit, &reg_value, RX_L2_ENf, &temp));
        if (temp) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_L2_ENABLE;
        }

        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_get
                               (unit, &reg_value, RX_IPV4_UDP_ENf, &temp));
        if (temp) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_IP4_ENABLE;
        }

        SOC_IF_ERROR_RETURN(soc_RX_CTLr_field_get
                               (unit, &reg_value, RX_IPV6_UDP_ENf, &temp));
        if (temp) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_IP6_ENABLE;
        }

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_2_Nr(unit, 2, &reg_value));
        if (!(reg_value & (1U << 14))) {
            conf->flags |= SOC_PORT_PHY_TIMESYNC_CLOCK_SRC_EXT;
        }
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_GMODE) {
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_get
                               (unit, &nse_nco_6_value, GMODEf, &temp));
        _drv_gex_timesync_decode_gmode(temp,&gmode);
        conf->gmode = gmode;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_FRAMESYNC_MODE) {
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_get
                               (unit, &nse_nco_6_value, FRAMESYN_MODEf, &temp));
        _drv_gex_timesync_decode_framesync_mode(temp, &framesync_mode);
        conf->framesync.mode = framesync_mode;

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_7_0r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_7_0r_field_get
                               (unit, &reg_value, LENGTH_THRESHOLDf, &temp));
        conf->framesync.length_threshold = temp;

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_7_1r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_7_1r_field_get
                               (unit, &reg_value, EVENT_OFFSETf, &temp));
        conf->framesync.event_offset = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_SYNCOUT_MODE) {
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_get
                               (unit, &nse_nco_6_value, SYNOUT_MODEf, &temp));
        _drv_gex_timesync_decode_syncout_mode(temp, &syncout_mode);
        conf->syncout.mode = syncout_mode;

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_3_0r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_3_1r(unit, &reg_value1));
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_3_2r(unit, &reg_value2));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_1r_field_get
                               (unit, &reg_value, PULSE_TRAIN_LENGTH_0f, &temp));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_2r_field_get
                               (unit, &reg_value1, PULSE_TRAIN_LENGTH_1f, &temp1));
        conf->syncout.pulse_1_length =  (temp1 << 2) | temp;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_2r_field_get
                               (unit, &reg_value2, FRMSYNC_PULSE_LENGTHf, &temp));
        conf->syncout.pulse_2_length = temp;
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_0r_field_get
                               (unit, &reg_value, INTERVAL_LENGTH_0f, &temp));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_3_1r_field_get
                               (unit, &reg_value1, INTERVAL_LENGTH_1f, &temp1));
        conf->syncout.interval =  (temp1 << 16) | temp;

        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_5_0r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_5_1r(unit, &reg_value1));
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_5_2r(unit, &reg_value2));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_5_0r_field_get
                               (unit, &reg_value, SYNOUT_TS_REG_0f, &temp));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_5_1r_field_get
                               (unit, &reg_value1, SYNOUT_TS_REG_1f, &temp1));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_5_2r_field_get
                               (unit, &reg_value2, SYNOUT_TS_REG_2f, &temp2));
        COMPILER_64_SET(conf->syncout.syncout_ts, temp2, ((temp1 << 16) | temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ITPID) {
        SOC_IF_ERROR_RETURN(REG_READ_VLAN_ITPIDr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_VLAN_ITPIDr_field_get
                               (unit, &reg_value, ITPIDf, &temp));
        conf->itpid = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID) {
        SOC_IF_ERROR_RETURN(REG_READ_VLAN_OTPIDr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_VLAN_OTPIDr_field_get
                               (unit, &reg_value, OTPIDf, &temp));
        conf->otpid = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_OTPID2) {
        SOC_IF_ERROR_RETURN(REG_READ_OTHER_OTPIDr(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_OTHER_OTPIDr_field_get
                               (unit, &reg_value, OTPID_2f, &temp));
        conf->otpid2 = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TS_DIVIDER) {
        SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_4r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_NCO_4r_field_get
                               (unit, &reg_value, NSE_REG_TS_DIVIDERf, &temp));
        conf->ts_divider = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_LINK_DELAY) {
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_LINK_DELAY_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_LINK_DELAY_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_LINK_DELAY_MSBr(unit, &reg_value1));
        } else {
            return SOC_E_PARAM;
        }

        SOC_IF_ERROR_RETURN(soc_RX_PORT_0_LINK_DELAY_LSBr_field_get
                               (unit, &reg_value, RX_LINK_DELAY_LSBf, &temp));
        SOC_IF_ERROR_RETURN(soc_RX_PORT_0_LINK_DELAY_MSBr_field_get
                               (unit, &reg_value1, RX_LINK_DELAY_MSBf, &temp1));
        conf->rx_link_delay = (temp1 << 16) | temp; 
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_ORIGINAL_TIMECODE) {
        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 0, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_get
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 1, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_get
                               (unit, &reg_value, TIME_CODE_Nf, &temp1));

        conf->original_timecode.nanoseconds = (temp1 << 16) | temp; 

        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 2, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_get
                               (unit, &reg_value, TIME_CODE_Nf, &temp));
        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 3, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_get
                               (unit, &reg_value, TIME_CODE_Nf, &temp1));
        SOC_IF_ERROR_RETURN(REG_READ_TIME_CODE_Nr(unit, 4, &reg_value));
        SOC_IF_ERROR_RETURN(soc_TIME_CODE_Nr_field_get
                               (unit, &reg_value, TIME_CODE_Nf, &temp2));
        COMPILER_64_SET(conf->original_timecode.seconds, temp2, ((temp1 << 16) | temp));
    }

    /* tx_timestamp_offset */
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_TIMESTAMP_OFFSET) {
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
        } else {
            return SOC_E_PARAM;
        }

        SOC_IF_ERROR_RETURN(soc_TX_PORT_4_TS_OFFSET_LSBr_field_get
                               (unit, &reg_value, TS_OFFSET_TX_LSBf, &temp));
        SOC_IF_ERROR_RETURN(soc_TX_PORT_4_TS_OFFSET_MSBr_field_get
                               (unit, &reg_value1, TS_OFFSET_TX_MSBf, &temp1));
        conf->tx_timestamp_offset = (temp1 << 16) | temp;
    }

    /* rx_timestamp_offset */
    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_TIMESTAMP_OFFSET) {
        if (port == 0) {
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 1){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 2){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 3){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 4){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 5){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 7){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
        } else if (port == 8){
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
        } else {
            return SOC_E_PARAM;
        }

        SOC_IF_ERROR_RETURN(soc_RX_PORT_4_TS_OFFSET_LSBr_field_get
                               (unit, &reg_value, TS_OFFSET_RX_LSBf, &temp));
        SOC_IF_ERROR_RETURN(soc_RX_PORT_4_TS_OFFSET_MSBr_field_get
                               (unit, &reg_value1, TS_OFFSET_RX_MSBf, &temp1));
        conf->rx_timestamp_offset = (temp1 << 16) | temp;
    }

    if (IS_CPU_PORT(unit, port)) {
        SOC_IF_ERROR_RETURN(REG_READ_TX_MODE_PORT_IMPr(unit, &reg_value));
    } else if (port == additional_soc_port_num) {
        SOC_IF_ERROR_RETURN(REG_READ_TX_MODE_PORT_P7r(unit, &reg_value));
    } else {
        SOC_IF_ERROR_RETURN(REG_READ_TX_MODE_PORTr(unit, index, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_SYNC_MODE) {
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_get
                               (unit, &reg_value, TX_MODE1_M0f, &temp));
        _drv_gex_timesync_decode_egress_message_mode(temp, &conf->tx_sync_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_DELAY_REQUEST_MODE) {
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_get
                               (unit, &reg_value, TX_MODE1_M1f, &temp));
        _drv_gex_timesync_decode_egress_message_mode(temp, &conf->tx_delay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_REQUEST_MODE) {
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_get
                               (unit, &reg_value, TX_MODE1_M2f, &temp));
        _drv_gex_timesync_decode_egress_message_mode(temp, &conf->tx_pdelay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_TX_PDELAY_RESPONSE_MODE) {
        SOC_IF_ERROR_RETURN(soc_TX_MODE_PORTr_field_get
                               (unit, &reg_value, TX_MODE1_M3f, &temp));
        _drv_gex_timesync_decode_egress_message_mode(temp, &conf->tx_pdelay_response_mode);
    }

    if (IS_CPU_PORT(unit, port)) {
       SOC_IF_ERROR_RETURN(REG_READ_RX_MODE_PORT_IMPr(unit, &reg_value));
    } else if (port == additional_soc_port_num) {
       SOC_IF_ERROR_RETURN(REG_READ_RX_MODE_PORT_P7r(unit, &reg_value));
    } else {
       SOC_IF_ERROR_RETURN(REG_READ_RX_MODE_PORTr(unit, index, &reg_value));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_SYNC_MODE) {
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_get
                               (unit, &reg_value, RX_MODE1_M0f, &temp));
        _drv_gex_timesync_decode_ingress_message_mode(temp, &conf->rx_sync_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_DELAY_REQUEST_MODE) {
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_get
                               (unit, &reg_value, RX_MODE1_M1f, &temp));
        _drv_gex_timesync_decode_ingress_message_mode(temp, &conf->rx_delay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_REQUEST_MODE) {
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_get
                               (unit, &reg_value, RX_MODE1_M2f, &temp));
        _drv_gex_timesync_decode_ingress_message_mode(temp, &conf->rx_pdelay_request_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_RX_PDELAY_RESPONSE_MODE) {
        SOC_IF_ERROR_RETURN(soc_RX_MODE_PORTr_field_get
                               (unit, &reg_value, RX_MODE1_M3f, &temp));
        _drv_gex_timesync_decode_ingress_message_mode(temp, &conf->rx_pdelay_response_mode);
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE) {
        /* Initial ref phase [15:0] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_2_Nr(unit, 0, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_2_Nr_field_get
                               (unit, &reg_value, REF_PHASE_Nf, &temp));
        /* Initial ref phase [31:16]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_2_Nr(unit, 1, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_2_Nr_field_get
                               (unit, &reg_value, REF_PHASE_Nf, &temp1));
        /*  Initial ref phase [47:32] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_2_Nr(unit, 2, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_2_Nr_field_get
                               (unit, &reg_value, REF_PHASE_Nf, &temp2));

        COMPILER_64_SET(conf->phy_1588_dpll_ref_phase, temp2, ((temp1 << 16) | temp));
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_REF_PHASE_DELTA) {
        /* Ref phase delta [15:0] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_3_Nr(unit, 0, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_3_Nr_field_get
                               (unit, &reg_value, REF_PHASE_DELTA_Nf, &temp));
        /* Ref phase delta [31:16]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_3_Nr(unit, 1, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_3_Nr_field_get
                               (unit, &reg_value, REF_PHASE_DELTA_Nf, &temp1));
        conf->phy_1588_dpll_ref_phase_delta = (temp1 << 16) | temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K1) {
        /* DPLL K1 */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_4r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_4r_field_get
                               (unit, &reg_value, DPLL_K1f, &temp));
        conf->phy_1588_dpll_k1 = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K2) {
        /* DPLL K2 */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_5r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_5r_field_get
                               (unit, &reg_value, DPLL_K2f, &temp));
        conf->phy_1588_dpll_k2 = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_K3) {
        /* DPLL K3 */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_6r(unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_6r_field_get
                               (unit, &reg_value, DPLL_K3f, &temp));
        conf->phy_1588_dpll_k3 = temp;
    }

    if (conf->validity_mask & SOC_PORT_PHY_TIMESYNC_VALID_PHY_1588_DPLL_LOOP_FILTER) {
        /* Initial loop filter[15:0] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 0, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_get
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp));
        /* Initial loop filter[31:16]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 1, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_get
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp1));
        /*  Initial loop filter[47:32] */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 2, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_get
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp2));
        /* Initial loop filter[63:48]  */
        SOC_IF_ERROR_RETURN(REG_READ_NSE_DPLL_7_Nr(unit, 3, &reg_value));
        SOC_IF_ERROR_RETURN(soc_NSE_DPLL_7_Nr_field_get
                               (unit, &reg_value, LOOP_FILTER_Nf, &temp3));
        COMPILER_64_SET(conf->phy_1588_dpll_loop_filter, ((temp3 << 16) | temp2), ((temp1 << 16) | temp));
    }

    return SOC_E_NONE;
}

int 
drv_gex_control_timesync_set(
     int unit, uint32 port, soc_port_control_phy_timesync_t type, uint64 value)
{
    uint32 reg_value = 0, reg_value1 = 0, temp = 0, val32;
    
    COMPILER_REFERENCE(port);
    switch (type) {
        case SOC_PORT_CONTROL_PHY_TIMESYNC_CAPTURE_TIMESTAMP:
        case SOC_PORT_CONTROL_PHY_TIMESYNC_HEARTBEAT_TIMESTAMP:
            return SOC_E_PARAM;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_NCOADDEND:
            reg_value = 0;
            temp = COMPILER_64_LO(value);
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_1_Nr_field_set
                                (unit, &reg_value, NSE_REG_NCO_FREQCNTRL_Nf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_1_Nr(unit, 1, &reg_value));
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_FRAMESYNC:
            SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_6r(unit, &reg_value));
            reg_value1 = reg_value;
            temp = 0x3;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                                (unit, &reg_value1, GMODEf, &temp));
            temp = 0x1;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                                (unit, &reg_value1, NSE_INITf, &temp));
            temp = 0x8;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                                (unit, &reg_value1, FRAMESYN_MODEf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_6r(unit, &reg_value1));
            sal_udelay(1);
            temp = 0x0;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                                (unit, &reg_value1, NSE_INITf, &temp));
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_set
                                (unit, &reg_value1, FRAMESYN_MODEf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_6r(unit, &reg_value1));
            sal_udelay(1);
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_6r(unit, &reg_value));
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_LOCAL_TIME:
            reg_value = 0;
            temp = COMPILER_64_LO(value) & 0xffff;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_2_Nr_field_set
                                (unit, &reg_value, LOCAL_TIME_UP_Nf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_2_Nr(unit, 0, &reg_value));
            reg_value = 0;
            temp = (COMPILER_64_LO(value) >> 16) & 0xffff;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_2_Nr_field_set
                                (unit, &reg_value, LOCAL_TIME_UP_Nf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_2_Nr(unit, 1, &reg_value));

            SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_2_Nr(unit, 2, &reg_value));
            temp = COMPILER_64_HI(value) & 0xfff;
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_2_Nr_field_set
                                (unit, &reg_value, LOCAL_TIME_UP_Nf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_NSE_NCO_2_Nr(unit, 2, &reg_value));
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_LOAD_CONTROL:
            reg_value = 0;
            reg_value1 = 0;
            temp = 1;
            val32 = COMPILER_64_LO(value);
        
            if (val32 &  SOC_PORT_PHY_TIMESYNC_TN_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, IEEE1588_TIME_CODE_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_TN_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, IEEE1588_TIME_CODE_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_TIMECODE_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, TIME_CODE_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_TIMECODE_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value, TIME_CODE_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_SYNCOUT_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, SYNCOUT_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_SYNCOUT_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, SYNCOUT_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, NCO_DIVIDER_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, NCO_DIVIDER_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, LOCAL_TIME_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, LOCAL_TIME_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, FREQ_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, FREQ_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, DPLL_LOOP_FILTER_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, DPLL_LOOP_FILTER_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, DPLL_REF_PHASE_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, DPLL_REF_PHASE_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, DPLL_REF_PHASE_DELTA_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, DPLL_REF_PHASE_DELTA_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_K3_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, DPLL_K3_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_K3_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, DPLL_K3_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_K2_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, DPLL_K2_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_K2_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, DPLL_K2_LOADf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_K1_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_set
                    (unit, &reg_value, DPLL_K1_CTRLf, &temp));
            }
            if (val32 &  SOC_PORT_PHY_TIMESYNC_DPLL_K1_ALWAYS_LOAD) {
                SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_set
                    (unit, &reg_value1, DPLL_K1_LOADf, &temp));
            }

            SOC_IF_ERROR_RETURN(REG_WRITE_SHD_CTLr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_WRITE_SHD_LDr(unit, &reg_value1));
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT:
            return SOC_E_PARAM;
        
        case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT_MASK:
            SOC_IF_ERROR_RETURN(REG_READ_INT_MASKr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_INT_MASKr_field_get
                                (unit, &reg_value, INTC_SOP_MASKf, &temp));
            val32 = COMPILER_64_LO(value);
            if (val32 & SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT_MASK) {
                temp |= (1U << port);
            } else {
                temp &= ~(1U << port);
            }
            SOC_IF_ERROR_RETURN(soc_INT_MASKr_field_set
                                (unit, &reg_value, INTC_SOP_MASKf, &temp));
            temp = 0;
            if (val32 & SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT_MASK) {
                temp = 1;
            }
            SOC_IF_ERROR_RETURN(soc_INT_MASKr_field_set
                                (unit, &reg_value, INTC_FSYNC_MASKf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_INT_MASKr(unit, &reg_value));
            break;
        
        case SOC_PORT_CONTROL_PHY_TIMESYNC_TX_TIMESTAMP_OFFSET:
            if (port == 0) {
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
            } else {
                return SOC_E_PARAM;
            }
            
            val32 = COMPILER_64_LO(value);
            temp = val32 & 0xffff;
            SOC_IF_ERROR_RETURN(soc_TX_PORT_4_TS_OFFSET_LSBr_field_set
                                   (unit, &reg_value, TS_OFFSET_TX_LSBf, &temp));
            temp = (val32 >> 16) & 0xf;
            SOC_IF_ERROR_RETURN(soc_TX_PORT_4_TS_OFFSET_MSBr_field_set
                                   (unit, &reg_value1, TS_OFFSET_TX_MSBf, &temp));
            if (port == 0) {
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
            }
            break;
        
        case SOC_PORT_CONTROL_PHY_TIMESYNC_RX_TIMESTAMP_OFFSET:
            if (port == 0) {
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
            } else {
                return SOC_E_PARAM;
            }
            
            val32 = COMPILER_64_LO(value);
            temp = val32 & 0xffff;
            SOC_IF_ERROR_RETURN(soc_RX_PORT_4_TS_OFFSET_LSBr_field_set
                                   (unit, &reg_value, TS_OFFSET_RX_LSBf, &temp));
            temp = (val32 >> 16) & 0xf;
            SOC_IF_ERROR_RETURN(soc_RX_PORT_4_TS_OFFSET_MSBr_field_set
                                   (unit, &reg_value1, TS_OFFSET_RX_MSBf, &temp));
            if (port == 0) {
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_WRITE_RX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
            }
            break;
        default:
            return SOC_E_PARAM;
    }

    return SOC_E_NONE;
}

int 
drv_gex_control_timesync_get(
     int unit, uint32 port, soc_port_control_phy_timesync_t type, uint64 *value)
{
    uint32 reg_value = 0, reg_value1 = 0, temp = 0, temp1 = 0, val32;
    uint32 value0 = 0, value1 = 0, value2 = 0, value3 = 0;
    
    COMPILER_REFERENCE(port);
    switch (type) {
        case SOC_PORT_CONTROL_PHY_TIMESYNC_HEARTBEAT_TIMESTAMP:
            SOC_IF_ERROR_RETURN(REG_READ_CNTR_DBGr(unit, &reg_value));
            temp = 0x1;
            SOC_IF_ERROR_RETURN(soc_CNTR_DBGr_field_set
                           (unit, &reg_value, HB_CNTLf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_CNTR_DBGr(unit, &reg_value));

            SOC_IF_ERROR_RETURN(REG_READ_HEARTBEAT_Nr(unit, 0, &value0));
            SOC_IF_ERROR_RETURN(REG_READ_HEARTBEAT_Nr(unit, 1, &value1));
            SOC_IF_ERROR_RETURN(REG_READ_HEARTBEAT_Nr(unit, 2, &value2));

            temp = 0x2;
            SOC_IF_ERROR_RETURN(soc_CNTR_DBGr_field_set
                           (unit, &reg_value, HB_CNTLf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_CNTR_DBGr(unit, &reg_value));
            COMPILER_64_SET((*value), (value2),  ((value1 << 16) | value0));
            break;
        
        case SOC_PORT_CONTROL_PHY_TIMESYNC_CAPTURE_TIMESTAMP:
            SOC_IF_ERROR_RETURN(REG_READ_CNTR_DBGr(unit, &reg_value));
            temp = port;
            SOC_IF_ERROR_RETURN(soc_CNTR_DBGr_field_set
                           (unit, &reg_value, TS_SLICE_SELf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_CNTR_DBGr(unit, &reg_value));

            SOC_IF_ERROR_RETURN(REG_READ_TS_READ_START_ENDr(unit, &reg_value));
            /* temp is for start bit, temp1 is for end bit */
            temp = 0x1; 
            temp1 = 0x0;
            if (port == 0) {
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT0_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT0_TS_READ_ENDf, &temp1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT1_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT1_TS_READ_ENDf, &temp1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT2_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT2_TS_READ_ENDf, &temp1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT3_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT3_TS_READ_ENDf, &temp1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT4_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT4_TS_READ_ENDf, &temp1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT5_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT5_TS_READ_ENDf, &temp1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT7_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT7_TS_READ_ENDf, &temp1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT8_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT8_TS_READ_ENDf, &temp1));
            } else {
                return SOC_E_PARAM;
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_TS_READ_START_ENDr(unit, &reg_value));

            SOC_IF_ERROR_RETURN(REG_READ_TIME_STAMP_Nr(unit, 0, &value0));
            SOC_IF_ERROR_RETURN(REG_READ_TIME_STAMP_Nr(unit, 1, &value1));
            SOC_IF_ERROR_RETURN(REG_READ_TIME_STAMP_Nr(unit, 2, &value2));
            SOC_IF_ERROR_RETURN(REG_READ_TIME_STAMP_3r(unit, &value3));

            temp = 0x0; 
            temp1 = 0x1;
            if (port == 0) {
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT0_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT0_TS_READ_ENDf, &temp1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT1_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT1_TS_READ_ENDf, &temp1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT2_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT2_TS_READ_ENDf, &temp1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT3_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT3_TS_READ_ENDf, &temp1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT4_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT4_TS_READ_ENDf, &temp1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT5_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT5_TS_READ_ENDf, &temp1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT7_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT7_TS_READ_ENDf, &temp1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT8_TS_READ_STARTf, &temp));
                SOC_IF_ERROR_RETURN(soc_TS_READ_START_ENDr_field_set
                               (unit, &reg_value, PORT8_TS_READ_ENDf, &temp1));
            }
            SOC_IF_ERROR_RETURN(REG_WRITE_TS_READ_START_ENDr(unit, &reg_value));
            COMPILER_64_SET((*value), ((value3 << 16) | value2),  ((value1 << 16 ) | value0));
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_NCOADDEND:
            SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_1_Nr(unit, 1, &reg_value));
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_1_Nr_field_get
                                (unit, &reg_value, NSE_REG_NCO_FREQCNTRL_Nf, &temp));
            /* *value = value0; */
            COMPILER_64_SET((*value), 0, temp);
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_FRAMESYNC:
            SOC_IF_ERROR_RETURN(REG_READ_NSE_NCO_6r(unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_NSE_NCO_6r_field_get
                                (unit, &reg_value, FRAMESYN_MODEf, &temp));
            COMPILER_64_SET((*value), 0, temp);
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_LOAD_CONTROL:
            SOC_IF_ERROR_RETURN(REG_READ_SHD_CTLr(unit, &reg_value));
            SOC_IF_ERROR_RETURN(REG_READ_SHD_LDr(unit, &reg_value1));

            val32 = 0;
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, IEEE1588_TIME_CODE_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_TN_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, IEEE1588_TIME_CODE_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_TN_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, TIME_CODE_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_TIMECODE_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, TIME_CODE_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_TIMECODE_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, SYNCOUT_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_SYNCOUT_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, SYNCOUT_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_SYNCOUT_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, NCO_DIVIDER_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, NCO_DIVIDER_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_NCO_DIVIDER_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, LOCAL_TIME_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, LOCAL_TIME_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_LOCAL_TIME_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, FREQ_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, FREQ_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_NCO_ADDEND_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, DPLL_LOOP_FILTER_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, DPLL_LOOP_FILTER_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_LOOP_FILTER_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, DPLL_REF_PHASE_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, DPLL_REF_PHASE_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, DPLL_REF_PHASE_DELTA_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, DPLL_REF_PHASE_DELTA_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_REF_PHASE_DELTA_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, DPLL_K3_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_K3_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, DPLL_K3_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_K3_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, DPLL_K2_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_K2_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, DPLL_K2_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_K2_ALWAYS_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_CTLr_field_get
                (unit, &reg_value, DPLL_K1_CTRLf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_K1_LOAD;
            }
            SOC_IF_ERROR_RETURN(soc_SHD_LDr_field_get
                (unit, &reg_value1, DPLL_K1_LOADf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_DPLL_K1_ALWAYS_LOAD;
            }
            COMPILER_64_SET(*value, 0, val32);
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT:
            SOC_IF_ERROR_RETURN(REG_READ_INT_STATr(unit, &reg_value));

            val32 = 0;
            SOC_IF_ERROR_RETURN(soc_INT_STATr_field_get
                                (unit, &reg_value, INTC_SOPf, &temp));
            if (temp & (1U << port)) {
                val32 |= SOC_PORT_PHY_TIMESYNC_TIMESTAMP_INTERRUPT;
            }

            SOC_IF_ERROR_RETURN(soc_INT_STATr_field_get
                                (unit, &reg_value, INTC_FSYNCf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT;
            }
            COMPILER_64_SET(*value, 0, val32);
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_INTERRUPT_MASK:
            SOC_IF_ERROR_RETURN(REG_READ_INT_MASKr(unit, &reg_value));

            val32 = 0;
            SOC_IF_ERROR_RETURN(soc_INT_MASKr_field_get
                                (unit, &reg_value, INTC_SOP_MASKf, &temp));
            if (temp & (1U << port)) {
                val32 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT_MASK;
            }

            SOC_IF_ERROR_RETURN(soc_INT_MASKr_field_get
                                (unit, &reg_value, INTC_FSYNC_MASKf, &temp));
            if (temp) {
                val32 |= SOC_PORT_PHY_TIMESYNC_FRAMESYNC_INTERRUPT_MASK;
            }
            COMPILER_64_SET(*value, 0, val32);
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_TX_TIMESTAMP_OFFSET:
            if (port == 0) {
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_TX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
            } else {
                return SOC_E_PARAM;
            }
            
            SOC_IF_ERROR_RETURN(soc_TX_PORT_0_TS_OFFSET_LSBr_field_get
                                   (unit, &reg_value, TS_OFFSET_TX_LSBf, &temp));
            
            SOC_IF_ERROR_RETURN(soc_TX_PORT_0_TS_OFFSET_MSBr_field_get
                                   (unit, &reg_value1, TS_OFFSET_TX_MSBf, &temp1));
            
            COMPILER_64_SET((*value), 0, ((temp1 << 16) | temp));
            break;
        case SOC_PORT_CONTROL_PHY_TIMESYNC_RX_TIMESTAMP_OFFSET:
            if (port == 0) {
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_0_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 1){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_1_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 2){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_2_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 3){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_3_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 4){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_4_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 5){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_5_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 7){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_7_TS_OFFSET_MSBr(unit, &reg_value1));
            } else if (port == 8){
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_LSBr(unit, &reg_value));
                SOC_IF_ERROR_RETURN(REG_READ_RX_PORT_8_TS_OFFSET_MSBr(unit, &reg_value1));
            } else {
                return SOC_E_PARAM;
            }
            
            SOC_IF_ERROR_RETURN(soc_RX_PORT_0_TS_OFFSET_LSBr_field_set
                                   (unit, &reg_value, TS_OFFSET_RX_LSBf, &temp));
            SOC_IF_ERROR_RETURN(soc_RX_PORT_0_TS_OFFSET_MSBr_field_set
                                   (unit, &reg_value1, TS_OFFSET_RX_MSBf, &temp1));
            COMPILER_64_SET((*value), 0, ((temp1 << 16) | temp));
            break;
        default:
            return SOC_E_PARAM;
    } 

    return SOC_E_NONE;
}

#endif /* POLAR || NORTHSTAR || NORTHSTARPLUS || STARFIGHTER3 */

