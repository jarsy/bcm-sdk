/*
 * $Id: ipfix.c,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:       ipfix.c
 * Purpose:    IPFIX common APIs
 */

#include <soc/drv.h>
#include <bcm/ipfix.h>

/*
 * Function:
 *     bcm_ipfix_config_t_init
 * Description:
 *     Initialize an IPFIX config structure
 * Parameters:
 *     config    pointer to IPFIX config structure
 * Return: none
 */
void
bcm_ipfix_config_t_init(bcm_ipfix_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(bcm_ipfix_config_t));
    }
    return;
}

/*
 * Function:
 *     bcm_ipfix_rate_t_init
 * Description:
 *     Initialize an IPFIX rate structure
 * Parameters:
 *     rate      pointer to IPFIX rate structure
 * Return: none
 */
void
bcm_ipfix_rate_t_init(bcm_ipfix_rate_t *rate)
{
    if (NULL != rate) {
        sal_memset(rate, 0, sizeof(bcm_ipfix_rate_t));
    }
    return;
}

/*
 * Function:
 *     bcm_ipfix_mirror_config_t_init
 * Description:
 *     Initialize an IPFIX mirror config structure
 * Parameters:
 *     config    pointer to IPFIX mirror config structure
 * Return: none
 */
void
bcm_ipfix_mirror_config_t_init(bcm_ipfix_mirror_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(bcm_ipfix_mirror_config_t));
    }
    return;
}
