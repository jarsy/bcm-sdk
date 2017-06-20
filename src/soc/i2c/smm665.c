/*
 * $Id: smm665.c,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * I2C Device Driver for Summit SMM665C Active DC Control 
 *
 */

#include <sal/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/i2c.h>
#include <soc/smm665.h>
#include <shared/bsl.h>
static dac_calibrate_t *dac_params;
static int dac_param_len;
static uint32 smm_delay = 6000;

int 
smm_is_device_ready(int unit, int devno) 
{
    int rv;
    uint8 saddr;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    saddr = soc_i2c_addr(unit, devno);
    rv = soc_i2c_ack_poll(unit, saddr, 10000);
    if (rv < 0) {
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META_U(unit,
                             "smm_is_device_ready:  timeout waiting for device")));
        rv = SOC_E_TIMEOUT;
    } else {
        rv = SOC_E_NONE;
    }
    return rv;
}

/* ADC based */
int 
smm665_map_channel(int channel)
{
    int ch;
    switch (channel) {
    case 0:
        ch = SMM665_ADC_CH_A;
        break;
    case 1:
        ch = SMM665_ADC_CH_B;
        break;
    case 2:
        ch = SMM665_ADC_CH_C;
        break;
    case 3:
        ch = SMM665_ADC_CH_D;
        break;
    case 4:
        ch = SMM665_ADC_CH_E;
        break;
    case 5:
        ch = SMM665_ADC_CH_F;
        break;
    default:
        /* Failsafe */
        ch = SMM665_ADC_CH_A;
    }
    return ch;
}

int 
smm665_map_name_to_ch(char *name)
{
    int ch;
    if (sal_strcmp(name, "a")==0) {
        ch = SMM665_ADC_CH_A;
    } else if (sal_strcmp(name, "b")==0) {
        ch = SMM665_ADC_CH_B;
    } else if (sal_strcmp(name, "c")==0) {
        ch = SMM665_ADC_CH_C;
    } else if (sal_strcmp(name, "d")==0) {
        ch = SMM665_ADC_CH_D;
    } else if (sal_strcmp(name, "e")==0) {
        ch = SMM665_ADC_CH_E;
    } else if (sal_strcmp(name, "f")==0) {
        ch = SMM665_ADC_CH_F;
    } else if (sal_strcmp(name, "vdd")==0) {
        ch = SMM665_ADC_CH_VDD;
    } else if (sal_strcmp(name, "12vin")==0) {
        ch = SMM665_ADC_CH_12VIN;
    } else if (sal_strcmp(name, "temp")==0) {
        ch = SMM665_ADC_CH_INT_TEMP;
    } else if (sal_strcmp(name, "ain1")==0) {
        ch = SMM665_ADC_CH_AIN1;
    } else if (sal_strcmp(name, "ain2")==0) {
        ch = SMM665_ADC_CH_AIN2;
    } else {
        return 0; /*ch a*/
    }
    return ch;
}

int 
smm665_adc_conversion(int ch, int adc)
{
    int value = 0;
    adc &= SMM665_ADC_MASK;

    switch (ch) {

    case SMM665_ADC_CH_A:
    case SMM665_ADC_CH_B:
    case SMM665_ADC_CH_C:
    case SMM665_ADC_CH_D:
    case SMM665_ADC_CH_E:
    case SMM665_ADC_CH_F:
    case SMM665_ADC_CH_VDD:
        value = (adc * SMM665_VREF_ADC) / 256;
        break;
    case SMM665_ADC_CH_12VIN:
        value = (adc * SMM665_VREF_ADC * 3) / 256;
        break;
    case SMM665_ADC_CH_AIN1:
    case SMM665_ADC_CH_AIN2:
        value = (adc * SMM665_VREF_ADC * 3) / 256;
        break;
    case SMM665_ADC_CH_INT_TEMP:
        if (adc > 511) {
            value = adc / 4;
        } else {
            value = (adc - 0x400) / 4;
        }
        break;
    default:
        LOG_CLI((BSL_META("smm665_adc_conversion: Invalid channel (%d) requested\n"),
                 ch));
        return 0;
    }
    return value;
}

int 
smm665_adc_value_get(int unit, int devno, int ch, int *adc) 
{

    int rv = SOC_E_NONE;
    uint8 rch = 0;
    uint16 data;
    int num_retries = 10;
    uint8 saddr;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    saddr = soc_i2c_addr(unit, devno);
   
    /* Device ADC response is bad with polling mode! */
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    LOG_CLI((BSL_META_U(unit,
                        "\n")));

    do {
 /*
        sal_usecs_t s, e;

        s = sal_time_usecs();
*/
        /* Initiate ADC */
        /*LOG_CLI((BSL_META_U(unit,
                              "smm665_get_adc_value: wrote %x "), ch));*/
        rv = soc_i2c_write_byte(unit, saddr, ch);
        if (SOC_FAILURE(rv)) {
            return rv;
        }
    
#if 0
        rv = smm_is_device_ready(unit, devno);
        if (SOC_FAILURE(rv)) {
            return rv; 
        }
#else
        sal_usleep(smm_delay); 
#endif
        rv = soc_i2c_read_word(unit, saddr, &data);
        soc_i2c_device(unit, devno)->rbyte +=2;   
        rch = (data >> 8) & SMM665_ADC_CH_MASK;
        /*e = sal_time_usecs();
        LOG_CLI((BSL_META_U(unit,
                            "\n Response Time Stamp: [%u]"), SAL_USECS_SUB(e, s)));
        s = sal_time_usecs();*/
        LOG_CLI((BSL_META_U(unit,
                            "Addr %d Ch:%x: got ch echo %x read value %x\n"),
                 saddr, ch, rch, data));
        /*e = sal_time_usecs();
        LOG_CLI((BSL_META_U(unit,
                            "\n Print time Time Stamp: [%u]"), SAL_USECS_SUB(e, s)));*/
        *adc = data;
    } while ((rch != ch) && (num_retries--));
        
    return rv;
}

/*
 * Read the SMM to get the voltage based on ADC
 * Handles both temperature sensor and voltage
 * Caller must handle units 
 */
int 
smm665_channel_voltage_get(int unit, int devno, int ch, int *value)
{

    int rv, adc_val=0;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    rv = smm665_adc_value_get(unit, devno, ch, &adc_val);
    if (SOC_SUCCESS(rv)) {
        *value = smm665_adc_conversion(ch, adc_val);
    }
    return rv; 
}

/*
 * Read the SMM to get the adoc status of channel
 */
int 
smm665_channel_status_get(int unit, int devno, int ch , int *value)
{
    int rv;
    uint8 saddr = soc_i2c_addr(unit, devno);
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, saddr, ch, (uint16*)value);
    soc_i2c_device(unit, devno)->rbyte +=2;   
    LOG_CLI((BSL_META_U(unit,
                        "smm665_get_channel_status: got %x"), *value));

    return rv; 
}

/*
 * Read the SMM to get the device status
 */
int 
smm665_device_status_get(int unit, int devno, int *value)
{
    int rv;
    uint8 saddr = soc_i2c_addr(unit, devno);
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, saddr, SMM665_DEVICE_STATUS_REG,
                               (uint16*)value);
    soc_i2c_device(unit, devno)->rbyte +=2;   
    LOG_CLI((BSL_META_U(unit,
                        "smm665_device_status_get: got %x"), *value));

    return rv; 
}

/*
 * Check CFG lock status
 * Locking config is not reversible!
 */
int 
smm665_check_cfg_lock(int unit, int devno, int *locked)
{
    int rv = SOC_E_NONE;
    uint8 data = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_byte_data(unit, saddr, SMM665_CONFIG_CONTROL_REG, &data);
    if (SOC_SUCCESS(rv)) {
        *locked = (data & SMM665_CONFIG_CFG_LOCKED);
        soc_i2c_device(unit, devno)->tbyte +=2;   
    }
    return rv;
}

/*
 * Set ADOC control for channel
 */
int 
sm665_enable_adoc_control(int unit, int devno, uint8 ch)
{
    int rv = SOC_E_NONE;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_write_byte_data(unit, saddr, SMM665_CONFIG_CONTROL_REG, ch);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->tbyte +=2;   
    }
    return rv;
}

/*
 * Lock or Unlock the SMM config/mem space
 */
int 
smm665_write_protect_set(int unit, int devno, int lock)
{
    int rv = SOC_E_NONE;
    uint8 data = 0;
    uint8 saddr = soc_i2c_addr(unit, devno);

#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    if (lock) {
        data = SMM665_LOCK;
    } else {
        data = SMM665_UNLOCK;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_write_byte_data(unit, saddr, SMM665_WRITE_PROTECT_REG, data);
    soc_i2c_device(unit, devno)->tbyte +=2;   
    return rv;
}

uint8 
smm665_command_map_ch_to_pos(int ch) 
{
    switch (ch) {
    case 0: return SMM665_COMMAND_CH_A_POS;
    case 1: return SMM665_COMMAND_CH_B_POS;
    case 2: return SMM665_COMMAND_CH_C_POS;
    case 3: return SMM665_COMMAND_CH_D_POS;
    case 4: return SMM665_COMMAND_CH_E_POS;
    case 5: return SMM665_COMMAND_CH_F_POS;
    default:
        LOG_CLI((BSL_META("ERROR: map_ch_to_pos unknown channel (%d)"), ch));
        return 0xff;
    }
}

uint8 
smm665_map_ch_to_reg(int ch, int base) {
    int reg = 0;
    switch (ch) {
    case 0:
        reg = SMM665_VO_CTRL_CH_1; break;
    case 1:
        reg = SMM665_VO_CTRL_CH_2; break;
    case 2:
        reg = SMM665_VO_CTRL_CH_3; break;
    case 3:
        reg = SMM665_VO_CTRL_CH_4; break;
    case 4:
        reg = SMM665_VO_CTRL_CH_5; break;
    case 5:
        reg = SMM665_VO_CTRL_CH_6; break;
    default:
        LOG_CLI((BSL_META("ERROR: smm665_map_ch_to_reg unknown channel (%d)"), ch));
        return 0xff;
    }
    return (base + reg);
}

int
get_multiplier(int voltage) 
{
    if (voltage > SMM665_VREF_CTRL) {
        return SMM665_VREF_MULT_1;
    }
    else if (voltage > SMM665_80PERCENT_VREF_CTRL) {
        return SMM665_VREF_MULT_0_75;
    }
    else if (voltage > SMM665_55PERCENT_VREF_CTRL) {
        return SMM665_VREF_MULT_0_50;
    }
    else if (voltage > SMM665_30PERCENT_VREF_CTRL) {
        return SMM665_VREF_MULT_0_25;
    } else {
        LOG_CLI((BSL_META("ERROR: Invalid voltage cant figure multipler")));
        return SMM665_VREF_MULT_1;
    }
}

int 
get_dac_value(int voltage) 
{
    int mult;
    int dac, ret;
    mult = get_multiplier(voltage);

    if (mult == SMM665_VREF_MULT_1) {
        /* dac = 1024 * vref_ctl / adoc_voltage */
        dac = 1024 * SMM665_VREF_CTRL / voltage;
    }
    else if (mult == SMM665_VREF_MULT_0_75) {
        /* dac = 1024 * 0.75 * vref_ctl / adoc_voltage */
        dac = 768 * SMM665_VREF_CTRL / voltage;
    }
    else if (mult == SMM665_VREF_MULT_0_50) {
        /* dac = 1024 * 0.50 * vref_ctl / adoc_voltage */
        dac = 512 * SMM665_VREF_CTRL / voltage;
    }
    else if (mult == SMM665_VREF_MULT_0_25) {
        /* dac = 1024 * 0.25 * vref_ctl / adoc_voltage */
        dac = 256 * SMM665_VREF_CTRL / voltage;
    } else {
        /* Defaulting to multiplier 1 */
        /* dac = 1024 * vref_ctl / adoc_voltage */
        dac = 1024 * SMM665_VREF_CTRL / voltage;
    }

    /* [x x x x m m d d] [d d d d d d d d] */
    ret = (mult << 2) | SMM665_ADOC_CONTROL_HI(dac);
    ret = (ret << 8) | SMM665_ADOC_CONTROL_LOW(dac);

    return (ret);
}

