/*
 * The general level I2C driver for KeyStone I2C interface.
 *
 * The I2C interfaces (two I2C I/O ports) in Keystone can support the I2C 
 * features are :
 *  1. Speed mode : 
 *      - Standard Mode (SM): 100 kbit/s
 *      - Fast Mode(FM) : 400 kbit/s
 *      - High-speed Mode(HM) : 3.4 mbit/s
 *  2. M/S mode : 
 *      - Master Mode : 
 *      - Slave Mode : HW supported but SW is not supported yet.
 *      - Mix Mode : In the case for Multi-Master connected on the I2C bus. 
 *                  SW is not supported yet.
 *  3. Addressing mode.
 *      -7 bits Mode : default supported.
 *      - 10 bits mode : HW supportd but SW is not supported yet.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: chipc_i2c.c,v 1.6 Broadcom SDK $
 */

#include <typedefs.h>
#include <osl.h>
#include <sbchipc.h>
#include <hndsoc.h>
#include <siutils.h>
#include <pcicfg.h>
#include <nicpci.h>
#include <bcmutils.h>
#include <bcmdevs.h>
#include <chipc_i2c.h>

#include "siutils_priv.h"

#define I2C_CTRL_IFLG_CLEAR(_ctrl)     \
        (_ctrl) = (CC_I2CCTRL_INTEN | CC_I2CCTRL_BUSEN)

#ifdef BCMDBG_ERR
#define	I2C_ERROR(args)	printf args
#else
#define	I2C_ERROR(args)
#endif	/* BCMDBG_ERR */

#ifdef BCMDBG
#define	I2C_MSG(args)	printf args
#else
#define	I2C_MSG(args)
#endif	/* BCMDBG */

static si_t *ksi2c_sih = NULL;
static ks_i2c_bus_t ksi2c_bus[CC_I2C_NUM_BUS];

#define KSI2C_INIT_CHK  \
    if (ksi2c_sih == NULL) {\
         I2C_MSG(("%s,I2C bus not init yet!\n", __FUNCTION__)); \
         return KSI2C_ERR_INTERNAL; \
    }

#ifndef KHZ_TO_HZ
#define KHZ_TO_HZ(n) ((n)*1000)
#endif  /* KHZ_TO_HZ */

/* definitions for the driver's verification stage :
 *  - These definitions will be the final driver style after all verification 
 *      on read chip is done. And these symbole must be removed then.
 *  1. KSI2C_BUS_DRIVER_READY
 */
#define KSI2C_BUS_DRIVER_READY          1

/* --------- I2C register access level driver --------- 
 *  The drivers in this level read/write i2c register directly.
 */
 
/* change logical "focus" to the i2c core for optimized access */
void *
si_i2c_setcore(si_t *sih)
{
    return (si_setcoreidx(sih, SI_CC_IDX));
}

/* enable the I2C I/O port */
int
si_i2c_select(si_t *sih, uint8 i2c_id, int en)
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint32  reg_val = 0, temp;
    chipcregs_t *cc;
    
    /* select the I2C port */
    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    }
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    origidx = si_coreidx(sih);

    cc = (chipcregs_t *)si_i2c_setcore(sih);
    ASSERT(cc);

    /* CHECK: if the register name is not changed */
    reg_val = R_REG(sii->osh, &cc->SERIAL_IO_SEL);
    temp = (i2c_id == CC_I2C_ID0) ? CC_SERIAL_IO_I2C0_MASK : 
            (i2c_id == CC_I2C_ID1) ? CC_SERIAL_IO_I2C1_MASK : 0;
    if (en){
        reg_val |= temp;
    } else {
        reg_val &= ~temp;
    }
    W_REG(sii->osh, &cc->SERIAL_IO_SEL, reg_val);

    /* restore the original index */
    si_setcoreidx(sih, origidx);

    INTR_RESTORE(sii, intr_val);
    return 0;
    
}

/* enable CC lever interrupt on the I2C I/O port */
int
si_i2c_ccint_enable(si_t *sih, uint8 i2c_id, bool en)
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint32  reg_val = 0, temp;
    chipcregs_t *cc;
    
    /* select the I2C port */
    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    }
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    origidx = si_coreidx(sih);

    cc = (chipcregs_t *)si_i2c_setcore(sih);
    ASSERT(cc);

    /* CHECK: if the register name is not changed */
    reg_val = R_REG(sii->osh, &cc->SERIAL_IO_INTMASK);
    temp = (i2c_id == CC_I2C_ID0) ? CC_SERIAL_IO_I2C0_MASK : 
            (i2c_id == CC_I2C_ID1) ? CC_SERIAL_IO_I2C1_MASK : 0;
    if (en){
        reg_val |= temp;
    } else {
        reg_val &= ~temp;
    }

    W_REG(sii->osh, &cc->SERIAL_IO_INTMASK, reg_val);

    /* restore the original index */
    si_setcoreidx(sih, origidx);

    INTR_RESTORE(sii, intr_val);
    return 0;
}

/* mask&set i2c_swreset bits */
uint8 
si_i2c_swreset(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val)
{
    uint regoff = 0;

    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
    } else {
        regoff = OFFSETOF(chipcregs_t, i2c_swreset);
        regoff |= CC_I2C_OFFSETOF(i2c_id);
    }
    
    return (si_corereg8(sih, SI_CC_IDX, regoff, mask, val));
}

/* mask&set i2c_data bits */
uint8
si_i2c_data(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val)
{
    uint regoff = 0;

    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    } else {
        regoff = OFFSETOF(chipcregs_t, i2c_data);
        regoff |= CC_I2C_OFFSETOF(i2c_id);
    }
    
    return (si_corereg8(sih, SI_CC_IDX, regoff, mask, val));
}

/* mask&set i2c_slaveaddr bits */
uint8
si_i2c_slaveaddr(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val)
{
    uint regoff = 0;

    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    } else {
        regoff = OFFSETOF(chipcregs_t, i2c_slaveaddr);
        regoff |= CC_I2C_OFFSETOF(i2c_id);
    }
    
    return (si_corereg8(sih, SI_CC_IDX, regoff, mask, val));
}


/* mask&set i2c_control bits */
uint8
si_i2c_control(si_t *sih, uint8 i2c_id, uint8 mask, uint8 val)
{
    uint regoff = 0;

    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    } else {
        regoff = OFFSETOF(chipcregs_t, i2c_control);
        regoff |= CC_I2C_OFFSETOF(i2c_id);
    }
    
    return (si_corereg8(sih, SI_CC_IDX, regoff, mask, val));
}

/* mask&set i2c_sts_ccr bits.
 *  - status register is read only.
 *  - Clock_Control_Reg is write only.
 * 
 * Note :
 *  1. read CCR is not allowed.
 */
int
si_i2c_clock(si_t *sih, uint8 i2c_id, uint8 val)
{
    uint    origidx = 0;
    uint    intr_val = 0;

    si_info_t *sii;
    chipcregs_t *cc;

    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));    
        return -1;
    }
    
    sii = SI_INFO(sih);

    INTR_OFF(sii, intr_val);
    
    /* save current core index */
    origidx = si_coreidx(&sii->pub);

    cc = (chipcregs_t *)si_i2c_setcore(sih);
    ASSERT(cc);

    val &=  CC_I2CCCR_MASK;

    if (i2c_id == CC_I2C_ID1){
        W_REG(sii->osh, &cc->i2c1_sts_ccr, val);
    } else {
        W_REG(sii->osh, &cc->i2c_sts_ccr, val);
    }

    /* restore the original index */
    si_setcoreidx(sih, origidx);

    INTR_RESTORE(sii, intr_val);

    return 0;
    
}

/* mask&set i2c_sts_ccr bits.
 *  - status register is read only.
 *  - Clock_Control_Reg is write only.
 */
int
si_i2c_status(si_t *sih, uint8 i2c_id, uint8 mask, uint8 *val )
{
    si_info_t *sii;
    uint    origidx;
    uint    intr_val = 0;
    uint8    reg_val = 0;
    chipcregs_t *cc;

    /* select the I2C port */
    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    }
    
    sii = SI_INFO(sih);
    INTR_OFF(sii, intr_val);
    
    /* save current core index */
    origidx = si_coreidx(sih);

    cc = (chipcregs_t *)si_i2c_setcore(sih);
    ASSERT(cc);

    if (i2c_id == CC_I2C_ID1){
        reg_val = R_REG(sii->osh, &cc->i2c1_sts_ccr);
    } else {
        reg_val = R_REG(sii->osh, &cc->i2c_sts_ccr);
    }

    *val = (uint8) (reg_val & mask);
    
    /* restore the original index */
    si_setcoreidx(sih, origidx);
    
    INTR_RESTORE(sii, intr_val);
    
    return 0;
}

/* mask&set i2c_extaddr bits */
uint8
si_i2c_extaddr (si_t *sih, uint8 i2c_id, uint8 mask, uint8 val)
{
    uint regoff = 0;

    /* prevent the invalid extented address in val */
    val =  CC_I2CEXTADDR_PREFIX | (val & CC_I2CEXTADDR_MASK);

    if (!CC_I2C_ID_IS_VALID(i2c_id)){
        SI_ERROR(("%s: invalid i2c_id\n", __FUNCTION__));
        return -1;
    } else {
        regoff = OFFSETOF(chipcregs_t, i2c_extaddr);
        regoff |= CC_I2C_OFFSETOF(i2c_id);
    }
    
    return (si_corereg8(sih, SI_CC_IDX, regoff, mask, val));
}


/* --------- I2C Bus level(Low Level) driver --------- 
 *  The drivers in this level will combine the Keystone's register 
 *      configuration to proceed the requesting process.
 *  - interface between I2C Core driver and High level driver.
 */

/* Function : ksi2c_bus_attach 
 *  - Enable Keystone's I2C port.
 * Return :
 * Note :
 */
int  
ksi2c_bus_attach(si_t *sih, cc_i2c_id_t id,  uint32 flags, int speed_khz) 
{
    ks_i2c_bus_t   *i2cbus;
    uint32 speed_hz;
    uint8   i2creg_val, bus_status;

    I2C_MSG(("%s,id=%d,flag=0x%x,speed(khz)=%d\n", 
            __FUNCTION__, id, flags, speed_khz));
    ASSERT(sih);

    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }
    i2cbus = ksi2c_bus + id;    
    
    if (i2cbus->flags & KS_I2C_BUS_ATTACHED){
        return KSI2C_ERR_INTERNAL;
    }
    I2C_MSG(("%s, current flag =0x%x\n", __FUNCTION__, i2cbus->flags));
    I2C_MSG(("%s, oldspeed=%d newspeed=%d\n", 
            __FUNCTION__, i2cbus->frequency, KHZ_TO_HZ(speed_khz)));

    /* Choose one or the other IO mode, default
     * to PIO driven
     */
    if ((flags & KS_I2C_BUS_INTR) == ((flags & KS_I2C_BUS_PIO))) {
        /* both INTR and PIO is set or not set >> use polling mode */
        i2cbus->flags = KS_I2C_BUS_PIO;
    } else {
        i2cbus->flags = flags & (KS_I2C_BUS_INTR | KS_I2C_BUS_PIO);
    }

    /* Number of PIO's (IFLG/ACK) */
    i2cbus->pio_retries = KS_I2C_PIO_RETRY;

    /* Use default speed if zero or bad value specified */
    if ( (speed_khz <= 0) || (speed_khz > KS_I2C_SPEED_HIGH) ) {
            speed_khz = KS_I2C_SPEED_DEFAULT;
    }
    speed_hz = KHZ_TO_HZ(speed_khz);
    
    /* 1) set I2C clock */
    if (ksi2c_bus_set_freq(sih, id, speed_hz) ) {
        I2C_ERROR(("%s, failed on setting clock at I2C_%d!\n",__FUNCTION__, id));
        return KSI2C_ERR_INTERNAL;
    }

    /* set slave addr */
    i2creg_val = si_i2c_slaveaddr(sih, (uint8)id, CC_I2CSADDR_ADDR_MASK, 
            (KSI2C_SLAVE_ADDR_BASE | id) << CC_I2CSADDR_ADDR_OFFSET );
    i2cbus->master_addr = i2creg_val;
    I2C_MSG(("%s,I2C_%d set to slave-addr=0x%x(%x)\n", 
            __FUNCTION__, id, 
            (KSI2C_SLAVE_ADDR_BASE | id) << CC_I2CSADDR_ADDR_OFFSET, 
            i2cbus->master_addr));
    
    /* update bus op status */
    i2cbus->opcode = KS_I2C_PROBE;

    bus_status = ksi2c_bus_status(sih, id);
    if (bus_status == CC_I2CSTS_NO_STATUS){
        i2cbus->opcode = KS_I2C_IDLE;
    }
    i2cbus->flags |= KS_I2C_BUS_ATTACHED;
    
    return KSI2C_ERR_NONE;

}

