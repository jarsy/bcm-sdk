#include <soc/mcm/memregs.h>
#include <soc/dpp/SAND/Utils/sand_integer_arithmetic.h>
/* $Id: qax_fabric.c,v 1.96 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

/*************
 * INCLUDES  *
 *************/
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/mem.h>
#include <soc/register.h>
/* { */
#include <soc/dcmn/error.h>
#include <soc/dcmn/dcmn_mem.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/cosq.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/QAX/qax_fabric.h>
#include <soc/dpp/QAX/qax_init.h>
#include <soc/dpp/QAX/qax_mgmt.h>

/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */


/*********************************************************************
* NAME:
*     soc_qax_fabric_force_set
* FUNCTION:
*     DIAG function. Force fabric interface for local / fabric or restore back to operational mode
* INPUT:
*       int   unit - Identifier of the device to access.
*       soc_dpp_fabric_force_t force - enum for requested force mode (local/fabric/restore)
* RETURNS:
*       OK or ERROR indication.
* REMARKS:
*       Relevant for Kalia only. Not supported in qax.
*       Used in mbcm dispatcher.
*********************************************************************/
soc_error_t
  soc_qax_fabric_force_set(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN soc_dpp_fabric_force_t        force
  )
{
    uint32  reg32_val;
    uint8 is_traffic_enabled;
    uint8 is_ctrl_cells_enabled;
    SOCDNX_INIT_FUNC_DEFS;

    /* check if traffic is enabled on NIF/Fabric sides. Writing to ECI_GLOBAL registers under traffic can cause undefined behavior. */
    SOC_SAND_IF_ERR_EXIT(jer_mgmt_enable_traffic_get(unit, &is_traffic_enabled));
    SOC_SAND_IF_ERR_EXIT(arad_mgmt_all_ctrl_cells_enable_get(unit, &is_ctrl_cells_enabled));

    if (is_traffic_enabled || is_ctrl_cells_enabled) {
        LOG_WARN(BSL_LS_SOC_FABRIC, (BSL_META_U(unit,
                                " Warning: fabric force should not be done when traffic is enabled.\nTo disable traffic, use bcm_stk_module_enable and bcm_fabric_control_set with bcmFabricControlCellsEnable parameter.\n")));
    }

    if (force == socDppFabricForceFabric) {

        /*Force traffic routing to Fabric*/
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32_val));
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_FABRICf, 0x1); /*Forcing Fabric*/
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_LOCALf, 0x0); /*Disabling local forcing, to allow local->fabric immediate transition (without restoring first)*/
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32_val));

        /*Force fabric for control cells*/
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            SOCDNX_IF_ERR_EXIT(READ_FCT_FCT_ENABLER_REGISTERr(unit, &reg32_val));
            soc_reg_field_set(unit, FCT_FCT_ENABLER_REGISTERr, &reg32_val, DIS_LCLRTf, 0x1); /*Disables control cells local routing*/
            SOCDNX_IF_ERR_EXIT(WRITE_FCT_FCT_ENABLER_REGISTERr(unit, reg32_val));
        }
    } else if (force == socDppFabricForceLocal) {

        /*Force traffic routing to local Egress*/
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32_val));
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_LOCALf, 0x1); /*Forcing Local*/
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_FABRICf, 0x0); /*Disabling Fabric forcing, to allow fabric->local immediate transition (without restoring first)*/
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32_val));

        /*Allow local route for control cells*/
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            SOCDNX_IF_ERR_EXIT(READ_FCT_FCT_ENABLER_REGISTERr(unit, &reg32_val));
            soc_reg_field_set(unit, FCT_FCT_ENABLER_REGISTERr, &reg32_val, DIS_LCLRTf, 0x0); /*Allow control cells local routing*/
            SOCDNX_IF_ERR_EXIT(WRITE_FCT_FCT_ENABLER_REGISTERr(unit, reg32_val));
        }
    } else if (force == socDppFabricForceRestore) {

        /* Restore default configurations by disabling local / fabric traffic routing forcing */
        SOCDNX_IF_ERR_EXIT(READ_ECI_GLOBAL_GENERAL_CFG_2r(unit, &reg32_val));
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_LOCALf, 0x0); /*Disabling Local forcing*/
        soc_reg_field_set(unit, ECI_GLOBAL_GENERAL_CFG_2r, &reg32_val, FORCE_FABRICf, 0x0); /*Disabling Fabric forcing*/
        SOCDNX_IF_ERR_EXIT(WRITE_ECI_GLOBAL_GENERAL_CFG_2r(unit, reg32_val));

        /*Allow local route for control cells*/
        if (!soc_feature(unit, soc_feature_no_fabric)) {
            SOCDNX_IF_ERR_EXIT(READ_FCT_FCT_ENABLER_REGISTERr(unit, &reg32_val));
            soc_reg_field_set(unit, FCT_FCT_ENABLER_REGISTERr, &reg32_val, DIS_LCLRTf, 0x0); /*Allow control cells local routing*/
            SOCDNX_IF_ERR_EXIT(WRITE_FCT_FCT_ENABLER_REGISTERr(unit, reg32_val));
        }
    } else /*Error checking - force mode is not supported*/{

        cli_out("Option not supported\n");
        cli_out("Supported options are fabric, local, default\n");
        SOCDNX_IF_ERR_EXIT(_SHR_E_UNAVAIL);

    }


exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
*     soc_qax_fabric_multicast_set
* FUNCTION:
*     Setting destination for a specific multicast id in kalia
* INPUT:
*       int             unit            - Identifier of the device to access.
*       soc_multicast_t mc_id           - multicast id
*       uint32          destid_count    - number of destination for this mc_id
*       soc_module_t    *destid_array   - specific destination for replication for this specific mc_id
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Mesh MC mode only.
*********************************************************************/

soc_error_t
soc_qax_fabric_multicast_set(
    SOC_SAND_IN  int                            unit,
    SOC_SAND_IN  soc_multicast_t                mc_id,
    SOC_SAND_IN  uint32                         destid_count,
    SOC_SAND_IN  soc_module_t                   *destid_array
  )
{
    int index, offset, i;
    uint32 mc_rep = 0;
    soc_module_t local_dest;
    uint32 my_core0_fap_id;
    SHR_BITDCLNAME (table_data, 137);
    SOCDNX_INIT_FUNC_DEFS;

    index = mc_id / SOC_JER_FABRIC_MESH_MC_FAP_GROUP_SIZE;
    offset = (mc_id % SOC_JER_FABRIC_MESH_MC_FAP_GROUP_SIZE) * SOC_JER_FABRIC_MESH_MC_REPLICATION_LENGTH;

    SOCDNX_IF_ERR_EXIT(qax_mgmt_system_fap_id_get(unit, &my_core0_fap_id));

    SOCDNX_IF_ERR_EXIT(READ_FDT_IPT_MESH_MCm(unit, MEM_BLOCK_ANY, index, table_data));
    for (i = 0; i < destid_count; ++i) {
        if (destid_array[i] == my_core0_fap_id){
            mc_rep |= SOC_JER_FABRIC_MESH_MC_REPLICATION_LOCAL_0_BIT;
        } else { /*dest_id is dest FAP*/
            local_dest = SOC_DPP_FABRIC_GROUP_MODID_UNSET(destid_array[i]);
            if (local_dest == 0) { /*In Kalia, only modid_set 0 is supported.*/
                mc_rep |= SOC_JER_FABRIC_MESH_MC_REPLICATION_DEST_0_BIT ;
            } else {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("dest %d is invalid"),destid_array[i]));
            }

        }
    }
    SHR_BITCOPY_RANGE(table_data, offset, &mc_rep, 0, SOC_JER_FABRIC_MESH_MC_REPLICATION_LENGTH);
    SOCDNX_IF_ERR_EXIT(WRITE_FDT_IPT_MESH_MCm(unit, MEM_BLOCK_ANY, index, table_data));

exit:
  SOCDNX_FUNC_RETURN;
}



/*********************************************************************
* NAME:
*     soc_qax_fabric_link_config_ovrd
* FUNCTION:
*     Overwriting qax default fabric configuration in case of kalia
* INPUT:
*       int   unit - Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia or QAX mesh (not relevant for QAX stand-alone)
*********************************************************************/
soc_error_t
  soc_qax_fabric_link_config_ovrd(
    int                unit
  )
{
    char* fabric_connect_mode = NULL;
    int is_mesh;
    SOCDNX_INIT_FUNC_DEFS;

    /* current function is called before soc_dpp_str_prop_fabric_connect_mode_get,
       so fabric connect mode isn't updated at this point -> need to get it directly */
    fabric_connect_mode = soc_property_get_str(unit, spn_FABRIC_CONNECT_MODE);
    if (fabric_connect_mode == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Property fabric_connect_mode must be defined")));
    }
    is_mesh = (sal_strcmp(fabric_connect_mode, "MESH") == 0);

    if (SOC_IS_KALIA(unit) || is_mesh) {
        SOC_DPP_DEFS_SET(unit, nof_fabric_links, 16);
        SOC_DPP_DEFS_SET(unit, nof_instances_fmac, 4);
        SOC_DPP_DEFS_SET(unit, first_fabric_link_id, 0);
        SOC_DPP_DEFS_SET(unit, nof_sch_active_links, 16);
        SOC_DPP_DEFS_SET(unit, nof_logical_ports, 528);

        SOC_DPP_IMP_DEFS_SET(unit, rci_severity_factor_1, 5);
        SOC_DPP_IMP_DEFS_SET(unit, rci_severity_factor_2, 6);
        SOC_DPP_IMP_DEFS_SET(unit, rci_severity_factor_3, 7);
        SOC_DPP_IMP_DEFS_SET(unit, rci_threshold_nof_congested_links, 8);
    } 

exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
*     soc_qax_fabric_cosq_control_backward_flow_control_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 03 2015
* FUNCTION:
*     Enable / disable backwards flow control on supported fifos
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          port -
*     gport number.
*  SOC_SAND_IN int                                   enable -
*     Whether to enable / disable the feature.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*
* REMARKS:
*     Used in mbcm dispatcher.
*     For Kalia only (not relevant for QAX)
*********************************************************************/
soc_error_t
  soc_qax_fabric_cosq_control_backward_flow_control_set(
      SOC_SAND_IN int                                   unit,
      SOC_SAND_IN soc_gport_t                           port,
      SOC_SAND_IN int                                   enable,
      SOC_SAND_IN soc_dpp_cosq_gport_egress_core_fifo_t fifo_type
  )
{
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;

    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOC_MSG("QUX doesn't support this function!")));
    }
    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_local_mcast:
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_MESHMC_FC_ENr(unit, &reg_val));
        soc_reg_field_set(unit, FDA_FDA_MESHMC_FC_ENr, &reg_val, MESHMC_FC_ENf, enable);
        SOCDNX_IF_ERR_EXIT(WRITE_FDA_FDA_MESHMC_FC_ENr(unit, reg_val));
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast:
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_mcast:
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_tdm:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("backwards flow control for cosq egress fabric ucast / mcast / tdm fifos are not supported in QAX\n")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("invalid argument fifo_type\n")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}