int 
smm665_error_correction(int vref_ctrl, uint16 err) 
{
    int err_voltage = 0;
    err_voltage = 1000 * ((err & SMM665_ERR_COEF_PERCENT_MASK) >> 
                             SMM665_ERR_COEF_PERCENT_SHIFT);
    err_voltage += 100 * ((err & SMM665_ERR_COEF_TENTH_PERCENT_MASK) >> 
                             SMM665_ERR_COEF_TENTH_PERCENT_SHIFT);
    err_voltage += 10 * ((err & SMM665_ERR_COEF_HUNDREDTH_PERCENT_MASK) >> 
                             SMM665_ERR_COEF_HUNDREDTH_PERCENT_SHIFT);
    err_voltage += (err & SMM665_ERR_COEF_THOUSANDTH_PERCENT_MASK) >> 
                             SMM665_ERR_COEF_THOUSANDTH_PERCENT_SHIFT;
    if (err & SMM665_ERR_COEF_NEGATIVE_BIAS) {
        err_voltage = -err_voltage;
    }
    return (vref_ctrl * (1 + err_voltage/1000));
}

int 
smm655_get_voltage(int unit, int devno, uint16 dac) 
{

    /* This routine applies error coeffcients */
    int mult, rv;
    int voltage = 0;
    uint16 err = 0;
    int vref_ctrl_accurate = SMM665_VREF_CTRL;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    mult = SMM665_VREF_MULTIPLIER_DAC(dac);
    if (mult == SMM665_VREF_MULT_1) {
        /* adoc_voltage = 1024 * vref_ctl / dac */
        voltage = 1024 * vref_ctrl_accurate / dac;
    }
    else if (mult == SMM665_VREF_MULT_0_75) {
        /* Read the error coeffecient and apply */
        reg = SMM665_ERR_COEF_REG_MULT_0_75;
        rv = soc_i2c_read_word_data(unit, saddr, reg, &err);
        if (SOC_SUCCESS(rv)) {
            soc_i2c_device(unit, devno)->rbyte += 2;
            vref_ctrl_accurate = smm665_error_correction(SMM665_VREF_CTRL, err);
        }
        
        /* adoc_voltage = 1024 * 0.75 * vref_ctl / dac */
        voltage = 768 * vref_ctrl_accurate / dac;
    }
    else if (mult == SMM665_VREF_MULT_0_50) {
        /* Read the error coeffecient and apply */
        reg = SMM665_ERR_COEF_REG_MULT_0_50;
        rv = soc_i2c_read_word_data(unit, saddr, reg, &err);
        if (SOC_SUCCESS(rv)) {
            soc_i2c_device(unit, devno)->rbyte += 2;
            vref_ctrl_accurate = smm665_error_correction(SMM665_VREF_CTRL, err);
        }
        /* adoc_voltage = 1024 * 0.50 * vref_ctl / dac */
        voltage = 512 * vref_ctrl_accurate / dac;
    }
    else if (mult == SMM665_VREF_MULT_0_25) {
        /* Read the error coeffecient and apply */
        reg = SMM665_ERR_COEF_REG_MULT_0_25;
        rv = soc_i2c_read_word_data(unit, saddr, reg, &err);
        if (SOC_SUCCESS(rv)) {
            soc_i2c_device(unit, devno)->rbyte += 2;
            vref_ctrl_accurate = smm665_error_correction(SMM665_VREF_CTRL, err);
        }
        /* adoc_voltage = 1024 * 0.25 * vref_ctl / dac */
        voltage = 256 * vref_ctrl_accurate / dac;
    }

    return (voltage);
}
 
int 
get_ch_nominal_voltage(int unit, int devno, int ch, int *voltage)
{
    uint16 dac_value;
    int rv = SOC_E_NONE;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    reg = smm665_map_ch_to_reg(ch, SMM665_VO_CTRL_NOMINAL);
    if (reg == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, saddr, reg, &dac_value);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->rbyte += 2;
        *voltage = smm655_get_voltage(unit, devno, dac_value);
    }
    return rv;
}

int 
set_ch_nominal_voltage(int unit, int devno, int ch, int voltage)
{
    uint16 dac_value;
    int rv = SOC_E_NONE;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    dac_value = get_dac_value(voltage);
    reg = smm665_map_ch_to_reg(ch, SMM665_VO_CTRL_NOMINAL);
    if (reg == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_write_word_data(unit, saddr, reg, dac_value);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->tbyte += 2;
    }
    return SOC_E_NONE;
}

