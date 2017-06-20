/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer_stat_if.h
 */

#ifndef __JER_STAT_IF_INCLUDED__

#define __JER_STAT_IF_INCLUDED__

#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/ARAD/arad_api_stat_if.h>

uint32 soc_jer_stat_if_init_set(int unit, ARAD_INIT_STAT_IF *stat_if);
uint32 soc_jer_stat_if_report_info_set(int unit, ARAD_INIT_STAT_IF *stat_if);
soc_error_t soc_jer_stat_if_queue_range_set(int unit, int core, int is_scrubber, int min_queue, int max_queue);
soc_error_t soc_jer_stat_if_queue_range_get(int unit, int core, int is_scrubber, int* min_queue, int* max_queue);

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /*__JER_STAT_IF_INCLUDED__*/

