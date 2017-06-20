/*
 * $Id: uk-proxy-kcom.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Provides a kcom interface using User/Kernel proxy services
 */

#ifndef __UK_PROXY_KCOM_H__
#define __UK_PROXY_KCOM_H__

extern void *
uk_proxy_kcom_open(char *name);

extern int
uk_proxy_kcom_close(void *handle);

extern int
uk_proxy_kcom_msg_send(void *handle, void *msg,
                       unsigned int len, unsigned int bufsz);

extern int
uk_proxy_kcom_msg_recv(void *handle, void *msg,
                       unsigned int bufsz);

#endif /* __UK_PROXY_KCOM_H__ */
