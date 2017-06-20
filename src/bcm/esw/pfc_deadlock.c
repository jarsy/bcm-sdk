/*
 * $Id: pfc_deadlock.c $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * PFC Deadlock Detection & Recovery
 */

#include <shared/bsl.h>

#include <soc/defs.h>
#include <sal/core/libc.h>
#include <sal/core/dpc.h>
#include <sal/core/time.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/tomahawk.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm_int/esw/mbcm.h>
#include <bcm_int/esw/stack.h>
#include <bcm_int/esw/cosq.h>
#include <bcm_int/esw/pfc_deadlock.h>
#include <bcm_int/esw/virtual.h>
#include <bcm_int/esw/tomahawk.h>

#include <bcm_int/esw_dispatch.h>

#ifdef BCM_WARM_BOOT_SUPPORT
#include <bcm_int/esw/switch.h>
#endif /* BCM_WARM_BOOT_SUPPORT */

#if defined(BCM_TOMAHAWK_SUPPORT)
#define _MMU_PORT_STRIDE SOC_TH_MMU_PORT_STRIDE
#else
#define _MMU_PORT_STRIDE 64
#endif

_bcm_pfc_deadlock_control_t *_bcm_pfc_deadlock_control[BCM_MAX_NUM_UNITS];
_bcm_pfc_deadlock_cb_t *_bcm_pfc_deadlock_cb[BCM_MAX_NUM_UNITS];

STATIC int
_bcm_pfc_deadlock_ignore_pfc_xoff_clear(int unit, int cos, bcm_port_t port)
{
    int priority = 0;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    uint32 rval;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    if (port >= MAX_PORT(unit)) {
        return BCM_E_PARAM; /* Process for valid ports */
    }

    priority = pfc_deadlock_control->pfc_cos2pri[cos];
    /* For that port, set ignore_pfc_xoff = 0 (per port reg) */
    rval = 0;
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, hw_res->port_config, port, 0, &rval));
    rval &= ~(1 << priority);
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, hw_res->port_config, port, 0, rval));

    return BCM_E_NONE;
}
STATIC int
 _bcm_pfc_deadlock_ignore_pfc_xoff_gen(int unit, int priority,
                                        bcm_port_t port, uint32 *rval32)
{

#ifdef BCM_TOMAHAWK_SUPPORT
    if (SOC_IS_TOMAHAWKX(unit)) {
        BCM_IF_ERROR_RETURN(
            bcm_th_pfc_deadlock_ignore_pfc_xoff_gen(unit, priority,
                                              port, rval32));
    } else
#endif
    {
        return BCM_E_UNAVAIL;
    }

    return BCM_E_NONE;
}

/* Routine to Reset the recovery state of a port and config the port back in
 * original state
 */
STATIC int
_bcm_pfc_deadlock_recovery_end(int unit, int cos, bcm_port_t port)
{
    soc_info_t *si;
    int phy_port, mmu_port, pipe_mmu_port, pipe, priority = 0;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    uint64 rval64, temp_rval64, temp_mask64, temp_en64;
    uint32 rval, uc_cos_bmp;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    _bcm_pfc_deadlock_cb_t *pfc_deadlock_cb = NULL;

    si = &SOC_INFO(unit);
    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    if (port >= MAX_PORT(unit)) {
        return BCM_E_PARAM; /* Process for valid ports */
    }
    phy_port = si->port_l2p_mapping[port];
    mmu_port = si->port_p2m_mapping[phy_port];
    pipe_mmu_port = mmu_port % _MMU_PORT_STRIDE;

    SOC_IF_ERROR_RETURN(soc_port_pipe_get(unit, port, &pipe));
    priority = pfc_deadlock_control->pfc_cos2pri[cos];
    pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);

    COMPILER_64_ZERO(temp_en64);
    COMPILER_64_ZERO(temp_mask64);
    COMPILER_64_ZERO(temp_rval64);

    if (pipe_mmu_port < 32) {
        COMPILER_64_SET(temp_rval64, 0, (1 << pipe_mmu_port));
    } else {
        COMPILER_64_SET(temp_rval64, (1 << (pipe_mmu_port - 32)), 0);
    }
    COMPILER_64_OR(temp_mask64, temp_rval64);
    COMPILER_64_OR(temp_en64, temp_rval64);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "PFC Deadlock Recovery ends: Prio %d port=%d\n"),
                         priority, port));

    /* 3.a: For that port, set ignore_pfc_xoff = 0 (per port reg) */
    rval = 0;
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, hw_res->port_config, port, 0, &rval));

    rval = _bcm_pfc_deadlock_ignore_pfc_xoff_gen(unit, priority,
                                                 port, &uc_cos_bmp);
    if(rval != BCM_E_NONE) {
        rval = 0;
        rval &= ~(1 << priority);
    } else {
        rval = 0;
        rval &= ~uc_cos_bmp;
    }
    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, hw_res->port_config, port, 0, rval));

     /* 3.b: Mask the Intr DD_TIMER_MASK_A|_B by setting 0 (pbmp) */
    COMPILER_64_ZERO(rval64);

    SOC_IF_ERROR_RETURN(
        soc_reg_get(unit, hw_res->timer_mask[cos], pipe,
                    0, &rval64));

    COMPILER_64_NOT(temp_mask64);
    COMPILER_64_AND(rval64, temp_mask64);
    SOC_IF_ERROR_RETURN(
        soc_reg_set(unit, hw_res->timer_mask[cos], pipe, 0, rval64));

    /* 3.c: Turn timer off DD_TIMER_ENABLE_A|_B by setting 1 (pbmp) */
    COMPILER_64_ZERO(rval64);

    SOC_IF_ERROR_RETURN(
        soc_reg_get(unit, hw_res->timer_en[cos], pipe,
                    0, &rval64));

    COMPILER_64_OR(rval64, temp_en64);
    SOC_IF_ERROR_RETURN(
        soc_reg_set(unit, hw_res->timer_en[cos], pipe, 0, rval64));

    pfc_deadlock_cb = _BCM_UNIT_PFC_DEADLOCK_CB(unit);
    if(pfc_deadlock_cb->pfc_deadlock_cb) {
        pfc_deadlock_cb->pfc_deadlock_cb(unit, port, priority,
                              bcmCosqPfcDeadlockRecoveryEventEnd,
                              pfc_deadlock_cb->pfc_deadlock_userdata);
    }


    SOC_PBMP_PORT_REMOVE(pfc_deadlock_pri_config->deadlock_ports, port);

    return BCM_E_NONE;
}

