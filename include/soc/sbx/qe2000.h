/*
 * $Id: qe2000.h,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 (BCM83200) SOC Initialization
 */
#ifndef _QE2000_H
#define _QE2000_H

#include <soc/sbx/sbx_drv.h>

#include <bcm/types.h>
#include <bcm/error.h>

#define HW_QE2000_EB_MEM_MAX_OFFSET          0x7FFF

#define SB_FAB_DEVICE_QE2000_MAX_WRED_TEMPLATES         (16)
#define SB_FAB_DEVICE_QE2000_MAX_PORT                   (50)
#define SB_FAB_DEVICE_QE2000_CPU_PORT                   (49) /* 0 based */

#define SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_SEL        (15)
#define SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_VALUE      (127)

#define SB_FAB_DEVICE_QE2000_MC_PORT_TIMEOUT            (576) /* microseconds: this is 8X the time for a jumbo frame to be XMIT'd out a 10Mbit port */

#define SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_SEL        (15)
#define SB_FAB_DEVICE_QE2000_MAX_PKT_LEN_ADJ_VALUE      (127)

#define SB_FAB_DEVICE_QE2000_MC_PORT_TIMEOUT            (576) /* microseconds: this is 8X the time for a jumbo frame to be XMIT'd out a 10Mbit port */

#define SB_FAB_DEVICE_QE2000_NO_QUEUES_IN_ARB_PORT      (16)

#define SB_FAB_DEVICE_QE2000_ARB_PORT_MAX_PORT          (32)
#define SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_MASK         (0x01F)
#define SB_FAB_DEVICE_QE2000_ARB_PORT_PORT_SHIFT        (0)

#define SB_FAB_DEVICE_QE2000_ARB_PORT_MAX_NODE          (32)
#define SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_MASK         (0x3E0)
#define SB_FAB_DEVICE_QE2000_ARB_PORT_NODE_SHIFT        (5)

#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_PORT_MASK   (0x03F)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_PORT_SHIFT  (0)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_QE1K_MASK   (0x040)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_QE1K_SHIFT  (6)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_EF_MASK     (0x080)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_EF_SHIFT    (7)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_TXDMA_MASK  (0x100)
#define SB_FAB_DEVICE_QE2000_EGRESS_PORT_REMAP_FIC_TXDMA_SHIFT (8)

#define SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_DELAY       (10 * 1000 * 1000) /* 10 msec-thin_delay */
#define SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_DELAY  (10)               /* 10 usec-sal_udelay */
#define SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_COUNT  (100)
#define SB_FAB_DEVICE_QE2000_MULTICAST_NEF_FIFO_OFFSET   (50)

#define SB_FAB_DEVICE_QE2000_CONN_AGE_UNIT               (8) /* 8 us */
#define SB_FAB_DEVICE_QE2000_CONN_AGE_MAX_TEMPLATES      (16)
#define SB_FAB_DEVICE_QE2000_CONN_UTIL_MAX_TEMPLATES     (8)

#define SB_FAB_DEVICE_QE2000_FIFO_EMPTY_DELAY            (1 * 1000 * 1000) /* 1 msec,thin_delay */
#define SB_FAB_DEVICE_QE2000_FIFO_EMPTY_ITER_DELAY       (10)              /* 10 usec,sal_udelay */
#define SB_FAB_DEVICE_QE2000_FIFO_EMPTY_ITER_COUNT       (1000)


#define SB_FAB_DEVICE_QE2000_PORT_FLUSH_DELAY            (1 * 1000 * 1000) /* 1 msec,thin_delay */
#define SB_FAB_DEVICE_QE2000_PORT_FLUSH_ITER_DELAY       (10)              /* 10 usec,sal_udelay */
#define SB_FAB_DEVICE_QE2000_PORT_FLUSH_ITER_COUNT       (1000)

#define SB_FAB_DEVICE_QE2000_INGRESS_SHAPER_MAX_BURST    0x007FFFFF

/*
 *  For configuring min_util, which needs to be configured differently
 *  according to the type of destination, we need to know the type of
 *  destination to know which number of lines_per_* field in the qm_config2
 *  register to use in computing the actual number of lines for min_util from
 *  the percentage that the API uses as representation.
 */
