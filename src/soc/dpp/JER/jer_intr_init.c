
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
#include <soc/dcmn/dcmn_dev_feature_manager.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/drv.h>

#ifdef BCM_JERICHO_PLUS_SUPPORT
#include <soc/dpp/JERP/jerp_intr.h>
#endif
#ifdef BCM_QUX_SUPPORT
#include <soc/dpp/QUX/qux_intr.h>
#endif
#ifdef BCM_DPP_SUPPORT
#include <soc/dpp/mbcm.h>
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

/*************
 * FUNCTIONS *
 *************/

int
soc_jer_interrupts_disable(int unit)
{
    int rc;
    int i, copy_no;
    soc_interrupt_db_t* interrupts;
    soc_block_types_t  block;
    soc_reg_t reg;
    int blk, blktype;
    int nof_interrupts;
    soc_reg_above_64_val_t data;

    SOCDNX_INIT_FUNC_DEFS;

#ifdef BCM_QUX_SUPPORT
    if (SOC_IS_QUX(unit)) {
        rc = soc_qux_nof_interrupts(unit, &nof_interrupts);
    } else
#endif
#ifdef BCM_JERICHO_PLUS_SUPPORT
    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        rc = soc_jerp_nof_interrupts(unit, &nof_interrupts);
    } else
#endif
    {
        rc = MBCM_DPP_DRIVER_CALL(unit,mbcm_dpp_nof_interrupts,(unit, &nof_interrupts));
    }

    SOCDNX_IF_ERR_EXIT(rc);

    if (!SAL_BOOT_NO_INTERRUPTS) {

        /* disable all block interrupts */
        SOC_REG_ABOVE_64_CLEAR(data);
        for (blk = 0; ((SOC_BLOCK_TYPE(unit, blk) >= 0) && (SOC_BLOCK_TYPE(unit, blk) != SOC_BLK_EMPTY)); blk++) {
            reg = SOC_CONTROL(unit)->interrupts_info->interrupt_tree_info[SOC_BLOCK_INFO(unit, blk).cmic].int_mask_reg;

            if (dcmn_device_block_for_feature(unit,DCMN_NO_EXT_RAM_FEATURE)){
                blktype = SOC_BLOCK_TYPE(unit, blk);
                if (blktype==SOC_BLK_BRDC_DRC || blktype==SOC_BLK_DRCA || blktype==SOC_BLK_DRCB || blktype==SOC_BLK_DRCC || blktype==SOC_BLK_DRCD || \
                    blktype==SOC_BLK_DRCE || blktype==SOC_BLK_DRCF || blktype==SOC_BLK_DRCG || blktype==SOC_BLK_DRCH || blktype==SOC_BLK_DRCBROADCAST) {
                    continue;
                }
            }

            if (dcmn_device_block_for_feature(unit,DCMN_ONE_EXT_RAM_FEATURE)){
                blktype = SOC_BLOCK_TYPE(unit, blk);
                if (blktype==SOC_BLK_DRCB || blktype==SOC_BLK_DRCC) {
                    continue;
                }
            }

            if (SOC_REG_IS_VALID(unit, reg)) {
                copy_no = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                rc = soc_reg_above_64_set(unit, reg, copy_no,  0, data); 
                SOCDNX_IF_ERR_EXIT(rc);
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
                if (dcmn_device_block_for_feature(unit,DCMN_ONE_EXT_RAM_FEATURE)){
                blktype = SOC_BLOCK_TYPE(unit, blk);
                    if (blktype==SOC_BLK_DRCB || blktype==SOC_BLK_DRCC) {
                        continue;
                    }
                }
                copy_no = (SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_CLP || SOC_BLOCK_TYPE(unit, blk) == SOC_BLK_XLP) ? SOC_BLOCK_PORT(unit, blk) : SOC_BLOCK_NUMBER(unit, blk);
                if (interrupts[i].vector_info) {
                    rc = soc_reg_above_64_set(unit, interrupts[i].vector_info->int_mask_reg, copy_no,  0, data);
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_interrupts_deinit(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /* mask all interrupts in cmic (even in WB mode). This Masking update WB DB, althoght we dont use WB DB. */ 
    soc_cmicm_intr3_disable(unit, 0xffffffff);
    soc_cmicm_intr4_disable(unit, 0xffffffff);
    soc_cmicm_intr5_disable(unit, 0xffffffff);
    soc_cmicm_intr6_disable(unit, 0xffffffff);

    if (soc_ipoll_disconnect(unit) < 0) {
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "error disconnecting polled interrupt mode\n")));
    }
    SOC_CONTROL(unit)->soc_flags &= ~SOC_F_POLLED;
    /* unit # is ISR arg */
    if (soc_cm_interrupt_disconnect(unit) < 0) {
        LOG_ERROR(BSL_LS_SOC_INIT,
                  (BSL_META_U(unit,
                              "could not disconnect interrupt line\n")));
    }

    
    if (dcmn_intr_handler_deinit(unit) < 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("error at interrupt db deinitialization")));
    }
           
exit:               
    SOCDNX_FUNC_RETURN;
}



void  soc_jer_cmic_interrupts_disable(int unit)
{
    /* mask all interrupts in cmic (even in WB mode). This Masking update WB DB, althoght we dont use WB DB. */ 
    soc_cmicm_intr3_disable(unit, 0xffffffff);
    soc_cmicm_intr4_disable(unit, 0xffffffff);
    soc_cmicm_intr5_disable(unit, 0xffffffff);
    soc_cmicm_intr6_disable(unit, 0xffffffff);

}


int soc_jer_interrupts_init(int unit)
{
    int cmc;
    int polling_mode;

    SOCDNX_INIT_FUNC_DEFS;

    cmc = SOC_PCI_CMC(unit);

    if (!SAL_BOOT_NO_INTERRUPTS) {

        /* Init interrupt DB */
        if (dcmn_intr_handler_init(unit) < 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("error initializing polled interrupt mode")));
        }
     
        polling_mode = soc_property_get(unit, spn_POLLED_IRQ_MODE, 1);     
#if defined(__DUNE_GTO_BCM_CPU__)
        if ((SOC_IS_JERICHO(unit)) && (polling_mode == 0)) {
            LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "** GTO platform does not support irq interrupt mode for 88X7X. Changing configuration to irq polling mode **\n")));
            polling_mode = 1;
        }
#endif
        /* connect interrupts / start interrupt thread */
        if (polling_mode) {
            if (soc_ipoll_connect(unit, soc_cmicm_intr, INT_TO_PTR(unit)) < 0) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("error initializing polled interrupt mode")));
            }
            SOC_CONTROL(unit)->soc_flags |= SOC_F_POLLED;
        } else {
            if (soc_cm_interrupt_connect(unit, soc_cmicm_intr, INT_TO_PTR(unit)) < 0) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("could not connect interrupt line")));
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
    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME
