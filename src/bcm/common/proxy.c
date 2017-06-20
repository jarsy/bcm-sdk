/*
 * $Id: proxy.c,v 1.1.2.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * Proxy initializers
 */
#if defined(INCLUDE_L3)

#include <sal/core/libc.h>
 
#include <bcm/proxy.h>

/* 
 * Function:
 *      bcm_proxy_server_t_init
 * Purpose:
 *      Initialize the proxy server struct
 * Parameters: 
 *      proxy_server - Pointer to the struct to be init'ed
 */
void
bcm_proxy_server_t_init(bcm_proxy_server_t *proxy_server)
{   
    if (proxy_server != NULL) {
        sal_memset(proxy_server, 0, sizeof(*proxy_server));
    }
    return;
}

/* 
 * Function:
 *      bcm_proxy_egress_t_init
 * Purpose:
 *      Initialize proxy egress object  struct.
 * Parameters: 
 *      egr - pointer to the proxy egress object struct.
 * Returns:
 *      NONE
 */
void
bcm_proxy_egress_t_init(bcm_proxy_egress_t *proxy_egress)
{
    if (NULL != proxy_egress) {
        sal_memset(proxy_egress, 0, sizeof (*proxy_egress));
    }
    return;
}

#else
int _bcm_proxy_not_empty;
#endif  /* INCLUDE_L3 */

