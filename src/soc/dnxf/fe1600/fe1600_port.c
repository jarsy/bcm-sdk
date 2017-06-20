/*
 * $Id: ramon_fe1600_port.c,v 1.47 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * SOC RAMON_FE1600 PORT
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/mem.h>
#include <soc/defs.h>
#include <soc/phyreg.h>
#include <soc/error.h>
#include <soc/mcm/allenum.h>
#include <soc/mcm/memregs.h>

#include <shared/bitop.h>
#include <bcm/port.h>
#include <soc/register.h>

#include <shared/util.h>
#if defined(BCM_88790_A0)
#include <soc/dnxc/legacy/dnxc_defs.h>

#include <soc/dnxf/cmn/dnxf_drv.h>
#include <soc/dnxf/cmn/dnxf_port.h>
#include <soc/dnxf/cmn/dnxf_warm_boot.h>

#include <soc/dnxf/fe1600/fe1600_defs.h>
#include <soc/dnxf/fe1600/fe1600_stat.h>
#include <soc/dnxf/fe1600/fe1600_fabric_links.h>
#include <soc/dnxf/fe1600/fe1600_port.h>
#include <soc/dnxf/fe1600/fe1600_drv.h>
#include <soc/dnxf/fe1600/fe1600_config_imp_defs.h>

extern int (*_phy_wcmod_firmware_set_helper[SOC_MAX_NUM_DEVICES]) (int unit , int port, uint8 *arr,int arr_len);
typedef int (* dnxf_wcmod_firmware_func)(int, int, uint8 *,int);

STATIC int
soc_ramon_fe1600_force_signal_detect_set(int unit, int mac_instance)
{
   uint32 force_signal_detect, base_port, i;
   soc_dnxc_loopback_mode_t loopback;
   soc_dnxc_port_pcs_t pcs;
   uint32 reg_val;
   DNXC_INIT_FUNC_DEFS;
   SOC_RAMON_FE1600_ONLY(unit);

   force_signal_detect = 0;
   base_port = SOC_RAMON_FE1600_NOF_LINKS_IN_MAC*mac_instance;

   for(i=0 ; i<SOC_RAMON_FE1600_NOF_LINKS_IN_MAC ; i++) {
       DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_loopback_get(unit, base_port+i, &loopback));
       if(soc_dnxc_loopback_mode_mac_pcs == loopback || soc_dnxc_loopback_mode_mac_outer == loopback || soc_dnxc_loopback_mode_mac_async_fifo == loopback) {
           force_signal_detect = 1;
           break;
       }

       if(SOC_IS_FE1600_A0(unit)) {
           DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, base_port+i, &pcs));
           if(soc_dnxc_port_pcs_64_66_fec == pcs || soc_dnxc_port_pcs_64_66_bec == pcs) {
               force_signal_detect = 1;
               break;
           }
       }

   }

#ifdef BCM_88754_A0
   if (SOC_IS_BCM88754_A0(unit))
   {
       int lane;

       for (lane = 0; lane < SOC_RAMON_FE1600_NOF_LINKS_IN_MAC; lane++)
       {
           DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, mac_instance, lane, &reg_val));
           soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg_val, FMAL_N_FORCE_SIGNAL_DETECTf, force_signal_detect);
           DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, mac_instance, lane, reg_val));
       }
   } else 
#endif /*BCM_88754_A0*/
   {
       DNXC_IF_ERR_EXIT(READ_FMAC_GENERAL_CONFIGURATION_REGISTERr_REG32(unit, mac_instance, &reg_val));
       soc_reg_field_set(unit, FMAC_GENERAL_CONFIGURATION_REGISTERr, &reg_val, FORCE_SIGNAL_DETECTf, force_signal_detect);
       DNXC_IF_ERR_EXIT(WRITE_FMAC_GENERAL_CONFIGURATION_REGISTERr_REG32(unit, mac_instance, reg_val));
   }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_soc_init
 * Purpose:
 *      Initalize port module
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      SOC_E_NONE     No Error  
 *      SOC_E_UNAVAIL  Feature unavailable  
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_ramon_fe1600_port_soc_init(int unit)
{
    soc_port_t inner_port, port_i;
    int blk_id;
    uint32 reg_val, lcpll, puc, pll_control_1;
    int rv;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*set port refclk*/
    for (port_i = 0; port_i < SOC_DNXF_DEFS_GET(unit ,nof_links); ++port_i) {
        lcpll = INT_DEVIDE(port_i, SOC_RAMON_FE1600_NOF_PORT_IN_LCPLL);
        DNXC_IF_ERR_EXIT(READ_ECI_POWER_UP_CONFIGURATIONr(unit, &puc));

#ifdef BCM_88754_A0
        if (SOC_IS_BCM88754_A0(unit))
        {
            /*Might be changed according to link rate*/
            SOC_INFO(unit).port_refclk_int[port_i] = 156;
        } else
#endif /*BCM_88754_A0*/
        if(SHR_BITGET(&puc,7)) {
            /*if PUC[7]==1 then LC PLL config is taken from CMIC regs*/
            switch(lcpll) {
            case 0:
                DNXC_IF_ERR_EXIT(READ_CMIC_XGXS0_PLL_CONTROL_1r(unit, &pll_control_1));
                break;
            case 1:
                DNXC_IF_ERR_EXIT(READ_CMIC_XGXS1_PLL_CONTROL_1r(unit, &pll_control_1));
                break;
            case 2:
                DNXC_IF_ERR_EXIT(READ_CMIC_XGXS2_PLL_CONTROL_1r(unit, &pll_control_1));
                break;
            case 3:
            default:
                DNXC_IF_ERR_EXIT(READ_CMIC_XGXS3_PLL_CONTROL_1r(unit, &pll_control_1));
                break;
            }
            if(SOC_RAMON_FE1600_LCPLL_CONTROL1_125_VAL == pll_control_1) { /*control word for 125MHz*/
                SOC_INFO(unit).port_refclk_int[port_i] = 125;
            } else if(SOC_RAMON_FE1600_LCPLL_CONTROL1_156_25_VAL == pll_control_1){ /*control word for 156.25MHz*/
                SOC_INFO(unit).port_refclk_int[port_i] = 156;
            } else {
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Unrecognized LC-PLL value")));
            }
        } else {
            /*if PUC[7]==0 then LC PLL config is taken from the PUC*/
            switch(lcpll) {
            case 0:
                SOC_INFO(unit).port_refclk_int[port_i] = SHR_BITGET(&puc,8)  ? 125 : 156;
                break;
            case 1:
                SOC_INFO(unit).port_refclk_int[port_i] = SHR_BITGET(&puc,10) ? 125 : 156;
                break;
            case 2:
                SOC_INFO(unit).port_refclk_int[port_i] = SHR_BITGET(&puc,12) ? 125 : 156;
                break;
            case 3:
            default:
                SOC_INFO(unit).port_refclk_int[port_i] = SHR_BITGET(&puc,14) ? 125 : 156;
                break;
            }
        }
    }

    /* 
     * Initalize the comma and control burst structures. 
     * It is done by reading the information from registers. 
     */
    for (port_i = 0; port_i < SOC_DNXF_DEFS_GET(unit, nof_links); ++port_i)
    {
        blk_id = INT_DEVIDE(port_i, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
        inner_port = port_i % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
        rv = SOC_DNXF_WARM_BOOT_ARR_VAR_SET(unit, PORT_COMMA_BURST_CONF, port_i, &reg_val);
        DNXC_IF_ERR_EXIT(rv);
    }

    for (blk_id = 0; blk_id < SOC_DNXF_DEFS_GET(unit, nof_instances_mac); ++blk_id)
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, &reg_val));
        rv = SOC_DNXF_WARM_BOOT_ARR_VAR_SET(unit, PORT_CTRL_BURST_CONF, blk_id, &reg_val);
        DNXC_IF_ERR_EXIT(rv);
    }
    
exit:
    DNXC_FUNC_RETURN;
}


soc_error_t 
soc_ramon_fe1600_port_burst_control_set(int unit, soc_port_t port, soc_dnxc_loopback_mode_t loopback)
{
    /*Do nothing*/
    return SOC_E_NONE;
}


STATIC
soc_error_t 
soc_ramon_fe1600_wcmod_firmware_register(int unit, dnxf_wcmod_firmware_func func)
{
    DNXC_INIT_FUNC_DEFS;

    _phy_wcmod_firmware_set_helper[unit] = func;

    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_init
 * Purpose:
 *      Initalize port module
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Returns:
 *      SOC_E_NONE     No Error  
 *      SOC_E_UNAVAIL  Feature unavailable  
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_ramon_fe1600_port_init(int unit)
{
    int                 rc;
    int port_local, clause = 22;
    uint16 phy_addr, data_mmd_select, data_lane_reset, data_lane_reset_new;
    uint16 data_lane_reset_cur;
    int qrtt_active, i;
    DNXC_INIT_FUNC_DEFS;

    /*register wcmod_load func*/
    rc = soc_ramon_fe1600_wcmod_firmware_register(unit, soc_ramon_fe1600_port_wcmod_ucode_load);

    if ((rc = soc_phy_common_init(unit)) != SOC_E_NONE) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("Failed software port init: %s"), soc_errmsg(rc)));
    }
    MIIM_LOCK(unit);
    rc = soc_phyctrl_software_init(unit);
    MIIM_UNLOCK(unit);    
    if (rc != SOC_E_NONE) {
        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("Failed soc_phyctrl_software_init %s"), soc_errmsg(rc)));
    }

    if (!SOC_WARM_BOOT(unit) && !SOC_IS_RELOADING(unit)) {
        if(!SAL_BOOT_PLISIM) {

            /*Core reset - follows sequence of wcmod_core_reset.
              During the process phyctrl_probe_init the following stages are performed (in a loop per port):
              1. Cores reset
              2. FW load
              3. rest of init

              For fabric link FW load is done in broadcast to all cores for the first port that gets into FW load.
              To avoid FW override by the Core reset procedure the core reset is done at this stage.*/

            data_mmd_select = (0x0001 |
                               0x0002 |
                               0x0004 |
                               0x0008 |
                               0x4000);

            data_lane_reset = (0x8000 |
                               0x00f0 |
                               0x000f);


            for (port_local=0; port_local<SOC_DNXF_DEFS_GET(unit, nof_links); port_local+=SOC_RAMON_FE1600_NOF_LINKS_IN_MAC) {

                qrtt_active = 0;
                for (i=0; i<4; i++) {
                    if (SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_local + i)) {
                        qrtt_active = 1;
                        break;
                    }
                }
                if (qrtt_active == 0) {
                     continue;
                }
         
                phy_addr = PORT_TO_PHY_ADDR_INT(unit, port_local);

                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause, phy_addr, 0x1f, 0xffd0));
                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause, phy_addr, 0x1e, 0x0));
                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause , phy_addr, 0x1f, 0x8000)); 
                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause, phy_addr, 0x1d, data_mmd_select));
                 
                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause , phy_addr, 0x1f, 0x8100)); 
                DNXC_IF_ERR_EXIT(soc_dcmn_miim_read(unit, clause, phy_addr, 0x1a, &data_lane_reset_cur));

                data_lane_reset_new = data_lane_reset | data_lane_reset_cur;

                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause, phy_addr, 0x1a, data_lane_reset_new));

            }

            sal_usleep(10);

            for (port_local=0; port_local<SOC_DNXF_DEFS_GET(unit, nof_links); port_local+=SOC_RAMON_FE1600_NOF_LINKS_IN_MAC) {

                qrtt_active = 0;
                for (i=0; i<4; i++) {
                    if (SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_local + i)) {
                        qrtt_active = 1;
                        break;
                    }
                }
                if (qrtt_active == 0) {
                     continue;
                }

                phy_addr = PORT_TO_PHY_ADDR_INT(unit, port_local);

                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause , phy_addr, 0x1f, 0x8000)); 
                DNXC_IF_ERR_EXIT(soc_dcmn_miim_write(unit, clause, phy_addr, 0x1d, data_mmd_select)); 

            }

        }
    }

exit:
    DNXC_FUNC_RETURN; 
}


soc_error_t 
soc_ramon_fe1600_port_detach(int unit, soc_port_t port)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
   
    rc = soc_phyctrl_detach(unit, port);
    DNXC_IF_ERR_EXIT(rc);
exit:
    DNXC_FUNC_RETURN;
}


int
soc_ramon_fe1600_port_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp, int is_init_sequence)
{
    int rc = SOC_E_NONE;
    bcm_port_t port;
    int pcs, speed, cl72, rx_polarity, tx_polarity,
                    rxlane_map, txlane_map, lane_map;
    char *str;

    DNXC_INIT_FUNC_DEFS;


    rc = soc_phyctrl_pbm_probe_init(unit,pbmp,okay_pbmp);
    DNXC_IF_ERR_EXIT(rc);

    if (!SOC_WARM_BOOT(unit)) {
        SOC_PBMP_ITER(pbmp, port) {

            str = soc_dnxf_property_port_get_str(unit, spn_BACKPLANE_SERDES_ENCODING, port, 0, NULL);
            rc = soc_dnxf_property_str_to_enum(unit, spn_BACKPLANE_SERDES_ENCODING, soc_dnxf_property_str_enum_backplane_serdes_encoding, str, &pcs);

            speed = soc_property_port_get(unit, port, spn_PORT_INIT_SPEED, 6250);
            cl72 = soc_property_port_get(unit, port, spn_PORT_INIT_CL72, 0) ? 1 : 0;

            rx_polarity = soc_property_port_get(unit, port, spn_PB_SERDES_LANE_SWAP_POLARITY_RX, FALSE);
            tx_polarity = soc_property_port_get(unit, port, spn_PB_SERDES_LANE_SWAP_POLARITY_TX, FALSE);

            rxlane_map = soc_property_port_get(unit, port, spn_XGXS_RX_LANE_MAP, DNXF_PORT_RX_LANE_MAP_DEFAULT);
            txlane_map = soc_property_port_get(unit, port, spn_XGXS_TX_LANE_MAP, DNXF_PORT_TX_LANE_MAP_DEFAULT);
            lane_map = (txlane_map << 16) + rxlane_map;

            LOG_VERBOSE(BSL_LS_SOC_PORT,
                        (BSL_META_U(unit,
                                    "unit %d: port %s - PCS: %d, Speed: %d, CL72: %d, rx_polarity: %d, tx_polarity: %d, lane_map: 0x%08x\n"), unit, SOC_PORT_NAME(unit, port), pcs, speed, cl72, rx_polarity, tx_polarity, lane_map));

            DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_LANE_SWAP , lane_map)); 

            /* Probe function should leave port disabled */
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_phy_enable_set(unit, port, 0));
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_set(unit, port, 0, soc_dnxc_port_power_off));

            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_set(unit, port, pcs));
            
            /* BCM_IF_ERR_EXIT(bcm_port_speed_set(unit, port, speed));  */
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_phy_cl72_set(unit, port, cl72));
            DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_RX_POLARITY, rx_polarity));
            DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_TX_POLARITY, tx_polarity));

#ifdef BCM_88790_A0
            if (SOC_IS_FE1600_B0_AND_ABOVE(unit)) {
                /*Support for mixed rate in a quad*/
                
                uint32 os, val;
                uint64 reg64_val;
                int quad, lane;

                rc = soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_OVERSAMPLE_MODE, &os);
                DNXC_IF_ERR_EXIT(rc);
                
                quad = port / SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;
                lane = port % SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;

                val = (os == 0 ? 1 : 0);
                if (SOC_IS_BCM88754_A0(unit))
                {
#ifdef BCM_88754_A0
                    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, quad, lane, ITEM_6f /*RX full rate enable*/, val));
                    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, quad, lane, ITEM_13f /*TX full rate enable*/, val));
#endif /*BCM_88754_A0*/
                } else {
                    DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, quad, lane, &reg64_val));
                    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_N_RX_FULL_RATE_ENf, val);
                    soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_N_TX_FULL_RATE_ENf, val);
                    DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, quad, lane, reg64_val));
                }
            }
#endif
               
        }
    }

 exit:
    DNXC_FUNC_RETURN; 
    
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_pcs_set
 * Purpose:
 *      Set port PCS
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      port  - (IN)     Device or logical port number  
 *      pcs   - (IN)     PCS for port
 * Returns:
 *      SOC_E_NONE     No Error  
 *      SOC_E_UNAVAIL  Feature unavailable  
 *      SOC_E_PORT     Invalid Port number specified  
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_ramon_fe1600_port_control_pcs_set(int unit, soc_port_t port, soc_dnxc_port_pcs_t pcs)
{
    soc_port_t inner_port;
    int blk_id;
    int disable_fec;
    int set_high_latency=0;
    int disable_error_marking=0;
    uint32 reg_val32,  
           encoding_type, 
           syndrom,
           tx_ack_mask_period,
           rx_fec_fec_en,
           tx_fec_en;
    uint64 async_fifo_config;
    uint32 slow_read_rate;
    soc_dnxc_port_power_t power;
    uint32 rx_serdes_reset;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;
   
    switch(pcs)
    {
        case soc_dnxc_port_pcs_8_9_legacy_fec:
            encoding_type = 0;
            slow_read_rate = 1;
            syndrom = 0;
            tx_ack_mask_period = 8;
            disable_fec=0;
            set_high_latency=1;
            disable_error_marking=1;
            break;
        case soc_dnxc_port_pcs_8_10:
            encoding_type = 1;
            slow_read_rate = 1;
            syndrom = 0;
            tx_ack_mask_period = 8;
            disable_fec=0;
            set_high_latency=1;
            disable_error_marking=1;
            break;
        case soc_dnxc_port_pcs_64_66_fec:
            encoding_type = 2;
            slow_read_rate = 0;
            syndrom = 0;
            tx_ack_mask_period = 8;
            disable_fec=0;
            break;
        case soc_dnxc_port_pcs_64_66_bec:
            encoding_type =3;
            slow_read_rate = 0;
            syndrom = 1;
            tx_ack_mask_period = 4;
            disable_fec=0;
            disable_error_marking=1;
            break;
        case soc_dnxc_port_pcs_64_66:
            encoding_type = 2;
            slow_read_rate = 0;
            syndrom = 0;
            tx_ack_mask_period = 8;
            disable_fec=1;
            set_high_latency=1;
            disable_error_marking=1;
            break;

        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Wrong PCS value %d"),pcs));
    }

    /* 
     * First disable FMAC
     */
    DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_RX_RESET, &rx_serdes_reset));
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_get(unit,port,&power));
    if(soc_dnxc_port_power_on == power) {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_set(unit,port,rx_serdes_reset ? SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE : 0, soc_dnxc_port_power_off));
    }

    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
    soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg_val32, FMAL_N_ENCODING_TYPEf, encoding_type);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));

#ifdef BCM_88754_A0
    if (SOC_IS_BCM88754_A0(unit))
    {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, blk_id, inner_port, FMAL_RX_SLOW_READ_RATEf, slow_read_rate));
    } else
#endif /*BCM_88754_A0*/
    {

        DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, blk_id, inner_port, &async_fifo_config));
        soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &async_fifo_config, FMAL_N_RX_SLOW_READ_RATEf, slow_read_rate);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, blk_id, inner_port, async_fifo_config));
    }

    DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
    soc_reg_field_set(unit, FMAC_FPS_RX_FEC_CONFIGURATIONr, &reg_val32, FPS_N_RX_FEC_SYNDROM_SELf, syndrom);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));

    DNXC_IF_ERR_EXIT(READ_FMAC_FPS_TX_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
    soc_reg_field_set(unit, FMAC_FPS_TX_CONFIGURATIONr, &reg_val32, FPS_N_TX_FEC_SYNDROM_SELf, syndrom);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FPS_TX_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));

    DNXC_IF_ERR_EXIT(READ_FMAC_BEC_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
    soc_reg_field_set(unit, FMAC_BEC_CONFIGURATIONr, &reg_val32, BEC_N_TX_SAME_ACK_MASK_PERIODf, tx_ack_mask_period);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_BEC_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_force_signal_detect_set(unit, blk_id));

    if (disable_fec)
    {
        rx_fec_fec_en=0;
        tx_fec_en=0;
    } else
    {
        rx_fec_fec_en=1;
        tx_fec_en=1;
    }

    DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));       /* if we want to disable fec, we will write 0 to disable rx*/
    soc_reg_field_set(unit,FMAC_FPS_RX_FEC_CONFIGURATIONr,&reg_val32,FPS_N_RX_FEC_FEC_ENf,rx_fec_fec_en); 
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));


    DNXC_IF_ERR_EXIT(READ_FMAC_FPS_TX_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32)); /* if we want to disable fec, we will write 0 to disable tx*/
    soc_reg_field_set(unit,FMAC_FPS_TX_CONFIGURATIONr,&reg_val32,FPS_N_TX_FEC_ENf,tx_fec_en);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FPS_TX_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));

    if (set_high_latency)
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
        soc_reg_field_set(unit,FMAC_FMAL_GENERAL_CONFIGURATIONr,&reg_val32,FMAL_N_LOW_LATENCY_LLFCf,0);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
    }

    if (disable_error_marking)
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
        soc_reg_field_set(unit,FMAC_FPS_RX_FEC_CONFIGURATIONr,&reg_val32,FPS_N_RX_FEC_ERR_MARK_ALLf,0);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
    }



    /*
     *  Enable back FMAC
     */
    if(soc_dnxc_port_power_on == power) {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_set(unit,port, rx_serdes_reset ? SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE : 0, soc_dnxc_port_power_on));
    }
   
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_pcs_get
 * Purpose:
 *      Set port PCS
 * Parameters:
 *      unit  - (IN)     Unit number.
 *      port  - (IN)     Device or logical port number  
 *      pcs   - (OUT)    PCS for port
 * Returns:
 *      SOC_E_NONE     No Error  
 *      SOC_E_UNAVAIL  Feature unavailable  
 *      SOC_E_PORT     Invalid Port number specified  
 *      SOC_E_XXX      Error occurred  
 */
soc_error_t 
soc_ramon_fe1600_port_control_pcs_get(int unit, soc_port_t port, soc_dnxc_port_pcs_t* pcs)
{
    soc_port_t inner_port;
    int blk_id,nof_links_in_mac;
    uint32 general_config,
           reg_val32,
           rx_fec_fec_en,  
           encoding_type;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    nof_links_in_mac = SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);

    blk_id = INT_DEVIDE(port, nof_links_in_mac);
    inner_port = port % nof_links_in_mac;
    
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &general_config));
    encoding_type = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, general_config, FMAL_N_ENCODING_TYPEf);

    DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
    rx_fec_fec_en=soc_reg_field_get(unit,FMAC_FPS_RX_FEC_CONFIGURATIONr,reg_val32,FPS_N_RX_FEC_FEC_ENf);

    
    switch(encoding_type)
    {
        case 0: 
            *pcs = soc_dnxc_port_pcs_8_9_legacy_fec;
            break;
        case 1: 
            *pcs = soc_dnxc_port_pcs_8_10;
            break;
        case 2: 
            if(rx_fec_fec_en) *pcs = soc_dnxc_port_pcs_64_66_fec;
            else              *pcs = soc_dnxc_port_pcs_64_66;
            break;
        case 3: 
            *pcs = soc_dnxc_port_pcs_64_66_bec;
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Can't retrive PCS value %d"),encoding_type));  
    }
    
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_power_set
 * Purpose:
 *      Turn on/off port power
 * Parameters:
 *      unit     -  (IN) Unit number.
 *      port     -  (IN) fabric port #
 *      flags    -  (IN) relevant flags: SOC_DNXF_PORT_CONTROL_RX_SERDES_IGNORE
 *      power    -  (IN) on/off
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_power_set(int unit, soc_port_t port, uint32 flags, soc_dnxc_port_power_t power)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_rx_enable_set(unit, port,flags, soc_dnxc_port_power_on == power ? 1 : 0));
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_tx_enable_set(unit, port, soc_dnxc_port_power_on == power ? 1 : 0));
    
exit:
    DNXC_FUNC_RETURN;     
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_power_get
 * Purpose:
 *      Get port power state
 * Parameters:
 *      unit     - (IN)  Unit number.
 *      group    - (IN)  Multicast group ID
 *      port     - (OUT) Physical GPORT ID 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_power_get(int unit, soc_port_t port, soc_dnxc_port_power_t* power)
{
    int rx,tx;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_rx_enable_get(unit, port, &rx));
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_tx_enable_get(unit, port, &tx));
    
    if(rx && tx) {
        *power = soc_dnxc_port_power_on;
    }
    else {
        *power = soc_dnxc_port_power_off;
    }
    
exit:
    DNXC_FUNC_RETURN;    
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_strip_crc_set
 * Purpose:
 *      Enable / Disable strip CRC
 * Parameters:
 *      unit      - (IN) Unit number.
 *      port      - (IN) Device or logical port number 
 *      strip_crc - (IN) 1- Enable strip CEC, 0 - Disable strip CRC
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_strip_crc_set(int unit, soc_port_t port, int strip_crc)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 general_config;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;
    
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &general_config));
    soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &general_config, FMAL_N_ENABLE_CELL_CRCf, strip_crc ? 0 : 1);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, general_config));
    
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_strip_crc_get
 * Purpose:
 *      Get strip CRC state
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  Device or logical port number 
 *      strip_crc - (OUT) 1- Strip CEC enabled, 0 - Strip CRC disable
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_strip_crc_get(int unit, soc_port_t port, int* strip_crc)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 general_config;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);
           
    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;
    
    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &general_config));
    *strip_crc = !soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, general_config, FMAL_N_ENABLE_CELL_CRCf);

    
exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_rx_enable_set
 * Purpose:
 *      Enable\disable RX
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number
 *      flags     - (IN)  relevant flags: SOC_DNXF_PORT_CONTROL_RX_SERDES_IGNORE
 *      enable    - (IN)  1- Enable 0- Disable
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_rx_enable_set(int unit, soc_port_t port, uint32 flags, int enable)
{
    soc_port_t inner_port;
    int blk_id,nof_links_in_mac;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;

    nof_links_in_mac = SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);
    blk_id = INT_DEVIDE(port, nof_links_in_mac);
    inner_port = port % nof_links_in_mac;

    if(enable) {
        if (!(flags & SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE))
        {
            DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_RX_RESET , 0));
        }
        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
        SHR_BITCLR(field_val,inner_port);
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf,*field_val);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,reg_val));
    } else {
        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
        SHR_BITSET(field_val,inner_port);
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_RX_RST_Nf,*field_val);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,reg_val));
        if (!(flags & SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE))
        {
            DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_RX_RESET , 1));
        }
    }

exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_tx_enable_set
 * Purpose:
 *      Enable\disable TX
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable    - (IN)  1- Enable 0- Disable
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_tx_enable_set(int unit, soc_port_t port, int enable)
{
    soc_port_t inner_port;
    int blk_id,nof_links_in_mac;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;

    nof_links_in_mac = SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);
    blk_id = INT_DEVIDE(port, nof_links_in_mac);
    inner_port = port % nof_links_in_mac;

    if(enable) {
        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);
        SHR_BITCLR(field_val,inner_port);
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf,*field_val);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,reg_val));
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_TX_RESET , 0));
    } else {
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_TX_RESET , 1));
        DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);
        SHR_BITSET(field_val,inner_port);
        soc_reg_field_set(unit, FMAC_RECEIVE_RESET_REGISTERr, &reg_val, FMAC_TX_RST_Nf,*field_val);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,reg_val));
    }

exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_rx_enable_get
 * Purpose:
 *      Get RX status (Enabled\disabled)
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable    - (OUT) 1- Enable 0- Disable
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_rx_enable_get(int unit, soc_port_t port, int* enable)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_RX_RST_Nf);
    *enable = SHR_BITGET(field_val,inner_port) ? 0 : 1;

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_tx_enable_get
 * Purpose:
 *      Get TX status (Enabled\disabled)
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable    - (OUT) 1- Enable 0- Disable
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_control_tx_enable_get(int unit, soc_port_t port, int* enable)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 reg_val, field_val[1];
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    DNXC_IF_ERR_EXIT(READ_FMAC_RECEIVE_RESET_REGISTERr(unit,blk_id,&reg_val));
    *field_val = soc_reg_field_get(unit, FMAC_RECEIVE_RESET_REGISTERr, reg_val, FMAC_TX_RST_Nf);
    *enable = SHR_BITGET(field_val,inner_port) ? 0 : 1;

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_loopback_set
 * Purpose:
 *      Set port loopback
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      loopback  - (IN)  soc_dnxc_loopback_mode_t
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_loopback_set(int unit, soc_port_t port, soc_dnxc_loopback_mode_t loopback)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 reg_val, field_val[1];
    soc_error_t rc;
    int lane0,lane1,lane2,lane3;
    soc_dnxc_port_pcs_t pcs;
    soc_dnxc_loopback_mode_t prev_loopback;
    soc_dnxc_port_power_t power;
    uint32 rx_serdes_reset;
    int rv;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    /*MAC PCS llopback is valid only for 64/66 FEC encoding*/
    if(soc_dnxc_loopback_mode_mac_pcs == loopback) {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));
        if(soc_dnxc_port_pcs_64_66_fec != pcs) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM,(_BSL_DNXC_MSG("MAC PCS llopback is valid only for 64/66 FEC encoding")));
        }
    }

    /*get previous loopback*/
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_loopback_get(unit, port, &prev_loopback));

    if(prev_loopback != loopback) {

        /* First disable FMAC */
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_RX_RESET, &rx_serdes_reset));
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_get(unit,port,&power));
        if(soc_dnxc_port_power_on == power) {
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_set(unit,port, rx_serdes_reset ? SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE : 0, soc_dnxc_port_power_off));
        }

        /*soc_dnxc_loopback_mode_mac_async_fifo*/
        if(soc_dnxc_loopback_mode_mac_async_fifo == loopback || soc_dnxc_loopback_mode_mac_async_fifo == prev_loopback) {
#ifdef BCM_88754_A0
            if (SOC_IS_BCM88754_A0(unit))
            {
                *field_val = (soc_dnxc_loopback_mode_mac_async_fifo == loopback) ? 1 : 0;
                DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, blk_id, inner_port, FMAL_SIF_LOOPBACK_IN_CORE_CLOCKf, *field_val));
                DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, blk_id, inner_port, FMAL_20B_SRD_20B_LOOPBACKf, *field_val));
            } else
#endif /*BCM_88754_A0*/
            {
                DNXC_IF_ERR_EXIT(READ_FMAC_LOOPBACK_ENABLE_REGISTERr(unit, blk_id, &reg_val));
                *field_val = soc_reg_field_get(unit, FMAC_LOOPBACK_ENABLE_REGISTERr, reg_val, LCL_LPBK_ONf);
                if(soc_dnxc_loopback_mode_mac_async_fifo == loopback) {
                    SHR_BITSET(field_val, inner_port);
                } else {
                    SHR_BITCLR(field_val, inner_port);
                }
                soc_reg_field_set(unit, FMAC_LOOPBACK_ENABLE_REGISTERr, &reg_val, LCL_LPBK_ONf, *field_val);
                DNXC_IF_ERR_EXIT(WRITE_FMAC_LOOPBACK_ENABLE_REGISTERr(unit, blk_id, reg_val));
            }
        }
        
        /*soc_dnxc_loopback_mode_mac_outer*/
        if(soc_dnxc_loopback_mode_mac_outer == loopback || soc_dnxc_loopback_mode_mac_outer == prev_loopback) {
            if(soc_dnxc_loopback_mode_mac_outer == loopback) 
            {
                /*if all the mac ports are not in mac outer loopback save default value first*/
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 0, &reg_val));
                lane0 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 1, &reg_val));
                lane1 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 2, &reg_val));
                lane2 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 3, &reg_val));
                lane3 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                if(lane0 && lane1 && lane2 && lane3) 
                {
                    DNXC_IF_ERR_EXIT(WRITE_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, reg_val));
                    rv = SOC_DNXF_WARM_BOOT_ARR_VAR_SET(unit, PORT_CTRL_BURST_CONF, blk_id, &reg_val);
                    DNXC_IF_ERR_EXIT(rv);
                }

                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
                soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg_val, FMAL_N_CORE_40B_LOOPBACKf, 1);
                DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, reg_val));

                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
                rv = SOC_DNXF_WARM_BOOT_ARR_VAR_SET(unit, PORT_COMMA_BURST_CONF, port, &reg_val);
                DNXC_IF_ERR_EXIT(rv);
                DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, 0x00a0e513));

                DNXC_IF_ERR_EXIT(READ_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, &reg_val));
                rv = SOC_DNXF_WARM_BOOT_ARR_VAR_SET(unit, PORT_CTRL_BURST_CONF, blk_id, &reg_val);
                DNXC_IF_ERR_EXIT(rv);
                DNXC_IF_ERR_EXIT(WRITE_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, 0x1801c00d));
            } else {
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
                soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg_val, FMAL_N_CORE_40B_LOOPBACKf, 0);
                DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, reg_val));

                rv = SOC_DNXF_WARM_BOOT_ARR_VAR_GET(unit, PORT_COMMA_BURST_CONF, port, &reg_val);
                DNXC_IF_ERR_EXIT(rv);
                DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_COMMA_BURST_CONFIGURATIONr(unit, blk_id, inner_port, reg_val));

                /*if all the mac ports not in loopback in to restore default value*/
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 0, &reg_val));
                lane0 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 1, &reg_val));
                lane1 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 2, &reg_val));
                lane2 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, 3, &reg_val));
                lane3 = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
                if(!lane0 && !lane1 && !lane2 && !lane3) 
                {
                    rv = SOC_DNXF_WARM_BOOT_ARR_VAR_GET(unit, PORT_CTRL_BURST_CONF, blk_id, &reg_val);
                    DNXC_IF_ERR_EXIT(rv);
                    DNXC_IF_ERR_EXIT(WRITE_FMAC_CONTROL_CELL_BURST_REGISTERr(unit, blk_id, reg_val));
                }
            }

        }

        if(soc_dnxc_loopback_mode_mac_pcs == loopback || soc_dnxc_loopback_mode_mac_pcs == prev_loopback) {
            /*soc_dnxc_loopback_mode_mac_pcs*/
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
            soc_reg_field_set(unit, FMAC_KPCS_CONFIGURATIONr, &reg_val, KPCS_N_RX_DSC_LOOPBACK_ENf, soc_dnxc_loopback_mode_mac_pcs == loopback ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_KPCS_CONFIGURATIONr(unit, blk_id, inner_port, reg_val));
        }
        
        /*soc_dnxc_loopback_mode_phy_gloop*/
        if(soc_dnxc_loopback_mode_phy_gloop == loopback || soc_dnxc_loopback_mode_phy_gloop == prev_loopback) {
            rc = soc_phyctrl_loopback_set(unit, port, soc_dnxc_loopback_mode_phy_gloop == loopback ? 1 : 0, 1);
            DNXC_IF_ERR_EXIT(rc);
        }

        /*soc_dnxc_loopback_mode_phy_rloop*/
        if(soc_dnxc_loopback_mode_phy_rloop == loopback || soc_dnxc_loopback_mode_phy_rloop == prev_loopback) {
            rc = soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, soc_dnxc_loopback_mode_phy_rloop == loopback ? 1 : 0);
            DNXC_IF_ERR_EXIT(rc);
        }

         DNXC_IF_ERR_EXIT(soc_ramon_fe1600_force_signal_detect_set(unit, blk_id));

        if(soc_dnxc_port_power_on == power) {
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_power_set(unit,port, rx_serdes_reset ? SOC_DNXC_PORT_CONTROL_FLAGS_RX_SERDES_IGNORE : 0, soc_dnxc_port_power_on));
        }
    } 

exit:
    DNXC_FUNC_RETURN;

}

/*
 * Function:
 *      soc_ramon_fe1600_port_loopback_get
 * Purpose:
 *      Get port loopback
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      loopback  - (OUT) soc_dnxf_loopback_mode_t
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_loopback_get(int unit, soc_port_t port, soc_dnxc_loopback_mode_t* loopback)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 serdes_xrloop;
    int serdes_gloop;
    uint32 reg_val, field_val[1];
    soc_error_t rc;
    int async_fifo_loopback;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    /*soc_dnxf_loopback_mode_mac_async_fifo*/

    reg_val = 0;
    *field_val = 0;
    async_fifo_loopback = 0;

#ifdef BCM_88754_A0
    if (SOC_IS_BCM88754_A0(unit))
    {
        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_get(unit, blk_id, inner_port, FMAL_20B_SRD_20B_LOOPBACKf, field_val));
        async_fifo_loopback = (*field_val ? 1 : 0);

    } else
#endif /*BCM_88754_A0*/
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_LOOPBACK_ENABLE_REGISTERr(unit, blk_id, &reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_LOOPBACK_ENABLE_REGISTERr, reg_val, LCL_LPBK_ONf);
        async_fifo_loopback = SHR_BITGET(field_val, inner_port);
    }

    if(async_fifo_loopback) {
        *loopback = soc_dnxc_loopback_mode_mac_async_fifo;
    } else {
        
        /*soc_dnxc_loopback_mode_mac_outer*/
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
        *field_val = soc_reg_field_get(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, reg_val, FMAL_N_CORE_40B_LOOPBACKf);
        if(*field_val) {
            *loopback = soc_dnxc_loopback_mode_mac_outer;
        } else {

            /*soc_dnxc_loopback_mode_mac_pcs*/
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val));
            *field_val = soc_reg_field_get(unit, FMAC_KPCS_CONFIGURATIONr, reg_val, KPCS_N_RX_DSC_LOOPBACK_ENf);
            if(*field_val) {
                *loopback = soc_dnxc_loopback_mode_mac_pcs;
            } else {

                /*soc_dnxc_loopback_mode_phy_gloop*/
                rc = soc_phyctrl_loopback_get(unit, port, &serdes_gloop);
                if(SOC_FAILURE(rc)) {
                    DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed in soc_phyctrl_loopback_get, error %s"),soc_errmsg(rc)));   
                } else if(serdes_gloop) {
                    *loopback = soc_dnxc_loopback_mode_phy_gloop;
                } else {
                
                    /*soc_dnxc_loopback_mode_phy_rloop*/
                    rc = soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS, &serdes_xrloop);
                    if(SOC_FAILURE(rc)) {
                        DNXC_EXIT_WITH_ERR(rc, (_BSL_DNXC_MSG("failed in soc_phyctrl_control_get(SOC_PHY_CONTROL_LOOPBACK_REMOTE_PCS_BYPASS), error %s"),soc_errmsg(rc))); 
                    } else if(serdes_xrloop) {
                        *loopback = soc_dnxc_loopback_mode_phy_rloop;
                    } else {

                        /*soc_dnxc_loopback_mode_none*/
                         *loopback = soc_dnxc_loopback_mode_none;
                    }
                }
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_fe1600_port_fault_get
 * Purpose:
 *      Get port loopback
 * Parameters:
 *      unit -  (IN)  BCM device number 
 *      port -  (IN)  Device or logical port number .
 *      flags - (OUT) Flags to indicate fault type 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_fault_get(int unit, soc_port_t port, uint32* flags)
{
    soc_reg_above_64_val_t reg_val;
    DNXC_INIT_FUNC_DEFS;

    (*flags) = 0;
    DNXC_IF_ERR_EXIT(READ_RTP_LINK_STATE_VECTORr(unit, reg_val));
    if(!SHR_BITGET(reg_val,port)) {
        (*flags) |= BCM_PORT_FAULT_LOCAL;
    }

    DNXC_IF_ERR_EXIT(READ_RTP_ACL_VECTORr(unit, reg_val));
    if(!SHR_BITGET(reg_val,port)) {
        (*flags) |= BCM_PORT_FAULT_REMOTE;
    }  

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_fe1600_port_speed_max
 * Purpose:
 *      Get the current operating speed of a port
 * Parameters:
 *      unit - Unit #.
 *      port - port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t 
soc_ramon_fe1600_port_speed_max(int unit, soc_port_t port, int *speed)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

   if(SOC_DNXF_CONFIG(unit).system_is_fe600_in_system) {
        *speed = 6250;
    } else {
        *speed = 12500;
    }

    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_link_status_get
 * Purpose:
 *      Return link status
 * Parameters:
 *      unit - Unit #.
 *      port - port #.
 *      up - (OUT) Links status (up\down)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t
soc_ramon_fe1600_port_link_status_get(int unit, soc_port_t port, int *up)
{
    int         rc;
    int         enable;
    uint32 
        is_down=0,
        reg_val,
        blk_id,
        inner_link,
        fld_val[1];
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*check if port is enabled*/
    rc = soc_phyctrl_enable_get(unit, port, &enable);
    DNXC_IF_ERR_EXIT(rc);

    if (enable) {
        /*clear rxlos interrupt*/

        blk_id = port/SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;
        inner_link = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;
        reg_val = 0;
        *fld_val = (1 << inner_link);
        soc_reg_field_set(unit, FMAC_INTERRUPT_REGISTER_2r, &reg_val, RX_LOST_OF_SYNCf, *fld_val);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,reg_val));
        
        /*check rxlos interrupt - link is up*/

        DNXC_IF_ERR_EXIT(READ_FMAC_INTERRUPT_REGISTER_2r(unit,blk_id,&reg_val));
        *fld_val = soc_reg_field_get(unit, FMAC_INTERRUPT_REGISTER_2r, reg_val, RX_LOST_OF_SYNCf);
        is_down = SHR_BITGET(fld_val, inner_link);

        *up = (is_down == 0);
    } else {
        *up = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_bucket_fill_rate_validate(int unit, uint32 bucket_fill_rate)
{
    DNXC_INIT_FUNC_DEFS;
    

    if(bucket_fill_rate > SOC_RAMON_FE1600_MAX_BUCKET_FILL_RATE) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("BUCKET_FILL_RATE: %d is out-of-ranget"), bucket_fill_rate));
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_control_low_latency_set
 * Purpose:
 *      Enable/Disable low latency for FEC/BEC
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      value     - (IN)  1 to enable, 0 to disable
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */

soc_error_t
soc_ramon_fe1600_port_control_low_latency_set(int unit,soc_port_t port,int value)
{
    soc_port_t inner_port;
    int blk_id;
    int valid=0;
    soc_dnxc_port_pcs_t pcs;
    uint32 reg_val32;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

    switch (pcs)
    {
        case soc_dnxc_port_pcs_8_9_legacy_fec:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Low Latency is not Supported for non FEC/BEC")));
            break;
        case soc_dnxc_port_pcs_8_10:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Low Latency is not Supported for non FEC/BEC")));
            break;
        case soc_dnxc_port_pcs_64_66_fec:
            valid=1;
            break;
        case soc_dnxc_port_pcs_64_66_bec:
            valid=1;
            break;
        case soc_dnxc_port_pcs_64_66:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Low Latency is not Supported for non FEC/BEC")));
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid PCS type")));
            break;

    }
    if (valid)
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
        soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg_val32, FMAL_N_LOW_LATENCY_LLFCf, value? 1:0);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_fe1600_port_control_low_latency_get
 * Purpose:
 *      Return low latency status (enabled/disabled)
 * Parameters:
 *      unit - Unit #.
 *      port - port #.
 *      value - (OUT) 1 if enabled, 0 if disabled
 * Returns:
 *      SOC_E_XXX
 */



soc_error_t
soc_ramon_fe1600_port_control_low_latency_get(int unit, soc_port_t port, int *value)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 reg_val32;
    soc_dnxc_port_pcs_t pcs;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;
    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

    if (! (pcs == soc_dnxc_port_pcs_64_66_fec || pcs == soc_dnxc_port_pcs_64_66_bec ) )
    {
         DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid PCS type")));

    }

    DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit,blk_id,inner_port,&reg_val32));
    *value=soc_reg_field_get(unit,FMAC_FMAL_GENERAL_CONFIGURATIONr,reg_val32,FMAL_N_LOW_LATENCY_LLFCf);


exit:
    DNXC_FUNC_RETURN;



}

soc_error_t
soc_ramon_fe1600_port_control_fec_error_detect_set(int unit,soc_port_t port,int value)
{
    soc_port_t inner_port;
    int blk_id;
    int valid=0;
    soc_dnxc_port_pcs_t pcs;
    uint32 reg_val32;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

    switch (pcs)
    {
        case soc_dnxc_port_pcs_8_9_legacy_fec:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Error Detecting is not Supported for non FEC")));
            break;
        case soc_dnxc_port_pcs_8_10:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Error Detecting is not Supported for non FEC")));
            break;
        case soc_dnxc_port_pcs_64_66_fec:
            valid=1;
            break;
        case soc_dnxc_port_pcs_64_66_bec:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Error Detecting is not Supported for non FEC")));
            break;
        case soc_dnxc_port_pcs_64_66:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Error Detecting is not Supported for non FEC")));
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid PCS type"))); 
            break;

    }
    if (valid)
    {
        DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
        soc_reg_field_set(unit, FMAC_FPS_RX_FEC_CONFIGURATIONr, &reg_val32, FPS_N_RX_FEC_ERR_MARK_ALLf, value? 1:0);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_port_control_fec_error_detect_get(int unit,soc_port_t port,int* value)
{
    soc_port_t inner_port;
    int blk_id;
    uint32 reg_val32;
    soc_dnxc_port_pcs_t pcs;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
    inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

    DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));


    if (!(pcs == soc_dnxc_port_pcs_64_66_fec ))
    {
         DNXC_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_DNXC_MSG("Invalid PCS type"))); 

    }

    DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_FEC_CONFIGURATIONr(unit,blk_id,inner_port,&reg_val32));
    *value=soc_reg_field_get(unit,FMAC_FPS_RX_FEC_CONFIGURATIONr,reg_val32,FPS_N_RX_FEC_ERR_MARK_ALLf);


exit:
    DNXC_FUNC_RETURN;
}



/*mode=1 MAC, mode=0 PHY*/
soc_error_t 
soc_ramon_fe1600_port_prbs_tx_enable_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    int blk_id, inner_port;
    uint32 reg_val32;
    soc_dnxc_port_pcs_t pcs;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
        inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

        switch(pcs) {
        case soc_dnxc_port_pcs_64_66_bec:
        case soc_dnxc_port_pcs_64_66_fec:
        case soc_dnxc_port_pcs_64_66:
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_TEST_TX_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            soc_reg_field_set(unit, FMAC_KPCS_TEST_TX_CONFIGURATIONr, &reg_val32, KPCS_N_TST_TX_ENf, value ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_KPCS_TEST_TX_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
            break;

        case soc_dnxc_port_pcs_8_10:
            DNXC_IF_ERR_EXIT(READ_FMAC_TEST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            soc_reg_field_set(unit, FMAC_TEST_CONFIGURATIONr, &reg_val32, EDS_N_TX_TST_ENABLEf, value ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_TEST_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
            break;

        case soc_dnxc_port_pcs_8_9_legacy_fec:
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("MAC PRBS isn't supported for PCS %d"), pcs));
            break;
        }
    } else /*PHY*/ {
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRBS_TX_ENABLE, value));
    }
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_prbs_tx_enable_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    int blk_id, inner_port;
    uint32 reg_val32;
    soc_dnxc_port_pcs_t pcs;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
        inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

        switch(pcs) {
        case soc_dnxc_port_pcs_64_66_bec:
        case soc_dnxc_port_pcs_64_66_fec:
        case soc_dnxc_port_pcs_64_66:

            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_TEST_TX_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            *value = soc_reg_field_get(unit, FMAC_KPCS_TEST_TX_CONFIGURATIONr, reg_val32, KPCS_N_TST_TX_ENf);
            break;

        case soc_dnxc_port_pcs_8_10:
            DNXC_IF_ERR_EXIT(READ_FMAC_TEST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            *value = soc_reg_field_get(unit, FMAC_TEST_CONFIGURATIONr, reg_val32, EDS_N_TX_TST_ENABLEf);
            break;

        case soc_dnxc_port_pcs_8_9_legacy_fec:
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("MAC PRBS isn't supported for PCS %d"), pcs));
            break;
        }
    } else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port,SOC_PHY_CONTROL_PRBS_TX_ENABLE, (uint32*)value));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_prbs_rx_enable_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    int blk_id, inner_port;
    uint32 reg_val32;
    soc_dnxc_port_pcs_t pcs;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
        inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

        
        switch(pcs) {
        case soc_dnxc_port_pcs_64_66_bec:
        case soc_dnxc_port_pcs_64_66_fec:
        case soc_dnxc_port_pcs_64_66:
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_TEST_RX_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            soc_reg_field_set(unit, FMAC_KPCS_TEST_RX_CONFIGURATIONr, &reg_val32, KPCS_N_TST_RX_ENf, value ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_KPCS_TEST_RX_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
            break;

        case soc_dnxc_port_pcs_8_10:
            DNXC_IF_ERR_EXIT(READ_FMAC_TEST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            soc_reg_field_set(unit, FMAC_TEST_CONFIGURATIONr, &reg_val32, EDS_N_RX_TST_ENABLEf, value ? 1 : 0);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_TEST_CONFIGURATIONr(unit, blk_id, inner_port, reg_val32));
            break;

        case soc_dnxc_port_pcs_8_9_legacy_fec:
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("MAC PRBS isn't supported for PCS %d"), pcs));
            break;
        }
    }else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRBS_RX_ENABLE, value));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_prbs_rx_enable_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    int blk_id, inner_port;
    uint32 reg_val32;
    soc_dnxc_port_pcs_t pcs;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
        inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

        switch(pcs) {
        case soc_dnxc_port_pcs_64_66_bec:
        case soc_dnxc_port_pcs_64_66_fec:
        case soc_dnxc_port_pcs_64_66:

            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_TEST_RX_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            *value = soc_reg_field_get(unit, FMAC_KPCS_TEST_RX_CONFIGURATIONr, reg_val32, KPCS_N_TST_RX_ENf);
            break;

        case soc_dnxc_port_pcs_8_10:
            DNXC_IF_ERR_EXIT(READ_FMAC_TEST_CONFIGURATIONr(unit, blk_id, inner_port, &reg_val32));
            *value = soc_reg_field_get(unit, FMAC_TEST_CONFIGURATIONr, reg_val32, EDS_N_RX_TST_ENABLEf);
            break;

        case soc_dnxc_port_pcs_8_9_legacy_fec:
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("MAC PRBS isn't supported for PCS %d"), pcs));
            break;
        }
    }else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_PRBS_RX_ENABLE, (uint32*)value));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_prbs_rx_status_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    int blk_id, inner_port;
    uint32 reg_val32, reg_val32_counter, reg_val32_sync;
    soc_dnxc_port_pcs_t pcs;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        blk_id = INT_DEVIDE(port, SOC_RAMON_FE1600_NOF_LINKS_IN_MAC);
        inner_port = port % SOC_RAMON_FE1600_NOF_LINKS_IN_MAC;

        DNXC_IF_ERR_EXIT(soc_ramon_fe1600_port_control_pcs_get(unit, port, &pcs));

        switch(pcs) {
        case soc_dnxc_port_pcs_64_66_bec:
        case soc_dnxc_port_pcs_64_66_fec:
        case soc_dnxc_port_pcs_64_66:

            /*sync status*/
            DNXC_IF_ERR_EXIT(READ_FMAC_FPS_RX_STATUSr(unit, blk_id, inner_port, &reg_val32_sync));
            DNXC_IF_ERR_EXIT(READ_FMAC_KPCS_TEST_RX_STATUSr(unit, blk_id, inner_port, &reg_val32_counter));
            if (!soc_reg_field_get(unit, FMAC_FPS_RX_STATUSr, reg_val32_sync, FPS_N_RX_SYNC_STATUSf)) {
                *value = -1;
            }  else {
                /*error counter*/
                *value = (int)reg_val32_counter;
            }
            break;

        case soc_dnxc_port_pcs_8_10:
            /*sync status*/
            DNXC_IF_ERR_EXIT(READ_FMAC_TEST_STATUSr(unit, blk_id, inner_port, &reg_val32));
             if (!soc_reg_field_get(unit, FMAC_TEST_STATUSr, reg_val32, TST_N_SYNCEDf)) {
                 *value = -1;
             } else {
                /*error counter*/
                *value = (int)soc_reg_field_get(unit, FMAC_TEST_STATUSr, reg_val32, TST_N_ERR_CNTf);
             }
            break;

        case soc_dnxc_port_pcs_8_9_legacy_fec:
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("MAC PRBS isn't supported for PCS %d"), pcs));
            break;
        }
    }else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_PRBS_RX_STATUS, (uint32*)value));
    }

exit:
    DNXC_FUNC_RETURN;

}

soc_error_t 
soc_ramon_fe1600_port_prbs_polynomial_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Polynomial set isn't supported for MAC PRBS\n")));
    }else { /*PHY*/
        switch(value){
        case SOC_PHY_PRBS_POLYNOMIAL_X7_X6_1:
        case SOC_PHY_PRBS_POLYNOMIAL_X15_X14_1:
        case SOC_PHY_PRBS_POLYNOMIAL_X23_X18_1:
        case SOC_PHY_PRBS_POLYNOMIAL_X31_X28_1:
            DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRBS_POLYNOMIAL, value));
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid value %d"), value));
        }
    }
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_prbs_polynomial_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("Polynomial get isn't supported for MAC PRBS\n")));
    }else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_PRBS_POLYNOMIAL, (uint32*)value));
    }
exit:
    DNXC_FUNC_RETURN;
}


soc_error_t 
soc_ramon_fe1600_port_prbs_tx_invert_data_set(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int value)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("PRBS TX invert data set isn't supported for MAC PRBS\n")));
    }else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA, value));
    }
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t 
soc_ramon_fe1600_port_prbs_tx_invert_data_get(int unit, soc_port_t port, soc_dnxc_port_prbs_mode_t mode, int* value)
{
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    if(mode){ /*MAC*/
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("PRBS TX invert data get isn't supported for MAC PRBS\n")));
    }else { /*PHY*/
        DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_PRBS_TX_INVERT_DATA, (uint32*)value));
    }
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_speed_set
 * Purpose:
 *      Setting the speed for a given port
 * Parameters:
 *      unit - Unit #.
 *      port - port #.
 *      speed - Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t
soc_ramon_fe1600_port_speed_set(int unit, soc_port_t port, int speed)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

#ifdef BCM_88790_A0
    if (SOC_IS_FE1600_B0_AND_ABOVE(unit)) {
        /*change values to default before phyctrl speed set*/
        uint64 reg64_val;
        int quad, lane;

        rc = soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_OVERSAMPLE_MODE, 0);
        DNXC_IF_ERR_EXIT(rc);
        rc = soc_phyctrl_speed_set(unit, port, SOC_PHY_NON_CANNED_SPEED);
        DNXC_IF_ERR_EXIT(rc);
        
        quad = port / SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;
        lane = port % SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;

#ifdef BCM_88754_A0
        if (SOC_IS_BCM88754_A0(unit))
        {               
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, quad, lane, ITEM_6f /*RX full rate enable*/, 0x1));
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, quad, lane, ITEM_13f /*TX full rate enable*/, 0x1));
        } else
#endif /*BCM_88754_A0*/
        {
            DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, quad, lane, &reg64_val));
            soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_N_RX_FULL_RATE_ENf, 0x1);
            soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_N_TX_FULL_RATE_ENf, 0x1);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, quad, lane, reg64_val));
        }
    }
#endif

    rc = soc_phyctrl_speed_set(unit, port, speed);
    DNXC_IF_ERR_EXIT(rc);

#ifdef BCM_88790_A0
    if (SOC_IS_FE1600_B0_AND_ABOVE(unit)) {
        uint32 os;
        uint64 reg64_val;
        uint32 val;
        int quad, lane;

        rc = soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_OVERSAMPLE_MODE, &os);
        DNXC_IF_ERR_EXIT(rc);
        
        quad = port / SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;
        lane = port % SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;

        val =  (os == 0 ? 1 : 0);
#ifdef BCM_88754_A0
        if (SOC_IS_BCM88754_A0(unit))
        {               
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, quad, lane, ITEM_6f /*RX full rate enable*/, val));
            DNXC_IF_ERR_EXIT(soc_ramon_fe1600_drv_bcm88754_async_fifo_set(unit, quad, lane, ITEM_13f /*TX full rate enable*/, val));
        } else
#endif /*BCM_88754_A0*/
        {
            DNXC_IF_ERR_EXIT(READ_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, quad, lane, &reg64_val));
            soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_N_RX_FULL_RATE_ENf, val);
            soc_reg64_field32_set(unit, FMAC_ASYNC_FIFO_CONFIGURATIONr, &reg64_val, FMAL_N_TX_FULL_RATE_ENf, val);
            DNXC_IF_ERR_EXIT(WRITE_FMAC_ASYNC_FIFO_CONFIGURATIONr(unit, quad, lane, reg64_val));
        }
    }
#endif

    /*Configure ALDWP*/
    rc = soc_ramon_fe1600_fabric_links_aldwp_config(unit, port);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

/*
 * Function:
 *      soc_ramon_fe1600_port_speed_get
 * Purpose:
 *      Getting the speed of the port
 * Parameters:
 *      unit - Unit #.
 *      port - port #.
 *      speed - (OUT) Value in megabits/sec (10, 100, etc)
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t
soc_ramon_fe1600_port_speed_get(int unit, soc_port_t port, int *speed)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    rc = soc_phyctrl_speed_get(unit, port, speed);
    DNXC_IF_ERR_EXIT(rc);

exit:
    DNXC_FUNC_RETURN; 
}

soc_error_t
soc_ramon_fe1600_port_interface_get(int unit, soc_port_t port, soc_port_if_t* intf)
{
    *intf = _SHR_PORT_IF_NOCXN;
    return SOC_E_NONE;
}
/*
 * Function:
 *      soc_ramon_fe1600_port_rate_egress_ppt_set
 * Description:
 *      packets per second shaper
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      nof_packets , nof_tiks (out)- Rate in packet per tiks. A value of zero disables rate limiting (full speed).
 * Returns:
 *      BCM_E_XXX
 */

soc_error_t
soc_ramon_fe1600_port_rate_egress_ppt_set(int unit, soc_port_t port, uint32 burst, uint32 nof_tiks)
{
    uint32 limit_max;
    int max_length;
    uint64 reg64_val;
    int fmac_index; 
    DNXC_INIT_FUNC_DEFS;

    if (SOC_IS_FE1600_A0(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("soc_ramon_fe1600_port_rate_egress_ppt_set is unsupported for ramon_fe1600_a0"))); 
    }
    
    /* 
     *Valdiate input
     */
    max_length = soc_reg_field_length(unit, FMAC_TX_CELL_LIMITr,  CELL_LIMIT_COUNTf);
    limit_max = 0;
    SHR_BITSET_RANGE(&limit_max, 0, max_length);
    if (limit_max <  burst) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Burst %u is too big - should be < %u"), burst, limit_max)); 
    }

    max_length = soc_reg_field_length(unit, FMAC_TX_CELL_LIMITr,  CELL_LIMIT_PERIODf);
    limit_max = 0;
    SHR_BITSET_RANGE(&limit_max, 0, max_length);
    if (limit_max <  nof_tiks) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Burst %u is too big relative to pps %u"), burst, nof_tiks)); 
    }

    if (nof_tiks == 0 && burst != 0) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("num of ticks %u is too big relative to burst %u"), nof_tiks, burst)); 
    }

    /*disable shaper handle*/
    if (burst == 0) {
        fmac_index = port / SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);
        COMPILER_64_SET(reg64_val, 0, 0);
        DNXC_IF_ERR_EXIT(WRITE_FMAC_TX_CELL_LIMITr(unit, fmac_index, reg64_val));
        SOC_EXIT;
    } else if (burst < 3){
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Burst %u is too small - should be > 3"), burst));  
    }
    
    fmac_index = port / SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);
    COMPILER_64_SET(reg64_val, 0, 0);
    soc_reg64_field32_set(unit, FMAC_TX_CELL_LIMITr, &reg64_val, CELL_LIMIT_COUNTf, burst - 2 /*-2 for shaper accuracy*/);
    soc_reg64_field32_set(unit, FMAC_TX_CELL_LIMITr, &reg64_val, CELL_LIMIT_PERIODf, nof_tiks);
    DNXC_IF_ERR_EXIT(WRITE_FMAC_TX_CELL_LIMITr(unit, fmac_index, reg64_val));

exit:
    DNXC_FUNC_RETURN; 
}
soc_error_t

/*
 * Function:
 *      soc_ramon_fe1600_port_rate_egress_ppt_get
 * Description:
 *      packets per second shaper
 * Parameters:
 *      unit - Device number
 *      port - Port number
 *      nof_packets , nof_tiks (out)- Rate in packet per tiks. A value of zero disables rate limiting (full speed).
 * Returns:
 *      BCM_E_XXX
 */

soc_ramon_fe1600_port_rate_egress_ppt_get(int unit, soc_port_t port, uint32 *burst, uint32 *nof_tiks)
{
    uint64 reg64_val;
    int fmac_index; 
    DNXC_INIT_FUNC_DEFS;

    if (SOC_IS_FE1600_A0(unit)) {
        DNXC_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_DNXC_MSG("soc_ramon_fe1600_port_rate_egress_ppt_set is unsupported for ramon_fe1600_a0"))); 
    }
    
    fmac_index = port / SOC_DNXF_DEFS_GET(unit, nof_links_in_mac);

    DNXC_IF_ERR_EXIT(READ_FMAC_TX_CELL_LIMITr(unit, fmac_index, &reg64_val));
    *burst =  soc_reg64_field32_get(unit, FMAC_TX_CELL_LIMITr, reg64_val, CELL_LIMIT_COUNTf) + 2/*+2 for shaper accuracy*/;
    *nof_tiks = soc_reg64_field32_get(unit, FMAC_TX_CELL_LIMITr, reg64_val, CELL_LIMIT_PERIODf);

exit:
    DNXC_FUNC_RETURN; 
}


/*
 * Function:
 *      soc_ramon_fe1600_port_wcmod_ucode_load
 * Purpose:
 *      optimizing loading firmware time
 * Parameters:
 *      unit - Unit #.
 *      port - port #. (check if the firmware is already loaded for the specific port, but load firmware fo all ports if necessary
 *      arr - wcmod table
 *      arr_len - wcmod table length
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t
soc_ramon_fe1600_port_wcmod_ucode_load(int unit , int port, uint8 *arr,int arr_len) {

    int j, n;
    int max_ndx;
    int rv;
    uint32 reg_val32;
    int port_local, blk;
    uint8  phy_id;
    uint16 phy_wr_data, phy_rd_data;
    uint32 phy_data;
    soc_reg_above_64_val_t wr_data;
    soc_timeout_t to;
    int clause = 22; /*RAMON_FE1600 access intenal phy with CL22*/

    /*const values*/
    uint32 xgswl_microcommand_a = 0x12;
    uint32 xgswl_microcommand3_a = 0x1c;
    uint32 xgswl_microblk_download_status = 0x15;
    uint32 xgswl_microblk_a = 0xffc;
    uint32 xgswl_blockaddress_a  = 0x1f;  
    uint32 xgswl_xgxsblk0_a = 0x800;
    uint32 xgswl_xgxscontrol_a = 0x10; 

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

#ifdef BCM_88754_A0
    /*Not required for BCM88754_A0*/
    if (SOC_IS_BCM88754_A0(unit))
    {
        SOC_EXIT;
    }
#endif /*BCM_88754_A0*/

    /*check if should run - wcmod version*/
    phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
    phy_data = 0x81f0;
    phy_wr_data = (uint16) (phy_data & 0xffff);
    rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_blockaddress_a, phy_wr_data);
    DNXC_IF_ERR_EXIT(rv);

    rv = soc_dcmn_miim_read(unit, clause, phy_id, 0x10, &phy_rd_data);
    DNXC_IF_ERR_EXIT(rv);

    if(phy_rd_data != 0) {
        SOC_EXIT;
    }

    max_ndx = 32*1024;
    /* set div core clock  8*/
    for (port_local = 0; port_local < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); port_local++) {
        DNXC_IF_ERR_EXIT(READ_FSRD_WC_UC_MEM_ACCESSr(unit, port_local, &reg_val32));
        soc_reg_field_set(unit, FSRD_WC_UC_MEM_ACCESSr, &reg_val32, CLK_DIVf, 1);
        DNXC_IF_ERR_EXIT(WRITE_FSRD_WC_UC_MEM_ACCESSr(unit, port_local, reg_val32));

        DNXC_IF_ERR_EXIT(WRITE_FSRD_WC_UC_MEM_MASK_BITMAPr(unit, port_local, 0xf));
    }

    /* init sequence*/
    /* open write access - enable_rom_mem_access*/
    for (port_local=0; port_local<SOC_DNXF_DEFS_GET(unit, nof_links); port_local+=SOC_RAMON_FE1600_NOF_LINKS_IN_MAC) {
        if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_local)) {
            continue;
        }
        phy_id = PORT_TO_PHY_ADDR_INT(unit, port_local);
        
        /*Disable pll start sequencer*/
        phy_data = xgswl_xgxsblk0_a << 4;
        phy_wr_data = (uint16) (phy_data & 0xffff);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_blockaddress_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        rv = soc_dcmn_miim_read(unit, clause, phy_id, xgswl_xgxscontrol_a, &phy_rd_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_rd_data &= 0xdfff;
        phy_wr_data = (uint16) (phy_rd_data & 0xffff);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_xgxscontrol_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        /*initialize command*/
        phy_data = xgswl_microblk_a << 4;
        phy_wr_data = (uint16) (phy_data & 0xffff);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_blockaddress_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_data = 0x8000;
        phy_wr_data = (uint16) (phy_data & 0xffff);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_microcommand_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

    }

    /*Polling for init done*/
    for (port_local=0; port_local<SOC_DNXF_DEFS_GET(unit, nof_links); port_local+=SOC_RAMON_FE1600_NOF_LINKS_IN_MAC) {
        if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_local)) {
            continue;
        }
        phy_id = PORT_TO_PHY_ADDR_INT(unit, port_local);
        /*cfg timeout*/
        soc_timeout_init(&to, 100000 /*100 milisec*/, 100);   
        
        while(1)
        {
            rv = soc_dcmn_miim_read(unit, clause, phy_id, xgswl_microblk_download_status, &phy_rd_data);
            DNXC_IF_ERR_EXIT(rv);

            if(phy_rd_data & 0x8000 /*bit 15 is set*/) {            
                break;
            }
            
            if (soc_timeout_check(&to)) {
                DNXC_EXIT_WITH_ERR(SOC_E_TIMEOUT, (_BSL_DNXC_MSG("WL Initialize program memory - timeout"))); 
            }
        }
    }
    for (port_local=0; port_local<SOC_DNXF_DEFS_GET(unit, nof_links); port_local+=SOC_RAMON_FE1600_NOF_LINKS_IN_MAC) {
        if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_local)) {
            continue;
        }
        phy_id = PORT_TO_PHY_ADDR_INT(unit, port_local);

        /*ext mem command*/
        rv = soc_dcmn_miim_read(unit, clause, phy_id, xgswl_microcommand3_a, &phy_rd_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_wr_data = (uint16) ((phy_rd_data & 0xfffc) | 0x0003);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_microcommand3_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_data = arr_len;
        phy_wr_data = (uint16) (phy_data & 0xffff);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, 0x10, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

    }

    for (port_local = 0; port_local < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); port_local++) {
        DNXC_IF_ERR_EXIT(WRITE_FSRD_WC_UC_MEM_MASK_BITMAPr(unit, port_local, 0xf));
    }
    
    /*direct write to fsrd*/
    for (j=0; j<max_ndx; j+=16) {
         
        SOC_REG_ABOVE_64_CLEAR(wr_data);
         for (n=0; n<16; n++) {
             if (j+n<arr_len) {
                 wr_data[(15- n)/4] |= (arr[j+n] & 0xff) <<  ((15-n) % 4)*8;
             }
         }

        SOC_BLOCK_ITER(unit, blk, SOC_BLK_FSRD) {
            DNXC_IF_ERR_EXIT(soc_mem_write(unit, FSRD_FSRD_WL_EXT_MEMm, blk, j/16, wr_data));
        }
        

    }

    for (port_local = 0; port_local < SOC_DNXF_DEFS_GET(unit, nof_instances_mac_fsrd); port_local++) {
        DNXC_IF_ERR_EXIT(WRITE_FSRD_WC_UC_MEM_MASK_BITMAPr(unit, port_local, 0x0));
    }

    for (port_local=0; port_local<SOC_DNXF_DEFS_GET(unit, nof_links); port_local+=SOC_RAMON_FE1600_NOF_LINKS_IN_MAC) {
        if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port_local)) {
            continue;
        }
        phy_id = PORT_TO_PHY_ADDR_INT(unit, port_local);
        
        /*Set Block address to XGXS Digital6 Block*/
        phy_data = xgswl_microblk_a << 4;
        phy_wr_data = (uint16) (phy_data & 0xffff);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_blockaddress_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        /*ext mem enable*/
        rv = soc_dcmn_miim_read(unit, clause, phy_id, xgswl_microcommand3_a, &phy_rd_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_wr_data = (uint16) (phy_rd_data & 0xfffc);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_microcommand3_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        /*To prevent corruption of the program memory*/
        rv = soc_dcmn_miim_read(unit, clause, phy_id, xgswl_microcommand_a, &phy_rd_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_wr_data = (uint16) ((phy_rd_data & 0xE7FF) | 0x0800);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_microcommand_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

        /*To start the firmware, the microcontroller reset needs to be released*/
        rv = soc_dcmn_miim_read(unit, clause, phy_id, xgswl_microcommand_a, &phy_rd_data);
        DNXC_IF_ERR_EXIT(rv);

        phy_wr_data = (uint16) ((phy_rd_data & 0xFFEF) | 0x0010);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, xgswl_microcommand_a, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);

    }
 
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_serdes_power_disable
 * Purpose:
 *      disable serdes
 * Parameters:
 *      unit - Unit #.
 *      quad - quad #. 
 *      is_enable.
 * Returns:
 *      SOC_E_NONE
 *      SOC_E_XXX
 */
soc_error_t
soc_ramon_fe1600_port_serdes_power_disable(int unit , soc_pbmp_t disable_pbmp) {

    int blk_ins;
    int quad, quad_index;
    uint32 reg_val;
    soc_info_t          *si;
    soc_pbmp_t disabled_bitmap;
    int is_quad_off, lane, is_disable;

    DNXC_INIT_FUNC_DEFS;
    SOC_RAMON_FE1600_ONLY(unit);

    /*in case of ramon_fe1600_b0 there is no need to power on and power off SerDers in order to save power ==> do nothing*/
#ifdef BCM_88790_A0
    if (SOC_IS_FE1600_B0_AND_ABOVE(unit)) {
        SOC_EXIT;
    }
#endif

    /*checks if all lanes in quad are off*/
    si  = &SOC_INFO(unit);
    SOC_PBMP_ASSIGN(disabled_bitmap, si->sfi.disabled_bitmap);

    for (quad =0 ; quad < SOC_DNXF_DEFS_GET(unit, nof_instances_mac); quad++) {
        is_quad_off = 1;

        for (lane =0; lane < SOC_RAMON_FE1600_NOF_LINKS_IN_MAC; lane++) {
            is_disable = SOC_PBMP_MEMBER(disabled_bitmap, quad*SOC_RAMON_FE1600_NOF_LINKS_IN_MAC + lane);
            /*checks if at least on of the lanes were on - if yes, the serdes is on*/
            if (!is_disable) {
                is_quad_off = 0;
                break;
            }
        }
        
        if (!is_quad_off) {
            continue;
        }
    

        blk_ins = quad/SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;
        quad_index = quad%SOC_RAMON_FE1600_NOF_LINKS_IN_QUAD;

        DNXC_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad_index, &reg_val));
        soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_POWER_DOWNf,  1);
        soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_IDDQf,  1);
        DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad_index, reg_val));
        sal_usleep(20);

        soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_HWf, 0);
        DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad_index, reg_val));
        sal_usleep(20);

        soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_MDIO_REGSf, 0);
        DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad_index, reg_val));
        sal_usleep(20);

        soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_PLLf, 0);
        DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad_index, reg_val));
        sal_usleep(20);

        soc_reg_field_set(unit, FSRD_SRD_QUAD_CTRLr, &reg_val, SRD_QUAD_N_RSTB_FIFOf, 0);
        DNXC_IF_ERR_EXIT(WRITE_FSRD_SRD_QUAD_CTRLr(unit, blk_ins, quad_index, reg_val));
        sal_usleep(20);
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_fe1600_port_phy_control_set
 * Purpose:
 *      set configuration on the SerDes
 * Parameters:
 *      unit    - (IN)  Unit number.
 *      port    - (IN)  port number
 *      type    - (IN)  configuration type
 *      value   - (IN)  
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_phy_control_set(int unit, soc_port_t port, int phyn, int lane, int is_sys_side, soc_phy_control_t type, uint32 value)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, type, value));

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Function:
 *      soc_ramon_fe1600_port_phy_control_get
 * Purpose:
 *      get configuration of the SerDes
 * Parameters:
 *      unit    - (IN)  Unit number.
 *      port    - (IN)  port number
 *      type    - (IN)  configuration type
 *      value   - (IN)  
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_phy_control_get(int unit, soc_port_t port, int phyn, int lane, int is_sys_side, soc_phy_control_t type, uint32 *value)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, type, value));

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_phy_enable_set
 * Purpose:
 *      Enable\Disable SerDes
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable  - (IN)  
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_phy_enable_set(int unit, soc_port_t port, int enable)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_phyctrl_enable_set(unit, port, enable));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_phy_enable_get
 * Purpose:
 *      Get SerDes enable
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable  - (OUT) 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_phy_enable_get(int unit, soc_port_t port, int *enable)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_phyctrl_enable_get(unit, port, enable));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_phy_cl72_set
 * Purpose:
 *      Enable\Disable CL72
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable  - (IN)  
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_phy_cl72_set(int unit, soc_port_t port, int enable)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_phyctrl_control_set(unit, port, SOC_PHY_CONTROL_CL72, enable));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Function:
 *      soc_ramon_fe1600_port_phy_cl72_get
 * Purpose:
 *      Get SerDes CL72
 * Parameters:
 *      unit      - (IN)  Unit number.
 *      port      - (IN)  port number 
 *      enable  - (OUT) 
 * Returns:
 *      SOC_E_xxx
 * Notes:
 */
soc_error_t 
soc_ramon_fe1600_port_phy_cl72_get(int unit, soc_port_t port, int *enable)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_phyctrl_control_get(unit, port, SOC_PHY_CONTROL_CL72,(uint32 *) enable));
exit:
    DNXC_FUNC_RETURN;
}


int
soc_ramon_fe1600_port_deinit(int unit)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    /*unregister wcmod_load func*/
    rc = soc_ramon_fe1600_wcmod_firmware_register(unit, NULL);
    DNXC_IF_ERR_CONT(rc);

    if((rc = soc_phyctrl_software_deinit(unit)) != SOC_E_NONE) {
        LOG_WARN(BSL_LS_SOC_PORT,
                 (BSL_META_U(unit,
                             "Failed soc_phyctrl_software_deinit %s\n"),
                  soc_errmsg(rc)));
        DNXC_IF_ERR_CONT(rc);
    }

    if((rc = soc_phy_common_detach(unit)) != SOC_E_NONE) {
        LOG_WARN(BSL_LS_SOC_PORT,
                 (BSL_META_U(unit,
                             "Failed soc_phy_common_detach %s\n"),
                  soc_errmsg(rc)));
        DNXC_IF_ERR_CONT(rc);
    }

    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_port_phy_get(int unit, soc_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 *phy_data)
{
    uint32  phy_id;
    uint32 phy_reg;
    uint16 phy_rd_data;
    uint32 reg_flag;
    int    rv;
    int clause;
    DNXC_INIT_FUNC_DEFS;

    clause = (flags & SOC_PHY_CLAUSE45 ? 45 : 22);

    if (flags & (SOC_PHY_I2C_DATA8 | SOC_PHY_I2C_DATA16)) {
        rv = soc_phyctrl_reg_read(unit, port, flags, phy_reg_addr, phy_data);
        DNXC_IF_ERR_EXIT(rv);
    } else {
        reg_flag = SOC_PHY_REG_FLAGS(phy_reg_addr);
        if (reg_flag & SOC_PHY_REG_INDIRECT) {
            if (flags & SOC_PHY_NOMAP) {
                /* Indirect register access is performed through PHY driver.
                 * Therefore, indirect register access is not supported if
                 * SOC_PHY_NOMAP flag is set.
                 */
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("indirect register access is not supported if SOC_PHY_NOMAP flag is set")));
            }
            phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
            rv = soc_phyctrl_reg_read(unit, port, flags, phy_reg_addr, phy_data);
            DNXC_IF_ERR_EXIT(rv);
        } else {
            if (flags & SOC_PHY_NOMAP) {
                phy_id = port;
            } else if (flags & SOC_PHY_INTERNAL) {
                phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
            } else {
                phy_id = PORT_TO_PHY_ADDR(unit, port);
            }


            phy_reg = phy_reg_addr;
            rv = soc_dcmn_miim_read(unit, clause, phy_id, phy_reg, &phy_rd_data);
            DNXC_IF_ERR_EXIT(rv);

            *phy_data = phy_rd_data;

        }
    }
exit:
    DNXC_FUNC_RETURN; 
}



soc_error_t
soc_ramon_fe1600_port_phy_set(int unit, soc_port_t port, uint32 flags,
                 uint32 phy_reg_addr, uint32 phy_data)
{
    uint32  phy_id;
    uint32 phy_reg;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;
    int clause;
    DNXC_INIT_FUNC_DEFS;


    clause = (flags & SOC_PHY_CLAUSE45 ? 45 : 22);

    if (flags & (SOC_PHY_I2C_DATA8 | SOC_PHY_I2C_DATA16)) {
        rv = soc_phyctrl_reg_write(unit, port, flags, phy_reg_addr, phy_data);
        DNXC_IF_ERR_EXIT(rv);
    } else {
        reg_flag = SOC_PHY_REG_FLAGS(phy_reg_addr);
        if (reg_flag & SOC_PHY_REG_INDIRECT) {
            if (flags & SOC_PHY_NOMAP) {
                /* Indirect register access is performed through PHY driver.
                 * Therefore, indirect register access is not supported if
                 * SOC_PHY_NOMAP flag is set.
                 */
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("indirect register access is not supported if SOC_PHY_NOMAP flag is set")));
            }
            phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
            rv = soc_phyctrl_reg_write(unit, port, flags, phy_reg_addr, phy_data);
            DNXC_IF_ERR_EXIT(rv);
        } else {
            if (flags & SOC_PHY_NOMAP) {
                phy_id = port;
            } else if (flags & SOC_PHY_INTERNAL) {
                phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
            } else {
                phy_id = PORT_TO_PHY_ADDR(unit, port);
            }

            phy_wr_data = (uint16) (phy_data & 0xffff);
            phy_reg = phy_reg_addr;
            rv = soc_dcmn_miim_write(unit, clause, phy_id, phy_reg, phy_wr_data);
            DNXC_IF_ERR_EXIT(rv);
        }
    }

exit:
    DNXC_FUNC_RETURN; 
}


soc_error_t
soc_ramon_fe1600_port_phy_modify(int unit, soc_port_t port, uint32 flags,
                        uint32 phy_reg_addr, uint32 phy_data, uint32 phy_mask)
{
    uint32  phy_id;
    uint32 phy_reg;
    uint16 phy_rd_data;
    uint16 phy_wr_data;
    uint32 reg_flag;
    int    rv;
    int clause;
    DNXC_INIT_FUNC_DEFS;

    clause = (flags & SOC_PHY_CLAUSE45 ? 45 : 22);

    reg_flag = SOC_PHY_REG_FLAGS(phy_reg_addr);
    if (reg_flag & SOC_PHY_REG_INDIRECT) {
        if (flags & SOC_PHY_NOMAP) {
            /* Indirect register access is performed through PHY driver.
             * Therefore, indirect register access is not supported if
             * SOC_PHY_NOMAP flag is set.
             */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("indirect register access is not supported if SOC_PHY_NOMAP flag is set")));
        }
        phy_reg_addr &= ~SOC_PHY_REG_INDIRECT;
        rv = soc_phyctrl_reg_modify(unit, port, flags, phy_reg_addr, phy_data, phy_mask);
        DNXC_IF_ERR_EXIT(rv);
    } else {
        if (flags & SOC_PHY_NOMAP) {
            phy_id = port;
        } else if (flags & SOC_PHY_INTERNAL) {
            phy_id = PORT_TO_PHY_ADDR_INT(unit, port);
        } else {
            phy_id = PORT_TO_PHY_ADDR(unit, port);
        }

        phy_wr_data = (uint16) (phy_data & phy_mask & 0xffff);
        phy_reg = phy_reg_addr;
        rv = soc_dcmn_miim_read(unit, clause, phy_id, phy_reg, &phy_rd_data);
        DNXC_IF_ERR_EXIT(rv);
        phy_wr_data |= (phy_rd_data & ~phy_mask);
        rv = soc_dcmn_miim_write(unit, clause, phy_id, phy_reg, phy_wr_data);
        DNXC_IF_ERR_EXIT(rv);
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_port_dynamic_port_update(int unit, int port, int enable, soc_dnxf_port_update_type_t type)
{
    soc_info_t          *si;
    int                 quad, dis;

    DNXC_INIT_FUNC_DEFS;

    if ((!(enable) && (type==soc_dnxf_port_update_type_sfi_disabled)) || (enable && (type==soc_dnxf_port_update_type_sfi))) {
        quad=port/SOC_DNXF_DEFS_GET(unit, nof_links_in_quad);
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_quad_disabled, (unit, quad, &dis)));
        if (dis) {
            DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("Fabric Quad %d is disabled for device %s"), quad, soc_dev_name(unit)));
        }
    }

    si  = &SOC_INFO(unit);

    if (enable)
    {
        switch (type)
        {
            case soc_dnxf_port_update_type_sfi:
                DNXF_ADD_DYNAMIC_PORT(sfi, port);
                break;
            case soc_dnxf_port_update_type_port:
                DNXF_ADD_DYNAMIC_PORT(port, port);
                break;
            case soc_dnxf_port_update_type_all:
                DNXF_ADD_DYNAMIC_PORT(all, port);
                break;
            case soc_dnxf_port_update_type_sfi_disabled:
                DNXF_ADD_DYNAMIC_DISABLED_PORT(sfi, port);
                break;
            case soc_dnxf_port_update_type_port_disabled:
                DNXF_ADD_DYNAMIC_DISABLED_PORT(port, port);
                break;
            case soc_dnxf_port_update_type_all_disabled:
                DNXF_ADD_DYNAMIC_DISABLED_PORT(all, port);
                break;                      
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Unknown port update type")));
                break;
        }
    }
    else
    {
        switch (type)
        {
            case soc_dnxf_port_update_type_sfi:
                DNXF_REMOVE_DYNAMIC_PORT(sfi, port);
                break;
            case soc_dnxf_port_update_type_port:
                DNXF_REMOVE_DYNAMIC_PORT(port, port);
                break;
            case soc_dnxf_port_update_type_all:
                DNXF_REMOVE_DYNAMIC_PORT(all, port);
                break;
            case soc_dnxf_port_update_type_sfi_disabled:
                DNXF_REMOVE_DYNAMIC_DISABLED_PORT(sfi, port);
                break;
            case soc_dnxf_port_update_type_port_disabled:
                DNXF_REMOVE_DYNAMIC_DISABLED_PORT(port, port);
                break;
            case soc_dnxf_port_update_type_all_disabled:
                DNXF_REMOVE_DYNAMIC_DISABLED_PORT(all, port);
                break;
            default:
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Unknown port update type")));
                break;
        }
    }
exit:
    DNXC_FUNC_RETURN;

}

/*Turn off link pump when leaky bucket is down for better link utilization*/
soc_error_t
soc_ramon_fe1600_port_pump_enable_set(int unit, soc_port_t port, int enable)
{
    int blk, inner_link;
    uint32 reg32_val;
    DNXC_INIT_FUNC_DEFS;

    if (!SOC_IS_FE1600_A0(unit))
    {
        DNXC_IF_ERR_EXIT(MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_drv_link_to_block_mapping, (unit, port, &blk, &inner_link, SOC_BLK_FMAC)));

        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk, inner_link, &reg32_val));
        soc_reg_field_set(unit, FMAC_FMAL_GENERAL_CONFIGURATIONr, &reg32_val ,FMAL_N_TX_PUMP_WHEN_LB_DNf, enable ? 1 : 0);
        DNXC_IF_ERR_EXIT(READ_FMAC_FMAL_GENERAL_CONFIGURATIONr(unit, blk, inner_link, &reg32_val));
    }

exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_port_enable_set(int unit, soc_port_t port, int enable)
{
    int rc;
    DNXC_INIT_FUNC_DEFS;

    if (enable) {
        if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit,all), port)) {
            SOC_EXIT; 
        }

        rc =  MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_control_power_set, (unit,port, 0, soc_dnxc_port_power_on));
        DNXC_IF_ERR_EXIT(rc);
        
        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_phy_enable_set, (unit, port, TRUE));
        DNXC_IF_ERR_EXIT(rc);
        

    } else {

        rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_phy_enable_set, (unit, port, FALSE));
        DNXC_IF_ERR_EXIT(rc);

        rc =  MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_control_power_set, (unit, port, 0, soc_dnxc_port_power_off));
        DNXC_IF_ERR_EXIT(rc);
    }
exit:
    DNXC_FUNC_RETURN;
}

soc_error_t
soc_ramon_fe1600_port_enable_get(int unit, soc_port_t port, int *enable)
{
    int rc;

    DNXC_INIT_FUNC_DEFS;


    rc = MBCM_DNXF_DRIVER_CALL(unit, mbcm_dnxf_port_phy_enable_get, (unit, port, enable));
    DNXC_IF_ERR_EXIT(rc);


exit:
    DNXC_FUNC_RETURN;
}


soc_error_t
soc_ramon_fe1600_port_quad_disabled(int unit, int quad, int *disabled)
{
    soc_error_t rv = SOC_E_NONE;

/*
    if (SOC_IS_BCM88753(unit)) {
        switch (quad) {
        case 4:
        case 5:
        case 6:
        case 7:
        case 12:
        case 13:
        case 14:
        case 15:
        case 22:
        case 23:
        case 28:
        case 29:
            *disabled = 0;
            break;
        default:
            *disabled = 1;
            break;
        }
    } else {
        *disabled = 0;
    }
*/

    *disabled = 0;

    return rv; 
}



#endif /*defined(BCM_88790_A0)*/

#undef _ERR_MSG_MODULE_NAME


