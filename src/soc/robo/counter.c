/*
 * $Id: counter.c,v 1.116 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Packet Statistics Counter Management
 *
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/core/spl.h>
#include <sal/core/sync.h>
#include <sal/core/time.h>

#include <soc/robo/mcm/driver.h>
#include <soc/robo/mcm/memregs.h>
#include <soc/drv.h>
#include <soc/counter.h>
#include <soc/ll.h>
#include <soc/debug.h>
#include <soc/error.h>
#include <soc/cm.h>
#include <soc/robo_stat.h>
#ifdef BCM_TB_SUPPORT
#include "tbx/robo_tbx.h"
#endif

#define COUNTER_ATOMIC_DEF soc_control_t *
#define COUNTER_ATOMIC_BEGIN(_soc) \
    do { \
        if ((_soc) && (_soc)->counter_lock) { \
            sal_spinlock_lock((_soc)->counter_lock); \
        } \
    } while (0)
#define COUNTER_ATOMIC_END(_soc) \
    do { \
        if ((_soc) && (_soc)->counter_lock) { \
            sal_spinlock_unlock((_soc)->counter_lock); \
        } \
    } while (0)

/* Per port mapping to counter map structures */
static soc_cmap_t *_port_cmap[SOC_MAX_NUM_SWITCH_DEVICES][SOC_MAX_NUM_PORTS];

#define PORT_CTR_REG(unit, port, idx) \
    (&_port_cmap[unit][port]->cmap_base[idx])
#define PORT_CTR_NUM(unit, port) \
    (_port_cmap[unit][port]->cmap_size)

#ifdef BCM_TB_SUPPORT
/* TX/RX counter mapping to counter map structures */
static soc_cmap_t *_txrx_cmap[SOC_MAX_NUM_SWITCH_DEVICES][TXRX_CTR_TYPE_NUM];

#define TXRX_CTR_REG(unit, type, idx) \
    (&_txrx_cmap[unit][type]->cmap_base[idx])
#define TXRX_CTR_NUM(unit, type) \
    (_txrx_cmap[unit][type]->cmap_size)
#endif


typedef enum bcm53242_family_e {
    SOC_BCM53242,
    SOC_BCM53212,
    SOC_BCM53202
} bcm53242_family_t;

typedef enum bcm53284_family_e {
    SOC_BCM53284,
    SOC_BCM53283,
    SOC_BCM53282
} bcm53284_family_t;

static int bcm53242_flag[SOC_MAX_NUM_SWITCH_DEVICES];
static int bcm53284_flag[SOC_MAX_NUM_SWITCH_DEVICES];

static int bcm5389a1_flag[SOC_MAX_NUM_SWITCH_DEVICES];

#ifdef BCM_DINO16_SUPPORT
#define DINO16_MIB_SEL_G0  0
#define DINO16_MIB_SEL_G1  1
#define DINO16_MIB_SEL_G2  2

int dino16_mib_sel_field[SOC_MAX_NUM_PORTS] = {
        INDEX(MIB_SEL_P0f), INDEX(MIB_SEL_P1f), INDEX(MIB_SEL_P2f), 
        INDEX(MIB_SEL_P3f), INDEX(MIB_SEL_P4f), INDEX(MIB_SEL_P5f), 
        INDEX(MIB_SEL_P6f), INDEX(MIB_SEL_P7f), INDEX(MIB_SEL_P8f), 
        INDEX(MIB_SEL_P9f), INDEX(MIB_SEL_P10f), INDEX(MIB_SEL_P11f),
        INDEX(MIB_SEL_P12f), INDEX(MIB_SEL_P13f), INDEX(MIB_SEL_P14f), 
        INDEX(MIB_SEL_P15f), INDEX(MIB_SEL_P16f)};
#endif /* BCM_DINO16_SUPPORT */

/*
 * Function:
 *      soc_robo_counter_idx_get
 * Purpose:
 *      Get the index of a counter given the counter and port
 * Parameters:
 *      unit - The SOC unit number
 *      reg - The register number
 *      port - The port for which index is being calculated
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */

int
soc_robo_counter_idx_get(int unit, soc_reg_t reg, int port)
{
    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        int  i;
        soc_ctr_ref_t  *ctr_ref;
        int shift_port = 0;

        if (bcm53284_flag[unit] == SOC_BCM53282) {
            if (port >= 24) {
                shift_port = port-16;
            } else {
                shift_port = port;
            }
        } else if (bcm53284_flag[unit] == SOC_BCM53283) {
            if (port >= 24) {
                shift_port = port-8;
            } else {
                shift_port = port;
            }
        } else {
            shift_port = port;
        }

        /* check the reg counter index is RX or TX counter type */
        if (SOC_REG_INFO(unit, reg).ctr_idx < TXRX_CTR_NUM(unit, TXRX_CTR_TYPE_RX)) {
            for (i = 0; i < TXRX_CTR_NUM(unit, TXRX_CTR_TYPE_RX); i++) {
                ctr_ref = TXRX_CTR_REG(unit, TXRX_CTR_TYPE_RX, i);
                if (reg == ctr_ref->reg) {
                    /* counter_index = SOC_CTR_TYPE_RX */
                    return ((shift_port * TXRX_MAX_COUNTER_NUM(unit)) + 
                            (SOC_REG_INFO(unit, reg).ctr_idx));
                }
            }
        }

        /* counter_index = SOC_CTR_TYPE_TX */
        return ((shift_port * TXRX_MAX_COUNTER_NUM(unit)) + 
                RX_MAX_COUNTER_NUM(unit) + (SOC_REG_INFO(unit, reg).ctr_idx));
#endif
    } else {
        return ((port * SOC_MAX_COUNTER_NUM(unit)) + 
                (SOC_REG_INFO(unit, reg).ctr_idx));
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_cmap_init
 * Purpose:
 *      Initialize the counter map according to port types
 * Parameters:
 *      unit     - unit number
 * Returns:
 *      None
 * Notes:
 *      
 */
void
soc_robo_counter_cmap_init(int unit)
{
    int  port;
#ifdef BCM_TB_SUPPORT
    int type = 0;
#endif


    /* Set up and install counter maps */
    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        for (type = 0; type < TXRX_CTR_TYPE_NUM; type++) {
            if (type == TXRX_CTR_TYPE_RX) {
                _txrx_cmap[unit][type] = 
                    &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_RX]);
            } else if (type == TXRX_CTR_TYPE_TX) {
                _txrx_cmap[unit][type] = 
                    &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_TX]);
            } else {
                _txrx_cmap[unit][type] = NULL;
            }
            assert(_txrx_cmap[unit][type]);
            assert(_txrx_cmap[unit][type]->cmap_base);
        }
#endif
    } else {
        PBMP_ITER(PBMP_ALL(unit),port){
            _port_cmap[unit][port] = NULL;
            if (IS_FE_PORT(unit, port)) {
                _port_cmap[unit][port] = 
                    &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_FE]);
            } else if (IS_GE_PORT(unit, port)) {
                _port_cmap[unit][port] =
                    &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_GE]);
            } else if (IS_HG_PORT(unit, port) || (port == IPIC_PORT(unit))) {
                _port_cmap[unit][port] =
                    &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_HG]);
            } else if (IS_XE_PORT(unit, port)) {
                _port_cmap[unit][port] =
                    &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_XE]);
            } else if (IS_CPU_PORT(unit, port)) { /* For Robo, CMIC = MII = FE. */
                if( SOC_IS_LOTUS(unit)){
                    _port_cmap[unit][port] =
                        &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_FE]);
                }
                if( SOC_IS_HARRIER(unit) ||
                    SOC_IS_VULCAN(unit)||SOC_IS_BLACKBIRD(unit) ||
                    SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                    SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                    SOC_IS_DINO(unit) ||  SOC_IS_NORTHSTARPLUS(unit) ||
                    SOC_IS_STARFIGHTER3(unit)) {
                     _port_cmap[unit][port] =
                        &(SOC_DRIVER(unit)->counter_maps[SOC_CTR_TYPE_GE]);
                }
            }
            assert(_port_cmap[unit][port]);
            assert(_port_cmap[unit][port]->cmap_base);
        }
    }

    if (SOC_IS_HARRIER(unit)) {
        uint32  reg_value = 0;
        int rv = 0;

        rv = REG_READ_BONDING_PADr
            (unit, &reg_value);
        if (!rv) {
            return;
        }
        reg_value &= 0x1f;
        reg_value >>= 1;
        bcm53242_flag[unit] = SOC_BCM53242;
        if (reg_value == 0x5) {
            bcm53242_flag[unit] = SOC_BCM53212;
        } else if (reg_value == 0x3) {
            bcm53242_flag[unit] = SOC_BCM53202;
        }
    }

    if (SOC_IS_TBX(unit)) {
        uint16      dev_id;
        uint8       rev_id;

        soc_cm_get_id(unit, &dev_id, &rev_id);
        switch (dev_id) {
            case BCM53283_DEVICE_ID:
                bcm53284_flag[unit] = SOC_BCM53283;
                break;
            case BCM53282_DEVICE_ID:
                bcm53284_flag[unit] = SOC_BCM53282;
                break;
            default:
                bcm53284_flag[unit] = SOC_BCM53284;
                break;
        }
    }

    bcm5389a1_flag[unit] = 0;
#ifdef BCM_DINO8_SUPPORT
    if (SOC_IS_DINO8(unit)) {
        uint32  model_id = 0, rev_id = 0;

        REG_READ_MODEL_IDr(unit, &model_id);
        REG_READ_CHIP_REVIDr(unit, &rev_id);
        if ((model_id == BCM5389_A1_DEVICE_ID) &&
            (rev_id == BCM5389_A1_REV_ID)) {
            bcm5389a1_flag[unit] = 1;
        }
    }
#endif /* BCM_DINO8_SUPPORT */

}

/*
 * Function:
 *      soc_robo_port_cmap_get/set
 * Purpose:
 *      Access the counter map structure per port
 */

soc_cmap_t *
soc_robo_port_cmap_get(int unit, soc_port_t port)
{
    return (_port_cmap[unit][port]);
}

int
soc_robo_port_cmap_set(int unit, soc_port_t port, soc_ctr_type_t ctype)
{
    _port_cmap[unit][port] = &(SOC_DRIVER(unit)->counter_maps[ctype]);

    return SOC_E_NONE;
}

#ifdef BCM_TB_SUPPORT
/*
 * Function:
 *      soc_tb_cmap_get/set
 * Purpose:
 *      Access the counter map structure per port
 */

