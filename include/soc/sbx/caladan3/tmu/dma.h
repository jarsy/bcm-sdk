/*
 * $Id: dma.h,v 1.5.134.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * Purpose: DMA command manager
 */

#ifndef _SBX_CALADN3_TMU_DMA_H_
#define _SBX_CALADN3_TMU_DMA_H_

/*
 *
 * Function:
 *     tmu_dma_tx
 * Purpose:
 *     TMU Transmit DMA
 */
extern int tmu_dma_tx(int unit, 
                      int fifoid, 
                      soc_sbx_caladan3_tmu_cmd_t *cmd,
                      soc_sbx_tmu_cmd_post_flag_e_t flag);

extern int tmu_dma_tx_share_cache_cmd(int unit, int *slave_units, int num_slaves, int fifoid, 
               soc_sbx_caladan3_tmu_cmd_t *cmd); 

extern int tmu_dma_tx_master_flush_cmd(int unit, int *slave_units, int num_slaves, int fifoid);


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
extern int tmu_dma_rx(int unit, int fifoid, soc_sbx_caladan3_tmu_cmd_t *response) ;

/*
 *
 * Function:
 *     tmu_dma_rx_advance_ring
 * Purpose:
 *     advance dma rx ring pointer
 */
extern int tmu_dma_rx_advance_ring(int unit, int fifoid, int num_bytes);

/*
 *
 * Function:
 *     tmu_dma_rx_copy_data
 * Purpose:
 *     copy received data from dma rx buffer
 */
extern int tmu_dma_rx_copy_data(int unit, 
                                int fifoid, 
                                uint32 *data_buffer,
                                int data_buffer_len, 
                                int resp_data_size_64b); 

/*
 *
 * Function:
 *     tmu_cmd_dma_mgr_destroy
 * Purpose:
 *     destroy TMU command DMA manager
 */
extern int tmu_cmd_dma_mgr_destroy(int unit, int fifoid);

/*
 *
 * Function:
 *     tmu_cmd_dma_mgr_init
 * Purpose:
 *     Initialize command TMU DMA manager
 */
extern int tmu_cmd_dma_mgr_init(int unit, int fifoid);
/*
 *
 * Function:
 *     tmu_cmd_dma_mgr_uninit
 * Purpose:
 *     Cleanup
 */
extern int tmu_cmd_dma_mgr_uninit(int unit, int fifoid);

/*
 *
 * Function:
 *     tmu_resp_dma_mgr_destroy
 * Purpose:
 *     destroy TMU response DMA manager
 */
extern int tmu_resp_dma_mgr_destroy(int unit, int fifoid);

/*
 *
 * Function:
 *     tmu_resp_dma_mgr_init
 * Purpose:
 *     Initialize response TMU DMA manager
 */
extern int tmu_resp_dma_mgr_init(int unit, int fifoid);
/*
 *
 * Function:
 *     tmu_resp_dma_mgr_uninit
 * Purpose:
 *     cleanup
 */
extern int tmu_resp_dma_mgr_uninit(int unit, int fifoid);

extern unsigned int tmu_dma_skip_tx, tmu_dma_skip_rx;
#endif
