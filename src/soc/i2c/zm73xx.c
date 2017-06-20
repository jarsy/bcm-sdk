/*
 * $Id: zm73xx.c,v 1.16 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *               [BSC] - Broadcom Serical Control (I2C constroller)
 *                 |
 *       ==========O=======o================ I2C bus
 *                         |
 *                    [pca9548 - 8-port mux]
 *                      | | | | | | | |
 *           [eeprom]  -  | | | | | | |
 *            [max6653]  -  | | | | | |
 *             [max6653]   -  | | |
 *               [max6653]   -  | |
 *                 [max6653]   -   - [zm73xx]
 */


#ifdef INCLUDE_I2C
#ifdef BCM_FE2000_SUPPORT

#include <sal/types.h>
#include <soc/debug.h>
#include <soc/drv.h>
#include <soc/error.h>
#include <soc/bsc.h>
#include <soc/zm73xx.h>
#include <soc/sbx/fe2k_common/sbFe2000Common.h>
#include <shared/bsl.h>
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


/* Following table is from ZM7300 Programming Manual, Table 58 */
float vos_to_volt[] = {
	0.5000, 0.5125, 0.5250, 0.5375, 0.5500, 0.5625, 0.5750, 0.5875,
	0.6000, 0.6125, 0.6250, 0.6375, 0.6500, 0.6625, 0.6750, 0.6875,
	0.7000, 0.7125, 0.7250, 0.7375, 0.7500, 0.7625, 0.7750, 0.7875,
	0.8000, 0.8125, 0.8250, 0.8375, 0.8500, 0.8625, 0.8750, 0.8875,
	0.9000, 0.9125, 0.9250, 0.9375, 0.9500, 0.9625, 0.9750, 0.9875,
	1.0000, 1.0125, 1.0250, 1.0375, 1.0500, 1.0625, 1.0750, 1.0875,
	1.1000, 1.1125, 1.1250, 1.1375, 1.1500, 1.1625, 1.1750, 1.1875,
	1.2000, 1.2125, 1.2250, 1.2375, 1.2500, 1.2625, 1.2750, 1.2875,

	1.3000, 1.3125, 1.3250, 1.3375, 1.3500, 1.3625, 1.3750, 1.3875,
	1.4000, 1.4125, 1.4250, 1.4375, 1.4500, 1.4625, 1.4750, 1.4875,
	1.5000, 1.5125, 1.5250, 1.5375, 1.5500, 1.5625, 1.5750, 1.5875,
	1.6000, 1.6125, 1.6250, 1.6375, 1.6500, 1.6625, 1.6750, 1.6875,
	1.7000, 1.7125, 1.7250, 1.7375, 1.7500, 1.7625, 1.7750, 1.7875,
	1.8000, 1.8125, 1.8250, 1.8375, 1.8500, 1.8625, 1.8750, 1.8875,
	1.9000, 1.9125, 1.9250, 1.9375, 1.9500, 1.9625, 1.9750, 1.9875,
	2.0000, 2.0250, 2.0500, 2.0750, 2.1000, 2.1250, 2.1500, 2.1750,

	2.2000, 2.2250, 2.2500, 2.2750, 2.3000, 2.3250, 2.3500, 2.3750,
	2.4000, 2.4250, 2.4500, 2.4750, 2.5000, 2.5250, 2.5500, 2.5750,
	2.6000, 2.6250, 2.6500, 2.6750, 2.7000, 2.7250, 2.7500, 2.7750,
	2.8000, 2.8250, 2.8500, 2.8750, 2.9000, 2.9250, 2.9500, 2.9750,
	3.0000, 3.0250, 3.0500, 3.0750, 3.1000, 3.1250, 3.1500, 3.1750,
	3.2000, 3.2250, 3.2500, 3.2750, 3.3000, 3.3250, 3.3500, 3.3750,
	3.4000, 3.4250, 3.4500, 3.4750, 3.5000, 3.5250, 3.5500, 3.5750,
	3.6000, 3.6250, 3.6500, 3.6750, 3.7000, 3.7250, 3.7500, 3.7750,

	3.8000, 3.8250, 3.8500, 3.8750, 3.9000, 3.9250, 3.9500, 3.9750,
	4.0000, 4.0250, 4.0500, 4.0750, 4.1000, 4.1250, 4.1500, 4.1750,
	4.2000, 4.2250, 4.2500, 4.2750, 4.3000, 4.3250, 4.3500, 4.3750,
	4.4000, 4.4250, 4.4500, 4.4750, 4.5000, 4.5250, 4.5500, 4.5750,
	4.6000, 4.6250, 4.6500, 4.6750, 4.7000, 4.7250, 4.7500, 4.7750,
	4.8000, 4.8250, 4.8500, 4.8750, 4.9000, 4.9250, 4.9500, 4.9750,
	5.0000, 5.0250, 5.0500, 5.0750, 5.1000, 5.1250, 5.1500, 5.1750,
	5.2000, 5.2250, 5.2500, 5.3000, 5.3500, 5.4000, 5.4500, 5.5000,
};