soc_cmap_t *
soc_tb_cmap_get(int unit, int type)
{
    return (_txrx_cmap[unit][type]);
}

int
soc_tb_cmap_set(int unit, soc_ctr_type_t ctype)
{
    _txrx_cmap[unit][ctype] = &(SOC_DRIVER(unit)->counter_maps[ctype]);

    return SOC_E_NONE;
}
#endif

/*
 * Function:
 *  _soc_robo_counter_illegal
 * Purpose:
 *  Routine to display info about bad counter and halt.
 */

void
_soc_robo_counter_illegal(int unit, soc_reg_t ctr_reg)
{
    LOG_CLI((BSL_META_U(unit,
                        "soc_robo_counter_get: unit %d: ctr_reg %d (%s) is not a counter\n"),
             unit, ctr_reg,
             (SOC_IS_ROBO(unit) && SOC_REG_IS_VALID(unit, ctr_reg) ?
             SOC_ROBO_REG_NAME(unit, ctr_reg):"invalid")));
    /* assert(0); */
}

#ifdef BCM_TB_SUPPORT
/*
 * Function:
 *      _soc_tb_counter_collect_sync
 * Purpose:
 *      This routine gets called to sync and accumulate sw counter for specified register of port.
 * Parameters:
 *      unit - uint number.
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 *      port - port number. 
 *      ctr_reg - ref structure for counter register.
 * Returns:
 *      SOC_E_XXX

 */

static int
_soc_tb_counter_collect_sync(int unit, int discard, soc_port_t port, 
    soc_reg_t ctr_reg)
{
    soc_control_t  *soc = SOC_CONTROL(unit);
    int  counter_index = 0;
    uint32  reg_val32, ctr_new32, ctraddr, temp;
    uint64  ctr_new, ctr_prev, ctr_diff;
    COUNTER_ATOMIC_DEF s = SOC_CONTROL(unit);

    /* Port munber for MIB Port selection */
    SOC_IF_ERROR_RETURN(REG_READ_MIB_PORT_SELr
        (unit, &reg_val32));
    temp = (uint32)port;
    SOC_IF_ERROR_RETURN(soc_MIB_PORT_SELr_field_set
        (unit, &reg_val32, MIB_PORTf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MIB_PORT_SELr
        (unit, &reg_val32));

    counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);

    ctraddr = DRV_REG_ADDR(unit, ctr_reg, 0, 0);

    ctr_prev = soc->counter_hw_val[counter_index];

    /* Read the counter value from H/W */
    switch (ctr_reg) {
    case INDEX(RXOCTETSr):
    case INDEX(RXGOODOCTETSr):
    case INDEX(TXOCTETSr):
    case INDEX(TXQQ0OCTETSr):
    case INDEX(TXQQ1OCTETSr):
    case INDEX(TXQQ2OCTETSr):
    case INDEX(TXQQ3OCTETSr):
    case INDEX(TXQQ4OCTETSr):
    case INDEX(TXQQ5OCTETSr):
    case INDEX(TXQQ6OCTETSr):
    case INDEX(TXQQ7OCTETSr):
        /* For the 64-bit counter */
        SOC_IF_ERROR_RETURN(DRV_REG_READ
            (unit, ctraddr, &ctr_new, 8));
        break;
    default:
        /* For the 32-bit counter */
        SOC_IF_ERROR_RETURN(DRV_REG_READ
            (unit, ctraddr, &ctr_new32, 4));
        COMPILER_64_SET(ctr_new, 0, ctr_new32);
    }

    if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
        COUNTER_ATOMIC_BEGIN(s);
        COMPILER_64_ZERO(soc->counter_delta[counter_index]);
        COUNTER_ATOMIC_END(s);
        return SOC_E_NONE;
    }

    if (discard) {
        COUNTER_ATOMIC_BEGIN(s);
        /* Update the previous value buffer */
        soc->counter_hw_val[counter_index] = ctr_new;
        COMPILER_64_ZERO(soc->counter_delta[counter_index]);
        COUNTER_ATOMIC_END(s);
        return SOC_E_NONE;
    }

    ctr_diff = ctr_new;

    if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
        int  width;
        uint64  wrap_amt;

        /*
         * Counter must have wrapped around.
         * Add the proper wrap-around amount.
         */
        width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;

        if (width < 32) {
            COMPILER_64_SET(wrap_amt, 0, 1UL << width);
            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
        } else if (width < 64) {
            COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
        }
    }

    COMPILER_64_SUB_64(ctr_diff, ctr_prev);

    COUNTER_ATOMIC_BEGIN(s);
    /* 
     * For ROBO chips, 
     * MIB counter TxPausePkts always counts both backpressure(half duplex) and
     * pause frames(full duplex). But this counter should not accumulate 
     * when duplex is half.
     * Thus, update SW counter table only when duplex is full.
     */
    if (ctr_reg == INDEX(TXPAUSEPKTSr)) {
        uint32  fd = 0;
        DRV_PORT_GET(unit, port, DRV_PORT_PROP_DUPLEX, &fd);
        if (fd == DRV_PORT_STATUS_DUPLEX_FULL) {
            COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
            soc->counter_delta[counter_index] = ctr_diff;
        } else {
            COMPILER_64_ZERO(soc->counter_delta[counter_index]);
        }
        soc->counter_hw_val[counter_index] = ctr_new;
    } else {
        COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
        soc->counter_delta[counter_index] = ctr_diff;
        soc->counter_hw_val[counter_index] = ctr_new;
    }

    COUNTER_ATOMIC_END(s);

    /*
     * Allow other tasks to run between processing each port.
     */
    sal_thread_yield();

    return SOC_E_NONE;
}
#endif

/*
 * Function:
 *      soc_robo_counter_collect_sync
 * Purpose:
 *      This routine gets called to sync and accumulate sw counter for specified register of port.
 * Parameters:
 *      unit - uint number.
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 *      port - port number. 
 *      ctr_reg - ref structure for counter register.
 * Returns:
 *      SOC_E_XXX
 *
 */
static int
soc_robo_counter_collect_sync(int unit, int discard, soc_port_t port, 
    soc_reg_t ctr_reg)
{
    soc_control_t  *soc = SOC_CONTROL(unit);
    int  counter_index = 0;    
    uint32  ctr_new32, ctraddr;
    uint64  ctr_new, ctr_prev, ctr_diff;
    int  shift_port=0;
    COUNTER_ATOMIC_DEF s = SOC_CONTROL(unit);

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        return _soc_tb_counter_collect_sync(unit, discard, port, ctr_reg);
#endif
    }

    if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) || 
        SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
        bcm5389a1_flag[unit] || SOC_IS_STARFIGHTER3(unit)) {
        /* 
         * For BCM53115, CPU port (port 8, parsed from portbitmap)
         * should change its value as 6 when calculate counter_index.
         */
        if (port == 8) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                shift_port = NUM_ALL_PORT(unit) - 1;
            } else {
                shift_port = 5;
            }
        } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
            /* Port 4 is not used in Starfighter3 */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }

        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
               SOC_IS_NORTHSTARPLUS(unit)) {
        if (port >= 7) {
        	/* Port 6 is "no use" in Polar/Northstar */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_HARRIER(unit)) {
        if (bcm53242_flag[unit] == SOC_BCM53202) {
            if (port >= 24) {
                shift_port = port-16;
            } else {
               shift_port = port;
            }
        } else if (bcm53242_flag[unit] == SOC_BCM53212) {
            if (port >= 24) {
                shift_port = port-8;
            } else {
               shift_port = port;
            }
        } else {
           shift_port = port;
        }

        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else {
        counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);  
    }
    ctraddr = DRV_REG_ADDR(unit, ctr_reg, port, 0);

    ctr_prev = soc->counter_hw_val[counter_index];

    COMPILER_64_ZERO(ctr_new);

    if (SOC_IS_HARRIER(unit)|| SOC_IS_ROBO_ARCH_VULCAN(unit) ||
        SOC_IS_DINO8(unit)) {
        /* Read the counter value from H/W */
        switch (ctr_reg) {
        case INDEX(TXOCTETSr):
        case INDEX(RXOCTETSr):
        case INDEX(RXGOODOCTETSr):
        case INDEX(TXQOS0OCTETSr):
        case INDEX(TXQOS1OCTETSr):
        case INDEX(TXQOS2OCTETSr):
        case INDEX(TXQOS3OCTETSr):
        case INDEX(TXQOSOCTETSr):
        case INDEX(RXQOSOCTETSr):
            /* For the 64-bit counter */
            SOC_IF_ERROR_RETURN(DRV_REG_READ
                (unit, ctraddr, &ctr_new, 8));
            break;
        default:
            /* For the 32-bit counter */
            SOC_IF_ERROR_RETURN(DRV_REG_READ
                (unit, ctraddr, &ctr_new32, 4));
            COMPILER_64_SET(ctr_new, 0, ctr_new32);
        }
    }
    
    if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
        COUNTER_ATOMIC_BEGIN(s);
        COMPILER_64_ZERO(soc->counter_delta[counter_index]);
        COUNTER_ATOMIC_END(s);
        return SOC_E_NONE;
    }

    if (discard) {
        COUNTER_ATOMIC_BEGIN(s);
        /* Update the previous value buffer */
        soc->counter_hw_val[counter_index] = ctr_new;
        COMPILER_64_ZERO(soc->counter_delta[counter_index]);
        COUNTER_ATOMIC_END(s);
        return SOC_E_NONE;
    }

    ctr_diff = ctr_new;

    if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
        int             width;
        uint64          wrap_amt;

        /*
         * Counter must have wrapped around.
         * Add the proper wrap-around amount.
         */
        width = SOC_REG_INFO(unit, ctr_reg).fields[0].len;

        if (width < 32) {
            COMPILER_64_SET(wrap_amt, 0, 1UL << width);
            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
        } else if (width < 64) {
            COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
            COMPILER_64_ADD_64(ctr_diff, wrap_amt);
        }
    }

    COMPILER_64_SUB_64(ctr_diff, ctr_prev);

    COUNTER_ATOMIC_BEGIN(s);
    /* 
     * For ROBO chips, 
     * MIB counter TxPausePkts always counts both backpressure(half duplex) and
     * pause frames(full duplex). But this counter should not accumulate 
     * when duplex is half.
     * Thus, update SW counter table only when duplex is full.
     */
    if (ctr_reg == INDEX(TXPAUSEPKTSr)) {
        uint32 fd = 0;
        DRV_PORT_GET(unit, port, DRV_PORT_PROP_DUPLEX, &fd);
        if (fd == DRV_PORT_STATUS_DUPLEX_FULL) {
            COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
            soc->counter_delta[counter_index] = ctr_diff;
        } else {
            COMPILER_64_ZERO(soc->counter_delta[counter_index]);
        }
        soc->counter_hw_val[counter_index] = ctr_new;
    } else {
        COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
        soc->counter_delta[counter_index] = ctr_diff;
        soc->counter_hw_val[counter_index] = ctr_new;
    }

    COUNTER_ATOMIC_END(s);


    /*
     * Allow other tasks to run between processing each port.
     */
    sal_thread_yield();

    return SOC_E_NONE;
}