/* Routine to update the recovery status of ports which are in Recovery state
 */
STATIC int
_bcm_pfc_deadlock_recovery_update(int unit, int cos)
{
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    bcm_port_t port;
    int priority = 0;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    priority = pfc_deadlock_control->pfc_cos2pri[cos];
    pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);

    BCM_PBMP_ITER(pfc_deadlock_pri_config->deadlock_ports, port) {
        if (port >= MAX_PORT(unit)) {
            break; /* Process for valid ports */
        }
        /*
         * COVERITY
         *
         * Max Port num will be updated dynamically per device.
         */
        /* coverity[overrun-local : FALSE] */
        if (pfc_deadlock_pri_config->port_recovery_count[port]) {
            pfc_deadlock_pri_config->port_recovery_count[port]--;
        } else {
            BCM_IF_ERROR_RETURN(
                _bcm_pfc_deadlock_recovery_end(unit, cos, port));
        }
    }
    return BCM_E_NONE;
}

/* Routine to begin recovery when a Port is deaclared to be in Deadlock by
 * Hardware
 */
STATIC int
_bcm_pfc_deadlock_recovery_begin(int unit, int cos, int pipe, int pipe_mmu_port)
{
    soc_info_t *si;
    uint32 rval,uc_cos_bmp;
    int phy_port, port, mmu_port, priority = 0;
    uint64 rval64, temp_rval64, temp_mask64, temp_en64;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    _bcm_pfc_deadlock_cb_t *pfc_deadlock_cb = NULL;

    si = &SOC_INFO(unit);
    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    /* convert local mmu_port(per pipe) to global mmu_port */
    mmu_port = pipe_mmu_port + (pipe * _MMU_PORT_STRIDE);
    phy_port = si->port_m2p_mapping[mmu_port];
    port = si->port_p2l_mapping[phy_port];
    COMPILER_64_ZERO(temp_en64);
    COMPILER_64_ZERO(temp_mask64);
    COMPILER_64_ZERO(temp_rval64);

    if (port >= MAX_PORT(unit)) {
        return BCM_E_PARAM; /* Process for valid ports */
    }
    if (pipe_mmu_port < 32) {
        COMPILER_64_SET(temp_rval64, 0, (1 << pipe_mmu_port));
    } else {
        COMPILER_64_SET(temp_rval64, (1 << (pipe_mmu_port - 32)), 0);
    }
    COMPILER_64_OR(temp_mask64, temp_rval64);
    COMPILER_64_OR(temp_en64, temp_rval64);

    LOG_INFO(BSL_LS_BCM_COSQ,
             (BSL_META_U(unit,
                         "PFC Deadlock Detected: Cos %d port=%d\n"),
                         cos, port));

    /* 2.a: Mask the Intr DD_TIMER_MASK_A|_B by setting 1 (pbmp) */
    COMPILER_64_ZERO(rval64);

    SOC_IF_ERROR_RETURN(
        soc_reg_get(unit, hw_res->timer_mask[cos], pipe,
                    0, &rval64));

    COMPILER_64_OR(rval64, temp_mask64);
    SOC_IF_ERROR_RETURN(
        soc_reg_set(unit, hw_res->timer_mask[cos], pipe,
                    0, rval64));

    /* 2.b: Turn timer off DD_TIMER_ENABLE_A|_B by setting 0 (pbmp) */
    COMPILER_64_ZERO(rval64);

    SOC_IF_ERROR_RETURN(
        soc_reg_get(unit, hw_res->timer_en[cos], pipe,
                    0, &rval64));

    COMPILER_64_NOT(temp_en64);
    COMPILER_64_AND(rval64, temp_en64);
    SOC_IF_ERROR_RETURN(
        soc_reg_set(unit, hw_res->timer_en[cos], pipe,
                    0, rval64));

    /* 2.c: For that port, set ignore_pfc_xoff = 1 (per port reg) */
    rval = 0;
    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, hw_res->port_config, port, 0, &rval));
    priority = pfc_deadlock_control->pfc_cos2pri[cos];

    rval = _bcm_pfc_deadlock_ignore_pfc_xoff_gen(unit, priority,
                                                 port, &uc_cos_bmp);
    if(rval != BCM_E_NONE) {
        rval = 0;
        rval |= (1 << priority);
    } else {
        rval = 0;
        rval |= uc_cos_bmp;
    }

    SOC_IF_ERROR_RETURN(
        soc_reg32_set(unit, hw_res->port_config, port, 0, rval));
  
    pfc_deadlock_cb = _BCM_UNIT_PFC_DEADLOCK_CB(unit);    
    if(pfc_deadlock_cb->pfc_deadlock_cb) {
        pfc_deadlock_cb->pfc_deadlock_cb(unit, port, priority,
                              bcmCosqPfcDeadlockRecoveryEventBegin,
                              pfc_deadlock_cb->pfc_deadlock_userdata);
    }

    pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);

    /* Recovery count calculated from Configured Recovery timer.
     * Note: recovery_timer is in msecs and cb_interval is in usecs.
     */
    pfc_deadlock_pri_config->port_recovery_count[port] = 1 +
                        ((pfc_deadlock_pri_config->recovery_timer * 1000) /
                         _BCM_PFC_DEADLOCK_CB_INTERVAL(0));
    BCM_PBMP_PORT_ADD(pfc_deadlock_pri_config->deadlock_ports, port);
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_monitor
 * Purpose:
 *     1) Monitor the hardware for Deadlock status
 *     2) Start Recovery process for the Deadlocked port/queue
 *     3) Reset Port back to original state on expiry of recovery phase
 * Parameters:
 *     unit             - (IN) unit number
 * Returns:
 *     BCM_E_XXX
 * Notes:
 *     Step 1 - Monitor (Deadlock Detection phase):
 *          Sw polls for Deadlock detection intr set for port
 *          DD_TIMER_STATUS_A|_B (pbmp)
 *
 *     Step 2: Recovery Begin phase
 *          2.a: Mask the Intr DD_TIMER_MASK_A|_B by setting 1 (pbmp)
 *          2.b: Turn timer off DD_TIMER_ENABLE_A|_B by setting 0 (pbmp)
 *          2.c: For that port, set ignore_pfc_xoff = 1 (per port reg)
 *          2.4: Start recovery timer
 *      Step 3: Recovery End phase (On expiry of recovery timer)
 *          3.a: Reset ignore_xoff; set ignore_pfc_xoff = 0
 *          3.b: UnMask the Intr DD_TIMER_MASK_A|_B by setting 0 (pbmp)
 *          3.c: Turn timer off DD_TIMER_ENABLE_A|_B by setting 1 (pbmp)
 */
