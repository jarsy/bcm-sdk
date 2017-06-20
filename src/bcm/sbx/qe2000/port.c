/*
 * $Id: port.c,v 1.57 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Port API
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_ka_auto.h>
#include <soc/sbx/qe2000.h>
#include <soc/sbx/qe2000_spi.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>

#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm/link.h>

#include <bcm_int/sbx/port.h>
#include <bcm_int/sbx/state.h>

#define UNICAST_0   1
#define UNICAST_1   2
#define MULTICAST_0   0x10
#define MULTICAST_1   0x20

#define MAX_QE2000_SHAPERS SB_FAB_DEVICE_QE2000_MAX_EGRESS_SHAPERS
#define MAX_QE2000_UNITS   SOC_MAX_NUM_DEVICES

#define QE2000_PORT_CHECK(unit, portNum)              \
  do {                                                \
         if ((!SOC_PBMP_PORT_VALID(portNum)) ||       \
             (!IS_ALL_PORT(unit, portNum))) {         \
             return BCM_E_PARAM;                      \
     }                                                \
  } while(0)

STATIC void
_qe2000_sfi_enable_set(int unit, int port, int enable)
{
    if (enable) {

        SAND_HAL_RMW_FIELD_STRIDE(unit, KA, SF, port,
                                  SF0_SI_DEBUG1, FORCE_SERDES_TX_LOW, (enable)?0:1);
        SAND_HAL_RMW_FIELD_STRIDE(unit, KA, SF, port,
                                  SF0_SI_CONFIG0, ENABLE, (enable)?1:0);
    } else {
        SAND_HAL_RMW_FIELD_STRIDE(unit, KA, SF, port,
                                  SF0_SI_CONFIG0, ENABLE, (enable)?1:0);

        SAND_HAL_RMW_FIELD_STRIDE(unit, KA, SF, port,
                                  SF0_SI_DEBUG1, FORCE_SERDES_TX_LOW, (enable)?0:1);
    }
}

STATIC void
_qe2000_sci_enable_set(int unit, int port, int enable)
{
    if (port == 0) {
        if (enable) {
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI0_DEBUG1, FORCE_SERDES_TX_LOW, (enable)?0:1);
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI0_CONFIG0, ENABLE, (enable)?1:0);
        } else {
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI0_CONFIG0, ENABLE, (enable)?1:0);
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI0_DEBUG1, FORCE_SERDES_TX_LOW, (enable)?0:1);
        }
    } else {
        if (enable) {
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI1_DEBUG1, FORCE_SERDES_TX_LOW, (enable)?0:1);
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI1_CONFIG0, ENABLE, (enable)?1:0);
        } else {
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI1_CONFIG0, ENABLE, (enable)?1:0);
            SAND_HAL_RMW_FIELD(unit, KA, SC_SI1_DEBUG1, FORCE_SERDES_TX_LOW, (enable)?0:1);
        }
    }
}

STATIC void
_qe2000_sfi_enable_get(int unit, int port, int *enable)
{
    uint32 val;

    val =  SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF,
				port, SF0_SI_DEBUG1);

    if (SAND_HAL_GET_FIELD(KA, SF0_SI_DEBUG1, FORCE_SERDES_TX_LOW, val)) {
        *enable = FALSE;
    } else {
        *enable = TRUE;
    }
}

STATIC void
_qe2000_sci_enable_get(int unit, int port, int *enable)
{
    uint32 val;

    if (port == 0) {
        val =  SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_DEBUG1);

        if (SAND_HAL_GET_FIELD(KA, SC_SI0_DEBUG1,
                           FORCE_SERDES_TX_LOW, val)) {

            *enable = FALSE;
        } else {
            *enable = TRUE;
        }
    } else {
        val =  SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_DEBUG1);

        if (SAND_HAL_GET_FIELD(KA, SC_SI1_DEBUG1,
                               FORCE_SERDES_TX_LOW, val)) {
            *enable = FALSE;
        } else {
            *enable = TRUE;
        }
    }
}

STATIC void
_qe2000_sfi_link_status_get(int unit, int port, int *up)
{
    uint32 status = 0;
    int    timeAligned     = FALSE;
    int    byteAligned     = FALSE;

    /* mark link as down */
    (*up) = FALSE;

    /* SDK-30949 */
    /* If the state indicates that we are not byte aligned regardless of sticky state,  */
    /* report link as down.  This is because the sticky state might be 0 if the link is */
    /* down and has been down continuously after being checked and cleared.             */
    status = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, port, SF0_SI_STATE);
    if ((SAND_HAL_GET_FIELD(KA, SF0_SI_STATE, MSM_RUN_BYTE_ALIGNMENT, status) == 1)) {
	*up = FALSE;
	return;
    }

    status = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, port, SF0_SI_STICKY_STATE);

    /* check Byte alignment */
    if (SAND_HAL_GET_FIELD(KA, SF0_SI_STICKY_STATE, S_MSM_RUN_TIME_ALIGN, status) == 1) {
        byteAligned = TRUE;
    
        /* consistency check */
        if (SAND_HAL_GET_FIELD(KA, SF0_SI_STICKY_STATE, S_MSM_RUN_BYTE_ALIGNMENT, status) == 1) {
            byteAligned = FALSE;
        }
    }

    /* check Time alignment */
    if ( (SAND_HAL_GET_FIELD(KA, SF0_SI_STICKY_STATE, S_TASM_IDLE, status) != 1) &&
	 (SAND_HAL_GET_FIELD(KA, SF0_SI_STICKY_STATE, S_TASM_SOT_SEARCH, status) != 1) &&
	 (SAND_HAL_GET_FIELD(KA, SF0_SI_STICKY_STATE, S_TASM_SOT_EARLY, status) != 1) &&
	 (SAND_HAL_GET_FIELD(KA, SF0_SI_STICKY_STATE, S_TASM_SOT_MISSING, status) != 1) ) {
        timeAligned = TRUE;
    }

    /* Clear sticky state for next read */
    SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, port, SF0_SI_STICKY_STATE, status);


    if (timeAligned != TRUE) {
        return;
    }
    if (byteAligned != TRUE) {
        return;
    }

    /* mark link as up */
    (*up) = TRUE;
    return;


}

STATIC void
_qe2000_sci_link_status_get(int unit, int port, int *up)
{
    uint32 status = 0;
    int    timeAligned     = FALSE;
    int    byteAligned     = FALSE;

    /* mark link as down */
    (*up) = FALSE;

    /* SDK-30949 */
    /* If the state indicates that we are not byte aligned regardless of sticky state,  */
    /* report link as down.  This is because the sticky state might be 0 if the link is */
    /* down and has been down continuously after being checked and cleared.             */
    if (port == 0) {
	status = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_STATE);
    } else {
	status = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_STATE);
    }
    if ((SAND_HAL_GET_FIELD(KA, SC_SI0_STATE, MSM_RUN_BYTE_ALIGNMENT, status) == 1)) {
	*up = FALSE;
	return;
    }


    if (port == 0) {
	status = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_STICKY_STATE);
    } else {
	status = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_STICKY_STATE);
    }

    /* check Byte alignment */
    if (SAND_HAL_GET_FIELD(KA, SC_SI0_STICKY_STATE, S_MSM_RUN_TIME_ALIGN, status) == 1) {
        byteAligned = TRUE;
    
        /* consistency check */
        if (SAND_HAL_GET_FIELD(KA, SC_SI0_STICKY_STATE, S_MSM_RUN_BYTE_ALIGNMENT, status) == 1) {
            byteAligned = FALSE;
        }
    }

    /* check Time alignment */
    if ( (SAND_HAL_GET_FIELD(KA, SC_SI0_STICKY_STATE, S_TASM_IDLE, status) != 1) &&
	 (SAND_HAL_GET_FIELD(KA, SC_SI0_STICKY_STATE, S_TASM_SOT_SEARCH, status) != 1) &&
	 (SAND_HAL_GET_FIELD(KA, SC_SI0_STICKY_STATE, S_TASM_SOT_EARLY, status) != 1) &&
	 (SAND_HAL_GET_FIELD(KA, SC_SI0_STICKY_STATE, S_TASM_SOT_MISSING, status) != 1) ) {
        timeAligned = TRUE;
    }

    /* Clear sticky state for next read */
    if (port == 0) {
	SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_STICKY_STATE, status);
    } else {
	SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_STICKY_STATE, status);
    }

    if (timeAligned != TRUE) {
        return;
    }
    if (byteAligned != TRUE) {
        return;
    }

    /* mark link as up */
    (*up) = TRUE;

    return;
}

STATIC void
_qe2000_sfi_loopback_set(int unit, int port, int loopback)
{
    uint32 uData;

    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF,
				 port, SF0_SI_DEBUG0);

    uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_DEBUG0, LPBK_EN, uData, loopback);

    SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF,
			  port, SF0_SI_DEBUG0, uData);
}

STATIC void
_qe2000_sci_loopback_set(int unit, int port, int loopback)
{
    uint32 uData;

    if (port == 0) {
        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_DEBUG0);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_DEBUG0, LPBK_EN, uData, loopback);
        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_DEBUG0, uData);

    } else {
        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_DEBUG0);
        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_DEBUG0, LPBK_EN, uData, loopback);
        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_DEBUG0, uData);
    }
}

STATIC void
_qe2000_sfi_loopback_get(int unit, int port, int *loopback)
{
    uint32 uData;

    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF,
				 port, SF0_SI_DEBUG0);
    *loopback = SAND_HAL_GET_FIELD(KA, SF0_SI_DEBUG0, LPBK_EN, uData);
}

STATIC void
_qe2000_sci_loopback_get(int unit, int port, int *loopback)
{
    uint32 uData;

    if (port == 0) {
        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_DEBUG0);
        *loopback = SAND_HAL_GET_FIELD(KA, SC_SI0_DEBUG0, LPBK_EN, uData);
    } else {
        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_DEBUG0);
        *loopback = SAND_HAL_GET_FIELD(KA, SC_SI1_DEBUG0, LPBK_EN, uData);
    }
}

STATIC void
_qe2000_port_id_set(int unit, int port, int id)
{
    uint32 uData;
    int port_reg_index;


    port_reg_index = port / 2;
    uData = SAND_HAL_READ_INDEX((sbhandle)unit, KA, EG_MC_SRC_ID_0, port_reg_index);

    id = id & SAND_HAL_KA_EG_MC_SRC_ID_0_P0_SRC_ID_MASK;
    if (port % 2 == 0) {
        uData = SAND_HAL_MOD_FIELD(KA, EG_MC_SRC_ID_0, P0_SRC_ID, uData, id);
    }
    else {
        uData = SAND_HAL_MOD_FIELD(KA, EG_MC_SRC_ID_0, P1_SRC_ID, uData, id);
    }
    SAND_HAL_WRITE_INDEX((sbhandle)unit, KA, EG_MC_SRC_ID_0, port_reg_index, uData);
}

STATIC void
_qe2000_port_id_get(int unit, int port, int *id)
{
    uint32 uData;
    int port_reg_index;


    port_reg_index = port / 2;
    uData = SAND_HAL_READ_INDEX((sbhandle)unit, KA, EG_MC_SRC_ID_0, port_reg_index);

    if (port % 2 == 0) {
        (*id) = SAND_HAL_GET_FIELD(KA, EG_MC_SRC_ID_0, P0_SRC_ID, uData);
    }
    else {
        (*id) = SAND_HAL_GET_FIELD(KA, EG_MC_SRC_ID_0, P1_SRC_ID, uData);
    }
}


STATIC int
qe2000_port_egress_multicast_pause_set(int unit, int port, int value)
{
    int rv = BCM_E_NONE;
    uint32 uConfigData;
    int adj_port;


    /* retreive current setting */
    if (port >= 32) {
        uConfigData = SAND_HAL_READ((sbhandle)unit, KA, EI_EG_PORTS_CONFIG1);
        adj_port = port - 32;
    }
    else {
        uConfigData = SAND_HAL_READ((sbhandle)unit, KA, EI_EG_PORTS_CONFIG0);
        adj_port = port;
    }

    /* modify current setting */
    if (value == FALSE) {
        uConfigData &= ~(1 << (adj_port));
    }
    else {
        uConfigData |= (1 << (adj_port));
    }

    /* configure modified setting */
    if (port >= 32) {
        SAND_HAL_WRITE((sbhandle)unit, KA, EI_EG_PORTS_CONFIG1, uConfigData);
    }
    else {
        SAND_HAL_WRITE((sbhandle)unit, KA, EI_EG_PORTS_CONFIG0, uConfigData);
    }

    return(rv);
}

STATIC int
qe2000_port_egress_multicast_pause_get(int unit, int port, int *value)
{
    int rv = BCM_E_NONE;
    uint32 uConfigData;
    int adj_port;


    if (port >= 32) {
        uConfigData = SAND_HAL_READ((sbhandle)unit, KA, EI_EG_PORTS_CONFIG1);
        adj_port = port - 32;
    }
    else {
        uConfigData = SAND_HAL_READ((sbhandle)unit, KA, EI_EG_PORTS_CONFIG0);
        adj_port = port;
    }

    (*value) = (uConfigData & (1 << (adj_port))) ? TRUE : FALSE;

    return(rv);
}

STATIC int
qe2000_port_egress_multicast_lossless_set(int unit, int port, int is_lossless)
{
    int rv = BCM_E_NONE;
    uint32 uEfData0, uEfData1, uEfData2;
    uint32 uNefData0, uNefData1, uNefData2;
    int is_mc_paused, i;


    if ( (is_lossless != TRUE) && (is_lossless != FALSE) ) {
        return(BCM_E_PARAM);
    }

    /* retreive pause multicast state of a port */
    qe2000_port_egress_multicast_pause_get(unit, port, &is_mc_paused);

    /* enable pause multicast on port */
    qe2000_port_egress_multicast_pause_set(unit, port, TRUE);

    /* delay - pause to get reflected to EG block */
    thin_delay(SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_DELAY);

    /* poll for read and write pointers to be equal */
    for (i = 0; i < SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_COUNT; i++) {
        rv = soc_qe2000_eg_mem_read(unit, port, 0x8, &uEfData0, &uEfData1, &uEfData2);
        if (rv != BCM_E_NONE) {
            goto err;
        }

        rv = soc_qe2000_eg_mem_read(unit, (SB_FAB_DEVICE_QE2000_MULTICAST_NEF_FIFO_OFFSET + port),
                                                          0x8, &uNefData0, &uNefData1, &uNefData2);
        if (rv != BCM_E_NONE) {
            goto err;
        }

        if ( (((uEfData0 >> 13) & 0x3F) == ((uEfData0 >> 19) & 0x3F)) &&
             (((uNefData0 >> 13) & 0x3F) == ((uNefData0 >> 19) & 0x3F)) ) {
            break;
        }

        sal_udelay(SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_DELAY);
    }

    if (i >= SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_COUNT) {
        rv = BCM_E_TIMEOUT;
        goto err;
    }

    /* configure lossless/lossy behaviour */
    if (is_lossless == TRUE) {
        uEfData0 &= ~(1 << 25);
        uNefData0 &= ~(1 << 25);
    }
    else {
        uEfData0 |= (1 << 25);
        uNefData0 |= (1 << 25);
    }
    rv = soc_qe2000_eg_mem_write(unit, port, 0x8, uEfData0, uEfData1, uEfData2);
    rv = soc_qe2000_eg_mem_write(unit, (SB_FAB_DEVICE_QE2000_MULTICAST_NEF_FIFO_OFFSET + port),
                                                         0x8, uNefData0, uNefData1, uNefData2);

    /* restore pause multicast state of a port */
    if (is_mc_paused == FALSE) {
        qe2000_port_egress_multicast_pause_set(unit, port, FALSE);
    }

    return(rv);

err:
    if (is_mc_paused == FALSE) {
        qe2000_port_egress_multicast_pause_set(unit, port, FALSE);
    }

    return(rv);
}

STATIC int
qe2000_port_egress_multicast_lossless_get(int unit, int port, int *is_lossless)
{
    int rv = BCM_E_NONE;
    uint32 uEfData0, uEfData1, uEfData2;
    uint32 uNefData0, uNefData1, uNefData2;
    int is_ef_lossless, is_nef_lossless;


    /* retreive ef state */
    rv = soc_qe2000_eg_mem_read(unit, port, 0x8, &uEfData0, &uEfData1, &uEfData2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }
    is_ef_lossless = (uEfData0 & (1 << 25)) ? FALSE : TRUE;

    /* retreive nef state */
    rv = soc_qe2000_eg_mem_read(unit, (SB_FAB_DEVICE_QE2000_MULTICAST_NEF_FIFO_OFFSET + port),
                                                       0x8, &uNefData0, &uNefData1, &uNefData2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }
    is_nef_lossless = (uNefData0 & (1 << 25)) ? FALSE : TRUE;

    /* consistency check */
    if (is_ef_lossless != is_nef_lossless) {
        return(BCM_E_INTERNAL);
    }

    (*is_lossless) = is_ef_lossless;

    return(rv);
}

static int
_qe2000_spi_subport_info_get(int unit, bcm_port_t port, int *spi, int *spi_port)
{
    int rv = BCM_E_NONE;

    (*spi) = SOC_PORT_BLOCK_NUMBER(unit, port);
    (*spi_port) = SOC_PORT_BLOCK_INDEX(unit, port);

    return(rv);
}

static int
_qe2000_spi_subport_enable_set(int unit, bcm_port_t port, int enable)
{
    int rv = BCM_E_NONE;
    int spi, spi_port;
    uint32 uData;


    _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);

    /* retreive current configuration */
    if (spi == 0) {
        if (spi_port < 32) {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI0_CONFIG1);
        }
        else {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI0_CONFIG2);
        }
    }
    else { /* spi == 1 */
        if (spi_port < 32) {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI1_CONFIG1);
        }
        else {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI1_CONFIG2);
        }
    }

    /* update configuration */
    if (enable == TRUE) {
        /* coverity[large_shift : FALSE] */
        uData |= (1 << spi_port);
    }
    else {
        /* coverity[large_shift : FALSE] */
        uData &= ~(1 << spi_port);
    }

    /* set configuration */
    if (spi == 0) {
        if (spi_port < 32) {
            SAND_HAL_WRITE((sbhandle)unit, KA, EI_SPI0_CONFIG1, uData);
        }
        else {
            SAND_HAL_WRITE((sbhandle)unit, KA, EI_SPI0_CONFIG2, uData);
        }
    }
    else { /* spi == 1 */
        if (spi_port < 32) {
            SAND_HAL_WRITE((sbhandle)unit, KA, EI_SPI1_CONFIG1, uData);
        }
        else {
            SAND_HAL_WRITE((sbhandle)unit, KA, EI_SPI1_CONFIG2, uData);
        }
    }

    return(rv);
}

static int
_qe2000_spi_subport_enable_get(int unit, bcm_port_t port, int *enable)
{
    int rv = BCM_E_NONE;
    int spi, spi_port;
    uint32 uData;


    _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);

    /* retreive current configuration */
    if (spi == 0) {
        if (spi_port < 32) {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI0_CONFIG1);
        }
        else {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI0_CONFIG2);
        }
    }
    else { /* spi == 1 */
        if (spi_port < 32) {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI1_CONFIG1);
        }
        else {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI1_CONFIG2);
        }
    }

    /* update configuration */
    /* coverity[large_shift : FALSE] */
    (*enable) = (((uData & (1 << spi_port)) >> spi_port) == TRUE) ? TRUE : FALSE;

    return(rv);
}

static int
_qe2000_spi_subport_flush(int unit, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    int spi, spi_port, is_flushed, i;
    uint32 uData = 0;


    _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);

    uData = SAND_HAL_SET_FIELD(KA, EI_SPI0_FLUSH, REQ, 1) |
            SAND_HAL_SET_FIELD(KA, EI_SPI0_FLUSH, ACK, 1) |
            SAND_HAL_SET_FIELD(KA, EI_SPI0_FLUSH, PORT, spi_port);

    if (spi == 0) {
        SAND_HAL_WRITE((sbhandle)unit, KA, EI_SPI0_FLUSH, uData);
    }
    else { /* spi == 1 */
        SAND_HAL_WRITE((sbhandle)unit, KA, EI_SPI1_FLUSH, uData);
    }


    is_flushed = FALSE;
    thin_delay(SB_FAB_DEVICE_QE2000_PORT_FLUSH_DELAY);

    for (i = 0; i < SB_FAB_DEVICE_QE2000_PORT_FLUSH_ITER_COUNT; i++) {
        if (spi == 0) {
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI0_FLUSH);
        }
        else { /* spi == 1 */
            uData = SAND_HAL_READ((sbhandle)unit, KA, EI_SPI1_FLUSH);
        }


        is_flushed = SAND_HAL_GET_FIELD(KA, EI_SPI0_FLUSH, ACK, uData);
        if (is_flushed == 1) {
            break;
        }

        sal_udelay(SB_FAB_DEVICE_QE2000_PORT_FLUSH_ITER_DELAY);
    }

    if (i >= SB_FAB_DEVICE_QE2000_PORT_FLUSH_ITER_COUNT) {
        return(BCM_E_TIMEOUT);
    }

    return(rv);
}

static int
_qe2000_spi_subport_init(int unit, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    int spi, spi_port, nbr_ports;
    qe2000_ei_port_config_t *port_config = NULL;
    int port_nbr;
    uint32 uData;


    _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);

    /* allocate data structures */
    port_config = sal_alloc((sizeof(qe2000_ei_port_config_t) * SB_FAB_DEVICE_QE2000_MAX_PORT),
                                                                             "ei port info");
    if (port_config == NULL) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "Unable to allocate memory for ei port configuration")));
        return(BCM_E_MEMORY);
    }

     nbr_ports = SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0] +
                         SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[1];

    /* Read configuration */
    for (port_nbr = 0; port_nbr < nbr_ports; port_nbr++) {
        soc_qe2000_ei_mem_read(unit, 0x2, port_nbr, &uData);
        (port_config + port_nbr)->size_mask = 
                          (SAND_HAL_GET_FIELD(KA, EI_MEM_ACC_DATA, SIZE_MASK8, uData) << 8) |
                          SAND_HAL_GET_FIELD(KA, EI_MEM_ACC_DATA, SIZE_MASK7_0, uData);
        (port_config + port_nbr)->byte_ptr =
                          SAND_HAL_GET_FIELD(KA, EI_MEM_ACC_DATA, BYTE_PTR, uData);

        if (port_nbr < SOC_SBX_CFG_QE2000(unit)->uNumPhySpiPorts[0]) {
            soc_qe2000_ei_mem_read(unit, 0x1, port_nbr, &uData);
        }
        else {
            soc_qe2000_ei_mem_read(unit, 0x0, port_nbr, &uData);
        }
        (port_config + port_nbr)->dest_channel = 
                          SAND_HAL_GET_FIELD(KA, EI_MEM_ACC_DATA, DEST_CHANNEL, uData);
        (port_config + port_nbr)->is_rb_loopback =
                          SAND_HAL_GET_FIELD(KA, EI_MEM_ACC_DATA, RB_LOOPBACK_ONLY, uData);
        (port_config + port_nbr)->line_ptr =
                          SAND_HAL_GET_FIELD(KA, EI_MEM_ACC_DATA, LINE_PTR, uData);
    }

    /* update configuration */
    /* NOTE: This step is currently not done as the configuration is read from */
    /*       the "other" SPI interface.                                        */

    /* write back configuration */
    uData=0;
    uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, DEST_CHANNEL, uData,
                                                       (port_config + port)->dest_channel);
    uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, SIZE_MASK8, uData,
                                                       (port_config + port)->size_mask >> 8);
    uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, SIZE_MASK7_0, uData,
                                                       (port_config + port)->size_mask & 0xFF);
    uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, RB_LOOPBACK_ONLY, uData,
                                                       (port_config + port)->is_rb_loopback);
    uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, LINE_PTR, uData,
                                                       (port_config + port)->line_ptr);
    uData = SAND_HAL_MOD_FIELD(KA, EI_MEM_ACC_DATA, BYTE_PTR, uData, 0);
    soc_qe2000_ei_mem_write(unit, 1, port_nbr, uData);
    soc_qe2000_ei_mem_write(unit, 2, port_nbr, uData);
    soc_qe2000_ei_mem_write(unit, 3, port_nbr, uData);

    sal_free(port_config);

    return(rv);

#if 0
err;

    if (port_config != NULL) {
        sal_free(port_config);
    }

    return(rv);
#endif /* 0 */
}

static int
_bcm_qe2000_port_flush(int unit, bcm_port_t port)
{
    int rv = BCM_E_NONE;
    int is_enabled;
    int spi, spi_port;


    /* check that functionality is supported in the configured mode */
    /* NOTE: should never reach here as this check is done before   */
    /*       invoking this function.                                */
    _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);
    if (SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[spi] != SOC_SBX_PORT_MODE_BURST_IL) {
        return(BCM_E_UNAVAIL);
    }

    /* retreive current configuration */
    _qe2000_spi_subport_enable_get(unit, port, &is_enabled);

    /* disable port */
    _qe2000_spi_subport_enable_set(unit, port, FALSE);

    /* flush port and wait for flush to finish */
    rv = _qe2000_spi_subport_flush(unit, port);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    /* re-initialize FIFO parameters */
    _qe2000_spi_subport_init(unit, port);

    /* restore port enable/disable state */
    if (is_enabled == TRUE) {
        _qe2000_spi_subport_enable_set(unit, port, TRUE);
    }

    return(rv);

err:
    /* re-initialize FIFO parameters */
    _qe2000_spi_subport_init(unit, port);

    /* restore port enable/disable state */
    if (is_enabled == TRUE) {
        _qe2000_spi_subport_enable_set(unit, port, TRUE);
    }

    return(rv);
}


int
bcm_qe2000_port_init(int unit)
{

    int rv = BCM_E_NONE;
    SOC_SBX_STATE(unit)->port_state->fabric_header_format = BCM_PORT_CONTROL_FABRIC_HEADER_83200;
    return rv;

}


int
bcm_qe2000_port_enable_set(int unit, bcm_port_t port, int enable)
{
    int rv = BCM_E_NONE;
    int spi, spi_port;


    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port)) {
        _qe2000_sfi_enable_set(unit, SOC_PORT_BLOCK_INDEX(unit, port), enable);
    } else if (IS_SCI_PORT(unit, port)) {
        _qe2000_sci_enable_set(unit, SOC_PORT_BLOCK_INDEX(unit, port), enable);
    } else if (IS_SPI_PORT(unit, port)) {
        hwQe2000Spi4RxEnable(SOC_SBX_SBHANDLE(unit), enable,
                             SOC_PORT_BLOCK_NUMBER(unit, port));
        hwQe2000Spi4TxEnable(SOC_SBX_SBHANDLE(unit), enable,
                             SOC_PORT_BLOCK_NUMBER(unit, port));
    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {

        

        /* check that functionality is supported in the configured mode */
        _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);
        if (SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[spi] != SOC_SBX_PORT_MODE_BURST_IL) {
            rv = BCM_E_UNAVAIL;
        }
        else {
            if (enable == TRUE) {
                /* Flush Port */
                _bcm_qe2000_port_flush(unit, port);

                /* Enable Port */
                _qe2000_spi_subport_enable_set(unit, port, enable);
            }
            else { /* (enable == FALSE) */
               /* Disable Port */
                _qe2000_spi_subport_enable_set(unit, port, enable);

               /* Flush Port */
               _bcm_qe2000_port_flush(unit, port);
            }
        
            rv = BCM_E_NONE;
        }

    } else {
        /*
         * No support for enable/disable spi subports and cpu port
         */
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}


int
bcm_qe2000_port_ability_get(int unit, bcm_port_t port,
                            bcm_port_abil_t *ability_mask)
{
    int rv = BCM_E_NONE;

    
    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {

    } else if (IS_SPI_PORT(unit, port)) {

    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {

    } else {
        rv = BCM_E_PORT;
    }

    return rv;

}

int
bcm_qe2000_port_enable_get(int unit, bcm_port_t port, int *enable)
{
    int rv = BCM_E_NONE;

    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port)) {
        _qe2000_sfi_enable_get(unit, SOC_PORT_BLOCK_INDEX(unit, port), enable);
    } else if (IS_SCI_PORT(unit, port)) {
        _qe2000_sci_enable_get(unit, SOC_PORT_BLOCK_INDEX(unit, port), enable);
    } else if (IS_SPI_PORT(unit, port)) {

        uint32 tx_ena, rx_ena;

        hwQe2000Spi4RxEnableGet(SOC_SBX_SBHANDLE(unit), &rx_ena,
                                SOC_PORT_BLOCK_NUMBER(unit, port));

        hwQe2000Spi4TxEnableGet(SOC_SBX_SBHANDLE(unit), &tx_ena,
                                SOC_PORT_BLOCK_NUMBER(unit, port));
        *enable = tx_ena && rx_ena;
    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {
        /* retreive port enable/disable status */
        _qe2000_spi_subport_enable_get(unit, port, enable);
    } else {
        /*
         * spi ports and cpu port are always enabled
         */
        *enable = TRUE;
    }

    return rv;
}

int
bcm_qe2000_port_speed_set(int unit, bcm_port_t port, int speed)
{
    int rv = BCM_E_UNAVAIL;
    /* Port speed fixed on QE2000 */
    return rv;
}

int
bcm_qe2000_port_speed_get(int unit, bcm_port_t port, int *speed)
{
    int rv = BCM_E_NONE;

    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port) || IS_SCI_PORT(unit, port)) {
	if (SOC_SBX_CFG_QE2000(unit)->bSv2_5GbpsLinks) {
	    *speed = 2500;         /* 2.5Gbps link   */
	} else {
	    *speed = 3125;         /* 3.125Gbps link */
	}

    } else if (IS_SPI_PORT(unit, port)) {
        *speed = 0;

    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {
        /* SPI port 1Gbps or 10Gbps, if more than 1 subport
         * on the SPI interface, assumes 1Gbps, otherwise
         * assumes 10Gbps
         */
        int spi;

        if ((port = SOC_PORT_BLOCK_INDEX(unit, port)) <
            SOC_SBX_CONTROL(unit)->spi_subport_count[0])
        {
            spi = 0;
        } else {
            spi = 1;
	    port -= SOC_SBX_CONTROL(unit)->spi_subport_count[0];
        }

	*speed = SOC_SBX_CFG_QE2000(unit)->uSpiSubportSpeed[spi][port];

    } else {
	/* No support for cpu port speed get */
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

int
bcm_qe2000_port_link_get(int unit, bcm_port_t port, int *up)
{
    int rv = BCM_E_NONE;
    soc_persist_t *sop = SOC_PERSIST(unit);

    QE2000_PORT_CHECK(unit, port);


    if (IS_SFI_PORT(unit, port)) {
        _qe2000_sfi_link_status_get(unit, SOC_PORT_BLOCK_INDEX(unit, port), up);

	if (*up) {
	    SOC_PBMP_PORT_ADD(sop->lc_pbm_link, port);
	} else {
	    SOC_PBMP_PORT_REMOVE(sop->lc_pbm_link, port);
	}

    } else if (IS_SCI_PORT(unit, port)) {
        _qe2000_sci_link_status_get(unit, SOC_PORT_BLOCK_INDEX(unit, port), up);

	if (*up) {
	    SOC_PBMP_PORT_ADD(sop->lc_pbm_link, port);
	} else {
	    SOC_PBMP_PORT_REMOVE(sop->lc_pbm_link, port);
	}

    } else if (IS_SPI_PORT(unit, port)) {

        if (hwQe2000Spi4RxStatus(SOC_SBX_SBHANDLE(unit),
                                 SOC_PORT_BLOCK_NUMBER(unit, port)))
        {
            *up = FALSE;
        } else {
            *up = TRUE;
        }

    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {

        /*
         * SPI subport and cpu port are always up
         */
        *up = TRUE;
    } else {
        rv = BCM_E_PORT;
    }

    return rv;
}

int
bcm_qe2000_port_link_status_get(int unit, bcm_port_t port, int *up)
{
    soc_persist_t *sop = SOC_PERSIST(unit);

    QE2000_PORT_CHECK(unit, port);

    /* If linkscan maintains the state, return the saved state, otherwise read the hw state */
    if (bcm_linkscan_enable_port_get(unit, port) == BCM_E_DISABLED) {

	return bcm_qe2000_port_link_get(unit, port, up);

    } else {
	*up = (SOC_PBMP_MEMBER(sop->lc_pbm_link, port)) ?
	    BCM_PORT_LINK_STATUS_UP : BCM_PORT_LINK_STATUS_DOWN;
    }
    return(BCM_E_NONE);
}

int
bcm_qe2000_port_loopback_set(int unit, bcm_port_t port, int loopback)
{
    int rv = BCM_E_NONE;

    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port)) {
        _qe2000_sfi_loopback_set(unit, SOC_PORT_BLOCK_INDEX(unit, port),
				 (loopback!=BCM_PORT_LOOPBACK_NONE)?1:0);
    } else if (IS_SCI_PORT(unit, port)) {
        _qe2000_sci_loopback_set(unit, SOC_PORT_BLOCK_INDEX(unit, port),
				 (loopback!=BCM_PORT_LOOPBACK_NONE)?1:0);
    } else {
        /*
         * No support for SPI subport and cpu port loopback
         */
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

int
bcm_qe2000_port_loopback_get(int unit, bcm_port_t port, int *loopback)
{

    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port)) {
        _qe2000_sfi_loopback_get(unit, SOC_PORT_BLOCK_INDEX(unit, port), loopback);
    } else if (IS_SCI_PORT(unit, port)) {
        _qe2000_sci_loopback_get(unit, SOC_PORT_BLOCK_INDEX(unit, port), loopback);
    } else {
        /*
         * SPI subport and cup port are never loopbacked
         */
        *loopback = FALSE;
    }

    return BCM_E_NONE;
}

int
bcm_qe2000_port_control_set(int unit, bcm_port_t port,
                            bcm_port_control_t type, int value)
{
    uint32 uData;
    int nSfiLink, nSciLink, nSpiPort;
    int rv = BCM_E_NONE;
    int num_port;
    int module_id;

    if (type == bcmPortControlFabricHeaderFormat) {
	switch (value) {
	    case BCM_PORT_CONTROL_FABRIC_HEADER_83200:
		break;
	    case BCM_PORT_CONTROL_FABRIC_HEADER_88230:
		break;
	    case BCM_PORT_CONTROL_FABRIC_HEADER_88020_83200_88230_INTEROP:
		break;
	    case BCM_PORT_CONTROL_FABRIC_HEADER_CUSTOM:
		break;
	    default:
		return BCM_E_UNAVAIL;
	}
	SOC_SBX_STATE(unit)->port_state->fabric_header_format = value;
	rv = BCM_E_UNAVAIL;
	return rv;
    }

    if (((type == bcmPortControlFabricKnockoutId) &&
                               (BCM_GPORT_IS_LOCAL(port) || BCM_GPORT_IS_MODPORT(port)))) {
        if (BCM_GPORT_IS_LOCAL(port)) {
            num_port = BCM_GPORT_LOCAL_GET(port);
            if  ( (num_port >= SOC_PORT_MIN(unit, spi_subport)) &&
                                       (num_port <= SOC_PORT_MAX(unit, spi_subport)) ) {
                num_port = num_port - SOC_PORT_MIN(unit, spi_subport);
            }
            else {
                return(BCM_E_PARAM);
            }

	    _qe2000_port_id_set(unit, num_port, value);
        }
        else if (BCM_GPORT_IS_MODPORT(port)) {
            bcm_qe2000_stk_modid_get(unit, &module_id);
            if (BCM_GPORT_MODPORT_MODID_GET(port) != module_id) {
                return(BCM_E_NONE);
            }

            num_port = BCM_GPORT_MODPORT_PORT_GET(port);
            if  ( (num_port >= SOC_PORT_MIN(unit, spi_subport)) &&
                                   (num_port <= SOC_PORT_MAX(unit, spi_subport)) ) {
                num_port = num_port - SOC_PORT_MIN(unit, spi_subport);
            }
            else {
                return(BCM_E_PARAM);
            }

	    _qe2000_port_id_set(unit, num_port, value);
        }

        return(rv);
    }

    if (BCM_GPORT_IS_MODPORT(port)) {
        port = BCM_GPORT_MODPORT_PORT_GET(port);
    }

    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port)) {
        nSfiLink = SOC_PORT_BLOCK_INDEX(unit, port);
        switch (type) {
            case bcmPortControlPrbsPolynomial:
                if ((value != BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1) &&
                    (value != BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1)) {
                    return BCM_E_PARAM;
                }
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_POLY_SEL, uData, value);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
                break;
            case bcmPortControlPrbsTxInvertData:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_INVERT, uData, (value)?1:0);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
                break;
            case bcmPortControlPrbsForceTxError:
		/* Read the SF0_SI_PRBS_STATUS register will clear the error count.
		   We retrieve the state from the stored software state. This is
		   Assuming all other fields are read only
		*/
		uData = 0;
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_PRBS_STATUS, FORCE_ERROR, uData, (value)?1:0);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS, uData);
		/* store the force tx state */
		SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[nSfiLink] =  (value)?1:0;
                break;
            case bcmPortControlPrbsTxEnable:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_GENERATOR_EN, uData, (value)?1:0);
                SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
                break;
            case bcmPortControlPrbsRxEnable:
                if (value) {
                    /* Enable PRBS, disable normal rx path */
                    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, ENABLE, uData, 0);
                    SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);

                    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_MONITOR_EN, uData, 1);
                    SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);

		    /* Read to clear prbs_err_cnt */
		    SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS);
                } else {
                    /* Disable PRBS, enable normal rx path */
                    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, PRBS_MONITOR_EN, uData, 0);
                    SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);

                    uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SF0_SI_CONFIG0, ENABLE, uData, 1);
                    SAND_HAL_WRITE_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0, uData);
                }
                break;
            case bcmPortControlSerdesDriverStrength:
                if ( (value > 100) || (value < 0)) {
                    return BCM_E_PARAM;
                }
                SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSfiLink].uDriverStrength = value;
                soc_qe2000_config_linkdriver(unit, nSfiLink, &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSfiLink]));
                break;
            case bcmPortControlSerdesDriverEqualization:
                if ( (value > 100) || (value < 0)) {
                    return BCM_E_PARAM;
                }
                SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSfiLink].uDriverEqualization = value;
                soc_qe2000_config_linkdriver(unit, nSfiLink, &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSfiLink]));
                break;
            case bcmPortControlAbility:
                /* SCI/SFI is fixed for QE2000 and no multiplexing is supported */
                return BCM_E_UNAVAIL;
            default:
                return BCM_E_UNAVAIL;
        }
    } else if (IS_SCI_PORT(unit, port)) {
	nSciLink = SOC_PORT_BLOCK_INDEX(unit, port);
        switch (type) {
            case bcmPortControlPrbsPolynomial:
                if ((value != BCM_PORT_PRBS_POLYNOMIAL_X7_X6_1) &&
                    (value != BCM_PORT_PRBS_POLYNOMIAL_X15_X14_1)) {
                    return BCM_E_PARAM;
                }
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_POLY_SEL, uData, value);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_POLY_SEL, uData, value);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);
                }
                break;
            case bcmPortControlPrbsTxInvertData:
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_INVERT, uData, (value)?1:0);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_INVERT, uData, (value)?1:0);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);
                }
                break;
            case bcmPortControlPrbsForceTxError:
		/* Read the SC_SI_PRBS_STATUS register will clear the error count.
		   We retrieve the state from the stored software state. This is
		   Assuming all other fields are read only
		*/
                if (nSciLink == 0) {
                    uData = 0;
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_PRBS_STATUS, FORCE_ERROR, uData, (value)?1:0);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_PRBS_STATUS, uData);
                } else {
                    uData = 0;
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_PRBS_STATUS, FORCE_ERROR, uData, (value)?1:0);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_PRBS_STATUS, uData);
                }
		/* store the force tx state */
		SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[SB_FAB_DEVICE_QE2000_SFI_LINKS+nSciLink] =  (value)?1:0;
                break;
            case bcmPortControlPrbsTxEnable:
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_GENERATOR_EN, uData, (value)?1:0);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_GENERATOR_EN, uData, (value)?1:0);
                    SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);
                }
                break;
            case bcmPortControlPrbsRxEnable:
                if (nSciLink == 0) {
                    /* Read to clear prbs_err_cnt */
                    SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_PRBS_STATUS);
                    if (value) {
                        /* Enable PRBS, disable normal rx path */
                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, ENABLE, uData, 0 /* disable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);

                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_MONITOR_EN, uData, 1 /* enable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);
                    } else {
                        /* Disable PRBS, enable normal rx path */
                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, PRBS_MONITOR_EN, uData, 0 /* disable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);

                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI0_CONFIG0, ENABLE, uData, 1 /* enable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI0_CONFIG0, uData);
                    }
                } else {
                    /* Read to clear prbs_err_cnt */
                    SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_PRBS_STATUS);
                    if (value) {
                        /* Enable PRBS, disable normal rx path */
                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, ENABLE, uData, 0 /* disable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);

                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_MONITOR_EN, uData, 1 /* enable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);
                    }
                    else {
                        /* Disable PRBS, enable normal rx path */
                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, PRBS_MONITOR_EN, uData, 0 /* disable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);

                        uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                        uData = SAND_HAL_MOD_FIELD(KA, SC_SI1_CONFIG0, ENABLE, uData, 1 /* enable */);
                        SAND_HAL_WRITE((sbhandle)unit, KA, SC_SI1_CONFIG0, uData);
                    }
                }
                break;
            case bcmPortControlSerdesDriverStrength:
                if ( (value > 100) || (value < 0)) {
                    return BCM_E_PARAM;
                }
                nSciLink += SB_FAB_DEVICE_QE2000_SFI_LINKS;
                SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSciLink].uDriverStrength = value;
                soc_qe2000_config_linkdriver(unit, nSciLink, &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSciLink]));
                break;
            case bcmPortControlSerdesDriverEqualization:
                if ( (value > 100) || (value < 0)) {
                    return BCM_E_PARAM;
                }
                nSciLink += SB_FAB_DEVICE_QE2000_SFI_LINKS;
                SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSciLink].uDriverEqualization = value;
                soc_qe2000_config_linkdriver(unit, nSciLink, &(SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSciLink]));
                break;
            case bcmPortControlAbility:
                /* SCI/SFI is fixed for QE2000 and no multiplexing is supported */
                return BCM_E_UNAVAIL;
            default:
                return BCM_E_UNAVAIL;
        }
    } else if (IS_SPI_PORT(unit, port)) {
        nSpiPort = SOC_PORT_BLOCK_INDEX(unit, port);
        switch (type) {
            case bcmPortControlTrain:
                if ( (value != TRUE) && (value != FALSE)) {
                    rv = BCM_E_PARAM;
                }
                else {
                    (value == TRUE) ? hwQe2000Spi4TxForceTrainingOn((sbhandle)unit, nSpiPort) :
                                      hwQe2000Spi4TxForceTrainingOff((sbhandle)unit, nSpiPort);
                }
                break;

            case bcmPortControlRxEnable:
                if ( (value != TRUE) && (value != FALSE)) {
                    rv = BCM_E_PARAM;
                }
                else {
                    hwQe2000Spi4RxEnable((sbhandle)unit, value, nSpiPort);
                }
                break;

            case bcmPortControlTxEnable:
                if ( (value != TRUE) && (value != FALSE)) {
                    rv = BCM_E_PARAM;
                }
                else {
                    hwQe2000Spi4TxEnable((sbhandle)unit, value, nSpiPort);
                }
                break;

            case bcmPortControlRxLink:
                rv = BCM_E_UNAVAIL;
                break;

            case bcmPortControlTxLink:
                rv = BCM_E_UNAVAIL;
                break;

            default:
                rv = BCM_E_UNAVAIL;
                break;
        }
    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {
        switch (type) {
            case bcmPortControlFabricKnockoutId:
                _qe2000_port_id_set(unit, port, value);
                break;

            case bcmPortControlEgressNonUnicastLossless:
                rv = qe2000_port_egress_multicast_lossless_set(unit, port, value);
                break;

            default:
                rv = BCM_E_UNAVAIL;
                break;
        }
    } else {
        /*
         * Port control doesn't apply to cpu port
         */
        rv = BCM_E_UNAVAIL;
    }
    return rv;
}

int
bcm_qe2000_port_control_get(int unit, bcm_port_t port,
                            bcm_port_control_t type, int *value)
{
    uint32 uData;
    int nSfiLink, nSciLink, nSpiPort;
    int rv = BCM_E_NONE;
    int spi_port;
    int num_port;
    int module_id;

    if (type == bcmPortControlFabricHeaderFormat) {
	*value = SOC_SBX_STATE(unit)->port_state->fabric_header_format;
	rv = BCM_E_UNAVAIL;
	return rv;
    }

    if (((type == bcmPortControlFabricKnockoutId) &&
                               (BCM_GPORT_IS_LOCAL(port) || BCM_GPORT_IS_MODPORT(port)))) {
        if (BCM_GPORT_IS_LOCAL(port)) {
            num_port = BCM_GPORT_LOCAL_GET(port);
            if  ( (num_port >= SOC_PORT_MIN(unit, spi_subport)) &&
                                       (num_port <= SOC_PORT_MAX(unit, spi_subport)) ) {
                num_port = num_port - SOC_PORT_MIN(unit, spi_subport);
            }
            else {
                return(BCM_E_PARAM);
            }

	    _qe2000_port_id_get(unit, num_port, value);
        } else if (BCM_GPORT_IS_MODPORT(port)) {
            bcm_qe2000_stk_modid_get(unit, &module_id);
            if (BCM_GPORT_MODPORT_MODID_GET(port) != module_id) {
                return(BCM_E_UNAVAIL);
            }

            num_port = BCM_GPORT_MODPORT_PORT_GET(port);
            if  ( (num_port >= SOC_PORT_MIN(unit, spi_subport)) &&
                                   (num_port <= SOC_PORT_MAX(unit, spi_subport)) ) {
                num_port = num_port - SOC_PORT_MIN(unit, spi_subport);
            }
            else {
                return(BCM_E_PARAM);
            }

	    _qe2000_port_id_get(unit, num_port, value);
        }
        return rv;
    }

    QE2000_PORT_CHECK(unit, port);

    if (IS_SFI_PORT(unit, port)) {
        nSfiLink = SOC_PORT_BLOCK_INDEX(unit, port);
        switch (type) {
            case bcmPortControlPrbsPolynomial:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                *value = SAND_HAL_GET_FIELD(KA, SF0_SI_CONFIG0, PRBS_POLY_SEL, uData);
                break;
            case bcmPortControlPrbsTxInvertData:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                *value = SAND_HAL_GET_FIELD(KA, SF0_SI_CONFIG0, PRBS_INVERT, uData);
                break;
            case bcmPortControlPrbsForceTxError:
		/* Read the SF0_SI_PRBS_STATUS register will clear the error count.
		   We retrieve the state from the stored software state
		   uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS);
		   *value = SAND_HAL_GET_FIELD(KA, SF0_SI_PRBS_STATUS, FORCE_ERROR, uData);
		 */
		*value = SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[nSfiLink];
                break;
            case bcmPortControlPrbsTxEnable:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                *value = SAND_HAL_GET_FIELD(KA, SF0_SI_CONFIG0, PRBS_GENERATOR_EN, uData);
                break;
            case bcmPortControlPrbsRxEnable:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_CONFIG0);
                *value = SAND_HAL_GET_FIELD(KA, SF0_SI_CONFIG0, PRBS_MONITOR_EN, uData);
                break;
            case bcmPortControlPrbsRxStatus:
                uData = SAND_HAL_READ_STRIDE((sbhandle)unit, KA, SF, nSfiLink, SF0_SI_PRBS_STATUS);
                *value = SAND_HAL_GET_FIELD(KA, SF0_SI_PRBS_STATUS, PRBS_ERR_CNT, uData);
                break;
            case bcmPortControlSerdesDriverStrength:
                /* Return the cached value instead of recontruct from hardware */
                *value = SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSfiLink].uDriverStrength;
                break;
            case bcmPortControlSerdesDriverEqualization:
                /* Return the cached value instead of reconstruct from hardware */
                *value = SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSfiLink].uDriverEqualization;
                break;
            case bcmPortControlAbility:
                /* SCI/SFI is fixed for QE2000 and no multiplexing is supported */
                *value = BCM_PORT_ABILITY_SFI;
                break;
            default:
                return BCM_E_UNAVAIL;
        }
    } else if (IS_SCI_PORT(unit, port)) {
	nSciLink = SOC_PORT_BLOCK_INDEX(unit, port);
        switch (type) {
            case bcmPortControlPrbsPolynomial:
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI0_CONFIG0, PRBS_POLY_SEL, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI1_CONFIG0, PRBS_POLY_SEL, uData);
                }
                break;
            case bcmPortControlPrbsTxInvertData:
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI0_CONFIG0, PRBS_INVERT, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI1_CONFIG0, PRBS_INVERT, uData);
                }
                break;
            case bcmPortControlPrbsForceTxError:

		    *value = SOC_SBX_STATE(unit)->port_state->uPrbsForceTxError[SB_FAB_DEVICE_QE2000_SFI_LINKS+nSciLink];

                break;
            case bcmPortControlPrbsTxEnable:
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI0_CONFIG0, PRBS_GENERATOR_EN, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI1_CONFIG0, PRBS_GENERATOR_EN, uData);
                }
                break;
            case bcmPortControlPrbsRxEnable:
                if (nSciLink == 0) {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI0_CONFIG0, PRBS_MONITOR_EN, uData);
                } else {
                    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_CONFIG0);
                    *value = SAND_HAL_GET_FIELD(KA, SC_SI1_CONFIG0, PRBS_MONITOR_EN, uData);
                }
                break;
            case bcmPortControlPrbsRxStatus:
                if (nSciLink == 0) {
		    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI0_PRBS_STATUS);
		    *value = SAND_HAL_GET_FIELD(KA, SC_SI0_PRBS_STATUS, PRBS_ERR_CNT, uData);
		} else {
		    uData = SAND_HAL_READ((sbhandle)unit, KA, SC_SI1_PRBS_STATUS);
		    *value = SAND_HAL_GET_FIELD(KA, SC_SI1_PRBS_STATUS, PRBS_ERR_CNT, uData);
		}
                break;
            case bcmPortControlSerdesDriverStrength:
                nSciLink += SB_FAB_DEVICE_QE2000_SFI_LINKS;
                *value = SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSciLink].uDriverStrength;
                break;
            case bcmPortControlSerdesDriverEqualization:
                nSciLink += SB_FAB_DEVICE_QE2000_SFI_LINKS;
                *value = SOC_SBX_CFG_QE2000(unit)->linkDriverConfig[nSciLink].uDriverEqualization;
                break;
            case bcmPortControlAbility:
                /* SCI/SFI is fixed for QE2000 and no multiplexing is supported */
                *value = BCM_PORT_ABILITY_SCI;
                break;
            default:
                return BCM_E_UNAVAIL;
        }
    } else if (IS_SPI_PORT(unit, port)) {
        nSpiPort = SOC_PORT_BLOCK_INDEX(unit, port);
        switch (type) {
            case bcmPortControlTrain:
                rv = BCM_E_UNAVAIL;
                break;

            case bcmPortControlRxEnable:
                rv = BCM_E_UNAVAIL;
                break;

            case bcmPortControlTxEnable:
                rv = BCM_E_UNAVAIL;
                break;

            case bcmPortControlRxLink:
                (*value) = (hwQe2000Spi4RxStatus((sbhandle)unit, nSpiPort) == 0) ? TRUE : FALSE;
                break;

            case bcmPortControlTxLink:
                (*value) = (hwQe2000Spi4TxStatus((sbhandle)unit, nSpiPort) == 0) ? TRUE : FALSE;
                break;

            default:
                rv = BCM_E_UNAVAIL;
                break;
        }
    } else if (IS_SPI_SUBPORT_PORT(unit, port)) {
        int spi;

        if ((port = SOC_PORT_BLOCK_INDEX(unit, port)) <
            SOC_SBX_CONTROL(unit)->spi_subport_count[0])
        {
            spi = 0;
            spi_port = port;
        } else {
            spi = 1;
	    spi_port = port - SOC_SBX_CONTROL(unit)->spi_subport_count[0];
        }
        switch (type) {
            case bcmPortControlPacketFlowMode:
                if (COMPILER_64_BITTEST(SOC_SBX_CFG_QE2000(unit)->uuRequeuePortsMask[spi], spi_port)) {
		    *value = 1;
		} else {
		    *value = 0;
		}
		break;
            case bcmPortControlFabricKnockoutId:
                _qe2000_port_id_get(unit, port, value);
                break;

            case bcmPortControlEgressNonUnicastLossless:
                rv = qe2000_port_egress_multicast_lossless_get(unit, port, value);
                break;

            default:
                return BCM_E_UNAVAIL;
	}
    } else {
        /*
         * This should be the CPU port.
         */
        switch (type) {
            case bcmPortControlPacketFlowMode:
		/* The CPU port can't be requeue. */
		*value = 0;
		break;
            default:
                return BCM_E_UNAVAIL;
	}
    }
    return rv;
}

int
bcm_qe2000_port_frame_max_set(int unit,
                              bcm_port_t port,
                              int size)
{
    int result;
    int spi, spiPort, min;
    uint32 data;

    QE2000_PORT_CHECK(unit, port);

    if (IS_SPI_SUBPORT_PORT(unit, port)) {
        if ((0x3FFF - HW_QE2000_SHIMHDR_SZ_K) < size) {
            /* frame is too big (>(2^14)-1 bytes with shim header added) */
            result = BCM_E_PARAM;
        } else { /* if (frame size is invalid) */
            result = _qe2000_spi_subport_info_get(unit, port, &spi, &spiPort);
            if (BCM_E_NONE == result) {
                data = SAND_HAL_READ_INDEX_STRIDE((sbhandle)unit,
                                                  KA,
                                                  SR,
                                                  spi,
                                                  SR0_P0_FRAME_SIZE,
                                                  spiPort);
                min = SAND_HAL_GET_FIELD(KA,
                                         SR0_P0_FRAME_SIZE,
                                         MIN_FRAME_SIZE,
                                         data) +
                      (26 - HW_QE2000_SHIMHDR_SZ_K + HW_QE2000_CRC_SZ_K);
                
                if (size < min) {
                    /* max size is smaller than min size */
                    result = BCM_E_CONFIG;
                } else { /* if (size < min) */
                    /*
                     *  Adjust setting of max frame size, leaving min frame
                     *  size setting alone.  Note the adjustment to the max
                     *  frame size is to allow a frame of the specified size
                     *  PLUS the shim header, on the assumption that we don't
                     *  remove parts.
                     */
                    
                    data = SAND_HAL_MOD_FIELD(KA,
                                              SR0_P0_FRAME_SIZE,
                                              MAX_FRAME_SIZE,
                                              data,
                                              size + HW_QE2000_SHIMHDR_SZ_K);
                    SAND_HAL_WRITE_INDEX_STRIDE((sbhandle)unit,
                                                KA,
                                                SR,
                                                spi,
                                                SR0_P0_FRAME_SIZE,
                                                spiPort,
                                                data);
                } /* if (size < min) */
            } /* if (BCM_E_NONE == result) */
        } /* if (frame size is invalid) */
    } else { /* if (IS_SPI_SUBPORT_PORT(unit, port)) */
        /* can't set MxTU on this port */
        result = BCM_E_UNAVAIL;
    } /* if (IS_SPI_SUBPORT_PORT(unit, port)) */
    return result;
}

int
bcm_qe2000_port_frame_max_get(int unit,
                              bcm_port_t port,
                              int *size)
{
    int result = BCM_E_NONE;
    int spi, spiPort;
    uint32 data;

    QE2000_PORT_CHECK(unit, port);

    if (IS_SPI_SUBPORT_PORT(unit, port)) {
        result = _qe2000_spi_subport_info_get(unit, port, &spi, &spiPort);
        if (BCM_E_NONE == result) {
            data = SAND_HAL_READ_INDEX_STRIDE((sbhandle)unit,
                                              KA,
                                              SR,
                                              spi,
                                              SR0_P0_FRAME_SIZE,
                                              spiPort);
            *size = SAND_HAL_GET_FIELD(KA,
                                       SR0_P0_FRAME_SIZE,
                                       MAX_FRAME_SIZE,
                                       data) -
                    HW_QE2000_SHIMHDR_SZ_K;
            /* The claimed max frame size is adj. by the shim header size. */
            
            if (*size < 0) {
                /* the claimed size is too small; make it at least zero */
                *size = 0;
            }
        } /* if (BCM_E_NONE == result) */
    } else { /* if (IS_SPI_SUBPORT_PORT(unit, port)) */
        /* can't get MxTU on this port */
        result = BCM_E_UNAVAIL;
    } /* if (IS_SPI_SUBPORT_PORT(unit, port)) */
    return result;
}

int
_bcm_qe2000_fifo_egress_shaper_set (int unit, int  nFifoParmIndex,
                                uint32 uShaperId, int nType, int bAdd)
{

    int  rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;
    int32 nShaperId;

    /* Get current settings */
    rv = soc_qe2000_eg_mem_read(unit, nFifoParmIndex, 0x2, &uData0, &uData1, &uData2);
#ifdef EGRESS_SHAPER_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "*** bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x ShaperType %d Id %d\n"), uData0, uData1, uData2, nType, uShaperId));
#endif
    if (rv != BCM_E_NONE) {
        return(rv);
    }


    /* Update entry and write back */
    /* if add is not True, we disable the shaper */
    nShaperId = (bAdd == TRUE) ? uShaperId : 0xFF;
    if (nType == BCM_SBX_PORT_EGRESS_SHAPER_PORT) {
        uData0 = ((uData0 & ~(0xFF00)) | (nShaperId << 8));
    }
    else {
        uData0 = ((uData0 & ~(0xFF)) | nShaperId);
    }



    rv = soc_qe2000_eg_mem_write(unit, nFifoParmIndex, 0x2, uData0, uData1, uData2);
    /*
    LOG_CLI((BSL_META_U(unit,
                        "_bcm_qe2000_fifo_egress_set ret: 0x%x (Fifo Index %d: 0x%x 0x%x 0x%x)\n"),
             rv, nFifoParmIndex, uData2, uData1, uData0));
                 */
    return (rv);

}
int
bcm_qe2000_fifo_egress_shaper_get (int unit, int  nFifoParmIndex,
				   uint32 *pShaperIdHiPort, uint32 *pShaperIdLoFifo)
{

    int  rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;

    /* Get current settings */
    rv = soc_qe2000_eg_mem_read(unit, nFifoParmIndex, 0x2, &uData0, &uData1, &uData2);
#ifdef EGRESS_SHAPER_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "*** bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x\n"), uData0, uData1, uData2));
#endif
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    *pShaperIdHiPort = (uData0 & 0xFF00) >> 8;
    *pShaperIdLoFifo = (uData0 & 0x00FF);

    return (rv);

}

int
bcm_qe2000_port_rate_egress_shaper_set(int unit, bcm_port_t port, uint32 uShaperId,
                                uint32 kbits_sec, uint32 kbits_burst)
{
    int rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;
    uint64 uuMaxRateInBitsPerSec;
    uint32   maxRateInBitsPerSec = 0;
    uint32 uShapeRate;
    uint32 uBurstRate;
    uint32 uFifoSrcMask;
    int  nType;
    int  nFifoParmIndex;
    int  bAdd;
    sal_usecs_t t;
    uint32 elapsed;


    /*
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_set (Port %d: ShaperId %d 0x%x 0x%x)\n"), port, uShaperId, kbits_sec, kbits_burst));
    */

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "soc_feature_egress_metering Unavailable")));
        return BCM_E_UNAVAIL;
     }

    if (kbits_burst > 190) {
      LOG_WARN(BSL_LS_BCM_COMMON,
               (BSL_META_U(unit,
                           "Burst size exceeded: Max Burst size(%d,000)bits will be adjusted to 190 kbits\n"),
                kbits_burst));
      kbits_burst = 190;
    }

    /* Shaper is now automatically allocated */
#ifdef NO_LONGER_NEEDED
    /* Need to add shaper allocation. For now we set the port equal to shaper */
    uShaperId = port;
#endif

    /* Need to add Fifo Src Selection. For now select 2:1 and both Multicast enabled to Port */

    uFifoSrcMask = ( MULTICAST_1 | MULTICAST_0 | UNICAST_1 | UNICAST_0 );


    nType = BCM_SBX_PORT_EGRESS_SHAPER_PORT;   /* Port Shaper Type (not FIFO) */

    /*  get the shaper parameters ... debugging only */
    rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);
    /*
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x\n"), uData0, uData1, uData2));
    LOG_CLI((BSL_META_U(unit,
                        "found port %d\n"), (uData0 >> 8) & 0x3f));
    */

    uData0 = 0;
    uData1 = 0;
    uData2 = 0;

    /* if non-zero value setup and enable shaper */
    /* If zero rate specified, disable shaper */

    if (kbits_sec) {

    bAdd = TRUE;

    COMPILER_64_SET(uuMaxRateInBitsPerSec,0, kbits_sec);
    
    COMPILER_64_UMUL_32(uuMaxRateInBitsPerSec, 384);
    if (soc_sbx_div64(uuMaxRateInBitsPerSec, 625, &maxRateInBitsPerSec) == -1) {
      return BCM_E_INTERNAL;
    }
    uShapeRate = maxRateInBitsPerSec;

    /* Compute burst rate in bytes */
    uBurstRate = kbits_burst * 1000;    /* bits per burst */
    uBurstRate = uBurstRate / 8;       /* 8 Bits per byte */

    uData0 |= 0x1;
    uData0 |= ((uFifoSrcMask & 0x3f) << 1);
    uData0 |= (1 << 7);    /* Hi side is egress shaper port */
    uData0 |= ((port & 0x3F) << 8);        /* Port assignment */
    uData0 |= ((uBurstRate & 0x7fff) << 14);  /* Bucket Size */
    uData0 |= ((uShapeRate & 0x7) << 29);  /* lower 3 bits of rate */
    uData1 |= ((uShapeRate & 0xfffff8) >> 3);  /* upper 21 bits of rate */

    /*
    LOG_CLI((BSL_META_U(unit,
                        "kbits_sec %d => uShapeRate %d. kbits_burst %d => uBurstRate %d\n"),
             kbits_sec, uShapeRate, kbits_burst, uBurstRate));
            */

    } else {

        bAdd = FALSE;
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Zero rate specified. Disabling Shaper")));
        /*    LOG_CLI((BSL_META_U(unit,
                                  "Zero rate speicified. Disabling shaper\n"))); */

	/* We must disable the shaper first before making changes. Otherwise the traffic flow that was
	   using the shaper may get into a blocking state. */
	rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);
	uData0 &= ~1;
	rv = soc_qe2000_eg_mem_write(unit, uShaperId, 0x5, uData0, uData1, uData2);
	uData0 = uData1 = 0;
    }

    t = sal_time_usecs();
    /* Unicast 0 */
    nFifoParmIndex = port * 2;
    _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);

    /* Unicast 1 */
    nFifoParmIndex = (port * 2) + 1;
    _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);

    /* Multicast 0 */
    nFifoParmIndex = port + 100;
    _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);

    /* Multicast 1 */
    nFifoParmIndex = port + 150;
    _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);

    elapsed =  SAL_USECS_SUB(sal_time_usecs(), t);
    if (elapsed < 20) {
      sal_usleep(20-elapsed);
    }

    /* Now set the shaper to the port and enable */
    rv = soc_qe2000_eg_mem_write(unit, uShaperId, 0x5, uData0, uData1, uData2);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "Set Shaper %d [0x%x 0x%x]\n"),
              uShaperId, uData1, uData0));
    /*
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_set ret: 0x%x (Port %d: 0x%x 0x%x 0x%x)\n"), rv, port, uData0, uData1, uData2));
    */
    rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);
    /*
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x\n"), uData0, uData1, uData2));
    */

    return rv;
}


int
bcm_qe2000_port_rate_egress_shaper_get(int unit, bcm_port_t port, uint32 uShaperId,
                                uint32 *kbits_sec, uint32 *kbits_burst)
{
    int rv = BCM_E_NONE;

    uint32 uData0, uData1, uData2;
    uint64 uuMaxRateInBitsPerSec;
    uint32   maxRateInBitsPerSec = 0;
    uint32 uShapeRate;
    uint32 uBurstRate;

    uData2 = 0;

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "soc_feature_egress_metering Unavailable")));
        return BCM_E_UNAVAIL;
     }

    rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);

    uShapeRate = (uData0 >> 29 ) & 0x7;
    uShapeRate |= (uData1 << 3 ) & 0xfffff8;
    COMPILER_64_SET(uuMaxRateInBitsPerSec,0,uShapeRate);

    
    COMPILER_64_UMUL_32(uuMaxRateInBitsPerSec, 625);
    if (soc_sbx_div64(uuMaxRateInBitsPerSec, 384, &maxRateInBitsPerSec) == -1) {
      return BCM_E_INTERNAL;
    }
    *kbits_sec = maxRateInBitsPerSec;

    uBurstRate = (uData0 >> 14 ) & 0x7fff;
    uBurstRate *= 8;                 /* 8 bits in a byte */
    *kbits_burst = uBurstRate / 1000;  /* convert bit to kbits */

    /*
    LOG_CLI((BSL_META_U(unit,
                        "Max 0x%x%08x Burst 0x%x\n"), COMPILER_64_HI(maxRateInBitsPerSec), COMPILER_64_LO(maxRateInBitsPerSec), uBurstRate));
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x\n"), uData2, uData1, uData0));
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_get ret: 0x%x. Port %d: Rate %d kb Burst %d kb (0x%x 0x%x)\n"),
             rv, port,  *kbits_sec, *kbits_burst, *kbits_sec, *kbits_burst));
            */

    return rv;
}

int
bcm_qe2000_port_rate_egress_traffic_set(int unit,
        bcm_port_t port,
        uint32 uShaperId,
        uint32 traffic_types,
        uint32 kbits_sec,
        uint32 kbits_burst)
{
    int rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;
    uint64 uuMaxRateInBitsPerSec;
    uint32   maxRateInBitsPerSec = 0;
    uint32 uShapeRate;
    uint32 uBurstRate;
    uint32 uFifoSrcMask;
    int  nType;
    int  nFifoParmIndex;
    int  bAdd;
    sal_usecs_t t;
    uint32 elapsed;

#ifdef EGRESS_TRAFFIC_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_traffic_set... 0x%x 0x%x 0x%x 0x%x 0x%x\n"),
             unit, port, traffic_types, kbits_sec, kbits_burst));
#endif

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "soc_feature_egress_metering Unavailable")));
        return BCM_E_UNAVAIL;
     }

    if (kbits_burst > 190) {
      LOG_WARN(BSL_LS_BCM_COMMON,
               (BSL_META_U(unit,
                           "Burst size exceeded: Max Burst size(%d,000)bits will be adjusted to 190 kbits\n"),
                kbits_burst));
      kbits_burst = 190;
    }

    /* Need to add Fif oSrc Selection. For now select 2:1 and both Multicast enabled to Port */

    uFifoSrcMask = 0;

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_UC_EF) {
       uFifoSrcMask |= UNICAST_0 ;
    }

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_UC_NON_EF) {
       uFifoSrcMask |= UNICAST_1 ;
    }

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_NON_UC_EF) {
       uFifoSrcMask |= MULTICAST_0 ;
    }

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF) {
       uFifoSrcMask |= MULTICAST_1 ;
    }


    nType = BCM_SBX_PORT_EGRESS_SHAPER_FIFO;   /* Fifo Shaper Type (not port) */

    /*  get the shaper parameters ... debugging only */
    /*
    rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x\n"), uData0, uData1, uData2));
    LOG_CLI((BSL_META_U(unit,
                        "found port %d\n"), (uData0 >> 8) & 0x3f));
    */

    uData0 = 0;
    uData1 = 0;
    uData2 = 0;

    /* if non-zero value setup and enable shaper */
    /* If zero rate specified, disable shaper */

    if (kbits_sec) {

    bAdd = TRUE;

    COMPILER_64_SET(uuMaxRateInBitsPerSec,0, kbits_sec);
    
    COMPILER_64_UMUL_32(uuMaxRateInBitsPerSec, 384);
    if (soc_sbx_div64(uuMaxRateInBitsPerSec, 625, &maxRateInBitsPerSec) == -1) {
      return BCM_E_INTERNAL;
    }

    uShapeRate = maxRateInBitsPerSec;

    /* Compute burst rate in bytes */
    uBurstRate = kbits_burst * 1000;    /* bits per burst */
    uBurstRate = uBurstRate / 8;       /* 8 Bits per byte */

    uData0 |= 0x1;         /* Enable */
    uData0 |= ((uFifoSrcMask & 0x3f) << 1);
    uData0 |= (0 << 7);    /* Lo side is egress shaper fifo */
    uData0 |= ((port & 0x3F) << 8);        /* Port assignment */
    uData0 |= ((uBurstRate & 0x7fff) << 14);  /* Bucket Size */
    uData0 |= ((uShapeRate & 0x7) << 29);  /* lower 3 bits of rate */
    uData1 |= ((uShapeRate & 0xfffff8) >> 3);  /* upper 21 bits of rate */

    /*
    LOG_CLI((BSL_META_U(unit,
                        "kbits_sec %d => uShapeRate %d. kbits_burst %d => uBurstRate %d\n"),
             kbits_sec, uShapeRate, kbits_burst, uBurstRate));
            */

    } else {

        bAdd = FALSE;
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "Zero rate specified. Disabling Shaper")));
        /*    LOG_CLI((BSL_META_U(unit,
                                  "Zero rate speicified. Disabling shaper\n"))); */

	/* We must disable the shaper first before making changes. Otherwise the traffic flow that was
	   using the shaper may get into a blocking state. */
	rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);
    if (rv) {
        return(rv);
    }
	uData0 &= ~1;
	rv = soc_qe2000_eg_mem_write(unit, uShaperId, 0x5, uData0, uData1, uData2);
    if (rv) {
        return(rv);
    }
	uData0 = uData1 = 0;
	
    }

    t = sal_time_usecs();
    if (traffic_types & BCM_PORT_RATE_TRAFFIC_UC_EF) {
        /* Unicast 0 */
        nFifoParmIndex = port * 2;
        _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);
    }

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_UC_NON_EF) {
        /* Unicast 1 */
        nFifoParmIndex = (port * 2) + 1;
        _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);
    }

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_NON_UC_EF) {
        /* Multicast 0 */
        nFifoParmIndex = port + 150;
        _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);
    }

    if (traffic_types & BCM_PORT_RATE_TRAFFIC_NON_UC_NON_EF) {
        /* Multicast 1 */
        nFifoParmIndex = port + 100;
        _bcm_qe2000_fifo_egress_shaper_set (unit, nFifoParmIndex, uShaperId, nType, bAdd);
    }

    elapsed =  SAL_USECS_SUB(sal_time_usecs(), t);
    if (elapsed < 20) {
      sal_usleep(20-elapsed);
    }

    /* Now set the shaper to the port and enable */
    rv = soc_qe2000_eg_mem_write(unit, uShaperId, 0x5,  uData0, uData1, uData2);
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "Set Shaper %d [0x%x 0x%x]\n"),
              uShaperId, uData1, uData0));
#ifdef EGRESS_TRAFFIC_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_traffic_set ret: 0x%x (Port %d: 0x%x 0x%x 0x%x)\n"), rv, port, uData2, uData1, uData0));
#endif


    return rv;
}

int
bcm_qe2000_port_rate_egress_traffic_get(int unit,
        bcm_port_t port,
        uint32 uShaperId,
        uint32 *traffic_types,
        uint32 *kbits_sec,
        uint32 *kbits_burst)
{
    int rv = BCM_E_NONE;

    uint32 uData0, uData1, uData2;
    uint64 uuMaxRateInBitsPerSec;
    uint32 maxRateInBitsPerSec = 0;
    uint32 uShapeRate;
    uint32 uBurstRate;

    uData2 = 0;

#ifdef EGRESS_TRAFFIC_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_traffic_get... ShaperId %d traffic_type 0x%x\n"),
             uShaperId, *traffic_types));
#endif

    if (!soc_feature(unit, soc_feature_egress_metering)) {
        LOG_INFO(BSL_LS_BCM_PORT,
                 (BSL_META_U(unit,
                             "soc_feature_egress_metering Unavailable")));
        return BCM_E_UNAVAIL;
     }


    /* Need to add fifo allocation. For now we set the port equal to shaper */

    rv = soc_qe2000_eg_mem_read(unit, uShaperId, 0x5, &uData0, &uData1, &uData2);

    uShapeRate = (uData0 >> 29 ) & 0x7;
    uShapeRate |= (uData1 << 3 ) & 0xfffff8;
    COMPILER_64_SET(uuMaxRateInBitsPerSec,0,uShapeRate);

    

    COMPILER_64_UMUL_32(uuMaxRateInBitsPerSec, 625);
    if(soc_sbx_div64(uuMaxRateInBitsPerSec, 384, &maxRateInBitsPerSec) == -1) {
      return BCM_E_INTERNAL;
    }
    *kbits_sec = maxRateInBitsPerSec;

    uBurstRate = (uData0 >> 14 ) & 0x7fff;
    uBurstRate *= 8;                 /* 8 bits in a byte */
    *kbits_burst = uBurstRate / 1000;  /* convert bit to kbits */

#ifdef EGRESS_TRAFFIC_DEBUG
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_eg_mem_read:Data 0x%x 0x%x 0x%x\n"), uData0, uData1, uData2));
    LOG_CLI((BSL_META_U(unit,
                        "Max 0x%x%08x Burst 0x%x\n"), COMPILER_64_HI(maxRateInBitsPerSec), COMPILER_64_LO(maxRateInBitsPerSec), uBurstRate));
    LOG_CLI((BSL_META_U(unit,
                        "bcm_qe2000_port_rate_egress_get ret: 0x%x. Port %d: Rate %d kb Burst %d kb (0x%x 0x%x)\n"),
             rv, port,  *kbits_sec, *kbits_burst, *kbits_sec, *kbits_burst));
#endif

    return rv;
}


static int
_bcm_qe2000_fifo_size_set(int unit, int fifo_parm_index,
                                uint32 bytes_min, uint32 bytes_max)
{

    int  rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;
    uint32 pages_min, pages_max;


    /* Get current settings */
    rv = soc_qe2000_eg_mem_read(unit, fifo_parm_index, 0x2, &uData0, &uData1, &uData2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* Update entry and write back */
    pages_min = (bytes_min / 1024) + (((bytes_min % 1024) == 0) ? 0 : 1);
    pages_max = (bytes_max / 1024) + (((bytes_max % 1024) == 0) ? 0 : 1);
    uData0 = (uData0 & ~(0x03FF0000)) | (pages_min << 16);
    uData0 = (uData0 & ~(0xFC000000)) | ((pages_max & 0x3F) << 26);
    uData1 = (uData1 & ~(0xF)) | (pages_max >> 6);
    rv = soc_qe2000_eg_mem_write(unit, fifo_parm_index, 0x2, uData0, uData1, uData2);

    return (rv);
}


static int
_bcm_qe2000_fifo_size_get(int unit, int fifo_parm_index,
                                uint32 *bytes_min, uint32 *bytes_max)
{
    int  rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;
    uint32 pages_min, pages_max;


    /* Get current settings */
    rv = soc_qe2000_eg_mem_read(unit, fifo_parm_index, 0x2, &uData0, &uData1, &uData2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    /* retreive current setting */
    pages_min = (uData0 & 0x03FF0000) >> 16;
    pages_max = ((uData0 & 0xFC000000) >> 26) | ((uData1 & 0xF) << 6);
    (*bytes_min) = pages_min * 1024;
    (*bytes_max) = pages_max * 1024;

    return (rv);
}

static int
_bcm_qe2000_fifo_force_full_set(int unit, int fifo_parm_index, int is_asserted)
{
    int  rv = BCM_E_NONE;
    int  reg_block, reg_offset;
    uint32 value;


    reg_block = fifo_parm_index / 32;
    reg_offset = fifo_parm_index % 32;

    /* retreive current configuration */
    switch(reg_block) {
        case 0:
            value =  SAND_HAL_READ((sbhandle)unit, KA, EG_FORCE_FULL_0);
            break;

        case 1:
            value =  SAND_HAL_READ((sbhandle)unit, KA, EG_FORCE_FULL_1);
            break;

        case 2:
            value =  SAND_HAL_READ((sbhandle)unit, KA, EG_FORCE_FULL_2);
            break;

        case 3:
        default:
            value =  SAND_HAL_READ((sbhandle)unit, KA, EG_FORCE_FULL_3);
            break;
    }

    /* update configuration */
    if (is_asserted == TRUE) {
        value |= (1 << reg_offset);
    }
    else {
        value &= ~(1 << reg_offset); 
    }

    /* store configuration */
    switch(reg_block) {
        case 0:
            SAND_HAL_WRITE((sbhandle)unit, KA, EG_FORCE_FULL_0, value);
            break;

        case 1:
            SAND_HAL_WRITE((sbhandle)unit, KA, EG_FORCE_FULL_1, value);
            break;

        case 2:
            SAND_HAL_WRITE((sbhandle)unit, KA, EG_FORCE_FULL_2, value);
            break;

        case 3:
        default:
            SAND_HAL_WRITE((sbhandle)unit, KA, EG_FORCE_FULL_3, value);
            break;
    }

    return (rv);
}

static int
_bcm_qe2000_fifo_empty(int unit, int fifo_parm_index, int *is_empty)
{
    int rv = BCM_E_NONE;
    uint32 uData0, uData1, uData2;
    int depth, i;


    (*is_empty) = FALSE;
    thin_delay(SB_FAB_DEVICE_QE2000_FIFO_EMPTY_DELAY);

    for (i = 0; i < SB_FAB_DEVICE_QE2000_FIFO_EMPTY_ITER_COUNT; i++) {
        rv = soc_qe2000_eg_mem_read(unit, fifo_parm_index, 0x1, &uData0, &uData1, &uData2);
        if (rv != BCM_E_NONE) {
            return(rv);
        }

        /* check if depth is 0 */
        depth = (uData0 & 0x001FFFE0) >> 5;
        if (depth == 0) {
            break;
        }

        sal_udelay(SB_FAB_DEVICE_QE2000_FIFO_EMPTY_ITER_DELAY);
    }

    if (i >= SB_FAB_DEVICE_QE2000_FIFO_EMPTY_ITER_COUNT) {
        return(rv);
    }

    (*is_empty) = TRUE;
    
    return (rv);
}

int
bcm_qe2000_port_fifo_enable_set(int unit,
				int cos,
				int sysport,
				int port,
				int local,
				int enable)
{
    int rv = BCM_E_UNAVAIL;
#if 0
    int fifo_parm_index, ef = 0;
    int is_full_asserted = FALSE, is_empty;
    uint32 uData0, uData1, uData2;
    int spi, spi_port;

    /* consistency checks */
    if (!IS_SPI_SUBPORT_PORT(unit, port)) {
        return(BCM_E_PARAM);
    }

    /* unicast EF/N-EF */
    if ( (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_EF) && (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_NEF) ) { 
        return(BCM_E_PARAM);
    }

    ef = (cos == BCM_INT_XCORE_COS_FIFO_UNICAST_EF) ? 1 : 0;

    fifo_parm_index = (sysport & 0x3F);

    if (local == 0) {
	fifo_parm_index |= (ef << 8);
    }

    /* Assert Flow Control */
    rv = _bcm_qe2000_fifo_force_full_set(unit, fifo_parm_index, TRUE);
    is_full_asserted = TRUE; /* This assumes this is the only place where Full is asserted */

    /* Check if fifo depth is zero i.e. empty */
    rv = _bcm_qe2000_fifo_empty(unit, fifo_parm_index, &is_empty);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    if (is_empty == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "FIFO not empty fifo: %d, port: %d\n"),
                   fifo_parm_index, port));

        /* check that functionality is supported in the configured mode */
        _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);
        if (SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[spi] != SOC_SBX_PORT_MODE_BURST_IL) {
            rv = BCM_E_UNAVAIL;
            goto err;
        }

        /* Flush port */
        rv = _bcm_qe2000_port_flush(unit, port);
        if (rv != BCM_E_NONE) {
            goto err;
        }
    }

    rv = soc_qe2000_eg_mem_read(unit, fifo_parm_index, 0x0, &uData0, &uData1, &uData2);
    if (rv != BCM_E_NONE) {
	return(rv);
    }
    
    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "Port Remap Entry (%d) 0x%x\n"),
              uData0));
    rv = soc_qe2000_eg_mem_port_remap_table_write(unit, fifo_parm_index, ((uData0>>8)&1), local, ((enable == TRUE) ? 1 : 0), (uData0 & 0x7F));

    /* De-assert Flow Control */
    rv = _bcm_qe2000_fifo_force_full_set(unit, fifo_parm_index, FALSE);

    return(rv);

err:
    if (is_full_asserted == TRUE) {
        _bcm_qe2000_fifo_force_full_set(unit, fifo_parm_index, FALSE);
    }
#endif
    return(rv);
}


int
bcm_qe2000_port_fifo_enable_get(int unit,
				int cos,
				int sysport,
				int port, 
				int local,
				int *enable)
{
    int rv = BCM_E_UNAVAIL;
#if 0
    int fifo_parm_index, ef = 0;
    uint32 uData0, uData1, uData2;

    /* consistency checks */
    if (!IS_SPI_SUBPORT_PORT(unit, port)) {
        return(BCM_E_PARAM);
    }

    /* unicast EF/N-EF */
    if ( (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_EF) && (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_NEF) ) { 
        return(BCM_E_PARAM);
    }
 
    ef = (cos == BCM_INT_XCORE_COS_FIFO_UNICAST_EF) ? 1 : 0;

    fifo_parm_index = (sysport & 0x3F);

    if (local == 0) {
	fifo_parm_index |= (ef << 8);
    }

    /* Get current settings */
    rv = soc_qe2000_eg_mem_read(unit, fifo_parm_index, 0x0, &uData0, &uData1, &uData2);
    if (rv != BCM_E_NONE) {
        return(rv);
    }

    LOG_INFO(BSL_LS_BCM_PORT,
             (BSL_META_U(unit,
                         "Port Remap Entry (%d) 0x%x\n"),
              uData0));

    /* retreive current setting */
    *enable = (uData0 >>7) & 1; /* fifo_enable */
#endif
    return(rv);
}


int
bcm_qe2000_port_size_set(int unit,
			 bcm_port_t port,
			 int cos,
			 uint32  bytes_min,
			 uint32  bytes_max)
{
    int rv = BCM_E_NONE;
    int fifo_parm_index;
    int is_full_asserted = FALSE, is_empty;
    int spi, spi_port;

 
    /* consistency checks */
    if (!IS_SPI_SUBPORT_PORT(unit, port)) {
        return(BCM_E_PARAM);
    }
    if ( (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_EF) && (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_NEF) ) { /* unicast EF/N-EF */
        return(BCM_E_PARAM);
    }

    fifo_parm_index = (cos == 0) ? (port * 2) : ((port * 2) + 1);

    /* Assert Flow Control */
    rv = _bcm_qe2000_fifo_force_full_set(unit, fifo_parm_index, TRUE);
    is_full_asserted = TRUE; /* This assumes this is the only place where Full is asserted */

    /* Check if fifo depth is zero i.e. empty */
    rv = _bcm_qe2000_fifo_empty(unit, fifo_parm_index, &is_empty);
    if (rv != BCM_E_NONE) {
        goto err;
    }

    if (is_empty == FALSE) {
        LOG_ERROR(BSL_LS_BCM_COMMON,
                  (BSL_META_U(unit,
                              "FIFO not empty fifo: %d, port: %d\n"),
                   fifo_parm_index, port));

        /* check that functionality is supported in the configured mode */
        _qe2000_spi_subport_info_get(unit, port, &spi, &spi_port);
        if (SOC_SBX_CFG_QE2000(unit)->bEiSpiFullPacketMode[spi] != SOC_SBX_PORT_MODE_BURST_IL) {
            rv = BCM_E_UNAVAIL;
            goto err;
        }

        /* Flush port */
        rv = _bcm_qe2000_port_flush(unit, port);
        if (rv != BCM_E_NONE) {
            goto err;
        }
    }

    /* configure size settings */
    rv = _bcm_qe2000_fifo_size_set(unit, fifo_parm_index, bytes_min, bytes_max);

    /* De-assert Flow Control */
    rv = _bcm_qe2000_fifo_force_full_set(unit, fifo_parm_index, FALSE);

    return(rv);

err:
    if (is_full_asserted == TRUE) {
        _bcm_qe2000_fifo_force_full_set(unit, fifo_parm_index, FALSE);
    }

    return(rv);
}


int
bcm_qe2000_port_size_get(int unit,
			 bcm_port_t port,
			 int cos,
			 uint32  *bytes_min,
			 uint32  *bytes_max)
{
    int rv = BCM_E_NONE;
    int fifo_parm_index;


    /* consistency checks */
    if (!IS_SPI_SUBPORT_PORT(unit, port)) {
        return(BCM_E_PARAM);
    }
    if ( (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_EF) && (cos != BCM_INT_XCORE_COS_FIFO_UNICAST_NEF) ) { /* unicast EF/N-EF */
        return(BCM_E_PARAM);
    }
 
    /* retreive configuration                                                  */ 
    /* NOTE: Hardware configuration is retreived. This could be different from */
    /*       the application settings.                                         */
    fifo_parm_index = (cos == 0) ? (port * 2) : ((port * 2) + 1);
    rv = _bcm_qe2000_fifo_size_get(unit, fifo_parm_index, bytes_min, bytes_max);

    return(rv);
}




int
bcm_qe2000_port_multicast_size_set(int unit,
				   int cos,
				   uint32  bytes_min,
				   uint32  bytes_max)
{
    int rv = BCM_E_NONE;
    bcm_port_t port;
    uint32 uEfData0, uEfData1, uEfData2;
    uint32 uNefData0, uNefData1, uNefData2;
    int is_mc_paused, i;
    int paused_port_mask = 0;
    uint32 uData;


    if ( (cos != BCM_INT_XCORE_COS_FIFO_MULTICAST_EF) && (cos != BCM_INT_XCORE_COS_FIFO_MULTICAST_NEF) ) { /* multicast EF/N-EF */
        return(BCM_E_PARAM);
    }
    if ((bytes_min < 1024) || (bytes_max < 1024)) {
	return(BCM_E_PARAM);
    } 

    /* retrieve and store pause multicast state of all ports */
    for (port = 0; port < SOC_SBX_CFG(unit)->max_ports; port++) {
	qe2000_port_egress_multicast_pause_get(unit, port, &is_mc_paused);
	if (is_mc_paused) {
	    paused_port_mask |= 1 << port;
	}
    }

    /* pause all ports */
    for (port = 0; port < SOC_SBX_CFG(unit)->max_ports; port++) {
	qe2000_port_egress_multicast_pause_set(unit, port, TRUE);
    }

    /* delay - pause to get reflected to EG block */
    thin_delay(SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_DELAY);

    /* poll for read and write pointers to be equal */
    for (port = 0; port < SOC_SBX_CFG(unit)->max_ports; port++) {
	for (i = 0; i < SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_COUNT; i++) {

	    rv = soc_qe2000_eg_mem_read(unit, port, 0x8, &uEfData0, &uEfData1, &uEfData2);
	    if (rv != BCM_E_NONE) {
		goto err;
	    }

	    rv = soc_qe2000_eg_mem_read(unit, (SB_FAB_DEVICE_QE2000_MULTICAST_NEF_FIFO_OFFSET + port),
					0x8, &uNefData0, &uNefData1, &uNefData2);
	    if (rv != BCM_E_NONE) {
		goto err;
	    }

	    if ( (((uEfData0 >> 13) & 0x3F) == ((uEfData0 >> 19) & 0x3F)) &&
		 (((uNefData0 >> 13) & 0x3F) == ((uNefData0 >> 19) & 0x3F)) ) {
		break;
	    }

	    sal_udelay(SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_DELAY);
	}

	if (i >= SB_FAB_DEVICE_QE2000_MULTICAST_PAUSE_ITER_COUNT) {
	    rv = BCM_E_TIMEOUT;
	    goto err;
	}
    }

    /* Set the multicast pointer fifo size */
    if (cos == 2) {

	uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MC_EF_THRESH);
	uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, PAGES_HIGH, uData, bytes_max/1024);
	uData = SAND_HAL_MOD_FIELD(KA, EG_MC_EF_THRESH, PAGES_LOW, uData, bytes_min/1024);
	SAND_HAL_WRITE((sbhandle)unit, KA, EG_MC_EF_THRESH, uData);

	/* drop EF MC packet if it requires the EF MC fifo to use > pages + 20 */
	uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MC_OVERFLOW);
	uData = SAND_HAL_MOD_FIELD(KA, EG_MC_OVERFLOW, EF_THRESH, 0, (bytes_max/1024)+20);
	SAND_HAL_WRITE((sbhandle)unit, KA, EG_MC_OVERFLOW, uData);
	
    } else {

	/* SET UP MC FIFO THRESHOLDS */
	uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MC_THRESH);
	uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, PAGES_HIGH, uData, bytes_max/1024);
	uData = SAND_HAL_MOD_FIELD(KA, EG_MC_THRESH, PAGES_LOW, uData, bytes_min/1024);
	SAND_HAL_WRITE((sbhandle)unit, KA, EG_MC_THRESH, uData);

	/* drop MC packet if it requires the EF MC fifo to use > pages + 20 */
	uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MC_OVERFLOW);
	uData = SAND_HAL_MOD_FIELD(KA, EG_MC_OVERFLOW, THRESH, uData, (bytes_max/1024)+20);
	SAND_HAL_WRITE((sbhandle)unit, KA, EG_MC_OVERFLOW, uData);
    }

err:
    for (port = 0; port < SOC_SBX_CFG(unit)->max_ports; port++) {

	is_mc_paused = paused_port_mask & (1 << port);

	if (is_mc_paused == 0) {
	    qe2000_port_egress_multicast_pause_set(unit, port, FALSE);
	}
    }

    return(rv);
}


int
bcm_qe2000_port_multicast_size_get(int unit,
				   int cos,
				   uint32  *bytes_min,
				   uint32  *bytes_max)
{
    int rv = BCM_E_NONE;
    uint32 uData;


    if ( (cos != BCM_INT_XCORE_COS_FIFO_MULTICAST_EF) && (cos != BCM_INT_XCORE_COS_FIFO_MULTICAST_NEF) ) { /* multicast EF/N-EF */
        return(BCM_E_PARAM);
    }
 
    /* Set the multicast pointer fifo size */
    if (cos == 2) {

	uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MC_EF_THRESH);
	*bytes_max = SAND_HAL_GET_FIELD(KA, EG_MC_EF_THRESH, PAGES_HIGH, uData);
	*bytes_min = SAND_HAL_GET_FIELD(KA, EG_MC_EF_THRESH, PAGES_LOW, uData);

	
    } else {

	/* SET UP MC FIFO THRESHOLDS */
	uData = SAND_HAL_READ((sbhandle)unit, KA, EG_MC_THRESH);
	*bytes_max = SAND_HAL_GET_FIELD(KA, EG_MC_THRESH, PAGES_HIGH, uData);
	*bytes_min = SAND_HAL_GET_FIELD(KA, EG_MC_THRESH, PAGES_LOW, uData);
    }

    *bytes_min = *bytes_min * 1024;
    *bytes_max = *bytes_max * 1024;

    return(rv);
}


