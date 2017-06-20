/*
 * $Id: cosq.c,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_COSQ

#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/cosq.h>
#include <soc/dnx/legacy/mbcm.h>
#include <shared/swstate/access/sw_state_access.h>
/*#include <bcm_int/dnx/utils.h>*/

#include <soc/dnx/dbal/dbal.h>

soc_error_t
  soc_dnx_cosq_flow_and_up_info_get(
    DNX_SAND_IN  int                             unit,
    DNX_SAND_IN  int                             core,
    DNX_SAND_IN  uint8                              is_flow,
    DNX_SAND_IN  uint32                             dest_id, /* Destination port or flow ndx */
    DNX_SAND_IN  uint32                             reterive_status,
    DNX_SAND_INOUT  DNX_TMC_SCH_FLOW_AND_UP_INFO    *flow_and_up_info
  )
{
  uint8    
    valid=0;
  uint32
    ret = SOC_E_NONE,
    flow_id =0,
    queue_quartet_ndx=0;
  uint32
    mapped_fap_id = 0,
    mapped_fap_port_id = 0;
  int 
      core_idx = core;
  uint8 
      is_fap_id_local,
      is_sw_only = FALSE;
#if defined(BROADCOM_DEBUG)
  
#endif

  DNXC_INIT_FUNC_DEFS;  

  if (flow_and_up_info->credit_sources_nof == 0)
  {
      /*First level info*/
      if (is_flow) {
         flow_id = flow_and_up_info->base_queue = dest_id;
      } else {
         ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_sys_phys_to_local_port_map_get,(unit, dest_id, &mapped_fap_id, &mapped_fap_port_id));

         if (dnx_sand_get_error_code_from_error_word(ret) != DNX_SAND_OK)
         {
            LOG_INFO(BSL_LS_SOC_COSQ,
                     (BSL_META_U(unit,
                                 "soc_petra_sys_phys_to_local_port_map_get.\n\r")));
            DNXC_SAND_IF_ERR_RETURN(ret);
         }

         DNXC_IF_ERR_EXIT(soc_dnx_is_fap_id_local_and_get_core_id(unit, mapped_fap_id, &is_fap_id_local, &core_idx));

         if (!is_fap_id_local)
         {
            LOG_INFO(BSL_LS_SOC_COSQ,
                     (BSL_META_U(unit,
                                 "Destination is on remote FAP. Cannot print flow and up.\n\r")));
            DNXC_SAND_IF_ERR_RETURN(ret);
         }

         ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ipq_destination_id_packets_base_queue_id_get,(unit, core_idx, dest_id, &valid, &is_sw_only, &(flow_and_up_info->base_queue)));
         
         if (dnx_sand_get_error_code_from_error_word(ret) != DNX_SAND_OK)
         {
            LOG_INFO(BSL_LS_SOC_COSQ,
                     (BSL_META_U(unit,
                                 "soc_petra_ipq_destination_id_packets_base_queue_id_get.\n\r")));
            DNXC_IF_ERR_EXIT(ret);
         }

         if (!valid)
         {
           DNXC_IF_ERR_EXIT(SOC_E_EXISTS);
         }

         queue_quartet_ndx = (flow_and_up_info->base_queue) / 4;

         ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ipq_queue_to_flow_mapping_get,(unit, core, queue_quartet_ndx, &(flow_and_up_info->qrtt_map_info)));

         if (dnx_sand_get_error_code_from_error_word(ret) != DNX_SAND_OK)
         {        
            LOG_INFO(BSL_LS_SOC_COSQ,
                     (BSL_META_U(unit,
                                 "soc_petra_ipq_queue_to_flow_mapping_get.\n\r")));
            DNXC_SAND_IF_ERR_RETURN(ret);
         }
         flow_id = (flow_and_up_info->qrtt_map_info).flow_quartet_index * 4;
      }
  }

  if(SOC_IS_QAX(unit)){
    
    flow_id = -1 /*BCM_COSQ_FLOW_ID_JER2_QAX_ADD_OFFSET(unit, flow_id)*/;
  }
  ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_flow_and_up_info_get,(unit, core_idx, flow_id, reterive_status, flow_and_up_info));
  DNXC_SAND_IF_ERR_RETURN(ret);

  if(SOC_IS_QAX(unit)){
      
    flow_and_up_info->sch_consumer.sub_flow[0].id = /*BCM_COSQ_FLOW_ID_JER2_QAX_SUB_OFFSET(unit, flow_and_up_info->sch_consumer.sub_flow[0].id)*/ -1;
    flow_and_up_info->sch_consumer.sub_flow[0].credit_source.id -= /*JER2_QAX_SE_ID_OFFSET*/ -1;
    if(flow_and_up_info->sch_consumer.sub_flow[1].is_valid){
        flow_and_up_info->sch_consumer.sub_flow[1].id -= /*JER2_QAX_FLOW_ID_OFFSET(unit)*/ -1;
        flow_and_up_info->sch_consumer.sub_flow[1].credit_source.id -= /*JER2_QAX_SE_ID_OFFSET*/ -1;
    }
  }

