#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_fabric.c,v 1.96 Broadcom SDK $
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
#include <soc/mem.h>
/* { */

#include <soc/dnxc/legacy/error.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/TMC/tmc_api_framework.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/fabric.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>
#include <soc/dnx/legacy/ARAD/arad_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_api_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_defs.h>
#include <soc/dnx/legacy/mbcm.h>

#include <soc/dnxc/legacy/fabric.h>

#include <bcm/port.h>

#include <sal/compiler.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */
/* Arad connect mode definitions */
#define JER2_ARAD_FBC_CONNECT_FE_VAL_1        (0x0)
#define JER2_ARAD_FBC_CONNECT_FE_MULT_VAL_1   (0x0)

#define JER2_ARAD_FBC_CONNECT_FE_VAL_2        (0x0)
#define JER2_ARAD_FBC_CONNECT_FE_MULT_VAL_2   (0x0)
#define JER2_ARAD_FBC_CONNECT_BACK2BACK_VAL_2 (0x1)
#define JER2_ARAD_FBC_CONNECT_MESH_VAL_3      (0x1)
#define JER2_ARAD_FBC_CONNECT_MESH_BYPASS_TDM (0x0)

#define JER2_ARAD_FABRIC_LINK_FC_ENABLE_MODE_ON_VAL  (1)
#define JER2_ARAD_FABRIC_LINK_FC_ENABLE_MODE_OFF_VAL (0)



#define JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_CONGESTION_LEVEL_MAX    (0x3ff)

/*GCI Leaky Bucket*/
#define JER2_ARAD_FABRIC_GCI_BUCKET_MAX                          (0xff)

#define JER2_ARAD_FABRIC_LLFC_RX_MAX                             (127)

/*RCI Params*/
#define JER2_ARAD_FABRIC_RCI_THRESHOLD_MAX                       (127)
#define JER2_ARAD_FABRIC_RCI_THRESHOLD_SINGLE_PIPE_DEFAULT       (64)
#define JER2_ARAD_FABRIC_RCI_THRESHOLD_DUAL_PIPE_DEFAULT         (32)
#define JER2_ARAD_FABRIC_RCI_INC_VAL_MAX                         (0x7f)


/*Empty cell size*/
#define JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_MIN                     (64)
#define JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_MAX                     (127)
#define JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_REMOTE_LINK_REPEATER    (84)



#define JER2_ARAD_MAX_BUCKET_FILL_RATE                           0xB 

#define JER2_ARAD_FABRIC_ALDWP_FAP_OFFSET                        (3)


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

#define DNX_SAND_FAILURE(_sand_ret) \
    ((dnx_handle_sand_result(_sand_ret)) < 0)

/* } */

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
*    jer2_arad_fabric_regs_init
* FUNCTION:
*   Initialization of the Arad blocks configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_fabric_regs_init(
    DNX_SAND_IN  int       unit
  )
{
    DNXC_INIT_FUNC_DEFS;

    /*initialize common blocks*/
    DNXC_IF_ERR_EXIT(jer2_arad_fabric_common_regs_init(unit));

    /*initialize features unique to jer2_arad*/
    /*Drop CRC errors packtes - the last CRC error packet read from the DRAM will be deleted*/
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FDT_FDT_ENABLER_REGISTERr, REG_PORT_ANY, 0, DEL_CRC_PKTf,  0x1));

exit:
  DNXC_FUNC_RETURN;

}


/*********************************************************************
* NAME:
*     jer2_arad_fabric_common_regs_init
* FUNCTION:
*   Initialization of the common blocks from JER2_ARAD, JER2_JERICHO configured in this module.
*   This function directly accesses registers/tables for
*   initializations that are not covered by API-s
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
  jer2_arad_fabric_common_regs_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32
    byte_mode,
    period,
    llfc_enabler_mode,
    fmac_index,
    link_ndx,
    blk_id,
    brst_size_init_val,
    fbr_nof_links,
    fbr_nof_macs,
    fbr_nof_links_in_mac;
  uint32 reg_val;
  int i;

    DNXC_INIT_FUNC_DEFS;
  
    /* fix soc_dnx_defines_t in case of QMX */ 
    if (SOC_IS_QMX(unit)) {
        SOC_DNX_DEFS_SET(unit, nof_fabric_links, 16); 
        SOC_DNX_DEFS_SET(unit, nof_instances_fmac, 4);
    }

    fbr_nof_links = SOC_DNX_DEFS_GET(unit, nof_fabric_links);
      
    for (link_ndx=0; link_ndx < fbr_nof_links; link_ndx++) {
        fmac_index = link_ndx % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac);
        blk_id = DNX_SAND_DIV_ROUND_DOWN(link_ndx,SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac));

        brst_size_init_val = 1;
        byte_mode = 0x1;
        llfc_enabler_mode = 0x3;
        period = SOC_DNX_DEFS_GET(unit, fabric_comma_burst_period);

        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr ,  blk_id, fmac_index, FMAL_N_CM_BRST_SIZEf,  brst_size_init_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr ,  blk_id, fmac_index, FMAL_N_CM_BRST_SIZE_LLFCf,  brst_size_init_val));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr ,  blk_id, fmac_index, FMAL_N_CM_BRST_LLFC_ENABLERf,  llfc_enabler_mode));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr ,  blk_id, fmac_index, FMAL_N_CM_TX_BYTE_MODEf,  byte_mode));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_FMAL_TX_COMMA_BURST_CONFIGURATIONr ,  blk_id, fmac_index, FMAL_N_CM_TX_PERIODf,  period));

        /* 
        * Initalize the comma and control burst structures. 
        * It is done by reading the information from registers. 
        */
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_TX_CONTROL_BURST_CONFIGURATIONr(unit, blk_id, 0, &reg_val));
        SOC_DNX_PORT_PARAMS(unit).comma_burst_conf[link_ndx] = reg_val;        
    }


    fbr_nof_macs = SOC_DNX_DEFS_GET(unit, nof_instances_fmac);
    for (blk_id = 0; blk_id < fbr_nof_macs; ++blk_id) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_GENERAL_CONFIGURATION_REGISTERr,  blk_id, 0, FAP_MODEf,  0x1));
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_TX_CONTROL_BURST_CONFIGURATIONr(unit, blk_id, 0, &reg_val));
        SOC_DNX_PORT_PARAMS(unit).control_burst_conf[blk_id] = reg_val;

        /*FMAC Leaky bucket reset configuration*/
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr,  blk_id, 0, SIG_DET_BKT_RST_ENAf,  0x1));
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, FMAC_LEAKY_BUCKET_CONTROL_REGISTERr,  blk_id, 0, ALIGN_LCK_BKT_RST_ENAf,  0x1));
    }

    /* Enable Fabric Multicast */
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, RTP_MULTICAST_DISTRIBUTION_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_MCL_UPDATESf,  0x1));
   
    /*enable RX_LOS_SYNC interrupt*/
    fbr_nof_links_in_mac = SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac);
    for(i = 0; i < fbr_nof_links_in_mac; i++) {
        DNXC_IF_ERR_EXIT(READ_FMAC_FPS_CONFIGURATION_RX_SYNCr(unit, REG_PORT_ANY, i, &reg_val));
        soc_reg_field_set(unit, FMAC_FPS_CONFIGURATION_RX_SYNCr, &reg_val ,FPS_N_RX_SYNC_FORCE_LCK_ENf, 0);
        soc_reg_field_set(unit, FMAC_FPS_CONFIGURATION_RX_SYNCr, &reg_val ,FPS_N_RX_SYNC_FORCE_SLP_ENf, 0);
        DNXC_IF_ERR_EXIT(WRITE_BRDC_FMAC_FPS_CONFIGURATION_RX_SYNCr(unit, i, reg_val));
    }

exit:
    DNXC_FUNC_RETURN;
}



/*********************************************************************
* NAME:
*     jer2_arad_fabric_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*   Note! Must run after the SerDes initialization.
*********************************************************************/
uint32
  jer2_arad_fabric_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_INIT_FABRIC*      fabric
  )
{
  uint32
    link_i;
  JER2_ARAD_FABRIC_FC
    fc;  
  uint32
    res = DNX_SAND_OK;
  uint8 is_mesh;
  uint32 fld_val = 0;

    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_INIT);

  /*
   *  Set default - enabled.
   */
  jer2_arad_JER2_ARAD_FABRIC_FC_clear(unit, &fc);

  res = jer2_arad_fabric_regs_init(
          unit
        );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /*Required for TDM with PB headers*/
  if(!fabric->segmentation_enable && fabric->dual_pipe_tdm_packet) {
      fld_val = 1;
  } else if(fabric->segmentation_enable) {
      fld_val = 1;
  }  

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_FDT_SEGMENTATION_AND_INTERLEAVINGr, REG_PORT_ANY, 0, INTERLEAVING_IPT_TO_IRE_ENf,  fld_val));

  /*
   * configure load balance mode
   */
  is_mesh = DNX_SAND_NUM2BOOL(SOC_DNX_IS_MESH((unit)));
  DNX_SAND_CHECK_FUNC_RESULT(res, 55, exit);
  if (!is_mesh) {
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_LOAD_BALANCING_CONFIGURATIONr, REG_PORT_ANY, 0, ENABLE_SWITCHING_NETWORKf,  0x1));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  61,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_LOAD_BALANCING_CONFIGURATIONr, REG_PORT_ANY, 0, SWITCHING_NETWORK_LFSR_CNTf,  500));
  }

  for (link_i=0; link_i<SOC_DNX_DEFS_GET(unit, nof_fabric_links); ++link_i)
  {
    fc.enable[link_i] = TRUE;
  }

  if (!SOC_IS_ARDON(unit)) {
      res = jer2_arad_fabric_flow_control_init(
              unit,
              JER2_ARAD_CONNECTION_DIRECTION_BOTH,
              &fc
            );
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
  }

  res = jer2_arad_fabric_scheduler_adaptation_init(
          unit);
  DNX_SAND_CHECK_FUNC_RESULT(res, 77, exit);

  /* 
   *configure repeater remote links
   */
  /*This configuration is shared among all the links in a specfic quad*/
#ifdef BCM_88660_A0
  if (SOC_IS_ARADPLUS((unit)))
  {
      uint32 quad_repeater_bitmap;
      uint32 fmac_index;
      uint32 link_internal;
      int is_remote_link_repeater;   

      for (fmac_index = 0; fmac_index < SOC_DNX_DEFS_GET(unit, nof_instances_fmac); fmac_index++)
      {
          quad_repeater_bitmap = 0;
          for (link_internal = 0; link_internal < SOC_JER2_ARAD_NOF_LINKS_IN_MAC; link_internal++)
          {
              link_i = link_internal + fmac_index*SOC_JER2_ARAD_NOF_LINKS_IN_MAC;
              is_remote_link_repeater = soc_property_port_get(unit, link_i, spn_REPEATER_LINK_ENABLE, 0) ? 1 : 0;
              quad_repeater_bitmap |= is_remote_link_repeater << link_internal; 
          }

          if (quad_repeater_bitmap == 0xf)
          {
              /*All the links in the quad connected to remote link*/
              res = jer2_arad_fabric_empty_cell_size_set(
                        unit,
                        fmac_index,
                        JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_REMOTE_LINK_REPEATER
                    );
              DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
          } else if (quad_repeater_bitmap == 0x0) {
              /*None of the links connected to repeater*/
              /*Do nothing*/
          } else {
              /*Mixed configuration - not supported*/
              DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_MIXED_CONFIGURATION_ERR, 40, exit);
          }
      }
  }
#endif /*BCM_88660_A0*/
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_init()",0,0);
}

/*********************************************************************
*     This procedure enables/disables  link level flow-control on fabric
*     links.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  )
{
  uint32
    fld_val,
    reg_val,
    tx_field,
    rx_field;
  uint32
    fmac_index = 0,
    link_ndx = 0,
    inner_link = 0;
  uint8
    rx_enable = FALSE,
    tx_enable = FALSE;
  uint32 res;
   
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_ENABLE_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);

  


  rx_enable = JER2_ARAD_IS_DIRECTION_RX(direction_ndx);
  tx_enable = JER2_ARAD_IS_DIRECTION_TX(direction_ndx);

  /*init link level flow control*/
  for (link_ndx = 0; link_ndx < SOC_DNX_DEFS_GET(unit, nof_fabric_links); ++link_ndx)
  {
    fld_val = DNX_SAND_BOOL2NUM(info->enable[link_ndx]);
    fmac_index = DNX_SAND_DIV_ROUND_DOWN(link_ndx,SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac));
    inner_link = link_ndx % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac);

    if (tx_enable)
    {
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit,fmac_index,&reg_val));
      tx_field = soc_reg_field_get(unit,FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr,reg_val,LNK_LVL_FC_TX_ENf);
      DNX_SAND_SET_BIT(tx_field,fld_val,inner_link);
      soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val,LNK_LVL_FC_TX_ENf, tx_field);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit,fmac_index,reg_val));
    }

    if (rx_enable)
    {
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1020, exit, READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit,fmac_index,&reg_val));
      rx_field = soc_reg_field_get(unit,FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr,reg_val,LNK_LVL_FC_RX_ENf);
      DNX_SAND_SET_BIT(rx_field,fld_val,inner_link);
      soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val,LNK_LVL_FC_RX_ENf, rx_field);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, WRITE_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit,fmac_index,reg_val));
    }

  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_enable_set_unsafe()",0,0);
}

/*********************************************************************
*     This procedure initlize  fabric flow control mechanism: RCI, GCI and fabric LLFC.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_flow_control_init(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  )
{
  uint32
    reg_val,
    rx_field;
  uint8
    rx_enable = FALSE;
  uint32 res;
   
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_NULL_INPUT(info);

  


  rx_enable = JER2_ARAD_IS_DIRECTION_RX(direction_ndx);

  /*init link level flow control*/
  res = jer2_arad_fabric_fc_enable_set_unsafe(
      unit,
      direction_ndx,
      info
    );
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /*init RCI*/
  if (rx_enable) {
      /*enabled fabric RCI and local RCI*/
      rx_field = 0x1;
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1040, exit, READ_SCH_RCI_PARAMSr(unit, REG_PORT_ANY,&reg_val));
      soc_reg_field_set(unit, SCH_RCI_PARAMSr, &reg_val,RCI_ENAf, rx_field);
      soc_reg_field_set(unit, SCH_RCI_PARAMSr, &reg_val,DEVICE_RCI_ENAf, rx_field);
      DNX_SAND_SOC_IF_ERROR_RETURN(res, 1041, exit, WRITE_SCH_RCI_PARAMSr(unit, REG_PORT_ANY,reg_val));

      /*Set local RCI default threshold*/
      if (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.dual_pipe_tdm_packet)
      {
          res = jer2_arad_fabric_rci_config_set(unit, JER2_ARAD_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH, JER2_ARAD_FABRIC_RCI_THRESHOLD_DUAL_PIPE_DEFAULT);
          DNX_SAND_CHECK_FUNC_RESULT(res, 1042, exit);
      } else {
          res = jer2_arad_fabric_rci_config_set(unit, JER2_ARAD_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH, JER2_ARAD_FABRIC_RCI_THRESHOLD_SINGLE_PIPE_DEFAULT);
          DNX_SAND_CHECK_FUNC_RESULT(res, 1043, exit);
      }

  }

  /*init GCI*/
  /*no need: GCI is enabled by default*/


  /*Init GCI backoff mechanism*/
  /*Default GCI mechanism is GCI Leaky bucket -  Disable GCI backoff*/
  if (rx_enable)
  {
      /*Disable Gci backoff mechansim*/
      res = jer2_arad_fabric_gci_enable_set(unit, JER2_ARAD_FABRIC_GCI_TYPE_RANDOM_BACKOFF, 0);    
 
      /* call mbcm */
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1059,  exit, JER2_ARAD_REG_ACCESS_ERR, MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_fabric_gci_backoff_masks_init, (unit)));

      /*GCI backoff level configuration*/
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1060,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_GCI_BACKOFF_CS_WEIGHTSr, REG_PORT_ANY, 0, CNGST_LVL_WORSE_WEIGHTf,  0x1));
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1061,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPT_GCI_BACKOFF_CS_WEIGHTSr, REG_PORT_ANY, 0, CNGST_LVL_BETTER_WEIGHTf,  0x1));


  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_flow_control_init()",0,0);
}


/*********************************************************************
*     This procedure initlize  fabric flow control mechanism:  GCI .
*********************************************************************/
soc_error_t
  jer2_arad_fabric_gci_backoff_masks_init(
    DNX_SAND_IN  int                 unit
  )
{
  uint32 gci_backoff_mask_table[SOC_MAX_MEM_WORDS];
  DNXC_INIT_FUNC_DEFS;

    /*Random Time masking - per gci status mask LFSR for random time generate*/
  /*Each line represents GCI indication and each mask represents GCI backoff level*/
  /*GCI 1*/
  DNXC_IF_ERR_EXIT(READ_IPT_GCI_BACKOFF_MASKm(unit, MEM_BLOCK_ANY, 0,  gci_backoff_mask_table));
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_0f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_0);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_1f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_1);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_2f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_2);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_3f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_1_LEVEL_3);
  DNXC_IF_ERR_EXIT(WRITE_IPT_GCI_BACKOFF_MASKm(unit, MEM_BLOCK_ANY, 0,  gci_backoff_mask_table));

  /*GCI 2*/
  DNXC_IF_ERR_EXIT(READ_IPT_GCI_BACKOFF_MASKm(unit, MEM_BLOCK_ANY, 1,  gci_backoff_mask_table));
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_0f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_0);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_1f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_1);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_2f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_2);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_3f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_2_LEVEL_3);
  DNXC_IF_ERR_EXIT(WRITE_IPT_GCI_BACKOFF_MASKm(unit, MEM_BLOCK_ANY, 1,  gci_backoff_mask_table));

  /*GCI 3*/
  DNXC_IF_ERR_EXIT(READ_IPT_GCI_BACKOFF_MASKm(unit, MEM_BLOCK_ANY, 2,  gci_backoff_mask_table));
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_0f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_0);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_1f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_1);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_2f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_2);
  soc_mem_field32_set(unit, IPT_GCI_BACKOFF_MASKm, gci_backoff_mask_table, MASK_3f, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_GCI_3_LEVEL_3);
  DNXC_IF_ERR_EXIT(WRITE_IPT_GCI_BACKOFF_MASKm(unit, MEM_BLOCK_ANY, 2,  gci_backoff_mask_table));

exit:
    DNXC_FUNC_RETURN;
}
/*********************************************************************
*     This procedure enables/disables flow-control on fabric
*     links.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_verify(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  JER2_ARAD_CONNECTION_DIRECTION direction_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC            *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_ENABLE_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(info);

  DNX_SAND_MAGIC_NUM_VERIFY(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    direction_ndx, JER2_ARAD_NOF_CONNECTION_DIRECTIONS,
    JER2_ARAD_CONNECTION_DIRECTION_OUT_OF_RANGE_ERR, 10, exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_enable_verify()",0,0);
}

/*********************************************************************
*     This procedure enables/disables flow-control on fabric
*     links.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_enable_get_unsafe(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC            *info_rx,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC            *info_tx
  )
{
  uint32
    rx_field[1],
    tx_field[1],
    reg_val;
  uint32
    fmac_index,
    link_idx,
    inner_link;
  JER2_ARAD_FABRIC_FC
    info_local_rx,
    info_local_tx;
  uint32 res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_ENABLE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info_rx);
  DNX_SAND_CHECK_NULL_INPUT(info_tx);

  jer2_arad_JER2_ARAD_FABRIC_FC_clear(unit, &info_local_rx);
  jer2_arad_JER2_ARAD_FABRIC_FC_clear(unit, &info_local_tx);

  

  for (link_idx = 0; link_idx < SOC_DNX_DEFS_GET(unit, nof_fabric_links); ++link_idx)
  {
    fmac_index = DNX_SAND_DIV_ROUND_DOWN(link_idx,SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac));
    inner_link = link_idx % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac);

    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit,fmac_index,&reg_val));
    *rx_field = soc_reg_field_get(unit,FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr,reg_val,LNK_LVL_FC_RX_ENf);
    *tx_field = soc_reg_field_get(unit,FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr,reg_val,LNK_LVL_FC_TX_ENf);
    if(SHR_BITGET(rx_field, inner_link))
    {
      info_rx->enable[link_idx] = DNX_SAND_NUM2BOOL(JER2_ARAD_FABRIC_LINK_FC_ENABLE_MODE_ON_VAL); 
    }
    else
    {
      info_rx->enable[link_idx] = DNX_SAND_NUM2BOOL(JER2_ARAD_FABRIC_LINK_FC_ENABLE_MODE_OFF_VAL);
    }

    if(SHR_BITGET(tx_field, inner_link))
    {
      info_tx->enable[link_idx] = DNX_SAND_NUM2BOOL(JER2_ARAD_FABRIC_LINK_FC_ENABLE_MODE_ON_VAL);
    }
    else
    {
      info_tx->enable[link_idx] = JER2_ARAD_FABRIC_LINK_FC_ENABLE_MODE_OFF_VAL;
    }
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_enable_get_unsafe()",0,0);
}

/*********************************************************************
*     This procedure sets parameters of the data shaper
*     and flow control shaper of the given link.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_set_unsafe(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                        link_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER            *info,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER            *exact_info
  )
{
  uint32
    res,
    bytes_cells_mode,
    fmac_index,
    blk_id,
    fc_cells,
    fc_bytes,
    data_bytes,
    data_cells,
    fc_period,
    data_period;
    
        
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_SHAPER_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);
  DNX_SAND_CHECK_NULL_INPUT(exact_info);
  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_clear(exact_info);


  fmac_index = DNX_SAND_DIV_ROUND_DOWN(link_ndx,SOC_DNX_DEFS_GET(unit, nof_instances_fmac));
  blk_id = link_ndx % SOC_DNX_DEFS_GET(unit, nof_instances_fmac);
  

  switch (shaper_mode->shaper_mode) 
  {
    case (JER2_ARAD_FABRIC_SHAPER_BYTES_MODE):
    {
      bytes_cells_mode = 0x1;

      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_BYTE_MODEf,  bytes_cells_mode)); 
      /*Setting link's data shaper values*/
      data_bytes = info->data_shaper.bytes;
      data_period = dnx_sand_log2_round_up(data_bytes);

      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIODf,  data_period)); 
      exact_info->data_shaper.bytes = dnx_sand_power_of_2(data_period);

      /*Setting link's flow control shaper values*/
      fc_bytes = info->fc_shaper.bytes;
      fc_period = dnx_sand_log2_round_up(fc_bytes);
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIOD_LLFCf,  fc_period));
      exact_info->fc_shaper.bytes = dnx_sand_power_of_2(fc_period);
      break;
   }
   case (JER2_ARAD_FABRIC_SHAPER_CELLS_MODE): 
   {
     bytes_cells_mode = 0x0;
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_BYTE_MODEf,  bytes_cells_mode)); 

     /*Setting link's data shaper values*/
     data_cells = info->data_shaper.cells;
     data_period = dnx_sand_log2_round_up(data_cells);
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  50,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIODf,  data_period)); 
     exact_info->data_shaper.cells = dnx_sand_power_of_2(data_period);

     /*Setting link's flow control shaper values*/
     fc_cells = info->fc_shaper.cells;
     fc_period = dnx_sand_log2_round_up(fc_cells);
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  60,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIOD_LLFCf,  fc_period));
     exact_info->fc_shaper.cells = dnx_sand_power_of_2(fc_period);
     break;
   }
   default:  
     DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_SHAPER_MODE_OUT_OF_RANGE_ERR, 70, exit);   
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_shaper_set_unsafe()",link_ndx,0);
}

/*********************************************************************
*     This procedure verifies the given parameters for the data shaper
*     and flow control shaper of the given link.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_verify(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  uint32                        link_ndx,
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_FC_SHAPER            *info,
    DNX_SAND_OUT JER2_ARAD_FABRIC_FC_SHAPER            *exact_info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_SHAPER_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(info);
  DNX_SAND_CHECK_NULL_INPUT(exact_info);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    link_ndx, SOC_DNX_DEFS_GET(unit, nof_fabric_links),
    JER2_ARAD_FBR_LINK_ID_OUT_OF_RANGE_ERR, 10, exit
  );

  DNX_SAND_ERR_IF_ABOVE_MAX(
    info->data_shaper.bytes, JER2_ARAD_FABRIC_SHAPER_BYTES_MAX,
    JER2_ARAD_FABRIC_SHAPER_BYTES_OUT_OF_RANGE_ERR, 20, exit
  );

  DNX_SAND_ERR_IF_ABOVE_MAX(
    info->fc_shaper.bytes, JER2_ARAD_FABRIC_SHAPER_BYTES_MAX,
    JER2_ARAD_FABRIC_SHAPER_BYTES_OUT_OF_RANGE_ERR, 30, exit
  );

  DNX_SAND_ERR_IF_ABOVE_MAX(
    info->data_shaper.cells, JER2_ARAD_FABRIC_SHAPER_CELLS_MAX,
    JER2_ARAD_FABRIC_SHAPER_CELLS_OUT_OF_RANGE_ERR, 40, exit
  );

  DNX_SAND_ERR_IF_ABOVE_MAX(
    info->fc_shaper.cells, JER2_ARAD_FABRIC_SHAPER_CELLS_MAX,
    JER2_ARAD_FABRIC_SHAPER_CELLS_OUT_OF_RANGE_ERR, 50, exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_shaper_verify()",link_ndx,0);
}   
  
/*********************************************************************
*     This procedure returns parameters of the data shaper
*     and flow control shaper of the given link.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_fc_shaper_get_unsafe(
    DNX_SAND_IN  int                         unit,
    DNX_SAND_IN  uint32                         link_ndx,
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO  *shaper_mode,  
    DNX_SAND_OUT  JER2_ARAD_FABRIC_FC_SHAPER            *info
  )
{
  uint32
    res,
    bytes_cells_mode,
    fmac_index,
    blk_id,
    data_period,
    fc_period;
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_FC_SHAPER_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);

  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_MODE_INFO_clear(shaper_mode);
  jer2_arad_JER2_ARAD_FABRIC_FC_SHAPER_clear(info);


  fmac_index = DNX_SAND_DIV_ROUND_DOWN(link_ndx,SOC_DNX_DEFS_GET(unit, nof_instances_fmac));
  blk_id = link_ndx % SOC_DNX_DEFS_GET(unit, nof_instances_fmac);
  

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_BYTE_MODEf, &bytes_cells_mode)); 
  shaper_mode->shaper_mode = bytes_cells_mode;

  switch (bytes_cells_mode) 
  {
    case (JER2_ARAD_FABRIC_SHAPER_BYTES_MODE):
    {
      /*Getting link's data shaper values*/
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIODf, &data_period)); 
      info->data_shaper.bytes = dnx_sand_power_of_2(data_period);

      /*Getting link's flow control shaper values*/
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIOD_LLFCf, &fc_period)); 
      info->fc_shaper.bytes = dnx_sand_power_of_2(fc_period);
      break;
   }
   case (JER2_ARAD_FABRIC_SHAPER_CELLS_MODE): 
   {
     /*Getting link's data shaper values*/
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIODf, &data_period)); 
     info->data_shaper.cells = dnx_sand_power_of_2(data_period);

     /*Getting link's flow control shaper values*/
     DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FMAC_FMAL_COMMA_BURST_CONFIGURATIONr,  blk_id,  fmac_index, FMAL_N_CM_TX_PERIOD_LLFCf, &fc_period)); 
     info->fc_shaper.cells = dnx_sand_power_of_2(fc_period);
     break;
   }
   default:  
     DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_SHAPER_MODE_OUT_OF_RANGE_ERR, 70, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_fc_shaper_get_unsafe()",link_ndx,0);
}


/*********************************************************************
*     This procedure return value_1 by the table values_arr according
*     to the speed and the encoding
*********************************************************************/
static
int
    _jer2_arad_fabric_mesh_value1_get(
       DNX_SAND_IN int unit,
       DNX_SAND_OUT uint32 *value_1,
       DNX_SAND_IN int speed,
       DNX_SAND_IN soc_dnxc_port_pcs_t pcs
       )
{
    struct values_table_entry
    {
        int speed;
        uint32 value[_SHR_PORT_PCS_COUNT];
    };

    struct values_table_entry values_arr[] = {
        {5750,  {53, 34, 104, 104, 40}},
        {6250,  {49, 31, 96, 96, 37}},
        {8125,  {38, 24, 75, 75, 29}},
        {8500,  {37, 23, 72, 72, 28}},
        {10312, {30, 20, 60, 60, 23}},
        {11250, {24, 24, 55, 55, 22}},
        {11500, {28, 28, 54, 54, 21}},
        {12500, {22, 22, 50, 50, 20}}
    };

    int i = 0;
    DNXC_INIT_FUNC_DEFS;

    if (pcs  > 4 /*table size above*/)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("unsporrted pcs % \n"), pcs));
    }

    *value_1=0;
    for( i = 0 ; i < sizeof(values_arr)/sizeof(values_arr[0]); i++) {
        if(values_arr[i].speed == speed){
            *value_1 = values_arr[i].value[pcs] * SOC_DNX_CONFIG(unit)->jer2_arad->init.core_freq.system_ref_clock /300000;
            break;
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     This procedure set the mesh topology registers for mesh mode
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
    jer2_arad_fabric_mesh_topology_values_config(
       DNX_SAND_IN int unit,
       DNX_SAND_IN int speed,
       DNX_SAND_IN soc_dnxc_port_pcs_t pcs
       )
{
    uint32 value_1;
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_IF_ERR_EXIT(_jer2_arad_fabric_mesh_value1_get(unit, &value_1, speed , pcs));
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_REG_010Cr, REG_PORT_ANY, 0, FIELD_0_10f,  value_1));
    if(value_1 != 0){
        value_1++;
    }
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_SYNC_MSG_RX_ADJ_FACTORr, REG_PORT_ANY, 0, SYNC_MSG_RX_ADJ_FACTORf,  value_1));    

exit:
    DNXC_FUNC_RETURN;
}
/*********************************************************************
*     This procedure initlize scheduler adaptation to links' states
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
    jer2_arad_fabric_scheduler_adaptation_init(
       DNX_SAND_IN int    unit
       )
{
  uint32 res,reg_val32;
  int field_val=0;
 
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  if (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.scheduler_adapt_to_links) field_val=1;

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_RTP_RTP_ENABLEr_REG32(unit,&reg_val32));
  soc_reg_field_set(unit, RTP_RTP_ENABLEr, &reg_val32,EN_LOCAL_LINK_REDUCTION_MCf, field_val);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_RTP_RTP_ENABLEr_REG32(unit,reg_val32));



exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_scheduler_adaptation_init()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 is_single_fap_mode
  )
{
  uint32
    res,
    fld_val = 0;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_SET_UNSAFE);

  fld_val = DNX_SAND_BOOL2NUM(is_single_fap_mode);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, STAN_ALNf,  fld_val));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, MULTI_FAP_2f,  fld_val));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_stand_alone_fap_mode_set_unsafe()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint8                 is_single_fap_mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_VERIFY);

  JER2_ARAD_DO_NOTHING_AND_EXIT;

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_stand_alone_fap_mode_verify()",0,0);
}

/*********************************************************************
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_stand_alone_fap_mode_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint8                 *is_single_fap_mode
  )
{
  uint32
    res,
    fld_val = 0;
   
    

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_STAND_ALONE_FAP_MODE_GET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(is_single_fap_mode);

  

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, STAN_ALNf, &fld_val));
  *is_single_fap_mode = DNX_SAND_NUM2BOOL(fld_val);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_stand_alone_fap_mode_get_unsafe()",0,0);
}
/*********************************************************************
*     Configure the fabric mode to work in one of the
*     following modes: FE, back to back, mesh or multi stage
*     FE.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_connect_mode_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode
  )
{
  soc_port_t port;
  uint32
    res,
    value_1,
    value_2,
    value_3;
   
    
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_CONNECT_MODE_SET_UNSAFE);

  switch(fabric_mode)
  {
  case JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP:
  case JER2_ARAD_FABRIC_CONNECT_MODE_FE:

    value_1 = JER2_ARAD_FBC_CONNECT_FE_VAL_1;
    value_2 = JER2_ARAD_FBC_CONNECT_FE_VAL_2;
    break;

  case JER2_ARAD_FABRIC_CONNECT_MODE_MULT_STAGE_FE:

    value_1 = JER2_ARAD_FBC_CONNECT_FE_MULT_VAL_1;
    value_2 = JER2_ARAD_FBC_CONNECT_FE_MULT_VAL_2;
    break;
  case JER2_ARAD_FABRIC_CONNECT_MODE_MESH:
    value_3 = (SOC_DNX_CONFIG(unit)->tdm.is_bypass) ? JER2_ARAD_FBC_CONNECT_MESH_BYPASS_TDM : JER2_ARAD_FBC_CONNECT_MESH_VAL_3;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  7,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, ECI_GLOBAL_1r, REG_PORT_ANY, 0, MESH_MODEf,  value_3));
  /* fall through */
  case JER2_ARAD_FABRIC_CONNECT_MODE_BACK2BACK:
    
    value_1 = 0;
    PBMP_SFI_ITER(unit, port)
    {
       
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
      speed = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, 0);
      if(speed != -1)
      {
        /* assume all active ports have the same speed*/
        if(speed == 0)
        {
            res = jer2_arad_port_speed_max(unit, port, &speed);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res , 50 , exit);
        }
        res = jer2_arad_port_control_pcs_get(unit, port, &pcs);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit);
        res = _jer2_arad_fabric_mesh_value1_get(unit, &value_1 , speed, pcs);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 52, exit);
        break;
      }
#endif 
    }
    value_2 = JER2_ARAD_FBC_CONNECT_BACK2BACK_VAL_2;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  55,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_REG_010Cr, REG_PORT_ANY, 0, FIELD_0_10f,  value_1));
    if(value_1 != 0)
    {
      value_1 += 1;
    }
    break;
  default:
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_ILLEGAL_CONNECT_MODE_FE_ERR, 10, exit);
  }

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_SYNC_MSG_RX_ADJ_FACTORr, REG_PORT_ANY, 0, SYNC_MSG_RX_ADJ_FACTORf,  value_1));
  

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, MESH_TOPOLOGY_MESH_TOPOLOGYr, REG_PORT_ANY, 0, MULTI_FAPf,  value_2));

  if (
      (fabric_mode == JER2_ARAD_FABRIC_CONNECT_MODE_FE) ||
      (fabric_mode == JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP) ||
      (fabric_mode == JER2_ARAD_FABRIC_CONNECT_MODE_MULT_STAGE_FE)
    )
  {
      
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  35,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_MESH_TOPOLOGY_INITr(unit,  0xd2d));
  }
  else
  {
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  37,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_MESH_TOPOLOGY_INITr(unit,  0xd00));
  }
  value_1 = (fabric_mode == JER2_ARAD_FABRIC_CONNECT_MODE_SINGLE_FAP)?1:0;
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,WRITE_IPT_FORCE_LOCAL_OR_FABRICr(unit,  value_1));

  
  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_connect_mode_set_unsafe()",0,0);
}

/*********************************************************************
*     Configure the fabric mode to work in one of the
*     following modes: FE, back to back, mesh or multi stage
*     FE.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_connect_mode_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CONNECT_MODE fabric_mode
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_CONNECT_MODE_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(
      fabric_mode,  JER2_ARAD_FABRIC_NOF_CONNECT_MODES - 1,
      JER2_ARAD_FABRIC_ILLEGAL_CONNECT_MODE_FE_ERR, 10, exit
  );

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_connect_mode_verify()",0,0);
}


soc_error_t
    jer2_arad_link_power_set(
        int unit, 
        soc_port_t port, 
        uint32 flags, 
        soc_dnxc_port_power_t power
        )
{
    JER2_ARAD_LINK_STATE_INFO info;
    DNXC_INIT_FUNC_DEFS;

    jer2_arad_JER2_ARAD_LINK_STATE_INFO_clear(&info);
    if (power == soc_dnxc_port_power_on) {
        info.on_off = JER2_ARAD_LINK_STATE_ON;
    } else {
        info.on_off = JER2_ARAD_LINK_STATE_OFF;
    }

    info.serdes_also = TRUE;

    DNXC_IF_ERR_EXIT(jer2_arad_link_on_off_set(unit, port, &info));

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set Fabric link, and optionally, the appropriate SerDes,
*     on/off state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_link_on_off_set(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  soc_port_t             port,
    DNX_SAND_IN  JER2_ARAD_LINK_STATE_INFO   *info
  )
{
    uint32
        reg_val,
        pwr_up_val,
        res,
        rx_field[1], 
        tx_field[1],
        high_64bits,
        low_64bits;
    uint32
        blk_id,
        inner_link,
        reg_idx,
        fld_idx;
    uint64
        reg_64val;
    int 
        link_ndx;
    DNXC_INIT_FUNC_DEFS;
  
    link_ndx = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);

    reg_idx = JER2_ARAD_REG_IDX_GET(link_ndx, DNX_SAND_REG_SIZE_BITS);
    fld_idx = JER2_ARAD_FLD_IDX_GET(link_ndx, DNX_SAND_REG_SIZE_BITS);

    pwr_up_val = (info->on_off == JER2_ARAD_LINK_STATE_ON)?0x1:0x0;
  
    blk_id = DNX_SAND_DIV_ROUND_DOWN(link_ndx, SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac));
    inner_link = SOC_IS_ARDON(unit) ? link_ndx % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac) : link_ndx % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac);
  
    if (info->on_off == JER2_ARAD_LINK_STATE_ON)
    {  
        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, &reg_val));
        *rx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
        *tx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);
  
        SHR_BITCLR(rx_field, inner_link);
        SHR_BITCLR(tx_field, inner_link);
  
        /* MAC-TX */
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf, *tx_field);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, reg_val)); 
      
        /* RTP */
        DNXC_IF_ERR_EXIT(READ_RTP_ALLOWED_LINKSr_REG64(unit, &reg_64val));
        high_64bits = COMPILER_64_HI(reg_64val);
        low_64bits = COMPILER_64_LO(reg_64val);
        if(reg_idx == 0) {
            DNX_SAND_SET_BIT(low_64bits, pwr_up_val, fld_idx);
        } else {
            DNX_SAND_SET_BIT(high_64bits, pwr_up_val, fld_idx);
        }
        COMPILER_64_SET(reg_64val, high_64bits, low_64bits);
        DNXC_IF_ERR_EXIT(WRITE_RTP_ALLOWED_LINKSr_REG64(unit,  reg_64val));
    
        /* MAC-RX */
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf, *rx_field);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, reg_val));

        /* SerDes */
        if (info->serdes_also == TRUE) {
            MIIM_LOCK(unit);
            res = soc_phyctrl_enable_set(unit, port, TRUE);
            MIIM_UNLOCK(unit);
            DNXC_IF_ERR_EXIT(res);    
        }
    } else { /* Same as above, in reverse order */

        /* SerDes */
        if (info->serdes_also == TRUE)
        {
            MIIM_LOCK(unit);
            res = soc_phyctrl_enable_set(unit, port, FALSE);
            MIIM_UNLOCK(unit);
            DNXC_IF_ERR_EXIT(res);
        }

        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, &reg_val));
        *rx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
        *tx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);

        SHR_BITSET(rx_field, inner_link);
        SHR_BITSET(tx_field, inner_link);
  
        /* MAC-RX */
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf, *rx_field);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, reg_val));
    
        /* RTP */
        DNXC_IF_ERR_EXIT(READ_RTP_ALLOWED_LINKSr_REG64(unit, &reg_64val));
        high_64bits = COMPILER_64_HI(reg_64val);
        low_64bits = COMPILER_64_LO(reg_64val);
        if(reg_idx == 0) {
            DNX_SAND_SET_BIT(low_64bits, pwr_up_val, fld_idx);
        } else {
            DNX_SAND_SET_BIT(high_64bits, pwr_up_val, fld_idx);
        }
        COMPILER_64_SET(reg_64val, high_64bits, low_64bits);
        DNXC_IF_ERR_EXIT(WRITE_RTP_ALLOWED_LINKSr_REG64(unit,  reg_64val));

        /* MAC-TX */
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf, *tx_field);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, reg_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set Fabric link, and optionally, the appropriate SerDes,
*     on/off state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_link_on_off_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 link_ndx,
    DNX_SAND_IN  JER2_ARAD_LINK_STATE_INFO     *info
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_LINK_ON_OFF_VERIFY);

  DNX_SAND_CHECK_NULL_INPUT(info);
  DNX_SAND_MAGIC_NUM_VERIFY(info);

  DNX_SAND_ERR_IF_ABOVE_MAX(
    link_ndx, SOC_DNX_DEFS_GET(unit, nof_fabric_links),
    JER2_ARAD_FBR_LINK_ID_OUT_OF_RANGE_ERR, 10, exit
  );

  DNX_SAND_ERR_IF_ABOVE_MAX(
    info->on_off, JER2_ARAD_LINK_NOF_STATES,
    JER2_ARAD_FBR_LINK_ON_OFF_STATE_OUT_OF_RANGE_ERR, 20, exit
  );
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_link_on_off_verify()",link_ndx,0);
}

soc_error_t
    jer2_arad_link_power_get(
        int unit, 
        soc_port_t port, 
        soc_dnxc_port_power_t* power
        )
{
    JER2_ARAD_LINK_STATE_INFO info;
    DNXC_INIT_FUNC_DEFS;

    jer2_arad_JER2_ARAD_LINK_STATE_INFO_clear(&info);

    DNXC_IF_ERR_EXIT(jer2_arad_link_on_off_get(unit, port, &info)); 
    *power = (info.on_off == DNX_TMC_LINK_STATE_ON ? soc_dnxc_port_power_on : soc_dnxc_port_power_off);

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set Fabric link, and optionally, the appropriate SerDes,
*     on/off state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_link_on_off_get(
    DNX_SAND_IN  int                    unit,
    DNX_SAND_IN  soc_port_t             port,
    DNX_SAND_OUT JER2_ARAD_LINK_STATE_INFO   *info
  )
{
    uint32
        reg_val,
        res,
        rx_field[1], 
        tx_field[1];
    int32
        srd_pwr_state;
    uint32
        blk_id,
        inner_link;
    int 
        link_ndx;
    DNXC_INIT_FUNC_DEFS;
  
    link_ndx = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);

    blk_id = DNX_SAND_DIV_ROUND_DOWN(link_ndx, SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac));
    inner_link = link_ndx % SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac);

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, &reg_val));

    /* get MAC Rx,Tx vals */
    *rx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
    *tx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);

    jer2_arad_JER2_ARAD_LINK_STATE_INFO_clear(info);
    if(SHR_BITGET(rx_field, inner_link) || SHR_BITGET(tx_field, inner_link)) {
        info->on_off = JER2_ARAD_LINK_STATE_OFF;
    } else {
        info->on_off = JER2_ARAD_LINK_STATE_ON;
    }

    /* get SerDes val */
    MIIM_LOCK(unit);
    res = soc_phyctrl_enable_get(unit,port, &srd_pwr_state);
    MIIM_UNLOCK(unit);
    DNXC_IF_ERR_EXIT(res);    

    if (
      ((info->on_off == JER2_ARAD_LINK_STATE_ON) && (srd_pwr_state == JER2_ARAD_SRD_POWER_STATE_UP)) ||
      ((info->on_off == JER2_ARAD_LINK_STATE_OFF) && (srd_pwr_state == JER2_ARAD_SRD_POWER_STATE_DOWN))
     ) {
        info->serdes_also = TRUE;
    } else {
        info->serdes_also = FALSE;
    }

exit:
    DNXC_FUNC_RETURN
}

/*
 * Function:
 *      jer2_arad_fabric_port_speed_set
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - unit #.
 *      port - port #.
 *      speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
uint32
jer2_arad_fabric_port_speed_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN uint32                   port,
    DNX_SAND_IN int                     speed
    )
{
    uint32 res;

    DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_PORT_SPEED_SET);

    MIIM_LOCK(unit);
    res = soc_phyctrl_speed_set(unit, port, speed);
    MIIM_UNLOCK(unit);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 33, exit);
    if(SOC_DNX_IS_MESH(unit))
    {
      soc_dnxc_port_pcs_t pcs = 0;
     
     DNXC_LEGACY_FIXME_ASSERT; 
#ifdef FIXME_DNX_LEGACY
      res = jer2_arad_port_control_pcs_get(unit, port, &pcs);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 34, exit);
#endif 
    
      res = jer2_arad_fabric_mesh_topology_values_config(unit, speed, pcs);
      DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 35, exit);
    }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_port_speed_set()",port, speed);

}

/*
 * Function:
 *      jer2_arad_fabric_port_speed_get
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - unit #.
 *      port - port #.
 *      speed (OUT)- Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
int
jer2_arad_fabric_port_speed_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  port,
    DNX_SAND_OUT int                    *speed
    )
{
    uint32 res;

    DNXC_INIT_FUNC_DEFS;

    MIIM_LOCK(unit);
    res = soc_phyctrl_speed_get(unit, port, speed);
    MIIM_UNLOCK(unit);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;

}

uint32
jer2_arad_fabric_priority_bits_mapping_to_fdt_index_get(
    DNX_SAND_IN  int                              unit, 
    DNX_SAND_IN  uint32                              tc, 
    DNX_SAND_IN  uint32                              dp,
    DNX_SAND_IN  uint32                              flags,
    DNX_SAND_OUT uint32                              *index
  )
{
    uint32
        is_hp = 0;

    DNXC_INIT_FUNC_DEFS;
    *index = 0;

    if (flags & SOC_DNX_FABRIC_QUEUE_PRIORITY_HIGH_ONLY) {
        is_hp = 1;
    } /*else low priority only*/
    *index  |=  ((is_hp     << JER2_ARAD_FBC_PRIORITY_NDX_IS_HP_OFFSET)  & JER2_ARAD_FBC_PRIORITY_NDX_IS_HP_MASK  )|
                ((tc        << JER2_ARAD_FBC_PRIORITY_NDX_TC_OFFSET)     & JER2_ARAD_FBC_PRIORITY_NDX_TC_MASK     )|
                ((dp        << JER2_ARAD_FBC_PRIORITY_NDX_DP_OFFSET)     & JER2_ARAD_FBC_PRIORITY_NDX_DP_MASK     )
#if 1
                ;
#endif

  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Setting fabric priority
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
jer2_arad_fabric_priority_set(
    DNX_SAND_IN  int                              unit, 
    DNX_SAND_IN  uint32                              tc, 
    DNX_SAND_IN  uint32                              dp,
    DNX_SAND_IN  uint32                              flags,
    DNX_SAND_IN  int                                 fabric_priority
  )
{
    uint32 index,
           offset,
           val;
    uint32 fabric_priority_table[SOC_MAX_MEM_WORDS];
    int res;
    DNXC_INIT_FUNC_DEFS;

    /*validate fabric_priority size*/
    if (!SOC_DNX_CONFIG(unit)->tdm.is_tdm_over_primary_pipe) {
        if (fabric_priority >= JER2_ARAD_FBC_PRIORITY_NOF-1 /*last one for tdm only*/) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("fabric priority out of range")));
        }
    } else {
        if (fabric_priority >= JER2_ARAD_FBC_PRIORITY_NOF-2 /*2 for tdm only priorities*/) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("fabric priority out of range")));
        }
    }
    /*get index for IPT_PRIORITY_BITS_MAP_2_FDT*/
    res = jer2_arad_fabric_priority_bits_mapping_to_fdt_index_get(unit, tc, dp, flags, &index);
    DNXC_IF_ERR_EXIT(res);
    offset = index*JER2_ARAD_FBC_PRIORITY_LENGTH /*bits length*/;


    res = READ_IPT_PRIORITY_BITS_MAP_2_FDTm(unit, MEM_BLOCK_ANY, 0, fabric_priority_table);
    DNXC_IF_ERR_EXIT(res);
    val = fabric_priority;
    SHR_BITCOPY_RANGE(fabric_priority_table, offset, &val, 0, JER2_ARAD_FBC_PRIORITY_LENGTH);
    res = WRITE_IPT_PRIORITY_BITS_MAP_2_FDTm(unit, MEM_BLOCK_ALL, 0, fabric_priority_table);
    DNXC_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Getting fabric priority
*     Details: in the H file. (search for prototype)
*********************************************************************/
soc_error_t
jer2_arad_fabric_priority_get(
    DNX_SAND_IN  int                              unit, 
    DNX_SAND_IN  uint32                              tc, 
    DNX_SAND_IN  uint32                              dp,
    DNX_SAND_IN  uint32                              flags,
    DNX_SAND_OUT int                                 *fabric_priority
  )
{
    uint32 res,
           index,
           offset,
           val = 0;
    uint32 fabric_priority_table[SOC_MAX_MEM_WORDS];
    DNXC_INIT_FUNC_DEFS;

    /*get index for IPT_PRIORITY_BITS_MAP_2_FDT*/
    res = jer2_arad_fabric_priority_bits_mapping_to_fdt_index_get(unit, tc, dp, flags, &index);
    DNXC_IF_ERR_EXIT(res);
    offset = index*JER2_ARAD_FBC_PRIORITY_LENGTH /*bits length*/;

    
    res = READ_IPT_PRIORITY_BITS_MAP_2_FDTm(unit, MEM_BLOCK_ANY, 0, fabric_priority_table);
    DNXC_IF_ERR_EXIT(res);
    SHR_BITCOPY_RANGE(&val, 0, fabric_priority_table, offset, JER2_ARAD_FBC_PRIORITY_LENGTH); 
    *fabric_priority = val;

exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Retrieve the connectivity map from the device.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_topology_status_connectivity_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                    link_index_min,
    DNX_SAND_IN  int                    link_index_max,
    DNX_SAND_OUT JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR *connectivity_map
  )
{
  int
    source_lvl;
  bcm_port_t
    link_id, link_index;
  uint32
    reg_val,
    is_active;
  uint64
    mask;
  uint32 res = DNX_SAND_OK;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_TOPOLOGY_STATUS_CONNECTIVITY_GET_UNSAFE);

  if (soc_feature(unit, soc_feature_no_fabric)) {
    SOC_EXIT;
  }
  DNX_SAND_CHECK_NULL_INPUT(connectivity_map);
  DNX_SAND_ERR_IF_ABOVE_MAX(
    link_index_max, SOC_DNX_DEFS_GET(unit, nof_fabric_links) + SOC_DNX_DEFS_GET(unit, first_fabric_link_id) - 1,
    JER2_ARAD_FBR_LINK_ID_OUT_OF_RANGE_ERR, 10, exit
  );
  jer2_arad_JER2_ARAD_FABRIC_LINKS_CON_STAT_INFO_ARR_clear(connectivity_map);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 1480, exit, READ_RTP_LINK_ACTIVE_MASKr_REG64(unit, &mask));

  COMPILER_64_NOT(mask);

  
  for(link_index = link_index_min; link_index <= link_index_max; ++link_index)
  {

    link_id = link_index;
    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1490, exit, READ_FCR_CONNECTIVITY_MAP_REGISTERSr(unit, link_id, &reg_val));
    is_active = COMPILER_64_BITTEST(mask, link_id);
    if(is_active)
    {
      connectivity_map->link_info[link_index].far_unit = soc_reg_field_get(unit, FCR_CONNECTIVITY_MAP_REGISTERSr, reg_val, SOURCE_DEVICE_ID_Nf);

      source_lvl = soc_reg_field_get(unit, FCR_CONNECTIVITY_MAP_REGISTERSr, reg_val, SOURCE_DEVICE_LEVEL_Nf);
#ifdef PLISIM
      if (SAL_BOOT_PLISIM) {
          source_lvl = 0x3; /* To be FE2 */
      }
#endif
      /* 
       *  
       * 3'bx0x => FOP
       * 3'b010 => FE3,
       * 3'bX11 => FE2,
       * 3'b110 => FE1
       */

      if((source_lvl & 0x2) == 0)
      {
        connectivity_map->link_info[link_index].far_dev_type = JER2_ARAD_FAR_DEVICE_TYPE_FAP;
      }
      else if (source_lvl == 0x2)
      {
        connectivity_map->link_info[link_index].far_dev_type = JER2_ARAD_FAR_DEVICE_TYPE_FE3;
      }
      else if((source_lvl & 0x3) == 0x3)
      {
        connectivity_map->link_info[link_index].far_dev_type = JER2_ARAD_FAR_DEVICE_TYPE_FE2;
      }
      else
      {
        connectivity_map->link_info[link_index].far_dev_type = JER2_ARAD_FAR_DEVICE_TYPE_FE1;
      }

      connectivity_map->link_info[link_index].far_link_id = soc_reg_field_get(unit, FCR_CONNECTIVITY_MAP_REGISTERSr, reg_val, SOURCE_DEVICE_LINK_Nf);
      connectivity_map->link_info[link_index].is_logically_connected=1; 
    }
    else
    {
      connectivity_map->link_info[link_index].far_link_id = DNXC_FABRIC_LINK_NO_CONNECTIVITY;
    }      
  }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_topology_status_connectivity_get_unsafe()",0,0);
}

/*
 * Function:
 *      jer2_arad_fabric_reachability_status_get
 * Purpose:
 *      Get reachability status
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      moduleid            - (IN)  Module to check reachbility to
 *      links_max           - (IN)  Max size of links_array
 *      links_array         - (OUT) Links which moduleid is erachable through
 *      links_count         - (OUT) Size of links_array
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_reachability_status_get(int unit, int moduleid, int links_max, uint32 *links_array, int *links_count)
{
    int i, offset, port;
    soc_reg_above_64_val_t rtp_reg_val;
    uint64 link_active_mask;
    DNXC_INIT_FUNC_DEFS;
    
    SOC_REG_ABOVE_64_CLEAR(rtp_reg_val);
    if(moduleid < 0 || moduleid > 2048) {
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("module id invalid")));
    }
    DNXC_IF_ERR_EXIT(READ_RTP_UNICAST_DISTRIBUTION_MEMORY_FOR_DATA_CELLSm(unit, MEM_BLOCK_ANY, moduleid/2, rtp_reg_val));
    DNXC_IF_ERR_EXIT(READ_RTP_LINK_ACTIVE_MASKr_REG64(unit, &link_active_mask));
    
    *links_count = 0;
    offset = 36*(moduleid%2);

    PBMP_SFI_ITER(unit, port)
    {
        i = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
        
        if(SHR_BITGET(rtp_reg_val,i + offset) && !COMPILER_64_BITTEST(link_active_mask,i)) 
        {
            if(*links_count >= links_max) {
                DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("links_array is too small")));
            }
            
            links_array[*links_count] = i;
            (*links_count)++;
        }
    } 
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_dnx_fabric_link_status_all_get
 * Purpose:
 *      Get all links status
 * Parameters:
 *      unit                 - (IN)  Unit number.
 *      links_array_max_size - (IN)  max szie of link_status array
 *      link_status          - (OUT) array of link status per link
 *      errored_token_count  - (OUT) array error token count per link
 *      links_array_count    - (OUT) array actual size
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
jer2_arad_fabric_link_status_all_get(int unit, int links_array_max_size, uint32* link_status, uint32* errored_token_count, int* links_array_count)
{
    int i, rc, port;
    DNXC_INIT_FUNC_DEFS;

    (*links_array_count) = 0;

    PBMP_SFI_ITER(unit, port) {
        i = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
        if ((*links_array_count) > links_array_max_size) {
            break;
        }
        rc = jer2_arad_fabric_link_status_get(unit, i, &(link_status[(*links_array_count)]), &(errored_token_count[(*links_array_count)]));
        DNXC_IF_ERR_EXIT(rc);
        (*links_array_count)++;
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_fabric_link_status_clear
 * Purpose:
 *      clear link status
 * Parameters:
 *      unit                - (IN)  Unit number
 *      link                - (IN) Link #
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t
jer2_arad_fabric_link_status_clear(int unit, soc_port_t link)
{
    int blk_id, inner_link;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;

    blk_id = (link - SOC_DNX_DEFS_GET(unit, first_fabric_link_id))/4;
    inner_link = link % 4;

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, &reg_val, RX_CRC_ERR_N_INTf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAC_INTERRUPT_REGISTER_1r(unit,blk_id,reg_val));

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, &reg_val, WRONG_SIZE_INTf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAC_INTERRUPT_REGISTER_1r(unit,blk_id,reg_val));

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_FMAC_INTERRUPT_REGISTER_2r, &reg_val, RX_LOST_OF_SYNCf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,reg_val));
   
    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_FMAC_INTERRUPT_REGISTER_4r, &reg_val, DEC_ERR_INTf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAC_INTERRUPT_REGISTER_4r(unit,blk_id,reg_val));


exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_fabric_link_status_get
 * Purpose:
 *      Get link status
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      link_id             - (IN)  Link
 *      link_status         - (OUT) According to link status get
 *      errored_token_count - (OUT) Errored token count
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_link_status_get(int unit, soc_port_t link_id, uint32 *link_status, uint32 *errored_token_count)
{
    uint32 reg_val, field_val[1], sig_acc = 0;
    int blk_id, reg_select;
    soc_port_t inner_lnk;
    int rv;
    DNXC_INIT_FUNC_DEFS;
    
    *link_status = 0;
    
    blk_id = (link_id - SOC_DNX_DEFS_GET(unit, first_fabric_link_id))/4;
    reg_select = link_id % 4;
   
    /*leaky bucket*/
    if (reg_select >= 0 && reg_select <= 3) {
        DNXC_IF_ERR_EXIT(READ_FMAC_LEAKY_BUCKETr(unit,blk_id, reg_select, &reg_val));
        *errored_token_count = soc_reg_field_get(unit, FMAC_LEAKY_BUCKETr, reg_val, MACR_N_LKY_BKT_VALUEf);
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Can't find register for link %d"),link_id));
    }
    
  
  /*link status
    BCM_FABRIC_LINK_STATUS_CRC_ERROR Non-zero CRC rate  
    BCM_FABRIC_LINK_STATUS_SIZE_ERROR Non-zero size error-count  
    BCM_FABRIC_LINK_STATUS_CODE_GROUP_ERROR Non-zero code group error-count  
    BCM_FABRIC_LINK_STATUS_MISALIGN Link down, misalignment error  
    BCM_FABRIC_LINK_STATUS_NO_SIG_LOCK Link down, SerDes signal lock error  
    BCM_FABRIC_LINK_STATUS_NO_SIG_ACCEP Link up, but not accepting reachability cells  
    BCM_FABRIC_LINK_STATUS_ERRORED_TOKENS Low value, indicates bad link connectivity or link down, based on reachability cells */
  
    inner_lnk = link_id % 4;
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAC_INTERRUPT_REGISTER_1r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, reg_val, RX_CRC_ERR_N_INTf);
    if(SHR_BITGET(field_val, inner_lnk))
        *link_status |= DNXC_FABRIC_LINK_STATUS_CRC_ERROR;
    
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, reg_val, WRONG_SIZE_INTf);
    if(SHR_BITGET(field_val, inner_lnk))
        *link_status |= DNXC_FABRIC_LINK_STATUS_SIZE_ERROR;  
       
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_2r, reg_val, RX_LOST_OF_SYNCf);
    if(SHR_BITGET(field_val, inner_lnk))
       *link_status |= DNXC_FABRIC_LINK_STATUS_MISALIGN;  
      
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAC_INTERRUPT_REGISTER_4r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_4r, reg_val, DEC_ERR_INTf);
    if(SHR_BITGET(field_val, inner_lnk))
       *link_status |= DNXC_FABRIC_LINK_STATUS_CODE_GROUP_ERROR;  
      
    /*BCM_FABRIC_LINK_STATUS_NO_SIG_LOCK - Serdes TBD*/
    if(soc_feature(unit, soc_feature_portmod))
    {
#if defined(PORTMOD_SUPPORT)
        rv = soc_dnxc_port_rx_locked_get(unit, SOC_DNX_FABRIC_LINK_TO_PORT(unit, link_id), &sig_acc);
        DNXC_IF_ERR_EXIT(rv);
#endif

    } 
    else
    {
        MIIM_LOCK(unit); 
        if (!SOC_IS_ARDON(unit)) {
            rv = soc_phyctrl_control_get(unit, SOC_DNX_FABRIC_LINK_TO_PORT(unit, link_id), SOC_PHY_CONTROL_RX_SIGNAL_DETECT, &sig_acc); 
        } else {
            rv = soc_phyctrl_control_get(unit, SOC_DNX_FABRIC_LINK_TO_PORT(unit, link_id), SOC_PHY_CONTROL_RX_SEQ_DONE, &sig_acc); 
        }
        MIIM_UNLOCK(unit);
        DNXC_IF_ERR_EXIT(rv);
    }
    if(!sig_acc) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_NO_SIG_ACCEP;
    }
    
    if(*errored_token_count < 63) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_ERRORED_TOKENS;
    }

    /*Clear sticky indication*/
    rv = jer2_arad_fabric_link_status_clear(unit, link_id);
    DNXC_IF_ERR_EXIT(rv);
    
exit:
    DNXC_FUNC_RETURN;
  
}

soc_error_t
jer2_arad_fabric_force_signal_detect_set(int unit, int mac_instance)
{
   uint32 force_signal_detect, base_link, i;
   soc_dnxc_loopback_mode_t loopback;
   uint32 reg_val;
  
   DNXC_INIT_FUNC_DEFS;

   force_signal_detect = 0;
   base_link = SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac)*mac_instance;

   for(i=0 ; i<SOC_DNX_DEFS_GET(unit, nof_fabric_links_in_mac) ; i++) {
       DNXC_IF_ERR_EXIT(jer2_arad_fabric_loopback_get(unit, SOC_DNX_FABRIC_LINK_TO_PORT(unit, base_link+i), &loopback));
       if(soc_dnxc_loopback_mode_mac_pcs == loopback || soc_dnxc_loopback_mode_mac_outer == loopback || soc_dnxc_loopback_mode_mac_async_fifo == loopback) {
           force_signal_detect = 1;
           break;
       }
       
       DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
       DNXC_IF_ERR_EXIT(jer2_arad_port_control_pcs_get(unit,SOC_DNX_FABRIC_LINK_TO_PORT(unit, base_link+i) , &pcs));
       if(soc_dnxc_port_pcs_64_66_fec == pcs || soc_dnxc_port_pcs_64_66_bec == pcs) {
           force_signal_detect = 1;
           break;
       }
#endif 
    
   }

    DNXC_IF_ERR_EXIT(READ_FMAC_GENERAL_CONFIGURATION_REGISTERr_REG32(unit, mac_instance, &reg_val));
    soc_reg_field_set(unit, FMAC_GENERAL_CONFIGURATION_REGISTERr, &reg_val, FORCE_SIGNAL_DETECTf, force_signal_detect);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_GENERAL_CONFIGURATION_REGISTERr_REG32(unit, mac_instance, reg_val));

