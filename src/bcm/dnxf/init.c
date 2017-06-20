/*
 * $Id: init.c,v 1.14 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *   This module calls the initialization routine of each BCM module.
 *
 * Initial System Configuration
 *
 *   Each module should initialize itself without reference to other BCM
 *   library modules to avoid a chicken-and-the-egg problem.  To do
 *   this, each module should initialize its respective internal state
 *   and hardware tables to match the Initial System Configuration.  The
 *   Initial System Configuration is:
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_INIT
#include <shared/bsl.h>

#include <sal/types.h>
#include <sal/core/time.h>
#include <sal/core/boot.h>

#include <soc/drv.h>
#include <soc/l2x.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/counter.h>

#include <soc/dnxf/cmn/dnxf_drv.h>

#include <bcm/debug.h>
#include <bcm/error.h>
#include <bcm/link.h>
#include <bcm/init.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/common/family.h>
#include <bcm_int/common/debug.h>

#include <bcm_int/dnxf_dispatch.h>
#include <bcm_int/dnxf/stat.h>
#include <bcm_int/dnxf/stack.h>
#include <bcm_int/dnxf/port.h>
#include <bcm_int/dnxf/link.h>
#include <bcm_int/dnxf/fabric.h>
#include <bcm_int/dnxf/rx.h>

int _bcm_dnxf_detach(int unit);

static int _bcm_dnxf_init_finished_ok[BCM_MAX_NUM_UNITS] = { 0 };

#define BCM_DNXF_DRV_INIT_LOG(_unit_, _msg_str_)\
                LOG_INFO(BSL_LS_BCM_INIT,\
                         (BSL_META_U(_unit_,\
                                     "    + %d: %s\n"), _unit_ , _msg_str_))

int 
bcm_dnxf_init_check(
    int unit)
{
    int rc;
    BCMDNX_INIT_FUNC_DEFS;

    rc = (_bcm_dnxf_init_finished_ok[unit] == 1 ? BCM_E_NONE : BCM_E_UNIT);
    if (rc == BCM_E_UNIT)
    {
        /*bcm init not finished - return BCM_E_UNIT withot error printing*/
        BCM_ERR_EXIT_NO_MSG(rc);
    } else {
        BCMDNX_IF_ERR_EXIT(rc);
    }
exit:
    BCMDNX_FUNC_RETURN; 
}
 
int
bcm_dnxf_init_selective(
    int unit,
    uint32 module_number)
{
    int rc;
    BCMDNX_INIT_FUNC_DEFS;

    switch (module_number) {
         case BCM_MODULE_PORT     :
                   rc = bcm_dnxf_port_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
         case BCM_MODULE_LINKSCAN  :
                   rc = bcm_dnxf_linkscan_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
         case BCM_MODULE_STAT     :
                   rc = bcm_dnxf_stat_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
         case BCM_MODULE_STACK    :
                   rc = bcm_dnxf_stk_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
         case BCM_MODULE_MULTICAST:
                   rc = bcm_dnxf_multicast_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
         case BCM_MODULE_FABRIC:
                   rc = bcm_dnxf_fabric_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
        case BCM_MODULE_RX:
                   rc = bcm_dnxf_rx_init(unit);
                   BCMDNX_IF_ERR_EXIT(rc);
                   break;
        default:
                   BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("module %d not supported"), module_number));
    }

exit:
    BCMDNX_FUNC_RETURN; 
}

int
bcm_dnxf_init(int unit)
{
    bcm_switch_event_control_t ec; 
    int rc = SOC_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    DNXF_UNIT_LOCK_TAKE(unit);

    rc = bcm_chip_family_set(unit, BCM_FAMILY_DNXF);
    BCMDNX_IF_ERR_EXIT(rc);

    BCM_DNXF_DRV_INIT_LOG(unit, "Port");
    rc = bcm_dnxf_init_selective(unit, BCM_MODULE_PORT);
    BCMDNX_IF_ERR_EXIT(rc);

    BCM_DNXF_DRV_INIT_LOG(unit, "Linkscan");
    rc = bcm_dnxf_init_selective(unit, BCM_MODULE_LINKSCAN);
    BCMDNX_IF_ERR_EXIT(rc); 

    BCM_DNXF_DRV_INIT_LOG(unit, "Stat");
    rc = bcm_dnxf_init_selective(unit, BCM_MODULE_STAT);
    BCMDNX_IF_ERR_EXIT(rc);

    BCM_DNXF_DRV_INIT_LOG(unit, "Multicast");
    rc = bcm_dnxf_init_selective(unit, BCM_MODULE_MULTICAST);
    BCMDNX_IF_ERR_EXIT(rc);

    BCM_DNXF_DRV_INIT_LOG(unit, "Fabric");
    rc = bcm_dnxf_init_selective(unit, BCM_MODULE_FABRIC);
    BCMDNX_IF_ERR_EXIT(rc);

    BCM_DNXF_DRV_INIT_LOG(unit, "Stack");
    rc = bcm_dnxf_init_selective(unit, BCM_MODULE_STACK);
    BCMDNX_IF_ERR_EXIT(rc);

    if (SOC_IS_RAMON(unit))
    {
        BCM_DNXF_DRV_INIT_LOG(unit, "Rx");
        rc = bcm_dnxf_init_selective(unit, BCM_MODULE_RX);
        BCMDNX_IF_ERR_EXIT(rc);
    }
    
    if (!SOC_IS_RAMON(unit)) 
    {                
        if (!SOC_WARM_BOOT(unit) && !SAL_BOOT_NO_INTERRUPTS) {
            /*clear interrupts*/
            ec.event_id=BCM_SWITCH_EVENT_CONTROL_ALL;
            ec.action=bcmSwitchEventClear;
            rc = bcm_dnxf_switch_event_control_set(unit,BCM_SWITCH_EVENT_DEVICE_INTERRUPT,ec,1);
            BCMDNX_IF_ERR_EXIT(rc);
        }
    }

    /*ALDWP configuration*/
    if (!SOC_WARM_BOOT(unit))
    {
        rc = MBCM_DNXF_DRIVER_CALL_NO_ARGS(unit, mbcm_dnxf_fabric_links_aldwp_init);
        BCMDNX_IF_ERR_EXIT(rc);
    }

    _bcm_dnxf_init_finished_ok[unit] = 1;

exit:
    DNXF_UNIT_LOCK_RELEASE(unit);
    if(BCMDNX_FUNC_ERROR) {
       _bcm_dnxf_detach(unit);
    }
    BCMDNX_FUNC_RETURN; 
}


