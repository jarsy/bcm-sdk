/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * $Id: etu_tcam.h,v 1.2 Broadcom SDK $
 * File: etu_tcam.h
 * Purpose: Defintions for the External TCAM
 */

#ifdef BCM_CALADAN3_SUPPORT
#ifndef __ETU_TCAM_H
#define __ETU_TCAM_H

/**
 ** External TCAM definitions
 **/

#define SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_MDIO_ADDR      (0x60)

#define SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_METAFRAME_LEN  (2048)

/*
 * Casade upto 4 TCAMs
 * -- Need to check if this will work with LRP timing
 */
#define SOC_SBX_CALADAN3_ETU_TCAM_DEV_MAX   (4)

/* TCAM is mapped on external bus 3 */
#define SOC_SBX_CALADAN3_ETU_TCAM_MDIO_BUS   (3) /* MIIM_TC */


#define SOC_SBX_CALADAN3_ETU_TCAM_KEY_SIZE_DEFAULT 320   /* default keysize in bits */
#define SOC_SBX_CALADAN3_ETU_TCAM_KEY_SIZE_MAX     640   /* maximum keysize in bits */

/* Following MAX defs are in 32bit words */
#define SOC_SBX_CALADAN3_ETU_TCAM_CW_SIZE_MAX        2     
#define SOC_SBX_CALADAN3_ETU_TCAM_DW_SIZE_MAX     ((SOC_SBX_CALADAN3_ETU_TCAM_KEY_SIZE_MAX) >> 5)
#define SOC_SBX_CALADAN3_ETU_TCAM_RSP_SIZE_MAX       4

#define ETU_DATA_LSW              (0) 
#define ETU_DATA_MSW              (1) 

/* Key constructor */
#define SOC_SBX_CALADAN3_ETU_TCAM_KC_SUBKEYS_MAX      (4)
#define SOC_SBX_CALADAN3_ETU_TCAM_KC_SEGMENT_MAX     (10)
#define SOC_SBX_CALADAN3_ETU_TCAM_KC_SEGMENT_NB_MAX  (15)

/* Results */
#define SOC_SBX_CALADAN3_ETU_TCAM_RESULTS_MAX  (4)
#define SOC_SBX_CALADAN3_ETU_TCAM_RESULTS_PORT_DEFAULT  (0)


#define SOC_SBX_CALADAN3_ETU_TCAM_CXT_BUF_START (0x8000)
#define SOC_SBX_CALADAN3_ETU_TCAM_CXT_BUF_END   (0xBFFF)

/* OPCODES */
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_REG_WRITE          1
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_REG_READ           2
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_WRITE           1
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_DATA_OR_X_READ  2
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_DB_MASK_OR_Y_READ  3
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP1               1 
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CMP2               2
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_CXTBUF_WRITE       4
#define SOC_SBX_CALADAN3_ETU_TCAM_OPCODE_NOP                0

/* Addressing */
#define SOC_SBX_CALADAN3_ETU_TCAM_DB_ACCESS         1
#define SOC_SBX_CALADAN3_ETU_TCAM_REG_ACCESS        0

#define NL_REG_BUF_SIZE  16
typedef uint8 nl_reg_buf_t[NL_REG_BUF_SIZE]; /* Little-endian */


/* Prototypes */
int
soc_etu_nl_mdio_test_reg_access(int unit, unsigned mdio_portid);

int
soc_etu_nl_mdio_init_seq(int unit,
                         unsigned num_nl,
                         unsigned rx_fifo_thr,
                         unsigned tx_fifo_thr,
                         unsigned rx_swap,
                         unsigned tx_swap,
                         unsigned tx_burst_short_16b);
int
soc_etu_nl_mdio_init_ready(int unit, unsigned num_nl);

void
soc_etu_nl_print_register(int unit, char *regname, nl_reg_buf_t buf);

int
soc_etu_nl_device_id_print(int unit, unsigned dev_id);

int
soc_etu_nk_mdio_register_access(int unit, int op, int portid, uint16 dev_id, uint16 regaddr, uint16 *regval);

int
soc_etu_nl_mdio_chk_error_counters_status(int unit, unsigned dev_id, unsigned chk_crx);

int
soc_etu_nl_mdio_print_csm_status(int unit, unsigned dev_id);



#endif /* __ETU_TCAM_H */

#endif /* BCM_CALADAN3_SUPPORT */
