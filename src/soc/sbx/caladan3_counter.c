/* 
 * $Id: caladan3_counter.c,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        caladan3_counter.c
 * Purpose:     Software Counter Collection module for CALADAN (QE4K).
 */

#if defined(BCM_CALADAN3_SUPPORT)
#include <shared/bsl.h>

#include <soc/error.h>
#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/counter.h>
#include <soc/debug.h>
#include <soc/sbx/caladan3_counter.h>
#include <soc/sbx/hal_ca_auto.h>


/* The QM stats listed here are for Sirius, not Caladan3 */
/* What is returned to the counter thread is always 0    */
/* because the enable of the stats is never called. This */
/* code should be further cleaned up to not start up the */
/* counter block at all in the future.                   */
/* Much of the code in this file is compiled out because */
/* the following flag is not defined.                    */

#undef ENABLE_DEFUNCT_STATS

static int soc_caladan3_counter_enable_flag[SOC_MAX_NUM_DEVICES];
static int soc_caladan3_counter_current_base[SOC_MAX_NUM_DEVICES];

STATIC int _soc_caladan3_counter_qm_read(int unit, int set, int counter,
                                       uint64 *val, int *width);
STATIC int _soc_caladan3_counter_qm_write(int unit, int set, int counter,
                                        uint64 val);
#ifdef ENABLE_DEFUNCT_STATS
static int isBrickSameCu (uint32 unit, uint32 brick, uint32 cu_num);
static int cleanProvisionedGroup(uint32 unit, uint32 segment);
#endif


/*
 * Counter Blocks Types
 */
typedef enum soc_caladan3_counter_block_e {
    caladan3CounterBlockQm = 0,
    caladan3CounterBlockCount
} soc_caladan3_counter_block_t;


/*
 * Counter Blocks
 *
 * Counter blocks for the Software Counter module to collect statistics on.
 *
 * NOTE:  The order of the counter blocks must be the same
 *        as the blocks defined in 'fe2000_counter_block_t'
 */
soc_sbx_counter_block_info_t    caladan3_counter_blocks[] = {
    { caladan3CounterBlockQm,
      CALADAN3_COUNTER_BLOCK_QM_NUM_SETS,
      CALADAN3_COUNTER_QM_COUNT,
      _soc_caladan3_counter_qm_read,
      _soc_caladan3_counter_qm_write,
    },
};


/*
 * Function:
 *     soc_sbx_caladan3_counter_init
 * Purpose:
 *     Initialize and start the counter collection, software
 *     accumulation process.
 * Parameters:
 *     unit     - Device number
 *     flags    - SOC_COUNTER_F_xxx flags
 *     interval - Collection period in micro-seconds,
 *                using 0 is the same as calling soc_sbx_counter_stop()
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_counter_init(int unit, uint32 flags, int interval, pbmp_t pbmp)
{
    int         rv = SOC_E_NONE;
    int         block;
    int         set;

    /* Init Software Counter Collection module */
    SOC_IF_ERROR_RETURN(soc_sbx_counter_init(unit, &caladan3_counter_blocks[0],
                                             caladan3CounterBlockCount));

    block = 0;  /* At present only one block possible */

    /* Add counter sets to main counter collector */

    for (set = 0; set < CALADAN3_COUNTER_BLOCK_QM_NUM_SETS; set++) {
   
        soc_sbx_counter_bset_add(unit, block, set);

    }

    soc_caladan3_counter_enable_flag[unit] = FALSE;
    soc_caladan3_counter_current_base[unit] = -1;

    /* Start software counter collector */
    SOC_IF_ERROR_RETURN(soc_sbx_counter_start(unit, flags, interval, pbmp));
    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_controlled_counter_init(unit));

    return rv;
}

