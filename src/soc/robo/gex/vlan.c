/*
 * $Id: vlan.c,v 1.17 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include "robo_gex.h"

#ifdef GEX_EVT_IDT_SUPPORT

static drv_egr_vt_db_info_t  egr_vt_db[SOC_MAX_NUM_PORTS];

static int new_evr_vt_entry_id = 0;    /* read-clear for return to BCM layer */

/* global var for 
 *  1. indicating the double tagging mode (none/ DT_MODE/ iDT_MODE)
 *  2. log the CFP for supporting EVR group and entry initial status.
 *      - this is a run-time init flag. 
 *      - Field bcm API was init after VLAN API so here we designed as a 
 *          runtime init mechanism.
 */
static int device_dtag_mode = DRV_VLAN_DT_MODE_DISABLE;

/*
 *  Function : _drv_vulcan_evr_vt_sw_search
 *      - search if a entry with given port+vid is existed.
 *
 *  Parameter :
 *      - entry_id  : the target entry_id(sw) if existed. the most close
 *                  entry_id(sw) if not existed.
 *      - free_id   : the first found free entry_id(sw). if no entry been free
 *                  this value will = DRV_EVRID_VT_SW_MAX_CNT.
 *                  (the free_entry.next and free_entry.prev will be "-1")
 *  Return :
 *      TRUE(1)     : search result is existed.
 *      FALSE(0)    : search result is not existed. 
 *  Note : 
 *  1. return 0(False) is not found. and 
 *      a. entry_id = DRV_EVRID_VT_SW_MAX_CNT when the table on this port
 *          is full. else
 *      b. entry_id = (valid entry index) to point to the most close item in 
 *          this sorted table on this port. The real case for the search 
 *          result might be one of below:
 *          - (entry_id).ori_vid > vid(not full yet and this entry_id 
 *              indicating the fisrt one entry within vid large than given vid
 *          - (entry_id).ori_vid < vid(not full yet and all exist entries' vid
 *              are smaller than vid. 
 *      c. entry_id = -1. no entry created on this port.
 *  2. if entry is found, the entry_id indicate the match one(port+vid). 
 *
 */
static int 
_drv_gex_evr_vt_sw_search(int unit, uint32 port, 
                uint16 vid, int *entry_id, int *free_id)
{
    
    int     evr_sw_db_head;
    int     i, found = FALSE;
    uint16  temp_vid;
    
    drv_vlan_egr_vt_info_t  *temp_evr_sw_db_entry;
    
    /* for Vulcan, this routine is port based DB search */
    assert(port < SOC_ROBO_MAX_NUM_PORTS);
    evr_sw_db_head = egr_vt_db[port].start_id;
    
    /* check if VID is valid */
    if (vid < 1 || vid > 4094){
        LOG_INFO(BSL_LS_SOC_VLAN,
                 (BSL_META_U(unit,
                             "%s: invalid VID=%d\n"), FUNCTION_NAME(), vid));
        return FALSE;
    }
    
    /* check if sw_db is empty */
    if (egr_vt_db[port].count == 0){
        *entry_id = -1;
        *free_id = 0;
        return FALSE;
    }
    
    for (i = evr_sw_db_head; i < DRV_EVRID_VT_SW_MAX_CNT; 
                    i = temp_evr_sw_db_entry->next_id){
        
        temp_evr_sw_db_entry = egr_vt_db[port].egr_vt_info + i;
        *entry_id = i;
        
        temp_vid = temp_evr_sw_db_entry->ori_vid;
        if (temp_vid == vid){
            found = TRUE;
            break;
        } else if(temp_vid > vid) {
            found = FALSE;
            break;
        }
    }
    
    /* get the free entry index */
    *free_id = -1;
    for (i = 0; i < DRV_EVRID_VT_SW_MAX_CNT; i++){
        temp_evr_sw_db_entry = egr_vt_db[port].egr_vt_info + i;
        /* next = -1 and prev = -1 means this node is free */
        if ((temp_evr_sw_db_entry->next_id == -1) && 
                    (temp_evr_sw_db_entry->prev_id == -1)){
            *free_id = i;
            break;
        }
    }
    
    /* check if sw_db is full */
    if (found){
        return TRUE;
    } else {
        if (egr_vt_db[port].count == DRV_EVRID_VT_SW_MAX_CNT){
            *entry_id = DRV_EVRID_VT_SW_MAX_CNT;
            *free_id = DRV_EVRID_VT_SW_MAX_CNT;
        }
        return FALSE;
    }
    
}
  
/*
 *  Function : _drv_gex_evr_vt_sw_db_update
 *      - update the EVR sw database for different operation.
 *  Parmeter :
 *      op      :   insert | delete | reset
 *      port    :   port
 *      vid     :   vid
 *      vt_mode :   mapping | trasparent
 *      fast_id :   the most closed index
 *      this_id :   the operating index
 *      
 *  Note : 
 *
 */
static void 
_drv_gex_evr_vt_sw_db_update(int op, uint32 port, 
                    uint16  ori_vid, uint16  new_vid, int vt_mode,
                    int fast_id, int this_id)
{
    int temp_id;
    
    /* check valid port */
    if (port >= SOC_ROBO_MAX_NUM_PORTS){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: invalid port=%d\n"), FUNCTION_NAME(), port));
        return;   
    }
    
    if (!(IS_VALID_EGRVT_DB_ENTRY_ID(fast_id))){
        if (fast_id != -1) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META("%s: invalid case on fast_id \n"), FUNCTION_NAME()));
            return;
        }
    }

    if (!(IS_VALID_EGRVT_DB_ENTRY_ID(this_id))){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META("%s: invalid case on this_id \n"), FUNCTION_NAME()));
        return;   
    }
    
    switch(op){
    case EVR_SWDB_OP_FLAG_INSERT :
        if (fast_id == -1){     /* means the first node */
            egr_vt_db[port].start_id = this_id;
            egr_vt_db[port].egr_vt_info[this_id].prev_id = -1;
            egr_vt_db[port].egr_vt_info[this_id].next_id = 
                            DRV_EVRID_VT_SW_MAX_CNT;
        } else {
            /* insert to the front of fast_id node */
            if (egr_vt_db[port].egr_vt_info[fast_id].ori_vid > ori_vid){
                /* check if head */
                if (egr_vt_db[port].egr_vt_info[fast_id].prev_id == -1){
                    /* insert to the head */
                    egr_vt_db[port].start_id = this_id;
                    
                    egr_vt_db[port].egr_vt_info[this_id].prev_id = -1;
                    egr_vt_db[port].egr_vt_info[this_id].next_id = fast_id;
                    
                    egr_vt_db[port].egr_vt_info[fast_id].prev_id = this_id;
                } else {
                    /* insert to normal */
                    temp_id = egr_vt_db[port].egr_vt_info[fast_id].prev_id;
                    
                    egr_vt_db[port].egr_vt_info[temp_id].next_id = this_id;
                    
                    egr_vt_db[port].egr_vt_info[this_id].prev_id = temp_id;
                    egr_vt_db[port].egr_vt_info[this_id].next_id = fast_id;
                    
                    egr_vt_db[port].egr_vt_info[fast_id].prev_id = this_id;
                    
                }
            } else {    /* insert to the end of fast_id node */
                temp_id = egr_vt_db[port].egr_vt_info[fast_id].next_id;
            
                egr_vt_db[port].egr_vt_info[fast_id].next_id = this_id;
                
                egr_vt_db[port].egr_vt_info[this_id].prev_id = fast_id;
                egr_vt_db[port].egr_vt_info[this_id].next_id = temp_id;
                
                if (temp_id != DRV_EVRID_VT_SW_MAX_CNT){
                    /* this case should not be happened */
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META("%s: invalid case on fast_id=%d \n"), 
                               FUNCTION_NAME(), fast_id));
                    egr_vt_db[port].egr_vt_info[temp_id].prev_id = this_id;
                }
            }
        }
        egr_vt_db[port].egr_vt_info[this_id].ori_vid = ori_vid;
        egr_vt_db[port].egr_vt_info[this_id].new_vid = new_vid;
        egr_vt_db[port].egr_vt_info[this_id].vt_mode = vt_mode;
        
        egr_vt_db[port].count ++;
        break;
    case EVR_SWDB_OP_FLAG_DELETE : 
        /* the first node */
        if (egr_vt_db[port].egr_vt_info[this_id].prev_id == -1){ 
            temp_id = egr_vt_db[port].egr_vt_info[this_id].next_id;
            egr_vt_db[port].start_id = temp_id;
            
            egr_vt_db[port].egr_vt_info[temp_id].prev_id = -1;
        
        /* the last node */
        } else if(egr_vt_db[port].egr_vt_info[this_id].next_id == 
                        -1){
            temp_id = egr_vt_db[port].egr_vt_info[this_id].prev_id;
            egr_vt_db[port].egr_vt_info[temp_id].next_id = -1;                
        
        /* normal node */
        } else {  
            temp_id = egr_vt_db[port].egr_vt_info[this_id].prev_id;
            egr_vt_db[port].egr_vt_info[temp_id].next_id = 
                        egr_vt_db[port].egr_vt_info[this_id].next_id;
                        
            temp_id = egr_vt_db[port].egr_vt_info[this_id].next_id;
            egr_vt_db[port].egr_vt_info[temp_id].prev_id = 
                        egr_vt_db[port].egr_vt_info[this_id].prev_id;
        }

        egr_vt_db[port].egr_vt_info[this_id].ori_vid = 0;
        egr_vt_db[port].egr_vt_info[this_id].new_vid = 0;
        egr_vt_db[port].egr_vt_info[this_id].vt_mode = 0;
        egr_vt_db[port].egr_vt_info[this_id].prev_id = -1;
        egr_vt_db[port].egr_vt_info[this_id].next_id = -1;
        
        egr_vt_db[port].count --;
        break;
    case EVR_SWDB_OP_FLAG_RESET :
        /* -------- TBD -------- */
        break;
    default :
        break;
    }
}
    

