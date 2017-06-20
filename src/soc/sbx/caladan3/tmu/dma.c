/*
 * $Id: dma.c,v 1.35.24.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * Purpose: Caladan3 on TMU dma manager
 * Requires:
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <soc/sbx/caladan3/tmu/tmu.h>
#include <shared/util.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <sal/core/time.h>
#include <soc/sbx/caladan3/tmu/cmd.h>
#include <soc/sbx/caladan3/tmu/dma.h>
#include <soc/sbx/caladan3/soc_sw_db.h>


/* DMA Command Manager */
/* Purpose: consumes commands from DM/FIB & issues them to device */
/* circular produce consumer buffer */
/* DM/FIB managers post pointers to command buffer & dma manager
 * consumes & trasmits into device FIFO DMA */

#define SOC_SBX_CALADAN3_TMU_CMD_FIFO_SIZE_IN_64b_WORDS (1024) /* 1k entries */
#define SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS (256)
#define SOC_SBX_TMU_CMD_DMA_MGR_SEQ_MAX (63)

/* Note: Maximum ring pointer supported by CMIC = 32K x 32words or 16K x 64bit entries */
#define SOC_SBX_TMU_RESP_DMA_MGR_RING_SIZE (16 * 1024) 

#define TMU_DBASE(unit) (_tmu_dbase[unit])

#define TMU_FIFO_CMD_MGR(unit,fifoid) (TMU_DBASE(unit)->cmd_dma_cfg.cmdmgr[fifoid])
#define TMU_FIFO_RESP_MGR(unit,fifoid) (TMU_DBASE(unit)->resp_dma_cfg.respmgr[fifoid])

#define TMU_INCR_SEQ_NUM(seqnum) \
    do { if(seqnum == SOC_SBX_TMU_CMD_DMA_MGR_SEQ_MAX){ seqnum = 0;} else {seqnum++;} } while(0)    

unsigned int tmu_dma_rx_debug=0, tmu_dma_tx_debug=0, tmu_dma_skip_tx=0, tmu_dma_skip_rx=0;

#define TIME_STAMP_DBG
#undef TIME_STAMP_DBG
#ifdef TIME_STAMP_DBG 
static sal_usecs_t        start;
#define TIME_STAMP_START start = sal_time_usecs();
#define TIME_STAMP(msg)                                             \
  do {                                                              \
    LOG_CLI((BSL_META("\n %s: Time Stamp: [%u]"),                         \
             msg, SAL_USECS_SUB(sal_time_usecs(), start)));      \
  } while(0);
#else
#define TIME_STAMP_START 
#define TIME_STAMP(msg)   
#endif


#ifdef TMU_DMA_USE_LOCKS
#ifdef TMU_DMA_USE_SPINLOCK
#include <pthread.h>
#define TMU_DMA_LOCK(unit)      pthread_spin_lock(&_tmu_dbase[unit]->dma_spin_lock); 
#define TMU_DMA_UNLOCK(unit)    pthread_spin_unlock(&_tmu_dbase[unit]->dma_spin_lock);
#else
#define TMU_DMA_LOCK(unit)     sal_mutex_take(_tmu_dbase[unit]->dma_mutex, sal_mutex_FOREVER);
#define TMU_DMA_UNLOCK(unit)   sal_mutex_give(_tmu_dbase[unit]->dma_mutex);
#endif 
#else
#define TMU_DMA_LOCK(unit)    
#define TMU_DMA_UNLOCK(unit)    
#endif


#define SOC_SBX_TMU_NUM_CMD_WORDS (BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE))


int soc_dma_reg_field_set(int unit, soc_reg_t reg, uint32 *regval,
                      soc_field_t field, uint32 value) {
    soc_reg_field_set(unit, reg, regval, field, value);
    return SOC_E_NONE;
}


/*
 *
 * Function:
 *     tmu_dma_tx
 * Purpose:
 *     TMU Transmit DMA
 */
