/*
 * $Id: acl_field.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        acl_field.h
 * Purpose:     
 *     Header file for field specific functionality to ACL.
 */

#ifndef _ACL_FIELD_H_
#define _ACL_FIELD_H_

#if defined(INCLUDE_ACL)

#include <appl/acl/acl.h>
#include "acl_util.h"

extern int _acl_field_init(void);
extern int _acl_field_detach(void);
extern int _acl_field_merge(_acl_control_t *control);
extern int _acl_field_install(void);
extern int _acl_field_uninstall(void);
extern int _acl_field_rule_remove(bcma_acl_rule_id_t rid);

#ifdef BROADCOM_DEBUG
extern int _acl_field_show(void);
#endif /* BROADCOM_DEBUG */
#endif /* INCLUDE_ACL */

#endif /* _ACL_FIELD_H_ */
