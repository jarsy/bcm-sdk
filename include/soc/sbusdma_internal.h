/*
 * $Id: sbusdmaIinternal.h,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbusdma_internal.h
 * Purpose:     Maps out structures used for SBUSDMA operations and
 *              exports routines.
 */

#ifndef _SOC_SBUSDMA_INTERNAL_H
#define _SOC_SBUSDMA_INTERNAL_H

#include <soc/defs.h>
#include <soc/types.h>
#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/sync.h>
#include <sal/core/thread.h>
#include <soc/sbusdma.h>

#define CMIC_CMCx_SBUSDMA_CHSELECT_TIMEOUT    (1000)

#define SOC_CHECK_MULTI_CMC(unit) \
         (soc_feature(unit, soc_feature_cmicm_multi_dma_cmc) && \
                (SOC_PCI_CMCS_NUM(unit) > 1))

typedef struct soc_sbusdma_reg_param_s {
    soc_mem_t mem;
    int array_id_start;
    int array_id_end;
    int index_begin;
    int index_end;
    int copyno;
    int flags;
    uint8 mem_clear;
    void *buffer;
}soc_sbusdma_reg_param_t;

/* Register SBUS DMA function pointers */
typedef struct soc_sbusdma_reg_drv_s {
    int (*soc_mem_sbusdma_read)(int unit, soc_sbusdma_reg_param_t  *param);
    int (*soc_mem_sbusdma_write)(int unit, soc_sbusdma_reg_param_t *param);
    int (*soc_mem_sbusdma_clear)(int unit, soc_sbusdma_reg_param_t *param);
}soc_sbusdma_reg_drv_t;

/* Descriptor SBUS DMA function pointers */

typedef struct soc_sbusdma_desc_drv_s {
int (*soc_sbusdma_desc_create)(int unit, soc_sbusdma_desc_ctrl_t *ctrl,
                        soc_sbusdma_desc_cfg_t *cfg, void **desc);
int (*soc_sbusdma_program)(int unit, int *cmc, _soc_sbusdma_state_t *swd,
                           int *ch);
int (*soc_sbusdma_desc_detach)(int unit);
int (*soc_sbusdma_wait)(int unit, int cmc, int ch, int interval,
                       uint8 intrEnb);
}soc_sbusdma_desc_drv_t;

typedef struct soc_sbusdma_cmic_ch_s {
    int (*soc_sbusdma_ch_try_get)(int unit, int cmc, int ch);
    int (*soc_sbusdma_ch_put)(int unit, int cmc, int ch);
} soc_sbusdma_cmic_ch_t;


/*******************************************
* @function cmicm_sbusdma_ch_init
* purpose API to Initialize sbusdma channel seletion mechanism
*
* @param unit [in] unit
* @param timeout [in] int
* @param cmic_ch [out] pointer type soc_sbusdma_cmic_ch_t
*
* @returns SOC_E_NONE
* @returns SOC_E_XXX
*
* @end
********************************************/
extern int
cmicm_sbusdma_ch_init(int unit, int timeout,
                           soc_sbusdma_cmic_ch_t *cmic_ch);



/*******************************************
* @function cmicm_sbusdma_ch_put
* purpose put back the freed channel on cmc
*
* @param cmc [in] cmc number 0-2
* @param ch [in] channel number 0-2
*
* @returns SOC_E_PARAM
* @returns SOC_E_NONE
*
* @end
********************************************/
extern int
cmicm_sbusdma_ch_put(int unit, int cmc, int ch);

/*******************************************
* @function cmicm_sbusdma_ch_get
* purpose get idle channel on cmc
*
* @param unit [in] unit #
* @param cmc [out] cmc number 0-2
* @param ch [out] channel number 0-2
*
* @returns SOC_E_BUSY
* @returns SOC_E_NONE
*
* @end
********************************************/
extern int
cmicm_sbusdma_ch_get(int unit, int *cmc, int *ch);

/*******************************************
* @function cmicm_sbusdma_ch_try_get
* purpose get idle channel on cmc
*
* @param unit [in] unit #
* @param cmc [in] cmc number 0-2
* @param ch [in] channel number 0-2
*
* @returns SOC_E_BUSY
* @returns SOC_E_NONE
*
* @end
********************************************/
extern int
cmicm_sbusdma_ch_try_get(int unit, int cmc, int ch);

/*******************************************
* @function cmicm_sbusdma_reg_init
* purpose API to Initialize cmicm register DMA
*
* @param unit [in] unit
* @param drv [out] soc_sbusdma_reg_drv_t pointer
*
* @returns SOC_E_NONE
* @returns SOC_E_XXX
*
* @end
********************************************/
extern int cmicm_sbusdma_reg_init(int unit, soc_sbusdma_reg_drv_t *drv);

/*******************************************
* @function cmicm_sbusdma_desc_init
* purpose API to Initialize cmicm Descriptor DMA
*
* @param unit [in] unit
* @param drv [out] soc_sbusdma_desc_drv_t pointer
*
* @returns SOC_E_NONE
* @returns SOC_E_XXX
*
* @end
********************************************/
extern int cmicm_sbusdma_desc_init(int unit, soc_sbusdma_desc_drv_t *drv);

/*******************************************
* @function cmicm_sbusdma_curr_op_details
* purpose API to print cmic operation registers
*
* @param unit [in] unit
* @param cmc [in] cmc
* @param ch [in] channel#
*
* @returns void
*
* @end
********************************************/
extern void
cmicm_sbusdma_curr_op_details(int unit, int cmc, int ch);

/*******************************************
* @function cmicm_sbusdma_error_details
* purpose API to print cmic error registers
*
* @param unit [in] unit
* @param rval [in] return value
*
* @returns void
*
* @end
********************************************/
extern void
cmicm_sbusdma_error_details(int unit, uint32 rval);

#endif /* !_SOC_SBUSDMA_INTERNAL_H */
