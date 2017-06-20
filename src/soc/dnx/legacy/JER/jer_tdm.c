/*
 * $Id: jer2_jer_tdm.c, v1 18/11/2014 09:55:39 azarrin $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_TDM
#define SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_DEST_0_BIT    (0x1)
#define SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_DEST_1_BIT    (0x2)
#define SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_DEST_2_BIT    (0x4)
#define SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_LOCAL_0_BIT   (0x8)
#define SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_LOCAL_1_BIT   (0x10)
#define IRE_TDM_MESH_MC_BITS_MAX 5
/*************
 * INCLUDES  *
 *************/
#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/ARAD/arad_tdm.h>
#include <soc/dnx/legacy/ARAD/arad_api_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/JER/jer_mgmt.h>


/* 
 * Configure all ports that allow TDM traffic to ingress the device. TDM traffic on any other port will be dropped. 
* Modifying the set of TDM-enabled ports can be used for fast protection-switching of TDM traffic. 
*/ 
int jer2_jer_tdm_ingress_failover_set(int unit, bcm_pbmp_t tdm_enable_pbmp)
{
    soc_reg_above_64_val_t reg_above_64;
    int i;

    DNXC_INIT_FUNC_DEFS;

    for (i=0; i <= (SOC_REG_INFO(unit, IRE_TDM_CONTEXT_DROPr).fields[0].len-1)/32; i++) {
        reg_above_64[i] = 0xFFFFFFFF - tdm_enable_pbmp.pbits[i];
    }

    DNXC_IF_ERR_EXIT(soc_reg_above_64_set(unit, IRE_TDM_CONTEXT_DROPr, REG_PORT_ANY, 0, reg_above_64));

exit:
    DNXC_FUNC_RETURN;
}