int tmu_dma_tx(int unit, 
               int fifoid, 
               soc_sbx_caladan3_tmu_cmd_t *cmd,
               soc_sbx_tmu_cmd_post_flag_e_t flag) 
{
    soc_sbx_caladan3_tmu_cmd_t nop_cmd;
    int status=SOC_E_NONE, num_bits=0, org_seq_num;
    int index, num_nop_cmd=0;
    soc_mem_t cmd_fifo_mem[SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO] = \
                   {TMB_UPDATER_CMD_FIFO0m, TMB_UPDATER_CMD_FIFO1m};

    if (tmu_dma_tx_debug) {
        LOG_CLI((BSL_META_U(unit,
                            " DMA TX: \n")));
    }

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

TIME_STAMP_START

    if (cmd) { 
        tmu_cmd_get_size_bits(cmd, &num_bits);
    }

    org_seq_num = TMU_FIFO_CMD_MGR(unit,fifoid).seq_num;

    /* validation */
    if (flag == SOC_SBX_TMU_CMD_POST_CACHE) {

        /* verify if the new command could be cached */
        /* if full request users to flush them */
        if (BITS2WORDS(num_bits) > (TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len - 
            (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer -
             TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer))) {
	    LOG_DEBUG(BSL_LS_SOC_COMMON,
	              (BSL_META_U(unit,
	                          "Command Cache full, issue a flush !!!! \n")));
            status = SOC_E_FULL;
        }

    } else if (flag == SOC_SBX_TMU_CMD_POST_FLUSH) {
        if (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer == 
            TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer) {
            LOG_DEBUG(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "No Commands cached to flush !!!! \n")));
            status = SOC_E_EMPTY;
        }
    } else if (flag == SOC_SBX_TMU_CMD_POST_FLAG_NONE) {
        if (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer != 
            TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer) {
            LOG_CLI((BSL_META_U(unit,
                                "Commands cached pending to be flushed out !!!! \n")));
            status = SOC_E_PARAM;
        }
    }

    if (SOC_SUCCESS(status)) {
        /* flush does not accept commands */
        if (cmd) { 
            /* Assign sequence number */
            cmd->seqnum = TMU_FIFO_CMD_MGR(unit,fifoid).seq_num;
            TMU_INCR_SEQ_NUM(TMU_FIFO_CMD_MGR(unit,fifoid).seq_num);

            status = tmu_cmd_writeBuf(unit, cmd,
                                      (uint32*)TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer,
                                      TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len - 
                                      (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer - 
                                       TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer));
            
            if (tmu_dma_tx_debug) {
                tmu_cmd_printf(unit, cmd);
            }
            
            TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer += BITS2WORDS(num_bits);
        }
        
        /* flush to hardware not cache command */
        if (flag != SOC_SBX_TMU_CMD_POST_CACHE) {

            num_bits = WORDS2BITS(TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer - 
                                  TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer);

            /* Pad to 256bit boundary */
            if (num_bits % SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS > 0) {
                num_nop_cmd = SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS -
                                  (num_bits % SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS);
                assert(num_nop_cmd % SOC_SBX_TMU_CMD_WORD_SIZE == 0); /* must be on 64bit boundary */
                num_nop_cmd /= SOC_SBX_TMU_CMD_WORD_SIZE;

                tmu_cmd_init(&nop_cmd);

                while(SOC_SUCCESS(status) && (num_nop_cmd--)) {
                    nop_cmd.opcode = SOC_SBX_TMU_CMD_NOP;
                    nop_cmd.seqnum = TMU_FIFO_CMD_MGR(unit,fifoid).seq_num;
                    nop_cmd.cmd.nop.echo = 0;
                    TMU_INCR_SEQ_NUM(TMU_FIFO_CMD_MGR(unit,fifoid).seq_num);
                    
                    status = tmu_cmd_writeBuf(unit, &nop_cmd,
                                              (uint32*)TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer,
                                              TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len -
                                              (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer - 
                                               TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer));

                    TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer += SOC_SBX_TMU_NUM_CMD_WORDS;
                    num_bits += SOC_SBX_TMU_CMD_WORD_SIZE;

                    if (tmu_dma_tx_debug) {
                        tmu_cmd_printf(unit, &nop_cmd);
                    }
                }
            }

            if ((num_bits % SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS) != 0) {
		assert(0);
	    }

            if (tmu_dma_tx_debug) {
                LOG_CLI((BSL_META_U(unit,
                                    " DMA TX BUFFER: \n")));
                for(index=0; index < BITS2WORDS(num_bits); index++) {
                    LOG_CLI((BSL_META_U(unit,
                                        "0x%08X "), TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer[index]));
                }
                LOG_CLI((BSL_META_U(unit,
                                    "\n")));
            }

            if (SOC_SUCCESS(status) && !SAL_BOOT_PLISIM && !tmu_dma_skip_tx) {
                /* kick start DMA */
                /*    coverity[negative_returns : FALSE]    */
                status = soc_mem_write_range(unit, cmd_fifo_mem[fifoid],
                                             MEM_BLOCK_ANY, 0,
                                             num_bits/SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS - 1, 
                                             (uint32 *)TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer);     
            }

            if (SOC_FAILURE(status)) {
                TMU_FIFO_CMD_MGR(unit,fifoid).seq_num = org_seq_num;
            }

            /* reset ring pointer to start of command buffer */
            TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer = TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer;
        }
    }
     
TIME_STAMP("%%% SBUS DMA time")
   
    TMU_DMA_UNLOCK(unit);
    return status;
}
/*
 *
 * Function:
 *     tmu_dma_tx_share_cache_cmd
 * Purpose:
 *     Cache cmds for both master and slave units
 */
int tmu_dma_tx_share_cache_cmd(int unit, int *slave_units, int num_slaves, int fifoid, 
               soc_sbx_caladan3_tmu_cmd_t *cmd) 
{
    int status=SOC_E_NONE, num_bits=0, slave_idx = 0;

    if (tmu_dma_tx_debug) {
        LOG_CLI((BSL_META_U(unit,
                            " DMA TX: \n")));
    }

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    TMU_DMA_LOCK(unit);

TIME_STAMP_START

    tmu_cmd_get_size_bits(cmd, &num_bits);

    /* validation */
    /* verify if the new command could be cached */
    /* if full request users to flush them */
    if (BITS2WORDS(num_bits) > (TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len - 
        (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer -
         TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer))) {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Command Cache full, issue a flush !!!! \n")));
        status = SOC_E_FULL;
    }

    if (SOC_SUCCESS(status)) {
        /* flush does not accept commands */
        /* Assign sequence number */
        cmd->seqnum = TMU_FIFO_CMD_MGR(unit,fifoid).seq_num;
        TMU_INCR_SEQ_NUM(TMU_FIFO_CMD_MGR(unit,fifoid).seq_num);

        status = tmu_cmd_writeBuf(unit, cmd,
                                  (uint32*)TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer,
                                  TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len - 
                                  (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer - 
                                   TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer));
        
        if (tmu_dma_tx_debug) {
            tmu_cmd_printf(unit, cmd);
        }
        while(slave_idx < num_slaves){
            sal_memcpy((void *)TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer, 
                        (void *)TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer,
                        sizeof(uint32) * BITS2WORDS(num_bits));
            cmd->slave_seqnum[slave_idx] = TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).seq_num;
            TMU_INCR_SEQ_NUM(TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).seq_num);
            
            tmu_cmd_seqnum_set_in_buffer(cmd->slave_seqnum[slave_idx], 
                            (uint32*)TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer);
            TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer += BITS2WORDS(num_bits);
            ++slave_idx;
        }
        
        TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer += BITS2WORDS(num_bits);
    }
     