#ifdef ENABLE_DEFUNCT_STATS
/*
 * Function:
 *     soc_sbx_caladan_process_global_stats
 * Purpose:
 *     Process the Global Stats Memory
 *
 * Parameters:
 *     unit     - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_process_global_stats(int unit)
{
      int rv = SOC_E_NONE;
      soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
      soc_sbx_cs_update_t *stat_update = NULL;
      global_stats_entry_t gstat;
      uint16 gbl_idx = 0;
      uint32 temp;
      struct b {
	  union t {
	      uint64 gbl_bytes;
	      uint32 buf[2];
	  } s;
      } bytes;

      for (gbl_idx = 0; gbl_idx <= SOC_MEM_INFO(unit, GLOBAL_STATSm).index_max; gbl_idx++) {
	/*
	 * Read entries out of the Global Stats Counter Table
	 * and store it in a memory buffer
	 */
	
	stat_update = (soc_sbx_cs_update_t *) &cal->cs.gbl_stats[gbl_idx * 2];

	COMPILER_64_ZERO(bytes.s.gbl_bytes);
	SOC_IF_ERROR_RETURN(READ_GLOBAL_STATSm(unit, MEM_BLOCK_ANY, gbl_idx, &gstat));
	soc_GLOBAL_STATSm_field_get(unit, &gstat, COUNTAf, bytes.s.buf);
	temp = bytes.s.buf[0];
	bytes.s.buf[0] = bytes.s.buf[1];
	bytes.s.buf[1] = temp;
	COMPILER_64_ADD_32(stat_update->pkts64, soc_mem_field32_get(unit, GLOBAL_STATSm, &gstat, COUNTBf));
	COMPILER_64_ADD_64(stat_update->bytes64, bytes.s.gbl_bytes);

	if ((COMPILER_64_HI(bytes.s.gbl_bytes) == 0x7) && (COMPILER_64_LO(bytes.s.gbl_bytes) == 0xffffffff))
	  rv = SOC_E_FULL;
      }
      return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_process_fd_drop_stats
 * Purpose:
 *     Process the FD drop Stats Collection
 *
 * Parameters:
 *     unit     - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_process_fd_drop_stats(int unit, int clear)
{
      int rv = SOC_E_NONE;
      soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
      soc_sbx_cs_update_t *stat_update = NULL;
      eg_fd_per_port_drop_count1_entry_t fd_drop;
      uint8 drop1 = 0, drop2 = 0;
      int fd_idx = 0;
      uint32 temp;
      struct b {
	  union t {
	      uint64 fd_bytes;
	      uint32 buf[2];
	  } s;
      } bytes;

      SOC_IF_ERROR_RETURN(READ_FD_CONFIGr(unit, &temp));
      drop1 = soc_reg_field_get(unit,FD_CONFIGr,temp,FD_PER_PORT_DROP_COUNT1_SELf);
      drop2 = soc_reg_field_get(unit,FD_CONFIGr,temp,FD_PER_PORT_DROP_COUNT2_SELf);

      if (clear != 0) {
	  COUNTER_LOCK(unit);
	  /*
	   * Clear the table
	   */
	  for (fd_idx = 0; fd_idx <= SOC_MEM_INFO(unit, EG_FD_PER_PORT_DROP_COUNT1m).index_max; fd_idx++) {
	      if (clear == 1) {
		  rv = READ_EG_FD_PER_PORT_DROP_COUNT1m(unit, MEM_BLOCK_ANY, fd_idx, &fd_drop);
	      } else if (clear == 2) {
		  rv = READ_EG_FD_PER_PORT_DROP_COUNT2m(unit, MEM_BLOCK_ANY, fd_idx, &fd_drop);
	      }
	      if (rv != BCM_E_NONE) {
		  COUNTER_UNLOCK(unit);
		  return rv;
	      }
	  }
	  COUNTER_UNLOCK(unit);
	  return rv;
      }

      /* 0=all, 1=green, 2=yellow, 3=red, 4=mc, 5,6,7=disabled */

      if (drop1 < FD_DROP_TYPE_COUNT) {
	  for (fd_idx = 0; fd_idx <= SOC_MEM_INFO(unit, EG_FD_PER_PORT_DROP_COUNT1m).index_max; fd_idx++) {
	      /*
	       * Read entries out of the FD Stats Counter Table
	       * and store it in a memory buffer
	       */
	      
	      stat_update = (soc_sbx_cs_update_t *) &cal->cs.fd_drop[drop1][fd_idx * 2];
	      
	      COMPILER_64_ZERO(bytes.s.fd_bytes);
	      SOC_IF_ERROR_RETURN(READ_EG_FD_PER_PORT_DROP_COUNT1m(unit, MEM_BLOCK_ANY, fd_idx, &fd_drop));
	      soc_EG_FD_PER_PORT_DROP_COUNT1m_field_get(unit, &fd_drop, BYTE_DROP_COUNTf, bytes.s.buf);
	      temp = bytes.s.buf[0];
	      bytes.s.buf[0] = bytes.s.buf[1];
	      bytes.s.buf[1] = temp;
	      COMPILER_64_ADD_32(stat_update->pkts64, 
				 soc_mem_field32_get(unit, EG_FD_PER_PORT_DROP_COUNT1m, &fd_drop, PACKET_DROP_COUNTf));
	      COMPILER_64_ADD_64(stat_update->bytes64, bytes.s.fd_bytes);
	      
	      if ((COMPILER_64_HI(bytes.s.fd_bytes) == 0x7) && (COMPILER_64_LO(bytes.s.fd_bytes) == 0xffffffff))
		  rv = SOC_E_FULL;
	  }
      }

      if (drop2 < FD_DROP_TYPE_COUNT) {
	  for (fd_idx = 0; fd_idx <= SOC_MEM_INFO(unit, EG_FD_PER_PORT_DROP_COUNT2m).index_max; fd_idx++) {
	      /*
	       * Read entries out of the FD Stats Counter Table
	       * and store it in a memory buffer
	       */
	      
	      stat_update = (soc_sbx_cs_update_t *) &cal->cs.fd_drop[drop2][fd_idx * 2];
	      
	      COMPILER_64_ZERO(bytes.s.fd_bytes);
	      SOC_IF_ERROR_RETURN(READ_EG_FD_PER_PORT_DROP_COUNT2m(unit, MEM_BLOCK_ANY, fd_idx, &fd_drop));
	      soc_EG_FD_PER_PORT_DROP_COUNT2m_field_get(unit, &fd_drop, BYTE_DROP_COUNTf, bytes.s.buf);
	      temp = bytes.s.buf[0];
	      bytes.s.buf[0] = bytes.s.buf[1];
	      bytes.s.buf[1] = temp;
	      COMPILER_64_ADD_32(stat_update->pkts64, 
				 soc_mem_field32_get(unit, EG_FD_PER_PORT_DROP_COUNT2m, &fd_drop, PACKET_DROP_COUNTf));
	      COMPILER_64_ADD_64(stat_update->bytes64, bytes.s.fd_bytes);
	      
	      if ((COMPILER_64_HI(bytes.s.fd_bytes) == 0x7) && (COMPILER_64_LO(bytes.s.fd_bytes) == 0xffffffff))
		  rv = SOC_E_FULL;
	  }
      }

      return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_process_slq_stats
 * Purpose:
 *     Process the SLQ Stats Memory
 *
 * Parameters:
 *     unit     - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_process_slq_stats(int unit)
{
      int rv = SOC_E_NONE;
      soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
      soc_sbx_cs_update_t *stat_update = NULL;
      uint16 slq_base_idx = 0;
      uint16 slq_q_idx = 0;
      uint32 slq_pkts = 0;
      uint32 slq_bytes = 0;
      uint16 slq_cntr_addr = 0;
      uint32 regval = 0;

      for (slq_base_idx = 0; slq_base_idx < 16; slq_base_idx++) {
	/*
	 * Only retrieve statistics on queues that have
	 * been activated
	 */
	if ((cal->cs.slq_q_active & (1 << slq_base_idx)) == 0)
	    continue;

	regval = 0;
	soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, SLQ_PTRf, slq_base_idx << 4);
	soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, GOf, 1);
	soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, AUTO_INCRf, 1);
	SOC_IF_ERROR_RETURN(WRITE_QMB_SLQ_PTRr(unit, regval));

	for (slq_q_idx = 0; slq_q_idx < 16; slq_q_idx++) {
	  /*
	   * Read entries out of the SLQ registers
	   * and store it in a memory buffer
	   */

	  slq_cntr_addr = (slq_base_idx << 4) + slq_q_idx;
	  stat_update = (soc_sbx_cs_update_t *) &cal->cs.slq_stats[slq_cntr_addr*2];

	  /* 
	   * Last counter pair in current queue, disable auto_incr
	   */
	  if (slq_q_idx == 15) {
	    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, SLQ_PTRf, slq_cntr_addr);
	    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, GOf, 1);
	    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, AUTO_INCRf, 0);
	    SOC_IF_ERROR_RETURN(WRITE_QMB_SLQ_PTRr(unit, regval));
	  }
	  
	  SOC_IF_ERROR_RETURN(READ_QMB_SLQ_PKT_CNTr(unit, &slq_pkts));
	  SOC_IF_ERROR_RETURN(READ_QMB_SLQ_BYTE_CNTr(unit, &slq_bytes));

	  COMPILER_64_ADD_32(stat_update->pkts64, slq_pkts);
	  COMPILER_64_ADD_32(stat_update->bytes64, slq_bytes);
	  
	  /*
	   * Check to see if we wrap the buffer
	   */
	  if (slq_bytes == 0xffffffff)
	    rv = SOC_E_FULL;
	}
      }
      return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_process_cs_dma_fifo
 * Purpose:
 *     Process the Central Statistics Memory Fifo for 
 *     new entries.
 * Parameters:
 *     unit     - Device number
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_process_cs_dma_fifo(int unit)
{
      int rv = SOC_E_NONE;
      soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
      soc_sbx_cs_rbuf_entry_t *fifo_entry = (soc_sbx_cs_rbuf_entry_t *) cal->cs.fifo_dma_rbuf_read_ptr;
      soc_sbx_cs_update_t *stat_update = NULL;
      uint32 regval = 0;

      /*
       * If read pointer is NULL, then
       * CS has not been initialized and 
       * no DMA Stat fifo exists. 
       */

      if (cal->cs.fifo_dma_rbuf_read_ptr == NULL)
          return SOC_E_INIT;

      /*
       * If the mem_addr field contains zero, then there are
       * no new ejection messages to process
       */

      while (fifo_entry->mem_addr != 0) {

	/*
	 * Insure that CS memory initialization doesn't cause
	 * invalid address write to ring buffer.
	 */
	if (fifo_entry->mem_addr < (SB_FAB_DEVICE_CALADAN3_NUM_QUEUES << 4)) {
	    LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "CS: Invalid Memory Address 0x%x: unit %d\n"), 
                       fifo_entry->mem_addr, unit));
	    return SOC_E_MEMORY;
	}

	/*
	 * If an address has been filled in, then we need
	 * to process this ejection message.
	 */
	
	if ((stat_update = (soc_sbx_cs_update_t *)
	     CS_DMA_FIFO_MEM_ADDR(fifo_entry->mem_addr)) != 0) {
	  COMPILER_64_ADD_32(stat_update->bytes64, fifo_entry->entry_bytes);
	  COMPILER_64_ADD_32(stat_update->pkts64, fifo_entry->entry_pkts);
	}
	
	/*
	 * This is the last message in a manual eject request.
	 * Send a message back notifying the sender that
	 * the stat request has completed.
	 */
	
	if (fifo_entry->mem_addr & CS_DMA_CNTR_MSG_LAST) {

	  /*
	   * The manual eject process is not simultaneous.
	   * Only one manual flush may be active at a time.
	   */

	  if (cal->cs.flags & CS_FLUSHING) {
	    sal_sem_give(cal->cs.CsFifoSem);
	  } else {
	    LOG_INFO(BSL_LS_SOC_COUNTER,
                     (BSL_META_U(unit,
                                 "CS: Invalid Manual Flush state: unit %d\n"), unit));
	  }
	}
	
	/*
	 * Clear the processed ejection message mem_addr.
	 * No need to process for Dummy bit. That just
	 * indicated the ejection message is a filler and
	 * contains no data
	 */
	
	fifo_entry->mem_addr = 0;
	
	/*
	 * Advance the ring buffer read pointer and write it back to 
	 * the register. Handle appropriate ring buffer wrap
	 */
	
	if ((cal->cs.fifo_dma_rbuf_read_ptr += CS_DMA_FIFO_MSG_SIZE) >= cal->cs.fifo_dma_rbuf_end)
	  cal->cs.fifo_dma_rbuf_read_ptr = cal->cs.fifo_dma_rbuf_begin;
	
	fifo_entry = (soc_sbx_cs_rbuf_entry_t *) cal->cs.fifo_dma_rbuf_read_ptr;

	/*
	 * use regval to indicate that cs_fifo_dma_rbuf_read_ptr advanced
	 */
	regval = 1;
      }

      /*
       * If regval non-zero, then we've processed FIFO messages.
       * Update hw read_ptr address
       */ 

      if (regval != 0) {

	/*
	 * Set the read ptr to the head of the DMA Ring Buffer and
	 * write that value to the CMIC register.
	 */
	
	regval = soc_cm_l2p(unit, cal->cs.fifo_dma_rbuf_read_ptr);
#if 0	
	if((rv = WRITE_CMIC_FIFO_CH0_RD_DMA_HOSTMEM_READ_PTRr(unit, regval)) < 0) {
          /* coverity[dead_error_begin] */
	  LOG_INFO(BSL_LS_SOC_COUNTER,
                   (BSL_META_U(unit,
                               "CS: failed to write DMA_HOSTMEM_READ_PTRr %x\n"),
                    regval));
	  
	  /*
	   * RSXX
	   * If we can't update read pointer, then we need to stop
	   * statistics gathering because CS is busted.
	   */
	}
#endif     
      rv = WRITE_CMIC_FIFO_CH0_RD_DMA_HOSTMEM_READ_PTRr(unit, regval);
      }
      return rv;
}