exit:
    DNXC_FUNC_RETURN;
}


/*
* Function:
*      jer2_arad_fabric_loopback_set
* Purpose:
*      Set port loopback
* Parameters:
*      unit      - (IN)  Unit number.
*      link      - (IN)  link number 
*      loopback  - (IN)  soc_dnxc_loopback_mode_t
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_fabric_loopback_set(int unit, soc_port_t port, soc_dnxc_loopback_mode_t loopback)
{
    soc_port_t inner_port;
    int blk_id, link_id;
    uint32 reg_val, field_val[1];
    int lane0,lane1,lane2,lane3;
    JER2_ARAD_LINK_STATE_INFO     info;
    soc_dnxc_loopback_mode_t prev_loopback;
    DNX_TMC_LINK_STATE on_off;
    uint32 rx_field[1], tx_field[1];
    int locked;

    DNXC_INIT_FUNC_DEFS;
  
    link_id = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    locked = 0;

    /* First disable FMAC */
    DNXC_IF_ERR_EXIT(jer2_arad_link_on_off_get(unit, port, &info));
    on_off = info.on_off;
    if(info.on_off == DNX_TMC_LINK_STATE_ON) {
        info.on_off = DNX_TMC_LINK_STATE_OFF;
        info.serdes_also = TRUE;
        DNXC_IF_ERR_EXIT(jer2_arad_link_on_off_set(unit, port, &info));
    }
    
    blk_id = link_id/4;
    inner_port = link_id % 4;
    
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
    /*MAC PCS llopback is valid only for 64/66 FEC encoding*/
    if(soc_dnxc_loopback_mode_mac_pcs == loopback) {
        DNXC_IF_ERR_EXIT(jer2_arad_port_control_pcs_get(unit, port, &pcs));
        if(soc_dnxc_port_pcs_64_66_fec != pcs) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("MAC PCS llopback is valid only for 64/66 FEC encoding")));
        }
    }
#endif 
    
    /*get previous loopback*/
    DNXC_IF_ERR_EXIT(jer2_arad_fabric_loopback_get(unit, port, &prev_loopback));

    if(prev_loopback != loopback) {

        /*soc_dnxc_loopback_mode_mac_async_fifo*/
        if(soc_dnxc_loopback_mode_mac_async_fifo == loopback || soc_dnxc_loopback_mode_mac_async_fifo == prev_loopback) {
            DNXC_IF_ERR_EXIT(READ_FMAC_LOOPBACK_ENABLE_REGISTERr(unit, blk_id, &reg_val));
            *field_val = soc_reg_field_get(unit, FMAC_LOOPBACK_ENABLE_REGISTERr, reg_val, LCL_LPBK_ONf);
            if(soc_dnxc_loopback_mode_mac_async_fifo == loopback) {
                SHR_BITSET(field_val, inner_port);
            } else {
                SHR_BITCLR(field_val, inner_port);
            }
            soc_reg_field_set(unit, FMAC_LOOPBACK_ENABLE_REGISTERr, &reg_val, LCL_LPBK_ONf, *field_val);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_LOOPBACK_ENABLE_REGISTERr(unit, blk_id, reg_val));
        }
        
        /*soc_dnxc_loopback_mode_mac_outer*/
        if(soc_dnxc_loopback_mode_mac_outer == loopback || soc_dnxc_loopback_mode_mac_outer == prev_loopback) {
            DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
            soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg_val, FMAL_N_CORE_40B_LOOPBACKf, soc_dnxc_loopback_mode_mac_outer == loopback ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, reg_val));

            if(soc_dnxc_loopback_mode_mac_outer == loopback) {
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
                SOC_DNX_PORT_PARAMS(unit).comma_burst_conf[port]=reg_val;
                DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, 0x00a0e513));

                DNXC_IF_ERR_EXIT(READ_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, &reg_val));
                SOC_DNX_PORT_PARAMS(unit).control_burst_conf[blk_id]=reg_val;
                DNXC_IF_ERR_EXIT(WRITE_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, 0x1801c00d));
            } else {
                DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, SOC_DNX_PORT_PARAMS(unit).comma_burst_conf[port]));

                /*if all the mac ports not in loopback in to restore default value*/
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 0, &reg_val));
                lane0 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 1, &reg_val));
                lane1 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 2, &reg_val));
                lane2 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 3, &reg_val));
                lane3 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                if(!lane0 && !lane1 && !lane2 && !lane3) {
                    DNXC_IF_ERR_EXIT(WRITE_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, SOC_DNX_PORT_PARAMS(unit).control_burst_conf[blk_id]));
                }
            }
        }

        if(soc_dnxc_loopback_mode_mac_pcs == loopback || soc_dnxc_loopback_mode_mac_pcs == prev_loopback) {
            /*soc_dnxc_loopback_mode_mac_pcs*/
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
            soc_reg_field_set(unit, FMAC_KPCS_CONFIGURATIONr, &reg_val, KPCS_N_RX_DSC_LOOPBACK_ENf, soc_dnxc_loopback_mode_mac_pcs == loopback ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_KPCS_CONFIGURATIONr(unit, blk_id, inner_port, reg_val));
        }
        
        MIIM_LOCK(unit);
        locked = 1;
        
        /*soc_dnxc_loopback_mode_phy_gloop*/
        if(soc_dnxc_loopback_mode_phy_gloop == loopback || soc_dnxc_loopback_mode_phy_gloop == prev_loopback) {
            if (!SOC_IS_ARDON(unit)) {
                DNXC_IF_ERR_EXIT(soc_phyctrl_loopback_set(unit, port, soc_dnxc_loopback_mode_phy_gloop == loopback ? 1 : 0, 1)); 
            } else {
                DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_LOOPBACK_PMD, soc_dnxc_loopback_mode_phy_gloop == loopback ? 1 : 0));
            }
        }

        /*soc_dnxc_loopback_mode_phy_rloop*/
        if(soc_dnxc_loopback_mode_phy_rloop == loopback || soc_dnxc_loopback_mode_phy_rloop == prev_loopback) {
            if (!SOC_IS_ARDON(unit)) {
                DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, soc_dnxc_loopback_mode_phy_rloop == loopback ? 1 : 0));
            } else {
                DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_LOOPBACK_REMOTE, soc_dnxc_loopback_mode_phy_rloop == loopback ? 1 : 0));
            }
        }
        
        locked = 0;
        MIIM_UNLOCK(unit); 
        
        DNXC_IF_ERR_EXIT(jer2_arad_fabric_force_signal_detect_set(unit, blk_id));

    }

    if(on_off == DNX_TMC_LINK_STATE_ON) {
        info.on_off = DNX_TMC_LINK_STATE_ON;
        info.serdes_also = TRUE;
        DNXC_IF_ERR_EXIT(jer2_arad_link_on_off_set(unit, port, &info));
    }

    /*MAC outer loopback required TX and RX to be out of reset in the same time. 
      Assuming CL72 is off*/
    if(soc_dnxc_loopback_mode_mac_outer == loopback){
        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, &reg_val));
        *rx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
        *tx_field = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);

        SHR_BITSET(rx_field, inner_port);
        SHR_BITSET(tx_field, inner_port);
  
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf, *rx_field);
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf, *tx_field);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, reg_val));

        sal_usleep(10);

        SHR_BITCLR(rx_field, inner_port);
        SHR_BITCLR(tx_field, inner_port);
  
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf, *rx_field);
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf, *tx_field);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit, blk_id, reg_val));
    }

exit:
  if (locked) {
      MIIM_UNLOCK(unit);
  }
  DNXC_FUNC_RETURN;
}

/*
* Function:
*      jer2_arad_fabric_port_loopback_get
* Purpose:
*      Get port loopback
* Parameters:
*      unit      - (IN)  Unit number.
*      link_id   - (IN)  link number 
*      loopback  - (OUT) soc_dnxc_loopback_mode_t
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_fabric_loopback_get(int unit, soc_port_t port, soc_dnxc_loopback_mode_t* loopback)
{
    uint32 inner_port;
    int blk_id;
    uint32 serdes_xrloop, serdes_xgloop;
    int serdes_gloop;
    uint32 reg_val, field_val[1];
    soc_error_t rc;
    int locked, link_id;
    DNXC_INIT_FUNC_DEFS;

    locked = 0;
  
    link_id = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link_id/4;
    inner_port = link_id % 4;

    /*soc_dnxc_loopback_mode_mac_async_fifo*/
    DNXC_IF_ERR_EXIT(READ_FMAC_LOOPBACK_ENABLE_REGISTERr(unit, blk_id, &reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_LOOPBACK_ENABLE_REGISTERr, reg_val, LCL_LPBK_ONf);
    if(SHR_BITGET(field_val, inner_port)) {
        *loopback = soc_dnxc_loopback_mode_mac_async_fifo;
    } else {        
        /*soc_dnxc_loopback_mode_mac_outer*/
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
        if(*field_val) {
            *loopback = soc_dnxc_loopback_mode_mac_outer;
        } else {
            /*soc_dnxc_loopback_mode_mac_pcs*/
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
            *field_val = soc_reg_field_get(unit, FMAC_KPCS_CONFIGURATIONr, reg_val, KPCS_N_RX_DSC_LOOPBACK_ENf);
            if(*field_val) {
                *loopback = soc_dnxc_loopback_mode_mac_pcs;
            } else {
                MIIM_LOCK(unit);
                locked = 1;
                /*soc_dnxc_loopback_mode_phy_gloop*/
                if (!SOC_IS_ARDON(unit)) {
                    rc = soc_phyctrl_loopback_get(unit, port, &serdes_gloop); 
                } else {
                    rc = soc_phyctrl_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_PMD, &serdes_xgloop);

                    serdes_gloop = serdes_xgloop ? 1 : 0;
                }
                locked = 0;
                MIIM_UNLOCK(unit);
                if(SOC_FAILURE(rc)) {
                    DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed in soc_phyctrl_loopback_get, error %s"),soc_errmsg(rc)));   
                } else if(serdes_gloop) {
                    *loopback = soc_dnxc_loopback_mode_phy_gloop;
                } else {
                    MIIM_LOCK(unit);
                    locked = 1;
                    /*soc_dnxc_loopback_mode_phy_rloop*/
                    if (!SOC_IS_ARDON(unit)) {
                        rc = soc_phyctrl_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, &serdes_xrloop);
                    } else {
                        rc = soc_phyctrl_control_get(unit, port, BCM_PORT_PHY_CONTROL_LOOPBACK_REMOTE, &serdes_xrloop);
                    }
                    locked = 0;
                    MIIM_UNLOCK(unit);
                    if(SOC_FAILURE(rc)) {
                        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed in soc_phyctrl_control_get(SOC_PHY_CONTROL_LOOPBACK_REMOTE), error %s"),soc_errmsg(rc))); 
                    } else if(serdes_xrloop) {
                        *loopback = soc_dnxc_loopback_mode_phy_rloop;
                    } else {

                        /*soc_dnxc_loopback_mode_none*/
                         *loopback = soc_dnxc_loopback_mode_none;
                    }
                }
            }
        }
    }

exit:
    if (locked) {
        MIIM_UNLOCK(unit);
    }
    DNXC_FUNC_RETURN;
}

/*
* Function:
*      jer2_arad_link_control_strip_crc_set
* Purpose:
*      Enable / Disable strip CRC
* Parameters:
*      unit      - (IN) Unit number.
*      port      - (IN) Device or logical port number 
*      strip_crc - (IN) 1- Enable strip CEC, 0 - Disable strip CRC
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_link_control_strip_crc_set(int unit, soc_port_t port, int strip_crc)
{
    uint32 inner_port;
    int blk_id, link;
    uint32 general_config;
    DNXC_INIT_FUNC_DEFS;

    link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link/4;
    inner_port = link % 4;

    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &general_config));
    soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &general_config, FMAL_N_ENABLE_CELL_CRCf, strip_crc ? 0 : 1);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, general_config));
    
exit:
    DNXC_FUNC_RETURN; 
}

/*
* Function:
*      jer2_arad_link_control_strip_crc_get
* Purpose:
*      Get strip CRC state
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  Device or logical port number 
*      strip_crc - (OUT) 1- Strip CEC enabled, 0 - Strip CRC disable
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_link_control_strip_crc_get(int unit, soc_port_t port, int* strip_crc)
{
    uint32 inner_port;
    int blk_id, link;
    uint32 general_config;
    DNXC_INIT_FUNC_DEFS;

    link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link/4;
    inner_port = link % 4;

    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &general_config));
    *strip_crc = !soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, general_config, FMAL_N_ENABLE_CELL_CRCf);
 
exit:
    DNXC_FUNC_RETURN; 
}

/*
* Function:
*      jer2_arad_link_control_rx_enable_set
* Purpose:
*      Enable\disable RX
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  port number 
*      enable    - (IN)  1- Enable 0- Disable
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_link_control_rx_enable_set(int unit, soc_port_t port, uint32 flags, int enable)
{
    uint32 inner_port;
    int blk_id, link;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;

    link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link/4;
    inner_port = link % 4;

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
    if(enable) {
        SHR_BITCLR(field_val,inner_port);
    } else {
        SHR_BITSET(field_val,inner_port);
    }

    soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf,*field_val);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,reg_val));

exit:
    DNXC_FUNC_RETURN; 
}

/*
* Function:
*      jer2_arad_link_control_tx_enable_set
* Purpose:
*      Enable\disable TX
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  port number 
*      enable    - (IN)  1- Enable 0- Disable
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_link_control_tx_enable_set(int unit, soc_port_t port, int enable)
{
    uint32 inner_port;
    int blk_id, link;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;

    link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link/4;
    inner_port = link % 4;

    MIIM_LOCK(unit);
  
    if (!enable) {
        /* a must when working with CL72 */
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_TX_LANE_SQUELCH, 1));
    }

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);
    if(enable) {
        SHR_BITCLR(field_val,inner_port);
    } else {
        SHR_BITSET(field_val,inner_port);
    }
    soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf,*field_val);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,reg_val));
    if (enable) {
        /* a must when working with CL72 */
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_TX_LANE_SQUELCH, 0));
    }

exit:
  MIIM_UNLOCK(unit);
  DNXC_FUNC_RETURN; 
}