STATIC int
_bcm_pfc_deadlock_monitor(int unit)
{
    int i = 0, pipe, mmu_port, priority = 0;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    uint64 status64, mask64;

    COMPILER_64_ZERO(status64);
    COMPILER_64_ZERO(mask64);
    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    for (i = 0; i < pfc_deadlock_control->pfc_deadlock_cos_max; i++) {
        /* Check if index is in Use */
        if (pfc_deadlock_control->hw_cos_idx_inuse[i] == TRUE) {
            /* Check Hardware if New ports have been declared to be in
             * Deadlock condition
             */
            for (pipe = 0; pipe < NUM_PIPE(unit); pipe++) {
                SOC_IF_ERROR_RETURN(
                    soc_reg_get(unit, hw_res->timer_status[i], pipe,
                                0, &status64));
                SOC_IF_ERROR_RETURN(
                    soc_reg_get(unit, hw_res->timer_mask[i], pipe,
                                0, &mask64));
                /* Mask - bit 0 indicates enabled for reporting */
                COMPILER_64_NOT(mask64);
                COMPILER_64_AND(status64, mask64);

                if (COMPILER_64_IS_ZERO(status64) == 0) {
                    /* New ports declared to be in Deadlock status */
                    for (mmu_port = 0; mmu_port < 34; mmu_port++) {
                        if (mmu_port < 32) {
                            if ((COMPILER_64_LO(status64) &
                                (1 << mmu_port)) == FALSE) {
                                continue;
                            }
                        } else {
                            if ((COMPILER_64_HI(status64) &
                                (1 << (mmu_port - 32))) == FALSE) {
                                continue;
                            }
                        }
                        BCM_IF_ERROR_RETURN(
                            _bcm_pfc_deadlock_recovery_begin(unit, i,
                                                             pipe, mmu_port));
                    }
                }
            }

            /* Update the count for ports already in Deadlock state and Reset
             * those ports where recovery timer has expired.
             */
            priority = pfc_deadlock_control->pfc_cos2pri[i];
            pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);
            /* Updates required for Enabled COS */
            if (SOC_PBMP_NOT_NULL(pfc_deadlock_pri_config->deadlock_ports)) {
                BCM_IF_ERROR_RETURN(_bcm_pfc_deadlock_recovery_update(unit, i));
            }
        }
    }
    return BCM_E_NONE;
}

/* Callback routine registered with DPC framework for,
 *  1) Monitoring Deadlock status
 *  2) Start/Stop Recovery process
 */
STATIC void
pfc_deadlock_cb(void* owner, void* time_as_ptr, void *cb_time_as_ptr,
                void *unused_2, void* unused_3)
{
    int callback_time = _BCM_PFC_DEADLOCK_CB_INTERVAL(0);

    sal_dpc_cancel(INT_TO_PTR(&pfc_deadlock_cb));
    _bcm_pfc_deadlock_monitor(0);

    if (_BCM_PFC_DEADLOCK_CB_ENABLED(0)) {
        sal_dpc_time(callback_time, &pfc_deadlock_cb, 0,
                     INT_TO_PTR(sal_time_usecs()), INT_TO_PTR(callback_time),
                     0, 0);
    }
    _BCM_PFC_DEADLOCK_CB_COUNT(0)++;
}

STATIC int
_bcm_pfc_deadlock_hw_cos_index_get(int unit, bcm_cos_t priority,
                                   int *hw_cos_index)
{
    int i, cos_val;
    uint32 rval = 0;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, hw_res->chip_config[0], REG_PORT_ANY, 0, &rval));

    for (i = 0; i < pfc_deadlock_control->pfc_deadlock_cos_max; i++) {
        cos_val = soc_reg_field_get(unit, hw_res->chip_config[0], rval,
                                    hw_res->cos_field[i]);
        if ((cos_val == priority) &&
            (pfc_deadlock_control->hw_cos_idx_inuse[i] == TRUE)) {
            *hw_cos_index = i;
            return BCM_E_NONE;
        }
    }
    return BCM_E_NOT_FOUND;
}

