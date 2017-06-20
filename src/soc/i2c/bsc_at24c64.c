/*
 * $Id: bsc_at24c64.c,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *               [BSC] - Broadcom Serial Control (I2C constroller)
 *                 |
 *       ==========O=======o======================o========== I2C bus
 *                         |                      |
 *                    [pca9548 - 8-port mux]  [at24c64 - serial eeprom]
 *                      | | | | | | | |
 *           [eeprom]  -  | | | | | | |
 *            [max6653]  -  | | | | | |
 *             [max6653]   -  | | |
 *               [max6653]   -  | |
 *                 [max6653]   -   - [dpm73]
 */


#if defined(INCLUDE_I2C) && defined(BCM_FE2000_SUPPORT)
#include <sal/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/bsc.h>
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <soc/sbx/sbx_drv.h>
extern unsigned int
sbFe2000UtilPciIICRead(int unit, unsigned int uRegAddr, unsigned int *puData);

extern unsigned int
sbFe2000UtilPciIICWrite(int unit, unsigned int uRegAddr, unsigned int uData);

extern unsigned int
sbFe2000UtilIICRead(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int *data);

extern unsigned int
sbFe2000UtilIICWrite(	int unit,
			unsigned int slave_dev_addr,
			unsigned int reg_index,
			unsigned int data);

extern void thin_delay(unsigned int nanosecs);
#ifndef PLISIM
static int get_board_info(uint8 *board_id, uint8 *slot_id);
#endif /* ndef PLISIM */

#define LC2464_WRITE_CYCLE_DELAY   (10*MILLISECOND_USEC) /* 10 ms MAX */

/*
 * Function: eep24c64_checksum
 *
 * Purpose: 16 bit incremental checksum; used for checking data
 *          validity of the configuration parameter block.
 * Parameters:
 *        partial - partial checksum, or zero if initial checksum,
 *                  updated on each call.
 *        data - data buffer to checksum
 *        len - size of data buffer to checksum
 *
 * Returns: updated checksum value
 *
 * Notes:
 *       This routine is called incrementally, one or more times,
 *       and each time, the new checksum value is returned. The first
 *       time this routine is called, pass 0 as the initial checksum
 *       value. On succesive calls, pass the last computed checksum
 *       value.
 *
 */
static uint16
eep24c64_chksum(uint16 partial, uint8* data, int len)
{
    uint8* dp;
    uint8 c0, c1;
    int i;

    partial = soc_ntohs(partial);

    c0 = (uint8) partial;
    c1 = (uint8) (partial >> 8);

    for (i=0, dp = (uint8*)data; i < len; i++, dp++) {
            c0 += *dp;
            c1 += c0;
    }
    partial = (c1 << 8) + c0;
    return soc_htons(partial);
}

/*
 * Function: eep24c64_get_params
 *
 * Purpose: Get EEPROM configuration block from BSC NVRAM.
 *          Used internally by driver, demonstrates how to reliably
 *          store configuration parameters with a checksum.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    config - NVRAM configuration block.
 *
 * Returns:
 *    SOC_E_INTERNAL - checksum bad, contents invalid.
 *    SOC_E_NONE - checksum read and contents valid.
 *
 */
static int
eep24c64_get_params(uint32 unit, uint32 devno, eep24c64_t* config)
{
    int retv;
    uint32 len = sizeof(eep24c64_t);
    uint16 savesum, sum;

    if (!config) {
        return SOC_E_PARAM;
    }

    if ((retv = soc_eep24c64_read(unit, devno,
                    AT24C64_CONFIG_START, (uint8*)config, &len)) < 0 ) {
        LOG_CLI((BSL_META_U(unit,
                            "eep24c64_get_params: %s\n"), soc_errmsg(retv)));
        return retv;
    }

    /* Checksum config block */
    savesum = config->chksum;
    sum = eep24c64_chksum(0,  (uint8 *)&config->size, sizeof(uint16) );
    sum = eep24c64_chksum(sum,(uint8 *)&config->type, sizeof(uint16) );
    sum = eep24c64_chksum(sum,(uint8 *)&config->name, LC2464_NAME_LEN);

    if (sum != savesum) {
        LOG_CLI((BSL_META_U(unit,
                            "EEPROM: bad chksum: 0x%04x calc: 0x%04x\n"),
                 savesum, sum));
        return SOC_E_INTERNAL;
    }
    return SOC_E_NONE ;
}