/*
 * Function:
 *      _soc_robo_counter_get
 * Purpose:
 *      Given a counter register number and port number, fetch the
 *      64-bit software-accumulated counter value.  The software-
 *      accumulated counter value is zeroed if requested.
 * Parameters:
 *      unit - uint number.
 *      port - port number.
 *      ctr_ref - ref structure for counter register.
 *      zero - if TRUE, current counter is zeroed after reading.
 *      sync_hw - if TRUE, read from device else software accumulated value.
 *      val - (OUT) 64-bit counter value.
 * Returns:
 *      SOC_E_XXX.
 * Notes:
 *      Returns 0 if ctr_reg is INVALID_Rr.
 */
static INLINE int
_soc_robo_counter_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
         int zero, int sync_hw, uint64 *val)
{
    soc_control_t           *soc;
    int             counter_index = 0;    
    COUNTER_ATOMIC_DEF  s = SOC_CONTROL(unit);
    int  shift_port = 0;

    soc = SOC_CONTROL(unit);

    /* The input register is invalid */
    if (SOC_COUNTER_INVALID(unit, ctr_reg)) {
        return SOC_E_UNAVAIL;
    }

    /* The input register is not counter register */
    if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
        _soc_robo_counter_illegal(unit, ctr_reg);
        COMPILER_64_ZERO(*val);
        return SOC_E_NONE;
    }

    if (sync_hw == TRUE) {
        /* Sync the software counter with hardware counter */
        soc_robo_counter_collect_sync(unit, FALSE, port, ctr_reg);
    }

    if ((SOC_IS_ROBO(unit) && (SOC_IS_VULCAN(unit) || 
        SOC_IS_LOTUS(unit)|| SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_BLACKBIRD2(unit))) || bcm5389a1_flag[unit] ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* 
         * For BCM53115, CPU port (port 8, parsed from portbitmap)
         * should change its value as 6 when calculate counter_index.
         */
        if (port == 8) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                shift_port = NUM_ALL_PORT(unit) - 1;  
            } else {
                shift_port = 5;
            }
        } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
            /* Port 4 is not used in Starfighter3 */
            shift_port = port - 1;
        } else {
            shift_port = port;     
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
               SOC_IS_NORTHSTARPLUS(unit)) {
        if (port >= 7) {
            /* Port 6 is "no use" in Polar/Northstar */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }    
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_HARRIER(unit)){
        if (bcm53242_flag[unit] == SOC_BCM53202) {
            if (port >= 24) {
                shift_port = port-16;
            } else {
                shift_port = port;
            }
        } else if (bcm53242_flag[unit] == SOC_BCM53212) {
            if (port >= 24) {
                shift_port = port-8;
            } else {
                shift_port = port;
            }
        } else {
            /* 53242 */
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);
#endif
    } else {
        counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +             
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);  
    }

    if (zero) {
        COUNTER_ATOMIC_BEGIN(s);
        *val = soc->counter_sw_val[counter_index];
        COMPILER_64_ZERO(soc->counter_sw_val[counter_index]);
        COUNTER_ATOMIC_END(s);
    } else {
        COUNTER_ATOMIC_BEGIN(s);
        *val = soc->counter_sw_val[counter_index];
        COUNTER_ATOMIC_END(s);
    }

    return SOC_E_NONE;
}

#ifdef BCM_TB_SUPPORT
#ifdef MIB_SNAPSHOT
/*
 * Function:
 *      _soc_tb_counter_snapshot_get
 * Purpose:
 *      Given a counter register number and port number, fetch the
 *      64-bit software-accumulated counter value by MIB Snapshot.  
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      port - RoboSwitch port #.
 *      ctr_ref - ref structure for counter register.
 *      val - (OUT) 64-bit counter value.
 * Returns:
 *      SOC_E_XXX.
 * Notes:
 *      Returns 0 if ctr_reg is INVALID_Rr.
 */
static INLINE int
_soc_tb_counter_snapshot_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
         uint64 *val)
{
    uint32  reg_addr, reg_len;
    uint32  reg_value, temp, count;
    uint64  reg_value64;
    int rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(REG_READ_MIB_SNAPSHOT_CTLr
        (unit, &reg_value));

    temp = (uint32)port;
    SOC_IF_ERROR_RETURN(soc_MIB_SNAPSHOT_CTLr_field_set
        (unit, &reg_value, SNAPSHOT_PORTf, &temp));

    temp = 1;
    SOC_IF_ERROR_RETURN(soc_MIB_SNAPSHOT_CTLr_field_set
        (unit, &reg_value, SNAPSHOT_STDNf, &temp));

    SOC_IF_ERROR_RETURN(REG_WRITE_MIB_SNAPSHOT_CTLr
        (unit, &reg_value));

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        SOC_IF_ERROR_RETURN(REG_READ_MIB_SNAPSHOT_CTLr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_MIB_SNAPSHOT_CTLr_field_get
            (unit, &reg_value, SNAPSHOT_STDNf, &temp));
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        return SOC_E_TIMEOUT;
    }

    reg_addr = DRV_REG_ADDR(unit, ctr_reg, 0, 0);
    /* 
     * Mapping MIB counter to MIB Snapshot memory.
     * TB: Page reassign 0x51(RX)~0x52(TX) --> 0x53(RX)~0x54(TX).
     */
    reg_addr = (reg_addr & 0xffff) + 0x200;
    reg_len = DRV_REG_LENGTH_GET(unit, ctr_reg);

    if ((rv = DRV_REG_READ
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    *val = reg_value64;
    
    return rv;
}
#endif /* MIB_SNAPSHOT */
/*
 * Function:
 *      _soc_tb_counter_collect
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.  It collects the counters.
 * Parameters:
 *      unit - uint number.
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      SOC_E_XXX

 */

static int
_soc_tb_counter_collect(int unit, int discard)
{
    soc_control_t  *soc = SOC_CONTROL(unit);
    int  type, i, counter_index = 0;
    uint32  reg_val32, ctr_new32, ctraddr, temp;
    soc_port_t  port;
    uint64  ctr_new, ctr_prev, ctr_diff;
    soc_ctr_ref_t  *ctr_ref;

    PBMP_ITER(soc->counter_pbmp, port) {
        /* Port munber for MIB Port selection */
        SOC_IF_ERROR_RETURN(REG_READ_MIB_PORT_SELr
            (unit, &reg_val32));
        temp = (uint32)port;
        SOC_IF_ERROR_RETURN(soc_MIB_PORT_SELr_field_set
            (unit, &reg_val32, MIB_PORTf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_MIB_PORT_SELr
            (unit, &reg_val32));
        for (type = 0; type < TXRX_CTR_TYPE_NUM; type++) {
            for (i = 0; i < TXRX_CTR_NUM(unit, type); i++) {
                COUNTER_ATOMIC_DEF s = SOC_CONTROL(unit);
                ctr_ref = TXRX_CTR_REG(unit, type, i);
                if (ctr_ref->reg == INDEX(INVALID_Rr)) {
                    continue;
                }
                counter_index = soc_robo_counter_idx_get(unit, ctr_ref->reg, port);

                ctraddr = DRV_REG_ADDR(unit, ctr_ref->reg, 0, 0);
    
                ctr_prev = soc->counter_hw_val[counter_index];
    
                /* Read the counter value from H/W */
                switch (ctr_ref->reg) {
                case INDEX(RXOCTETSr):
                case INDEX(RXGOODOCTETSr):
                case INDEX(TXOCTETSr):
                case INDEX(TXQQ0OCTETSr):
                case INDEX(TXQQ1OCTETSr):
                case INDEX(TXQQ2OCTETSr):
                case INDEX(TXQQ3OCTETSr):
                case INDEX(TXQQ4OCTETSr):
                case INDEX(TXQQ5OCTETSr):
                case INDEX(TXQQ6OCTETSr):
                case INDEX(TXQQ7OCTETSr):
                    /* For the 64-bit counter */
                    SOC_IF_ERROR_RETURN(DRV_REG_READ
                        (unit, ctraddr, &ctr_new, 8));
                    break;
                default:
                    /* For the 32-bit counter */
                    SOC_IF_ERROR_RETURN(DRV_REG_READ
                        (unit, ctraddr, &ctr_new32, 4));
                    COMPILER_64_SET(ctr_new, 0, ctr_new32);
                }

                if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                    COUNTER_ATOMIC_BEGIN(s);
                    COMPILER_64_ZERO(soc->counter_delta[counter_index]);
                    COUNTER_ATOMIC_END(s);
                    continue;
                }
    
                if (discard) {
                    COUNTER_ATOMIC_BEGIN(s);
                    /* Update the previous value buffer */
                    soc->counter_hw_val[counter_index] = ctr_new;
                    COMPILER_64_ZERO(soc->counter_delta[counter_index]);
                    COUNTER_ATOMIC_END(s);
                    continue;
                }
    
                ctr_diff = ctr_new;
    
                if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                    int  width;
                    uint64  wrap_amt;
    
                    /*
                     * Counter must have wrapped around.
                     * Add the proper wrap-around amount.
                     */
                    width = SOC_REG_INFO(unit, ctr_ref->reg).fields[0].len;
    
                    if (width < 32) {
                        COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                        COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                    } else if (width < 64) {
                        COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
                        COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                    }
                }
    
                COMPILER_64_SUB_64(ctr_diff, ctr_prev);
    
                COUNTER_ATOMIC_BEGIN(s);
                /* 
                 * For ROBO chips, 
                 * MIB counter TxPausePkts always counts both backpressure(half duplex) and
                 * pause frames(full duplex). But this counter should not accumulate 
                 * when duplex is half.
                 * Thus, update SW counter table only when duplex is full.
                 */
                if (ctr_ref->reg == INDEX(TXPAUSEPKTSr)) {
                    uint32  fd = 0;
                    DRV_PORT_GET(unit, port, DRV_PORT_PROP_DUPLEX, &fd);
                    if (fd == DRV_PORT_STATUS_DUPLEX_FULL) {
                        COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
                        soc->counter_delta[counter_index] = ctr_diff;
                    } else {
                        COMPILER_64_ZERO(soc->counter_delta[counter_index]);
                    }
                    soc->counter_hw_val[counter_index] = ctr_new;
                } else {
                    COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
                    soc->counter_delta[counter_index] = ctr_diff;
                    soc->counter_hw_val[counter_index] = ctr_new;
                }
    
                COUNTER_ATOMIC_END(s);
    
            }
        }

        /*
         * Allow other tasks to run between processing each port.
         */
        sal_thread_yield();
    }

    return SOC_E_NONE;
}

