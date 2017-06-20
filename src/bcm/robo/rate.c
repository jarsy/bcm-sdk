/*
 * $Id: rate.c,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Rate - Broadcom RoboSwitch Rate Limiting API.
 */

#include <shared/bsl.h>

#include <sal/types.h>

#include <soc/drv.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <bcm/rate.h>
#include <bcm/port.h>
#include <bcm/types.h>
#include <bcm/error.h>

/*
 * One suppression status for each SOC device containing storm control
 *  enable status for that device.
 */
static int bcm_robo_storm_en[BCM_MAX_NUM_UNITS] = {FALSE};

/* Rate API working process for Robo5324:
 *  1. There are two Rate control buckets (bucket0 and bucket1), if conflict 
 *     setting occurred, bucket1 will be chose for the higher priority nature.
 *  2. Rate API is servicing for Storm control funciton. The defined storm 
 *     suppression is controled by bucket1 setting.(bucket_size1 and ref_cnt1)
 *  3. Independent rate limit value and burst size is permitted in Port Storm 
 *     Suppression and Port ingress rate control. 
 *     (ingress rate control use bucket0 for such implementation) 
 */

/*
 * Function:
 *  _bcm_robo_rate_valid_pbmp_check
 * Description:
 *  Check if port is valid for rate control.
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  port - Port number need to check.
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_PARAM - Invalid port number.
 */
int _bcm_robo_rate_valid_pbmp_check(int unit, int port)
{
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX];
    int i;

    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
    for (i=0; i < SOC_PBMP_WORD_MAX; i++){
        SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
    }
    if (!BCM_PBMP_MEMBER(valid_bmp, port)) {
        return BCM_E_PARAM;
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *  _bcm_robo_rate_init
 * Description:
 *  Initialize rate controls.
 * Parameters:
 *  unit        - Device unit number
 * Returns:
 *      BCM_E_xxx
 */
int 
_bcm_robo_rate_init(int unit)
{
    bcm_pbmp_t  pbmp;
    bcm_port_t  port = 0;
    int  enable = 0;

    BCM_PBMP_CLEAR(pbmp);

    if (SOC_IS_TBX(unit)) {
        /* Default to enable per-port ingress rate control Drop enable */
        BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));
        enable = 1;
    
        PBMP_ITER(pbmp, port) {
            BCM_IF_ERROR_RETURN(bcm_port_control_set
                (unit, port, bcmPortControlIngressRateControlDrop, enable));
        }
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *  bcm_robo_rate_set
 * Description:
 *  Configure rate limit and on/off state of
 *  DLF, MCAST, and BCAST limiting
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - Rate limit value in packets/second
 *  flags - Bitmask with one or more of BCM_RATE_*
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *  - RoboSwitch supports only one rate limit for all 3 types.
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will use this value by Kb per second.
 */
int bcm_robo_rate_set(int unit, int pps, int flags)
{
    int     port;
    pbmp_t  tr_pbmp;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX];
    int i;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_set()..\n")));
    rate_type |= (flags & BCM_RATE_BCAST) ? 
                DRV_STORM_CONTROL_BCAST : 0;
    rate_type |= (flags & BCM_RATE_MCAST) ? 
                DRV_STORM_CONTROL_MCAST : 0;
    rate_type |= (flags & BCM_RATE_DLF) ? 
                DRV_STORM_CONTROL_DLF : 0;
    
    /* Rate_Limit value :
     * The rate_limit value in API is packet per second, but Drv layer 
     * will use this value by Kb per second.
     */  
    rate_limit = pps;
    
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_ASSIGN(tr_pbmp, PBMP_ALL(unit));
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
    for (i=0; i < SOC_PBMP_WORD_MAX; i++){
        SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
    }
    BCM_PBMP_AND(tr_pbmp, valid_bmp);

    /* make sure the storm suppression has enabled */
    if (bcm_robo_storm_en[unit] == FALSE){
        bcm_robo_storm_en[unit] = TRUE;
        PBMP_ITER(tr_pbmp, port) {
            /* set the storm control rate_type & rate_limit also */
            BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_ENABLE_SET
                (unit, port, bcm_robo_storm_en[unit]));
        }
    }

    /* set the storm control rate_type & rate_limit also */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_SET
        (unit, tr_pbmp, rate_type, rate_limit, burst_size));

    return BCM_E_NONE;
}               

