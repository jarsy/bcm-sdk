/*
 * $Id: stat.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <bcm/stat.h>
#include <sal/types.h>
#include <soc/mem.h>

/*
 * Function:
 *      bcm_stat_group_mode_attr_selector_t_init
 * Description:
 *      Initialize an attribute selector of Stat Flex Group Mode
 * Parameters:
 *      attr_selector : (INOUT) Attribute Selector for Stat Flex Group Mode
 * Returns:
 *      NONE
 *     
 */
void bcm_stat_group_mode_attr_selector_t_init(
     bcm_stat_group_mode_attr_selector_t *attr_selector)
{
     if (attr_selector != NULL) {
         sal_memset(attr_selector, 0,
                    sizeof(bcm_stat_group_mode_attr_selector_t));
     }
     return;
}

/*
 * Function:
 *      bcm_stat_value_t_init
 * Description:
 *      Initialize a data structure bcm_stat_value_t
 *      void bcm_stat_value_t_init(bcm_stat_value_t *stat_value)
 * Parameters:
 *      stat_value : (INOUT) Pointer to bcm_stat_value_t structure to be initialized
 * Returns:
 *      NONE
 *
 */
void bcm_stat_value_t_init(
     bcm_stat_value_t *stat_value)
{
     if (stat_value != NULL) {
         sal_memset(stat_value, 0,
                    sizeof(bcm_stat_value_t));
     }
     return;
}

void bcm_stat_group_mode_id_config_t_init(
    bcm_stat_group_mode_id_config_t *stat_config)
{
     if (stat_config != NULL) {
         sal_memset(stat_config, 0,
                    sizeof(bcm_stat_group_mode_id_config_t));
     }
     return;
}

void bcm_stat_group_mode_hint_t_init(
    bcm_stat_group_mode_hint_t *stat_hint)
{
     if (stat_hint != NULL) {
         sal_memset(stat_hint, 0,
                    sizeof(bcm_stat_group_mode_hint_t));
     }
     return;
}

/*
 * Function:
 * bcm_stat_counter_input_data_t_init
 * Purpose:
 * Initialize a stat_counter_input_data object struct.
 * Parameters:
 * stat_input_data - pointer to stat_counter_input_data object struct.
 * Returns:
 * NONE
 */
void bcm_stat_counter_input_data_t_init(
    bcm_stat_counter_input_data_t *stat_input_data)
{
    if (stat_input_data != NULL) {
        sal_memset(stat_input_data, 0,
                  sizeof (bcm_stat_counter_input_data_t));
        stat_input_data->counter_source_gport = -1;
        stat_input_data->counter_source_id = -1;
    }
    return;
}

/*
 * Function:
 * bcm_stat_flex_pool_stat_info_t_init
 * Purpose:
 * Initialize a stat_flex_pool_stat_info object struct.
 * Parameters:
 * stat_flex_pool_stat_info - pointer to stat_flex_pool_stat_info object struct.
 * Returns:
 * NONE
 */
void bcm_stat_flex_pool_stat_info_t_init(
        bcm_stat_flex_pool_stat_info_t *stat_flex_pool_stat_info)
{
    if (stat_flex_pool_stat_info != NULL) {
        sal_memset(stat_flex_pool_stat_info, 0,
                    sizeof (bcm_stat_flex_pool_stat_info_t));
    }
    return;
}