#define SB_FAB_DEVICE_QE2000_QUEUE_DEST_LOCAL            1
#define SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE1000           2
#define SB_FAB_DEVICE_QE2000_QUEUE_DEST_QE2000           3

typedef struct qe2000_ei_port_config_s {
    uint16 dest_channel;
    uint16 size_mask;
    uint16 is_rb_loopback;
    uint16 line_ptr;
    uint16 byte_ptr;
} qe2000_ei_port_config_t;


/* Some QE2000 memories need retry of writes due to contention with hardware access */
#define QE2000_QS_MEM_RETRY_CNT 10

typedef struct qe2000_qs_mem_desc_s {
    sbBool_t     bCheckMemory;
    uint32     nMemMask[5];
} qe2000_qs_mem_desc_t;


extern int
soc_qe2000_spi_init(int unit, soc_sbx_config_t *cfg);

extern soc_error_t
soc_qe2000_config_linkdriver(int unit, int nLink, sbLinkDriverConfig_t *pLinkDriverConfig);

extern soc_error_t
soc_qe2000_q2ec_set(int unit, int nQueue, int nMc, int nNode, int nEPort, int nECos);

extern soc_error_t
soc_qe2000_q2ec_get(int unit, int nQueue, int *pnMc, int *pnNode, int *pnEPort, int *pnECos);

extern soc_error_t
soc_qe2000_rate_a_get(int unit, int queue, int *value);

extern soc_error_t
soc_qe2000_rate_a_set(int unit, int queue, int value);

extern soc_error_t
soc_qe2000_rate_b_get(int unit, int queue, uint32 *value);

extern soc_error_t
soc_qe2000_rate_b_set(int unit, int queue, uint32 value);

extern soc_error_t
soc_qe2000_credit_get(int unit, int queue, uint32 *value);

extern soc_error_t
soc_qe2000_credit_set(int unit, int queue, uint32 value);

extern soc_error_t
soc_qe2000_depth_length_get(int unit, int queue, int * nDepth, int *nPktLen);

extern soc_error_t
soc_qe2000_depth_length_set(int unit, int queue, int nDepth, int nPktLen);

extern void
soc_qe2000_egress_to_spi_port_set(int unit, int nEPort, int nSpi, int nSpiPort);

extern void
soc_qe2000_egress_to_spi_port_get(int unit, int nEPort, int *nSpi, int *nSpiPort);

extern soc_error_t
soc_qe2000_shape_maxburst_get (int unit, int queue, int *shape_enable, int *maxburst);

extern soc_error_t
soc_qe2000_shape_maxburst_set (int unit, int queue, int shape_enable, int maxburst);

extern soc_error_t
soc_qe2000_shape_bucket_get (int unit, int queue, int *shape_bucket);

extern soc_error_t
soc_qe2000_shape_bucket_set (int unit, int queue, int shape_bucket);

extern soc_error_t
soc_qe2000_age_get (int unit, int queue, int *nonempty, int *anemic_event, int *ef_event, int *cnt);

extern soc_error_t
soc_qe2000_age_set (int unit, int queue, int nonempty, int anemic_event, int ef_event, int cnt);

extern soc_error_t
soc_qe2000_age_thresh_key_get (int unit, int queue, int *age_thresh_key);

extern soc_error_t
soc_qe2000_age_thresh_key_set (int unit, int queue, int age_thresh_key);

extern soc_error_t
soc_qe2000_age_thresh_get (int unit, int idx, int *anemic_thresh, int *ef_thres);

extern soc_error_t
soc_qe2000_age_thresh_set (int unit, int idx, int anemic_thresh, int ef_thres);

extern soc_error_t
soc_qe2000_pri_lut_get (int unit, int idx, int *pri, int *next_pri);

extern soc_error_t
soc_qe2000_pri_lut_set (int unit, int idx, int pri, int next_pri);

extern soc_error_t
soc_qe2000_priority_get (int unit, int idx, int *shaped, int *pri, int *next_pri);

extern soc_error_t
soc_qe2000_priority_set (int unit, int idx, int shaped, int pri, int next_pri);

extern soc_error_t
soc_qe2000_lastsentpri_get (int unit, int idx, int *pri, int *next_pri);

extern soc_error_t
soc_qe2000_lastsentpri_set (int unit, int idx, int pri, int next_pri);

extern soc_error_t
soc_qe2000_e2q_set(int unit, int nMc, int nENode, int nEPort, int nQueue, int nEnable);

extern soc_error_t
soc_qe2000_e2q_get(int unit, int nMc, int uENode, int uEPort, int *pnQueue, int *pnEnable);

extern soc_error_t
soc_qe2000_queue_para_get(int unit, int queue, int *local, int *hold_ts, int *q_type);

extern soc_error_t
soc_qe2000_queue_para_set(int unit, int queue, int local, int hold_ts, int q_type);

extern soc_error_t
soc_qe2000_shape_rate_get(int unit, int queue, int *mode, int *shape_rate);

extern soc_error_t
soc_qe2000_shape_rate_set(int unit, int queue, int mode, int shape_rate);

extern soc_error_t
soc_qe2000_sfi_xcnfg_mem_ack_wait( int unit, bcm_port_t port,
				   uint32 uMsTimeout );

extern soc_error_t
soc_qe2000_sfi_rd_wr_xcfg( int unit, int nRead, bcm_port_t port,
			   int modid, bcm_port_t* pXbport );

extern uint32
soc_qe2000_eb_mem_read( int unit,
			uint32 uOffset,
			uint32 puData[8]);

extern int
soc_qe2000_eb_mem_write( int unit,
			 uint32 uOffset,
			 uint32 uData[8]);

extern uint32
soc_qe2000_qs_rank_write(int unit,
			 uint32 uAddress,
			 uint32 uData);

extern uint32
soc_qe2000_qs_rank_read(int unit,
			uint32 uAddress,
			uint32* pData);

extern uint32
soc_qe2000_qs_rand_write(int unit,
			 uint32 uAddress,
			 uint32 uData);

extern uint32
soc_qe2000_qs_rand_read(int unit,
			uint32 uAddress,
			uint32* pData);

extern uint32
soc_qe2000_qs_lna_rank_write(int unit,
			     uint32 uAddress,
			     uint32 uData);

extern uint32
soc_qe2000_qs_lna_rank_read(int unit,
			    uint32 uAddress,
			    uint32* pData);

extern uint32
soc_qe2000_qs_lna_rand_write(int unit,
			     uint32 uAddress,
			     uint32 uData);

extern uint32
soc_qe2000_qs_lna_rand_read(int unit,
			    uint32 uAddress,
			    uint32* pData);

extern uint32
soc_qe2000_qs_mem_write(int unit,
			uint32 nTableId,
			uint32 uAddress,
			uint32 uData);

extern uint32
soc_qe2000_qs_mem_read(int unit,
		       uint32 nTableId,
		       uint32 uAddress,
		       uint32* pData);

extern uint32
soc_qe2000_qs_mem_read_easy_reload(int unit,
                                   uint32 nTableId,
                                   uint32 uAddress,
                                   uint32* pData);

extern uint32
soc_qe2000_qs_mem_lna_write(int unit,
			    uint32 nTableId,
			    uint32 uAddress,
			    uint32 uData4,
			    uint32 uData3,
			    uint32 uData2,
			    uint32 uData1,
			    uint32 uData0);

extern uint32
soc_qe2000_qs_mem_lna_read(int unit,
			   uint32 nTableId,
			   uint32 uAddress,
			   uint32* pData4,
			   uint32* pData3,
			   uint32* pData2,
			   uint32* pData1,
			   uint32* pData0);

extern uint32
soc_qe2000_qm_fb_cache_fifo_write(int unit,
				  uint32 uData);

extern uint32
soc_qe2000_qm_fb_cache_fifo_read(int unit,
				 uint32* pData);

extern uint32
soc_qe2000_qm_mem_write(int unit,
			int32 nAddr,
			int32 nTable,
			uint32 uData0,
			uint32 uData1,
			uint32 uData2,
			uint32 uData3);