/* Function : ksi2c_bus_enable 
 *  - Enable Keystone's I2C port.
 * Return :
 * Note :
 */
int 
ksi2c_bus_enable(si_t *sih, cc_i2c_id_t id, int en) 
{
    ks_i2c_bus_t   *i2cbus;
    int retry;

    ASSERT(sih);
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }
    
    i2cbus = ksi2c_bus + id;    

    if (KSI2C_BUS_ENABLED(i2cbus)){
        if (en) {
            I2C_MSG(("Msg: %s, I2C_%d enabled already!\n", __FUNCTION__, id));
            return KSI2C_ERR_NONE;;
        }
    } else {
        if (!en) {
            I2C_MSG(("Msg: %s, I2C_%d disabled already!\n", __FUNCTION__, id));
            return KSI2C_ERR_NONE;;
        }
    }

    /* check if the bus is not reserved and the i2c OP is IDLE */
    for (retry = 0; retry < i2cbus->pio_retries; retry++){
        if (KSI2C_BUS_OP_IDLE_CHECK(i2cbus)) {
            break;
        }
    }
        
    if (retry == i2cbus->pio_retries) {
        I2C_ERROR(("%s, Timeout on %s I2C_%d!\n", 
                    __FUNCTION__, (en) ? "Enabling" : "Disabling", id));
        return KSI2C_ERR_TIMEOUT;
    }

    /* select the i2c interface */
    if (si_i2c_select(sih, (uint8)id, (en) ? TRUE : FALSE)){
        I2C_ERROR(("%s, Failed on %s I2C_%d !", 
                    __FUNCTION__, (en) ? "Enabling" : "Disabling", id));
        return KSI2C_ERR_INTERNAL;
    }

    /* check if the Serial IO interrupt need to open here 
     *  - the interrupt flag will open the CC layer interrupt 
     */ 
    if (i2cbus->flags & KS_I2C_BUS_INTR) {
        if (si_i2c_ccint_enable(sih, (uint8)id, en)){
            I2C_ERROR(("%s, Failed on %s I2C_%d !", 
                        __FUNCTION__, (en) ? "Enabling" : "Disabling", id));
            return KSI2C_ERR_INTERNAL;
        }
    }
    if (en) {
        i2cbus->flags |= KS_I2C_BUS_ENABLED;
    } else {
        i2cbus->flags &= ~KS_I2C_BUS_ENABLED;
    }
    
    I2C_MSG(("DEBUG:%s,i2c_%d, enabling(%d) is DONE!\n", 
            __FUNCTION__, id, en));
    return KSI2C_ERR_NONE;
}

/* Function : ksi2c_bus_set_freq 
 *  - set Keystone's I2C frequency
 * Return :
 * Note :
 *  1. I2C clock is Keystone backplant
 *  2. the unit of the speed is khz.
 *  3. i2c_fre = sys_fre / ((M+1)*2 ^(N+1) * 10)
 *  4. System Clock will be retrieved from SI interface and a proper i2c 
 *      clock value will be auto-generated and selected to match user's 
 *      request frequency.
 *      - This routine will choose a pair of 'm' and 'n' for the closest i2c 
 *          clock. 
 */