/*
 *  Function : _drv_gex_evr_entry_set
 *      - to set a Egress VLAN Remark(EVR) table entry.
 *
 *  Parameter :
 *      - ent_id : must be read id on EVR table (not sw used DB_ID)
 *
 *  Note : 
 *      1. ent_id will be valid only in the pre-defined id range for 
 *          Vulcan used rang. (check the robo_gex.h)
 */
static int 
_drv_gex_evr_entry_set(int unit, uint32 port, int ent_id, 
                    uint16 out_op, uint16 out_vid, 
                    uint16 in_op, uint16 in_vid)
{

    egress_vid_remark_entry_t   evr_ent;
    uint32  fld_val32;
    uint32  entry_addr;
    
    /* Check the validation on entry index */
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: port=%d,entry_id=%d,out_op=%d,in_op=%d\n"), 
              FUNCTION_NAME(), port, ent_id, out_op, in_op));
    if ((ent_id < 0) || 
            (ent_id > (DRV_EVRID_VT_ACTION_FISRT + 
                    DRV_EVRID_VT_SW_MAX_CNT - 1))){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: invalid ent_id=%d\n"), FUNCTION_NAME(), ent_id));
        return SOC_E_FAIL;
    }
    
    /* check valid op */
    if (!(IS_EVR_OP_VALID(out_op) && IS_EVR_OP_VALID(in_op))){
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "%s: invalid op flag! out_op=%d, in_op=%d\n"), 
                   FUNCTION_NAME(), out_op,in_op));
        return SOC_E_PARAM;
    }
    
    /* ---- set default entry into EVR table ---- */
    sal_memset(&evr_ent, 0, sizeof (evr_ent));

    fld_val32 = EVT_OP_FLAG_TO_OP_VALUE(out_op);
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_OP,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32));
    fld_val32 = out_vid & DRV_EVR_VID_MASK;
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_OUTER_VID,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32));
    fld_val32 = EVT_OP_FLAG_TO_OP_VALUE(in_op);
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_OP,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32));
    fld_val32 = in_vid & DRV_EVR_VID_MASK;
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
            (unit, DRV_MEM_EGRVID_REMARK, DRV_MEM_FIELD_INNER_VID,
                (uint32 *)&evr_ent, 
                (uint32 *)&fld_val32));

    entry_addr = DRV_EGR_V2V_ENTRY_ID_BUILD(port, ent_id);
    SOC_IF_ERROR_RETURN(DRV_MEM_WRITE
                    (unit, DRV_MEM_EGRVID_REMARK, entry_addr, 1, 
                    (uint32 *)&evr_ent));
    
    return SOC_E_NONE;
}

#if 0 /* Masked it for not uesd */   
/*
 *  Function : _drv_gex_evr_entry_get
 *      - to get a Egress VLAN Remark(EVR) table entry.
 *
 *  Parameter :
 *      - ent_id : must be read id on EVR table (not sw used DB_ID)
 *
 */
static int 
_drv_gex_evr_entry_get(int unit, uint32 port, int ent_id, 
                uint16 *out_op, uint16 *out_vid, 
                uint16 *in_op, uint16 *in_vid)
{

    /* dummy currently! for no request on this routine */
    return SOC_E_NONE;
}
#endif
    
    
/*
 *  Function : _drv_gex_evr_init
 *      - clear every entry on each port.
 *
 *  Note : 
 *  1. init mem and the sw database.
 *  2. this routine will operates when VLAN init.
 */
