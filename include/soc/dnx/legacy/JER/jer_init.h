/*
 * $Id: jer2_jer_init.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_DNX_JER2_JER_INIT_H
#define _SOC_DNX_JER2_JER_INIT_H

/* 
 * Defines
 */


#define JER2_JER_INIT_FDT_IPT_MESH_MC_TABLE_MAX_INX       2048
#define JER2_JER_IQM_PACKING_MODE_TABLE_MAX_INX           128

#define JER2_JERICHO_INIT_FMC_64_REP_512K_DBUFF_MODE      0

/* 
 * Functions
 */
/* init tbls */
int soc_jer2_jer_tbls_init(int unit);

/* deinit tbls */
int soc_jer2_jer_tbls_deinit(int unit);

/* Init functions */
int soc_jer2_jer_init_sequence_phase1(int unit);

/* Access init functions */
int soc_jer2_jer_init_brdc_blk_id_set(int unit);

/* Init PP Jericho and above modules */
soc_error_t soc_jer2_jer_pp_mgmt_functional_init(int unit);

/* Init Jericho plls*/
int jer2_jer_pll_init(int unit);

/* Init Jericho Synce*/
int jer2_jer_synce_init(int unit);

/* Init Jericho blocks configuration */
int soc_jer2_jer_init_blocks_conf(int unit);

/* set indirect Write mask to allow dynamic access and initialize tables*/
int soc_jer2_jer_write_masks_set (int unit);

/* Init IPT valid contexts*/
uint32
soc_jer2_jer_ipt_contexts_init(
  DNX_SAND_IN int     unit
);

/*********************************************************************
*     Perform an mbist check on JER2_JER and ARDON.
*     Stop on errors or not depending on skip_errors.
*********************************************************************/
int soc_bist_all_jer2_jer(const int unit, const int skip_errors);
int soc_bist_all_jer2_qax(const int unit, const int skip_errors);
int jer2_qax_mbist_fix_arm_core(const int unit, const int skip_errors);

#endif /* !_SOC_DNX_JER2_JER_INIT_H  */