/* Following table is from ZM7300 Programming Manual, Table 35 */
struct pol_coef {
	char *name;
	float vo;
	float vg;
	float vs;
	float io;
	float ig;
	float is;
	float to1;
	float to2;
	float tg;
	float ts;
} pol_coefficient[] = {
/* 0 */		{0},
/* 1 */		{0},
/* 2 */		{0},
/* 3 */		{"ZY7007", 0.0, 1.0, 0.021568,  3.024, 0.928, 0.054406, 0.0, -50.0, 1.0, 0.7843},
/* 4 */		{0},
/* 5 */		{"ZY7010", 0.0, 1.0, 0.021568, 10.266, 0.835, 0.072478, 0.0, -50.0, 1.0, 0.7643},
/* 6 */		{0},
/* 7 */		{"ZY7015", 0.0, 1.0, 0.021568, 10.851, 0.846, 0.112933, 0.0, -50.0, 1.0, 0.7843},
/* 8 */		{"ZY7115", 0.0, 1.0, 0.021568, 10.851, 0.846, 0.112933, 0.0, -50.0, 1.0, 0.7843},
/* 9 */		{0},
/* 0x0A */	{"ZY7120", 0.0, 1.0, 0.021568, 14.070, 0.851, 0.119493, 0.0, -50.0, 1.0, 0.7843},
/* 0x0B */	{0},
/* 0x0C */	{0},
/* 0x0D */	{0},
/* 0x0E */	{0},
/* 0x0F */	{0},
/* 0x10 */	{0},
};

struct pol_coef *get_pol_coef(int pid)
{
	int num_pol = sizeof(pol_coefficient)/sizeof(pol_coefficient[0]);

	if ((pid < 0) || (pid >= num_pol)) {
		return 0;
	}
	return &pol_coefficient[pid];
}

static void show_bsc_regs(int unit);
static int zm73xx_direct_pol_read(int unit, int devno, int pol, int reg, int *data);
static int zm73xx_direct_pol_write(int unit, int devno, int pol, int reg, int data);

/*
 * BSC mode is 'write then read'.
 * It writes txbuf_len bytes then reads back rxbuf_len bytes
 * including the 'return' byte.  So there's at lease reads one byte back.
 * The 'return' byte is specific to ZM73xx,
 *     0: indicates transaction failed, 1: indicates OK.
 */
unsigned int zm73xx_msg_rw(	int unit,
				int devno,
				char *txbuf,
				uint32 txbuf_len,
				char *rxbuf,
				uint32 rxbuf_len)
{
	soc_bsc_bus_t *bscbus = BSCBUS(unit);
	int chan = bscbus->devs[devno]->chan;
	int saddr = bscbus->devs[devno]->saddr;
	int mux = bscbus->devs[devno]->mux;
	int retv, i;
        unsigned int timeout, ack, data;

        /* Max num write bytes, and max num read bytes */
        if ((txbuf_len > 8) || (rxbuf_len > 8) || (rxbuf_len < 1)) {
                return SB_FE2000_STS_INIT_BAD_ARGS;
        }

	BSC_LOCK(unit);

	/* Select MUX to slave device */
	retv = sbFe2000UtilIICWrite(unit, mux, 0, chan);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
	}

	/*
	 * Work on BSC registers preparing 'write then read' transaction
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
        /* Set CNT Reg to transmit 'txbuf_len' bytes and receive 'rxbuf_len' bytes */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CNT_REG,
                                        (rxbuf_len << 4) | txbuf_len);
        if(retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
        }

        /*
         * Set CTRL Reg for 'write then read' transfer
         *
         * Since there's always a return byte,
         * it's always in 'write then read' transfer mode
         */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CTRL_REG, 0x03);
        if( SB_FE2000_STS_INIT_OK_K != retv ) {
		goto err_exit;
        }

        /* Set Enable Reg to start transfer */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x3);
        if( SB_FE2000_STS_INIT_OK_K != retv ) {
		goto err_exit;
        }

        /*Wait for transfer complete */
        ack = 0;
        timeout = 0;
        while (((ack & 0x2) != 2) && (timeout < SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT)) {
                retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_ENABLE_REG, &ack);
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
                retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT0_REG + i, &data);
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