static int 
_drv_gex_evr_init(int unit)
{
    uint32  reg_value, temp;
    int     rv, i, j;
    int     retry;
    uint16  t_outer_op, t_inner_op;
    int     port;
    soc_pbmp_t temp_bmp;

    /* clear EVR table 
     *  1. here we use the HW provied reset bit to speed the init progress.
     */
    temp = 1;
    soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
        RESET_EVTf, &temp);
    soc_EGRESS_VID_RMK_TBL_ACSr_field_set(unit, &reg_value, 
        START_DONEf, &temp);
    if ((rv = REG_WRITE_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
         goto mem_write_exit;
    }
    
    /* wait for complete */
    for (retry = 0; retry < SOC_TIMEOUT_VAL; retry++) {
        if ((rv = REG_READ_EGRESS_VID_RMK_TBL_ACSr(unit, &reg_value)) < 0) {
            goto mem_write_exit;
        }
        soc_EGRESS_VID_RMK_TBL_ACSr_field_get(unit, &reg_value, 
            START_DONEf, &temp);
        if (!temp) {
            break;
        }
    }
    if (retry >= SOC_TIMEOUT_VAL) {
        rv = SOC_E_TIMEOUT;
        goto mem_write_exit;
    }
    
    /* 1.1 assigning the default EVR entry on each port 
     *  - entry 0 on EVR table will be referenced on the iDT mode if no 
     *      CFP hit to drive a newClassID.
     *  a. normal egress port : {OuterOp(Removed),InnerOp(AsRx)}
     *      >> all ports default is none-isp port so the DefaultOp=Remove.
     *  b. CPU egress port : As default {OuterOp(AsIs),InnerOp(AsRx)}
     * 
     * Note :
     *  1. CFP action is bypast on the CPU egress direct packet when device 
     *     is at iDT mode. Such packet will use the ClassID=0 to reference 
     *     EVR table.
     */
    SOC_PBMP_ASSIGN(temp_bmp, PBMP_PORT_ALL(unit));
    SOC_PBMP_PORT_ADD(temp_bmp, CMIC_PORT(unit));
    SOC_PBMP_ITER(temp_bmp, port){ 
        if (IS_CPU_PORT(unit, port)){
            /* innerOp set to AsRx is the most reasonable for normal double 
             * tagging process. (The same behavior as other ROBO devices)
             */
            t_outer_op = DRV_EVR_OP_FLAG_ASIS;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
        } else {
            t_outer_op = DRV_EVR_OP_FLAG_REMOVE;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
        }
        rv = _drv_gex_evr_entry_set(unit, port, DRV_EVRID_TAG_ACTION_DEF, 
                        t_outer_op, 0, t_inner_op, 0);
    }
    
    /* 2. -------- reset iDT_Mode ----------- */
    if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
        return rv;
    }
    soc_VLAN_CTRL4r_field_get(unit, &reg_value, 
        EN_DOUBLE_TAGf, &temp);
        
    temp &= ~DRV_VLAN_INTELLIGENT_DT_MODE;

    soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
        EN_DOUBLE_TAGf, &temp);
        
    if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
        return rv;
    }
    device_dtag_mode = temp;
   
    /* 2.1 -------- reset ISP port ----------- */
    if ((rv = REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* CPU set to ISP port when init */
    reg_value |= (1 << CMIC_PORT(unit));
    if ((rv = REG_WRITE_ISP_SEL_PORTMAPr(unit, &reg_value)) < 0) {
        return rv;
    }
    /* assigning the isp port bitmap */
    SOC_PBMP_CLEAR(temp_bmp);
    SOC_PBMP_WORD_SET(temp_bmp, 0, reg_value);

    /* 3. -------- EVR database init ----------- */
    sal_memset(egr_vt_db, 0 , 
                sizeof(drv_egr_vt_db_info_t) * SOC_MAX_NUM_PORTS);
    
    /* assign the none zero initial value */
    for (i=0; i<SOC_MAX_NUM_PORTS; i++){
        egr_vt_db[i].start_id = -1;
        egr_vt_db[i].isp_port = SOC_PBMP_MEMBER(temp_bmp, i) ? TRUE : FALSE;
        for (j=0; j<DRV_EVRID_VT_SW_MAX_CNT; j++){
            egr_vt_db[i].egr_vt_info[j].prev_id = -1;
            egr_vt_db[i].egr_vt_info[j].next_id = -1;
        }
    }
    
mem_write_exit:     
    return rv;
}

/*
 *  Function : _drv_gex_evr_vt_isp_change
 *      - maintain the EVR entry for VT and DT_mode action on those given 
 *          ports (dt_mode changed ports)
 *
 *  Note : 
 *  1. ISP port : The Outer tag action is "As Is"
 *  2. None-ISP port : The Outer tag action is "Remove"
 */
static int 
_drv_gex_evr_vt_isp_change(int unit, soc_pbmp_t changed_bmp)
{
    
    int     new_isp, vt_mode, port, evr_db_id, real_id, rv = 0;
    uint16  vt_new_vid;
    drv_vlan_egr_vt_info_t  *evr_sw_db_entry;
    uint16  t_outer_op, t_inner_op;
    
    /* set the default EVR table entry no matther the iDT or DT mode :
     *  - loop on each port in port bitmap.
     */
    SOC_PBMP_ITER(changed_bmp, port){
        /* CPU default entry won't be change */
        if (IS_CPU_PORT(unit, port)){
            continue;
        }
        
        SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_GET
                        (unit, DRV_VLAN_PROP_ISP_PORT, 
                         port,  (uint32 *) &new_isp));
        if (new_isp){   /* ISP port */
            t_outer_op = DRV_EVR_OP_FLAG_ASIS;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
            
        } else {    /* None-ISP port */
            t_outer_op = DRV_EVR_OP_FLAG_REMOVE;
            t_inner_op = DRV_EVR_OP_FLAG_ASRX;
        }
        rv = _drv_gex_evr_entry_set(unit, port, DRV_EVRID_TAG_ACTION_DEF, 
                        t_outer_op, 0, t_inner_op, 0);
    }
    
    /* check if iDT_Mode is enabled */
    if (!(device_dtag_mode & DRV_VLAN_INTELLIGENT_DT_MODE)){
        /* debug message */
        LOG_INFO(BSL_LS_SOC_VLAN,
                 (BSL_META_U(unit,
                             "EVR table can't work if iDT_Mode is not enabled!\n")));
    }
    
    /* loop on each port in port bitmap and assigning new isp_mode action on 
     *  each port's VT entries.
     */
    SOC_PBMP_ITER(changed_bmp, port){
        
        /* check if EVR is empty */
        if (IS_PORT_EGRVT_EMPTY(port)){
            continue;
        }
        
        new_isp = egr_vt_db[port].isp_port;
        for (evr_db_id = egr_vt_db[port].start_id; 
                    evr_db_id < DRV_EVRID_VT_SW_MAX_CNT;
                    evr_db_id = evr_sw_db_entry->next_id){
                        
            evr_sw_db_entry = egr_vt_db[port].egr_vt_info + evr_db_id;
            vt_mode = evr_sw_db_entry->vt_mode;
            vt_new_vid = evr_sw_db_entry->new_vid;
            real_id = EGRVT_DB_ID_TO_REAL_ID(evr_db_id);
            
            if (new_isp){
                /* vt_mode : 1 is Mapping mode; 0 is transparent mode */
                if (vt_mode){ 
                    /* Action :
                     *  1. outer->Modify; 
                     *  2. inner->remove; 
                     *  3. outer/inner vid reassign 
                     */
                    rv = _drv_gex_evr_entry_set(
                                unit, port, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, vt_new_vid, 
                                DRV_EVR_OP_FLAG_REMOVE, 0);
                    assert(rv == SOC_E_NONE);
                } else {
                    /* Action :
                     *  1. outer->Modify; 
                     *  2. inner->as_is; 
                     *  3. outer/inner vid reassign 
                     */
                    rv = _drv_gex_evr_entry_set(
                                unit, port, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, vt_new_vid, 
                                DRV_EVR_OP_FLAG_ASIS, 0);
                }
            } else {    /* new none-isp */
            
                /* Action :
                 *  1. outer->Remove; 
                 *  2. inner->Modify; 
                 *  3. outer/inner vid reassign 
                 */
                rv = _drv_gex_evr_entry_set(
                            unit, port, real_id, 
                            DRV_EVR_OP_FLAG_REMOVE, 0, 
                            DRV_EVR_OP_FLAG_MODIFY, vt_new_vid);
            }
        }
            
    }
    
    return rv;
}

/*
 *  Function : drv_vlan_vt_add
 *
 *  Purpose :
 *      Add the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     :   VT table type. (ingress/egress/..) 
 *      port        :   port id. 
 *      cvid        :   customer vid(= inner_vid = old_vid)
 *      sp_vid      :   service provide vid( = outer_vid = new_vid)
 *      pri         :   priority (not used in ingress VT)
 *      mode        :   vt_mode (trasparent / mapping)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. Vulcan support device basis VLAN translation though the 
 *          Egress VLAN Remark(EVR) table is egress port basis.
 *          - this is for the VLAN XLATE have to work with ingress basis 
 *              filetering (CFP).
 *      2. priority is not supported in EVR table.
 */
int 
drv_gex_vlan_vt_add(int unit, uint32 vt_type, uint32 port,  uint32 cvid, 
                uint32 sp_vid, uint32 pri, uint32 mode)
{
    int     i, rv = SOC_E_NONE;
    int  search_id = 0, free_id = 0,real_id;
    int     temp_isp_mode;
    soc_pbmp_t  temp_bmp;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: vt_type=%d,port=%d,ori_vid=%d,new_vid=%d\n"),
              FUNCTION_NAME(), vt_type, port, cvid, sp_vid));
    switch (vt_type){
    case DRV_VLAN_XLAT_EVR :
        /* in Vulcan, VLAN XLATE is ingress filtering but egress action */
                
        /* can't keep going if previous creating id still not been read */
        if (new_evr_vt_entry_id != 0){
            return SOC_E_BUSY;
        }
        
        /* 1. check the the port+vid is existed 
         *  - return if the entry is existed already.
         *  - else popup the proper sw entry id which is sorted by 
         *      vid in this port(keep the sw entry id).
         *  - hard coded the port = 0, for the VLAN XLATE in Vulcan is 
         *      designed as device basis currently. That means all port on 
         *      the same entry id will serivce the same VLAN trnaslation.
         */
        if (_drv_gex_evr_vt_sw_search(unit, 0, cvid, 
                        &search_id, &free_id)){     /* exist */
            LOG_INFO(BSL_LS_SOC_VLAN,
                     (BSL_META_U(unit,
                                 "%s:EVR for port=%d,vid=%d is existed! Can't Add it!\n"), 
                      FUNCTION_NAME(), port, cvid));
    
            return SOC_E_EXISTS;
        } else {    /* not exist */
            /* check if full */
            if ((search_id == DRV_EVRID_VT_SW_MAX_CNT) ||
                        (free_id == DRV_EVRID_VT_SW_MAX_CNT)){
                LOG_VERBOSE(BSL_LS_SOC_VLAN,
                            (BSL_META_U(unit,
                                        "%s:EVR for port=%d,vid=%d is existed! Can't Add it!\n"), 
                             FUNCTION_NAME(), port, cvid));
                return SOC_E_FULL;
            }
            
        }
        
        /* 2. add the EVR entry on NNI and UNI ports */
        assert(IS_VALID_EGRVT_DB_ENTRY_ID(free_id));
        
        real_id = EGRVT_DB_ID_TO_REAL_ID(free_id);

        SOC_PBMP_ASSIGN(temp_bmp, PBMP_PORT_ALL(unit));
        SOC_PBMP_REMOVE(temp_bmp, PBMP_CMIC(unit));

        SOC_PBMP_ITER(temp_bmp, i){
            if (IS_CPU_PORT(unit, i)){
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "%s: EVR on CPU is not implemented\n"), 
                           FUNCTION_NAME()));
                continue;
            }

            temp_isp_mode = egr_vt_db[i].isp_port;
            sp_vid &= DRV_EVR_VID_MASK; 
            if (temp_isp_mode == TRUE){    /* set ISP port */
                /* mode : 1 is means Mapping mode; 0 is transparent mode */
                if (mode){  
                    /* outer -> Modify; inner -> remove */  
                    rv = _drv_gex_evr_entry_set(
                                unit, i, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, sp_vid, 
                                DRV_EVR_OP_FLAG_REMOVE, 0);
                } else {
                    /* outer -> Modify; inner -> as_is */  
                    rv = _drv_gex_evr_entry_set(
                                unit, i, real_id, 
                                DRV_EVR_OP_FLAG_MODIFY, sp_vid, 
                                DRV_EVR_OP_FLAG_ASIS, 0);
                }
                
                assert(rv == SOC_E_NONE);
                
            } else {    /* set none-ISP port */
                /* mode is not proper to set on UNI port  */
                /* outer -> remove; inner -> modify */  
                rv = _drv_gex_evr_entry_set(
                            unit, i, real_id, 
                            DRV_EVR_OP_FLAG_REMOVE, 0, 
                            DRV_EVR_OP_FLAG_MODIFY, sp_vid);
            }
            assert(rv == SOC_E_NONE);
            
            /* 3. maintain port based sw database */
            _drv_gex_evr_vt_sw_db_update(EVR_SWDB_OP_FLAG_INSERT, i,
                            cvid, sp_vid, mode, search_id, free_id);
        }
                
        /* keep the new created id and waiting for read-clear */
        new_evr_vt_entry_id = real_id;
        
        break;
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_EGRESS:
        rv = SOC_E_UNAVAIL;
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }
    
  return rv;
}

/*
 *  Function : drv_vlan_vt_delete
 *
 *  Purpose :
 *      Delete the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     :   VT table type. (ingress/egress/..) 
 *      port        :   port id. (not used for ingress VT)
 *      vid         :   VLAN ID 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. Vulcan VLAN XLATE is designed as device basis.
 *      2. not port VT delete allowed in Vulcan.
 */
int 
drv_gex_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid)
{
    int     i, rv = SOC_E_NONE;
    int  search_id = 0, free_id, real_id;
    soc_pbmp_t  temp_bmp;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: vt_type=%d,port=%d,vid=%d\n"),
              FUNCTION_NAME(), vt_type, port, vid));
    switch (vt_type){
    case DRV_VLAN_XLAT_EVR :
        /* can't keep going if previous creating id still not been read */
        if (new_evr_vt_entry_id != 0){
            return SOC_E_BUSY;
        }
        
        /* 1. check the existence on the port+vid 
         *  - hard coded the port = 0, for the VLAN XLATE in Vulcan is 
         *      designed as device basis currently. That means all port on 
         *      the same entry id will serivce the same VLAN trnaslation.
         */
        if ((_drv_gex_evr_vt_sw_search(unit, 0, vid, 
                        &search_id, &free_id)) == FALSE){ 
            LOG_INFO(BSL_LS_SOC_VLAN,
                     (BSL_META_U(unit,
                                 "%s:EVR for port=%d,vid=%d is not existed!\n"), 
                      FUNCTION_NAME(), port, vid));
    
            return SOC_E_NOT_FOUND;
        } else {    /* exist */
            if (!(IS_VALID_EGRVT_DB_ENTRY_ID(search_id))){
                LOG_VERBOSE(BSL_LS_SOC_VLAN,
                            (BSL_META_U(unit,
                                        "%s:Unexcepted EVR search result!\n"), FUNCTION_NAME()));
                return SOC_E_INTERNAL;
            }
        }
        
        /* 2. remove EVR table : set all zero value to the entry */
        real_id = EGRVT_DB_ID_TO_REAL_ID(search_id); 

        SOC_PBMP_ASSIGN(temp_bmp, PBMP_PORT_ALL(unit));
        SOC_PBMP_REMOVE(temp_bmp, PBMP_CMIC(unit));

        SOC_PBMP_ITER(temp_bmp, i){
            rv = _drv_gex_evr_entry_set(unit, i, real_id, 
                            DRV_EVR_OP_FLAG_ASIS, 0, 
                            DRV_EVR_OP_FLAG_ASIS, 0);
                            
            /* 3. maintain port based sw database */
            _drv_gex_evr_vt_sw_db_update(EVR_SWDB_OP_FLAG_DELETE, i,
                            vid, 0, 0, 0, search_id);
        }

        break;
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_EGRESS:
        
        rv = SOC_E_UNAVAIL;
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 *  Function : drv_vlan_vt_delete_all
 *
 *  Purpose :
 *      Delete all the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      vt_type     :   VT table type. (ingress/egress/..) 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_gex_vlan_vt_delete_all(int unit, uint32 vt_type)
{
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: vt_type=%d\n"), FUNCTION_NAME(), vt_type));
    switch (vt_type){
    case DRV_VLAN_XLAT_EVR :
        
        /* no calling reference currently */
        rv = SOC_E_NONE;
        break;
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_EGRESS:
        rv = SOC_E_UNAVAIL;
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }
    return rv;
}

/*
 *  Function : drv_vlan_vt_set
 *
 *  Purpose :
 *      Set the VLAN translation property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      port         : port id. (not used for ingress VT)
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff (int = -1), means get device basis value.
 *      2. sp_tpid currently will be 0x8100 / 0x9100 / 0x9200 /???
 *
 */
int 
drv_gex_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 prop_val)
{
    uint32  field_val32;
    uint32  reg_value;
    int     rv = SOC_E_NONE;
    
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop_type=%d,vid=%d,port=%d,val=%x\n"),
              FUNCTION_NAME(), unit, prop_type, vid, port, prop_val));
    switch(prop_type){
    case DRV_VLAN_PROP_VT_MODE:      /*  trasparent / mapping */
        /* for bcm53115, no upper layer calling reference yet! */
        break;
    case DRV_VLAN_PROP_EGR_VT_SPVID:     /* egress SP VID */
    case DRV_VLAN_PROP_EGR_VT_PRI:
    case DRV_VLAN_PROP_ING_VT_SPVID:     /* ingress SP VID */
    case DRV_VLAN_PROP_ING_VT_PRI:
    case DRV_VLAN_PROP_EGR_VT_SPTPID:    /* egress SP TPID */
        rv = SOC_E_UNAVAIL;
        break;
    case DRV_VLAN_PROP_ING_VT_SPTPID:    /* ingress SP TPID */
        SOC_IF_ERROR_RETURN(REG_READ_DTAG_TPIDr(unit, &reg_value));
        
        field_val32 = prop_val;
        soc_DTAG_TPIDr_field_set(unit, &reg_value, 
            ISP_TPIDf, &field_val32);
        
        break;
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 *  Function : drv_vlan_vt_get
 *
 *  Purpose :
 *      Get the VLAN translation property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      port        : port id. (not used for ingress VT)
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff (int = -1), means get device basis value.
 *      2. VLAN translation in Vulcan is designed as system basis.
 *
 */
int 
drv_gex_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 *prop_val)
{
    uint32  field_val32 = 0;
    uint32  reg_value;
    int     rv = SOC_E_NONE;
    uint32  vt_mode, new_vid;
    int  search_id = 0, free_id ;
    drv_vlan_egr_vt_info_t  *temp_evt_sw_db_entry;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop_type=%d,vid=%d,port=%d\n"),
              FUNCTION_NAME(), unit, prop_type, vid, port));

    switch(prop_type){
    case DRV_VLAN_PROP_ING_VT_SPTPID:
        SOC_IF_ERROR_RETURN(REG_READ_DTAG_TPIDr(unit, &reg_value));
        
        soc_DTAG_TPIDr_field_get(unit, &reg_value, 
            ISP_TPIDf, &field_val32);
        *prop_val = field_val32;
        
        break;
    case DRV_VLAN_PROP_VT_MODE:      /*  trasparent / mapping */
    case DRV_VLAN_PROP_EGR_VT_SPVID:     /* egress SP VID */

        /* can't keep going if previous creating id still not been read */
        if (new_evr_vt_entry_id != 0){
            return SOC_E_BUSY;
        }
        
        /* 1. check the existence on the port+vid 
         *  - hard coded the port = 0, for the VLAN XLATE in bcm53115 is 
         *      designed as device basis currently. That means all port on 
         *      the same entry id will serivce the same VLAN trnaslation.
         */
        if ((_drv_gex_evr_vt_sw_search(unit, 0, vid, 
                        &search_id, &free_id)) == FALSE){ 
            LOG_INFO(BSL_LS_SOC_VLAN,
                     (BSL_META_U(unit,
                                 "%s:EVR for port=%d,vid=%d is not existed!\n"), 
                      FUNCTION_NAME(), port, vid));
    
            return SOC_E_NOT_FOUND;
        } else {    /* exist */
            if (!(IS_VALID_EGRVT_DB_ENTRY_ID(search_id))){
                LOG_VERBOSE(BSL_LS_SOC_VLAN,
                            (BSL_META_U(unit,
                                        "%s:Unexcepted EVR search result!\n"), FUNCTION_NAME()));
                return SOC_E_INTERNAL;
            }
        }
        
        /* 2. get the searched VT_Mode */
        temp_evt_sw_db_entry = egr_vt_db[port].egr_vt_info + search_id;
        
        if (prop_type == DRV_VLAN_PROP_VT_MODE) {
            
            vt_mode = temp_evt_sw_db_entry->vt_mode;
            
            *prop_val = vt_mode;
        } else { /* prop_type == DRV_VLAN_PROP_EGR_VT_SPVID */
            
            new_vid = temp_evt_sw_db_entry->new_vid;
            
            *prop_val = new_vid;
        }
        break;
    case DRV_VLAN_PROP_EGR_VT_PRI:
    case DRV_VLAN_PROP_ING_VT_SPVID:     /* ingress SP VID */
    case DRV_VLAN_PROP_ING_VT_PRI:
    case DRV_VLAN_PROP_EGR_VT_SPTPID:    /* egress SP TPID */
    default :
        rv = SOC_E_UNAVAIL;
        break;
    }

    return rv;
}

/*
 *  Function : _drv_gex_vt_idt_prop_set
 *
 *  Purpose :
 *      Set the VLAN at VT or IDT property value.
 *
 *  Note :
 *  1. in Vulcan, double tagging mode can be DT_Mode(original design)
 *      or iDT_Mode(new design, means intelligent DT_Mode).
 *      - Our design for this feature are :
 *          a. user enable double tagging mode -> set to DT_Mode.
 *          b. user enable vlan translation -> set to iDT_Mode.
 */
static int 
_drv_gex_vt_idt_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_value, temp;
    int rv = SOC_E_NONE;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32  current_dt, cfi_rmk_enabled;
    int     t_port;
    pbmp_t  t_pbm;
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, prop_val));
    switch (prop_type) {
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
    case DRV_VLAN_PROP_IDT_MODE_ENABLE:
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        
        /* if prefer iDT_mode is set, the working flow here will be 
         *  - no DT_mode through SoC driver. 
         *
         * if prefer DT_mode is set, for the concern about VT feature, the 
         *  working flow must handle mode transfer between DT_mode and 
         *  iDT_mode when device DT enabled.
         *  - DT_mode is set when DT is enabled from disable.
         *  - iDT_mode is set internally only. Once the VT related process is
         *      requested, the iDT_mode will be set. 
         */ 
        if (DRV_VLAN_PREFER_DT_MODE == DRV_VLAN_INTELLIGENT_DT_MODE) {
            temp = (prop_val == TRUE) ? 
                    DRV_VLAN_INTELLIGENT_DT_MODE : DRV_VLAN_DT_MODE_DISABLE;
            
        } else {    /* DT_Mode */
            soc_VLAN_CTRL4r_field_get(unit, &reg_value, 
                EN_DOUBLE_TAGf, &temp);
            if (prop_type == DRV_VLAN_PROP_IDT_MODE_ENABLE){ /* iDT_mode */
                /* check current value */
                if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                    if (prop_val){
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    } else {
                        temp &= ~DRV_VLAN_INTELLIGENT_DT_MODE;
                    }
                } else {
                    if (prop_val){
                        temp = DRV_VLAN_INTELLIGENT_DT_MODE;
                    } else {
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    }
                }
            } else {    /* DT_mode flow */
                /* check current value */
                if (temp & DRV_VLAN_NORMAL_DT_MODE){
                    if (prop_val){
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    } else {
                        temp &= ~DRV_VLAN_NORMAL_DT_MODE;
                        if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                            temp &= ~DRV_VLAN_INTELLIGENT_DT_MODE;
                        }
                    }
                } else {
                    if (prop_val){
                        if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                            /* return for Vulcan is at iDT_mode */
                            return SOC_E_NONE;
                        } else {
                            temp = DRV_VLAN_NORMAL_DT_MODE;
                        }
                    } else {
                        /* return for nothing to change */
                        return SOC_E_NONE;
                    }
                }
            }
        }

        soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_DOUBLE_TAGf, &temp);
            
        if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        device_dtag_mode = temp;
                        
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            /* NS+, SF3 special design :
             *  In this routine if DT mode is changed and CFI egress remarking 
             *  is currently enabled, we need to call to CFI remarking process 
             *  again to ensure the CFI remarking can still working properly.
             */
            current_dt = temp;
            if (current_dt != device_dtag_mode) {
                /* DT mode changed, check if CFI remarking is enabled */
                SOC_PBMP_CLEAR(t_pbm); 
                PBMP_E_ITER(unit, t_port){
                    SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, t_port, 
                            DRV_PORT_PROP_EGRESS_CFI_REMARK, &cfi_rmk_enabled));
                    if (cfi_rmk_enabled){
                        SOC_PBMP_PORT_ADD(t_pbm, t_port);
                    }
                }
                
                SOC_PBMP_COUNT(t_pbm, temp);
                if (temp > 0) {
                    LOG_WARN(BSL_LS_SOC_COMMON,
                             (BSL_META_U(unit,
                                         "%s:re-enable CFI egress remark setting.\n"),
                              FUNCTION_NAME()));
                    SOC_IF_ERROR_RETURN(DRV_PORT_SET(unit, 
                            t_pbm, DRV_PORT_PROP_EGRESS_CFI_REMARK, TRUE));
                }

            }
            device_dtag_mode = current_dt;
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            device_dtag_mode = temp;
        }
        break;
    case DRV_VLAN_PROP_V2V:  /* device based VT enabling */
        
        /* enable/disable global VT feature */
        if ((prop_val) == (device_dtag_mode & DRV_VLAN_INTELLIGENT_DT_MODE)){
            /* current setting is the same */
            return SOC_E_NONE;
        }
        temp = (prop_val) ? DRV_VLAN_INTELLIGENT_DT_MODE : 
                (device_dtag_mode & (~DRV_VLAN_INTELLIGENT_DT_MODE));
        
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_DOUBLE_TAGf, &temp);
        if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        
        device_dtag_mode = temp;
        
        break;
    case DRV_VLAN_PROP_EVR_INIT: /* Egress VLAN Remark(EVR) init */
        if (prop_val){
            /* init EVR only, the related CFP will be init in the runtime 
             *  (This is for the Field bcm API is not init yet during VLAN
             *      init section.)
             */
            rv =_drv_gex_evr_init(unit);
        }
        break;
    case DRV_VLAN_PROP_EVR_VT_ISP_CHANGE: /* Egress VLAN Remark(EVR) init */
        if (prop_val){
            soc_pbmp_t  changed_bmp;
            
            SOC_PBMP_CLEAR(changed_bmp);
            SOC_PBMP_WORD_SET(changed_bmp, 0, prop_val);
            
            rv =_drv_gex_evr_vt_isp_change(unit, changed_bmp);
        }
        break;
    default:
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

static int 
_drv_gex_vt_idt_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_value, temp = 0;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, *prop_val));
    switch (prop_type) {
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
    case DRV_VLAN_PROP_IDT_MODE_ENABLE:     /* check iDT_Mode status */
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL4r_field_get(unit, &reg_value, 
                EN_DOUBLE_TAGf, &temp);

        /* if the DT mode is preferred at iDT_mode.
         *  - only iDT mode is the recoganized DT mode.
         *  - when the DT_mode is retrived then the DT mode will be set to 
         *    DT mode disabled and return false.
         */
        temp &= DRV_VLAN_DT_MODE_MASK;
        if (DRV_VLAN_PREFER_DT_MODE == DRV_VLAN_INTELLIGENT_DT_MODE) {
            /* preferred at iDT_mode and indicating to get iDT_mode */
            if (prop_type == DRV_VLAN_PROP_IDT_MODE_ENABLE){
                *prop_val = (temp & DRV_VLAN_INTELLIGENT_DT_MODE) ? 
                                TRUE : FALSE ;
            } else {   
                if (temp & DRV_VLAN_INTELLIGENT_DT_MODE){
                    *prop_val = TRUE;
                } else {
                    *prop_val = FALSE;
                    if (temp & DRV_VLAN_NORMAL_DT_MODE){
                        /* disable double tagging mode */
                        LOG_WARN(BSL_LS_SOC_COMMON,
                                 (BSL_META_U(unit,
                                             "%s: Internal modification from DT to iDT!\n"),
                                  FUNCTION_NAME()));
                        temp = DRV_VLAN_DT_MODE_DISABLE;
                        soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                            EN_DOUBLE_TAGf, &temp);
                            
                        if ((rv = REG_WRITE_VLAN_CTRL4r(
                            unit, &reg_value)) < 0) {
                            return rv;
                        }
                    }
                }
            }
        } else {
            if (prop_type == DRV_VLAN_PROP_IDT_MODE_ENABLE){
                *prop_val = (temp & DRV_VLAN_INTELLIGENT_DT_MODE) ? 
                                TRUE : FALSE ;
            } else {
                *prop_val = (temp & DRV_VLAN_NORMAL_DT_MODE) ? 
                            TRUE : ((temp & DRV_VLAN_INTELLIGENT_DT_MODE) ? 
                                TRUE : FALSE) ;
            }
        }
         
        break;
    case DRV_VLAN_PROP_V2V:  /* device based VT enabling */
        *prop_val = (device_dtag_mode & DRV_VLAN_INTELLIGENT_DT_MODE); 

        break;
    case DRV_VLAN_PROP_EVR_VT_NEW_ENTRY_ID:
        *prop_val = new_evr_vt_entry_id;
        new_evr_vt_entry_id = 0;
        break;
    default:
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

#endif  /* GEX_EVT_IDT_SUPPORT */

static int 
_drv_gex_no_vt_idt_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_value, temp;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, prop_val));
    switch (prop_type) {
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        if (prop_val){
            temp = DRV_VLAN_NORMAL_DT_MODE;
        } else {
            temp = DRV_VLAN_DT_MODE_DISABLE;
        }

        soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_DOUBLE_TAGf, &temp);
            
        if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        
        break;
    default:
        rv = SOC_E_UNAVAIL;
    }
    
    return rv;
}

static int 
_drv_gex_no_vt_idt_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_value, temp = 0;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, *prop_val));
    switch (prop_type) {
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL4r_field_get(unit, &reg_value,
            EN_DOUBLE_TAGf, &temp);

        if (temp == DRV_VLAN_NORMAL_DT_MODE) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        
        break;
    default:
        rv = SOC_E_UNAVAIL;
    
    }
    
    return rv;
}

/*
 *  Function : drv_gex_vlan_prop_set
 *
 *  Purpose :
 *      Set the VLAN  property value.
 */
int 
drv_gex_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_value, temp;
    int rv = SOC_E_NONE;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 pbmp = 0;
#endif    

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, prop_val));
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        if ((rv = REG_READ_VLAN_CTRL5r(unit, &reg_value)) < 0) {
             return rv;
        }
        if (prop_val) {
           temp = 1;
        } else {
           temp = 0;
        }
        soc_VLAN_CTRL5r_field_set(unit, &reg_value, 
            DROP_VTABLE_MISSf, &temp);
        if ((rv = REG_WRITE_VLAN_CTRL5r(unit, &reg_value)) < 0) {
             return rv;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            /* use port based setting to approach system settign */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_IVL_SVL_CTRLr(unit, 
                    &reg_value));
            temp = 1;
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_set(unit, &reg_value, 
                    PORT_IVL_SVL_ENf, &temp));
            /* Note : IVL/SVL selection value is different between system 
             * and port based.
             * - user requets TRUE for SVL and FLASE for IVL.
             */
            if (SOC_IS_STARFIGHTER3(unit)) {
                pbmp = 0x12F;
            }
            if (SOC_IS_NORTHSTARPLUS(unit)) {
                pbmp = 0x1BF;
            }
            temp = (prop_val) ? pbmp : 0;
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_set(unit, &reg_value, 
                    PORT_IVL_SVL_SELf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_PORT_IVL_SVL_CTRLr(unit, 
                    &reg_value));
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
                return rv;
            }
            if (prop_val) {
                temp = 0;
            } else {
                temp = 3;
            }
            soc_VLAN_CTRL0r_field_set(unit, &reg_value, 
                VLAN_LEARN_MODEf, &temp);
    
            if ((rv = REG_WRITE_VLAN_CTRL0r(unit, &reg_value)) < 0) {
                return rv;
            }
        }
        break;            
    case DRV_VLAN_PROP_SP_TAG_TPID:
#if defined(BCM_DINO8_SUPPORT) || defined(BCM_DINO16_SUPPORT)
        if (SOC_IS_DINO(unit)) {
            return SOC_E_UNAVAIL;
        }
#endif /* BCM_DINO8_SUPPORT || BCM_DINO16_SUPPORT */
        if ((rv = REG_READ_DTAG_TPIDr(unit, &reg_value)) < 0) {
            return rv;
        }

        temp = prop_val & 0xFFFF;
        soc_DTAG_TPIDr_field_set(unit, &reg_value, 
            ISP_TPIDf, &temp);

        if ((rv = REG_WRITE_DTAG_TPIDr(unit, &reg_value)) < 0) {
            return rv;
        }
            
        break;
    default:
#ifdef GEX_EVT_IDT_SUPPORT
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = _drv_gex_vt_idt_prop_set(unit, prop_type, prop_val);
        } else if (SOC_IS_BLACKBIRD(unit) || SOC_IS_LOTUS(unit) || 
            SOC_IS_BLACKBIRD2(unit)) {
            rv = _drv_gex_no_vt_idt_prop_set(unit, prop_type, prop_val);
        } else if (SOC_IS_DINO(unit)) {
            rv = SOC_E_UNAVAIL;
        } else {
            /* unexpected process here */
            rv = SOC_E_NONE;
        }
#else   /* GEX_EVT_IDT_SUPPORT */
        if (SOC_IS_BLACKBIRD(unit) || SOC_IS_LOTUS(unit) || 
            SOC_IS_BLACKBIRD2(unit)) {
            rv = _drv_gex_no_vt_idt_prop_set(unit, prop_type, prop_val);
        } else if (SOC_IS_DINO(unit)) {
            rv = SOC_E_UNAVAIL;
        } else {
            /* unexpected process here */
            rv = SOC_E_NONE;
        }
#endif  /* GEX_EVT_IDT_SUPPORT */
    }
   
    return rv;
}

/*
 *  Function : drv_vlan_prop_get
 *
 *  Purpose :
 *      Get the VLAN property value.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_type   :   vlan property type.
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_gex_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_value, temp = 0;
    int rv = SOC_E_NONE;
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32 pbmp = 0;
#endif    

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit = %d, property type = %d, value = %x\n"),
              FUNCTION_NAME(), unit, prop_type, *prop_val));
    switch (prop_type) {
    case DRV_VLAN_PROP_VTABLE_MISS_DROP:
        if ((rv = REG_READ_VLAN_CTRL5r(unit, &reg_value)) < 0) {
             return rv;
        }
        soc_VLAN_CTRL5r_field_get(unit, &reg_value, 
            DROP_VTABLE_MISSf, &temp);
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        break;
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            /* report TRUE for SVL and FALSE for IVL */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_IVL_SVL_CTRLr(unit, 
                    &reg_value));
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_get(unit, 
                    &reg_value, PORT_IVL_SVL_ENf, &temp));

            if (temp) {
                SOC_IF_ERROR_RETURN(
                        soc_PORT_IVL_SVL_CTRLr_field_get(unit,
                        &reg_value, PORT_IVL_SVL_SELf, &temp));

                if (SOC_IS_STARFIGHTER3(unit)) {
                    pbmp = 0x12F;
                }
                if (SOC_IS_NORTHSTARPLUS(unit)) {
                    pbmp = 0x1BF;
                }
                /* the report value will be converted in the caller */
                if (temp == 0) {
                    /* all at IVL */
                    *prop_val = TRUE;
                } else if (temp == pbmp) {
                    /* all at SVL */
                    *prop_val = FALSE;
                } else {
                    /* not all at SVL */
                    *prop_val = TRUE;
                }
            } else {
                SOC_IF_ERROR_RETURN(REG_READ_VLAN_CTRL0r(unit, 
                    &reg_value));
                SOC_IF_ERROR_RETURN(soc_VLAN_CTRL0r_field_get(unit, 
                        &reg_value, VLAN_LEARN_MODEf, &temp));
                /* the report value will be converted in the caller */
                *prop_val = (temp) ? TRUE : FALSE; 
            }
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            if ((rv = REG_READ_VLAN_CTRL0r(unit, &reg_value)) < 0) {
                return rv;
            }
            soc_VLAN_CTRL0r_field_get(unit, &reg_value, 
                VLAN_LEARN_MODEf, &temp);
            
            if (temp) {
                *prop_val = TRUE;
            } else {
                *prop_val = FALSE;
            }
        }
        break;
    default:
