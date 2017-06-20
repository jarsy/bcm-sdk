/** \file access.c
 * access related SOC functions and Access related CBs for init mechanism.
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_SOC_INIT

/*
 * INCLUDE FILES:
 * {
 */

#include <shared/shrextend/shrextend_debug.h>
#include <soc/dnx/access.h>
#include <soc/schanmsg.h>
#include <soc/drv.h>

/*
 * }
 */
shr_error_e
dnx_access_mem_mutex_init(
    int unit)
{
    soc_mem_t mem;
    soc_control_t *soc;
    SHR_FUNC_INIT_VARS(unit);

    soc = SOC_CONTROL(unit);

    /*
     * Initialize memory index_maxes. Chip specific overrides follow. 
     */
    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        if (SOC_MEM_IS_VALID(unit, mem))
        {
            /*
             * should only create mutexes for valid memories. 
             */
            if ((soc->memState[mem].lock = sal_mutex_create(SOC_MEM_NAME(unit, mem))) == NULL)
            {
                SHR_IF_ERR_EXIT_WITH_LOG(_SHR_E_MEMORY, "Failed to allocate memory lock\n%s%s%s", EMPTY, EMPTY, EMPTY);
            }

            /*
             * Set cache copy pointers to NULL 
             */
            sal_memset(soc->memState[mem].cache, 0, sizeof(soc->memState[mem].cache));
        }
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_access_mem_mutex_deinit(
    int unit)
{
    soc_mem_t mem;
    soc_control_t *soc;
    SHR_FUNC_INIT_VARS(unit);

    soc = SOC_CONTROL(unit);

    /*
     * Destroy memory mutex 
     */
    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        if (soc->memState[mem].lock != NULL)
        {
            sal_mutex_destroy(soc->memState[mem].lock);
            soc->memState[mem].lock = NULL;
        }
    }

    SHR_FUNC_EXIT;
}