static int
_soc_tb_counter_sw_table_set(int unit, soc_pbmp_t pbmp, uint64 val)
{
    soc_control_t  *soc;
    int  type, i;
    soc_port_t  port;
    soc_ctr_ref_t  *ctr_ref;
    soc_reg_t  ctr_reg;
    int  counter_index = 0;    
    COUNTER_ATOMIC_DEF  s = SOC_CONTROL(unit);

    soc = SOC_CONTROL(unit);

    PBMP_ITER(pbmp, port) {
        for (type = 0; type < TXRX_CTR_TYPE_NUM; type++) {
            for (i = 0; i < TXRX_CTR_NUM(unit, type); i++) {
                ctr_ref = TXRX_CTR_REG(unit, type, i);
                ctr_reg = ctr_ref->reg;
                if (ctr_reg != INDEX(INVALID_Rr)) {            
                    counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);
    
                    /* Update the S/W counter */
                    COUNTER_ATOMIC_BEGIN(s);
                    soc->counter_sw_val[counter_index] = val;
                    soc->counter_hw_val[counter_index] = val;
                    COMPILER_64_SET(soc->counter_delta[counter_index], 0, 0);
                    /* 
                     * Update to record S/W counter : 
                     * check S/W counter ever been cleared or not for diag.
                     */
                    soc->counter_buf64[counter_index] = val;
                    COUNTER_ATOMIC_END(s);  
                }
            }
        }
    }

    return SOC_E_NONE;
}

#endif
#ifdef MIB_SNAPSHOT
/*
 * Function:
 *      _soc_robo_counter_snapshot_get
 * Purpose:
 *      Given a counter register number and port number, fetch the
 *      64-bit software-accumulated counter value by MIB Snapshot.  
 * Parameters:
 *      unit - RoboSwitch unit #.
 *      port - RoboSwitch port #.
 *      ctr_ref - ref structure for counter register.
 *      val - (OUT) 64-bit counter value.
 * Returns:
 *      SOC_E_XXX.
 * Notes:
 *      Returns 0 if ctr_reg is INVALID_Rr.
 */
static INLINE int
_soc_robo_counter_snapshot_get(int unit, soc_port_t port, soc_reg_t ctr_reg,
         uint64 *val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_value, temp, count;
    uint64 reg_value64;
    int rv = SOC_E_NONE;

    SOC_IF_ERROR_RETURN(REG_READ_MIB_SNAPSHOT_CTLr
        (unit, &reg_value));
    temp = (uint32)port;
    SOC_IF_ERROR_RETURN(soc_MIB_SNAPSHOT_CTLr_field_set
        (unit, &reg_value, SNAPSHOT_PORTf, &temp));
    temp = 1;
    SOC_IF_ERROR_RETURN(soc_MIB_SNAPSHOT_CTLr_field_set
        (unit, &reg_value, SNAPSHOT_STDONEf, &temp));
    SOC_IF_ERROR_RETURN(REG_WRITE_MIB_SNAPSHOT_CTLr
        (unit, &reg_value));

    /* wait for complete */
    for (count = 0; count < SOC_TIMEOUT_VAL; count++) {
        SOC_IF_ERROR_RETURN(REG_READ_MIB_SNAPSHOT_CTLr
            (unit, &reg_value));
        SOC_IF_ERROR_RETURN(soc_MIB_SNAPSHOT_CTLr_field_get
            (unit, &reg_value, SNAPSHOT_STDONEf, &temp));
        if (!temp)
            break;
    }

    if (count >= SOC_TIMEOUT_VAL) {
        return SOC_E_TIMEOUT;
    }

    reg_addr = DRV_REG_ADDR(unit, ctr_reg, 0, 0);
    /* 
     * Mapping MIB counter to MIB Snapshot memory.
     * BCM53242: Page reassign 0x50~0x84 --> 0x85.
     * BCM53115: Page reassign 0x20~0x28 --> 0x71.
     * BCM53118: Page reassign 0x20~0x28 --> 0x71.
     */
    if (SOC_IS_ROBO_ARCH_VULCAN(unit)) {
        reg_addr = (reg_addr & 0x00ff) + 0x7100;
    } else if (SOC_IS_HARRIER(unit)) {
        reg_addr = (reg_addr & 0x00ff) + 0x8500;
    }

    reg_len = DRV_REG_LENGTH_GET(unit, ctr_reg);
    if ((rv = DRV_REG_READ
        (unit, reg_addr, (uint32 *)&reg_value64, reg_len)) < 0) {
        return rv;
    }
    *val = reg_value64;
    
    return rv;
}
#endif /* MIB_SNAPSHOT */

/*
 * Function:
 *      soc_robo_counter_set, soc_robo_counter_set32
 * Purpose:
 *      Given a counter register number, port number, and a counter
 *      value, set both the hardware and software-accumulated counters
 *      to the value.
 * Parameters:
 *      unit - uint number.
 *      port - port number.
 *      ctr_reg - counter register to retrieve.
 *      val - 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX
 * Notes:
 *      The 32-bit version sets the upper 32-bits of the 64-bit
 *      software counter to zero.  The specified value is truncated
 *      to the width of the hardware register when storing to hardware.
 *      Use the value 0 to clear counters.
 */
static INLINE int
_soc_robo_counter_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                 uint64 val)
{
    soc_control_t           *soc;
    int             counter_index = 0;    
    COUNTER_ATOMIC_DEF  s = SOC_CONTROL(unit);
    int  shift_port = 0;

    soc = SOC_CONTROL(unit);

    /* The input register is invalid */
    if (SOC_COUNTER_INVALID(unit, ctr_reg)) {
        return SOC_E_UNAVAIL;
    }

    /* The input register is not counter register */
    if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
        _soc_robo_counter_illegal(unit, ctr_reg);
        return SOC_E_NONE;
    }

    if ((SOC_IS_ROBO(unit) && (SOC_IS_VULCAN(unit) || 
        SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_BLACKBIRD2(unit))) || bcm5389a1_flag[unit] ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* 
         * For BCM53115, CPU port (port 8, parsed from portbitmap)
         * should change its value as 6 when calculate counter_index.
         */
        if (port == 8) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                shift_port = NUM_ALL_PORT(unit) - 1;
            } else {
                shift_port = 5;
            }
        } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
            /* Port 4 is not used in Starfighter3 */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
               SOC_IS_NORTHSTARPLUS(unit)) {
        if (port >= 7) {
            /* Port 6 is "no use" in Polar/Northstar */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }    
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_HARRIER(unit)) {
        if (bcm53242_flag[unit] == SOC_BCM53202) {
            if (port >= 24) {
                shift_port = port-16;
            } else {
                shift_port = port;
            }
        } else if (bcm53242_flag[unit] == SOC_BCM53212) {
            if (port >= 24) {
                shift_port = port-8;
            } else {
                shift_port = port;
            }
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);
#endif
    } else {
        counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +             
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);  
    }

    /* Update the S/W counter */
    COUNTER_ATOMIC_BEGIN(s);
    soc->counter_sw_val[counter_index] = val;
    COMPILER_64_SET(soc->counter_delta[counter_index], 0, 0);
    /* 
     * Update to record S/W counter : 
     * check S/W counter ever been cleared or not for diag.
     */
    soc->counter_buf64[counter_index] = val;
    COUNTER_ATOMIC_END(s);

    /* Do not update the H/W counter */
    return SOC_E_NONE;
}


int
_soc_robo_counter_sw_table_set(int unit, soc_pbmp_t pbmp, uint64 val)
{
    soc_control_t           *soc;
    int             i;
    soc_port_t      port;
    soc_ctr_ref_t   *ctr_ref;
    soc_reg_t ctr_reg;
    int             counter_index = 0;    
    COUNTER_ATOMIC_DEF  s = SOC_CONTROL(unit);
    int shift_port=0;
   
    soc = SOC_CONTROL(unit);

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        return _soc_tb_counter_sw_table_set(unit, pbmp, val);
#endif
    }

    PBMP_ITER(pbmp, port) {
        for (i = 0; i < PORT_CTR_NUM(unit, port); i++) {
            ctr_ref = PORT_CTR_REG(unit, port, i);
            ctr_reg = ctr_ref->reg;
            if (ctr_reg != INDEX(INVALID_Rr)) {            
                 if ((SOC_IS_ROBO(unit) && (SOC_IS_VULCAN(unit) || 
                    SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
                    SOC_IS_BLACKBIRD2(unit))) || bcm5389a1_flag[unit]||
                    SOC_IS_STARFIGHTER3(unit)) {
                    /* 
                     * For BCM53115, CPU port (port 8, parsed from portbitmap)
                     * should change its value as 6 when calculate counter_index.
                     */
                    if (port == 8) {
                        if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                            SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                            SOC_IS_STARFIGHTER3(unit)) {
                            shift_port = NUM_ALL_PORT(unit) - 1;            
                        } else {
                        shift_port = 5;
                        }
                    } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
                        /* Port 4 is not used in Starfighter3 */
                        shift_port = port - 1;
                    } else {
                        shift_port = port;
                    }
                    counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                      (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
                } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                           SOC_IS_NORTHSTARPLUS(unit)) {
                    if (port >= 7) {
                        /* Port 6 is "no use" in Polar/Northstar */
                        shift_port = port - 1;
                    } else {
                        shift_port = port;
                    }    
                    counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                      (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
                } else if (SOC_IS_HARRIER(unit)) {
                    if (bcm53242_flag[unit] == SOC_BCM53202) {
                        if (port >= 24) {
                            shift_port = port-16;
                        } else {
                           shift_port = port;
                        }
                    } else if (bcm53242_flag[unit] == SOC_BCM53212) {
                        if (port >= 24) {
                            shift_port = port-8;
                        } else {
                           shift_port = port;
                        }
                    } else {
                       shift_port = port;
                    }
                   counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                         (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
                } else {
                    counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +             
                      (SOC_REG_INFO(unit, ctr_reg).ctr_idx);  
                }

                /* Update the S/W counter */
                COUNTER_ATOMIC_BEGIN(s);
                soc->counter_sw_val[counter_index] = val;
                soc->counter_hw_val[counter_index] = val;
                COMPILER_64_SET(soc->counter_delta[counter_index], 0, 0);
                /* 
                 * Update to record S/W counter : 
                 * check S/W counter ever been cleared or not for diag.
                 */
                soc->counter_buf64[counter_index] = val;
                COUNTER_ATOMIC_END(s);  
            }
        }
    }        
    return SOC_E_NONE;
}
   