soc_error_t
  soc_qax_fabric_cosq_control_backward_flow_control_get(
      SOC_SAND_IN int                                   unit,
      SOC_SAND_IN soc_gport_t                           port,
      SOC_SAND_OUT int                                  *enable,
      SOC_SAND_IN soc_dpp_cosq_gport_egress_core_fifo_t fifo_type
  )
{
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;
    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOC_MSG("QUX doesn't support this function!")));
    }
    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_local_mcast:
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_MESHMC_FC_ENr(unit, &reg_val));
        *enable = soc_reg_field_get(unit, FDA_FDA_MESHMC_FC_ENr, reg_val, MESHMC_FC_ENf);
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast:
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_mcast:
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_tdm:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("backwards flow control for cosq egress fabric ucast / mcast / tdm fifos are not supported in QAX\n")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("invalid argument fifo_type\n")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*   soc_qax_update_params_based_on_FDA_fifos_weight
* TYPE:
*   PROC
* DATE:
*   Jun 06 2016
* FUNCTION:
*     Internal
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*
* REMARKS:
*     not relevant for QAX stand-alone.
*     Not called. May be useful in the future.
*     Should be called from
*     soc_qax_fabric_egress_core_cosq_gport_sched_set, when changing weight of a pipe
*     (case soc_dpp_cosq_gport_egress_core_fifo_fabric_generic_pipe).
*     Should be STATIC.
*********************************************************************/

soc_error_t
    soc_qax_update_params_based_on_FDA_fifos_weight (
      SOC_SAND_IN  int                                unit)
{
    uint32 nof_pipes, pipe, min_pipe_weight, min_pipe_weight_tmp; /*smaller pipe weight value actually means larger weight in WFQ*/
    uint32 FIELD_4_22_len, FIELD_4_22_max_val = 0;
    /* variables for FDA_FDA_EGQ_WFQr */
    soc_reg_above_64_val_t reg_above_64_val;
    const soc_field_t fields[3] = {EGQ_WFQ_WEIGHT_FABFIF_0f, EGQ_WFQ_WEIGHT_FABFIF_1f, EGQ_WFQ_WEIGHT_FABFIF_2f};
    uint32 fields_vals[3];
    /* variables for FDR_REG_0100_1,2,3r and FDR_REG_011D_1,2,3r */
    const soc_reg_t regs_0100[3] = {FDR_REG_0100_1r, FDR_REG_0100_2r, FDR_REG_0100_3r};
    const soc_reg_t regs_011D[3] = {FDR_REG_011D_1r, FDR_REG_011D_2r, FDR_REG_011D_3r};
    uint32 reg_val;
    uint32 field_val;

    SOCDNX_INIT_FUNC_DEFS;

    nof_pipes = SOC_DPP_CONFIG(unit)->arad->init.fabric.fabric_pipe_map_config.nof_pipes;

    /* get FDA EGQ WFQ FIFOs weights. FDA FIFOs are one-to-one mapping of fabric pipes */
    SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_WFQr(unit, 0, reg_above_64_val));
    for (pipe = 0; pipe < nof_pipes; ++pipe) {
        fields_vals[pipe] = soc_reg_above_64_field32_get(unit, FDA_FDA_EGQ_WFQr, reg_above_64_val, fields[pipe]);
    }

    switch (nof_pipes) {
    case 1:
        min_pipe_weight = fields_vals[0];
        break;
    case 2:
        min_pipe_weight = SOC_SAND_MIN(fields_vals[0], fields_vals[1]);
        break;
    case 3:
        min_pipe_weight_tmp = SOC_SAND_MIN(fields_vals[0], fields_vals[1]);
        min_pipe_weight = SOC_SAND_MIN(min_pipe_weight_tmp, fields_vals[2]);
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOC_MSG("number of pipes %d in invalid"), nof_pipes));
    }

    /*we can have overflow in this field (in calculations below)*/
    FIELD_4_22_len = soc_reg_field_length(unit, FDR_REG_011D_1r, FIELD_4_22f);
    SHR_BITSET_RANGE(&FIELD_4_22_max_val, 0, FIELD_4_22_len);

    for (pipe = 0; pipe < nof_pipes; ++pipe) {
        /* write to FDR_REG_0100_r and FDR_REG_011D_r registers per pipe, the normalized value based on FDA FIFOs weights:
           <value for even pipes> * <pipe weight> / <min(pipes weights)> */

        field_val = 0x800 * (fields_vals[pipe] / min_pipe_weight);
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, regs_0100[pipe], REG_PORT_ANY, 0, &reg_val));
        soc_reg_field_set(unit, regs_0100[pipe], &reg_val, FIELD_0_18f, field_val);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, regs_0100[pipe], REG_PORT_ANY, 0, reg_val));

        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, regs_011D[pipe], REG_PORT_ANY, 0, &reg_val));
        field_val = soc_reg_field_get(unit, regs_011D[pipe], reg_val, FIELD_0_0f);
        if (field_val) { /* if feature is enabled */
            field_val = 0x1800 * (fields_vals[pipe] / min_pipe_weight);
            if (field_val > FIELD_4_22_max_val) { /*avoid overflow*/
                field_val = FIELD_4_22_max_val;
            }
            soc_reg_field_set(unit, regs_011D[pipe], &reg_val, FIELD_4_22f, field_val);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, regs_011D[pipe], REG_PORT_ANY, 0, reg_val));
        }
    }

    exit:
      SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_qax_fabric_egress_core_cosq_gport_sched_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 03 2015
