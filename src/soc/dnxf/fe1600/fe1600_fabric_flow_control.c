/*
 * $Id: ramon_fe1600_fabric_flow_control.c,v 1.11 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 FABRIC FLOW CONTROL
 */
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC

#include <shared/bsl.h>

#include <soc/dnxc/legacy/error.h>

#include <bcm/types.h>
#include <soc/defs.h>

#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>
#include <soc/error.h>
#include <soc/dnxf/cmn/dnxf_drv.h>

#include <shared/bitop.h>

#if defined(BCM_88790_A0)
 
#include <soc/dnxf/fe1600/fe1600_fabric_flow_control.h>
#include <soc/dnxf/fe1600/fe1600_defs.h>

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rci_gci_control_source_set
 * Purpose:
 *      Set control source for RCI / GCI
 * Parameters:
 *      unit - (IN) Unit number.
 *      type - (IN) bcmFabricRCIControlSource or bcmFabricGCIControlSource
 *      val  - (IN) Control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rci_gci_control_source_set(int unit, bcm_fabric_control_t type, soc_dnxc_fabric_pipe_t val) 
{
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);
    
    /* 0:primary 
       1:secondary
       2:primary | secondary  
    */
   
    DNXC_IF_ERR_EXIT(READ_DCMC_DCM_ENABLERS_REGISTERr(unit, &reg_val));
    
    switch(type)
    {
        case bcmFabricRCIControlSource:
            soc_reg_field_set(unit, DCMC_DCM_ENABLERS_REGISTERr, &reg_val, RCI_SOURCEf, val-1); /* -1 because primary is 0, secondary 1 , primary|scondary 2*/
            break;
        
        case bcmFabricGCIControlSource:
            soc_reg_field_set(unit, DCMC_DCM_ENABLERS_REGISTERr, &reg_val, GCI_SOURCEf, val-1);
            break;
        
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong control source type %d"),type)); 
    }
    
    DNXC_IF_ERR_EXIT(WRITE_DCMC_DCM_ENABLERS_REGISTERr(unit, reg_val));
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rci_gci_control_source_get
 * Purpose:
 *      Get control source for RCI / GCI
 * Parameters:
 *      unit - (IN)  Unit number.
 *      type - (IN)  bcmFabricRCIControlSource or bcmFabricGCIControlSource
 *      val  - (OUT) Control source
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rci_gci_control_source_get(int unit, bcm_fabric_control_t type, soc_dnxc_fabric_pipe_t* val) 
{
    uint32 reg_val, enabler_val,mask=0;
    int nof_pipes;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    nof_pipes=SOC_DNXF_FABRIC_PIPES_CONFIG(unit).nof_pipes;
    SOC_DNXC_FABRIC_PIPE_ALL_PIPES_SET(&mask,nof_pipes); /* creating a mask */
    
    DNXC_IF_ERR_EXIT(READ_DCMC_DCM_ENABLERS_REGISTERr(unit, &reg_val));
      
    switch(type)
    {
        case bcmFabricRCIControlSource:
            enabler_val = soc_reg_field_get(unit, DCMC_DCM_ENABLERS_REGISTERr, reg_val, RCI_SOURCEf);
            break;
        
        case bcmFabricGCIControlSource:
            enabler_val = soc_reg_field_get(unit, DCMC_DCM_ENABLERS_REGISTERr, reg_val, GCI_SOURCEf);
            break;
        
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong control source type %d"),type)); 
    }
    
    /* 0:primary 
       1:secondary
       2:primary | secondary  
    */
    enabler_val=SOC_DNXC_FABRIC_PIPE_ALL_PIPES_GET(enabler_val,mask);
    *val=enabler_val+1;

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_fabric_flow_control_thresholds_flags_validate(int unit,uint32 flags){
    uint32 valid_flags= BCM_FABRIC_LINK_TH_FE1_LINKS_ONLY | BCM_FABRIC_LINK_TH_FE3_LINKS_ONLY | BCM_FABRIC_LINK_TH_PRIM_ONLY |
                        BCM_FABRIC_LINK_TH_SCND_ONLY | BCM_FABRIC_LINK_THRESHOLD_WITH_ID | BCM_FABRIC_LINK_THRESHOLD_RX_FIFO_ONLY |
                        BCM_FABRIC_LINK_THRESHOLD_TX_FIFO_ONLY;
    DNXC_INIT_FUNC_DEFS;

    if (~valid_flags & flags )
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid flags for threshold handle")));
    }
      
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_llfc_threshold_validate
 * Purpose:
 *      Set RX LLFC threshold
 * Parameters:
 *      unit       - (IN) Unit number.
 *      type_index - (IN) Selected index(0/1)
 *      pipe       - (IN) Selected pipe
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */      
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_llfc_threshold_validate(int unit, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int value)
{
    soc_error_t rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    { 
        if(type_index == soc_dnxf_fabric_link_fifo_type_index_0){
            rc = soc_reg_field_validate(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Pr, LNK_LVL_FC_TH_0_Pf, value);
        } else {
            rc = soc_reg_field_validate(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Pr, LNK_LVL_FC_TH_1_Pf, value); 
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for RX LLFC threshold"),value)); 
    }
    
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {       
        if(type_index == soc_dnxf_fabric_link_fifo_type_index_0) {
            rc = soc_reg_field_validate(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Sr, LNK_LVL_FC_TH_0_Sf, value);
        } else {
            rc = soc_reg_field_validate(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Sr, LNK_LVL_FC_TH_1_Sf, value); 
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for RX LLFC threshold"),value)); 
    }
    
      
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_llfc_threshold_set
 * Purpose:
 *      Set RX LLFC threshold
 * Parameters:
 *      unit       - (IN) Unit number.
 *      type_index - (IN) Selected index(0/1)
 *      pipe       - (IN) Selected pipe
 *      fe1        - (IN) Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN) Is FE3 threshold (0-false, 1-true)
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */      
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_llfc_threshold_set(int unit, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = SOC_DNXF_DEFS_GET(unit, nof_instances_dch) - 1;}
    else if(fe1)
        {min_idx = 0; max_idx = 1;}
    else
        {min_idx = 2; max_idx = 3;}

   
    
    /*Configure all selected blocks*/
    for(idx = min_idx ; idx <= max_idx ; idx++) {

        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
        {
            DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_Pr(unit, idx, &reg_val));
            
            if(type_index == soc_dnxf_fabric_link_fifo_type_index_0)
                soc_reg_field_set(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Pr,&reg_val,LNK_LVL_FC_TH_0_Pf,value);
            else
                soc_reg_field_set(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Pr,&reg_val,LNK_LVL_FC_TH_1_Pf,value); 
           
            DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_Pr(unit, idx, reg_val));
        }
        
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
        {
            DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_Sr(unit, idx, &reg_val));
            
            if(type_index == soc_dnxf_fabric_link_fifo_type_index_0)
                soc_reg_field_set(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Sr,&reg_val,LNK_LVL_FC_TH_0_Sf,value);
            else
                soc_reg_field_set(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Sr,&reg_val,LNK_LVL_FC_TH_1_Sf,value); 
            
            DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_Sr(unit, idx, reg_val));
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_llfc_threshold_get
 * Purpose:
 *      Get RX LLFC threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index (0/1)
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */   
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_llfc_threshold_get(int unit, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
    /*Select block for retriving information*/
    if(fe1)
        blk_id = 0;
    else
        blk_id = 2;
    
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_Pr(unit, blk_id, &reg_val));
        
        if(type_index == soc_dnxf_fabric_link_fifo_type_index_0)
            *value = soc_reg_field_get(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Pr,reg_val,LNK_LVL_FC_TH_0_Pf);
        else
            *value = soc_reg_field_get(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Pr,reg_val,LNK_LVL_FC_TH_1_Pf); 
    }
    else 
    {
        DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_Sr(unit, blk_id, &reg_val));
        
        if(type_index == soc_dnxf_fabric_link_fifo_type_index_0)
            *value = soc_reg_field_get(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Sr,reg_val,LNK_LVL_FC_TH_0_Sf);
        else
            *value = soc_reg_field_get(unit,DCH_LINK_LEVEL_FLOW_CONTROL_Sr,reg_val,LNK_LVL_FC_TH_1_Sf); 
    }
  
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_gci_threshold_validate
 * Purpose:
 *      Set RX GCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type       - (IN)  Selected type
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_gci_threshold_validate(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int value, int is_fe1, int is_fe3)
{
    soc_error_t rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, DCH_LOCAL_GCI_0_TYPE_0_TH_Pf, value); break;
                case bcmFabricLinkRxGciLvl2FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, DCH_LOCAL_GCI_1_TYPE_0_TH_Pf, value); break;
                case bcmFabricLinkRxGciLvl3FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr, DCH_LOCAL_GCI_2_TYPE_0_TH_Pf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
        else
        {     
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr, DCH_LOCAL_GCI_0_TYPE_1_TH_Pf, value); break;
                case bcmFabricLinkRxGciLvl2FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr, DCH_LOCAL_GCI_1_TYPE_1_TH_Pf, value); break;
                case bcmFabricLinkRxGciLvl3FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr, DCH_LOCAL_GCI_2_TYPE_1_TH_Pf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for RX GCI threshold"),value)); 
    }

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {    
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, DCH_LOCAL_GCI_0_TYPE_0_TH_Sf, value); break;
                case bcmFabricLinkRxGciLvl2FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, DCH_LOCAL_GCI_1_TYPE_0_TH_Sf, value); break;
                case bcmFabricLinkRxGciLvl3FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, DCH_LOCAL_GCI_2_TYPE_0_TH_Sf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
        else
        {
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, DCH_LOCAL_GCI_0_TYPE_1_TH_Sf, value); break;
                case bcmFabricLinkRxGciLvl2FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, DCH_LOCAL_GCI_1_TYPE_1_TH_Sf, value); break;
                case bcmFabricLinkRxGciLvl3FC: rc = soc_reg_field_validate(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, DCH_LOCAL_GCI_2_TYPE_1_TH_Sf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for RX GCI threshold"),value)); 
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_gci_threshold_set
 * Purpose:
 *      Set RX GCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type       - (IN)  Selected type
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_gci_threshold_set(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = SOC_DNXF_DEFS_GET(unit, nof_instances_dch) - 1;}
    else if(fe1)
        {min_idx = 0; max_idx = 1;}
    else
        {min_idx = 2; max_idx = 3;}
    
    /*Configure all selected blocks*/
    for(idx = min_idx ; idx <= max_idx ; idx++) {
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
        {
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkRxGciLvl1FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr,&reg_val, DCH_LOCAL_GCI_0_TYPE_0_TH_Pf, value); break;
                    case bcmFabricLinkRxGciLvl2FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr,&reg_val, DCH_LOCAL_GCI_1_TYPE_0_TH_Pf, value); break;
                    case bcmFabricLinkRxGciLvl3FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr,&reg_val, DCH_LOCAL_GCI_2_TYPE_0_TH_Pf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
                }
                
                DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr(unit, idx, reg_val));
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkRxGciLvl1FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr,&reg_val, DCH_LOCAL_GCI_0_TYPE_1_TH_Pf, value); break;
                    case bcmFabricLinkRxGciLvl2FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr,&reg_val, DCH_LOCAL_GCI_1_TYPE_1_TH_Pf, value); break;
                    case bcmFabricLinkRxGciLvl3FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr,&reg_val, DCH_LOCAL_GCI_2_TYPE_1_TH_Pf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
                }
                
                DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr(unit, idx, reg_val));
            }
        }
        
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
        {
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkRxGciLvl1FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr,&reg_val, DCH_LOCAL_GCI_0_TYPE_0_TH_Sf, value); break;
                    case bcmFabricLinkRxGciLvl2FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr,&reg_val, DCH_LOCAL_GCI_1_TYPE_0_TH_Sf, value); break;
                    case bcmFabricLinkRxGciLvl3FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr,&reg_val, DCH_LOCAL_GCI_2_TYPE_0_TH_Sf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
                }
                 
                DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr(unit, idx, reg_val));
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkRxGciLvl1FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr,&reg_val, DCH_LOCAL_GCI_0_TYPE_1_TH_Sf, value); break;
                    case bcmFabricLinkRxGciLvl2FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr,&reg_val, DCH_LOCAL_GCI_1_TYPE_1_TH_Sf, value); break;
                    case bcmFabricLinkRxGciLvl3FC: soc_reg_field_set(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr,&reg_val, DCH_LOCAL_GCI_2_TYPE_1_TH_Sf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
                }
                 
                DNXC_IF_ERR_EXIT(WRITE_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr(unit, idx, reg_val));
            }
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_gci_threshold_get
 * Purpose:
 *      Get RX GCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type       - (IN)  Selected type
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_gci_threshold_get(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select block for retriving information*/
    if(fe1)
        blk_id = 0;
    else
        blk_id = 2;
        
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr(unit, blk_id, &reg_val));
            
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr,reg_val, DCH_LOCAL_GCI_0_TYPE_0_TH_Pf); break;
                case bcmFabricLinkRxGciLvl2FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr,reg_val, DCH_LOCAL_GCI_1_TYPE_0_TH_Pf); break;
                case bcmFabricLinkRxGciLvl3FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Pr,reg_val, DCH_LOCAL_GCI_2_TYPE_0_TH_Pf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr(unit, blk_id, &reg_val));
            
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr,reg_val, DCH_LOCAL_GCI_0_TYPE_1_TH_Pf); break;
                case bcmFabricLinkRxGciLvl2FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr,reg_val, DCH_LOCAL_GCI_1_TYPE_1_TH_Pf); break;
                case bcmFabricLinkRxGciLvl3FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Pr,reg_val, DCH_LOCAL_GCI_2_TYPE_1_TH_Pf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
    }
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr(unit, blk_id, &reg_val));
            
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, reg_val, DCH_LOCAL_GCI_0_TYPE_0_TH_Sf); break;
                case bcmFabricLinkRxGciLvl2FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, reg_val, DCH_LOCAL_GCI_1_TYPE_0_TH_Sf); break;
                case bcmFabricLinkRxGciLvl3FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_0_TH_Sr, reg_val, DCH_LOCAL_GCI_2_TYPE_0_TH_Sf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr(unit, blk_id, &reg_val));
            
            switch(type)
            {
                case bcmFabricLinkRxGciLvl1FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, reg_val, DCH_LOCAL_GCI_0_TYPE_1_TH_Sf); break;
                case bcmFabricLinkRxGciLvl2FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, reg_val, DCH_LOCAL_GCI_1_TYPE_1_TH_Sf); break;
                case bcmFabricLinkRxGciLvl3FC: *value = soc_reg_field_get(unit, DCH_DCH_LOCAL_GCI_TYPE_1_TH_Sr, reg_val, DCH_LOCAL_GCI_2_TYPE_1_TH_Sf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_drop_threshold_validate
 * Purpose:
 *      Set RX drop threshold
 * Parameters:
 *      unit       - (IN) Unit number.
 *      type       - (IN) Selected type
 *      pipe       - (IN) Selected pipe
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_drop_threshold_validate(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxc_fabric_pipe_t pipe, int value)
{
    soc_error_t rc1 = SOC_E_NONE, rc2 = SOC_E_NONE;
    uint64      val64;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
	 
    COMPILER_64_SET(val64, 0, value);
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        switch(type)
        {
            case bcmFabricLinkRxPrio0Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, DCMUA_P_0_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, DCMUB_P_0_DROP_THf, val64); 
                break;
            case bcmFabricLinkRxPrio1Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, DCMUA_P_1_DROP_THf, val64);
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, DCMUB_P_1_DROP_THf, val64); 
                break;
            case bcmFabricLinkRxPrio2Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, DCMUA_P_2_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, DCMUB_P_2_DROP_THf, val64); 
                break;
            case bcmFabricLinkRxPrio3Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, DCMUA_P_3_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr, DCMUB_P_3_DROP_THf, val64); 
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
        }
    }

    if(SOC_FAILURE(rc1) || SOC_FAILURE(rc2)) {
        DNXC_EXIT_WITH_ERR(SOC_FAILURE(rc1) ? rc1 : rc2, (_BSL_DNXC_MSG("value %d isn't a valid for RX drop threshold"),value)); 
    }

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {       
        switch(type)
        {
            case bcmFabricLinkRxPrio0Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, DCMMA_P_0_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, DCMMB_P_0_DROP_THf, val64); 
                break;
            case bcmFabricLinkRxPrio1Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, DCMMA_P_1_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, DCMMB_P_1_DROP_THf, val64); 
                break;
            case bcmFabricLinkRxPrio2Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, DCMMA_P_2_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, DCMMB_P_2_DROP_THf, val64); 
                break;
            case bcmFabricLinkRxPrio3Drop: 
                rc1 = soc_reg64_field_validate(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, DCMMA_P_3_DROP_THf, val64); 
                rc2 = soc_reg64_field_validate(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr, DCMMB_P_3_DROP_THf, val64); 
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
        }
    }
   
    if(SOC_FAILURE(rc1) || SOC_FAILURE(rc2)) {
        DNXC_EXIT_WITH_ERR(SOC_FAILURE(rc1) ? rc1 : rc2, (_BSL_DNXC_MSG("value %d isn't a valid for RX drop threshold"),value)); 
    }

    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_drop_threshold_set
 * Purpose:
 *      Set RX drop threshold
 * Parameters:
 *      unit       - (IN) Unit number.
 *      type       - (IN) Selected type
 *      pipe       - (IN) Selected pipe
 *      fe1        - (IN) Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN) Is FE3 threshold (0-false, 1-true)
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_drop_threshold_set(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint64 reg_valA, reg_valB, val64;
    uint32 reg_dcma_registers_en[SOC_RAMON_FE1600_NOF_INSTANCES_DCMA], reg_dcmb_registers_en[SOC_RAMON_FE1600_NOF_INSTANCES_DCMB];
    uint32 enable_a, enable_b, prio;

    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);
	COMPILER_64_SET(val64, 0, value);
    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = 1;}
    else if(fe1)
        {min_idx = 0; max_idx = 0;}
    else
        {min_idx = 1; max_idx = 1;}
         
    /*Configure all selected blocks*/ 
    for(idx = min_idx ; idx <= max_idx ; idx++){
            
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
        {
            DNXC_IF_ERR_EXIT(READ_DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr(unit, idx, &reg_valA));
            DNXC_IF_ERR_EXIT(READ_DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr(unit, idx, &reg_valB));

            DNXC_IF_ERR_EXIT(READ_DCMA_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcma_registers_en+ idx));
            DNXC_IF_ERR_EXIT(READ_DCMB_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcmb_registers_en+ idx));
            enable_a = soc_reg_field_get(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en[idx], LOW_PR_DROP_EN_Pf);
            enable_b = soc_reg_field_get(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en[idx], LOW_PR_DROP_EN_Pf);
      
            switch(type)
            {
                case bcmFabricLinkRxPrio0Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMUA_P_0_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMUB_P_0_DROP_THf, val64);
                    prio = 0;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_b);
                    break;
                case bcmFabricLinkRxPrio1Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMUA_P_1_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMUB_P_1_DROP_THf, val64);
                    prio = 1;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_b);
                    break;
                case bcmFabricLinkRxPrio2Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMUA_P_2_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMUB_P_2_DROP_THf, val64);
                    prio = 2;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_b);
                    break;
                case bcmFabricLinkRxPrio3Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMUA_P_3_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMUB_P_3_DROP_THf, val64);
                    prio = 3;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Pf, enable_b);
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
            
            DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr(unit, idx, reg_valA));
            DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMUB_PRIORITY_DROP_THRESHOLDr(unit, idx, reg_valB));
            DNXC_IF_ERR_EXIT(WRITE_DCMA_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcma_registers_en[idx]));
            DNXC_IF_ERR_EXIT(WRITE_DCMB_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcmb_registers_en[idx]));
        }

        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
        {
            DNXC_IF_ERR_EXIT(READ_DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr(unit, idx, &reg_valA));
            DNXC_IF_ERR_EXIT(READ_DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr(unit, idx, &reg_valB));

            DNXC_IF_ERR_EXIT(READ_DCMA_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcma_registers_en+ idx));
            DNXC_IF_ERR_EXIT(READ_DCMB_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcmb_registers_en+ idx));
            enable_a = soc_reg_field_get(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en[idx], LOW_PR_DROP_EN_Sf);
            enable_b = soc_reg_field_get(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en[idx], LOW_PR_DROP_EN_Sf);
            
            switch(type)
            {
                case bcmFabricLinkRxPrio0Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMMA_P_0_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMMB_P_0_DROP_THf, val64);
                    prio = 0;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_b);
                    break;
                case bcmFabricLinkRxPrio1Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMMA_P_1_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMMB_P_1_DROP_THf, val64);
                    prio = 1;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_b);
                    break;
                case bcmFabricLinkRxPrio2Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMMA_P_2_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMMB_P_2_DROP_THf, val64);
                    prio = 2;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_b);            
                    break;
                case bcmFabricLinkRxPrio3Drop: 
                    soc_reg64_field_set(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr,&reg_valA, DCMMA_P_3_DROP_THf, val64); 
                    soc_reg64_field_set(unit, DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr,&reg_valB, DCMMB_P_3_DROP_THf, val64);
                    prio = 3;
                    enable_a |= 1 << prio;
                    enable_b |= 1 << prio;
                    soc_reg_field_set(unit, DCMA_DCM_ENABLERS_REGISTERr, reg_dcma_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_a);
                    soc_reg_field_set(unit, DCMB_DCM_ENABLERS_REGISTERr, reg_dcmb_registers_en + idx, LOW_PR_DROP_EN_Sf, enable_b);                   
                    break;
                default:
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
            
            DNXC_IF_ERR_EXIT(WRITE_DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr(unit, idx, reg_valA));
            DNXC_IF_ERR_EXIT(WRITE_DCMB_DCMMB_PRIORITY_DROP_THRESHOLDr(unit, idx, reg_valB));
            DNXC_IF_ERR_EXIT(WRITE_DCMA_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcma_registers_en[idx]));
            DNXC_IF_ERR_EXIT(WRITE_DCMB_DCM_ENABLERS_REGISTERr(unit, idx, reg_dcmb_registers_en[idx]));
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_rx_drop_threshold_get
 * Purpose:
 *      Get RX drop threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_rx_drop_threshold_get(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxc_fabric_control_source_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint64 reg_valA;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
    /*Select block for retriving information*/
    if(fe1)
        blk_id = 0;
    else
        blk_id = 1;
      
    if(pipe == soc_dnxc_fabric_control_source_primary || pipe == soc_dnxc_fabric_control_source_both)
    {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr(unit, blk_id, &reg_valA));
        
        switch(type)
        {
            case bcmFabricLinkRxPrio0Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMUA_P_0_DROP_THf); 
                break;
            case bcmFabricLinkRxPrio1Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMUA_P_1_DROP_THf); 
                break;
            case bcmFabricLinkRxPrio2Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMUA_P_2_DROP_THf); 
                break;
            case bcmFabricLinkRxPrio3Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMUA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMUA_P_3_DROP_THf); 
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
        }
    }
    else /*if(pipe == soc_dnxc_fabric_control_source_secondary || pipe == soc_dnxc_fabric_control_source_both)*/
    {
        DNXC_IF_ERR_EXIT(READ_DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr(unit, blk_id, &reg_valA));
        switch(type)
        {
            case bcmFabricLinkRxPrio0Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMMA_P_0_DROP_THf); 
                break;
            case bcmFabricLinkRxPrio1Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMMA_P_1_DROP_THf); 
                break;
            case bcmFabricLinkRxPrio2Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMMA_P_2_DROP_THf);  
                break;
            case bcmFabricLinkRxPrio3Drop: 
                *value = soc_reg64_field32_get(unit, DCMA_DCMMA_PRIORITY_DROP_THRESHOLDr, reg_valA, DCMMA_P_3_DROP_THf); 
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
        }
    }
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_rci_threshold_validate
 * Purpose:
 *      Set TX RCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */    
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_rci_threshold_validate(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int value)
{
    soc_error_t rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);  

    if (type != bcmFabricLinkRciFC)
    {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("invalid type for ramon_fe1600")));
    }

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index) {
            rc = soc_reg_field_validate(unit, DCL_TYPE_01_RCI_TH_Pr, RCI_TH_HIGH_0_Pf, value);
        } else {
            rc = soc_reg_field_validate(unit, DCL_TYPE_01_RCI_TH_Pr, RCI_TH_HIGH_1_Pf, value);  
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for TX RCI threshold"),value)); 
    }

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    { 
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index) {
            rc = soc_reg_field_validate(unit, DCL_TYPE_01_RCI_TH_Sr, RCI_TH_HIGH_0_Sf, value);
        } else {
            rc = soc_reg_field_validate(unit, DCL_TYPE_01_RCI_TH_Sr, RCI_TH_HIGH_1_Sf, value);
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for TX RCI threshold"),value)); 
    }
    
