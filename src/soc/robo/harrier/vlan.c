/*
 * $Id: vlan.c,v 1.4 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

#define VT_SPVID_MASK       0xfff
#define VT_OPMODE_MASK      0x1

/* VT_Mode value deifnition for VT entry read/write */
#define VT_MODE_TRANSPARENT     0
#define VT_MODE_MAP             1

/* SW-info for bcm53242/53262 only */
#define V2V_ENT_COUNT       4096
#define V2V_ENT_PORT_COUNT  128     /* port based V2V entry count(per port) */
#define V2V_ENT_VALID_MASK  0x8000
#define V2V_ENT_MODE_MASK   0x1000

#define SYSTEM_BASED_VT 0   /* system basis VT mode */
#define PORT_BASED_VT   1   /* port basis VT mode(supported after B0 chip) */
#define IS_PORT_BASED_VT        (port_vt == PORT_BASED_VT)

#define V2V_ENT_VALID_SET(_ent_id)   \
            igr_vt_db.vt_list[(_ent_id)] |= V2V_ENT_VALID_MASK  
#define V2V_ENT_VALID_RESET(_ent_id)   \
            igr_vt_db.vt_list[(_ent_id)] &= ~V2V_ENT_VALID_MASK  
#define V2V_ENT_VALID_CHK(_ent_id)   \
            ((igr_vt_db.vt_list[(_ent_id)] & V2V_ENT_VALID_MASK) ? \
            TRUE : FALSE)
#define V2V_ENT_MODE_SET(_ent_id, _mode)   \
            if ((_mode) == VT_MODE_MAP) {       \
                igr_vt_db.vt_list[(_ent_id)] |= V2V_ENT_MODE_MASK;  \
            } else {    /* transparent */       \
                igr_vt_db.vt_list[(_ent_id)] &= ~V2V_ENT_MODE_MASK; \
            }
#define V2V_ENT_MODE_GET(_ent_id)   \
            ((igr_vt_db.vt_list[(_ent_id)] & V2V_ENT_MODE_MASK) ? \
            VT_MODE_MAP : VT_MODE_TRANSPARENT)
#define V2V_ENT_VID_GET(_ent_id)   \
            (igr_vt_db.vt_list[(_ent_id)] & VT_SPVID_MASK)
#define V2V_ENT_VID_SET(_ent_id, _vid)   \
            (igr_vt_db.vt_list[(_ent_id)] &= ~VT_SPVID_MASK);   \
            (igr_vt_db.vt_list[(_ent_id)] |= (VT_SPVID_MASK & (_vid)))
            

typedef struct drv_igr_vt_db_info_s {
   
    uint16     vt_en;       /* 1:enable/ 0:disable */
    uint16     count;       /* log the valid count in DB entry (init = 0)*/
    
    /* bcm53242 V2V table is a 4k 1QVLAN fully support VT table
     *  - 4K deep array for fully mapping the V2V table.
     *  - node definition :
     *      1. bit0-bit11 : new VID
     *      2. bit12 : vt_mode (0:transparent; 1:mapping)
     *      3. bit15 : valid bit. (speeding search)
     */
    uint16  vt_list[V2V_ENT_COUNT];    
}drv_igr_vt_db_info_t;

static drv_igr_vt_db_info_t  igr_vt_db;

/* flag for the v2v reset at speed/clear mode 
 *  - speed mode : clear the valid(SW) entry only.
 *  - clear mode : clear 4k entries.
 */
static  int v2v_reset_done = 0;
static  int port_vt = SYSTEM_BASED_VT;     /* 1:Port_VT/ 0:System_VT */

#define V2V_HW_RESET    1
#define V2V_SW_RESET    0
/*
 *  Function : _drv_v2v_reset
 *
 *  Purpose :
 *      Reset VLAN2VLAN table. 
 *
 * Note :
 *  1. HW reset feature is provided after B0 chip but SW didn't apply it for 
 *      the default V2V entry is reset to mapping mode. 
 */