#ifdef GEX_EVT_IDT_SUPPORT
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            rv = _drv_gex_vt_idt_prop_get(unit, prop_type, prop_val);
        } else if (SOC_IS_BLACKBIRD(unit) || SOC_IS_LOTUS(unit) || 
            SOC_IS_BLACKBIRD2(unit)) {
            rv = _drv_gex_no_vt_idt_prop_get(unit, prop_type, prop_val);
        } else if (SOC_IS_DINO(unit)) {
            rv = SOC_E_UNAVAIL;
        } else {
            /* unexpected process here */
            rv = SOC_E_NONE;
        }
#else   /* GEX_EVT_IDT_SUPPORT */
        if (SOC_IS_BLACKBIRD(unit) || SOC_IS_LOTUS(unit) || 
            SOC_IS_BLACKBIRD2(unit)) {
            rv = _drv_gex_no_vt_idt_prop_get(unit, prop_type, prop_val);
        } else if (SOC_IS_DINO(unit)) {
            rv = SOC_E_UNAVAIL;
        } else {
            /* unexpected process here */
            rv = SOC_E_NONE;
        }
#endif  /* GEX_EVT_IDT_SUPPORT */
    }
    
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_set
 *
 *  Purpose :
 *      Set the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      bmp         : port bitmap
 *      val         : value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *
 */
int 
drv_gex_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{    
    uint32  reg_value = 0, temp = 0;
    int rv = SOC_E_NONE;
    soc_pbmp_t set_bmp, temp_bmp;
#if defined(GEX_EVT_IDT_SUPPORT) || defined(BCM_LOTUS_SUPPORT)
    int port;
#endif  /* GEX_EVT_IDT_SUPPORT || BCM_LOTUS_SUPPORT */
#if defined( BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
    uint32  fld_val = 0;
    uint32  cfi_rmk_enabled;
    int     t_port;
    pbmp_t  t_pbm, en_pbm;
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop=%d, value=0x%x\n"), 
              FUNCTION_NAME(), unit, prop_type, val));
    
    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        SOC_IF_ERROR_RETURN(
            REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value));
        soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value, 
            ISP_PORTMAPf, &temp);
    
        SOC_PBMP_CLEAR(temp_bmp);
        SOC_PBMP_WORD_SET(temp_bmp, 0, temp);
        
        /* check the action process */
        SOC_PBMP_CLEAR(set_bmp);
        SOC_PBMP_OR(set_bmp, temp_bmp);
        
        if (val == TRUE){       /* set for enable */
            SOC_PBMP_OR(set_bmp, bmp);
        }else {
            SOC_PBMP_REMOVE(set_bmp, bmp);
        }
            
        /* check if the set value is equal to current setting */
        if (SOC_PBMP_EQ(temp_bmp, set_bmp)){
            /* do nothing */
            return SOC_E_NONE;
        }
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        /* to retrieve the pbmp of changed setting */
        SOC_PBMP_ASSIGN(t_pbm, temp_bmp);
        SOC_PBMP_XOR(t_pbm, set_bmp);
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
       
#ifdef GEX_EVT_IDT_SUPPORT
        if (SOC_IS_VULCAN(unit) || SOC_IS_STARFIGHTER(unit) || 
            SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
            SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            /* GNATS 41031 :
             *  - After VT entries created, once the ISP is changed to NON-ISP, 
             *    the unexpected outer tag will be transmitted from this changed 
             *    port(NON-ISP alrady). 
             */
            /* set sw flag on isp_port */
            SOC_PBMP_ITER(PBMP_ALL(unit), port){
                if (SOC_PBMP_MEMBER(set_bmp, port)){
                    egr_vt_db[port].isp_port = TRUE;
                } else {
                    egr_vt_db[port].isp_port = FALSE;
                }
            }
        }
#endif  /* GEX_EVT_IDT_SUPPORT */

        /* write to register */
        temp = SOC_PBMP_WORD_GET(set_bmp, 0);    

        soc_ISP_SEL_PORTMAPr_field_set(unit, &reg_value, 
            ISP_PORTMAPf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_ISP_SEL_PORTMAPr(unit, &reg_value));

        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)){
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            /* NS+,SF3 special design :
             *  if ISP mode is changed and CFI egress remarking is currently 
             *  enabled, we need to call to CFI remarking process again to 
             *  ensure the CFI remarking can still working properly.
             */
            SOC_PBMP_CLEAR(en_pbm); 
            SOC_PBMP_ITER(t_pbm, t_port){
                SOC_IF_ERROR_RETURN(DRV_PORT_GET(unit, t_port, 
                        DRV_PORT_PROP_EGRESS_CFI_REMARK, 
                        &cfi_rmk_enabled));
                if (cfi_rmk_enabled){
                    SOC_PBMP_PORT_ADD(en_pbm, t_port);
                }
            }
            SOC_PBMP_COUNT(en_pbm, temp);
            if (temp > 0) {
                LOG_WARN(BSL_LS_SOC_COMMON,
                         (BSL_META_U(unit,
                                     "%s:re-enable CFI egress remark setting.\n"),
                          FUNCTION_NAME()));
                SOC_IF_ERROR_RETURN(DRV_PORT_SET(unit, 
                        en_pbm, DRV_PORT_PROP_EGRESS_CFI_REMARK, TRUE));
            }
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        }

        break;
#ifdef GEX_EVT_IDT_SUPPORT
    case    DRV_VLAN_PROP_V2V_PORT :
        /* 1. VLAN XLATE was supported in Vulcan with device basis 
         *       (port will be ignored)
         * 2. function boundled with iDT_mode.
         */
        SOC_IF_ERROR_RETURN(drv_gex_vlan_prop_set
                        (unit, DRV_VLAN_PROP_V2V, val));
        break;
    case    DRV_VLAN_PROP_INNER_TAG_PORT :
        /* Vulcan can support this , TBD */
        rv = SOC_E_UNAVAIL;
        break;
#endif  /* GEX_EVT_IDT_SUPPORT */
#ifdef BCM_LOTUS_SUPPORT
    case DRV_VLAN_PROP_POLICING:
        if (SOC_IS_LOTUS(unit)) {
            if (val == TRUE) {
                temp = 1;
            } else {
                temp = 0;
            }
            PBMP_ITER(bmp, port) {
                if (IS_CPU_PORT(unit, port)) {
                    SOC_IF_ERROR_RETURN(
                        REG_READ_BC_SUP_RATECTRL_IMPr(unit, 0, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(
                        REG_READ_BC_SUP_RATECTRL_Pr(unit, port, &reg_value));
                }
                soc_BC_SUP_RATECTRL_Pr_field_set(unit, &reg_value, 
                    EN_VLAN_POLICINGf, &temp);
                if (IS_CPU_PORT(unit, port)) {
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_BC_SUP_RATECTRL_IMPr(unit, 0, &reg_value));
                } else {
                    SOC_IF_ERROR_RETURN(
                        REG_WRITE_BC_SUP_RATECTRL_Pr(unit, port, &reg_value));
                }
            }
            /* Unmask the packet type */
            if (temp) {




                SOC_IF_ERROR_RETURN(DRV_RATE_CONFIG_SET(
                    unit, PBMP_ALL(unit), DRV_RATE_CONFIG_PKT_MASK, 0x0));
            }
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
#endif  /* BCM_LOTUS_SUPPORT */
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined (BCM_STARFIGHTER3_SUPPORT)
    case DRV_VLAN_PROP_JOIN_ALL_VLAN:
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
            SOC_IF_ERROR_RETURN(
                REG_READ_JOIN_ALL_VLAN_ENr(unit, &reg_value));
            soc_JOIN_ALL_VLAN_ENr_field_get(unit, &reg_value ,
                                JOIN_ALL_VLAN_ENf, &fld_val);
            temp = SOC_PBMP_WORD_GET(bmp, 0);
            if (val) {
                fld_val |= temp;
            } else {
                fld_val &= ~temp;
            }
            soc_JOIN_ALL_VLAN_ENr_field_set(unit, &reg_value ,
                                JOIN_ALL_VLAN_ENf, &fld_val);
            SOC_IF_ERROR_RETURN(
                REG_WRITE_JOIN_ALL_VLAN_ENr(unit, &reg_value));
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */        
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit) ) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            /* use port based setting to approach system settign */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_IVL_SVL_CTRLr(unit, 
                    &reg_value));
            temp = 1;
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_set(unit, &reg_value, 
                    PORT_IVL_SVL_ENf, &temp));
            /* Note : IVL/SVL selection value is different between system 
             * and port based.
             * - user requets TRUE for SVL and FLASE for IVL.
             */
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_get(unit, &reg_value, 
                    PORT_IVL_SVL_SELf, &temp));

            PBMP_ITER(bmp, port) {
                if (port < SOC_ROBO_MAX_NUM_PORTS) {
                    if (val) {
                        temp |= (0x1 << port);
                    } else {
                        temp &= ~(0x1 << port);
                    }
                }
            }
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_set(unit, &reg_value, 
                    PORT_IVL_SVL_SELf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_PORT_IVL_SVL_CTRLr(unit, 
                    &reg_value));
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            rv = SOC_E_UNAVAIL; 
        }
        break;
    default :
        return SOC_E_UNAVAIL;
        break;
    }
     
    return rv;
}

