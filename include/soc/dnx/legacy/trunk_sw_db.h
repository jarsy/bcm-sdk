/*
 * $Id: trunk_sw_db.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */



#ifndef _BCM_INT_DNX_TRUNK_SW_DB_H_
#define _BCM_INT_DNX_TRUNK_SW_DB_H_

#include <soc/dnx/legacy/PPD/ppd_api_lag.h>

int soc_dnx_trunk_sw_db_get_nof_replications(int unit, int tid, const SOC_PPC_LAG_MEMBER *lag_member, int* nof_replications, int* last_replicated_member_index);
int soc_dnx_trunk_sw_db_get_nof_enabled_members(int unit, int tid, int* nof_enabled_members);
int soc_dnx_trunk_sw_db_get_first_replication_index(int unit, int tid, const SOC_PPC_LAG_MEMBER *lag_member, int* first_replication_index);
int soc_dnx_trunk_sw_db_add(int unit, int tid, const SOC_PPC_LAG_MEMBER *lag_member);
int soc_dnx_trunk_sw_db_remove_all(int unit, int tid);
int soc_dnx_trunk_sw_db_set(int unit, int tid, const SOC_PPC_LAG_INFO *lag_info);
int soc_dnx_trunk_sw_db_remove(int unit, int tid, SOC_PPC_LAG_MEMBER *lag_member);
int soc_dnx_trunk_sw_db_set_trunk_attributes(int unit, int tid, int psc, int is_stateful);
int soc_dnx_trunk_sw_db_get(int unit, int tid, SOC_PPC_LAG_INFO *lag_info);

#endif /* _BCM_INT_DNX_TRUNK_SW_DB_H_ */