STATIC int
_bcm_pfc_deadlock_hw_cos_index_set(int unit, bcm_cos_t priority,
                                   int *hw_cos_index)
{
    int i, first_avail = -1, rv = BCM_E_NONE;
    int temp_hw_index = -1;
    uint32 rval = 0;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_config = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    rv = _bcm_pfc_deadlock_hw_cos_index_get(unit, priority, &temp_hw_index);
    if (rv != BCM_E_NONE) {
        if (rv != BCM_E_NOT_FOUND) {
            return rv;
        }
    }

    if (temp_hw_index != -1) {
        /* Config for this priority exists already. Donot reprogram */
        *hw_cos_index = temp_hw_index;
        return BCM_E_NONE;
    }

    if (pfc_deadlock_control->pfc_deadlock_cos_used >=
        pfc_deadlock_control->pfc_deadlock_cos_max) {
        /* Config does not exist for given Priority and System Max reached */
        return BCM_E_RESOURCE;;
    }

    SOC_IF_ERROR_RETURN(
        soc_reg32_get(unit, hw_res->chip_config[0], REG_PORT_ANY, 0, &rval));

    for (i = 0; i < pfc_deadlock_control->pfc_deadlock_cos_max; i++) {
        if ((first_avail == -1) &&
            (pfc_deadlock_control->hw_cos_idx_inuse[i] == FALSE)) {
            /* Store the First available Cos idx for Use */
            first_avail = i;
            break;
        }
    }

    if (first_avail == -1) {
        /* New index required, yet No available slots */
        return BCM_E_RESOURCE;
    }

    /* New priority. Free slot found (first_avail) for programming */
    pfc_deadlock_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);
    pfc_deadlock_config->priority = priority;
    pfc_deadlock_config->flags |= _BCM_PFC_DEADLOCK_F_ENABLE;
    pfc_deadlock_control->hw_cos_idx_inuse[first_avail] = TRUE;
    pfc_deadlock_control->pfc_deadlock_cos_used++;
    *hw_cos_index = first_avail;
    return BCM_E_NONE;
}

/* Routine to check given detection_timer against the time_unit configured using
 * Switch control bcmSwitchPFCDeadlockDetectionTimeInterval
 *
 * Return:
 *      BCM_E_PARAM - if timer does not fall within the range
 *      BCM_E_NONE  - Operation success
 * Note:
 *      detection_timer - (INOUT) - Aligned timer(floor value) will be returned.
 */
STATIC int
_bcm_pfc_deadlock_detection_timer_validate(int unit, uint32 *detection_timer)
{
    uint32 detection_granularity = 0, time_offset;

    detection_granularity = (_BCM_PFC_DEADLOCK_TIME_UNIT(unit) ==
        bcmSwitchPFCDeadlockDetectionInterval10MiliSecond) ? 10 : 100;

    time_offset = *detection_timer / detection_granularity;
    if (time_offset > 15) {
        /* Time val can take value from 0-15 (4b) */
        return BCM_E_PARAM;
    }
    *detection_timer = time_offset * detection_granularity;
    return BCM_E_NONE;
}

/* Routine to perform the Set/Get operation for the Priority */
STATIC int
_bcm_pfc_deadlock_hw_oper(int unit,
                          _bcm_pfc_deadlock_oper_t operation,
                          bcm_cos_t priority,
                          _bcm_pfc_deadlock_config_t *config)
{
    int hw_cos_index = -1;
    uint32 rval = 0;
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    uint32 detection_granularity = 0;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;
    detection_granularity = (_BCM_PFC_DEADLOCK_TIME_UNIT(unit) ==
        bcmSwitchPFCDeadlockDetectionInterval10MiliSecond) ? 10 : 100;

    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_hw_cos_index_set(unit, priority, &hw_cos_index));

    if (hw_cos_index == -1) {
        /* No matching or free hw_index available for use */
        return BCM_E_RESOURCE;
    }

    rval = 0;
    SOC_IF_ERROR_RETURN(soc_reg32_get(unit, hw_res->chip_config[0],
                                      REG_PORT_ANY, 0, &rval));
    if (operation == _bcmPfcDeadlockOperGet) {

        config->detection_timer = soc_reg_field_get(unit,
                                        hw_res->chip_config[0], rval,
                                        hw_res->time_init_val[hw_cos_index]);
        config->detection_timer *= detection_granularity;
    } else { /* _bcmPfcDeadlockOperSet */
        soc_reg_field_set(unit, hw_res->chip_config[0], &rval,
                          hw_res->time_init_val[hw_cos_index],
                          (config->detection_timer / detection_granularity));
        soc_reg_field_set(unit, hw_res->chip_config[0], &rval,
                          hw_res->cos_field[hw_cos_index], priority);
        SOC_IF_ERROR_RETURN(soc_reg32_set(unit, hw_res->chip_config[0],
                                          REG_PORT_ANY, 0, rval));
        config->priority = priority;
        pfc_deadlock_control->pfc_cos2pri[hw_cos_index] = priority;
        pfc_deadlock_control->pfc_pri2cos[priority] = hw_cos_index;
    }

    return BCM_E_NONE;
}