/*
 * Function:
 *      soc_robo_counter_get
 * Purpose:
 *      Retrieves the value of a 64-bit software shadow counter.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      sync_hw - if TRUE, read from device else software accumulated value.
 *      val     - (OUT) Pointer to place to store the 64-bit result.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_get(int unit, soc_port_t port, soc_reg_t ctr_reg, 
    int sync_hw, uint64 *val)
{
    COMPILER_REFERENCE(sync_hw);
#ifdef MIB_SNAPSHOT
    if (SOC_IS_HARRIER(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit)){
        return _soc_robo_counter_snapshot_get(unit, port, ctr_reg, val);
    } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        return _soc_tb_counter_snapshot_get(unit, port, ctr_reg, val);
#endif
    } else {
    return _soc_robo_counter_get(unit, port, ctr_reg, FALSE, sync_hw, val);
    }
#else
    return _soc_robo_counter_get(unit, port, ctr_reg, FALSE, sync_hw, val);
#endif
}


/*
 * Function:
 *      soc_robo_counter_get32
 * Purpose:
 *      Retrieve the value of a 64-bit software shadow 
 *  counter, truncated to lower 32 bits only.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - (OUT) Pointer to place to store the 64-bit result.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_get32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                  uint32 *val)
{
    uint64          val64;
    int             rv;
    int  sync_hw = FALSE; /* Read software accumulated counters */

    COMPILER_64_ZERO(val64);

    rv = _soc_robo_counter_get(unit, port, ctr_reg, FALSE, sync_hw, &val64);

    COMPILER_64_TO_32_LO(*val, val64);

    return rv;
}

/*
 * Function:
 *      soc_robo_counter_get_zero
 * Purpose:
 *      Atomically retrieve and clear the value of a 64-bit shadow counter.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - (OUT) Pointer to place to store the 64-bit result.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_get_zero(int unit, soc_port_t port,
             soc_reg_t ctr_reg, uint64 *val)
{
    int  sync_hw = FALSE; /* Read software accumulated counters */
    return _soc_robo_counter_get(unit, port, ctr_reg, TRUE, sync_hw, val);
}

/*
 * Function:
 *      soc_robo_counter_get32_zero
 * Purpose:
 *      Atomically retrieve and clear the value of a 64-bit 
 *  shadow counter, and return the lower 32-bits of the 
 *  original value.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - (OUT) Pointer to place to store the 64-bit result.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_get32_zero(int unit, soc_port_t port,
               soc_reg_t ctr_reg, uint32 *val)
{
    uint64          val64;
    int             rv;
    int  sync_hw = FALSE; /* Read software accumulated counters */

    COMPILER_64_ZERO(val64);

    rv = _soc_robo_counter_get(unit, port, ctr_reg, TRUE, sync_hw, &val64);

    COMPILER_64_TO_32_LO(*val, val64);

    return rv;
}

