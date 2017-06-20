/*
 * $Id: jer2_arad_drv.c, Modified Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#ifdef _ERR_MSG_MODULE_NAME 
    #error "_ERR_MSG_MODULE_NAME redefined" 
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT

/*includes*/
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <soc/ipoll.h>
#include <soc/linkctrl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnxc/legacy/fabric.h>
#include <soc/dnxc/legacy/dnxc_dev_feature_manager.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/error.h>
#include <soc/dnx/legacy/fabric.h>
#include <soc/dnx/legacy/port_map.h>
#include <soc/dnx/legacy/ARAD/arad_drv.h>
#include <soc/dnx/legacy/ARAD/arad_stat.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_device.h>
#include <soc/dnx/legacy/ARAD/arad_init.h>
#include <soc/dnx/legacy/ARAD/arad_link.h>
#include <soc/dnx/legacy/ARAD/arad_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/ps_db.h>
#include <soc/dnx/legacy/ARAD/arad_chip_regs.h>

#include <soc/dnx/legacy/JER/jer_egr_queuing.h>
#include <soc/dnx/legacy/JER/jer_sbusdma_desc.h>

#include <soc/dnx/legacy/mbcm.h>
#include <soc/dnxc/legacy/dnxc_cmic.h>

#ifdef CRASH_RECOVERY_SUPPORT
#include <soc/hwstate/hw_log.h>
#endif /* CRASH_RECOVERY_SUPPORT */

#include <soc/dnxc/legacy/dnxc_mem.h>


#include <shared/shr_resmgr.h>
#include <shared/shr_occupation.h>


/* extern functions */
extern int
bcm_dnx_am_cosq_scheduler_allocate(int unit,
                                       int core,
                                       uint32 nof_remote_cores,
                                       uint32 flags,
                                       int is_composite,
                                       int is_enhanced,
                                       int is_dual,
                                       int is_non_contiguous,
                                       int num_cos,
                                       DNX_TMC_AM_SCH_FLOW_TYPE flow_type,
                                       uint8* src_modid,
                                       int *flow_id);

extern int
bcm_dnx_am_cosq_scheduler_deallocate(int unit,
                                       int core,
                                       uint32 flags,
                                       int is_composite,
                                       int is_enhanced,
                                       int is_dual,
                                       int is_non_contiguous,
                                       int num_cos,
                                       DNX_TMC_AM_SCH_FLOW_TYPE flow_type,
                                       int flow_id);

/* end of extern functions */

/* JER2_ARAD DRV defines */
#define SOC_DNX_JER2_ARAD_DEFAULT_TDM_SOURCE_FAP_ID_OFFSET       (256)
#define SOC_DNX_FIRST_SFI_PHY_PORT(unit)                    SOC_DNX_DEFS_GET(unit, first_sfi_phy_port)
#define SOC_DNX_JER2_ARAD_NUM_CPU_COSQ                           (64) 

/* Interrupts defines */
#define SOC_DNX_JER2_ARAD_INTERRUPTS_CMIC_REGISTER_2_MASK 0x1e
#define SOC_DNX_JER2_ARAD_INTERRUPTS_CMIC_REGISTER_3_MASK 0xffffffff
#define SOC_DNX_JER2_ARAD_INTERRUPTS_CMIC_REGISTER_4_MASK 0xffffffff

#define JER2_JER_DEFAULT_NOF_MC_GROUPS (80 * 1024) /* default number of Jericho MC groups in dual core mode */
#define JER2_JER_MAX_NOF_MC_GROUPS (128 * 1024) /* maximum number of Jericho MC groups */
#define JER2_JER_MAX_NOF_EGRESS_MESH_MC_GROUPS (64 * 1024) /* maximum number of Jericho egress MC groups using mesh MC as fabric */
#define JER2_QAX_DEFAULT_NOF_MC_GROUPS (16 * 1024) /* default number of JER2_QAX ingress and egress MC groups */
#define JER2_QAX_MAX_NOF_MC_GROUPS (SOC_IS_QUX(unit) ? (32 * 1024) : (64 * 1024)) /* maximum number of JER2_QAX MC groups */
#define JER2_QAX_MAX_NOF_EGRESS_MESH_MC_GROUPS (SOC_IS_QUX(unit) ? (16 * 1024): (32 * 1024)) /* maximum number of JER2_QAX mesh MC groups */
#define JER2_QAX_MC_MAX_CUD  0x3ffff

#define JER2_QAX_COSQ_TOTAL_FLOW_REGIONS (64)
#define QUX_COSQ_TOTAL_FLOW_REGIONS (32)



#define SOC_DNX_DEFAULT_NOF_EGR_DSCP_EXP_MARKING(unit)   (SOC_IS_JERICHO_PLUS(unit) ? 16 : 4)  /*maximum number of egress dscp exp marking*/

/* JER2_ARAD DRV Enums */
enum
{
  SOC_DNX_JER2_ARAD_PCI_CMC  = 0,
  SOC_DNX_JER2_ARAD_ARM1_CMC = 1,
  SOC_DNX_JER2_ARAD_ARM2_CMC = 2,
  SOC_DNX_JER2_ARAD_NUM_CMCS = 3
};

