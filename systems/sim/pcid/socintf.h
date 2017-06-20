/*
 * $Id: socintf.h,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        socintf.h
 * Purpose:     
 */

#ifndef   _PCID_SOCINTF_H_
#define   _PCID_SOCINTF_H_

#include "pcid.h"

extern int pcid_setup_socket(int port);
extern int pcid_wait_for_cnxn(int sockfd);
extern void pcid_close_cnxn(int sockfd);
extern int pcid_process_request(pcid_info_t *pcid_info, int sockfd,
                    struct timeval *tmout);




#endif /* _PCID_SOCINTF_H_ */