/* 
 * Retrieve all ports that allow TDM traffic to ingress the device. TDM traffic on any other port will be dropped. 
* Modifying the set of TDM-enabled ports can be used for fast protection-switching of TDM traffic. 
*/ 
int jer2_jer_tdm_ingress_failover_get(int unit, bcm_pbmp_t *tdm_enable_pbmp)
{
    soc_reg_above_64_val_t reg_above_64;
    int i;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_reg_above_64_get(unit, IRE_TDM_CONTEXT_DROPr, REG_PORT_ANY, 0, reg_above_64));

    for (i=0; i <= (SOC_REG_INFO(unit, IRE_TDM_CONTEXT_DROPr).fields[0].len-1)/32; i++) {
        tdm_enable_pbmp->pbits[i] = 0xFFFFFFFF - reg_above_64[i];
    }

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
* NAME:
*     jer2_jer_tdm_init
* FUNCTION:
*     Initialization of the TDM configuration depends on the tdm mode.
* INPUT:
*    int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
int
  jer2_jer_tdm_init(int unit)
{
    uint32
        fld_val,
        tdm_found,
        tdm_egress_priority,
        tdm_egress_dp,
        tm_port;
    JER2_ARAD_MGMT_TDM_MODE
        tdm_mode;
    JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE
        ilkn_tdm_dedicated_queuing;
        uint8
        is_local;
    soc_reg_above_64_val_t
        data;
    int fabric_priority;
    int core, port_i;
    uint32 is_valid;
    DNX_TMC_EGR_OFP_SCH_INFO ofp_sch_info;

    DNXC_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(data);

    DNXC_IF_ERR_EXIT(dnx_handle_sand_result(jer2_arad_tdm_unit_has_tdm(unit,&tdm_found)));
    tdm_mode = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_mode;
    ilkn_tdm_dedicated_queuing = SOC_DNX_CONFIG(unit)->jer2_arad->init.ilkn_tdm_dedicated_queuing;  
    tdm_egress_priority = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_egress_priority;
    tdm_egress_dp = SOC_DNX_CONFIG(unit)->jer2_arad->init.tdm_egress_dp;
    fabric_priority = SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.fabric_tdm_priority_min;

    jer2_arad_sw_db_tdm_mode_set(unit, tdm_mode);
    jer2_arad_sw_db_ilkn_tdm_dedicated_queuing_set(unit, ilkn_tdm_dedicated_queuing);

    /* TDM packet size limit range 65-254 */
    if (!SOC_IS_QAX(unit)) {
        fld_val = JER2_ARAD_TDM_CELL_SIZE_MIN;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IRE_TDM_SIZEr, REG_PORT_ANY, 0, TDM_MIN_SIZEf,  fld_val));

        fld_val = JER2_ARAD_TDM_CELL_SIZE_MAX;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IRE_TDM_SIZEr, REG_PORT_ANY, 0, TDM_MAX_SIZEf,  fld_val));    
    }
    else{ 
        if (!soc_feature(unit, soc_feature_no_tdm)) {
            fld_val = JER2_ARAD_TDM_CELL_SIZE_MIN;
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IRE_TDM_CONFIGURATIONSr, REG_PORT_ANY, 0, TDM_MIN_SIZEf,  fld_val));

            fld_val = JER2_ARAD_TDM_CELL_SIZE_MAX;
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IRE_TDM_CONFIGURATIONSr, REG_PORT_ANY, 0, TDM_MAX_SIZEf,  fld_val));    
        }
    }
    /* IRE FTMH version for TDM packet to identify the packets as TDM flows. */
    fld_val = JER2_ARAD_TDM_VERSION_ID;
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IRE_STATIC_CONFIGURATIONr, REG_PORT_ANY, 0, FTMH_VERSIONf, fld_val));

    
    if (!SOC_IS_QAX(unit)) {
        /* IPT TDM enable */
        fld_val = 0x1;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IPT_IPT_ENABLESr, REG_PORT_ANY, 0, TDM_ENf,  fld_val));
    }

    /* Enable push queue for TDM packets */
    fld_val = DNX_TMC_TDM_PUSH_QUEUE_TYPE;
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IPS_PUSH_QUEUE_TYPES_CONFIGr, SOC_CORE_ALL, 0, PUSH_QUEUE_TYPEf,  fld_val));

    fld_val = 0x1;
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IPS_PUSH_QUEUE_TYPES_CONFIGr, SOC_CORE_ALL, 0, PUSH_QUEUE_TYPE_ENf,  fld_val));

    /*
    * In NON Fabric mode, enable traffic tdm local only.
    * Note that fabric module must be initialize before TDM module
    */
    if (tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_STA)
    {
        is_local = (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP)? TRUE:FALSE;

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, 0, FORCE_ALL_LOCALf,  is_local));
    }

    if (!soc_feature(unit, soc_feature_no_fabric)) {
        /* 
        * MODE #2 does not read from RTP - only looks at the link status. 
        * Used for TDM static routing.*/
        
        fld_val = 0x2;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_LINK_BITMAP_CONFIGURATIONr, REG_PORT_ANY, 0, IRE_TDM_MASK_MODEf,  fld_val));

        /* Fabric MC from TDM at mesh mode is enabled */
        fld_val = 1;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, 0, TDM_FMC_ENf,  fld_val));

        /* Mesh MC replication bitmap from TDM will be taken from MeshMc table in FDT. Note that at this mode the number of remote replications is limited to 2.*/
        fld_val = (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.fabric_mesh_multicast_enable != 2) ? 1 : 0;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, 0, TDM_MESH_MC_BMP_SRCf,  fld_val));

        /* FDA TDM priority */
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDA_FDA_ENABLERSr, REG_PORT_ANY, 0, TDM_HEADER_PRIORITYf,  fabric_priority));
    }
    /* TDM SP MODE CONFIGURATION*/
    if (ilkn_tdm_dedicated_queuing == JER2_ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) {
        /*Enable ilkn_tdm_dedicated_queueing*/
        fld_val = 0x1;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_0_INTERLEAVE_ENf,  fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_0_INTERLEAVE_ENf,  fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_1_INTERLEAVE_ENf,  fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_1_INTERLEAVE_ENf,  fld_val));
        if (SOC_IS_QAX(unit)) {
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_2_INTERLEAVE_ENf,  fld_val));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_2_INTERLEAVE_ENf,  fld_val));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_3_INTERLEAVE_ENf,  fld_val));
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_ILAKEN_3_INTERLEAVE_ENf,  fld_val));
        }
    }

    /* TDM general configuration */  
    if (tdm_found) /* TDM bypass is enabled */
    {
        /* Enable tdm cell mode only */
        fld_val = 0x1;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, REG_PORT_ANY, 0, EGRESS_TDM_MODEf,  fld_val));

        /* Enable 2 bytes ftmh only in optimize ftmh mode */
        fld_val = (tdm_mode == JER2_ARAD_MGMT_TDM_MODE_TDM_OPT)?0x1:0x0;
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_FTMHf,  fld_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EPNI_TDM_EPE_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_2_BYTES_FTMHf,  fld_val));  
        if (!SOC_IS_QAX(unit)) {
            DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_TDM_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_FTMH_OPTIMIZEDf,  fld_val));
        }
        else{
        	if (!soc_feature(unit, soc_feature_no_tdm)) {
                DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IRE_TDM_CONFIGURATIONSr, REG_PORT_ANY, 0, TDM_FTMH_OPTIMIZEDf,  fld_val));
            }
        }
    }

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, SOC_CORE_ALL, 0, TDM_CONTEXT_MODEf, tdm_found));

    /* TDM MC use only VLAN membership table (i.e. no need for TDM special format) */
    fld_val = 0;
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, SOC_CORE_ALL, 0, TDM_REP_FORMAT_ENf,  fld_val));

    /* TDM egress priority and dp configuration */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_PKT_TCf,  tdm_egress_priority));
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_TDM_GENERAL_CONFIGURATIONr, REG_PORT_ANY, 0, TDM_PKT_DPf,  tdm_egress_dp));

    /* TDM Always High Priority Scheduling */
    DNX_TMC_EGR_OFP_SCH_INFO_clear(&ofp_sch_info);

    for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; ++port_i) {
        /* Invalid port */
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_valid_port_get(unit, port_i, &is_valid));
        if (!is_valid) {
            continue;
        }

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port_i, &tm_port, &core));

        if (IS_TDM_PORT(unit, port_i)) {
            DNXC_IF_ERR_EXIT(dnx_handle_sand_result(jer2_arad_egr_ofp_scheduling_get(unit, core, tm_port, &ofp_sch_info)));
            ofp_sch_info.nif_priority = DNX_TMC_EGR_OFP_INTERFACE_PRIO_HIGH;
            DNXC_IF_ERR_EXIT(dnx_handle_sand_result(jer2_arad_egr_ofp_scheduling_set(unit, core, tm_port, &ofp_sch_info)));
        }
    }