int 
ksi2c_bus_set_freq(si_t *sih, cc_i2c_id_t id, uint32 speed_hz) 
{
    ks_i2c_bus_t   *i2cbus;
    uint8   ccr_val;
    uint    sys_clock, i2c_clock = 0;
    int     i;
    int     m, n;
    uint    i2c_clk_tmp = 0, diff0, diff1;
    int     found = 0;

    ASSERT(sih);
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }
    i2cbus = ksi2c_bus + id;

    /* check if the bus is not reserved and the i2c OP is IDLE */
    for (i = 0; i < i2cbus->pio_retries; i++){
        if (KSI2C_BUS_OP_IDLE_CHECK(i2cbus)) {
            break;
        }
    }
    
    if (i == i2cbus->pio_retries) {
        I2C_ERROR(("%s, Timeout on I2C_%d !\n", __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT;
    }
    
    /* -------- set i2c clock --------- */
    /* get Fsys */
    sys_clock = si_clock(sih);
    
    /* retrive the M and N value */
    diff0 = diff1 = 0;
    /* i2c clock counting formula  : i2c_fre = sys_fre/((M+1)*2^(N+1)*10) */
    for (n = 0; n < CCI2C_CCR_NMAX; n++){
        for (m = 0; m < CCI2C_CCR_MMAX; m++){
            i2c_clock = sys_clock / ((m + 1) * (1 << (n +1)) * 10);

            /* 1. Normally, the i2c clock will be decreased when the m is 
             *      increased.
             * 2. When N is increased, the counted i2c clock will less than
             *      the clock in previous loop. 
             * >> Those faster clock value items when N is just increased can
             *      be skip to fast the search process for the clock 
             *      resolution will be large than previous n loop.
             */
            if ((i2c_clk_tmp != 0) && (i2c_clock > i2c_clk_tmp)){
                /* i2c_clk_tmp!=0 means the previous clock is a valid clock */
                continue;
            }
            
            if (i2c_clock > speed_hz){
                diff0 = ABS(i2c_clock - speed_hz);
                i2c_clk_tmp = i2c_clock;
            } else {
                if (m == 0 && n == 0){
                    /* means all valid i2c clock are slower than user's 
                     * requested i2c clock!
                     */
                    I2C_ERROR(("%s, sys_clock=%d, user's i2c_clock=%d is too " 
                            "fast to be configurred!\n", 
                            __FUNCTION__, i2c_clock, speed_hz));
                    return KSI2C_ERR_PARAM;
                }
                diff1 = ABS(i2c_clock - speed_hz);
                
                found = 1;
                break;
            }
        }
        if (found) {
            break;
        }
    }

    if (found == 0){
        /* means all valid i2c clock are faster than user's 
         * requested i2c clock!
         */
        I2C_ERROR(("%s, sys_clock=%d, user's i2c_clock=%d is too slow to "
                "be configurred!\n", __FUNCTION__, i2c_clock, speed_hz));
        return KSI2C_ERR_PARAM;
    } else {
        /* here can select a better frequency setting :
         *  - choose the most close to user's request frequency 
         */
        if (diff0 < diff1){         /* previous i2c clock will be selected */
            if (m == 0){
                n--;
            } else {
                m--;
            }
            i2c_clock = i2c_clk_tmp;    /* set the clock back to previous */
        }
        
        i2cbus->m_val = m;
        i2cbus->n_val = n;
        ccr_val =((m << CC_I2CCCR_MSHIFT) & CC_I2CCCR_MMASK) | 
                        (n & CC_I2CCCR_NMASK);
    }

    /* set to i2c_ccr register */
    si_i2c_clock(sih, (uint8)id, ccr_val);

    /* keep the current working frequency */
    i2cbus->frequency = i2c_clock;    
    printf("I2C_Clock set to %d on i2c_bus%d\n", i2cbus->frequency, id);
    I2C_MSG(("DEBUG:%s, frequency=%d !\n", __FUNCTION__, i2cbus->frequency));
        
    return KSI2C_ERR_NONE;
}

/* Function : ksi2c_bus_wait_for_iflg_set 
 *  - wait the i2c interrupt flag.
 * Return :
 * Note :
 */
static int  
_ksi2c_bus_wait_for_iflg_set(si_t *sih, cc_i2c_id_t id) 
{
    ks_i2c_bus_t   *i2cbus;
    uint8   i2creg_val = 0;
    uint32 retry;

    ASSERT(sih);
    i2cbus = ksi2c_bus + id;
    retry = i2cbus->pio_retries;

    I2C_MSG(("%s,%d,Start..\n",__FUNCTION__,__LINE__));
    /* check if the i2c IFLG is set */
    while(!((i2creg_val = si_i2c_control
                (sih, (uint8)id, CCI2C_READREG_MASK, i2creg_val )) & 
                CC_I2CCTRL_INTFLAG) && --retry) {
        OSL_DELAY(1);
        i2creg_val = 0; /* clear for read through SI interface */
    }
    I2C_MSG(("%s,%d,i2creg_val=%x,retry=%d\n",
            __FUNCTION__,__LINE__,i2creg_val,retry));

    /* update the statistics to SW information */
    i2cbus->iflg_polls = ABS((int)i2cbus->pio_retries - (int)retry);
    
    return (retry > 0) ? KSI2C_ERR_NONE : KSI2C_ERR_TIMEOUT;
}

/* Function : ksi2c_bus_wait 
 *  - wait the i2c interrupt flag.
 * Return :
 * Note :
 */
int  
ksi2c_bus_wait(si_t *sih, cc_i2c_id_t id) 
{
    ks_i2c_bus_t   *i2cbus;
    uint8   i2creg_val = 0;
    int rv = KSI2C_ERR_NONE;

    ASSERT(sih);
    i2cbus = ksi2c_bus + id;    

    if (i2cbus->flags & KS_I2C_BUS_INTR) {
        /* wait interrupt till timeout */
        /* ####### TBD : enable interrupt ####### */
    } else {
        /* PIO : poll IFLG = 1 */
        rv = _ksi2c_bus_wait_for_iflg_set(sih, id);
    }

    /* get bus status */
    i2creg_val = ksi2c_bus_status(sih, id);
    
    I2C_MSG(("%s, current status=0x%x at I2C_%d\n",__FUNCTION__, i2creg_val,id));
    return rv;
    
}

/* Function : _ksi2c_bus_start_bit 
 *  - Request to send a I2C Start bit (internal usage)
 * Return :
 * Note :
 *  1. This is an single action routine and no error check is requirred. 
 *      The caller need to confirm if the bus status is proper for 
 *      sending start bit.
 */
static void 
_ksi2c_bus_start_bit(si_t *sih, cc_i2c_id_t id) 
{
    ks_i2c_bus_t   *i2cbus;
    uint8   ctrlbyte = 0;

    ASSERT(sih);
    i2cbus = ksi2c_bus + id;

    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte |= CC_I2CCTRL_MSTART;
        
    /* If in interrupt mode, make sure I2C interrupt is disabled. */
    if (i2cbus->flags & KS_I2C_BUS_INTR) {
        /* ####### TBD : disable interrupt ####### */
    }

    /* send start bit */
    ctrlbyte = si_i2c_control(sih, (uint8)id, CC_I2CCTRL_MASK,ctrlbyte );

    /* If in interrupt mode, make sure I2C interrupt is enabled. */
    if (i2cbus->flags & KS_I2C_BUS_INTR) {
        /* ####### TBD : enable interrupt ####### */
    }
    I2C_MSG(("%s,%d DONE!\n", __FUNCTION__, __LINE__));

}

/* Function : _ksi2c_bus_next_bus_phase 
 *  - Interrupt clear/next operation continue.  Clear the IFLG
 * to trigger or complete a CPU initiated transaction.  If tx_ack is
 * set, then an acknowledgement will be sent; usually tx_ack is only
 * relevant for receive operations.
 * Return :
 * Purpose: 
 * Note :
 *    The nature of the next bus phase depends on the current state
 *    of the I2C controller.

 */
static void  
_ksi2c_bus_next_bus_phase(si_t *sih, cc_i2c_id_t id, bool aack) 
{
    ks_i2c_bus_t   *i2cbus;
    uint8   ctrlbyte = 0;

    ASSERT(sih);
    i2cbus = ksi2c_bus + id;    
    ctrlbyte = si_i2c_control(sih, (uint8)id, CCI2C_READREG_MASK, ctrlbyte);
    
    /* clear the IFLG to start next phase */
    ctrlbyte &= ~CC_I2CCTRL_INTFLAG;

    /* AACK request */
    if (aack) {
        ctrlbyte |= CC_I2CCTRL_AAK;
    } else {
        ctrlbyte &= ~CC_I2CCTRL_AAK;
    }

    I2C_MSG(("%s,%d, set control to %x\n",__FUNCTION__,__LINE__,ctrlbyte));
    ctrlbyte = si_i2c_control(sih, (uint8)id, CC_I2CCTRL_MASK, ctrlbyte);

    /* if in the interrupt mode, re-enable the I2C interrupt */
    if (i2cbus->flags & KS_I2C_BUS_INTR) {
        /* ####### TBD : enable interrupt ####### */
    }
    
}

/* Function : ksi2c_bus_start 
 *  - Request to send a I2C Start.
 * Return :
 * Note :
 * 
 */
int 
ksi2c_bus_start(si_t *sih, cc_i2c_id_t id, i2c_addr_t slave_addr, 
                bool op, bool rep) 
{
    int rv;
    ks_i2c_bus_t   *i2cbus;
    ks_i2c_status_t correct_next_stat = KS_I2C_NO_STATUS;
    uint8   i2creg_val = 0;

    ASSERT(sih);
    ASSERT(CC_I2C_ID_IS_VALID(id));
    I2C_MSG(("%s, id=%d,saddr=0x%x,op=%d,repeat=%d\n", 
            __FUNCTION__, id, slave_addr, op, rep));

    i2cbus = ksi2c_bus + id;

    /* check if the bus is not reserved and the i2c OP is IDLE and the has 
     *  no bus event.
     */
    if (!rep){
        ASSERT(KSI2C_BUS_OP_IDLE_CHECK(i2cbus));
    }
    
    /* send START bit */
    _ksi2c_bus_start_bit(sih, id);

    if (rep) {
        i2cbus->opcode = KS_I2C_REP_START;
        correct_next_stat = KS_I2C_REP_START_TX;
    } else {
        i2cbus->opcode = KS_I2C_START;
        correct_next_stat = KS_I2C_START_TX;
    }

    if (ksi2c_bus_wait(sih, id)){
        /* timeout occurred */
        I2C_ERROR(("%s, TIMEOUT after START at I2C_%d\n", __FUNCTION__, id));
        rv = KSI2C_ERR_TIMEOUT;
        goto err_stop;
    } else {

        if (i2cbus->stat == correct_next_stat) {
            i2c_addr_t  saddr_internal;
            
            saddr_internal = (slave_addr << I2C_SADDR_ADDR_OFFSET) & 
                        I2C_SADDR_ADDR_MASK;
            /*
             * We generated start, now send the slave's bus address byte 
             * with OP code added.
             * (7-bit address mode only)
             */
            i2creg_val = saddr_internal | ((op == I2C_OP_READ) ? 
                    I2C_OP_READ : I2C_OP_WRITE);
            I2C_MSG(("%s,%d,saddr+op(%x)\n", __FUNCTION__, __LINE__,i2creg_val));
            i2creg_val = si_i2c_data(sih, (uint8)id, CC_I2CDATA_MASK, 
                    i2creg_val);

            _ksi2c_bus_next_bus_phase(sih, id, FALSE);
           
        } else {
            I2C_ERROR(("%s, unhandled status at I2C_%d\n", __FUNCTION__, id));
            rv = KSI2C_ERR_UNKNOW;
            goto err_stop;
        }
    }

   /*
    * Now, wait again for an interrupt. One of four possible
    * interrupts should occur:
    *
    * Ready for IO: (R/W)
    *    KS_I2C_ADDR_WR_BIT_TX_ACK_RX - Device ready for writing
    *    KS_I2C_ADDR_RD_BIT_TX_ACK_RX - Device ready for reading
    * No Device :
    *    KS_I2C_ADDR_WR_BIT_TX_NO_ACK_RX - No device ready
    *                                       for write
    *    KS_I2C_ADDR_RD_BIT_TX_NO_ACK_RX - No device ready
    *                                       for read
    */
   if ((rv = ksi2c_bus_wait(sih, id)) == KSI2C_ERR_NONE) {

        ks_i2c_status_t bus_status = ksi2c_bus_status(sih, id);
        
        if (bus_status == KS_I2C_ADDR_WR_BIT_TX_ACK_RX) {
            i2cbus->opcode = KS_I2C_TX;
            rv = KSI2C_ERR_NONE;
        } else if (bus_status == KS_I2C_ADDR_RD_BIT_TX_ACK_RX) {
            i2cbus->opcode = KS_I2C_RX;
            rv = KSI2C_ERR_NONE;
        } else if (bus_status == KS_I2C_ADDR_RD_BIT_TX_NO_ACK_RX ||
                bus_status == KS_I2C_ADDR_WR_BIT_TX_NO_ACK_RX) {
            I2C_MSG(("%s, i2c device (saddr=0x%02x) no response\n",
                    __FUNCTION__,  slave_addr));
            rv = KSI2C_ERR_TIMEOUT;
        } else {
           I2C_MSG(("%s, i2c device (saddr=0x%02x) no response\n",
                   __FUNCTION__,  slave_addr));
           rv = KSI2C_ERR_INTERNAL;
        }

   } else {
       I2C_MSG(("%s,TIMEOUT on retriving the bus status at I2C_%d\n",
                __FUNCTION__, id));
       rv = KSI2C_ERR_TIMEOUT;
   }

err_stop:
   if (rv != KSI2C_ERR_NONE) {
       /* Very important, if anything went wrong,
        * we MUST release the bus to return to idle state!
        * >> Caller need to handle the error return and issue STOP.
        */
       I2C_MSG(("%s, Need STOP to release bus to idle state!!\n", __FUNCTION__));
   }

   return rv;

}

/* Function : ksi2c_bus_repstart 
 *  - Request to send a I2C repeat Start.
 * Return :
 * Note :
 *  Caller need to issue STOP if error returned.
 */
int 
ksi2c_bus_repstart(si_t *sih, cc_i2c_id_t id, i2c_addr_t slave_addr, bool op) 
{
    return ksi2c_bus_start(sih, id, slave_addr, op, TRUE);
}

/* Function : _ksi2c_stop_bits 
 *  - Generate stop condition on the I2C Bus.
 *           This also recovers from an I2C bus error.
 * Return :
 * Note :
 */
static void  
_ksi2c_bus_stop_bits(si_t *sih, cc_i2c_id_t id) 
{
    ks_i2c_bus_t	*i2cbus;
    uint8   ctrlbyte = 0;

    ASSERT(sih);
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return;
    }
    i2cbus = ksi2c_bus + id;

    /*
     *  Clear Intr from any previous phase, and initiate STOP phase.
     *      IFLG will NOT set when the STOP phase has completed.
     */
    ctrlbyte = CC_I2CCTRL_BUSEN | CC_I2CCTRL_MSTOP;

    /* send stop bit */
    I2C_MSG(("%s,%d,set control(%x)\n",__FUNCTION__,__LINE__,ctrlbyte));
    ctrlbyte = si_i2c_control(sih, (uint8)id, CC_I2CCTRL_MASK,ctrlbyte );

    /* If in interrupt mode, leave I2C interrupt unmasked (enabled). */
    if ((i2cbus->flags & KS_I2C_BUS_INTR)) {
        /* ####### TBD : disable interrupt ####### */
    }
}

/* Function : ksi2c_bus_stop 
 *  - Generate stop condition on the I2C bus. This routine is
 *          used to signal the end of a data transfer and releases
 *          the bus according to the I2C protocol.
 * Return :
 * Note :
 */
int 
ksi2c_bus_stop(si_t *sih, cc_i2c_id_t id) 
{
    ks_i2c_bus_t   *i2cbus;
    
    ASSERT(sih);
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }
    
    i2cbus = ksi2c_bus + id;
    
    i2cbus->opcode = KS_I2C_STOP;
    _ksi2c_bus_stop_bits(sih, id);

    return KSI2C_ERR_NONE;
}

/* Function : ksi2c_bus_nbytes_read 
 *  - Request to read n bytes with or without ack at latest byte.
 * Parameters:
 *    data - address to place data byte received from slave
 *    len - (in)  number of bytes to read from slave.
 *          (out) number of bytes actually read.
 *    ack_last_byte -
 *             if set, an ACK will automatically be sent by the
 *             controller for the last byte read from the slave.
 *             If not set, a NAK is pulsed when the last byte
 *             has been received. Set when a master would like
 *             to signify that this block read is NOT the last
 *             data to be read from the slave.
 * Return :
 * Note :
 *  1. The ack_last_byte field only affects the last byte; this
 *       allows support for the following I2C byte read transactions:
 *
 *       START Addr Rd [ACK] [Data] ACK REPSTART ...
 *       START Addr Rd [ACK] [Data] NAK REPSTART ...
 *
 *       START Addr Rd [ACK] [Data] ACK [DATA] ACK STOP
 *       START Addr Rd [ACK] [Data] ACK [DATA] NAK STOP
 *
 *       We always transmit an ACK when two or more bytes
 *       remain to be read, i.e. the non-last byte(s).
 *
 *       Before this routine can be used, the I2C controller must
 *       be in a read-ready state, i.e. following a START-READ_SADDR
 *       phase or a previous read operation.
 */
