/*
 * $Id: dnxf_intr.c,v 1.87 Broadcom SDK $

 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

/*************
 * INCLUDES  *
 *************/
#include <soc/error.h>
#include <soc/ipoll.h>
#include <soc/drv.h>

#include <soc/dnxc/legacy/error.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/fe1600/fe1600_interrupts.h> 

/*************
 * DEFINES   *
 *************/
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INTR

/*************
 * FUNCTIONS *
 *************/
int soc_ramon_fe1600_interrupt_deinit(int unit)
{
    DNXC_INIT_FUNC_DEFS;
                           
    if(!SAL_BOOT_NO_INTERRUPTS) {
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
    }



    if(!SAL_BOOT_NO_INTERRUPTS) 
    {
        ramon_fe1600_interrupts_dnxf_control_data_deinit(unit);
        
        ramon_fe1600_interrupts_array_deinit(unit);       
    }

    DNXC_FUNC_RETURN;
}

int soc_ramon_fe1600_interrupt_init(int unit) 
{
    int rv;
    soc_control_t* soc;

    DNXC_INIT_FUNC_DEFS;

    soc = SOC_CONTROL(unit);

    if(!SAL_BOOT_NO_INTERRUPTS) {
        rv = ramon_fe1600_interrupts_array_init(unit);
        DNXC_IF_ERR_EXIT(rv);

        rv = ramon_fe1600_interrupts_dnxf_control_data_init(unit);
        DNXC_IF_ERR_EXIT(rv);
    }
 

    /* take from HW the interrupts mask */
    /* note: there is al so high level of cmic mask that dosn't saved in wormboot. */

    soc_intr_block_lo_disable(unit, 0xFFFFFFFF);
    soc_intr_block_hi_disable(unit, 0xFFFFFFFF);

    if(!SAL_BOOT_NO_INTERRUPTS) {
        if (soc_property_get(unit, spn_POLLED_IRQ_MODE, 1)) {
            if (soc_ipoll_connect(unit, dnxf_local_soc_intr, INT_TO_PTR(unit)) < 0) {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("error initializing polled interrupt mode")));
            }
            soc->soc_flags |= SOC_F_POLLED;
        } else {
            /* unit # is ISR arg */
            if (soc_cm_interrupt_connect(unit, dnxf_local_soc_intr, INT_TO_PTR(unit)) < 0) {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("could not connect interrupt line")));
            }
        }
    }
exit:
    if (DNXC_FUNC_ERROR)
    {
        soc_ramon_fe1600_interrupt_deinit(unit);
    }
    DNXC_FUNC_RETURN;
}

