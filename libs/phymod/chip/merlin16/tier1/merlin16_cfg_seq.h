/*----------------------------------------------------------------------
 * $Id: merlin16_cfg_seq.h,v 1.1.2.2 Broadcom SDK $ 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Broadcom Corporation
 * Proprietary and Confidential information
 * All rights reserved
 * This source file is the property of Broadcom Corporation, and
 * may not be copied or distributed in any isomorphic form without the
 * prior written consent of Broadcom Corporation.
 *---------------------------------------------------------------------
 * File       : merlin16_cfg_seq.h
 * Description: c functions implementing Tier1s for TEMod Serdes Driver
 *---------------------------------------------------------------------*/
/*
 *  $Copyright: (c) 2014 Broadcom Corporation All Rights Reserved.$
 *  $Id$
*/


#ifndef MERLIN16_CFG_SEQ_H
#define MERLIN16_CFG_SEQ_H

#include "common/srds_api_err_code.h"
#include "merlin16_enum.h"

err_code_t merlin16_uc_active_set(const phymod_access_t *pa, uint32_t enable);              /* set microcontroller active or not  */
err_code_t merlin16_uc_active_get(const phymod_access_t *pa, uint32_t *enable);             /* get microcontroller active or not  */
err_code_t merlin16_pmd_ln_h_rstb_pkill_override( const phymod_access_t *pa, uint16_t val);
err_code_t merlin16_core_soft_reset_release(const phymod_access_t *pa, uint32_t enable);    /* release the pmd core soft reset */
err_code_t merlin16_lane_soft_reset_release(const phymod_access_t *pa, uint32_t enable);    /* release the pmd lane soft reset */
err_code_t merlin16_pmd_tx_disable_pin_dis_set(const phymod_access_t *pa, uint32_t enable);
err_code_t merlin16_pmd_loopback_set(const phymod_access_t *pa, uint32_t enable);
err_code_t merlin16_pmd_loopback_get(const phymod_access_t *pa, uint32_t *enable);
err_code_t merlin16_rmt_lpbk_get(const phymod_access_t *pa, uint32_t *lpbk);
err_code_t merlin16_osr_mode_get(const phymod_access_t *pa, int *osr_mode);
int merlin16_osr_mode_to_enum(int osr_mode, phymod_osr_mode_t* osr_mode_en);
err_code_t merlin16_pmd_cl72_enable_get(const phymod_access_t* sa__, uint32_t* cl72_en);
err_code_t merlin16_polarity_set(const phymod_access_t *sa__, uint32_t tx_polarity, uint32_t rx_polarity);
err_code_t merlin16_polarity_get(const phymod_access_t *sa__, uint32_t* tx_polarity, uint32_t* rx_polarity);

#endif /* MERLIN16_CFG_SEQ_H */