int 
ksi2c_bus_nbytes_read(si_t *sih, cc_i2c_id_t id, uint8 *data, 
                int *len, bool aack_last_byte) 
{
    int aack;
    uint32 nread, nbytes;
    uint8* ptr;
    ks_i2c_bus_t   *i2cbus;
    uint8   i2creg_val = 0;
    ks_i2c_status_t s;

    ASSERT(sih);
    if (!len || (*len <= 0)) {
        return KSI2C_ERR_PARAM;
    }
    
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }

    i2cbus = ksi2c_bus + id;    /* Make sure we're in a read-ready state. */
    if (i2cbus->opcode != KS_I2C_RX) {
        return KSI2C_ERR_INTERNAL;
    }

    nbytes = *len;
    ptr = data;

    I2C_MSG(("%s,%d,STARTing to read %d bytes...\n", __FUNCTION__,__LINE__,*len));
    /* Read up to len bytes ... */
    *len = 0;

    /*
     * Some sort of start condition and slave address has been sent
     * by the I2C controller (master) and ACK'd by the slave device,
     * or one or more bytes have already been read.
     * The I2C controller is now in a state that will perform byte
     * reads until a stop or repeat start is explicitly initiated.
     * We'll only do the requested number of byte reads here, and
     * leave it up to the calling code to do the stop or start.
     */
    for (nread = 0; nread < nbytes; nread++) {
        /* ACK the byte we're about to read */
        if (!aack_last_byte) {
            aack = (nread == nbytes - 1) ? 0 : 1;
        } else {
            aack = 1;
        }
        I2C_MSG(("%s,%d,read byte_%d,aack=%d\n", 
                __FUNCTION__, __LINE__, nread,aack));

        /* Initiate the next byte read. */
        _ksi2c_bus_next_bus_phase(sih, id, aack);

        if (KSI2C_ERR_NONE!= ksi2c_bus_wait(sih, id)) {
            I2C_MSG(("%s, no IFLG at i2c_%d\n", __FUNCTION__, id));
            return KSI2C_ERR_TIMEOUT;
        } else {

            /* Store the read data byte, or deal with error condition. */
            if (((s=ksi2c_bus_status(sih, id)) == KS_I2C_DATA_BYTE_RX_ACK_TX) 
                    || (s == KS_I2C_DATA_BYTE_RX_NO_ACK_TX) ) {

                i2cbus->opcode = KS_I2C_RX;
                i2creg_val = si_i2c_data(
                        sih, (uint8)id, CCI2C_READREG_MASK, 0);

                *ptr++ = (uint8) i2creg_val ;
                *len = *len + 1;
            } else {
                return KSI2C_ERR_INTERNAL;
            }
        }
    } /* read nbytes bytes */

    return KSI2C_ERR_NONE;
}

/* Function : ksi2c_bus_byte_write 
 *  - Request to write n bytes to i2c device 
 * Parameters:
 *    data - address to place data byte received from slave
 *    len - (in)  number of bytes to write to slave.
 *          (out) number of bytes actually write.
 * Return :
 * Note :
 */
int 
ksi2c_bus_nbytes_write(si_t *sih, cc_i2c_id_t id, uint8 *data, int *len) 
{
    uint32 nwrite, nbytes;
    uint8   i2creg_val = 0;
    ks_i2c_bus_t   *i2cbus;
    ks_i2c_status_t s;

    ASSERT(sih);
    if (!len || (*len <= 0) || (data == NULL)) {
        return KSI2C_ERR_PARAM;
    }
    
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("%s,Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }

    i2cbus = ksi2c_bus + id;    /* Make sure we're in a write-ready state. */
    if (i2cbus->opcode != KS_I2C_TX) {
        I2C_ERROR(("%s,Improper OP state=%d\n",__FUNCTION__,i2cbus->opcode));
        return KSI2C_ERR_INTERNAL;
    }

    nbytes = *len;
    /* Write up to len bytes ... */
    *len = 0;

    /*
     * Some sort of start condition and slave address has been sent
     * by the I2C controller (master) and ACK'd by the slave device,
     * or one or more bytes have already been write.
     * The I2C controller is now in a state that will perform byte
     * writes until a stop or repeat start is explicitly initiated.
     * We'll only do the requested number of byte wrties here, and
     * leave it up to the calling code to do the stop or start.
     */
    for (nwrite = 0; nwrite < nbytes; nwrite++) {

        /* write byte to i2c device */
        i2creg_val = si_i2c_data(
                sih, (uint8)id, CC_I2CDATA_MASK, (uint8)data[nwrite]);

        _ksi2c_bus_next_bus_phase(sih, id, FALSE);

        /* wait for next tx byte */
        if (KSI2C_ERR_NONE!= ksi2c_bus_wait(sih, id)) {
            I2C_MSG(("%s, no IFLG at i2c_%d\n", __FUNCTION__, id));
            return KSI2C_ERR_TIMEOUT;
        } else {

            if ( (s=ksi2c_bus_status(sih, id)) == KS_I2C_DATA_BYTE_TX_ACK_RX){

                i2cbus->opcode = KS_I2C_TX;
                *len = *len + 1;

            } else {
                /* Standard allowed "KS_I2C_DATA_BYTE_TX_NO_ACK_RX" when the 
                 * latest byte was write. 
                 */
                if (s == KS_I2C_DATA_BYTE_TX_NO_ACK_RX){
                    if (nwrite == (nbytes - 1)){
                        I2C_MSG(("%s,No ACK on the latest byte write!\n", 
                                    __FUNCTION__));
                        *len = *len + 1;
                    } else {
                        I2C_MSG(("%s, No ACK after one byte write\n", 
                                    __FUNCTION__));
                        return KSI2C_ERR_INTERNAL;
                    }
                } else {
                    I2C_MSG(("%s, unexpected status after one byte write "
                            "at i2c_%d\n", __FUNCTION__, id));
                    return KSI2C_ERR_INTERNAL;
                }
            }
        }
        
    }

    return KSI2C_ERR_NONE;
}
  
/* Function : ksi2c_bus_status 
 *     - get i2c bus interrupt flags(31 events).
 * Return :
 *     - Return the current I2C status.
 * Note :
 */
uint8 
ksi2c_bus_status(si_t *sih, cc_i2c_id_t id) 
{
    uint8   i2c_status = 0;
    ks_i2c_bus_t   *i2cbus;

    ASSERT(sih);
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }
    
    i2cbus = ksi2c_bus + id; 

    if (si_i2c_status(sih, id, CC_I2CSTS_MASK, &i2c_status)){
        I2C_ERROR(("%s,I2C_%d bus error\n",__FUNCTION__, id));
        i2c_status = CC_I2CSTS_BUS_ERR;    /* value = 0 */
    }
    i2c_status &= CC_I2CSTS_MASK;
    i2cbus->stat = i2c_status;

    return i2c_status;

}
 
/* Function : ksi2c_bus_reset 
 *  -reset the Keystone's I2C bus to IDLE state.
 * Return :
 * Note :
 *  1. Any I2C transaction will be stopped
 */
void 
ksi2c_bus_reset(si_t *sih, cc_i2c_id_t id) 
{
    ASSERT(sih);
    si_i2c_swreset(sih, id, CC_I2CRESET_MASK, CC_I2CRESET_VAL);
    /* CHECK about the proper time for wait */
    OSL_DELAY(100);
}

/* Function : ksi2c_bus_detach 
 *  - Detach the Keystone's I2C bus handler.
 * Return :
 * Note :
 */
int  
ksi2c_bus_detach(si_t *sih, cc_i2c_id_t id) 
{
    ks_i2c_bus_t   *i2cbus;
    
    ASSERT(sih);
    if (!CC_I2C_ID_IS_VALID(id)){
        I2C_ERROR(("Invalid I2C Bus ID!\n",__FUNCTION__));
        return KSI2C_ERR_PARAM;
    }
    
    i2cbus = ksi2c_bus + id; 

    /* clear all assigned info on this i2c bus */
    bzero(i2cbus, sizeof(*i2cbus));

    return KSI2C_ERR_NONE;;
}


/* --------- I2C High level driver --------- 
 * The drivers in this level also know as general I2C driver. 
 *  - Request the I2C read/write on I2C device through I2C bus driver.
 * Function : ksi2c_init 
 * Function : ksi2c_open 
 * Function : ksi2c_write 
 * Function : ksi2c_read 
 * Function : ksi2c_rw_pack 
 * Function : ksi2c_close 
 */

/* Function : ksi2c_init 
 *  - init Keystone's I2C bus_0 and bus_1.
 * Return :
 * Note :
 *  1. disable all i2c buses.
 *  2. reset all i2c buses.
 */
int 
ksi2c_init(void) 
{
    int rv = KSI2C_ERR_NONE;
    int i2c_id;
    
#if KSI2C_BUS_DRIVER_READY
    ks_i2c_bus_t   *i2cbus;
#endif
    ksi2c_sih = si_kattach(SI_OSH);

#if KSI2C_BUS_DRIVER_READY

    for (i2c_id = 0; i2c_id < CC_I2C_NUM_BUS; i2c_id++){
        /* driver handler reset */
        i2cbus = ksi2c_bus + i2c_id;
        ASSERT(i2cbus);
        bzero(i2cbus, sizeof(*i2cbus));

        /* force bus reset to prevent any unexpected status on the bus */
        ksi2c_bus_reset(ksi2c_sih, i2c_id);

        /* disable I2C bus and mask off the relevant interrupt */
        rv |= ksi2c_bus_enable(ksi2c_sih, i2c_id, FALSE);

    }
#else   /* KSI2C_BUS_DRIVER_READY */


    for (i2c_id = 0; i2c_id < CC_I2C_NUM_BUS; i2c_id++){
        /* force bus reset to prevent any unexpected status on the bus */
        ksi2c_bus_reset(ksi2c_sih, i2c_id);

        /* disable the i2c interface */
        if (si_i2c_select(ksi2c_sih, (uint8)i2c_id, FALSE)){
            I2C_ERROR(("%s,Failed on disabling I2C_%d!", __FUNCTION__, i2c_id));
            rv |= KSI2C_ERR_INTERNAL;
        }
    }
#endif  /* KSI2C_BUS_DRIVER_READY */

    return rv;
}
 
/* Function : ksi2c_test_busio 
 *  - i2c bus I/O register read/write dirictly for debug usage.
 * Return : 
 * Note :
 *  1. act : 1 for Read, 0 for write.
 */
int  
ksi2c_test_busio(cc_i2c_id_t id, int op, int act, uint8 *val) 
{
    uint8   reg_val;
    uint8 act_mask;
    
    if (ksi2c_sih == NULL){
        printf("\n i2c not init yet! \n");
        return -1;
    }
    
    if (act == I2C_OP_READ) {
        *val = 0;   /* must be zero for read process */
    }

    switch (op){
    case 0 :    /* i2c_data */
        act_mask = (act) ? 0 : CC_I2CDATA_MASK;
        
        reg_val = si_i2c_data(ksi2c_sih, (uint8)id, act_mask, *val);
        if (act) {
            *val = reg_val;
        }
        return 0;
        break;
    case 1 :    /* i2c_control */
        act_mask = (act) ? 0 : CC_I2CSTS_MASK;
        
        reg_val = si_i2c_control(ksi2c_sih, (uint8)id, act_mask, *val);
        if (act) {
            *val = reg_val;
        }
        return 0;
        break;
    case 2 :    /* i2c_slaveaddr */
        act_mask = (act) ? 0 : 
                (CC_I2CSADDR_ADDR_MASK | CC_I2CSADDR_GENC_MASK);
        
        reg_val = si_i2c_slaveaddr(ksi2c_sih, (uint8)id, act_mask, *val);
        if (act) {
            *val = reg_val;
        }
        return 0;
        break;
    case 3 :    /* i2c_status (read only)*/
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &reg_val)){
            printf("\n %s %d, failed on getting status.\n",__FUNCTION__,__LINE__);
        }
        *val = reg_val;
        return 0;
        break;
    case 4 :    /* i2c_ccr (write only)*/
        reg_val = *val & CC_I2CCCR_MASK;
        return si_i2c_clock(ksi2c_sih, (uint8)id, reg_val);
        break;
    default :
        printf("%s,Unknow OP code!\n", __FUNCTION__);
        return -1;
    }
}
/* Function : ksi2c_test_dumpreg 
 *  - Dump Keystone I2C's related register in CC.
 * Return : 
 * Note :
 */