/*
 * Function:
 *     soc_sbx_calaban3_get_segment
 * Purpose:
 * Locate a segment for statistics generation
 *
 * Parameters:
 *     unit     - Device number
 *     segment  - if associated with gport/cosq, return segment
 *                otherwise, allocate new segment
 *     cu       - CU number to associate with this segment
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_get_segment(uint32 unit, uint8 cu, int32 *rSegment)
{
    soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
    uint8      segment = 0;

    /*
     * Search the segmentInUse bitmap for an available segment number
     */

    for(segment=0; segment < CS_NUM_SEGMENTS; segment++) {
      if ((cal->cs.segmentInUse & (1 << segment)) == 0) {
	cal->cs.segmentInUse |= (1 << segment);
	cal->cs.segment[segment].cu_num = cu;
	*rSegment = segment;
	return SOC_E_NONE;
      }
    }

    /*
     * No segment is available.Set segment numer to an invalid value and
     * return unavailable resource.
     */

    *rSegment = -1;
    return SOC_E_UNAVAIL;
}

/*
 * Function:
 *     soc_sbx_caladan3_free_segment
 * Purpose:
 * Free a segment that has been allocated
 *
 * Parameters:
 *     unit     - Device number
 *     segment  - used segment
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_free_segment(uint32 unit, uint32 segment)
{
    soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
    uint32                   regval = 0;

    if ((cal->cs.segmentInUse & (1 << segment)) == 0) {
      /*
       * Segment not allocated
       */
      return SOC_E_FAIL;
    }

    /*
     * Make sure resources associated with segment
     * have been cleaned up.
     */

    if (COMPILER_64_IS_ZERO(cal->cs.segment[segment].cntrIdMap) == FALSE) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Free segment unit %d segment %d cntrIdMap 0x%x%08x\n"),
                unit, segment, COMPILER_64_HI(cal->cs.segment[segment].cntrIdMap), COMPILER_64_LO(cal->cs.segment[segment].cntrIdMap)));
      return SOC_E_FAIL;
    }

    if (cal->cs.segment[segment].segmentSize != 0) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Free segment unit %d segment %d Segment Size %d\n"),
                unit, segment, cal->cs.segment[segment].segmentSize));
      return SOC_E_FAIL;
    }

    SOC_IF_ERROR_RETURN(READ_QM_CS_CONFIG0r(unit, &regval));
    if ((soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, DEQ_CS_STATS_ENf) == 1) && 
	(segment == soc_reg_field_get(unit, QM_CS_CONFIG0r, regval, CS_DEQ_SEGMENTf))) {
	soc_reg_field_set(unit, QM_CS_CONFIG0r, &regval, DEQ_CS_STATS_ENf, 0);
	soc_reg_field_set(unit, QM_CS_CONFIG0r, &regval, CS_DEQ_SEGMENTf, 0);
	SOC_IF_ERROR_RETURN(WRITE_QM_CS_CONFIG0r(unit, regval));
    }

    /*
     * Free memory buffer
     */

    if (cal->cs.segment[segment].ullCountOrig != NULL)
      sal_free(cal->cs.segment[segment].ullCountOrig);

    cal->cs.segment[segment].ullCountOrig = NULL;
    cal->cs.segment[segment].ullCount = NULL;
    cal->cs.segment[segment].cu_num = 0;
    cal->cs.segment[segment].shared = 0;
    sal_memset(cal->cs.segment[segment].flushTimeDelta, 0, sizeof(cal->cs.segment[segment].flushTimeDelta));
    sal_memset(cal->cs.segment[segment].group, 0, sizeof(cal->cs.segment[segment].group));
    
    cal->cs.segmentInUse &= ~(1 << segment);

    return SOC_E_NONE;
}