exit:
    DNXC_FUNC_RETURN;
}   

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_rci_threshold_set
 * Purpose:
 *      Set TX RCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */    
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_rci_threshold_set(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl) - 1;}
    else if(fe1)
        {min_idx = 2; max_idx = 3;}
    else
        {min_idx = 0; max_idx = 1;}
    
    /*Configure all selected blocks*/ 
    for(idx = min_idx ; idx <= max_idx ; idx++) {
      
      if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
      {
          DNXC_IF_ERR_EXIT(READ_DCL_TYPE_01_RCI_TH_Pr(unit, idx, &reg_val));
          
          if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
              soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Pr, &reg_val, RCI_TH_HIGH_0_Pf, value);
          else
              soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Pr, &reg_val, RCI_TH_HIGH_1_Pf, value);
          
          DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_01_RCI_TH_Pr(unit, idx, reg_val));
        
      }
      
      if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
      {
          DNXC_IF_ERR_EXIT(READ_DCL_TYPE_01_RCI_TH_Sr(unit, idx, &reg_val));
          
          if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
              soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Sr, &reg_val, RCI_TH_HIGH_0_Sf, value);
          else
              soc_reg_field_set(unit, DCL_TYPE_01_RCI_TH_Sr, &reg_val, RCI_TH_HIGH_1_Sf, value);
          
          DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_01_RCI_TH_Sr(unit, idx, reg_val));
      }
    }
    
