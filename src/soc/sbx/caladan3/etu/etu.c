/*
 * $Id: etu.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    etu.c
 * Purpose: Caladan3 External TCAM driver
 * 
 */

#include <shared/bsl.h>

#include <soc/types.h>
#include <soc/drv.h>

#ifdef BCM_CALADAN3_SUPPORT
#include <soc/sbx/caladan3.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/mem.h>
#include <soc/mcm/allenum.h>
#include <shared/util.h>
#include <soc/sbx/caladan3/etu.h>
#include <soc/sbx/caladan3/util.h>
#include <soc/sbx/sbDq.h>
#include <sal/appl/io.h>

/*#define USE_HPPC_TEST*/
#ifdef USE_HPPC_TEST
    extern void c3hppc_etu_interface_bringup(int unit);
#endif

extern int wcmod_esm_serdes_fifo_reset(int unit);

int _c3_tcam_mdio_portid[SOC_MAX_NUM_DEVICES][SOC_SBX_CALADAN3_ETU_TCAM_DEV_MAX];

int _soc_sbx_caladan3_etu_num_tcam[SOC_MAX_NUM_DEVICES];

/*
 * Total num of TCAM's detected
 */
#define SOC_SBX_CALADAN3_ETU_TCAM_DEV_AVAIL (_soc_sbx_caladan3_etu_num_tcam)


/*
 * The Tcam index to MDIO port id map
 */
#define ETU_TCAM_MDIO_PORTID(u, d) _c3_tcam_mdio_portid[(u)][(d)]

int
soc_sbx_caladan3_mdio_portid_get(int unit, int dev) {
    if ((unit >=0) && (unit < SOC_MAX_NUM_DEVICES) &&
        (dev >=0) && (dev < SOC_SBX_CALADAN3_ETU_TCAM_DEV_MAX)) {
        return (_c3_tcam_mdio_portid[unit][dev]);
    }
    LOG_ERROR(BSL_LS_SOC_COMMON,
              (BSL_META_U(unit,
                          "Unit %d Invalid TCAM device mapping Dev %d\n"), unit, dev));
    return 0;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_devid_config
 * Purpose: 
 *    Understand where the TCAMs are located
 */
void 
soc_sbx_caladan3_etu_tcam_devid_config(int unit)
{
    int i, devid = 0;

    for (i=0;i<SOC_SBX_CALADAN3_ETU_TCAM_DEV_MAX; i++) {
        devid = soc_property_suffix_num_get(unit, i,
                                            spn_PORT_PHY_ADDR, "ext_tcam", 
                                            i+SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_MDIO_ADDR);
        ETU_TCAM_MDIO_PORTID(unit,i) = devid;
    }
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_detect
 * Purpose: 
 *    Detect and init TCAM devices
 */
int 
soc_sbx_caladan3_etu_tcam_detect(int unit, int *ntcam)
{
    int i, rv = SOC_E_NONE;
    int count = 0;

    soc_sbx_caladan3_etu_tcam_devid_config(unit);
    for (i=0; i<SOC_SBX_CALADAN3_ETU_TCAM_DEV_MAX; i++) {
        rv = soc_etu_nl_mdio_test_reg_access(unit, ETU_TCAM_MDIO_PORTID(unit, i));
        if (SOC_SUCCESS(rv)) {
             count++;
        }
    }
    *ntcam = count;
    return (count > 0 ? SOC_E_NONE : SOC_E_UNAVAIL);
}

void
soc_sbx_caladan3_etu_program_table_setup(int unit, int etu_prgid ) {
}

/*
 * Function
 *   soc_sbx_caladan3_etu_wait_for_rsp
 * Purpose
 *   Wait for the capture rsp or till timeout
 */
int
soc_sbx_caladan3_etu_wait_for_rsp(int unit)
{
    return SOC_E_NONE;
}

/*
 * Function
 *   soc_sbx_caladan3_etu_ilamac_clear_errors
 * Purpose
 *   clear the ILA error counters
 */
int
soc_sbx_caladan3_etu_ilamac_clear_errors(int unit)
{
    int i;
    for (i=0; i < SOC_SBX_CALADAN3_WCL_NUM_RX_LANES; i++) {
        WRITE_ILAMAC_RX_WORD_SYNC_ERRORS_COUNTr(unit, i, 0);
        WRITE_ILAMAC_RX_FRAMING_ERRORS_COUNTr(unit, i, 0);
        WRITE_ILAMAC_RX_BAD_TYPE_ERRORS_COUNTr(unit, i, 0);
        WRITE_ILAMAC_RX_WORD_SYNC_ERRORS_COUNTr(unit, i, 0);
        WRITE_ILAMAC_RX_LANE_CRC_ERRORS_COUNTr(unit, i, 0);
    }
    WRITE_ILAMAC_RX_LANE_INTR0_CLEARr(unit, 0xffffff);
    WRITE_ILAMAC_RX_LANE_INTR1_CLEARr(unit, 0xffffff);
    WRITE_ILAMAC_RX_LANE_INTR2_CLEARr(unit, 0xffffff);
    WRITE_ILAMAC_RX_LANE_INTR3_CLEARr(unit, 0xffffff);
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_ilmac_config
 * Purpose: 
 *    Setup ILMAC
 */
int
soc_sbx_caladan3_etu_ilmac_config(int unit)
{
    uint32 tx_cfg_buf;
    uint32 rx_cfg_buf;
    /*uint32 regval; */

    /* 200ns timing pulse *|
    READ_ETU_WRAP_GLOBAL_CONFIGr(unit, &regval);
    soc_reg_field_set(unit, ETU_WRAP_GLOBAL_CONFIGr, &regval, PULSE_GEN_ENABLEf, 0);
    WRITE_ETU_WRAP_GLOBAL_CONFIGr(unit, regval);

    soc_reg_field_set(unit, ETU_WRAP_GLOBAL_CONFIGr, &regval, PULSE_GEN_COUNTf, 0x3b);
    WRITE_ETU_WRAP_GLOBAL_CONFIGr(unit, regval);

    soc_reg_field_set(unit, ETU_WRAP_GLOBAL_CONFIGr, &regval, PULSE_GEN_ENABLEf, 1);
    WRITE_ETU_WRAP_GLOBAL_CONFIGr(unit, regval); */

    READ_ILAMAC_TX_CONFIG0r(unit, &tx_cfg_buf);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, BIT_ORDER_INVERTf, 1);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_XOFF_MODEf, 0);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_RLIM_ENABLEf, 0);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_RDYOUT_THRESHf, 0);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_DISABLE_SKIPWORDf, 0);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_BURSTSHORTf, 1);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_BURSTMAXf, 1);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, 
                      TX_MFRAMELEN_MINUS1f, 
                      SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_METAFRAME_LEN - 1);

    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_ENABLEf, 1);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, XON_RX_CH1f, 0);
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, XON_RX_CH0f, 1);
    SOC_IF_ERROR_RETURN(
           WRITE_ILAMAC_TX_CONFIG0r(unit, tx_cfg_buf));

    /* Allow ETU_RX_RSP_FIFO.XON_RX to go out. 
       At this point in time, etu_rx_rsp_fifo is empty so XON_RX=1
       This signals NL that it is ok to send response packets
       We have not even checked if ilamac_rx is aligned - so why tell NL to send packets ??
       It will not matter because NL is slave device - it sends response packets
       only if we send a request
    */

    /* Error injection fields. Some default values of non-zero - so make it 0 */
    SOC_IF_ERROR_RETURN(
           WRITE_ILAMAC_TX_CONFIG4r(unit, 0));


    SOC_IF_ERROR_RETURN(READ_ILAMAC_RX_CONFIGr(unit, &rx_cfg_buf));
    soc_reg_field_set(unit, ILAMAC_RX_CONFIGr, &rx_cfg_buf, BIT_ORDER_INVERTf, 1);
    soc_reg_field_set(unit, ILAMAC_RX_CONFIGr, &rx_cfg_buf, RX_BURSTMAXf, 0);
    soc_reg_field_set(unit, ILAMAC_RX_CONFIGr, &rx_cfg_buf, RX_MFRAMELEN_MINUS1f, 
                      SOC_SBX_CALADAN3_ETU_TCAM_DEFAULT_METAFRAME_LEN - 1);
    SOC_IF_ERROR_RETURN(
           WRITE_ILAMAC_RX_CONFIGr(unit, rx_cfg_buf));

    return SOC_E_NONE;
}


/*
 * Function:
 *    soc_sbx_caladan3_etu_config
 * Purpose: 
 *    enable the tcam
 */
int
soc_sbx_caladan3_etu_config(int unit)
{
    uint32 etu_config4_buf;
    uint32 etu_bist_ctl_buf;
    int rv;

    /* ILA TX reset */
    SOC_IF_ERROR_RETURN(READ_ETU_CONFIG4r(unit, &etu_config4_buf));
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_TX_SERDES_RST_f, 0);
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_TX_LBUS_RST_f, 0);
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_RX_SERDES_RST_f, 0);
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_RX_LBUS_RST_f, 0);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));
    sal_usleep(100000);     /* Wait 100 ms */

    rv = soc_sbx_caladan3_etu_ilmac_config(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed initializing ILA, status(%d)\n"), unit, rv));
        return rv;
    }

    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_TX_LBUS_RST_f, 1);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));
    sal_usleep(100000);     /* Wait 100 ms */
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_TX_SERDES_RST_f, 1);
    SOC_IF_ERROR_RETURN(
         WRITE_ETU_CONFIG4r(unit, etu_config4_buf));

    sal_usleep(100000);     /* Wait 100 ms */
    rv = wcmod_esm_serdes_fifo_reset(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed to reset serdes phase compensation FIFO, status(%d)\n"), unit, rv));
        return rv;
    }

    sal_usleep(100000);     /* Wait 100 ms */

    SOC_IF_ERROR_RETURN(READ_ETU_BIST_CTLr(unit, &etu_bist_ctl_buf));
    soc_reg_field_set(unit, ETU_BIST_CTLr, &etu_bist_ctl_buf, ENABLEf, 0);
    SOC_IF_ERROR_RETURN(
          WRITE_ETU_BIST_CTLr(unit, etu_bist_ctl_buf));

    /* ILA Rx out of reset */
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_RX_LBUS_RST_f, 1);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));

    sal_usleep(100000);
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, ILAMAC_RX_SERDES_RST_f, 0xfff);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));


    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_prbs
 * Purpose: 
 *    Setup and run PRBS
 */
int
soc_sbx_caladan3_etu_prbs(int unit, uint32 poly)
{
    int rv = SOC_E_NONE, lane = 0;
    uint32 value = 0;
    if (poly == 0) {
    }

    for (lane = 0; lane < SOC_SBX_CALADAN3_WCL_NUM_LANES_MAX; lane++) {
        
        /* Setup c3 */
        rv = wcmod_esm_serdes_control_set(unit, lane, 
                                          SOC_PHY_CONTROL_PRBS_POLYNOMIAL, &poly);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Failed to setup the polynomial %x rv(%d)\n"), 
                       unit, poly, rv));
            return rv;
        }
    
        /* Setup tcam */
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Failed to setup prbs on tcam rv(%d)\n"), 
                       unit, rv));
            return rv;
        }
    
        /* PRBS tx and rx */
        rv = wcmod_esm_serdes_control_set(unit, lane, 
                                          SOC_PHY_CONTROL_PRBS_TX_ENABLE, &poly);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Failed to start prbs rv(%d)\n"), 
                       unit, rv));
            return rv;
        }
    
        /* Start Tcam */
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Failed to start prbs rv(%d)\n"), 
                       unit, rv));
            return rv;
        }
        
        /* check c3 */
        rv = wcmod_esm_serdes_control_get(unit, lane, SOC_PHY_CONTROL_PRBS_RX_STATUS, &value);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d Failed to start prbs rv(%d)\n"),
                       unit, rv));
            return rv;
        }

        /* Check Tcam */
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d PRBS failed at TCAM rv(%d)\n"), 
                       unit, rv));
            return rv;
        }
    }
    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_wait_for_pll_lock
 * Purpose: 
 *    Polls TXPLL lock for ntimeout times, each timeout is 2000us
 */
int
soc_sbx_caladan3_wcl_wait_for_txpll_lock(int unit, int ntimeouts) 
{

#define IS_WCL_TXPLL_LOCKED(r) (r & ((1 << SOC_SBX_CALADAN3_WCL_NUM)-1) << 12)

    uint32 regval = 0;
    /* check lock status */
    do {
        regval = 0;
        SOC_IF_ERROR_RETURN(READ_WCL_CUR_STSr(unit, &regval));
        if (IS_WCL_TXPLL_LOCKED(regval)) break;
        sal_usleep(2000);
    } while (ntimeouts--);

    if (ntimeouts <= 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d WCL:Failed while waiting for txpll lock %x"), unit,regval));
        return SOC_E_TIMEOUT;
    }
    return SOC_E_NONE;
}
/*
 * Function:
 *    soc_sbx_caladan3_etu_wait_for_rx_seq_done
 * Purpose: 
 *    Polls RxSeqDone ntimeout times, each timeout is 2000us
 */
int
soc_sbx_caladan3_wcl_wait_for_rx_seq_done(int unit, int ntimeouts) 
{

#define IS_WCL_RXSEQ_DONE(r) (r & ((1 << 12)-1))

    uint32 regval = 0;
    /* check lock status */
    do {
        regval = 0;
        SOC_IF_ERROR_RETURN(READ_WCL_CUR_STSr(unit, &regval));
        if (IS_WCL_RXSEQ_DONE(regval)) break;
        sal_usleep(2000);
    } while (ntimeouts--);

    if (ntimeouts <= 0) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d WCL:Failed while waiting for rxseqdone %x\n"), unit,regval));
        return SOC_E_TIMEOUT;
    }
    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_wcl_config
 * Purpose: 
 *    configure wcl serdes
 */
int
soc_sbx_caladan3_wcl_config(int unit)
{
    int rv = SOC_E_NONE, id = 0;
    uint32 regval = 0;

    if (SAL_BOOT_QUICKTURN) {
        return SOC_E_NONE;
    }
    for (id = 0; id < SOC_SBX_CALADAN3_WCL_NUM; ++id) {
        SOC_IF_ERROR_RETURN(READ_WCL_CTLr(unit, id, &regval));
        soc_reg_field_set(unit, WCL_CTLr, &regval, PWRDWNf, 1);
        soc_reg_field_set(unit, WCL_CTLr, &regval, IDDQf, 1);
        soc_reg_field_set(unit, WCL_CTLr, &regval, RSTB_HWf, 0);
        soc_reg_field_set(unit, WCL_CTLr, &regval, RSTB_MDIOREGSf, 0);
        soc_reg_field_set(unit, WCL_CTLr, &regval, RSTB_PLLf, 0);
        soc_reg_field_set(unit, WCL_CTLr, &regval, TXD1G_FIFO_RSTBf, 0);
        soc_reg_field_set(unit, WCL_CTLr, &regval, PLL_BYPASSf, 0);
        soc_reg_field_set(unit, WCL_CTLr, &regval, LCREF_ENf, 0); 
        soc_reg_field_set(unit, WCL_CTLr, &regval, REFOUT_ENf, 0); 
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
    }


    for (id = 0; id < SOC_SBX_CALADAN3_WCL_NUM; ++id) {
        SOC_IF_ERROR_RETURN(READ_WCL_CTLr(unit, id, &regval));
        soc_reg_field_set(unit, WCL_CTLr, &regval, IDDQf, 0);
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
    }
    sal_usleep(2000);
    for (id = 0; id < SOC_SBX_CALADAN3_WCL_NUM; ++id) {
        SOC_IF_ERROR_RETURN(READ_WCL_CTLr(unit, id, &regval));
        soc_reg_field_set(unit, WCL_CTLr, &regval, PWRDWNf, 0);
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
    }
    sal_usleep(2000);
    for (id = 0; id < SOC_SBX_CALADAN3_WCL_NUM; ++id) {
        SOC_IF_ERROR_RETURN(READ_WCL_CTLr(unit, id, &regval));
        if (id == SOC_SBX_CALADAN3_WCL_MASTER_ID) {
            soc_reg_field_set(unit, WCL_CTLr, &regval, LCREF_ENf, 0); 
            soc_reg_field_set(unit, WCL_CTLr, &regval, REFOUT_ENf, 1); 
        } else {
            soc_reg_field_set(unit, WCL_CTLr, &regval, LCREF_ENf, 1); 
            soc_reg_field_set(unit, WCL_CTLr, &regval, REFOUT_ENf, 0); 
        }
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
    }
    sal_usleep(2000);
    for (id = 0; id < SOC_SBX_CALADAN3_WCL_NUM; ++id) {
        SOC_IF_ERROR_RETURN(READ_WCL_CTLr(unit, id, &regval));
        soc_reg_field_set(unit, WCL_CTLr, &regval, RSTB_HWf, 1);
        soc_reg_field_set(unit, WCL_CTLr, &regval, RSTB_MDIOREGSf, 1);
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
    }
    sal_usleep(1000000);

    for (id = 0; id < SOC_SBX_CALADAN3_WCL_NUM; ++id) {
        SOC_IF_ERROR_RETURN(READ_WCL_CTLr(unit, id, &regval));
        soc_reg_field_set(unit, WCL_CTLr, &regval, RSTB_PLLf, 1);
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
        sal_usleep(2000);

        soc_reg_field_set(unit, WCL_CTLr, &regval, TXD1G_FIFO_RSTBf, 0xf);
        SOC_IF_ERROR_RETURN(
               WRITE_WCL_CTLr(unit, id, regval));
    }

    sal_usleep(1000000);
    rv = soc_sbx_caladan3_wcl_wait_for_txpll_lock(unit, 10);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "Failed while waiting for WCL TXPLL lock\n")));
    }


    /* Serdes probe and init *|
    rv = wcmod_esm_serdes_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "Failed while initialing wcmod for esm serdes rv=(%d)\n"), rv));
    }
    |* Wait for RxSeqDone *|
    rv = soc_sbx_caladan3_wcl_wait_for_rx_seq_done(unit, 10);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "Failed while waiting for WCL RxSeqDone \n")));
    }*/
 

    return rv;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_tcam_config
 * Purpose: 
 *    Bringup NL11K TCAM
 */
int
soc_sbx_caladan3_etu_tcam_config(int unit)
{
    int i, rv = SOC_E_NONE;
    int rx_fifo_thr = 0; /* Leave at default */ 
    int tx_fifo_thr =0; /* Leave at default */ 
    int tx_burst_short_16b = 1;
    int tx_swap, rx_swap;
    uint32 etu_config4_buf, tx_cfg_buf;

    /* TCAM reset seq */
    SOC_IF_ERROR_RETURN(READ_ETU_CONFIG4r(unit, &etu_config4_buf));
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, EXT_TCAM_SRST_Lf, 0);
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, EXT_TCAM_CRST_Lf, 0);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));
    sal_usleep(1000000);        

    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, EXT_TCAM_SRST_Lf, 1);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));

    sal_usleep(1000000);        

    /* Detect and Enable Ext TCAMs */
    rv = soc_sbx_caladan3_etu_tcam_detect(unit, &_soc_sbx_caladan3_etu_num_tcam[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed Detecting TCAM, status(%d)\n"), unit, rv));
        return rv;
    }

    /* Init sequence */
    tx_swap = soc_property_get(unit, spn_EXT_TCAM_TX_LANE_SWAP, 0);
    rx_swap = soc_property_get(unit, spn_EXT_TCAM_RX_LANE_SWAP, 0);
    rv = soc_etu_nl_mdio_init_seq(unit, _soc_sbx_caladan3_etu_num_tcam[unit], 
                                  rx_fifo_thr, tx_fifo_thr, 
                                  rx_swap, tx_swap,
                                  tx_burst_short_16b);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed to init TCAM rv(%d)\n"), unit, rv));
    }

    sal_usleep(300000);        
    soc_reg_field_set(unit, ETU_CONFIG4r, &etu_config4_buf, EXT_TCAM_CRST_Lf, 1);
    SOC_IF_ERROR_RETURN(
           WRITE_ETU_CONFIG4r(unit, etu_config4_buf));

    sal_usleep(300000);        

    /* Get the TCAM running */
    rv = soc_etu_nl_mdio_init_ready(unit, _soc_sbx_caladan3_etu_num_tcam[unit]);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed to initialize TCAM rv(%d)\n"), unit, rv));
    }

    /* Read and print Dev id */
    for (i=0; i < _soc_sbx_caladan3_etu_num_tcam[unit]; i++) {
        soc_etu_nl_device_id_print(unit, i);
    }

    /* Enable ILA Tx */
    SOC_IF_ERROR_RETURN(READ_ILAMAC_TX_CONFIG0r(unit, &tx_cfg_buf));
    soc_reg_field_set(unit, ILAMAC_TX_CONFIG0r, &tx_cfg_buf, TX_ENABLEf, 1);
    SOC_IF_ERROR_RETURN(
               WRITE_ILAMAC_TX_CONFIG0r(unit, tx_cfg_buf));

    return rv;
}

int
soc_sbx_caladan3_etu_wait_for_sync(int unit) 
{
    int i, rv;
    /* Wait for Sync */
    for (i=0; i < SOC_SBX_CALADAN3_WCL_NUM_RX_LANES; i++) {
        rv = SOC_SBX_CALADAN3_POLL_REGISTER(unit, ILAMAC_RX_INTF_STATE0r, RX_ALIGNEDf, 1);
        if (SOC_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit %d, ILAMAC Rx Align failed %d\n"), unit, rv));
        } else {
            rv = SOC_SBX_CALADAN3_POLL_REGISTER(unit, ILAMAC_RX_INTF_STATE0r, RX_SYNCEDf, 0xfff);
            if (SOC_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_SOC_COMMON,
                          (BSL_META_U(unit,
                                      "Unit %d, ILAMAC Rx SYNC failed %d\n"), unit, rv));
            } else {
                rv = SOC_SBX_CALADAN3_POLL_REGISTER(unit, ILAMAC_RX_INTF_STATE0r, RX_WORD_SYNCf, 0xfff);
                if (SOC_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_SOC_COMMON,
                              (BSL_META_U(unit,
                                          "Unit %d, ILAMAC Rx WORD Sync failed %d\n"), unit, rv));
                }
            }
        }
    }
    /* Clear counters */
    if (SOC_SUCCESS(rv)) {
        soc_sbx_caladan3_etu_ilamac_clear_errors(unit);
    }

    return rv;
}

int
soc_sbx_caladan3_wcl_rx_sequencer_reset(int unit) 
{

    int nESM;
    int g_anESM_PhyIDs[] = { 0xe1, 0xe5, 0xe9, 0xed, 0xf1, 0xf5 };

    /* Disable Rx Sequencer */
    for ( nESM = 0; nESM < 6; ++nESM ) {
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1f, 0xffd0 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1e, 0x01ff )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1f, 0x8210 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1e, 0xa000 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1f, 0xffd0 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1e, 0x0000 )); 
    }

    /* Enable Rx Sequencer */
    for ( nESM = 0; nESM < 6; ++nESM ) {
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1f, 0xffd0 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1e, 0x01ff )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1f, 0x8210 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1e, 0x2000 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1f, 0xffd0 )); 
	SOC_IF_ERROR_RETURN(soc_miim_write( unit, g_anESM_PhyIDs[nESM], 0x1e, 0x0000 )); 
    }

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_driver_uninit
 * Purpose: 
 *    Cleanup after ETU
 */
int
soc_sbx_caladan3_etu_driver_uninit(int unit)
{
    uint32 uRegisterValue;
    
    READ_CX_CONFIGr( unit, &uRegisterValue );
    soc_reg_field_set( unit, CX_CONFIGr, &uRegisterValue, ETU_CLK_DISABLEf, 1 );
    WRITE_CX_CONFIGr( unit, uRegisterValue );

    return SOC_E_NONE;
}

/*
 * Function:
 *    soc_sbx_caladan3_etu_driver_init
 * Purpose: 
 *    Enable and configure ETU
 */
int
soc_sbx_caladan3_etu_driver_init(int unit)
{
#ifdef USE_HPPC_TEST
    c3hppc_etu_interface_bringup(0);
#else
    int rv = SOC_E_NONE;
    uint16 dev_id;
    uint8 rev_id;
	uint32 uRegisterValue;

    if (!soc_property_get(unit, spn_CALADAN3_ETU_ENABLE, TRUE)) {
	 READ_CX_CONFIGr( unit, &uRegisterValue );	  
	 soc_reg_field_set( unit, CX_CONFIGr, &uRegisterValue, ETU_CLK_DISABLEf, 1 );	 
	 WRITE_CX_CONFIGr( unit, uRegisterValue ); 
	
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_ALIGNMENT_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_ALIGNMENT_FAILURES_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_BAD_TYPE_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_BURSTMAX_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_CRC_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_DESCRAM_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_FRAMING_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_LANE_CRC_ERRORS_COUNTr] = NULL;
	 SOC_DRIVER(unit)->reg_info[ILAMAC_RX_WORD_SYNC_ERRORS_COUNTr] = NULL;
	
	 return SOC_E_NONE;
    } /* if (not enabling Caladan3 ETU, with default to enable) */

    soc_cm_get_id(unit, &dev_id, &rev_id);
    if (dev_id == BCM88034_DEVICE_ID) {
	/* etu not supported on 88034 */
	return rv;
    }

    if (SAL_BOOT_BCMSIM) {
	/* do nothing for now under bcmsim until model is ready */
	return rv;
    }

    /* Enable WCL */
    rv = soc_sbx_caladan3_wcl_config(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed initializing WCL, status(%d)\n"), unit, rv));
        return rv;
    }

    /* Serdes probe and init */
    rv = wcmod_esm_serdes_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed while initialing wcmod for esm serdes rv=(%d)\n"), rv));
    }

    /* Enable ETU */
    rv = soc_sbx_caladan3_etu_config(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed initializing ETU, status(%d)\n"), unit, rv));
        return rv;
    }

    rv = soc_sbx_caladan3_wcl_rx_sequencer_reset(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed to reset WCL, status(%d)\n"), unit, rv));
        return rv;
    }

    /* Wait for RxSeqDone *|
    |* SEQDONE not triggering *|
    rv = soc_sbx_caladan3_wcl_wait_for_rx_seq_done(unit, 10);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Failed while waiting for WCL RxSeqDone \n")));
    } */

    rv = soc_sbx_caladan3_etu_tcam_config(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed initializing TCAM, status(%d)\n"), 
                   unit, rv));
    }

    soc_sbx_caladan3_etu_wait_for_sync(unit);

    /* Verify connectivity and status */
    /* Attempt PRBS and verify connectivity *|
    rv = soc_sbx_caladan3_etu_prbs(unit);
    if (SOC_FAILURE(rv)) {
        LOG_CLI((BSL_META_U(unit,
                            "Failed while initialing wcmod for esm serdes rv=(%d)\n"), rv));
    } */

    rv = soc_sbx_caladan3_etu_fifo_init(unit);
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_SOC_COMMON,
                  (BSL_META_U(unit,
                              "Unit %d Failed ETU Fifo init, status(%d)\n"), 
                   unit, rv));
    }

#endif
    return SOC_E_NONE;
}


void
soc_sbx_caladan3_etu_key_capture_setup(int unit) {
}

void
soc_sbx_caladan3_etu_key_capture_dump(int unit) {
}


#endif