exit:
  DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_hr2ps_info_get(
    DNX_SAND_IN   int                                unit,
    DNX_SAND_IN   int                                core,
    DNX_SAND_IN   uint32                             se_id,
    DNX_SAND_OUT  dnx_soc_hr2ps_info_t                   *hr2ps_info

  )
{

  uint32                          ret = SOC_E_NONE;
  uint32                          port_id = 0, tc = 0;

  DNX_TMC_SCH_PORT_INFO sch_port_info;

  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(hr2ps_info);

  DNX_TMC_SCH_PORT_INFO_clear(&sch_port_info);

  /* get port_id and tc */
  ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_sch_se2port_tc_id, (unit, core, se_id, &port_id, &tc));
  if (DNX_SAND_FAILURE(ret)) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "error in mbcm_dnx_sch_se2port_tc_id\n")));
    DNXC_SAND_IF_ERR_RETURN(ret);
  }

  /* get scheduler info */
  ret = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_sch_aggregate_get, (unit, core, se_id, &(hr2ps_info->se_info), &(hr2ps_info->flow_info))); 
  if (DNX_SAND_FAILURE(ret)) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "error in mbcm_dnx_sch_se_get_type_by_id\n")));
    DNXC_SAND_IF_ERR_RETURN(ret);
  }

  /* get rate info*/
  ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_sch_port_priority_rate_get,(unit, core, port_id , tc, &(hr2ps_info->kbits_sec_max)));
  if (DNX_SAND_FAILURE(ret)) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "error in mbcm_dnx_ofp_rates_sch_port_priority_rate_get\n")));
    DNXC_SAND_IF_ERR_RETURN(ret);
  }

  /* get burst info*/
  ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ofp_rates_sch_port_priority_max_burst_get,(unit,core, port_id , tc, &(hr2ps_info->max_burst)));
  if (DNX_SAND_FAILURE(ret)) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "error in mbcm_dnx_ofp_rates_sch_port_priority_max_burst_get\n")));
    DNXC_SAND_IF_ERR_RETURN(ret);
  }

  /* get sched info*/
  ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_sch_port_sched_get,(unit, core, port_id, &sch_port_info));
  if (DNX_SAND_FAILURE(ret)) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "error in mbcm_dnx_sch_port_sched_get\n")));
    DNXC_SAND_IF_ERR_RETURN(ret);
  }
  hr2ps_info->mode = sch_port_info.tcg_ndx[tc];
  hr2ps_info->weight = 0;

  DNXC_SAND_IF_ERR_RETURN(ret);