void  
ksi2c_test_dumpreg() 
{
    uint8   val, val2;
    uint32  val3;
    si_info_t   *sii;
    uint        intr_val = 0;
    uint        origidx;
    chipcregs_t *cc;
    
    if (ksi2c_sih == NULL){
        printf("\n i2c not init yet! \n");
        return;
    }
    sii = SI_INFO(ksi2c_sih);
    INTR_OFF(sii, intr_val);
    origidx = si_coreidx(ksi2c_sih);

    printf("\n --- i2c I/O cocntrol registers in ChipCommon --- \n");
    cc = (chipcregs_t *)si_i2c_setcore(ksi2c_sih);
    ASSERT(cc);

    val3 = R_REG(sii->osh, &cc->SERIAL_IO_INTMASK);
    printf("\n\t CCi2c_Intr_en = 0x%04x,  ", val3);

    val3 = R_REG(sii->osh, &cc->SERIAL_IO_SEL);
    printf("\n\t Serial_IO_Sel = 0x%04x, i2c_0(%d), i2c_1(%d)\n", 
            val3, (val3&0x1)>0, (val3&0x2)>0);
       
    printf("\n --- i2c bus_0 registers in ChipCommon --- \n");
    val = R_REG(sii->osh, &cc->i2c_slaveaddr);
    val2 = R_REG(sii->osh, &cc->i2c_data);
    printf("\n\t Slave_Addr = 0x%02x,  Data = 0x%02x\n", val, val2);
    val = R_REG(sii->osh, &cc->i2c_control);
    val2 = R_REG(sii->osh, &cc->i2c_sts_ccr);
    printf("\t Control = 0x%02x,  Status/CCR = 0x%02x\n", val, val2);
    val = R_REG(sii->osh, &cc->i2c_extaddr);
    val2 = R_REG(sii->osh, &cc->i2c_swreset);
    printf("\t Ext_Salve_addr = 0x%02x,  swreset = 0x%02x\n", 
            val, val2);

    printf("\n --- i2c bus_1 registers in ChipCommon --- \n");
    val = R_REG(sii->osh, &cc->i2c1_slaveaddr);
    val2 = R_REG(sii->osh, &cc->i2c1_data);
    printf("\n\t Slave_Addr = 0x%02x,  Data = 0x%02x\n", val, val2);
    val = R_REG(sii->osh, &cc->i2c1_control);
    val2 = R_REG(sii->osh, &cc->i2c1_sts_ccr);
    printf("\t Control = 0x%02x,  Status/CCR = 0x%02x\n", val, val2);
    val = R_REG(sii->osh, &cc->i2c1_extaddr);
    val2 = R_REG(sii->osh, &cc->i2c1_swreset);
    printf("\t Ext_Salve_addr = 0x%02x,  eset = 0x%02x\n --- Done ---\n", 
            val, val2);

    /* restore the original index */
    si_setcoreidx(ksi2c_sih, origidx);

    INTR_RESTORE(sii, intr_val);

}

/* Function : ksi2c_reset 
 *  - reset Keystone's I2C.
 * Return : 
 * Note :
 */
void  
ksi2c_reset(cc_i2c_id_t id) 
{
    if (ksi2c_sih == NULL){
        printf("\n%d i2c not init yet! \n", id);
        return;
    }
    ksi2c_bus_reset(ksi2c_sih, id);
}

/* Function : ksi2c_open 
 *  - open Keystone's I2C.
 * Return :
 * Note :
 */
int 
ksi2c_open(cc_i2c_id_t id, uint32 flags, int speed_khz) 
{
    KSI2C_INIT_CHK;
#if KSI2C_BUS_DRIVER_READY
    if (ksi2c_bus_attach(ksi2c_sih, id, flags, speed_khz)) {
        I2C_ERROR(("%s, Failed on attaching I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_INTERNAL;
    }

    if (ksi2c_bus_enable(ksi2c_sih, id, TRUE)) {
        I2C_ERROR(("%s, Failed on enabling I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_INTERNAL;
    }
    
#else   /* KSI2C_BUS_DRIVER_READY */
    
    /* select the i2c interface */
    if (si_i2c_select(ksi2c_sih, (uint8)id, TRUE)){
        I2C_ERROR(("%s, Failed on %s I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_INTERNAL;
    }

    /* set bus clock */
    uint32  m, n, speed_hz;
    uint8   feq;
    uint    sys_clock, i2c_clock = 0, i2c_clk_tmp = 0, diff0, diff1;
    int     found = 0;

    /* get Fsys */
    sys_clock = si_clock();

    /* Use default speed if zero or bad value specified */
    if ( (speed_khz <= 0) || (speed_khz > KS_I2C_SPEED_HIGH) ) {
            speed_khz = KS_I2C_SPEED_DEFAULT;
    }
    speed_hz = KHZ_TO_HZ(speed_khz);

    /* retrive the M and N value */
    diff0 = diff1 = 0;
    /* i2c clock counting formula  : i2c_fre = sys_fre / ((M+1)*2^(N+1)*10) */
    for (n = 0; n < CCI2C_CCR_NMAX; n++){
        for (m = 0; m < CCI2C_CCR_MMAX; m++){
            i2c_clock = sys_clock / ((m + 1) * (1 << (n +1)) * 10);

            /* 1. Normally, the i2c clock will be decreased when the m is 
             *      increased.
             * 2. When N is increased, the counted i2c clock will less than
             *      the clock in previous loop. 
             * >> Those faster clock value items when N is just increased can
             *      be skip to fast the search process for the clock 
             *      resolution will be large than previous n loop.
             */
            if ((i2c_clk_tmp != 0) && (i2c_clock > i2c_clk_tmp)){
                /* i2c_clk_tmp!=0 means the previous clock is a valid clock */
                continue;
            }
            
            if (i2c_clock > speed_hz){
                diff0 = ABS(i2c_clock - speed_hz);
                i2c_clk_tmp = i2c_clock;
            } else {
                if (m == 0 && n == 0){
                    /* means all valid i2c clock are slower than user's 
                     * requested i2c clock!
                     */
                    I2C_ERROR(("%s, sys_clock=%d, user's i2c_clock=%d is too " 
                            "fast to be configurred!\n", 
                            __FUNCTION__, i2c_clock, speed_hz));
                    return KSI2C_ERR_PARAM;
                }
                diff1 = ABS(i2c_clock - speed_hz);
                
                found = 1;
                break;
            }
        }
        if (found) {
            break;
        }
    }

    if (found == 0){
        /* means all valid i2c clock are faster than user's 
         * requested i2c clock!
         */
        I2C_ERROR(("%s, sys_clock=%d, user's i2c_clock=%d is too slow to "
                "be configurred!\n", __FUNCTION__, i2c_clock, speed_hz));
        return KSI2C_ERR_PARAM;
    } else {
        /* here can select a better frequency setting :
         *  - choose the most close to user's request frequency 
         */
        if (diff0 < diff1){         /* previous i2c clock will be selected */
            if (m == 0){
                n--;
            } else {
                m--;
            }
            i2c_clock = i2c_clk_tmp;    /* set the clock back to previous */
        }
        
        feq =((m << CC_I2CCCR_MSHIFT) & CC_I2CCCR_MMASK) | 
                        (n & CC_I2CCCR_NMASK);
    }
    
    I2C_MSG(("%s, setting I2C_%d CCR=%x!\n", __FUNCTION__, id, feq));
    if (si_i2c_clock(ksi2c_sih, (uint8)id, feq)){
        I2C_ERROR(("%s, failed on setting I2C_%d clock!\n", __FUNCTION__, id));
    }
#endif  /* KSI2C_BUS_DRIVER_READY */
    return KSI2C_ERR_NONE;
}

/* Function : ksi2c_write 
 *  - Write operation through Keystone's I2C.
 * Return :
 * Note : 
 */
int 
ksi2c_write(cc_i2c_id_t id, i2c_addr_t slave_addr, uint8 *data, int len) 
{
#if KSI2C_BUS_DRIVER_READY
    ks_i2c_bus_t   *i2cbus;
    int rv, retry; 

    KSI2C_INIT_CHK;
    if (data == NULL || len == 0){
        I2C_ERROR(("%s, No data to write at I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_PARAM ;
    }
    
    i2cbus = ksi2c_bus + id;

    if (!KSI2C_BUS_ENABLED(i2cbus)){
        I2C_ERROR(("%s,I2C_%d is not enabled yet!\n", __FUNCTION__, id));
        return KSI2C_ERR_UNAVAIL;
    }

    I2C_MSG(("%s, i2c_%d WRITE starting (saddr=0x%02x, len=%d)...\n", 
            __FUNCTION__, id,slave_addr,len));

    /* check bus idle status */
    for (retry = 0; retry < i2cbus->pio_retries; retry++){
        if (!KSI2C_BUS_OP_IDLE_CHECK(i2cbus)){
            ks_i2c_status_t bus_status;
            
            bus_status = ksi2c_bus_status(ksi2c_sih, id);
            if (bus_status == CC_I2CSTS_BUS_ERR){
                rv = KSI2C_ERR_BUS;     /* set bus error here for reset */
                goto fail_i2c_transaction;
            } else {
                if (bus_status == CC_I2CSTS_NO_STATUS){
                    i2cbus->opcode = KS_I2C_IDLE;
                    break;
                }
            }
        } else {
            break;
        }
    }
    
    if (!KSI2C_BUS_OP_IDLE_CHECK(i2cbus) || retry == i2cbus->pio_retries){
        rv = KSI2C_ERR_BUS;     /* set bus error here for reset */
        goto fail_i2c_transaction;
    } else {
        /* start the i2c read transaction */
        I2C_MSG(("%s,Send START..\n", __FUNCTION__));
        /* start with slave_addr + op */
        rv = ksi2c_bus_start(ksi2c_sih, id, 
                slave_addr, I2C_OP_WRITE, FALSE);
        if (rv){
            I2C_ERROR(("%s,Failed on START stage in Read I2C OP\n", 
                    __FUNCTION__));
            goto fail_i2c_transaction;
        }
        
        /* data write */
        I2C_MSG(("%s,write bytes..\n", __FUNCTION__));
        rv = ksi2c_bus_nbytes_write(ksi2c_sih, id, data, &len);
        if (rv){
            I2C_ERROR(("%s,Failed on DATA stage in Write I2C OP! rv=%d"
                        "(%d bytes write already)\n", __FUNCTION__, rv, len));
            goto fail_i2c_transaction;
        }
        
        /* stop */
        I2C_MSG(("%s,send STOP..\n", __FUNCTION__));
        rv = ksi2c_bus_stop(ksi2c_sih, id);
        if (rv){
            I2C_ERROR(("%s,Failed on STOP stage in Write I2C OP"
                        "(%d bytes sent already)\n", __FUNCTION__, len));
            goto fail_i2c_transaction;
        }
        
        return KSI2C_ERR_NONE;
    }

fail_i2c_transaction :
    if (rv == KSI2C_ERR_TIMEOUT || rv == KSI2C_ERR_UNKNOW || 
            rv == KSI2C_ERR_INTERNAL) {
        I2C_ERROR(("%s, Bus receovering through STOP\n", __FUNCTION__));
        if (ksi2c_bus_stop(ksi2c_sih, id)){
            I2C_ERROR(("%s, Bus receovering through Reset\n", __FUNCTION__));
            ksi2c_bus_reset(ksi2c_sih, id);
        }
    } else if (rv == KSI2C_ERR_BUS) {
        I2C_ERROR(("%s, Bus receovering through Reset\n", __FUNCTION__));
        ksi2c_bus_reset(ksi2c_sih, id);
    } else {
        I2C_ERROR(("%s, rv=%d, i2c write failed!\n", __FUNCTION__,rv));
    }
    return rv;

#else   /* KSI2C_BUS_DRIVER_READY */

    uint8   databyte = 0, ctrlbyte = 0, i2c_iflg = CC_I2CSTS_NO_STATUS;
    uint8   temp_val;
    int i, retry;
    i2c_addr_t  saddr_internal;
    
    saddr_internal = (slave_addr << I2C_SADDR_ADDR_OFFSET) & 
            I2C_SADDR_ADDR_MASK;
    
    KSI2C_INIT_CHK;
    if (data == NULL || len == 0){
        I2C_ERROR(("%s, No data to write at I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_PARAM ;
    } else {
        if (len > sizeof(data)){
            I2C_ERROR(("%s, Unexpected Data size!\n", __FUNCTION__));
            return KSI2C_ERR_PARAM ;
        }
    }

    /* 0. start the transaction and check IFLG */
    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte |= CC_I2CCTRL_MSTART;
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, CC_I2CCTRL_MASK, ctrlbyte);
    
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }

        if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after START(expect:0x08)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    I2C_MSG(("%s,%d,retry=%d!!\n",__FUNCTION__,__LINE__,retry));
    
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to write at I2C_%d, TIMEOUT!\n", __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    } 

    /* 1. set addr+op to i2c data buffer 
     *  - Note if the slave_addr is pre-shift already.
     */
    databyte = saddr_internal | (I2C_OP_WRITE & I2C_SADDR_OP_MASK);
    temp_val = si_i2c_data(ksi2c_sih, (uint8)id, CC_I2CDATA_MASK, databyte);

    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, 
                CC_I2CCTRL_MASK, ctrlbyte);

    I2C_MSG(("%s,%d,saddr+op!!!!\n",__FUNCTION__,__LINE__));
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }

        if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after SADDR(expect:0x18)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    I2C_MSG(("%s,%d,retry=%d!!\n", __FUNCTION__, __LINE__, retry));
    
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to write at I2C_%d, TIMEOUT!\n", __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    } 
    
    /* 2. write n bytes data */
    for (i = 0; i < len; i++){
        databyte = data[i];
        I2C_MSG(("%s,%d,writing #%d byte = 0x%02x!\n",
                __FUNCTION__,__LINE__,i,databyte));
        temp_val = si_i2c_data(ksi2c_sih, (uint8)id, 
                CC_I2CDATA_MASK, databyte);

        /* reset IFLG for write data byte */
        I2C_CTRL_IFLG_CLEAR(ctrlbyte);
        ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, 
                    CC_I2CCTRL_MASK, ctrlbyte);
        
        /* 2.1 check IFLG and clear IFLG for going to next process */
        for (retry=0; retry<1000; retry++){
            OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
                
            if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
                continue;
            }
            if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
                I2C_MSG(("%s, IFLG=[%02x] after DATA[%d](expect:0x28)\n", 
                        __FUNCTION__, i2c_iflg, i));
                break;
            }
        }
        if (retry == 1000) {
            I2C_ERROR(("%s, failed to write at I2C_%d, TIMEOUT!\n", 
                    __FUNCTION__, id));
            return KSI2C_ERR_TIMEOUT ;
        }
    }
    
    /* 2.2 check IFLG after latest date byte write */
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }
        if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after DATA[%d](expect:0x28/0x30)\n", 
                    __FUNCTION__, i2c_iflg, i));
            break;
        }
    }
    if (retry == 1000000) {
        I2C_ERROR(("%s, failed to write at I2C_%d, TIMEOUT!\n", 
                __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    }

    /* 3. Stop transaction */
    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte |= CC_I2CCTRL_MSTOP;
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, CC_I2CCTRL_MASK,ctrlbyte);

    /* check the IFLG */
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }

        if (i2c_iflg == CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after STOP(expect:0xf8)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to send STOP at I2C_%d, TIMEOUT!\n", 
                __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    } else {
        I2C_MSG(("%s, write DONE at I2C_%d\n", __FUNCTION__, id));
        return KSI2C_ERR_NONE;;
    }
    I2C_MSG(("%s, write DONE at I2C_%d\n", __FUNCTION__, id));
    return KSI2C_ERR_NONE;;
    
    