exit:
    DNXC_FUNC_RETURN;
}      

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_rci_threshold_get
 * Purpose:
 *      Get TX RCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_rci_threshold_get(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select block for retriving information*/
    if(fe1)
        blk_id = 2;
    else
        blk_id = 0;

    if (type != bcmFabricLinkRciFC)
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid type for ramon_fe1600 %d"),type)); 
    }
    
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_01_RCI_TH_Pr(unit, blk_id, &reg_val));
        
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            *value = soc_reg_field_get(unit, DCL_TYPE_01_RCI_TH_Pr, reg_val, RCI_TH_HIGH_0_Pf);
        else
            *value = soc_reg_field_get(unit, DCL_TYPE_01_RCI_TH_Pr, reg_val, RCI_TH_HIGH_1_Pf);
      
    }
    else /*if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))*/
    {
        DNXC_IF_ERR_EXIT(READ_DCL_TYPE_01_RCI_TH_Pr(unit, blk_id, &reg_val));
        
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            *value = soc_reg_field_get(unit, DCL_TYPE_01_RCI_TH_Sr, reg_val, RCI_TH_HIGH_0_Sf);
        else
            *value = soc_reg_field_get(unit, DCL_TYPE_01_RCI_TH_Sr, reg_val, RCI_TH_HIGH_1_Sf);
    }
    
exit:
    DNXC_FUNC_RETURN;
}    

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_bypass_llfc_threshold_validate
 * Purpose:
 *      Set TX bypass LLFC threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_bypass_llfc_threshold_validate(int unit,soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int value)
{
    soc_error_t rc = SOC_E_NONE;
    uint64  val64;


    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);
	COMPILER_64_SET(val64, 0, value);
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            rc = soc_reg64_field_validate(unit, DCL_DCL_LLFC_THr, DCLU_LLFC_TH_TYPE_0f, val64);
        else
            rc = soc_reg64_field_validate(unit, DCL_DCL_LLFC_THr, DCLU_LLFC_TH_TYPE_1f, val64);
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for TX BYBASS LLFC threshold"),value)); 
    }

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            rc = soc_reg64_field_validate(unit, DCL_DCL_LLFC_THr, DCLM_LLFC_TH_TYPE_0f, val64);
        else
            rc = soc_reg64_field_validate(unit, DCL_DCL_LLFC_THr, DCLM_LLFC_TH_TYPE_1f, val64);
    }
     if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid for TX BYBASS LLFC threshold"),value)); 
    }   

  
exit:
    DNXC_FUNC_RETURN;
}   

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_bypass_llfc_threshold_set
 * Purpose:
 *      Set TX bypass LLFC threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_bypass_llfc_threshold_set(int unit,soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint64 reg_val, val64;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
	 
    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl) - 1;}
    else if(fe1)
        {min_idx = 2; max_idx = 3;}
    else
        {min_idx = 0; max_idx = 1;}
    
    COMPILER_64_SET(val64, 0, value);
    /*Configure all selected blocks*/ 
    for(idx = min_idx ; idx <= max_idx ; idx++)  {
      
        DNXC_IF_ERR_EXIT(READ_DCL_DCL_LLFC_THr(unit, idx, &reg_val));
        
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
        {
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
                soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg_val, DCLU_LLFC_TH_TYPE_0f, val64);
            else
                soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg_val, DCLU_LLFC_TH_TYPE_1f, val64);
        }
        
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
        {
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
                soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg_val, DCLM_LLFC_TH_TYPE_0f, val64);
            else
                soc_reg64_field_set(unit, DCL_DCL_LLFC_THr, &reg_val, DCLM_LLFC_TH_TYPE_1f, val64);
        }
        
        DNXC_IF_ERR_EXIT(WRITE_DCL_DCL_LLFC_THr(unit, idx, reg_val));
    }
    
exit:
    DNXC_FUNC_RETURN;
}   

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_bypass_llfc_threshold_get
 * Purpose:
 *      Get TX bypass LLFC threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_bypass_llfc_threshold_get(int unit,soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint64 reg_val, ret_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select block for retriving information*/
    if(fe1)
        blk_id = 2;
    else
        blk_id = 0;
      
    DNXC_IF_ERR_EXIT(READ_DCL_DCL_LLFC_THr(unit, blk_id, &reg_val));
    
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            ret_val = soc_reg64_field_get(unit, DCL_DCL_LLFC_THr, reg_val, DCLU_LLFC_TH_TYPE_0f);
        else
            ret_val = soc_reg64_field_get(unit, DCL_DCL_LLFC_THr, reg_val, DCLU_LLFC_TH_TYPE_1f); 
    }
    else /*if(pipe == soc_dnxc_fabric_control_source_secondary || pipe == soc_dnxc_fabric_control_source_both)*/
    {
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
            ret_val = soc_reg64_field_get(unit, DCL_DCL_LLFC_THr, reg_val, DCLM_LLFC_TH_TYPE_0f);
        else
            ret_val = soc_reg64_field_get(unit, DCL_DCL_LLFC_THr, reg_val, DCLM_LLFC_TH_TYPE_1f);
    }
    *value = COMPILER_64_LO(ret_val);
exit:
    DNXC_FUNC_RETURN;
}   

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_gci_threshold_validate
 * Purpose:
 *      Set TX GCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_gci_threshold_validate(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int value)
{
    soc_error_t rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: rc = soc_reg_field_validate(unit, DCL_TYPE_0_GCI_TH_Pr, GCI_TH_LOW_0_Pf, value); break;
                case bcmFabricLinkTxGciLvl2FC: rc = soc_reg_field_validate(unit, DCL_TYPE_0_GCI_TH_Pr, GCI_TH_MED_0_Pf, value); break;
                case bcmFabricLinkTxGciLvl3FC: rc = soc_reg_field_validate(unit, DCL_TYPE_0_GCI_TH_Pr, GCI_TH_HIGH_0_Pf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
        else
        {
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: rc = soc_reg_field_validate(unit, DCL_TYPE_1_GCI_TH_Pr, GCI_TH_LOW_1_Pf, value); break;
                case bcmFabricLinkTxGciLvl2FC: rc = soc_reg_field_validate(unit, DCL_TYPE_1_GCI_TH_Pr, GCI_TH_MED_1_Pf, value); break;
                case bcmFabricLinkTxGciLvl3FC: rc = soc_reg_field_validate(unit, DCL_TYPE_1_GCI_TH_Pr, GCI_TH_HIGH_1_Pf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid threshold for type %d"),value,type)); 
    } 

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: rc = soc_reg_field_validate(unit, DCL_TYPE_0_GCI_TH_Sr, GCI_TH_LOW_0_Sf, value); break;
                case bcmFabricLinkTxGciLvl2FC: rc = soc_reg_field_validate(unit, DCL_TYPE_0_GCI_TH_Sr, GCI_TH_MED_0_Sf, value); break;
                case bcmFabricLinkTxGciLvl3FC: rc = soc_reg_field_validate(unit, DCL_TYPE_0_GCI_TH_Sr, GCI_TH_HIGH_0_Sf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
        else
        {
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: rc = soc_reg_field_validate(unit, DCL_TYPE_1_GCI_TH_Sr, GCI_TH_LOW_1_Sf, value); break;
                case bcmFabricLinkTxGciLvl2FC: rc = soc_reg_field_validate(unit, DCL_TYPE_1_GCI_TH_Sr, GCI_TH_MED_1_Sf, value); break;
                case bcmFabricLinkTxGciLvl3FC: rc = soc_reg_field_validate(unit, DCL_TYPE_1_GCI_TH_Sr, GCI_TH_HIGH_1_Sf, value); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid threshold for type %d"),value,type)); 
    }
      
exit:
    DNXC_FUNC_RETURN;
} 

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_gci_threshold_set
 * Purpose:
 *      Set TX GCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (IN) Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_gci_threshold_set(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl) - 1;}
    else if(fe1)
        {min_idx = 2; max_idx = 3;}
    else
        {min_idx = 0; max_idx = 1;}
    
    /*Configure all selected blocks*/  
    for(idx = min_idx ; idx <= max_idx ; idx++) { 
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
        {
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_GCI_TH_Pr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxGciLvl1FC: soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Pr, &reg_val, GCI_TH_LOW_0_Pf, value); break;
                    case bcmFabricLinkTxGciLvl2FC: soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Pr, &reg_val, GCI_TH_MED_0_Pf, value); break;
                    case bcmFabricLinkTxGciLvl3FC: soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Pr, &reg_val, GCI_TH_HIGH_0_Pf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
                }
                
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_GCI_TH_Pr(unit, idx, reg_val));
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_GCI_TH_Pr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxGciLvl1FC: soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Pr, &reg_val, GCI_TH_LOW_1_Pf, value); break;
                    case bcmFabricLinkTxGciLvl2FC: soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Pr, &reg_val, GCI_TH_MED_1_Pf, value); break;
                    case bcmFabricLinkTxGciLvl3FC: soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Pr, &reg_val, GCI_TH_HIGH_1_Pf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
                }
             
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_GCI_TH_Pr(unit, idx, reg_val));
            }
        }
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
        {
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_GCI_TH_Sr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxGciLvl1FC: soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Sr, &reg_val, GCI_TH_LOW_0_Sf, value); break;
                    case bcmFabricLinkTxGciLvl2FC: soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Sr, &reg_val, GCI_TH_MED_0_Sf, value); break;
                    case bcmFabricLinkTxGciLvl3FC: soc_reg_field_set(unit, DCL_TYPE_0_GCI_TH_Sr, &reg_val, GCI_TH_HIGH_0_Sf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
                }
               
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_GCI_TH_Sr(unit, idx, reg_val));
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_GCI_TH_Sr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxGciLvl1FC: soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Sr, &reg_val, GCI_TH_LOW_1_Sf, value); break;
                    case bcmFabricLinkTxGciLvl2FC: soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Sr, &reg_val, GCI_TH_MED_1_Sf, value); break;
                    case bcmFabricLinkTxGciLvl3FC: soc_reg_field_set(unit, DCL_TYPE_1_GCI_TH_Sr, &reg_val, GCI_TH_HIGH_1_Sf, value); break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
                }
                
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_GCI_TH_Sr(unit, idx, reg_val));
            }
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}   

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_gci_threshold_get
 * Purpose:
 *      Get TX GCI threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_gci_threshold_get(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select block for retriving information*/
    if(fe3)
        blk_id = 0;
    else if(fe1)
        blk_id = 2;
    else
    {
        LOG_WARN(BSL_LS_SOC_FABRIC,
                 (BSL_META_U(unit,
                             "No blocks to configure\n")));
        SOC_EXIT;
    }
      
      
    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_GCI_TH_Pr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: *value = soc_reg_field_get(unit, DCL_TYPE_0_GCI_TH_Pr, reg_val, GCI_TH_LOW_0_Pf); break;
                case bcmFabricLinkTxGciLvl2FC: *value = soc_reg_field_get(unit, DCL_TYPE_0_GCI_TH_Pr, reg_val, GCI_TH_MED_0_Pf); break;
                case bcmFabricLinkTxGciLvl3FC: *value = soc_reg_field_get(unit, DCL_TYPE_0_GCI_TH_Pr, reg_val, GCI_TH_HIGH_0_Pf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_GCI_TH_Pr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: *value = soc_reg_field_get(unit, DCL_TYPE_1_GCI_TH_Pr, reg_val, GCI_TH_LOW_1_Pf); break;
                case bcmFabricLinkTxGciLvl2FC: *value = soc_reg_field_get(unit, DCL_TYPE_1_GCI_TH_Pr, reg_val, GCI_TH_MED_1_Pf); break;
                case bcmFabricLinkTxGciLvl3FC: *value = soc_reg_field_get(unit, DCL_TYPE_1_GCI_TH_Pr, reg_val, GCI_TH_HIGH_1_Pf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
    }
    else /*if(pipe == soc_dnxc_fabric_control_source_secondary || pipe == soc_dnxc_fabric_control_source_both)*/
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_GCI_TH_Sr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: *value = soc_reg_field_get(unit, DCL_TYPE_0_GCI_TH_Sr, reg_val, GCI_TH_LOW_0_Sf); break;
                case bcmFabricLinkTxGciLvl2FC: *value = soc_reg_field_get(unit, DCL_TYPE_0_GCI_TH_Sr, reg_val, GCI_TH_MED_0_Sf); break;
                case bcmFabricLinkTxGciLvl3FC: *value = soc_reg_field_get(unit, DCL_TYPE_0_GCI_TH_Sr, reg_val, GCI_TH_HIGH_0_Sf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_GCI_TH_Sr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxGciLvl1FC: *value = soc_reg_field_get(unit, DCL_TYPE_1_GCI_TH_Sr, reg_val, GCI_TH_LOW_1_Sf); break;
                case bcmFabricLinkTxGciLvl2FC: *value = soc_reg_field_get(unit, DCL_TYPE_1_GCI_TH_Sr, reg_val, GCI_TH_MED_1_Sf); break;
                case bcmFabricLinkTxGciLvl3FC: *value = soc_reg_field_get(unit, DCL_TYPE_1_GCI_TH_Sr, reg_val, GCI_TH_HIGH_1_Sf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type)); 
            }
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_drop_threshold_validate
 * Purpose:
 *      Get TX drop threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_drop_threshold_validate(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int value)
{
    
    uint64          val64;
    soc_error_t rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;  
	SOC_RAMON_FE1600_ONLY(unit);
	COMPILER_64_SET(val64, 0, value);

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PPr, TYPE_0_DRP_P_0_Pf, val64); break;
                case bcmFabricLinkTxPrio1Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PPr, TYPE_0_DRP_P_1_Pf, val64); break;
                case bcmFabricLinkTxPrio2Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PPr, TYPE_0_DRP_P_2_Pf, val64); break;
                case bcmFabricLinkTxPrio3Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PPr, TYPE_0_DRP_P_3_Pf, val64); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
        else
        {     
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PPr, TYPE_1_DRP_P_0_Pf, val64); break;
                case bcmFabricLinkTxPrio1Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PPr, TYPE_1_DRP_P_1_Pf, val64); break;
                case bcmFabricLinkTxPrio2Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PPr, TYPE_1_DRP_P_2_Pf, val64); break;
                case bcmFabricLinkTxPrio3Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PPr, TYPE_1_DRP_P_3_Pf, val64); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid threshold for type %d"),value, type));
    }

    if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {       
          switch(type)
          {
              case bcmFabricLinkTxPrio0Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PSr, TYPE_0_DRP_P_0_Sf, val64); break;
              case bcmFabricLinkTxPrio1Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PSr, TYPE_0_DRP_P_1_Sf, val64); break;
              case bcmFabricLinkTxPrio2Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PSr, TYPE_0_DRP_P_2_Sf, val64); break;
              case bcmFabricLinkTxPrio3Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_0_DRP_PSr, TYPE_0_DRP_P_3_Sf, val64); break;
              default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
          }
        }
        else
        { 
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PSr, TYPE_1_DRP_P_0_Sf, val64); break;
                case bcmFabricLinkTxPrio1Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PSr, TYPE_1_DRP_P_1_Sf, val64); break;
                case bcmFabricLinkTxPrio2Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PSr, TYPE_1_DRP_P_2_Sf, val64); break;
                case bcmFabricLinkTxPrio3Drop: rc = soc_reg64_field_validate(unit, DCL_TYPE_1_DRP_PSr, TYPE_1_DRP_P_3_Sf, val64); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
        }
    }
    if(SOC_FAILURE(rc)) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("value %d isn't a valid threshold for type %d"),value, type));
    }
    
      
exit:
    DNXC_FUNC_RETURN;
}  

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_drop_threshold_set
 * Purpose:
 *      Get TX drop threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (IN)  Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_drop_threshold_set(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int value)
{
    int min_idx, max_idx,idx;
    uint64 reg_val, val64;
    uint32 reg_dcl_registers_en[SOC_RAMON_FE1600_NOF_INSTANCES_DCL];
    uint32 enable, prio;
    DNXC_INIT_FUNC_DEFS;  
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select blocks*/
    if(fe1 && fe3)
        {min_idx = 0; max_idx = SOC_DNXF_DEFS_GET(unit, nof_instances_dcl) - 1;}
    else if(fe1)
        {min_idx = 2; max_idx = 3;}
    else
        {min_idx = 0; max_idx = 1;}
    
    COMPILER_64_SET(val64, 0, value);
    /*Configure all selected blocks*/ 
    for(idx = min_idx ; idx <= max_idx ; idx++) {
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0))
        {
            DNXC_IF_ERR_EXIT(READ_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, idx, reg_dcl_registers_en+ idx));
            enable = soc_reg_field_get(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en[idx], LOW_PR_DROP_EN_Pf);

            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_DRP_PPr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxPrio0Drop:
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg_val, TYPE_0_DRP_P_0_Pf, val64);
                        prio = 0;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);
                        break;
                    case bcmFabricLinkTxPrio1Drop:
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg_val, TYPE_0_DRP_P_1_Pf, val64);
                        prio = 1;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);
                        break;
                    case bcmFabricLinkTxPrio2Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg_val, TYPE_0_DRP_P_2_Pf, val64);
                        prio = 2;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);
                        break;
                    case bcmFabricLinkTxPrio3Drop:
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PPr, &reg_val, TYPE_0_DRP_P_3_Pf, val64);
                        prio = 3;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);
                        break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
                }
        
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_DRP_PPr(unit, idx, reg_val));
               
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_DRP_PPr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxPrio0Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg_val, TYPE_1_DRP_P_0_Pf, val64);
                        prio = 0;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);
                        break;
                    case bcmFabricLinkTxPrio1Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg_val, TYPE_1_DRP_P_1_Pf, val64);
                        prio = 1;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable); 
                        break;
                    case bcmFabricLinkTxPrio2Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg_val, TYPE_1_DRP_P_2_Pf, val64); 
                        prio = 2;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);     
                        break;
                    case bcmFabricLinkTxPrio3Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PPr, &reg_val, TYPE_1_DRP_P_3_Pf, val64);
                        prio = 3;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Pf, enable);   
                        break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
                }
        
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_DRP_PPr(unit, idx, reg_val));
            }
             DNXC_IF_ERR_EXIT(WRITE_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, idx, reg_dcl_registers_en[idx]));
        }
        if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1))
        {
            DNXC_IF_ERR_EXIT(READ_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, idx, reg_dcl_registers_en+ idx));
            enable = soc_reg_field_get(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en[idx], LOW_PR_DROP_EN_Pf);

            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_DRP_PSr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxPrio0Drop:
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg_val, TYPE_0_DRP_P_0_Sf, val64);
                        prio = 0;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable); 
                        break;
                    case bcmFabricLinkTxPrio1Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg_val, TYPE_0_DRP_P_1_Sf, val64);
                        prio = 1;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);  
                        break;
                    case bcmFabricLinkTxPrio2Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg_val, TYPE_0_DRP_P_2_Sf, val64);
                        prio = 2;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);      
                        break;
                    case bcmFabricLinkTxPrio3Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_0_DRP_PSr, &reg_val, TYPE_0_DRP_P_3_Sf, val64);
                        prio = 3;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);  
                        break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
                }
        
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_0_DRP_PSr(unit, idx, reg_val));
            }
            else
            {
                DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_DRP_PSr(unit, idx, &reg_val));
                
                switch(type)
                {
                    case bcmFabricLinkTxPrio0Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg_val, TYPE_1_DRP_P_0_Sf, val64); 
                        prio = 0;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);
                        break;
                    case bcmFabricLinkTxPrio1Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg_val, TYPE_1_DRP_P_1_Sf, val64);
                        prio = 1;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);  
                        break;
                    case bcmFabricLinkTxPrio2Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg_val, TYPE_1_DRP_P_2_Sf, val64);
                        prio = 2;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);      
                        break;
                    case bcmFabricLinkTxPrio3Drop: 
                        soc_reg64_field_set(unit, DCL_TYPE_1_DRP_PSr, &reg_val, TYPE_1_DRP_P_3_Sf, val64);
                        prio = 3;
                        enable |= 1 << prio;
                        soc_reg_field_set(unit, DCL_DCL_ENABLERS_REGISTERr, reg_dcl_registers_en + idx, LOW_PR_DROP_EN_Sf, enable);  
                        break;
                    default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
                }
        
                DNXC_IF_ERR_EXIT(WRITE_DCL_TYPE_1_DRP_PSr(unit, idx, reg_val));
            }
             DNXC_IF_ERR_EXIT(WRITE_DCL_DCL_ENABLERS_REGISTERr_REG32(unit, idx, reg_dcl_registers_en[idx]));
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}  

/*
 * Function:
 *      soc_ramon_fe1600_fabric_flow_control_tx_drop_threshold_get
 * Purpose:
 *      Get TX drop threshold
 * Parameters:
 *      unit       - (IN)  Unit number.
 *      type_index - (IN)  Selected index
 *      type       - (IN)  Selected type
 *      pipe       - (IN)  Selected pipe
 *      fe1        - (IN)  Is FE1 threshold (0-false, 1-true)
 *      fe3        - (IN)  Is FE3 threshold (0-false, 1-true)
 *      value      - (OUT) Threshold
 * Returns:
 *      SOC_E_xxx
 */ 
