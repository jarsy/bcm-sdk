/*
 * $Id: jer2_qax_init.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_DNX_JER2_QAX_INIT_H
#define _SOC_DNX_JER2_QAX_INIT_H

/* 
 * Defines
 */

/* 
 * Functions
 */

int soc_jer2_qax_init_blocks_init_conf(int unit);

/* JER2_QAX register/memory settings as overrides, to be later moved to the correct functions and removed */
int soc_jer2_qax_init_overrides(int unit);

/* JER2_QAX define share overrides for emulation */
int soc_jer2_qax_init_dnx_defs_overrides(int unit);

/* Init TXQ contexts */
soc_error_t soc_jer2_qax_tdq_contexts_init(int unit);

uint32 soc_jer2_qax_pdq_dtq_contexts_init(DNX_SAND_IN int unit);

#endif /* !_SOC_DNX_JER2_QAX_INIT_H  */

