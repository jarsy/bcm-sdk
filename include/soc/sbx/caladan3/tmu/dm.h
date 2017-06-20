/*
 * $Id: dm.h,v 1.4.6.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * TMU DM defines
 *
 *-----------------------------------------------------------------------------*/
#ifndef _SBX_CALADN3_TMU_DM_H_
#define _SBX_CALADN3_TMU_DM_H_

/* Ecc boundaries at 128bits */
#define SOC_SBX_CALADAN3_TMU_DM_ECC_BITS (9)

extern int soc_sbx_caladan3_tmu_dm_table_alloc(int unit, int size, 
                                               int entry_size_bits,
                                               int *tableid);

/* allocate dm table with id */
extern int soc_sbx_caladan3_tmu_dm_table_alloc_id(int unit, int size, 
                                                  int entry_size_bits,
                                                  int *tableid);

extern int soc_sbx_caladan3_tmu_dm_table_free(int unit, int tableid);

extern int soc_sbx_caladan3_tmu_dm_set(int unit, int tableid, int entry_index, 
                                       uint32 *entry, int entry_len);

extern int soc_sbx_caladan3_tmu_dm_get(int unit, int tableid, int entry_index, 
                                       uint32 *entry, int entry_len);

extern int soc_sbx_caladan3_tmu_dm_multi_set(int unit, int tableid, int *entry_index, 
					     uint32 *entry, int entry_len, int num_entries);

extern int soc_sbx_caladan3_tmu_dm_driver_init(int unit);

#endif /* _SBX_CALADN3_TMU_DM_H_ */