err_exit:
	BSC_UNLOCK(unit);
        return retv;
}

static int zm73xx_read(int unit, int devno, uint32 reg, uint32 *data)
{
	int retv;
	char txbuf[8], rxbuf[8];
	int txbuf_len, rxbuf_len;

	sal_memset(txbuf, 0, sizeof(txbuf));
	txbuf[0] = ZM73_CMD_RRM;
	txbuf[1] = reg & 0xff;
	txbuf_len = 2;

	sal_memset(rxbuf, 0, sizeof(rxbuf));
	rxbuf_len = 2;

	retv = zm73xx_msg_rw(unit, devno, txbuf, txbuf_len, rxbuf, rxbuf_len);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to read dpm reg rel0\n")));
		goto err_exit;
	}
	/* 1: good transaction and zm73 acked */
	if (rxbuf[1] != 1) {
		goto err_exit;
	}
	*data = rxbuf[0];
	return SOC_E_NONE;

err_exit:
	return -1;
}

static int zm73xx_write(int unit, int devno, uint32 reg, uint32 data)
{
	char txbuf[8], rxbuf[8];
	int txbuf_len, rxbuf_len;
	int retv;

	sal_memset(txbuf, 0, sizeof(txbuf));
	txbuf[0] = ZM73_CMD_WRM;
	txbuf[1] = reg & 0xff;
	txbuf[2] = data & 0xff;
	txbuf_len = 3;

	sal_memset(rxbuf, 0, sizeof(rxbuf));
	rxbuf_len = 1;

	retv = zm73xx_msg_rw(unit, devno, txbuf, txbuf_len, rxbuf, rxbuf_len);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to read dpm reg rel0\n")));
		goto err_exit;
	}
	/* 1: good transaction and zm73 acked */
	if (rxbuf[0] != 1) {
		goto err_exit;
	}
	return SOC_E_NONE;

err_exit:
	return -1;
}

static int zm73xx_ioctl(int unit, int devno, int opcode, void* data, int len)
{
        int retv;
	uint32 dpm_reg_rel0, dpm_reg_rel1;

	switch(opcode) {
	case BSC_IOCTL_DEVICE_PRESENT:
		/*
		 * Get the ID and SW release register information,
		 * we need that to know how many POLs we potentially have
		 */
		retv = zm73xx_read(unit, devno, ZM73XX_DPM_REG_REL0, &dpm_reg_rel0);
		if (retv != SOC_E_NONE) {
			return 0; /* not present */
		}

		retv = zm73xx_read(unit, devno, ZM73XX_DPM_REG_REL1, &dpm_reg_rel1);
		if (retv != SOC_E_NONE) {
			return 0; /* not present */
		}

		return 1; /* present */
		break;

	default:
		return 0;
		break;
	}

	return 0;
}

