/*
 * $Id$ 
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __ND_RESULTS_H__
#define __ND_RESULTS_H__

#include "ag_common.h"
#include "ag_results.h"

#define AG_E_ND_MODULE_CREATED    (AG_E_FAIL | (AG_RES_ND_BASE + 1))
#define AG_E_ND_MODULE_REMOVED    (AG_E_FAIL | (AG_RES_ND_BASE + 2))
#define AG_E_ND_DEVICE_ISOPEN     (AG_E_FAIL | (AG_RES_ND_BASE + 3))
#define AG_E_ND_DEVICE_ISCLOSE    (AG_E_FAIL | (AG_RES_ND_BASE + 4))
#define AG_E_ND_HANDLE            (AG_E_FAIL | (AG_RES_ND_BASE + 5))

#define AG_E_ND_ALLOC             (AG_E_FAIL | (AG_RES_ND_BASE + 6))
#define AG_E_ND_ARG               (AG_E_FAIL | (AG_RES_ND_BASE + 7))
#define AG_E_ND_FTIP              (AG_E_FAIL | (AG_RES_ND_BASE + 8))
#define AG_E_ND_ENABLE            (AG_E_FAIL | (AG_RES_ND_BASE + 9))
#define AG_E_ND_UNSUPPORTED       (AG_E_FAIL | (AG_RES_ND_BASE + 10))
#define AG_E_ND_NOT_READY         (AG_E_FAIL | (AG_RES_ND_BASE + 11))
#define AG_E_ND_IN_PROGRESS       (AG_E_FAIL | (AG_RES_ND_BASE + 12))
#define AG_E_ND_CES_PORT          (AG_E_FAIL | (AG_RES_ND_BASE + 13))
#define AG_E_ND_STATE             (AG_E_FAIL | (AG_RES_ND_BASE + 14))
#define AG_E_ND_ASSIGNED          (AG_E_FAIL | (AG_RES_ND_BASE + 15))

#endif /* __ND_RESULTS_H__ */