/*Arad Plus Specific*/
#define EXT_MODE_4LANES_PHY_PORTS ((1<<11) | (1<<12) | (1<<27) | (1<<28))
#define EXT_MODE_8LANES_PHY_PORTS (EXT_MODE_4LANES_PHY_PORTS | (1<<13) | (1<<14) | (1<<15) | (1<<16))

/* JER2_ARAD DRV Macros */
#define SOC_DNX_JER2_ARAD_FABRIC_PORT_TO_PHY_PORT(unit, fabric_port) \
    (fabric_port-FABRIC_LOGICAL_PORT_BASE(unit)-SOC_DNX_DEFS_GET(unit, first_fabric_link_id)+SOC_DNX_FIRST_SFI_PHY_PORT(unit))

#define SOC_DNX_JER2_ARAD_DRAM_MODE_REG_SET(field, prop) \
      val = soc_property_port_get(unit, 0, prop, prop_invalid); \
      if (val != prop_invalid) { \
        field = val; \
        dnx_jer2_arad->init.dram.dram_conf.params_mode.params.auto_mode = FALSE; \
      }

#define SOC_MTR_BLK(unit)	\
	( SOC_IS_QAX(unit) ? SOC_BLK_IMP : SOC_BLK_MRPS )

#define SOC_DNX_VERIFIY_SSM_NOT_EXCEEDS_MAX_VALUE(ssm_value,max_val) ( (ssm_value <= max_val) ? SOC_E_NONE : SOC_E_FAIL)

/* Functions */

int
soc_jer2_arad_q_pair_channel_mapping_get(int unit)
{
  int interface_i;
  int queue_i, core;
  int is_uc;
  char *s, *prefix, *tmp_s;
  int prefix_len, id;
  int nof_ilkn_ports;

  DNXC_INIT_FUNC_DEFS;

  nof_ilkn_ports = SOC_DNX_DEFS_GET(unit, nof_interlaken_ports);
  /* DEFAULT SETTINGS */
  for(interface_i = 0; interface_i < nof_ilkn_ports; interface_i++) 
  {
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][0]=0;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][1]=1;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][2]=4;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][3]=0;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][4]=5;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][5]=0;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][6]=0;
    SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][7]=0;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][0]=0;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][1]=0;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][2]=2;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][3]=3;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][4]=0;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][5]=0;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][6]=0;
    SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][7]=0;
  }


  SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
      prefix = "ILKN";
      prefix_len = sal_strlen(prefix);

      for(queue_i = 0; queue_i < nof_ilkn_ports * JER2_ARAD_Q_PAIRS_ILKN * 2; queue_i++)
      {
        is_uc = (queue_i < 16 ? 1 : 0);

        s = soc_property_port_suffix_num_get_str(unit, queue_i, core , spn_EGRESS_QUEUE, "core");

        if(s != NULL)
        {
          if (!sal_strncasecmp(s, prefix, prefix_len)) /* verify interface is ILKN */
          {
            /* Found ILKN */
            s += prefix_len;
            interface_i = sal_ctoi(s, &tmp_s);
              s += 2;
              id = sal_ctoi(s, &tmp_s);
              if(is_uc)
              {
                SOC_DNX_CONFIG(unit)->tm.uc_q_pair2channel_id[interface_i][queue_i % JER2_ARAD_Q_PAIRS_ILKN] = id;
              }
              else
              {
                SOC_DNX_CONFIG(unit)->tm.mc_q_pair2channel_id[interface_i][queue_i % JER2_ARAD_Q_PAIRS_ILKN] = id;
              }
          }
          else
          {
              /* Raise Error (illegal interface type) */
              DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ERROR: queue pair to channel mapping: illegal interface type, only ILKN is allowed")));
          }
        }/*if(null)*/
      }/*for queue_i*/
  } /* for core */
  
exit:
    DNXC_FUNC_RETURN;
}


int
soc_jer2_arad_default_config_tm_get(int unit)
{
    int i, core;
    int def_cal_len = 0;

    DNXC_INIT_FUNC_DEFS;

    SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports = SOC_DNX_DEFS_GET(unit, nof_interlaken_ports);
    for (i = 0; i < SOC_DNX_CONFIG(unit)->tm.max_interlaken_ports; i++) {
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_type[i] = DNX_TMC_FC_CAL_INB_TYPE_ILKN;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_mode[i] = SOC_DNX_FC_CAL_MODE_DISABLE;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_length[i][DNX_TMC_CONNECTION_DIRECTION_RX] = SOC_DNX_DEFS_GET(unit, fc_inb_cal_len_max);
        def_cal_len = (SOC_IS_JERICHO(unit) ? SOC_DNX_DEFS_GET(unit, fc_inb_cal_len_max) : (SOC_DNX_DEFS_GET(unit, fc_inb_cal_len_max)-1));
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_length[i][DNX_TMC_CONNECTION_DIRECTION_TX] = def_cal_len;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_rep_count[i][DNX_TMC_CONNECTION_DIRECTION_RX] = DNX_TMC_FC_OOB_CAL_REP_MIN;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_rep_count[i][DNX_TMC_CONNECTION_DIRECTION_TX] = DNX_TMC_FC_OOB_CAL_REP_MIN;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_calender_llfc_mode[i] = SOC_DNX_FC_INBAND_INTLKN_CAL_LLFC_MODE_DISABLE;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_llfc_mub_enable_mask[i] = SOC_DNX_FC_INBAND_INTLKN_LLFC_MUB_DISABLE;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_channel_mub_enable_mask[i][DNX_TMC_CONNECTION_DIRECTION_RX] = SOC_DNX_FC_INBAND_INTLKN_CHANNEL_MUB_DISABLE;
        SOC_DNX_CONFIG(unit)->tm.fc_inband_intlkn_channel_mub_enable_mask[i][DNX_TMC_CONNECTION_DIRECTION_TX] = SOC_DNX_FC_INBAND_INTLKN_CHANNEL_MUB_DISABLE;
    }

    SOC_DNX_CONFIG(unit)->tm.max_oob_ports = (SOC_IS_QAX(unit) ? 1 : 2);
    for (i = 0; i < SOC_DNX_CONFIG(unit)->tm.max_oob_ports; i++) {
        SOC_DNX_CONFIG(unit)->tm.fc_oob_type[i] = DNX_TMC_FC_CAL_TYPE_NONE;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_mode[i] = SOC_DNX_FC_CAL_MODE_DISABLE;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_calender_length[i][DNX_TMC_CONNECTION_DIRECTION_RX] = DNX_TMC_FC_OOB_CAL_LEN_MAX;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_calender_length[i][DNX_TMC_CONNECTION_DIRECTION_TX] = DNX_TMC_FC_OOB_CAL_LEN_MAX;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_calender_rep_count[i][DNX_TMC_CONNECTION_DIRECTION_RX] = DNX_TMC_FC_OOB_CAL_REP_MIN;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_calender_rep_count[i][DNX_TMC_CONNECTION_DIRECTION_TX] = DNX_TMC_FC_OOB_CAL_REP_MIN;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_ilkn_indication_invert[i][DNX_TMC_CONNECTION_DIRECTION_RX] = 0;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_ilkn_indication_invert[i][DNX_TMC_CONNECTION_DIRECTION_TX] = 0;
        SOC_DNX_CONFIG(unit)->tm.fc_oob_spi_indication_invert[i] = 0;
    }

    SOC_DNX_CONFIG(unit)->tm.max_ses = DNX_TMC_SCH_MAX_SE_ID_JER2_ARAD + 1;  
    SOC_DNX_CONFIG(unit)->tm.cl_se_min = DNX_TMC_CL_SE_ID_MIN_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.cl_se_max = DNX_TMC_CL_SE_ID_MAX_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.fq_se_min = DNX_TMC_FQ_SE_ID_MIN_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.fq_se_max = DNX_TMC_FQ_SE_ID_MAX_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.hr_se_min = DNX_TMC_HR_SE_ID_MIN_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.hr_se_max = DNX_TMC_HR_SE_ID_MAX_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.port_hr_se_min = DNX_TMC_HR_SE_ID_MIN_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.port_hr_se_max = DNX_TMC_HR_SE_ID_MIN_JER2_ARAD + DNX_TMC_SCH_MAX_PORT_ID_JER2_ARAD;

    SOC_DNX_CONFIG(unit)->tm.max_connectors = DNX_TMC_SCH_MAX_FLOW_ID_JER2_ARAD + 1;  
    SOC_DNX_CONFIG(unit)->tm.max_egr_q_prio = DNX_TMC_EGR_NOF_Q_PRIO_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.invalid_port_id_num = DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.invalid_se_id_num = DNX_TMC_SCH_SE_ID_INVALID_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.invalid_voq_connector_id_num = DNX_TMC_SCH_FLOW_ID_INVALID_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.nof_vsq_category = DNX_TMC_ITM_VSQ_GROUP_LAST_JER2_ARAD;
    SOC_DNX_CONFIG(unit)->tm.is_port_tc_enable = TRUE;

    /* We are not going to change macros to avoid such cases */
    /* coverity[same_on_both_sides] */
    for(core=0 ; core<SOC_DNX_DEFS_MAX(NOF_CORES) ; core++) {
        SOC_DNX_CONFIG(unit)->tm.hr_isq[core] = DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD; /* PORT ISQ */
        for (i = 0; i < DNX_TMC_MULT_FABRIC_NOF_CLS; i++) {
            SOC_DNX_CONFIG(unit)->tm.hr_fmqs[core][i] = DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD; /* PORT FMQ */
        }
    }

    SOC_DNX_CONFIG(unit)->tm.fc_calendar_coe_mode = DNX_TMC_FC_COE_MODE_PAUSE;
    SOC_DNX_CONFIG(unit)->tm.fc_calendar_pause_resolution = 1;  
    SOC_DNX_CONFIG(unit)->tm.fc_calendar_e2e_status_of_entries = DNX_TMC_FC_E2E_STATUS_SIZE_8B;  
    SOC_DNX_CONFIG(unit)->tm.fc_calendar_indication_invert = 0;  

    SOC_DNX_CONFIG(unit)->tm.fc_coe_mac_address[0] = 0x01;
    SOC_DNX_CONFIG(unit)->tm.fc_coe_mac_address[1] = 0x80;
    SOC_DNX_CONFIG(unit)->tm.fc_coe_mac_address[2] = 0xC2;
    SOC_DNX_CONFIG(unit)->tm.fc_coe_mac_address[3] = 0x00;
    SOC_DNX_CONFIG(unit)->tm.fc_coe_mac_address[4] = 0x00;
    SOC_DNX_CONFIG(unit)->tm.fc_coe_mac_address[5] = 0x01;    
    SOC_DNX_CONFIG(unit)->tm.fc_coe_ethertype = 0x8808;  
    SOC_DNX_CONFIG(unit)->tm.fc_coe_data_offset = 2;  

    DNXC_IF_ERR_EXIT(soc_jer2_arad_q_pair_channel_mapping_get(unit));

#if 1
    SOC_DNX_CONFIG(unit)->tm.queue_level_interface_enable = soc_property_get(unit, spn_QUEUE_LEVEL_INTERFACE, 0);
#endif

exit:
    DNXC_FUNC_RETURN;
}

