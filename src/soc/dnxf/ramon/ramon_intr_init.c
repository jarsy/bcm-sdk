
/*
 * $Id: ramon_intr_init.c, v1 16/06/2014 09:55:39 azarrin $
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

#include <soc/dnxf/cmn/dnxf_warm_boot.h>

#include <soc/dnxf/ramon/ramon_intr_cb_func.h>
#include <soc/dnxf/ramon/ramon_intr_corr_act_func.h>
#include <soc/dnxf/ramon/ramon_intr.h>
#include <soc/dnxf/ramon/ramon_defs.h>

#include <soc/dnxc/legacy/dnxc_intr_handler.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>
#include <soc/drv.h>

#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

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
static 
soc_reg_t soc_ramon_interrupt_monitor_mem_reg[] = {
    BRDC_CCS_ECC_ERR_2B_MONITOR_MEM_MASKr,
    BRDC_DCH_ECC_ERR_1B_MONITOR_MEM_MASKr,
    BRDC_DCH_ECC_ERR_2B_MONITOR_MEM_MASKr,
    BRDC_DCL_ECC_ERR_1B_MONITOR_MEM_MASKr,
    BRDC_DCL_ECC_ERR_2B_MONITOR_MEM_MASKr,
    BRDC_DCM_ECC_ERR_1B_MONITOR_MEM_MASKr,
    BRDC_DCM_ECC_ERR_2B_MONITOR_MEM_MASKr,
    ECI_ECC_ERR_1B_MONITOR_MEM_MASKr,
    ECI_ECC_ERR_2B_MONITOR_MEM_MASKr,
    BRDC_FSRD_ECC_ERR_1B_MONITOR_MEM_MASKr,
    BRDC_FSRD_ECC_ERR_2B_MONITOR_MEM_MASKr,
    BRDC_FSRD_WC_UC_MEM_MASK_BITMAPr,
    RTP_ECC_ERR_1B_MONITOR_MEM_MASKr,
    RTP_ECC_ERR_2B_MONITOR_MEM_MASKr,
    RTP_DRHPA_ECC_ERR_2B_MONITOR_MEM_MASKr,
    RTP_DRHPB_ECC_ERR_2B_MONITOR_MEM_MASKr,
    RTP_DRHPA_ECC_ERR_1B_MONITOR_MEM_MASKr,
    RTP_DRHPB_ECC_ERR_1B_MONITOR_MEM_MASKr,
    INVALIDr
};

/*************
 * FUNCTIONS *
 *************/

int 
soc_ramon_interrupt_all_enable_set(int unit, int enable)
{
    DNXC_INIT_FUNC_DEFS;

    if (enable) {
        soc_cmicm_intr3_enable(unit, 0xffffffff);
        soc_cmicm_intr4_enable(unit, 0xffffffff);
        soc_cmicm_intr5_enable(unit, 0xffffffff);
    } else {
        soc_cmicm_intr3_disable(unit, 0xffffffff);
        soc_cmicm_intr4_disable(unit, 0xffffffff);
        soc_cmicm_intr5_disable(unit, 0xffffffff);
    }

    DNXC_FUNC_RETURN;
}

int 
soc_ramon_interrupt_all_enable_get(int unit, int *enable)
{
    int rc;
    int mask;
    DNXC_INIT_FUNC_DEFS;

    rc = soc_interrupt_is_all_mask(unit, &mask);
    DNXC_IF_ERR_EXIT(rc);

    *enable = mask ? 0 : 1;
exit:
    DNXC_FUNC_RETURN;
}

