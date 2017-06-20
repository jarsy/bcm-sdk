/*
 * $Id: switch.c,v 1.23 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        switch.c
 * Purpose:     BCM definitions  for bcm_switch_control
 *              
 */

#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/switch.h>
#include <bcm/error.h>
#include <bcm/stack.h>
#include <bcm/link.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/switch.h>
#include <bcm_int/sbx_dispatch.h>
#include <bcm_int/sbx/state.h>

#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/soc_sw_db.h>
#include <soc/sbx/counter.h>
#include <soc/mem.h>
#include <soc/sbx/caladan3/wb_db_ppe.h>

#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/caladan3/cosq.h>
#include <bcm_int/sbx/caladan3/l3.h>
#include <bcm_int/sbx/caladan3/mpls.h>
#include <bcm_int/sbx/caladan3/ipmc.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/caladan3/port.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/g3p1.h>
#include <bcm_int/sbx/caladan3/oam/oam.h>
#include <bcm_int/sbx/caladan3/wb_db_trunk.h>
#include <bcm_int/sbx/caladan3/wb_db_vswitch.h>
#include <bcm_int/sbx/caladan3/wb_db_mim.h>
#include <bcm_int/sbx/caladan3/wb_db_vlan.h>
#include <bcm_int/sbx/caladan3/wb_db_l2.h>
#include <bcm_int/sbx/caladan3/wb_db_l2cache.h>
#include <bcm_int/sbx/caladan3/wb_db_l3.h>
#include <bcm_int/sbx/caladan3/wb_db_mpls.h>
#include <bcm_int/sbx/caladan3/wb_db_allocator.h>
#include <bcm_int/sbx/caladan3/wb_db_field.h>
#include <bcm_int/sbx/caladan3/wb_db_mirror.h>
#include <bcm_int/sbx/caladan3/wb_db_init.h>
#include <sal/appl/sal.h>
#include <sal/core/libc.h>
#include <sal/core/time.h>

/* uncomment next line to display warmboot sync times on the console */
/* #define WB_TIME_STAMP_DBG */

#ifdef WB_TIME_STAMP_DBG
sal_usecs_t        wb_start0;
sal_usecs_t        wb_start1;
#define WB_TIME_STAMP_START0 wb_start0 = sal_time_usecs();
#define WB_TIME_STAMP_START1 wb_start1 = sal_time_usecs();
#define WB_TIME_STAMP(t, msg)                    \
  do { \
    sal_usecs_t us, diff;                   \
    us = (t) ?  wb_start1 : wb_start0;                   \
    diff = SAL_USECS_SUB(sal_time_usecs(),us)/1000000; \
    if (diff) \
        LOG_CLI((BSL_META(" %s: %us\n"), msg, diff));        \
    else \
        LOG_CLI((BSL_META(" %s: <1s\n"), msg));   \
  } while(0);
#else
#define WB_TIME_STAMP_START0
#define WB_TIME_STAMP_START1
#define WB_TIME_STAMP(t, msg)
#endif


typedef enum _caladan3_xt_cos_override_ids_e {
    caladan3_xt_learn                   = 0,
    
    caladan3_xt_cos_override_ids_count
} caladan3_xt_cos_override_ids_e;

typedef struct _caladan3_xt_cos_override_t {
    uint32 xt;
    int      cos;  /* as added to the BASE, not the default, Higher the cos
                    * numerically, the LOWER the prirority in the QE*/
} caladan3_xt_cos_override_t;

soc_sbx_caladan3_port_queue_t
    port_queue_config[BCM_MAX_NUM_UNITS][SOC_SBX_CALADAN3_PORT_MAP_ENTRIES];

#ifdef BCM_CALADAN3_G3P1_SUPPORT
/*
 * Function: 
 *   _bcm_g3p1_exception_drop_set
 * Purpose:
 *  Set the g3p1 exception type's drop preference
 */
int
_bcm_g3p1_exception_drop_set(int unit, uint8 eType, uint8 bDrop)
{
    int rv = BCM_E_INTERNAL;
    soc_sbx_g3p1_xt_t xt;

    rv = soc_sbx_g3p1_xt_get(unit, eType, &xt);
    if (SOC_SUCCESS(rv)) {
        xt.forward = !bDrop;
        rv = soc_sbx_g3p1_xt_set(unit, eType, &xt);
    }

    return rv;
}

/*
 * Function: 
 *   _bcm_g3p1_exception_cos_set
 * Purpose:
 *  Set the g3p1 exception type's drop cos
 */
int
_bcm_g3p1_exception_cos_set(int unit, uint8 eType, uint8 uCos)
{
    int rv = BCM_E_INTERNAL;
    soc_sbx_g3p1_xt_t xt;
    uint32 num_cos, mask;

    rv = soc_sbx_g3p1_xt_get(unit, eType, &xt);
    if (SOC_SUCCESS(rv)) {
	num_cos = sbx_num_cosq[unit];
        if (num_cos > 4) {
            mask = 0x7;
        }else if (num_cos > 2) {
            mask = 0x3;
        }else{
            mask = 0x1;
        }

        xt.qid = (xt.qid & (~mask)) | uCos;
        rv = soc_sbx_g3p1_xt_set(unit, eType, &xt);
    }

    return rv;
}

/*
 * Function:
 *   _bcm_g3p1_exception_cos_get
 * Purpose:
 *  Get the g3p1 exception type's cos
 */
int
_bcm_g3p1_exception_cos_get(int unit, uint8 eType, uint8* uCos)
{
    int rv = BCM_E_INTERNAL;
    soc_sbx_g3p1_xt_t xt;
    uint32 num_cos, mask;

    rv = soc_sbx_g3p1_xt_get(unit, eType, &xt);
    if (SOC_SUCCESS(rv)) {
        num_cos = sbx_num_cosq[unit];
        if (num_cos > 4) {
            mask = 0x7;
        }else if (num_cos > 2) {
            mask = 0x3;
        }else{
            mask = 0x1;
        }
        *uCos = xt.qid & mask;
    }

    return rv;
}


int
_bcm_caladan3_g3p1_switch_exc_table_set(int unit, int base_qid, int rmw)
{
    int                     rv  = BCM_E_INTERNAL;
    caladan3_xt_cos_override_t  override_cos[caladan3_xt_cos_override_ids_count];
    int                     ovrIdx, override, i;
    int                     qid = 0;
    int                     xt_table_size = soc_sbx_g3p1_xt_table_size_get(unit);
    soc_sbx_g3p1_xt_t       xt;

    /* give learning exceptions a slightly lower priority to allow for
     * bpdus to faster service when under a high rate of learning
     */
    BCM_IF_ERROR_RETURN(
        soc_sbx_g3p1_exc_smac_learn_idx_get(unit, 
                                            &override_cos[caladan3_xt_learn].xt));
    override_cos[caladan3_xt_learn].cos = _BCM_SWITCH_EXC_DEFAULT_COS + 1;

    rv = BCM_E_NONE;
    for (i=0; ((i < xt_table_size) && (SOC_SUCCESS(rv))); i++) {
        if (rmw) {
            rv = soc_sbx_g3p1_xt_get(unit, i, &xt);
        } else {
            soc_sbx_g3p1_xt_t_init(&xt);
        }
        
        if (SOC_SUCCESS(rv)) {
            override = 0;
            
            qid = base_qid;
            for (ovrIdx = 0; 
                 ovrIdx < caladan3_xt_cos_override_ids_count && !override; 
                 ovrIdx++) 
            {
                if (override_cos[ovrIdx].xt == i) {
                    qid += override_cos[ovrIdx].cos;
                    override = 1;
                }
            }
            
            if (!override) {
                qid += _BCM_SWITCH_EXC_DEFAULT_COS;
            }
             
            xt.qid = qid;
            rv = soc_sbx_g3p1_xt_set(unit, i, &xt);
        } else {
            LOG_ERROR(BSL_LS_BCM_SWITCH,
                      (BSL_META_U(unit,
                                  "Failed get xti=%d\n"),
                       i));
        }
    }
    
    if (SOC_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_SWITCH,
                  (BSL_META_U(unit,
                              "Failed to set xti=%d to qid=0x%x\n"),
                   i, qid));
    }

    return rv;
}


/*=============================================================================
 * switch control on broadshield checks
 *-----------------------------------------------------------------------------
 */

/* max number of G2 exceptions mapped to a single bcmSwitchDosAttack* type */
#define MAX_MAPPED   10


/* global drop preference for captured BroadShield packets */
STATIC uint8 _drop[BCM_LOCAL_UNITS_MAX];

/* Structures to translate between the BCM Broadshield types and the caladan3
 * implementation.  In general, there are three types:
 *  BOOL - Enable or Disable the check with a global drop preference
 *  CPU  - Define the drop preference for a particular check that is
 *         always enabled.
 *  REG  - For 'integer' value switch controls; these have specific registers
 *         associated to influence hardware checking.
 */
typedef enum {
    sbx_sw_ctrl_type_bool,
    sbx_sw_ctrl_type_reg,
    sbx_sw_ctrl_type_cpu
} sbx_sw_ctrl_type;

typedef struct {
    uint8           sb_type;    /* The hardware exception code */
    sbx_sw_ctrl_type  ctrl_type;  /* reg type, bool type, to cpu type */
    
    /*  valid only for reg type */
    uint32 reg;
    uint32 field;
} sbx_sw_ctrl_desc_t;

#define CPU_TYPE(x)  (x.ctrl_type == sbx_sw_ctrl_type_cpu)
#define REG_TYPE(x)  (x.ctrl_type == sbx_sw_ctrl_type_reg)
#define BOOL_TYPE(x) (x.ctrl_type == sbx_sw_ctrl_type_bool)


typedef struct {
  int                  bsc_type;
  bcm_switch_control_t bcm_type;
  uint8                flags;
#define C3_BSC_DISABLE 0x1
#define C3_BSC_DROP    0x2
#define C3_BSC_REPORT  0x4

} c3_broadshield_check_t;

static c3_broadshield_check_t bschecks[] = {
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_INV_PPP_ADDR_CTL,      0,                                     C3_BSC_DISABLE },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_INV_PPP_PID,           0,                                     C3_BSC_DISABLE },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN0_FFF,        0,                                     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN1_FFF,        0,                                     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_VLAN2_FFF,        0,                                     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_SMAC_EQ_DMAC,     bcmSwitchDosAttackMACSAEqualMACDA,     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_GRE_INV_RES0,          0,                                     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_MAC_ZERO,         bcmSwitchSourceMacZeroDrop,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_TYPE_VALUE,       0,                                     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_ENET_SMAC_MCAST,       0,                                     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_RUNT_PKT,         bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_OPTIONS,          bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_CHECKSUM,     bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_VER,          bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_RUNT_HDR,         bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_LEN_ERR,          bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_PKT_LEN_ERR,      bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_SA,           bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_INV_DA,           bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_SA_EQ_DA,         bcmSwitchDosAttackSipEqualDip,         C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_LOOPBACK,         bcmSwitchMartianAddr,                  C3_BSC_DISABLE },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_MARTIAN_ADDR,     bcmSwitchMartianAddr,                  C3_BSC_DISABLE },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV4_ICMP_FRAG,        bcmSwitchDosAttackIcmpFragments,       C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_RUNT,             bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_VER,          bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_PKT_LEN_ERR,      bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_SA,           bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_INV_DA,           bcmSwitchDosAttackL3Header,            C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_SA_EQ_DA,         bcmSwitchDosAttackSipEqualDip,         C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_IPV6_LOOPBACK,         bcmSwitchMartianAddr,                  C3_BSC_DISABLE },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SP_EQ_DP,           bcmSwitchDosAttackSipEqualDip,         C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_NULL_SCAN,          bcmSwitchDosAttackFlagZeroSeqZero,     C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_XMAS_SCAN,          bcmSwitchDosAttackTcpXMas,             C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SYN_FIN,            bcmSwitchDosAttackTcpFlagsSF,          C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_TINY_FRAG,          bcmSwitchDosAttackTcpFrag,             C3_BSC_DROP },
  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_SYN_SPORT_LT_1024,  bcmSwitchDosAttackSynFrag,             C3_BSC_DROP },

  {SOC_SBX_CALADAN3_PPE_HEADER_CHECKER_EXC_L4_TINY_FRAG,          bcmSwitchDosAttackMinTcpHdrSize,       C3_BSC_DROP },
};


/* Note: The fe2000 version of these functions are defined in <bcm_int/sbx/l2.h>. This file
 * needs to be broken into separate versions for fe2000 and caladan3. In the meantime
 * these external functions are defined here.
 */
extern int bcm_caladan3_l2_egress_range_reserve(int unit, int highOrLow, uint32 val);
extern int bcm_caladan3_l2_egress_range_get(int unit, uint32 *low, uint32 *high);
extern int system_shutdown(int unit, int cleanup);

extern soc_sbx_caladan3_port_config_t
    interface_config[BCM_MAX_NUM_UNITS][SOC_SBX_CALADAN3_PORT_MAP_ENTRIES];

static void
_rmw_reg(int unit, sbx_sw_ctrl_desc_t *ctrl_desc, uint32 val)
{  
    uint32 data;
    /* coverity[check_return] */
    soc_reg32_get(unit, ctrl_desc->reg, REG_PORT_ANY, 0, &data);
    soc_reg_field_set(unit, ctrl_desc->reg, &data, ctrl_desc->field, val);
    /* coverity[check_return] */
    soc_reg32_set(unit, ctrl_desc->reg, REG_PORT_ANY, 0, data);
}


static void
_read_reg(int unit, sbx_sw_ctrl_desc_t *ctrl_desc, uint32 *val)
{
    uint32 data;
    
    /* coverity[check_return] */
    soc_reg32_get(unit, ctrl_desc->reg, REG_PORT_ANY, 0, &data);

    *val = soc_reg_field_get(unit, ctrl_desc->reg, data, ctrl_desc->field);
}

/*
 * Function:
 *   _bcm_g3p1_drop_on_exception_set
 * Purpose:
 *   Iterate over all BOOL_TYPES_ARRAY that are currently enabled and
 *   set the drop preference.
 */
int
_bcm_g3p1_drop_on_exception_set(int unit, uint8 drop)
{
    uint8                 bEnable = 1;
    int rv = BCM_E_NONE;
    int i;

  for (i=0; i<sizeof(bschecks)/sizeof(c3_broadshield_check_t); i++) {
    if (bschecks[i].flags & C3_BSC_REPORT) {
      continue;
    }
    rv = soc_sbx_caladan3_ppe_broad_shield_check_get(unit, bschecks[i].bsc_type, &bEnable);
    if (SOC_SUCCESS(rv)) {
      /* update the exception path only if it is enabled */
      if (bEnable) {
	rv = _bcm_g3p1_exception_drop_set(unit, bschecks[i].bsc_type, drop);
      }
    }
  }
    
  return rv;
}

/*
 * Function:
 *   _bcm_to_caladan3_broad_shield_type_get
 * Purpose:
 *   Return the 1-many mapping of bcmSwitchDosAttack* type to G2 Exceptions
 */
int
_bcm_to_caladan3_broad_shield_type_get(int unit,
                                  bcm_switch_control_t bcm_type,
                                  sbx_sw_ctrl_desc_t ctrl_desc[],
                                  int *num_mapped)
{
    int num = 0;
    int rv = BCM_E_NONE;
    int i;

    if (*num_mapped < MAX_MAPPED) {
        return BCM_E_PARAM;
    }
    *num_mapped = 0;
   
    /* Set 'ctrl_desc' members and Increment 'num' index ONCE */
#define STI_BASE(sbt, t) \
     ctrl_desc[num].sb_type = sbt;    \
     ctrl_desc[num].ctrl_type = t;   \
     num++;

    /* Macros for setting the switch control data type and 
     * incrementing the num_mapped counter
     */
#define STI_BOOL(sbt)  STI_BASE(sbt, sbx_sw_ctrl_type_bool)
#define STI_CPU(sbt)   STI_BASE(sbt, sbx_sw_ctrl_type_cpu)
#define STI_REG(unit, _reg, _field) \
    ctrl_desc[num].reg = _reg; \
    ctrl_desc[num].field = _field; \
    STI_BASE(0, sbx_sw_ctrl_type_reg); 

  for (i=0; i<sizeof(bschecks)/sizeof(c3_broadshield_check_t) && i<MAX_MAPPED; i++) {
    if (bschecks[i].bcm_type != bcm_type || bschecks[i].bcm_type == 0)
      continue;
    if (bschecks[i].flags & C3_BSC_REPORT) {
      STI_CPU(bschecks[i].bsc_type);
    } else {
      /* There are cases that have associated configurable values.
	 For example, we can filter on IP addresses (PP_HC_IPV4_FILTER*).
	 To use these, we need to define new switch control enumerations.
       */
      switch (bcm_type) {
      case bcmSwitchDosAttackMinTcpHdrSize:
        STI_REG(unit, PP_HC_CONFIG4r, MIN_TFRAGf);
	break;
      default:
	STI_BOOL(bschecks[i].bsc_type);
	break;
      }
    }
  }
  *num_mapped = num;
  return rv;
}


/*-----------------------------------------------------------------------------
 * END switch control on broadshield checks
 *=============================================================================
 */

int
_bcm_caladan3_g3p1_switch_control_init(int unit)
{
  int                 i, rv = BCM_E_NONE;

  rv = _bcm_caladan3_g3p1_switch_exc_table_set(unit, SBX_EXC_QID_BASE(unit),
                                               FALSE);
 
  for (i=0; i<sizeof(bschecks)/sizeof(c3_broadshield_check_t); i++) {
    uint8 enable = ((bschecks[i].flags & C3_BSC_DISABLE) != C3_BSC_DISABLE);
    
    rv = soc_sbx_caladan3_ppe_broad_shield_check_set(unit, bschecks[i].bsc_type, enable);
    
    if (bschecks[i].flags & C3_BSC_DROP) {
      rv = _bcm_g3p1_exception_drop_set(unit, bschecks[i].bsc_type, 1);
    }
    if (rv != SOC_E_NONE) {
      rv = BCM_E_INTERNAL;
      break;
    }
  }

  return rv;
}


int
_bcm_caladan3_port_config_install(int unit)
{
    int                         status = BCM_E_NONE;
    soc_port_t                  line_port;
    bcm_c3_cosq_port_queues_t   port_queues;
    int                         queue;
    int                         queue_count;

    /* Create the database for the port and queue configuration */
    /* coverity [overrun-buffer-arg] */
    sal_memset(&port_queue_config[unit], 0, sizeof(port_queue_config));

    for (line_port = 0; line_port < SOC_SBX_CALADAN3_MAX_LINE_PORT; line_port++) {
        if (!interface_config[unit][line_port].valid) {
            continue;
        }
        /*LOG_CLI((BSL_META_U(unit,
                              "%s: Configuring port %d\n"), __FUNCTION__, line_port));*/
        port_queue_config[unit][line_port].valid = TRUE;

        /* Get the queue(s) for this port */
        status = bcm_c3_cosq_queues_from_port_get(unit, line_port, &port_queues);
        if (status != SOC_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: bcm_c3_cosq_queues_from_port_get failed: %s\n"),
                     FUNCTION_NAME(), bcm_errmsg(status)));
        }

        /* Get the line source queue(s) */
        /* For now just get the first queue number */
        BCM_PBMP_ITER(port_queues.sq_bmp, queue) {
            break;
        }
        port_queue_config[unit][line_port].line_sq_base = queue;
        BCM_PBMP_COUNT(port_queues.sq_bmp, queue_count);
        if (queue_count > 1) {
            port_queue_config[unit][line_port].sq_count = queue_count;
        }

        /* Get the line destination queue(s) */
        /* For now just get the first queue number */
        BCM_PBMP_ITER(port_queues.dq_bmp, queue) {
            break;
        }
        queue += 128;
        port_queue_config[unit][line_port].line_dq_base = queue;
        BCM_PBMP_COUNT(port_queues.dq_bmp, queue_count);
        if (queue_count > 1) {
            port_queue_config[unit][line_port].dq_count = queue_count;
        }

        /* Get the fabric port and queues */
        status = bcm_c3_cosq_dest_port_from_sq_get(unit,
                                                   port_queue_config[unit][line_port].line_sq_base,
                                                   &port_queue_config[unit][line_port].fabric_port,
                                                   &port_queue_config[unit][line_port].fabric_dq_base);
        if (status != SOC_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: bcm_c3_cosq_dest_port_from_sq_get failed: %s\n"),
                     FUNCTION_NAME(), bcm_errmsg(status)));
        }
        status = bcm_c3_cosq_src_port_from_dq_get(unit,
                                                   port_queue_config[unit][line_port].line_dq_base,
                                                   &port_queue_config[unit][line_port].fabric_port,
                                                   &port_queue_config[unit][line_port].fabric_sq_base);
        if (status != SOC_E_NONE) {
            LOG_CLI((BSL_META_U(unit,
                                "%s: bcm_c3_cosq_dest_port_from_dq_get failed: %s\n"),
                     FUNCTION_NAME(), bcm_errmsg(status)));
        }
    }

#if CALADAN3_DEBUG_PORT_CONFIG_INSTALL
    for (line_port = 0; line_port < SOC_SBX_CALADAN3_MAX_LINE_PORT; line_port++) {
        if (!interface_config[unit][line_port].valid) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            
                            "%s: port %d, encaps %d, speed %d, phy_port %d, if_type %d\n"),
                 FUNCTION_NAME(),
                 line_port,
                 interface_config[unit][line_port].encaps,
                 interface_config[unit][line_port].speed,
                 interface_config[unit][line_port].phy_port,
                 interface_config[unit][line_port].if_type));
    }
    for (line_port = 0; line_port < SOC_SBX_CALADAN3_MAX_LINE_PORT; line_port++) {
        if (!interface_config[unit][line_port].valid) {
            continue;
        }
        LOG_CLI((BSL_META_U(unit,
                            
                            "%s: port %d, line sq, %d line dq %d, fabric sq %d, fabric dq %d, fabric port %d\n"),
                 FUNCTION_NAME(),
                 line_port,
                 port_queue_config[unit][line_port].line_sq_base,
                 port_queue_config[unit][line_port].line_dq_base,
                 port_queue_config[unit][line_port].fabric_sq_base,
                 port_queue_config[unit][line_port].fabric_dq_base,
                 port_queue_config[unit][line_port].fabric_port));
    }
#endif

    /* Set the reconfig and hotswap flags */
    SOC_CONTROL(unit)->reconfig_flag = TRUE;
    SOC_CONTROL(unit)->hotswap_flag = TRUE;

#ifndef BCM_CALADAN3_SIM
    status = system_shutdown(unit, 0);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: system_shutdown failed: %s\n"),
                 FUNCTION_NAME(), bcm_errmsg(status)));
    }
#endif

    status = soc_sbx_counter_detach(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: soc_sbx_counter_detach FAILED: %s\n"),
                 FUNCTION_NAME(), soc_errmsg(status)));
    }

    SOC_CONTROL(unit)->interface_config = interface_config[unit];
    SOC_CONTROL(unit)->port_queue_config = port_queue_config[unit];
    status = soc_sbx_caladan3_hotswap(unit);

    status = bcm_port_init(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: bcm_port_init failed: %s\n"),
                 FUNCTION_NAME(), bcm_errmsg(status)));
    }

    status = bcm_stg_init(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: bcm_stg_init failed: %s\n"),
                 FUNCTION_NAME(), bcm_errmsg(status)));
    }

    status = bcm_vlan_init(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: bcm_vlan_init failed: %s\n"),
                 FUNCTION_NAME(), bcm_errmsg(status)));
    }

    status = bcm_stat_init(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: bcm_stat_init failed: %s\n"),
                 FUNCTION_NAME(), bcm_errmsg(status)));
    }

    status = bcm_linkscan_init(unit);
    if (status != SOC_E_NONE) {
        LOG_CLI((BSL_META_U(unit,
                            "%s: bcm_linkscan_init failed: %s\n"),
                 FUNCTION_NAME(), bcm_errmsg(status)));
    }

    /* Clear flags */
    SOC_CONTROL(unit)->reconfig_flag = FALSE;
    SOC_CONTROL(unit)->hotswap_flag = FALSE;

    /* Clear hotswap flag TBD */

    SOC_PBMP_CLEAR(SOC_CONTROL(unit)->all_skip_pbm);
    SOC_PBMP_CLEAR(SOC_CONTROL(unit)->mac_phy_skip_pbm);

    return BCM_E_NONE;
}


#ifdef BCM_WARM_BOOT_SUPPORT

#define _BCM_SYNC_SUCCESS(unit) \
        (BCM_SUCCESS(rv) || (BCM_E_INIT == rv) || (BCM_E_UNAVAIL == rv))
int
_bcm_caladan3_switch_control_sync(int unit, int arg)
{
    int rv = BCM_E_NONE;
    
    LOG_CLI((BSL_META_U(unit,
                        "***WARM_BOOT_TODO: %s\n"),__FUNCTION__));

    WB_TIME_STAMP_START0

    /* SOC SYNC */
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = soc_caladan3_sw_db_sync(unit, arg);
        WB_TIME_STAMP(1, "soc_caladan3_sw_db_sync")
    }

    /* BCM SYNC */
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_init_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_init_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_allocator_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_allocator_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_trunk_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_trunk_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_vswitch_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_vswitch_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_vlan_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_vlan_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_l2_state_sync(unit, arg);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_l2_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_l2_cache_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_l2_cache_state_sync")
    }

    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_l3_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_l3_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_mpls_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_mpls_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = bcm_caladan3_wb_mim_state_sync(unit);
        WB_TIME_STAMP(1, "bcm_caladan3_wb_mim_state_sync")
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        rv = bcm_caladan3_wb_field_state_sync(unit);
    }
    if (_BCM_SYNC_SUCCESS(rv)) {
        rv = bcm_caladan3_wb_mirror_state_sync(unit);
    }

    /* Now send all data to the persistent storage */
    if (_BCM_SYNC_SUCCESS(rv)) {
        WB_TIME_STAMP_START1
        rv = soc_scache_commit(unit);
        WB_TIME_STAMP(1, "soc_scache_commit")
    }
    /* Mark scache as clean */
    SOC_CONTROL_LOCK(unit);
    SOC_CONTROL(unit)->scache_dirty = 0;
    SOC_CONTROL_UNLOCK(unit);

    WB_TIME_STAMP(0, "--- total warmboot sync ---")
    return rv;
}
#endif /* BCM_WARM_BOOT_SUPPORT */