/*
 * Function: eep24c64_set_params
 *
 * Purpose: Set EEPROM configuration block into NVRAM.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    config - NVRAM configuration block.
 *
 * Returns:
 *    SOC_E_TIMEOUT  - write error or chip timeout
 *    SOC_E_NONE - configuration block and checksum updated, written to NVRAM.
 *
 */
static int
eep24c64_set_params(uint32 unit, uint32 devno, eep24c64_t* config)
{
    int retv;
    int len = sizeof(eep24c64_t);
    uint16 sum = 0;

    if (!config)
            return SOC_E_PARAM;

    /* Checksum config block (over length and checksum) */
    sum = eep24c64_chksum(0,  (uint8 *)&config->size, sizeof(uint16) );
    sum = eep24c64_chksum(sum,(uint8 *)&config->type, sizeof(uint16) );
    sum = eep24c64_chksum(sum,(uint8 *)&config->name, LC2464_NAME_LEN);
    config->chksum = sum;

    LOG_CLI((BSL_META_U(unit,
                        "EEPROM: set config at 0x%04x\n"), (int)AT24C64_CONFIG_START));
    retv = soc_eep24c64_write(unit, devno,
                    AT24C64_CONFIG_START, (uint8*)config, len);
    return retv;
}

/*
 * Function: at24c64_init
 *
 * Purpose: initialize the ATMEL 24C64 NVRAM chip.  Attempt to read
 *          checksum, if checksum is invalid, attempt to write a pattern
 *          to the first three quadrants of the 64K chip, and then read
 *          it all back, checking each byte to verify data integrity. When
 *          finished, write the new configuration block to the start
 *          of the chip.
 *
 *          The last quadrant of the NVRAM is write protectable and
 *          stores read-only static information about the platform.
 *
 *          If the checksum is valid, simply display the checksum and
 *          number of bytes tested initially when the chip was probed.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    data - address of test data
 *    len - size of test data
 *
 *
 * Notes: When the initial NVRAM is configured, this test destructively
 *       modifies the NVRAM contents. If the NVRAM ever becomes
 *       corrupt, this driver will clear the NVRAM with a new data
 *       pattern. All reads and writes should be performed past the
 *       NVRAM configuration block (AT24C64_DATA_START).
 *
 */
static int
at24c64_init(int unit, int devno)
{
    eep24c64_t config;
    const char *devname;
    int retv = SOC_E_NONE;
    int ltemp = 0;

    devname = soc_bsc_devname(unit, devno);
    sal_memset(&config, 0 , sizeof(eep24c64_t));
    if (eep24c64_get_params(unit, devno, &config) < 0 ) {
        /* Write configuration block to NVRAM */
        config.size = AT24C64_DEVICE_RW_SIZE - AT24C64_PARAMS_SIZE;
        config.type = 0x70;
        sal_memset(config.name, 0x0,LC2464_NAME_LEN);

        ltemp = sal_strlen(devname);
        /* Coverity : 21386 */
        if(ltemp > LC2464_NAME_LEN) {
            LOG_WARN(BSL_LS_SOC_I2C,
                     (BSL_META_U(unit,
                                 "Device name %s too long, trimming it\n"),
                      devname));
            sal_memcpy(config.name, devname, LC2464_NAME_LEN - 1);
            config.name[LC2464_NAME_LEN] = '\0';
        } else {
            sal_strncpy(config.name, devname, ltemp);
            if (ltemp)
                config.name[ltemp] = '\0';
        }
        LOG_CLI((BSL_META_U(unit,
                            "EEPROM: initiailzing ... \n")));
        retv = eep24c64_set_params(unit, devno, &config);
        LOG_CLI((BSL_META_U(unit,
                            "EEPROM: done\n")));
    }
    return retv;
}