/*
 * Function:
 *      soc_robo_counter_get_rate
 * Purpose:
 *      Returns the counter incrementing rate in units of 1/sec.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      rate    - (OUT) 64-bit rate value.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_get_rate(int unit, soc_port_t port,
             soc_reg_t ctr_reg, uint64 *rate)
{
    soc_control_t       *soc;

    soc = SOC_CONTROL(unit);

    if (soc->counter_interval == 0) {
        COMPILER_64_ZERO(*rate);
    } else {
#ifdef  COMPILER_HAS_DOUBLE
        int         counter_index = 0;
        int  shift_port = 0;
        uint32          delta32, rate32;
        double          interval;

        /* The input register is not counter register */
        if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
            _soc_robo_counter_illegal(unit, ctr_reg);
            return SOC_E_NONE;
        }

        if ((SOC_IS_ROBO(unit) && (SOC_IS_VULCAN(unit) ||
            SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
            SOC_IS_BLACKBIRD2(unit))) || bcm5389a1_flag[unit] ||
            SOC_IS_STARFIGHTER3(unit)) {
            /*
             * For BCM53115, CPU port (port 8, parsed from portbitmap)
             * should change its value as 6 when calculate counter_index.
             */
            if (port == 8) {
                if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                    SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                    SOC_IS_STARFIGHTER3(unit)) {
                    shift_port = NUM_ALL_PORT(unit) - 1;
                } else {
                    shift_port = 5;
                }
            } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
                /* Port 4 is not used in Starfighter3 */
                shift_port = port - 1;
            } else {
                shift_port = port;
            }
            counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                  (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
        } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
                   SOC_IS_NORTHSTARPLUS(unit)) {
            if (port >= 7) {
                /* Port 6 is "no use" in Polar/Northstar */
                shift_port = port - 1;
            } else {
                shift_port = port;
            }
            counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                  (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
        } else if (SOC_IS_HARRIER(unit)){
            if (bcm53242_flag[unit] == SOC_BCM53202) {
                if (port >= 24) {
                    shift_port = port-16;
                } else {
                    shift_port = port;
                }
            } else if (bcm53242_flag[unit] == SOC_BCM53212) {
                if (port >= 24) {
                    shift_port = port-8;
                } else {
                    shift_port = port;
                }
            } else {
                shift_port = port;
            }
            counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                  (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
        } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
            counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);
#endif
        } else {
            counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +
                  (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
        }
    
        /*
         * This uses floating point right now because uint64 multiply/divide
         * support is missing from many compiler libraries.
         */

        COMPILER_64_TO_32_LO(delta32, soc->counter_delta[counter_index]);

        interval = SAL_USECS_SUB(soc->counter_coll_cur,
                     soc->counter_coll_prev) / 1000000.0;

        if (interval < 0.0001) {
            rate32 = 0;
        } else {
            rate32 = (uint32) (delta32 / interval + 0.5);
        }

        COMPILER_64_SET(*rate, 0, rate32);
#else   /* !COMPILER_HAS_DOUBLE */
        COMPILER_64_ZERO(*rate);
#endif  /* !COMPILER_HAS_DOUBLE */
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_get32_rate
 * Purpose:
 *      Returns the counter incrementing rate in units of 1/sec.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      rate    - (OUT) 32-bit delta value.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_get32_rate(int unit, soc_port_t port,
               soc_reg_t ctr_reg, uint32 *rate)
{
    uint64          rate64;

    COMPILER_64_ZERO(rate64);

    SOC_IF_ERROR_RETURN(soc_robo_counter_get_rate(unit, port, ctr_reg, &rate64));

    COMPILER_64_TO_32_LO(*rate, rate64);

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_set
 * Purpose:
 *      Set a 64-bit software shadow counter to a given value.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - Pointer to place to store the 64-bit result.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_set(int unit, soc_port_t port, soc_reg_t ctr_reg,
                uint64 val)
{
    return _soc_robo_counter_set(unit, port, ctr_reg, val);
}

/*
 * Function:
 *      soc_robo_counter_set32
 * Purpose:
 *      Set a 64-bit software shadow counter to a given 32-bit value.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      port    - RoboSwitch port number.
 *      ctr_reg - counter register to retrieve.
 *      val     - Pointer to place to store the 64-bit result.
 * Returns:
 *      SOC_E_XXX.
 */
int
soc_robo_counter_set32(int unit, soc_port_t port, soc_reg_t ctr_reg,
                  uint32 val)
{
    uint64          val64;

    COMPILER_64_SET(val64, 0, val);

    return _soc_robo_counter_set(unit, port, ctr_reg, val64);
}

/*
 * Function:
 *      soc_robo_counter_set_by_port
 * Purpose:
 *      Sets all 64-bit software shadow counters for a given 
 *  set of ports to a given 64-bit value.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      pbm  - Port bitmap of the ports to set. Use PBMP_ALL for all ports.
 *      val  - 64-bit value to set the counters to.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_set_by_port(int unit, pbmp_t pbmp, uint64 val)
{
    int             i;
    soc_port_t      port;
    soc_ctr_ref_t   *ctr_ref;
#ifdef BCM_TB_SUPPORT
    int type = 0;
#endif
   
    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        PBMP_ITER(pbmp, port) {
            for (type = 0; type < TXRX_CTR_TYPE_NUM; type++) {
                for (i = 0; i < TXRX_CTR_NUM(unit, type); i++) {
                    ctr_ref = TXRX_CTR_REG(unit, type, i);
                    if (ctr_ref->reg != INDEX(INVALID_Rr)) {
                        SOC_IF_ERROR_RETURN
                            (_soc_robo_counter_set(unit, port, ctr_ref->reg, val));
                    }
                }
            }
        }
#endif
    } else {
        PBMP_ITER(pbmp, port) {
            for (i = 0; i < PORT_CTR_NUM(unit, port); i++) {
                ctr_ref = PORT_CTR_REG(unit, port, i);
                if (ctr_ref->reg != INDEX(INVALID_Rr)) {
                    SOC_IF_ERROR_RETURN
                        (_soc_robo_counter_set(unit, port, ctr_ref->reg, val));
                }
            }
        }
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_set32_by_port
 * Purpose:
 *      Sets all 64-bit software shadow counters for a given 
 *  set of ports to a given 32-bit value.
 * Parameters:
 *      unit - RoboSwitch unit number.
 *      pbm  - Port bitmap of the ports to set. Use PBMP_ALL for all ports.
 *      val  - 32-bit value to set the counters to.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_set32_by_port(int unit, pbmp_t pbmp, uint32 val)
{
    uint64          val64;

    COMPILER_64_SET(val64, 0, val);

    return soc_robo_counter_set_by_port(unit, pbmp, val64);
}

/*
 * Function:
 *      soc_robo_counter_set_by_reg
 * Purpose:
 *      For all ports, sets the specified 64-bit software 
 *  shadow counter to a given 64-bit value.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      ctr_reg - counter register to set.
 *      val     - 64-bit value to set the counters to.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_set_by_reg(int unit, soc_reg_t ctr_reg, uint64 val)
{
    soc_port_t  port, portNum;

    portNum = NUM_ALL_PORT(unit);/* fe|ge|xe|cmic(mii) */

    for (port = 0; port < portNum; port++) {
        SOC_IF_ERROR_RETURN(_soc_robo_counter_set(unit, port, ctr_reg, val));
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_set32_by_reg
 * Purpose:
 *      For all ports, sets the specified 64-bit software 
 *  shadow counter to a given 32-bit value.
 * Parameters:
 *      unit    - RoboSwitch unit number.
 *      ctr_reg - counter register to set.
 *      val     - 32-bit value to set the counters to.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_set32_by_reg(int unit, soc_reg_t ctr_reg, uint32 val)
{
    uint64          val64;

    COMPILER_64_SET(val64, 0, val);

    return soc_robo_counter_set_by_reg(unit, ctr_reg, val64);
}

/*
 * Function:
 *      soc_robo_counter_collect32
 * Purpose:
 *      This routine gets called each time the counter transfer has
 *      completed a cycle.  It collects the 32-bit counters.
 * Parameters:
 *      unit - uint number.
 *      discard - If true, the software counters are not updated; this
 *              results in only synchronizing the previous hardware
 *              count buffer.
 * Returns:
 *      SOC_E_XXX

 */
int
soc_robo_counter_collect(int unit, int discard)
{
    soc_control_t           *soc = SOC_CONTROL(unit);
    int             i, counter_index = 0;    
    uint32          ctr_new32, ctraddr;
    soc_port_t      port;
    uint64          ctr_new, ctr_prev, ctr_diff;
    soc_ctr_ref_t   *ctr_ref;
    int shift_port=0;
#ifdef BCM_DINO16_SUPPORT
    uint32          temp = 0;
#endif /* BCM_DINO16_SUPPORT */

    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        return _soc_tb_counter_collect(unit, discard);
#endif
    }
    
    PBMP_ITER(soc->counter_pbmp, port) {
        for (i = 0; i < PORT_CTR_NUM(unit, port); i++) {
            COUNTER_ATOMIC_DEF s = SOC_CONTROL(unit);
            ctr_ref = PORT_CTR_REG(unit, port, i);
            if (ctr_ref->reg == INDEX(INVALID_Rr)) {
                continue;
            }

            if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) || 
                SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                bcm5389a1_flag[unit] || SOC_IS_STARFIGHTER3(unit)) {
                /* 
                 * For BCM53115, CPU port (port 8, parsed from portbitmap)
                 * should change its value as 6 when calculate counter_index.
                 */
                if (port == 8) {
                    if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                        SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                        SOC_IS_STARFIGHTER3(unit)) {
                        shift_port = NUM_ALL_PORT(unit) - 1;
                    } else {
                        shift_port = 5;
                    }
                } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
                    /* Port 4 is not used in Starfighter3 */
                    shift_port = port - 1;
                } else {
                    shift_port = port;
                }

                counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                      (SOC_REG_INFO(unit, ctr_ref->reg).ctr_idx);
            } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
                       SOC_IS_NORTHSTARPLUS(unit)) {
                if (port >= 7) {
                	/* Port 6 is "no use" in Polar/Northstar */
                    shift_port = port - 1;
                } else {
                    shift_port = port;
                }
                counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                      (SOC_REG_INFO(unit, ctr_ref->reg).ctr_idx);
            } else if (SOC_IS_HARRIER(unit)){
                if (bcm53242_flag[unit] == SOC_BCM53202) {
                    if (port >= 24) {
                        shift_port = port-16;
                    } else {
                       shift_port = port;
                    }
                } else if (bcm53242_flag[unit] == SOC_BCM53212) {
                    if (port >= 24) {
                        shift_port = port-8;
                    } else {
                       shift_port = port;
                    }
                } else {
                   shift_port = port;
                }

                counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
                      (SOC_REG_INFO(unit, ctr_ref->reg).ctr_idx);
            } else {
                counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +
                      (SOC_REG_INFO(unit, ctr_ref->reg).ctr_idx);
            }
            ctraddr = DRV_REG_ADDR(unit, ctr_ref->reg, port, 0);

            ctr_prev = soc->counter_hw_val[counter_index];

            COMPILER_64_ZERO(ctr_new);

            if (SOC_IS_HARRIER(unit)|| SOC_IS_ROBO_ARCH_VULCAN(unit) ||
                SOC_IS_DINO8(unit)) {
                /* Read the counter value from H/W */
                switch (ctr_ref->reg) {
                case INDEX(TXOCTETSr):
                case INDEX(RXOCTETSr):
                case INDEX(RXGOODOCTETSr):
                case INDEX(TXQOS0OCTETSr):
                case INDEX(TXQOS1OCTETSr):
                case INDEX(TXQOS2OCTETSr):
                case INDEX(TXQOS3OCTETSr):
                case INDEX(TXQOSOCTETSr):
                case INDEX(RXQOSOCTETSr):
                    /* For the 64-bit counter */
                    SOC_IF_ERROR_RETURN(DRV_REG_READ
                        (unit, ctraddr, &ctr_new, 8));
                    break;
                default:
                    /* For the 32-bit counter */
                    SOC_IF_ERROR_RETURN(DRV_REG_READ
                        (unit, ctraddr, &ctr_new32, 4));
                    COMPILER_64_SET(ctr_new, 0, ctr_new32);
                }
#ifdef BCM_DINO16_SUPPORT
            } else if (SOC_IS_DINO16(unit)) {
                switch (ctr_ref->reg) {
                    case TXGOODPKTSr :
                    case TXUNICASTPKTSr:
                    case RXGOODPKTSr:
                    case RXUNICASTPKTSr:
                        temp = DINO16_MIB_SEL_G0;
                        break;                        
                    case TXCOLLISIONSr:
                    case TXOCTETSr:
                    case RXFCSERRORSr:
                    case RXGOODOCTETSr:
                        temp = DINO16_MIB_SEL_G1;
                        break;
                    case TXDROPPKTSr:
                    case TXPAUSEPKTSr:
                    case RXDROPPKTSr:
                    case RXOVERSIZEPKTSr:
                        temp = DINO16_MIB_SEL_G2;
                        break;
                    default:
                        temp = DINO16_MIB_SEL_G0;
                        break;
                }
                if (temp != (soc->counter_flags)) {
                    ctr_new32 = 0;
                } else {
                    SOC_IF_ERROR_RETURN(DRV_REG_READ
                        (unit, ctraddr, &ctr_new32, 4));
                }
                COMPILER_64_SET(ctr_new, 0, ctr_new32);
#endif /* BCM_DINO16_SUPPORT */
            }
            
            if (COMPILER_64_EQ(ctr_new, ctr_prev)) {
                COUNTER_ATOMIC_BEGIN(s);
                COMPILER_64_ZERO(soc->counter_delta[counter_index]);
                COUNTER_ATOMIC_END(s);
                continue;
            }

            if (discard) {
                COUNTER_ATOMIC_BEGIN(s);
                /* Update the previous value buffer */
                soc->counter_hw_val[counter_index] = ctr_new;
                COMPILER_64_ZERO(soc->counter_delta[counter_index]);
                COUNTER_ATOMIC_END(s);
                continue;
            }

            ctr_diff = ctr_new;

            if (COMPILER_64_LT(ctr_diff, ctr_prev)) {
                int             width;
                uint64          wrap_amt;

                /*
                 * Counter must have wrapped around.
                 * Add the proper wrap-around amount.
                 */
                width = SOC_REG_INFO(unit, ctr_ref->reg).fields[0].len;

                if (width < 32) {
                    COMPILER_64_SET(wrap_amt, 0, 1UL << width);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                } else if (width < 64) {
                    COMPILER_64_SET(wrap_amt, 1UL << (width - 32), 0);
                    COMPILER_64_ADD_64(ctr_diff, wrap_amt);
                }
            }

            COMPILER_64_SUB_64(ctr_diff, ctr_prev);

            COUNTER_ATOMIC_BEGIN(s);
            /* 
             * For ROBO chips, 
             * MIB counter TxPausePkts always counts both backpressure(half duplex) and
             * pause frames(full duplex). But this counter should not accumulate 
             * when duplex is half.
             * Thus, update SW counter table only when duplex is full.
             */
            if (ctr_ref->reg == INDEX(TXPAUSEPKTSr)) {
                uint32 fd = 0;
                DRV_PORT_GET(unit, port, DRV_PORT_PROP_DUPLEX, &fd);
                if (fd == DRV_PORT_STATUS_DUPLEX_FULL) {
                    COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
                    soc->counter_delta[counter_index] = ctr_diff;
                } else {
                    COMPILER_64_ZERO(soc->counter_delta[counter_index]);
                }
                soc->counter_hw_val[counter_index] = ctr_new;
            } else {
                COMPILER_64_ADD_64(soc->counter_sw_val[counter_index], ctr_diff);
                soc->counter_delta[counter_index] = ctr_diff;
                soc->counter_hw_val[counter_index] = ctr_new;
            }

            COUNTER_ATOMIC_END(s);

        }

        /*
         * Allow other tasks to run between processing each port.
         */
        sal_thread_yield();
    }

    return SOC_E_NONE;
}

static int counter_thread_run = 1;
/*
 * Starts or stops the counter thread
 */
void
soc_robo_counter_thread_run_set(int run)
{
    counter_thread_run = run;
}

/*
 * Function:
 *      soc_robo_counter_thread
 * Purpose:
 *      Master counter collection and accumulation thread.
 * Parameters:
 *      unit_vp - uint number (as a void *).
 * Returns:
 *      Nothing, does not return.
 */