* FUNCTION:
*     Set WFQ weight on supported fifos.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          pipe -
*     Which pipe to access.
*  SOC_SAND_IN int                                  weight -
*     Weight value to configure.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*
* REMARKS:
*     Used in mbcm dispatcher.
*     For Kalia only (not relevant for QAX)
*********************************************************************/
soc_error_t
  soc_qax_fabric_egress_core_cosq_gport_sched_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_IN  int                                weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t   fifo_type)
{
    soc_reg_above_64_val_t above_64_reg_val;
    const soc_field_t fields[3] = {EGQ_WFQ_WEIGHT_FABFIF_0f, EGQ_WFQ_WEIGHT_FABFIF_1f, EGQ_WFQ_WEIGHT_FABFIF_2f};

    SOCDNX_INIT_FUNC_DEFS;

    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOC_MSG("QUX doesn't support this function!")));
    }
    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_generic_pipe:
        if ((pipe < 0) || (pipe >= (sizeof (fields)/sizeof(soc_field_t)))){
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Generic pipe must be between 0 and %d (including)\n"), (sizeof (fields)/sizeof(soc_field_t)-1)));
        }
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_WFQr(unit, 0, above_64_reg_val));
        soc_reg_above_64_field32_set(unit, FDA_FDA_EGQ_WFQr, above_64_reg_val, fields[pipe], weight);
        SOCDNX_IF_ERR_EXIT(WRITE_FDA_FDA_EGQ_WFQr(unit, 0, above_64_reg_val));
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_local_mcast:
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_WFQr(unit, 0, above_64_reg_val));
        soc_reg_above_64_field32_set(unit, FDA_FDA_EGQ_WFQr, above_64_reg_val, EGQ_N_WFQ_WEIGHT_MESHMCf, weight);
        SOCDNX_IF_ERR_EXIT(WRITE_FDA_FDA_EGQ_WFQr(unit, 0, above_64_reg_val));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("invalid argument fifo_type\n")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}


soc_error_t
  soc_qax_fabric_egress_core_cosq_gport_sched_get(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_OUT int                                *weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t   fifo_type
  )
{
    soc_reg_above_64_val_t above_64_reg_val;
    const soc_field_t fields[3] = {EGQ_WFQ_WEIGHT_FABFIF_0f, EGQ_WFQ_WEIGHT_FABFIF_1f, EGQ_WFQ_WEIGHT_FABFIF_2f};

    SOCDNX_INIT_FUNC_DEFS;

    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOC_MSG("QUX doesn't support this function!")));
    }
    SOCDNX_NULL_CHECK(weight);
    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_generic_pipe:
        if ((pipe < 0) || (pipe >= (sizeof (fields)/sizeof(soc_field_t)))){
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("Generic pipe must be between 0 and %d (including)\n"), (sizeof (fields)/sizeof(soc_field_t)-1)));
        }
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_WFQr(unit, 0, above_64_reg_val));
        *weight = soc_reg_above_64_field32_get(unit, FDA_FDA_EGQ_WFQr, above_64_reg_val, fields[pipe]);
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_local_mcast:
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_WFQr(unit, 0, above_64_reg_val));
        *weight = soc_reg_above_64_field32_get(unit, FDA_FDA_EGQ_WFQr, above_64_reg_val, EGQ_N_WFQ_WEIGHT_MESHMCf);
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("invalid argument fifo_type\n")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
*     soc_qax_cosq_gport_sched_set/get
* FUNCTION:
*     Configuration of weight for WFQs in fabric pipes:
*     all, ingress, egress.
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  int                                pipe -
*     Which pipe's weight to configure (0,1,2)
*  SOC_SAND_IN/SOC_SAND_OUT  int/int*              weight -
*     value to configure/retrieve pipe's weight
*  SOC_SAND_IN  soc_dpp_cosq_gport_fabric_pipe_t   fabric_pipe_type -
*     type of fabric pipe to configure (all, ingress, egress)
*     Note: egress is not legal argument for QAX. "All" argument is actually identical to "ingress" argument
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for QAX)
*********************************************************************/
soc_error_t
  soc_qax_cosq_gport_sched_set(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_IN  int                                weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_fabric_pipe_t   fabric_pipe_type
  )
{

    soc_reg_above_64_val_t above_64_reg_val;
    soc_field_t field;

    SOCDNX_INIT_FUNC_DEFS;

    if (fabric_pipe_type != soc_dpp_cosq_gport_fabric_pipe_egress) { /*ingress or all*/
        /* configure ingress part (FDT) */
        switch (pipe) {
        case 0:
            field = IPT_0_WFQ_CTX_0_WEIGHTf;
            break;
        case 1:
            field = IPT_0_WFQ_CTX_1_WEIGHTf;
            break;
        case 2:
            field = IPT_0_WFQ_CTX_2_WEIGHTf;
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unit %d, invalid pipe index %d\n"), unit ,pipe));
            break;
        }
        SOCDNX_IF_ERR_EXIT(READ_FDT_IPT_0_WFQ_CONFIGr(unit, above_64_reg_val));
        soc_reg_above_64_field32_set(unit, FDT_IPT_0_WFQ_CONFIGr, above_64_reg_val, field, weight);
        SOCDNX_IF_ERR_EXIT(WRITE_FDT_IPT_0_WFQ_CONFIGr(unit, above_64_reg_val));
    }else{
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("fabric_pipe_egress is not legal argument for QAX\n")));
    }