void
soc_eep24c64_test(int unit, int devno)
{
    uint8 pattern;
    int i, j, retv;
    uint32 len;
    uint8* outbuf = NULL;
    uint8* inbuf = NULL;

    if (!soc_bsc_is_attached(unit)) {
        soc_bsc_attach(unit);
    }

    LOG_CLI((BSL_META_U(unit,
                        " testing data integrity\n")));

    /* Full buffer write in one shot ...*/
    len = AT24C64_DEVICE_RW_SIZE;
    outbuf = (uint8*)sal_alloc(len, "bsc");
    if (outbuf == NULL) {
        LOG_CLI((BSL_META_U(unit,
                            "No memory\n")));
        return;
    }
    inbuf = (uint8*)sal_alloc(len, "bsc");
    if (inbuf == NULL) {
        sal_free(outbuf);
        LOG_CLI((BSL_META_U(unit,
                            "No memory\n")));
        return;
    }

    /* Write known pattern, clobber the parameter block */
    pattern = (uint8) sal_time_usecs();
    sal_memset(outbuf,pattern, len);
    sal_memset(inbuf, 0x0, len);

    LOG_CLI((BSL_META_U(unit,
                        " writing %d bytes pattern=0x%x\n"), len, pattern));

    retv = soc_eep24c64_write(unit, devno, 0, outbuf, len);
    if (retv != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            " write failed: %s\n"), soc_errmsg(len)));
        goto err_exit;
    }

    LOG_CLI((BSL_META_U(unit,
                        " reading %d bytes\n"), len));

    retv = soc_eep24c64_read(unit, devno, 0, inbuf, &len);
    if ((len == AT24C64_DEVICE_RW_SIZE) && (retv == SOC_E_NONE)) {
        for (j = 0; j < AT24C64_DEVICE_RW_SIZE; j++) {
            if (inbuf[j] != outbuf[j]) {
                LOG_CLI((BSL_META_U(unit,
                                    " miscompare at addr: %d"
                                    " wr: 0x%02x rd: 0x%02x\n"),
                         j, outbuf[j], inbuf[j]));
                for (i = j; i < j + 10; i++) {
                    LOG_CLI((BSL_META_U(unit,
                                        " %02x"), inbuf[i]));
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n")));
                break;
            }
        }
        if (j == AT24C64_DEVICE_RW_SIZE) {
            LOG_CLI((BSL_META_U(unit,
                                " test passed (%d bytes verified)\n"), len));
        }
    }

err_exit:
    sal_free(inbuf);
    sal_free(outbuf);
}

#ifndef PLISIM
static int
_my_mc_fpga_read8(int addr, uint8 *v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int pl = -1;
    int word;
    int shift = (3 - (addr & 3)) * 8;
    int unit=-1;

    int i;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))){
        if (lcm0==-1)
          lcm0=SOC_NDEV_IDX2DEV(i);
        else if (lcm1==-1)
          lcm1=SOC_NDEV_IDX2DEV(i);
      }else  if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i))){
        pl = SOC_NDEV_IDX2DEV(i);
      }
    }


    if (lcm1 != -1){
      word = FPGA_BASE + (addr & ~0x3);
      unit = lcm1;
    }else if (pl != -1){
      word = FPGA_PL_BASE + (addr & ~0x3);
      unit = pl;
    }else{
      LOG_CLI((BSL_META("LCM/BME with FPGA not found\n")));
      return SOC_E_INTERNAL;
    }
    *v = (CMREAD(unit, word) >> shift) & 0xff;
    return SOC_E_NONE;
}

static int get_board_info(uint8 *board_id, uint8 *slot_id)
{
        int retv;

        retv = _my_mc_fpga_read8(FPGA_BOARD_ID_OFFSET, board_id);
        if (retv) {
                LOG_CLI((BSL_META("get_board_info: FPGA read error\n")));
                return retv;
        }
        retv = _my_mc_fpga_read8(FPGA_LC_PL_SLOT_ID_OFFSET, slot_id);
        if (retv) {
                LOG_CLI((BSL_META("get_board_info: FPGA read error\n")));
                return retv;
        }
        return 0;
}
#endif /* ndef PLISIM */