void
soc_robo_counter_thread(void *unit_vp)
{
    int             unit = PTR_TO_INT(unit_vp);
    soc_control_t          *soc = SOC_CONTROL(unit);
    int             rv;
    int             interval;
    int         sync_gnt = FALSE;

    /*
     * There's a race condition since the PID is used as a
     * signal of whether the counter thread is running.  We sleep
     * here to make sure it gets initialized properly in SOC_CONTROL
     * by the thread_create return.
     */

    sal_sleep(1);
    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_robo_counter_thread: unit=%d\n"), unit));

    /*
     * Create a semaphore used to time the trigger scans.
     */

    soc->counter_notify = sal_sem_create("counter notify", sal_sem_BINARY, 0);

    if (soc->counter_notify == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_robo_counter_thread: semaphore init failed\n")));
        rv = SOC_E_INTERNAL;
        goto done;
    }

    while ((interval = soc->counter_interval) != 0) {

        /*
         * Use a semaphore timeout instead of a sleep to achieve the
         * desired delay between scans.  This allows this thread to exit
         * immediately when soc_robo_counter_stop wants it to.
         */

        LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                    (BSL_META_U(unit,
                                "soc_robo_counter_thread: sleep %d\n"), interval));

        (void)sal_sem_take(soc->counter_notify, interval);

        if (soc->counter_interval == 0) {   /* Exit signalled */
            break;
        }

	if ( !counter_thread_run )
	  continue;

        if (soc->counter_sync_req) {
            sync_gnt = TRUE;
        }

        /*
         * Add up changes to counter values.
         */
        soc->counter_coll_prev = soc->counter_coll_cur;
        soc->counter_coll_cur = sal_time_usecs();

        if ((rv = soc_robo_counter_collect(unit, 0)) < 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_robo_counter_thread: collect failed. \n")));
        }
        if (sync_gnt) {
            soc->counter_sync_req = 0;
            sync_gnt = 0;
        }
    }

    rv = SOC_E_NONE;

 done:
    if (rv < 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_robo_counter_thread: Operation failed - exiting\n")));
        soc_event_generate(unit, SOC_SWITCH_EVENT_THREAD_ERROR, 
                           SOC_SWITCH_EVENT_THREAD_COUNTER, __LINE__, rv);
    }


    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_robo_counter_thread: exiting\n")));

    soc->counter_pid = SAL_THREAD_ERROR;
    soc->counter_interval = 0;

    sal_thread_exit(0);
}

/*
 * Function:
 *      soc_robo_counter_start
 * Purpose:
 *      Perform counter MIB Autocast copying counters to memory.
 * Parameters:
 *      unit      - RoboSwitch unit number.
 *      flags    - Mode settings:
 *         0: Use polled mode for collecting statistics.
 *                 Others: SOC_COUNTER_F_DMA
 *      interval - Hardware counter scan interval in microseconds, maximum
 *         1048575.
 *      pbmp     - Port bitmap of the ports to monitor. Use PBMP_ALL for 
 *         all ports.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_start(int unit, uint32 flags, int interval, pbmp_t pbmp)
{
    soc_control_t   *soc = SOC_CONTROL(unit);
    char    pfmt[SOC_PBMP_FMT_LEN];
    sal_sem_t           sem;
#ifdef BCM_DINO16_SUPPORT
    uint32  temp, val, port;
    uint64  val64;
#endif /* BCM_DINO16_SUPPORT */

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_robo_counter_start: unit=%d flags=0x%x "
                         "interval=%d pbmp=%s\n"),
              unit, flags, interval, SOC_PBMP_FMT(pbmp, pfmt)));

    /*
     * If the following assert triggers, 'unsigned long long' must not
     * be 64 bits on your platform.  You may need to fix compiler.h to
     * the correct 64 bit type or disable COMPILER_HAS_LONGLONG.
     */

    assert(sizeof(uint64) == 8);

    /* Stop if already running */
    if (soc->counter_pid != SAL_THREAD_ERROR) {
        SOC_IF_ERROR_RETURN(soc_robo_counter_stop(unit));
    }

    /* Configure */
    sal_sprintf(soc->counter_name, "tCOUNTER.%d", unit);

    /* Create fresh semaphores */

    if ((sem = soc->counter_notify) != NULL) {
        soc->counter_notify = NULL;    /* Stop others from waking sem */
        sal_sem_destroy(sem);           /* Then destroy it */
    }
    soc->counter_notify =
        sal_sem_create("counter_notify", sal_sem_BINARY, 0);

    if (soc->counter_lock != NULL) {
        sal_spinlock_destroy(soc->counter_lock);
        soc->counter_lock = NULL;
    }
    soc->counter_lock = sal_spinlock_create("counter spinlock");
    if (soc->counter_lock == NULL) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "soc_robo_counter_start: lock create failed\n")));
        return SOC_E_INTERNAL;
    }

    soc->counter_pbmp = pbmp;
    soc->counter_flags = flags;

    /* Synchronize counter 'prev' values with current hardware counters */
    soc->counter_coll_prev = soc->counter_coll_cur = sal_time_usecs();

#ifdef BCM_DINO16_SUPPORT
    if (SOC_IS_DINO16(unit)) {
        /* Reset MIB Counter*/
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &val));
        temp = 1;
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
            (unit, &val, RST_MIB_CNTf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
            (unit, &val));

        /* Select MIB Mode */
        switch (soc->counter_flags) {
            case SOC_COUNTER_DINO16_GP0:
                temp = DINO16_MIB_SEL_G0;
                break;
            case SOC_COUNTER_DINO16_GP1:
                temp = DINO16_MIB_SEL_G1;
                break;
            case SOC_COUNTER_DINO16_GP2:
                temp = DINO16_MIB_SEL_G2;
                break;
            default:
                temp = DINO16_MIB_SEL_G0;
                break;
        }
        SOC_IF_ERROR_RETURN(REG_READ_MIB_SELECTr
            (unit, (uint32 *)&val64));
        PBMP_ITER(soc->counter_pbmp, port) {
            SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET(unit, MIB_SELECTr, 
                (uint32 *)&val64, dino16_mib_sel_field[port], &temp));
        }
        SOC_IF_ERROR_RETURN(REG_WRITE_MIB_SELECTr
            (unit, (uint32 *)&val64));
        
        /* Reset MIB Counter clear reset bit */
        SOC_IF_ERROR_RETURN(REG_READ_GMNGCFGr
            (unit, &val));
        temp = 0;
        SOC_IF_ERROR_RETURN(soc_GMNGCFGr_field_set
            (unit, &val, RST_MIB_CNTf, &temp));
        SOC_IF_ERROR_RETURN(REG_WRITE_GMNGCFGr
            (unit, &val));
    }
#endif /* BCM_DINO16_SUPPORT */

    SOC_IF_ERROR_RETURN(soc_robo_counter_collect(unit, 1));

    /*
     * The hardware timer can only be used for intervals up to about
     * 1.048 seconds.  This implementation uses a software timer instead
     * of the hardware timer.
     */
#ifdef MIB_SNAPSHOT
    if (SOC_IS_HARRIER(unit) || SOC_IS_ROBO_ARCH_VULCAN(unit) ||
        SOC_IS_TBX(unit)) {
        interval = 0;
    }
#endif

    soc->counter_interval = interval;

    if (interval) {
        soc->counter_pid =
            sal_thread_create(soc->counter_name,
                              SAL_THREAD_STKSZ,
                              50,
                              soc_robo_counter_thread, INT_TO_PTR(unit));

        if (soc->counter_pid == SAL_THREAD_ERROR) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_robo_counter_start: Thread create failed\n")));
            return (SOC_E_MEMORY);
        }

        LOG_INFO(BSL_LS_SOC_COUNTER,
                 (BSL_META_U(unit,
                             "soc_robo_counter_start: Complete\n")));
    } else {
        soc->counter_pid = SAL_THREAD_ERROR;
        LOG_INFO(BSL_LS_SOC_COUNTER,
                 (BSL_META_U(unit,
                             "soc_robo_counter_start: Inactive\n")));
    }

    return (SOC_E_NONE);
}


/*
 * Function:
 *  soc_robo_counter_sync
 * Purpose:
 *  Force an immediate counter update
 * Parameters:
 *  unit - uint number.
 * Returns:
 *  SOC_E_XXX
 * Notes:
 *  Ensures that ALL counter activity that occurred before the sync
 *  is reflected in the results of any soc_robo_counter_get()-type
 *  routine that is called after the sync.
 */

int
soc_robo_counter_sync(int unit)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
    soc_timeout_t   to;
    int         interval;
    uint32                 stat_sync_timeout =0;

    if ((interval = soc->counter_interval) == 0) {
        return SOC_E_DISABLED;
    }

    /* Trigger a collection */

    soc->counter_sync_req = TRUE;

    sal_sem_give(soc->counter_notify);

    stat_sync_timeout = 1000000 * NUM_ALL_PORT(unit);
    stat_sync_timeout = soc_property_get(unit,
                                         spn_BCM_STAT_SYNC_TIMEOUT,
                                         stat_sync_timeout);
    
    soc_timeout_init(&to, stat_sync_timeout, 0);
    /* 
     * The timeout value should be defined as meanningful for port number.
     */

    while (soc->counter_sync_req) {
        if (soc_timeout_check(&to)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "soc_robo_counter_sync: counter thread not responding\n")));
            soc->counter_sync_req = FALSE;
            return SOC_E_INTERNAL;
        }

        sal_usleep(10000);
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_status
 * Purpose:
 *      Get the status of counter collection, S/W accumulation process.
 * Parameters:
 *      unit - uint number.
 *      flags - SOC_COUNTER_F_xxx flags.
 *      interval - collection period in micro-seconds.
 *      pbmp - bit map of ports to collact counters on.
 * Returns:
 *      SOC_E_XXX
 */

int
soc_robo_counter_status(int unit, uint32 *flags, int *interval, pbmp_t *pbmp)
{
    soc_control_t       *soc = SOC_CONTROL(unit);
 
    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_counter_status: unit=%d\n"), unit));

    *interval = soc->counter_interval;
    *flags = soc->counter_flags;
    SOC_PBMP_ASSIGN(*pbmp, soc->counter_pbmp);

    return (SOC_E_NONE);
}


