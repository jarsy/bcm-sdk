/*
 * $Id: mstp.c,v 1.1 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/debug.h>

/*
 *  Function : drv_harrier_mstp_port_set
 *
 *  Purpose :
 *      Set the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid    :   multiple spanning tree id.
 *      port        :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */
int 
drv_harrier_mstp_port_set(int unit, uint32 mstp_gid, uint32 port, uint32 port_state)
{
    uint32  shift, wd = 0;
    mspt_tab_entry_t  mstp_entry;
    uint32  reg_value;
    uint32  max_gid;
    uint64  data64, temp;
    uint32  *entry;

    LOG_INFO(BSL_LS_SOC_STP,
             (BSL_META_U(unit,
                         "drv_mstp_port_set : unit %d, STP id = %d, port = %d, port_state = %d \n"),
              unit, mstp_gid, port, port_state));

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid));

    if (!soc_feature(unit, soc_feature_mstp)){
        /* error checking */
        if (mstp_gid != STG_ID_DEFAULT) {
            return SOC_E_PARAM;
        }

        switch (port_state)
        {
            case DRV_PORTST_DISABLE:
                COMPILER_64_SET(temp, 0x0, 0x1);
                break;
            case DRV_PORTST_BLOCK:
                COMPILER_64_SET(temp, 0x0, 0x2);
                break;
            case DRV_PORTST_LISTEN:
                COMPILER_64_SET(temp, 0x0, 0x3);
                break;
            case DRV_PORTST_LEARN:
                COMPILER_64_SET(temp, 0x0, 0x4);
                break;
            case DRV_PORTST_FORWARD:
                COMPILER_64_SET(temp, 0x0, 0x5);
                break;
            default:
                return SOC_E_PARAM;
        }

        if(IS_GE_PORT(unit, port)) {
            SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr
                (unit, port, &reg_value));
            SOC_IF_ERROR_RETURN(soc_G_PCTLr_field_set
                (unit, &reg_value, G_MISTP_STATEf, (uint32 *)&temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_G_PCTLr
                (unit, port, &reg_value));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_TH_PCTLr
                (unit, port, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TH_PCTLr_field_set
                (unit, &reg_value, MISTP_STATEf, (uint32 *)&temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_TH_PCTLr
                (unit, port, &reg_value));
        }
    } else {
        /* error checking */
        if (mstp_gid > max_gid) {
            return SOC_E_PARAM;
        }

        mstp_gid = mstp_gid % max_gid;
        sal_memset(&mstp_entry, 0, sizeof(mstp_entry));

        /* write mstp id to vlan entry */
        SOC_IF_ERROR_RETURN(DRV_MEM_READ
            (unit, DRV_MEM_MSTP, mstp_gid, 1, (uint32 *)&mstp_entry));

        /* 
         * Because the memory field services didn't contain port information, 
         * we can't access the port state by port
         */
        if (port < 16) {
            shift = 3 * port + 76;  /* skip length of RESERVED bit */
        } else {                    /* TABLE_DATA0[63:0], TABLE_DATA1[11:0] */
            shift = 3 * port + 80;  /* skip length of RESERVED bit TABLE_DATA0[63:0], */
        }                           /* TABLE_DATA1[11:0], TABLE_DATA1[63:60] */

        wd = (shift / 64);
        shift -= (shift / 64) * 64;
        
        entry = (uint32 *)&mstp_entry;
        COMPILER_64_SET(data64, entry[2*wd+1], entry[2*wd]);
        
        COMPILER_64_SET(temp, 0x0, 0x7);
        COMPILER_64_SHL(temp, shift);
        COMPILER_64_NOT(temp);

        COMPILER_64_AND(data64, temp);
        switch (port_state)
        {
            case DRV_PORTST_DISABLE:
                COMPILER_64_SET(temp, 0x0, 0x1);
                break;
            case DRV_PORTST_BLOCK:
                COMPILER_64_SET(temp, 0x0, 0x2);
                break;
            case DRV_PORTST_LISTEN:
                COMPILER_64_SET(temp, 0x0, 0x3);
                break;
            case DRV_PORTST_LEARN:
                COMPILER_64_SET(temp, 0x0, 0x4);
                break;
            case DRV_PORTST_FORWARD:
                COMPILER_64_SET(temp, 0x0, 0x5);
                break;
            default:
                return SOC_E_PARAM;
        }
        COMPILER_64_SHL(temp, shift);
        COMPILER_64_OR(data64, temp);
        
        entry[2*wd] = COMPILER_64_LO(data64);
        entry[2*wd+1] = COMPILER_64_HI(data64);

        SOC_IF_ERROR_RETURN(DRV_MEM_WRITE
            (unit, DRV_MEM_MSTP, mstp_gid, 1, (uint32 *)&mstp_entry));
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_harrier_mstp_port_get
 *
 *  Purpose :
 *      Get the port state of a selected stp id.
 *
 *  Parameters :
 *      unit        :   unit id
 *      mstp_gid    :   multiple spanning tree id.
 *      port        :   port number.
 *      port_state  :   state of the port.
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *      
 *
 */

int 
drv_harrier_mstp_port_get(int unit, uint32 mstp_gid, uint32 port, uint32 *port_state)
{
    uint32  portstate;
    mspt_tab_entry_t  mstp_entry;
    uint32  reg_value;
    uint32  max_gid;
    uint32  shift, wd = 0;
    uint64  data64, temp;
    uint32  *entry;

    SOC_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_MSTP_NUM, &max_gid));

    if (!soc_feature(unit, soc_feature_mstp)){

        /* error checking */
        if (mstp_gid != STG_ID_DEFAULT) {
            return SOC_E_PARAM;
        }

        if(IS_GE_PORT(unit, port) ){
            SOC_IF_ERROR_RETURN(REG_READ_G_PCTLr
                (unit, port, &reg_value));
            SOC_IF_ERROR_RETURN(soc_G_PCTLr_field_get
                (unit, &reg_value, G_MISTP_STATEf, &portstate));
        } else {
            SOC_IF_ERROR_RETURN(REG_READ_TH_PCTLr
                (unit, port, &reg_value));
            SOC_IF_ERROR_RETURN(soc_TH_PCTLr_field_get
                (unit, &reg_value, MISTP_STATEf, &portstate));
        }
    
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
                break;
            case 4:
                *port_state = DRV_PORTST_LEARN;
                break;
            case 5:
                *port_state = DRV_PORTST_FORWARD;
                break;
            default:
            return SOC_E_INTERNAL;
        }

        LOG_INFO(BSL_LS_SOC_STP,
                 (BSL_META_U(unit,
                             "drv_mstp_port_get : unit %d, STP id = %d, port = %d, port_state = %d \n"),
                  unit, mstp_gid, port, *port_state));

    }else{
        /* error checking */
        if (mstp_gid > max_gid) {
            return SOC_E_PARAM;
        }
        mstp_gid = mstp_gid % max_gid;

        /* Read mstp id to vlan entry */
        SOC_IF_ERROR_RETURN(DRV_MEM_READ
            (unit, DRV_MEM_MSTP, mstp_gid, 1, (uint32 *)&mstp_entry));

        /* 
         * Because the memory field services didn't contain port information, 
         * we can't access the port state by port
         */
        if (port < 16) {
            shift = 3 * port + 76;  /* skip length of RESERVED bit */
        } else {                    /* TABLE_DATA0[63:0], TABLE_DATA1[11:0] */
            shift = 3 * port + 80;  /* skip length of RESERVED bit TABLE_DATA0[63:0], */
        }                           /* TABLE_DATA1[11:0], TABLE_DATA1[63:60] */
        
        entry = (uint32 *)&mstp_entry;

        wd = (shift / 64);
        shift -= (shift / 64) * 64;

        COMPILER_64_SET(data64,entry[2*wd+1],entry[2*wd]);
        
        COMPILER_64_SET(temp, 0x0, 0x7);
        COMPILER_64_SHR(data64, shift);
        COMPILER_64_AND(data64, temp);
        COMPILER_64_TO_32_LO(portstate, data64);
        switch (portstate)
        {
            case 1:
                *port_state = DRV_PORTST_DISABLE;
                break;
            case 2:
                *port_state = DRV_PORTST_BLOCK;
                break;
            case 3:
                *port_state = DRV_PORTST_LISTEN;
                break;
            case 4:
                *port_state = DRV_PORTST_LEARN;
                break;
            case 5:
                *port_state = DRV_PORTST_FORWARD;
                break;
            default:
                return SOC_E_INTERNAL;
        }
        LOG_INFO(BSL_LS_SOC_STP,
                 (BSL_META_U(unit,
                             "drv_mstp_port_get : unit %d, STP id = %d, port = %d, port_state = %d \n"),
                  unit, mstp_gid, port, *port_state));
    }

    return SOC_E_NONE;
}

