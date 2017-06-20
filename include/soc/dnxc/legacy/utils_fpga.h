
/* $Id: utils_fpga.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef _DNXC_UTILS_FPGA_H_
#define _DNXC_UTILS_FPGA_H_

#define SOC_DNX_FPGA_BUFF_SIZE (80 * 1024 * 1024)

#ifdef __DUNE_WRX_BCM_CPU__
/* REGS ADDR */
#define SOC_DNX_GPOUTDR_PIN_REG_ADDR    0x34108 /* 0x000E0040 */
#define SOC_DNX_GPINDR_PIN_REG_ADDR     0x34110 /* 0x000E0050 */

/* GPIO pins used for FPGA */
#define GPIO_CONFDONE_PIN_READ 16 /* connector pin 49 */
#define GPIO_NCONFIG_PIN_WRITE 12 /* connector pin 54 */
#define GPIO_DATA0_PIN_WRITE   13 /* connector pin 50 */
#define GPIO_DCLK_PIN_WRITE    14 /* connector pin 48 */
#define GPIO_NSTATUS_PIN_READ  11 /* connector pin 58 */
#define GPIO_CONFDONE_MASK     (1 << GPIO_CONFDONE_PIN_READ)
#define GPIO_NCONFIG_MASK      (1 << GPIO_NCONFIG_PIN_WRITE)
#define GPIO_DATA0_MASK        (1 << GPIO_DATA0_PIN_WRITE)
#define GPIO_DCLK_MASK         (1 << GPIO_DCLK_PIN_WRITE)
#define GPIO_NSTATUS_MASK      (1 << GPIO_NSTATUS_PIN_READ)

/* GPOUT REGISTER BITS */
#define SOC_DNX_GPOUTDR_NCONFIG_BIT   GPIO_NCONFIG_MASK /*     0x00200000 */ /* gpout_1 = gpout[10] revert bits */
#define SOC_DNX_GPOUTDR_DATA_BIT      GPIO_DATA0_MASK /*   0x00100000 */ /* gpout_2 = gpout[11] revert bits */
#define SOC_DNX_GPOUTDR_DCLK_BIT      GPIO_DCLK_MASK /*   0x00080000 */ /* gpout_3 = gpout[12] revert bits */

/* GPIN REGISTER BITS */
#define SOC_DNX_GPINDR_CONF_DONE_BIT  GPIO_CONFDONE_MASK /*  0x00800000 */ /* gpin_5 = gpin[8] revert bits */
#define SOC_DNX_GPINDR_NSTATUS_BIT    GPIO_NSTATUS_MASK /*    0x00400000 */ /* gpin_0 = gpin[9] revert bits */

#else /* __DUNE_WRX_BCM_CPU__ */

/* REGS ADDR */
#define SOC_DNX_GPOUTDR_PIN_REG_ADDR	0x000E0040
#define SOC_DNX_GPINDR_PIN_REG_ADDR		0x000E0050

/* GPOUT REGISTER BITS */
#define SOC_DNX_GPOUTDR_NCONFIG_BIT		0x00200000 /* gpout_1 = gpout[10] revert bits */
#define SOC_DNX_GPOUTDR_DATA_BIT		0x00100000 /* gpout_2 = gpout[11] revert bits */
#define SOC_DNX_GPOUTDR_DCLK_BIT		0x00080000 /* gpout_3 = gpout[12] revert bits */

/* GPIN REGISTER BITS */
#define SOC_DNX_GPINDR_CONF_DONE_BIT	0x00800000 /* gpin_5 = gpin[8] revert bits */
#define SOC_DNX_GPINDR_NSTATUS_BIT		0x00400000 /* gpin_0 = gpin[9] revert bits */

#endif

typedef struct
{
  int prog_reg_addr; /* data */
  int status_reg_addr;
  char conf_done_bit_offset; /* offset of done bit in status register */  
} SocDppFpgaRegsMapping;

extern int soc_dnx_fpga_load(int unit, char *file_name);
#endif /* _DNXC_UTILS_FPGA_H_ */