static int 
_drv_v2v_reset(int unit, int reset_type)
{
    uint32  reg_value, temp, i;
    uint16  dev_id;
    uint8   rev_id;
    int     en_temp;

    if (!v2v_reset_done){
        igr_vt_db.vt_en = 0;
    }
    
    if (reset_type == V2V_SW_RESET) {
        uint32  men_ent_count = 0;
        uint32  field_val32;
        vlan2vlan_entry_t vt_entry;
        
        /* transparent mode bit field is 0 */
        sal_memset(&vt_entry, 0, sizeof (vlan2vlan_entry_t));
        
        SOC_IF_ERROR_RETURN(DRV_MEM_LENGTH_GET(unit, 
                DRV_MEM_VLANVLAN, &men_ent_count));
        for (i = 0; i < men_ent_count; i++){

            if (IS_PORT_BASED_VT){
                field_val32 = (i & VT_SPVID_MASK) % V2V_ENT_PORT_COUNT;
            } else {
                field_val32 = i & VT_SPVID_MASK;
            }
            SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET(unit, 
                    DRV_MEM_VLANVLAN, DRV_MEM_FIELD_NEW_VLANID,
                    (uint32 *)&vt_entry, &field_val32));

            SOC_IF_ERROR_RETURN(DRV_MEM_WRITE(unit, 
                    DRV_MEM_VLANVLAN,(uint32)i, 1, (uint32 *)&vt_entry));
        }
    } else if (reset_type == V2V_HW_RESET) {
        /* prevent A0 chip on calling this routine */
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if (((dev_id == BCM53242_DEVICE_ID) && 
                        (rev_id == BCM53242_A0_REV_ID)) || 
                        ((dev_id == BCM53262_DEVICE_ID) && 
                        (rev_id == BCM53262_A0_REV_ID))) {
            return SOC_E_UNAVAIL;
        }
    
        /* V2V table init */
        reg_value = 0;
        temp = 1;
        soc_RST_TABLE_MEM1r_field_set(unit, &reg_value,
            RST_VLAN2VLANf, &temp);
        
        SOC_IF_ERROR_RETURN(
            REG_WRITE_RST_TABLE_MEM1r(unit, &reg_value));
    
        /* check if the reset is done */
        for (i = SOC_TIMEOUT_VAL; i ; i--){
            SOC_IF_ERROR_RETURN(
                REG_READ_RST_TABLE_MEM1r(unit, &reg_value));
            soc_RST_TABLE_MEM1r_field_get(unit, &reg_value,
                RST_VLAN2VLANf, &temp);
            if (!temp) {
                break;
            }
        }
        if (i == 0) {
            LOG_WARN(BSL_LS_SOC_COMMON,
                     (BSL_META_U(unit,
                                 "%s: VLAN2VLAN table reset timeout!\n"), FUNCTION_NAME()));
            return SOC_E_TIMEOUT;
        }
    } else {
        return SOC_E_UNAVAIL;
    }
    
    /* reset V2V table SW-Info */
    en_temp = igr_vt_db.vt_en;
    igr_vt_db.count = 0;
    sal_memset(&igr_vt_db, 0 , sizeof(drv_igr_vt_db_info_t));
    igr_vt_db.vt_en = en_temp;
    
    /* assumed to set once in a power cycle(for performance concern) */
    v2v_reset_done = 1;     

    return SOC_E_NONE;
}

/*
 *  Function : drv_vlan_vt_add
 *
 *  Purpose :
 *      Add the a specific VLAN translation entry.
 *
 *  Parameters :
 *      unit        : RoboSwitch unit number.
 *      vt_type     : VT table type. (ingress/egress/..) 
 *      port        : port id. 
 *      cvid        : customer vid(= inner_vid = old_vid)
 *      sp_vid      : service provide vid( = outer_vid = new_vid)
 *      pri         : priority (not used in ingress VT)
 *      mode        : vt_mode (trasparent / mapping)
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. designed as VT table is VID basis. (port is not used in VT table)
 *
 */
int 
drv_harrier_vlan_vt_add(int unit, uint32 vt_type, uint32 port,  uint32 cvid, 
                uint32 sp_vid, uint32 pri, uint32 mode)
{
    int     rv = SOC_E_NONE;
    vlan2vlan_entry_t vt_entry;
    uint32  field_val32;
    uint32  vt_index;
        
    switch (vt_type){
    case DRV_VLAN_XLAT_INGRESS :
        /* check if V2V is reset already */
        if (!v2v_reset_done) {
            rv = _drv_v2v_reset(unit, V2V_SW_RESET);
            if (rv){
                LOG_INFO(BSL_LS_SOC_VLAN,
                         (BSL_META_U(unit,
                                     "%s,can't reset V2V table\n"), FUNCTION_NAME()));
            }
        }
    
        /* check if table is full */
        if (igr_vt_db.count == V2V_ENT_COUNT) {
            return SOC_E_FULL;
        }
        
        /* vid translate for sys_based/port-based */
        if (IS_PORT_BASED_VT) {
            if (cvid >= V2V_ENT_PORT_COUNT) {
                /* to support the per port per vlan translation, the 
                 * max supported vid(as VLAN2VLAN table's index) is 
                 * limited up to 127.
                 */
                return SOC_E_PARAM;
            }
            vt_index = (port << 7) + cvid;
        } else {
            vt_index = cvid;
        }
        
        /* check if existed already */
        if (V2V_ENT_VALID_CHK(vt_index) == TRUE){
            return SOC_E_EXISTS;
        }

        /* add process */
        sal_memset(&vt_entry, 0, sizeof (vlan2vlan_entry_t));

        field_val32 = mode & VT_OPMODE_MASK;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLANVLAN, DRV_MEM_FIELD_MAPPING_MODE,
                        (uint32 *)&vt_entry, &field_val32));
        
        /* sys/port basis V2V accept the 4k vid value  */
        sp_vid &= VT_SPVID_MASK;
        field_val32 = sp_vid;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLANVLAN, DRV_MEM_FIELD_NEW_VLANID,
                        (uint32 *)&vt_entry, &field_val32));

        SOC_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLANVLAN,(uint32)vt_index, 1, 
                        (uint32 *)&vt_entry));
                        
        /* update SW-Info */
        V2V_ENT_VALID_SET(vt_index);
        V2V_ENT_MODE_SET(vt_index, (mode & VT_OPMODE_MASK));
        V2V_ENT_VID_SET(vt_index, sp_vid);
        igr_vt_db.count++;
        
        break;
    case DRV_VLAN_XLAT_EGRESS:
        /* target table is DRV_MEM_FLOWVLAN !
         *      - USE CFP for this feature.
         */
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
 *      unit        : RoboSwitch unit number.
 *      vt_type     : VT table type. (ingress/egress/..) 
 *      port        : port id. (not used for ingress VT)
 *      vid         : VLAN ID 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. reset driver service is for the usage to clear/delete the ROBO 
 *          VLAN2VLAN entry.
 *      2. V2V table entry have no valid bit
 *      3. Our design to reset a VT entry by 
 *          a. mapping = 0
 *          b. new_vid = entry_id(old_vid)
 *
 */
int 
drv_harrier_vlan_vt_delete(int unit, uint32 vt_type, uint32 port, uint32 vid)
{
    int rv = SOC_E_NONE;
    vlan2vlan_entry_t vt_entry;
    uint32 field_val32, t_vid;
    uint32  vt_index;

    int     del_vt_mode;

    switch (vt_type){
    /* delete VT entry with no concerning about the VT_Mode */
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_INGRESS_MAP :
    case DRV_VLAN_XLAT_INGRESS_TRAN :
    case DRV_VLAN_XLAT_INGRESS_PER_PORT :
        /* check if V2V is reset already */
        if (!v2v_reset_done) {
            rv = _drv_v2v_reset(unit, V2V_SW_RESET);
            if (rv){
                LOG_INFO(BSL_LS_SOC_VLAN,
                         (BSL_META_U(unit,
                                     "%s,can't reset V2V table\n"), FUNCTION_NAME()));
            }
        }
    
        /* vid translate for sys_based/port-based 
         *  - if DRV_VLAN_XLAT_INGRESS_PER_PORT is assigned, that means this 
         *   is called from vt_delete_all routine to do delete all process.
         *  - Without "DRV_VLAN_XLAT_INGRESS_PER_PORT" assignment, if the 
         *   current mode is at per port V2V. Than the VID will be limited 
         *   under 128.
         */
        if (vt_type == DRV_VLAN_XLAT_INGRESS_PER_PORT) {
            /* for the special case to delete all port based V2V table
             *  - in this case, the vid will be valid from 0-4095 
             */
            vt_index = vid;
            t_vid = vid % V2V_ENT_PORT_COUNT;
        } else {
            /* check V2V table mode first */
            if (IS_PORT_BASED_VT) {
                if (vid >= V2V_ENT_PORT_COUNT) {
                    /* to support the per port per vlan translation, the 
                     *  max supported vid(as VLAN2VLAN table's index) is 
                     *  limited up to 127.
                     */
                    return SOC_E_PARAM;
                }
                
                vt_index = (port << 7) + vid;
            } else {
                vt_index = vid;
            }
            t_vid = vid;
        }
    
        /* check exist status and than check the mode */
        if (V2V_ENT_VALID_CHK(vt_index) != TRUE){
            return SOC_E_NOT_FOUND;
        } else {
            del_vt_mode = 
                    (vt_type == DRV_VLAN_XLAT_INGRESS_MAP) ? VT_MODE_MAP : 
                    ((vt_type == DRV_VLAN_XLAT_INGRESS_TRAN) ? 
                            VT_MODE_TRANSPARENT : -1);
            
            /* vt_type reassigned for the mode detect is done! */
            vt_type = DRV_VLAN_XLAT_INGRESS;
            if (del_vt_mode != -1){   /* means delete without mode detect */
                if (V2V_ENT_MODE_GET(vt_index) != del_vt_mode){
                    return SOC_E_NOT_FOUND;
                }
            }
        }
        
        /* delete process */
        sal_memset(&vt_entry, 0, sizeof (vlan2vlan_entry_t));

        /* The vid here in system basis will be 0-4095 and in port basis will 
         *  be 0-127.
         */
        field_val32 = t_vid & VT_SPVID_MASK;
        
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLANVLAN, DRV_MEM_FIELD_NEW_VLANID,
                        (uint32 *)&vt_entry, &field_val32));

        field_val32 = VT_MODE_TRANSPARENT & VT_OPMODE_MASK;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                        (unit, DRV_MEM_VLANVLAN, DRV_MEM_FIELD_MAPPING_MODE,
                        (uint32 *)&vt_entry, &field_val32));

        SOC_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, DRV_MEM_VLANVLAN,(uint32)vt_index, 1, 
                        (uint32 *)&vt_entry));
                        
        /* update SW-Info (*/
        V2V_ENT_VALID_RESET(vt_index);
        V2V_ENT_MODE_SET(vt_index, VT_MODE_TRANSPARENT);
        V2V_ENT_VID_SET(vt_index, t_vid);
        igr_vt_db.count--;
        
        break;

    case DRV_VLAN_XLAT_EGRESS:
        /* target table is DRV_MEM_FLOWVLAN !
         *      - USE CFP for this feature.
         */
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
 *      Delete all the VLAN translation entry per vt_mode.
 *
 *  Parameters :
 *      unit        : RoboSwitch unit number.
 *      vt_type     : VT table type. (ingress/egress/..) 
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. V2V table is always existed without no valid bit
 *
 */
int 
drv_harrier_vlan_vt_delete_all(int unit, uint32 vt_type)
{
    int rv = SOC_E_NONE, tmp_rv = SOC_E_NONE;
    uint32  men_ent_count = 0;
    int i;
    uint32 tmp;

    switch (vt_type){
    case DRV_VLAN_XLAT_INGRESS :
    case DRV_VLAN_XLAT_INGRESS_MAP :
    case DRV_VLAN_XLAT_INGRESS_TRAN :
        if (v2v_reset_done == 0){
            rv = _drv_v2v_reset(unit, V2V_SW_RESET);
            if (rv){
                LOG_INFO(BSL_LS_SOC_VLAN,
                         (BSL_META_U(unit,
                                     "%s,can't reset V2V table!\n"), FUNCTION_NAME()));
            }
        }
        
        /* check if there is no valid entry */
        if (igr_vt_db.count == 0){
            return rv;
        }
        
        SOC_IF_ERROR_RETURN(DRV_MEM_LENGTH_GET
                        (unit, DRV_MEM_VLANVLAN, &men_ent_count));

        tmp = 0;
        tmp_rv = DRV_VLAN_PROP_GET(unit, 
                DRV_VLAN_PROP_PER_PORT_TRANSLATION, &tmp);
        if ((tmp_rv == SOC_E_NONE) && tmp) {
            for (i = 0; i < men_ent_count; i++){
                drv_harrier_vlan_vt_delete(unit, 
                        DRV_VLAN_XLAT_INGRESS_PER_PORT, (uint32)-1, i);
            }
        } else {
            for (i = 0; i < men_ent_count; i++){
                /* entry valid or mode will be detected in delete routine */
                drv_harrier_vlan_vt_delete(unit, vt_type, (uint32)-1, i);
            }
        }

        /* update SW info */
        if (vt_type == DRV_VLAN_XLAT_INGRESS) { /* all VT delete action */
            igr_vt_db.count = 0;
        }

        break;
    case DRV_VLAN_XLAT_EGRESS:
        /* target table is DRV_MEM_FLOWVLAN !
         *      - use CFE for this feature.
         */
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
 *      port        :   port id. (not used for ingress VT)
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
drv_harrier_vlan_vt_set(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 prop_val)
{
    uint32  mem = 0, field = 0, field_val32;
    vlan2vlan_entry_t vt_entry;
    uint32  reg_value;
    uint32 tmp;
    int tmp_rv = SOC_E_NONE;
    
    switch(prop_type){
    case DRV_VLAN_PROP_VT_MODE:      /*  trasparent / mapping */
        mem = DRV_MEM_VLANVLAN;
        sal_memset(&vt_entry, 0, sizeof (vlan2vlan_entry_t));
        field = DRV_MEM_FIELD_MAPPING_MODE;
        break;
    case DRV_VLAN_PROP_ING_VT_SPVID:     /* ingress SP VID */
        mem = DRV_MEM_VLANVLAN;
        sal_memset(&vt_entry, 0, sizeof (vlan2vlan_entry_t));
        field = DRV_MEM_FIELD_NEW_VLANID;
        break;
    case DRV_VLAN_PROP_EGR_VT_SPVID:     /* egress SP VID */
        return SOC_E_UNAVAIL;
        break;
    case DRV_VLAN_PROP_ING_VT_PRI:
        return SOC_E_UNAVAIL;
        break;
    case DRV_VLAN_PROP_EGR_VT_PRI:
        return SOC_E_UNAVAIL;
        break;
    case DRV_VLAN_PROP_ING_VT_SPTPID:    /* ingress SP TPID */
        SOC_IF_ERROR_RETURN(
            REG_READ_ISP_VIDr(unit, &reg_value));
        
        field_val32 = prop_val;
        soc_ISP_VIDr_field_set(unit, &reg_value,
            ISP_VLAN_DELIMITERf, &field_val32);

        SOC_IF_ERROR_RETURN(
            REG_WRITE_ISP_VIDr(unit, &reg_value));
        return SOC_E_NONE;
        break;
    case DRV_VLAN_PROP_EGR_VT_SPTPID:    /* egress SP TPID */
        return SOC_E_UNAVAIL;
        break;
    default :
        return SOC_E_UNAVAIL;
        break;
    }

    if (mem){
        if (prop_type == DRV_VLAN_PROP_ING_VT_SPVID) {
            tmp = 0;
            tmp_rv = DRV_VLAN_PROP_GET(unit, 
                    DRV_VLAN_PROP_PER_PORT_TRANSLATION, &tmp);
            if ((tmp_rv == SOC_E_NONE) && tmp) {
                if (vid >= V2V_ENT_PORT_COUNT) {
                    /* for per port per vlan translation, 
                     * max supported vid(as VLAN2VLAN table's index) is 127.
                     */
                    return SOC_E_PARAM;
                }
                tmp = 0;
                tmp = (port << 7) + vid;
            } else {
                tmp = vid;
            }
        } else {
            tmp = vid;
        }
        /* Upper layer already checks that vid is valid */
        SOC_IF_ERROR_RETURN(DRV_MEM_READ
                    (unit, mem, tmp, 1, (uint32 *)&vt_entry));
        field_val32 = prop_val;
        SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
                    (unit, mem, field, (uint32 *)&vt_entry, &field_val32));
        SOC_IF_ERROR_RETURN(DRV_MEM_WRITE
                        (unit, mem,tmp, 1, (uint32 *)&vt_entry));
    }
    return SOC_E_NONE;
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
 *      port        :   port id. (used for port based ingress VT)
 *      prop_val    :   vlan property value.
 *
 *  Return :
 *      SOC_E_NONE
 *
 *  Note :
 *      1. if port_n = 0xffffffff (int = -1), means get device basis value.
 *
 */
int 
drv_harrier_vlan_vt_get(int unit, uint32 prop_type, uint32 vid, 
                                uint32 port, uint32 *prop_val)
{
    uint32  reg_value;
    uint32  field_val32 = 0, t_vid;
    uint32  tmp;
  
    /* check if V2V is reset already */
    if (!v2v_reset_done) {
         if (_drv_v2v_reset(unit, V2V_SW_RESET)){
            LOG_INFO(BSL_LS_SOC_VLAN,
                     (BSL_META_U(unit,
                                 "%s,can't reset V2V table!\n"), FUNCTION_NAME()));
        }
    }
    
    if (IS_PORT_BASED_VT){
        if (vid >= V2V_ENT_PORT_COUNT) {
            /* for per port per vlan translation, 
             * max supported vid(as VLAN2VLAN table's index) is 127.
             */
            return SOC_E_PARAM;
        }
        t_vid = 0;
        t_vid = (port << 7) + vid;
    } else {
        t_vid = vid;
    }

    if (V2V_ENT_VALID_CHK(t_vid) != TRUE){
        return SOC_E_NOT_FOUND;
    } else {
        switch(prop_type){
        case DRV_VLAN_PROP_VT_MODE:      /*  trasparent / mapping */
            *prop_val = V2V_ENT_MODE_GET(t_vid);
            break;
        case DRV_VLAN_PROP_ING_VT_SPVID:     /* ingress SP VID */
            if (IS_PORT_BASED_VT){
                tmp = V2V_ENT_MODE_GET(t_vid);
                *prop_val = tmp % V2V_ENT_PORT_COUNT;
            } else {
                *prop_val = V2V_ENT_VID_GET(t_vid);
            }
            break;
        case DRV_VLAN_PROP_EGR_VT_SPVID:     /* egress SP VID */
            break;
        case DRV_VLAN_PROP_ING_VT_PRI:
            break;
        case DRV_VLAN_PROP_EGR_VT_PRI:
            break;
        case DRV_VLAN_PROP_ING_VT_SPTPID:    /* ingress SP TPID */
            SOC_IF_ERROR_RETURN(
                REG_READ_ISP_VIDr(unit, &reg_value));
            
            soc_ISP_VIDr_field_get(unit, &reg_value,
                ISP_VLAN_DELIMITERf, &field_val32);
            *prop_val = field_val32;
            
            break;
        case DRV_VLAN_PROP_EGR_VT_SPTPID:    /* egress SP TPID */
            break;
        default :
            break;
        }
    }

    return SOC_E_NONE;
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
drv_harrier_port_vlan_set(int unit, uint32 port, soc_pbmp_t bmp)
{
    uint32	temp = 0;
    uint64  reg_value64;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: port=%d, bmp=0x%x\n"),
              FUNCTION_NAME(), port, SOC_PBMP_WORD_GET(bmp, 0)));
    SOC_IF_ERROR_RETURN(
        REG_READ_PORT_EGCTLr(unit, port, (uint32 *)&reg_value64));
    temp = SOC_PBMP_WORD_GET(bmp, 0);
    soc_PORT_EGCTLr_field_set(unit, (uint32 *)&reg_value64, 
        PORT_EGRESS_ENf, &temp);
    SOC_IF_ERROR_RETURN(
        REG_WRITE_PORT_EGCTLr(unit, port, (uint32 *)&reg_value64));

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
drv_harrier_port_vlan_get(int unit, uint32 port, soc_pbmp_t *bmp)
{
    uint32	temp = 0;
    uint64  reg_value64;

    SOC_IF_ERROR_RETURN(
        REG_READ_PORT_EGCTLr(unit, port, (uint32 *)&reg_value64));
    soc_PORT_EGCTLr_field_get(unit, (uint32 *)&reg_value64, 
        PORT_EGRESS_ENf, &temp);
    SOC_PBMP_WORD_SET(*bmp, 0, temp);
    
    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: port=%d, bmp=0x%x\n"),
              FUNCTION_NAME(), port, SOC_PBMP_WORD_GET(*bmp, 0)));
    return SOC_E_NONE;
}

/*
 *  Function : drv_vlan_prop_set
 *
 *  Purpose :
 *      Set the VLAN property value.
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
 *
 */
int 
drv_harrier_vlan_prop_set(int unit, uint32 prop_type, uint32 prop_val)
{
    uint32  reg_value, temp;
    int rv = SOC_E_NONE;
    uint16  dev_id;
    uint8   rev_id;

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
        break;            
    case DRV_VLAN_PROP_SP_TAG_TPID:
        if ((rv = REG_READ_ISP_VIDr(unit, &reg_value)) < 0) {
            return rv;
        }

        temp = prop_val & 0xFFFF;
        soc_ISP_VIDr_field_set(unit, &reg_value,
            ISP_VLAN_DELIMITERf, &temp);

        if ((rv = REG_WRITE_ISP_VIDr(unit, &reg_value)) < 0) {
            return rv;
        }
            
        break;
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        if ((rv = REG_READ_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
            return rv;
        }
        if (prop_val) {
           temp = 0;
        } else {
           temp = 1;
        }
        soc_DTAG_GLO_CTLr_field_set(unit, &reg_value, 
            EN_DTAG_ISPf, &temp);
        
        if ((rv = REG_WRITE_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
            return rv;
        }
        break;

    /* set system basis VLAN translation enable status */
    case DRV_VLAN_PROP_V2V:
        prop_val = (prop_val) ? 1 : 0;
        
        /* handle the V2V table reset process 
         *  - proceed the reset only if V2V is not reset yet.
         */
        if (prop_val){
            if (!v2v_reset_done) {
                rv = _drv_v2v_reset(unit, V2V_SW_RESET);
                if (rv){
                    LOG_INFO(BSL_LS_SOC_VLAN,
                             (BSL_META_U(unit,
                                         "%s,can't reset V2V table!\n"), FUNCTION_NAME()));
                }
            } 
        }
            
        /* Proceed if enable status changed 
         *  - case 1 >> disable/enable changed
         */
        if (igr_vt_db.vt_en != prop_val){
            soc_pbmp_t  set_bmp;

            /* enable/disable VT on all port */
            SOC_PBMP_CLEAR(set_bmp);
            SOC_PBMP_ASSIGN(set_bmp, PBMP_ALL(unit));
            SOC_PBMP_REMOVE(set_bmp, PBMP_CMIC(unit));
            
            SOC_IF_ERROR_RETURN(DRV_VLAN_PROP_PORT_ENABLE_SET(unit, 
                    DRV_VLAN_PROP_V2V_PORT, set_bmp, prop_val));
                        
            /* update SW-info */
            igr_vt_db.vt_en = prop_val;
        }
         
        break;

    /* Special V2V talbe init routine for performance issue  
     *  - this routine is assummed to be called in bcm/VLAN init routine.
     *
     *  Reset the control flag only. The real V2V table reset will be done 
     *  once VT feature be enabled or VT entry been added.
     */ 
    case DRV_VLAN_PROP_V2V_INIT:
        v2v_reset_done = 0;     /* reset the control flag */
        port_vt = SYSTEM_BASED_VT;  /* reset to system based VT */
        
        break;

    case DRV_VLAN_PROP_PER_PORT_TRANSLATION:
        soc_cm_get_id(unit, &dev_id, &rev_id);
        if ((rev_id == BCM53242_A0_REV_ID) || (rev_id == BCM53262_A0_REV_ID)){
            /* The feature is only available on chips after B0 version. */
            return SOC_E_UNAVAIL;
        }

        prop_val = (prop_val) ? 1 : 0;
        /* for system/port basis V2V mode change */
        prop_val = (prop_val) ? PORT_BASED_VT : SYSTEM_BASED_VT;
        if (port_vt != prop_val){   /* mode changed */
            /* 1. set to port based VT */
            if ((rv = REG_READ_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            if (prop_val == PORT_BASED_VT) {
               temp = 1;
            } else {
               temp = 0;
            }
            soc_VLAN_CTRL4r_field_set(unit, &reg_value, 
                EN_V2V_INDEX_BY_INPORTf, &temp);
            
            if ((rv = REG_WRITE_VLAN_CTRL4r(unit, &reg_value)) < 0) {
                return rv;
            }
            
            /* 2. update SW-info */
            port_vt = prop_val;
    
            /* 3. reset the V2V table */
            if (_drv_v2v_reset(unit, V2V_SW_RESET)){
                LOG_INFO(BSL_LS_SOC_VLAN,
                         (BSL_META_U(unit,
                                     "%s,can't reset V2V table!\n"), 
                          FUNCTION_NAME()));
            }
        }

        break;
    default:
        rv = SOC_E_UNAVAIL;
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
 *
 */
int 
drv_harrier_vlan_prop_get(int unit, uint32 prop_type, uint32 *prop_val)
{
    uint32  reg_value, temp = 0;
    int rv = SOC_E_NONE;

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
        break;            
    case DRV_VLAN_PROP_SP_TAG_TPID:
        if ((rv = REG_READ_ISP_VIDr(unit, &reg_value)) < 0) {
            return rv;
        }

        soc_ISP_VIDr_field_get(unit, &reg_value,
            ISP_VLAN_DELIMITERf, &temp);

        *prop_val = temp & 0xFFFF;
        
        break;
    case DRV_VLAN_PROP_DOUBLE_TAG_MODE:
        if ((rv = REG_READ_DTAG_GLO_CTLr(unit, &reg_value)) < 0) {
            return rv;
        }
        soc_DTAG_GLO_CTLr_field_get(unit, &reg_value, 
            EN_DTAG_ISPf, &temp);
    
        if (temp) {
            *prop_val = TRUE;
        } else {
            *prop_val = FALSE;
        }
        
        break;

    /* get system basis VLAN translation enable status */
    case DRV_VLAN_PROP_V2V:
    
        if (v2v_reset_done) { 
            *prop_val = (igr_vt_db.vt_en) ? TRUE : FALSE;
        } else {
            LOG_INFO(BSL_LS_SOC_VLAN,
                     (BSL_META_U(unit,
                                 "%s, V2V table not reset yet!\n"), FUNCTION_NAME()));
            *prop_val = FALSE;
        }
        break;

    case DRV_VLAN_PROP_PER_PORT_TRANSLATION:
    
        if (v2v_reset_done) { 
            *prop_val = (IS_PORT_BASED_VT) ? TRUE : FALSE;
        } else {
            LOG_INFO(BSL_LS_SOC_VLAN,
                     (BSL_META_U(unit,
                                 "%s, V2V table not reset yet!\n"), FUNCTION_NAME()));
            *prop_val = FALSE;
        }
        break;
    default:
        rv = SOC_E_UNAVAIL;
        break;
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
 */
int 
drv_harrier_vlan_prop_port_enable_set(int unit, uint32 prop_type, 
                soc_pbmp_t bmp, uint32 val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_index = 0, fld_index = 0;    
    uint64  reg_value64;
    uint32 temp;
    int rv = SOC_E_NONE;
    soc_pbmp_t set_bmp, temp_bmp;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop=%d, value=0x%x\n"), 
              FUNCTION_NAME(), unit, prop_type, val));

    if (SOC_PBMP_IS_NULL(bmp)){ /* no port to set */
        return SOC_E_NONE;
    }
    
    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        reg_index = INDEX(ISP_SEL_PORTMAPr);
        fld_index  = INDEX(ISP_PORT_PBMPf);
        break;
    case    DRV_VLAN_PROP_V2V_PORT :

        /* check if V2V is reset already */
        if (val){
            if (!v2v_reset_done) {
                rv = _drv_v2v_reset(unit, V2V_SW_RESET);
                if (rv){
                    LOG_INFO(BSL_LS_SOC_VLAN,
                             (BSL_META_U(unit,
                                         "%s,can't reset V2V table!\n"), FUNCTION_NAME()));
                }
            }
        }
        
        reg_index = INDEX(VLAN2VLAN_CTLr);
        fld_index  = INDEX(VLAN2VLAN_CTLf);
        break;
    case    DRV_VLAN_PROP_MAC2V_PORT :
        reg_index = INDEX(MAC2VLAN_CTLr);
        fld_index  = INDEX(MAC2VLAN_CTLf);
        break;
    case    DRV_VLAN_PROP_PROTOCOL2V_PORT :
        reg_index = INDEX(PROTOCOL2VLAN_CTLr);
        fld_index  = INDEX(PROTOCOL2VLAN_CTLf);
        break;
    case    DRV_VLAN_PROP_TRUST_VLAN_PORT :
        reg_index = INDEX(TRUST_CVLANr);
        fld_index  = INDEX(TRUST_CVLANf);
        break;
    default :
        return SOC_E_UNAVAIL;
        break;

    }
    reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    SOC_IF_ERROR_RETURN(DRV_REG_READ(unit, 
            reg_addr, (uint32 *)&reg_value64, reg_len));
    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET(unit, 
            reg_index,(uint32 *)&reg_value64, fld_index, &temp));

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
    if (SOC_IS_ROBO53242(unit) && (prop_type == DRV_VLAN_PROP_TRUST_VLAN_PORT)) {
        /* 
         * The default value of register TRUST_CVLAN of BCM53242 and BCM53262
         * are both 0x1fffffff. For BCM53242, the value includes non-exist ports 
         * ge2 and ge3.
         * A validation is needed when program this register.
         */
        SOC_PBMP_AND(set_bmp, PBMP_ALL(unit));    
    } else {
        if (SOC_PBMP_EQ(temp_bmp, set_bmp)){
            /* do nothing */
            return SOC_E_NONE;
        }
    }

    /* write to register */
    temp = SOC_PBMP_WORD_GET(set_bmp, 0);    

    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_SET(unit, 
            reg_index, (uint32 *)&reg_value64, fld_index, &temp));
    SOC_IF_ERROR_RETURN(DRV_REG_WRITE(unit, 
            reg_addr, (uint32 *)&reg_value64, reg_len));

    if (prop_type == DRV_VLAN_PROP_V2V_PORT) {
        /* SW info update for VT feature */
        if (temp == 0){    /* means no port service the VT feature */ 
            /* update SW-Info if VT is changing from enable to disable */
            if (igr_vt_db.vt_en) {    /* check if current VT is enabled */
                igr_vt_db.vt_en = FALSE;
            }
        } else {
            /* update SW-Info if VT is changed from disable to enable */
            if (!igr_vt_db.vt_en) {    /* check if current VT is disabled */
                /* in this case, set to system basis enable always!
                 *  - if current vt is at port based VT mode, no action on 
                 *    igr_vt_db.vt_en.
                 */
                igr_vt_db.vt_en = TRUE;
            }
        }
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
drv_harrier_vlan_prop_port_enable_get(int unit, uint32 prop_type, 
                uint32 port_n, uint32 *val)
{
    uint32 reg_addr, reg_len;
    uint32 reg_index = 0, fld_index = 0;    
    uint64  reg_value64;
    uint32 temp;
    soc_pbmp_t pbmp;
    int rv = SOC_E_NONE;

    LOG_INFO(BSL_LS_SOC_VLAN,
             (BSL_META_U(unit,
                         "%s: unit=%d, prop=%d, port=%d\n"), 
              FUNCTION_NAME(), unit, prop_type, port_n));

    switch(prop_type){
    case    DRV_VLAN_PROP_ISP_PORT :
        reg_index = INDEX(ISP_SEL_PORTMAPr);
        fld_index  = INDEX(ISP_PORT_PBMPf);
        break;
    case    DRV_VLAN_PROP_V2V_PORT :
        if (!v2v_reset_done) { 
            *val = FALSE;
        }

        reg_index = INDEX(VLAN2VLAN_CTLr);
        fld_index  = INDEX(VLAN2VLAN_CTLf);
        break;
    case    DRV_VLAN_PROP_MAC2V_PORT :
        reg_index = INDEX(MAC2VLAN_CTLr);
        fld_index  = INDEX(MAC2VLAN_CTLf);
        break;
    case    DRV_VLAN_PROP_PROTOCOL2V_PORT :
        reg_index = INDEX(PROTOCOL2VLAN_CTLr);
        fld_index  = INDEX(PROTOCOL2VLAN_CTLf);
        break;
    case    DRV_VLAN_PROP_TRUST_VLAN_PORT :
        reg_index = INDEX(TRUST_CVLANr);
        fld_index  = INDEX(TRUST_CVLANf);
        break;
    default :
        return SOC_E_UNAVAIL;

    }
    reg_addr = DRV_REG_ADDR(unit, reg_index, 0, 0);
    reg_len = DRV_REG_LENGTH_GET(unit, reg_index);
    SOC_IF_ERROR_RETURN(DRV_REG_READ(unit, 
            reg_addr, (uint32 *)&reg_value64,reg_len));
    SOC_IF_ERROR_RETURN(DRV_REG_FIELD_GET(unit, 
            reg_index,(uint32 *)&reg_value64, fld_index, &temp));

    /* check if the value get is port basis or device basis. */
    SOC_PBMP_CLEAR(pbmp);
    SOC_PBMP_WORD_SET(pbmp, 0, temp);

    if (SOC_IS_ROBO53242(unit) && (prop_type == DRV_VLAN_PROP_TRUST_VLAN_PORT)) {
        /* 
         * The default value of register TRUST_CVLAN of BCM53242 and BCM53262
         * are both 0x1fffffff. For BCM53242, the value includes non-exist ports 
         * ge2 and ge3.
         * A validation is needed when program this register.
         */
        SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
    }
    
    if (port_n == 0xffffffff) {     /* device basis */
        int     i;

        for (i = 0; i < SOC_PBMP_WORD_MAX; i++){
            *(val + i) = SOC_PBMP_WORD_GET(pbmp, i);
        }
    } else {
        *val = (SOC_PBMP_MEMBER(pbmp, port_n)) ? TRUE : FALSE;
    }

    return rv;

}

