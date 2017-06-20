/*
 * $Id: kconfig.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

extern char *kconfig_get(const char *name);
extern int kconfig_get_next(char **name, char **value);
extern int kconfig_set(char *name, char *value);