#endif  /* KSI2C_BUS_DRIVER_READY */
}

/* internal function to serve I2C random read/write which is to performing 
 * the read or write OP with i2c device address indicated.
 */
static int 
_ksi2c_i2cdev_rwop(cc_i2c_id_t id, int op,
        i2c_addr_t slave_addr, uint8 *data, int len, 
        uint32 dev_addr, int dev_addr_len) 
{
    ks_i2c_bus_t   *i2cbus;
    int rv, retry, temp_len;
    bool    this_op;
    uint8   i, dev_addr_bytes[KSI2C_DEV_ADDR_MAX_BYTES];
    uint8   *new_data, *data_ptr;
    int     new_len = dev_addr_len + len;

    KSI2C_INIT_CHK;
    if (data == NULL || len == 0){
        I2C_ERROR(("%s, No data to this operation at I2C_%d!\n", 
                __FUNCTION__, id));
        return KSI2C_ERR_PARAM ;
    }
    
    if (dev_addr_len == 0){
        I2C_ERROR(("%s, No device address was indicated to I2C_%d!\n", 
                __FUNCTION__, id));
        return KSI2C_ERR_PARAM;
    } else if (dev_addr_len > KSI2C_DEV_ADDR_MAX_BYTES){
        I2C_ERROR(("%s, Up to %d bytes to indicate the i2c device address!\n", 
                __FUNCTION__, KSI2C_DEV_ADDR_MAX_BYTES));
        return KSI2C_ERR_PARAM ;
    }

    this_op = (op == I2C_OP_WRITE) ? I2C_OP_WRITE : I2C_OP_READ;

    /* translate dev_addr to bytes array for i2c_write: up to 4 byte length */
    memset(dev_addr_bytes, 0, KSI2C_DEV_ADDR_MAX_BYTES);
    for (i = 0; i < dev_addr_len; i++){
#if ENDIAN_BIG 
        dev_addr_bytes[i] = *((uint8 *)&dev_addr + 
                KSI2C_DEV_ADDR_MAX_BYTES - (i + 1));
#else   /* LITTLE_ENDIAN */
        dev_addr_bytes[i] = *((uint8 *)&dev_addr + i);
#endif
    }

    i2cbus = ksi2c_bus + id;

    if (!KSI2C_BUS_ENABLED(i2cbus)){
        I2C_ERROR(("%s,I2C_%d is not enabled yet!\n", __FUNCTION__, id));
        return KSI2C_ERR_UNAVAIL;
    }
    
    I2C_MSG(("%s, i2c_%d for this OP is starting(saddr=0x%02x, len=%d)\n", 
            __FUNCTION__, id,slave_addr, len));
    
    /* check bus idle status */
    for (retry = 0; retry < i2cbus->pio_retries; retry++){
        if (!KSI2C_BUS_OP_IDLE_CHECK(i2cbus)){
            ks_i2c_status_t bus_status;
            
            bus_status = ksi2c_bus_status(ksi2c_sih, id);
            if (bus_status == CC_I2CSTS_BUS_ERR){
                rv = KSI2C_ERR_BUS;     /* set bus error here for reset */
                goto fail_i2c_dev_transaction;
            } else {
                if (bus_status == CC_I2CSTS_NO_STATUS){
                    i2cbus->opcode = KS_I2C_IDLE;
                    break;
                }
            }
        } else {
            break;
        }
    }

    if (!KSI2C_BUS_OP_IDLE_CHECK(i2cbus) || retry == i2cbus->pio_retries){
        rv = KSI2C_ERR_BUS;     /* set bus error here for reset */
        goto fail_i2c_dev_transaction;
    } else {

        /* start the i2c transaction */
        I2C_MSG(("%s,Send START..\n", __FUNCTION__));
        /* start with slave_addr + op */
        rv = ksi2c_bus_start(ksi2c_sih, id, 
                slave_addr, I2C_OP_WRITE, FALSE);
        if (rv){
            I2C_ERROR(("%s,Failed on START stage in Read I2C OP\n", 
                    __FUNCTION__));
            goto fail_i2c_dev_transaction;
        }

        if (this_op == I2C_OP_READ){
            /* READ OP need an additional I2C_WRITE to assign the device 
             *  address.
             */
            
            /* assigning i2c device address */
            I2C_MSG(("%s,write dev_addr..\n", __FUNCTION__));
            temp_len = dev_addr_len;
            rv = ksi2c_bus_nbytes_write(ksi2c_sih, id, 
                    dev_addr_bytes, &temp_len);
            if (rv){
                I2C_ERROR(("%s,Failed on assigning dev_addr stage! rv=%d"
                        "(%d bytes write already)\n", 
                        __FUNCTION__, rv, dev_addr_len));
                goto fail_i2c_dev_transaction;
            }
            
            /* Rep-start for this i2c transaction with indicated OP */
            I2C_MSG(("%s,Send START..\n", __FUNCTION__));
            /* start with slave_addr + op */
            rv = ksi2c_bus_start(ksi2c_sih, id, 
                    slave_addr, I2C_OP_READ, TRUE);
            if (rv){
                I2C_ERROR(("%s,Failed on START stage in this I2C OP\n", 
                        __FUNCTION__));
                goto fail_i2c_dev_transaction;
            }
            
            /* read */
            bzero(data, len);   /* set to zero on data */
            rv = ksi2c_bus_nbytes_read(ksi2c_sih, id, data, &len, FALSE);
        } else {
            
            /* OP is write */

            /* prepare the final data to be writen(include dev_addr) */
            new_data = (uint8 *)MALLOC(ksi2c_sih, new_len);
            ASSERT(new_data != NULL);
            bzero(new_data, new_len);
            data_ptr = new_data;
            memcpy(data_ptr, dev_addr_bytes, dev_addr_len);
            data_ptr += dev_addr_len;
            memcpy(data_ptr, data, len);

            rv = ksi2c_bus_nbytes_write(ksi2c_sih, id, new_data, &new_len);
            if (rv){
                len = new_len - dev_addr_len; 
                if (new_len < dev_addr_len){
                    I2C_ERROR(("%s,Failed on assigning device address!\n", 
                            __FUNCTION__));
                    len = 0;
                }
            }
        }

        /* data read/write */
        if (rv){
            I2C_ERROR(("%s,Failed on DATA R/W OP!(%d bytes write already)\n", 
                    __FUNCTION__, len));
            goto fail_i2c_dev_transaction;
        }
        
        /* stop */
        I2C_MSG(("%s,send STOP..\n", __FUNCTION__));
        rv = ksi2c_bus_stop(ksi2c_sih, id);
        if (rv){
            I2C_ERROR(("%s,Failed on STOP stage.\n", __FUNCTION__));
            goto fail_i2c_dev_transaction;
        }
        
        return KSI2C_ERR_NONE;
    }

fail_i2c_dev_transaction :
    if (rv == KSI2C_ERR_TIMEOUT || rv == KSI2C_ERR_UNKNOW || 
            rv == KSI2C_ERR_INTERNAL) {
        I2C_ERROR(("%s, Bus receovering through STOP\n", __FUNCTION__));
        if (ksi2c_bus_stop(ksi2c_sih, id)){
            I2C_ERROR(("%s, Bus receovering through Reset\n", __FUNCTION__));
            ksi2c_bus_reset(ksi2c_sih, id);
        }
    } else if (rv == KSI2C_ERR_BUS) {
        I2C_ERROR(("%s, Bus receovering through Reset\n", __FUNCTION__));
        ksi2c_bus_reset(ksi2c_sih, id);
        /* check if i2c clock is requirred to set again after reset */
    } else {
        I2C_ERROR(("%s, rv=%d, i2c random %s failed!\n", 
                __FUNCTION__, rv, 
                (this_op == I2C_OP_WRITE) ? "WRITE" : "READ"));
    }
    return rv;
}


