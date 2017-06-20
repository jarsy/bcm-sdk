/*
 *         
 * $Id:$
 * 
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *         
 *
 */

#ifndef _PORTMOD_INTERNAL_H_
#define _PORTMOD_INTERNAL_H_

#include <soc/portmod/portmod.h>
#include <appl/diag/diag.h>

#define MAX_PMS_PER_PHY (3)
#define MAX_VARS_PER_BUFFER (64)
#define VERSION(_ver) (_ver)

#define PORTMOD_PORT_IS_DEFAULT_TX_PARAMS_UPDATED(port_dynamic_state)     (port_dynamic_state & 0x1)
#define PORTMOD_PORT_DEFAULT_TX_PARAMS_UPDATED_SET(port_dynamic_state)    (port_dynamic_state |= 0x1)
#define PORTMOD_PORT_DEFAULT_TX_PARAMS_UPDATED_CLR(port_dynamic_state)    (port_dynamic_state &= 0xfffe)

#define PORTMOD_PORT_IS_AUTONEG_MODE_UPDATED(port_dynamic_state)          (port_dynamic_state & 0x2)
#define PORTMOD_PORT_AUTONEG_MODE_UPDATED_SET(port_dynamic_state)         (port_dynamic_state |= 0x2)
#define PORTMOD_PORT_AUTONEG_MODE_UPDATED_CLR(port_dynamic_state)         (port_dynamic_state &= 0xfffd) 

/* following defines is for FEC init value */
#define PORTMOD_PORT_FEC_OFF         0x0
#define PORTMOD_PORT_FEC_CL74        0x1
#define PORTMOD_PORT_FEC_CL91        0x2

typedef struct pm4x10_s *pm4x10_t;
typedef struct pm4x10q_s *pm4x10q_t;
typedef struct pm4x25_s *pm4x25_t;
typedef struct pm12x10_s *pm12x10_t;
typedef struct dnx_fabric_s *dnx_fabric_t;
typedef struct pm8x50_fabric_s *pm8x50_fabric_t;
typedef struct pmOsIlkn_s *pmOsIlkn_t;
typedef struct pm_qtc_s *pm_qtc_t;
typedef struct pm4x2p5_s *pm4x2p5_t;

typedef union pm_db_u{
    pm4x10_t        pm4x10_db;
    pm4x10q_t       pm4x10q_db;
    dnx_fabric_t    dnx_fabric;
    pm8x50_fabric_t pm8x50_fabric;
    pm4x25_t        pm4x25_db;
    pm12x10_t       pm12x10_db;
    pmOsIlkn_t      pmOsIlkn_db;
    pm_qtc_t        pm_qtc_db;
    pm4x2p5_t       pm4x2p5_db;
}pm_db_t;

/* This structure cntain specific PM state. 
    it's pointed from pms array in portmod.c */
struct pm_info_s{
    portmod_dispatch_type_t type; /* PM type (used manly for dispatching)*/
    int unit; /* PM unit ID */
    int wb_buffer_id; /* Buffer id is given for each PM (see warmboot description in portmod.c) */
    int wb_vars_ids[MAX_VARS_PER_BUFFER]; /* Allotcaed WB variables for this PM */
    pm_db_t pm_data; /* PM internal state. This information is internal, and mainted in the speicifc PM c code*/
};

typedef enum pm_type_e {
    PM_TYPE_4X10,
    PM_TYPE_4X25,
    PM_TYPE_12X10,
    PM_TYPE_4X10Q,
    PM_TYPE_FCPM,
    PM_TYPE_COUNT
}pm_type_t;

typedef enum pm_tech_type_e {
    PM_TECH_28NM,         /* Technology process.  28nm */
    PM_TECH_16NM,         /* Technology process.  16nm */
    PM_TECH_COUNT
}pm_tech_type_t;

typedef enum pm_version_num_e {
    PM_VERSION_NUM_0,
    PM_VERSION_NUM_1,
    PM_VERSION_NUM_2,
    PM_VERSION_NUM_3,
    PM_VERSION_NUM_4,
    PM_VERSION_NUM_5,
    PM_VERSION_NUM_6,
    PM_VERSION_NUM_7
}pm_version_num_t;

typedef enum pm_revision_e {
    PM_REVISION_A,
    PM_REVISION_B,
    PM_REVISION_C,
    PM_REVISION_D,
    PM_REVISION_COUNT
}pm_revision_t;

struct pm_version_s{
    pm_type_t       type;
    pm_tech_type_t  tech_process;
    int8           version_number;
    pm_revision_t   revision_letter;
};

#define PM_VERSION_NUMBER_UNKNOWN -1
#define PORTMOD_PM_VERSION_NUMBER_IS_UNKNOWN(number)  (number == PM_VERSION_NUMBER_UNKNOWN)

#define PORTMOD_PM_VERSION_REVISION_IS_A(revision)  (revision == PM_REVISION_A)
#define PORTMOD_PM_VERSION_REVISION_IS_B(revision)  (revision == PM_REVISION_B)
#define PORTMOD_PM_VERSION_REVISION_IS_C(revision)  (revision == PM_REVISION_C)
#define PORTMOD_PM_VERSION_REVISION_IS_D(revision)  (revision == PM_REVISION_D)
#define PORTMOD_PM_VERSION_REVISION_IS_UNKNOWN(revision)  (revision == PM_REVISION_COUNT)

typedef struct pm_info_s *pm_info_t;
typedef struct pm_version_s pm_version_t;

/* When port macro is added to the PMM the user passing in specific PM required information.
   In some cases the PMM rebuild another structure before sending the information to the PM 
   the *_internal_t strucutres represents the internal information as the PMM send to the PM
   (translation code can be found in portmod.c */

/* PM4x25: no translation is required */
typedef portmod_pm4x25_create_info_t portmod_pm4x25_create_info_internal_t;

/* PM4x10: no translation is required */
typedef portmod_pm4x10_create_info_t portmod_pm4x10_create_info_internal_t;

/* QTC: no translation is required */
typedef portmod_pm_qtc_create_info_t portmod_pm_qtc_create_info_internal_t;

/* DNX fabric internal create structure*/
typedef struct portmod_dnx_fabric_create_info_internal_s{
    phymod_ref_clk_t ref_clk; /**< SerDes quad ref clock */
    phymod_access_t access; /**< phymod access structure; defines the register access for the SerDes Core */
    phymod_lane_map_t lane_map;
    phymod_firmware_load_method_t fw_load_method;
    phymod_firmware_loader_f external_fw_loader; /**< firmware loader that will be used in case that fw_load_method=phymodFirmwareLoadMethodExternal */
    phymod_polarity_t polarity; /**< Lanes Polarity */
    int fmac_schan_id; /**< FMAC schan id */
    int fsrd_schan_id; /**< FSRD schan id */
    int fsrd_internal_quad; /**< Core instance in FSRD */
    int first_phy_offset;
    int core_index;
    int is_over_nif;
    pm_info_t *pms; /**< PM used for fabric over nif */
}portmod_dnx_fabric_create_info_internal_t;

/* PM8x50_fabric internal create structure*/
typedef struct portmod_pm8x50_fabric_create_info_internal_s{
    phymod_ref_clk_t ref_clk; /**< SerDes quad ref clock */
    phymod_access_t access; /**< phymod access structure; defines the register access for the SerDes Core */
    phymod_lane_map_t lane_map;
    phymod_firmware_load_method_t fw_load_method;
    phymod_firmware_loader_f external_fw_loader; /**< firmware loader that will be used in case that fw_load_method=phymodFirmwareLoadMethodExternal */
    phymod_polarity_t polarity; /**< Lanes Polarity */
    int fmac_schan_id; /**< FMAC schan id */
    int fsrd_schan_id; /**< FSRD schan id */
    int fsrd_internal_quad; /**< Core instance in FSRD */
    int first_phy_offset;
    int core_index;
    int is_over_nif;
    pm_info_t *pms; /**< PM used for fabric over nif */
}portmod_pm8x50_fabric_create_info_internal_t;

/* PM4x10Q internal create structure*/
typedef struct portmod_pm4x10q_create_info_internal_s{
    pm_info_t pm4x10;
    uint32 blk_id;
    void* qsgmii_user_acc;
    void* pm4x10_user_acc;
}portmod_pm4x10q_create_info_internal_t;

/* PM12x10 internal create structure*/
typedef struct portmod_pm12x10_create_info_internal_s{
    pm_info_t pm4x25;
    pm_info_t pm4x10[3];
    uint32 flags;
    int blk_id;
    int refclk_source;
}portmod_pm12x10_create_info_internal_t;

/* PM4x2p5: no translation is required */
typedef portmod_pm4x2p5_create_info_t portmod_pm4x2p5_create_info_internal_t;

/* OS ILKN internal create structure*/
typedef struct portmod_os_ilkn_create_info_internal_s{
    int nof_aggregated_pms;
    pm_info_t *pms;
    int wm_high[PORTMOD_MAX_ILKN_PORTS_PER_ILKN_PM]; /**< watermark high value */
    int wm_low[PORTMOD_MAX_ILKN_PORTS_PER_ILKN_PM]; /**< watermark low value */
    uint8 is_over_fabric;
    uint32 core_clock_khz;
}portmod_os_ilkn_create_info_internal_t;

/* Union for internal create structures*/
typedef union portmod_pm_specific_create_info_internal_u{
    portmod_dnx_fabric_create_info_internal_t dnx_fabric;
    portmod_pm8x50_fabric_create_info_internal_t pm8x50_fabric;
    portmod_pm4x25_create_info_internal_t pm4x25;
    portmod_pm4x10_create_info_internal_t pm4x10;
    portmod_pm4x10q_create_info_internal_t pm4x10q;
    portmod_pm_qtc_create_info_internal_t pm_qtc;
    portmod_pm12x10_create_info_internal_t pm12x10;
    portmod_os_ilkn_create_info_internal_t os_ilkn;
    portmod_pm4x2p5_create_info_internal_t pm4x2p5;
}portmod_pm_specific_create_info_internal_t;

/* the structure which is used as input to portmod_port_macro_add function*/
typedef struct portmod_pm_create_info_internal_s {
    portmod_dispatch_type_t type; /**< PM type */
    portmod_pbmp_t phys; /**< which PHYs belongs to the PM */
    portmod_pm_specific_create_info_internal_t pm_specific_info;
} portmod_pm_create_info_internal_t;

/* PMs can implement xxx_default_bus_update() internal API to update phymod bus
   When this API is implemented the below struct is used as input*/
typedef struct portmod_bus_update_s {
    phymod_firmware_loader_f external_fw_loader;
    phymod_bus_t* default_bus;
    void* user_acc;
    int blk_id;
} portmod_bus_update_t;

/* External Phy information */
typedef struct portmod_ext_phy_core_info_s
{
    phymod_core_access_t core_access; /**< core access */
    int is_initialized; /**< Phy is Initialized - need to convert to WB */
} portmod_ext_phy_core_info_t;

#define PM_DRIVER(pm_info) (__portmod__dispatch__[(pm_info)->type])

#define XPHY_PBMP_IDX_MAX                       PORTMOD_MAX_NUM_XPHY_SUPPORTED
#define XPHY_PBMP_WORD_WIDTH                    32

#define XPHY_PBMP_WORD_MAX                      \
            ((XPHY_PBMP_IDX_MAX+XPHY_PBMP_WORD_WIDTH-1)/XPHY_PBMP_WORD_WIDTH)

#define XPHY_PBMP_WORD_GET(bm,word)             ((bm).pbits[(word)])
#define XPHY_PBMP_WENT(idx)                     ((idx)/XPHY_PBMP_WORD_WIDTH)

#define XPHY_PBMP_WBIT(idx)                     \
                                        (1U << ((idx) % XPHY_PBMP_WORD_WIDTH))

#define XPHY_PBMP_ENTRY(bm, idx)                \
                                    (XPHY_PBMP_WORD_GET(bm,XPHY_PBMP_WENT(idx)))

#define XPHY_PBMP_MEMBER(bm, idx)               \
                        ((XPHY_PBMP_ENTRY(bm, idx) & XPHY_PBMP_WBIT(idx)) != 0)

#define XPHY_PBMP_CLEAR(pbm)                    do { \
        int _w; \
        for (_w = 0; _w < XPHY_PBMP_WORD_MAX; _w++) { \
            XPHY_PBMP_WORD_GET(bm, _w) = 0; \
        } \
    } while (0)

#define XPHY_PBMP_IDX_ADD(bm, idx)             \
                            ( XPHY_PBMP_ENTRY(bm, idx) |=  XPHY_PBMP_WBIT(idx))

#define XPHY_PBMP_IDX_REMOVE(bm, idx)          \
                            ( XPHY_PBMP_ENTRY(bm, idx) &=  ~XPHY_PBMP_WBIT(idx))

#define XPHY_PBMP_ITER(bmp, idx)                \
    for ((idx) = 0; (idx) < XPHY_PBMP_IDX_MAX; (idx)++) \
        if (XPHY_PBMP_MEMBER((bmp), (idx)))

typedef struct xphy_pbmp {
    uint32  pbits[XPHY_PBMP_WORD_MAX];
} xphy_pbmp_t;

#define PORTMOD_DIV_ROUND_UP(x,y) ( ((x)+((y)-1)) / (y) )

#define PORTMOD_BIT_COUNT(pbm, count)  (count = _shr_popcount(pbm))

/* Initialize portmod_pm_create_info_internal_t structure */
int portmod_pm_create_info_internal_t_init(int unit, portmod_pm_create_info_internal_t *create_info_internal);

/* Translate PM id to PM type */
int portmod_pm_id_pm_type_get(int unit, int pm_id, portmod_dispatch_type_t *type);

/* Get  internal PM info from a port */
int portmod_pm_info_get(int unit, int port, pm_info_t *pm_info);
/* Get list of PMs attached to a phy */
int portmod_phy_pms_info_get(int unit, int phy, int max_pms, pm_info_t *pms_info, int *nof_pms);

int portmod_pm_info_type_get(int unit, int port, portmod_dispatch_type_t type, pm_info_t* pm_info);
int portmod_pm_info_from_pm_id_get(int unit, int pm_id, pm_info_t* pm_info);
int portmod_port_pm_id_get(int unit, int port, int *pm_id);
int portmod_port_interface_type_get(int unit, int port, soc_port_if_t *interface);
int portmod_port_main_core_access_get(int unit, int port, int phyn,
                                      phymod_core_access_t *core_access,
                                      int *nof_cores);

/*is interface type supported by PM should be supported by all PMs types*/
int portmod_pm_interface_type_is_supported(int unit, pm_info_t pm_info, soc_port_if_t interface, int* is_supported);

int portmod_pm_init(int unit, const portmod_pm_create_info_internal_t* pm_add_info, int wb_buffer_index, pm_info_t pm_info);
int portmod_pm_destroy(int unit, pm_info_t pm_info);
int portmod_pm_core_info_get(int unit, pm_info_t pm_info, int phyn, portmod_pm_core_info_t* core_info);
int portmod_port_attach(int unit, int port, const portmod_port_add_info_t* add_info);
int portmod_port_detach(int unit, int port);
int portmod_port_replace(int unit, int port, int new_port);

int portmod_next_wb_var_id_get(int unit, int *var_id);
int portmod_ext_phy_lane_attach_to_pm(int unit, pm_info_t pm_info, int iphy, int phyn, const portmod_lane_connection_t* lane_connection);
int portmod_ext_phy_lane_detach_from_pm(int unit, pm_info_t pm_info, int iphy, int phyn, portmod_lane_connection_t* lane_connection);

/*!
 * portmod_port_mac_rsv_mask_set
 *
 * @brief  portmod port rsv mask set
 *
 * @param [in]  unit            - unit id
 * @param [in]  port            - logical port
 * @param [in]  rsv_mask        - rsv mask
 */
int portmod_port_mac_rsv_mask_set(int unit, int port, uint32 rsv_mask);

/*!
 * portmod_port_mib_reset_toggle
 *
 * @brief  portmod port mib reset toggle
 *
 * @param [in]  unit            - unit id
 * @param [in]  port            - logical port
 * @param [in]  port_index      - internal port
 */
int portmod_port_mib_reset_toggle(int unit, int port, int port_index);

int portmod_xphy_lane_attach_to_pm(int unit, pm_info_t pm_info, int iphy, int phyn, const portmod_xphy_lane_connection_t* lane_connection);
int portmod_xphy_lane_detach_from_pm(int unit, pm_info_t pm_info, int iphy, int phyn, portmod_xphy_lane_connection_t* lane_connection);
int portmod_max_pms_get(int unit, int* max_pms);
int portmod_eyescan_diag_dispatch(int unit, soc_port_t port, args_t *a);
int portmod_phy_pm_type_get(int unit, int phy, portmod_dispatch_type_t *type);
int portmod_xphy_addr_set(int unit, int idx, int xphy_addr);
int portmod_xphy_db_addr_set(int unit, int idx, int xphy_addr);
int portmod_xphy_all_valid_phys_get(int unit, xphy_pbmp_t *active_phys);
int portmod_xphy_valid_phy_set (int unit, int xphy_idx, int valid);
int portmod_xphy_valid_phy_get (int unit, int xphy_idx, int *valid);
int portmod_xphy_db_addr_get(int unit, int xphy_idx, int* xphy_addr);


/*!
 * portmod_pm_is_in_pm12x10
 *
 * @brief Get whether the Port Macro is part of PM12x10
 *
 * @param [in]  unit            - unit id
 * @param [in]  pm_info         -
 * @param [out]  in_pm12x10     -
 */
int portmod_pm_is_in_pm12x10(int unit, pm_info_t pm_info, int* in_pm12x10);

/*!
 * portmod_port_lane_count_get
 *
 * @brief Get the number of lanes belong to  a logical port.
 *
 * @param [in]  unit            - unit id
 * @param [in]  port            - logical port
 * @param [in]  line_side       - line side 1, sys_side 0
 * @param [out]  num_lanes       - num of line side lanes
 */
int portmod_port_lane_count_get(int unit, int port, int line_side, int* num_lanes);

#endif /*_PORTMOD_INTERNAL_H_*/
