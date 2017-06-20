/*
 * $Id: ramon_fabric_status.c,v 1.9.48.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON FABRIC STATUS
 */
 
#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_FABRIC


#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/ramon/ramon_fabric_status.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>
#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/error.h>

#include <soc/mem.h>


/*
 * Function:
 *      soc_ramon_fabric_link_status_get
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
soc_ramon_fabric_link_status_get(int unit, soc_port_t link_id, uint32 *link_status, uint32 *errored_token_count)
{
    uint32 reg_val, field_val[1];
    uint32 sig_acc = 0;
    uint32 sig_lock[1] = {0};
    int blk_id, reg_select;
    soc_port_t inner_lnk;
    int srd_id, srd_arr_id;
    int rv;
    DNXC_INIT_FUNC_DEFS;

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
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAC_INTERRUPT_REGISTER_1r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, reg_val, RX_CRC_ERR_N_INTf);
    if(SHR_BITGET(field_val, inner_lnk)) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_CRC_ERROR;
    }
    
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, reg_val, WRONG_SIZE_INTf);
    if(SHR_BITGET(field_val, inner_lnk)) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_SIZE_ERROR;  
    }
       
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_2r, reg_val, RX_LOST_OF_SYNCf);
    if(SHR_BITGET(field_val, inner_lnk)) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_MISALIGN;  
    }
      
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAC_INTERRUPT_REGISTER_4r(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_FMAC_INTERRUPT_REGISTER_4r, reg_val, DEC_ERR_INTf);
    if(SHR_BITGET(field_val, inner_lnk)) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_CODE_GROUP_ERROR; 
    } 
      
    srd_id = (link_id*SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd))/SOC_DNXF_DEFS_GET(unit, nof_links);
    srd_arr_id = (link_id % SOC_DNXF_DEFS_GET(unit, nof_links_in_fsrd))/SOC_DNXF_DEFS_GET(unit, nof_links_in_quad); /* quad number in fsrd */
    DNXC_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_STATUSr(unit, srd_id, srd_arr_id, &reg_val));
    *sig_lock = soc_reg_field_get(unit, FSRD_SRD_QUAD_STATUSr, reg_val, SRD_QUAD_N_SYNC_STATUSf);
    if(!(SHR_BITGET(sig_lock,inner_lnk))) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_NO_SIG_LOCK;
    }
 
    
    rv = soc_dnxc_port_rx_locked_get(unit, link_id, &sig_acc);
    DNXC_IF_ERR_EXIT(rv);
    if(!sig_acc) {
        *link_status |= DNXC_FABRIC_LINK_STATUS_NO_SIG_ACCEP;
    }
    
    if(*errored_token_count < 63) {
       *link_status |= DNXC_FABRIC_LINK_STATUS_ERRORED_TOKENS;
    }
    
    /*Clear sticky indications*/
    rv = soc_ramon_fabric_link_status_clear(unit, link_id);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
  
}

/*
 * Function:
 *      soc_ramon_fabric_link_status_clear
 * Purpose:
 *      Clear link status - interrupts
 * Parameters:
 *      unit                - (IN)  Unit number.
 *      link                - (IN) Link #
 * Returns:
 *      SOC_E_xxx
 */
soc_error_t 
soc_ramon_fabric_link_status_clear(int unit, soc_port_t link)
{
    int blk_id, inner_link;
    uint32 reg_val;
    DNXC_INIT_FUNC_DEFS;

    blk_id = link / 4;
    inner_link = link % 4;

    reg_val = 0x0;
    soc_reg_field_set(unit, FMAC_FMAC_INTERRUPT_REGISTER_1r, &reg_val, RX_CRC_ERR_N_INTf, 1 << inner_link);
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


#undef _ERR_MSG_MODULE_NAME