/* Function : ksi2c_i2cdev_read 
 *  - Read operation through Keystone's I2C with i2c device's address 
 *      indicated for read operation.
 * Return :
 * Note :
 *  1. Parameters :
 *      - data : the data buffer for reporting the read data.
 *      - len : the length of the read data.
 *      - dev_addr : The input value to indicate the i2c device's address for 
 *              this read opration.
 *      - dev_addr_len : the i2c device's address lengh (byte unit)
 *      >> P.S. the real length of data and dev_addr must be verified before 
 *              calling this API.
 *  2. Normally, this API is used for reading the i2c device at a specific 
 *      device address. For the i2c deivce may have different length of 
 *      address format, this API need user to tell the length(byte) of device 
 *      address and the i2c protocol will preform a i2c_write befor i2c_read  
 *      to tell the device about the target address for read operation. 
 */
int 
ksi2c_i2cdev_read(cc_i2c_id_t id, 
        i2c_addr_t slave_addr, uint8 *data, int len, 
        uint32 dev_addr, int dev_addr_len) 
{
    return _ksi2c_i2cdev_rwop(id, I2C_OP_READ,
            slave_addr, data, len, dev_addr, dev_addr_len);
}

/* Function : ksi2c_i2cdev_write 
 *  - Write operation through Keystone's I2C with i2c device's address 
 *      indicated.
 * Return :
 * Note :
 *  1. Parameters :
 *      - data : the data buffer for reporting the write data.
 *      - len : the length of the read data.
 *      - dev_addr : The input value to indicate the i2c device's address for 
 *              this read opration.
 *      - dev_addr_len : the i2c device's address lengh (byte unit)
 *      >> P.S. the real length of data and dev_addr must be verified before 
 *              calling this API.
 *  2. Normally, this API is used for writing data to i2c device at a specific 
 *      device address. For the i2c deivce may have different length of 
 *      address format, this API need user to tell the length(byte) of device 
 *      address and the i2c protocol will preform a i2c_write befor i2c_read  
 *      to tell the device about the target address for read operation. 
 */
int 
ksi2c_i2cdev_write(cc_i2c_id_t id, 
        i2c_addr_t slave_addr, uint8 *data, int len, 
        uint32 dev_addr, int dev_addr_len) 
{
    return _ksi2c_i2cdev_rwop(id, I2C_OP_WRITE,
            slave_addr, data, len, dev_addr, dev_addr_len);
}

/* Function : ksi2c_read 
 *  - Read operation through Keystone's I2C.
 * Return :
 * Note :
 */
int 
ksi2c_read(cc_i2c_id_t id, i2c_addr_t slave_addr, uint8 *data, int len) 
{
#if KSI2C_BUS_DRIVER_READY
    ks_i2c_bus_t   *i2cbus;
    int rv, retry; 

    KSI2C_INIT_CHK;
    if (data == NULL || len == 0){
        I2C_ERROR(("%s, No data to READ at I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_PARAM ;
    }

    i2cbus = ksi2c_bus + id;

    if (!KSI2C_BUS_ENABLED(i2cbus)){
        I2C_ERROR(("%s,I2C_%d is not enabled yet!\n", __FUNCTION__, id));
        return KSI2C_ERR_UNAVAIL;
    }
    
    I2C_MSG(("%s, i2c_%d READ starting (saddr=0x%02x, len=%d)...\n", 
            __FUNCTION__, id,slave_addr,len));
    
    /* check bus idle status */
    for (retry = 0; retry < i2cbus->pio_retries; retry++){
        if (!KSI2C_BUS_OP_IDLE_CHECK(i2cbus)){
            ks_i2c_status_t bus_status;
            
            bus_status = ksi2c_bus_status(ksi2c_sih, id);
            if (bus_status == CC_I2CSTS_BUS_ERR){
                rv = KSI2C_ERR_BUS;     /* set bus error here for reset */
                goto fail_i2c_transaction;
            } else {
                if (bus_status == CC_I2CSTS_NO_STATUS){
                    i2cbus->opcode = KS_I2C_IDLE;
                    break;
                }
            }
        } else {
            break;
        }
    }
    
    if (!KSI2C_BUS_OP_IDLE_CHECK(i2cbus) || retry == i2cbus->pio_retries){
        rv = KSI2C_ERR_BUS;     /* set bus error here for reset */
        goto fail_i2c_transaction;
    } else {
        /* start the i2c read transaction */
        I2C_MSG(("%s,Send START..\n", __FUNCTION__));
        /* start with slave_addr + op */
        rv = ksi2c_bus_start(ksi2c_sih, id, 
                slave_addr, I2C_OP_READ, FALSE);
        if (rv){
            I2C_ERROR(("%s,Failed on START stage in Read I2C OP\n", 
                    __FUNCTION__));
            goto fail_i2c_transaction;
        }
        
        /* data read */
        I2C_MSG(("%s,read bytes..\n", __FUNCTION__));
        bzero(data, len);   /* set to zero on data */
        rv = ksi2c_bus_nbytes_read(ksi2c_sih, id, data, &len, FALSE);
        if (rv){
            I2C_ERROR(("%s,Failed on DATA stage in Read I2C OP"
                        "(%d bytes sent already)\n", __FUNCTION__, len));
            goto fail_i2c_transaction;
        }
        
        /* stop */
        I2C_MSG(("%s,send STOP..\n", __FUNCTION__));
        rv = ksi2c_bus_stop(ksi2c_sih, id);
        if (rv){
            I2C_ERROR(("%s,Failed on STOP stage in Read I2C OP"
                        "(%d bytes sent already)\n", __FUNCTION__, len));
            goto fail_i2c_transaction;
        }
        
        return KSI2C_ERR_NONE;
    }

fail_i2c_transaction :
    if (rv == KSI2C_ERR_TIMEOUT || rv == KSI2C_ERR_UNKNOW || 
            rv == KSI2C_ERR_INTERNAL) {
        I2C_ERROR(("%s, Bus receovering through STOP\n", __FUNCTION__));
        if (ksi2c_bus_stop(ksi2c_sih, id)){
            I2C_ERROR(("%s, Bus receovering through Reset\n", __FUNCTION__));
            ksi2c_bus_reset(ksi2c_sih, id);
        }
    } else if (rv == KSI2C_ERR_BUS) {
        I2C_ERROR(("%s, Bus receovering through Reset\n", __FUNCTION__));
        ksi2c_bus_reset(ksi2c_sih, id);
        /* check if i2c clock is requirred to set again after reset */
    } else {
        I2C_ERROR(("%s, rv=%d, i2c read failed!\n", __FUNCTION__,rv));
    }
    return rv;
#else   /* KSI2C_BUS_DRIVER_READY */
    uint8   databyte = 0, ctrlbyte = 0, i2c_iflg = CC_I2CSTS_NO_STATUS;
    uint8   temp_val;
    int i, retry;
    i2c_addr_t  saddr_internal;
    
    saddr_internal = (slave_addr << I2C_SADDR_ADDR_OFFSET) & 
            I2C_SADDR_ADDR_MASK;
    
    KSI2C_INIT_CHK;
    if ( len == 0){
        I2C_ERROR(("%s, No data to read at I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_PARAM ;
    } else {
        if (len > sizeof(data)){
            I2C_ERROR(("%s, Unexpected Data size!\n", __FUNCTION__));
            return KSI2C_ERR_PARAM ;
        }
    }

    /* 0. start the transaction and check IFLG */
    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte |= CC_I2CCTRL_MSTART;
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, 
                    CC_I2CCTRL_MASK, ctrlbyte);
    
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us  */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }

        /* @@ should check start */
        if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after START (Expect:0x08)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    I2C_MSG(("%s,%d,retry=%d!!\n", __FUNCTION__, __LINE__, retry));
    
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to read at I2C_%d, TIMEOUT!\n", __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    } 

    /* 1. set addr+op to i2c data buffer 
     *  - Note if the slave_addr is pre-shift already.
     */
    databyte = saddr_internal | (I2C_OP_READ & I2C_SADDR_OP_MASK);
    temp_val = si_i2c_data(ksi2c_sih, (uint8)id, CC_I2CDATA_MASK, databyte);

    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, CC_I2CCTRL_MASK,ctrlbyte);

    I2C_MSG(("%s,%d,saddr+op!!!!\n", __FUNCTION__, __LINE__));
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }

        /* @@ should check addr+op tx with ACK */
        if (i2c_iflg != CC_I2CSTS_NO_STATUS) { 
            I2C_MSG(("%s, IFLG=[%02x] after SADDR+OP(Expect:0x40)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    I2C_MSG(("%s,%d,retry=%d!!\n", __FUNCTION__, __LINE__, retry));
    
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to read at I2C_%d, TIMEOUT!\n", __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    } 
    
    /* reset to start data read */
    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, CC_I2CCTRL_MASK,ctrlbyte);
        
    /* 2. read n bytes data : */
    for (i = 0; i < len; i++){

        /* 2.1 check IFLG and clear IFLG for going to next process */
        for (retry=0; retry<1000; retry++){
            OSL_DELAY(1);   /* delay 1 us */
                
            if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
                continue;
            }

            if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
                I2C_MSG(("%s,IFLG=[%02x] after rx DATA[%d],(expect:0x50/58\n", 
                        __FUNCTION__, i2c_iflg, i));
                break;
            }
        }
        if (retry == 1000) {
            I2C_ERROR(("%s, failed to read at I2C_%d, TIMEOUT!\n", 
                    __FUNCTION__, id));
            return KSI2C_ERR_TIMEOUT ;
        }
        
        databyte = 0;
        databyte = si_i2c_data(ksi2c_sih, (uint8)id, CCI2C_READREG_MASK, 0);
        I2C_MSG(("%s,%d,read #%d byte = 0x%02x!\n",
                __FUNCTION__, __LINE__, i, databyte));
        data[i] = databyte;
        
        I2C_CTRL_IFLG_CLEAR(ctrlbyte);
        /* skip the Assert ACK at the latest data bye read */        
        if (i < (len - 1)) {
            /* AACK and reset IFLG */
            ctrlbyte |= CC_I2CCTRL_AAK;
        }
        ctrlbyte = si_i2c_control(ksi2c_sih, id, CC_I2CCTRL_MASK, ctrlbyte);
    }
    /* 2.2 check IFLG and clear IFLG for going to next process */
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }

        if (i2c_iflg != CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after last DATA byte,(expect:0x58)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to read at I2C_%d, TIMEOUT!\n", __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    }


    /* 3. Stop transaction */
    I2C_CTRL_IFLG_CLEAR(ctrlbyte);
    ctrlbyte |= CC_I2CCTRL_MSTOP;
    ctrlbyte = si_i2c_control(ksi2c_sih, (uint8)id, CC_I2CCTRL_MASK,ctrlbyte);

    /* check the IFLG */
    I2C_MSG(("%s,%d,stoping!!!!\n",__FUNCTION__,__LINE__));
    for (retry=0; retry<1000; retry++){
        OSL_DELAY(1);   /* delay 1 us and timeout after 1 S. */
            
        if (si_i2c_status(ksi2c_sih, id, CC_I2CSTS_MASK, &i2c_iflg)){
            continue;
        }
        if (i2c_iflg == CC_I2CSTS_NO_STATUS) {
            I2C_MSG(("%s, IFLG=[%02x] after STOP,(expect:0xf8)\n", 
                    __FUNCTION__, i2c_iflg));
            break;
        }
    }
    if (retry == 1000) {
        I2C_ERROR(("%s, failed to send STOP at I2C_%d, TIMEOUT!\n", 
                __FUNCTION__, id));
        return KSI2C_ERR_TIMEOUT ;
    }
    I2C_MSG(("%s, read DONE at I2C_%d\n", __FUNCTION__, id));
    return KSI2C_ERR_NONE;;
    
#endif  /* KSI2C_BUS_DRIVER_READY */
}