/*
 * Function:
 *     isBrickSameCu
 * Purpose:
 * Check all the groups on a brick to enforce
 * that only one CU is configured to it.
 *
 * Parameters:
 *     unit     - Device number
 *     brick    - Current brick
 *     cu_num   - Current CU
 * Returns:
 *     TRUE     - Success
 *     FALSE    - Failure
 */
int
isBrickSameCu (uint32 unit, uint32 brick, uint32 cu_num)
{
    soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
    uint32 group = 0;

  for ( group = 0; group < CS_NUM_GROUPS; group++) {
    if ( (cal->cs.groupConfig[brick][group].group_used != CS_GROUP_FREE) &&
	 (cal->cs.groupConfig[brick][group].cu_num != cu_num ))
	     return FALSE;
  }
  return TRUE;
}


/*
 * Function:
 *     cleanProvisionedGroup
 * Purpose:
 * Clean up groupConfig structure
 * due to lack of enough resources
 *
 * Parameters:
 *     unit     - Device number
 *     segment  - current segment being provisioned
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
cleanProvisionedGroup(uint32 unit, uint32 segment)
{
    soc_sbx_caladan3_config_t *cal    = SOC_SBX_CFG_CALADAN3(unit);
    cs_segment_t            *segInfo = &cal->cs.segment[segment];
    cs_grp_cfg_t            *grpCfg = NULL;
    uint32                 brick  = 0;
    uint32                 group  = 0;
    uint32                 curr_cfg_grp = 0;
    uint64                 temp = COMPILER_64_INIT(0,0);

    /*
     * We do not have enough resources to create this group.
     * Wipe the group from the groupConfig table before 
     * returning.
     */
    
    for (brick = 0; brick < CS_NUM_BRICKS; brick++) {
      for (group = 0; group < CS_NUM_GROUPS; group++) {
	grpCfg = &cal->cs.groupConfig[brick][group];
	if ((grpCfg->segment == segment) &&
	    (grpCfg->group_used == CS_GROUP_PROVISIONING)) {
	  
	  /*
	   * Convert the offset back into the counter ID group
	   * allocated in the Counter Id Bitmap
	   */
	  
	  curr_cfg_grp = grpCfg->offset >> CS_CFG_GRP_SHIFT;
	  if (curr_cfg_grp < 32)
	    COMPILER_64_SET(temp, 0, 1 << curr_cfg_grp);
	  else
	    COMPILER_64_SET(temp, 1 << (curr_cfg_grp - 32), 0);
          COMPILER_64_NOT(temp);
	  COMPILER_64_AND(segInfo->cntrIdMap, temp);

	  grpCfg->group_used = CS_GROUP_FREE;
	  grpCfg->cu_num = 0;
	  grpCfg->offset = 0;
	  grpCfg->segment = 0;
	}
      }
    }
    return SOC_E_NONE;
}


/*
 * Function:
 *     soc_sbx_calaban3_create_group
 * Purpose:
 * Provision a free group.
 * This function will allocate 1K groups associated
 * with the counter ID range. 
 *
 * Parameters:
 *     unit     - Device number
 *     segment  - segment associated with the new group
 *     cu_num   - The CU i/f 
 *     cntrId   - Initial Counter ID num
 *     size     - number of counters
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_create_group(uint32 unit, uint32 segment, uint32 cu_num, uint32 cntrId, uint32 size)
{
    soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
    cs_segment_t *segInfo = &cal->cs.segment[segment];
    cs_grp_cfg_t *grpCfg = NULL;
    void         *pBuf = NULL;
    uint8      brick = 0, last_brick = 0;
    uint8      group = 0;
    uint16     ctrSize = size;
    uint32     cfg_grp_start = (cntrId >> CS_CFG_GRP_SHIFT);
    uint32     cfg_grp_end = ((cntrId + size) >> CS_CFG_GRP_SHIFT);
    uint32     curr_cfg_grp = cfg_grp_start;
    uint32     regval = 0;
    uint64     temp = COMPILER_64_INIT(0,0);

    /*
     * Search and allocate group
     */

    while (curr_cfg_grp <= cfg_grp_end ) {
	
      /*
       * Check to see if this region has already
       * been allocated. Do not reallocate.
       */

      COMPILER_64_ADD_64(temp, segInfo->cntrIdMap);
      if (COMPILER_64_BITTEST(temp, curr_cfg_grp) != 0) {
	brick = segInfo->group[curr_cfg_grp] / CS_NUM_GROUPS;
	group = segInfo->group[curr_cfg_grp] % CS_NUM_GROUPS;
	grpCfg = &cal->cs.groupConfig[brick][group];
	
	/* 
	 * keep track of resources allocated per group
	 */
	
	if ((cfg_grp_start == cfg_grp_end) ||
	    (curr_cfg_grp == cfg_grp_end))
	  grpCfg->usage += ctrSize;
	else if (curr_cfg_grp == cfg_grp_start) {
	  grpCfg->usage += (CS_GRP_SIZE - cntrId);
	  ctrSize -= (CS_GRP_SIZE - cntrId);
	} else {
	  grpCfg->usage += CS_GRP_SIZE;
	  ctrSize -= CS_GRP_SIZE;
	}
	curr_cfg_grp++;
	continue;
      }

      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Create Group Unit %d New Segment %d cu_num %d cntrId %d size %d\n"),
                unit, segment, cu_num, cntrId, size));

      /*
       * Set the bit associated with the 1K region
       * of counters.
       */

      if (curr_cfg_grp < 32)
	COMPILER_64_SET(temp, 0, 1 << curr_cfg_grp);
      else
	COMPILER_64_SET(temp, 1 << (curr_cfg_grp - 32), 0);
      COMPILER_64_OR(segInfo->cntrIdMap, temp);

      /*
       * Find next available brick and group
       */
      
      for (brick = last_brick; brick < CS_NUM_BRICKS; brick++) {
	for (group = 0; group < CS_NUM_GROUPS; group++) {
	  if (( cal->cs.groupConfig[brick][group].group_used == CS_GROUP_FREE) &&
	      isBrickSameCu(unit, brick, cu_num) == TRUE)
	    goto got_group;
	}
      }

      /*
       * We do not have enough resources to create this Group.
       * Wipe the group from the groupConfig table before 
       * returning.
       */

      cleanProvisionedGroup(unit, segment);
      
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Unit %d: Unable to Allocate Group Config %d for Segment %d\n"),
                unit, curr_cfg_grp, segment));
      
      return SOC_E_RESOURCE;
      