/*
 * Function:
 *      soc_robo_counter_stop
 * Purpose:
 *      Cancel counter MIB Autocast and software counter collection.
 * Parameters:
 *      unit - RoboSwitch unit number.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_stop(int unit)
{
    soc_control_t          *soc = SOC_CONTROL(unit);
    int             rv = SOC_E_NONE;
    soc_timeout_t   to;
    int             fail = 0;

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_robo_counter_stop: unit=%d\n"), unit));

    /* Stop thread if present.  It notices interval becoming 0. */
    soc->counter_interval = 0;

    if (soc->counter_pid != SAL_THREAD_ERROR) {

        /* Wake up thread to speed its exit */
        if (soc->counter_notify) {
            sal_sem_give(soc->counter_notify);
        }

        /* Give thread a few seconds to wake up and exit */
        soc_timeout_init(&to, 8000000, 0);
        /*
         * The timeout value should be defined meanningful for port number.
         */

        while (soc->counter_pid != SAL_THREAD_ERROR) {
            if (soc_timeout_check(&to)) {
                if (fail > 2) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "soc_robo_counter_stop: thread will not exit\n")));
                    rv = SOC_E_INTERNAL;
                    break;
                } else {
                    fail++;
                    soc_timeout_init(&to, 8000000, 0);
                }
            }
        }
    }

    if (soc->counter_lock != NULL) {
        sal_spinlock_destroy(soc->counter_lock);
        soc->counter_lock = NULL;
    }

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META_U(unit,
                         "soc_robo_counter_stop: stopped\n")));

    return (rv);
}

/*
 * Function:
 *      soc_robo_counter_attach
 * Purpose:
 *      Initialize counter module.
 * Notes:
 *      Allocates counter collection buffers.
 */
int
soc_robo_counter_attach(int unit)
{
    soc_control_t       *soc;
    int         n_bytes;
    soc_port_t  portNum;
    int                 blk, bindex,phy_port;

    assert(SOC_UNIT_VALID(unit));
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);

    soc->counter_pid = SAL_THREAD_ERROR;
    soc->counter_interval = 0;
    SOC_PBMP_CLEAR(soc->counter_pbmp);
    soc->counter_notify = NULL;
    /*soc->counter_flags = SOC_COUNTER_F_DMA;*/
    soc->counter_coll_prev = soc->counter_coll_cur = sal_time_usecs();

    portNum = 0;

    for (phy_port = 0; ; phy_port++) {
        blk = SOC_PORT_INFO(unit, phy_port).blk;
        bindex = SOC_PORT_INFO(unit, phy_port).bindex;
        if (blk < 0 && bindex < 0) {                    /* end of list */
            break;
        }
        portNum = phy_port;
    }
    portNum++;

    /* For Robo, we use the 64-bit s/w to store all h/w counters */
    n_bytes = 0;
    if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        n_bytes = portNum * TXRX_MAX_COUNTER_NUM(unit) * sizeof(uint64);
#endif
    } else {
        n_bytes = portNum * SOC_MAX_COUNTER_NUM(unit) * sizeof(uint64);
    }

    assert(n_bytes > 0);

    /* Prev buf */
    if (soc->counter_hw_val == NULL) {
        soc->counter_hw_val = sal_alloc(n_bytes, "cdma prev");
        if (soc->counter_hw_val == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_hw_val, 0, n_bytes);

    /* Value buf */
    if (soc->counter_sw_val == NULL) {
        soc->counter_sw_val = sal_alloc(n_bytes, "cdma val");
        if (soc->counter_sw_val == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_sw_val, 0, n_bytes);

    /* Delta buf */
    if (soc->counter_delta == NULL) {
        soc->counter_delta = sal_alloc(n_bytes, "cdma delta");
        if (soc->counter_delta == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_delta, 0, n_bytes);

    /* Value buf64 */
    if (soc->counter_buf64 == NULL) {
        soc->counter_buf64 = sal_alloc(n_bytes, "cdma buf64");
        if (soc->counter_buf64 == NULL) {
            goto error;
        }
    }
    sal_memset(soc->counter_buf64, 0, n_bytes);

    soc_robo_counter_cmap_init(unit);

    return SOC_E_NONE;

 error:

    if (soc->counter_hw_val != NULL) {
        sal_free(soc->counter_hw_val);
        soc->counter_hw_val = NULL;
    }

    if (soc->counter_delta != NULL) {
        sal_free(soc->counter_delta);
        soc->counter_delta = NULL;
    }

    if (soc->counter_sw_val != NULL) {
        sal_free(soc->counter_sw_val);
        soc->counter_sw_val = NULL;
    }

    if (soc->counter_buf64 != NULL) {
        sal_free(soc->counter_buf64);
        soc->counter_buf64 = NULL;
    }

    return SOC_E_MEMORY;
}

/*
 * Function:
 *      soc_robo_counter_detach
 * Purpose:
 *      Finalize counter module.
 * Notes:
 *      Stops counter task if running.
 *      Deallocates counter collection buffers.
 */
int
soc_robo_counter_detach(int unit)
{
    soc_control_t          *soc;

    assert(SOC_UNIT_VALID(unit));
    if (!SOC_UNIT_VALID(unit)) {
        return SOC_E_UNIT;
    }

    soc = SOC_CONTROL(unit);

    SOC_IF_ERROR_RETURN(soc_robo_counter_stop(unit));

    if (soc->counter_hw_val != NULL) {
        sal_free(soc->counter_hw_val);
        soc->counter_hw_val = NULL;
    }

    if (soc->counter_delta != NULL) {
        sal_free(soc->counter_delta);
        soc->counter_delta = NULL;
    }

    if (soc->counter_sw_val != NULL) {
        sal_free(soc->counter_sw_val);
        soc->counter_sw_val = NULL;
    }

    if (soc->counter_buf64 != NULL) {
        sal_free(soc->counter_buf64);
        soc->counter_buf64 = NULL;
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_prev_get
 * Purpose:
 *      Given a counter register number, port number, and a counter
 *      value, get the software counters(counter_buf64) for previous 
 *      S/W counter value for diag.
 * Parameters:
 *      unit - uint number.
 *      port - port number.
 *      ctr_reg - counter register to retrieve.
 *      val - 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_prev_get(int unit, int port, soc_reg_t ctr_reg, uint64 *val)
{
    soc_control_t           *soc;
    int             counter_index = 0;    
    COUNTER_ATOMIC_DEF  s = SOC_CONTROL(unit);
    int  shift_port = 0;

    soc = SOC_CONTROL(unit);
    
    /* The input register is not counter register */
    if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
        _soc_robo_counter_illegal(unit, ctr_reg);
        return SOC_E_NONE;
    }

    if ((SOC_IS_ROBO(unit) && (SOC_IS_VULCAN(unit) || 
        SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_BLACKBIRD2(unit))) || bcm5389a1_flag[unit] ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* 
         * For BCM53115, CPU port (port 8, parsed from portbitmap)
         * should change its value as 6 when calculate counter_index.
         */
        if (port == 8) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                shift_port = NUM_ALL_PORT(unit) - 1;  
            } else {
                shift_port = 5;
            }
        } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
            /* Port 4 is not used in Starfighter3 */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) ||
               SOC_IS_NORTHSTARPLUS(unit)) {
        if (port >= 7) {
            /* Port 6 is "no use" in Polar/Northstar */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if (bcm53242_flag[unit] == SOC_BCM53202) {
            if (port >= 24) {
                shift_port = port-16;
            } else {
               shift_port = port;
            }
        } else if (bcm53242_flag[unit] == SOC_BCM53212) {
            if (port >= 24) {
                shift_port = port-8;
            } else {
               shift_port = port;
            }
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);
#endif
    } else {
        counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    }

    COUNTER_ATOMIC_BEGIN(s);
    *val = soc->counter_buf64[counter_index];
    COUNTER_ATOMIC_END(s);

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_robo_counter_prev_set
 * Purpose:
 *      Given a counter register number, port number, and a counter
 *      value, set the software counters(counter_buf64) to the value
 *      to record previous S/W counter value for diag.
 * Parameters:
 *      unit - uint number.
 *      port - port number.
 *      ctr_reg - counter register to retrieve.
 *      val - 64/32-bit counter value.
 * Returns:
 *      SOC_E_XXX
 */
int
soc_robo_counter_prev_set(int unit, int port, soc_reg_t ctr_reg, uint64 val)
{
    soc_control_t           *soc;
    int             counter_index = 0;    
    COUNTER_ATOMIC_DEF  s = SOC_CONTROL(unit);
    int  shift_port = 0;

    soc = SOC_CONTROL(unit);
    
    /* The input register is not counter register */
    if (!SOC_REG_IS_COUNTER(unit, ctr_reg)) {
        _soc_robo_counter_illegal(unit, ctr_reg);
        return SOC_E_NONE;
    }

    if ((SOC_IS_ROBO(unit) && (SOC_IS_VULCAN(unit) || 
        SOC_IS_LOTUS(unit) || SOC_IS_STARFIGHTER(unit) ||
        SOC_IS_BLACKBIRD2(unit))) || bcm5389a1_flag[unit] ||
        SOC_IS_STARFIGHTER3(unit)) {
        /* 
         * For BCM53115, CPU port (port 8, parsed from portbitmap)
         * should change its value as 6 when calculate counter_index.
         */
        if (port == 8) {
            if (SOC_IS_VULCAN(unit) || SOC_IS_LOTUS(unit) ||
                SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit) ||
                SOC_IS_STARFIGHTER3(unit)) {
                shift_port = NUM_ALL_PORT(unit) - 1;
            } else {
                shift_port = 5;
            }
        } else if ((port == 5) && SOC_IS_STARFIGHTER3(unit)) {
            /* Port 4 is not used in Starfighter3 */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
               SOC_IS_NORTHSTARPLUS(unit)) {
        if (port >= 7) {
            /* Port 6 is "no use" in Polar/Northstar */
            shift_port = port - 1;
        } else {
            shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
        if (bcm53242_flag[unit] == SOC_BCM53202) {
            if (port >= 24) {
                shift_port = port-16;
            } else {
               shift_port = port;
            }
        } else if (bcm53242_flag[unit] == SOC_BCM53212) {
            if (port >= 24) {
                shift_port = port-8;
            } else {
               shift_port = port;
            }
        } else {
           shift_port = port;
        }
        counter_index = (shift_port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    } else if (SOC_IS_TBX(unit)) {
#ifdef BCM_TB_SUPPORT
        counter_index = soc_robo_counter_idx_get(unit, ctr_reg, port);
#endif
    } else {
        counter_index = (port * SOC_MAX_COUNTER_NUM(unit)) +
              (SOC_REG_INFO(unit, ctr_reg).ctr_idx);
    }

    COUNTER_ATOMIC_BEGIN(s);
    soc->counter_buf64[counter_index] = val;
    COUNTER_ATOMIC_END(s);

    return SOC_E_NONE;
}
