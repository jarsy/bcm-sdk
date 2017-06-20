/*
 * $Id: chipc_i2c.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _chipc_i2c_h_
#define _chipc_i2c_h_

#include <sbchipc.h>

/* QuickTure PC test */
#ifdef CFG_QUICKTURN
/* QTPC allows quite limited frequency range.  
 * >> i2c_ccr at 0x43 : m=8,n=3, clock ->1.04MHz
 */
#define QTPC_KSI2C_CCR      0x43
#define QTPC_KSI2C_CLK_KHZ  1041     /* counted from QTPC_KSI2C_CCR */
#endif      /* CFG_QUICKTURN */

/* Bus Controller flag bit values and their meanings */
#define KS_I2C_BUS_ATTACHED     0x01 /* attached status */
#define KS_I2C_BUS_ENABLED      0x02 /* enabled status */
#define KS_I2C_BUS_INTR         0x04 /* Interrupt mode */
#define KS_I2C_BUS_PIO          0x08 /* Programmed IO mode (Polling mode) */

/* define for the usage to read ChipCommon i2c related register through 
 *  SI interfaces.
 */
#define CCI2C_READREG_MASK      0x0

typedef uint8 i2c_addr_t; 
#define I2C_SADDR_ADDR_MASK     0xfe
#define I2C_SADDR_ADDR_OFFSET   1
#define I2C_SADDR_OP_MASK       0x01
#define I2C_OP_READ     1
#define I2C_OP_WRITE    0

#define CCI2C_CCR_MMAX      0xF     /* max m value for i2c clock(CCR) */
#define CCI2C_CCR_NMAX      0x7     /* max n value for i2c clock(CCR) */

/* this definition is the same ESW chip, SW effort is requirred to tell 
 *      which is working at master mode.
 *
 *  Note :
 *  1. in those I2C bus connected devices, the slave address on each 
 *      device shoud be unique.
 *  2. to avoid some reserved I2C address, the KSI2C slave address is 
 *      set to 0x10.
 *  3. default reset slave address is 0x44 (on both bus)
 */
#define KSI2C_SLAVE_ADDR_BASE  0x10

/*
 * I2C Bus Status codes:
 *
 * The 31 possible status codes of the Keystone I2C port
 * register bits: 0-7, enumerated for namespace reasons. Note that
 * all of the status codes are multiples of 8.
 */
typedef enum ks_i2c_status_e {
    KS_I2C_BUSERR = CC_I2CSTS_BUS_ERR,
    KS_I2C_START_TX = CC_I2CSTS_TX_MSTART,
    KS_I2C_REP_START_TX = CC_I2CSTS_TX_MRSTART,
    KS_I2C_ADDR_WR_BIT_TX_ACK_RX = CC_I2CSTS_TX_MWADDR_ACK,
    KS_I2C_ADDR_WR_BIT_TX_NO_ACK_RX = CC_I2CSTS_TX_MWADDR_NACK,
    KS_I2C_DATA_BYTE_TX_ACK_RX = CC_I2CSTS_TX_MDATA_ACK,
    KS_I2C_DATA_BYTE_TX_NO_ACK_RX = CC_I2CSTS_TX_MDATA_NACK,
    KS_I2C_ARB_LOST = CC_I2CSTS_ABT_LOST,
    KS_I2C_ADDR_RD_BIT_TX_ACK_RX = CC_I2CSTS_TX_MRADDR_ACK,
    KS_I2C_ADDR_RD_BIT_TX_NO_ACK_RX = CC_I2CSTS_TX_MRADDR_NACK,
    KS_I2C_DATA_BYTE_RX_ACK_TX = CC_I2CSTS_RX_MDATA_ACK,
    KS_I2C_DATA_BYTE_RX_NO_ACK_TX = CC_I2CSTS_RX_MDATA_NACK,
    KS_I2C_SADDR_RX_WR_BIT_RX_ACK_TX = CC_I2CSTS_RX_WADDR_ACK,
    KS_I2C_ARB_LOST_SADDR_RX_WR_BIT_RX_ACK_TX = CC_I2CSTS_ABT_LOST1_ACK,
    KS_I2C_GC_ADDR_RX_ACK_TX = CC_I2CSTS_RX_GENC_ACK,
    KS_I2C_ARB_LOST_GC_ADDR_RX_ACK_TX = CC_I2CSTS_ABT_LOST2_ACK,
    KS_I2C_DATA_BYTE_RX_AFTER_SADDR_RX_ACK_TX = CC_I2CSTS_RX_ADDRDATA_ACK,
    KS_I2C_DATA_BYTE_RX_AFTER_SADDR_RX_NO_ACK_TX = CC_I2CSTS_RX_ADDRDATA_NACK,
    KS_I2C_DATA_BYTE_RX_AFTER_GC_ADDR_RX_ACK_TX = CC_I2CSTS_RX_GENCDATA_ACK,
    KS_I2C_DATA_BYTE_RX_AFTER_GC_ADDR_RX_NO_ACK_TX = CC_I2CSTS_RX_GENCDATA_NACK,
    KS_I2C_STOP_OR_REP_START_COND_RX_IN_SLAVE_MODE = CC_I2CSTS_RX_SSTOP_SRSTART,
    KS_I2C_SADDR_RX_RD_BIT_RX_ACK_TX = CC_I2CSTS_RX_SRADDR_ACK,
    KS_I2C_ARB_LOST_IN_ADDR_PHASE_SADDR_RX_RD_BIT_RX_ACK_TX = CC_I2CSTS_ABT_LOST3_ACK,
    KS_I2C_SM_DATA_BYTE_TX_ACK_RX = CC_I2CSTS_TX_SDATA_ACK,
    KS_I2C_SM_DATA_BYTE_TX_NO_ACK_RX = CC_I2CSTS_TX_SDATA_NACK,
    KS_I2C_SM_LAST_BYTE_TX_ACK_RX = CC_I2CSTS_TX_SLASTDATA_ACK,
    KS_I2C_2ND_ADDR_BYTE_TX_WR_BIT_TX_ACK_RX = CC_I2CSTS_TX_MW2NDADDR_ACK,
    KS_I2C_2ND_ADDR_BYTE_TX_WR_BIT_TX_NO_ACK_RX = CC_I2CSTS_TX_MW2NDADDR_NACK,
    KS_I2C_2ND_ADDR_BYTE_TX_RD_BIT_TX_ACK_RX = CC_I2CSTS_TX_MR2NDADDR_ACK,
    KS_I2C_2ND_ADDR_BYTE_TX_RD_BIT_TX_NO_ACK_RX = CC_I2CSTS_TX_MR2NDADDR_NACK,
    KS_I2C_UNDEFINED = 0xF0, /* Not defined, for symmetry only */
    KS_I2C_NO_STATUS = CC_I2CSTS_NO_STATUS,
    KS_I2C_NUM_STATUS_CODES /* Always last please */
} ks_i2c_status_t;