int get_ch_margin_high_voltage(int unit, int devno, int ch, int *voltage)
{
    uint16 dac_value;
    int rv = SOC_E_NONE;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    reg = smm665_map_ch_to_reg(ch, SMM665_VO_CTRL_MARGIN_HIGH);
    if (reg == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, saddr, reg, &dac_value);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->rbyte += 2;
        *voltage = smm655_get_voltage(unit, devno, dac_value);
    }
    return rv;
}

int 
set_ch_margin_high_voltage(int unit, int devno, int ch, int voltage)
{
    uint16 dac_value;
    int rv = SOC_E_NONE;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    dac_value = get_dac_value(voltage);
    reg = smm665_map_ch_to_reg(ch, SMM665_VO_CTRL_MARGIN_HIGH);
    if (reg == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_write_word_data(unit, saddr, reg, dac_value);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->tbyte += 2;
    }
    return SOC_E_NONE;
}

int 
get_ch_margin_low_voltage(int unit, int devno, int ch, int *voltage)
{
    uint16 dac_value;
    int rv = SOC_E_NONE;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    reg = smm665_map_ch_to_reg(ch, SMM665_VO_CTRL_MARGIN_LOW);
    if (reg == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, saddr, reg, &dac_value);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->rbyte += 2;
        *voltage = smm655_get_voltage(unit, devno, dac_value);
    }
    return rv;
}

int 
set_ch_margin_low_voltage(int unit, int devno, int ch, int voltage)
{
    uint16 dac_value;
    int rv = SOC_E_NONE;
    uint8 reg = 0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    dac_value = get_dac_value(voltage);
    reg = smm665_map_ch_to_reg(ch, SMM665_VO_CTRL_MARGIN_LOW);
    if (reg == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_write_word_data(unit, saddr, reg, dac_value);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->tbyte += 2;
    }
    return SOC_E_NONE;
}


/* COMMANDS */
 
int 
set_ch_margin_high(int unit, int devno, int ch) 
{
    uint16 rcmd = 0, wcmd = 0;
    int rv = SOC_E_NONE;
    int pos = smm665_command_map_ch_to_pos(ch);
    uint8 raddr = I2C_ADOC_SADDR0;
    uint8 saddr = I2C_ADOC_SADDR3;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
 
    if (pos == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, raddr, SMM665_ADOC_STATUS_REG, &rcmd);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->rbyte += 2;
        rcmd &= ~(SMM665_COMMAND_MASK << pos);
        wcmd = (rcmd & SMM665_ADOC_CH_STATUS) |
                (SMM665_COMMAND_MARGIN_HIGH << pos);
        rv = soc_i2c_write_word_data(unit, saddr, SMM665_COMMAND_REG, wcmd);
        if (SOC_SUCCESS(rv)) {
            soc_i2c_device(unit, devno)->tbyte += 2;
        }
    }
    return SOC_E_NONE;
}

int 
set_ch_margin_low(int unit, int devno, int ch) 
{
    uint16 rcmd = 0, wcmd = 0;
    int rv = SOC_E_NONE;
    int pos = smm665_command_map_ch_to_pos(ch);
    uint8 saddr = I2C_ADOC_SADDR3;
    uint8 raddr = I2C_ADOC_SADDR0;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    if (pos == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, raddr, SMM665_ADOC_STATUS_REG, &rcmd);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->rbyte += 2;
        rcmd &= ~(SMM665_COMMAND_MASK << pos);
        wcmd = (rcmd & SMM665_ADOC_CH_STATUS) |
                 (SMM665_COMMAND_MARGIN_LOW << pos);
        rv = soc_i2c_write_word_data(unit, saddr, SMM665_COMMAND_REG, wcmd);
        if (SOC_SUCCESS(rv)) {
            soc_i2c_device(unit, devno)->tbyte += 2;
        }
    }
    return SOC_E_NONE;
}

int 
set_ch_nominal(int unit, int devno, int ch) 
{
    uint16 rcmd = 0, wcmd = 0;
    int rv = SOC_E_NONE;
    int pos = smm665_command_map_ch_to_pos(ch);
    uint8 saddr = I2C_ADOC_SADDR3;
    uint8 raddr = I2C_ADOC_SADDR0;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    if (pos == 0xff) {
        return SOC_E_PARAM;
    }
    rv = smm_is_device_ready(unit, devno);
    if (SOC_FAILURE(rv)) {
        return rv; 
    }
    rv = soc_i2c_read_word_data(unit, raddr, SMM665_ADOC_STATUS_REG, &rcmd);
    if (SOC_SUCCESS(rv)) {
        soc_i2c_device(unit, devno)->rbyte += 2;
        rcmd &= ~(SMM665_COMMAND_MASK << pos);
        wcmd = (rcmd & SMM665_ADOC_CH_STATUS) |
                 (SMM665_COMMAND_NOMINAL << pos);
        rv = soc_i2c_write_word_data(unit, saddr, SMM665_COMMAND_REG, wcmd);
        if (SOC_SUCCESS(rv)) {
            soc_i2c_device(unit, devno)->tbyte += 2;
        }
    }
    return SOC_E_NONE;
}

int 
set_ch_max_voltage(int unit, int devno, int ch) 
{
    int rv, voltage;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    voltage = dac_params[ch].dac_max_hwval;
    rv = set_ch_margin_high_voltage(unit, devno, ch, voltage);
    if (SOC_SUCCESS(rv)) {
        rv = set_ch_margin_high(unit, devno, ch);
    }
    return rv;
}

int 
set_ch_min_voltage(int unit, int devno, int ch) 
{
    int rv, voltage;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    voltage = dac_params[ch].dac_min_hwval;
    rv = set_ch_margin_low_voltage(unit, devno, ch, voltage);
    if (SOC_SUCCESS(rv)) {
        rv = set_ch_margin_low(unit, devno, ch);
    }
    return rv;
}

int 
set_ch_mid_voltage(int unit, int devno, int ch) 
{
    int rv, voltage;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    voltage = dac_params[ch].dac_mid_hwval;
    rv = set_ch_nominal_voltage(unit, devno, ch, voltage);
    if (SOC_SUCCESS(rv)) {
        rv = set_ch_nominal(unit, devno, ch);
    }
    return rv;
}



STATIC int
smm665_read(int unit, int devno,
	  uint16 addr, uint8* data, uint32* len)
{
    
    int rv = SOC_E_NONE;
    uint8 saddr;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    saddr = soc_i2c_addr(unit, devno);

    if (!*len) {
        return SOC_E_NONE;
    }

    /* reads a single byte from a device, from a designated register*/
    if (*len == 1) {
        rv = soc_i2c_read_byte_data(unit, saddr, addr,data);
        soc_i2c_device(unit, devno)->rbyte++;   
    } else if (*len == 2) {
        rv = soc_i2c_read_word_data(unit, saddr, addr,(uint16 *)data);
        soc_i2c_device(unit, devno)->rbyte +=2;   
    } else {
        /* not supported for now */
    }

    return rv;
}

STATIC int
smm665_write(int unit, int devno,
	   uint16 addr, uint8* data, uint32 len)
{
    int rv = SOC_E_NONE;
    uint8 saddr;
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    saddr = soc_i2c_addr(unit, devno);

    if (len == 0) {
        /* simply writes command code to device */
        rv = soc_i2c_write_byte(unit, saddr, addr);
    } else if (len == 1) {
        rv = soc_i2c_write_byte_data(unit, saddr, addr,*data);
        soc_i2c_device(unit, devno)->tbyte++;   
    } else if (len == 2) {
        rv = soc_i2c_write_word_data(unit, saddr, addr,
             (data[1] << 8) | data[0]);
        soc_i2c_device(unit, devno)->tbyte += 2;   
    }
    return rv;
}