static int zm73xx_init(int unit, int devno)
{
        soc_bsc_bus_t *bscbus;
	struct zm73xx_sc *sc;
	char txbuf[8], rxbuf[8];
	int txbuf_len, rxbuf_len;
	uint32 dpm_reg_rel0, dpm_reg_rel1;
	int i, retv, vos;
	struct pol_coef *pc;

	sc = sal_alloc(sizeof(*sc), "zm73xx_sc");
	if (sc == NULL) {
		return SOC_E_MEMORY;
	}
	sal_memset(sc, 0, sizeof(*sc));

	/*
	 * Allow voltage change.
	 *
	 * Bit-7: user memory read/write enable
	 * Bit-6: reserve
	 * Bit-5: DPM configuration registers read/write enable
	 * Bit-4: reserve
	 * Bit-3: Write commands to POL bypassing DPM read/write enable
	 * Bit-2: reserve
	 * Bit-1: POL set registers read/write enable
	 */
	retv = zm73xx_write(unit, devno, ZM73XX_DPM_REG_WP, 0xff);
	if (retv != SOC_E_NONE) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to write dpm reg ZM73XX_DPM_REG_WP\n")));
		goto err_exit;
	}

	/*
	 * Get the ID and SW release register information,
	 * we need that to know how many POLs we potentially have
	 */
	retv = zm73xx_read(unit, devno, ZM73XX_DPM_REG_REL0, &dpm_reg_rel0);
	if (retv != SOC_E_NONE) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to read dpm reg ZM73XX_DPM_REG_REL0\n")));
		goto err_exit;
	}

	retv = zm73xx_read(unit, devno, ZM73XX_DPM_REG_REL1, &dpm_reg_rel1);
	if (retv != SOC_E_NONE) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to read dpm reg ZM73XX_DPM_REG_REL1\n")));
		goto err_exit;
	}

	/*
	 * Parse ZM73xx dpm_reg_rel0 and dpm_reg_rel1
	 */
	sc->dpm_gen = dpm_reg_rel0 & 0x0F;
        /*
	if (sc->dpm_gen > 1) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: DPM Generation (0x%X) not recognized!\n"), sc->dpm_gen));
		goto err_exit;
	}
        */

	sc->fw_idx = (dpm_reg_rel0 & 0xF0) >> 4;

	sc->fw_ver = dpm_reg_rel1 & 0x0F;
	dpm_reg_rel1 = (dpm_reg_rel1 & 0xF0) >> 4;

	switch (dpm_reg_rel1) {
	case 0:
		sc->numpols = 4;
		break;
	case 2:
		sc->numpols = 8;
		break;
	case 6:
		sc->numpols = 16;
		break;
	case 14:
		sc->numpols = 32;
		break;
	default:
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: DPM Rel1 Reg decode error, 0x%x, "
                                    "number of POLs unknown\n"),
                         dpm_reg_rel1));
		goto err_exit;
	}

	LOG_CLI((BSL_META_U(unit,
                            "Found ZM7%d%02d fw %c.%d\n"),
                 sc->dpm_gen == 0 ? 1:3, sc->numpols,
                 'A' + sc->fw_ver, sc->fw_idx));

	/*
	 * Now that we know the number of POLs we can go an feinth the IDs.
	 * If the ID comes back 00, then there is no POL attached
	 * Due to BSC limitation (can only read max 8 bytes in one transaction
	 * we are not able to use ZM73_CMD_RPID here.
	 * ZM73_CMD_RPID can read 32 POL regisers at once.
	 */
	for (i = 0; i < sc->numpols; i++) {
		sal_memset(txbuf, 0, sizeof(txbuf));
		txbuf[0] = ZM73_CMD_RRM;
		txbuf[1] = ZM73XX_DPM_REG_PID + i;
		txbuf_len = 2;

		sal_memset(rxbuf, 0, sizeof(rxbuf));
		rxbuf_len = 2;

		retv = zm73xx_msg_rw(unit, devno, txbuf, txbuf_len, rxbuf, rxbuf_len);
		if (retv != SB_FE2000_STS_INIT_OK_K) {
			LOG_CLI((BSL_META_U(unit,
                                            "zm73xx: unable to read dpm reg rel0\n")));
			goto err_exit;
		}
		/* 1: good transaction and zm73 acked */
		if (rxbuf[1] != 1) {
			goto err_exit;
		}
		sc->pids[i] = rxbuf[0]; /* POL id */
	}

	for (i = 0; i < sc->numpols; i++) {
		pc = get_pol_coef(sc->pids[i]);
		if (pc->name == 0) {
			sc->pids[i] = 0; /* unknown type */
		}
		retv = zm73xx_direct_pol_read(unit, devno, i, 0x07, &vos);
		if (retv != SOC_E_NONE) {
			goto err_exit;
		}
	}

	/* Add soft control */
        bscbus = BSCBUS(unit);
	bscbus->devs[devno]->sc = sc;

	return SOC_E_NONE;

err_exit:
	if (sc) {
		sal_free(sc);
	}
	return -1;
}