exit:
  SOCDNX_FUNC_RETURN;
}


soc_error_t
  soc_qax_cosq_gport_sched_get(
    SOC_SAND_IN  int                                unit,
    SOC_SAND_IN  int                                pipe,
    SOC_SAND_OUT int*                               weight,
    SOC_SAND_IN  soc_dpp_cosq_gport_fabric_pipe_t   fabric_pipe_type
  )
{
    soc_reg_above_64_val_t reg_above_64_val;
    soc_field_t field;

    SOCDNX_INIT_FUNC_DEFS;

    if (weight == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unit %d, invalid address %#x\n"), unit ,weight));
    }

    if (fabric_pipe_type != soc_dpp_cosq_gport_fabric_pipe_egress) { /*ingress or all*/
        switch (pipe) {
        case 0:
            field = IPT_0_WFQ_CTX_0_WEIGHTf;
            break;
        case 1:
            field = IPT_0_WFQ_CTX_1_WEIGHTf;
            break;
        case 2:
            field = IPT_0_WFQ_CTX_2_WEIGHTf;
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unit %d, invalid pipe index %d\n"), unit ,pipe));
            break;
        }
        SOCDNX_IF_ERR_EXIT(READ_FDT_IPT_0_WFQ_CONFIGr(unit, reg_above_64_val));
        *weight = soc_reg_above_64_field32_get(unit, FDT_IPT_0_WFQ_CONFIGr, reg_above_64_val, field);
    } else {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("fabric_pipe_egress is not legal argument for QAX\n")));
    }

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_qax_fabric_cosq_gport_priority_drop_threshold_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 10 2015
* FUNCTION:
*     Set priority drop threshold on supported fifos.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          gport -
*     gport number.
*  SOC_SAND_IN  soc_cosq_threshold_t                *threshold_val -
*     sturuct which contains the threshold value
*     to configure / retreive.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*     NOTE: Only soc_dpp_cosq_gport_egress_core_fifo_local_mcast fifo_type is supported in QAX!
*
* REMARKS:
*     Used in mbcm dispatcher.
*     For Kalia only (not relevant for QAX)
*********************************************************************/
soc_error_t
  soc_qax_fabric_cosq_gport_priority_drop_threshold_set(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  soc_gport_t                            gport,
    SOC_SAND_IN  soc_cosq_threshold_t                   *threshold,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  )
{
    int i;
    soc_field_t field;
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;

    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOC_MSG("QUX doesn't support this function!")));
    }
    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_local_mcast:
        if (threshold->dp == 0) {
            field = EGQ_MESHMCFIF_PRIO_0_DROP_THRf;
        } else if (threshold->dp == 1) {
            field = EGQ_MESHMCFIF_PRIO_1_DROP_THRf;
        } else {
            field = EGQ_MESHMCFIF_PRIO_2_DROP_THRf;
        }
        for (i = 0; i < SOC_DPP_DEFS_GET(unit, nof_cores); ++i) {
            SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_MESHMC_PRIO_DROP_THRr(unit, i, &reg_val));
            soc_reg_field_set(unit, FDA_FDA_EGQ_MESHMC_PRIO_DROP_THRr, &reg_val, field, threshold->value);
            SOCDNX_IF_ERR_EXIT(WRITE_FDA_FDA_EGQ_MESHMC_PRIO_DROP_THRr(unit, i, reg_val));
        }
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast:
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_mcast:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("priority drop threshold for soc_dpp_cosq_gport_egress_core_fifo_fabric_mcast / ucast are not supported in QAX")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unsupported fifo type")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}

soc_error_t
  soc_qax_fabric_cosq_gport_priority_drop_threshold_get(
    SOC_SAND_IN  int                                    unit,
    SOC_SAND_IN  soc_gport_t                            gport,
    SOC_SAND_INOUT  soc_cosq_threshold_t                *threshold,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  )
{
    soc_field_t field;
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;
    if (soc_feature(unit, soc_feature_no_fabric)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL,
                             (_BSL_SOC_MSG("QUX doesn't support this function!")));
    }
    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_local_mcast:
        if (threshold->dp == 0) {
            field = EGQ_MESHMCFIF_PRIO_0_DROP_THRf;
        } else if (threshold->dp == 1) {
            field = EGQ_MESHMCFIF_PRIO_1_DROP_THRf;
        } else {
            field = EGQ_MESHMCFIF_PRIO_2_DROP_THRf;
        }
        SOCDNX_IF_ERR_EXIT(READ_FDA_FDA_EGQ_MESHMC_PRIO_DROP_THRr(unit, 0, &reg_val));
        threshold->value = soc_reg_field_get(unit, FDA_FDA_EGQ_MESHMC_PRIO_DROP_THRr, reg_val, field);
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast:
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_mcast:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("priority drop threshold for soc_dpp_cosq_gport_egress_core_fifo_fabric_mcast / ucast are not supported in QAX")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unsupported fifo type")));
        break;
    }
exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_qax_fabric_cosq_gport_rci_threshold_set / get
* TYPE:
*   PROC
* DATE:
*   Dec 10 2015
* FUNCTION:
*     Set / get rci threshold on supported fifos.
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN soc_gport_t                          gport -
*     gport number.
*  SOC_SAND_IN  int                                 threshold_val -
*     Threshold value to configure.
*  SOC_SAND_IN int                                  fifo_type -
*     Type of fifo to configure
*     Note: For QAX, only soc_dpp_cosq_gport_egress_core_fifo_local_ucast is supported.
*
* REMARKS:
*     Used in mbcm dispatcher.
*     For Kalia only (not relevant for QAX)
*********************************************************************/

