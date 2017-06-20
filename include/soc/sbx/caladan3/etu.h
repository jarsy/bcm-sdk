/*
 * $Id: etu.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    etu.c
 * Purpose: Caladan3 External TCAM driver
 * 
 */

#ifdef BCM_CALADAN3_SUPPORT

#ifndef __ETU_H
#define __ETU_H

#include <soc/sbx/caladan3/etu_tcam.h>

/**
 ** WCL defintions
 **/
#define SOC_SBX_CALADAN3_WCL_NUM 6

#define SOC_SBX_CALADAN3_WCL_NUM_LANES_PER_CORE 4

#define SOC_SBX_CALADAN3_WCL_NUM_LANES_MAX   (SOC_SBX_CALADAN3_WCL_NUM * SOC_SBX_CALADAN3_WCL_NUM_LANES_PER_CORE)  
#define SOC_SBX_CALADAN3_WCL_NUM_TX_LANES    (SOC_SBX_CALADAN3_WCL_NUM * SOC_SBX_CALADAN3_WCL_NUM_LANES_PER_CORE)
#define SOC_SBX_CALADAN3_WCL_NUM_RX_LANES    (3 * SOC_SBX_CALADAN3_WCL_NUM_LANES_PER_CORE)

#define SOC_SBX_CALADAN3_WCL_MASTER_ID 1



/**
 ** ETU defintions
 **/
#define SOC_SBX_CALADAN3_ETU_NUM_PROG_MAX   (512)

/*
 * LRP Key can directly map (1:1) to ETU program
 */
#define SOC_SBX_CALADAN3_ETU_NUM_PROG_LRP_DIRECT   (16)

/*
 * TCAM CXT buffer to use for Host based lookup
 * Lrp uses 0 - 0x1c00, any valid addr beyond this should be ok
 */
#define SOC_SBX_CALADAN3_ETU_LOOKUP_CXT_BUFFER_ADDR  (0x2000)


/*
 * Allow ucode to write to ETU, need additional program to enable write operations
 */ 
#define SOC_SBX_CALADAN3_ETU_PROG_UCODE_WRITE_EN   (0)

/* 
 * Allow ucode to specify the program number as part of the key 
 * This is the extenion mode.
 */
#define SOC_SBX_CALADAN3_ETU_PROG_EXTN_EN   (0)

/*
 * Max usable ETU programs given the configurations above
 */
#define SOC_SBX_CALADAN3_ETU_NUM_PROG_USABLE_MAX   \
               (512 >> SOC_SBX_CALADAN3_ETU_PROG_UCODE_WRITE_EN)

/*
 * Use HW byteswap if applicable
 */
#define SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP   0
#define SOC_SBX_CALADAN3_ETU_NO_HW_BYTE_SWAP  (!(SOC_SBX_CALADAN3_ETU_USE_HW_BYTE_SWAP))

typedef struct etu_tcam_lookup_s {
    uint32 cxt_buf_addr;
    uint8 ltr;
    uint8 *key;
    uint16 key_width_in_bits;
    uint8 num_results;
} etu_tcam_lookup_t;

typedef struct etu_tcam_result_s {
    uint8 valid;
    uint8 hit;
    uint32 result;
} etu_tcam_result_t;


/* Prototypes */
int
soc_sbx_caladan3_etu_fifo_init(int unit);

int
soc_sbx_caladan3_etu_prog_memory_set(int unit, int idx, int ndw, int ns, int ut,
                                     uint32 app0, uint32 app1, uint32 app2);
int
soc_sbx_caladan3_etu_fifo_access(int unit, uint32 mode, uint32 *cw,
                                 uint32 *data, uint32 num_dw, uint32 *response, uint32 write);
int
soc_sbx_caladan3_etu_tcam_reg_read(int unit, int devid, int reg, uint8 *data);
int
soc_sbx_caladan3_etu_tcam_reg_write(int unit, int devid, int reg, 
                                    uint8 *data, uint8 *response);
int
soc_sbx_caladan3_etu_tcam_db_read_datax(int unit, int devid, int index, 
                                        uint8 *data, uint8 *valid);
int
soc_sbx_caladan3_etu_tcam_db_read_masky(int unit, int devid, int index, uint8 *mask);
int
soc_sbx_caladan3_etu_tcam_db_read(int unit, int devid, int index, 
                                  uint8 *data, uint8 *mask, uint8 *valid);
int
soc_sbx_caladan3_etu_tcam_db_write(int unit, int devid, int index, 
                                   uint8 *data, uint8 *mask, 
                                   int valid, int wr_mode, uint8 *response);
int
soc_sbx_caladan3_etu_tcam_db_lookup(int unit, 
                                    etu_tcam_lookup_t *lkup, 
                                    etu_tcam_result_t *results);
int
soc_sbx_caladan3_etu_tcam_cxtbuf_write(int unit, uint32 cxtaddr,
                                       uint8 *data, int keysize);
int
soc_sbx_caladan3_etu_tcam_nop(int unit, uint8 *response);

int
soc_sbx_caladan3_etu_tcam_compare(int unit, uint8 ltr, int cxt_buffer_addr,
				  uint8 *key, uint8 cmp1, int sz,
				  uint8 *response);

int
soc_sbx_caladan3_mdio_portid_get(int unit, int dev);

int
soc_sbx_caladan3_etu_driver_init(int unit);

int
soc_sbx_caladan3_etu_driver_uninit(int unit);

int
soc_sbx_caladan3_etu_tcam_config(int unit);

void
soc_sbx_caladan3_etu_debug_set(int unit, int level);


#endif /* __ETU_H */

#endif /* BCM_CALADAN3_SUPPORT */