int
bcm_dnxf_info_get(int unit,
                 bcm_info_t *info)
{
    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_NULL_CHECK(info);

    info->vendor = SOC_PCI_VENDOR(unit);
    info->device = SOC_PCI_DEVICE(unit);
    info->revision = SOC_PCI_REVISION(unit);
    info->capability = BCM_CAPA_LOCAL | BCM_CAPA_FABRIC;

exit:
    BCMDNX_FUNC_RETURN; 
}


/*
 * Function:
 *         _bcm_dnxf_modules_deinit
 * Purpose:
 *         De-initialize bcm modules
 * Parameters:
 *     unit - (IN) BCM device number.
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_dnxf_modules_deinit(int unit)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    rc = _bcm_dnxf_stk_deinit(unit);
    BCMDNX_IF_ERR_CONT(rc);

    rc = _bcm_dnxf_fabric_deinit(unit);
    BCMDNX_IF_ERR_CONT(rc);

    rc = bcm_dnxf_multicast_detach(unit);
    BCMDNX_IF_ERR_CONT(rc);

    rc = _bcm_dnxf_stat_deinit(unit);
    BCMDNX_IF_ERR_CONT(rc);

    rc = bcm_dnxf_linkscan_detach(unit); 
    BCMDNX_IF_ERR_CONT(rc);

    rc = _bcm_dnxf_port_deinit(unit);
    BCMDNX_IF_ERR_CONT(rc);

    if (SOC_IS_RAMON(unit) && (SOC_DNXF_CONTROL(unit)->rx_thread_fifo_dma_semaphore != NULL))
    {
        rc = _bcm_dnxf_rx_deinit(unit);
        BCMDNX_IF_ERR_CONT(rc);
    }

    BCMDNX_FUNC_RETURN; 
}


int
_bcm_dnxf_attach(int unit, char *subtype)
{
    int         dunit, rc ;
    BCMDNX_INIT_FUNC_DEFS;

    COMPILER_REFERENCE(subtype);

    BCM_CONTROL(unit)->capability |= BCM_CAPA_LOCAL;

    dunit = BCM_CONTROL(unit)->unit;

    if (SOC_UNIT_VALID(dunit)) {
        BCM_CONTROL(unit)->chip_vendor = SOC_PCI_VENDOR(dunit);
        BCM_CONTROL(unit)->chip_device = SOC_PCI_DEVICE(dunit);
        BCM_CONTROL(unit)->chip_revision = SOC_PCI_REVISION(dunit);
        BCM_CONTROL(unit)->capability |= BCM_CAPA_FABRIC;
    }

    rc = bcm_dnxf_init(unit);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      _bcm_dnxf_threads_shutdown
 * Purpose:
 *      Terminate all the spawned threads for specific unit.
 * Parameters:
 *      unit - unit being detached
 * Returns:
 *      BCM_E_XXX
 */
int
_bcm_dnxf_threads_shutdown(int unit)
{
    int rc;     /* Operation return status. */
    BCMDNX_INIT_FUNC_DEFS;

    rc = bcm_dnxf_linkscan_enable_set(unit, 0);
    BCMDNX_IF_ERR_CONT(rc);

    rc = soc_counter_stop(unit);
    BCMDNX_IF_ERR_CONT(rc);

    BCMDNX_FUNC_RETURN; 
}

int
_bcm_dnxf_detach(int unit)
{
    int rc;                    /* Operation return status. */
    BCMDNX_INIT_FUNC_DEFS;

    /* Shut down all the spawned threads. */
    rc = _bcm_dnxf_threads_shutdown(unit);
    BCMDNX_IF_ERR_CONT(rc);
    
    /* 
     *  Don't move up, holding lock or disabling hw operations
     *  might prevent theads clean exit.
     */
    DNXF_UNIT_LOCK_TAKE(unit);

    rc = _bcm_dnxf_modules_deinit(unit);
    BCMDNX_IF_ERR_CONT(rc);

    _bcm_dnxf_init_finished_ok[unit] = 0;

exit:
    DNXF_UNIT_LOCK_RELEASE(unit);
    BCMDNX_FUNC_RETURN; 
}


int
_bcm_dnxf_match(int unit, char *subtype_a, char *subtype_b)
{
    BCMDNX_INIT_FUNC_DEFS;
    COMPILER_REFERENCE(unit);
    BCMDNX_IF_ERR_EXIT(sal_strcmp(subtype_a, subtype_b));
exit:
    BCMDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

