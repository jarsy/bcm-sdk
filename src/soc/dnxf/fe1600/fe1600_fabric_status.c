/*
 * $Id: ramon_fe1600_fabric_status.c,v 1.13 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 FABRIC STATUS
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/mem.h>

#include <soc/defs.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <shared/bitop.h>
#include <soc/dnxc/legacy/dnxc_defs.h>

#if defined(BCM_88790_A0)

#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_fabric_status.h>

/*
 * Function:
 *      soc_ramon_fe1600_fabric_link_status_all_get
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
soc_ramon_fe1600_fabric_link_status_all_get(int unit, int links_array_max_size, uint32* link_status, uint32* errored_token_count, int* links_array_count)
{
    int i, rc;
    DNXC_INIT_FUNC_DEFS;

    (*links_array_count) = 0;

    for(i=0 ; i<SOC_DNXF_DEFS_GET(unit, nof_links) && i<links_array_max_size; i++ ) {
        if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), i)) {
            link_status[i] = DNXC_FABRIC_LINK_NO_CONNECTIVITY;
            errored_token_count[i] = 0;
        } else {
            rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_fabric_link_status_get, (unit, i, &(link_status[i]), &(errored_token_count[i])));
            DNXC_IF_ERR_EXIT(rc);
        }
        (*links_array_count)++;
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_fe1600_fabric_link_status_get
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
soc_ramon_fe1600_fabric_link_status_get(int unit, soc_port_t link_id, uint32 *link_status, uint32 *errored_token_count)
{
    uint32 reg_val, field_val[1], sig_acc = 0;
    uint32 sig_lock[1] = {0};
    int blk_id, reg_select;
    soc_port_t inner_lnk;
    int srd_id, srd_arr_id;
    int rv;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    *link_status = 0;

    blk_id = INT_DEVIDE(link_id, 4);
    reg_select = link_id % 4;
   
    /*leaky bucket*/
    if (reg_select >= 0 && reg_select < 4)
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_LEAKY_BUCKETr(unit, blk_id, reg_select, &reg_val));
        *errored_token_count = soc_reg_field_get(unit, FMAC_LEAKY_BUCKETr, reg_val, MACR_N_LKY_BKT_VALUEf);
    } else {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Can't find register for link %d"),link_id));
    }
    
  
  /*link status
    DNXC_FABRIC_LINK_STATUS_CRC_ERROR Non-zero CRC rate  
    DNXC_FABRIC_LINK_STATUS_SIZE_ERROR Non-zero size error-count  
    DNXC_FABRIC_LINK_STATUS_CODE_GROUP_ERROR Non-zero code group error-count  
    DNXC_FABRIC_LINK_STATUS_MISALIGN Link down, misalignment error  
    DNXC_FABRIC_LINK_STATUS_NO_SIG_LOCK Link down, SerDes signal lock error  
    DNXC_FABRIC_LINK_STATUS_NO_SIG_ACCEP Link up, but not accepting reachability cells  
    DNXC_FABRIC_LINK_STATUS_ERRORED_TOKENS Low value, indicates bad link connectivity or link down, based on reachability cells */
  
    inner_lnk = link_id % 4;
    DNXC_IF_ERR_EXIT(READ_FMAC_INTERRUPT_REGISTER_1r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_INTERRUPT_REGISTER_1r, reg_val, RX_CRC_ERR_N_INTf);
    if(SHR_BITGET(field_val, inner_lnk)) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_CRC_ERROR;
    }
    
    *field_val = soc_reg_field_get(unit, FMAC_INTERRUPT_REGISTER_1r, reg_val, WRONG_SIZE_INTf);
    if(SHR_BITGET(field_val, inner_lnk)) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_SIZE_ERROR;  
    }
       
    DNXC_IF_ERR_EXIT(READ_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_INTERRUPT_REGISTER_2r, reg_val, RX_LOST_OF_SYNCf);
    if(SHR_BITGET(field_val, inner_lnk)) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_MISALIGN;  
    }
      
    DNXC_IF_ERR_EXIT(READ_FMAC_INTERRUPT_REGISTER_4r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_INTERRUPT_REGISTER_4r, reg_val, DEC_ERR_INTf);
    if(SHR_BITGET(field_val, inner_lnk)) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_CODE_GROUP_ERROR; 
    } 
      
#ifdef BCM_88754_A0
    if (!SOC_IS_BCM88754_A0(unit))
    {
#endif
        srd_id = (link_id*SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd))/SOC_DNXF_DEFS_GET(unit, nof_links);
        srd_arr_id = (link_id%(SOC_DNXF_DEFS_GET(unit, nof_links)/SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd)))/SOC_RAMON_FE1600_NOF_QUADS_IN_FSRD;
        DNXC_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_STATUSr(unit, srd_id, srd_arr_id, &reg_val));
        *sig_lock = soc_reg_field_get(unit, FSRD_SRD_QUAD_STATUSr, reg_val, SRD_QUAD_N_SYNC_STATUSf);
        if(!(SHR_BITGET(sig_lock,inner_lnk))) {
            *link_status |= DNXC_FABRIC_LINK_STATUS_NO_SIG_LOCK;
        }
#ifdef BCM_88754_A0
    }
#endif
    
    DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, link_id, SOC_PHY_CONTROL_RX_SIGNAL_DETECT, &sig_acc)); 
    if(!sig_acc) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_NO_SIG_ACCEP;
    }
    
    if(*errored_token_count < 63) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_ERRORED_TOKENS;
    }
    
    /*Clear sticky indications*/
    rv = soc_ramon_fe1600_fabric_link_status_clear(unit, link_id);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
  
}

/*
 * Function:
 *      soc_ramon_fe1600_fabric_link_status_clear
 * Purpose:
 *      Clear link status - interrupts
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      link                - (IN) Link #
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fe1600_fabric_link_status_clear(int unit, soc_port_t link)
{
    int blk_id, inner_link;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;
	SOC_RAMON_FE1600_ONLY(unit);

    blk_id = link / 4;
    inner_link = link % 4;

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_INTERRUPT_REGISTER_1r, &reg_val, RX_CRC_ERR_N_INTf, 1 << inner_link);
    soc_reg_field_set(unit, FMAC_INTERRUPT_REGISTER_1r, &reg_val, WRONG_SIZE_INTf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_INTERRUPT_REGISTER_1r(unit,blk_id,reg_val));

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_INTERRUPT_REGISTER_2r, &reg_val, RX_LOST_OF_SYNCf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,reg_val));

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_INTERRUPT_REGISTER_4r, &reg_val, DEC_ERR_INTf, 1 << inner_link);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_INTERRUPT_REGISTER_4r(unit,blk_id,reg_val));
    
exit:
    DNXC_FUNC_RETURN;
  
}
/*
 * Function:
 *      soc_ramon_fe1600_fabric_reachability_status_get
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
soc_ramon_fe1600_fabric_reachability_status_get(int unit, int moduleid, int links_max, uint32 *links_array, int *links_count)
{
    int i;
    uint32 rtp_reg_val[5];
    soc_reg_above_64_val_t intergrity_vec;
    int nof_links;
    DNXC_INIT_FUNC_DEFS;
    
    DNXC_IF_ERR_EXIT(READ_RTP_RMHMTm(unit, MEM_BLOCK_ANY, moduleid, rtp_reg_val));
    DNXC_IF_ERR_EXIT(READ_RTP_LINK_INTEGRITY_VECTORr(unit, intergrity_vec));
    
    *links_count = 0;

    nof_links = SOC_DNXF_DEFS_GET(unit, nof_links);
    
    for(i=0 ; i<nof_links ; i++)
    {
        /*Check if link was defined*/
        if (!IS_SFI_PORT(unit, i)) {
            continue;
        }

        if(SHR_BITGET(intergrity_vec,i) && SHR_BITGET(rtp_reg_val,i)) 
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
 *      soc_ramon_fe1600_fabric_link_connectivity_status_get
 * Purpose:
 *      Retrieves the current link-partner information of a link, for
 *      all existing links up to link_partner_max
 * Parameters:
 *      unit         - (IN)  Unit number.
 *      link_id      - (IN)  Link id
 *      link_partner - (OUT) link partner information
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_fabric_link_connectivity_status_get(int unit, soc_port_t link_id, bcm_fabric_link_connectivity_t *link_partner)
{
    int blk_id, source_lvl;
    soc_port_t internal_link_id;
    uint32 reg_val;
    soc_reg_above_64_val_t intergrity_vec;
    int nof_links_in_dcl;
    int link_mask;
    DNXC_INIT_FUNC_DEFS;
    

    DNXC_IF_ERR_EXIT(READ_RTP_LINK_INTEGRITY_VECTORr(unit, intergrity_vec));
    if(SHR_BITGET(intergrity_vec,link_id)) {

        nof_links_in_dcl = SOC_DNXF_DEFS_GET(unit, nof_links_in_dcl);

        internal_link_id = link_id % nof_links_in_dcl;
        blk_id = INT_DEVIDE(link_id, nof_links_in_dcl);
        
        switch(internal_link_id)
        {
            case 0:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_0r(unit, blk_id, &reg_val));
                break;
            case 1:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_1r(unit, blk_id, &reg_val)); 
                break;
            case 2:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_2r(unit, blk_id, &reg_val));
                break;
            case 3:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_3r(unit, blk_id, &reg_val));
                break;
            case 4:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_4r(unit, blk_id, &reg_val));
                break;
            case 5:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_5r(unit, blk_id, &reg_val));
                break;
            case 6:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_6r(unit, blk_id, &reg_val));
                break;
            case 7:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_7r(unit, blk_id, &reg_val));
                break;
            case 8:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_8r(unit, blk_id, &reg_val));
                break;
            case 9:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_9r(unit, blk_id, &reg_val));
                break;
            case 10:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_10r(unit, blk_id, &reg_val));
                break;
            case 11:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_11r(unit, blk_id, &reg_val));
                break;
            case 12:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_12r(unit, blk_id, &reg_val));
                break;
            case 13:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_13r(unit, blk_id, &reg_val));
                break;
            case 14:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_14r(unit, blk_id, &reg_val));
                break;
            case 15:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_15r(unit, blk_id, &reg_val));
                break;
            case 16:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_16r(unit, blk_id, &reg_val));
                break;
            case 17:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_17r(unit, blk_id, &reg_val)); 
                break;
            case 18:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_18r(unit, blk_id, &reg_val));
                break;
            case 19:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_19r(unit, blk_id, &reg_val));
                break;
            case 20:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_20r(unit, blk_id, &reg_val));
                break;
            case 21:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_21r(unit, blk_id, &reg_val));
                break;
            case 22:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_22r(unit, blk_id, &reg_val));
                break;
            case 23:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_23r(unit, blk_id, &reg_val));
                break;
            case 24:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_24r(unit, blk_id, &reg_val)); 
                break;
            case 25:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_25r(unit, blk_id, &reg_val));
                break;
            case 26:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_26r(unit, blk_id, &reg_val));
                break;
            case 27:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_27r(unit, blk_id, &reg_val));
                break;
            case 28:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_28r(unit, blk_id, &reg_val));
                break;
            case 29:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_29r(unit, blk_id, &reg_val)); 
                break;
            case 30:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_30r(unit, blk_id, &reg_val));
                break;
            case 31:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_31r(unit, blk_id, &reg_val));
                break;
#ifdef BCM_88790
             case 32:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_32r(unit, blk_id, &reg_val));
                break;
             case 33:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_33r(unit, blk_id, &reg_val));
                break;
             case 34:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_34r(unit, blk_id, &reg_val));
                break;
             case 35:
                DNXC_IF_ERR_EXIT(READ_DCL_CONNECTIVITY_LINK_35r(unit, blk_id, &reg_val));
                break;
#endif
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong link id")));
        }
        
        if(reg_val & (0x1 << 22)) /*bit 22 is valid indication*/
        {
            link_partner->module_id = reg_val & 0x7FF;
            
            source_lvl = (reg_val >> 11) & 0x7;
            
            /*3'b110:fe1
              3'bX11-fe2
              3'b010-fe3
              3'bX0x-fap*/
          
          
            switch(source_lvl)
            {
                case soc_dnxc_device_type_actual_value_FAP_1:
                case soc_dnxc_device_type_actual_value_FIP:
                case soc_dnxc_device_type_actual_value_FOP:
                case soc_dnxc_device_type_actual_value_FAP:
                    link_partner->device_type = bcmFabricDeviceTypeFAP;
                    break;
                case soc_dnxc_device_type_actual_value_FE3:
                case soc_dnxc_device_type_actual_value_FE1:
                    link_partner->device_type = bcmFabricDeviceTypeFE13;
                    break;
                case soc_dnxc_device_type_actual_value_FE2:
                case soc_dnxc_device_type_actual_value_FE2_1:
                    link_partner->device_type = bcmFabricDeviceTypeFE2;
                    break;
            } 
            
            link_mask = SOC_DNXF_DEFS_GET(unit, link_mask);
            link_partner->link_id = (reg_val >> 14) & link_mask;
        }
        else
        {
            link_partner->link_id = DNXC_FABRIC_LINK_NO_CONNECTIVITY; 
        }
    } else {
        link_partner->link_id = DNXC_FABRIC_LINK_NO_CONNECTIVITY; 
    }
    
exit:
    DNXC_FUNC_RETURN;
}

#endif /*defined(BCM_88790_A0)*/

#undef _ERR_MSG_MODULE_NAME

