
/*
 *$Id: hal_qe_auto_ex.h,v 1.1.2.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

/*
  #defines work with HAL _IDX routines, providing indexed access to register sets.
 */
#ifndef HAL_QE_AUTO_EX_H 
#define HAL_QE_AUTO_EX_H 



#ifndef SAND_HAL_REG_OFFSET_IDX
#define SAND_HAL_REG_OFFSET_IDX(device,reg,idx) \
	(SAND_HAL_##device##_##reg##_OFFSET + \
	SAND_HAL_##device##_##reg##_STRIDE*(idx))

#define SAND_HAL_READ_IDX(addr,device,reg,idx) SAND_HAL_READ_OFFS((addr), \
	SAND_HAL_REG_OFFSET_IDX(device,reg,(idx)))

#define SAND_HAL_WRITE_IDX(addr,device,reg,idx,value) SAND_HAL_WRITE_OFFS((addr), \
	SAND_HAL_REG_OFFSET_IDX(device,reg,(idx)),(value))

#define SAND_HAL_RMW_FIELD_IDX(addr,device,reg,idx,field,fieldValue) \
	do { \
	UINT nRegValue; \
	nRegValue=SAND_HAL_READ_IDX(addr,device,reg,idx); \
	nRegValue=SAND_HAL_MOD_FIELD(device,reg,field,nRegValue,(fieldValue)); \
	SAND_HAL_WRITE_IDX(addr,device,reg,idx,nRegValue); \
	}while(0)
#endif

/* Registers */

#define SAND_HAL_QE_EGRESS_BYTE_CNT_OFFSET SAND_HAL_QE_EGRESS0_BYTE_CNT_OFFSET
#define SAND_HAL_QE_EGRESS_BYTE_CNT_MASK SAND_HAL_QE_EGRESS0_BYTE_CNT_MASK
#define SAND_HAL_QE_EGRESS_BYTE_CNT_MSB SAND_HAL_QE_EGRESS0_BYTE_CNT_MSB
#define SAND_HAL_QE_EGRESS_BYTE_CNT_LSB SAND_HAL_QE_EGRESS0_BYTE_CNT_LSB

#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_OFFSET SAND_HAL_QE_ST_P0_TX_BYTE_CNT_OFFSET
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_MASK SAND_HAL_QE_ST_P0_TX_BYTE_CNT_MASK
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_MSB SAND_HAL_QE_ST_P0_TX_BYTE_CNT_MSB
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_LSB SAND_HAL_QE_ST_P0_TX_BYTE_CNT_LSB

#define SAND_HAL_QE_SI_STICKY_STATE_STATUS_OFFSET SAND_HAL_QE_SI0_STICKY_STATE_STATUS_OFFSET
#define SAND_HAL_QE_SI_STICKY_STATE_STATUS_MASK SAND_HAL_QE_SI0_STICKY_STATE_STATUS_MASK
#define SAND_HAL_QE_SI_STICKY_STATE_STATUS_MSB SAND_HAL_QE_SI0_STICKY_STATE_STATUS_MSB
#define SAND_HAL_QE_SI_STICKY_STATE_STATUS_LSB SAND_HAL_QE_SI0_STICKY_STATE_STATUS_LSB

#define SAND_HAL_QE_SI_STATE_STATUS_OFFSET SAND_HAL_QE_SI0_STATE_STATUS_OFFSET
#define SAND_HAL_QE_SI_STATE_STATUS_MASK SAND_HAL_QE_SI0_STATE_STATUS_MASK
#define SAND_HAL_QE_SI_STATE_STATUS_MSB SAND_HAL_QE_SI0_STATE_STATUS_MSB
#define SAND_HAL_QE_SI_STATE_STATUS_LSB SAND_HAL_QE_SI0_STATE_STATUS_LSB