exit:
  DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_non_empty_queues_info_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  int                   core_id,
    DNX_SAND_IN  uint32                queue_to_read_from,
    DNX_SAND_IN  uint32                max_array_size,
    DNX_SAND_OUT dnx_soc_ips_queue_info_t* queues,
    DNX_SAND_OUT uint32*               nof_queues_filled,
    DNX_SAND_OUT uint32*               next_queue,
    DNX_SAND_OUT uint32*               reached_end
  )
{

    uint32 ret;
#if defined(BROADCOM_DEBUG)
  
#endif

  DNXC_INIT_FUNC_DEFS; 

  

    ret = MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_ips_non_empty_queues_info_get,(unit, core_id, queue_to_read_from,
                                                                            max_array_size,queues,nof_queues_filled,next_queue,reached_end));
    DNXC_SAND_IF_ERR_RETURN(ret);

  DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_ingress_queue_info_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT DNX_TMC_ITM_QUEUE_INFO      *info    
  )
{
    uint32 ret;
#if defined(BROADCOM_DEBUG)
    
#endif

    DNXC_INIT_FUNC_DEFS;
  
    ret = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_queue_info_get,(unit, SOC_CORE_ALL, queue_ndx, info)));
    DNXC_SAND_IF_ERR_RETURN(ret);

    DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_ingress_queue_category_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT int                  *voq_category
  )
{
    uint32 ret;
    DNX_TMC_ITM_CATEGORY_RNGS info;
    int queue_category = 0;	
#if defined(BROADCOM_DEBUG)
    
#endif

    DNXC_INIT_FUNC_DEFS;

    DNX_TMC_ITM_CATEGORY_RNGS_clear(&info);

    ret = (MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_itm_category_rngs_get,(unit, BCM_CORE_ALL, &info)));
    DNXC_SAND_IF_ERR_RETURN(ret);

    if (queue_ndx <= info.vsq_ctgry0_end) {
        queue_category = 0;
    } else if (queue_ndx <= info.vsq_ctgry1_end) {
        queue_category = 1;
    } else if (queue_ndx <= info.vsq_ctgry2_end) {
        queue_category = 2;
    } else {
        queue_category = 3;
    }
    *voq_category = queue_category;

    DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_ingress_queue_to_flow_mapping_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_ndx,
    DNX_SAND_OUT DNX_TMC_IPQ_QUARTET_MAP_INFO          *queue_map_info
  )
{
    uint32 ret;
#if defined(BROADCOM_DEBUG)
    
#endif

    DNXC_INIT_FUNC_DEFS;
  
    ret = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_ipq_queue_to_flow_mapping_get,
            (unit, SOC_CORE_ALL, DNX_TMC_IPQ_Q_TO_QRTT_ID(queue_ndx), queue_map_info));
	DNXC_SAND_IF_ERR_RETURN(ret);

    DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_ingress_test_tmplt_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rt_cls_ndx,
    DNX_SAND_IN  uint32                 drop_precedence_ndx,
    DNX_SAND_OUT  DNX_TMC_ITM_ADMIT_TSTS     *test_tmplt
  )
{	
    uint32 ret;
#if defined(BROADCOM_DEBUG)
    
#endif
	
    DNXC_INIT_FUNC_DEFS;
	  
    ret = MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_itm_queue_test_tmplt_get,
				(unit, rt_cls_ndx, drop_precedence_ndx, test_tmplt));
    DNXC_SAND_IF_ERR_RETURN(ret);
	
    DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_cosq_vsq_index_global_to_group_get(
    DNX_SAND_IN  int                     unit,
    DNX_SAND_IN  uint32                     vsq_id,
    DNX_SAND_OUT DNX_TMC_ITM_VSQ_GROUP      *soc_vsq_group_type,
    DNX_SAND_OUT uint32                        *vsq_index,
    DNX_SAND_OUT uint8                         *is_ocb_only
  )
{
    uint32 ret;
#if defined(BROADCOM_DEBUG)
    
#endif

    DNXC_INIT_FUNC_DEFS;

    ret = (MBCM_DNX_DRIVER_CALL_WITHOUT_DEV_ID(unit, mbcm_dnx_itm_vsq_index_global2group,(unit, vsq_id, soc_vsq_group_type, vsq_index, is_ocb_only)));
    DNXC_SAND_IF_ERR_RETURN(ret);

    DNXC_FUNC_RETURN;
}