/*
* Function:
*      jer2_arad_link_control_rx_enable_get
* Purpose:
*      Get RX status (Enabled\disabled)
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  port number 
*      enable    - (OUT) 1- Enable 0- Disable
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_link_control_rx_enable_get(int unit, soc_port_t port, int* enable)
{
    uint32 inner_port;
    int blk_id, link;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;

    link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link/4;
    inner_port = link % 4;

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
    *enable = SHR_BITGET(field_val,inner_port) ? 0 : 1;

exit:
    DNXC_FUNC_RETURN;
}

/*
* Function:
*      jer2_arad_link_control_tx_enable_get
* Purpose:
*      Get TX status (Enabled\disabled)
* Parameters:
*      unit      - (IN)  Unit number.
*      port      - (IN)  port number 
*      enable    - (OUT) 1- Enable 0- Disable
* Returns:
*      SOC_E_xxx
* Notes:
*/
soc_error_t 
jer2_arad_link_control_tx_enable_get(int unit, soc_port_t port, int* enable)
{
    uint32 inner_port;
    int blk_id, link;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;

    link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
    blk_id = link/4;
    inner_port = link % 4;

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);
    *enable = SHR_BITGET(field_val,inner_port) ? 0 : 1;

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_link_port_fault_get
 * Purpose:
 *      Get port loopback
 * Parameters:
 *      unit -  (IN)  BCM device number 
 *      port -  (IN)  Device or logical port number .
 *      flags - (OUT) Flags to indicate fault type 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
jer2_arad_link_port_fault_get(int unit, bcm_port_t link_id, uint32* flags)
{
  uint64 reg_64val; /* soc_reg_above_64_val_t in FE1600 */
  uint32 arr_64bits[2];
  DNXC_INIT_FUNC_DEFS;

  (*flags) = 0;
  DNXC_IF_ERR_EXIT(READ_RTP_LOCALLY_GENERATED_ACLr_REG64(unit, &reg_64val)); /* RTP_LINK_STATE_VECTORr in FE1600 */
  arr_64bits[1] = COMPILER_64_HI(reg_64val);
  arr_64bits[0] = COMPILER_64_LO(reg_64val);

  if(!SHR_BITGET(arr_64bits,link_id)) {
      (*flags) |= BCM_PORT_FAULT_LOCAL;
  }

  DNXC_IF_ERR_EXIT(READ_RTP_ACL_RECEIVEDr_REG64(unit, &reg_64val)); /* RTP_ACL_VECTORr in FE1600 */
  arr_64bits[1] = COMPILER_64_HI(reg_64val);
  arr_64bits[0] = COMPILER_64_LO(reg_64val);
  if(!SHR_BITGET(arr_64bits,link_id)) {
      (*flags) |= BCM_PORT_FAULT_REMOTE;
  }  

exit:
  DNXC_FUNC_RETURN;
}

soc_error_t
jer2_arad_link_port_bucket_fill_rate_validate(int unit, uint32 bucket_fill_rate)
{
  DNXC_INIT_FUNC_DEFS;

  if(bucket_fill_rate > JER2_ARAD_MAX_BUCKET_FILL_RATE) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("BUCKET_FILL_RATE: %d is out-of-ranget"), bucket_fill_rate));
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_fabric_links_llf_control_source_set
 * Purpose:
 *      Set LLFC control source
 * Parameters:
 *      unit     - (IN) Unit number.
 *      link     - (IN) Link
 *      val      - (IN) LLFC control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_llf_control_source_set(int unit, soc_port_t link, soc_dnxc_fabric_control_source_t val) 
{
  soc_port_t inner_lnk;
  uint32 blk_id, reg_val;
  uint32 field_val_rx[1], field_val_tx[1];
  DNXC_INIT_FUNC_DEFS;

  blk_id = link/4;
  inner_lnk = link % 4;

  /*Enable\disable LLFC*/
  DNXC_IF_ERR_EXIT(READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, blk_id, &reg_val));
  *field_val_rx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_RX_ENf);
  *field_val_tx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_TX_ENf);
  if(soc_dnxc_fabric_control_source_none == val) {
    SHR_BITCLR(field_val_rx, inner_lnk);
    SHR_BITCLR(field_val_tx, inner_lnk);
  } else {
    SHR_BITSET(field_val_rx, inner_lnk);
    SHR_BITSET(field_val_tx, inner_lnk);
  }
  soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val, LNK_LVL_FC_RX_ENf, *field_val_rx);
  soc_reg_field_set(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, &reg_val, LNK_LVL_FC_TX_ENf, *field_val_tx);
  DNXC_IF_ERR_EXIT(WRITE_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, blk_id, reg_val));

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_fe1600_fabric_links_llf_control_source_get
 * Purpose:
 *      Get LLFC control source
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      link     - (IN)  Link
 *      val      - (OUT) LLFC control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_llf_control_source_get(int unit, soc_port_t link, soc_dnxc_fabric_control_source_t* val) 
{
  soc_port_t inner_lnk;
  uint32 blk_id, reg_val;
  uint32 field_val_rx[1], field_val_tx[1];
  DNXC_INIT_FUNC_DEFS;

  blk_id = link/4;
  inner_lnk = link % 4;

  /*First check LLFC enablers*/
  DNXC_IF_ERR_EXIT(READ_FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr(unit, blk_id, &reg_val));
  *field_val_rx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_RX_ENf);
  *field_val_tx = soc_reg_field_get(unit, FMAC_LINK_LEVEL_FLOW_CONTROL_ENABLE_REGISTERr, reg_val, LNK_LVL_FC_TX_ENf);
  if(!SHR_BITGET(field_val_rx, inner_lnk) && !SHR_BITGET(field_val_tx, inner_lnk)) {
    *val = soc_dnxc_fabric_control_source_none;
  } else {
    DNXC_IF_ERR_EXIT(READ_ECI_GLOBALEr(unit, &reg_val));
    if(soc_reg_field_get(unit, ECI_GLOBALEr, reg_val, PARALLEL_DATA_PATHf)) {
      *val = soc_dnxc_fabric_control_source_both;
    } else {
      *val = soc_dnxc_fabric_control_source_primary;
    }

  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_fabric_links_isolate_set
 * Purpose:
 *      Activate / Isolate link
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link to activate / isolate
 *      val   - (IN) Activate / Isolate
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_isolate_set(int unit, soc_port_t link, soc_dnxc_isolation_status_t val)
{
  uint64 reachability_allowed_bm;
  uint64 specific_link_bm;
  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(READ_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit,&reachability_allowed_bm));

  COMPILER_64_SET(specific_link_bm,0,1);
  COMPILER_64_SHL(specific_link_bm,link);

  if(soc_dnxc_isolation_status_active == val)
  {
    /*turn on relevant link bit*/
    COMPILER_64_OR(reachability_allowed_bm,specific_link_bm); 
  }
  else
  {
    /* turn off relevant link bit*/
    COMPILER_64_NOT(specific_link_bm);
    COMPILER_64_AND(reachability_allowed_bm, specific_link_bm);
  }

  DNXC_IF_ERR_EXIT(WRITE_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit,reachability_allowed_bm));

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_fabric_links_isolate_set
 * Purpose:
 *      Get link state (Activated / Isolated)
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Link state (Activated / Isolated)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_isolate_get(int unit, soc_port_t link, soc_dnxc_isolation_status_t* val)
{
  uint64 reachability_allowed_bm;
  uint64 specific_link_bm;
  DNXC_INIT_FUNC_DEFS;

  DNXC_IF_ERR_EXIT(READ_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit,&reachability_allowed_bm));

  COMPILER_64_SET(specific_link_bm,0,1);
  COMPILER_64_SHL(specific_link_bm,link);

  COMPILER_64_AND(specific_link_bm,reachability_allowed_bm);
  if(!COMPILER_64_IS_ZERO(specific_link_bm))
    *val = soc_dnxc_isolation_status_active;
  else
    *val = soc_dnxc_isolation_status_isolated;

exit:
  DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      jer2_arad_fabric_links_cell_interleaving_set
 * Purpose:
 *      Set link interleaving mode
 * Parameters:
 *      unit  - (IN) Unit number.
 *      link  - (IN) Link
 *      val   - (IN) Is link in interleaving mode (0-false, 1-true)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_cell_interleaving_set(int unit, soc_port_t link, int val)
{
  uint32 interleaving_bm_32[1];
  uint32 blk_id;
  soc_port_t inner_lnk;
  DNXC_INIT_FUNC_DEFS;

  blk_id = (int)link/4;
  inner_lnk = link % 4;

  DNXC_IF_ERR_EXIT(READ_FMAC_CNTRL_INTRLVD_MODE_REGr(unit, blk_id, interleaving_bm_32));

  if(val)
  {
    /*turn on relevant link bit*/
    SHR_BITSET(interleaving_bm_32, inner_lnk);
  }
  else
  {
    /*turn off relevant link bit*/
    SHR_BITCLR(interleaving_bm_32, inner_lnk);
  }

  DNXC_IF_ERR_EXIT(WRITE_FMAC_CNTRL_INTRLVD_MODE_REGr(unit,blk_id, *interleaving_bm_32));

exit:
  DNXC_FUNC_RETURN;

}

/*
 * Function:
 *      jer2_arad_fabric_links_cell_interleaving_get
 * Purpose:
 *      Get link interleaving mode
 * Parameters:
 *      unit  - (IN)  Unit number.
 *      link  - (IN)  Link
 *      val   - (OUT) Is link in interleaving mode (0-false, 1-true)
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
jer2_arad_fabric_links_cell_interleaving_get(int unit, soc_port_t link, int* val)
{
  uint32 interleaving_bm_32[1];
  uint32 blk_id;
  soc_port_t inner_lnk;
  DNXC_INIT_FUNC_DEFS;

  blk_id = (int)link/4;
  inner_lnk = link % 4; 

  DNXC_IF_ERR_EXIT(READ_FMAC_CNTRL_INTRLVD_MODE_REGr(unit, blk_id, interleaving_bm_32));

  if(SHR_BITGET(interleaving_bm_32, inner_lnk))
  {
    *val = 1;
  }
  else
  {
    *val = 0;
  }   

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      jer2_arad_fabric_links_nof_links_get
 * Purpose:
 *      Get number of links
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      nof_links  - (OUT) Number of links.
 * Returns:
 *      Number of links
 */
soc_error_t 
jer2_arad_fabric_links_nof_links_get(int unit, int* nof_links)
{
  DNXC_INIT_FUNC_DEFS;

  DNXC_NULL_CHECK(nof_links);

  *nof_links = SOC_DNX_DEFS_GET(unit, nof_fabric_links);
exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     This procedure sets fabric links operation mode.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_fabric_cell_format_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_CELL_FORMAT  *info
  )
{
  uint32
    fld_val = 0,
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_FABRIC_CELL_FORMAT_SET_UNSAFE);

  DNX_SAND_CHECK_NULL_INPUT(info);


  fld_val = DNX_SAND_BOOL2NUM(info->segmentation_enable);
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDT_FDT_SEGMENTATION_AND_INTERLEAVINGr, REG_PORT_ANY, 0, SEGMENT_PKTf,  fld_val));

  

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_cell_format_set_unsafe()",0,0);
}

/*********************************************************************
*     This procedure gets fabric link status up\down.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_fabric_link_up_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_port_t          port,
    DNX_SAND_OUT int                 *up
  )
{
    uint32 
        link,
        is_down=0,
        reg_val,
        blk_id,
        inner_link,
        fld_val[1];
    soc_error_t
        rv;
    JER2_ARAD_LINK_STATE_INFO     info;

    DNXC_INIT_FUNC_DEFS;
  
    /*check if link is disabled*/
    rv = jer2_arad_link_on_off_get(unit, port, &info);
    DNXC_IF_ERR_EXIT(rv);

    /*clear rxlos interrupt*/
    if(info.on_off == DNX_TMC_LINK_STATE_ON) {
        link = SOC_DNX_FABRIC_PORT_TO_LINK(unit, port);
        blk_id = link/SOC_JER2_ARAD_NOF_LINKS_IN_MAC;
        inner_link = link % SOC_JER2_ARAD_NOF_LINKS_IN_MAC;
        reg_val = 0;
        *fld_val = (1 << inner_link);
        soc_reg_field_set(unit, FMAC_FMAC_INTERRUPT_REGISTER_2r, &reg_val, RX_LOST_OF_SYNCf, *fld_val);
        SOC_DNX_ALLOW_WARMBOOT_WRITE(WRITE_FMAC_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,reg_val),rv);
        DNXC_IF_ERR_EXIT(rv);

        /*check rxlos interrupt - link is up*/
        rv = READ_FMAC_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,&reg_val);
        DNXC_IF_ERR_EXIT(rv);
        *fld_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_2r, reg_val, RX_LOST_OF_SYNCf);
        is_down = SHR_BITGET(fld_val, inner_link);
        *up = (is_down == 0);
    } else {
        *up = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_fabric_nof_links_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT int                    *nof_links
  )
{
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_CHECK_NULL_INPUT(nof_links);

    *nof_links = SOC_DNX_DEFS_GET(unit, nof_fabric_links);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_fabric_nof_links_get()",0,0);
}

uint32 jer2_arad_fabric_gci_enable_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_GCI_TYPE                    type,
    DNX_SAND_OUT int                                    *enable
  )
{
    uint32
        res = DNX_SAND_OK;
    uint32 value;
    int i;
    soc_reg_t registers_gci_map_masking[7];
    soc_reg_t fields_gci_map_masking[7];
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    /*Create register and fields array of all the relevant FC mapping*/
    switch (type)
    {
       case JER2_ARAD_FABRIC_GCI_TYPE_LEAKY_BUCKET:
           /*GCI LB to IPT DTQ*/
           registers_gci_map_masking[0] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[0] = GCI_LB_2_IPT_DTQ_MC_FC_MAPf;
           /*GCI LB to IPT GFMC*/
           registers_gci_map_masking[1] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[1] = GCI_LB_2_IPT_GFMC_FC_MAPf;
           /*GCI LB to IPT BFMC*/
           registers_gci_map_masking[2] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[2] = GCI_LB_2_IPT_BFMC_FC_MAPf;
           /*GCI LB to IPS GFMC*/
           registers_gci_map_masking[3] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[3] = GCI_LB_2_IPS_GFMC_FC_MAPf;
           /*GCI LB to IPT BFMC 0*/
           registers_gci_map_masking[4] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[4] = GCI_LB_2_IPS_BFMC_0_FC_MAPf;
           /*GCI LB to IPT BFMC 1*/
           registers_gci_map_masking[5] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[5] = GCI_LB_2_IPS_BFMC_1_FC_MAPf;
           /*GCI LB to IPT BFMC 2*/
           registers_gci_map_masking[6] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[6] = GCI_LB_2_IPS_BFMC_2_FC_MAPf;
           break;
        case JER2_ARAD_FABRIC_GCI_TYPE_RANDOM_BACKOFF:
           /*GCI BCKOF to IPT DTQ*/
           registers_gci_map_masking[0] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[0] = GCI_BCKOF_2_IPT_DTQ_MC_FC_MAPf;
           /*GCI BCKOF to IPT GFMC*/
           registers_gci_map_masking[1] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[1] = GCI_BCKOF_2_IPT_GFMC_FC_MAPf;
           /*GCI BCKOF to IPT BFMC*/
           registers_gci_map_masking[2] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[2] = GCI_BCKOF_2_IPT_BFMC_FC_MAPf;
           /*GCI BCKOF to IPS GFMC*/
           registers_gci_map_masking[3] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[3] = GCI_BCKOF_2_IPS_GFMC_FC_MAPf;
           /*GCI BCKOF to IPT BFMC 0*/
           registers_gci_map_masking[4] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[4] = GCI_BCKOF_2_IPS_BFMC_0_FC_MAPf;
           /*GCI BCKOF to IPT BFMC 1*/
           registers_gci_map_masking[5] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[5] = GCI_BCKOF_2_IPS_BFMC_1_FC_MAPf;
           /*GCI BCKOF to IPT BFMC 2*/
           registers_gci_map_masking[6] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[6] = GCI_BCKOF_2_IPS_BFMC_2_FC_MAPf;
           break;
        default:
            DNX_SAND_SET_ERROR_CODE(DNX_TMC_INPUT_OUT_OF_RANGE, 70, exit);   
            break;
    }

    /*read the first map configuration*/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1000,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, registers_gci_map_masking[0], REG_PORT_ANY, 0, fields_gci_map_masking[0], &value));
    *enable = value ? 1 : 0;

    /*Make sure the GCI mask configuration is aligned to it*/
    for (i=0; i<7; i++)
    {
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  i,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, registers_gci_map_masking[i], REG_PORT_ANY, 0, fields_gci_map_masking[i], &value));
        if ((value ? 1 : 0) != *enable)
        {
            /*Mixed configuration of GCI mapping is not allowed*/
            DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 100 + i, exit);
        }
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_gci_enable_get()", 0, 0);
}