/*
 * Function:
 *  bcm_robo_rate_get
 * Description:
 *  Get rate limit and on/off state of
 *  DLF, MCAST, and BCAST limiting
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - (OUT) Place to store returned rate limit value
 *  flags - (OUT) Place to store returned flag bitmask with
 *      one or more of BCM_RATE_*
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *  Actually returns the rate for BCAST only, but assumes the
 *  bcm_rate_set call was used so DLF, MCAST and BCAST should be equal.
 */
int bcm_robo_rate_get(int unit, int *pps, int *flags)
{
    int     port;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_get()..\n")));
    /* get first enabled ethernet port */
    PBMP_E_ITER(unit, port) {
        break;
    }
    
    /* get the current storm control type */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));
    
    *flags = 0;
    *flags |= (rate_type & DRV_STORM_CONTROL_BCAST) ?
                 BCM_RATE_BCAST : 0;
    *flags |= (rate_type & DRV_STORM_CONTROL_MCAST) ?
                 BCM_RATE_MCAST : 0;
    *flags |= (rate_type & DRV_STORM_CONTROL_DLF) ?
                 BCM_RATE_DLF : 0;
    
    *pps = rate_limit;

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_rate_mcast_set
 * Description:
 *  Configure rate limit for MCAST packets for the given port
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - Rate limit value in packets/second
 *  flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which MCAST limit needs to be set
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_UNAVAIL - Not supported.
 * Notes:
 *  - Robo chip on rate/storm control use the same rate_limit value.
 *    This limit value is system basis but port basis.
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will use this value by Kb per second.
 */
int bcm_robo_rate_mcast_set(int unit, int pps, int flags, int port)
{
    pbmp_t  tr_pbmp;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    int tmp_port=0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_mcast_set()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    
    /* make sure the storm suppression has enabled */
    if (bcm_robo_storm_en[unit] == FALSE){
        bcm_robo_storm_en[unit] = TRUE;
         PBMP_E_ITER(unit, tmp_port) {
            /* set the storm control rate_type & rate_limit also */
            BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_ENABLE_SET
                (unit, tmp_port, bcm_robo_storm_en[unit]));
        }

    }

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    if (flags & BCM_RATE_MCAST) {
        rate_type |= DRV_STORM_CONTROL_MCAST ;
        rate_limit = pps;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_MCAST ;
    }
   
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_PORT_ADD(tr_pbmp, port);

    /* set the storm control rate_type & rate_limit also */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_SET
        (unit, tr_pbmp, rate_type, rate_limit, burst_size));

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_rate_bcast_set
 * Description:
 *  Configure rate limit for BCAST packets
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - Rate limit value in packets/second
 *  flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which BCAST limit needs to be set
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_UNAVAIL - Not supported.
 * Notes:
 *  - Robo chip on rate/storm control use the same rate_limit value.
 *    This limit value is system basis but port basis.
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will view this value as Kb per second.
 */
