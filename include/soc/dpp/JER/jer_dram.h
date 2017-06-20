/*
 * $Id: jer_ocb_dram_buffers.h Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef _JER_DRAM_H
#define _JER_DRAM_H

/*
 * Functions
 */

int soc_jer_dram_info_verify(int unit, soc_dpp_drc_combo28_info_t *drc_info);
int soc_jer_dram_init_drc_soft_init(int unit, soc_dpp_drc_combo28_info_t *drc_info, uint32  init);
int soc_jer_dram_recovery_init(int unit);

#endif /* _JER_DRAM_H */