static int at24c64_ioctl(int unit, int devno, int opcode, void* data, int len)
{
#ifndef PLISIM
    soc_bsc_bus_t *bscbus = BSCBUS(unit);
    int retv = 0;
    uint8 board_id, slot_id, tmp_data;
    uint32 tmp_len = 1;

    switch(opcode) {
    case BSC_IOCTL_DEVICE_PRESENT:
        retv = get_board_info(&board_id, &slot_id);
        if (retv != 0) {
            LOG_CLI((BSL_META_U(unit,
                                "get_board_info: FPGA read error\n")));
            goto err_exit;
        }

        bscbus->devs[devno]->saddr =
                BSC_AT24C32_SADDR + (slot_id & 0x3);
        if (board_id == FPGA_LC_PL_BOARD) {
            if (slot_id & FPGA_LC_PL_BIB_CHASIS) {
                 bscbus->devs[devno]->saddr =
                                        BSC_AT24C32_SADDR;
            }
        }
        /*
         * Slot ID 0 address conflicts with EEPROM on CPU card.
         * Ignore probe to avoid corrupting CPU card EEPROM
         */
        if (slot_id == 0) {
            return 0;
        }
        /* do {*/
        LOG_INFO(BSL_LS_SOC_I2C,
                 (BSL_META_U(unit,
                             "at24c64_ioctl: BoardID:0x%02x Slot:0x%02x SADDR:0x%02x\n"),
                  board_id, slot_id, bscbus->devs[devno]->saddr));
            /*
             * There's NO better way other than just read/write/read
             * one byte back to approve the eeprom's there.
             */
            tmp_len = 1;
            if (soc_eep24c64_read(unit, devno, 0,
                                  &tmp_data, &tmp_len) == 0) {
                return 1; /* found */
            }
            /*
             * Didn't find serial EEPROM. Scan all slots
            bscbus->devs[devno]->saddr =
                                   BSC_AT24C32_SADDR +
                                   ((bscbus->devs[devno]->saddr + 1) & 0x3);
        } while ((slot_id & 0x3) != (bscbus->devs[devno]->saddr & 0x3));
             */
        break;

    default:
        break;
    }
err_exit:
#endif /* !PLISIM */

    return 0;
}

static int at24c64_write(int unit, int devno, uint32 addr, uint32 wdata)
{
    soc_bsc_bus_t *bscbus = BSCBUS(unit);
    int saddr = bscbus->devs[devno]->saddr;
    uint8 rxbuf[8], txbuf[8];
    int retv, i, txbuf_len;
    unsigned int timeout, ack;

    if (addr >= AT24C64_DEVICE_SIZE) {
        return SOC_E_PARAM;
    }

    sal_memset(txbuf, 0, sizeof(txbuf));
    sal_memset(rxbuf, 0, sizeof(rxbuf));
    txbuf[0] = (addr & 0xff00) >> 8;	/* address MSB */
    txbuf[1] = addr & 0x00ff;		/* address MLB */
    txbuf[2] = wdata & 0xff;
    txbuf_len = 3;

    BSC_LOCK(unit);

    /*
     * Work on BSC registers preparing read/write transactions
     *
     * Clear Enable Reg
     */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
    if (retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }

    /* Set slave device address */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CHIP_ADDR_REG,
                                            (saddr << 1) & 0xff);
    if (retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }

    for (i = 0; i < txbuf_len; i++) {
        retv = sbFe2000UtilPciIICWrite(unit,
                SB_FE2000_IIC_DATA_IN0_REG + i, (uint32)txbuf[i]);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
            goto err_exit;
        }
    }

    /*
     * Set CNT Reg to
     * transmit 'txbuf_len' bytes and receive 0 bytes
     */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CNT_REG,
                                    (0 << 4) | txbuf_len);
    if(retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }

    /*
     * Set CTRL Reg for write only transfor
     * to setup initial eeprom address for sequencial read or write
     */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CTRL_REG,
                    BSC_DIV_CLK | BSC_SCL_SEL_200KHZ | BSC_WRITE_ONLY);
    if( SB_FE2000_STS_INIT_OK_K != retv ) {
        goto err_exit;
    }

    /* Set Enable Reg to start transfer */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x3);
    if( SB_FE2000_STS_INIT_OK_K != retv ) {
        goto err_exit;
    }

    /* Wait for transfer complete */
    ack = 0;
    timeout = 0;
    while (((ack & 0x2) != 2) &&
                    (timeout < SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT)) {
        retv = sbFe2000UtilPciIICRead(unit,
                                SB_FE2000_IIC_ENABLE_REG, &ack);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
                goto err_exit;
        }
        thin_delay(250);
        timeout++;
    }

    if (SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT == timeout) {
        retv = SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR;
        goto err_exit;
    }

    /* Clear Enable Reg */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
    if (retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }
    BSC_UNLOCK(unit);

    /* Write Cycle Time delay: 10ms with 2.7v */
    sal_usleep(2*LC2464_WRITE_CYCLE_DELAY);	/* 20ms on the safe side */

    return SOC_E_NONE;

