/* $Id: mpc107.h,v 1.2 2011/07/21 16:14:08 yshtil Exp $ */
/* file: mpc107.h - contains MPC107 register numbers and values  
 * Copyright Motorola inc, 1998, all rights reserved.
 * Modification history						 
 * --------------------	
 * 27mar02, jmb Add new registers for Kahlua-2, the MPC8245
 * 14Oct98, My	Added Kahlua support					
 * 05Mar98, My	Created					
 */

/* 824x specific PCI internal Embedded Utilities Block registers */

#define VEN_DEV_ID      0x00021057    	/* Vendor and Dev. ID for MPC106 */
#define KAHLUA_ID	0x00031057    	/* Vendor & Dev Id for Kahlua's PCI */
#define MPC8245_ID	0x00061057    	/* Vendor & Dev Id for MPC 8245 */
#define KAHLUA2_ID	0x00061057    	/* 8245 is aka Kahlua-2 */
#define BMC_BASE	0x80000000      /* Kahlua EUMB:PCI mem-mapped base */

#define LOC_MEM_BAR	0x80000010	/* Local mem base address reg (BAR) */
#define PCSRBAR		0x80000014	/* Peripheral Control & Status BAR */
#define EUMBBAR_REG	0x80000078	/* Embedded Util. Memory Block BAR */

#define PCI_COMMAND	0x80000004	/* MPC106 PCI command register */
#define PCI_STATUS	0x80000006	/* MPC106 PCI status register */
#define OUT_DRV_CONT	0x80000073	/* Output Driver Control register */
#define ODCR_ADR_X	0x80000070	/* Output Driver Control register */
#define ODCR_SHIFT      3
#define PMCR2_ADR	0x80000072	/* Power Mgmnt Cfg 2 register */
#define PMCR2_ADR_X	0x80000070	
#define PMCR2_SHIFT     2
#define PMCR1_ADR	0x80000070	/* Power Mgmnt Cfg 1 reister */
#define MEM_START1_ADR	0x80000080    	/* Memory starting addr */
#define MEM_START2_ADR  0x80000084    	/* Memory starting addr-lower */
#define XMEM_START1_ADR 0x80000088   	/* Extended mem. starting addr-upper */
#define XMEM_START2_ADR 0x8000008c	/* Extended mem. starting addr-lower */
#define MEM_END1_ADR	0x80000090	/* Memory ending address */
#define MEM_END2_ADR	0x80000094	/* Memory ending address-lower */
#define XMEM_END1_ADR   0x80000098	/* Extended mem. ending address-upper*/
#define XMEM_END2_ADR   0x8000009c	/* Extended mem. ending address-lower*/
#define MEM_EN_ADR	0x800000a0	/* Memory bank enable */
#define PAGE_MODE	0x800000a3	/* Page Mode Counter/Timer */
#define MPM_ADR_X	0x800000a0	/* Page Mode Counter/Timer */
#define MPM_SHIFT	3		/* Page Mode Counter/Timer */
#define PROC_INT1_ADR   0x800000a8	/* Processor interface config. 1 */
#define PROC_INT2_ADR   0x800000ac	/* Processor interface config. 2 */
#define SBE_CTR_ADR   	0x800000b8	/* Single-bit error counter */
#define SBE_TRIG_ADR  	0x800000b9	/* Single-bit error trigger thresh. */
#define MEM_ERREN1_ADR  0x800000c0	/* Memory error enable 1 */
#define MEM_ERRDET1_ADR 0x800000c1	/* Memory error detection 1 */
#define MEM_BUSERRS_ADR 0x800000c3	/* Proc. Bus Error status (8245) */
#define MEM_ERREN2_ADR  0x800000c4	/* Memory error enable 2 */
#define MEM_ERRDET2_ADR	0x800000c5	/* Memory error detection 2 */
#define MEM_PCIERRS_ADR	0x800000c7	/* PCI error status */
#define MEM_PCIERRA_ADR	0x800000c8	/* CPU/PCI error address */
#define XROM_CFG1_ADR   0x800000d0	/* Extended ROM cfg reg. 1 (8245) */
#define XROM_CFG2_ADR   0x800000d4	/* Extended ROM cfg reg. 2 (8245) */
#define XROM_CFG3_ADR   0x800000d8	/* Extended ROM cfg reg. 3 (8245) */
#define XROM_CFG4_ADR   0x800000dc	/* Extended ROM cfg reg. 4 (8245) */
#define MAP_OPTS_ADR    0x800000e0	/* Address map B options (8245) */
#define PLL_CFG_ADR     0x800000e2	/* PLL Configuration (8245) */
#define PLL_CFG_ADR_X   0x800000e0	/* PLL Configuration (8245) */
#define PLL_CFG_ADR_SHIFT   2		/* PLL Configuration (8245) */
#define MEM_CONT1_ADR   0x800000f0	/* Memory control config. 1 */
#define MEM_CONT2_ADR   0x800000f4	/* Memory control config. 2 */
#define MEM_CONT3_ADR   0x800000f8	/* Memory control config. 3 */
#define MEM_CONT4_ADR   0x800000fc	/* Memory control config. 4 */

/* 8245 Specific registers */
#define MIOCR1		0x80000076  /* Miscellaneous I/O Control Register 1 */
#define MIOCR1_ADR_X	0x80000074  /* Miscellaneous I/O Control Register 1 */
#define MIOCR1_SHIFT	2
#define MIOCR2		0x80000077  /* Miscellaneous I/O Control Register 2 */
#define MIOCR2_ADR_X	0x80000074  /* Miscellaneous I/O Control Register 1 */
#define MIOCR2_SHIFT	3


#define MOT_VENDOR_ID   0x1057          /* Motorola */
#define MPC107_DEV_ID   0x0003          /* MPC107 */

/* Processor Interface Configuration Register1 (PICR1) */
#define PICR1_CONFIG    0xa8            /* In PCI config space */
#define PICR1_CONFIG_MPC107_ST_GATH_EN      (1<<6) /* Enable cpu->PCI wr st gathering */
#define PICR1_CONFIG_MPC107_PCI_SPEC_RD     (1<<2) /* Speculative PCI reads */


