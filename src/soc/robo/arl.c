/*
 * $Id: arl.c,v 1.61 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    arlmsg.c
 * Purpose: Keep a synchronized ARL shadow table.
 *      Provide a reliable stream of ARL insert/delete messages.
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/time.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/cm.h>
#include <soc/robo.h>

#include <soc/robo/mcm/driver.h>
#include <soc/arl.h>

/*
 * While the ARL is frozen, the ARLm lock is held.
 *
 * All tasks must obtain the ARLm lock before modifying the CML bits or
 * age timer registers.
 */

typedef struct freeze_s {
    int                 frozen;
    int                 save_cml[SOC_MAX_NUM_PORTS];
    int                 save_age_sec;
    int                 save_age_ena;
} freeze_t;

STATIC freeze_t arl_freeze_state[SOC_MAX_NUM_DEVICES];




#define _TB_LEARN_FREEZE    1
#define _TB_LEARN_THAW      2

#define _TB_PORTMASK_TABLE_ID           0x52
#define _TB_MEM_OP_READ                 1
#define _TB_MEM_OP_WRITE                2

#define _TB_MEM_DATA_2_SHIFT                128     /* 64(bits)*2 */
#define _TB_PORTMASK_MEM_SA_LRN_SHIFT       151
#define _TB_PORTMASK_MEM_LRN_CTRL_SHIFT     152

static int
_soc_tb_arl_fast_learn_control(int unit, int op)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    freeze_t        *f = &arl_freeze_state[unit];
    uint32          temp_cml = 0, sw_lrn = 0, sa_lrn_dis = 0;
    uint32          reg32_val = 0, temp = 0, test_bit = 0;
    uint64          reg64_val, temp64_config;
    soc_port_t      port;
    int             rv = SOC_E_NONE, retry;

    COMPILER_64_ZERO(reg64_val);
    COMPILER_64_ZERO(temp64_config);

    if (!((op == _TB_LEARN_FREEZE) || (op == _TB_LEARN_THAW))){
        return SOC_E_PARAM;
    }
    
    MEM_RWCTRL_REG_LOCK(soc);
    
    /* processes on PORTMASKm on each port */
    reg32_val = _TB_PORTMASK_TABLE_ID;
    rv = REG_WRITE_MEM_INDEXr(unit, &reg32_val);
    if (rv != SOC_E_NONE){
        goto failed_unlock;
    }
    
    PBMP_E_ITER(unit, port) {
        reg32_val = (uint32)port;
        rv = REG_WRITE_MEM_ADDR_0r(unit, &reg32_val);
        if (rv != SOC_E_NONE){
            goto failed_unlock;
        }

        /* performing READ process */
        reg32_val = 0;
        temp = _TB_MEM_OP_READ;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg32_val);
        if (rv != SOC_E_NONE){
            goto failed_unlock;
        }

        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg32_val);
            if (rv < 0) {
                goto failed_unlock;
            }
            soc_MEM_CTRLr_field_get(unit, &reg32_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto failed_unlock;
        }

        /* read the learning configuration related data buffer only for 
         * performance issue.
         */
        rv = REG_READ_MEM_DATA_2r(unit, (void *)&reg64_val);
        if (rv < 0) {
            goto failed_unlock;
        }

        if (op == _TB_LEARN_FREEZE){
            /* get learning related configurations */
            sal_memcpy(&temp64_config, &reg64_val, sizeof(uint64));
            test_bit = _TB_PORTMASK_MEM_SA_LRN_SHIFT - _TB_MEM_DATA_2_SHIFT;
            sa_lrn_dis = COMPILER_64_BITTEST(temp64_config, test_bit);
            test_bit = _TB_PORTMASK_MEM_LRN_CTRL_SHIFT - _TB_MEM_DATA_2_SHIFT;
            sw_lrn = COMPILER_64_BITTEST(temp64_config, test_bit);

            /* save learning configurations */
            temp_cml = (sa_lrn_dis) ? DRV_PORT_DISABLE_LEARN : 
                        ((sw_lrn) ? DRV_PORT_SW_LEARN : DRV_PORT_HW_LEARN);
            f->save_cml[port] = temp_cml;
        
            /* disable TB's learning configuration :
             *  - To freeze ARL learning on TB, set SA_LRN_DISABLE(bit 151)
             *      is enough.
             *  - considering the performance request. If this port is already
             *      disable SA learning, skip mem_write process on this port.
             *
             *  1. write to data_2 buffer
             *  2. entry write.
             */
            if (temp_cml == DRV_PORT_DISABLE_LEARN) {
                LOG_INFO(BSL_LS_SOC_ARL,
                         (BSL_META_U(unit,
                                     "port%d no freeze for SA_LEARN disabled already./n"),
                          port));
                continue;
            }
            sa_lrn_dis = 1;
            COMPILER_64_SET(temp64_config, 0, sa_lrn_dis);
            temp = _TB_PORTMASK_MEM_SA_LRN_SHIFT - _TB_MEM_DATA_2_SHIFT;
            COMPILER_64_SHL(temp64_config, temp);
            
            COMPILER_64_OR(reg64_val, temp64_config);
        } else {
            
            /* restore original learning configuration for THAW 
             *  - considering the performance request. If this port is already
             *      disable SA learning, skip mem_write process on this port.
             */
            temp_cml = f->save_cml[port];
            if (temp_cml == DRV_PORT_DISABLE_LEARN){
                LOG_INFO(BSL_LS_SOC_ARL,
                         (BSL_META_U(unit,
                                     "port%d no thaw for SA_LEARN disabled originally.\n"),
                          port));
                continue;
            }

            sa_lrn_dis = 1;     /* for NOT operation */
            COMPILER_64_SET(temp64_config, 0, sa_lrn_dis);
            temp = _TB_PORTMASK_MEM_SA_LRN_SHIFT - _TB_MEM_DATA_2_SHIFT;
            COMPILER_64_SHL(temp64_config, temp);
            COMPILER_64_NOT(temp64_config);
            
            COMPILER_64_AND(reg64_val, temp64_config);
        }
        
        /* performing WRITE process */
        rv = REG_WRITE_MEM_DATA_2r(unit, (void *)&reg64_val);
        if (rv < 0) {
            goto failed_unlock;
        }

        reg32_val = 0;
        temp = _TB_MEM_OP_WRITE;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, OP_CMDf, &temp);
        temp = 1;
        soc_MEM_CTRLr_field_set(unit, &reg32_val, MEM_STDNf, &temp);
        rv = REG_WRITE_MEM_CTRLr(unit, &reg32_val);
        if (rv != SOC_E_NONE){
            goto failed_unlock;
        }
        
        /* wait for complete */
        for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
            rv = REG_READ_MEM_CTRLr(unit, &reg32_val);
            if (rv < 0) {
                goto failed_unlock;
            }
            soc_MEM_CTRLr_field_get(unit, &reg32_val, MEM_STDNf, &temp);
            if (!temp) {
                break;
            }
        }
        if (retry >= SOC_TIMEOUT_VAL) {
            rv = SOC_E_TIMEOUT;
            goto failed_unlock;
        }                
    }

    MEM_RWCTRL_REG_UNLOCK(soc);
    return SOC_E_NONE;

failed_unlock:
    LOG_WARN(BSL_LS_SOC_COMMON,
             (BSL_META_U(unit,
                         "TB's ARL can't be %s! (rv=%d)\n"),
              (op == _TB_LEARN_FREEZE) ? "frozen" : "thawed", rv));
    MEM_RWCTRL_REG_UNLOCK(soc);
    return rv;

}

/*
 * Function:
 *      soc_robo_arl_is_frozen
 * Purpose:
 *   	Provides indication if ARL table is in frozen state
 * Parameters:
 *      unit    - (IN) BCM device number. 
 *      frozen  - (OUT) TRUE if ARL table is frozen, FLASE otherwise 
 * Returns:
 *      SOC_E_NONE
 */

int
soc_robo_arl_is_frozen(int unit, int *frozen)
{
   freeze_t     *f = &arl_freeze_state[unit];

   *frozen = (f->frozen > 0) ? TRUE : FALSE ;

   return (SOC_E_NONE);
}

/*
 * Function:
 *  soc_robo_arl_freeze
 * Purpose:
 *  Temporarily quiesce ARL from all activity (learning, aging)
 * Parameters:
 *  unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *  SOC_E_NONE      Success
 *  SOC_E_XXX       Error (tables not locked)
 * Notes:
 *  Leaves ARLm locked until corresponding thaw.
 *  PTABLE is locked in order to lockout bcm_port calls
 *  bcm_port calls will callout to soc_arl_frozen_cml_set/get
 */

int
soc_robo_arl_freeze(int unit)
{
    freeze_t        *f = &arl_freeze_state[unit];
    soc_port_t      port;
    soc_pbmp_t       pbmp;
    int         rv;
    uint32 val, dis_learn;

    /*
     * First time through: lock arl, disable learning and aging
     */
    soc_mem_lock(unit, INDEX(L2_ARLm));
    if (f->frozen++) {
        soc_mem_unlock(unit, INDEX(L2_ARLm));
        return SOC_E_NONE;      /* Recursive freeze OK */
    }

    if (SOC_IS_TBX(unit)){
        if(SOC_IS_TB_AX(unit)){
            rv = _soc_tb_arl_fast_learn_control(unit, _TB_LEARN_FREEZE);
            if (rv < 0) {
                goto fail;
            }
        } else {
            rv = REG_READ_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
            dis_learn = 1;
            soc_GARLCFGr_field_set(unit, &val, GLB_SA_LRN_CTRLf, &dis_learn);
            rv = REG_WRITE_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
        }        
    } else {
        /* disable learning */
        PBMP_E_ITER(unit, port) {
            rv = DRV_ARL_LEARN_ENABLE_GET(unit, port, 
                (uint32 *) &f->save_cml[port]);
            if (rv < 0) {
                goto fail;
            }
        }
        pbmp = PBMP_E_ALL(unit);

        rv = DRV_ARL_LEARN_ENABLE_SET(unit, pbmp, 
            DRV_PORT_DISABLE_LEARN);
        if (rv < 0) {
            goto fail;
        }
    }
        
    /* disable aging */
    rv = DRV_AGE_TIMER_GET(unit, (uint32 *)&f->save_age_ena, 
        (uint32 *)&f->save_age_sec);
    if (rv < 0) {
        goto fail;
    }
    if (f->save_age_ena) {
        rv = DRV_AGE_TIMER_SET(unit, 0, 
            f->save_age_sec);
    if (rv < 0) {
            goto fail;
        }
    }

    return SOC_E_NONE;

 fail:
    f->frozen--;
    soc_mem_unlock(unit, INDEX(L2_ARLm));
    return rv;
}

/*
 * Function:
 *  soc_arl_thaw
 * Purpose:
 *  Restore normal ARL activity.
 * Parameters:
 *  unit - StrataSwitch PCI device unit number (driver internal).
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Unlocks ARLm.
 */

int
soc_robo_arl_thaw(int unit)
{
    freeze_t        *f = &arl_freeze_state[unit];
    soc_port_t      port;
    int         rv,cml;
    soc_pbmp_t       pbm;
    uint32 dis_learn, val;

    assert(f->frozen > 0);

    if (--f->frozen) {
        return SOC_E_NONE;      /* Thaw a recursive freeze */
    }

    /*
     * Last thaw enables learning and aging, and unlocks arl
     */
    rv = SOC_E_NONE;

    if (SOC_IS_TBX(unit)){        
        if(SOC_IS_TB_AX(unit)){
            rv = _soc_tb_arl_fast_learn_control(unit, _TB_LEARN_THAW);
            if (rv < 0) {
                goto fail;
            }
        } else {
            rv = REG_READ_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
            dis_learn = 0;
            soc_GARLCFGr_field_set(unit, &val, GLB_SA_LRN_CTRLf, &dis_learn);
            rv = REG_WRITE_GARLCFGr(unit, &val);
            if (SOC_FAILURE(rv)) {
                goto fail;
            }
        }
    } else {
        PBMP_E_ITER(unit, port) {
            cml = f->save_cml[port];
            if (cml) {
                SOC_PBMP_CLEAR(pbm);
                SOC_PBMP_PORT_SET(pbm,port);
                rv = DRV_ARL_LEARN_ENABLE_SET(unit,
                    pbm, cml);
        
                if (rv < 0) {
                    goto fail;
                }
            }
        }
        
    }
    
    if (f->save_age_ena) {
        rv = DRV_AGE_TIMER_SET(unit,
                        f->save_age_ena,
                        f->save_age_sec);
        if (rv < 0) {
            goto fail;
        }
    }

 fail:
    soc_mem_unlock(unit, INDEX(L2_ARLm));
    return rv;
}

