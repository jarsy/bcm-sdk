/*
 * $Id:$
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifndef __JER2_ARAD_PS_DB_INCLUDED__
/* { */
#define __JER2_ARAD_PS_DB_INCLUDED__

#include <soc/types.h>

int jer2_arad_ps_db_init(int unit);
int jer2_arad_ps_db_find_free_binding_ps(int unit, soc_port_t port, int out_port_priority, int is_init, int* base_q_pair);
int jer2_arad_ps_db_alloc_binding_ps_with_id(int unit, soc_port_t port, int out_port_priority, int base_q_pair);
int jer2_arad_ps_db_find_free_non_binding_ps(int unit, int core, int is_init, int* base_q_pair);
int jer2_arad_ps_db_find_free_non_binding_ps_with_id(int unit, soc_port_t port, int base_q_pair);
int jer2_arad_ps_db_release_binding_ps(int unit, soc_port_t port, int base_q_pair);
int jer2_arad_ps_db_release_non_binding_ps(int unit, int core, int out_port_priority, int base_q_pair);

/*__JER2_ARAD_PS_DB_INCLUDED__}*/
#endif