int
     soc_jer2_arad_reset_cmicm_regs(int unit) 
{
    uint32 rval, divisor, dividend, mdio_delay;
    DNXC_INIT_FUNC_DEFS;

    /*In each Byte swap rings*/
    if (SOC_IS_ARDON(unit)) {

        /* SBUS ring map:
         * Ring 0: FMAC0 (10), FMAC1 (11), FMAC2 (12), FMAC3 (13), FMAC4 (14),
         *         FMAC5 (15), FMAC6 (16), FMAC7 (17), FMAC8 (18)FSRD0 (19),
         *         FSRD1 (20), FSRD2 (21), CLP0 (24), CLP1 (25),
         *         XLP0 (27), XLP1 (28), DRCE (44), DRCF (45), DRCG (46),
         *         DRCH (47), BRDC_FSRD(60), BRDC_FMAC(61), Broadcast (63)
         *         CFC (1),EGQ (2),EPNI (3),FCR (4),FDR (56), FDT (6), MESH_TOPOLOGY (8),
         *         RTP (9), NBI (26),CRPS (35), IPS (36), IPT (37), IQM (38), 
         *         FCT (39), CGM (54), OAMP (7) , IRE (49), IDR (50), 
         *         IRR (51), IHP (52), IHB (53), SCH (55)
         * Ring 1: MMU (22), OCB (23) DRCA (40), DRCB (41), DRCC (42), DRCD (43)
         * Ring 2: OTPC (57) , AVS (48)
         * Ring 5: ECI (0)
         */
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit,   0x00000005));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit,  0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x11000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x00000000));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x00001111));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x00000002));
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x00000020));
    } else {

        /* SBUS ring map:
         * Ring 0: OTPC (57)
         * Ring 1: FMAC0 (10), FMAC1 (11), FMAC2 (12), FMAC3 (13), FMAC4 (14), 
         *         FMAC5 (15), FMAC6 (16), FMAC7 (17), FMAC8 (18)FSRD0 (19), 
         *         FSRD1 (20), FSRD2 (21), CLP0 (24), CLP1 (25),
         *         XLP0 (27), XLP1 (28), DRCA (40), DRCB (41), DRCC (42), 
         *         DRCD (43), DRCE (44), DRCF (45), DRCG (46), DRCH (47),
         *         BRDC_FSRD(60), BRDC_FMAC(61), Broadcast (63)
         * Ring 2: CFC (1),EGQ (2),EPNI (3),FCR (4),FDR (5), FDT (6), MESH_TOPOLOGY (8), 
         *         RTP (9), NBI (26),CRPS (35), IPS (36), IPT (37), IQM (38), 
         *        FCT (39), OLP (48), CGM (54), OAMP (56)
         * Ring 3: MMU (22), OCB (23), IRE (49), IDR (50), 
         *         IRR (51), IHP (52), IHB (53), SCH (55)    
         * Ring 7:  ECI (0)
         *     
         */

        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_0_7r(unit, 0x02222227));    
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_8_15r(unit, 0x11111122));   
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_16_23r(unit, 0x33111111));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_24_31r(unit, 0x00011211));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_32_39r(unit, 0x22222000));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_40_47r(unit, 0x11111111));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_48_55r(unit, 0x32333332));  
        DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_RING_MAP_56_63r(unit, 0x10110012));  
    }

    /* Mdio - internal*/
    dividend = soc_property_get(unit, spn_RATE_INT_MDIO_DIVIDEND, 1);
    divisor = soc_property_get(unit, spn_RATE_INT_MDIO_DIVISOR, 24);
    rval = 0;
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_INT_MDIOr, &rval, DIVIDENDf, dividend);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_RATE_ADJUST_INT_MDIOr(unit, rval));

    /* Mdio - external*/
    dividend = soc_property_get(unit, spn_RATE_EXT_MDIO_DIVIDEND, 1);
    divisor = soc_property_get(unit, spn_RATE_EXT_MDIO_DIVISOR, 24);
    rval = 0;
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_EXT_MDIOr, &rval, DIVISORf, divisor);
    soc_reg_field_set(unit, CMIC_RATE_ADJUST_EXT_MDIOr, &rval, DIVIDENDf, dividend);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_RATE_ADJUST_EXT_MDIOr(unit, rval));

    /*Mdio -Delay*/
    rval = 0;
    mdio_delay = 0xf;
    soc_reg_field_set(unit, CMIC_MIIM_CONFIGr, &rval, MDIO_OUT_DELAYf, mdio_delay);
    DNXC_IF_ERR_EXIT(WRITE_CMIC_MIIM_CONFIGr(unit, rval));
 
    /* Led processor */  

    /* mapping xe port 1-32 to led processro memory indexes 1 -32 */  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_0_3r(unit, 0x75E7E0));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_4_7r(unit, 0x65A6DC));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_8_11r(unit, 0x5565D8));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_12_15r(unit, 0x4524D4));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_16_19r(unit, 0x24A2CC));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_20_23r(unit, 0x1461C8));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_24_27r(unit, 0x0420C4));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_28_31r(unit, 0x34E3D0));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_32_35r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_36_39r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_40_43r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_44_47r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_48_51r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_52_55r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_56_59r(unit, 0x000000));  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_PORT_ORDER_REMAP_60_63r(unit, 0x000000));  

    /* setting JER2_ARAD ports statuses scan values */  
    DNXC_IF_ERR_EXIT(WRITE_CMIC_LEDUP0_CTRLr(unit, 0xba));  

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_arad_info_config_device_ports(int unit) 
{
    soc_info_t          *si;

    DNXC_INIT_FUNC_DEFS;

    si  = &SOC_INFO(unit);

    si->num_time_interface = 1;

    DNXC_FUNC_RETURN;

}


int
soc_jer2_arad_cmic_sbus_timeout_set(int unit)
{
    soc_control_t        *soc;
    uint32 frequency, ticks,
           max_uint = 0xFFFFFFFF,
           max_ticks= 0x3FFFFF;
    
    DNXC_INIT_FUNC_DEFS;

    soc = SOC_CONTROL(unit);

        
    DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
     /* configure ticks to be a HW timeout that is 75% of SW timeout: */
    /* units:
     * schanTimeout is in microsecond
     * frequency is recieved in KHz, and modified to be in MHz.
     * after the modification: ticks = frequency * Timeout 
     */
    rv = soc_jer2_arad_core_frequency_config_get(unit, 600000, &(SOC_DNX_CONFIG(unit)->jer2_arad->init.core_freq.frequency));
    DNXC_IF_ERR_EXIT(rv);
    rv = soc_jer2_arad_schan_timeout_config_get(unit, &(SOC_CONTROL(unit)->schanTimeout));
    DNXC_IF_ERR_EXIT(rv);
#endif 


    frequency = (SOC_DNX_CONFIG(unit)->jer2_arad->init.core_freq.frequency) / 1000;

    if ((max_uint / frequency) > soc->schanTimeout) { /* make sure ticks can be represented in 32 bits*/
        ticks = frequency * soc->schanTimeout;
        ticks = ((ticks / 100) * 75); /* make sure hardware timeout is smaller than software*/
    } else {
        ticks = max_ticks;
    }

    DNXC_IF_ERR_EXIT(WRITE_CMIC_SBUS_TIMEOUTr(unit, ticks));

exit:
    DNXC_FUNC_RETURN;
}


int
soc_jer2_arad_is_olp(int unit, soc_port_t port, uint32* is_olp)
{
    soc_port_if_t interface;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface));
    *is_olp = (SOC_PORT_IF_OLP == interface ? 1: 0);

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_arad_is_oamp(int unit, soc_port_t port, uint32* is_oamp)
{
    soc_port_if_t interface;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface));
    *is_oamp = (SOC_PORT_IF_OAMP == interface ? 1: 0);

exit:
    DNXC_FUNC_RETURN;
}

void
soc_jer2_arad_dma_mutex_destroy(int unit)
{
    (void)soc_sbusdma_lock_deinit(unit);
}

void soc_jer2_arad_free_cache_memory(int unit)
{
    soc_mem_t mem;
    soc_error_t rc = SOC_E_NONE;

    for (mem = 0; mem < NUM_SOC_MEM; mem++)
    {
        if (!SOC_MEM_IS_VALID(unit, mem))
        {
            continue;
        }
        /* Deallocate table cache memory, if caching enabled */
        rc = soc_mem_cache_set(unit, mem, COPYNO_ALL, FALSE);
        if (SOC_FAILURE(rc))
        {
            LOG_ERROR(BSL_LS_SOC_INIT, (BSL_META_U(unit, "Error to deallocate cache for mem %d\n"), mem));
        }
    }
}

int
soc_jer2_arad_free_tm_port_and_recycle_channel(int unit, int port)
{
    uint32 is_valid;
    soc_port_if_t interface_type;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_is_valid_port_get(unit, port, &is_valid));
    if (!is_valid) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("invalid port %d"), port));
    }
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_interface_type_get(unit, port, &interface_type));

    if (interface_type != SOC_PORT_IF_RCY) {
        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("port(%d) not allocated by soc_jer2_arad_allocate_rcy_port()"), port));
    }
   

    DNXC_IF_ERR_EXIT(dnx_port_sw_db_initialized_set(unit, port, 0));
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_port_remove(unit, port));
  
exit:
    DNXC_FUNC_RETURN;
}


int
soc_jer2_arad_fc_oob_mode_validate(int unit, int port)
{
    DNXC_INIT_FUNC_DEFS;

    DNXC_FUNC_RETURN;
}


/* reserved base q pairs for isq and fmq, needed in case of dynamic nif */ 
int soc_jer2_arad_ps_reserved_mapping_init(int unit)
{
    int  val, i, core;
    char *propval;
    DNXC_INIT_FUNC_DEFS;

    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        SOC_PBMP_CLEAR(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_isq_base_q_pair[core]);
        SOC_PBMP_CLEAR(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_fmq_base_q_pair[core]);
    }
    
    /* Static mapping by SOC property. Explicit OTM-queue base pair assignment for ISQ-ROOT */
    SOC_DNX_CORES_ITER(SOC_CORE_ALL, core) {
        propval = soc_property_suffix_num_only_suffix_str_get(unit, core, spn_OTM_BASE_Q_PAIR, "isq_core");
        if (propval != NULL) {
            val = _shr_ctoi(propval);
            if (val < 0 || val >= SOC_MAX_NUM_PORTS) {
                DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("soc_jer2_arad_ps_reserved_mapping_init error in soc_jer2_arad_info_config")));
            }
            SOC_PBMP_PORT_ADD(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_isq_base_q_pair[core], val);
        } else {
            propval = soc_property_suffix_num_only_suffix_str_get(unit, 0, spn_OTM_BASE_Q_PAIR, "isq");
            if (propval != NULL) {
                val = _shr_ctoi(propval);
                if (val < 0 || val >= SOC_MAX_NUM_PORTS) {
                    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("soc_jer2_arad_ps_reserved_mapping_init error in soc_jer2_arad_info_config")));
                }
                SOC_PBMP_PORT_ADD(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_isq_base_q_pair[core], val);
            }
        }

        /* Static mapping by SOC property. Explicit OTM-queue base pair assignment for FMQ-ROOT */
        for (i=0; i<DNX_TMC_MULT_FABRIC_NOF_CLS; i++) 
        {
            char buf[20];
            sal_snprintf(buf, 20,"fmq%d_core", i);
            propval = soc_property_suffix_num_only_suffix_str_get(unit, core, spn_OTM_BASE_Q_PAIR, buf);
            if (propval != NULL) {
                val = _shr_ctoi(propval);
                if (val < 0 || val >= SOC_MAX_NUM_PORTS) {
                    DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("soc_jer2_arad_ps_reserved_mapping_init error in soc_jer2_arad_info_config")));
                }
                SOC_PBMP_PORT_ADD(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_fmq_base_q_pair[core], val);
            } else {
                propval = soc_property_suffix_num_only_suffix_str_get(unit, i, spn_OTM_BASE_Q_PAIR, "fmq");
                if (propval != NULL) {
                    val = _shr_ctoi(propval);
                    if (val < 0 || val >= SOC_MAX_NUM_PORTS) {
                        DNXC_EXIT_WITH_ERR(SOC_E_FAIL, (_BSL_DNXC_MSG("soc_jer2_arad_ps_reserved_mapping_init error in soc_jer2_arad_info_config")));
                    }
                    SOC_PBMP_PORT_ADD(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_fmq_base_q_pair[core], val);
                }
            }
        }
     }

exit:
    DNXC_FUNC_RETURN;
}

static int
soc_jer2_arad_fmq_isq_hr_init(int unit, bcm_core_t core, uint8 is_fmq)
{
    int index = 0, base_q_pair = 0, found = 0;
    uint8 is_hr_free = 0;

    DNXC_INIT_FUNC_DEFS;

    /* Allocate free Q-Pair for ISQ */
    if (!is_fmq && (SOC_DNX_CONFIG(unit)->tm.hr_isq[core] == DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD) /* PORT ISQ */) {
        for (base_q_pair=0; base_q_pair<SOC_MAX_NUM_PORTS; base_q_pair++) 
        {
            if (SOC_PBMP_MEMBER(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_isq_base_q_pair[core], base_q_pair)) {
                /* validate HR is free */
                DNXC_IF_ERR_EXIT(soc_jer2_arad_validate_hr_is_free(unit, core, base_q_pair, &is_hr_free));
                if (is_hr_free) {
                    found = 1;
                    break;
                }
            }
        }
        if (!found)
        {
            DNXC_IF_ERR_EXIT(jer2_arad_ps_db_find_free_non_binding_ps(unit, core, 0 /*is_init*/, &base_q_pair));
        }

        SOC_DNX_CONFIG(unit)->tm.hr_isq[core] = base_q_pair; 
    }
    
    /* Allocate free Q-Pairs (4) for FMQ */
    if (is_fmq && (SOC_DNX_CONFIG(unit)->tm.hr_fmqs[core][0] == DNX_TMC_SCH_PORT_ID_INVALID_JER2_ARAD) /* PORT FMQ */) {
        for (base_q_pair=0; base_q_pair<SOC_MAX_NUM_PORTS; base_q_pair++) 
        {
            if (SOC_PBMP_MEMBER(SOC_DNX_CONFIG(unit)->jer2_arad->reserved_fmq_base_q_pair[core], base_q_pair) && index<4) {
                SOC_DNX_CONFIG(unit)->tm.hr_fmqs[core][index] = base_q_pair;
                index++;
            }
        }
        while (index < DNX_TMC_MULT_FABRIC_NOF_CLS) {
            DNXC_IF_ERR_EXIT(jer2_arad_ps_db_find_free_non_binding_ps(unit, core, 0 /*is_init*/, &base_q_pair));
            SOC_DNX_CONFIG(unit)->tm.hr_fmqs[core][index] = base_q_pair;
            index++;
        }      
    }

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_arad_isq_hr_get(int unit, bcm_core_t core, int *hr_isq)
{
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    rv = soc_jer2_arad_fmq_isq_hr_init(unit, core, FALSE);
    DNXC_IF_ERR_EXIT(rv);

    *hr_isq = SOC_DNX_CONFIG(unit)->tm.hr_isq[core];

exit:
    DNXC_FUNC_RETURN;
}

int
soc_jer2_arad_fmq_base_hr_get(int unit, bcm_core_t core, int** base_hr_fmq)
{
    soc_error_t rv;
    DNXC_INIT_FUNC_DEFS;

    rv = soc_jer2_arad_fmq_isq_hr_init(unit, core, TRUE);
    DNXC_IF_ERR_EXIT(rv);

    *base_hr_fmq = &(SOC_DNX_CONFIG(unit)->tm.hr_fmqs[core][0]);

exit:
    DNXC_FUNC_RETURN;
}


int soc_jer2_arad_cmic_info_config(int unit)
{

    int cmc_i;

    DNXC_INIT_FUNC_DEFS;

    SOC_PCI_CMC(unit)   = soc_property_uc_get(unit, 0, spn_PCI_CMC, SOC_DNX_JER2_ARAD_PCI_CMC);
    SOC_ARM_CMC(unit, 0) = soc_property_uc_get(unit, 1, spn_CMC, SOC_DNX_JER2_ARAD_ARM1_CMC);
    SOC_ARM_CMC(unit, 1) = soc_property_uc_get(unit, 2, spn_CMC, SOC_DNX_JER2_ARAD_ARM2_CMC);

    /* CMC COSQ configuration */
    SOC_CMCS_NUM(unit) = SOC_DNX_JER2_ARAD_NUM_CMCS;
    NUM_CPU_COSQ(unit) = SOC_DNX_JER2_ARAD_NUM_CPU_COSQ - 1;

    
    

    /* Get these values from SOC Properties */
    NUM_CPU_ARM_COSQ(unit, 0) = soc_property_uc_get(unit, 0, spn_NUM_QUEUES, NUM_CPU_COSQ(unit));
    NUM_CPU_ARM_COSQ(unit, 1) = soc_property_uc_get(unit, 1, spn_NUM_QUEUES, 0);
    NUM_CPU_ARM_COSQ(unit, 2) = soc_property_uc_get(unit, 2, spn_NUM_QUEUES, 0);

    /* clear ('0') the cosq bitmaps per cmc */
    for (cmc_i = 0; cmc_i < SOC_CMCS_NUM(unit); cmc_i++) {
        SHR_BITCLR_RANGE(CPU_ARM_QUEUE_BITMAP(unit, cmc_i), 0, NUM_CPU_COSQ_MAX);
    }

    /* set ('1') the cosq bitmaps per cmc */
    SHR_BITSET_RANGE(CPU_ARM_QUEUE_BITMAP(unit, 0), 0, NUM_CPU_ARM_COSQ(unit, 0));
    SHR_BITSET_RANGE(CPU_ARM_QUEUE_BITMAP(unit, 1), NUM_CPU_ARM_COSQ(unit, 0), NUM_CPU_ARM_COSQ(unit, 1));
    SHR_BITSET_RANGE(CPU_ARM_QUEUE_BITMAP(unit, 2), NUM_CPU_ARM_COSQ(unit, 0) + NUM_CPU_ARM_COSQ(unit, 1), NUM_CPU_ARM_COSQ(unit, 2));
    
    DNXC_FUNC_RETURN;
}


int soc_jer2_arad_dma_mutex_init(int unit)
{
    int rv;
    
    DNXC_INIT_FUNC_DEFS

    rv = soc_sbusdma_lock_init(unit);
    if (rv != SOC_E_NONE) {
        DNXC_EXIT_WITH_ERR(rv, (_BSL_DNXC_MSG("failed to allocate TSLAMDMA/ TABLEDMA Locks")));
    }

exit:
    DNXC_FUNC_RETURN;
}

/* dummy warmboot callbacks to be used if WArmboot is not initialized */
#ifdef BCM_WARM_BOOT_SUPPORT
int
soc_dnx_read_dummy_func(int unit, uint8 *buf, int offset, int nbytes)
{
    return SOC_E_RESOURCE;
}

int
soc_dnx_write_dummy_func(int unit, uint8 *buf, int offset, int nbytes)
{
    return SOC_E_RESOURCE;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

#ifdef BCM_WARM_BOOT_SUPPORT
void
soc_jer2_arad_init_empty_scache(int unit)
{
    int     stable_location = 0;
    uint32  stable_flags = 0;
    int     stable_size = 0;

    if (soc_scache_is_config(unit)!=SOC_E_NONE) {
       /* EMPTY SCACHE INITIALIZATION ->
          in case stable_* parameters are not defined in configuration file, 
          initiating scache with size 0(zero). in order that scache commits 
          wont fail and cause application exit upon startup */
       if (soc_switch_stable_register(unit,
                                      &soc_dnx_read_dummy_func,
                                      &soc_dnx_write_dummy_func,
                                      NULL, NULL) < 0) {
              LOG_ERROR(BSL_LS_SOC_INIT,
                        (BSL_META_U(unit,
                                    "soc_switch_stable_register failure.\n")));
       }

       if (soc_stable_set(unit, stable_location, stable_flags) < 0) {
              LOG_ERROR(BSL_LS_SOC_INIT,
                        (BSL_META_U(unit,
                                    "soc_stable_set failure\n")));
       } else if (soc_stable_size_set(unit, stable_size) < 0) {
              LOG_ERROR(BSL_LS_SOC_INIT,
                        (BSL_META_U(unit,
                                    "soc_stable_size_set failure\n")));
       }
       /* <- EMPTY SCACHE INITIALIZATION */
    }
}
#endif /* BCM_WARM_BOOT_SUPPORT */

int
soc_jer2_arad_device_reset(int unit, int mode, int action)
{
    
    DNXC_LEGACY_FIXME_ASSERT;
    return -1;
}

int jer2_arad_info_config_custom_reg_access(int unit)
{
    DNXC_INIT_FUNC_DEFS;
    /* Empty implementation */
    DNXC_FUNC_RETURN;
}


int
soc_jer2_arad_validate_hr_is_free(int unit, int core, uint32 base_q_pair, uint8 *is_free)
{
    int se_id, flow_id, rc;

    DNXC_INIT_FUNC_DEFS;

    /* retrieve corresponding SE element */
    se_id = base_q_pair + JER2_ARAD_HR_SE_ID_MIN;

    if (se_id == (SOC_DNX_CONFIG(unit)->tm.invalid_se_id_num)) {
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("error in retreiving seId" )));
    }

    /* retrieve corresponding flow id */
    flow_id = (MBCM_DNX_DRIVER_CALL_WITHOUT_DEV_ID(unit,mbcm_dnx_sch_se2flow_id,((se_id))));
    if (flow_id == SOC_DNX_CONFIG(unit)->tm.invalid_voq_connector_id_num) {
        DNXC_EXIT_WITH_ERR(BCM_E_INTERNAL, (_BSL_DNXC_MSG("error in retreiving FlowId for seId(0x%x)"), se_id));
    }

    /* try to reserve HR in allocation manager */
    rc = bcm_dnx_am_cosq_scheduler_allocate(unit, core, 1, SHR_RES_ALLOC_WITH_ID, FALSE, FALSE, FALSE, TRUE, 1, DNX_TMC_AM_SCH_FLOW_TYPE_HR, NULL, &flow_id);
    if (rc != SOC_E_NONE) 
    {
        *is_free = FALSE;
    } 
    else 
    {
        *is_free = TRUE;
        /* free HR resource */
        DNXC_IF_ERR_EXIT(bcm_dnx_am_cosq_scheduler_deallocate(unit, core, SHR_RES_ALLOC_WITH_ID, FALSE, FALSE, FALSE, TRUE, 1,DNX_TMC_AM_SCH_FLOW_TYPE_HR, flow_id));
    }

exit:
  DNXC_FUNC_RETURN;
}

#undef _ERR_MSG_MODULE_NAME

