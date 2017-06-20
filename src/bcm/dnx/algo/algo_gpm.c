/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

/** \file algo_gpm.c
 *  $Id$ Gport Managment procedures for DNX.
 */
#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_BCMDNX_PORT
/*
 * Includes.
 * {
 */
#include <shared/shrextend/shrextend_debug.h>
#include <soc/dnx/dbal/dbal.h>
#include <bcm_int/dnx/algo/lif_mngr/lif_mapping.h>
#include <bcm_int/common/multicast.h>
#include <bcm/types.h>
#include <bcm_int/dnx/algo/algo_gpm.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <bcm_int/dnx/algo/algo_port.h>
/*
 * }
 */

/*
 * Defines.
 * {
 */
/*
* Code will be added later
*/
#define WAITING_FOR_IMPLEMENTATION 0

/*
 * }
 */

/*
 * See .h file
 * Note: PON is not supported.
 */
shr_error_e
dnx_algo_gpm_gport_phy_info_get(
    int unit,
    bcm_gport_t port,
    uint32 operations,
    dnx_algo_gpm_gport_phy_info_t * gport_info)
{

    bcm_gport_t gport;
    bcm_gport_t pp_port;
    int core_id;
    bcm_module_t modid = 0, my_modid = 0;
    bcm_port_t mod_port = 0;
    
#if WAITING_FOR_IMPLEMENTATION
    /*
     * {
     */
    uint16 master_sysport;
    int trunk_id, member_idx;
    bcm_gport_t inter_gport = 0;
    int queue_id;
    _bcm_dpp_gport_sw_resources gport_sw_resources;
    SOC_TMC_IPQ_QUARTET_MAP_INFO queue_map_info;
    /*
     * }
     */
#endif

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify the pointer 'gport_info' is not NULL and exit with error if it is.
     */
    SHR_NULL_CHECK(gport_info, _SHR_E_PARAM, "gport_info");

    /*
     * Init Struct.
     */
    
#if WAITING_FOR_IMPLEMENTATION
    /*
     * {
     */
    gport_info->sys_port = SOC_TMC_SYS_PHYS_PORT_INVALID;
    /*
     * }
     */
#else
    /*
     * {
     */
    gport_info->sys_port = 0;
    /*
     * }
     */
#endif
    gport_info->flags = 0;
    gport_info->local_port = SOC_MAX_NUM_PORTS;
    gport_info->internal_port_pp = SOC_MAX_NUM_PORTS;
    gport_info->internal_port_tm = SOC_MAX_NUM_PORTS;
    
    gport_info->internal_core_id = 0;

    /*
     * If GPORT is invalid EXIT
     */
    if (port == BCM_GPORT_INVALID)
    {
        SHR_EXIT();
    }

    /*
     * Check if gport is actually local port that is not encoded as GPORT
     */
    if (BCM_GPORT_IS_SET(port))
    {
        gport = port;
    }
    
#if WAITING_FOR_IMPLEMENTATION
    /*
     * {
     */
    else if (SOC_PORT_VALID(unit, port))
    {
        BCM_GPORT_LOCAL_SET(gport, port);
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PORT, "!!! Local port %d not valid.\r\n", port);
    }
    /*
     * }
     */
#else
    /*
     * {
     */
    else
        /*
         * }
         */
#endif
    {
        BCM_GPORT_LOCAL_SET(gport, port);
    }

    
#if WAITING_FOR_IMPLEMENTATION
    /*
     * {
     */
    rv = bcm_dnx_stk_my_modid_get(unit, &my_modid);
    BCMDNX_IF_ERR_EXIT(rv);
    /*
     * }
     */
#else
    /*
     * {
     */
    my_modid = 0;
    /*
     * }
     */
#endif

    if (BCM_GPORT_IS_LOCAL(gport))
    {
        /*
         * Local Port
         */
        gport_info->local_port = BCM_GPORT_LOCAL_GET(gport);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;

        /*
         * get physical system port, identify <mod,port>
         */
        if (operations & DNX_ALGO_GPM_GPORT_TO_PHY_OP_RETRIVE_SYS_PORT)
        {
            
#if WAITING_FOR_IMPLEMENTATION
            /*
             * {
             */
            rv = soc_port_sw_db_local_to_tm_port_get(unit, gport_info->local_port, &gport_info->internal_port_tm,
                                                     &gport_info->internal_core_id);
            BCMDNX_IF_ERR_EXIT(rv);
            /*
             * }
             */
#else
            /*
             * {
             */
            gport_info->internal_port_tm = gport_info->local_port;
            
            gport_info->internal_core_id = 0;
            /*
             * }
             */
#endif

            
#if WAITING_FOR_IMPLEMENTATION
            /*
             * {
             */
            soc_sand_rv =
                (MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_local_to_sys_phys_port_map_get,
                                      (unit, my_modid + gport_info->internal_core_id, gport_info->internal_port_tm,
                                       &(gport_info->sys_port))));
            BCM_SAND_IF_ERR_EXIT(soc_sand_rv);
            if (gport_info->sys_port == SOC_TMC_SYS_PHYS_PORT_INVALID)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "given gport: 0x%08x is invalid port.\r\n", gport);
            }
            /*
             * }
             */
#else
            /*
             * {
             */
            gport_info->sys_port = gport_info->local_port;
            /*
             * }
             */