int _bcm_pfc_deadlock_update_cos_used(int unit)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_config = NULL;
    int idx = 0;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    pfc_deadlock_control->pfc_deadlock_cos_used = 0;

    for (idx = 0; idx < BCM_COS_COUNT; idx++) {
        pfc_deadlock_config = _BCM_PFC_DEADLOCK_CONFIG(unit, idx);
        if (pfc_deadlock_config &&
            SOC_PBMP_NOT_NULL(pfc_deadlock_config->enabled_ports)) {
            pfc_deadlock_control->pfc_deadlock_cos_used++;
        }
    }
    if (pfc_deadlock_control->pfc_deadlock_cos_used == 0) {
        sal_dpc_cancel(INT_TO_PTR(&pfc_deadlock_cb));
        pfc_deadlock_control->cb_enabled = FALSE;
    } else {
        if (pfc_deadlock_control->cb_enabled == FALSE) {
            pfc_deadlock_control->cb_enabled = TRUE;
            pfc_deadlock_cb(INT_TO_PTR(&pfc_deadlock_cb),
                            INT_TO_PTR(_BCM_PFC_DEADLOCK_CB_INTERVAL(unit)),
                            INT_TO_PTR(sal_time_usecs()), 0, 0);
        }
    }

    if (pfc_deadlock_control->pfc_deadlock_cos_used >
        pfc_deadlock_control->pfc_deadlock_cos_max) {
        /* Illegal case */
        return BCM_E_INIT;
    }
    return BCM_E_NONE;
}

STATIC int
_bcm_pfc_deadlock_q_config_helper(int unit,
                                  _bcm_pfc_deadlock_oper_t operation,
                                  bcm_gport_t gport,
                                  bcm_cosq_pfc_deadlock_queue_config_t *config,
                                  uint8 *enable_status)
{
    int rv = BCM_E_NONE;
    uint32 temp_val_lo = 0, temp_val_hi = 0;
    uint64 rval64;
    bcm_port_t local_port;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    soc_reg_t reg = INVALIDr;
    soc_info_t *si = &SOC_INFO(unit);
    int phy_port, mmu_port, pipe, priority, hw_cos_index = -1;

    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    if (!BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
        /* Invalid Gport */
        return BCM_E_PARAM;
    }

    hw_res = &pfc_deadlock_control->hw_regs_fields;
    if (pfc_deadlock_control->cosq_inv_mapping_get) {
        rv = pfc_deadlock_control->cosq_inv_mapping_get(unit, gport, -1,
                                          BCM_COSQ_GPORT_UCAST_QUEUE_GROUP,
                                          &local_port, &priority);
    } else {
        /* Mapping function not defined */
        return BCM_E_INIT;
    }
    if (rv != BCM_E_NONE) {
        if (rv == BCM_E_NOT_FOUND) {
            /* Given GPort/Queue has No Input priority mapping */
            return BCM_E_RESOURCE;
        }
        return rv;
    }
    pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);

    SOC_IF_ERROR_RETURN(soc_port_pipe_get(unit, local_port, &pipe));

    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_hw_cos_index_get(unit, priority, &hw_cos_index));
    if (hw_cos_index == -1) {
        /* No matching or free hw_index available for use */
        return BCM_E_RESOURCE;
    }

    reg = hw_res->timer_en[hw_cos_index];

    phy_port = si->port_l2p_mapping[local_port];
    mmu_port = si->port_p2m_mapping[phy_port];
    mmu_port %= SOC_TH_MMU_PORT_STRIDE;
    COMPILER_64_ZERO(rval64);

    SOC_IF_ERROR_RETURN(soc_reg_get(unit, reg, pipe, 0, &rval64));

    if (operation == _bcmPfcDeadlockOperGet) { /* GET */
        if (enable_status) {
            *enable_status =
                BCM_PBMP_MEMBER(pfc_deadlock_pri_config->deadlock_ports,
                            local_port);
        }
        if (config) {
            if (mmu_port < 32) {
                config->enable =
                    (COMPILER_64_LO(rval64) & (1 << mmu_port)) ? TRUE : FALSE;
            } else {
                config->enable =
                    (COMPILER_64_HI(rval64) & (1 << (mmu_port - 32))) ?
                                 TRUE : FALSE;
            }
        }
        return BCM_E_NONE;
    } else { /* _bcmPfcDeadlockOperSet */
        temp_val_lo = COMPILER_64_LO(rval64);
        temp_val_hi = COMPILER_64_HI(rval64);

        if (config->enable) {
            if (mmu_port < 32) {
                temp_val_lo |= (1 << mmu_port);
            } else {
                temp_val_hi |= (1 << (mmu_port - 32));
            }
            BCM_PBMP_PORT_ADD(pfc_deadlock_pri_config->enabled_ports,
                              local_port);
        } else {
            if (mmu_port < 32) {
                temp_val_lo &= ~(1 << mmu_port);
            } else {
                temp_val_hi &= ~(1 << (mmu_port - 32));
            }

            SOC_PBMP_PORT_REMOVE(pfc_deadlock_pri_config->enabled_ports,
                                 local_port);
        }
        COMPILER_64_SET(rval64, temp_val_hi, temp_val_lo);
        SOC_IF_ERROR_RETURN(soc_reg_set(unit, reg, pipe, 0, rval64));
        if (SOC_PBMP_IS_NULL(pfc_deadlock_pri_config->enabled_ports)) {
            /*First for that port, set ignore_pfc_xoff = 0 (per port reg) */
            BCM_IF_ERROR_RETURN(
                _bcm_pfc_deadlock_ignore_pfc_xoff_clear(unit, hw_cos_index, local_port));
            pfc_deadlock_control->hw_cos_idx_inuse[hw_cos_index] = FALSE;
            pfc_deadlock_pri_config->flags &= ~_BCM_PFC_DEADLOCK_F_ENABLE;
            pfc_deadlock_control->pfc_cos2pri[hw_cos_index] = -1;
            pfc_deadlock_control->pfc_pri2cos[priority] = -1;
        }
    }

    BCM_IF_ERROR_RETURN(_bcm_pfc_deadlock_update_cos_used(unit));
    return BCM_E_NONE;
}