bsc_driver_t _soc_bsc_zm73xx_driver = {
	0x0,			/* flags */
	0x0,			/* devno */
	ZM7300_DEVICE_TYPE,	/* id */
	zm73xx_read,
	zm73xx_write,
	zm73xx_ioctl,
	zm73xx_init,
};

/* Useful debug routine */
static void show_bsc_regs(int unit)
{
	uint32 data;
	int retv;

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_CHIP_ADDR_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_CHIP_ADDR_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN0_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN0_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN1_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN1_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN2_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN2_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN3_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN3_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN4_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN4_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN5_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN5_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN6_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN6_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_IN7_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_IN7_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_CNT_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_CNT_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_CTRL_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_CTRL_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_ENABLE_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_ENABLE_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT0_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT0_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT1_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT1_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT2_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT2_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT3_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT3_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT4_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT4_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT5_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT5_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT6_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT6_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT7_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_DATA_OUT7_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_CTRL_HI_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_CTRL_HI_REG: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_PARAM_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- SB_FE2000_IIC_PARAM_REG: 0x%02x\n"), data));

err_exit:
	return;
}


/*
 * Usefult debug routine
 * Mux channel is pre-set
 */
void soc_bsc_readdpm(int unit)
{
	int retv;
        unsigned int timeout, ack, data;

	/* Make sure that we're already attached, or go get attached */
	if (!soc_bsc_is_attached(unit)) {
		soc_bsc_attach(unit);
	}

	BSC_LOCK(unit);

	/*
	 * Work on BSC registers preparing 'write then read' transaction
	 *
         * Clear Enable Reg
	 */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
        }

        /* Set slave device address */
        retv = sbFe2000UtilPciIICWrite(unit,
					SB_FE2000_IIC_CHIP_ADDR_REG, BSC_ZM73XX_SADDR << 1);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
        }

	/* First data byte */
	retv = sbFe2000UtilPciIICWrite(unit,
					SB_FE2000_IIC_DATA_IN0_REG, ZM73_CMD_RRM);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
	}

	/* Second data byte */
	retv = sbFe2000UtilPciIICWrite(unit,
					SB_FE2000_IIC_DATA_IN1_REG, ZM73XX_DPM_REG_REL0);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
	}

	/* Third data byte */
	retv = sbFe2000UtilPciIICWrite(unit,
					SB_FE2000_IIC_DATA_IN2_REG, 0);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
	}

        /* Set CNT Reg to transmit 3 bytes and receive 2 bytes */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CNT_REG, 0x22);
        if(retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
        }

        /*
         * Set CTRL Reg for 'write then read' transfer
         *
         * Since there's always a return byte,
         * it's always in 'write then read' transfer mode
         */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_CTRL_REG, 0x03);
        if( SB_FE2000_STS_INIT_OK_K != retv ) {
		goto err_exit;
        }

	show_bsc_regs(unit);

        /* Set Enable Reg to start transfer */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x3);
        if( SB_FE2000_STS_INIT_OK_K != retv ) {
		goto err_exit;
        }

        /*Wait for transfer complete */
        ack = 0;
        timeout = 0;
        while (((ack & 0x2) != 2) && (timeout < SB_FE2000_IIC_SLAVE_WAIT_TIMEOUT)) {
                retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_ENABLE_REG, &ack);
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

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT0_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- ZM73XX_DPM_REG_REL0: 0x%02x\n"), data));

	retv = sbFe2000UtilPciIICRead(unit, SB_FE2000_IIC_DATA_OUT1_REG, &data);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
			goto err_exit;
	}
	LOG_CLI((BSL_META_U(unit,
                            " -- return byte: 0x%02x\n"), data));

        /* Clear Enable Reg */
        retv = sbFe2000UtilPciIICWrite(unit, SB_FE2000_IIC_ENABLE_REG, 0x0);
        if (retv != SB_FE2000_STS_INIT_OK_K) {
		goto err_exit;
        }

err_exit:
	BSC_UNLOCK(unit);
        return;
}

static int zm73xx_direct_pol_read(int unit, int devno, int pol, int reg, int *data)
{
        int retv;
	char txbuf[8], rxbuf[8];
	int txbuf_len, rxbuf_len;

	sal_memset(txbuf, 0, sizeof(txbuf));
	txbuf[0] = ZM73_CMD_RRP;
	txbuf[1] = pol & 0xff;
	txbuf[2] = reg & 0xff;
	txbuf_len = 3;

	sal_memset(rxbuf, 0, sizeof(rxbuf));
	rxbuf_len = 2;

	retv = zm73xx_msg_rw(unit, devno, txbuf, txbuf_len, rxbuf, rxbuf_len);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to read pol %d reg %x\n"), pol, reg));
		return SOC_E_FAIL;
	}
	/* 1: good transaction and zm73 acked */
	if (rxbuf[1] != 1) {
		return SOC_E_FAIL; /* not present */
	}

	/* return good byte */
	*data = rxbuf[0];

        return SOC_E_NONE;
}

static int zm73xx_direct_pol_write(int unit, int devno, int pol, int reg, int data)
{
	char txbuf[8], rxbuf[8];
	int txbuf_len, rxbuf_len;
	unsigned int polmask = 1 << pol;
	int retv;

	sal_memset(txbuf, 0, sizeof(txbuf));
	txbuf[0] = ZM73_CMD_WRP;
	txbuf[1] = (polmask >> 0) & 0xff;
	txbuf[2] = (polmask >> 8) & 0xff;
	txbuf[3] = (polmask >> 16) & 0xff;
	txbuf[4] = (polmask >> 24) & 0xff;
	txbuf[5] = reg & 0xff;
	txbuf[6] = data & 0xff;
	txbuf_len = 7;

	sal_memset(rxbuf, 0, sizeof(rxbuf));
	rxbuf_len = 1;
#if 000 /* useful debug lines */
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[0]: %x\n"), txbuf[0]));
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[1]: %x\n"), txbuf[1]));
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[2]: %x\n"), txbuf[2]));
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[3]: %x\n"), txbuf[3]));
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[4]: %x\n"), txbuf[4]));
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[5]: %x\n"), txbuf[5]));
	LOG_CLI((BSL_META_U(unit,
                            " -- txbuf[6]: %x\n"), txbuf[6]));
#endif
	retv = zm73xx_msg_rw(unit, devno, txbuf, txbuf_len, rxbuf, rxbuf_len);
	if (retv != SB_FE2000_STS_INIT_OK_K) {
		LOG_CLI((BSL_META_U(unit,
                                    "zm73xx: unable to write pol %d reg %x\n"), pol, reg));
		return SOC_E_FAIL;
	}
	/* 1: good transaction and zm73 acked */
	if (rxbuf[0] != 1) {
		return SOC_E_FAIL; /* not present */
	}

        return SOC_E_NONE;
}

void zm73xx_dpm_reg_show(int unit, int devno)
{
	int data = 0;
	int i, retv;

	LOG_CLI((BSL_META_U(unit,
                            " -- DPM Setup Regs --")));
	for (i = 0; i < NUM_DPM_REGS; i++) {
		if ((i % 16) == 0) {
			LOG_CLI((BSL_META_U(unit,
                                            "\n 0x%02x:"), i));
		} else if ((i % 8) == 0) {
			LOG_CLI((BSL_META_U(unit,
                                            " - ")));
		}
		retv = zm73xx_read(unit, devno, i, (uint32*)&data);
		if (retv != SOC_E_NONE) {
			LOG_CLI((BSL_META_U(unit,
                                            "\nzm73xx: unable to read dpm reg %d\n"), i));
		} else {
            LOG_CLI((BSL_META_U(unit,
                                " %02x"), data));
                }
	}
	LOG_CLI((BSL_META_U(unit,
                            "\n")));
}

void zm73xx_direct_pol_show(int unit, int devno, int pol)
{
	int data = 0;
	int i, retv;

	LOG_CLI((BSL_META_U(unit,
                            " -- POL Setup Regs --")));
	for (i = 0; i < NUM_POL_REGS; i++) {
		if ((i % 16) == 0) {
			LOG_CLI((BSL_META_U(unit,
                                            "\n 0x%02x:"), i));
		} else if ((i % 8) == 0) {
			LOG_CLI((BSL_META_U(unit,
                                            " - ")));
		}
		retv = zm73xx_direct_pol_read(unit, devno, pol, i, &data);
		if (retv != SOC_E_NONE) {
			LOG_CLI((BSL_META_U(unit,
                                            "\nzm73xx: unable to read pol reg %d\n"), i));
		} else {
                        LOG_CLI((BSL_META_U(unit,
                                            " %02x"), data));
                }
	}
	LOG_CLI((BSL_META_U(unit,
                            "\n")));
}