#define SAND_HAL_QE_SI_CONFIG1_OFFSET SAND_HAL_QE_SI0_CONFIG1_OFFSET
#define SAND_HAL_QE_SI_CONFIG1_MASK SAND_HAL_QE_SI0_CONFIG1_MASK
#define SAND_HAL_QE_SI_CONFIG1_MSB SAND_HAL_QE_SI0_CONFIG1_MSB
#define SAND_HAL_QE_SI_CONFIG1_LSB SAND_HAL_QE_SI0_CONFIG1_LSB

#define SAND_HAL_QE_SCI_PACK_INJECT_DATA_OFFSET SAND_HAL_QE_SCI0_PACK_INJECT_DATA_OFFSET
#define SAND_HAL_QE_SCI_PACK_INJECT_DATA_MASK SAND_HAL_QE_SCI0_PACK_INJECT_DATA_MASK
#define SAND_HAL_QE_SCI_PACK_INJECT_DATA_MSB SAND_HAL_QE_SCI0_PACK_INJECT_DATA_MSB
#define SAND_HAL_QE_SCI_PACK_INJECT_DATA_LSB SAND_HAL_QE_SCI0_PACK_INJECT_DATA_LSB

#define SAND_HAL_QE_EGRESS_DROP_PKT_CNT_OFFSET SAND_HAL_QE_EGRESS0_DROP_PKT_CNT_OFFSET
#define SAND_HAL_QE_EGRESS_DROP_PKT_CNT_MASK SAND_HAL_QE_EGRESS0_DROP_PKT_CNT_MASK
#define SAND_HAL_QE_EGRESS_DROP_PKT_CNT_MSB SAND_HAL_QE_EGRESS0_DROP_PKT_CNT_MSB
#define SAND_HAL_QE_EGRESS_DROP_PKT_CNT_LSB SAND_HAL_QE_EGRESS0_DROP_PKT_CNT_LSB

#define SAND_HAL_QE_ST_P_TX_PKT_CNT_OFFSET SAND_HAL_QE_ST_P0_TX_PKT_CNT_OFFSET
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_MASK SAND_HAL_QE_ST_P0_TX_PKT_CNT_MASK
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_MSB SAND_HAL_QE_ST_P0_TX_PKT_CNT_MSB
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_LSB SAND_HAL_QE_ST_P0_TX_PKT_CNT_LSB

#define SAND_HAL_QE_SI_ERROR_OFFSET SAND_HAL_QE_SI0_ERROR_OFFSET
#define SAND_HAL_QE_SI_ERROR_MASK SAND_HAL_QE_SI0_ERROR_MASK
#define SAND_HAL_QE_SI_ERROR_MSB SAND_HAL_QE_SI0_ERROR_MSB
#define SAND_HAL_QE_SI_ERROR_LSB SAND_HAL_QE_SI0_ERROR_LSB

#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_OFFSET SAND_HAL_QE_SR_P0_RX_BYTE_CNT_OFFSET
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_MASK SAND_HAL_QE_SR_P0_RX_BYTE_CNT_MASK
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_MSB SAND_HAL_QE_SR_P0_RX_BYTE_CNT_MSB
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_LSB SAND_HAL_QE_SR_P0_RX_BYTE_CNT_LSB

#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_OFFSET SAND_HAL_QE_ST_TX_W0_WRITE_DATA_OFFSET
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_MASK SAND_HAL_QE_ST_TX_W0_WRITE_DATA_MASK
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_MSB SAND_HAL_QE_ST_TX_W0_WRITE_DATA_MSB
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_LSB SAND_HAL_QE_ST_TX_W0_WRITE_DATA_LSB

#define SAND_HAL_QE_SR_P_FRM_SIZE_OFFSET SAND_HAL_QE_SR_P0_FRM_SIZE_OFFSET
#define SAND_HAL_QE_SR_P_FRM_SIZE_MASK SAND_HAL_QE_SR_P0_FRM_SIZE_MASK
#define SAND_HAL_QE_SR_P_FRM_SIZE_MSB SAND_HAL_QE_SR_P0_FRM_SIZE_MSB
#define SAND_HAL_QE_SR_P_FRM_SIZE_LSB SAND_HAL_QE_SR_P0_FRM_SIZE_LSB

