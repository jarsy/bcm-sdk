/*
 * $Id: qax_init.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _SOC_DPP_QAX_INIT_H
#define _SOC_DPP_QAX_INIT_H

/* 
 * Defines
 */


/*
 * Typedefs
 */

typedef enum soc_fabric_dtq_mode_type_e
{
    SOC_FABRIC_DTQ_MODE_SINGLE_QUEUE = 0x0,
    SOC_FABRIC_DTQ_MODE_UC_MC = 0x1,
    SOC_FABRIC_DTQ_MODE_UC_HMC_LMC = 0x2
} soc_fabric_dtq_mode_type_t ;


/* 
 * Functions
 */

int soc_qax_init_blocks_init_conf(int unit);
int soc_qax_enable_dynamic_memories(int unit);

/* QAX register/memory settings as overrides, to be later moved to the correct functions and removed */
int soc_qax_init_overrides(int unit);

/* QAX define share overrides for emulation */
int soc_qax_init_dpp_defs_overrides(int unit);

/* Init TXQ contexts */
soc_error_t soc_qax_tdq_contexts_init(int unit);

uint32 soc_qax_pdq_dtq_contexts_init(SOC_SAND_IN int unit);

#endif /* !_SOC_DPP_QAX_INIT_H  */

