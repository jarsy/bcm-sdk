/* 
 * $Id: mgr.h,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mgr.h
 * Purpose:     Board manager shared declarations
 */

#ifndef   _SRC_BOARD_MANAGER_MGR_H_
#define   _SRC_BOARD_MANAGER_MGR_H_

#include <sal/core/sync.h>

extern sal_mutex_t board_lock;

#define BOARD_INIT { if (board_lock == NULL) { return BCM_E_INIT; } }
#define BOARD_INIT_NULL { if (board_lock == NULL) { return NULL; } }
#define BOARD_LOCK sal_mutex_take(board_lock, sal_mutex_FOREVER)
#define BOARD_UNLOCK sal_mutex_give(board_lock)

#endif /* _SRC_BOARD_MANAGER_MGR_H_ */