int soc_ramon_ser_init(int unit)
{
    int idx, array_index, array_index_max = 1;
    int rc;
    soc_reg_above_64_val_t above_64;

    DNXC_INIT_FUNC_DEFS;

    /* unmask SER monitor registers*/
    SOC_REG_ABOVE_64_ALLONES(above_64);
    for(idx=0; soc_ramon_interrupt_monitor_mem_reg[idx] != INVALIDr; idx++) {

        if (soc_ramon_interrupt_monitor_mem_reg[idx] == BRDC_FSRD_ECC_ERR_1B_MONITOR_MEM_MASKr ||
            soc_ramon_interrupt_monitor_mem_reg[idx] == BRDC_FSRD_ECC_ERR_2B_MONITOR_MEM_MASKr ||
            soc_ramon_interrupt_monitor_mem_reg[idx] == BRDC_FSRD_WC_UC_MEM_MASK_BITMAPr)
        {
            continue;
        }

        if (SOC_REG_IS_ARRAY(unit, soc_ramon_interrupt_monitor_mem_reg[idx])) {
             array_index_max = SOC_REG_NUMELS(unit, soc_ramon_interrupt_monitor_mem_reg[idx]);
        }

        for (array_index = 0; array_index < array_index_max; array_index++) {
            rc = soc_reg_above_64_set(unit, soc_ramon_interrupt_monitor_mem_reg[idx], 0, array_index, above_64);
            DNXC_IF_ERR_EXIT(rc);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int soc_ramon_interrupts_disable(int unit)
{
    int rc;
    int i, copy_no;
    soc_interrupt_db_t* interrupts;
    soc_block_types_t  block;
    soc_reg_t reg;
    int blk;
    int nof_interrupts;
    soc_reg_above_64_val_t data;

    DNXC_INIT_FUNC_DEFS;

    rc = soc_ramon_nof_interrupts(unit, &nof_interrupts);
    DNXC_IF_ERR_EXIT(rc);

    if (!SAL_BOOT_NO_INTERRUPTS) {

        /* disable all block interrupts */
        SOC_REG_ABOVE_64_CLEAR(data);
        for (i = 0; i < SOC_RAMON_NOF_BLK; i++) {
            if (SOC_CONTROL(unit)->interrupts_info->interrupt_tree_info[i].int_mask_reg != INVALIDr) {
                rc = soc_reg_above_64_set(unit, SOC_CONTROL(unit)->interrupts_info->interrupt_tree_info[i].int_mask_reg, 0,  0, data); 
                DNXC_IF_ERR_EXIT(rc);
            }
        }
        /* disable all interrupt vectors */
        interrupts = SOC_CONTROL(unit)->interrupts_info->interrupt_db_info;
        for (i=0 ; i < nof_interrupts; i++) { 
            reg = interrupts[i].reg;
            /* Unsupported interrupts */
            if (!SOC_REG_IS_VALID(unit, reg))
            {
               continue;
            }
            block = SOC_REG_INFO(unit, reg).block;
            SOC_REG_ABOVE_64_CLEAR(data);
            SOC_BLOCKS_ITER(unit, blk, block) {
                copy_no = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                if (interrupts[i].vector_info) {
                    rc = soc_reg_above_64_set(unit, interrupts[i].vector_info->int_mask_reg, copy_no,  0, data);
                }
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

int soc_ramon_interrupts_deinit(int unit)
{
    DNXC_INIT_FUNC_DEFS;

    /* mask all interrupts in cmic (even in WB mode). This Masking update WB DB, althoght we dont use WB DB. */ 
    soc_ramon_interrupt_all_enable_set(unit, 0);

    if (soc_property_get(unit, spn_POLLED_IRQ_MODE, 1)) {
        if (soc_ipoll_disconnect(unit) < 0) {
            LOG_ERROR(BSL_LS_SOC_INIT,
                      (BSL_META_U(unit,
                                  "error disconnecting polled interrupt mode\n")));
        }
    } else {
        /* unit # is ISR arg */
        if (soc_cm_interrupt_disconnect(unit) < 0) {
            LOG_ERROR(BSL_LS_SOC_INIT,
                      (BSL_META_U(unit,
                                  "could not disconnect interrupt line\n")));
        }
    }
    
    if (dnxc_intr_handler_deinit(unit) < 0) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("error at interrupt db deinitialization")));
    }
           
exit:               
    DNXC_FUNC_RETURN;
}

int soc_ramon_interrupts_init(int unit)
{
    int cmc;
    int rc;

    DNXC_INIT_FUNC_DEFS;

    cmc = SOC_PCI_CMC(unit);
     
    /* mask all interrupts in cmic (even in WB mode). This Masking update WB DB, althoght we dont use WB DB. */ 
    soc_ramon_interrupt_all_enable_set(unit, 0);

    if (!SAL_BOOT_NO_INTERRUPTS) {

        /* Init interrupt DB */
        if (dnxc_intr_handler_init(unit) < 0) {
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("error initializing polled interrupt mode")));
        }
     
        /* init warmboot database */     
        if (!SOC_WARM_BOOT(unit))
        {
            rc = SOC_DNXF_WARM_BOOT_ARR_MEMSET(unit, INTR_STORM_TIMED_PERIOD, 0);
            DNXC_IF_ERR_EXIT(rc);
            rc = SOC_DNXF_WARM_BOOT_ARR_MEMSET(unit, INTR_STORM_TIMED_COUNT, 0);
            DNXC_IF_ERR_EXIT(rc);
            rc = SOC_DNXF_WARM_BOOT_ARR_MEMSET(unit, INTR_FLAGS, 0);
            DNXC_IF_ERR_EXIT(rc);
        }
             
        /* connect interrupts / start interrupt thread */
        if (soc_property_get(unit, spn_POLLED_IRQ_MODE, 1)) {
            if (soc_ipoll_connect(unit, soc_cmicm_intr, INT_TO_PTR(unit)) < 0) {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("error initializing polled interrupt mode")));
            }
            SOC_CONTROL(unit)->soc_flags |= SOC_F_POLLED;
        } else {
            if (soc_cm_interrupt_connect(unit, soc_cmicm_intr, INT_TO_PTR(unit)) < 0) {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("could not connect interrupt line")));
            }
        }    

        if (!SOC_WARM_BOOT(unit)) {
            uint32 rval;
            uint32 enable_msi = 1;

            if (soc_feature(unit, soc_feature_iproc) &&
                 (soc_cm_get_bus_type(unit) & SOC_DEV_BUS_MSI) == 0) {
               /*
                           * For iProc designs, MSI must be turned off in the CMIC
                           * to prevent invalid MSI messages from corrupting host
                           * CPU memory.
                           */
                enable_msi = 0;
            }

            rval = soc_pci_read(unit, CMIC_CMCx_PCIE_MISCEL_OFFSET(cmc));
            soc_reg_field_set(unit, CMIC_CMC0_PCIE_MISCELr, &rval, ENABLE_MSIf, enable_msi);
            soc_pci_write(unit, CMIC_CMCx_PCIE_MISCEL_OFFSET(cmc), rval);
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME
