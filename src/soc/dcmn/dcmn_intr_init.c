
/*
 * $Id: soc_dcmn_intr_handler.c, v1 16/06/2014 09:55:39 azarrin $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement soc interrupt handler.
 */

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>

#include <soc/intr.h>
#include <soc/ipoll.h>
#include <soc/dpp/JER/jer_intr_cb_func.h>
#include <soc/dpp/JER/jer_intr.h>
#include <soc/dpp/JER/jer_defs.h>

#include <soc/dcmn/dcmn_intr_handler.h>
#include <soc/dcmn/error.h>
#include <soc/drv.h>
#include <soc/register.h>

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

/*************
 * DECLARATIONS *
 *************/

/*************
 * FUNCTIONS *
 *************/
STATIC 
int
soc_dcmn_ser_init_cb(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    soc_reg_t reg = ainfo->reg;
    char *reg_name = SOC_REG_NAME(unit,reg);
    int rc;
    int inst=0;
    int blk;
    soc_reg_above_64_val_t above_64;
    SOCDNX_INIT_FUNC_DEFS;
    
    if(sal_strstr(reg_name, "MEM_MASK") == NULL ||  sal_strstr(reg_name, "MESH_TOPOLOGY_ECC_ERR") != NULL)
        SOC_EXIT;

    /* unmask SER monitor registers*/
    SOC_REG_ABOVE_64_ALLONES(above_64);

    /*exception - PACKET_CRC ecc ser protection should not be enabled according HW bug */
    if (reg == IDR_ECC_ERR_2B_MONITOR_MEM_MASKr) {
        soc_reg_above_64_field32_set(unit, reg, above_64, PACKET_CRC_ECC_2B_ERR_MASKf, 0); 
    }
    if (reg == IDR_ECC_ERR_1B_MONITOR_MEM_MASKr) {
        soc_reg_above_64_field32_set(unit, reg, above_64, PACKET_CRC_ECC_1B_ERR_MASKf, 0); 
    }

    if (SOC_IS_JERICHO(unit)) {
        /* exception - ASIC bug that cause the IPT.LargeLatencyPktDrop interrupt to jump before we have trafic. */
        /* if trafic started, its ok to turn on the feature */
        if (reg == IPT_PACKET_LATENCY_MEASURE_CFGr) {
            soc_reg_above_64_field32_set(unit, reg, above_64, EN_LATENCY_DROPf , 0); 
        }
		
        /* This memories couse ECI.Ecc2bErrInt */
        /* They are debug memories so its ok to mask them */
        if (reg == ECI_ECC_ERR_1B_MONITOR_MEM_MASKr) {
            soc_field_t field = SOC_IS_QAX(unit) ? MBU_MEM_ECC_1B_ERR_MASKf : (SOC_IS_JERICHO_PLUS(unit) ? ECC_ERR_1B_MONITOR_MEM_MASKf : FIELD_0_0f);
            soc_reg_above_64_field32_set(unit, reg, above_64, field , 0); 
        }
        if (reg == ECI_ECC_ERR_2B_MONITOR_MEM_MASKr) {
            soc_field_t field = SOC_IS_QAX(unit) ? MBU_MEM_ECC_2B_ERR_MASKf : (SOC_IS_JERICHO_PLUS(unit) ? ECC_ERR_2B_MONITOR_MEM_MASKf : FIELD_0_0f);
            soc_reg_above_64_field32_set(unit, reg, above_64, field , 0); 
        }

        /*exception - MMU_LBM ecc ser protection should not be enabled according HW bug */
        if (reg == MMU_ECC_ERR_1B_MONITOR_MEM_MASK_1r) {
            soc_reg_above_64_field32_set(unit, reg, above_64, LBM_ECC_1B_ERR_MASKf, 0); 
        }
        if (reg == MMU_ECC_ERR_2B_MONITOR_MEM_MASK_1r) {
            soc_reg_above_64_field32_set(unit, reg, above_64, LBM_ECC_2B_ERR_MASKf, 0); 
        }

        /*The memory FLEX_VER_MASK_TEMP has HW bug*/
        if ((reg == OAMP_ECC_ERR_1B_MONITOR_MEM_MASKr) && SOC_IS_QAX(unit) && !SOC_IS_QUX(unit)) {
            soc_reg_above_64_field32_set(unit, reg, above_64, FLEX_VER_MASK_TEMP_ECC_1B_ERR_MASKf, 0);
        }
        if ((reg == OAMP_ECC_ERR_2B_MONITOR_MEM_MASKr) && SOC_IS_QAX(unit) && !SOC_IS_QUX(unit)) {
            soc_reg_above_64_field32_set(unit, reg, above_64, FLEX_VER_MASK_TEMP_ECC_2B_ERR_MASKf, 0);
        }

        /*The memory FDT_IPT_PAYLOAD_FIFO is readonly, so the hardware has bug*/
        if ((reg == FDT_ECC_ERR_1B_MONITOR_MEM_MASKr) && SOC_IS_JERICHO_PLUS_ONLY(unit)) {
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_0_ECC_1B_ERR_MASKf, 0);
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_1_ECC_1B_ERR_MASKf, 0);
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_2_ECC_1B_ERR_MASKf, 0);
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_3_ECC_1B_ERR_MASKf, 0);
        }
        if ((reg == FDT_ECC_ERR_2B_MONITOR_MEM_MASKr) && SOC_IS_JERICHO_PLUS_ONLY(unit)) {
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_0_ECC_2B_ERR_MASKf, 0);
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_1_ECC_2B_ERR_MASKf, 0);
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_2_ECC_2B_ERR_MASKf, 0);
            soc_reg_above_64_field32_set(unit, reg, above_64, IPT_PAYLOAD_FIFO_3_ECC_2B_ERR_MASKf, 0);
        }

        if ((reg == OAMP_ECC_ERR_1B_MONITOR_MEM_MASKr) && SOC_IS_QUX(unit)) {
            soc_reg_above_64_field32_set(unit, reg, above_64, RMEP_DB_EXT_ECC_1B_ERR_MASKf, 0);
        }
        if ((reg == OAMP_ECC_ERR_2B_MONITOR_MEM_MASKr) && SOC_IS_QUX(unit)) {
            soc_reg_above_64_field32_set(unit, reg, above_64, RMEP_DB_EXT_ECC_2B_ERR_MASKf, 0);
        }
    }

    SOC_BLOCK_ITER_ALL(unit, blk, SOC_REG_FIRST_BLK_TYPE(SOC_REG_INFO(unit, reg).block)) {
        if (SOC_INFO(unit).block_valid[blk]) {
            rc = soc_reg_above_64_set(unit, reg, inst, 0, above_64);
            SOCDNX_IF_ERR_EXIT(rc);
        }
        inst++;
    }
exit:
    SOCDNX_FUNC_RETURN;

}

int
soc_dcmn_ser_init(int unit)
{

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(soc_reg_iterate(unit, soc_dcmn_ser_init_cb, NULL));


exit:
    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME
