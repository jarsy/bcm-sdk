/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 $Id$
 */

#ifndef BCM_IMACSEC_H
#define BCM_IMACSEC_H
#include<soc/error.h>
#include <soc/phy/phyctrl.h>
#include "bcm_plp_base_t_secy_api.h"
#include "bcm_plp_base_t_sec_common.h"

#define IMACSEC_PLP_BASE_T_SEC_ACCESS_GET(pc,pa,dev_id) { \
                        pa = (bcm_plp_base_t_sec_access_t *)(((char *)pc->driver_data + pc->size) - sizeof(bcm_plp_base_t_sec_access_t));\
                        if(pa == NULL){\
	                       return SOC_E_INTERNAL;\
                        }\
                        pa->phy_info.phy_addr= pa->macsec_dev_addr;\
                        pa->macsec_side = dev_id;\
                        }
int imacsec_plp_base_t_secy_init(int unit, soc_port_t port,
                               unsigned int macsec_side,
                               bcm_plp_base_t_sec_mutex_t *mutex,
                               bcm_plp_base_t_secy_settings_t *setting_p);
int imacsec_plp_base_t_secy_vport_add(int unit, soc_port_t port, unsigned int macsec_side,
                                    bcm_plp_base_t_secy_vport_handle_t *vport_handle_p);
int imacsec_plp_base_t_secy_vport_remove(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_vport_handle_t vport_handle);

int imacsec_plp_base_t_secy_uninit(int unit, soc_port_t port, unsigned int macsec_side);

int imacsec_plp_base_t_secy_sa_add(int unit, soc_port_t port, unsigned int macsec_side,
                                 const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                 bcm_plp_base_t_secy_sa_handle_t *sa_handle_p,
                                 const bcm_plp_base_t_secy_sa_t *sa_p);
int imacsec_plp_base_t_secy_sa_update(int unit, soc_port_t port, unsigned int macsec_side,
                                    const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                    const bcm_plp_base_t_secy_sa_t *sa_p);
int imacsec_plp_base_t_secy_sa_read(int unit, soc_port_t port, unsigned int macsec_side,
                                  const bcm_plp_base_t_secy_sa_handle_t sa_handle,
                                  const unsigned int word_offset,
                                  const unsigned int word_count,
                                  unsigned int * transform_p);

int imacsec_plp_base_t_secy_sa_remove(int unit, soc_port_t port, unsigned int macsec_side,
                                    const bcm_plp_base_t_secy_sa_handle_t sa_handle
                                    );
int imacsec_plp_base_t_secy_bypass_set(int unit, soc_port_t port, unsigned int macsec_side,
                                    const unsigned char f_static_bypass,
                                    const unsigned char f_bypass_no_class);
int imacsec_plp_base_t_secy_bypass_get(int unit, soc_port_t port, unsigned int macsec_side,
                                    unsigned char *f_static_bypass,
                                    unsigned char *f_bypass_no_class);
int imacsec_plp_base_t_secy_rule_add(int unit, soc_port_t port, unsigned int macsec_side,
                                   const bcm_plp_base_t_secy_vport_handle_t vport_handle,
                                   bcm_plp_base_t_secy_rule_handle_t *rule_handle_p,
                                   const bcm_plp_base_t_secy_rule_t *rule_p);

int imacsec_plp_base_t_secy_rule_remove(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle);

int imacsec_plp_base_t_secy_rule_update(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                      const bcm_plp_base_t_secy_rule_t *rule_p);

int imacsec_plp_base_t_secy_rule_enable(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                      const unsigned char fsync);

int imacsec_plp_base_t_secy_rule_disable(int unit, soc_port_t port, unsigned int macsec_side,
                                      const bcm_plp_base_t_secy_rule_handle_t rule_handle,
                                      const unsigned char fsync);
int imacsec_plp_base_t_secy_rule_enable_disable(int unit, soc_port_t port, unsigned int macsec_side,
                                              const bcm_plp_base_t_secy_rule_handle_t rule_handle_disable,
                                              const bcm_plp_base_t_secy_rule_handle_t rule_handle_enable,
                                              const unsigned char enable_all,
                                              const unsigned char disable_all,
                                              const unsigned char fsync);
int imacsec_plp_base_t_version_get(int unit,soc_port_t port, bcm_plp_base_t_version_t* version_info);

int imacsec_plp_addr_read(int unit,            /* attached switch unit */
                          unsigned int port,         /* port number */
                          unsigned int reg_addr,     /* register address */
                          unsigned int *value);    /* read register value */

int imacsec_plp_addr_write(int unit,           /* attached switch unit */
                           unsigned int port,        /* port number */
                           unsigned int reg_addr,    /* register address  */
                           unsigned int value);    /* register value to write */
#endif /* BCM_IMACSEC_H */
