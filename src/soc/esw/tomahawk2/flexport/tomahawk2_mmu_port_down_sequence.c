/*
* $Id: $
* $Copyright: (c) 2016 Broadcom.
* Broadcom Proprietary and Confidential. All rights reserved.$
*
* $File:  tomahawk2_mmu_port_down_sequence.c
*/


#include <shared/bsl.h>
#include <soc/drv.h>
#include <soc/defs.h>
#include <soc/mem.h>
#include <soc/esw/port.h>
#include <soc/tdm/core/tdm_top.h>


#if defined(BCM_TOMAHAWK2_SUPPORT)
#include <soc/tomahawk2.h>
#include <soc/tomahawk.h>
#include <soc/tomahawk2_tdm.h>
/*! @file tomahawk2_mmu_port_down_sequence.c
 *  @brief
 */

#include <soc/flexport/tomahawk2_flexport.h>


/*! @fn int soc_tomahawk2_mmu_port_down_sequence(int unit,
 *              soc_port_schedule_state_t *port_schedule_state_t)
 *  @param unit Device number
 *  @param *port_schedule_state_t Port Schedule_State struct
 *  @brief API to update all MMU related functionalities during flexport
 *         port down operation.
 */
int
soc_tomahawk2_flex_mmu_port_down(
    int unit, soc_port_schedule_state_t *port_schedule_state_t)
{
    int port;
    uint64 temp64;

    for (port=0; port<port_schedule_state_t->nport; port++) {
        if (port_schedule_state_t->resource[port].physical_port == -1) {
            COMPILER_64_SET(temp64, 0, TH2_MMU_FLUSH_ON);
            soc_tomahawk2_mmu_vbs_port_flush(
                unit, &port_schedule_state_t->resource[port], temp64);
            soc_tomahawk2_mmu_rqe_port_flush(
                unit, &port_schedule_state_t->resource[port], temp64);
            soc_tomahawk2_mmu_mtro_port_flush(
                unit, &port_schedule_state_t->resource[port], temp64);
            sal_usleep(TH2_CT_FIFO_WAIT +
                       (SAL_BOOT_QUICKTURN ? 1 : 0) * EMULATION_FACTOR);
        }
    }

    return SOC_E_NONE;
}


/*! @fn int soc_tomahawk2_mmu_reconfig_tdm(int unit,
 *              soc_port_schedule_state_t *port_schedule_state_t) {
 *  @param unit Device number
 *  @param *port_schedule_state_t Port Schedule_State struct
 *  @brief API to reconfigure TDM calendars for MMU.
 */
int
soc_tomahawk2_flex_mmu_reconfigure_phase1(
    int unit, soc_port_schedule_state_t *port_schedule_state_t)
{
    soc_tomahawk2_tdm_flexport_mmu(unit, port_schedule_state_t);
    _soc_tomahawk2_tdm_mmu_calendar_set(unit, port_schedule_state_t);
    _soc_tomahawk2_tdm_mmu_hsp_set(unit, port_schedule_state_t);
    return SOC_E_NONE;
}


#endif /* BCM_TOMAHAWK2_SUPPORT */
