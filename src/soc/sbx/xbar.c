/*
 * $Id: xbar.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 *
 * QE2000 Crossbar configuration accessors
 */

#include <soc/sbx/xbar.h>

#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/hal_pt_auto.h>

#include <soc/sbx/qe2000_util.h>

#include <bcm/debug.h>
#include <bcm/error.h>


#define BCM_XBAR_WAIT_XCNFG_TIME (4) /* milliseconds */

STATIC int _qe2000_sfi_rd_wr_xcfg( int unit, int nRead, bcm_port_t port, 
                                   int modid, bcm_port_t* pXbport );


STATIC int _qe2000_sfi_xcnfg_mem_ack_wait( int unit, bcm_port_t port, 
                                           uint32 uMsTimeout );


/*
 * Function:
 *     soc_sbx_xbar_config_set
 * Description:
 *     Set the physical SFI port number of a QE on logical xbar
 * Parameters:
 *     unit   - QE BCM device number
 *     port   - QE device port number of the SFI port to configure.
 *     modid  - Module ID of the remote QE reached by xbport
 *     xbport - Device port number on the crossbar switching device 
 *              (SE or BME) that reaches module modid.
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_xbar_config_set( int unit, 
                         bcm_port_t port, 
                         int modid, 
                         bcm_port_t xbport )
{
    bcm_error_t rv;

    if( SOC_IS_SBX_QE(unit) ) {
        rv = _qe2000_sfi_rd_wr_xcfg( unit, 0 /*==write*/, port, modid, &xbport );
        if( BCM_FAILURE(rv) ) {
            return rv;
        }
    
        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}

/*
 * Function:
 *     soc_sbx_xbar_config_delete
 * Description:
 *     Remove the physical SFI port number of a QE from a logical xbar
 *     configuration.
 * Parameters:
 *     unit   - QE BCM device number
 *     port   - QE device port number of the SFI port to delete
 *     modid  - Module ID of the remote QE reached by this xbar configuration
 * 
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_xbar_config_delete( int unit, bcm_port_t port, int modid ) 
{
    if( SOC_IS_SBX_QE(unit) ) {
        int xbport = 0x7f;
        /* set the xcfg for loopback when deleting */
        return _qe2000_sfi_rd_wr_xcfg( unit, 1 /*==read*/, port, modid, &xbport );
    }

    return BCM_E_UNIT;
}


/*
 * Function:
 *     soc_sbx_xbar_config_get
 * Description:
 *     Get the physical SFI port number of a QE on logical xbar
 * Parameters:
 *     unit   - QE BCM device number
 *     port   - QE device port number of the SFI port to get.
 *     modid  - Module ID of the remote QE reached by xbport
 *     xbport - (OUT) Device port number on the crossbar switching device 
 *              (SE or BME) that reaches module modid.
 * Returns:
 *     BCM_E_NONE    - Success
 *     BCM_E_XXX     - Failure, other errors
 */
bcm_error_t
soc_sbx_xbar_config_get(int unit, bcm_port_t port, bcm_module_t modid, 
                        bcm_port_t *xbport)
{
    if( SOC_IS_SBX_QE(unit) ) {
        return _qe2000_sfi_rd_wr_xcfg( unit, 1 /*==read*/, port, modid, xbport );
    }

    return BCM_E_UNIT;
}


bcm_error_t
soc_sbx_xbar_mode_set(int unit, soc_sbx_xbar_mode_e mode)
{
    if (SOC_IS_SBX_BME(unit)) {
        if (mode == sbx_xbar_mode_normal)
        {
            SAND_HAL_RMW_FIELD(unit, PT, XB_CONFIG0, NO_DESKEW, 0);
        } else {
            SAND_HAL_RMW_FIELD(unit, PT, XB_CONFIG0, NO_DESKEW, 1);
        }

        SAND_HAL_RMW_FIELD(unit, PT, XB_CONFIG0, XCFG_MODE, mode);

        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}

bcm_error_t
soc_sbx_xbar_mode_get(int unit, soc_sbx_xbar_mode_e *mode)
{
    if (SOC_IS_SBX_BME(unit)) {
        uint32 data = SAND_HAL_READ(unit, PT, XB_CONFIG0);
        *mode = SAND_HAL_GET_FIELD(PT, XB_CONFIG0, XCFG_MODE, data);
        return BCM_E_NONE;
    }

    return BCM_E_UNIT;    
}


bcm_error_t
soc_sbx_xbar_fixed_config(int unit, int configAB, 
                          bcm_port_t xcfg[], int num_xcfgs)
{


    if (SOC_IS_SBX_BME(unit)) {
        if (num_xcfgs < 40) {
            return BCM_E_PARAM;
        }


#define SBX_XCFG_SET(r,x)     SAND_HAL_SET_FIELD(PT, XB_FIXED_XCFG_##r, XCFG##x, xcfg[(x)])

        if (configAB) {
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A0, 
                           (SBX_XCFG_SET(A0, 0) |
                            SBX_XCFG_SET(A0, 1) |
                            SBX_XCFG_SET(A0, 2) |
                            SBX_XCFG_SET(A0, 3) |
                            SBX_XCFG_SET(A0, 4)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A1, 
                           (SBX_XCFG_SET(A1, 5) |
                            SBX_XCFG_SET(A1, 6) |
                            SBX_XCFG_SET(A1, 7) |
                            SBX_XCFG_SET(A1, 8) |
                            SBX_XCFG_SET(A1, 9)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A2, 
                           (SBX_XCFG_SET(A2, 10) |
                            SBX_XCFG_SET(A2, 11) |
                            SBX_XCFG_SET(A2, 12) |
                            SBX_XCFG_SET(A2, 13) |
                            SBX_XCFG_SET(A2, 14)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A3, 
                           (SBX_XCFG_SET(A3, 15) |
                            SBX_XCFG_SET(A3, 16) |
                            SBX_XCFG_SET(A3, 17) |
                            SBX_XCFG_SET(A3, 18) |
                            SBX_XCFG_SET(A3, 19)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A4, 
                           (SBX_XCFG_SET(A4, 20) |
                            SBX_XCFG_SET(A4, 21) |
                            SBX_XCFG_SET(A4, 22) |
                            SBX_XCFG_SET(A4, 23) |
                            SBX_XCFG_SET(A4, 24)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A5, 
                           (SBX_XCFG_SET(A5, 25) |
                            SBX_XCFG_SET(A5, 26) |
                            SBX_XCFG_SET(A5, 27) |
                            SBX_XCFG_SET(A5, 28) |
                            SBX_XCFG_SET(A5, 29)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A6, 
                           (SBX_XCFG_SET(A6, 30) |
                            SBX_XCFG_SET(A6, 31) |
                            SBX_XCFG_SET(A6, 32) |
                            SBX_XCFG_SET(A6, 33) |
                            SBX_XCFG_SET(A6, 34)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_A7, 
                           (SBX_XCFG_SET(A7, 35) |
                            SBX_XCFG_SET(A7, 36) |
                            SBX_XCFG_SET(A7, 37) |
                            SBX_XCFG_SET(A7, 38) |
                            SBX_XCFG_SET(A7, 39)));


                
        } else {

            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B0, 
                           (SBX_XCFG_SET(B0, 0) |
                            SBX_XCFG_SET(B0, 1) |
                            SBX_XCFG_SET(B0, 2) |
                            SBX_XCFG_SET(B0, 3) |
                            SBX_XCFG_SET(B0, 4)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B1, 
                           (SBX_XCFG_SET(B1, 5) |
                            SBX_XCFG_SET(B1, 6) |
                            SBX_XCFG_SET(B1, 7) |
                            SBX_XCFG_SET(B1, 8) |
                            SBX_XCFG_SET(B1, 9)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B2, 
                           (SBX_XCFG_SET(B2, 10) |
                            SBX_XCFG_SET(B2, 11) |
                            SBX_XCFG_SET(B2, 12) |
                            SBX_XCFG_SET(B2, 13) |
                            SBX_XCFG_SET(B2, 14)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B3, 
                           (SBX_XCFG_SET(B3, 15) |
                            SBX_XCFG_SET(B3, 16) |
                            SBX_XCFG_SET(B3, 17) |
                            SBX_XCFG_SET(B3, 18) |
                            SBX_XCFG_SET(B3, 19)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B4, 
                           (SBX_XCFG_SET(B4, 20) |
                            SBX_XCFG_SET(B4, 21) |
                            SBX_XCFG_SET(B4, 22) |
                            SBX_XCFG_SET(B4, 23) |
                            SBX_XCFG_SET(B4, 24)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B5, 
                           (SBX_XCFG_SET(B5, 25) |
                            SBX_XCFG_SET(B5, 26) |
                            SBX_XCFG_SET(B5, 27) |
                            SBX_XCFG_SET(B5, 28) |
                            SBX_XCFG_SET(B5, 29)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B6, 
                           (SBX_XCFG_SET(B6, 30) |
                            SBX_XCFG_SET(B6, 31) |
                            SBX_XCFG_SET(B6, 32) |
                            SBX_XCFG_SET(B6, 33) |
                            SBX_XCFG_SET(B6, 34)));
            SAND_HAL_WRITE(unit, PT, XB_FIXED_XCFG_B7, 
                           (SBX_XCFG_SET(B7, 35) |
                            SBX_XCFG_SET(B7, 36) |
                            SBX_XCFG_SET(B7, 37) |
                            SBX_XCFG_SET(B7, 38) |
                            SBX_XCFG_SET(B7, 39)));



        }

        return BCM_E_NONE;
    }

    return BCM_E_UNIT;
}





STATIC int
_qe2000_sfi_rd_wr_xcfg( int unit, int nRead, bcm_port_t port,
                        int modid, bcm_port_t* pXbport )
{
    bcm_error_t rv = BCM_E_NONE;
    uint32 uRegData;

    /* if write action, write the xconfig to the data reg for indirect
     * memory write
     */
    if (nRead == 0)
    {
        uRegData = 
            SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, XCNFG, *pXbport);

        SAND_HAL_WRITE_STRIDE(unit, KA, SF, port, SF0_XCNFG_REMAP_MEM_ACC_DATA, 
                              uRegData);
    }

    uRegData = 
        ( SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, ACK, 1) |
          SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, REQ, 1) |
          SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, RD_WR_N, nRead) |
          SAND_HAL_SET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, XCNFG, modid) );    

    SAND_HAL_WRITE_STRIDE(unit, KA, SF, port, SF0_XCNFG_REMAP_MEM_ACC_CTRL, 
                          uRegData);

    /* wait for indirect memory access to complete */
    rv = _qe2000_sfi_xcnfg_mem_ack_wait( unit, port, BCM_XBAR_WAIT_XCNFG_TIME );

    if (BCM_SUCCESS(rv)) {
        uRegData = SAND_HAL_READ_STRIDE(unit, KA, SF, port, 
                                         SF0_XCNFG_REMAP_MEM_ACC_DATA);
        *pXbport = SAND_HAL_GET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_DATA, 
                                      XCNFG, uRegData);
    }

    return rv;
}


STATIC bcm_error_t
_qe2000_sfi_xcnfg_mem_ack_wait( int unit, bcm_port_t port, 
                                uint32 uMsTimeout )
{
    uint32 uRegData = 0;
    bcm_error_t rv = BCM_E_NONE;
    int i;

    for (i = 0; i < uMsTimeout; i++) {
        uRegData = SAND_HAL_READ_STRIDE(unit, KA, SF, port,
                                        SF0_XCNFG_REMAP_MEM_ACC_CTRL);
        
        if (SAND_HAL_GET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, ACK,
                               uRegData))
        {
            break;
        }
        sal_usleep(1000); /* sleep one millisecond */
    }

    /* Read complete, or we timed out */
    if (SAND_HAL_GET_FIELD(KA, SF0_XCNFG_REMAP_MEM_ACC_CTRL, 
                           ACK, uRegData) == 0 )
    {
        rv = BCM_E_TIMEOUT;
    }

    return rv;
}