soc_error_t
  soc_qax_fabric_cosq_gport_rci_threshold_set(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  soc_gport_t            gport,
    SOC_SAND_IN  int                    threshold_val,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  )
{

    soc_reg_above_64_val_t reg_above_64_val;
    SOCDNX_INIT_FUNC_DEFS;

    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_local_ucast:
        SOCDNX_IF_ERR_EXIT(READ_TXQ_LOCAL_FIFO_CFGr(unit, reg_above_64_val));
        soc_reg_above_64_field32_set(unit, TXQ_LOCAL_FIFO_CFGr, reg_above_64_val, SRAM_DTQ_LOC_GEN_RCI_THf, threshold_val);
        SOCDNX_IF_ERR_EXIT(WRITE_TXQ_LOCAL_FIFO_CFGr(unit, reg_above_64_val));
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("rci threshold for soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast is not supported in QAX")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unsupported fifo type")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}


soc_error_t
  soc_qax_fabric_cosq_gport_rci_threshold_get(
    SOC_SAND_IN  int                    unit,
    SOC_SAND_IN  soc_gport_t            gport,
    SOC_SAND_OUT  int                    *threshold_val,
    SOC_SAND_IN  soc_dpp_cosq_gport_egress_core_fifo_t  fifo_type
  )
{

    soc_reg_above_64_val_t reg_above_64_val;
    SOCDNX_INIT_FUNC_DEFS;

    switch (fifo_type) {
    case soc_dpp_cosq_gport_egress_core_fifo_local_ucast:
        SOCDNX_IF_ERR_EXIT(READ_TXQ_LOCAL_FIFO_CFGr(unit, reg_above_64_val));
        *threshold_val = soc_reg_above_64_field32_get(unit, TXQ_LOCAL_FIFO_CFGr, reg_above_64_val, SRAM_DTQ_LOC_GEN_RCI_THf);
        break;
    case soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("rci threshold for soc_dpp_cosq_gport_egress_core_fifo_fabric_ucast is not supported in QAX")));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("unsupported fifo type")));
        break;
    }

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_qax_fabric_priority_bits_mapping_to_fdt_index_get
* TYPE:
*   PROC
* DATE:
*   Dec 13 2015
* FUNCTION:
*     configure cell attributes(is_hp, tc, dp, is_mc)
*     to an index in TXQ_PRIORITY_BITS_MAPPING_2_FDT table
* INPUT:
*  SOC_SAND_IN  int                                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                             tc -
*     traffic class
*  SOC_SAND_IN  uint32                             dp -
*     drop precedence
*  SOC_SAND_IN  uint32                              flags -
*     relevant flags for cell (is_mc, is_hp)
* OUTPUT:
*  SOC_SAND_OUT uint32                              *index
*     retrieved entry index to TXQ_PRIORITY_BITS_MAPPING_2_FDT
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   STATIC function
*   For Kalia only (not relevant for QAX)
*********************************************************************/

STATIC soc_error_t
soc_qax_fabric_priority_bits_mapping_to_fdt_index_get(
    SOC_SAND_IN  int                        unit,
    SOC_SAND_IN  uint32                     tc,
    SOC_SAND_IN  uint32                     dp,
    SOC_SAND_IN  uint32                     flags,
    SOC_SAND_OUT uint32                     *index
  )
{
    uint32 is_mc  = 0, is_hp = 0;
    SOCDNX_INIT_FUNC_DEFS;

    *index = 0;

    is_hp  = (flags & SOC_FABRIC_QUEUE_PRIORITY_HIGH_ONLY) ? 1 : 0 ;
    is_mc  = (flags & SOC_FABRIC_PRIORITY_MULTICAST)       ? 1 : 0 ;

    *index  |=  ((is_hp     << SOC_QAX_FABRIC_PRIORITY_NDX_IS_HP_OFFSET)  & SOC_QAX_FABRIC_PRIORITY_NDX_IS_HP_MASK  )|
                ((tc        << SOC_QAX_FABRIC_PRIORITY_NDX_TC_OFFSET)     & SOC_QAX_FABRIC_PRIORITY_NDX_TC_MASK     )|
                ((dp        << SOC_QAX_FABRIC_PRIORITY_NDX_DP_OFFSET)     & SOC_QAX_FABRIC_PRIORITY_NDX_DP_MASK     )|
                ((is_mc     << SOC_QAX_FABRIC_PRIORITY_NDX_IS_MC_OFFSET)  & SOC_QAX_FABRIC_PRIORITY_NDX_IS_MC_MASK  );

    SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_qax_fabric_priority_set / set
* TYPE:
*   PROC
* DATE:
*   Dec 13 2015
* FUNCTION:
*     Set / Get fabric priority according to:
*     traffic_class, queue_type: hc/lc (flags), dp(color).
* INPUT:
*  SOC_SAND_IN  int                                unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                             tc -
*     traffic class
*  SOC_SAND_IN  uint32                             dp -
*     drop precedence
*  SOC_SAND_IN  uint32                             flags -
*     relevant flags for cell (is_mc, is_hp)
*  SOC_SAND_IN/OUT   int/int*                      fabric_priority -
*     fabric priority to set/ get in TXQ_PRIORITY_BITS_MAPPING_2_FDT
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia only (not relevant for QAX)
*********************************************************************/

soc_error_t
soc_qax_fabric_priority_set(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32             tc,
    SOC_SAND_IN  uint32             dp,
    SOC_SAND_IN  uint32             flags,
    SOC_SAND_IN  int                fabric_priority
  )
{
    uint32 index;
    uint32 tdm_min_prio;
    SOCDNX_INIT_FUNC_DEFS;

    tdm_min_prio = SOC_DPP_CONFIG(unit)->arad->init.fabric.fabric_tdm_priority_min;
    /*validate fabric_priority*/
    if ((fabric_priority < 0) || (fabric_priority >= tdm_min_prio)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("fabric PKT priority is %d but must be between 0 and TDM min priority (that was set to to %d) \n"), fabric_priority, tdm_min_prio));
    }
    /*get index for TXQ_PRIORITY_BITS_MAPPING_2_FDT*/
    SOCDNX_IF_ERR_EXIT(soc_qax_fabric_priority_bits_mapping_to_fdt_index_get(unit, tc, dp, flags, &index));
    /*fill table with fabric priority in the index found*/
    SOCDNX_IF_ERR_EXIT(dcmn_fill_partial_table_with_entry(unit, TXQ_PRIORITY_BITS_MAPPING_2_FDTm,
                                                          0, 0, MEM_BLOCK_ALL, index, index, &fabric_priority));

