
/*******************************************************************

* Copyright (C) 2002. Redux Communications Ltd. All rights reserved.

Module Name:
	flow

File Name:
    trnsmit_and_QM_cfg.h

File Description:

$Revision: 1.1.2.1 $  - Visual SourceSafe revision number

History:
    Date            Name            Comment
    ----            ----            -------
    5/2/2001		Uri
*******************************************************************/
#ifndef TRNS_AND_QM_CFG_H
#define TRNS_AND_QM_CFG_H

#include "ag_common.h"
#include "drivers/qmd_types.h"
#include "drivers/bmd_types.h"

#ifdef TRNS_AND_QM_CFG_C
#define AG_EXTERN
#else
#define AG_EXTERN extern
#endif


/*****************************************
		QM CONFIGUTRATION FUNCTION
 *****************************************/

#ifdef __cplusplus
extern "C"
{
#endif

AG_EXTERN AgResult trans_init(void);


AG_EXTERN AgResult ag_qm_cfg_set_block (AgQmdBlockCfg * p_block_cfg);

AG_EXTERN AgResult ag_qm_set_std_queue (AG_U8 n_queue_number, AG_BOOL b_cpu_queue,
										AG_U16 n_max_entries);

AG_EXTERN AgResult ag_qm_get_std_queue (AG_U8 n_queue_number, AG_BOOL *p_cpu_queue,
										AG_U16 *p_max_entries);

AG_EXTERN AgResult ag_qm_cfg_set_queue (AgQmdQueueCfg * p_queue_cfg);

AG_EXTERN AgResult ag_qm_cfg_get_queue (AgQmdQueueCfg * p_queue_cfg);

AG_EXTERN AgResult ag_qm_cfg_get_block (AgQmdBlockCfg * p_block_cfg);

AG_EXTERN AgResult ag_qm_cfg_get_global(AgQmdGlobalCfg * p_global_cfg);

AG_EXTERN AgResult ag_qm_info_get_queue_length (AG_U8  n_queue_num, AG_U16  *p_queue_length);

AG_EXTERN AgResult ag_qm_info_red_get_aver_queue_len (AG_U8  n_queue_num, AG_U16*p_RED_aver_queue_length);

AG_EXTERN AgResult ag_qm_info_get_queue_status (AG_U8  n_queue_num, AG_U8 *p_queue_state);

/*****************************************
			TRANSMIT FUNCTION
*****************************************/

AG_EXTERN AgResult ag_trns_send_frm(AgBmdBundPtr p_bund_ptr);

#ifdef __cplusplus
} /*end of extern "C"*/
#endif


#undef AG_EXTERN
#endif  /* TRANSMITTER_H */