err_exit:
    BSC_UNLOCK(unit);
    LOG_CLI((BSL_META_U(unit,
                        "at24c64_write: error\n")));
    return retv;
}

static int at24c64_read(int unit, int devno, uint32 addr, uint32 *rdata)
{
    soc_bsc_bus_t *bscbus = BSCBUS(unit);
    int saddr = bscbus->devs[devno]->saddr;
    uint8 rxbuf[8], txbuf[8];
    int retv, i, txbuf_len, rxbuf_len;
    unsigned int timeout, ack, data;

    if (addr >= AT24C64_DEVICE_SIZE) {
        return SOC_E_PARAM;
    }

    sal_memset(txbuf, 0, sizeof(txbuf));
    sal_memset(rxbuf, 0, sizeof(rxbuf));
    txbuf[0] = (addr & 0xff00) >> 8;		/* address MSB */
    txbuf[1] = addr & 0x00ff;		        /* address MSB */
    txbuf_len = 2;
    rxbuf_len = 1;

    BSC_LOCK(unit);

    /*
     * Work on BSC registers preparing read/write transactions
     *
     * Clear Enable Reg
     */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
    if (retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }

    /* Set slave device address */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CHIP_ADDR_REG,
                                            (saddr << 1) & 0xff);
    if (retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }

    for (i = 0; i < txbuf_len; i++) {
        retv = sbFe2000UtilPciIICWrite(unit,
                SB_FE2000_IIC_DATA_IN0_REG + i, (uint32)txbuf[i]);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
            goto err_exit;
        }
    }

    /*
     * Set CNT Reg to
     * transmit 'txbuf_len' bytes and receive 'rxbuf_len' bytes
     */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CNT_REG,
                                    (rxbuf_len << 4) | txbuf_len);
    if(retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }

    /*
     * Set CTRL Reg for write only transfor
     * to setup initial eeprom address for sequencial read or write
     */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CTRL_REG,
                    BSC_DIV_CLK | BSC_SCL_SEL_200KHZ | BSC_WRITE_THEN_READ);
    if( SB_FE2000_STS_INIT_OK_K != retv ) {
        goto err_exit;
    }

    /* Set Enable Reg to start transfer */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x3);
    if( SB_FE2000_STS_INIT_OK_K != retv ) {
        goto err_exit;
    }

    /* Wait for transfer complete */
    ack = 0;
    timeout = 0;
    while (((ack & 0x2) != 2) &&
                    (timeout < SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT)) {
        retv = sbFe2000UtilPciIICRead(unit,
                                SB_FE2000_IIC_ENABLE_REG, &ack);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
            goto err_exit;
        }
        thin_delay(250);
        timeout++;
    }

    if (SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT == timeout) {
        retv = SB_FE2000_STS_INIT_IIC_READ_TIMEOUT_ERR;
        goto err_exit;
    }

    for (i = 0; i < rxbuf_len; i++) {
        retv = sbFe2000UtilPciIICRead(unit,
                        SB_FE2000_IIC_DATA_OUT0_REG + i, &data);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
            goto err_exit;
        }
        rxbuf[i] = data & 0xff;
    }

    /* Clear Enable Reg */
    retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
    if (retv != SB_FE2000_STS_INIT_OK_K) {
        goto err_exit;
    }
    *rdata = rxbuf[0];

    BSC_UNLOCK(unit);
    return SOC_E_NONE;