int 
soc_dnx_voq_max_size_drop(int unit, uint32 *is_max_size) 
{
    uint32 ret;
    DNXC_INIT_FUNC_DEFS;

    ret = (MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_ingress_drop_status,(unit, is_max_size)));
    DNXC_SAND_IF_ERR_RETURN(ret);

    DNXC_FUNC_RETURN;
}


/*
 * This function receives a FAP ID and returnes wethrer the FAP ID is local
 * (of a core of the local device) and if so, returns the core ID.
 * For single core devices or devices in single core mode, the core ID is always 0.
 */
soc_error_t soc_dnx_is_fap_id_local_and_get_core_id(
    DNX_SAND_IN   int      unit,
    DNX_SAND_IN   uint32   fap_id,    /* input FAP ID */
    DNX_SAND_OUT  uint8    *is_local, /* returns TRUE/FASLE based on if fap_id is of a local core */
    DNX_SAND_OUT  int      *core_id   /* if is_local returns TRUE, will contain the core ID of the FAP ID */
)
{
    uint8 is_fap_id_local;
    uint32 local_base_fap_id;
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(core_id);

    DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_system_fap_id_get, (unit, &local_base_fap_id))); /* get the base FAP ID of the local device */

    if (fap_id >= local_base_fap_id && fap_id < local_base_fap_id + SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
        is_fap_id_local = TRUE;
        *core_id = fap_id - local_base_fap_id;
    } else {
        is_fap_id_local = FALSE;
        *core_id = SOC_CORE_ALL;
    }

    if (is_local != NULL) {
        *is_local = is_fap_id_local;
    }

exit:
  DNXC_FUNC_RETURN;
}

soc_error_t
  soc_dnx_fc_status_info_get(
    DNX_SAND_IN int                      unit,
    DNX_SAND_IN DNX_TMC_FC_STATUS_KEY   *fc_status_key,
    DNX_SAND_OUT DNX_TMC_FC_STATUS_INFO *fc_status_info 
  )
{
    uint32 ret;

    DNXC_INIT_FUNC_DEFS;
  
    ret = (MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fc_status_info_get, (unit, fc_status_key, fc_status_info)));
    DNXC_SAND_IF_ERR_RETURN(ret);

    DNXC_FUNC_RETURN;
}


/*
 * Convert gport describing a destination to to TM dest information in DNX_TMC_DEST_INFO.
 * Identifies destination type and ID.
 */