int bcm_robo_rate_bcast_set(int unit, int pps, int flags, int port)
{
    pbmp_t  tr_pbmp;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    int tmp_port=0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_bcast_set()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));

    /* make sure the storm suppression has enabled */
    if (bcm_robo_storm_en[unit] == FALSE){
        bcm_robo_storm_en[unit] = TRUE;
         PBMP_E_ITER(unit, tmp_port) {
            /* set the storm control rate_type & rate_limit also */
            BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_ENABLE_SET
                (unit, tmp_port, bcm_robo_storm_en[unit]));
        }

    }

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    if (flags & BCM_RATE_BCAST) {
        rate_type |= DRV_STORM_CONTROL_BCAST ;
        rate_limit = pps;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_BCAST ;
    }
   
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_PORT_ADD(tr_pbmp, port);

    /* set the storm control rate_type & rate_limit also */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_SET
        (unit, tr_pbmp, rate_type, rate_limit, burst_size));

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_rate_dlfbc_set
 * Description:
 *  Configure rate limit for DLFBC packets
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - Rate limit value in packets/second
 *  flags - Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which DLFBC limit needs to be set
 * Returns:
 *  BCM_E_NONE - Success.
 *  BCM_E_UNAVAIL - Not supported.
 * Notes:
 *  - Robo chip on rate/storm control use the same rate_limit value.
 *    This limit value is system basis but port basis.
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will use this value by Kb per second.
 */
int bcm_robo_rate_dlfbc_set(int unit, int pps, int flags, int port)
{
    pbmp_t  tr_pbmp;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
     int tmp_port=0;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_dlfbc_set()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));

    /* make sure the storm suppression has enabled */
    if (bcm_robo_storm_en[unit] == FALSE){
        bcm_robo_storm_en[unit] = TRUE;
        PBMP_E_ITER(unit, tmp_port) {
            /* set the storm control rate_type & rate_limit also */
            BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_ENABLE_SET
                (unit, tmp_port, bcm_robo_storm_en[unit]));
        }

    }

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    if (flags & BCM_RATE_DLF) {
        rate_type |= DRV_STORM_CONTROL_DLF ;
        rate_limit = pps;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_DLF ;
    }
   
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_PORT_ADD(tr_pbmp, port);

    /* set the storm control rate_type & rate_limit also */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_SET
        (unit, tr_pbmp, rate_type, rate_limit, burst_size));

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_rate_mcast_get
 * Description:
 *  Get rate limit for MCAST packets
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - (OUT) Rate limit value in packets/second
 *  flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which MCAST limit is requested
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will use this value by Kb per second.
 */
int bcm_robo_rate_mcast_get(int unit, int *pps, int *flags, int port)
{
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_mcast_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    *flags |= (rate_type & DRV_STORM_CONTROL_MCAST) ? 
                BCM_RATE_MCAST : 0;
   
    *pps = rate_limit;
    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_rate_dlfbc_get
 * Description:
 *  Get rate limit for DLFBC packets
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - (OUT) Rate limit value in packets/second
 *  flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which DLFBC limit is requested
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will use this value by Kb per second.
 */
int bcm_robo_rate_dlfbc_get(int unit, int *pps, int *flags, int port)
{
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_dlfbc_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    *flags |= (rate_type & DRV_STORM_CONTROL_DLF) ? 
                BCM_RATE_DLF : 0;
   
    *pps = rate_limit;
    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_rate_bcast_get
 * Description:
 *  Get rate limit for BCAST packets
 * Parameters:
 *  unit - RoboSwitch PCI device unit number
 *  pps - (OUT) Rate limit value in packets/second
 *  flags - (OUT) Bitmask with one or more of BCM_RATE_*
 *      port - Port number for which BCAST limit is requested
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  - The rate_limit value in API is packet per second, but Drv layer 
 *      will use this value by Kb per second.
 */
int bcm_robo_rate_bcast_get(int unit, int *pps, int *flags, int port)
{
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_bcast_get()..\n")));

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) {
        return BCM_E_PORT;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    *flags |= (rate_type & DRV_STORM_CONTROL_BCAST) ? 
                BCM_RATE_BCAST : 0;
   
    *pps = rate_limit;
    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_rate_type_get
 * Description:
 *  Front end to bcm_*cast_rate_get functions.
 *      Uses a single data structure to read from
 *      all the 3 rate control registers
 * Parameters:
 *  unit - unit number
 *      rl - (OUT) data structure containing info to be acquired from the
 *           rate control registers
 * Returns:
 *  BCM_E_XXX
 *
 * Note :
 *  1. The Storm control type in Robo chip is allowed port basis setting.
 *     It means each port may contains different storm control type.
 *  2. In this API, we get the first port's storm contorl type to represent 
 *     all port's storm control type.
 */
int bcm_robo_rate_type_get(int unit, bcm_rate_limit_t *rl)
{
    int     port;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_type_get()..\n")));
    /* get first enabled ethernet port */
    PBMP_E_ITER(unit, port) {
        break;
    }
    
    /* get the current storm control type */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));
    
    rl->flags = 0;
    rl->flags |= (rate_type & DRV_STORM_CONTROL_BCAST) ?
                 BCM_RATE_BCAST : 0;
    rl->flags |= (rate_type & DRV_STORM_CONTROL_MCAST) ?
                 BCM_RATE_MCAST : 0;
    rl->flags |= (rate_type & DRV_STORM_CONTROL_DLF) ?
                 BCM_RATE_DLF : 0;

    if (rate_type & DRV_STORM_CONTROL_BCAST) {
        rl->br_bcast_rate = rate_limit;
    }
    
    if (rate_type & DRV_STORM_CONTROL_MCAST) {
        rl->br_mcast_rate = rate_limit;
    }

    if (rate_type & DRV_STORM_CONTROL_DLF) {
        rl->br_dlfbc_rate = rate_limit;
    }

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *  bcm_robo_rate_type_set
 * Description:
 *  Front end to bcm_*cast_rate_set functions.
 *      Uses a single data structure to write into
 *      all the 3 rate control registers
 * Parameters:
 *  unit - unit number
 *      rl - data structure containing info to be written to the
 *           rate control registers
 * Returns:
 *  BCM_E_XXX
 *
 * Note :
 *  1. The Storm control type in Robo chip is allowed port basis setting.
 *     It means each port may contains different storm control type.
 *  2. In this API, we set the storm contorl type on all ports.
 *  3. Robo Chip is not allowed port based rate limit. We set the rate 
 *     based on Broadcast rate setting.
 */
int bcm_robo_rate_type_set(int unit, bcm_rate_limit_t *rl)
{
    int     port;
    pbmp_t  tr_pbmp;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    bcm_pbmp_t  valid_bmp;
    uint32 value[SOC_PBMP_WORD_MAX];
    uint32  temp_flag = rl->flags & 
                ~(BCM_RATE_BCAST | BCM_RATE_MCAST | BCM_RATE_DLF);
    int i;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_type_set()..\n")));
                
    /* error return if the carrying flag is out of supporting type */
    if (temp_flag){
        
        LOG_WARN(BSL_LS_BCM_COMMON,
                 (BSL_META_U(unit,
                             "%s, flag=0x%X, the 0x%X are not supported!\n"), 
                  FUNCTION_NAME(), rl->flags, temp_flag));
        return BCM_E_UNAVAIL;
    }
                
    rate_type |= (rl->flags & BCM_RATE_BCAST) ? 
                DRV_STORM_CONTROL_BCAST : 0;
    rate_type |= (rl->flags & BCM_RATE_MCAST) ? 
                DRV_STORM_CONTROL_MCAST : 0;
    rate_type |= (rl->flags & BCM_RATE_DLF) ? 
                DRV_STORM_CONTROL_DLF : 0;
    
    if (rl->br_bcast_rate) {
        if (!(rl->flags & BCM_RATE_BCAST)) {
            return BCM_E_PARAM;
        }
    }
    if (rl->br_mcast_rate) {
        if (!(rl->flags & BCM_RATE_MCAST)) {
            return BCM_E_PARAM;
        }
    }
    if (rl->br_dlfbc_rate) {
        if (!(rl->flags & BCM_RATE_DLF)) {
            return BCM_E_PARAM;
        }
    }
    
    /* 
     *  - The rate_limit value in API is packet per second, but Drv layer 
     *      will use this value by Kb per second.
     */  
    if (rl->br_bcast_rate) {        
        rate_limit = rl->br_bcast_rate;
        if ((rate_limit > rl->br_mcast_rate) && (rl->br_mcast_rate > 0)) {
            rate_limit = rl->br_mcast_rate;
        }
        if ((rate_limit > rl->br_dlfbc_rate) && (rl->br_dlfbc_rate > 0)) {
            rate_limit = rl->br_dlfbc_rate;
        }
    } else if (rl->br_mcast_rate) {
        rate_limit = rl->br_mcast_rate;
        if ((rate_limit > rl->br_dlfbc_rate) && (rl->br_dlfbc_rate > 0)) {
            rate_limit = rl->br_dlfbc_rate;
        }
    } else if (rl->br_dlfbc_rate) {
        rate_limit = rl->br_dlfbc_rate;
    }


    
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_ASSIGN(tr_pbmp, PBMP_ALL(unit));
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_RATE_CONTROL_PBMP, &value[0]));
    for (i=0; i < SOC_PBMP_WORD_MAX; i++) {
        SOC_PBMP_WORD_SET(valid_bmp, i, value[i]);
    }
    BCM_PBMP_AND(tr_pbmp, valid_bmp);

    /* make sure the storm suppression has enabled */
    if (bcm_robo_storm_en[unit] == FALSE){
        bcm_robo_storm_en[unit] = TRUE;
        PBMP_ITER(tr_pbmp, port) {
            /* set the storm control rate_type & rate_limit also */
            BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_ENABLE_SET
                (unit, port, bcm_robo_storm_en[unit]));
        }

    }

    /* set the storm control rate_type & rate_limit also */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_SET
        (unit, tr_pbmp, rate_type, rate_limit, burst_size));

    return BCM_E_NONE;
}               

/*
 * Function:
 *      bcm_robo_rate_bandwidth_get
 * Description:
 *      Get rate bandwidth limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Bitmask with one of the following:
 *              BCM_RATE_BCAST
 *              BCM_RATE_MCAST
 *              BCM_RATE_DLF
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *                  Rate of 0 disabled rate limiting.
 *      kbits_burst - Maximum burst size in kilobits(1000 bits)
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported
 *      BCM_E_XXX - Error.
*/

int 
bcm_robo_rate_bandwidth_get(int unit, bcm_port_t port, int flags, 
                       uint32 *kbits_sec, uint32 *kbits_burst)
{
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    int loc_flags;
    
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_bandwidth_get()..\n")));
    
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));

    if (flags & BCM_RATE_CTRL_BUCKET_1) {
        rate_type |= DRV_STORM_CONTROL_BUCKET_1;
    } else if (flags & BCM_RATE_CTRL_BUCKET_2) {
        rate_type |= DRV_STORM_CONTROL_BUCKET_2;
    }

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    loc_flags = flags & BCM_RATE_ALL;
    switch (loc_flags) {
        case BCM_RATE_BCAST :
            if (rate_type & DRV_STORM_CONTROL_BCAST) {
                *kbits_sec= rate_limit;
                *kbits_burst= burst_size;
            }
            break;
        case BCM_RATE_MCAST :
            if (rate_type & DRV_STORM_CONTROL_MCAST) {
                *kbits_sec= rate_limit;
                *kbits_burst= burst_size;
            }
            break;
        case BCM_RATE_DLF :
            if (rate_type & DRV_STORM_CONTROL_DLF) {
                *kbits_sec= rate_limit;
                *kbits_burst= burst_size;
            }
            break;
        case BCM_RATE_UCAST :
            if (rate_type & DRV_STORM_CONTROL_UCAST) {
                *kbits_sec= rate_limit;
                *kbits_burst= burst_size;
            }
            break;
        case BCM_RATE_SALF:
            if (rate_type & DRV_STORM_CONTROL_SALF) {
                *kbits_sec= rate_limit;
                *kbits_burst= burst_size;
            }
            break;
        case BCM_RATE_RSVD_MCAST:
            if (rate_type & DRV_STORM_CONTROL_RSV_MCAST) {
                *kbits_sec= rate_limit;
                *kbits_burst= burst_size;
            }
            break;
        default: 
            return BCM_E_PARAM;
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *      bcm_robo_rate_bandwidth_set
 * Description:
 *      Set rate bandwidth limiting parameters
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      flags - Bitmask with one of the following:
 *              BCM_RATE_BCAST
 *              BCM_RATE_MCAST
 *              BCM_RATE_DLF
 *      kbits_sec - Rate in kilobits (1000 bits) per second.
 *                  Rate of 0 disables rate limiting.
 *      kbits_burst - Maximum burst size in kilobits(1000 bits)
 * Returns:
 *      BCM_E_NONE - Success.
 *      BCM_E_UNAVAIL - Not supported
 *      BCM_E_XXX - Error.
*/

int 
bcm_robo_rate_bandwidth_set(int unit, bcm_port_t port, int flags, 
                       uint32 kbits_sec, uint32 kbits_burst)
{
    pbmp_t  tr_pbmp;
    uint32  rate_type = 0, rate_limit = 0, burst_size = 0; 
    int tmp_port=0;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_rate_bandwidth_set()..\n")));
    if (!SOC_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            bcm_port_local_get(unit, port, &port));
    }
    if (!SOC_PORT_VALID(unit, port)) { 
        return BCM_E_PORT; 
    }

    if (!(flags & BCM_RATE_ALL)) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(_bcm_robo_rate_valid_pbmp_check(unit, port));
    /* make sure the storm suppression has enabled */
    if (bcm_robo_storm_en[unit] == FALSE){
        bcm_robo_storm_en[unit] = TRUE;
         PBMP_E_ITER(unit, tmp_port) {
            /* set the storm control rate_type & rate_limit also */
            BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_ENABLE_SET
                (unit, tmp_port, bcm_robo_storm_en[unit]));
        }

    }

    if (flags & BCM_RATE_CTRL_BUCKET_1) {
        rate_type |= DRV_STORM_CONTROL_BUCKET_1;
    } else if (flags & BCM_RATE_CTRL_BUCKET_2) {
        rate_type |= DRV_STORM_CONTROL_BUCKET_2;
    }

    /* get original storm control setting */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_GET
        (unit, port, &rate_type, &rate_limit, &burst_size));

    if (flags & BCM_RATE_BCAST) {
        rate_type |= DRV_STORM_CONTROL_BCAST ;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_BCAST ;
    }

    if (flags & BCM_RATE_MCAST) {
        rate_type |= DRV_STORM_CONTROL_MCAST ;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_MCAST ;
    }

    if (flags & BCM_RATE_DLF) {
        rate_type |= DRV_STORM_CONTROL_DLF;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_DLF ;
    }

    if (flags & BCM_RATE_UCAST) {
        rate_type |= DRV_STORM_CONTROL_UCAST;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_UCAST ;
    }

    if (flags & BCM_RATE_SALF) {
        rate_type |= DRV_STORM_CONTROL_SALF;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_SALF ;
    }

    if (flags & BCM_RATE_RSVD_MCAST) {
        rate_type |= DRV_STORM_CONTROL_RSV_MCAST;
    } else {
        rate_type &= ~DRV_STORM_CONTROL_RSV_MCAST ;
    }

    rate_limit = kbits_sec;
    burst_size = kbits_burst;
   
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_PORT_ADD(tr_pbmp, port);

    /* set the storm control rate_type & rate_limit also */
    BCM_IF_ERROR_RETURN(DRV_STORM_CONTROL_SET
        (unit, tr_pbmp, rate_type, rate_limit, burst_size));

    return BCM_E_NONE;
}

