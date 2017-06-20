/*
 * $Id: ramon_fabric_cell.h,v 1.6 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * RAMON FIFO DMA H
 */


#ifndef _SOC_RAMON_FIFO_DMA_H_
#define _SOC_RAMON_FIFO_DMA_H_

/**********************************************************/
/*                  Includes                              */
/**********************************************************/

#include <soc/error.h>
#include <soc/dnxf/cmn/dnxf_fifo_dma.h>


/**********************************************************/
/*                  Defines                               */
/**********************************************************/

typedef enum soc_ramon_fifo_dma_channel_e
{
    soc_ramon_fifo_dma_channel_fabric_cell_dcl_0 = 0,
    soc_ramon_fifo_dma_channel_fabric_cell_dcl_1 = 1,
    soc_ramon_fifo_dma_channel_fabric_cell_dcl_2 = 2, 
    soc_ramon_fifo_dma_channel_fabric_cell_dcl_3 = 3
} soc_ramon_fifo_dma_channel_t;

/**********************************************************/
/*                  Functions                             */
/**********************************************************/

soc_error_t soc_ramon_fifo_dma_channel_init(int unit, int channel, soc_dnxf_fifo_dma_t *fifo_dma_info);
soc_error_t soc_ramon_fifo_dma_channel_deinit(int unit, int channel, soc_dnxf_fifo_dma_t *fifo_dma_info);
soc_error_t soc_ramon_fifo_dma_channel_clear(int unit, int channel, soc_dnxf_fifo_dma_t *fifo_dma_info);
soc_error_t soc_ramon_fifo_dma_channel_read_entries(int unit, int channel, soc_dnxf_fifo_dma_t *fifo_dma_info);
soc_error_t soc_ramon_fifo_dma_fabric_cell_validate(int unit);


#endif /*!_SOC_RAMON_FABRIC_CELL_H_*/

