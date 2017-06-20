/*
 * $Id: TkMsg.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkMsg.h
 * Purpose: 
 *
 */

#ifndef _SOC_EA_TkMsg_H
#define _SOC_EA_TkMsg_H

#if defined(__cplusplus)
extern "C"  {
#endif

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/ea/tk371x/TkOsSync.h>
#include <soc/ea/tk371x/TkOsAlloc.h>


#define WAIT_FOREVER        (-1)
#define NO_WAIT             0
#define WAIT_MAX_CYCLES     1024

#define MAX_MSGS            3
#define MAX_MSG_LEN         1518
#define MIN_MSG_LEN         60

#define SAL_MSG_CNT         5
#define SAL_MSG_LEN         2048

#define TK_MAX_RX_TX_DATA_LENGTH    1600


typedef struct sal_msg_buffer_s {
    unsigned int        len;
    char                buff[SAL_MSG_LEN];
} sal_msg_buffer_t;

typedef struct sal_msg_desc_s {
    sal_sem_t           sem;    /*multiple valued semphore*/
    int                 id;
    unsigned int        max_cnt;
    unsigned int        msg_cnt;
    sal_msg_buffer_t  * msg;
} sal_msg_desc_t;


typedef struct {
    long    mtype;
    uint8   buff[TK_MAX_RX_TX_DATA_LENGTH];
} TkMsgBuff;



sal_msg_desc_t * sal_msg_create (char *name, unsigned int count);

void    sal_msg_destory (sal_msg_desc_t * msg);
int     sal_msg_snd (sal_msg_desc_t * msg, char *buff, int len, int ms);
int     sal_msg_rcv (sal_msg_desc_t * msg, char *buff, int len, int ms);
int     sal_msg_clear (sal_msg_desc_t * msg);

#if defined(__cplusplus)
}
#endif

#endif /* _SOC_EA_TkMsg_H */