exit:
  SOCDNX_FUNC_RETURN;
}


soc_error_t
soc_qax_fabric_priority_get(
    SOC_SAND_IN  int                unit,
    SOC_SAND_IN  uint32             tc,
    SOC_SAND_IN  uint32             dp,
    SOC_SAND_IN  uint32             flags,
    SOC_SAND_OUT  int                *fabric_priority
  )
{
    uint32 index, val = 0;
    SOCDNX_INIT_FUNC_DEFS;

    /*get index for TXQ_PRIORITY_BITS_MAPPING_2_FDT*/
    SOCDNX_IF_ERR_EXIT(soc_qax_fabric_priority_bits_mapping_to_fdt_index_get(unit, tc, dp, flags, &index));
    /*retrieve table entry in the index found*/
    SOCDNX_IF_ERR_EXIT(READ_TXQ_PRIORITY_BITS_MAPPING_2_FDTm(unit, MEM_BLOCK_ANY, index, &val));
    *fabric_priority = val;

exit:
  SOCDNX_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     soc_qax_fabric_queues_info_get
* TYPE:
*   PROC
* DATE:
*   Jun 8 2016
* FUNCTION:
*     Get DTQ and PDQ (DQCQ/DBLF) queues status
* INPUT:
*  SOC_SAND_IN  int                         unit -
*     Identifier of the device to access.
*  SOC_SAND_OUT soc_dpp_fabric_queues_info_t*
*                                           queues_info-
*     struct to hold the status.
* REMARKS:
*   Used in mbcm dispatcher.
*********************************************************************/
uint32
  soc_qax_fabric_queues_info_get(
    SOC_SAND_IN  int                             unit,
    SOC_SAND_OUT soc_dpp_fabric_queues_info_t    *queues_info
  )
{
    uint32 reg32_val, dtq_mode;
    uint64 reg64_val;
    soc_reg_above_64_val_t  reg_above64_val;
    int i, num_of_DTQs;

    SOCDNX_INIT_FUNC_DEFS;

    /******get DTQ info******/

    /*Get num of used DTQs (out of 6) for FAP's configuration*/
    if (SOC_DPP_CONFIG(unit)->arad->init.fabric.connect_mode == ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP) { /*Fabric DTQs hold traffic to go to FE, so not configured in single FAP*/
        num_of_DTQs = 0;
    } else if (SOC_DPP_IS_MESH(unit)){
        num_of_DTQs = 6; /*it's actually 4 queues. 2 queues [2,3] are skipped in MESH*/
    } else {
        SOCDNX_IF_ERR_EXIT(READ_TXQ_TXQ_GENERAL_CONFIGURATIONr(unit, &reg32_val));
        dtq_mode = soc_reg_field_get(unit, TXQ_TXQ_GENERAL_CONFIGURATIONr, reg32_val, DTQ_MODEf);
        num_of_DTQs = (dtq_mode + 1)*2; /*DTQ mode can be 0/1/2 -> 1/2/3 queues x [SRAM, DRAM]*/
    }

    /*avoid reading more TXQ_DTQ_STATUSr numels than exist and beyond max index in <queues_info->soc_dpp_fabric_dtq_info> array defined in fabric.h*/
    if(num_of_DTQs > SOC_DPP_MAX_NUM_OF_FABRIC_DTQS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("error in num_of_DTQs calculation or SOC_DPP_MAX_NUM_OF_FABRIC_DTQS macro")));
    }

    for (i = 0; i < num_of_DTQs; ++i) {
        if (SOC_DPP_IS_MESH(unit) && ((i == 2) || (i == 3))) { /*DTQs 2,3 are not used in Mesh*/
            continue;
        }
        SOCDNX_IF_ERR_EXIT(READ_TXQ_DTQ_STATUSr(unit, i, &reg64_val));
        queues_info->soc_dpp_fabric_dtq_info[i].soc_dpp_dtq_occ_val = soc_reg64_field32_get(unit, TXQ_DTQ_STATUSr, reg64_val, DTQ_N_WORD_CNTf);
        queues_info->soc_dpp_fabric_dtq_info[i].soc_dpp_dtq_min_occ_val = soc_reg64_field32_get(unit, TXQ_DTQ_STATUSr, reg64_val, DTQ_N_WORD_CNT_MINf);
        queues_info->soc_dpp_fabric_dtq_info[i].soc_dpp_dtq_max_occ_val = soc_reg64_field32_get(unit, TXQ_DTQ_STATUSr, reg64_val, DTQ_N_WORD_CNT_MAXf);
        queues_info->soc_dpp_fabric_dtq_info[i].is_valid = 1;
    }

    /*avoid reading/writing beyond max index in <queues_info->soc_dpp_local_dtq_info> array defined in fabric.h*/
    if (SOC_REG_NUMELS(unit, TXQ_LTQ_STATUSr) > SOC_DPP_MAX_NUM_OF_LOCAL_DTQS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOC_MSG("error in SOC_DPP_MAX_NUM_OF_LOCAL_DTQS define")));
    }

    /*2 local DTQs- one SRAM and one DRAM*/
    for (i = 0; i < SOC_REG_NUMELS(unit, TXQ_LTQ_STATUSr); ++i) {

        SOCDNX_IF_ERR_EXIT(READ_TXQ_DTQ_STATUSr(unit, i, &reg64_val));
        queues_info->soc_dpp_local_dtq_info[i].soc_dpp_dtq_occ_val = soc_reg64_field32_get(unit, TXQ_LTQ_STATUSr, reg64_val, LTQ_N_WORD_CNTf);
        queues_info->soc_dpp_local_dtq_info[i].soc_dpp_dtq_min_occ_val = soc_reg64_field32_get(unit, TXQ_LTQ_STATUSr, reg64_val, LTQ_N_WORD_CNT_MINf);
        queues_info->soc_dpp_local_dtq_info[i].soc_dpp_dtq_max_occ_val = soc_reg64_field32_get(unit, TXQ_LTQ_STATUSr, reg64_val, LTQ_N_WORD_CNT_MAXf);
        queues_info->soc_dpp_local_dtq_info[i].is_valid = 1;
    }

    /******get DQCF/DBLF (PDQ) info******/

    SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, PTS_FIFOS_MAX_OCC_0r, REG_PORT_ANY, 0, reg_above64_val));

    queues_info->soc_dpp_fabric_sram_dqcf_max_occ_val.info = soc_reg_above_64_field32_get(unit, PTS_FIFOS_MAX_OCC_0r, reg_above64_val, SRAM_DQCF_MAX_OCf);
    queues_info->soc_dpp_fabric_sram_dqcf_max_occ_ctx.info = soc_reg_above_64_field32_get(unit, PTS_FIFOS_MAX_OCC_0r, reg_above64_val, SRAM_DQCQ_MAX_OC_CNTXf);
    queues_info->soc_dpp_fabric_dram_dblf_max_occ_val.info = soc_reg_above_64_field32_get(unit, PTS_FIFOS_MAX_OCC_0r, reg_above64_val, DRAM_DBLF_MAX_OCf);
    queues_info->soc_dpp_fabric_dram_dblf_max_occ_ctx.info = soc_reg_above_64_field32_get(unit, PTS_FIFOS_MAX_OCC_0r, reg_above64_val, DRAM_DBLF_MAX_OC_CNTXf);

    queues_info->soc_dpp_fabric_sram_dqcf_max_occ_val.is_valid = 1;
    queues_info->soc_dpp_fabric_sram_dqcf_max_occ_ctx.is_valid = 1;
    queues_info->soc_dpp_fabric_dram_dblf_max_occ_val.is_valid = 1;
    queues_info->soc_dpp_fabric_dram_dblf_max_occ_ctx.is_valid = 1;