soc_error_t 
dnx_gport_to_tm_dest_info( /* This code was originally in _bcm_dnx_gport_to_tm_dest_info() */
    DNX_SAND_IN  int               unit,
    DNX_SAND_IN  soc_gport_t       dest,      /* input destination gport */
    DNX_SAND_OUT DNX_TMC_DEST_INFO *dest_info /* output destination structure */
)
{
    soc_port_t   port;
    uint32       modid, tm_port;
    soc_gport_t  gport;
    int          core, sysport_erp = 0;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(dest_info);

    if (_SHR_GPORT_IS_SET(dest)) {
        gport = dest;
    } else if (SOC_PORT_VALID(unit, dest)) { 
        SOC_GPORT_LOCAL_SET(gport,dest);
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_DNXC_MSG("invalid port given as input"))); 
    }
    
    if (SOC_GPORT_IS_LOCAL(gport)) { /* local-port */
        DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_mgmt_system_fap_id_get,(unit, &modid)));
        port = SOC_GPORT_LOCAL_GET(gport);
        if (port == SOC_DNX_PORT_INTERNAL_ERP(0) || port == SOC_DNX_PORT_INTERNAL_ERP(1)) {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_core_get(unit, port, &core));
            
            /*DNXC_IF_ERR_EXIT(sw_state_access[unit].dnx.bcm.stack._sysport_erp.get(unit, SOC_DNX_CORE_TO_MODID(modid, core), &sysport_erp));*/
            dest_info->id = sysport_erp;
        } else { /* get the TM port and core, and then get the system port from the module ID and the TM port */
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
            DNX_SAND_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_modport_to_sys_phys_port_map_get, (unit, SOC_DNX_CORE_TO_MODID(modid, core), tm_port, &dest_info->id)));
        }
        dest_info->type = DNX_TMC_DEST_TYPE_SYS_PHY_PORT;
        dest_info->dbal_type = DBAL_FIELD_PORT_ID;
    } else if (SOC_GPORT_IS_MODPORT(gport)) { /* mod-port, get system port from it */
        DNX_SAND_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_modport_to_sys_phys_port_map_get, (unit, SOC_GPORT_MODPORT_MODID_GET(gport), SOC_GPORT_MODPORT_PORT_GET(gport), &dest_info->id)));
        dest_info->type = DNX_TMC_DEST_TYPE_SYS_PHY_PORT;
        dest_info->dbal_type = DBAL_FIELD_PORT_ID;
    } else if (SOC_GPORT_IS_TRUNK(gport)) {  /*trunk*/
        dest_info->type = DNX_TMC_DEST_TYPE_LAG;
        dest_info->id = SOC_GPORT_TRUNK_GET(gport);
        dest_info->dbal_type = DBAL_FIELD_LAG_ID;
    } else if (_SHR_GPORT_IS_SYSTEM_PORT(gport)) {  /*sytem port*/
        dest_info->type = DNX_TMC_DEST_TYPE_SYS_PHY_PORT;
        dest_info->id = _SHR_GPORT_SYSTEM_PORT_ID_GET(gport);
        dest_info->dbal_type = DBAL_FIELD_PORT_ID;
    } else if (SOC_GPORT_IS_UCAST_QUEUE_GROUP(gport)) { /* COSQ / Queue */
        dest_info->type = DNX_TMC_DEST_TYPE_QUEUE;
        dest_info->id = _SHR_GPORT_UNICAST_QUEUE_GROUP_QID_GET(gport);
        dest_info->dbal_type = DBAL_FIELD_FLOW_ID;
    } else if (_SHR_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
        dest_info->type = DNX_TMC_DEST_TYPE_QUEUE;
        dest_info->id = _SHR_GPORT_MCAST_QUEUE_GROUP_QID_GET(gport);
        dest_info->dbal_type = DBAL_FIELD_FLOW_ID;
    } else if (_SHR_GPORT_IS_MCAST(gport)) {
        dest_info->type = DNX_TMC_DEST_TYPE_MULTICAST;
        dest_info->id = _SHR_MULTICAST_ID_GET(_SHR_GPORT_MCAST_GET(gport));
        dest_info->dbal_type = DBAL_FIELD_FLOW_ID;
    } else if (SOC_GPORT_IS_BLACK_HOLE(gport)) {
        dest_info->type = DNX_TMC_DEST_TYPE_QUEUE;
        dest_info->id = 0; /* drop the packet */
    } else if (SOC_GPORT_IS_LOCAL_CPU(gport)) { /* CPU-port*/
        DNXC_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_mgmt_system_fap_id_get,(unit, &modid)));
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, CMIC_PORT(unit), &tm_port, &core));
        /* get physical system port, identify <mod,port>*/
        DNX_SAND_IF_ERR_EXIT(MBCM_DNX_DRIVER_CALL(unit,mbcm_dnx_local_to_sys_phys_port_map_get, (unit, SOC_DNX_CORE_TO_MODID(modid, core), tm_port, &dest_info->id)));
        dest_info->type = DNX_TMC_DEST_TYPE_SYS_PHY_PORT;
        dest_info->dbal_type = DBAL_FIELD_PORT_ID;
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Gport given can not be parsed as TM information")));
    }

exit:
    DNXC_FUNC_RETURN;
}
