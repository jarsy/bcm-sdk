/*
 * $Id: dos.c,v 1.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC driver for DOS attach filter
 */

#include <soc/types.h>
#include <soc/error.h>
#include <soc/debug.h>
#include <soc/drv_if.h>
#include <soc/drv.h>

#define _TB_DRV_DOS_TCP_HDR_SIZE_MIN (0)
#define _TB_DRV_DOS_TCP_HDR_SIZE_MAX (255)
#define _TB_DRV_DOS_ICMP_SIZE_MIN (0)
#define _TB_A0_DRV_DOS_ICMP_SIZE_MAX (2048)
#define _TB_DRV_DOS_ICMP_SIZE_MAX (9728)

int 
drv_tbx_dos_enable_set(int unit, uint32 type, uint32 param)
{
    uint32 reg_value = 0, temp = 0;
    int min = 0;
    int max = 0;

    /* none DOS filter registers related */
    switch (type) {
        case DRV_DOS_MIN_TCP_HDR_SZ:
            /* register's value range : 0 - 255 */
            min = _TB_DRV_DOS_TCP_HDR_SIZE_MIN;
            max = _TB_DRV_DOS_TCP_HDR_SIZE_MAX;
            if (((int)param < min) || ((int)param > max)) {
                return SOC_E_PARAM;
            }
            temp = param;
            SOC_IF_ERROR_RETURN(REG_READ_MIN_TCP_HEADER_SIZEr(
                    unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_MIN_TCP_HEADER_SIZEr_field_set(
                    unit, &reg_value, MIN_TCP_HDR_SIZEf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_MIN_TCP_HEADER_SIZEr(
                    unit, &reg_value));
            return SOC_E_NONE;
            break;
        case DRV_DOS_MAX_ICMPV6_SIZE:
            /* Set MAX_ICMPv6_Size, value range: 0 - 2048(A0)/9728(B0) in bytes */
            min = _TB_DRV_DOS_ICMP_SIZE_MIN;
            if (SOC_IS_TB_AX(unit)) {
                max = _TB_A0_DRV_DOS_ICMP_SIZE_MAX;
            } else {
                max = _TB_DRV_DOS_ICMP_SIZE_MAX;
            }
            if (((int)param < min) || ((int)param > max)) {
                return SOC_E_PARAM;
            }
            temp = param;
            SOC_IF_ERROR_RETURN(REG_READ_MAX_ICMPV6_SIZEr(
                    unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_MAX_ICMPV6_SIZEr_field_set(
                    unit, &reg_value, MAX_ICMPV6_SIZEf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_MAX_ICMPV6_SIZEr(
                    unit, &reg_value));
            return SOC_E_NONE;
            break;
        case DRV_DOS_MAX_ICMPV4_SIZE:
            /* Set MAX_ICMPv4_Size, value range: 0 - 2048(A0)/9728(B0) in bytes */
            min = _TB_DRV_DOS_ICMP_SIZE_MIN;
            if (SOC_IS_TB_AX(unit)) {
                max = _TB_A0_DRV_DOS_ICMP_SIZE_MAX;
            } else {
                max = _TB_DRV_DOS_ICMP_SIZE_MAX;
            }
            if (((int)param < min) || ((int)param > max)) {
                return SOC_E_PARAM;
            }
            temp = param;
            SOC_IF_ERROR_RETURN(REG_READ_MAX_ICMPV4_SIZEr(
                    unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_MAX_ICMPV4_SIZEr_field_set(
                    unit, &reg_value, MAX_ICMPV4_SIZEf, &temp));
            SOC_IF_ERROR_RETURN(REG_WRITE_MAX_ICMPV4_SIZEr(
                    unit, &reg_value));
            return SOC_E_NONE;
            break;
        default : 
            break;
    }
    
    /* dos attack filter register realted */
    SOC_IF_ERROR_RETURN(REG_READ_DOS_ATTACK_FILTER_DROP_CTLr(
            unit, &reg_value));
    switch (type) {
        case DRV_DOS_NULL_WITH_TCP_SEQ_ZERO:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_NULL_SCAN_DROPf, &temp));
            break;
        case DRV_DOS_ICMPV4_LONG_PING:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_ICMPV4_LONGPING_DROPf, &temp));
            break;
        case DRV_DOS_ICMPV6_LONG_PING:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_ICMPV6_LONGPING_DROPf, &temp));
            break;
        case DRV_DOS_ICMPV4_FRAGMENTS:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_ICMPV4_FRAG_DROPf, &temp));
            break;
        case DRV_DOS_ICMPV6_FRAGMENTS:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_ICMPV6_FRAG_DROPf, &temp));
            break;
        case DRV_DOS_TCP_FRAG_OFFSET:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_FRAG_ERR_DROPf, &temp));
            break;
        case DRV_DOS_TCP_FRAG:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_FRAG_ERR_DROPf, &temp));
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_SHORT_HDR_DROPf, &temp));
            break;
        case DRV_DOS_TCP_SHORT_HDR:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_SHORT_HDR_DROPf, &temp));
            break;
        case DRV_DOS_BLAT:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_BLAT_DROPf, &temp));
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_UDP_BLAT_DROPf, &temp));
            break;
        case DRV_DOS_TCP_BLAT:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_BLAT_DROPf, &temp));
            break;
        case DRV_DOS_UDP_BLAT:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_UDP_BLAT_DROPf, &temp));
            break;
        case DRV_DOS_LAND:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_IP_LAND_DROPf, &temp));
            break;
        case DRV_DOS_SYN_WITH_SP_LT1024:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_SYN_ERR_DROPf, &temp));
            break;
        case DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_XMAS_SCAN_DROPf, &temp));
            break;
        case DRV_DOS_SYN_FIN_SCAN:
            temp = (param) ? 1 : 0;
            SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_set(
                    unit, &reg_value, EN_TCP_SINFIN_SCAN_DROPf, &temp));
            break;
        default : 
            return SOC_E_UNAVAIL;
            break;
    }
    SOC_IF_ERROR_RETURN(REG_WRITE_DOS_ATTACK_FILTER_DROP_CTLr(
            unit, &reg_value));
    return SOC_E_NONE;

}

int 
drv_tbx_dos_enable_get(int unit, uint32 type, uint32 *param)
{
    uint32 reg_value = 0, temp = 0;

    /* none DOS fileter registers related */
    switch (type) {
        case DRV_DOS_MIN_TCP_HDR_SZ:
            /* register's value range : 0 - 255 */
            SOC_IF_ERROR_RETURN(REG_READ_MIN_TCP_HEADER_SIZEr(
                    unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_MIN_TCP_HEADER_SIZEr_field_get(
                    unit, &reg_value, MIN_TCP_HDR_SIZEf, &temp));
            *param = temp;
            return SOC_E_NONE;
            break;
        case DRV_DOS_MAX_ICMPV6_SIZE:
            SOC_IF_ERROR_RETURN(REG_READ_MAX_ICMPV6_SIZEr(
                    unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_MAX_ICMPV6_SIZEr_field_get(
                    unit, &reg_value, MAX_ICMPV6_SIZEf, &temp));
            *param = temp;
            return SOC_E_NONE;
            break;
        case DRV_DOS_MAX_ICMPV4_SIZE:
            SOC_IF_ERROR_RETURN(REG_READ_MAX_ICMPV4_SIZEr(
                    unit, &reg_value));
            SOC_IF_ERROR_RETURN(soc_MAX_ICMPV4_SIZEr_field_get(
                    unit, &reg_value, MAX_ICMPV4_SIZEf, &temp));
            *param = temp;
            return SOC_E_NONE;
            break;
        default : 
            break;
    }


    /* dos attack filter register related */
    SOC_IF_ERROR_RETURN(REG_READ_DOS_ATTACK_FILTER_DROP_CTLr(
            unit, &reg_value));
    switch (type) {
        case DRV_DOS_NULL_WITH_TCP_SEQ_ZERO:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_NULL_SCAN_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_ICMPV4_LONG_PING:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_ICMPV4_LONGPING_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_ICMPV6_LONG_PING:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_ICMPV6_LONGPING_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_ICMPV4_FRAGMENTS:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_ICMPV4_FRAG_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_ICMPV6_FRAGMENTS:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_ICMPV6_FRAG_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_TCP_FRAG:
        case DRV_DOS_TCP_FRAG_OFFSET:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_FRAG_ERR_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_TCP_SHORT_HDR:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_SHORT_HDR_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_BLAT:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_BLAT_DROPf, &temp));
            *param = temp;
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_UDP_BLAT_DROPf, &temp));
            *param |= temp;
            break;
        case DRV_DOS_TCP_BLAT:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_BLAT_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_UDP_BLAT:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_UDP_BLAT_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_LAND:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_IP_LAND_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_SYN_WITH_SP_LT1024:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_SYN_ERR_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_XMASS_WITH_TCP_SEQ_ZERO:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_XMAS_SCAN_DROPf, &temp));
            *param = temp;
            break;
        case DRV_DOS_SYN_FIN_SCAN:
            SOC_IF_ERROR_RETURN(
                    soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                    unit, &reg_value, EN_TCP_SINFIN_SCAN_DROPf, &temp));
            *param = temp;
            break;
        default : 
            return SOC_E_UNAVAIL;
            break;
    }

    return SOC_E_NONE;
}

/*
 *  Function : drv_tbx_dos_event_bitmap_get
 *
 *  Purpose :
 *      Retreive a HW reported DOS event into SW basis DOS event bitmap.
 *
 *  Parameters :
 *      unit            :   unit id
 *      type            :   indicate the bitmap type
 *      event_bitmap    :   (OUT)dos event bitmap
 *
 *  Return :
 *      SOC_E_XXX
 *
 *  Note :
 *  1. The returned event bitmap must be SW basis.
 *
 */

int 
drv_tbx_dos_event_bitmap_get(int unit, uint32 op, uint32 *event_bitmap)
{
    int rv = SOC_E_NONE;
    uint32  reg_value = 0, temp = 0;
    uint32  evt_bmp = 0;

    *event_bitmap = 0;
    if (op == DRV_DOS_EVT_OP_ENABLED) {
        /* get HW dos enable status */
        SOC_IF_ERROR_RETURN(REG_READ_DOS_ATTACK_FILTER_DROP_CTLr(
                unit, &reg_value));
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_NULL_SCAN_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_NULL_SCAN) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_ICMPV4_LONGPING_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV4_LONGPING) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_ICMPV6_LONGPING_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV6_LONGPING) : 0;
        
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_ICMPV4_FRAG_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV4_FRAG) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_ICMPV6_FRAG_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV6_FRAG) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_FRAG_ERR_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_FRAG_ERR) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_SHORT_HDR_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_SHORT_HDR) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_BLAT_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_BLAT) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_UDP_BLAT_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_UDP_BLAT) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_IP_LAND_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_IP_LAND) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_SYN_ERR_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_SYN_ERR) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_XMAS_SCAN_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_XMAS_SCAN) : 0;

        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_DROP_CTLr_field_get(
                unit, &reg_value, EN_TCP_SINFIN_SCAN_DROPf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_SINFIN_SCAN) : 0;
    } else if (op == DRV_DOS_EVT_OP_STATUS) {
        /* TB's DOS event is a read-clear register, thus all event must be 
         *  retrieved together.
         */
        SOC_IF_ERROR_RETURN(REG_READ_DOS_ATTACK_FILTER_EVENTr(
                unit, &reg_value));
        
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, ICMPV6_LONGPING_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV6_LONGPING) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, ICMPV4_LONGPING_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV4_LONGPING) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, ICMPV6_FRAG_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV6_FRAG) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, ICMPV4_FRAG_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_ICMPV4_FRAG) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_FRAG_ERR_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_FRAG_ERR) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_SHORT_HDR_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_SHORT_HDR) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_SYN_ERR_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_SYN_ERR) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_SINFIN_SCAN_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_SINFIN_SCAN) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_XMAS_SCAN_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_XMAS_SCAN) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_NULL_SCAN_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_NULL_SCAN) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, TCP_BLAT_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_TCP_BLAT) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, UDP_BLAT_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_UDP_BLAT) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, IP_LAND_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_IP_LAND) : 0;
                
        SOC_IF_ERROR_RETURN(soc_DOS_ATTACK_FILTER_EVENTr_field_get(
                unit, &reg_value, MAC_LAND_EVENTf, &temp));
        evt_bmp |= (temp) ? (1 << DRV_DOS_EVT_MAC_LAND) : 0;
               
    } else {
        rv = SOC_E_UNAVAIL;
    }
    
    *event_bitmap = evt_bmp; 
    return rv;
}