/*
 * CPU/Master Initiated actions.
 */
typedef enum ks_i2c_op_e {
    KS_I2C_IDLE,
    KS_I2C_START,
    KS_I2C_REP_START,
    KS_I2C_TX,
    KS_I2C_RX,
    KS_I2C_STOP,
    KS_I2C_PROBE
} ks_i2c_op_t;

/* keystone I2C bus controler */
typedef struct ks_i2c_bus_s {
    uint32 flags;               /* Bitmask of control flag : see below */
    uint32 master_addr;         /* Master address (reset default 0x44) */
    uint32 frequency;           /* Frequency */
    uint8 m_val;                /* M for frequency setting */
    uint8 n_val;                /* N for frequency setting */
    ks_i2c_op_t  opcode;       /* Current bus operation in progress */
    uint32  pio_retries;        /* Max number of times to sleep for IFLG=0 */
    uint32  iflg_polls;         /* Polls of IFLG on last operation (PIO)*/
    ks_i2c_status_t stat;      /* STAT: current state of bus */

}ks_i2c_bus_t;

#define KSI2C_BUS_ENABLED(_i2cbus)          \
            ((_i2cbus)->flags & KS_I2C_BUS_ENABLED )
#define KSI2C_BUS_OP_IDLE_CHECK(_i2cbus)    \
            ((_i2cbus)->opcode == KS_I2C_IDLE )

/* retry times on checking I2C IFLG */
#define KS_I2C_PIO_RETRY    1000000

/* KeyStone I2C speed mode */
#define KS_I2C_SPEED_STANDARD       100
#define KS_I2C_SPEED_FAST           400
#define KS_I2C_SPEED_HIGH           3400        /* Maximum i2c speed in latest i2c spec. */
#define KS_I2C_SPEED_DEFAULT        KS_I2C_SPEED_STANDARD

/* reutrn value for I2C driver */
#define KSI2C_ERR_NONE          0
#define KSI2C_ERR_TIMEOUT       -1
#define KSI2C_ERR_INTERNAL      -2
#define KSI2C_ERR_PARAM         -3
#define KSI2C_ERR_UNAVAIL       -4
#define KSI2C_ERR_UNKNOW        -5
#define KSI2C_ERR_BUS           -6  /* need RESET to release bus to idel*/

#define KSI2C_DEV_ADDR_MAX_BYTES    4

