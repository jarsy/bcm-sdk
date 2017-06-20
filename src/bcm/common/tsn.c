/*
 * $Id: tsn.c,v 1.17 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <soc/mem.h>
#include <bcm/tsn.h>


void bcm_tsn_sr_port_config_t_init(bcm_tsn_sr_port_config_t *port_config)
{
    if (NULL != port_config) {
        sal_memset(port_config, 0, sizeof(*port_config));
    }
}

void bcm_tsn_pri_map_entry_t_init(bcm_tsn_pri_map_entry_t *entry)
{
    if (NULL != entry) {
        sal_memset(entry, 0, sizeof(*entry));
    }
}

void bcm_tsn_pri_map_config_t_init(bcm_tsn_pri_map_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_sr_tx_flow_config_t_init(bcm_tsn_sr_tx_flow_config_t *flow_config)
{
    if (NULL != flow_config) {
        sal_memset(flow_config, 0, sizeof(*flow_config));
    }
}

void bcm_tsn_sr_rx_flow_config_t_init(bcm_tsn_sr_rx_flow_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_sr_flowset_status_t_init(bcm_tsn_sr_flowset_status_t *status)
{
    if (NULL != status) {
        sal_memset(status, 0, sizeof(*status));
    }
}

void bcm_tsn_sr_tx_flow_status_t_init(bcm_tsn_sr_tx_flow_status_t *status)
{
    if (NULL != status) {
        sal_memset(status, 0, sizeof(*status));
    }
}

void bcm_tsn_sr_rx_flow_status_t_init(bcm_tsn_sr_rx_flow_status_t *status)
{
    if (NULL != status) {
        sal_memset(status, 0, sizeof(*status));
    }
}

void bcm_tsn_flow_config_t_init(bcm_tsn_flow_config_t *flow_config)
{
    if (NULL != flow_config) {
        sal_memset(flow_config, 0, sizeof(*flow_config));
    }
}

void bcm_tsn_flowset_status_t_init(bcm_tsn_flowset_status_t *status)
{
    if (NULL != status) {
        sal_memset(status, 0, sizeof(*status));
    }
}

void bcm_tsn_sr_auto_learn_group_config_t_init(bcm_tsn_sr_auto_learn_group_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_sr_auto_learn_config_t_init(bcm_tsn_sr_auto_learn_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_stat_threshold_config_t_init(bcm_tsn_stat_threshold_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_event_source_t_init(bcm_tsn_event_source_t *source)
{
    if (NULL != source) {
        sal_memset(source, 0, sizeof(*source));
    }
}

void bcm_tsn_mtu_config_t_init(bcm_tsn_mtu_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_ingress_mtu_config_t_init(bcm_tsn_ingress_mtu_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

void bcm_tsn_ingress_stu_config_t_init(bcm_tsn_ingress_stu_config_t *config)
{
    if (NULL != config) {
        sal_memset(config, 0, sizeof(*config));
    }
}