#define SAND_HAL_QE_SI_ERROR_MASK_OFFSET SAND_HAL_QE_SI0_ERROR_MASK_OFFSET
#define SAND_HAL_QE_SI_ERROR_MASK_MASK SAND_HAL_QE_SI0_ERROR_MASK_MASK
#define SAND_HAL_QE_SI_ERROR_MASK_MSB SAND_HAL_QE_SI0_ERROR_MASK_MSB
#define SAND_HAL_QE_SI_ERROR_MASK_LSB SAND_HAL_QE_SI0_ERROR_MASK_LSB

#define SAND_HAL_QE_SI_STATUS_OFFSET SAND_HAL_QE_SI0_STATUS_OFFSET
#define SAND_HAL_QE_SI_STATUS_MASK SAND_HAL_QE_SI0_STATUS_MASK
#define SAND_HAL_QE_SI_STATUS_MSB SAND_HAL_QE_SI0_STATUS_MSB
#define SAND_HAL_QE_SI_STATUS_LSB SAND_HAL_QE_SI0_STATUS_LSB

#define SAND_HAL_QE_SR_P_RX_PKT_CNT_OFFSET SAND_HAL_QE_SR_P0_RX_PKT_CNT_OFFSET
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_MASK SAND_HAL_QE_SR_P0_RX_PKT_CNT_MASK
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_MSB SAND_HAL_QE_SR_P0_RX_PKT_CNT_MSB
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_LSB SAND_HAL_QE_SR_P0_RX_PKT_CNT_LSB

#define SAND_HAL_QE_SI_DEBUG_MUX_OFFSET SAND_HAL_QE_SI0_DEBUG_MUX_OFFSET
#define SAND_HAL_QE_SI_DEBUG_MUX_MASK SAND_HAL_QE_SI0_DEBUG_MUX_MASK
#define SAND_HAL_QE_SI_DEBUG_MUX_MSB SAND_HAL_QE_SI0_DEBUG_MUX_MSB
#define SAND_HAL_QE_SI_DEBUG_MUX_LSB SAND_HAL_QE_SI0_DEBUG_MUX_LSB

#define SAND_HAL_QE_SI_DEBUG_MUX_SEL_OFFSET SAND_HAL_QE_SI0_DEBUG_MUX_SEL_OFFSET
#define SAND_HAL_QE_SI_DEBUG_MUX_SEL_MASK SAND_HAL_QE_SI0_DEBUG_MUX_SEL_MASK
#define SAND_HAL_QE_SI_DEBUG_MUX_SEL_MSB SAND_HAL_QE_SI0_DEBUG_MUX_SEL_MSB
#define SAND_HAL_QE_SI_DEBUG_MUX_SEL_LSB SAND_HAL_QE_SI0_DEBUG_MUX_SEL_LSB

#define SAND_HAL_QE_PCI_RXBUF_LOAD_OFFSET SAND_HAL_QE_PCI0_RXBUF_LOAD_OFFSET
#define SAND_HAL_QE_PCI_RXBUF_LOAD_MASK SAND_HAL_QE_PCI0_RXBUF_LOAD_MASK
#define SAND_HAL_QE_PCI_RXBUF_LOAD_MSB SAND_HAL_QE_PCI0_RXBUF_LOAD_MSB
#define SAND_HAL_QE_PCI_RXBUF_LOAD_LSB SAND_HAL_QE_PCI0_RXBUF_LOAD_LSB

#define SAND_HAL_QE_SI_DEBUG_01_OFFSET SAND_HAL_QE_SI0_DEBUG_01_OFFSET
#define SAND_HAL_QE_SI_DEBUG_01_MASK SAND_HAL_QE_SI0_DEBUG_01_MASK
#define SAND_HAL_QE_SI_DEBUG_01_MSB SAND_HAL_QE_SI0_DEBUG_01_MSB
#define SAND_HAL_QE_SI_DEBUG_01_LSB SAND_HAL_QE_SI0_DEBUG_01_LSB