exit:
    SOCDNX_FUNC_RETURN;
}


/*********************************************************************
* NAME:
*     qax_fabric_pcp_dest_mode_config_set
* TYPE:
*   PROC
* DATE:
*   Dec 16 2015
* FUNCTION:
*     Enables set / get operations on fabric-pcp (packet cell packing)
*     per destination device.
*     there are three supported pcp modes:
*       - 0- No Packing
*       - 1- Simple Packing
*       - 2- Continuous Packing
* INPUT:
*  SOC_SAND_IN  int                         unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  uint32                      flags-
*  SOC_SAND_IN  uint32                  	modid-
*     Id of destination device			
*  SOC_SAND_IN/OUT unit32/uint32*     		pcp_mode-
*     mode of pcp to set/get.
* REMARKS:
*   Used in mbcm dispatcher.
*   For Kalia FE mode only (not MESH).
*********************************************************************/
soc_error_t
  qax_fabric_pcp_dest_mode_config_set(
    SOC_SAND_IN int              unit,
    SOC_SAND_IN uint32           flags,
    SOC_SAND_IN uint32           modid,
    SOC_SAND_IN uint32           pcp_mode
  )
{
    SHR_BITDCLNAME (fdt_data, 137);
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_FDT_IPT_MESH_MCm(unit, MEM_BLOCK_ANY, modid, fdt_data));
    SHR_BITCOPY_RANGE(fdt_data, 6, &pcp_mode, 0, 2);
    SOCDNX_IF_ERR_EXIT(WRITE_FDT_IPT_MESH_MCm(unit, MEM_BLOCK_ALL, modid, fdt_data));


exit:
    SOCDNX_FUNC_RETURN;
}

/*
* Function to check if any of the Fabric Quads are disabled and according to that sets the BROADCAST_ID and LAST_IN_CHAIN for the FSRD.
* Not applied for QAX, empty function to avoid errors form the MBCM dispatcher.
*/
int
soc_qax_brdc_fsrd_blk_id_set(int unit){

    SOCDNX_INIT_FUNC_DEFS;

    /*Do nothing*/

    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

#include <soc/dpp/SAND/Utils/sand_footer.h>