/* Function : ksi2c_rw_pack 
 *  - Mix the read/write operations in a i2c transaction.
 * Return :
 * Note :
 *  1. the parameter is not defined yet.
 */
int 
ksi2c_rw_pack(cc_i2c_id_t id) 
{
    KSI2C_INIT_CHK;
    /* TBD, implemented in next stage */
    return KSI2C_ERR_UNAVAIL;
}



/* Function : ksi2c_close 
 *  - close Keystone's I2C port.
 * Return :
 * Note :
 */
int 
ksi2c_close(cc_i2c_id_t id) 
{
    KSI2C_INIT_CHK;
#if KSI2C_BUS_DRIVER_READY
    if (ksi2c_bus_enable(ksi2c_sih, id, FALSE)) {
        I2C_ERROR(("%s, Failed on disabling I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_INTERNAL;
    }
    return ksi2c_bus_detach(ksi2c_sih, id);
#else   /* KSI2C_BUS_DRIVER_READY */

    /* select the i2c interface */
    if (si_i2c_select(ksi2c_sih, (uint8)id, FALSE)){
        I2C_ERROR(("%s, Failed on %s I2C_%d!\n", __FUNCTION__, id));
        return KSI2C_ERR_INTERNAL;
    }

    return KSI2C_ERR_NONE;
#endif  /* KSI2C_BUS_DRIVER_READY */

}

/* =================== GPIO funcitons ===================== 
 *  GPIO interface to register interfaces.
 */

#define GPIO_REG_WMASK 0xffffffff   /* mask for write */
#define GPIO_REG_RMASK 0x0          /* mask for read */

/* TBD : 
 *  check the core package(check gpio supports io pins count is 16 or 12) 
 */

void 
ksgpio_Init(void)
{
    ksi2c_sih = si_kattach(SI_OSH);
}

int 
ksgpio_InEventMaskSet(uint16 emask)
{
    uint32  cc_val = 0;
    
    cc_val = emask;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioevent(ksi2c_sih, GPIO_REGEVT_INTMSK, GPIO_REG_WMASK, cc_val);
    if (cc_val != emask){
        printf("%s,CHECKME!! Mismatched(%x,%x)!\n",__FUNCTION__, cc_val, emask);
        return -1;
    }
    
    return 0;
}

int 
ksgpio_InEventChk(uint16 *event){
    uint32  cc_val = 0;
    
    *event = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioevent(ksi2c_sih, GPIO_REGEVT, 0, cc_val);
    *event = (cc_val & 0xffff);
    
    return 0;
}

int 
ksgpio_InGet(uint16 *val){
    uint32  cc_val = 0;
    
    *val = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioin(ksi2c_sih);
    *val = (cc_val & 0xffff);
    
    return 0;
}

int 
ksgpio_OutSet(uint16 val){
    uint32  cc_val = 0;
    
    cc_val = val;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioout(ksi2c_sih, GPIO_REG_WMASK, cc_val, GPIO_DRV_PRIORITY);
    if (cc_val != val){
        printf("%s,CHECKME!! Mismatched(%x,%x)!\n",__FUNCTION__, cc_val, val);
        return -1;
    }
    return 0;
}

int 
ksgpio_OutGet(uint16 *val){
    uint32  cc_val = 0;
    
    *val = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioout(ksi2c_sih, GPIO_REG_RMASK, cc_val, GPIO_DRV_PRIORITY);
    *val = (cc_val & 0xffff);
    
    return 0;
}

int
ksgpio_OutEnGet(uint16 *enmask){
    uint32  cc_val = 0;
    
    *enmask = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioouten(ksi2c_sih, GPIO_REG_RMASK, cc_val, GPIO_DRV_PRIORITY);
    *enmask = (cc_val & 0xffff);
    
    return 0;
}

int
ksgpio_OutEnSet(uint16 enmask){
    uint32  cc_val = 0;
    
    cc_val = enmask;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpioouten(ksi2c_sih, GPIO_REG_WMASK, cc_val, GPIO_DRV_PRIORITY);

    if (cc_val != enmask){
        printf("%s,CHECKME!! Mismatched(%x,%x)!\n",__FUNCTION__, cc_val, enmask);
        return -1;
    }
    return 0;
}

int
ksgpio_CtrlGet(uint16 *ctrl){
    uint32  cc_val = 0;
    
    *ctrl = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpiocontrol(ksi2c_sih, GPIO_REG_RMASK, cc_val, GPIO_DRV_PRIORITY);
    *ctrl = (cc_val & 0xffff);
    
    return 0;
}

int
ksgpio_CtrlSet(uint16 ctrl){
    uint32  cc_val = 0;
    
    cc_val = ctrl;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpiocontrol(ksi2c_sih, GPIO_REG_WMASK, cc_val, GPIO_DRV_PRIORITY);

    if (cc_val != ctrl){
        printf("%s,CHECKME!! Mismatched(%x,%x)!\n",__FUNCTION__, cc_val, ctrl);
        return -1;
    }
    return 0;
}

int 
ksgpio_TimerEnableSet(uint16 enmask)
{
    uint32  cc_val = 0;
    
    cc_val = enmask;
    KSI2C_INIT_CHK;
    cc_val = si_gpiotimer_outmask(ksi2c_sih, GPIO_REG_WMASK, cc_val);

    if (cc_val != enmask){
        printf("%s,CHECKME!! Mismatched(%x,%x)!\n",__FUNCTION__, cc_val, enmask);
        return -1;
    }
    return 0;
}

int 
ksgpio_TimerEnableGet(uint16 *enmask)
{
    uint32  cc_val = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpiotimer_outmask(ksi2c_sih, 0, cc_val);
    *enmask = (cc_val & 0xffff);

    return 0;
}

int 
ksgpio_TimerSet(uint16 on_cnt, uint16 off_cnt)
{
    uint32  cc_val = 0;
    
    cc_val = ((on_cnt & 0xffff) << 16) | (off_cnt & 0xffff);
    
    KSI2C_INIT_CHK;
    cc_val = si_gpiotimerval(ksi2c_sih, GPIO_REG_WMASK, cc_val);

    if (cc_val != (((on_cnt & 0xffff) << 16) | (off_cnt & 0xffff))){
        printf("%s,CHECKME!! Mismatched(%x,on(%x)+off(%x))!\n",
                __FUNCTION__, cc_val, on_cnt, off_cnt);
        return -1;
    }

    return 0;
}

int 
ksgpio_TimerGet(uint16 *on_cnt, uint16 *off_cnt)
{
    uint32  cc_val = 0;
    
    *on_cnt = 0;
    *off_cnt = 0;
    
    KSI2C_INIT_CHK;
    cc_val = si_gpiotimerval(ksi2c_sih, GPIO_REG_RMASK, cc_val);
    *off_cnt= cc_val & 0xffff;
    *on_cnt= (cc_val >> 16) & 0xffff;
    
    return 0;
}


int
ksgpio_PullGet(bool op, uint16 *val)
{
    uint32  temp = 0;
    KSI2C_INIT_CHK;
    
    /* op = 1 means PullDown; op=0 means PullUp */
    temp = si_gpiopull(ksi2c_sih, op, GPIO_REG_RMASK, (uint32)*val);
    *val = (uint16)temp;

    return 0;
}

int
ksgpio_PullSet(bool op, uint16 val)
{
    uint32  temp = 0;
    
    KSI2C_INIT_CHK;
    
    temp = val;
    /* op = 1 means PullDown; op=0 means PullUp */
    temp = si_gpiopull(ksi2c_sih, op, GPIO_REG_WMASK, temp);

    if (temp != val){
        printf("%s,CHECKME!! failed on set GPIO pull down/up!\n", __FUNCTION__);
        return -1;
    }
    return 0;
}

void
ksgpio_dumpreg(void)
{
    uint16 val = 0, val2 = 0;
    int rv;
    
    if (ksi2c_sih == NULL){
        printf("%s, run 'i2c init' before gpio's test!!\n",__FUNCTION__);
    }
    printf("\n -- Dump GPIO registers --\n");
    /* 1 means pulldown */
    /* 0 means pullUp */
    val = si_gpiopull(ksi2c_sih, 1, 0, 0);
    val2 = si_gpiopull(ksi2c_sih, 0, 0, 0);
    printf("\t PullDown=0x%04x, PullUp=0x%04x\n",val, val2);
    
    val = 0;
    val = si_gpiointpolarity(ksi2c_sih, 0, val, GPIO_DRV_PRIORITY);
    val2 = 0;
    val2 = si_gpiointmask(ksi2c_sih, 0, val2, GPIO_DRV_PRIORITY);
    printf("\t InputIntPolarity=0x%04x, InputIntMask=0x%04x\n",val, val2);
    
    val = 0;
    val = si_gpioevent(ksi2c_sih, GPIO_REGEVT, 0, val);
    val2 = 0;
    val2 = si_gpioevent(ksi2c_sih, GPIO_REGEVT_INTMSK, 0, val2);
    printf("\t Event=0x%04x, EvtIntMask=0x%04x, ",val, val2);
    val = 0;
    val = si_gpioevent(ksi2c_sih, GPIO_REGEVT_INTPOL, 0, val);
    printf("EvtIntPolarity=0x%04x\n", val);
        
    rv = ksgpio_InGet(&val);
    rv = ksgpio_OutGet(&val2);
    printf("\t Input=0x%04x, Output=0x%04x, ",val, val2);
    rv = ksgpio_OutEnGet(&val);
    printf("OutuptEn=0x%04x\n",val);
    
    rv = ksgpio_CtrlGet(&val);
    rv = ksgpio_TimerEnableGet(&val2);
    printf("\t Control=0x%04x, TimerEn=0x%04x\n",val, val2);
    
    rv = ksgpio_TimerGet(&val, &val2);
    printf("\t OnCount=0x%04x, OffCount=0x%04x!!\n",val,val2);
    
}