/* i2c register access level function prototype */
extern void *si_i2c_setcore(si_t *sih);
extern int si_i2c_select(si_t *sih, uint8 i2c_id, int en);
extern int si_i2c_ccint_enable(si_t *sih, uint8 i2c_id, bool en);
extern uint32 si_i2c_ccint_flag(si_t *sih, uint32 mask, uint32 val);
extern uint8 si_i2c_swreset(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val);
extern uint8 si_i2c_data(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val);
extern uint8 si_i2c_slaveaddr(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val);
extern uint8 si_i2c_control(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val);
extern int si_i2c_clock(si_t *sih, uint8 i2c_id, uint8 val);
extern int si_i2c_status(si_t *sih, uint8 i2c_id, uint8 mask, uint8 *val);
extern uint8 si_i2c_extaddr(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val);

/* i2c bus level function prototype */
extern int ksi2c_bus_set_freq(si_t *sih, cc_i2c_id_t id, uint32 speed_hz);
extern int ksi2c_bus_attach(si_t *sih, cc_i2c_id_t id,  uint32 flags, int speed_khz);
extern int ksi2c_bus_detach(si_t *sih, cc_i2c_id_t id);
extern int ksi2c_bus_enable(si_t *sih, cc_i2c_id_t id, int en);
extern int ksi2c_bus_nbytes_read(si_t *sih, cc_i2c_id_t id, uint8 *data, int *len, bool aack_last_byte);
extern int ksi2c_bus_nbytes_write(si_t *sih, cc_i2c_id_t id, uint8 *data, int *len);
extern int ksi2c_bus_repstart(si_t *sih, cc_i2c_id_t id, i2c_addr_t slave_addr, bool op);
extern void ksi2c_bus_reset(si_t *sih, cc_i2c_id_t id);
extern int ksi2c_bus_start(si_t *sih, cc_i2c_id_t id, i2c_addr_t slave_addr, bool op, bool rep);
extern uint8 ksi2c_bus_status(si_t *sih, cc_i2c_id_t id);
extern int ksi2c_bus_stop(si_t *sih, cc_i2c_id_t id);
extern int ksi2c_bus_wait(si_t *sih, cc_i2c_id_t id);

/* i2c high level function prototype */
extern int ksi2c_init(void);
extern int ksi2c_open(cc_i2c_id_t id, uint32 flags, int speed_khz);
extern int ksi2c_read(cc_i2c_id_t id, i2c_addr_t slave_addr, uint8 *data, int len);
extern int ksi2c_write(cc_i2c_id_t id, i2c_addr_t slave_addr, uint8 *data, int len);
extern int ksi2c_i2cdev_read(cc_i2c_id_t id, i2c_addr_t slave_addr, 
        uint8 *data, int len, uint32 dev_addr, int dev_addr_len);
extern int ksi2c_i2cdev_write(cc_i2c_id_t id, i2c_addr_t slave_addr, 
        uint8 *data, int len, uint32 dev_addr, int dev_addr_len);
extern int ksi2c_rw_pack(cc_i2c_id_t id);
extern int ksi2c_close(cc_i2c_id_t id);
extern void ksi2c_reset(cc_i2c_id_t id);
extern void ksi2c_test_dumpreg(void);
extern int ksi2c_test_busio(cc_i2c_id_t id, int op, int act, uint8 *val);

/* ------------ for GPIO QT test usage only */
extern void ksgpio_Init(void);
extern int ksgpio_InEventMaskSet(uint16 emask);
extern int ksgpio_InEventChk(uint16 *event);
extern int ksgpio_InGet(uint16 *val);
extern int ksgpio_OutSet(uint16 val);
extern int ksgpio_OutGet(uint16 *val);
extern int ksgpio_OutEnGet(uint16 *enmask);
extern int ksgpio_OutEnSet(uint16 enmask);
extern int ksgpio_CtrlGet(uint16 *ctrl);
extern int ksgpio_CtrlSet(uint16 ctrl);
extern int ksgpio_TimerEnableSet(uint16 enmask);
extern int ksgpio_TimerEnableGet(uint16 *enmask);
extern int ksgpio_TimerSet(uint16 on_cnt, uint16 off_cnt);
extern int ksgpio_TimerGet(uint16 *on_cnt, uint16 *off_cnt);
extern int ksgpio_PullGet(bool op, uint16 *val);
extern int ksgpio_PullSet(bool op, uint16 val);
extern void ksgpio_dumpreg(void);

extern int ksgpio_test_init(void);
extern int ksgpio_test_output(uint16 pin_mask);
extern int ksgpio_test_input(uint16 pin_mask);
#endif /* _chipc_i2c_h_ */

