#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_multicast_fabric.c,v 1.20 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/drv.h>

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_multicast_fabric.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/port_sw_db.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */



#define JER2_ARAD_MULT_NOF_INGRESS_SHAPINGS             (2)
#define JER2_ARAD_MULT_TC_MAPPING_FABRIC_MULT_NO_IS     (16)
#define JER2_ARAD_MULT_TC_MAPPING_FABRIC_MULT_WITH_IS   (17)

/* } */

/*************
 *  MACROS   *
 *************/
/* { */

/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* } */

/*************
 * GLOBALS   *
 *************/
/* { */

/* } */

/*************
 * FUNCTIONS *
 *************/
/* { */
/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a multicast class (0..3). This multicast class will be
*     further used for egress/fabric replication.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS              tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_CLS     new_mult_cls
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_VERIFY);

  res = jer2_arad_traffic_class_verify(tr_cls_ndx);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

  /* JER2_ARAD_MULT_FABRIC_CLS_MIN may be changed and be grater than 0 */
  /* coverity[unsigned_compare : FALSE] */
  if ((new_mult_cls < JER2_ARAD_MULT_FABRIC_CLS_MIN)||
    (new_mult_cls > JER2_ARAD_MULT_FABRIC_CLS_MAX))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MULT_FABRIC_ILLEGAL_MULTICAST_CLASS_ERR, 20, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_verify()",0,0);
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a multicast class (0..3). This multicast class will be
*     further used for egress/fabric replication.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS              tr_cls_ndx,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_CLS     new_mult_cls
  )
{
  uint32
    res;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_SET_UNSAFE);
  res = jer2_arad_ipq_traffic_class_profile_map_set_unsafe(
            unit,
            SOC_CORE_ALL,
            0 /*profile_ndx*/,
            FALSE/*is_flow_profile*/,
            TRUE /*is_multicast_profile*/,
            tr_cls_ndx,
            new_mult_cls);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_set_unsafe()",0,0);
}
/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a multicast class (0..3). This multicast class will be
*     further used for egress/fabric replication.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get_unsafe(
    DNX_SAND_IN  int              unit,
    DNX_SAND_IN  JER2_ARAD_TR_CLS           tr_cls_ndx,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_CLS  *new_mult_cls
  )
{
  uint32
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_TRAFFIC_CLASS_TO_MULTICAST_CLS_MAP_GET_UNSAFE);
  DNX_SAND_CHECK_NULL_INPUT(new_mult_cls);
  res = jer2_arad_ipq_traffic_class_profile_map_get_unsafe(
            unit,
            SOC_CORE_ALL,
            0 /*profile_ndx*/,
            FALSE/*is_flow_profile*/,
            TRUE /*is_multicast_profile*/,
            tr_cls_ndx,
            new_mult_cls);
  DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_traffic_class_to_multicast_cls_map_get_unsafe()",0,0);
}
/*********************************************************************
*     This procedure configures the base queue of the
*     multicast egress/fabric.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_id
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_BASE_QUEUE_VERIFY);

  if (!jer2_arad_is_queue_valid(unit, queue_id))
  {
    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_MULT_FABRIC_ILLEGAL_NUMBER_OF_QUEUE_ERR, 10, exit);
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_base_queue_verify()",0,0);
}

/*********************************************************************
*     This procedure configures the base queue of the
*     multicast egress/fabric.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                  queue_id
  )
{
  uint32
    res;
  soc_reg_t reg;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_BASE_QUEUE_SET_UNSAFE);

  if (SOC_IS_QAX(unit)) {
      reg = TAR_FABRIC_MC_BASE_QUEUEr;
  } else {
      reg = IRR_FABRIC_MULTICAST_BASE_QUEUEr;
  }

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, reg, REG_PORT_ANY, 0, FABRIC_MC_BASE_QUEUEf,  queue_id));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_base_queue_set_unsafe()",0,0);
}

/*********************************************************************
*     This procedure configures the base queue of the
*     multicast egress/fabric.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_base_queue_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT uint32                  *queue_id
  )
{
  uint32
    res;
  soc_reg_t reg;

  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_MULT_FABRIC_BASE_QUEUE_GET_UNSAFE);

  if (SOC_IS_QAX(unit)) {
      reg = TAR_FABRIC_MC_BASE_QUEUEr;
  } else {
      reg = IRR_FABRIC_MULTICAST_BASE_QUEUEr;
  }

  DNX_SAND_CHECK_NULL_INPUT(queue_id);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, reg, REG_PORT_ANY, 0, FABRIC_MC_BASE_QUEUEf, queue_id));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_base_queue_get_unsafe()",0,0);
}

/*********************************************************************
*     Set the Fabric Multicast credit generator configuration.
*     Details: in the H file. (search for prototype)
*********************************************************************/
static int
  jer2_arad_mult_fabric_credit_source_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_INFO    *info
  )
{
    uint8
        sch_in = FALSE,
        sch_ou = FALSE;

    DNXC_INIT_FUNC_DEFS;

    DNX_MAGIC_NUM_VERIFY(info);

    sch_in = TRUE;
    sch_in = sch_in && (info->best_effort.be_sch_port[JER2_ARAD_MULTICAST_CLASS_0].be_sch_port.multicast_class_valid);
    sch_in = sch_in && (info->best_effort.be_sch_port[JER2_ARAD_MULTICAST_CLASS_1].be_sch_port.multicast_class_valid);
    sch_in = sch_in && (info->best_effort.be_sch_port[JER2_ARAD_MULTICAST_CLASS_2].be_sch_port.multicast_class_valid);
    sch_in = sch_in && (info->guaranteed.gr_sch_port.multicast_class_valid);

    sch_ou = TRUE;
    sch_ou = sch_ou && (info->credits_via_sch);

    if (((sch_in) && (!sch_ou)) ||
        ((!sch_in) && (sch_ou)))
    {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("JER2_ARAD_MULT_FABRIC_ILLEGAL_CONF_ERR")));
    }

exit:
    DNXC_FUNC_RETURN;
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
static
  int
    jer2_arad_mult_fabric_credit_source_port_get(
      DNX_SAND_IN  int                          unit,
      DNX_SAND_IN  int                          core,
      DNX_SAND_IN  JER2_ARAD_MULTICAST_CLASS         multicast_class_ndx,
      DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_PORT_INFO   *info
    )
{
    uint32
        multicast_class_valid = 0,
        tm_port,
        buffer = 0,
        base_port_tc,
        flags;
    soc_reg_t
        mcast_reg;
    uint32 
        nof_prio;
    soc_pbmp_t 
        ports_bm;
    int
        core_i;
    soc_port_t
        port;
    soc_field_t
        mcast_class_port_id_fld[JER2_ARAD_NOF_MULTICAST_CLASSES] 
            = {MCAST_GFMC_PORT_IDf, MCAST_BFMC_1_PORT_IDf, MCAST_BFMC_2_PORT_IDf, MCAST_BFMC_3_PORT_IDf},
        multicast_class_valid_fld[JER2_ARAD_NOF_MULTICAST_CLASSES] 
            = {MULTICAST_GFMC_ENABLEf, MULTICAST_BFMC_1_ENABLEf, MULTICAST_BFMC_2_ENABLEf, MULTICAST_BFMC_3_ENABLEf};

    DNXC_INIT_FUNC_DEFS;

    switch (multicast_class_ndx) {
        case JER2_ARAD_MULTICAST_CLASS_0:
        case JER2_ARAD_MULTICAST_CLASS_1:
            mcast_reg = SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r;
            break;
        case JER2_ARAD_MULTICAST_CLASS_2:
        case JER2_ARAD_MULTICAST_CLASS_3:
            mcast_reg = SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r;
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("JER2_ARAD_MULT_FABRIC_ILLEGAL_MULTICAST_CLASS_ERR")));
            break;
    }
  
    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, mcast_reg, core,  0, &buffer));

    base_port_tc = soc_reg_field_get(unit, mcast_reg, buffer, mcast_class_port_id_fld[multicast_class_ndx]);

    /* Look for match base q pair */
    DNXC_IF_ERR_EXIT(dnx_port_sw_db_valid_ports_get(unit, 0, &ports_bm));

    SOC_PBMP_ITER(ports_bm, port)
    {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core_i));

        if(core != core_i) {
            continue;
        }

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_flags_get(unit, port, &flags));
        if (DNX_PORT_IS_ELK_INTERFACE(flags) || DNX_PORT_IS_STAT_INTERFACE(flags) || DNX_PORT_IS_LB_MODEM(flags)) {
            continue;
        }

        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core_i, tm_port, &nof_prio));

        if (base_port_tc == nof_prio) {
            info->mcast_class_port_id = tm_port;
        }
    } 

    multicast_class_valid = soc_reg_field_get(unit, mcast_reg, buffer, multicast_class_valid_fld[multicast_class_ndx]);
    info->multicast_class_valid = multicast_class_valid == 1 ? TRUE : FALSE;


exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_port_set(
      DNX_SAND_IN  int                          unit,
      DNX_SAND_IN  int                          core,
      DNX_SAND_IN  JER2_ARAD_MULTICAST_CLASS         multicast_class_ndx,
      DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_PORT_INFO   *info
    )
{
    uint32
        mcast_class_port_id = info->mcast_class_port_id,
        multicast_class_valid = info->multicast_class_valid == TRUE ? 1 : 0,
        buffer = 0,
        base_port_tc;
    JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO
        shaper;
    JER2_ARAD_OFP_RATE_INFO
        ofp;
    soc_reg_t
        mcast_reg;
    soc_field_t
        mcast_class_port_id_fld[JER2_ARAD_NOF_MULTICAST_CLASSES] 
            = {MCAST_GFMC_PORT_IDf, MCAST_BFMC_1_PORT_IDf, MCAST_BFMC_2_PORT_IDf, MCAST_BFMC_3_PORT_IDf},
        multicast_class_valid_fld[JER2_ARAD_NOF_MULTICAST_CLASSES] 
            = {MULTICAST_GFMC_ENABLEf, MULTICAST_BFMC_1_ENABLEf, MULTICAST_BFMC_2_ENABLEf, MULTICAST_BFMC_3_ENABLEf};
    int
        rv;

    DNXC_INIT_FUNC_DEFS;

    jer2_arad_JER2_ARAD_OFP_RATE_INFO_clear(&ofp);
    jer2_arad_JER2_ARAD_OFP_RATES_INTERFACE_SHPR_INFO_clear(&shaper);

    switch (multicast_class_ndx) {
        case JER2_ARAD_MULTICAST_CLASS_0:
        case JER2_ARAD_MULTICAST_CLASS_1:
            mcast_reg = SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_0r;
            break;
        case JER2_ARAD_MULTICAST_CLASS_2:
        case JER2_ARAD_MULTICAST_CLASS_3:
            mcast_reg = SCH_SCH_FABRIC_MULTICAST_PORT_CONFIGURATION_REGISTER_1r;
            break;
        default:
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOC_MSG("JER2_ARAD_MULT_FABRIC_ILLEGAL_MULTICAST_CLASS_ERR")));
    }

    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, mcast_reg, core, 0, &buffer));

    if(multicast_class_valid) {
        rv = dnx_port_sw_db_tm_port_to_base_q_pair_get(unit, core, mcast_class_port_id,  &base_port_tc);  
        DNXC_IF_ERR_EXIT(rv);

        soc_reg_field_set(unit, mcast_reg, &buffer, mcast_class_port_id_fld[multicast_class_ndx], base_port_tc);
    }

    soc_reg_field_set(unit, mcast_reg, &buffer, multicast_class_valid_fld[multicast_class_ndx], multicast_class_valid);

    DNXC_IF_ERR_EXIT(soc_reg32_set(unit, mcast_reg, core, 0, buffer));

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_shaper_set(
      DNX_SAND_IN  int                              unit,
      DNX_SAND_IN  int                              core,
      DNX_SAND_IN  soc_reg_t                        reg_desc,
      DNX_SAND_IN  soc_field_t                      max_burst_f,
      DNX_SAND_IN  soc_field_t                      shaper_rate_f,
      DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_SHAPER_INFO     *info
    )
{
    uint32
        res;
    uint32
        rate = 0,
        buffer = 0;
    int 
        rv;

    DNXC_INIT_FUNC_DEFS;

    rv = soc_reg32_get(unit, reg_desc, core, 0, &buffer);
    DNXC_IF_ERR_EXIT(rv);

    soc_reg_field_set(unit ,reg_desc, &buffer, max_burst_f, info->max_burst);

    res = jer2_arad_intern_rate2clock(
          unit,
          info->rate,
          FALSE, /* is_for_ips FALSE: for FMC */
          &rate
        );
    DNX_SAND_IF_ERR_EXIT(res);

    soc_reg_field_set(unit ,reg_desc, &buffer, shaper_rate_f, rate);

    rv = soc_reg32_set(unit, reg_desc, core, 0, buffer);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_shaper_get(
      DNX_SAND_IN  int                              unit,
      DNX_SAND_IN  int                              core,
      DNX_SAND_IN  soc_reg_t                        reg_desc,
      DNX_SAND_IN  soc_field_t                      max_burst_f,
      DNX_SAND_IN  soc_field_t                      shaper_rate_f,
      DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_SHAPER_INFO     *info
    )
{
    uint32
        res;
    uint32
        rate = 0,
        buffer = 0;

    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(soc_reg32_get(unit, reg_desc, core, 0, &buffer));

    info->max_burst = soc_reg_field_get(unit , reg_desc, buffer, max_burst_f);
    rate = soc_reg_field_get(unit , reg_desc, buffer, shaper_rate_f);

    res = jer2_arad_intern_clock2rate(
          unit,
          rate,
          FALSE, /* is_for_ips FALSE: for FMC */
          &(info->rate)
        );
    DNX_SAND_IF_ERR_EXIT(res);

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_be_wfq_set(
      DNX_SAND_IN  int                          unit,
      DNX_SAND_IN  int                          core,
      DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_BE_INFO     *info
    )
{
    uint32
        wfq_enable = info->wfq_enable == TRUE ? 0x1 : 0x0,
        buffer = 0;
    int
        rv;

    DNXC_INIT_FUNC_DEFS;

    /*
    * SP vs WFQ info
    * if SP do not write WFQ. Otherwise write.
    */

    rv = READ_IPS_BFMC_CLASS_CONFIGr(unit, core, &buffer);
    DNXC_IF_ERR_EXIT(rv);

    soc_reg_field_set(unit, IPS_BFMC_CLASS_CONFIGr, &buffer, BFMC_WFQ_ENf, wfq_enable);

    if (wfq_enable)
    {
        /* note: fileds names doesn't match class names */
        soc_reg_field_set(unit, IPS_BFMC_CLASS_CONFIGr, &buffer, BFMC_CLASS_2_Wf, info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_0].weight);
        soc_reg_field_set(unit, IPS_BFMC_CLASS_CONFIGr, &buffer, BFMC_CLASS_1_Wf, info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_1].weight);
        soc_reg_field_set(unit, IPS_BFMC_CLASS_CONFIGr, &buffer, BFMC_CLASS_0_Wf, info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_2].weight);
    }

    rv = WRITE_IPS_BFMC_CLASS_CONFIGr(unit, core, buffer);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}


static
  int
    jer2_arad_mult_fabric_credit_source_be_wfq_get(
      DNX_SAND_IN  int                           unit,
      DNX_SAND_IN  int                           core,
      DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_BE_INFO       *info
    )
{
    uint32
        wfq_enable = 0,
        buffer = 0;
    int
        rv;

    DNXC_INIT_FUNC_DEFS;

    info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_0].weight = 0;
    info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_1].weight = 0;
    info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_2].weight = 0;

    /*
    * SP vs WFQ info
    * if SP do not write WFQ. Otherwise write.
    */
    rv = READ_IPS_BFMC_CLASS_CONFIGr(unit, core, &buffer);
    DNXC_IF_ERR_EXIT(rv);

    wfq_enable = soc_reg_field_get(unit, IPS_BFMC_CLASS_CONFIGr, buffer, BFMC_WFQ_ENf);

    info->wfq_enable = (wfq_enable==0x1 ? TRUE : FALSE);

    if (info->wfq_enable)
    {
        /* note: fileds names doesn't match class names */
        info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_0].weight = soc_reg_field_get(unit, IPS_BFMC_CLASS_CONFIGr, buffer, BFMC_CLASS_2_Wf);
        info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_1].weight = soc_reg_field_get(unit, IPS_BFMC_CLASS_CONFIGr, buffer, BFMC_CLASS_1_Wf);
        info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_2].weight = soc_reg_field_get(unit, IPS_BFMC_CLASS_CONFIGr, buffer, BFMC_CLASS_0_Wf);
    }

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_be_set(
      DNX_SAND_IN  int                          unit,
      DNX_SAND_IN  int                          core,
      DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_BE_INFO    *info
    )
{
    int rv;
    DNXC_INIT_FUNC_DEFS;

    /* SP and WFQ info */
    rv = jer2_arad_mult_fabric_credit_source_be_wfq_set(
          unit,
          core,
          info
        );
    DNXC_IF_ERR_EXIT(rv);

    /* Set shaper info */
    rv = jer2_arad_mult_fabric_credit_source_shaper_set(
          unit,
          core,
          IPS_BFMC_SHAPER_CONFIGr,
          BFMC_MAX_BURSTf,
          BFMC_MAX_CR_RATEf,
          &(info->be_shaper)
        );
    DNXC_IF_ERR_EXIT(rv);

    /* Set port info */
    rv = jer2_arad_mult_fabric_credit_source_port_set(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_3,
          &(info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_0].be_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_mult_fabric_credit_source_port_set(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_2,
          &(info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_1].be_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_mult_fabric_credit_source_port_set(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_1,
          &(info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_2].be_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_be_get(
      DNX_SAND_IN  int                       unit,
      DNX_SAND_IN  int                       core,
      DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_BE_INFO *info
    )
{
    int rv;

    DNXC_INIT_FUNC_DEFS;

    /* Get SP and WFQ info */
    rv = jer2_arad_mult_fabric_credit_source_be_wfq_get(
          unit,
          core,
          info
        );
    DNXC_IF_ERR_EXIT(rv);

    /* Get shaper info */
    rv = jer2_arad_mult_fabric_credit_source_shaper_get(
          unit,
          core,
          IPS_BFMC_SHAPER_CONFIGr,
          BFMC_MAX_BURSTf,
          BFMC_MAX_CR_RATEf,
          &(info->be_shaper)
        );
    DNXC_IF_ERR_EXIT(rv);

    /*
    * Get port info- inversion in the scheduler for the order
    * of importance of the multicast classes MC3 < MC2 < MC1 (best effort)
    */
    rv = jer2_arad_mult_fabric_credit_source_port_get(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_3,
          &(info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_0].be_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_mult_fabric_credit_source_port_get(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_2,
          &(info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_1].be_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_mult_fabric_credit_source_port_get(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_1,
          &(info->be_sch_port[JER2_ARAD_MULTICAST_CLASS_2].be_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_gu_set(
      DNX_SAND_IN  int                       unit,
      DNX_SAND_IN  int                       core,
      DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_GR_INFO *info
    )
{
    int rv;

    DNXC_INIT_FUNC_DEFS;

    /* Set shaper info */
    rv = jer2_arad_mult_fabric_credit_source_shaper_set(
          unit,
          core,
          IPS_GFMC_SHAPER_CONFIGr,
          GFMC_MAX_BURSTf,
          GFMC_MAX_CR_RATEf,
          &(info->gr_shaper)
        );
    DNXC_IF_ERR_EXIT(rv);

    /*
    * Set port info - inversion in the scheduler for the order of importance of the
    * multicast classes: the guaranteed multicast class is 0
    */
    rv = jer2_arad_mult_fabric_credit_source_port_set(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_0,
          &(info->gr_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

static
  int
    jer2_arad_mult_fabric_credit_source_gu_get(
      DNX_SAND_IN  int                       unit,
      DNX_SAND_IN  int                       core,
      DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_GR_INFO *info
    )
{
    int rv;

    DNXC_INIT_FUNC_DEFS;

    /* Set shaper info */
    rv = jer2_arad_mult_fabric_credit_source_shaper_get(
          unit,
          core,
          IPS_GFMC_SHAPER_CONFIGr,
          GFMC_MAX_BURSTf,
          GFMC_MAX_CR_RATEf,
          &(info->gr_shaper)
        );
    DNXC_IF_ERR_EXIT(rv);

    /*
    * Set port info- inversion in the scheduler for the order of importance of the
    * multicast classes: the guaranteed multicast class is 0
    */
    rv = jer2_arad_mult_fabric_credit_source_port_get(
          unit,
          core,
          JER2_ARAD_MULTICAST_CLASS_0,
          &(info->gr_sch_port)
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}



/*********************************************************************
*     Set the Fabric Multicast credit generator configuration
*     for the Default Fabric Multicast Queue configuration.
*     The fabric multicast queues are 0 - 3, and the credits
*     comes either directly to these queues or according to a
*     scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_mult_fabric_credit_source_set(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  int                        core,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_INFO      *info
  )
{
    int
        rv;
    uint32
        buffer,
        rate = 0,
        res;
   
    DNXC_INIT_FUNC_DEFS;

    rv = jer2_arad_mult_fabric_credit_source_verify(unit, info);
    DNXC_IF_ERR_EXIT(rv);

    res = jer2_arad_intern_rate2clock(
          unit,
          info->max_rate,
          FALSE, /* is_for_ips FALSE: for FMC */
          &rate
        );
    DNX_SAND_IF_ERR_EXIT(res);

    if (SOC_IS_QAX(unit)) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, REG_PORT_ANY, core, FMC_CREDITS_FROM_SCHf,  info->credits_via_sch));
    } else {
        DNXC_IF_ERR_EXIT(READ_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, core, &buffer));
        soc_reg_field_set(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, &buffer, FMC_CREDITS_FROM_SCHf, info->credits_via_sch);
        DNXC_IF_ERR_EXIT(WRITE_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, core, buffer));
    }


    DNXC_IF_ERR_EXIT(READ_IPS_FMC_SHAPER_CONFIGr(unit, core ,&buffer));
    soc_reg_field_set(unit, IPS_FMC_SHAPER_CONFIGr, &buffer, FMC_MAX_CR_RATEf, rate);
    soc_reg_field_set(unit, IPS_FMC_SHAPER_CONFIGr, &buffer, FMC_MAX_BURSTf, info->max_burst);
    DNXC_IF_ERR_EXIT(WRITE_IPS_FMC_SHAPER_CONFIGr(unit, core ,buffer));

    rv = jer2_arad_mult_fabric_credit_source_be_set(
          unit,
          core,
          &(info->best_effort)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_mult_fabric_credit_source_gu_set(
          unit,
          core,
          &(info->guaranteed)
        );
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Set the Fabric Multicast credit generator configuration
*     for the Default Fabric Multicast Queue configuration.
*     The fabric multicast queues are 0 - 3, and the credits
*     comes either directly to these queues or according to a
*     scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
int
  jer2_arad_mult_fabric_credit_source_get(
    DNX_SAND_IN  int                        unit,
    DNX_SAND_IN  int                        core,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_INFO      *info
  )
{
    int
        rv;
    uint32
        buffer,
        credits_via_sch = 0,
        rate = 0,
        res;
   
    DNXC_INIT_FUNC_DEFS;

    DNX_TMC_MULT_FABRIC_INFO_clear(info);

    rv = jer2_arad_mult_fabric_credit_source_gu_get(
          unit,
          core,
          &(info->guaranteed)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = jer2_arad_mult_fabric_credit_source_be_get(
          unit,
          core,
          &(info->best_effort)
        );
    DNXC_IF_ERR_EXIT(rv);

    rv = READ_IPS_FMC_SHAPER_CONFIGr(unit, core ,&buffer);
    DNXC_IF_ERR_EXIT(rv);

    rate = soc_reg_field_get(unit, IPS_FMC_SHAPER_CONFIGr, buffer, FMC_MAX_CR_RATEf);

    res = jer2_arad_intern_clock2rate(
          unit,
          rate,
          FALSE, /* is_for_ips FALSE: for FMC */
          &(info->max_rate)
        );
    DNX_SAND_IF_ERR_EXIT(res);

    info->max_burst = soc_reg_field_get(unit, IPS_FMC_SHAPER_CONFIGr, buffer, FMC_MAX_BURSTf);

    if (SOC_IS_QAX(unit)) {
        DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, REG_PORT_ANY, core, FMC_CREDITS_FROM_SCHf, &credits_via_sch));
    } else {
        rv = READ_IPS_IPS_GENERAL_CONFIGURATIONSr_REG32(unit, core, &buffer);
        DNXC_IF_ERR_EXIT(rv);

        credits_via_sch = soc_reg_field_get(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, buffer, FMC_CREDITS_FROM_SCHf);
    }
    info->credits_via_sch = (credits_via_sch ? TRUE : FALSE);

exit:
    DNXC_FUNC_RETURN;
}
/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_enhanced_set_unsafe(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  DNX_SAND_U32_RANGE                            *queue_range
  )
{
  int
    res;
  uint32
    reg_val = 0; 
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(queue_range);

  res = READ_ECI_GLOBAL_17r(unit, &reg_val);
  DNXC_IF_ERR_EXIT(res);

  soc_reg_field_set(unit, ECI_GLOBAL_17r, &reg_val, FMC_QNUM_LOWf, queue_range->start);

  res = WRITE_ECI_GLOBAL_17r(unit, reg_val);
  DNXC_IF_ERR_EXIT(res);

  res = READ_ECI_GLOBAL_18r(unit, &reg_val);
  DNXC_IF_ERR_EXIT(res);

  soc_reg_field_set(unit, ECI_GLOBAL_18r, &reg_val, FMC_QNUM_HIGHf, queue_range->end);

  res = WRITE_ECI_GLOBAL_18r(unit, reg_val);
  DNXC_IF_ERR_EXIT(res);

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_enhanced_set_verify(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  DNX_SAND_U32_RANGE                            *queue_range
  )
{
  DNXC_INIT_FUNC_DEFS;
  if (queue_range->start > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
    LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Queue start %d out of range\n"), queue_range->start));
    DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }
  if (queue_range->end > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
    LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Queue end %d out of range\n"), queue_range->end));
    DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }

  if (queue_range->start > queue_range->end) {
      LOG_ERROR(BSL_LS_SOC_STAT,(BSL_META_U(unit, "Queue start %d is higher than Queue end %d\n"), queue_range->start, queue_range->end));
      DNXC_SAND_IF_ERR_EXIT(SOC_E_PARAM);
  }

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     Configure the Enhanced Fabric Multicast Queue
*     configuration: the fabric multicast queues are defined
*     in a configured range, and the credits are coming to
*     these queues according to a scheduler scheme.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_mult_fabric_enhanced_get_unsafe(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_OUT DNX_SAND_U32_RANGE                  *queue_range
  )
{
  int
    res;
  uint32
    reg_val = 0;
  
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(queue_range);
 
  res = READ_ECI_GLOBAL_17r(unit, &reg_val);
  DNXC_IF_ERR_EXIT(res);

  queue_range->start = soc_reg_field_get(unit, ECI_GLOBAL_17r, reg_val, FMC_QNUM_LOWf);

  res = READ_ECI_GLOBAL_18r(unit, &reg_val);
  DNXC_IF_ERR_EXIT(res);

  queue_range->end = soc_reg_field_get(unit, ECI_GLOBAL_18r, reg_val, FMC_QNUM_HIGHf);

exit:
  DNXC_FUNC_RETURN;
}

uint32
  jer2_arad_mult_fabric_flow_control_set_verify(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  )
{

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_NULL_INPUT(fc_map);
  DNX_SAND_MAGIC_NUM_VERIFY(fc_map);

  if(fc_map->gfmc_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      DNX_SAND_ERR_IF_ABOVE_MAX(fc_map->gfmc_lb_fc_map, 0xf, JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR, 1, exit);
  }

  if(fc_map->bfmc0_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      DNX_SAND_ERR_IF_ABOVE_MAX(fc_map->bfmc0_lb_fc_map, 0xf, JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR, 2, exit);
  }

  if(fc_map->bfmc1_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      DNX_SAND_ERR_IF_ABOVE_MAX(fc_map->bfmc1_lb_fc_map, 0xf, JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR, 3, exit);
  }

  if(fc_map->bfmc2_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      DNX_SAND_ERR_IF_ABOVE_MAX(fc_map->bfmc2_lb_fc_map, 0xf, JER2_ARAD_QUEUE_NUM_OUT_OF_RANGE_ERR, 4, exit);
  }


exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_flow_control_set_verify()", 0, 0);
}

  
uint32
  jer2_arad_mult_fabric_flow_control_set_unsafe(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_IN  JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  )
{
  uint32 fld_value, res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  if(fc_map->gfmc_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      fld_value = fc_map->gfmc_lb_fc_map & 0xf;
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, GFMC_FC_MAPf,  fld_value));
  }

  if(fc_map->bfmc0_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      fld_value = fc_map->bfmc0_lb_fc_map & 0xf;
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, BFMC_0_FC_MAPf,  fld_value));
  }

  if(fc_map->bfmc1_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      fld_value = fc_map->bfmc1_lb_fc_map & 0xf;
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, BFMC_1_FC_MAPf,  fld_value));
  }

  if(fc_map->bfmc2_lb_fc_map != JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_DONT_MAP) {
      fld_value = fc_map->bfmc2_lb_fc_map & 0xf;
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, BFMC_2_FC_MAPf,  fld_value));
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_flow_control_set_unsafe()",0,0);
}

uint32
  jer2_arad_mult_fabric_flow_control_get_verify(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP      *fc_map
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_CHECK_NULL_INPUT(fc_map);
  DNX_SAND_MAGIC_NUM_VERIFY(fc_map);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_flow_control_set_verify()", 0, 0);
}

uint32
  jer2_arad_mult_fabric_flow_control_get_unsafe(
    DNX_SAND_IN  int                                 unit,
    DNX_SAND_OUT JER2_ARAD_MULT_FABRIC_FLOW_CONTROL_MAP     *fc_map
  )
{
  uint32 res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  1,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, GFMC_FC_MAPf, &fc_map->gfmc_lb_fc_map));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  2,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, BFMC_0_FC_MAPf, &fc_map->bfmc0_lb_fc_map));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  3,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, BFMC_1_FC_MAPf, &fc_map->bfmc1_lb_fc_map));
  DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  4,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IPS_IPT_FLOW_CONTROL_CONFIGURATIONr, SOC_CORE_ALL, 0, BFMC_2_FC_MAPf, &fc_map->bfmc2_lb_fc_map));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_mult_fabric_flow_control_get_unsafe()",0,0);
}

/*
* Function:
*      jer2_arad_multicast_table_size_get
* Purpose:
*      Get MC table size
* Parameters:
*      unit           - (IN)  Unit number.
* Returns:
*      SOC_E_xxx
*/
soc_error_t 
jer2_arad_multicast_table_size_get(int unit, uint32* mc_table_size)
{
    DNXC_INIT_FUNC_DEFS;

    *mc_table_size = SOC_DNX_CONFIG(unit)->tm.nof_mc_ids;

    DNXC_FUNC_RETURN;    
}

#undef _ERR_MSG_MODULE_NAME

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */
