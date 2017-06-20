/*
 * $Id: sbx_red.c,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Redundancy API for SBX Switching technology
 */

#include <shared/bsl.h>

#include <soc/sbx/sbx_red.h>
#include <soc/sbx/xbar.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/hal_pt_auto.h>
#include <soc/sbx/hal_common.h>
#include <soc/sbx/qe2000_util.h>

#include <bcm/error.h>


#define BM3200_NS_PER_CLOCK (4) /* 250MHz clock, 4 ns, per clock */


STATIC bcm_error_t _sbxred_bm3200_sfi_set(int unit, bcm_pbmp_t sfi);
STATIC bcm_error_t _sbxred_bm3200_bme_add(int unit, bcm_port_t port, int num, int active);
STATIC bcm_error_t _sbxred_qe2000_bme_add(int unit, bcm_port_t port, int num, int active);


STATIC int32
_sbxred_get_timeslot_size(int unit, int32 nTotalLogicalCrossbars, 
                          int32 bSetTimeslotSizeForHalfBus,
                          int32 bHybrid)
{

    return(soc_sbx_fabric_get_timeslot_size(unit, nTotalLogicalCrossbars, bSetTimeslotSizeForHalfBus, bHybrid));
}


/*
 * Function:
 *     soc_sbx_red_sfi_get
 * Description:
 *     Get the nomimal switch fabric data plane bitmap
 * Parameters:
 *     unit  - BME BCM device number
 *     sfi   - (OUT) data plane bitmap of SFIs participating in
 *             active switching on the fabric
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_red_sfi_get( int unit, bcm_pbmp_t *sfi )
{
    uint32 uData, i;

    if( SOC_IS_SBX_BME(unit) ) {

        uData = SAND_HAL_READ(unit, PT, FO_CONFIG0);
        uData = SAND_HAL_GET_FIELD(PT, FO_CONFIG0, LINK_EN, uData);

        BCM_PBMP_CLEAR( *sfi );
        for( i=0; i<32; i++ ) {
            if( uData & (1<<i) ) {
                BCM_PBMP_PORT_ADD( *sfi, i );
            }
        }

        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}

/*
 * Function:
 *     soc_sbx_red_sfi_set
 * Description:
 *     Set the nomimal switch fabric data plane bitmap
 * Parameters:
 *     unit  - BME BCM device number
 *     sfi   - Data plane bitmap of SFIs participating in
 *             active switching on the fabric
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_red_sfi_set(int unit, bcm_pbmp_t sfi)
{

    if( SOC_IS_SBX_BME(unit) ) {
        return _sbxred_bm3200_sfi_set(unit, sfi);
    }
    else if (SOC_IS_SBX_QE(unit)) {
        return BCM_E_NONE; 
    }

    return BCM_E_UNIT;
}


/*
 * Function:
 *     soc_sbx_red_bme_add
 * Description:
 *     Add the BME unit to the switching topology
 * Parameters:
 *     unit   - BCM device number
 *     port   - Device port (SCI) where the BME is connected 
 *              (ignored if BME unit)
 *     num    - BME number in the system (0 or 1)
 *     active - TRUE if BME is active, otehrwise BME is standby
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_red_bme_add( int unit, bcm_port_t port, int num, int active )
{
    bcm_error_t rv = BCM_E_UNIT;

    if( SOC_IS_SBX_BME(unit) ) {
        rv = _sbxred_bm3200_bme_add(unit, port, num, active);
    } else if( SOC_IS_SBX_QE(unit) ) {
        rv = _sbxred_qe2000_bme_add(unit, port, num, active);
    }

    return rv;
}

/*
 * Function:
 *     soc_sbx_red_bme_delete
 * Description:
 *     Remove the BME unit from the switching topology
 * Parameters:
 *     unit   - BCM device number
 *     port   - Device port (SCI) where the BME is connected 
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_red_bme_delete( int unit, bcm_port_t port )
{

    if( SOC_IS_SBX_BME(unit) ) {
        SAND_HAL_WRITE(unit, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_DIS, 1));
        
        SAND_HAL_WRITE(unit, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, TAG_CHECK_DIS, 0) |
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_EN, 0));
    

        return BCM_E_NONE;
    }
    
    return BCM_E_UNIT;
}




STATIC bcm_error_t
_sbxred_qe2000_bme_add(int unit, bcm_port_t port, int num, int active)
{

    /*SAND_HAL_RMW_FIELD(unit, KA, SC_CONFIG0, ENABLE_AUTO_SWITCHOVER, 1);*/

    if (active) {
        SAND_HAL_RMW_FIELD(unit, KA, SC_CONFIG0, DEFAULT_BM, num);
        
    } else {
        
    }

    return BCM_E_NONE;
}


STATIC bcm_error_t
_sbxred_bm3200_bme_add(int unit, bcm_port_t port, int num, int active)
{
    bcm_error_t rv = BCM_E_NONE;
    int32 nRegister;
    int32 n10MsDelayCnt;
    
    nRegister = SAND_HAL_READ(unit, PT, FO_CONFIG0);
    nRegister = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, MAX_DIS_LINKS, nRegister, 3);
    nRegister = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, ENABLE_MORE_LINK_DIS, nRegister, 1);
    nRegister = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, ENABLE_AUTO_LINK_DIS, nRegister, 1);
    /*nRegister = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, ENABLE_AUTO_SWITCHOVER, nRegister, 1);*/
    nRegister = SAND_HAL_MOD_FIELD(PT, FO_CONFIG0, LOCAL_BM_ID, nRegister, num);
    SAND_HAL_WRITE(unit, PT, FO_CONFIG0, nRegister);

    if (active) {

        SAND_HAL_RMW_FIELD(unit, PT, FO_CONFIG0, DEFAULT_BM_ID, num);
        
        
        nRegister = SAND_HAL_READ(unit, PT, BW_EPOCH_CONFIG);

        nRegister = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, BW_MSG_ENABLE, nRegister, 1);
        nRegister = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, BW_ENABLE, nRegister, 1);
        nRegister = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, WRED_MSG_ENABLE, nRegister, 0);
        nRegister = SAND_HAL_MOD_FIELD(PT, BW_EPOCH_CONFIG, WRED_ENABLE, nRegister, 0);
    
        SAND_HAL_WRITE(unit, PT, BW_EPOCH_CONFIG, nRegister);
    
        SAND_HAL_WRITE(unit, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, TAG_CHECK_DIS, 0) |
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_EN, 1));
    
        for (n10MsDelayCnt = 0; n10MsDelayCnt < 100; n10MsDelayCnt ++) {
        
            /* Delay 10 ms at a time */
            sal_usleep(100000);
        
            nRegister = SAND_HAL_READ(unit, PT, BW_EVENT);
            if (SAND_HAL_GET_FIELD(PT, BW_EVENT, SCHED_ACT, nRegister)) {
                SAND_HAL_WRITE(unit, PT, BW_EVENT, ~0);
                rv = BCM_E_NONE;
                break;
            }
        }
         
        /* Delay 20ms */
        sal_usleep(2000000);
        nRegister = SAND_HAL_READ(unit, PT, BW_ERROR);
        if (nRegister != 0) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "bw_error after enabling scheduler (0x%08x)\n"), 
                       nRegister));
            rv = BCM_E_FAIL;
        }

    } else { /* stand by */
        SAND_HAL_WRITE(unit, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_DIS, 1));
        SAND_HAL_WRITE(unit, PT, BW_CONFIG,
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, TAG_CHECK_DIS, 0) |
                       SAND_HAL_SET_FIELD(PT, BW_CONFIG, SCHED_EN, 0));
        rv = BCM_E_NONE;
    }
    return rv;
}


STATIC bcm_error_t
_sbxred_bm3200_sfi_set(int unit, bcm_pbmp_t sfi)
{
    bcm_error_t rv = BCM_E_NONE;
    uint32 uLinkEn = 0, i;
    int32  nTsSizeNormNs, nTsSizeDegrNs;
    int32  nTsSizeNormClocks, nTsSizeDegrClocks;
    int32  nSfiCount;
    uint32 uData, uTmp;
    int32  nNewTsLarger;

    
    int32  nHalfBus = 0;
    int32  nMaxFailedLink = 1;

    /* set the link enables on the BME */
    nSfiCount = 0;
    BCM_PBMP_ITER( sfi, i ){
        if (i < 32) {
            uLinkEn |= 1 << i;
            nSfiCount++;
        }
    }
        
    nTsSizeNormNs = _sbxred_get_timeslot_size(unit, nSfiCount, nHalfBus, soc_feature(unit, soc_feature_hybrid) );
    nTsSizeDegrNs = _sbxred_get_timeslot_size(unit, nSfiCount - nMaxFailedLink,
                                               nHalfBus, soc_feature(unit, soc_feature_hybrid) );

    /* The timeslot_size and deg_timeslot_size fields in the ac_config1 
     * register are set to one less than the value you wish to be used. 
     * See the BM3200 spec
     */
    nTsSizeNormClocks = (nTsSizeNormNs / BM3200_NS_PER_CLOCK) - 1;
    nTsSizeDegrClocks = (nTsSizeDegrNs / BM3200_NS_PER_CLOCK) - 1;


    /* When adjusting the Timeslot size, the following must be done:
     * 1. Force the arbiter to generate null grants to avoid drops 
     * 2. if the TS gets smaller, update grant offset first
     * 3. update timeslot sizes (normal & degraded)
     * 4. if TS get larger, update grant offset last
     * 5. update link_en last
     * 6. disable force null grants
     */

    SAND_HAL_RMW_FIELD(unit, PT, AC_CONFIG0, FORCE_NULL_GRANT, 1);

    /* if TS size decreases, update the Grant offset first */
    uData = SAND_HAL_READ(unit, PT, AC_CONFIG1);
    if (SAND_HAL_GET_FIELD(PT, AC_CONFIG1, TIMESLOT_SIZE, uData) > 
        nTsSizeNormClocks)
    {
        nNewTsLarger = 0;

        uTmp = SAND_HAL_READ(unit, PT, AC_CONFIG2);
        uTmp = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_GRANT_OFFSET, 
                                  uTmp, nTsSizeNormClocks - 0xf);
        SAND_HAL_WRITE(unit, PT, AC_CONFIG2, uTmp);
    } else {
        nNewTsLarger = 1;
    }

    /* update the normal and degraded timeslot sizes */
    uData = SAND_HAL_MOD_FIELD(PT, AC_CONFIG1, TIMESLOT_SIZE, uData, nTsSizeNormClocks);
    uData = SAND_HAL_MOD_FIELD(PT, AC_CONFIG1, DEG_TIMESLOT_SIZE, uData, nTsSizeDegrClocks);
    SAND_HAL_WRITE(unit, PT, AC_CONFIG1, uData);

    if ( nNewTsLarger ) {
        uTmp = SAND_HAL_READ(unit, PT, AC_CONFIG2);
        uTmp = SAND_HAL_MOD_FIELD(PT, AC_CONFIG2, SEND_GRANT_OFFSET, 
                                  uTmp, nTsSizeNormClocks - 0xf);
        SAND_HAL_WRITE(unit, PT, AC_CONFIG2, uTmp);
    }

    /* configure the fabric data plane */
    SAND_HAL_RMW_FIELD(unit, PT, FO_CONFIG0, LINK_EN, uLinkEn);
        
    /* Done: re-enable the BME */
    SAND_HAL_RMW_FIELD(unit, PT, AC_CONFIG0, FORCE_NULL_GRANT, 0);

    return rv;
}