#endif
        }
    }
    else if (BCM_GPORT_IS_MODPORT(gport))
    {
        /*
         * Mod Port
         */
        modid = BCM_GPORT_MODPORT_MODID_GET(gport);
        mod_port = BCM_GPORT_MODPORT_PORT_GET(gport);

        if ((modid >= my_modid) && (modid < (my_modid + SOC_DNX_DEFS_GET(unit, nof_cores))))
        {
            gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;

            gport_info->internal_core_id = modid - my_modid;
            
#if WAITING_FOR_IMPLEMENTATION
            /*
             * {
             */
            SOC_DPP_CORE_VALIDATE(unit, gport_info->internal_core_id, FALSE);
            rv = soc_port_sw_db_tm_to_local_port_get(unit, gport_info->internal_core_id, mod_port,
                                                     (soc_port_t *) & local_port);
            BCMDNX_IF_ERR_EXIT(rv);
            /*
             * }
             */
#else
            /*
             * {
             */
            gport_info->local_port = mod_port;
            /*
             * }
             */
#endif

        }

        /*
         * get physical system port, identify <mod,port>
         */
        if (operations & DNX_ALGO_GPM_GPORT_TO_PHY_OP_RETRIVE_SYS_PORT)
        {
            
#if WAITING_FOR_IMPLEMENTATION
            /*
             * {
             */
            soc_sand_rv =
                (MBCM_DPP_DRIVER_CALL
                 (unit, mbcm_dpp_local_to_sys_phys_port_map_get, (unit, modid, mod_port, &(gport_info->sys_port))));
            BCM_SAND_IF_ERR_EXIT(soc_sand_rv);
            if (gport_info->sys_port == SOC_TMC_SYS_PHYS_PORT_INVALID)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "given gport: 0x%x is invalid port.\r\n", gport);
            }
            /*
             * }
             */
#else
            /*
             * {
             */
            gport_info->sys_port = gport_info->local_port;
            /*
             * }
             */
#endif
        }
    }
    else if (BCM_GPORT_IS_DEVPORT(gport))
    {
        /*
         * Device Port
         */
        gport_info->local_port = BCM_GPORT_DEVPORT_PORT_GET(gport);
        if (unit == BCM_GPORT_DEVPORT_DEVID_GET(gport))
        {
            gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
        }
        
#if WAITING_FOR_IMPLEMENTATION
        /*
         * {
         */
    }
    else if (BCM_GPORT_IS_TRUNK(gport))
    {
        /*
         * trunk - no local port, only system port
         */
        trunk_id = BCM_GPORT_TRUNK_GET(gport);
        /*
         * map lag to system port
         */
        if (trunk_id > BCM_DPP_MAX_TRUNK_ID(unit))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Trunk id 0x%x is higher than the maximal value.\r\n", gport);
        }
        soc_sand_rv =
            (MBCM_DPP_DRIVER_CALL
             (unit, mbcm_dpp_ports_logical_sys_id_build_with_device,
              (unit, TRUE, trunk_id, 0, 0, &(gport_info->sys_port))));
        BCM_SAND_IF_ERR_EXIT(soc_sand_rv);

        gport_info->flags |= _BCM_DPP_GPORT_INFO_F_IS_LAG;
        /*
         * }
         */
#endif
    }
    else if (BCM_GPORT_IS_SYSTEM_PORT(gport))
    {
        /*
         * System Port
         */
        gport_info->sys_port = BCM_GPORT_SYSTEM_PORT_ID_GET(gport);

        
#if WAITING_FOR_IMPLEMENTATION
        /*
         * {
         */
        /*
         * map sys port to fap and tm-port
         */
        soc_sand_rv =
            (MBCM_DPP_DRIVER_CALL
             (unit, mbcm_dpp_sys_phys_to_local_port_map_get,
              (unit, gport_info->sys_port, (uint32 *) & modid, (uint32 *) & mod_port)));
        BCM_SAND_IF_ERR_EXIT(soc_sand_rv);

        soc_sand_rv = arad_sw_db_modport2sysport_get(unit, modid, mod_port, &master_sysport);
        BCM_SAND_IF_ERR_EXIT(soc_sand_rv);

        /*
         * In HQOS mode, only the first mapped system port should be used as a handle for PP
         */
        if ((ARAD_IS_HQOS_MAPPING_ENABLE(unit)) && (master_sysport != gport_info->sys_port))
        {
            BCMDNX_ERR_EXIT_MSG(_SHR_E_BADID, (_BSL_BCM_MSG
                                               ("in HQOS mapping mode, the handle has to be the first mapped system port")));
        }

        BCM_DPP_UNIT_CHECK(unit);
        if (SOC_DPP_IS_MODID_AND_BASE_MODID_ON_SAME_FAP(unit, modid, my_modid))
        {
            gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;

            gport_info->internal_core_id = SOC_DPP_MODID_TO_CORE(unit, my_modid, modid);
            rv = soc_port_sw_db_tm_to_local_port_get(unit, gport_info->internal_core_id, mod_port,
                                                     (soc_port_t *) & gport_info->local_port);
            BCMDNX_IF_ERR_EXIT(rv);

        }
        /*
         * }
         */
#else
        /*
         * {
         */
        gport_info->local_port = gport_info->sys_port;
        /*
         * }
         */
#endif
        
#if WAITING_FOR_IMPLEMENTATION
        /*
         * {
         */
    }
    else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport))
    {   /* unicast queue */

        queue_id = BCM_GPORT_UNICAST_QUEUE_GROUP_QID_GET(gport);

        /*
         * map flow to sys port
         */
        soc_sand_rv =
            (MBCM_DPP_DRIVER_CALL
             (unit, mbcm_dpp_ipq_queue_to_flow_mapping_get,
              (unit, gport_info->internal_core_id, (queue_id / 4), &queue_map_info)));
        BCM_SAND_IF_ERR_EXIT(soc_sand_rv);

        gport_info->sys_port = queue_map_info.system_physical_port;

        /*
         * map sys port to fap and tm-port
         */
        soc_sand_rv =
            (MBCM_DPP_DRIVER_CALL
             (unit, mbcm_dpp_sys_phys_to_local_port_map_get,
              (unit, gport_info->sys_port, (uint32 *) & modid, (uint32 *) & mod_port)));
        BCM_SAND_IF_ERR_EXIT(soc_sand_rv);

        if (SOC_DPP_IS_MODID_AND_BASE_MODID_ON_SAME_FAP(unit, modid, my_modid))
        {
            gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;

            gport_info->internal_core_id = SOC_DPP_MODID_TO_CORE(unit, my_modid, modid);
            rv = soc_port_sw_db_tm_to_local_port_get(unit, gport_info->internal_core_id, mod_port,
                                                     (soc_port_t *) & gport_info->local_port);
            BCMDNX_IF_ERR_EXIT(rv);

        }
    }
    else if (BCM_GPORT_IS_LOCAL_CPU(gport))
    {   /* CPU-port */

        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
        gport_info->local_port = CMIC_PORT(unit);

        /*
         * get physical system port, identify <mod,port>
         */
        if (operations & DNX_ALGO_GPM_GPORT_TO_PHY_OP_RETRIVE_SYS_PORT)
        {
            soc_sand_rv =
                (MBCM_DPP_DRIVER_CALL
                 (unit, mbcm_dpp_local_to_sys_phys_port_map_get,
                  (unit, my_modid, gport_info->local_port, &(gport_info->sys_port))));
            BCM_SAND_IF_ERR_EXIT(soc_sand_rv);
            if (gport_info->sys_port == SOC_TMC_SYS_PHYS_PORT_INVALID)
            {
                SHR_ERR_EXIT(_SHR_E_PARAM, "System port value is invalid port.\r\n");
            }
        }
        /*
         * }
         */
#endif
    }
    else if (BCM_PHY_GPORT_IS_PHYN(port))
    {
        gport_info->local_port = BCM_PHY_GPORT_PHYN_PORT_PORT_GET(port);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
    }
    else if (BCM_PHY_GPORT_IS_PHYN_LANE(port))
    {
        gport_info->local_port = BCM_PHY_GPORT_PHYN_LANE_PORT_PORT_GET(port);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
    }
    else if (BCM_PHY_GPORT_IS_PHYN_SYS_SIDE(port))
    {
        gport_info->local_port = BCM_PHY_GPORT_PHYN_SYS_SIDE_PORT_PORT_GET(port);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_SYS_SIDE;
    }
    else if (BCM_PHY_GPORT_IS_PHYN_SYS_SIDE_LANE(port))
    {
        gport_info->local_port = BCM_PHY_GPORT_PHYN_SYS_SIDE_LANE_PORT_PORT_GET(port);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_SYS_SIDE;
    }
    else if (BCM_PHY_GPORT_IS_LANE(gport))
    {
        gport_info->local_port = BCM_PHY_GPORT_LANE_PORT_PORT_GET(port);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
        
#if WAITING_FOR_IMPLEMENTATION
        /*
         * {
         */
    }
    else if (BCM_GPORT_IS_LOCAL_FABRIC(gport))
    {
        gport_info->local_port = BCM_GPORT_LOCAL_FABRIC_GET(gport) + FABRIC_LOGICAL_PORT_BASE(unit);
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT;
        /*
         * }
         */
#endif
    }
    else if (BCM_GPORT_IS_BLACK_HOLE(gport))
    {
        gport_info->flags |= DNX_ALGO_GPM_GPORT_INFO_F_IS_BLACK_HOLE;
    }
    /*
     * Return Error, as port is not physical
     */
    else
    {
        SHR_ERR_EXIT(_SHR_E_INTERNAL,
                     "GPORT %d is not physical, and thus cannot be input to dnx_algo_gpm_gport_phy_info_get.\r\n",
                     gport);
    }

    if (operations & DNX_ALGO_GPM_GPORT_TO_PHY_OP_LOCAL_IS_MANDATORY)
    {
        if (!(gport_info->flags & DNX_ALGO_GPM_GPORT_INFO_F_IS_LOCAL_PORT))
        {
            SHR_ERR_EXIT(_SHR_E_PORT, "given gport: 0x%x that is not resolved to local port.\r\n", gport);
        }
    }

    
    SHR_IF_ERR_EXIT(algo_local_port_to_pp_port(unit, gport_info->local_port, &pp_port, &core_id));

    gport_info->internal_core_id = core_id;
    gport_info->internal_port_pp = pp_port;
    gport_info->internal_port_tm = gport_info->local_port;

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

/*
 * See .h file
 * Note: PON is not supported.
 */
shr_error_e
dnx_algo_gpm_gport_is_physical(
    int unit,
    bcm_gport_t port,
    uint8 * is_physical_port)
{

    bcm_gport_t gport;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Verify the pointer 'gport_info' is not NULL and exit with error if it is.
     */
    SHR_NULL_CHECK(is_physical_port, _SHR_E_PARAM, "is_physical_port");

    /*
     * If GPORT is invalid EXIT
     */
    if (port == BCM_GPORT_INVALID)
    {
        SHR_EXIT();
    }

    /*
     * Check if gport is actually local port that is not encoded as GPORT
     */
    if (BCM_GPORT_IS_SET(port))
    {
        gport = port;
    }
    else if (SOC_PORT_VALID(unit, port))
    {
        BCM_GPORT_LOCAL_SET(gport, port);
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "PORT %d is not valid.\r\n", port);
    }

    /*
     * GPORT is physical in case it's not one of the following types:
     * VPAN PORT / MPLS PORT / MIM PORT / TRILL PORT / TUNNEL / L2GRE PORT / VXLAN PORT / FORWARD PORT / EXTENDER PORT
     */
    if (BCM_GPORT_IS_VLAN_PORT(gport) || BCM_GPORT_IS_MPLS_PORT(gport) || BCM_GPORT_IS_MIM_PORT(gport) ||
        BCM_GPORT_IS_TRILL_PORT(gport) || BCM_GPORT_IS_TUNNEL(gport) || BCM_GPORT_IS_L2GRE_PORT(gport) ||
        BCM_GPORT_IS_VXLAN_PORT(gport) || BCM_GPORT_IS_FORWARD_PORT(gport) || BCM_GPORT_IS_EXTENDER_PORT(gport))
    {

        *is_physical_port = 0;
    }
    else
    {
        *is_physical_port = 1;
    }

    SHR_EXIT();

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_gpm_gport_validity_check(
    int unit,
    bcm_gport_t gport)
{

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Currently the only check is GPORT encoding.
     */
    if (!BCM_GPORT_IS_SET(gport))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "gport %d is not encoded as GPORT.\r\n", gport);
    }

exit:
    SHR_FUNC_EXIT;
}

/**
* \brief
*   Given a gport, returns related global HW resources -
*   Global-LIF or FEC. Both ingress and egress gports are
*   returned.
*
*  \par DIRECT INPUT:
*    \param [in] unit -
*      Relevant unit.
*    \param [in] gport -
*      In GPORT given by user
*    \param [in] global_in_lif_id -
*      Pointer to global_in_lif_id
*    \param [in] global_out_lif_id -
*      Pointer to global_out_lif_id
*    \param [in] fec_id -
*      Pointer to fec_id
*  \par DIRECT OUTPUT:
*    shr_error_e -
*      Error return value
*  \par INDIRECT OUTPUT:
*    *global_in_lif_id -
*      Global In Lif, DNX_ALGO_GPM_LIF_INVALID if invalid
*    *global_out_lif_id -
*      Global Out Lif, DNX_ALGO_GPM_LIF_INVALID if invalid
*    *fec_id -
*      FEC id, DNX_ALGO_GPM_FEC_INVALID if invalid
*  \remark
*    Current implementation assume that only one of the two -
*    global_in/out_lif_id or fec_id can be returned but not
*    both. In case both will be needed, it will require more
*    implementation.
*****************************************************/
static shr_error_e
dnx_algo_gpm_gport_to_global_resources(
    int unit,
    bcm_gport_t gport,
    int *global_in_lif_id,
    int *global_out_lif_id,
    int *fec_id)
{
    int gport_internal_id;

    SHR_FUNC_INIT_VARS(unit);

    /*
     *  Validity - if port is not valid or not set then nothing to do
     */
    if ((gport == BCM_GPORT_INVALID) || !BCM_GPORT_IS_SET(gport))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "gport given is invalid when trying to map gport to lif, gport (0x%x)\n", gport);
    }

    *global_in_lif_id = DNX_ALGO_GPM_LIF_INVALID;
    *global_out_lif_id = DNX_ALGO_GPM_LIF_INVALID;
    *fec_id = DNX_ALGO_GPM_FEC_INVALID;

    /*
     *  1. In these cases we can extract the HW resources directly from 'gport' encoding
     */

    /*
     * Use gport_internal_id in case GPORT has GPORT SUBTYPE, for example GPORT of type VLAN_PORT or MPLS PORT.
     * In that case we need to check GPORT SUBTYPE to understand which resources are relevant
     */
    gport_internal_id = BCM_GPORT_INVALID;

    /*
     *  Go through all possible GPORT types
     */
    if (BCM_GPORT_IS_VLAN_PORT(gport))
    {
        /*
         * VLAN PORT - store internal id, check SUB_TYPE later on
         */
        gport_internal_id = BCM_GPORT_VLAN_PORT_ID_GET(gport);
    }
    else if (BCM_GPORT_IS_MPLS_PORT(gport))
    {
        /*
         * MPLS PORT - store internal id, check SUB_TYPE later on
         */
        gport_internal_id = BCM_GPORT_MPLS_PORT_ID_GET(gport);
    }
    else if (BCM_GPORT_IS_MIM_PORT(gport))
    {
        /*
         * MIM PORT - store FEC, no valid LIF
         */
        *fec_id = BCM_GPORT_MIM_PORT_ID_GET(gport);
    }
    else if (BCM_GPORT_IS_EXTENDER_PORT(gport))
    {
        /*
         * EXTENDER PORT - store LIF, no valid FEC
         */
        *global_in_lif_id = *global_out_lif_id = BCM_GPORT_EXTENDER_PORT_ID_GET(gport);
    }
    else if (BCM_GPORT_IS_TUNNEL(gport))
    {
        /*
         * IP-tunnel / MPLS tunnel - in/out-LIF ID, no valid FEC
         */
        if (!BCM_GPORT_SUB_TYPE_IS_MULTICAST(gport))
        {
            *global_in_lif_id = *global_out_lif_id = BCM_GPORT_TUNNEL_ID_GET(gport);
        }
    }
    else if (BCM_GPORT_IS_L2GRE_PORT(gport))
    {
        /*
         * L2 GRE PORT - in-LIF ID, no valid FEC or out lif
         */
        *global_in_lif_id = BCM_GPORT_L2GRE_PORT_ID_GET(gport);
    }
    else if (BCM_GPORT_IS_VXLAN_PORT(gport))
    {
        /*
         * VXLAN PORT - in-LIF ID, no valid FEC or out lif
         */
        *global_in_lif_id = BCM_GPORT_VXLAN_PORT_ID_GET(gport);
    }
    else if (BCM_GPORT_IS_FORWARD_PORT(gport))
    {
        /*
         * FORWARD PORT - only FEC is valid
         */
        *fec_id = BCM_GPORT_FORWARD_PORT_GET(gport);
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "Unknown GPORT type, gport (0x%x)\n", gport);
    }

    /*
     * Check subtype if relevant
     */
    if (gport_internal_id != BCM_GPORT_INVALID)
    {
        /*
         * For example in case of VLAN_PORT and MPLS_PORT
         */
        if (BCM_GPORT_SUB_TYPE_IS_FORWARD_GROUP(gport))
        {
            /*
             * Forward - get from subtype encoding of the gport
             */
            *fec_id = BCM_GPORT_SUB_TYPE_FORWARD_GROUP_GET(gport_internal_id);
        }
        else if (BCM_GPORT_SUB_TYPE_IS_LIF(gport))
        {
            /*
             * LIF - can be either ingress only/egress only or both
             * Get from subtype encoding of the gport
             */
            if ((BCM_GPORT_SUB_TYPE_LIF_EXC_GET(gport_internal_id) == BCM_GPORT_SUB_TYPE_LIF_EXC_INGRESS_ONLY))
            {
                *global_in_lif_id = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(gport_internal_id);
                *global_out_lif_id = DNX_ALGO_GPM_LIF_INVALID;
            }
            else if ((BCM_GPORT_SUB_TYPE_LIF_EXC_GET(gport_internal_id) == BCM_GPORT_SUB_TYPE_LIF_EXC_EGRESS_ONLY))
            {
                *global_out_lif_id = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(gport_internal_id);
                *global_in_lif_id = DNX_ALGO_GPM_LIF_INVALID;
            }
            else
            {
                *global_in_lif_id = *global_out_lif_id = BCM_GPORT_SUB_TYPE_LIF_VAL_GET(gport_internal_id);
            }
        }
    }

    /*
     * 2. These resources we can get only from reading SW state
     * Currntly nothing to do here - might be required in the future
     */

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
dnx_algo_gpm_gport_to_hw_resources(
    int unit,
    bcm_gport_t gport,
    uint32 flags,
    dnx_algo_gpm_gport_hw_resources_t * gport_hw_resources)
{
    int *global_in_lif = NULL;
    int *global_out_lif = NULL;
    int *fec_id = NULL;
    int *local_in_lif = NULL;
    int *local_out_lif = NULL;
    int global_in_lif_dummy = DNX_ALGO_GPM_LIF_INVALID;
    int global_out_lif_dummy = DNX_ALGO_GPM_LIF_INVALID;
    int fec_dummy = DNX_ALGO_GPM_FEC_INVALID;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(gport_hw_resources, _SHR_E_PARAM, "gport_hw_resources");

    /*
     * Verify that at least one flag is set.
     */
    if ((flags & (DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_AND_GLOBAL_LIF | DNX_ALGO_GPM_GPORT_HW_RESOURCES_FEC)) == 0)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "No flags requested, gport (0x%x)\n", gport);
    }

    /*
     * Verify that caller is not requesting both FEC and LIF.
     * This is not supported by current implementation.
     */
    if (((flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF) || (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF))
        && (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_FEC))
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "FEC and LIF resources cannot be requested in the same time, gport (0x%x)\n", gport);
    }

    /*
     * Init values
     */
    gport_hw_resources->fec_id = DNX_ALGO_GPM_FEC_INVALID;
    gport_hw_resources->global_in_lif = DNX_ALGO_GPM_LIF_INVALID;
    gport_hw_resources->global_out_lif = DNX_ALGO_GPM_LIF_INVALID;
    gport_hw_resources->local_in_lif = DNX_ALGO_GPM_LIF_INVALID;
    gport_hw_resources->local_out_lif = DNX_ALGO_GPM_LIF_INVALID;

    if (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_INGRESS)
    {
        global_in_lif = &(gport_hw_resources->global_in_lif);
    }
    if (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_EGRESS)
    {
        global_out_lif = &(gport_hw_resources->global_out_lif);
    }
    if (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_INGRESS)
    {
        local_in_lif = &(gport_hw_resources->local_in_lif);
    }
    if (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_EGRESS)
    {
        local_out_lif = &(gport_hw_resources->local_out_lif);
    }
    if (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_FEC)
    {
        fec_id = &(gport_hw_resources->fec_id);
    }

    /*
     * First, retrieve GLOBAL LIFs and FEC
     */
    SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_to_global_resources
                    (unit, gport, &global_in_lif_dummy, &global_out_lif_dummy, &fec_dummy));
    SHR_IF_NOT_NULL_FILL(global_in_lif, global_in_lif_dummy);
    SHR_IF_NOT_NULL_FILL(global_out_lif, global_out_lif_dummy);
    SHR_IF_NOT_NULL_FILL(fec_id, fec_dummy);

    /*
     * Second, if LOCAL LIFs required, get them
     */
    if (local_in_lif)
    {
        if (global_in_lif && (*global_in_lif != DNX_ALGO_GPM_LIF_INVALID))
        {
            SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_global_to_local_get(unit, DNX_ALGO_LIF_INGRESS, *global_in_lif, local_in_lif));
        }
        else
        {
            *local_in_lif = DNX_ALGO_GPM_LIF_INVALID;
        }
    }

    if (local_out_lif)
    {
        if (global_out_lif && (*global_out_lif != DNX_ALGO_GPM_LIF_INVALID))
        {

            SHR_IF_ERR_EXIT(dnx_algo_lif_mapping_global_to_local_get(unit, DNX_ALGO_LIF_EGRESS, *global_out_lif, local_out_lif));
        }
        else
        {
            *local_out_lif = DNX_ALGO_GPM_LIF_INVALID;
        }
    }

    /*
     * Strict checks - return error in case one of the requested parameters wasn't found
     */
    if (flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_STRICT_CHECK)
    {
        /*
         * FEC
         */
        if ((flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_FEC) && (gport_hw_resources->fec_id == DNX_ALGO_GPM_FEC_INVALID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Requested FEC_ID but it is invalid, gport (0x%x)\n", gport);
        }
        /*
         * Ingress Global LIF
         */
        if ((flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_INGRESS) &&
            (gport_hw_resources->global_in_lif == DNX_ALGO_GPM_LIF_INVALID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Requested GLOBAL_LIF_INGRESS but it is invalid, gport (0x%x)\n", gport);
        }
        /*
         * Egress Global LIF
         */
        if ((flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_GLOBAL_LIF_EGRESS) &&
            (gport_hw_resources->global_out_lif == DNX_ALGO_GPM_LIF_INVALID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Requested GLOBAL_LIF_EGRESS but it is invalid, gport (0x%x)\n", gport);
        }
        /*
         * Ingress Local LIF
         */
        if ((flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_INGRESS) &&
            (gport_hw_resources->local_in_lif == DNX_ALGO_GPM_LIF_INVALID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Requested LOCAL_LIF_INGRESS but it is invalid, gport (0x%x)\n", gport);
        }
        /*
         * Egress Local LIF
         */
        if ((flags & DNX_ALGO_GPM_GPORT_HW_RESOURCES_LOCAL_LIF_EGRESS) &&
            (gport_hw_resources->local_out_lif == DNX_ALGO_GPM_LIF_INVALID))
        {
            SHR_ERR_EXIT(_SHR_E_PARAM, "Requested LOCAL_LIF_EGRESS but it is invalid, gport (0x%x)\n", gport);
        }
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * Same interface as
 * algo_gpm_encode_destination_field_from_gport, but this
 * function also returns one more parameter:
 *
 *  gport_is_valid_destination -
 *      If GPORT cannot be translated to destination pointed
 *      memory is loaded by FALSE, otherwise TRUE
 *
 */
static shr_error_e
algo_gpm_encode_destination_field_from_gport_internal(
    int unit,
    bcm_gport_t port,
    uint32 * destination,
    uint8 * gport_is_valid_destination)
{
    bcm_gport_t gport;
    dnx_algo_gpm_gport_phy_info_t gport_info;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Check if gport is actually local port that is not encoded as GPORT
     */
    if (BCM_GPORT_IS_SET(port))
    {
        gport = port;
    }
    
#if WAITING_FOR_IMPLEMENTATION
    /*
     * {
     */
    else if (SOC_PORT_VALID(unit, port))
    {
        BCM_GPORT_LOCAL_SET(gport, port);
    }
    else
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "gport given is invalid port, gport (0x%x)\n", port);
    }
#else
    else
    {
        BCM_GPORT_LOCAL_SET(gport, port);
    }
#endif
    /*
     * }
     */

    *gport_is_valid_destination = TRUE;

    /*
     * Set DESTINATION according to GPORT TYPE
     */

    if (BCM_GPORT_IS_BLACK_HOLE(gport))
    {
        /*
         * GPORT type is BLACK HOLE and should be dropped
         */
#if WAITING_FOR_IMPLEMENTATION
        /*
         * Add here trap with destination 0 when trap is implemented
         */
#else
        SHR_ERR_EXIT(_SHR_E_PARAM, "BLACK HOLE is currently not supported, gport (0x%x)\n", port);
#endif
    }
    else if (BCM_GPORT_IS_LOCAL(gport) ||
             BCM_GPORT_IS_LOCAL_CPU(gport) || BCM_GPORT_IS_MODPORT(gport) || BCM_GPORT_IS_SYSTEM_PORT(gport))
    {
        /*
         * GPORT type is PORT
         */
        /*
         * Get system port from gport
         */
        SHR_IF_ERR_EXIT(dnx_algo_gpm_gport_phy_info_get
                        (unit, gport, DNX_ALGO_GPM_GPORT_TO_PHY_OP_RETRIVE_SYS_PORT, &gport_info));
        /*
         * Encode destination as system port
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set
                        (unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_PORT_ID, gport_info.sys_port, destination));
    }
    else if (BCM_GPORT_IS_TRUNK(gport))
    {
        /*
         * GPORT is LAG, Encode destination as lag
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set
                        (unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_LAG_ID, BCM_GPORT_TRUNK_GET(gport), destination));
    }
    else if (BCM_GPORT_IS_MCAST(gport))
    {
        /*
         * GPORT is MULTICAST, Encode destination as multicast
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set(unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_MC_ID,
                                                             _BCM_MULTICAST_ID_GET(BCM_GPORT_MCAST_GET(gport)),
                                                             destination));
    }
    else if (((BCM_GPORT_IS_VLAN_PORT(gport)) || (BCM_GPORT_IS_EXTENDER_PORT(gport)) ||
              (BCM_GPORT_IS_MPLS_PORT(gport))) && ((BCM_GPORT_SUB_TYPE_IS_MULTICAST(gport))))
    {
        /*
         * GPORT type is sub-type MULTICAST
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set
                        (unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_MC_ID, BCM_GPORT_SUB_TYPE_MULTICAST_VAL_GET(gport),
                         destination));

    }
    else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport))
    {
        /*
         * GPORT type is UNICAST GROUP (CosQ/Flow), Encode destination as flow id
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set(unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_FLOW_ID,
                                                             BCM_GPORT_UNICAST_QUEUE_GROUP_QID_GET(gport),
                                                             destination));
    }

    else if (BCM_GPORT_IS_TRAP(gport))
    {
        /*
         * GPORT type is TRAP
         */

#if WAITING_FOR_IMPLEMENTATION
        ALGO_GPM_DESTINATION_TYPE_TRAP_SET(BCM_GPORT_TRAP_GET_ID(gport),
                                           BCM_GPORT_TRAP_GET_STRENGTH(gport),
                                           BCM_GPORT_TRAP_GET_SNOOP_STRENGTH(gport), destination);

        SOC_PPC_TRAP_CODE ppd_trap_code;

        trap_id = BCM_GPORT_TRAP_GET_ID(gport);
        /*
         * trap_id may be a virtual trap. convert it to ppd trap to create forward decision
         */
        rv = _bcm_rx_ppd_trap_code_from_trap_id(unit, trap_id, &ppd_trap_code);
        BCMDNX_IF_ERR_EXIT(rv);

        snoop_str = BCM_GPORT_TRAP_GET_SNOOP_STRENGTH(gport);
        trap_str = BCM_GPORT_TRAP_GET_STRENGTH(gport);
        SOC_PPD_FRWRD_DECISION_TRAP_SET(unit, fwd_decsion, ppd_trap_code, trap_str, snoop_str, soc_sand_rv);
        BCM_SAND_IF_ERR_EXIT(soc_sand_rv);
        BCM_EXIT;
#else
        SHR_ERR_EXIT(_SHR_E_PARAM, "TRAP is currently not supported, gport (0x%x)\n", port);
#endif
    }
    else if (BCM_GPORT_IS_FORWARD_PORT(gport))
    {
        /*
         * GPORT type is FEC (forward group), encode destination as FEC
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set
                        (unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_FEC, BCM_GPORT_FORWARD_PORT_GET(gport), destination));
    }
    else if (((BCM_GPORT_IS_VLAN_PORT(gport)) || (BCM_GPORT_IS_EXTENDER_PORT(gport)) ||
              (BCM_GPORT_IS_MPLS_PORT(gport))) && ((BCM_GPORT_SUB_TYPE_IS_FORWARD_GROUP(gport))))
    {
        /*
         * GPORT type is FEC (subtype forward group)
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set
                        (unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_FEC, BCM_GPORT_SUB_TYPE_FORWARD_GROUP_GET(gport),
                         destination));
    }
    else if (((BCM_GPORT_IS_VLAN_PORT(gport)) || (BCM_GPORT_IS_EXTENDER_PORT(gport)) ||
              (BCM_GPORT_IS_MPLS_PORT(gport))) && ((BCM_GPORT_SUB_TYPE_IS_PROTECTION(gport))))
    {
        /*
         * GPORT type is FEC (subtype protection)
         */
        SHR_IF_ERR_EXIT(dbal_fields_parent_field32_value_set
                        (unit, DBAL_FIELD_DESTINATION, DBAL_FIELD_FEC, BCM_GPORT_SUB_TYPE_PROTECTION_GET(gport),
                         destination));
    }
    else
    {
        /*
         * Unsupported GPORT type
         */
        *gport_is_valid_destination = FALSE;
    }

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief
 * See documentation in .h file
 */
shr_error_e
algo_gpm_encode_destination_field_from_gport(
    int unit,
    bcm_gport_t port,
    uint32 * destination)
{
    uint8 gport_is_valid_destination;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(destination, _SHR_E_PARAM, "destination");

    SHR_IF_ERR_EXIT(algo_gpm_encode_destination_field_from_gport_internal
                    (unit, port, destination, &gport_is_valid_destination));
    if (!gport_is_valid_destination)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "port is not a valid destination gport.\r\n");
    }

exit:
    SHR_FUNC_EXIT;
}

shr_error_e
algo_gpm_gport_and_encap_to_forward_information(
    int unit,
    bcm_gport_t gport,
    uint32 encap_id,
    dnx_algo_gpm_forward_info_t * forward_info)
{
    uint8 gport_is_valid_destination;
    uint32 entry_handle_id;
    uint32 rv;

    SHR_FUNC_INIT_VARS(unit);

    SHR_NULL_CHECK(forward_info, _SHR_E_PARAM, "forward_info");

    /*
     * Destination - either from GPORT or from SW state
     */
    SHR_IF_ERR_EXIT(algo_gpm_encode_destination_field_from_gport_internal
                    (unit, gport, &forward_info->destination, &gport_is_valid_destination));
    /*
     * If GPORT is not valid destination - probably logical gport, try to get forward info from SW state
     */
    if (gport_is_valid_destination)
    {
        /*
         * If GPORT is valid destination - get additional info from encap id
         */
        if (encap_id != BCM_FORWARD_ENCAP_ID_INVALID)
        {
            /*
             * If ecnap_id valid, get type from encap type
             */
            if ((BCM_GPORT_IS_VLAN_PORT(gport) || BCM_GPORT_IS_MPLS_PORT(gport))
                && BCM_GPORT_SUB_TYPE_GET(gport) == BCM_GPORT_SUB_TYPE_FORWARD_GROUP)
            {
                forward_info->fwd_info_result_type = DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_OUTLIF;
                forward_info->outlif = BCM_FORWARD_ENCAP_ID_VAL_GET(encap_id);
            }
            else if (BCM_FORWARD_ENCAP_ID_IS_OUTLIF(encap_id))
            {
                forward_info->fwd_info_result_type = DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_OUTLIF;
                forward_info->outlif = BCM_FORWARD_ENCAP_ID_VAL_GET(encap_id);
            }
            else
            {
                /*
                 * EEI
                 */
                if (BCM_FORWARD_ENCAP_ID_EEI_USAGE_GET(encap_id) == BCM_FORWARD_ENCAP_ID_EEI_USAGE_MPLS_PORT)
                {
                    /*
                     * VC + Push Profile
                     */
                    forward_info->fwd_info_result_type = DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_EEI;
                    forward_info->eei = BCM_FORWARD_ENCAP_ID_VAL_GET(encap_id);
                }
                else if (BCM_FORWARD_ENCAP_ID_EEI_USAGE_GET(encap_id) == BCM_FORWARD_ENCAP_ID_EEI_USAGE_ENCAP_POINTER)
                {
                    /*
                     * Outlif
                     */
                    forward_info->fwd_info_result_type = DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_OUTLIF;
                    forward_info->outlif = BCM_FORWARD_ENCAP_ID_VAL_GET(encap_id);
                }
                else
                {
                    SHR_ERR_EXIT(_SHR_E_PARAM, "Unknown EEI type, gport (0x%x)\n", gport);
                }
            }
        }
        else
        {
            /*
             * If ecnap_id not valid, type is destination only
             */
            forward_info->fwd_info_result_type = DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_ONLY;
        }
    }
    else
    {
        /*
         * If GPORT is not valid destination - probably logical gport, try to get forward info from SW state
         */
        /*
         * Get from Gport to Forward Info table (SW state)
         */
        SHR_IF_ERR_EXIT(dbal_entry_handle_take(unit, DBAL_TABLE_SW_STATE_GPORT_TO_FORWARDING_INFO, &entry_handle_id));
        dbal_entry_key_field32_set(unit, entry_handle_id, DBAL_FIELD_LOGICAL_GPORT, INST_SINGLE, gport);
        rv = dbal_entry_get(unit, entry_handle_id, DBAL_GET_ALL_FIELDS_AND_KEEP_HANDLE);
        if (rv)
        {
            dbal_entry_handle_release(unit, entry_handle_id);
            SHR_ERR_EXIT(rv, "dbal_entry_handle_release failed\n");
        }

        /*
         * Get type from entry
         */
        rv = dbal_entry_handle_value_field32_get(unit, entry_handle_id, DBAL_FIELD_RESULT_TYPE, INST_SINGLE,
                                                 &forward_info->fwd_info_result_type);
        if (rv)
        {
            dbal_entry_handle_release(unit, entry_handle_id);
            SHR_ERR_EXIT(rv, "dbal_entry_handle_release failed\n");
        }
        /*
         * Destination - always exists
         */
        dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_DESTINATION, INST_SINGLE,
                                     &forward_info->destination);
        /*
         * According to type - fill OutLif / EEI
         */
        if (forward_info->fwd_info_result_type == DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_OUTLIF)
        {
            dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, &forward_info->outlif);
        }
        else if (forward_info->fwd_info_result_type == DBAL_RESULT_TYPE_SW_STATE_GPORT_TO_FORWARDING_INFO_DEST_EEI)
        {
            dbal_entry_value_field32_get(unit, entry_handle_id, DBAL_FIELD_OUT_LIF, INST_SINGLE, &forward_info->eei);

        }
        dbal_entry_handle_release(unit, entry_handle_id);
    }

exit:
    SHR_FUNC_EXIT;
}