STATIC int
_bcm_pfc_deadlock_config_helper(int unit,
                                _bcm_pfc_deadlock_oper_t operation,
                                bcm_cos_t priority,
                                bcm_cosq_pfc_deadlock_config_t *config,
                                bcm_cosq_pfc_deadlock_info_t *info)
{
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;
    int hw_cos_index = -1;

    pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);

    if (priority >= BCM_COS_COUNT) {
        return BCM_E_PARAM;
    }
    if ((config == NULL) && (info == NULL)) {
        return BCM_E_PARAM;
    }

    if (operation == _bcmPfcDeadlockOperGet) { /* GET */
        BCM_IF_ERROR_RETURN(
            _bcm_pfc_deadlock_hw_cos_index_get(unit, priority, &hw_cos_index));
        if ((hw_cos_index == -1) ||
            (!(pfc_deadlock_pri_config->flags & _BCM_PFC_DEADLOCK_F_ENABLE))) {
            return BCM_E_NOT_FOUND;
        }
        if (config != NULL) {
            config->detection_timer = pfc_deadlock_pri_config->detection_timer;
            config->recovery_timer = pfc_deadlock_pri_config->recovery_timer;
        }
        if (info != NULL) {
            info->enabled_pbmp = pfc_deadlock_pri_config->enabled_ports;
            /* Call to get current status */
            info->deadlock_pbmp = pfc_deadlock_pri_config->deadlock_ports;
        }
    } else { /* _bcmPfcDeadlockOperSet */
        if (config != NULL) {
            if (SOC_PBMP_NOT_NULL(pfc_deadlock_pri_config->deadlock_ports)) {
                /* Return BUSY if trying to modify config for existing Priority,
                 * when recovery is in progress for some ports
                 */
                return BCM_E_BUSY;
            }
            pfc_deadlock_pri_config->detection_timer = config->detection_timer;
            pfc_deadlock_pri_config->recovery_timer = config->recovery_timer;
            BCM_IF_ERROR_RETURN(
                _bcm_pfc_deadlock_hw_oper(unit, operation, priority,
                                          pfc_deadlock_pri_config));
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_pfc_deadlock_control_set
 * Description:
 *      Set PFC Deadlock feature's switch config.
 * Parameters:
 *      unit - Device unit number
 *      type - The desired configuration parameter to retrieve.
 *      arg  - Pointer to retrieved value
 * Returns:
 *      BCM_E_xxx
 * Notes:
 *      bcmSwitchPFCDeadlockCosMax - Set operation is not permitted
 */
int _bcm_pfc_deadlock_control_set(int unit, bcm_switch_control_t type, int arg)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_hw_resorces_t *hw_res = NULL;
    uint32 rval = 0, field_val = 0;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    hw_res = &pfc_deadlock_control->hw_regs_fields;

    switch (type) {
        case bcmSwitchPFCDeadlockDetectionTimeInterval:
            if ((arg < 0) ||
                (arg >= bcmSwitchPFCDeadlockDetectionIntervalCount)) {
                return BCM_E_PARAM;
            }

            rval = 0;
            if (arg == bcmSwitchPFCDeadlockDetectionInterval10MiliSecond) {
                field_val = 0;
            } else {
                field_val = 1; /* Default Unit - Order of 100ms */
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, hw_res->chip_config[0],
                                REG_PORT_ANY, 0, &rval));
            soc_reg_field_set(unit, hw_res->chip_config[0], &rval,
                              hw_res->time_unit_field, field_val);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, hw_res->chip_config[0],
                                              REG_PORT_ANY, 0, rval));
            pfc_deadlock_control->time_unit = arg;
            break;
        case bcmSwitchPFCDeadlockRecoveryAction:
            if ((arg < 0) ||
                (arg >= bcmSwitchPFCDeadlockActionMaxCount)) {
                return BCM_E_PARAM;
            }

            rval = 0;
            if (arg == bcmSwitchPFCDeadlockActionDrop) {
                field_val = 1;
            } else {
                field_val = 0; /* Default action - Transmit */
            }
            SOC_IF_ERROR_RETURN(soc_reg32_get(unit, hw_res->chip_config[1],
                                REG_PORT_ANY, 0, &rval));
            soc_reg_field_set(unit, hw_res->chip_config[1], &rval,
                              hw_res->recovery_action_field, field_val);
            SOC_IF_ERROR_RETURN(soc_reg32_set(unit, hw_res->chip_config[1],
                                              REG_PORT_ANY, 0, rval));
            pfc_deadlock_control->recovery_action = arg;
            break;
        default:
            return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_pfc_deadlock_control_get
 * Description:
 *      Retrieve PFC Deadlock feature's switch config.
 * Parameters:
 *      unit - Device unit number
 *      type - The desired configuration parameter to retrieve.
 *      arg  - Pointer to retrieved value
 * Returns:
 *      BCM_E_xxx
 */