err_exit:
    BSC_UNLOCK(unit);
    LOG_CLI((BSL_META_U(unit,
                        "at24c64_read: error\n")));
    return retv;
}

/*
 * Function: soc_eep24c64_read
 *
 * Purpose: Read len bytes of data into buffer, update len with total
 *          amount read.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    addr - NVRAM memory address to read from
 *    data - address of data buffer to read into
 *    len - address containing number of bytes read into data buffer (updated
 *          with number of bytes read on completion).
 *
 * Returns: data bufffer filled in with data from address, number of
 *          bytes read is updated in len field. Status code:
 *
 *          SOC_E_NONE -- no error encounter
 *          SOC_E_TIMEOUT - chip timeout or data error
 *
 * Notes:
 *         Currently uses random address byte read to initiate the read; if
 *         more than one byte of data is requested at the current address, a
 *         sequential read operation is performed.
 */
int
soc_eep24c64_read(int unit, int devno, uint16 addr, uint8* data, uint32 *len)
{
    int retv, i = 0;
    uint32 rdata;

    if (!soc_bsc_is_attached(unit)) {
        soc_bsc_attach(unit);
    }

    if (!len || (*len == 0) || !data ||
                    ((addr + *len) > AT24C64_DEVICE_SIZE)) {
        return SOC_E_PARAM;
    }
    for (i = 0; i < *len; i++) {
        retv = at24c64_read(unit, devno, addr + i, &rdata);
        if (retv != SOC_E_NONE) {
            goto err_exit;
        }
        data[i] = rdata & 0xff;
    }
    *len = i;
    return SOC_E_NONE;

err_exit:
    *len = i;
    return retv;
}

/*
 * Function: eep24c64_write
 * Purpose: Write len bytes of data, return the number of bytes written.
 * Uses PAGE mode to write up to 32 bytes at a time between address
 * stages for maximum performance. See AT24C64 data sheet for more info.
 *
 * Parameters:
 *    unit - StrataSwitch device number or BSC bus number
 *    devno - chip device id
 *    addr - NVRAM memory address to write to
 *    data - address of data buffer to write from
 *    len - number of bytes to write
 *
 * Returns:
 *          SOC_E_NONE -- no error encountered
 *          SOC_E_TIMEOUT - chip timeout or data error
 *
 *
 * Notes:
 *     Uses page mode to write in 32-byte chunks, when an address
 *     which does not begin on a page boundary is provided, the write
 *     operation handles unaligned accesses by breaking up the write
 *     into one write which is not page aligned, and the remainder as
 *     page aligned accesses.
 *
 */
int
soc_eep24c64_write(int unit, int devno, uint16 addr, uint8* data, uint32 len)
{
    int retv, i;
    uint32 wdata;

    if (!soc_bsc_is_attached(unit)) {
        soc_bsc_attach(unit);
    }

    if (!len || !data || ((addr + len) > AT24C64_DEVICE_SIZE))
            return SOC_E_PARAM;

    for (i = 0; i < len; i++) {
        wdata = data[i];
        retv =  at24c64_write(unit, devno, addr + i, wdata);
        if (retv != SOC_E_NONE) {
            goto err_exit;
        }
    }
    return SOC_E_NONE;

err_exit:
    return retv;
}

bsc_driver_t _soc_bsc_at24c64_driver = {
    0x0,			/* flags */
    0x0,			/* devno */
    AT24C32_DEVICE_TYPE,	/* id */
    at24c64_read,		/* read */
    at24c64_write,		/* write */
    at24c64_ioctl,		/* ioctl */
    at24c64_init,		/* init */
};

#else/* BCM_FE2000_SUPPORT && INCLUDE_I2C */
int _soc_i2c_at24c64_not_empty;
#endif /* BCM_FE2000_SUPPORT && INCLUDE_I2C */