extern uint32
soc_qe2000_qm_mem_read(int unit,
		       int32 nAddr,
		       int32 nTable,
		       uint32 *puData0,
		       uint32 *puData1,
		       uint32 *puData2,
		       uint32 *puData3);


extern uint32
soc_qe2000_qm_counter_read(int unit,
        int32 set,
        uint32 *puValue);


extern uint32
soc_qe2000_qm_counter_base_set(int unit,
        int32 nBaseQueue,
        int32 enable);

extern uint32
soc_qe2000_qm_counter_base_get(int unit,
        int32 *nBaseQueue);

extern uint32
soc_qe2000_qstate_mem_read(int unit, int32 nQueue,
			   int32 *pbEnable, int32 *pnAnemicWatermarkSel,
			   int32 *pnMinBuffers, int32 *pnMaxBuffers);

extern uint32
soc_qe2000_qwred_config_set(int unit,
			    int32 queue,
			    int32 template,
			    int32 gain);

extern uint32
soc_qe2000_qdemand_config_read(int unit,
			       int32 queue,
			       int32 *p_rate_delta_max,
			       int32 *p_qla_demand_mask);

extern uint32
soc_qe2000_qdemand_config_set(int unit,
			      int32 queue,
			      int32 rate_delta_max_index,
			      int32 qla_demand_mask);

extern uint32
soc_qe2000_qbyte_adjust_set(int unit,
			    int32 queue,
			    int32 sign,
			    int32 bytes);

extern uint32
soc_qe2000_eg_mem_read(int unit,
		       uint32 uAddress,
		       uint32 uTableId,
		       uint32* pData0,
		       uint32* pData1,
		       uint32* pData2);

int32
soc_qe2000_eg_mem_write(int unit, int32 nAddr, int32 nTable,
			uint32 uData0, uint32 uData1, uint32 uData2);

extern uint32
soc_qe2000_ei_mem_write(int unit,
			uint32 uTableId,
			uint32 uAddress,
			uint32 uData0);

extern uint32
soc_qe2000_ei_mem_read(int unit,
		       uint32 uTableId,
		       uint32 uAddress,
		       uint32* pData0);

extern uint32
soc_qe2000_pm_mem_write(int unit,
			uint32 uAddress,
			uint32 uData3,
			uint32 uData2,
			uint32 uData1,
			uint32 uData0);

extern uint32
soc_qe2000_pm_mem_read(int unit,
		       uint32 uAddress,
		       uint32* pData3,
		       uint32* pData2,
		       uint32* pData1,
		       uint32* pData0);

extern uint32
soc_qe2000_sf_mem_write(int unit,
			uint32 uAddress,
			uint32 uData);


extern uint32
soc_qe2000_sf_mem_read(int unit,
		       uint32 uAddress,
		       uint32* pData);

extern uint32
soc_qe2000_ep_amcl_mem_write(int unit,
			     uint32 uAddress,
			     uint32 uData1,
			     uint32 uData0,
			     sbBool_t bCauseParityError);

extern uint32
soc_qe2000_ep_amcl_mem_read(int unit,
			    uint32 uAddress,
			    uint32 bClearOnRead,
			    uint32* pData1,
			    uint32* pData0);

extern uint32
soc_qe2000_ep_bmbf_mem_write(int unit,
			     uint32 uAddress,
			     uint32 uData0);

extern uint32
soc_qe2000_ep_bmbf_mem_read(int unit,
			    uint32 uAddress,
			    uint32* pData0);

extern uint32
soc_qe2000_ep_mmip_mem_write(int unit,
			     uint32 uAddress,
			     uint32 uData1,
			     uint32 uData0);

extern uint32
soc_qe2000_ep_mmip_mem_read(int unit,
			    uint32 uAddress,
			    uint32 bClearOnRead,
			    uint32* pData1,
			    uint32* pData0);

extern uint32
soc_qe2000_ep_mmbf_mem_write(int unit,
			     uint32 uAddress,
			     uint32 uData1,
			     uint32 uData0);

extern uint32
soc_qe2000_ep_mmbf_mem_read(int unit,
			    uint32 uAddress,
			    sbBool_t bClearOnRead,
			    uint32* pData1,
			    uint32* pData0);

