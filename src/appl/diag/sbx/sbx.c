/*
 * $Id: sbx.c,v 1.452.14.1 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        sbx.c
 * Purpose:     First cut at some sbx commands
 * Requires:
 */


#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <sal/appl/io.h>
#include <soc/defs.h>
#include <appl/diag/system.h>
#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/hal_user.h>
#include <bcm_int/sbx/error.h>
#include <sal/core/time.h>
#include <appl/diag/sbx/sbx.h>
#ifdef BCM_QE2000_SUPPORT
#include <soc/sbx/qe2000_spi.h>
#include <soc/sbx/qe2000.h>
#include <appl/diag/sbx/qe2000_cmds.h>
#include <soc/sbx/qe2k/qe2k.h>
#endif
#ifdef BCM_BME3200_SUPPORT
#include <soc/sbx/bme3200.h>
#include <appl/diag/sbx/bm3200_cmds.h>
#endif
#ifdef BCM_BM9600_SUPPORT
#include <soc/sbx/bm9600.h>
#include <soc/sbx/bm9600_soc_init.h>
#include <appl/diag/sbx/bm9600_cmds.h>
#endif

#include <soc/sbx/sbWrappers.h>

#include <bcm_int/sbx/cosq.h>

#ifdef BCM_EASY_RELOAD_SUPPORT
#include <bcm_int/sbx/multicast.h>
#endif
#ifdef BCM_SIRIUS_SUPPORT
#include <soc/sbx/sirius.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/trunk.h>
#include <appl/diag/sbx/sirius_cmds.h>
#endif
#ifdef BCM_CALADAN3_SUPPORT
#include <appl/diag/sbx/caladan3_cmds.h>
#ifdef BCM_CALADAN3_G3P1_SUPPORT
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#endif
#endif

#include <appl/diag/cmdlist.h>
#include <appl/diag/shell.h>
#include <appl/diag/sbx/sbx.h>
#include <appl/diag/sbx/register.h>
#include <appl/diag/sbx/field.h>
#include <appl/diag/sbx/gu2.h>
#include <appl/diag/sbx/brd_sbx.h>
#include <sal/appl/pci.h>


#include <bcm/error.h>
#include <bcm/init.h>
#include <bcm/rx.h>
#include <bcm/vlan.h>
#include <bcm/vswitch.h>
#include <bcm/stg.h>
#include <bcm/mcast.h>
#include <bcm/trunk.h>
#include <bcm/stack.h>
#include <bcm/cosq.h>
#include <bcm/port.h>
#include <bcm_int/sbx/rx.h>
#include <bcm_int/sbx/fabric.h>
#include <bcm_int/sbx/port.h>
#include <bcm/fabric.h>
#include <bcm/multicast.h>

#include <shared/idxres_fl.h>
#include <shared/idxres_afl.h>

#include <bcm_int/control.h>

/* #define TEST_REQUEUE - define for testing only requeue path */
/* #define TEST_3125 - define for testing 3125 and undefine GNATS_36884 for hybrid test */
/* #define TEST_REQUEUE 0 */
/* #define GNATS36884_TEST_CONFIG 1 A-A default, A-B local when defined */
#define GNATS36884_TEST_CONFIG 1
  
#define MAC_FMT       "%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x"
#define MAC_PFMT(mac) (mac)[0], (mac)[1], (mac)[2], \
                      (mac)[3], (mac)[4], (mac)[5]

#define SBX_CARD_TYPE_PL_CHASSIS_LINE_CARD            1
#define SBX_CARD_TYPE_PL_CHASSIS_FABRIC_CARD          2
#define SBX_CARD_TYPE_PL_STANDALONE_LINE_CARD         3
#define SBX_CARD_TYPE_FE2KXT_CHASSIS_LINE_CARD        4
#define SBX_CARD_TYPE_SIRIUS_SIM_STANDALONE_LINE_CARD (BOARD_TYPE_SIRIUS_SIM)
#define SBX_CARD_TYPE_PL_IPASS                        (BOARD_TYPE_POLARIS_IPASS)  
#define SBX_CARD_TYPE_SIRIUS_IPASS                    (BOARD_TYPE_SIRIUS_IPASS)
#define SBX_CARD_TYPE_OTHER                           0
int card_type = SBX_CARD_TYPE_OTHER;

static int board_type = 0;

int failover_count = 0;

#define SBX_CHASSIS_TYPE_STANDALONE 0
#define SBX_CHASSIS_TYPE_MULTICARD  1

#define MODID_QE_OFFSET      10000
#define PL_FAB_UNIT_BM96     0

/* NOTE, that for supporting Polaris LC revs, we need to change these numbers
   so changing from #define's to int's */
#ifdef BCM_BM9600_SUPPORT
static int PL_QE0_MODID = 10023; /* connected to SI23 on BM96 */
static int PL_QE1_MODID = 10057; /* connected to SI57 on BM96 */
#endif
static int PL_FE0_MODID = 23;
static int PL_FE1_MODID = 25;    /* this is 57 modulo 32 */

#ifdef BCM_BM9600_SUPPORT
static int PL1_QE0_MODID = 10024; /* connected to SI24 on BM96 */
static int PL1_QE1_MODID = 10027; /* connected to SI26 on BM96 */
#endif
static int PL1_FE0_MODID = 24;
static int PL1_FE1_MODID = 27;

#ifdef BCM_BM9600_SUPPORT
static int sbx_appl_is_lgl_node_present = FALSE;
static int PL_FAB_LC0_QE0_NODE   =  68;
static int PL_FAB_LC0_QE1_NODE   =  71;
static int PL_FAB_LC0_QE0_MODID  =  (MODID_QE_OFFSET + 68);
static int PL_FAB_LC0_QE1_MODID  =  (MODID_QE_OFFSET + 71);
static int PL_FAB_LC1_QE0_NODE   =  56;
static int PL_FAB_LC1_QE1_NODE   =   6;
static int PL_FAB_LC1_QE0_MODID  =  (MODID_QE_OFFSET + 56);
static int PL_FAB_LC1_QE1_MODID  =  (MODID_QE_OFFSET + 6);
static int PL_FAB_LC2_QE0_NODE   =  46;
static int PL_FAB_LC2_QE1_NODE   =  16;
static int PL_FAB_LC2_QE0_MODID  =  (MODID_QE_OFFSET + 46);
static int PL_FAB_LC2_QE1_MODID  =  (MODID_QE_OFFSET + 16);
static int PL_FAB_LC3_QE0_NODE   =  34;
static int PL_FAB_LC3_QE1_NODE   =  24;
static int PL_FAB_LC3_QE0_MODID  =  (MODID_QE_OFFSET + 34);
static int PL_FAB_LC3_QE1_MODID  =  (MODID_QE_OFFSET + 24);
static int pl_modids[4][2];
static int sbx_pllc_slot_node_map[2][6] = {
    {-1, -1, 71,  6, 16, 24},
    {-1, -1, 68, 56, 46, 34}
};
#endif

#define FE2KXT_PL_FAB_LC0_QE0_NODE         24
#define FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC   32
#define FE2KXT_PL_FAB_LC0_QE0_MODID        (MODID_QE_OFFSET + FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC)
#ifdef BCM_BM9600_SUPPORT
#ifndef SV_QE2000_SIRIUS_SETUP
#define SV_QE2000_SIRIUS_SETUP
#endif
static int FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC  =  FE2KXT_PL_FAB_LC0_QE0_MODID;
#endif

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_BM9600_SUPPORT)
#define PL_SIRIUS_IPASS_NODE       (28)
#define PL_BCM56634_IPASS_MODID     (PL_SIRIUS_IPASS_NODE)
#define PL_SIRIUS_IPASS_MODID      (MODID_QE_OFFSET + PL_SIRIUS_IPASS_NODE)
static int PL_IPASS_SIRIUS_MODID = (PL_SIRIUS_IPASS_MODID);



#endif

#ifdef BCM_SIRIUS_SUPPORT
static bcm_gport_t qe2000_sfi_base_gport = 0;
static bcm_gport_t qe2000_sfi_end_gport = 0;
static bcm_gport_t sirius_sfi_base_gport = 0;
static bcm_gport_t sirius_sfi_end_fic_gport = 0;
static bcm_gport_t sirius_sfi_base_lcl_gport = 0;
static bcm_gport_t sirius_sfi_end_gport = 0;
#endif
#ifdef BCM_BM9600_SUPPORT
static bcm_gport_t polaris_sfi_sci_gport0 = 0;
static bcm_gport_t polaris_sfi_sci_gport1 = 0;
static bcm_gport_t polaris_sfi_base_gport0 = 0;
static bcm_gport_t polaris_sfi_end_gport0 = 0;
static bcm_gport_t polaris_sfi_base_gport1 = 0;
static bcm_gport_t polaris_sfi_end_gport1 = 0;
static bcm_gport_t polaris_sfi_base_gport2 = 0;
static bcm_gport_t polaris_sfi_end_gport2 = 0;
#endif
#ifdef BCM_SIRIUS_SUPPORT
bcm_port_congestion_config_t sirius_congestion_info1 = {
                                            /* flags */
    (BCM_PORT_CONGESTION_CONFIG_E2ECC | BCM_PORT_CONGESTION_CONFIG_TX),
    0,                                      /* port_bits */
    0,                                      /* packets_per_sec */
    0x1C00,                                 /* src_port */
    0,                                      /* multicast_id */
    1,                                      /* traffic_class */
    1,                                      /* color */
    0,                                      /* vlan */
    0,                                      /* packet priority */
    0,                                      /* packet cfi */
    { 0x00, 0x10, 0x18, 0x20, 0x30, 0x00 }, /* src_mac */
    { 0x00, 0x10, 0x18, 0x20, 0x30, 0x01 }, /* dest_mac */
    0xC0D0,                                 /* ethertype */
    0xE0F0                                  /* opcode */
};

typedef struct {
    int port;
    int cos;
    int mask;
} appl_fc_status_t;

appl_fc_status_t fc_status_config1[] =
{
    { BCM_GPORT_INVALID, 0, 0x04 },
    { BCM_GPORT_INVALID, 1, 0x01 },
    { BCM_GPORT_INVALID, 2, 0x02 },
    { BCM_GPORT_INVALID, 3, 0x02 }
};


/*
 * NOTE: This function should be invoked after other configuration is done.
 *       This is E2ECC configuration for one particular system configuration. 
 *       Additional E2ECC configurations have to be added for other
 *       system configurations.      
 */
int
sirius_fc_config1(int unit, int pp_modid, int qe_modid)
{
    bcm_port_congestion_config_t  congestion_info;
    bcm_gport_t port;
    int rv = BCM_E_NONE;
    int i;


    cli_out("Appl: Enabling E2ECC\n");  /* REMOVE */

    /* flow control mapping, front panel Queues to Fabric Egress queues */
    for (i = 0; i < sizeof(fc_status_config1)/sizeof(appl_fc_status_t); i++) {
        rv = bcm_cosq_gport_flow_control_set(unit,
                  fc_status_config1[i].port, fc_status_config1[i].cos, fc_status_config1[i].mask);
        if (rv != BCM_E_NONE) {
            cli_out("  bcm_cosq_gport_flow_control_set FAILED(%d, 0x%x)\n", rv, rv);
            return(rv);
        }
    }

    /* flow control message size */
    rv = bcm_fabric_congestion_size_set(unit, pp_modid, 64);
    if (rv != BCM_E_NONE) {
        cli_out("  bcm_fabric_congestion_size_set FAILED(%d, 0x%x)\n", rv, rv);
        return(rv);
    }

    /* HiGig flow control configuration */
    congestion_info = sirius_congestion_info1;
    BCM_GPORT_MODPORT_SET(port, qe_modid, 0);

    rv = bcm_port_congestion_config_set(unit, port, &congestion_info);
    if (rv != BCM_E_NONE) {
        cli_out("  bcm_port_congestion_config_set FAILED(%d, 0x%x)\n", rv, rv);
        return(rv);
    }

    return(rv);
}

#endif /* BCM_SIRIUS_SUPPORT */

#ifdef BCM_BM9600_SUPPORT
void
sbx_appl_polaris_system_update_topology(void)
{
    int module;
    int no_dev, no_slot;
    int chassis_type;


    chassis_type = soc_property_get(0, spn_DIAG_CHASSIS, 0);

    if (chassis_type == SBX_CHASSIS_TYPE_STANDALONE) {
        if ( (soc_ndev == 5) && (SOC_IS_SBX_BM9600(4)) ) {
           module = soc_property_port_get(4,  23, spn_SCI_PORT_MODID, -1);
           if (module != -1) {
               PL_QE0_MODID = module;
               SOC_SBX_NODE_FROM_MODID(module, PL_FE0_MODID);
               sbx_appl_is_lgl_node_present = TRUE;
           }

           module = soc_property_port_get(4,  57, spn_SCI_PORT_MODID, -1);
           if (module != -1) {
               PL_QE1_MODID = module;
               SOC_SBX_NODE_FROM_MODID(module, PL_FE1_MODID);
               sbx_appl_is_lgl_node_present = TRUE;
           }

           module = soc_property_port_get(4,  24, spn_SCI_PORT_MODID, -1);
           if (module != -1) {
               PL1_QE0_MODID = module;
               SOC_SBX_NODE_FROM_MODID(module, PL1_FE0_MODID);
               sbx_appl_is_lgl_node_present = TRUE;
           }

           module = soc_property_port_get(4,  27, spn_SCI_PORT_MODID, -1);
           if (module != -1) {
               PL1_QE1_MODID = module;
               SOC_SBX_NODE_FROM_MODID(module, PL1_FE1_MODID);
               sbx_appl_is_lgl_node_present = TRUE;
           }
        }
        else if ( (soc_ndev == 3) && (SOC_IS_SBX_BM9600(2)) ) {
           module = soc_property_port_get(2,  24, spn_SCI_PORT_MODID, -1);
           if (module != -1) {
               PL_QE0_MODID = module;
               SOC_SBX_NODE_FROM_MODID(module, PL_FE0_MODID);
               PL1_QE0_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL1_FE0_MODID);
               sbx_appl_is_lgl_node_present = TRUE;
           }
        }
    }

    if (chassis_type != SBX_CHASSIS_TYPE_STANDALONE) {
            module = soc_property_port_get(0,  68, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC0_QE0_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC0_QE0_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  71, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC0_QE1_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC0_QE1_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  56, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC1_QE0_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC1_QE0_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  6, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC1_QE1_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC1_QE1_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  46, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC2_QE0_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC2_QE0_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  16, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC2_QE1_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC2_QE1_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  34, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC3_QE0_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC3_QE0_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            module = soc_property_port_get(0,  24, spn_SCI_PORT_MODID, -1);
            if (module != -1) {
                PL_FAB_LC3_QE1_MODID = module;
                SOC_SBX_NODE_FROM_MODID(module, PL_FAB_LC3_QE1_NODE);
                sbx_appl_is_lgl_node_present = TRUE;
            }

            pl_modids[0][0] = PL_FAB_LC0_QE0_MODID;
            pl_modids[0][1] = PL_FAB_LC0_QE1_MODID;
            pl_modids[1][0] = PL_FAB_LC1_QE0_MODID;
            pl_modids[1][1] = PL_FAB_LC1_QE1_MODID;
            pl_modids[2][0] = PL_FAB_LC2_QE0_MODID;
            pl_modids[2][1] = PL_FAB_LC2_QE1_MODID;
            pl_modids[3][0] = PL_FAB_LC3_QE0_MODID;
            pl_modids[3][1] = PL_FAB_LC3_QE1_MODID;

            sbx_pllc_slot_node_map[0][2] = PL_FAB_LC0_QE1_NODE;
            sbx_pllc_slot_node_map[0][3] = PL_FAB_LC1_QE1_NODE;
            sbx_pllc_slot_node_map[0][4] = PL_FAB_LC2_QE1_NODE;
            sbx_pllc_slot_node_map[0][5] = PL_FAB_LC3_QE1_NODE;
            sbx_pllc_slot_node_map[1][2] = PL_FAB_LC0_QE0_NODE;
            sbx_pllc_slot_node_map[1][3] = PL_FAB_LC1_QE0_NODE;
            sbx_pllc_slot_node_map[1][4] = PL_FAB_LC2_QE0_NODE;
            sbx_pllc_slot_node_map[1][5] = PL_FAB_LC3_QE0_NODE;
    }

    if (chassis_type == SBX_CHASSIS_TYPE_STANDALONE) {
        if ( (soc_ndev == 5) && (SOC_IS_SBX_BM9600(4)) ) {
            cli_out("\nPolaris Chassis Configuration\n");
            cli_out("Standalone Rev0, QE0: %d, FE0: %d, QE1: %d FE1: %d\n",
                    PL_QE0_MODID, PL_FE0_MODID, PL_QE1_MODID, PL_FE1_MODID);

            cli_out("Standalone Rev1, QE0: %d, FE0: %d, QE1: %d FE1: %d\n",
                    PL1_QE0_MODID, PL1_FE0_MODID, PL1_QE1_MODID, PL1_FE1_MODID);
        }
    }

   if (chassis_type == SBX_CHASSIS_TYPE_STANDALONE) {
        if ( (soc_ndev == 3) && (SOC_IS_SBX_BM9600(2)) ) {
            cli_out("\nPolaris FE2KXT Configuration\n");
            cli_out("Standalone Rev0, QE0: %d, FE0: %d\n",
                    PL_QE0_MODID, PL_FE0_MODID);

            cli_out("Standalone Rev1, QE0: %d, FE0: %d\n",
                    PL1_QE0_MODID, PL1_FE0_MODID);
        }
    }
    if (chassis_type != SBX_CHASSIS_TYPE_STANDALONE) {
        if ( (soc_ndev == 1) && (SOC_IS_SBX_BM9600(0)) ) {
            cli_out("Chassis, LC0_QE0: %d(%d), LC0_QE1: %d(%d)\n",
                    PL_FAB_LC0_QE0_MODID, PL_FAB_LC0_QE0_NODE, PL_FAB_LC0_QE1_MODID, PL_FAB_LC0_QE1_NODE);

            cli_out("Chassis, LC1_QE0: %d(%d), LC1_QE1: %d(%d)\n",
                    PL_FAB_LC1_QE0_MODID, PL_FAB_LC1_QE0_NODE, PL_FAB_LC1_QE1_MODID, PL_FAB_LC1_QE1_NODE);

            cli_out("Chassis, LC2_QE0: %d(%d), LC2_QE1: %d(%d)\n",
                    PL_FAB_LC2_QE0_MODID, PL_FAB_LC2_QE0_NODE, PL_FAB_LC2_QE1_MODID, PL_FAB_LC2_QE1_NODE);

            cli_out("Chassis, LC3_QE0: %d(%d), LC3_QE1: %d(%d)\n",
                    PL_FAB_LC3_QE0_MODID, PL_FAB_LC3_QE0_NODE, PL_FAB_LC3_QE1_MODID, PL_FAB_LC3_QE1_NODE);

            for (no_slot = 0; no_slot < 4; no_slot++) {
                cli_out("Chassis, Slot%d, ", no_slot);
                for (no_dev = 0; no_dev < 2; no_dev++) {
                    cli_out("Dev%d: %d ", no_dev, pl_modids[no_slot][no_dev]);
                }
                cli_out("\n");
            }

            for (no_dev = 0; no_dev < 2; no_dev++) {
                cli_out("Chassis, Dev%d, ", ((no_dev == 0) ? 1 : 0));
                for (no_slot = 2; no_slot < 6; no_slot++) {
                    cli_out("Slot%d: %d ", no_slot, sbx_pllc_slot_node_map[no_dev][no_slot]);
                }
                cli_out("\n");
            }
        }
    }

    cli_out("\n");
}
#endif /* BCM_BM9600_SUPPORT */


cmd_result_t cmd_sbx_mclcinit_table_config(int unit, args_t *a);
cmd_result_t cmd_sbx_mcfabinit_tbl_config(int unit, args_t *a);
soc_sbx_chip_info_t* sbx_chip_info_add(int chip_id, char *dev);


/*
 * Following tables of commands used to configure metrocore and Polaris line cards.
 * Picking up info from config.bcm not used since some data overridden by defaults.
 *  Rules for configuring:
 *  - Polaris card requires module id of QEs to be the serdes port that they connect to
 *  - Crossbar tables reside in Polaris, if present, else in QEs
 */

typedef struct {
    int cmd;
    int unit;
    int arg1;
    int arg2;
    int arg3;
    int arg4;
    int arg5;
    int arg6;
} dev_cmd_rec;

cmd_result_t
cmd_sbx_card_config(int unit, int node_id, int node_id1, int link_enable, dev_cmd_rec cmd_tbl[]);

#define MAX_CMDS          1000

#define PORT_ENABLE                 0
#define SET_MODID                   1
#define XBAR_CONNECT_SET            2
#define FAB_CONTROL_SET             3
#define XBAR_ENABLE_SET             4
#define MODULE_ENABLE_SET           5
#define REDUND_REGISTER             6
#define PORT_ENABLE_RANGE           7
#define PORT_ENABLE_ITER            8
#define PORT_SPEED_SET              9
#define PORT_SPEED_RANGE           10
#define PORT_CONTROL               11
#define XBAR_CONNECT_RANGE         12
#define LCM_BM3200_XCFG_INIT       13
#define LCM_MODE_SET               14
#define LCM_BM9600_XCFG_SET        15
#define PORT_CONTROL_RANGE         16
#define SET_MODULE_PROTOCOL        17
#define XBAR_CONNECT_ADD           18
#define XBAR_CONNECT_UPDATE        19
#define SET_LGL_MODID              20
#define SET_LGL_MODULE_PROTOCOL    21
#define XBAR_LGL_CONNECT_ADD       22
#define LGL_MODULE_ENABLE_SET      23
#define LCM_BM9600_XCFG_GET        24
#define PORT_EQUALIZATION          25
#define PORT_ENABLE_SI             26
#define PORT_ENABLE_SI_RANGE       27
#define PORT_ENABLE_HYPERCORE      28
#define PORT_ENABLE_HYPERCORE_RANGE 29
#define LGL_XBAR_MAPPING_SET       30
#define GPORT_CONTROL_RANGE        31
#define GPORT_CONTROL              32
#define SWITCH_EVENT_REGISTER      33

#define CMD_CARD_ALL 0                       /* means that no filter should be used */
#define CMD_CARD_PLLC_REV0 0x1               /* command should only run on rev0 PL LC */
#define CMD_CARD_PLLC_REV1 0x2               /* command should only run on rev1 PL LC */
static int _cmd_card_filter = CMD_CARD_ALL;

/* diag_fabric_mode = 0 -> 1P1
   diag_fabric_mode = 1 -> Load Share */
#define CMD_FABRIC_MODE_1P1 0x0  /* A/B used to either one fabric card or the other */
#define CMD_FABRIC_MODE_LS  0x2  /* both fabric cards used */

#define MCLC_UNIT_QE     0
#define MCLC_UNIT_FE     1
#define MCLC_UNIT_LCM0   2
#define MCLC_UNIT_LCM1   3
#define MCLC_MODID_QE    10000
#define MCLC_MODID_FE    0
#define MODID_QE_OFFSET    10000

#define SS_IPASS_UNIT  0
#define PL_IPASS_UNIT  0

#define AUTO_MODID_QE0        -1
#define AUTO_MODID_QE1        -2
#define AUTO_MODID_FE0        -3
#define AUTO_MODID_FE1        -4

#define PLLC_UNIT_QE0     0
#define PLLC_UNIT_FE0     1
#define PLLC_UNIT_FE1     2
#define PLLC_UNIT_QE1     3
#define PLLC_UNIT_BM96    4
#define PLLC_UNIT_BM96_FE2KXT_LC 2


/* #define _PL_FAB_NO_QE1_  1 */
/* indexes are  logcial xbar, node number */
#define _XCFG_MAX_XBARS 32
#define _XCFG_MAX_NODES 72
static int _xcfg_remap[_XCFG_MAX_XBARS][_XCFG_MAX_NODES];

#undef SBX_DEBUG_CONNECTION_SET
#ifdef SBX_DEBUG_CONNECTION_SET
static int _xcfg_remap_read;
#endif

/*
 * Start, Sirius Model related setup/configuration
 *
 * Following is an example configuration and the associated
 * internal mappings
 *
 * NOTE: Sysport for local queues has a base of ((4K - 1) - 512)
 *
 *
 * - SOC properties
 *       - if_subports.0 = 1 (HG0 interface has 1 port)
 *       - if_subports.1 = 1 (HG1 interface has 1 port)
 *       - if_subports.2 = 1 (HG2 interface has 1 port)
 *       - if_subports.3 = 1 (HG3 interface has 1 port)
 *
 * - The gports for the above have the following encoding
 *       - HG0 interface port 1, ModId: 10000, Port 0
 *       - HG1 interface port 1, ModId: 10000, Port 1
 *       - HG2 interface port 1, ModId: 10000, Port 2
 *       - HG3 interface port 1, ModId: 10000, Port 3
 *
 * - The queue groups going to the above GPORTS have base queue and sysport of
 *   Only a single queue group per port
 *       - HG0 interface port 1, Base Queue 0,  Sysport: LocalBase + 0
 *       - HG1 interface port 1, Base Queue 8,  Sysport: LocalBase + 1
 *       - HG2 interface port 1, Base Queue 16, Sysport: LocalBase + 2
 *       - HG3 interface port 1, Base Queue 24, Sysport: LocalBase + 3
 *
 * - On the Egress the FIFO base for the above queue groups is
 *       - HG0 interface port 1, FIFO Base 0 ((port << 2) + 0),
 *                               Port 0, all queues using Class 0 and thus FIFO 0 (uc, nef class)
 *       - HG1 interface port 1, FIFO Base 4 ((port << 2) + 0),
 *                               Port 1, all queues using class 0 and thus FIFO 4 (uc. nef class)
 *       - HG2 interface port 1, FIFO Base 8 ((port << 2) + 0),
 *                               Port 2, all queues using class 0 and thus FIFO 8 (uc, nef class)
 *       - HG3 interface port 1, FIFO Base 12 ((port << 2) + 0),
 *                               Port 3, all queues using class 0 and thus FIFO 12 (uc, nef class)
 *
 */
#define MODEL_SIRIUS_MODID          (BCM_MODULE_FABRIC_BASE + 0)
#define MODEL_BCM56634_MODID         (0)
#define MODEL_BCM56634_CPU_PORT      (0)
#define MODEL_FE2KXT_CPU_PORT        31

#define MODEL_SIRIUS_UNIT           (0)
#define MODEL_BCM56634_UNIT          (1)

#define _MODEL_SIRIUS_SUPPORT_      1
/* #define _BCM56634_MODEL_             0 */

#define _SIRUS_MODEL_MULTICAST_

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_BM9600_SUPPORT)


#define MCAST_TRUNK 1
#define MCAST_TRUNK_8FIFO 0
#if MCAST_TRUNK_8FIFO
#define _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT 8
#else
#define _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT 4
#endif

/* this function is used to setup ports/queues for sirius/polaris FIC mode single node test */
cmd_result_t
cmd_sbx_sirius_ipass_config(int unit, args_t *a, int sirius_modid)
{
    cmd_result_t rc = CMD_OK;
    bcm_error_t bcm_err = BCM_E_NONE;
    int no_ports, intf, port;
    int no_cos = 0, flags = 0;
    int hg_subports[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    bcm_gport_t intf_gport[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    bcm_gport_t child_gport[132];
    bcm_gport_t child_egress_gport[132];
    bcm_gport_t unicast_gport[132];
    bcm_gport_t intf_cpu_gport = BCM_GPORT_INVALID;
    bcm_pbmp_t pbmp;
    bcm_fabric_distribution_t ds_id;
    bcm_gport_t multicast_gport = 0;
#ifdef BCM_SIRIUS_SUPPORT
    bcm_multicast_t mcgroup;
    bcm_gport_t switch_port, fabric_port;
#endif
    int cpu_port;
#if MCAST_TRUNK 
    bcm_trunk_t tid;
    bcm_trunk_info_t  ta_info;
    bcm_trunk_member_t member_array[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    bcm_trunk_chip_info_t ta_chip_info;
    int i, no_higig;
    bcm_gport_t egr_intf_gport = 0;
    bcm_gport_t mcSchGport0 = 0, mcSchGport1 = 0;
    bcm_gport_t mcEgrFabricPort = 0, mcEgrGport = 0;
#endif /* MCAST_TRUNK */
    cli_out("Appl: cmd_sbx_sirius_ipass_config()\n");

    sal_memset(intf_gport, 0, sizeof(bcm_gport_t) * SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS);
    sal_memset(child_gport, 0, sizeof(bcm_gport_t) * 132);
    sal_memset(child_egress_gport, 0, sizeof(bcm_gport_t) * 132);
    sal_memset(unicast_gport, 0, sizeof(bcm_gport_t) * 132);

    /* read SOC properties to determine the number of ports/channels on each */
    /* interface                                                    */
    for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
        hg_subports[intf] = soc_property_port_get(unit, intf, spn_IF_SUBPORTS, 0);
    }
    for (intf = 0, no_ports = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
        no_ports += hg_subports[intf];
    }

    if (no_ports > 128) {
        return CMD_OK;
    }

    /* setup HG interface gports. The port number is to be derived from "pbmp" */
    BCM_PBMP_ASSIGN(pbmp, PBMP_CMIC(unit));
    BCM_PBMP_OR(pbmp, PBMP_HG_ALL(unit));
    if (soc_property_get(unit, spn_PBMP_XPORT_XE, 0)) {
        BCM_PBMP_OR(pbmp, PBMP_XE_ALL(unit));
    } 
    if (soc_property_get(unit, spn_PBMP_XPORT_GE, 0)) {
        BCM_PBMP_OR(pbmp, PBMP_GE_ALL(unit));
    }
    PBMP_ITER(pbmp, port) {
        if (IS_HG_PORT(unit, port) || 
            (IS_XE_PORT(unit, port) && soc_property_get(unit, spn_PBMP_XPORT_XE, 0)) ||
            (IS_GE_PORT(unit, port) && soc_property_get(unit, spn_PBMP_XPORT_GE, 0))) {
            if (port >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) {
                /* ignore fabric side Xport for now */
                continue;
            }
            BCM_GPORT_MODPORT_SET(intf_gport[port], sirius_modid, port);
        } else if (IS_CPU_PORT(unit, port)) {
            BCM_GPORT_MODPORT_SET(intf_cpu_gport, sirius_modid, port);
        }
    }

    if (board_type == BOARD_TYPE_SIRIUS_IPASS) {
#ifdef BCM_SIRIUS_SUPPORT
        /* configure module id */
        bcm_err = bcm_stk_modid_set(MODEL_SIRIUS_UNIT, sirius_modid);
        if (BCM_FAILURE(bcm_err)) {
            cli_out("bcm_stk_modid_set() failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
            return(CMD_FAIL);
        }
#endif
    }

    if (soc_property_get(MODEL_SIRIUS_UNIT, spn_IF_SUBPORTS_CREATE, 0)) {
        /* setup the child gports */
        for (port = 0; port < no_ports; port++) {
            BCM_GPORT_CHILD_SET(child_gport[port], sirius_modid, port);
            BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[port], sirius_modid, port);
        }
        /* last port is CPU port */
        cpu_port = port;
        BCM_GPORT_CHILD_SET(child_gport[cpu_port], PL_SIRIUS_IPASS_MODID, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
        BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[cpu_port], PL_SIRIUS_IPASS_MODID, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
    } else {
        /* last port is CPU port */
        cpu_port = no_ports;
        BCM_GPORT_CHILD_SET(child_gport[cpu_port], PL_SIRIUS_IPASS_MODID, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
        BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[cpu_port], PL_SIRIUS_IPASS_MODID, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
    }

    if ( (board_type == BOARD_TYPE_POLARIS_IPASS) && SOC_SBX_CFG(unit)->bHybridMode ) {
	/* NOTE: this is assuming the requeue ports are created right after the local ports */
	/* hybrid mode grant the requeue ports */
        for (port = 0; port < no_ports; port++) {
	    child_gport[port] += no_ports;
	    child_egress_gport[port] += no_ports;
        }
	/* CPU requeue port */
	child_gport[port] += (2 * no_ports - SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
	child_egress_gport[port] += (2 * no_ports - SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
    }

#ifdef BCM_SIRIUS_SUPPORT
    /* configure switch port to fabric port mapping, 1-to-1 mapping */
    for (port = 0; port < no_ports; port++) {
      if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
        BCM_GPORT_MODPORT_SET(switch_port, PL_BCM56634_IPASS_MODID, (port+1));
      } else {
        BCM_GPORT_MODPORT_SET(switch_port, PL_BCM56634_IPASS_MODID, port);
      }
      
      BCM_GPORT_CHILD_SET(fabric_port, sirius_modid, port);
      
        bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
        if (BCM_FAILURE(bcm_err)) {
            cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
            return(CMD_FAIL);
        }
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(unit,
                                "Mapping, Switch Gport: 0x%x, Fabric Port: 0x%x\n"),
                     switch_port, fabric_port));
    }
    cpu_port = port;
    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	BCM_GPORT_MODPORT_SET(switch_port, PL_BCM56634_IPASS_MODID, MODEL_BCM56634_CPU_PORT);
    } else {
	BCM_GPORT_MODPORT_SET(switch_port, PL_BCM56634_IPASS_MODID, MODEL_FE2KXT_CPU_PORT);
    }
    BCM_GPORT_CHILD_SET(fabric_port, sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);

    bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("bcm_stk_fabric_map_set failed for CPU port err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
        return(CMD_FAIL);
    }
    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                (BSL_META_U(unit,
                            "Mapping, Switch Gport: 0x%x, Fabric Port: 0x%x\n"),
                 switch_port, fabric_port));
#endif
#if MCAST_TRUNK
    /* If required configure HiGig Trunking */
    if ((!SOC_IS_SBX_BME(unit)) &&
	(SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS)) {
	cli_out("Appl: Configuring HiGig Trunking, unit=%d\n", unit);
	
	bcm_err = bcm_trunk_chip_info_get(unit, &ta_chip_info);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_trunk_chip_info_get(), Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }
        tid = ta_chip_info.trunk_fabric_id_min;
	
        bcm_err = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_trunk_create(), Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }
	
	BCM_GPORT_EGRESS_MODPORT_SET(egr_intf_gport, 
				     BCM_GPORT_MODPORT_MODID_GET(intf_gport[0]), 
				     BCM_GPORT_MODPORT_PORT_GET(intf_gport[0])); 
	
	bcm_err = bcm_cosq_gport_add(unit, egr_intf_gport, 1, BCM_COSQ_GPORT_SCHEDULER, &mcSchGport0);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_gport_add() Egress scheduler0, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	
	bcm_err = bcm_cosq_gport_attach(unit, egr_intf_gport, mcSchGport0, -1);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_gport_attach() Egress scheduler1, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	
	bcm_err = bcm_cosq_gport_add(unit, egr_intf_gport, 1, BCM_COSQ_GPORT_SCHEDULER, &mcSchGport1);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_gport_add() Egress scheduler1, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	
	bcm_err =  bcm_cosq_gport_attach(unit, mcSchGport0, mcSchGport1, -1);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_gport_attach() Egress scheduler1, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	
	bcm_err = bcm_fabric_port_create(unit,egr_intf_gport,-1,BCM_FABRIC_PORT_EGRESS_MULTICAST, &mcEgrFabricPort);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_fabric_port_create() Egress Multicast Port, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	
	bcm_err = bcm_cosq_gport_add(unit, mcEgrFabricPort, _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT, BCM_COSQ_GPORT_EGRESS_GROUP, &mcEgrGport);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_gport_ad() Egress Multicast Group, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	    
	bcm_err = bcm_cosq_gport_attach(unit, mcSchGport1, mcEgrGport, -1);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_gport_attach() Egress Multicast Group, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}

#if (8 == _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT)
	bcm_err = bcm_cosq_control_set (unit, BCM_GPORT_INVALID, 3, bcmCosqControlClassMap, BCM_COS_MULTICAST_EF);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_control_set bcmCosqControlClassMap EF, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
	    
	bcm_err = bcm_cosq_control_set (unit, BCM_GPORT_INVALID, 6, bcmCosqControlClassMap, BCM_COS_MULTICAST_NON_EF);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Appl: Error bcm_cosq_control_set bcmCosqControlClassMap NON_EF, Err=0x%x\n", bcm_err);
	    return(CMD_FAIL);
	}
#endif /* (8 == _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT) */

	bcm_err = bcm_cosq_control_set (unit, mcEgrGport, 0, bcmCosqControlFabricPortScheduler, mcEgrFabricPort);

        sal_memset(&ta_info, 0, sizeof(ta_info));
        for (i = 0, no_higig = 0; i < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; i++) {
            if (intf_gport[i] == 0) {
                continue;
            }
	    
	    if (bcm_err != BCM_E_NONE){
		cli_out("Appl: Error bcm_cosq_control_set() Multicast Fabric Port to Scheduler, Err=0x%x\n", bcm_err);
		return(CMD_FAIL);
	    }
	    
            bcm_trunk_member_t_init(&member_array[no_higig]);
            member_array[no_higig].gport = intf_gport[i]; 
            no_higig++;
	}	    
	
        bcm_err = bcm_trunk_set(unit, tid, &ta_info, no_higig, member_array);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_trunk_set(), Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }
    }
#endif /* MCAST_TRUNK */
    /* setup unicast logical ports/queue group for each of the ports/channels */
    if (SOC_SBX_CFG(unit)->bHybridMode ) {
	/* setup unicast logical ports/queue group for each of the ports/channels */
	no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 8);
	flags = 0;
	for (port = 0; port < (no_ports); port++) {
	    LOG_VERBOSE(BSL_LS_APPL_COMMON,
	                (BSL_META_U(unit,
	                            "Queue Group, Fabric Port: 0x%x\n"),
	                 child_gport[port]));
	    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, child_gport[port],
					 no_cos, flags,  &unicast_gport[port]);
	    if (bcm_err != BCM_E_NONE){
		cli_out("Error adding queue group for phys_gport=0x%x\n", child_gport[port]);
		return(CMD_FAIL);
	    }
	}

    } else {
	if (soc_property_get(unit, spn_DIAG_COSQ_INIT,0) == 1) {
	    /* setup unicast logical ports/queue group for each of the ports/channels */
	    no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 8);
	    flags = 0;
	    if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
		for (port = 0; port <= no_ports; port++) {
		    LOG_VERBOSE(BSL_LS_APPL_COMMON,
		                (BSL_META_U(unit,
		                            "Queue Group, Fabric Port: 0x%x\n"),
		                 child_gport[port]));
		    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, child_gport[port],
						 no_cos, flags,  &unicast_gport[port]);
		    if (bcm_err != BCM_E_NONE){
			cli_out("Error adding queue group for phys_gport=0x%x\n", child_gport[port]);
			return(CMD_FAIL);
		    }
		}
	    } else {
                if (!soc_property_get(MODEL_SIRIUS_UNIT, spn_IF_SUBPORTS_CREATE, 0)) {
		    bcm_err = bcm_fabric_port_create(MODEL_SIRIUS_UNIT, intf_cpu_gport, 0, flags, &child_gport[cpu_port]);
		    if (BCM_FAILURE(bcm_err)) {
			cli_out("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
			return(CMD_FAIL);
		    }
		}
		bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, child_egress_gport[cpu_port],
					     no_cos, flags,  &unicast_gport[cpu_port]);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error adding queue group for phys_gport=0x%x\n", child_egress_gport[cpu_port]);
		    return(CMD_FAIL);
		}
	    }

	} else {
	    if (!soc_property_get(MODEL_SIRIUS_UNIT, spn_IF_SUBPORTS_CREATE, 0)) {
		bcm_err = bcm_fabric_port_create(MODEL_SIRIUS_UNIT, intf_cpu_gport, 0, flags, &child_gport[cpu_port]);
                if (BCM_FAILURE(bcm_err)) {
                    cli_out("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                    return(CMD_FAIL);
                }
		bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, child_egress_gport[cpu_port],
					     no_cos, flags,  &unicast_gport[cpu_port]);
		if (bcm_err != BCM_E_NONE){
		  cli_out("Error adding queue group for phys_gport=0x%x\n", child_egress_gport[cpu_port]);
		  return(CMD_FAIL);
		}
	    }
	} /* diag_cosq_init=1 */
    }

    if (soc_property_get(unit, spn_DIAG_COSQ_INIT,0) == 1){
	/* multicast ports */
	/* setup multicast logical port - create queue group first */
	ds_id = 0;
	multicast_gport = 0x30000008;
	flags = 0; /* to assign multicast gport, change flags to 1 */
	bcm_err = bcm_cosq_fabric_distribution_add(MODEL_SIRIUS_UNIT, ds_id, no_cos,
						   flags, &multicast_gport);
	if (bcm_err != BCM_E_NONE) {
	    cli_out("Error creating multicast fabric distribution queue group error(%d)\n", bcm_err);
	    return(CMD_FAIL);
	}
	
	if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
	    /* create multicast group 1 */
	    mcgroup = 1;
	    cli_out("Appl: Creating MulticastGroup: 0x%x\n", mcgroup);
	    bcm_err = bcm_multicast_create(MODEL_SIRIUS_UNIT, BCM_MULTICAST_WITH_ID, &mcgroup);
	    if (bcm_err != BCM_E_NONE) {
		cli_out("Error allocating multicast group error(%d)\n", bcm_err);
		return(CMD_FAIL);
	    }
	    
	    if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
		/* add all ports to the multicast group */
		cli_out("Appl: Adding ports to MulticastGroup: 0x%x\n", mcgroup);
		for (port = 0; port < no_ports; port++) {
		    bcm_err = bcm_multicast_egress_add(MODEL_SIRIUS_UNIT, mcgroup,
						       child_gport[port], 0 /* encap_id */);
		    if (bcm_err != BCM_E_NONE) {
			cli_out("Error adding ports to the multicast group error(%d)\n", bcm_err);
			return(CMD_FAIL);
		    }
		}
	    }
	    
	    if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
            /* Associate mcgroup with queue group */
		cli_out("Appl: Associating MulticastGroup: 0x%x with ds_id: 0x%x\n", mcgroup, ds_id);
		bcm_err = bcm_multicast_fabric_distribution_set(MODEL_SIRIUS_UNIT, mcgroup, ds_id);
		
		if (bcm_err != BCM_E_NONE) {
		    cli_out("Error associating multicast group with ds_id\n");
		}
	    }
#endif
	}
    } /* diag_cosq_init=1 */
    return(rc);
}

/* this function is used to setup ports for sirius/polaris FIC mode qe2k interop multi-node test
 *   Note that this function will only make sure child gports are created, and will leave queue 
 *   creation to testing environment. For now, We are only supporting sirius connect to SBX device,
 *   and not issueing the fabric port to front panel port mapping APIs
 */
cmd_result_t
cmd_sbx_sirius_ipass_chassis_config(int unit, args_t *a, int sirius_modid)
{
    cmd_result_t rc = CMD_OK;

    if (!soc_property_get(MODEL_SIRIUS_UNIT, spn_IF_SUBPORTS_CREATE, 0)) {
	cli_out("Error: require if_subports_create=1 in config.bcm\n");
	return(CMD_FAIL);	
    }

    return(rc);
}
#endif


#ifdef BCM_SIRIUS_SUPPORT
/* this function is used to setup ports/queues for sirius/polaris TME/Hybrid mode */
cmd_result_t
cmd_sbx_sirius_tme_config(int unit, args_t *a, int sirius_modid)
{
    cmd_result_t rc = CMD_OK;
    bcm_error_t bcm_err = BCM_E_NONE;
    int no_ports = 0, intf, port, intf_port, port_offset = 0;
    bcm_gport_t switch_port, fabric_port;
    int no_cos, flags = 0;
    int no_levels_to_alloc;
    static int hg_subports[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    static bcm_gport_t intf_gport[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    static bcm_gport_t child_gport[132];
    static bcm_gport_t child_egress_gport[132];
    static bcm_gport_t egress_group_gport[132];
    static bcm_gport_t unicast_gport[132];
    static bcm_gport_t rq_child_gport[132];
    static bcm_gport_t rq_child_egress_gport[132];
    static bcm_gport_t rq_unicast_gport[132];
    static bcm_gport_t intf_cpu_gport = -1;
    static bcm_gport_t prev_sched_gport, sched_gport, no_levels;
    bcm_pbmp_t pbmp;
    int cpu_port;
    int rq_port, mc_intf_gport = -1, rq_intf_gport = -1;
    int bcm56634_modid, bcm56634_req_modid;

#ifdef _SIRUS_MODEL_MULTICAST_
    static bcm_gport_t multicast_gport;
    static bcm_gport_t multicast_child_gport;
    static bcm_gport_t multicast_sched_gport;
    bcm_fabric_distribution_t ds_id;
    bcm_multicast_t mcgroup;
    bcm_gport_t egr_intf_gport = 0;
    bcm_gport_t mcSchGport0 = 0, mcSchGport1 = 0;
    bcm_gport_t mcEgrFabricPort = 0, mcEgrGport = 0;
#if MCAST_TRUNK
    bcm_trunk_t tid;
    bcm_trunk_info_t  ta_info;
    bcm_trunk_member_t member_array[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    bcm_trunk_chip_info_t ta_chip_info;
    int i, no_higig;
#endif /* MCAST_TRUNK */
#endif /* _SIRIUS_MODEL_MULTICAST_ */

    sal_memset(&child_gport,-1,sizeof(bcm_gport_t)*132);
    /* read SOC properties to determine the number of ports/channels on each */
    /* interface                                                    */
    for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
        hg_subports[intf] = soc_property_port_get(unit, intf, spn_IF_SUBPORTS, 0);
	no_ports += hg_subports[intf];
    }

    if (no_ports > 128) {
        return CMD_OK;
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
	/* use the fic modid for ingress traffic, and tme modid for requeue traffic */
	sirius_modid = PL_SIRIUS_IPASS_MODID;
	bcm56634_modid = PL_BCM56634_IPASS_MODID;
	bcm56634_req_modid = MODEL_BCM56634_MODID;
    } else {
	bcm56634_modid = sirius_modid - BCM_MODULE_FABRIC_BASE;
	bcm56634_req_modid = bcm56634_modid;
    }

    /* setup HG interface gports. The port number is to be derived from "pbmp" */
    BCM_PBMP_ASSIGN(pbmp, PBMP_CMIC(unit));
    BCM_PBMP_OR(pbmp, PBMP_HG_ALL(unit));
    if (soc_property_get(unit, spn_PBMP_XPORT_XE, 0)) {
        BCM_PBMP_OR(pbmp, PBMP_XE_ALL(unit));
    } 
    if (soc_property_get(unit, spn_PBMP_XPORT_GE, 0)) {
        BCM_PBMP_OR(pbmp, PBMP_GE_ALL(unit));
    }
    PBMP_ITER(pbmp, port) {
        if (IS_HG_PORT(unit, port) || 
            (IS_XE_PORT(unit, port) && soc_property_get(unit, spn_PBMP_XPORT_XE, 0)) ||
            (IS_GE_PORT(unit, port) && soc_property_get(unit, spn_PBMP_XPORT_GE, 0))) {
            if (port >= SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS) {
                /* ignore fabric side Xport for now */
                continue;
            }
            BCM_GPORT_MODPORT_SET(intf_gport[port], sirius_modid, port);
        } else if (IS_CPU_PORT(unit, port)) {
            BCM_GPORT_MODPORT_SET(intf_cpu_gport, sirius_modid, port);
        }
    }

    /* use last req interface port */
    PBMP_REQ_ITER(unit, rq_port) {
	BCM_GPORT_MODPORT_SET(mc_intf_gport, sirius_modid, rq_port);
	BCM_GPORT_MODPORT_SET(rq_intf_gport, sirius_modid, rq_port);
    }
    if (SAL_BOOT_BCMSIM) {
	/* multicast ingress tree attach it to cpu interface */
	mc_intf_gport = intf_cpu_gport;
    }

    /* configure module id */
    bcm_err = bcm_stk_modid_set(MODEL_SIRIUS_UNIT, sirius_modid);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("bcm_stk_modid_set() failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
        return(CMD_FAIL);
    }

    if (soc_property_get(MODEL_SIRIUS_UNIT, spn_IF_SUBPORTS_CREATE, 0)) {
	/* setup the child gports */
	for (port = 0; port < no_ports; port++) {
	    BCM_GPORT_CHILD_SET(child_gport[port], sirius_modid, port);
	    BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[port], sirius_modid, port);
	    BCM_GPORT_EGRESS_GROUP_SET(egress_group_gport[port], sirius_modid, port);
	}
	/* last port is CPU port */
	cpu_port = port;
	BCM_GPORT_CHILD_SET(child_gport[cpu_port], sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
	BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[cpu_port], sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);

	LOG_VERBOSE(BSL_LS_APPL_COMMON,
	            (BSL_META_U(unit,
	                        "ports created at init bcm time\n")));
    } else {
        /* setup the child gports */
        flags = 0;
        port = 0;
        for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
            for (port_offset = 0; port_offset < hg_subports[intf]; port_offset++, port++) {
                bcm_err = bcm_fabric_port_create(MODEL_SIRIUS_UNIT, intf_gport[intf], port_offset, flags, &child_gport[port]);
                if (BCM_FAILURE(bcm_err)) {
                    cli_out("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                    return(CMD_FAIL);
                } else {
		    if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
			LOG_VERBOSE(BSL_LS_APPL_COMMON,
			            (BSL_META_U(unit,
			                        "created port %d for higig %d offset %d, handle (0x%x)\n"),
			             port, intf, port_offset, child_gport[port]));
		    }

                    BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[port], BCM_GPORT_CHILD_MODID_GET(child_gport[port]),
                                               BCM_GPORT_CHILD_PORT_GET(child_gport[port]));
                }
            }
        }

        /* last port is CPU port */
        cpu_port = port;
	if (!SOC_SBX_CFG(unit)->fabric_egress_setup) {
	    BCM_GPORT_CHILD_SET(child_gport[cpu_port], sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
	}
        bcm_err = bcm_fabric_port_create(MODEL_SIRIUS_UNIT, intf_cpu_gport, 0, flags, &child_gport[cpu_port]);
        if (BCM_FAILURE(bcm_err)) {
            cli_out("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
            return(CMD_FAIL);
        } else {
	    LOG_VERBOSE(BSL_LS_APPL_COMMON,
	                (BSL_META_U(unit,
	                            "created port for cpu, handle (0x%x)\n"),
	                 child_gport[cpu_port]));
            BCM_GPORT_EGRESS_CHILD_SET(child_egress_gport[cpu_port], BCM_GPORT_CHILD_MODID_GET(child_gport[cpu_port]),
                                       BCM_GPORT_CHILD_PORT_GET(child_gport[cpu_port]));
        }
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
	/* create requeue ports for hybrid */
	for (port = 0; port <= no_ports; port++) {
	    bcm_err = bcm_fabric_port_create(MODEL_SIRIUS_UNIT, rq_intf_gport, port_offset, 
					     flags, &rq_child_gport[port]);
	    if (BCM_FAILURE(bcm_err)) {
		cli_out("bcm_fabric_port_create failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		return(CMD_FAIL);
	    } else {
		if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
		    LOG_VERBOSE(BSL_LS_APPL_COMMON,
		                (BSL_META_U(unit,
		                            "created port %d for requeue offset %d, handle (0x%x)\n"),
		                 port, port, rq_child_gport[port]));
		}
		BCM_GPORT_EGRESS_CHILD_SET(rq_child_egress_gport[port], BCM_GPORT_CHILD_MODID_GET(rq_child_gport[port]),
					   BCM_GPORT_CHILD_PORT_GET(rq_child_gport[port]));
	    }
	}

	/* configure switch port to fabric port mapping ingress traffic */
	    for (port = 0; port < no_ports; port++) {
	      if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	        /* +1: on bcm56634, front panel ports start from 2 */
	        BCM_GPORT_MODPORT_SET(switch_port, bcm56634_modid, (port + 1));
	      } else {
		BCM_GPORT_MODPORT_SET(switch_port, bcm56634_modid, port);
	      }
	        BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, BCM_GPORT_CHILD_PORT_GET(rq_child_gport[port]));
	    
	        bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
	        if (BCM_FAILURE(bcm_err)) {
		    cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		    return(CMD_FAIL);
	        }

                LOG_VERBOSE(BSL_LS_APPL_COMMON,
                            (BSL_META_U(unit,
                                        "Mapping, Switch Gport: 0x%x, Fabric Port: 0x%x\n"),
                             switch_port, fabric_port));
	    
#ifdef _BCM56634_MODEL_
	        bcm_err = bcm_stk_fabric_map_set(MODEL_BCM56634_MODID, switch_port, fabric_port);
	        if (BCM_FAILURE(bcm_err)) {
		    cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		    return(CMD_FAIL);
	        }
#endif /* _BCM56634_MODEL_ */
	    }

	cpu_port = port;
	if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	  BCM_GPORT_MODPORT_SET(switch_port, bcm56634_modid, MODEL_BCM56634_CPU_PORT);
	} else {
	  BCM_GPORT_MODPORT_SET(switch_port, bcm56634_modid, cpu_port);
	}
	BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
	
	bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
	if (BCM_FAILURE(bcm_err)) {
	    cli_out("bcm_stk_fabric_map_set failed for CPU port err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
	    return(CMD_FAIL);
	}
	
	/* configure switch port to fabric port mapping for the requeue traffic */
	for (port = 0; port < no_ports; port++) {
	    /* +1: on bcm56634, front panel ports start from 2 */
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	        BCM_GPORT_MODPORT_SET(switch_port, bcm56634_req_modid, (port + 1));
            }
            else {
	        BCM_GPORT_MODPORT_SET(switch_port, bcm56634_req_modid, port);
            }
	    BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, BCM_GPORT_CHILD_PORT_GET(child_gport[port]));
	    
	    bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
	    if (BCM_FAILURE(bcm_err)) {
	        cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		return(CMD_FAIL);
	    }
	    
#ifdef _BCM56634_MODEL_
	    bcm_err = bcm_stk_fabric_map_set(MODEL_BCM56634_MODID, switch_port, fabric_port);
	    if (BCM_FAILURE(bcm_err)) {
		cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		return(CMD_FAIL);
	    }
#endif /* _BCM56634_MODEL_ */
	}

	cpu_port = port;
	if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
	  BCM_GPORT_MODPORT_SET(switch_port, bcm56634_req_modid, MODEL_BCM56634_CPU_PORT);
	} else {
	  BCM_GPORT_MODPORT_SET(switch_port, bcm56634_req_modid, cpu_port);
	}
	BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
	
	bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
	if (BCM_FAILURE(bcm_err)) {
	    cli_out("bcm_stk_fabric_map_set failed for CPU port err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
	    return(CMD_FAIL);
	}

	/* setup FIC side unicast logical ports/queue group for each of the ports/channels */
	no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 8);
	flags = 0;
	for (port = 0; port < (no_ports); port++) {
	    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, rq_child_egress_gport[port],
					 no_cos, flags,  &rq_unicast_gport[port]);
	    if (bcm_err != BCM_E_NONE){
		cli_out("Error adding queue group for requeue phys_gport=0x%x\n", rq_child_egress_gport[port]);
		return(CMD_FAIL);
	    }
	}

	/* setup Local unicast queue group for each of the ports/channels */
	no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 16);
	flags = BCM_COSQ_GPORT_LOCAL;

	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
	  flags |= BCM_COSQ_GPORT_WITH_ID;
#ifdef TEST_REQUEUE
	  /*
	   * Higig0
	   */
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[0], 0x3000);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[1], 0x3100);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[2], 0x3200);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[3], 0x3300);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[4], 0x3400);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[5], 0x3500);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[6], 0x3600);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[7], 0x3700);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[8], 0x3800);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[9], 0x3900);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[10], 0x3a00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[11], 0x3b00);

	  /*
	   * Higig1
	   */
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[12], 0x3c00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[13], 0x3d00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[14], 0x3e00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[15], 0x3f00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[16], 0x4000);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[17], 0x4100);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[18], 0x4200);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[19], 0x4300);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[20], 0x4400);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[21], 0x4500);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[22], 0x4600);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[23], 0x4700);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[24], 0x4f00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[25], 0x4800);
#else
	  /*
	   * Higig0
	   */
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[0], 0x4000);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[1], 0x4100);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[2], 0x4200);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[3], 0x4300);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[4], 0x4400);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[5], 0x4500);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[6], 0x4600);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[7], 0x4700);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[8], 0x4800);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[9], 0x4900);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[10], 0x4a00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[11], 0x4b00);

	  /*
	   * Higig1
	   */
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[12], 0x4c00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[13], 0x4d00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[14], 0x4e00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[15], 0x4f00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[16], 0x5000);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[17], 0x5100);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[18], 0x5200);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[19], 0x5300);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[20], 0x5400);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[21], 0x5500);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[22], 0x5600);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[23], 0x5700);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[24], 0x5f00);
	  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(unicast_gport[25], 0x5800);
#endif
	}

	for (port = 0; port <  no_ports; port++) {
	    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, child_egress_gport[port],
					 no_cos, flags,  &unicast_gport[port]);
	    if (bcm_err != BCM_E_NONE){
		cli_out("Error adding queue group for local phys_gport=0x%x\n", child_egress_gport[port]);
		return(CMD_FAIL);
	    }
	}
	flags = 0;
    } else {
	/* configure switch port to fabric port mapping, 1-to-1 mapping */
	for (port = 0; port < no_ports; port++) {
	    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX) {
	      BCM_GPORT_MODPORT_SET(switch_port,0,port);
	    } else {
	      /* +1: on bcm56634, front panel ports start from 2 */
	      BCM_GPORT_MODPORT_SET(switch_port, MODEL_BCM56634_MODID, (port + 1));
	    }

	    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
		switch (port) {
		    case 1:
			if (no_ports >= 25) {
			    BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, 26);
			} else {
			    BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, port);
			}
			break;
		    case 4:
			if (no_ports >= 24) {
			    BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, 25);
			} else {
			    BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, port);
			}
			break;
		    case 25:
			BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, 1);
			break;
		    case 26:
			BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, 4);
			break;
		    default:
			BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, port);
			break;
		}
	    } else {
		BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, port);
	    }
	    
	    bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
	    if (BCM_FAILURE(bcm_err)) {
		cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		return(CMD_FAIL);
	    }
	    
            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit,
                                    "Mapping, Switch Gport: 0x%x, Fabric Port: 0x%x\n"),
                         switch_port, fabric_port));
	    
#ifdef _BCM56634_MODEL_
	    bcm_err = bcm_stk_fabric_map_set(MODEL_BCM56634_MODID, switch_port, fabric_port);
	    if (BCM_FAILURE(bcm_err)) {
		cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
		return(CMD_FAIL);
	    }
#endif /* _BCM56634_MODEL_ */
	}

	cpu_port = port;
	if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX) {
	  BCM_GPORT_MODPORT_SET(switch_port,sirius_modid,cpu_port);
	} else {
	  BCM_GPORT_MODPORT_SET(switch_port, MODEL_BCM56634_MODID, MODEL_BCM56634_CPU_PORT);
	}

	BCM_GPORT_EGRESS_CHILD_SET(fabric_port, sirius_modid, SB_FAB_DEVICE_SIRIUS_CPU_HANDLE);
	
	bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
	if (BCM_FAILURE(bcm_err)) {
	    cli_out("bcm_stk_fabric_map_set failed for CPU port err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
	    return(CMD_FAIL);
	}
	
	/* setup unicast logical ports/queue group for each of the ports/channels */
	no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 16);
	if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	    flags = BCM_COSQ_GPORT_LOCAL;
	}

	/* Allocate gports if fabric egress setup enabled */
	if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
	    for (port = 0; port <= no_ports; port++) {
		bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, child_egress_gport[port],
					     no_cos, flags,  &unicast_gport[port]);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error adding queue group for phys_gport=0x%x\n", child_egress_gport[port]);
		    return(CMD_FAIL);
		}
	    }
	}
    }

    if (SOC_SBX_CFG(unit)->bTmeMode != SOC_SBX_QE_MODE_FIC) {
	/* setup the TS for each of the above allocated queue groups */
	/* The setup is done from Top to Bottom. It could also be    */
	/* done from Bottom to Top                                   */
	port = 0;
	no_cos = soc_property_get(unit, spn_BCM_NUM_COS, 16);
	flags = BCM_COSQ_GPORT_SCHEDULER;

	for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
	    for (intf_port = 0; intf_port < hg_subports[intf]; intf_port++) {
		if (hg_subports[intf] > 8) {
		    /* subport nodes at level 5 */
		    no_levels_to_alloc = 4;
		} else {
		    /* subport nodes at level 6 */
		    no_levels_to_alloc = 5;
		}
		for (no_levels = 0; no_levels < no_levels_to_alloc; no_levels++) {
		    
		    /* could also specify the interface gport */
		    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, intf_gport[intf],
						 no_cos, flags,  &sched_gport);
		    if (bcm_err != BCM_E_NONE){
			cli_out("Error adding scheduler group for phys_gport=0x%x\n",
                                child_gport[port]);
			return(CMD_FAIL);
		    } else {
			LOG_VERBOSE(BSL_LS_APPL_COMMON,
			            (BSL_META_U(unit,
			                        "Added scheduler gport 0x%x\n"),
			             sched_gport));
		    }

		    if (no_levels == 0) {
			/* attach the scheduler to the port allocated resource */
			bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, child_gport[port],
							sched_gport, -1);
			if (bcm_err != BCM_E_NONE){
			    cli_out("Error attaching scheduler to port resource, port 0x%x\n",
                                    child_gport[port]);
			    return(CMD_FAIL);
			} else {
			    LOG_VERBOSE(BSL_LS_APPL_COMMON,
			                (BSL_META_U(unit,
			                            "Attached scheduler gport 0x%x to gport 0x%x\n"),
			                 sched_gport, child_gport[port]));
			}
		    }
		    
		    else if (no_levels < (no_levels_to_alloc - 1)) {
			/* attach scheduler to the previous scheduler */
			bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, prev_sched_gport,
							sched_gport, -1);
			if (bcm_err != BCM_E_NONE){
			    cli_out("Error attaching intermediate schedulers, port 0x%x\n",
                                    child_gport[port]);
			    return(CMD_FAIL);
			} else {
			    LOG_VERBOSE(BSL_LS_APPL_COMMON,
			                (BSL_META_U(unit,
			                            "Attached scheduler gport 0x%x to gport 0x%x\n"),
			                 sched_gport, prev_sched_gport));
			}
		    }
		    
		    else { /* (no_levels = (no_levels_to_alloc - 1)) */
			
			/* attach scheduler to the previous scheduler */
			bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, prev_sched_gport,
							sched_gport, -1);
			if (bcm_err != BCM_E_NONE){
			    cli_out("Error attaching intermediate schedulers, port 0x%x\n",
                                    child_gport[port]);
			    return(CMD_FAIL);
			} else {
			    LOG_VERBOSE(BSL_LS_APPL_COMMON,
			                (BSL_META_U(unit,
			                            "Attached scheduler gport 0x%x to gport 0x%x\n"),
			                 sched_gport, prev_sched_gport));
			}

			if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
			    /* attach queue group to the scheduler */
			    bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, sched_gport,
							    unicast_gport[port], -1);
			    if (bcm_err != BCM_E_NONE){
				cli_out("Error attaching unicast queue group to a scheduler, port 0x%x\n",
                                        child_gport[port]);
				return(CMD_FAIL);
			    } else {
				LOG_VERBOSE(BSL_LS_APPL_COMMON,
				            (BSL_META_U(unit,
				                        "Attached queue group gport 0x%x to gport 0x%x\n"),
				             unicast_gport[port], sched_gport));
			    }
			} else {
				LOG_VERBOSE(BSL_LS_APPL_COMMON,
				            (BSL_META_U(unit,
				                        "Attach EGRESS GROUP 0x%x to sched_port 0x%x gport\n"),
				             egress_group_gport[port], sched_gport));
			}
		    }
		    
		    prev_sched_gport = sched_gport;
		}
		port++;
	    }
	}


	/* setup the TS for cpu queue groups */
	flags = BCM_COSQ_GPORT_SCHEDULER;
	no_levels_to_alloc = 5;
	cpu_port = no_ports;
	for (no_levels = 0; no_levels < no_levels_to_alloc; no_levels++) {
	    
	    /* could also specify the interface gport */
	    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, intf_cpu_gport,
					 no_cos, flags,  &sched_gport);
	    if (bcm_err != BCM_E_NONE){
		cli_out("Error adding scheduler group for phys_gport=0x%x\n",
                        child_gport[cpu_port]);
		return(CMD_FAIL);
	    }
	    
	    if (no_levels == 0) {
		/* attach the scheduler to the port allocated resource */
		bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, child_gport[cpu_port],
						sched_gport, -1);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error attaching scheduler to port resource, port 0x%x\n",
                            child_gport[cpu_port]);
		    return(CMD_FAIL);
		}
	    }
	    else if (no_levels < (no_levels_to_alloc - 1)) {
		/* attach scheduler to the previous scheduler */
		bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, prev_sched_gport,
						sched_gport, -1);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error attaching intermediate schedulers, port 0x%x\n",
                            child_gport[cpu_port]);
		    return(CMD_FAIL);
		}
	    }
	    else { /* (no_levels = (no_levels_to_alloc - 1)) */
		
		/* attach scheduler to the previous scheduler */
		bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, prev_sched_gport,
						sched_gport, -1);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error attaching intermediate schedulers, port 0x%x\n",
                            child_gport[cpu_port]);
		    return(CMD_FAIL);
		}
		if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
		    /* attach queue group to the scheduler */
		    bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, sched_gport,
						    unicast_gport[cpu_port], -1);
		    if (bcm_err != BCM_E_NONE){
			cli_out("Error attaching unicast queue group to a scheduler, port 0x%x\n",
                                child_gport[cpu_port]);
			return(CMD_FAIL);
		    }
		}
	    }
	    prev_sched_gport = sched_gport;
	}
    }

#ifdef _SIRUS_MODEL_MULTICAST_
    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) {
        BCM_GPORT_EGRESS_MODPORT_SET(egr_intf_gport,
                                     BCM_GPORT_MODPORT_MODID_GET(intf_gport[0]),
                                     BCM_GPORT_MODPORT_PORT_GET(intf_gport[0]));

        bcm_err = bcm_cosq_gport_add(unit, egr_intf_gport, 1, BCM_COSQ_GPORT_SCHEDULER, &mcSchGport0);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_cosq_gport_add() Egress scheduler0, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        bcm_err = bcm_cosq_gport_attach(unit, egr_intf_gport, mcSchGport0, -1);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_cosq_gport_attach() Egress scheduler1, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        bcm_err = bcm_cosq_gport_add(unit, egr_intf_gport, 1, BCM_COSQ_GPORT_SCHEDULER, &mcSchGport1);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_cosq_gport_add() Egress scheduler1, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        bcm_err =  bcm_cosq_gport_attach(unit, mcSchGport0, mcSchGport1, -1);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_cosq_gport_attach() Egress scheduler1, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        bcm_err = bcm_fabric_port_create(unit,egr_intf_gport,-1,BCM_FABRIC_PORT_EGRESS_MULTICAST, &mcEgrFabricPort);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_fabric_port_create() Egress Multicast Port, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        bcm_err = bcm_cosq_gport_add(unit, mcEgrFabricPort, _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT, BCM_COSQ_GPORT_EGRESS_GROUP, &mcEgrGport);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_cosq_gport_ad() Egress Multicast Group, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        bcm_err = bcm_cosq_gport_attach(unit, mcSchGport1, mcEgrGport, -1);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_cosq_gport_attach() Egress Multicast Group, Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

#if (8 == _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT)
        bcm_err = bcm_cosq_control_set(unit, BCM_GPORT_INVALID, 3, bcmCosqControlClassMap, BCM_COS_MULTICAST_EF);
        if (BCM_E_NONE != bcm_err) {
            cli_out("Appl: Error bcm_cosq_control_set bcmCosqControlClassMap, EF; Err=%d(%s)\n",bcm_err, _SHR_ERRMSG(bcm_err));
            return CMD_FAIL;
        }

        bcm_err = bcm_cosq_control_set(unit, BCM_GPORT_INVALID, 6, bcmCosqControlClassMap, BCM_COS_MULTICAST_NON_EF);
        if (BCM_E_NONE != bcm_err) {
            cli_out("Appl: Error bcm_cosq_control_set bcmCosqControlClassMap, NON-EF; Err=%d(%s)\n",bcm_err, _SHR_ERRMSG(bcm_err));
            return CMD_FAIL;
        }
#endif /* (8 == _SIRIUS_FIFOS_PER_MC_INTERNAL_PORT) */

        bcm_err = bcm_cosq_control_set (unit, mcEgrGport, 0, bcmCosqControlFabricPortScheduler, mcEgrFabricPort);

#if MCAST_TRUNK
        bcm_err = bcm_trunk_chip_info_get(unit, &ta_chip_info);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_trunk_chip_info_get(), Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }
        tid = ta_chip_info.trunk_fabric_id_min;

        bcm_err = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_trunk_create(), Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }

        sal_memset(&ta_info, 0, sizeof(ta_info));
        for (i = 0, no_higig = 0; i < (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS >> 1); i++) {
            if (intf_gport[i] == 0) {
                continue;
            }
            bcm_trunk_member_t_init(&member_array[no_higig]);
            member_array[no_higig].gport = intf_gport[i];
            no_higig++;
        }

        bcm_err = bcm_trunk_set(unit, tid, &ta_info, no_higig, member_array);
        if (bcm_err != BCM_E_NONE){
            cli_out("Appl: Error bcm_trunk_set(), Err=0x%x\n", bcm_err);
            return(CMD_FAIL);
        }
#endif /* MCAST_TRUNK */
    } /* if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS) */

    /* Create dummy ESET */
    ds_id = 0;

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_HYBRID) {
      /* setup multicast logical port - create FIC queue group first */
      bcm_err = bcm_cosq_fabric_distribution_add(MODEL_SIRIUS_UNIT, ds_id, no_cos,
						 0 /* flags */, &multicast_gport);
      if (bcm_err != BCM_E_NONE) {
        cli_out("Error creating multicast fabric distribution queue group error(%d)\n", bcm_err);
        return(CMD_FAIL);
      }
    } else {
	if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
	    /* setup multicast logical port - create local queue group first */
	    bcm_err = bcm_cosq_fabric_distribution_add(MODEL_SIRIUS_UNIT, ds_id, no_cos,
						       BCM_COSQ_GPORT_LOCAL /* flags */,
						       &multicast_gport);
	    if (bcm_err != BCM_E_NONE) {
		cli_out("Error creating multicast fabric distribution local queue group error(%d)\n", bcm_err);
		return(CMD_FAIL);
	    }
	}
    }

    if (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME) {
	
        /* Create an ingress multicast fabric/child gport */
        bcm_err = bcm_fabric_port_create(MODEL_SIRIUS_UNIT, mc_intf_gport, 0, BCM_FABRIC_PORT_INGRESS_MULTICAST, &multicast_child_gport);
	
	if (bcm_err != BCM_E_NONE){
	    cli_out("Error adding subport scheduler group for multicast gport=0x%x on interface gport=0x%x\n",
                    multicast_child_gport, mc_intf_gport);
	    return(CMD_FAIL);
	}

	cli_out("multicast child_gport(0x%x)\n", multicast_child_gport);

	/* Create the level 6 subport gport - for multicast, this isn't created at init time like the others */
	flags = BCM_COSQ_GPORT_SCHEDULER;
	bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, mc_intf_gport,
				     no_cos, flags,  &multicast_sched_gport);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Error adding subport scheduler group for multicast gport=0x%x on interface gport=0x%x\n",
                    multicast_sched_gport, mc_intf_gport);
	    return(CMD_FAIL);
	}

	cli_out("multicast sched_gport(0x%x)\n", multicast_sched_gport);

	/* attach the scheduler to last requeue interface (level 6)  */
	bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, mc_intf_gport,
					multicast_sched_gport, -1);
	if (bcm_err != BCM_E_NONE){
	    cli_out("Error attaching scheduler to interface (for multicast) resource, port 0x%x\n",
                    multicast_sched_gport);
	    return(CMD_FAIL);
	}
	
#define TS_NO_LEVELS_TO_ALLOCATE    (5)
    
	for (no_levels = 0; no_levels < TS_NO_LEVELS_TO_ALLOCATE; no_levels++) {
	    
	    /* could also specify the interface gport */
	    bcm_err = bcm_cosq_gport_add(MODEL_SIRIUS_UNIT, mc_intf_gport,
					 no_cos, flags,  &sched_gport);
	    if (bcm_err != BCM_E_NONE){
		cli_out("Error adding scheduler group for multicast gport=0x%x\n",
                        multicast_child_gport);
		return(CMD_FAIL);
	    }

	    if (no_levels == 0) {
		/* attach the scheduler to the port allocated resource */
		bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, multicast_sched_gport,
						sched_gport, -1);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error attaching scheduler to multicast port resource, port 0x%x\n",
                            multicast_sched_gport);
		    return(CMD_FAIL);
		}
	    }
	    
	    else if (no_levels < (TS_NO_LEVELS_TO_ALLOCATE - 1)) {
		/* attach scheduler to the previous scheduler */
		bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, prev_sched_gport,
						sched_gport, -1);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error attaching intermediate schedulers, port 0x%x\n",
                            multicast_child_gport);
		    return(CMD_FAIL);
		}
	    }
	    
	    else { /* (no_levels = (TS_NO_LEVELS_TO_ALLOCATE - 1)) */
		
		/* attach scheduler to the previous scheduler */
		bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, prev_sched_gport,
						sched_gport, -1);
		if (bcm_err != BCM_E_NONE){
		    cli_out("Error attaching intermediate schedulers, port 0x%x\n",
                            multicast_child_gport);
		    return(CMD_FAIL);
		}
		
		if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
		    /* attach queue group to the scheduler */
		    bcm_err = bcm_cosq_gport_attach(MODEL_SIRIUS_UNIT, sched_gport,
						    multicast_gport, -1);
		    if (bcm_err != BCM_E_NONE){
			cli_out("Error attaching unicast queue group to scheduler 0x%x, port 0x%x\n",
                                sched_gport, child_gport[port]);
			return(CMD_FAIL);
		    }
		} else {
		    cli_out("Attach unicast queue group to scheduler, port 0x%x\n",
                            sched_gport);
		}
	    }
	    
	    prev_sched_gport = sched_gport;
	}
    }

    /* create multicast group 1 */
    mcgroup = 1;
    bcm_err = bcm_multicast_create(MODEL_SIRIUS_UNIT, BCM_MULTICAST_WITH_ID, &mcgroup);
    
    if (bcm_err != BCM_E_NONE) {
        cli_out("Error allocating multicast group error(%d)\n", bcm_err);
        return(CMD_FAIL);
    }

    if (SOC_SBX_CFG(unit)->fabric_egress_setup) {
	for (port = 0; port < no_ports; port++) {
	    bcm_err = bcm_multicast_egress_add(MODEL_SIRIUS_UNIT, mcgroup,
					       child_gport[port], 0 /* encap_id */);
	    if (bcm_err != BCM_E_NONE) {
		cli_out("Error adding ports to the multicast group error(%d)\n", bcm_err);
		return(CMD_FAIL);
	    } else {
		LOG_VERBOSE(BSL_LS_APPL_COMMON,
		            (BSL_META_U(unit,
		                        "Added multicast child gport 0x%x\n"),
		             child_gport[port]));
	    }
	}

        if (SOC_SBX_IF_PROTOCOL_XGS == SOC_SBX_CFG(unit)->uInterfaceProtocol) {
            /* Associate mcgroup with queue group */
            bcm_err = bcm_multicast_fabric_distribution_set(MODEL_SIRIUS_UNIT, mcgroup, ds_id);

            if (bcm_err != BCM_E_NONE) {
                cli_out("Error associating multicast group with ds_id\n");
            }
        }
    } else {
	for (port = 0; port < no_ports; port++) {
	    cli_out("bcm_multicast_egress_add 0 0x%d 0x%x 0\n",mcgroup, egress_group_gport[port]);
	}

	cli_out("bcm_multicast_fabric_distribution_set 0 %d %d\n", mcgroup, ds_id);
    }


#endif /* _SIRUS_MODEL_MULTICAST_ */

    return(rc);
}
#endif /* BCM_SIRIUS_SUPPORT */

/*
 * End, Sirius Model related setup/configuration
 */

int
_mc_fpga_read8(int addr, uint8 *v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int pl = -1;
    int word;
    int shift = (3 - (addr & 3)) * 8;
    int unit=-1;

    int i;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))){
        if (lcm0==-1)
          lcm0=SOC_NDEV_IDX2DEV(i);
        else if (lcm1==-1)
          lcm1=SOC_NDEV_IDX2DEV(i);
      }else  if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i))){
        pl = SOC_NDEV_IDX2DEV(i);
      }
    }


    if (lcm1 != -1){
      word = FPGA_BASE + (addr & ~0x3);
      unit = lcm1;
    }else if (pl != -1){
      word = FPGA_PL_BASE + (addr & ~0x3);
      unit = pl;
    }else{
      cli_out("LCM/BME with FPGA not found\n");
      return CMD_FAIL;
    }
    *v = (CMREAD(unit, word) >> shift) & 0xff;
    return CMD_OK;
}

int
_mc_fpga_write8(int addr, uint8 v)
{
    int lcm0 = -1;
    int lcm1 = -1;
    int pl = -1;
    int shift = (3 - (addr & 3)) * 8;
    int word = FPGA_BASE + (addr & ~0x3);
    int unit = -1;
    int i;
    uint32 v0;

    for (i=0;i<soc_ndev;i++){
      if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))){
        if (lcm0==-1)
          lcm0=SOC_NDEV_IDX2DEV(i);
        else if (lcm1==-1)
          lcm1=SOC_NDEV_IDX2DEV(i);
      }else if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i))){
        pl = SOC_NDEV_IDX2DEV(i);
      }
    }

    if (lcm1 != -1){
      unit = lcm1;
      word = FPGA_BASE + (addr & ~0x3);
    }else if (pl != -1){
      unit = pl;
      word = FPGA_PL_BASE + (addr & ~0x3);
    }else{
      cli_out("LCM/BME with FPGA not found\n");
      return CMD_FAIL;
    }

    v0 = CMREAD(unit, word);
    v0 &= ~(0xff << shift);
    v0 |= (v << shift);
    CMWRITE(unit, word, v0);

    return CMD_OK;
}

int
sbx_diag_init(int brd_type)
{
    int rv = 0;
    uint8 fpga_info=0;
    uint8 fpga_id=0;
    uint8 board_rev=0;
    uint8 data;

   if (brd_type == BOARD_TYPE_LCMODEL || brd_type == BOARD_TYPE_SIRIUS_SIM ||
 	brd_type == BOARD_TYPE_SIRIUS_IPASS || brd_type== BOARD_TYPE_POLARIS_IPASS) {
        return 0;
    }

    /* Nothing to be done here for the QE2K Benchscreen Board */
    if (brd_type == BOARD_TYPE_QE2K_BSCRN_LC) {
        return 0;
    }

    /* check for polaris rev1 and change modids, and steer config
       according to new rv LC
    */

    /* criteria should be card type and board id, but for now
       we need to use what's available */
    rv = _mc_fpga_read8(FPGA_REVISION_OFFSET, &fpga_info);
    if (rv){
      cli_out("FPGA read error\n");
      return rv;
    }
    _mc_fpga_read8(FPGA_ID_OFFSET, &fpga_id);
    _mc_fpga_read8(FPGA_BOARD_REV_OFFSET, &board_rev);
    if (fpga_id == 0x18){
      if (board_rev == 2){ /* Rev 010 Polaris LC */
        _cmd_card_filter = CMD_CARD_PLLC_REV1;
      }else{
        _cmd_card_filter = CMD_CARD_PLLC_REV0;
      }
      if (!soc_property_get(0, spn_DIAG_DISABLE_INTERRUPTS, 0)){
        /* unmask interrupts for Polaris */
        _mc_fpga_read8(FPGA_LC_PL_INT_OFFSET, &data);
        data |= FPGA_LC_PL_INT;
        _mc_fpga_write8(FPGA_LC_PL_INT_OFFSET, data);
      }else{
        cli_out("Disabling interrupts for all devices\n");
        _mc_fpga_read8(FPGA_LC_PL_INT_OFFSET, &data);
        data &= (~FPGA_LC_PL_INT);
        data &= (~FPGA_LC_PL_PCI_INT);
        _mc_fpga_write8(FPGA_LC_PL_INT_OFFSET, data);
      }
    }else if (fpga_id == 0x19){
      /* PL FC */
      /* unmask interrupts for Polaris */
      _mc_fpga_read8(FPGA_FC_PL_INT_OFFSET, &data);
      data |= FPGA_FC_PL_INT;
      _mc_fpga_write8(FPGA_FC_PL_INT_OFFSET, data);
    } else if (fpga_id == 0x20) {
      if (!soc_property_get(0, spn_DIAG_DISABLE_INTERRUPTS, 0)){
        /* unmask interrupts for FE2kxt board */
        _mc_fpga_read8(FPGA_LC_PL_INT_OFFSET, &data);
        data |= FPGA_LC_PL_INT;
        _mc_fpga_write8(FPGA_LC_PL_INT_OFFSET, data);
      }else{
        cli_out("Disabling interrupts for all devices\n");
        _mc_fpga_read8(FPGA_LC_PL_INT_OFFSET, &data);
        data &= (~FPGA_LC_PL_INT);
        data &= (~FPGA_LC_PL_PCI_INT);
        _mc_fpga_write8(FPGA_LC_PL_INT_OFFSET, data);
      }
    }

    return rv;
}

void
cmd_sbx_reset_sram_dll(sbhandle sbh)
{
    _mc_fpga_write8(FPGA_FE2K_DLL_ENABLE_OFFSET, 0);
    _mc_fpga_write8(FPGA_FE2K_DLL_ENABLE_OFFSET, FPGA_FE2K_DLL_ENABLE);
}

#define QE2000_SFI_PORT_BASE 50
#define SIRIUS_SFI_PORT_BASE 9
#define SIRIUS_SCI_PORT_BASE 31 	  	 

char cmd_sbx_mclcinit_config_usage[] =
"Usage:\n"
"mclcinit nodeid=<num> [LinkEnable=0x3ffff]\n"
"  nodeid is the line card's node in the system\n"
;

cmd_result_t
cmd_sbx_mclcinit_config(int unit, args_t *a)
{
  return cmd_sbx_mclcinit_table_config(unit, a);
}

#ifdef BCM_QE2000_SUPPORT
/* These tables program the VLAN remap table in the EP. If they look
 * strange, they're because (a) they're byte-swapped and (b) they're
 * swapped between words but not within words. The intent is also that
 * VLAN 2 get the base gport.
 */
static uint32
vlan_table_16[] = { 0x00001000, 0x20013001, 0x40005000, 0x20003000,
                    0x80009000, 0x60007000, 0xc000d000, 0xa000b000,
                    0x00011001, 0xe000f000 };
static uint32
vlan_table_8[]  = { 0x00000800, 0x90009800, 0x20002800, 0x10001800,
                    0x40004800, 0x30003800, 0x60006800, 0x50005800,
                    0x80008800, 0x70007800 };
#endif

#define _NODE_PRESENT(n) _sbx_diag_cosq_node_present(n)

static int
_sbx_diag_cosq_node_present(int n)
{
  /* This macro allows us to set up only a subset of num_modules nodes */
  /* note that node 56 and node 24 would be the same if modulo 32 is done due to fe2k
     so we take out node 56 */


  uint32 nodes_pl_chassis = ((n==68)|(n==71) /*| (n==56)*/ |(n==6)|(n==46)|(n==16)|(n==34)|(n==24));
  uint32 nodes_pl_standalone_rev0 = ((n == PL_FE0_MODID) | (n == PL_FE1_MODID));
  uint32 nodes_pl_standalone_rev2 = ((n == PL1_FE0_MODID) | (n == PL1_FE1_MODID));
  uint32 nodes_pl_fe2kxt = (n == PL1_FE0_MODID);
  uint32 nodes_pl_fe2kxt_chassis = (n == (FE2KXT_PL_FAB_LC0_QE0_NODE % 32));

  uint32 nodes_pt_chassis = ((n==0)|(n==1)|(n==2)|(n==3));
  uint32 nodes_tme = (n==0);
  uint8 fpga_id, board_rev;

  if (soc_property_get(0 /* qe unit */, spn_QE_TME_MODE, 0) == 1) {
      return nodes_tme;
  }

#ifdef BCM_BM9600_SUPPORT
  if (sbx_appl_is_lgl_node_present == FALSE) {
      nodes_pl_chassis = ((n==68)|(n==71) /*| (n==56)*/ |(n==6)|(n==46)|(n==16)|(n==34)|(n==24));
  } else {
    nodes_pl_chassis = ((n == PL_FAB_LC0_QE0_NODE) | (n == PL_FAB_LC0_QE1_NODE) | (n == PL_FAB_LC1_QE0_NODE) | (n == PL_FAB_LC1_QE1_NODE) | (n == PL_FAB_LC2_QE0_NODE) | (n == PL_FAB_LC2_QE1_NODE) | (n == PL_FAB_LC3_QE0_NODE) | (n == PL_FAB_LC3_QE1_NODE));
  }
#endif

  _mc_fpga_read8(FPGA_ID_OFFSET, &fpga_id);
  _mc_fpga_read8(FPGA_BOARD_REV_OFFSET, &board_rev);
  if (fpga_id == 0x18){
    /* PL LC */
    if (soc_property_get(0, spn_DIAG_CHASSIS, 0)){
      return nodes_pl_chassis;
    }else{
      if (board_rev>=2){
        return nodes_pl_standalone_rev2;
      }else{
        return nodes_pl_standalone_rev0;
      }
    }
  }else if (fpga_id == 0x19){
    return nodes_pl_chassis;
  }else if (fpga_id == 0x20) {
    if (soc_property_get(0, spn_DIAG_CHASSIS, 0)){
      return nodes_pl_fe2kxt_chassis;
    } else {
      return nodes_pl_fe2kxt;
    }
  }else{
    return nodes_pt_chassis;
  }
  return 0;
}

static void
_sbx_diag_cosq_init_all(void)
{
  /* This function will provision the fabric by adding gports for all ports.
     The queue id's must match up with those hard coded in the FE*/
  int unit;
  int rv;
  int queue = 0;
  int node;
  int tme;
  int num_nodes;
  int num_cos;
  bcm_gport_t phys_gport = BCM_GPORT_INVALID, gport;
  int gport_count=0;
  int error_count=0, total_errors=0;
  int eset;
  int port;
  int is_requeue[SBX_MAX_PORTS] = {-1};
  int last_requeue_port;
#ifdef BCM_QE2000_SUPPORT
  sbG2EplibCtxt_st *ep = NULL;
  uint32 ip_table_mem[2];
#endif
  int mymodid;
  int i;
  int temp_tme, chassis_type;
#ifdef BCM_QE2000_SUPPORT
  uint32 *vlan_table;
#endif
  int max_esets;

  cli_out("Provisioning fabric\n");
  LOG_VERBOSE(BSL_LS_APPL_COMMON,
              (BSL_META("DEBUG: [%s:%d]: Provisioning fabric\n"),
               FUNCTION_NAME(), __LINE__));
  for (unit=0; unit<soc_ndev; unit++) {
    if (!((SOC_NDEV_IDX2DEV(unit)) >= 0 && ((SOC_NDEV_IDX2DEV(unit)) < (BCM_MAX_NUM_UNITS)))) {
        continue;
    }

    tme = soc_property_get(SOC_NDEV_IDX2DEV(unit), spn_QE_TME_MODE, 0);
    num_nodes = (tme == 1) ? 1 : SOC_SBX_CFG(0)->cfg_num_nodes; /* there may be a better way to get this */
    num_cos = soc_property_get(SOC_NDEV_IDX2DEV(unit), spn_BCM_NUM_COS, (tme == 1) ? 16 : 8);

    if (!SOC_SBX_INIT(SOC_NDEV_IDX2DEV(unit))) {
        cli_out("SKIPPING COMMAND for UNIT %d\n", SOC_NDEV_IDX2DEV(unit));
        LOG_VERBOSE(BSL_LS_APPL_COMMON,
                    (BSL_META_U(SOC_NDEV_IDX2DEV(unit),
                                "DEBUG: [%s:%d]: SKIPPING COMMAND for UNIT %d\n"),
                     FUNCTION_NAME(), __LINE__, SOC_NDEV_IDX2DEV(unit)));
        continue;
    }

    rv = bcm_stk_my_modid_get(SOC_NDEV_IDX2DEV(unit), &mymodid);
    if (rv != BCM_E_UNAVAIL && rv != BCM_E_NONE){
      error_count++;
      cli_out("Error getting modid, unit=%d\n", SOC_NDEV_IDX2DEV(unit));
      /* gsrao 082108 - If there is a problem getting modid for the unit then skip it. This
       * needed for running diags on boards that might not be completely populated like the
       * C1/C2 boards that may only have the QE.
       */
      continue;
    } else {
      cli_out("Processing unit=%d\n", SOC_NDEV_IDX2DEV(unit));
      LOG_VERBOSE(BSL_LS_APPL_COMMON,
                  (BSL_META_U(SOC_NDEV_IDX2DEV(unit),
                              "DEBUG: [%s:%d]: Processing unit=%d\n"),
                   FUNCTION_NAME(), __LINE__, SOC_NDEV_IDX2DEV(unit)));
    }

    if (!SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(unit)) && !SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(unit)) && 
        !SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(unit))) {
        cli_out("Appl: No Processing unit=%d\n", SOC_NDEV_IDX2DEV(unit));
        continue;
    } 

    if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(unit)) || (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(unit)))) {
        chassis_type = soc_property_get(SOC_NDEV_IDX2DEV(unit), spn_DIAG_CHASSIS, 0);
        if (chassis_type != SBX_CHASSIS_TYPE_STANDALONE) {
            /*
             *    spn_BM_DEVICE_MODE implies LCM
             */  
            if (soc_property_get(SOC_NDEV_IDX2DEV(unit), spn_BM_DEVICE_MODE, 2) == 3) {
                continue;
            }
        }
    }

    if (soc_property_get(SOC_NDEV_IDX2DEV(unit), spn_DIAG_COSQ_INIT,0) == 1){
      error_count=0;

#ifdef BCM_QE2000_SUPPORT
      if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(unit))) {
        ep = gu2_unit2ep(SOC_NDEV_IDX2DEV(unit));
        if (ep == NULL) {
          error_count++;
          cli_out("Error: no EP handle available\n");
        }
      }
#endif
      /* unicast 50 ports per node */
      if (SOC_NDEV_IDX2DEV(unit) == 0) {
        /* we check for requeue ports here, unit 0 had better be the same as
         * any other QE because we can't check once we're at the BM
         */
        for (port=0; port<SBX_MAX_PORTS; port++){
          if (IS_SPI_SUBPORT_PORT(SOC_NDEV_IDX2DEV(unit), port)) {
            rv = bcm_port_control_get(SOC_NDEV_IDX2DEV(unit), port, bcmPortControlPacketFlowMode, &is_requeue[port]);
            if (rv != BCM_E_NONE || port >= 49){
              is_requeue[port] = -1;
            }
          } else {
            is_requeue[port] = -1;
          }
        }
      }
      last_requeue_port = -1;
      temp_tme = tme;

      for (port=0; port<SBX_MAX_PORTS; port++){
        if (temp_tme == 2) {
          if (is_requeue[port] == -1)
            continue;
          if (!is_requeue[port]) {
            /* This is a non-requeue port. Find a requeue port to associate it with. */
            for (last_requeue_port++; last_requeue_port < SBX_MAX_PORTS; last_requeue_port++) {
              if (is_requeue[last_requeue_port] == 1) {
                break;
              }
            }
            if (last_requeue_port < SBX_MAX_PORTS) {
              /* We'll set up a FIC queue on the requeue port and 20 TME queues
               * on the target port, then link them.
               */
              for (node=0; node<num_nodes;node++){
                if (!_NODE_PRESENT(node))
                  continue;
                /* Set up the FIC queue. We assume that the requeue/non-requeue mapping
                 * is the same for each line card.
                 */
                queue = SOC_SBX_NODE_PORT_TO_QID(SOC_NDEV_IDX2DEV(unit),node, port, num_cos);
                if (!tme) {
                  BCM_GPORT_UCAST_QUEUE_GROUP_SET(gport, queue);
                } else {
                  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(gport, queue);
                }
                BCM_GPORT_MODPORT_SET(phys_gport, BCM_MODULE_FABRIC_BASE+node, last_requeue_port);
                gport_count++;
                rv = bcm_cosq_gport_add(SOC_NDEV_IDX2DEV(unit), phys_gport, num_cos, BCM_COSQ_GPORT_WITH_ID, &gport);
                if (rv != BCM_E_UNAVAIL && rv != BCM_E_NONE){
                  error_count++;
                  cli_out("[%s:%d] Error adding phys_gport=0x%x queue=%d, gport=0x%x\n", FUNCTION_NAME()
                          ,__LINE__,phys_gport, queue, gport);
                }
              }/*node */

              if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(unit))) {
                for (i = 0; i < 20; i++) {
                  gport = -1;
                  BCM_GPORT_MODPORT_SET(phys_gport, mymodid, port);
                  gport_count++;
                  rv = bcm_cosq_gport_add(SOC_NDEV_IDX2DEV(unit), phys_gport, num_cos, BCM_COSQ_GPORT_LOCAL, &gport);
                  if (rv != BCM_E_UNAVAIL && rv != BCM_E_NONE){
                    error_count++;
                    cli_out("[%s:%d] Error adding phys_gport=0x%x queue=%d, gport=0x%x\n", FUNCTION_NAME()
                            ,__LINE__,phys_gport, queue, gport);
                  } else {
                    /* cli_out("Added2: queue=0x%x, phys_gport=0x%x, gport=0x%x, modid=%d, port=%d\n", queue, phys_gport, gport, mymodid, port); */
                    LOG_VERBOSE(BSL_LS_APPL_COMMON,
                                (BSL_META_U(SOC_NDEV_IDX2DEV(unit),
                                            "DEBUG: [%s:%d]: Added: queue=0x%x, phys_gport=0x%x, gport=0x%x, modid=%d, port=%d\n"),
                                 FUNCTION_NAME(), __LINE__, queue, phys_gport, gport, mymodid, port));
                  }
                }
                /* Tell the EP where the base queue is */
#ifdef BCM_QE2000_SUPPORT
                gport -= 19*num_cos; /* go back to the base gport */

                rv = sbG2EplibPortEncapGet(ep, last_requeue_port, ip_table_mem);
                if (rv != SB_ELIB_OK) {
                  error_count++;
                  cli_out("Error retrieving EP port table for unit %d\n", SOC_NDEV_IDX2DEV(unit));
                }
                queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(gport);
                queue = (queue >> 8) | ((queue & 0xFF) << 8);
                switch (last_requeue_port & 3) {
                case 0:
                  ip_table_mem[1] &= 0xffff;
                  ip_table_mem[1] |= (queue << 16);
                  break;
                case 1:
                  ip_table_mem[1] &= 0xffff0000;
                  ip_table_mem[1] |= queue;
                  break;
                case 2:
                  ip_table_mem[0] &= 0xffff;
                  ip_table_mem[0] |= (queue << 16);
                  break;
                case 3:
                  ip_table_mem[0] &= 0xffff0000;
                  ip_table_mem[0] |= queue;
                  break;
                }
                rv = sbG2EplibPortEncapSet(ep, last_requeue_port, ip_table_mem);
                if (rv != SB_ELIB_OK) {
                  error_count++;
                  cli_out("Error writing EP port table for unit %d\n", SOC_NDEV_IDX2DEV(unit));
                }
#endif
              }
            } else {
              /* We have run out of requeue ports. Treat the rest of the ports as conventional FIC ports. */
              temp_tme = 0;
            }
          }
        }

        if (temp_tme != 2 && (tme != 2 || is_requeue[port] == 0)) {
          for (node=0; node<num_nodes;node++){
            if (!_NODE_PRESENT(node))
              continue;
            queue = SOC_SBX_NODE_PORT_TO_QID(SOC_NDEV_IDX2DEV(unit),node, port, num_cos);
            if (!tme) {
              BCM_GPORT_UCAST_QUEUE_GROUP_SET(gport, queue);
            } else {
              BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(gport, queue);
            }
            BCM_GPORT_MODPORT_SET(phys_gport, BCM_MODULE_FABRIC_BASE+node, port);
            gport_count++;
            rv = bcm_cosq_gport_add(SOC_NDEV_IDX2DEV(unit), phys_gport, num_cos, BCM_COSQ_GPORT_WITH_ID, &gport);
            if (rv != BCM_E_UNAVAIL && rv != BCM_E_NONE){
                error_count++;
                cli_out("[%s:%d] Error adding phys_gport=0x%x queue=%d, gport=0x%x\n", FUNCTION_NAME()
                        ,__LINE__,phys_gport, queue, gport);
            } else {
              /* cli_out("Added3: queue=0x%x, phys_gport=0x%x, gport=0x%x, node=%d, port=%d\n", queue, phys_gport, gport, node, port); */
              LOG_VERBOSE(BSL_LS_APPL_COMMON,
                          (BSL_META_U(SOC_NDEV_IDX2DEV(unit),
                                      "DEBUG: [%s:%d]: Added: queue=0x%x, phys_gport=0x%x, gport=0x%x, node=%d, port=%d\n"),
                           FUNCTION_NAME(), __LINE__, queue, phys_gport, gport, node, port));
            }
          }/*node */
        }
      }

      /* multicast - esets are added (not supported for hybrid mode yet) */
      if (tme != 2) {
	if (tme) {
 	  max_esets = 31;
	}else {
	  max_esets = SOC_SBX_CFG(SOC_NDEV_IDX2DEV(unit))->num_ds_ids;
	}
        for(eset = 0; eset < max_esets; eset++) {
          queue = SOC_SBX_DS_ID_TO_QID(SOC_NDEV_IDX2DEV(unit), eset, num_cos);
          
          if (!tme ) {
            BCM_GPORT_MCAST_QUEUE_GROUP_SYSQID_SET(gport, 0, queue);
          } else {
            BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(gport,queue);
          }
          rv = bcm_cosq_fabric_distribution_add(SOC_NDEV_IDX2DEV(unit), eset, num_cos, BCM_COSQ_GPORT_WITH_ID, &gport);
          if (rv != BCM_E_UNAVAIL && rv != BCM_E_NONE){
            error_count++;
            cli_out("[%s:%d] Error adding phys_gport=0x%x queue=%d, gport=0x%x multicast\n", FUNCTION_NAME()
                    ,__LINE__,phys_gport, queue, gport);
          }
        }
      }

#ifdef BCM_QE2000_SUPPORT
      if (tme == 2 && SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(unit))) {
        /* Set up the VLAN table so that the VLANs will point to separate queues. */
        vlan_table = NULL;
        if (num_cos == 16) {
          vlan_table = vlan_table_16;
        } else if (num_cos == 8) {
          vlan_table = vlan_table_8;
        }
        if (vlan_table) {
          for (i = 0; i < 128; i += 4) {
            rv = sbG2EplibVlanRemapSet(ep, i, &vlan_table[(i / 2) % (sizeof(vlan_table_16)/sizeof(vlan_table_16[0]))]);
            if (rv != SB_ELIB_OK) {
              error_count++;
              cli_out("Error writing EP port table for unit %d\n", SOC_NDEV_IDX2DEV(unit));
            }
          }
        } else {
          error_count++;
          cli_out("Unsupported number of COS levels %d (only 8 or 16 supported)\n", num_cos);
        }
      }
#endif

      total_errors += error_count;
      if (error_count){
        cli_out("diag_cosq_init: ERROR: unit %d got %d errors when adding gports\n", SOC_NDEV_IDX2DEV(unit), error_count);
      }

    }

  } /* unit */

  cli_out("diag_cosq_init: Added %d gports. Exiting with %d errors\n", gport_count, total_errors);

}



/*
 ***************************************************
 *
 *  Metrocore LC in chassis
 *
 ***************************************************
 */
#define MCLC_CHAS_UNIT_QE    0
#define MCLC_CHAS_UNIT_FE    1
#define MCLC_CHAS_UNIT_LCM0  2
#define MCLC_CHAS_UNIT_LCM1  3

#define MCLC_CHAS_MODID_QE0   10000
#define MCLC_CHAS_MODID_QE1   10001
#define MCLC_CHAS_MODID_QE2   10002
#define MCLC_CHAS_MODID_QE3   10003

#define MCLC_CHAS_MODID_FE0   0

dev_cmd_rec mc_lc_chassis_cmd_tbl[] = {
        {SET_MODID,         MCLC_CHAS_UNIT_QE, AUTO_MODID_QE0,  0, 0, 0},
        {SET_MODID,         MCLC_CHAS_UNIT_FE, AUTO_MODID_FE0, 0, 0, 0},

        {PORT_ENABLE_RANGE, MCLC_CHAS_UNIT_LCM0, 7, 39, 0, 0},
        {PORT_ENABLE_RANGE, MCLC_CHAS_UNIT_LCM1, 7, 39, 0, 0},
        {PORT_ENABLE_RANGE, MCLC_CHAS_UNIT_QE,  50, 67, 0, 0},
        {PORT_ENABLE, MCLC_CHAS_UNIT_QE, 68, 0, 0, 0},
        {PORT_ENABLE, MCLC_CHAS_UNIT_QE, 69, 0, 0, 0},

/*
 *     see xcfg pin array info below for pin mapping index info
 */
#ifdef BCM_BME3200_SUPPORT
        {LCM_BM3200_XCFG_INIT, MCLC_CHAS_UNIT_LCM0,  1,  0, 0, 0, 0, 0},
        {LCM_BM3200_XCFG_INIT, MCLC_CHAS_UNIT_LCM0,  0,  1, 0, 0, 0, 0},
        {LCM_BM3200_XCFG_INIT, MCLC_CHAS_UNIT_LCM1,  1,  0, 0, 0, 0, 0},
        {LCM_BM3200_XCFG_INIT, MCLC_CHAS_UNIT_LCM1,  0,  1, 0, 0, 0, 0},

        {LCM_MODE_SET,         MCLC_CHAS_UNIT_LCM0,  lcmModeHwSelect,  0, 0, 0, 0, 0},
        {LCM_MODE_SET,         MCLC_CHAS_UNIT_LCM1,  lcmModeHwSelect,  0, 0, 0, 0, 0},

        { SET_MODULE_PROTOCOL, MCLC_CHAS_UNIT_QE, MCLC_CHAS_MODID_QE0, bcmModuleProtocol1, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, MCLC_CHAS_UNIT_QE, MCLC_CHAS_MODID_QE1, bcmModuleProtocol1, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, MCLC_CHAS_UNIT_QE, MCLC_CHAS_MODID_QE2, bcmModuleProtocol1, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, MCLC_CHAS_UNIT_QE, MCLC_CHAS_MODID_QE3, bcmModuleProtocol1, 0, 0, 0 },

/*
 *
 */
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE,  0,  8, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE0, 31},
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE,  0,  8, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE1, 21},
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE,  0,  8, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE2, 11},
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE,  0,  8, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE3, 1},

        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE, 10, 17, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE0, 8},
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE, 10, 17, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE1, 16},
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE, 10, 17, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE2, 24},
        {XBAR_CONNECT_RANGE, MCLC_CHAS_UNIT_QE, 10, 17, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE3, 32},

        {XBAR_CONNECT_SET,   MCLC_CHAS_UNIT_QE,  9, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE0, 0},
        {XBAR_CONNECT_SET,   MCLC_CHAS_UNIT_QE,  9, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE1, 10},
        {XBAR_CONNECT_SET,   MCLC_CHAS_UNIT_QE,  9, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE2, 20},
        {XBAR_CONNECT_SET,   MCLC_CHAS_UNIT_QE,  9, MCLC_CHAS_MODID_QE0, 0, MCLC_CHAS_MODID_QE3, 30},

        {XBAR_ENABLE_SET,   MCLC_CHAS_UNIT_QE, 0x3ffff, 0, 0, 0, 0},
        {MODULE_ENABLE_SET, MCLC_CHAS_UNIT_QE, MCLC_CHAS_MODID_QE0, 1, 0, 0, 0},
#endif /* BCM_BME3200_SUPPORT */
        {-1, 0, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
};

/*
 ***************************************************
 * metrocore command table for tme mode
 ***************************************************
 */
dev_cmd_rec mc_lc_tmemode_cmd_tbl[] = {
        {PORT_ENABLE_RANGE, MCLC_UNIT_LCM0, 31, 39, 0, 0},   /* SFIs */
        {PORT_ENABLE, MCLC_UNIT_LCM0, 0,  0, 0, 0},   /* SCI  */

        {PORT_ENABLE_RANGE, MCLC_UNIT_LCM1, 31, 39, 0, 0},

        {SET_MODID,   MCLC_UNIT_QE, MCLC_MODID_QE, 0, 0, 0},

        {PORT_ENABLE, MCLC_UNIT_QE, 68, 0, 0, 0},
        {PORT_ENABLE_RANGE, MCLC_UNIT_QE, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},

        {MODULE_ENABLE_SET, MCLC_UNIT_LCM0, MCLC_MODID_QE, 1, 0, 0, 0},
        {-1, 0, 0, 0, 0, 0, 0},
    };


#ifdef BCM_BM9600_SUPPORT
/*
 ***************************************************
 *
 *  Polaris LC in chassis
 *
 ***************************************************
 */

#define QE2000_SCI_PORT_BASE   68

dev_cmd_rec pl_lc_tmemode_cmd_tbl[] = {
        {-1, 0, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
        {-1, 2, 0, 0, 0, 0, 0},
        {-1, 3, 0, 0, 0, 0, 0},
        {-1, 4, 0, 0, 0, 0, 0},
};

dev_cmd_rec fe2kxt_lc_tmemode_cmd_tbl[] = {
        {SET_MODID, 1, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
        {-1, 2, 0, 0, 0, 0, 0},
};

dev_cmd_rec qe2k_bscrn_lc_tmemode_cmd_tbl[] = {
        {-1, 0, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
};

dev_cmd_rec pl_lc_chassis_cmd_tbl[] = {

        {SET_MODID,         PLLC_UNIT_QE0, AUTO_MODID_QE0,  0, 0, 0},
        {SET_MODID,         PLLC_UNIT_QE1, AUTO_MODID_QE1,  0, 0, 0},
        {SET_MODID,         PLLC_UNIT_FE0, AUTO_MODID_FE0,  0, 0, 0},
        {SET_MODID,         PLLC_UNIT_FE1, AUTO_MODID_FE1,  0, 0, 0},

        {LCM_MODE_SET,      PLLC_UNIT_BM96, lcmModeHwSelect},

        /* set module protocol */
        { SET_MODULE_PROTOCOL, PLLC_UNIT_BM96, AUTO_MODID_QE0, bcmModuleProtocol1, 0, 0, 0 },
#ifndef _PL_FAB_NO_QE1_
        { SET_MODULE_PROTOCOL, PLLC_UNIT_BM96, AUTO_MODID_QE1, bcmModuleProtocol2, 0, 0, 0 },
#endif /* _PL_FAB_NO_QE1_ */

        /* set port ability */
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 4, 21, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 40,55, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 58,59, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 60, 61, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 64, 95, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 0, 1, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#else
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 60, 61, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 64, 95, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 0, 1, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#endif /* _PL_FAB_NO_QE1_ */


#ifndef _PL_FAB_NO_QE1_
#if 000 /* disable transmit equalization setting until they are known  */
        /* when including this once known, need to add a parameter for */
        /* each slot we are located in and these.                      */
        /* Only for 6G+ links */
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 72, 0, 4},   /* SFIs to backplane */
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 73, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 74, 0, 8},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 75, 0,10},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 76, 0, 8},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 77, 0, 4},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 78, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 79, 0, 8},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 89, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 90, 0, 4},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 88, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 92, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 93, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 94, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 95, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 71, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96,  0, 0, 4},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96,  1, 0, 2},
#endif
#endif
        /* lcm from qe1(software name) to backplane */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 40, 72}, /* HC 18/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 41, 73}, /* HC 18/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 42, 74}, /* HC 18/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 43, 75}, /* HC 18/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 44, 76}, /* HC 19/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 45, 77}, /* HC 19/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 46, 78}, /* HC 19/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 47, 79}, /* HC 19/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 51, 89}, /* HC 22/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 50, 90}, /* HC 22/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 49, 88}, /* HC 22/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 48, 92}, /* HC 23/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 55, 93}, /* HC 23/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 54, 94}, /* HC 23/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 53, 95}, /* HC 23/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 52, 71}, /* HC 17/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 58,  0}, /* HC 0/0  */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 59,  1},

        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 40, 61}, /* HC 18/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 41, 64}, /* HC 18/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 42, 65}, /* HC 18/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 43, 66}, /* HC 18/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 44, 67}, /* HC 19/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 45, 68}, /* HC 19/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 46, 69}, /* HC 19/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 47, 70}, /* HC 19/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 51, 81}, /* HC 22/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 50, 80}, /* HC 22/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 49, 82}, /* HC 22/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 48, 86}, /* HC 23/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 55, 85}, /* HC 23/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 54, 84}, /* HC 23/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 53, 83}, /* HC 23/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 52, 60}, /* HC 17/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 58, 87}, /* HC 0/0  */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 59, 91},


#ifndef _PL_FAB_NO_QE1_
        /* lcm from qe0(software name) to backplane */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  4, 72 }, /* HC 18/0 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  5, 73 }, /* HC 18/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  6, 74 }, /* HC 18/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  7, 75 }, /* HC 18/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  8, 76 }, /* HC 19/0 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  9, 77 }, /* HC 19/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 10, 78 }, /* HC 19/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 11, 79 }, /* HC 19/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 12, 89 }, /* HC 22/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 13, 90 }, /* HC 22/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 14, 88 }, /* HC 22/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 15, 92 }, /* HC 23/0 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 16, 93 }, /* HC 23/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 17, 94 }, /* HC 23/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 18, 95 }, /* HC 23/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 19, 71 }, /* HC 17/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 20,  0 }, /* HC 0/0  */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 21,  1 }, /* HC 0/1  */

        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  4, 61 }, /* HC 18/0 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  5, 64 }, /* HC 18/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  6, 65 }, /* HC 18/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  7, 66 }, /* HC 18/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  8, 67 }, /* HC 19/0 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  9, 68 }, /* HC 19/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 10, 69 }, /* HC 19/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 11, 70 }, /* HC 19/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 12, 81 }, /* HC 22/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 13, 80 }, /* HC 22/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 14, 82 }, /* HC 22/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 15, 86 }, /* HC 23/0 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 16, 85 }, /* HC 23/1 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 17, 84 }, /* HC 23/2 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 18, 83 }, /* HC 23/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 19, 60 }, /* HC 17/3 */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 20, 87 }, /* HC 0/0  */
        { LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 21, 91 }, /* HC 0/1  */
#endif /* _PL_FAB_NO_QE1_ */

        { XBAR_ENABLE_SET,  PLLC_UNIT_QE0 , 0x3ffff, 0, 0, 0, 0 },
        { XBAR_ENABLE_SET,  PLLC_UNIT_QE1 , 0x3ffff, 0, 0, 0, 0 },

        {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
        {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE + 1, 0, 0},
        {PORT_ENABLE_RANGE, PLLC_UNIT_QE1, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
        {PORT_ENABLE_RANGE, PLLC_UNIT_QE1, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE + 1, 0, 0},

        /* GNATS 22312, enable links to QE prior to links to backplane */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96,  4, 21, 0, 0},   /* SFIs to QE0 */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96, 40, 55, 0, 0},   /* SFIs to QE1 */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96, 58, 59, 0, 0},   /* SFIs to QE1 */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96,  0,  1, 0, 0},   /* SFIs to backplane */
        /* GNATS 21469, SFIs to backplane start at 60 */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96, 60, 95, 0, 0},   /* SFIs to backplane */

        /* lcm from qe1(software name) to backplane */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 40, 72}, /* HC 18/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 41, 73}, /* HC 18/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 42, 74}, /* HC 18/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 43, 75}, /* HC 18/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 44, 76}, /* HC 19/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 45, 77}, /* HC 19/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 46, 78}, /* HC 19/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 47, 79}, /* HC 19/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 51, 89}, /* HC 22/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 50, 90}, /* HC 22/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 49, 88}, /* HC 22/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 48, 92}, /* HC 23/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 55, 93}, /* HC 23/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 54, 94}, /* HC 23/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 53, 95}, /* HC 23/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 52, 71}, /* HC 17/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 58,  0}, /* HC 0/0  */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 1, 59,  1},

        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 40, 61}, /* HC 18/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 41, 64}, /* HC 18/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 42, 65}, /* HC 18/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 43, 66}, /* HC 18/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 44, 67}, /* HC 19/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 45, 68}, /* HC 19/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 46, 69}, /* HC 19/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 47, 70}, /* HC 19/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 51, 81}, /* HC 22/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 50, 80}, /* HC 22/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 49, 82}, /* HC 22/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 48, 86}, /* HC 23/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 55, 85}, /* HC 23/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 54, 84}, /* HC 23/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 53, 83}, /* HC 23/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 52, 60}, /* HC 17/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 58, 87}, /* HC 0/0  */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE0, 0, 59, 91},


#ifndef _PL_FAB_NO_QE1_
        /* lcm from qe0(software name) to backplane */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  4, 72 }, /* HC 18/0 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  5, 73 }, /* HC 18/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  6, 74 }, /* HC 18/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  7, 75 }, /* HC 18/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  8, 76 }, /* HC 19/0 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1,  9, 77 }, /* HC 19/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 10, 78 }, /* HC 19/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 11, 79 }, /* HC 19/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 12, 89 }, /* HC 22/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 13, 90 }, /* HC 22/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 14, 88 }, /* HC 22/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 15, 92 }, /* HC 23/0 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 16, 93 }, /* HC 23/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 17, 94 }, /* HC 23/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 18, 95 }, /* HC 23/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 19, 71 }, /* HC 17/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 20,  0 }, /* HC 0/0  */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 1, 21,  1 }, /* HC 0/1  */

        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  4, 61 }, /* HC 18/0 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  5, 64 }, /* HC 18/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  6, 65 }, /* HC 18/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  7, 66 }, /* HC 18/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  8, 67 }, /* HC 19/0 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0,  9, 68 }, /* HC 19/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 10, 69 }, /* HC 19/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 11, 70 }, /* HC 19/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 12, 81 }, /* HC 22/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 13, 80 }, /* HC 22/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 14, 82 }, /* HC 22/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 15, 86 }, /* HC 23/0 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 16, 85 }, /* HC 23/1 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 17, 84 }, /* HC 23/2 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 18, 83 }, /* HC 23/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 19, 60 }, /* HC 17/3 */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 20, 87 }, /* HC 0/0  */
        { LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96, AUTO_MODID_QE1, 0, 21, 91 }, /* HC 0/1  */
#endif /* _PL_FAB_NO_QE1_ */

        {-1, 0, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
        {-1, 2, 0, 0, 0, 0, 0},
        {-1, 3, 0, 0, 0, 0, 0},
        {-1, 4, 0, 0, 0, 0, 0},
};
#endif /* BCM_BM9600_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
/* Single Node Sirius FIC test sirius SVK bringup sequence */
dev_cmd_rec sirius_ipass_fic_cmd_tbl[] = {

        {SET_MODID,         SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID,  0, 0, 0},

#ifdef TEST_3125
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol5, 0, 0, 0 },
#else
        /* set module protocol */
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol3, 0, 0, 0 },
#endif
        /* set port ability */
        { PORT_CONTROL, SS_IPASS_UNIT, 31, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },

        { PORT_CONTROL, SS_IPASS_UNIT, 32, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },

#ifdef TEST_3125	
        { PORT_CONTROL_RANGE, SS_IPASS_UNIT, 9, 30, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#else
        { PORT_CONTROL_RANGE, SS_IPASS_UNIT, 9, 30, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#endif
        { XBAR_ENABLE_SET,  SS_IPASS_UNIT , 0x3ffff, 0, 0, 0, 0 },

        {PORT_ENABLE_RANGE, SS_IPASS_UNIT, SIRIUS_SCI_PORT_BASE, SIRIUS_SCI_PORT_BASE + 1, 0, 0},
        {PORT_ENABLE_RANGE, SS_IPASS_UNIT, SIRIUS_SFI_PORT_BASE, SIRIUS_SFI_PORT_BASE + 21, 0, 0},

        {-1, 0, 0, 0, 0, 0, 0},
};

/* Single Node Sirius Hybrid test sirius SVK bringup sequence  */
dev_cmd_rec sirius_ipass_hybrid_cmd_tbl[] = {

        {SET_MODID,         SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID,  0, 0, 0},
        /* set module protocol */
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol4, 0, 0, 0 },

        /* set port ability */
        { PORT_CONTROL, SS_IPASS_UNIT, 31, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },
        { PORT_CONTROL, SS_IPASS_UNIT, 32, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },


#ifndef TEST_3125
#ifndef GNATS36884_TEST_CONFIG

        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_fic_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_lcl_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_LOOPBACK, 0 },
#else
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
#endif
#endif /* TEST_3125 */

#ifdef TEST_3125
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
#endif
	/* unmap all xbars */
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  9 /* port 4-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 10 /* port 4-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 11 /* port 5-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 12 /* port 5-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 13 /* port 5-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 14 /* port 5-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 15 /* port 6-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 16 /* port 6-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 17 /* port 6-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 18 /* port 6-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 19 /* port 7-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 20 /* port 7-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 21 /* port 7-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 22 /* port 7-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 23 /* port 8-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 24 /* port 8-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 25 /* port 8-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 26 /* port 8-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 27 /* port 9-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 28 /* port 9-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 29 /* port 9-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 30 /* port 9-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 31 /* sfi_sci0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 32 /* sfi_sci1 */},

	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 0 /* xbar */,   9 /* port 4-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 1 /* xbar */,  10 /* port 4-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 2 /* xbar */,  11 /* port 5-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 3 /* xbar */,  12 /* port 5-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 4 /* xbar */,  13 /* port 5-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 5 /* xbar */,  14 /* port 5-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 6 /* xbar */,  15 /* port 6-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 7 /* xbar */,  16 /* port 6-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 8 /* xbar */,  17 /* port 6-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 9 /* xbar */,  18 /* port 6-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 10 /* xbar */, 19 /* port 7-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 11 /* xbar */, 20 /* port 7-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 12 /* xbar */, 21 /* port 7-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 13 /* xbar */, 22 /* port 7-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 14 /* xbar */, 23 /* port 8-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 15 /* xbar */, 24 /* port 8-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 16 /* xbar */, 25 /* port 8-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 17 /* xbar */, 26 /* port 8-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 18 /* xbar */, 27 /* port 9-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 19 /* xbar */, 28 /* port 9-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 20 /* xbar */, 29 /* port 9-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 21 /* xbar */, 30 /* port 9-3 */},

        /* { XBAR_ENABLE_SET,  SS_IPASS_UNIT , 0x1ff, 0, 0, 0, 0 }, */
        {-1, 0, 0, 0, 0, 0, 0},
};


/* Multi Node Sirius/Qe2k interop test sirius SVK bringup sequence  */
dev_cmd_rec sirius_ipass_qe2k_ss_interop_fic_cmd_tbl[] = {
        {SET_MODID, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID,  0, 0, 0},

        /* set module protocol for both nodes */
	/*        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol4, 0, 0, 0 },  XXX */
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol3, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, bcmModuleProtocol1, 0, 0, 0 },

        /* set port ability for scis */
        { PORT_CONTROL, SS_IPASS_UNIT, 31, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 },
        { PORT_CONTROL, SS_IPASS_UNIT, 32, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 },

        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },

	/* set port ability for qe sfis */
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&qe2000_sfi_base_gport, (int)&qe2000_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

        {PORT_ENABLE_RANGE, SS_IPASS_UNIT, SIRIUS_SCI_PORT_BASE, SIRIUS_SCI_PORT_BASE + 1, 0, 0},
        {PORT_ENABLE_RANGE, SS_IPASS_UNIT, SIRIUS_SFI_PORT_BASE, SIRIUS_SFI_PORT_BASE + 21, 0, 0},

	/* enable crossbars */
        { XBAR_ENABLE_SET,  SS_IPASS_UNIT , 0xffff, 0, 0, 0, 0 },

        {-1, 0, 0, 0, 0, 0, 0},
};


/* Multi Node Sirius/Qe2k interop test sirius SVK bringup sequence  */
dev_cmd_rec sirius_ipass_qe2k_ss_interop_hybrid_cmd_tbl[] = {
        {SET_MODID, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID,  0, 0, 0},


        /* set module protocol for both nodes */
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol4, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, bcmModuleProtocol1, 0, 0, 0 },

        /* set port ability for scis */
        { PORT_CONTROL, SS_IPASS_UNIT, 31, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },
        { PORT_CONTROL, SS_IPASS_UNIT, 32, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },


#ifndef TEST_3125
#ifndef GNATS36884_TEST_CONFIG
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_fic_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_lcl_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#else
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
#endif
#endif /* TEST_3125 */

#ifdef TEST_3125
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&sirius_sfi_base_gport, (int)&sirius_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
#endif

	/* set port ability for qe sfis */
        { GPORT_CONTROL_RANGE, SS_IPASS_UNIT, (int)&qe2000_sfi_base_gport, (int)&qe2000_sfi_end_gport, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

	/* unmap all sirius hybrid xbars */
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  9 /* port 4-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 10 /* port 4-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 11 /* port 5-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 12 /* port 5-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 13 /* port 5-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 14 /* port 5-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 15 /* port 6-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 16 /* port 6-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 17 /* port 6-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 18 /* port 6-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 19 /* port 7-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 20 /* port 7-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 21 /* port 7-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 22 /* port 7-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 23 /* port 8-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 24 /* port 8-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 25 /* port 8-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 26 /* port 8-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 27 /* port 9-0 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 28 /* port 9-1 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 29 /* port 9-2 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 30 /* port 9-3 */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 31 /* port 4-0 sfi_sci */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */, 32 /* port 4-1 sfi_sci */},

#ifndef GNATS36884_TEST_CONFIG
	/* 5 A/A + 13 A/B channels, 10 + 13= 23 logical crossbars */
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 0 /* xbar */,   9 /* port 4-2 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 1 /* xbar */,  10 /* port 4-3 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 2 /* xbar */,  11 /* port 5-0 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 3 /* xbar */,  12 /* port 5-1 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 4 /* xbar */,  13 /* port 5-2 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 5 /* xbar */,  14 /* port 5-3 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 6 /* xbar */,  15 /* port 6-0 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 7 /* xbar */,  16 /* port 6-1 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 8 /* xbar */,  17 /* port 6-2 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 9 /* xbar */,  18 /* port 6-3 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 10 /* xbar */, 19 /* port 7-0 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 11 /* xbar */, 19 /* port 7-0 even */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 12 /* xbar */, 20 /* port 7-1 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 13 /* xbar */, 20 /* port 7-1 even  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 14 /* xbar */, 21 /* port 7-2 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 15 /* xbar */, 21 /* port 7-2 even  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 16 /* xbar */, 22 /* port 7-3 odd  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 17 /* xbar */, 22 /* port 7-3 even  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 18 /* xbar */, 23 /* port 8-0 odd/even ss only  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 19 /* xbar */, 24 /* port 8-1 odd/even ss only  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 20 /* xbar */, 25 /* port 8-2 odd/even ss only  */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 21 /* xbar */, 26 /* port 8-3 odd/even ss only  */},
#else
        /* 18 Sirius to QE2k interop A/B channels requiring 1 logical xbar per channel */
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 0 /* xbar */,   9 /* port 4-2 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 1 /* xbar */,  10 /* port 4-3 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 2 /* xbar */,  11 /* port 5-0 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 3 /* xbar */,  12 /* port 5-1 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 4 /* xbar */,  13 /* port 5-2 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 5 /* xbar */,  14 /* port 5-3 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 6 /* xbar */,  15 /* port 6-0 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 7 /* xbar */,  16 /* port 6-1 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 8 /* xbar */,  17 /* port 6-2 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 9 /* xbar */,  18 /* port 6-3 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 10 /* xbar */, 19 /* port 7-0 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 11 /* xbar */, 20 /* port 7-1 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 12 /* xbar */, 21 /* port 7-2 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 13 /* xbar */, 22 /* port 7-3 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 14 /* xbar */, 23 /* port 8-0 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 15 /* xbar */, 24 /* port 8-1 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 16 /* xbar */, 25 /* port 8-2 odd/even local */},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 17 /* xbar */, 26 /* port 8-3 odd/even local */},

#if 000
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -2 /* xbar */,  9 /* port 4-2 odd/even local */},
#endif
#endif
	/* unmap all QE2000 lxbars */
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 1},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 2},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 3},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 4},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 5},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 6},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 7},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 8},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */, QE2000_SFI_PORT_BASE + 9},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 10},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 11},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 12},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 13},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 14},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 15},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 16},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, -1 /* xbar */,QE2000_SFI_PORT_BASE + 17},

	/* 18 QE2k channels */
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 0 /* xbar */, QE2000_SFI_PORT_BASE},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 1 /* xbar */, QE2000_SFI_PORT_BASE + 1},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 2 /* xbar */, QE2000_SFI_PORT_BASE + 2},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 3 /* xbar */, QE2000_SFI_PORT_BASE + 3},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 4 /* xbar */, QE2000_SFI_PORT_BASE + 4},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 5 /* xbar */, QE2000_SFI_PORT_BASE + 5},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 6 /* xbar */, QE2000_SFI_PORT_BASE + 6},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 7 /* xbar */, QE2000_SFI_PORT_BASE + 7},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 8 /* xbar */, QE2000_SFI_PORT_BASE + 8},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 9 /* xbar */, QE2000_SFI_PORT_BASE + 9},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 10 /* xbar */,QE2000_SFI_PORT_BASE + 10},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 11 /* xbar */,QE2000_SFI_PORT_BASE + 11},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 12 /* xbar */,QE2000_SFI_PORT_BASE + 12},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 13 /* xbar */,QE2000_SFI_PORT_BASE + 13},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 14 /* xbar */,QE2000_SFI_PORT_BASE + 14},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 15 /* xbar */,QE2000_SFI_PORT_BASE + 15},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 16 /* xbar */,QE2000_SFI_PORT_BASE + 16},
	{ LGL_XBAR_MAPPING_SET, SS_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_MODID, 0 /* arbiter id */, 17 /* xbar */,QE2000_SFI_PORT_BASE + 17},

        {PORT_ENABLE_RANGE, SS_IPASS_UNIT, SIRIUS_SCI_PORT_BASE, SIRIUS_SCI_PORT_BASE + 1, 0, 0},
        {PORT_ENABLE_RANGE, SS_IPASS_UNIT, SIRIUS_SFI_PORT_BASE, SIRIUS_SFI_PORT_BASE + 21, 0, 0},

	/* enable crossbars */
        { XBAR_ENABLE_SET,  SS_IPASS_UNIT , 0xffff, 0, 0, 0, 0 },

        {-1, 0, 0, 0, 0, 0, 0},
};

#endif

#ifdef BCM_BM9600_SUPPORT
dev_cmd_rec fe2kxt_lc_chassis_cmd_tbl[] = {
        {SET_MODID,         PLLC_UNIT_QE0, AUTO_MODID_QE0,  0, 0, 0},
        {SET_MODID,         PLLC_UNIT_FE0, AUTO_MODID_FE0,  0, 0, 0},

        {LCM_MODE_SET,      PLLC_UNIT_BM96_FE2KXT_LC, lcmModeHwSelect},

        /* set module protocol */
        { SET_MODULE_PROTOCOL, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, bcmModuleProtocol1, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, PLLC_UNIT_QE0, PL_SIRIUS_IPASS_MODID, bcmModuleProtocol3, 0, 0, 0 },
        { SET_MODULE_PROTOCOL, PLLC_UNIT_QE0, AUTO_MODID_QE0, bcmModuleProtocol1, 0, 0, 0 },

#ifdef SV_QE2000_SIRIUS_SETUP
	/* following ports are based on SV setup with a shim card */
        /* set port ability */
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 4, 21, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 64,67, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 72,85, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },


        /* NOTE: this lcm config is based on a shim card, not really matching to the board SPEC  */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 4,  64}, /* HC 16/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 5,  65}, /* HC 16/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 6,  66}, /* HC 16/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 7,  67}, /* HC 16/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 8,  72}, /* HC 18/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 9,  73}, /* HC 18/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 10, 74}, /* HC 18/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 11, 75}, /* HC 18/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 12, 76}, /* HC 19/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 13, 77}, /* HC 19/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 14, 78}, /* HC 19/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 15, 79}, /* HC 19/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 16, 80}, /* HC 20/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 17, 81}, /* HC 20/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 18, 82}, /* HC 20/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 19, 83}, /* HC 20/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 20, 84}, /* HC 21/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 21, 85}, /* HC 21/1 */

        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 4,  64}, /* HC 16/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 5,  65}, /* HC 16/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 6,  66}, /* HC 16/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 7,  67}, /* HC 16/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 8,  72}, /* HC 18/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 9,  73}, /* HC 18/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 10, 74}, /* HC 18/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 11, 75}, /* HC 18/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 12, 76}, /* HC 19/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 13, 77}, /* HC 19/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 14, 78}, /* HC 19/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 15, 79}, /* HC 19/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 16, 80}, /* HC 20/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 17, 81}, /* HC 20/1 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 18, 82}, /* HC 20/2 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 19, 83}, /* HC 20/3 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 20, 84}, /* HC 21/0 */
        {LCM_BM9600_XCFG_SET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 21, 85}, /* HC 21/1 */

        { XBAR_ENABLE_SET,  PLLC_UNIT_QE0 , 0x3ffff, 0, 0, 0, 0 },

        {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
        {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE + 1, 0, 0},

        /* GNATS 22312, enable links to QE prior to links to backplane */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 4,  21, 0, 0},   /* SFIs to QE0 */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 64, 67, 0, 0},   /* SFIs to backplane */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 72, 85, 0, 0},   /* SFIs to backplane */

        /* lcm from qe1(software name) to backplane */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 4,  64}, /* HC 16/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 5,  65}, /* HC 16/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 6,  66}, /* HC 16/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 7,  67}, /* HC 16/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 8,  72}, /* HC 18/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 9,  73}, /* HC 18/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 10, 74}, /* HC 18/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 11, 75}, /* HC 18/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 12, 76}, /* HC 19/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 13, 77}, /* HC 19/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 14, 78}, /* HC 19/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 15, 79}, /* HC 19/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 16, 80}, /* HC 20/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 17, 81}, /* HC 20/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 18, 82}, /* HC 20/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 19, 83}, /* HC 20/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 20, 84}, /* HC 21/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 1, 21, 85}, /* HC 21/1 */

        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 4,  64}, /* HC 16/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 5,  65}, /* HC 16/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 6,  66}, /* HC 16/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 7,  67}, /* HC 16/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 8,  72}, /* HC 18/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 9,  73}, /* HC 18/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 10, 74}, /* HC 18/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 11, 75}, /* HC 18/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 12, 76}, /* HC 19/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 13, 77}, /* HC 19/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 14, 78}, /* HC 19/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 15, 79}, /* HC 19/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 16, 80}, /* HC 20/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 17, 81}, /* HC 20/1 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 18, 82}, /* HC 20/2 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 19, 83}, /* HC 20/3 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 20, 84}, /* HC 21/0 */
        {LCM_BM9600_XCFG_GET, PLLC_UNIT_BM96_FE2KXT_LC, AUTO_MODID_QE0, 0, 21, 85}, /* HC 21/1 */
#endif

        {SWITCH_EVENT_REGISTER, PLLC_UNIT_QE0, 0, 0, 0, 0},

        {-1, 0, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
        {-1, 2, 0, 0, 0, 0, 0},
};
#endif

#ifdef BCM_BM9600_SUPPORT
/* following table setup protocols, port ability, and crossbar for 2 nodes
 * one for sirius and the other for qe2000. sirius is in FIC mode
 */
dev_cmd_rec polaris_ipass_qe2k_ss_interop_cmd_tbl[] = {
        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol3, 0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, bcmModuleProtocol1, 0, 0, 0 },

        /* set port ability for polaris port connected to sirius sfi ports */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT,  8, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },

	/* set port ability for polaris port connected to qe2000 sfi ports */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 36, 53, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

	/* set port ability for polaris port connected to sirius sci port */
        { PORT_CONTROL, PL_IPASS_UNIT, PL_SIRIUS_IPASS_NODE, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 },

	/* set port ability for polaris port connected to qe2000 sci port */
        { PORT_CONTROL, PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 },

        /* GNATS 22312, control links to QE prior to links to backplane */
        /* enable port connected to sirius sci port */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, PL_SIRIUS_IPASS_NODE,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, PL_SIRIUS_IPASS_NODE,  0, 0, 0 },

        /* enable port connected to qe2000 sci port */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC,  0, 0, 0 },

	/* enable ports connected to sirius sfi ports */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT,  8, 23, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        { PORT_ENABLE_RANGE, PL_IPASS_UNIT,  8, 23, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

	/* enable ports connected to qe2000 sfi ports */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT,  36, 53, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT,  36, 53, 0, 0 },

        /* enable crossbars on bme, default to enable all 18 links of qe2k */
        { XBAR_ENABLE_SET,   PL_IPASS_UNIT, 0x3ffff, 0, 0, 0, 0 },

	/************************************* lxbar ********** node ********* sfi *****/
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&PL_IPASS_SIRIUS_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&PL_IPASS_SIRIUS_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&PL_IPASS_SIRIUS_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&PL_IPASS_SIRIUS_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&PL_IPASS_SIRIUS_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&PL_IPASS_SIRIUS_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&PL_IPASS_SIRIUS_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&PL_IPASS_SIRIUS_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&PL_IPASS_SIRIUS_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&PL_IPASS_SIRIUS_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&PL_IPASS_SIRIUS_MODID, 16, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&PL_IPASS_SIRIUS_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&PL_IPASS_SIRIUS_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&PL_IPASS_SIRIUS_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&PL_IPASS_SIRIUS_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&PL_IPASS_SIRIUS_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&PL_IPASS_SIRIUS_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&PL_IPASS_SIRIUS_MODID, 15, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 18, (int)&PL_IPASS_SIRIUS_MODID,  8, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 19, (int)&PL_IPASS_SIRIUS_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 20, (int)&PL_IPASS_SIRIUS_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 21, (int)&PL_IPASS_SIRIUS_MODID, 11, 0, 0},

        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 36, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 37, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 38, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 39, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 40, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 41, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 42, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 43, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 44, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 45, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 46, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 47, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 48, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 49, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 50, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 51, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 52, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 53, 0, 0},

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 1, 0, 0, 0},
        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, 1, 0, 0, 0},

        {REDUND_REGISTER, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        { -1, 0, 0, 0, 0, 0, 0 },
};

/* following table setup protocols, port ability, and crossbar for 2 nodes
 * one for sirius and the other for qe2000. sirius is in Hybrid mode
 */
dev_cmd_rec polaris_ipass_qe2k_ss_interop_hybrid_cmd_tbl[] = {
        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol4, 0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, bcmModuleProtocol1, 0, 0, 0 },

#ifndef GNATS36884_TEST_CONFIG
        /* set port ability for polaris port connected to sirius sfi ports */
        /* set port ability, only 11 links are used */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT,  8, 17, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 20, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
#else
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 16, 17, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 20, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI_LOCAL, 0 },
#endif
	/* set port ability for polaris port connected to qe2000 sfi ports */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 36, 53, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

	/* set port ability for polaris port connected to sirius sci port */
        { PORT_CONTROL, PL_IPASS_UNIT, PL_SIRIUS_IPASS_NODE, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 },

	/* set port ability for polaris port connected to qe2000 sci port */
        { PORT_CONTROL, PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 },

        /* enable port connected to sirius sci port */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, PL_SIRIUS_IPASS_NODE,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, PL_SIRIUS_IPASS_NODE,  0, 0, 0 },

        /* enable port connected to qe2000 sci port */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC,  0, 0, 0 },

	/* enable ports connected to sirius sfi ports */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 16, 17, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 20, 23, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 16, 17, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 20, 23, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

	/* enable ports connected to qe2000 sfi ports */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT,  36, 53, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT,  36, 53, 0, 0 },

        /* enable links on bme, only enable first 11 links */
        { XBAR_ENABLE_SET,   PL_IPASS_UNIT, 0x1ff, 0, 0, 0, 0 },

#ifndef GNATS36884_TEST_CONFIG
	/* sequencing for hybrid dual channel links, one sfi port on sirius connect to two sfi ports on qe2k */
	/************************************* lxbar ********** node ********* sfi *****/
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&PL_IPASS_SIRIUS_MODID, 29, 0, 0}, /* a/b */
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&PL_IPASS_SIRIUS_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&PL_IPASS_SIRIUS_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&PL_IPASS_SIRIUS_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&PL_IPASS_SIRIUS_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&PL_IPASS_SIRIUS_MODID, 27, 0, 0}, 
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&PL_IPASS_SIRIUS_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&PL_IPASS_SIRIUS_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&PL_IPASS_SIRIUS_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&PL_IPASS_SIRIUS_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  10, (int)&PL_IPASS_SIRIUS_MODID, 16, 0, 0}, /* a/a */
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  11, (int)&PL_IPASS_SIRIUS_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  12, (int)&PL_IPASS_SIRIUS_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  13, (int)&PL_IPASS_SIRIUS_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  14, (int)&PL_IPASS_SIRIUS_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  15, (int)&PL_IPASS_SIRIUS_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  16, (int)&PL_IPASS_SIRIUS_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  17, (int)&PL_IPASS_SIRIUS_MODID, 15, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  18, (int)&PL_IPASS_SIRIUS_MODID,  8, 0, 0}, /* ss only a/a */
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  19, (int)&PL_IPASS_SIRIUS_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  20, (int)&PL_IPASS_SIRIUS_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  21, (int)&PL_IPASS_SIRIUS_MODID, 11, 0, 0},
#else
	/* A/B local config */
	/************************************* lxbar ********** node ********* sfi *****/
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&PL_IPASS_SIRIUS_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&PL_IPASS_SIRIUS_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&PL_IPASS_SIRIUS_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&PL_IPASS_SIRIUS_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&PL_IPASS_SIRIUS_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&PL_IPASS_SIRIUS_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&PL_IPASS_SIRIUS_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&PL_IPASS_SIRIUS_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&PL_IPASS_SIRIUS_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&PL_IPASS_SIRIUS_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&PL_IPASS_SIRIUS_MODID, 16, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&PL_IPASS_SIRIUS_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&PL_IPASS_SIRIUS_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&PL_IPASS_SIRIUS_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&PL_IPASS_SIRIUS_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&PL_IPASS_SIRIUS_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&PL_IPASS_SIRIUS_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&PL_IPASS_SIRIUS_MODID, 15, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 18, (int)&PL_IPASS_SIRIUS_MODID,  8, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 19, (int)&PL_IPASS_SIRIUS_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 20, (int)&PL_IPASS_SIRIUS_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 21, (int)&PL_IPASS_SIRIUS_MODID, 11, 0, 0},
#endif

        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 36, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 37, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 38, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 39, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 40, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 41, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 42, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 43, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 44, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 45, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 46, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 47, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 48, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 49, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 50, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 51, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 52, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 53, 0, 0},

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 1, 0, 0, 0},
        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, 1, 0, 0, 0},

        {REDUND_REGISTER, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        { -1, 0, 0, 0, 0, 0, 0 },
};

dev_cmd_rec polaris_ipass_sv_qe2k_ss_interop_cmd_tbl[] = {

        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol3, 0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol3, 0, 0, 0 },

        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, bcmModuleProtocol2, 0, 0, 0 },

        /* set port ability */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT,  8, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },

        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 36, 53, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 }, /* SFI lc1 QE0 */

        { PORT_CONTROL, PL_IPASS_UNIT, 28, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI0 lc0 QE0 */

        { PORT_CONTROL, PL_IPASS_UNIT, 32, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI0 lc1 QE0 */

        /* enable ports */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, 28,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, 28,  0, 0, 0 },

        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC,  0, 0, 0 },


        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT,  8, 23, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        { PORT_ENABLE_RANGE, PL_IPASS_UNIT,  8, 23, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT,  36, 53, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT,  36, 53, 0, 0 },

        /* enable links on bme  */
        { XBAR_ENABLE_SET,   PL_IPASS_UNIT, 0x3ffff, 0, 0, 0, 0 },

	/************************************* lxbar ********** node ********* sfi *****/
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&PL_IPASS_SIRIUS_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&PL_IPASS_SIRIUS_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&PL_IPASS_SIRIUS_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&PL_IPASS_SIRIUS_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&PL_IPASS_SIRIUS_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&PL_IPASS_SIRIUS_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&PL_IPASS_SIRIUS_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&PL_IPASS_SIRIUS_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&PL_IPASS_SIRIUS_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&PL_IPASS_SIRIUS_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&PL_IPASS_SIRIUS_MODID, 16, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&PL_IPASS_SIRIUS_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&PL_IPASS_SIRIUS_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&PL_IPASS_SIRIUS_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&PL_IPASS_SIRIUS_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&PL_IPASS_SIRIUS_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&PL_IPASS_SIRIUS_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&PL_IPASS_SIRIUS_MODID, 15, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 18, (int)&PL_IPASS_SIRIUS_MODID,  8, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 19, (int)&PL_IPASS_SIRIUS_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 20, (int)&PL_IPASS_SIRIUS_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 21, (int)&PL_IPASS_SIRIUS_MODID, 11, 0, 0},

        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 36, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 37, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 38, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 39, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 40, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 41, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 42, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 43, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 44, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 45, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 46, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 47, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 48, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 49, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 50, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 51, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 52, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 53, 0, 0},

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&FE2KXT_PL_FAB_LC0_QE0_MODID_ON_FC, 1, 0, 0, 0},
        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, 1, 0, 0, 0},

        {REDUND_REGISTER, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        { -1, 0, 0, 0, 0, 0, 0 },
};

dev_cmd_rec polaris_ipass_fic_cmd_tbl[] = {

#ifdef TEST_3125
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol5, 0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol5, 0, 0, 0 },
#else
        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol3, 0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol3, 0, 0, 0 },
#endif

#ifdef TEST_3125
        /* set port ability */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT,  8, 23, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#else
        /* set port ability */
         { PORT_CONTROL_RANGE, PL_IPASS_UNIT,  8, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
         { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
         { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#endif
         { PORT_CONTROL, PL_IPASS_UNIT,  28, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },
         { PORT_CONTROL, PL_IPASS_UNIT,  24, bcmPortControlAbility, BCM_PORT_ABILITY_SFI_SCI, 0 },

        /* enable ports */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, 28,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, 28,  0, 0, 0 },

        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, 24,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, 24,  0, 0, 0 },

        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT,  8, 23, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },


        { PORT_ENABLE_RANGE, PL_IPASS_UNIT,  8, 23, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        /* enable links on bme  */
        { XBAR_ENABLE_SET,   PL_IPASS_UNIT, 0x3ffff, 0, 0, 0, 0 },

#ifndef TEST_3125
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  29},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  30},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  31},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  25},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  26},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  27},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  20},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  21},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  22},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  23},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  16},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  17},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  18},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  19},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  12},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  13},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  14},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  15},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,   8},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,   9},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  10},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  11},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  28},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, -1 /* xbar */,  24},

	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  0 /* xbar */,  29},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  1/* xbar */,   30},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  2 /* xbar */,  31},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  3 /* xbar */,  25},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  4 /* xbar */,  26},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  5 /* xbar */,  27},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  6 /* xbar */,  20},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  7 /* xbar */,  21},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  8 /* xbar */,  22},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */,  9 /* xbar */,  23},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 10 /* xbar */,  16},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 11 /* xbar */,  17},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 12 /* xbar */,  18},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 13 /* xbar */,  19},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 14 /* xbar */,  12},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 15 /* xbar */,  13},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 16 /* xbar */,  14},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 17 /* xbar */,  15},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 18 /* xbar */,   8},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 19 /* xbar */,   9},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 20 /* xbar */,  10},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 21 /* xbar */,  11},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 22 /* xbar */,  28},
	{ LGL_XBAR_MAPPING_SET, PL_IPASS_UNIT, PL_SIRIUS_IPASS_MODID, 0 /* arbiter id */, 23 /* xbar */,  24},
#endif

	/************************************* lxbar ********** node ********* sfi *****/
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&PL_IPASS_SIRIUS_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&PL_IPASS_SIRIUS_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&PL_IPASS_SIRIUS_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&PL_IPASS_SIRIUS_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&PL_IPASS_SIRIUS_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&PL_IPASS_SIRIUS_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&PL_IPASS_SIRIUS_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&PL_IPASS_SIRIUS_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&PL_IPASS_SIRIUS_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&PL_IPASS_SIRIUS_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&PL_IPASS_SIRIUS_MODID, 16, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&PL_IPASS_SIRIUS_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 12, (int)&PL_IPASS_SIRIUS_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 13, (int)&PL_IPASS_SIRIUS_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 14, (int)&PL_IPASS_SIRIUS_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 15, (int)&PL_IPASS_SIRIUS_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 16, (int)&PL_IPASS_SIRIUS_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 17, (int)&PL_IPASS_SIRIUS_MODID, 15, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 18, (int)&PL_IPASS_SIRIUS_MODID,  8, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 19, (int)&PL_IPASS_SIRIUS_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 20, (int)&PL_IPASS_SIRIUS_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 21, (int)&PL_IPASS_SIRIUS_MODID, 11, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 22, (int)&PL_IPASS_SIRIUS_MODID, 28, 0, 0}, /* SFI_SCI */
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 23, (int)&PL_IPASS_SIRIUS_MODID, 24, 0, 0}, /* SFI_SCI */

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, 1, 0, 0, 0},

        {REDUND_REGISTER, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        { -1, 0, 0, 0, 0, 0, 0 },
};

/*
 * Hybrid mode will be protocol4
 * Links could be dual SFI on the same plane, or SFI, we have to invent new port ability for this
 * and modify all the plane/link masks
 */
dev_cmd_rec polaris_ipass_hybrid_cmd_tbl[] = {

        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, bcmModuleProtocol4, 0, 0, 0 },

        /* set port ability, only 11 links are used */
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 16, 17, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 20, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 25, 27, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_IPASS_UNIT, 29, 31, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },

        { PORT_CONTROL, PL_IPASS_UNIT, 28, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI0 lc0 QE0 */

        /* enable ports */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, 28,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, 28,  0, 0, 0 },

#if 000
        { PORT_ENABLE_HYPERCORE,       PL_IPASS_UNIT, 24,  0, 0, 0 },
        { PORT_ENABLE_SI,              PL_IPASS_UNIT, 24,  0, 0, 0 },
#endif

        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 16, 17, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 20, 23, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_HYPERCORE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 16, 17, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 20, 23, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 25, 27, 0, 0 },
        { PORT_ENABLE_RANGE, PL_IPASS_UNIT, 29, 31, 0, 0 },

        /* enable links on bme, only enable first 11 links */
        { XBAR_ENABLE_SET,   PL_IPASS_UNIT, 0x1ff, 0, 0, 0, 0 },

	/************************************* lxbar ********** node ********* sfi *****/
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  0, (int)&PL_IPASS_SIRIUS_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  1, (int)&PL_IPASS_SIRIUS_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  2, (int)&PL_IPASS_SIRIUS_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  3, (int)&PL_IPASS_SIRIUS_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  4, (int)&PL_IPASS_SIRIUS_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  5, (int)&PL_IPASS_SIRIUS_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  6, (int)&PL_IPASS_SIRIUS_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  7, (int)&PL_IPASS_SIRIUS_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  8, (int)&PL_IPASS_SIRIUS_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT,  9, (int)&PL_IPASS_SIRIUS_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 10, (int)&PL_IPASS_SIRIUS_MODID, 16, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_IPASS_UNIT, 11, (int)&PL_IPASS_SIRIUS_MODID, 17, 0, 0},

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        {LGL_MODULE_ENABLE_SET, PL_IPASS_UNIT, (int)&PL_IPASS_SIRIUS_MODID, 1, 0, 0, 0},

        {REDUND_REGISTER, PL_IPASS_UNIT, 0, 0, 0, 0, 0},

        { -1, 0, 0, 0, 0, 0, 0 },
};

#endif

#ifdef BCM_BME3200_SUPPORT

static int sbx_mclc_xcfg_port_map[2][40] = {
{0,1,2,3,4,5,6,31,32,33,34,35,36,37,38,39,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30, 7, 8, 9,10,11,12,13,14,15},
{0,1,2,3,4,5,6, 7, 8, 9,10,11,12,13,14,15,31,32,33,34,35,36,37,38,39,25,26,27,28,29,30,16,17,18,19,20,21,22,23,24},
};
#endif /* BCM_BME3200_SUPPORT */

cmd_result_t
cmd_sbx_mclcinit_table_config(int unit, args_t *a)
{
    int i,  rv;
    int link_enable=0x3ffff;
    int qe = -1, lcm0 = -1, lcm1 = -1, fe = -1;
    int node_id = -1;
#ifdef BCM_BM9600_SUPPORT
    int qe1 = -1, fe1 = -1;
    int node_id1 = -1;
#endif

    int bTmeMode = soc_property_get(unit, spn_QE_TME_MODE,0);
    link_enable  = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);

    if (soc_ndev == 4) {
        /* find qe, lcm0, lcm1*/
        for (i=0; i < soc_ndev; i++) {
            if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i)) && SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
                if (qe==-1)
                    qe = SOC_NDEV_IDX2DEV(i);
            }
            if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i)) && SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
                if (lcm0==-1)
                    lcm0=SOC_NDEV_IDX2DEV(i);
                else if (lcm1==-1)
                    lcm1=SOC_NDEV_IDX2DEV(i);
            }
        }
        cli_out("Found devices: qe=%d fe=%d lcm0=%d lcm1=%d\n",qe,fe,lcm0,lcm1);

        if (bTmeMode != 1){
            rv = _mc_fpga_write8(FPGA_SCI_ROUTING_OFFSET, FPGA_SCI_TO_BP);
            if (rv) {
                cli_out("FPGA master mode write failed\n");
                return rv;
            }
        }else{
            node_id = 0;
        }

        if (ARG_CNT(a)) {
            int ret_code;
            parse_table_t pt;
            parse_table_init(unit, &pt);

            parse_table_add(&pt, "nodeid", PQ_DFL | PQ_INT,
                            0, &node_id, NULL);

            parse_table_add(&pt, "linkenable", PQ_DFL | PQ_INT,
                            &link_enable, &link_enable, NULL);

            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            if (node_id < 0) {
                uint8 v;
                int slot;

                /*
                 * read the slot id from the fpga
                 */
                rv = _mc_fpga_read8(FPGA_SLOT_ID_OFFSET, &v);
                if (rv) {
                    cli_out("FPGA master mode read failed\n");
                    return rv;
                }
                slot = v & 0x3;
                node_id = 3 - slot;
                if (node_id < 0) {
                    /* coverity[dead_error_line] */ 
                    return CMD_USAGE;
                }
                cli_out("LC is in slot number %d NodeID %d assigned\n", slot, node_id);
            }
        }

        /* ensure SOC has been initialized */
        if( (qe == -1) || !SOC_SBX_INIT(qe) ){
            cli_out("QE unit %d, not initialized - call 'init soc' first!\n",
                    qe);
            return CMD_FAIL;
        }

        if (bTmeMode == 1){
            cli_out("---- Init standalone card in tme mode  unit=%d  node=%d links=%x\n",unit,node_id,link_enable);
            return cmd_sbx_card_config(unit, node_id, -1, link_enable, &mc_lc_tmemode_cmd_tbl[0]);
        }
        else {
            if( lcm0<0 || !SOC_SBX_INIT(lcm0) ) {
                cli_out("LCM0 unit %d, not initialized - call 'init soc' first!\n",
                        lcm0);
            }
            if( lcm1<0 || !SOC_SBX_INIT(lcm1) ) {
                cli_out("LCM1 unit %d, not initialized - call 'init soc' first!\n",
                        lcm1);
            }
            cli_out("---- Init standalone card in fic mode  unit=%d  node=%d links=%x\n",unit,node_id,link_enable);
            return cmd_sbx_card_config(unit, node_id, -1, link_enable, &mc_lc_chassis_cmd_tbl[0]);
        }

    }
#ifdef BCM_BM9600_SUPPORT
    else if (soc_ndev == 3) {
        /* find qe0, fe0, lcm */
        for (i=0; i < soc_ndev; i++) {
            if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i)) && SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
                if (qe==-1) {
                    qe = SOC_NDEV_IDX2DEV(i);
                }
            }
            if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i)) && SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
                if (lcm0==-1) {
                    lcm0=SOC_NDEV_IDX2DEV(i);
                }
            }
        }
        cli_out("Found devices: qe0=%d fe0=%d lcm0=%d\n",qe,fe,lcm0);
        
        if (bTmeMode != 1){
            
            rv = _mc_fpga_write8(17, 0x35);
            if (rv) {
                cli_out("FPGA master mode write failed\n");
                return rv;
            }
        }else{
            rv = _mc_fpga_write8(17, 0x3A);
            if (rv) {
                cli_out("FPGA master mode write failed\n");
                return rv;
            }
            node_id = 0;
        }

        if (ARG_CNT(a)) {
            int ret_code;
            parse_table_t pt;
            parse_table_init(unit, &pt);

            parse_table_add(&pt, "nodeid", PQ_DFL | PQ_INT,
                            0, &node_id, NULL);

            parse_table_add(&pt, "linkenable", PQ_DFL | PQ_INT,
                            &link_enable, &link_enable, NULL);

            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            if (node_id < 0) {
                int slot;
#ifdef NOTDEF_REAL_CHASSIS
                uint8 v;
		int addr = FPGA_LC_PL_SLOT_ID_OFFSET;
                /*
                 * read the slot id from the fpga
                 */
                rv = _mc_fpga_read8(addr, &v);
                if (rv) {
                    cli_out("FPGA master mode read failed\n");
                    return rv;
                }
                slot = (v & 3) + 2;

                /* allow override */
                slot = soc_property_get(unit, spn_DIAG_SLOT, slot);

                if ( (slot >=0) && (slot <= 5)) {
                    node_id = sbx_pllc_slot_node_map[0][slot];
                    node_id1 = sbx_pllc_slot_node_map[1][slot];
                } else {
                    node_id = node_id1 = -1;
                }
#else
		/*
		 * hardcode for single linecard for now
		 */
		slot = 0;
		node_id = FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC;
		node_id1= -1;
#endif
                if (node_id < 0) {
                    return CMD_USAGE;
                }
                cli_out("LC is in slot number %d NodeID %d assigned to qe0\n", slot, node_id);
            }
        }

        /* ensure SOC has been initialized */
        if( (qe==-1) || !SOC_SBX_INIT(qe) ){
            cli_out("QE unit %d, not initialized - call 'init soc' first!\n",
                    qe);
        }

        if (bTmeMode == 1){
            cli_out("---- Init linecard in tme mode  unit=%d  node=%d links=%x\n",unit,node_id,link_enable);
            return cmd_sbx_card_config(unit, node_id, -1, link_enable, fe2kxt_lc_tmemode_cmd_tbl);
        }
        else {
            if( (lcm0 == -1) || !SOC_SBX_INIT(lcm0) ) {
                cli_out("LCM0 unit %d, not initialized - call 'init soc' first!\n",
                        lcm0);
            }
            cli_out("---- Init linecard in fic mode  unit=%d  node=%d / %d links=%x\n",
                    unit,node_id,node_id1,link_enable);

            card_type = SBX_CARD_TYPE_FE2KXT_CHASSIS_LINE_CARD;

            rv = cmd_sbx_card_config(unit, node_id, node_id1, link_enable, fe2kxt_lc_chassis_cmd_tbl);
	    if (rv) {
	      return rv;
	    }
        }
    }
    else if (soc_ndev == 5) {
        /* find qe0, qe1, fe0, fe1, lcm*/
        for (i=0; i < soc_ndev; i++) {
            if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i)) && SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
                if (qe==-1)
                    qe = SOC_NDEV_IDX2DEV(i);
                else if(qe1==-1)
                    qe1 = SOC_NDEV_IDX2DEV(i);
            }
            if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i)) && SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
                if (lcm0==-1)
                    lcm0=SOC_NDEV_IDX2DEV(i);
            }
        }
        cli_out("Found devices: qe0=%d qe1=%d fe0=%d fe1=%d lcm0=%d\n",qe,qe1,fe,fe1,lcm0);

        if (bTmeMode != 1){
            
            rv = _mc_fpga_write8(16, 0x50);
            rv = _mc_fpga_write8(17, 0x05);
            if (rv) {
                cli_out("FPGA master mode write failed\n");
                return rv;
            }
        }else{
            node_id = 0;
        }

        if (ARG_CNT(a)) {
            int ret_code;
            parse_table_t pt;
            parse_table_init(unit, &pt);

            parse_table_add(&pt, "nodeid", PQ_DFL | PQ_INT,
                            0, &node_id, NULL);

            parse_table_add(&pt, "linkenable", PQ_DFL | PQ_INT,
                            &link_enable, &link_enable, NULL);

            if (!parseEndOk(a, &pt, &ret_code)) {
                return ret_code;
            }

            if (node_id < 0) {
                uint8 v;
                int addr = FPGA_LC_PL_SLOT_ID_OFFSET, slot;

                /*
                 * read the slot id from the fpga
                 */
                rv = _mc_fpga_read8(addr, &v);
                if (rv) {
                    cli_out("FPGA master mode read failed\n");
                    return rv;
                }
                slot = (v & 3) + 2;

                /* allow override */
                slot = soc_property_get(unit, spn_DIAG_SLOT, slot);

                if ( (slot >=0) && (slot <= 5)) {
                    node_id = sbx_pllc_slot_node_map[0][slot];
                    node_id1 = sbx_pllc_slot_node_map[1][slot];
                } else {
                    node_id = node_id1 = -1;
                }

                if (node_id < 0) {
                    return CMD_USAGE;
                }
                cli_out("LC is in slot number %d NodeID %d assigned to qe0, NodeID %d assigned to qe1\n",
                        slot, node_id, node_id1);
            }
        }

        /* ensure SOC has been initialized */
        if( (qe==-1) || !SOC_SBX_INIT(qe) ){
            cli_out("QE unit %d, not initialized - call 'init soc' first!\n",
                    qe);
        }
        if( (qe1==-1) || !SOC_SBX_INIT(qe1) ){
            cli_out("QE unit %d, not initialized - call 'init soc' first!\n",
                    qe1);
        }

        if (bTmeMode == 1){
            cli_out("---- Init standalone card in tme mode  unit=%d  node=%d links=%x\n",unit,node_id,link_enable);
            return cmd_sbx_card_config(unit, node_id, -1, link_enable, pl_lc_tmemode_cmd_tbl);
        }
        else {
            if( (lcm0 == -1) || !SOC_SBX_INIT(lcm0) ) {
                cli_out("LCM0 unit %d, not initialized - call 'init soc' first!\n",
                        lcm0);
            }
            cli_out("---- Init standalone card in fic mode  unit=%d  node=%d / %d links=%x\n",
                    unit,node_id,node_id1,link_enable);

            card_type = SBX_CARD_TYPE_PL_CHASSIS_LINE_CARD;

            return cmd_sbx_card_config(unit, node_id, node_id1, link_enable, pl_lc_chassis_cmd_tbl);
        }
    }
#endif /* BCM_BM9600_SUPPORT */

    return CMD_OK;
}


char cmd_sbx_mcfabinit_config_usage[] =
    "Usage:\n"
    "mcfabinit [LinkEnable=0x3ffff]\n"
    " uses soc params active_switch_controller_id and bme_switch_controller_id \n"
    ;

#ifdef BCM_BM9600_SUPPORT
/*
 **************************
 *
 * Polaris fabric card init
 *
 **************************
 */
/* SFM-1 */
dev_cmd_rec pl_fabric_card_dev_cmd_tbl[] = {

#if 0
        { PORT_SPEED_RANGE,  PL_FAB_UNIT_BM96,  0, 87, 6250, 0}, /* all ports at 6.25 */
#endif /* 0 */

        /* LC3 ***************************************************************/
        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC3_QE0_MODID, bcmModuleProtocol1, 0, 0, 0 },
#ifndef _PL_FAB_NO_QE1_
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC3_QE1_MODID, bcmModuleProtocol2, 0, 0, 0 },
#endif /* _PL_FAB_NO_QE1_ */

        /* set port ability */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 35, 43, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 25, 33, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#else
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 35, 43, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 25, 33, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#endif /* _PL_FAB_NO_QE1_ */

        { PORT_CONTROL, PL_FAB_UNIT_BM96, 34, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc3 QE0 */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL, PL_FAB_UNIT_BM96, 24, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc3 QE1 */
#endif
        /* GNATS 22312 enable hypercore and SI ports for control links */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 34,  0, 0, 0 },  /* lc3.qe0 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 34,  0, 0, 0 },  /* lc3.qe0 - sci */

#ifndef _PL_FAB_NO_QE1_
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 24,  0, 0, 0 },  /* lc3.qe1 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 24,  0, 0, 0 },  /* lc3.qe1 - sci */
#endif /* _PL_FAB_NO_QE1_ */

        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 35, 43, 0, 0 },  /* lc3.qe0, lc3.qe1 - sfi */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 25, 33, 0, 0 },  /* lc3.qe0, lc3.qe1 - sf1 */

#ifndef _PL_FAB_NO_QE1_
#if 000 /* disable for now */
        /* Only for 6G+ links */
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 36, 0, 0},   /* SFIs to backplane */
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 37, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 38, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 39, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 40, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 41, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 42, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 43, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 26, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 25, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 27, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 31, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 30, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 29, 0, 4},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 28, 0, 0},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 35, 0, 6},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 32, 0, 2},
        {PORT_EQUALIZATION, PLLC_UNIT_BM96, 33, 0, 6},
#endif
#endif
        /* LC2 ***************************************************************/

        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC2_QE0_MODID, bcmModuleProtocol1, 0, 0, 0 },
#ifndef _PL_FAB_NO_QE1_
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC2_QE1_MODID, bcmModuleProtocol2, 0, 0, 0 },
#endif /* _PL_FAB_NO_QE1_ */

        /* set port ability */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 44, 45, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 47, 55, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 17, 23, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#else
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 44, 45, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 47, 55, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 17, 23, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#endif /* _PL_FAB_NO_QE1_ */
        { PORT_CONTROL, PL_FAB_UNIT_BM96, 46, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc2 QE0 */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL, PL_FAB_UNIT_BM96, 16, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc2 QE1 */
#endif
        /* enable ports */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 46,  0, 0, 0 },  /* lc2.qe0 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 46,  0, 0, 0 },  /* lc2.qe0 - sci */

#ifndef _PL_FAB_NO_QE1_
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 16,  0, 0, 0 },  /* lc3.qe1 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 16,  0, 0, 0 },  /* lc3.qe1 - sci */
#endif /* _PL_FAB_NO_QE1_ */

        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 44, 45, 0, 0 },  /* lc2.qe0, lc2.qe1 - sfi */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 47, 55, 0, 0 },  /* lc2.qe0, lc2.qe1 - sfi */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 17, 23, 0, 0 },  /* lc2.qe0, lc2.qe1 - sf1 */

        /* LC1 ***************************************************************/
        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC1_QE0_MODID, bcmModuleProtocol1, 0, 0, 0 },
#ifndef _PL_FAB_NO_QE1_
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC1_QE1_MODID, bcmModuleProtocol2, 0, 0, 0 },
#endif /* _PL_FAB_NO_QE1_ */

        /* set port ability */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 57, 65, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96,  7, 15, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#else
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 57, 65, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96,  7, 15, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#endif /* _PL_FAB_NO_QE1_ */
        { PORT_CONTROL, PL_FAB_UNIT_BM96, 56, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc1 QE0 */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL, PL_FAB_UNIT_BM96,  6, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc1 QE1 */
#endif
        /* enable ports */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 56,  0, 0, 0 },  /* lc1.qe0 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 56,  0, 0, 0 },  /* lc1.qe0 - sci */

#ifndef _PL_FAB_NO_QE1_
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96,  6,  0, 0, 0 },  /* lc1.qe1 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96,  6,  0, 0, 0 },  /* lc1.qe1 - sci */
#endif /* _PL_FAB_NO_QE1_ */

        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 57, 65, 0, 0 },  /* lc1.qe0, lc1.qe1 - sfi */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96,  7, 15, 0, 0 },  /* lc1.qe0, lc1.qe1 - sf1 */

        /* LC0 ***************************************************************/
        /* set module protocol */
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC0_QE0_MODID, bcmModuleProtocol1, 0, 0, 0 },
#ifndef _PL_FAB_NO_QE1_
        { SET_LGL_MODULE_PROTOCOL, PL_FAB_UNIT_BM96, (int)&PL_FAB_LC0_QE1_MODID, bcmModuleProtocol2, 0, 0, 0 },
#endif /* _PL_FAB_NO_QE1_ */

        /* set port ability */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96,  0,  3, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 69, 70, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 72, 79, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 84, 87, bcmPortControlAbility, BCM_PORT_ABILITY_DUAL_SFI, 0 },
#else
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96,  0,  3, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 69, 70, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 72, 79, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PL_FAB_UNIT_BM96, 84, 87, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
#endif /* _PL_FAB_NO_QE1_ */
        { PORT_CONTROL, PL_FAB_UNIT_BM96, 68, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc0 QE0 */
#ifndef _PL_FAB_NO_QE1_
        { PORT_CONTROL, PL_FAB_UNIT_BM96, 71, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc0 QE1 */
#endif
        /* enable ports */
        /* GNATS 22312, control links to QE prior to links to backplane */
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 68,  0, 0, 0 },  /* lc0.qe0 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 68,  0, 0, 0 },  /* lc0.qe0 - sci */

#ifndef _PL_FAB_NO_QE1_
        { PORT_ENABLE_HYPERCORE,       PL_FAB_UNIT_BM96, 71,  0, 0, 0 },  /* lc0.qe1 - sci */
        { PORT_ENABLE_SI,              PL_FAB_UNIT_BM96, 71,  0, 0, 0 },  /* lc0.qe1 - sci */
#endif /* _PL_FAB_NO_QE1_ */

        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96,  0,  3, 0, 0 },  /* lc0.qe0, lc0.qe1 - sfi */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 69, 70, 0, 0 },  /* lc0.qe0, lc0.qe1 - sf1 */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 72, 79, 0, 0 },  /* lc0.qe0, lc0.qe1 - sfi */
        { PORT_ENABLE_HYPERCORE_RANGE, PL_FAB_UNIT_BM96, 84, 87, 0, 0 },  /* lc0.qe0, lc0.qe1 - sf1 */

        /* enable links on bme  */
        { XBAR_ENABLE_SET,   PL_FAB_UNIT_BM96, 0x3ffff, 0, 0, 0, 0 },

        /* Note that for crossbar data, the source module id is not actually relevant */
       /* # LC3  */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC3_QE0_MODID, 36, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC3_QE0_MODID, 37, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC3_QE0_MODID, 38, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC3_QE0_MODID, 39, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC3_QE0_MODID, 40, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC3_QE0_MODID, 41, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC3_QE0_MODID, 42, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC3_QE0_MODID, 43, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC3_QE0_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC3_QE0_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC3_QE0_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC3_QE0_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC3_QE0_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC3_QE0_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC3_QE0_MODID, 28, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC3_QE0_MODID, 35, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC3_QE0_MODID, 32, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC3_QE0_MODID, 33, 0, 0},

        /* # LC2 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC2_QE0_MODID, 48, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC2_QE0_MODID, 49, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC2_QE0_MODID, 50, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC2_QE0_MODID, 51, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC2_QE0_MODID, 52, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC2_QE0_MODID, 53, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC2_QE0_MODID, 54, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC2_QE0_MODID, 55, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC2_QE0_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC2_QE0_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC2_QE0_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC2_QE0_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC2_QE0_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC2_QE0_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC2_QE0_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC2_QE0_MODID, 47, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC2_QE0_MODID, 44, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC2_QE0_MODID, 45, 0, 0},

        /* # LC1 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC1_QE0_MODID, 58, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC1_QE0_MODID, 59, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC1_QE0_MODID, 60, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC1_QE0_MODID, 61, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC1_QE0_MODID, 62, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC1_QE0_MODID, 63, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC1_QE0_MODID, 64, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC1_QE0_MODID, 65, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC1_QE0_MODID,  8, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC1_QE0_MODID,  7, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC1_QE0_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC1_QE0_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC1_QE0_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC1_QE0_MODID, 11, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC1_QE0_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC1_QE0_MODID, 57, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC1_QE0_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC1_QE0_MODID, 15, 0, 0},

        /* # LC0 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC0_QE0_MODID, 72, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC0_QE0_MODID, 73, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC0_QE0_MODID, 74, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC0_QE0_MODID, 75, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC0_QE0_MODID, 76, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC0_QE0_MODID, 77, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC0_QE0_MODID, 78, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC0_QE0_MODID, 70, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC0_QE0_MODID, 84, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC0_QE0_MODID,  0, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC0_QE0_MODID, 79, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC0_QE0_MODID,  1, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC0_QE0_MODID, 87, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC0_QE0_MODID, 86, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC0_QE0_MODID, 85, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC0_QE0_MODID, 69, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC0_QE0_MODID,  2, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC0_QE0_MODID,  3, 0, 0},

#ifndef _PL_FAB_NO_QE1_

        /* # LC3  QE1 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC3_QE1_MODID, 36, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC3_QE1_MODID, 37, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC3_QE1_MODID, 38, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC3_QE1_MODID, 39, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC3_QE1_MODID, 40, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC3_QE1_MODID, 41, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC3_QE1_MODID, 42, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC3_QE1_MODID, 43, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC3_QE1_MODID, 26, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC3_QE1_MODID, 25, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC3_QE1_MODID, 27, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC3_QE1_MODID, 31, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC3_QE1_MODID, 30, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC3_QE1_MODID, 29, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC3_QE1_MODID, 28, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC3_QE1_MODID, 35, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC3_QE1_MODID, 32, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC3_QE1_MODID, 33, 0, 0},

        /* # LC2 QE1 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC2_QE1_MODID, 48, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC2_QE1_MODID, 49, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC2_QE1_MODID, 50, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC2_QE1_MODID, 51, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC2_QE1_MODID, 52, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC2_QE1_MODID, 53, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC2_QE1_MODID, 54, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC2_QE1_MODID, 55, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC2_QE1_MODID, 18, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC2_QE1_MODID, 17, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC2_QE1_MODID, 19, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC2_QE1_MODID, 23, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC2_QE1_MODID, 22, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC2_QE1_MODID, 21, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC2_QE1_MODID, 20, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC2_QE1_MODID, 47, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC2_QE1_MODID, 44, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC2_QE1_MODID, 45, 0, 0},

        /* # LC1 QE1 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC1_QE1_MODID, 58, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC1_QE1_MODID, 59, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC1_QE1_MODID, 60, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC1_QE1_MODID, 61, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC1_QE1_MODID, 62, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC1_QE1_MODID, 63, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC1_QE1_MODID, 64, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC1_QE1_MODID, 65, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC1_QE1_MODID,  8, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC1_QE1_MODID,  7, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC1_QE1_MODID,  9, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC1_QE1_MODID, 13, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC1_QE1_MODID, 12, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC1_QE1_MODID, 11, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC1_QE1_MODID, 10, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC1_QE1_MODID, 57, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC1_QE1_MODID, 14, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC1_QE1_MODID, 15, 0, 0},

        /* # LC0 QE1 */
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  0, (int)&PL_FAB_LC0_QE1_MODID, 72, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  1, (int)&PL_FAB_LC0_QE1_MODID, 73, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  2, (int)&PL_FAB_LC0_QE1_MODID, 74, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  3, (int)&PL_FAB_LC0_QE1_MODID, 75, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  4, (int)&PL_FAB_LC0_QE1_MODID, 76, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  5, (int)&PL_FAB_LC0_QE1_MODID, 77, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  6, (int)&PL_FAB_LC0_QE1_MODID, 78, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  7, (int)&PL_FAB_LC0_QE1_MODID, 70, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  8, (int)&PL_FAB_LC0_QE1_MODID, 84, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96,  9, (int)&PL_FAB_LC0_QE1_MODID,  0, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 10, (int)&PL_FAB_LC0_QE1_MODID, 79, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 11, (int)&PL_FAB_LC0_QE1_MODID,  1, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 12, (int)&PL_FAB_LC0_QE1_MODID, 87, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 13, (int)&PL_FAB_LC0_QE1_MODID, 86, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 14, (int)&PL_FAB_LC0_QE1_MODID, 85, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 15, (int)&PL_FAB_LC0_QE1_MODID, 69, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 16, (int)&PL_FAB_LC0_QE1_MODID,  2, 0, 0},
        { XBAR_LGL_CONNECT_ADD, PL_FAB_UNIT_BM96, 17, (int)&PL_FAB_LC0_QE1_MODID,  3, 0, 0},
#endif

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PL_FAB_UNIT_BM96, 0, 0, 0, 0, 0},


        {REDUND_REGISTER, PL_FAB_UNIT_BM96, 0, 0, 0, 0, 0},

        { -1, 0, 0, 0, 0, 0, 0 },
        { -1, 2, 0, 0, 0, 0, 0 },
};


cmd_result_t
cmd_sbx_plfabinit_tbl_config(int unit, args_t *a)
{
    int slave = 0;
    int link_enable = 0x3ffff;
    int node_enable = -1;
    int switch_controller_id = -1;
    int i, rv;
    uint8 v;

    link_enable = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
    node_enable = soc_property_get(unit, spn_DIAG_NODES_MASK, -1);
    switch_controller_id =
        soc_property_get(unit, spn_BME_SWITCH_CONTROLLER_ID, -1);
    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "slave", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &slave, NULL);

        parse_table_add(&pt, "linkenable", PQ_DFL | PQ_INT,
                        &link_enable, &link_enable, NULL);

        parse_table_add(&pt, "nodeenable", PQ_DFL | PQ_INT,
                        &node_enable, &node_enable, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    cli_out("TODO: Setting up for Polaris System... FC in %s mode\n",
            (slave) ? "SLAVE" : "MASTER");

    if (slave) {
        
    }

    if (node_enable == -1) {
        /* Go look at the FPGA to work out what nodes are present */
        rv = _mc_fpga_read8(0x18, &v);
        if (rv) {
            cli_out("FPGA node mask read failed\n");
            return rv;
        }
        /* Note that the node enables are active low */
        node_enable = (~v) & 0xF;
    }

    if (switch_controller_id == -1) {
        /* Go look at the FPGA to work out which slot we're in */
        rv = _mc_fpga_read8(0x19, &v);
        if (rv) {
            cli_out("FPGA slot read failed\n");
            return rv;
        }
        /* slot ID is in the low bit and negated */
        v = (~v) & 1;
        rv = bcm_fabric_control_set(unit, bcmFabricArbiterId, v);
        if (rv) {
            cli_out("Set fabric control failed\n");
            return rv;
        }
    }

    card_type = SBX_CARD_TYPE_PL_CHASSIS_FABRIC_CARD;
    rv = cmd_sbx_card_config(unit, 0, -1, link_enable, &pl_fabric_card_dev_cmd_tbl[0]);
    if (rv) {
        return rv;
    }


    for (i = 0; i < 4; i++) {
        if (((1 << i) & node_enable) == 0) {
            continue;
        }
        rv = bcm_stk_module_enable(PL_FAB_UNIT_BM96, pl_modids[i][0], -1, TRUE);
        if (rv) {
            cli_out("Node enable %d failed, code %d\n", i, rv);
            break;
        }
#ifndef _PL_FAB_NO_QE1_
        rv = bcm_stk_module_enable(PL_FAB_UNIT_BM96, pl_modids[i][1], -1, TRUE);
        if (rv) {
            cli_out("Node enable %d failed, code %d\n", i, rv);
            break;
        }
#endif
    }
    return rv;
}
#endif /* BCM_BM9600_SUPPORT */


/*
 ***************************************************
 *
 *   Fabric card init for metrocore chassis
 *
 ***************************************************
 */
#define MC_FAB_UNIT_BME 0
#define MC_FAB_UNIT_SE  1

dev_cmd_rec mc_fabric_card_dev_cmd_tbl[] = {
    {FAB_CONTROL_SET,  MC_FAB_UNIT_BME, bcmFabricArbiterId, 555,  0, 0, 0},  /* to be setup by caller */
    {FAB_CONTROL_SET,  MC_FAB_UNIT_BME, bcmFabricActiveArbiterId, 0, 0, 0, 0},
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_SE, 31, 39, 0, 0},  /* lc0 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_SE, 21, 29, 0, 0},  /* lc1 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_SE, 11, 19, 0, 0},  /* lc2 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_SE,  1,  9, 0, 0},  /* lc3 */
    {PORT_ENABLE,       MC_FAB_UNIT_SE,  0,  0, 0, 0},  /* lc0 */
    {PORT_ENABLE,       MC_FAB_UNIT_SE, 10,  0, 0, 0},  /* lc1 */
    {PORT_ENABLE,       MC_FAB_UNIT_SE, 20,  0, 0, 0},  /* lc2 */
    {PORT_ENABLE,       MC_FAB_UNIT_SE, 30,  0, 0, 0},  /* lc3 */

    {PORT_ENABLE_RANGE, MC_FAB_UNIT_BME,  8, 15, 0, 0},  /* lc0 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_BME, 16, 23, 0, 0},  /* lc1 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_BME, 24, 31, 0, 0},  /* lc2 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_BME, 32, 39, 0, 0},  /* lc3 */
    {PORT_ENABLE_RANGE, MC_FAB_UNIT_BME,  0,  3, 0, 0},  /* SCI */
/* enable links on bme  */
    {XBAR_ENABLE_SET,   MC_FAB_UNIT_BME, 0x3ffff, 0, 0, 0, 0},

/*
 * module ids for QEs are the SCI serdes port
 */
/*      {SET_MODID,   PLLC_UNIT_FE0,  0, 0, 0, 0},   not done ??? */

    {REDUND_REGISTER, MC_FAB_UNIT_BME, 0, 0, 0, 0, 0},
    
    {-1, 0, 0, 0, 0, 0, 0},
    {-1, 2, 0, 0, 0, 0, 0},
};



cmd_result_t
cmd_sbx_mcfabinit_tbl_config(int unit, args_t *a)
{
    int rv, i;
    uint8 v;
    int link_enable = 0x3ffff;
    int node_enable = 0xf;
    int slot;
    dev_cmd_rec *pCmd;

    link_enable = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
    node_enable = soc_property_get(unit, spn_DIAG_NODES_MASK, 0xF);
    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "linkenable", PQ_DFL | PQ_INT,
                        &link_enable, &link_enable, NULL);

        parse_table_add(&pt, "nodeenable", PQ_DFL | PQ_INT,
                        &node_enable, &node_enable, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    /*
     * From SFM document, page 22; Heartbeat/generic status register
     *   CHASSIS_ID   bit 5   RO 0x0 =1, Stand Alone vs =0, "In a Chassis"
     *   SLOT_ID      bit 6  RO 0x0 =0, Slot 0 vs =1, Slot 1
     */
    _mc_fpga_read8(0x11, &v);

    /* invert the slot number to match teh SCI connections such that 
     * QE.sci0->SFM0.BME and QE.sci1->SFM1.BME
     */
    slot = !(v & 0x40);
    cli_out("Setting up for Metrocore System...  SLOT %d\n", slot);           
    
    /* find the command that configures the arbiter id, 
     * and set the correct id found from the fpga
     */
    pCmd = mc_fabric_card_dev_cmd_tbl;
    while (pCmd->cmd >= 0) {
        if (pCmd->cmd == FAB_CONTROL_SET && pCmd->arg1 == bcmFabricArbiterId) {
            pCmd->arg2 = slot;
            break;
        }
        pCmd++;
    }

    rv = cmd_sbx_card_config(unit, 0, -1, link_enable, &mc_fabric_card_dev_cmd_tbl[0]);
    if (rv) {
        return rv;
    }

    for (i = 0; i < 4; i++) {
        if (((1 << i) & node_enable) == 0) {
            continue;
        }
        rv = bcm_stk_module_enable(MC_FAB_UNIT_BME, MODID_QE_OFFSET + i, -1, TRUE);
        if (rv) {
            cli_out("Node enable %d failed, code %d\n", i, rv);
            break;
        }
    }
    return rv;
}

cmd_result_t
cmd_sbx_mcfabinit_config(int unit, args_t *a)
{
    if (SOC_IS_SBX_BME3200(unit)){

        cli_out("Setting up for Metrocore Fabric card\n");
        return cmd_sbx_mcfabinit_tbl_config(unit, a);
    }
#ifdef BCM_BM9600_SUPPORT
    else {

        cli_out("Setting up for Polaris Fabric card\n");
        return cmd_sbx_plfabinit_tbl_config(unit, a);
    }
#endif
    return CMD_FAIL;
}

char cmd_sbx_mcpbinit_config_usage[] =
"Usage:\n"
"mcpbinit - Init PizzaBox using config.bcm params \n"
"  NOTE: Sets SCI to go to SEs instead of backplane\n"
;

cmd_result_t
cmd_sbx_mcpbinit_config(int unit, args_t *a)
{
    int i,port;
    int qe=-1, lcm0=-1, lcm1=-1;
    int node_id = -1, my_modid = 0;
    bcm_pbmp_t pbmpQeSfi, pbmpLcm0Sfi, pbmpLcm1Sfi;
    bcm_error_t bcm_err;
    int logical_xbar;
    int sfi_link;
    int rv;
    int qeId = 0;
    /* bcm_port_t qesci[2]; */
    bcm_port_t qesfi[18];
    bcm_port_t se0sfi[9];
    bcm_port_t se1sfi[9];

    /* find qe, lcm0, lcm1*/
    for (i=0; i < soc_ndev; i++){
      if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i))){
        if (qe==-1)
          qe = SOC_NDEV_IDX2DEV(i);
      }
      if (SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))){
        if (lcm0==-1)
          lcm0=SOC_NDEV_IDX2DEV(i);
        else if (lcm1==-1)
          lcm1=SOC_NDEV_IDX2DEV(i);
      }
    }

    if (qe == -1 || lcm0 == -1 || lcm1 == -1) {
        cli_out("Invalid device qe %d lcm0 %d lcm1 %d\n", qe, lcm0, lcm1);
        return CMD_FAIL;
    } 
    cli_out("Found devices: qe=%d  se0=%d se1=%d\n",qe,lcm0,lcm1);

    /* Choose SCIs from user defined config "port_is_sci<port>.<unit>"
     * Default to SCIs on 0 & 1 because the BME3200 requires SCI ports
     * starting at port 0
     */
    for(port=0; port < 2; port++){
        i = soc_property_port_get(qe,  port, spn_PORT_IS_SCI, 0);
        cli_out("qe sci port qe=%d port=%d  val = %d\n",qe,port,i);
        /* qesci[port] = i; */
    }
    for(port=0; port < 18; port++){
        i = soc_property_port_get(qe,  port, spn_PORT_IS_SFI, 0);
        cli_out("qe sfi port qe=%d port=%d  val = %d\n",qe,port,i);
        qesfi[port] = i;
    }

    for(port=0; port < 9; port++){
        i = soc_property_port_get(lcm0,  port, spn_PORT_IS_SFI, 0);
        cli_out("se0 sfi port qe=%d port=%d  val = %d\n",lcm0,port,i);
        se0sfi[port] = i;
    }
    for(port=0; port < 9; port++){
        i = soc_property_port_get(lcm1,  port, spn_PORT_IS_SFI, 0);
        cli_out("se1 sfi port qe=%d port=%d  val = %d\n",lcm1,port,i);
        se1sfi[port] = i;
    }

    rv = _mc_fpga_write8(FPGA_SCI_ROUTING_OFFSET, FPGA_SCI_TO_LCM);
    if (rv) {
        cli_out("FPGA master mode write failed\n");
        return rv;
    }

    i = CMWRITE(lcm0,0x2c0, 0x03fe00);
    i = CMWRITE(lcm1,0x2c0, 0x00000001);

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(unit, &pt);

        parse_table_add(&pt, "nodeid", PQ_DFL | PQ_INT,
                        0, &node_id, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (node_id < 0) {
        return CMD_USAGE;
    }

 /* ensure SOC has been initialized */
    if( !SOC_SBX_INIT(qe) ){
        cli_out("QE unit %d, not initialized - call 'init soc' first!\n",
                qe);
        return CMD_FAIL;
    }
    if( soc_control[lcm0] == 0 ) {
        cli_out("LCM0 unit %d, not installed - call 'init soc' first!\n", lcm0);
    } else if( !SOC_SBX_INIT(lcm0) ) {
        cli_out("LCM0 unit %d, not initialized - call 'init soc' first!\n", lcm0);
    }
    if( soc_control[lcm1] == 0 ) {
        cli_out("LCM1 unit %d, not installed - call 'init soc' first!\n", lcm0);
    } else if( !SOC_SBX_INIT(lcm1) ) {
        cli_out("LCM1 unit %d, not initialized - call 'init soc' first!\n",
                lcm1);
    }

    cli_out("Setting up for Metrocore System:  qe=%d lcm0=%d lcm1=%d...\n",
            qe, lcm0, lcm1);

    /* define the module ID of this line card within the system */
    SOC_SBX_MODID_FROM_NODE(node_id, my_modid);
    bcm_err = bcm_stk_modid_set(qe, my_modid);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("bcm_stk modid set failed err=%d %s\n", bcm_err,
                bcm_errmsg(bcm_err));
        return CMD_FAIL;
    }

    /* Enable physical links on each device */
    BCM_PBMP_CLEAR(pbmpQeSfi);
    for (i = 0; i < sizeof(qesfi)/sizeof(int); i++) {
        BCM_PBMP_PORT_ADD(pbmpQeSfi, qesfi[i]+QE2000_SFI_PORT_BASE);
        cli_out("PORT_ADD QeSfi  port=%d\n", qesfi[i]);
    }

    BCM_PBMP_CLEAR(pbmpLcm0Sfi);
    for (i = 0; i < sizeof(se0sfi)/sizeof(int); i++) {
        BCM_PBMP_PORT_ADD(pbmpLcm0Sfi, se0sfi[i]);
        cli_out("Ingress PORT_ADD Lcm0Sfi  port=%d\n", se0sfi[i]);
    }

    BCM_PBMP_CLEAR(pbmpLcm1Sfi);
    for (i = 0; i < sizeof(se1sfi)/sizeof(int); i++) {
        BCM_PBMP_PORT_ADD(pbmpLcm1Sfi, se1sfi[i]);
        cli_out("Ingress PORT_ADD Lcm1Sfi  port=%d\n", se1sfi[i]);
    }

    /* Enable SFI links */
    BCM_PBMP_ITER(pbmpLcm0Sfi, i) {
        bcm_err = bcm_port_enable_set(lcm0, i, 1);
        cli_out("port_enable lcm0=%d  i=%d\n",lcm0,i);
        if (BCM_FAILURE(bcm_err)) {
            cli_out("LCM0 bcm_port_enable_set failed on unit=%d port=%d err=%d\n",
                    lcm0, i, bcm_err);
            return CMD_FAIL;
        }
    }
    BCM_PBMP_ITER(pbmpLcm1Sfi, i) {
        bcm_err = bcm_port_enable_set(lcm1, i, 1);
        cli_out("port_enable lcm1=%d  i=%d\n",lcm1,i);
        if (BCM_FAILURE(bcm_err)) {
            cli_out("LCM1 bcm_port_enable_set failed on unit=%d port=%d err=%d\n",
                    lcm1, i, bcm_err);
            return CMD_FAIL;
        }
    }
    /* enable SE SCI ports */
    bcm_err = bcm_port_enable_set(lcm0, 0, 1);
    cli_out("port_enable lcm0=%d  SCI port 0\n",lcm0);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("LCM0 bcm_port_enable_set failed on unit=%d port=0 err=%d\n",
                lcm0,  bcm_err);
        return CMD_FAIL;
    }
    bcm_err = bcm_port_enable_set(lcm1, 0, 1);
    cli_out("port_enable lcm1=%d  SCI port 0\n",lcm0);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("LCM1 bcm_port_enable_set failed on unit=%d port=0 err=%d\n",
                lcm1,  bcm_err);
        return CMD_FAIL;
    }

    /*  Enable SCI links */
    bcm_err = bcm_port_enable_set(qe, 68, 1);
    cli_out("port_enable qe=%d  port=18\n",qe);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("bcm_port_enable_set failed for QE SCI, err=%d\n", bcm_err);
        return CMD_FAIL;
    }

    bcm_err = bcm_port_enable_set(qe, 69, 1);
    cli_out("port_enable qe=%d  port=19\n",qe);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("bcm_port_enable_set failed for QE SCI, err=%d\n", bcm_err);
        return CMD_FAIL;
    }
    /* Enable SFI ports */
    BCM_PBMP_ITER(pbmpQeSfi, i) {
        bcm_err = bcm_port_enable_set(qe, i, 1);
        cli_out("port_enable qe=%d  i=%d\n",qe,i);
        if (BCM_FAILURE(bcm_err)) {
            cli_out("LCM1 bcm_port_enable_set failed on unit=%d port=%d err=%d\n",
                    qe, i, bcm_err);
            return CMD_FAIL;
        }
    }

    /* For each Logical Link, configure each QE's xcfg
     * XCFG defines the path from one QE to all other QEs by assigning
     * the logical xbar to the physical xbar port of the dest QE
     */
    BCM_PBMP_ITER(pbmpQeSfi, sfi_link) {
        logical_xbar = sfi_link - QE2000_SFI_PORT_BASE;
        if (logical_xbar >= 9) {
             cli_out("Error: Invalid logical_xbar %d \n", logical_xbar);
             continue;
        } 
        cli_out("QE node=%d logicalXbar=%d  Reach Qe%d on PT port %d\n",
                node_id, logical_xbar,
                qeId,
                se0sfi[logical_xbar]);
        bcm_err = bcm_fabric_crossbar_connection_set(qe,
                                                     logical_xbar,
                                                     my_modid,
                                                     0, /* reserved for future system */
                                                     qeId + BCM_MODULE_FABRIC_BASE,
                                                     se0sfi[logical_xbar]);

        if (BCM_FAILURE(bcm_err)) {
            cli_out("xbar config failed QE%d:\n logcalXbar=%d RemoteMod=%d xbport=%d err=%d:%s\n",
                    node_id, logical_xbar, qeId,
                    se0sfi[logical_xbar],
                    bcm_err, bcm_errmsg(bcm_err));
            return CMD_FAIL;
        }
    }

    /* Add a BME to the system using the redundancy api */
    bcm_err = bcm_fabric_control_set(qe, bcmFabricActiveArbiterId, 0);
    if (BCM_FAILURE(bcm_err)) {
        cli_out("bcm_fabric_control_set failed err=%d\n", bcm_err);
        return CMD_FAIL;
    }
    return CMD_OK;
}


#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) 
/*
 * Print a one line summary for matching memories
 * If substr_match is NULL, match all memories.
 * If substr_match is non-NULL, match any memories whose name
 * or user-friendly name contains that substring.
 */

static void
mem_list_summary(int unit, char *substr_match)
{
    soc_mem_t           mem;
    int                 i, copies, dlen;
    int                 found = 0;
    char                *dstr;

    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (!soc_mem_is_valid(unit, mem)) {
            continue;
        }

        if (substr_match != NULL &&
            strcaseindex(SOC_MEM_NAME(unit, mem), substr_match) == NULL &&
            strcaseindex(SOC_MEM_UFNAME(unit, mem), substr_match) == NULL) {
            continue;
        }

        copies = 0;
        SOC_MEM_BLOCK_ITER(unit, mem, i) {
            copies += 1;
        }

        dlen = strlen(SOC_MEM_DESC(unit, mem));
        if (dlen > 38) {
            dlen = 34;
            dstr = "...";
        } else {
            dstr = "";
        }
        if (!found) {
            cli_out(" %-6s  %-22s%5s/%-4s %s\n",
                    "Flags", "Name", "Entry",
                    "Copy", "Description");
            found = 1;
        }

        cli_out(" %c%c%c%c%c%c  %-22s%5d",
                soc_mem_is_readonly(unit, mem) ? 'r' : '-',
                soc_mem_is_debug(unit, mem) ? 'd' : '-',
                soc_mem_is_sorted(unit, mem) ? 's' :
                soc_mem_is_hashed(unit, mem) ? 'h' :
                soc_mem_is_cam(unit, mem) ? 'A' : '-',
                soc_mem_is_cbp(unit, mem) ? 'c' : '-',
                (soc_mem_is_bistepic(unit, mem) ||
                soc_mem_is_bistffp(unit, mem) ||
                soc_mem_is_bistcbp(unit, mem)) ? 'b' : '-',
                soc_mem_is_cachable(unit, mem) ? 'C' : '-',
                SOC_MEM_UFNAME(unit, mem),
                soc_mem_index_count(unit, mem));
        if (copies == 1) {
            cli_out("%5s %*.*s%s\n",
                    "",
                    dlen, dlen, SOC_MEM_DESC(unit, mem), dstr);
        } else {
            cli_out("/%-4d %*.*s%s\n",
                    copies,
                    dlen, dlen, SOC_MEM_DESC(unit, mem), dstr);
        }
    }

    if (found) {
        cli_out("Flags: (r)eadonly, (d)ebug, (s)orted, (h)ashed\n"
                "       C(A)M, (c)bp, (b)ist-able, (C)achable\n");
    } else if (substr_match != NULL) {
        cli_out("No memory found with the substring '%s' in its name.\n",
                substr_match);
    }
}

/*
 * List the tables, or fields of a table entry
 */

cmd_result_t
cmd_sbx_cmic_mem_list(int unit, args_t *a)
{
    soc_mem_info_t      *m;
    soc_field_info_t    *fld;
    char                *tab, *s;
    soc_mem_t           mem;
    uint32              entry[SOC_MAX_MEM_WORDS];
    uint32              mask[SOC_MAX_MEM_WORDS];
    int                 have_entry, i, dw, copyno;
    int                 copies, disabled, dmaable;
    char                *dmastr;
    uint32              flags;
    int                 minidx, maxidx;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    tab = ARG_GET(a);

    if (!tab) {
        mem_list_summary(unit, NULL);
        return CMD_OK;
    }

    if (parse_memory_name(unit, &mem, tab, &copyno, 0) < 0) {
        if ((s = strchr(tab, '.')) != NULL) {
            *s = 0;
        }
        mem_list_summary(unit, tab);
        return CMD_OK;
    }

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        cli_out("ERROR: Memory \"%s\" not valid for this unit\n", tab);
        return CMD_FAIL;
    }

    if (copyno < 0) {
        copyno = SOC_MEM_BLOCK_ANY(unit, mem);
    } else if (!SOC_MEM_BLOCK_VALID(unit, mem, copyno)) {
        cli_out("ERROR: Invalid copy number %d for memory %s\n", copyno, tab);
        return CMD_FAIL;
    }

    m = &SOC_MEM_INFO(unit, mem);
    flags = m->flags;

    dw = BYTES2WORDS(m->bytes);

    if ((s = ARG_GET(a)) == 0) {
        have_entry = 0;
    } else {
        for (i = 0; i < dw; i++) {
            if (s == 0) {
                cli_out("Not enough data specified (%d words needed)\n", dw);
                return CMD_FAIL;
            }
            entry[i] = parse_integer(s);
            s = ARG_GET(a);
        }
        if (s) {
            cli_out("Extra data specified (ignored)\n");
        }
        have_entry = 1;
    }

    cli_out("Memory: %s.%s",
            SOC_MEM_UFNAME(unit, mem),
            SOC_BLOCK_NAME(unit, copyno));
    s = SOC_MEM_UFALIAS(unit, mem);
    if (s && *s && strcmp(SOC_MEM_UFNAME(unit, mem), s) != 0) {
        cli_out(" alias %s", s);
    }
    cli_out(" address 0x%08x\n", soc_mem_addr(unit, mem, 0, copyno, 0));

    cli_out("Flags:");
    if (flags & SOC_MEM_FLAG_READONLY) {
        cli_out(" read-only");
    }
    if (flags & SOC_MEM_FLAG_VALID) {
        cli_out(" valid");
    }
    if (flags & SOC_MEM_FLAG_DEBUG) {
        cli_out(" debug");
    }
    if (flags & SOC_MEM_FLAG_SORTED) {
        cli_out(" sorted");
    }
    if (flags & SOC_MEM_FLAG_CBP) {
        cli_out(" cbp");
    }
    if (flags & SOC_MEM_FLAG_CACHABLE) {
        cli_out(" cachable");
    }
    if (flags & SOC_MEM_FLAG_BISTCBP) {
        cli_out(" bist-cbp");
    }
    if (flags & SOC_MEM_FLAG_BISTEPIC) {
        cli_out(" bist-epic");
    }
    if (flags & SOC_MEM_FLAG_BISTFFP) {
        cli_out(" bist-ffp");
    }
    if (flags & SOC_MEM_FLAG_UNIFIED) {
        cli_out(" unified");
    }
    if (flags & SOC_MEM_FLAG_HASHED) {
        cli_out(" hashed");
    }
    if (flags & SOC_MEM_FLAG_WORDADR) {
        cli_out(" word-addressed");
    }
    if (flags & SOC_MEM_FLAG_BE) {
        cli_out(" big-endian");
    }
    cli_out("\n");

    cli_out("Blocks: ");
    copies = disabled = dmaable = 0;
    SOC_MEM_BLOCK_ITER(unit, mem, i) {
        if (SOC_INFO(unit).block_valid[i]) {
            dmastr = "";
            if (soc_mem_dmaable(unit, mem, i)) {
                dmastr = "/dma";
                dmaable += 1;
            }
            cli_out(" %s%s", SOC_BLOCK_NAME(unit, i), dmastr);
        } else {
            cli_out(" [%s]", SOC_BLOCK_NAME(unit, i));
            disabled += 1;
        }
        copies += 1;
    }
    cli_out(" (%d cop%s", copies, copies == 1 ? "y" : "ies");
    if (disabled) {
        cli_out(", %d disabled", disabled);
    }
    if (dmaable) {
        cli_out(", %d dmaable", dmaable);
    }
    cli_out(")\n");

    minidx = soc_mem_index_min(unit, mem);
    maxidx = soc_mem_index_max(unit, mem);
    cli_out("Entries: %d with indices %d-%d (0x%x-0x%x)",
            maxidx - minidx + 1,
            minidx,
            maxidx,
            minidx,
            maxidx);
    cli_out(", each %d bytes %d words\n", m->bytes, dw);

    cli_out("Entry mask:");
    soc_mem_datamask_get(unit, mem, mask);
    for (i = 0; i < dw; i++) {
        if (mask[i] == 0xffffffff) {
            cli_out(" -1");
        } else if (mask[i] == 0) {
            cli_out(" 0");
        } else {
            cli_out(" 0x%08x", mask[i]);
        }
    }
    cli_out("\n");

    s = SOC_MEM_DESC(unit, mem);
    if (s && *s) {
        cli_out("Description: %s\n", s);
    }

    for (fld = &m->fields[m->nFields - 1]; fld >= &m->fields[0]; fld--) {
        cli_out("  %s<%d", SOC_FIELD_NAME(unit, fld->field), fld->bp + fld->len - 1);
        if (fld->len > 1) {
            cli_out(":%d", fld->bp);
        }
        if (have_entry) {
            uint32 fval[SOC_MAX_MEM_FIELD_WORDS];
            char tmp[132];

            memset(fval, 0, sizeof (fval));
            soc_mem_field_get(unit, mem, entry, fld->field, fval);
            format_long_integer(tmp, fval, SOC_MAX_MEM_FIELD_WORDS);
            cli_out("> = %s\n", tmp);
        } else {
            cli_out(">\n");
        }
    }

    return CMD_OK;
}

char cmd_sbx_mem_list_usage[] =
  "Usage:\n"
  "Parameters: [<TABLE> [<DATA> ...]]\n\t"
  "If no parameters are given, displays a reference list of all\n\t"
  "memories and their attributes.\n\t"
  "If TABLE is given, displays the entry fields for that table.\n\t"
  "If DATA is given, decodes the data into entry fields.\n";

cmd_result_t
cmd_sbx_mem_list(int unit, args_t *args)
{
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)){
        return cmd_sbx_cmic_mem_list(unit, args);
    } else {
        cli_out("Command not supported on this device.\n");
        return CMD_FAIL;
    }
}
#endif /* BCM_SIRIUS_SUPPORT */


char cmd_sbx_mem_set_usage[] =
"Usage:\n"
"memset - set a field in indirect memory\n"
"    memset  <memname.field> <addr> <value> {<instance>}\n"
;

cmd_result_t
cmd_sbx_mem_set(int unit, args_t *args)
{
    int  status, addr __attribute__((unused)),val __attribute__((unused)), instance __attribute__((unused));
    char *pArg, *memname;

    memname = ARG_GET(args);
    if(memname == NULL)
        return CMD_FAIL;

    pArg = ARG_GET(args);
    if(pArg == NULL) {
        cli_out("Missing addr parameters\n");
        return CMD_FAIL;
    }
    addr = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);
    if(pArg == NULL) {
        cli_out("Missing value parameters\n");
        return CMD_FAIL;
    }
    val = sal_ctoi(pArg, 0);

    instance = -1;
    pArg = ARG_GET(args);
    if(pArg != NULL){
        instance = sal_ctoi(pArg, 0);
    }

    status = CMD_FAIL;
#ifdef BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)){
        status = sbQe2000MemSetField( unit, memname, addr, val);
    }
#endif
#ifdef BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)){
        status = sbBm9600MemSetField( unit, memname, addr, val, instance);
    }
#endif
#ifdef BCM_BME3200_SUPPORT
    if (SOC_IS_SBX_BME3200(unit)){
        status = sbBm3200MemSetField( unit, memname, addr, val, instance);
    }
#endif
    return status;
}
char cmd_sbx_mem_show_usage[] =
 "Usage:\n"
 "memshow - Read and display a range of entries from the designated memory\n"
 "   memshow   <memname> <rangestart> <rangeend> {<instance>}\n"
 ;

cmd_result_t
cmd_sbx_mem_show(int unit, args_t *args)
{
    int  status, rangeMin __attribute__((unused)), rangeMax __attribute__((unused)), instance __attribute__((unused));
    char *pArg, *memname;

    memname = ARG_GET(args);
    if(memname == NULL) {
      cli_out("%s", cmd_sbx_mem_show_usage);
      cli_out("   Available memories are:\n");
#ifdef BCM_QE2000_SUPPORT
      if (SOC_IS_SBX_QE2000(unit)) {
          sbQe2000ShowMemNames();
      }
#endif
#ifdef BCM_BM9600_SUPPORT
      if (SOC_IS_SBX_BM9600(unit)) {
          sbBm9600ShowMemNames();
      }
#endif
#ifdef BCM_BME3200_SUPPORT
      if (SOC_IS_SBX_BME3200(unit)) {
          sbBm3200ShowMemNames();
      }
#endif
      return CMD_FAIL;
    }

    pArg = ARG_GET(args);
    if(pArg == NULL) {
        cli_out("Missing range parameters\n");
        return CMD_FAIL;
    }
    rangeMin = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);
    if(pArg == NULL){
        cli_out("Missing range end parameter\n");
        return CMD_FAIL;
    }
    rangeMax = sal_ctoi(pArg, 0);

    instance = -1;
    pArg = ARG_GET(args);
    if(pArg != NULL){
        instance = sal_ctoi(pArg, 0);
    }


    status = CMD_FAIL;
#ifdef BCM_QE2000_SUPPORT
    if (SOC_IS_SBX_QE2000(unit)){
        status = sbQe2000MemShow( unit, memname, rangeMin, rangeMax);
        return status;
    }
#endif
#ifdef BCM_BM9600_SUPPORT
    if (SOC_IS_SBX_BM9600(unit)){
        status = sbBm9600MemShow( unit, memname, rangeMin, rangeMax, instance);
        return status;
    }
#endif
#ifdef BCM_BME3200_SUPPORT
    if (SOC_IS_SBX_BME3200(unit)){
        status = sbBm3200MemShow( unit, memname, rangeMin, rangeMax, instance);
        return status;
    }
#endif
    {
        cli_out("Invalid command for unit %d device type\n",unit);
        status = CMD_FAIL;
    }
    return status;
}



#if (defined(BCM_BM9600_SUPPORT) || defined(BCM_SIRIUS_SUPPORT))

char cmd_sbx_mdio_serdes_read_usage[] =
"Usage:\n"
"mdiosiread - Read and display a range of MDIO accessible si registers\n"
"   mdiosiread   <phyaddr> <lane> <rangestart> <rangeend>\n"
;

cmd_result_t
cmd_sbx_mdio_serdes_read(int unit, args_t *args)
{
    int status = CMD_FAIL;
    char *pArg;
    uint32 uPhyAddr;
    uint32 uRangeStartAddr;
    uint32 uRangeEndAddr;
    uint32 uDataRead = 0;
    uint32 uRegAddr;
    uint32 uLane;

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing phy address\n");
        return CMD_FAIL;
    }
    uPhyAddr = sal_ctoi(pArg, 0);


    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing lane\n");
        return CMD_FAIL;
    }
    uLane = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing address range start\n");
        return CMD_FAIL;
    }
    uRangeStartAddr = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing address range end\n");
        return CMD_FAIL;
    }
    uRangeEndAddr = sal_ctoi(pArg, 0);

    cli_out("phyaddr(%d) lane(%d) rangestart(%d) rangeend(%d)\n", uPhyAddr, uLane, uRangeStartAddr, uRangeEndAddr);
    cli_out("---------------------------------------\n");

    for (uRegAddr = uRangeStartAddr; uRegAddr <= uRangeEndAddr; uRegAddr++) {
        if (SOC_IS_SBX_BM9600(unit)) {
#ifdef BCM_BM9600_SUPPORT
            status = soc_bm9600_mdio_hc_read(unit, uPhyAddr, uLane, uRegAddr, &uDataRead);
#endif
        }else if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
            status = soc_sirius_mdio_hc_read(unit, uPhyAddr, uLane, uRegAddr, &uDataRead);
#endif
        }

        cli_out("regaddr(0x%08x):data(0x%08x)\n",uRegAddr, uDataRead);
    }
    return status;
}


char cmd_sbx_mdio_serdes_write_usage[] =
"Usage:\n"
"mdiosiwrite - Write to a MDIO accessible si register"
"   memshow   <phyaddr> <lane> <regaddr> <regvalue>\n"
;

cmd_result_t
cmd_sbx_mdio_serdes_write(int unit, args_t *args)
{
    int status = CMD_FAIL;
    char *pArg;
    uint32 uPhyAddr;
    uint32 uDataWritten;
    uint32 uRegAddr;
    uint32 uLane;

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing phy address\n");
        return CMD_FAIL;
    }
    uPhyAddr = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing lane\n");
        return CMD_FAIL;
    }
    uLane = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing address\n");
        return CMD_FAIL;
    }
    uRegAddr = sal_ctoi(pArg, 0);

    pArg = ARG_GET(args);

    if(pArg == NULL) {
        cli_out("Missing register value to write\n");
        return CMD_FAIL;
    }
    uDataWritten = sal_ctoi(pArg, 0);

    cli_out("phyaddr(%d) lane(%d) regaddr(%d) regvalue(%d)\n", uPhyAddr, uLane, uRegAddr, uDataWritten);
    cli_out("---------------------------------------\n");

    if (SOC_IS_SBX_BM9600(unit)) {
#ifdef BCM_BM9600_SUPPORT
        status = soc_bm9600_mdio_hc_write(unit, uPhyAddr, uLane, uRegAddr, uDataWritten);
#endif
    } else if (SOC_IS_SIRIUS(unit)) {
#ifdef BCM_SIRIUS_SUPPORT
        status = soc_sirius_mdio_hc_write(unit, uPhyAddr, uLane, uRegAddr, uDataWritten);
#endif
    }
    cli_out("regaddr(0x%08x):data(0x%08x)\n",uRegAddr, uDataWritten);

    return status;
}
#endif


static uint32 fromAsciiHexQuadByte(const char *x) {
    uint32 r = 0;
    while (0 != (*x)) {
        switch (*x) {
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
            r = (r << 4) + ((*x) & 0x0F);
            break;
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
            r = (r << 4) + (((*x) + 9) & 0x0F);
            break;
        default:
            r = r << 4;
        }
        x++;
    }
    return r;
}

static int fromAsciiMacAddress(const char *s, bcm_mac_t *m) {
    unsigned int i = 0;
    for (i = 0; i < 6; i++) {
        (*m)[i] = 0;
    }
    i = 0;
    while ((0 != *s) && (i < 6)) {
        switch (*s) {
        case 0x30:
        case 0x31:
        case 0x32:
        case 0x33:
        case 0x34:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x38:
        case 0x39:
            (*m)[i] = ((*m)[i] << 4) + ((*s) & 0x0F);
            break;
        case 0x41:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
        case 0x46:
        case 0x61:
        case 0x62:
        case 0x63:
        case 0x64:
        case 0x65:
        case 0x66:
            (*m)[i] = ((*m)[i] << 4) + (((*s) + 9) & 0x0F);
            break;
        default:
            i++;
        }
        s++;
    }
    if (5 == i) {
        return 1;
    } else {
        return 0;
    }
}

STATIC INLINE int _max(int x, int y) {
    if (x > y) {
        return x;
    } else {
        return y;
    }
}

#define _GET_NONBLANK(a,c) \
    do { \
        (c) = ARG_GET(a); \
        if (NULL == (c)) { \
            return CMD_USAGE; \
        } \
    } while (0)

#ifdef BCM_SIRIUS_SUPPORT

char cmd_sbx_multicast_usage[] =
    "Usages:\n"
    "  create Type=L2|L3|VPLS|SUBPORT|MIN|WLAN [Group=<num>]\n"
    "  destroy Group=<num>\n"
    "  l3encap Group=<num> Port=<num> Intf=<num>\n"
    "  l2encap Group=<num> Port=<num> Vlan=<num>\n"
    "  vplsencap Group=<num> Port=<num> MplsPortId=<num>\n"
    "  subportencap Group=<num> Port=<num> SubPort=<num>\n"
    "  mimencap Group=<num> Port=<num> MimPortId=<num>\n"
    "  wlanencap Group=<num> Port=<num> WlanPortId=<num>\n"
    "  add Group=<num> Port=<num> EncapId=<num>\n"
    "  delete Group=<num> [Port=<num> EncapId=<num>]\n"
    "  show Group=<num>\n"
    "  sirius <...>\n";
static const char cmd_sbx_sirius_multicast_usage[] =
    "Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "  defrag <start|stop>\n"
    "  oitt <enable|disable>\n"
    "  dump <level> [<firstGroupId> [<lastGroupId>]]\n"
    "    level is bitmap, in hex\n"
    "                           00000001 = unit information\n"
    "                           00000002 = unit GMT information\n"
    "                           00000004 = unit MDB information\n"
    "                           00000008 = unit OITT information\n"
    "                           00000010 = unit target -> GPORT information\n"
    "                           00000100 = unit active MC group data\n"
    "                           00000200 =  (include all groups -- SLOW)\n"
    "                           00000400 =  (using hardware probe -- SLOW)\n"
    "                           00000800 =  (include BCM layer group data)\n"
    "                           00001000 =  (include hardware group data)\n"
    "    if no group given, scans all\n"
    "    if only firstGrouId given, only considers that single group\n"
    "  show [<firstGroupId> [<lastGroupId>]]\n"
    "    equivalent to level '1900' of dump command\n"
    "  init\n"
#endif
;
static const char *cmd_multicast_parse_type[] = {
    "NONE",
    "L2",
    "L3",
    "VPLS",
    "SUBPORT",
    "MIM",
    "WLAN",
    NULL
};
#define _BCM_MULTICAST_TYPE_L2           1
#define _BCM_MULTICAST_TYPE_L3           2
#define _BCM_MULTICAST_TYPE_VPLS         3
#define _BCM_MULTICAST_TYPE_SUBPORT      4
#define _BCM_MULTICAST_TYPE_MIM          5
#define _BCM_MULTICAST_TYPE_WLAN         6
#define _BCM_MULTICAST_TYPE_SHIFT        24
#define _BCM_MULTICAST_TYPE_MASK         0xff
#define _BCM_MULTICAST_ID_SHIFT          0
#define _BCM_MULTICAST_ID_MASK           0xffffff
#define _BCM_MULTICAST_IS_SET(_group_)    \
        (((_group_) >> _BCM_MULTICAST_TYPE_SHIFT) != 0)
#define _BCM_MULTICAST_GROUP_SET(_group_, _type_, _id_) \
    ((_group_) = ((_type_) << _BCM_MULTICAST_TYPE_SHIFT)  | \
     (((_id_) & _BCM_MULTICAST_ID_MASK) << _BCM_MULTICAST_ID_SHIFT))
#define _BCM_MULTICAST_ID_GET(_group_) \
    (((_group_) >> _BCM_MULTICAST_ID_SHIFT) & _BCM_MULTICAST_ID_MASK)
#define _BCM_MULTICAST_TYPE_GET(_group_) \
    (((_group_) >> _BCM_MULTICAST_TYPE_SHIFT) & _BCM_MULTICAST_TYPE_MASK)

#ifndef _MAX
#define _MAX(_a,_b) (((_a)>(_b))?(_a):(_b))
#endif /* ndef _MAX */

cmd_result_t
cmd_sbx_multicast(int unit, args_t *a)
{
    char *subcmd = NULL;
    parse_table_t pt;
    int rv, i, type, port_count;
    uint32 flags;
    bcm_multicast_t group;
    bcm_gport_t port, port_array[SOC_MAX_NUM_PORTS];
    bcm_if_t intf; /* for l3_encap_get */
    bcm_vlan_t vlan; /* for l2_encap_get */
    bcm_gport_t mpls_port_id; /* for vpls_encap_get */
    bcm_gport_t subport; /* for subport_encap_get */
    bcm_gport_t mim_port_id; /* for mim_encap_get */
    bcm_gport_t wlan_port_id; /* for wlan_encap_get */
    bcm_if_t encap_id, encap_id_array[SOC_MAX_NUM_PORTS];
    bcm_if_t invalidIf = BCM_IF_INVALID;
    bcm_gport_t invalidGport = BCM_GPORT_INVALID;
    unsigned int first;
    unsigned int last;
    unsigned int level;
    int minusOne = -1;
    cmd_result_t cmdres;

    subcmd = ARG_GET(a);
    if (subcmd == NULL) {
        return CMD_USAGE;
    }

    if (!sal_strcasecmp(subcmd, "create")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Type", PQ_MULTI, 0, &type,
                        cmd_multicast_parse_type);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        switch (type) {
        case _BCM_MULTICAST_TYPE_L2:
            flags = BCM_MULTICAST_TYPE_L2;
            break;
        case _BCM_MULTICAST_TYPE_L3:
            flags = BCM_MULTICAST_TYPE_L3;
            break;
        case _BCM_MULTICAST_TYPE_VPLS:
            flags = BCM_MULTICAST_TYPE_VPLS;
            break;
        case _BCM_MULTICAST_TYPE_SUBPORT:
            flags = BCM_MULTICAST_TYPE_SUBPORT;
            break;
        case _BCM_MULTICAST_TYPE_MIM:
            flags = BCM_MULTICAST_TYPE_MIM;
            break;
        case _BCM_MULTICAST_TYPE_WLAN:
            flags = BCM_MULTICAST_TYPE_WLAN;
            break;
        default:
            return CMD_FAIL;
        }
        if (group != -1) {
            flags |= BCM_MULTICAST_WITH_ID;
        }
        rv = bcm_multicast_create(unit, flags, &group);
        if (rv < 0) {
            cli_out("%s ERROR: fail to create %s group - %s\n",
                    ARG_CMD(a), cmd_multicast_parse_type[type],
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }

        cli_out("group id 0x%x\n", group);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "destroy")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        if (parse_arg_eq(a, &pt) < 0 || group == -1) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        rv = bcm_multicast_destroy(unit, group);
        if (rv < 0) {
            cli_out("%s ERROR: fail to destroy group %d - %s\n",
                    ARG_CMD(a), group, bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "l3encap")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "Intf", PQ_INT, &minusOne, &intf, 0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (intf == -1) {
            cli_out("%s: Interface not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        rv = bcm_multicast_l3_encap_get(unit, group, port, intf, &encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: fail to get L3 encap - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("Encap ID %d\n", encap_id);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "l2encap")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "Vlan", PQ_INT, &minusOne, &vlan,
                        0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        rv = bcm_multicast_l2_encap_get(unit, group, port, vlan, &encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: fail to get L2 encap - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("Encap ID %d\n", encap_id);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "vplsencap")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "MplsPortId", PQ_INT, &invalidGport,
                        &mpls_port_id, 0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (group < 0) {
            cli_out("%s: Group not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (port == BCM_GPORT_INVALID) {
            cli_out("%s: Port not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (mpls_port_id == BCM_GPORT_INVALID) {
            cli_out("%s: MplsPortId not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        if (!BCM_GPORT_IS_SET(mpls_port_id)) {
            BCM_GPORT_MPLS_PORT_ID_SET(mpls_port_id, mpls_port_id);
        }
        rv = bcm_multicast_vpls_encap_get(unit, group, port, mpls_port_id,
                                          &encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: fail to get VPLS encap - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("Encap ID %d\n", encap_id);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "subportEncap")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "SubPort", PQ_INT, &invalidGport,
                        &subport, 0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (group < 0) {
            cli_out("%s: Group not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (port == BCM_GPORT_INVALID) {
            cli_out("%s: Port not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (subport == BCM_GPORT_INVALID) {
            cli_out("%s: SubPort not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        if (!BCM_GPORT_IS_SET(subport)) {
            BCM_GPORT_SUBPORT_PORT_SET(subport, subport);
        }
        rv = bcm_multicast_subport_encap_get(unit, group, port, subport,
                                             &encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: fail to get subport encap - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("Encap ID %d\n", encap_id);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "mimencap")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "MimPortId", PQ_INT, &invalidGport,
                        &mim_port_id, 0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (group < 0) {
            cli_out("%s: Group not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (port == BCM_GPORT_INVALID) {
            cli_out("%s: Port not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (mim_port_id == BCM_GPORT_INVALID) {
            cli_out("%s: MimPortId not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        if (!BCM_GPORT_IS_SET(mim_port_id)) {
            BCM_GPORT_MIM_PORT_ID_SET(mim_port_id, mim_port_id);
        }
        rv = bcm_multicast_mim_encap_get(unit, group, port, mim_port_id,
                                         &encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: fail to get MIM encap - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("Encap ID %d\n", encap_id);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "wlanencap")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "WlanPortId", PQ_INT, &invalidGport,
                        &wlan_port_id, 0);
        if (parse_arg_eq(a, &pt) < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (group < 0) {
            cli_out("%s: Group not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (port == BCM_GPORT_INVALID) {
            cli_out("%s: Port not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }
        if (wlan_port_id == BCM_GPORT_INVALID) {
            cli_out("%s: WlanPortId not specified\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        if (!BCM_GPORT_IS_SET(wlan_port_id)) {
            BCM_GPORT_WLAN_PORT_ID_SET(wlan_port_id, wlan_port_id);
        }
        rv = bcm_multicast_wlan_encap_get(unit, group, port, wlan_port_id,
                                          &encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: fail to get WLAN encap - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        cli_out("Encap ID %d\n", encap_id);
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "add")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "EncapId", PQ_INT, &invalidIf,
                        &encap_id, 0);

        if (parse_arg_eq(a, &pt) < 0 || group < 0 ||
            port == BCM_GPORT_INVALID) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (!BCM_GPORT_IS_SET(port)) {
            BCM_GPORT_LOCAL_SET(port, port);
        }
        rv = bcm_multicast_egress_add(unit, group, port, encap_id);
        if (rv < 0) {
            cli_out("%s ERROR: egress add failure - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "delete")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        parse_table_add(&pt, "Port", PQ_PORT, &invalidGport, &port,
                        0);
        parse_table_add(&pt, "EncapId", PQ_INT, &invalidIf,
                        &encap_id, 0);
        if (parse_arg_eq(a, &pt) < 0 || group < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if (port == BCM_GPORT_INVALID && encap_id == BCM_IF_INVALID) {
            rv = bcm_multicast_egress_delete_all(unit, group);
            if (rv < 0) {
                cli_out("%s ERROR: egress delete all failure - %s\n",
                        ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        } else {
            if (port == BCM_GPORT_INVALID || encap_id == BCM_IF_INVALID) {
                return CMD_FAIL;
            }
            if (!BCM_GPORT_IS_SET(port)) {
                BCM_GPORT_LOCAL_SET(port, port);
            }
            rv = bcm_multicast_egress_delete(unit, group, port, encap_id);
            if (rv < 0) {
                cli_out("%s ERROR: egress delete failure - %s\n",
                        ARG_CMD(a), bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
        return CMD_OK;
    }

    if (!sal_strcasecmp(subcmd, "show")) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Group", PQ_INT, &minusOne, &group, 0);
        if (parse_arg_eq(a, &pt) < 0 || group < 0) {
            cli_out("%s: Invalid argument: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        rv = bcm_multicast_egress_get(unit, group, SOC_MAX_NUM_PORTS,
                                      port_array, encap_id_array,
                                      &port_count);
        if (rv < 0) {
            cli_out("%s ERROR: egress get failure - %s\n",
                    ARG_CMD(a), bcm_errmsg(rv));
            return CMD_FAIL;
        }
        if (port_count >= SOC_MAX_NUM_PORTS) {
            cli_out("warning: displayed group may be incomplete"
                    " (buffer was filled)\n");
        }

        cli_out("Group 0x%x (%s), %d replicants\n", group,
                cmd_multicast_parse_type[_BCM_MULTICAST_TYPE_GET(group)],
                port_count);
        for (i = 0; i < port_count; i++) {
            cli_out("\tport 0x%x, encap id %d\n",
                    port_array[i], encap_id_array[i]);
        }
        return CMD_OK;
    }

    if (!sal_strncasecmp(subcmd, "sirius", _MAX(2, sal_strlen(subcmd)))) {
        if (SOC_IS_SIRIUS(unit)) {
            cmdres = CMD_USAGE;
            subcmd = ARG_GET(a);
            if (subcmd) {
                if (!sal_strncasecmp(subcmd,
                                     "dump",
                                     _MAX(2, sal_strlen(subcmd)))) {
                    subcmd = ARG_GET(a);
                    if (subcmd) {
                        level = fromAsciiHexQuadByte(subcmd);
                        subcmd = ARG_GET(a);
                        if (subcmd) {
                            first = parse_integer(subcmd);
                            subcmd = ARG_GET(a);
                            if (subcmd) {
                                last = parse_integer(subcmd);
                            } else {
                                last = first;
                            }
                        } else {
                            first = 0;
                            last = ~0;
                        }
						/* coverity[ stack_use_overflow ] */
                        rv = _bcm_sirius_multicast_dump(unit,
                                                        level,
                                                        first,
                                                        last);
                        cli_out("_bcm_sirius_multicast_dump(%d,%08X,%u,%u)"
                                " returned %d (%s)\n",
                                unit,
                                level,
                                first,
                                last,
                                rv,
                                _SHR_ERRMSG(rv));
                        if (BCM_E_NONE == rv) {
                            cmdres = CMD_OK;
                        } else {
                            cmdres = CMD_FAIL;
                        }
                    }
                } else if (!sal_strncasecmp(subcmd,
                                            "show",
                                            _MAX(2, sal_strlen(subcmd)))) {
                    level = 0x1900;
                    subcmd = ARG_GET(a);
                    if (subcmd) {
                        first = parse_integer(subcmd);
                        subcmd = ARG_GET(a);
                        if (subcmd) {
                            last = parse_integer(subcmd);
                        } else {
                            last = first;
                        }
                    } else {
                        first = 0;
                        last = ~0;
                    }
                    rv = _bcm_sirius_multicast_dump(unit,
                                                    level,
                                                    first,
                                                    last);
                    cli_out("_bcm_sirius_multicast_dump(%d,%08X,%u,%u)"
                            " returned %d (%s)\n",
                            unit,
                            level,
                            first,
                            last,
                            rv,
                            _SHR_ERRMSG(rv));
                    if (BCM_E_NONE == rv) {
                        cmdres = CMD_OK;
                    } else {
                        cmdres = CMD_FAIL;
                    }
                } else if (!sal_strncasecmp(subcmd,
                                            "defrag",
                                            _MAX(2, sal_strlen(subcmd)))) {
                    subcmd = ARG_GET(a);
                    if (subcmd) {
                        level = 2;
                        if (!sal_strncasecmp(subcmd,
                                             "start",
                                             _MAX(3, sal_strlen(subcmd)))) {
                            level = 1;
                        } else if (!sal_strncasecmp(subcmd,
                                                    "stop",
                                                    _MAX(3,
                                                         sal_strlen(subcmd)))) {
                            level = 0;
                        }
                        if (2 != level) {
                            rv = _bcm_sirius_multicast_defrag(unit, level);
                            cli_out("_bcm_sirius_multicast_defrag(%d,%s)"
                                    " returned %d (%s)\n",
                                    unit,
                                    level?"START":"STOP",
                                    rv,
                                    _SHR_ERRMSG(rv));
                            if (BCM_E_NONE == rv) {
                                cmdres = CMD_OK;
                            } else {
                                cmdres = CMD_FAIL;
                            }
                        }
                    }
                } else if (!sal_strncasecmp(subcmd,
                                            "oitt",
                                            _MAX(1, sal_strlen(subcmd)))) {
                    subcmd = ARG_GET(a);
                    if (subcmd) {
                        level = 2;
                        if (!sal_strncasecmp(subcmd,
                                             "enable",
                                             _MAX(1, sal_strlen(subcmd)))) {
                            level = 1;
                        } else if (!sal_strncasecmp(subcmd,
                                                    "disable",
                                                    _MAX(1,
                                                         sal_strlen(subcmd)))) {
                            level = 0;
                        }
                        if (2 != level) {
                            rv = _bcm_sirius_multicast_oitt_enable_set(unit,
                                                                       level);
                            cli_out("_bcm_sirius_multicast_oitt_enable_set"
                                    "(%d,%s) returned %d (%s)\n",
                                    unit,
                                    level?"TRUE":"FALSE",
                                    rv,
                                    _SHR_ERRMSG(rv));
                            if (BCM_E_NONE == rv) {
                                cmdres = CMD_OK;
                            } else {
                                cmdres = CMD_FAIL;
                            }
                        }
                    }
                } else if (!sal_strcasecmp(subcmd,"init")) {
                    rv = bcm_multicast_init(unit);
                    cli_out("bcm_multicast_init(%d) returned %d (%s)\n",
                            unit,
                            rv,
                            _SHR_ERRMSG(rv));
                    if (BCM_E_NONE == rv) {
                        cmdres = CMD_OK;
                    } else {
                        cmdres = CMD_FAIL;
                    }
                }
            } /* if (subcmd) */
            if (CMD_USAGE == cmdres) {
                
                cli_out("%s",cmd_sbx_sirius_multicast_usage);
                return CMD_FAIL;
            } else {
                return cmdres;
            }
        } else {
            cli_out("Can only use Sirius subcommands on Sirius chips\n");
            return CMD_FAIL;
        }
    }

    return CMD_USAGE;
}
#endif /* BCM_SIRIUS_SUPPORT */

char cmd_sbx_field_usage[] =
    "(caps are obligatory, lower optional; <var> = use var's value; [optional])\n"
    "  FIeld Show <prefix>\n"
    "  FIeld Group Dump <groupid>\n"
    "  FIeld Entry Dump <entryid>\n"
    "\n"
    "Numbers in field commands are expected to be in hex without leading 0x.\n"
    ;

char cmd_sbx_afl_usage[] =
    "(caps are obligatory, lower optional; <var> = use var's value; [optional])\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "  AFL Create <first> <last> <min_valid> <max_valid> <block_factor>\n"
    "  AFL Destroy\n"
    "  AFL Allocate <blocksize>\n"
    "  AFL Free <base>\n"
    "  AFL Elementstate <elem>\n"
    "  AFL State\n"
    "  AFL Reserve <first> <last>\n"
    "  AFL Blockreserve <first> <count>\n"
    "  AFL Test <testid> <iterations>\n"
    "  AFL Paranoia [<flags>]\n"
    "\n"
    "The test feature offers three modes: 0 = simple random, 1 = bias toward higher"
    "occupancy, 2 = bias toward lower occupancy.\n"
    "\n"
    "Numbers in afl commands are expected to be in hex without leading 0x.\n"
#endif
    ;

char cmd_sbx_fl_usage[] =
    "(caps are obligatory, lower optional; <var> = use var's value; [optional])\n"
    "  FL Create <first> <last> <min_valid> <max_valid> [<scale>]\n"
    "  FL Destroy\n"
    "  FL Allocate\n"
    "  FL Free <elem>\n"
    "  FL Elementstate <elem>\n"
    "  FL State\n"
    "  FL Reserve <first> <last>\n"
    "\n"
    "Numbers in fl commands are expected to be in hex without leading 0x.\n"
    ;

char cmd_sbx_mcast_usage[] =
    "(caps are obligatory, lower optional; <var> = use var's value; [optional])\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "  MCAST Init\n"
    "  MCAST Add <vid> <mc_maca> <portbmp> <uportbmp>\n"
    "  MCAST Add L2mcindex <l2mc_index> <vid> <mc_maca> <portbmp> <uportbmp>\n"
    "  MCAST Rem <vid> <mc_maca>\n"
    "  MCAST Rem L2mcindex <l2mc_index> <vid> <mc_maca>\n"
    "  MCAST Port Add <vid> <mc_maca> <portbmp> <uportbmp>\n"
    "  MCAST Port Rem <vid> <mc_maca> <portbmp>\n"
    "  MCAST Port Get <vid> <mc_maca>\n"
    "  MCAST Leave <vid> <mc_maca> <portnum>\n"
    "  MCAST Join  <vid> <mc_maca> <portnum>\n"
    "\n"
    "Numbers in mcast commands are expected to be in hex without leading 0x.\n"
    "<mc_maca> is a multicast MAC address, as six hex bytes with separators.\n"
#endif
    ;

char cmd_sbx_vlan_usage[] =
    "Usages:\n\t"
#ifndef COMPILER_STRING_CONST_LIMIT
    "  vlan create <id> [PortBitMap=<pbmp> UntagBitMap=<pbmp>]\n\t"
    "                                       - Create a VLAN\n\t"
    "  vlan destroy <id>                    - Destroy a VLAN\n\t"
    "  vlan clear                           - Destroy all VLANs\n\t"
    "  vlan add <id> [PortBitMap=<pbmp> UntagBitMap=<pbmp>\n\t"
    "                                       - Add port(s) to a VLAN\n\t"
    "  vlan remove <id> [PortBitMap=<pbmp>] - Remove ports from a VLAN\n\t"
    "  vlan MulticastFlood <id>  [Mode]     - Multicast flood setting\n\t"
    "  vlan show                            - Display all VLANs\n\t"
    "  vlan default [<id>]                  - Show or set the default VLAN\n\t"
    "  vlan translate [On|Off]\n\t"
    "  vlan translate add Port=<port> OldVLan=<vlanid> NewVLan=<vlanid>\n\t"
    "        Prio=<prio>\n\t"
#if 0

    "  vlan translate addrange Port=<port> OldVLanLow=<vlanid> OldVLanHi=<vlanid>\n\t"
    "        NewVLan=<vlanid>\n\t"
#endif
    "  vlan translate delete Port=<port> OldVLan=<vlanid>\n\t"
    "  vlan translate clear\n\t"
    "  vlan translate egress add Port=<port> OldVLan=<vlanid> NewVLan=<vlanid>\n\t"
    "        Prio=<prio>\n\t"
    "  vlan translate egress delete Port=<port> OldVLan=<vlanid>\n\t"
    "  vlan translate egress clear\n\t"
    "  vlan control <name> <value>\n\t"
    "  vlan <id> <name>=<vlaue>         - Set/Get per VLAN property\n"
#endif
    ;

static char *_bcm_vlan_mcast_flood_str[] = BCM_VLAN_MCAST_FLOOD_STR;

cmd_result_t
cmd_sbx_field(int unit, args_t *a){
#ifdef BROADCOM_DEBUG
    char                             *c;
    unsigned int                     groupid;
    unsigned int                     entryid;
    int                              result;

    _GET_NONBLANK(a,c);
    if (!sal_strncasecmp(c,"show",strlen(c))) {
        c = ARG_GET(a);
        if (!c) {
            c = "";
        }
        result = bcm_field_show(unit,c);
        cli_out("bcm_field_show(%d,\"%s\") returned %d (%s)\n",unit,c,result,_SHR_ERRMSG(result));
    } else if (!sal_strncasecmp(c,"entry",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"dump",strlen(c))) {
            _GET_NONBLANK(a,c);
            entryid = fromAsciiHexQuadByte(c);
            result = bcm_field_entry_dump(unit, entryid);
            cli_out("bcm_field_entry_dump(%d,%08X) returned %d (%s)\n",unit,entryid,result,_SHR_ERRMSG(result));
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"group", strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"dump",strlen(c))) {
            _GET_NONBLANK(a,c);
            groupid = fromAsciiHexQuadByte(c);
            result = bcm_field_group_dump(unit, groupid);
            cli_out("bcm_field_group_dump(%d,%08X) returned %d (%s)\n",unit,groupid,result,_SHR_ERRMSG(result));
        } else {
            return CMD_USAGE;
        }
    } else {
        return CMD_USAGE;
    }
#endif /* BROADCOM_DEBUG */
    return CMD_OK;
}

cmd_result_t
cmd_sbx_fl(int unit, args_t *a){
    char                             *c;
    static shr_idxres_list_handle_t  list = NULL;
    unsigned int                     first;
    unsigned int                     last;
    unsigned int                     min_valid;
    unsigned int                     max_valid;
    unsigned int                     scale;
    unsigned int                     element;
    unsigned int                     count;
    unsigned int                     free;
    int                              result;

    _GET_NONBLANK(a,c);
    if (!sal_strncasecmp(c,"create",strlen(c))) {
        if (!list) {
            _GET_NONBLANK(a,c);
            first = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            last = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            min_valid = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            max_valid = fromAsciiHexQuadByte(c);
            c = ARG_GET(a);
            if (c) {
                scale = fromAsciiHexQuadByte(c);
                result = shr_idxres_list_create_scaled(&list,first,last,min_valid,max_valid,scale,"fl test");
                cli_out("shr_idxres_list_create_scaled(*,%08X,%08X,%08X,%08X,%08X,*) returned %d (%s)\n",first,last,min_valid,max_valid,scale,result,_SHR_ERRMSG(result));
            } else {
                scale = 1;
                result = shr_idxres_list_create(&list,first,last,min_valid,max_valid,"fl test");
                cli_out("shr_idxres_list_create(*,%08X,%08X,%08X,%08X,*) returned %d (%s)\n",first,last,min_valid,max_valid,result,_SHR_ERRMSG(result));
            }
            cli_out("  list = *(%08X)\n",(unsigned int)list);
        } else {
            cli_out("a list already exists; destroy it first\n");
        }
    } else if (!sal_strncasecmp(c,"destroy",strlen(c))) {
        if (list) {
            result = shr_idxres_list_destroy(list);
            cli_out("shr_idxres_list_destroy(%08X) returned %d (%s)\n",(unsigned int)list,result,_SHR_ERRMSG(result));
            if (BCM_E_NONE == result) {
                list = NULL;
            }
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"allocate",strlen(c))) {
        if (list) {
            result = shr_idxres_list_alloc(list, &element);
            cli_out("shr_idxres_list_alloc(*,*) returned %d (%s)\n",result,_SHR_ERRMSG(result));
            cli_out("  first element = %08X\n",element);
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"free",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            element = fromAsciiHexQuadByte(c);
            result = shr_idxres_list_free(list,element);
            cli_out("shr_idxres_list_free(*,%08X) returned %d (%s)\n",element,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"elementstate",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            element = fromAsciiHexQuadByte(c);
            result = shr_idxres_list_elem_state(list,element);
            cli_out("shr_idxres_list_elem_state(*,%08X) returned %d (%s)\n",element,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"state",strlen(c))) {
        if (list) {
            result = shr_idxres_list_state_scaled(list,&first,&last,&min_valid,&max_valid,&free,&count,&scale);
            cli_out("shr_idxres_list_state_scaled(*) returned %d (%s)\n",result,_SHR_ERRMSG(result));
            cli_out("  list.first        = %08X\n",first);
            cli_out("  list.last         = %08X\n",last);
            cli_out("  list.valid_low    = %08X\n",min_valid);
            cli_out("  list.valid_high   = %08X\n",max_valid);
            cli_out("  list.free         = %08X\n",free);
            cli_out("  list.alloc        = %08X\n",count);
            cli_out("  list.scale        = %08X\n",scale);
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"reserve",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            first = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            last = fromAsciiHexQuadByte(c);
            result = shr_idxres_list_reserve(list,first,last);
            cli_out("shr_idxres_list_reserve(*,%08X,%08X) returned %d (%s)\n",first,last,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else {
        return CMD_USAGE;
    }
    return CMD_OK;
}

cmd_result_t
cmd_sbx_afl(int unit, args_t *a){
    char                             *c;
    static shr_aidxres_list_handle_t list = NULL;
    unsigned int                     first;
    unsigned int                     last;
    unsigned int                     min_valid;
    unsigned int                     max_valid;
    unsigned int                     block_factor;
    unsigned int                     element;
    unsigned int                     count;
    unsigned int                     free;
    unsigned int                     largest;
    unsigned int                     *elems;
    unsigned int                     *sizes;
    unsigned int                     iter;
    unsigned int                     iter_curr;
    unsigned int                     iter_max;
    unsigned int                     occ;
    unsigned int                     occ_min;
    unsigned int                     occ_max;
    unsigned int                     occb_min;
    unsigned int                     occb_max;
    unsigned int                     blk;
    unsigned int                     blk_min;
    unsigned int                     blk_max;
    unsigned int                     blko_min;
    unsigned int                     blko_max;
    unsigned int                     pass;
    unsigned int                     bias;
    const unsigned int               bias_level = 16;
    unsigned int                     success;
    unsigned int                     fail;
    int                              result;

    _GET_NONBLANK(a,c);
    if (!sal_strncasecmp(c,"create",strlen(c))) {
        if (!list) {
            _GET_NONBLANK(a,c);
            first = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            last = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            min_valid = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            max_valid = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            block_factor = fromAsciiHexQuadByte(c);
            result = shr_aidxres_list_create(&list,first,last,min_valid,max_valid,block_factor,"afl test");
            cli_out("shr_aidxres_list_create(*,%08X,%08X,%08X,%08X,%08X,*) returned %d (%s)\n",first,last,min_valid,max_valid,block_factor,result,_SHR_ERRMSG(result));
            cli_out("  list = *(%08X)\n",(unsigned int)list);
        } else {
            cli_out("a list already exists; destroy it first\n");
        }
    } else if (!sal_strncasecmp(c,"destroy",strlen(c))) {
        if (list) {
            result = shr_aidxres_list_destroy(list);
            cli_out("shr_aidxres_list_destroy(%08X) returned %d (%s)\n",(unsigned int)list,result,_SHR_ERRMSG(result));
            if (BCM_E_NONE == result) {
                list = NULL;
            }
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"allocate",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            count = fromAsciiHexQuadByte(c);
            if (1 == count) {
                result = shr_aidxres_list_alloc(list, &element);
                cli_out("shr_aidxres_list_alloc(*,*) returned %d (%s)\n",result,_SHR_ERRMSG(result));
            } else {
                result = shr_aidxres_list_alloc_block(list,count,&element);
                cli_out("shr_aidxres_list_alloc_block(*,%08X,*) returned %d (%s)\n",count,result,_SHR_ERRMSG(result));
            }
            cli_out("  first element = %08X\n",element);
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"free",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            element = fromAsciiHexQuadByte(c);
            result = shr_aidxres_list_free(list,element);
            cli_out("shr_aidxres_list_free(*,%08X) returned %d (%s)\n",element,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"elementstate",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            element = fromAsciiHexQuadByte(c);
            result = shr_aidxres_list_elem_state(list,element);
            cli_out("shr_aidxres_list_elem_state(*,%08X) returned %d (%s)\n",element,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"state",strlen(c))) {
        if (list) {
            result = shr_aidxres_list_state(list,&first,&last,&min_valid,&max_valid,&free,&count,&largest,&block_factor);
            cli_out("shr_aidxres_list_state(*) returned %d (%s)\n",result,_SHR_ERRMSG(result));
            cli_out("  list.first        = %08X\n",first);
            cli_out("  list.last         = %08X\n",last);
            cli_out("  list.valid_low    = %08X\n",min_valid);
            cli_out("  list.valid_high   = %08X\n",max_valid);
            cli_out("  list.free         = %08X\n",free);
            cli_out("  list.alloc        = %08X\n",count);
            cli_out("  list.largest      = %08X\n",largest);
            cli_out("  list.block_factor = %08X\n",block_factor);
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"reserve",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            first = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            last = fromAsciiHexQuadByte(c);
            result = shr_aidxres_list_reserve(list,first,last);
            cli_out("shr_aidxres_list_reserve(*,%08X,%08X) returned %d (%s)\n",first,last,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"blockreserve",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            first = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            count = fromAsciiHexQuadByte(c);
            result = shr_aidxres_list_reserve_block(list,first,count);
            cli_out("shr_aidxres_list_reserve_block(*,%08X,%08X) returned %d (%s)\n",first,count,result,_SHR_ERRMSG(result));
        } else {
            cli_out("no list exists; create it first\n");
        }
    } else if (!sal_strncasecmp(c,"test",strlen(c))) {
        if (list) {
            _GET_NONBLANK(a,c);
            first = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            switch (first) {
            case 0:
                result = shr_aidxres_list_state(list,&first,&last,&min_valid,&max_valid,&free,&count,&largest,&block_factor);
                if (BCM_E_NONE != result) {
                    cli_out("shr_aidxres_list_state failed: %d (%s)\n", result, _SHR_ERRMSG(result));
                    return CMD_FAIL;
                }
                /* min_valid = blocks in test */
                min_valid = free >> block_factor;
                cli_out("list %u free elements at blocking factor %u, so each iteration\n",
                        free,
                        block_factor);
                cli_out("will be %u alloc or free calls, with any remaining allocs being\n",
                        min_valid);
                cli_out("cleaned up once this number of calls has completed, or after fault.\n");
                elems = sal_alloc(sizeof(unsigned int) * min_valid, "afl test elems");
                sizes = sal_alloc(sizeof(unsigned int) * min_valid, "afl test sizes");
                if ((!elems) || (!sizes)) {
                    cli_out("unable to allocate memory for %d elems+sizes\n", min_valid);
                    if (elems) {
                        sal_free(elems);
                    }
                    if (sizes) {
                        sal_free(sizes);
                    }
                    return CMD_FAIL;
                }
                for (count = 0; count < min_valid; count++) {
                    elems[count] = max_valid + 1;
                    sizes[count] = 0;
                }
                /* count = test iterations */
                largest = 0;
                iter = 0;
                count = fromAsciiHexQuadByte(c);
                iter_max = count * min_valid;
                iter_curr = 0;
                occ = 0;
                occ_max = 0;
                occ_min = 0;
                occb_min = 0;
                occb_max = 0;
                blk = 0;
                blk_min = 0;
                blk_max = 0;
                blko_min = 0;
                blko_max = 0;
                pass = 0;
                success = 0;
                fail = 0;
                while (count > 0) {
                    /* one iteration = min_valid alloc/free calls */
                    for (first = 0; first < min_valid; first++) {
                        iter_curr++;
                        /* last = which block to manipulate */
                        last = sal_rand() % min_valid;
                        if (0 == sizes[last]) {
                            /* free = number of elements to allocate */
                            free = (sal_rand() % (1u << block_factor) + 1);
                            result = shr_aidxres_list_alloc_block(list, free, &(elems[last]));
                            if (BCM_E_NONE != result) {
                                fail++;
                                cli_out("unable to allocate %u elements: %d (%s)\n", free, result, _SHR_ERRMSG(result));
                                break;
                            } else {
                                success++;
                                occ += free;
                                blk++;
                                if (occ > occ_max) {
                                    occ_max = occ;
                                    occb_max = blk;
                                }
                                if (blk > blk_max) {
                                    blk_max = blk;
                                    blko_max = occ;
                                }
                                iter++;
                                sizes[last] = free;
                                cli_out("%12d/%12d: alloc %12u elems at %12u\n", iter_curr, iter_max, sizes[last], elems[last]);
                            }
                        } else { /* if (0 == sizes[last]) */
                            result = shr_aidxres_list_free(list, elems[last]);
                            if (BCM_E_NONE != result) {
                                fail++;
                                cli_out("unable to free %u elements at %u: %d (%s)\n", sizes[last], elems[last], result, _SHR_ERRMSG(result));
                                break;
                            } else {
                                success++;
                                occ -= sizes[last];
                                blk--;
                                if (occ < occ_min) {
                                    occ_min = occ;
                                    occb_min = blk;
                                }
                                if (blk < blk_min) {
                                    blk_min = blk;
                                    blko_min = occ;
                                }
                                largest++;
                                cli_out("%12d/%12d: free  %12u elems at %12u\n", iter_curr, iter_max, sizes[last], elems[last]);
                                sizes[last] = 0;
                            }
                        } /* if (0 == sizes[last]) */
                    } /* for (first = 0; first < min_valid; first++) */
                    if (BCM_E_NONE == result) {
                        count--;
                    } else {
                        break;
                    }
                    if (!pass) {
                        occ_min = occ;
                        occb_min = blk;
                        blk_min = blk;
                        blko_min = occ;
                    }
                    pass++;
                } /* while (count > 0) */
                free = 0;
                for (count = 0; count < min_valid; count++) {
                    if (sizes[count]) {
                        result = shr_aidxres_list_free(list, elems[count]);
                        if (BCM_E_NONE != result) {
                            cli_out("unable to free %u elements at %u: %d (%s)\n", sizes[count], elems[count], result, _SHR_ERRMSG(result));
                        } else {
                            free++;
                            cli_out("%12d             : free  %12u elems at %12u\n", free, sizes[count], elems[count]);
                            sizes[count] = 0;
                        }
                        sizes[count] = 0;
                    }
                }
                cli_out("%u total accesses: %u alloc + %u free + %u free (cleanup)\n",
                        iter + largest + free,
                        iter,
                        largest,
                        free);
                cli_out("successful accesses = %u; failed accesses = %u\n",
                        success,
                        fail);
                cli_out("occupancy: limit = %u elems, %u blocks; final = %u elems, %u blocks\n",
                        min_valid << block_factor,
                        min_valid,
                        occ,
                        blk);
                cli_out("maximum: blocks = %u elems, %u blocks; elems = %u elems, %u blocks\n",
                        blko_max,
                        blk_max,
                        occ_max,
                        occb_max);
                cli_out("minimum: blocks = %u elems, %u blocks; elems = %u elems, %u blocks\n",
                        blko_min,
                        blk_min,
                        occ_min,
                        occb_min);
                cli_out("note 'minimum' is after end of first pass\n");
                sal_free(sizes);
                sal_free(elems);
                if (BCM_E_NONE != result) {
                    return CMD_FAIL;
                }
                break;
            case 1:
                result = shr_aidxres_list_state(list,&first,&last,&min_valid,&max_valid,&free,&count,&largest,&block_factor);
                if (BCM_E_NONE != result) {
                    cli_out("shr_aidxres_list_state failed: %d (%s)\n", result, _SHR_ERRMSG(result));
                    return CMD_FAIL;
                }
                /* min_valid = blocks in test */
                min_valid = free >> block_factor;
                cli_out("list %u free elements at blocking factor %u, so each iteration\n",
                        free,
                        block_factor);
                cli_out("will be %u alloc or free calls, with any remaining allocs being\n",
                        min_valid);
                cli_out("cleaned up once this number of calls has completed, or after fault.\n");
                elems = sal_alloc(sizeof(unsigned int) * min_valid, "afl test elems");
                sizes = sal_alloc(sizeof(unsigned int) * min_valid, "afl test sizes");
                if ((!elems) || (!sizes)) {
                    cli_out("unable to allocate memory for %d elems+sizes\n", min_valid);
                    if (elems) {
                        sal_free(elems);
                    }
                    if (sizes) {
                        sal_free(sizes);
                    }
                    return CMD_FAIL;
                }
                for (count = 0; count < min_valid; count++) {
                    elems[count] = max_valid + 1;
                    sizes[count] = 0;
                }
                /* count = test iterations */
                largest = 0;
                iter = 0;
                count = fromAsciiHexQuadByte(c);
                iter_max = count * min_valid;
                iter_curr = 0;
                occ = 0;
                occ_max = 0;
                occ_min = 0;
                occb_min = 0;
                occb_max = 0;
                blk = 0;
                blk_min = 0;
                blk_max = 0;
                blko_min = 0;
                blko_max = 0;
                pass = 0;
                success = 0;
                fail = 0;
                while (count > 0) {
                    /* one iteration = min_valid alloc/free calls */
                    for (first = 0; first < min_valid; first++) {
                        iter_curr++;
                        /* last = which block to manipulate */
                        last = sal_rand() % min_valid;
                        for (bias = 0; bias < bias_level; bias++) {
                            if (0 != sizes[last]) {
                                /* want to bias toward alloc, so try again */
                                last = sal_rand() % min_valid;
                            } else {
                                break;
                            }
                        }
                        if (0 == sizes[last]) {
                            /* free = number of elements to allocate */
                            free = (sal_rand() % (1u << block_factor) + 1);
                            result = shr_aidxres_list_alloc_block(list, free, &(elems[last]));
                            if (BCM_E_NONE != result) {
                                fail++;
                                cli_out("unable to allocate %u elements: %d (%s)\n", free, result, _SHR_ERRMSG(result));
                                break;
                            } else {
                                success++;
                                occ += free;
                                blk++;
                                if (occ > occ_max) {
                                    occ_max = occ;
                                    occb_max = blk;
                                }
                                if (blk > blk_max) {
                                    blk_max = blk;
                                    blko_max = occ;
                                }
                                iter++;
                                sizes[last] = free;
                                cli_out("%12d/%12d: alloc %12u elems at %12u\n", iter_curr, iter_max, sizes[last], elems[last]);
                            }
                        } else { /* if (0 == sizes[last]) */
                            result = shr_aidxres_list_free(list, elems[last]);
                            if (BCM_E_NONE != result) {
                                fail++;
                                cli_out("unable to free %u elements at %u: %d (%s)\n", sizes[last], elems[last], result, _SHR_ERRMSG(result));
                                break;
                            } else {
                                success++;
                                occ -= sizes[last];
                                blk--;
                                if (occ < occ_min) {
                                    occ_min = occ;
                                    occb_min = blk;
                                }
                                if (blk < blk_min) {
                                    blk_min = blk;
                                    blko_min = occ;
                                }
                                largest++;
                                cli_out("%12d/%12d: free  %12u elems at %12u\n", iter_curr, iter_max, sizes[last], elems[last]);
                                sizes[last] = 0;
                            }
                        } /* if (0 == sizes[last]) */
                    } /* for (first = 0; first < min_valid; first++) */
                    if (BCM_E_NONE == result) {
                        count--;
                    } else {
                        break;
                    }
                    if (!pass) {
                        occ_min = occ;
                        occb_min = blk;
                        blk_min = blk;
                        blko_min = occ;
                    }
                    pass++;
                } /* while (count > 0) */
                free = 0;
                for (count = 0; count < min_valid; count++) {
                    if (sizes[count]) {
                        result = shr_aidxres_list_free(list, elems[count]);
                        if (BCM_E_NONE != result) {
                            cli_out("unable to free %u elements at %u: %d (%s)\n", sizes[count], elems[count], result, _SHR_ERRMSG(result));
                        } else {
                            free++;
                            cli_out("%12d             : free  %12u elems at %12u\n", free, sizes[count], elems[count]);
                            sizes[count] = 0;
                        }
                        sizes[count] = 0;
                    }
                }
                cli_out("%u total accesses: %u alloc + %u free + %u free (cleanup)\n",
                        iter + largest + free,
                        iter,
                        largest,
                        free);
                cli_out("successful accesses = %u; failed accesses = %u\n",
                        success,
                        fail);
                cli_out("occupancy: limit = %u elems, %u blocks; final = %u elems, %u blocks\n",
                        min_valid << block_factor,
                        min_valid,
                        occ,
                        blk);
                cli_out("maximum: blocks = %u elems, %u blocks; elems = %u elems, %u blocks\n",
                        blko_max,
                        blk_max,
                        occ_max,
                        occb_max);
                cli_out("minimum: blocks = %u elems, %u blocks; elems = %u elems, %u blocks\n",
                        blko_min,
                        blk_min,
                        occ_min,
                        occb_min);
                cli_out("note 'minimum' is after end of first pass\n");
                sal_free(sizes);
                sal_free(elems);
                if (BCM_E_NONE != result) {
                    return CMD_FAIL;
                }
                break;
            case 2:
                result = shr_aidxres_list_state(list,&first,&last,&min_valid,&max_valid,&free,&count,&largest,&block_factor);
                if (BCM_E_NONE != result) {
                    cli_out("shr_aidxres_list_state failed: %d (%s)\n", result, _SHR_ERRMSG(result));
                    return CMD_FAIL;
                }
                /* min_valid = blocks in test */
                min_valid = free >> block_factor;
                cli_out("list %u free elements at blocking factor %u, so each iteration\n",
                        free,
                        block_factor);
                cli_out("will be %u alloc or free calls, with any remaining allocs being\n",
                        min_valid);
                cli_out("cleaned up once this number of calls has completed, or after fault.\n");
                elems = sal_alloc(sizeof(unsigned int) * min_valid, "afl test elems");
                sizes = sal_alloc(sizeof(unsigned int) * min_valid, "afl test sizes");
                if ((!elems) || (!sizes)) {
                    cli_out("unable to allocate memory for %d elems+sizes\n", min_valid);
                    if (elems) {
                        sal_free(elems);
                    }
                    if (sizes) {
                        sal_free(sizes);
                    }
                    return CMD_FAIL;
                }
                for (count = 0; count < min_valid; count++) {
                    elems[count] = max_valid + 1;
                    sizes[count] = 0;
                }
                /* count = test iterations */
                largest = 0;
                iter = 0;
                count = fromAsciiHexQuadByte(c);
                iter_max = count * min_valid;
                iter_curr = 0;
                occ = 0;
                occ_max = 0;
                occ_min = 0;
                occb_min = 0;
                occb_max = 0;
                blk = 0;
                blk_min = 0;
                blk_max = 0;
                blko_min = 0;
                blko_max = 0;
                pass = 0;
                success = 0;
                fail = 0;
                while (count > 0) {
                    /* one iteration = min_valid alloc/free calls */
                    for (first = 0; first < min_valid; first++) {
                        iter_curr++;
                        /* last = which block to manipulate */
                        last = sal_rand() % min_valid;
                        for (bias = 0; bias < bias_level; bias++) {
                            if (0 == sizes[last]) {
                                /* want to bias toward alloc, so try again */
                                last = sal_rand() % min_valid;
                            } else {
                                break;
                            }
                        }
                        if (0 == sizes[last]) {
                            /* free = number of elements to allocate */
                            free = (sal_rand() % (1u << block_factor) + 1);
                            result = shr_aidxres_list_alloc_block(list, free, &(elems[last]));
                            if (BCM_E_NONE != result) {
                                fail++;
                                cli_out("unable to allocate %u elements: %d (%s)\n", free, result, _SHR_ERRMSG(result));
                                break;
                            } else {
                                success++;
                                occ += free;
                                blk++;
                                if (occ > occ_max) {
                                    occ_max = occ;
                                    occb_max = blk;
                                }
                                if (blk > blk_max) {
                                    blk_max = blk;
                                    blko_max = occ;
                                }
                                iter++;
                                sizes[last] = free;
                                cli_out("%12d/%12d: alloc %12u elems at %12u\n", iter_curr, iter_max, sizes[last], elems[last]);
                            }
                        } else { /* if (0 == sizes[last]) */
                            result = shr_aidxres_list_free(list, elems[last]);
                            if (BCM_E_NONE != result) {
                                fail++;
                                cli_out("unable to free %u elements at %u: %d (%s)\n", sizes[last], elems[last], result, _SHR_ERRMSG(result));
                                break;
                            } else {
                                success++;
                                occ -= sizes[last];
                                blk--;
                                if (occ < occ_min) {
                                    occ_min = occ;
                                    occb_min = blk;
                                }
                                if (blk < blk_min) {
                                    blk_min = blk;
                                    blko_min = occ;
                                }
                                largest++;
                                cli_out("%12d/%12d: free  %12u elems at %12u\n", iter_curr, iter_max, sizes[last], elems[last]);
                                sizes[last] = 0;
                            }
                        } /* if (0 == sizes[last]) */
                    } /* for (first = 0; first < min_valid; first++) */
                    if (BCM_E_NONE == result) {
                        count--;
                    } else {
                        break;
                    }
                    if (!pass) {
                        occ_min = occ;
                        occb_min = blk;
                        blk_min = blk;
                        blko_min = occ;
                    }
                    pass++;
                } /* while (count > 0) */
                free = 0;
                for (count = 0; count < min_valid; count++) {
                    if (sizes[count]) {
                        result = shr_aidxres_list_free(list, elems[count]);
                        if (BCM_E_NONE != result) {
                            cli_out("unable to free %u elements at %u: %d (%s)\n", sizes[count], elems[count], result, _SHR_ERRMSG(result));
                        } else {
                            free++;
                            cli_out("%12d             : free  %12u elems at %12u\n", free, sizes[count], elems[count]);
                            sizes[count] = 0;
                        }
                        sizes[count] = 0;
                    }
                }
                cli_out("%u total accesses: %u alloc + %u free + %u free (cleanup)\n",
                        iter + largest + free,
                        iter,
                        largest,
                        free);
                cli_out("successful accesses = %u; failed accesses = %u\n",
                        success,
                        fail);
                cli_out("occupancy: limit = %u elems, %u blocks; final = %u elems, %u blocks\n",
                        min_valid << block_factor,
                        min_valid,
                        occ,
                        blk);
                cli_out("maximum: blocks = %u elems, %u blocks; elems = %u elems, %u blocks\n",
                        blko_max,
                        blk_max,
                        occ_max,
                        occb_max);
                cli_out("minimum: blocks = %u elems, %u blocks; elems = %u elems, %u blocks\n",
                        blko_min,
                        blk_min,
                        occ_min,
                        occb_min);
                cli_out("note 'minimum' is after end of first pass\n");
                sal_free(sizes);
                sal_free(elems);
                if (BCM_E_NONE != result) {
                    return CMD_FAIL;
                }
                break;
            default:
                cli_out("no such test; supported tests are 0..0\n");
            }
        } else { /* if (list) */
            cli_out("no list exists at this time\n");
            return CMD_FAIL;
        } /* if (list) */
    } else if (!sal_strncasecmp(c,"paranoia",strlen(c))) {
        (c) = ARG_GET(a);
        if (NULL != c) {
            first = fromAsciiHexQuadByte(c);
            _aidxres_sanity_settings = first;
        }
        cli_out("aidxres paranoia flags set to %08X\n", _aidxres_sanity_settings);
    } else {
        return CMD_USAGE;
    }
    return CMD_OK;
}


cmd_result_t
cmd_sbx_mcast(int unit, args_t *a)
{
    char                    *c;
    bcm_mcast_addr_t        mcAddr;
    sal_mac_addr_t          mac;
    bcm_vlan_t              vid;
    int                     result;
    bcm_port_t              port;
    bcm_pbmp_t              pbmp;

    sal_memset(&mcAddr,0,sizeof(mcAddr));
    sal_memset(&mac, 0, sizeof(mac));

    _GET_NONBLANK(a,c);
    if (!sal_strncasecmp(c,"init",strlen(c))) {
        result = bcm_mcast_init(unit);
        cli_out("bcm_mcast_init(%d) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
    } else if (!sal_strncasecmp(c,"add",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"l2mcindex",strlen(c))) {
            _GET_NONBLANK(a,c);
            mcAddr.l2mc_index = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            mcAddr.vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&(mcAddr.mac))) {
                return CMD_USAGE;
            }
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.pbmp);
            _SHR_PBMP_WORD_SET(mcAddr.pbmp,0,fromAsciiHexQuadByte(c));
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.ubmp);
            _SHR_PBMP_WORD_SET(mcAddr.ubmp,0,fromAsciiHexQuadByte(c));
            result = bcm_mcast_addr_add_w_l2mcindex(unit, &mcAddr);
            cli_out("bcm_mcast_addr_add_w_l2mcindex(%d,*{%08X,%02X:%02X:%02X:%02X:%02X:%02X,%03X,%08X,%08X}) returned %d (%s)\n",unit,mcAddr.l2mc_index,mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5],mcAddr.vid,_SHR_PBMP_WORD_GET(mcAddr.pbmp,0),_SHR_PBMP_WORD_GET(mcAddr.ubmp,0),result,_SHR_ERRMSG(result));
        } else {
            mcAddr.vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&(mcAddr.mac))) {
                return CMD_USAGE;
            }
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.pbmp);
            _SHR_PBMP_WORD_SET(mcAddr.pbmp,0,fromAsciiHexQuadByte(c));
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.ubmp);
            _SHR_PBMP_WORD_SET(mcAddr.ubmp,0,fromAsciiHexQuadByte(c));
            result = bcm_mcast_addr_add(unit, &mcAddr);
            cli_out("bcm_mcast_addr_add(%d,*{%02X:%02X:%02X:%02X:%02X:%02X,%03X,%08X,%08X}) returned %d (%s)\n",unit,mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5],mcAddr.vid,_SHR_PBMP_WORD_GET(mcAddr.pbmp,0),_SHR_PBMP_WORD_GET(mcAddr.ubmp,0),result,_SHR_ERRMSG(result));
            cli_out("  resulting l2mc_index = %08X\n",mcAddr.l2mc_index);
        }
    } else if (!sal_strncasecmp(c,"remove",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"l2mcindex",strlen(c))) {
            _GET_NONBLANK(a,c);
            mcAddr.l2mc_index = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            mcAddr.vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&(mcAddr.mac))) {
                return CMD_USAGE;
            }
            result = bcm_mcast_addr_remove_w_l2mcindex(unit, &mcAddr);
            cli_out("bcm_mcast_addr_remove_w_l2mcindex(%d,(%08X,%02X:%02X:%02X:%02X:%02X:%02X,%03X)) returned %d (%s)\n",unit,mcAddr.l2mc_index,mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5],mcAddr.vid,result,_SHR_ERRMSG(result));
        } else {
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&mac)) {
                return CMD_USAGE;
            }
            result = bcm_mcast_addr_remove(unit, mac, vid);
            cli_out("bcm_mcast_addr_remove(%d,%02X:%02X:%02X:%02X:%02X:%02X,%03X) returned %d (%s)\n",unit,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vid,result,_SHR_ERRMSG(result));
        }
    } else if (!sal_strncasecmp(c,"port",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"add",strlen(c))) {
            _GET_NONBLANK(a,c);
            mcAddr.vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&(mcAddr.mac))) {
                return CMD_USAGE;
            }
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.pbmp);
            _SHR_PBMP_WORD_SET(mcAddr.pbmp,0,fromAsciiHexQuadByte(c));
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.ubmp);
            _SHR_PBMP_WORD_SET(mcAddr.ubmp,0,fromAsciiHexQuadByte(c));
            result = bcm_mcast_port_add(unit, &mcAddr);
            cli_out("bcm_mcast_port_add(%d,*{%02X:%02X:%02X:%02X:%02X:%02X,%03X,%08X,%08X}) returned %d (%s)\n",unit,mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5],mcAddr.vid,_SHR_PBMP_WORD_GET(mcAddr.pbmp,0),_SHR_PBMP_WORD_GET(mcAddr.ubmp,0),result,_SHR_ERRMSG(result));
        } else if (!sal_strncasecmp(c,"remove",strlen(c))) {
            _GET_NONBLANK(a,c);
            mcAddr.vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&(mcAddr.mac))) {
                return CMD_USAGE;
            }
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(mcAddr.pbmp);
            _SHR_PBMP_WORD_SET(mcAddr.pbmp,0,fromAsciiHexQuadByte(c));
            BCM_PBMP_CLEAR(mcAddr.ubmp);
            result = bcm_mcast_port_remove(unit, &mcAddr);
            cli_out("bcm_mcast_port_remove(%d,*{%02X:%02X:%02X:%02X:%02X:%02X,%03X,%08X,%08X}) returned %d (%s)\n",unit,mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5],mcAddr.vid,_SHR_PBMP_WORD_GET(mcAddr.pbmp,0),_SHR_PBMP_WORD_GET(mcAddr.ubmp,0),result,_SHR_ERRMSG(result));
        } else if (!sal_strncasecmp(c,"get",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            if (!fromAsciiMacAddress(c,&mac)) {
                return CMD_USAGE;
            }
            result = bcm_mcast_port_get(unit, mac, vid, &mcAddr);
            cli_out("bcm_mcast_port_get(%d,%02X:%02X:%02X:%02X:%02X:%02X,%03X,*) returned %d (%s)\n",unit,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vid,result,_SHR_ERRMSG(result));
            cli_out("  bcm_mcast_addr.mac        = %02X:%02X:%02X:%02X:%02X:%02X\n",mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5]);
            cli_out("  bcm_mcast_addr.vid        = %03X\n",mcAddr.vid);
            cli_out("  bcm_mcast_addr.cos_dst    = %08X\n",mcAddr.cos_dst);
            cli_out("  bcm_mcast_addr.pbmp       = %08X\n",_SHR_PBMP_WORD_GET(mcAddr.pbmp,0));
            cli_out("  bcm_mcast_addr.ubmp       = %08X\n",_SHR_PBMP_WORD_GET(mcAddr.ubmp,0));
            cli_out("  bcm_mcast_addr.l2mc_index = %08X\n",mcAddr.l2mc_index);
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"leave",strlen(c))) {
        _GET_NONBLANK(a,c);
        vid = fromAsciiHexQuadByte(c) & 0xFFF;
        _GET_NONBLANK(a,c);
        if (!fromAsciiMacAddress(c,&(mac))) {
            return CMD_USAGE;
        }
        _GET_NONBLANK(a,c);
        port = fromAsciiHexQuadByte(c);
        result = bcm_mcast_leave(unit, mac, vid, port);
        cli_out("bcm_mcast_leave(%d,%02X:%02X:%02X:%02X:%02X:%02X,%03X,%08X) returned %d (%s)\n",unit,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vid,port,result,_SHR_ERRMSG(result));
    } else if (!sal_strncasecmp(c,"join",strlen(c))) {
        _GET_NONBLANK(a,c);
        vid = fromAsciiHexQuadByte(c) & 0xFFF;
        _GET_NONBLANK(a,c);
        if (!fromAsciiMacAddress(c,&(mac))) {
            return CMD_USAGE;
        }
        _GET_NONBLANK(a,c);
        port = fromAsciiHexQuadByte(c);
        result = bcm_mcast_join(unit, mac, vid, port, &mcAddr, &pbmp);
        cli_out("bcm_mcast_join(%d,%02X:%02X:%02X:%02X:%02X:%02X,%03X,%08X) returned %d (%s)\n",unit,mac[0],mac[1],mac[2],mac[3],mac[4],mac[5],vid,port,result,_SHR_ERRMSG(result));
        cli_out("  bcm_mcast_addr.mac        = %02X:%02X:%02X:%02X:%02X:%02X\n",mcAddr.mac[0],mcAddr.mac[1],mcAddr.mac[2],mcAddr.mac[3],mcAddr.mac[4],mcAddr.mac[5]);
        cli_out("  bcm_mcast_addr.vid        = %03X\n",mcAddr.vid);
        cli_out("  bcm_mcast_addr.cos_dst    = %08X\n",mcAddr.cos_dst);
        cli_out("  bcm_mcast_addr.pbmp       = %08X\n",_SHR_PBMP_WORD_GET(mcAddr.pbmp,0));
        cli_out("  bcm_mcast_addr.ubmp       = %08X\n",_SHR_PBMP_WORD_GET(mcAddr.ubmp,0));
        cli_out("  bcm_mcast_addr.l2mc_index = %08X\n",mcAddr.l2mc_index);
        cli_out("  bcm_pbmp                  = %08X\n",_SHR_PBMP_WORD_GET(pbmp,0));
    } else {
        return CMD_USAGE;
    }
    return CMD_OK;
}

cmd_result_t
cmd_sbx_vlan(int unit, args_t *a)
{
    char                *subcmd, *c;
    int                 r = 0;
    vlan_id_t           id = VLAN_ID_INVALID;
    pbmp_t              arg_ubmp;
    pbmp_t              arg_pbmp;
    parse_table_t       pt;
    cmd_result_t        retCode;
    char                *bcm_vlan_mcast_flood_str[] = BCM_VLAN_MCAST_FLOOD_STR;

    BCM_PBMP_CLEAR(arg_ubmp);
    BCM_PBMP_CLEAR(arg_pbmp);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (sal_strcasecmp(subcmd, "create") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap",      PQ_DFL|PQ_PBMP,
                        (void *)(0), &arg_pbmp, NULL);
        parse_table_add(&pt, "UntagBitMap",     PQ_DFL|PQ_PBMP,
                        (void *)(0), &arg_ubmp, NULL);

        if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) > 0) {
            cli_out("%s: ERROR: Unknown option: %s\n",
                    ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        parse_arg_eq_done(&pt);

        if ((r = bcm_vlan_create(unit, id)) < 0) {
            goto bcm_err;
        }

        if ((r = bcm_vlan_port_add(unit, id, arg_pbmp, arg_ubmp)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "destroy") == 0) {
        if ((c = ARG_GET(a)) == NULL)
            return CMD_USAGE;

        id = parse_integer(c);

        if ((r = bcm_vlan_destroy(unit, id)) < 0)
            goto bcm_err;

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "clear") == 0) {
        if ((r = bcm_vlan_destroy_all(unit)) < 0) {
            goto bcm_err;
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "add") == 0 ||
        sal_strcasecmp(subcmd, "remove") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);

        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PortBitMap", PQ_DFL|PQ_PBMP,
                        (void *)(0), &arg_pbmp, NULL);
        if (sal_strcasecmp(subcmd, "add") == 0) {
            parse_table_add(&pt, "UntagBitMap", PQ_DFL|PQ_PBMP,
                            (void *)(0), &arg_ubmp, NULL);
        }

        if (!parseEndOk( a, &pt, &retCode)) {
            return retCode;
        }

        if (sal_strcasecmp(subcmd, "remove") == 0) {
            if ((r = bcm_vlan_port_remove(unit, id, arg_pbmp)) < 0) {
                goto bcm_err;
            }
        } else {
            if ((r = bcm_vlan_port_add(unit, id, arg_pbmp, arg_ubmp)) < 0) {
                goto bcm_err;
            }
        }

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "MulticastFlood") == 0) {
        bcm_vlan_mcast_flood_t  mode;
        if ((c = ARG_GET(a)) == NULL) {
            return CMD_USAGE;
        }

        id = parse_integer(c);
        if ((c = ARG_GET(a)) == NULL) {
            if ((r = bcm_vlan_mcast_flood_get(unit, id, &mode)) < 0) {
                goto bcm_err;
            }
            cli_out("vlan %d Multicast Flood Mode is %s\n",
                    id, bcm_vlan_mcast_flood_str[mode]);
            return CMD_OK;
        }
        mode = parse_integer(c);
        if ((r = bcm_vlan_mcast_flood_set(unit, id, mode)) < 0) {
            goto bcm_err;
        }
        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        bcm_vlan_data_t *list;
        int             count, i;
        char            bufp[FORMAT_PBMP_MAX], bufu[FORMAT_PBMP_MAX];
        char            pfmtp[SOC_PBMP_FMT_LEN];
        char            pfmtu[SOC_PBMP_FMT_LEN];

        if ((c = ARG_GET(a)) != NULL) {
            id = parse_integer(c);
        }

        if ((r = bcm_vlan_list(unit, &list, &count)) < 0) {
            goto bcm_err;
        }

        for (i = 0; i < count; i++) {
            if (id == VLAN_ID_INVALID || list[i].vlan_tag == id) {
                bcm_vlan_mcast_flood_t  mode;
                if ((r = bcm_vlan_mcast_flood_get(unit,
                                                  list[i].vlan_tag,
                                                  &mode)) < 0) {
                    if (r == BCM_E_UNAVAIL) {
                        mode = BCM_VLAN_MCAST_FLOOD_COUNT;
                    } else {
                        goto bcm_err;
                    }
                }
                format_pbmp(unit, bufp, sizeof(bufp), list[i].port_bitmap);
                format_pbmp(unit, bufu, sizeof(bufu), list[i].ut_port_bitmap);
                cli_out("VLAN: %4d  ports: %s (%s) \n   untagged ports: %s (%s)\n%s\n",
                        list[i].vlan_tag,
                        SOC_PBMP_FMT(list[i].port_bitmap, pfmtp), bufp,
                        SOC_PBMP_FMT(list[i].ut_port_bitmap, pfmtu), bufu,
                        bcm_vlan_mcast_flood_str[mode]
                        );
            }
        }

        bcm_vlan_list_destroy(unit, list, count);

        return CMD_OK;
    }

    if (sal_strcasecmp(subcmd, "default") == 0) {
        if ((c = ARG_GET(a)) != NULL) {
            id = parse_integer(c);
        }

        if (id == VLAN_ID_INVALID) {
            if ((r = bcm_vlan_default_get(unit, &id)) < 0) {
                goto bcm_err;
            }

            cli_out("Default VLAN ID is %d\n", id);
        } else {
            if ((r = bcm_vlan_default_set(unit, id)) < 0) {
                goto bcm_err;
            }

            cli_out("Default VLAN ID set to %d\n", id);
        }

        return CMD_OK;
    }

    /* Protocol vlan selection */
    if (sal_strcasecmp(subcmd, "protocol") == 0 ||
        sal_strcasecmp(subcmd, "proto") == 0) {

        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: missing protocol subcommand\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            bcm_pbmp_t  pbmp;
            bcm_port_t  port;
            int         frame, ether, vlan, prio;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_PBMP, 0, &pbmp, NULL);
            parse_table_add(&pt, "Frame", PQ_INT, 0, &frame, NULL);
            parse_table_add(&pt, "Ether", PQ_HEX, 0, &ether, NULL);
            parse_table_add(&pt, "VLan", PQ_INT, 0, &vlan, NULL);
            parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            PBMP_ITER(pbmp, port) {
                if ((r = bcm_port_protocol_vlan_add(unit, port, frame,
                                                    ether, vlan)) < 0) {
                    goto bcm_err;
                }
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            bcm_pbmp_t  pbmp;
            bcm_port_t  port;
            int         frame, ether;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_PBMP, 0, &pbmp, NULL);
            parse_table_add(&pt, "Frame", PQ_INT, 0, &frame, NULL);
            parse_table_add(&pt, "Ether", PQ_HEX, 0, &ether, NULL);
            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            PBMP_ITER(pbmp, port) {
                if ((r = bcm_port_protocol_vlan_delete(unit, port, frame,
                                                       ether)) < 0) {
                    goto bcm_err;
                }
            }
            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            bcm_pbmp_t  pbmp;
            bcm_port_t  port;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "PortBitMap", PQ_PBMP, 0, &pbmp, NULL);
            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            PBMP_ITER(pbmp, port) {
                if ((r = bcm_port_protocol_vlan_delete_all(unit,
                                                           port)) < 0) {
                    goto bcm_err;
                }
            }
            return CMD_OK;
        }

        cli_out("%s: ERROR: unknown protocol subcommand: %s\n",
                ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }

    /* MAC address vlan selection */
    if (sal_strcasecmp(subcmd, "mac") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: missing mac subcommand\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            bcm_mac_t   mac;
            int         vlan, prio, cng;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "MACaddress", PQ_MAC, 0, &mac, NULL);
            parse_table_add(&pt, "VLan", PQ_INT, 0, &vlan, NULL);
            parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
            parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            if (cng) {
                prio |= BCM_PRIO_DROP_FIRST;
            }

            if ((r = bcm_vlan_mac_add(unit, mac, vlan, prio)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            bcm_mac_t   mac;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "MACaddress", PQ_MAC, 0, &mac, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            if ((r = bcm_vlan_mac_delete(unit, mac)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_mac_delete_all(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        cli_out("%s: ERROR: Unknown MAC subcommand: %s\n", ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }

    /* VLAN translate selection */
    if (sal_strcasecmp(subcmd, "translate") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: Missing translate subcommand\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (sal_strcasecmp(subcmd, "On") == 0) {
            if ((r = bcm_vlan_control_set(unit, bcmVlanTranslate,
                                          TRUE)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "Off") == 0) {
            if ((r = bcm_vlan_control_set(unit, bcmVlanTranslate,
                                          FALSE)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            int         port, old_vlan, new_vlan, prio, cng;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
            parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);
            parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
            parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
            parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            if (cng) {
                prio |= BCM_PRIO_DROP_FIRST;
            }

            if ((r = bcm_vlan_translate_add(unit, port,
                                            old_vlan, new_vlan, prio)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            int         port, old_vlan;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
            parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            if ((r = bcm_vlan_translate_delete(unit, port, old_vlan)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_translate_delete_all(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "egress") == 0) {
            char * subsubcmd = ARG_GET(a);
            if (subsubcmd == NULL) {
                cli_out("%s: ERROR: Missing translate egress subcommand\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }

            if (sal_strcasecmp(subsubcmd, "add") == 0) {
                int             port, old_vlan, new_vlan, prio, cng;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);
                parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
                parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
                parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

                if (!parseEndOk( a, &pt, &retCode)) {
                    return retCode;
                }

                if (cng) {
                    prio |= BCM_PRIO_DROP_FIRST;
                }

                if ((r = bcm_vlan_translate_egress_add(unit, port,
                                            old_vlan, new_vlan, prio)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "delete") == 0) {
                int             port, old_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

                if (!parseEndOk( a, &pt, &retCode)) {
                    return retCode;
                }

                if ((r = bcm_vlan_translate_egress_delete(unit, port,
                                                          old_vlan)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "clear") == 0) {
                if ((r = bcm_vlan_translate_egress_delete_all(unit)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }
        }

        if (sal_strcasecmp(subcmd, "dtag") == 0) {
            char * subsubcmd = ARG_GET(a);
            if (subsubcmd == NULL) {
                cli_out("%s: ERROR: Missing translate dtag subcommand\n",
                        ARG_CMD(a));
                return CMD_FAIL;
            }

            if (sal_strcasecmp(subsubcmd, "add") == 0) {
                int             port, old_vlan, new_vlan, prio, cng;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);
                parse_table_add(&pt, "NewVLan", PQ_INT, 0, &new_vlan, NULL);
                parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
                parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

                if (!parseEndOk( a, &pt, &retCode)) {
                    return retCode;
                }

                if (cng) {
                    prio |= BCM_PRIO_DROP_FIRST;
                }

                if ((r = bcm_vlan_dtag_add(unit, port,
                                           old_vlan, new_vlan, prio)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }
            if (sal_strcasecmp(subsubcmd, "delete") == 0) {
                int             port, old_vlan;

                parse_table_init(unit, &pt);
                parse_table_add(&pt, "Port", PQ_INT, 0, &port, NULL);
                parse_table_add(&pt, "OldVLan", PQ_INT, 0, &old_vlan, NULL);

                if (!parseEndOk( a, &pt, &retCode)) {
                    return retCode;
                }

                if ((r = bcm_vlan_dtag_delete(unit, port, old_vlan)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }

            if (sal_strcasecmp(subsubcmd, "clear") == 0) {
                if ((r = bcm_vlan_dtag_delete_all(unit)) < 0) {
                    goto bcm_err;
                }

                return CMD_OK;
            }
        }

        cli_out("%s: ERROR: Unknown translate subcommand: %s\n",
                ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }

    /* IP4 subnet based vlan selection */
    if (sal_strcasecmp(subcmd, "ip4") == 0 ||
        sal_strcasecmp(subcmd, "ip") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: missing ip4 subcommand\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            bcm_ip_t    ipaddr, subnet;
            int         vlan, prio, cng;
            bcm_vlan_ip_t vlan_ip;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "IPaddr", PQ_IP, 0, &ipaddr, NULL);
            parse_table_add(&pt, "NetMask", PQ_IP, 0, &subnet, NULL);
            parse_table_add(&pt, "VLan", PQ_INT, 0, &vlan, NULL);
            parse_table_add(&pt, "Prio", PQ_INT, 0, &prio, NULL);
            parse_table_add(&pt, "Cng", PQ_INT, 0, &cng, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }
            if (cng) {
                prio |= BCM_PRIO_DROP_FIRST;
            }

            bcm_vlan_ip_t_init(&vlan_ip);
            vlan_ip.ip4 = ipaddr;
            vlan_ip.mask = subnet;
            vlan_ip.vid = vlan;
            vlan_ip.prio = prio;

	    if ((r = bcm_vlan_ip_add(unit, &vlan_ip)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            bcm_ip_t    ipaddr, subnet;
            bcm_vlan_ip_t vlan_ip;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "IPaddr", PQ_IP, 0, &ipaddr, NULL);
            parse_table_add(&pt, "NetMask", PQ_IP, 0, &subnet, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            bcm_vlan_ip_t_init(&vlan_ip);
            vlan_ip.ip4 = ipaddr;
            vlan_ip.mask = subnet;

	    if ((r = bcm_vlan_ip_delete(unit, &vlan_ip)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_ip_delete_all(unit)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }

        cli_out("%s: ERROR: Unknown ip4 subcommand: %s\n", ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }

    if (sal_strcasecmp(subcmd, "ip6") == 0) {
        subcmd = ARG_GET(a);
        if (subcmd == NULL) {
            cli_out("%s: ERROR: missing ip6 subcommand\n", ARG_CMD(a));
            return CMD_FAIL;
        }

        if (sal_strcasecmp(subcmd, "add") == 0) {
            bcm_vlan_ip_t       ipaddr;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "IPaddr", PQ_IP6, 0, &ipaddr.ip6, NULL);
            parse_table_add(&pt, "prefiX", PQ_INT, 0, &ipaddr.prefix, NULL);
            parse_table_add(&pt, "VLan", PQ_INT, 0, &ipaddr.vid, NULL);
            parse_table_add(&pt, "Prio", PQ_INT, 0, &ipaddr.prio, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                    return retCode;
            }
        ipaddr.flags = BCM_VLAN_SUBNET_IP6;
            if ((r = bcm_vlan_ip_add(unit, &ipaddr)) < 0) {
                    goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "delete") == 0) {
            bcm_vlan_ip_t       ipaddr;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "IPaddr", PQ_IP6, 0, &ipaddr.ip6, NULL);
            parse_table_add(&pt, "prefiX", PQ_INT, 0, &ipaddr.prefix, NULL);

            if (!parseEndOk( a, &pt, &retCode)) {
                    return retCode;
            }

        ipaddr.flags = BCM_VLAN_SUBNET_IP6;
            if ((r = bcm_vlan_ip_delete(unit, &ipaddr)) < 0) {
                    goto bcm_err;
            }

            return CMD_OK;
        }

        if (sal_strcasecmp(subcmd, "clear") == 0) {
            if ((r = bcm_vlan_ip_delete_all(unit)) < 0) {
                    goto bcm_err;
            }

            return CMD_OK;
        }

        cli_out("%s: ERROR: Unknown ip4 subcommand: %s\n", ARG_CMD(a), subcmd);

        return CMD_FAIL;
    }

    /* Vlan control */
    if (sal_strcasecmp(subcmd, "control") == 0 ||
        sal_strcasecmp(subcmd, "ctrl") == 0) {
        char    *value, *tname;
        int     ttype, i, varg, matched;

        static struct {                 /* match enum from bcm/vlan.h */
            int         type;
            char        *name;
        } typenames[] = {
            { bcmVlanDropUnknown,       "dropunknown" },
            { bcmVlanPreferIP4,         "preferip4" },
            { bcmVlanPreferMAC,         "prefermac" },
            { bcmVlanShared,            "shared" },
            { bcmVlanSharedID,          "sharedid" },
            { bcmVlanIgnorePktTag,      "ignorepkttag" },
            { 0,                        NULL }          /* LAST ENTRY */
        };

        subcmd = ARG_GET(a);
        value = ARG_GET(a);

        matched = 0;

        for (i = 0; typenames[i].name != NULL; i++) {
            tname = typenames[i].name;
            if (subcmd == NULL || sal_strcasecmp(subcmd, tname) == 0) {
                matched += 1;
                ttype = typenames[i].type;
                if (value == NULL) {
                    r = bcm_vlan_control_get(unit, ttype, &varg);
                    if (r < 0) {
                        cli_out("%-20s-\t%s\n", tname, bcm_errmsg(r));
                    } else {
                        cli_out("%-20s%d\n", tname, varg);
                    }
                } else {
                    varg = parse_integer(value);
                    r = bcm_vlan_control_set(unit, ttype, varg);
                    if (r < 0) {
                        cli_out("%s\tERROR: %s\n", tname, bcm_errmsg(r));
                    }
                }
            }
        }

        if (matched == 0) {
            cli_out("%s: ERROR: Unknown control name\n", subcmd);
            return CMD_FAIL;
        }

        return CMD_OK;
    }

    /* Vlan port control */
    if (sal_strcasecmp(subcmd, "port") == 0) {
        char    *value, *tname;
        int     ttype, i, varg, matched;
        bcm_port_t port;
        pbmp_t       pbm;
        static struct {                 /* match enum from bcm/vlan.h */
            int         type;
            char        *name;
        } typenames[] = {
            { bcmVlanPortPreferIP4,            "preferip4" },
            { bcmVlanPortPreferMAC,            "prefermac" },
            { bcmVlanTranslateIngressEnable,   "translateingress" },
            { bcmVlanTranslateIngressMissDrop, "translateingressmissdrop" },
            { bcmVlanTranslateEgressEnable,    "translateegress" },
            { bcmVlanTranslateEgressMissDrop,  "translateegressmissdrop" },
     { bcmVlanTranslateEgressMissUntaggedDrop, "translateegressmissuntagdrop" },
     { bcmVlanTranslateEgressMissUntag,        "translateegressmissuntag"},
            { bcmVlanLookupMACEnable,          "lookupmac" },
            { bcmVlanLookupIPEnable,           "lookupip" },
            { bcmVlanPortUseInnerPri,          "useinnerpri" },
            { bcmVlanPortVerifyOuterTpid,      "verifyoutertpid" },
            { bcmVlanPortOuterTpidSelect,      "outertpidselect"},
            { 0,                               NULL }   /* LAST ENTRY */
        };

        if ((c = ARG_GET(a)) == NULL) {
            BCM_PBMP_ASSIGN(pbm, PBMP_PORT_ALL(unit));
        } else if (parse_pbmp(unit, c, &pbm) < 0) {
            cli_out("%s: Error: unrecognized port bitmap: %s\n",
                    ARG_CMD(a), c);
            return CMD_FAIL;
        }

        BCM_PBMP_AND(pbm, PBMP_PORT_ALL(unit));
        if (BCM_PBMP_IS_NULL(pbm)) {
            cli_out("No ports specified.\n");
            return CMD_OK;
        }

        subcmd = ARG_GET(a);
        value = ARG_GET(a);

        matched = 0;

        PBMP_ITER(pbm, port) {
            cli_out("\nVlan Control on Port=%s\n", SOC_PORT_NAME(unit, port));
        for (i = 0; typenames[i].name != NULL; i++) {
            tname = typenames[i].name;
            if (subcmd == NULL || sal_strcasecmp(subcmd, tname) == 0) {
                matched += 1;
                ttype = typenames[i].type;
                if (value == NULL) {
                        r = bcm_vlan_control_port_get(unit, port,
                                                      ttype, &varg);
                    if (r < 0) {
                            cli_out("%-30s-\t%s\n", tname, bcm_errmsg(r));
                    } else {
                            cli_out("%-30s%d\n", tname, varg);
                        }
                } else {
                    varg = parse_integer(value);
                        r = bcm_vlan_control_port_set(unit, port,
                                                      ttype, varg);
                    if (r < 0) {
                        cli_out("%s\tERROR: %s\n", tname, bcm_errmsg(r));
                    }
                }
            }
        }
        }

        if (matched == 0) {
            cli_out("%s: ERROR: Unknown port control name\n", subcmd);
            return CMD_FAIL;
        }

        return CMD_OK;
    }


    /* Vlan inner tag */
    if (sal_strcasecmp(subcmd, "innertag") == 0) {
        int          priority, cfi;
        int          vid;
        bcm_port_t   port;
        uint16       innertag;
        pbmp_t       pbm;

        if ((c = ARG_GET(a)) == NULL) {
            BCM_PBMP_ASSIGN(pbm, PBMP_PORT_ALL(unit));
        } else if (parse_pbmp(unit, c, &pbm) < 0) {
            cli_out("%s: Error: unrecognized port bitmap: %s\n",
                    ARG_CMD(a), c);
            return CMD_FAIL;
        }

        BCM_PBMP_AND(pbm, PBMP_PORT_ALL(unit));
        if (BCM_PBMP_IS_NULL(pbm)) {
            cli_out("No ports specified.\n");
            return CMD_OK;
        }

        priority  = -1;
        cfi       = -1;
        vid       = -1;
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "Cfi", PQ_INT | PQ_DFL, 0, &cfi, NULL);
        parse_table_add(&pt, "Vlan", PQ_INT | PQ_DFL, 0, &vid, NULL);
        parse_table_add(&pt, "Priority", PQ_INT | PQ_DFL, 0, &priority, NULL);

        if (0 > parse_arg_eq(a, &pt)) {
            cli_out("%s: Error: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
            parse_arg_eq_done(&pt);
            return (CMD_FAIL);
        }

       if (priority == -1 || cfi == -1 || vid == -1) {
            PBMP_ITER(pbm, port) {
                if ((r = bcm_port_vlan_inner_tag_get(unit, port, &innertag))
                                                      < 0) {
                    goto bcm_err;
                }
                cli_out("Port=%s, Priority=%d, CFI=%d, VLAN=%d\n",
                        SOC_PORT_NAME(unit, port),
                        BCM_VLAN_CTRL_PRIO(innertag),
                        BCM_VLAN_CTRL_CFI(innertag),
                        BCM_VLAN_CTRL_ID(innertag));
           }
        } else {
            innertag = BCM_VLAN_CTRL(priority, cfi, vid);

            PBMP_ITER(pbm, port) {
                if ((r = bcm_port_vlan_inner_tag_set(unit, port, innertag))
                             < 0) {
                    goto bcm_err;
                }
            }
        }
        return CMD_OK;
    }

    if(sal_strcasecmp(subcmd, "debug") == 0) {
        cmd_result_t  cmd_sbx_vlan_debug(int unit, args_t *a);
        return cmd_sbx_vlan_debug(unit, a);
    }

    /* Set per Vlan property (Must be the last)*/
    {
        bcm_vlan_control_vlan_t vlan_control, default_control;
        int outer_tpid, learn_disable, unknown_ip6_mcast_to_cpu;
        int def_outer_tpid, def_learn_disable, def_unknown_ip6_mcast_to_cpu;
        int unknown_ip4_mcast_to_cpu, def_unknown_ip4_mcast_to_cpu;
        int def_mpls_disable, mpls_disable;

        id = parse_integer(subcmd);


        if (VLAN_ID_VALID(id)) {
            r = bcm_vlan_control_vlan_get(unit, id, &default_control);
            if (r < 0) {
                goto bcm_err;
            }

            sal_memcpy(&vlan_control, &default_control, sizeof(vlan_control));

            def_outer_tpid    = default_control.outer_tpid;
            outer_tpid        = def_outer_tpid;

            def_learn_disable = (default_control.flags &
                                BCM_VLAN_LEARN_DISABLE) ? 1 : 0;
            learn_disable     = def_learn_disable;

            def_unknown_ip6_mcast_to_cpu = (default_control.flags &
                                      BCM_VLAN_UNKNOWN_IP6_MCAST_TOCPU) ? 1 : 0;
            unknown_ip6_mcast_to_cpu     =  def_unknown_ip6_mcast_to_cpu;

            def_unknown_ip4_mcast_to_cpu = (default_control.flags &
                                      BCM_VLAN_UNKNOWN_IP4_MCAST_TOCPU) ? 1 : 0;
            unknown_ip4_mcast_to_cpu     = def_unknown_ip4_mcast_to_cpu;

            def_mpls_disable = (default_control.flags &
                                BCM_VLAN_MPLS_DISABLE) ? 1 : 0;
            mpls_disable     = def_mpls_disable;

            parse_table_init(unit, &pt);
            parse_table_add(&pt, "VRF", PQ_INT | PQ_DFL, &default_control.vrf,
                            &vlan_control.vrf, NULL);
            parse_table_add(&pt, "OuterTPID", PQ_HEX | PQ_DFL, &def_outer_tpid,
                            &outer_tpid, NULL);
            parse_table_add(&pt, "LearnDisable", PQ_INT | PQ_DFL,
                            &def_learn_disable, &learn_disable, NULL);
            parse_table_add(&pt, "UnknownIp6McastToCpu", PQ_INT | PQ_DFL,
                            &def_unknown_ip6_mcast_to_cpu,
                            &unknown_ip6_mcast_to_cpu, NULL);
            parse_table_add(&pt, "UnknownIp4McastToCpu", PQ_INT | PQ_DFL,
                            &def_unknown_ip4_mcast_to_cpu,
                            &unknown_ip4_mcast_to_cpu, NULL);
            parse_table_add(&pt, "MplsDisable", PQ_INT | PQ_DFL,
                            &def_mpls_disable, &mpls_disable, NULL);
            parse_table_add(&pt, "Ip6McastFloodMode", PQ_MULTI | PQ_DFL,
                            &default_control.ip6_mcast_flood_mode,
                            &vlan_control.ip6_mcast_flood_mode,
                            bcm_vlan_mcast_flood_str);
            parse_table_add(&pt, "Ip4McastFloodMode", PQ_MULTI | PQ_DFL,
                            &default_control.ip4_mcast_flood_mode,
                            &vlan_control.ip4_mcast_flood_mode,
                            bcm_vlan_mcast_flood_str);
            parse_table_add(&pt, "L2McastFloodMode", PQ_MULTI | PQ_DFL,
                            &default_control.l2_mcast_flood_mode,
                            &vlan_control.l2_mcast_flood_mode,
                            bcm_vlan_mcast_flood_str);

            if (!parseEndOk( a, &pt, &retCode)) {
                return retCode;
            }

            vlan_control.outer_tpid     = (uint16) outer_tpid;
            vlan_control.flags = (learn_disable ?
                                          BCM_VLAN_LEARN_DISABLE : 0);
            vlan_control.flags |= (unknown_ip6_mcast_to_cpu ?
                                          BCM_VLAN_UNKNOWN_IP6_MCAST_TOCPU : 0);
            vlan_control.flags |= (unknown_ip4_mcast_to_cpu?
                                          BCM_VLAN_UNKNOWN_IP4_MCAST_TOCPU : 0);
            vlan_control.flags |= (mpls_disable ?
                                          BCM_VLAN_MPLS_DISABLE : 0);

            if ((r = bcm_vlan_control_vlan_set(unit, id, vlan_control)) < 0) {
                goto bcm_err;
            }

            return CMD_OK;
        }
    }

    return CMD_USAGE;

 bcm_err:

    cli_out("%s: ERROR: %s\n", ARG_CMD(a), bcm_errmsg(r));

    return CMD_FAIL;
}

cmd_result_t
cmd_sbx_vlan_debug(int unit, args_t *a)
{
    char                    *c;
    int                     result;
    unsigned int            index;
    bcm_vlan_data_t         *vlanList;
    int                     vlanCount;
    int                     priority;
    int                     data;
    bcm_port_t              port;
    bcm_pbmp_t              pbmp;
    bcm_pbmp_t              ubmp;
    bcm_vlan_t              vid;
    bcm_vlan_t              hvid;
    bcm_vlan_t              dvid;
    bcm_stg_t               stg;
    bcm_vlan_mcast_flood_t  floodMode;
    bcm_vlan_control_vlan_t vlanControl;
    bcm_vlan_control_port_t portControl;

    _GET_NONBLANK(a,c);
    if (!sal_strncasecmp(c,"init",strlen(c))) {
        result = bcm_vlan_init(unit);
        cli_out("bcm_vlan_init(%d) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
    } else if (!sal_strncasecmp(c,"list",strlen(c))) {
        c = ARG_GET(a);
        if (NULL == c) {
            /* list all current VIDs */
            result = bcm_vlan_list(unit,&vlanList,&vlanCount);
            cli_out("bcm_vlan_list(%d,*,*) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
            cli_out("bcm_vlan_list indicates %d VLANs are configured\n",vlanCount);
        } else {
            /* list only vids with certain ports */
            BCM_PBMP_CLEAR(pbmp);
            _SHR_PBMP_WORD_SET(pbmp,0,fromAsciiHexQuadByte(c));
            result = bcm_vlan_list_by_pbmp(unit,pbmp,&vlanList,&vlanCount);
            cli_out("bcm_vlan_list_by_pbmp(%d,*,*) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
            cli_out("bcm_vlan_list_by_pbmp indicates %d VLANs are configured and match\n",vlanCount);
        }
        if (BCM_E_NONE == result) {
            for (index = 0; index < vlanCount; index++) {
                cli_out("  %4d - VLAN %03X : pbmp=%08X ubmp=%08X\n",
                        index,
                        vlanList[index].vlan_tag,
                        _SHR_PBMP_WORD_GET(vlanList[index].port_bitmap,0),
                        _SHR_PBMP_WORD_GET(vlanList[index].ut_port_bitmap,0)
                        );
            }
        }
        if (vlanList) {
            bcm_vlan_list_destroy(unit,vlanList,vlanCount);
        }
    } else if (!sal_strncasecmp(c,"create",_max(strlen(c),2))) {
        _GET_NONBLANK(a,c);
        vid = fromAsciiHexQuadByte(c) & 0xFFF;
        result = bcm_vlan_create(unit,vid);
        cli_out("bcm_vlan_create(%d,%03X) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
    } else if (!sal_strncasecmp(c,"destroy",_max(strlen(c),3))) {
        _GET_NONBLANK(a,c);
        if (!sal_strcasecmp(c,"all")) {
            result = bcm_vlan_destroy_all(unit);
            cli_out("bcm_vlan_destroy_all(%d) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
        } else {
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            result = bcm_vlan_destroy(unit,vid);
            cli_out("bcm_vlan_destroy(%d,%03X) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
        }
    } else if (!sal_strncasecmp(c,"port",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"add",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(pbmp);
            _SHR_PBMP_WORD_SET(pbmp,0,fromAsciiHexQuadByte(c));
            _GET_NONBLANK(a,c);
            BCM_PBMP_CLEAR(ubmp);
            _SHR_PBMP_WORD_SET(ubmp,0,fromAsciiHexQuadByte(c));
            result = bcm_vlan_port_add(unit,vid,pbmp,ubmp);
            cli_out("bcm_vlan_port_add(%d,%03X,%08X,%08X) returned %d (%s)\n",unit,vid,_SHR_PBMP_WORD_GET(pbmp,0),_SHR_PBMP_WORD_GET(ubmp,0),result,_SHR_ERRMSG(result));
        } else if (!sal_strncasecmp(c,"rem",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            _SHR_PBMP_WORD_SET(pbmp,0,fromAsciiHexQuadByte(c));
            result = bcm_vlan_port_remove(unit,vid,pbmp);
            cli_out("bcm_vlan_port_remove(%d,%03X,%08X) returned %d (%s)\n",unit,vid,_SHR_PBMP_WORD_GET(pbmp,0),result,_SHR_ERRMSG(result));
        } else if (!sal_strncasecmp(c,"get",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            result = bcm_vlan_port_get(unit,vid,&pbmp,&ubmp);
            cli_out("bcm_vlan_port_get(%d,%03X,*,*) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
            cli_out("bcm_vlan_port_get indicates ports %08X were on the VLAN\n",_SHR_PBMP_WORD_GET(pbmp,0));
            cli_out("bcm_vlan_port_get indicates ports %08X were untagged\n",_SHR_PBMP_WORD_GET(ubmp,0));
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"default",_max(strlen(c),3))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"get",strlen(c))) {
            result = bcm_vlan_default_get(unit,&vid);
            cli_out("bcm_vlan_default_get(%d,*) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
            cli_out("bcm_vlan_default_get indicates default VID is %03X\n",vid);
        } else if (!sal_strncasecmp(c,"set",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            result = bcm_vlan_default_set(unit,vid);
            cli_out("bcm_vlan_default_set(%d,%03X) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strcasecmp(c,"stg")) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"get",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            result = bcm_vlan_stg_get(unit,vid,&stg);
            cli_out("bcm_vlan_stg_get(%d,%03X,*) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
            cli_out("bcm_vlan_stg_get indicates VLAN %03X is in STG %08X\n",vid,stg);
        } else if (!sal_strncasecmp(c,"set",strlen(c))) {
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            stg = fromAsciiHexQuadByte(c);
            result = bcm_vlan_stg_set(unit,vid,stg);
            cli_out("bcm_vlan_stg_set(%d,%03X,%08X) returned %d (%s)\n",unit,vid,stg,result,_SHR_ERRMSG(result));
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"mcast",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"flood",strlen(c))) {
            _GET_NONBLANK(a,c);
            if (!sal_strncasecmp(c,"get",strlen(c))) {
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c) & 0xFFF;
                result = bcm_vlan_mcast_flood_get(unit,vid,&floodMode);
                cli_out("bcm_vlan_mcast_flood_get(%d,%03X,*) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
                cli_out("bcm_vlan_mcast_flood_get indicates multicast flood mode is %08X (%s)\n",floodMode,_bcm_vlan_mcast_flood_str[floodMode]);
            } else if (!sal_strncasecmp(c,"set",strlen(c))) {
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                floodMode = fromAsciiHexQuadByte(c);
                result = bcm_vlan_mcast_flood_set(unit,vid,floodMode);
                cli_out("bcm_vlan_mcast_flood_get(%d,%03X,%08X) returned %d (%s)\n",unit,vid,floodMode,result,_SHR_ERRMSG(result));
            } else {
                return CMD_USAGE;
            }
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"control",_max(strlen(c),2))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"vlan",strlen(c))) {
            _GET_NONBLANK(a,c);
            if (!sal_strncasecmp(c,"get",strlen(c))) {
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c) & 0xFFF;
                result = bcm_vlan_control_vlan_get(unit,vid,&vlanControl);
                cli_out("bcm_vlan_control_vlan_get(%d,%03X,*) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
                cli_out("  bcm_vlan_control_vlan_t.vrf                  = %08X\n",vlanControl.vrf);
                cli_out("  bcm_vlan_control_vlan_t.forwarding_vlan      = %08X\n",vlanControl.forwarding_vlan);
                cli_out("  bcm_vlan_control_vlan_t.outer_tpid           = %08X\n",vlanControl.outer_tpid);
                cli_out("  bcm_vlan_control_vlan_t.flags                = %08X\n",vlanControl.flags);
                cli_out("  bcm_vlan_control_vlan_t.ip6_mcast_flood_mode = %08X\n",vlanControl.ip6_mcast_flood_mode);
                cli_out("  bcm_vlan_control_vlan_t.ip4_mcast_flood_mode = %08X\n",vlanControl.ip4_mcast_flood_mode);
                cli_out("  bcm_vlan_control_vlan_t.l2_mcast_flood_mode  = %08X\n",vlanControl.l2_mcast_flood_mode);
            } else if (!sal_strncasecmp(c,"set",strlen(c))) {
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c) & 0xFFF;
                _GET_NONBLANK(a,c);
                vlanControl.vrf = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vlanControl.forwarding_vlan = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vlanControl.outer_tpid = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vlanControl.flags = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vlanControl.ip6_mcast_flood_mode = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vlanControl.ip4_mcast_flood_mode = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vlanControl.l2_mcast_flood_mode = fromAsciiHexQuadByte(c);
                /* coverity[uninit_use_in_call] */
                result = bcm_vlan_control_vlan_set(unit,vid,vlanControl);
                cli_out("bcm_vlan_control_vlan_set(%d,%03X,*) returned %d (%s)\n",unit,vid,result,_SHR_ERRMSG(result));
            } else {
                return CMD_USAGE;
            }
        } else if (!sal_strncasecmp(c,"port",strlen(c))) {
            _GET_NONBLANK(a,c);
            if (!sal_strncasecmp(c,"get",strlen(c))) {
                _GET_NONBLANK(a,c);
                port = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                portControl = fromAsciiHexQuadByte(c);
                result = bcm_vlan_control_port_get(unit,port,portControl,&data);
                cli_out("bcm_vlan_control_port_get(%d,%d,%08X,*) returned %d (%s)\n",unit,port,portControl,result,_SHR_ERRMSG(result));
                cli_out("bcm_vlan_control_port_get indicated a value of %08X\n",data);
            } else if (!sal_strncasecmp(c,"set",strlen(c))) {
                _GET_NONBLANK(a,c);
                port = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                portControl = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                data = fromAsciiHexQuadByte(c);
                result = bcm_vlan_control_port_set(unit,port,portControl,data);
                cli_out("bcm_vlan_control_port_set(%d,%d,%08X,%08X) returned %d (%s)\n",unit,port,portControl,data,result,_SHR_ERRMSG(result));
            } else {
                return CMD_USAGE;
            }
        } else {
            return CMD_USAGE;
        }
    } else if (!sal_strncasecmp(c,"translate",strlen(c))) {
        _GET_NONBLANK(a,c);
        if (!sal_strncasecmp(c,"add",strlen(c))) {
            _GET_NONBLANK(a,c);
            port = fromAsciiHexQuadByte(c);
            _GET_NONBLANK(a,c);
            vid = fromAsciiHexQuadByte(c) & 0xFFF;
            _GET_NONBLANK(a,c);
            dvid = fromAsciiHexQuadByte(c) & 0xFFF;
            c = ARG_GET(a);
            if (NULL == c) {
                priority = -1;
            } else {
                priority = fromAsciiHexQuadByte(c);
            }
            result = bcm_vlan_translate_add(unit,port,vid,dvid,priority);
            cli_out("bcm_vlan_translate_add(%d,%d,%03X,%03X,%08X) returned %d (%s)\n",unit,port,vid,dvid,priority,result,_SHR_ERRMSG(result));
        } else if (!sal_strncasecmp(c,"del",strlen(c))) {
            _GET_NONBLANK(a,c);
            if (!sal_strcasecmp(c,"all")) {
                result = bcm_vlan_translate_delete_all(unit);
                cli_out("bcm_vlan_translate_delete_all(%d) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
            } else {
                port = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c) & 0xFFF;
                result = bcm_vlan_translate_delete(unit,port,vid);
                cli_out("bcm_vlan_translate_delete(%d,%d,%03X) returned %d (%s)\n",unit,port,vid,result,_SHR_ERRMSG(result));
            }
        } else if (!sal_strncasecmp(c,"range",strlen(c))) {
            _GET_NONBLANK(a,c);
            if (!sal_strncasecmp(c,"add",strlen(c))) {
                _GET_NONBLANK(a,c);
                port = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c) & 0xFFF;
                _GET_NONBLANK(a,c);
                hvid = fromAsciiHexQuadByte(c) & 0xFFF;
                _GET_NONBLANK(a,c);
                dvid = fromAsciiHexQuadByte(c) & 0xFFF;
                c = ARG_GET(a);
                if (NULL == c) {
                    priority = -1;
                } else {
                    priority = fromAsciiHexQuadByte(c);
                }
                result = bcm_vlan_translate_range_add(unit,port,vid,hvid,dvid,priority);
                cli_out("bcm_vlan_translate_range_add(%d,%d,%03X,%03X,%03X,%08X) returned %d (%s)\n",unit,port,vid,hvid,dvid,priority,result,_SHR_ERRMSG(result));
            } else if (!sal_strncasecmp(c,"del",strlen(c))) {
                c = ARG_GET(a);
                if (!sal_strcasecmp(c,"all")) {
                    result = bcm_vlan_translate_range_delete_all(unit);
                    cli_out("bcm_vlan_translate_range_delete_all(%d) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
                } else {
                    port = fromAsciiHexQuadByte(c);
                    _GET_NONBLANK(a,c);
                    vid = fromAsciiHexQuadByte(c) & 0xFFF;
                    _GET_NONBLANK(a,c);
                    hvid = fromAsciiHexQuadByte(c) & 0xFFF;
                    result = bcm_vlan_translate_range_delete(unit,port,vid,hvid);
                    cli_out("bcm_vlan_translate_range_delete(%d,%d,%03X,%03X) returned %d (%s)\n",unit,port,vid,hvid,result,_SHR_ERRMSG(result));
                }
            } else {
                return CMD_USAGE;
            }
        } else if (!sal_strncasecmp(c,"egress",strlen(c))) {
            _GET_NONBLANK(a,c);
            if (!sal_strncasecmp(c,"add",strlen(c))) {
                _GET_NONBLANK(a,c);
                port = fromAsciiHexQuadByte(c);
                _GET_NONBLANK(a,c);
                vid = fromAsciiHexQuadByte(c) & 0xFFF;
                _GET_NONBLANK(a,c);
                dvid = fromAsciiHexQuadByte(c) & 0xFFF;
                c = ARG_GET(a);
                if (NULL == c) {
                    priority = -1;
                } else {
                    priority = fromAsciiHexQuadByte(c);
                }
                result = bcm_vlan_translate_egress_add(unit,port,vid,dvid,priority);
                cli_out("bcm_vlan_translate_egress_add(%d,%d,%03X,%03X,%08X) returned %d (%s)\n",unit,port,vid,dvid,priority,result,_SHR_ERRMSG(result));
            } else if (!sal_strncasecmp(c,"del",strlen(c))) {
                _GET_NONBLANK(a,c);
                if (!sal_strcasecmp(c,"all")) {
                    result = bcm_vlan_translate_egress_delete_all(unit);
                    cli_out("bcm_vlan_translate_egress_delete_all(%d) returned %d (%s)\n",unit,result,_SHR_ERRMSG(result));
                } else {
                    port = fromAsciiHexQuadByte(c);
                    _GET_NONBLANK(a,c);
                    vid = fromAsciiHexQuadByte(c) & 0xFFF;
                    result = bcm_vlan_translate_egress_delete(unit,port,vid);
                    cli_out("bcm_vlan_translate_egress_delete(%d,%d,%03X) returned %d (%s)\n",unit,port,vid,result,_SHR_ERRMSG(result));
                }
            } else {
                return CMD_USAGE;
            }
        } else {
            return CMD_USAGE;
        }
    } else {
        return CMD_USAGE;
    }

    return(CMD_OK);
}

char cmd_sbx_pvlan_usage[] =
    "Usages:\n\t"
    "  pvlan show <pbmp>\n\t"
    "        - Show PVLAN info for these ports.\n\t"
    "  pvlan set <pbmp> <vid>\n\t"
    "        - Set default VLAN tag for port(s)\n\t"
    "          Port bitmaps are read from the VTABLE entry for the VID.\n\t"
    "          <vid> must have been created and all ports in <pbmp> must\n\t"
    "          belong to that VLAN.\n";
cmd_result_t
cmd_sbx_pvlan(int u, args_t *a)
{
    char *subcmd, *argpbm, *argvid;
    vlan_id_t vid = BCM_VLAN_INVALID;
    soc_port_t port;
    int rv;
    pbmp_t pbm;

    if (!sh_check_attached(ARG_CMD(a), u)) {
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(a)) == NULL) {
        subcmd = "show";
    }

    if ((argpbm = ARG_GET(a)) == NULL) {
        pbm = PBMP_PORT_ALL(u);
    } else {
        if (parse_pbmp(u, argpbm, &pbm) < 0) {
            cli_out("%s: ERROR: unrecognized port bitmap: %s\n",
                    ARG_CMD(a), argpbm);
            return CMD_FAIL;
        }
    }

    if (sal_strcasecmp(subcmd, "show") == 0) {
        rv = BCM_E_NONE;

        PBMP_ITER(pbm, port) {
            if ((rv = bcm_port_untagged_vlan_get(u, port, &vid)) < 0) {
                cli_out("Error retrieving info for port %s: %s\n",
                        SOC_PORT_NAME(u, port), bcm_errmsg(rv));
                break;
            }
            cli_out("Port %s default VLAN is %d\n",
                    SOC_PORT_NAME(u, port), vid);
        }
        return (rv < 0) ? CMD_FAIL : CMD_OK;
    } else if (sal_strcasecmp(subcmd, "set") == 0) {
        if ((argvid = ARG_GET(a)) == NULL) {
            cli_out("Missing VID for set.\n");
            return CMD_USAGE;
        }

        vid = sal_ctoi(argvid, 0);

        /* Set default VLAN as indicated */
        rv = BCM_E_NONE;

        PBMP_ITER(pbm, port) {
            if ((rv = bcm_port_untagged_vlan_set(u, port, vid)) < 0) {
                cli_out("Error setting port %s default VLAN to %d: %s\n",
                        SOC_PORT_NAME(u, port), vid, bcm_errmsg(rv));

                if ((rv == BCM_E_NOT_FOUND) ||
                    (rv == BCM_E_CONFIG)) {
                    cli_out("VLAN %d must be created and contain the ports "
                            "before being used for port default VLAN.\n", vid);
                }

                break;
            }
        }
    }
    return CMD_OK;
}


#if (defined(BCM_BME3200_SUPPORT) || defined(BCM_BM9600_SUPPORT))

char cmd_sbx_xb_test_cnt_usage[] = "\n"
"xbtestcnt <direction> <xb port> <clear>\n"
"  Applicable on BME only.  Displays the count of timeslot headers with the\n"
"  test bit set on the given node in the given direction\n"
"  direction - EGress or INgress\n"
"  xb port   - physical port on BME for test packet count\n"
"  clear     - clear entry after reading\n";

cmd_result_t
cmd_sbx_xb_test_cnt(int unit, args_t *args)
{
    int32 nEgress=-1, nXbPort=-1, nCount;
    char* pArg;
    soc_error_t socErr = SOC_E_NONE;
    int32 nClear = 0;

    if (!SOC_IS_SBX_BME3200(unit) && !SOC_IS_SBX_BM9600(unit)) {
        cli_out("Invalid unit type for command xbtestcnt - must be BM3200/BM9600 unit(%d)\n", unit);
        return CMD_USAGE;
    }

    while ((pArg = ARG_GET(args))) {

        if (sal_strcmp(pArg, "eg") == 0 ||
            sal_strcmp(pArg, "egress") == 0)
        {
            nEgress = 1;
        }
        else if (sal_strcmp(pArg, "in") == 0 ||
                 sal_strcmp(pArg, "ingress") == 0)
        {
            nEgress = 0;
        }
        else if (sal_strcmp(pArg, "clear") == 0)
        {
            nClear = 1;
        }
        else {
            nXbPort = sal_ctoi(pArg, 0);
        }
    };

    if ((nEgress < 0) || (nXbPort < 0)) {
        return CMD_USAGE;
    }

    if (SOC_IS_SBX_BME3200(unit)) {
#ifdef BCM_BME3200_SUPPORT
        socErr = soc_bm3200_xb_test_pkt_get(unit, nEgress, nXbPort, &nCount);
#endif
    }
    else { /* BM9600 */
#ifdef BCM_BM9600_SUPPORT
      socErr = soc_bm9600_xb_test_pkt_get(unit, nEgress, nXbPort, &nCount);
#endif
    }
    if (SOC_SUCCESS(socErr)) {
        cli_out("test packet count=%d\n", nCount);
    } else {
        cli_out("failed to get test pkt count on xb port %d: err=%d\n",
                nXbPort, socErr);
        return CMD_FAIL;
    }

    if (nClear) {
	if (SOC_IS_SBX_BME3200(unit)) {
#ifdef BCM_BME3200_SUPPORT
	    socErr = soc_bm3200_xb_test_pkt_clear(unit, nEgress, nXbPort);
#endif
	}
	else { /* BM9600 */
#ifdef BCM_BM9600_SUPPORT
	    socErr = soc_bm9600_xb_test_pkt_clear(unit, nEgress, nXbPort);
#endif
	}
	if (SOC_SUCCESS(socErr)) {
	    return CMD_OK;
	} else {
	    cli_out("failed to clear pkt count on xb port %d: err=%d\n",
                    nXbPort, socErr);
	    return CMD_FAIL;
	}
    }
    return CMD_OK;
}

#endif /* BCM_BME3200_SUPPORT || BCM_BM9600_SUPPORT */

#if defined(BCM_QE2000_SUPPORT) || defined(BCM_FE2000_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
char cmd_sbx_qinfo_get_usage[] = "\n"
"queueinfo <queue number>\n";

cmd_result_t
cmd_sbx_qinfo_get(int unit, args_t *args)
{
    char *pQuNum;
    int nQueueNum;
    int nStatus = CMD_USAGE;

    if (!SOC_SBX_INIT(unit)) {
        cli_out("unit %d, not initialized - call 'init soc' first!\n",
                unit);
    }

    if(!(pQuNum = ARG_GET(args))) {
        return CMD_USAGE;
    }
    nQueueNum = sal_ctoi(pQuNum, 0);

    if (0) {
#ifdef BCM_QE2000_SUPPORT
    } else if (SOC_IS_SBX_QE2000(unit)) {
        nStatus = cmd_sbx_qe2000_print_queue_info(unit, nQueueNum);
#endif
#ifdef BCM_CALADAN3_SUPPORT
    } else if (SOC_IS_SBX_CALADAN3(unit)) {
        cmd_sbx_caladan3_print_queue_info(unit, nQueueNum);
        nStatus = CMD_OK;
#endif
#ifdef BCM_SIRIUS_SUPPORT
    } else if (SOC_IS_SIRIUS(unit)) {
        nStatus = cmd_sbx_sirius_print_queue_info(unit, nQueueNum);
#endif
    }

    return nStatus;
}
#endif


#if defined(BCM_BM9600_SUPPORT)
char cmd_sbx_sot_policing_usage[] = "\n"
"SotPolicing\n";

cmd_result_t
cmd_sbx_sot_policing(int unit, args_t *args)
{

    int nStatus = CMD_USAGE;
    uint32 xb_iport_channel_ptr[6];
    uint32 i, j;
#ifdef DEBUG_SDK30912
    int port;
#endif
    int rv;

    if (!SOC_SBX_INIT(unit)) {
        cli_out("unit %d, not initialized - call 'init soc' first!\n",
                unit);
	return nStatus;
    }

    rv = soc_bm9600_port_check_policing_errors_reset_xb_port(unit, xb_iport_channel_ptr);

    if (rv == SOC_E_NONE) {
	for (j=0; j<6; j++) {
	    for (i=0;i<16; i++) {
#ifdef DEBUG_SDK30912
		if ( (xb_iport_channel_ptr[j]) & (0x3 << (2*i)) ) {
		    port = i + (j *16);
		    cli_out("port(%d) xb error, port was disabled and re-enabled\n", port);
		}
#endif
	    }
	    cli_out("iport_channel_mask[%d]=0x%08x\n", j, xb_iport_channel_ptr[j]); 
	}

	nStatus = CMD_OK;
    } else {
	cli_out("Policing error check failed (%d)\n", rv);
	nStatus = CMD_FAIL;
    }

    return nStatus;
}
#endif


#ifdef BCM_FE2000_SUPPORT
char cmd_sbx_stg_usage[] = "\n"
                 "\t\tstg  clear \n"
                 "\t\tstg  count    get \n"
                 "\t\tstg  create \n"
                 "\t\tstg  create   id   <stg>\n"
                 "\t\tstg  default  get \n"
                 "\t\tstg  default  set  <stg>\n"
                 "\t\tstg  destroy  <stg>\n"
                 "\t\tstg  init \n"
                 "\t\tstg  list \n"
                 "\t\tstg  list     destroy \n"
                 "\t\tstg  stp      get  <stg>  <port>\n"
                 "\t\tstg  stp      set  <stg>  <port>  <state>\n"
                 "\t\tstg  stp      <stg> all <state>\n" 
                 "\t\tstg  vlan     add  <stg>  <vid>\n"
                 "\t\tstg  vlan     list <stg>\n"
                 "\t\tstg  vlan     list  destroy \n"
                 "\t\tstg  vlan     remove  <stg>  <vid>\n"
                 "\t\tstg  vlan     remove  all    <stg>\n"
;


STATIC int
sbx_show_stg_stp(int unit, bcm_stg_t stg)
{
    bcm_pbmp_t          pbmps[BCM_STG_STP_COUNT];
    char                buf[FORMAT_PBMP_MAX];
    int                 state, r;
    soc_port_t          port;
    bcm_port_config_t   pcfg;

    sal_memset(pbmps, 0, sizeof (pbmps));

    BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));

    BCM_PBMP_ITER(pcfg.port, port) {
        if ((r = bcm_stg_stp_get(unit, stg, port, &state)) < 0) {
            return r;
        }

        BCM_PBMP_PORT_ADD(pbmps[state], port);
    }

    /* In current chips, LISTEN is not distinguished from BLOCK. */

    for (state = 0; state < BCM_STG_STP_COUNT; state++) {
        if (!(BCM_PBMP_IS_NULL(pbmps[state]))) {
            format_bcm_pbmp(unit, buf, sizeof (buf), pbmps[state]);
            cli_out("  %7s: %s\n", FORWARD_MODE(state), buf);
        }
    }

    return BCM_E_NONE;
}

cmd_result_t
cmd_sbx_stg (int unit, args_t *args)
{
    char       *c, *get_or_set;
    bcm_stg_t   stg = 0;
    bcm_port_t  port = 0;
    bcm_vlan_t  vid = 0;
    int         state;
    int         rv = BCM_E_UNAVAIL;
    static bcm_stg_t  *list = NULL;
    static bcm_vlan_t *vlist = NULL;
    static int         count = 0;
    pbmp_t              pbmp;
    bcm_port_config_t   pcfg;

    if (!SOC_IS_SBX_FE(unit)) {
        return CMD_OK;
    }

    if (!(c = ARG_GET(args))) {     /* Nothing to do */
        return(CMD_USAGE);      /* Print usage line */
    }

    /* bcm_stg_clear() */
    if (!sal_strcasecmp(c, "clear")) {
        rv = bcm_stg_clear(unit);
    }

    /* bcm_stg_count_get() */
    else if (!sal_strcasecmp(c, "count")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "get")) {
            rv = bcm_stg_count_get(unit, &stg);
            if (rv == BCM_E_NONE) {
                cli_out("STG: %d Spanning Tree Groups available.\n", stg);
                return CMD_OK;
            }
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_stg_create() */
    /* bcm_stg_create_id() */
    else if (!sal_strcasecmp(c, "create")) {
        if ((c = ARG_GET(args)) == NULL) {
            rv = bcm_stg_create(unit, &stg);
            if (rv == BCM_E_NONE) {
                cli_out("STG: Created Group = %d.\n", stg);
                return CMD_OK;
            }
        }
        else if (!sal_strcasecmp(c, "id")) {
            if (!(c = ARG_GET(args)))
                return(CMD_USAGE);
            else {
                stg = sal_ctoi(c, 0);
                rv = bcm_stg_create_id(unit, stg);
            }
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_stg_default_get() */
    /* bcm_stg_default_set() */
    else if (!sal_strcasecmp(c, "default")) {
        if ((c = ARG_GET(args)) == NULL) {
            return(CMD_USAGE);
        }
        else if (!sal_strcasecmp(c, "get")) {
            rv = bcm_stg_default_get(unit, &stg);
            if (rv == BCM_E_NONE) {
                cli_out("STG: Default Spanning Tree Group = %d.\n", stg);
                return CMD_OK;
            }
        }
        else if (!sal_strcasecmp(c, "set")) {
            if ((c = ARG_GET(args)) == NULL) {
                return(CMD_USAGE);
            }
            else {
                stg = sal_ctoi(c, 0);
                rv = bcm_stg_default_set(unit, stg);
            }
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_stg_destroy() */
    else if (!sal_strcasecmp(c, "destroy")) {
        if ((c = ARG_GET(args)) != NULL) {
            stg = sal_ctoi(c, 0);
            rv = bcm_stg_destroy(unit, stg);
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_stg_init() */
    else if (!sal_strcasecmp(c, "init")) {
        rv = bcm_stg_init(unit);
    }

    /* bcm_stg_list() */
    /* bcm_stg_list_destroy() */
    else if (!sal_strcasecmp(c, "list")) {
        if ((c = ARG_GET(args)) == NULL) {
            rv = bcm_stg_list(unit, &list, &count);
            if (rv == BCM_E_NONE) {
                cli_out("STG: List Count = %d.\n", count);
                for (stg = 0; stg < count; stg++) {
                    cli_out("\t%d\n", list[stg]);
                }
                return CMD_OK;
            }
        }
        else if (!sal_strcasecmp(c, "destroy"))
            rv = bcm_stg_list_destroy(unit, list, count);
        else
            return(CMD_USAGE);
    }

    /* bcm_stg_stp_get() */
    /* bcm_stg_stp_set() */
    else if (!sal_strcasecmp(c, "stp")) {
        /* this is a get/set with user params. prefectch args */
        if ((get_or_set = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        } 

        if (!sal_strcasecmp(get_or_set, "get")) {
            if ((c = ARG_GET(args)) == NULL)
                return CMD_USAGE;
            stg = sal_ctoi(c, 0);
            if ((c = ARG_GET(args)) == NULL)
                 return CMD_USAGE;
            port = sal_ctoi(c, 0);
            rv = bcm_stg_stp_get(unit, stg, port, &state);
            if (rv == BCM_E_NONE) {
                cli_out("STG: STP Get stg/port = %d/%d;  state = %d.\n", stg, port, state);
                return CMD_OK;
            }
        }
        else if (!sal_strcasecmp(get_or_set, "set")) {
            if ((c = ARG_GET(args)) == NULL)
                 return CMD_USAGE;
            stg = sal_ctoi(c, 0);
            if ((c = ARG_GET(args)) == NULL)
                 return CMD_USAGE;
            port = sal_ctoi(c, 0);
            if ((c = ARG_GET(args)) == NULL) {
                return CMD_USAGE;
            } else {
                state = sal_ctoi(c, 0);
                rv = bcm_stg_stp_set(unit, stg, port, state);
            }
        } else {
            stg = parse_integer(get_or_set);

            if ((c = ARG_GET(args)) == NULL) {
                 cli_out("STG %d:\n", stg);

                 if ((rv = sbx_show_stg_stp(unit, stg)) < 0) {
                     return CMD_FAIL;
                 }

                 return CMD_OK;
            }

            if (parse_bcm_pbmp(unit, c, &pbmp) < 0) {
                return CMD_USAGE;
            }

            if ((c = ARG_GET(args)) == NULL) {
                 return CMD_USAGE;
            }

            for (state = 0; state < BCM_STG_STP_COUNT; state++) {
                 if (parse_cmp(forward_mode[state], c, '\0')) {
                     break;
                 }
            }

            if (state == BCM_STG_STP_COUNT) {
                return CMD_USAGE;
            }

            BCM_IF_ERROR_RETURN(bcm_port_config_get(unit, &pcfg));

            BCM_PBMP_AND(pbmp, pcfg.port);

            BCM_PBMP_ITER(pbmp, port) {
               if ((rv = bcm_stg_stp_set(unit, stg, port, state)) < 0) {
                   return CMD_FAIL;
               }
            }

            return CMD_OK;
       }
    }

    /* bcm_stg_vlan_add() */
    /* bcm_stg_vlan_list() */
    /* bcm_stg_vlan_list_destroy() */
    /* bcm_stg_vlan_remove() */
    /* bcm_stg_vlan_remove_all() */
    else if (!sal_strcasecmp(c, "vlan")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        if (!sal_strcasecmp(c, "add")) {
            if ((c = ARG_GET(args)) == NULL)
                return CMD_USAGE;
            stg = sal_ctoi(c, 0);
            if ((c = ARG_GET(args)) == NULL)
                return CMD_USAGE;
            vid = sal_ctoi(c, 0);
            rv = bcm_stg_vlan_add(unit, stg, vid);
        }
        else if (!sal_strcasecmp(c, "list")) {
            if ((c = ARG_GET(args)) == NULL)
                return CMD_USAGE;
            if (!sal_strcasecmp(c, "destroy")) {
                rv = bcm_stg_vlan_list_destroy(unit, vlist, count);
            } else {
                stg = sal_ctoi(c, 0);
                rv = bcm_stg_vlan_list(unit, stg, &vlist, &count);
                if (rv == BCM_E_NONE) {
                    cli_out("STG: Vlan List Count = %d.\n", count);
                    for (vid = 0; vid < count; vid++)
                        cli_out("\t%d\n", vlist[vid]);
                    return CMD_OK;
                }
            }
        }
        else if (!sal_strcasecmp(c, "remove")) {
            if ((c = ARG_GET(args)) == NULL)
                return CMD_USAGE;
            if (!sal_strcasecmp(c, "all")) {
                if ((c = ARG_GET(args)) == NULL)
                    return CMD_USAGE;
                stg = sal_ctoi(c, 0);
                rv = bcm_stg_vlan_remove_all(unit, stg);
            } else {
                stg = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return CMD_USAGE;
                vid = sal_ctoi(c, 0);
                rv = bcm_stg_vlan_remove(unit, stg, vid);
            }
        }
        else
            return(CMD_USAGE);
    }

    else {
        cli_out("STG: Command %s not supported.\n", c);
        return CMD_OK;
    }

    if (rv == BCM_E_NONE)
        cli_out("STG: OK!\n");
    else
        cli_out("STG: Fail, %s\n", bcm_errmsg(rv));

    return CMD_OK;
}

char cmd_sbx_trunk_usage[] = "\n"
#ifndef COMPILER_STRING_CONST_LIMIT
                 "\t\ttrunk  bitmap expand <hex_pbmp>\n"
                 "\t\ttrunk  chip info get\n"
                 "\t\ttrunk  create\n"
                 "\t\ttrunk  create id <tid>\n"
                 "\t\ttrunk  destroy   <tid>\n"
                 "\t\ttrunk  detach\n"
                 "\t\ttrunk  find <modid> <port>\n"
                 "\t\ttrunk  get <tid>\n"
                 "\t\ttrunk  init\n"
                 "\t\ttrunk  mcast join <tid> <vid> <mac> (i.e. 12:34:ef:8d:00:9c)\n"
                 "\t\ttrunk  port add    <tid> <modid> <port>\n"
                 "\t\ttrunk  port remove <tid> <modid> <port>\n"
                 "\t\ttrunk  port up|down <port>\n"
                 "\t\ttrunk  psc get <tid>\n"
                 "\t\ttrunk  psc set <tid> <psc>\n"
                 "\t\ttrunk  set <tid> ... \n"
                 "\t\ttrunk  vlan port remove <vid> <port> \n"
                 "\t\ttrunk  show [<Id=val>]\n"
                 "\t\ttrunk  debug\n"
                 "\t\ttrunk  unavail\n"
#endif
;

cmd_result_t
cmd_sbx_trunk (int unit, args_t *args)
{
    char           *c;
    bcm_pbmp_t      pbmp;
    int             garbage = 0;
    int             weights[BCM_TRUNK_MAX_PORTCNT];
    bcm_trunk_chip_info_t   ta_info;
    bcm_trunk_t             tid = 0;
    bcm_trunk_info_t        t_data;
    bcm_trunk_member_t      *member_array = NULL;
    int                     member_count, unused_count;
    int             index;
    sal_mac_addr_t  mac;
    uint32          a[6];
    int             psc;
    int             rv = BCM_E_FAIL;
    bcm_module_t    modid = 0;
    bcm_port_t      port = 0;
    bcm_vlan_t      vid = 0;

    BCM_PBMP_CLEAR(pbmp);
    sal_memset((void *)&t_data, 0, sizeof(bcm_trunk_info_t));
    sal_memset((void *)&ta_info, 0, sizeof(bcm_trunk_chip_info_t));
    sal_memset((void *)weights, 0, sizeof(weights));
    sal_memset((void *)mac, 0, sizeof(mac));

    if (!(c = ARG_GET(args))) {     /* Nothing to do */
        return(CMD_USAGE);          /* Print usage line */
    }

    /* bcm_trunk_bitmap_expand() */
    else if (!sal_strcasecmp(c, "bitmap")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "expand")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else {
                garbage = fromAsciiHexQuadByte(c);
                index = 0;
                while (garbage > 0) {
                    if (garbage & 0x1)
                        BCM_PBMP_PORT_ADD(pbmp, index);
                    index++;
                    garbage >>= 1;
                }
                rv = bcm_trunk_bitmap_expand(unit, &pbmp);
                if (rv == BCM_E_NONE) {
                    garbage = 0;
                    BCM_PBMP_ITER(pbmp, port) {
                        if (port > 31) {
                           cli_out("LAG,Port number %d is beyond the range.\n",port);
                           return CMD_FAIL;
                        }
                        garbage |= 1<< port;
                    }
                    cli_out("LAG: Bitmap 0x%08X.\n", garbage);
                    return CMD_OK;
                }
            }
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_trunk_chip_info_get() */
    else if (!sal_strcasecmp(c, "chip")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "info")) {
            if (!(c = ARG_GET(args)))
                return(CMD_USAGE);
            else if (!sal_strcasecmp(c, "get")) {
                rv = bcm_trunk_chip_info_get(unit, &ta_info);
                if (rv == BCM_E_NONE) {
                    cli_out("LAG: Chip Info ....\n");
                    cli_out("     Group Count:   %d\n", ta_info.trunk_group_count);
                    cli_out("     ID Min:        %d\n", ta_info.trunk_id_min);
                    cli_out("     ID Max:        %d\n", ta_info.trunk_id_max);
                    cli_out("     Ports Max:     %d\n", ta_info.trunk_ports_max);
                    cli_out("     Fab ID Min:    %d\n", ta_info.trunk_fabric_id_min);
                    cli_out("     Fab ID Max:    %d\n", ta_info.trunk_fabric_id_max);
                    cli_out("     Fab ports Max: %d\n", ta_info.trunk_fabric_ports_max);
                    return CMD_OK;
                }
            }
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_trunk_create() */
    else if (!sal_strcasecmp(c, "create")) {
        if ((c = ARG_GET(args)) == NULL) {
            rv = bcm_trunk_create(unit, 0, &tid);
            if (rv == BCM_E_NONE) {
                cli_out("LAG: CREATED Group = %d.\n", tid);
                return CMD_OK;
            }
        }
        else if (!sal_strcasecmp(c, "id")) {
            if (!(c = ARG_GET(args)))
                return(CMD_USAGE);
            else {
                tid = sal_ctoi(c, 0);
                rv = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &tid);
            }
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_trunk_destroy() */
    else if (!sal_strcasecmp(c, "destroy")) {
        if ((c = ARG_GET(args)) != NULL) {
            tid = sal_ctoi(c, 0);
            rv = bcm_trunk_destroy(unit, tid);
        }
        else
            return(CMD_USAGE);
    }

    /* bcm_trunk_detach() */
    else if (!sal_strcasecmp(c, "detach")) {
        rv = bcm_trunk_detach(unit);
    }

    /* bcm_trunk_find() */
    else if (!sal_strcasecmp(c, "find")) {
        if ((c = ARG_GET(args)) == NULL)
            return(CMD_USAGE);
        modid = sal_ctoi(c, 0);
        if ((c = ARG_GET(args)) == NULL)
            return(CMD_USAGE);
        port = sal_ctoi(c, 0);
        rv = bcm_trunk_find(unit, modid, port, &tid);
        if (rv == BCM_E_NONE) {
            cli_out("LAG: trunk ID = %d\n", tid);
            return CMD_OK;
        }
    }

    /* bcm_trunk_get() */
    else if (!sal_strcasecmp(c, "get")) {
        if ((c = ARG_GET(args)) == NULL)
            return(CMD_USAGE);
        else {
            tid = sal_ctoi(c, 0);
            rv = bcm_trunk_get(unit, tid, &t_data, 0, NULL, &member_count);
            if (BCM_FAILURE(rv)) {
                return CMD_FAIL;
            }
            if (member_count > 0) {
                member_array = sal_alloc(member_count *
                        sizeof(bcm_trunk_member_t), "trunk member array");
                if (NULL == member_array) {
                    return CMD_FAIL;
                }
                rv = bcm_trunk_get(unit, tid, &t_data, member_count,
                        member_array, &unused_count);
            }
            if (rv == BCM_E_NONE) {
                cli_out("LAG: %d\n", tid);
                cli_out("  flags:        0x%08X\n", t_data.flags);
                cli_out("  port count:     %d\n", member_count);
                cli_out("  port select:    %d\n", t_data.psc);
                cli_out("  dlf_index:      %d\n", t_data.dlf_index);
                cli_out("  mc index:       %d\n", t_data.mc_index);
                cli_out("  ipmc_index:     %d\n", t_data.ipmc_index);
                cli_out("  ports in trunk:");
                for (index = 0; index < member_count; index++) {
                    cli_out("  %d", BCM_GPORT_MODPORT_PORT_GET(
                            member_array[index].gport));
                }
                if (NULL != member_array) {
                    sal_free(member_array);
                }
                cli_out("\n");
                return CMD_OK;
            }
            if (NULL != member_array) {
                sal_free(member_array);
            }
        }
    }

    /* bcm_trunk_init() */
    else if (!sal_strcasecmp(c, "init")) {
        rv = bcm_trunk_init(unit);
    }

    /* bcm_trunk_mcast_join() */
    else if (!sal_strcasecmp(c, "mcast")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "join")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else {
                tid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                vid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
		/* coverity[secure_coding] */
                index = sscanf(c, "%02x:%02x:%02x:%02x:%02x:%02x",
                               &a[0], &a[1], &a[2], &a[3], &a[4], &a[5]);
                if ((EOF == index) || (6 != index))
                    return(CMD_USAGE);
                for (index = 0; index < 6; index++)
                    mac[index] = (uint8)a[index];
                rv = bcm_trunk_mcast_join(unit, tid, vid, mac);
            }
        }
        else
            return(CMD_USAGE);
    }

    else if (!sal_strcasecmp(c, "port")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "add")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else {
                tid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                modid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                port = sal_ctoi(c, 0);
                rv = bcm_trunk_get(unit, tid, &t_data, 0, NULL, &member_count);
                if (BCM_FAILURE(rv)) {
                    return CMD_FAIL;
                }
                member_array = sal_alloc((member_count + 1) *
                        sizeof(bcm_trunk_member_t), "trunk member array");
                if (NULL == member_array) {
                    return CMD_FAIL;
                }
                rv = bcm_trunk_get(unit, tid, &t_data, member_count + 1,
                        member_array, &unused_count);
                if (rv == BCM_E_NONE) {
                    for (index = 0; index < member_count; index++) {
                        if ((BCM_GPORT_MODPORT_MODID_GET(
                                        member_array[index].gport) == modid) &&
                            (BCM_GPORT_MODPORT_PORT_GET(
                                        member_array[index].gport) == port)) {
                            cli_out("LAG: Port is already a member of trunk %d\n", tid);
                            sal_free(member_array);
                            return CMD_OK;
                        }
                    }
                    bcm_trunk_member_t_init(&member_array[member_count]);
                    BCM_GPORT_MODPORT_SET(member_array[member_count].gport,
                            modid, port);
                    rv = bcm_trunk_set(unit, tid, &t_data, member_count + 1,
                            member_array);
                }
                sal_free(member_array);
            }
        }
        else if (!sal_strcasecmp(c, "remove")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else {
                tid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                modid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                port = sal_ctoi(c, 0);
                rv = bcm_trunk_get(unit, tid, &t_data, 0, NULL, &member_count);
                if (BCM_FAILURE(rv)) {
                    return CMD_FAIL;
                }
                if (member_count > 0) {
                    member_array = sal_alloc(member_count *
                            sizeof(bcm_trunk_member_t), "trunk member array");
                    if (NULL == member_array) {
                        return CMD_FAIL;
                    }
                    rv = bcm_trunk_get(unit, tid, &t_data, member_count,
                            member_array, &unused_count);
                }
                if (rv == BCM_E_NONE) {
                    for (index = 0; index < member_count; index++) {
                        if ((BCM_GPORT_MODPORT_MODID_GET(
                                        member_array[index].gport) == modid) &&
                            (BCM_GPORT_MODPORT_PORT_GET(
                                        member_array[index].gport) == port)) {
                            sal_memcpy(&member_array[index],
                                    &member_array[member_count-1],
                                    sizeof(bcm_trunk_member_t));
                            member_count--;
                            if (member_count > 0) {
                                rv = bcm_trunk_set(unit, tid, &t_data,
                                        member_count, member_array);
                            } else {
                                rv = bcm_trunk_set(unit, tid, &t_data,
                                        member_count, NULL);
                            }
                            break;
                        }
                    }
                }
                if (NULL != member_array) {
                    sal_free(member_array);
                }
            }
        } else if (!sal_strcasecmp(c, "down")) {
#ifdef BCM_SIRIUS_SUPPORT
            if (SOC_IS_SIRIUS(unit)) {
                if (!(c = ARG_GET(args))) {
                    return CMD_USAGE;
                }
                port = sal_ctoi(c, NULL);
                rv = bcm_sirius_trunk_port_down(unit, port);
            } else
#endif /* def BCM_SIRIUS_SUPPORT */
            {
                cli_out("not available on this unit\n");
                return CMD_FAIL;
            }
        } else if (!sal_strcasecmp(c, "up")) {
#ifdef BCM_SIRIUS_SUPPORT
            if (SOC_IS_SIRIUS(unit)) {
                if (!(c = ARG_GET(args))) {
                    return CMD_USAGE;
                }
                port = sal_ctoi(c, NULL);
                rv = bcm_sirius_trunk_port_up(unit, port);
            } else
#endif /* def BCM_SIRIUS_SUPPORT */
            {
                cli_out("not available on this unit\n");
                return CMD_FAIL;
            }
        } else {
            return CMD_USAGE;
        }
    }

    /* bcm_trunk_psc_get() */
    /* bcm_trunk_psc_set() */
    else if (!sal_strcasecmp(c, "psc")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "get")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else {
                tid = sal_ctoi(c, 0);
                rv = bcm_trunk_psc_get(unit, tid, &psc);
                if (rv == BCM_E_NONE) {
                    cli_out("LAG: psc = %d\n", psc);
                    return CMD_OK;
                }
            }
        }
        else if (!sal_strcasecmp(c, "set")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else {
                tid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                psc = sal_ctoi(c, 0);
                rv = bcm_trunk_psc_set(unit, tid, psc);
            }
        }
        else
            return(CMD_USAGE);
    }

    else if (!sal_strcasecmp(c, "vlan")) {
        if (!(c = ARG_GET(args)))
            return(CMD_USAGE);
        else if (!sal_strcasecmp(c, "port")) {
            if ((c = ARG_GET(args)) == NULL)
                return(CMD_USAGE);
            else if (!sal_strcasecmp(c, "remove")) {
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                vid = sal_ctoi(c, 0);
                if ((c = ARG_GET(args)) == NULL)
                    return(CMD_USAGE);
                port = sal_ctoi(c, 0);

            }
            else
                return(CMD_USAGE);
        }
        else
            return(CMD_USAGE);
    }




    /* bcm_trunk_set() */
    else if (!sal_strcasecmp(c, "set")) {
        cli_out("LAG: Call trunk port add/remove instead.\n");
    }

    else if (!sal_strcasecmp(c, "debug")) {
#ifdef BCM_SIRIUS_SUPPORT
        if (SOC_IS_SIRIUS(unit)) {
            bcm_sirius_trunk_debug(unit);
            return CMD_OK;
        } else
#endif /* def BCM_SIRIUS_SUPPORT */
        {
            cli_out("LAG: debug not supported by this unit\n");
            return CMD_FAIL;
        }
    }
    else if (!sal_strcasecmp(c, "show")) {
        return _bcm_diag_trunk_show(unit, args);
    }

    else if (!sal_strcasecmp(c, "unavail")) {
        if (BCM_E_UNAVAIL != (rv = bcm_trunk_egress_set(unit, 1, pbmp)))
            cli_out("LAG: bcm_trunk_egress_set() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_egress_get(unit, 1, &pbmp)))
            cli_out("LAG: bcm_trunk_egress_get() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_ucast_set(unit, 1, 1, 1, TRUE)))
            cli_out("LAG: bcm_trunk_override_ucast_set() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_ucast_get(unit, 1, 1, 1, &garbage)))
            cli_out("LAG: bcm_trunk_override_ucast_get() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_mcast_set(unit, 1, 1, 1, TRUE)))
            cli_out("LAG: bcm_trunk_override_mcast_set() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_mcast_get(unit, 1, 1, 1, &garbage)))
            cli_out("LAG: bcm_trunk_override_mcast_get() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_ipmc_set( unit, 1, 1, 1, TRUE)))
            cli_out("LAG: bcm_trunk_override_ipmc_set() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_ipmc_get( unit, 1, 1, 1, &garbage)))
            cli_out("LAG: bcm_trunk_override_ipmc_get() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_vlan_set( unit, 1, 1, 1, TRUE)))
            cli_out("LAG: bcm_trunk_override_vlan_set() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_override_vlan_get( unit, 1, 1, 1, &garbage)))
            cli_out("LAG: bcm_trunk_override_vlan_get() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_pool_set(unit, 1, 1, 1, weights)))
            cli_out("LAG: bcm_trunk_pool_set() Fail!\n");
        else if (BCM_E_UNAVAIL != (rv = bcm_trunk_pool_get(unit, 1, 1, &garbage, weights)))
            cli_out("LAG: bcm_trunk_pool_get() Fail!\n");
        else
            rv = BCM_E_NONE;
    }

    else {
        cli_out("LAG: Command %s not supported.\n", c);
        return CMD_OK;
    }

    if (rv == BCM_E_NONE)
        cli_out("LAG: OK!\n");
    else
        cli_out("LAG: Fail, %s\n", bcm_errmsg(rv));

    return CMD_OK;
}
#endif /* BCM_FE2000_SUPPORT */

char cmd_sbx_mcfpga_rw_usage[] =
"Usage:\n"
"  addr=<address>  - FPGA address (base 0)\n"
"  data=<value>    - Single byte value to write, if write is given\n"
"  write           - boolean flag - write single byte to address\n"
;

cmd_result_t
cmd_sbx_mcfpga_rw(int unit, args_t *a)
{
    int addr = 0;
    int data = 0;
    int write = 0;
    uint8 v;
    int rv;

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "addr", PQ_DFL | PQ_INT,
                        0, &addr, NULL);
        parse_table_add(&pt, "data", PQ_DFL | PQ_INT,
                        0, &data, NULL);
        parse_table_add(&pt, "write", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &write, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    } else {
        return CMD_USAGE;
    }

    if (write) {
        v = ((unsigned int) data) & 0xff;
        rv = _mc_fpga_write8(addr, v);
    } else {
        rv = _mc_fpga_read8(addr, &v);
        cli_out("metrocore FPGA[0x%02x]=0x%02x\n", addr, v);
    }

    return rv;
}

char cmd_sbx_forcemodmap_usage[] =
"Usage:\n"
"forcemodmap \n"
"  this command configures the chip's module IDs and smod/fmod mapping\n"
"  this command is a work-around 'til we get the smod/fmod APIs\n"
"  properly implemented, or we remove MVT diddling from FE-2000 BCM impl\n"
"  global is specified when calling this function once for entire card \n"
;
cmd_result_t
cmd_sbx_forcemodmap(int unit, args_t *a)
{
    int  port;
    int qe=-1;
    int fe=-1;
    int fe_modid[32];
    int qe_modid[32];
    int fport;
    int node;
    int rv = 0;
    bcm_gport_t switch_port, fabric_port;
    bcm_pbmp_t pbmp;

#ifdef BCM_SIRIUS_SUPPORT
#ifdef _MODEL_SIRIUS_SUPPORT_
    int hg_subports[SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS];
    bcm_error_t bcm_err = BCM_E_NONE;
    int no_ports, intf;
#endif /* _MODEL_SIRIUS_SUPPORT_ */
#endif

#ifdef BCM_SIRIUS_SUPPORT
    if (board_type == BOARD_TYPE_SIRIUS_IPASS) {
        cli_out("Running forcemodmap for Sirius Model\n");

        /* configure switch port to fabric port mapping, 1-to-1 mapping */
        for (port = 0; port < 50; port++) {
            BCM_GPORT_MODPORT_SET(switch_port, PL_BCM56634_IPASS_MODID, port);
            BCM_GPORT_MODPORT_SET(fabric_port, PL_SIRIUS_IPASS_MODID, port);

            bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
            if (BCM_FAILURE(bcm_err)) {
                cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                return(CMD_FAIL);
            }
        }
        return(CMD_OK);
    }

    if (board_type == BOARD_TYPE_SIRIUS_SIM) {
#ifdef _MODEL_SIRIUS_SUPPORT_
        cli_out("Running forcemodmap for Sirius Model\n");

        /* read SOC properties to determine the number of ports/channels on each */
        /* interface                                                    */
        for (intf = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
            hg_subports[intf] = soc_property_port_get(unit, intf, spn_IF_SUBPORTS, 0);
        }
        for (intf = 0, no_ports = 0; intf < SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS; intf++) {
            no_ports += hg_subports[intf];
        }

        /* configure switch port to fabric port mapping, 1-to-1 mapping */
        for (port = 0; port < no_ports; port++) {
            BCM_GPORT_MODPORT_SET(switch_port, MODEL_BCM56634_MODID, port);
            BCM_GPORT_MODPORT_SET(fabric_port, MODEL_SIRIUS_MODID, port);

            bcm_err = bcm_stk_fabric_map_set(MODEL_SIRIUS_UNIT, switch_port, fabric_port);
            if (BCM_FAILURE(bcm_err)) {
                cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                return(CMD_FAIL);
            }

#ifdef _BCM56634_MODEL_
            bcm_err = bcm_stk_fabric_map_set(MODEL_BCM56634_MODID, switch_port, fabric_port);
            if (BCM_FAILURE(bcm_err)) {
                cli_out("bcm_stk_fabric_map_set failed err=%d %s\n", bcm_err, bcm_errmsg(bcm_err));
                return(CMD_FAIL);
            }
#endif /* _BCM56634_MODEL_ */
        }

#endif /* _MODEL_SIRIUS_SUPPORT_ */
        return(CMD_OK);
    }
#endif /* BCM_SIRIUS_SUPPORT */

    for (node=0; node<32;node++){
        fe_modid[node] = node;
        qe_modid[node] = 10000+node;
    }


    if (!SOC_IS_SBX_CALADAN3(unit) &&
        !SOC_IS_SBX_QE2000(unit)  ) {
        cli_out("only forcemodmap on Caladan3 and QE-2000 \n");
        return CMD_OK;
    }

    /* Either use the QE chip right before
       or right after this unit - soon we should not need qe unit
       so this code should go away
    */
    if (SOC_IS_SBX_CALADAN3(unit)){
        /* if caladan3 simulation fake fabric unit */
        if (SOC_IS_SBX_CALADAN3(unit) && 
            (SAL_BOOT_PLISIM || SAL_BOOT_BCMSIM)) {
            /* set fabric to unit 1 */
            qe = unit + 1;
	} else if (SOC_IS_SBX_CALADAN3(unit)) {
	    if(soc_property_get(unit, "aedev", 0)) {
	       qe = soc_property_get(unit, "aedev_remote_unit", unit+1);
	    } else {
	       qe = unit + 1;
	    }
        } else {
            if (unit && SOC_IS_SBX_QE2000(unit-1)){
                qe = unit - 1;
            } else if (SOC_IS_SBX_QE2000(unit+1)){
                qe = unit + 1;
            } else{
                cli_out("ERROR: Could not associate QE device with FE\n");
                return CMD_FAIL;
            }
        }
    } else if (SOC_IS_SBX_QE2000(unit)) {
        cli_out("ERROR: could not associate FE device with QE\n");
        return CMD_FAIL;
    }

    if( !SOC_SBX_INIT(unit) ){
        cli_out("unit %d, not initialized - call 'init soc' first!\n",
                unit);
        return CMD_FAIL;
    }

    for (node = 0; node < 32; node++){
        /* This assumes the port configuration is default:    */
        /*     ports 0-11  (ge0-11)  -> fabric ports 0-11     */
        /*     ports 12-23 (ge12-23) -> fabric ports 14-25    */
        /*     port  24    (xg0)     -> fabric port  12       */
        /*     port  25    (xg1)     -> fabric ports 26       */
        /*     port  30    (spi0.13) -> fabric ports 13       */
        /*     port  31    (pci)     -> fabric port  27       */
        /* For C2-Sirius systems the following port configuration is assumed:    */
        /*     ports 0-11  (ge0-11)  -> fabric ports 0-11     */
        /*     ports 12-23 (ge12-23) -> fabric ports 12-23    */
        /*     port  31    (pci)     -> fabric port  24       */
        /*     port  24    (hg0)     -> fabric port  N/A      */
        /*     port  25    (hg1)     -> fabric ports N/A      */

        BCM_PBMP_CLEAR(pbmp);

    /*    coverity[new_values]    */
        if (SOC_IS_SBX_CALADAN3(unit)) {
            BCM_PBMP_ASSIGN(pbmp, PBMP_ALL(unit));

            BCM_PBMP_ITER(pbmp, port) {
                
                if (port < SBX_MAX_PORTS) {
                    /* FE Specific */
                    SOC_SBX_CONTROL(unit)->fabric_units[port] = (sbhandle) qe;
                } else {
                    break;
                }

                fport = port;
                BCM_GPORT_MODPORT_SET(switch_port, fe_modid[node], port);
                BCM_GPORT_MODPORT_SET(fabric_port, qe_modid[node], fport);
                rv = bcm_stk_fabric_map_set(unit,switch_port,fabric_port);
                if (BCM_FAILURE(rv)) {
                    cli_out("error: bcm_stk_fabric_map_set(%s): switch_port 0x%x fabric_port: 0x%x\n",
                            bcm_errmsg(rv),
                            switch_port,
                            fabric_port);
                    return rv;
                }
            }
        } else if (SOC_IS_SBX_QE2000(unit)) {

            /* QE iterates through Fabric ports, not front-panel ports */
            BCM_PBMP_ASSIGN(pbmp, PBMP_SPI_SUBPORT_ALL(unit));

            BCM_PBMP_ITER(pbmp, fport) {

                /* inverse mapping, fabric port to front panel port */
                if (fport == 12) {
                    port = 24;
                } else if (fport == 26) {
                    port = 25;
                } else if (fport == 13) {
                    port = 30;
                } else if (fport == 27) {
                    port = 31;
                } else if (fport < 12) {
                    port = fport;
                } else if (13 < fport && fport <= 25) {
                    port = fport - 2;
                } else {
                    cli_out("unexpected port number in forcemodmap: %d\n", fport);
                    return CMD_FAIL;
                }

                /* QE Specific - map the FrontPanel port to an FE
                 * Primarily used by Rx to determine the FE mapped to a front
                 * panel port.  Since the QE processes the packet on behalf of
                 * the FE, the Ingress port is an FE port, but the unit is a
                 * QE.  The forwarding_units structure is used to find the FE
                 * for bcm_reason mapping purposes.
                 */
                if (port >=0 && port < SBX_MAX_PORTS) {
                    SOC_SBX_CONTROL(unit)->forwarding_units[port] = (sbhandle) fe;
                }

                BCM_GPORT_MODPORT_SET(switch_port, fe_modid[node], port);
                BCM_GPORT_MODPORT_SET(fabric_port, qe_modid[node], fport);
                BCM_IF_ERROR_RETURN
                    (bcm_stk_fabric_map_set(unit, switch_port, fabric_port));
            }

        }

    }/* for node */

    return CMD_OK;
}

void
sbx_handle_fabric_redundancy_event(int unit,
                                   bcm_fabric_control_redundancy_info_t *redundancy_info)
{
    /*
     * Just print some info
     */
    cli_out("Handling the fabric redundancy event from unit %d\n", unit);
    cli_out("Active Sc %d, Active xbars 0x%08x_0x%08x\n", redundancy_info->active_arbiter_id,
            COMPILER_64_HI(redundancy_info->xbars), COMPILER_64_LO(redundancy_info->xbars));
    failover_count++;
}


#ifdef BCM_QE2000_SUPPORT
void
sbx_handle_switch_event(int unit,
			bcm_switch_event_t event, 
			uint32 arg1, 
			uint32 arg2, 
			uint32 arg3, 
			void *userdata)
{
    cli_out("Handling the switch event from unit %d\n", unit);
    if (event == BCM_SWITCH_EVENT_TUNE_ERROR) {
	cli_out("QM block, qm_error2(0x%08x)\n", arg2);
	cli_out("TUNE EVENT - QE2000 early_demand_request error BAA algorithm will not work\n");
    }
}
#endif

char cmd_sbx_mcstate_config_usage[] =
"Usage:\n"
"mcstate \n"
"  Returns 0 if all units have successfuly been 'init bcm' 'd\n";

cmd_result_t
cmd_sbx_mcstate_config(int unit, args_t *a)
{
  int i;
  int retval = 0;

  /* find sbx units*/
  for (i=0;i<soc_ndev;i++){
    if (SOC_IS_SBX(SOC_NDEV_IDX2DEV(i))){
      if (bcm_init_check(SOC_NDEV_IDX2DEV(i))==FALSE){
        cli_out("Unit %d init_check is not ok\n", SOC_NDEV_IDX2DEV(i));
        retval = -1;
      }else{
        cli_out("Unit %d init_check is ok\n", SOC_NDEV_IDX2DEV(i));
      }
    }
  }
  return retval;
}
/* GNATS 22312, enable links to QE prior to other links to backplane */
int32 pl_lc_chassis_link_tbl[] = {
    /* LCM to QE links */
   4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
  58, 59,
   /* LCM to fabric links */
   0,  1, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72,
  73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89,
  90, 91, 92, 93, 94, 95, -1
};

int32 pl_fabric_chassis_link_tbl[] = {
  34, 24, 46, 16, 56, 6,  68, 71, 35, 36, 37, 38, 39, 40, 41, 42, 43,
  25, 26, 27, 28, 29, 30, 31, 32, 33, 44, 45, 47, 48, 49, 50, 51, 52,
  53, 54, 55, 17, 18, 19, 20, 21, 22, 23, 57, 58, 59, 60, 61, 62, 63,
  64, 65,  7,  8,  9, 10, 11, 12, 13, 14, 15,  0,  1,  2,  3, 69, 70,
  72, 73, 74, 75, 76, 77, 78, 79, 84, 85, 86, 87, -1
};

int32 pl_lc_standalone_link_tbl[] = {
   4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55,
  58, 59, 22, 57, 24, 27, -1
};

int32 fe2kxt_lc_chassis_link_tbl[] = {
    /* LCM to QE links */
   4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
  21,
   /* LCM to fabric links */
  64, 65, 66, 67, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84,
  85, -1
};

char cmd_sbx_mctune_usage[] =
"Usage:\n"
"mctune \n"
"  tune hypercore receivers\n";

cmd_result_t
cmd_sbx_mctune(int unit, args_t *arg)
{
    int retval = 0;
    int32 *pLink;
    int myUnit;


    /* find sbx units*/
    for (myUnit=0;myUnit<soc_ndev;myUnit++){
        if (SOC_IS_SBX(SOC_NDEV_IDX2DEV(myUnit))){
            if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(myUnit))) {

                switch (card_type) {

                    case SBX_CARD_TYPE_PL_CHASSIS_LINE_CARD:
                        pLink = pl_lc_chassis_link_tbl;
                        break;
                    case SBX_CARD_TYPE_PL_CHASSIS_FABRIC_CARD:
                        pLink = pl_fabric_chassis_link_tbl;
                        break;
                    case SBX_CARD_TYPE_PL_STANDALONE_LINE_CARD:
                        pLink = pl_lc_standalone_link_tbl;
                        break;
	            case SBX_CARD_TYPE_FE2KXT_CHASSIS_LINE_CARD:
                        pLink = fe2kxt_lc_chassis_link_tbl;
                        break;
                    default:
                        cli_out("no links tuned for this card type\n");
                        return retval;
                }
                while (*pLink != -1) {
                    retval = bcm_port_control_set(SOC_NDEV_IDX2DEV(myUnit), *pLink, bcmPortControlSerdesDriverTune, 1);
                    pLink++;
                    if (retval) {
                        cli_out("ERROR: tune command failed for link(%d) unit(%d)\n",*pLink, SOC_NDEV_IDX2DEV(myUnit));
                        return retval;
                    }
                }
                cli_out("links tuned OK\n");
            }
        }
    }

    return retval;
}

char cmd_sbx_mcenablesiports_usage[] =
"Usage:\n"
"mcenablesi \n"
"  Enable BM9600 si ports - for sequenced LCM initialization sequencing purposes\n";

cmd_result_t
cmd_sbx_mcenablesiports(int unit, args_t *arg)
{
    int retval = 0;
    int32 *pLink;
    int myUnit;

    /* find sbx units*/
    for (myUnit=0;myUnit<soc_ndev;myUnit++){
        if (SOC_IS_SBX(SOC_NDEV_IDX2DEV(myUnit))){
            if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(myUnit))) {

                switch (card_type) {

                    case SBX_CARD_TYPE_PL_CHASSIS_LINE_CARD:
                        pLink = pl_lc_chassis_link_tbl;
                        break;
                    case SBX_CARD_TYPE_PL_CHASSIS_FABRIC_CARD:
                        pLink = pl_fabric_chassis_link_tbl;
                        break;
                    case SBX_CARD_TYPE_PL_STANDALONE_LINE_CARD:
                        cli_out("no LCM in this config, SI ports on BM9600 are already enabled\n");
                        return retval;
                        break;
	            case SBX_CARD_TYPE_FE2KXT_CHASSIS_LINE_CARD:
                        pLink = fe2kxt_lc_chassis_link_tbl;
                        break;
                    default:
                        cli_out("no config for this card type\n");
                        return retval;
                }
                while (*pLink != -1) {
                    retval = bcm_port_enable_set(SOC_NDEV_IDX2DEV(myUnit), *pLink, 1);
                    pLink++;
                    if (retval) {
                        cli_out("ERROR: SI port enable command failed for link(%d) unit(%d)\n",*pLink, SOC_NDEV_IDX2DEV(myUnit));
                        return retval;
                    }
                }
                cli_out("SI links enabled OK\n");
            }
        }
    }

    return retval;
}


/*
 * Following tables of commands used to configure metrocore and Polaris line cards.
 * Picking up info from config.bcm not used since some data overridden by defaults.
 *  Rules for configuring:
 *  - Polaris card requires module id of QEs to be the serdes port that they connect to
 *  - Crossbar tables reside in Polaris, if present, else in QEs
 */

dev_cmd_rec mc_lc_dev_cmd_tbl[] = {
        { SET_MODULE_PROTOCOL, MCLC_UNIT_QE, MCLC_MODID_QE, bcmModuleProtocol1, 0, 0, 0 },

        {PORT_ENABLE_RANGE, MCLC_UNIT_LCM0, 31, 39, 0, 0},   /* SFIs */
        {PORT_ENABLE, MCLC_UNIT_LCM0, 0,  0, 0, 0},   /* SCI  */

        {PORT_ENABLE_RANGE, MCLC_UNIT_LCM1, 31, 39, 0, 0},

        {SET_MODID,   MCLC_UNIT_QE, MCLC_MODID_QE, 0, 0, 0},
        {SET_MODID,   MCLC_UNIT_FE, MCLC_MODID_FE, 0, 0, 0},

        {PORT_ENABLE, MCLC_UNIT_QE, 68, 0, 0, 0},
        {PORT_ENABLE_RANGE, MCLC_UNIT_QE, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},


        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  0, MCLC_MODID_QE, 0, MCLC_MODID_QE, 31},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  1, MCLC_MODID_QE, 0, MCLC_MODID_QE, 32},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  2, MCLC_MODID_QE, 0, MCLC_MODID_QE, 33},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  3, MCLC_MODID_QE, 0, MCLC_MODID_QE, 34},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  4, MCLC_MODID_QE, 0, MCLC_MODID_QE, 35},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  5, MCLC_MODID_QE, 0, MCLC_MODID_QE, 36},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  6, MCLC_MODID_QE, 0, MCLC_MODID_QE, 37},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  7, MCLC_MODID_QE, 0, MCLC_MODID_QE, 38},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  8, MCLC_MODID_QE, 0, MCLC_MODID_QE, 39},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE,  9, MCLC_MODID_QE, 0, MCLC_MODID_QE, 31},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 10, MCLC_MODID_QE, 0, MCLC_MODID_QE, 32},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 11, MCLC_MODID_QE, 0, MCLC_MODID_QE, 33},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 12, MCLC_MODID_QE, 0, MCLC_MODID_QE, 34},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 13, MCLC_MODID_QE, 0, MCLC_MODID_QE, 35},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 14, MCLC_MODID_QE, 0, MCLC_MODID_QE, 36},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 15, MCLC_MODID_QE, 0, MCLC_MODID_QE, 37},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 16, MCLC_MODID_QE, 0, MCLC_MODID_QE, 38},
        {XBAR_CONNECT_SET, MCLC_UNIT_QE, 17, MCLC_MODID_QE, 0, MCLC_MODID_QE, 39},

        {FAB_CONTROL_SET, MCLC_UNIT_QE, bcmFabricArbiterId,          0, 0, 0, 0},
        {FAB_CONTROL_SET, MCLC_UNIT_QE, bcmFabricActiveArbiterId,    0, 0, 0, 0},
        {FAB_CONTROL_SET, MCLC_UNIT_QE, bcmFabricMaximumFailedLinks, 3, 0, 0, 0},
        {FAB_CONTROL_SET, MCLC_UNIT_QE, bcmFabricRedundancyMode,     0, 0, 0, 0},

        {FAB_CONTROL_SET, MCLC_UNIT_LCM0, bcmFabricArbiterId,          0, 0, 0, 0},
        {FAB_CONTROL_SET, MCLC_UNIT_LCM0, bcmFabricActiveArbiterId,    0, 0, 0, 0},
        {FAB_CONTROL_SET, MCLC_UNIT_LCM0, bcmFabricMaximumFailedLinks, 3, 0, 0, 0},
        {REDUND_REGISTER, MCLC_UNIT_LCM0, 0,                           0, 0, 0, 0},
        {FAB_CONTROL_SET, MCLC_UNIT_LCM0, bcmFabricRedundancyMode,     0, 0, 0, 0},

        {XBAR_ENABLE_SET,   MCLC_UNIT_LCM0, 0x3ffff, 0, 0, 0, 0},
        {MODULE_ENABLE_SET, MCLC_UNIT_LCM0, MCLC_MODID_QE, 1, 0, 0, 0},
        {XBAR_ENABLE_SET,   MCLC_UNIT_QE,   0x3ffff, 0, 0, 0, 0},
        {-1, 0, 0, 0, 0, 0, 0},
    };

char cmd_sbx_mcremoveall_usage[] =
"Usage:\n"
"mcremoveall \n"
"  Removes all flows \n";

cmd_result_t
cmd_sbx_mcremoveall(int unit, args_t *a)
{
  int retval = 0;
  int rv = 0;
  int i = 0;
  bcm_sbx_gport_cb_params_t cbParams;

  /* remove vlans/L2 entries for FE devices
   * remove gports for QE/BM devices
   */

    for(i=0;i<soc_ndev;i++) {
      if (!SOC_SBX_INIT(SOC_NDEV_IDX2DEV(i))) {
        cli_out("Skipping %s[u:%d] not initialized\n",SOC_CHIP_STRING(SOC_NDEV_IDX2DEV(i)),SOC_NDEV_IDX2DEV(i));
        continue;
      }
      if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i))) {
        rv = bcm_vlan_destroy_all(SOC_NDEV_IDX2DEV(i));
        if (BCM_FAILURE(rv)) {
          cli_out("ERROR: bcm_vlan_destory_all(%d)(%s) unit:%d \n",rv,bcm_errmsg(rv),SOC_NDEV_IDX2DEV(i));
          retval = -1;
        }
      }
      if (SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i)) ||
          SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i))) {
        int nEset;
        for (nEset = 0; nEset<SOC_SBX_CFG(unit)->num_ds_ids; nEset++){
          if (soc_property_get(SOC_NDEV_IDX2DEV(i), spn_DIAG_COSQ_INIT,0)==0){
            bcm_fabric_distribution_destroy(SOC_NDEV_IDX2DEV(i), (bcm_fabric_distribution_t) nEset);
          }
        }
      }
      if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i))  ||
          SOC_IS_SBX_BM9600(SOC_NDEV_IDX2DEV(i))  ||
          SOC_IS_SBX_BME3200(SOC_NDEV_IDX2DEV(i)) ||
	  SOC_IS_SIRIUS(SOC_NDEV_IDX2DEV(i))) {
        if (soc_property_get(SOC_NDEV_IDX2DEV(i), spn_DIAG_COSQ_INIT,0)==0){
          sal_memset(&cbParams, 0x00, sizeof(cbParams));
          cbParams.cmd = BCM_SBX_COSQ_GPORT_REMOVE_ALL;
          rv = bcm_cosq_gport_traverse(SOC_NDEV_IDX2DEV(i),_bcm_gport_delete,&cbParams);
	  if ((rv != BCM_E_UNAVAIL) && (rv != BCM_E_NONE)) {
            cli_out("ERROR: Deleting gports for %s[u:%d] failed (%s)\n",
                    SOC_CHIP_STRING(SOC_NDEV_IDX2DEV(i)),SOC_NDEV_IDX2DEV(i),bcm_errmsg(rv));
            retval = -1;
          }
        }
      }

      if (SOC_IS_SIRIUS(SOC_NDEV_IDX2DEV(i))) {
	sbx_gport_map_delete(unit);
      }


      if (SOC_IS_SBX_QE2000(SOC_NDEV_IDX2DEV(i))) {
        /* remove trunks too */
        rv = bcm_trunk_detach(SOC_NDEV_IDX2DEV(i));
        if (BCM_FAILURE(rv)){
          cli_out("ERROR: Trunk detach  for %s[u:%d] failed (%s)\n",
                  SOC_CHIP_STRING(SOC_NDEV_IDX2DEV(i)),SOC_NDEV_IDX2DEV(i),bcm_errmsg(rv));
          retval = -1;
        }
        rv = bcm_trunk_init(SOC_NDEV_IDX2DEV(i));
        if (BCM_FAILURE(rv)){
          cli_out("ERROR: Trunk re-init  for %s[u:%d] failed (%s)\n",
                  SOC_CHIP_STRING(SOC_NDEV_IDX2DEV(i)),SOC_NDEV_IDX2DEV(i),bcm_errmsg(rv));
          retval = -1;
        }
      }
    } /* for each unit */

    return retval;
}

/* this is temporary set to max for bm9600 */
#define DIAG_CLI_MAX_NODES 72

static char * bcmGportStatStr[] =
{ "GreenAcceptedPkts","GreenAcceptedBytes", "GreenMarkedPkts",
  "GreenMarkedBytes", "GreenDiscardPkts", "GreenDiscardBytes",
  "YellowAcceptedPkts", "YellowAcceptedBytes","YellowMarkedPkts",
  "YellowMarkedBytes", "YellowDiscardPkts","YellowDiscardBytes",
  "RedAcceptedPkts", "RedAcceptedBytes", "RedMarkedPkts",
  "RedMarkedBytes", "RedDiscardPkts", "RedDiscardBytes",
  "Dp3AcceptedPkts", "Dp3AcceptedBytes",
  "NonWredDroppedPkts", "NonWredDroppedBytes","DequeuedPkts",
  "DequeuedBytes" };

static int bcmGportStatType[] =
{   bcmCosqGportGreenAcceptedPkts,      /* Green/DP0, accepted packet count. */
    bcmCosqGportGreenAcceptedBytes,     /* Green/DP0, accepted byte count. */
    bcmCosqGportGreenCongestionMarkedPkts, /* Green/DP0, ECN Marked packets. */
    bcmCosqGportGreenCongestionMarkedBytes, /* Green/DP0, ECN Marked bytes. */
    bcmCosqGportGreenDiscardDroppedPkts, /* Green/DP0, WRED dropped packets. */
    bcmCosqGportGreenDiscardDroppedBytes, /* Green/DP0, WRED dropped bytes. */
    bcmCosqGportYellowAcceptedPkts,     /* Yellow/DP1, accepted packet count. */
    bcmCosqGportYellowAcceptedBytes,    /* Yellow/DP1, accepted byte count. */
    bcmCosqGportYellowCongestionMarkedPkts, /* Yellow/DP1, ECN Marked packets. */
    bcmCosqGportYellowCongestionMarkedBytes, /* Yellow/DP1, ECN nMarked bytes. */
    bcmCosqGportYellowDiscardDroppedPkts, /* Yellow/DP1, WRED dropped packets. */
    bcmCosqGportYellowDiscardDroppedBytes, /* Yellow/DP1, WRED dropped bytes. */
    bcmCosqGportRedAcceptedPkts,        /* Red/DP2, accepted packet count. */
    bcmCosqGportRedAcceptedBytes,       /* Red/DP2, accepted byte count. */
    bcmCosqGportRedCongestionMarkedPkts, /* Red/DP2, ECN Marked packets. */
    bcmCosqGportRedCongestionMarkedBytes, /* Red/DP2, ECN Marked bytes. */
    bcmCosqGportRedDiscardDroppedPkts,  /* Red/DP2, WRED dropped packets. */
    bcmCosqGportRedDiscardDroppedBytes, /* Red/DP2, WRED dropped bytes. */
    bcmCosqGportBlackAcceptedPkts,      /* Black/DP3, accepted packet count. */
    bcmCosqGportBlackAcceptedBytes,     /* Black/DP3, accepted byte count. */
    bcmCosqGportNonWredDroppedPkts,     /* NON-WRED dropped packet count. */
    bcmCosqGportNonWredDroppedBytes,    /* NON-WRED dropped byte count. */
    bcmCosqGportDequeuedPkts,           /* dequeued packets. */
    bcmCosqGportDequeuedBytes,          /* dequeued bytes. */
};

static char * bcmGportSiriusStatStr[] =
{ "GreenAcceptedPkts","GreenAcceptedBytes", "GreenMarkedPkts",
  "GreenMarkedBytes", "GreenDiscardPkts", "GreenDiscardBytes",
  "YellowAcceptedPkts", "YellowAcceptedBytes","YellowMarkedPkts",
  "YellowMarkedBytes", "YellowDiscardPkts","YellowDiscardBytes",
  "RedAcceptedPkts", "RedAcceptedBytes", "RedMarkedPkts",
  "RedMarkedBytes", "RedDiscardPkts", "RedDiscardBytes",
  "BlackAcceptedPkts", "BlackAcceptedBytes", "BlackMarkedPkts",
  "BlackMarkedBytes", "BlackDiscardPkts", "BlackDiscardBytes",
  "NonWredDroppedPkts", "NonWredDroppedBytes",
  "OverSubTtlDrpdPkts", "OverSubTtlDrpdBytes",
  "OverSubGuarDrpdPkts", "OverSubGuarDrpdBytes",
  "DequeuedPkts", "DequeuedBytes"};

static int bcmGportSiriusStatType[] =
{   bcmCosqGportGreenAcceptedPkts,      /* Green/DP0, accepted packet count. */
    bcmCosqGportGreenAcceptedBytes,     /* Green/DP0, accepted byte count. */
    bcmCosqGportGreenCongestionMarkedPkts, /* Green/DP0, ECN Marked packets. */
    bcmCosqGportGreenCongestionMarkedBytes, /* Green/DP0, ECN Marked bytes. */
    bcmCosqGportGreenDiscardDroppedPkts, /* Green/DP0, WRED dropped packets. */
    bcmCosqGportGreenDiscardDroppedBytes, /* Green/DP0, WRED dropped bytes. */
    bcmCosqGportYellowAcceptedPkts,     /* Yellow/DP1, accepted packet count. */
    bcmCosqGportYellowAcceptedBytes,    /* Yellow/DP1, accepted byte count. */
    bcmCosqGportYellowCongestionMarkedPkts, /* Yellow/DP1, ECN Marked packets. */
    bcmCosqGportYellowCongestionMarkedBytes, /* Yellow/DP1, ECN nMarked bytes. */
    bcmCosqGportYellowDiscardDroppedPkts, /* Yellow/DP1, WRED dropped packets. */
    bcmCosqGportYellowDiscardDroppedBytes, /* Yellow/DP1, WRED dropped bytes. */
    bcmCosqGportRedAcceptedPkts,        /* Red/DP2, accepted packet count. */
    bcmCosqGportRedAcceptedBytes,       /* Red/DP2, accepted byte count. */
    bcmCosqGportRedCongestionMarkedPkts, /* Red/DP2, ECN Marked packets. */
    bcmCosqGportRedCongestionMarkedBytes, /* Red/DP2, ECN Marked bytes. */
    bcmCosqGportRedDiscardDroppedPkts,  /* Red/DP2, WRED dropped packets. */
    bcmCosqGportRedDiscardDroppedBytes, /* Red/DP2, WRED dropped bytes. */
    bcmCosqGportBlackAcceptedPkts,      /* Black/DP3, accepted packet count. */
    bcmCosqGportBlackAcceptedBytes,     /* Black/DP3, accepted byte count. */
    bcmCosqGportNonWredDroppedPkts,     /* NON-WRED dropped packet count. */
    bcmCosqGportNonWredDroppedBytes,    /* NON-WRED dropped byte count. */
    bcmCosqGportDequeuedPkts,           /* dequeued packets. */
    bcmCosqGportDequeuedBytes,          /* dequeued bytes. */
    bcmCosqGportGreenDroppedPkts,       /* Green/DP0, non-WRED dropped pkts. */
    bcmCosqGportGreenDroppedBytes,      /* Green/DP0, non-WRED dropped bytes. */
    bcmCosqGportYellowDroppedPkts,      /* Yellow/DP1, non-WRED dropped pkts. */
    bcmCosqGportYellowDroppedBytes,     /* Yellow/DP1, non-WRED dropped bytes. */
    bcmCosqGportRedDroppedPkts,         /* Red/DP2, non-WRED dropped pkts. */
    bcmCosqGportRedDroppedBytes,        /* Red/DP2, non-WRED dropped bytes. */
    bcmCosqGportBlackCongestionMarkedPkts, /* Black/DP3, ECN Marked packets. */
    bcmCosqGportBlackCongestionMarkedBytes /* Black/DP3, ECN Marked bytes. */
};

char cmd_sbx_fabric_usage[] =
"Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
" [] specifies optional arguments\n"
" <mode> options: BE,WFQ,SP,EF,AF,SP_GLOBAL,SP0,SP1,SP2,SP3,SP4,SP5,SP6,SP7,AF0,AF1,AF2,AF3,CALENDAR,NONE\n"
" gport show [modid=<modid> port=<port> | gport=0x<gport>] [verbose=1]\n"
" gport remove [modid=<modid> port=<port> | gport=0x<gport>]\n"
" gport add modid=<modid> port=<port> [physical_gport=0x<gport>] [gport=0x<gport>] [cos=<cos>] [flags=<flags>]\n"
" gport bandwidth_get modid=<modid> port=<port> | gport=0x<gport> [cos=<cos>]\n"
" gport bandwidth_set modid=<modid> port=<port> | gport=0x<gport> [cos=<cos>] kbits_sec_min=<KbpsMin> kbits_sec_max=<KbpsMax>\n"
" gport sched_get modid=<modid> port=<port> | gport=0x<gport> [cos=<cos>]\n"
" gport sched_set modid=<modid> port=<port> | gport=0x<gport> [cos=<cos>] mode=<mode> [weight=<weight>]\n"
#ifdef BCM_SIRIUS_SUPPORT
" gport sched_show - display schedulers\n"
" gport ingress_tree_show verbose=depth - display ingress tree\n"
#endif /* BCM_SIRIUS_SUPPORT */
" gport stats_set modid=<modid> port=<port> | gport=0x<gport>  enable=true|false\n"
" gport stats_get modid=<modid> port=<port> | gport=0x<gport> \n"
" gport discard_show modid=<modid> port=<port> | gport=0x<gport> cos=<cos> [color=<0-2>]\n"
" gport discard_set modid=<modid> port=<port> | gport=0x<gport> cos=<cos> [color=<0-2>] [enable=true|false] min_thresh=<min_thresh> "
  "max_thresh=<max_thresh> drop_probability=<drop_probability> gain=<gain> ecn_thresh=<ecn_thresh>\n"

" gport egress size_set modid=<modid> port=<port> cos=<cos> min_size=<min_size> max_size=<max_size>\n"
" gport egress size_get modid=<modid> port=<port> cos=<cos>\n"
" gport map - displays switchport/fabricport mappings\n"
" distribution create [id=<dsgroupid>]\n"
" distribution destroy id=<dsgroupid>\n"
" distribution add id=<dsgroupid> modid=<modid>\n"
" distribution delete id=<dsgroupid> modid=<modid>\n"
" distribution show id=<dsgroup>\n"
" hqos show oix <oix>\n"
" hqos set oix <oix> queue <queue>\n"
" vlan show vlan <port>\n"
" vlan set vlan <port> queue <queue>\n"
" port show port <port>\n"
" port set port <port> queue <queue>\n"
#endif
;
cmd_result_t
cmd_sbx_fabric(int unit, args_t *a)
{

  int rv = 0;
  int retcode = CMD_OK;
  char *subcmd = NULL;
  bcm_gport_t gport = BCM_GPORT_INVALID;
  bcm_gport_t physical_port = 0;
  int modid,port;
  int verbose = 0;
  char *cmd = NULL;
  parse_table_t pt;
  int cos_levels,user_cos;
  uint32 flags;
  uint32 kbits_sec_min,kbits_sec_max = 0;
  int gport_base_queue, num_cos_levels;
  int cos = 0;
  int start_cos = 0;
  int mode,weight;
  char *mode_str = NULL;
  char *enable_str = NULL;
  uint8 bNoGport = FALSE;
  uint8 bNoModPort;
  int stat_enable = 0;
  int discard_enable = 0;
  bcm_cosq_gport_stats_t stat;
  uint64 value = COMPILER_64_INIT(0,0);
  bcm_sbx_gport_cb_params_t cb_params;
  int max_nodes;
  int count;
  int distmodids[DIAG_CLI_MAX_NODES];  /* fix eventually */
  int *dist_modids_p = NULL;
  bcm_fabric_distribution_t dsgroupid = -1;
  int idx;
  int bFoundModid = FALSE;
  int color, min_thresh, max_thresh, drop_probability, gain, ecn_thresh;
  char *color_str = NULL;
  bcm_cosq_gport_discard_t discard;
  int is_egress = FALSE;
  uint32 min_size, max_size;

#if defined(BCM_QE2000_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
  int oix = -1;
  int queue,vlan;
  char *c = NULL;
#endif
#if defined(BCM_QE2000_SUPPORT)
  sbG2EplibCtxt_st *ep;
  uint32 ip_table_mem[4];
#endif
#ifdef BCM_SIRIUS_SUPPORT
  bcm_cosq_subscriber_map_t map;
#endif

  sal_memset(&cb_params, 0, sizeof(bcm_sbx_gport_cb_params_t));

  if (!sh_check_attached(ARG_CMD(a), unit)) {
    return CMD_FAIL;
  }

  /* unit needs to be init'd */
  if (!SOC_SBX_INIT(unit)) {
    cli_out("%s[u:%d] not initialized\n",SOC_CHIP_STRING(unit),unit);
    return CMD_FAIL;
  }

  max_nodes = SOC_SBX_CFG(unit)->num_nodes;

  subcmd = ARG_GET(a);
  if (subcmd == NULL) {
    return CMD_USAGE;
  }
  if (sal_strcasecmp(subcmd,"gport") == 0 ) {
    /* correct indentation eventually */
    if ((cmd = ARG_GET(a)) == NULL) {
      return CMD_USAGE;
    }

    if (sal_strcasecmp(cmd, "egress") == 0) {
      is_egress = TRUE;
      if ((cmd = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
      }
    }

    if (is_egress == FALSE) {
      if (sal_strcasecmp(cmd, "show") == 0 ||
          sal_strcasecmp(cmd, "remove") == 0 ||
          sal_strcasecmp(cmd, "add") == 0 ||
          sal_strcasecmp(cmd, "map") == 0 ||
#ifdef BCM_SIRIUS_SUPPORT
          sal_strcasecmp(cmd, "sched_show") == 0 ||
          sal_strcasecmp(cmd, "ingress_tree_show") == 0 ||
#endif /* BCM_SIRIUS_SUPPORT */
          sal_strcasecmp(cmd, "bandwidth_set") == 0 ||
          sal_strcasecmp(cmd, "bandwidth_get") == 0 ||
          sal_strcasecmp(cmd, "sched_get") == 0 ||
          sal_strcasecmp(cmd, "sched_set") == 0 ||
          sal_strcasecmp(cmd, "stats_set") == 0 ||
          sal_strcasecmp(cmd, "stats_get") == 0 ||
          sal_strcasecmp(cmd, "discard_show") == 0 ||
          sal_strcasecmp(cmd, "discard_set") == 0 ) {
        /* parse valid arguments for above options */
        parse_table_init(unit,&pt);
        parse_table_add(&pt, "gport", PQ_INT,
                      (void *)(BCM_GPORT_INVALID),&gport, 0);
        parse_table_add(&pt, "modid", PQ_INT,
                      (void *)(-1),&modid, 0);
        parse_table_add(&pt, "port", PQ_INT,
                      (void *)(-1),&port, 0);
        parse_table_add(&pt, "physical_gport", PQ_INT,
                      (void *)(0),&physical_port, 0);
        parse_table_add(&pt,  "verbose",PQ_INT,(void *) 0,
                      &verbose, NULL);

        if (sal_strcasecmp(cmd, "add") == 0) {
          parse_table_add(&pt, "cos_levels", PQ_INT,
                        (void *)(NUM_COS(unit)),&cos_levels, 0);
          parse_table_add(&pt, "flags", PQ_INT,
                        (void *)(0),&flags, 0);
        }


        if (sal_strcasecmp(cmd, "bandwidth_set") == 0 ) {
          parse_table_add(&pt, "kbits_sec_min", PQ_INT,
                        (void *)(-1),&kbits_sec_min, 0);
          parse_table_add(&pt, "kbits_sec_max", PQ_INT,
                        (void *)(-1),&kbits_sec_max, 0);
          parse_table_add(&pt, "flags", PQ_INT,
                        (void *)(0),&flags, 0);
          parse_table_add(&pt, "cos", PQ_INT,
                        (void *)(-1),&user_cos, 0);
        }

        if (sal_strcasecmp(cmd, "bandwidth_get") == 0 ) {
          parse_table_add(&pt, "cos", PQ_INT,
                        (void *)(-1),&user_cos, 0);
        }

        if (sal_strcasecmp(cmd, "sched_get") == 0 ||
            sal_strcasecmp(cmd, "sched_set") == 0  ||
            sal_strcasecmp(cmd, "stats_get") == 0 ) {
          parse_table_add(&pt, "cos", PQ_INT,
                        (void *)(-1),&user_cos, 0);
        }

        if (sal_strcasecmp(cmd, "sched_set") == 0 ) {
          parse_table_add(&pt, "mode", PQ_STRING,(void *) "_none_",
                        &mode_str, 0);
          parse_table_add(&pt, "weight", PQ_INT,
                        (void *)(0),&weight, 0);
        }

        if (sal_strcasecmp(cmd, "stats_set") == 0 ) {
          parse_table_add(&pt, "enable", PQ_STRING, (void*) "true",
                        &enable_str,0);
        }

        if (sal_strcasecmp(cmd, "discard_show") == 0 ) {
          parse_table_add(&pt, "color", PQ_INT,
                        (void *)(0),&color, 0);
          parse_table_add(&pt, "cos", PQ_INT,
                        (void *)(-1),&user_cos, 0);
        }

        if (sal_strcasecmp(cmd, "discard_set") == 0 ) {
          parse_table_add(&pt, "enable", PQ_STRING, (void*) "true",
                        &enable_str,0);
          parse_table_add(&pt, "color", PQ_INT,
                        (void *)(0),&color, 0);
          parse_table_add(&pt, "cos", PQ_INT,
                        (void *)(-1),&user_cos, 0);
          parse_table_add(&pt, "min_thresh", PQ_INT,
                        (void *)(-1),&min_thresh, 0);
          parse_table_add(&pt, "max_thresh", PQ_INT,
                        (void *)(-1),&max_thresh, 0);
          parse_table_add(&pt, "drop_probability", PQ_INT,
                        (void *)(-1),&drop_probability, 0);
          parse_table_add(&pt, "gain", PQ_INT,
                        (void *)(-1),&gain, 0);
          parse_table_add(&pt, "ecn_thresh", PQ_INT,
                        (void *)(-1),&ecn_thresh, 0);
        }

        /* consume valid arguments */
        if (parse_arg_eq(a, &pt) < 0) {
          cli_out("%s: Invalid option: %s\n",
                  ARG_CMD(a), ARG_CUR(a));
          parse_arg_eq_done(&pt);
          return CMD_USAGE;
        }

        if (ARG_CNT(a) != 0) {
          cli_out("%s: extra options starting with \"%s\" \n",
                  ARG_CMD(a), ARG_CUR(a));
          parse_arg_eq_done(&pt);
          return CMD_USAGE;
        }

        /* The add command always requires modid and port, gport is optional */
        if (sal_strcasecmp(cmd,"add") == 0) {
	    if ((physical_port == 0) && (modid == -1 || port == -1 )) {
            cli_out("%s requires modid and port parameters\n",cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
          }
        }

        /* certain cmds require a gport or at least modid and port */
        if (sal_strcasecmp(cmd, "bandwidth_set") == 0 ||
            sal_strcasecmp(cmd, "bandwidth_get") == 0 ||
            sal_strcasecmp(cmd, "sched_get") == 0 ||
            sal_strcasecmp(cmd, "sched_set") == 0 ||
            sal_strcasecmp(cmd, "stats_set") == 0 ||
            sal_strcasecmp(cmd, "stats_get") == 0 ||
            sal_strcasecmp(cmd, "discard_show") == 0 ||
            sal_strcasecmp(cmd, "discard_set") == 0 ) {
          if ((gport == BCM_GPORT_INVALID) &&
            ((modid == -1) || (port == -1 ))) {
            cli_out("%s requires gport or modid and port\n",cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
          }
        }
      } else {
        cli_out(" ==> Invalid gport command(%s)\n",cmd);
        return CMD_USAGE;
      }

      /* bandwidth_set requires at least kbits_sec_min and kbits_sec_max */
      if (sal_strcasecmp(cmd, "bandwidth_set") == 0 ) {
        if ((kbits_sec_min == -1 || kbits_sec_max == -1)) {
          cli_out("%s requires kbits_sec_min and kbits_sec_max\n",cmd);
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }
      }

      if (sal_strcasecmp(cmd, "discard_show") == 0 ) {
        if (user_cos == -1) {
            cli_out("%s requires cos\n",cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        if ( (color < 0) || (color > 2) ) {
            cli_out("%s color out of range [0-2]\n",cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
      }

      if (sal_strcasecmp(cmd, "discard_set") == 0 ) {
        if ((user_cos == -1) || (min_thresh == -1) || (max_thresh == -1) ||
            (drop_probability == -1) || (gain == -1) || (ecn_thresh == -1)) {
            cli_out("%s requires cos, min_thresh, max_thresh, drop_probability, gain and ecn_thresh\n",cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
        if ( (color < 0) || (color > 2) ) {
            cli_out("%s color out of range [0-2]\n",cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
        }
      }


      if (sal_strcasecmp(cmd,"map") == 0) {      
         sbx_gport_map_show(unit);
         return CMD_OK;
       }

#ifdef BCM_SIRIUS_SUPPORT
      if (sal_strcasecmp(cmd, "sched_show") == 0) {
	sbx_gport_scheduler_show(unit);
	return CMD_OK;
      }
      if (sal_strcasecmp(cmd, "ingress_tree_show") == 0) {
          return sbx_sirius_ingress_tree_show(unit, verbose);
      }
#endif /* BCM_SIRIUS_SUPPORT */

      /* build the gport */
      if (sal_strcasecmp(subcmd,"gport") == 0) {
        if (gport == BCM_GPORT_INVALID) {
          bNoGport = TRUE;
          if (( modid != -1 && port != -1)) {
            BCM_GPORT_MODPORT_SET(gport,modid,port);
          }
        }
      }

      if ((modid == -1) || (port == -1)) {
        bNoModPort = TRUE;
      } else {
        bNoModPort = FALSE;
      }

      cb_params.verbose = verbose;
      cb_params.modid = modid;
      cb_params.port = port;

      /* run the gport fabric cmd */
      if (sal_strcasecmp(cmd,"show") == 0 ) {
	if ((gport == BCM_GPORT_INVALID)                      ||
	    BCM_GPORT_IS_UCAST_QUEUE_GROUP (gport)            ||
	    BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP (gport) ||
	    BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP (gport) ||
	    BCM_GPORT_IS_MCAST_QUEUE_GROUP (gport)) {
	    cli_out("    Gport      nCos                       Type            Modid     Port       Basequeue\n");
	    cli_out(" ---------------------------------------------------------------------------------------");
	    if (verbose)
		cli_out("------");
	    cli_out("\n");
	    if (bNoGport && bNoModPort) { /* show all gports */
		cb_params.cmd = BCM_SBX_COSQ_GPORT_SHOW_ALL;
		rv = bcm_cosq_gport_traverse(unit,_bcm_gport_show,&cb_params);
	    } else if (bNoGport) { /* show only gports that match this modid and/or port */
		cb_params.cmd = BCM_SBX_COSQ_GPORT_SHOW;
		rv = bcm_cosq_gport_traverse(unit,_bcm_gport_show,&cb_params);
	    } else { /* show this gport only */
		rv = sbx_cosq_gport_show(unit,gport,verbose);
	    }
	} else {
	  /* display information for non-queue gport types */
	  sbx_gport_display(unit,gport);
	}
      } else if (sal_strcasecmp(cmd,"remove") == 0) {
        if (bNoGport && bNoModPort) { /* remove all gports */
          cb_params.cmd = BCM_SBX_COSQ_GPORT_REMOVE_ALL;
          rv = bcm_cosq_gport_traverse(unit,_bcm_gport_delete,&cb_params);
        } else if (bNoGport) { /* remove only gports that match modid/and or port */
          cb_params.cmd = BCM_SBX_COSQ_GPORT_REMOVE;
          rv = bcm_cosq_gport_traverse(unit,_bcm_gport_delete,&cb_params);
        } else { /* remove this gport only */
          rv = bcm_cosq_gport_delete(unit,gport);
        }
      } else if (sal_strcasecmp(cmd,"add") == 0 ) {
	if (physical_port == 0) {
	    BCM_GPORT_MODPORT_SET(physical_port,modid,port); /* required */
	}
        if (bNoGport) { gport = 0; }
        rv = bcm_cosq_gport_add(unit,physical_port,cos_levels,flags,&gport);
        if (rv == 0) {
            cli_out("Added gport=(0x%x)\n",gport);
        }
      } else if (sal_strcasecmp(cmd,"bandwidth_get") == 0 ) {
        if (user_cos == -1) {
	  if (SOC_IS_SIRIUS(unit) && (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
				      BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
				      BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
				      BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport))) {
	    /* get bandwith params for all cos levels for this gport */
	    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit,gport,&gport_base_queue,&num_cos_levels);
	    if (BCM_FAILURE(rv)) {
	      cli_out("ERROR: bcm_cosq_get_base_queue_from_gport failed(%s)\n",bcm_errmsg(rv));
	      parse_arg_eq_done(&pt);
	      return CMD_FAIL;
	    }
	  } else {
	    /* EGRESS GROUP cos=-1 means subport level, MODPORT and CHILD Gport in sirius doesn't have
	       base queue associated with it
	    */
	    start_cos = -1;
	    num_cos_levels = start_cos + 1;
	  }
        } else {
          start_cos = user_cos;
          num_cos_levels = user_cos + 1;
        }
        for(cos=start_cos;cos<num_cos_levels;cos++) {
          rv = bcm_cosq_gport_bandwidth_get(unit,gport,cos,&kbits_sec_min,&kbits_sec_max,&flags);
          if (BCM_FAILURE(rv)) {
            cli_out("ERROR: %s failed for (cos=%d) (%s)\n",cmd,cos,bcm_errmsg(rv));
            retcode = -1;
          } else {
            cli_out("gport:0x%x cos=%d, kbits_sec_min=%d, kbits_sec_max=%d, flags=%x\n",
                    gport,cos,kbits_sec_min,kbits_sec_max,flags);
          }
        }
      } else if (sal_strcasecmp(cmd,"bandwidth_set") == 0 ) {
        if (user_cos == -1) {
          /* no cos specified , use cos=0 but set flag for COSQ_ALL */
	  flags = BCM_COSQ_ALL;
	  if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
	    cos = -1;
	  } else {
	    cos = 0;
	  }
          rv = bcm_cosq_gport_bandwidth_set(unit,gport,cos,kbits_sec_min,kbits_sec_max,flags);
          if (BCM_FAILURE(rv)) {
            cli_out("ERROR: %s failed for (cos=%d) (%s)\n",cmd,cos,bcm_errmsg(rv));
            retcode = -1;
          }
        } else {
          rv = bcm_cosq_gport_bandwidth_set(unit,gport,user_cos,kbits_sec_min,kbits_sec_max,flags);
          if (BCM_FAILURE(rv)) {
            cli_out("ERROR: %s failed for (cos=%d) (%s)\n",cmd,cos,bcm_errmsg(rv));
            retcode = -1;
          }
        }
      } else if (sal_strcasecmp(cmd,"sched_get") == 0 ) {
        if (user_cos == -1) {
	  if ((SOC_IS_SIRIUS(unit) ||
               SOC_IS_SBX_QE2000(unit) ||
               SOC_IS_SBX_BME3200(unit) ||
               SOC_IS_SBX_BM9600(unit)) &&
              (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
               BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
               BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
               BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport))) {
	    /* show scheduling params for all cos levels for this gport */
	    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit,gport,&gport_base_queue,&num_cos_levels);
	    if (BCM_FAILURE(rv)) {
	      cli_out("ERROR: bcm_cosq_get_base_queue_from_gport failed(%s)\n",bcm_errmsg(rv));
	      parse_arg_eq_done(&pt);
	      return CMD_FAIL;
	    }
	  } else {
	    start_cos = 0;
	    num_cos_levels = start_cos + 1;
	  }
        } else {
          start_cos = user_cos;
          num_cos_levels = user_cos + 1;
        }
        for(cos=start_cos;cos<num_cos_levels;cos++) {
          rv = bcm_cosq_gport_sched_get(unit,gport,cos,&mode,&weight);
          if (BCM_FAILURE(rv)) {
            cli_out("ERROR: %s failed for (cos=%d) (%s)\n",cmd,cos,bcm_errmsg(rv));
            retcode = -1;
          } else {
            cli_out("gport:0x%x,",gport);
            switch (mode) {
            case BCM_COSQ_BE:
              cli_out(" cos=%d, mode=BE\n",cos);
              break;
            case BCM_COSQ_SP:
              cli_out(" cos=%d, mode=SP, Priority=%d\n",cos,weight);
              break;
            case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
              cli_out(" cos=%d, mode=WFQ, Weight=%d\n",cos,weight);
              break;
            case BCM_COSQ_EF:
              cli_out(" cos=%d, mode=EF\n",cos);
              break;
            case BCM_COSQ_AF:
              cli_out(" cos=%d, mode=AF, Weight=%d\n",cos,weight);
              break;
            case BCM_COSQ_SP_GLOBAL:
              cli_out(" cos=%d, mode=SP_GLOBAL, priority=%d\n",cos,weight);
              break;
            case BCM_COSQ_SP0:
	      cli_out(" cos=%d, mode=SP0\n",cos);
	      break;
            case BCM_COSQ_SP1:
	      cli_out(" cos=%d, mode=SP1\n",cos);
	      break;
            case BCM_COSQ_SP2:
	      cli_out(" cos=%d, mode=SP2\n",cos);
	      break;
            case BCM_COSQ_SP3:
	      cli_out(" cos=%d, mode=SP3\n",cos);
	      break;
            case BCM_COSQ_SP4:
	      cli_out(" cos=%d, mode=SP4\n",cos);
	      break;
            case BCM_COSQ_SP5:
	      cli_out(" cos=%d, mode=SP5\n",cos);
	      break;
            case BCM_COSQ_SP6:
	      cli_out(" cos=%d, mode=SP6\n",cos);
	      break;
            case BCM_COSQ_SP7:
	      cli_out(" cos=%d, mode=SP7\n",cos);
	      break;
            case BCM_COSQ_AF0:
	      cli_out(" cos=%d, mode=AF0, priority=%d\n",cos,weight);
	      break;
            case BCM_COSQ_AF1:
	      cli_out(" cos=%d, mode=AF1, priority=%d\n",cos,weight);
	      break;
            case BCM_COSQ_AF2:
	      cli_out(" cos=%d, mode=AF2, priority=%d\n",cos,weight);
	      break;
            case BCM_COSQ_AF3:
	      cli_out(" cos=%d, mode=AF3, priority=%d\n",cos,weight);
	      break;
            case BCM_COSQ_CALENDAR:
	      cli_out(" cos=%d, mode=CALENDAR\n",cos);
	      break;
            case BCM_COSQ_NONE:
	      cli_out(" cos=%d, mode=NONE\n",cos);
	      break;
            default:
              cli_out("todo: support %d mode\n",mode);
              retcode = -1;
            }
          }
        }
      } else if (sal_strcasecmp(cmd,"sched_set") == 0 ) {
        if (mode_str != NULL) {
          if (sal_strcasecmp(mode_str,"BE") == 0 ) {
            mode = BCM_COSQ_BE;
          } else if (sal_strcasecmp(mode_str,"SP") == 0) {
            mode = BCM_COSQ_SP;
          } else if (sal_strcasecmp(mode_str,"WFQ") == 0) {
            mode = BCM_COSQ_WEIGHTED_FAIR_QUEUING;
          } else if (sal_strcasecmp(mode_str,"EF") == 0) {
            mode = BCM_COSQ_EF;
          } else if (sal_strcasecmp(mode_str,"AF") == 0) {
            mode = BCM_COSQ_AF;
          } else if (sal_strcasecmp(mode_str,"SP_GLOBAL") == 0) {
            mode = BCM_COSQ_SP_GLOBAL;
          } else if (sal_strcasecmp(mode_str,"SP0") == 0) {
            mode = BCM_COSQ_SP0;
          } else if (sal_strcasecmp(mode_str,"SP1") == 0) {
            mode = BCM_COSQ_SP1;
          } else if (sal_strcasecmp(mode_str,"SP2") == 0) {
            mode = BCM_COSQ_SP2;
          } else if (sal_strcasecmp(mode_str,"SP3") == 0) {
            mode = BCM_COSQ_SP3;
          } else if (sal_strcasecmp(mode_str,"SP4") == 0) {
            mode = BCM_COSQ_SP4;
          } else if (sal_strcasecmp(mode_str,"SP5") == 0) {
            mode = BCM_COSQ_SP5;
          } else if (sal_strcasecmp(mode_str,"SP6") == 0) {
            mode = BCM_COSQ_SP6;
          } else if (sal_strcasecmp(mode_str,"SP7") == 0) {
            mode = BCM_COSQ_SP7;
          } else if (sal_strcasecmp(mode_str,"AF0") == 0) {
            mode = BCM_COSQ_AF0;
          } else if (sal_strcasecmp(mode_str,"AF1") == 0) {
            mode = BCM_COSQ_AF1;
          } else if (sal_strcasecmp(mode_str,"AF2") == 0) {
            mode = BCM_COSQ_AF2;
          } else if (sal_strcasecmp(mode_str,"AF3") == 0) {
            mode = BCM_COSQ_AF3;
          } else if (sal_strcasecmp(mode_str,"CALENDAR") == 0) {
            mode = BCM_COSQ_CALENDAR;
          } else if (sal_strcasecmp(mode_str,"NONE") == 0) {
            mode = BCM_COSQ_NONE;
          } else {
            cli_out("mode %s not recognized\n",mode_str);
            parse_arg_eq_done(&pt);
            return CMD_USAGE;
          }
        } else { /* should never get here */
          cli_out("%s mode_str is null\n",cmd);
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }

        if (user_cos == -1) {
	  if (SOC_IS_SIRIUS(unit) && (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport) ||
				      BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport) ||
				      BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport) ||
				      BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport))) {
	    /* set same scheduling params for all cos levels for this gport */
	    rv = bcm_sbx_cosq_get_base_queue_from_gport(unit,gport,&gport_base_queue,&num_cos_levels);
	    if (BCM_FAILURE(rv)) {
	      cli_out("ERROR: bcm_cosq_get_base_queue_from_gport failed(%s)\n",bcm_errmsg(rv));
	      parse_arg_eq_done(&pt);
	      return CMD_FAIL;
	    }
	  } else {
	    start_cos = -1;
	    num_cos_levels = start_cos + 1;
	  }
        } else {
          start_cos = user_cos;
          num_cos_levels = user_cos + 1;
        }
        for(cos=start_cos;cos<num_cos_levels;cos++) {
          rv = bcm_cosq_gport_sched_set(unit,gport,cos,mode,weight);
          if (BCM_FAILURE(rv)) {
            cli_out("ERROR: %s failed for (cos=%d) (%s)\n",cmd,cos,bcm_errmsg(rv));
            retcode = -1;
          }
        }
      } else if (sal_strcasecmp(cmd, "stats_set") == 0 ) {
        if (enable_str != NULL) {
          if (!strncmp(enable_str, "t", 1)) {
            cli_out("Enabling stats..for gport=0x%x\n",gport);
            stat_enable = 1;
          } else {
            cli_out("Disabling stats..for gport=0x%x\n",gport);
          }
        }
        rv = bcm_cosq_gport_stat_enable_set(unit,gport,stat_enable);
      } else if (sal_strcasecmp(cmd, "stats_get") == 0 ) {
        /* be sure stats are enabled */
        bcm_cosq_gport_stat_enable_get(unit,gport,&stat_enable);
        if (stat_enable == 0) {
          cli_out("Enable stats with stat_set for gport=(0x%x) first\n",gport);
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }
        if (user_cos == -1) {
          /* return all the stats for each cos level for this gport */
          rv = bcm_sbx_cosq_get_base_queue_from_gport(unit,gport,&gport_base_queue,&num_cos_levels);
          if (BCM_FAILURE(rv)) {
            cli_out("ERROR: bcm_cosq_get_base_queue_from_gport failed(%s)\n",bcm_errmsg(rv));
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
          }
        } else {
          start_cos = user_cos;
          num_cos_levels = user_cos + 1;
        }
        for(cos=start_cos;cos<num_cos_levels;cos++) {
          cli_out("\ngport:(0x%x) cos=%d\n",gport,cos);

	  if (SOC_IS_SIRIUS(unit)) {
	    for (stat=0;stat<(sizeof(bcmGportSiriusStatType)/sizeof(bcmGportSiriusStatType[0]));stat++) {
	      rv = bcm_cosq_gport_stat_get(unit,gport,cos,bcmGportSiriusStatType[stat],&value);
	      if (BCM_FAILURE(rv)) {
		cli_out("ERROR %s failed getting stats for %s with cos=%d (%s) \n",cmd,bcmGportSiriusStatStr[stat],cos,
                        bcm_errmsg(rv));
		retcode = -1;
		continue;
	      } else {
                  if (!(stat%2)) {
                      cli_out("\n");
                  }
                  cli_out(" %20s=0x%x%08x     ",
                          bcmGportSiriusStatStr[stat],COMPILER_64_HI(value),
                          COMPILER_64_LO(value));
	      }
	    }
	  } else {
	    for (stat=0;stat<(sizeof(bcmGportStatType)/sizeof(bcmGportStatType[0]));stat++) {
	      rv = bcm_cosq_gport_stat_get(unit,gport,cos,bcmGportStatType[stat],&value);
	      if (BCM_FAILURE(rv)) {
		cli_out("ERROR %s failed getting stats for %s with cos=%d (%s) \n",cmd,bcmGportStatStr[stat],cos,
                        bcm_errmsg(rv));
		retcode = -1;
		continue;
	      } else {
                  if (!(stat%3)) {
                      cli_out("\n");
                  }
                  cli_out(" %20s=0x%x%08x     ",
                          bcmGportStatStr[stat],COMPILER_64_HI(value),
                          COMPILER_64_LO(value));
	      }
	    }
	  }
          cli_out("\n");
        }
      } else if (sal_strcasecmp(cmd, "discard_show") == 0 ) {
        cos = user_cos;
        discard.min_thresh = 0;
        discard.max_thresh = 0;
        discard.drop_probability = 0;
        discard.gain = 0;
        discard.ecn_thresh = 0;
        switch (color) {
        case 0:
          discard.flags = BCM_COSQ_DISCARD_COLOR_GREEN;
          color_str = "green";
          break;
        case 1:
          discard.flags = BCM_COSQ_DISCARD_COLOR_YELLOW;
          color_str = "yellow";
          break;
        case 2:
          discard.flags = BCM_COSQ_DISCARD_COLOR_RED;
          color_str = "red";
          break;
        }
        rv = bcm_cosq_gport_discard_get(unit, gport, cos, &discard);
        if (BCM_FAILURE(rv)) {
          cli_out("ERROR %s failed getting discard setting for cos=%d (%s) \n",cmd, cos,
                  bcm_errmsg(rv));
        } else {
          cli_out(" color           =%s    \n",color_str);
          cli_out(" min_thresh      =%d    \n",discard.min_thresh);
          cli_out(" max_thresh      =%d    \n",discard.max_thresh);
          cli_out(" drop_probability=%d    \n",discard.drop_probability);
          cli_out(" gain            =%d    \n",discard.gain);
          cli_out(" ecn_thresh      =%d    \n",discard.ecn_thresh);
        }
      } else if (sal_strcasecmp(cmd, "discard_set") == 0 ) {
        if (enable_str != NULL) {
          if (!strncmp(enable_str, "t", 1)) {
            cli_out("Enabling discard..for gport=0x%x\n",gport);
            discard_enable = 1;
          } else {
            cli_out("Disabling discard..for gport=0x%x\n",gport);
          }
        }
        cos = user_cos;
        switch (color) {
        case 0:
          discard.flags = BCM_COSQ_DISCARD_COLOR_GREEN;
          break;
        case 1:
          discard.flags = BCM_COSQ_DISCARD_COLOR_YELLOW;
          break;
        case 2:
          discard.flags = BCM_COSQ_DISCARD_COLOR_RED;
          break;
        }
        if (discard_enable) {
          discard.flags |= BCM_COSQ_DISCARD_ENABLE;
        }
        discard.min_thresh = min_thresh;
        discard.max_thresh = max_thresh;
        discard.drop_probability = drop_probability;
        discard.gain = gain;
        discard.ecn_thresh = ecn_thresh;
        rv = bcm_cosq_gport_discard_set(unit, gport, cos, &discard);
      }

      parse_arg_eq_done(&pt);
      /* change of indentation -- fix eventually */
    } else {

      parse_table_init(unit,&pt);

      if ( (sal_strcasecmp(cmd, "size_set") == 0) ||
           (sal_strcasecmp(cmd, "size_get") == 0) ) {
        /* parse valid arguments for above options */
        parse_table_add(&pt, "modid", PQ_INT, (void *)(-1), &modid, 0);
        parse_table_add(&pt, "port", PQ_INT, (void *)(-1), &port, 0);
        parse_table_add(&pt, "cos", PQ_INT, (void *)(-1), &user_cos, 0);
        parse_table_add(&pt,  "verbose",PQ_INT,(void *) 0, &verbose, NULL);
      }

      if (sal_strcasecmp(cmd, "size_set") == 0 ) {
        parse_table_add(&pt, "min_size", PQ_INT, (void *)(-1), &min_size, 0);
        parse_table_add(&pt, "max_size", PQ_INT, (void *)(-1), &max_size, 0);
      }

      if (sal_strcasecmp(cmd, "size_get") == 0 ) {
      }

      /* consume valid arguments */
      if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Invalid option: %s\n", ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
      }

      if (ARG_CNT(a) != 0) {
        cli_out("%s: extra options starting with \"%s\" \n", ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
      }

      if (sal_strcasecmp(cmd, "size_set") == 0) {
          if ( (modid == -1) || (port == -1) || (user_cos == -1) ||
                                       (min_size == -1) || (max_size == -1) ) {
            cli_out("%s requires modid, port, cos, min_size, max_size\n", cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
          }
      }
      if (sal_strcasecmp(cmd, "size_get") == 0) {
          if ( (modid == -1) || (port == -1) || (user_cos == -1) ) {
            cli_out("%s requires modid, port, cos\n", cmd);
            parse_arg_eq_done(&pt);
            return CMD_FAIL;
          }
      }

      if (sal_strcasecmp(cmd, "size_set") == 0) {
        BCM_GPORT_EGRESS_MODPORT_SET(gport, modid, port);
        rv = bcm_cosq_gport_size_set(unit, gport, user_cos, min_size, max_size);
        if (BCM_FAILURE(rv)) {
          cli_out("ERROR: %s failed for (cos=%d) (%s)\n", cmd, user_cos, bcm_errmsg(rv));
          retcode = -1;
        }
      }

      else if (sal_strcasecmp(cmd, "size_get") == 0) {
        BCM_GPORT_EGRESS_MODPORT_SET(gport, modid, port);
        rv = bcm_cosq_gport_size_get(unit, gport, user_cos, &min_size, &max_size);
        if (BCM_FAILURE(rv)) {
          cli_out("ERROR: %s failed for (cos=%d) (%s)\n", cmd, user_cos, bcm_errmsg(rv));
          retcode = -1;
        }

        cli_out("ModId: %d Port: %d cos:%d MinSize:%d (0x%x) MaxSize:%d (0x%x)\n",
                modid, port, user_cos, min_size, min_size, max_size, max_size);
      }
    }

    parse_arg_eq_done(&pt);

#if defined(BCM_QE2000_SUPPORT) || defined(BCM_SIRIUS_SUPPORT)
  } else if (sal_strcasecmp(subcmd, "hqos") == 0) {
#ifdef BCM_QE2000_SUPPORT
      if (SOC_IS_SBX_QE2000(unit)) {
	  oix = -1;
	  queue = -1;
	  ep = gu2_unit2ep(unit);
	  if (ep == NULL) {
	      cli_out("ERROR: no EP handle available\n");
	      return CMD_FAIL;
	  }
	  _GET_NONBLANK(a,subcmd);
	  while ((c = ARG_GET(a)) != NULL) {
	      if (!sal_strncasecmp(c,"oix",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  oix = sal_ctoi(c,0);
	      } else if (!sal_strncasecmp(c,"queue",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  queue = sal_ctoi(c,0);
	      } else {
		  return CMD_USAGE;
	      }
	  }
	  if (oix == -1) {
	      return CMD_USAGE;
	  }
	  rv = sbG2EplibHQosRemapGet(ep, oix, ip_table_mem);
	  if (rv != SB_ELIB_OK) {
	      cli_out("ERROR: showing hqos(%d) for %s[u:%d] failed\n", oix,
                      SOC_CHIP_STRING(unit),unit);
	      rv = -1;
	  } else if (!sal_strncasecmp(subcmd,"show",strlen(subcmd))) {
	      switch (oix & 3) {
		  case 0:
		      queue = ip_table_mem[1] >> 16;
		      break;
		  case 1:
		      queue = ip_table_mem[1] & 0xFFFF;
		      break;
		  case 2:
		      queue = ip_table_mem[0] >> 16;
		      break;
		  case 3:
		      queue = ip_table_mem[0] & 0xFFFF;
		      break;
	      }
	      queue = (queue >> 8) | ((queue & 0xFF) << 8);
	      cli_out("HQoS mapping for oix %d is %d (0x%x)\n", oix, queue, queue);
	      rv = 0;
	  } else if (!sal_strncasecmp(subcmd,"set",strlen(subcmd))) {
	      queue = (queue >> 8) | ((queue & 0xFF) << 8);
	      switch (oix & 3) {
		  case 0:
		      ip_table_mem[1] &= 0xffff;
		      ip_table_mem[1] |= (queue << 16);
		      break;
		  case 1:
		      ip_table_mem[1] &= 0xffff0000;
		      ip_table_mem[1] |= queue;
		      break;
		  case 2:
		      ip_table_mem[0] &= 0xffff;
		      ip_table_mem[0] |= (queue << 16);
		      break;
		  case 3:
		      ip_table_mem[0] &= 0xffff0000;
		      ip_table_mem[0] |= queue;
		      break;
	      }
	      rv = sbG2EplibHQosRemapSet(ep, oix, ip_table_mem);
	      if (rv != SB_ELIB_OK) {
		  cli_out("ERROR: setting hqos(%d) for %s[u:%d] failed\n", oix,
                          SOC_CHIP_STRING(unit),unit);
		  rv = -1;
	      } else {
		  rv = 0;
	      }
	  }
      }
#endif
#ifdef BCM_SIRIUS_SUPPORT 
      if (SOC_IS_SIRIUS(unit)) {
	  sal_memset(&map, 0, sizeof(bcm_cosq_subscriber_map_t));
	  oix = -1;
	  queue = -1;
	  _GET_NONBLANK(a,subcmd);
	  while ((c = ARG_GET(a)) != NULL) {
	      if (!sal_strncasecmp(c,"oix",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  oix = sal_ctoi(c,0);
	      } else if (!sal_strncasecmp(c,"queue",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  queue = sal_ctoi(c,0);
	      } else {
		  return CMD_USAGE;
	      }
	  }
	  if ((oix < 8192) || (oix >= 65536)) {
	      return CMD_USAGE;
	  }
	  if (!sal_strncasecmp(subcmd,"show",strlen(subcmd))) {
	      map.flags = BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID;
	      map.encap_id = oix;
	      rv = bcm_cosq_subscriber_map_get(unit,&map);
	      if (rv != BCM_E_NONE) {
		  cli_out("ERROR: showing hqos(%d) for %s[u:%d] failed\n", oix,
                          SOC_CHIP_STRING(unit),unit);
		  rv = -1;
	      } else {
		  cli_out("HQoS mapping for oix %d is %d (0x%x)\n", oix,  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map.queue_id),
                          BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map.queue_id));
		  rv = 0;
	      }
	  } else if (!sal_strncasecmp(subcmd,"set",strlen(subcmd))) {
	      map.flags = BCM_COSQ_SUBSCRIBER_MAP_ENCAP_ID;
	      map.encap_id = oix;
	      BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(map.queue_id, queue);
	      rv = bcm_cosq_subscriber_map_add(unit, &map);
	      if (rv != BCM_E_NONE) {
		  cli_out("ERROR: setting hqos(%d) for %s[u:%d] failed\n", oix,
                          SOC_CHIP_STRING(unit),unit);
		  rv = -1;
	      } else {
		  rv = 0;
	      }
	  }
      }
#endif
  } else if (sal_strcasecmp(subcmd, "vlan") == 0) {
#ifdef BCM_QE2000_SUPPORT
      if (SOC_IS_SBX_QE2000(unit)) {
	  vlan = -1;
	  queue = -1;
	  ep = gu2_unit2ep(unit);
	  if (ep == NULL) {
	      cli_out("ERROR: no EP handle available\n");
	      return CMD_FAIL;
	  }
	  _GET_NONBLANK(a,subcmd);
	  while ((c = ARG_GET(a)) != NULL) {
	      if (!sal_strncasecmp(c,"vlan",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  vlan = sal_ctoi(c,0);
	      } else if (!sal_strncasecmp(c,"queue",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  queue = sal_ctoi(c,0);
	      } else {
		  return CMD_USAGE;
	      }
	  }
	  if (vlan == -1) {
	      return CMD_USAGE;
	  }
	  rv = sbG2EplibVlanRemapGet(ep, vlan, ip_table_mem);
	  if (rv != SB_ELIB_OK) {
	      cli_out("ERROR: showing vlan(%d) for %s[u:%d] failed\n", vlan,
                      SOC_CHIP_STRING(unit),unit);
	      rv = -1;
	  } else if (!sal_strncasecmp(subcmd,"show",strlen(subcmd))) {
	      switch (vlan & 3) {
		  case 0:
		      queue = ip_table_mem[1] >> 16;
		      break;
		  case 1:
		      queue = ip_table_mem[1] & 0xFFFF;
		      break;
		  case 2:
		      queue = ip_table_mem[0] >> 16;
		      break;
		  case 3:
		      queue = ip_table_mem[0] & 0xFFFF;
		      break;
	      }
	      queue = (queue >> 8) | ((queue & 0xFF) << 8);
	      cli_out("VLAN mapping for vlan %d is %d (0x%x)\n", vlan, queue, queue);
	      rv = 0;
	  } else if (!sal_strncasecmp(subcmd,"set",strlen(subcmd))) {
	      queue = (queue >> 8) | ((queue & 0xFF) << 8);
	      switch (vlan & 3) {
		  case 0:
		      ip_table_mem[1] &= 0xffff;
		      ip_table_mem[1] |= (queue << 16);
		      break;
		  case 1:
		      ip_table_mem[1] &= 0xffff0000;
		      ip_table_mem[1] |= queue;
		      break;
		  case 2:
		      ip_table_mem[0] &= 0xffff;
		      ip_table_mem[0] |= (queue << 16);
		      break;
		  case 3:
		      ip_table_mem[0] &= 0xffff0000;
		      ip_table_mem[0] |= queue;
		      break;
	      }
	      rv = sbG2EplibVlanRemapSet(ep, vlan, ip_table_mem);
	      if (rv != SB_ELIB_OK) {
		  cli_out("ERROR: setting vlan(%d) for %s[u:%d] failed\n", vlan,
                          SOC_CHIP_STRING(unit),unit);
		  rv = -1;
	      } else {
		  rv = 0;
	      }
	  }
      } 
#endif
#ifdef BCM_SIRIUS_SUPPORT
      if (SOC_IS_SIRIUS(unit)) {
	  if (sal_strcasecmp(subcmd, "vlan") == 0) {
	      vlan = -1;
	      queue = -1;
	      _GET_NONBLANK(a,subcmd);
	      while ((c = ARG_GET(a)) != NULL) {
		  if (!sal_strncasecmp(c,"vlan",strlen(c))) {
		      _GET_NONBLANK(a,c);
		      vlan = sal_ctoi(c,0);
		  } else if (!sal_strncasecmp(c,"queue",strlen(c))) {
		      _GET_NONBLANK(a,c);
		      queue = sal_ctoi(c,0);
		  } else {
		      return CMD_USAGE;
		  }
	      }
	      cli_out("Use port command for hqos support\n");
	      rv = 0;
	  }
      }
#endif
  } else if (sal_strcasecmp(subcmd, "port") == 0) {
#ifdef BCM_QE2000_SUPPORT
      if (SOC_IS_SBX_QE2000(unit)) {
	  port = -1;
	  queue = -1;
	  ep = gu2_unit2ep(unit);
	  if (ep == NULL) {
	      cli_out("ERROR: no EP handle available\n");
	      return CMD_FAIL;
	  }
	  _GET_NONBLANK(a,subcmd);
	  while ((c = ARG_GET(a)) != NULL) {
	      if (!sal_strncasecmp(c,"port",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  port = sal_ctoi(c,0);
	      } else if (!sal_strncasecmp(c,"queue",strlen(c))) {
		  _GET_NONBLANK(a,c);
		  queue = sal_ctoi(c,0);
	      } else {
		  return CMD_USAGE;
	      }
	  }
	  if (port == -1) {
	      return CMD_USAGE;
	  }
	  rv = sbG2EplibPortEncapGet(ep, port, ip_table_mem);
	  if (rv != SB_ELIB_OK) {
	      cli_out("ERROR: showing port(%d) for %s[u:%d] failed\n", port,
                      SOC_CHIP_STRING(unit),unit);
	      rv = -1;
	  } else if (!sal_strncasecmp(subcmd,"show",strlen(subcmd))) {
	      switch (port & 3) {
		  case 0:
		      queue = ip_table_mem[1] >> 16;
		      break;
		  case 1:
		      queue = ip_table_mem[1] & 0xFFFF;
		      break;
		  case 2:
		      queue = ip_table_mem[0] >> 16;
		      break;
		  case 3:
		      queue = ip_table_mem[0] & 0xFFFF;
		      break;
	      }
	      queue = (queue >> 8) | ((queue & 0xFF) << 8);
	      cli_out("Port mapping for port %d is %d (0x%x)\n", port, queue, queue);
	      rv = 0;
	  } else if (!sal_strncasecmp(subcmd,"set",strlen(subcmd))) {
	      queue = (queue >> 8) | ((queue & 0xFF) << 8);
	      switch (port & 3) {
		  case 0:
		      ip_table_mem[1] &= 0xffff;
		      ip_table_mem[1] |= (queue << 16);
		      break;
		  case 1:
		      ip_table_mem[1] &= 0xffff0000;
		      ip_table_mem[1] |= queue;
		      break;
		  case 2:
		      ip_table_mem[0] &= 0xffff;
		      ip_table_mem[0] |= (queue << 16);
		      break;
		  case 3:
		      ip_table_mem[0] &= 0xffff0000;
		      ip_table_mem[0] |= queue;
		      break;
	      }
	      rv = sbG2EplibPortEncapSet(ep, port, ip_table_mem);
	      if (rv != SB_ELIB_OK) {
		  cli_out("ERROR: setting port(%d) for %s[u:%d] failed\n", port,
                          SOC_CHIP_STRING(unit),unit);
		  rv = -1;
	      } else {
		  rv = 0;
	      }
	  }
      } 
#endif
#ifdef BCM_SIRIUS_SUPPORT
      if (SOC_IS_SIRIUS(unit)) {
	  sal_memset(&map, 0, sizeof(bcm_cosq_subscriber_map_t));
	  if (sal_strcasecmp(subcmd, "port") == 0) {
	      port = -1;
	      queue = -1;
	      _GET_NONBLANK(a,subcmd);
	      while ((c = ARG_GET(a)) != NULL) {
		  if (!sal_strncasecmp(c,"port",strlen(c))) {
		      _GET_NONBLANK(a,c);
		      port = sal_ctoi(c,0);
		  } else if (!sal_strncasecmp(c,"queue",strlen(c))) {
		      _GET_NONBLANK(a,c);
		      queue = sal_ctoi(c,0);
		  } else {
		      return CMD_USAGE;
		  }
	      }
	      if (port == -1) {
		  return CMD_USAGE;
	      }
	      if (!sal_strncasecmp(subcmd,"show",strlen(subcmd))) {
		  map.flags = BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN;
		  map.port = port;
		  rv = bcm_cosq_subscriber_map_get(unit,&map);
		  if (rv != BCM_E_NONE) {
		      cli_out("ERROR: showing hqos(%d) for %s[u:%d] failed\n", oix,
                              SOC_CHIP_STRING(unit),unit);
		      rv = -1;
		  } else {
		      cli_out("Port mapping for port %d is %d (0x%x)\n", port,  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map.queue_id),
                              BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(map.queue_id));
		      rv = 0;
		  }
	      } else if (!sal_strncasecmp(subcmd,"set",strlen(subcmd))) {
		  map.flags = BCM_COSQ_SUBSCRIBER_MAP_PORT_VLAN;
		  map.port = port;
		  BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_SET(map.queue_id, queue);
		  rv = bcm_cosq_subscriber_map_add(unit, &map);
		  if (rv != BCM_E_NONE) {
		      cli_out("ERROR: setting port(%d) for %s[u:%d] failed\n", port,
                              SOC_CHIP_STRING(unit),unit);
		      rv = -1;
		  } else {
		      rv = 0;
		  }
	      }
	  }
      }
#endif
#endif /* BCM_QE2000_SUPPORT || BCM_SIRIUS_SUPPORT */
  } else if (sal_strcasecmp(subcmd,"distribution") == 0 ) {
    if ((cmd = ARG_GET(a)) == NULL) {  return CMD_USAGE; }
#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit) && (SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME
				|| SOC_SBX_CFG(unit)->bTmeMode == SOC_SBX_QE_MODE_TME_BYPASS)) {
      cli_out("%s commands require fic or hybrid mode.\n",subcmd);
      return CMD_FAIL;
    }
#endif
    if (sal_strcasecmp(cmd,"create") == 0  ||
        sal_strcasecmp(cmd,"destroy") == 0 ||
        sal_strcasecmp(cmd,"add") == 0     ||
        sal_strcasecmp(cmd,"delete") == 0  ||
        sal_strcasecmp(cmd,"show") == 0 )  {
      /* dsgroup cmds */
      parse_table_init(unit,&pt);
      parse_table_add(&pt, "id", PQ_INT,
                      (void *)(-1),&dsgroupid, 0);

      if (sal_strcasecmp(cmd,"add") == 0 ||
          sal_strcasecmp(cmd,"delete") == 0 ){
        parse_table_add(&pt,"modid",PQ_INT,
                        (void *)(-1),&modid,0);

      }

      if (parse_arg_eq(a, &pt) < 0) {
        cli_out("%s: Invalid option: %s\n",
                ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
      }

      if (ARG_CNT(a) != 0) {
        cli_out("%s: extra options starting with \"%s\" \n",
                ARG_CMD(a), ARG_CUR(a));
        parse_arg_eq_done(&pt);
        return CMD_USAGE;
      }

      /* some cmds require certain arguments to be entered */
      if (sal_strcasecmp(cmd,"destroy") == 0 ||
          sal_strcasecmp(cmd,"show") == 0 ) {
        if (dsgroupid == -1) {
          cli_out("%s needs an id\n",cmd);
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }
      }

      if (sal_strcasecmp(cmd,"delete") == 0 ||
          sal_strcasecmp(cmd,"add") == 0 ) {
        if ((modid == -1 || dsgroupid == -1)) {
          cli_out("%s needs an id and modid\n",cmd);
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }
      }

      if (sal_strcasecmp(cmd, "create") == 0) {
        flags = (dsgroupid == -1) ? 0 : BCM_FABRIC_DISTRIBUTION_WITH_ID;
        rv = bcm_fabric_distribution_create(unit,flags,&dsgroupid);
        if (rv == CMD_OK) {
            cli_out("Created distribution group (0x%x)\n",
                    dsgroupid);
        }
      } else if (sal_strcasecmp(cmd ,"destroy") == 0) {
        rv = bcm_fabric_distribution_destroy(unit,dsgroupid);
        if (rv == CMD_OK) {
            cli_out("Destroyed distribution group(0x%x)\n",
                    dsgroupid);
        }
      } else if (sal_strcasecmp(cmd, "add") == 0) {
          sal_memset(distmodids,0,DIAG_CLI_MAX_NODES*sizeof(int));
          /* get modids that exists in this distribution group */
          rv = bcm_fabric_distribution_get(unit,dsgroupid,max_nodes,distmodids,&count);
          if (rv != CMD_OK) { parse_arg_eq_done(&pt); goto bcm_err; }
          /* check that this modid does not exist already in this distribution group */
          for (idx=0;idx<count;idx++) {
            if(distmodids[idx] == modid) {
              cli_out("This modid (%d) already exists in distribution group(%d)\n",modid,dsgroupid);
              parse_arg_eq_done(&pt);
              return CMD_FAIL;
            }
          }
          /* add this modid to the group */
          distmodids[count] = modid;
          rv = bcm_fabric_distribution_set(unit,dsgroupid,count+1,distmodids);
          if (rv != CMD_OK) { parse_arg_eq_done(&pt); goto bcm_err; }
      } else if (sal_strcasecmp(cmd,"delete") == 0) {
        sal_memset(distmodids,0,DIAG_CLI_MAX_NODES*sizeof(int));
        rv = bcm_fabric_distribution_get(unit,dsgroupid,max_nodes,distmodids,&count);
        if (rv != CMD_OK) { parse_arg_eq_done(&pt); goto bcm_err; }
        if (count == 0) {
          cli_out("No modids found in this distribution group. Use add to add some.\n");
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }
        dist_modids_p = sal_alloc((sizeof(int) * count), "updated eset group");
        if (dist_modids_p == NULL) {
          cli_out("Insufficent Memory\n");
          parse_arg_eq_done(&pt);
          return CMD_FAIL;
        }
        sal_memset(dist_modids_p,0,count*sizeof(int));
        for (idx=0;idx<count;idx++) {
          if (distmodids[idx] == modid) {
            bFoundModid = TRUE;
          } else {
            /* save the modids that are not getting deleted */
            *dist_modids_p++ = distmodids[idx];
          }
        }
        if (bFoundModid == FALSE) {
          cli_out("modid(%d) does not exist in distribution group(%d).\n",modid,dsgroupid);
          dist_modids_p -= count;
          retcode = CMD_FAIL;
          goto bcm_err;
        } else {
          dist_modids_p -= count-1;
          rv = bcm_fabric_distribution_set(unit,dsgroupid,count-1,dist_modids_p);
          if (rv != CMD_OK) { parse_arg_eq_done(&pt); goto bcm_err; }
        }
      } else if (sal_strcasecmp(cmd,"show") == 0) {
        sal_memset(distmodids,0,DIAG_CLI_MAX_NODES*sizeof(int));
        rv = bcm_fabric_distribution_get(unit,dsgroupid,max_nodes,distmodids,&count);
        if (rv != CMD_OK) { parse_arg_eq_done(&pt); goto bcm_err; }
        cli_out("\n Distribution group     Modids\n");
        cli_out("---------------------------------\n");
        for (idx=0;idx<count;idx++) {
          if (idx == 0) {
            cli_out("%10d\t\t%5d\n",dsgroupid,distmodids[idx]);
          } else {
            cli_out("\t\t\t%5d\n",distmodids[idx]);
          }
        }
        cli_out("Total entries = %d\n",count);
      }
      parse_arg_eq_done(&pt);
    } else {
      cli_out(" ==> Invalid distribution cmd (%s)\n",cmd);
      return CMD_USAGE;
    }
  } else {
    return CMD_USAGE;
  }

 bcm_err:
  if (dist_modids_p != NULL) {
    sal_free(dist_modids_p);
  }

  if (BCM_FAILURE(rv)) {
    if (rv == BCM_E_UNAVAIL) {
      cli_out("%s on %s\n",bcm_errmsg(rv),SOC_CHIP_STRING(unit));
    } else {
      cli_out("ERROR: %s failed for %s[u:%d] (%s)\n",cmd,SOC_CHIP_STRING(unit),
              unit,bcm_errmsg(rv));
      return CMD_FAIL;
    }
  }
  return retcode;
}

#ifdef BCM_BM9600_SUPPORT

/*
 ***************************************************
 *
 * Polaris LC
 *
 ***************************************************
 */
dev_cmd_rec polaris_lc_cmd_tbl[] = {

/*
 * module ids for QEs are the SCI serdes port
 */
        {SET_LGL_MODID,   PLLC_UNIT_FE0,  (int)&PL1_FE0_MODID, 0, 0, 0},
        {SET_LGL_MODID,   PLLC_UNIT_FE1,  (int)&PL1_FE1_MODID, 0, 0, 0},

        {SET_LGL_MODID,   PLLC_UNIT_QE0,  (int)&PL_QE0_MODID, CMD_CARD_PLLC_REV0, 0, 0},
        {SET_LGL_MODID,   PLLC_UNIT_QE1,  (int)&PL_QE1_MODID, CMD_CARD_PLLC_REV0, 0, 0},
        {SET_LGL_MODID,   PLLC_UNIT_QE0,  (int)&PL1_QE0_MODID, CMD_CARD_PLLC_REV1, 0, 0},
        {SET_LGL_MODID,   PLLC_UNIT_QE1,  (int)&PL1_QE1_MODID, CMD_CARD_PLLC_REV1, 0, 0},

        { SET_LGL_MODULE_PROTOCOL, PLLC_UNIT_BM96, (int)&PL_QE1_MODID, bcmModuleProtocol1, CMD_CARD_PLLC_REV0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PLLC_UNIT_BM96, (int)&PL_QE0_MODID, bcmModuleProtocol1, CMD_CARD_PLLC_REV0, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PLLC_UNIT_BM96, (int)&PL1_QE1_MODID, bcmModuleProtocol1, CMD_CARD_PLLC_REV1, 0, 0 },
        { SET_LGL_MODULE_PROTOCOL, PLLC_UNIT_BM96, (int)&PL1_QE0_MODID, bcmModuleProtocol1, CMD_CARD_PLLC_REV1, 0, 0 },

        {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
        {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE, 0, 0},

        {PORT_ENABLE_RANGE, PLLC_UNIT_QE1, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
        {PORT_ENABLE_RANGE, PLLC_UNIT_QE1, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE, 0, 0},

        { PORT_CONTROL, PLLC_UNIT_BM96, 24, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc3 QE0 */
        { PORT_CONTROL, PLLC_UNIT_BM96, 27, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI lc3 QE1 */
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 4, 21, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 40,55, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },
        { PORT_CONTROL_RANGE, PLLC_UNIT_BM96, 58,59, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96, 4,  21, 0, 0},   /* SFIs to QE0  */
        {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96, 40, 55, 0, 0},   /* SFIs to QE1  */
        {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96, 58, CMD_CARD_PLLC_REV1, 0, 0},   /* SFIs to QE0  */
        {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96, 59, CMD_CARD_PLLC_REV1, 0, 0},   /* SFIs to QE0  */

        {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96, 22, CMD_CARD_PLLC_REV0, 0, 0},   /* SCIs to QE0  */
        {PORT_ENABLE_SI,        PLLC_UNIT_BM96, 22, CMD_CARD_PLLC_REV0, 0, 0},   /* SCIs to QE0 SI enable */
        {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96, 57, CMD_CARD_PLLC_REV0, 0, 0},   /* SCIs to QE1  */
        {PORT_ENABLE_SI,        PLLC_UNIT_BM96, 57, CMD_CARD_PLLC_REV0, 0, 0},   /* SCIs to QE1 SI enable */

        {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96, 24, CMD_CARD_PLLC_REV1, 0, 0},   /* SCIs to QE0  */
        {PORT_ENABLE_SI,        PLLC_UNIT_BM96, 24, CMD_CARD_PLLC_REV1, 0, 0},   /* SCIs to QE0 SI enable */
        {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96, 27, CMD_CARD_PLLC_REV1, 0, 0},   /* SCIs to QE1  */
        {PORT_ENABLE_SI,        PLLC_UNIT_BM96, 27, CMD_CARD_PLLC_REV1, 0, 0},   /* SCIs to QE1 SI enable */

        {PORT_ENABLE_SI_RANGE, PLLC_UNIT_BM96, 4,  21, 0, 0},   /* SFIs to QE0  */
        {PORT_ENABLE_SI_RANGE, PLLC_UNIT_BM96, 40, 55, 0, 0},   /* SFIs to QE1  */
        {PORT_ENABLE_SI,       PLLC_UNIT_BM96, 58, CMD_CARD_PLLC_REV1, 0, 0},   /* SFIs to QE0  */
        {PORT_ENABLE_SI,       PLLC_UNIT_BM96, 59, CMD_CARD_PLLC_REV1, 0, 0},   /* SFIs to QE0  */

/*
 * crossbar data resides in BM96 instead of QEs
 */

/* NOTE: **********  REV 0 LC */
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  0, (int)&PL_QE1_MODID, 40, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  1, (int)&PL_QE1_MODID, 41, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  2, (int)&PL_QE1_MODID, 42, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  3, (int)&PL_QE1_MODID, 43, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  4, (int)&PL_QE1_MODID, 44, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  5, (int)&PL_QE1_MODID, 45, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  6, (int)&PL_QE1_MODID, 46, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  7, (int)&PL_QE1_MODID, 47, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  8, (int)&PL_QE1_MODID, 51, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  9, (int)&PL_QE1_MODID, 50, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 10, (int)&PL_QE1_MODID, 49, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 11, (int)&PL_QE1_MODID, 48, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 12, (int)&PL_QE1_MODID, 55, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 13, (int)&PL_QE1_MODID, 54, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 14, (int)&PL_QE1_MODID, 53, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 15, (int)&PL_QE1_MODID, 52, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 16, (int)&PL_QE1_MODID, 59, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 17, (int)&PL_QE1_MODID, 58, CMD_CARD_PLLC_REV0, 0},

        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  0, (int)&PL_QE0_MODID,  4, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  1, (int)&PL_QE0_MODID,  5, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  2, (int)&PL_QE0_MODID,  6, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  3, (int)&PL_QE0_MODID,  7, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  4, (int)&PL_QE0_MODID,  8, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  5, (int)&PL_QE0_MODID,  9, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  6, (int)&PL_QE0_MODID, 10, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  7, (int)&PL_QE0_MODID, 11, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  8, (int)&PL_QE0_MODID, 12, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  9, (int)&PL_QE0_MODID, 13, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 10, (int)&PL_QE0_MODID, 14, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 11, (int)&PL_QE0_MODID, 15, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 12, (int)&PL_QE0_MODID, 16, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 13, (int)&PL_QE0_MODID, 17, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 14, (int)&PL_QE0_MODID, 18, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 15, (int)&PL_QE0_MODID, 19, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 16, (int)&PL_QE0_MODID, 20, CMD_CARD_PLLC_REV0, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 17, (int)&PL_QE0_MODID, 21, CMD_CARD_PLLC_REV0, 0},

        /* NOTE *******  REV1 LC */

        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  0, (int)&PL1_QE1_MODID, 40, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  1, (int)&PL1_QE1_MODID, 41, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  2, (int)&PL1_QE1_MODID, 42, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  3, (int)&PL1_QE1_MODID, 43, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  4, (int)&PL1_QE1_MODID, 44, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  5, (int)&PL1_QE1_MODID, 45, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  6, (int)&PL1_QE1_MODID, 46, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  7, (int)&PL1_QE1_MODID, 47, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  8, (int)&PL1_QE1_MODID, 51, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  9, (int)&PL1_QE1_MODID, 50, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 10, (int)&PL1_QE1_MODID, 49, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 11, (int)&PL1_QE1_MODID, 48, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 12, (int)&PL1_QE1_MODID, 55, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 13, (int)&PL1_QE1_MODID, 54, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 14, (int)&PL1_QE1_MODID, 53, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 15, (int)&PL1_QE1_MODID, 52, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 17, (int)&PL1_QE1_MODID, 59, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 16, (int)&PL1_QE1_MODID, 58, CMD_CARD_PLLC_REV1, 0},

        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  0, (int)&PL1_QE0_MODID,  4, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  1, (int)&PL1_QE0_MODID,  5, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  2, (int)&PL1_QE0_MODID,  6, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  3, (int)&PL1_QE0_MODID,  7, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  4, (int)&PL1_QE0_MODID,  8, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  5, (int)&PL1_QE0_MODID,  9, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  6, (int)&PL1_QE0_MODID, 10, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  7, (int)&PL1_QE0_MODID, 11, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  8, (int)&PL1_QE0_MODID, 12, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96,  9, (int)&PL1_QE0_MODID, 13, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 10, (int)&PL1_QE0_MODID, 14, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 11, (int)&PL1_QE0_MODID, 15, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 12, (int)&PL1_QE0_MODID, 16, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 13, (int)&PL1_QE0_MODID, 17, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 14, (int)&PL1_QE0_MODID, 18, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 15, (int)&PL1_QE0_MODID, 19, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 16, (int)&PL1_QE0_MODID, 20, CMD_CARD_PLLC_REV1, 0},
        {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96, 17, (int)&PL1_QE0_MODID, 21, CMD_CARD_PLLC_REV1, 0},

        /* commit all the above _ADD's */
        { XBAR_CONNECT_UPDATE, PLLC_UNIT_BM96, 0, 0, 0, 0, 0},


        {FAB_CONTROL_SET, PLLC_UNIT_BM96, bcmFabricArbiterId,          0, 0, 0, 0},
        {FAB_CONTROL_SET, PLLC_UNIT_BM96, bcmFabricMaximumFailedLinks, 3, 0, 0, 0},
        {FAB_CONTROL_SET, PLLC_UNIT_BM96, bcmFabricActiveArbiterId,    0, 0, 0, 0},
        {FAB_CONTROL_SET, PLLC_UNIT_BM96, bcmFabricRedundancyMode,     0, 0, 0, 0},

        {FAB_CONTROL_SET, PLLC_UNIT_QE0, bcmFabricActiveArbiterId,    0, 0, 0, 0},
        {FAB_CONTROL_SET, PLLC_UNIT_QE0, bcmFabricRedundancyMode,     0, 0, 0, 0},

        {FAB_CONTROL_SET, PLLC_UNIT_QE1, bcmFabricActiveArbiterId,    0, 0, 0, 0},
        {FAB_CONTROL_SET, PLLC_UNIT_QE1, bcmFabricRedundancyMode,     0, 0, 0, 0},
/*
        {REDUND_REGISTER, PLLC_UNIT_LCM0, 0,                           0, 0, 0, 0},
*/
        {XBAR_ENABLE_SET,   PLLC_UNIT_QE0, 0x3ffff, 0, 0, 0, 0},
        {XBAR_ENABLE_SET,   PLLC_UNIT_QE1, 0x3ffff, 0, 0, 0, 0},
        {XBAR_ENABLE_SET,   PLLC_UNIT_BM96, 0x3ffff, 0, 0, 0, 0},
/* turn on PRBS to test */
/*
        {PORT_CONTROL, PLLC_UNIT_QE0,  5, bcmPortControlPrbsTxEnable, 1, 0},
        {PORT_CONTROL, PLLC_UNIT_BM96, 9, bcmPortControlPrbsRxEnable, 1, 0},
*/
/*
        {LGL_MODULE_ENABLE_SET, PLLC_UNIT_QE0, (int)&PL_QE0_MODID, 1, 0, 0, 0},
        {LGL_MODULE_ENABLE_SET, PLLC_UNIT_QE1, (int)&PL_QE1_MODID, 1, 0, 0, 0},
*/

        {LGL_MODULE_ENABLE_SET, PLLC_UNIT_BM96, (int)&PL_QE0_MODID, 1, CMD_CARD_PLLC_REV0, 0, 0},
        {LGL_MODULE_ENABLE_SET, PLLC_UNIT_BM96, (int)&PL_QE1_MODID, 1, CMD_CARD_PLLC_REV0, 0, 0},
        {LGL_MODULE_ENABLE_SET, PLLC_UNIT_BM96, (int)&PL1_QE0_MODID, 1, CMD_CARD_PLLC_REV1, 0, 0},
        {LGL_MODULE_ENABLE_SET, PLLC_UNIT_BM96, (int)&PL1_QE1_MODID, 1, CMD_CARD_PLLC_REV1, 0, 0},

        {-1, 0, 0, 0, 0, 0, 0},
        {-1, 1, 0, 0, 0, 0, 0},
        {-1, 2, 0, 0, 0, 0, 0},
        {-1, 3, 0, 0, 0, 0, 0},
};

dev_cmd_rec fe2kxt_lc_cmd_tbl[] = {
    /*
     * module ids for QEs are the SCI serdes port
     */

    {SET_LGL_MODID,   PLLC_UNIT_FE0, (int)&PL1_FE0_MODID, 0, 0, 0},

    {SET_LGL_MODID,   PLLC_UNIT_QE0,  (int)&PL1_QE0_MODID, CMD_CARD_ALL, 0, 0},

    { SET_LGL_MODULE_PROTOCOL, PLLC_UNIT_BM96_FE2KXT_LC, (int)&PL1_QE0_MODID, bcmModuleProtocol1, CMD_CARD_ALL, 0, 0 },

    {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
    {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE, 0, 0},

    { PORT_CONTROL, PLLC_UNIT_BM96_FE2KXT_LC, 24, bcmPortControlAbility, BCM_PORT_ABILITY_SCI, 0 }, /* SCI  QE0 */
    { PORT_CONTROL_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 4, 21, bcmPortControlAbility, BCM_PORT_ABILITY_SFI, 0 },

    {PORT_ENABLE_HYPERCORE_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 4,  21, 0, 0},   /* SFIs to QE0  */
    {PORT_ENABLE_SI_RANGE, PLLC_UNIT_BM96_FE2KXT_LC, 4,  21, 0, 0},   /* SFIs to QE0  */

    {PORT_ENABLE_HYPERCORE, PLLC_UNIT_BM96_FE2KXT_LC, 24, CMD_CARD_ALL, 0, 0},   /* SCIs to QE0  */
    {PORT_ENABLE_SI, PLLC_UNIT_BM96_FE2KXT_LC, 24, CMD_CARD_ALL, 0, 0},   /* SCIs to QE0  */

    /*
     * crossbar data resides in BM96 instead of QEs
     */

    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  0, (int)&PL1_QE0_MODID,  4, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  1, (int)&PL1_QE0_MODID,  5, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  2, (int)&PL1_QE0_MODID,  6, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  3, (int)&PL1_QE0_MODID,  7, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  4, (int)&PL1_QE0_MODID,  8, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  5, (int)&PL1_QE0_MODID,  9, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  6, (int)&PL1_QE0_MODID, 10, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  7, (int)&PL1_QE0_MODID, 11, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  8, (int)&PL1_QE0_MODID, 12, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC,  9, (int)&PL1_QE0_MODID, 13, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 10, (int)&PL1_QE0_MODID, 14, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 11, (int)&PL1_QE0_MODID, 15, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 12, (int)&PL1_QE0_MODID, 16, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 13, (int)&PL1_QE0_MODID, 17, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 14, (int)&PL1_QE0_MODID, 18, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 15, (int)&PL1_QE0_MODID, 19, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 16, (int)&PL1_QE0_MODID, 20, CMD_CARD_ALL, 0},
    {XBAR_LGL_CONNECT_ADD, PLLC_UNIT_BM96_FE2KXT_LC, 17, (int)&PL1_QE0_MODID, 21, CMD_CARD_ALL, 0},

    /* commit all the above _ADD's */
    { XBAR_CONNECT_UPDATE, PLLC_UNIT_BM96_FE2KXT_LC, 0, 0, 0, 0, 0},


    {FAB_CONTROL_SET, PLLC_UNIT_BM96_FE2KXT_LC, bcmFabricArbiterId,          0, 0, 0, 0},
    {FAB_CONTROL_SET, PLLC_UNIT_BM96_FE2KXT_LC, bcmFabricMaximumFailedLinks, 3, 0, 0, 0},
    {FAB_CONTROL_SET, PLLC_UNIT_BM96_FE2KXT_LC, bcmFabricActiveArbiterId,    0, 0, 0, 0},
    {FAB_CONTROL_SET, PLLC_UNIT_BM96_FE2KXT_LC, bcmFabricRedundancyMode,     0, 0, 0, 0},

    {FAB_CONTROL_SET, PLLC_UNIT_QE0, bcmFabricActiveArbiterId,    0, 0, 0, 0},
    {FAB_CONTROL_SET, PLLC_UNIT_QE0, bcmFabricRedundancyMode,     0, 0, 0, 0},
    /*
      {REDUND_REGISTER, PLLC_UNIT_LCM0, 0,                           0, 0, 0, 0},
    */
    {XBAR_ENABLE_SET,   PLLC_UNIT_QE0, 0x3ffff, 0, 0, 0, 0},
    {XBAR_ENABLE_SET,   PLLC_UNIT_BM96_FE2KXT_LC, 0x3ffff, 0, 0, 0, 0},
    /* turn on PRBS to test */
    /*
      {PORT_CONTROL, PLLC_UNIT_QE0,  5, bcmPortControlPrbsTxEnable, 1, 0},
      {PORT_CONTROL, PLLC_UNIT_BM96_FE2KXT_LC, 9, bcmPortControlPrbsRxEnable, 1, 0},
    */
    /*
      {MODULE_ENABLE_SET, PLLC_UNIT_QE0, PL_QE0_MODID, 1, 0, 0, 0},
    */

    {SWITCH_EVENT_REGISTER, PLLC_UNIT_QE0, 0, 0, 0, 0},

    {LGL_MODULE_ENABLE_SET, PLLC_UNIT_BM96_FE2KXT_LC, (int)&PL1_QE0_MODID, 1, CMD_CARD_ALL, 0, 0},

    {-1, 0, 0, 0, 0, 0, 0},
    {-1, 1, 0, 0, 0, 0, 0},
    {-1, 2, 0, 0, 0, 0, 0},
    {-1, 3, 0, 0, 0, 0, 0},
};

dev_cmd_rec qe2k_bscrn_lc_cmd_tbl[] = {
    /*
     * module ids for QEs are the SCI serdes port
     */

    {SET_LGL_MODID,   PLLC_UNIT_QE0,  (int)&PL_QE0_MODID,  CMD_CARD_ALL, 0, 0},
    {SET_LGL_MODID,   PLLC_UNIT_QE1,  (int)&PL_QE1_MODID,  CMD_CARD_ALL, 0, 0},
    {SET_LGL_MODID,   PLLC_UNIT_QE0,  (int)&PL1_QE0_MODID, CMD_CARD_ALL, 0, 0},
    {SET_LGL_MODID,   PLLC_UNIT_QE1,  (int)&PL1_QE1_MODID, CMD_CARD_ALL, 0, 0},

    /* {SET_LGL_MODID,   PLLC_UNIT_QE0,  (int)&PL1_QE0_MODID, CMD_CARD_ALL, 0, 0}, */


    {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
    {PORT_ENABLE_RANGE, PLLC_UNIT_QE0, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE, 0, 0},

    {PORT_ENABLE_RANGE, PLLC_UNIT_QE1, QE2000_SFI_PORT_BASE, QE2000_SFI_PORT_BASE + 17, 0, 0},
    {PORT_ENABLE_RANGE, PLLC_UNIT_QE1, QE2000_SCI_PORT_BASE, QE2000_SCI_PORT_BASE, 0, 0},


    {FAB_CONTROL_SET, PLLC_UNIT_QE0, bcmFabricActiveArbiterId,    0, 0, 0, 0},
    {FAB_CONTROL_SET, PLLC_UNIT_QE0, bcmFabricRedundancyMode,     0, 0, 0, 0},

    {FAB_CONTROL_SET, PLLC_UNIT_QE1, bcmFabricActiveArbiterId,    0, 0, 0, 0},
    {FAB_CONTROL_SET, PLLC_UNIT_QE1, bcmFabricRedundancyMode,     0, 0, 0, 0},
    /*
    */
    {XBAR_ENABLE_SET,   PLLC_UNIT_QE0, 0x3ffff, 0, 0, 0, 0},
    {XBAR_ENABLE_SET,   PLLC_UNIT_QE1, 0x3ffff, 0, 0, 0, 0},

    {-1, 0, 0, 0, 0, 0, 0},
    {-1, 1, 0, 0, 0, 0, 0},
};

#endif /* BCM_BM9600_SUPPORT */

cmd_result_t
cmd_sbx_card_config(int unit, int node_id, int node_id1, int link_enable, dev_cmd_rec cmd_tbl[])
{
    int i, rv, pval,mask_shift, mask_bit;
    uint32 port;
    int enable_mask, sfi_port;
    int xbar, index;
    bcm_error_t bcm_err;
    dev_cmd_rec *pcmd;
    bcm_port_config_t config;
    uint64 uuVal;

    for (i=0;i<_XCFG_MAX_NODES;i++){
        for (port=0;port<_XCFG_MAX_XBARS;port++){
            _xcfg_remap[port][i] = -1;
        }
    }

    cli_out("*****  sbx_card_config:   node_id=%d node_id1=%d links=%x ndev=%d \n",node_id, node_id1, link_enable, soc_ndev);

    for (i = 0; i < MAX_CMDS; i++){

        bcm_err = BCM_E_NONE;

        pcmd = &cmd_tbl[i];

        if (pcmd->unit > 5) {
            cli_out("INVALID UNIT CMD idx=%d  args: cmd=%d unit=%d arg1=%d  arg2=%d  arg3=%d arg4=%d\n",i,
                    pcmd->cmd, pcmd->unit, pcmd->arg1,
                    pcmd->arg2, pcmd->arg3,pcmd->arg4);
            return BCM_E_NONE;
        }
        
        /* added SOC_SBX_INIT check for systems with known bad hw */
        if (!BCM_UNIT_VALID(pcmd->unit) || !SOC_SBX_INIT(pcmd->unit)) {
            cli_out("SKIPPING COMMAND for UNIT %d\n", pcmd->unit);
            LOG_VERBOSE(BSL_LS_APPL_COMMON,
                        (BSL_META_U(unit,
                                    "DEBUG: [%s:%d]: SKIPPING COMMAND for UNIT %d\n"),
                         FUNCTION_NAME(), __LINE__, pcmd->unit));
        }
        else {
/*
  cli_out("COMMAND idx=%d  args: cmd=%d unit=%d arg1=%d  arg2=%d  arg3=%d arg4=%d\n",i,
          pcmd->cmd, pcmd->unit, pcmd->arg1,
          pcmd->arg2, pcmd->arg3,pcmd->arg4);
*/
            switch(pcmd->cmd){
            case PORT_ENABLE:
            case PORT_ENABLE_SI: /* for BM9600, this enables the SI block */
            {
                int card_id = pcmd->arg2;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (!skip_command)
                    bcm_err = bcm_port_enable_set(pcmd->unit, pcmd->arg1, 1);
                break;
            }

            case PORT_ENABLE_HYPERCORE: /* for BM9600, this enables the hypercore rx and tx block */
            {
                int card_id = pcmd->arg2;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (!skip_command) {
                    bcm_err = bcm_port_control_set(pcmd->unit, pcmd->arg1, bcmPortControlRxEnable, 1);
                    if (bcm_err != BCM_E_NONE) {
                        cli_out("Port Enable Hypercore Rx Command failed port=%d  err=%d\n",port, bcm_err);
                        return CMD_FAIL;
                    }
                    bcm_err = bcm_port_control_set(pcmd->unit, pcmd->arg1, bcmPortControlTxEnable, 1);
                    if (bcm_err != BCM_E_NONE) {
                        cli_out("Port Enable Hypercore Tx Command failed port=%d  err=%d\n",port, bcm_err);
                        return CMD_FAIL;
                    }
                }
                break;
            }
            case PORT_EQUALIZATION:
            {
                int card_id = pcmd->arg2;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (!skip_command)
                    bcm_err = bcm_port_control_set(pcmd->unit, pcmd->arg1, bcmPortControlSerdesDriverEqualization, pcmd->arg3);
                break;
            }
            case SET_MODID:
            {
                int card_id = pcmd->arg2;
                int skip_command=0;
                int target_unit, target_modid;

                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;

                target_unit = pcmd->unit;
                if(pcmd->arg1 >= 0) {
                    target_modid = pcmd->arg1;
                } else {
                    /* auto modid */
                    if (pcmd->arg1 == AUTO_MODID_QE0) {
                        target_modid =  BCM_STK_NODE_TO_MOD(node_id);
                    } else if (pcmd->arg1 == AUTO_MODID_QE1) {
                        target_modid =  BCM_STK_NODE_TO_MOD(node_id1);
                    } else if (pcmd->arg1 == AUTO_MODID_FE0) {
                        /* use node_id as fe0 module id */
                        target_modid = node_id % 32;
                    } else if (pcmd->arg1 == AUTO_MODID_FE1) {
                        /* use node_id1 as fe0 module id */
                        target_modid = node_id1 % 32;
                    } else {
                        target_modid = pcmd->arg1;
                    }
                }

                bcm_err = bcm_stk_modid_set(target_unit, target_modid);

                /* set the cpu destination on for the FE to point to the
                 * QE's cpu port
                 */

                break;
            }

            case SET_LGL_MODID:
            {
                int card_id = pcmd->arg2;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if(*((int *)(pcmd->arg1)) >= 0) {
                    bcm_err = bcm_stk_modid_set(pcmd->unit, *((int *)(pcmd->arg1)));
                } else {
                    /* auto modid */
                    if (*((int *)(pcmd->arg1)) == AUTO_MODID_QE0) {
                        bcm_err =bcm_stk_modid_set(pcmd->unit, node_id + 10000);
                    } else if (*((int *)(pcmd->arg1)) == AUTO_MODID_QE1) {
                        bcm_err =bcm_stk_modid_set(pcmd->unit, node_id1 + 10000);
                    } else if (*((int *)(pcmd->arg1)) == AUTO_MODID_FE0) {
                        /* use node_id as fe0 module id */
                        bcm_err =bcm_stk_modid_set(pcmd->unit, node_id%32 );
                    } else if (*((int *)(pcmd->arg1)) == AUTO_MODID_FE1) {
                        /* use node_id1 as fe0 module id */
                        bcm_err =bcm_stk_modid_set(pcmd->unit, node_id1%32 );
                    } else {
                        bcm_err =bcm_stk_modid_set(pcmd->unit, *((int *)(pcmd->arg1)) );
                    }
                }
                break;
            }

            case XBAR_CONNECT_ADD:
            {
                int node = pcmd->arg2 - BCM_MODULE_FABRIC_BASE;
                int card_id = pcmd->arg4;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if (pcmd->arg1>=_XCFG_MAX_XBARS || node >= _XCFG_MAX_NODES || node <0){
                    bcm_err = BCM_E_PARAM;
                    break;
                }
                _xcfg_remap[pcmd->arg1][node] = pcmd->arg3;
            }
            break;

            case XBAR_LGL_CONNECT_ADD:
            {
                int node = *((int *)(pcmd->arg2)) - BCM_MODULE_FABRIC_BASE;
                int card_id = pcmd->arg4;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if (pcmd->arg1>=_XCFG_MAX_XBARS || pcmd->arg1 < 0 || node >= _XCFG_MAX_NODES || node <0){
                    bcm_err = BCM_E_PARAM;
                    break;
                }
                _xcfg_remap[pcmd->arg1][node] = pcmd->arg3;
            }
            break;

	    case LGL_XBAR_MAPPING_SET:
	    {
	      /* arg1=modid arg2=switch fabric arbiter id arg3=lgl xbar arg4=port */
	      bcm_err = bcm_fabric_crossbar_mapping_set(pcmd->unit, pcmd->arg1,
							pcmd->arg2, pcmd->arg3,
							pcmd->arg4);
	    }
	    break;


            case XBAR_CONNECT_UPDATE:
            {
                int src,dst;
                bcm_err = BCM_E_NONE;
#ifdef SBX_DEBUG_CONNECTION_SET
		uint64 xbars;
#endif
                for (src=0;src<_XCFG_MAX_NODES;src++){
                    for (dst=0;dst<_XCFG_MAX_NODES;dst++){
                        for (port=0;port<_XCFG_MAX_XBARS;port++){
                            if (_xcfg_remap[port][src] != -1 && _xcfg_remap[port][dst] != -1){
                                rv = bcm_fabric_crossbar_connection_set(pcmd->unit, port,
                                                                        src+BCM_MODULE_FABRIC_BASE, _xcfg_remap[port][src],
                                                                        dst+BCM_MODULE_FABRIC_BASE, _xcfg_remap[port][dst]);
                                if (rv != BCM_E_NONE && bcm_err == BCM_E_NONE) {
				    cli_out("xbar_update port(%d) error returned(%d)\n", port, bcm_err);
                                    bcm_err = rv;
				}

#ifdef SBX_DEBUG_CONNECTION_SET
				cli_out("src_node(%d) port(%d) dest_node(%d) port(%d)(0x%0x)\n", src, _xcfg_remap[port][src],
                                        dst, _xcfg_remap[port][dst], _xcfg_remap[port][dst]);

				rv = bcm_fabric_crossbar_connection_get(pcmd->unit, port,
									src+BCM_MODULE_FABRIC_BASE, _xcfg_remap[port][src],
									dst+BCM_MODULE_FABRIC_BASE, &_xcfg_remap_read);
   
                              if (rv != BCM_E_NONE && bcm_err == BCM_E_NONE) {
				    cli_out("xbar_update port(%d) read error returned(%d)\n", port, bcm_err);
                                    bcm_err = rv;
				}

				cli_out("src_node(%d) port(%d) dest_node(%d) read port(%d)(0x%0x)\n", src, _xcfg_remap[port][src],
                                        dst, _xcfg_remap_read, _xcfg_remap_read);

				rv = bcm_fabric_crossbar_connection_status_get(pcmd->unit, 
									       src+BCM_MODULE_FABRIC_BASE,
									       dst+BCM_MODULE_FABRIC_BASE,
									       bcmFabricXbarConnectionModeC,
									       &xbars);
				if (rv) {
				  cli_out("error returned from connection_status_get() (%d)\n", rv);
				}
				cli_out("xbars_lo=0x%08x xbars_hi=0x%08x\n", (int) xbars, (int)(xbars>>32));
#endif				
                            }
                        }/* port */
                    }/* dst */
                }/* src */
            }
            break;
            case XBAR_CONNECT_SET:
                /* set protocol for modules used by xbar */
                bcm_err = bcm_fabric_crossbar_connection_set(pcmd->unit, pcmd->arg1,
                                                             pcmd->arg2, pcmd->arg3,
                                                             pcmd->arg4, pcmd->arg5);
                break;
                /* arg1=start, arg2=end, arg3=src_modid, arg4=portstart, arg5=dst_modid, arg6=dstport_start*/
            case XBAR_CONNECT_RANGE:
                /* set range of xbars with incrementing src/dst ports  */
                index = 0;
                for(xbar = pcmd->arg1; xbar <= pcmd->arg2; xbar++) {
                    bcm_err = bcm_fabric_crossbar_connection_set(pcmd->unit, xbar,
                                                                 pcmd->arg3 + node_id, pcmd->arg4 + index,
                                                                 /*  pcmd->arg3, pcmd->arg4,  if QE, src port is zero + index, */
                                                                 pcmd->arg5, pcmd->arg6 + index);
                    cli_out("---xbar_connection_set unit=%d Xbar=%d  modid=%d  port=%d  dstmodid=%d  dstport=%d \n",
                            pcmd->unit, xbar,pcmd->arg3, pcmd->arg4 + index,
                            pcmd->arg5, pcmd->arg6 + index);
                    index++;
                    if (bcm_err != BCM_E_NONE) {
                        cli_out("Xbar connection set failed index=%d xbar=%d  err=%d\n",
                                i,xbar, bcm_err);
                    }
                }
                break;

            case FAB_CONTROL_SET:
                bcm_err = bcm_fabric_control_set(pcmd->unit, pcmd->arg1,
                                                 pcmd->arg2);
                break;
            case XBAR_ENABLE_SET:
                if(link_enable > 0)     /* if input param is non-zero, use that */
		  enable_mask = link_enable;
                else                    /* else use value from command table */
		  enable_mask = pcmd->arg1;
		/*               cli_out("--- xbar_enable unit=%d  mask=%x \n",pcmd->unit,enable_mask); */
                COMPILER_64_SET(uuVal, 0, enable_mask);
	      bcm_err = bcm_fabric_crossbar_enable_set(pcmd->unit, uuVal);
                break;
            case MODULE_ENABLE_SET:
            {
                int card_id = pcmd->arg3;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if(pcmd->arg1 > 0) {
                    bcm_err = bcm_stk_module_enable(pcmd->unit, pcmd->arg1, -1, TRUE);
                } else {
                    /* auto modid */
                    if (pcmd->arg1 == AUTO_MODID_QE0) {
                        bcm_err = bcm_stk_module_enable(pcmd->unit, node_id + 10000, -1, TRUE);
                    } else if (pcmd->arg1 == AUTO_MODID_QE1) {
                        bcm_err = bcm_stk_module_enable(pcmd->unit, node_id1 + 10000, -1, TRUE);
                    } else {
                        cli_out("ModuleEnableSet failed: unit=%d node_id=%d node_id1=%d arg1=%d\n",
                                pcmd->unit, node_id, node_id1, pcmd->arg1);
                        return CMD_FAIL;
                    }
                }
                break;
            }

            case LGL_MODULE_ENABLE_SET:
            {
                int card_id = pcmd->arg3;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if(*((int *)(pcmd->arg1)) > 0) {
                    bcm_err = bcm_stk_module_enable(pcmd->unit, *((int *)(pcmd->arg1)), -1, TRUE);
                } else {
                    /* auto modid */
                    if (*((int *)(pcmd->arg1)) == AUTO_MODID_QE0) {
                        bcm_err = bcm_stk_module_enable(pcmd->unit, node_id + 10000, -1, TRUE);
                    } else if (*((int *)(pcmd->arg1)) == AUTO_MODID_QE1) {
                        bcm_err = bcm_stk_module_enable(pcmd->unit, node_id1 + 10000, -1, TRUE);
                    } else {
                        cli_out("ModuleEnableSet failed: unit=%d node_id=%d node_id1=%d arg1=%d\n",
                                pcmd->unit, node_id, node_id1, *((int *)(pcmd->arg1)));
                        return CMD_FAIL;
                    }
                }
                break;
            }

            case REDUND_REGISTER:
                bcm_err = bcm_fabric_control_redundancy_register(pcmd->unit,
                                                                 sbx_handle_fabric_redundancy_event);
                break;


	    case SWITCH_EVENT_REGISTER:
#ifdef BCM_QE2000_SUPPORT
		bcm_err = bcm_switch_event_register(pcmd->unit, /* unit */ 
						    sbx_handle_switch_event,
						    NULL);
#endif
		break;

            case PORT_ENABLE_RANGE:
            case PORT_ENABLE_SI_RANGE: /* for BM9600, this enables the SI port */
                for(port = pcmd->arg1; port <= pcmd->arg2; port++) {
                    sfi_port = soc_property_port_get(pcmd->unit,  port, spn_PORT_IS_SFI, 0);
                    if((sfi_port) && SOC_IS_SBX_QE2000(pcmd->unit)){ /* filter QE ports w/ link_enable bits */
                        mask_shift = port - QE2000_SFI_PORT_BASE;
                        mask_bit = 1 << mask_shift;
                    }
                    else {   /* let all other ports through for now */
                        mask_bit = 0xffff;
                    }
                    if(link_enable & mask_bit) {

                        bcm_err = bcm_port_enable_set(pcmd->unit, port, 1);
                        if (bcm_err != BCM_E_NONE) {
                            cli_out("Port Enable Command failed index=%d port=%d  err=%d\n",
                                    i,port, bcm_err);
                            return CMD_FAIL;
                        }
                    }
                    else {
                        cli_out("--- FILTER   port unit=%d  port=%d\n",pcmd->unit,port);
                    }
                }
                break;
            case PORT_ENABLE_HYPERCORE_RANGE: /* for BM9600 ports */
                for(port = pcmd->arg1; port <= pcmd->arg2; port++) {
                    sfi_port = soc_property_port_get(pcmd->unit,  port, spn_PORT_IS_SFI, 0);
                    if((sfi_port) && SOC_IS_SBX_QE2000(pcmd->unit)){ /* filter QE ports w/ link_enable bits */
                        mask_shift = port - QE2000_SFI_PORT_BASE;
                        mask_bit = 1 << mask_shift;
                    }
                    else {   /* let all other ports through for now */
                        mask_bit = 0xffff;
                    }
                    if (link_enable & mask_bit) {

                        bcm_err = bcm_port_control_set(pcmd->unit, port, bcmPortControlRxEnable, 1);
                        if (bcm_err != BCM_E_NONE) {
                            cli_out("Port Enable Hypercore Rx Command failed index=%d port=%d  err=%d\n",
                                    i,port, bcm_err);
                            return CMD_FAIL;
                        }

                        bcm_err = bcm_port_control_set(pcmd->unit, port, bcmPortControlTxEnable, 1);
                        if (bcm_err != BCM_E_NONE) {
                            cli_out("Port Enable Hypercore Tx Command failed index=%d port=%d  err=%d\n",
                                    i,port, bcm_err);
                            return CMD_FAIL;
                        }

                    }
                    else {
                        cli_out("--- FILTER   port unit=%d  port=%d\n",pcmd->unit,port);
                    }
                }
                break;

            case PORT_ENABLE_ITER:   /* use iter to enable ports from config.bcm */
                bcm_err = bcm_port_config_get(pcmd->unit, &config);
                if (bcm_err != BCM_E_NONE) {
                    cli_out("Failed get port config \n");
                    return CMD_FAIL;
                }
                BCM_PBMP_ITER(config.sfi, port) {

                    pval = soc_property_port_get(pcmd->unit, port, spn_PORT_IS_SFI, 0);
                    cli_out("--- SFI Property unit=%d Port= %d  pval=%d\n",pcmd->unit,port,pval);
                    if(pval == 1) {
                        bcm_err = bcm_port_enable_set(pcmd->unit, port, 1);
                        if (bcm_err != BCM_E_NONE) {
                            cli_out("Port Enable Command failed index=%d port=%d  err=%d\n",
                                    i,port, bcm_err);
                            return CMD_FAIL;
                        }
                    }
                }
                break;
            case PORT_SPEED_SET:
                bcm_err = bcm_port_speed_set(pcmd->unit, pcmd->arg1, 1);
                break;
            case PORT_SPEED_RANGE:
                for(port = pcmd->arg1; port <= pcmd->arg2; port++) {
                    bcm_err = bcm_port_speed_set(pcmd->unit, port, 1);
                    if (bcm_err != BCM_E_NONE) {
                        cli_out("Port Speed Command failed index=%d port=%d  err=%d\n",
                                i,port, bcm_err);
                        return CMD_FAIL;
                    }
                }
                break;
            case PORT_CONTROL:
                bcm_err = bcm_port_control_set(pcmd->unit, pcmd->arg1, pcmd->arg2,pcmd->arg3);
                break;

            case PORT_CONTROL_RANGE:
                for (port = pcmd->arg1; port <= pcmd->arg2; port++) {
                    bcm_err = bcm_port_control_set(pcmd->unit, port, pcmd->arg3, pcmd->arg4);
                    if (bcm_err != BCM_E_NONE) {
                        cli_out("Port Control(Ability) Command failed, port=%d  err=%d\n", port, bcm_err);
                        return CMD_FAIL;
                    }
                }
                break;

            case GPORT_CONTROL_RANGE:
                for (port = *((int*)(pcmd->arg1)); port <= *((int*)(pcmd->arg2)); port++) {
                    bcm_err = bcm_port_control_set(pcmd->unit, port, pcmd->arg3, pcmd->arg4);
                    if (bcm_err != BCM_E_NONE) {
                        cli_out("Port Control(Ability) Command failed, port=%d  err=%d\n", port, bcm_err);
                        return CMD_FAIL;
                    }
                }
                break;

            case GPORT_CONTROL:
		port =  *((int*)(pcmd->arg1)); 
		bcm_err = bcm_port_control_set(pcmd->unit, port, pcmd->arg2, pcmd->arg3);
		if (bcm_err != BCM_E_NONE) {
		    cli_out("Port Control(Ability) Command failed, port=%d  err=%d\n", port, bcm_err);
		    return CMD_FAIL;
		}

                break;

            case SET_MODULE_PROTOCOL:
            {
                int card_id = pcmd->arg3;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if (pcmd->arg1 == AUTO_MODID_QE0) {
                    bcm_err = bcm_stk_module_protocol_set(pcmd->unit, (node_id + BCM_MODULE_FABRIC_BASE), pcmd->arg2);
                }
                else if (pcmd->arg1 == AUTO_MODID_QE1) {
                    bcm_err = bcm_stk_module_protocol_set(pcmd->unit, (node_id1 + BCM_MODULE_FABRIC_BASE), pcmd->arg2);
                }
                else {
                    bcm_err = bcm_stk_module_protocol_set(pcmd->unit, pcmd->arg1, pcmd->arg2);
                }

                if (bcm_err != BCM_E_NONE) {
                    cli_out("Set Module Protocol Command failed, Module=%d  err=%d\n", pcmd->arg1, bcm_err);
#if 0
                    return CMD_FAIL;
#endif /* 0 */
                    bcm_err = BCM_E_NONE;
                }
                break;
            }

            case SET_LGL_MODULE_PROTOCOL:
            {
                int card_id = pcmd->arg3;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if (*((int *)(pcmd->arg1)) == AUTO_MODID_QE0) {
                    bcm_err = bcm_stk_module_protocol_set(pcmd->unit, (node_id + BCM_MODULE_FABRIC_BASE), pcmd->arg2);
                }
                else if (*((int *)(pcmd->arg1)) == AUTO_MODID_QE1) {
                    bcm_err = bcm_stk_module_protocol_set(pcmd->unit, (node_id1 + BCM_MODULE_FABRIC_BASE), pcmd->arg2);
                }
                else {
                    bcm_err = bcm_stk_module_protocol_set(pcmd->unit, *((int *)(pcmd->arg1)), pcmd->arg2);
                }

                if (bcm_err != BCM_E_NONE) {
                    cli_out("Set Module Protocol Command failed, Module=%d  err=%d\n", *((int *)(pcmd->arg1)), bcm_err);
#if 0
                    return CMD_FAIL;
#endif /* 0 */
                    bcm_err = BCM_E_NONE;
                }
                break;
            }


#ifdef BCM_BME3200_SUPPORT
            case LCM_BM3200_XCFG_INIT:
#if 0
                cli_out("final xcfg config LCM%d FC=%d  XCFG_%c:\n", pcmd->unit, pcmd->arg1, (!pcmd->arg1) ? 'A':'B');
                for (x=0; x<40; x++) {
                    cli_out("xcfg[%2d]=%2d\n", x, sbx_mclc_xcfg_port_map[pcmd->arg2][x]);
                }
                cli_out("-------------\n");
#endif
                bcm_err = soc_bm3200_lcm_fixed_config(pcmd->unit, pcmd->arg1,sbx_mclc_xcfg_port_map[pcmd->arg2], 40);
                break;

#endif /* BCM_BME3200_SUPPORT */
            case LCM_MODE_SET:
#ifdef BCM_BME3200_SUPPORT
                if (SOC_IS_SBX_BME3200(pcmd->unit)) {
                    bcm_err = soc_bm3200_lcm_mode_set(pcmd->unit, pcmd->arg1);
                }
#endif
#ifdef BCM_BM9600_SUPPORT
                if (SOC_IS_SBX_BM9600(pcmd->unit)) {
                    bcm_err = soc_bm9600_lcm_mode_set(pcmd->unit, pcmd->arg1);
                }
#endif
                break;
#ifdef BCM_BM9600_SUPPORT
            case LCM_BM9600_XCFG_SET:
            {
                int card_id = pcmd->arg2>>4;
                int configAB = pcmd->arg2&0x1;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if (SOC_IS_SBX_BM9600(pcmd->unit)) {
                    if (pcmd->arg1 == AUTO_MODID_QE0) {
                        bcm_err = soc_bm9600_lcm_fixed_config(pcmd->unit, (node_id + BCM_MODULE_FABRIC_BASE),
                                                              configAB, pcmd->arg3, pcmd->arg4);
                        if (bcm_err) {
                            cli_out("LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg3, pcmd->arg4);
                        }
                        bcm_err = soc_bm9600_lcm_fixed_config(pcmd->unit, (node_id + BCM_MODULE_FABRIC_BASE),
                                                              configAB, pcmd->arg4, pcmd->arg3);
                        if (bcm_err) {
                            cli_out("LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg4, pcmd->arg3);
                        }
                    } else if(pcmd->arg1 == AUTO_MODID_QE1) {
                        bcm_err = soc_bm9600_lcm_fixed_config(pcmd->unit, (node_id1 + BCM_MODULE_FABRIC_BASE),
                                                              configAB, pcmd->arg3, pcmd->arg4);
                        if (bcm_err) {
                            cli_out("LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg3, pcmd->arg4);
                        }
                        bcm_err = soc_bm9600_lcm_fixed_config(pcmd->unit, (node_id1 + BCM_MODULE_FABRIC_BASE),
                                                              configAB, pcmd->arg4, pcmd->arg3);
                        if (bcm_err) {
                            cli_out("LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg4, pcmd->arg3);
                        }
                    } else {
                        bcm_err = soc_bm9600_lcm_fixed_config(pcmd->unit, pcmd->arg1, configAB, pcmd->arg3, pcmd->arg4);

                        if (bcm_err) {
                            cli_out("LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg3, pcmd->arg4);
                        }
                        bcm_err = soc_bm9600_lcm_fixed_config(pcmd->unit, pcmd->arg1, configAB, pcmd->arg4, pcmd->arg3);
                        if (bcm_err) {
                            cli_out("LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg4, pcmd->arg3);
                        }
                    }
                }
                break;
            }
#endif /* BCM_BM9600_SUPPORT */
#ifdef BCM_BM9600_SUPPORT
            case LCM_BM9600_XCFG_GET:
            {
                int card_id = pcmd->arg2>>4;
                int configAB = pcmd->arg2&0x1;
                int skip_command=0;
                if (card_id != CMD_CARD_ALL )
                    if (card_id != _cmd_card_filter)
                        skip_command = 1;
                if (skip_command)
                    break;
                if (SOC_IS_SBX_BM9600(pcmd->unit)) {
                    if (pcmd->arg1 == AUTO_MODID_QE0) {
                        bcm_err = soc_bm9600_lcm_fixed_config_validate(pcmd->unit, (node_id + BCM_MODULE_FABRIC_BASE),
                                                                       configAB, pcmd->arg3, pcmd->arg4);
                        if (bcm_err) {
                            cli_out("ERROR:LCM fixed xcfg error validating(%d to %d)\n", pcmd->arg3, pcmd->arg4);
                        }
                        bcm_err = soc_bm9600_lcm_fixed_config_validate(pcmd->unit, (node_id + BCM_MODULE_FABRIC_BASE),
                                                                       configAB, pcmd->arg4, pcmd->arg3);
                        if (bcm_err) {
                            cli_out("ERROR:LCM fixed xcfg error validating(%d to %d)\n", pcmd->arg4, pcmd->arg3);
                        }
                    } else if(pcmd->arg1 == AUTO_MODID_QE1) {
                        bcm_err = soc_bm9600_lcm_fixed_config_validate(pcmd->unit, (node_id1 + BCM_MODULE_FABRIC_BASE),
                                                                       configAB, pcmd->arg3, pcmd->arg4);
                        if (bcm_err) {
                            cli_out("ERROR:LCM fixed xcfg error validating(%d to %d)\n", pcmd->arg3, pcmd->arg4);
                        }
                        bcm_err = soc_bm9600_lcm_fixed_config_validate(pcmd->unit, (node_id1 + BCM_MODULE_FABRIC_BASE),
                                                                       configAB, pcmd->arg4, pcmd->arg3);
                        if (bcm_err) {
                            cli_out("ERROR:LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg4, pcmd->arg3);
                        }
                    } else {
                        bcm_err = soc_bm9600_lcm_fixed_config_validate(pcmd->unit, pcmd->arg1, configAB, pcmd->arg3, pcmd->arg4);

                        if (bcm_err) {
                            cli_out("ERROR:LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg3, pcmd->arg4);
                        }
                        bcm_err = soc_bm9600_lcm_fixed_config_validate(pcmd->unit, pcmd->arg1, configAB, pcmd->arg4, pcmd->arg3);

                        if (bcm_err) {
                            cli_out("ERROR:LCM fixed xcfg error setting connection(%d to %d)\n", pcmd->arg4, pcmd->arg3);
                        }
                    }
                }
                break;
            }
#endif /* BCM_BM9600_SUPPORT */

            case -1:
                i = MAX_CMDS;
                break;
            default:
                cli_out("Unrecognized command at table index=%d: cmd=%d\n",i,pcmd->cmd);
                break;
            }
            if (BCM_FAILURE(bcm_err)) {
                cli_out("Config Cmd FAILED err=%d idx=%d  args: cmd=%d unit=%d arg1=%d  arg2=%d  arg3=%d arg4=%d\n",
                        bcm_err,i, pcmd->cmd, pcmd->unit, pcmd->arg1, pcmd->arg2, pcmd->arg3,pcmd->arg4);

            }
        } /* skip unit */
    } /* for i loop */

    /* diag_cosq_init */
    _sbx_diag_cosq_init_all();

    return CMD_OK;
}

char cmd_sbx_mclcstandalone_config_usage[] =
"Usage:\n"
"mclcstandaloneinit [nodeid=<num>] [serdesmask=<num>]\n"
"  nodeid is the line card's node in the system\n"
"  note that this configures the local LCM as SE/BME for standalone operation\n"
"  serdesmask can be used to enable more than links 0x3ffff";

cmd_result_t
cmd_sbx_mclcstandalone_config(int unit, args_t *a)
{
    int rv;
    int link_enable;
    int arg_modid, serdes_mask;
    int ret_code;
    parse_table_t pt;
    uint8 board_id = 0;
    int bTmeMode = soc_property_get(unit, spn_QE_TME_MODE,0);

    link_enable = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);

    /* 
     * FPGA does not exist for the QE2K Benchscreen Board, so board_id is
     * the board_type which was defined earlier.
     */
    if (board_type == BOARD_TYPE_QE2K_BSCRN_LC) {
      board_id = board_type;
    } else {
      _mc_fpga_read8(FPGA_BOARD_ID_OFFSET, &board_id);
    }

   /*
    * pick up args
    */

    if (ARG_CNT(a)) {
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "nodeid", PQ_DFL | PQ_INT,
                        0, &arg_modid, NULL);
        parse_table_add(&pt, "serdesmask", PQ_DFL | PQ_INT,
                        (void*)0x3ffff, &serdes_mask, NULL);
        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

/*  MetroCore LC ONLY !!!! */
    if (soc_ndev == 4) {
        rv = _mc_fpga_write8(FPGA_SCI_ROUTING_OFFSET, FPGA_SCI_TO_LCM);
        if (rv) {
            cli_out("ERROR: FPGA master mode write failed\n");
            return rv;
        }
        /* very old code, screws up easy reload since CMWRITE doesn't go through SAND_HAL macros */
        /* removed.                                                                              */
#if 000
        i = CMWRITE(2,0x2c0, 0x03fe00);
        i = CMWRITE(3,0x2c0, 0x00000001);
#endif
        if(bTmeMode == 1) {
            cli_out("mclcstandalone_config tme mode link_enable=%x\n",link_enable);
            return cmd_sbx_card_config(unit,0,-1,link_enable, &mc_lc_tmemode_cmd_tbl[0]);
        }
        else {
            cli_out("mclcstandalone_config fic mode link_enable=%x\n",link_enable);
            return cmd_sbx_card_config(unit,0,-1,link_enable, &mc_lc_dev_cmd_tbl[0]);
        }
    }
#ifdef BCM_BM9600_SUPPORT
    else if (board_id == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC ||
             board_id == BOARD_TYPE_FE2KXT_4X10G_QE2K_POLARIS_LC) {
      rv = _mc_fpga_write8(17,0x0A);
    } else if (board_id == BOARD_TYPE_QE2K_BSCRN_LC) {
      /* No FPGA on the QE2K Benchscreen board, so just print a message */
      cli_out("Running mclc standalone config tme mode on QE2K Benchscreen line card!\n");
    } else {
       rv = _mc_fpga_write8(16, 0xA0);
       rv = _mc_fpga_write8(17, 0x0A);
    }

    if ((board_id == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) ||
        (board_id == BOARD_TYPE_FE2KXT_4X10G_QE2K_POLARIS_LC) ||
        (board_id == BOARD_TYPE_QE2K_BSCRN_LC)) {
      if (bTmeMode) {
        if (board_id == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
          cli_out("CALLING card_config FE2KXT LC table TME mode link_enable=%x\n",link_enable);
          return cmd_sbx_card_config( unit, 0, -1, link_enable, fe2kxt_lc_tmemode_cmd_tbl);
        } else if (board_id == BOARD_TYPE_QE2K_BSCRN_LC) {
          /* QE2K Benchscreen Card, will use the same config as the C2 Board */
          cli_out("CALLING card_config QE2K BENCHSCREEN LC table TME mode link_enable=%x\n",link_enable);
          return cmd_sbx_card_config( unit, 0, -1, link_enable, qe2k_bscrn_lc_tmemode_cmd_tbl);
        } else {
          cli_out("CALLING card_config FE2KXT 4x10G LC table TME mode link_enable=%x\n",link_enable);
          return cmd_sbx_card_config( unit, 0, -1, link_enable, fe2kxt_lc_tmemode_cmd_tbl);
        }
      } else {
#ifdef BCM_QE2000_SUPPORT
        bcm_gport_t cpu_gport;
        int qe_modid;
        if (board_id == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
          cli_out("CALLING card_config FE2KXT LC table FIC mode link_enable=%x\n",link_enable);
          ret_code = cmd_sbx_card_config( unit, 0, -1, link_enable, fe2kxt_lc_cmd_tbl);
          /* The FE modid is the QE node id, convert to QE Modid */
          qe_modid = BCM_STK_NODE_TO_MOD(PL1_FE0_MODID);
          BCM_GPORT_MODPORT_SET(cpu_gport, qe_modid, SBX_QE_CPU_PORT);
          ret_code = bcm_switch_control_set(1, bcmSwitchCpuCopyDestination, cpu_gport);
          return ret_code;
        } else if (board_id == BOARD_TYPE_QE2K_BSCRN_LC) {
          cli_out("CALLING card_config QE2K BENCHSCREEN LC table FIC mode link_enable=%x\n",link_enable);
          ret_code = cmd_sbx_card_config( unit, 0, -1, link_enable, qe2k_bscrn_lc_cmd_tbl);
          /* The FE modid is the QE node id, convert to QE Modid */
          qe_modid = BCM_STK_NODE_TO_MOD(PL1_FE0_MODID);
          BCM_GPORT_MODPORT_SET(cpu_gport, qe_modid, SBX_QE_CPU_PORT);
          ret_code = bcm_switch_control_set(1, bcmSwitchCpuCopyDestination, cpu_gport);
          return ret_code;
        } else {
          cli_out("CALLING card_config FE2KXT 4x10G LC table FIC mode link_enable=%x\n",link_enable);
          ret_code = cmd_sbx_card_config( unit, 0, -1, link_enable, fe2kxt_lc_cmd_tbl);
          /* The FE modid is the QE node id, convert to QE Modid */
          qe_modid = BCM_STK_NODE_TO_MOD(PL1_FE0_MODID);
          BCM_GPORT_MODPORT_SET(cpu_gport, qe_modid, SBX_QE_CPU_PORT);
          ret_code = bcm_switch_control_set(1, bcmSwitchCpuCopyDestination, cpu_gport);
          return ret_code;
        }
#else
        return CMD_OK;
#endif /* BCM_QE2000_SUPPORT */
      }

    } else {
        if (bTmeMode == 1){
            cli_out("---- Init standalone card in tme mode  unit=%d  links=%x\n",unit, link_enable);
            return cmd_sbx_card_config(unit, 0, -1, link_enable, pl_lc_tmemode_cmd_tbl);
	} else {
	    cli_out("CALLING card_config Polaris LC table link_enable=%x\n",link_enable);
            card_type = SBX_CARD_TYPE_PL_STANDALONE_LINE_CARD;
	    return cmd_sbx_card_config( unit, 0, -1, link_enable, polaris_lc_cmd_tbl);
	}
    }

#endif

    return CMD_FAIL;
}


char cmd_sbx_mcinit_config_usage[] =
"Usage:\n"
"mcinit \n"
"  init metrocore card, card setting read from config.bcm file \n"
;

cmd_result_t
cmd_sbx_mcinit_config(int unit, args_t *orig_a) {
    int chassis_type;
    int nodeid, serdesmask = 0x0;
#ifdef BCM_BM9600_SUPPORT
    int slave;
#endif
    args_t *new_a = NULL;
    char *c_next;
    char c[256];
    uint8 board_id = 0;
    int rv = CMD_OK;
#ifdef BCM_SIRIUS_SUPPORT
    int tme_mode;

    BCM_GPORT_MODPORT_SET(qe2000_sfi_base_gport, FE2KXT_PL_FAB_LC0_QE0_MODID,QE2000_SFI_PORT_BASE);
    BCM_GPORT_MODPORT_SET(qe2000_sfi_end_gport, FE2KXT_PL_FAB_LC0_QE0_MODID, QE2000_SFI_PORT_BASE + 17);
    BCM_GPORT_MODPORT_SET(sirius_sfi_base_gport, PL_SIRIUS_IPASS_MODID, SIRIUS_SFI_PORT_BASE);
    BCM_GPORT_MODPORT_SET(sirius_sfi_end_fic_gport, PL_SIRIUS_IPASS_MODID, SIRIUS_SFI_PORT_BASE + 9);
    BCM_GPORT_MODPORT_SET(sirius_sfi_base_lcl_gport, PL_SIRIUS_IPASS_MODID, SIRIUS_SFI_PORT_BASE + 10);
    BCM_GPORT_MODPORT_SET(sirius_sfi_end_gport, PL_SIRIUS_IPASS_MODID, SIRIUS_SFI_PORT_BASE + 21);
#endif
#ifdef BCM_BM9600_SUPPORT
    BCM_GPORT_MODPORT_SET(polaris_sfi_base_gport0, PL_SIRIUS_IPASS_MODID, 8);
    BCM_GPORT_MODPORT_SET(polaris_sfi_end_gport0, PL_SIRIUS_IPASS_MODID, 23);
    BCM_GPORT_MODPORT_SET(polaris_sfi_base_gport1, PL_SIRIUS_IPASS_MODID,25);
    BCM_GPORT_MODPORT_SET(polaris_sfi_end_gport1, PL_SIRIUS_IPASS_MODID, 27);
    BCM_GPORT_MODPORT_SET(polaris_sfi_base_gport2, PL_SIRIUS_IPASS_MODID,29);
    BCM_GPORT_MODPORT_SET(polaris_sfi_end_gport2, PL_SIRIUS_IPASS_MODID, 31);
    BCM_GPORT_MODPORT_SET(polaris_sfi_sci_gport0, PL_SIRIUS_IPASS_MODID, 28);
    BCM_GPORT_MODPORT_SET(polaris_sfi_sci_gport1, PL_SIRIUS_IPASS_MODID, 24);
#endif
    new_a = sal_alloc((sizeof(args_t)), "new arguments");

    cli_out("Appl: cmd_sbx_mcinit_config()\n");

    if(!new_a) {
        cli_out("Mcinit failed to allocate memory for new arguments: out of memory \n");
        return CMD_FAIL;
    }

    /* Check if this is Sirus simulation model */
    if (SAL_BOOT_BCMSIM && board_type == BOARD_TYPE_SIRIUS_SIM) {
        if (unit == MODEL_SIRIUS_UNIT) {
            cli_out("Running init for Sirius Model\n");
#ifdef _MODEL_SIRIUS_SUPPORT_
#ifdef BCM_SIRIUS_SUPPORT
            rv = cmd_sbx_sirius_tme_config(unit, NULL, MODEL_SIRIUS_MODID);
#endif /* BCM_SIRIUS_SUPPORT */
#else /* _MODEL_SIRIUS_SUPPORT_ */
            rv = CMD_OK;
#endif /* !(_MODEL_SIRIUS_SUPPORT_) */
            sal_free(new_a);
            return rv;
        } else if (unit == MODEL_BCM56634_UNIT) {
#ifdef _BCM56634_MODEL_
            bcm_port_config_t port_config;
            bcm_port_t        port, hg_port;

            cli_out("Configuring port force forward on Triumph \n");
            /* set up force forward to HG on Bcm56634/TR */
            rv = bcm_port_config_get(MODEL_BCM56634_UNIT, &port_config);
            if (BCM_FAILURE(rv)) {
                sal_free(new_a);
                return CMD_FAIL;
            }

            PBMP_ITER(port_config.ge, port) {
                if (port < 13) {
                    hg_port = 27;
                } else if (port < 24) {
                    hg_port = 28;
                } else if (port < 27 || (port > 30 && port < 40)) {
                    hg_port = 29;
                } else {
                    hg_port = 30;
                }
                rv = bcm_port_force_forward_set(MODEL_BCM56634_UNIT, port, hg_port, TRUE);
                if (BCM_FAILURE(rv)) {
                    cli_out("Failed in %s port %d hg port %d: %s\n",
                            FUNCTION_NAME(), port, hg_port, bcm_errmsg(rv));
                    rv = CMD_FAIL;
                }
            }

            sal_free(new_a);
            return rv;
#else
            sal_free(new_a);
            return CMD_OK;
#endif /* _BCM56634_MODEL_ */
        }


    }

#ifdef BCM_BM9600_SUPPORT
    if (board_type == BOARD_TYPE_POLARIS_IPASS) {
	/* init and bringup the card (ports, protocol, xcfg, etc) */
	cli_out("Running mcinit on polaris ipass fabric card\n");
	serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
	cli_out("linkenable=0x%x", serdesmask);
	
	if (soc_property_get(unit, spn_DIAG_EMULATOR_PARTIAL_INIT, 0)) {
	    /* this is init for SV team qe2k/sirius interop testing  */
	    rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* node_id */,
				     FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC /* node_id1 */,
				     serdesmask, polaris_ipass_sv_qe2k_ss_interop_cmd_tbl);
	} else {
	    if (soc_property_get(unit, spn_DIAG_CHASSIS, 0) ) {
		/* chassis mode */
		/* this is init for the fabric team qe2k/sirius interop testing */

		if (soc_property_get(unit, spn_HYBRID_MODE, 0) ) {
		    
		    rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* node_id */,
					     FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC /* node_id1 */,
					     serdesmask, polaris_ipass_qe2k_ss_interop_hybrid_cmd_tbl);
		} else {
		    rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* node_id */,
					     FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC /* node_id1 */,
					     serdesmask, polaris_ipass_qe2k_ss_interop_cmd_tbl);
		}
		if (rv) {
            cli_out("cmd_sbx_card_config failed");
            sal_free(new_a);
            return rv;
        }
		

		/* make sure all child gports are created */
		rv = cmd_sbx_sirius_ipass_chassis_config(unit, NULL, PL_IPASS_SIRIUS_MODID);

	    } else {
		/* standalone mode */
		if (soc_property_get(unit, spn_HYBRID_MODE, 0) ) {
		    /* hybrid mode */
		    rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* node_id */, 0  /* node_id1 */, serdesmask, polaris_ipass_hybrid_cmd_tbl);
		} else {
		    rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* node_id */, 0  /* node_id1 */, serdesmask, polaris_ipass_fic_cmd_tbl);
		}

        if (rv) {
            cli_out("cmd_sbx_card_config failed");
            sal_free(new_a);
            return rv;
        }
		/* create queues */
		rv = cmd_sbx_sirius_ipass_config(unit, NULL, PL_IPASS_SIRIUS_MODID);
	    }
	}
	
	if (new_a != NULL) {
	    sal_free(new_a);
	}

	return rv;
    }
#endif

#ifdef BCM_BM9600_SUPPORT
    if ( (soc_property_get(unit, spn_QE_TME_MODE, 0) == 0) ||
	 (soc_property_get(unit, spn_QE_TME_MODE, 0) == 2) ) {
        sbx_appl_polaris_system_update_topology();
    }
#endif /* BCM_BM9600_SUPPORT */

    /* 
     * FPGA does not exist for the QE2K Benchscreen Board, so board_id is
     * the board_type which was defined earlier.
     */
    if (board_type == BOARD_TYPE_QE2K_BSCRN_LC ||
        board_type == BOARD_TYPE_SIRIUS_IPASS) {
      board_id = board_type;
      cli_out("Appl: board_type: %d\n", board_type);
    } else {
      _mc_fpga_read8(FPGA_BOARD_ID_OFFSET, &board_id);
    }

    chassis_type = soc_property_get(unit, spn_DIAG_CHASSIS, 0);
    if (chassis_type == SBX_CHASSIS_TYPE_STANDALONE) {
        /* Standalone line card system */
        if ((soc_ndev == 4)                                     &&
            ((strcmp(soc_dev_name(0), "QE2000_A1") == 0)   ||
             (strcmp(soc_dev_name(0), "QE2000_A2") == 0)   ||
             (strcmp(soc_dev_name(0), "QE2000_A3") == 0)   ||
             (strcmp(soc_dev_name(0), "QE2000_A4") == 0))       &&
            ((strcmp(soc_dev_name(1), "BCM88020_A0") == 0) ||
             (strcmp(soc_dev_name(1), "BCM88020_A1") == 0) ||
             (strcmp(soc_dev_name(1), "BCM88020_A2") == 0))     &&
            ((strcmp(soc_dev_name(2), "BME3200_A0") == 0)  ||
             (strcmp(soc_dev_name(2), "BME3200_B0") == 0))      &&
            ((strcmp(soc_dev_name(3), "BME3200_A0") == 0)  ||
             (strcmp(soc_dev_name(3), "BME3200_B0") == 0))) {
            /* Supported standalone lincard has 4 units,
               with QE2000 as unit 0, FE2000 as unit 1,
               BM3200 as unit 2 and unit 3
            */
            cli_out("Running mcinit on standalone 4-dev line card!\n");
            /* standalone mode always assume nodeid as 0 */
            nodeid = 0;
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=%d serdesmask=0x%x", nodeid, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcstandalone_config(unit, new_a);
            sal_free(new_a);
            return rv;
        }
#ifdef BCM_BM9600_SUPPORT
        else if ((soc_ndev == 5) &&
                  (SOC_IS_SBX_QE2000(0) &&
                   SOC_IS_SBX_QE2000(3) &&
                   SOC_IS_SBX_BM9600(4))) {
            /* Supported standalone lincard has 4 units,
               with QE2000 as unit 0, FE2000 as unit 1,
               BM3200 as unit 2 and unit 3
            */
            cli_out("Running mcinit on standalone 5-dev line card!\n");
            /* standalone mode always assume nodeid as 0 */
            nodeid = 0;
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=%d serdesmask=0x%x", nodeid, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcstandalone_config(unit, new_a);
            sal_free(new_a);
            return rv;
        } else if (board_id == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
            cli_out("Running mcinit standalone on FE2KXT Polaris line card!\n");
            /* standalone mode always assume nodeid as 0 */
            nodeid = 0;
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=%d serdesmask=0x%x", nodeid, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcstandalone_config(unit, new_a);
            sal_free(new_a);
            return rv;
        } else if (board_id == BOARD_TYPE_FE2KXT_4X10G_QE2K_POLARIS_LC) {
            cli_out("Running mcinit standalone on FE2KXT 4x10G Polaris line card!\n");
            /* standalone mode always assume nodeid as 0 */
            nodeid = 0;
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=%d serdesmask=0x%x", nodeid, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
          sal_free(new_a);
              return CMD_FAIL;
            }
            rv = cmd_sbx_mclcstandalone_config(unit, new_a);      
            sal_free(new_a);
            return rv;
        } else if (board_id == BOARD_TYPE_QE2K_BSCRN_LC) {
            cli_out("Running mcinit standalone on QE2K Benchscreen line card!\n");
            /* standalone mode always assume nodeid as 0 */
            nodeid = 0;
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=%d serdesmask=0x%x", nodeid, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcstandalone_config(unit, new_a);
            sal_free(new_a);
            return rv;
        }
#endif /* BCM_BM9600_SUPPORT */
#ifdef BCM_SIRIUS_SUPPORT
        else if ((soc_ndev == 1) && SOC_IS_SIRIUS(0)) {
	    tme_mode = soc_property_get(unit, spn_QE_TME_MODE,0);

	    if (tme_mode == SOC_SBX_QE_MODE_FIC) {
                cli_out("Running mcinit on sirius ipass line card - FIC mode\n");
		serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
		
		cli_out("nodeid=28 linkenable=0x%x\n", serdesmask);
		rv = cmd_sbx_card_config(unit, 28 /* node_id */, 0  /* node_id1 */, serdesmask, sirius_ipass_fic_cmd_tbl);
		if (rv != CMD_OK) {
		    cli_out("errors when bringup sirius SVK in FIC mode\n");		
		}

		rv = cmd_sbx_sirius_ipass_config(unit, NULL, PL_IPASS_SIRIUS_MODID);
		if (rv != CMD_OK) {
		    cli_out("errors when config sirius SVK in FIC mode\n");		
		}

		
		rv = sirius_fc_config1(unit, PL_BCM56634_IPASS_MODID, PL_IPASS_SIRIUS_MODID);
		if (rv != CMD_OK) {
		    cli_out("errors when config flow control for sirius SVK in FIC mode\n");
		}
	    } else if (tme_mode == SOC_SBX_QE_MODE_TME) {
		cli_out("Running mcinit on Sirius iPass linecard - TME mode\n");

		rv = cmd_sbx_sirius_tme_config(unit, NULL, MODEL_SIRIUS_MODID);
		if (rv != CMD_OK) {
		    cli_out("errors when config sirius SVK in TME mode\n");		
		}
		
		
		rv = sirius_fc_config1(unit, MODEL_BCM56634_MODID, MODEL_SIRIUS_MODID);
		if (rv != CMD_OK) {
		    cli_out("errors when config flow control for sirius SVK in TME mode\n");		
		}
	    } else if (tme_mode == SOC_SBX_QE_MODE_HYBRID) {
		cli_out("Running mcinit on Sirius iPass linecard - Hybrid mode\n");

		rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* node_id */, 0  /* node_id1 */, serdesmask, sirius_ipass_hybrid_cmd_tbl);

		if (rv != CMD_OK) {
		    cli_out("errors when bringup sirius SVK in Hybrid mode\n");		
		}

#ifdef TEST_REQUEUE
		rv = bcm_fabric_control_set(unit, bcmFabricQueueMax, 12*1024-1);
		if (rv) {
		  cli_out("error setting max voq\n");
		}
		rv = bcm_fabric_control_set(unit, bcmFabricEgressQueueMin, 12*1024);
		if (rv) {
		  cli_out("error setting max voq\n");
		}
#endif

		rv = cmd_sbx_sirius_tme_config(unit, NULL, MODEL_SIRIUS_MODID);
		if (rv != CMD_OK) {
		    cli_out("errors when config sirius SVK in Hybrid mode\n");		
		}

		
		rv = sirius_fc_config1(unit, MODEL_BCM56634_MODID, MODEL_SIRIUS_MODID);
		if (rv != CMD_OK) {
		    cli_out("errors when config flow control for sirius SVK in Hybrid mode\n");
		}

	    }		

            if (new_a != NULL) {
                sal_free(new_a);
            }
            return rv;
        }
#endif /* BCM_SIRIUS_SUPPORT */
        else {
            if (soc_ndev == 5) {
                rv = cmd_sbx_mclcstandalone_config(unit, new_a);
                sal_free(new_a);
                return rv;
            } else {
                cli_out("Unrecognized standalone line card type!\n");
                sal_free(new_a);
                return CMD_FAIL;
            }
        }
    } else {
        if (board_type == BOARD_TYPE_QE2K_BSCRN_LC) {
          cli_out("[%s:%d] ERROR: QE2K Benchscreen board can be run STANDALONE only!!!\n", FUNCTION_NAME(), __LINE__);
          sal_free(new_a);
          return CMD_FAIL;
        }

        /* Multicards system */
        if ((soc_ndev == 2)                                     &&
            ((strcmp(soc_dev_name(0), "BME3200_A0") == 0)  ||
             (strcmp(soc_dev_name(0), "BME3200_B0") == 0))      &&
            ((strcmp(soc_dev_name(1), "BME3200_A0") == 0)  ||
             (strcmp(soc_dev_name(1), "BME3200_B0") == 0))) {
            /* Supported fabric card has 2 units,
               with BM3200 as unit 0 and unit 1
            */
            cli_out("Running mcinit on fabric card!\n");
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "linkenable=0x%x", serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mcfabinit_config(unit, new_a);
            sal_free(new_a);
            return rv;
        } else if ((soc_ndev == 4)                                     &&
                   ((strcmp(soc_dev_name(0), "QE2000_A1") == 0)   ||
                    (strcmp(soc_dev_name(0), "QE2000_A2") == 0)   ||
                    (strcmp(soc_dev_name(0), "QE2000_A3") == 0)   ||
                    (strcmp(soc_dev_name(0), "QE2000_A4") == 0))       &&
                   ((strcmp(soc_dev_name(1), "BCM88020_A0") == 0) ||
                    (strcmp(soc_dev_name(1), "BCM88020_A1") == 0) ||
                    (strcmp(soc_dev_name(1), "BCM88020_A2") == 0))     &&
                   ((strcmp(soc_dev_name(2), "BME3200_A0") == 0)  ||
                    (strcmp(soc_dev_name(2), "BME3200_B0") == 0))      &&
                   ((strcmp(soc_dev_name(3), "BME3200_A0") == 0)  ||
                    (strcmp(soc_dev_name(3), "BME3200_B0") == 0))) {
            cli_out("Running mcinit on line card!\n");
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=-1 linkenable=0x%x", serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcinit_table_config(unit, new_a);
            sal_free(new_a);
            return rv;
        }
#ifdef BCM_BM9600_SUPPORT
        else if ((soc_ndev == 1) &&
                   SOC_IS_SBX_BM9600(0)) {
            /* Supported fabric card has 1 units,
               with BM9600 as unit 0
            */
            cli_out("Running mcinit on polaris fabric card!\n");
            slave = soc_property_get(unit, spn_DIAG_SLAVE_FC, 0);
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "slave=%d linkenable=0x%x", slave, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mcfabinit_config(unit, new_a);
            sal_free(new_a);
            return rv;
        }else if ((soc_ndev == 5) &&
                  (SOC_IS_SBX_QE2000(0) &&
                   SOC_IS_SBX_QE2000(3) &&
                   SOC_IS_SBX_BM9600(4))) {
            cli_out("Running mcinit on polaris line card!\n");
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=-1 linkenable=0x%x", serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcinit_table_config(unit, new_a);
            sal_free(new_a);
            return rv;
        } else if (board_id == BOARD_TYPE_FE2KXT_QE2K_POLARIS_LC) {
            cli_out("Running mcinit on FE2KXT Polaris line card!\n");
	    if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX) {
		PL1_FE0_MODID = PL_SIRIUS_IPASS_NODE;
		PL1_QE0_MODID = PL_SIRIUS_IPASS_MODID;
		nodeid = PL1_FE0_MODID;
	    } else {
		nodeid=-1;
	    }
            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            /* coverity[secure_coding] */
            sal_sprintf(c, "nodeid=%d linkenable=0x%x", nodeid, serdesmask);
            if (diag_parse_args(c, &c_next, new_a)) {
                sal_free(new_a);
                return CMD_FAIL;
            }
            rv = cmd_sbx_mclcinit_table_config(unit, new_a);
            sal_free(new_a);
            return rv;
        }
#endif /* BCM_BM9600_SUPPORT */
#ifdef BCM_SIRIUS_SUPPORT
	else if ((soc_ndev == 1) && SOC_IS_SIRIUS(0)) {
	    /* sirius multi-node testing, support sirius/qe2k interop test only for now */
            cli_out("Running mcinit on sirius ipass line card - FIC mode for chassis\n");

            serdesmask = soc_property_get(unit, spn_DIAG_SERDES_MASK, 0x3FFFF);
            cli_out("nodeid=28 linkenable=0x%x", serdesmask);


	    if (soc_property_get(unit, spn_HYBRID_MODE, 0) ) {

		rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* sirius node id */,
					 FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC  /* qe2000 node id */,
					 serdesmask, sirius_ipass_qe2k_ss_interop_hybrid_cmd_tbl);
	    } else {

		rv = cmd_sbx_card_config(unit, PL_SIRIUS_IPASS_NODE /* sirius node id */,
					 FE2KXT_PL_FAB_LC0_QE0_NODE_ON_FC  /* qe2000 node id */,
					 serdesmask, sirius_ipass_qe2k_ss_interop_fic_cmd_tbl);
	    }


	    /* make sure all child gports are created */
            rv = cmd_sbx_sirius_ipass_chassis_config(unit, NULL, PL_IPASS_SIRIUS_MODID);

            /* Flow Control Setup */
            rv = sirius_fc_config1(unit, PL_BCM56634_IPASS_MODID, PL_IPASS_SIRIUS_MODID);

            if (new_a != NULL) {
                sal_free(new_a);
            }
            return rv;
	}
#endif
        else {
            cli_out("Unrecognized fabric/line card type!\n");
            sal_free(new_a);
            return CMD_FAIL;
        }
    }
}

#if defined(BCM_FE2000_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)

static int learn_debug = 0;
static int learning_running = 0;
static int fe_unit = -1;
static int qe_unit = -1;
static int age_entry_num = 0;
static int learn_count = 0;
static int echo_instead_of_learn=0; /* functionality for WB RX/TX test */
static int echo_node=31;

void _l2_age_cb(int unit,
                bcm_l2_addr_t *l2addr,
                int insert,
                void *userdata)
{
   age_entry_num++;
}

bcm_rx_t
_learn_rx_cb(int unit, bcm_pkt_t *pkt, void *cookie)
{
    int i, j;
    int rv;
    char s[128];
    bcm_l2_addr_t l2addr;
    bcm_mac_t smac;
    uint32 ethertype;
    bcm_vlan_t vid = 0xffff;
    uint8 *p;

    COMPILER_REFERENCE(echo_node);
    COMPILER_REFERENCE(echo_instead_of_learn);

   if (fe_unit == -1 || qe_unit == -1) {
        for (i = 0; i < soc_all_ndev; i++) {
            if (SOC_IS_SBX_QE2000(i)) {
                qe_unit = i;
            }
            if (SOC_IS_SBX_CALADAN3(unit)) {
                fe_unit = qe_unit = i;
            }
        }
    }

    if (learn_debug) {
        cli_out("DiagRxCallBack: pkt len=%d rxUnit=%d rxPort=%d"
                " rxReason=0x%x\n",
                pkt->pkt_len, pkt->rx_unit, pkt->rx_port, pkt->rx_reason);
        cli_out("route header:\n");
        j = 0;
        for (i = 0; i < 12; i++) {
            /* coverity[secure_coding] */
            j += sal_sprintf(&s[j], "%02x ", pkt->_sbx_rh[i]);
        }

        j = 0;
        cli_out("%s\n", s);
        cli_out("payload:\n");
        for (i = 0; i < 32; i++) {
            /* coverity[secure_coding] */
            j += sal_sprintf(&s[j], "%02x ", pkt->pkt_data->data[i]);
            if (i % 16 == 15) {
                cli_out("%s\n", s);
                j = 0;
            }
        }
        cli_out("\n");
    }

    if ((pkt->_sbx_hdr_len != SOC_SBX_G3P1_ERH_LEN_ARAD) 
        && (pkt->_sbx_hdr_len != SOC_SBX_G3P1_ERH_LEN_SIRIUS)) {
        cli_out("unexpected ERH len %d\n", pkt->_sbx_hdr_len);
        return BCM_RX_NOT_HANDLED;
    } else {
        pkt->pkt_data->data += pkt->_sbx_hdr_len;
        pkt->pkt_len -= pkt->_sbx_hdr_len;
    }

    p = (uint8 *)pkt->pkt_data->data;
    if (learn_debug) {
        cli_out("data 0x%x len %d\n", (uint32)pkt->pkt_data->data, pkt->pkt_len);
    }

    
    if (echo_instead_of_learn) {
        /* Please note that this functionality was developed just to 
           generate traffic on TX. It appears that the packet we send
           out is not quite right, as we get rb length mismatch errors.
           If "real" echo functionality is needed, further debug will be
           necessary */
        learn_count++;
        /* Echo packet to some remote, non-existent node */
        pkt->dest_mod=echo_node;
        rv = bcm_tx(unit, pkt, NULL);
        if (BCM_FAILURE(rv)) {
            cli_out("Failed to echo packet: %s\n", bcm_errmsg(rv));
        }
        sal_usleep(10); /* Rate-limit the tx packets */
        return BCM_RX_HANDLED;
    }


#if defined(BCM_FE2000_P3_SUPPORT) || defined(BCM_CALADAN3_G3P1_SUPPORT)
    if (BCM_RX_REASON_GET(pkt->rx_reasons, bcmRxReasonCpuLearn)) {
      if (pkt->pkt_len < 32) {
	cli_out("first buffer is too short: %d\n", pkt->pkt_len);
      }
      sal_memcpy(smac, &p[6], 6);
      ethertype = ((p[12] << 8) | p[13]);
      
      if (ethertype == 0x8100) {
	vid = ((p[14] << 8) | p[15]);
      }
      if (learn_debug) {
	cli_out("port %d vid %d vlan %d \n", pkt->rx_port, vid, pkt->vlan);
      }
      
      bcm_l2_addr_t_init(&l2addr, smac, pkt->vlan);
      l2addr.modid = pkt->rx_unit;
      l2addr.port = pkt->rx_port;
      if (learn_debug) {
	cli_out("learning %02x-%02x-%02x-%02x-%02x-%02x "
                "on module %d, port %d\n",
                smac[0], smac[1], smac[2], smac[3], smac[4], smac[5],
                pkt->rx_unit, pkt->rx_port);
      }
      rv = bcm_l2_addr_add(fe_unit, &l2addr);
      if (BCM_FAILURE(rv)) {
	cli_out("ERROR:  bcm_l2_addr_add(%d) %02x-%02x-%02x-%02x-%02x-%02x returns %d: %s\n",
                fe_unit, smac[0], smac[1], smac[2], smac[3], smac[4], smac[5],
                rv, bcm_errmsg(rv));
      } else if (BCM_SUCCESS(rv)) {
	learn_count++;
      }
    
      return BCM_RX_HANDLED;
    }
#endif /* BCM_FE2000_P3_SUPPORT */

    return BCM_RX_NOT_HANDLED;
}


static int  bcm_test_v4_load_pfx(FILE *fp, int *vrf, uint32 *prefix, uint32 *pfxLen);

int load_routes(int unit, char *filename, int max_entries, int batch)
{
  bcm_l3_intf_t      l3_intf;
  bcm_l3_egress_t    l3_egr;
  bcm_if_t           l3_egr_id;
  bcm_l3_route_t     route_info;
  int                status;
  uint32             routes = 0;
  sal_usecs_t        st, e, d_usec, d_sec;
  int                original_batch_mode;
  int                vrf=0;
  uint32             prefix;
  uint32             prefixMask;
#ifdef  BCM_CALADAN3_G3P1_SUPPORT
  int flush_cnt=100;
#endif 
  FILE *fp;

  if(!(fp = sal_fopen(filename, "r"))){
    cli_out("Cant open %s", filename);
    return CMD_FAIL;
  }

  status = bcm_switch_control_get(unit, bcmSwitchL3RouteCache, &original_batch_mode);
  if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_get l3cache failed err=%d %s\n", status,
              bcm_errmsg(status));
      sal_fclose(fp);
      return CMD_FAIL;

  }

  status = bcm_switch_control_set(unit, bcmSwitchL3RouteCache, batch);
  if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_set l3cache failed err=%d %s\n", status,
              bcm_errmsg(status));
      sal_fclose(fp);
      return CMD_FAIL;

  }

  st = sal_time_usecs();

#if 1
  bcm_l3_intf_t_init(&l3_intf);

  l3_intf.l3a_mac_addr[0] = 0x00;
  l3_intf.l3a_mac_addr[1] = 0x00;
  l3_intf.l3a_mac_addr[2] = 0x00;
  l3_intf.l3a_mac_addr[3] = 0x00;
  l3_intf.l3a_mac_addr[4] = 0x00;
  l3_intf.l3a_mac_addr[5] = 0x33;
  l3_intf.l3a_vid        =  3;

  status = bcm_l3_intf_create(unit, &l3_intf);
  if (BCM_FAILURE(status)) {
    cli_out("create l3 interface failed err=%d %s\n", status, bcm_errmsg(status));
    sal_fclose(fp);
    return CMD_FAIL;
  }

  bcm_l3_egress_t_init(&l3_egr);

  l3_egr.mac_addr[0] = 0x00;
  l3_egr.mac_addr[1] = 0x00;
  l3_egr.mac_addr[2] = 0x00;
  l3_egr.mac_addr[3] = 0x00;
  l3_egr.mac_addr[4] = 0x00;
  l3_egr.mac_addr[5] = 0x11;
  l3_egr.module = 0;
  l3_egr.port   = 0;
  l3_egr.intf   = l3_intf.l3a_intf_id;

  status = bcm_l3_egress_create(unit, 0, &l3_egr, &l3_egr_id);
  if (BCM_FAILURE(status)) {
    cli_out("create l3 egress failed err=%d %s\n", status, bcm_errmsg(status));
    sal_fclose(fp);
    return CMD_FAIL;
  }
#endif

  while ((status = bcm_test_v4_load_pfx(fp, &vrf, &prefix, &prefixMask)) == BCM_E_NONE) {
    bcm_l3_route_t_init(&route_info);
    
    if (prefix == 0 && prefixMask == 0 && vrf != 0 && vrf != 1) {
      /* this is a signal for a new VRF */
      bcm_mpls_vpn_config_t vpn_config;
      bcm_l3_egress_t             l3_nexthop_port1;
      bcm_if_t                    l3_nexthop_port1_id;
      bcm_l3_egress_t             l3_nexthop_port2;
      bcm_if_t                    l3_nexthop_port2_id;
      int port_a = 0;
      int port_b = 0;

      status = bcm_vlan_create(unit, vrf);
      if (status != BCM_E_NONE && status != BCM_E_EXISTS) {
	printf("bcm_vlan_create failed: %s\n", bcm_errmsg(status));
	break;
      }

      bcm_mpls_vpn_config_t_init(&vpn_config);
      vpn_config.flags = BCM_MPLS_VPN_L3;
      vpn_config.vpn   = vrf;
      /*vpn_config.flags |= BCM_MPLS_VPN_WITH_ID;*/
      vpn_config.lookup_id = vrf;
      status = bcm_mpls_vpn_id_create (unit, &vpn_config);
      if (status != BCM_E_NONE && status != BCM_E_EXISTS && status != BCM_E_BUSY) {
	printf("bcm_mpls_vpn_id_create failed: %d (%s)\n", status, bcm_errmsg(status));
	break;
      }

      /* Create L3 interface */
      bcm_l3_intf_t_init(&l3_intf);
      l3_intf.l3a_mac_addr[0] = 0x00;
      l3_intf.l3a_mac_addr[1] = 0xaa;
      l3_intf.l3a_mac_addr[2] = 0x99;
      l3_intf.l3a_mac_addr[3] = 0x88;
      l3_intf.l3a_mac_addr[4] = 0x77;
      l3_intf.l3a_mac_addr[5] = 0x66;
      l3_intf.l3a_vid = vrf;
      l3_intf.l3a_mtu = 9000;
      l3_intf.l3a_ttl = 37;
      l3_intf.l3a_vrf = vrf;
      status = bcm_l3_intf_create(unit, &l3_intf);
      if (status != BCM_E_NONE) {
	printf("setup_l3: bcm_l3_intf_create failed: %s\n", bcm_errmsg(status));
	break;
      }
      printf("setup_l3: Created l3 interface 0x%x\n", l3_intf.l3a_intf_id);

      /* Create L3 nexthop egress. This is used to forward traffic */
      bcm_l3_egress_t_init(&l3_nexthop_port1);
      l3_nexthop_port1.intf = l3_intf.l3a_intf_id;
      l3_nexthop_port1.mac_addr[0] = 0x00;
      l3_nexthop_port1.mac_addr[1] = 0xaa;
      l3_nexthop_port1.mac_addr[2] = 0x99;
      l3_nexthop_port1.mac_addr[3] = 0x88;
      l3_nexthop_port1.mac_addr[4] = 0x77;
      l3_nexthop_port1.mac_addr[5] = 0x66;
      l3_nexthop_port1.port = port_b;
      status = bcm_l3_egress_create(unit, 0, &l3_nexthop_port1, &l3_nexthop_port1_id);
      if (status != BCM_E_NONE) {
        printf("setup_l3: bcm_l3_egress_create (forward) failed: %s\n", bcm_errmsg(status));
	break;
      }
      printf("setup_l3: Created l3 nexthop egress (port %d) 0x%x\n", port_b, l3_nexthop_port1_id);

      /* Create L3 nexthop egress. This is used to forward traffic */
      bcm_l3_egress_t_init(&l3_nexthop_port2);
      l3_nexthop_port2.intf = l3_intf.l3a_intf_id;
      /* These two flags work */
      l3_nexthop_port2.mac_addr[0] = 0x00;
      l3_nexthop_port2.mac_addr[1] = 0x11;
      l3_nexthop_port2.mac_addr[2] = 0x22;
      l3_nexthop_port2.mac_addr[3] = 0x33;
      l3_nexthop_port2.mac_addr[4] = 0x44;
      l3_nexthop_port2.mac_addr[5] = 0x02;
      l3_nexthop_port2.port = port_a;  /* send port 0 */
      status = bcm_l3_egress_create(unit, 0, &l3_nexthop_port2, &l3_nexthop_port2_id);
      if (status != BCM_E_NONE) {
        printf("setup_l3: bcm_l3_egress_create (forward) failed: %s\n", bcm_errmsg(status));
	break;
      }
      printf("setup_l3: Created l3 nexthop egress (port 2) 0x%x\n", l3_nexthop_port2_id);
    }

    route_info.l3a_vrf = vrf;
    route_info.l3a_subnet   = prefix;
    route_info.l3a_ip_mask  = bcm_ip_mask_create(prefixMask);
    route_info.l3a_intf     = l3_egr_id;

    status = bcm_l3_route_add(unit, &route_info);

    if (status != BCM_E_NONE && status != BCM_E_EXISTS && status != BCM_E_FULL) {
      cli_out("bcm_l3_route_add failed, error %d (%s)\n", status, bcm_errmsg(status));
      break;
    }

    if (status == BCM_E_EXISTS || status == BCM_E_FULL) {
      continue;
    }
    routes++;

#ifdef  BCM_CALADAN3_G3P1_SUPPORT
    if(batch)
    {
        if(routes%flush_cnt == 0)
        {
            status = bcm_switch_control_set(unit, bcmSwitchL3RouteCommit, TRUE);
            if (BCM_FAILURE(status)){
                cli_out("bcm_switch_control_set l3cache Flush failed err=%d %s\n", status,
                        bcm_errmsg(status));
                sal_fclose(fp);
                return CMD_FAIL;
            }
        }
    }
#endif
    if ((max_entries > 0) && (routes >= max_entries)) {
        break;
    }

    if ((routes % 1000) == 0) {
        e = sal_time_usecs();
        d_usec = SAL_USECS_SUB(e, st);
        d_sec = d_usec / 1000000;
        cli_out("Adding %d IP Addresses in %u microseconds (%d Entries/sec)\n",
                routes, d_usec, (d_sec)?routes/d_sec:routes);
    }
  }


  if (batch) {
    status = bcm_switch_control_set(unit, bcmSwitchL3RouteCommit, 0);
    if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_set l3routecommit failed err=%d %s\n", status,
              bcm_errmsg(status));
      sal_fclose(fp);
      return CMD_FAIL;
      
    }
  }

  e = sal_time_usecs();
  d_usec = SAL_USECS_SUB(e, st);
  d_sec = d_usec / 1000000;
  cli_out("Added %d IP Addresses in %u microseconds (%d Entries/sec)\n",
          routes, d_usec, (d_sec)?routes/d_sec:routes);

  status = bcm_switch_control_set(unit, bcmSwitchL3RouteCache, original_batch_mode);
  if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_set l3routecache failed err=%d %s\n", status,
              bcm_errmsg(status));
      sal_fclose(fp);
      return CMD_FAIL;
  }

  sal_fclose(fp);
  return CMD_OK;
}


char cmd_sbx_learn_usage[] =
"Usage:\n"
"learn [options]\n"
"  enable learning\n"
"Options: \n"
"  start       - start learning\n"
"  echo        - start receiving, but echo packets instead of learn\n"
"  stop        - stop learning\n"
"  age         - get age entry count\n"
;

cmd_result_t
cmd_sbx_learn(int unit, args_t *a)
{
    int i;
    int rv;
    int d = -1;
    int start = -1;
    int echo = 0;
    int stop = -1;
    int sync = 0;
    int ilib = 0;
    int ip = 0;
    int max_entries = -1;
    int age = -1;
    int cb = -1;

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "start", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &start, NULL);
        parse_table_add(&pt, "echo", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &echo, NULL);
        parse_table_add(&pt, "stop", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &stop, NULL);
        parse_table_add(&pt, "debug", PQ_DFL | PQ_INT, 0, &d, NULL);
        parse_table_add(&pt, "sync", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &sync, NULL);
        parse_table_add(&pt, "ilib", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ilib, NULL);
        parse_table_add(&pt, "ip", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &ip, NULL);
        parse_table_add(&pt, "maxentries", PQ_DFL | PQ_INT,
                        0, &max_entries, NULL);
        parse_table_add(&pt, "age", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &age, NULL);
        parse_table_add(&pt, "cb", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &cb, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }

    if (start != -1 && stop != -1) {
        cli_out("can only specify one of start or stop\n");
        return CMD_USAGE;
    }

    if (stop == -1) {/* echo falls through here */
        start = 1;
        stop = 0;
    } else {
        start = 0;
        stop = 1;
    }

    if (d != -1) {
        learn_debug = d;
        return CMD_OK;
    }

    if (fe_unit == -1 || qe_unit == -1) {
        for (i = 0; i < soc_all_ndev; i++) {
            if (SOC_IS_SBX_QE2000(i)) {
                qe_unit = i;
            }
		    if (SOC_IS_SBX_CALADAN3(unit)) {
		      	fe_unit = qe_unit = i;
		    }
        }
    }
	
	if (age != -1) {
		if (cb != -1) {
			rv = bcm_l2_addr_register(qe_unit, _l2_age_cb, NULL);
			if (BCM_FAILURE(rv)) {
				cli_out("ERROR: bcm_l2_addr_register failed %d\n",
                                        rv);
				return CMD_FAIL;
			}
		} else {
			cli_out("Age callback called %d times vs L2 learned %d times \n",
                                age_entry_num, learn_count);
			/* reset counts */
			age_entry_num = 0;
			learn_count = 0;
		}

		return CMD_OK;
	}

    if (qe_unit == -1) {
        cli_out("QE unit not found\n");
        return CMD_FAIL;
    }
    if (fe_unit == -1) {
        cli_out("FE unit not found\n");
        return CMD_FAIL;
    }

    if (stop) {
        if (!learning_running) {
            cli_out("learning thread is not running\n");
            return CMD_FAIL;
        }
		else {
			rv = bcm_rx_unregister(qe_unit, _learn_rx_cb, 0x40);
			if (BCM_FAILURE(rv)) {
		        cli_out("ERROR: bcm_rx_unregister(%d) failed:%d:%s\n",
                                qe_unit, rv, bcm_errmsg(rv));
		        return CMD_FAIL;
    		}
			learning_running = 0;
		}

    } else {
        if(echo){
		  cli_out("ECHO is enabled\n");
		  echo_instead_of_learn=1;
		}else{
		  echo_instead_of_learn=0;
		}
	    if (!bcm_rx_active(qe_unit)) {
	        rv = bcm_rx_init(qe_unit);
	        if (BCM_FAILURE(rv)) {
	            cli_out("ERROR:  bcm_rx_init(%d) returns %d: %s\n",
                            qe_unit, rv, bcm_errmsg(rv));
	            return CMD_FAIL;
	        }
	        rv = bcm_rx_start(qe_unit, 0);
	        if (BCM_FAILURE(rv)) {
	            cli_out("ERROR: bcm_rx_start(%d) failed:%d:%s\n",
                            qe_unit, rv, bcm_errmsg(rv));
	            return CMD_FAIL;
	        }
	    }

	    rv = bcm_rx_register(qe_unit, "learn CB", _learn_rx_cb, 0x40, NULL, BCM_RCO_F_ALL_COS);
	    if (BCM_FAILURE(rv)) {
	        cli_out("ERROR: bcm_register(%d) failed:%d:%s\n",
                        qe_unit, rv, bcm_errmsg(rv));
	        return CMD_FAIL;
    	}

    	learning_running = 1;
    }

    return CMD_OK;
}

#define DEFAULT_NUM_PREFIX 1000
#define DEFAULT_NUM_BATCH 10
#define MAX_RAND_RETRY (5)
#define DEF_START_PFX  (0x01000000) /* 1.0.0.0 */

#define MIN_PFX_SLICE    (8)
#define MAX_PFX_SLICE    (32)
#define MAX_SLICE_RANGE  (MAX_PFX_SLICE - MIN_PFX_SLICE + 1)
#define MIN_V6_PFX_SLICE    (8)
#define MAX_V6_PFX_SLICE    (64)
#define MAX_V6_SLICE_RANGE  (MAX_V6_PFX_SLICE - MIN_V6_PFX_SLICE + 1)
#define DEF_PFX_SLICE    (24)
#define MAX_SLICE_WEIGHT (200)

#define DEF_SEQ_START_PFX_LEN    (24)
#define DEF_V6_SEQ_START_PFX_LEN    (32)
#define DEF_V6_START_PFX  { 1, 0, 0, 0, 0, 0, 0, 0}  /* 1::/64 */
#define MIN_NUM_NEXT_HOP    (1)
#define MAX_NUM_NEXT_HOP    (16*1024)

/* V4:
 *    Default route prefix distribution weights 8 - 32
 *    Put majority on /24 and /16 
 * V6: 
 *   Default route prefix distribution weights 8 - 64
 */

/* Space set to max required */
int sliceWeight[MAX_V6_SLICE_RANGE];
#define LPM_DEF_FILE_NAME "route.txt"

typedef struct bcm_test_lpm_arg_s{
  int numroutes;
  int numvrf;
  int random;
  int rndlen;
  int batch;
  int numbatch;
  int seq;
  int seqmask;
  int seqpfx;
  int ilib;
  int load;
  char *fname;
  int debug;
  int noadd;
  int nodel;
  int numnhop;
  int stats;
  int verbose;
  int clear;
  int rpf;
  int ipv6;
  int ipv6em;
  bcm_ip6_t v6seqpfx;
  int v6masklen;
} bcm_test_lpm_arg_t;

static
void bcm_test_lpm_arg_init (bcm_test_lpm_arg_t *pArgs)
{
  bcm_ip6_t def_ipv6_pfx = DEF_V6_START_PFX;
  SB_ASSERT(pArgs);
  pArgs->numroutes = DEFAULT_NUM_PREFIX;
  pArgs->random    = 0;
  pArgs->rndlen    = 0;
  pArgs->batch     = 0;
  pArgs->seq       = 0;
  pArgs->seqmask   = DEF_SEQ_START_PFX_LEN;
  pArgs->seqpfx    = DEF_START_PFX;
  pArgs->numbatch  = DEFAULT_NUM_BATCH;
  pArgs->ilib      = 0;
  pArgs->load      = 0;
  pArgs->fname     = LPM_DEF_FILE_NAME;
  pArgs->debug     = 0;
  pArgs->noadd     = 0;
  pArgs->nodel     = 0;
  pArgs->numnhop   = MIN_NUM_NEXT_HOP;
  pArgs->numvrf   = 1;
  pArgs->stats     = 0;
  pArgs->verbose   = 0;
  pArgs->rpf       = 0;
  pArgs->clear     = 0;
  pArgs->ipv6 = 0;
  pArgs->ipv6em = 0;
  sal_memcpy(pArgs->v6seqpfx, def_ipv6_pfx, sizeof(bcm_ip6_t));
  pArgs->v6masklen  = DEF_V6_SEQ_START_PFX_LEN;
}

static int bcm_test_lpm_validate_args(bcm_test_lpm_arg_t *pArgs)
{
  /* if randomization & seq not set promote Seq as default method 
   * for prefix generation 
   */
  if(!(pArgs->seq | pArgs->random)){
    pArgs->seq = 1;
  }

  if ((pArgs->ipv6) && (pArgs->v6masklen == 128)) {
      /* EM enabled, prefix weightage will be bypassed */
      pArgs->ipv6em = 1;
  }
  if(pArgs->seq){
    if (pArgs->ipv6) {
      if ((!pArgs->ipv6em) && ((pArgs->v6masklen < MIN_V6_PFX_SLICE) || (pArgs->v6masklen > MAX_V6_PFX_SLICE))) {
        cli_out("\n Bad Sequencing Prefix Mask Length. Must be between [8-64]!!\n");
        return CMD_FAIL;
      }
    } else {
      if(pArgs->seqmask < MIN_PFX_SLICE || pArgs->seqmask > MAX_PFX_SLICE){
        cli_out("\n Bad Sequencing Prefix Mask Length. Must be between [8-32]!!\n");
        return CMD_FAIL;
      }
    }
  }

  if(pArgs->seq & pArgs->random){
    cli_out("\n Sequencing & Randomization are Mutually Exclusive cant use them together");
    return CMD_FAIL;
  }


  if(pArgs->numnhop < MIN_NUM_NEXT_HOP || pArgs->numnhop > MAX_NUM_NEXT_HOP) {
    cli_out("\n Bad Number of Next hop Supported Value: [%d] - [%d]",
            MIN_NUM_NEXT_HOP, MAX_NUM_NEXT_HOP);
    return CMD_FAIL;
  }
  if(pArgs->numvrf <=0 || pArgs->numvrf > SBX_MAX_VRF) {
    cli_out("\n Bad Number of vrfs (max supported [%d])",
            SBX_MAX_VRF);
    return CMD_FAIL;
  }

  return CMD_OK;
}

#if 0
static void bcm_test_v4_lpm_adjust_weight(uint32 *maskIndex, uint32 *maskWeight, uint32 *pfxMask)
{
  if(*maskWeight == 0){
    if(*maskIndex < MAX_SLICE_RANGE-1) {
      *maskIndex += 1;
    } else {
      *maskIndex = 0;
    }
    SB_ASSERT(*maskIndex >=0 && *maskIndex < MAX_SLICE_RANGE);
    *maskWeight = sliceWeight[*maskIndex];
    SB_ASSERT(*maskWeight);
    *pfxMask = *maskIndex + MIN_PFX_SLICE;
  } else {
    *pfxMask = *maskIndex + MIN_PFX_SLICE;
  }
  *maskWeight -= 1;
}
#endif

static
void bcm_v6_incr_pfx_mask(uint32 *pfxLen)
{
    SB_ASSERT(pfxLen);
    /* Prefix length ran out of address, move to next prefix length */
    if(*pfxLen == MAX_V6_PFX_SLICE){
      *pfxLen = MIN_V6_PFX_SLICE;
    } else {
      *pfxLen += 1;
    }
}
static
void bcm_v4_incr_pfx_mask(uint32 *pfxLen)
{
    SB_ASSERT(pfxLen);
    /* Prefix length ran out of address, move to next prefix length */
    if(*pfxLen == MAX_PFX_SLICE){
      *pfxLen = MIN_PFX_SLICE;
    } else {
      *pfxLen += 1;
    }
}
int bcm_test_v6_lpm_inc_pfx(uint8* prefix, uint8* mask, int i)
{
    unsigned int val, rem;
    unsigned char inc;
    val = prefix[i] & mask[i];
    rem = prefix[i] & ~mask[i];
    inc = ~mask[i] + 1;
    val += inc;
    val += rem;
    if (val > 255) {
        if ((i<=0) || (bcm_test_v6_lpm_inc_pfx(prefix, mask, i-1)!=0)) {
            return 1;
        }
    }
    prefix[i] = val & 0xFF;
    return 0;
}
void print_ip6_host(char*msg, uint8 *addr)
{
   char buffer[256];
   /* coverity[secure_coding] */
   sal_sprintf(buffer,"%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], addr[8], addr[9], addr[10], addr[11], addr[12], addr[13], addr[14], addr[15]);
   cli_out("\n%s %s",msg, buffer);
}

void print_ip6_pfx(char*msg, uint8 *addr, int masklen)
{
   char buffer[256];
   /* coverity[secure_coding] */
   sal_sprintf(buffer,"[%02x%02x:%02x%02x:%02x%02x:%02x%02x]/%d", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5], addr[6], addr[7], masklen);
   cli_out("\n%s %s",msg, buffer);

}

void bcm_test_v6_lpm_inc_seq_pfx(uint8 *prefix, int masklen)
{
  int bytes;
    bcm_ip6_t mask;
    /* Check for mcast */
    if (prefix[0] == 0xFF) {
        cli_out("\n Invalid prefix for LPM");
        return;
    }

    /* Validate mask */
    bytes = masklen >> 3;
    bcm_ip6_mask_create(mask, masklen);
    while (bcm_test_v6_lpm_inc_pfx(prefix, mask, bytes-1)) {
        /* Exceeded range, change mask */
        masklen++;
        if (masklen > MAX_V6_PFX_SLICE)  {
            masklen = MIN_V6_PFX_SLICE;
        }
        bcm_test_v6_lpm_inc_seq_pfx(prefix, masklen);
    }
}
        
static void bcm_test_v4_lpm_adjust_seq_pfx(uint32 *prefix, uint32 *pfxLen)
{
  uint32 nbytes, bits;
  int bIndex, dirty;
  uint32 tmpAddr, value;
#define LPM_V4_GEN_MASK (val) ((1<<(val))-1)

  SB_ASSERT(prefix);
  SB_ASSERT(pfxLen);

  /* Only Network Portion of the Prefix will be incremented */
  /* Ip address
   * | byte 0| byte 1| byte 2| byte 3|
   */
  *prefix &= bcm_ip_mask_create(*pfxLen); /* bcm macro */
  nbytes = *pfxLen / 8;
  bits  = *pfxLen - nbytes * 8;
  *prefix = *prefix >> (MAX_PFX_SLICE - *pfxLen);
  SB_ASSERT( nbytes > 0 || bits > 0);
  (*prefix)++;
  tmpAddr = *prefix >> bits;
  dirty = 0;

  /* Check if Bytes of any prefix is 0 or 255
   * Dont admit FF addresses & prefix 0 for First Byte
   * of IP address
   */
  for(bIndex = nbytes-1; bIndex >= 0; bIndex--) {
    value = tmpAddr >> (8 * (nbytes - bIndex -1));
    value &= 0xFF;

    if(value == 0xFF){
      if(!bIndex){
        /* Prefix length ran out of address, move to next prefix length */
        bcm_v4_incr_pfx_mask(pfxLen);
        tmpAddr = 1 << ((nbytes-1)*8);
        tmpAddr <<= 1;
      } else {
        /* Advance to non 0xFF prefix */
        tmpAddr++;
      }
      dirty = 1;
    } else if(!value && !bIndex) {
      /* dont have 0 starting prefix */
      tmpAddr |= 0x1 << ((nbytes-1)*8);
      dirty = 1;
    }
  }

  if(dirty){
    *prefix = tmpAddr << bits;
  } else if(!nbytes){
    /* for shorter prefix len eg., 8 the network byte could have reached 0xff
     * check this and increment prefix length if required */
    if(*prefix == 0xFF) {
      *pfxLen += 1;
      *prefix = 2;
    }
  }

  *prefix = *prefix << (MAX_PFX_SLICE - *pfxLen);
}

static int bcm_test_gen_mac(uint8 *mac, int random)
{
  /* Sequential Mac */
  if(!random) {
      if(mac[5] >= 255){
        mac[5] = 0;
        mac[4]++;

        if(mac[4] >= 255){
            mac[4] = 0;
            mac[3]++;

            if(mac[3] >= 255){
              mac[3] = 0;
              mac[2]++;

              if(mac[2] >= 255) {
                mac[2] = 0;
                mac[1]++;

                if(mac[1] >= 255){
                    mac[1] = 0;
                    mac[0] >>= 1;
                    mac[0]++;

                    /* dont generte mcast mac*/
                    if(mac[0] >= 127){
                      mac[0] = 0;
                      mac[0] <<= 1;
                    }
                }
              }
            }
        }
      } else {
        mac[5]++;
      }
  } else {
      uint64 randmac;
      COMPILER_64_SET(randmac,sal_rand(),sal_rand());
      mac[0] =  COMPILER_64_LO(randmac) & 0xff;
      mac[1] = (COMPILER_64_LO(randmac) >> 8) & 0xff;
      mac[2] = (COMPILER_64_LO(randmac) >> 16) & 0xff;
      mac[3] = (COMPILER_64_LO(randmac) >> 24) & 0xff;
      mac[4] =  COMPILER_64_HI(randmac) & 0xff;
      mac[5] = (COMPILER_64_HI(randmac) >> 8) & 0xff;
  }
  return BCM_E_NONE;
}

#ifdef BCM_CALADAN3_G3P1_SUPPORT
static int bcm_test_gen_mac_vsi(uint8 *mac, uint32 *vsi, int random)
{

    if (!mac || !vsi) return CMD_FAIL;

    if (bcm_test_gen_mac(mac, random) == 0) {
        if (random) {
            *vsi = sal_rand() & ((1<<16)-1);
        } else {
            if (mac[0] == 0xFF && 
                mac[1] == 0xFF &&
                mac[2] == 0xFF &&
                mac[3] == 0XFF &&
                mac[4] == 0xFF &&
                mac[5] == 0xFF) {
                (*vsi)++;
            }
        }
    } else {
        return CMD_FAIL;
    }

    return CMD_OK;
}
#endif

uint32 bcm_test_v4_ipstr2int(char *cip)
{
  char *dptr, *ptr;
  uint32 ip=0, ipbyte=0;
  uint32 dotcnt=0, done=0;

  ptr = cip;

  while(done == 0)
  {
      if((dptr = strchr(ptr,'.')) != NULL) {
        dotcnt++;
        while(ptr != dptr) {
          ipbyte = (ipbyte * 10) + (*ptr - '0');
          ptr++;
        }
      } else {
        while(*ptr != '\0'){
          ipbyte = (ipbyte * 10) + (*ptr - '0');
          ptr++;
        }
        done = 1;
      }

      ip = (ip << 8)   + ipbyte;
      ptr++;
      ipbyte = 0;
  }
  /* take the last octet between last do and null */

  if(dotcnt != 3) {
      return -1;
  }

  return ip;
}

static
int  bcm_test_v4_load_flen(FILE *fp)
{
#ifndef __KERNEL__
  char   buf[256];
#endif
  int    flen = 0;
  char   *tokstr=NULL;

  SB_ASSERT(fp);

#ifndef __KERNEL__
  while(fgets(buf, sizeof(buf), fp)){
      if(sal_strtok_r(buf,"/", &tokstr)){
          flen++;
      }
  }
#endif
  return flen;
}

static
int  bcm_test_v4_load_pfx(FILE *fp, int *vrf, uint32 *prefix, uint32 *pfxLen)
{
#ifndef __KERNEL__
  char   buf[256];
  char   *ptr;
  uint32 pfx;
#endif
  int    status = BCM_E_INTERNAL;
  int    vrf_given;
  char   *tokstr=NULL;
  char   *tokstr_1=NULL;

#ifndef __KERNEL__
  SB_ASSERT(fp);
  SB_ASSERT(prefix);
  SB_ASSERT(pfxLen);

  while(1) {
      /* Get acceptable pfx/mask pair from file or keep moving to
       * next lines */
      if(fgets(buf, sizeof(buf), fp)){
           /* chomp /n */
          ptr = &buf[0] + strlen(buf);
          *(ptr-1) = '\0';

	  if (buf[0] == '#') 
	    continue;

	  ptr = sal_strtok_r(buf, ":", &tokstr);
	  if (ptr) {
	    vrf_given = atoi(ptr);
	    *vrf = vrf_given % SBX_MAX_VRF;
	    ptr += strlen(ptr) + 1;
	  } else {
	    ptr = buf;
	  }
	   
          ptr = sal_strtok_r(ptr,"/", &tokstr_1);
          if (ptr) {
              pfx = bcm_test_v4_ipstr2int(ptr);
          } else {
              status = BCM_E_PARAM;
              break;
          }

          *prefix = pfx;
          ptr = sal_strtok_r(NULL,"/", &tokstr_1);
          if (ptr) {
              *pfxLen = atoi(ptr);
              status = BCM_E_NONE;
          } else {
              status = BCM_E_PARAM;
          }
	  /*
	  cli_out("loading: vrf %d=>%d %u.%u.%u.%u/%u\n", vrf_given, *vrf, 
                  *prefix>>24, (*prefix>>16)&0xff, (*prefix>>8) & 0xff, *prefix&0xFF, *pfxLen);
		       */
          break;
      } else {
        status = BCM_E_EMPTY;
        break;
      }
  }
#endif

  return status;
}

static
int bcm_test_lpm_capacity (int unit, bcm_test_lpm_arg_t *pArgs)
{
  bcm_l3_intf_t      l3_intf;
  bcm_l3_egress_t    l3_egr;
  bcm_if_t           *l3_egr_id=NULL;
  bcm_if_t           *pivot_egr_id=NULL;
  bcm_mac_t          mymac;
  bcm_l3_route_t     route_info;
  int                status;
  int flag;
  uint32             routes = 0, routesPerBatch=0;
  sal_usecs_t        start, end, accum, bstart=0, bend=0, baccum=0;
  int                retry=0, index=0, cache=0, batchIndex=0;
  uint32             prefix = DEF_START_PFX; /* 1.0.0.0 */
  uint32             prefixMask = DEF_SEQ_START_PFX_LEN;

  bcm_ip6_t          v6prefix = DEF_V6_START_PFX;
  uint32             v6masklen = DEF_V6_SEQ_START_PFX_LEN;
#if 0
  uint32             maskIndex=0, maskWeight=0;
#endif
  uint32             prefixSeed = sal_rand();
  uint32             pfxPerVrf=0, pfxPerNhop=0, pfxLenRetry;
  FILE               *fp=NULL;
  int                vrf_id = 0;
  uint8              first;
  int i;
#ifdef BCM_FE2000_P3_SUPPORT
  void               *da_cstate;
  void               *sa_cstate;
#endif
  uint32             slice_range;

  SB_ASSERT(pArgs);

  if( !SOC_SBX_INIT(unit) ){
    cli_out("Unit %d, not initialized - call 'init soc' first!\n", unit);
    return CMD_FAIL;
  }
  if (!SOC_IS_CALADAN3(unit)) {
    cli_out("only supported on Caladan3\n");
    return CMD_FAIL;
  }

switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_FE2000_P3_SUPPORT
case SOC_SBX_UCODE_TYPE_G2P3:

  if (pArgs->ipv6) {
    if (pArgs->ipv6em) {
      da_cstate = soc_sbx_g2p3_ipv6dhost_complex_state_get(unit);
      sa_cstate = soc_sbx_g2p3_ipv6shost_complex_state_get(unit);
    } else {
      da_cstate = soc_sbx_g2p3_ipv6da_complex_state_get(unit);
      sa_cstate = soc_sbx_g2p3_ipv6sa_complex_state_get(unit);
    }
  } else {
    da_cstate = soc_sbx_g2p3_ipv4da_complex_state_get(unit);
    sa_cstate = soc_sbx_g2p3_ipv4sa_complex_state_get(unit);
  }
  if(pArgs->stats) {
      /* BOGUS */
      if(pArgs->clear) {
          if (soc_sbx_g2p3_lpm_stats_clear(unit, da_cstate)) {
              cli_out("ip da stats clear failed\n");
              return CMD_FAIL;
          }
          if (soc_sbx_g2p3_lpm_stats_clear(unit, sa_cstate)) {
              cli_out("ip sa stats clear failed\n");
              return CMD_FAIL;
          }
      } else {
          if (soc_sbx_g2p3_lpm_stats_dump(unit, da_cstate, pArgs->verbose)) {
              cli_out("ip da stats dump failed\n");
              return CMD_FAIL;
          }
          if (soc_sbx_g2p3_lpm_stats_dump(unit, sa_cstate, pArgs->verbose)) {
              cli_out("ip sa stats dump failed\n");
              /*return CMD_FAIL;*/
          }
      }

      return CMD_OK;
  }
  break;
#endif
#ifdef BCM_CALADAN3_G3P1_SUPPORT
case SOC_SBX_UCODE_TYPE_G3P1:
  break;
#endif
default:
  cli_out("ERROR: unsupported microcode type: %d\n",
          SOC_SBX_CONTROL(unit)->ucodetype);
  return CMD_FAIL;
}

  /* Load Prefix from file if file name specified */
  if(pArgs->load){
    if (load_routes(unit, "route.txt", pArgs->numroutes, 0) == CMD_OK)
      return CMD_OK;

      if (pArgs->ipv6) {
          cli_out("\n Load not supported for IPv6");
          return CMD_FAIL;
      }
      if(!(fp = sal_fopen(pArgs->fname, "r"))){
          cli_out("\n Cant open File[%s]", pArgs->fname);
          return CMD_FAIL;
      } else {
          /* Find number of prefix on the file */
          pArgs->numroutes = bcm_test_v4_load_flen(fp);
      }
      if(pArgs->numroutes <= 0) {
          cli_out("\n File contains no valid Prefix/Mask pair");
          sal_fclose(fp);
          return CMD_FAIL;
      } else {
#ifndef __KERNEL__
          cli_out("\n File contains [%d] valid Prefix/Mask pair", pArgs->numroutes);
          rewind(fp);
#endif
      }
  }

  if (pArgs->debug) {
        cli_out("Adding new IP Addresses using BCM API\n");
        if (pArgs->numroutes > 0)
            cli_out("Measuring for up to %d IP entries...\n",
                    pArgs->numroutes);
  }
  cli_out("\n --------------------");
  cli_out("\n Adding IP entries");
  cli_out("\n --------------------");

  /* No slices for EM 128bit routes */
  if (!pArgs->ipv6em) {
    /* Assign default prefix mask weights = 1 */
    slice_range = (pArgs->ipv6) ? MAX_V6_SLICE_RANGE : MAX_SLICE_RANGE; 
    for(index=0; index < slice_range; index++){
      /* If random length is selected use random weights for prefix length */
      if(pArgs->rndlen){
        sliceWeight[index] = sal_rand() % MAX_SLICE_WEIGHT;
      } else {
        /* Default Weights */
        sliceWeight[index] = 1;
      }
    }

    if(pArgs->rndlen){
      if (!pArgs->ipv6) {
        /* Assign more weights to /16 & /24 */
        /* Default Weights */
        sliceWeight[16-MIN_PFX_SLICE] = 20;
        sliceWeight[24-MIN_PFX_SLICE] = 30;
      }
    }
  }

  status = bcm_switch_control_get(unit, bcmSwitchL3RouteCache, &cache);
  if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_get l3cache failed err=%d %s\n", status,
              bcm_errmsg(status));
      if(pArgs->load){
        sal_fclose(fp);
      }
      return CMD_FAIL;

  }

  status = bcm_switch_control_set(unit, bcmSwitchL3RouteCache, pArgs->batch);
  if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_set l3cache failed err=%d %s\n", status,
              bcm_errmsg(status));
      if(pArgs->load){
        sal_fclose(fp);
      }
      return CMD_FAIL;

  }

  sal_memset(&mymac, 0, sizeof(bcm_mac_t));

  /* Allocate Next Hop */
  l3_egr_id = (bcm_if_t*) sal_alloc(sizeof(bcm_if_t) * pArgs->numnhop, "LPM tool Nhops");

  if(!l3_egr_id) {
      cli_out("LPM tool Error allocating Next hops: out of memory \n");
  } else {

      start = sal_time_usecs();
      bcm_l3_intf_t_init(&l3_intf);
      bcm_test_gen_mac(&mymac[0], 0);
      l3_intf.l3a_vid        =  3;
      sal_memcpy(&l3_intf.l3a_mac_addr, &mymac, sizeof(bcm_mac_t));

      status = bcm_l3_intf_create(unit, &l3_intf);
      if (BCM_FAILURE(status)) {
        cli_out("create l3 interface failed err=%d %s\n", status, bcm_errmsg(status));
        sal_free(l3_egr_id);
        if(pArgs->load){
          sal_fclose(fp);
        }
        return CMD_FAIL;
      }

      if(pArgs->debug){
          cli_out("\n Create Interface[0x%x]", l3_intf.l3a_intf_id);
      }

      for(index=0; index < pArgs->numnhop; index++) {

          bcm_l3_egress_t_init(&l3_egr);
          bcm_test_gen_mac(&mymac[0], 0);
          sal_memcpy(&l3_egr.mac_addr, &mymac, sizeof(bcm_mac_t));
          l3_egr.module = 0;
          l3_egr.port   = 0;
          l3_egr.intf   = l3_intf.l3a_intf_id;

          status = bcm_l3_egress_create(unit, 0, &l3_egr, l3_egr_id + index);
          if (BCM_FAILURE(status)) {
            while(index--){
              bcm_l3_egress_destroy(unit, *(l3_egr_id+index));
            }
            sal_free(l3_egr_id);
            cli_out("create l3 egress failed err=%d %s\n", status, bcm_errmsg(status));
            bcm_l3_intf_delete(unit, &l3_intf);
            if(pArgs->load){
             sal_fclose(fp);
            }
            return CMD_FAIL;
          }

          if(pArgs->debug){
              cli_out("\n Create Egress Interface[0x%x]", *(l3_egr_id + index));
          }
      }

      end = sal_time_usecs();
      cli_out("\n L3 Interface Creation Time %u", SAL_USECS_SUB(end,start));

      /* Compute Prefix per next hop */
      pfxPerNhop = pArgs->numroutes/pArgs->numnhop;

      /* Compute Prefix per VRF  */
      pfxPerVrf = pArgs->numroutes/pArgs->numvrf;
      

      /* Set the Seed for predictable sequence */
      if(pArgs->random) {
        sal_srand(prefixSeed);
      }

      if(pArgs->batch) {
          routesPerBatch = pArgs->numroutes / pArgs->numbatch;
      }

      accum      = 0;
      baccum     = 0;
#if 0
      maskIndex  = 0;
      maskWeight = sliceWeight[0];
#endif
      retry      = 0;
      pfxLenRetry = 0;
      prefix     = pArgs->seqpfx;
      prefixMask = pArgs->seqmask;
      sal_memcpy(v6prefix, pArgs->v6seqpfx, sizeof(bcm_ip6_t));
      v6masklen = pArgs->v6masklen;
      batchIndex = 0;
      first = 1;
      pivot_egr_id = l3_egr_id;
          cli_out("\n Time Stamp Before Route Add [%u]", sal_time_usecs());
      if (pArgs->debug) {
        if (pArgs->ipv6) 
          cli_out("\nRt Add Start PfxLen [%d] slice", v6masklen);
        else 
          cli_out("\nRt Add Start PfxLen [%d] slice", prefixMask);
      }

      while(((pArgs->numroutes > 0) && (routes < pArgs->numroutes))
             && (!pArgs->noadd)) {

        if(pArgs->load) {
          if((status = bcm_test_v4_load_pfx(fp, &vrf_id, &prefix, &prefixMask)) != BCM_E_NONE) {
            if(status == BCM_E_PARAM){
              cli_out("\nAdd Error in processing File !!!");
              break;
            } else{
              cli_out("\n[EOF] End of File reached ");
              status = BCM_E_NONE;
            }
          }
        } else {
            if(pArgs->random) {
              prefix = sal_rand();
#if 0
              /*if(retry == 0) {*/
                bcm_test_v4_lpm_adjust_weight(&maskIndex, &maskWeight, &prefixMask);
              /*}*/
#else
              if (pArgs->ipv6) {
                  for(i=0;i<(v6masklen>>3);i++) 
                     v6prefix[i]=sal_rand() & 0xFF;
              }
                     
              if(pfxLenRetry) {
                if (pArgs->ipv6) {
                    bcm_v6_incr_pfx_mask(&v6masklen);
                    if (pArgs->debug)
                      cli_out("\nRt Add PfxLen Retry Moved to [%d] slice", v6masklen);
                } else {
                    bcm_v4_incr_pfx_mask(&prefixMask);
                    if (pArgs->debug)
                      cli_out("\nRt Add PfxLen Retry Moved to [%d] slice", prefixMask);
                }
                if (pArgs->debug)
                    cli_out("\nRoutes Added [%d]", routes);
              }
#endif
            } else {
              /* Default */
              SB_ASSERT(pArgs->seq);
              if(!first) {
                if (pArgs->ipv6) {
                   bcm_test_v6_lpm_inc_seq_pfx(v6prefix, v6masklen);
                } else {
                   bcm_test_v4_lpm_adjust_seq_pfx(&prefix, &prefixMask);
                }
              } else {
                  first = 0;
              }
            }
        }

        /* If file end is reached, dont add any more routes but commit */
        if(!pArgs->load ||
           (pArgs->load && status == BCM_E_NONE)) {

            start = sal_time_usecs();
            bcm_l3_route_t_init(&route_info);

            if(pArgs->rpf) {
                route_info.l3a_flags |= BCM_L3_RPF;
            }
            if (pArgs->ipv6) {
               sal_memcpy(route_info.l3a_ip6_net, v6prefix, sizeof(bcm_ip6_t)); 
               bcm_ip6_mask_create(route_info.l3a_ip6_mask, v6masklen);
               route_info.l3a_flags |= BCM_L3_IP6;
            } else {
               route_info.l3a_subnet   = prefix;
               route_info.l3a_ip_mask  = bcm_ip_mask_create(prefixMask);
            }

            /* Advance to next interface is pfx count of one next hop is reached */
            if(routes && (routes % pfxPerNhop == 0)){
              pivot_egr_id++;
            }
            if ((pArgs->numvrf > 1) && routes && (routes % pfxPerVrf == 0)){
              vrf_id++;   
              vrf_id = (vrf_id > pArgs->numvrf) ? 1 : vrf_id;
            }
            route_info.l3a_intf     = *pivot_egr_id;
            route_info.l3a_vrf     = vrf_id;

            status = bcm_l3_route_add(unit, &route_info);
        }

        if (pArgs->random && (status == BCM_E_EXISTS)){
            /* Retry if duplicate random prefix were generated */
            if(retry > MAX_RAND_RETRY) {
                /* ipv6em case */
                if (pArgs->ipv6em)
                  break;
                if(pfxLenRetry > MAX_RAND_RETRY) {
                  cli_out("\n ERROR IN RANDOMIZATION ...");
                  break;
                } else {
                    /* give a shot in next pfx length */
                    pfxLenRetry++;
                }
            } else {
              retry++;
            }
            if(pArgs->debug){
                if (pArgs->ipv6) {
                    if (pArgs->ipv6em) {
                      print_ip6_host(" Randomization Retrying Prefix:", v6prefix); 
                    } else {
                      print_ip6_pfx(" Randomization Retrying Prefix:", v6prefix, v6masklen);
                    }
                } else {
                    cli_out(\
                            "\n Randomization Retrying Prefix[0x%x] Mask[0x%x]",\
                            prefix, bcm_ip_mask_create(prefixMask));
                }
            }
        } else if (status == BCM_E_NONE) {
            end = sal_time_usecs();
            accum += SAL_USECS_SUB(end,start);

            if(pArgs->debug){
                if (pArgs->ipv6) {
                    if (pArgs->ipv6em) {
                      print_ip6_host(" Added Prefix:", v6prefix); 
                    } else {
                      print_ip6_pfx(" Addded Prefix:", v6prefix, v6masklen);
                    }
                    cli_out(" over EgrIf[%x]", route_info.l3a_intf);
                } else {
                    cli_out(\
                            "\n Addded Prefix[0x%x] Mask[0x%x] over EgrIf[%x]",\
                            prefix, bcm_ip_mask_create(prefixMask),route_info.l3a_intf);
                }
            }

            routes++;
            retry = 0;
            pfxLenRetry = 0;

            if(pArgs->batch) {
               if((routes % routesPerBatch == 0) || (routes == pArgs->numroutes)) {
                 bstart = sal_time_usecs();
                 flag = (pArgs->rpf) ? 3 : 1;
                 status = bcm_switch_control_set(unit, bcmSwitchL3RouteCommit, flag);
                 if (BCM_FAILURE(status)) {
                     cli_out(\
                             "bcm_switch_control_set l3routecommit failed\
                             err=%d %s\n", status,
                             bcm_errmsg(status));
                     routes -= routesPerBatch;
                     /* go to delete and clean up added routes */
                     break;
                 } else {
                     bend = sal_time_usecs();
                     cli_out("\n Add Batch --[%d]-- TimeStamp:", ++batchIndex);
                     cli_out(\
                             "\n Routes Per Batch [%d] Number of Batches [%d]",\
                             routesPerBatch, pArgs->numbatch);
                     cli_out(\
                             "\n Time for Route Adds %u", accum);
                     cli_out(\
                             "\n Time for Commit %u", SAL_USECS_SUB(bend,bstart));
                     baccum += (accum + SAL_USECS_SUB(bend,bstart));
                     accum = 0;
                 }
              }
            }
        } else {
            /* break here so we could remove all added routes */
            if (pArgs->ipv6) {
                if (pArgs->ipv6em) {
                  print_ip6_host(" ERROR ADDING ROUTES", v6prefix);
                } else {
                  print_ip6_pfx(" ERROR ADDING ROUTES", v6prefix, v6masklen);
                }
                cli_out(" status[%d:%s]...", status, bcm_errmsg(status));
            } else {
                cli_out(\
                        "\n ERROR ADDING ROUTES prefix[0x%x] status[%d:%s]...",\
                        prefix, status, bcm_errmsg(status));
            }

            /* If batching enabled commit all routes added up to date */
            /* this matters for capacity measurement                  */
            if(pArgs->batch) {
                 bstart = sal_time_usecs();
                 flag = (pArgs->rpf) ? 3 : 1;
                 status = bcm_switch_control_set(unit, bcmSwitchL3RouteCommit,flag );
                 if (BCM_FAILURE(status)) {
                     cli_out(\
                             "\n bcm_switch_control_set l3routecommit failed\
                             err=%d %s", status,
                             bcm_errmsg(status));
                 } else {
                     bend = sal_time_usecs();
                     cli_out("\n Add Batch --[%d]-- TimeStamp:", ++batchIndex);
                     cli_out(\
                             "\n Routes Per Batch [%d] Number of Batches [%d]",\
                             routesPerBatch, pArgs->numbatch);
                     cli_out(\
                             "\n Time for Route Adds %u", accum);
                     cli_out(\
                             "\n Time for Commit %u", SAL_USECS_SUB(bend,bstart));
                     baccum += (accum + SAL_USECS_SUB(bend,bstart));
                     accum = 0;
                 }
            }
            break;
        }
      }

      if(!pArgs->noadd){
          cli_out("\n Time Stamp After Route Add [%u]", sal_time_usecs());
          cli_out("\n Number of Routes Requested[%d] Added[%d]", pArgs->numroutes, routes);

          cli_out(\
                  "\n Adding %d IP Addresses in %u microseconds\n", \
                  routes, pArgs->batch?baccum:accum);
      }


      cli_out("\n --------------------");
      cli_out("\n Removing IP entries");
      cli_out("\n --------------------");

      /* Set the Seed for predictable sequence */
      if(pArgs->random) {
        sal_srand(prefixSeed);
      }

      retry      = 0;
      pfxLenRetry = 0;
      accum      = 0;
      baccum     = 0;
#if 0
      maskIndex  = 0;
      maskWeight = sliceWeight[0];
#endif
      prefix     = pArgs->seqpfx;
      prefixMask = pArgs->seqmask;
      sal_memcpy(v6prefix, pArgs->v6seqpfx, sizeof(bcm_ip6_t));
      v6masklen = pArgs->v6masklen;
      batchIndex = 0;
      vrf_id = 0;
      first = 1;
      pivot_egr_id = l3_egr_id;
      cli_out("\n Time Stamp Before Route Delete [%u]", sal_time_usecs());
      if (pArgs->debug) {
        if (pArgs->ipv6) 
          cli_out("\nRt Del Start PfxLen [%d] slice", prefixMask);
        else
          cli_out("\nRt Del Start PfxLen [%d] slice", v6masklen);
      }

      if(pArgs->noadd){
          routes = pArgs->numroutes;
      }

      if(pArgs->load) {
#ifndef __KERNEL__
        rewind(fp);
#endif
      }

      if(!pArgs->nodel) {

          for(index=0; index < routes || pArgs->load;) {

            bcm_l3_route_t_init(&route_info);

            if(pArgs->load) {
              if((status = bcm_test_v4_load_pfx(fp, &vrf_id, &prefix, &prefixMask))
                 != BCM_E_NONE) {
                if(status == BCM_E_INTERNAL){
                  cli_out("\nAdd Error in processing File !!!");
                  break;
                } else{
                  cli_out("\n[EOF] End of File reached ");
                }
              }
            } else {
                if(pArgs->random) {
                  prefix = sal_rand();
                  /*if(retry == 0) {*/
#if 0
                    bcm_test_v4_lpm_adjust_weight(&maskIndex, &maskWeight, &prefixMask);
#else
                  if (pArgs->ipv6) {
                      for(i=0;i<(v6masklen>>3);i++) 
                           v6prefix[i]=sal_rand() & 0xFF;
                  }
                  if(pfxLenRetry) {
                    if (pArgs->ipv6) {
                      bcm_v6_incr_pfx_mask(&v6masklen);
                      if (pArgs->debug)
                        cli_out("\nRt Del PfxLen Retry Moved to [%d] slice", v6masklen);
                    } else {
                      bcm_v4_incr_pfx_mask(&prefixMask);
                      if (pArgs->debug)
                        cli_out("\nRt Del PfxLen Retry Moved to [%d] slice", prefixMask);
                    }
                    if (pArgs->debug)
                       cli_out("\nRoutes deleted [%d]", routes);
                  }
#endif
                  /*}*/
                } else {
                  /* Default */
                  SB_ASSERT(pArgs->seq);
                  if(!first) {
                    if (pArgs->ipv6) {
                       bcm_test_v6_lpm_inc_seq_pfx(v6prefix, v6masklen);
                    } else {
                       bcm_test_v4_lpm_adjust_seq_pfx(&prefix, &prefixMask);
                    }
                  } else {
                      first = 0;
                  }
                }
            }

            if(pArgs->debug){
                if (pArgs->ipv6) {
                    if (pArgs->ipv6em) {
                      print_ip6_host(" Deleting Prefix:", v6prefix); 
                    } else {
                      print_ip6_pfx(" Deleting Prefix:", v6prefix, v6masklen);
                    }
                } else {
                    cli_out(\
                            "\n Deleting Prefix[0x%x] Mask[0x%x]",\
                            prefix, bcm_ip_mask_create(prefixMask));
                }
            }

            /* If file end is reached, dont delete any more routes but commit */
            if(!pArgs->load ||
               (pArgs->load && status == BCM_E_NONE)) {

                start = sal_time_usecs();
                if(pArgs->rpf) {
                    route_info.l3a_flags |= BCM_L3_RPF;
                }
      
                if (pArgs->ipv6) {
                   sal_memcpy(route_info.l3a_ip6_net, v6prefix, sizeof(bcm_ip6_t)); 
                   bcm_ip6_mask_create(route_info.l3a_ip6_mask, v6masklen);
                   route_info.l3a_flags |= BCM_L3_IP6;
                } else {
                   route_info.l3a_subnet   = prefix;
                   route_info.l3a_ip_mask  = bcm_ip_mask_create(prefixMask);
                }

                /* Advance to next interface is pfx count of one next hop is reached */
                if(index && (index % pfxPerNhop == 0)){
                  pivot_egr_id++;
                }
                if ((pArgs->numvrf > 1) && index && (index % pfxPerVrf == 0)){
                  vrf_id++; 
                  vrf_id = (vrf_id > pArgs->numvrf) ? 1 : vrf_id;
                }
                route_info.l3a_intf     = *pivot_egr_id;
                route_info.l3a_vrf     = vrf_id;

                status = bcm_l3_route_delete(unit, &route_info);
            }

            if (pArgs->random && (status == BCM_E_NOT_FOUND)){

                /* Retry if duplicate random prefix were generated */
                if(retry > MAX_RAND_RETRY) {
                    /* ipv6 em case 8*/
                    if (pArgs->ipv6em) {
                      break;
                    }
                    if(pfxLenRetry > MAX_RAND_RETRY) {
                      cli_out("\n ERROR IN RANDOMIZATION ...");
                      break;
                    } else {
                        /* give a shot in next pfx length */
                        pfxLenRetry++;
                    }
                } else {
                  retry++;
                }
                if(pArgs->debug){
                    if (pArgs->ipv6) {
                      if (pArgs->ipv6em) {
                        print_ip6_host(" Randomization Retrying Prefix:", v6prefix);
                      } else {
                        print_ip6_pfx(" Randomization Retrying Prefix:", v6prefix, v6masklen);
                      }
                    } else {
                        cli_out(\
                                "\nDelete Randomization Retrying Prefix[0x%x] Mask[0x%x]",\
                                prefix, bcm_ip_mask_create(prefixMask));
                   }
                }
            } else if (status == BCM_E_NONE) {
                retry = 0;
                pfxLenRetry = 0;
                end = sal_time_usecs();
                accum += SAL_USECS_SUB(end,start);
                index++;

                if(pArgs->batch) {
                   if((index % routesPerBatch == 0) || (index == routes)) {
                     bstart = sal_time_usecs();
                     flag = (pArgs->rpf) ? 3 : 1;
                     status = bcm_switch_control_set(unit, bcmSwitchL3RouteCommit, flag);
                     if (BCM_FAILURE(status)) {
                         cli_out(\
                                 "bcm_switch_control_set l3routecommit failed\
                                 err=%d %s\n", status,
                                 bcm_errmsg(status));
                         /* go to delete and clean up added routes */
                         index -= routesPerBatch;
                         break;
                     } else {
                         bend = sal_time_usecs();
                         cli_out("\n Delete Batch --[%d]-- TimeStamp:", ++batchIndex);
                         cli_out(\
                                 "\n Routes Per Batch [%d] Number of Batches [%d]",\
                                 routesPerBatch, pArgs->numbatch);
                         cli_out(\
                                 "\n Time for Route Deletes %u", accum);
                         cli_out(\
                                 "\n Time for Commit %u", SAL_USECS_SUB(bend,bstart));
                         baccum += (accum + SAL_USECS_SUB(bend,bstart));
                         accum = 0;
                     }
                  }
                }

            } else {
                /* break here so we could remove all added routes */
                if (pArgs->ipv6) {
                    if (pArgs->ipv6em) {
                        print_ip6_host(" ERROR DELETING ROUTES ", v6prefix);
                    } else {
                        print_ip6_pfx(" ERROR DELETING ROUTES ", 
                                      v6prefix, v6masklen);
                    }
                   cli_out(" status[%d:%s]...", status, bcm_errmsg(status));
                } else {
                   cli_out(\
                           "\n ERROR DELETING ROUTES prefix[0x%x] status[%d:%s]...",\
                           prefix, status, bcm_errmsg(status));
                }

                /* If batching enabled commit all routes added up to date */
                /* this matters for capacity measurement                  */
                if(pArgs->batch) {
                     bstart = sal_time_usecs();
                     flag = (pArgs->rpf) ? 3 : 1;
                     status = bcm_switch_control_set(unit, bcmSwitchL3RouteCommit, flag);
                     if (BCM_FAILURE(status)) {
                         cli_out(\
                                 "bcm_switch_control_set l3routecommit failed\
                                 err=%d %s\n", status,
                                 bcm_errmsg(status));
                     } else {
                         bend = sal_time_usecs();
                         cli_out("\n Add Batch --[%d]-- TimeStamp:", ++batchIndex);
                         cli_out(\
                                 "\n Routes Per Batch [%d] Number of Batches [%d]",\
                                 routesPerBatch, pArgs->numbatch);
                         cli_out(\
                                 "\n Time for Route Adds %u", accum);
                         cli_out(\
                                 "\n Time for Commit %u", SAL_USECS_SUB(bend,bstart));
                         baccum += (accum + SAL_USECS_SUB(bend,bstart));
                         accum = 0;
                     }
                }
                break;
            }
          }

          cli_out("\n Time Stamp After Route Delete [%u]", sal_time_usecs());
          cli_out("\n Number of Routes Delete Requested[%d] Deleted[%d]", routes, index);
          cli_out(\
                  "\n Deleting %d IP Addresses in %u microseconds \n", \
                  routes, pArgs->batch?baccum:accum);
      }

      bcm_l3_intf_delete(unit, &l3_intf);

      for(index=0; index < pArgs->numnhop; index++) {
          bcm_l3_egress_destroy(unit, *(l3_egr_id+index));
      }
      sal_free(l3_egr_id);
  }

  status = bcm_switch_control_set(unit, bcmSwitchL3RouteCache, cache);
  if (BCM_FAILURE(status)) {
      cli_out("bcm_switch_control_set l3cache failed err=%d %s\n", status,
              bcm_errmsg(status));
      if(pArgs->load) {
        sal_fclose(fp);
      }
      return CMD_FAIL;
  }

  if(pArgs->load) {
    sal_fclose(fp);
  }

  return CMD_OK;
}

char cmd_sbx_lpm_usage[] =
"Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
"lpm [options]\n"
"  Instruments IPV4 LPM Performance\n"
"Options: \n"
"------------                                                       \n"
" * numroutes - number of routes                                    \n"
"                                                                   \n"
" * random - generate random[1] (or) sequential routes[0]           \n"
"      Prefix Mask is generated based on default weights            \n"
"      rndlen option will help to randomize weights. By default     \n"
"      By default /24 has most weight, /16 next and all others      \n"
"      have equivalent weights                                      \n"
"      For IPv6, all slices are evenly loaded                       \n"
"                                                                   \n"
" * batch  - Enable route caching and commit in batches             \n"
"     **{numbatch}  - Number of routes to commit per batch          \n"
"                                                                   \n"
" * seq- generates prefix & mask sequentially[ON by default]        \n"
"     ** {seqmask}- Length of Mask from [8-32] to start masking     \n"
"                   sequentially generated prefix                   \n"
"     ** {seqpfx} - Prefix to Start Address generation              \n"
"                   eg.,0x01000000                                  \n"
"     ** {v6seqpfx} - prefix to start IPv6 Address generation       \n"
"     ** {v6masklen} - Len of mask [8-64] for IPv6                  \n"
"     Set v6masklen=128 for IPv6 Exact match address generation     \n"
"   NOTE:                                                                \n"
"     these v6 parameters are used only if ipv6 is requested        \n"
"                                                                   \n"
" * ipv6 - enables the ipv6 mode of operation                       \n"
"                                                                   \n"
" * stats - displays lpm statistics                                 \n"
"    ** {clear}   - clears statistics                               \n"
"    ** {verbose} - verbose statistics                              \n"
"                                                                   \n"
" * rpf - enables RPF on routing interface                          \n"
"                                                                   \n"
" * numnhop - Number of Next hop to use                             \n"
" * numvrf  - Number of vrf to use                                  \n"
" * noadd  - dont add routes                                        \n"
" * nodel  - dont delete routes                                     \n"
" * debug  - debug traces                                           \n"
"                                                                   \n"
" * load [0 or 1] - loads route from text file                      \n"
"          - Nut supported for IPv6                                 \n"
"   *fname - file name to load (default - route.txt) [not supported]\n"
"          - for now file name must always be route.txt             \n"
"                                                                   \n"
" ~~~ not supported options ~~~~                                    \n"
" * ilib   - Use ilib Api for SDK Api                               \n"
" * rndlen - generates random[1] (or) fixed prefix length           \n"
#endif
;

cmd_result_t
cmd_sbx_lpm(int unit, args_t *a)
{
    int rv=CMD_FAIL;
    bcm_test_lpm_arg_t lpmArgs;
    bcm_test_lpm_arg_init(&lpmArgs);

    if (ARG_CNT(a)) {
        parse_table_t pt;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "ipv6", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &lpmArgs.ipv6, NULL);
        parse_table_add(&pt, "numroutes", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &lpmArgs.numroutes, NULL);
        parse_table_add(&pt, "random", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.random, NULL);
        parse_table_add(&pt, "rndlen", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.rndlen, NULL);
        parse_table_add(&pt, "debug", PQ_DFL | PQ_INT, 0, &lpmArgs.debug, NULL);
        parse_table_add(&pt, "batch", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.batch, NULL);
        parse_table_add(&pt, "numbatch", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &lpmArgs.numbatch, NULL);
        parse_table_add(&pt, "numnhop", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &lpmArgs.numnhop, NULL);
        parse_table_add(&pt, "numvrf", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &lpmArgs.numvrf, NULL);
        parse_table_add(&pt, "ilib", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.ilib, NULL);


        parse_table_add(&pt, "seq", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.seq, NULL);
        parse_table_add(&pt, "v6masklen", PQ_DFL | PQ_INT, 0, &lpmArgs.v6masklen, NULL);
        parse_table_add(&pt, "v6seqpfx", PQ_DFL | PQ_IP6, 0, &lpmArgs.v6seqpfx, NULL);
        parse_table_add(&pt, "seqmask", PQ_DFL | PQ_INT, 0, &lpmArgs.seqmask, NULL);
        parse_table_add(&pt, "seqpfx", PQ_DFL | PQ_INT, 0, &lpmArgs.seqpfx, NULL);
        parse_table_add(&pt, "noadd", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.noadd, NULL);
        parse_table_add(&pt, "nodel", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.nodel, NULL);
        parse_table_add(&pt, "load", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.load, NULL);
        parse_table_add(&pt, "stats", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.stats, NULL);
        parse_table_add(&pt, "verbose", PQ_DFL | PQ_INT ,
                        0, &lpmArgs.verbose, NULL);
        parse_table_add(&pt, "clear", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.clear, NULL);
        parse_table_add(&pt, "rpf", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &lpmArgs.rpf, NULL);
        
        /*parse_table_add(&pt, "fname", PQ_DFL|PQ_STRING,"route.txt",&lpmArgs.fname,0);*/
        if (!parseEndOk(a, &pt, &rv)) {
            return rv;
        }
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
    case SOC_SBX_UCODE_TYPE_G2P3:
    case SOC_SBX_UCODE_TYPE_G3P1:
    case SOC_SBX_UCODE_TYPE_T3P1:
        rv = bcm_test_lpm_validate_args(&lpmArgs);
        if(CMD_OK == rv){
          rv = bcm_test_lpm_capacity(unit, &lpmArgs);
        }
        break;
    default:
        cli_out("ERROR: unsupported microcode type: %d\n",
                SOC_SBX_CONTROL(unit)->ucodetype);
        rv = CMD_FAIL;
    }
    return rv;
}


#define DEFAULT_NUM_MACS (1000)
#define DEFAULT_MAC_RETRY (5)

typedef struct bcm_test_mac_arg_s{
  int nummacs;
  int random;
  int batch;
  int numbatch;
  int debug;
  int retry;
  int seed;
  /* if set issues soc level API */    
  int soc;
  bcm_mac_t seqmac;
  int noadd;
  int nodel;
  int age;
} bcm_test_mac_arg_t;

static
void bcm_test_mac_arg_init (bcm_test_mac_arg_t *pArgs)
{
  SB_ASSERT(pArgs);
  pArgs->nummacs   = DEFAULT_NUM_PREFIX;
  pArgs->random    = 0;
  pArgs->batch     = 0;
  pArgs->numbatch  = DEFAULT_NUM_BATCH;
  pArgs->debug     = 0;
  pArgs->noadd     = 0;
  pArgs->nodel     = 0;
  pArgs->retry     = DEFAULT_MAC_RETRY;
  sal_memset(&pArgs->seqmac, 0, sizeof(bcm_mac_t));
  pArgs->seqmac[5] = 1;
  pArgs->seed      = 0;
  pArgs->age      = 0;
  pArgs->soc      = 0;
}

static int bcm_test_mac_validate_args(int unit, bcm_test_mac_arg_t *pArgs)
{
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
    case SOC_SBX_UCODE_TYPE_G3P1:
    case SOC_SBX_UCODE_TYPE_T3P1:
        /* no batching supported yet */
        if (pArgs->batch) return CMD_USAGE;
        /* Cannot use aging and bypass the BCM layer */
        if (pArgs->age && pArgs->soc) return CMD_USAGE;
        break;
    default: 
        break;
    }

    return CMD_OK;
}


static int bcm_sbx_test_gen_mac(bcm_test_mac_arg_t *macargs, uint8 *mac)
{
  return   bcm_test_gen_mac(mac, macargs->random);
}


#ifdef BCM_CALADAN3_G3P1_SUPPORT
static int bcm_test_g3p1_soc_mac_capacity(int unit, bcm_test_mac_arg_t *macargs)
{
    sal_usecs_t start=0, end=0, accum=0;
    int i=0, retry=0, rv=SOC_E_NONE;
    uint32 vsi=0;
    soc_sbx_g3p1_6_byte_t mymac;
    soc_sbx_g3p1_mac_t entry;
    double ul_i, ul_accum;

    if (!macargs) return CMD_FAIL;

    soc_sbx_g3p1_mac_t_init(&entry);
    entry.smac_hit = 1;
    entry.dmac_hit = 1;
    entry.dontage = 1;
    entry.ftidx   = 0x200;
    
    sal_memcpy(&mymac, &macargs->seqmac, sizeof(bcm_mac_t));

    if(macargs->age) {
        entry.dontage = 0;
    }


    if(!macargs->noadd) {    
        if(macargs->random) {
            sal_srand(macargs->seed);
        }

        for(i=0, accum=0, vsi=0; i < macargs->nummacs;) {
            if (i > 0) {
                if (bcm_test_gen_mac_vsi(mymac, &vsi, macargs->random) < 0) {
                    cli_out("!!Mac Add Error Generating Mac Address \n");
                    return CMD_FAIL;
                }
            }

            if(macargs->debug) {
                cli_out("\n Adding VSI=%d MAC 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                        vsi, mymac[0], mymac[1], mymac[2],
                        mymac[3], mymac[4], mymac[5]);
            }

            start = sal_time_usecs();

            rv = soc_sbx_g3p1_mac_add(unit, mymac, vsi, &entry);
            if ((SOC_E_EXISTS == rv) && (++retry >= macargs->retry)) {
                cli_out("!! MAC Address Generation Retry[%d] Failed: Mac Added[%d]\n",
                        retry-1, i);    
                break;
            } else if (rv != SOC_E_NONE) {
                cli_out("!! MAC Address Add: MAC 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x" 
                        " Failed: Mac Added[%d]\n",
                        mymac[0], mymac[1], mymac[2],
                        mymac[3], mymac[4], mymac[5], i);
                break;
            } else {
                end = sal_time_usecs();
                accum += SAL_USECS_SUB(end,start);
                i++;        
                retry = 0;
            }
        }

        ul_i = i;
        ul_i = ul_i * 1000000;
        ul_accum = accum;

        cli_out("\n### Added %d MACs in %u microseconds (%u MACs/sec)\n",
                i, accum, (unsigned int)(ul_i/ul_accum));
    }
    
    sal_memcpy(&mymac, &macargs->seqmac, sizeof(bcm_mac_t));

    if(!macargs->nodel) { 
        cli_out("Removing MAC entries\n");
        
        /* Set the Seed for predictable sequence */
        if(macargs->random) {
            sal_srand(macargs->seed);
        }

        for(i=0, vsi=0, accum=0; i < macargs->nummacs;) {
            if (i > 0) {
                if (bcm_test_gen_mac_vsi(mymac, &vsi, macargs->random) < 0) {
                    cli_out("!!Mac Delete Error Generating Mac Address \n");
                    return CMD_FAIL;
                }
            }

            if(macargs->debug) {
                cli_out("\n Deleting MAC 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                        mymac[0], mymac[1], mymac[2],
                        mymac[3], mymac[4], mymac[5]);
            }

            start = sal_time_usecs();

            rv = soc_sbx_g3p1_mac_delete(unit, mymac, vsi);
            if ((SOC_E_NOT_FOUND == rv) && (++retry >= macargs->retry)) {
                cli_out("!! MAC Address Generation Retry[%d] Failed: Mac Added[%d]\n",
                        retry-1, i);    
                break;
            } else if (rv != SOC_E_NONE) {
                cli_out("!! MAC Address Delete: MAC %02x:%02x:%02x:%02x:%02x:%02x" 
                        " Failed: Mac Added[%d]\n",
                        mymac[0], mymac[1], mymac[2],
                        mymac[3], mymac[4], mymac[5], i);
                /*break;*/
            } else {
                end = sal_time_usecs();
                accum += SAL_USECS_SUB(end,start);
                i++;        
                retry = 0;
            }
        }

      ul_i = i;
      ul_i = ul_i * 1000000;
      ul_accum = accum;
      cli_out("Deleted %d MACs in %u microseconds (%u MACs/sec)\n",
              i, accum, (unsigned int)(ul_i/ul_accum));
    }

    return CMD_OK;
}
#endif

static
int bcm_test_sbx_mac_capacity (int unit, bcm_test_mac_arg_t *macargs)
{
  int i, macsPerBatch, numadd, retry=0;
  double ul_i, ul_accum;
  bcm_l2_addr_t l2addr;
  bcm_mac_t mymac;
  int res;
  sal_usecs_t start, end, accum;
  bcm_vlan_t vlan = 0xabc;
  int batch;

  if( !SOC_SBX_INIT(unit) ){
    cli_out("Unit %d, not initialized - call 'init soc' first!\n", unit);
    return CMD_FAIL;
  }
  if (!SOC_IS_SBX_CALADAN3(unit)) {
    cli_out("only supported on Caladan3\n");
    return CMD_FAIL;
  }

  if(!macargs) {
    cli_out("\n Null Argument Pointer ");
    return CMD_FAIL;
  }

  /* Set the Seed for predictable sequence */
  if(macargs->random) {
      sal_srand(macargs->seed);
  }

  if (macargs->soc) {
      if (!SOC_IS_SBX_CALADAN3(unit)) {
          cli_out("only supported on Caladan3\n");
          return CMD_FAIL;
      }

      cli_out("Adding new MACs using SOC API\n");    
#ifdef BCM_CALADAN3_G3P1_SUPPORT
      return bcm_test_g3p1_soc_mac_capacity (unit, macargs);
#endif  
      
  } else {
      cli_out("Adding new MACs using BCM API\n");
      i = 0;
      start = sal_time_usecs();

      res = bcm_switch_control_get(unit, bcmSwitchL2Cache, &batch);
      if (BCM_FAILURE(res)) {
          cli_out("bcm_switch_control_get l2cache failed err=%d %s\n", res,
                  bcm_errmsg(res));
          return CMD_FAIL;

      }

      res = bcm_switch_control_set(unit, bcmSwitchL2Cache, macargs->batch);
      if (BCM_FAILURE(res)) {
          cli_out("bcm_switch_control_set l2cache failed err=%d %s\n", res,
                  bcm_errmsg(res));
          return CMD_FAIL;

      }
  }

  sal_memcpy(&mymac, &macargs->seqmac, sizeof(bcm_mac_t));

  if(!macargs->batch) {
     macargs->numbatch = 1;
  }

  macsPerBatch = macargs->nummacs / macargs->numbatch;

  retry = 0;
  accum = 0;

  if(!macargs->noadd) {

      for(i=0; i < macargs->nummacs;) {

        if(BCM_E_NONE != bcm_sbx_test_gen_mac(macargs, &mymac[0])){
            cli_out("!!Mac Add Error Generating Mac Address \n");
            return CMD_FAIL;
        }

        if(macargs->debug) {
            cli_out("\n Adding MAC 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                    mymac[0], mymac[1], mymac[2],
                    mymac[3], mymac[4], mymac[5]);
        }

        /* hold on time stamp for retry */
        if(!retry) {
          start = sal_time_usecs();
        }

        bcm_l2_addr_t_init (&l2addr, mymac, vlan);

        l2addr.flags |=  macargs->age ? 0:BCM_L2_STATIC;

        l2addr.port = 1;

        res = bcm_l2_addr_add(unit, &l2addr);
        if (res != BCM_E_NONE) {
            if(++retry >= macargs->retry) {
                /* commit and break for batching */
                if(!macargs->batch) {
                    cli_out("!! MAC ADD Retry[%d] Failed: Mac Added[%d]\n",
                            retry-1, i);
                    break;
                }
            } else {
              continue;
            }
        }

        /* If batching is on, commit if batch boundary is reached */
        if((i == macargs->nummacs-1) ||
           (i % macsPerBatch == 0) ||
           (retry >= macargs->retry)) {

          if(macargs->batch) {
              res = bcm_switch_control_set(unit, bcmSwitchL2Commit, 0);
              if (BCM_FAILURE(res)) {
                  cli_out("bcm_switch_control_set l2commit failed err=%d %s\n", res,
                          bcm_errmsg(res));
                  break;
              } else {
                  if(macargs->debug) {
                      cli_out("Batch[%d] Commited \n",
                              i/macargs->numbatch);
                  }
              }
              if(retry >= macargs->retry) {
                  cli_out("!! Batch MAC ADD Retry[%d] Failed: Mac Added[%d]\n",
                          retry-1, i);
                  break;
              }
          }
        }

        /* Accumulate for retry failure */
        end = sal_time_usecs();
        accum += SAL_USECS_SUB(end,start);
        i++;
      }

      ul_i = i;
      ul_i = ul_i * 1000000;
      ul_accum = accum;
      cli_out("Added %d MACs in %u microseconds (%u MACs/sec)\n",
              i, accum, (unsigned int)(ul_i/ul_accum));
  }

  if(!macargs->nodel) {

      cli_out("Removing MAC entries\n");

      /* Set the Seed for predictable sequence */
      if(macargs->random) {
          sal_srand(macargs->seed);
      }

      if(macargs->noadd){
        numadd = macargs->nummacs;
      } else {
        numadd = i;
      }

      sal_memcpy(&mymac, &macargs->seqmac, sizeof(bcm_mac_t));
      retry = 0;
      accum = 0;

      for(i=0; i < numadd;) {

        if(BCM_E_NONE != bcm_sbx_test_gen_mac(macargs, &mymac[0])){
            cli_out("!!Mac Del Error Generating Mac Address \n");
            return CMD_FAIL;
        }

        if(macargs->debug) {
            cli_out("\n Deleting MAC 0x%x:0x%x:0x%x:0x%x:0x%x:0x%x",
                    mymac[0], mymac[1], mymac[2],
                    mymac[3], mymac[4], mymac[5]);
        }

        /* hold on time stamp for retry */
        if(!retry) {
          start = sal_time_usecs();
        }

        bcm_l2_addr_t_init (&l2addr, mymac, vlan);
        l2addr.flags |= BCM_L2_STATIC;
        l2addr.port = 1;

        res = bcm_l2_addr_delete(unit, mymac, vlan);
        if (res != BCM_E_NONE) {
            /* Try to retry more and delete */
            if(++retry >= macargs->retry) {
                /* commit and break for batching */
                if(!macargs->batch) {
                  cli_out("!! MAC DEL Retry[%d] Failed: Mac Deleted[%d]\n",
                          retry-1, i);
                  break;
                }
            } else {
              continue;
            }
        }

        /* If batching is on, commit if batch boundary is reached */
        if((i == numadd-1) ||
           (i % macsPerBatch == 0) ||
           (retry >= macargs->retry)) {

          if(macargs->batch) {
              res = bcm_switch_control_set(unit, bcmSwitchL2Commit, 0);
              if (BCM_FAILURE(res)) {
                  cli_out("bcm_switch_control_set l2commit failed err=%d %s\n", res,
                          bcm_errmsg(res));
                  break;
              } else {
                  if(macargs->debug) {
                      cli_out("Batch[%d] Commited \n",i/macargs->numbatch);
                  }
              }
              if(retry >= macargs->retry) {
                  cli_out("!! Batch MAC DEL Retry[%d] Failed: Mac Deleted[%d]\n",
                          retry-1, i);
                  break;
              }
          }
        }

        /* Accumulate for retry failure */
        end = sal_time_usecs();
        accum += SAL_USECS_SUB(end,start);
        i++;
      }

      ul_i = i;
      ul_i = ul_i * 1000000;
      ul_accum = accum;
      cli_out("Deleted %d MACs in %u microseconds (%u MACs/sec)\n",
              i, accum, (unsigned int)(ul_i/ul_accum));
  }

  res = bcm_switch_control_set(unit, bcmSwitchL2Cache, batch);
  if (BCM_FAILURE(res)) {
      cli_out("bcm_switch_control_set l2cache failed err=%d %s\n", res,
              bcm_errmsg(res));
      return CMD_FAIL;

  }

  return CMD_OK;
}

char cmd_sbx_mac_usage[] =
"Usage:\n"
#ifndef COMPILER_STRING_CONST_LIMIT
"mac [options]\n"
"  Instruments MAC EM Performance\n"
"Options: \n"
"------------                                                       \n"
" * nummacs - number of mac address to add                          \n"
"                                                                   \n"
" * random - generate random[1] (or) sequential routes[0]           \n"
"    *random not yet supported                                      \n"
"    **retry - maximum retry                                        \n"
"    **seed - randomization seed                                    \n"
" * seqmac - Sequential Start MAC                                   \n"
"                                                                   \n"
" * batch  - Enable mac caching and commit in batches               \n"
"     **{numbatch}  - Number of macs to commit per batch            \n"
"                                                                   \n"
" * debug  - debug traces                                           \n"
"                                                                   \n"
" * noadd - if set only deletes happen                              \n"
" * age - if set allows aging. Cannot be used with soc=1            \n"
" * nodel - if set only adds happen                                 \n"
" * soc   - if set soc layer API are used for generation            \n"
#endif
;
cmd_result_t
cmd_sbx_mac(int unit, args_t *a)
{
    int rv=CMD_FAIL;
    bcm_test_mac_arg_t macArgs;
    bcm_test_mac_arg_init(&macArgs);

    if (ARG_CNT(a)) {
        parse_table_t pt;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "nummacs", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &macArgs.nummacs, NULL);
        parse_table_add(&pt, "random", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &macArgs.random, NULL);
        parse_table_add(&pt, "retry", PQ_DFL | PQ_INT, 0, &macArgs.retry, NULL);
        parse_table_add(&pt, "seed", PQ_DFL | PQ_INT, 0, &macArgs.seed, NULL);
        parse_table_add(&pt, "debug", PQ_DFL | PQ_INT, 0, &macArgs.debug, NULL);
        parse_table_add(&pt, "batch", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &macArgs.batch, NULL);
        parse_table_add(&pt, "numbatch", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &macArgs.numbatch, NULL);
        parse_table_add(&pt, "seqmac", PQ_DFL | PQ_MAC,
                        0, &macArgs.seqmac, NULL);
        parse_table_add(&pt, "noadd", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &macArgs.noadd, NULL);
        parse_table_add(&pt, "age", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                               0, &macArgs.age, NULL);
        parse_table_add(&pt, "nodel", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &macArgs.nodel, NULL);
        parse_table_add(&pt, "soc", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &macArgs.soc, NULL);
        if (!parseEndOk(a, &pt, &rv)) {
            return rv;
        }
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
    case SOC_SBX_UCODE_TYPE_G2P3:
    case SOC_SBX_UCODE_TYPE_G3P1:
    case SOC_SBX_UCODE_TYPE_T3P1:
        rv = bcm_test_mac_validate_args(unit, &macArgs);
        if(CMD_OK == rv){
          rv = bcm_test_sbx_mac_capacity(unit, &macArgs);
        }
        break;
    default:
        cli_out("ERROR: unsupported microcode type: %d\n",
                SOC_SBX_CONTROL(unit)->ucodetype);
        rv = CMD_FAIL;
    }
    return rv;
}


static int sbx_rx_cb_count;
static volatile int sbx_enqueue_pkts[BCM_MAX_NUM_UNITS];
static volatile int sbx_rx_pkt_count[BCM_MAX_NUM_UNITS];
static sal_mutex_t sbx_pkt_queue_lock[BCM_MAX_NUM_UNITS];
static sal_sem_t sbx_pkts_are_ready[BCM_MAX_NUM_UNITS];
static volatile uint32 *sbx_pkt_free_queue[BCM_MAX_NUM_UNITS];
static volatile uint32 *sbx_pkt_data[BCM_MAX_NUM_UNITS];

STATIC void
sbx_rx_free_pkts(void *cookie)
{
    uint32 *to_free, *pkt_data;
    uint32 *next;
    int unit = PTR_TO_INT(cookie), rv;

    while (TRUE) {
        sal_sem_take(sbx_pkts_are_ready[unit], sal_sem_FOREVER);
        sal_mutex_take(sbx_pkt_queue_lock[unit], sal_mutex_FOREVER);

        to_free = (uint32 *)sbx_pkt_free_queue[unit];
        pkt_data = (uint32 *)sbx_pkt_data[unit];
        sbx_pkt_free_queue[unit] = NULL;
        sbx_pkt_data[unit] = NULL;
        sbx_rx_pkt_count[unit] = 0;

        sal_mutex_give(sbx_pkt_queue_lock[unit]);

        while (to_free != NULL) {
            next = NULL;
            ((bcm_pkt_t *)to_free)->pkt_data->data = (uint8 *)pkt_data;
            LOG_INFO(BSL_LS_APPL_RX,
                     (BSL_META_U(unit,
                                 "Dequeued RX packet pointer 0x%x data 0x%x alloc 0x%x\n"),
                      (uint32)to_free,
                      (uint32)((bcm_pkt_t *)to_free)->pkt_data->data,
                      (uint32)((bcm_pkt_t *)to_free)->alloc_ptr));
            rv = _learn_rx_cb(unit, (bcm_pkt_t *)to_free, NULL);
            if (rv != BCM_RX_HANDLED) {
                /* bcm_rx_free(unit, ((bcm_pkt_t *)to_free)->pkt_data->data); */
                bcm_rx_free(unit, ((bcm_pkt_t *)to_free));
            }
            to_free = next;
            pkt_data = next;
        }
    }
}

STATIC bcm_rx_t
_sbx_rx_cb_handler(int unit, bcm_pkt_t *info, void *cookie)
{
    int         count;
    int rv;

    COMPILER_REFERENCE(cookie);

    count = ++sbx_rx_cb_count;

    LOG_INFO(BSL_LS_APPL_RX,
             (BSL_META_U(unit,
                         "RX packet %d: unit=%d len=%d rx_port=%d reason=%d cos=%d\n"),
              count, unit, info->tot_len, info->rx_port, info->rx_reason,
              info->cos));

    LOG_INFO(BSL_LS_APPL_RX,
             (BSL_META_U(unit,
                         "packet pointer=0x%x data=0x%x _data=0x%08x len=%d alloc=0x%x\n"),
              (uint32)info, (uint32)(info->pkt_data->data),
              (uint32)(info->_pkt_data.data),
              info->pkt_data->len, (uint32)info->alloc_ptr));
    
    rv = _learn_rx_cb(unit, info, NULL);
    if (BCM_FAILURE(rv)) {
         LOG_INFO(BSL_LS_APPL_RX,
                  (BSL_META_U(unit,
                              "learn cb failed: %d %s\n"),
                   rv, bcm_errmsg(rv)));
    }

    return BCM_RX_HANDLED;


#if 0
    /* If/when enabled, be sure to change _learn_rx_cb to free & null
     * the packet data
     */
    if (sbx_enqueue_pkts[unit] > 0) {

        LOG_INFO(BSL_LS_APPL_RX,
                 (BSL_META_U(unit,
                             "wating on queue lock\n")));
        sal_mutex_take(sbx_pkt_queue_lock[unit], sal_mutex_FOREVER);

        /* *(uint32 **)(info->alloc_ptr) = (uint32 *)sbx_pkt_free_queue[unit]; */
        LOG_INFO(BSL_LS_APPL_RX,
                 (BSL_META_U(unit,
                             "Enqueued RX packet pointer 0x%x data 0x%x len %d alloc 0x%x\n"),
                  (uint32)info, (uint32)(info->pkt_data->data),
                  info->pkt_data->len, (uint32)info->alloc_ptr));
        sbx_pkt_free_queue[unit] = (uint32 *)info;
        sbx_pkt_data[unit] = (uint32 *)info->pkt_data->data;
        sbx_rx_pkt_count[unit]++;

        if (sbx_rx_pkt_count[unit] >= sbx_enqueue_pkts[unit]) {
            LOG_INFO(BSL_LS_APPL_RX,
                     (BSL_META_U(unit,
                                 "Giving pkt_are_ready sem\n")));
            sal_sem_give(sbx_pkts_are_ready[unit]);
            sal_thread_yield();
        }

        LOG_INFO(BSL_LS_APPL_RX,
                 (BSL_META_U(unit,
                             "Giving queueLock\n")));
        sal_mutex_give(sbx_pkt_queue_lock[unit]);
#if defined(BCM_RXP_DEBUG)
        bcm_rx_pool_own(info->alloc_ptr, "rxmon");
#endif
        return BCM_RX_HANDLED_OWNED;
    }

    return BCM_RX_HANDLED;
#endif
}

#define BASIC_PRIO 100

STATIC int
_sbx_init_rx_api(int unit)
{
    int r;

    if (bcm_rx_active(unit)) {
        cli_out("RX is already running\n");
        return -1;
    }

    if ((r = bcm_rx_start(unit, 0)) < 0) {
        cli_out("rxmon: Error: Cannot start RX: %s.\n", bcm_errmsg(r));
        return -1;
    }

    return 0;
}

char cmd_sbx_rx_mon_usage[] =
    "Parameters [init|start|stop|show|[-]enqueue [n]]\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "With no parameters, show whether or not active.\n"
    "    init:           Initialize the RX API, but don't register handler\n"
    "    start:          Call RX start with local pkt dump routine\n"
    "                    Modify the configuration with the rxcfg command\n"
    "    stop:           Call RX stop\n"
    "    [-]enqueue [n]: Enqueue packets to be freed later in thread\n"
    "                    If n > 0, enqueue at least n pkts before freeing\n"
    "    stop:           Call RX stop\n"
    "    show:           Call RX show\n"
#endif
    ;

cmd_result_t
cmd_sbx_rx_mon(int unit, args_t *args)
/*
 * Function:    rx
 * Purpose:     Perform simple RX test
 * Parameters:  unit - unit number
 *              args - arguments
 * Returns:     CMD_XX
 */
{
    char                *c;
    uint32              active;
    int                 r;

    if (!sh_check_attached(ARG_CMD(args), unit)) {
        return(CMD_FAIL);
    }

    bcm_rx_channels_running(unit, &active);

    c = ARG_GET(args);
    if (c == NULL) {
        cli_out("Active bitmap for RX is %x.\n", active);
        return CMD_OK;
    }

    if (sal_strcasecmp(c, "init") == 0) {
        if (_sbx_init_rx_api(unit) < 0) {
            return CMD_FAIL;
        } else {
            return CMD_OK;
        }

    } else if (sal_strcasecmp(c, "enqueue") == 0) {
        if (sbx_pkt_queue_lock[unit] == NULL) { /* Init free pkt stuff */
            sbx_pkt_queue_lock[unit] = sal_mutex_create("sbx_rxmon");
            sbx_pkts_are_ready[unit] = sal_sem_create("sbx_rxmon", sal_sem_BINARY, 0);
            if (sal_thread_create("sbx_rxmon", SAL_THREAD_STKSZ, 80, sbx_rx_free_pkts,
                                  INT_TO_PTR(unit)) == SAL_THREAD_ERROR) {
                cli_out("FAILED to start rxmon packet free thread\n");
                sal_mutex_destroy(sbx_pkt_queue_lock[unit]);
                sbx_pkt_queue_lock[unit] = NULL;
                sal_sem_destroy(sbx_pkts_are_ready[unit]);
                sbx_pkts_are_ready[unit] = NULL;
                return CMD_FAIL;
            }
        }
        sbx_enqueue_pkts[unit] = 1;
        if ((c = ARG_GET(args)) != NULL) {
            sbx_enqueue_pkts[unit] = strtoul(c, NULL, 0);
        }
    } else if (sal_strcasecmp(c, "-enqueue") == 0) {
        sbx_enqueue_pkts[unit] = 0;
    } else if (sal_strcasecmp(c, "start") == 0) {
        sbx_rx_cb_count = 0;

        if (!bcm_rx_active(unit)) { /* Try to initialize */
            if (_sbx_init_rx_api(unit) < 0) {
                cli_out("Warning:  init failed.  Will attempt register\n");
            }
        }

        /* Register to accept all cos */
        if ((r = bcm_rx_register(unit, "Sbx RX CMD", _sbx_rx_cb_handler,
                                 0x40, NULL, 0)) < 0) {
            cli_out("%s: bcm_rx_register failed: %s\n",
                    ARG_CMD(args), bcm_errmsg(r));
            return CMD_FAIL;
        }

        cli_out("NOTE:  'debugmod diag rx' required for rxmon output\n");

    } else if (sal_strcasecmp(c, "stop") == 0) {
        if ((r = bcm_rx_stop(unit, 0)) < 0) {
            cli_out("%s: Error: Cannot stop RX: %s.  Is it running?\n",
                    ARG_CMD(args), bcm_errmsg(r));
            return CMD_FAIL;
        }
        /* Unregister handler */
        if ((r = bcm_rx_unregister(unit, _sbx_rx_cb_handler, BASIC_PRIO)) < 0) {
            cli_out("%s: bcm_rx_unregister failed: %s\n",
                    ARG_CMD(args), bcm_errmsg(r));
            return CMD_FAIL;
        }

    } else if (sal_strcasecmp(c, "show") == 0) {
#ifdef  BROADCOM_DEBUG
        bcm_rx_show(unit);
#else
        cli_out("%s: ERROR: cannot show in non-BROADCOM_DEBUG compilation\n",
                ARG_CMD(args));
        return CMD_FAIL;
#endif  /* BROADCOM_DEBUG */
    } else {
        return CMD_USAGE;
    }

    return CMD_OK;
}

#endif /* BCM_FE2000_SUPPORT */

char cmd_sbx_board_usage[] =
"Usage:\n"
"board [options]\n"
"     init type=<board type>   - Board level initialization before SOC init\n"
;

cmd_result_t
cmd_sbx_board(int unit, args_t *args)
{
    int rc;
    char *c;

    if (!(c = ARG_GET(args))) {     /* Nothing to do */
        return(CMD_USAGE);          /* Print usage line */
    } else if (!sal_strcasecmp(c, "init")) {
        if (ARG_CNT(args)) {
            int ret_code;
            int  brd_type;

            parse_table_t pt;
            parse_table_init(0, &pt);
            parse_table_add(&pt, "type", PQ_DFL | PQ_INT,
                            0, &brd_type, NULL);

            if (!parseEndOk(args, &pt, &ret_code)) {
                return ret_code;
            }

            board_type = brd_type;
            if ( sbx_diag_init(brd_type) != 0 )
              cli_out("sbx_diag_init failed\n");
            rc = board_preinit(brd_type);
            return(rc);
        }
    }

    return CMD_USAGE;
}

char cmd_sbx_xxsocreload_usage[] =
"Parameters - none\n"
"Checks the soc property diag_easy_reload and enables EASY_RELOAD if 1\n";

cmd_result_t
cmd_sbx_xxsocreload(int unit, args_t *args)
{
#ifdef BCM_EASY_RELOAD_SUPPORT
    int reload;
    SOC_RELOAD_MODE_SET(unit, soc_property_get(unit, spn_DIAG_EASY_RELOAD, 0));

    reload = SOC_IS_RELOADING(unit);
    cli_out("Unit %d is in %s mode.\n",
            unit,
            reload ? "reload" : "normal");
#else /* !defined BCM_EASY_RELOAD_SUPPORT */
#ifndef BCM_WARM_BOOT_SUPPORT
    cli_out("Easy reload not defined set BCM_EASY_RELOAD_SUPPORT in Make.local to enable if desired\n");
#endif
#endif /* BCM_EASY_RELOAD SUPPORT */
    return (CMD_OK);
}

#ifdef BCM_EASY_RELOAD_SUPPORT
#ifdef BCM_EASY_RELOAD_SUPPORT_SW_DUMP
char cmd_sbx_get_state_usage[] =
"Usage:\n"
" get internal state\n"
;


static uint32
sbx_archive_write(void *fd, void *data, uint32 size, uint32 nItems) {
    uint32 savedItems;
    if ((savedItems = fwrite(data, size, nItems, (FILE *)fd)) != nItems ) {
        cli_out("ERROR: user function: sbx_archive_write FAILURE!!!\n");
    }
    return savedItems;
}

static void*
sbx_archive_open(void *name, char *options) {
    FILE *tmpFd;
    if (!(tmpFd = sal_fopen((char *)name, options))) {
        cli_out("ERROR:user function: sbx_archive_open FAILURE!!!\n");
    }
    return (void *)tmpFd;
}

static int
sbx_archive_close(void *fd) {
    if(sal_fclose((FILE *)fd)){
        return TRUE;
    }else{
        return FALSE;
    }
}

#define STATE_BUF_SIZE 4000
char statebuf[STATE_BUF_SIZE];
char statebuf1[STATE_BUF_SIZE];
char statebuf2[STATE_BUF_SIZE];
cmd_result_t
cmd_sbx_get_state(int unit, args_t *args)
{
    int rv;
    char sFileName[64];
    void *fd;


    /* If we are reloading, write the state to a different file for comparison */
    if (SOC_IS_RELOADING(unit)) {
      /* coverity[secure_coding] */
      sal_sprintf(sFileName, "sbx_state_unit%d_post_reload.txt", unit);
    }else {
      /* coverity[secure_coding] */
      sal_sprintf(sFileName, "sbx_state_unit%d_pre_reload.txt", unit);
    }

    fd = sbx_archive_open(sFileName, "wb");
    if (fd < 0) {
        cli_out("ERROR: failed to open sbx_easy_reload_state.txt file error(%d)\n", (int)fd);
        return -1;
    } else {
      cli_out("Opened %s for writing\n", sFileName);
    }

    rv = bcm_sbx_cosq_get_state(unit, statebuf);

    if (rv == BCM_E_NONE) {
        cli_out("%s", statebuf);
    }
    else {
        cli_out("error from bcm_sbx_cosq_get_state() rv:%s\n", bcm_errmsg(rv));
    }

    rv = bcm_sbx_multicast_get_state(unit, statebuf1);
    if (rv == BCM_E_NONE) {
        cli_out("%s", statebuf1);
    }
    else {
        cli_out("error from bcm_sbx_multicast_get_state() rv:%s\n", bcm_errmsg(rv));
    }

    rv = bcm_sbx_port_get_state(unit, statebuf2);
    if (rv == BCM_E_NONE) {
        cli_out("%s", statebuf2);
    }
    else {
        cli_out("error from bcm_sbx_port_get_state() rv:%s\n", bcm_errmsg(rv));
    }

    rv = sbx_archive_write(fd, statebuf, STATE_BUF_SIZE, 1);
    if (rv != 1) {
        cli_out("Error writing statebuf to file\n");
    }
    rv = sbx_archive_write(fd, statebuf1, STATE_BUF_SIZE, 1);
    if (rv != 1) {
        cli_out("Error writing statebuf1 to file\n");
    }
    rv = sbx_archive_write(fd, statebuf2, STATE_BUF_SIZE, 1);
    if (rv != 1) {
        cli_out("Error writing statebuf1 to file\n");
    }
    if (sbx_archive_close(fd)) {
        cli_out("Error failed to close file\n");
    }

    return CMD_OK;
}
#endif /* BCM_EASY_RELOAD_SUPPORT_SW_DUMP */
#endif /* BCM_EASY_RELOAD_SUPPORT */

char cmd_sbx_failover_count_usage[] =
"Usage:\n"
"fo_count [options]\n"
"     reset   - Reset the failover count\n"
;

cmd_result_t
cmd_sbx_failover_count(int unit, args_t *args)
{
    char *c;

    cli_out("Current failover count is %d.\n", failover_count);

    c = ARG_GET(args);
    if (c && !sal_strcasecmp(c, "reset")) {
        failover_count = 0;
        cli_out("Failover count reset to zero.\n");
    }

    return CMD_OK;
}

int get_board_info(uint8 *board_id, uint8 *slot_id)
{
        int retv;

        retv = _mc_fpga_read8(FPGA_BOARD_ID_OFFSET, board_id);
        if (retv) {
                cli_out("get_board_info: FPGA read error\n");
                return retv;
        }
        retv = _mc_fpga_read8(FPGA_LC_PL_SLOT_ID_OFFSET, slot_id);
        if (retv) {
                cli_out("get_board_info: FPGA read error\n");
                return retv;
        }
        return 0;
}

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
/*
 * Utility routine to concatenate the first argument ("first"), with
 * the remaining arguments, with commas separating them.
 */

static void
collect_comma_args(args_t *a, char *valstr, char *first)
{
    char *s;

    sal_strcpy(valstr, first);

    while ((s = ARG_GET(a)) != 0) {
        strcat(valstr, ",");
        strcat(valstr, s);
    }
}

static void
check_global(int unit, soc_mem_t mem, char *s, int *is_global)
{
    soc_field_info_t    *fld;
    soc_mem_info_t      *m = &SOC_MEM_INFO(unit, mem);
    char                *eqpos;
    
    eqpos = strchr(s, '=');
    if (eqpos != NULL) {
        *eqpos++ = 0;
    }
    for (fld = &m->fields[0]; fld < &m->fields[m->nFields]; fld++) {
        if (!sal_strcasecmp(s, SOC_FIELD_NAME(unit, fld->field)) &&
            (fld->flags & SOCF_GLOBAL)) {
            break;
        }
    }
    if (fld == &m->fields[m->nFields]) {
        *is_global = 0;
    } else {
        *is_global = 1;
    }
}

static int
collect_comma_args_with_view(args_t *a, char *valstr, char *first, 
                             char *view, int unit, soc_mem_t mem)
{
    char *s, *s_copy = NULL, *f_copy = NULL;
    int is_global, rv = 0;

    if ((f_copy = sal_alloc(strlen(first) + 1, "first")) == NULL) {
        cli_out("cmd_sbx_cmic_mem_write : Out of memory\n");
        rv = -1;
        goto done;
    }
    memset(f_copy, 0, strlen(first) + 1);
    sal_strcpy(f_copy, first);

    /* Check if field is global before applying view prefix */
    check_global(unit, mem, f_copy, &is_global);
    if (!is_global) {
        sal_strcpy(valstr, view);
        strcat(valstr, first);
    } else {
        sal_strcpy(valstr, first);
    }

    while ((s = ARG_GET(a)) != 0) {
        if ((s_copy = sal_alloc(strlen(s) + 1, "s_copy")) == NULL) {
            cli_out("cmd_sbx_cmic_mem_write : Out of memory\n");
            rv = -1;
            goto done;
        }
        memset(s_copy, 0, strlen(s) + 1);
        sal_strcpy(s_copy, s);
        check_global(unit, mem, s_copy, &is_global);
        sal_free(s_copy);
        strcat(valstr, ",");
        if (!is_global) {
            strcat(valstr, view);
            strcat(valstr, s);
        } else {
            strcat(valstr, s);
        }
    }
done:
    if (f_copy != NULL) {
        sal_free(f_copy);
    }
    return rv;
}

static int
parse_dwords(int count, uint32 *dw, args_t *a)
{
    char        *s;
    int         i;

    for (i = 0; i < count; i++) {
        if ((s = ARG_GET(a)) == NULL) {
            cli_out("Not enough data values (have %d, need %d)\n",
                    i, count);
            return -1;
        }

        dw[i] = parse_integer(s);
    }

    if (ARG_CNT(a) > 0) {
        cli_out("Ignoring extra data on command line "
                "(only %d words needed)\n",
                count);
    }

    return 0;
}

/*
 * modify_mem_fields
 *
 *   Verify similar to modify_reg_fields (see reg.c) but works on
 *   memory table entries instead of register values.  Handles fields
 *   of any length.
 *
 *   If mask is non-NULL, it receives an entry which is a mask of all
 *   fields modified.
 *
 *   Values may be specified with optional increment or decrement
 *   amounts; for example, a MAC address could be 0x1234+2 or 0x1234-1
 *   to specify an increment of +2 or -1, respectively.
 *
 *   If incr is FALSE, the increment is ignored and the plain value is
 *   stored in the field (e.g. 0x1234).
 *
 *   If incr is TRUE, the value portion is ignored.  Instead, the
 *   increment value is added to the existing value of the field.  The
 *   field value wraps around on overflow.
 *
 *   Returns -1 on failure, 0 on success.
 */

static int
modify_mem_fields(int unit, soc_mem_t mem, uint32 *entry,
                  uint32 *mask, char *mod, int incr)
{
    soc_field_info_t    *fld;
    char                *fmod, *fval, *s;
    char                *modstr = NULL;
    uint32              fvalue[SOC_MAX_MEM_FIELD_WORDS];
    uint32              fincr[SOC_MAX_MEM_FIELD_WORDS];
    int                 i, entry_dw;
    soc_mem_info_t      *m = &SOC_MEM_INFO(unit, mem);
    char                *tokstr=NULL;

    entry_dw = BYTES2WORDS(m->bytes);

    if ((modstr = sal_alloc(ARGS_BUFFER, "modify_mem")) == NULL) {
        cli_out("modify_mem_fields: Out of memory\n");
        return CMD_FAIL;
    }

    strncpy(modstr, mod, ARGS_BUFFER);/* Don't destroy input string */
    modstr[ARGS_BUFFER - 1] = 0;
    mod = modstr;

    if (mask) {
        memset(mask, 0, entry_dw * 4);
    }

    while ((fmod = sal_strtok_r(mod, ",", &tokstr)) != 0) {
        mod = NULL;                     /* Pass strtok NULL next time */
        fval = strchr(fmod, '=');
        if (fval != NULL) {             /* Point fval to arg, NULL if none */
            *fval++ = 0;                /* Now fmod holds only field name. */
        }
        if (fmod[0] == 0) {
            cli_out("Null field name\n");
            sal_free(modstr);
            return -1;
        }
        if (!sal_strcasecmp(fmod, "clear")) {
            memset(entry, 0, entry_dw * sizeof (*entry));
            if (mask) {
                memset(mask, 0xff, entry_dw * sizeof (*entry));
            }
            continue;
        }
        for (fld = &m->fields[0]; fld < &m->fields[m->nFields]; fld++) {
            if (!sal_strcasecmp(fmod, SOC_FIELD_NAME(unit, fld->field))) {
                break;
            }
        }
        if (fld == &m->fields[m->nFields]) {
            cli_out("No such field \"%s\" in memory \"%s\".\n",
                    fmod, SOC_MEM_UFNAME(unit, mem));
            sal_free(modstr);
            return -1;
        }
        if (!fval) {
            cli_out("Missing %d-bit value to assign to \"%s\" field \"%s\".\n",
                    fld->len,
                    SOC_MEM_UFNAME(unit, mem), SOC_FIELD_NAME(unit, fld->field));
            sal_free(modstr);
            return -1;
        }
        s = strchr(fval, '+');
        if (s == NULL) {
            s = strchr(fval, '-');
        }
        if (s == fval) {
            s = NULL;
        }
        if (incr) {
            if (s != NULL) {
                parse_long_integer(fincr, SOC_MAX_MEM_FIELD_WORDS,
                                   s[1] ? &s[1] : "1");
                if (*s == '-') {
                    neg_long_integer(fincr, SOC_MAX_MEM_FIELD_WORDS);
                }
                if (fld->len & 31) {
                    /* Proper treatment of sign extension */
                    fincr[fld->len / 32] &= ~(0xffffffff << (fld->len & 31));
                }
                soc_mem_field_get(unit, mem, entry, fld->field, fvalue);
                add_long_integer(fvalue, fincr, SOC_MAX_MEM_FIELD_WORDS);
                if (fld->len & 31) {
                    /* Proper treatment of sign extension */
                    fvalue[fld->len / 32] &= ~(0xffffffff << (fld->len & 31));
                }
                soc_mem_field_set(unit, mem, entry, fld->field, fvalue);
            }
        } else {
            if (s != NULL) {
                *s = 0;
            }
            parse_long_integer(fvalue, SOC_MAX_MEM_FIELD_WORDS, fval);
            for (i = fld->len; i < SOC_MAX_MEM_FIELD_BITS; i++) {
                if (fvalue[i / 32] & 1 << (i & 31)) {
                    cli_out("Value \"%s\" too large for "
                            "%d-bit field \"%s\".\n",
                            fval, fld->len, SOC_FIELD_NAME(unit, fld->field));
                    sal_free(modstr);
                    return -1;
                }
            }
            soc_mem_field_set(unit, mem, entry, fld->field, fvalue);
        }
        if (mask) {
            memset(fvalue, 0, sizeof (fvalue));
            for (i = 0; i < fld->len; i++) {
                fvalue[i / 32] |= 1 << (i & 31);
            }
            soc_mem_field_set(unit, mem, mask, fld->field, fvalue);
        }
    }

    sal_free(modstr);
    return 0;
}

cmd_result_t
cmd_sbx_cmic_mem_write(int unit, args_t *a)
{
    int                 i, index, start, count, copyno;
    char                *tab, *idx, *cnt, *s, *memname, *slam_buf = NULL;
    soc_mem_t           mem;
    uint32              entry[SOC_MAX_MEM_WORDS];
    int                 entry_dw, view_len, entry_bytes;
    char                copyno_str[8];
    int                 r, update;
    int                 rv = CMD_FAIL;
    char                *valstr = NULL, *view = NULL;
    int                 no_cache = 0;
    int                 use_slam = 0;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        goto done;
    }

    tab = ARG_GET(a);
    if (tab != NULL && sal_strcasecmp(tab, "nocache") == 0) {
        no_cache = 1;
        tab = ARG_GET(a);
    }
    idx = ARG_GET(a);
    cnt = ARG_GET(a);
    s = ARG_GET(a);

    /* you will need at least one value and all the args .. */
    if (!tab || !idx || !cnt || !s || !isint(cnt)) {
        return CMD_USAGE;
    }

    /* Deal with VIEW:MEMORY if applicable */
    memname = strstr(tab, ":");
    view_len = 0;
    if (memname != NULL) {
        memname++;
        view_len = memname - tab;
    } else {
        memname = tab;
    }

    if (parse_memory_name(unit, &mem, memname, &copyno, 0) < 0) {
        cli_out("ERROR: unknown table \"%s\"\n",tab);
        goto done;
    }

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        cli_out("Error: Memory %s not valid for chip %s.\n",
                SOC_MEM_UFNAME(unit, mem), SOC_UNIT_NAME(unit));
        goto done;
    }

    if (soc_mem_is_readonly(unit, mem)) {
        cli_out("Error: Table %s is read-only\n",
                SOC_MEM_UFNAME(unit, mem));
        goto done;
    }

    start = parse_memory_index(unit, mem, idx);
    count = parse_integer(cnt);

    if (copyno == COPYNO_ALL) {
        copyno_str[0] = 0;
    } else {
        /* coverity[secure_coding] */
        sal_sprintf(copyno_str, ".%d", copyno);
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    valstr = sal_alloc(ARGS_BUFFER, "reg_set");
    if (valstr == NULL) {
        cli_out("cmd_sbx_cmic_mem_write : Out of memory\n");
        goto done;
    }

    /*
     * If a list of fields were specified, generate the entry from them.
     * Otherwise, generate it by reading raw dwords from command line.
     */

    if (!isint(s)) {
        /* List of fields */

        if (view_len == 0) {
            collect_comma_args(a, valstr, s);
        } else {
            if ((view = sal_alloc(view_len + 1, "view_name")) == NULL) {
                cli_out("cmd_sbx_cmic_mem_write : Out of memory\n");
                goto done;
            }
            memset(view, 0, view_len + 1);
            memcpy(view, tab, view_len);
            if (collect_comma_args_with_view(a, valstr, s, view, unit, mem) < 0) {
                cli_out("Out of memory: aborted\n");
                goto done;
            }
        }

        memset(entry, 0, sizeof (entry));

        if (modify_mem_fields(unit, mem, entry, NULL, valstr, FALSE) < 0) {
            cli_out("Syntax error: aborted\n");
            goto done;
        }

        update = TRUE;
    } else {
        /* List of numeric values */

        ARG_PREV(a);

        if (parse_dwords(entry_dw, entry, a) < 0) {
            goto done;
        }

        update = FALSE;
    }

    if (bsl_check(bslLayerAppl, bslSourceSocmem, bslSeverityNormal, unit)) {
        cli_out("WRITE[%s%s], DATA:", SOC_MEM_UFNAME(unit, mem), copyno_str);
        for (i = 0; i < entry_dw; i++) {
            cli_out(" 0x%x", entry[i]);
        }
        cli_out("\n");
    }

    /*
     * Created entry, now write it
     */
    use_slam = soc_property_get(unit, spn_DIAG_SHELL_USE_SLAM, 0);
    if (use_slam && count > 1) {
        entry_bytes = soc_mem_entry_bytes(unit, mem);
        slam_buf = soc_cm_salloc(unit, count * entry_bytes, "slam_entry");
        if (slam_buf == NULL) {
            cli_out("cmd_sbx_cmic_mem_write : Out of memory\n");
            goto done;
        }
        for (i = 0; i < count; i++) {
            sal_memcpy(slam_buf + i * entry_bytes, entry, entry_bytes);
        }
        /*    coverity[negative_returns : FALSE]    */
        r = soc_mem_write_range(unit, mem, copyno, start, start + count - 1, slam_buf);
        soc_cm_sfree(unit, slam_buf);
        if (r < 0) {
            cli_out("Slam ERROR: table %s.%s: %s\n",
                    SOC_MEM_UFNAME(unit, mem), copyno_str, soc_errmsg(r));
            goto done;
        }
        for (index = start; index < start + count; index++) {
            if (update) {
                modify_mem_fields(unit, mem, entry, NULL, valstr, TRUE);
            }
        }
    } else {
        for (index = start; index < start + count; index++) {
            if ((r = soc_mem_write(unit, mem, copyno,
                                   no_cache ? -index : index, entry)) < 0) {
                cli_out("Write ERROR: table %s.%s[%d]: %s\n",
                        SOC_MEM_UFNAME(unit, mem), copyno_str,
                        index, soc_errmsg(r));
                goto done;
            }

            if (update) {
                modify_mem_fields(unit, mem, entry, NULL, valstr, TRUE);
            }
        }
    }
    rv = CMD_OK;

 done:
    sal_free(valstr);
    if (view != NULL) {
       sal_free(view);
    }
    return rv;
}

char cmd_sbx_mem_write_usage[] =
    "Parameters: <TABLE>[.<COPY>] <ENTRY> <ENTRYCOUNT>\n\t"
    "        { <DW0> .. <DWN> | <FIELD>=<VALUE>[,...] }\n\t"
    "Number of <DW> must be a multiple of table entry size.\n\t"
    "Writes entry(s) into table index(es).\n";

cmd_result_t
cmd_sbx_mem_write(int u, args_t *a)
{
    if (SOC_IS_SIRIUS(u) || SOC_IS_CALADAN3(u)) {
        return (cmd_sbx_cmic_mem_write(u, a));
    } else {
        cli_out("Invalid command for unit %d device type\n",u);
        return CMD_FAIL;
    }
}

/*
 * Modify the fields of a table entry
 */
cmd_result_t
cmd_sbx_cmic_mem_modify(int unit, args_t *a)
{
    int			index, start, count, copyno, i, view_len;
    char		*tab, *idx, *cnt, *s, *memname;
    soc_mem_t		mem;
    uint32		entry[SOC_MAX_MEM_WORDS], mask[SOC_MAX_MEM_WORDS];
    uint32		changed[SOC_MAX_MEM_WORDS];
    char		*valstr = NULL, *view = NULL;
    int			r, rv = CMD_FAIL;
    int			blk;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
	return CMD_FAIL;
    }

    tab = ARG_GET(a);
    idx = ARG_GET(a);
    cnt = ARG_GET(a);
    s = ARG_GET(a);

    /* you will need at least one dword and all the args .. */
    if (!tab || !idx || !cnt || !s || !isint(cnt)) {
	return CMD_USAGE;
    }

    if ((valstr = sal_alloc(ARGS_BUFFER, "mem_modify")) == NULL) {
        cli_out("cmd_esw_mem_modify : Out of memory\n");
        goto done;
    }

    memname = strstr(tab, ":"); 
    view_len = 0; 
    if (memname != NULL) { 
        memname++; 
        view_len = memname - tab; 
    } else { 
        memname = tab; 
    } 

    if (parse_memory_name(unit, &mem, memname, &copyno, 0) < 0) {
	cli_out("ERROR: unknown table \"%s\"\n",tab);
    goto done;
    }

    if (view_len == 0) {
        collect_comma_args(a, valstr, s);
    } else {
        if ((view = sal_alloc(view_len + 1, "view_name")) == NULL) {
            cli_out("cmd_esw_mem_modify : Out of memory\n");
            goto done;
        }

        memcpy(view, tab, view_len);
        view[view_len] = 0;

        if (collect_comma_args_with_view(a, valstr, s, view, unit, mem) < 0) {
            cli_out("Out of memory: aborted\n");
            goto done;
        }
    }

    if (!SOC_MEM_IS_VALID(unit, mem)) {
        cli_out("Error: Memory %s not valid for chip %s.\n",
                SOC_MEM_UFNAME(unit, mem), SOC_UNIT_NAME(unit));
        goto done;
    }

    if (soc_mem_is_readonly(unit, mem)) {
	cli_out("ERROR: Table %s is read-only\n", SOC_MEM_UFNAME(unit, mem));
        goto done;
    }

    memset(changed, 0, sizeof (changed));

    if (modify_mem_fields(unit, mem, changed, mask, valstr, FALSE) < 0) {
	cli_out("Syntax error: aborted\n");
        goto done;
    }

    start = parse_memory_index(unit, mem, idx);
    count = parse_integer(cnt);

    /*
     * Take lock to ensure atomic modification of memory.
     */

    soc_mem_lock(unit, mem);

    rv = CMD_OK;

    for (index = start; index < start + count; index++) {
	SOC_MEM_BLOCK_ITER(unit, mem, blk) {
	    if (copyno != COPYNO_ALL && copyno != blk) {
		continue;
	    }

	    /*
	     * Retrieve the current entry, set masked fields to changed
	     * values, and write back.
	     */
	    r = soc_mem_read(unit, mem, blk, index, entry);

	    if (r < 0) {
		cli_out("ERROR: read from %s table copy %d failed: %s\n",
                        SOC_MEM_UFNAME(unit, mem), blk, soc_errmsg(r));
		rv = CMD_FAIL;
		break;
	    }

	    for (i = 0; i < SOC_MAX_MEM_WORDS; i++) {
		entry[i] = (entry[i] & ~mask[i]) | changed[i];
	    }

	    r = soc_mem_write(unit, mem, blk, index, entry);

	    if (r < 0) {
		cli_out("ERROR: write to %s table copy %d failed: %s\n",
                        SOC_MEM_UFNAME(unit, mem), blk, soc_errmsg(r));
		rv = CMD_FAIL;
		break;
	    }
	}

	if (rv != CMD_OK) {
	    break;
	}

	modify_mem_fields(unit, mem, changed, NULL, valstr, TRUE);
    }

    soc_mem_unlock(unit, mem);

 done:
    if (view != NULL) {
        sal_free(view);
    }
    sal_free(valstr);
    return rv;
}

char cmd_sbx_mem_modify_usage[] =
    "Parameters: <TABLE>[.<COPY>] <ENTRY> <ENTRYCOUNT>\n\t"
    "        <FIELD>=<VALUE>[,...]\n\t"
    "Read/modify/write field(s) of a table entry(s).\n";

cmd_result_t
cmd_sbx_mem_modify(int u, args_t *a)
{
    if (SOC_IS_SIRIUS(u) || SOC_IS_CALADAN3(u)) {
        return (cmd_sbx_cmic_mem_modify(u, a));
    } else {
        cli_out("Invalid command for unit %d device type\n",u);
        return CMD_FAIL;
    }
}
#endif /* BCM_SIRIUS_SUPPORT || BCM_CALADAN3_SUPPORT */

char cmd_sbx_pcic_write_usage[] =
    "Writes pcic bus=<num> dev=<num> func=<num> offset=<num> data=<num>.\n";

cmd_result_t
cmd_sbx_pcic_write(int u, args_t *a)
{
    int rv=CMD_FAIL;
    int bus=0, dev=0, func=0, offset=0, data=0;

    if (ARG_CNT(a)) {
        parse_table_t pt;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "bus", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &bus, NULL);
        parse_table_add(&pt, "dev", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &dev, NULL);
        parse_table_add(&pt, "func", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &func, NULL);
        parse_table_add(&pt, "offset", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &offset, NULL);
        parse_table_add(&pt, "data", PQ_DFL | PQ_HEX | PQ_NO_EQ_OPT,
                        0, &data, NULL);
        if (!parseEndOk(a, &pt, &rv)) {
            return rv;
        }
    }

#ifdef VXWORKS
    cli_out("writing data 0x%x to bus %d dev %d func %d offset %d\n", data, bus, dev, func, offset);
    pciConfigOutLong(bus,dev,func,offset,(unsigned int)data);
#endif

    return CMD_OK;

}

char cmd_sbx_pcic_read_usage[] =
    "Read pcic bus=<num> dev=<num> func=<num> offset=<num>.\n";

cmd_result_t
cmd_sbx_pcic_read(int u, args_t *a)
{
#ifdef VXWORKS
    int rv=CMD_FAIL;
    int bus=0, dev=0, func=0, offset=0;
    unsigned int data;

    if (ARG_CNT(a)) {
        parse_table_t pt;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "bus", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &bus, NULL);
        parse_table_add(&pt, "dev", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &dev, NULL);
        parse_table_add(&pt, "func", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &func, NULL);
        parse_table_add(&pt, "offset", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &offset, NULL);
        if (!parseEndOk(a, &pt, &rv)) {
            return rv;
        }
    }

    pciConfigInLong(bus,dev,func,offset,&data);
    cli_out("Read data 0x%x from bus %d dev %d func %d offset %d\n", data, bus, dev, func, offset);
#endif

    return CMD_OK;
}

static int
_sbx_mim_vpn_display(int unit, bcm_mim_vpn_config_t *info)
{
    cli_out("-- VPN 0x%04x --\n", info->vpn);
    cli_out("flags:  0x%08x    lookupId:  0x%08x  tpid: 0x%04x\n",
            info->flags, info->lookup_id, info->match_service_tpid);
    cli_out("broadcastGroup:        0x%08x\n", info->broadcast_group);
    cli_out("unknownUnicastGroup:   0x%08x\n", info->unknown_unicast_group);
    cli_out("unknownMulticastGroup: 0x%08x\n", info->unknown_multicast_group);

    return BCM_E_NONE;
}

static int
_sbx_mim_port_display(int unit, bcm_mim_port_t *mimPort)
{
    cli_out("\n--------------------------------------\n");
    if(mimPort->flags & BCM_MIM_PORT_TYPE_BACKBONE) {
        if(BCM_MAC_IS_ZERO(mimPort->egress_tunnel_dstmac)) {
            cli_out("$$ DEFAULT BACKBONE PORT $$\n");
        } else {
            cli_out("##  BACKBONE PORT ##\n");
        }
    } else if(mimPort->flags & BCM_MIM_PORT_TYPE_ACCESS) {
        cli_out("^^  ACCESS PORT ^^\n");
    } else {
        cli_out("\n Unrecognized MiM Port Type \n");
    }
    cli_out("--------------------------------------\n");

    cli_out("MIM GPORT 0x%08x\n", mimPort->mim_port_id);
    cli_out("flags:  0x%08x  port:  ", mimPort->flags);
    if (BCM_GPORT_IS_MODPORT(mimPort->port)) {
        cli_out("Mod/Port %d/%d\n",
                BCM_GPORT_MODPORT_MODID_GET(mimPort->port),
                BCM_GPORT_MODPORT_PORT_GET(mimPort->port));

    } else if (BCM_GPORT_IS_TRUNK(mimPort->port)) {
        cli_out("Trunk 0x%x\n", BCM_GPORT_TRUNK_GET(mimPort->port));
    } else {
        cli_out("Gport 0x%08x\n", mimPort->port);
    }
    cli_out("criteria: 0x%x  matchVlan: 0x%03x matchTunnelVlan:  0x%03x\n",
            mimPort->criteria, mimPort->match_vlan, mimPort->match_tunnel_vlan);
    cli_out("matchTunnelSrcMac: " MAC_FMT "\n",
            MAC_PFMT(mimPort->match_tunnel_srcmac));
    cli_out("matchServiceTpid:  0x%04x\n", mimPort->match_service_tpid);
    cli_out("egressTunnelVlan:  0x%04x\n", mimPort->egress_tunnel_vlan);
    cli_out("egressTunnelSrcMac: " MAC_FMT "\n",
            MAC_PFMT(mimPort->egress_tunnel_srcmac));
    cli_out("egressTunnelDestMac: " MAC_FMT "\n",
            MAC_PFMT(mimPort->egress_tunnel_dstmac));
    cli_out("egressTunnelService:  0x%06x  egressServiceTpid:  0x%04x\n",
            mimPort->egress_tunnel_service, mimPort->egress_service_tpid);
    cli_out("egressServiceVlan:  0x%03x\n", mimPort->egress_service_vlan);
    cli_out("EncapId: 0x%08x\n", mimPort->encap_id);
    cli_out("\n--------------END--------------------\n");
    return BCM_E_NONE;
}

typedef struct _sbx_mim_vpn_display_data_s {
    bcm_mim_port_t* portArray;
    int             numPorts;
} _sbx_mim_vpn_display_data_t;

static int
_sbx_mim_vpn_traverse_display(int unit,
                              bcm_mim_vpn_config_t *info,
                              void *user_data)
{
    _sbx_mim_vpn_display_data_t *workBuffer;
    int portCount = 0;
    int rv, index=0;

    workBuffer = (_sbx_mim_vpn_display_data_t *)user_data;

    _sbx_mim_vpn_display(unit, info);

    rv = bcm_mim_port_get_all(unit, info->vpn, workBuffer->numPorts,
                              workBuffer->portArray, &portCount);
    if (BCM_FAILURE(rv)) {
        cli_out("Failed to get_all mim ports: %d %s\n", rv, bcm_errmsg(rv));
        return rv;
    }

    cli_out("Displaying %d mim ports\n", portCount);
    if(portCount > 0) {
        for(index=0; index < portCount; index++) {
            _sbx_mim_port_display(unit, &workBuffer->portArray[index]);
        }
    }

    return BCM_E_NONE;
}

#define CMD_MIM_USAGE_DISPLAY \
"  mim display <[vpn=<vpnId>] [port=<mimGport>] [all]> - \n" \
"                    all port members of a vpn, a mim gport, or \n" \
"                    all vpns and all ports"

static cmd_result_t
_cmd_sbx_mim_display(int unit, args_t *args)
{
    _sbx_mim_vpn_display_data_t tmpBuf;
    int ret_code;
    parse_table_t pt;
    int rv;
    int vpn = ~0;
    int port = ~0;
    int all = 0;

    if (!ARG_CNT(args)) {
        cli_out("Must supply at least one argument\n");
        return CMD_USAGE;
    }

    parse_table_init(0, &pt);
    parse_table_add(&pt, "vpn",  PQ_DFL | PQ_INT, 0, &vpn, NULL);
    parse_table_add(&pt, "port", PQ_DFL | PQ_INT, 0, &port, NULL);
    parse_table_add(&pt, "all",   PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                    0, &all, NULL);
    if (!parseEndOk(args, &pt, &ret_code)) {
        return ret_code;
    }


    if (all || ((vpn != ~0) && (port == ~0))) {
        /* display all vpns and all ports */

        tmpBuf.numPorts = 32;
        tmpBuf.portArray = sal_alloc(sizeof(bcm_mim_port_t) * tmpBuf.numPorts,
                                     "mim port buffer");
        sal_memset(tmpBuf.portArray, 0, sizeof(bcm_mim_port_t) * tmpBuf.numPorts);
        if (all) {
            rv = bcm_mim_vpn_traverse(unit, _sbx_mim_vpn_traverse_display,
                                      (void*)&tmpBuf);
        } else {
            /* display all ports in vpn */
            int portCount = 0;

            rv = bcm_mim_port_get_all(unit, vpn, tmpBuf.numPorts,
                                      tmpBuf.portArray, &portCount);
            if (BCM_FAILURE(rv)) {
                cli_out("Failed to get_all mim ports: %d %s\n", rv, bcm_errmsg(rv));
                return rv;
            }
            
            cli_out("Displaying %d mim ports\n", portCount);
            while (portCount > 0) {
                _sbx_mim_port_display(unit, &tmpBuf.portArray[portCount]);
                portCount--;
            }
        }
        
        sal_free(tmpBuf.portArray);
        tmpBuf.portArray = NULL;

        cli_out("traversal returned: %d %s\n", rv, bcm_errmsg(rv));
    } else if ((vpn != ~0) && (port != ~0)) {
        /* display one port */
        bcm_mim_port_t mimPort;

        bcm_mim_port_t_init(&mimPort);
        mimPort.mim_port_id = port;

        rv = bcm_mim_port_get(unit, vpn, &mimPort);
        if (BCM_SUCCESS(rv)) {
            _sbx_mim_port_display(unit, &mimPort);
        } else {
            cli_out("Failed to find mim port: %d %s\n", rv, bcm_errmsg(rv));
        }

    } else {
        cli_out("Must specify port or vpn\n");
        return CMD_USAGE;
    }

    return CMD_OK;
}


char cmd_sbx_mim_usage[] =
"\n"
"  mim <option> [args...]\n"
"    Supported on FE devices only.\n\n"
"  Commands:\n"
    CMD_MIM_USAGE_DISPLAY  "\n"
;

static cmd_t _cmd_sbx_mim_list[] = {
    {"DISPLAY",    _cmd_sbx_mim_display,   "\n" CMD_MIM_USAGE_DISPLAY,   NULL},
};

cmd_result_t
cmd_sbx_mim(int unit, args_t *args)
{
    if (!SOC_IS_SBX_FE(unit)) {
        cli_out("Command only supported on FE\n");
        return CMD_USAGE;
    }

    return subcommand_execute(unit, args,
                              _cmd_sbx_mim_list, COUNTOF(_cmd_sbx_mim_list));
}


/* Sample scripts for MiM */
bcm_multicast_t mim_id1_vpn_bcast_group;
bcm_mim_vpn_t mim_id1_vpn_id;
int mim_id1_bvid;

cmd_result_t
cmd_sbx_mim_portmode_config(int feunit, int qeunit, args_t *args)
{
    bcm_mim_vpn_config_t vpnInfo;
    bcm_pbmp_t pbm, upbm;
    bcm_mim_port_t access_mimport, def_backbone_mimport, learn_backbone_mimport;
    bcm_if_t encap_id;
    int module_id;
    bcm_gport_t fegport, qegport;
    bcm_gport_t defbbonegport, accessgport, modportgport, learnbbonegport;
    uint32 mcgflags = BCM_MULTICAST_WITH_ID | BCM_MULTICAST_TYPE_L2;
    bcm_multicast_t vpn_bcast_group;
    int i_sid = 20; /* I-SID */
    bcm_mim_vpn_t vpn_id = 0;
    int ret = 0;
    int bvid=20;
    int isid_vsi=0;
    bcm_mac_t cdmac;
    bcm_l2_addr_t l2_addr;
    
    /* Create BVLAN add add back bone ports as test1 */
    /* Create BVLAN add add back bone ports as members */
    BCM_PBMP_CLEAR(pbm);
    BCM_PBMP_PORT_ADD(pbm, 21);
    BCM_PBMP_CLEAR(upbm);
    BCM_IF_ERROR_RETURN(bcm_vlan_create(feunit, bvid));
    BCM_IF_ERROR_RETURN(bcm_vlan_port_add(feunit, bvid, pbm, upbm));

    BCM_IF_ERROR_RETURN(bcm_multicast_create(qeunit,  mcgflags, &bvid));
    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(feunit, &module_id));
    BCM_GPORT_MODPORT_SET(fegport, module_id, 21);
    BCM_IF_ERROR_RETURN(bcm_multicast_l2_encap_get(feunit, bvid, fegport, bvid, &encap_id));

    BCM_IF_ERROR_RETURN(bcm_stk_fabric_map_get(feunit, fegport, &qegport));
    BCM_IF_ERROR_RETURN(bcm_multicast_egress_add(qeunit, bvid, qegport, encap_id));
    
    /* create a multicast group for this VPN */
    BCM_IF_ERROR_RETURN(bcm_multicast_create(qeunit,  BCM_MULTICAST_TYPE_MIM, &vpn_bcast_group));
    mim_id1_vpn_bcast_group = vpn_bcast_group;
    
    bcm_mim_vpn_config_t_init(&vpnInfo);
    
    vpnInfo.flags = BCM_MIM_VPN_MIM;
    vpnInfo.lookup_id = i_sid;
    vpnInfo.broadcast_group = vpn_bcast_group;
    vpnInfo.unknown_unicast_group = vpn_bcast_group;
    vpnInfo.unknown_multicast_group = vpn_bcast_group;

    cli_out("###bcm_mim_vpn_create start\n");
    ret = bcm_mim_vpn_create(feunit , &vpnInfo);
    cli_out("###bcm_mim_vpn_create end ret = %x \n",ret);
    
    vpn_id = vpnInfo.vpn;
    isid_vsi = vpn_id;
    cli_out("###vpn_id = %d \n",vpn_id);
    mim_id1_vpn_id = vpn_id;
    mim_id1_bvid = bvid;
    
    /* Create a Default Back bone port */
    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(feunit, &module_id));
    BCM_GPORT_MODPORT_SET(defbbonegport, module_id, 21);
    bcm_mim_port_t_init(&def_backbone_mimport);
    def_backbone_mimport.flags = BCM_MIM_PORT_TYPE_BACKBONE ;
    def_backbone_mimport.port = defbbonegport;
    def_backbone_mimport.criteria = BCM_MIM_PORT_MATCH_TUNNEL_VLAN_SRCMAC;
    def_backbone_mimport.match_tunnel_srcmac[0] = 0xCC;
    def_backbone_mimport.match_tunnel_srcmac[1] = 0xCC;
    def_backbone_mimport.match_tunnel_srcmac[2] = 0xCC;
    def_backbone_mimport.match_tunnel_srcmac[3] = 0xCC;
    def_backbone_mimport.match_tunnel_srcmac[4] = 0xCC;
    def_backbone_mimport.match_tunnel_srcmac[5] = 0xCC;
    def_backbone_mimport.match_tunnel_vlan   = bvid;
    def_backbone_mimport.egress_service_vlan = bvid;
    def_backbone_mimport.egress_service_tpid = 0x88a8;
    def_backbone_mimport.egress_tunnel_srcmac[0] = 0xCC;
    def_backbone_mimport.egress_tunnel_srcmac[1] = 0xCC;
    def_backbone_mimport.egress_tunnel_srcmac[2] = 0xCC;
    def_backbone_mimport.egress_tunnel_srcmac[3] = 0xCC;
    def_backbone_mimport.egress_tunnel_srcmac[4] = 0xCC;
    def_backbone_mimport.egress_tunnel_srcmac[5] = 0xCC;
     
    cli_out("### Default Back bone bcm_mim_port_add start\n");
    ret = bcm_mim_port_add(feunit , vpn_id , &def_backbone_mimport);
    cli_out("###bcm_mim_port_add end ret = %x \n",ret);
    defbbonegport = def_backbone_mimport.mim_port_id;
    
    /* Create a Port Mode Access port */
    BCM_GPORT_MODPORT_SET(accessgport, module_id, 20);
    bcm_mim_port_t_init(&access_mimport);
    access_mimport.port = accessgport;
    access_mimport.flags    = BCM_MIM_PORT_TYPE_ACCESS;
    access_mimport.criteria = BCM_MIM_PORT_MATCH_PORT;

    cli_out("### Port Based Access - bcm_mim_port_add start\n");
    ret = bcm_mim_port_add(feunit, vpn_id, &access_mimport);

    cli_out("###bcm_mim_port_add end ret = %x \n",ret);
    accessgport = access_mimport.mim_port_id;

   /* Add the Default Back bone to VPN Multicast Group */
   BCM_GPORT_MODPORT_SET(modportgport, module_id, 21);
   BCM_IF_ERROR_RETURN(bcm_multicast_mim_encap_get(feunit, vpn_bcast_group, modportgport, 
                                                    defbbonegport, &encap_id));
   BCM_IF_ERROR_RETURN(bcm_stk_fabric_map_get(feunit,  modportgport, &qegport));
   BCM_IF_ERROR_RETURN(bcm_multicast_egress_add(qeunit, vpn_bcast_group, qegport, encap_id));

   /* Add the Access Port to VPN Multicast Group */
   BCM_GPORT_MODPORT_SET(modportgport, module_id, 20);
   BCM_IF_ERROR_RETURN(bcm_multicast_mim_encap_get(feunit, vpn_bcast_group, modportgport, 
                                                    accessgport, &encap_id));
   BCM_IF_ERROR_RETURN(bcm_stk_fabric_map_get(feunit,  modportgport, &qegport));
   BCM_IF_ERROR_RETURN(bcm_multicast_egress_add(qeunit, vpn_bcast_group, qegport, encap_id));

    cli_out("###mim_test3 end\n");


    /* Known Unicast Tunnel Entry configurations */
    cli_out("\n $$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    cli_out("$$$$$$$$$$$$$$ Known Unicast Tunnel Entry Configurations $$$$$$$$\n");
    cli_out("$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$\n");
    /* Create a Back bone port for Learned B-DMAC*/
    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(feunit, &module_id));
    BCM_GPORT_MODPORT_SET(modportgport, module_id, 21);
    bcm_mim_port_t_init(&learn_backbone_mimport);
    learn_backbone_mimport.flags = BCM_MIM_PORT_TYPE_BACKBONE ;
    learn_backbone_mimport.port = modportgport;
    learn_backbone_mimport.criteria = BCM_MIM_PORT_MATCH_TUNNEL_VLAN_SRCMAC;
    learn_backbone_mimport.match_tunnel_srcmac[0] = 0xCC;
    learn_backbone_mimport.match_tunnel_srcmac[1] = 0xCC;
    learn_backbone_mimport.match_tunnel_srcmac[2] = 0xCC;
    learn_backbone_mimport.match_tunnel_srcmac[3] = 0xCC;
    learn_backbone_mimport.match_tunnel_srcmac[4] = 0xCC;
    learn_backbone_mimport.match_tunnel_srcmac[5] = 0xCC;

    learn_backbone_mimport.match_tunnel_vlan   = bvid;
    learn_backbone_mimport.egress_service_vlan = bvid;
    learn_backbone_mimport.egress_service_tpid = 0x88a8;

    learn_backbone_mimport.egress_tunnel_srcmac[0] = 0xCC;
    learn_backbone_mimport.egress_tunnel_srcmac[1] = 0xCC;
    learn_backbone_mimport.egress_tunnel_srcmac[2] = 0xCC;
    learn_backbone_mimport.egress_tunnel_srcmac[3] = 0xCC;
    learn_backbone_mimport.egress_tunnel_srcmac[4] = 0xCC;
    learn_backbone_mimport.egress_tunnel_srcmac[5] = 0xCC;

    learn_backbone_mimport.egress_tunnel_dstmac[0] = 0xAA;
    learn_backbone_mimport.egress_tunnel_dstmac[1] = 0xAA;
    learn_backbone_mimport.egress_tunnel_dstmac[2] = 0xAA;
    learn_backbone_mimport.egress_tunnel_dstmac[3] = 0xAA;
    learn_backbone_mimport.egress_tunnel_dstmac[4] = 0xAA;
    learn_backbone_mimport.egress_tunnel_dstmac[5] = 0xAA;

    cli_out("### Learned Back bone bcm_mim_port_add start\n");
    ret = bcm_mim_port_add(feunit , vpn_id , &learn_backbone_mimport);
    cli_out("###bcm_mim_port_add end ret = %x \n",ret);

    /* Only Default Back bone ports needs to added to vpn bcast group, not required to add
     * leared bmac back bone ports */
    
    /* Add the Learned CMAC over the Back Bone GPORT */
    /* get the mim GPORT ID created by SDK on bcm_mim_port_add */
    learnbbonegport = learn_backbone_mimport.mim_port_id;
    cdmac[0] = 0x00;
    cdmac[1] = 0x02;
    cdmac[2] = 0x03;
    cdmac[3] = 0x04;
    cdmac[4] = 0x05;
    cdmac[5] = 0x06;
    bcm_l2_addr_t_init(&l2_addr, cdmac, isid_vsi);
    l2_addr.port = learnbbonegport;
    l2_addr.modid = module_id;
    BCM_IF_ERROR_RETURN(bcm_l2_addr_add(feunit, &l2_addr));


    cli_out("\n************************* \n");
    cli_out("  TUNNEL EXIT CONFIGURATION ");
    cli_out("\n***************************\n");
   
    /*  To Test Known Unicast from Backbone 21 into Access port 20
        BDMAC - must be equal to the Local Station MAC CC:CC:CC:CC:CC:CC
        Since BSMAC exists for AA:AA:AA:AA:AA:AA, L2 lookup is performed on the CDMAC.
        Add a L2 MAC address to forward the CDMAC to the access port*/

 
    /* Add the Learned CMAC over the Access port GPORT */
    cdmac[0] = 0x00;
    cdmac[1] = 0xBB;
    cdmac[2] = 0xCC;
    cdmac[3] = 0xDD;
    cdmac[4] = 0xEE;
    cdmac[5] = 0xFF;
    bcm_l2_addr_t_init(&l2_addr, cdmac, isid_vsi);

    /* get the mim GPORT ID created by SDK on bcm_mim_port_add */
    l2_addr.port = accessgport;
    l2_addr.modid = module_id;
    BCM_IF_ERROR_RETURN(bcm_l2_addr_add(feunit, &l2_addr));
    return CMD_OK;
}

cmd_result_t
cmd_sbx_mim_portmode_config_destory(int feunit, int qeunit, args_t *args)
{
    int module_id;
    bcm_gport_t fegport,qegport;
    bcm_if_t encap_id;

    /* delete all ports on vpn */
    BCM_IF_ERROR_RETURN(bcm_mim_port_delete_all(feunit, mim_id1_vpn_id));

    /* destroy vpn multicast group */
    BCM_IF_ERROR_RETURN(bcm_multicast_destroy(qeunit,mim_id1_vpn_bcast_group));

    /* destroy vpn */
    BCM_IF_ERROR_RETURN(bcm_mim_vpn_destroy(feunit,mim_id1_vpn_id)); 

    /* destroy vlan */ 
    /* prune port from the mcast group */
    BCM_IF_ERROR_RETURN(bcm_stk_modid_get(feunit, &module_id));
    BCM_GPORT_MODPORT_SET(fegport, module_id, 21);
    BCM_IF_ERROR_RETURN(bcm_multicast_l2_encap_get(feunit, mim_id1_bvid, fegport, mim_id1_bvid, &encap_id));
    BCM_IF_ERROR_RETURN(bcm_stk_fabric_map_get(feunit, fegport, &qegport));
    BCM_IF_ERROR_RETURN(bcm_multicast_egress_delete(qeunit, mim_id1_bvid, qegport, encap_id));
    BCM_IF_ERROR_RETURN(bcm_vlan_destroy(feunit, mim_id1_bvid));
    return CMD_OK;
}

cmd_result_t
cmd_sbx_mcast_join_config(int feunit, int qeunit, int joinport)
{
    bcm_pbmp_t pbm, upbm;
    bcm_if_t encap_id;
    int module_id;
    bcm_gport_t fegport, qegport;
    int ret = 0;
    int vlan=3;
    int mcgroup=0;
    bcm_mac_t mac;
    bcm_l2_addr_t l2Addr;
    int port;

    
    
    /* Create VLAN add add ports ge0-ge4 */
    BCM_PBMP_CLEAR(pbm);
    for(port=0; port<4; port++) {
        BCM_PBMP_PORT_ADD(pbm, port);
    }
    BCM_PBMP_CLEAR(upbm);
    ret = bcm_vlan_create(feunit, vlan);

    if(ret == BCM_E_NONE) { 
        BCM_IF_ERROR_RETURN(bcm_vlan_port_add(feunit, vlan, pbm, upbm));
        BCM_IF_ERROR_RETURN(bcm_multicast_create(qeunit,  
                                                 BCM_MULTICAST_WITH_ID | BCM_MULTICAST_TYPE_L2, 
                                                 &vlan));

        BCM_IF_ERROR_RETURN(bcm_stk_modid_get(feunit, &module_id));

        /*ge0 - ge4*/
        for(port=0; port<4; port++) {
            BCM_GPORT_MODPORT_SET(fegport, module_id, port);
            BCM_IF_ERROR_RETURN(bcm_multicast_l2_encap_get(feunit, vlan, fegport, vlan, &encap_id));
            BCM_IF_ERROR_RETURN(bcm_stk_fabric_map_get(feunit, fegport, &qegport));
            BCM_IF_ERROR_RETURN(bcm_multicast_egress_add(qeunit, vlan, qegport, encap_id));
        }
    } else if (ret == BCM_E_EXISTS) {
        /* vlan already exists so ignore.. */
        BCM_IF_ERROR_RETURN(bcm_stk_modid_get(feunit, &module_id));
    } else {
        cli_out("\n Internal Error \n");
        return CMD_FAIL;
    }

    /* Check if L2 Multicast Address Exists on Table, if it does it is sufficient 
     * to Add port membership to multicast Group */
    mac[0] = 0x01;
    mac[1] = 0x00;
    mac[2] = 0x5E;
    mac[3] = 0x00;
    mac[4] = 0x00;
    mac[5] = 0x01;

    ret = bcm_l2_addr_get(feunit, mac, vlan, &l2Addr);
    if(ret == BCM_E_NONE) {
        /* if mac is present, use the l2 mcindex from the l2 address */
        mcgroup = l2Addr.l2mc_group;
    } else if (ret == BCM_E_NOT_FOUND) {
        /* create a L2 multicast group for IGMP join */
        BCM_IF_ERROR_RETURN(bcm_multicast_create(qeunit, BCM_MULTICAST_TYPE_L2, &mcgroup));
    } else {
        cli_out("\n Internal Error \n");
        return CMD_FAIL;
    }
    
    /* Add the required port lets say port0 to multicast group */
    BCM_GPORT_MODPORT_SET(fegport, module_id, joinport); 
    /* This is not an error and the parameter to pass is vlan */
    /* coverity[copy_paste_error] */
    BCM_IF_ERROR_RETURN(bcm_multicast_l2_encap_get(feunit, mcgroup, fegport, vlan, &encap_id));
    BCM_IF_ERROR_RETURN(bcm_stk_fabric_map_get(feunit, fegport, &qegport));
    BCM_IF_ERROR_RETURN(bcm_multicast_egress_add(qeunit, mcgroup, qegport, encap_id));  
    
    if(ret == BCM_E_NOT_FOUND) {
        /* Add L2 Multicast Address to Fe */
        bcm_l2_addr_t_init(&l2Addr, mac, vlan);
        l2Addr.l2mc_group = mcgroup;
        l2Addr.flags  = BCM_L2_MCAST | BCM_L2_STATIC ;
        BCM_IF_ERROR_RETURN(bcm_l2_addr_add(feunit, &l2Addr));
    }

    return CMD_OK;
}

char cmd_sbx_mim_test_usage[] =
"\n"
"  mim_test <testid> [args...]\n"
"    Supported on FE devices only.\n\n"
"  Commands:\n"
"   ID \n"
"      1 - configures portbased access port vpn \n"
"      2 - Mcast Join -  \n"
"      3 - delete test ID 1 configuration   \n"
"          - port -> port number to join \n"
;

cmd_result_t
cmd_sbx_mim_test(int unit, args_t *args)
{
    int id=0;
    int port;
    int rv=CMD_FAIL;

    if (!SOC_IS_SBX_FE(unit)) {
        cli_out("Command only supported on FE\n");
        return CMD_USAGE;
    }

    if (ARG_CNT(args)) {
        parse_table_t pt;
        parse_table_init(0, &pt);
        parse_table_add(&pt, "ID", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &id, NULL);
        parse_table_add(&pt, "port", PQ_DFL | PQ_INT | PQ_NO_EQ_OPT,
                        0, &port, NULL);
        if (!parseEndOk(args, &pt, &rv)) {
            return rv;
        }
    }

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
    case SOC_SBX_UCODE_TYPE_G2P3:
        if(id == 1) {
            cmd_sbx_mim_portmode_config(unit, 0, args);
        } else if(id == 2) {
            cmd_sbx_mcast_join_config(unit, 0, port);
        } else if(id == 3) {
            cmd_sbx_mim_portmode_config_destory(unit, 0, args);
        }else {
            cli_out("\n Unsupported Test ID  ERROR \n");
        }
        break;
    default:
        cli_out("ERROR: unsupported microcode type: %d\n",
                SOC_SBX_CONTROL(unit)->ucodetype);
        rv = CMD_FAIL;
    }
    return rv;
}

#ifdef BCM_WARM_BOOT_SUPPORT
#define SBX_DIAG_STABLE_SIZE_DEFAULT (16 * 1024 * 1024)
static uint32 sbx_diag_stable_size = SBX_DIAG_STABLE_SIZE_DEFAULT;
static uint8* sbx_diag_stable;
/* Note that to/from file functionality is controlled via SOC param stable_filename*/
#ifndef NO_FILEIO
static FILE* sbx_diag_stable_fd = 0; /* file descriptor */
#endif

static int
sbx_diag_stable_read(int unit, uint8 *buf, int offset, int nbytes)
{
#ifndef NO_FILEIO
  if (sbx_diag_stable_fd){
    int result;
    /* First read bytes out of file, if stable_filename was provided*/
    if (0 != fseek(sbx_diag_stable_fd, offset, SEEK_SET)) {
      return SOC_E_FAIL;
    }
    result = fread(sbx_diag_stable + offset , 1, nbytes, sbx_diag_stable_fd);
    if (result != nbytes) {
      return SOC_E_MEMORY;
    }
  } 
#endif
    sal_memcpy(buf, sbx_diag_stable + offset, nbytes);
    return SOC_E_NONE;
}

static int 
sbx_diag_stable_write(int unit, uint8 *buf, int offset, int nbytes)
{
    cli_out("%d: %s:  Writing: 0x%08x at offset 0x%08x, 0x%08x bytes\n",
            unit, FUNCTION_NAME(), (int)buf, offset, nbytes);
    assert (offset + nbytes <= sbx_diag_stable_size);
    sal_memcpy(sbx_diag_stable + offset, buf, nbytes);
#ifndef NO_FILEIO
    if (sbx_diag_stable_fd){
      int result;
      /* also dump to file if the descripto is valid */
      if (0 != fseek(sbx_diag_stable_fd, offset, SEEK_SET))
        return SOC_E_FAIL;
      result = fwrite(sbx_diag_stable+offset, 1, nbytes, sbx_diag_stable_fd);
      if (result != nbytes)
	return SOC_E_MEMORY;
      fflush(sbx_diag_stable_fd);
    }
#endif
    return SOC_E_NONE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

void*
sbx_diag_stable_alloc(uint32 sz)
{
    void *ptr = sal_alloc(sz, "sbx wb hdl");    
    cli_out("XCORE_ALLOC: 0x%08x of size 0x%08x\n", (int)ptr, sz);
    return ptr;
}

void
sbx_diag_stable_free(void* ptr)
{    
    cli_out("XCORE_FREE: 0x%08x\n", (int)ptr);
    sal_free(ptr);
}

char cmd_sbx_stable_usage[] =
"stable [size=n] - interact with stable for warmboot\n"
"   size - first time only sets size of stable, if 0, default size is used.\n"
"   no args displays the current state of the stable\n";

cmd_result_t
cmd_sbx_stable(int unit, args_t *args)
{
    parse_table_t pt;
    int size = -1;
#ifdef BCM_WARM_BOOT_SUPPORT
    int rv, type;
#endif

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "size", PQ_INT | PQ_DFL, 0, &size, NULL);

    if (parse_arg_eq(args, &pt) < 0) {
        cli_out("%s: Invalid argument: %s\n", ARG_CMD(args), ARG_CUR(args));
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    parse_arg_eq_done(&pt);

#ifdef BCM_WARM_BOOT_SUPPORT


    if (size < 0) {
        uint32 flags;
        rv = soc_stable_get(unit, &type, &flags);
        if (BCM_FAILURE(rv)) {
            cli_out("Failed to get stable type: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

        rv = soc_stable_size_get(unit, &size);
        if (BCM_FAILURE(rv)) {
            cli_out("Failed to get stable size: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

        cli_out("size=0x%x type=%d\n", size, type);
            
        return CMD_OK;
    }

    if (size > 0) {
        sbx_diag_stable_size = size;
    }

    if (sbx_diag_stable == NULL) {
        sbx_diag_stable = 
            sal_alloc(sbx_diag_stable_size, "warm boot storage table");
    }
    if (sbx_diag_stable == NULL) {
        return CMD_FAIL;
    }
    cli_out("allocated %d bytes for stable\n", sbx_diag_stable_size);

    type = _SHR_SWITCH_STABLE_APPLICATION;
    rv = soc_stable_set(unit, type, 0);
    if (BCM_FAILURE(rv)) {
        cli_out("Failed to set stable type: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }
        
    rv = soc_stable_size_set(unit, sbx_diag_stable_size);
    if (BCM_FAILURE(rv)) {
        cli_out("Failed to set stable size: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }
        
    rv = soc_switch_stable_register(unit, 
                                    sbx_diag_stable_read,
                                    sbx_diag_stable_write,
                                    sbx_diag_stable_alloc,
                                    sbx_diag_stable_free);

    if (BCM_FAILURE(rv)) {
      cli_out("Failed to set stable accessors: %s\n", bcm_errmsg(rv));
      return CMD_FAIL;
    }

#ifndef NO_FILEIO
    if (NULL !=  soc_property_get_str(unit, "stable_filename") && sbx_diag_stable_fd==NULL) {
      if ((sbx_diag_stable_fd = sal_fopen(soc_property_get_str(unit, "stable_filename"), SOC_WARM_BOOT(unit) ? 
                                          "r+" : "w+")) == 0) {
          cli_out("Error opening scache file: %s", soc_property_get_str(unit, "stable_filename"));
          return -1;
      }
      cli_out("Opened scache file: %s\n", soc_property_get_str(unit, "stable_filename"));
    }
#endif
    
    cli_out("Configured stable type %d\n", type);
#endif /* BCM_WARM_BOOT_SUPPORT */
        
    return CMD_OK;
}



/**************************** etest stuff ************************** */
#if 0

#define _CHECK(x) do{ \
  int rv; \
  sal_usecs_t t; \
  t = sal_time_usecs(); \
  if( ((rv=(x))) != BCM_E_NONE) {					\
    cli_out(\
            "ERROR: %s:%d: %s -> %d (%s)\n", FUNCTION_NAME(), __LINE__, #x, rv, bcm_errmsg(rv)); \
      /*return rv; */}							  \
  cli_out(\
          "%s took %u us\n", #x, SAL_USECS_SUB(sal_time_usecs(), t)); \
  }while(0)

#define TIMED_CALL(x) do{ \
  int rv; \
  sal_usecs_t t; \
  t = sal_time_usecs(); \
  if( ((rv=(x))) != BCM_E_NONE) {					\
    cli_out(\
            "ERROR: %s:%d: %s -> %d (%s)\n", FUNCTION_NAME(), __LINE__, #x, rv, bcm_errmsg(rv)); \
      /*return rv; */}							  \
  cli_out(\
          "%s took %u us\n", #x, SAL_USECS_SUB(sal_time_usecs(), t)); \
  }while(0)


/* pb2_trunk
   Two customer ports, which can be trunked. Two provider ports which can 
   be trunked. LP Splitting feature as well */

#define _PB2_TRUNK_C0 0
#define _PB2_TRUNK_C1 1
#define _PB2_TRUNK_P0 2
#define _PB2_TRUNK_P1 3
#define _PB2_TRUNK_PORTS 4

#define _PB2_POLICER_GROUPS 10

cmd_result_t
cmd_sbx_etest_pb2_trunk(int unit, int test){
  int retval = 0;
  int fe=1, qe=0;
  int fe_modid = 0;
  int qe_modid = 0 + BCM_MODULE_FABRIC_BASE;
  int port[_PB2_TRUNK_PORTS] = {0,1,2,3};
  int i;
  int cvid = 200;
  int svid = 100;
  bcm_vlan_t vsi;
  bcm_gport_t gport[_PB2_TRUNK_PORTS];
  bcm_gport_t qgport[_PB2_TRUNK_PORTS];
  bcm_gport_t vlan_gport[_PB2_TRUNK_PORTS];
  bcm_mac_t mac0, mac1, mac2, mac3, mac_empty;
  bcm_vlan_port_t vlan_port;
  bcm_multicast_t multicast_group;
  int encap_id[_PB2_TRUNK_PORTS];
  int mode = test >> 4;
  bcm_qos_map_t qos_map;
  int map_id;
  uint32 flags;


  bcm_trunk_t trunk_id=0;
  bcm_trunk_add_info_t trunk_add_info;
  bcm_gport_t trunk_gport;

  bcm_l2_addr_t l2_addr;

  bcm_policer_t policer_id[_PB2_POLICER_GROUPS];
  int num_policers;
  bcm_policer_config_t policer_config;
  int policer_group;

  test = test & 0xf;

  fromAsciiMacAddress("00:00:01:00:01:00", &mac0);
  fromAsciiMacAddress("00:00:01:00:02:00", &mac1);
  fromAsciiMacAddress("00:00:01:00:03:00", &mac2);
  fromAsciiMacAddress("00:00:01:00:04:00", &mac3);
  fromAsciiMacAddress("00:00:00:00:00:00", &mac_empty);

  if (mode==2){ /* AB501 070910 remote gports */
    fe_modid=28;
    qe_modid=10028;
  }
    
  for (i=0;i<_PB2_TRUNK_PORTS;i++){
    BCM_GPORT_MODPORT_SET(gport[i], fe_modid, port[i]);
    BCM_GPORT_MODPORT_SET(qgport[i], qe_modid, port[i]);
  }

  /* service vlan */
#if 0
  _CHECK(bcm_vswitch_create(fe, &vsi));
#else
  vsi = svid;
  _CHECK(bcm_vlan_create(fe, vsi));
#endif
  cli_out("vswitch created vsi=0x%x\n", vsi);
  cli_out("Creating service vlan vid=0x%x\n", svid);

  multicast_group = vsi;
  _CHECK(bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID, 
			      &multicast_group));

  for (i=0;i<_PB2_TRUNK_PORTS; i++){
    bcm_vlan_port_t_init(&vlan_port);
    if (i<_PB2_TRUNK_P0){
      /* AB501 101209 Customer ports */
      _CHECK(bcm_port_dtag_mode_set(fe, port[i], BCM_PORT_DTAG_MODE_EXTERNAL));
      vlan_port.flags = BCM_VLAN_PORT_INNER_VLAN_PRESERVE | 
	BCM_VLAN_PORT_INNER_VLAN_ADD;
      if (mode==1){
	vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT;
      }else{
	vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
	vlan_port.match_vlan = cvid;
	vlan_port.egress_vlan = cvid;
      }
      vlan_port.port = gport[i];
      _CHECK(bcm_vlan_port_create(fe, &vlan_port));
      vlan_gport[i] = vlan_port.vlan_port_id;
      _CHECK(bcm_vswitch_port_add(fe, vsi, vlan_port.vlan_port_id));
      encap_id[i] = vlan_port.encap_id;
      if (mode!=2)
	_CHECK(bcm_multicast_egress_add(qe, vsi, qgport[i], encap_id[i])); 
      cli_out("Created customer port gport=0x%x encap_id=0x%x\n", 
              vlan_port.vlan_port_id, encap_id[i]);
    }else{
      _CHECK(bcm_port_dtag_mode_set(fe, port[i], BCM_PORT_DTAG_MODE_INTERNAL));
      vlan_port.flags = 0;
      vlan_port.port = gport[i];
      vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
      vlan_port.match_vlan = svid;
      vlan_port.egress_vlan = svid;
      _CHECK(bcm_vlan_port_create(fe, &vlan_port));
      vlan_gport[i] = vlan_port.vlan_port_id;
      _CHECK(bcm_vswitch_port_add(fe, vsi, vlan_port.vlan_port_id));
      if (mode!=2){
	_CHECK(bcm_multicast_l2_encap_get(fe, vsi, vlan_port.vlan_port_id, vsi, &encap_id[i]));
	_CHECK(bcm_multicast_egress_add(qe, vsi, qgport[i], vlan_port.encap_id)); 
      }
      cli_out("Created Provider gport=0x%x encap_id=0x%x (0x%x)\n", vlan_port.vlan_port_id,
              vlan_port.encap_id, encap_id[i]);
    }
  }

  if (mode!=2){
    cli_out("Associating policers with All ports\n");
    for (policer_group=0; policer_group<_PB2_POLICER_GROUPS; policer_group++){
      _CHECK(bcm_policer_group_create(fe, bcmPolicerGroupModeTypedAll, 
				      &policer_id[policer_group], &num_policers));
      cli_out("Created policer group %d with id=0x%x\n", policer_group, policer_id[policer_group]);
      /* Policer configs */
      for (i=0;i<5;i++){
	bcm_policer_config_t_init(&policer_config);
	policer_config.flags = BCM_POLICER_DROP_RED | 
	  BCM_POLICER_WITH_ID | BCM_POLICER_COLOR_BLIND;
	policer_config.mode = bcmPolicerModeSrTcm;
	policer_config.ckbits_sec = 20000;
	policer_config.pkbits_sec = 0;
	policer_config.ckbits_burst = 256;
	policer_config.pkbits_burst = 256;
	_CHECK(bcm_policer_set(fe, policer_id[policer_group]+i, &policer_config));
	/* cli_out("Set policer id 0x%x - flags=0x%x\n", policer_id[policer_group]+i, policer_config.flags); */
      }
      if (policer_group < _PB2_TRUNK_PORTS){
	_CHECK(bcm_port_policer_set(fe, vlan_gport[policer_group], 
				    policer_id[policer_group]));
      }
    }

    cli_out("Setting default vlan on Customer ports to %d\n", cvid);
    for(i=0;i<_PB2_TRUNK_P0;i++){
      _CHECK(bcm_port_untagged_vlan_set(fe, port[_PB2_TRUNK_C0], cvid));
      _CHECK(bcm_port_untagged_vlan_set(fe, port[_PB2_TRUNK_C1], cvid));
    }

    flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2 ;
    _CHECK(bcm_qos_map_create(fe, flags, &map_id));
    cli_out("Setting QOS egress remap: map_id=0x%x for C1\n", map_id);
    bcm_qos_map_t_init(&qos_map);
    for (i=0;i<8;i++){
      flags = BCM_QOS_MAP_L2;
      qos_map.remark_int_pri=i;
      qos_map.pkt_pri=2;
      _CHECK(bcm_qos_map_add(fe, flags, &qos_map, map_id));
    }
    _CHECK(bcm_qos_port_map_set(fe, vlan_gport[_PB2_TRUNK_C1], -1, map_id));

    flags = BCM_QOS_MAP_EGRESS | BCM_QOS_MAP_L2 ;
    _CHECK(bcm_qos_map_create(fe, flags, &map_id));
    cli_out("Setting QOS egress remap: map_id=0x%x for P0\n", map_id);
    bcm_qos_map_t_init(&qos_map);
    for (i=0;i<8;i++){
      flags = BCM_QOS_MAP_L2;
      qos_map.remark_int_pri=i;
      qos_map.pkt_pri=2;
      _CHECK(bcm_qos_map_add(fe, flags, &qos_map, map_id));
    }
    _CHECK(bcm_qos_port_map_set(fe, vlan_gport[_PB2_TRUNK_P0], -1, map_id));

    flags = BCM_QOS_MAP_INGRESS | BCM_QOS_MAP_L2 ;
    _CHECK(bcm_qos_map_create(fe, flags, &map_id));
    cli_out("Setting QOS ingress remap: map_id=0x%x for C0\n", map_id);
    bcm_qos_map_t_init(&qos_map);
    for (i=0;i<8;i++){
      flags = BCM_QOS_MAP_L2;
      qos_map.pkt_pri=i;
      qos_map.int_pri=5;
      qos_map.remark_int_pri=6;
      _CHECK(bcm_qos_map_add(fe, flags, &qos_map, map_id));
    }
    _CHECK(bcm_qos_port_map_set(fe, vlan_gport[_PB2_TRUNK_C0], map_id, -1));


  }

  /* AB501 101209 Add some unicast routes to the customer ports */
  cli_out("Adding L2 Address mac0 to C0\n");
  bcm_l2_addr_t_init(&l2_addr, mac0, vsi);
  l2_addr.modid = 0;
  l2_addr.port = vlan_gport[_PB2_TRUNK_C0];
  _CHECK(bcm_l2_addr_add(fe, &l2_addr));

  cli_out("Adding L2 Address mac1 to C1\n");
  bcm_l2_addr_t_init(&l2_addr, mac1, vsi);
  l2_addr.modid = 0;
  l2_addr.port = vlan_gport[_PB2_TRUNK_C1];
  _CHECK(bcm_l2_addr_add(fe, &l2_addr));

  cli_out("Adding L2 Address mac3 to P0\n");
  bcm_l2_addr_t_init(&l2_addr, mac2, vsi);
  l2_addr.modid = 0;
  l2_addr.port = vlan_gport[_PB2_TRUNK_P0];
  _CHECK(bcm_l2_addr_add(fe, &l2_addr));



  if (test==2 || test==3 || test==4 || test==6 || test==7){
    /* AB501 101209 TRUNK Customer Ports */
    cli_out("Adding trunk to existing C0 and C1 ports\n");
    cli_out("Delete MAC addresses \n");

    bcm_l2_addr_t_init(&l2_addr, mac0, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_C0];
    _CHECK(bcm_l2_addr_delete(fe, mac0, vsi));

    bcm_l2_addr_t_init(&l2_addr, mac1, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_C1];
    _CHECK(bcm_l2_addr_delete(fe, mac1, vsi));

    cli_out("Delete the gports\n");
    for (i=0; i< _PB2_TRUNK_P0 ;i++){
      _CHECK(bcm_qos_port_map_set(fe, vlan_gport[i], -1, 0));
      _CHECK(bcm_port_policer_set(fe, vlan_gport[i],     0));
      /* AB501 052510 delete QE */
      _CHECK(bcm_multicast_egress_delete(qe, vsi, qgport[i], encap_id[i]));
      _CHECK(bcm_vswitch_port_delete(fe, vsi, vlan_gport[i]));
      _CHECK(bcm_vlan_port_destroy(fe, vlan_gport[i]));
    }

    cli_out("Create trunk\n");
    /* AB501 082409 Trunk the Provider port  */
    /* trunk create */
    _CHECK(bcm_trunk_create_id(fe, trunk_id));
    _CHECK(bcm_trunk_create_id(qe, trunk_id));
    BCM_GPORT_TRUNK_SET(trunk_gport, trunk_id);


    bcm_trunk_add_info_t_init(&trunk_add_info);
    trunk_add_info.flags = 0;
    trunk_add_info.num_ports = 2;
    trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
    trunk_add_info.tp[0] = port[_PB2_TRUNK_C0];
    trunk_add_info.tm[0] = fe_modid;
    trunk_add_info.tp[1] = port[_PB2_TRUNK_C1];
    trunk_add_info.tm[1] = fe_modid;
    _CHECK(bcm_trunk_set(fe, trunk_id, &trunk_add_info));
    trunk_add_info.tm[0] = qe_modid;
    trunk_add_info.tm[1] = qe_modid;
    _CHECK(bcm_trunk_set(qe, trunk_id, &trunk_add_info));
    
    
    /* AB501 101209 Must now re-add
     */
    for (i=0;i<=_PB2_TRUNK_C0; i++){
      _CHECK(bcm_port_dtag_mode_set(fe, port[i], BCM_PORT_DTAG_MODE_EXTERNAL));
      bcm_vlan_port_t_init(&vlan_port);
      /* AB501 101209 Customer ports */
      vlan_port.flags = BCM_VLAN_PORT_INNER_VLAN_PRESERVE | 
	BCM_VLAN_PORT_INNER_VLAN_ADD;
      if(mode==1){
	vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT;
      }else{
	vlan_port.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
	vlan_port.match_vlan = cvid;
	vlan_port.egress_vlan = cvid;
      }
      vlan_port.port = trunk_gport;
      _CHECK(bcm_vlan_port_create(fe, &vlan_port));
      vlan_gport[i] = vlan_port.vlan_port_id;
      _CHECK(bcm_vswitch_port_add(fe, vsi, vlan_port.vlan_port_id));
      encap_id[i] = vlan_port.encap_id;
      cli_out("Created customer port gport=0x%x encap_id=0x%x\n", 
              vlan_port.vlan_port_id, encap_id[i]);

      _CHECK(bcm_port_policer_set(fe, vlan_gport[i], 
				  policer_id[i]));
    }

    for (i=0;i<_PB2_TRUNK_C1; i++){
      /* AB501 052510 must call egress add on all trunk members. MVT for
	 designate port should be the only one that's enabled */
      _CHECK(bcm_multicast_egress_add(qe, vsi, qgport[i], encap_id[_PB2_TRUNK_C0])); 
    }

    /* readd MAC addresses */
    cli_out("Adding L2 Address mac0 and mac1 to Customer trunk\n");
    bcm_l2_addr_t_init(&l2_addr, mac0, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_C0];
    _CHECK(bcm_l2_addr_add(fe, &l2_addr));
    bcm_l2_addr_t_init(&l2_addr, mac1, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_C0];
    _CHECK(bcm_l2_addr_add(fe, &l2_addr));

    /* Change CVID here for operations below */
    /*
    if (mode==1)
      cvid=0;
    */
  } /* trunking */

  if (test==5){
    /* Removal of the PB ports */
    cli_out("Remove all ports\n");

    cli_out("Delete MAC addresses \n");
    bcm_l2_addr_t_init(&l2_addr, mac0, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_C0];
    _CHECK(bcm_l2_addr_delete(fe, mac0, vsi));

    bcm_l2_addr_t_init(&l2_addr, mac1, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_C1];
    _CHECK(bcm_l2_addr_delete(fe, mac1, vsi));

    bcm_l2_addr_t_init(&l2_addr, mac2, vsi);
    l2_addr.modid = 0;
    l2_addr.port = vlan_gport[_PB2_TRUNK_P0];
    _CHECK(bcm_l2_addr_delete(fe, mac2, vsi));

    cli_out("Delete the gports\n");
    for (i=0; i< _PB2_TRUNK_PORTS ;i++){
      if (mode!=2){
	_CHECK(bcm_port_policer_set(fe, vlan_gport[i],     0));
      }
      /* AB501 052510 delete QE */
      _CHECK(bcm_multicast_egress_delete(qe, vsi, qgport[i], encap_id[i]));
      _CHECK(bcm_vswitch_port_delete(fe, vsi, vlan_gport[i]));
      _CHECK(bcm_vlan_port_destroy(fe, vlan_gport[i]));
    }


  }

  if (test == 6) {
    bcm_vlan_vector_t vlan_vector;
    int v;

    cli_out("test vector_set\n");

    BCM_VLAN_VEC_ZERO(vlan_vector);
      for (v=0x3c8; v<4095; v++) {
      BCM_VLAN_VEC_SET(vlan_vector, v);
    }
    _CHECK(bcm_port_vlan_vector_set(fe, vlan_port.vlan_port_id, vlan_vector));
  }

  if (test == 7) {
    bcm_trunk_add_info_t_init(&trunk_add_info);
    trunk_add_info.flags = 0;
    trunk_add_info.num_ports = 1;
    trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
    trunk_add_info.tp[0] = port[_PB2_TRUNK_C0];
    trunk_add_info.tm[0] = fe_modid;
    trunk_add_info.tp[1] = port[_PB2_TRUNK_C1];
    trunk_add_info.tm[1] = fe_modid;
    _CHECK(bcm_trunk_set(fe, trunk_id, &trunk_add_info));
    trunk_add_info.tm[0] = qe_modid;
    trunk_add_info.tm[1] = qe_modid;
    _CHECK(bcm_trunk_set(qe, trunk_id, &trunk_add_info));
  }
  
  return retval;
    
}

/*
  use a trunk as CEP

  port_a - trunk member port 1
  port_b - trunk member port 2
*/
void test_372291(int svlan, int cvlan, int port_a, int port_b)
{
  int fe = 1;
  int qe = 0;
  int femod, qemod;
  int tgid = 0;
  bcm_gport_t trunk_gport;
  bcm_vlan_port_t vlanport;
  bcm_multicast_t mcgroup;
  bcm_trunk_add_info_t trunk_add_info;

  bcm_stk_modid_get(fe, &femod);
  bcm_stk_modid_get(qe, &qemod);

  /* create service vlan */
  _CHECK(bcm_vlan_create(fe, svlan));

  /* create a trunk */
  _CHECK(bcm_trunk_create(fe, &tgid));

  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = 0;
  trunk_add_info.num_ports = 2;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  trunk_add_info.tp[0] = port_a;
  trunk_add_info.tm[0] = femod;
  trunk_add_info.tp[0] = port_b;
  trunk_add_info.tm[0] = femod;
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));
  BCM_GPORT_TRUNK_SET(trunk_gport, tgid);

  /* the trunk is used as CEP. set dtag mode for all members */
  _CHECK(bcm_port_dtag_mode_set(fe, port_a, BCM_PORT_DTAG_MODE_EXTERNAL));
  _CHECK(bcm_port_dtag_mode_set(fe, port_b, BCM_PORT_DTAG_MODE_EXTERNAL));
  
  /* create vlan gport on the trunk */
  bcm_vlan_port_t_init(&vlanport);
  vlanport.flags = BCM_VLAN_PORT_INNER_VLAN_PRESERVE;
  vlanport.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
  vlanport.match_vlan = cvlan;
  vlanport.port = trunk_gport;
  vlanport.vlan_port_id = 0;
  vlanport.egress_vlan = cvlan;
  _CHECK(bcm_vlan_port_create(fe, &vlanport));
  printf("vlan gport 0x%08x created\n", vlanport.vlan_port_id);


  /* add trunk into the service vlan as a CEP*/
  _CHECK(bcm_vswitch_port_add(fe, svlan, vlanport.vlan_port_id));


  /* map cvlan */
  {
    bcm_vlan_vector_t	vector;
    BCM_VLAN_VEC_ZERO(vector);
    BCM_VLAN_VEC_SET(vector, cvlan);

    _CHECK(bcm_port_vlan_vector_set(fe, vlanport.vlan_port_id, vector));
  }

  /* untagged - why to vid 1? */
  _CHECK(bcm_port_untagged_vlan_set(fe, port_a, 1));
  _CHECK(bcm_port_untagged_vlan_set(fe, port_b, 1));

  /* create flooding */
  mcgroup = svlan;
  _CHECK(bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID, &mcgroup));
  
  /* put one member port in mcgroup as the designated port */
  {
    bcm_gport_t gport;
    int encap_id;
    BCM_GPORT_MODPORT_SET(gport, qemod, port_a);
    _CHECK(bcm_multicast_l2_encap_get(fe, svlan, vlanport.vlan_port_id, svlan, &encap_id));
    _CHECK(bcm_multicast_egress_add(qe, mcgroup, gport, encap_id)); 
  }

  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = 0;
  trunk_add_info.num_ports = 1;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  trunk_add_info.tp[0] = port_b;
  trunk_add_info.tm[0] = femod;
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));

}

/*
  bcm_port_vlan_vector_set takes too long
*/
void test_372348(int svlan, int cvlan)
{
  int fe = 1;
  int qe = 0;
  int femod, qemod;
  int tgid = 0;
  bcm_gport_t trunk_gport;
  bcm_vlan_port_t vlanport;
  bcm_multicast_t mcgroup;
  bcm_trunk_add_info_t trunk_add_info;
  int lag_size = 4;
  int lag_members[] = { 0, 1, 2, 3, 5, 6, 7, 8, 9 };
  int i;

  bcm_stk_modid_get(fe, &femod);
  bcm_stk_modid_get(qe, &qemod);

  /* create service vlan */
  _CHECK(bcm_vlan_create(fe, svlan));

  /* create a trunk */
  _CHECK(bcm_trunk_create(fe, &tgid));

  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = 0;
  trunk_add_info.num_ports = lag_size;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  for (i=0; i<sizeof (lag_members)/sizeof(int); i++) {
    trunk_add_info.tp[i] = lag_members[i];
    trunk_add_info.tm[i] = femod;
  }
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));
  BCM_GPORT_TRUNK_SET(trunk_gport, tgid);

  /* the trunk is used as CEP. set dtag mode for all members */
  for (i=0; i<sizeof (lag_members)/sizeof(int); i++) {
    _CHECK(bcm_port_dtag_mode_set(fe, lag_members[i], BCM_PORT_DTAG_MODE_EXTERNAL));
  }
  
  /* create vlan gport on the trunk */
  bcm_vlan_port_t_init(&vlanport);
  vlanport.flags = BCM_VLAN_PORT_INNER_VLAN_PRESERVE;
#if 1
  vlanport.criteria = BCM_VLAN_PORT_MATCH_PORT;
#else
  vlanport.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
  vlanport.match_vlan = cvlan;
#endif
  vlanport.port = trunk_gport;
  vlanport.vlan_port_id = 0;
  vlanport.egress_vlan = cvlan;
  _CHECK(bcm_vlan_port_create(fe, &vlanport));
  printf("vlan gport 0x%08x created\n", vlanport.vlan_port_id);


  /* add trunk into the service vlan as a CEP*/
  _CHECK(bcm_vswitch_port_add(fe, svlan, vlanport.vlan_port_id));


  _CHECK(bcm_port_untagged_vlan_set(fe, vlanport.vlan_port_id, 4095));

  /* create flooding */
  mcgroup = svlan;
  _CHECK(bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID, &mcgroup));
  
  /* put one member port in mcgroup as the designated port */
  {
    bcm_gport_t gport;
    int encap_id;
    BCM_GPORT_MODPORT_SET(gport, qemod, lag_members[0]);
    _CHECK(bcm_multicast_l2_encap_get(fe, svlan, vlanport.vlan_port_id, svlan, &encap_id));
    _CHECK(bcm_multicast_egress_add(qe, mcgroup, gport, encap_id)); 
  }

  /* map cvlan */
  {
    bcm_vlan_vector_t	vector;
    int v;
    BCM_VLAN_VEC_ZERO(vector);
#if 0
    BCM_VLAN_VEC_SET(vector, cvlan);
#else
    for (v=1; v<4096; v++) {
#endif
      BCM_VLAN_VEC_SET(vector, v);
    }
      

    _CHECK(bcm_port_vlan_vector_set(fe, vlanport.vlan_port_id, vector));
  }

}

/*
  bcm_trunk_set takes long
*/

int test_379291()
{
  int fe = 1;
  int qe = 0;
  int femod, qemod;
  int tgid = 0;
  bcm_gport_t trunk_gport;
  bcm_vlan_port_t vlanport;
  bcm_multicast_t mcgroup;
  bcm_trunk_add_info_t trunk_add_info;
  int member_ports[][2] = {
    {0, 0 },
    {0, 1 },
    {0, 2 },
    {0, 3 },
    {0, 4 },
    {0, 5 },
    {1, 0 },
    {1, 1 },
    {1, 2 },
  };
  int i;
  int svlan;
  int cvlan;
#define NUM_VLAN_GPORT  2
  static bcm_gport_t vlan_port_ids[NUM_VLAN_GPORT];

  bcm_stk_modid_get(fe, &femod);
  bcm_stk_modid_get(qe, &qemod);

  _CHECK(bcm_trunk_create(fe, &tgid));

  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = 0;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  trunk_add_info.num_ports = 3;
  for (i = 0; i<trunk_add_info.num_ports; i++) {
    trunk_add_info.tm[i] = member_ports[i][0] + femod;
    trunk_add_info.tp[i] = member_ports[i][1];
   _CHECK(bcm_port_dtag_mode_set(fe, member_ports[i][1], BCM_PORT_DTAG_MODE_EXTERNAL));
  }
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));

  BCM_GPORT_TRUNK_SET(trunk_gport, tgid);
  for(i=0; i<NUM_VLAN_GPORT; i++) {
    svlan = 100+i;
    cvlan = 1000+i;
    printf("Creating SVALN %d\n", svlan);
    _CHECK(bcm_vlan_create(fe, svlan));
    
    bcm_vlan_port_t_init(&vlanport);
    vlanport.flags = BCM_VLAN_PORT_INNER_VLAN_PRESERVE;
    vlanport.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vlanport.match_vlan = cvlan;
#if 1
    vlanport.port = trunk_gport;
#else
    BCM_GPORT_MODPORT_SET(vlanport.port, femod, 12);
#endif
    vlanport.vlan_port_id = 0;
    vlanport.egress_vlan = cvlan;
    _CHECK(bcm_vlan_port_create(fe, &vlanport));
    vlan_port_ids[i] = vlanport.vlan_port_id;
    printf("vlan gport 0x%08x created\n", vlanport.vlan_port_id);
    
    _CHECK(bcm_vswitch_port_add(fe, svlan, vlanport.vlan_port_id));
    
    {
      bcm_vlan_vector_t	vector;
      int v;
      BCM_VLAN_VEC_ZERO(vector);
#if 0
    BCM_VLAN_VEC_SET(vector, cvlan);
#else
    for (v=1; v<4096; v++)
#endif
      BCM_VLAN_VEC_SET(vector, cvlan);
      
    /*      _CHECK(bcm_port_vlan_vector_set(fe, vlanport.vlan_port_id, vector));*/
    }
    mcgroup = svlan;
    _CHECK(bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID, &mcgroup));
    
#if 0
    {
      bcm_gport_t gport;
      int encap_id;
      int designate_port = 0;
      BCM_GPORT_MODPORT_SET(gport, qemod, designate_port);
      _CHECK(bcm_multicast_l2_encap_get(fe, svlan, vlanport.vlan_port_id, svlan, &encap_id));
      _CHECK(bcm_multicast_egress_add(qe, mcgroup, gport, encap_id)); 
    }
#endif

#if 1 
  { /* 393999: test bcm_port_vlan_priority_mapping_set */
    bcm_priority_mapping_t primap;
    
    sal_memset(&primap, 0, sizeof(primap));
    primap.internal_pri = 0;
    primap.remark_internal_pri = 0;
    primap.remark_color = 0;
    primap.color = 0;
    primap.policer_offset = 0;
    _CHECK(bcm_port_vlan_priority_mapping_set(fe, vlanport.vlan_port_id, cvlan, 0, 0, &primap));
  }
#endif

#if 0
  { /* 396789: test STP set */
    _CHECK(bcm_vlan_stp_set(fe, cvlan, vlanport.vlan_port_id, 0));
  }
#endif


  }




#if 1 /* delete a member port */
  /* update trunk without updating each vlan gport */
  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = BCM_TRUNK_FLAG_VLAN_SKIP_CHANGE_HANDLER;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  trunk_add_info.num_ports = 2;
  for (i = 0; i<trunk_add_info.num_ports; i++) {
    trunk_add_info.tm[i] = member_ports[i][0] + femod;
    trunk_add_info.tp[i] = member_ports[i][1];
   _CHECK(bcm_port_dtag_mode_set(fe, member_ports[i][1], BCM_PORT_DTAG_MODE_EXTERNAL));
  }
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));


  /* update vlan gports one by one */
  for (i=0; i<NUM_VLAN_GPORT; i++) {
    cli_out("\nupdating VLAN gport 0x%08x\n", vlan_port_ids[i]);
    trunk_add_info.flags = BCM_TRUNK_FLAG_PROCESS_ONE_GPORT;
    trunk_add_info.psc = vlan_port_ids[i];
    TIMED_CALL(bcm_trunk_set(fe, tgid, &trunk_add_info));
  }
  
  /* commit the change - we use BCM_TRUNK_FLAG_PROCESS_ONE_GPORT and trunk_add_info.psc == 0 to signal this*/
  /*
  trunk_add_info.psc = 0;
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));
  */
#endif

#if 0 /* add the member port back */
  /* update trunk without updating each vlan gport */
  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = BCM_TRUNK_FLAG_VLAN_SKIP_CHANGE_HANDLER;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  trunk_add_info.num_ports = 3;
  for (i = 0; i<trunk_add_info.num_ports; i++) {
    trunk_add_info.tm[i] = member_ports[i][0] + femod;
    trunk_add_info.tp[i] = member_ports[i][1];
   _CHECK(bcm_port_dtag_mode_set(fe, member_ports[i][1], BCM_PORT_DTAG_MODE_EXTERNAL));
  }
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));


  /* update vlan gports one by one */
  for (i=0; i<NUM_VLAN_GPORT; i++) {
    cli_out("\nupdating VLAN gport 0x%08x\n", vlan_port_ids[i]);
    trunk_add_info.flags = BCM_TRUNK_FLAG_PROCESS_ONE_GPORT;
    trunk_add_info.psc = vlan_port_ids[i];
    _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));
  }
  
  /* commit the change - we use BCM_TRUNK_FLAG_PROCESS_ONE_GPORT and trunk_add_info.psc == 0 to signal this*/
  /*
  trunk_add_info.psc = 0;
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));
  */
#endif

#if 0
  for(i=0; i<NUM_VLAN_GPORT; i++) {
    svlan = 100+i;
    cvlan = 1000+i;
    _CHECK(bcm_multicast_destroy(qe, svlan));
    _CHECK(bcm_vswitch_port_delete(fe, svlan, vlan_port_ids[i]));
    _CHECK(bcm_vlan_port_destroy(fe, vlan_port_ids[i]));
    _CHECK(bcm_vlan_destroy(fe, svlan));
  }
  bcm_trunk_add_info_t_init(&trunk_add_info);
  trunk_add_info.flags = 0;
  trunk_add_info.psc = BCM_TRUNK_PSC_SRCMAC;
  trunk_add_info.num_ports = 0;
  _CHECK(bcm_trunk_set(fe, tgid, &trunk_add_info));
  _CHECK(bcm_trunk_destroy(fe, tgid));
#endif

  return 0;
}

void test_379291_a()
{
  int fe = 1;
  int qe = 0;
  int femod, qemod;
  /*  bcm_multicast_t mcgroup;*/
  int i;
  int svlan;
  int cvlan;
#undef NUM_VLAN_GPORT
#define NUM_VLAN_GPORT  3
  bcm_gport_t vlan_port_ids[NUM_VLAN_GPORT];
  bcm_vlan_port_t vlanport;

  bcm_stk_modid_get(fe, &femod);
  bcm_stk_modid_get(qe, &qemod);

  for(i=0; i<NUM_VLAN_GPORT; i++) {
    svlan = 100+i;
    cvlan = 1000+i;
    printf("Creating SVALN %d\n", svlan);
    _CHECK(bcm_vlan_create(fe, svlan));
    bcm_vlan_port_t_init(&vlanport);
    vlanport.flags = BCM_VLAN_PORT_INNER_VLAN_PRESERVE;
    vlanport.criteria = BCM_VLAN_PORT_MATCH_PORT_VLAN;
    vlanport.match_vlan = cvlan;
    BCM_GPORT_MODPORT_SET(vlanport.port, femod, 12);
    vlanport.vlan_port_id = 0;
    vlanport.egress_vlan = cvlan;
    _CHECK(bcm_vlan_port_create(fe, &vlanport));
    vlan_port_ids[i] = vlanport.vlan_port_id;
    printf("vlan gport 0x%08x created\n", vlanport.vlan_port_id);
    
#if 1    
    _CHECK(bcm_vswitch_port_add(fe, svlan, vlanport.vlan_port_id));
#endif    
    {
      bcm_vlan_vector_t	vector;
      int v;
      BCM_VLAN_VEC_ZERO(vector);
#if 0
    BCM_VLAN_VEC_SET(vector, cvlan);
#else
    for (v=1; v<4096; v++)
#endif
      BCM_VLAN_VEC_SET(vector, cvlan);
      
    /*      _CHECK(bcm_port_vlan_vector_set(fe, vlanport.vlan_port_id, vector));*/
    }
#if 0
    mcgroup = svlan;
    _CHECK(bcm_multicast_create(qe, BCM_MULTICAST_TYPE_L2 | BCM_MULTICAST_WITH_ID, &mcgroup));
    
    {
      bcm_gport_t gport;
      int encap_id;
      int designate_port = 0;
      BCM_GPORT_MODPORT_SET(gport, qemod, designate_port);
      _CHECK(bcm_multicast_l2_encap_get(fe, svlan, vlanport.vlan_port_id, svlan, &encap_id));
      _CHECK(bcm_multicast_egress_add(qe, mcgroup, gport, encap_id)); 
    }
#endif
  }

  for(i=0; i<NUM_VLAN_GPORT; i++) {
    svlan = 100+i;
    cvlan = 1000+i;
    /* _CHECK(bcm_multicast_destroy(qe, svlan));*/
#if 1
    _CHECK(bcm_vswitch_port_delete(fe, svlan, vlan_port_ids[i]));
#endif
    _CHECK(bcm_vlan_port_destroy(fe, vlan_port_ids[i]));
    _CHECK(bcm_vlan_destroy(fe, svlan));
  }
}

char cmd_sbx_etest_usage[] =
"Usage:\n"
"mctest \n"
"  testing for specific traffic scenarios\n"
"  Current tests:\n"
"    etest pb_trunkcp         : Customer port trunk with port becoming provider port\n"
"    etest pb2_trunk          : old test for reference - cust and provider pbb\n"
"    etest pb2_trunk2         : old test for reference\n"
"    etest vector_set         : testing bcm_vlan_vector_set\n"
"    etest remove_member      : remove a member port\n"
"    etest trunk_set          : trunk_set speed measurement\n"
  ;

cmd_result_t
cmd_sbx_etest(int unit, args_t *a)
{
  int retval = 0;
  char *tmp,*cmd;
#if 00
  uint32 address, data, table;
#endif

  cli_out("N testing\n");

  /* Test scenarios:
     mctune test 1
     1. Removes all ports from vlan 2
     2. Adds ports 0 and 1 to vlan 2
     3. set up policer group 'typed' for ingress port 0
  */
  if (ARG_CNT(a)==0 || sal_strcasecmp(_ARG_CUR(a), "help")==0){
    cli_out("N Testing help:\n");
    return 0;
  }
  cmd = ARG_GET(a);
  tmp = cmd;
  if (strcmp(tmp, "pb_trunkcp")==0){
    cli_out("E Test \n");
#if 00
    cmd_sbx_etest_pb_trunkcp(unit, 1);
#endif
  }else if (strcmp(tmp, "pb2_trunk")==0){
    cli_out("E pb2_trunk \n");
    cmd_sbx_etest_pb2_trunk(unit, 1);
  }else if (strcmp(tmp, "pb2_trunk2")==0){
    cli_out("E pb2_trunk \n");
    cmd_sbx_etest_pb2_trunk(unit, 2);
  }else if (strcmp(tmp, "vector_set")==0){
    test_372348(100, 1);
  }else if (strcmp(tmp, "remove_member")==0){
    test_372291(100, 200, 0, 9);
  }else if (strcmp(tmp, "trunk_set")==0){
    int i=0;
    for (i=0; i<1; i++) {
      cli_out("\nTest run %d\n", i);
      TIMED_CALL( test_379291(100, 200, 0, 9) );
    }
  }else {
    cli_out("Unknown test: %s\n", tmp);
  }
  return retval;
}

#endif
