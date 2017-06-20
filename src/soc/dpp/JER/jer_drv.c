/*
 * $Id: jer_drv.c Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME
#error "_ERR_MSG_MODULE_NAME redefined"
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*
 * Includes
 */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
/* SAL includes */
#include <sal/appl/sal.h>

/* SOC includes */
#include <soc/uc.h>
#include <soc/error.h>
#include <soc/iproc.h>
#include <soc/ipoll.h>
#include <soc/linkctrl.h>

/*SOC DCMN includes*/
#include <soc/dcmn/dcmn_cmic.h>
#include <soc/dcmn/dcmn_iproc.h>
#include <soc/dcmn/dcmn_intr.h>
#include <soc/dcmn/dcmn_mem.h>
#include <soc/dcmn/dcmn_dev_feature_manager.h>

/* SOC DPP includes */
#include <soc/dpp/drv.h>
#include <soc/dpp/dpp_defs.h>

/* SOC DPP JER includes */
#include <soc/dpp/JER/jer_drv.h>
#include <soc/dpp/JER/jer_init.h>
#include <soc/dpp/JER/jer_nif.h>
#include <soc/dpp/JER/jer_regs.h>
#include <soc/dpp/JER/jer_egr_queuing.h>
#include <soc/dpp/JER/jer_intr.h>
#include <soc/dpp/JER/jer_reg_access.h>
#include <soc/dpp/JER/jer_link.h>
#include <soc/dpp/JER/jer_mgmt.h>
#include <soc/dpp/JER/jer_sbusdma_desc.h>
/* SOC DPP Arad includes */
#include <soc/dpp/ARAD/arad_chip_regs.h>
#include <soc/dpp/ARAD/arad_drv.h>
#include <soc/dpp/ARAD/arad_init.h>
#include <soc/dpp/ARAD/arad_ports.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/ARAD/NIF/ports_manager.h>
#ifdef CMODEL_SERVER_MODE
#include <soc/dnx/cmodel/cmodel_reg_access.h>
#endif


/******************************************/
/******       port information       ******/
/******************************************/

typedef struct soc_jer_ucode_port_config_s {
    soc_port_if_t   interface;
    soc_pbmp_t      phy_pbmp;
    uint32          channel;
    int             core;
    uint32          tm_port;
    uint32          is_hg;
    uint32          is_nif;
    uint32          is_sif;
    uint32          is_kbp;
    uint32          is_reserved;
    uint32          is_lb_modem;
    uint32          protocol_offset;
} soc_jer_ucode_port_config_t;

STATIC int 
soc_jer_general_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    int first_phy, i;
    soc_pbmp_t pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp);
    first_phy = (id * default_nof_lanes) + 1;

    for(i=0 ; i<default_nof_lanes ; i++) {
        SOC_PBMP_PORT_ADD(pbmp, first_phy+i);
    }
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit, &pbmp, &config->phy_pbmp));
   
exit:  
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_xaui_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    int i, first_phy;
    soc_pbmp_t pbmp;
    uint32 xaui_if_base_lane    = SOC_DPP_DEFS_GET(unit, xaui_if_base_lane);
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp);
    first_phy = (id * default_nof_lanes) + xaui_if_base_lane;

    for(i=0 ; i<default_nof_lanes ; i++) {
        SOC_PBMP_PORT_ADD(pbmp, first_phy + i);
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit, &pbmp, &config->phy_pbmp));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_rxaui_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    int i, first_phy;
    soc_pbmp_t pbmp;
    uint32 rxaui_if_base_lane    = SOC_DPP_DEFS_GET(unit, rxaui_if_base_lane);
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp);
    first_phy = (id * default_nof_lanes) + rxaui_if_base_lane;

    for(i=0 ; i<default_nof_lanes ; i++) {
        SOC_PBMP_PORT_ADD(pbmp, first_phy + i);
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit, &pbmp, &config->phy_pbmp));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_qsgmii_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    soc_pbmp_t phy_pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_nif_qsgmii_pbmp_get, (unit, port, id, &phy_pbmp)));

    SOC_PBMP_ASSIGN(config->phy_pbmp, phy_pbmp);
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_cpu_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (config->core == 0) {
        SOC_PBMP_PORT_ADD(config->phy_pbmp, 0);
    } else {
        SOC_PBMP_PORT_ADD(config->phy_pbmp, SOC_JERICHO_CPU_PHY_CORE_1);
    }


    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_caui_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    int i, first_phy;
    int nof_pms_per_nbi = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    soc_pbmp_t pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp); 
    if (SOC_IS_JERICHO_PLUS_ONLY(unit) && id >= nof_pms_per_nbi) {
        id += nof_pms_per_nbi; /*CAUI6-11 are in NBIL1*/
    }
    first_phy = (id * default_nof_lanes) + 1;
    
    for(i=0 ; i<default_nof_lanes ; i++) {
        SOC_PBMP_PORT_ADD(pbmp, first_phy + i);
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit, &pbmp, &config->phy_pbmp));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_xlaui2_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{
    int i, first_phy;
    soc_pbmp_t pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp); 
    if (SOC_IS_JERICHO_PLUS_ONLY(unit) && id >= 12) {
        id += 12; /*XLAUI2 12-23 are in NBIL1*/
    }
    first_phy = (id * default_nof_lanes) + 1;
    
    for(i=0 ; i<default_nof_lanes ; i++) {
        SOC_PBMP_PORT_ADD(pbmp, first_phy + i);
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit, &pbmp, &config->phy_pbmp));

exit:
    SOCDNX_FUNC_RETURN;
}


STATIC int 
soc_jer_ilkn_phy_pbmp_get(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config)
{ 
    soc_pbmp_t phy_pbmp;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(phy_pbmp);

    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_nif_ilkn_pbmp_get, (unit, port, id, &phy_pbmp, NULL)));

    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit, &phy_pbmp, &config->phy_pbmp));

exit:
    SOCDNX_FUNC_RETURN;
}


typedef int (*soc_jer_phy_pbmp_get_f)(int unit, soc_port_t port, uint32 id, uint32 default_nof_lanes, soc_jer_ucode_port_config_t* config);

typedef struct soc_jer_ucode_port_s {
    char* prefix;
    soc_port_if_t interface;
    uint32 default_nof_lanes;
    soc_jer_phy_pbmp_get_f phy_pbmp_get;
    uint32 flags;
} soc_jer_ucode_port_t;

#define SOC_JER_UCODE_FLAG_REQUIRED_INDEX   0x1
#define SOC_JER_UCODE_FLAG_NIF              0x2
#define SOC_JER_UCODE_FLAG_RESERVED         0x4

static soc_jer_ucode_port_t ucode_ports[] = {
    {"XE",          SOC_PORT_IF_XFI,        1,  &soc_jer_general_phy_pbmp_get,  SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"10GBase-R",   SOC_PORT_IF_XFI,        1,  &soc_jer_general_phy_pbmp_get,  SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF}, /*same as XE, legacy name*/
    {"XLGE2_",      SOC_PORT_IF_XLAUI2,     2,  &soc_jer_xlaui2_phy_pbmp_get,   SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"XLGE",        SOC_PORT_IF_XLAUI,      4,  &soc_jer_general_phy_pbmp_get,  SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"CGE",         SOC_PORT_IF_CAUI,       4,  &soc_jer_caui_phy_pbmp_get,     SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"ILKN",        SOC_PORT_IF_ILKN,       12, &soc_jer_ilkn_phy_pbmp_get,     SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"GE",          SOC_PORT_IF_SGMII,      1,  &soc_jer_general_phy_pbmp_get,  SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"SGMII",       SOC_PORT_IF_SGMII,      1,  &soc_jer_general_phy_pbmp_get,  SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF}, /*same as GE, legacy name*/
    {"XAUI",        SOC_PORT_IF_DNX_XAUI,   4,  &soc_jer_xaui_phy_pbmp_get,     SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"RXAUI",       SOC_PORT_IF_RXAUI,      2,  &soc_jer_rxaui_phy_pbmp_get,    SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"QSGMII",      SOC_PORT_IF_QSGMII,     1,  &soc_jer_qsgmii_phy_pbmp_get,   SOC_JER_UCODE_FLAG_REQUIRED_INDEX | SOC_JER_UCODE_FLAG_NIF},
    {"CPU",         SOC_PORT_IF_CPU,        1,  &soc_jer_cpu_phy_pbmp_get,      0},
    {"RCY",         SOC_PORT_IF_RCY,        0,  &soc_jer_general_phy_pbmp_get,  0},
    {"SAT",         SOC_PORT_IF_SAT,        0,  &soc_jer_general_phy_pbmp_get,  0},
    {"IPSEC",       SOC_PORT_IF_IPSEC,      0,  &soc_jer_general_phy_pbmp_get,  0},
    {"IGNORE",      SOC_PORT_IF_NULL,       0,  NULL,                           0},
    {"RESERVED",    SOC_PORT_IF_NULL,       0,  NULL,                           SOC_JER_UCODE_FLAG_RESERVED},
    {NULL,          SOC_PORT_IF_NULL,       0,  NULL,                           0}, /*last*/

};


STATIC int
soc_jer_str_prop_parse_ucode_port(int unit, soc_port_t port, soc_jer_ucode_port_config_t* port_config)
{
    uint32 local_chan;
    char *propkey, *propval, *s, *ss;
    char *prefix;
    int prefix_len, id = 0, idx;

    SOCDNX_INIT_FUNC_DEFS;

    port_config->is_hg = 0;
    port_config->is_nif = 0;
    port_config->is_sif = 0;
    port_config->is_kbp = 0;
    port_config->channel = 0;
    port_config->interface = SOC_PORT_IF_NULL;
    port_config->core = -1;
    port_config->tm_port = 0xFFFFFFFF;
    port_config->protocol_offset = 0;
    port_config->is_reserved = 0;
    port_config->is_lb_modem = 0;
    SOC_PBMP_CLEAR(port_config->phy_pbmp);

    propkey = spn_UCODE_PORT;
    propval = soc_property_port_get_str(unit, port, propkey);    
    s = propval;

    /* Parse interfaces */
    if (propval){

        idx = 0;

        while(ucode_ports[idx].prefix) {
            prefix = ucode_ports[idx].prefix;
            prefix_len = sal_strlen(prefix);

            /*check prefix*/
            if (!sal_strncasecmp(s, prefix, prefix_len)) {
                s += prefix_len;
                port_config->interface = ucode_ports[idx].interface;
                
                if(ucode_ports[idx].flags & SOC_JER_UCODE_FLAG_REQUIRED_INDEX) {
                    id = sal_ctoi(s, &ss);
                    if (s == ss) {
                        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("No interface index in (\"%s\") for %s"), propval, propkey)); 
                    }  
                    s = ss;
                }

                break;
            }

            idx++;
        }

        if (!ucode_ports[idx].prefix /*not found*/) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Unexpected property value (\"%s\") for %s"), propval, propkey)); 
        }

        /* check if reserved port */
        if (ucode_ports[idx].flags & SOC_JER_UCODE_FLAG_RESERVED) {
            port_config->is_reserved = TRUE;
        }

        if (s && (*s == '.')) {
            /* Parse channel number */
            ++s;
            local_chan = sal_ctoi(s, &ss);
            if (s != ss) {
                port_config->channel = local_chan;
            }
            s = ss;
        }

        /* search for additional properties */
        while (s && (*s == ':')) {
            ++s;

            /* Check if higig port */
            prefix = "hg";
            prefix_len = sal_strlen(prefix);

            if (!sal_strncasecmp(s, prefix, prefix_len)) {
                s += prefix_len;
                port_config->is_hg = 1;
                continue;
            } 

            prefix = "core_"; 
            prefix_len = sal_strlen(prefix);

            if (!sal_strncasecmp(s, prefix, prefix_len)) {

                s += prefix_len;
                port_config->core = sal_ctoi(s, &ss);
                if (s == ss) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Core not specify for (\"%s\") for %s"), propval, propkey));
                }
                s = ss;

                if (s && (*s == '.')) {
                    /* Parse tm_port number */
                    ++s;
                    port_config->tm_port = sal_ctoi(s, &ss);
                    if (s == ss) {
                        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("TM port not specify for (\"%s\") for %s"), propval, propkey));
                    }
                    s = ss; 
                }

                if (s && (*s == ':'))
                {
                    prefix = "stat";
                    prefix_len = sal_strlen(prefix);
                    ++s;
                    if (!sal_strncasecmp(s, prefix, prefix_len))
                    {
                       port_config->is_sif = 1;
                    } else 
                    {
                        prefix = "kbp";
                        prefix_len = sal_strlen(prefix);
                        if (!sal_strncasecmp(s, prefix, prefix_len))
                        {
                           port_config->is_kbp = 1;
                        } else 
                        {
                            prefix = "modem";
                            prefix_len = sal_strlen(prefix);
                            if (!sal_strncasecmp(s, prefix, prefix_len))
                            {
                                port_config->is_lb_modem = 1;
                            } 
                        }
                    }
                }

                continue;
            } 
        }

        if(port_config->core == -1) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Core not specify for (\"%s\") for %s"), propval, propkey));
        }

        if(port_config->tm_port == 0xFFFFFFFF && (!(port_config->is_sif)) && (!(port_config->is_kbp))) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("TM port not specify for (\"%s\") for %s"), propval, propkey));
        }

        if(ucode_ports[idx].flags & SOC_JER_UCODE_FLAG_NIF) {
            port_config->is_nif = 1;
        }

        if(ucode_ports[idx].phy_pbmp_get) {
            SOCDNX_IF_ERR_EXIT(ucode_ports[idx].phy_pbmp_get(unit, port, id, ucode_ports[idx].default_nof_lanes, port_config));
        }

        port_config->protocol_offset = id;
    } 


exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_ports_config(int unit) 
{
    uint32 flags = 0,is_fiber_perf, is_pon, init_vid_enabled,value;
    uint32 nof_channels = 0;
    int port_i;
    int primary_port = 0, val, port_offset = 0;
    soc_jer_ucode_port_config_t port_config;
    int rv, core, is_channelized;
    soc_info_t          *si;
    uint32 erp_tm_port_id, erp_base_q_pair;
    soc_pbmp_t phy_ports, ports_bm,pon_port_bm, is_tdm_queuing_on_bm;
    soc_port_if_t interface;
    ARAD_PORT_HEADER_TYPE header_type_in, header_type_out;
    uint32 is_hg_header;

    SOCDNX_INIT_FUNC_DEFS;
    
    si  = &SOC_INFO(unit);
    if (SOC_WARM_BOOT(unit)) {
       /*  
        * Take a snapshot of port sw data base and then restore it back in order to update bitmaps
        */       
       SOCDNX_IF_ERR_EXIT(soc_port_sw_db_snapshot_take(unit));
    }
    /* VOQ mapping mode - needed for sysport2basequeue data structure - must be before soc_port_sw_db_init()*/
    rv = soc_arad_str_prop_voq_mapping_mode_get(unit, &SOC_DPP_CONFIG(unit)->arad->voq_mapping_mode);
    SOCDNX_IF_ERR_EXIT(rv);

    /* HQOS mapping enable - needed for sysport2modport data structure (enables many sys ports to one mod port mapping) - must be before soc_port_sw_db_init()*/
    rv = soc_arad_str_prop_hqos_mapping_get(unit, &SOC_DPP_CONFIG(unit)->arad->hqos_mapping_enable);
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_init(unit));

    /* init reserved ports */
    sal_memset(SOC_DPP_CONFIG(unit)->arad->reserved_ports, 0x0, sizeof(SOC_DPP_CONFIG(unit)->arad->reserved_ports));

    /* init reserved ISQ and FMQ base q pairs */
    soc_arad_ps_reserved_mapping_init(unit);


    if (SOC_WARM_BOOT(unit)) {

       SOCDNX_IF_ERR_EXIT(soc_port_sw_db_snapshot_restore(unit));

    }
    SOCDNX_IF_ERR_EXIT(soc_jer_egr_interface_init(unit));

    for (port_i = 0; port_i <  SOC_DPP_DEFS_GET(unit, nof_logical_ports); ++port_i) 
    {
        SOC_INFO(unit).port_l2pp_mapping[port_i] = -1;
        SOC_INFO(unit).port_l2po_mapping[port_i] = -1;
        if(SOC_E_NONE == soc_phy_primary_and_offset_get(unit, port_i, &primary_port, &port_offset)){
            SOC_INFO(unit).port_l2pp_mapping[port_i] = primary_port;
            SOC_INFO(unit).port_l2po_mapping[port_i] = port_offset;
        }
        SOC_INFO(unit).port_l2pa_mapping[port_i] = soc_property_port_get(unit, port_i, spn_PORT_PHY_ADDR, 0xFF);
        /* parse ucode_port */
        rv = soc_jer_str_prop_parse_ucode_port(unit, port_i, &port_config);
        SOCDNX_IF_ERR_EXIT(rv);

        /*Check for errors*/
        if ((port_config.interface != _SHR_PORT_IF_NULL) && (port_i >= FABRIC_LOGICAL_PORT_BASE(unit))) {
            LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Port %d is out-of-range. Max legal port is %d!\n"), port_i, (FABRIC_LOGICAL_PORT_BASE(unit)-1)));
            SOCDNX_IF_ERR_EXIT(SOC_E_CONFIG);
        }

        if ((SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores == 1) && (port_config.core > 0)) {
            LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error, in single core mode all ports must be on core 0\n")) );
            SOCDNX_IF_ERR_EXIT(SOC_E_CONFIG);
        }

        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_nif_sku_restrictions, (unit, port_config.phy_pbmp, port_config.interface, port_config.protocol_offset, port_config.is_kbp)));

        /* pre defined ports, needed in case dynamic nif feature is active in order to preserve tm ports */
        if (port_config.is_reserved) {
            SOC_DPP_CONFIG(unit)->arad->reserved_ports[port_i].is_reserved = TRUE;
            SOC_DPP_CONFIG(unit)->arad->reserved_ports[port_i].core = port_config.core;
            SOC_DPP_CONFIG(unit)->arad->reserved_ports[port_i].tm_port = port_config.tm_port;
            continue;
        }

        if(port_config.interface == SOC_PORT_IF_NULL) {
            continue;
        }

        flags = 0;

        /* Is NIF ? */
        if(port_config.is_nif) {
            flags |= SOC_PORT_FLAGS_NETWORK_INTERFACE;
        }

        /* Is STIF ? */
        if(port_config.is_sif) {
            flags |= SOC_PORT_FLAGS_STAT_INTERFACE;
        }

        /* Is KBIF ? */
        if(port_config.is_kbp) {
            flags |= SOC_PORT_FLAGS_ELK;
        }

        /* Is LBIF ? */
        if(port_config.is_lb_modem) {
            flags |= SOC_PORT_FLAGS_LB_MODEM;
        }

        /* Is PON ? */
        rv = soc_arad_str_prop_parse_pon_port(unit, port_i, &is_pon);
        SOCDNX_IF_ERR_EXIT(rv);

        if (is_pon) {
            SOC_DPP_CONFIG(unit)->pp.pon_application_enable = 1;
            flags |= SOC_PORT_FLAGS_PON_INTERFACE;
        }

        /* indicate pon port */
        if (is_pon) {
            PORT_SW_DB_PORT_ADD(pon, port_i);
        }

        /* Is init VID enabled ? */
        rv = soc_arad_str_prop_parse_init_vid_enabled_port_get(unit, port_i, &init_vid_enabled);
        SOCDNX_IF_ERR_EXIT(rv);

        /* In this port, support use Initial-VID with no differencebetween 
         * untagged packets and tagged packets. and need install initial-vid
         * program when init isem DB.
         * The decision to install programs on init isem DB is decided by 
         * port_use_initial_vlan_only_enabled parameter.
         */
        if (!init_vid_enabled) {
            SOC_DPP_CONFIG(unit)->pp.port_use_initial_vlan_only_enabled = 1;
            flags |= SOC_PORT_FLAGS_INIT_VID_ONLY;
        } else {
            /* indicates if we really need to allocate some default programs */
            /* In case Initial-VID is enabled for all ports then we can skip some programs */
            SOC_DPP_CONFIG(unit)->pp.port_use_outer_vlan_and_initial_vlan_enabled = 1;
        }

        if (!SOC_WARM_BOOT(unit)) 
        {
            /* Add to DB */
            rv = soc_port_sw_db_port_validate_and_add(unit, port_config.core, port_i, port_config.channel, flags, port_config.interface, port_config.phy_pbmp);
            SOCDNX_IF_ERR_EXIT(rv);

            if ( port_config.interface == SOC_PORT_IF_ILKN || port_config.interface == SOC_PORT_IF_CAUI || 
                 port_config.interface == SOC_PORT_IF_QSGMII) {
                rv = soc_port_sw_db_protocol_offset_set(unit, port_i, 0, port_config.protocol_offset);
                SOCDNX_IF_ERR_EXIT(rv);
            }

            rv = soc_port_sw_db_is_hg_set(unit, port_i, port_config.is_hg);
            SOCDNX_IF_ERR_EXIT(rv);

            rv = soc_port_sw_db_local_to_tm_port_set(unit, port_i, port_config.tm_port);
            SOCDNX_IF_ERR_EXIT(rv);

            rv = soc_port_sw_db_local_to_pp_port_set(unit, port_i, port_config.tm_port); /* tm_port == pp_port*/
            SOCDNX_IF_ERR_EXIT(rv);

            /* Is Fiber ? */
            is_fiber_perf = soc_property_port_get(unit, port_i, spn_SERDES_FIBER_PREF, 0);
            if(is_fiber_perf) {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flag_add(unit, port_i, SOC_PORT_FLAGS_FIBER));
            }
        }
    }

    if (!SOC_WARM_BOOT(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_jer_port_nif_quad_to_core_validate(unit));
    }

    SOC_PBMP_CLEAR(pon_port_bm);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, SOC_PORT_FLAGS_PON_INTERFACE, &pon_port_bm));

    SOC_PBMP_ITER(pon_port_bm, port_i) {
        /* CPU port is already channelized. Skip it. */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port_i, &interface));

        if (SOC_PORT_IF_CPU == interface) {
            continue;
        }

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_of_channels_get(unit, port_i, &nof_channels));

        if (nof_channels > 1) {
          SOC_DPP_CONFIG(unit)->pp.pon_port_channelization_enable = 1;
        }

        /* Get max channel num of PON port */
        if (nof_channels > SOC_DPP_CONFIG(unit)->pp.pon_port_channelization_num) {
            SOC_DPP_CONFIG(unit)->pp.pon_port_channelization_num = nof_channels;
        }
    }


    /* Add OLP/OAMP/ERP ports */
    SOC_PBMP_CLEAR(phy_ports);

    erp_base_q_pair = (ARAD_EGR_NOF_PS-1)*ARAD_EGR_NOF_Q_PAIRS_IN_PS;
    SOC_DPP_CORES_ITER(SOC_CORE_ALL, core) 
    {
        si->olp_port[core] = -1;
        val = soc_property_suffix_num_get(unit, core, spn_NUM_OLP_TM_PORTS, "core", 0);
        if (val > 0) {
            si->olp_port[core] = ARAD_OLP_PORT_ID + core;
            if (!SOC_WARM_BOOT(unit)) 
            {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_validate_and_add(unit, core, si->olp_port[core], 0, 0, SOC_PORT_IF_OLP, phy_ports));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_set(unit, si->olp_port[core], ARAD_OLP_PORT_ID));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_pp_port_set(unit, si->olp_port[core], ARAD_OLP_PORT_ID));  
            }
        }

        si->oamp_port[core] = -1;
        SOCDNX_IF_ERR_EXIT(dcmn_property_suffix_num_get(unit, core, spn_NUM_OAMP_PORTS, "core", 0,&value));
        val = value;
        if (val > 0 || soc_property_get(unit, spn_SAT_ENABLE, 0)) {
            si->oamp_port[core] = ARAD_OAMP_PORT_ID + core;
            if (!SOC_WARM_BOOT(unit)) 
            {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_validate_and_add(unit, core, si->oamp_port[core], 0, 0, SOC_PORT_IF_OAMP, phy_ports));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_set(unit, si->oamp_port[core], ARAD_OAMP_PORT_ID));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_pp_port_set(unit, si->oamp_port[core], ARAD_OAMP_PORT_ID));
            }
        }

        si->erp_port[core] = -1;
        val = soc_property_suffix_num_get(unit, core, spn_NUM_ERP_TM_PORTS, "core", 0);
        if (val > 0) {
            si->erp_port[core] = SOC_DPP_PORT_INTERNAL_ERP(core);
            if (!SOC_WARM_BOOT(unit)) 
            {
                erp_tm_port_id = soc_property_suffix_num_get(unit, si->erp_port[core], spn_LOCAL_TO_TM_PORT, "erp", ARAD_ERP_PORT_ID);
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_validate_and_add(unit, core, si->erp_port[core], 0, 0, SOC_PORT_IF_ERP, phy_ports));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_set(unit, si->erp_port[core], erp_tm_port_id)); 
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_pp_port_set(unit, si->erp_port[core], erp_tm_port_id)); /*meaningless, added for compliance*/  
                rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.set(unit, si->erp_port[core], erp_base_q_pair);
                SOCDNX_IF_ERR_EXIT(rv);
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_out_port_priority_set(unit, si->erp_port[core], 1));
            }
        }
    }

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, 0, &ports_bm));

    /* Get Speeds for all ports */
    if (!SOC_WARM_BOOT(unit)) 
    {

        SOC_PBMP_ITER(ports_bm, port_i) {
            val = soc_property_port_get(unit, port_i, spn_PORT_INIT_SPEED, -1);
            if(-1 == val) {
                SOCDNX_IF_ERR_EXIT(soc_pm_default_speed_get(unit, port_i, &val));
            }
            if (dcmn_device_block_for_feature(unit,DCMN_NO_DC_FEATURE)) {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port_i, &interface));
                if ((val == 50000 && interface == SOC_PORT_IF_XLAUI2) ||
                    (val == 25000 && interface == SOC_PORT_IF_XFI)) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("device %s blocked for interface XLAUI2 with speed 50000  or interface XFI with speed 25000"), 
                                                      soc_dev_name(unit)));
                }
            }

            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_set(unit, port_i, val));
        }
    }

    /* ILKN TDM SP mode */
    val = soc_property_get(unit, spn_ILKN_TDM_DEDICATED_QUEUING, 0);
    SOC_DPP_CONFIG(unit)->arad->init.ilkn_tdm_dedicated_queuing = (val == 0) ? ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_OFF : ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON;

    /* allocate egress interface per core */
    if (!SOC_WARM_BOOT(unit)) {
        SOC_PBMP_CLEAR(is_tdm_queuing_on_bm);
        SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.jericho.tm.is_tdm_queuing_on.set(unit, is_tdm_queuing_on_bm));
        SOC_PBMP_ITER(ports_bm, port_i) {

            /* At this point we need to update out header type before egress interface allocation in order to know whether a port is TDM or not
             * Header typesa are later read again in - arad_drv.c
             */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port_i, &interface));
            if (interface != SOC_PORT_IF_ERP) {
                if (soc_dpp_str_prop_parse_tm_port_header_type(unit, port_i, &(header_type_in), &(header_type_out), &is_hg_header)) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("soc_dpp_str_prop_parse_tm_port_header_type error")));       
                }
            }
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_hdr_type_out_set(unit,port_i,header_type_out));

            val = soc_property_port_get(unit, port_i, spn_TDM_QUEUING_FORCE, 0);
            if (val)
            {
                SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.jericho.tm.is_tdm_queuing_on.pbmp_port_add(unit,port_i));
            }
            SOCDNX_IF_ERR_EXIT(soc_jer_egr_interface_alloc(unit, port_i));
        }
    }

    /* Set calendar mode */
    if (!SOC_WARM_BOOT(unit)) 
    {
        SOC_PBMP_ITER(ports_bm, port_i) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_channelized_port_get(unit, port_i, &is_channelized));
            if (is_channelized) {
                /* by default dual mode is used */
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_single_cal_mode_set(unit, port_i, FALSE));
            } else {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_single_cal_mode_set(unit, port_i, TRUE));
            }
        }
    }

    SOC_DPP_CONFIG(unit)->jer->tm.is_ilkn_big_cal = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "ilkn_big_cal", 0);

exit:
    SOCDNX_FUNC_RETURN;
}



/*
 * Configures soc data structures specific to Jericho
 */
int soc_dpp_get_default_config_jer(
    int unit)
{

    soc_dpp_config_jer_t *jer;

    SOCDNX_INIT_FUNC_DEFS;

    jer = SOC_DPP_JER_CONFIG(unit);

    /* Call special jer/qax functions to fix soc_dpp_defines_t
     * (in case of qmx (jer) or kalia (qax) */
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_fabric_link_config_ovrd,(unit)));



    /* Already reset in SOC attach */
    sal_memset(jer, 0, sizeof(soc_dpp_config_jer_t));

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_jer_pp_config_protection_get
 * Purpose:
 *      Retrieve Jericho specific configuration for the PP Protection module.
 *      Retrieve SOC Property values.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      void
 */
void soc_jer_pp_config_protection_get(
    int unit)
{
    soc_dpp_config_jer_pp_t *jer_pp_config = &(SOC_DPP_JER_CONFIG(unit)->pp);
    char *prop_key;
    uint32 prop_value;

    /* Get the SOC Property value for the Ingress protection Coupled mode
       Default: Coupled mode (1)
       The mode is valid only for Jericho and above */
    prop_key = spn_BCM886XX_INGRESS_PROTECTION_COUPLED_MODE;
    prop_value = soc_property_get(unit, prop_key, 1);
    jer_pp_config->protection_ingress_coupled_mode = (prop_value) ? 1 : 0;

    /* Get the SOC Property value for the Egress protection Coupled mode
       Default: Coupled mode (1)
       The mode is valid only for Jericho and above */
    prop_key = spn_BCM886XX_EGRESS_PROTECTION_COUPLED_MODE;
    prop_value = soc_property_get(unit, prop_key, 1);
    jer_pp_config->protection_egress_coupled_mode = (prop_value) ? 1 : 0;

    /* Get the SOC Property value for the FEC protection accelerated reroute mode
       Default: Not accelerated (0)
       The mode is valid only for Jericho and above */
    prop_key = spn_BCM886XX_FEC_ACCELERATED_REROUTE_MODE;
    prop_value = soc_property_get(unit, prop_key, 0);
    jer_pp_config->protection_fec_accelerated_reroute_mode = (prop_value) ? 1 : 0;
}

/*
 * Function:
 *      soc_jer_pp_config_kaps_get
 * Purpose:
 *      Retrieve Jericho specific configuration for the PP KAPS module.
 *      Retrieve SOC Property values.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      void
 */
void soc_jer_pp_config_kaps_get(
    int unit)
{
    soc_dpp_config_jer_pp_t *jer_pp_config = &(SOC_DPP_JER_CONFIG(unit)->pp);

    char *prop_key;
    uint32 prop_value;

    /* Get the SOC Property value for the KAPS public table size
       Default: 0
       The property is valid only for Jericho and above */
    prop_key = spn_PUBLIC_IP_FRWRD_TABLE_SIZE;
    prop_value = soc_property_get(unit, prop_key, 0);
    jer_pp_config->kaps_public_ip_frwrd_table_size = prop_value;

    /* Get the SOC Property value for the KAPS private table size
       Default: 0
       The property is valid only for Jericho and above */
    prop_key = spn_PRIVATE_IP_FRWRD_TABLE_SIZE;
    prop_value = soc_property_get(unit, prop_key, 0);
    jer_pp_config->kaps_private_ip_frwrd_table_size = prop_value;

    /* Get the SOC Property value for the KAPS direct aceess table size
       Default: 0
       The property is valid only for Jericho and above */
    prop_key = spn_PMF_KAPS_LARGE_DB_SIZE;
    prop_value = soc_property_get(unit, prop_key, 0);
    jer_pp_config->kaps_large_db_size = prop_value;

    
}

/*
 * Function:
 *      soc_jer_pp_config_lif_get
 * Purpose:
 *      Retrieve Jericho specific configuration for the PP lif module.
 *      Retrieve SOC Property values.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      void
 */
void soc_jer_pp_config_lif_get(
    int unit){
    soc_dpp_config_pp_t *dpp_pp = &(SOC_DPP_CONFIG(unit))->pp;
    uint32 prop_val;

    /* Nof global out lifs: If system is in Arad mode, then we must limit the global lif size to 16 bits (64k).
        Otherwise, just use the default (already defined). */
    prop_val = soc_property_get(unit, spn_SYSTEM_IS_ARAD_IN_SYSTEM, 0);

    if (prop_val) {
        dpp_pp->nof_global_out_lifs = SOC_DPP_NOF_GLOBAL_LIFS_ARAD;
    }
}


/*
 * Function:
 *      soc_jer_pp_config_rif_get
 * Purpose:
 *      check Jericho specific configuration.
 *      Retrieve SOC Property values.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_pp_config_rif_get(
    int unit)
{
    soc_dpp_config_l3_t *dpp_l3 = &(SOC_DPP_CONFIG(unit))->l3;
    char *prop_key;
    uint32 prop_value;
    int rif_multiplier = SOC_DPP_DEFS_GET((unit), nof_rifs); 

    SOCDNX_INIT_FUNC_DEFS;

    /* Get the SOC Property value for the RIF max id 
       Default: 4096
       The property is valid only for Jericho and above
       Check parameter is in range*/
    prop_key = spn_RIF_ID_MAX;
    prop_value = soc_property_get(unit, prop_key, rif_multiplier);
    /* Both multiples of the device's multiplier, and multiplier-1 are acceptable. */
    if (prop_value % rif_multiplier == (rif_multiplier - 1)) {
        prop_value++;
    }
    if (prop_value % rif_multiplier != 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Soc property RIF_ID_MAX must be a multiple of %d."), rif_multiplier));
    }
    if (prop_value > SOC_DPP_MAX_RIF_ID + 1)
    {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Parameter for %s out of range. Valid range 0 - 32K-1."), prop_key));
    }


    dpp_l3->nof_rifs = prop_value;

exit: 
    SOCDNX_FUNC_RETURN;

}


/*
 * Function:
 *      soc_jer_specific_info_config_direct
 * Purpose:
 *      Configures soc properties specific for Jericho which are not dependent on
 *      other common soc properties to both Jericho and Arad.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_specific_info_config_direct(int unit)
{
    int rv, fmc_dbuff_mode,offset;
    uint32 dram_total_size,value;
    SOCDNX_INIT_FUNC_DEFS;

    

    /* only ILKN4/ILKN5 can be directed to fabric */
    SOCDNX_IF_ERR_EXIT(dcmn_property_suffix_num_get(unit, 4, spn_USE_FABRIC_LINKS_FOR_ILKN_NIF, "ilkn", 0,&value));
    SOC_DPP_CONFIG(unit)->jer->nif.ilkn_over_fabric[0] = value;
    SOCDNX_IF_ERR_EXIT(dcmn_property_suffix_num_get(unit, 5, spn_USE_FABRIC_LINKS_FOR_ILKN_NIF, "ilkn", 0,&value));
    SOC_DPP_CONFIG(unit)->jer->nif.ilkn_over_fabric[1] = value;

    for (offset = 0; offset < MAX_NUM_OF_PMS_IN_ILKN; offset++) {
            SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_short[offset] = soc_property_port_get(unit, offset, spn_ILKN_BURST_SHORT, 32);
            SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_min[offset] = soc_property_port_get(unit, offset, spn_ILKN_BURST_MIN, SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_short[offset]);
            SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_max[offset] = soc_property_port_get(unit, offset, spn_ILKN_BURST_MAX, 256);
    }

    SOC_INFO(unit).fabric_logical_port_base = soc_property_get(unit, spn_FABRIC_LOGICAL_PORT_BASE, SOC_DPP_FABRIC_LOGICAL_PORT_BASE_DEFAULT);
    if (SOC_IS_QAX(unit)) {
        SOC_DPP_CONFIG(unit)->qax->link_bonding_enable = soc_property_get(unit, spn_LINK_BONDING_ENABLE, 0);
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_ports_config(unit));

    rv = soc_jer_str_prop_mc_nbr_full_dbuff_get(unit, &fmc_dbuff_mode);
    SOCDNX_IF_ERR_EXIT(rv);
    SOC_DPP_CONFIG(unit)->arad->init.dram.fmc_dbuff_mode = fmc_dbuff_mode;

    
    if (SOC_IS_QAX(unit)) {
        /* In this calculation the original order of the multiplications caused a wrap around in certain cases, so the order was changed */
        SOC_DPP_CONFIG(unit)->jer->dbuffs.max_nof_dram_buffers = (((SOC_DPP_CONFIG(unit)->arad->init.dram.dram_size_total_mbyte) * 1024) / (SOC_DPP_CONFIG(unit)->arad->init.dram.dbuff_size)) * 1024;
    } else {
        SOCDNX_IF_ERR_EXIT(handle_sand_result(arad_init_pdm_nof_entries_calc(unit, SOC_DPP_CONFIG(unit)->arad->init.dram.pdm_mode, &dram_total_size)));
        /* In this calculation the original order of the multiplications caused a wrap around in certain cases, so the order was changed */
        SOC_DPP_CONFIG(unit)->jer->dbuffs.max_nof_dram_buffers = SOC_SAND_MIN(dram_total_size, ((SOC_DPP_CONFIG(unit)->arad->init.dram.dram_size_total_mbyte * 1024) / SOC_DPP_CONFIG(unit)->arad->init.dram.dbuff_size) * 1024); 
    }

    /* Marking non-dynamic tables as cachable */
    arad_tbl_mark_cachable(unit);
    /* Enable/Disable truncate (IRPP editing) for counter processor */
    SOC_DPP_CONFIG(unit)->jer->tm.truncate_delta_in_pp_counter[0] = soc_property_suffix_num_get(unit, 0, spn_TRUNCATE_DELTA_IN_PP_COUNTER, "0", 0);
    SOC_DPP_CONFIG(unit)->jer->tm.truncate_delta_in_pp_counter[1] = soc_property_suffix_num_get(unit, 0, spn_TRUNCATE_DELTA_IN_PP_COUNTER, "1", 0);

exit: 
    SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_jer_specific_info_config_derived
 * Purpose:
 *      Configures soc properties specific for Jericho which are dependent on soc properties 
 *      which are either common to both Jericho and Arad or uniqe for Jericho.
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_specific_info_config_derived(int unit) 
{
    ARAD_MGMT_INIT *init = &(SOC_DPP_CONFIG(unit)->arad->init);
    int pml = 0, pll_binding = 0;
    SOCDNX_INIT_FUNC_DEFS;

    if (init->pp_enable == TRUE) {
        /* Get Jericho specific Protection configuration */
        soc_jer_pp_config_protection_get(unit);
        /* Get KAPS configuration */
        soc_jer_pp_config_kaps_get(unit);

        SOCDNX_IF_ERR_EXIT(soc_jer_pp_config_rif_get(unit));
        
        soc_jer_pp_config_lif_get(unit);
    }

    /* arp entry msbs for roo host format. 
     * This soc property is used to configure MactFormat3EeiBits[2:1]. 
     * in arad+ eei format: 1111 HI(4) Native-ARP(15) 0.
     *  jericho eei format: 11 MactFormat3EeiBits[2:1] HI(4) Native-ARP(15) MactFormat3EeiBits[0]
     */
    SOC_DPP_CONFIG(unit)->jer->pp.roo_host_arp_msbs = (soc_property_get(unit, spn_BCM886XX_ROO_HOST_ARP_MSBS, 0x0)); 

    /* configure LAG to use the LSB/MSB of the lb-key */
    init->ports.smooth_division_resolve_using_msb = (soc_property_get(unit, spn_TRUNK_RESOLVE_USE_LB_KEY_MSB_SMOOTH_DIVISION, 0x0));
    init->ports.stack_resolve_using_msb = (soc_property_get(unit, spn_TRUNK_RESOLVE_USE_LB_KEY_MSB_STACK, 0x0));

    SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[0] = soc_property_suffix_num_get(unit, 0, spn_SERDES_NIF_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
    SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_out[0] = soc_property_suffix_num_get(unit, 0, spn_SERDES_NIF_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
    SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_synce_out[0] = soc_property_suffix_num_get(unit, 0, spn_SYNC_ETH_CLK_DIVIDER, "clk", soc_dcmn_init_serdes_ref_clock_156_25);
    SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_synce_out[1] = soc_property_suffix_num_get(unit, 1, spn_SYNC_ETH_CLK_DIVIDER, "clk", soc_dcmn_init_serdes_ref_clock_156_25);
    if (SOC_IS_QUX(unit)) {
        /* For Qux the input ref clk is 156.25/125 MHz, the output ref clk is 156.25Mhz */
        SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmx_in = soc_property_suffix_num_get(unit, 3, spn_SERDES_NIF_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
        SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmx_out = soc_dcmn_init_serdes_ref_clock_156_25;
    } else {
          SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[1] = soc_property_suffix_num_get(unit, 1, spn_SERDES_NIF_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
          SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_out[1] = soc_property_suffix_num_get(unit, 1, spn_SERDES_NIF_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
          SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmh_in = soc_property_suffix_num_get(unit, 2, spn_SERDES_NIF_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
          SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmh_out = soc_property_suffix_num_get(unit, 2, spn_SERDES_NIF_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
          if (SOC_IS_QAX(unit)) {
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_in[0] = soc_property_suffix_num_get(unit, 2, spn_SERDES_NIF_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_out[0] = soc_property_suffix_num_get(unit, 2, spn_SERDES_NIF_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_in[1] = soc_property_suffix_num_get(unit, 2, spn_SERDES_NIF_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_out[1] = soc_property_suffix_num_get(unit, 2, spn_SERDES_NIF_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
          } else {
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_in[0] = soc_property_suffix_num_get(unit, 0, spn_SERDES_FABRIC_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_out[0] = soc_property_suffix_num_get(unit, 0, spn_SERDES_FABRIC_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_in[1] = soc_property_suffix_num_get(unit, 1, spn_SERDES_FABRIC_CLK_FREQ, "in", soc_dcmn_init_serdes_ref_clock_156_25);
              SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_out[1] = soc_property_suffix_num_get(unit, 1, spn_SERDES_FABRIC_CLK_FREQ, "out", soc_dcmn_init_serdes_ref_clock_156_25);
          }
      }

    if (SOC_IS_QAX(unit)) {
        /* Check for QAX ILKN PLL binding mode */
        for (pml = 0; pml < SOC_DPP_DEFS_GET(unit, nof_instances_nbil); ++pml) {
            pll_binding = soc_property_suffix_num_get(unit, pml, spn_SERDES_NIF_CLK_BINDING, "in", 0); 
            if (pll_binding) {
                if (SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[pml] != SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmh_out) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Soc properties mismatch. Both SERDES_NIF_CLK_BINDING is enabled and "
                                                                        "SERDES_NIF_CLK_FREQ_IN is set to value different than SERDES_NIF_CLK_FREQ_OUT_2. "
                                                                        "Please match these configurations")));
                }
                /* to enable ILKN to work on more than one NBI block,
                 * the input to PML0/1 PLL is forced from the output of PMH PLL */
                SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[pml] = SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmh_out;
                SOC_DPP_CONFIG(unit)->jer->pll.is_pll_binding_pml[pml] = 1;
            } else {
                SOC_DPP_CONFIG(unit)->jer->pll.is_pll_binding_pml[pml] = 0;
            }
        } 
    }

exit: 
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_init_dma_init
 * Purpose:
 *      init cmic DMA mechanizems: SBUSDMA, Packet DMA
 * Parameters:
 *      unit -  unit number
 * Returns:
 *      SOC_E_XXX
 *
 */
int soc_jer_init_dma_init(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

    /* Packet DMA descriptors Initilazation */
    soc_dcb_unit_init(unit);

    /* Setup Packet DMA structures when a device is attached */
    SOCDNX_IF_ERR_EXIT(soc_dma_attach(unit, 1 /* Reset */));

    /* Init cmic_pcie_userif_purge_ctrl */
    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_pcie_userif_purge_ctrl_init(unit));

    /* Init mutexes for Table/Slam DMA */
    SOCDNX_IF_ERR_EXIT(soc_arad_dma_mutex_init(unit));

#ifdef BCM_SBUSDMA_SUPPORT
    /* Init SBUSDMA descriptors */
    if (soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm)) ||
      soc_mem_slamable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm))) { /* check if DMA is enabled */
        SOCDNX_IF_ERR_EXIT(soc_sbusdma_init(unit, 0, 0));

        /* Init descriptor DMA */
        if (soc_property_get(unit, spn_DMA_DESC_AGGREGATOR_CHAIN_LENGTH_MAX, 0)) {
            uint32 desc_num_max, desc_mem_buff_size, desc_timeout_usec;
            if (((soc_property_get(unit, spn_DMA_DESC_AGGREGATOR_BUFF_SIZE_KB, 0)) == 0) ||
                ((soc_property_get(unit, spn_DMA_DESC_AGGREGATOR_TIMEOUT_USEC, 0)) == 0)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("desc_num_max, desc_mem_buff_size and desc_timeout_usec must be all initialized together.")));
            }

            desc_num_max = soc_property_get(unit, spn_DMA_DESC_AGGREGATOR_CHAIN_LENGTH_MAX, 0);
            /* divide by 2 for each buffer in the double-buffer and translate from kb to uint32 */
            desc_mem_buff_size = soc_property_get(unit, spn_DMA_DESC_AGGREGATOR_BUFF_SIZE_KB, 0) * 128;
            desc_timeout_usec = soc_property_get(unit, spn_DMA_DESC_AGGREGATOR_TIMEOUT_USEC, 0);

            SOCDNX_IF_ERR_EXIT(jer_sbusdma_desc_init(unit, desc_num_max, desc_mem_buff_size, desc_timeout_usec));
        }
    }
#endif

    SOCDNX_IF_ERR_EXIT(dcmn_init_fill_table(unit));   
    SOCDNX_IF_ERR_EXIT(jer_mgmt_dma_fifo_source_channels_db_init(unit));   

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Purpose:
 *      de-init cmic DMA mechanizems: SBUSDMA, Packet DMA
 * Parameters:
 *      unit -  unit number
 * Returns:
 *      SOC_E_XXX
 *
 */
int soc_jer_deinit_dma(int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

#ifdef BCM_SBUSDMA_SUPPORT
    /* De-init SBUSDMA descriptors */
    if (soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm)) ||
      soc_mem_slamable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm))) { /* check if DMA is enabled */
        SOCDNX_IF_ERR_EXIT(soc_sbusdma_desc_detach(unit));
    }
#endif
    SOCDNX_IF_ERR_EXIT(dcmn_deinit_fill_table(unit));

    /* Detach DMA */
    soc_arad_dma_mutex_destroy(unit);

    SOCDNX_IF_ERR_EXIT(soc_dma_detach(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

/* Configure CMIC. */
int soc_jer_init_reset_cmic_regs(
    int unit)
{
    uint32 core_freq = 0x0;
    uint32 rval = 0;
    int schan_timeout = 0x0;
    int dividend, divisor;
    int mdio_int_freq, mdio_delay;

    SOCDNX_INIT_FUNC_DEFS;

    /*
     * Map the blocks to their Sbus rings.
     * SBUS ring map:
     * Ring 2:
     * Ring 3:
     * Ring 5:
     * Ring 7:
     */
    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x00222227));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x24442220));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x22222222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x53333222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x55555555));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x32333335));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x00000022));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x00000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x00000000));
    } else if (SOC_IS_QAX(unit)) {
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x00022227));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x00333220));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x22022000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x22222022));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x00222222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x02000000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x30330002));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x33033333));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x55555553));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x55555555));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x55555555));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x55555555));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x00005555));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x22222000));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x02044402));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x00000002));

    } else if (SOC_IS_JERICHO_PLUS_A0(unit)) {
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x04444447));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x22222334));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x33433222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x33333333));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x22222233));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x22222222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x55555522));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x44444665));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x44444444));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x66226666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x44555066));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x00004633));

        } else {

        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x04444447)); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x22222334));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x33433222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x33333333));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x22222233));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x55222222));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x46655555));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x42444444));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_64_71r(unit, 0x66666664));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_72_79r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_80_87r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_88_95r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_96_103r(unit, 0x66666666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_104_111r(unit, 0x55220666));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_112_119r(unit, 0x62633445));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_120_127r(unit, 0x20000000));

        /* LED PROCESSOR */  

        /* setting Jericho ports status scan values */  
        /* In order to match the phisical order in led data memory, */
        /* i.e. port1 resides in index 1, port2 in index2 etc...,  */
        /* LED processor 0 - will handle ports  1-24.  port 1  resides in index 1 in data ram */
        /* LED processor 1 - will handle ports 25-84.  port 25 resides in index 1 in data ram */
        /* LED processor 2 - will handle ports 85-144. port 85 resides in index 1 in data ram */

        /* PMH ports chain is connected to LED0, 24 ports */  
        /* Phisical ports status chain to LED0 is following, port4 resides in port0 place:*/        
        /* 21->22->23->24->13->14->15->16->17->18->19->20->9->10->11->12->5->6->7->8->1->2->3->4 */
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_0f, 4); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_1f, 3); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_2f, 2); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_3f, 1); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_4f, 8); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_5f, 7); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_6f, 6); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_7f, 5); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_8f,  12); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_9f,  11); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_10f, 10); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_11f,  9); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_12f, 20); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_13f, 19); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_14f, 18); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_15f, 17); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_16f, 16); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_17f, 15); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_18f, 14); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_19f, 13); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_20f, 24); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_21f, 23); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_22f, 22); 
        soc_reg_field_set(unit, CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_23f, 21); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r(unit,   rval));

        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_24_27r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_28_31r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_32_35r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_36_39r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_40_43r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_44_47r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_48_51r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_52_55r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_56_59r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_60_63r(unit, 0x0));

        /* PML0 ports chain is connected to LED1, 24 ports */  
        /* Phisical ports status chain to LED1 is following, port36 resides in port0 place:*/        
        /* 69->73->77->81->53->57->61->65->37->41->45->49->29->30->31->32->25->26->27->28->33->34->35->36 */
        /* index in phisical LED DRAM memory port should reside (reduce 24 from phisical port number): */        
        /* 45->49->53->57->29->33->37->41->13->17->21->25-> 5-> 6-> 7-> 8-> 1-> 2-> 3-> 4-> 9->10->11->12 */
        
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_0f, 12); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_1f, 11); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_2f, 10); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_3f,  9); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_0_3r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_4f, 4); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_5f, 3); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_6f, 2); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_7f, 1); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_4_7r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_8f,  8); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_9f,  7); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_10f, 6); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_11f, 5); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_8_11r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_12f, 25); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_13f, 21); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_14f, 17); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_15f, 13); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_12_15r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_16f, 41); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_17f, 37); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_18f, 33); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_19f, 29); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_16_19r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_20f, 57); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_21f, 53); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_22f, 49); 
        soc_reg_field_set(unit, CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_23f, 45); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_20_23r(unit,   rval));

        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_24_27r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_28_31r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_32_35r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_36_39r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_40_43r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_44_47r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_48_51r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_52_55r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_56_59r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_PORT_ORDER_REMAP_60_63r(unit, 0x0));

        /* PML1 ports chain is connected to LED2, 24 ports */  
        /* Phisical ports status chain to LED1 is following, port36 resides in port0 place:*/        
        /* 129->133->137->141->113->117->121->125->97->101->105->109->89->90->91->92->85->86->87->88->93->94->95->96 */
        /* index in phisical LED DRAM memory port should reside (reduce 84 from phisical port number): */        
        /*  45-> 49-> 53-> 57-> 29-> 33-> 37-> 41->13-> 17-> 21-> 25-> 5-> 6-> 7-> 8-> 1-> 2-> 3-> 4-> 9->10->11->12 */

        
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_0f, 12); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_1f, 11); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_2f, 10); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r, &rval, REMAP_PORT_3f,  9); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_0_3r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_4f, 4); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_5f, 3); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_6f, 2); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r, &rval, REMAP_PORT_7f, 1); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_4_7r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_8f,  8); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_9f,  7); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_10f, 6); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r, &rval, REMAP_PORT_11f, 5); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_8_11r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_12f, 25); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_13f, 21); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_14f, 17); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r, &rval, REMAP_PORT_15f, 13); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_12_15r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_16f, 41); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_17f, 37); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_18f, 33); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r, &rval, REMAP_PORT_19f, 29); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_16_19r(unit,   rval));

        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_20f, 57); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_21f, 53); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_22f, 49); 
        soc_reg_field_set(unit, CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r, &rval, REMAP_PORT_23f, 45); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_20_23r(unit,   rval));

        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_24_27r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_28_31r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_32_35r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_36_39r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_40_43r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_44_47r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_48_51r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_52_55r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_56_59r(unit, 0x0));
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_PORT_ORDER_REMAP_60_63r(unit, 0x0));

        /* setting Jericho ports status scan values */  
        /*               bits 9-4 (scan delay); bits 3-1 (scan port delay); bit 0 (enable) */    
        /* processor 0 :        32                              5               0          */
        /* processor 1 :        40                              5               0          */
        /* processor 2 :        40                              5               0          */
   
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CTRLr(unit, 0x20a));  
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_CTRLr(unit, 0x28a));  
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_CTRLr(unit, 0x28a));  


        /* Required led clock period is 200 ns and switch clock runs at 720MHz.  */    
        /* value = Clk freq * (period/2) = (720 * 10^6)*(100 * 10^-9) = 72       */
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_CLK_DIVr, &rval, LEDCLK_HALF_PERIODf, 72); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CLK_DIVr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_CLK_DIVr, &rval, LEDCLK_HALF_PERIODf, 72); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_CLK_DIVr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_CLK_DIVr, &rval, LEDCLK_HALF_PERIODf, 72); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_CLK_DIVr(unit, rval)); 


        /* Required refresh period is 30 ms and switch clock period is ~1.388ns(720MHz) */  
        /* Then the value should be: (30*10^-3)/(1.388...*10^-9) = 21600000 = 0x1499700 Timeout value in switch clocks */
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP0_CLK_PARAMSr, &rval, REFRESH_CYCLE_PERIODf, 0x1499700); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CLK_PARAMSr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP1_CLK_PARAMSr, &rval, REFRESH_CYCLE_PERIODf, 0x1499700); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP1_CLK_PARAMSr(unit, rval)); 
        rval = 0;
        soc_reg_field_set(unit, CMIC_LEDUP2_CLK_PARAMSr, &rval, REFRESH_CYCLE_PERIODf, 0x1499700); 
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_LEDUP2_CLK_PARAMSr(unit, rval)); 
         
    }

    /* Set SBUS timeout */
    SOCDNX_IF_ERR_EXIT(soc_arad_core_frequency_config_get(unit, SOC_JER_CORE_FREQ_KHZ_DEFAULT, &core_freq));
    SOCDNX_IF_ERR_EXIT(soc_arad_schan_timeout_config_get(unit, &schan_timeout));
    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_sbus_timeout_set(unit, core_freq, schan_timeout));

    /* Mdio - internal*/

    /*Dividend values*/

    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, -1); 
    if (dividend == -1) 
    {
        /*default value*/
        dividend =  SOC_DPP_IMP_DEFS_GET(unit, mdio_int_dividend_default);

    }

    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, -1); 
    if (divisor == -1) 
    {
        /*Calc default dividend and divisor*/
        mdio_int_freq = SOC_DPP_IMP_DEFS_GET(unit, mdio_int_freq_default);
        divisor = core_freq * dividend / (2* mdio_int_freq);

    }

    mdio_delay = SOC_DPP_IMP_DEFS_GET(unit, mdio_int_out_delay_default);

    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_mdio_config(unit,dividend,divisor,mdio_delay));

    /* Clear SCHAN_ERR */
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CMC0_SCHAN_ERRr(unit, 0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CMC1_SCHAN_ERRr(unit, 0));
    SOCDNX_IF_ERR_EXIT(WRITE_CMIC_CMC2_SCHAN_ERRr(unit, 0));

    /* MDIO configuration */
    SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_mdio_set(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_init_reset(
    int unit,
    int reset_action)
{
    int disable_hard_reset = 0x0;

    SOCDNX_INIT_FUNC_DEFS;

    /* Configure PAXB, enabling the access of CMIC */
    SOCDNX_IF_ERR_EXIT(soc_dcmn_iproc_config_paxb(unit));

    /* Arad CPS Reset */
    disable_hard_reset = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "init_without_device_hard_reset", 0);
    if (disable_hard_reset == 0) {
        SOCDNX_IF_ERR_EXIT(soc_dcmn_cmic_device_hard_reset(unit, reset_action));
    }

    SOCDNX_IF_ERR_EXIT(soc_dcmn_iproc_config_paxb(unit));

    /* Config Endianess */
    soc_endian_config(unit);
    soc_pci_ep_config(unit, 0);

    /* Config Default/Basic cmic registers */
    if (soc_feature(unit, soc_feature_cmicm)) {
        SOCDNX_IF_ERR_EXIT(soc_jer_init_reset_cmic_regs(unit));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_rcpu_base_q_pair_init(int unit, int port_i)
{
    uint32 base_q_pair = 0, rval = 0;
    soc_error_t rv;
    
    SOCDNX_INIT_FUNC_DEFS;
    
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.base_q_pair.get(unit, port_i, &base_q_pair);
    SOCDNX_IF_ERR_EXIT(rv);

    if (base_q_pair < 32) 
    {
        SOCDNX_IF_ERR_EXIT(READ_CMIC_PKT_PORTS_0r(unit, &rval));
        rval |= 0x1 << base_q_pair;
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_0r(unit, rval));
    } else if (base_q_pair < 64) 
    {
        SOCDNX_IF_ERR_EXIT(READ_CMIC_PKT_PORTS_1r(unit, &rval));
        rval |= 0x1 << (base_q_pair - 32);
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_1r(unit, rval));
    } else if (base_q_pair < 96) 
    {
        SOCDNX_IF_ERR_EXIT(READ_CMIC_PKT_PORTS_2r(unit, &rval));
        rval |= 0x1 << (base_q_pair - 64);
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_2r(unit, rval));
    } else if (base_q_pair == 96) 
    {
        rval = 0x1;
        SOCDNX_IF_ERR_EXIT(WRITE_CMIC_PKT_PORTS_3r(unit, rval));
    } else 
    {
        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error: RCPU base_q_pair range is 0 - 96\n")) );
        SOCDNX_IF_ERR_EXIT(SOC_E_INTERNAL);
    }              
    exit:
         SOCDNX_FUNC_RETURN;
}

int soc_jer_rcpu_init(int unit, soc_dpp_config_t *dpp)
{
    int port_i = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_ITER(dpp->arad->init.rcpu.slave_port_pbmp, port_i) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_rcpu_base_q_pair_init(unit, port_i));
    }

    exit:
         SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_str_prop_ingress_congestion_management_guarantee_mode_get(int unit, SOC_TMC_ITM_CGM_MGMT_GUARANTEE_MODE *ingress_congestion_management_guarantee_mode)
{
    char *propkey, *propval;
    SOCDNX_INIT_FUNC_DEFS;

    propkey = spn_INGRESS_CONGESTION_MANAGEMENT;
    propval = soc_property_suffix_num_str_get(unit, -1, propkey, "guarantee_mode");

    if (propval) {
        if (sal_strcmp(propval, "STRICT") == 0) {
            *ingress_congestion_management_guarantee_mode = SOC_TMC_ITM_CGM_MGMT_GUARANTEE_STRICT;
        } else if (sal_strcmp(propval, "LOOSE") == 0) {
            *ingress_congestion_management_guarantee_mode = SOC_TMC_ITM_CGM_MGMT_GUARANTEE_LOOSE;
        } else {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Unexpected property value (\"%s\") for %s"), propval, propkey));
        }
    } else {
        *ingress_congestion_management_guarantee_mode = SOC_TMC_ITM_CGM_MGMT_GUARANTEE_STRICT;
    }

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * Function:
 *      soc_set_bcm88x7x
 * Purpose:
 *      Special set sequencing for BCM88x7x to fix "bus error" issue.
 */
STATIC void 
soc_set_bcm88x7x(int unit, int core, int value) 
{
    uint32 cfg_ctrl;

    if (core == 0) {
        READ_MHOST_0_CR5_CFG_CTRLr(unit, &cfg_ctrl);
        soc_reg_field_set(unit, MHOST_0_CR5_CFG_CTRLr, &cfg_ctrl,
                          RSVDf, value);
        WRITE_MHOST_0_CR5_CFG_CTRLr(unit, cfg_ctrl);
    } else if (core == 1) { 
        READ_MHOST_1_CR5_CFG_CTRLr(unit, &cfg_ctrl);
        soc_reg_field_set(unit, MHOST_1_CR5_CFG_CTRLr, &cfg_ctrl,
                          RSVDf, value);
        WRITE_MHOST_1_CR5_CFG_CTRLr(unit, cfg_ctrl);
    } else {
        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error: CORE is 0 - 1\n")) );
    }        
}

/*
 * Function:
 *      soc_get_bcm88x7x
 * Purpose:
 *      Special get sequencing for BCM88x7x to fix "bus error" issue.
 */
STATIC void
soc_get_bcm88x7x(int unit, int core, int *value)
{
    uint32 cfg_ctrl;

    if (core == 0) {
        READ_MHOST_0_CR5_CFG_CTRLr(unit, &cfg_ctrl);
        *value = soc_reg_field_get(unit, MHOST_0_CR5_CFG_CTRLr, cfg_ctrl, RSVDf);
    } else if (core == 1) {
        READ_MHOST_1_CR5_CFG_CTRLr(unit, &cfg_ctrl);
        *value = soc_reg_field_get(unit, MHOST_1_CR5_CFG_CTRLr, cfg_ctrl, RSVDf);
    } else {
        LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error: CORE is 0 - 1\n")) );
    }
}

/*
 * Function:
 *      soc_reset_bcm88x7x
 * Purpose:
 *      Special reset sequencing for BCM88x7x to fix "bus error" issue.
 */
STATIC void 
soc_reset_bcm88x7x(int unit, int core) 
{
    soc_set_bcm88x7x(unit, core, 1);
}

/*
 * Function:
 *      soc_restore_bcm88x7x
 * Purpose:
 *      Special restore sequencing for BCM88x7x to fix "bus error" issue.
 */
void 
soc_restore_bcm88x7x(int unit, int core) 
{
    soc_set_bcm88x7x(unit, core, 0);
}

/* TCAM Parity machine allows to detect SER events without CPU intervention */
int
soc_jer_tcam_parity_machine_enable(int unit)
{
    uint64 reg_val;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(READ_PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr(unit, &reg_val));
    soc_reg64_field32_set(unit, PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr, &reg_val, TCAM_PARITY_BANK_BITMAPf, 0xFFFF);
    SOCDNX_IF_ERR_EXIT(WRITE_PPDB_A_TCAM_PARITY_MACHINE_CONFIGURATIONr(unit, reg_val));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_getstatus_bcm88x7x
 * Purpose:
 *      Special restore sequencing for BCM88x7x to fix "bus error" issue.
 */
int
soc_getstatus_bcm88x7x(int unit, int core)
{
    int value = 0;
    soc_get_bcm88x7x(unit, core, &value);

    return value;
}

/*
 * Function:
 *      soc_dpp_jericho_init
 * Purpose:
 *      Optionally reset, and initialize the Device.
 * Parameters:
 *      unit -  unit number
 *      reset_action - Integer, indicates the reset action.
 * Returns:
 *      SOC_E_XXX
 *      This routine may be called after a device is attached
 *      or whenever a chip reset is required.
 *
 */
int soc_dpp_jericho_init(
    int unit,
    int reset_action)
{

    soc_dpp_config_t *dpp = NULL;
    int  schan_intr_enb;
    uint32 bist_enable;

#ifdef JERICHO_HW_IMPLEMENTATION
    int silent = 0;
    int port_i, tm_port;
    int header_type_in;
    uint32 base_q_pair, rval;
#endif                          /* JERICHO_HW_IMPLEMENTATION */

    SOCDNX_INIT_FUNC_DEFS;
    DCMN_RUNTIME_DEBUG_PRINT_LOC(unit, "entering soc_dpp_jericho_init");

    if (SOC_CONTROL(unit) == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INIT, (_BSL_SOCDNX_MSG("SOC_CONTROL() is not allocated.")));
    }

    if (!SOC_IS_JERICHO(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_UNAVAIL, (_BSL_SOCDNX_MSG("Jericho function. Invalid Device")));
    }

    dpp = SOC_DPP_CONFIG(unit);
    if ((dpp == NULL) || (dpp->arad == NULL) || (dpp->jer == NULL)) {         
        SOCDNX_EXIT_WITH_ERR(SOC_E_INIT, (_BSL_SOCDNX_MSG("Soc control structures are not allocated.")));
    }

#ifdef BCM_WARM_BOOT_SUPPORT
    soc_arad_init_empty_scache(unit);
#endif
    /* WA for "bus error" issue that SDK configure the RSVD field of 
     * MHOST_0_CR5_CFG_CTRL, then uKernel polling this register field to kill 
     * or stop the thread. 
     */
/* The sockets of the c model are not open in this stage */
#ifndef CMODEL_SERVER_MODE
    soc_reset_bcm88x7x(unit, 0);
#endif
    sal_usleep(50000); /* sleep for 50ms */
    soc_uc_reset(unit, 0);

    SOCDNX_IF_ERR_EXIT(soc_dpp_info_config_ports(unit));

    /* Set default configuration */
    DCMN_RUNTIME_DEBUG_PRINT(unit);
    LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Read SOC property Configuration\n"),unit));
    SOCDNX_IF_ERR_EXIT(soc_dpp_get_default_config_jer(unit));

    SOCDNX_IF_ERR_EXIT(soc_dpp_device_specific_info_config_direct(unit));

    SOCDNX_IF_ERR_EXIT(soc_arad_info_config(unit));

    SOCDNX_IF_ERR_EXIT(soc_dpp_device_specific_info_config_derived(unit));

    /*
     * Reset device.
     * Also enable device access, set default Iproc/CmicD configuration
     * No access allowed before this stage.
     */
    if (!SOC_WARM_BOOT(unit)) {
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        DISPLAY_MEM ;
        DISPLAY_SW_STATE_MEM ;

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Device Reset and Access Enable\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_init_reset(unit, reset_action));
        if (reset_action == SOC_DPP_RESET_ACTION_IN_RESET) {
            SOC_EXIT;
        }
    }
    /* mask all interrupts in cmic (even in WB mode). This Masking update WB DB, althoght we dont use WB DB. */ 
    soc_jer_cmic_interrupts_disable(unit);

    /* save schan interrupt mode and move to schan polling mode
     * because we dont want acceess to fail in interrupt mode 
     * before we connect the interrupt mode callbacks 
     */
    schan_intr_enb = SOC_CONTROL(unit)->schanIntrEnb;
    SOC_CONTROL(unit)->schanIntrEnb = 0;
    /* ECI Access check/test */
    if (!SOC_WARM_BOOT(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_jer_regs_eci_access_check(unit));
    }

    /* Set device blocks in and out of reset */
    if (!SOC_WARM_BOOT(unit)) {
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        DISPLAY_MEM ;
        DISPLAY_SW_STATE_MEM ;

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Blocks OOR and PLL configuration\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_device_reset(unit, SOC_DPP_RESET_MODE_BLOCKS_RESET, SOC_DPP_RESET_ACTION_INOUT_RESET));

        DISPLAY_SW_STATE_MEM ;
        DISPLAY_SW_STATE_MEM_PRINTF(("%s(): Just out of %s\r\n",__FUNCTION__,"soc_jer_device_reset")) ;
    }

    bist_enable = soc_property_get(unit, spn_BIST_ENABLE, 0);
    if (!bist_enable && SOC_DPP_CONFIG(unit)->tm.various_bm & DPP_VARIOUS_BM_FORCE_MBIST_TEST) {
        bist_enable = 1;
    }

    /* For j+ we perform mbist earlier due to
    * a problem with mbist script and dram tune 
    * we need to rerun hw reset again for that purpose 
    */
    if (!SOC_WARM_BOOT(unit)) {
#ifdef PLISIM
       if (!SAL_BOOT_PLISIM) 
#endif
           {
               if (SOC_IS_JERICHO_PLUS_ONLY(unit) || SOC_IS_QUX(unit)) {

                   /* perform MBIST if configured to do so */
                   if (bist_enable) {
                       LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: running internal memories BIST\n"),unit));
                       SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_soc_bist_all,(unit, (bist_enable == 2 ? 1 : 0))));

                       LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Device Reset and Access Enable\n"),unit));
                       SOCDNX_IF_ERR_EXIT(soc_jer_init_reset(unit, reset_action));

                       /* ECI Access check/test */
                       SOCDNX_IF_ERR_EXIT(soc_jer_regs_eci_access_check(unit));
                       LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Blocks OOR and PLL configuration\n"),unit));
                       SOCDNX_IF_ERR_EXIT(soc_jer_device_reset(unit, SOC_DPP_RESET_MODE_BLOCKS_RESET, SOC_DPP_RESET_ACTION_INOUT_RESET));

                   }           
               }


           }
       }

       /* Init interrupts */
       SOCDNX_IF_ERR_EXIT(soc_jer_interrupts_init(unit));

       /** 
        *  recover the cmic interrupt mode after we initiate
        *  the callbacks
       */
       SOC_CONTROL(unit)->schanIntrEnb = schan_intr_enb;
       #ifdef CMODEL_SERVER_MODE
       cmodel_reg_access_init(unit);
       #endif


      /* Cahce Enable*/
       SOCDNX_IF_ERR_EXIT(soc_dpp_cache_enable_init(unit));
 
       if (!SOC_WARM_BOOT(unit)) {

           SOCDNX_IF_ERR_EXIT(soc_jer_interrupts_disable(unit));
       }


#ifndef CMODEL_SERVER_MODE
    /* Attach/Init DMA */
    SOCDNX_IF_ERR_EXIT(soc_jer_init_dma_init(unit));
#endif /* CMODEL_SERVER_MODE */

    /* Common device init */
    SOCDNX_IF_ERR_EXIT(soc_dpp_common_init(unit));



    if (!SOC_WARM_BOOT(unit)) {
        /* Stop all traffic and Control Cells */
        DCMN_RUNTIME_DEBUG_PRINT(unit);
        SOCDNX_SAND_IF_ERR_EXIT(jer_mgmt_enable_traffic_set(unit, FALSE));

        DISPLAY_MEM ;
        DISPLAY_SW_STATE_MEM ;

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: Traffic Disable\n"),unit));
        SOCDNX_SAND_IF_ERR_EXIT(arad_mgmt_all_ctrl_cells_enable_set(unit, FALSE));
    }

/*#ifdef JERICHO_HW_IMPLEMENTATION*/
    /*
     * Initialize SOC link control module
     */
    SOCDNX_IF_ERR_RETURN(soc_linkctrl_init(unit, &soc_linkctrl_driver_jer));
/*#endif                 */         /* JERICHO_HW_IMPLEMENTATION */

    if (SOC_WARM_BOOT(unit)) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_mcds_multicast_init2, (unit)));
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_str_prop_ingress_congestion_management_guarantee_mode_get(unit, &dpp->jer->tm.cgm_mgmt_guarantee_mode));

    if (SOC_IS_QAX(unit)) {
        dpp->qax->per_packet_compensation_legacy_mode = 
            (soc_property_get(unit, spn_INGRESS_CONGESTION_MANAGEMENT_PKT_HEADER_COMPENSATION_ENABLE, 0) == 0);
    }

    if (!SOC_WARM_BOOT(unit)) {
        arad_sw_db_is_petrab_in_system_set(unit, SOC_DPP_CONFIG(unit)->tm.is_petrab_in_system);
    }


    /* Init phase 1 */
    SOCDNX_SAND_IF_ERR_EXIT(soc_jer_init_sequence_phase1(unit));


    if (!SOC_WARM_BOOT(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_dpp_common_tm_init(unit, &(dpp->tm.multicast_egress_bitmap_group_range)));
    }
    
    /*
     * pp init
     */


    if (dpp->arad->init.pp_enable) {
        DCMN_RUNTIME_DEBUG_PRINT(unit);

        DISPLAY_SW_STATE_MEM ;

        LOG_INFO(BSL_LS_SOC_INIT, (BSL_META_U(unit, "\t+ %d: PP Initialization\n"),unit));
        SOCDNX_IF_ERR_EXIT(soc_arad_pp_init(unit));

        DISPLAY_SW_STATE_MEM ;
        DISPLAY_SW_STATE_MEM_PRINTF(("%s(): Just out of %s\r\n",__FUNCTION__,"soc_arad_pp_init")) ;
    }

    /* no need to reinit TDM during warm reboot. */
    if (!SOC_WARM_BOOT(unit)) 
    {
        /* Init RCPU */
        SOCDNX_IF_ERR_EXIT(soc_jer_rcpu_init(unit, dpp));

        SOCDNX_IF_ERR_EXIT(soc_dcmn_ser_init(unit));

        if (bist_enable) {
            SOCDNX_IF_ERR_EXIT(soc_jer_tcam_parity_machine_enable(unit));
        }
    }
    arad_fast_regs_and_fields_access_init(unit);

exit:

    DISPLAY_MEM ;
    DISPLAY_SW_STATE_MEM ;
    DISPLAY_MEM_PRINTF(("%s(): unit %d: Exit\r\n",__FUNCTION__,unit)) ;

    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_dpp_jericho_deinit
 * Purpose:
 *      De-initialize the Device.
 * Parameters:
 *      unit -  unit number
 * Returns:
 *      SOC_E_XXX
 *      This routine may be called after a soc_dpp_jericho_init()
 *
 */
int soc_dpp_jericho_deinit(
    int unit)
{
    SOCDNX_INIT_FUNC_DEFS;

#ifndef CMODEL_SERVER_MODE
    /* Detach/De-init interrupts */
    SOCDNX_IF_ERR_EXIT(soc_jer_interrupts_deinit(unit));
#endif /* CMODEL_SERVER_MODE */

    /* Detach/De-init tables */
    SOCDNX_IF_ERR_EXIT(soc_jer_tbls_deinit(unit));

#ifndef CMODEL_SERVER_MODE
    /* Detach/De-init DMA */
    SOCDNX_IF_ERR_EXIT(soc_jer_deinit_dma(unit));
#endif /* CMODEL_SERVER_MODE */

exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_info_config_custom_reg_access(int unit)
{
    soc_custom_reg_access_t* reg_access;
    SOCDNX_INIT_FUNC_DEFS;

    reg_access = &(SOC_INFO(unit).custom_reg_access);

    reg_access->custom_reg32_get = soc_jer_ilkn_reg32_get;
    reg_access->custom_reg32_set = soc_jer_ilkn_reg32_set;

    reg_access->custom_reg64_get = soc_jer_ilkn_reg64_get;
    reg_access->custom_reg64_set = soc_jer_ilkn_reg64_set;

    reg_access->custom_reg_above64_get = soc_jer_ilkn_reg_above_64_get;
    reg_access->custom_reg_above64_set = soc_jer_ilkn_reg_above_64_set;

    BCM_PBMP_ASSIGN(reg_access->custom_port_pbmp, PBMP_IL_ALL(unit)); 
   
    SOCDNX_FUNC_RETURN;
}

/*
* Function to get the current configuration of the PLLs in the device.  
*/
int soc_jer_pll_info_get(int unit, soc_dpp_pll_info_t *pll_info)
{
    int rv;
    uint32 output_buffer=0;
    uint32 field;
    soc_reg_above_64_val_t reg_val_long;
    soc_dpp_pll_t *pll_val=NULL;

    SOCDNX_INIT_FUNC_DEFS;

    sal_memset(pll_info, 0, sizeof(soc_dpp_pll_info_t));

    /* Core PLL */
    pll_val=&(pll_info->core_pll);
    pll_val->ref_clk = 25; /* Has to be hardcoded */
    pll_val->p_div   = 1;  /* Pre divider, hardcoder */
    SOCDNX_IF_ERR_EXIT(READ_ECI_POWERUP_CONFIGr_REG32(unit, &reg_val_long[0]));

    /* Feedback divider */
    soc_sand_bitstream_get_any_field(&reg_val_long[0], 0, 8, &output_buffer);
    pll_val->n_div=output_buffer;
    output_buffer=0;
    if (pll_val->n_div==0) {
        pll_val->n_div=1024;
    }

    /* M0 divider determines the frequency on Channel 0 */
    rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 8, 4, &output_buffer);
    SOCDNX_SAND_IF_ERR_EXIT(rv);
    pll_val->m0_div=output_buffer;
    output_buffer=0;
    if (pll_val->m0_div==0) {
        pll_val->m0_div=256;
    }

    if (SOC_IS_QAX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_ECI_OGER_1028r(unit, reg_val_long));
        /* M1 divider determines the frequency on Channel 1 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 8, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m1_div=output_buffer;
        output_buffer=0;

        /* M4 divider determines the frequency on Channel 4 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 16, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m4_div=output_buffer;
        output_buffer=0;

        /* M5 divider determines the frequency on Channel 5 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 24, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m5_div=output_buffer;
        output_buffer=0;
    }

    /* Voltage Control Oscilator */
    pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

    /* Frequency on Channel 0*/
    pll_val->ch0=pll_val->vco/pll_val->m0_div;

    /* Frequency on Channel 1*/
    if (pll_val->m1_div) {
        pll_val->ch1=pll_val->vco/pll_val->m1_div;
    } else {
        pll_val->ch1=0;
    }

    /* Frequency on Channel 4*/
    if (pll_val->m4_div) {
        pll_val->ch4=pll_val->vco/pll_val->m4_div;
    } else {
        pll_val->ch4=0;
    }

    /* Frequency on Channel 5*/
    if (pll_val->m5_div) {
        pll_val->ch5=pll_val->vco/pll_val->m5_div;
    } else {
        pll_val->ch5=0;
    }

    /* Core PLL lock */
    reg_val_long[0]=0;
    SOCDNX_IF_ERR_EXIT(READ_ECI_CORE_PLL_STATUSr(unit, &reg_val_long[0]));
    pll_val->locked=soc_reg_field_get(unit, ECI_CORE_PLL_STATUSr, reg_val_long[0], CORE_PLL_LOCKEDf);

    /* UC PLL */
    pll_val=&(pll_info->uc_pll);
    pll_val->ref_clk = 25; /* Has to be hardcoded */
    if (SOC_IS_QAX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_ECI_OGER_1027r(unit, reg_val_long));
        /* Pre divider*/
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[1], 26, 3, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->p_div=output_buffer;
        output_buffer=0;
        if (pll_val->p_div==0) {
            pll_val->p_div=1;
        }

        /* Feedback divider */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[2], 3, 10, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->n_div=output_buffer;
        output_buffer=0;
        if (pll_val->n_div==0) {
            pll_val->n_div=140;
        }

        /* M0 divider determines the frequency on Channel 0 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 0, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m0_div=output_buffer;
        output_buffer=0;
        if (pll_val->m0_div==0) {
            pll_val->m0_div=28;
        }

        /* M1 divider determines the frequency on Channel 1 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 8, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m1_div=output_buffer;
        output_buffer=0;

        /* M4 divider determines the frequency on Channel 4 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 0, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m4_div=output_buffer;
        output_buffer=0;

        /* M5 divider determines the frequency on Channel 5 */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 24, 8, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->m5_div=output_buffer;
        output_buffer=0;


    } else {
        /* Hardcoded for Jericho */
        pll_val->p_div   = 1; /* Pre divider. hardcoded  */
        pll_val->n_div   = 120; /* Feedback devider. hardcoded */
        pll_val->m0_div  = 24; /* hardcoded */
        pll_val->m4_div  = 12; /* hardcoded */
        pll_val->m5_div  = 6; /* hardcoded */
    }


    /*Voltage Control Oscilator*/
    pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

    /* Frequency on Channel 0*/
    pll_val->ch0=pll_val->vco/pll_val->m0_div;

    /* Frequency on Channel 1*/
    if (pll_val->m1_div) {
        pll_val->ch1=pll_val->vco/pll_val->m1_div;
    } else {
        pll_val->ch1=0;
    }

    /* Frequency on Channel 4*/
    if (pll_val->m4_div) {
        pll_val->ch4=pll_val->vco/pll_val->m4_div;
    } else {
        pll_val->ch4=0;
    }

    /* Frequency on Channel 5*/
    if (pll_val->m5_div) {
        pll_val->ch5=pll_val->vco/pll_val->m5_div;
    } else {
        pll_val->ch5=0;
    }

    /* UC PLL lock */
    reg_val_long[0]=0;
    SOCDNX_IF_ERR_EXIT(READ_ECI_UC_PLL_STATUSr(unit, &reg_val_long[0]));
    pll_val->locked=soc_reg_field_get(unit, ECI_UC_PLL_STATUSr, reg_val_long[0], UC_PLL_LOCKEDf);

     /* TS PLL */
    pll_val=&(pll_info->ts_pll);
    pll_val->ref_clk = 25; /* Has to be hardcoded */
    SOCDNX_IF_ERR_EXIT(READ_ECI_TS_PLL_CONFIGr(unit, reg_val_long));
    /* Pre divider*/
    pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_TS_PLL_CONFIGr, reg_val_long, TS_PLL_CFG_PDIVf);
    if (pll_val->p_div==0) {
        pll_val->p_div=8;
    }

    /* Feedback divider */
    pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_TS_PLL_CONFIGr, reg_val_long, TS_PLL_CFG_NDIVf);
    if (pll_val->n_div==0) {
        pll_val->n_div=1024;
    }

    /* M0 divider determines the frequency on Channel 0 */
    pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_TS_PLL_CONFIGr, reg_val_long, TS_PLL_CFG_CH_0_MDIVf);
    if (pll_val->m0_div==0) {
        pll_val->m0_div=256;
    }

    /* Voltage Control Oscilator */
    pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

    /* Frequency on Channel 0*/
    pll_val->ch0=pll_val->vco/pll_val->m0_div;

    /* TS PLL lock */
    reg_val_long[0]=0;
    SOCDNX_IF_ERR_EXIT(READ_ECI_TS_PLL_STATUSr(unit, &reg_val_long[0]));
    pll_val->locked=soc_reg_field_get(unit, ECI_TS_PLL_STATUSr, reg_val_long[0], MISC_PLL_0_LOCKEDf);

    if (SOC_IS_QAX(unit)) {
        /* BS0 PLL */
        pll_val=&(pll_info->bs_pll[0]);
        pll_val->ref_clk = 25; /* Has to be hardcoded */
        SOCDNX_IF_ERR_EXIT(READ_ECI_BS_0_PLL_CONFIGr(unit, reg_val_long));
        /* Pre divider*/
        pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_BS_0_PLL_CONFIGr, reg_val_long, BS_0_PLL_PDIVf);
        if (pll_val->p_div==0) {
            pll_val->p_div=1;
        }   

        /* Feedback divider */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 0, 10, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->n_div=output_buffer;
        output_buffer=0;
        if (pll_val->n_div==0) {
            pll_val->n_div=120;
        }

        /* M0 divider determines the frequency on Channel 0 */
        pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_BS_0_PLL_CONFIGr, reg_val_long, BS_0_PLL_CH_0_MDIVf);
        if (pll_val->m0_div==0) {
            pll_val->m0_div=120;
        }

        /* Voltage Control Oscilator */
        pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

        /* Frequency on Channel 0*/
        pll_val->ch0=pll_val->vco/pll_val->m0_div;

        /* BS0 PLL lock */
        reg_val_long[0]=0;
        SOCDNX_IF_ERR_EXIT(READ_ECI_BS_0_PLL_STATUSr(unit, &reg_val_long[0]));
        pll_val->locked=soc_reg_field_get(unit, ECI_BS_0_PLL_STATUSr, reg_val_long[0], MISC_PLL_1_LOCKEDf);

        /* BS1 PLL */
        pll_val=&(pll_info->bs_pll[1]);
        pll_val->ref_clk = 25; /* Has to be hardcoded */
        SOCDNX_IF_ERR_EXIT(READ_ECI_BS_1_PLL_CONFIGr(unit, reg_val_long));
        /* Pre divider*/
        pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_BS_1_PLL_CONFIGr, reg_val_long, BS_1_PLL_PDIVf);
        if (pll_val->p_div==0) {
            pll_val->p_div=1;
        }   

        /* Feedback divider */
        rv=soc_sand_bitstream_get_any_field(&reg_val_long[0], 0, 10, &output_buffer);
        SOCDNX_SAND_IF_ERR_EXIT(rv);
        pll_val->n_div=output_buffer;
        output_buffer=0;
        if (pll_val->n_div==0) {
            pll_val->n_div=120;
        }

        /* M0 divider determines the frequency on Channel 0 */        
        pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_BS_1_PLL_CONFIGr, reg_val_long, BS_1_PLL_CH_0_MDIVf);
        if (pll_val->m0_div==0) {
            pll_val->m0_div=120;
        }

        /* Voltage Control Oscilator */
        pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

        /* Frequency on Channel 0*/
        pll_val->ch0=pll_val->vco/pll_val->m0_div;

        /* BS1 PLL lock */
        reg_val_long[0]=0;
        SOCDNX_IF_ERR_EXIT(READ_ECI_BS_1_PLL_STATUSr(unit, &reg_val_long[0]));
        pll_val->locked=soc_reg_field_get(unit, ECI_BS_1_PLL_STATUSr, reg_val_long[0], MISC_PLL_2_LOCKEDf);

    } else {
        /* BS PLL */
        pll_val=&(pll_info->bs_pll[0]);
        pll_val->ref_clk = 25; /* Has to be hardcoded */
        SOCDNX_IF_ERR_EXIT(READ_ECI_BS_PLL_CONFIGr(unit, reg_val_long));
        /* Pre divider*/
        pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_BS_PLL_CONFIGr, reg_val_long, BS_PLL_CFG_PDIVf);
        if (pll_val->p_div==0) {
            pll_val->p_div=8;
        }   

        /* Feedback divider */
        pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_BS_PLL_CONFIGr, reg_val_long, BS_PLL_CFG_NDIVf);
        if (pll_val->n_div==0) {
            pll_val->n_div=1024;
        }

        /* M0 divider determines the frequency on Channel 0 */
        pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_BS_PLL_CONFIGr, reg_val_long, BS_PLL_CFG_CH_0_MDIVf);
        if (pll_val->m0_div==0) {
            pll_val->m0_div=256;
        }

        /* Voltage Control Oscilator */
        pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

        /* Frequency on Channel 0*/
        pll_val->ch0=pll_val->vco/pll_val->m0_div;

        /* BS PLL lock */
        reg_val_long[0]=0;
        SOCDNX_IF_ERR_EXIT(READ_ECI_BS_PLL_STATUSr(unit, &reg_val_long[0]));
        pll_val->locked=soc_reg_field_get(unit, ECI_BS_PLL_STATUSr, reg_val_long[0], MISC_PLL_1_LOCKEDf);
    }       

    /* PML0 PLL */
    pll_val=&(pll_info->pml_pll[0]);
    pll_val->ref_clk = 156.25;
    SOCDNX_IF_ERR_EXIT(READ_ECI_NIF_PML_0_PLL_CONFIGr(unit, reg_val_long));
    if (SOC_IS_QAX(unit)) {
        field=PML_0_PLL_PDIVf;
    } else {
        field=NIF_PML_0_PLL_CFG_PDIVf;
    }
    /* Pre divider*/
    pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->p_div==0) {
        pll_val->p_div=8;
    }   

    /* Feedback divider */
    if (SOC_IS_QAX(unit)) {
        field=PML_0_PLL_FBDIV_NDIV_INTf;
    } else {
        field=NIF_PML_0_PLL_CFG_NDIVf;
    }
    pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->n_div==0) {
        pll_val->n_div=1024;
    }

    /* M0 divider determines the frequency on Channel 0 */
    if (SOC_IS_QAX(unit)) {
        field=PML_0_PLL_CH_0_MDIVf;
    } else {
        field=NIF_PML_0_PLL_CFG_CH_0_MDIVf;
    }

    pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->m0_div==0) {
        pll_val->m0_div=256;
    }

    if (SOC_IS_QAX(unit)) {
        pll_val->m4_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val_long, PML_0_PLL_CH_4_MDIVf);
    }

    /* Voltage Control Oscilator */
    pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

    /* Frequency on Channel 0*/
    pll_val->ch0=pll_val->vco/pll_val->m0_div;

    /* Frequency on Channel 4*/
    if (pll_val->m4_div) {
        pll_val->ch4=pll_val->vco/pll_val->m4_div;
    } else {
        pll_val->ch4=0;
    }

    /* PML0 PLL lock */
    reg_val_long[0]=0;
    SOCDNX_IF_ERR_EXIT(READ_ECI_PML_0_PLL_STATUSr(unit, &reg_val_long[0]));
    pll_val->locked=soc_reg_field_get(unit, ECI_PML_0_PLL_STATUSr, reg_val_long[0], MISC_PLL_5_LOCKEDf);

    /* PML1 PLL */
    pll_val=&(pll_info->pml_pll[1]);
    pll_val->ref_clk = 156.25;
    SOCDNX_IF_ERR_EXIT(READ_ECI_NIF_PML_1_PLL_CONFIGr(unit, reg_val_long));
    if (SOC_IS_QAX(unit)) {
        field=PML_1_PLL_PDIVf;
    } else {
        field=NIF_PML_1_PLL_CFG_PDIVf;
    }
    /* Pre divider*/
    pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->p_div==0) {
        pll_val->p_div=8;
    }   

    /* Feedback divider */
    if (SOC_IS_QAX(unit)) {
        field=PML_1_PLL_FBDIV_NDIV_INTf;
    } else {
        field=NIF_PML_1_PLL_CFG_NDIVf;
    }

    pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->n_div==0) {
        pll_val->n_div=1024;
    }

    /* M0 divider determines the frequency on Channel 0 */
    if (SOC_IS_QAX(unit)) {
        field=PML_1_PLL_CH_0_MDIVf;
    } else {
        field=NIF_PML_1_PLL_CFG_CH_0_MDIVf;
    }

    pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->m0_div==0) {
        pll_val->m0_div=256;
    }

    if (SOC_IS_QAX(unit)) {
        pll_val->m4_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val_long, PML_1_PLL_CH_4_MDIVf);
    }

    /* Voltage Control Oscilator */
    pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

    /* Frequency on Channel 0*/
    pll_val->ch0=pll_val->vco/pll_val->m0_div;

    /* Frequency on Channel 4*/
    if (pll_val->m4_div) {
        pll_val->ch4=pll_val->vco/pll_val->m4_div;
    } else {
        pll_val->ch4=0;
    }

    /* PML1 PLL lock */
    reg_val_long[0]=0;
    SOCDNX_IF_ERR_EXIT(READ_ECI_PML_1_PLL_STATUSr(unit, &reg_val_long[0]));
    pll_val->locked=soc_reg_field_get(unit, ECI_PML_1_PLL_STATUSr, reg_val_long[0], MISC_PLL_6_LOCKEDf);

    /* PMH PLL */
    pll_val=&(pll_info->pmh_pll);
    pll_val->ref_clk = 156.25;
    SOCDNX_IF_ERR_EXIT(READ_ECI_NIF_PMH_PLL_CONFIGr(unit, reg_val_long));
    if (SOC_IS_QAX(unit)) {
        field=PMH_PLL_PDIVf;
    } else {
        field=NIF_PMH_PLL_CFG_PDIVf;
    }

    /* Pre divider*/
    pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->p_div==0) {
        pll_val->p_div=8;
    }   

    /* Feedback divider */
    if (SOC_IS_QAX(unit)) {
        field=PMH_PLL_FBDIV_NDIV_INTf;
    } else {
        field=NIF_PMH_PLL_CFG_NDIVf;
    }

    pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->n_div==0) {
        pll_val->n_div=1024;
    }

    /* M0 divider determines the frequency on Channel 0 */
    if (SOC_IS_QAX(unit)) {
        field=PMH_PLL_CH_0_MDIVf;
    } else {
        field=NIF_PMH_PLL_CFG_CH_0_MDIVf;
    }

    pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val_long, field);
    if (pll_val->m0_div==0) {
        pll_val->m0_div=256;
    }

    if (SOC_IS_QAX(unit)) {
        pll_val->m4_div=soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val_long, PMH_PLL_CH_4_MDIVf);
    }

    /* Voltage Control Oscilator */
    pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

    /* Frequency on Channel 0*/
    pll_val->ch0=pll_val->vco/pll_val->m0_div;

    /* Frequency on Channel 4*/
    if (pll_val->m4_div) {
        pll_val->ch4=pll_val->vco/pll_val->m4_div;
    } else {
        pll_val->ch4=0;
    }

    /* PMH PLL lock */
    reg_val_long[0]=0;
    SOCDNX_IF_ERR_EXIT(READ_ECI_PMH_PLL_STATUSr(unit, &reg_val_long[0]));
    pll_val->locked=soc_reg_field_get(unit, ECI_PMH_PLL_STATUSr, reg_val_long[0], MISC_PLL_4_LOCKEDf);

    if (!SOC_IS_QAX(unit)) {
        /* FAB0 PLL */
        pll_val=&(pll_info->fab_pll[0]);
        pll_val->ref_clk = 156.25;
        SOCDNX_IF_ERR_EXIT(READ_ECI_FAB_0_PLL_CONFIGr(unit, reg_val_long));
        /* Pre divider*/
        pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_FAB_0_PLL_CONFIGr, reg_val_long, FAB_0_PLL_CFG_PDIVf);
        if (pll_val->p_div==0) {
            pll_val->p_div=8;
        }   

        /* Feedback divider */
        pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_FAB_0_PLL_CONFIGr, reg_val_long, FAB_0_PLL_CFG_NDIVf);
        if (pll_val->n_div==0) {
            pll_val->n_div=1024; 
        }

        /* M0 divider determines the frequency on Channel 0 */
        pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_FAB_0_PLL_CONFIGr, reg_val_long, FAB_0_PLL_CFG_CH_0_MDIVf);
        if (pll_val->m0_div==0) {
            pll_val->m0_div=256;
        }   

        /* Voltage Control Oscilator */
        pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

        /* Frequency on Channel 0*/
        pll_val->ch0=pll_val->vco/pll_val->m0_div;

        /* FAB0 PLL lock */
        reg_val_long[0]=0;
        SOCDNX_IF_ERR_EXIT(READ_ECI_FAB_0_PLL_STATUSr(unit, &reg_val_long[0]));
        pll_val->locked=soc_reg_field_get(unit, ECI_FAB_0_PLL_STATUSr, reg_val_long[0], MISC_PLL_2_LOCKEDf);

        /* FAB1 PLL */
        pll_val=&(pll_info->fab_pll[1]);
        pll_val->ref_clk = 156.25;
        SOCDNX_IF_ERR_EXIT(READ_ECI_FAB_0_PLL_CONFIGr(unit, reg_val_long));
        /* Pre divider*/
        pll_val->p_div=soc_reg_above_64_field32_get(unit, ECI_FAB_1_PLL_CONFIGr, reg_val_long, FAB_1_PLL_CFG_PDIVf);
        if (pll_val->p_div==0) {
            pll_val->p_div=8;
        }   

        /* Feedback divider */
        pll_val->n_div=soc_reg_above_64_field32_get(unit, ECI_FAB_1_PLL_CONFIGr, reg_val_long, FAB_1_PLL_CFG_NDIVf);
        if (pll_val->n_div==0) {
            pll_val->n_div=1024; 
        }

        /* M0 divider determines the frequency on Channel 0 */
        pll_val->m0_div=soc_reg_above_64_field32_get(unit, ECI_FAB_1_PLL_CONFIGr, reg_val_long, FAB_1_PLL_CFG_CH_0_MDIVf);
        if (pll_val->m0_div==0) {
            pll_val->m0_div=256;
        }   

        /* Voltage Control Oscilator */
        pll_val->vco=pll_val->ref_clk/pll_val->p_div*pll_val->n_div;

        /* Frequency on Channel 0*/
        pll_val->ch0=pll_val->vco/pll_val->m0_div;

        /* FAB1 PLL lock */
        reg_val_long[0]=0;
        SOCDNX_IF_ERR_EXIT(READ_ECI_FAB_1_PLL_STATUSr(unit, &reg_val_long[0]));
        pll_val->locked=soc_reg_field_get(unit, ECI_FAB_1_PLL_STATUSr, reg_val_long[0], MISC_PLL_3_LOCKEDf);
    }


exit:
  SOCDNX_FUNC_RETURN;
}