extern uint32
soc_qe2000_rb_pol_mem_write(int unit,
			    uint32 uTableId,
			    uint32 uAddress,
			    uint32 uData);

extern uint32
soc_qe2000_rb_pol_mem_read(int unit,
			   uint32 uTableId,
			   uint32 uAddress,
			   uint32* pData);

extern uint32
soc_qe2000_rb_class_mem_write(int unit,
			      uint32 nSpi,
			      uint32 uTableId,
			      uint32 uAddress,
			      uint32 uData0,
			      uint32 uData1);

extern uint32
soc_qe2000_rb_class_mem_read(int unit,
			     uint32 nSpi,
			     uint32 uTableId,
			     uint32 uAddress,
			     uint32* pData0,
			     uint32* pData1);

extern uint32
soc_qe2000_pm_dll_lut_write(int unit,
			    uint32 uTableId,
			    uint32 uAddress,
			    uint32 uData);

extern uint32
soc_qe2000_pm_dll_lut_read(int unit,
			   uint32 uTableId,
			   uint32 uAddress,
			   uint32* pData);

extern int
soc_qe2000_get_available_queue(int unit, int numCos);

extern int
soc_qe2000_find_base_queue(int unit, int nNode, int port, int nMc);

extern void
soc_qe2000_isr(void *_unit);

extern int
soc_qe2000_init(int unit, soc_sbx_config_t *cfg);

extern int
soc_sbx_qe2000_detach(int unit);

extern soc_error_t
soc_qe2000_queue_info_get(int unit, int q, int *mc, int *node, int *port,
			  int *cos, int* baseq, int *enabled);

extern soc_error_t
soc_qe2000_modid_set(int unit, int node);


extern int32
soc_qe2000_lna_mem_full_remap_table_write(int unit, int32 nVirtualNodePort, int32 nDestPortId);

int32
soc_qe2000_eg_mem_port_remap_table_write(int unit, int32 nVirtualNodePort,
					 sbBool_t bMc, sbBool_t bTme,
					 sbBool_t bEnableFifo, int32 nEgressFifoId);

extern int
soc_qe2000_port_info_config(int unit);

extern int
soc_qe2000_features(int unit, soc_feature_t feature);

uint32
soc_qe2000_rate_delta_config(int unit, uint32 timeslot_size, uint32 epoch_size);

uint32
soc_qe2000_rate_delta_index_get(int unit, uint32 bag_rate, uint32 *index);

extern int
soc_qe2000_queue_min_util_set(int unit, int32 nQueue, int template);

extern int
soc_qe2000_queue_min_util_get(int unit, int32 nQueue, int *template);

extern int
soc_qe2000_queue_max_age_set(int unit, int32 nQueue, int template);

extern int
soc_qe2000_queue_max_age_get(int unit, int32 nQueue, int *template);

extern int
soc_qe2000_template_min_util_recall(int unit, int dest, int hwUtil, int *util);

extern int
soc_qe2000_template_min_util_adjust(int unit, int dest, int util, int *hwUtil);

extern int
soc_qe2000_template_min_util_set(int unit, int32 template, int hwUtil);

extern int
soc_qe2000_template_max_age_recall(int unit, int hwAge, int *age);

extern int
soc_qe2000_template_max_age_adjust(int unit, int age, int *hwAge);

extern int
soc_qe2000_template_max_age_set(int unit, int32 template, int hwAge);

extern int
soc_qe2000_mc_pointer_fifo_set(int unit, int32 fifo, int32 size, int32 base, int32 drop_on_full);

extern int
soc_qe2000_mc_pointer_fifo_get(int unit, int32 fifo, int32 *size, int32 *base, int32 *drop_on_full);

extern int
soc_qe2000_burst_size_lines_get(int unit, int32 *value, int32* value_cong, uint64 xbars);

extern soc_driver_t soc_driver_bcm83200_a3;

#ifdef BCM_WARM_BOOT_SUPPORT
extern int 
_bcm_qe2000_wb_cosq_state_sync(int unit, uint8 **pptr, int flag);
#endif /* BCM_WARM_BOOT_SUPPORT */
#endif