int soc_bsc_zm73xx_set(int unit, int pol, float volt)
{
	int devno, retv, vos, vos_cur, num_volt;
	soc_bsc_bus_t *bscbus;
	int num_bsc_devices;
	struct zm73xx_sc *sc;
	float volt_cur, volt_delta;

	/* Make sure that we're already attached, or go get attached */
	if (!soc_bsc_is_attached(unit)) {
		soc_bsc_attach(unit);
	}

	bscbus = BSCBUS(unit);
	num_bsc_devices = soc_bsc_device_count(unit);

	/*
	 * Search for DPM device (devno)
	 * It's reasonable to think that only one DPM exist and needed.
	 */
	for (devno = 0; devno < num_bsc_devices; devno++) {
		if (bscbus->devs[devno] && (bscbus->devs[devno]->saddr == BSC_ZM73XX_SADDR)) {
			break;
		}
	}
	if (devno >= num_bsc_devices) {
		LOG_CLI((BSL_META_U(unit,
                                    "No DPM device found\n")));
		return SOC_E_FAIL;
	}

	sc = bscbus->devs[devno]->sc;
	if (!sc) {
		return SOC_E_FAIL;
	}

	if ((pol < 0) || (pol >= MAX_POLS) || (!sc->pids[pol])) {
		LOG_CLI((BSL_META_U(unit,
                                    "No pol for this DPM device found\n")));
		return SOC_E_FAIL;
	}

	retv = zm73xx_direct_pol_read(unit, devno, pol, ZM73XX_POL_REG_VOS, &vos_cur);
	if (retv != SOC_E_NONE) {
		LOG_CLI((BSL_META_U(unit,
                                    "Error: zm73xx_direct_pol_read ZM73XX_POL_REG_VOS\n")));
		return SOC_E_FAIL;
	}
	volt_cur = vos_to_volt[vos_cur];
	/* Only allow to change volt within 10% */
	if (volt > volt_cur) {
		volt_delta = volt - volt_cur;
	} else {
		volt_delta = volt_cur - volt;
	}
	if (volt_delta > volt_cur * 0.1) {
		LOG_CLI((BSL_META_U(unit,
                                    "Error: new setting has to be within 10%% "
                                    "of current : %6.4fv\n"),
                         volt_cur));
		return SOC_E_FAIL;
	}

	/* Check legal voltage range */
	num_volt = sizeof(vos_to_volt)/sizeof(float);
	if ((volt < vos_to_volt[0]) || (volt > vos_to_volt[num_volt - 1])) {
		LOG_CLI((BSL_META_U(unit,
                                    "Error: setting volt out of range: %6.4f .. %6.4f\n"),
                         vos_to_volt[0], vos_to_volt[num_volt - 1]));
	}

	for (vos = 0; vos < num_volt; vos++) {
		if (volt <= vos_to_volt[vos]) {
			break;
		}
	}

	retv = zm73xx_direct_pol_write(unit, devno, pol, ZM73XX_POL_REG_VOS, vos);
	if (retv != SOC_E_NONE) {
		LOG_CLI((BSL_META_U(unit,
                                    "Error: writing 0x%02x ZM73XX_POL_REG_VOS\n"), vos));
		return SOC_E_FAIL;
	}
	LOG_CLI((BSL_META_U(unit,
                            "Set POL %d: %6.4fv\n"), pol, vos_to_volt[vos & 0xff]));

#if 000 /* useful debug routines */
	LOG_CLI((BSL_META_U(unit,
                            " -- set VOS: 0x%02x vos_cur: 0x%02x\n"), vos, vos_cur));
	zm73xx_dpm_reg_show(unit, devno);
	zm73xx_direct_pol_show(unit, devno, pol);
#endif
	return SOC_E_NONE;
}

static void zm73xx_thread(void *unitp)
{
        soc_bsc_bus_t *bscbus;
        int unit;

        unit = PTR_TO_INT(unitp);
        bscbus = BSCBUS(unit);

        while (bscbus->mon_delay) {
                LOG_CLI((BSL_META_U(unit,
                                    "\n -- unit %d: dpm monitoring every "
                                    "%d seconds --\n"),
                         unit, bscbus->mon_delay));
                soc_bsc_zm73xx_show(unit);
                sal_usleep(bscbus->mon_delay * SECOND_USEC);
        }
        LOG_CLI((BSL_META_U(unit,
                            "unit %d: dpm monitoring completed\n"), unit));
        sal_thread_exit(0);
}