STATIC int
smm665_ioctl(int unit, int devno,
	   int opcode, void* data, int len)
{
    int rv = SOC_E_NONE;
    i2c_smm655_t smm655_data;
    dac_calibrate_t *params = NULL;
    int voltage = 0;
    int ch;
#ifdef  COMPILER_HAS_DOUBLE
    double fval;
#else
    int fval;
#endif
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif

    /* length field is actually used as an index into the dac_params table*/

    switch ( opcode ){

	/* Upload calibration table */
    case I2C_DAC_IOC_SET_CALTAB:
        params = (dac_calibrate_t*)data;
        dac_params = params;
        dac_param_len = len;
	break;

    case I2C_DAC_IOC_SETDAC_MIN:
	/* Set MIN voltage */
	rv = set_ch_min_voltage(unit, devno, len);
	break;

    case I2C_DAC_IOC_SETDAC_MAX:
	/* Set MAX voltage */
	rv = set_ch_max_voltage(unit, devno, len);
	break;

    case I2C_DAC_IOC_SETDAC_MID:
	/* Set mid-range voltage */
	rv = set_ch_mid_voltage(unit, devno, len);
	break;

    case I2C_DAC_IOC_CALIBRATE_MAX:
	/* Set MAX output value (from ADC) */
#ifdef	COMPILER_HAS_DOUBLE
	fval = *((double*)data);
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "CALIBRATE_MAX setting %f\n"), fval));
#else
	fval = *((int *)data);
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "CALIBRATE_MAX setting %d\n"), fval));
#endif
	dac_params[len].max = fval;
	break;

    case I2C_DAC_IOC_CALIBRATE_MIN:
	/* Set MIN output value (from ADC) */
#ifdef	COMPILER_HAS_DOUBLE
	fval = *((double*)data);
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "CALIBRATE_MIN setting %f\n"), fval));
#else
	fval = *((int *)data);
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "CALIBRATE_MIN setting %d\n"), fval));
#endif
	dac_params[len].min = fval;
	break;

    case I2C_DAC_IOC_CALIBRATE_STEP:
	/* Calibrate stepsize */
	dac_params[len].step =
            (dac_params[len].use_max ? -1 : 1) *
	    (dac_params[len].max - dac_params[len].min) / 
               (dac_params[len].dac_max_hwval - dac_params[len].dac_min_hwval);
#ifdef	COMPILER_HAS_DOUBLE
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit %d i2c %s: SMM665c calibration on function %s:"
                                "(max=%f,min=%f,step=%f)\n"),
                     unit, soc_i2c_devname(unit,devno),
                     dac_params[len].name,
                     dac_params[len].max,
                     dac_params[len].min,
                     dac_params[len].step));
#else
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit %d i2c %s: SMM665c calibration on function %s:"
                                "(max=%d,min=%d,step=%d)\n"),
                     unit, soc_i2c_devname(unit,devno),
                     dac_params[len].name,
                     dac_params[len].max,
                     dac_params[len].min,
                     dac_params[len].step));
#endif

	break;

    case I2C_DAC_IOC_SET_VOUT:
	/* Set output voltage */
#ifdef	COMPILER_HAS_DOUBLE
	fval = *((double*)data);
        voltage = fval * 1000; /* convert to mV */
#else
	voltage = *((int *)data);
#endif
        if ((voltage > dac_params[len].dac_max_hwval)  ||
            (voltage < dac_params[len].dac_min_hwval)) {
	    LOG_ERROR(BSL_LS_SOC_I2C,
                      (BSL_META_U(unit,
                                  "unit %d i2c %d: "
                                  "smm665c given voltage beyond range for ch %d"),
                       unit, devno, len));
            return SOC_E_PARAM;
        }
        rv = set_ch_nominal_voltage(unit, devno, len, voltage);
	return rv;

    case I2C_SMM_CFG_NOMINAL:
#ifdef	COMPILER_HAS_DOUBLE
	fval = *((double*)data);
        voltage = fval * 1000; /* convert to mV */
#else
	voltage = *((int *)data);
#endif
        rv = set_ch_nominal_voltage(unit, devno, len, voltage);
	break;

    case I2C_SMM_CFG_MARGIN_HIGH:
#ifdef	COMPILER_HAS_DOUBLE
	fval = *((double*)data);
        voltage = fval * 1000; /* convert to mV */
#else
	voltage = *((int *)data);
#endif
        rv = set_ch_margin_high_voltage(unit, devno, len, voltage);
	break;

    case I2C_SMM_CFG_MARGIN_LOW:
#ifdef	COMPILER_HAS_DOUBLE
	fval = *((double*)data);
        voltage = fval * 1000; /* convert to mV */
#else
	voltage = *((int *)data);
