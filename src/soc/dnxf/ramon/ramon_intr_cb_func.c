/*
 * $Id: ramon_intr_cb_func.c, v1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Purpose:    Implement CallBacks function for RAMON interrupts.
 */
 
/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/drv.h>
#include <soc/dnxc/legacy/dnxc_intr_handler.h>

#include <soc/cm.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/intr.h>

#include <soc/dnxf/ramon/ramon_intr.h>
#include <soc/dnxf/ramon/ramon_intr_cb_func.h>

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
/*
 * Generic None handles - for CB without specific handling
 */


int ramon_intr_handle_generic_none(int unit, int block_instance, ramon_interrupt_type en_ramon_interrupt,char *msg)
{
    return 0;
}

int ramon_intr_recurring_action_handle_generic_none(int unit, int block_instance, ramon_interrupt_type en_ramon_interrupt, char *msg)
{
    return 0;
}

void ramon_interrupt_cb_init(int unit)
{
}

#undef _ERR_MSG_MODULE_NAME