int
_bcm_caladan3_g3p1_switch_control_set(int unit, bcm_switch_control_t type, 
                                    int arg)
{
    int                 i, rv = BCM_E_NONE, num_mapped = MAX_MAPPED;
    bcm_gport_t         gport;
    uint32            qe_mod, qe_port, node, qid = 0, temp;
    uint8             bDrop, uCos;
    sbx_sw_ctrl_desc_t  desc[MAX_MAPPED];
    soc_sbx_g3p1_ft_t   cpu_ft;
    soc_sbx_g3p1_xt_t   xt;
    uint32 exc_idx;

    switch (type) {

#ifdef BCM_WARM_BOOT_SUPPORT
    case bcmSwitchControlAutoSync:
    case bcmSwitchControlSync:
        rv = _bcm_caladan3_switch_control_sync(unit, arg);
        break;
    case bcmSwitchStableSelect:
        rv = soc_stable_set(unit, arg, 0);
        break;
    case bcmSwitchStableSize:
        rv = soc_stable_size_set(unit, arg);
        break;
    case bcmSwitchWarmBoot:
        /* If true, set the Warm Boot state; clear otherwise */
        if (arg) {
            SOC_WARM_BOOT_START(unit);
            rv = BCM_E_NONE;
        } else {
            SOC_WARM_BOOT_DONE(unit);
                    }
        break;

#else  /* BCM_WARM_BOOT_SUPPORT */
    case bcmSwitchControlAutoSync:
    case bcmSwitchControlSync:
    case bcmSwitchStableSelect:
    case bcmSwitchStableSize:
    case bcmSwitchStableUsed:
    case bcmSwitchWarmBoot:
        rv = BCM_E_UNAVAIL;
        break;
#endif  /* BCM_WARM_BOOT_SUPPORT */

    case bcmSwitchCpuProtoBpduPriority:
        uCos = arg;
        if (uCos >= sbx_num_cosq[unit]) {
            return BCM_E_PARAM;
        }
        rv = soc_sbx_g3p1_exc_l2cp_copy_idx_get(unit, &exc_idx);
        if (BCM_SUCCESS(rv)) {
            rv = _bcm_g3p1_exception_cos_set(unit, exc_idx, uCos);
        }
        break;
    case bcmSwitchDosAttackToCpu:
        bDrop = !arg;
        if (_drop[unit] != bDrop) {
            /* iterate over all G2 types that were previously enabled and
             * change the drop preference.
             */
            rv = _bcm_g3p1_drop_on_exception_set(unit, bDrop);

            /* set the global drop preference for future 'sets */
            _drop[unit] = bDrop;
        }
        break;
    case bcmSwitchUnknownIpmcToCpu:
    case bcmSwitchL3UrpfFailToCpu:
        rv = BCM_E_UNAVAIL;
        break;
    case bcmSwitchCpuCopyDestination:
        gport = arg;
        qe_mod = BCM_GPORT_MODPORT_MODID_GET(gport);
        qe_port = BCM_GPORT_MODPORT_PORT_GET(gport);
        if (qe_mod == -1) {
            rv = BCM_E_PARAM;
        }
        /* check for switch gport*/
        if (!BCM_STK_MOD_IS_NODE(qe_mod)) {
            if (qe_mod < SBX_MAX_MODIDS) {
                temp = SOC_SBX_CONTROL(unit)->modport[qe_mod][qe_port];
                qe_port = temp & 0xffff;
                qe_mod = (temp >> 16) & 0xffff;
            } else {
                rv = BCM_E_PARAM;
            }
        }
        
        if (BCM_SUCCESS(rv)) {
            node = BCM_STK_MOD_TO_NODE(qe_mod);
            /* assume maximum number of cos levels to CPU */
            qid = SOC_SBX_NODE_PORT_TO_QID(unit,node, qe_port, NUM_COS(unit));
            
            rv = _bcm_caladan3_g3p1_switch_exc_table_set(unit, qid, TRUE);
        }

        /* set the cpu fte */
        if (BCM_SUCCESS(rv)) {
            rv = soc_sbx_g3p1_ft_get(unit, SBX_CPU_FTE(unit), &cpu_ft);
            if (BCM_SUCCESS(rv)) {
                cpu_ft.qid = qid;
                cpu_ft.lag = 0;
                cpu_ft.lagbase = 0;
                cpu_ft.lagsize = 0;

                rv = soc_sbx_g3p1_ft_set(unit, SBX_CPU_FTE(unit), &cpu_ft);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_SWITCH,
                              (BSL_META_U(unit,
                                          "Set fti=%d failed: %d %s\n"), 
                               SBX_CPU_FTE(unit), rv, bcm_errmsg(rv)));
                }
            } else { 
                LOG_ERROR(BSL_LS_BCM_SWITCH,
                          (BSL_META_U(unit,
                                      "get fti=%d failed: %d %s\n"), 
                           SBX_CPU_FTE(unit), rv, bcm_errmsg(rv)));
            }
        }

        break;
    case bcmSwitchL2Cache:
        if (arg) {
            SOC_SBX_STATE(unit)->cache_l2 = TRUE;
        }else{
            SOC_SBX_STATE(unit)->cache_l2 = FALSE;
        }
        break;
    case bcmSwitchL2Commit: /* C3 doesn't use batching. */
        break;
    case bcmSwitchL2AgeDelete:
    {
        if (arg) {
            SOC_SBX_STATE(unit)->l2_age_delete = TRUE;
        }else{
            SOC_SBX_STATE(unit)->l2_age_delete = FALSE;
        }
        break;
    }
    case bcmSwitchIpmcCache:
        if (arg) {
            SOC_SBX_STATE(unit)->cache_ipmc = TRUE;
        }else{
            SOC_SBX_STATE(unit)->cache_ipmc = FALSE;
        }
         break;
    case bcmSwitchIpmcCommit:
        /* force flush of any cached changes */
        _bcm_caladan3_ipmc_flush_cache(unit);
         break;
    case bcmSwitchL3HostCommit:
        /* force flush of any cached changes */
        _bcm_caladan3_l3_flush_cache(unit, arg);
        break;
    case bcmSwitchL3RouteCache:
        if (arg) {
            SOC_SBX_STATE(unit)->cache_l3route = TRUE;
        }else{
            SOC_SBX_STATE(unit)->cache_l3route = FALSE;
        }
        /* Tell the TAPs driver to cache or not */
        taps_set_caching(unit, arg);
        break;
    case bcmSwitchL3RouteCommit:
        /* force flush of any cached changes */
        rv= _bcm_caladan3_l3_flush_cache(unit, arg);
        break;

    case bcmSwitchReserveLowL3InterfaceId:
        if (arg >= BCM_CALADAN3_L3_INTF_BASE) {
            arg = _CALADAN3_IFID_FROM_USER_HANDLE(arg);
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_IFID,
                                        FALSE, (uint32)arg);
        } else {
            rv = BCM_E_PARAM;
        }
        break;

    case bcmSwitchReserveHighL3InterfaceId:
        if (arg >= BCM_CALADAN3_L3_INTF_BASE) {
            arg = _CALADAN3_IFID_FROM_USER_HANDLE(arg);
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_IFID,
                                        TRUE, (uint32)arg);
        } else {
            rv = BCM_E_PARAM;
        }
        break;

    case bcmSwitchReserveLowL3EgressId:
        if (arg >= BCM_CALADAN3_L3_FTE_BASE) {
            arg = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(arg);
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_L3,
                                        FALSE, (uint32)arg);
        } else {
            rv = BCM_E_PARAM;
        }
        break;

    case bcmSwitchReserveHighL3EgressId:
        if (arg >= BCM_CALADAN3_L3_FTE_BASE) {
            arg = _CALADAN3_GET_FTE_IDX_FROM_USER_HANDLE(arg);
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_L3,
                                        TRUE, (uint32)arg);
        } else {
            rv = BCM_E_PARAM;
        }
        break;

    case bcmSwitchReserveLowVlanPort:
        if (arg) {
            arg = BCM_GPORT_VLAN_PORT_ID_GET(arg);
            arg = VLAN_VGPORT_ID_TO_FT_INDEX(unit, arg);
        }
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT,
                                    FALSE, (uint32)arg);
        break;

    case bcmSwitchReserveHighVlanPort:
        if (arg) {
            arg = BCM_GPORT_VLAN_PORT_ID_GET(arg);
            arg = VLAN_VGPORT_ID_TO_FT_INDEX(unit, arg);
        }
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT,
                                    TRUE, (uint32)arg);
        break;

    case bcmSwitchReserveLowMplsPort:
        /* convert FTI based gportId to the VSI */
        
        if (arg) {
            arg = BCM_GPORT_MPLS_PORT_ID_GET(arg);
        }
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_VPWS_UNI_GPORT,
                                        FALSE, (uint32)arg);
        break;

    case bcmSwitchReserveHighMplsPort:
        /* convert FTI based gportId to the VSI */
            
            if (arg) {
                arg = BCM_GPORT_MPLS_PORT_ID_GET(arg);
            }
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_VPWS_UNI_GPORT,
                                        TRUE, (uint32)arg);
        break;

#define SBX_GLOBAL_GPORT_SIZE(unit) (SBX_GLOBAL_GPORT_FTE_END(unit) - SBX_GLOBAL_GPORT_FTE_BASE(unit))
    case bcmSwitchReserveLowMimPort:
        /* verify arg */
        if ((arg < 0) || (arg > SBX_GLOBAL_GPORT_SIZE(unit))) {
            rv = BCM_E_PARAM;
        } else {
            if (arg) {
                /* convert to 0 based mim port id */
                arg += SBX_GLOBAL_GPORT_FTE_BASE(unit); 
            }
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                        FALSE, (uint32)arg);
        }
        break;
    case bcmSwitchReserveHighMimPort:
        /* verify arg */
        if ((arg < 0) || (arg > SBX_GLOBAL_GPORT_SIZE(unit))) {
            rv = BCM_E_PARAM;
        } else {
            if (arg) {
                /* convert to 0 based mim port id */
                arg += SBX_GLOBAL_GPORT_FTE_BASE(unit);
            }
            rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                        TRUE, (uint32)arg);
        }
        break;
    case bcmSwitchReserveLowEncap:
        if (arg) {
            arg = SOC_SBX_OHI_FROM_ENCAP_ID(arg);
        }
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_OHI,
                                    FALSE, (uint32)arg);
        break;
    case bcmSwitchReserveHighEncap:
        if (arg) {
            arg = SOC_SBX_OHI_FROM_ENCAP_ID(arg);
        }
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_OHI,
                                    TRUE, (uint32)arg);
        break;
    case bcmSwitchReserveLowL2Egress:
        rv = bcm_caladan3_l2_egress_range_reserve(unit, FALSE, (uint32)arg);
        break;
    case bcmSwitchReserveHighL2Egress:
        rv = bcm_caladan3_l2_egress_range_reserve(unit, TRUE, (uint32)arg);
        break;
    case bcmSwitchReserveLowVpn:
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_VSI,
                                    FALSE, (uint32)arg);

        break;
    case bcmSwitchReserveHighVpn:
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_VSI,
                                    TRUE, (uint32)arg);
        break;
    case bcmSwitchReserveLowFailoverId:
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                    FALSE, (uint32)arg);
        break;
    case bcmSwitchReserveHighFailoverId:
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                    TRUE, (uint32)arg);
        break;
    case bcmSwitchReserveLowOamEndPointId:
        rv = bcm_caladan3_oam_endpoint_range_reserve(unit, FALSE, (uint32)arg);
       break;
    case bcmSwitchReserveHighOamEndPointId:
        rv = bcm_caladan3_oam_endpoint_range_reserve(unit, TRUE, (uint32)arg);
        break;
    case bcmSwitchReserveLowOamGroupId:
        rv = bcm_caladan3_oam_group_range_reserve(unit, FALSE, (uint32)arg);
        break;
    case bcmSwitchReserveHighOamGroupId:
        rv = bcm_caladan3_oam_group_range_reserve(unit, TRUE, (uint32)arg);
        break;
    case bcmSwitchSynchronousPortClockSource:
        rv = BCM_E_UNAVAIL;
        break;
    case bcmSwitchIgmpPktToCpu:
        BCM_IF_ERROR_RETURN(soc_sbx_g3p1_exc_igmp_idx_get(unit, &exc_idx));
        soc_sbx_g3p1_xt_t_init(&xt);
        rv = soc_sbx_g3p1_xt_get(unit, exc_idx, &xt);
        if (rv == BCM_E_NONE) {
            if (arg) {
                xt.forward = TRUE;
            } else {
                xt.forward = FALSE;
            }
            rv = soc_sbx_g3p1_xt_set(unit, exc_idx, &xt);
        }
        break;
    case bcmSwitchIgmpPktDrop:
        rv = soc_sbx_g3p1_igmp_proxy_mode_set(unit, (uint32)arg);
        break;
/*
 *  MLD snooping is controlled by IGMP snooping control: bcmSwitchIgmpPktDrop
 *
    case bcmSwitchMldPktDrop:
        rv = soc_sbx_g3p1_igmp_proxy_mode_set(unit, (uint32)arg);
        break;
 */
    case bcmSwitchMeterAdjust:
        rv = soc_sbx_g3p1_switch_meter_adjust_set(unit, (uint32)arg);
        break;
    case bcmSwitchMplsLabelCache:
        if (arg == 0) {
            arg = 1;
        } else {
            arg = 0;
        }
        rv = _bcm_caladan3_mpls_label_commit_enable_set(unit, (uint32)arg);
        break;
    case bcmSwitchMplsLabelCommit:
        rv = _bcm_caladan3_mpls_label_commit(unit);
        break;

    case bcmSwitchControlPortConfigInstall:
        if (SOC_IS_CALADAN3_REVB(unit)) {
            rv = _bcm_caladan3_port_config_install(unit);
        } else {
            rv = BCM_E_UNAVAIL;
        }
        break;

    case bcmSwitchReserveLowTunnelId:
        arg = BCM_GPORT_TUNNEL_ID_GET(arg);
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_TUNNEL_ID, FALSE, (uint32)arg);

        break;
    case bcmSwitchReserveHighTunnelId:
        arg = BCM_GPORT_TUNNEL_ID_GET(arg);
        rv = _sbx_caladan3_range_reserve(unit, SBX_CALADAN3_USR_RES_TUNNEL_ID, TRUE, (uint32)arg);
        break;

    default:
        /* Get mapping of bcm switch control broad shield type to
         * Caladan3 exceptions
         */
        rv = _bcm_to_caladan3_broad_shield_type_get(unit, type, desc, &num_mapped);

        for (i=0; i<num_mapped; i++) {

            if (REG_TYPE(desc[i])) {
                _rmw_reg(unit, &desc[i], (uint32)arg);

            } else if (BOOL_TYPE(desc[i]) || CPU_TYPE(desc[i])) {

                /* CPU type is assumed to be ON, instead use the caller
                 * supplied argument to specify the drop preference in the
                 * exception table
                 *
                 * BOOL type uses the global _drop preference and the caller
                 * supplied argument is used to enable/disable the check
                 */
                rv = soc_sbx_caladan3_ppe_broad_shield_check_set(unit, 
								 desc[i].sb_type,
								 arg);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }

                if (BOOL_TYPE(desc[i])) {
                    bDrop = _drop[unit];

                } else {
                    /* CPU type */
                    bDrop = !arg; /* == toCpu */
                }

                rv = _bcm_g3p1_exception_drop_set(unit, desc[i].sb_type, bDrop);
                if (BCM_FAILURE(rv)) {
                    return rv;
                }
                
            } else {
                return BCM_E_INTERNAL;
            }
        }
    }
    return rv;
}

int
_bcm_caladan3_g3p1_switch_control_get(int unit, bcm_switch_control_t type,
                                    int *arg)
{
    int                     rv = BCM_E_NONE, num_mapped = MAX_MAPPED;
    uint32                  junk;
    soc_sbx_g3p1_ft_t       cpu_ft;
    int                     modid, qe_node, qe_port;
    uint8                   bEnable = 0, uCos;
    sbx_sw_ctrl_desc_t      desc[MAX_MAPPED];
    soc_sbx_g3p1_xt_t       xt;
    uint32 exc_idx;

    switch (type) {
#ifdef BCM_WARM_BOOT_SUPPORT
    case bcmSwitchControlAutoSync:
    case bcmSwitchControlSync:
        rv = BCM_E_NONE;
        break;
    case bcmSwitchStableSelect:
        {
            uint32 flags;
            rv = soc_stable_get(unit, arg, &flags);
        }
        break;
    case bcmSwitchStableSize:
        rv = soc_stable_size_get(unit, arg);
        break;
    case bcmSwitchStableUsed:
        rv = soc_stable_used_get(unit, arg);
        break;
    case bcmSwitchWarmBoot:
        *arg = SOC_WARM_BOOT(unit);
        break;

#else  /* BCM_WARM_BOOT_SUPPORT */
    case bcmSwitchControlAutoSync:
    case bcmSwitchControlSync:
    case bcmSwitchStableSelect:
    case bcmSwitchStableSize:
    case bcmSwitchStableUsed:
    case bcmSwitchWarmBoot:
        rv = BCM_E_UNAVAIL;
        break;
#endif  /* BCM_WARM_BOOT_SUPPORT */


    case bcmSwitchCpuProtoBpduPriority:
        rv = soc_sbx_g3p1_exc_l2cp_copy_idx_get(unit, &exc_idx);
        if (BCM_SUCCESS(rv)) {
            rv = _bcm_g3p1_exception_cos_get(unit, exc_idx, &uCos);
            if (BCM_SUCCESS(rv)) {
                *arg = uCos;
            }
        }
        break;
    case bcmSwitchDosAttackToCpu:
        *arg = !_drop[unit];
        break;
    case bcmSwitchL3UrpfFailToCpu:
    case bcmSwitchUnknownIpmcToCpu:
        rv = BCM_E_UNAVAIL;
        break;
    case bcmSwitchCpuCopyDestination:
        rv = soc_sbx_g3p1_ft_get(unit, SBX_CPU_FTE(unit), &cpu_ft);
        if (SOC_SUCCESS(rv)) {
            SOC_SBX_NODE_PORT_FROM_QID(unit, cpu_ft.qid, qe_node, qe_port, 
                                       SBX_MAX_COS);
            modid = BCM_STK_NODE_TO_MOD(qe_node);
            BCM_GPORT_MODPORT_SET(*arg, modid, qe_port);
        }
        break;
    case bcmSwitchL2Cache:
        if (SOC_SBX_STATE(unit)->cache_l2) {
            *arg = TRUE;
        }else{
            *arg = FALSE;
        }
        break;
    case bcmSwitchL2Commit:
        *arg = FALSE;
        break;
    case bcmSwitchL2AgeDelete:
        if (SOC_SBX_STATE(unit)->l2_age_delete) {
            *arg = TRUE;
        }else{
            *arg = FALSE;
        }
        break;
    case bcmSwitchIpmcCache:
        if (SOC_SBX_STATE(unit)->cache_ipmc) {
            *arg = TRUE;
        } else {
            *arg = FALSE;
        }
        break;
    case bcmSwitchL3HostCommit:
        *arg = FALSE;
        break;
    case bcmSwitchL3RouteCache:
        if (SOC_SBX_STATE(unit)->cache_l3route) {
            *arg = TRUE;
        }else{
            *arg = FALSE;
        }
        break;
    case bcmSwitchL3RouteCommit:
        *arg = FALSE;
        break;

    case bcmSwitchReserveLowL3InterfaceId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_IFID,
                                      (uint32*)arg, &junk);
        *arg = _CALADAN3_USER_HANDLE_FROM_IFID(*arg);
        break;

    case bcmSwitchReserveHighL3InterfaceId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_IFID,
                                      &junk, (uint32*)arg);
        *arg = _CALADAN3_USER_HANDLE_FROM_IFID(*arg);
        break;

    case bcmSwitchReserveLowL3EgressId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_L3,
                                      (uint32*)arg, &junk);
        *arg = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(*arg);
        break;

    case bcmSwitchReserveHighL3EgressId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_L3,
                                      &junk, (uint32*)arg);
        *arg = _CALADAN3_GET_USER_HANDLE_FROM_FTE_IDX(*arg);
        break;

    case bcmSwitchReserveLowVlanPort:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT,
                                      (uint32*)arg, &junk);
        *arg = VLAN_FT_INDEX_TO_VGPORT_ID(unit, *arg);
        BCM_GPORT_VLAN_PORT_ID_SET(*arg, *arg);
        break;

    case bcmSwitchReserveHighVlanPort:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT,
                                      &junk, (uint32*)arg);
        *arg = VLAN_FT_INDEX_TO_VGPORT_ID(unit, *arg);
        BCM_GPORT_VLAN_PORT_ID_SET(*arg, *arg);
        break;

    case bcmSwitchReserveLowMplsPort:
            
            rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_VPWS_UNI_GPORT,
                                          (uint32*)arg, &junk);
        BCM_GPORT_MPLS_PORT_ID_SET(*arg, *arg);
        break;
    case bcmSwitchReserveHighMplsPort:
            
            rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_VPWS_UNI_GPORT,
                                          &junk, (uint32*)arg);
        BCM_GPORT_MPLS_PORT_ID_SET(*arg, *arg);
        break;
    case bcmSwitchReserveLowMimPort:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                      (uint32*)arg, &junk);
        *arg -= SBX_GLOBAL_GPORT_FTE_BASE(unit); /* convert to 0 based mim port id */
        break;
    case bcmSwitchReserveHighMimPort:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_FTE_GLOBAL_GPORT,
                                      &junk, (uint32*)arg);
        *arg -= SBX_GLOBAL_GPORT_FTE_BASE(unit); /* convert to 0 based mim port id */
        break;
    case bcmSwitchReserveLowEncap:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_OHI,
                                      (uint32*)arg, &junk);
        *arg = SOC_SBX_ENCAP_ID_FROM_OHI(*arg);
        break;
    case bcmSwitchReserveHighEncap:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_OHI,
                                      &junk, (uint32*)arg);
        *arg = SOC_SBX_ENCAP_ID_FROM_OHI(*arg);
        break;
    case bcmSwitchReserveLowL2Egress:
        rv = bcm_caladan3_l2_egress_range_get(unit, (uint32*)arg, &junk);
        break;
    case bcmSwitchReserveHighL2Egress:
        *arg = SOC_SBX_OFFSET_FROM_L2_ENCAP_ID(*arg);
        rv = bcm_caladan3_l2_egress_range_get(unit, &junk, (uint32*)arg);
        break;
    case bcmSwitchReserveLowVpn:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_VSI,
                                      (uint32*)arg, &junk);
        break;
    case bcmSwitchReserveHighVpn:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_VSI,
                                      &junk, (uint32*)arg);
        break;
    case bcmSwitchReserveLowFailoverId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                    (uint32*)arg, &junk);
        break;
    case bcmSwitchReserveHighFailoverId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_PROTECTION,
                                    &junk, (uint32*)arg);
        break;
    case bcmSwitchReserveLowOamEndPointId:
        rv = bcm_caladan3_oam_endpoint_range_get(unit, (uint32*)arg, &junk);
        break;
    case bcmSwitchReserveHighOamEndPointId:
        rv = bcm_caladan3_oam_endpoint_range_get(unit, &junk, (uint32*)arg);
        break;
    case bcmSwitchReserveLowOamGroupId:
        rv = bcm_caladan3_oam_group_range_get(unit, (uint32*)arg, &junk);
        break;
    case bcmSwitchReserveHighOamGroupId:
        rv = bcm_caladan3_oam_group_range_get(unit, &junk, (uint32*)arg);
        break;
    case bcmSwitchSynchronousPortClockSource:
        rv = BCM_E_UNAVAIL;

        break;
    case bcmSwitchIgmpPktToCpu:
        rv = soc_sbx_g3p1_exc_igmp_idx_get(unit, &exc_idx);
        if (rv != BCM_E_NONE) {
            break;
        }
        soc_sbx_g3p1_xt_t_init(&xt);
        rv = soc_sbx_g3p1_xt_get(unit, exc_idx, &xt);
        if (rv == BCM_E_NONE) {
            *arg = (xt.forward && (xt.qid == SBX_EXC_QID_BASE(unit)));
        }
        break;
    case bcmSwitchIgmpPktDrop:
        rv = soc_sbx_g3p1_igmp_proxy_mode_get(unit, (uint32*)arg);
        break;
/*
 *  MLD snooping is controlled by IGMP snooping control
 *
    case bcmSwitchMldPktDrop:
        rv = soc_sbx_g3p1_igmp_proxy_mode_get(unit, (uint32*)arg);
        break;
 */
    case bcmSwitchMeterAdjust:
        rv = soc_sbx_g3p1_switch_meter_adjust_get(unit, (uint32*)arg);
        break;
    case bcmSwitchMplsLabelCache:
        rv = _bcm_caladan3_mpls_label_commit_enable_get(unit, (uint32*)arg);
        if (*arg == 0) {
            *arg = 1;
        } else {
            *arg = 0;
        }
        break;
    case bcmSwitchReserveLowTunnelId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_TUNNEL_ID, (uint32*)arg, &junk);
        BCM_GPORT_TUNNEL_ID_SET(*arg, *arg);

        break;
    case bcmSwitchReserveHighTunnelId:
        rv = _sbx_caladan3_alloc_range_get(unit, SBX_CALADAN3_USR_RES_TUNNEL_ID, &junk, (uint32*)arg);
        BCM_GPORT_TUNNEL_ID_SET(*arg, *arg);
        break;

    default:
        rv = _bcm_to_caladan3_broad_shield_type_get(unit, type, desc, &num_mapped);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
	if (num_mapped == 0) {
	    return BCM_E_PARAM;
	}

        /* There may be multple SB types mapped to a single BCM type,
         * assuem they're all the same value, so only need to 'get' one
         */
        if (REG_TYPE(desc[0])) {
	  _read_reg(unit, &desc[0], (uint32*)arg);
        } else if (BOOL_TYPE(desc[0])) {
            rv = soc_sbx_caladan3_ppe_broad_shield_check_get(unit, desc[0].sb_type, 
							     &bEnable);
            if (SOC_SUCCESS(rv)) {
                *arg = bEnable;
            }
        } else if (CPU_TYPE(desc[0])) {
            rv = soc_sbx_g3p1_xt_get(unit, desc[0].sb_type, &xt);
            if (SOC_SUCCESS(rv)) {
                *arg = xt.forward; /* == toCPU */
            }
        } else {
            return BCM_E_INTERNAL;
        }
    }

    return rv;
}

int
_bcm_caladan3_g3p1_switch_control_port_set(int unit, bcm_port_t port,
                                         bcm_switch_control_t type, int arg)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g3p1_ep2e_t     ep2e;

    switch (type) {
    case bcmSwitchKeepEgressRtHdr:
        rv = soc_sbx_g3p1_ep2e_get(unit, port, &ep2e);
        if (SOC_SUCCESS(rv)) {
            ep2e.keeperh = arg;
            rv = soc_sbx_g3p1_ep2e_set(unit, port, &ep2e);
        }
        break;
    default:
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}

int
_bcm_caladan3_g3p1_switch_control_port_get(int unit, bcm_port_t port, 
                                         bcm_switch_control_t type, int *arg)
{
    int                     rv = BCM_E_NONE;
    soc_sbx_g3p1_ep2e_t     ep2e;
    
    switch (type) {
    case bcmSwitchKeepEgressRtHdr:
        rv = soc_sbx_g3p1_ep2e_get(unit, port, &ep2e);
        if (SOC_SUCCESS(rv)) {
            *arg = ep2e.keeperh;
        }
        break;
    default:
        rv = BCM_E_UNAVAIL;
    }

    return rv;
}
#endif /* BCM_CALADAN3_G3P1_SUPPORT */

int
_bcm_caladan3_switch_control_init(int unit)
{
    int rv = BCM_E_INTERNAL;

   switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_switch_control_init(unit);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
   default:
       SBX_UNKNOWN_UCODE_WARN(unit);
       rv = BCM_E_INTERNAL;
   }
    /* coverity [overrun-buffer-arg] */
    sal_memset(&port_queue_config[unit], 0, sizeof(port_queue_config));

    return rv;
}

int
bcm_caladan3_switch_control_set(int unit, bcm_switch_control_t type, 
                              int arg)
{
    int rv = BCM_E_INTERNAL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        /* We don't believe this common code is needed for c3 */
        /*  rv = bcm_sbx_switch_control_set(unit, type, arg); */
        rv = _bcm_caladan3_g3p1_switch_control_set(unit, type, arg);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}

int
bcm_caladan3_switch_control_get(int unit, bcm_switch_control_t type, 
                              int *arg)
{
    int rv = BCM_E_INTERNAL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_switch_control_get(unit, type, arg);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
#ifdef BCM_WARM_BOOT_SUPPORT
        if(!SOC_WARM_BOOT(unit))
        {
#endif  /* BCM_WARM_BOOT_SUPPORT */
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
#ifdef BCM_WARM_BOOT_SUPPORT
        }
        else
        {
            LOG_CLI((BSL_META_U(unit,
                                "WARM_BOOT_TODO: bcm_caladan3_switch_control_get() needs ucode init for this to work right\n")));
            rv = BCM_E_NONE;
        }
#endif  /* BCM_WARM_BOOT_SUPPORT */
    } 

    return rv;
}

int
bcm_caladan3_switch_control_port_set(int unit, 
                                   bcm_port_t port,
                                   bcm_switch_control_t type,
                                   int arg)
{
    int rv = BCM_E_INTERNAL;

    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_switch_control_port_set(unit, port, type, arg);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 

    return rv;
}


int
bcm_caladan3_switch_control_port_get(int unit, 
                                   bcm_port_t port,
                                   bcm_switch_control_t type,
                                   int *arg)
{
    int rv = BCM_E_INTERNAL;
    
    switch (SOC_SBX_CONTROL(unit)->ucodetype) {
#ifdef BCM_CALADAN3_G3P1_SUPPORT
    case SOC_SBX_UCODE_TYPE_G3P1:
        rv = _bcm_caladan3_g3p1_switch_control_port_get(unit, port, type, arg);
        break;
#endif /* BCM_CALADAN3_G3P1_SUPPORT */
    default:
        SBX_UNKNOWN_UCODE_WARN(unit);
        rv = BCM_E_INTERNAL;
    } 
    
    return rv;
}
