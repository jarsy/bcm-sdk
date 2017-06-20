/* $Id$
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/types.h>
#include <soc/ll.h>

#include <bcm/range.h>
void 
bcm_range_config_t_init (
        bcm_range_config_t *range_config)
{
    if(range_config != NULL) {
        range_config->rid       = -1;
        range_config->rtype     = bcmRangeTypeCount;
        range_config->min       = 0;
        range_config->max       = 0;
        range_config->udf_id    = -1;
        range_config->width     = 0;
        range_config->offset    = 0;
        SOC_PBMP_CLEAR(range_config->ports);
    }
    return;
}