int _bcm_pfc_deadlock_control_get(int unit, bcm_switch_control_t type, int *arg)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    switch (type) {
        case bcmSwitchPFCDeadlockDetectionTimeInterval:
            *arg = pfc_deadlock_control->time_unit;
            break;
        case bcmSwitchPFCDeadlockRecoveryAction:
            *arg = pfc_deadlock_control->recovery_action;
            break;
        default:
            return BCM_E_UNAVAIL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_pfc_deadlock_max_cos_get
 * Description:
 *      Retrieve PFC Deadlock feature's max supported cos.
 * Parameters:
 *      unit     - Device unit number
 *      entries  - Pointer to retrieved value
 * Returns:
 *      BCM_E_xxx
 */
int _bcm_pfc_deadlock_max_cos_get(int unit, int *entries)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);
    *entries = pfc_deadlock_control->pfc_deadlock_cos_max;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_config_set
 * Purpose:
 *     Setup PFC deadlock feature to monitor for the given priority with
 *     associated values.
 * Parameters:
 *     unit             - (IN) unit number
 *     priority         - (IN) priority
 *     config           - (IN) Config for a given priority
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_pfc_deadlock_config_set(int unit, bcm_cos_t priority,
                                bcm_cosq_pfc_deadlock_config_t *config)
{
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_detection_timer_validate(unit,
                                                   &config->detection_timer));
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_config_helper(unit, _bcmPfcDeadlockOperSet,
                                        priority, config, NULL));
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_config_get
 * Purpose:
 *     Get config values for a given priority in PFC deadlock feature.
 * Parameters:
 *     unit             - (IN) unit number
 *     priority         - (IN) priority
 *     config           - (OUT) Config for a given priority
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_pfc_deadlock_config_get(int unit, bcm_cos_t priority,
                                bcm_cosq_pfc_deadlock_config_t *config)
{
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_config_helper(unit, _bcmPfcDeadlockOperGet,
                                        priority, config, NULL));
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_queue_config_set
 * Purpose:
 *     Enable or disable the given Port/Queue for PFC deadlock feature to
 *     monitor.
 * Parameters:
 *     unit             - (IN) unit number
 *     gport            - (IN) gport
 *     config           - (IN) Config for a given UC Queue Gport
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_pfc_deadlock_queue_config_set(int unit, bcm_gport_t gport,
                                bcm_cosq_pfc_deadlock_queue_config_t *config)
{
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_q_config_helper(unit, _bcmPfcDeadlockOperSet,
                                          gport, config, NULL));
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_queue_config_get
 * Purpose:
 *     Get the Enable or disable monitoring status for the given Port/Queue.
 * Parameters:
 *     unit             - (IN) unit number
 *     gport            - (IN) gport
 *     config           - (OUT) Config for a given UC Queue Gport
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_pfc_deadlock_queue_config_get(int unit, bcm_gport_t gport,
                                bcm_cosq_pfc_deadlock_queue_config_t *config)
{
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_q_config_helper(unit, _bcmPfcDeadlockOperGet,
                                          gport, config, NULL));
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_queue_status_get
 * Purpose:
 *     Get the current Deadlock status for the given Port/Queue.
 * Parameters:
 *     unit             - (IN) unit number
 *     gport            - (IN) gport
 *     deadlock_status  - (OUT) Deatlock status for the given UC Queue Gport
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_pfc_deadlock_queue_status_get(int unit, bcm_gport_t gport,
                                       uint8 *enable)
{
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_q_config_helper(unit, _bcmPfcDeadlockOperGet,
                                          gport, NULL, enable));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_cosq_pfc_deadlock_callback_register
 * Purpose:
 *      Register pfc deadlock recovery callback in PFC deadlock feature.
 * Parameters:
 *     unit             - (IN) unit number
 *     callback         - (IN) callback function
 *     userdata         - (IN) user data
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_cosq_pfc_deadlock_recovery_event_register(
    int unit, 
    bcm_cosq_pfc_deadlock_recovery_event_cb_t callback, 
    void *userdata)
{
    _bcm_pfc_deadlock_cb_t *pfc_deadlock_cb = NULL;

    pfc_deadlock_cb = _BCM_UNIT_PFC_DEADLOCK_CB(unit);
    
    pfc_deadlock_cb->pfc_deadlock_cb = callback;
    pfc_deadlock_cb->pfc_deadlock_userdata = userdata;

    return BCM_E_NONE;
    
}

/*
 * Function:
 *     _bcm_cosq_pfc_deadlock_callback_unregister
 * Purpose:
 *      Unregister pfc deadlock recovery callback in PFC deadlock feature.
 * Parameters:
 *     unit             - (IN) unit number
 *     callback         - (IN) callback function
 *     userdata         - (IN) user data
 * Returns:
 *     BCM_E_XXX
 */

int _bcm_cosq_pfc_deadlock_recovery_event_unregister(
    int unit, 
    bcm_cosq_pfc_deadlock_recovery_event_cb_t callback, 
    void *userdata)
{
    _bcm_pfc_deadlock_cb_t *pfc_deadlock_cb = NULL;

    pfc_deadlock_cb = _BCM_UNIT_PFC_DEADLOCK_CB(unit);
    
    pfc_deadlock_cb->pfc_deadlock_cb = NULL;
    pfc_deadlock_cb->pfc_deadlock_userdata = NULL;
    
    return BCM_E_NONE;
}

/*
 * Function:
 *     bcm_esw_cosq_pfc_deadlock_info_get
 * Purpose:
 *     Get bitmap of Enabled(Admin) and Current Deadlock ports status for a
 *     given priority in PFC deadlock feature.
 * Parameters:
 *     unit             - (IN) unit number
 *     priority         - (IN) priority
 *     info             - (OUT) Info for a given priority
 * Returns:
 *     BCM_E_XXX
 */
int _bcm_pfc_deadlock_info_get(int unit, bcm_cos_t priority,
                                bcm_cosq_pfc_deadlock_info_t *pfc_deadlock_info)
{
    BCM_IF_ERROR_RETURN(
        _bcm_pfc_deadlock_config_helper(unit, _bcmPfcDeadlockOperGet, priority,
                                        NULL, pfc_deadlock_info));
    return BCM_E_NONE;
}

