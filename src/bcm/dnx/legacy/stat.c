/*
 * $Id: stat.c,v 1.30 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC_DNX STAT
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_BCM_STAT
#include <shared/bsl.h>
#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm_int/common/debug.h>
#include <bcm/debug.h>
#include <bcm/stat.h>
#include <soc/defs.h>
#include <bcm_int/dnx/legacy/stat.h>
#include <bcm_int/dnx/legacy/switch.h>

#include <bcm_int/dnx_dispatch.h>
#include <bcm_int/dnx/legacy/gport_mgmt.h>

#include <soc/dnx/legacy/ARAD/arad_stat.h>


#include <soc/dnxc/legacy/dnxc_wb.h>

#include <soc/drv.h>

#include <soc/error.h>

#include <soc/dnx/legacy/port_sw_db.h>

#include <soc/dnx/legacy/mbcm.h>

#define SOC_IS_NIF_CONTROLLED_COUNTER_TYPE(type)             \
                ((type == snmpEtherStatsDropEvents) ||       \
                (type == snmpBcmRxFecCorrectable)  ||       \
                (type == snmpBcmRxFecUncorrectable))

/*
 * Function:
 *      bcm_dnx_stat_init
 * Purpose:
 *      Initialize the BCM statistics module
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      BCM_E_NONE        Success  
 *      BCM_E_INTERNAL    Device access failure  
 */
int 
bcm_dnx_stat_init(
    int unit)
{
    int rc = BCM_E_NONE;
    int interval;
    uint32 flags, counters_database_max_port;
    pbmp_t pbmp;
    soc_port_t          p;
    soc_control_t       *soc = SOC_CONTROL(unit);
    bcm_pbmp_t ports_to_remove;
    bcm_port_t port_base;
    int phy_port;
    BCMDNX_INIT_FUNC_DEFS;

    rc = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_stat_fabric_init, (unit));
    BCMDNX_IF_ERR_EXIT(rc);
    if (!SOC_IS_ARDON(unit)) {
        rc = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_stat_nif_init, (unit));
        BCMDNX_IF_ERR_EXIT(rc);
    }

    if (soc_property_get_str(unit, spn_BCM_STAT_PBMP) == NULL) {
        SOC_PBMP_ASSIGN(pbmp, PBMP_PORT_ALL(unit));
    } else {
        pbmp = soc_property_get_pbmp(unit, spn_BCM_STAT_PBMP, 0);
    }

    SOC_PBMP_CLEAR(ports_to_remove);
    /* Remove ports, if:
     * 1. Ports is exceeding database range (i.e. above counters_database_max_port)
     * 2. Channelized ports which is not port base*/
    counters_database_max_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DNX_DEFS_GET(unit, first_fabric_link_id) + SOC_DNX_DEFS_GET(unit, nof_fabric_links) - 1;
    PBMP_ITER(pbmp, p) {
        if (p <= counters_database_max_port){
			/*get port base for channelized case */
			phy_port = SOC_INFO(unit).port_l2p_mapping[p];
			port_base = SOC_INFO(unit).port_p2l_mapping[phy_port];
			if (!SOC_JER2_ARAD_STAT_COUNTER_MODE_PACKETS_PER_CHANNEL(unit, p)  && p != port_base /*nchannelized and not port_Base*/) {
				SOC_PBMP_PORT_ADD(ports_to_remove, p);
			}
        }else{ /*Port is exceeding database range*/
        	SOC_PBMP_PORT_ADD(ports_to_remove, p);
        	LOG_ERROR(BSL_LS_SOC_COUNTER,
                         (BSL_META_U(unit,"ERROR: Port %d is exceeding counters database size (max port in database is %d).\n"
     	        							"Port was allocated but with no counter associated!\n"), p, counters_database_max_port));
        }
    }

    SOC_PBMP_REMOVE(pbmp, ports_to_remove);

    interval = (SAL_BOOT_BCMSIM) ?
               SOC_COUNTER_INTERVAL_SIM_DEFAULT : SOC_COUNTER_INTERVAL_DEFAULT;
    interval = soc_property_get(unit, spn_BCM_STAT_INTERVAL, interval);
    flags = soc_property_get(unit, spn_BCM_STAT_FLAGS, 0);

    rc = soc_counter_start(unit, flags, interval, pbmp);
    BCMDNX_IF_ERR_EXIT(rc);

    /*update counters bitmap in case counter thread is not start*/
    if (interval == 0) {
        SOC_PBMP_ASSIGN(soc->counter_pbmp, pbmp);
        PBMP_ITER(soc->counter_pbmp, p) {
            if ((SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), p))) {
                SOC_PBMP_PORT_REMOVE(soc->counter_pbmp, p);
            }
            if (IS_LB_PORT(unit, p)) {
                SOC_PBMP_PORT_REMOVE(soc->counter_pbmp, p);
            }
        }
    }

exit:
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_stat_stop
 * Purpose:
 *      Deinit the BCM statistics module
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      BCM_E_NONE        Success
 *      BCM_E_INTERNAL    Device access failure
 */
int
bcm_dnx_stat_stop(int unit)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;

    rc = soc_counter_stop(unit);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_stat_clear
 * Purpose:
 *      Clear the port-based statistics for the indicated device port
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      port  - (IN)     Zero-based device or logical port number 
 * Returns:
 *      BCM_E_NONE        Success  
 *      BCM_E_INTERNAL    Device access failure  
 */
int 
bcm_dnx_stat_clear(
    int unit, 
    bcm_port_t port)
{
    int rv;
    BCMDNX_INIT_FUNC_DEFS;

    if (!SOC_PORT_VALID(unit,port)) {
        return BCM_E_PORT;
    }

    if (BCM_PBMP_MEMBER(PBMP_CMIC(unit), port)) {
        /* Rudimentary CPU statistics -- needs soc_reg_twork */
        SOC_CONTROL(unit)->stat.dma_rbyt = 0;
        SOC_CONTROL(unit)->stat.dma_rpkt = 0;
        SOC_CONTROL(unit)->stat.dma_tbyt = 0;
        SOC_CONTROL(unit)->stat.dma_tpkt = 0;
        BCM_EXIT;
    }

    if ( _SOC_CONTROLLED_COUNTER_USE(unit, port)){
        rv = soc_controlled_counter_clear(unit, port);
        BCMDNX_IF_ERR_EXIT(rv);
    }

    if (!_SOC_CONTROLLED_COUNTER_USE(unit, port) && soc_feature(unit, soc_feature_generic_counters)) {
        bcm_port_t port_base;
        int phy_port;
        pbmp_t        pbm;

        /*get port base for channelized case */
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
        port_base = SOC_INFO(unit).port_p2l_mapping[phy_port];

        SOC_PBMP_CLEAR(pbm);
        SOC_PBMP_PORT_ADD(pbm, port_base);
        BCM_IF_ERROR_RETURN(soc_counter_set32_by_port(unit, pbm, 0));

    }


exit:
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_stat_sync
 * Purpose:
 *      Synchronize software counters with hardware
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      BCM_E_NONE        Success  
 *      BCM_E_INTERNAL    Device access failure  
 *      BCM_E_DISABLED    Unit's counter disabled  
 */
int 
bcm_dnx_stat_sync(
    int unit)
{
    int rc = BCM_E_NONE;
    BCMDNX_INIT_FUNC_DEFS;
     
    rc = soc_counter_sync(unit);
    BCMDNX_IF_ERR_EXIT(rc);

exit:
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_stat_get
 * Purpose:
 *      Get the specified statistics from the device
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      port  - (IN)     Zero-based device or logical port number 
 *      type  - (IN)     SNMP statistics type defined in bcm_stat_val_t
 *      value - (OUT)    Counter value 
 * Returns:
 *      BCM_E_NONE       Success.  
 *      BCM_E_PARAM      Illegal parameter.  
 *      BCM_E_BADID      Illegal port number.  
 *      BCM_E_INTERNAL   Device access failure.  
 *      BCM_E_UNAVAIL    Counter/variable is not implemented on this current chip.  
 */
int 
bcm_dnx_stat_get(
    int unit, 
    bcm_port_t port, 
    bcm_stat_val_t type, 
    uint64 *value)
{
    int rv;
    /*
     * This procedure is hard-coded to accept up to 2 counters for loading *value
     */
    uint32 cnt_type[3] = {0xffffff,0xffffff, 0xffffff} ;
    /*
     * Total number of counters actually used for loading *value
     */
    int num_cntrs, cntr_index ;
    /*
     * Accumulator used for summing 'num_cntrs' (eventually loaded into *value)
     */
    uint64 val ;
    _bcm_dnx_gport_info_t gport_info;

    BCMDNX_INIT_FUNC_DEFS;

    BCMDNX_NULL_CHECK(value);

    /*Check valid type*/
    if (type < 0 || type >= snmpValCount) {
	return BCM_E_PARAM;
    }

    /*-1 is not a valid port for this API.*/
    if (BCM_GPORT_INVALID == port) {
        return BCM_E_PORT;
    }

    rv = BCM_E_NONE ;
    num_cntrs = sizeof(cnt_type) / sizeof(cnt_type[0]) ;
    BCMDNX_IF_ERR_EXIT(_bcm_dnx_gport_to_phy_port(unit, port, _BCM_DNX_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY, &gport_info));
    port = gport_info.local_port;

    if (BCM_PBMP_MEMBER(PBMP_CMIC(unit), port)) {
        /* Rudimentary CPU statistics -- needs work */
        switch (type) {
        case snmpIfInOctets:
            COMPILER_64_SET(*value, 0, SOC_CONTROL(unit)->stat.dma_rbyt);
            break;
        case snmpIfInUcastPkts:
            COMPILER_64_SET(*value, 0, SOC_CONTROL(unit)->stat.dma_rpkt);
            break;
        case snmpIfOutOctets:
            COMPILER_64_SET(*value, 0, SOC_CONTROL(unit)->stat.dma_tbyt);
            break;
        case snmpIfOutUcastPkts:
            COMPILER_64_SET(*value, 0, SOC_CONTROL(unit)->stat.dma_tpkt);
            break;
        default:
            COMPILER_64_ZERO(*value);
            break;
        }
        BCM_EXIT;
    }
    
    if ( (SOC_IS_JERICHO(unit) && SOC_IS_NIF_CONTROLLED_COUNTER_TYPE(type)) || (_SOC_CONTROLLED_COUNTER_USE(unit, port)) ) {


        rv = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_mapping_stat_get, (unit,port ,cnt_type,&num_cntrs,type,3));
        BCMDNX_IF_ERR_EXIT(rv);
  
        COMPILER_64_ZERO(val) ;
        for (cntr_index = 0 ; cntr_index < num_cntrs ; cntr_index++)
        {
            int supported = 0, printable;

            rv = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_stat_controlled_counter_enable_get,(unit, port, cnt_type[cntr_index], &supported, &printable));
            if (!supported)
            {
                BCMDNX_ERR_EXIT_MSG(BCM_E_UNAVAIL, (_BSL_BCM_MSG("Counter index %d is not supported for port %d\n"), cnt_type[cntr_index], port));
            }
            if (COUNTER_IS_COLLECTED(
                SOC_CONTROL(unit)->controlled_counters[cnt_type[cntr_index]]) && SOC_CONTROL(unit)->counter_interval != 0) {
                /*counter is collected by counter thread*/
                rv = soc_counter_get(unit, port, cnt_type[cntr_index], 0, value);
            } else {
                /*counter isn't collected by counter thread */
                rv = SOC_CONTROL(unit)->
                    controlled_counters[cnt_type[cntr_index]].controlled_counter_f(unit,
                        SOC_CONTROL(unit)->controlled_counters[cnt_type[cntr_index]].counter_id, port, value);
            }
            if (rv != BCM_E_NONE) {
                /*
                 * If an error is encountered then stop collecting data and quit.
                 */
                break ;
            }
            COMPILER_64_ADD_64(val,*value) ;  /*  val += (*value) ; */
        }
        *value = val ;
    } else if (soc_feature(unit, soc_feature_generic_counters)) {
        bcm_port_t port_base;
        int phy_port;
        /*get port base for channelized case */
        phy_port = SOC_INFO(unit).port_l2p_mapping[port];
        port_base = SOC_INFO(unit).port_p2l_mapping[phy_port];

        if (!IS_QSGMII_PORT(unit, port)) {
            rv = _bcm_dnx_stat_generic_get(unit, port_base, type, value);
        } else {
            rv = _bcm_dnx_stat_ge_get(unit, port_base, type, value); 
        }
        _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
        return rv;
    } else {
        _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
        return BCM_E_UNAVAIL;
    }


    BCMDNX_IF_ERR_EXIT(rv);
    
exit:   
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_stat_multi_get
 * Purpose:
 *      Get the specified statistics from the device
 * Parameters:
 *      unit      - (IN)     Unit number.
 *      port      - (IN)     Zero-based device or logical port number 
 *      nstat     - (IN)     Number of elements in stat array
 *      stat_arr  - (IN)     Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT)    Collected 64-bit or 32-bit statistics values 
 * Returns:
 *  BCM_E_XXX
 */
int 
bcm_dnx_stat_multi_get(
    int unit, 
    bcm_port_t port, 
    int nstat,
    bcm_stat_val_t *stat_arr, 
    uint64 *value_arr)
{
    int rc = BCM_E_NONE;
    int i;
    BCMDNX_INIT_FUNC_DEFS;

    if (nstat <= 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Invalid nstat")));
    }

    BCMDNX_NULL_CHECK(stat_arr);
    BCMDNX_NULL_CHECK(value_arr);

    for(i=0 ; i<nstat ; i++ ) {
        rc = bcm_dnx_stat_get(unit, port, stat_arr[i], &(value_arr[i]));
        BCMDNX_IF_ERR_EXIT(rc);
    }

exit:   
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    BCMDNX_FUNC_RETURN;
}


/*
 * Function:
 *  bcm_dnx_stat_get32
 * Description:
 *  Get the specified statistic from the StrataSwitch
 * Parameters:
 *  unit - StrataSwitch PCI device unit number (driver internal).
 *  port - zero-based port number
 *  type - SNMP statistics type (see stat.h)
 *      val - (OUT) 32-bit counter value.
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_PARAM - Illegal parameter.
 *  BCM_E_BADID - Illegal port number.
 *  BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *  Same as bcm_dnx_stat_get, except converts result to 32-bit.
 */
int 
bcm_dnx_stat_get32(
    int unit, 
    bcm_port_t port, 
    bcm_stat_val_t type, 
    uint32 *value)
{
    int      rv;
    uint64    val64;

    BCMDNX_INIT_FUNC_DEFS;

    
    rv = bcm_dnx_stat_get(unit, port, type, &val64);

    COMPILER_64_TO_32_LO(*value, val64);

    BCMDNX_IF_ERR_EXIT(rv);
exit:
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    BCMDNX_FUNC_RETURN;
}

/*
 * Function:
 *      bcm_dnx_stat_multi_get32
 * Purpose:
 *      Get the specified statistics from the device.  The 64-bit
 *      values may be truncated to fit.
 * Parameters:
 *      unit - (IN) Unit number.
 *      port - (IN) 
 *      nstat - (IN) Number of elements in stat array
 *      stat_arr - (IN) Array of SNMP statistics types defined in bcm_stat_val_t
 *      value_arr - (OUT) Collected 32-bit statistics values
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int 
bcm_dnx_stat_multi_get32(
    int unit, 
    bcm_port_t port, 
    int nstat,
    bcm_stat_val_t *stat_arr, 
    uint32 *value_arr)
{
    int i;
    BCMDNX_INIT_FUNC_DEFS;
   
    if (nstat <= 0) {
        BCMDNX_ERR_EXIT_MSG(BCM_E_PARAM, (_BSL_BCM_MSG("Invalid nstat")));
    }

    BCMDNX_NULL_CHECK(stat_arr);
    BCMDNX_NULL_CHECK(value_arr);

    for(i=0 ; i<nstat ; i++ ) {
        BCMDNX_IF_ERR_EXIT
            (bcm_dnx_stat_get32(unit, port, stat_arr[i],
                                &(value_arr[i])));
    }

exit:
    _DNXC_BCM_WARM_BOOT_API_TEST_MODE_SKIP_WB_SEQUENCE(unit);
    BCMDNX_FUNC_RETURN;
}


int
_bcm_dnx_stat_counter_non_dma_extra_get(int unit,
                                    soc_counter_non_dma_id_t non_dma_id,
                                    soc_port_t port,
                                    uint64 *val)
{
    return BCM_E_NONE;
}


#undef _ERR_MSG_MODULE_NAME

