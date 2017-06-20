#ifndef _AIMEMC_H
#define _AIMEMC_H

/* $Id: aimemc.h,v 1.2 2011/07/21 16:14:28 yshtil Exp $
 * PL341 Specific Parameters
 */
#define MEMC_BURST_LENGTH       (4)


/*
 * PLL clock configuration
 */
#ifndef CFG_DDR_PLL_CLOCK
#define CFG_DDR_PLL_CLOCK   (300)
#endif /* !CFG_DDR_PLL_CLOCK */
#define PLL_NDIV_INT_VAL    (16 * (CFG_DDR_PLL_CLOCK) / 100)


/*
 * Convenient macros
 */
#define MEMCYCLES(psec)   ((psec) * (CFG_DDR_PLL_CLOCK) / 1000000)


/*
 * DDR23PHY registers
 */
#define DDR23PHY_PLL_STATUS             0x010
#define DDR23PHY_PLL_CONFIG             0x014
#define DDR23PHY_PLL_PRE_DIVIDER        0x018
#define DDR23PHY_PLL_DIVIDER            0x01c
#define DDR23PHY_STATIC_VDL_OVERRIDE    0x030
#define DDR23PHY_ZQ_PVT_COMP_CTL        0x03c
#define DDR23PHY_BL3_VDL_CALIBRATE      0x104
#define DDR23PHY_BL3_VDL_STATUS         0x108
#define DDR23PHY_BL3_READ_CONTROL       0x130
#define DDR23PHY_BL3_WR_PREAMBLE_MODE   0x148
#define DDR23PHY_BL2_VDL_CALIBRATE      0x204
#define DDR23PHY_BL2_VDL_STATUS         0x208
#define DDR23PHY_BL2_READ_CONTROL       0x230
#define DDR23PHY_BL2_WR_PREAMBLE_MODE   0x248
#define DDR23PHY_BL1_VDL_CALIBRATE      0x304
#define DDR23PHY_BL1_VDL_STATUS         0x308
#define DDR23PHY_BL1_READ_CONTROL       0x330
#define DDR23PHY_BL1_WR_PREAMBLE_MODE   0x348
#define DDR23PHY_BL0_VDL_CALIBRATE      0x404
#define DDR23PHY_BL0_VDL_STATUS         0x408
#define DDR23PHY_BL0_READ_CONTROL       0x430
#define DDR23PHY_BL0_WR_PREAMBLE_MODE   0x448

/*
 * PL341 registers
 */
#define PL341_memc_status               0x000
#define PL341_memc_cmd                  0x004
#define PL341_direct_cmd                0x008
#define PL341_memory_cfg                0x00c
#define PL341_refresh_prd               0x010
#define PL341_cas_latency               0x014
#define PL341_write_latency             0x018
#define PL341_t_mrd                     0x01c
#define PL341_t_ras                     0x020
#define PL341_t_rc                      0x024
#define PL341_t_rcd                     0x028
#define PL341_t_rfc                     0x02c
#define PL341_t_rp                      0x030
#define PL341_t_rrd                     0x034
#define PL341_t_wr                      0x038
#define PL341_t_wtr                     0x03c
#define PL341_t_xp                      0x040
#define PL341_t_xsr                     0x044
#define PL341_t_esr                     0x048
#define PL341_memory_cfg2               0x04c
#define PL341_memory_cfg3               0x050
#define PL341_t_faw                     0x054
#define PL341_chip_0_cfg                0x200
#define PL341_user_config0              0x304

/*
 * Values for PL341 Direct Command Register
 */
#define MCHIP_CMD_PRECHARGE_ALL         (0x0 << 18)
#define MCHIP_CMD_AUTO_REFRESH          (0x1 << 18)
#define MCHIP_CMD_MODE_REG              (0x2 << 18)
#define MCHIP_CMD_NOP                   (0x3 << 18)
#define MCHIP_MODEREG_SEL(x)            ((x) << 16)
#define MCHIP_MR_WRITE_RECOVERY(x)      (((x) - 1) << 9)
#define MCHIP_MR_DLL_RESET(x)           ((x) << 8)
#define MCHIP_MR_CAS_LATENCY(x)         ((x) << 4)
#if MEMC_BURST_LENGTH == 4
#define MCHIP_MR_BURST_LENGTH           (2)
#else
#define MCHIP_MR_BURST_LENGTH           (3)
#endif
#define MCHIP_EMR1_DLL_DISABLE(x)       ((x) << 0)
#define MCHIP_EMR1_RTT_ODT_DISABLED     (0)
#define MCHIP_EMR1_RTT_75_OHM           (1 << 2)
#define MCHIP_EMR1_RTT_150_OHM          (1 << 6)
#define MCHIP_EMR1_RTT_50_OHM           ((1 << 6) | (1 << 2))
#define MCHIP_EMR1_OCD_CALI_EXIT        (0x0 << 7)
#define MCHIP_EMR1_OCD_CALI_DEFAULT     (0x3 << 7)


#endif  /* _AIMEMC_H */