TIME_STAMP("%%% SBUS DMA time")
   
    TMU_DMA_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     tmu_dma_tx_master_flush_cmd
 * Purpose:
 *     Fluch all cached cmds for both master and slave units
 */
int tmu_dma_tx_master_flush_cmd(int unit, int *slave_units, int num_slaves, int fifoid) 
{
    soc_sbx_caladan3_tmu_cmd_t nop_cmd;
    int status=SOC_E_NONE, num_bits=0, org_seq_num;
    int index, num_nop_cmd=0, slave_idx = 0;
    soc_mem_t cmd_fifo_mem[SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO] = \
                   {TMB_UPDATER_CMD_FIFO0m, TMB_UPDATER_CMD_FIFO1m};

    if (tmu_dma_tx_debug) {
        LOG_CLI((BSL_META_U(unit,
                            " DMA TX: \n")));
    }

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

TIME_STAMP_START

    org_seq_num = TMU_FIFO_CMD_MGR(unit,fifoid).seq_num;

    /* validation */
    if (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer == 
        TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer) {
        LOG_DEBUG(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "No Commands cached to flush !!!! \n")));
        status = SOC_E_EMPTY;
    }
    
    if (SOC_SUCCESS(status)) {
        
        /* flush to hardware not cache command */

        num_bits = WORDS2BITS(TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer - 
                              TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer);

        /* Pad to 256bit boundary */
        if (num_bits % SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS > 0) {
            num_nop_cmd = SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS -
                              (num_bits % SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS);
            assert(num_nop_cmd % SOC_SBX_TMU_CMD_WORD_SIZE == 0); /* must be on 64bit boundary */
            num_nop_cmd /= SOC_SBX_TMU_CMD_WORD_SIZE;
            tmu_cmd_init(&nop_cmd);

            while(SOC_SUCCESS(status) && (num_nop_cmd--)) {
                nop_cmd.opcode = SOC_SBX_TMU_CMD_NOP;
                nop_cmd.seqnum = TMU_FIFO_CMD_MGR(unit,fifoid).seq_num;
                nop_cmd.cmd.nop.echo = 0;
                TMU_INCR_SEQ_NUM(TMU_FIFO_CMD_MGR(unit,fifoid).seq_num);
                
                status = tmu_cmd_writeBuf(unit, &nop_cmd,
                                          (uint32*)TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer,
                                          TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len -
                                          (TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer - 
                                           TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer));

                while (slave_idx < num_slaves) {
                    sal_memcpy((void *)TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer, 
                                (void *)TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer,
                                sizeof(uint32) * SOC_SBX_TMU_NUM_CMD_WORDS);
                    nop_cmd.slave_seqnum[slave_idx] = TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).seq_num;
                    TMU_INCR_SEQ_NUM(TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).seq_num);
            
                    tmu_cmd_seqnum_set_in_buffer(nop_cmd.slave_seqnum[slave_idx], 
                                        (uint32*)TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer);
                    TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer += SOC_SBX_TMU_NUM_CMD_WORDS;
                    ++slave_idx;
                }
                slave_idx = 0;
                TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer += SOC_SBX_TMU_NUM_CMD_WORDS;
                num_bits += SOC_SBX_TMU_CMD_WORD_SIZE;

                if (tmu_dma_tx_debug) {
                    tmu_cmd_printf(unit, &nop_cmd);
                }
            }
        }

    	assert((num_bits % SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS) == 0);

        if (tmu_dma_tx_debug) {
            LOG_CLI((BSL_META_U(unit,
                                " DMA TX BUFFER: \n")));
            for(index=0; index < BITS2WORDS(num_bits); index++) {
                LOG_CLI((BSL_META_U(unit,
                                    "0x%08X "), TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer[index]));
            }
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }

        if (SOC_SUCCESS(status) && !SAL_BOOT_PLISIM && !tmu_dma_skip_tx) {
            /* kick start DMA */
            /*    coverity[negative_returns : FALSE]    */
            status = soc_mem_write_range(unit, cmd_fifo_mem[fifoid],
                                         MEM_BLOCK_ANY, 0,
                                         num_bits/SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS - 1, 
                                         (uint32 *)TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer);
            if (SOC_FAILURE(status)) {
                LOG_CLI((BSL_META_U(unit,
                                    "fail to write mem unit  %d\n"), unit));
            }
            while (slave_idx < num_slaves) {
                /*    coverity[negative_returns : FALSE]    */
                status = soc_mem_write_range(slave_units[slave_idx], cmd_fifo_mem[fifoid],
                                             MEM_BLOCK_ANY, 0,
                                             num_bits/SOC_SBX_CALADAN3_TMU_FIFO_BOUNDARY_BITS - 1, 
                                             (uint32 *)TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).dma_buffer);
                if (SOC_FAILURE(status)) {
                    LOG_CLI((BSL_META_U(unit,
                                        "fail to write mem unit 1\n")));
                }
                ++slave_idx;
            }
            slave_idx = 0;
        }

        if (SOC_FAILURE(status)) {
            LOG_CLI((BSL_META_U(unit,
                                "num_bits %d\n"), num_bits));
            TMU_FIFO_CMD_MGR(unit,fifoid).seq_num = org_seq_num;
        }
        while (slave_idx < num_slaves){
            TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).ring_pointer 
                    = TMU_FIFO_CMD_MGR(slave_units[slave_idx],fifoid).dma_buffer;
            ++slave_idx;
        }
        /* reset ring pointer to start of command buffer */
        TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer = TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer;
    }
     
TIME_STAMP("%%% SBUS DMA time")
   
    TMU_DMA_UNLOCK(unit);
    return status;
}


/*
 *
 * Function:
 *     tmu_dma_rx
 * Purpose:
 *  TMU response manager thread
 *  Waits for a timeout to see if device responds. Waits wise enought to 
 *  make sure that response is consumed till a point where TMU can consume.
 *  DOES NOT MOVE ring pointers. TMU moves the ring pointer for now.  
 */
int tmu_dma_rx(int unit, int fifoid, soc_sbx_caladan3_tmu_cmd_t *response) 
{
    int status=SOC_E_NONE, num_data_beat_words, iteration=0, ringptr_to_end_words=0;
    int num_taps_responses;
    uint8 empty=TRUE;
    soc_mem_t resp_fifo_mem[] = {TMB_UPDATER_RSP_FIFO0m, TMB_UPDATER_RSP_FIFO1m};
    sal_usecs_t curr_time=0, time_stamp=0;
    soc_sbx_caladan3_tmu_cmd_t nextcmd;
    int index;
    VOL uint32* ringptr=NULL;

#define SOC_SBX_TMU_DEF_QT_RESP_TIMEOUT (50  * MILLISECOND_USEC) 
#define SOC_SBX_TMU_DEF_RESP_TIMEOUT (10 * MILLISECOND_USEC)

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO || !response) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

    tmu_cmd_init(response);
    
    num_data_beat_words = soc_mem_entry_words(unit, resp_fifo_mem[fifoid]);
    ringptr = TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer;

    assert(TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len - 
           (ringptr - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer) >=
           num_data_beat_words);
#if 0
    /* wrap ring pointer if required */
    if (TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len == 
        ringptr - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer) {
        ringptr = TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer;
    }
#endif
    /* Trailer words are supposed to terminate commands , until then
     * look for all zero words */
    do {
        iteration++;
        soc_cm_sinval(unit, (void *)ringptr, WORDS2BYTES(num_data_beat_words));

        status = tmu_cmd_readBuf(unit, response, (uint32*)ringptr,
                                 num_data_beat_words, TRUE);
        
        if (SOC_SUCCESS(status)) {
            /*LOG_CLI((BSL_META_U(unit,
                                  "### Flushed DMA Rx buffer for command \n")));*/
            if (tmu_dma_rx_debug) {
                do {
                    LOG_CLI((BSL_META_U(unit,
                                        "$$$RX: DMA Buffer [%p] Ring Pointer [%p] \n"), 
                             (void *)TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer,
                             (void *)ringptr));
#if 0
                    for(index=0; index < ringptr - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer; index++) {
                        LOG_CLI((BSL_META_U(unit,
                                            "0x%08X "), TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer[index]));
                    }
#endif
                    LOG_CLI((BSL_META_U(unit,
                                        "\n DMA RX BUFFER: \n")));
                    for(index=0; index < num_data_beat_words; index++) {
                        LOG_CLI((BSL_META_U(unit,
                                            "0x%08X "), ringptr[index]));
                    }
                    LOG_CLI((BSL_META_U(unit,
                                        "\n")));
                    
                    tmu_cmd_printf(unit, response);
                } while(0);
            }
            tmu_cmd_is_empty_response(unit, response, &empty);

            if (empty) {
		if (time_stamp == 0) {
		    time_stamp = sal_time_usecs();
		    sal_usleep(100);
		} else {
		    curr_time = sal_time_usecs();
		    if (SAL_USECS_SUB(curr_time,time_stamp) >
			((SAL_BOOT_QUICKTURN)?SOC_SBX_TMU_DEF_QT_RESP_TIMEOUT:SOC_SBX_TMU_DEF_RESP_TIMEOUT)) {
			LOG_CLI((BSL_META_U(unit,
                                            "!!! TMU DMA RX Timeout: Iterations(%d)!!! \n"), iteration));
			status = SOC_E_TIMEOUT;
		    } else {
			sal_usleep(200);
		    }
		}
            } else {
                iteration = 0;
                empty = TRUE;
                tmu_cmd_init(&nextcmd);
                response->response_trailer = FALSE;
		time_stamp = 0;

                /* wrap and flush as applicable */
                ringptr += num_data_beat_words;
                ringptr_to_end_words = TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len -
                                       (ringptr - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer);

                while (empty && SOC_SUCCESS(status)) {

		    if (response->response_type == SOC_SBX_TMU_RESPONSE) {
			/* non-TAPS response */
			if (ringptr_to_end_words < (num_data_beat_words * (response->resp_ctrl.resp.size + 1))) {
			    soc_cm_sinval(unit,(void *)ringptr, WORDS2BYTES(1) * ringptr_to_end_words);
			    soc_cm_sinval(unit,(void *)TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer,
					  WORDS2BYTES(1) * 
					  (num_data_beat_words * (response->resp_ctrl.resp.size + 1) - 
					   ringptr_to_end_words));
			    if (iteration == 0) {
				ringptr =  TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer +
				    (num_data_beat_words * response->resp_ctrl.resp.size) -
				    ringptr_to_end_words;
			    }
			} else {
			    soc_cm_sinval(unit,(void *)ringptr,
					  WORDS2BYTES(num_data_beat_words) * (response->resp_ctrl.resp.size + 1));
			    if (iteration == 0) {
				ringptr += response->resp_ctrl.resp.size * 2;
			    }
			}
		    } else {
			/* TAPS response */
			num_taps_responses = (response->resp_ctrl.taps.valid0)?1:0 + (response->resp_ctrl.taps.valid1)?1:0;
			if (ringptr_to_end_words < (num_taps_responses * BITS2WORDS(SOC_SBX_TMU_TAPS_RESPONSE_SIZE)
						    + num_data_beat_words)) {
			    soc_cm_sinval(unit,(void *)ringptr, WORDS2BYTES(1) * ringptr_to_end_words);
			    soc_cm_sinval(unit,(void *)TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer,
					  (BITS2BYTES(SOC_SBX_TMU_TAPS_RESPONSE_SIZE) * num_taps_responses + 
					   WORDS2BYTES(num_data_beat_words) - ringptr_to_end_words));
			    if (iteration == 0) {
				ringptr =  TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer +
				    BITS2WORDS(SOC_SBX_TMU_TAPS_RESPONSE_SIZE) * num_taps_responses -
				    ringptr_to_end_words;
			    }
			} else {
			    soc_cm_sinval(unit, (void *)ringptr,
					  BITS2BYTES(SOC_SBX_TMU_TAPS_RESPONSE_SIZE) * num_taps_responses + 
					  WORDS2BYTES(num_data_beat_words));
			    if (iteration == 0) {
				ringptr += BITS2WORDS(SOC_SBX_TMU_TAPS_RESPONSE_SIZE) * num_taps_responses;
			    }
			}
		    }

                    iteration++;

                    /* Look for next command word or trailers to conclude all data made through */
                    /*SOC_SBX_TMU_CMD_TRAILER*/
                    status = tmu_cmd_readBuf(unit, &nextcmd,
                                             (uint32*)ringptr,
                                             num_data_beat_words,
                                             TRUE);
                    if (SOC_FAILURE(status)) {
                        LOG_CLI((BSL_META_U(unit,
                                            "!!! %d Data/Trailer DMA buffer processing failed: %d !!! \n"), unit, status));
                    }

                    tmu_cmd_is_empty_response(unit, &nextcmd, &empty);

		    if (empty) {
			if (time_stamp == 0) {
			    time_stamp = sal_time_usecs();
			    sal_usleep(100);
			} else {
			    curr_time = sal_time_usecs();
			    if (SAL_USECS_SUB(curr_time,time_stamp) >
				((SAL_BOOT_QUICKTURN)?SOC_SBX_TMU_DEF_QT_RESP_TIMEOUT:SOC_SBX_TMU_DEF_RESP_TIMEOUT)) {
				status = SOC_E_TIMEOUT;
				LOG_CLI((BSL_META_U(unit,
                                                    "!!! TMU DMA RX Next command Timeout: Iteration(%d) !!! \n"), iteration));
			    } else {
				sal_usleep(200);
			    }
			}
		    } else { 
			if (nextcmd.opcode == SOC_SBX_TMU_CMD_TRAILER) {
			    response->response_trailer = TRUE;
			}
		    }
                }

                if (tmu_dma_rx_debug) {
                    for(index=0; index < num_data_beat_words; index++) {
                        LOG_CLI((BSL_META_U(unit,
                                            "0x%08X "), ringptr[index]));
                    }
                    LOG_CLI((BSL_META_U(unit,
                                        "\n")));
                    tmu_cmd_printf(unit, &nextcmd);
                }
            }
        } else {
            LOG_CLI((BSL_META_U(unit,
                                "!!! %d Response DMA buffer processing failed: %d !!!\n"), unit, status));
        }
    } while (SOC_SUCCESS(status) && empty) ;

    TMU_DMA_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     tmu_dma_rx_advance_ring
 * Purpose:
 *     advance dma rx ring pointer
 */
int tmu_dma_rx_advance_ring(int unit, int fifoid, int advance_num_words) 
{
    int status = SOC_E_NONE, ringptr_to_end_words;
    int index, num_words=advance_num_words;

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

    if (tmu_dma_rx_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "$$$$$ Old Ring Pointer: %p - Words to advance [%d]\n"),
                 (void *)TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer, num_words));
    }

    ringptr_to_end_words = TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len - 
                            (TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer
                             - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer);

    /* zero out consumed area */
    /* rollover check */
    if (ringptr_to_end_words < num_words) {
        for (index = 0; index < ringptr_to_end_words; index++) {
            TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer[index] = 0;
            /*sal_memset(TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer, 0, 
              ringptr_to_end_words * sizeof(unsinged int));*/
        }

        TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer = TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer;
        num_words -= ringptr_to_end_words;
    }

    for (index = 0; index < num_words; index++) {
        TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer[index] = 0;
        /*sal_memset(TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer, 0, 
          num_words * sizeof(unsinged int));*/
    }

    TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer += num_words;
    if (TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len ==  
                            (TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer
                             - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer)) {
        TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer = TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer;        
        if (tmu_dma_rx_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "$$$$ Wrapping Response Ring pointer ..... \n")));
        }
    }

    soc_pci_write(unit, 
                  CMIC_CMCx_FIFO_CHy_RD_DMA_NUM_OF_ENTRIES_READ_FRM_HOSTMEM_OFFSET(0, 
                  TMU_FIFO_RESP_MGR(unit,fifoid).channel), advance_num_words/2);

    if (tmu_dma_rx_debug) {
        LOG_CLI((BSL_META_U(unit,
                            "$$$$$ New Ring Pointer: %p \n"), (void *)TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer));
    }

    TMU_DMA_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     tmu_dma_rx_copy_data
 * Purpose:
 *     copy received data from dma rx buffer
 */
int tmu_dma_rx_copy_data(int unit, 
                         int fifoid, 
                         uint32 *data_buffer,
                         int data_buffer_len, 
                         int resp_data_size_64b) 
{
    int status = SOC_E_NONE, ringptr_to_end_words;
    int offset, resp_data_size, index;

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO || !data_buffer) {
        return SOC_E_PARAM;
    }

    resp_data_size = resp_data_size_64b * 2; /* convert 64 bit size into 32 bits */
    offset = BITS2WORDS(SOC_SBX_TMU_CMD_WORD_SIZE);

    if (data_buffer_len < resp_data_size) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

    if (data_buffer_len < TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len) {

        ringptr_to_end_words = TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len - 
                              (TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer
                               - TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer);
        ringptr_to_end_words -= offset; /* offset control information */

        if (ringptr_to_end_words < resp_data_size) {
            sal_memcpy(data_buffer, 
                       (uint32*)(_tmu_dbase[unit]->resp_dma_cfg.respmgr[fifoid].ring_pointer + offset), 
                       ringptr_to_end_words * sizeof(uint32)); 

            if (tmu_dma_rx_debug) {
                for(index=0; index < ringptr_to_end_words; index++) {
                    LOG_CLI((BSL_META_U(unit,
                                        " 0x%08X "), data_buffer[index]));
                }
            }

            resp_data_size -= ringptr_to_end_words;

            sal_memcpy(&data_buffer[ringptr_to_end_words], 
                       (uint32 *)(_tmu_dbase[unit]->resp_dma_cfg.respmgr[fifoid].dma_buffer), 
                       resp_data_size * sizeof(uint32)); 

            if (tmu_dma_rx_debug) {
                for(index=ringptr_to_end_words; index < resp_data_size; index++) {
                    LOG_CLI((BSL_META_U(unit,
                                        " 0x%08X "), data_buffer[index]));
                }
            }
        } else {
            sal_memcpy(data_buffer, 
                       (void*)(_tmu_dbase[unit]->resp_dma_cfg.respmgr[fifoid].ring_pointer + offset), 
                       resp_data_size * sizeof(uint32)); 

            if (tmu_dma_rx_debug) {
                for(index=0; index < resp_data_size; index++) {
                    LOG_CLI((BSL_META_U(unit,
                                        " 0x%08X "), data_buffer[index]));
                }
            }
        }
        if (tmu_dma_rx_debug) {
            LOG_CLI((BSL_META_U(unit,
                                "\n")));
        }
    } else {
        status = SOC_E_PARAM;
    }

    TMU_DMA_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     tmu_cmd_dma_mgr_destroy
 * Purpose:
 *     destroy TMU command DMA manager
 */
int tmu_cmd_dma_mgr_destroy(int unit, int fifoid)
{
    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

    /* destroy thread */
    TMU_FIFO_CMD_MGR(unit,fifoid).thread_running = FALSE;

    /* release memories */
    if (TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer) {
        soc_cm_sfree(unit, (uint32*)TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer);
    }

    sal_memset(&TMU_FIFO_CMD_MGR(unit,fifoid), 0, sizeof(tmu_dma_mgr_dbase_t));
    TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer = NULL;
    TMU_FIFO_CMD_MGR(unit,fifoid).thread_id = NULL;    
    TMU_FIFO_CMD_MGR(unit,fifoid).valid = FALSE;

    TMU_DMA_UNLOCK(unit);
    return SOC_E_NONE;
}

/*
 *
 * Function:
 *     tmu_cmd_dma_mgr_uninit
 * Purpose:
 *     Cleanup
 */
int tmu_cmd_dma_mgr_uninit(int unit, int fifoid) 
{
        return(tmu_cmd_dma_mgr_destroy(unit, fifoid));
}

/*
 *
 * Function:
 *     tmu_cmd_dma_mgr_init
 * Purpose:
 *     Initialize command TMU DMA manager
 */
int tmu_cmd_dma_mgr_init(int unit, int fifoid)
{
    uint32 regval = 0;
    char buf[64];
    uint8 channels[] = {0,1};
    int status = SOC_E_NONE;

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);
    
    /* Allocate DMA buffer */
    TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len = 2 * SOC_SBX_CALADAN3_TMU_CMD_FIFO_SIZE_IN_64b_WORDS;
    
    sal_sprintf(&buf[0], "tmu-cmd-mgr-fifo:%d", fifoid);
    TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer = (uint32 *) soc_cm_salloc(unit,
                                                                        TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer_len * 
                                                                        sizeof(uint32),
                                                                        buf);
    if (TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer == NULL) {
        status = SOC_E_MEMORY;
    }

    if (SOC_SUCCESS(status)) {
        TMU_FIFO_CMD_MGR(unit,fifoid).ring_pointer = &TMU_FIFO_CMD_MGR(unit,fifoid).dma_buffer[0];
        TMU_FIFO_CMD_MGR(unit,fifoid).thread_running = TRUE;
        TMU_FIFO_CMD_MGR(unit,fifoid).seq_num = 0;
        TMU_FIFO_CMD_MGR(unit,fifoid).unit = unit;
        TMU_FIFO_CMD_MGR(unit,fifoid).valid = TRUE;
        TMU_FIFO_CMD_MGR(unit,fifoid).channel = channels[fifoid];
        TMU_FIFO_CMD_MGR(unit,fifoid).fifoid = fifoid;
        /* spawn a DMA command manager thread in future to bulk */
    }

    if (SOC_FAILURE(status)) {
        tmu_cmd_dma_mgr_destroy(unit, fifoid);
    } else {
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE_NO_ERR(READ_CMIC_CMC0_CONFIGr(unit, &regval), status);
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE_NO_ERR(soc_dma_reg_field_set(unit, CMIC_CMC0_CONFIGr, &regval, 
                                                            (channels[fifoid]) ? ENABLE_SBUSDMA_CH1_FLOW_CONTROLf:
                                                            ENABLE_SBUSDMA_CH0_FLOW_CONTROLf, 1 ), status);
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(WRITE_CMIC_CMC0_CONFIGr(unit, regval), status);
    }

    TMU_DMA_UNLOCK(unit);
    return status;
}

/*
 *
 * Function:
 *     tmu_resp_dma_mgr_destroy
 * Purpose:
 *     destroy TMU response DMA manager
 */
int tmu_resp_dma_mgr_destroy(int unit, int fifoid)
{
    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

    /* destroy thread */
    TMU_FIFO_RESP_MGR(unit,fifoid).thread_running = FALSE;

    /* release memories */
    if (TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer) {
        soc_cm_sfree(unit, (void*)TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer);
    }

    sal_memset(&TMU_FIFO_RESP_MGR(unit,fifoid), 0, sizeof(tmu_dma_mgr_dbase_t));
    TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer = NULL;
    TMU_FIFO_RESP_MGR(unit,fifoid).thread_id = NULL;    
    TMU_FIFO_RESP_MGR(unit,fifoid).valid = FALSE;


    TMU_DMA_UNLOCK(unit);
    return SOC_E_NONE;
}

#if 0
int tmu_caladan3_resp_fifo_dma_mgr_start(int nUnit, int fifoid)
{
    uint32 uDataBeats, uEntryNum, uRspRingSize;
    soc_mem_t aTmuUpdaterRspFifoMem[] = { TMB_UPDATER_RSP_FIFO0m, TMB_UPDATER_RSP_FIFO1m };
    int nRspFifo, copyno;
    uint8 at;
    uint32 uRegisterValue;
    schan_msg_t msg;
    

    READ_CMIC_CMC1_CONFIGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, CMIC_CMC1_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH0_FLOW_CONTROLf, 1 );
    soc_reg_field_set( nUnit, CMIC_CMC1_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH1_FLOW_CONTROLf, 1 );
    soc_reg_field_set( nUnit, CMIC_CMC1_CONFIGr, &uRegisterValue, ENABLE_SBUSDMA_CH2_FLOW_CONTROLf, 1 );
    WRITE_CMIC_CMC1_CONFIGr( nUnit, uRegisterValue );

    uRspRingSize = SOC_SBX_TMU_RESP_DMA_MGR_RING_SIZE;
    switch (uRspRingSize) {
    case 64:    uEntryNum = 0; break;
    case 128:   uEntryNum = 1; break;
    case 256:   uEntryNum = 2; break;
    case 512:   uEntryNum = 3; break;
    case 1024:  uEntryNum = 4; break;
    case 2048:  uEntryNum = 5; break;
    case 4096:  uEntryNum = 6; break;
    case 8192:  uEntryNum = 7; break;
    case 16384: uEntryNum = 8; break;
    default: assert(0);
    }
    uDataBeats = soc_mem_entry_words( nUnit, aTmuUpdaterRspFifoMem[0] );

  /*
   * Setup CMIC_CMC0_FIFO_CH2_RD_DMA and CMIC_CMC0_FIFO_CH3_RD_DMA for update response ring processing
  */
  for ( nRspFifo = 0; nRspFifo < 1; ++nRspFifo ) {

    /* Provision for 2us */
    /*WRITE_TMB_UPDATER_RSP_FIFO_INACTIVITY_THRESHr( nUnit, nRspFifo, 1200 );*/

    copyno = SOC_MEM_BLOCK_ANY(nUnit, aTmuUpdaterRspFifoMem[nRspFifo] );
    uRegisterValue = soc_mem_addr_get( nUnit, aTmuUpdaterRspFifoMem[nRspFifo], 0, copyno, 0, &at);
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_SBUS_START_ADDRESSr( nUnit, uRegisterValue );

    schan_msg_clear(&msg);
    msg.readcmd.header.opcode = FIFO_POP_CMD_MSG;
    msg.readcmd.header.dstblk = SOC_BLOCK2SCH( nUnit, copyno );
    msg.readcmd.header.datalen = uDataBeats * sizeof(uint32);
    /* Set 1st schan ctrl word as opcode */
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_OPCODEr( nUnit, msg.dwords[0] );

    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, 
                                   (uint32) (soc_cm_l2p(nUnit, (void *)TMU_FIFO_RESP_MGR(nUnit,fifoid).dma_buffer)) );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_HOSTMEM_START_ADDRESSr( nUnit, 
                                   (uint32) (soc_cm_l2p(nUnit, (void *)TMU_FIFO_RESP_MGR(nUnit,fifoid).dma_buffer)) );

    if ( nRspFifo )
      READ_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, &uRegisterValue );
    else
      READ_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, &uRegisterValue );
    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, BEAT_COUNTf, uDataBeats );
    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, HOST_NUM_ENTRIES_SELf, uEntryNum );
    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, ENDIANESSf, 1 );
    if ( nRspFifo ) {
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, uRegisterValue );
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uRspRingSize-64) );
    } else {
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, uRegisterValue );
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_HOSTMEM_THRESHOLDr( nUnit, (uRspRingSize-64) );
    }

    soc_reg_field_set( nUnit, CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr, &uRegisterValue, ENABLEf, 1 );
    if ( nRspFifo )
      WRITE_CMIC_CMC0_FIFO_CH3_RD_DMA_CFGr( nUnit, uRegisterValue );
    else
      WRITE_CMIC_CMC0_FIFO_CH2_RD_DMA_CFGr( nUnit, uRegisterValue );
  }

  return SOC_E_NONE;
    
}
#endif
/*
 *
 * Function:
 *     tmu_resp_dma_mgr_uninit
 * Purpose:
 *     Cleanup
 */
int tmu_resp_dma_mgr_uninit(int unit, int fifoid)
{
    int status;

    status = tmu_cmd_dma_mgr_destroy(unit, fifoid);
    if (SOC_FAILURE(status)) {
        return status;
    }

    status = tmu_resp_dma_mgr_destroy(unit, fifoid);
    if (SOC_FAILURE(status)) {
        return status;
    }

    return status;
}
    
/*
 *
 * Function:
 *     tmu_resp_dma_mgr_init
 * Purpose:
 *     Initialize response TMU DMA manager
 */
int tmu_resp_dma_mgr_init(int unit, int fifoid)
{
    char buf[64];
    uint32 regval;
    soc_mem_t resp_fifo_mem[] = {TMB_UPDATER_RSP_FIFO0m, TMB_UPDATER_RSP_FIFO1m};
    uint8 channels[] = {2,3};
    int status=SOC_E_NONE;

    if (fifoid < 0 || fifoid >= SOC_SBX_CALADAN3_TMU_CMD_NUM_FIFO) {
        return SOC_E_PARAM;
    }

    SOC_IF_TMU_UNINIT_RETURN(unit);
    TMU_DMA_LOCK(unit);

    /* Allocate DMA buffer */
    TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len = soc_mem_entry_words(unit, resp_fifo_mem[fifoid]) * 
                                                    SOC_SBX_TMU_RESP_DMA_MGR_RING_SIZE;

    sal_sprintf(&buf[0], "tmu-resp-mgr-fifo:%d", fifoid);
    TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer = (uint32 *) soc_cm_salloc(unit,
                                                                         TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len *
                                                                         sizeof(uint32),
                                                                         buf);

    if (TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer == NULL) {
        status =  SOC_E_MEMORY;
    }

    if (SOC_SUCCESS(status)) {

        sal_memset((void*)TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer,
                   0, TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len);

        TMU_FIFO_RESP_MGR(unit,fifoid).ring_pointer = &TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer[0];
        TMU_FIFO_RESP_MGR(unit,fifoid).thread_running = FALSE;
        TMU_FIFO_RESP_MGR(unit,fifoid).channel = channels[fifoid]; 
        TMU_FIFO_RESP_MGR(unit,fifoid).seq_num = 0;
        TMU_FIFO_RESP_MGR(unit,fifoid).unit = unit;
        TMU_FIFO_RESP_MGR(unit,fifoid).valid = TRUE;
        TMU_FIFO_RESP_MGR(unit,fifoid).fifoid = fifoid;

        /* initialize Trailer response */
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE(READ_TMB_UPDATER_RSP_FIFO_INACTIVITY_THRESHr(unit, fifoid, &regval), status);
        if (SOC_FAILURE(status)) {
            TMU_DMA_UNLOCK(unit);
            return status;
        }

        soc_reg_field_set(unit, TMB_UPDATER_RSP_FIFO_INACTIVITY_THRESHr, &regval, 
                          THRESHOLDf, SOC_SBX_CALADAN3_TMU_DEF_RESP_TRAILER_THRESHOLD);
        status = WRITE_TMB_UPDATER_RSP_FIFO_INACTIVITY_THRESHr(unit, fifoid, regval);


        if (SOC_FAILURE(status)) {
            TMU_DMA_UNLOCK(unit);
            return status;
        }

        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE_NO_ERR(READ_CMIC_CMC0_CONFIGr(unit, &regval), status);
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE_NO_ERR(soc_dma_reg_field_set(unit, CMIC_CMC0_CONFIGr, &regval, 
                                                                   ENABLE_SBUSDMA_CH2_FLOW_CONTROLf, 1), 
                                                 status);
        SOC_CALADAN3_ALLOW_WARMBOOT_WRITE_NO_ERR(WRITE_CMIC_CMC0_CONFIGr(unit, regval), status);
        

        /* start rx fifo dma */

        if (SOC_WARM_BOOT(unit)) {
            soc_mem_fifo_dma_stop(unit, channels[fifoid]);
        }

#if 1
        status = soc_mem_fifo_dma_start(unit, channels[fifoid],
                                        resp_fifo_mem[fifoid],
                                        MEM_BLOCK_ANY,
                                        TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer_len /
                                        soc_mem_entry_words(unit, resp_fifo_mem[fifoid]),
                                        (uint32 *)TMU_FIFO_RESP_MGR(unit,fifoid).dma_buffer);


#else
        status = tmu_caladan3_resp_fifo_dma_mgr_start(unit, fifoid);
#endif
    }

    if (SOC_FAILURE(status)) {
        tmu_cmd_dma_mgr_destroy(unit, fifoid);
    }

    TMU_DMA_UNLOCK(unit);
    return status;
}


#endif