/*GCI LB default mapping*/
#define JER2_ARAD_FABRIC_GCI_LB_2_IPT_DTQ_DEFAULT_VALUE          (0xf)
#define JER2_ARAD_FABRIC_GCI_LB_2_IPT_GFMC_DEFAULT_VALUE         (0x8)
#define JER2_ARAD_FABRIC_GCI_LB_2_IPT_BFMC_DEFAULT_VALUE         (0xf)
#define JER2_ARAD_FABRIC_GCI_LB_2_IPS_GFMC_DEFAULT_VALUE         (0x8)
#define JER2_ARAD_FABRIC_GCI_LB_2_IPS_BFMC_0_DEFAULT_VALUE       (0xf)
#define JER2_ARAD_FABRIC_GCI_LB_2_IPS_BFMC_1_DEFAULT_VALUE       (0xe)
#define JER2_ARAD_FABRIC_GCI_LB_2_IPS_BFMC_2_DEFAULT_VALUE       (0xc)
/*GCI Backoff default mapping*/
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPT_DTQ_DEFAULT_VALUE       (0x7)
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPT_GFMC_DEFAULT_VALUE      (0x4)
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPT_BFMC_DEFAULT_VALUE      (0x7)
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_GFMC_DEFAULT_VALUE      (0x4)
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_BFMC_0_DEFAULT_VALUE    (0x7)
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_BFMC_1_DEFAULT_VALUE    (0x7)
#define JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_BFMC_2_DEFAULT_VALUE    (0x6)

uint32 jer2_arad_fabric_gci_enable_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN JER2_ARAD_FABRIC_GCI_TYPE                    type,
    DNX_SAND_IN int                                     enable
  )
{
    uint32
        res = DNX_SAND_OK;
    int i;
    soc_reg_t registers_gci_map_masking[7];
    soc_reg_t fields_gci_map_masking[7];
    soc_reg_t values_gci_map_masking[7];
    
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);



    /*Create register, fields and default values array of all the relevant FC mapping*/
    switch (type)
    {
       case JER2_ARAD_FABRIC_GCI_TYPE_LEAKY_BUCKET:
           /*GCI LB to IPT DTQ*/
           registers_gci_map_masking[0] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[0] = GCI_LB_2_IPT_DTQ_MC_FC_MAPf;
           values_gci_map_masking[0] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPT_DTQ_DEFAULT_VALUE : 0;
           /*GCI LB to IPT GFMC*/
           registers_gci_map_masking[1] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[1] = GCI_LB_2_IPT_GFMC_FC_MAPf;
           values_gci_map_masking[1] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPT_GFMC_DEFAULT_VALUE : 0;
           /*GCI LB to IPT BFMC*/
           registers_gci_map_masking[2] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[2] = GCI_LB_2_IPT_BFMC_FC_MAPf;
           values_gci_map_masking[2] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPT_BFMC_DEFAULT_VALUE : 0;
           /*GCI LB to IPS GFMC*/
           registers_gci_map_masking[3] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[3] = GCI_LB_2_IPS_GFMC_FC_MAPf;
           values_gci_map_masking[3] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPS_GFMC_DEFAULT_VALUE : 0;
           /*GCI LB to IPT BFMC 0*/
           registers_gci_map_masking[4] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[4] = GCI_LB_2_IPS_BFMC_0_FC_MAPf;
           values_gci_map_masking[4] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPS_BFMC_0_DEFAULT_VALUE : 0;
           /*GCI LB to IPT BFMC 1*/
           registers_gci_map_masking[5] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[5] = GCI_LB_2_IPS_BFMC_1_FC_MAPf;
           values_gci_map_masking[5] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPS_BFMC_1_DEFAULT_VALUE : 0;
           /*GCI LB to IPT BFMC 2*/
           registers_gci_map_masking[6] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[6] = GCI_LB_2_IPS_BFMC_2_FC_MAPf;
           values_gci_map_masking[6] = enable ? JER2_ARAD_FABRIC_GCI_LB_2_IPS_BFMC_2_DEFAULT_VALUE : 0;
           break;
        case JER2_ARAD_FABRIC_GCI_TYPE_RANDOM_BACKOFF:
           /*GCI BCKOF to IPT DTQ*/
           registers_gci_map_masking[0] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[0] = GCI_BCKOF_2_IPT_DTQ_MC_FC_MAPf;
           values_gci_map_masking[0] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPT_DTQ_DEFAULT_VALUE : 0;
           /*GCI BCKOF to IPT GFMC*/
           registers_gci_map_masking[1] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[1] = GCI_BCKOF_2_IPT_GFMC_FC_MAPf;
           values_gci_map_masking[1] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPT_GFMC_DEFAULT_VALUE : 0;
           /*GCI BCKOF to IPT BFMC*/
           registers_gci_map_masking[2] = (SOC_IS_QAX(unit) ? PTS_IPT_INTRNL_FMC_FC_MAPr : IPT_IPT_INTRNL_FMC_FC_MAPr);
           fields_gci_map_masking[2] = GCI_BCKOF_2_IPT_BFMC_FC_MAPf;
           values_gci_map_masking[2] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPT_BFMC_DEFAULT_VALUE : 0;
           /*GCI BCKOF to IPS GFMC*/
           registers_gci_map_masking[3] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[3] = GCI_BCKOF_2_IPS_GFMC_FC_MAPf;
           values_gci_map_masking[3] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_GFMC_DEFAULT_VALUE : 0;
           /*GCI BCKOF to IPT BFMC 0*/
           registers_gci_map_masking[4] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[4] = GCI_BCKOF_2_IPS_BFMC_0_FC_MAPf;
           values_gci_map_masking[4] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_BFMC_0_DEFAULT_VALUE : 0;
           /*GCI BCKOF to IPT BFMC 1*/
           registers_gci_map_masking[5] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[5] = GCI_BCKOF_2_IPS_BFMC_1_FC_MAPf;
           values_gci_map_masking[5] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_BFMC_1_DEFAULT_VALUE : 0;
           /*GCI BCKOF to IPT BFMC 2*/
           registers_gci_map_masking[6] = (SOC_IS_QAX(unit) ? PTS_IPT_FMC_IPS_FC_MAPr : IPT_IPT_FMC_IPS_FC_MAP_1r);
           fields_gci_map_masking[6] = GCI_BCKOF_2_IPS_BFMC_2_FC_MAPf;
           values_gci_map_masking[6] = enable ? JER2_ARAD_FABRIC_GCI_BCKOF_2_IPS_BFMC_2_DEFAULT_VALUE : 0;
           break;
        default:
            DNX_SAND_SET_ERROR_CODE(DNX_TMC_INPUT_OUT_OF_RANGE, 70, exit);   
            break;
    }

    /*Config Default mapping*/
    for (i=0; i<7; i++)
    {
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  i,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, registers_gci_map_masking[i], REG_PORT_ANY, 0, fields_gci_map_masking[i],  values_gci_map_masking[i]));
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_gci_enable_set()", 0, 0);
}

uint32 jer2_arad_fabric_gci_config_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN JER2_ARAD_FABRIC_GCI_CONFIG_TYPE             type,
    DNX_SAND_OUT int                                    *value
  )
{
    uint32 res = DNX_SAND_OK;
    soc_reg_t gci_backoff_range_thresholds_reg = (SOC_IS_QAX(unit) ? PTS_GCI_BACKOFF_RANGE_THRESHOLDSr : IPT_GCI_BACKOFF_RANGE_THRESHOLDSr);
    soc_reg_t gci_leaky_bucket_configuration_register_0_reg = (SOC_IS_QAX(unit) ? PTS_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_0r : IPT_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_0r);
    soc_reg_t gci_leaky_bucket_configuration_register_2_reg = (SOC_IS_QAX(unit) ? PTS_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_2r : IPT_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_2r);
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);


    switch (type)
    {
        /*GCI backoff thresholda*/
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_0:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, gci_backoff_range_thresholds_reg, REG_PORT_ANY, 0, CNGST_LVL_THRESH_0f, (uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_1:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, gci_backoff_range_thresholds_reg, REG_PORT_ANY, 0, CNGST_LVL_THRESH_1f, (uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_2:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, gci_backoff_range_thresholds_reg, REG_PORT_ANY, 0, CNGST_LVL_THRESH_2f, (uint32*)value));
            break;

        /*GCI Leaky Bucket congested*/
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_1f,(uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_2f,(uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_3f,(uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  7,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_4f,(uint32*)value));
            break;

         /*GCI Leaky Bucket full*/
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_1f,(uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  9,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_2f,(uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_3f,(uint32*)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_4f,(uint32*)value));
            break;


       default:
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_OUT_OF_RANGE, 70, exit);   
            break;
    }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_gci_config_get()", 0, 0);
}

uint32 jer2_arad_fabric_gci_config_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN JER2_ARAD_FABRIC_GCI_CONFIG_TYPE            type,
    DNX_SAND_IN int                                    value
  )
{
    uint32 res = DNX_SAND_OK;
    soc_reg_t gci_backoff_range_thresholds_reg = (SOC_IS_QAX(unit) ? PTS_GCI_BACKOFF_RANGE_THRESHOLDSr : IPT_GCI_BACKOFF_RANGE_THRESHOLDSr);
    soc_reg_t gci_leaky_bucket_configuration_register_0_reg = (SOC_IS_QAX(unit) ? PTS_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_0r : IPT_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_0r);
    soc_reg_t gci_leaky_bucket_configuration_register_2_reg = (SOC_IS_QAX(unit) ? PTS_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_2r : IPT_GCI_LEAKY_BUCKET_CONFIGURATION_REGISTER_2r);
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    /*Verify*/
    switch (type)
    {
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_0:
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_1:
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_2:
            DNX_SAND_ERR_IF_ABOVE_MAX(
            value, JER2_ARAD_FABRIC_IPT_GCI_BACKOFF_CONGESTION_LEVEL_MAX,
            DNX_TMC_INPUT_OUT_OF_RANGE, 100, exit
            );
            break;

       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_CONGESTION_TH:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_CONGESTION_TH:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_CONGESTION_TH:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_CONGESTION_TH:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_FULL:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_FULL:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_FULL:
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_FULL:
           DNX_SAND_ERR_IF_ABOVE_MAX(
            value, JER2_ARAD_FABRIC_GCI_BUCKET_MAX,
            DNX_TMC_INPUT_OUT_OF_RANGE, 101, exit
           );
            break;

       default:
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_OUT_OF_RANGE, 70, exit);   
            break;
    }

    /*Configure*/
    switch (type)
    {
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_0:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_backoff_range_thresholds_reg,REG_PORT_ANY,0, CNGST_LVL_THRESH_0f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_1:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_backoff_range_thresholds_reg,REG_PORT_ANY,0, CNGST_LVL_THRESH_1f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_BACKOFF_TH_2:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_backoff_range_thresholds_reg,REG_PORT_ANY,0, CNGST_LVL_THRESH_2f, (uint32)value));
            break;

        /*GCI Leaky Bucket congested*/
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_1f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  5,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_2f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  6,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_3f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_CONGESTION_TH:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  7,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_2_reg,REG_PORT_ANY,0, LKY_BKT_CNG_TH_4f, (uint32)value));
            break;

         /*GCI Leaky Bucket full*/
       case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_1_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  8,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_1f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_2_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  9,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_2f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_3_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_3f, (uint32)value));
            break;
        case JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_LB_4_FULL:
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit,gci_leaky_bucket_configuration_register_0_reg,REG_PORT_ANY,0, LKY_BKT_MAX_CNT_4f, (uint32)value));
            break;
        /* must default. Otherwise - compilation error. */
        /*coverity[dead_error_begin:FALSE] */
        default:
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FABRIC_GCI_CONFIG_TYPE_OUT_OF_RANGE, 70, exit);   
            break;
    }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_gci_config_set()", 0, 0);
}


uint32 jer2_arad_fabric_llfc_threshold_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_OUT int                                    *value
  )
{
    uint32
        res = DNX_SAND_OK;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    *value=0;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit,FDR_LINK_LEVEL_FLOW_CONTROLr,REG_PORT_ANY,0, LNK_LVL_FC_THf,(uint32*)value));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_llfc_threshold_get()", 0, 0);
}

uint32 jer2_arad_fabric_llfc_threshold_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN int                                    value
  )
{
    uint32
        res = DNX_SAND_OK;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    DNX_SAND_ERR_IF_ABOVE_MAX(
       value, JER2_ARAD_FABRIC_LLFC_RX_MAX,
       DNX_TMC_INPUT_OUT_OF_RANGE, 10, exit
     );

    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_LINK_LEVEL_FLOW_CONTROLr, REG_PORT_ANY, 0, LNK_LVL_FC_THf,  value));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_llfc_threshold_set()", 0, 0);
}

uint32 jer2_arad_fabric_minimal_links_to_dest_set(
    DNX_SAND_IN int                                  unit,
    DNX_SAND_IN soc_module_t                         module_id,
    DNX_SAND_IN int                                  minimum_links
   )
{
   DNXC_INIT_FUNC_DEFS;
    
   if (SOC_IS_ARDON(unit)) {
       DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, RTP_MIN_NUM_OF_LINKS_PER_FAPr, REG_PORT_ANY, 0, MIN_NUM_OF_LINKS_PER_FAPf,  minimum_links)); 
       DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, RTP_MIN_NUM_OF_LINKS_PER_FAPr, REG_PORT_ANY, 0, MIN_NUM_OF_LINKS_PER_FAP_ENf,  minimum_links? 1:0));
   } else {
       DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, RTP_SPARE_REGISTER_2r, REG_PORT_ANY, 0, ECO_MIN_NOF_LINKSf,  minimum_links)); 
       DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, RTP_SPARE_REGISTER_2r, REG_PORT_ANY, 0, ECO_ENABLEDf,  minimum_links? 1:0));
   }

exit:
  DNXC_FUNC_RETURN;
}

uint32 jer2_arad_fabric_minimal_links_to_dest_get(
    DNX_SAND_IN int                                 unit,
    DNX_SAND_IN soc_module_t                        module_id,
    DNX_SAND_OUT int                                *minimum_links
    )
{
  uint32
        res = DNX_SAND_OK;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);
  if (SOC_IS_ARDON(unit)) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 1, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, RTP_MIN_NUM_OF_LINKS_PER_FAPr, REG_PORT_ANY, 0, MIN_NUM_OF_LINKS_PER_FAPf, (uint32*)minimum_links));
  } else {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res, 1, exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, RTP_SPARE_REGISTER_2r, REG_PORT_ANY, 0, ECO_MIN_NOF_LINKSf, (uint32*)minimum_links));
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_minimal_links_to_dest_get()",0,0);
    
}

uint32 jer2_arad_fabric_rci_enable_get ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_OUT soc_dnxc_fabric_control_source_t       *value
  )
{
    uint32
        res = DNX_SAND_OK;
    uint32 fap_rci_en;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    *value=0;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_RCI_PARAMSr, SOC_CORE_ALL, 0, DEVICE_RCI_ENAf, &fap_rci_en));

    /*Enable/Disable per pipe is not supported*/
    if (fap_rci_en)
    {
        *value = soc_dnxc_fabric_control_source_both;
    } else {
        *value = soc_dnxc_fabric_control_source_none;
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_rci_enable_get()", 0, 0);
}

