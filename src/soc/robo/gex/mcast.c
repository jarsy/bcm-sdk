/*
 * $Id: mcast.c,v 1.2 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#include <shared/bsl.h>

#include <soc/robo.h>
#include <soc/drv.h>
#include <soc/types.h>
#include <soc/debug.h>
#include <soc/mcast.h>

/*
 *  Function : drv_gex_mcast_bmp_get
 *
 *  Purpose :
 *      Get the multicast member ports from multicast entry
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer 
 *      bmp     :   group port member
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int  
drv_gex_mcast_bmp_get(int unit, uint32 *entry, soc_pbmp_t *bmp)
{
    uint32	pbmp;
    
    assert(entry);
    assert(bmp);

    /* get the multicast id */
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_GET
        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &pbmp));
    
    SOC_PBMP_WORD_SET(*bmp, 0, pbmp);

    LOG_INFO(BSL_LS_SOC_L2TABLE,
             (BSL_META_U(unit,
                         "drv_mcast_bmp_get: unit %d, bmp = %x\n"),
              unit, SOC_PBMP_WORD_GET(*bmp, 0)));

    return SOC_E_NONE;
}

 /*
 *  Function : drv_gex_mcast_bmp_set
 *
 *  Purpose :
 *      Set the multicast member ports from multicast entry
 *
 *  Parameters :
 *      unit    :   unit id
 *      entry   :   entry data pointer 
 *      bmp     :   group port member
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *
 */
int  
drv_gex_mcast_bmp_set(int unit, uint32 *entry, 
                soc_pbmp_t bmp, uint32 flag)
{
    uint32  temp;
    uint32  reg_value;
    uint32  fld_value = 0;
    
    assert(entry);
    LOG_INFO(BSL_LS_SOC_L2TABLE,
             (BSL_META_U(unit,
                         "drv_mcast_bmp_set: unit %d, bmp = %x flag %x\n"),
              unit, SOC_PBMP_WORD_GET(bmp, 0), flag));

    /* Skip to set the field "IP_MC" for BCM53101 */
    if (SOC_IS_VULCAN(unit) || SOC_IS_BLACKBIRD(unit)){
        /* ensure the IP_Multicast scheme is enabled. */
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CTRLr
            (unit, &reg_value));
        temp = 1;
        soc_NEW_CTRLr_field_set(unit, &reg_value, 
            IP_MCf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_NEW_CTRLr(unit, &reg_value));
    } else if (SOC_IS_DINO(unit)) {
        /* ensure the IP_Multicast scheme is enabled. */
        SOC_IF_ERROR_RETURN(REG_READ_NEW_CONTROLr
            (unit, &reg_value));
        temp = 1;
        soc_NEW_CONTROLr_field_set(unit, &reg_value, 
            IP_MULTICASTf, &temp);
        SOC_IF_ERROR_RETURN(
            REG_WRITE_NEW_CONTROLr(unit, &reg_value));
    }
    
    fld_value= SOC_PBMP_WORD_GET(bmp, 0);
    SOC_IF_ERROR_RETURN(DRV_MEM_FIELD_SET
        (unit, DRV_MEM_MARL, DRV_MEM_FIELD_DEST_BITMAP, entry, &fld_value));

    /* Insert this address into arl table. */
    SOC_IF_ERROR_RETURN(DRV_MEM_INSERT
        (unit, DRV_MEM_ARL, entry, 
        (DRV_MEM_OP_BY_HASH_BY_MAC | DRV_MEM_OP_BY_HASH_BY_VLANID | 
        DRV_MEM_OP_REPLACE)));

    return SOC_E_NONE;
}