#define SAND_HAL_QE_EGRESS_PKT_CNT_OFFSET SAND_HAL_QE_EGRESS0_PKT_CNT_OFFSET
#define SAND_HAL_QE_EGRESS_PKT_CNT_MASK SAND_HAL_QE_EGRESS0_PKT_CNT_MASK
#define SAND_HAL_QE_EGRESS_PKT_CNT_MSB SAND_HAL_QE_EGRESS0_PKT_CNT_MSB
#define SAND_HAL_QE_EGRESS_PKT_CNT_LSB SAND_HAL_QE_EGRESS0_PKT_CNT_LSB

#define SAND_HAL_QE_SI_DEBUG_02_OFFSET SAND_HAL_QE_SI0_DEBUG_02_OFFSET
#define SAND_HAL_QE_SI_DEBUG_02_MASK SAND_HAL_QE_SI0_DEBUG_02_MASK
#define SAND_HAL_QE_SI_DEBUG_02_MSB SAND_HAL_QE_SI0_DEBUG_02_MSB
#define SAND_HAL_QE_SI_DEBUG_02_LSB SAND_HAL_QE_SI0_DEBUG_02_LSB

#define SAND_HAL_QE_SI_DEBUG_03_OFFSET SAND_HAL_QE_SI0_DEBUG_03_OFFSET
#define SAND_HAL_QE_SI_DEBUG_03_MASK SAND_HAL_QE_SI0_DEBUG_03_MASK
#define SAND_HAL_QE_SI_DEBUG_03_MSB SAND_HAL_QE_SI0_DEBUG_03_MSB
#define SAND_HAL_QE_SI_DEBUG_03_LSB SAND_HAL_QE_SI0_DEBUG_03_LSB

#define SAND_HAL_QE_EGRESS_DROP_BYTE_CNT_OFFSET SAND_HAL_QE_EGRESS0_DROP_BYTE_CNT_OFFSET
#define SAND_HAL_QE_EGRESS_DROP_BYTE_CNT_MASK SAND_HAL_QE_EGRESS0_DROP_BYTE_CNT_MASK
#define SAND_HAL_QE_EGRESS_DROP_BYTE_CNT_MSB SAND_HAL_QE_EGRESS0_DROP_BYTE_CNT_MSB
#define SAND_HAL_QE_EGRESS_DROP_BYTE_CNT_LSB SAND_HAL_QE_EGRESS0_DROP_BYTE_CNT_LSB


/* Register Strides */

#define SAND_HAL_QE_EGRESS_BYTE_CNT_STRIDE 4
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_STRIDE 4
#define SAND_HAL_QE_SI_STICKY_STATE_STATUS_STRIDE 4
#define SAND_HAL_QE_SI_STATE_STATUS_STRIDE 4
#define SAND_HAL_QE_SI_CONFIG1_STRIDE 4
#define SAND_HAL_QE_SCI_PACK_INJECT_DATA_STRIDE 4
#define SAND_HAL_QE_EGRESS_DROP_PKT_CNT_STRIDE 4
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_STRIDE 4
#define SAND_HAL_QE_SI_ERROR_STRIDE 4
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_STRIDE 4
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_STRIDE 4
#define SAND_HAL_QE_SR_P_FRM_SIZE_STRIDE 4
#define SAND_HAL_QE_SI_ERROR_MASK_STRIDE 4
#define SAND_HAL_QE_SI_STATUS_STRIDE 4
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_STRIDE 4
#define SAND_HAL_QE_SI_DEBUG_MUX_STRIDE 4
#define SAND_HAL_QE_SI_DEBUG_MUX_SEL_STRIDE 4
#define SAND_HAL_QE_PCI_RXBUF_LOAD_STRIDE 4
#define SAND_HAL_QE_SI_DEBUG_01_STRIDE 4
#define SAND_HAL_QE_EGRESS_PKT_CNT_STRIDE 4
#define SAND_HAL_QE_SI_DEBUG_02_STRIDE 4
#define SAND_HAL_QE_SI_DEBUG_03_STRIDE 4
#define SAND_HAL_QE_EGRESS_DROP_BYTE_CNT_STRIDE 4

/* Register Fields */

#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_WR_DATA_MASK SAND_HAL_QE_ST_TX_W0_WRITE_DATA_W0_WR_DATA_MASK
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_WR_DATA_SHIFT SAND_HAL_QE_ST_TX_W0_WRITE_DATA_W0_WR_DATA_SHIFT
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_WR_DATA_MSB SAND_HAL_QE_ST_TX_W0_WRITE_DATA_W0_WR_DATA_MSB
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_WR_DATA_LSB SAND_HAL_QE_ST_TX_W0_WRITE_DATA_W0_WR_DATA_LSB
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_WR_DATA_TYPE SAND_HAL_QE_ST_TX_W0_WRITE_DATA_W0_WR_DATA_TYPE
#define SAND_HAL_QE_ST_TX_W_WRITE_DATA_WR_DATA_DEFAULT SAND_HAL_QE_ST_TX_W0_WRITE_DATA_W0_WR_DATA_DEFAULT

#define SAND_HAL_QE_SR_P_FRM_SIZE_MIN_FRM_SIZE_MASK SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MIN_FRM_SIZE_MASK
#define SAND_HAL_QE_SR_P_FRM_SIZE_MIN_FRM_SIZE_SHIFT SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MIN_FRM_SIZE_SHIFT
#define SAND_HAL_QE_SR_P_FRM_SIZE_MIN_FRM_SIZE_MSB SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MIN_FRM_SIZE_MSB
#define SAND_HAL_QE_SR_P_FRM_SIZE_MIN_FRM_SIZE_LSB SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MIN_FRM_SIZE_LSB
#define SAND_HAL_QE_SR_P_FRM_SIZE_MIN_FRM_SIZE_TYPE SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MIN_FRM_SIZE_TYPE
#define SAND_HAL_QE_SR_P_FRM_SIZE_MIN_FRM_SIZE_DEFAULT SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MIN_FRM_SIZE_DEFAULT

#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_RX_BYTE_CNT_MASK SAND_HAL_QE_SR_P0_RX_BYTE_CNT_P0_RX_BYTE_CNT_MASK
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_RX_BYTE_CNT_SHIFT SAND_HAL_QE_SR_P0_RX_BYTE_CNT_P0_RX_BYTE_CNT_SHIFT
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_RX_BYTE_CNT_MSB SAND_HAL_QE_SR_P0_RX_BYTE_CNT_P0_RX_BYTE_CNT_MSB
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_RX_BYTE_CNT_LSB SAND_HAL_QE_SR_P0_RX_BYTE_CNT_P0_RX_BYTE_CNT_LSB
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_RX_BYTE_CNT_TYPE SAND_HAL_QE_SR_P0_RX_BYTE_CNT_P0_RX_BYTE_CNT_TYPE
#define SAND_HAL_QE_SR_P_RX_BYTE_CNT_RX_BYTE_CNT_DEFAULT SAND_HAL_QE_SR_P0_RX_BYTE_CNT_P0_RX_BYTE_CNT_DEFAULT

#define SAND_HAL_QE_ST_P_TX_PKT_CNT_TX_PKT_CNT_MASK SAND_HAL_QE_ST_P0_TX_PKT_CNT_P0_TX_PKT_CNT_MASK
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_TX_PKT_CNT_SHIFT SAND_HAL_QE_ST_P0_TX_PKT_CNT_P0_TX_PKT_CNT_SHIFT
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_TX_PKT_CNT_MSB SAND_HAL_QE_ST_P0_TX_PKT_CNT_P0_TX_PKT_CNT_MSB
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_TX_PKT_CNT_LSB SAND_HAL_QE_ST_P0_TX_PKT_CNT_P0_TX_PKT_CNT_LSB
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_TX_PKT_CNT_TYPE SAND_HAL_QE_ST_P0_TX_PKT_CNT_P0_TX_PKT_CNT_TYPE
#define SAND_HAL_QE_ST_P_TX_PKT_CNT_TX_PKT_CNT_DEFAULT SAND_HAL_QE_ST_P0_TX_PKT_CNT_P0_TX_PKT_CNT_DEFAULT

#define SAND_HAL_QE_SR_P_RX_PKT_CNT_RX_PKT_CNT_MASK SAND_HAL_QE_SR_P0_RX_PKT_CNT_P0_RX_PKT_CNT_MASK
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_RX_PKT_CNT_SHIFT SAND_HAL_QE_SR_P0_RX_PKT_CNT_P0_RX_PKT_CNT_SHIFT
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_RX_PKT_CNT_MSB SAND_HAL_QE_SR_P0_RX_PKT_CNT_P0_RX_PKT_CNT_MSB
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_RX_PKT_CNT_LSB SAND_HAL_QE_SR_P0_RX_PKT_CNT_P0_RX_PKT_CNT_LSB
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_RX_PKT_CNT_TYPE SAND_HAL_QE_SR_P0_RX_PKT_CNT_P0_RX_PKT_CNT_TYPE
#define SAND_HAL_QE_SR_P_RX_PKT_CNT_RX_PKT_CNT_DEFAULT SAND_HAL_QE_SR_P0_RX_PKT_CNT_P0_RX_PKT_CNT_DEFAULT

#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_TX_BYTE_CNT_MASK SAND_HAL_QE_ST_P0_TX_BYTE_CNT_P0_TX_BYTE_CNT_MASK
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_TX_BYTE_CNT_SHIFT SAND_HAL_QE_ST_P0_TX_BYTE_CNT_P0_TX_BYTE_CNT_SHIFT
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_TX_BYTE_CNT_MSB SAND_HAL_QE_ST_P0_TX_BYTE_CNT_P0_TX_BYTE_CNT_MSB
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_TX_BYTE_CNT_LSB SAND_HAL_QE_ST_P0_TX_BYTE_CNT_P0_TX_BYTE_CNT_LSB
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_TX_BYTE_CNT_TYPE SAND_HAL_QE_ST_P0_TX_BYTE_CNT_P0_TX_BYTE_CNT_TYPE
#define SAND_HAL_QE_ST_P_TX_BYTE_CNT_TX_BYTE_CNT_DEFAULT SAND_HAL_QE_ST_P0_TX_BYTE_CNT_P0_TX_BYTE_CNT_DEFAULT

#define SAND_HAL_QE_SR_P_FRM_SIZE_MAX_FRM_SIZE_MASK SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MAX_FRM_SIZE_MASK
#define SAND_HAL_QE_SR_P_FRM_SIZE_MAX_FRM_SIZE_SHIFT SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MAX_FRM_SIZE_SHIFT
#define SAND_HAL_QE_SR_P_FRM_SIZE_MAX_FRM_SIZE_MSB SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MAX_FRM_SIZE_MSB
#define SAND_HAL_QE_SR_P_FRM_SIZE_MAX_FRM_SIZE_LSB SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MAX_FRM_SIZE_LSB
#define SAND_HAL_QE_SR_P_FRM_SIZE_MAX_FRM_SIZE_TYPE SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MAX_FRM_SIZE_TYPE
#define SAND_HAL_QE_SR_P_FRM_SIZE_MAX_FRM_SIZE_DEFAULT SAND_HAL_QE_SR_P0_FRM_SIZE_P0_MAX_FRM_SIZE_DEFAULT



#endif
