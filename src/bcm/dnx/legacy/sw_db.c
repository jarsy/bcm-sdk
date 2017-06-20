/*
 * $Id: sw_db.c,v 1.62 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_INIT
#include <shared/bsl.h>
#include <bcm_int/common/debug.h>
#include <bcm/module.h>
#include <bcm_int/dnx/legacy/sw_db.h>
#include <bcm_int/dnx/legacy/utils.h>
#include <bcm_int/common/link.h>
#include <bcm/error.h>
#include <soc/error.h>
#include <soc/types.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/SAND/Utils/sand_multi_set.h>
#include <bcm_int/dnx/legacy/error.h>
#include <shared/swstate/access/sw_state_access.h>
#include <shared/swstate/sw_state_sync_db.h>

#ifdef BCM_JER2_ARAD_SUPPORT
#endif /* BCM_JER2_ARAD_SUPPORT */

#include <soc/scache.h>

/* this flag is lowered by scache.c commit function after syncing the SW state*/
#define MARK_SCACHE_DIRTY(unit) \
    SOC_CONTROL_LOCK(unit);\
    SOC_CONTROL(unit)->scache_dirty = 1;\
    SOC_CONTROL_UNLOCK(unit);



#define BCM_SW_DB_CELL_ID_MAX                        ((1L<<11) - 1)




/* global variables - AKA the SW state */

static  int _cell_id_curr[BCM_MAX_NUM_UNITS];

/************************************************************************/
/*                       set/get functions                              */
/************************************************************************/

 
int
_bcm_sw_db_cell_id_curr_get(int unit, int *cell_id)
{
    BCMDNX_INIT_FUNC_DEFS;
    *cell_id = _cell_id_curr[unit];
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}

int
_bcm_sw_db_cell_id_curr_set(int unit, int *cell_id)
{
    BCMDNX_INIT_FUNC_DEFS;
    if(*cell_id > BCM_SW_DB_CELL_ID_MAX)
    {
        _cell_id_curr[unit] = 0;
    }
    else
    {
        sal_memcpy(&(_cell_id_curr[unit]), &cell_id, sizeof(int));  
    }
    MARK_SCACHE_DIRTY(unit);
    BCM_EXIT;
exit:
    BCMDNX_FUNC_RETURN;
}


/************************************************************************/
/*               init/sync functions for the whole module               */
/************************************************************************/


int _bcm_sw_db_init(int unit)
{

    BCMDNX_INIT_FUNC_DEFS;


    BCMDNX_FUNC_RETURN;
}




int _bcm_sw_db_deinit(int unit)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_FUNC_RETURN;
}