soc_error_t 
soc_ramon_fe1600_fabric_flow_control_tx_drop_threshold_get(int unit, bcm_fabric_link_threshold_type_t type, soc_dnxf_fabric_link_fifo_type_index_t type_index, soc_dnxc_fabric_pipe_t pipe, int fe1, int fe3, int* value)
{
    int blk_id;
    uint64 reg_val, ret_val;
    DNXC_INIT_FUNC_DEFS;  
    SOC_RAMON_FE1600_ONLY(unit);

    /*Select block for retriving information*/
    if(fe3)
        blk_id = 0;
    else if(fe1)
        blk_id = 2;
    else
    {
        LOG_WARN(BSL_LS_SOC_FABRIC,
                 (BSL_META_U(unit,
                             "No blocks to configure\n")));
        SOC_EXIT;
    }
      
    if(pipe == soc_dnxc_fabric_control_source_primary || pipe == soc_dnxc_fabric_control_source_both)
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_DRP_PPr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PPr, reg_val, TYPE_0_DRP_P_0_Pf); break;
                case bcmFabricLinkTxPrio1Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PPr, reg_val, TYPE_0_DRP_P_1_Pf); break;
                case bcmFabricLinkTxPrio2Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PPr, reg_val, TYPE_0_DRP_P_2_Pf); break;
                case bcmFabricLinkTxPrio3Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PPr, reg_val, TYPE_0_DRP_P_3_Pf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
            *value = COMPILER_64_LO(ret_val);
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_DRP_PPr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PPr, reg_val, TYPE_1_DRP_P_0_Pf); break;
                case bcmFabricLinkTxPrio1Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PPr, reg_val, TYPE_1_DRP_P_1_Pf); break;
                case bcmFabricLinkTxPrio2Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PPr, reg_val, TYPE_1_DRP_P_2_Pf); break;
                case bcmFabricLinkTxPrio3Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PPr, reg_val, TYPE_1_DRP_P_3_Pf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
            *value = COMPILER_64_LO(ret_val);
        }
    }
    else /*if(pipe == soc_dnxc_fabric_control_source_secondary || pipe == soc_dnxc_fabric_control_source_both)*/
    {
        if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_0_DRP_PSr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PSr, reg_val, TYPE_0_DRP_P_0_Sf); break;
                case bcmFabricLinkTxPrio1Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PSr, reg_val, TYPE_0_DRP_P_1_Sf); break;
                case bcmFabricLinkTxPrio2Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PSr, reg_val, TYPE_0_DRP_P_2_Sf); break;
                case bcmFabricLinkTxPrio3Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_0_DRP_PSr, reg_val, TYPE_0_DRP_P_3_Sf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
            *value = COMPILER_64_LO(ret_val);
        }
        else
        {
            DNXC_IF_ERR_EXIT(READ_DCL_TYPE_1_DRP_PSr(unit, blk_id, &reg_val));
            switch(type)
            {
                case bcmFabricLinkTxPrio0Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PSr, reg_val, TYPE_1_DRP_P_0_Sf); break;
                case bcmFabricLinkTxPrio1Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PSr, reg_val, TYPE_1_DRP_P_1_Sf); break;
                case bcmFabricLinkTxPrio2Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PSr, reg_val, TYPE_1_DRP_P_2_Sf); break;
                case bcmFabricLinkTxPrio3Drop: ret_val = soc_reg64_field_get(unit, DCL_TYPE_1_DRP_PSr, reg_val, TYPE_1_DRP_P_3_Sf); break;
                default: DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong threshold type %d"),type));
            }
            *value = COMPILER_64_LO(ret_val);
        }
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_links_link_type_set
 * Purpose:
 *      Set links thresholds type
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      pipe                - (IN)  primary / Secondary
 *      type_index          - (IN)  Threshold type index
 *      is_rx               - (IN)  Whether we configure rx links
 *      is_tx               - (IN)  Whether we configure tx links
 *      is_fe1              - (IN)  Whether we configure fe1 links
 *      is_fe3              - (IN)  Whether we configure fe3 links
 *      links_count         - (IN)  Size of links
 *      links               - (IN)  Links array
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_links_link_type_set(int unit, soc_dnxc_fabric_pipe_t pipe, soc_dnxf_fabric_link_fifo_type_index_t type_index, 
                                        int is_rx, int is_tx, int is_fe1, int is_fe3, uint32 links_count, soc_port_t* links)
{
    uint32 reg_val_prim = 0, reg_val_scnd = 0;
    int blk_id, i;
    soc_port_t link, inner_lnk;
    int half_of_links = (SOC_DNXF_DEFS_GET(unit, nof_links)/2);
    DNXC_INIT_FUNC_DEFS;  
    SOC_RAMON_FE1600_ONLY(unit);

    for(i=0 ; i<links_count ; i++)
    {
        link = links[i];
        blk_id = INT_DEVIDE(link,32);
        inner_lnk = link % 32; 
        
        if(is_rx && ((is_fe1 && link<half_of_links) || (is_fe3 &&link>=half_of_links)))
        {
            /*read corrent state*/
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0)) {
                DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_BMP_Pr(unit, blk_id, &reg_val_prim));
            }
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1)) {
                DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_BMP_Sr(unit, blk_id, &reg_val_scnd));

            }
            
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                /*turn off bit*/
                SHR_BITCLR(&reg_val_prim, inner_lnk);
                SHR_BITCLR(&reg_val_scnd, inner_lnk);
            }
            else
            {
                /*turn on bit*/
                SHR_BITSET(&reg_val_prim, inner_lnk);
                SHR_BITSET(&reg_val_scnd, inner_lnk);
            }
            
            /* write new state*/
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0)) {
                DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_BMP_Pr(unit, blk_id, reg_val_prim));
            }
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1)) {
                DNXC_IF_ERR_EXIT(WRITE_DCH_LINK_LEVEL_FLOW_CONTROL_BMP_Sr(unit, blk_id, reg_val_scnd));

            }
        }
        
        if(is_tx && ((is_fe1 && link>=half_of_links) || (is_fe3 && link<half_of_links)))
        {
            /*read corrent state*/
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0)) {
                DNXC_IF_ERR_EXIT(READ_DCL_LINK_TYPE_BMP_Pr(unit, blk_id, &reg_val_prim));
            }
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1)) {
                DNXC_IF_ERR_EXIT(READ_DCL_LINK_TYPE_BMP_Sr(unit, blk_id, &reg_val_scnd));
            }
            
            if(soc_dnxf_fabric_link_fifo_type_index_0 == type_index)
            {
                /*turn off bit*/
                SHR_BITCLR(&reg_val_prim, inner_lnk);
                SHR_BITCLR(&reg_val_scnd, inner_lnk);
            }
            else
            {
                /*turn on bit*/
                SHR_BITSET(&reg_val_prim, inner_lnk);
                SHR_BITSET(&reg_val_scnd, inner_lnk);
            }
            
            /*write new state, both primary and secondary*/
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0)) {
                DNXC_IF_ERR_EXIT(WRITE_DCL_LINK_TYPE_BMP_Pr(unit, blk_id, reg_val_prim));
            }
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,1)) {
                DNXC_IF_ERR_EXIT(WRITE_DCL_LINK_TYPE_BMP_Sr(unit, blk_id, reg_val_scnd));

            }
        }
    }
    
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_links_link_type_get
 * Purpose:
 *      Get links thresholds type
 * Parameters:
 *      unit             - (IN)   Unit number.
 *      pipe             - (IN)   primary / Secondary
 *      type_index       - (IN)   Threshold type index
 *      is_rx            - (IN)   Whether we retrieve rx links
 *      is_tx            - (IN)   Whether we retrieve tx links
 *      is_fe1           - (IN)   Whether we retrieve fe1 links
 *      is_fe3           - (IN)   Whether we retrieve fe3 links
 *      links_count_max  - (IN)   Max size of links
 *      links            - (OUT)  Links array
 *      links_count      - (OUT)  Actual size of links
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_links_link_type_get(int unit, soc_dnxc_fabric_pipe_t pipe, soc_dnxf_fabric_link_fifo_type_index_t type_index, 
                                          int is_rx, int is_tx, int is_fe1, int is_fe3, uint32 links_count_max, soc_port_t* links, uint32* links_count)
{
    uint32 reg_val[1];
    int blk_id;
    soc_port_t link, inner_lnk;
    int actual_type_index;
    int half_of_links = (SOC_DNXF_DEFS_GET(unit, nof_links)/2);
    pbmp_t valid_link_bitmap;
    DNXC_INIT_FUNC_DEFS;  
    SOC_RAMON_FE1600_ONLY(unit);

    *links_count = 0;
    SOC_PBMP_ASSIGN(valid_link_bitmap, SOC_INFO(unit).sfi.bitmap);
    SOC_PBMP_ITER(valid_link_bitmap, link)
    {
        actual_type_index = -1;
        blk_id = INT_DEVIDE(link,32);
        inner_lnk = link % 32; 
        
        if(is_rx && ((is_fe1 && link<half_of_links) || (is_fe3 &&link>=half_of_links)))
        {
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0)) {
                DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_BMP_Pr(unit, blk_id, reg_val));

            } else /*if(soc_dnxc_fabric_control_source_secondary == pipe || soc_dnxc_fabric_control_source_both == pipe)*/ {
                DNXC_IF_ERR_EXIT(READ_DCH_LINK_LEVEL_FLOW_CONTROL_BMP_Sr(unit, blk_id, reg_val));
            }
                
            if(SHR_BITGET(reg_val, inner_lnk))
                actual_type_index = soc_dnxf_fabric_link_fifo_type_index_1;
            else
                actual_type_index = soc_dnxf_fabric_link_fifo_type_index_0;
        }
        else if(is_tx && ((is_fe1 && link>=half_of_links) || (is_fe3 && link<half_of_links)))
        {
            if(SOC_DNXC_FABRIC_PIPE_IS_SET(&pipe,0)) {
                DNXC_IF_ERR_EXIT(READ_DCL_LINK_TYPE_BMP_Pr(unit, blk_id, reg_val));
            } else /*if(soc_dnxc_fabric_control_source_secondary == pipe || soc_dnxc_fabric_control_source_both == pipe)*/ {
                DNXC_IF_ERR_EXIT(READ_DCL_LINK_TYPE_BMP_Sr(unit, blk_id, reg_val));
            }
            
            if(SHR_BITGET(reg_val, inner_lnk))
                actual_type_index = soc_dnxf_fabric_link_fifo_type_index_1;
            else
                actual_type_index = soc_dnxf_fabric_link_fifo_type_index_0;
        }
        
        if(actual_type_index == type_index)
        {
            if(*links_count >= links_count_max)
            {
                *links_count = 0;
                DNXC_EXIT_WITH_ERR(SOC_E_FULL, (_BSL_DNXC_MSG("links_count_max %d is too small"),links_count_max));
            }
            links[*links_count] = link;
            (*links_count)++;
        }
    }
      
exit:
    DNXC_FUNC_RETURN;  
}

#endif /*defined(BCM_88790_A0)*/

#undef _ERR_MSG_MODULE_NAME