got_group:

      /*
       * Assign group  number
       */
      
      grpCfg = &cal->cs.groupConfig[brick][group];
      grpCfg->group_used = CS_GROUP_PROVISIONING;
      grpCfg->cu_num = cu_num;
      grpCfg->offset = curr_cfg_grp << CS_CFG_GRP_SHIFT;
      grpCfg->segment = segment;
      grpCfg->usage += ctrSize;
      segInfo->group[curr_cfg_grp] = brick*4 + group;
      
      last_brick = brick;

      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Unit %d: Segment %d range %d - %d: Brick %d Group %d\n"), 
                unit, segment, 
                curr_cfg_grp * CS_GRP_SIZE, 
                curr_cfg_grp * CS_GRP_SIZE + CS_GRP_SIZE - 1, 
                brick, group));
      
      curr_cfg_grp++;
    }
    
    /*
     * Allocate a buffer to store the statistics and provision a group
     */

    if (segInfo->ullCountOrig == NULL) {
      
      pBuf = segInfo->ullCountOrig = sal_alloc((sizeof(uint64) * 2) * (1<<16) + 0xf, "CS Counters");
      
      /*
       * Did the alloc succeed?
       */

      if (segInfo->ullCountOrig == NULL) {
	cleanProvisionedGroup(unit, segment);
	LOG_INFO(BSL_LS_SOC_COUNTER,
                 (BSL_META("CS: Unit %d failed to allocate ullCount[%d] \n"), unit, segment));
	return SOC_E_MEMORY;
      }

      sal_memset(segInfo->ullCountOrig, 0, ((sizeof(uint64) * 2) * (1<<16) + 0xf));

      /* Buffer needs to be 64-bit aligned. Most mallocs do this automatically
       * when they are compiled with uint64 support, even on 32-bit archs.
       * Support can be added by allocating an extra 64-bit word and sliding
       * the buffer reference to be aligned. */

      pBuf = (char *) pBuf + 0xf;
      pBuf = (void *) ((uint32) pBuf & ~0xf);

      segInfo->ullCount = (uint64 *) pBuf;
      
    }
    
    /*
     * Disable background statistics gathering on a segment
     * that has groups allocated to it
     */
    
    SOC_IF_ERROR_RETURN(READ_CS_CONFIG_BACKGROUND_ENABLEr(unit, &regval));
    regval &= ~(1 << segment);
    SOC_IF_ERROR_RETURN(WRITE_CS_CONFIG_BACKGROUND_ENABLEr(unit, regval));

    /*
     * Provision the groups now that we know the resources are available.
     */

    SOC_IF_ERROR_RETURN(soc_sbx_caladan3_provision_group(unit, segment, segInfo->ullCount));
    
    /*
     * Enable background statistics gathering on a segment
     * that has groups allocated to it
     */
    
    SOC_IF_ERROR_RETURN(READ_CS_CONFIG_BACKGROUND_ENABLEr(unit, &regval));
    regval |= (1 << segment);
    SOC_IF_ERROR_RETURN(WRITE_CS_CONFIG_BACKGROUND_ENABLEr(unit, regval));
    
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_provision_group
 * Purpose:
 * Provision free groups that have resources assigned
 * to them.
 *
 * Parameters:
 *     unit       - Device number
 *     segment    - segment to associate group with
 *     ullCntAddr - Location in Hostmem where statistics are stored
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_provision_group(uint32 unit, uint32 segment, uint64 *ullCntAddr)
{
    soc_sbx_caladan3_config_t     *cal    = SOC_SBX_CFG_CALADAN3(unit);
    cs_segment_t                  *segInfo = &cal->cs.segment[segment];
    cs_grp_cfg_t                  *grpCfg = NULL;
    uint8                        brick  = 0;
    uint8                        group  = 0;
    cs_brick_config_table_entry_t  brickCfg;

    /*
     * First we go through group table and provision the group
     */

    for ( brick = 0; brick < CS_NUM_BRICKS; brick++)
      for (group = 0;group < CS_NUM_GROUPS; group++) {
	
	/*
	 * Get group Num
	 */
	
	sal_memset(&brickCfg, 0, sizeof(cs_brick_config_table_entry_t));
	grpCfg = &cal->cs.groupConfig[brick][group];
	
	if ( grpCfg->group_used == CS_GROUP_PROVISIONING &&
	     grpCfg->segment == segment ) {

	  soc_mem_field32_set(unit, CS_BRICK_CONFIG_TABLEm, &brickCfg, ENABLEf, 1);
	  soc_mem_field32_set(unit, CS_BRICK_CONFIG_TABLEm, &brickCfg, SEGMENTf, segment);
	  soc_mem_field32_set(unit, CS_BRICK_CONFIG_TABLEm, &brickCfg, BASE_CNTRIDf, grpCfg->offset);
	  soc_mem_field32_set(unit, CS_BRICK_CONFIG_TABLEm, &brickCfg, PHYSICALf, (uint32) ullCntAddr);
	  SOC_IF_ERROR_RETURN(WRITE_CS_BRICK_CONFIG_TABLEm(unit, MEM_BLOCK_ANY, (brick*CS_NUM_GROUPS + group), &brickCfg));
	
	  /*
	   * Initialize the Group memory to 0 for clean statistics
	   */

	  soc_sbx_caladan3_init_group(unit, segment, grpCfg->offset, CS_GRP_SIZE);

	  /*
	   * Set the group to active
	   */

	  grpCfg->group_used = CS_GROUP_ACTIVE;
	  segInfo->segmentSize += CS_GRP_SIZE;
	}
      }

    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_init_group
 * Purpose:
 * Initialize an allocated and provisioned group
 *
 * Parameters:
 *     unit      - Device number
 *     segment   - provisioned segment
 *     cntr_id   - First counter ID num
 *     num_cntrs - Number of counters requested
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_init_group(uint32 unit, uint32 segment, uint32 cntr_id, uint32 num_cntrs)
{
    soc_timeout_t timeout;
    uint32      nInitDone = 0, to_usec = 0;
    uint32      regval = 0;

    /*
     * Initializes the brick rams for a given group.
     */
    
    /*
     * Write the Segment #
     */
    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CONFIG0r, &regval, SEGMENTf, segment );
    SOC_IF_ERROR_RETURN(WRITE_CS_MANUAL_EJECT_CONFIG0r(unit, regval));

    /*
     * Write the Low Address:
     */

    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CONFIG1r, &regval, START_CNTRIDf, cntr_id );
    SOC_IF_ERROR_RETURN(WRITE_CS_MANUAL_EJECT_CONFIG1r(unit, regval));

    /*
     * Write the hi address:
     * guarantee a 1K boundary
     */

    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CONFIG2r, &regval, END_CNTRIDf,((cntr_id + num_cntrs - 1) | 0x3ff));
    SOC_IF_ERROR_RETURN(WRITE_CS_MANUAL_EJECT_CONFIG2r(unit, regval));

    /*
     * Write the Initialize and Go bits to clear memory region
     */

    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CTRLr, &regval, INITIALIZEf,1);
    SOC_IF_ERROR_RETURN(WRITE_CS_MANUAL_EJECT_CTRLr(unit, regval));

    soc_reg_field_set(unit, CS_MANUAL_EJECT_CTRLr, &regval, GOf,1);
    SOC_IF_ERROR_RETURN(WRITE_CS_MANUAL_EJECT_CTRLr(unit, regval));

    /*
     * Wait for the Done...
     */
    
    if (SAL_BOOT_QUICKTURN) {
      to_usec = SCHAN_TIMEOUT_QT;
    } else if (SAL_BOOT_BCMSIM) {
      to_usec = SCHAN_TIMEOUT_PLI;
    } else {
      to_usec = SCHAN_TIMEOUT + 200000;
    }
    to_usec = soc_property_get(unit, spn_CDMA_TIMEOUT_USEC, to_usec);

    soc_timeout_init(&timeout, to_usec,0);
    while(!soc_timeout_check(&timeout)) {
        sal_usleep(1000);
	SOC_IF_ERROR_RETURN(READ_CS_MANUAL_EJECT_CTRLr(unit, &regval));

	nInitDone = soc_reg_field_get(unit,CS_MANUAL_EJECT_CTRLr,regval,EJECT_DONEf);
	if (nInitDone)
	    break;
    }

    if (!nInitDone) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("CS BLOCK Initialization done timeout\n")));
	return SOC_E_TIMEOUT;
    }

    /*
     * Clear the Eject Done bit
     */

    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CTRLr, &regval, EJECT_DONEf,1);
    SOC_IF_ERROR_RETURN(WRITE_CS_MANUAL_EJECT_CTRLr(unit, regval));

    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META("CS:Group %d has been initialized on unit %d\n"), segment, unit));
    return SOC_E_NONE;
}

/*
 * Function:
 *     soc_sbx_caladan3_remove_group
 * Purpose:
 * Decrememnt resource for a group. If no resources, delete group.
 * This function will allocate 1K groups associated
 * with the counter ID range. 
 *
 * Parameters:
 *     unit     - Device number
 *     segment  - segment associated with a particular group
 *     cu_num   - The CU i/f 
 *     cntrId   - Initial Counter ID num
 *     size     - number of counters
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_remove_group(uint32 unit, uint32 segment, uint32 cu_num, uint32 cntrId, uint32 size)
{
    soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
    cs_segment_t *segInfo = &cal->cs.segment[segment];
    cs_grp_cfg_t *grpCfg = NULL;
    cs_brick_config_table_entry_t  brickCfg;
    uint8      brick = 0, group = 0;
    uint32     ctrSize = size;
    uint32     cfg_grp_start = (cntrId >> CS_CFG_GRP_SHIFT);
    uint32     cfg_grp_end = ((cntrId + size) >> CS_CFG_GRP_SHIFT);
    uint32     curr_cfg_grp = cfg_grp_start;
    uint32     regval = 0;
    uint64     temp = COMPILER_64_INIT(0,0);
    int          rv = SOC_E_NONE;

    /*
     * Search and allocate group
     */

    while (curr_cfg_grp <= cfg_grp_end ) {

      /*
       * Check to see if this region has already
       * been deallocated.
       */

      COMPILER_64_ADD_64(temp, segInfo->cntrIdMap);
      if (COMPILER_64_BITTEST(temp, curr_cfg_grp) == 0) {
	rv = SOC_E_NOT_FOUND;
	curr_cfg_grp++;
	continue;
      }

      rv = SOC_E_NONE;

      brick = segInfo->group[curr_cfg_grp] / CS_NUM_GROUPS;
      group = segInfo->group[curr_cfg_grp] % CS_NUM_GROUPS;

      grpCfg = &cal->cs.groupConfig[brick][group];

      /* 
       * keep track of resources allocated per group
       */

      if ((cfg_grp_start == cfg_grp_end) ||
	  (curr_cfg_grp == cfg_grp_end))
	grpCfg->usage -= ctrSize;
      else if (curr_cfg_grp == cfg_grp_start) {
	grpCfg->usage -= (CS_GRP_SIZE - cntrId);
	ctrSize -= (CS_GRP_SIZE - cntrId);
      } else {
	grpCfg->usage -= CS_GRP_SIZE;
	ctrSize -= CS_GRP_SIZE;
      }
	
      if (grpCfg->usage != 0) {
	curr_cfg_grp++;
	continue;
      }

      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Remove Group Unit %d Segment %d cu_num %d cntrId %d size %d\n"),
                unit, segment, cu_num, cntrId, size));

      /*
       * Clear the bit associated with the 1K region
       * of counters.
       */

      if (curr_cfg_grp < 32)
	COMPILER_64_SET(temp, 0, 1 << curr_cfg_grp);
      else
	COMPILER_64_SET(temp, 1 << (curr_cfg_grp - 32), 0);
      COMPILER_64_NOT(temp);
      COMPILER_64_AND(segInfo->cntrIdMap, temp);
      
      /*
       * Assign group  number
       */
      
      grpCfg->group_used = CS_GROUP_FREE;
      grpCfg->cu_num = 0;
      grpCfg->offset = 0;
      grpCfg->segment = 0;

      /*
       * Disable background statistics gathering on this segment
       * while modifying the CS_BRICK_CONFIG_TABLE or
       * strange things can happen
       */
      
      SOC_IF_ERROR_RETURN(READ_CS_CONFIG_BACKGROUND_ENABLEr(unit, &regval));
      
      regval &= ~(1 << segment);
      
      SOC_IF_ERROR_RETURN(WRITE_CS_CONFIG_BACKGROUND_ENABLEr(unit, regval));
      
      sal_memset(&brickCfg, 0, sizeof(cs_brick_config_table_entry_t));
      SOC_IF_ERROR_RETURN(WRITE_CS_BRICK_CONFIG_TABLEm(unit, MEM_BLOCK_ANY, segInfo->group[curr_cfg_grp], &brickCfg));

      segInfo->segmentSize -= CS_GRP_SIZE;
      segInfo->group[curr_cfg_grp] = 0;
      
      curr_cfg_grp++;
    }

    /*
     * If the segment is still in use, re-enable
     * background statistics. Otherwise, free the segment
     */

    if (segInfo->segmentSize != 0) {

      /*
       * Enable background statistics gathering on this segment
       */
      
      SOC_IF_ERROR_RETURN(READ_CS_CONFIG_BACKGROUND_ENABLEr(unit, &regval));
      
      if ((regval & (1 << segment)) == 0) {
	regval |= (1 << segment);
	
	SOC_IF_ERROR_RETURN(WRITE_CS_CONFIG_BACKGROUND_ENABLEr(unit, regval));
      }
    } else
      rv = soc_sbx_caladan3_free_segment(unit, segment);

    return rv;
}

/*
 * Function:
 *     soc_sbx_caladan3_flush_segment
 * Purpose:
 * Flush cached counters from CS to the Hostmem Ring Buffer
 *
 * Parameters:
 *     unit      - Device number
 *     segment   - provisioned segment
 *     cntr_id   - First counter ID num
 *     num_cntrs - Number of counters requested
 *
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_flush_segment(uint32 unit, uint32 segment, uint32 cntrId, uint32 num_cntrs)
{
    soc_sbx_caladan3_config_t *cal = SOC_SBX_CFG_CALADAN3(unit);
    cs_segment_t *segInfo = &cal->cs.segment[segment];
    uint32     regval = 0, timeCheck = 0;
    uint32     start_cntr = ((cntrId >> CS_CFG_GRP_SHIFT) * CS_GRP_SIZE);
    uint32     end_cntr = ((((cntrId + num_cntrs) >> CS_CFG_GRP_SHIFT) + 1) * CS_GRP_SIZE - 1);
    int          rv = SOC_E_FAIL;
    uint32      nInitDone = 0, to_usec = 0;
    uint32      curr_time = sal_time_usecs(), delta = 0;
    uint64     temp = COMPILER_64_INIT(0,0);

    /*
     * If this region hasn't been allocated, then we cannot
     * do a flush on the segment.
     */
    
    for (timeCheck = (start_cntr >> CS_CFG_GRP_SHIFT); 
	 timeCheck < ((end_cntr + 1) >> CS_CFG_GRP_SHIFT); timeCheck++) {

      COMPILER_64_ADD_64(temp, segInfo->cntrIdMap);
      if (COMPILER_64_BITTEST(temp, timeCheck) == 0) {
	return SOC_E_FAIL;
      }
    }

   timeCheck = start_cntr >> CS_CFG_GRP_SHIFT;
    while ((timeCheck < ((end_cntr + 1) >> CS_CFG_GRP_SHIFT ))) {

      /*
       * If any requested segment is stale,
       * then flush the whole thing
       */
      if ((delta = SAL_USECS_SUB(curr_time, segInfo->flushTimeDelta[timeCheck++])) > SECOND_USEC)
	goto flush;
    }

    /*
     * If all segments are still within a second of the last flush,
     * then don't flush cached values
     */

    if (timeCheck == ((end_cntr + 1) >> CS_CFG_GRP_SHIFT ))
      return SOC_E_NONE;

flush:
      
    LOG_INFO(BSL_LS_SOC_COUNTER,
             (BSL_META("CS:Flush Segment Unit %d Segment %d Counter %d Range %d\n"), 
              unit, segment, cntrId, num_cntrs));

    /*
     * Update the timestamp for the segment / counter ID region
     */

    timeCheck = start_cntr >> CS_CFG_GRP_SHIFT;
    while ((timeCheck < ((end_cntr + 1) >> CS_CFG_GRP_SHIFT )))
      segInfo->flushTimeDelta[timeCheck++] = curr_time;

    if (cal->cs.flags & CS_FLUSHING) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS:Flush Segment Unit %d BUSY\n"), unit));
      return SOC_E_BUSY;
    }

    /* 
     * Write the segment number into CS_MANUAL_EJECT_CONFIG0.
     */
    
    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CONFIG0r, &regval, SEGMENTf, segment);
    if ((rv = WRITE_CS_MANUAL_EJECT_CONFIG0r(unit,regval)) < 0) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS: unable to write CS_MANUAL_EJECT_CONFIG0r register for unit %d with %d\n"),
                unit, regval));
      return rv;
    }

    /* 
     * Write the Low Address into CS_MANUAL_EJECT_CONFIG1.
     */
    
    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CONFIG1r, &regval, START_CNTRIDf, start_cntr);
    if ((rv = WRITE_CS_MANUAL_EJECT_CONFIG1r(unit,regval)) < 0) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS: unable to write CS_MANUAL_EJECT_CONFIG1r register for unit %d with %d\n"),
                unit, regval));
      return rv;
    }

    /* 
     * Write the High Address into CS_MANUAL_EJECT_CONFIG2.
     */
    
    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CONFIG2r, &regval, END_CNTRIDf, end_cntr);
    if ((rv = WRITE_CS_MANUAL_EJECT_CONFIG2r(unit,regval)) < 0) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS: unable to write CS_MANUAL_EJECT_CONFIG2r register for unit %d with %d\n"),
                unit, regval));
      return rv;
    }

    /*
     * Start flushing
     */

    regval = 0;
    soc_reg_field_set(unit, CS_MANUAL_EJECT_CTRLr, &regval, GOf,1);
    if ((rv = WRITE_CS_MANUAL_EJECT_CTRLr(unit, regval)) < 0) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS: unable to write CS_MANUAL_EJECT_CTRLr register for unit %d with %x\n"),
                unit, regval));
      return rv;
    }

    /*
     * Notify the process_cs_dma_fifo routine that flushing is in progress.
     * Only the manual flush bit set in a ejection message can reset this
     * bit.
     */

    cal->cs.flags |= CS_FLUSHING;

    if (SAL_BOOT_QUICKTURN) {
      to_usec = SCHAN_TIMEOUT_QT;
    } else if (SAL_BOOT_BCMSIM) {
      to_usec = SCHAN_TIMEOUT_PLI;
    } else {
      to_usec = SCHAN_TIMEOUT;
    }
    
    /*
     * Cannot test on model, so don't bother
     */
     if (SAL_BOOT_PLISIM) {
       return SOC_E_NONE;
     }

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META("soc_sbx_caladan3_flush_segment: sleep %d\n"), to_usec));

    (void)sal_sem_take(cal->cs.CsFifoSem, to_usec);


    /*
     * Check to see if the CS Block set the EJECT_DONE bit.
     * If so, clear it.CS_MANUAL_EJECT_CTRLr
     */
    
    regval = 0;
    if ((rv = READ_CS_MANUAL_EJECT_CTRLr(unit, &regval)) < 0) {
      LOG_INFO(BSL_LS_SOC_COUNTER,
               (BSL_META("CS: unable to write CS_MANUAL_EJECT_CTRLr register for unit %d with %x\n"),
                unit, regval));
    } else {
      
      nInitDone = soc_reg_field_get(unit,CS_MANUAL_EJECT_CTRLr,regval,EJECT_DONEf);
      
      /*
       * Manual flush complete. Free up the flush resource again.
       */
      
      cal->cs.flags &= ~CS_FLUSHING;
      
      if (!nInitDone) {
	LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("MANUAL EJECT done timeout\n")));
      } else {
	
	/*
	 * Clear the EJECT_DONE bit
	 */
	
	regval = 0;
	soc_reg_field_set(unit, CS_MANUAL_EJECT_CTRLr, &regval, EJECT_DONEf,1);
	if ((rv = WRITE_CS_MANUAL_EJECT_CTRLr(unit, regval)) < 0) {
	  LOG_INFO(BSL_LS_SOC_COUNTER,
                   (BSL_META("CS: unable to write CS_MANUAL_EJECT_CTRLr register for unit %d with %x\n"),
                    unit, regval));
	}
      }
    }

    return SOC_E_NONE;
}



uint32
soc_caladan3_qm_counter_base_set(int unit,
			       int32 nBaseQueue,
			       int32 enable)
{
    if (nBaseQueue > SOC_SBX_CFG(unit)->num_queues - 16) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Invalid Base Queue Requested %d [0x%x]\n"), nBaseQueue, nBaseQueue));
        return SOC_E_PARAM;
    }



    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Set Counter Base to %d [0x%x]\n"), nBaseQueue, nBaseQueue));
    SOC_IF_ERROR_RETURN(WRITE_QMB_SELECTED_Qr(unit, nBaseQueue));

    return SOC_E_NONE;
}


uint32
soc_caladan3_qm_counter_base_get(int unit,
			       int32 *nBaseQueue)
{
    uint32 regval = 0;

    SOC_IF_ERROR_RETURN(READ_QMB_SELECTED_Qr(unit, &regval));
    LOG_VERBOSE(BSL_LS_SOC_COMMON,
                (BSL_META_U(unit,
                            "Get Counter Base to %d [0x%x]\n"), regval, regval));
    *nBaseQueue = regval;
    return SOC_E_NONE;
}