int soc_bsc_zm73xx_show(int unit)
{
	int devno, retv, i, vos, vom, iom, tmp;
	soc_bsc_bus_t *bscbus;
	int num_bsc_devices;
	struct zm73xx_sc *sc;
	struct pol_coef *pc;
	float watts = 0, watts_x = 0;

	/* Make sure that we're already attached, or go get attached */
	if (!soc_bsc_is_attached(unit)) {
		soc_bsc_attach(unit);
	}

	bscbus = BSCBUS(unit);
	num_bsc_devices = soc_bsc_device_count(unit);

	/* Search for DPM device (devno) */
	for (devno = 0; devno < num_bsc_devices; devno++) {
		if (bscbus->devs[devno] && (bscbus->devs[devno]->saddr == BSC_ZM73XX_SADDR)) {
			break;
		}
	}
	if (devno >= num_bsc_devices) {
		LOG_CLI((BSL_META_U(unit,
                                    "No DPM device found\n")));
		return SOC_E_FAIL;
	}

	sc = bscbus->devs[devno]->sc;
	if (!sc) {
		return SOC_E_FAIL;
	}

	for (i = 0; i < sc->numpols; i++) {
		if (!sc->pids[i]) {
			continue;
		}
		pc = get_pol_coef(sc->pids[i]);
		if (pc->name == 0) {
			return SOC_E_FAIL;
		}
		retv = zm73xx_direct_pol_read(unit, devno, i, ZM73XX_POL_REG_VOS, &vos);
		if (retv != SOC_E_NONE) {
			return SOC_E_FAIL;
		}

		retv = zm73xx_direct_pol_read(unit, devno, i, ZM73XX_POL_REG_VOM, &vom);
		if (retv != SOC_E_NONE) {
			return SOC_E_FAIL;
		}
		retv = zm73xx_direct_pol_read(unit, devno, i, ZM73XX_POL_REG_IOM, &iom);
		if (retv != SOC_E_NONE) {
			return SOC_E_FAIL;
		}
		retv = zm73xx_direct_pol_read(unit, devno, i, ZM73XX_POL_REG_TOM, &tmp);
		if (retv != SOC_E_NONE) {
			return SOC_E_FAIL;
		}

		watts_x = ((vom + pc->vo) * pc->vg * pc->vs) * ((iom + pc->io) * pc->ig * pc->is);
		watts += watts_x;
		LOG_CLI((BSL_META_U(unit,
                                    " POL %d (%s): %6.4fv(%6.4fv) %6.3f amps "
                                    "%5.2fC  %7.4f watts\n"),
                         i, pc->name,
                         (vom + pc->vo) * pc->vg * pc->vs,
                         vos_to_volt[vos & 0xff],
                         (iom + pc->io) * pc->ig * pc->is,
                         (tmp + pc->to1) * pc->tg * pc->ts + pc->to2,
                         watts_x));
	}

	LOG_CLI((BSL_META_U(unit,
                            " Total power consumption: %7.4f watts\n"), watts));

	return SOC_E_NONE;
}

void soc_bsc_zm73xx_watch(int unit, int enable, int nsecs)
{
        soc_bsc_bus_t *bscbus;

        if (!soc_bsc_is_attached(unit)) {
                soc_bsc_attach(unit);
        }
        bscbus = BSCBUS(unit);

        if (!enable) {
                bscbus->mon_delay = 0;
                return;
        }
        bscbus->mon_delay = nsecs;

        sal_thread_create("",   SAL_THREAD_STKSZ,
                                50,
                                (void (*)(void*))zm73xx_thread,
                                INT_TO_PTR(unit));
        LOG_CLI((BSL_META_U(unit,
                            "unit %d: dpm monitoring enabled\n"), unit));
}

#else
int _zm73xx_c_bcm_fe2000_support_not_empty;
#endif /* BCM_FE2000_SUPPORT */
int _zm73xx_c_not_empty;
#endif /* INCLUDE_I2C */
