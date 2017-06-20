/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: ocm.h,v 1.18 Broadcom SDK $
 *
 * ocm.h : OCM defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_OCM_H_
#define _SBX_CALADN3_OCM_H_


typedef enum sbx_caladan3_ocm_port_e {
    SOC_SBX_CALADAN3_OCM_PORT_INVALID = -1,
    SOC_SBX_CALADAN3_OCM_LRP0_PORT = 0,
    SOC_SBX_CALADAN3_OCM_LRP1_PORT = 1,
    SOC_SBX_CALADAN3_OCM_LRP2_PORT = 2,
    SOC_SBX_CALADAN3_OCM_LRP3_PORT = 3,
    SOC_SBX_CALADAN3_OCM_LRP4_PORT = 4,
    SOC_SBX_CALADAN3_OCM_LRP5_PORT = 5,
    SOC_SBX_CALADAN3_OCM_CMU0_PORT = 6,
    SOC_SBX_CALADAN3_OCM_COP0_PORT = 7,
    SOC_SBX_CALADAN3_OCM_LRP_BUBBLE_PORT = 8,
    SOC_SBX_CALADAN3_OCM_LRP6_PORT = 9,
    SOC_SBX_CALADAN3_OCM_LRP7_PORT = 10,
    SOC_SBX_CALADAN3_OCM_LRP8_PORT = 11,
    SOC_SBX_CALADAN3_OCM_LRP9_PORT = 12,
    SOC_SBX_CALADAN3_OCM_CMU1_PORT = 13,
    SOC_SBX_CALADAN3_OCM_COP1_PORT = 14,
    SOC_SBX_CALADAN3_MAX_OCM_PORT
} sbx_caladan3_ocm_port_e_t;

typedef enum sbx_caladan3_ocm_seg_datum_size_e {
    SOC_SBX_CALADAN3_DATUM_SIZE_BIT =1,      /* 1 bit  */
    SOC_SBX_CALADAN3_DATUM_SIZE_DBIT=2,      /* 2 bit  */
    SOC_SBX_CALADAN3_DATUM_SIZE_NIBBLE=4,    /* 4 bit  */
    SOC_SBX_CALADAN3_DATUM_SIZE_BYTE=8,      /* 8 bit  */
    SOC_SBX_CALADAN3_DATUM_SIZE_WORD=16,     /*16 bit  */
    SOC_SBX_CALADAN3_DATUM_SIZE_LONGWORD=32, /* 32 bit */
    SOC_SBX_CALADAN3_DATUM_SIZE_QUADWORD=64, /* 64 bit */
    SOC_SBX_CALADAN3_DATUM_SIZE_MAX
} sbx_caladan3_ocm_seg_datum_size_t;

/* segment 0 on all segmented ports are reserved for dma */
#define _SOC_SBX_CALADAN3_DMA_SEGMENT (0)
#define _SOC_SBX_MAX_LEN_TBL_NAME 16

#define _SOC_SBX_OCM_MAX_TABLE_ID 64
#define _SOC_SBX_OCM_MAX_BANK_ID 64


#define SOC_SBX_CALADAN3_OCM_NUM_MEM (2)
#define SOC_SBX_CALADAN3_OCM_PHY_BLK_PER_OCM (64)
extern uint32 ocm_mem_bank_occupation[][SOC_SBX_CALADAN3_OCM_NUM_MEM];
extern uint32 per_port_ocm_mem_bank_occupation[][SOC_SBX_CALADAN3_MAX_OCM_PORT][SOC_SBX_CALADAN3_OCM_NUM_MEM];

typedef struct sbx_caladan3_ocm_port_alloc_s {
    int port;
    int segment;
    int size;
    sbx_caladan3_ocm_seg_datum_size_t datum_size;

    /* Optional associated data from creator of segments. 
     * For correlation of state info.
     * MUST be set to zero if NOT used.
     */
    int table_id;
    int bank_id;
} sbx_caladan3_ocm_port_alloc_t;

extern uint32 ocm_total_alloc_size_in_bits[];

extern const char *soc_sbx_ocm_port_num_to_name(sbx_caladan3_ocm_port_e_t port_num);

extern int soc_sbx_caladan3_ocm_hw_init(int unit);

extern int soc_sbx_caladan3_ocm_driver_init(int unit);

extern int soc_sbx_caladan3_ocm_driver_uninit(int unit);

extern int soc_sbx_caladan3_ocm_port_mem_read(int unit, 
                                              sbx_caladan3_ocm_port_e_t port,
                                              int segment,
                                              int index_min, 
                                              int index_max,
                                              uint32 *entry_data);

extern int soc_sbx_caladan3_ocm_port_mem_write(int unit, 
                                               sbx_caladan3_ocm_port_e_t port,
                                               int segment, 
                                               int index_min,
                                               int index_max,
                                               uint32 *entry_data);

extern int soc_sbx_caladan3_ocm_port_segment_alloc(int unit, 
                                                   sbx_caladan3_ocm_port_alloc_t *desc);

extern int soc_sbx_caladan3_ocm_port_segment_free(int unit, 
                                                  sbx_caladan3_ocm_port_alloc_t *desc);

extern int soc_sbx_caladan3_ocm_port_mem_alloc(int unit, 
                                               sbx_caladan3_ocm_port_alloc_t *desc);

extern int soc_sbx_caladan3_ocm_port_mem_free(int unit, 
                                              sbx_caladan3_ocm_port_alloc_t *desc);

extern void soc_sbx_caladan3_ocm_util_allocator_dump(int unit, 
                                                     sbx_caladan3_ocm_port_e_t port);
#endif /* _SBX_CALADN3_OCM_H_ */