uint32
soc_caladan3_qm_counter_read(int unit,
			   int32 set,
			   uint32 *puCounterBase)
{
    uint32 regval = 0;
    uint32 uPacketCount = 0;
    uint32 uByteCount = 0;
    int nCounter = 0;

    /* We only get the requested queue at a time */

    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, SLQ_PTRf, set*16 + nCounter);
    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, GOf, 1);
    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, AUTO_INCRf, 1);
    SOC_IF_ERROR_RETURN(WRITE_QMB_SLQ_PTRr(unit, regval));

    for ( nCounter=0; nCounter < (CALADAN3_COUNTER_QM_COUNT/2); nCounter++) {
	
	SOC_IF_ERROR_RETURN(READ_QMB_SLQ_PKT_CNTr(unit, &uPacketCount));
	if (nCounter == ((CALADAN3_COUNTER_QM_COUNT/2)-1)) {
	  soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, AUTO_INCRf, 0);
	  SOC_IF_ERROR_RETURN(WRITE_QMB_SLQ_PTRr(unit, regval));
	}
	SOC_IF_ERROR_RETURN(READ_QMB_SLQ_BYTE_CNTr(unit, &uByteCount));

#define REAL_COUNTER
#ifdef REAL_COUNTER
        *puCounterBase++ = uPacketCount;
        *puCounterBase++ = uByteCount;
        if (uPacketCount || uByteCount ) {
            LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                        (BSL_META_U(unit,
                                    "Q %d C %d p %d b %d\n"), set, nCounter, uPacketCount, uByteCount));
        }
#else
        /* Simulate Counter action */
        *puCounterBase++ = 1 + (nCounter*2);
        *puCounterBase++ = 2 + (nCounter*2);
#endif
    }

    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, SLQ_PTRf, (set*16 + nCounter) & 0xFF);
    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, GOf, 0);
    soc_reg_field_set(unit, QMB_SLQ_PTRr, &regval, AUTO_INCRf, 0);
    SOC_IF_ERROR_RETURN(WRITE_QMB_SLQ_PTRr(unit, regval));

    return SOC_E_NONE;
}
#endif /* ENABLE_DEFUNCT_STATS */
/*
 * Function:
 *     _soc_caladan3_counter_qm_read
 * Purpose:
 *     Read specified am counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in AM block
 *     counter - Counter in given set to read
 *     val     - (OUT) Counter value
 *     width   - (OUT) Bits in hardware register counter
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_caladan3_counter_qm_read(int unit, int set, int counter,
                            uint64 *val, int *width)
{
    int     rv COMPILER_ATTRIBUTE((unused));
#ifdef ENABLE_DEFUNCT_STATS
    uint32  data[CALADAN3_COUNTER_QM_COUNT];
#endif /* ENABLE_DEFUNCT_STATS */
    int counter_index;

    LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                (BSL_META_U(unit,
                            "_soc_caladan3_counter_qm_read set %d cntr %d enb %d\n"),
                 set, counter, soc_caladan3_counter_enable_flag[unit]));

    rv = SOC_E_NONE;

    if (soc_caladan3_counter_enable_flag[unit] != TRUE) {
        for ( counter_index = 0 ; counter_index < CALADAN3_COUNTER_QM_COUNT; counter_index++) {
          COMPILER_64_ZERO(*val);
         val++;
        }
        return SOC_E_NONE;
    }
#ifdef ENABLE_DEFUNCT_STATS
    rv = soc_caladan3_qm_counter_read(unit, set, &data[0]);

    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "caladan3 counter read failure\n")));
        return rv;
    }

    /*
     * Skip 'get' field from data value, since it's not needed here
     */
    for ( counter_index = 0 ; counter_index < CALADAN3_COUNTER_QM_COUNT; counter_index++) {
        if ( data[counter_index]) {
            LOG_INFO(BSL_LS_SOC_COUNTER,
                     (BSL_META_U(unit,
                                 " Count %d:%d: %08x, "), set, counter_index, data[counter_index]));
        } else {
            LOG_VERBOSE(BSL_LS_SOC_COUNTER,
                        (BSL_META_U(unit,
                                    " Count %d:%d: %08x, "), set, counter_index, data[counter_index]));
        }

         COMPILER_64_SET((*val), 0, data[counter_index]);
         val++;

    }
#endif /* ENABLE_DEFUNCT_STATS */
    *width = CALADAN3_COUNTER_REG_QM_WIDTH;

    return SOC_E_NONE;
}


/*
 * Function:
 *     _soc_caladan3_counter_qm_write
 * Purpose:
 *     Write specified AM counter.
 * Parameters:
 *     unit    - Device number
 *     set     - Counter set in AM block
 *     counter - Counter in given set
 *     val     - Counter value to write
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 * Notes:
 *    Assumes unit and set are valid.
 */
STATIC int
_soc_caladan3_counter_qm_write(int unit, int set, int counter, uint64 val)
{
    int     rv = SOC_E_NONE;

    return rv;
}
#ifdef ENABLE_DEFUNCT_STATS
/*
 * Function:
 *     soc_sbx_caladan3_counter_port_get
 * Purpose:
 *     Get the specified software-accumulated counter value for given port.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     counter - Counter
 *     val     - (OUT) Software accumulated value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_counter_port_get(int unit, int base_queue,
                                int block, int set,
                                int counter, uint64 *val)
{
    
    /* Should validate base counter is in agreement */

    return soc_sbx_counter_get(unit, block, set, counter, val);
}


/*
 * Function:
 *     soc_sbx_caladan3_counter_port_set
 * Purpose:
 *     Set the counter value for given port to the requested value.
 * Parameters:
 *     unit    - Device number
 *     port    - Device port number
 *     counter - Counter to set (a negative value means all counters for port)
 *     val     - Counter value
 * Returns:
 *     SOC_E_NONE - Success
 *     SOC_E_XXX  - Failure
 */
int
soc_sbx_caladan3_counter_port_set(int unit, int base_queue,
                                int block, int set,
                                int counter, uint64 val)
{
    int rv = SOC_E_NONE;
    int  num_counters;
    uint64 uuZero = COMPILER_64_INIT(0,0);
    
    /* Should add check that base_queue address is in agreement with set value */

    /* Set value to all counters on given port */
    num_counters = caladan3_counter_blocks[block].num_counters;

    if( counter >= num_counters ) return SOC_E_PARAM;

    rv = soc_sbx_counter_set(unit, block, set, counter, uuZero);

    return rv;
}

/*
 * Get the state of the enable and base
 */

int
soc_caladan3_counter_enable_get(int unit, int *base, int *enable)
{
    *enable = soc_caladan3_counter_enable_flag[unit];
    return BCM_E_UNAVAIL;
}

int
soc_caladan3_counter_enable_set(int unit, int base)
{
    if(soc_caladan3_counter_enable_flag[unit]) return BCM_E_INTERNAL;

    soc_sbx_counter_bset_clear( unit, 0);

    soc_caladan3_counter_enable_flag[unit] = TRUE;
    soc_caladan3_counter_current_base[unit] = base ;
    return BCM_E_NONE;
}

/*
 * Clear the current counter. We do not currently check for correct base 
 */
int
soc_caladan3_counter_enable_clear(int unit)
{
    soc_caladan3_counter_enable_flag[unit] = FALSE;
    soc_caladan3_counter_current_base[unit] = -1;
    
    return BCM_E_NONE;
}
#endif /*  ENABLE_DEFUNCT_STATS */
#else
int caladan3_counter_not_empty;
#endif /* BCM_CALADAN3_SUPPORT */
