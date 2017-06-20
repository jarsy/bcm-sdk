/*
 * $Id: TkOsAlDataInOut.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     TkOsAlDataInOut.c
 * Purpose: 
 *
 */

#include <soc/ea/tk371x/TkTypes.h>
#include <soc/drv.h>


/*
 *  Function : TkOsDataTx
 *
 *  Purpose :
 *
 *  Parameters :
 *
 *  Return :
 *
 *  Note :
 *      Set the limit and burst size to bucket 0(storm control use bucket1). 
 *
 */
Bool
TkOsDataTx(uint8 pathId, uint8 * buf, uint16 len)
{
    soc_control_t  *soc;
    soc_ea_oam_ctrl_if_t *ctrl_if;
    int             index;
    char            src_mac[6] = { 0x00, 0x11, 0x22, 0x33, 0x44, 0x55 };

    sal_memcpy(&(buf[6]), src_mac, 6);

    for (index = 12; index < len; index++) {
        buf[index] = buf[index + 4];
    }

    if (SOC_IS_EA(pathId)) {
        soc = SOC_CONTROL(pathId);
    }else{
        return FALSE;
    }
    
    if (soc->soc_flags & SOC_F_ATTACHED) {
        ctrl_if = soc->oam_ctrlops;
        ctrl_if->request(pathId, buf, len+4);
    }
    
    return TRUE;
}

void
TkOsGetIfMac(uint8 * mac)
{
    mac[0] = 0x00;
    mac[1] = 0x0d;
    mac[2] = 0xb6;
    mac[3] = 0xff;
    mac[4] = 0xff;
    mac[5] = 0xff;
	return ;
}
