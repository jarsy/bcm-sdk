/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.
 * BROADCOM SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 *
 * $Id: ci.h,v 1.4.6.2 Broadcom SDK $
 *
 * TMU CI defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TMU_CI_H_
#define _SBX_CALADN3_TMU_CI_H_

#define SOC_SBX_CALADAN3_TMU_CI_INSTANCE_NUM (16)

extern int soc_sbx_caladan3_tmu_ci_init(int unit);

extern int soc_sbx_caladan3_tmu_ci_memory_pattern_init(int unit);

extern int soc_sbx_caladan3_tmu_is_ci_memory_init_done(int unit);

int _soc_sbx_caladan3_ddr_init(int unit);

#endif /* _SBX_CALADN3_TMU_CI_H_ */
