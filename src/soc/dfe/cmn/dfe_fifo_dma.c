/*
 * $Id: dfe_port.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC FIFO DMA
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/defs.h>
#include <soc/error.h>
#include <soc/drv.h>

#ifdef BCM_DFE_SUPPORT

#include <soc/dcmn/error.h>

#include <soc/dfe/cmn/dfe_fifo_dma.h>
#include <soc/dfe/cmn/dfe_defs.h>
#include <soc/dfe/cmn/dfe_drv.h>

/*FIFO DMA data base*/
/*should not restored by warm boot*/
soc_dfe_fifo_dma_t _soc_dfe_fifo_dma[SOC_MAX_NUM_DEVICES][SOC_DFE_IMP_DEFS_MAX(FIFO_DMA_NOF_CHANNELS)];

soc_error_t
soc_dfe_fifo_dma_init(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /*Initilize settings*/
    sal_memset(_soc_dfe_fifo_dma[unit], 0, sizeof(soc_dfe_fifo_dma_t) *  SOC_DFE_IMP_DEFS_MAX(FIFO_DMA_NOF_CHANNELS));

    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_dfe_fifo_dma_deinit(int unit)
{
    int channel;
    SOCDNX_INIT_FUNC_DEFS;

    
    for (channel = 0; channel  < SOC_DFE_IMP_DEFS_MAX(FIFO_DMA_NOF_CHANNELS); channel++)
    {
        if (_soc_dfe_fifo_dma[unit][channel].enable)
        {
            SOCDNX_IF_ERR_EXIT(soc_dfe_fifo_dma_channel_deinit(unit, channel));
            _soc_dfe_fifo_dma[unit][channel].enable = 0;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_dfe_fifo_dma_channel_init(int unit, int channel)
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fifo_dma_channel_init, (unit, channel, &_soc_dfe_fifo_dma[unit][channel]));
    SOCDNX_IF_ERR_EXIT(rv);
    _soc_dfe_fifo_dma[unit][channel].enable = 1;

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_dfe_fifo_dma_channel_deinit(int unit, int channel)
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fifo_dma_channel_deinit, (unit, channel, &_soc_dfe_fifo_dma[unit][channel]));
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dfe_fifo_dma_channel_entry_get
 * Purpose:
 *      FIFO DMA pull <nof_entries> entries
 * Parameters:
 *      unit                            - (IN)  Unit number.
 *      channel                         - (IN)  fifo dma channel
 *      max_entry_size                  - (IN)  size of data_in
 *      nof_entries                     - (IN)  num of enteries to copy to entry
 *      entry                           - (OUT) data out
 * Returns:
 *      SOC_E_XXX
 */
soc_error_t
soc_dfe_fifo_dma_channel_entry_get(int unit, int channel, uint32  max_entry_size, uint32 nof_entries, uint8 *entry)
{
    int rv;
    uint32 buffer_offset;
    SOCDNX_INIT_FUNC_DEFS;

    /*If no available entries in the hostmem - try to load from the fifo*/
    if (_soc_dfe_fifo_dma[unit][channel].nof_unread_entries < nof_entries)
    {
        _soc_dfe_fifo_dma[unit][channel].read_entries = nof_entries;
        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fifo_dma_channel_read_entries, (unit, channel, &_soc_dfe_fifo_dma[unit][channel]));
        SOCDNX_IF_ERR_EXIT(rv);
        /*wait for entry load*/
        sal_usleep(100);
    }

    /*copy entry and update data base*/
    if (_soc_dfe_fifo_dma[unit][channel].nof_unread_entries > 0)
    {
        if (_soc_dfe_fifo_dma[unit][channel].config.entry_size * nof_entries < max_entry_size)
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Required bigger entry\n")));
        }

        buffer_offset = _soc_dfe_fifo_dma[unit][channel].current_entry_id * _soc_dfe_fifo_dma[unit][channel].config.entry_size;
        sal_memcpy(entry, &_soc_dfe_fifo_dma[unit][channel].buffer[buffer_offset], sizeof(uint8) * max_entry_size * nof_entries);
        _soc_dfe_fifo_dma[unit][channel].current_entry_id = (_soc_dfe_fifo_dma[unit][channel].current_entry_id + nof_entries) % _soc_dfe_fifo_dma[unit][channel].config.max_entries; /*Cyclic buffer*/

        _soc_dfe_fifo_dma[unit][channel].nof_unread_entries -=  nof_entries;
        _soc_dfe_fifo_dma[unit][channel].read_entries = 0;

    } else {
        SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_EMPTY);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_dfe_fifo_dma_channel_clear(int unit, int channel)
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_fifo_dma_channel_clear, (unit, channel, &_soc_dfe_fifo_dma[unit][channel]));
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

#endif /* BCM_DFE_SUPPORT */

#undef _ERR_MSG_MODULE_NAME