uint32 jer2_arad_fabric_rci_enable_set ( 
    DNX_SAND_IN  int                                 unit,  
    DNX_SAND_IN soc_dnxc_fabric_control_source_t        value
  )
{
    int fap_rci_en = 0;
    uint32
        res = DNX_SAND_OK;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   /*Verify*/
    switch (value)
    {
       case soc_dnxc_fabric_control_source_none:
           fap_rci_en = 0;
           break;
       case soc_dnxc_fabric_control_source_both:
           fap_rci_en = 1;
           break;
       default:
           /*Enable/Disable per pipe is not supported*/
           DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 10, exit);
           break;

    }

    /*Configure registers*/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_RCI_PARAMSr, SOC_CORE_ALL, 0, DEVICE_RCI_ENAf,  fap_rci_en));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_rci_enable_set()", 0, 0);
}

uint32 jer2_arad_fabric_rci_config_get ( 
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_FABRIC_RCI_CONFIG_TYPE            rci_config_type,  
    DNX_SAND_OUT int                                    *value
  )
{
    uint32
        res = DNX_SAND_OK;
    int value_low, value_high;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    *value=0;
    switch (rci_config_type)
    {
       /*Local RCI threshold*/
       case DNX_TMC_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH:
        /* 
         *RCI is a 2 bit indication
         *LOW threshold invokes the LSB and HIGH threshold invokes the MSB
         *However, SCH block ignores the level LOW or HIGH.
         */
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FDR_LOCAL_FIFO_RCI_LEVELr, REG_PORT_ANY, 0, RCI_LOW_LEVELf, (uint32*)&value_low));
        DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, FDR_LOCAL_FIFO_RCI_LEVELr, REG_PORT_ANY, 0, RCI_HIGH_LEVELf, (uint32*)&value_high));

        if (value_low != value_high)
        {
            DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 10, exit);
        }
        *value = value_low;
        break;

       /*RCI bucket increment value*/
       case DNX_TMC_FABRIC_RCI_CONFIG_TYPE_INCREMENT_VALUE:
           DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, SCH_RCI_PARAMSr, SOC_CORE_ALL, 0, RCI_INC_VALf, (uint32*)value));
           break;

       default:
           DNX_SAND_SET_ERROR_CODE(DNX_TMC_INPUT_OUT_OF_RANGE, 100, exit);   
           break;
    }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_rci_config_get()", 0, 0);
}

uint32 jer2_arad_fabric_rci_config_set ( 
    DNX_SAND_IN  int                                unit,  
    DNX_SAND_IN  JER2_ARAD_FABRIC_RCI_CONFIG_TYPE           rci_config_type,
    DNX_SAND_IN int                                    value
  )
{
    uint32
        res = DNX_SAND_OK;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    switch (rci_config_type)
    {
       /*Local RCI threshold*/
       case DNX_TMC_FABRIC_RCI_CONFIG_TYPE_LOCAL_RX_TH:
           DNX_SAND_ERR_IF_ABOVE_MAX(
               value, JER2_ARAD_FABRIC_RCI_THRESHOLD_MAX,
               DNX_TMC_INPUT_OUT_OF_RANGE, 10, exit
           );

            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_LOCAL_FIFO_RCI_LEVELr, REG_PORT_ANY, 0, RCI_LOW_LEVELf,  value));
            DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FDR_LOCAL_FIFO_RCI_LEVELr, REG_PORT_ANY, 0, RCI_HIGH_LEVELf,  value));
            break;

       /*RCI bucket increment value*/
       case DNX_TMC_FABRIC_RCI_CONFIG_TYPE_INCREMENT_VALUE:
           DNX_SAND_ERR_IF_ABOVE_MAX(
               value, JER2_ARAD_FABRIC_RCI_INC_VAL_MAX,
               DNX_TMC_INPUT_OUT_OF_RANGE, 10, exit
           );
           DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  11,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, SCH_RCI_PARAMSr, SOC_CORE_ALL, 0, RCI_INC_VALf,  value));
           break;

       default:
           DNX_SAND_SET_ERROR_CODE(DNX_TMC_INPUT_OUT_OF_RANGE, 100, exit);   
           break;
    }
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_rci_config_set()", 0, 0);
}

#ifdef BCM_88660_A0
/*********************************************************************
* NAME:
*     jer2_arad_fabric_empty_cell_size_set
* FUNCTION:
*   Empty cell and LLFC cell size configuration.
*   Available per FMAC instance.
*   Supported by JER2_ARAD_PLUS only.
*   The remote reapeater can process a limited number of cells (according to remote reapeater core clock),
*   Setting larger cell size will reduce the empty cells rate
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  DNX_SAND_IN  uint32                 fmac_index -
*     FMAC #
*DNX_SAND_IN  uint32                    cell_size - 
*     The empty cells size in bytes.          
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
jer2_arad_fabric_empty_cell_size_set (
   DNX_SAND_IN  int                                 unit,
   DNX_SAND_IN  uint32                                 fmac_index,  
   DNX_SAND_IN  uint32                                 cell_size
   )
{
    uint32
        res = DNX_SAND_OK;
    uint32 cell_size_hw_format;
    DNX_SAND_INIT_ERROR_DEFINITIONS(0);

    

    /*Supported only by JER2_ARAD_PLUS*/
    if (SOC_IS_ARAD_B1_AND_BELOW(unit))
    {
        DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 1, exit);
    }

    /*Verify*/
    DNX_SAND_ERR_IF_BELOW_MIN (
        cell_size, JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_MIN,
        DNX_TMC_INPUT_OUT_OF_RANGE, 10, exit
    );
    DNX_SAND_ERR_IF_ABOVE_MAX (
        cell_size, JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_MAX,
        DNX_TMC_INPUT_OUT_OF_RANGE, 20, exit
    );

    /*Configure registers*/
    cell_size_hw_format = cell_size - JER2_ARAD_FABRIC_EMPTY_CELL_SIZE_MIN;
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  30,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_SPARE_REGISTER_2r,  fmac_index,  0, EMPTY_CELL_SIZEf,  cell_size_hw_format)); /*Enable enhance shaper mode*/
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  40,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, FMAC_SPARE_REGISTER_2r,  fmac_index,  0, OVERRIDE_EMPTY_CELL_SIZEf,  0x1)); /*Enable cell size configuration*/

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_fabric_empty_cell_size_set()", 0, 0);

}
#endif /*BCM_88660_A0*/



/*********************************************************************
* NAME:
*     jer2_arad_fabric_aldwp_config
* FUNCTION:
*   Configure ALDWP value
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
int 
jer2_arad_fabric_aldwp_config(
   DNX_SAND_IN  int                                 unit)
{
    uint32 max_cell_size, aldwp, highest_aldwp;
    uint32 core_clock_speed;
    int speed;
    soc_port_t port_index;
    uint64 reg64_val;
    uint32 rv;
    int enable;
    soc_field_t aldwp_field;

    DNXC_INIT_FUNC_DEFS;

    /* 
     * Calc the max number of clock ticks to receive 3 cells:
     *  
     *          3 *  max cell size * core clock
     *  aldwp =   ------------------------------
     *                   link rate
     *  
     * Note: should find the highest aldwp at the specific FDR instance 
     */

    /*Reterive the relevant parameters: link rate and cell_format*/

    /*Find the highest aldwp over a specific FDR*/


    /*Reterive global configuration*/
    if (SOC_DNX_CONFIG(unit)->jer2_arad->init.fabric.is_fe600) {
        max_cell_size = JER2_ARAD_FABRIC_VSC128_MAX_CELL_SIZE;
    } else {
        max_cell_size = SOC_DNX_DEFS_GET(unit, fabric_vsc256_max_cell_size);
    }
    core_clock_speed = jer2_arad_chip_kilo_ticks_per_sec_get(unit);


    highest_aldwp = 0;
    for (port_index = SOC_DNX_FABRIC_LINK_TO_PORT(unit, 0); 
          port_index < SOC_DNX_FABRIC_LINK_TO_PORT(unit, SOC_DNX_DEFS_GET(unit, nof_fabric_links)); 
          port_index++)
    {

        if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_index))
        {
            /*port is shut down*/
            continue;
        }

        rv = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_port_enable_get, (unit, port_index, 0, &enable));
        DNXC_IF_ERR_EXIT(rv);
        if (!enable) {
            /*port is disabled*/
            continue;
        }
        
        rv = MBCM_DNX_SOC_DRIVER_CALL(unit, mbcm_dnx_port_speed_get, (unit, port_index, &speed));
        DNXC_IF_ERR_EXIT(rv);
        if (speed == 0) {
            /*port is disabled*/
            continue;
        }

        /*Calc the max number of tiks to receive 3 cells*/
        aldwp = (3* max_cell_size * core_clock_speed) / (speed / 8 /*bits to bytes*/); /*mili tiks*/
        aldwp = aldwp /(1024*64) /*units of 64 ticks*/ + ((aldwp % (1024*64) != 0) ? 1 : 0) /*round up*/; 

        /*update MAX*/
        highest_aldwp = (aldwp > highest_aldwp ? aldwp : highest_aldwp);  /* +3 to be on the safe side with possible delays from fe2/fe3*/  
    }

    if (highest_aldwp != 0 /*at least one port is enabled*/) {
        highest_aldwp += JER2_ARAD_FABRIC_ALDWP_FAP_OFFSET;
    }


    /*Check reterived values*/
    if (highest_aldwp == 0)
    {
        /*All port are disabled or powered down*/
        SOC_EXIT;
    } else if (highest_aldwp < JER2_ARAD_FABRIC_ALDWP_MIN)
    {
        /*hightest_aldwp may be lower than JER2_ARAD_FABRIC_ALDWP_MIN*/
        /* coverity[dead_error_line:FALSE] */
        highest_aldwp = JER2_ARAD_FABRIC_ALDWP_MIN;
    } else if (highest_aldwp > JER2_ARAD_FABRIC_ALDWP_MAX)
    {
        highest_aldwp = JER2_ARAD_FABRIC_ALDWP_MAX;
    }

    /*Configure relevant register*/
    DNXC_IF_ERR_EXIT(READ_FDR_FDR_ENABLERS_REGISTER_1r(unit, &reg64_val));
    aldwp_field = ( (SOC_IS_QAX(unit) || SOC_IS_JERICHO_PLUS_A0(unit)) ? FIELD_0_3f : FIELD_19_22f);
    soc_reg64_field32_set(unit, FDR_FDR_ENABLERS_REGISTER_1r , &reg64_val, aldwp_field, highest_aldwp);
    DNXC_IF_ERR_EXIT(WRITE_FDR_FDR_ENABLERS_REGISTER_1r(unit, reg64_val));

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     jer2_arad_fabric_link_tx_traffic_disable_set
* FUNCTION:
*   Allow/do not allow to send traffic on a specific link.
* INPUT:
*      unit - unit #
*      link - link id #
*      disable - 1: do not allow, 0: allow  
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

soc_error_t
jer2_arad_fabric_link_tx_traffic_disable_set(int unit, soc_port_t link, int disable)
{
    int rv;
    uint64 reg64_val;
    DNXC_INIT_FUNC_DEFS;

    /*Disable/enable UC cells*/
    rv = READ_RTP_ALLOWED_LINKSr_REG64(unit, &reg64_val);
    DNXC_IF_ERR_EXIT(rv);
    if (disable)
    {
        COMPILER_64_BITCLR(reg64_val, link);
    } else {
        COMPILER_64_BITSET(reg64_val, link);
    }
    rv = WRITE_RTP_ALLOWED_LINKSr_REG64(unit, reg64_val);
    DNXC_IF_ERR_EXIT(rv);

    /*Disable/enable MC cells*/
    rv = READ_RTP_MULTICAST_LINK_UPr_REG64(unit, &reg64_val);
    DNXC_IF_ERR_EXIT(rv);
    if (disable)
    {
        COMPILER_64_BITCLR(reg64_val, link);
    } else {
        COMPILER_64_BITSET(reg64_val, link);
    }
    rv = WRITE_RTP_MULTICAST_LINK_UPr_REG64(unit, reg64_val);
    DNXC_IF_ERR_EXIT(rv);


exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
* NAME:
*     jer2_arad_fabric_link_tx_traffic_disable_set
* FUNCTION:
*   Get allow/do not allow to send traffic on a specific link.
* INPUT:
*      unit - unit #
*      link - link id #
*      disable (OUT)- 1: do not allow, 0: allow  
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/

soc_error_t
jer2_arad_fabric_link_tx_traffic_disable_get(int unit, soc_port_t link, int *disable)
{
    int rv;
    uint64 reg64_val;
    DNXC_INIT_FUNC_DEFS;

    /*Assuming RTP_ALLOWED_LINKS and RTP_MULTICAST_LINK_UP are synced*/
    rv = READ_RTP_ALLOWED_LINKSr_REG64(unit, &reg64_val);
    DNXC_IF_ERR_EXIT(rv);
    
    *disable = COMPILER_64_BITTEST(reg64_val, link) ? 0 : 1;

exit:
    DNXC_FUNC_RETURN;
}



/*********************************************************************
* NAME:
*     jer2_arad_fabric_mesh_check
* FUNCTION:
*     check mesh status
* INPUT:
*  DNX_SAND_IN  int                                  unit -
*     Identifier of the device to access.
*  DNX_SAND_IN uint8                                    stand_alone,
*     Is device stand alone.
*  DNX_SAND_OUT uint8 *success - 
*     mesh status check.
* RETURNS:
*   OK or ERROR indication.
* REMARKS: 
*   None. 
*********************************************************************/

uint32 
  jer2_arad_fabric_mesh_check(
    DNX_SAND_IN  int                                     unit, 
    DNX_SAND_IN uint8                                    stand_alone,
    DNX_SAND_OUT uint8                                   *success
  )
{
    uint32 buffer_1, buffer_2;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, MESH_TOPOLOGY_MESH_STATUSESr, REG_PORT_ANY, 0, MESH_STATUS_2f, &buffer_1));
    DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, MESH_TOPOLOGY_THRESHOLDSr, REG_PORT_ANY, 0, THRESHOLD_0f, &buffer_2));

    if ((buffer_1 < buffer_2) || (stand_alone == TRUE))
    {
      *success = TRUE;
    }
    else
    {
    *success = FALSE; 
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
jer2_arad_fabric_prbs_polynomial_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    int rv;
    DNXC_INIT_FUNC_DEFS;

    if(mode){ /*MAC*/
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Polynomial set isn't supported for MAC PRBS\n")));
    }else { /*PHY*/
        switch(value){
        case SOC_PHY_PRBS_POLYNOMIAL_X7_X6_1:
        case SOC_PHY_PRBS_POLYNOMIAL_X15_X14_1:
        case SOC_PHY_PRBS_POLYNOMIAL_X23_X18_1:
        case SOC_PHY_PRBS_POLYNOMIAL_X31_X28_1:
            MIIM_LOCK(unit);
            rv = soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRBS_POLYNOMIAL, value);
            MIIM_UNLOCK(unit);
            DNXC_IF_ERR_EXIT(rv);
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid value %d"), value));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
jer2_arad_fabric_prbs_polynomial_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    int rv;
    DNXC_INIT_FUNC_DEFS;

    if(mode){ /*MAC*/
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Polynomial get isn't supported for MAC PRBS\n")));
    }else { /*PHY*/
        MIIM_LOCK(unit);
        rv = soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_PRBS_POLYNOMIAL, (uint32*)value);
        MIIM_UNLOCK(unit);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */


