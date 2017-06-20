/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File: jer_egr_queuing.c
 */

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_PORT
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/portmod/portmod.h>
#include <soc/portmod/portmod_common.h>
#include <soc/portmod/portmod_chain.h>
#include <soc/portmod/portmod_legacy_phy.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dpp/drv.h>
#include <soc/dcmn/dcmn_port.h>
#include <soc/dcmn/dcmn_dev_feature_manager.h>
#include <soc/dpp/JER/jer_nif.h>
#include <soc/dpp/QAX/qax_nif.h>
#include <soc/dpp/JER/jer_nif_prd.h>
#include <soc/dpp/JER/jer_stat.h>
#include <soc/dpp/JER/jer_ports.h>
#include <soc/dpp/JER/jer_defs.h>
#include <soc/dpp/JER/jer_flow_control.h>
#include <soc/phy/phymod_sim.h>
#include <soc/dpp/port_sw_db.h>
#include <soc/dcmn/error.h>
#include <sal/appl/sal.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/hwstate/hw_log.h>
#endif

#include <phymod/chip/bcmi_tsce_xgxs_defs.h>
#ifndef PHYMOD_EXCLUDE_CHIPLESS_TYPES
#define PHYMOD_EXCLUDE_CHIPLESS_TYPES
#include <phymod/chip/bcmi_tscf_xgxs_defs.h>
#undef PHYMOD_EXCLUDE_CHIPLESS_TYPES
#endif /* PHYMOD_EXCLUDE_CHIPLESS_TYPES */

#define SOC_JERICHO_NOF_LANES_PER_CORE    4
#define SOC_JERICHO_NOF_PMS_PER_NBI       6
#define SOC_JERICHO_NOF_ILKN_WRAP         3
#define SOC_JERICHO_MAX_PMS_PER_ILKN_PORT 6
#define SOC_JERICHO_NUM_PMS_ILKN45        3
#define SOC_JERICHO_NUM_FIRST_PM_ILKN45   3
#define JER_NIF_ILKN_BURST_MAX_VAL        256

#define SOC_JERICHO_SIM_MASK_NBIH   (0 << 5)
#define SOC_JERICHO_SIM_MASK_NBIL0  (1 << 5)
#define SOC_JERICHO_SIM_MASK_NBIL1  (2 << 5)

#define JER_NIF_PHY_SIF_PORT_0_NBIL0 (68)
#define JER_NIF_PHY_SIF_PORT_1_NIBL0_PM4 (52)
#define JER_NIF_PHY_SIF_PORT_1_NBIL0_PM5 (76)

#define JER_NIF_PHY_SIF_PORT_0_NBIL1 (128)
#define JER_NIF_PHY_SIF_PORT_1_NBIL1_PM4 (112)
#define JER_NIF_PHY_SIF_PORT_1_NBIL1_PM5 (136)

#define JER_NIF_PHY_SIF_PORT_0_NBIH_PM5 (20)
#define JER_NIF_PHY_SIF_PORT_1_NBIH_PM5 (22)
#define JER_NIF_PHY_SIF_PORT_1_NBIH_PM4 (16)

#define JER_NIF_PM4x25_WM_HIGH          (14)
#define JER_NIF_PM4x25_WM_LOW           (15)
#define JER_NIF_PM4x10_WM_HIGH          (9)
#define JER_NIF_PM4x10_WM_LOW           (10)

#define SOC_JERICHO_PORT_PHY_ADDR_INVALID 0xffffffff

#define SOC_QMX_NUM_OF_PMLS   2

#define JER_SOC_TSCE_VCO_6_25_PLL_DIV  (40)

#define JER_SOC_PRD_MODE_VLAN   (0)
#define JER_SOC_PRD_MODE_ITMH   (1)
#define JER_SOC_PRD_MODE_HIGIG  (2)

#define JER_SOC_PRD_MAP_BITS_PER_PRIORITY  (2)
#define JER_NIF_PRIO_TDM_LEVEL 2
#define JER_NIF_PRIO_HIGH_LEVEL 1
#define JER_NIF_PRIO_LOW_LEVEL 0

#define SOC_JER_PLUS_PRD_PORT_TYPE_ITMH             (0)
#define SOC_JER_PLUS_PRD_PORT_TYPE_HIGIG            (1)
#define SOC_JER_PLUS_PRD_PORT_TYPE_FTMH             (2)
#define SOC_JER_PLUS_PRD_PORT_TYPE_TDM              (3)
#define SOC_JER_PLUS_PRD_PORT_TYPE_TDM_ETH_HYBRID   (4)
#define SOC_JER_PLUS_PRD_PORT_TYPE_PTCH1            (5)
#define SOC_JER_PLUS_PRD_PORT_TYPE_PTCH2            (6)
#define SOC_JER_PLUS_PRD_PORT_TYPE_ETH              (7)

#define SOC_JER_PLUS_PRD_EN_ILKN_FIRST_TWO_SEGMENTS  0x1000000
#define SOC_JER_PLUS_PRD_EN_ILKN_SECOND_TWO_SEGMENTS 0x2000000

#define SOC_JER_PLUS_PRD_TM_MAP_MAX_VAL     (0x3F)
#define SOC_JER_PLUS_PRD_IP_MAP_MAX_VAL     (0x3F)
#define SOC_JER_PLUS_PRD_ETH_MAP_MAX_VAL    (0xF)
#define SOC_JER_PLUS_PRD_MPLS_MAP_MAX_VAL   (0x7)

#define SOC_JER_PLUS_PRD_THRESHOLD_MAX_VAL  (0x7FF)

#define SOC_JER_PLUS_PRD_TPID_MAX_VAL       (0xFFFF)

#define SOC_JER_NIF_PRD_MAX_KEY_BUILD_OFFSETS  SOC_TMC_PORT_PRD_MAX_KEY_BUILD_OFFSETS

#define SOC_JER_PLUS_PRD_CONFIG_ETHER_CODE_MIN  (1)
#define SOC_JER_PLUS_PRD_CONFIG_ETHER_CODE_MAX  (6)

#define SOC_JER_PLUS_PRD_CONFIG_ETHER_TYPE_MAX_VAL      (0xFFFF)
#define SOC_JER_PLUS_PRD_CTRL_FRAME_ETH_CODE_MAX_VAL    (0xF)
#define SOC_JER_PLUS_PRD_KEY_ENTRY_MAX_VAL              (0x1F)
#define SOC_JER_PLUS_PRD_KEY_FIELD_MAX_VAL              (0xFF)
#define SOC_JER_PLUS_PRD_KEY_BUILD_OFFSET_MAX_VAL       (0x3F)

#define SOC_JER_NIF_QUEUE_LEVEL_FLOW_CTRL_NOF_CHANNELS  (16)

#define SOC_JER_PLUS_PRD_CTRL_FRAME_MAC_DA_MAX_VAL(max_mac_da)  \
        (COMPILER_64_SET(max_mac_da, 0xFFFF, 0xFFFFFFFF))

#define SOC_JER_NIF_XPHY_USER_ACCESS(unit, port) \
        xphy_user_acc[(unit * SOC_MAX_NUM_PORTS) + port]

typedef enum
{
  SOC_JER_TX_EGRESS_CREDITS_CMD_FLUSH,
  SOC_JER_TX_EGRESS_CREDITS_CMD_STOP,

  /* Total number of operation types. */
  SOC_JER_TX_EGRESS_CREDITS_CMD_COUNT
} SOC_JER_TX_EGRESS_CREDITS_CMD;

STATIC void* pm4x25_user_acc[SOC_MAX_NUM_DEVICES]= {NULL};
STATIC void* pm4x10_user_acc[SOC_MAX_NUM_DEVICES]= {NULL};
STATIC void* pm4x10q_user_acc[SOC_MAX_NUM_DEVICES]= {NULL};
STATIC void *fabric_user_acc[SOC_MAX_NUM_DEVICES] = {NULL};
STATIC void* xphy_user_acc[SOC_MAX_NUM_DEVICES * SOC_MAX_NUM_PORTS] = {NULL};
#ifdef PORTMOD_PM4X2P5_SUPPORT
STATIC void* pm4x2p5_user_acc[SOC_MAX_NUM_DEVICES]= {NULL};
#endif /* PORTMOD_PM4X2P5_SUPPORT */

/*STATIC function declaration*/
STATIC int soc_jer_portmod_check_for_qmlf_conflict(int unit, int new_port);
STATIC int soc_jer_port_nbi_ports_rstn(int unit, soc_port_t port, int enable);
STATIC soc_port_if_t soc_jer_get_interface_type(int unit, soc_port_t port, uint32 defl);
STATIC int soc_jer_port_eqg_nif_credit_init(int unit, soc_port_t port);
STATIC int soc_jer_port_nbi_tx_egress_credits_set(int unit, soc_port_t port, SOC_JER_TX_EGRESS_CREDITS_CMD cmd, int enable);
STATIC int soc_jer_portmod_ilkn_speed_validate(int unit, soc_port_t port);
STATIC int soc_jer_phy_mdio_c22_reg_read( void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t *val);
STATIC int soc_jer_phy_mdio_c22_reg_write( void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t val);
STATIC int soc_jer_ilkn_serdes_qmlfs_get( int unit, soc_pbmp_t* phy_pbmp, SHR_BITDCL* serdes_qmlfs);
STATIC int soc_jer_ilkn_memory_qmlfs_get(int unit, soc_port_t port, SHR_BITDCL* memory_qmlf);

extern unsigned char  tscf_ucode[];
extern unsigned short tscf_ucode_len;

#define SUB_PHYS_IN_QSGMII                  4

#define SOC_JER_NIF_NOF_LANE_QUADS          18

#define QMLF_0_BIT   1
#define QMLF_1_BIT   2
#define QMLF_2_BIT   4

phymod_bus_t portmod_ext_default_bus = {
    "MDIO Bus",
    portmod_ext_phy_mdio_c45_reg_read,
    portmod_ext_phy_mdio_c45_reg_write,
    NULL,
    portmod_common_mutex_take,
    portmod_common_mutex_give,
    PHYMOD_BUS_CAP_WR_MODIFY | PHYMOD_BUS_CAP_LANE_CTRL
};

phymod_bus_t portmod_c22_bus = {
    "MDIO Cl22 Bus",
    soc_jer_phy_mdio_c22_reg_read,
    soc_jer_phy_mdio_c22_reg_write,
    NULL,
    portmod_common_mutex_take,
    portmod_common_mutex_give,
    PHYMOD_BUS_CAP_WR_MODIFY
};

static soc_dpp_pm_entry_t soc_jer_pml_table[] = {
     /* is_qsgmii, is_vaild, pml_instance , PHY ID (used as core address in PHYMOD*/
        {0,         1,          0,            0x00},
        {0,         1,          0,            0x04},
        {0,         1,          0,            0x08},
        {0,         1,          0,            0x0c},
        {1,         1,          0,            0x10},
        {1,         1,          0,            0x14},

        {0,         0,          1,            0x00},
        {0,         0,          1,            0x04},
        {0,         0,          1,            0x08},
        {0,         1,          1,            0x0c},
        {1,         1,          1,            0x10},
        {1,         1,          1,            0x14},
};

static soc_dpp_pm_entry_t soc_qmx_pml_table[] = {
     /* is_qsgmii, is_vaild, pml_instance , PHY ID (used as core address in PHYMOD*/
        {0,         1,          0,            0x00},
        {0,         1,          0,            0x04},
        {0,         1,          0,            0x08},
        {0,         1,          0,            0x0c},
        {1,         1,          0,            0x10},
        {1,         1,          0,            0x14},

        {0,         1,          1,            0x00},
        {0,         1,          1,            0x04},
        {0,         1,          1,            0x08},
        {0,         1,          1,            0x0c},
        {1,         1,          1,            0x10},
        {1,         1,          1,            0x14},
};

static soc_dpp_pm_entry_t soc_jer_plus_pml_table[] = {
     /* is_qsgmii, is_vaild, pml_instance , PHY ID (used as core address in PHYMOD*/
        {0,         1,          0,            0x00},
        {0,         1,          0,            0x04},
        {0,         1,          0,            0x08},
        {0,         1,          0,            0x0c},
        {1,         1,          0,            0x10},
        {1,         1,          0,            0x14},

        {0,         1,          1,            0x00},
        {0,         1,          1,            0x04},
        {0,         1,          1,            0x08},
        {0,         1,          1,            0x0c},
        {0,         1,          1,            0x10},
        {0,         1,          1,            0x14},
};

static portmod_pm_instances_t jer_pm_instances[] = {
    {portmodDispatchTypeDnx_fabric, SOC_JERICHO_PM_FABRIC},
    {portmodDispatchTypePm4x25, SOC_JERICHO_PM_4x25},
    {portmodDispatchTypePm4x10, 5},
    {portmodDispatchTypePm4x10Q, 4},
    {portmodDispatchTypePmOsILKN, 3},
    /*QMX extras*/
    {portmodDispatchTypePm4x10, 3},
};

static portmod_pm_instances_t jer_plus_pm_instances[] = {
    {portmodDispatchTypeDnx_fabric, SOC_JERICHO_PLUS_PM_FABRIC},
    {portmodDispatchTypePm4x25, SOC_JERICHO_PLUS_PM_4x25},
    {portmodDispatchTypePm4x10, 4},
    {portmodDispatchTypePm4x10Q, SOC_JERICHO_PLUS_PM_QSGMII},
    {portmodDispatchTypePmOsILKN, 3},
};

static portmod_pm_identifier_t jer_ilkn_pm_table[SOC_JERICHO_NOF_ILKN_WRAP][SOC_JERICHO_MAX_PMS_PER_ILKN_PORT] =
{
        {
          {portmodDispatchTypePm4x25, 1},
          {portmodDispatchTypePm4x25, 5},
          {portmodDispatchTypePm4x25, 9},
          {portmodDispatchTypePm4x25, 13},
          {portmodDispatchTypePm4x25, 17},
          {portmodDispatchTypePm4x25, 21}
        },
        {
          {portmodDispatchTypePm4x10, 25},
          {portmodDispatchTypePm4x10, 29},
          {portmodDispatchTypePm4x10, 33},
          {portmodDispatchTypePm4x10, 37},
          {portmodDispatchTypePm4x10, 41},
          {portmodDispatchTypePm4x10, 45}
        },
        {
          {portmodDispatchTypePm4x10, 49},
          {portmodDispatchTypePm4x10, 53},
          {portmodDispatchTypePm4x10, 57},
          {portmodDispatchTypePm4x10, 61},
          {portmodDispatchTypePm4x10, 65},
          {portmodDispatchTypePm4x10, 69}
        },
};


static portmod_pm_identifier_t jer_plus_ilkn_pm_table[SOC_JERICHO_NOF_ILKN_WRAP][SOC_JERICHO_MAX_PMS_PER_ILKN_PORT] =
{
        {
          {portmodDispatchTypePm4x25, 1},
          {portmodDispatchTypePm4x25, 5},
          {portmodDispatchTypePm4x25, 9},
          {portmodDispatchTypePm4x25, 13},
          {portmodDispatchTypePm4x25, 17},
          {portmodDispatchTypePm4x25, 21}
        },
        {
          {portmodDispatchTypePm4x10, 25},
          {portmodDispatchTypePm4x10, 29},
          {portmodDispatchTypePm4x10, 33},
          {portmodDispatchTypePm4x10, 37},
          {portmodDispatchTypePm4x10, 41},
          {portmodDispatchTypePm4x10, 45}
        },
        {
          {portmodDispatchTypePm4x25, 49},
          {portmodDispatchTypePm4x25, 53},
          {portmodDispatchTypePm4x25, 57},
          {portmodDispatchTypePm4x25, 61},
          {portmodDispatchTypePm4x25, 65},
          {portmodDispatchTypePm4x25, 69}
        },
};


STATIC
soc_field_t soc_jer_nbi_tx_flush_egress_fields[] =
{
        TX_FLUSH_EGRESS_PORT_0_MLF_0_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_1_MLF_0_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_2_MLF_0_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_3_MLF_0_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_0_MLF_1_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_1_MLF_1_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_2_MLF_1_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_3_MLF_1_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_0_MLF_2_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_1_MLF_2_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_2_MLF_2_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_3_MLF_2_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_0_MLF_3_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_1_MLF_3_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_2_MLF_3_QMLF_Nf,
        TX_FLUSH_EGRESS_PORT_3_MLF_3_QMLF_Nf
};

STATIC
soc_field_t soc_jer_nbi_tx_stop_egress_fields[] =
{
        TX_STOP_EGRESS_PORT_0_MLF_0_QMLF_Nf,
        TX_STOP_EGRESS_PORT_1_MLF_0_QMLF_Nf,
        TX_STOP_EGRESS_PORT_2_MLF_0_QMLF_Nf,
        TX_STOP_EGRESS_PORT_3_MLF_0_QMLF_Nf,
        TX_STOP_EGRESS_PORT_0_MLF_1_QMLF_Nf,
        TX_STOP_EGRESS_PORT_1_MLF_1_QMLF_Nf,
        TX_STOP_EGRESS_PORT_2_MLF_1_QMLF_Nf,
        TX_STOP_EGRESS_PORT_3_MLF_1_QMLF_Nf,
        TX_STOP_EGRESS_PORT_0_MLF_2_QMLF_Nf,
        TX_STOP_EGRESS_PORT_1_MLF_2_QMLF_Nf,
        TX_STOP_EGRESS_PORT_2_MLF_2_QMLF_Nf,
        TX_STOP_EGRESS_PORT_3_MLF_2_QMLF_Nf,
        TX_STOP_EGRESS_PORT_0_MLF_3_QMLF_Nf,
        TX_STOP_EGRESS_PORT_1_MLF_3_QMLF_Nf,
        TX_STOP_EGRESS_PORT_2_MLF_3_QMLF_Nf,
        TX_STOP_EGRESS_PORT_3_MLF_3_QMLF_Nf
};

STATIC int enable_ilkn_fields[] = {
    ENABLE_PORT_0f, 
    ENABLE_PORT_1f, 
    ENABLE_PORT_2f, 
    ENABLE_PORT_3f, 
    ENABLE_PORT_4f, 
    ENABLE_PORT_5f
};

STATIC int tdm_data_hrf_fields[] = {
    ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_0f,
    ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_1f, 
    ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_2f, 
    ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_3f, 
    ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_4f, 
    ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_5f
};

STATIC int
soc_jer_phy_mdio_c22_reg_read( void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t *val)
{
    int rv= SOC_E_NONE;
    portmod_default_user_access_t *user_data = user_acc;
    uint32 phy_id;
    uint16 rd_data;

    phy_id = core_addr;

    if(user_data == NULL){
        return SOC_E_PARAM;
    }

    rv = soc_dcmn_miim_cl22_read(user_data->unit, phy_id, reg_addr, &rd_data);
    *val = rd_data;

    return rv;
}

STATIC int
soc_jer_phy_mdio_c22_reg_write( void* user_acc, uint32_t core_addr, uint32_t reg_addr, uint32_t val)
{
    int rv= SOC_E_NONE;
    portmod_default_user_access_t *user_data = user_acc;
    uint32 phy_id;
    uint16 wr_data;

    if(user_data == NULL){
        return SOC_E_PARAM;
    }

    phy_id = core_addr;
    wr_data = val;

    rv = soc_dcmn_miim_cl22_write(user_data->unit, phy_id, reg_addr, wr_data);

    return rv;
}

int
soc_jer_pml_table_get(int unit, soc_dpp_pm_entry_t **soc_pml_table)
{
    SOCDNX_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        *soc_pml_table = &soc_jer_plus_pml_table[0];
    } 
    else if (SOC_IS_QMX(unit)) 
    {
        *soc_pml_table = &soc_qmx_pml_table[0];
    }
    else
    {
        *soc_pml_table = &soc_jer_pml_table[0];
    }

    SOCDNX_FUNC_RETURN;
}

/* 
 * function - soc_jer_ilkn_table_get 
 * get correct ilkn table according to nbi instance. 
 * nbi_index: 0- NBIH, 1- NBIL0, 2- NBIL1 
 */
STATIC int
soc_jer_ilkn_table_get(int unit, int nbi_index, portmod_pm_identifier_t **ilkn_table)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit))
    {
        *ilkn_table = jer_plus_ilkn_pm_table[nbi_index];
    }
    else
    {
        *ilkn_table = jer_ilkn_pm_table[nbi_index];
    }

    SOCDNX_FUNC_RETURN;
}


int
soc_jer_pm_instances_get(int unit, portmod_pm_instances_t **pm_instances, int *pms_instances_arr_len)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit))
    {
          *pm_instances = &jer_plus_pm_instances[0];
          *pms_instances_arr_len = 5;      
    }   
    else if (SOC_IS_QMX(unit))
    {
          *pm_instances = &jer_pm_instances[0];
          *pms_instances_arr_len = 6;
    }
    else
    {
          *pm_instances = &jer_pm_instances[0];
          *pms_instances_arr_len = 5;
    }

    SOCDNX_FUNC_RETURN;
}

/* 
 * phy should be 1-based (1-72), 
 * new_phy returned is also 1-based (1-144) 
 */ 
int
soc_jer_qsgmii_offsets_add(int unit, uint32 phy, uint32 *new_phy)
{
    int qsgmii_count, skip;
    SOCDNX_INIT_FUNC_DEFS;

    if(phy < 37 || phy >= 192) {
        skip = 0;
    } else {
        qsgmii_count = phy - 37;
        if (qsgmii_count > 12) {
            qsgmii_count = 12;
        }
        if (phy > 15*4) {
           qsgmii_count += phy - 61;
        }

        skip = qsgmii_count*(SUB_PHYS_IN_QSGMII - 1);
    }

    *new_phy = phy + skip;

    SOCDNX_FUNC_RETURN;
}

int
soc_jer_qsgmii_offsets_add_pbmp(int unit, soc_pbmp_t* pbmp, soc_pbmp_t* new_pbmp)
{
    uint32 phy, new_phy;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*new_pbmp);

    SOC_PBMP_ITER(*pbmp, phy)
    {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_add, (unit, phy, &new_phy))); 
        SOC_PBMP_PORT_ADD(*new_pbmp, new_phy);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* 
 * phy should be 1-based (1-144), 
 * new_phy returned is also 1-based (1-72) 
 */ 
int
soc_jer_qsgmii_offsets_remove(int unit, uint32 phy, uint32 *new_phy)
{
    int qsgmii_count;
    int reduce = 0;
    SOCDNX_INIT_FUNC_DEFS;

    reduce = 0;

    if(phy >= 37 && phy < 192) {
        qsgmii_count = phy - 37;
        if (qsgmii_count > 48) {
            qsgmii_count = 48;
        } 

        if (phy >= 97) {
           qsgmii_count += phy - 97;
        }

        reduce = qsgmii_count - (qsgmii_count/SUB_PHYS_IN_QSGMII);

    }

    *new_phy = phy - reduce;

    SOCDNX_FUNC_RETURN;
}

int
soc_jer_qsgmii_offsets_remove_pbmp(int unit, soc_pbmp_t* pbmp, soc_pbmp_t* new_pbmp) 
{
    uint32 phy, new_phy;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*new_pbmp);

    SOC_PBMP_ITER(*pbmp, phy) 
    {

        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy, &new_phy)));
        SOC_PBMP_PORT_ADD(*new_pbmp, new_phy);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_lane_map_get(int unit, int quad, phymod_lane_map_t* lane_map)
{
    uint32 txlane_map, rxlane_map, mask, shift;
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(phymod_lane_map_t_init(lane_map));

    txlane_map = soc_property_suffix_num_get(unit, quad, spn_PHY_TX_LANE_MAP, "quad", 0x3210);
    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "quad %d - txlane_map 0x%x \n"),quad , txlane_map));
    rxlane_map = soc_property_suffix_num_get(unit, quad, spn_PHY_RX_LANE_MAP, "quad", 0x3210);
    LOG_DEBUG(BSL_LS_SOC_INIT,
             (BSL_META_U(unit,
                         "quad %d - rxlane_map 0x%x \n"),quad , rxlane_map));

    lane_map->num_of_lanes = NUM_OF_LANES_IN_PM;

    mask = 0xf;
    shift = 0;
    for(i=0 ; i<NUM_OF_LANES_IN_PM ; i++) {
        lane_map->lane_map_tx[i] =  ((txlane_map & mask) >> shift);
        lane_map->lane_map_rx[i] =  ((rxlane_map & mask) >> shift);
        mask = mask << 4;
        shift = shift + 4;
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

/* 
 * get ILKN port phys as bitmap of up to 24 bits, 0-based.     *
 * if is_format_adjust is set - ILKN 1/3/5 bitmap will be reversed    *  
 *                                                             */
int
soc_jer_nif_ilkn_phys_aligned_pbmp_get(int unit, soc_port_t port, soc_pbmp_t *phys_aligned, int is_format_adjust) 
{
    uint32 offset, phy_i, phy, align;
    soc_pbmp_t phys, phy_ports;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*phys_aligned);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phy_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phy_ports, &phys));

    align = SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset) ? 192 : 1;

    SOC_PBMP_ITER(phys, phy_i){
        phy = (phy_i - align) % JER_NIF_ILKN_MAX_NOF_LANES;
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            phy += SOC_DPP_DEFS_GET(unit, first_fabric_link_id);
        }
        /*in SLE the odd ILKNs (ILKN1/3/5) are reversed, so if is_format_adjuat is set, reverse the lane index*/
        SOC_PBMP_PORT_ADD(*phys_aligned, ((offset & 1) && is_format_adjust) ? JER_NIF_ILKN_MAX_NOF_LANES - phy - 1 : phy); 
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_nif_ilkn_lane_map_get(int unit, soc_port_t port, int *ilkn_lane_map_override, int ilkn_lane_map[JER_NIF_ILKN_MAX_NOF_LANES]) 
{
    uint32 nof_lanes, offset, lane_i;
    char *map_str;
    int mapping, lane, i, map_index;
    soc_pbmp_t phys, phy_ports, ilkn_pbmp;
    int temp_lane_map[JER_NIF_ILKN_MAX_NOF_LANES] = {0};
    int active_lanes[JER_NIF_ILKN_MAX_NOF_LANES] = {0};
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(ilkn_pbmp);

    *ilkn_lane_map_override = 0; /*no ovverride by default*/

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &nof_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));

    for (lane = 0; lane < nof_lanes; lane++)
    {
        /*Format: ilkn_lane_map_<logical_port>_lane<num>=<mapping>*/
        /*To easily support reverse-order mapping the following convention can be used: ilkn_lane_map_1=reversed*/
        /*Default value is 1-1 mapping*/
        map_str = soc_property_port_suffix_num_get_str(unit, port, lane, spn_ILKN_LANE_MAP, "lane");
        if (map_str == NULL) {
            mapping = lane; 
        } else if (sal_strcmp(map_str, "reversed")== 0) {
            mapping = nof_lanes - 1 - lane;

            /*if the soc property explictly set - mark override*/
            *ilkn_lane_map_override = 1;
        } else {
            mapping = soc_property_port_suffix_num_get(unit, port, lane, spn_ILKN_LANE_MAP, "lane", lane);

            /*if the soc property explictly set - mark override*/
            *ilkn_lane_map_override = 1;
        }
        
        ilkn_lane_map[lane] = mapping;
    }

    if (*ilkn_lane_map_override == 1) {
        /* adjust the mapping to the actual lanes in the core */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phy_ports));
        SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phy_ports, &phys));

        /* get the ILKN lanes bitmap - reverted for ILKN 1/3/5 */
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_port_nif_ilkn_phys_aligned_pbmp_get, (unit, port, &ilkn_pbmp, 1)));

        /* create the active lanes array */
        lane = 0;
        SOC_PBMP_ITER(ilkn_pbmp, lane_i) {
            if (lane >= JER_NIF_ILKN_MAX_NOF_LANES) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Lane %d is out of range"), lane_i));
            }
            if (offset & 1) {
                /*ILKN1/3/5 is reversed*/
            	active_lanes[nof_lanes - 1 - lane] = lane_i;
            } else {
            	active_lanes[lane] = lane_i;
            }
            ++lane;
        }
        /* match the mapping to the active lanes index */
        lane = 0;
        SOC_PBMP_ITER(ilkn_pbmp, lane_i) {
            if (lane_i >= JER_NIF_ILKN_MAX_NOF_LANES || lane >= JER_NIF_ILKN_MAX_NOF_LANES) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Lane %d is out of range"), lane_i));
            }
            map_index = ilkn_lane_map[lane];
            temp_lane_map[lane_i] = active_lanes[map_index];
            ++lane;
        }
        /* copy the temp back to the output array */
        for (i = 0; i < JER_NIF_ILKN_MAX_NOF_LANES; ++i) {
            ilkn_lane_map[i] = temp_lane_map[i];
        }
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_portmod_calc_os(int unit, phymod_phy_access_t* phy_access, uint32* os_int, uint32* os_remainder)
{
    int rv;
    phymod_phy_diagnostics_t phy_diag;
    SOCDNX_INIT_FUNC_DEFS;

    rv = phymod_phy_diagnostics_t_init(&phy_diag);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = phymod_phy_diagnostics_get(phy_access, &phy_diag);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = phymod_osr_mode_to_actual_os(phy_diag.osr_mode, os_int, os_remainder);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

/* 
 * Segment is a resource of the ILKN core, which should be shared by the two ILKN ports on the same core.
 * The more segment a port has, the more bandwidth it can carry.
 * This function takes the number of segments that the port should have, and write is to HW. 
 * This function is NOT symmetric to nof_segments_get and the procedure described here should only be used.                       
 * after an activity that might change the number of segments has been called - for example port_speed_set.
 */
int soc_jer_nif_ilkn_nof_segments_set(int unit, int port, uint32 nof_segments) {

    soc_port_t reg_port;
    uint32 reg_val, offset;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    /* ILKN0/1 -> port=REG_PORT_ANY, ILKN2/3 -> port=0, ILKN4/5 -> port=1 */
    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4); 

    SOCDNX_IF_ERR_EXIT(READ_NBIH_ENABLE_INTERLAKENr(unit, &reg_val)); 
    /* in Jericho, ILKN 0/2/4 always get 4 segments, and then ILKN 1/3/5 cannot work. 
    * in order to give ILKN 0/2/4 only 2 segements, we enable ILKN 1/3/5 even if it is not a real port.
    * in Jer_plus this logic is fixed and it is possible to assign only 2 segments to ILKN0/2/4 
    * it is needed to set this fields even if only ILKN 1/3/5 is active*/ 
    if (offset < 2 && SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_RXf, (nof_segments == 2) ? 1 : 0); 
        soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_TXf, (nof_segments == 2) ? 1 : 0); 
    } else if (((offset & 1) == 0)) {
        soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, enable_ilkn_fields[offset + 1], (nof_segments == 2) ? 1 : 0); 
    }
    SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ENABLE_INTERLAKENr(unit, reg_val));

    if(offset >= 2) {
        SOCDNX_IF_ERR_EXIT(READ_NBIL_ENABLE_INTERLAKENr(unit, reg_port, &reg_val)); 
        if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
            soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_RXf, (nof_segments == 2) ? 1 : 0); 
            soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_TXf, (nof_segments == 2) ? 1 : 0); 
        } else if (((offset & 1) == 0)) {
            soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, ENABLE_PORT_1f, (nof_segments == 2) ? 1 : 0);
        }
        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ENABLE_INTERLAKENr(unit, reg_port, reg_val));
    }

    /* Change nof segments on the ILKN side */
    SOCDNX_IF_ERR_EXIT(portmod_port_ilkn_nof_segments_set(unit, port, nof_segments));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_nif_ilkn_nof_segments_get(int unit, int port, uint32 *nof_segments) {

    SOCDNX_INIT_FUNC_DEFS;

    /* Get nof segments from the ILKN side */
    SOCDNX_IF_ERR_EXIT(portmod_port_ilkn_nof_segments_get(unit, port, nof_segments));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC 
int soc_jer_nif_ilkn_nof_segments_calc_internal(int unit, soc_port_t port, int device_core_clk_khz, int num_lanes, int ilkn_burst_short, int serdes_speed, uint32* nof_segments) 
{
    int core_clock=0;
    int packet_size;
    int max_core_clock;
    int max_core_clock_dividend;
    int max_core_clock_divisor, lane2pipe_ratio_freq;
    const int ui_f_width=256;
    SOCDNX_INIT_FUNC_DEFS;

    /* Iterating with 2 and 4 segments to calculate the core clock which will be compared to the device frequency*/
    for(*nof_segments = (num_lanes <= 12) ? 2 : 4; *nof_segments <= 4; *nof_segments = *nof_segments + 2) {
        for(packet_size = 1; packet_size < 516; packet_size++) {
            max_core_clock_dividend=((SOC_SAND_DIV_ROUND_UP((packet_size*8),ui_f_width)*100)/(*nof_segments))*num_lanes*serdes_speed;
            if(((packet_size+7)/8)>(ilkn_burst_short/8)){
                max_core_clock_divisor=67*((SOC_SAND_DIV_ROUND_UP(packet_size,8))+(SOC_SAND_DIV_ROUND_UP(packet_size,512)));
            }else{
                max_core_clock_divisor=67*((ilkn_burst_short/8)+(SOC_SAND_DIV_ROUND_UP(packet_size,512)));
            }
            max_core_clock=SOC_SAND_DIV_ROUND(max_core_clock_dividend,max_core_clock_divisor);
            if(max_core_clock>core_clock){
                core_clock=max_core_clock;
            }
        }
        lane2pipe_ratio_freq = (num_lanes * serdes_speed * 1000) / (((*nof_segments == 2) ? 7 : 14) * 67);
        /*Check if core clock is still bigger than the device frequency*/
        if((core_clock*10) > device_core_clk_khz || lane2pipe_ratio_freq > device_core_clk_khz){
            core_clock = 0;
            continue;
        } else {
            break;
        }
    }

    SOCDNX_FUNC_RETURN;
}

/* 
 * Segment is a resource of the ILKN core, which should be shared by the two ILKN ports on the same core.
 * The more segment a port has, the more bandwidth it can carry.
 * in this function, a calculation is made to determine how many segments a port SHOULD have. 
 * Valid segments values are 2 and 4, when "small" ILKN ports (ILKN1/3/5) can only have 2 segments.         
 * When ILKN0/2/4 have all 4 segments, ILKN1/3/5 (respectively) cannot be loaded.
 */
int soc_jer_nif_ilkn_nof_segments_calc(int unit, int port, uint32 *nof_segments) 
{    
    uint32 offset, num_lanes;
    int serdes_speed, burst_short, device_core_clock_khz;
    SOCDNX_INIT_FUNC_DEFS;

    device_core_clock_khz = SOC_DPP_CONFIG(unit)->arad->init.core_freq.frequency;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    burst_short = SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_short[offset];
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_get(unit, port, &serdes_speed));

    /* Actual calculation - affected by: device core clock, ILKN lane rate, number of lanes, and ILKN burstShort */
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_nof_segments_calc_internal(unit, port, device_core_clock_khz, num_lanes, burst_short, serdes_speed, nof_segments));

    /* If speed is not supported you can't take any segments return error message */
    if(*nof_segments > 4 || ((offset & 1) && (*nof_segments > 2))) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("ILKN%d cannot support %d lanes at %d speed with Core Clock %dMHz\n"), 
                                              offset, num_lanes, serdes_speed, device_core_clock_khz/1000));
    }
    /* For ILKN1/3/5 - make sure ILKN0/2/4 doesnt hold all 4 segments */
    if ((offset & 1) && SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset - 1] == 4) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("ILKN%d doesn't have enough resources to load. consider reducing the speed for ILKN%d\n"), 
                                              offset, offset - 1));
    }
    /* For ILKN0/2/4 with 4 segments - make sure ILKN1/3/5 not present*/
    if ((*nof_segments == 4) && SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset + 1] > 0) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("ILKN%d cannot support %d lanes at %d speed with Core Clock %dMHz. "
                                             "Consider removing ILKN%d.\n"), offset, num_lanes, serdes_speed, device_core_clock_khz/1000, offset + 1));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_soft_reset(int unit, soc_port_t port, portmod_call_back_action_type_t action)
{
    int reg, reg_port, index, val;
    uint32  phy_port, lane, lane_i, rst_port;
    soc_pbmp_t phys, lanes;
    uint64 reg_val;
    soc_port_if_t interface_type;
    uint32 nof_pms_per_nbi  = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    uint32 nof_lanes_per_nbi = nof_pms_per_nbi * NUM_OF_LANES_IN_PM;
    uint32 nof_port_per_nbih = nof_lanes_per_nbi;
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_QAX_WITH_FABRIC_ENABLED(unit) && (IS_SFI_PORT(unit,port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port))) {
        interface_type = SOC_PORT_IF_SFI;
        if (soc_feature(unit, soc_feature_logical_port_num)) {
            phy_port = SOC_INFO(unit).port_l2p_mapping[port];
        } else {
            phy_port = port;
        }
        lane = phy_port;
    } else {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy_port, &lane)));
    }
    
    switch (action) {
    case portmodCallBackActionTypeDuring:
        if (SOC_IS_QUX(unit)) {
            reg = NIF_TX_QMLF_CONFIGr;
            reg_port = REG_PORT_ANY;
            index = ((lane - 1) % nof_lanes_per_nbi) / NUM_OF_LANES_IN_PM;

            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
            SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &lanes));
            val = (interface_type == SOC_PORT_IF_QSGMII) ? 0xf : 0x1;
            rst_port = 0;
            SOC_PBMP_ITER(lanes, lane_i) {
                rst_port |= val << (((lane_i - 1) % 4) * 4);
            }
        } else if (SOC_IS_QAX_WITH_FABRIC_ENABLED(unit) && (IS_SFI_PORT(unit,port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port))) {
            reg = NBIH_TX_QMLF_CONFIGr;
            reg_port = REG_PORT_ANY;
            index = (lane - 1) % nof_lanes_per_nbi / NUM_OF_LANES_IN_PM;
            rst_port = 0Xffff << (index * 4);
        } else {
            reg = (lane < nof_port_per_nbih + 1) ? NBIH_TX_QMLF_CONFIGr : NBIL_TX_QMLF_CONFIGr;
            reg_port = (lane < nof_port_per_nbih + 1) ? REG_PORT_ANY : lane > (2 * nof_lanes_per_nbi);
            index = ((lane - 1) % nof_lanes_per_nbi) / NUM_OF_LANES_IN_PM;

            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
            SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &lanes));
            val = (interface_type == SOC_PORT_IF_QSGMII) ? 0xf : 0x1;
            rst_port = 0;
            SOC_PBMP_ITER(lanes, lane_i) {
                rst_port |= val << (((lane_i - 1) % 4) * 4);
            }
        }

        SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, reg_port, index, &reg_val));
        soc_reg64_field32_set(unit, reg, &reg_val, TX_RESET_PORT_CREDITS_VALUE_QMLF_Nf, 0);
        soc_reg64_field32_set(unit, reg, &reg_val, TX_RESET_PORT_CREDITS_QMLF_Nf, 0);
        SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, index, reg_val));

        SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, reg_port, index, &reg_val));
        soc_reg64_field32_set(unit, reg, &reg_val, TX_RESET_PORT_CREDITS_QMLF_Nf, rst_port);
        SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, index, reg_val));

        SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, reg_port, index, &reg_val));
        soc_reg64_field32_set(unit, reg, &reg_val, TX_RESET_PORT_CREDITS_QMLF_Nf, 0);
        SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, index, reg_val));
        break;
    default:
        break;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_portmod_pmh_init(int unit)
{
    int pm, phy, l, ams_pll_val;
    soc_reg_t reg;
    soc_field_t field;
    soc_dpp_config_t *dpp = NULL;
    portmod_pm_create_info_t pm_info;
    portmod_default_user_access_t* user_acc;
    int is_sim, blk, first_phy, is_pml1_jer_plus, pm_idx, idx, quad;
    uint32 rx_polarity, tx_polarity, reg_val, otp_bits, pm_base_lane, ident_bits;
    uint32 nof_pm4x25    = SOC_DPP_DEFS_GET(unit, nof_pm4x25);
    uint32 pmh_base_lane = SOC_DPP_DEFS_GET(unit, pmh_base_lane);
    uint32 pml1_base_lane = SOC_DPP_DEFS_GET(unit, pml1_base_lane);
    uint32 nof_pms_per_nbi  = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    SOCDNX_INIT_FUNC_DEFS;

    dpp = SOC_DPP_CONFIG(unit);

    /* allocate user access struct */
    user_acc = sal_alloc(sizeof(portmod_default_user_access_t)*nof_pm4x25, "PM4x25_USER_ACCESS");
    if(user_acc == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate user access memory for PM4x25")));
    }
    sal_memset(user_acc, 0, sizeof(portmod_default_user_access_t)*nof_pm4x25);
    pm4x25_user_acc[unit] = user_acc;

    for(pm=0 ; pm<nof_pm4x25 ; pm++) {
        is_pml1_jer_plus = (pm >= 6 && SOC_IS_JERICHO_PLUS_ONLY(unit));
        SOCDNX_IF_ERR_EXIT(portmod_pm_create_info_t_init(unit, &pm_info));

        pm_info.type = portmodDispatchTypePm4x25;
        pm_base_lane = is_pml1_jer_plus ? pml1_base_lane : pmh_base_lane;
        pm_idx = pm % nof_pms_per_nbi;
        first_phy = pm_base_lane + NUM_OF_LANES_IN_PM*pm_idx + 1;

        SOCDNX_IF_ERR_EXIT(phymod_polarity_t_init(&(pm_info.pm_specific_info.pm4x25.polarity)));
        for(l=0 ; l<NUM_OF_LANES_IN_PM ; l++) {
            phy = first_phy + l;
            SOC_PBMP_PORT_ADD(pm_info.phys, phy);

            rx_polarity = soc_property_suffix_num_get(unit, phy, spn_PHY_RX_POLARITY_FLIP, "phy", 0);
            tx_polarity = soc_property_suffix_num_get(unit, phy, spn_PHY_TX_POLARITY_FLIP, "phy", 0);

            pm_info.pm_specific_info.pm4x25.polarity.rx_polarity |= ((rx_polarity & 0x1) << l);
            pm_info.pm_specific_info.pm4x25.polarity.tx_polarity |= ((tx_polarity & 0x1) << l);
        }

        if (is_pml1_jer_plus) {
            SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, dpp->jer->pll.ref_clk_pml_out[1], &(pm_info.pm_specific_info.pm4x25.ref_clk))); 
        } else {
            SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, dpp->jer->pll.ref_clk_pmh_out, &(pm_info.pm_specific_info.pm4x25.ref_clk))); 
        }

        /* Init phy access structure */
        SOCDNX_IF_ERR_EXIT(phymod_access_t_init(&pm_info.pm_specific_info.pm4x25.access));

        SOCDNX_IF_ERR_EXIT(portmod_default_user_access_t_init(unit, &(user_acc[pm])));
        PORTMOD_USER_ACCESS_FW_LOAD_REVERSE_SET((&(user_acc[pm])));
        user_acc[pm].unit = unit;
        blk = CLP_BLOCK(unit, pm);
        user_acc[pm].blk_id = blk;
        user_acc[pm].mutex = sal_mutex_create("core mutex");
        if(user_acc[pm].mutex == NULL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate mutex for PM4x25")));
        }

        pm_info.pm_specific_info.pm4x25.access.user_acc = &(user_acc[pm]);
        pm_info.pm_specific_info.pm4x25.access.addr = pm_idx*NUM_OF_LANES_IN_PM; /*PMH PHY addresses are 0,4,8,...*/
        pm_info.pm_specific_info.pm4x25.access.bus = NULL; /* Use default bus */
        ident_bits = is_pml1_jer_plus ? SOC_JERICHO_SIM_MASK_NBIL1 : SOC_JERICHO_SIM_MASK_NBIH;
        SOCDNX_IF_ERR_EXIT(soc_physim_check_sim(unit, phymodDispatchTypeTscf, &(pm_info.pm_specific_info.pm4x25.access), ident_bits, &is_sim));

        /* Firmward loader */
        quad = is_pml1_jer_plus ? pm + nof_pms_per_nbi : pm;
        if(is_sim) {
            pm_info.pm_specific_info.pm4x25.fw_load_method = phymodFirmwareLoadMethodNone;            
            SOC_DPP_JER_CONFIG(unit)->nif.fw_verify[quad] = 0;
        } else {
            pm_info.pm_specific_info.pm4x25.fw_load_method = soc_property_suffix_num_get(unit, pm, spn_LOAD_FIRMWARE, "quad", phymodFirmwareLoadMethodExternal);
            SOC_DPP_JER_CONFIG(unit)->nif.fw_verify[quad] = (pm_info.pm_specific_info.pm4x25.fw_load_method & 0xff00 ? 1 : 0);
            pm_info.pm_specific_info.pm4x25.fw_load_method &= 0xff;
        }
        pm_info.pm_specific_info.pm4x25.external_fw_loader = NULL; /* Use default external loader */

        /* Lane mapping */
        SOCDNX_IF_ERR_EXIT(soc_jer_lane_map_get(unit, is_pml1_jer_plus ? pm + nof_pms_per_nbi : pm, &(pm_info.pm_specific_info.pm4x25.lane_map)));

        /* General PM4x25 configuration */
        pm_info.pm_specific_info.pm4x25.in_pm_12x10 = 0;

        pm_info.pm_specific_info.pm4x25.portmod_mac_soft_reset = soc_jer_portmod_soft_reset;
        pm_info.pm_specific_info.pm4x25.rescal = -1;

        if (SOC_IS_JERICHO_A0(unit) || SOC_IS_QMX_A0(unit)) {
            ams_pll_val = soc_property_suffix_num_get(unit, 0, "custom_feature", "ams_pll_override", 0x1f); 
            
            if ((ams_pll_val != 0x19) && (ams_pll_val != 0x1E) && (ams_pll_val != 0x1f)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ams pll value 0x%x not valid"), ams_pll_val));
            }
            pm_info.pm_specific_info.pm4x25.afe_pll.afe_pll_change_default = 1; 
            pm_info.pm_specific_info.pm4x25.afe_pll.ams_pll_en_hrz = ams_pll_val & 1; /*only bit 0 of the val*/
            pm_info.pm_specific_info.pm4x25.afe_pll.ams_pll_iqp = (ams_pll_val &~ 1) >> 1; /*bits 1-4 of the val*/
        }

        /* Add PM to PortMod*/
        SOCDNX_IF_ERR_EXIT(portmod_port_macro_add(unit, &pm_info));

        if (!SOC_WARM_BOOT(unit)) {
            /* Power Down on all ports - only active ports will be enabled */
            if (SOC_IS_QUX(unit)) {
                reg = NIF_NIF_PM_CFGr;
                field = PM_N_OTP_PORT_BOND_OPTIONf;
                idx = pm_idx;
            } else if (is_pml1_jer_plus) {
                switch (pm_idx) {
                case 3:
                    reg = NBIL_NIF_PM_CFG_3r;
                    idx = 0;
                    break;
                case 4:
                    reg = NBIL_NIF_PM_CFG_4r;
                    idx = 0;
                    break;
                case 5:
                    reg = NBIL_NIF_PM_CFG_5r;
                    idx = 0;
                    break;
                default:
                    reg = NBIL_NIF_PM_CFGr;
                    idx = pm_idx;
                    break;
                 }
                field = PML_N_OTP_PORT_BOND_OPTIONf;
            } else {
                reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIH_REG_0C06r : NBIH_NIF_PM_CFGr;
                field = (SOC_IS_JERICHO_PLUS(unit)) ? PMH_N_OTP_PORT_BOND_OPTIONf : FIELD_0_8f; 
                idx = pm;
            }
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, REG_PORT_ANY, idx, &reg_val));
            otp_bits =  soc_reg_field_get(unit, reg, reg_val, field);
            otp_bits |= 0xf0;
            otp_bits &= ~0x100; /* disable all quads - bit[8]  */
            soc_reg_field_set(unit, reg, &reg_val, field, otp_bits);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, REG_PORT_ANY, idx, reg_val));
        }
    }


exit:
    if(SOCDNX_FUNC_ERROR) {
        if(pm4x25_user_acc[unit] != NULL) {
            for(pm=0 ; pm<nof_pm4x25 ; pm++) {
                user_acc = (portmod_default_user_access_t*)(pm4x25_user_acc[unit]);
                if(user_acc[pm].mutex != NULL) {
                    sal_mutex_destroy(user_acc[pm].mutex);
                }
            }
            sal_free(pm4x25_user_acc);
            pm4x25_user_acc[unit] = NULL;
        }
    }
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_portmod_pml_init(int unit)
{
    int rv, pm, phy, l, quad, pml, xlp_instance, block_index, is_sim;
    soc_dpp_config_t *dpp = NULL;
    portmod_pm_create_info_t pm_info;
    portmod_pm4x10_create_info_t *pm4x10_info;
    portmod_pm4x10q_create_info_t *pm4x10q_info;
    portmod_default_user_access_t* user_acc;
    portmod_pm4x10q_user_data_t* qsgmii_user_acc;
    int first_phy, pm_idx, idx, otp_bits, qsgmii_index;
    uint32 ident_bits, rx_polarity, tx_polarity, reg_val;
    soc_reg_t reg;
    soc_field_t  field;
    soc_dpp_pm_entry_t      *soc_pml_info = NULL;
    uint32 nof_pms_per_nbi  = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    uint32 nof_pm4x10       = SOC_DPP_DEFS_GET(unit, nof_pm4x10);
    uint32 nof_pm4x10q      = SOC_DPP_DEFS_GET(unit, nof_pm4x10q);
    uint32 pml_base_lane    = SOC_DPP_DEFS_GET(unit, pml0_base_lane);
    SOCDNX_INIT_FUNC_DEFS;

    dpp = SOC_DPP_CONFIG(unit);
    rv = MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_soc_pml_table_get,(unit, &soc_pml_info));
    SOCDNX_IF_ERR_EXIT(rv);
    /* allocate user access struct */    
    user_acc = sal_alloc(sizeof(portmod_default_user_access_t) * nof_pm4x10, "PM4x10_USER_ACCESS");
    if(user_acc == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate user access memory for PM4x10")));
    }
    pm4x10_user_acc[unit] = user_acc;
    qsgmii_user_acc = sal_alloc(sizeof(portmod_pm4x10q_user_data_t) * nof_pm4x10q, "PM4x10Q_USER_ACCESS");
    if(user_acc == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate user access memory for PM4x10Q")));
    }

    pm4x10q_user_acc[unit] = qsgmii_user_acc;

    qsgmii_index = 0;
    xlp_instance = 0;
    /* Initialize PM4x10\PM4x10Q */
    for(pm=0 ; pm<nof_pm4x10 ; pm++) {
        if(!soc_pml_info[pm].is_valid) {
            ++xlp_instance;
            continue;
        }
        SOCDNX_IF_ERR_EXIT(portmod_pm_create_info_t_init(unit, &pm_info));
        block_index = XLP_BLOCK(unit, xlp_instance); 

        if (soc_pml_info[pm].is_qsgmii) {
            pm_info.type = portmodDispatchTypePm4x10Q;
            pm4x10_info = &pm_info.pm_specific_info.pm4x10q.pm4x10_info;
            pm4x10q_info = &pm_info.pm_specific_info.pm4x10q;
        } else {
            pm_info.type = portmodDispatchTypePm4x10;
            pm4x10_info = &pm_info.pm_specific_info.pm4x10;
            pm4x10q_info = NULL;
        }
        pml = soc_pml_info[pm].pml_instance;
        first_phy = pml_base_lane + NUM_OF_LANES_IN_PM*pm + 1;

        SOCDNX_IF_ERR_EXIT(phymod_polarity_t_init(&(pm4x10_info->polarity)));

        for(l=0 ; l<NUM_OF_LANES_IN_PM ; l++) {
            phy = first_phy + l;
            SOC_PBMP_PORT_ADD(pm_info.phys, phy);

            rx_polarity = soc_property_suffix_num_get(unit, phy, spn_PHY_RX_POLARITY_FLIP, "phy", 0);
            tx_polarity = soc_property_suffix_num_get(unit, phy, spn_PHY_TX_POLARITY_FLIP, "phy", 0);

            pm4x10_info->polarity.rx_polarity |= ((rx_polarity & 0x1) << l);
            pm4x10_info->polarity.tx_polarity |= ((tx_polarity & 0x1) << l);            
        }

        SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_add_pbmp(unit,(soc_pbmp_t*)&pm_info.phys, (soc_pbmp_t*)&pm_info.pm_specific_info.pm4x10.phy_ports));

        /*Sets the PLL refclk separately for PML0 and PML1*/
        if (pm < nof_pms_per_nbi) {
            SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, dpp->jer->pll.ref_clk_pml_out[0], &(pm_info.pm_specific_info.pm4x10.ref_clk)));
        } else {
            SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, dpp->jer->pll.ref_clk_pml_out[1], &(pm_info.pm_specific_info.pm4x10.ref_clk)));
        }

        SOCDNX_IF_ERR_EXIT(phymod_access_t_init(&pm4x10_info->access));

        SOCDNX_IF_ERR_EXIT(portmod_default_user_access_t_init(unit, &(user_acc[pm])));
        PORTMOD_USER_ACCESS_FW_LOAD_REVERSE_SET((&(user_acc[pm])));
        user_acc[pm].unit = unit;
        user_acc[pm].blk_id = block_index;
        user_acc[pm].mutex = sal_mutex_create("core mutex");
        if(user_acc[pm].mutex == NULL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate mutex for PM4x10")));
        }

        if (soc_pml_info[pm].is_qsgmii) {
            SOCDNX_IF_ERR_EXIT(portmod_default_user_access_t_init(unit, &(qsgmii_user_acc[qsgmii_index].qsgmiie_user_data)));
            PORTMOD_USER_ACCESS_FW_LOAD_REVERSE_SET((&(qsgmii_user_acc[qsgmii_index].qsgmiie_user_data)));
            qsgmii_user_acc[qsgmii_index].qsgmiie_user_data.unit = unit;
            qsgmii_user_acc[qsgmii_index].qsgmiie_user_data.blk_id = PMQ_BLOCK(unit, qsgmii_index);
            qsgmii_user_acc[qsgmii_index].qsgmiie_user_data.mutex = sal_mutex_create("core mutex");
            if(qsgmii_user_acc[qsgmii_index].qsgmiie_user_data.mutex == NULL) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate mutex for PM4x10")));
            }
            pm4x10q_info->blk_id = PMQ_BLOCK(unit, qsgmii_index);
        }


        /* Firmward loader */
        quad = (pml_base_lane / SOC_JERICHO_NOF_LANES_PER_CORE) + pm;
        pm4x10_info->fw_load_method = soc_property_suffix_num_get(unit, quad, spn_LOAD_FIRMWARE, "quad", phymodFirmwareLoadMethodExternal);
        SOC_DPP_JER_CONFIG(unit)->nif.fw_verify[quad] = (pm4x10_info->fw_load_method & 0xff00 ? 1 : 0);
        pm4x10_info->fw_load_method &= 0xff;
        pm4x10_info->external_fw_loader = NULL; /* Use default external loader */

        pm4x10_info->access.user_acc = &(user_acc[pm]);
        pm4x10_info->access.addr = soc_pml_info[pm].phy_id;
        pm4x10_info->access.bus = NULL; /* Use default bus */

        if (soc_pml_info[pm].is_qsgmii) {
            pm4x10q_info->qsgmii_user_acc = (void*)(&(qsgmii_user_acc[qsgmii_index]));
            qsgmii_user_acc[qsgmii_index++].pm4x10_access = pm4x10_info->access;
            pm4x10_info->is_pm4x10q = 1;
        } else {
            pm4x10_info->is_pm4x10q = 0;
        }

        /* check sim */
        ident_bits = pml ? SOC_JERICHO_SIM_MASK_NBIL1 : SOC_JERICHO_SIM_MASK_NBIL0;
        SOCDNX_IF_ERR_EXIT(soc_physim_check_sim(unit, phymodDispatchTypeTsce, &(pm_info.pm_specific_info.pm4x10.access), ident_bits, &is_sim));
        if(is_sim) {
            pm_info.pm_specific_info.pm4x10.fw_load_method = phymodFirmwareLoadMethodNone;
            SOC_DPP_JER_CONFIG(unit)->nif.fw_verify[quad] = 0;
        } 

        SOCDNX_IF_ERR_EXIT(soc_jer_lane_map_get(unit, quad, &(pm4x10_info->lane_map)));

        pm_info.pm_specific_info.pm4x10.portmod_mac_soft_reset = soc_jer_portmod_soft_reset;
        pm_info.pm_specific_info.pm4x10.rescal = -1;

        SOCDNX_IF_ERR_EXIT(portmod_port_macro_add(unit, &pm_info));
        xlp_instance++;

        pm_idx = pm % nof_pms_per_nbi;
         /* QUX only one NBI block */
        if (SOC_IS_QUX(unit)) {
            reg = NIF_NIF_PM_CFGr;
            idx = pm_idx;
            pml = REG_PORT_ANY;
            field = PM_N_OTP_PORT_BOND_OPTIONf;
        } else {
            switch (pm_idx) {
                case 3:
                    reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_3r : NBIL_NIF_PM_CFG_3r;
                    idx = 0;
                    break;
                case 4:
                    reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_4r : NBIL_NIF_PM_CFG_4r;
                    idx = 0;
                    break;
                case 5:
                    reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_5r : NBIL_NIF_PM_CFG_5r;
                    idx = 0;
                    break;
                default:
                    reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A06r : NBIL_NIF_PM_CFGr;
                    idx = pm_idx;
                    break;
            }
            field = SOC_IS_JERICHO_PLUS(unit) ? PML_N_OTP_PORT_BOND_OPTIONf : FIELD_0_13f;
        }
        if (!SOC_WARM_BOOT(unit)) {
            /* Power Down on all ports - only active ports will be enabled */
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, pml, idx, &reg_val));
            otp_bits =  soc_reg_field_get(unit, reg, reg_val, field);
            otp_bits |= 0xf0;
            otp_bits &= ~0x100; /* disable all quads- bit [8] */
            soc_reg_field_set(unit, reg, &reg_val, field, otp_bits);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, pml, idx, reg_val));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

#ifdef PORTMOD_PM4X2P5_SUPPORT
STATIC int
soc_jer_portmod_pmx_init(int unit)
{
    int pm, phy, l;
    int xlp_instance;
    int otp_bits;
    soc_dpp_config_t *dpp = NULL;
    portmod_pm_create_info_t pm_info;
    portmod_default_user_access_t* user_acc;
    int is_sim, blk, first_phy, pm_idx, quad;
    uint32 rx_polarity, tx_polarity, ident_bits, reg_val;
    uint32 nof_pms_per_nbi  = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    uint32 nof_pm4x25       = SOC_DPP_DEFS_GET(unit, nof_pm4x25);
    uint32 nof_pm4x10       = SOC_DPP_DEFS_GET(unit, nof_pm4x10);
    uint32 nof_pm4x2p5    = SOC_DPP_DEFS_GET(unit, nof_pm4x2p5);
    uint32 pm_base_lane = SOC_DPP_DEFS_GET(unit, pmx_base_lane);
    SOCDNX_INIT_FUNC_DEFS;

    dpp = SOC_DPP_CONFIG(unit);

    /* allocate user access struct */
    user_acc = sal_alloc(sizeof(portmod_default_user_access_t)*nof_pm4x2p5, "PM4x2P5_USER_ACCESS");
    if(user_acc == NULL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate user access memory for PM4x2P5")));
    }
    sal_memset(user_acc, 0, sizeof(portmod_default_user_access_t)*nof_pm4x2p5);
    pm4x2p5_user_acc[unit] = user_acc;

    for(pm=0 ; pm<nof_pm4x2p5 ; pm++) {
        SOCDNX_IF_ERR_EXIT(portmod_pm_create_info_t_init(unit, &pm_info));

        pm_info.type = portmodDispatchTypePm4x2p5;
        pm_idx = pm;
        first_phy = pm_base_lane + NUM_OF_LANES_IN_PM*pm_idx + 1;

        SOCDNX_IF_ERR_EXIT(phymod_polarity_t_init(&(pm_info.pm_specific_info.pm4x2p5.polarity)));
        for(l=0 ; l<NUM_OF_LANES_IN_PM ; l++) {
            phy = first_phy + l;
            SOC_PBMP_PORT_ADD(pm_info.phys, phy);

            rx_polarity = soc_property_suffix_num_get(unit, phy, spn_PHY_RX_POLARITY_FLIP, "phy", 0);
            tx_polarity = soc_property_suffix_num_get(unit, phy, spn_PHY_TX_POLARITY_FLIP, "phy", 0);

            pm_info.pm_specific_info.pm4x2p5.polarity.rx_polarity |= ((rx_polarity & 0x1) << l);
            pm_info.pm_specific_info.pm4x2p5.polarity.tx_polarity |= ((tx_polarity & 0x1) << l);
        }

        SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, dpp->jer->pll.ref_clk_pmx_out, &(pm_info.pm_specific_info.pm4x2p5.ref_clk)));
        /* Init phy access structure */
        SOCDNX_IF_ERR_EXIT(phymod_access_t_init(&pm_info.pm_specific_info.pm4x2p5.access));

        SOCDNX_IF_ERR_EXIT(portmod_default_user_access_t_init(unit, &(user_acc[pm])));
        PORTMOD_USER_ACCESS_FW_LOAD_REVERSE_SET((&(user_acc[pm])));
        user_acc[pm].unit = unit;
        xlp_instance = (int)nof_pm4x10 + pm;
        blk = XLP_BLOCK(unit, xlp_instance);
        user_acc[pm].blk_id = blk;
        user_acc[pm].mutex = sal_mutex_create("core mutex");
        if(user_acc[pm].mutex == NULL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate mutex for PM4x2P5")));
        }

        pm_info.pm_specific_info.pm4x2p5.access.user_acc = &(user_acc[pm]);
        pm_idx = (nof_pm4x10+pm)%nof_pms_per_nbi;
        pm_info.pm_specific_info.pm4x2p5.access.addr = (pm_idx-nof_pm4x10)*NUM_OF_LANES_IN_PM + 0xa0;  /* PML and PMX are in the same NBI, addresses are continuous */
        pm_info.pm_specific_info.pm4x2p5.access.bus = &portmod_c22_bus;

        /* Firmward loader */
        quad = nof_pm4x25 + nof_pm4x10 + pm;
        pm_info.pm_specific_info.pm4x2p5.fw_load_method = phymodFirmwareLoadMethodNone;   /* Don't need to load firmware */
        SOC_DPP_JER_CONFIG(unit)->nif.fw_verify[quad] = 0;
        pm_info.pm_specific_info.pm4x2p5.fw_load_method &= 0xff;
        pm_info.pm_specific_info.pm4x2p5.external_fw_loader = NULL; /* Use default external loader */

        /* check sim */
        ident_bits = SOC_JERICHO_SIM_MASK_NBIL1;
        SOCDNX_IF_ERR_EXIT(soc_physim_check_sim(unit, phymodDispatchTypeViper, &(pm_info.pm_specific_info.pm4x2p5.access), ident_bits, &is_sim));

        /* Lane mapping */
        SOCDNX_IF_ERR_EXIT(soc_jer_lane_map_get(unit, quad, &(pm_info.pm_specific_info.pm4x2p5.lane_map)));

        /* General PM4x2P5 configuration */
        pm_info.pm_specific_info.pm4x2p5.portmod_mac_soft_reset = soc_jer_portmod_soft_reset;

        /* Add PM to PortMod*/
        SOCDNX_IF_ERR_EXIT(portmod_port_macro_add(unit, &pm_info));

        if (!SOC_WARM_BOOT(unit)) {
            /* Power Down on all ports - only active ports will be enabled */
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, NIF_NIF_PM_CFGr, REG_PORT_ANY, pm_idx, &reg_val));
            otp_bits =  soc_reg_field_get(unit, NIF_NIF_PM_CFGr, reg_val, PM_N_OTP_PORT_BOND_OPTIONf);
            otp_bits |= 0xf0;
            otp_bits &= ~0x100; /* disable all quads- bit [8] */
            soc_reg_field_set(unit, NIF_NIF_PM_CFGr, &reg_val, PM_N_OTP_PORT_BOND_OPTIONf, otp_bits);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, NIF_NIF_PM_CFGr, REG_PORT_ANY, pm_idx, reg_val));
        }
    }

exit:
    if(SOCDNX_FUNC_ERROR) {
        if(pm4x2p5_user_acc[unit] != NULL) {
            for(pm=0 ; pm<nof_pm4x2p5 ; pm++) {
                user_acc = (portmod_default_user_access_t*)(pm4x2p5_user_acc[unit]);
                if(user_acc[pm].mutex != NULL) {
                    sal_mutex_destroy(user_acc[pm].mutex);
                }
            }
            sal_free(pm4x2p5_user_acc);
            pm4x2p5_user_acc[unit] = NULL;
        }
    }
    SOCDNX_FUNC_RETURN;
}
#endif /* PORTMOD_PM4X2P5_SUPPORT */

STATIC
int soc_jer_fabric_mdio_address_get(int unit, int core_index, uint16 *phy_addr){
    uint16 bus_id = 0;
    uint16 port = 0;

    /* 
      Bus 1 cores: 0-2
      Bus 2 cores  3-5
      Bus 3 cores  6-8
     */
    bus_id = (core_index / 3) + 1 ; 

    /* phy ids: 0,4,8 */
    port = (core_index % 3) * 4;

    /* 
      encode for MIIM format:
      bits 0-4 for the "port" address 
      bits 5-6 and 8-9 for bus
      bit 7 for internal/external 
      0x80 for internal port
     */
    *phy_addr = 0x80 | ((bus_id & 0x3)<<PHY_ID_BUS_LOWER_SHIFT) | ((bus_id & 0xc)<<PHY_ID_BUS_UPPER_SHIFT) | port;

    return SOC_E_NONE;
}

/*
 * Function:
 *      soc_jer_dynamic_fabric_port_restore
 * Purpose:
 *      Restore SFI bitmap from portmod WB
 * Parameters:
 *      unit  - (IN)     Unit number.
 * Comments:
 *      Should be called after portmod initiation
 */
STATIC soc_error_t
soc_jer_dynamic_fabric_port_restore(int unit)
{
    int port, valid;
    soc_info_t   *si;
    SOCDNX_INIT_FUNC_DEFS;

    si  = &SOC_INFO(unit);

    for (port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id);
            port < FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id) + SOC_DPP_DEFS_GET(unit, nof_fabric_links);
        ++port) {

        SOCDNX_IF_ERR_EXIT(portmod_port_is_valid(unit, port, &valid));

        if (valid)
        {
            SOC_PBMP_PORT_ADD(si->sfi.bitmap, port);
            SOC_PBMP_PORT_ADD(si->port.bitmap, port);
            SOC_PBMP_PORT_ADD(si->all.bitmap, port);

            SOC_PBMP_PORT_REMOVE(si->sfi.disabled_bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->port.disabled_bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->all.disabled_bitmap, port);
        }
        else
        {
            SOC_PBMP_PORT_ADD(si->sfi.disabled_bitmap, port);
            SOC_PBMP_PORT_ADD(si->port.disabled_bitmap, port);
            SOC_PBMP_PORT_ADD(si->all.disabled_bitmap, port);

            SOC_PBMP_PORT_REMOVE(si->sfi.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->port.bitmap, port);
            SOC_PBMP_PORT_REMOVE(si->all.bitmap, port);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_portmod_ilkn_pmh_init(int unit)
{
    int pm_index, i;
    portmod_pbmp_t ilkn_phys;
    portmod_pm_create_info_t pm_info;
    portmod_pm_identifier_t ilkn_pms[MAX_NUM_OF_PMS_IN_ILKN];
    portmod_pm_identifier_t *ilkn_table;
    uint32 core_freq = 0x0;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_arad_core_frequency_config_get(unit, SOC_JER_CORE_FREQ_KHZ_DEFAULT, &core_freq));

    /* ILKN 0,1 */

    PORTMOD_PBMP_CLEAR(ilkn_phys);
    for(pm_index = 0 ; pm_index < MAX_NUM_OF_PMS_IN_ILKN ; pm_index++)
    {
        portmod_pm_identifier_t_init(unit, &ilkn_pms[pm_index]);
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_ilkn_table_get(unit, 0/*NBIH*/, &ilkn_table));

    for(pm_index=0 ; pm_index<MAX_NUM_OF_PMS_IN_ILKN ; pm_index++)
    {
        ilkn_pms[pm_index].type = ilkn_table[pm_index].type;
        ilkn_pms[pm_index].phy = ilkn_table[pm_index].phy;

        PORTMOD_PBMP_PORTS_RANGE_ADD(ilkn_phys, ilkn_pms[pm_index].phy, NUM_OF_LANES_IN_PM);
    }

    SOCDNX_IF_ERR_EXIT(portmod_pm_create_info_t_init(unit, &pm_info));
    pm_info.type = portmodDispatchTypePmOsILKN;
    pm_info.pm_specific_info.os_ilkn.controlled_pms = ilkn_pms;
    pm_info.pm_specific_info.os_ilkn.nof_aggregated_pms = MAX_NUM_OF_PMS_IN_ILKN;
    pm_info.pm_specific_info.os_ilkn.core_clock_khz = core_freq;
    for (i = 0; i < PORTMOD_MAX_ILKN_PORTS_PER_ILKN_PM; ++i) {
        pm_info.pm_specific_info.os_ilkn.wm_high[i] = JER_NIF_PM4x25_WM_HIGH; 
        pm_info.pm_specific_info.os_ilkn.wm_low[i] = JER_NIF_PM4x25_WM_LOW;
    }
    PORTMOD_PBMP_ASSIGN(pm_info.phys, ilkn_phys);
    SOCDNX_IF_ERR_EXIT(portmod_port_macro_add(unit, &pm_info));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_portmod_ilkn_pml_init(int unit) {

    portmod_pm_create_info_t pm_info;
    soc_port_t fabric_port;
    portmod_pm_identifier_t *ilkn_table;
    portmod_pbmp_t ilkn_phys[SOC_QMX_NUM_OF_PMLS];
    soc_pbmp_t ilkn_over_fabric_ports;
    portmod_pm_identifier_t ilkn_pms[SOC_QMX_NUM_OF_PMLS][MAX_NUM_OF_PMS_IN_ILKN];
    int nof_fabric_ilkn_pms, first_pm, lane, i, is_pml1_jer_plus;
    int fab_pm, pm, pml, phy, pm_over_fabric = 0, nof_ilkn_pms[SOC_QMX_NUM_OF_PMLS]= {0 , 0};
    uint32 over_fabric_type;
    soc_dpp_pm_entry_t *soc_pml_table;
    uint32 core_freq = 0x0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_arad_core_frequency_config_get(unit, SOC_JER_CORE_FREQ_KHZ_DEFAULT, &core_freq));

    /* clear structs and PBMPs */
    SOC_PBMP_CLEAR(ilkn_over_fabric_ports);
    for(pml = 0; pml < SOC_QMX_NUM_OF_PMLS; ++pml) {
        PORTMOD_PBMP_CLEAR(ilkn_phys[pml]);
        for(pm = 0; pm < MAX_NUM_OF_PMS_IN_ILKN; ++pm) {
            portmod_pm_identifier_t_init(unit, &ilkn_pms[pml][pm]);
        }
    }

    if (SOC_JER_NIF_IS_ILKN_OVER_FABRIC_ENABLED(unit)) {
        over_fabric_type = SOC_IS_QMX(unit) ? 2 : 1;
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_over_fabric_pbmp_get(unit, &ilkn_over_fabric_ports));
    } else {
        over_fabric_type = 0;
    }

    nof_fabric_ilkn_pms = SOC_IS_QMX(unit) ? SOC_QMX_PM_FABRIC : MAX_NUM_OF_PMS_IN_ILKN;

    SOCDNX_IF_ERR_EXIT(soc_jer_pml_table_get(unit, &soc_pml_table));

    for(pml = 0; pml < SOC_QMX_NUM_OF_PMLS; ++pml) {

        is_pml1_jer_plus = (pml == 1 && SOC_IS_JERICHO_PLUS_ONLY(unit));
        SOCDNX_IF_ERR_EXIT(soc_jer_ilkn_table_get(unit, pml + 1, &ilkn_table));

        /* build ILKN PM info */
        first_pm = pml * MAX_NUM_OF_PMS_IN_ILKN;
        fab_pm = SOC_IS_QMX(unit) ? -2 : 0; /* QMX 1st lane = jericho 8th lane */
        for(pm = first_pm; pm < first_pm + MAX_NUM_OF_PMS_IN_ILKN; ++pm, ++fab_pm) {
            /* if one lane from the PM is over fabric, the entire PM will be over fabric */
            pm_over_fabric = 0;
            if(soc_pml_table[pm].is_valid == 0) {
                pm_over_fabric = 1; /*PM will be over fabric to complete ilkn_pms to 6 - even if ILKN port does not actually use these phys*/
            }
            if (SOC_JER_NIF_IS_ILKN_OVER_FABRIC_ENABLED(unit) && pml == 1 && fab_pm >= 0 && fab_pm < nof_fabric_ilkn_pms) {
                for (lane = 0; lane < NUM_OF_LANES_IN_PM; ++lane) {
                    fabric_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id) + fab_pm * NUM_OF_LANES_IN_PM;
                    if (SOC_PBMP_MEMBER(ilkn_over_fabric_ports, fabric_port + lane)) {
                        pm_over_fabric = 1;
                        break;
                    }
                }
            }
            if (pm_over_fabric) {
                fabric_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id) + fab_pm * NUM_OF_LANES_IN_PM; 
                ilkn_pms[pml][nof_ilkn_pms[pml]].phy = SOC_INFO(unit).port_l2p_mapping[fabric_port];
                ilkn_pms[pml][nof_ilkn_pms[pml]].type = portmodDispatchTypeDnx_fabric;
                
            } else {
                if(soc_pml_table[pm].is_valid == 0) {
                    continue;
                }
                ilkn_pms[pml][nof_ilkn_pms[pml]].phy = ilkn_table[pm - first_pm].phy;
                ilkn_pms[pml][nof_ilkn_pms[pml]].type = ilkn_table[pm - first_pm].type;
            }

            for(lane = 0; lane < NUM_OF_LANES_IN_PM; ++lane) {
                phy = ilkn_pms[pml][nof_ilkn_pms[pml]].phy + lane;
                PORTMOD_PBMP_PORT_ADD(ilkn_phys[pml], phy);
            }
            ++nof_ilkn_pms[pml];
        }

        /* Add ILKN PM */
        SOCDNX_IF_ERR_EXIT(portmod_pm_create_info_t_init(unit, &pm_info));
        pm_info.type = portmodDispatchTypePmOsILKN;
        pm_info.pm_specific_info.os_ilkn.controlled_pms = ilkn_pms[pml];
        pm_info.pm_specific_info.os_ilkn.nof_aggregated_pms = nof_ilkn_pms[pml];
        pm_info.pm_specific_info.os_ilkn.core_clock_khz = core_freq;
        PORTMOD_PBMP_ASSIGN(pm_info.phys, ilkn_phys[pml]);
        for (i = 0; i < PORTMOD_MAX_ILKN_PORTS_PER_ILKN_PM; ++i) {
            pm_info.pm_specific_info.os_ilkn.wm_high[i] = is_pml1_jer_plus ? JER_NIF_PM4x25_WM_HIGH : JER_NIF_PM4x10_WM_HIGH; 
            pm_info.pm_specific_info.os_ilkn.wm_low[i] =  is_pml1_jer_plus ? JER_NIF_PM4x25_WM_LOW  : JER_NIF_PM4x10_WM_LOW;
        }
        pm_info.pm_specific_info.os_ilkn.is_over_fabric = over_fabric_type;
        SOCDNX_IF_ERR_EXIT(portmod_port_macro_add(unit, &pm_info));
    }

    /* disable port in SOC_PBMP_SFI */
    for (i = 0; i < nof_fabric_ilkn_pms * NUM_OF_LANES_IN_PM; ++i) {
        fabric_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id) + i;
        if (SOC_PBMP_MEMBER(ilkn_over_fabric_ports, fabric_port)) {
            SOC_PBMP_PORT_REMOVE(SOC_PORT_BITMAP(unit, sfi), fabric_port); 
            SOC_PBMP_PORT_REMOVE(SOC_PORT_BITMAP(unit, port), fabric_port);
            SOC_PBMP_PORT_REMOVE(SOC_PORT_BITMAP(unit, all), fabric_port);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_ilkn_init(int unit) {

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_portmod_ilkn_pmh_init(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_portmod_ilkn_pml_init(unit));

exit:
    SOCDNX_FUNC_RETURN;
}


STATIC int
soc_jer_portmod_fabric_init(int unit)
{
    int rv, quads_in_fsrd, nof_pms;
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_QAX(unit) && !SOC_IS_QAX_WITH_FABRIC_ENABLED(unit))
    {
        SOC_EXIT;
    }

    quads_in_fsrd = SOC_DPP_DEFS_GET(unit, nof_quads_in_fsrd);

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        nof_pms = SOC_JERICHO_PLUS_PM_FABRIC;
    } else if (SOC_IS_QAX(unit)) { /* Kalia or QAX mesh */
        nof_pms = SOC_QAX_PM_FABRIC;
    } else if (SOC_IS_QMX(unit)) {
        nof_pms = SOC_QMX_PM_FABRIC;
    } else {
        nof_pms = SOC_JERICHO_PM_FABRIC;
    }
    rv = soc_dcmn_fabric_pms_add(unit, nof_pms, FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id), 1, quads_in_fsrd, soc_jer_fabric_mdio_address_get, &fabric_user_acc[unit]);
    SOCDNX_IF_ERR_EXIT(rv);

    if (SOC_WARM_BOOT(unit))
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_dynamic_fabric_port_restore(unit));
    }


exit:
    SOCDNX_FUNC_RETURN;
}


static int
soc_jer_portmod_xphy_core_config_update(int unit, int lport, int pport, int phy_idx, uint32_t xphy_addr) {

    int fw_ld_method = 0x1, i;
    uint32_t rx_polarity = 0, tx_polarity = 0;
    uint32_t rx_lane_map = 0, tx_lane_map = 0;
    uint32_t psc_repeater_mode = 0;
    uint32 mask, shift;

    uint8_t gearbox_enable;
    uint8_t pin_compatibility_enable;
    uint8_t phy_mode_reverse;
    int     force_fw_load, is_legacy_phy;
    phymod_firmware_load_method_t fw_load_method;
    phymod_core_access_t core_access;
    phymod_polarity_t  polarity;
    phymod_lane_map_t lane_map;


    SOCDNX_INIT_FUNC_DEFS;

     

    SOCDNX_IF_ERR_EXIT(portmod_xphy_core_access_get(unit, xphy_addr, &core_access, &is_legacy_phy));

    /* soc read for phy device operation mode */
    gearbox_enable = (soc_property_port_get(unit, lport, spn_PHY_GEARBOX_ENABLE, FALSE));
    pin_compatibility_enable = soc_property_port_get(unit, lport, spn_PHY_PIN_COMPATIBILITY_ENABLE, FALSE);
    phy_mode_reverse = soc_property_port_get(unit, lport, spn_PORT_PHY_MODE_REVERSE, 0);
    psc_repeater_mode = soc_property_port_get(unit, lport, spn_PHY_PCS_REPEATER, 0);

    if(gearbox_enable) {
        PHYMOD_INTF_CONFIG_PHY_GEARBOX_ENABLE_SET(&core_access);
    }
    if (pin_compatibility_enable) {
        PHYMOD_INTF_CONFIG_PHY_PIN_COMPATIBILITY_ENABLE_SET(&core_access);
    }
    if (phy_mode_reverse) {
        PHYMOD_INTF_CONFIG_PORT_PHY_MODE_REVERSE_SET(&core_access);
    }
    if (psc_repeater_mode) {
        PHYMOD_INTF_CONFIG_PHY_PSC_REPEATER_MODE_SET(&core_access);
    }

    SOCDNX_IF_ERR_EXIT(portmod_xphy_core_access_set(unit, xphy_addr, &core_access, is_legacy_phy));

    /*lower nibble to represent Force FW load and upper nibble to represent
      load method */
    fw_ld_method = 0x11;
    fw_ld_method = soc_property_port_get(unit, lport, spn_PHY_FORCE_FIRMWARE_LOAD, fw_ld_method);
    switch ((fw_ld_method >> 4) & 0xf) {
        case 0:
            fw_load_method = phymodFirmwareLoadMethodNone;
        break;
        case 1:
            fw_load_method = phymodFirmwareLoadMethodInternal;
        break;
        case 2:
            fw_load_method = phymodFirmwareLoadMethodProgEEPROM;
        break;
        default:
            fw_load_method = phymodFirmwareLoadMethodInternal;
        break;
    }
    
    SOCDNX_IF_ERR_EXIT(portmod_xphy_fw_load_method_set(unit, xphy_addr, fw_load_method));

    switch (fw_ld_method & 0xf) {
        case 0:
            /* skip download */
            force_fw_load = phymodFirmwareLoadSkip;
        break;
        case 1:
            /* force download */
            force_fw_load = phymodFirmwareLoadForce;
        break;
        case 2:
            /* auto download. download firware if two versions are diffirent */
            force_fw_load = phymodFirmwareLoadAuto;
        break;
        default:
            force_fw_load = phymodFirmwareLoadSkip;
        break;
    }
    SOCDNX_IF_ERR_EXIT(portmod_xphy_force_fw_load_set(unit, xphy_addr, force_fw_load));

    /* get the plarity settings for the core. look for the legacy config (logical port based) if 
     * not available then look for a new config mode */
    rx_polarity = soc_property_phy_get (unit, pport, phy_idx, 0, 0, spn_PHY_CHAIN_RX_POLARITY_FLIP_PHYSICAL, 0xFFFFFFFF);
    if (rx_polarity == 0xFFFFFFFF) { 
        rx_polarity = soc_property_port_get(unit, lport, spn_PHY_RX_POLARITY_FLIP, 0);
    }
    tx_polarity = soc_property_phy_get (unit, pport, phy_idx, 0, 0, spn_PHY_CHAIN_TX_POLARITY_FLIP_PHYSICAL, 0xFFFFFFFF);
    if (tx_polarity == 0xFFFFFFFF) {
        tx_polarity =  soc_property_port_get(unit, lport, spn_PHY_TX_POLARITY_FLIP, 0);
    } 
    polarity.rx_polarity = rx_polarity;
    polarity.tx_polarity = tx_polarity;

    SOCDNX_IF_ERR_EXIT(portmod_xphy_polarity_set(unit, xphy_addr, polarity));

    /* get the lane map information */
    rx_lane_map = soc_property_phy_get (unit, pport, phy_idx, 0, 0, spn_PHY_CHAIN_RX_LANE_MAP_PHYSICAL, 0xFFFFFFFF);
    if (rx_lane_map == 0xFFFFFFFF) {
        rx_lane_map = soc_property_phy_get(unit, pport, phy_idx, 0, 0, spn_PHY_RX_LANE_MAP, 0xFFFFFFFF);
    }
    if (rx_lane_map == 0xFFFFFFFF) {
        rx_lane_map = soc_property_port_get(unit, lport, spn_PHY_RX_LANE_MAP, 0x3210);
    }
    /* get the lane map information */
    tx_lane_map = soc_property_phy_get (unit, pport, phy_idx, 0, 0, spn_PHY_CHAIN_TX_LANE_MAP_PHYSICAL, 0xFFFFFFFF);
    if (tx_lane_map == 0xFFFFFFFF) {
        tx_lane_map = soc_property_phy_get(unit, pport, phy_idx, 0, 0, spn_PHY_TX_LANE_MAP, 0xFFFFFFFF);
    }
    if (tx_lane_map == 0xFFFFFFFF) {
        tx_lane_map = soc_property_port_get(unit, lport, spn_PHY_TX_LANE_MAP, 0x3210);
    }   
    /*FIX harcoding the number of lanes to 4 for now 
     * need to figure out the better way to fix this */
    lane_map.num_of_lanes = NUM_OF_LANES_IN_PM;
    mask = 0xf;
    shift = 0;
    for (i = 0; i < NUM_OF_LANES_IN_PM; i++) {
        shift = i*4;
        lane_map.lane_map_rx[i] = ((rx_lane_map >> shift) & mask);
        lane_map.lane_map_tx[i] = ((tx_lane_map >> shift) & mask);
    }    
    SOCDNX_IF_ERR_EXIT(portmod_xphy_lane_map_set(unit, xphy_addr, lane_map));

exit:
    SOCDNX_FUNC_RETURN;

}


STATIC int
soc_jer_portmod_register_external_phys(int unit)
{
    int port_i;
    soc_port_t xphy_logical_port;
    uint32 xphy_mdio_addr = 0, xphy_physical_port = 0, tmp_xphy_physical_port = 0, xphy_gearbox_mode = 0;
    phymod_core_access_t tmp_ext_phy_access; 
    portmod_default_user_access_t* xphy_user_access;
    int iphy = 0, lport = 0, core_index = PORTMOD_CORE_INDEX_INVALID;
    int port_primary = 0, port_offset = 0, port_phy_addr = 0;
    portmod_ext_phy_core_info_t core_info;
    portmod_lane_connection_t lane_connection;
    phymod_ref_clk_t                ref_clk;
    phymod_firmware_load_method_t   fw_load_method;
    uint32 primary_core_num = 0;

    portmod_xphy_lane_connection_t    xphy_lane_connection;
    soc_pbmp_t phy_ports;
    phymod_core_access_t core_access;
    int xphy_idx = PORTMOD_XPHY_EXISTING_IDX;

    SOCDNX_INIT_FUNC_DEFS;
    /*
     *  NIF Exteranl PHY
     */
    /* TBD */
    
    SOCDNX_IF_ERR_EXIT(soc_phy_common_init(unit));
    SOCDNX_IF_ERR_EXIT(soc_phyctrl_software_init(unit));

    for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; ++port_i) {

        xphy_mdio_addr = soc_property_port_get(unit, port_i, spn_PORT_PHY_ADDR, SOC_JERICHO_PORT_PHY_ADDR_INVALID);
        xphy_gearbox_mode = soc_property_port_get(unit, port_i, spn_PHY_GEARBOX_ENABLE, 0);
        if (xphy_mdio_addr == SOC_JERICHO_PORT_PHY_ADDR_INVALID) {
            continue;
        }

        if (IS_IL_PORT(unit,port_i)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("XPHY: no support for ILKN interface")));
        }

        if (IS_SFI_PORT(unit,port_i)) {
            continue;
        }

        xphy_logical_port = port_i;

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, xphy_logical_port, &phy_ports));

        xphy_user_access = sal_alloc(sizeof(portmod_default_user_access_t), "XPHY_USER_ACCESS");
        if(xphy_user_access == NULL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate user access memory for port %d"), xphy_logical_port));
        }
        SOC_JER_NIF_XPHY_USER_ACCESS(unit, xphy_logical_port) = xphy_user_access;

        SOCDNX_IF_ERR_EXIT(portmod_default_user_access_t_init(unit, xphy_user_access));
        PORTMOD_USER_ACCESS_FW_LOAD_REVERSE_SET(xphy_user_access);
        xphy_user_access->unit = unit;
        xphy_user_access->mutex = sal_mutex_create("core mutex");
        if(xphy_user_access->mutex == NULL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate mutex for external phy of port %d."), xphy_logical_port));
        }

        phymod_core_access_t_init(&tmp_ext_phy_access);
        tmp_ext_phy_access.access.user_acc = SOC_JER_NIF_XPHY_USER_ACCESS(unit, xphy_logical_port);
        tmp_ext_phy_access.access.bus = &portmod_ext_default_bus;
        tmp_ext_phy_access.access.addr = xphy_mdio_addr;
        tmp_ext_phy_access.type = phymodDispatchTypeCount; /* Make sure it is invalid. */
        /*  xphy_core_info. polarity*/
        sal_memcpy(&core_access, &tmp_ext_phy_access, sizeof(tmp_ext_phy_access));
        SOCDNX_IF_ERR_EXIT(portmod_xphy_add(unit, xphy_mdio_addr, &core_access, &xphy_idx));

        SOC_PBMP_ITER(phy_ports, tmp_xphy_physical_port) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, tmp_xphy_physical_port, &xphy_physical_port)));
            portmod_xphy_lane_connection_t_init(unit, &xphy_lane_connection);
            xphy_lane_connection.xphy_id = xphy_mdio_addr;
            xphy_lane_connection.ss_lane_mask = 0x1 << ((xphy_physical_port-1) % 4);
            /*This only support 2x25G to 4x10G gearbox*/
            if (xphy_gearbox_mode) {
                xphy_lane_connection.ls_lane_mask = 0x3 << (((xphy_physical_port-1) % 4) * 2);
            } else {
                xphy_lane_connection.ls_lane_mask = xphy_lane_connection.ss_lane_mask;
            }
            SOCDNX_IF_ERR_EXIT(portmod_xphy_lane_attach(unit, xphy_physical_port, 1, &xphy_lane_connection));
        }

        /* update the value from config */
        ref_clk = phymodRefClk156Mhz;
        fw_load_method = phymodFirmwareLoadMethodInternal;
        primary_core_num = xphy_mdio_addr;
        
        SOCDNX_IF_ERR_EXIT(portmod_xphy_ref_clk_set(unit, xphy_mdio_addr, ref_clk));
        SOCDNX_IF_ERR_EXIT(portmod_xphy_fw_load_method_set(unit, xphy_mdio_addr, fw_load_method));
        SOCDNX_IF_ERR_EXIT(portmod_xphy_primary_core_num_set(unit, xphy_mdio_addr, primary_core_num));

        /* get xphy core information from SOC proberty */
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_xphy_core_config_update(unit, xphy_logical_port, xphy_physical_port, \
            1, xphy_mdio_addr));

    }

    /*
     *  Fabric Exteranl PHY
     */
    SOCDNX_IF_ERR_EXIT(portmod_ext_phy_core_info_t_init(unit, &core_info));
    SOCDNX_IF_ERR_EXIT(portmod_lane_connection_t_init(unit, &lane_connection));       
    for (iphy = 0; iphy < SOC_MAX_NUM_PORTS; iphy++)
    {
        lport = SOC_INFO(unit).port_p2l_mapping[iphy];
        if (lport <= 0)
        {
            continue;
        }
        if(IS_SFI_PORT(unit, lport)) {
            port_primary = SOC_INFO(unit).port_l2pp_mapping[lport];
            port_offset = SOC_INFO(unit).port_l2po_mapping[lport];
            port_phy_addr = SOC_INFO(unit).port_l2pa_mapping[port_primary];
            if ((PORTMOD_PRIMARY_PORT_INVALID != port_primary) && \
                (PORTMOD_PORT_OFFSET_INVALID != port_offset)){
                core_index = port_primary;
                SOCDNX_IF_ERR_EXIT(soc_dcmn_external_phy_core_access_get(unit, port_phy_addr, &core_info.core_access.access));
                SOCDNX_IF_ERR_EXIT(portmod_phychain_ext_phy_info_set(unit, 1, core_index, &core_info));
                lane_connection.core_index = core_index;
                lane_connection.lane_index = port_offset;
                SOCDNX_IF_ERR_EXIT(portmod_ext_phy_lane_attach(unit, iphy, 1, &lane_connection));
            }
        }
                
    }

exit:
    if(SOCDNX_FUNC_ERROR) {
        for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; ++port_i) {
            if (SOC_JER_NIF_XPHY_USER_ACCESS(unit, port_i) != NULL) {
                xphy_user_access = (portmod_default_user_access_t*)(SOC_JER_NIF_XPHY_USER_ACCESS(unit, port_i));
                if(xphy_user_access->mutex != NULL) {
                    sal_mutex_destroy(xphy_user_access->mutex);
                }
                sal_free(xphy_user_access);
                SOC_JER_NIF_XPHY_USER_ACCESS(unit, port_i) = NULL;
            }
        }
    }
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_tx_stop_data_to_pm(int unit)
{
    int i, inst, is_nbil_en;
    uint64 reg64_val;
    uint32 nof_pms_per_nbi    = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    uint32 nof_instances_nbil = SOC_DPP_DEFS_GET(unit, nof_instances_nbil);

    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_WARM_BOOT(unit)) {
        if (SOC_IS_QUX(unit)) {
            /* QUX only 1 NBI block */
            for (i = 0; i < nof_pms_per_nbi; ++i) {
                SOCDNX_IF_ERR_EXIT(READ_NIF_TX_QMLF_CONFIGr(unit, i, &reg64_val)); 
                soc_reg64_field32_set(unit, NIF_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_0_QMLF_Nf, 1);
                soc_reg64_field32_set(unit, NIF_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_1_QMLF_Nf, 1);
                soc_reg64_field32_set(unit, NIF_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_2_QMLF_Nf, 1);
                soc_reg64_field32_set(unit, NIF_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_3_QMLF_Nf, 1);
                SOCDNX_IF_ERR_EXIT(WRITE_NIF_TX_QMLF_CONFIGr(unit, i, reg64_val));
            }
        } else {
            for (i = 0; i < nof_pms_per_nbi; ++i) {
                SOCDNX_IF_ERR_EXIT(READ_NBIH_TX_QMLF_CONFIGr(unit, i, &reg64_val)); 
                soc_reg64_field32_set(unit, NBIH_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_0_QMLF_Nf, 1);
                soc_reg64_field32_set(unit, NBIH_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_1_QMLF_Nf, 1);
                soc_reg64_field32_set(unit, NBIH_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_2_QMLF_Nf, 1);
                soc_reg64_field32_set(unit, NBIH_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_3_QMLF_Nf, 1);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_TX_QMLF_CONFIGr(unit, i, reg64_val));

                is_nbil_en = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "is_nbil_valid", 1);
                if (is_nbil_en) {
                    for (inst = 0; inst < nof_instances_nbil; ++inst) {
                        SOCDNX_IF_ERR_EXIT(READ_NBIL_TX_QMLF_CONFIGr(unit, inst, i, &reg64_val)); 
                        soc_reg64_field32_set(unit, NBIL_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_0_QMLF_Nf, 1);
                        soc_reg64_field32_set(unit, NBIL_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_1_QMLF_Nf, 1);
                        soc_reg64_field32_set(unit, NBIL_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_2_QMLF_Nf, 1);
                        soc_reg64_field32_set(unit, NBIL_TX_QMLF_CONFIGr, &reg64_val, TX_STOP_DATA_TO_PORT_MACRO_MLF_3_QMLF_Nf, 1);
                        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_TX_QMLF_CONFIGr(unit, inst, i, reg64_val));
                    }
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_config_start_tx_thrs(int unit)
{
    int i, inst, is_nbil_en, mac_fifo_start_tx_thrs;
    uint64 reg64_val;
    uint32 nof_pms_per_nbi    = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    uint32 nof_instances_nbil = SOC_DPP_DEFS_GET(unit, nof_instances_nbil);

    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_WARM_BOOT(unit)) {
        mac_fifo_start_tx_thrs = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "mac_fifo_start_tx_thrs", 0x4);
        if (SOC_IS_QUX(unit)) {
            for (i = 0; i < nof_pms_per_nbi; ++i) {
                SOCDNX_IF_ERR_EXIT(READ_NIF_TX_QMLF_CONFIGr(unit, i, &reg64_val));
                soc_reg64_field32_set(unit, NIF_TX_QMLF_CONFIGr, &reg64_val, TX_START_TX_THRESHOLD_QMLF_Nf, mac_fifo_start_tx_thrs);
                SOCDNX_IF_ERR_EXIT(WRITE_NIF_TX_QMLF_CONFIGr(unit, i, reg64_val));
            }
        } else {
            for (i = 0; i < nof_pms_per_nbi; ++i) {
                SOCDNX_IF_ERR_EXIT(READ_NBIH_TX_QMLF_CONFIGr(unit, i, &reg64_val));
                soc_reg64_field32_set(unit, NBIH_TX_QMLF_CONFIGr, &reg64_val, TX_START_TX_THRESHOLD_QMLF_Nf, mac_fifo_start_tx_thrs);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_TX_QMLF_CONFIGr(unit, i, reg64_val));

                is_nbil_en = soc_property_suffix_num_get(unit, -1, spn_CUSTOM_FEATURE, "is_nbil_valid", 1);
                if (is_nbil_en) {
                    for (inst = 0; inst < nof_instances_nbil; ++inst) {
                        SOCDNX_IF_ERR_EXIT(READ_NBIL_TX_QMLF_CONFIGr(unit, inst, i, &reg64_val));
                        soc_reg64_field32_set(unit, NBIL_TX_QMLF_CONFIGr, &reg64_val, TX_START_TX_THRESHOLD_QMLF_Nf, mac_fifo_start_tx_thrs);
                        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_TX_QMLF_CONFIGr(unit, inst, i, reg64_val));
                    }
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_init(int unit)
{
    portmod_pm_instances_t *pm_types_and_instances = NULL;
    int rv, pms_instances_arr_len = 0;
    uint32 nof_pm4x25       = SOC_DPP_DEFS_GET(unit, nof_pm4x25);
    uint32 nof_pm4x10       = SOC_DPP_DEFS_GET(unit, nof_pm4x10);
    soc_reg_above_64_val_t reg_above_64_zero_val;
    uint32 reg_zero_val = 0;
#ifdef PORTMOD_PM4X2P5_SUPPORT
    uint32 nof_pm4x2p5    = SOC_DPP_DEFS_GET(unit, nof_pm4x2p5);
#endif /* PORTMOD_PM4X2P5_SUPPORT */

    SOCDNX_INIT_FUNC_DEFS;

    rv = MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_soc_pm_instances_get,(unit, &pm_types_and_instances, &pms_instances_arr_len));
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(soc_jer_tx_stop_data_to_pm(unit));

    SOCDNX_IF_ERR_EXIT(soc_jer_config_start_tx_thrs(unit));

    SOCDNX_IF_ERR_EXIT(portmod_create(unit, 0, SOC_MAX_NUM_PORTS, SOC_MAX_NUM_PORTS, pms_instances_arr_len, pm_types_and_instances));

    if (nof_pm4x25 > 0) {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_pmh_init(unit));
    }
    if (nof_pm4x10 > 0) {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_pml_init(unit));
    }
#ifdef PORTMOD_PM4X2P5_SUPPORT
    if (nof_pm4x2p5 > 0) {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_pmx_init(unit));
    }
#endif /* PORTMOD_PM4X2P5_SUPPORT */

    SOCDNX_IF_ERR_EXIT(soc_jer_portmod_fabric_init(unit));

    if (!(SOC_DPP_CONFIG(unit)->emulation_system)) {
        /* Not working in emulation for some reason. If ILKN is needed in
           emulation, this needs some debugging */
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_ilkn_init, (unit)));
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_portmod_register_external_phys(unit));

    /*Clear all nif priority both for ETH and ILKN*/
    if(!SOC_WARM_BOOT(unit)){
		SOC_REG_ABOVE_64_CLEAR(reg_above_64_zero_val);
        if(SOC_IS_QUX(unit)) {
            /*Registers only for QUX*/
            SOCDNX_IF_ERR_EXIT( soc_reg_above_64_set(unit, NIF_RX_REQ_PIPE_0_LOW_ENr, REG_PORT_ANY, 0, reg_above_64_zero_val));
            SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, NIF_RX_REQ_PIPE_0_HIGH_ENr, REG_PORT_ANY, 0, reg_zero_val));
            /*QUX do not support TDM*/
        } else {
            /*Registry cleaning for QAX,QMX and Jericho*/
            SOCDNX_IF_ERR_EXIT( soc_reg_above_64_set(unit, NBIH_RX_REQ_PIPE_0_LOW_ENr, REG_PORT_ANY, 0, reg_above_64_zero_val));
            SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, NBIH_RX_REQ_PIPE_0_HIGH_ENr, REG_PORT_ANY, 0, reg_zero_val));
            SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, NBIH_RX_REQ_PIPE_0_TDM_ENr, REG_PORT_ANY, 0, reg_zero_val));

        }

        if (SOC_DPP_DEFS_GET(unit, nof_cores)>1) {
            /*Registry cleaning for Jericho and QMX*/
            SOCDNX_IF_ERR_EXIT( soc_reg_above_64_set(unit, NBIH_RX_REQ_PIPE_1_LOW_ENr, REG_PORT_ANY, 0, reg_above_64_zero_val));
            SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, NBIH_RX_REQ_PIPE_1_HIGH_ENr, REG_PORT_ANY, 0, reg_zero_val));
            SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, NBIH_RX_REQ_PIPE_1_TDM_ENr, REG_PORT_ANY, 0, reg_zero_val));
        }

    }

exit:
    SOCDNX_FUNC_RETURN;
}

int 
soc_jer_portmod_port_quad_get(int unit, soc_port_t port, soc_pbmp_t* quad_bmp)
{
    soc_pbmp_t phy_ports, phys;
    int phy;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*quad_bmp);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phy_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phy_ports, &phys));

    SOC_PBMP_ITER(phys, phy){
        SOC_PBMP_PORT_ADD(*quad_bmp, (phy - 1) / NUM_OF_LANES_IN_PM);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_port_ilkn_bypass_interface_enable
 * Purpose:
 *      enable/disable ILKN bypass interface.
 * Parameters:
 *      unit - Device unit number.
 *      port - Port number.
 *      enable - Enable / Disable request
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      If ilkn_first_packet_sw_bypass SOC property is set - this function manipulate only the ILKN Tx bypass interface and let
 *      soc_jer_port_ilkn_bypass_interface_rx_check_and_enable below handle the Rx interface.
 *      An exception to the above are the KBP ports. In KBP ports, this function enables/disables both Rx and Tx bypass interface,
 *      regardless to the ilkn_first_packet_sw_bypass SOC property.
 */

int
soc_jer_port_ilkn_bypass_interface_enable(int unit, int port, int enable) {

    soc_pbmp_t phys, phy_lanes;
    soc_port_t phy, reg_port;
    soc_reg_t reg;
    int shift;
    int first_packet_sw_bypass;
    uint32 offset, rst_lanes = 0, reg_val, flags=0, sw_db_flags;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &phy_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &sw_db_flags));
    first_packet_sw_bypass = SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn_first_packet_sw_bypass;

    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4);


    if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
        /*In ILKN over NIF, FMAC reset is the equivilant for ilkn_bypass_interface*/
        flags = 0;
        PORTMOD_PORT_ENABLE_MAC_SET(flags);
        /*
         * if we are not in first_packet_sw_bypass - we always manipulate the Rx side (NBIx_NIF_PM_ILKN_RX_RSTN)
         * if we are in first_packet_sw_bypass, we manipulate the Rx side only at disable case. This is since:
         * 1. Sometimes we want to close ilkn_bypass_interface without getting the phy permanently down, for example, when changing phy configuration.
         * 2. linkscan is disabled before port is disabled so we will not get the linkscan event that calls
         *    soc_jer_port_ilkn_bypass_interface_rx_check_and_enable to close the Rx interface.
         * In KBP ports we never activate the ILKN First Packet SW Bypass, even if the SOC property was set.
         */
        if ((first_packet_sw_bypass) && (enable) && (!SOC_PORT_IS_ELK_INTERFACE(sw_db_flags))){
            PORTMOD_PORT_ENABLE_TX_SET(flags);
        }
        SOCDNX_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, enable));
    }else{
        SOC_PBMP_ITER(phy_lanes, phy){
            if (phy == 0 /*Should be always false since NIF is 1 based*/)
            {
                continue;
            }
            shift = (phy - 1) % JER_NIF_ILKN_MAX_NOF_LANES;
            rst_lanes |= (1 << shift);
        }
        /*
         * if we are not in first_packet_sw_bypass - we always manipulate the Rx side (NBIx_NIF_PM_ILKN_RX_RSTN)
         * if we are in first_packet_sw_bypass, we manipulate the Rx side only at disable case. This is since:
         * 1. Sometimes we want to close ilkn_bypass_interface without getting the phy permanently down, for example, when changing phy configuration.
         * 2. linkscan is disabled before port is disabled so we will not get the linkscan event that calls
         *    soc_jer_port_ilkn_bypass_interface_rx_check_and_enable to close the Rx interface.
         * In KBP ports we never activate the ILKN First Packet SW Bypass, even if the SOC property was set.
         */
        if ((!first_packet_sw_bypass) || (!enable) || SOC_PORT_IS_ELK_INTERFACE(sw_db_flags)){
            reg = (offset < 2) ? NBIH_NIF_PM_ILKN_RX_RSTNr : NBIL_NIF_PM_ILKN_RX_RSTNr;
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
            reg_val = (enable) ? (reg_val | rst_lanes) : (reg_val & (~rst_lanes));
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));
        }

        reg = (offset < 2) ? NBIH_NIF_PM_ILKN_TX_RSTNr : NBIL_NIF_PM_ILKN_TX_RSTNr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
        reg_val = (enable) ? (reg_val | rst_lanes) : (reg_val & (~rst_lanes));
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));

    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_port_ilkn_bypass_interface_rx_check_and_enable
 * Purpose:
 *      SW bypass for first packet issue. Invoked only when ilkn_first_packet_sw_bypass SOC property is set. In that case:
 *      1. The normal soc_jer_port_ilkn_bypass_interface_enable doesn't enable/disable the ILKN Rx bypass interface.
 *      2. Instead, this functions is called periodically from SW linkscan mechanism and enable/disable the ILKN Rx bypass interface
 *         according to the logic that described in the function boddy.
 * Parameters:
 *      unit - Device unit number.
 *      port - Port number.
 * Returns:
 *      BCM_E_NONE
 *      BCM_E_XXX
 * Notes:
 *      Requires SW linkscan to be enabled and ilkn_first_packet_sw_bypass SOC property to be set.
 */

soc_error_t
soc_jer_port_ilkn_bypass_interface_rx_check_and_enable(int unit, int port) {

    soc_pbmp_t phys, phy_lanes;
    soc_port_t phy, reg_port;
    soc_reg_t reg = INVALIDr, tsc_reg = INVALIDr;
    int shift, flags = 0;
    uint32 offset, rst_lanes = 0, reg_val, tx_reg_val;
    soc_field_t field[] = {TSC_LANE_STATUS_0__Nf,TSC_LANE_STATUS_1__Nf,TSC_LANE_STATUS_2__Nf,TSC_LANE_STATUS_3__Nf};
    int temp_phy_status, phy_status, current_rx_status, user_request, need_to_set, quad_idx, lane_idx;
    int fsrd_idx, quad_idx_inside_fsrd, sync_status, rx_lock, lane_sync_status, lane_rx_lock;
    int acc_phy_status, phy_link_is_up;
    int phy_was_checked = FALSE;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &phy_lanes));

    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4);


    if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
        phy_link_is_up = JER_FABRIC_SRD_LINK_IS_UP;
    }else{
        tsc_reg = (offset < 2) ? NBIH_NIF_TSC_LANE_STATUSr : NBIL_NIF_TSC_LANE_STATUSr;
        phy_link_is_up = JER_NIF_TSC_LINK_IS_UP;
    }
    acc_phy_status = phy_link_is_up;

    /*
    We have 3 inputs:
    1. User request for the interface, has can be seen from Tx interface register (called user_request).
    2. Phy status as can be seen from NBIx_NIF_TSC_LANE_STATUS register (called phy_status).
    3. Current Rx status as can be seen from Rx interface register (called current_rx_status).

    need_to_set = user_request && phy_status.
    We are setting it only if need_to_set != current_rx_status
    */

    SOC_PBMP_ITER(phy_lanes, phy){
        if (phy == 0 /*Should be always false since NIF is 1 based*/)
        {
            continue;
        }
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            shift = (phy - SOC_DPP_DEFS_GET(unit,first_sfi_phy_port)) % JER_NIF_ILKN_MAX_NOF_LANES;
            if (SOC_IS_QMX(unit)){ /*For QMX, ILKN over fabric starts at quad 2, i.e. lane 8)*/
                shift += 8;
            }
        }else
        {
            shift = (phy - 1) % JER_NIF_ILKN_MAX_NOF_LANES;
        }
        rst_lanes |= (1 << shift);

        /*Getting lane current status*/
        lane_idx = shift % NUM_OF_LANES_IN_PM;
        quad_idx = shift / NUM_OF_LANES_IN_PM;
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            fsrd_idx = shift/(SOC_DPP_DEFS_GET(unit, nof_links_in_fsrd));                                  /* fsrd id */
            quad_idx_inside_fsrd = quad_idx % (SOC_DPP_DEFS_GET(unit, nof_quads_in_fsrd));                 /* quad id inside fsrd */
            SOCDNX_IF_ERR_EXIT(READ_FSRD_SRD_QUAD_STATUSr(unit, fsrd_idx, quad_idx_inside_fsrd, &reg_val));
            sync_status = soc_reg_field_get(unit, FSRD_SRD_QUAD_STATUSr, reg_val, SRD_QUAD_N_SYNC_STATUSf);
            rx_lock = soc_reg_field_get(unit, FSRD_SRD_QUAD_STATUSr, reg_val, SRD_QUAD_N_RX_LOCKf);
            lane_sync_status = (sync_status & (1 << lane_idx)) ? JER_FABRIC_SRD_QUAD_STATUS_REG_SYNC_STATUS_FLAG : 0;
            lane_rx_lock = (rx_lock & (1 << lane_idx)) ? JER_FABRIC_SRD_QUAD_STATUS_REG_RX_LOCK_FLAG : 0;
            temp_phy_status = lane_sync_status | lane_rx_lock;
        }else{
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, tsc_reg, reg_port, quad_idx, &reg_val));
            temp_phy_status =  soc_reg_field_get(unit, tsc_reg, reg_val, field[lane_idx]);
        }
        acc_phy_status &= temp_phy_status; /*it is enough that one phy will be down to decide that phy is down*/
        phy_was_checked = TRUE;
    }
    /*Getting phy status*/
    phy_status = (acc_phy_status == phy_link_is_up) && phy_was_checked;

    /*getting user_request - according to Tx*/
    if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
        flags = 0;
        PORTMOD_PORT_ENABLE_MAC_SET(flags);
        PORTMOD_PORT_ENABLE_TX_SET(flags);
        SOCDNX_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, &user_request));
    }else{
        reg = (offset < 2) ? NBIH_NIF_PM_ILKN_TX_RSTNr : NBIL_NIF_PM_ILKN_TX_RSTNr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &tx_reg_val));
        user_request = ((rst_lanes & tx_reg_val) != 0);
    }
    need_to_set = user_request && phy_status;

    /*getting interface status according to Rx (current_rx_status)*/
    if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
        flags = 0;
        PORTMOD_PORT_ENABLE_MAC_SET(flags);
        PORTMOD_PORT_ENABLE_RX_SET(flags);
        SOCDNX_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, &current_rx_status));
    }else{
        reg = (offset < 2) ? NBIH_NIF_PM_ILKN_RX_RSTNr : NBIL_NIF_PM_ILKN_RX_RSTNr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
        current_rx_status = ((rst_lanes & reg_val) == rst_lanes);
    }

    /*if need_to_set is up but interface is down - enable the bypass_interface_rx*/
    /*if need_to_set is down but interface is up - disable the bypass_interface_rx*/
    if (current_rx_status != need_to_set){
        LOG_VERBOSE(BSL_LS_SOC_PORT, (BSL_META_U(unit, "ilkn_bypass_interface_rx_check_and_enable: port = %d, phy_status = %d, user_request = %d\n"), port, phy_status, user_request));
        LOG_VERBOSE(BSL_LS_SOC_PORT, (BSL_META_U(unit, "    acc_phy_status = %d, current_rx_status = %d, need_to_set = %d\n"), acc_phy_status, current_rx_status, need_to_set));
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            LOG_VERBOSE(BSL_LS_SOC_PORT, (BSL_META_U(unit, "    rst_lanes = %x\n"), rst_lanes));
            /*In ILKN over NIF, FMAC reset is the equivilant for ilkn_bypass_interface*/
            flags = 0;
            PORTMOD_PORT_ENABLE_MAC_SET(flags);
            PORTMOD_PORT_ENABLE_RX_SET(flags);
            SOCDNX_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, need_to_set));
        }else{
            LOG_VERBOSE(BSL_LS_SOC_PORT, (BSL_META_U(unit, "    rst_lanes = %x, rx_reg_val (before action taken) = %x, tx_reg_val = %x\n"), rst_lanes, reg_val, tx_reg_val));
            reg_val = (need_to_set) ? (reg_val | rst_lanes) : (reg_val & (~rst_lanes));
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC soc_port_if_t soc_jer_get_interface_type(int unit, soc_port_t port, uint32 defl)
{
    soc_port_if_t interface_type;
    char *interface_type_as_str = NULL;
    char *interface_types_names[] = SOC_PORT_IF_NAMES_INITIALIZER;
    int i, num_of_interfaces;

    interface_type_as_str = soc_property_port_get_str(unit, port, spn_SERDES_IF_TYPE);
    if (interface_type_as_str != NULL){
        num_of_interfaces = sizeof(interface_types_names)/sizeof(*interface_types_names);
        for(i = 0, interface_type = _SHR_PORT_IF_NOCXN; i < num_of_interfaces; ++i, ++interface_type){
            if(sal_strcasecmp(interface_type_as_str, interface_types_names[i]) == 0){
                break;
            }
        }
        if(i == num_of_interfaces){
            interface_type = soc_property_port_get(unit, port, spn_SERDES_IF_TYPE, defl);
        }
    }
    else{
            interface_type = soc_property_port_get(unit, port, spn_SERDES_IF_TYPE, defl);
    }
    return interface_type;
}

int
soc_jer_portmod_post_init(int unit, soc_pbmp_t* ports)
{
    int val, rv;
    uint32 is_master;
    uint32 runt_pad;
    soc_port_t port;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    if (!SOC_WARM_BOOT(unit)) {
        SOC_PBMP_ITER(*ports, port) {

            interface_type = soc_jer_get_interface_type(unit, port, SOC_PORT_IF_NULL);

            if (interface_type > SOC_PORT_IF_NULL) {
                SOCDNX_IF_ERR_EXIT(soc_jer_portmod_port_interface_set(unit,  port, interface_type)); 
            }
            
            if (!IS_SFI_PORT(unit, port)) {

                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_master_get(unit, port, &is_master));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));

                if (interface_type != SOC_PORT_IF_ILKN) {
                    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_runt_pad_get(unit, port, &runt_pad));
                    SOCDNX_IF_ERR_EXIT(portmod_port_pad_size_set(unit, port, runt_pad));
                }

                if(is_master) {

                    /* speed config */
                    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_get(unit, port, &val));
                    if(-1 != val && 42000 != val) {
                        if(val == 0) {
                            val = SOC_INFO(unit).port_speed_max[port];
                        }
                        rv = soc_jer_portmod_port_speed_set(unit, port, val);
                        SOCDNX_IF_ERR_EXIT(rv);
                    }
                    
                    if (interface_type != SOC_PORT_IF_ILKN) {

                        /* AN config */
                        val = soc_property_port_get(unit, port, spn_PORT_INIT_AUTONEG, -1);
                        if(val != -1) {
                            rv = soc_jer_portmod_autoneg_set(unit, port, val);
                            SOCDNX_IF_ERR_EXIT(rv);
                        }  

                        /* MAX RX Size */
                        val = soc_property_get(unit, spn_BCM_STAT_JUMBO, -1);
                        if(val != -1) {
                            rv = portmod_port_cntmaxsize_set(unit, port, val);
                            SOCDNX_IF_ERR_EXIT(rv);    
                        }

                    } else {
                        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 1)));
                        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 1));
                    }
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_deinit(int unit)
{
    int port_i;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_destroy(unit));

    if(pm4x25_user_acc[unit] != NULL) {
        sal_free(pm4x25_user_acc[unit]);
        pm4x25_user_acc[unit] = NULL;
    }

    if(pm4x10_user_acc[unit] != NULL) {
        sal_free(pm4x10_user_acc[unit]);
        pm4x10_user_acc[unit] = NULL;
    }

    if(pm4x10q_user_acc[unit] != NULL) {
        sal_free(pm4x10q_user_acc[unit]);
        pm4x10q_user_acc[unit] = NULL;
    }
    
    if(fabric_user_acc[unit] != NULL) {
        sal_free(fabric_user_acc[unit]);
        fabric_user_acc[unit] = NULL;
    }

    for (port_i = 0; port_i < SOC_MAX_NUM_PORTS; ++port_i) {
        if(SOC_JER_NIF_XPHY_USER_ACCESS(unit, port_i) != NULL) {
            sal_free(SOC_JER_NIF_XPHY_USER_ACCESS(unit, port_i));
            SOC_JER_NIF_XPHY_USER_ACCESS(unit, port_i) = NULL;
        }
    }
#ifdef PORTMOD_PM4X2P5_SUPPORT
    if(pm4x2p5_user_acc[unit] != NULL) {
        sal_free(pm4x2p5_user_acc[unit]);
        pm4x2p5_user_acc[unit] = NULL;
    }
#endif /* PORTMOD_PM4X2P5_SUPPORT */

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
_soc_jer_portmod_speed_to_if_config(int unit, soc_port_t port, int speed, portmod_port_interface_config_t* config)
{
    uint32 is_hg, flags, num_lanes;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_t_init(unit, config));

    config->speed = speed;
    config->max_speed = SOC_INFO(unit).port_speed_max[port];

    /*NIF  config*/
    if (!SOC_PBMP_MEMBER(PBMP_SFI_ALL(unit), port))
    {
        
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
        config->interface = interface_type;

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
        config->port_num_lanes = num_lanes;

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_hg_get(unit, port, &is_hg));
        if (is_hg) {
            PHYMOD_INTF_MODES_HIGIG_SET(config);
            config->encap_mode = SOC_ENCAP_HIGIG2;
        }

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));

        if (SOC_PORT_IS_SCRAMBLER(flags)) {
            PHYMOD_INTF_MODES_SCR_SET(config);
        }

        if (SOC_PORT_IS_FIBER(flags)) {
            PHYMOD_INTF_MODES_FIBER_SET(config);
        }

        if (SOC_PORT_IS_COPPER(flags)) {
            PHYMOD_INTF_MODES_COPPER_SET(config);
        }

    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC soc_error_t
_soc_jer_port_fabric_config_get(int unit, soc_port_t port, int is_init_sequence, dcmn_port_fabric_init_config_t* port_config)
{
    SOCDNX_INIT_FUNC_DEFS;

    /* Set defaults */
    port_config->pcs = PORTMOD_PCS_64B66B_RS_FEC;
    port_config->speed = 25000;
    port_config->cl72 = 1;

    /* Update according to soc properties */
    if(is_init_sequence) {
        SOCDNX_IF_ERR_EXIT(soc_dcmn_port_config_get(unit, port, port_config));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_jer_port_fabric_clk_freq_init(int unit, soc_pbmp_t pbmp)
{
    soc_port_t port;
    int index;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_ITER(pbmp, port)
    {
        index = SOC_DPP_FABRIC_PORT_TO_LINK(unit, port) / (SOC_DPP_DEFS_GET(unit, nof_fabric_links) / 2);
        if((soc_dcmn_init_serdes_ref_clock_125 == SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_fabric_out[index])) {
           SOC_INFO(unit).port_refclk_int[port] = 125;
        } else {
           SOC_INFO(unit).port_refclk_int[port] = 156;
        }
    }
    

    SOCDNX_FUNC_RETURN;
}

int 
soc_jer_nif_ilkn_pbmp_get(int unit, soc_port_t port, uint32 ilkn_id, soc_pbmp_t* phys , soc_pbmp_t* src_pbmp)
{
    int i, lanes, phy_offset;
    char* propval;
    char* propkey;
    int first_phy;
    soc_pbmp_t bm, phy_pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(phy_pbmp);

    propkey = spn_ILKN_LANES;
    propval = soc_property_suffix_num_str_get(unit, ilkn_id, propkey, "");
    /* src_pbmp is passed when port is added dynamically, otherwise the soc property is read. */
    if(src_pbmp != NULL) {
        BCM_PBMP_ASSIGN(bm,*src_pbmp);
    } else if (propval != NULL) {
        if (_shr_pbmp_decode(propval, &bm) < 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("failed to decode (\"%s\") for %s"), propval, propkey)); 
        }
    }

    if((propval != NULL) || (src_pbmp != NULL)) {
        first_phy = SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, ilkn_id) ? SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit) - 1: (ilkn_id / 2) * JER_NIF_ILKN_MAX_NOF_LANES;

        SOC_PBMP_COUNT(bm, lanes);
        if ((ilkn_id & 1) && lanes > 12) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ILKN%d can't have more than 12 lanes"), ilkn_id)); 
        }
        if ((SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, ilkn_id)) && 
            ((first_phy + lanes) > (SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit) + (SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit))))) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ILKN%d lanes are out of range"), ilkn_id)); 
        }
        SOC_PBMP_ITER(bm, i) {
            if(i >= JER_NIF_ILKN_MAX_NOF_LANES) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("lanes %d is out of range"), i));       
            }
            SOC_PBMP_PORT_ADD(phy_pbmp, first_phy + i + 1);
        }

    } else {
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, ilkn_id)) {
            /*Fabric over ILKN*/
            phy_offset = (ilkn_id == 5) ? 12 : 0;
            first_phy = SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit) + phy_offset;
        } else {
            first_phy = ilkn_id * 12 + 1;
        }
        lanes = soc_property_port_get(unit, ilkn_id, spn_ILKN_NUM_LANES, 12);

        if ((ilkn_id & 1) && lanes > 12) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ILKN%d can't have more than 12 lanes"), ilkn_id)); 
        }
        if ((SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, ilkn_id)) && 
            ((first_phy + lanes) > (SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit) + (SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit))))) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ILKN%d lanes are out of range"), ilkn_id)); 
        }
        for(i = 0 ; i < lanes; i++) {
            SOC_PBMP_PORT_ADD(phy_pbmp, first_phy + i);
        }
    }
    SOC_PBMP_ASSIGN(*phys, phy_pbmp);

exit:
    SOCDNX_FUNC_RETURN;
}



int 
soc_jer_nif_ilkn_over_fabric_pbmp_get(int unit, soc_pbmp_t* phys)
{
    int i, lanes, test_lanes;
    char* propval;
    char* propkey;
    uint32 value;
    int first_port;
    soc_pbmp_t bm, phy_pbmp, test_bmp;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(phy_pbmp);
    SOC_PBMP_CLEAR(test_bmp);

    SOCDNX_IF_ERR_EXIT(dcmn_property_suffix_num_get(unit, 5, spn_USE_FABRIC_LINKS_FOR_ILKN_NIF, "bmp", 0,&value));
    propkey = spn_USE_FABRIC_LINKS_FOR_ILKN_NIF;
    propval = soc_property_suffix_num_str_get(unit, 0, propkey, "bmp");

    if(propval != NULL) {
        first_port =  FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id);

        if (_shr_pbmp_decode(propval, &bm) < 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("failed to decode (\"%s\") for %s"), propval, propkey)); 
        }
        SOC_PBMP_COUNT(bm, lanes);
        if (lanes > (SOC_IS_QMX(unit) ? SOC_JER_NIF_NOF_ILKN_OVER_FABRIC_LINKS_QMX : SOC_JER_NIF_NOF_ILKN_OVER_FABRIC_LINKS_JERICHO)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ILKN over fabric can't have more than %d lanes"),SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit))); 
        }

        SOC_PBMP_PORTS_RANGE_ADD(test_bmp, 0, (SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit)));
        SOC_PBMP_NEGATE(test_bmp, test_bmp);
        SOC_PBMP_AND(test_bmp, bm);
        SOC_PBMP_COUNT(test_bmp, test_lanes);
        if (test_lanes) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("ILKN over fabric lanes are outside valid range"))); 
        }

        SOC_PBMP_ITER(bm, i) {
            SOC_PBMP_PORT_ADD(phy_pbmp, first_port + i);
        }

    } else {
        
        /*Fabric over ILKN*/
        first_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id);
        lanes = SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit);
        
        for(i = 0 ; i < lanes; i++) {
            SOC_PBMP_PORT_ADD(phy_pbmp, first_port + i);
        }
    }
    SOC_PBMP_ASSIGN(*phys, phy_pbmp);

exit:
    SOCDNX_FUNC_RETURN;
}


STATIC int
soc_jer_port_ilkn_config_get(int unit, soc_port_t port, portmod_port_add_info_t* add_info)
{
    soc_pbmp_t phys;
    int phy_i;
    uint32 offset, flags;
    ARAD_PORTS_ILKN_CONFIG *ilkn_config;
    uint32 oob_if = 0;
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));

    ilkn_config = &SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn[offset];
    add_info->ilkn_core_id = (offset & 1);
    add_info->ilkn_nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];
    
    add_info->rx_retransmit = ilkn_config->retransmit.enable_rx;
    add_info->tx_retransmit = ilkn_config->retransmit.enable_tx;
    add_info->reserved_channel_rx = ilkn_config->retransmit.reserved_channel_id_rx;
    add_info->reserved_channel_tx = ilkn_config->retransmit.reserved_channel_id_tx;
    
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));

    if (SOC_PORT_IS_ELK_INTERFACE(flags))
    {
        PORTMOD_PORT_ADD_F_ELK_SET(add_info);
    }
	
    add_info->ilkn_burst_max = SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_max[offset];
    if (add_info->ilkn_burst_max != 128 && add_info->ilkn_burst_max != 256) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, 
                             (_BSL_SOCDNX_MSG("Burst max %d not supported (only values 128, 256 are supported)"), add_info->ilkn_burst_max)); 
    }

    add_info->ilkn_burst_short = SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_short[offset];
    if ((add_info->ilkn_burst_short > add_info->ilkn_burst_max / 2) || (add_info->ilkn_burst_short % 32 != 0) || (add_info->ilkn_burst_short == 0)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL,
                             (_BSL_SOCDNX_MSG("Burst short %d should be lesser or equal than half of burst max %d, must not be equal to 0 and must be a multiplier of 32"), 
                              add_info->ilkn_burst_short, add_info->ilkn_burst_max));
    }

    add_info->ilkn_burst_min = SOC_DPP_JER_CONFIG(unit)->nif.ilkn_burst_min[offset];
    if ((add_info->ilkn_burst_min > add_info->ilkn_burst_max / 2) || (add_info->ilkn_burst_min % 32 != 0) || (add_info->ilkn_burst_min < add_info->ilkn_burst_short) || (add_info->ilkn_burst_min == 0)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL,
                             (_BSL_SOCDNX_MSG("Burst min %d should be lesser or equal than half of burst max %d, must be grater than or equal to burst short %d, must not be equal to 0 and must be a multiplier of 32"), 
                              add_info->ilkn_burst_min, add_info->ilkn_burst_max, add_info->ilkn_burst_short));
    }

    add_info->ilkn_metaframe_period = soc_property_port_get(unit, offset, spn_ILKN_METAFRAME_SYNC_PERIOD, 2048);
    if (add_info->ilkn_metaframe_period < 64) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Metaframe period should be at least 64"), add_info->ilkn_metaframe_period)); 
    }
    if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_nif_ilkn_pbmp_get, (unit, port, offset, &phys, NULL)));
        PORTMOD_PBMP_CLEAR(add_info->phys);
        PORTMOD_PBMP_OR(add_info->phys, phys);
        add_info->ilkn_port_is_over_fabric = 1;
        SOC_PBMP_ITER(phys, phy_i){
            SOC_INFO(unit).port_p2l_mapping[phy_i] = port;
        }
    }

    add_info->ilkn_inb_cal_len_rx = SOC_DPP_CONFIG(unit)->tm.fc_inband_intlkn_calender_length[offset][SOC_TMC_CONNECTION_DIRECTION_RX] * 
        SOC_DPP_CONFIG(unit)->tm.fc_inband_intlkn_calender_rep_count[offset][SOC_TMC_CONNECTION_DIRECTION_RX];
    add_info->ilkn_inb_cal_len_tx = SOC_DPP_CONFIG(unit)->tm.fc_inband_intlkn_calender_length[offset][SOC_TMC_CONNECTION_DIRECTION_TX] *
        SOC_DPP_CONFIG(unit)->tm.fc_inband_intlkn_calender_rep_count[offset][SOC_TMC_CONNECTION_DIRECTION_TX];

    SOCDNX_IF_ERR_EXIT(jer_fc_find_oob_inf_for_ilkn_inf(unit, port, &oob_if));
    if (oob_if != -1) {
        add_info->ilkn_oob_cal_len_rx = SOC_DPP_CONFIG(unit)->tm.fc_oob_calender_length[oob_if][SOC_TMC_CONNECTION_DIRECTION_RX] *
            SOC_DPP_CONFIG(unit)->tm.fc_oob_calender_rep_count[oob_if][SOC_TMC_CONNECTION_DIRECTION_RX];
        add_info->ilkn_oob_cal_len_tx = SOC_DPP_CONFIG(unit)->tm.fc_oob_calender_length[oob_if][SOC_TMC_CONNECTION_DIRECTION_TX] *
            SOC_DPP_CONFIG(unit)->tm.fc_oob_calender_rep_count[oob_if][SOC_TMC_CONNECTION_DIRECTION_TX];
    }

exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_nif_pm_credits_override(int unit, soc_port_t port)
{
    soc_pbmp_t phys, lanes;
    uint32 first_phy, phy_lane;
    soc_reg_t reg;
    int qmlf_index = 0, reg_port, max_ports_nbih, max_ports_nbil;
    uint64 reg64_val;
    int nof_phys, is_qsgmii, val;
    soc_port_t lane;
    uint32 orig_port_crdt, credits_val, rst_port_crdt;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    max_ports_nbih = SOC_DPP_DEFS_GET(unit, nof_ports_nbih);
    max_ports_nbil = SOC_DPP_DEFS_GET(unit, nof_ports_nbil);

    /* override_pm_credits_to_nbi */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy, &phy_lane)));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    --phy_lane; /* phy_lane returned is one-based */
    --first_phy; /* first_phy returned is one-based */

	if (SOC_IS_QUX(unit)) {
        /* QUX no NBIH, only NBIL */
        reg_port = REG_PORT_ANY;
        qmlf_index = (phy_lane % max_ports_nbil) / NUM_OF_LANES_IN_PM;
    } else {
        reg_port = (first_phy < max_ports_nbih) ? REG_PORT_ANY : (first_phy / (max_ports_nbih + max_ports_nbil));
        qmlf_index = (phy_lane % max_ports_nbih) / NUM_OF_LANES_IN_PM; 
    }
    is_qsgmii = (interface_type == SOC_PORT_IF_QSGMII);
    SOC_PBMP_COUNT(phys, nof_phys);

    credits_val = (is_qsgmii) ? 4 : (nof_phys * 8);
    val = (is_qsgmii) ? 0xf : 0x1;
    rst_port_crdt = 0;
    
    SOC_PBMP_ITER(lanes, lane) {
        if (lane == 0) {
            continue; /*Coverity protection*/
        }
        rst_port_crdt |= val << (((lane - 1) % 4) * 4); 
    }

    /* reset the credits for selected ports */
    if (SOC_IS_QUX(unit)) {
        reg = NIF_TX_QMLF_CONFIGr;
    } else {
        reg = (first_phy < max_ports_nbih) ? NBIH_TX_QMLF_CONFIGr : NBIL_TX_QMLF_CONFIGr;
    }
    SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, reg_port, qmlf_index, &reg64_val));
    orig_port_crdt = soc_reg64_field32_get(unit, reg, reg64_val, TX_RESET_PORT_CREDITS_QMLF_Nf);
    rst_port_crdt |= orig_port_crdt;
    soc_reg64_field32_set(unit, reg, &reg64_val, TX_RESET_PORT_CREDITS_QMLF_Nf, rst_port_crdt);
    soc_reg64_field32_set(unit, reg, &reg64_val, TX_RESET_PORT_CREDITS_VALUE_QMLF_Nf, credits_val);
    SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, qmlf_index, reg64_val));

    /* restore default values*/
    soc_reg64_field32_set(unit, reg, &reg64_val, TX_RESET_PORT_CREDITS_QMLF_Nf, 0);
    soc_reg64_field32_set(unit, reg, &reg64_val, TX_RESET_PORT_CREDITS_VALUE_QMLF_Nf, 0);
    SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, qmlf_index, reg64_val));
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_portmod_power_enable_set(int unit, soc_port_t port, int enable) {

    soc_pbmp_t quad_ports, phys, lanes;
    int idx, rv, pm_idx, reg_port, quad_nof_ports ;
    soc_field_t field;
    soc_reg_t reg;
    uint32 otp_bits, reg_val, quad, phy_lane, first_phy;
    int nof_pms_per_nbi;
    soc_port_if_t interface_type;
    uint32 nof_pm4x10       = SOC_DPP_DEFS_GET(unit, nof_pm4x10);
    uint32 nof_pm4x25       = SOC_DPP_DEFS_GET(unit, nof_pm4x25);
    SOCDNX_INIT_FUNC_DEFS;

    nof_pms_per_nbi = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    /* ILKN over fabric ports dont need this configuration */
    if (first_phy >= SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit)) {
        SOC_EXIT;
    }

    rv = soc_port_sw_db_phy_ports_get(unit, port, &phys); 
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &lanes);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_jer_port_ports_to_same_quad_get(unit, port, &quad_ports);
    SOCDNX_IF_ERR_EXIT(rv);

    SOC_PBMP_COUNT(quad_ports, quad_nof_ports);
    /* if there is more than one port on this quad, do nothing*/
    if (!enable && quad_nof_ports > 1) {
        SOC_EXIT;
    }


    SOC_PBMP_ITER(lanes, phy_lane) {
        quad = (phy_lane - 1) / NUM_OF_LANES_IN_PM; 
        reg_port = (quad / nof_pms_per_nbi == 0) ? REG_PORT_ANY : (quad / nof_pms_per_nbi) - 1;
        pm_idx = quad % nof_pms_per_nbi;
        if (SOC_IS_QUX(unit)) {
            reg = NIF_NIF_PM_CFGr;
            field = PM_N_OTP_PORT_BOND_OPTIONf;
            idx = pm_idx;
        } else {
            if (reg_port == REG_PORT_ANY) {
                reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIH_REG_0C06r : NBIH_NIF_PM_CFGr;
                field = (SOC_IS_JERICHO_PLUS(unit)) ? PMH_N_OTP_PORT_BOND_OPTIONf : FIELD_0_8f;
                idx = pm_idx;
            } else {
                field = (SOC_IS_JERICHO_PLUS(unit)) ? PML_N_OTP_PORT_BOND_OPTIONf : FIELD_0_13f;
                switch (pm_idx) {
                    case 3:
                        reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_3r : NBIL_NIF_PM_CFG_3r;
                        idx = 0;
                        break;
                    case 4:
                        reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_4r : NBIL_NIF_PM_CFG_4r;
                        idx = 0;
                        break;
                    case 5:
                        reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_5r : NBIL_NIF_PM_CFG_5r;
                        idx = 0;
                        break;
                    default:
               	        reg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A06r : NBIL_NIF_PM_CFGr;
               	        idx = pm_idx;
                        break;
                }
            }
        }
        /* Enable power on active lanes */
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, idx, &reg_val));
        otp_bits = soc_reg_field_get(unit, reg, reg_val, field);
        otp_bits = (enable) ? (otp_bits | 0x100) : (otp_bits &~ 0x100); /* enable/disable this quad */
        otp_bits = (enable) ? (otp_bits &~ 0xf0) : (otp_bits | 0xf0); /* enable/disable all its lanes (once the quad is active - no power cost for enabling all its lanes) */
        if ((SOC_IS_QUX(unit)) && (interface_type == SOC_PORT_IF_DNX_XAUI) && (enable)
             && (quad >= (nof_pm4x10+nof_pm4x25))) {
            otp_bits = otp_bits | 0x80;
        }
        soc_reg_field_set(unit, reg, &reg_val, field, otp_bits);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, idx, reg_val));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_port_detach(int unit, int port){

    int is_pmd_lb;
    uint32 nof_channels;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_loopback_get(unit, port, portmodLoopbackPhyGloopPMD, &is_pmd_lb));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_of_channels_get(unit, port, &nof_channels));

    if (is_pmd_lb && nof_channels == 1) {
        SOCDNX_IF_ERR_EXIT(portmod_port_loopback_set(unit, port, portmodLoopbackPhyGloopPMD, 0));

        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_port_enable_set(unit, port, 0, 0));
    }
    
    SOCDNX_IF_ERR_EXIT(portmod_port_remove(unit, port));

    if (nof_channels == 1) {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_power_enable_set(unit, port, 0)); 
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
_soc_jer_portmod_config_info_init(int unit, soc_port_t port, portmod_port_add_info_t *add_info, int is_init_sequence)
{
    int rv, speed, cl72;
    int an_cl37 = 0, an_cl73=0;
    int i, index;
    soc_pbmp_t phys;
    int fw_verify = 0;
    uint32 first_phy_quad, first_phy = 0, first_lane, phy_lane;
    phymod_operation_mode_t phy_op_mode=0;
    phymod_lane_map_t lane_map, orig_lane_map;
    int serdes_1000x_at_6250_vco;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_add_info_t_init(unit, add_info);
                SOCDNX_IF_ERR_EXIT(rv);

    /* Skip autoneg configuration in port attach */
    PORTMOD_PORT_ADD_F_AUTONEG_CONFIG_SKIP_SET(add_info);
    /* Fill in init config */
    an_cl37 = soc_property_port_get(unit, port, spn_PHY_AN_C37, 0);
    an_cl73 = soc_property_port_get(unit, port, spn_PHY_AN_C73, 0);
    add_info->init_config.an_mode = phymod_AN_MODE_CL73 ;
    add_info->init_config.an_cl72 = 1 ;
    if (PORTMOD_CL73_WO_BAM == an_cl73) {
        add_info->init_config.an_mode = phymod_AN_MODE_CL73;
        add_info->init_config.an_cl72 = 1 ;
    }
    else if (PORTMOD_CL73_W_BAM == an_cl73) {
        add_info->init_config.an_mode = phymod_AN_MODE_CL73BAM;
        add_info->init_config.an_cl72 = 1 ;
    }
    else if (PORTMOD_CL37_W_BAM == an_cl37) {
        add_info->init_config.an_mode = phymod_AN_MODE_CL37BAM;
        add_info->init_config.an_cl72 = 0 ;
    }
    else if (PORTMOD_CL37_WO_BAM == an_cl37) {
        add_info->init_config.an_mode = phymod_AN_MODE_CL37;
        add_info->init_config.an_cl72 = 0 ;
    }
    add_info->init_config.fs_cl72 = soc_property_port_get(unit, port, \
        spn_PORT_INIT_CL72, 0);
    add_info->init_config.an_cl72 = soc_property_port_get(unit, port, \
        spn_PHY_AN_C72, add_info->init_config.an_cl72);
    add_info->init_config.an_fec = soc_property_port_get(unit, port, \
        spn_PHY_AN_FEC, add_info->init_config.an_fec);
    add_info->init_config.cx4_10g = soc_property_port_get(unit, port, \
                                     spn_10G_IS_CX4, TRUE);

    if ((SOC_DPP_CONFIG(unit))->pp.ptp_48b_stamping_enable) {
        add_info->flags |= PORTMOD_PORT_ADD_F_EGR_1588_TIMESTAMP_MODE_48BIT;
    }

    /* default mode - strup & append CRC at MAC level*/
    PORTMOD_PORT_ADD_F_RX_SRIP_CRC_SET(add_info);
    PORTMOD_PORT_ADD_F_TX_APPEND_CRC_SET(add_info);

    rv = soc_port_sw_db_speed_get(unit, port, &speed);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = _soc_jer_portmod_speed_to_if_config(unit, port, speed, &(add_info->interface_config));
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_port_sw_db_phy_ports_get(unit, port, &phys);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, (soc_pbmp_t*)&(add_info->phys));
    SOCDNX_IF_ERR_EXIT(rv);

    rv = soc_port_sw_db_first_phy_port_get(unit, port, &first_phy);
    SOCDNX_IF_ERR_EXIT(rv);

    if (add_info->interface_config.interface == SOC_PORT_IF_QSGMII) {
        add_info->sub_phy = (first_phy - 1) % 4;
    }

    SOC_PBMP_CLEAR(add_info->phy_ports);
    if (add_info->sub_phy) {
        SOC_PBMP_PORT_ADD(add_info->phy_ports, first_phy - add_info->sub_phy);
    } else {
        PORTMOD_PBMP_CLEAR(add_info->phy_ports);
        PORTMOD_PBMP_OR(add_info->phy_ports, phys);
    }

    SOC_PBMP_ITER(add_info->phys, phy_lane) {
        if (phy_lane >= SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit)) {
            add_info->ilkn_port_is_over_fabric = 1;
            break;
        }
    }
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy, &first_lane)));
    first_phy_quad = (first_lane - 1) / 4;
    /*coverity[overrun-local]*/
    fw_verify = SOC_DPP_JER_CONFIG(unit)->nif.fw_verify[first_phy_quad];

    if(fw_verify) {
        PORTMOD_PORT_ADD_F_FIRMWARE_LOAD_VERIFY_SET(add_info);
    } else {
        PORTMOD_PORT_ADD_F_FIRMWARE_LOAD_VERIFY_CLR(add_info);
    }
    if (add_info->interface_config.interface == SOC_PORT_IF_QSGMII) {
        SOCDNX_IF_ERR_EXIT(soc_jer_lane_map_get(unit, first_phy_quad, &lane_map));
        /*
         * QSGMII PCS does not support lane swap. so in order for all lanes to initiate properly,
         *  need to override the rx lane map to 0x3210 (no swap).
         */
        orig_lane_map = lane_map;
        for (i = 0; i < SOC_JERICHO_NOF_LANES_PER_CORE; ++i) {
            index = lane_map.lane_map_rx[i];
            lane_map.lane_map_tx[index] = orig_lane_map.lane_map_tx[i];
            lane_map.lane_map_rx[i] = i;
        }
        add_info->init_config.lane_map[0] = lane_map;
    }
    if (add_info->interface_config.interface == SOC_PORT_IF_ILKN) {
        SOCDNX_IF_ERR_EXIT(jer_fc_connect_ilkn_inf_to_oob_inf(unit, port));

        SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_config_get(unit, port, add_info));
    }

     /* configure link training */
    if (is_init_sequence) {
        cl72 = soc_property_port_get(unit, port, spn_PORT_INIT_CL72, 0) ? 1 : 0;
        add_info->link_training_en = (uint8)cl72;

        phy_op_mode = soc_property_port_get(unit, port, spn_PHY_PCS_REPEATER,
                                             phymodOperationModeRetimer);

        add_info->phy_op_mode = (phymod_operation_mode_t) phy_op_mode;
        add_info->interface_config.port_op_mode = (int) phy_op_mode;

        add_info->interface_config.line_interface = soc_jer_get_interface_type(unit, port, SOC_PORT_IF_NULL);

        serdes_1000x_at_6250_vco = soc_property_port_get(unit, port, spn_SERDES_1000X_AT_6250_VCO, 0);

        if (serdes_1000x_at_6250_vco) {
            add_info->interface_config.pll_divider_req = JER_SOC_TSCE_VCO_6_25_PLL_DIV;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;

}

/* this procedure is meant to init SW structs when loading in WarmBoot state */
STATIC 
int soc_jer_nif_wb_init(int unit, soc_pbmp_t pbmp){

    soc_port_t port;
    uint32 nof_segments, ilkn_id;
    soc_pbmp_t phys, phy_lanes;
    SHR_BITDCL* serdes_qmlfs;
    SHR_BITDCL* memory_qmlfs;
    soc_jer_ilkn_qmlf_resources_t *ilkn_qmlf_resources = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_qmlf_resources;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_ITER(pbmp, port) {
        if (IS_IL_PORT(unit, port)) {

            SOC_PBMP_CLEAR(phys);
            SOC_PBMP_CLEAR(phy_lanes);

            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &ilkn_id));
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_nof_segments_calc(unit, port, &nof_segments));
            SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[ilkn_id] = nof_segments;

            /* init ilkn qmlfs */
            /* for ILKN over Fabric leave all serdes QMLFs to be taken by Eth ports*/
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
            SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &phy_lanes));

            if (!SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, ilkn_id)) {
                serdes_qmlfs = ilkn_qmlf_resources[ilkn_id].serdes_qmlfs;
                SOCDNX_IF_ERR_EXIT(soc_jer_ilkn_serdes_qmlfs_get(unit, &phys, serdes_qmlfs));
            }
            memory_qmlfs = ilkn_qmlf_resources[ilkn_id].memory_qmlfs;
            SOCDNX_IF_ERR_EXIT(soc_jer_ilkn_memory_qmlfs_get(unit, port, memory_qmlfs));

        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_probe(int unit, pbmp_t pbmp, pbmp_t *okay_pbmp, int is_init_sequence)
{
    int rv, counter_interval;
    soc_port_t port;
    portmod_port_add_info_t add_info;
    uint32 is_master, flags, counter_flags;
    dcmn_port_fabric_init_config_t port_config;
    soc_pbmp_t counter_pbmp;
    int broadcast_load_fabric = 0;
    dcmn_port_init_stage_t stage;
    phymod_firmware_load_method_t fw_load_method_fabric = phymodFirmwareLoadMethodNone;
    int fw_verify_fabric = 0;
    int fsrd_block_, fmac_block_, blk, is_first_link, i;
    soc_info_t          *si;
    soc_port_if_t interface_type;
    pbmp_t nif_pbmp;
    int phy_rstn = -1;
    int phy_curr;
    SOCDNX_INIT_FUNC_DEFS;

    si  = &SOC_INFO(unit);

    if (SOC_WARM_BOOT(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_wb_init(unit, pbmp));
        SOC_PBMP_ASSIGN(*okay_pbmp, pbmp);
    } else {
        SOC_PBMP_CLEAR(*okay_pbmp);
        SOC_PBMP_CLEAR(nif_pbmp);

        if (is_init_sequence) {
            fw_load_method_fabric = soc_property_suffix_num_get(unit, -1, spn_LOAD_FIRMWARE, "fabric", phymodFirmwareLoadMethodExternal);
            fw_verify_fabric = (fw_load_method_fabric & 0xff00 ? 1 : 0);
            fw_load_method_fabric &= 0xff;

            if(fw_load_method_fabric == phymodFirmwareLoadMethodExternal) {
                 broadcast_load_fabric = 1;
            }
        }


        SOC_PBMP_ITER(pbmp, port) {
            if (IS_SFI_PORT(unit, port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {

                if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
                    rv = soc_jer_port_first_link_in_fsrd_get(unit, port, &is_first_link, 1); 

                    if (is_first_link == 1) 
                    {
                        fsrd_block_ = SOC_DPP_FABRIC_PORT_TO_LINK(unit, port)/SOC_DPP_DEFS_GET(unit, nof_links_in_fsrd);

                        fmac_block_ = fsrd_block_ * SOC_DPP_DEFS_GET(unit, nof_quads_in_fsrd);

                        for (i = fmac_block_; i < fmac_block_ +  SOC_DPP_DEFS_GET(unit, nof_quads_in_fsrd) ; i++)
                        {
                            blk = si->fmac_block[i];
                            si->block_valid[blk] = 1;
                        }

                        if (!SOC_IS_QAX(unit)) {
                            blk = si->fsrd_block[fsrd_block_];
                            si->block_valid[blk] = 1;

                            rv = soc_jer_port_update_fsrd_block(unit, port, 1);
                            SOCDNX_IF_ERR_EXIT(rv);
                        }
                    }
                }
                if (SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
                     SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_open_fab_o_nif_path, (unit, port)));
                }

                /*make sure fabric port is not used for ILKN*/
                /* SFI is loaded in 2 stages - this is stage 1*/
                rv = _soc_jer_port_fabric_config_get(unit, port, is_init_sequence, &port_config);
                SOCDNX_IF_ERR_EXIT(rv);

                stage = broadcast_load_fabric ? dcmn_port_init_until_fw_load : dcmn_port_init_full;

                rv = soc_dcmn_fabric_port_probe(unit, port, stage, fw_verify_fabric, &port_config);
                SOCDNX_IF_ERR_EXIT(rv);
                if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
                    SOC_PBMP_PORT_ADD(si->sfi.bitmap, port);
                    SOC_PBMP_PORT_ADD(si->port.bitmap, port);
                    SOC_PBMP_PORT_ADD(si->all.bitmap, port);

                    SOC_PBMP_PORT_REMOVE(si->sfi.disabled_bitmap, port);
                    SOC_PBMP_PORT_REMOVE(si->port.disabled_bitmap, port);
                    SOC_PBMP_PORT_REMOVE(si->all.disabled_bitmap, port);
                }
            }
        }
        /*NIF: Probe ,initialize and FW download*/
        /*step1: probe Serdes and external PHY core*/
        SOC_PBMP_ITER(pbmp, port) {
            /* Init NIF ports*/
            if (IS_SFI_PORT(unit, port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
                continue;
            }

            if (IS_IL_PORT(unit, port)) {
                /* Validate serdes speed is not too high for core clock */
                rv = soc_jer_portmod_ilkn_speed_validate(unit, port);
                SOCDNX_IF_ERR_EXIT(rv);
            }

            /* Init NIF ports*/
            rv = soc_jer_port_open_path(unit, port);
            SOCDNX_IF_ERR_EXIT(rv);
            rv = portmod_port_add_info_t_init(unit, &add_info);
            SOCDNX_IF_ERR_EXIT(rv);
            rv = _soc_jer_portmod_config_info_init(unit, port, &add_info, is_init_sequence);
            SOCDNX_IF_ERR_EXIT(rv);
            if (add_info.interface_config.interface == SOC_PORT_IF_XFI) {
                SOCDNX_IF_ERR_EXIT(portmod_common_default_interface_get(&add_info.interface_config));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_set(unit, port, add_info.interface_config.interface));
            }
            SOCDNX_IF_ERR_EXIT(soc_jer_portmod_power_enable_set(unit, port, 1));

            if (add_info.interface_config.interface != SOC_PORT_IF_ILKN && (!is_init_sequence) && !SOC_IS_QAX(unit)) {
                /* Before adding the port - check that port doesn't overwrite an existing ILKN QMLF */
                SOCDNX_IF_ERR_EXIT (soc_jer_portmod_check_for_qmlf_conflict (unit, port));
            }
            /*ILKN and QSGMII ports only need one time init.*/
            if ((add_info.interface_config.interface != SOC_PORT_IF_ILKN) &&
                 (add_info.interface_config.interface != SOC_PORT_IF_QSGMII)) {
                 PORTMOD_PORT_ADD_F_INIT_CORE_PROBE_SET(&add_info);
                 rv = portmod_port_add(unit, port, &add_info);
                 PORTMOD_PORT_ADD_F_INIT_CORE_PROBE_CLR(&add_info);
            } else {
                rv = portmod_port_add(unit, port, &add_info); 
            }
            SOCDNX_IF_ERR_EXIT(rv);
        }

        /*step2 : initialize PASS1 for SerDes and external PHY*/
        SOC_PBMP_ITER(pbmp, port) {
            if (IS_SFI_PORT(unit, port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
                continue;
            }
            rv = portmod_port_add_info_t_init(unit, &add_info);
            SOCDNX_IF_ERR_EXIT(rv);
            rv = _soc_jer_portmod_config_info_init(unit, port, &add_info, is_init_sequence);
            SOCDNX_IF_ERR_EXIT(rv);
            if (add_info.interface_config.interface == SOC_PORT_IF_XFI) {
                SOCDNX_IF_ERR_EXIT(portmod_common_default_interface_get(&add_info.interface_config));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_set(unit, port, add_info.interface_config.interface));
            }
            if ((add_info.interface_config.interface != SOC_PORT_IF_ILKN) &&
                 (add_info.interface_config.interface != SOC_PORT_IF_QSGMII)) {
                 PORTMOD_PORT_ADD_F_INIT_PASS1_SET(&add_info);
                 rv = portmod_port_add(unit, port, &add_info);
                 PORTMOD_PORT_ADD_F_INIT_PASS1_CLR(&add_info);
            }
            /* Call portmod_port_add() second time to release ILKN_BYPASS_RSTN */
            SOC_PBMP_ITER(add_info.phys, phy_curr) {
                break;
            }
            if ((add_info.interface_config.interface == SOC_PORT_IF_QSGMII) &&
                (phy_curr != phy_rstn)) {
                PORTMOD_PORT_ADD_F_INIT_PASS1_SET(&add_info);
                rv = portmod_port_add(unit, port, &add_info);
                PORTMOD_PORT_ADD_F_INIT_PASS1_CLR(&add_info);
                phy_rstn = phy_curr;
            }
            SOCDNX_IF_ERR_EXIT(rv);


            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_master_get(unit, port, &is_master));
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));

            if (is_master && (interface_type != SOC_PORT_IF_ILKN) && (interface_type != SOC_PORT_IF_QSGMII)) {
                SOC_PBMP_PORT_ADD(nif_pbmp, port);
            }
        }

        /* step3:broadcast firmware download for all external phys inculde legacy and Phymod PHYs*/
        rv = portmod_legacy_ext_phy_init(unit, nif_pbmp);
        SOCDNX_IF_ERR_EXIT(rv);
        rv = portmod_common_ext_phy_fw_bcst(unit, nif_pbmp);
        SOCDNX_IF_ERR_EXIT(rv);

        /*step4:initialize PASS2 for Serdes and external PHY*/
        SOC_PBMP_ITER(pbmp, port) {
            if (IS_SFI_PORT(unit, port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
                continue;
            }
            rv = portmod_port_add_info_t_init(unit, &add_info);
            SOCDNX_IF_ERR_EXIT(rv);
            rv = _soc_jer_portmod_config_info_init(unit, port, &add_info, is_init_sequence);
            SOCDNX_IF_ERR_EXIT(rv);
            if (add_info.interface_config.interface == SOC_PORT_IF_XFI) {
                SOCDNX_IF_ERR_EXIT(portmod_common_default_interface_get(&add_info.interface_config));
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_set(unit, port, add_info.interface_config.interface));
            }
            if ((add_info.interface_config.interface != SOC_PORT_IF_ILKN) &&
                (add_info.interface_config.interface != SOC_PORT_IF_QSGMII)) {
                PORTMOD_PORT_ADD_F_INIT_PASS2_SET(&add_info);
                rv = portmod_port_add(unit, port, &add_info);
                PORTMOD_PORT_ADD_F_INIT_PASS2_CLR(&add_info);
            }

            SOCDNX_IF_ERR_EXIT(rv);
            if (add_info.interface_config.interface == SOC_PORT_IF_ILKN) {
                int lane_map_override;
                int ilkn_lane_map[JER_NIF_ILKN_MAX_NOF_LANES];
                uint32 nof_lanes;

                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &nof_lanes));
                SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_lane_map_get(unit, port, &lane_map_override, ilkn_lane_map));

                if (lane_map_override) {
                    SOCDNX_IF_ERR_EXIT(portmod_port_logical_lane_order_set(unit, port, ilkn_lane_map, nof_lanes));
                }
            }
            else if (add_info.interface_config.interface == SOC_PORT_IF_QSGMII) {
                SOCDNX_IF_ERR_EXIT(soc_jer_nif_pm_credits_override(unit, port));
            }
            rv = soc_port_sw_db_initialized_set(unit, port, 1);
            SOCDNX_IF_ERR_EXIT(rv);

            SOC_PBMP_PORT_ADD(*okay_pbmp, port);
        }

        if( SOC_PBMP_NOT_NULL(SOC_PORT_BITMAP(unit, sfi)) && broadcast_load_fabric && (!SOC_IS_QAX(unit) || SOC_IS_QAX_WITH_FABRIC_ENABLED(unit))) {
            /* Load fabric firmware*/
            rv = soc_dcmn_fabric_broadcast_firmware_loader(unit, tscf_ucode_len, tscf_ucode);
            SOCDNX_IF_ERR_EXIT(rv);
        }

        SOC_PBMP_ITER(pbmp, port) {
            if(IS_SFI_PORT(unit, port) && !SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
                if(broadcast_load_fabric) {
                    /* fabric side initialization - stage 2*/
                    rv = _soc_jer_port_fabric_config_get(unit, port, is_init_sequence, &port_config);
                    SOCDNX_IF_ERR_EXIT(rv);
                    rv = soc_dcmn_fabric_port_probe(unit, port, dcmn_port_init_resume_after_fw_load, fw_verify_fabric, &port_config);
                    SOCDNX_IF_ERR_EXIT(rv);
                }

                SOC_PBMP_PORT_ADD(*okay_pbmp, port);
            }
        }
       
        /* Update Counter thread*/
        SOCDNX_IF_ERR_EXIT(soc_counter_status(unit, &counter_flags, &counter_interval, &counter_pbmp));
        if(counter_interval > 0) {
            soc_counter_stop(unit);

            SOC_PBMP_ITER(*okay_pbmp, port) {
                if ( !(IS_SFI_PORT(unit, port) || SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) ) {

                    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_master_get(unit, port, &is_master));

                    if(!is_master) {
                        /* don't count per channel*/
                        if(SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn_counters_mode != soc_arad_stat_ilkn_counters_mode_packets_per_channel) {
                            continue;
                        }
                        /* Count per channel is supported only for ILKN */
                        if(add_info.interface_config.interface != SOC_PORT_IF_ILKN) {
                            continue;
                        }
                    }

                    /* counter thread is supported only for NIF or fabric ports */
                    SOCDNX_IF_ERR_RETURN(soc_port_sw_db_flags_get(unit, port, &flags));
                    if((!SOC_PORT_IS_NETWORK_INTERFACE(flags)) || SOC_PORT_IS_LB_MODEM(flags)) {
                        continue;
                    }

                    SOC_PBMP_PORT_ADD(counter_pbmp, port);
                } else {
                    SOC_PBMP_PORT_ADD(counter_pbmp, port);
                }
            }

            SOCDNX_IF_ERR_EXIT(soc_counter_start(unit, counter_flags, counter_interval, counter_pbmp));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_nif_ilkn_wrapper_port_enable_set(int unit, soc_port_t port, int enable)
{
    uint32  reg_val, offset;
    soc_port_t reg_port;
    soc_reg_t reg;
    soc_field_t tx_field, rx_field;
    SOCDNX_INIT_FUNC_DEFS;

    /** set_ilkn_port_reset in wrapper **/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    reg = (offset < 2) ? ILKN_PMH_ILKN_RESETr : ILKN_PML_ILKN_RESETr;
    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4);

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
    tx_field = (offset & 1) ? ILKN_TX_1_PORT_RSTNf : ILKN_TX_0_PORT_RSTNf;
    soc_reg_field_set(unit, reg, &reg_val, tx_field, enable ? 1 : 0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));
    sal_usleep(100000); /*this fixes the first_packet problem in loopback*/

    rx_field = (offset & 1) ? ILKN_RX_1_PORT_RSTNf : ILKN_RX_0_PORT_RSTNf;
    soc_reg_field_set(unit, reg, &reg_val, rx_field, enable ? 1 : 0);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_port_enable_set(int unit, soc_port_t port, uint32 mac_only, int enable)
{
    uint32 offset, flags = 0;
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
        SOC_EXIT;
    }
    if(SOC_PBMP_MEMBER(PBMP_CMIC(unit), port)) {
        SOC_EXIT;
    }
    if(mac_only) {
        PORTMOD_PORT_ENABLE_MAC_SET(flags);
    }
    if (IS_IL_PORT(unit, port) && (BCM_E_NONE != bcm_petra_init_check(unit))) {
        /*During init, ILKN_over_fabric shouldn't activate its FMAC*/
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            PORTMOD_PORT_ENABLE_PHY_SET(flags);
        }
    }

    if (enable)
    {
        if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 0)));
        } else if (IS_SFI_PORT(unit,port) && SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_fabric_o_nif_bypass_interface_enable, (unit, port, 0)));
        }

        if (!IS_IL_PORT(unit,port) && !IS_SFI_PORT(unit,port))
        {
            SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_tx_egress_credits_set(unit, port, SOC_JER_TX_EGRESS_CREDITS_CMD_FLUSH, 0));
            SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_ports_rstn(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(soc_jer_port_eqg_nif_credit_init(unit, port));
            SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_ports_rstn(unit, port, 1));
        }

        SOCDNX_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, 1));

        if (!IS_IL_PORT(unit,port) && !IS_SFI_PORT(unit,port))
        {
            SOCDNX_IF_ERR_EXIT(soc_jer_port_control_tx_nif_enable_set(unit, port, 1));
        }

        if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 1)));
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 1));
        } else if (IS_SFI_PORT(unit,port) && SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_fabric_o_nif_bypass_interface_enable, (unit, port, 1)));
        }
    }
    else
    {
        if (IS_IL_PORT(unit, port)) {
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 0)));
        } else if (IS_SFI_PORT(unit,port) && SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_fabric_o_nif_bypass_interface_enable, (unit, port, 0)));
        }

        if (!IS_IL_PORT(unit,port) && !IS_SFI_PORT(unit,port))
        {
            SOCDNX_IF_ERR_EXIT(soc_jer_port_control_tx_nif_enable_set(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_tx_egress_credits_set(unit, port, SOC_JER_TX_EGRESS_CREDITS_CMD_FLUSH, 1));
        }

        SOCDNX_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, 0));
    }


exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_portmod_port_enable_get(int unit, soc_port_t port, uint32 mac_only, int* enable)
{
    int flags = 0;
    SOCDNX_INIT_FUNC_DEFS;
    

    if (IS_SFI_PORT(unit, port) && SOC_PBMP_MEMBER(SOC_PORT_DISABLED_BITMAP(unit, sfi), port)) {
        *enable = 0;
    } else {
        if(mac_only) {
            PORTMOD_PORT_ENABLE_MAC_SET(flags);
        }
        SOCDNX_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, enable)); 
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* This function should be called from speed_set function or other procedure
 * that might change the required number of segments for an ILKN port.
 * purpose of this function is to check if number of segments should change.
 * if it should change, update the HW accordingly.
 * valid number of segments is 2 or 4.                                                                
 */
STATIC 
int soc_jer_nif_ilkn_nof_segments_change(int unit, soc_port_t port)
{
    int current_nof_segments = 0;
    uint32 offset, new_nof_segments = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    current_nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_nof_segments_calc(unit, port, &new_nof_segments));
    
    if (new_nof_segments != current_nof_segments) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_ilkn_nof_segments_set, (unit, port, new_nof_segments)));
        SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset] = new_nof_segments;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_portmod_adjust_default_interface(int unit, soc_port_if_t *intf)
{
    SOCDNX_INIT_FUNC_DEFS;

    switch (*intf) {
        case SOC_PORT_IF_CAUI4:
            *intf = SOC_PORT_IF_CAUI;
            break;

        default:
            break;
    }

    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_port_speed_set(int unit, soc_port_t port, int speed)
{
    portmod_port_interface_config_t config;
    portmod_port_interface_config_t config_pm;
    soc_port_if_t default_interface = SOC_PORT_IF_NULL;
    int ret = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &config_pm, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));

    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_speed_sku_restrictions, (unit, port, speed)));

    /* Update SW DB */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_set(unit, port, speed));

    if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_ilkn_speed_validate(unit, port));
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 0));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 0)));
    } else if (IS_SFI_PORT(unit,port) && SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_fabric_o_nif_bypass_interface_enable, (unit, port, 0)));
    }

    SOCDNX_IF_ERR_EXIT(_soc_jer_portmod_speed_to_if_config(unit, port, speed, &config));
    config.pll_divider_req = config_pm.pll_divider_req;

    /* keep the correct phy interface, not the main interface stored in sw_db */
    config.interface = config_pm.interface;

    /* this sets the speed */
    ret = portmod_port_interface_config_set(unit, port, &config,
                                          PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);
    if (ret == SOC_E_PARAM) {
        /* If an "Invalid Parameter" happens, retrieve a default interface type
         * based on this input "speed" and try set this interface/speed pair again.
         */
        SOCDNX_IF_ERR_EXIT(portmod_port_default_interface_get(unit, port,
                            &config, &default_interface));
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_adjust_default_interface(unit, &default_interface));
        if (default_interface != SOC_PORT_IF_NULL) {
            config.interface = default_interface;
            ret = portmod_port_interface_config_set(unit, port, &config,
                                                      PORTMOD_INIT_F_EXTERNAL_MOST_ONLY);
            /* In case it fails to set this default interface type and speed, try to restore previous settings,
             * specifically, previous interface type that stored in portmod.
             */
            if (ret != SOC_E_NONE) {
                config.interface = config_pm.interface;
                config.speed = config_pm.speed;
                SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_set(unit, port, &config,
                                        PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
            }

            SOCDNX_IF_ERR_EXIT(ret);
        }
    } else {
        SOCDNX_IF_ERR_EXIT(ret);
    }

    if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
        /*Check if number of segments need to change*/
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_nof_segments_change(unit, port));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 1))); 
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 1));
    } else if (IS_SFI_PORT(unit,port) && SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_fabric_o_nif_bypass_interface_enable, (unit, port, 1)));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_port_speed_get(int unit, soc_port_t port, int* speed)
{
    portmod_port_interface_config_t config;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));

    (*speed) = config.speed; 
    
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_port_interface_set(int unit, soc_port_t port, soc_port_if_t intf)
{
    portmod_port_interface_config_t config;
    SOCDNX_INIT_FUNC_DEFS;
    
    if (!IS_SFI_PORT(unit, port)) {

        SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_t_init(unit, &config));
        SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));

        if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 0));
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 0)));
        }
        /* fiber/copper are relevant to the phy interface, thus clear them and set according to new interface */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flag_remove(unit, port, SOC_PORT_FLAGS_FIBER));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flag_remove(unit, port, SOC_PORT_FLAGS_COPPER));
        PHYMOD_INTF_MODES_FIBER_CLR(&config);
        PHYMOD_INTF_MODES_COPPER_CLR(&config);
        
        switch (intf) {
            case SOC_PORT_IF_SR:
            case SOC_PORT_IF_SR4:
            case SOC_PORT_IF_SR10:
            case SOC_PORT_IF_SR2:
            case SOC_PORT_IF_LR:
            case SOC_PORT_IF_LR4:
            case SOC_PORT_IF_LR10:
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flag_add(unit, port, SOC_PORT_FLAGS_FIBER));
                PHYMOD_INTF_MODES_FIBER_SET(&config);
                break;
            case SOC_PORT_IF_CR:
            case SOC_PORT_IF_CR4:
            case SOC_PORT_IF_CR2:
            case SOC_PORT_IF_CR10:
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flag_add(unit, port, SOC_PORT_FLAGS_COPPER));
                PHYMOD_INTF_MODES_COPPER_SET(&config);
                break;
            default:
                break;
        }

        config.interface = intf;
        /* Do NOT effect interface setting into HW until soc_jer_portmod_port_speed_set() is called. */
        config.flags |= PHYMOD_INTF_F_INTF_PARAM_SET_ONLY;
        SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_set(unit, port, &config,
                                            PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
        if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
                SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 1)));
                SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 1));
        }
    } else {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_fabric_port_interface_set(unit, port, intf));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_port_interface_get(int unit, soc_port_t port, soc_port_if_t* intf)
{
    portmod_port_interface_config_t config;
    SOCDNX_INIT_FUNC_DEFS;

    /* Get actual configured value */
    if (IS_SFI_PORT(unit, port)) {
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_fabric_port_interface_get(unit, port, intf));
    } else {
        SOCDNX_IF_ERR_EXIT(portmod_port_interface_config_get(unit, port, &config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));

#ifdef PORTMOD_PM4x10Q_SUPPORT
        {
            int real_port = 0;
            portmod_dispatch_type_t pm_type = portmodDispatchTypeCount;

            SOCDNX_IF_ERR_EXIT(portmod_port_pm_type_get(unit, port, &real_port, &pm_type));
            if ((portmodDispatchTypePm4x10Q == pm_type) &&
                (SOC_PORT_IF_SGMII == config.interface)) {
                config.interface = SOC_PORT_IF_QSGMII;
            }
        }
#endif /* PORTMOD_PM4x10Q_SUPPORT */

        (*intf) = config.interface; 
    }

    

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_fabric_port_interface_set(int unit, soc_port_t port, soc_port_if_t intf)
{
    phymod_ref_clk_t ref_clk;
    phymod_phy_inf_config_t phy_config;
    phymod_phy_access_t phys[SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT];
    int phys_returned;
    int lane = -1, i;
    portmod_access_get_params_t params; 

    SOCDNX_INIT_FUNC_DEFS;


    portmod_access_get_params_t_init(unit, &params);
    params.lane = lane;
    params.phyn = PORTMOD_PHYN_LAST_ONE;
    params.sys_side = PORTMOD_SIDE_LINE;

    SOCDNX_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT, phys, &phys_returned, NULL));


    SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, SOC_INFO(unit).port_refclk_int[port], &ref_clk));


    for (i=0 ; i<phys_returned ; i++) {
        SOC_IF_ERROR_RETURN(phymod_phy_interface_config_get(&phys[i], 0 /* flags */, ref_clk, &phy_config)); 
        PHYMOD_INTF_MODES_FIBER_CLR(&phy_config); 
        PHYMOD_INTF_MODES_COPPER_CLR(&phy_config); 
        switch (intf) {
        case SOC_PORT_IF_SR:
            PHYMOD_INTF_MODES_FIBER_SET(&phy_config); 
            break;
        case SOC_PORT_IF_CR:
            PHYMOD_INTF_MODES_COPPER_SET(&phy_config); 
            break;
        default:
            break;
        }
        if (SOC_IS_QAX_WITH_FABRIC_ENABLED(unit)){
            phy_config.interface_type = phymodInterfaceBypass;
        }

        SOC_IF_ERROR_RETURN(phymod_phy_interface_config_set(&phys[i], 0 /* flags */,&phy_config));

    }
 
exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_portmod_fabric_port_interface_get(int unit, soc_port_t port, soc_port_if_t* intf)
{
    phymod_ref_clk_t ref_clk;
    phymod_phy_inf_config_t phy_config;
    phymod_phy_access_t phys[SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT];
    int phys_returned;
    int lane = -1;
    portmod_access_get_params_t params; 

    SOCDNX_INIT_FUNC_DEFS;


    portmod_access_get_params_t_init(unit, &params);
    params.lane = lane;
    params.phyn = PORTMOD_PHYN_LAST_ONE;
    params.sys_side = PORTMOD_SIDE_LINE;

    SOCDNX_IF_ERR_EXIT(portmod_port_phy_lane_access_get(unit, port, &params, SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT, phys, &phys_returned, NULL));


    SOCDNX_IF_ERR_EXIT(soc_to_phymod_ref_clk(unit, SOC_INFO(unit).port_refclk_int[port], &ref_clk));


    
    SOC_IF_ERROR_RETURN(phymod_phy_interface_config_get(&phys[0], 0 /* flags */, ref_clk, &phy_config)); 


    if (PHYMOD_INTF_MODES_FIBER_GET(&phy_config)) {
        *intf = SOC_PORT_IF_SR; 
    } else if (PHYMOD_INTF_MODES_COPPER_GET(&phy_config)) {
        *intf = SOC_PORT_IF_CR;
    } else {
        *intf = SOC_PORT_IF_KR;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int 
soc_jer_portmod_port_link_state_get(int unit, soc_port_t port, int clear_status, int *is_link_up, int *is_latch_down) 
{
    uint32 offset, reg32_val, reg_port = 0, latch_down;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_link_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, is_link_up));

    if (IS_SFI_PORT(unit, port))
    {
        SOC_EXIT;
    }
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));

    if (SOC_PORT_IF_ILKN == interface_type) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_latch_down_get(unit, port, is_latch_down));
        if (!(*is_latch_down) && (*is_link_up)) {
            /* ILKN0/1 -> port=REG_PORT_ANY, ILKN2/3 -> port=0, ILKN4/5 -> port=1 */
             reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4); 

            if (offset < 2 ) { /* PMH */
                SOCDNX_IF_ERR_EXIT(READ_ILKN_PMH_RX_ILKN_STATUSr(unit, reg_port, offset % 2, &reg32_val));
                latch_down = soc_reg_field_get(unit, ILKN_PMH_RX_ILKN_STATUSr, reg32_val, RX_N_STAT_ALIGNED_LATCH_LOWf); 
            } else { /* PML */
                
                SOCDNX_IF_ERR_EXIT(READ_ILKN_PML_RX_ILKN_STATUSr(unit, reg_port, offset % 2, &reg32_val));
                latch_down = soc_reg_field_get(unit, ILKN_PML_RX_ILKN_STATUSr, reg32_val, RX_N_STAT_ALIGNED_LATCH_LOWf);
            }
            (*is_latch_down) = (latch_down ? 0 : 1);            
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_latch_down_set(unit, port, *is_latch_down));
        } 

        if (clear_status) {
            /* ILKN0/1 -> port=REG_PORT_ANY, ILKN2/3 -> port=0, ILKN4/5 -> port=1 */
             reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4); 
            /* read 'clear on read' register */
            if (offset < 2 ) { /* PMH */
                SOCDNX_IF_ERR_EXIT(READ_ILKN_PMH_RX_ILKN_STATUSr(unit, reg_port, offset % 2, &reg32_val));
            } else { /* PML */
                SOCDNX_IF_ERR_EXIT(READ_ILKN_PML_RX_ILKN_STATUSr(unit, reg_port, offset % 2, &reg32_val));
            }
        }
    } else {
        SOCDNX_IF_ERR_EXIT(portmod_port_link_latch_down_get(unit, port, 
                                                            clear_status ? PORTMOD_PORT_LINK_LATCH_DOWN_F_CLEAR : 0, is_latch_down));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_latch_down_set(unit, port, *is_latch_down));
    }

    if (clear_status) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_latch_down_set(unit, port, 0));
    }

exit:
    SOCDNX_FUNC_RETURN

}

int
soc_jer_portmod_is_supported_encap_get(int unit, int mode, int* is_supported) 
{
    SOCDNX_INIT_FUNC_DEFS;

    switch(mode) {
        case SOC_ENCAP_HIGIG2:
        case SOC_ENCAP_IEEE:
        case SOC_ENCAP_SOP_ONLY:
            (*is_supported) = 1;
            break;

        default:
            (*is_supported) = 0;
            break;

    }

    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_link_get(int unit, soc_port_t port, int *link_up)
{
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(portmod_port_link_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, link_up));

exit:
    SOCDNX_FUNC_RETURN;

}

int
soc_jer_portmod_autoneg_set(int unit, soc_port_t port, int enable) 
{
    phymod_autoneg_control_t an;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_autoneg_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));
    an.enable = enable;
    SOCDNX_IF_ERR_EXIT(portmod_port_autoneg_set(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));

exit:
    SOCDNX_FUNC_RETURN;

}

int
soc_jer_portmod_autoneg_get(int unit, soc_port_t port, int* enable) 
{
    phymod_autoneg_control_t an;
    SOCDNX_INIT_FUNC_DEFS;

    phymod_autoneg_control_t_init(&an);
    SOCDNX_IF_ERR_EXIT(portmod_port_autoneg_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));
    (*enable) = an.enable;

exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_port_ability_remote_get(int unit, soc_port_t port, soc_port_ability_t *ability_mask) 
{
    int rv;
    phymod_autoneg_status_t an_status;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_autoneg_status_get(unit, port, &an_status));
    if (!an_status.enabled) {
        rv = BCM_E_DISABLED;
    } else if (!an_status.locked) {
        rv = BCM_E_BUSY;
    } else {
        rv = portmod_port_ability_remote_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, ability_mask);
    }
    SOCDNX_EXIT_WITH_ERR_NO_MSG(rv);
exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_port_ability_advert_set(int unit, soc_port_t port, soc_port_ability_t *ability_mask) 
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_ability_advert_set(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, ability_mask);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_ability_advert_get(int unit, soc_port_t port, soc_port_ability_t *ability_mask) 
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_ability_advert_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, ability_mask);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_port_ability_local_get(int unit, soc_port_t port, soc_port_ability_t *ability_mask) 
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_ability_local_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, ability_mask);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_mdix_get(int unit, soc_port_t port, soc_port_mdix_t *mode)
{
    int rv;
    int ext_phy_present = 0;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_is_legacy_ext_phy_present(unit, port, &ext_phy_present);
    SOCDNX_IF_ERR_EXIT(rv);

    if (ext_phy_present) {
        rv = portmod_port_ext_phy_mdix_get(unit, port, mode);
        SOCDNX_IF_ERR_EXIT(rv);
    } else {
        /* MDIX is supported for external phys only, so we return a fixed
         * value for internal phys
         */
        *mode = SOC_PORT_MDIX_NORMAL;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_mdix_set(int unit, soc_port_t port, soc_port_mdix_t mode)
{
    int rv;
    int ext_phy_present = 0;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_is_legacy_ext_phy_present(unit, port, &ext_phy_present);
    SOCDNX_IF_ERR_EXIT(rv);

    if (ext_phy_present) {
        rv = portmod_port_ext_phy_mdix_set(unit, port, mode);
        SOCDNX_IF_ERR_EXIT(rv);
    } else {
        /* The behavior of this function should match legacy mdix API's,
         * so if mdix mode is other than normal, we return error
         */
        if (mode != SOC_PORT_MDIX_NORMAL) {
            SOCDNX_IF_ERR_EXIT(SOC_E_UNAVAIL);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_mdix_status_get(int unit, soc_port_t port, soc_port_mdix_status_t *status)
{
    int rv;
    int ext_phy_present = 0;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_is_legacy_ext_phy_present(unit, port, &ext_phy_present);
    SOCDNX_IF_ERR_EXIT(rv);

    if (ext_phy_present) {
        rv = portmod_port_ext_phy_mdix_status_get(unit, port, status);
        SOCDNX_IF_ERR_EXIT(rv);
    } else {
        /* MDIX is supported for external phys only, so we return a fixed
         * value for internal phys
         */
        *status = SOC_PORT_MDIX_STATUS_NORMAL;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_pfc_refresh_set(int unit, soc_port_t port, int value)
{
    portmod_pfc_control_t control;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_pfc_control_get(unit, port, &control));
    if(value == 0) { /* disable PFC refresh */
        control.refresh_timer = -1;
    } else {
        control.refresh_timer = value;
    }
    SOCDNX_IF_ERR_EXIT(portmod_port_pfc_control_set(unit, port, &control));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_portmod_pfc_refresh_get(int unit, soc_port_t port, int* value)
{
    portmod_pfc_control_t control;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_pfc_control_get(unit, port, &control));
    if(control.refresh_timer == -1) { /* disable PFC refresh */
        *value = 0;
    } else {
        *value = control.refresh_timer;
    }

exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_port_phy_reset(int unit, soc_port_t port) 
{
    int rv;
    SOCDNX_INIT_FUNC_DEFS;

    rv = portmod_port_reset_set(unit, port, 0, 0, 0); 
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(SOC_E_UNAVAIL);

exit:
    SOCDNX_FUNC_RETURN; 
}

int 
soc_jer_port_mac_sa_set(int unit, int port, sal_mac_addr_t mac_sa)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_tx_mac_sa_set(unit, port, mac_sa));
    SOCDNX_IF_ERR_EXIT(portmod_port_rx_mac_sa_set(unit, port, mac_sa));

exit:
    SOCDNX_FUNC_RETURN;
}

int 
soc_jer_port_mac_sa_get(int unit, int port, sal_mac_addr_t mac_sa)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_rx_mac_sa_get(unit, port, mac_sa));

exit:
    SOCDNX_FUNC_RETURN;
}

/* ILKN port takes over QMLF memory. (ILKN0 => QMLF 0-1, ILKN1 => QMLF 3-4)
 * ETH cannot work on these quads with ILKN. if using same quad for data and tdm, ILKN only use 1 quad's memory*/
STATIC int
soc_jer_nif_ilkn_vs_eth_collision_resolve(int unit, int port, int ilkn_id, int* free_quad)
{
    uint32 port_i, phy_port, phy_lane, num_phys;
    soc_pbmp_t all_valid_ports;
    int first_phy_quad, quad, ilkn_quad, is_collision = 0;
    int first_quad, found_free_quad = 0;
    SOCDNX_INIT_FUNC_DEFS;

    *free_quad = -1;
    first_quad = ilkn_id * 3;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy_port, &phy_lane)));
    ilkn_quad = (phy_lane - 1) / NUM_OF_LANES_IN_PM;

    /* optimization - check if ILKN is already on one of the designated QMLFs*/
    for (quad = first_quad; quad < first_quad + 3; ++quad) {
        if (ilkn_quad == quad) {
            *free_quad = quad % 3;
            SOC_EXIT;
        }
    }
    /* search for collision between ethernet ports and ILKN port*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, 0, &all_valid_ports));

    for (quad = first_quad; quad < first_quad + 3; ++quad) {
        is_collision = 0;
        SOC_PBMP_ITER(all_valid_ports, port_i){
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port_i, &num_phys));
            if (num_phys && port_i != port) {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port_i, &phy_port)); 
                SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy_port, &phy_lane)));
                first_phy_quad = (phy_lane - 1) / NUM_OF_LANES_IN_PM;
                /*collision-> move to next quad*/
                if (first_phy_quad == quad) {
                    found_free_quad = 0;
                    is_collision = 1;
                    break;
                }
            }
        }
        if (is_collision) {
            continue;
        } else {
            found_free_quad = 1;
            break;
        }
    }

    if (found_free_quad == 1) {
        /*return a number between 0-2*/
        *free_quad = (quad % 3);
    } else {
        SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Interface ILKN%d collides with ports in quads %d, %d, %d. please make one of these quads available for ILKN"),
                                            ilkn_id, first_quad, first_quad + 1, first_quad + 2));
    }
exit:
    SOCDNX_FUNC_RETURN;
}


/* The purpose of this function is to verify that the added port is not conflicting with allocated ILKN QMLF
 * QMLF allocation theory (each line is stronger than the lines following it):
 * If ILKN is not in use or ILKN in KBP format                                                => 0 QMLFs are in use
 * else, if tdm and data are sharing the same QMLF according to
 *   NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF (and by implementation num_of_lanes <= 12) => 1 QMLFs is in use, according to NBIL_HRF_RX_MEM_CONFIG
 * else, if num_of_lanes <= 12 (and from above - tdm is using a dedicated channel)            => 2 QMLFs are in use: 0 and 1
 * else, if num_of_lanes > 12                                                                 => 3 QMLFs are in use: 0, 1 and 2
 * else, not supported. We shouldn't get here
 *
 *
*/
STATIC int
soc_jer_portmod_check_for_qmlf_conflict(int unit, int new_port)
{

    uint32 num_lanes, is_tdm_and_data_sharing_hrf, new_port_offset_within_trio, reg_val;
    uint32 new_port_first_phy_qsgmii, new_port_first_phy, new_port_trio;
    uint32 QMLF_bit_map, conflict_port_bit;
    uint32 flags, new_port_PM, offset;
    soc_port_t reg_port;
    soc_reg_t reg;
    int nof_segments;
    soc_port_t ilkn_logical_port;


    SOCDNX_INIT_FUNC_DEFS;
    if (SOC_IS_QUX(unit)) {   
        SOCDNX_FUNC_RETURN;
    }

    /*Getting the PM of the new (non-ILKN) port*/
    SOCDNX_IF_ERR_EXIT (soc_port_sw_db_first_phy_port_get(unit, new_port, &new_port_first_phy_qsgmii));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove(unit, new_port_first_phy_qsgmii, &new_port_first_phy));
    new_port_PM = (new_port_first_phy - 1) / NUM_OF_LANES_IN_PM;
    new_port_trio = new_port_PM/3; /*new port trio is the non-ILKN equivalent for ILKN offset*/
    new_port_offset_within_trio = new_port_PM % 3;

    /*Checking - do we have ILKN on this offset (i.e. group of 3 PMs for which the new PM belongs)*/
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, new_port_trio, &ilkn_logical_port));
    if (ilkn_logical_port != SOC_JER_INVALID_PORT) {                /* There is ILKN on this 3 PM offsets! */
        /* Now, if we have ILKN on this 3 PM group - check that its QMLF doesn't conflict with new port PM */

        /*Get info on ILKN port*/
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, ilkn_logical_port, &num_lanes));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, ilkn_logical_port, &flags));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, ilkn_logical_port, 0, &offset));
        reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4); /*Any for 0-1 (NBIH), 0 for 2-3 (NBIL0), 1 for 4-5 (NBIL1)*/
        SOCDNX_IF_ERR_EXIT(READ_NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, &reg_val));
        is_tdm_and_data_sharing_hrf = soc_reg_field_get(unit, NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, reg_val, tdm_data_hrf_fields[offset]);

        /*Sanity test - is offset is correct*/
        if (offset != new_port_trio){
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Incompatibility between ILKN offset %d and new_port_trio %d"),offset, new_port_trio));
        }

        /*Getting the internal offsets (0-2) of the ILKN QMLF - see description in function header*/
        if (SOC_PORT_IS_ELK_INTERFACE(flags)){
            QMLF_bit_map = 0;
        }else{
            if (is_tdm_and_data_sharing_hrf){
                reg = (offset < 2) ? NBIH_HRF_RX_MEM_CONFIGr : NBIL_HRF_RX_MEM_CONFIGr;
                SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, offset & 1, &reg_val));
                QMLF_bit_map = reg_val;
            }else{
                nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];
                if (nof_segments == 2) {
                    QMLF_bit_map = QMLF_0_BIT | QMLF_1_BIT;
                }else{
                    QMLF_bit_map = QMLF_0_BIT | QMLF_1_BIT | QMLF_2_BIT;
                }
            }
        }

        /*Checking that ILKN QMLF doesn't conflict with new port PM*/
        conflict_port_bit = QMLF_bit_map & (1 << new_port_offset_within_trio);
        if (conflict_port_bit) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("New port %d is using PM %d. However, this PM is occupied by ILKN QMLF since ILKN %d is using its %d PM!"),
                    new_port, new_port_PM, offset, soc_sand_log2_round_up(conflict_port_bit)));
        }

    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_port_ilkn_tdm_and_data_share_hrf_get(int unit, int port, int* shr_hrf)
{
    uint32 offset;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));

    *shr_hrf = soc_property_port_suffix_num_get(unit, offset, 0, "custom_feature", "ilkn_tdm_and_data_shr_hrf", 0); 
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_port_ilkn_hrf_config_close_path(int unit, int port, int offset){

    int shr_hrf;
    soc_field_t field = INVALIDf;
    soc_port_t reg_port;
    soc_reg_t reg;
    uint32 reg_val, num_lanes, nof_channels;
    bcm_pbmp_t pbmp;
    soc_port_t port_i, second_ilkn_port;
    uint32 tdm_channel_exist = 0, close_only_tdm_path = 0, flags;
    int nof_segments;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(pbmp);
    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4);

    /* Configure HRFs and TDM in NBI (for every active ILKN core) */
    /* if ilkn 0/1 configure only nbih, else configure both nbil(with 0/1) and nbih(with 2-5) */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
    nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];

    if (nof_segments == 2) {

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &pbmp));
        SOC_PBMP_COUNT(pbmp, nof_channels);
        SOC_PBMP_ITER(pbmp, port_i) {
            if(IS_TDM_PORT(unit, port_i)) {
                if ((port_i == port)) {
                    if (nof_channels > 1) {
                        /*close_only_tdm_path means were closing path of last TDM channel, but not last total channel*/
                        close_only_tdm_path = 1;
                    }
                    /* dont count the current channel as tdm, 
                     * since we are about to close it */
                    continue;
                }
                tdm_channel_exist = 1; 
            }
        }

        /* enabling an ILKN port can sometimed require configurations for the other ILKN of the same core
        before removing an ILKN port, we need to find out if the other ILKN port is enabled */
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, ((offset & 1) ? offset - 1 : offset + 1), &second_ilkn_port));
        if ((second_ilkn_port != SOC_JER_INVALID_PORT) || ((nof_channels > 1) && (close_only_tdm_path == 0))) {
            SOC_EXIT;
        }

        SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_tdm_and_data_share_hrf_get(unit, port, &shr_hrf));
        if (!tdm_channel_exist || shr_hrf) {
            /* There are several registers in the BNI in which the NBIH controls all 6 ILKN ports and each NBIL also control the 2 ILKN ports on it.      *
             * in such cases, all ILKN ports should configure the NBIH register, and ILKN ports in NBIL also need to be configured with the NBIL register */
            SOCDNX_IF_ERR_EXIT(READ_NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, &reg_val)); 
            soc_reg_field_set(unit, NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, tdm_data_hrf_fields[offset], close_only_tdm_path ? 1 : 0);
            if ((offset == 0) && (nof_segments == 2) && (!SOC_IS_JERICHO_PLUS_ONLY(unit))) {
                /* in Jericho, ILKN0/2/4 always get 4 segments, and then ILKN1/3/5 cannot work. 
                 * in order to give ILKN0/2/4 only 2 segements, we enable ILKN1/3/5 even if it is not a real port. 
                 * fot this virtual port, we also want to occupy the least QMLFs memory,                                                .
                 * so we set this bit for the virtual port as well. 
                 * in Jer_plus this logic is fixed so no need for this configuration */
                  soc_reg_field_set(unit, NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, tdm_data_hrf_fields[offset + 1], close_only_tdm_path ? 1 : 0);
            }
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, reg_val));

            if (offset > 1) {
                field = (offset & 1) ? ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_1f : ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_0f;
                SOCDNX_IF_ERR_EXIT(READ_NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, reg_port, &reg_val)); 
                soc_reg_field_set(unit, NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, field, close_only_tdm_path ? 1 : 0);
                if (((offset & 1) == 0) && (nof_segments == 2) && (!SOC_IS_JERICHO_PLUS_ONLY(unit))) {
                    soc_reg_field_set(unit, NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_1f, close_only_tdm_path ? 1 : 0);
                }
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, reg_port, reg_val));
            }
        }
        if (!close_only_tdm_path /*close full path, not just tdm*/) {
            reg = (offset < 2) ? NBIH_HRF_RX_MEM_CONFIGr : NBIL_HRF_RX_MEM_CONFIGr; 
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, offset & 1, 0x0)); /*Take no QMLF mamory*/
        
            reg = (offset < 2) ? NBIH_HRF_TX_MEM_CONFIGr : NBIL_HRF_TX_MEM_CONFIGr;
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, offset & 1, 0x0)); /*Take no QMLF mamory*/
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int 
soc_jer_port_ilkn_is_tdm_channel_exist_get(int unit, int port, int *tdm_channel_exist)
{
    soc_port_t port_i;
    uint32 nof_channels;
    soc_pbmp_t pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    *tdm_channel_exist = 0;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &pbmp));
    SOC_PBMP_COUNT(pbmp, nof_channels);
    SOC_PBMP_ITER(pbmp, port_i) {
        if(IS_TDM_PORT(unit, port_i)) {
            *tdm_channel_exist = 1;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int 
soc_jer_port_ilkn_hrf_config_open_path(int unit, int port, int offset)
{
    int free_quad_for_ilkn = -1, shr_hrf, open_tdm_path = 0;
    soc_field_t field;
    soc_port_t reg_port;
    soc_reg_t reg;
    uint32 reg_val, num_lanes;
    bcm_pbmp_t pbmp;
    soc_port_t port_i;
    uint32 tdm_channel_exist = 0, is_master = 0;
    uint32 flags, nof_channels = 0;
    int nof_segments;
    SOCDNX_INIT_FUNC_DEFS;

    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4);

    /* Configure HRFs and TDM in NBI (for every active ILKN core) */
    /* if ilkn 0/1 configure only nbih, else configure both nbil(with 0/1) and nbih(with 2-5) */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_master_get(unit, port, &is_master));
    nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];

    if (nof_segments == 2) {

        if (!SOC_PORT_IS_ELK_INTERFACE(flags)) {
            if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
                free_quad_for_ilkn = 0;
            } else {
                SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_vs_eth_collision_resolve(unit, port, offset, &free_quad_for_ilkn));
            }
        }

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &pbmp));
        SOC_PBMP_COUNT(pbmp, nof_channels);
        SOC_PBMP_ITER(pbmp, port_i) {
            if(IS_TDM_PORT(unit, port_i)) {
                if ((port_i == port)) {
                    if (nof_channels > 1) {
                        /*open_tdm_path means were opening path of first TDM channel, but not first total channel*/
                        open_tdm_path = 1;
                        /* dont count the current channel as tdm, since we are about to open it */
                        continue;
                    }
                }
                tdm_channel_exist = 1;
            }
        }
        if (!is_master && !open_tdm_path) {
            /*only cases meant to perform this function are first channel and first TDM channel*/
            SOC_EXIT;
        }
        if (free_quad_for_ilkn != 0 && open_tdm_path) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("When TDM present, QMLF's %d and %d should be available for ILKN"), (offset*3), (offset*3)+1));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_tdm_and_data_share_hrf_get(unit, port, &shr_hrf)); 
        if (!tdm_channel_exist || shr_hrf) {
            /* There are several registers in the BNI in which the NBIH controls all 6 ILKN ports and each NBIL also control the 2 ILKN ports on it.      *
             * in such cases, all ILKN ports should configure the NBIH register, and ILKN ports in NBIL also need to be configured with the NBIL register */
            SOCDNX_IF_ERR_EXIT(READ_NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, &reg_val)); 
            soc_reg_field_set(unit, NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, tdm_data_hrf_fields[offset], open_tdm_path ? 0 : 1);
            if ((offset == 0) && (nof_segments == 2) && (!SOC_IS_JERICHO_PLUS_ONLY(unit))) {
                /* in Jericho, ILKN0/2/4 always get 4 segments, and then ILKN1/3/5 cannot work. 
                 * in order to give ILKN0/2/4 only 2 segements, we enable ILKN1/3/5 even if it is not a real port. 
                 * fot rhis virtual port, we also want to occupy the least QMLFs memory,                                                .
                 * so we set this bit for the virtual port as well. 
                 * in Jer_plus this logic is fixed so no need for this configuration */
                  soc_reg_field_set(unit, NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, tdm_data_hrf_fields[offset + 1], open_tdm_path ? 0 : 1);
            }
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, reg_val));

            if (offset > 1) {
                field = (offset & 1) ? ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_1f : ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_0f;
                SOCDNX_IF_ERR_EXIT(READ_NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, reg_port, &reg_val)); 
                soc_reg_field_set(unit, NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, field, open_tdm_path ? 0 : 1);
                if (((offset & 1) == 0) && (nof_segments == 2) && (!SOC_IS_JERICHO_PLUS_ONLY(unit))) {
                    soc_reg_field_set(unit, NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr, &reg_val, ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRF_ILKN_1f, open_tdm_path ? 0 : 1);
                }
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ILKN_RX_TDM_AND_DATA_TRAFFIC_ON_SAME_HRFr(unit, reg_port, reg_val));
            }
        }
    } else {
        free_quad_for_ilkn = 0; /*HW default*/
    }
    if (free_quad_for_ilkn != -1 && (!tdm_channel_exist || shr_hrf) && !SOC_PORT_IS_ELK_INTERFACE(flags)) {
        reg = (offset < 2) ? NBIH_HRF_RX_MEM_CONFIGr : NBIL_HRF_RX_MEM_CONFIGr;
        /*first zero the reg from the default value*/
        reg_val = 0;
        if (free_quad_for_ilkn == 0) {
            field = HRF_RX_MEM_0_HRF_Nf; 
        } else if (free_quad_for_ilkn == 1) {
            field = HRF_RX_MEM_1_HRF_Nf;
        } else /*free_quad_for_ilkn == 2*/ {
            field = HRF_RX_MEM_2_HRF_Nf;
        }
        soc_reg_field_set(unit, reg, &reg_val, field, 1); 
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, offset & 1, reg_val));
        
        reg = (offset < 2) ? NBIH_HRF_TX_MEM_CONFIGr : NBIL_HRF_TX_MEM_CONFIGr;
        /*first zero the reg from the default value*/
        reg_val = 0;
        if (free_quad_for_ilkn == 0) {
            field = HRF_TX_MEM_0_HRF_Nf; 
        } else if (free_quad_for_ilkn == 1) {
            field = HRF_TX_MEM_1_HRF_Nf;
        } else /*free_quad_for_ilkn == 2*/ {
            field = HRF_TX_MEM_2_HRF_Nf;
        }
        soc_reg_field_set(unit, reg, &reg_val, field, 1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, offset & 1, reg_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_sch_config_for_tdm_hrfs_set(int unit, uint32 ilkn_id, int core)
{
    static soc_field_t HRF_RX_PIPE_Nl[] = { HRF_RX_PIPE_2f, HRF_RX_PIPE_3f, HRF_RX_PIPE_6f, HRF_RX_PIPE_7f, HRF_RX_PIPE_10f, HRF_RX_PIPE_11f};
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT( soc_reg_field32_modify(unit, NBIH_RX_SCH_CONFIG_FOR_TDM_HRFSr, REG_PORT_ANY, HRF_RX_PIPE_Nl[ilkn_id], core));

exit:
    SOCDNX_FUNC_RETURN;

}

int
soc_jer_port_ilkn_over_fabric_set(int unit, soc_port_t port, uint32 ilkn_id) {

    uint32 first_phy_port;
    soc_pbmp_t phys;
    soc_port_t fabric_port;
    int nof_fabric_ilkn_pms, i;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy_port));
    
    if (first_phy_port >= SOC_DPP_FIRST_FABRIC_PHY_PORT(unit) && ilkn_id >= 4) {
        SOC_DPP_CONFIG(unit)->jer->nif.ilkn_over_fabric[ilkn_id & 1] = 1;

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));

        nof_fabric_ilkn_pms = SOC_IS_QMX(unit) ? SOC_QMX_PM_FABRIC : MAX_NUM_OF_PMS_IN_ILKN;
        /* disable port in SOC_PBMP_SFI */
        for (i = 0; i < nof_fabric_ilkn_pms * NUM_OF_LANES_IN_PM; ++i) {
            fabric_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id) + i;
            if (SOC_PBMP_MEMBER(phys, fabric_port)) {
                SOC_PBMP_PORT_REMOVE(SOC_PORT_BITMAP(unit, sfi), fabric_port); 
                SOC_PBMP_PORT_REMOVE(SOC_PORT_BITMAP(unit, port), fabric_port);
                SOC_PBMP_PORT_REMOVE(SOC_PORT_BITMAP(unit, all), fabric_port);
            }
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_protocol_offset_verify(int unit, soc_port_t port, uint32 protocol_offset) {
    uint32 phy_port;
    uint32 lane_id;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));

    SOCDNX_IF_ERR_EXIT(MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy_port, &lane_id)));
    
    if (!SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, protocol_offset) &&
        protocol_offset / ARAD_NIF_NUM_OF_OFFSETS_IN_PROTOCOL_GROUP != (lane_id - 1) / ARAD_NIF_NUM_OF_LANES_IN_PROTOCOL_GROUP) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Protocol offset %d is out of range for the given interface"), protocol_offset)); 
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_ilkn_serdes_qmlfs_get( int unit, soc_pbmp_t* phy_pbmp, SHR_BITDCL* serdes_qmlfs)
{
    int i;
    soc_pbmp_t phy_pbmp_offset_removed_1_based;

    SOCDNX_INIT_FUNC_DEFS;

    /* Clear destination bit map */
    SHR_BITCLR_RANGE(serdes_qmlfs, 0, SOC_JERICHO_NOF_QMLFS);

    /* Remove offset */
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, phy_pbmp, &phy_pbmp_offset_removed_1_based));

    /* Fill destination bit map according to lane bit map, 4 lanes per qmlf */
    SOC_PBMP_ITER(phy_pbmp_offset_removed_1_based, i) {
        SHR_BITSET(serdes_qmlfs, (i-1)/4);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
soc_jer_ilkn_memory_qmlfs_get(int unit, soc_port_t port, SHR_BITDCL* memory_qmlfs)
{
    uint32 ilkn_id = 0, nof_lanes = 0, flags = 0;
    int ilkn_memory_quad = -1;
    int tdm_channel_exist = 0, shr_hrf = 0;
    SOCDNX_INIT_FUNC_DEFS;

    /* Clear destination bit map */
    SHR_BITCLR_RANGE(memory_qmlfs, 0, SOC_JERICHO_NOF_QMLFS);

    /* Check if ILKN is ELK port*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
    if (SOC_PORT_IS_ELK_INTERFACE(flags)) {
        /* ELK port get no mwmory quad */
        SOC_EXIT;
    }

    /* Get ILKN id */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &ilkn_id));

    /* Get nof lanes*/
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &nof_lanes));

    if (nof_lanes > 12) {
        /* ILKN port with >12 lanes always takes quads 0,2 from the three possible quads */
        SHR_BITSET(memory_qmlfs, (ilkn_id * 3)); 
        SHR_BITSET(memory_qmlfs, 2 + (ilkn_id * 3)); 
    }
    /* Check if we have TDM channel */
    SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_is_tdm_channel_exist_get(unit, port, &tdm_channel_exist));
    
    if (tdm_channel_exist) {
        /* Check if the TDM and Data are sharing HRF */
        SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_tdm_and_data_share_hrf_get(unit, port, &shr_hrf));
    }

    if (tdm_channel_exist && !shr_hrf) {
        /* TDM port should take quads 0,1 from the three possible quads,
           But we still allocate only the memory quad used for Data (quad 0),
           since the memory quad used for TDM (quad 1) can be required for ETH
           if all TDM channels are dinamically removed. */
        SHR_BITSET_RANGE(memory_qmlfs, (ilkn_id * 3), 1);
    }

    if (nof_lanes <= 12 && (!tdm_channel_exist || shr_hrf)) {
        /* Find ILKN free memory quad*/
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_vs_eth_collision_resolve(unit, port, ilkn_id, &ilkn_memory_quad));
    }

    if (ilkn_memory_quad != -1) {
        /* Set the ilkn memory quad to pbmp */
        SHR_BITSET(memory_qmlfs, ilkn_memory_quad + (ilkn_id * 3));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_open_ilkn_path(int unit, int port) {

    ARAD_PORTS_ILKN_CONFIG *ilkn_config;
    int is_tdm;
    soc_port_t reg_port, phy, link, second_ilkn_port;
    soc_pbmp_t phys, phy_lanes;
    soc_pbmp_t ilkn_over_fabric_ports;
    soc_reg_t reg;
    soc_field_t field;
    soc_reg_above_64_val_t reg_above_64_val;
    uint64 reg64_val;
    uint64 reachability_allowed_bm;
    uint32 reg_val, offset, retrans_multiply_tx, il_over_fabric;
    uint32 is_master, base_index, num_lanes, egr_if, fld_val[1], nof_segments;
    int mubits_tx_polarity, mubits_rx_polarity, fc_tx_polarity, fc_rx_polarity, core, shr_hrf;
    ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    soc_error_t rv;
    SHR_BITDCL* serdes_qmlfs;
    SHR_BITDCL* memory_qmlfs;
    soc_jer_ilkn_qmlf_resources_t *ilkn_qmlf_resources = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_qmlf_resources;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(phys);
    SOC_PBMP_CLEAR(phy_lanes);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_master_get(unit, port, &is_master));

    /* offset = 0/1 -> ILKN_PMH, offset = 2/3 -> ILKN_PML0, offset = 4/5 -> ILKN_PML1 */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    ilkn_config = &SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn[offset];
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_nof_segments_calc(unit, port, &nof_segments));

    SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset] = nof_segments;
        
    /* ILKN0/1 -> port=REG_PORT_ANY, ILKN2/3 -> port=0, ILKN4/5 -> port=1 */
    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4); 

    if (is_master) {
        SOC_PBMP_PORT_ADD(SOC_INFO(unit).custom_reg_access.custom_port_pbmp, port);
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
        SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &phy_lanes));

        /* init ilkn qmlfs */
        /* for ILKN over Fabric leave all serdes QMLFs to be taken by Eth ports*/
        if (!SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            serdes_qmlfs = ilkn_qmlf_resources[offset].serdes_qmlfs;
            SOCDNX_IF_ERR_EXIT(soc_jer_ilkn_serdes_qmlfs_get(unit, &phys, serdes_qmlfs));
        }
        memory_qmlfs = ilkn_qmlf_resources[offset].memory_qmlfs;
        SOCDNX_IF_ERR_EXIT(soc_jer_ilkn_memory_qmlfs_get(unit, port, memory_qmlfs));

        /* First - zero out the memory chosen by the ILKN in the NBI -
         * to avoid using the wrong memory when the ILKN is out-of-reset 
         * this value will be overriden in soc_jer_port_ilkn_hrf_config_open_path() 
         */
        reg = (offset < 2) ? NBIH_HRF_RX_MEM_CONFIGr : NBIL_HRF_RX_MEM_CONFIGr; 
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, offset & 1, 0x0)); /*Take no QMLF mamory*/
    
        reg = (offset < 2) ? NBIH_HRF_TX_MEM_CONFIGr : NBIL_HRF_TX_MEM_CONFIGr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, offset & 1, 0x0)); /*Take no QMLF mamory*/

        /* Wake up relevant wrapper */
        field = (offset & 1) ? ILKN_1_PORT_RSTNf : ILKN_0_PORT_RSTNf;
        reg = (offset < 2 ) ? ILKN_PMH_ILKN_RESETr : ILKN_PML_ILKN_RESETr;

        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
        soc_reg_field_set(unit, reg, &reg_val, field, 1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));
        /* Fabric mux - only in PML1 */
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_PML_ILKN_OVER_FABRICr(unit, 1, &reg_val));
            il_over_fabric = soc_reg_field_get(unit, ILKN_PML_ILKN_OVER_FABRICr, reg_val, ILKN_OVER_FABRICf);
            SOC_PBMP_ITER(phy_lanes, phy){
                il_over_fabric |= 1 << (((phy)  % JER_NIF_ILKN_MAX_NOF_LANES) + (SOC_IS_QMX(unit) ? 8 : 0));
            }
    
            soc_reg_field_set(unit, ILKN_PML_ILKN_OVER_FABRICr, &reg_val, ILKN_OVER_FABRICf, il_over_fabric);
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_PML_ILKN_OVER_FABRICr(unit, 1, reg_val));
             
            SOC_PBMP_CLEAR(ilkn_over_fabric_ports);    
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_over_fabric_pbmp_get(unit, &ilkn_over_fabric_ports));
        
            SOCDNX_IF_ERR_EXIT(READ_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit,&reachability_allowed_bm));
            COMPILER_64_NOT(reachability_allowed_bm);
            SOC_PBMP_ITER(ilkn_over_fabric_ports, link){
                if ((link - SOC_DPP_DEFS_GET(unit, first_fabric_link_id) - FABRIC_LOGICAL_PORT_BASE(unit) ) <= SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit)) {
                    COMPILER_64_BITSET(reachability_allowed_bm, (link - SOC_DPP_DEFS_GET(unit, first_fabric_link_id) - FABRIC_LOGICAL_PORT_BASE(unit) )); 
                } else {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Invalid ILKN over fabric lane"))); 
                }
            }
            COMPILER_64_NOT(reachability_allowed_bm);
            SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_REG_010Fr(unit, reachability_allowed_bm));
            SOCDNX_IF_ERR_EXIT(WRITE_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit, reachability_allowed_bm));
        }
        /* Enable ILKN in NBIH */
        SOCDNX_IF_ERR_EXIT(READ_NBIH_ENABLE_INTERLAKENr(unit, &reg_val)); 
        soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, enable_ilkn_fields[offset], 1);
        /* in Jericho, ILKN 0/2/4 always get 4 segments, and then ILKN 1/3/5 cannot work. 
        * in order to give ILKN 0/2/4 only 2 segements, we enable ILKN 1/3/5 even if it is not a real port.
        * in Jer_plus this logic is fixed and it is possible to assign only 2 segments to ILKN0/2/4 
        * it is needed to set this fields even if only ILKN 1/3/5 is active*/ 
        if (offset < 2 && SOC_IS_JERICHO_PLUS_ONLY(unit) && (nof_segments == 2)) {
            soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_RXf, 1); 
            soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_TXf, 1); 
        } else if (((offset & 1) == 0) && (nof_segments == 2)) {
            /*Check if we have actually configured the small ILKN*/
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, offset + 1, &second_ilkn_port));
            if (second_ilkn_port == -1) {
                /*We make sure that when we enable the small (not real) ILKN port 
                  it wont actually take an HRF and interfere with the QMLF logic*/
                reg = (offset < 2) ? NBIH_HRF_RX_MEM_CONFIGr : NBIL_HRF_RX_MEM_CONFIGr;
                SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 1, 0x0)); /*Take no QMLF memory for the dummy ILKN port*/
                reg = (offset < 2) ? NBIH_HRF_TX_MEM_CONFIGr : NBIL_HRF_TX_MEM_CONFIGr;
                SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 1, 0x0)); /*Take no QMLF memory for the dummy ILKN port*/

                soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, &reg_val, enable_ilkn_fields[offset + 1], 1);
            }
        }
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ENABLE_INTERLAKENr(unit, reg_val));

        if(offset >= 2) {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_ENABLE_INTERLAKENr(unit, reg_port, &reg_val)); 
            soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, enable_ilkn_fields[offset & 1], 1);
            if (SOC_IS_JERICHO_PLUS_ONLY(unit) && (nof_segments == 2)) {
                soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_RXf, 1); 
                soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_TXf, 1); 
            } else if (((offset & 1) == 0) && (nof_segments == 2)) {
                soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, &reg_val, ENABLE_PORT_1f, 1);
            }
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ENABLE_INTERLAKENr(unit, reg_port, reg_val));
        }

        /* Config ILKN ports in NBI */
        reg = (offset < 2) ? NBIH_HRF_TX_CONFIG_HRFr : NBIL_HRF_TX_CONFIG_HRFr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, (offset & 1), &reg_val)); 
        soc_reg_field_set(unit, reg, &reg_val, HRF_TX_NUM_CREDITS_TO_EGQ_HRF_Nf, ilkn_config->retransmit.enable_tx ? 0x400 : 0x200);
        if (ilkn_config->retransmit.enable_tx) {                                                 
            soc_reg_field_set(unit, reg, &reg_val, HRF_TX_USE_EXTENDED_MEM_HRF_Nf, 1); 
        }
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, (offset & 1), reg_val));

        switch(ilkn_config->retransmit.nof_seq_number_repetitions_tx) {
        case 1:
            retrans_multiply_tx = 0;
            break;
        case 2:
            retrans_multiply_tx = 1;
            break;
        case 4:
            retrans_multiply_tx = 2;
            break;
        case 8:
            retrans_multiply_tx = 3;
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid nof_seq_number_repetitions value %d"),ilkn_config->retransmit.nof_seq_number_repetitions_rx));
            break;
        }
        reg = (offset < 2) ? NBIH_ILKN_TX_RETRANSMIT_CONFIG_HRFr : NBIL_ILKN_TX_RETRANSMIT_CONFIG_HRFr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, (offset & 1), &reg_val));
        soc_reg_field_set(unit, reg, &reg_val, ILKN_TX_RETRANS_ENABLE_HRF_Nf, ilkn_config->retransmit.enable_tx);
        soc_reg_field_set(unit, reg, &reg_val, ILKN_TX_RETRANS_NUM_ENTRIES_TO_SAVE_HRF_Nf, ilkn_config->retransmit.buffer_size_entries);
        soc_reg_field_set(unit, reg, &reg_val, ILKN_TX_RETRANS_MULTIPLY_HRF_Nf, retrans_multiply_tx);
        soc_reg_field_set(unit, reg, &reg_val, ILKN_TX_RETRANS_WAIT_FOR_SEQ_NUM_CHANGE_HRF_Nf, ilkn_config->retransmit.tx_wait_for_seq_num_change);
        soc_reg_field_set(unit, reg, &reg_val, ILKN_TX_RETRANS_IGNORE_REQ_WHEN_ALMOST_EMPTY_HRF_Nf, ilkn_config->retransmit.tx_ignore_requests_when_fifo_almost_empty);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, (offset & 1), reg_val));
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_hrf_config_open_path(unit, port, offset));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
    if (is_master) {
        /* configure by default ilkn nif priority to high */
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_priority_set( unit, core, offset, 1, 0, 1, JER_NIF_PRIO_HIGH_LEVEL));
    }
    SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_tdm_and_data_share_hrf_get(unit, port, &shr_hrf));
    is_tdm = (IS_TDM_PORT(unit, port) && !shr_hrf);

    if (is_tdm) {
        /* if ilkn is tdm add tdm nif priority */
        SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_set( unit, core, offset, 1, 0, 1, JER_NIF_PRIO_TDM_LEVEL));

        /* configure the pipe id for the tdm hrf */
        SOCDNX_IF_ERR_EXIT( soc_jer_sch_config_for_tdm_hrfs_set(unit, offset, core));
    }

    reg = (offset < 2) ? NBIH_HRF_RX_CONFIG_HRFr : NBIL_HRF_RX_CONFIG_HRFr;
    /* HRF 0-1 -> data traffic, HRF 2-3 -> tdm traffic */
    base_index = (is_tdm) ? 2 : 0;
    SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, base_index + (offset & 1), &reg64_val)); 
    soc_reg64_field32_set(unit, reg, &reg64_val, HRF_RX_BURST_MERGE_EN_HRF_Nf, 1);
    soc_reg64_field32_set(unit, reg, &reg64_val, HRF_RX_BURST_MERGE_TH_HRF_Nf, 0x8); /*default val for now*/
    soc_reg64_field32_set(unit, reg, &reg64_val, HRF_RX_BURST_MERGE_FORCE_HRF_Nf, ilkn_config->interleaved ? 0 : 1);
    SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, base_index + (offset & 1), reg64_val));

    /* set_ilkn_tx_hrf_rstn - Make sure HRF is not enabled before it is configured */
    reg = (offset < 2) ? NBIH_HRF_RESETr : NBIL_HRF_RESETr;
    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val)); 
    field = (offset & 1) ? HRF_TX_1_CONTROLLER_RSTNf : HRF_TX_0_CONTROLLER_RSTNf;
    soc_reg_field_set(unit, reg, &reg_val, field, 1);
    field = (offset & 1) ? HRF_RX_1_CONTROLLER_RSTNf : HRF_RX_0_CONTROLLER_RSTNf;
    soc_reg_field_set(unit, reg, &reg_val, field, 1);
    if (is_tdm) {
        field = (offset & 1) ? HRF_RX_3_CONTROLLER_RSTNf : HRF_RX_2_CONTROLLER_RSTNf;
        soc_reg_field_set(unit, reg, &reg_val, field, 1); 
    }
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));

    /* All HRFs indications in NBIF for SCH */
    /* set_ilkn_rx_hrf_en */
    switch (offset) {
    case 0:
        field = (is_tdm) ? RX_HRF_ENABLE_HRF_2f : RX_HRF_ENABLE_HRF_0f;
        break;
    case 1:
        field = (is_tdm) ? RX_HRF_ENABLE_HRF_3f : RX_HRF_ENABLE_HRF_1f;
        break;
    case 2:
        field = (is_tdm) ? RX_HRF_ENABLE_HRF_6f : RX_HRF_ENABLE_HRF_4f;
        break;
    case 3:
        field = (is_tdm) ? RX_HRF_ENABLE_HRF_7f : RX_HRF_ENABLE_HRF_5f;
        break;
    case 4:
        field = (is_tdm) ? RX_HRF_ENABLE_HRF_10f : RX_HRF_ENABLE_HRF_8f;
        break;
    case 5:
        field = (is_tdm) ? RX_HRF_ENABLE_HRF_11f : RX_HRF_ENABLE_HRF_9f;
        break;
    }
    SOCDNX_IF_ERR_EXIT(READ_NBIH_ENABLE_INTERLAKEN_HRFr(unit, &reg_val)); 
    soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKEN_HRFr, &reg_val, field, 1);
    SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ENABLE_INTERLAKEN_HRFr(unit, reg_val));

    if (is_master) {
        /* set_enable_ilkn_port */
        reg = (offset < 2) ? ILKN_PMH_ENABLE_INTERLAKENr : ILKN_PML_ENABLE_INTERLAKENr;
        field = (offset & 1) ? ENABLE_PORT_1f : ENABLE_PORT_0f;

        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, &reg_val));
        soc_reg_field_set(unit, reg, &reg_val, field, 1);
        soc_reg_field_set(unit, reg, &reg_val, ENABLE_CORE_CLOCKf, 1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, reg_val));
    
        /* set_fc_ilkn_cfg */
        mubits_tx_polarity = ilkn_config->mubits_tx_polarity;
        mubits_rx_polarity = ilkn_config->mubits_rx_polarity;
        fc_tx_polarity = ilkn_config->fc_tx_polarity;
        fc_rx_polarity = ilkn_config->fc_tx_polarity;
        if (mubits_tx_polarity || mubits_rx_polarity || fc_tx_polarity || fc_rx_polarity) {
            reg = (offset < 2) ? ILKN_PMH_ILKN_INVERT_POLARITY_SIGNALSr : ILKN_PML_ILKN_INVERT_POLARITY_SIGNALSr; 
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, (offset & 1), &reg_val));
            soc_reg_field_set(unit, reg, &reg_val, ILKN_N_INVERT_RX_MUBITS_POLARITYf, mubits_rx_polarity);
            soc_reg_field_set(unit, reg, &reg_val, ILKN_N_INVERT_TX_MUBITS_POLARITYf, mubits_tx_polarity);
            soc_reg_field_set(unit, reg, &reg_val, ILKN_N_INVERT_RECEIVED_FC_POLARITYf, fc_rx_polarity);
            soc_reg_field_set(unit, reg, &reg_val, ILKN_N_INVERT_TX_FC_POLARITYf, fc_tx_polarity);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, (offset & 1), reg_val));
        }

        /* set_tx_retransmit_enable */
        reg = (offset < 2) ? ILKN_PMH_ILKN_TX_CONFr : ILKN_PML_ILKN_TX_CONFr;
        field = (offset & 1) ? TX_1_RETRANS_ENf : TX_0_RETRANS_ENf;

        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, reg_port, 0, reg_above_64_val)); 
        soc_reg_above_64_field32_set(unit, reg, reg_above_64_val, field, 1);
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, reg, reg_port, 0, reg_above_64_val));

        /* set fragmentation */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
        SOCDNX_IF_ERR_EXIT(rv);
        /* handle ILKN dedicated mode */
        ilkn_tdm_dedicated_queuing = SOC_DPP_CONFIG(unit)->arad->init.ilkn_tdm_dedicated_queuing;
        if (ilkn_tdm_dedicated_queuing == ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) 
        {
            SOC_SAND_SET_BIT(egr_if, 0, 0); /* handle fast ports first */
        }

        SOCDNX_IF_ERR_EXIT(READ_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));

        *fld_val = soc_reg_above_64_field32_get(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, reg_above_64_val, NIF_NO_FRAG_Lf);        
        SHR_BITCLR(fld_val, egr_if);
        soc_reg_above_64_field32_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_above_64_val,NIF_NO_FRAG_Lf, *fld_val);

        SOCDNX_IF_ERR_EXIT(WRITE_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));

        /* handle ILKN dedicated mode */
        if (ilkn_tdm_dedicated_queuing == ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) 
        {
            egr_if++;
            SOCDNX_IF_ERR_EXIT(READ_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));

            *fld_val = soc_reg_above_64_field32_get(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, reg_above_64_val, NIF_NO_FRAG_Lf);        
            SHR_BITCLR(fld_val, egr_if);
            soc_reg_above_64_field32_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_above_64_val,NIF_NO_FRAG_Lf, *fld_val);

            SOCDNX_IF_ERR_EXIT(WRITE_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_open_nbi_path(int unit, int port) {

    const soc_field_t fields[4] = {TX_STOP_DATA_TO_PORT_MACRO_MLF_0_QMLF_Nf, TX_STOP_DATA_TO_PORT_MACRO_MLF_1_QMLF_Nf, TX_STOP_DATA_TO_PORT_MACRO_MLF_2_QMLF_Nf, TX_STOP_DATA_TO_PORT_MACRO_MLF_3_QMLF_Nf};
    int pm_index = 0, reg_port, qmlf_index, core, nbi_inst, max_ports_nbih, nof_lanes_nbi;
    uint32 first_lane; /*1-72*/
    uint64 reg64_val;
    uint32 flags, first_phy_port, reg_val, mode, phys_count;
    soc_reg_t reg;
    soc_pbmp_t phys, lanes;
    soc_port_t lane_i;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    max_ports_nbih = SOC_DPP_DEFS_GET(unit, nof_ports_nbih);
    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy_port));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy_port, &first_lane)));
    --first_phy_port; /* first_phy returned is one-based */
    --first_lane; /* first_lane returned is one-based */

    nbi_inst = first_lane / nof_lanes_nbi; /* NBIH = 0, NBIL0 = 1, NBIL1 = 2*/
    reg_port = (nbi_inst == 0) ? REG_PORT_ANY : nbi_inst - 1;
    pm_index = (first_lane % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;
    qmlf_index = first_lane / NUM_OF_LANES_IN_PM; /*qmlf index in entire NIF (0-17)*/

    /* take MLF out of reset - if port is ELK, SIF or ILKN- no need to take MLF ooo */
    if (!SOC_PORT_IS_ELK_INTERFACE(flags) && !SOC_PORT_IS_STAT_INTERFACE(flags)) {
        if (SOC_IS_QUX(unit)) {
            reg = NIF_TX_QMLF_CONFIGr;
            reg_port = REG_PORT_ANY;
        } else {
            reg = (first_phy_port < max_ports_nbih) ? NBIH_TX_QMLF_CONFIGr : NBIL_TX_QMLF_CONFIGr;
        }

        SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, pm_index, &reg64_val));
        SOC_PBMP_ITER(lanes, lane_i) {
            soc_reg64_field32_set(unit, reg, &reg64_val, fields[(lane_i - 1) % 4], 0);
        }
        SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, pm_index, reg64_val));
    }

    /* Port Mode */
    SOC_PBMP_COUNT(phys, phys_count);
    if (interface_type == SOC_PORT_IF_QSGMII) {
        mode = 3;
    } else if (phys_count == 1) {
        mode = 2; /* Four Ports */
    } else if (phys_count == 2){
        mode = 1; /* Two ports */
    } else {
        mode = 0; /* One Port */
    }
    if (!SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_QMLF_CONFIGr(unit, qmlf_index, &reg_val));
        soc_reg_field_set(unit, NBIH_RX_QMLF_CONFIGr, &reg_val, RX_PORT_MODE_QMLF_Nf, mode);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_RX_QMLF_CONFIGr(unit, qmlf_index, reg_val));

        if(first_phy_port >= max_ports_nbih) {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_QMLF_CONFIGr(unit, reg_port, pm_index, &reg_val));
            soc_reg_field_set(unit, NBIL_RX_QMLF_CONFIGr, &reg_val, RX_PORT_MODE_QMLF_Nf, mode);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_QMLF_CONFIGr(unit, reg_port, pm_index, reg_val));
        }
        reg = (first_phy_port < max_ports_nbih) ? NBIH_TX_QMLF_CONFIGr : NBIL_TX_QMLF_CONFIGr;
    } else {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_QMLF_CONFIGr(unit, pm_index, &reg_val));
        soc_reg_field_set(unit, NIF_RX_QMLF_CONFIGr, &reg_val, RX_PORT_MODE_QMLF_Nf, mode);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_RX_QMLF_CONFIGr(unit, pm_index, reg_val));
        reg = NIF_TX_QMLF_CONFIGr;
        reg_port = REG_PORT_ANY;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, reg_port, pm_index, &reg64_val));
    soc_reg64_field32_set(unit, reg, &reg64_val, TX_PORT_MODE_QMLF_Nf, mode);
    SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, pm_index, reg64_val));

    SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_ports_rstn(unit, port, 1));

exit:
    SOCDNX_FUNC_RETURN;
}

/* receive ILKN protocol offset and return the logical port if exist */ 
int soc_jer_nif_is_ilkn_port_exist(int unit, int ilkn_id, soc_port_t* port)
{
    uint32 offset;
    soc_port_t port_i;
    soc_pbmp_t all_valid_ports;
    SOCDNX_INIT_FUNC_DEFS;

    *port = SOC_JER_INVALID_PORT;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, 0, &all_valid_ports));

    SOC_PBMP_ITER(all_valid_ports, port_i){
        if (IS_IL_PORT(unit, port_i)) {
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port_i, 0, &offset));
            if (offset == ilkn_id) {
                *port = port_i;
                break;
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}



int 
soc_jer_port_close_ilkn_path(int unit, soc_port_t port) {

    int rv;
    int core;
    uint32 nof_channels;
    uint32 egr_if, fld_val[1], reg_val[1], offset, num_lanes;
    soc_reg_above_64_val_t reg_above_64_val;
    ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE ilkn_tdm_dedicated_queuing;
    uint32 first_phy, first_lane;
    int reg_port, i, tdm_channels = 0;
    uint32 il_over_fabric;
    soc_reg_t reg = INVALIDr, phy, link;
    soc_pbmp_t ilkn_over_fabric_ports, channels;
    soc_field_t field = INVALIDf;
    soc_field_t tdm_field = INVALIDf, data_field = INVALIDf;
    soc_pbmp_t phy_ports, phy_lanes;
    SOC_TMC_PORT_HEADER_TYPE hdr_type;
    uint32 base_index;
    uint64 reg64_val;
    uint64 reachability_allowed_bm;
    int index, nof_fabric_ilkn_pms;
    soc_port_t fabric_port, channel_i, second_ilkn_port;
    int nof_segments;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phy_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phy_ports, &phy_lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove(unit, first_phy, &first_lane));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_of_channels_get(unit, port, &nof_channels));
    nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];

    if (nof_channels > 1) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_ports_to_same_interface_get(unit, port, &channels)); 
        SOC_PBMP_ITER(channels, channel_i){
            if (IS_TDM_PORT(unit, channel_i)) {
                ++tdm_channels;
            }
        }
        if (tdm_channels != 1 || !IS_TDM_PORT(unit, port)) {
            /* nothing to do if there is more than one channel and no TDM or more than one TDM channel, 
               or if we have only one TDM channel but the current port is not TDM */
            SOC_EXIT;
        }
    }

    --first_phy; /* first_phy returned is one-based */
    --first_lane; /* first_lane returned is one-based */

    nof_fabric_ilkn_pms = SOC_IS_QMX(unit) ? SOC_QMX_PM_FABRIC : MAX_NUM_OF_PMS_IN_ILKN;
    
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.get(unit, port, &hdr_type); 
    SOCDNX_IF_ERR_EXIT(rv);

    reg_port = (offset < 2) ? REG_PORT_ANY : (offset / 4);

    if (nof_channels == 1) {
        /*unset fragmentation */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
        rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
        SOCDNX_IF_ERR_EXIT(rv);
        /* handle ILKN dedicated mode */
        ilkn_tdm_dedicated_queuing = SOC_DPP_CONFIG(unit)->arad->init.ilkn_tdm_dedicated_queuing;
        if (ilkn_tdm_dedicated_queuing == ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) 
        {
            SOC_SAND_SET_BIT(egr_if, 0, 0); /* handle fast ports first */
        }

        SOCDNX_IF_ERR_EXIT(READ_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));

        *fld_val = soc_reg_above_64_field32_get(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, reg_above_64_val, NIF_NO_FRAG_Lf);        
        SHR_BITSET(fld_val, egr_if);
        soc_reg_above_64_field32_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_above_64_val,NIF_NO_FRAG_Lf, *fld_val);

        SOCDNX_IF_ERR_EXIT(WRITE_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));

        /* handle ILKN dedicated mode */
        if (ilkn_tdm_dedicated_queuing == ARAD_MGMT_ILKN_TDM_DEDICATED_QUEUING_MODE_ON) 
        {
            egr_if++;
             SOCDNX_IF_ERR_EXIT(READ_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));

             *fld_val = soc_reg_above_64_field32_get(unit, EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr, reg_above_64_val, NIF_NO_FRAG_Lf);        
             SHR_BITSET(fld_val, egr_if);
             soc_reg_above_64_field32_set(unit,EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr,reg_above_64_val,NIF_NO_FRAG_Lf, *fld_val);

             SOCDNX_IF_ERR_EXIT(WRITE_EPNI_EGRESS_INTERFACE_NO_FRAGMENTATION_MODE_CONFIGURATIONr(unit, core, reg_above_64_val));
        }


        /* unset_tx_retransmit_enable */
        reg = (offset < 2) ? ILKN_PMH_ILKN_TX_CONFr : ILKN_PML_ILKN_TX_CONFr;
        field = (offset & 1) ? TX_1_RETRANS_ENf : TX_0_RETRANS_ENf;

        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_get(unit, reg, reg_port, 0, reg_above_64_val)); 
        soc_reg_above_64_field32_set(unit, reg, reg_above_64_val, field, 0);
        SOCDNX_IF_ERR_EXIT(soc_reg_above_64_set(unit, reg, reg_port, 0, reg_above_64_val));


        /* unset_fc_ilkn_cfg */
        reg = (offset < 2) ? ILKN_PMH_ILKN_INVERT_POLARITY_SIGNALSr : ILKN_PML_ILKN_INVERT_POLARITY_SIGNALSr; 
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, (offset & 1), reg_val));
        soc_reg_field_set(unit, reg, reg_val, ILKN_N_INVERT_RX_MUBITS_POLARITYf, 0x0);
        soc_reg_field_set(unit, reg, reg_val, ILKN_N_INVERT_TX_MUBITS_POLARITYf, 0x0);
        soc_reg_field_set(unit, reg, reg_val, ILKN_N_INVERT_RECEIVED_FC_POLARITYf, 0x0);
        soc_reg_field_set(unit, reg, reg_val, ILKN_N_INVERT_TX_FC_POLARITYf, 0x0);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, (offset & 1), *reg_val));
        

        /* unset_enable_ilkn_port */
        reg = (offset < 2) ? ILKN_PMH_ENABLE_INTERLAKENr : ILKN_PML_ENABLE_INTERLAKENr; 
        field = (offset & 1) ? ENABLE_PORT_1f : ENABLE_PORT_0f;

        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, reg_val));
        soc_reg_field_set(unit, reg, reg_val, field, 0);
        soc_reg_field_set(unit, reg, reg_val, ENABLE_CORE_CLOCKf, 1);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, *reg_val));
    }

    if (nof_channels == 1 || IS_TDM_PORT(unit, port)/*last TDM channel*/) {
        /* All HRFs indications in NBIF for SCH */
        /* unset_ilkn_rx_hrf_en */
        switch (offset) {
        case 0:
            tdm_field  = RX_HRF_ENABLE_HRF_2f;
            data_field = RX_HRF_ENABLE_HRF_0f;
            break;
        case 1:
            tdm_field  = RX_HRF_ENABLE_HRF_3f;
            data_field = RX_HRF_ENABLE_HRF_1f;
            break;
        case 2:
            tdm_field  = RX_HRF_ENABLE_HRF_6f;
            data_field = RX_HRF_ENABLE_HRF_4f;
            break;
        case 3:
            tdm_field  = RX_HRF_ENABLE_HRF_7f;
            data_field = RX_HRF_ENABLE_HRF_5f;
            break;
        case 4:
            tdm_field  = RX_HRF_ENABLE_HRF_10f;
            data_field = RX_HRF_ENABLE_HRF_8f;
            break;
        case 5:
            tdm_field  = RX_HRF_ENABLE_HRF_11f;
            data_field = RX_HRF_ENABLE_HRF_9f;
            break;
        }
        SOCDNX_IF_ERR_EXIT(READ_NBIH_ENABLE_INTERLAKEN_HRFr(unit, reg_val)); 
        if (IS_TDM_PORT(unit, port)) {
            soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKEN_HRFr, reg_val, tdm_field, 0); 
        }
        if (nof_channels == 1) {
            soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKEN_HRFr, reg_val, data_field, 0); 
        }
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ENABLE_INTERLAKEN_HRFr(unit, *reg_val)); 
    

        /* unset_ilkn_tx_hrf_rstn - Make sure HRF is not enabled before it is configured */
        reg = (offset < 2) ? NBIH_HRF_RESETr : NBIL_HRF_RESETr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, reg_val)); 
        if (nof_channels == 1) {
            field = (offset & 1) ? HRF_TX_1_CONTROLLER_RSTNf : HRF_TX_0_CONTROLLER_RSTNf; 
            soc_reg_field_set(unit, reg, reg_val, field, 0);
            field = (offset & 1) ? HRF_RX_1_CONTROLLER_RSTNf : HRF_RX_0_CONTROLLER_RSTNf;
            soc_reg_field_set(unit, reg, reg_val, field, 0);
        }
        if (hdr_type == SOC_TMC_PORT_HEADER_TYPE_TDM) {
            field = (offset & 1) ? HRF_RX_3_CONTROLLER_RSTNf : HRF_RX_2_CONTROLLER_RSTNf;
            soc_reg_field_set(unit, reg, reg_val, field, 0); 
        }
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, *reg_val));

        reg = (offset < 2) ? NBIH_HRF_RX_CONFIG_HRFr : NBIL_HRF_RX_CONFIG_HRFr;
        /* HRF 0-1 -> data traffic, HRF 2-3 -> tdm traffic */
        if (IS_TDM_PORT(unit, port)) {
            base_index = 2; /* HRF 2-3 -> tdm traffic */
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, base_index + (offset & 1), &reg64_val)); 
            soc_reg64_field32_set(unit, NBIH_HRF_RX_CONFIG_HRFr, &reg64_val, HRF_RX_BURST_MERGE_EN_HRF_Nf, 0);
            soc_reg64_field32_set(unit, NBIH_HRF_RX_CONFIG_HRFr, &reg64_val, HRF_RX_BURST_MERGE_TH_HRF_Nf, 0x8); /*default val for now*/
            soc_reg64_field32_set(unit, NBIH_HRF_RX_CONFIG_HRFr, &reg64_val, HRF_RX_BURST_MERGE_FORCE_HRF_Nf, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, base_index + (offset & 1), reg64_val));
        }
        if (nof_channels == 1) {
            base_index = 0; /* HRF 0-1 -> data traffic */
            SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, base_index + (offset & 1), &reg64_val)); 
            soc_reg64_field32_set(unit, NBIH_HRF_RX_CONFIG_HRFr, &reg64_val, HRF_RX_BURST_MERGE_EN_HRF_Nf, 0);
            soc_reg64_field32_set(unit, NBIH_HRF_RX_CONFIG_HRFr, &reg64_val, HRF_RX_BURST_MERGE_TH_HRF_Nf, 0x8); /*default val for now*/
            soc_reg64_field32_set(unit, NBIH_HRF_RX_CONFIG_HRFr, &reg64_val, HRF_RX_BURST_MERGE_FORCE_HRF_Nf, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, base_index + (offset & 1), reg64_val));
        }
        /* when last TDM channel is removed, meybe we can free one QMLF's memory we needed for TDM  - so call this function with enable == 1*/
        SOCDNX_IF_ERR_EXIT(soc_jer_port_ilkn_hrf_config_close_path(unit, port, offset)); 

        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_nif_priority_ilkn_tdm_clear, (unit, offset)));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_nif_priority_ilkn_high_low_clear, (unit, offset)));
    }

    if (nof_channels == 1) { 

        /* 4 HRF's per core : two Data and 2 TDM. if TDM, can configure only pipe field. */
        index = 0;
        switch (offset) {
        case 0:
            reg = NBIH_RX_SCH_CONFIG_HRFr;
            index = offset;
            break;
        case 1:
            reg = NBIH_RX_SCH_CONFIG_HRFr;
            index = offset;
            break;
        case 2:
            reg = NBIH_RX_SCH_CONFIG_HRF_4r;
            break;
        case 3:
            reg = NBIH_RX_SCH_CONFIG_HRF_5r;
            break;
        case 4:
            reg = NBIH_RX_SCH_CONFIG_HRF_8r;
            break;
        case 5:
            reg = NBIH_RX_SCH_CONFIG_HRF_9r;
            break;
        }
        SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, reg, REG_PORT_ANY, index, &reg64_val)); 
        soc_reg64_field32_set(unit, reg, &reg64_val, HRF_RX_PIPE_Nf, 0);
        soc_reg64_field32_set(unit, reg, &reg64_val, HRF_RX_PRIORITY_HRF_Nf, 0);
        soc_reg64_field32_set(unit, reg, &reg64_val, HRF_RX_SCH_MAP_HRF_Nf, 0); 
        SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, reg, REG_PORT_ANY, index, reg64_val));

        /* Unconfig ILKN ports in NBI */
        reg = (offset < 2) ? NBIH_HRF_TX_CONFIG_HRFr : NBIL_HRF_TX_CONFIG_HRFr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, (offset & 1), reg_val)); 
        soc_reg_field_set(unit, reg, reg_val, HRF_TX_NUM_CREDITS_TO_EGQ_HRF_Nf, 0x200);                                          
        soc_reg_field_set(unit, reg, reg_val, HRF_TX_USE_EXTENDED_MEM_HRF_Nf, 0); 
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, (offset & 1), *reg_val));


        reg = (offset < 2) ? NBIH_ILKN_TX_RETRANSMIT_CONFIG_HRFr : NBIL_ILKN_TX_RETRANSMIT_CONFIG_HRFr;
        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, (offset & 1), reg_val)); 
        soc_reg_field_set(unit, reg, reg_val, ILKN_TX_RETRANS_ENABLE_HRF_Nf, 0);
        soc_reg_field_set(unit, reg, reg_val, ILKN_TX_RETRANS_NUM_ENTRIES_TO_SAVE_HRF_Nf, 0xff);
        soc_reg_field_set(unit, reg, reg_val, ILKN_TX_RETRANS_MULTIPLY_HRF_Nf, 0);
        soc_reg_field_set(unit, reg, reg_val, ILKN_TX_RETRANS_WAIT_FOR_SEQ_NUM_CHANGE_HRF_Nf, 0);
        soc_reg_field_set(unit, reg, reg_val, ILKN_TX_RETRANS_IGNORE_REQ_WHEN_ALMOST_EMPTY_HRF_Nf, 0);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, (offset & 1), *reg_val));

        /* enabling an ILKN port can sometimed require configurations for the other ILKN of the same core
        before removing an ILKN port, we need to find out if the other ILKN port is enabled */
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, ((offset & 1) ? offset - 1 : offset + 1), &second_ilkn_port));

        /* in Jericho, ILKN0/2/4 always get 4 segments, and then ILKN1/3/5 cannot work. 
         * in order to give ILKN0/2/4 only 2 segements, we enable ILKN1/3/5 even if it is not a real port.
         * in Jer_plus this logic is fixed and it is possible to assign only 2 segments to ILKN0/2/4, 
         * so we need to disable this logic only when we remove the last ILKN port of this ILKN core*/ 
        /* Disable ILKN in NBI - only for "big" ports */
        if ((offset & 1) == 0 || (second_ilkn_port == SOC_JER_INVALID_PORT)) {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_ENABLE_INTERLAKENr(unit, reg_val)); 
            soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, reg_val, enable_ilkn_fields[offset], 0);
            if ((offset < 2) && SOC_IS_JERICHO_PLUS_ONLY(unit) && (second_ilkn_port == SOC_JER_INVALID_PORT)) {
                 soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_RXf, 0); 
                 soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_TXf, 0); 
            }
            if (((offset & 1) == 0) && (nof_segments == 2) && (second_ilkn_port == SOC_JER_INVALID_PORT)) {
                soc_reg_field_set(unit, NBIH_ENABLE_INTERLAKENr, reg_val, enable_ilkn_fields[offset + 1], 0);
            }
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ENABLE_INTERLAKENr(unit, *reg_val));

            if(offset >= 2) {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_ENABLE_INTERLAKENr(unit, reg_port, reg_val)); 
                soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, reg_val, enable_ilkn_fields[offset & 1], 0);
                if (SOC_IS_JERICHO_PLUS_ONLY(unit) && (second_ilkn_port == SOC_JER_INVALID_PORT)) {
                     soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_RXf, 0); 
                     soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, reg_val, ILKN_PORT_0_USE_TWO_SEGMENTS_TXf, 0); 
                }
                if (((offset & 1) == 0) && (nof_segments == 2) && (second_ilkn_port == SOC_JER_INVALID_PORT)) {
                    soc_reg_field_set(unit, NBIL_ENABLE_INTERLAKENr, reg_val, ENABLE_PORT_1f, 0);
                }
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ENABLE_INTERLAKENr(unit, reg_port, *reg_val));
            }
        }

        /* Fabric mux - only in PML1 */
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_PML_ILKN_OVER_FABRICr(unit, 1, reg_val));
            il_over_fabric = soc_reg_field_get(unit, ILKN_PML_ILKN_OVER_FABRICr, *reg_val, ILKN_OVER_FABRICf);
            SOC_PBMP_ITER(phy_lanes, phy){
                il_over_fabric |= 1 << (((phy) % JER_NIF_ILKN_MAX_NOF_LANES) + (SOC_IS_QMX(unit) ? 8 : 0));
            }
            soc_reg_field_set(unit, ILKN_PML_ILKN_OVER_FABRICr, reg_val, ILKN_OVER_FABRICf, il_over_fabric);
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_PML_ILKN_OVER_FABRICr(unit, 1, *reg_val));

            SOC_PBMP_CLEAR(ilkn_over_fabric_ports);    
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_over_fabric_pbmp_get(unit, &ilkn_over_fabric_ports));
            
            SOCDNX_IF_ERR_EXIT(READ_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit,&reachability_allowed_bm));
            COMPILER_64_NOT(reachability_allowed_bm);
            SOC_PBMP_ITER(ilkn_over_fabric_ports, link){
            if ((link - SOC_DPP_DEFS_GET(unit, first_fabric_link_id) - FABRIC_LOGICAL_PORT_BASE(unit) ) <= SOC_JER_NIF_ILKN_OVER_FABRIC_MAX_LANE(unit)) {
                    COMPILER_64_BITSET(reachability_allowed_bm, (link - SOC_DPP_DEFS_GET(unit, first_fabric_link_id) - FABRIC_LOGICAL_PORT_BASE(unit) )); 
                } else {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Invalid ILKN over fabric lane"))); 
                }
            }
            COMPILER_64_NOT(reachability_allowed_bm);
            SOCDNX_IF_ERR_EXIT(WRITE_MESH_TOPOLOGY_REG_010Fr(unit, reachability_allowed_bm));
            SOCDNX_IF_ERR_EXIT(WRITE_RTP_ALLOWED_LINKS_FOR_REACHABILITY_MESSAGESr_REG64(unit, reachability_allowed_bm));

            for (i = 0; i < nof_fabric_ilkn_pms * NUM_OF_LANES_IN_PM; ++i) {
                fabric_port = FABRIC_LOGICAL_PORT_BASE(unit) + SOC_DPP_DEFS_GET(unit, first_fabric_link_id) + i;
                if (SOC_PBMP_MEMBER(phy_lanes, fabric_port)) {
                    SOC_PBMP_PORT_ADD(SOC_PORT_BITMAP(unit, sfi), fabric_port); 
                    SOC_PBMP_PORT_ADD(SOC_PORT_BITMAP(unit, port), fabric_port);
                    SOC_PBMP_PORT_ADD(SOC_PORT_BITMAP(unit, all), fabric_port);
                }
            }

            SOC_DPP_CONFIG(unit)->jer->nif.ilkn_over_fabric[offset & 1] = 0;
        }

        /* Zero the segments for the removed port */
        SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset] = 0;

        /* Shut down relevant wrapper */
        reg = (offset < 2 ) ? ILKN_PMH_ILKN_RESETr : ILKN_PML_ILKN_RESETr;

        SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, 0, reg_val));
        field = (offset & 1) ? ILKN_1_PORT_RSTNf : ILKN_0_PORT_RSTNf;
        soc_reg_field_set(unit, reg, reg_val, field, 0);
        field = (offset & 1) ? ILKN_RX_1_PORT_RSTNf : ILKN_RX_0_PORT_RSTNf;
        soc_reg_field_set(unit, reg, reg_val, field, 0);
        field = (offset & 1) ? ILKN_TX_1_PORT_RSTNf : ILKN_TX_0_PORT_RSTNf;
        soc_reg_field_set(unit, reg, reg_val, field, 0);
        SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, 0, *reg_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


int 
soc_jer_port_close_nbi_path(int unit, soc_port_t port) {

    const soc_field_t fields[4] = {TX_STOP_DATA_TO_PORT_MACRO_MLF_0_QMLF_Nf, TX_STOP_DATA_TO_PORT_MACRO_MLF_1_QMLF_Nf, TX_STOP_DATA_TO_PORT_MACRO_MLF_2_QMLF_Nf, TX_STOP_DATA_TO_PORT_MACRO_MLF_3_QMLF_Nf};
    int pm_index = 0, reg_port, qmlf_index, core, max_ports_nbih, max_ports_nbil, nof_lanes_nbi;
    uint32 first_lane; /*1-72*/
    uint64 reg64_val;
    uint32 flags, first_phy, reg_val, mode;
    soc_reg_t reg;
    soc_pbmp_t phys, lanes;
    soc_port_t lane_i;
    soc_port_if_t interface_type;
    SOCDNX_INIT_FUNC_DEFS;

    max_ports_nbih = SOC_DPP_DEFS_GET(unit, nof_ports_nbih);
    max_ports_nbil = SOC_DPP_DEFS_GET(unit, nof_ports_nbil);
    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phys, &lanes));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy, &first_lane)));
    --first_phy; /* first_phy returned is one-based */
    --first_lane; /* first_lane returned is one-based */

    /* 
     * When ETH port is disabled, the fifo is flushed by constantly granting EGQ credits for the corresponding port. 
     * before the port is removed, the flush idecation should be cleared in order to allow dynamicly adding another port.
     */
    SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_tx_egress_credits_set(unit, port, SOC_JER_TX_EGRESS_CREDITS_CMD_FLUSH, 0));

    SOCDNX_IF_ERR_EXIT(soc_jer_port_nbi_ports_rstn(unit, port, 0));

    reg_port = (first_phy < max_ports_nbih) ? REG_PORT_ANY : (first_phy / (max_ports_nbih + max_ports_nbil));
    pm_index = (first_lane % nof_lanes_nbi) / NUM_OF_LANES_IN_PM; /*internal PM index inside NBIH/L (0-5)*/
    qmlf_index = first_lane / NUM_OF_LANES_IN_PM; /*qmlf index in entire NIF (0-17)*/

    /* take MLF out of reset - if port is ELK, SIF or ILKN- no need to take MLF ooo */
    if (!SOC_PORT_IS_ELK_INTERFACE(flags) && !SOC_PORT_IS_STAT_INTERFACE(flags)) {
        if (SOC_IS_QUX(unit)) {
            reg = NIF_TX_QMLF_CONFIGr;
            reg_port = REG_PORT_ANY;
        } else {
            reg = (first_phy < max_ports_nbih) ? NBIH_TX_QMLF_CONFIGr : NBIL_TX_QMLF_CONFIGr;
        }

        SOCDNX_IF_ERR_EXIT(soc_reg_get(unit, reg, reg_port, pm_index, &reg64_val));
        SOC_PBMP_ITER(lanes, lane_i) {
            soc_reg64_field32_set(unit, reg, &reg64_val, fields[(lane_i - 1) % 4], 1);
        }
        SOCDNX_IF_ERR_EXIT(soc_reg_set(unit, reg, reg_port, pm_index, reg64_val));
    }

    /* Port Mode */
    mode = 2; /* Four Ports - default */
    if (!SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_QMLF_CONFIGr(unit, qmlf_index, &reg_val));
        soc_reg_field_set(unit, NBIH_RX_QMLF_CONFIGr, &reg_val, RX_PORT_MODE_QMLF_Nf, mode);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_RX_QMLF_CONFIGr(unit, qmlf_index, reg_val));

        if(first_phy >= max_ports_nbih) {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_QMLF_CONFIGr(unit, reg_port, pm_index, &reg_val));
            soc_reg_field_set(unit, NBIL_RX_QMLF_CONFIGr, &reg_val, RX_PORT_MODE_QMLF_Nf, mode);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_QMLF_CONFIGr(unit, reg_port, pm_index, reg_val));
        }
        reg = (first_phy < max_ports_nbih) ? NBIH_TX_QMLF_CONFIGr : NBIL_TX_QMLF_CONFIGr;
    } else {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_QMLF_CONFIGr(unit, pm_index, &reg_val));
        soc_reg_field_set(unit, NIF_RX_QMLF_CONFIGr, &reg_val, RX_PORT_MODE_QMLF_Nf, mode);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_RX_QMLF_CONFIGr(unit, pm_index, reg_val));
        reg = NIF_TX_QMLF_CONFIGr;
        reg_port = REG_PORT_ANY;
    }

    SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, reg_port, pm_index, &reg64_val));
    soc_reg64_field32_set(unit, reg, &reg64_val, TX_PORT_MODE_QMLF_Nf, mode);
    SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, reg_port, pm_index, reg64_val));

exit:
    SOCDNX_FUNC_RETURN;
}



int
soc_jer_nif_sif_set(int unit, uint32 first_phy)
{
    int port_num, pm_select = 0, nbil_index;
    uint32 reg_val;
    SOCDNX_INIT_FUNC_DEFS;
    
    if (SOC_IS_QUX(unit)) {   /* QUX don't call it */
        SOCDNX_FUNC_RETURN;
    }

    if (first_phy == JER_NIF_PHY_SIF_PORT_0_NBIH_PM5 || first_phy == JER_NIF_PHY_SIF_PORT_1_NBIH_PM5 || first_phy == JER_NIF_PHY_SIF_PORT_1_NBIH_PM4)
    {
        /* PHY_SIF_PORT_1_NBIH_PM4 is allowed only in JERICHO+ */
        if(!SOC_IS_JERICHO_PLUS_ONLY(unit) && (first_phy == JER_NIF_PHY_SIF_PORT_1_NBIH_PM4))
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Invalid phy port (%d) configured as statistics interface"), first_phy));
        }        
        port_num = (first_phy == JER_NIF_PHY_SIF_PORT_0_NBIH_PM5) ? 0:1;
        SOCDNX_IF_ERR_EXIT(READ_NBIH_SIF_CFGr(unit, port_num, &reg_val));
        soc_reg_field_set(unit, NBIH_SIF_CFGr, &reg_val, SIF_PORT_N_ENf, 1);
        if(SOC_IS_JERICHO_PLUS_ONLY(unit))
        {
            soc_reg_field_set(unit, NBIH_SIF_CFGr, &reg_val, SIF_PORT_N_PM_SELf, ((first_phy == JER_NIF_PHY_SIF_PORT_1_NBIH_PM4) ? 1 : 0));
        }
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_SIF_CFGr(unit, port_num, reg_val));

        SOCDNX_IF_ERR_EXIT(READ_NBIH_ADDITIONAL_RESETSr(unit, &reg_val));
        soc_reg_field_set(unit, NBIH_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_ADDITIONAL_RESETSr(unit, reg_val));
    }
    else
    {
        if (first_phy == JER_NIF_PHY_SIF_PORT_0_NBIL0 || first_phy == JER_NIF_PHY_SIF_PORT_1_NIBL0_PM4 || first_phy == JER_NIF_PHY_SIF_PORT_1_NBIL0_PM5)
        {
            nbil_index = 0;
            port_num = (first_phy == JER_NIF_PHY_SIF_PORT_0_NBIL0) ? 0:1;
            if (port_num == 1 && first_phy == JER_NIF_PHY_SIF_PORT_1_NIBL0_PM4)
            {
                pm_select= 1;
            }
        }
        else if (first_phy == JER_NIF_PHY_SIF_PORT_0_NBIL1 || first_phy == JER_NIF_PHY_SIF_PORT_1_NBIL1_PM4 || first_phy == JER_NIF_PHY_SIF_PORT_1_NBIL1_PM5) 
        {
            /* from J+ spec: -"There is no SIF (statistics port) in nif_pml1 (no need as there is SIF on NBIH and SIF on NBIL0)" */
            if(SOC_IS_JERICHO_PLUS_ONLY(unit))
            {
                SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Invalid phy port (%d) configured as statistics interface"), first_phy));
            }
            else
            {
                nbil_index = 1;
                port_num = (first_phy == JER_NIF_PHY_SIF_PORT_0_NBIL1) ? 0:1;
                if (port_num == 1 && first_phy == JER_NIF_PHY_SIF_PORT_1_NBIL1_PM4)
                {
                    pm_select = 1;
                }            
            }
        }
        else
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Invalid phy port (%d) configured as statistics interface"), first_phy));
        }

        SOCDNX_IF_ERR_EXIT(READ_NBIL_SIF_CFGr(unit, nbil_index, port_num, &reg_val));
        soc_reg_field_set(unit, NBIL_SIF_CFGr, &reg_val, SIF_PORT_N_ENf, 1);
        soc_reg_field_set(unit, NBIL_SIF_CFGr, &reg_val, SIF_PORT_N_PM_SELf, pm_select);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_SIF_CFGr(unit, nbil_index, port_num, reg_val));

        SOCDNX_IF_ERR_EXIT(READ_NBIL_ADDITIONAL_RESETSr(unit, nbil_index, &reg_val));
        soc_reg_field_set(unit, NBIL_ADDITIONAL_RESETSr, &reg_val, SIF_RSTNf, 1);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_ADDITIONAL_RESETSr(unit, nbil_index, reg_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_sch_config(int unit, soc_port_t port)
{
    soc_pbmp_t quad_bmp;
    int qmlf_index, core, priority_level;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
    SOCDNX_IF_ERR_EXIT(soc_jer_portmod_port_quad_get(unit, port, &quad_bmp));

    /* ReqEn register */
    priority_level = ( IS_TDM_PORT(unit, port)) ? JER_NIF_PRIO_TDM_LEVEL : JER_NIF_PRIO_HIGH_LEVEL;
    SOC_PBMP_ITER(quad_bmp, qmlf_index) {
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_priority_set(unit, core, qmlf_index, 0, 0, 1, priority_level));
    }

exit:
    SOCDNX_FUNC_RETURN;   
}

STATIC int 
soc_jer_port_nbi_ports_rstn(int unit, soc_port_t port, int enable){

    int index = 0, nof_lanes_nbi, max_ports_nbil, max_ports_nbih, nbi_inst;
    uint64 reg64_val, nbil_ports_rstn, nbi_ports_rstn;
    uint32 first_phy, first_lane, reg_val;
    soc_pbmp_t phys;
    uint32 nbih_ports_rstn[1] = {0};
    SOCDNX_INIT_FUNC_DEFS;

    max_ports_nbih = SOC_DPP_DEFS_GET(unit, nof_ports_nbih);
    max_ports_nbil = SOC_DPP_DEFS_GET(unit, nof_ports_nbil);
    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phys));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy, &first_lane)));
    --first_phy; /* first_phy returned is one-based */
    --first_lane; /* first_lane returned is one-based */
    nbi_inst = first_lane / nof_lanes_nbi; /* NBIH = 0, NBIL0 = 1, NBIL1 = 2*/
    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_PORTS_SRSTNr(unit, &reg64_val));
        nbi_ports_rstn = soc_reg64_field_get(unit, NIF_RX_PORTS_SRSTNr, reg64_val, RX_PORTS_SRSTNf);

        COMPILER_64_BITCLR(nbi_ports_rstn, first_phy);
        soc_reg64_field_set(unit, NIF_RX_PORTS_SRSTNr, &reg64_val, RX_PORTS_SRSTNf, nbi_ports_rstn);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_RX_PORTS_SRSTNr(unit, reg64_val));

        if (enable) {
            COMPILER_64_BITSET(nbi_ports_rstn, first_phy); 
            soc_reg64_field_set(unit, NIF_RX_PORTS_SRSTNr, &reg64_val, RX_PORTS_SRSTNf, nbi_ports_rstn);
            SOCDNX_IF_ERR_EXIT(WRITE_NIF_RX_PORTS_SRSTNr(unit, reg64_val));
        }

        SOCDNX_IF_ERR_EXIT(READ_NIF_TX_PORTS_SRSTNr(unit, &reg64_val));
        nbi_ports_rstn = soc_reg64_field_get(unit, NIF_TX_PORTS_SRSTNr, reg64_val, TX_PORTS_SRSTNf);

        COMPILER_64_BITCLR(nbi_ports_rstn, first_phy);
        soc_reg64_field_set(unit, NIF_TX_PORTS_SRSTNr, &reg64_val, TX_PORTS_SRSTNf, nbi_ports_rstn);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_TX_PORTS_SRSTNr(unit, reg64_val));

        if (enable) {
            COMPILER_64_BITSET(nbi_ports_rstn, first_phy); 
            soc_reg64_field_set(unit, NIF_TX_PORTS_SRSTNr, &reg64_val, TX_PORTS_SRSTNf, nbi_ports_rstn);
            SOCDNX_IF_ERR_EXIT(WRITE_NIF_TX_PORTS_SRSTNr(unit, reg64_val));
        }
    } else {
        if (first_phy < max_ports_nbih) {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_PORTS_SRSTNr(unit, &reg_val));
            *nbih_ports_rstn = soc_reg_field_get(unit, NBIH_RX_PORTS_SRSTNr, reg_val, RX_PORTS_SRSTNf);

            SHR_BITCLR(nbih_ports_rstn, first_phy);
            soc_reg_field_set(unit, NBIH_RX_PORTS_SRSTNr, &reg_val, RX_PORTS_SRSTNf, *nbih_ports_rstn);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_RX_PORTS_SRSTNr(unit, reg_val));

            if (enable) {
                SHR_BITSET(nbih_ports_rstn, first_phy); 
                soc_reg_field_set(unit, NBIH_RX_PORTS_SRSTNr, &reg_val, RX_PORTS_SRSTNf, *nbih_ports_rstn);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_RX_PORTS_SRSTNr(unit, reg_val));
            }

            SOCDNX_IF_ERR_EXIT(READ_NBIH_TX_PORTS_SRSTNr(unit, &reg_val));
            *nbih_ports_rstn = soc_reg_field_get(unit, NBIH_TX_PORTS_SRSTNr, reg_val, TX_PORTS_SRSTNf);
            SHR_BITCLR(nbih_ports_rstn, first_phy);
            soc_reg_field_set(unit, NBIH_TX_PORTS_SRSTNr, &reg_val, TX_PORTS_SRSTNf, *nbih_ports_rstn);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_TX_PORTS_SRSTNr(unit, reg_val));

            if (enable) {
                SHR_BITSET(nbih_ports_rstn, first_phy);
                soc_reg_field_set(unit, NBIH_TX_PORTS_SRSTNr, &reg_val, TX_PORTS_SRSTNf, *nbih_ports_rstn);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_TX_PORTS_SRSTNr(unit, reg_val));
            }
        } else {
            index = nbi_inst - 1;

            SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_PORTS_SRSTNr(unit, index, &reg64_val));
            nbil_ports_rstn = soc_reg64_field_get(unit, NBIL_RX_PORTS_SRSTNr, reg64_val, RX_PORTS_SRSTNf);

            COMPILER_64_BITCLR(nbil_ports_rstn, first_phy - max_ports_nbih - (index * max_ports_nbil));
            soc_reg64_field_set(unit, NBIL_RX_PORTS_SRSTNr, &reg64_val, RX_PORTS_SRSTNf, nbil_ports_rstn);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_PORTS_SRSTNr(unit, index, reg64_val));

            if (enable) {
                COMPILER_64_BITSET(nbil_ports_rstn, first_phy - max_ports_nbih - (index * max_ports_nbil)); 
                soc_reg64_field_set(unit, NBIL_RX_PORTS_SRSTNr, &reg64_val, RX_PORTS_SRSTNf, nbil_ports_rstn);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_PORTS_SRSTNr(unit, index, reg64_val));
            }

            SOCDNX_IF_ERR_EXIT(READ_NBIL_TX_PORTS_SRSTNr(unit, index, &reg64_val));
            nbil_ports_rstn = soc_reg64_field_get(unit, NBIL_TX_PORTS_SRSTNr, reg64_val, TX_PORTS_SRSTNf);
            COMPILER_64_BITCLR(nbil_ports_rstn, first_phy - max_ports_nbih - (index * max_ports_nbil));
            soc_reg64_field_set(unit, NBIL_TX_PORTS_SRSTNr, &reg64_val, TX_PORTS_SRSTNf, nbil_ports_rstn);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_TX_PORTS_SRSTNr(unit, index, reg64_val));

            if (enable) {
                COMPILER_64_BITSET(nbil_ports_rstn, first_phy - max_ports_nbih - (index * max_ports_nbil)); 
                soc_reg64_field_set(unit, NBIL_TX_PORTS_SRSTNr, &reg64_val, TX_PORTS_SRSTNf, nbil_ports_rstn);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_TX_PORTS_SRSTNr(unit, index, reg64_val));
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_open_path(int unit, soc_port_t port) {

    soc_port_if_t interface_type;
    int core;
    uint32 first_phy, flags, is_master;
    SOCDNX_INIT_FUNC_DEFS;
    
    SOCDNX_IF_ERR_RETURN(soc_port_sw_db_is_master_get(unit, port, &is_master));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core)); 
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
    --first_phy;

    if (SOC_PORT_IF_ILKN == interface_type) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_open_ilkn_path, (unit, port)));
    } else {
        if (is_master) {
            SOCDNX_IF_ERR_EXIT(soc_jer_port_open_nbi_path(unit, port));

            /* statistics interface */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
            if (SOC_PORT_IS_STAT_INTERFACE(flags)) {
                SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_nif_sif_set, (unit, first_phy)));            
            }
            /* configure NBI scheduler */
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_sch_config, (unit, port)));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;   
}


int
soc_jer_port_close_path(int unit, soc_port_t port) {

    uint32 nof_channels;
    int qmlf_index;
    soc_pbmp_t quad_ports;
    uint32 first_phy, phy_lane, flags, nof_ports_on_quad = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_of_channels_get(unit, port, &nof_channels));

    if (IS_IL_PORT(unit, port)) {
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_port_close_ilkn_path, (unit, port)));
    } else if (nof_channels == 1) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy, &phy_lane)));
        --phy_lane; /* phy_lane returned is one-based */
        --first_phy; /* first_phy returned is one-based */

        /* clear NBI scheduler */
        SOCDNX_IF_ERR_EXIT(soc_jer_port_ports_to_same_quad_get(unit, port, &quad_ports));
        SOC_PBMP_COUNT(quad_ports, nof_ports_on_quad);

        if(nof_ports_on_quad == 1)
        {
            qmlf_index = phy_lane / NUM_OF_LANES_IN_PM;
            if(SOC_IS_QAX(unit)) {
                SOCDNX_IF_ERR_EXIT(soc_qax_nif_priority_quad_tdm_high_low_clear(unit, qmlf_index, 1, 1));
            } else {
                SOCDNX_IF_ERR_EXIT(soc_jer_nif_priority_quad_tdm_high_low_clear(unit, qmlf_index, 1, 1));
            }
        }

        /* statistics interface */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
        if (SOC_PORT_IS_STAT_INTERFACE(flags)) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_nif_sif_set, (unit, first_phy)));
        }

        SOCDNX_IF_ERR_EXIT(soc_jer_port_close_nbi_path(unit, port));
    }

    if (nof_channels == 1) {
        /* clear FQP TXI ready bits in EGQ */
        SOCDNX_IF_ERR_EXIT(soc_jer_port_eqg_nif_credit_init(unit, port));
    }

exit:
    SOCDNX_FUNC_RETURN;
}



int 
soc_jer_port_pll_type_get(int unit, soc_port_t port, SOC_JER_NIF_PLL_TYPE *pll_type)
{
    uint32 first_phy_port;
    uint32 pll_type_pmh_last_phy_lane = SOC_DPP_DEFS_GET(unit, pll_type_pmh_last_phy_lane);
    uint32 pll_type_pml0_last_phy_lane = SOC_DPP_DEFS_GET(unit, pll_type_pml0_last_phy_lane);
    uint32 pll_type_pml1_last_phy_lane = SOC_DPP_DEFS_GET(unit, pll_type_pml1_last_phy_lane);

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy_port /*one based*/));

    first_phy_port--; /* zero based */

    if (first_phy_port <= pll_type_pmh_last_phy_lane) {
        *pll_type = SOC_JER_NIF_PLL_TYPE_PMH;
    } else if (first_phy_port > pll_type_pmh_last_phy_lane && first_phy_port <= pll_type_pml0_last_phy_lane) {
        *pll_type = SOC_JER_NIF_PLL_TYPE_PML0;
    } else if (first_phy_port > pll_type_pml0_last_phy_lane && first_phy_port <= pll_type_pml1_last_phy_lane) {
        *pll_type = SOC_JER_NIF_PLL_TYPE_PML1;
    } else if ((first_phy_port > SOC_JER_NIF_PLL_TYPE_FABRIC0_FIRST_PHY_LANE) &&
               (first_phy_port <= SOC_JER_NIF_PLL_TYPE_FABRIC0_LAST_PHY_LANE)) {
        *pll_type = SOC_JER_NIF_PLL_TYPE_FABRIC0;
    } else {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Invalid phy port %d"), first_phy_port));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_duplex_set(int unit, soc_port_t port, int duplex)
{
    phymod_autoneg_control_t an;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_autoneg_get(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));
    an.enable = FALSE;
    SOCDNX_IF_ERR_EXIT(portmod_port_autoneg_set(unit, port, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY, &an));

    SOCDNX_IF_ERR_EXIT(portmod_port_duplex_set(unit, port, duplex));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_fault_get(int unit, soc_port_t port, uint32 *flags)
{
    int local_fault;
    int remote_fault;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_local_fault_status_get(unit, port, &local_fault));
    SOCDNX_IF_ERR_EXIT(portmod_port_remote_fault_status_get(unit, port, &remote_fault));

    *flags = 0;
    if (remote_fault) {
        *flags |= BCM_PORT_FAULT_REMOTE;
    }
    if (local_fault) {
        *flags |= BCM_PORT_FAULT_LOCAL;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC soc_error_t
_soc_jer_port_pm_get(int unit, uint32 phy, uint32 *pm_idx)
{
    uint32 new_phy;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy, &new_phy)));
    *pm_idx = new_phy / NUM_OF_LANES_IN_PM;

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC int
_soc_jer_port_eee_pm_nof_ports_enabled(int unit, soc_port_t port, int *nof_ports_enabled)
{
    portmod_eee_t eee;
    soc_pbmp_t ports_bm;
    soc_port_t port_idx;
    uint32 first_lane, first_phy_port, last_phy_port, phy_port;
    uint32 enabled = 0;
    SOCDNX_INIT_FUNC_DEFS;

    /* Get first phy port in pm */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy_port /*one based*/));
    --first_phy_port; /*zero based*/

    /* Get last phy port in pm */
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, first_phy_port, &first_lane)));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_add, (unit, first_lane + NUM_OF_LANES_IN_PM - 1, &last_phy_port)));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, SOC_PORT_FLAGS_NETWORK_INTERFACE, &ports_bm));
    SOC_PBMP_ITER(ports_bm, port_idx) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port_idx, &phy_port /*one based*/));
        --phy_port; /*zero based*/
        if (phy_port >= first_phy_port && phy_port <= last_phy_port) {
            SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port_idx, &eee));
            if (eee.enable) {
                ++enabled;
            }
        }
    }

    *nof_ports_enabled = enabled;

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_eee_enable_set(int unit, soc_port_t port, uint32 value)
{
    portmod_eee_t eee;
    SOC_JER_NIF_PLL_TYPE pll_type;
    uint32 first_phy_port, is_enabled, pm_idx, pml_idx, reg_port, reg_idx;
    uint32 rval = 0;
    int nof_ports_enabled;
    uint32 nof_pms_per_nbi = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &first_phy_port /*one based*/));
    --first_phy_port; /*zero based*/

    if (!SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_jer_port_pll_type_get(unit, port, &pll_type));
    }

    SOCDNX_IF_ERR_EXIT(_soc_jer_port_pm_get(unit, first_phy_port, &pm_idx));

    /* Get nof ports that are eee enabled in pm */
    SOCDNX_IF_ERR_EXIT(_soc_jer_port_eee_pm_nof_ports_enabled(unit, port, &nof_ports_enabled));
    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    is_enabled = eee.enable;

    /* Enable NBI EEE */
    if (SOC_IS_QUX(unit)) {
        if (value && (0 == nof_ports_enabled)) {
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, NIF_NIF_PM_CFGr, REG_PORT_ANY, pm_idx, &rval));
            soc_reg_field_set(unit, NIF_NIF_PM_CFGr, &rval, PM_NEEE_PD_ENf, 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, NIF_NIF_PM_CFGr, REG_PORT_ANY, pm_idx, rval));
        } else if ((0 == value) && (1 == nof_ports_enabled) && is_enabled) {
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, NIF_NIF_PM_CFGr, REG_PORT_ANY, pm_idx, &rval));
            soc_reg_field_set(unit, NIF_NIF_PM_CFGr, &rval, PM_NEEE_PD_ENf, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, NIF_NIF_PM_CFGr, REG_PORT_ANY, pm_idx, rval));
        } else {
            /* do nothing */
        }
    } else if (pll_type == SOC_JER_NIF_PLL_TYPE_PMH) {
        soc_reg_t nbih_nif_pm_cfg = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIH_REG_0C06r : NBIH_NIF_PM_CFGr;
        soc_field_t pmh_neee_pd_en = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? FIELD_9_9f : PMH_NEEE_PD_ENf;

        if (value && (0 == nof_ports_enabled)) {
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, nbih_nif_pm_cfg, REG_PORT_ANY, pm_idx, &rval));
            soc_reg_field_set(unit, nbih_nif_pm_cfg, &rval, pmh_neee_pd_en, 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, nbih_nif_pm_cfg, REG_PORT_ANY, pm_idx, rval));
        } else if ((0 == value) && (1 == nof_ports_enabled) && is_enabled) {
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, nbih_nif_pm_cfg, REG_PORT_ANY, pm_idx, &rval));
            soc_reg_field_set(unit, nbih_nif_pm_cfg, &rval, pmh_neee_pd_en, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, nbih_nif_pm_cfg, REG_PORT_ANY, pm_idx, rval));
        } else {
            /* do nothing */
        }
    } else {
        soc_reg_t nbil_nif_pm_config;
        soc_field_t pml_neee_pd_en = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? FIELD_14_14f : PML_NEEE_PD_ENf;
        reg_port = (pm_idx / nof_pms_per_nbi) - 1;
        pml_idx = pm_idx % nof_pms_per_nbi;
        switch (pml_idx) {
            case 3:
                nbil_nif_pm_config = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_3r : NBIL_NIF_PM_CFG_3r;
                reg_idx = 0;
                break;
            case 4:
                nbil_nif_pm_config = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_4r : NBIL_NIF_PM_CFG_4r;
                reg_idx = 0;
                break;
            case 5:
                nbil_nif_pm_config = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A09_5r : NBIL_NIF_PM_CFG_5r;
                reg_idx = 0;
                break;
            default:
                nbil_nif_pm_config = (SOC_IS_QMX_B0(unit) || SOC_IS_JERICHO_B0(unit)) ? NBIL_REG_0A06r : NBIL_NIF_PM_CFGr;
                reg_idx = pml_idx;
                break;
        }
        if (value && (0 == nof_ports_enabled)) {
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, nbil_nif_pm_config, reg_port, reg_idx, &rval));
            soc_reg_field_set(unit, nbil_nif_pm_config, &rval, pml_neee_pd_en, 1);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, nbil_nif_pm_config, reg_port, reg_idx, rval));
        } else if ((0 == value) && (1 == nof_ports_enabled) && is_enabled) {
            SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, nbil_nif_pm_config, reg_port, reg_idx, &rval));
            soc_reg_field_set(unit, nbil_nif_pm_config, &rval, pml_neee_pd_en, 0);
            SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, nbil_nif_pm_config, reg_port, reg_idx, rval));
        } else {
            /* do nothing */
        }
    }

    /* Enable port EEE */
    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    eee.enable = value;
    SOCDNX_IF_ERR_EXIT(portmod_port_eee_set(unit, port, &eee));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_eee_enable_get(int unit, soc_port_t port, uint32 *value)
{
    portmod_eee_t eee;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    *value = eee.enable;

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_eee_tx_idle_time_set(int unit, soc_port_t port, uint32 value)
{
    portmod_eee_t eee;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    eee.tx_idle_time = value;
    SOCDNX_IF_ERR_EXIT(portmod_port_eee_set(unit, port, &eee));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_eee_tx_idle_time_get(int unit, soc_port_t port, uint32 *value)
{
    portmod_eee_t eee;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    *value = eee.tx_idle_time;

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_eee_tx_wake_time_set(int unit, soc_port_t port, uint32 value)
{
    portmod_eee_t eee;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    eee.tx_wake_time = value;
    SOCDNX_IF_ERR_EXIT(portmod_port_eee_set(unit, port, &eee));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_eee_tx_wake_time_get(int unit, soc_port_t port, uint32 *value)
{
    portmod_eee_t eee;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_eee_get(unit, port, &eee));
    *value = eee.tx_wake_time;

exit:
    SOCDNX_FUNC_RETURN;
}


int
soc_jer_port_nif_nof_lanes_get(int unit, soc_port_if_t interface, uint32 first_phy_port, uint32 nof_lanes_to_set, uint32 *nof_lanes)
{
    soc_port_t port_i;

    SOCDNX_INIT_FUNC_DEFS;

    *nof_lanes = 0;

    switch(interface) {
        case SOC_PORT_IF_XFI:
        case SOC_PORT_IF_SGMII:
        case SOC_PORT_IF_QSGMII:
            *nof_lanes = 1;
            break;

        case SOC_PORT_IF_CPU:
            *nof_lanes = 1;
            if (first_phy_port != 0) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("interface not supported on port")));
            }
            break;

        case SOC_PORT_IF_RXAUI:
        case SOC_PORT_IF_XLAUI2:
            *nof_lanes = 2;
            break;

        case SOC_PORT_IF_DNX_XAUI:
        case SOC_PORT_IF_XLAUI:
        case SOC_PORT_IF_CAUI:
            *nof_lanes = 4;
            break;

        case SOC_PORT_IF_ILKN:
            /* if this is the first ILKN port, then num_lanes should be as provided, otherwise get the current num_lanes */
            SOCDNX_IF_ERR_EXIT(soc_port_sw_db_port_from_interface_type_get(unit, interface, first_phy_port, &port_i));
            if (port_i != SOC_MAX_NUM_PORTS) {
                SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port_i, nof_lanes));                   
            } else {
                *nof_lanes = nof_lanes_to_set;
            }
            break;

        case SOC_PORT_IF_RCY:
        case SOC_PORT_IF_ERP:
        case SOC_PORT_IF_OLP:
        case SOC_PORT_IF_OAMP:
        case SOC_PORT_IF_SAT:
        case SOC_PORT_IF_IPSEC:
            *nof_lanes = 0;
            break;

        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("Interface %d isn't supported"), interface));
            break;
    }


    /* validity check */
    if ((interface != SOC_PORT_IF_ILKN)  &&
        (interface != SOC_PORT_IF_CPU)   &&
        (interface != SOC_PORT_IF_RCY)   &&
        (interface != SOC_PORT_IF_ERP)   &&
        (interface != SOC_PORT_IF_OLP)   &&
        (interface != SOC_PORT_IF_OAMP)  &&
        (interface != SOC_PORT_IF_IPSEC) &&
        (interface != SOC_PORT_IF_SAT)) {
        if ((first_phy_port - 1) % (*nof_lanes) != 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PORT, (_BSL_SOCDNX_MSG("interface not supported on port")));
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

uint32 jer_port_link_up_mac_update(int unit, soc_port_t port, int link)
{
    portmod_port_update_control_t portmod_port_update_control;
    portmod_port_interface_config_t if_config;
    int serdes_speed;
    phymod_autoneg_status_t an_status;
    int is_legacy_phy = 0, skip_spd_sync = 0;
    int rv;

    SOCDNX_INIT_FUNC_DEFS;
    if (link && !IS_SFI_PORT(unit, port) && !IS_IL_PORT(unit, port)) {
        /* PHY link up event */
        rv = (portmod_port_phy_link_up_event(unit, port));
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            LOG_WARN(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "u=%d p=%d portmod_port_phy_link_up_event rv=%d\n"),unit, port, rv));
            SOCDNX_IF_ERR_EXIT(rv);
        }

        rv = portmod_port_autoneg_status_get(unit, port, &an_status);
        if (BCM_FAILURE(rv) && (BCM_E_UNAVAIL != rv)) {
            LOG_WARN(BSL_LS_BCM_PORT,
                    (BSL_META_U(unit,
                                "u=%d p=%d portmod_port_autoneg_status_get rv=%d\n"),unit, port, rv));
            SOCDNX_IF_ERR_EXIT(rv);
        }

        if ((BCM_E_NONE == rv) && (an_status.enabled) && (an_status.locked)) {
            SOCDNX_IF_ERR_EXIT
                (portmod_port_interface_config_get(unit, port, &if_config, PORTMOD_INIT_F_EXTERNAL_MOST_ONLY));
            /* 1.If AN is enabled, get both "line side speed" and
                        "SERDES speed(multiply number of lanes", then compare them.
                        Otherwise, set LINKUP flag and continue.
             * 2. For PHY devices that has line and system side operate at different speeds
             * speed sync between line and system side is not required.
             */
            SOCDNX_IF_ERR_EXIT(portmod_port_speed_get(unit, port, &serdes_speed));
            SOCDNX_IF_ERR_EXIT(portmod_port_is_legacy_ext_phy_present(unit, port, &is_legacy_phy));
            if (is_legacy_phy) {
                skip_spd_sync = portmod_port_legacy_is_skip_spd_sync(unit, port);
            }
            if ((if_config.speed != serdes_speed) && (!skip_spd_sync)) {
                /* update serdes speed according to ext phy */

                switch (if_config.speed) {
                    case 100000:
                        if_config.serdes_interface = SOC_PORT_IF_CAUI;
                        break;
                    case 40000:
                        if_config.serdes_interface = SOC_PORT_IF_XLAUI;
                        break;
                    case 10000:
                    case 5000:
                    case 2500:
                        if_config.serdes_interface = SOC_PORT_IF_XGMII;
                        SOCDNX_IF_ERR_EXIT
                            (portmod_port_interface_config_set(unit, port, &if_config, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY));
                        break;
                    case 1000:
                    case 100:
                        if_config.serdes_interface = SOC_PORT_IF_SGMII;
                        SOCDNX_IF_ERR_EXIT
                            (portmod_port_interface_config_set(unit, port, &if_config, PORTMOD_INIT_F_INTERNAL_SERDES_ONLY));
                        break;
                    default:
                        break;
                }
            }
        }
    }

    SOCDNX_IF_ERR_EXIT(portmod_port_update_control_t_init(unit, &portmod_port_update_control));
    portmod_port_update_control.link_status = link;
    SOCDNX_IF_ERR_EXIT(portmod_port_update(unit, port, &portmod_port_update_control));

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_remote_fault_enable_set(int unit, bcm_port_t port, int enable)
{
    int rv; 
    portmod_remote_fault_control_t control;
    SOCDNX_INIT_FUNC_DEFS;
    
    rv = portmod_remote_fault_control_t_init(unit, &control);
    SOCDNX_IF_ERR_EXIT(rv);
    rv = portmod_port_remote_fault_control_get(unit, port, &control);
    SOCDNX_IF_ERR_EXIT(rv);
    control.enable = (uint8)enable;
    rv = portmod_port_remote_fault_control_set(unit, port, &control);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_port_nif_quad_to_core_validate(int unit)
{
    soc_pbmp_t ports_bm;
    soc_port_t port;
    int core;
    uint32 quad, phy_port, new_phy;
    soc_port_if_t interface_type; 
    int quads_core_array[SOC_JER_NIF_NOF_LANE_QUADS] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};


    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_valid_ports_get(unit, SOC_PORT_FLAGS_NETWORK_INTERFACE, &ports_bm));
    SOC_PBMP_ITER(ports_bm, port) {

        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));
        if (interface_type == SOC_PORT_IF_ILKN) {
            continue; /* ILKN quads are allowed to share quads */
        }

        /* calculate quad */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy_port, &new_phy)));
        new_phy = new_phy-1;
        quad = new_phy/4;

        /* get core */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));

        if ((quads_core_array[quad] != -1) && (quads_core_array[quad] != core)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d is mapped to a quad on a different core"), port)); 
        } else {
            quads_core_array[quad] = core;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_port_ports_to_same_quad_get
 * Purpose:
 *      gets all ports on the same quad
 * Parameters:
 *      unit    - Device Number
 *      port    - local port
 *      ports   - bit map with relevant ports
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_port_ports_to_same_quad_get(int unit, soc_port_t port, soc_pbmp_t* ports)
{
    int port_i;
    uint32 is_valid = 0;
    uint32 quad_count;
    soc_pbmp_t ref_quad_pbmp;
    soc_pbmp_t current_quad_pbmp;

    SOCDNX_INIT_FUNC_DEFS;

    /* Check input */
    SOCDNX_NULL_CHECK(ports);
    SOCDNX_IF_ERR_EXIT( soc_port_sw_db_is_valid_port_get( unit, port, &is_valid));
    if (!is_valid) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Port %d is not valid"), port)); 
    }

    /* get port's quad to ref_quad */
    SOC_PBMP_CLEAR(ref_quad_pbmp);
    SOCDNX_IF_ERR_EXIT( soc_jer_portmod_port_quad_get(unit, port, &ref_quad_pbmp));

    /* iterate over other ports and see which has the same quad */
    SOCDNX_IF_ERR_EXIT( soc_port_sw_db_valid_ports_get(unit, SOC_PORT_FLAGS_NETWORK_INTERFACE, ports));
    SOC_PBMP_ITER((*ports), port_i) 
    {
        SOC_PBMP_CLEAR(current_quad_pbmp);
        SOCDNX_IF_ERR_EXIT( soc_jer_portmod_port_quad_get(unit, port_i, &current_quad_pbmp));
        SOC_PBMP_AND(current_quad_pbmp, ref_quad_pbmp);
        SOC_PBMP_COUNT(current_quad_pbmp, quad_count);
        if (quad_count == 0) {
            SOC_PBMP_PORT_REMOVE(*ports, port_i);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_port_quad_ports_get
 * Purpose:
 *      gets all ports on given quad
 * Parameters:
 *      unit    - Device Number
 *      quad    - quad number
 *      ports   - bit map with relevant ports
 * Returns:
 *      SOC_E_XXX
 */

int soc_jer_port_quad_ports_get(int unit, uint32 quad, soc_pbmp_t* ports_bm)
{
    int port_i;
    soc_pbmp_t valid_ports;
    soc_pbmp_t quad_bmp;

    SOCDNX_INIT_FUNC_DEFS;    
    
    /* Check input */
    SOCDNX_NULL_CHECK(ports_bm);
    if(quad >= SOC_JER_NIF_NOF_LANE_QUADS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Quad %d is invalid"), quad)); 
    }

    SOC_PBMP_CLEAR(*ports_bm);
    /* get valid ports */
    SOCDNX_IF_ERR_EXIT( soc_port_sw_db_valid_ports_get(unit, SOC_PORT_FLAGS_NETWORK_INTERFACE, &valid_ports));

    /* iterate over valid ports */
    SOC_PBMP_ITER(valid_ports, port_i) {
        SOCDNX_IF_ERR_EXIT( soc_jer_portmod_port_quad_get(unit, port_i, &quad_bmp));
        /* find first valid port on reference quad */
        if( SOC_PBMP_MEMBER(quad_bmp, quad)) {
            /* find other ports on same quad */
            SOCDNX_IF_ERR_EXIT( soc_jer_port_ports_to_same_quad_get(unit, port_i, ports_bm));
            break;
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}


/* Set nbih trigger and wait until it reset */
int
soc_jer_wait_gtimer_trigger(int unit)
{
    int rv, counter;
    uint32 reg_val;

    SOCDNX_INIT_FUNC_DEFS;

    rv = READ_NBIH_GTIMER_TRIGGERr(unit, &reg_val);
    SOCDNX_IF_ERR_EXIT(rv);
    soc_reg_field_set(unit, NBIH_GTIMER_TRIGGERr, &reg_val, GTIMER_TRIGGERf, 0x0);
    rv = WRITE_NBIH_GTIMER_TRIGGERr(unit, reg_val);
    SOCDNX_IF_ERR_EXIT(rv);
    sal_usleep(500000);

    soc_reg_field_set(unit, NBIH_GTIMER_TRIGGERr, &reg_val, GTIMER_TRIGGERf, 0x1);
    rv = WRITE_NBIH_GTIMER_TRIGGERr(unit, reg_val);
    SOCDNX_IF_ERR_EXIT(rv);

    rv = READ_NBIH_GTIMER_TRIGGERr(unit, &reg_val);
    SOCDNX_IF_ERR_EXIT(rv);

    counter = 0;
    while (reg_val == 0x1) {
        sal_usleep(500000);
        rv = READ_NBIH_GTIMER_TRIGGERr(unit, &reg_val);
        SOCDNX_IF_ERR_EXIT(rv);
        if(10 == counter){
            rv = SOC_E_TIMEOUT;
            SOC_EXIT;
        }

        counter++;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_tdm_high_low_clear
 * Purpose:
 *      clears quad priority.
 * Parameters:
 *      unit            - Device Number
 *      quad            - quad to clear
 *      clear_tdm       - clear tdm priority configurations
 *      clear_high_low  - clear high and low priority configurations
 * Returns:
 *      SOC_E_XXX
 * Notes: 
 *      this function clears all quad priorities ( tdm, high and low ) 
 */
int soc_jer_nif_priority_quad_tdm_high_low_clear(int unit, uint32 quad, int clear_tdm, int clear_high_low)
{
    uint32 mask;
    uint32 reg32_val;
    soc_reg_above_64_val_t reg_above_64_val;
    soc_reg_above_64_val_t above64_mask;

    SOCDNX_INIT_FUNC_DEFS;

    /* Clearing mask */
    mask = ~(1 << quad);

    SOC_REG_ABOVE_64_SET_WORD_PATTERN(above64_mask, mask);

    /* Clear TDM */
    if (clear_tdm) {
        SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_REQ_PIPE_0_TDM_ENr(unit, &reg32_val)); 
        reg32_val &= mask;
        SOCDNX_IF_ERR_EXIT( WRITE_NBIH_RX_REQ_PIPE_0_TDM_ENr(unit, reg32_val));

        SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_REQ_PIPE_1_TDM_ENr(unit, &reg32_val)); 
        reg32_val &= mask;
        SOCDNX_IF_ERR_EXIT( WRITE_NBIH_RX_REQ_PIPE_1_TDM_ENr(unit, reg32_val));

    }

    if (clear_high_low) {
        /* Clear High */
        SOCDNX_IF_ERR_EXIT( READ_NBIH_RX_REQ_PIPE_0_HIGH_ENr(unit, &reg32_val));
        reg32_val &= mask;
        SOCDNX_IF_ERR_EXIT( WRITE_NBIH_RX_REQ_PIPE_0_HIGH_ENr(unit, reg32_val));

        SOCDNX_IF_ERR_EXIT( READ_NBIH_RX_REQ_PIPE_1_HIGH_ENr(unit, &reg32_val));
        reg32_val &= mask;
        SOCDNX_IF_ERR_EXIT( WRITE_NBIH_RX_REQ_PIPE_1_HIGH_ENr(unit, reg32_val));
	
        /* Clear Low */
        SOCDNX_IF_ERR_EXIT( READ_NBIH_RX_REQ_PIPE_0_LOW_ENr(unit, reg_above_64_val));
        SOC_REG_ABOVE_64_AND(reg_above_64_val, above64_mask);
        SOCDNX_IF_ERR_EXIT( WRITE_NBIH_RX_REQ_PIPE_0_LOW_ENr(unit, reg_above_64_val));

        SOCDNX_IF_ERR_EXIT( READ_NBIH_RX_REQ_PIPE_1_LOW_ENr(unit, reg_above_64_val));
        SOC_REG_ABOVE_64_AND(reg_above_64_val, above64_mask);
        SOCDNX_IF_ERR_EXIT( WRITE_NBIH_RX_REQ_PIPE_1_LOW_ENr(unit, reg_above_64_val));

    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_tdm_set
 * Purpose:
 *      set tdm priority according to core
 * Parameters:
 *      unit    - Device Number
 *      core    - core for setting priority
 *      quad    - quad to clear
 * Returns:
 *      SOC_E_XXX
 */
STATIC int soc_jer_nif_priority_quad_tdm_set(int unit, int core, uint32 quad)
{
    uint32 mask;
    uint32 reg32_val;
    soc_reg_t prio_reg;

    SOCDNX_INIT_FUNC_DEFS;

    prio_reg = (core == 0) ? NBIH_RX_REQ_PIPE_0_TDM_ENr : NBIH_RX_REQ_PIPE_1_TDM_ENr;

    /* Setting mask */
    mask = (1 << quad);

    /* get modify set reg */
    SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, prio_reg, REG_PORT_ANY, 0, &reg32_val));
    reg32_val |= mask;
    SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, prio_reg, REG_PORT_ANY, 0, reg32_val));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_high_set
 * Purpose:
 *      set high priority according to core
 * Parameters:
 *      unit    - Device Number
 *      core    - core for setting priority
 *      quad    - quad to clear
 * Returns:
 *      SOC_E_XXX
 */
STATIC int soc_jer_nif_priority_quad_high_set(int unit, int core, uint32 quad)
{
    uint32 mask;
    uint32 reg32_val;
    soc_reg_t prio_reg;

    SOCDNX_INIT_FUNC_DEFS;

    prio_reg = (core == 0) ? NBIH_RX_REQ_PIPE_0_HIGH_ENr : NBIH_RX_REQ_PIPE_1_HIGH_ENr;

    /* Setting mask */
    mask = (1 << quad);

    /* get modify set reg */
    SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, prio_reg, REG_PORT_ANY, 0, &reg32_val));
    reg32_val |= mask;
    SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, prio_reg, REG_PORT_ANY, 0, reg32_val));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_low_set
 * Purpose:
 *      set low priority according to core
 * Parameters:
 *      unit    - Device Number
 *      core    - core for setting priority
 *      quad    - quad to clear
 * Returns:
 *      SOC_E_XXX
 */
STATIC int soc_jer_nif_priority_quad_low_set(int unit, int core, uint32 quad)
{
    uint32 mask;
    soc_reg_above_64_val_t reg_above_64_val;
    soc_reg_above_64_val_t above64_mask;
    soc_reg_t prio_reg;

    SOCDNX_INIT_FUNC_DEFS;

    prio_reg = (core == 0) ? NBIH_RX_REQ_PIPE_0_LOW_ENr : NBIH_RX_REQ_PIPE_1_LOW_ENr;

    /* Setting mask */
    mask = (1 << quad);
    SOC_REG_ABOVE_64_SET_WORD_PATTERN(above64_mask, mask);

    /* get modify set reg */
    SOCDNX_IF_ERR_EXIT( soc_reg_above_64_get(unit, prio_reg, REG_PORT_ANY, 0, reg_above_64_val));
    SOC_REG_ABOVE_64_OR(reg_above_64_val, above64_mask);
    SOCDNX_IF_ERR_EXIT( soc_reg_above_64_set(unit, prio_reg, REG_PORT_ANY, 0, reg_above_64_val));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_ilkn_tdm_high_low_create_mask
 * Purpose:
 *      creates the set mask for given ilkn according to the needed priority.
 * Parameters:
 *      unit    - Device Number
 *      ilkn_id - id of the ilkn port (0-5)
 *      is_tdm  - set if mask needed is for tdm prio
 *      is_high - set if mask needed is for high prio
 *      mask    - return value of mask
 * Returns:
 *      SOC_E_XXX
 * Notes: 
 *      this function works for all priorities ( tdm, high and low ) 
 */

STATIC int soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(int unit, uint32 ilkn_id, int is_tdm, int is_high, uint32* mask)
{

    int phys_count,quad_count, quad;
    uint32 unique_mask;
    uint32 serdes_mask, memory_mask;
    SHR_BITDCL* serdes_qmlfs;
    SHR_BITDCL* memory_qmlfs;
    soc_pbmp_t phys, phys_copy, bm_mask;

    soc_port_t port;
    static int nif_ilkn_tdm_priority_bits[] = { 0xC0002, 0x300010, 0xC00080, 0x3000400, 0xC002000, 0x30010000};
    /* Each priority level and ilkn couple has thier own unique prioritt bits controling them, the following arrays describe those bits, array for high and low prio, and index mark the ilkn */
    static int nif_ilkn_uniqe_high_priority_bits[] = { 0xC0000, 0x300000, 0xC00000, 0x3000000, 0xC000000, 0x30000000};
    static int nif_ilkn_uniqe_low_priority_bits[] = { 0xF8600000, 0x0, 0x1C0000, 0x0, 0x7800000, 0x0};

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(mask);
    SOC_PBMP_CLEAR(phys);

    if (is_tdm) 
    {
        /* set mask according to ilkn */
        *mask = nif_ilkn_tdm_priority_bits[ilkn_id];
    } else {
        /* get taken serdes and memory qmlfs */
        serdes_qmlfs = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_qmlf_resources[ilkn_id].serdes_qmlfs; 
        memory_qmlfs = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_qmlf_resources[ilkn_id].memory_qmlfs;

        /* get uniqe mask according to wanted prio */
        unique_mask = is_high ? nif_ilkn_uniqe_high_priority_bits[ilkn_id] : nif_ilkn_uniqe_low_priority_bits[ilkn_id] ;

        /* get memory mask, the memory mask is for each ilkn the 3 coresponding qmlfs 0:0,1,2  1:3,4,5 etc. */
        memory_mask = 0x7 << (ilkn_id * 3);

        /* get serdes mask, from ILKN PMs 0-5 */
        serdes_mask = 0x3F << ((ilkn_id / 2) * SOC_JERICHO_MAX_PMS_PER_ILKN_PORT);

        SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, ilkn_id, &port));
        /*This function is called also during the init procedure to clear a scheduler register 
          and than the port argument will return invalid(-1)*/
        if (port!=-1) {
            /*get the number of the quads taken*/
            shr_bitop_range_count(serdes_qmlfs, 0, 32, &quad_count);

            /*Get a bitmap of the relevant phys taken*/
            /*The phys bitmap is 1 base and thats why it starts from the second bit*/
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_pbmp_get(unit,port,ilkn_id, &phys , NULL));

            /*get the number of the phys taken*/
            SOC_PBMP_COUNT(phys, phys_count);

            /*Check to see if we have a quad that is not completely configured by the ILKN 
              This makes it prone to nif priority conflicts if the other ILKN(small or big) is configured on the same quad
              So the logic now implemented is to reserve the shared quad bit eventually for the SMALL ILKN*/
            /* This will be executed only for ILKN0/2/4*/
            if (quad_count != phys_count/4 && !(ilkn_id & 1)) {

                /*Iteration on all possible quads for the given ILKN*/
                for (quad=0;quad<SOC_JERICHO_MAX_PMS_PER_ILKN_PORT;quad++) {

                    SOC_PBMP_CLEAR(bm_mask);
                    SOC_PBMP_CLEAR(phys_copy);

                    /*make a working buffer for the used phys*/
                    phys_copy=phys;

                    /*make a BitMask for the given quad by raising all relevant quad bits*/
                    SOC_PBMP_PORTS_RANGE_ADD(bm_mask, quad*4+(ilkn_id/2)*24+1, 4);

                    /*See how many lanes did the ILKN configured on the given quad and mask the relevant bits*/
                    SOC_PBMP_AND(phys_copy, bm_mask);

                    /*Check the two cases: 
                      1. If the two bitmaps are not equal this means that we haven't configured the whole quad
                      2. If the phys bitmap is 0 this means that we haven't configured anything on it.*/
                    if(SOC_PBMP_NEQ(phys_copy, bm_mask) && SOC_PBMP_NOT_NULL(phys_copy)) {
                        /*If both upper conditions are TRUE we clear this bit from the serdes_mask 
                          and reserve it for the small ILKN*/
                        SHR_BITCLR(&serdes_mask, ((ilkn_id / 2) * SOC_JERICHO_MAX_PMS_PER_ILKN_PORT) + quad);
                    }
                }
            }
        }

        /* set mask as uniqe mask and bits already taken by the qmlfs */
        *mask = unique_mask | (serdes_mask & serdes_qmlfs[0]) | (memory_mask & memory_qmlfs[0]);
        if (*mask == 0 && *serdes_qmlfs != 0 && *memory_qmlfs != 0) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("mask created for ILKN%d is 0."), ilkn_id)); 
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_ilkn_tdm_clear
 * Purpose:
 *      clears the tdm prio for given ilkn
 * Parameters:
 *      unit    - Device Number
 *      ilkn_id - id of the ilkn port (0-5)
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_nif_priority_ilkn_tdm_clear(int unit, uint32 ilkn_id)
{
    int i;
    int nof_cores = SOC_DPP_DEFS_GET(unit, nof_cores);
    uint32 reg32_val;
    uint32 mask;
    soc_reg_t prio_regs[] = {NBIH_RX_REQ_PIPE_0_TDM_ENr, NBIH_RX_REQ_PIPE_1_TDM_ENr};

    SOCDNX_INIT_FUNC_DEFS;

    /* create mask */
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, ilkn_id, 1, 0, &mask));
    mask = ~mask;

    /* clear prio */
    for (i = 0; i < nof_cores; ++i) {
        SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, prio_regs[i], REG_PORT_ANY, 0, &reg32_val));
        reg32_val &= mask;
        SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, prio_regs[i], REG_PORT_ANY, 0, reg32_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_ilkn_tdm_set
 * Purpose:
 *      sets the tdm prio for given ilkn
 * Parameters:
 *      unit    - Device Number
 *      core    - core for which to set priority
 *      ilkn_id - id of the ilkn port (0-5)
 * Returns:
 *      SOC_E_XXX
 */
STATIC int soc_jer_nif_priority_ilkn_tdm_set(int unit, int core, uint32 ilkn_id)
{
    uint32 reg32_val;
    uint32 mask;
    soc_reg_t prio_reg;

    SOCDNX_INIT_FUNC_DEFS;

    prio_reg = (core == 0) ? NBIH_RX_REQ_PIPE_0_TDM_ENr : NBIH_RX_REQ_PIPE_1_TDM_ENr;

    /* create mask */
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, ilkn_id, 1, 0, &mask));

    /* get modify set reg */
    SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, prio_reg, REG_PORT_ANY, 0, &reg32_val));
    reg32_val |= mask;
    SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, prio_reg, REG_PORT_ANY, 0, reg32_val));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_map_ilkn_to_hrf_reg
 * Purpose:
 *      maps an ilkn to its matching data hrf register
 * Parameters:
 *      unit    - Device Number
 *      ilkn_id - id of the ilkn port (0-5)
 *      hrf_reg - returned matching hrf data register.
 * Returns:
 *      SOC_E_XXX
 */
STATIC int soc_jer_nif_priority_map_ilkn_to_hrf_reg(int unit, uint32 ilkn_id, soc_reg_t* hrf_reg)
{
    static soc_reg_t NBIH_RX_SCH_CONFIG_HRF_Nl[] = { NBIH_RX_SCH_CONFIG_HRFr, NBIH_RX_SCH_CONFIG_HRFr, NBIH_RX_SCH_CONFIG_HRF_4r, NBIH_RX_SCH_CONFIG_HRF_5r, NBIH_RX_SCH_CONFIG_HRF_8r, NBIH_RX_SCH_CONFIG_HRF_9r};

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(hrf_reg);

    *hrf_reg = NBIH_RX_SCH_CONFIG_HRF_Nl[ilkn_id];

    exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_ilkn_high_low_clear
 * Purpose:
 *      clears high or low priorities for given ilkn.
 * Parameters:
 *      unit    - Device Number
 *      ilkn_id - id of the ilkn port (0-5)
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_nif_priority_ilkn_high_low_clear(int unit, uint32 ilkn_id)
{
    int i;
    int nof_cores = SOC_DPP_DEFS_GET(unit, nof_cores);
    soc_reg_t prio_regs_high[] = {NBIH_RX_REQ_PIPE_0_HIGH_ENr, NBIH_RX_REQ_PIPE_1_HIGH_ENr};
    soc_reg_t prio_regs_low[] = {NBIH_RX_REQ_PIPE_0_LOW_ENr, NBIH_RX_REQ_PIPE_1_LOW_ENr};
    uint32 reg32_val;
    soc_reg_above_64_val_t reg_above_64_val;
    uint32 mask;
    soc_reg_above_64_val_t above64_mask;

    SOCDNX_INIT_FUNC_DEFS;

    /* clear priority bits */
    for (i = 0; i < nof_cores; ++i) {
        /* Clear HIGH*/

        /* create mask */
        SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, ilkn_id, 0, 1, &mask));
        mask = ~mask;

        SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, prio_regs_high[i], REG_PORT_ANY, 0, &reg32_val));
        reg32_val &= mask;
        SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, prio_regs_high[i], REG_PORT_ANY, 0, reg32_val));

        /* Clear Low*/

        /* create mask */
        SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, ilkn_id, 0, 0, &mask));
        mask = ~mask;

        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_get(unit, prio_regs_low[i], REG_PORT_ANY, 0, reg_above_64_val));
        SOC_REG_ABOVE_64_SET_WORD_PATTERN(above64_mask, mask);
        SOC_REG_ABOVE_64_AND(reg_above_64_val, above64_mask);
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_set(unit, prio_regs_low[i], REG_PORT_ANY, 0, reg_above_64_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_nif_is_ilkn_share_quad(int unit, uint32 ilkn_id, int *is_share_quad, soc_port_t* share_quad_port) {

    uint32 second_ilkn_id, quad_count;
    soc_port_t port, second_ilkn_port;
    soc_pbmp_t quad_pbmp, current_quad_pbmp;
    SOCDNX_INIT_FUNC_DEFS;

    *is_share_quad = 0;
    *share_quad_port = SOC_JER_INVALID_PORT;

    /* 1. get this ilkn_id port */
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, ilkn_id, &port));
    /* 2. get the port of the other ilkn_id on the same ilkn core*/
    second_ilkn_id = (ilkn_id & 1) ? ilkn_id - 1 : ilkn_id + 1;
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, second_ilkn_id, &second_ilkn_port));

    if (second_ilkn_port != SOC_JER_INVALID_PORT) {
        /* 3. get each ports quad pbmp and compare to find common quad */
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_port_quad_get(unit, port, &current_quad_pbmp));
        SOCDNX_IF_ERR_EXIT(soc_jer_portmod_port_quad_get(unit, second_ilkn_port, &quad_pbmp));
        SOC_PBMP_AND(quad_pbmp, current_quad_pbmp);
        SOC_PBMP_COUNT(quad_pbmp, quad_count);
        if (quad_count) {
            *share_quad_port = second_ilkn_port;
            *is_share_quad = 1;
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_ilkn_high_low_set
 * Purpose:
 *      sets high or low priorities for given ilkn.
 * Parameters:
 *      unit    - Device Number
 *      core    - core of the ilkn
 *      is_high - set to 1 if priority is high
 *      ilkn_id - id of the ilkn port (0-5)
 * Returns:
 *      SOC_E_XXX
 */
STATIC int soc_jer_nif_priority_ilkn_high_low_set(int unit, int core, int is_high, uint32 ilkn_id)
{
    int index;
    uint32 mask, share_quad_ilkn_id, flags = 0;
    uint32 reg32_val;
    uint64 reg64_val;
    soc_reg_above_64_val_t reg_above_64_val;
    soc_reg_above_64_val_t above64_mask;
    soc_reg_t priority_enable_reg;
    soc_reg_t sch_config_hrf_reg;
    int is_share_quad, share_quad_port_core, priority;
    soc_port_t port, share_quad_port;

    SOCDNX_INIT_FUNC_DEFS;

    /* ILKN5 ELK over fabric will not work in low priority */
    if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, ilkn_id) && ilkn_id == 5 && is_high == 0) {
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_port_exist(unit, ilkn_id, &port));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_flags_get(unit, port, &flags));
        if (SOC_PORT_IS_ELK_INTERFACE(flags)) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("ILKN5 ELK over fabric links does not support low priority"))); 
        }
    }

    /* Check if we are trying to change priority of a shared quad */
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_is_ilkn_share_quad(unit, ilkn_id, &is_share_quad, &share_quad_port));
    if (is_share_quad) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, share_quad_port, &share_quad_port_core));
        share_quad_ilkn_id = (ilkn_id & 1) ? ilkn_id - 1 : ilkn_id + 1;
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_priority_get(unit, share_quad_port_core, share_quad_ilkn_id, 1, 0, 0, &priority));
        if (priority != (is_high ? 1 : 0)) {
            LOG_WARN(BSL_LS_BCM_PORT,(BSL_META_U(unit, "Detecting priority change for ILKN port sharing serdes quad/s with another ILKN port. "
                                                       "Note that all ports on the same serdes quad must have the same priority!\n")));
        }
    }

    /* create mask */
    SOCDNX_IF_ERR_EXIT(soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, ilkn_id, 0, 1, &mask));

    /* get relevant hrf data register */
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_map_ilkn_to_hrf_reg(unit, ilkn_id, &sch_config_hrf_reg));
    index = (ilkn_id == 1) ? 1 : 0;

    /* update info in hrf regiter, relevant prio bits, pipe and high/low prio */
    SOCDNX_IF_ERR_EXIT( soc_reg64_get(unit, sch_config_hrf_reg, REG_PORT_ANY, index, &reg64_val));
    soc_reg64_field32_set(unit, sch_config_hrf_reg, &reg64_val, HRF_RX_SCH_MAP_HRF_Nf, mask);
    soc_reg64_field32_set(unit, sch_config_hrf_reg, &reg64_val, HRF_RX_PIPE_Nf, core);
    soc_reg64_field32_set(unit, sch_config_hrf_reg, &reg64_val, HRF_RX_PRIORITY_HRF_Nf, is_high);
    SOCDNX_IF_ERR_EXIT( soc_reg64_set(unit, sch_config_hrf_reg, REG_PORT_ANY, index, reg64_val));

    /* set priority bits */
    if (is_high) 
    {
        priority_enable_reg = (core == 1) ? NBIH_RX_REQ_PIPE_1_HIGH_ENr : NBIH_RX_REQ_PIPE_0_HIGH_ENr; 
        SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, priority_enable_reg, REG_PORT_ANY, 0, &reg32_val));
        reg32_val |= mask;
        SOCDNX_IF_ERR_EXIT( soc_reg32_set(unit, priority_enable_reg, REG_PORT_ANY, 0, reg32_val));
    } else {
        priority_enable_reg = (core == 1) ? NBIH_RX_REQ_PIPE_1_LOW_ENr : NBIH_RX_REQ_PIPE_0_LOW_ENr; 
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_get(unit, priority_enable_reg, REG_PORT_ANY, 0, reg_above_64_val));
        SOC_REG_ABOVE_64_SET_WORD_PATTERN(above64_mask, mask);
        SOC_REG_ABOVE_64_OR(reg_above_64_val, above64_mask);
        SOCDNX_IF_ERR_EXIT( soc_reg_above_64_set(unit, priority_enable_reg, REG_PORT_ANY, 0, reg_above_64_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_set
 * Purpose:
 *      set nif priority
 * Parameters:
 *      unit            - Device Number
 *      core            - core of the ilkn
 *      quad_ilkn       - quad or ilkn id
 *      is_ilkn         - set to 1 if quad_ilkn is ilkn and not quad
 *      flags           - relevant flags
 *      allow_tdm       - allow configuring tdm
 *      priority_level  - tdm/high/low prio (2/1/0) 
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_nif_priority_set(   SOC_SAND_IN     int     unit,
                                SOC_SAND_IN     int     core,
                                SOC_SAND_IN     uint32  quad_ilkn,
                                SOC_SAND_IN     uint32  is_ilkn,
                                SOC_SAND_IN     uint32  flags,
                                SOC_SAND_IN     uint32  allow_tdm,
                                SOC_SAND_IN     int     priority_level)
{
    uint32 is_high_set, is_low_set, is_tdm_set, is_set_required;
    SOCDNX_INIT_FUNC_DEFS;

    if((priority_level < JER_NIF_PRIO_LOW_LEVEL) || (priority_level > JER_NIF_PRIO_HIGH_LEVEL)) 
    {
        if (priority_level != JER_NIF_PRIO_TDM_LEVEL) 
        {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("priority level %d is invalid for device"), priority_level)); 
        } else if ((priority_level == JER_NIF_PRIO_TDM_LEVEL) && (allow_tdm == 0)){
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("priority level %d can't be configured manually with this API"), priority_level)); 
        }
    }

    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_ilkn_tdm_get(unit, core, quad_ilkn, is_ilkn, &is_tdm_set));
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_ilkn_high_get(unit, core, quad_ilkn, is_ilkn, &is_high_set));
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_ilkn_low_get(unit, core, quad_ilkn, is_ilkn, &is_low_set));

    /*Check if priority set is requiried or it is already set to the correct value*/
    if ((priority_level == JER_NIF_PRIO_HIGH_LEVEL && !is_high_set) ||
        (priority_level == JER_NIF_PRIO_LOW_LEVEL && !is_low_set) ||
        (priority_level == JER_NIF_PRIO_TDM_LEVEL && !is_tdm_set))
    {
        is_set_required = 1;
    } else {
        is_set_required = 0;
    }

    if (is_set_required) {
        if (is_ilkn) 
        {
            /* Handle ILKN prio */
            if (priority_level == JER_NIF_PRIO_TDM_LEVEL) 
            {
                /* Clear existing configuration from both cores */
                SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_clear(unit, quad_ilkn));
                /* Set wanted configuration for relevant ILKN as TDM */
                SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_set(unit, core, quad_ilkn));
            } else {
                /* Clear existing configuration from both cores */
                SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_high_low_clear(unit, quad_ilkn));
                if (priority_level == JER_NIF_PRIO_HIGH_LEVEL)
                {
                    /* Set wanted configuration for relevant ILKN as HIGH */
                    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_high_low_set(unit, core, 1, quad_ilkn));
                } else /* priority_level == JER_NIF_PRIO_LOW_LEVEL */ {
                    /* Set wanted configuration for relevant ILKN as LOW */
                    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_high_low_set(unit, core, 0, quad_ilkn));
                }
            }
        } else {
            /* Handle Quad prio */
            if (priority_level == JER_NIF_PRIO_TDM_LEVEL) 
            {
                /* Clear existing configuration from both cores (TDM) */
                SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_tdm_high_low_clear(unit, quad_ilkn, 1, 0));
                /* Set wanted configuration for relevant Quad as TDM */
                SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_tdm_set(unit, core, quad_ilkn));
            } else {
                /* Clear existing configuration from both cores (HIGH and LOW) */
                SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_tdm_high_low_clear(unit, quad_ilkn, 0, 1));
                if (priority_level == JER_NIF_PRIO_HIGH_LEVEL)
                {
                    /* Set wanted configuration for relevant Quad as HIGH */
                    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_high_set(unit, core, quad_ilkn));
                } else /* priority_level == JER_NIF_PRIO_LOW_LEVEL */ {
                    /* Set wanted configuration for relevant Quad as LOW */
                    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_low_set(unit, core, quad_ilkn));
                }
            }
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_ilkn_tdm_get
 * Purpose:
 *      gets tdm priority for given quad/ilkn.
 * Parameters:
 *      unit        - Device Number
 *      core        - core
 *      quad_ilkn   - quad or ilkn id
 *      is_ilkn     - set to 1 if is ilkn
 *      is_set      - returns if tdm prio is set for quad/ilkn
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_nif_priority_quad_ilkn_tdm_get(int unit, int core, uint32 quad_ilkn, uint32 is_ilkn, uint32* is_set)
{

    uint32 mask;
    uint32 reg32_val;
    soc_reg_t tdm_en_prio_reg;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(is_set);

    /* creste mask */
    if (is_ilkn) {
        if (SOC_IS_QAX(unit)) {
            /*Each ILKN has 5 bits, starting from the 12th bit of the reg*/
            mask = 0x1F << (quad_ilkn*5 + 12);
        } else {
            SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, quad_ilkn, 1, 0, &mask));
        }
    } else {
        mask = 1 << quad_ilkn; 
    }

    /* see if any of the prio bits is set */
    tdm_en_prio_reg = (core == 1) ? NBIH_RX_REQ_PIPE_1_TDM_ENr : NBIH_RX_REQ_PIPE_0_TDM_ENr;
    SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, tdm_en_prio_reg, REG_PORT_ANY, 0, &reg32_val));
    *is_set = (reg32_val & mask) ? 1 : 0;

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_ilkn_high_get
 * Purpose:
 *      gets high priority for given quad/ilkn.
 * Parameters:
 *      unit        - Device Number
 *      core        - core
 *      quad_ilkn   - quad or ilkn id
 *      is_ilkn     - set to 1 if is ilkn
 *      is_set      - returns if tdm prio is set for quad/ilkn
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_nif_priority_quad_ilkn_high_get(int unit, int core, uint32 quad_ilkn, uint32 is_ilkn, uint32* is_set)
{

    uint32 mask;
    uint32 reg32_val;
    soc_reg_t high_en_prio_reg;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(is_set);

    /* see if any of the prio bits is set */
    high_en_prio_reg = (core == 1) ? NBIH_RX_REQ_PIPE_1_HIGH_ENr : NBIH_RX_REQ_PIPE_0_HIGH_ENr;
    SOCDNX_IF_ERR_EXIT( soc_reg32_get(unit, high_en_prio_reg, REG_PORT_ANY, 0, &reg32_val));

    /* create mask */
    if (is_ilkn) {
        /*Check only unique ILKN bits to make sure that indeed ILKN nif priority is set*/
        if (SOC_IS_QAX(unit)) {
            /*Each ILKN has 5 bits, starting from the 12th bit of the reg*/
            mask = 0x1F << (quad_ilkn*5 + 12);
        } else {
            SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, quad_ilkn, 0, 1, &mask));
        }
    } else {
        mask = 1 << quad_ilkn;
    }

    /* see if all of the prio bits are set */
    *is_set = ((reg32_val & mask) == mask) ? 1 : 0; 
exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_quad_ilkn_low_get
 * Purpose:
 *      gets low priority for given quad/ilkn.
 * Parameters:
 *      unit        - Device Number
 *      core        - core
 *      quad_ilkn   - quad or ilkn id
 *      is_ilkn     - set to 1 if is ilkn
 *      is_set      - returns if tdm prio is set for quad/ilkn
 * Returns:
 *      SOC_E_XXX
 */

int soc_jer_nif_priority_quad_ilkn_low_get(int unit, int core, uint32 quad_ilkn, uint32 is_ilkn, uint32* is_set)
{
    uint32 mask;
    soc_reg_above_64_val_t reg_above_64_val;
    soc_reg_above_64_val_t above64_mask;
    soc_reg_t low_en_prio_reg;

    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_NULL_CHECK(is_set);

    /*Get the relevant register*/
    low_en_prio_reg = (core == 1) ? NBIH_RX_REQ_PIPE_1_LOW_ENr : NBIH_RX_REQ_PIPE_0_LOW_ENr;
    SOCDNX_IF_ERR_EXIT( soc_reg_above_64_get(unit, low_en_prio_reg, REG_PORT_ANY, 0, reg_above_64_val));

    /* creste mask */
    if (is_ilkn) {
        /*Check only unique ILKN bits to make sure that indeed ILKN nif priority is set*/
        if (SOC_IS_QAX(unit)) {
            /*Each ILKN has 5 bits, starting from the 12th bit of the reg*/
            mask = 0x1F << (quad_ilkn*5 + 12);
        } else {
            SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_ilkn_tdm_high_low_create_mask(unit, quad_ilkn, 0, 0, &mask));
        }

    } else {
        mask = 1 << quad_ilkn; 
    }

    /* The LOW nif priority register is 128 bits wide
     and it repeats itself every 32 bits*/
    SOC_REG_ABOVE_64_CLEAR(above64_mask);
    SOC_REG_ABOVE_64_WORD_SET(above64_mask, mask, 0);
    SOC_REG_ABOVE_64_WORD_SET(above64_mask, mask, 1);
    SOC_REG_ABOVE_64_WORD_SET(above64_mask, mask, 2);
    SOC_REG_ABOVE_64_WORD_SET(above64_mask, mask, 3);

    SOC_REG_ABOVE_64_AND(reg_above_64_val, above64_mask);
    /* see if all of the prio bits are set */
    *is_set =  SOC_REG_ABOVE_64_IS_EQUAL(reg_above_64_val, above64_mask);

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_nif_priority_get
 * Purpose:
 *      get nif priority
 * Parameters:
 *      unit            - Device Number
 *      core            - core of the ilkn
 *      quad_ilkn       - quad or ilkn id
 *      is_ilkn         - set to 1 if quad_ilkn is ilkn and not quad
 *      flags           - relevant flags
 *      allow_tdm       - allow configuring tdm
 *      priority_level  - returned prio level tdm/high/low (2/1/0)
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_nif_priority_get(   SOC_SAND_IN     int     unit,
                                SOC_SAND_IN     int     core,
                                SOC_SAND_IN     uint32  quad_ilkn,
                                SOC_SAND_IN     uint32  is_ilkn,
                                SOC_SAND_IN     uint32  flags,
                                SOC_SAND_IN     uint32  allow_tdm,
                                SOC_SAND_OUT    int*    priority_level)
{
    uint32 is_set;

    SOCDNX_INIT_FUNC_DEFS;

    /* set priority level with invalid value */
    *priority_level = -1;

    /* check if tdm prio when relevant flag is on */
    if ( allow_tdm ) {
        SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_ilkn_tdm_get(unit, core, quad_ilkn, is_ilkn, &is_set));
        if (is_set) {
            *priority_level = JER_NIF_PRIO_TDM_LEVEL;
            SOC_EXIT;
        }
    }

    /* check if high prio */
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_ilkn_high_get(unit, core, quad_ilkn, is_ilkn, &is_set));
    if (is_set) {
        *priority_level = JER_NIF_PRIO_HIGH_LEVEL;
        SOC_EXIT;
    }

    /* check if low prio */
    SOCDNX_IF_ERR_EXIT( soc_jer_nif_priority_quad_ilkn_low_get(unit, core, quad_ilkn, is_ilkn, &is_set));
    if (is_set) {
        *priority_level = JER_NIF_PRIO_LOW_LEVEL;
        SOC_EXIT;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * How to get serdes frequency:
 * We can't measure the serdes freq directly, but we can measure the sync_eth counter, and reconstruct the serdes freq from it by multiplying it with the blocks dividers.
 *
 * In PML and PMH GSMII the dividers are:
 *  VCO                                                   Sync_eth counter
 *   _        --> PM synce_div --> NBIL/H synce div -->         _
 * _| |_                                                      _| |_
 * So 
 * VCO = Fsynce * PMH_40_PML_20 * PM_1_7_11
 * and we need to do:
 * SerDes_rate = VCO/Oversample = Fsynce * PMH_40_PML_20 * PM_1_7_11 / Oversample
 *
 *
 * In PMH which is not GSMII the dividers are:
 * Serdes freq                                            Sync_eth counter
 *   _         --> PM synce_div --> NBIL/H synce div -->        _
 * _| |_                                                      _| |_
 * So we need to do:
 * SerDes_rate = Fsynce * PMH_40_PML_20 * PM_1_7_11
 */


int
soc_jer_phy_nif_measure(int unit, soc_port_t port, uint32 *type_of_bit_clk, int *one_clk_time_measured_int, int *one_clk_time_measured_remainder, int *serdes_freq_int, int *serdes_freq_remainder, uint32 *lane)
{
    uint32 sync_sth_cnt;
    int serdes_freq_int_temp, serdes_freq_remainder_temp;
    int total_time_measured_int, total_time_measured_remainder;
    int number_of_gtimer_cycles = 10240;
    int clock_speed_int, clock_speed_remainder;
    int one_bit_clk_period_int, one_bit_clk_period_remainder;
    soc_dpp_config_arad_t *dpp_arad;
    uint32 reg_val, pmh_synce_rstn_prev = 0, synce_prev_config = 0;
    uint64 reg_val_64, default_configuration;
    int synce_read = 0, pmh_synce_read = 0;
    soc_reg_above_64_val_t reg_val_above_64;
    uint32 os_int, os_remainder;
    int rv = SOC_E_NONE;
    uint16 lane_map;
    uint32 swapped_lane;
    uint32 synce_div = 2, vco_div;
    SOC_JER_NIF_PLL_TYPE jer_pll_type;
    SOC_QUX_NIF_PLL_TYPE qux_pll_type;
    soc_port_if_t interface_type;
    portmod_access_get_params_t params;
    phymod_phy_access_t phy_access[SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT];
    int nof_phys, i, speed, is_tscf;

    SOCDNX_INIT_FUNC_DEFS;

    SOC_REG_ABOVE_64_CLEAR(reg_val_above_64);

    if (SOC_IS_QUX(unit)) {
        rv = soc_qux_port_pll_type_get(unit, port, &qux_pll_type);
    } else {
        rv = soc_jer_port_pll_type_get(unit, port, &jer_pll_type);
    }
    SOCDNX_IF_ERR_EXIT(rv);

    if (SOC_IS_QUX(unit) && qux_pll_type == SOC_QUX_NIF_PLL_TYPE_PMX) {
        *one_clk_time_measured_int = -1;
        rv = soc_qux_phy_nif_measure(unit, port, serdes_freq_int, serdes_freq_remainder);
        SOCDNX_IF_ERR_EXIT(rv);
        SOC_EXIT;
    }
    rv = soc_port_sw_db_interface_type_get(unit, port, &interface_type);
    SOCDNX_IF_ERR_EXIT(rv);



    /* Save previous configuration to restore at exit */
    if (SOC_IS_QUX(unit)) {
        rv = READ_NIF_GTIMER_CONFIGURATIONr(unit, &default_configuration);
    } else {
        rv = READ_NBIH_GTIMER_CONFIGURATIONr(unit, &default_configuration);
    }
    SOCDNX_IF_ERR_EXIT(rv);

    /* Get physical lane
     *
     * in the range of 1-72 */
    if(SOC_IS_DIRECT_PORT(unit, port)) {
        *lane = port - SOC_INFO(unit).physical_port_offset;
    } else {
        *lane = SOC_INFO(unit).port_l2p_mapping[port]; /*lane should be 1-based*/
        
        if (*lane >= SOC_JER_NIF_FIRST_FABRIC_PHY_PORT(unit) - 1)
        {
            *one_clk_time_measured_int = -1;
            SOC_EXIT;
        }
    }
    rv = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove,
            (unit, *lane, lane));
    SOCDNX_IF_ERR_EXIT(rv);
    *lane -= 1; /*make lane 0-based*/

    /* For ILKN with 5 lanes there is a problem measuring the NIF rate
       when the single lane is configured on a quad with swaped lane map
       so we don't take the board but the actual chip lane */
    if (IS_IL_PORT(unit, port)) {
        swapped_lane = (*lane);
    } else {
        /* For Ethernet ports we DO need the board's swapped lane*/
        /* get swapped lane map. ex: 2301 */
        lane_map = soc_property_suffix_num_get(unit, (*lane)/4, spn_PHY_RX_LANE_MAP, "quad", 0x3210) & 0xffff;
        /* find swapped number of lane (0-3). ex: if lane=14 -> swapped_lane=3 */
        swapped_lane = ((lane_map >> (((*lane)%4)*4)) & 0xf);
        /* add base of lane to get swapped_lane. ex: if lane=14 -> swapped_lane=14-2+3=15 */
        swapped_lane = (*lane)-((*lane)%4)+swapped_lane;
    }

    if (SOC_IS_QUX(unit)) {
        rv = READ_NIF_GTIMER_CONFIGURATIONr(unit, &reg_val_64);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Config NBIH Gtimer */
        soc_reg64_field32_set(unit, NIF_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_ENABLEf, 0x1);
        soc_reg64_field32_set(unit, NIF_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_CYCLEf, number_of_gtimer_cycles);
        soc_reg64_field32_set(unit, NIF_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_RESET_ON_TRIGGERf, 0x0);
        rv = WRITE_NIF_GTIMER_CONFIGURATIONr(unit, reg_val_64);
    } else {
        rv = READ_NBIH_GTIMER_CONFIGURATIONr(unit, &reg_val_64);
        SOCDNX_IF_ERR_EXIT(rv);

        /* Config NBIH Gtimer */
        soc_reg64_field32_set(unit, NBIH_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_ENABLEf, 0x1);
        soc_reg64_field32_set(unit, NBIH_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_CYCLEf, number_of_gtimer_cycles);
        soc_reg64_field32_set(unit, NBIH_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_RESET_ON_TRIGGERf, 0x0);
        rv = WRITE_NBIH_GTIMER_CONFIGURATIONr(unit, reg_val_64);
    }
    SOCDNX_IF_ERR_EXIT(rv);

    /* Enable synce */
    rv = READ_ECI_GP_CONTROL_9r(unit, reg_val_above_64);
    pmh_synce_read = 1;
    pmh_synce_rstn_prev = soc_reg_above_64_field32_get(unit, ECI_GP_CONTROL_9r, reg_val_above_64, PMH_SYNCE_RSTNf); /* Save configuration to restore at end of function */
    soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_val_above_64, PMH_SYNCE_RSTNf, 0x1);
    rv = WRITE_ECI_GP_CONTROL_9r(unit, reg_val_above_64);
    SOCDNX_IF_ERR_EXIT(rv);

    /* Config synce */
    if (SOC_IS_QUX(unit)) {
        rv = READ_NIF_SYNC_ETH_CFGr(unit, 0, &reg_val);
        SOCDNX_IF_ERR_EXIT(rv);
        synce_read = 1;
        synce_prev_config = reg_val; /* Save synce configuration to restore at end of function */
        soc_reg_field_set(unit, NIF_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_CLOCK_SEL_Nf, swapped_lane);
        soc_reg_field_set(unit, NIF_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_CLOCK_DIV_Nf, 0x1);
        soc_reg_field_set(unit, NIF_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_SQUELCH_EN_Nf, 0x1);
        soc_reg_field_set(unit, NIF_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_LINK_VALID_SEL_Nf, 0x1);
        soc_reg_field_set(unit, NIF_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_GTIMER_MODE_Nf, 0x1);
        rv = WRITE_NIF_SYNC_ETH_CFGr(unit, 0, reg_val);
    } else {
        rv = READ_NBIH_SYNC_ETH_CFGr(unit, 0, &reg_val);
        SOCDNX_IF_ERR_EXIT(rv);
        synce_read = 1;
        synce_prev_config = reg_val; /* Save synce configuration to restore at end of function */
        soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_CLOCK_SEL_Nf, swapped_lane);
        soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_CLOCK_DIV_Nf, 0x1);
        soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_SQUELCH_EN_Nf, 0x1);
        soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_LINK_VALID_SEL_Nf, 0x1);
        soc_reg_field_set(unit, NBIH_SYNC_ETH_CFGr, &reg_val, SYNC_ETH_GTIMER_MODE_Nf, 0x1);
        rv = WRITE_NBIH_SYNC_ETH_CFGr(unit, 0, reg_val);
    }
    SOCDNX_IF_ERR_EXIT(rv);

    rv = MBCM_DPP_DRIVER_CALL(unit, mbcm_dpp_wait_gtimer_trigger, (unit));
    SOCDNX_IF_ERR_EXIT(rv);

    dpp_arad = SOC_DPP_CONFIG(unit)->arad;
    if(0 == dpp_arad->init.core_freq.frequency){
        rv = SOC_E_INTERNAL;
        SOC_EXIT;
    }
    clock_speed_int = (1000000 / dpp_arad->init.core_freq.frequency);
    clock_speed_remainder = ((100000000 / (dpp_arad->init.core_freq.frequency / 100))) % 10000; /* devided by 100 instead of 10000 for better resolution */

    total_time_measured_int = (((clock_speed_int * 10000) + clock_speed_remainder) * number_of_gtimer_cycles) / 10000;
    total_time_measured_remainder = ((int)(((clock_speed_int * 10000) + clock_speed_remainder) * (number_of_gtimer_cycles)) % 10000);

    /* Read synce counter */
    if (SOC_IS_QUX(unit)) {
        rv = READ_NIF_SYNC_ETH_COUNTERr(unit, 0, &sync_sth_cnt);
    } else {
        rv = READ_NBIH_SYNC_ETH_COUNTERr(unit, 0, &sync_sth_cnt);
    }
    SOCDNX_IF_ERR_EXIT(rv);


    rv = portmod_access_get_params_t_init(unit, &params);
    SOCDNX_IF_ERR_EXIT(rv);
    for (i = 0; i < SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT; ++i) {
        rv = phymod_phy_access_t_init(&phy_access[i]);
        SOCDNX_IF_ERR_EXIT(rv);
    }

    params.phyn=0;
    rv = portmod_port_phy_lane_access_get(unit, port, &params, SOC_DCMN_PORT_MAX_CORE_ACCESS_PER_PORT,  phy_access, &nof_phys, NULL);
    if (rv != BCM_E_NONE) {
        cli_out("ERROR: Failed to get lane access: %s\n", bcm_errmsg(rv));
        goto exit;
    }

    if (IS_QSGMII_PORT(unit, port)) {
        if (phy_access[0].access.lane_mask < 0x10) {
            phy_access[0].access.lane_mask = 0x1;
        } else if (phy_access[0].access.lane_mask < 0x100) {
            phy_access[0].access.lane_mask = 0x2;
        } else if (phy_access[0].access.lane_mask < 0x1000) {
            phy_access[0].access.lane_mask = 0x4;
        } else if (phy_access[0].access.lane_mask < 0x10000) {
            phy_access[0].access.lane_mask = 0x8;
        }
    }
    if (SOC_IS_QUX(unit)) {
        is_tscf = FALSE;
    } else {
        is_tscf = ((jer_pll_type == SOC_JER_NIF_PLL_TYPE_PMH) || (SOC_IS_JERICHO_PLUS_ONLY(unit) && jer_pll_type == SOC_JER_NIF_PLL_TYPE_PML1));
    }
    if (is_tscf) {
        BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_t synce_ctrl;
        rv = BCMI_TSCF_XGXS_READ_MAIN0_SYNCE_CTLr(&phy_access[0].access, &synce_ctrl);
        SOCDNX_IF_ERR_EXIT(rv);
        if (phy_access[0].access.lane_mask & 0x1) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE0f_GET(synce_ctrl);
        } else if (phy_access[0].access.lane_mask & 0x2) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE1f_GET(synce_ctrl);
        } else if (phy_access[0].access.lane_mask & 0x4) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE2f_GET(synce_ctrl);
        } else if (phy_access[0].access.lane_mask & 0x8) {
            synce_div = BCMI_TSCF_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LANE3f_GET(synce_ctrl);
        } else {
            SOC_EXIT;
        }
    } else {
        BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_t synce_ctrl;
        rv = BCMI_TSCE_XGXS_READ_MAIN0_SYNCE_CTLr(&phy_access[0].access, &synce_ctrl);
        SOCDNX_IF_ERR_EXIT(rv);
        if (phy_access[0].access.lane_mask & 0x1) {
            synce_div = BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN0f_GET(synce_ctrl);
        } else if (phy_access[0].access.lane_mask & 0x2) {
            synce_div = BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN1f_GET(synce_ctrl);
        } else if (phy_access[0].access.lane_mask & 0x4) {
            synce_div = BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN2f_GET(synce_ctrl);
        } else if (phy_access[0].access.lane_mask & 0x8) {
            synce_div= BCMI_TSCE_XGXS_MAIN0_SYNCE_CTLr_SYNCE_MODE_PHY_LN3f_GET(synce_ctrl);
        } else {
            SOC_EXIT;
        }
    }

    if (2 == synce_div) {
        *type_of_bit_clk = 11;
    } else if (1 == synce_div) {
        *type_of_bit_clk = 7;
    } else {
        *type_of_bit_clk = 1;
    }

    if (SOC_IS_QUX(unit)) {
        rv = READ_NIF_GTIMER_CONFIGURATIONr(unit, &reg_val_64);
        SOCDNX_IF_ERR_EXIT(rv);
        soc_reg64_field32_set(unit, NIF_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_ENABLEf, 0x0);
        rv = WRITE_NIF_GTIMER_CONFIGURATIONr(unit, reg_val_64);
    } else {
        rv = READ_NBIH_GTIMER_CONFIGURATIONr(unit, &reg_val_64);
        SOCDNX_IF_ERR_EXIT(rv);
        soc_reg64_field32_set(unit, NBIH_GTIMER_CONFIGURATIONr, &reg_val_64, GTIMER_ENABLEf, 0x0);
        rv = WRITE_NBIH_GTIMER_CONFIGURATIONr(unit, reg_val_64);
    }
    SOCDNX_IF_ERR_EXIT(rv);

    /*
     * If the interface type is XAUI for TSCF, the VCO divider is 80,
     * otherwise VCO divider is 40 for TSCF and 20 for TSCE.
     */
    vco_div = (is_tscf) ? ((interface_type == BCM_PORT_IF_XAUI) ? 80 : 40) : 20;

    /* Get serdes_freq from total_time_measured by removing all the dividers between them */
    if(0 == sync_sth_cnt){
        rv = SOC_E_INTERNAL;
        SOC_EXIT;
    }
    *one_clk_time_measured_int = (((total_time_measured_int*10000) + (total_time_measured_remainder)) / (sync_sth_cnt * vco_div)) / 10000;
    *one_clk_time_measured_remainder = ((((total_time_measured_int*10000) + (total_time_measured_remainder)) / (sync_sth_cnt * vco_div)) % 10000);
    if(0 == *type_of_bit_clk){
        rv = SOC_E_INTERNAL;
        SOC_EXIT;
    }
    one_bit_clk_period_int = ((*one_clk_time_measured_int*10000 + *one_clk_time_measured_remainder) / *type_of_bit_clk) / 10000;
    one_bit_clk_period_remainder = (((*one_clk_time_measured_int*10000 + *one_clk_time_measured_remainder) / *type_of_bit_clk) % 10000);
    if(0 == (one_bit_clk_period_int*10000 + one_bit_clk_period_remainder)){
        rv = SOC_E_INTERNAL;
        SOC_EXIT;
    }

    serdes_freq_int_temp = (10000/(one_bit_clk_period_int*10000 + one_bit_clk_period_remainder));
    serdes_freq_remainder_temp = (10000000/(one_bit_clk_period_int*10000 + one_bit_clk_period_remainder)) % 1000;

    /* Get oversample */
    rv = soc_jer_portmod_calc_os(unit, &phy_access[0], &os_int, &os_remainder);
    SOCDNX_IF_ERR_EXIT(rv);
    
    /*  The HW is connected in a way that the oversample affects the SyncE->SerDesRate calculation
        only on TSCE and TSCF SGMII. Otherwise - we should ignore it ! */
    if ((is_tscf) && (!((interface_type == BCM_PORT_IF_SGMII) || (interface_type == BCM_PORT_IF_XAUI)))){
        os_int = 1;
        os_remainder = 0;
    }

    *serdes_freq_int = ((((serdes_freq_int_temp * 1000) + serdes_freq_remainder_temp) * 1000) / ((os_int * 1000) + os_remainder)) / 1000;
    *serdes_freq_remainder = ((((serdes_freq_int_temp * 1000) + serdes_freq_remainder_temp) * 1000) / ((os_int * 1000) + os_remainder)) % 1000;

    rv = bcm_port_speed_get(unit, port, &speed);
    SOCDNX_IF_ERR_EXIT(rv);

    if (speed == 100) {
        *serdes_freq_int = 0;
        *serdes_freq_remainder = *serdes_freq_remainder/10 + 100;
    }

    if (speed == 10) {
        *serdes_freq_int = 0;
        *serdes_freq_remainder = *serdes_freq_remainder/100 + 10;
    }



exit:
    if (unit < SOC_MAX_NUM_DEVICES) {
        if (SOC_IS_QUX(unit)) {
            if(WRITE_NIF_GTIMER_CONFIGURATIONr(unit, default_configuration) != SOC_E_NONE) {
                cli_out("WRITE_NIF_GTIMER_CONFIGURATIONr failed\n");
            }
        } else {
            if(WRITE_NBIH_GTIMER_CONFIGURATIONr(unit, default_configuration) != SOC_E_NONE) {
                cli_out("WRITE_NBIH_GTIMER_CONFIGURATIONr failed\n");
            }
        }
        if (pmh_synce_read) {
            if (READ_ECI_GP_CONTROL_9r(unit, reg_val_above_64) == SOC_E_NONE) {
                soc_reg_above_64_field32_set(unit, ECI_GP_CONTROL_9r, reg_val_above_64, PMH_SYNCE_RSTNf, pmh_synce_rstn_prev);
                if(WRITE_ECI_GP_CONTROL_9r(unit, reg_val_above_64) != SOC_E_NONE) {
                    cli_out("WRITE_ECI_GP_CONTROL_9r failed\n");
                }
            }
        }

        if (synce_read) {
            if (SOC_IS_QUX(unit)) {
                WRITE_NIF_SYNC_ETH_CFGr(unit, 0, synce_prev_config);
            } else {
                WRITE_NBIH_SYNC_ETH_CFGr(unit, 0, synce_prev_config);
            }
        }
    }

    SOCDNX_FUNC_RETURN;
}

int
soc_jer_phy_nif_pll_div_get(int unit, soc_port_t port, soc_dcmn_init_serdes_ref_clock_t *ref_clk, int *p_div, int *n_div, int *m0_div)
{
    int rv = SOC_E_NONE;
    soc_reg_above_64_val_t reg_val;

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_QUX(unit)) {
        SOC_QUX_NIF_PLL_TYPE qux_pll_type;
        rv = soc_qux_port_pll_type_get(unit, port, &qux_pll_type);
        SOCDNX_IF_ERR_EXIT(rv);

        if (qux_pll_type == SOC_QUX_NIF_PLL_TYPE_PML) {
            rv = READ_ECI_NIF_PML_PLL_CONFIGr(unit, reg_val);
            SOCDNX_IF_ERR_EXIT(rv);

            *n_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_PLL_CONFIGr, reg_val,  ITEM_294_303f);
            *p_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_PLL_CONFIGr, reg_val, ITEM_6_9f);
            *m0_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_PLL_CONFIGr, reg_val, ITEM_77_84f);
            *ref_clk = SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[0];
        } else { /* PMX */
            rv = READ_ECI_NIF_PMX_PLL_CONFIGr(unit, reg_val);
            SOCDNX_IF_ERR_EXIT(rv);

            *n_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PMX_PLL_CONFIGr, reg_val, ITEM_294_303f);
            *p_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PMX_PLL_CONFIGr, reg_val, ITEM_6_9f);
            *m0_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PMX_PLL_CONFIGr, reg_val, ITEM_77_84f);
            *ref_clk = SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmx_in;
        }
    } else {
        SOC_JER_NIF_PLL_TYPE jer_pll_type;
        rv = soc_jer_port_pll_type_get(unit, port, &jer_pll_type);
        SOCDNX_IF_ERR_EXIT(rv);
        if (jer_pll_type == SOC_JER_NIF_PLL_TYPE_PMH) {
            rv = READ_ECI_NIF_PMH_PLL_CONFIGr(unit, reg_val);
            SOCDNX_IF_ERR_EXIT(rv);

            *n_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PMH_PLL_FBDIV_NDIV_INTf : NIF_PMH_PLL_CFG_NDIVf));
            *p_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PMH_PLL_PDIVf : NIF_PMH_PLL_CFG_PDIVf));
            *m0_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PMH_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PMH_PLL_CH_0_MDIVf : NIF_PMH_PLL_CFG_CH_0_MDIVf));
            *ref_clk = SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pmh_in;
        } else if (jer_pll_type == SOC_JER_NIF_PLL_TYPE_PML0) {
            rv = READ_ECI_NIF_PML_0_PLL_CONFIGr(unit, reg_val);
            SOCDNX_IF_ERR_EXIT(rv);

            *n_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PML_0_PLL_FBDIV_NDIV_INTf : NIF_PML_0_PLL_CFG_NDIVf));
            *p_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PML_0_PLL_PDIVf : NIF_PML_0_PLL_CFG_PDIVf));
            *m0_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_0_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PML_0_PLL_CH_0_MDIVf : NIF_PML_0_PLL_CFG_CH_0_MDIVf));
            *ref_clk = SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[0];
        } else { /* pll_type == SOC_JER_NIF_PLL_TYPE_PML1 */
            rv = READ_ECI_NIF_PML_1_PLL_CONFIGr(unit, reg_val);
            SOCDNX_IF_ERR_EXIT(rv);

            *n_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PML_1_PLL_FBDIV_NDIV_INTf : NIF_PML_1_PLL_CFG_NDIVf));
            *p_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PML_1_PLL_PDIVf : NIF_PML_1_PLL_CFG_PDIVf));
            *m0_div = soc_reg_above_64_field32_get(unit, ECI_NIF_PML_1_PLL_CONFIGr, reg_val, (SOC_IS_QAX(unit) ? PML_1_PLL_CH_0_MDIVf : NIF_PML_1_PLL_CFG_CH_0_MDIVf));
            *ref_clk = SOC_DPP_CONFIG(unit)->jer->pll.ref_clk_pml_in[1];
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}


int
jer_nif_ilkn_counter_clear(int unit, soc_port_t port) {

    uint32 channel, reg_val;
    int first_channel, nof_channels;
    int ch_index;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_channel_get(unit, port, &channel));

    /* 
     * In Queue-level Flow Control mode the ILKN single logical port actually consists of 16 channels.
     * to support the ILKN SLE counters in this mode we clear all 16 channels. 
     */  
    first_channel = SOC_DPP_CONFIG((unit))->tm.queue_level_interface_enable ? 0 : channel;
    nof_channels = SOC_DPP_CONFIG((unit))->tm.queue_level_interface_enable ? SOC_JER_NIF_QUEUE_LEVEL_FLOW_CTRL_NOF_CHANNELS : 1;

    for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
        SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_ACCr(unit, port, &reg_val));
        soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_TYPEf, 0x0);
        soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_CMDf, 0x2);
        soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_ADDRf, ch_index);
        SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_RX_STATS_ACCr(unit, port, reg_val));


        SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_ACCr(unit, port, &reg_val));
        soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_TYPEf, 0x0);
        soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_CMDf, 0x2);
        soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_ADDRf, ch_index);
        SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_TX_STATS_ACCr(unit, port, reg_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


int
  jer_nif_ilkn_counter_get(
    SOC_SAND_IN  int                      unit,
    SOC_SAND_IN  soc_port_t               port,
    SOC_SAND_IN  soc_jer_counters_t       counter_type,
    SOC_SAND_OUT uint64                   *counter_val
  )
{
    int first_channel, nof_channels;
    uint32 channel, ctr_low, ctr_high, reg_val;
    uint64 tmp_counter;
    int ch_index;
    SOCDNX_INIT_FUNC_DEFS;

    COMPILER_64_ZERO(*counter_val);

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_channel_get(unit, port, &channel));

    /* 
     * In Queue-level Flow Control mode the ILKN single logical port actually consists of 16 channels.
     * to support the ILKN SLE counters in this mode we sum all 16 channels. 
     * in normal mode, the for loop will have a single iteration, only for the specified channel. 
     */ 
    first_channel = SOC_DPP_CONFIG((unit))->tm.queue_level_interface_enable ? 0 : channel;
    nof_channels = SOC_DPP_CONFIG((unit))->tm.queue_level_interface_enable ? SOC_JER_NIF_QUEUE_LEVEL_FLOW_CTRL_NOF_CHANNELS : 1;
    
    switch (counter_type) {
    case soc_jer_counters_ilkn_rx_pkt_counter:
        for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_ACCr(unit, port, &reg_val)); 
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_TYPEf, 0x0);
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_CMDf, 0x1);
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_ADDRf, ch_index);
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_set_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_RX_STATS_ACCr(unit, port, reg_val));
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_restore_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_RD_PKT_LOWr(unit, port, &ctr_low));
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_RD_PKT_HIGHr(unit, port, &ctr_high));
            COMPILER_64_SET(tmp_counter, ctr_high, ctr_low);
            COMPILER_64_ADD_64(*counter_val, tmp_counter);
        }
        break;
    case soc_jer_counters_ilkn_tx_pkt_counter:
        for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_ACCr(unit, port, &reg_val));
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_TYPEf, 0x0);
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_CMDf, 0x1);
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_ADDRf, ch_index);
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_set_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_TX_STATS_ACCr(unit, port, reg_val));
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_restore_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_RD_PKT_LOWr(unit, port, &ctr_low));
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_RD_PKT_HIGHr(unit, port, &ctr_high));
            COMPILER_64_SET(tmp_counter, ctr_high, ctr_low);
            COMPILER_64_ADD_64(*counter_val, tmp_counter);
        }
        break;
    case soc_jer_counters_ilkn_rx_byte_counter:
        for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_ACCr(unit, port, &reg_val));
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_TYPEf, 0x0);
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_CMDf, 0x1);
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_ADDRf, ch_index);
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_set_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_RX_STATS_ACCr(unit, port, reg_val));
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_restore_immediate_hw_access(unit);
#endif

            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_RD_BYTE_LOWr(unit, port, &ctr_low));
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_RD_BYTE_HIGHr(unit, port, &ctr_high));
            COMPILER_64_SET(tmp_counter, ctr_high, ctr_low);
            COMPILER_64_ADD_64(*counter_val, tmp_counter);
        }
        break;
    case soc_jer_counters_ilkn_tx_byte_counter:
        for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_ACCr(unit, port, &reg_val));
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_TYPEf, 0x0);
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_CMDf, 0x1);
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_ADDRf, ch_index);
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_set_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_TX_STATS_ACCr(unit, port, reg_val));
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_restore_immediate_hw_access(unit);
#endif

            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_RD_BYTE_LOWr(unit, port, &ctr_low));
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_RD_BYTE_HIGHr(unit, port, &ctr_high));
            COMPILER_64_SET(tmp_counter, ctr_high, ctr_low);
            COMPILER_64_ADD_64(*counter_val, tmp_counter);
        }
        break;
    case soc_jer_counters_ilkn_rx_err_pkt_counter:
        for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_ACCr(unit, port, &reg_val));
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_TYPEf, 0x0);
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_CMDf, 0x1);
            soc_reg_field_set(unit, ILKN_SLE_RX_STATS_ACCr, &reg_val, SLE_RX_STATS_ACC_ADDRf, ch_index);
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_set_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_RX_STATS_ACCr(unit, port, reg_val));
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_restore_immediate_hw_access(unit);
#endif

            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_RD_ERR_LOWr(unit, port, &ctr_low));
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_RX_STATS_RD_ERR_HIGHr(unit, port, &ctr_high));
            COMPILER_64_SET(tmp_counter, ctr_high, ctr_low);
            COMPILER_64_ADD_64(*counter_val, tmp_counter);
        }
        break;
    case soc_jer_counters_ilkn_tx_err_pkt_counter:
        for (ch_index = first_channel; ch_index < first_channel + nof_channels; ++ch_index) {
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_ACCr(unit, port, &reg_val));
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_TYPEf, 0x0);
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_CMDf, 0x1);
            soc_reg_field_set(unit, ILKN_SLE_TX_STATS_ACCr, &reg_val, SLE_TX_STATS_ACC_ADDRf, ch_index);
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_set_immediate_hw_access(unit);
#endif
            SOCDNX_IF_ERR_EXIT(WRITE_ILKN_SLE_TX_STATS_ACCr(unit, port, reg_val));
#ifdef CRASH_RECOVERY_SUPPORT
soc_hw_restore_immediate_hw_access(unit);
#endif

            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_RD_ERR_LOWr(unit, port, &ctr_low));
            SOCDNX_IF_ERR_EXIT(READ_ILKN_SLE_TX_STATS_RD_ERR_HIGHr(unit, port, &ctr_high));
            COMPILER_64_SET(tmp_counter, ctr_high, ctr_low);
            COMPILER_64_ADD_64(*counter_val, tmp_counter);
        }
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Counter type %d is invalid"), counter_type)); 
    }
exit:
    SOCDNX_FUNC_RETURN;
}


int
  soc_jer_nif_port_rx_enable_get(
    int                         unit,
    soc_port_t                  port,
    int                         *enable
  )
{

    int flags;
    SOCDNX_INIT_FUNC_DEFS;
    flags = PORTMOD_PORT_ENABLE_RX;

   SOCDNX_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, enable));

exit:
    SOCDNX_FUNC_RETURN;;

}

int
  soc_jer_nif_port_rx_enable_set(
    int                         unit,
    soc_port_t                  port,
    int                       enable
  )
{
    uint32 offset;
    int flags;
    SOCDNX_INIT_FUNC_DEFS;
    flags = PORTMOD_PORT_ENABLE_RX;

    if (IS_IL_PORT(unit, port)) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset)) {
            /*in dnx_fabric the only special case to handle RX is RX+MAC*/
            flags |= PORTMOD_PORT_ENABLE_MAC;
        }
    }
    SOCDNX_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, enable)); 

exit:
    SOCDNX_FUNC_RETURN;

}

int
  soc_jer_nif_port_tx_enable_get(
    int                         unit,
    soc_port_t                  port,
    int                         *enable
  )
{

    int flags;
    SOCDNX_INIT_FUNC_DEFS;
    flags = PORTMOD_PORT_ENABLE_TX;

   SOCDNX_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, enable));

exit:
    SOCDNX_FUNC_RETURN;;

}

int
  soc_jer_nif_port_tx_enable_set(
    int                         unit,
    soc_port_t                  port,
    int                       enable
  )
{

    int flags;
    SOCDNX_INIT_FUNC_DEFS;
    flags = PORTMOD_PORT_ENABLE_TX;

    SOCDNX_IF_ERR_EXIT(portmod_port_enable_set(unit, port, flags, enable));

exit:
    SOCDNX_FUNC_RETURN;

}

int soc_jer_port_is_pcs_loopback(int unit, soc_port_t port, int *result)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(portmod_port_loopback_get(unit, port, portmodLoopbackPhyGloopPCS, result));

    exit:
        SOCDNX_FUNC_RETURN;
}

STATIC 
int soc_jer_plus_port_prd_enable_set(int unit, soc_port_t port, uint32 flags, int enable_mode) 
{
    int phy, tm_port = 0, eth_port = 0, en_soft_stage_tm = 0, en_soft_stage_eth = 0;
    uint32 prd_port_type = 0, is_hg, en_bit, en_mask=0, curr_en_mask, nof_lanes_nbi, num_lanes, offset;
    SOC_TMC_PORT_HEADER_TYPE hdr_type;
    soc_pbmp_t nif_ports, phys;
    soc_jer_nif_prd_hard_stage_control_t hard_stage_ctrl;
    soc_jer_nif_prd_hard_stage_properties_t hard_stage_properties;
    soc_jer_nif_prd_config_t prd_config;
    int nof_segments;
    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.get(unit, port, &hdr_type));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_hg_get(unit, port, &is_hg));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    /* Calculate PRD enable val acording to the port physical phys.
     * for ILKN there is one bit per ILKN IF */
    SOC_PBMP_ITER(phys, phy) 
    {
        en_bit = 1 << ( (phy-1) % nof_lanes_nbi);
        en_mask |= en_bit;
    }
   
    if (IS_IL_PORT(unit, port)) 
    {
        /* ILKN PRD enable bits are 25:24 for HRFs 0 and 1  - override the en_mask */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
        nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];
        en_mask = (offset & 1) ? SOC_JER_PLUS_PRD_EN_ILKN_SECOND_TWO_SEGMENTS : SOC_JER_PLUS_PRD_EN_ILKN_FIRST_TWO_SEGMENTS;
        if (nof_segments == 4) {
            /* if ILKN0/2/4 has more than 12 lanes, both bits of ILKN0/1 should be enabled */
            en_mask |= SOC_JER_PLUS_PRD_EN_ILKN_SECOND_TWO_SEGMENTS;
        }
    }

    /* Get current PRD properties */
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &hard_stage_properties));
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_config_get(unit, port, &prd_config));

    /* ITMH type : 0 = Arad based ITMH, 1 = Jericho based ITMH */
    hard_stage_ctrl.itmh_type = (soc_property_get(unit, spn_ITMH_ARAD_MODE_ENABLE, 0) == 0) ? 1 : 0;
    hard_stage_ctrl.small_chunk_priority_override = 1;

    /* Find PRD port type */
    if (is_hg) 
    {
        prd_port_type = SOC_JER_PLUS_PRD_PORT_TYPE_HIGIG;
        eth_port = 1;
    } 
    else 
    {
        switch (hdr_type) 
        {
        case SOC_TMC_PORT_HEADER_TYPE_TM:
            prd_port_type = SOC_JER_PLUS_PRD_PORT_TYPE_ITMH;
            tm_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_STACKING:
            prd_port_type = SOC_JER_PLUS_PRD_PORT_TYPE_FTMH;
            tm_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED:
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED_PP:
            prd_port_type = SOC_JER_PLUS_PRD_PORT_TYPE_PTCH1;
            eth_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED_2:
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED_2_PP:
            prd_port_type = SOC_JER_PLUS_PRD_PORT_TYPE_PTCH2;
            eth_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_ETH:
            prd_port_type = SOC_JER_PLUS_PRD_PORT_TYPE_ETH;
            prd_config.never_drop_control_frames = 1;    
            eth_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_TDM:
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Priority Drop feature is not supported for TDM ports\n")));
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unsupported header type %d\n"), hdr_type));
        }
    }

    if (enable_mode == socJerNifPrdDisable) /*Disable hard stage*/
    {
        /** disable Hard stage */
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_enable_get(unit, port, &curr_en_mask));
        curr_en_mask &= ~en_mask;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_enable_set(unit, port, curr_en_mask));

        /** Disable Soft Stage */
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_enable_set(unit,port, 0, 0));
    } 
    else 
    {
        /** Set Hard stage control */
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_control_set(unit, port, &hard_stage_ctrl));

        /** Set Hard stage properties */
        /* if port extender mode - outer tag size = 8B, else - outer tag size = 4B. reg val: 0->4B, 1->6B, 2->8B */
        hard_stage_properties.outer_tag_size = (flags & SOC_JER_NIF_PRD_F_PORT_EXTERNDER) ? 2 : 0;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &hard_stage_properties));

        /** Set PRD configurations */
        prd_config.never_drop_tdm_packets = 1;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_config_set(unit, port, &prd_config));

        /**Set PRD Hard stage port type */ 
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_port_type_set(unit, port, prd_port_type));

        /** Enable Soft Stage - if required */
        en_soft_stage_eth = ((enable_mode == socJerNifPrdEnableHardAndSoftStage) && (eth_port)) ? 1 : 0;
        en_soft_stage_tm  = ((enable_mode == socJerNifPrdEnableHardAndSoftStage) && (tm_port))  ? 1 : 0;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_enable_set(unit, port, en_soft_stage_eth, en_soft_stage_tm));

        /** Enable Hard Stage */
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_enable_get(unit, port, &curr_en_mask));
        curr_en_mask |= en_mask;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_enable_set(unit, port, curr_en_mask));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_only_port_prd_enable_set(int unit, soc_port_t port, uint32 flags, int enable)
{
    int rv, phy, first_phy=-1;
    soc_reg_above_64_val_t reg_prd_config;
    uint32 reg_val, qmlf_index, field, prd_mode=0, is_hg, en_bit, en_mask=0, nof_lanes_nbi;
    SOC_TMC_PORT_HEADER_TYPE hdr_type;
    soc_pbmp_t nif_ports, phys;
#ifdef BCM_QUX_SUPPORT
    uint64 reg64_val, field_64, mask_64;
    uint32 mask_high = 0, mask_low = 0;
#endif
    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.get(unit, port, &hdr_type);
    SOCDNX_IF_ERR_EXIT(rv);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_hg_get(unit, port, &is_hg));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));
#ifdef BCM_QUX_SUPPORT
    COMPILER_64_ZERO(mask_64);
    COMPILER_64_ZERO(reg64_val);
    if (SOC_IS_QUX(unit)) {
        SOC_PBMP_ITER(phys, phy) {
            if (first_phy < 0){
                first_phy = (phy-1);
            }
            if ((phy % nof_lanes_nbi) > SAL_UINT32_NOF_BITS) {
                 mask_high |= 1 << ( (phy-1-SAL_UINT32_NOF_BITS) % nof_lanes_nbi);
            } else {
                 mask_low |= 1 << ( (phy-1) % nof_lanes_nbi);
            }
         }
         COMPILER_64_SET(mask_64, mask_high, mask_low);
    } else
#endif
    {
        SOC_PBMP_ITER(phys, phy) {
            if (first_phy < 0)
            {
                first_phy = (phy-1);
            }
            en_bit = 1 << ( (phy-1) % nof_lanes_nbi);
            en_mask |= en_bit;
        }
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;

    if (enable) /* Enable PRD */
    {
        if (is_hg)
        {
            prd_mode = JER_SOC_PRD_MODE_HIGIG;
        }
        else
        {
            switch(hdr_type)
            {
            case SOC_TMC_PORT_HEADER_TYPE_ETH:
                prd_mode = JER_SOC_PRD_MODE_VLAN;
                break;

            case SOC_TMC_PORT_HEADER_TYPE_TM:
                prd_mode = JER_SOC_PRD_MODE_ITMH;
                break;

            default:
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unsupported header type\n")));
            }
        }
#ifdef BCM_QUX_SUPPORT
        if (SOC_IS_QUX(unit)) {
            SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_ENr(unit, &reg64_val));
            field_64 =  soc_reg64_field_get(unit, NIF_PRD_ENr, reg64_val, PRD_ENf);
            COMPILER_64_OR(field_64, mask_64);
            soc_reg64_field_set(unit, NIF_PRD_ENr, &reg64_val, PRD_ENf, field_64);
            SOCDNX_IF_ERR_EXIT(WRITE_NIF_PRD_ENr(unit, reg64_val));

            SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
            soc_reg_above_64_field32_set(unit, NIF_PRD_CONFIG_QMLFr, reg_prd_config, PRD_PKT_TYPE_QMLF_Nf, prd_mode);
            SOCDNX_IF_ERR_EXIT(WRITE_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
        } else
#endif
        {
            if (first_phy < nof_lanes_nbi)  /* NBIH */
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_ENr(unit, &reg_val));
                field =  soc_reg_field_get(unit, NBIH_PRD_ENr, reg_val, PRD_ENf);
                soc_reg_field_set(unit, NBIH_PRD_ENr, &reg_val, PRD_ENf, field | en_mask);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_PRD_ENr(unit, reg_val));

                SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
                soc_reg_above_64_field32_set(unit, NBIH_PRD_CONFIGr, reg_prd_config, PRD_PKT_TYPE_QMLF_Nf, prd_mode);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));

            }
            else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_ENr(unit, 0, &reg_val));
                field =  soc_reg_field_get(unit, NBIL_PRD_ENr, reg_val, PRD_ENf);
                soc_reg_field_set(unit, NBIL_PRD_ENr, &reg_val, PRD_ENf, field | en_mask);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_ENr(unit, 0, reg_val));

                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
                soc_reg_above_64_field32_set(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PKT_TYPE_QMLF_Nf, prd_mode);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
            }
            else /* NBIL1 */
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_ENr(unit, 1, &reg_val));
                field =  soc_reg_field_get(unit, NBIL_PRD_ENr, reg_val, PRD_ENf);
                soc_reg_field_set(unit, NBIL_PRD_ENr, &reg_val, PRD_ENf, field | en_mask);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_ENr(unit, 1, reg_val));

                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
                soc_reg_above_64_field32_set(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PKT_TYPE_QMLF_Nf, prd_mode);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
            }
        }
    }
    else /* Disable PRD */
    {
#ifdef BCM_QUX_SUPPORT
        if (SOC_IS_QUX(unit)) {
            SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_ENr(unit, &reg64_val));
            field_64 =  soc_reg64_field_get(unit, NIF_PRD_ENr, reg64_val, PRD_ENf);
            COMPILER_64_NOT(mask_64);
            COMPILER_64_AND(field_64, mask_64);
            soc_reg64_field_set(unit, NIF_PRD_ENr, &reg64_val, PRD_ENf, field_64);
            SOCDNX_IF_ERR_EXIT(WRITE_NIF_PRD_ENr(unit, reg64_val));
        } else
#endif
        {
            if (first_phy < nof_lanes_nbi)  /* NBIH */
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_ENr(unit, &reg_val));
                field =  soc_reg_field_get(unit, NBIH_PRD_ENr, reg_val, PRD_ENf);
                soc_reg_field_set(unit, NBIH_PRD_ENr, &reg_val, PRD_ENf, field & ~en_mask);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_PRD_ENr(unit, reg_val));
            }
            else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_ENr(unit, 0, &reg_val));
                field =  soc_reg_field_get(unit, NBIL_PRD_ENr, reg_val, PRD_ENf);
                soc_reg_field_set(unit, NBIL_PRD_ENr, &reg_val, PRD_ENf, field & ~en_mask);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_ENr(unit, 0, reg_val));
            }
            else /* NBIL1 */
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_ENr(unit, 1, &reg_val));
                field =  soc_reg_field_get(unit, NBIL_PRD_ENr, reg_val, PRD_ENf);
                soc_reg_field_set(unit, NBIL_PRD_ENr, &reg_val, PRD_ENf, field & ~en_mask);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_ENr(unit, 1, reg_val));
            }
        }
    }
    exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_enable_set(int unit, soc_port_t port, uint32 flags, int enable_mode) 
{
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_enable_set(unit, port, flags, enable_mode));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_enable_set(unit, port, flags, enable_mode));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_plus_port_prd_enable_get(int unit, soc_port_t port, uint32 flags, int *enable_mode)
{
    int phy, tm_port = 0, eth_port = 0, en_soft_stage_tm, en_soft_stage_eth;
    uint32 is_hg, en_bit, en_mask=0, curr_en_mask, nof_lanes_nbi, offset, num_lanes, en_hard_stage;
    SOC_TMC_PORT_HEADER_TYPE hdr_type;
    soc_pbmp_t nif_ports, phys;
    int nof_segments;
    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.header_type_out.get(unit, port, &hdr_type));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_is_hg_get(unit, port, &is_hg));
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    /* Calculate PRD enable val acording to the port physical phys.
     * for ILKN there is one bit per ILKN IF */
    SOC_PBMP_ITER(phys, phy) 
    {
        en_bit = 1 << ( (phy-1) % nof_lanes_nbi);
        en_mask |= en_bit;
    }

    if (IS_IL_PORT(unit, port)) 
    {
        /* ILKN PRD enable bits are 25:24 for HRFs 0 and 1  - override the en_mask */
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
        nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];
        en_mask = (offset & 1) ? 0x2000000 : 0x1000000;
        if (nof_segments == 4) {
            /* if ILKN0/2/4 has more than 12 lanes, both bits of ILKN0/1 should be enabled */
            en_mask |= 0x2000000;
        }
    }

    /* Find PRD port type */
    if (is_hg) 
    {
        eth_port = 1;
    } 
    else 
    {
        switch (hdr_type) {
        case SOC_TMC_PORT_HEADER_TYPE_TM:
        case SOC_TMC_PORT_HEADER_TYPE_STACKING:
            tm_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED:
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED_PP:
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED_2:
        case SOC_TMC_PORT_HEADER_TYPE_INJECTED_2_PP:
        case SOC_TMC_PORT_HEADER_TYPE_ETH:
            eth_port = 1;
            break;
        case SOC_TMC_PORT_HEADER_TYPE_TDM:
            /* PRD is not supported for TDM ports */
            *enable_mode = 0;
            break;
        default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unsupported header type %d\n"), hdr_type));
        }
    }

    /** Get enable Hard Stage */
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_enable_get(unit, port, &curr_en_mask));
    en_hard_stage = curr_en_mask & en_mask;

    if (en_hard_stage) 
    {
        /** Get enable Soft Stage */
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_enable_get(unit, port, &en_soft_stage_eth, &en_soft_stage_tm));
        if ((en_soft_stage_eth && eth_port) || (en_soft_stage_tm && tm_port)) 
        {
            *enable_mode = socJerNifPrdEnableHardAndSoftStage;
        }
        else 
        {
            *enable_mode = socJerNifPrdEnableHardStage;
        }
    } 
    else 
    {
        *enable_mode = socJerNifPrdDisable;
    }

    
exit:
    SOCDNX_FUNC_RETURN;
}


STATIC
int soc_jer_only_port_prd_enable_get(int unit, soc_port_t port, uint32 flags, int *enable)
{
    int phy, first_phy=-1;
    uint32 reg_val, field=0, nof_lanes_nbi, en_bit, en_mask=0;
    soc_pbmp_t nif_ports, phys;
#ifdef BCM_QUX_SUPPORT
    uint64 reg64_val, field_64, mask_64;
    uint32 mask_high = 0, mask_low = 0;
#endif

    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));
#ifdef BCM_QUX_SUPPORT
    COMPILER_64_ZERO(mask_64);
    COMPILER_64_ZERO(reg64_val);
    if (SOC_IS_QUX(unit)) {
        SOC_PBMP_ITER(phys, phy) {
            if (first_phy < 0){
                first_phy = (phy-1);
            }
            if ((phy % nof_lanes_nbi) > SAL_UINT32_NOF_BITS) {
                mask_high |= 1 << ( (phy-1-SAL_UINT32_NOF_BITS) % nof_lanes_nbi);
            } else {
                mask_low |= 1 << ( (phy-1) % nof_lanes_nbi);
            }
        }
        COMPILER_64_SET(mask_64, mask_high, mask_low);
        SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_ENr(unit, &reg64_val));
        field_64 =  soc_reg64_field_get(unit, NIF_PRD_ENr, reg64_val, PRD_ENf);
        COMPILER_64_AND(field_64, mask_64);
        *enable = COMPILER_64_IS_ZERO(field_64) ? 0 : 1;
    } else
#endif
    {
        SOC_PBMP_ITER(phys, phy) {
            if (first_phy < 0)
            {
                first_phy = (phy-1);
            }
            en_bit = 1 << ( (phy-1) % nof_lanes_nbi);
            en_mask |= en_bit;
        }

        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_ENr(unit, &reg_val));
            field =  soc_reg_field_get(unit, NBIH_PRD_ENr, reg_val, PRD_ENf);
        }
        else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_ENr(unit, 0, &reg_val));
            field =  soc_reg_field_get(unit, NBIL_PRD_ENr, reg_val, PRD_ENf);
        }
        else /* NBIL1 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_ENr(unit, 1, &reg_val));
            field =  soc_reg_field_get(unit, NBIL_PRD_ENr, reg_val, PRD_ENf);
        }
        *enable = (field & en_mask) ? 1 : 0;
    }
exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_port_prd_enable_get(int unit, soc_port_t port, uint32 flags, int *enable_mode) {

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_enable_get(unit, port, flags, enable_mode));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_enable_get(unit, port, flags, enable_mode));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_config_set(int unit, soc_port_t port, uint32 flags, soc_dpp_port_prd_config_t *config)
{
    int phy, first_phy=-1;
    soc_reg_above_64_val_t reg_prd_config;
    uint32 qmlf_index, nof_lanes_nbi;
    uint32 untagged_pcp;
    soc_pbmp_t nif_ports, phys;

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Untagged PCP configuration is not supported on BCM88680." \
                                                              "please use bcm_cosq_ingress_port_drop_default_priority_set instead.\n")));
    }
    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;
    untagged_pcp = config->untagged_pcp;

    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
        soc_reg_above_64_field32_set(unit, NIF_PRD_CONFIG_QMLFr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf, untagged_pcp);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
            soc_reg_above_64_field32_set(unit, NBIH_PRD_CONFIGr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf, untagged_pcp);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
        }
        else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
            soc_reg_above_64_field32_set(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf, untagged_pcp);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
        }
        else /* NBIL1 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
            soc_reg_above_64_field32_set(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf, untagged_pcp);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_config_get(int unit, soc_port_t port, uint32 flags, soc_dpp_port_prd_config_t *config)
{
    int phy, first_phy=-1;
    soc_reg_above_64_val_t reg_prd_config;
    uint32 qmlf_index, nof_lanes_nbi, untagged_pcp;
    soc_pbmp_t nif_ports, phys;

    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Untagged PCP configuration is not supported on BCM88680." \
                                                              "please use bcm_cosq_ingress_port_drop_default_priority_get instead.\n")));
    }

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;

    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
        untagged_pcp = soc_reg_above_64_field32_get(unit, NIF_PRD_CONFIG_QMLFr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf);
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
            untagged_pcp = soc_reg_above_64_field32_get(unit, NBIH_PRD_CONFIGr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf);
        }
        else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
            untagged_pcp = soc_reg_above_64_field32_get(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf);
        }
        else /* NBIL1 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
            untagged_pcp = soc_reg_above_64_field32_get(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_UNTAG_PCP_QMLF_Nf);
        }
    }
    config->untagged_pcp = untagged_pcp;

exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_port_prd_threshold_set(int unit, soc_port_t port, uint32 flags, int priority, uint32 value)
{
    int phy, first_phy=-1;
    uint64 reg_val64;
    uint32 qmlf_index, nof_lanes_nbi, field, offset, num_lanes;
    soc_pbmp_t nif_ports, phys;
    soc_reg_t reg = INVALIDr;
    int nof_segments = 0;
    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    if (value > SOC_JER_PLUS_PRD_THRESHOLD_MAX_VAL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Threshold value %d is out of range."),value));
    }

    /* PRD is supported for ILKN only on JER plus,
     * but input validation was done at higher level, 
     * so here we asume that if the port is ILKN, we are on Jer plus.
     */
    if (IS_IL_PORT(unit, port)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_num_lanes_get(unit, port, &num_lanes));
        nof_segments = SOC_DPP_CONFIG(unit)->jer->nif.ilkn_nof_segments[offset];
    }

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;

    switch (priority)
    {
    case 0:
        if (IS_IL_PORT(unit, port)) 
        {
            field  = (offset & 1) ? HRF_RX_PRD_THRESHOLD_0_HRF_1f : HRF_RX_PRD_THRESHOLD_0_HRF_0f;
        } 
        else 
        {
            field = RX_PRD_THRESHOLD_0_QMLF_Nf; 
        }
        break;
    case 1:
        if (IS_IL_PORT(unit, port)) 
        {
            field  = (offset & 1) ? HRF_RX_PRD_THRESHOLD_1_HRF_1f : HRF_RX_PRD_THRESHOLD_1_HRF_0f;
        } 
        else 
        {
            field = RX_PRD_THRESHOLD_1_QMLF_Nf;
        }
        break;
    case 2:
        if (IS_IL_PORT(unit, port)) 
        {
            field  = (offset & 1) ? HRF_RX_PRD_THRESHOLD_2_HRF_1f : HRF_RX_PRD_THRESHOLD_2_HRF_0f;
        } 
        else 
        {
            field = RX_PRD_THRESHOLD_2_QMLF_Nf;
        }
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Priority %d is unsupported.\n")));
    }

    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, qmlf_index, &reg_val64));
        soc_reg64_field32_set(unit, NIF_RX_MLF_PRD_THRESHOLDS_CONFIGr, &reg_val64, field, value);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, qmlf_index, reg_val64));
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            if (IS_IL_PORT(unit, port)) 
            {
                reg = (offset & 1) ? NBIH_HRF_RX_PRD_THRESHOLDS_CONFIG_1r : NBIH_HRF_RX_PRD_THRESHOLDS_CONFIG_0r;
                SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                soc_reg64_field32_set(unit, reg, &reg_val64, field, value);
                SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, REG_PORT_ANY, 0, reg_val64));
                if (nof_segments == 4) 
                {
                    /* if ILKN port has more than 12 lanes - 
                     * than only one ILKN is active, and we need to configure both ILKN0 and ILKN1 */
                    SOCDNX_IF_ERR_EXIT(READ_NBIH_HRF_RX_PRD_THRESHOLDS_CONFIG_1r(unit, &reg_val64));
                    soc_reg64_field32_set(unit, reg, &reg_val64, HRF_RX_PRD_THRESHOLD_0_HRF_1f, value);
                    SOCDNX_IF_ERR_EXIT(WRITE_NBIH_HRF_RX_PRD_THRESHOLDS_CONFIG_1r(unit, reg_val64));
                }
            } 
            else 
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, qmlf_index, &reg_val64)); 
                soc_reg64_field32_set(unit, NBIH_RX_MLF_PRD_THRESHOLDS_CONFIGr, &reg_val64, field, value);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIH_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, qmlf_index, reg_val64));
            }

        }
        else if (first_phy < 2*nof_lanes_nbi)  /* NBIL0 */
        {
            if (IS_IL_PORT(unit, port)) 
            {
                reg = (offset & 1) ? NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r : NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_0r;
                SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, 0, 0, &reg_val64));
                soc_reg64_field32_set(unit, reg, &reg_val64, field, value);
                SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, 0, 0, reg_val64));
                if (nof_segments == 4) 
                {
                    /* if ILKN port has more than 12 lanes - 
                     * than only one ILKN is active, and we need to configure both ILKN0 and ILKN1 */
                    SOCDNX_IF_ERR_EXIT(READ_NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r(unit, 0, &reg_val64));
                    soc_reg64_field32_set(unit, reg, &reg_val64, HRF_RX_PRD_THRESHOLD_0_HRF_1f, value);
                    SOCDNX_IF_ERR_EXIT(WRITE_NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r(unit, 0, reg_val64));
                }
            } 
            else 
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, 0, qmlf_index, &reg_val64));
                soc_reg64_field32_set(unit, NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr, &reg_val64, field, value);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, 0, qmlf_index, reg_val64));
            }
        }
        else /* NBIL1 */
        {
            if (IS_IL_PORT(unit, port)) 
            {
                reg = (offset & 1) ? NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r : NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_0r;
                SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, 1, 0, &reg_val64));
                soc_reg64_field32_set(unit, reg, &reg_val64, field, value);
                SOCDNX_IF_ERR_EXIT(soc_reg64_set(unit, reg, 1, 0, reg_val64));
                if (nof_segments == 4) 
                {
                    /* if ILKN port has more than 12 lanes - 
                     * than only one ILKN is active, and we need to configure both ILKN0 and ILKN1 */
                    SOCDNX_IF_ERR_EXIT(READ_NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r(unit, 1, &reg_val64));
                    soc_reg64_field32_set(unit, reg, &reg_val64, HRF_RX_PRD_THRESHOLD_0_HRF_1f, value);
                    SOCDNX_IF_ERR_EXIT(WRITE_NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r(unit, 1, reg_val64));
                }
            } 
            else 
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, 1, qmlf_index, &reg_val64));
                soc_reg64_field32_set(unit, NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr, &reg_val64, field, value);
                SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, 1, qmlf_index, reg_val64));
            }
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_threshold_get(int unit, soc_port_t port, uint32 flags, int priority, uint32 *value)
{
    int phy, first_phy=-1;
    uint64 reg_val64;
    uint32 qmlf_index, nof_lanes_nbi, field, th_value, offset;
    soc_reg_t reg;
    soc_pbmp_t nif_ports, phys;

    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    /* PRD is supported for ILKN only on JER plus,
     * but input validation was done at higher level, 
     * so here we asume that if the port is ILKN, we are on Jer plus.
     */
    if (IS_IL_PORT(unit, port)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
    }

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;

    switch (priority)
    {
    case 0:
        if (IS_IL_PORT(unit, port)) 
        {
            field  = (offset & 1) ? HRF_RX_PRD_THRESHOLD_0_HRF_1f : HRF_RX_PRD_THRESHOLD_0_HRF_0f;
        } 
        else 
        {
            field = RX_PRD_THRESHOLD_0_QMLF_Nf; 
        }
        break;
    case 1:
        if (IS_IL_PORT(unit, port)) 
        {
            field  = (offset & 1) ? HRF_RX_PRD_THRESHOLD_1_HRF_1f : HRF_RX_PRD_THRESHOLD_1_HRF_0f;
        } 
        else 
        {
            field = RX_PRD_THRESHOLD_1_QMLF_Nf; 
        }
        break;
    case 2:
        if (IS_IL_PORT(unit, port)) 
        {
            field  = (offset & 1) ? HRF_RX_PRD_THRESHOLD_2_HRF_1f : HRF_RX_PRD_THRESHOLD_2_HRF_0f;
        } 
        else 
        {
            field = RX_PRD_THRESHOLD_2_QMLF_Nf; 
        }
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unsupported priority\n")));
    }

    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, qmlf_index, &reg_val64));
        th_value = soc_reg64_field32_get(unit, NIF_RX_MLF_PRD_THRESHOLDS_CONFIGr, reg_val64, field);
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            if (IS_IL_PORT(unit, port)) 
            {
                reg = (offset & 1) ? NBIH_HRF_RX_PRD_THRESHOLDS_CONFIG_1r : NBIH_HRF_RX_PRD_THRESHOLDS_CONFIG_0r;
                SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, REG_PORT_ANY, 0, &reg_val64));
                th_value = soc_reg64_field32_get(unit, reg, reg_val64, field);
            } 
            else 
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, qmlf_index, &reg_val64));
                th_value = soc_reg64_field32_get(unit, NBIH_RX_MLF_PRD_THRESHOLDS_CONFIGr, reg_val64, field);
            }
    
        }
        else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
        {
            if (IS_IL_PORT(unit, port)) 
            {
                reg = (offset & 1) ? NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r : NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_0r;
                SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, 0, 0, &reg_val64));
                th_value = soc_reg64_field32_get(unit, reg, reg_val64, field);
            } 
            else 
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, 0, qmlf_index, &reg_val64));
                th_value = soc_reg64_field32_get(unit, NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr, reg_val64, field);
            }
        }
        else /* NBIL1 */
        {
            if (IS_IL_PORT(unit, port)) 
            {
                reg = (offset & 1) ? NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_1r : NBIL_HRF_RX_PRD_THRESHOLDS_CONFIG_0r;
                SOCDNX_IF_ERR_EXIT(soc_reg64_get(unit, reg, 1, 0, &reg_val64));
                th_value = soc_reg64_field32_get(unit, reg, reg_val64, field);
            } 
            else 
            {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr(unit, 1, qmlf_index, &reg_val64));
                th_value = soc_reg64_field32_get(unit, NBIL_RX_MLF_PRD_THRESHOLDS_CONFIGr, reg_val64, field);
            }
        }
    }
    *value = th_value;
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_plus_port_prd_map_set(int unit, soc_port_t port, uint32 flags, soc_dpp_prd_map_t map, uint32 key, int priority)
{
    uint32 prio_map;
    soc_reg_above_64_val_t reg_above64_prio_map;
    int key_tbl_index;
    SOCDNX_INIT_FUNC_DEFS;

    key_tbl_index = key * JER_SOC_PRD_MAP_BITS_PER_PRIORITY;

    /* Call the correct HW access function according to the map type*/
    switch (map) {
    case socDppPrdTmTcDpPriorityTable:
        if (key > SOC_JER_PLUS_PRD_TM_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_tm_tc_dp_get(unit, port, reg_above64_prio_map)); 
        SOC_REG_ABOVE_64_RANGE_COPY(reg_above64_prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_tm_tc_dp_set(unit, port, reg_above64_prio_map));
        break;
    case socDppPrdIpDscpToPriorityTable:
        if (key > SOC_JER_PLUS_PRD_IP_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_ip_dscp_get(unit, port, reg_above64_prio_map));
        SOC_REG_ABOVE_64_RANGE_COPY(reg_above64_prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_ip_dscp_set(unit, port, reg_above64_prio_map));
        break;
    case socDppPrdEthPcpDeiToPriorityTable:
        if (key > SOC_JER_PLUS_PRD_ETH_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_eth_pcp_dei_get(unit, port, &prio_map));
        SHR_BITCOPY_RANGE(&prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_eth_pcp_dei_set(unit, port, prio_map));
        break;
    case socDppPrdMplsExpToPriorityTable:
        if (key > SOC_JER_PLUS_PRD_MPLS_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_mpls_exp_get(unit, port, &prio_map));
        SHR_BITCOPY_RANGE(&prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_mpls_exp_set(unit, port, prio_map));
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Priority drop map %d is invalid"), map)); 
        break;
    }

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_only_port_prd_map_set(int unit, soc_port_t port, uint32 flags, soc_dpp_prd_map_t map, uint32 key, int priority)
{
    int phy, first_phy=-1;
    uint32 qmlf_index, nof_lanes_nbi;
    soc_reg_above_64_val_t reg_prd_config;
    soc_pbmp_t nif_ports, phys;
    int key_tbl_index;
    soc_reg_above_64_val_t prio_map;
    SOCDNX_INIT_FUNC_DEFS;

    switch (map) {
    case socDppPrdEthPcpDeiToPriorityTable:
        break;
    case socDppPrdTmTcDpPriorityTable:
        if (key > SOC_JER_PLUS_PRD_TM_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Priority drop map %d is invalid"), map)); 
        break;
    }

    if ((priority < SOC_DPP_COSQ_PORT_PRIORITY_MIN) || (priority > SOC_DPP_COSQ_PORT_PRIORITY_MAX)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Priority %d is invalid"), priority));
    }

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM; /*quad index inside PM*/

    key_tbl_index = key * JER_SOC_PRD_MAP_BITS_PER_PRIORITY;

    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
        soc_reg_above_64_field_get(unit, NIF_PRD_CONFIG_QMLFr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
        SOC_REG_ABOVE_64_RANGE_COPY(prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        soc_reg_above_64_field_set(unit, NIF_PRD_CONFIG_QMLFr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
            soc_reg_above_64_field_get(unit, NBIH_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
            SOC_REG_ABOVE_64_RANGE_COPY(prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
            soc_reg_above_64_field_set(unit, NBIH_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
        }
        else if (first_phy < 2*nof_lanes_nbi)  /* NBIL0 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
            soc_reg_above_64_field_get(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
            SOC_REG_ABOVE_64_RANGE_COPY(prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
            soc_reg_above_64_field_set(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
        }
        else /* NBIL1 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
            soc_reg_above_64_field_get(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
            SOC_REG_ABOVE_64_RANGE_COPY(prio_map, key_tbl_index, (uint32*)&priority, 0, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
            soc_reg_above_64_field_set(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
            SOCDNX_IF_ERR_EXIT(WRITE_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_map_set(int unit, soc_port_t port, uint32 flags, soc_dpp_prd_map_t map, uint32 key, int priority)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_map_set(unit, port, flags, map, key, priority));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_map_set(unit, port, flags, map, key, priority));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_plus_port_prd_map_get(int unit, soc_port_t port, uint32 flags, soc_dpp_prd_map_t map, uint32 key, int *priority)
{
    uint32 prio_map;
    soc_reg_above_64_val_t reg_above64_prio_map;
    int key_tbl_index;
    SOCDNX_INIT_FUNC_DEFS;

    key_tbl_index = key * JER_SOC_PRD_MAP_BITS_PER_PRIORITY;

    /* Call the correct HW access function according to the map type*/
    switch (map) {
    case socDppPrdTmTcDpPriorityTable:
        if (key > SOC_JER_PLUS_PRD_TM_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_tm_tc_dp_get(unit, port, reg_above64_prio_map));
        SOC_REG_ABOVE_64_RANGE_COPY((uint32*)priority, 0, reg_above64_prio_map, key_tbl_index, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        break;
    case socDppPrdIpDscpToPriorityTable:
        if (key > SOC_JER_PLUS_PRD_IP_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_ip_dscp_get(unit, port, reg_above64_prio_map));
        SOC_REG_ABOVE_64_RANGE_COPY((uint32*)priority, 0, reg_above64_prio_map, key_tbl_index, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        break;
    case socDppPrdEthPcpDeiToPriorityTable:
        if (key > SOC_JER_PLUS_PRD_ETH_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_eth_pcp_dei_get(unit, port, &prio_map));
        SHR_BITCOPY_RANGE((uint32*)priority, 0, &prio_map, key_tbl_index, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        break;
    case socDppPrdMplsExpToPriorityTable:
        if (key > SOC_JER_PLUS_PRD_MPLS_MAP_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Key %d is invalid."), key));
        }
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_mpls_exp_get(unit, port, &prio_map));
        SHR_BITCOPY_RANGE((uint32*)priority, 0, &prio_map, key_tbl_index, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Priority drop map %d is invalid"), map)); 
        break;
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_only_port_prd_map_get(int unit, soc_port_t port, uint32 flags, soc_dpp_prd_map_t map, uint32 key, int *priority)
{
    int phy, first_phy=-1;
    uint32 qmlf_index, nof_lanes_nbi;
    soc_reg_above_64_val_t reg_prd_config;
    soc_pbmp_t nif_ports, phys;
    int key_tbl_index; 
    soc_reg_above_64_val_t prio_map;
    SOCDNX_INIT_FUNC_DEFS;

    *priority = 0;

    switch (map) {
    case socDppPrdEthPcpDeiToPriorityTable:
    case socDppPrdTmTcDpPriorityTable:
        break;
    default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Priority drop map %d is invalid"), map)); 
        break;
    }

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM; /*quad index inside PM*/

    if (SOC_IS_QUX(unit))
    {
        SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_CONFIG_QMLFr(unit, qmlf_index, reg_prd_config));
        soc_reg_above_64_field_get(unit, NIF_PRD_CONFIG_QMLFr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_CONFIGr(unit, qmlf_index, reg_prd_config));
            soc_reg_above_64_field_get(unit, NBIH_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
        }
        else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 0, qmlf_index, reg_prd_config));
            soc_reg_above_64_field_get(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
        }
        else /* NBIL1 */
        {
            SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_CONFIGr(unit, 1, qmlf_index, reg_prd_config));
            soc_reg_above_64_field_get(unit, NBIL_PRD_CONFIGr, reg_prd_config, PRD_PRIO_MAP_QMLF_Nf, prio_map);
        }
    }
    key_tbl_index = key * JER_SOC_PRD_MAP_BITS_PER_PRIORITY;

    SOC_REG_ABOVE_64_RANGE_COPY((uint32*)priority, 0, prio_map, key_tbl_index, JER_SOC_PRD_MAP_BITS_PER_PRIORITY);

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_map_get(int unit, soc_port_t port, uint32 flags, soc_dpp_prd_map_t map, uint32 key, int *priority)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_map_get(unit, port, flags, map, key, priority));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_map_get(unit, port, flags, map, key, priority));
    }
exit:
    SOCDNX_FUNC_RETURN;
}


STATIC
int soc_jer_only_port_prd_drop_count_get(int unit, soc_port_t port, uint32 *count)
{
    int phy, first_phy=-1;
    uint32 reg_val;
    uint32 nof_lanes_nbi, lane;
    soc_pbmp_t nif_ports, phys;
    SOCDNX_INIT_FUNC_DEFS;

    nof_lanes_nbi = SOC_DPP_DEFS_GET(unit, nof_lanes_per_nbi);
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &nif_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &nif_ports, &phys));

    SOC_PBMP_ITER(phys, phy) {
        first_phy = (phy-1);
        break;
    }

    /*qmlf_index = (first_phy % nof_lanes_nbi) / NUM_OF_LANES_IN_PM;*/

    if (SOC_IS_QUX(unit))
    {
        lane = first_phy;
        SOCDNX_IF_ERR_EXIT(READ_NIF_PRD_PKT_DROP_CNT_PORTr(unit, lane, count));
    } else
    {
        if (first_phy < nof_lanes_nbi)  /* NBIH */
        {
            lane = first_phy;
            SOCDNX_IF_ERR_EXIT(READ_NBIH_PRD_PKT_DROP_CNT_PORTr_REG32(unit, lane, count));
        }
        else if ( first_phy < 2*nof_lanes_nbi )  /* NBIL0 */
        {
            lane = first_phy - nof_lanes_nbi;
            if (SOC_IS_QAX(unit)) {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_PKT_DROP_CNT_PORTr_REG32(unit, 0, lane, &reg_val));
                *count = soc_reg_field_get(unit, NBIL_PRD_PKT_DROP_CNT_PORTr, reg_val, PRD_PKT_DROP_CNT_PORTf);
            }
            else {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_PRD_PKT_DROP_CNTr(unit, 0, lane, &reg_val));
                *count = soc_reg_field_get(unit, NBIL_RX_PRD_PKT_DROP_CNTr, reg_val, PRD_PKT_DROP_CNT_PORT_Nf);
            }
        }
        else /* NBIL1 */
        {
            lane = first_phy - 2*nof_lanes_nbi;
            if (SOC_IS_QAX(unit)) {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_PRD_PKT_DROP_CNT_PORTr_REG32(unit, 1, lane, &reg_val));
            *count = soc_reg_field_get(unit, NBIL_PRD_PKT_DROP_CNT_PORTr, reg_val, PRD_PKT_DROP_CNT_PORTf);
            }
            else {
                SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_PRD_PKT_DROP_CNTr(unit, 1, lane, &reg_val));
                *count = soc_reg_field_get(unit, NBIL_RX_PRD_PKT_DROP_CNTr, reg_val, PRD_PKT_DROP_CNT_PORT_Nf);
            }
        }
    }
exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_port_prd_drop_count_get(int unit, soc_port_t port, uint64 *count)
{
    uint32 cnt32bit;
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_drop_count_get(unit, port, count));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_drop_count_get(unit, port, &cnt32bit));
        COMPILER_64_SET(*count, 0, cnt32bit);
    }
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_plus_port_prd_tpid_set(int unit, soc_port_t port, uint32 flags, int index, uint32 tpid)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_tpid_set(unit, port, index, tpid));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_only_port_prd_tpid_set(int unit, uint32 flags, int index, uint32 tpid)
{
    uint32 reg_val;
    soc_field_t field;
    SOCDNX_INIT_FUNC_DEFS;

    field = (index == 0 ? PRD_VLAN_ETHERTYPE_1f : PRD_VLAN_ETHERTYPE_2f);

    if (SOC_IS_QUX(unit))
    {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_PRD_VLAN_ETHERTYPEr(unit, &reg_val));
        soc_reg_field_set(unit, NIF_RX_PRD_VLAN_ETHERTYPEr, &reg_val, field, (uint32)tpid);
        SOCDNX_IF_ERR_EXIT(WRITE_NIF_RX_PRD_VLAN_ETHERTYPEr(unit, reg_val));
    } else
    {
        SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_PRD_VLAN_ETHERTYPEr(unit, &reg_val));
        soc_reg_field_set(unit, NBIH_RX_PRD_VLAN_ETHERTYPEr, &reg_val, field, (uint32)tpid);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIH_RX_PRD_VLAN_ETHERTYPEr(unit, reg_val));

    SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_PRD_VLAN_ETHERTYPEr(unit, 0, &reg_val));
    soc_reg_field_set(unit, NBIL_RX_PRD_VLAN_ETHERTYPEr, &reg_val, field, tpid);
    SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_PRD_VLAN_ETHERTYPEr(unit, 0, reg_val));

        SOCDNX_IF_ERR_EXIT(READ_NBIL_RX_PRD_VLAN_ETHERTYPEr(unit, 1, &reg_val));
        soc_reg_field_set(unit, NBIL_RX_PRD_VLAN_ETHERTYPEr, &reg_val, field, (uint32)tpid);
        SOCDNX_IF_ERR_EXIT(WRITE_NBIL_RX_PRD_VLAN_ETHERTYPEr(unit, 1, reg_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_tpid_set(int unit, soc_port_t port, uint32 flags, int index, uint32 tpid)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (tpid > SOC_JER_PLUS_PRD_TPID_MAX_VAL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("TPID %d is invalid"), tpid)); 
    }

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_tpid_set(unit, port, flags, index, tpid));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_tpid_set(unit, flags, index, tpid));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_plus_port_prd_tpid_get(int unit, soc_port_t port, uint32 flags, int index, uint32 *tpid)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_tpid_get(unit, port, index, tpid));

exit:
    SOCDNX_FUNC_RETURN;
}

STATIC
int soc_jer_only_port_prd_tpid_get(int unit, uint32 flags, int index, uint32 *tpid)
{
    uint32 reg_val, field_val;
    soc_field_t field;
    SOCDNX_INIT_FUNC_DEFS;

    field = (index == 0 ? PRD_VLAN_ETHERTYPE_1f : PRD_VLAN_ETHERTYPE_2f);

    if (SOC_IS_QUX(unit)) {
        SOCDNX_IF_ERR_EXIT(READ_NIF_RX_PRD_VLAN_ETHERTYPEr(unit, &reg_val));
        field_val = soc_reg_field_get(unit, NIF_RX_PRD_VLAN_ETHERTYPEr, reg_val, field);
    } else {
        SOCDNX_IF_ERR_EXIT(READ_NBIH_RX_PRD_VLAN_ETHERTYPEr(unit, &reg_val));
        field_val = soc_reg_field_get(unit, NBIH_RX_PRD_VLAN_ETHERTYPEr, reg_val, field);
    }
    *tpid = (uint16)field_val;

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_port_prd_tpid_get(int unit, soc_port_t port, uint32 flags, int index, uint32 *tpid)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (SOC_IS_JERICHO_PLUS_ONLY(unit)) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_tpid_get(unit, port, flags, index, tpid));
    } 
    else 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_only_port_prd_tpid_get(unit, flags, index, tpid));
    }
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_ip_dscp_set(int unit, soc_port_t port, uint32 flags, uint32 ip_dscp_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    properties.trust_ip_sdcp = ip_dscp_ignore ? 0 : 1;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &properties));
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_ip_dscp_get(int unit, soc_port_t port, uint32 flags, uint32 *ip_dscp_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    *ip_dscp_ignore = properties.trust_ip_sdcp ? 0 : 1;

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_mpls_exp_set(int unit, soc_port_t port, uint32 flags, uint32 mpls_exp_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    properties.trust_mpls_exp = mpls_exp_ignore ? 0 : 1;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &properties));
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_mpls_exp_get(int unit, soc_port_t port, uint32 flags, uint32 *mpls_exp_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    *mpls_exp_ignore = properties.trust_mpls_exp ? 0 : 1;

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_inner_tag_set(int unit, soc_port_t port, uint32 flags, uint32 inner_tag_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    properties.trust_inner_eth_tag = inner_tag_ignore ? 0 : 1;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &properties));
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_inner_tag_get(int unit, soc_port_t port, uint32 flags, uint32 *inner_tag_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    *inner_tag_ignore = properties.trust_inner_eth_tag ? 0 : 1;

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_outer_tag_set(int unit, soc_port_t port, uint32 flags, uint32 outer_tag_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    properties.trust_outer_eth_tag = outer_tag_ignore ? 0 : 1;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &properties));
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_ignore_outer_tag_get(int unit, soc_port_t port, uint32 flags, uint32 *outer_tag_ignore)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    *outer_tag_ignore = properties.trust_outer_eth_tag ? 0 : 1;

exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_plus_prd_default_priority_set(int unit, soc_port_t port, uint32 flags, uint32 default_priority)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    properties.default_priority = default_priority;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &properties));
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_default_priority_get(int unit, soc_port_t port, uint32 flags, uint32 *default_priority)
{
    soc_jer_nif_prd_hard_stage_properties_t properties;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_get(unit, port, &properties));
    *default_priority = properties.default_priority;

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_custom_ether_type_set(int unit, soc_port_t port, uint32 flags, uint32 ether_type_code, uint32 ether_type_val)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (ether_type_code < SOC_JER_PLUS_PRD_CONFIG_ETHER_CODE_MIN || ether_type_code > SOC_JER_PLUS_PRD_CONFIG_ETHER_CODE_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: Eth code %d is invalid. Valid eth code values are 1-6"), unit ,port, ether_type_code));
    }

    if (ether_type_val > SOC_JER_PLUS_PRD_CONFIG_ETHER_TYPE_MAX_VAL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: Eth type value %d is invalid. Valid eth type values are up to 0x%x"), 
                                           unit ,port, ether_type_val, SOC_JER_PLUS_PRD_CONFIG_ETHER_TYPE_MAX_VAL));
    }

    /* There are 15 ether types in the parser. only 7 of them are configurable to the user. 
     * each of the ether types get an ether code. the configurable ether codes are 1-7.
     * the name of each ether code in the register is 0-6, so we call with (ether_type_code - 1).                                                                                .
     */ 
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_ether_type_set(unit, port, ether_type_code - 1, ether_type_val));

    /* ether code 7 must be reserved for the TM soft stage.
     * we reserve it by configuring ether codes 6 and 7 to the same ether type,
     * so when a matching ether type comes, it will always get ether code 6. */
    if (ether_type_code == 6) 
    {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_ether_type_set(unit, port, 6, ether_type_val));
    }

exit:
    SOCDNX_FUNC_RETURN;
}


int soc_jer_plus_prd_custom_ether_type_get(int unit, soc_port_t port, uint32 flags, uint32 ether_type_code, uint32 *ether_type_val)
{
    SOCDNX_INIT_FUNC_DEFS;

    if (ether_type_code < SOC_JER_PLUS_PRD_CONFIG_ETHER_CODE_MIN || ether_type_code > SOC_JER_PLUS_PRD_CONFIG_ETHER_CODE_MAX) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: Eth code %d is invalid. Valid eth code values are 1-6"), unit ,port, ether_type_code));
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_ether_type_get(unit, port, ether_type_code - 1, ether_type_val));
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_control_frame_set(int unit, soc_port_t port, uint32 flags, soc_jer_nif_prd_control_plane_t *prd_ctrl_plane)
{
    uint64 max_mac_da_val;
    SOCDNX_INIT_FUNC_DEFS;

    /*SOC_JER_PLUS_PRD_CTRL_FRAME_MAC_DA_MAX_VAL(max_mac_da_val);*/
    COMPILER_64_SET(max_mac_da_val, 0xFFFF, 0xFFFFFFFF);

    if (COMPILER_64_GT(prd_ctrl_plane->mac_da_val, max_mac_da_val) || 
        COMPILER_64_GT(prd_ctrl_plane->mac_da_mask, max_mac_da_val)) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: MAC DA or MAC DA mask are invalid."), unit ,port));
    }
    if (prd_ctrl_plane->ether_type_code > SOC_JER_PLUS_PRD_CTRL_FRAME_ETH_CODE_MAX_VAL || 
        prd_ctrl_plane->ether_type_code > SOC_JER_PLUS_PRD_CTRL_FRAME_ETH_CODE_MAX_VAL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: Ether code or Ether code mask are invalid."), unit ,port));
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_control_plane_set(unit, port, prd_ctrl_plane));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_control_frame_get(int unit, soc_port_t port, uint32 flags, soc_jer_nif_prd_control_plane_t *prd_ctrl_plane)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_control_plane_get(unit, port, prd_ctrl_plane));

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_flex_key_construct_set(int unit, soc_port_t port, uint32 flags, uint32* offset_array, uint32 array_size)
{
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    for (i = 0; i < SOC_JER_NIF_PRD_MAX_KEY_BUILD_OFFSETS; ++i) {
        if (offset_array[i] > SOC_JER_PLUS_PRD_KEY_BUILD_OFFSET_MAX_VAL) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("offset %d is invalid"), offset_array[i]));
        }
    }

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_key_construct_set(unit, port, offset_array)); 
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_flex_key_construct_get(int unit, soc_port_t port, uint32 flags, uint32* offset_array, uint32* array_size)
{
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_key_construct_get(unit, port, offset_array));
    *array_size = SOC_JER_NIF_PRD_MAX_KEY_BUILD_OFFSETS; 

exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_flex_key_entry_set(int unit, soc_port_t port, uint32 flags, uint32 key_index, soc_jer_nif_prd_flex_key_entry_t* key_entry)
{
    soc_jer_nif_prd_tcam_entry_t tcam_entry;
    uint32 key_value = 0, key_mask = 0;
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    if (key_index > SOC_JER_PLUS_PRD_KEY_ENTRY_MAX_VAL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: Key index %d is invalid. Valid values are 0-31."), unit ,port, key_index));
    }
    if (key_entry->ether_code > SOC_JER_PLUS_PRD_CTRL_FRAME_ETH_CODE_MAX_VAL ||
        key_entry->ether_code_mask > SOC_JER_PLUS_PRD_CTRL_FRAME_ETH_CODE_MAX_VAL) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: Key Ether code or Ether code mask are invalid."), unit ,port));
    }
    if (key_entry->num_key_fields > SOC_JER_NIF_PRD_MAX_KEY_BUILD_OFFSETS) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, port %d: number of key fields %d is invalid."), unit ,port, key_entry->num_key_fields));
    }

    for (i = 0; i < key_entry->num_key_fields; ++i) {
        if (key_entry->key_fields_values[i] > SOC_JER_PLUS_PRD_KEY_FIELD_MAX_VAL ||
            key_entry->key_fields_masks[i] > SOC_JER_PLUS_PRD_KEY_FIELD_MAX_VAL ) {

            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("unit %d, key field value % or mask % is invalid"),
                                                unit , key_entry->key_fields_values[i], key_entry->key_fields_masks[i]));
        }
    }

    for (i = 0; i < key_entry->num_key_fields; ++i) {
        key_value |= (key_entry->key_fields_values[i] & 0xFF) << (i * 8);
        key_mask  |= (key_entry->key_fields_masks[i] & 0xFF) << (i * 8);
    }
    /*Ether code shuld be in bits 33-36 of the key */
    COMPILER_64_SET(tcam_entry.key, key_entry->ether_code, key_value);
    /*in the register key mask: 0 = valid, 1 = don't care, so we need to reverse the mask*/
    COMPILER_64_SET(tcam_entry.mask, (~key_entry->ether_code_mask & 0xF), ~key_mask);

    tcam_entry.priority = key_entry->priority;
    tcam_entry.valid = 1;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_key_entry_set(unit, port, key_index, &tcam_entry)); 
    
exit:
    SOCDNX_FUNC_RETURN;
}

int soc_jer_plus_prd_flex_key_entry_get(int unit, soc_port_t port, uint32 flags, uint32 key_index, soc_jer_nif_prd_flex_key_entry_t* key_entry)
{
    soc_jer_nif_prd_tcam_entry_t tcam_entry;
    uint32 key_value = 0, key_temp_value = 0, key_mask = 0, key_temp_mask = 0;
    int i;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_key_entry_get(unit, port, key_index, &tcam_entry));

    /*Ether code shuld be in bits 33-36 of the key */
    key_entry->ether_code = COMPILER_64_HI(tcam_entry.key);
    /*in the register key mask: 0 = valid, 1 = don't care, so we need to reverse the mask*/
    key_entry->ether_code_mask = (~COMPILER_64_HI(tcam_entry.mask)) & 0xF;

    key_value = COMPILER_64_LO(tcam_entry.key);
    /*in the register key mask: 0 = valid, 1 = don't care, so we need to reverse the mask*/
    key_mask = ~COMPILER_64_LO(tcam_entry.mask);
    
    for (i = 0; i < SOC_JER_NIF_PRD_MAX_KEY_BUILD_OFFSETS; ++i) {
        key_temp_value = key_value & (0xFF << (i * 8));
        key_entry->key_fields_values[i] = (key_temp_value >> (i * 8));
        key_temp_mask = key_mask & (0xFF << (i * 8));
        key_entry->key_fields_masks[i]  = (key_temp_mask >> (i * 8));
    }
    key_entry->num_key_fields = SOC_JER_NIF_PRD_MAX_KEY_BUILD_OFFSETS;
    key_entry->priority = tcam_entry.priority;
    
exit:
    SOCDNX_FUNC_RETURN;
}


/* 
 * This function is meant to be called when removing a port in a running system (Dynamic port feature) 
 * NOTE: all features which are configured per quad can be only restored if the port is the last in the quad!
 */
int soc_jer_port_prd_restore_hw_defaults(int unit, soc_port_t port) 
{
    int i, is_enabled;
    uint32 nof_ports_on_smae_quad, zero_map = 0, offset_array[4] = {0};
    soc_jer_nif_prd_hard_stage_properties_t properties;
    soc_jer_nif_prd_control_plane_t control_frame_conf;
    soc_jer_nif_prd_config_t prd_config;
    soc_jer_nif_prd_hard_stage_control_t hard_stage_ctrl;
    soc_jer_nif_prd_tcam_entry_t key_entry;
    soc_pbmp_t ports_on_same_quad;
    soc_reg_above_64_val_t zero_map_above_64;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(ports_on_same_quad);

    /** Clear port configurations */
    /*Disable PRD - only if enabled */
    SOCDNX_IF_ERR_EXIT(soc_jer_port_prd_enable_get(unit, port, 0, &is_enabled));
    if (is_enabled) {
        SOCDNX_IF_ERR_EXIT(soc_jer_port_prd_enable_set(unit, port, 0, socJerNifPrdDisable)); 
    }

    /*The followig configurations are only supported in Jer plus*/
    if (!SOC_IS_JERICHO_PLUS_ONLY(unit)) {
        SOC_EXIT;
    }

    /*zero TPIDs*/
    for (i = 0; i < 4; ++i) {
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_port_prd_tpid_set(unit, port, 0, i, 0));
    }
    /*PRD Hard stage port properties*/
    properties.default_priority = 0;
    properties.outer_tag_size = 0;
    properties.trust_inner_eth_tag = 1;
    properties.trust_ip_sdcp = 1;
    properties.trust_mpls_exp = 1;
    properties.trust_outer_eth_tag = 1;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_properties_set(unit, port, &properties));

    /*Control plane*/
    COMPILER_64_SET(control_frame_conf.mac_da_val, 0, 0);
    COMPILER_64_SET(control_frame_conf.mac_da_mask, 0, 0);
    control_frame_conf.ether_type_code = 0;
    control_frame_conf.ether_type_code_mask = 0;
    SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_control_plane_set(unit, port, &control_frame_conf));

    /** Check if port is last in quad */
    SOCDNX_IF_ERR_EXIT(soc_jer_port_ports_to_same_quad_get(unit, port, &ports_on_same_quad));
    SOC_PBMP_COUNT(ports_on_same_quad, nof_ports_on_smae_quad);
    if (nof_ports_on_smae_quad == 1) { /*last port on this quad*/
        /** Clear quad configurations */
        /*PRD general configs*/
        prd_config.never_drop_control_frames = 1;
        prd_config.never_drop_tdm_packets = 1;
        prd_config.seperate_tdm_and_non_tdm = 0;
        prd_config.seperate_tdm_and_non_tdm_two_ports = 0;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_config_set(unit, port, &prd_config));

        /*Hard stage control*/
        hard_stage_ctrl.itmh_type = 1;
        hard_stage_ctrl.small_chunk_priority_override = 1;
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_hard_stage_control_set(unit, port, &hard_stage_ctrl));

        /*Configurable Ether types*/
        for (i = 1; i < 7; ++i) {
            SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_custom_ether_type_set(unit, port, 0, i, 0));
        }

        /*zero all maps*/
        SOC_REG_ABOVE_64_CLEAR(zero_map_above_64);
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_tm_tc_dp_set(unit, port, zero_map_above_64));
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_ip_dscp_set(unit, port, zero_map_above_64));
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_eth_pcp_dei_set(unit, port, zero_map));
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_prd_map_mpls_exp_set(unit, port, zero_map));

        /*soft stage key construction*/
        SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_key_construct_set(unit, port, offset_array));

        /*clean the TCAM*/
        COMPILER_64_SET(key_entry.key, 0, 0);
        COMPILER_64_SET(key_entry.mask, 0, 0);
        key_entry.priority = 0;
        key_entry.valid = 0;
        for (i = 0; i < 32; ++i) {
            SOCDNX_IF_ERR_EXIT(soc_jer_plus_soft_stage_key_entry_set(unit, port, i, &key_entry));
        }
    }
    
exit:
    SOCDNX_FUNC_RETURN;
}

int
soc_jer_nif_qsgmii_pbmp_get(int unit, soc_port_t port, uint32 id, soc_pbmp_t *phy_pbmp)
{
    int first_phy;
    SOCDNX_INIT_FUNC_DEFS;

    SOC_PBMP_CLEAR(*phy_pbmp);
    if(id < 32) {
        first_phy = id  + 53;
    } else {
        first_phy = id - 32 + 113;
    }

    SOC_PBMP_PORT_ADD(*phy_pbmp, first_phy);

    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_port_eqg_nif_credit_init
 * Purpose:
 *      initializes the FQP credit balance per port
 * Preconditions:
 *      This function must be called when QMLF\HRF is in reset state
 * Parameters:
 *      unit    - Unit Number
 *      port    - local port
 * Returns:
 *      SOC_E_XXX
 */
STATIC
int soc_jer_port_eqg_nif_credit_init(int unit, soc_port_t port)
{

    int rv, core;
    uint32 egr_if;
    soc_reg_above_64_val_t reg_above64_val;

    SOCDNX_INIT_FUNC_DEFS;

    /* clear FQP TXI ready bits in EGQ */
    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_core_get(unit, port, &core));
    rv = sw_state_access[unit].dpp.soc.arad.tm.logical_ports_info.egr_interface.get(unit, port, &egr_if);
    SOCDNX_IF_ERR_EXIT(rv);

    SOCDNX_IF_ERR_EXIT(READ_EGQ_INIT_FQP_TXI_NIFr(unit, core, reg_above64_val));
    SHR_BITSET(reg_above64_val, egr_if);
    SOCDNX_IF_ERR_EXIT(WRITE_EGQ_INIT_FQP_TXI_NIFr(unit, core, reg_above64_val));

    sal_usleep(1);

    SHR_BITCLR(reg_above64_val, egr_if);
    SOCDNX_IF_ERR_EXIT(WRITE_EGQ_INIT_FQP_TXI_NIFr(unit, core, reg_above64_val));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_port_nbi_tx_egress_credits_set
 * Parameters:
 *      unit        - Unit Number
 *      port        - logical port
 *      cmd         - command.
 *                    SOC_JER_TX_EGRESS_CREDITS_CMD_FLUSH - continueously grants EGQ credits for the corresponding port
 *                    SOC_JER_TX_EGRESS_CREDITS_CMD_STOP (Note: This mode is non-recoverable) - will stop sending credits to the EGQ for the corresponding port
 *      enable     -  0: disable, 1:enable
 * Returns:
 *      SOC_E_XXX
 */
STATIC
int soc_jer_port_nbi_tx_egress_credits_set(int unit, soc_port_t port, SOC_JER_TX_EGRESS_CREDITS_CMD cmd, int enable)
{
    soc_pbmp_t phys, phy_ports;
    uint32 is_qsgmii=0, phy_port, lane, lane_in_nbi, lane_in_qmlf, qmlf_in_nbi, rval, qsgmii_offset, field_idx;
    int reg, reg_port, reg_idx;
    soc_field_t field;
    uint32 nof_pms_per_nbi  = SOC_DPP_DEFS_GET(unit, nof_pms_per_nbi);
    uint32 nof_lanes_per_nbi = nof_pms_per_nbi * NUM_OF_LANES_IN_PM;
    uint32 nof_port_per_nbih = nof_lanes_per_nbi;
    soc_port_if_t interface_type;

    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_phy_ports_get(unit, port, &phy_ports));
    SOCDNX_IF_ERR_EXIT(soc_jer_qsgmii_offsets_remove_pbmp(unit, &phy_ports, &phys));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_first_phy_port_get(unit, port, &phy_port));
    SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_dpp_qsgmii_offsets_remove, (unit, phy_port, &lane)));

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_interface_type_get(unit, port, &interface_type));

    if (lane == 0) { /* Coverity */
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("illegal lane value %d"), lane));
    }

    if (phy_port == 0) { /* Coverity */
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("illegal phy_port value %d"), phy_port));
    }

    lane--; /* switch to 0-based index */
    phy_port --; /* switch to 0-based index */
    lane_in_nbi = lane % nof_lanes_per_nbi;
    lane_in_qmlf = lane % NUM_OF_LANES_IN_PM;
    qmlf_in_nbi = lane_in_nbi / NUM_OF_LANES_IN_PM;
    qsgmii_offset = phy_port % SUB_PHYS_IN_QSGMII;

    if (interface_type == SOC_PORT_IF_QSGMII) {
        is_qsgmii = 1;
    }

    if (SOC_IS_QUX(unit)) {
        reg_port = REG_PORT_ANY;
        reg = NIF_TX_EGRESS_CREDITS_DEBUG_PMr;
        reg_idx = qmlf_in_nbi;
    } else {
        if (lane < nof_port_per_nbih)
        {
            reg_port = REG_PORT_ANY;
            reg = NBIH_TX_EGRESS_CREDITS_DEBUG_PMr;
            reg_idx = qmlf_in_nbi;
        }
        else
        {
            reg_port = (lane < 2*nof_lanes_per_nbi) ? 0 : 1;
    
            switch (qmlf_in_nbi)
            {
            case 0:
            case 1:
            case 2:
                reg = NBIL_TX_EGRESS_CREDITS_DEBUG_PMr;
                reg_idx = qmlf_in_nbi;
                break;
            case 3:
                reg = NBIL_TX_EGRESS_CREDITS_DEBUG_PM_3r;
                reg_idx = 0;
                break;
            case 4:
                reg = NBIL_TX_EGRESS_CREDITS_DEBUG_PM_4r;
                reg_idx = 0;
                break;
            case 5:
                reg = NBIL_TX_EGRESS_CREDITS_DEBUG_PM_5r;
                reg_idx = 0;
                break;
            default:
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("qmlf index in nbi %d"), qmlf_in_nbi));
            }
    
        }
    }
    field_idx = lane_in_qmlf*SUB_PHYS_IN_QSGMII + is_qsgmii*qsgmii_offset;

    if (cmd == SOC_JER_TX_EGRESS_CREDITS_CMD_FLUSH)
    {
        field = soc_jer_nbi_tx_flush_egress_fields[field_idx];
    }
    else if (cmd == SOC_JER_TX_EGRESS_CREDITS_CMD_STOP)
    {
        field = soc_jer_nbi_tx_stop_egress_fields[field_idx];
    }
    else
    {
        SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("illegal tx egress credits command %d"), cmd));
    }

    SOCDNX_IF_ERR_EXIT(soc_reg32_get(unit, reg, reg_port, reg_idx, &rval));
    soc_reg_field_set(unit, reg, &rval, field, enable);
    SOCDNX_IF_ERR_EXIT(soc_reg32_set(unit, reg, reg_port, reg_idx, rval));

exit:
    SOCDNX_FUNC_RETURN;
}

/*
 * Function:
 *      soc_jer_portmod_ilkn_speed_validate
 * Purpose:
 *      Validate that ILKN's serdes rate doesn't exceed the limit of
 *      the core clock speed.
 * Parameters:
 *      unit        - Unit Number
 *      port        - ILKN port
 * Returns:
 *      SOC_E_XXX
 */
STATIC
int soc_jer_portmod_ilkn_speed_validate(int unit, soc_port_t port)
{
    uint32 core_clock_speed = 0;
    int speed = 0;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_port_sw_db_speed_get(unit, port, &speed));
    core_clock_speed = SOC_DPP_CONFIG(unit)->arad->init.core_freq.frequency;
    if (speed >= core_clock_speed * 40 / 1000) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_CONFIG, (_BSL_SOCDNX_MSG("Speed %d for port %d is too high for core clock speed %d"),speed,port,core_clock_speed));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/*
* NIF SW restrictions for SKU chips.
*/
int
soc_jer_nif_sku_restrictions(int unit, soc_pbmp_t phy_pbmp, soc_port_if_t interface, uint32 protocol_offset, uint32 is_kbp){

        SOCDNX_INIT_FUNC_DEFS;

        if (dcmn_device_block_for_feature(unit,DCMN_JER_NIF_24_44_FEATURE)) {

            /*QUAD 15 (PM10-9) is invalid*/
            if(SOC_PBMP_MEMBER(phy_pbmp, 97) ||
               SOC_PBMP_MEMBER(phy_pbmp, 101) ||
               SOC_PBMP_MEMBER(phy_pbmp, 105) ||
               SOC_PBMP_MEMBER(phy_pbmp, 109)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Device %s can't mapped to Quad 15 (PM10-9)"),
                                                  soc_dev_name(unit)));
            }
        }

        if (dcmn_device_block_for_feature(unit,DCMN_JER_NIF_40_20_FEATURE)) {

            /*QUADs 6,12 and 13 are invalid*/
            if(SOC_PBMP_MEMBER(phy_pbmp, 25) ||
               SOC_PBMP_MEMBER(phy_pbmp, 26) ||
               SOC_PBMP_MEMBER(phy_pbmp, 27) ||
               SOC_PBMP_MEMBER(phy_pbmp, 28) ||
               SOC_PBMP_MEMBER(phy_pbmp, 85) ||
               SOC_PBMP_MEMBER(phy_pbmp, 86) ||
               SOC_PBMP_MEMBER(phy_pbmp, 87) ||
               SOC_PBMP_MEMBER(phy_pbmp, 88) ||
               SOC_PBMP_MEMBER(phy_pbmp, 89) ||
               SOC_PBMP_MEMBER(phy_pbmp, 90) ||
               SOC_PBMP_MEMBER(phy_pbmp, 91) ||
               SOC_PBMP_MEMBER(phy_pbmp, 92)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Device %s can't mapped to Quads 6,12 and 13"),
                                                  soc_dev_name(unit)));
            }
        }

        if (dcmn_device_block_for_feature(unit,DCMN_JER_NIF_48_0_FEATURE)) {

            /*QUADs 6,7,8,9,10 and 11 are invalid*/
            if(SOC_PBMP_MEMBER(phy_pbmp, 25) ||
               SOC_PBMP_MEMBER(phy_pbmp, 26) ||
               SOC_PBMP_MEMBER(phy_pbmp, 27) ||
               SOC_PBMP_MEMBER(phy_pbmp, 28) ||
               SOC_PBMP_MEMBER(phy_pbmp, 29) ||
               SOC_PBMP_MEMBER(phy_pbmp, 30) ||
               SOC_PBMP_MEMBER(phy_pbmp, 31) ||
               SOC_PBMP_MEMBER(phy_pbmp, 32) ||
               SOC_PBMP_MEMBER(phy_pbmp, 33) ||
               SOC_PBMP_MEMBER(phy_pbmp, 34) ||
               SOC_PBMP_MEMBER(phy_pbmp, 35) ||
               SOC_PBMP_MEMBER(phy_pbmp, 36) ||
               SOC_PBMP_MEMBER(phy_pbmp, 37) ||
               SOC_PBMP_MEMBER(phy_pbmp, 41) ||
               SOC_PBMP_MEMBER(phy_pbmp, 45) ||
               SOC_PBMP_MEMBER(phy_pbmp, 49) ||
               SOC_PBMP_MEMBER(phy_pbmp, 53) ||
               SOC_PBMP_MEMBER(phy_pbmp, 54) ||
               SOC_PBMP_MEMBER(phy_pbmp, 55) ||
               SOC_PBMP_MEMBER(phy_pbmp, 56) ||
               SOC_PBMP_MEMBER(phy_pbmp, 57) ||
               SOC_PBMP_MEMBER(phy_pbmp, 58) ||
               SOC_PBMP_MEMBER(phy_pbmp, 59) ||
               SOC_PBMP_MEMBER(phy_pbmp, 60) ||
               SOC_PBMP_MEMBER(phy_pbmp, 61) ||
               SOC_PBMP_MEMBER(phy_pbmp, 62) ||
               SOC_PBMP_MEMBER(phy_pbmp, 63) ||
               SOC_PBMP_MEMBER(phy_pbmp, 64) ||
               SOC_PBMP_MEMBER(phy_pbmp, 65) ||
               SOC_PBMP_MEMBER(phy_pbmp, 66) ||
               SOC_PBMP_MEMBER(phy_pbmp, 67) ||
               SOC_PBMP_MEMBER(phy_pbmp, 68) ||
               SOC_PBMP_MEMBER(phy_pbmp, 69) ||
               SOC_PBMP_MEMBER(phy_pbmp, 70) ||
               SOC_PBMP_MEMBER(phy_pbmp, 71) ||
               SOC_PBMP_MEMBER(phy_pbmp, 72) ||
               SOC_PBMP_MEMBER(phy_pbmp, 73) ||
               SOC_PBMP_MEMBER(phy_pbmp, 74) ||
               SOC_PBMP_MEMBER(phy_pbmp, 75) ||
               SOC_PBMP_MEMBER(phy_pbmp, 76) ||
               SOC_PBMP_MEMBER(phy_pbmp, 77) ||
               SOC_PBMP_MEMBER(phy_pbmp, 78) ||
               SOC_PBMP_MEMBER(phy_pbmp, 79) ||
               SOC_PBMP_MEMBER(phy_pbmp, 80) ||
               SOC_PBMP_MEMBER(phy_pbmp, 81) ||
               SOC_PBMP_MEMBER(phy_pbmp, 82) ||
               SOC_PBMP_MEMBER(phy_pbmp, 83) ||
               SOC_PBMP_MEMBER(phy_pbmp, 84)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Device %s can't mapped to Quads 6,7,8,9,10 and 11"),
                                                  soc_dev_name(unit)));
            }
        }

        if (dcmn_device_block_for_feature(unit,DCMN_JER_NIF_12_32_FEATURE)) {

            /*QUADs 2,4,5 and 15 are invalid*/
            if(SOC_PBMP_MEMBER(phy_pbmp, 9) ||
               SOC_PBMP_MEMBER(phy_pbmp, 10) ||
               SOC_PBMP_MEMBER(phy_pbmp, 11) ||
               SOC_PBMP_MEMBER(phy_pbmp, 12) ||
               SOC_PBMP_MEMBER(phy_pbmp, 17) ||
               SOC_PBMP_MEMBER(phy_pbmp, 18) ||
               SOC_PBMP_MEMBER(phy_pbmp, 19) ||
               SOC_PBMP_MEMBER(phy_pbmp, 20) ||
               SOC_PBMP_MEMBER(phy_pbmp, 21) ||
               SOC_PBMP_MEMBER(phy_pbmp, 22) ||
               SOC_PBMP_MEMBER(phy_pbmp, 23) ||
               SOC_PBMP_MEMBER(phy_pbmp, 24) ||
               SOC_PBMP_MEMBER(phy_pbmp, 97) ||
               SOC_PBMP_MEMBER(phy_pbmp, 101) ||
               SOC_PBMP_MEMBER(phy_pbmp, 105) ||
               SOC_PBMP_MEMBER(phy_pbmp, 109)) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Device %s can't be mapped to Quads 2,4,5 and 15."),
                                                  soc_dev_name(unit)));
            }
        }

        /* 4 ILKN ports */
        if (dcmn_device_block_for_feature(unit,DCMN_JER_4_ILKN_PORTS_FEATURE)) {
            if (interface == SOC_PORT_IF_ILKN &&
                protocol_offset >= 4) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_SOCDNX_MSG("Device %s can use ILKN 0-3 only."), 
                                                  soc_dev_name(unit)));
                }
        }
exit:
    SOCDNX_FUNC_RETURN;
}

/*
* Port speed SW restrictions for SKU chips. Does nothing for Jericho, needed for the MBCM dispatcher.
*/
int
soc_jer_port_speed_sku_restrictions(int unit, soc_port_t port, int speed){

    SOCDNX_INIT_FUNC_DEFS;

    /* Do nothing */

    SOCDNX_FUNC_RETURN;
}

soc_error_t
soc_jer_port_loopback_set(int unit, soc_port_t port, soc_dcmn_loopback_mode_t loopback)
{
    soc_dcmn_loopback_mode_t current_lb = soc_dcmn_loopback_mode_none;
    int enable, flags, first_packet_sw_bypass;
    uint32 offset;
    SOCDNX_INIT_FUNC_DEFS;

    SOCDNX_IF_ERR_EXIT(soc_dcmn_port_loopback_get(unit, port, &current_lb));
    if(current_lb == loopback){
        /*nothing to do*/
        SOC_EXIT;
    }
    if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
        /* enable_get of ilkn_over_fabric (dnx_fabric_port_enable_get) enquire also Rx MAC. This will give false alarm 
         *  (i.e. disabled) when first_packet_sw_bypass is set and phy is down.
         *  Therefore, when ilkn_over_fabric and irst_packet_sw_bypass are set => checking enable_get only for Tx MAC.
         * This problem is not relevant for PM25/PM10 ILKN NIF port (pm4x25_port_enable_get / pm4x10_port_enable_get) since then,
         *  OS ILKN checks status according to phy, and ignoring Rx MAC */
        flags = 0;
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_protocol_offset_get(unit, port, 0, &offset));
        first_packet_sw_bypass = SOC_DPP_CONFIG(unit)->arad->init.ports.ilkn_first_packet_sw_bypass;
        if (SOC_JER_NIF_IS_ILKN_IF_OVER_FABRIC(unit, offset) && first_packet_sw_bypass) {
            PORTMOD_PORT_ENABLE_MAC_SET(flags);
            PORTMOD_PORT_ENABLE_TX_SET(flags);
        }
        SOCDNX_IF_ERR_EXIT(portmod_port_enable_get(unit, port, flags, &enable));
        SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 0));
        SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, 0)));
    }

    SOCDNX_IF_ERR_EXIT(soc_dcmn_port_loopback_set(unit, port, loopback));

    if (IS_IL_PORT(unit, port) && (BCM_E_NONE == bcm_petra_init_check(unit))) {
            SOCDNX_IF_ERR_EXIT(MBCM_DPP_SOC_DRIVER_CALL(unit, mbcm_port_ilkn_bypass_interface_enable, (unit, port, enable)));
            SOCDNX_IF_ERR_EXIT(soc_jer_nif_ilkn_wrapper_port_enable_set(unit, port, 1));
    }

exit:
    SOCDNX_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