/*
 *  Function : drv_vlan_prop_port_enable_get
 *
 *  Purpose :
 *      Get the port enable status by different VLAN property.
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      prop_val    :   vlan property value.
 *      port_n      : port number. 
 *      val         : (OUT) value
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff , means get device basis value.

 *
 */
int 
drv_gex_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    uint32  reg_value = 0, temp = 0;
    soc_pbmp_t pbmp;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop=%d, port=%d\n"), 
              FUNCTION_NAME(), unit, prop_type, port_n));

    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        SOC_IF_ERROR_RETURN(
            REG_READ_ISP_SEL_PORTMAPr(unit, &reg_value));
        soc_ISP_SEL_PORTMAPr_field_get(unit, &reg_value, 
            ISP_PORTMAPf, &temp);
    
        /* check if the value get is port basis or device basis. */
        SOC_PBMP_CLEAR(pbmp);
        SOC_PBMP_WORD_SET(pbmp, 0, temp);
        
        if (port_n == 0xffffffff) {     /* device basis */
            int     i;
    
            for (i = 0; i < SOC_PBMP_WORD_MAX; i++){
                *(val + i) = SOC_PBMP_WORD_GET(pbmp, i);
            }
        } else {
            *val = (SOC_PBMP_MEMBER(pbmp, port_n)) ? TRUE : FALSE;
        }
    
        break;
#ifdef GEX_EVT_IDT_SUPPORT
    case    DRV_VLAN_PROP_V2V_PORT :
        /* 1. VLAN XLATE was supported in Vulcan with device basis 
         *       (port will be ignored)
         * 2. function boundled with iDT_mode.
         */
        SOC_IF_ERROR_RETURN(drv_gex_vlan_prop_get
            (unit, DRV_VLAN_PROP_V2V, val));
        break;
    case    DRV_VLAN_PROP_INNER_TAG_PORT :
        /* bcm53115 can support this , TBD */
        rv = SOC_E_UNAVAIL;
        break;
#endif  /* GEX_EVT_IDT_SUPPORT */
#ifdef BCM_LOTUS_SUPPORT
    case DRV_VLAN_PROP_POLICING:
        if (SOC_IS_LOTUS(unit)) {
            if (IS_CPU_PORT(unit, port_n)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_BC_SUP_RATECTRL_IMPr(unit, 0, &reg_value));
            } else {
                SOC_IF_ERROR_RETURN(
                    REG_READ_BC_SUP_RATECTRL_Pr(unit, port_n, &reg_value));
            }
            soc_BC_SUP_RATECTRL_Pr_field_get(unit, &reg_value, 
                    EN_VLAN_POLICINGf, &temp);
            if (temp) {
                *val = TRUE;
            } else {
                *val = FALSE;
            }
        } else {
            rv = SOC_E_UNAVAIL;
        }
        break;
#endif  /* BCM_LOTUS_SUPPORT */
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
        case DRV_VLAN_PROP_JOIN_ALL_VLAN:
            if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
                SOC_IF_ERROR_RETURN(
                    REG_READ_JOIN_ALL_VLAN_ENr(unit, &reg_value));
                soc_JOIN_ALL_VLAN_ENr_field_get(unit, &reg_value ,
                                    JOIN_ALL_VLAN_ENf, &temp);
                if (temp & (0x1 << port_n)) {
                    *val = TRUE;
                } else {
                    *val = FALSE;
                }
            } else {
                rv = SOC_E_UNAVAIL;
            }
            break;
#endif /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
    case DRV_VLAN_PROP_VLAN_LEARNING_MODE:
        if (SOC_IS_NORTHSTARPLUS(unit) || SOC_IS_STARFIGHTER3(unit)) {
#if defined(BCM_NORTHSTARPLUS_SUPPORT) || defined(BCM_STARFIGHTER3_SUPPORT)
            /* user expects TRUE for SVL and FLASE for IVL */
            SOC_IF_ERROR_RETURN(REG_READ_PORT_IVL_SVL_CTRLr(unit, 
                    &reg_value));
            SOC_IF_ERROR_RETURN(
                    soc_PORT_IVL_SVL_CTRLr_field_get(unit, 
                    &reg_value, PORT_IVL_SVL_ENf, &temp));

            if (temp) {
                SOC_IF_ERROR_RETURN(
                        soc_PORT_IVL_SVL_CTRLr_field_get(unit,
                        &reg_value, PORT_IVL_SVL_SELf, &temp));
                *val = (temp & (0x1 << port_n)) ? TRUE : FALSE;
            } else {    
                /* return TRUE if systme is at all SVL mode */
                SOC_IF_ERROR_RETURN(REG_READ_VLAN_CTRL0r(unit, 
                    &reg_value));
                SOC_IF_ERROR_RETURN(soc_VLAN_CTRL0r_field_get(unit, 
                        &reg_value, VLAN_LEARN_MODEf, &temp));
                *val = (!temp) ? TRUE : FALSE; 
            }
#endif  /* BCM_NORTHSTARPLUS_SUPPORT || BCM_STARFIGHTER3_SUPPORT */
        } else {
            rv = SOC_E_UNAVAIL; 
        }
        break;

    default :
        return SOC_E_UNAVAIL;

    }
    return rv;

}

/*
 *  Function : drv_port_vlan_set
 *
 *  Purpose :
 *      Set the group member ports of the selected port. (port-base)
 *
 *  Parameters :
 *      unit        :   RoboSwitch unit number.
 *      port   :   port number.
 *      bmp     :   group member port bitmap.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_port_vlan_set(int unit, uint32 port, soc_pbmp_t bmp)
{
    uint32	reg_value, temp;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: port=%d, bmp=0x%x\n"),
              FUNCTION_NAME(), port, SOC_PBMP_WORD_GET(bmp, 0)));

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTL_P7r(unit, &reg_value));
    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */
    {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, &reg_value));
    }

    temp = SOC_PBMP_WORD_GET(bmp, 0);
    soc_PORT_VLAN_CTLr_field_set(unit, &reg_value,
        PORT_EGRESS_ENf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_PORT_VLAN_CTLr(unit, port, &reg_value));

    return SOC_E_NONE;
}

/*
 *  Function : drv_port_vlan_get
 *
 *  Purpose :
 *      Get the group member ports of the selected port. (port-base)
 *
 *  Parameters :
 *      unit    :   RoboSwitch unit number.
 *      port    :   port number.
 *      bmp     :   group member port bitmap.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      
 *
 */
int 
drv_gex_port_vlan_get(int unit, uint32 port, soc_pbmp_t *bmp)
{
    uint32	reg_value, temp = 0;
#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    uint32  specified_port_num;

    if (SOC_IS_POLAR(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_INTERNAL_CPU_PORT_NUM, &specified_port_num));
    } else if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
            (unit, DRV_DEV_PROP_ADDITIONAL_SOC_PORT_NUM, &specified_port_num));
    }
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT || 
        * BCM_NORTHSTARPLUS_SUPPORT
        */

#if defined(BCM_POLAR_SUPPORT) || defined(BCM_NORTHSTAR_SUPPORT) || \
    defined(BCM_NORTHSTARPLUS_SUPPORT)
    if ((SOC_IS_POLAR(unit) || SOC_IS_NORTHSTAR(unit) || 
        SOC_IS_NORTHSTARPLUS(unit)) 
        && (port == specified_port_num)) {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTL_P7r(unit, &reg_value));
    } else
#endif /* BCM_POLAR_SUPPORT || BCM_NORTHSTAR_SUPPORT ||
        * BCM_NORTHSTARPLUS_SUPPORT
        */
    {
        SOC_IF_ERROR_RETURN(
            REG_READ_PORT_VLAN_CTLr(unit, port, &reg_value));
    }
    soc_PORT_VLAN_CTLr_field_get(unit, &reg_value,
        PORT_EGRESS_ENf, &temp);
    SOC_PBMP_WORD_SET(*bmp, 0, temp);
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: port=%d, bmp=0x%x\n"),
              FUNCTION_NAME(), port, SOC_PBMP_WORD_GET(*bmp, 0)));

    return SOC_E_NONE;
}