exit:
    DNXC_FUNC_RETURN;
}

static CONST int soc_jer2_jer_fabric_ire_mesh_multicast_replication_dests[] = {
    SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_DEST_0_BIT,
    SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_DEST_1_BIT,
    SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_DEST_2_BIT
};

static uint32 
soc_jer2_jer_fabric_multicast_rep_bitmap_convert(
   DNX_SAND_IN  uint32                         unit, 
   DNX_SAND_IN  uint32                         destid_count,
   DNX_SAND_IN  soc_module_t                  *destid_array,
   DNX_SAND_OUT uint32                        *mc_rep
   ){
    
    soc_module_t remote_dest;
    int i, max_external_faps;
    uint32 my_core0_fap_id, my_core1_fap_id;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(jer2_jer_mgmt_system_fap_id_get(unit, &my_core0_fap_id));
    my_core1_fap_id = my_core0_fap_id + ((SOC_DNX_DEFS_GET(unit, nof_cores) == 2) ? 1 : 0);

    max_external_faps = (IRE_TDM_MESH_MC_BITS_MAX - SOC_DNX_DEFS_GET(unit, nof_cores)); 

    *mc_rep =0;
    for (i = 0; i < destid_count; ++i) {
        if (destid_array[i] == my_core0_fap_id){
            *mc_rep |= SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_LOCAL_0_BIT;
        } else if (destid_array[i] == my_core1_fap_id){
            *mc_rep |= SOC_JER2_JER_FABRIC_TDM_MESH_MC_REPLICATION_LOCAL_1_BIT;
        } else { 
            /* dest_id is dest FAP */
            remote_dest = SOC_DNX_FABRIC_GROUP_MODID_UNSET(destid_array[i]);
            if (remote_dest >= 0 && remote_dest < max_external_faps){ 
                *mc_rep |= soc_jer2_jer_fabric_ire_mesh_multicast_replication_dests[remote_dest];
            }
            else {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("dest %d is invalid"),destid_array[i]));
            }
        }
    }

    exit:
      DNXC_FUNC_RETURN;
}


int
jer2_jer_mesh_tdm_multicast_set(
   int                            unit, 
   soc_port_t                     port,
   uint32                         flags, 
   uint32                         destid_count,
   soc_module_t                   *destid_array
   ){
    JER2_ARAD_IRE_TDM_CONFIG_TBL_DATA jer2_arad_ire_tdm_config_tbl_data;
    uint32 mc_rep;

    DNXC_INIT_FUNC_DEFS;

    /* update the IRE table */
    DNX_SAND_IF_ERR_EXIT(jer2_arad_ire_tdm_config_tbl_get_unsafe(unit, port, &jer2_arad_ire_tdm_config_tbl_data ))  ;
    DNXC_IF_ERR_EXIT(soc_jer2_jer_fabric_multicast_rep_bitmap_convert(unit, destid_count, destid_array, &mc_rep));
    jer2_arad_ire_tdm_config_tbl_data.mc_replication = mc_rep; 
    DNX_SAND_IF_ERR_EXIT(jer2_arad_ire_tdm_config_tbl_set_unsafe(unit, port, &jer2_arad_ire_tdm_config_tbl_data));
        
    exit:
        DNXC_FUNC_RETURN;
}

