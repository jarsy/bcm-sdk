/*
 * $Id: lrp.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * lrp.h : LRP defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_LRP_H_
#define _SBX_CALADN3_LRP_H_

#define SOC_SBX_CALADAN3_LR_NUM_OF_STREAMS      (12)
#define SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS       (1536)
#define SOC_SBX_CALADAN3_LR_NUM_OF_PE           (64)
#define SOC_SBX_CALADAN3_LR_INST_MEM_ECC_BYTES  (2)
#define SOC_SBX_CALADAN3_LR_INST_MEM_INST_BYTES (12)
#define SOC_SBX_CALADAN3_LR_INST_NUM_BANKS      (2)

#define SOC_SBX_CALADAN3_LR_CONTEXTS_MAX         8
#define SOC_SBX_CALADAN3_LR_CONTEXTS_RSVD        3
#define SOC_SBX_CALADAN3_LR_CONTEXTS_USER_MAX    \
             (SOC_SBX_CALADAN3_LR_CONTEXTS_MAX - SOC_SBX_CALADAN3_LR_CONTEXTS_RSVD)

#define SOC_SBX_CALADAN3_LR_EPOCH_LENGTH_MIN     426
#define SOC_SBX_CALADAN3_LR_EPOCH_LENGTH_MAX              \
		SOC_SBX_CALADAN3_LR_NUM_OF_INSTRS
#define SOC_SBX_CALADAN3_LR_FRAMES_PER_CONTEXT_MAX 128

#define SOC_SBX_CALADAN3_DUPLEX_MODE            1
#define SOC_SBX_CALADAN3_SIMPLEX_MODE           0

#define SOC_SBX_CALADAN3_LR_STREAM_ONLINE(lrp, stream) \
                 ((lrp)->streams_online | (1 << (stream))

#define SOC_SBX_CALADAN3_LR_IS_STREAM_ONLINE(lrp, stream) \
                 ((lrp)->streams_online & (1 << (stream)) ? 1 : 0)


/* LRP */
typedef struct soc_sbx_caladan3_lrp_s {
    int  epoch_length;
    int  num_context;
    int  detected_context[SOC_SBX_CALADAN3_LR_INST_NUM_BANKS];
    int  frames_per_context;
    int  num_active_pe;
    int  bank_select;
    int   debug_load;
    int   debug_len;
    uint8 bypass;
    uint8 duplex;
    uint8 loader_enable;
    uint32 streams_online;
    int   init_done;
    int   ucode_done;
    int   ucode_reload;
    uint32 oam_num_endpoints;
    lra_inst_b0_mem0_entry_t *s01;
} soc_sbx_caladan3_lrp_t;


extern int soc_sbx_caladan3_lr_driver_init(int unit);

int soc_sbx_caladan3_lr_iwrite(void* handle, uint32 snum, 
                               uint32 inum, uint8 *inst);

int soc_sbx_caladan3_lr_iread(void *handle, uint32 snum, 
                              uint32 inum, uint8 *inst);

extern int soc_sbx_caladan3_lr_host_bubble(int unit,
                                  int stream,
                                  int task,
                                  uint32 id);
int
soc_sbx_caladan3_lr_bubble_enable(int unit, uint32 enable, uint32 size_in_bytes);

extern int soc_sbx_caladan3_lr_shared_register_iaccess(int unit,
                                  int operation,
                                  uint32 address,
                                  uint32 *data);

extern int soc_sbx_lrp_setup_tmu_program(int unit, 
				  int table_index,
				  uint32 program, 
                                  int update,
				  uint8 key0valid,
                                  uint8 key1valid );
int
soc_sbx_caladan3_lr_isr_enable(int unit);
int
soc_sbx_caladan3_lrb_isr_enable(int unit);


#endif /* _SBX_CALADN3_LRP_H_ */