/* Set Chip level defautl values */
STATIC int
_bcm_pfc_deadlock_default(int unit)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    int i = 0;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    _BCM_PFC_DEADLOCK_HW_RES_INIT(&pfc_deadlock_control->hw_regs_fields);
    pfc_deadlock_control->pfc_deadlock_cos_max = 0;
    pfc_deadlock_control->pfc_deadlock_cos_used = 0;

    for (i = 0; i < COUNTOF(pfc_deadlock_control->hw_cos_idx_inuse); i++) {
        pfc_deadlock_control->hw_cos_idx_inuse[i] = FALSE;
    }
    for (i = 0; i < BCM_COS_COUNT; i++) {
        pfc_deadlock_control->pfc_cos2pri[i] = -1;
        pfc_deadlock_control->pfc_pri2cos[i] = -1;
    }
    sal_memset(pfc_deadlock_control->pfc_cos2pri, -1,
               (BCM_COS_COUNT * sizeof(bcm_cos_t)));
    sal_memset(pfc_deadlock_control->pfc_pri2cos, -1,
               (BCM_COS_COUNT * sizeof(bcm_cos_t)));
    pfc_deadlock_control->cb_enabled = FALSE;
    pfc_deadlock_control->time_unit = 0; /* 100 ms */
    pfc_deadlock_control->cb_interval = 100 * 1000; /* 100 ms */
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_recovery_reset
 * Purpose:
 *     Recover the system back to default state.
 *     Restore all ports from Deadlock/Recovery state, if Warm boot when
 *     recovery was in progress
 * Parameters:
 *     unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_pfc_deadlock_recovery_reset(int unit)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_config_t *pfc_deadlock_pri_config = NULL;
    int priority = 0, i;
    bcm_port_t port;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    for (i = 0; i < pfc_deadlock_control->pfc_deadlock_cos_max; i++) {
        priority = pfc_deadlock_control->pfc_cos2pri[i];
        /* Skipping invalid priorities */
        if ((priority < 0) || (priority >= _TH_MMU_NUM_INT_PRI)) {
            continue;
        }
        pfc_deadlock_pri_config = _BCM_PFC_DEADLOCK_CONFIG(unit, priority);

        /*
         * COVERITY: Max Port will take care of Port beyond supported range
         */
        /* coverity[overrun-local : FALSE] */
        BCM_PBMP_ITER(pfc_deadlock_pri_config->deadlock_ports, port) {
            if (port >= MAX_PORT(unit)) {
                break; /* Process for valid ports */
            }
            /*
             * COVERITY
             *
             * Max Port num will be updated dynamically per device.
             */
            /* coverity[overrun-local : FALSE] */
            pfc_deadlock_pri_config->port_recovery_count[port] = 0;
            BCM_IF_ERROR_RETURN(
                _bcm_pfc_deadlock_recovery_end(unit, i, port));
        }
    }
    pfc_deadlock_control->cb_enabled = FALSE;
    BCM_IF_ERROR_RETURN(_bcm_pfc_deadlock_update_cos_used(unit));
    pfc_deadlock_control->cosq_inv_mapping_get = NULL;

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_deinit
 * Purpose:
 *     De-Initialize allocated memory for PFC Deadlock feature
 * Parameters:
 *     unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_pfc_deadlock_deinit(int unit)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    if (pfc_deadlock_control != NULL) {
        sal_free(pfc_deadlock_control);
        _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit) = NULL;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_pfc_deadlock_init
 * Purpose:
 *     Initialize (clear) all states/data-structures
 * Parameters:
 *     unit - unit number
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_pfc_deadlock_init(int unit)
{
    _bcm_pfc_deadlock_control_t *pfc_deadlock_control = NULL;
    _bcm_pfc_deadlock_cb_t *pfc_deadlock_cb = NULL;

    pfc_deadlock_control = _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit);

    if (NULL == pfc_deadlock_control) {
        pfc_deadlock_control =
            sal_alloc(sizeof(_bcm_pfc_deadlock_control_t), "pfc_deadlock_ctrl");
        if (!pfc_deadlock_control) {
            return BCM_E_MEMORY;
        }
    }
    sal_memset(pfc_deadlock_control, 0, sizeof(_bcm_pfc_deadlock_control_t));
    _BCM_UNIT_PFC_DEADLOCK_CONTROL(unit) = pfc_deadlock_control;

    pfc_deadlock_cb = _BCM_UNIT_PFC_DEADLOCK_CB(unit);

    if (NULL == pfc_deadlock_cb) {
        pfc_deadlock_cb =
            sal_alloc(sizeof(_bcm_pfc_deadlock_cb_t), "pfc_deadlock_cb");
        if (!pfc_deadlock_cb) {
            return BCM_E_MEMORY;
        }
    }
    sal_memset(pfc_deadlock_cb, 0, sizeof(_bcm_pfc_deadlock_cb_t));
    pfc_deadlock_cb->pfc_deadlock_cb = NULL;
    pfc_deadlock_cb->pfc_deadlock_userdata = NULL;
    _BCM_UNIT_PFC_DEADLOCK_CB(unit) = pfc_deadlock_cb;

    BCM_IF_ERROR_RETURN(_bcm_pfc_deadlock_default(unit));

#if defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_TOMAHAWKX(unit)) {
        SOC_IF_ERROR_RETURN(_bcm_th_pfc_deadlock_init(unit));
    }
#else
    return BCM_E_INIT;
#endif
    if (!SOC_WARM_BOOT(unit)) {
        /* Default Detection timer Unit of Time is 100ms */
        BCM_IF_ERROR_RETURN(
            _bcm_pfc_deadlock_control_set(unit,
                        bcmSwitchPFCDeadlockDetectionTimeInterval,
                        bcmSwitchPFCDeadlockDetectionInterval100MiliSecond));
    }
    return BCM_E_NONE;
}