#endif
        rv = set_ch_margin_low_voltage(unit, devno, len, voltage);
	break;

    case I2C_SMM_SET_NOMINAL:
	/* Set nominal voltage as output */
        rv = set_ch_nominal(unit, devno, len);
	break;

    case I2C_SMM_SET_MARGIN_HIGH:
	/* Set margin high voltage as output */
	rv = set_ch_margin_high(unit, devno, len);
	break;

    case I2C_SMM_SET_MARGIN_LOW:
	/* Set margin low voltage as output */
	rv = set_ch_margin_low(unit, devno, len);
	break;

    case I2C_SMM_DEVICE_STATUS_GET:
         rv = smm665_device_status_get(unit, devno, data);
	 break;

    case I2C_SMM_CHANNEL_STATUS_GET:
         memcpy(&smm655_data, data, sizeof(smm655_data));
         ch = smm665_map_name_to_ch(smm655_data.name);
         rv = smm665_channel_status_get(unit, devno, ch, 
                                &smm655_data.value);
         memcpy(data, &smm655_data, sizeof(smm655_data));
	 break;

    case I2C_SMM_CHANNEL_VOLTAGE_GET:
         memcpy(&smm655_data, data, sizeof(smm655_data));
         if (sal_strcmp(smm655_data.name, "temp")==0) {
             rv = SOC_E_PARAM;
         } else {
             ch = smm665_map_name_to_ch(smm655_data.name);
             rv = smm665_channel_voltage_get(unit, devno, ch, 
                                &smm655_data.value);
             memcpy(data, &smm655_data, sizeof(smm655_data));
         }
	 break;

    case I2C_SMM_INT_TEMP_GET:
         memcpy(&smm655_data, data, sizeof(smm655_data));
         if (sal_strcmp(smm655_data.name, "temp")==0) {
             ch = smm665_map_name_to_ch(smm655_data.name);
             rv = smm665_channel_voltage_get(unit, devno, ch, 
                                &smm655_data.value);
             memcpy(data, &smm655_data, sizeof(smm655_data));
         } else {
             rv = SOC_E_PARAM;
         }
	 break;
    case I2C_ADC_SET_BOARD_TYPE:
         /* nothing to do */
         break;

    case I2C_ADC_QUERY_CHANNEL:
         if (data) {
             ch = smm665_map_channel(len);
             smm665_channel_voltage_get(unit, devno, ch, &voltage);
#ifdef	COMPILER_HAS_DOUBLE
	     fval = (double)voltage / 1000 /* convert from mV */;
#else
	     fval = voltage;
#endif
             
             ((i2c_adc_t*)data)->max = 
                ((i2c_adc_t*)data)->min = 
                   ((i2c_adc_t*)data)->val = fval;
             ((i2c_adc_t*)data)->delta = 0; 
             ((i2c_adc_t*)data)->nsamples = 1;
         } else {
             rv = SOC_E_PARAM;
         }
         break;
    case I2C_SMM_SET_ADC_DELAY:
	/* Set MIN voltage */
        if (data) {
	    smm_delay = *(uint32*)data;
        }
	break;
    default:
	LOG_VERBOSE(BSL_LS_SOC_COMMON,
                    (BSL_META_U(unit,
                                "unit %d i2c %s: smm665c_ioctl: invalid opcode (%d)\n"),
                     unit, soc_i2c_devname(unit,devno),
                     opcode));
	break;
    }

    sal_udelay(10000);
    
    return rv;
}

STATIC int
smm665_init(int unit, int devno,
	  void* data, int len)
{
#ifdef BCM_CALADAN3_SVK
    unit = -1;
#endif
    /* Unlock config (diable Write protectection) */
    smm665_write_protect_set(unit, devno, 0);

    /* Enable all channels for Output Control */
    sm665_enable_adoc_control(unit, devno, 
                        SMM665_CONFIG_CH_ALL_ADOC_ENABLED);
    
    soc_i2c_devdesc_set(unit, devno, "SMM665C Voltage Control");

    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "smm665c_init: %s, devNo=0x%x\n"),
                 soc_i2c_devname(unit, devno),devno));

    return SOC_E_NONE;
}


/* SMM665C Active DC voltage control Chip Driver callout */
i2c_driver_t _soc_i2c_smm665c_driver = {
    0x0, 0x0, /* System assigned bytes */
    SMM665C_DEVICE_TYPE,
    smm665_read,
    smm665_write,
    smm665_ioctl,
    smm665_init,
    NULL,
};

