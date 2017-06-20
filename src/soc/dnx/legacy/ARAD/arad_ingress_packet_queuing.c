#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#if defined(BCM_88690_A0)
/* $Id: jer2_arad_ingress_packet_queuing.c,v 1.48 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_INGRESS

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/cosq.h>
#include <soc/dnx/legacy/TMC/tmc_api_egr_queuing.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/ARAD/arad_api_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_ingress_packet_queuing.h>
#include <soc/dnx/legacy/ARAD/arad_scheduler_end2end.h>


#include <soc/dnx/legacy/ARAD/arad_reg_access.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>
#include <soc/dnx/legacy/ARAD/arad_general.h>
#include <soc/dnx/legacy/ARAD/arad_mgmt.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_chip_tbls.h>
#include <soc/dnx/legacy/ARAD/arad_debug.h>
#include <soc/dnx/legacy/mbcm.h>
#include <soc/mem.h>

#include <soc/dnx/legacy/SAND/Utils/sand_u64.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */



/*
 * Port to interface mapping register value indicating
 * unmapped interface
 */

/* Max & min values for enum JER2_ARAD_IPQ_TR_CLS_RNG:      */
#define JER2_ARAD_IPQ_TR_CLS_RNG_MIN 0
#define JER2_ARAD_IPQ_TR_CLS_RNG_MAX (JER2_ARAD_IPQ_TR_CLS_RNG_LAST-1)

/* Max & min values for struct JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO:      */
#define JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_BASE_QUEUE_ID_MIN      0

/* Max & min values for sys_physical_port_ndx:      */

/* Max & min values for dest_fap_id:      */

/* Max & min values for dest_fap_id:      */


/* Max & min values for Mapping of Queues to Flows:      */
#define JER2_ARAD_IPQ_MIN_INTERDIGIT_FLOW_QUARTET(unit)   SOC_DNX_DEFS_GET(unit, min_interdigit_flow_quartet)
    
#define JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MIN    0
#define JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_INDIRECT JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_INDIRECT
#define JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_DIRECT   JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID

#define  JER2_ARAD_INGR_QUEUE_TABLE_BYTE_RESOL 16

/* Default TC profile - used for global mapping */
#define JER2_ARAD_IPQ_TC_PROFILE_DFLT                (0)

/* two entries for Dest-Sys-Port, two entries for Dest-Flow-Id for  */
#define JER2_ARAD_IPQ_NOF_UC_DEST_TC_ENTRIES         (4)
#define JER2_ARAD_IPQ_NOF_UC_SYS_PORT_DEST_TC_ENTRIES (2)
#define JER2_ARAD_IPQ_NOF_UC_FLOW_DEST_TC_ENTRIES (2)
#define JER2_ARAD_IPQ_FLOW_ID_TC_OFFSET (8)

/* 
 * If destination is system port then is_flow_or_multicast==0
 * If is_flow then is_flow_or_multicast==1
 * If is_multicast then is_flow_or_multicast==2
 *  
 * The traffic class is mapped according to packet's source and destination for pipe 0/1.
 * Table is accessed with the following key key_msb,ingress_shape,orig_tc, where:
 * key_msb (4 bits):
 *    - 2'b00,TC-Mapping-Profile if Destination is System-Port-ID
 *    - 2'b01,Flow-Profile if Destination is Flow
 *    - 2'b10,2'b00 if Destination is fabric or egress multicast
 * ingress_shape (1 bit) is '0' before ingress shaping, and '1' after ingress shaping
 * orig_tc (3 bits) is the original traffic class
 */
#define _JER2_JER_IPQ_TC_CLS_OFF 0
#define _JER2_JER_IPQ_TC_CLS_NOF_BITS 3
#define _JER2_JER_IPQ_TC_CLS_MASK ((1 << _JER2_JER_IPQ_TC_CLS_NOF_BITS) - 1)

#define _JER2_JER_IPQ_TC_SHAPE_OFF(unit) (_JER2_JER_IPQ_TC_CLS_NOF_BITS + _JER2_JER_IPQ_TC_CLS_OFF)
#define _JER2_JER_IPQ_TC_SHAPE_NOF_BITS(unit) (SOC_IS_QAX(unit) ? 0 : 1) /* there is no ingress shaping in JER2_QAX */
#define _JER2_JER_IPQ_TC_SHAPE_MASK(unit)   (SOC_IS_QAX(unit) ? 0 : ((1 << _JER2_JER_IPQ_TC_SHAPE_NOF_BITS(unit)) - 1)) /* there is no ingress shaping in JER2_QAX */

#define _JER2_JER_IPQ_TC_PROF_OFF(unit) (_JER2_JER_IPQ_TC_SHAPE_NOF_BITS(unit) + _JER2_JER_IPQ_TC_SHAPE_OFF(unit))
#define _JER2_JER_IPQ_TC_PROF_NOF_BITS 2
#define _JER2_JER_IPQ_TC_PROF_MASK ((1 << _JER2_JER_IPQ_TC_PROF_NOF_BITS) - 1)

#define _JER2_JER_IPQ_TC_FLOW_OFF(unit) (_JER2_JER_IPQ_TC_PROF_NOF_BITS + _JER2_JER_IPQ_TC_PROF_OFF(unit))
#define _JER2_JER_IPQ_TC_FLOW_NOF_BITS 2
#define _JER2_JER_IPQ_TC_FLOW_MASK ((1 << _JER2_JER_IPQ_TC_FLOW_NOF_BITS) - 1)

#define JER2_JER_IPQ_TC_ENTRY(unit,is_flow_or_multicast, profile_ndx, is_ingress_shape ,tr_cls_ndx) \
    (\
    ((is_flow_or_multicast & _JER2_JER_IPQ_TC_FLOW_MASK)        << _JER2_JER_IPQ_TC_FLOW_OFF(unit))   | \
    ((profile_ndx          & _JER2_JER_IPQ_TC_PROF_MASK)        << _JER2_JER_IPQ_TC_PROF_OFF(unit))   | \
    ((is_ingress_shape     & _JER2_JER_IPQ_TC_SHAPE_MASK(unit)) << _JER2_JER_IPQ_TC_SHAPE_OFF(unit))  | \
    ((tr_cls_ndx           & _JER2_JER_IPQ_TC_CLS_MASK)         << _JER2_JER_IPQ_TC_CLS_OFF)            \
    )

#define JER2_ARAD_MULT_NOF_INGRESS_SHAPINGS             (2)
#define JER2_ARAD_MULT_TC_MAPPING_FABRIC_MULT_NO_IS     (16)
#define JER2_ARAD_MULT_TC_MAPPING_FABRIC_MULT_WITH_IS   (17)

#define JER2_ARAD_IPQ_MULT_TC_ENTRY(is_ingress_shape) \
                ((is_ingress_shape == 0) ? JER2_ARAD_MULT_TC_MAPPING_FABRIC_MULT_NO_IS : JER2_ARAD_MULT_TC_MAPPING_FABRIC_MULT_WITH_IS)

#define JER2_ARAD_IPQ_UC_SYS_PORT_TC_ENTRY(tm_profile, uc_ndx) \
  (2 * tm_profile + uc_ndx)

#define JER2_ARAD_IPQ_UC_FLOW_ID_TC_ENTRY(tm_profile, uc_ndx) \
  (2 * tm_profile + uc_ndx + JER2_ARAD_IPQ_FLOW_ID_TC_OFFSET)

#define JER2_ARAD_JER2_ARAD_IPQ_TC_ENTRY(is_flow_or_multicast, is_ingress_shape, profile_ndx)                           \
             ((is_flow_or_multicast == 0x2) ? (JER2_ARAD_IPQ_MULT_TC_ENTRY(is_ingress_shape)) :                    \
             ((is_flow_or_multicast == 0x1) ? (JER2_ARAD_IPQ_UC_FLOW_ID_TC_ENTRY(profile_ndx, is_ingress_shape)) : \
                                              (JER2_ARAD_IPQ_UC_SYS_PORT_TC_ENTRY(profile_ndx, is_ingress_shape))))

#define JER2_ARAD_IPQ_TC_ENTRY(unit, is_flow_or_multicast, profile_ndx, is_ingress_shape ,tr_cls_ndx) \
            (SOC_IS_ARADPLUS_AND_BELOW(unit) ?                                                   \
            (JER2_ARAD_JER2_ARAD_IPQ_TC_ENTRY(is_flow_or_multicast, is_ingress_shape, profile_ndx)) :      \
            (JER2_JER_IPQ_TC_ENTRY(unit,is_flow_or_multicast, profile_ndx, is_ingress_shape ,tr_cls_ndx))) \

/* Max system physical port is only 12 bits and not 15 */
#define JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_IN_IPS DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID /*4095*/

#define JER2_ARAD_IPQ_QUEUE_EMPTY_ITERATIONS 5
#define JER2_ARAD_DEVICE_QUEUE_RESERVED_FLOW_START SHR_DEVICE_QUEUE_RESERVED_FLOW_START

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

static uint32
jer2_arad_cosq_ips_non_empty_queues_info_get_unsafe(
   DNX_SAND_IN  int                   unit,
   DNX_SAND_IN  int                   core,
   DNX_SAND_IN  uint32                queue_id,
   DNX_SAND_OUT uint32*               is_empty
   );


/*********************************************************************
* NAME:
*     jer2_arad_ipq_init
* FUNCTION:
*     Initialization of the Arad blocks configured in this module.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the initialization sequence.
*********************************************************************/
uint32
jer2_arad_ipq_init(
   DNX_SAND_IN  int                 unit
   ) {
   uint32 res = DNX_SAND_OK;
   JER2_ARAD_IPQ_TR_CLS cls_id;
   uint32 system_red = JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ? 1 : 0;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_INIT);

   /*
    *  Traffic Class mapping -
    *    initialize to "no change"
    */
   for (cls_id = JER2_ARAD_IPQ_TR_CLS_MIN; cls_id <= JER2_ARAD_IPQ_TR_CLS_MAX; cls_id++) {
      res = jer2_arad_ipq_traffic_class_map_set_unsafe(unit, cls_id, cls_id);
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
   }
    /* Set System red enable by default */
    DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  140,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IPS_IPS_GENERAL_CONFIGURATIONSr, SOC_CORE_ALL, 0, ENABLE_SYSTEM_REDf,  system_red));

#ifdef BCM_88660_A0
   if (SOC_IS_ARADPLUS(unit))
   {
       uint32
           mcr_limit_uc;

       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  190,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IRR_MCR_FIFO_CONFIGr, REG_PORT_ANY, 0, MCR_LIMIT_UCf, &mcr_limit_uc));

       /*
        * When FC is received from IQM, use separate thresholds for snoop/mirror, and separate counters
        */
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  150,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRR_UC_FIFO_TH_CONFIGr, REG_PORT_ANY, 0, UC_FIFO_SNOOP_THRESHOLDf,  JER2_ARAD_IPQ_UC_FIFO_SNOOP_THRESHOLD(mcr_limit_uc)));
       DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  155,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRR_UC_FIFO_TH_CONFIGr, REG_PORT_ANY, 0, UC_FIFO_MIRROR_THRESHOLDf, JER2_ARAD_IPQ_UC_FIFO_MIRROR_THRESHOLD(mcr_limit_uc)));
   }
#endif /* BCM_88660_A0 */
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_init()", 0, 0);
}
/*********************************************************************
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_explicit_mapping_mode_info_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_VERIFY);

   DNX_SAND_CHECK_NULL_INPUT(info);
   DNX_SAND_MAGIC_NUM_VERIFY(info);

   /*
    * JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_BASE_QUEUE_ID_MIN may be changed and be grater then zero.
    */
   /* coverity[unsigned_compare] */
   if ((info->base_queue_id < JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_BASE_QUEUE_ID_MIN) ||
       (info->base_queue_id > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1))) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_BASE_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_explicit_mapping_mode_info_verify()", 0, 0);
}

/*********************************************************************
*     Sets the Explicit Flow Unicast packets mapping to queue.
*     Doesn't affect packets that arrive with destination_id
*     in the header.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_explicit_mapping_mode_info_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) {
   uint32
      profile_id,
      fld_val,
      res;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_SET_UNSAFE);

   DNX_SAND_CHECK_NULL_INPUT(info);

   /*
    * All the Base_Q Flow is implemented trough the IRR table:
    * See Arch-PP-Spec figure 15: per flow, a profile is got to set its TC-mapping and its Base-Flow
    * No reason for a Base-Flow per Flow-Id, so a global one is set
    */
   fld_val = info->base_queue_id;
   for (profile_id = 0; profile_id < JER2_ARAD_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES; ++profile_id) {
      DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10 + profile_id,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_modify(unit, IRR_FLOW_BASE_QUEUEr, REG_PORT_ANY,  profile_id, FLOW_BASE_QUEUE_Nf,  fld_val));
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_explicit_mapping_mode_info_set_unsafe()", 0, 0);
}

uint32
jer2_arad_ipq_explicit_mapping_mode_info_get_unsafe(
   DNX_SAND_IN  int                            unit,
   DNX_SAND_OUT JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO *info
   ) {

   uint32
      fld_val,
      res;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_GET_UNSAFE);

   jer2_arad_JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(info);

   DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,soc_reg_above_64_field32_read(unit, IRR_FLOW_BASE_QUEUEr, REG_PORT_ANY,  0, FLOW_BASE_QUEUE_Nf, &fld_val));
   info->base_queue_id = fld_val;
   info->queue_id_add_not_decrement = TRUE;

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_explicit_mapping_mode_info_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Configures Base-Q configuration to an invalid value.
*     This configuration is changed by
*     jer2_arad_ipq_explicit_mapping_mode_info_set.
*     API-s that are dependent on a valid Base-Q configuration
*     may use this value to verify Base-Q is already set.
*********************************************************************/
uint32
jer2_arad_ipq_base_q_is_valid_get_unsafe(
   DNX_SAND_IN  int  unit,
   DNX_SAND_OUT uint8  *is_valid
   ) {
   uint32
      res;
   JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO
      base_q_info;
   uint8
      is_invalid;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_BASE_Q_IS_VALID_GET_UNSAFE);

   jer2_arad_JER2_ARAD_IPQ_EXPLICIT_MAPPING_MODE_INFO_clear(&base_q_info);

   res = jer2_arad_ipq_explicit_mapping_mode_info_get_unsafe(
      unit,
      &base_q_info
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   is_invalid = DNX_SAND_NUM2BOOL(
      (base_q_info.base_queue_id == 0) &&
      (base_q_info.queue_id_add_not_decrement == FALSE)
      );

   *is_valid = !(is_invalid);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_base_q_is_valid_get_unsafe()", 0, 0);
}
/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Note that a class
*     that is mapped to class '0' is equivalent to disabling
*     adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_traffic_class_map_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_VERIFY);

   /*
    * JER2_ARAD_IPQ_TR_CLS_RNG_MIN may be changed and be grater then zero.
    */
   /* coverity[unsigned_compare] */
   if ((tr_cls_ndx < JER2_ARAD_IPQ_TR_CLS_RNG_MIN) ||
       (tr_cls_ndx > JER2_ARAD_IPQ_TR_CLS_RNG_MAX)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_TR_CLS_OUT_OF_RANGE_ERR, 10, exit);
   }
   if ((new_class < JER2_ARAD_IPQ_TR_CLS_RNG_MIN) ||
       (new_class > JER2_ARAD_IPQ_TR_CLS_RNG_MAX)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_TR_CLS_OUT_OF_RANGE_ERR, 10, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_traffic_class_map_verify()", 0, 0);
}

/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Settings is done per
*     TM profile. For Petra-B/JER2_ARAD default TM profile,
*     use JER2_ARAD_IPQ_TC_PROFILE_DFLT.
*     Note that a class that is mapped to class '0' is equivalent to
*     disabling adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_traffic_class_profile_map_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core_id,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS_PROFILE  profile_ndx,
   DNX_SAND_IN  uint8                is_flow_profile,
   DNX_SAND_IN  uint8                is_multicast_profile,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
   ) {
   uint32
      entry_offset,
      is_ingress_shape,
      res,
      nof_entries,
      data = 0,
      is_flow_or_multicast;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_SET_UNSAFE);
   if (is_multicast_profile) {
       is_flow_or_multicast = 2;
   } else if (is_flow_profile) {
       is_flow_or_multicast = 1;
   } else {
       is_flow_or_multicast = 0;
   }
   /* 
    * Given a specific TM-profile, set the following entries:
    * Destination type system port before and after ingress shaping.
    * Destination type flow id before and after ingress shaping.
    */
   nof_entries = (SOC_IS_QAX(unit) ? 1 : 2); /* No ingress shaping in JER2_QAX */ 
   for (is_ingress_shape = 0; is_ingress_shape < nof_entries; ++is_ingress_shape) {
       entry_offset = JER2_ARAD_IPQ_TC_ENTRY(unit, is_flow_or_multicast, profile_ndx, is_ingress_shape, tr_cls_ndx);
       if (!SOC_IS_ARADPLUS_AND_BELOW(unit)) {
          if (SOC_IS_QAX(unit)) { /* JER2_QAX */
               soc_mem_field32_set(unit, TAR_TRAFFIC_CLASS_MAPPINGm, &data, TCf, new_class);
               res = WRITE_TAR_TRAFFIC_CLASS_MAPPINGm(unit, MEM_BLOCK_ANY, entry_offset, &data);
               DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
          } else { /* Jericho */
              if (core_id == 0 || core_id == SOC_CORE_ALL) {
                   soc_mem_field32_set(unit, IRR_TRAFFIC_CLASS_MAPPING_0m, &data, TCf, new_class);
                   res = WRITE_IRR_TRAFFIC_CLASS_MAPPING_0m(unit, MEM_BLOCK_ANY, entry_offset, &data);
                   DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
               }
               if (core_id == 1 || core_id == SOC_CORE_ALL) {
                   soc_mem_field32_set(unit, IRR_TRAFFIC_CLASS_MAPPING_1m, &data, TCf, new_class);
                   res = WRITE_IRR_TRAFFIC_CLASS_MAPPING_1m(unit, MEM_BLOCK_ANY, entry_offset, &data);
                   DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 21, exit);
               }
          }
      } else { /* JER2_ARAD */
          JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_DATA
             tbl_data;
         /* 
          *  The traffic class is mapped according to packet's source and destination. Table is accessed as follows:
          *    Before ingress shaping:
          *    line 0,2,4,6 - Destination is System-Port-ID with TC-Mapping-Profile equal to 0,1,2,3 Respectively
          *    line 8,10,12,14 - Destination is Flow with Flow-Profile equal to 0,1,2,3 Respectively
          *    line 16 - Destination is fabric or egress multicast
          *  After ingress shaping:
          *    line 1,3,5,7 - Destination is System-Port-ID with TC-Mapping-Profile equal to 0,1,2,3 Respectively
          *    line 9,11,13,15 - Destination is Flow with Flow-Profile equal to 0,1,2,3 Respectively
          *    line 17 - Destination is fabric or egress multicast
          */
          res = jer2_arad_irr_traffic_class_mapping_tbl_get_unsafe(
             unit,
             core_id,
             entry_offset,
             &tbl_data
             );
          DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

          tbl_data.traffic_class_mapping[tr_cls_ndx] = new_class;

          res = jer2_arad_irr_traffic_class_mapping_tbl_set_unsafe(
             unit,
             core_id,
             entry_offset,
             &tbl_data
             );
          DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      }
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_traffic_class_map_set_unsafe()", 0, 0);
}


/*********************************************************************
*     Maps the embedded traffic class in the packet header to
*     a logical traffic class. This logical traffic class will
*     be further used for traffic management. Settings is done per
*     TM profile. For Petra-B/JER2_ARAD default TM profile,
*     use JER2_ARAD_IPQ_TC_PROFILE_DFLT.
*     Note that a class that is mapped to class '0' is equivalent to
*     disabling adding the class.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_traffic_class_profile_map_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core_id,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS_PROFILE  profile_ndx,
   DNX_SAND_IN  uint8                is_flow_profile,
   DNX_SAND_IN  uint8                is_multicast_profile,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
   DNX_SAND_OUT JER2_ARAD_IPQ_TR_CLS          *new_class
   ) {
   uint32
      entry_offset,
      res,
      data = 0,
      is_ingress_shape = 0,
      is_flow_or_multicast;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_GET_UNSAFE);
   if (is_multicast_profile) {
       is_flow_or_multicast = 2;
   } else if (is_flow_profile) {
       is_flow_or_multicast = 1;
   } else {
       is_flow_or_multicast = 0;
   }

   entry_offset = JER2_ARAD_IPQ_TC_ENTRY(unit, is_flow_or_multicast, profile_ndx, is_ingress_shape, tr_cls_ndx);
   if (!SOC_IS_ARADPLUS_AND_BELOW(unit)) {
       if (SOC_IS_QAX(unit)) { /* JER2_QAX */
            res = READ_TAR_TRAFFIC_CLASS_MAPPINGm(unit, MEM_BLOCK_ANY, entry_offset, &data);
            DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
            *new_class = soc_mem_field32_get(unit, TAR_TRAFFIC_CLASS_MAPPINGm, &data, TCf);
       } else { /* Jericho */
           if (core_id == 0 || core_id == SOC_CORE_ALL) {
                res = READ_IRR_TRAFFIC_CLASS_MAPPING_0m(unit, MEM_BLOCK_ANY, entry_offset, &data);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 11, exit);
                *new_class = soc_mem_field32_get(unit, IRR_TRAFFIC_CLASS_MAPPING_0m, &data, TCf);
            } else if (core_id == 1) {
                res = READ_IRR_TRAFFIC_CLASS_MAPPING_1m(unit, MEM_BLOCK_ANY, entry_offset, &data);
                DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 21, exit);
                *new_class = soc_mem_field32_get(unit, IRR_TRAFFIC_CLASS_MAPPING_1m, &data, TCf);
            }
       }
   } else { /* JER2_ARAD */
       JER2_ARAD_IRR_TRAFFIC_CLASS_MAPPING_TBL_DATA
          tbl_data;
       res = jer2_arad_irr_traffic_class_mapping_tbl_get_unsafe(
          unit,
          core_id,
          entry_offset,
          &tbl_data
          );
       DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

       *new_class = tbl_data.traffic_class_mapping[tr_cls_ndx];
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_traffic_class_map_get_unsafe()", 0, 0);
}

uint32
jer2_arad_ipq_traffic_class_profile_map_verify(
   DNX_SAND_IN  int                unit,
   DNX_SAND_IN  int                core_id,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS_PROFILE  profile_ndx,
   DNX_SAND_IN  uint8                is_flow_profile,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_PROFILE_MAP_VERIFY);


   DNX_SAND_ERR_IF_ABOVE_MAX(tr_cls_ndx, DNX_TMC_NOF_TRAFFIC_CLASSES - 1, JER2_ARAD_IPQ_INVALID_FLOW_ID_ERR, 10, exit);
   DNX_SAND_ERR_IF_ABOVE_MAX(new_class, DNX_TMC_NOF_TRAFFIC_CLASSES - 1, JER2_ARAD_IPQ_INVALID_FLOW_ID_ERR, 20, exit);
   DNX_SAND_ERR_IF_ABOVE_MAX(profile_ndx, JER2_ARAD_IPQ_NOF_UC_DEST_TC_ENTRIES - 1, JER2_ARAD_IPQ_INVALID_FLOW_ID_ERR, 30, exit);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_traffic_class_profile_map_verify()", 0, 0);
}

uint32
jer2_arad_ipq_traffic_class_map_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          new_class
   ) {
   uint32
      res;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_SET_UNSAFE);

   /* Petra-B compatible API, Use default TM profile */
   res = jer2_arad_ipq_traffic_class_profile_map_set_unsafe(
      unit,
      SOC_CORE_ALL,
      JER2_ARAD_IPQ_TC_PROFILE_DFLT,
      0,
      0,
      tr_cls_ndx,
      new_class
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_traffic_class_map_set_unsafe()", 0, 0);
}

uint32
jer2_arad_ipq_traffic_class_map_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  JER2_ARAD_IPQ_TR_CLS          tr_cls_ndx,
   DNX_SAND_OUT JER2_ARAD_IPQ_TR_CLS          *new_class
   ) {
   uint32
      res;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MAP_GET_UNSAFE);

   /* Petra-B compatible API, Use default TM profile */
   /* This API set global mapping only */
   res = jer2_arad_ipq_traffic_class_profile_map_get_unsafe(
      unit,
      SOC_CORE_ALL,
      JER2_ARAD_IPQ_TC_PROFILE_DFLT,
      0,
      0,
      tr_cls_ndx,
      new_class
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_traffic_class_map_get_unsafe()", 0, 0);
}

uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_set_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_IN  uint8                  enable
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_SET_VERIFY);

  DNX_SAND_ERR_IF_ABOVE_MAX(traffic_class, 7, JER2_ARAD_IPQ_INVALID_TC_ERR, 10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_multicast_priority_map_set_verify()",0,0);
}

uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_IN  uint8                  enable
  )
{
  uint32
    field_val[1],
    data,
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_SET_UNSAFE);



  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IRR_STATIC_CONFIGURATIONr(unit, &data));
  *field_val = soc_reg_field_get(
    unit,
    IRR_STATIC_CONFIGURATIONr,
    data,
    TRAFFIC_CLASS_HPf);
  if (enable) {
      SHR_BITSET(field_val, traffic_class);
  } else {
      SHR_BITCLR(field_val, traffic_class);
  }
  soc_reg_field_set(
    unit,
    IRR_STATIC_CONFIGURATIONr,
    &data,
    TRAFFIC_CLASS_HPf,
    *field_val);

  DNX_SAND_SOC_IF_ERROR_RETURN(res, 20, exit, WRITE_IRR_STATIC_CONFIGURATIONr(unit, data));

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_multicast_priority_map_set_unsafe()",0,0);
}


uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_get_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_OUT uint8                  *enable
  )
{
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_GET_VERIFY);
  DNX_SAND_CHECK_NULL_INPUT(enable);
  DNX_SAND_ERR_IF_ABOVE_MAX(traffic_class, 7, JER2_ARAD_IPQ_INVALID_TC_ERR,10, exit);

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_multicast_priority_map_get_verify()",0,0);
}

uint32
  jer2_arad_ipq_traffic_class_multicast_priority_map_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 traffic_class,
    DNX_SAND_OUT uint8                  *enable
  )
{
  uint32
    field_val[1],
    data,
    res;
  DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TRAFFIC_CLASS_MULTICAST_PRIORITY_MAP_GET_UNSAFE);
  DNX_SAND_CHECK_NULL_INPUT(enable);


  DNX_SAND_SOC_IF_ERROR_RETURN(res, 10, exit, READ_IRR_STATIC_CONFIGURATIONr(unit, &data));
  *field_val = soc_reg_field_get(
    unit,
    IRR_STATIC_CONFIGURATIONr,
    data,
    TRAFFIC_CLASS_HPf);

  *enable = SHR_BITGET(field_val , traffic_class) ? TRUE : FALSE;
exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in jer2_arad_ipq_traffic_class_multicast_priority_map_get_unsafe()",0,0);
}


/*********************************************************************
*     Verify the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_destination_id_packets_base_queue_id_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32              dest_ndx,
   DNX_SAND_IN  uint8               valid,
   DNX_SAND_IN  uint8               sw_only,
   DNX_SAND_IN  uint32              base_queue
   ) {
    uint32
       res;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_VERIFY);

   /* JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_DEST_NDX_MIN may be changed and be grater than 0*/
   /* coverity[unsigned_compare : FALSE] */
    if ((dest_ndx < JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MIN) ||
        (dest_ndx > (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ?
            JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_INDIRECT :
            JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_DIRECT))) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_DEST_NDX_OUT_OF_RANGE_ERR, 10, exit);
   }

   /* JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_BASE_QUEUE_MIN may be changed and be grater than 0*/
   /* coverity[unsigned_compare : FALSE] */
   if (!((base_queue >= JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_BASE_QUEUE_MIN) || (base_queue <= (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) ||
         (base_queue >= JER2_ARAD_IPQ_DESTINATION_ID_STACKING_BASE_QUEUE_MIN) || (base_queue <= JER2_ARAD_IPQ_DESTINATION_ID_STACKING_BASE_QUEUE_MAX(unit)))) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_BASE_QUEUE_OUT_OF_RANGE_ERR, 20, exit);
   }
   /* SW only configuration is allowed only for asymmetric mode and for a specific core */
   if (valid && sw_only && (core == SOC_CORE_ALL || SOC_DNX_CORE_MODE_IS_SYMMETRIC(unit))) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_CONFIG_ERR, 30, exit);
   }
   if ((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != SOC_CORE_ALL) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR, 40, exit);
   }

   if (valid &&
       !SOC_DNX_CORE_MODE_IS_SYMMETRIC(unit) && 
       !sw_only &&
       (core != SOC_CORE_ALL)) {
       /*validate that if we configure a syspoty-to-queue mapping in HW,
         there isn't an existing mapping allready for the same sysport in other core*/
       uint8
           valid_i[SOC_DNX_DEFS_MAX(NOF_CORES)],
           sw_only_i[SOC_DNX_DEFS_MAX(NOF_CORES)];
       uint32
           base_queue_old[SOC_DNX_DEFS_MAX(NOF_CORES)];
       int core_i = 0;
       for (core_i = 0;core_i < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; core_i++){
           res = jer2_arad_sw_db_sysport2queue_get(unit, core_i, dest_ndx, &valid_i[core_i], &sw_only_i[core_i], &base_queue_old[core_i]);
           DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
       }
       for (core_i = 0;core_i < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; core_i++){
           if (!sw_only && core != core_i && valid_i[core_i] && !sw_only_i[core_i] && base_queue != base_queue_old[core_i]) {
               LOG_ERROR(BSL_LS_SOC_COSQ, 
                         (BSL_META_U(unit, 
                                     "Cannot map destination system port %d in core %d, to base queue 0x%x Since in core %d it is mapped to base queue 0x%x.\n"), 
                          dest_ndx, core, base_queue, core_i, base_queue_old[core_i]));
               DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_CONFIG_ERR, 60, exit);
           }
       }
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_destination_id_packets_base_queue_id_verify()", 0, 0);
}

/*********************************************************************
*     Sets the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_destination_id_packets_base_queue_id_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32              dest_ndx,
   DNX_SAND_IN  uint8               valid,
   DNX_SAND_IN  uint8               sw_only,
   DNX_SAND_IN  uint32              base_queue
   ) {
   uint32
      res;
   JER2_ARAD_IRR_DESTINATION_TABLE_TBL_DATA
      irr_destination_table_tbl_data;
   int core_i = 0;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_SET_UNSAFE);

    if (!sw_only) {
       /*if sw_only==FALSE we do not get the parameters from HW, since we assume that the can be configured iff the queue is allready created*/
       for (core_i = 0; core_i < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; core_i++) {
           if (core == SOC_CORE_ALL || core == core_i) {
               irr_destination_table_tbl_data.valid[core_i] = valid;
           } else {
               irr_destination_table_tbl_data.valid[core_i] =  0;
           }
       }
       irr_destination_table_tbl_data.queue_number = (valid) ? base_queue : JER2_ARAD_IPQ_DESTINATION_ID_INVALID_QUEUE(unit);
       irr_destination_table_tbl_data.tc_profile = JER2_ARAD_IPQ_TC_PROFILE_DFLT;

       res = jer2_arad_irr_destination_table_tbl_set_unsafe(
         unit,
         dest_ndx,
         &irr_destination_table_tbl_data
         );
       DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
    }
    /*if sw_only==TRUE we assume that the profie is allready configured as valid=0, so we should do nothing*/
    res = jer2_arad_sw_db_sysport2queue_set(unit, core, dest_ndx, valid, sw_only, base_queue);
    DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);

    if (JER2_ARAD_IS_HQOS_MAPPING_ENABLE(unit)) {
        res = jer2_arad_sw_db_queuequartet2sysport_set(unit, core, base_queue/4, dest_ndx);
        DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 50, exit);
    }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_destination_id_packets_base_queue_id_set_unsafe()", 0, 0);
}

/*********************************************************************
*     Sets the system-port (destination-id) mapping to base queue
*     Doesn't affect packets that arrive with
*     explicit queue-id in the header. Each destination ID is
*     mapped to a base_queue, when the packet is stored in
*     queue: base_queue + class
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_destination_id_packets_base_queue_id_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32              dest_ndx,
   DNX_SAND_OUT uint8               *valid,
   DNX_SAND_OUT uint8               *sw_only,
   DNX_SAND_OUT uint32              *base_queue
      ) {
   uint32
      res;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_QUEUE_ID_GET_UNSAFE);
   DNX_SAND_CHECK_NULL_INPUT(valid);
   DNX_SAND_CHECK_NULL_INPUT(sw_only);
   DNX_SAND_CHECK_NULL_INPUT(base_queue);

   /* JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_DEST_NDX_MIN may be changed and be grater than 0*/
    if (((int)dest_ndx < JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MIN) ||
        (dest_ndx > (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ?
            JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_INDIRECT :
            JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_DIRECT))) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_DESTINATION_ID_PACKETS_BASE_DEST_NDX_OUT_OF_RANGE_ERR, 10, exit);
   }

   if ((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != SOC_CORE_ALL) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR, 20, exit);
   }

   res = jer2_arad_sw_db_sysport2queue_get(unit, core, dest_ndx, valid, sw_only, base_queue);
   DNX_SAND_SOC_CHECK_FUNC_RESULT(res, 30, exit);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_destination_id_packets_base_queue_id_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Sets the stack_lag packets mapping to queue
*     information.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_stack_lag_packets_base_queue_id_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint32                 tmd,
   DNX_SAND_IN  uint32                 entry,
   DNX_SAND_IN  uint32                 base_queue
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   /* JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MIN may be changed and be grater than 0*/
   /* coverity[unsigned_compare : FALSE] */
   if ((tmd < JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MIN) || (tmd > JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MAX)) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 10, exit);
   }
   /* JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MIN may be changed and be grater than 0*/
   /* coverity[unsigned_compare : FALSE] */
   if (((entry < JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MIN) || (entry > JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MAX)) && (entry != JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_ALL)) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 20, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_stack_lag_packets_base_queue_id_verify()", 0, 0);
}

uint32
jer2_arad_ipq_stack_lag_packets_base_queue_id_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint32                 tmd,
   DNX_SAND_IN  uint32                 entry,
   DNX_SAND_IN  uint32                 base_queue
   ) {
   uint32
      res,
      entry_offset;
   JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA
      irr_stack_trunk_table_tbl_data;

   DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   if (entry == JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_ALL) {

      entry_offset = (tmd << JER2_ARAD_IPQ_STACK_LAG_ENTRY_PER_TMD_BIT_NUM);
      res = jer2_arad_fill_partial_table_with_entry(unit, IRR_STACK_TRUNK_RESOLVEm, 0, 0, MEM_BLOCK_ANY, entry_offset, entry_offset + JER2_ARAD_IPQ_STACK_LAG_STACK_TRUNK_RESOLVE_ENTRY_MAX - 1, &base_queue);
      DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);

   } else {
      entry_offset = (tmd << JER2_ARAD_IPQ_STACK_LAG_ENTRY_PER_TMD_BIT_NUM) | entry;
      res = jer2_arad_irr_stack_trunk_resolve_table_tbl_get_unsafe(unit, entry_offset, &irr_stack_trunk_table_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

      irr_stack_trunk_table_tbl_data.base_queue = base_queue;

      res = jer2_arad_irr_stack_trunk_resolve_table_tbl_set_unsafe(unit, entry_offset, &irr_stack_trunk_table_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_stack_lag_packets_base_queue_id_set_unsafe()", entry_offset, base_queue);
}

uint32
jer2_arad_ipq_stack_lag_packets_base_queue_id_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint32                 tmd,
   DNX_SAND_IN  uint32                 entry,
   DNX_SAND_OUT uint32              *base_queue
   ) {
   uint32
      res,
      entry_offset;
   JER2_ARAD_IRR_STACK_TRUNK_RESOLVE_TBL_DATA
      irr_stack_trunk_table_tbl_data;

   DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   DNX_SAND_CHECK_NULL_INPUT(base_queue);

   entry_offset = (tmd << JER2_ARAD_IPQ_STACK_LAG_ENTRY_PER_TMD_BIT_NUM) | entry;
   res = jer2_arad_irr_stack_trunk_resolve_table_tbl_get_unsafe(unit, entry_offset, &irr_stack_trunk_table_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   *base_queue = irr_stack_trunk_table_tbl_data.base_queue;

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_stack_lag_packets_base_queue_id_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Sets the stack fec mapping to stack trunk mapping
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_stack_fec_map_stack_lag_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint32                 tmd,
   DNX_SAND_IN  uint32                 entry,
   DNX_SAND_IN  uint32                 stack_lag
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   /* JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MIN may be changed and be grater than 0*/
   /* coverity[unsigned_compare : FALSE] */
   if ((tmd < JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MIN) || (tmd > JER2_ARAD_IPQ_STACK_LAG_DOMAIN_MAX)) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 10, exit);
   }
   /* JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MIN may be changed and be grater than 0*/
   /* coverity[unsigned_compare : FALSE] */
   if (((entry < JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MIN) || (entry > JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_MAX)) && (entry != JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL)) {
      DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 20, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_stack_fec_map_stack_lag_verify()", 0, 0);
}

uint32
jer2_arad_ipq_stack_fec_map_stack_lag_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint32                 tmd,
   DNX_SAND_IN  uint32                 entry,
   DNX_SAND_IN  uint32                 stack_lag
   ) {
   uint32
     res,
     i,
     entry_offset=0,
     nof_entries,
     fec_val,
     offset;

   DNX_SAND_INIT_ERROR_DEFINITIONS(0);
/*
 * Each 4 entries represent 1 stacking trunk. maximum number of stacking trunk is 4.
 */
   if (entry == JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL) {
       nof_entries = 16;
       offset = 0;
   } else {
       nof_entries = 4;
       offset = entry * 4;
   }

   for(i=0; i < nof_entries; i++) {
/*
 *  stacking port are multiple by 4 to create more entries per domain in the stack_resolve_ table.
 */
      entry_offset = (tmd << JER2_ARAD_IPQ_STACK_FEC_ENTRY_PER_TMD_BIT_NUM) + offset + i % nof_entries;
      fec_val = stack_lag * 4 + i % 4;
      res = jer2_arad_fill_partial_table_with_entry(unit, IRR_STACK_FEC_RESOLVEm, 0, 0, MEM_BLOCK_ANY, entry_offset, entry_offset , &fec_val);
      DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
   }



exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_stack_fec_map_stack_lag_set_unsafe()", entry_offset, stack_lag);
}

uint32
jer2_arad_ipq_stack_fec_map_stack_lag_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  uint32                 tmd,
   DNX_SAND_IN  uint32                 entry,
   DNX_SAND_OUT uint32 *stack_lag
   ) {
   uint32
     res,
     entry_offset=0,
     val;

   DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   if (entry == JER2_ARAD_IPQ_STACK_LAG_STACK_FEC_RESOLVE_ENTRY_ALL) {

       DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 10, exit);

   } else {
     entry_offset = (tmd << JER2_ARAD_IPQ_STACK_FEC_ENTRY_PER_TMD_BIT_NUM) + entry * 4;
     res = soc_mem_read(unit, IRR_STACK_FEC_RESOLVEm, MEM_BLOCK_ANY, entry_offset, &val);
     DNX_SAND_CHECK_FUNC_RESULT(res, 90, exit);
   }
/*
 * stack_lag divided to 4, because in set it was multiple by fore.
 */
   *stack_lag = val / 4;

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_stack_fec_map_stack_lag_get_unsafe()", 0, 0);
}


/*********************************************************************
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_interdigitated_mode_verify(
   DNX_SAND_IN  int                  unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                  k_queue_ndx,
   DNX_SAND_IN  uint8                 is_interdigitated
   ) {
   uint32
      queue_index;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_VERIFY);

   queue_index = k_queue_ndx << 10;
   if (queue_index > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_K_QUEUE_INDEX_OUT_OF_RANGE_ERR, 10, exit);
   }

   if ((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != SOC_CORE_ALL) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR, 20, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_interdigitated_mode_verify()", 0, 0);
}

/*********************************************************************
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_interdigitated_mode_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                  k_queue_ndx,
   DNX_SAND_IN  uint8                 is_interdigitated
   ) {
   uint32
      res,
      buffer;
   uint32
      queue_quartet_i, region_size;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_SET_UNSAFE);

   DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR, READ_IPS_INTERDIGITATED_MODEr(unit, core, &buffer));
   /*
    * Interdigitated register can vary between 1K-4K queue regions (depends in nof queues),
    * but k_queue_ndx is always in 1K queue regions, so need to divide it by
    * interdigitated region.
    */
   if (is_interdigitated) {
      /* turn bit on */
      buffer |= DNX_SAND_BIT(k_queue_ndx / SOC_DNX_DEFS_GET(unit, nof_pools_per_interdigitated_region));
   } else {
      /* turn bit off */
      buffer &= DNX_SAND_RBIT(k_queue_ndx / SOC_DNX_DEFS_GET(unit, nof_pools_per_interdigitated_region));
   }
    
   DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  20,  exit, JER2_ARAD_REG_ACCESS_ERR, WRITE_IPS_INTERDIGITATED_MODEr(unit, core,  buffer));

   /* reseting all queues in the k_queues_index */
   queue_quartet_i = (k_queue_ndx << 8);
   region_size = (1 << 8);
   res = jer2_arad_ipq_k_quartet_reset_unsafe(
      unit,
      core,
      queue_quartet_i,
      region_size
      );

   DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_interdigitated_mode_set_unsafe()", 0, 0);
}

/*********************************************************************
*     Map 1K of queues (256 quartets) to be in interdigitated
*     mode or not. Queue Quartets that configured to be in
*     interdigitated mode should only be configured with
*     interdigitated flow quartets, and the other-way around.
*     When interdigitated mode is set, all queue quartets
*     range are reset using jer2_arad_ipq_quartet_reset(), in
*     order to prevent illegal interdigitated/composite state.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_interdigitated_mode_get_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                  k_queue_ndx,
   DNX_SAND_OUT uint8                 *is_interdigitated
   ) {
   uint32
      res,
      buffer;


   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_INTERDIGITATED_MODE_GET_UNSAFE);

   DNX_SAND_CHECK_NULL_INPUT(is_interdigitated);




   DNX_SAND_SOC_IF_ERROR_RETURN_ERR_VAL(res,  10,  exit, JER2_ARAD_REG_ACCESS_ERR,READ_IPS_INTERDIGITATED_MODEr(unit, core, &buffer));

   /*
    * Interdigitated register can vary between 1K-4K queue regions (depends in nof queues),
    * but k_queue_ndx is always in 1K queue regions, so need to divide it by
    * interdigitated region.
    */
   /* true only if the the k_queue_index is on */
   *is_interdigitated = ((buffer & DNX_SAND_BIT(k_queue_ndx / SOC_DNX_DEFS_GET(unit, nof_pools_per_interdigitated_region))) != 0 ? TRUE : FALSE);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_interdigitated_mode_get_unsafe()", 0, 0);
}
/*********************************************************************
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_to_flow_mapping_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                  queue_quartet_ndx,
   DNX_SAND_IN  JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
   ) {
   uint32
      res,
      queue_index,
      flow_index;
   uint8
      is_interdigitated;
   JER2_ARAD_IPQ_QUARTET_MAP_INFO
      prev_flow_quartet_info;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_VERIFY);

   DNX_SAND_CHECK_NULL_INPUT(info);

   DNX_SAND_MAGIC_NUM_VERIFY(info);

   if ((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != SOC_CORE_ALL) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR, 5, exit);
   }

   queue_index = queue_quartet_ndx * 4;
   if (queue_index > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);
   }

   flow_index = (info->flow_quartet_index) * 4;
   if (flow_index > (SOC_DNX_DEFS_GET((unit), nof_flows) - 1)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_FLOW_ID_OUT_OF_RANGE_ERR, 20, exit);
   }


   if (info->is_modport && JER2_ARAD_IS_VOQ_MAPPING_DIRECT(unit)) {
      DNX_SAND_ERR_IF_ABOVE_MAX(info->fap_id, JER2_ARAD_MAX_FAP_ID, JER2_ARAD_FAP_FABRIC_ID_OUT_OF_RANGE_ERR, 35, exit);
      DNX_SAND_ERR_IF_ABOVE_MAX(info->fap_port_id, JER2_ARAD_MAX_FAP_PORT_ID, JER2_ARAD_FAP_PORT_ID_INVALID_ERR, 36, exit);
      /*
       * JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MIN may be changed and be grater then zero.
       */
      /* coverity[unsigned_compare] */
   } else if ((info->system_physical_port < JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MIN) ||
              (info->system_physical_port > (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ?
                                             JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_INDIRECT :
                                             JER2_ARAD_IPQ_QUARTET_MAP_INFO_SYSTEM_PHYSICAL_PORT_MAX_DIRECT))) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_SYS_PHYSICAL_PORT_NDX_OUT_OF_RANGE_ERR, 30, exit);
   }

   /*
    * Verify that interdigitated/composite/base_queue does not conflict
    */

   res = jer2_arad_ipq_queue_interdigitated_mode_get_unsafe(
      unit,
      core,
      DNX_SAND_DIV_ROUND_DOWN(queue_quartet_ndx * 4, 1024)/* k_queue_index */,
      &is_interdigitated
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);

   if (queue_quartet_ndx > 0) {
      res = jer2_arad_ipq_queue_to_flow_mapping_get_unsafe(
         unit,
         core,
         queue_quartet_ndx - 1,
         &prev_flow_quartet_info
         );
      DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);


      if ((is_interdigitated) &&                                                                            /*Invalid connector - can be either intrdigitated or not*/
             (info->flow_quartet_index < JER2_ARAD_IPQ_MIN_INTERDIGIT_FLOW_QUARTET(unit)) && (info->flow_quartet_index != JER2_ARAD_DEVICE_QUEUE_RESERVED_FLOW_START)) {DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_BASE_FLOW_FOR_INTERDIGIT_QUEUE_QUARTET_TOO_LOW_ERR, 60, exit);

      }

      if (((is_interdigitated && !info->is_composite) ||
           (!is_interdigitated && info->is_composite)) &&
          (!dnx_sand_is_even(info->flow_quartet_index))) {
         /* if interdigitated or composite (but not both), base flow quartet must
            be even */
         DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_BASE_FLOW_QUARTET_NOT_EVEN_ERR, 80, exit);
      }

      if (is_interdigitated && !info->is_composite &&
          ((info->flow_quartet_index % 4) != 0)) {
         /* if interdigitated but not composite, and not a multiply of 4,
            previous base flow quartet must not be composite as well */
         if (prev_flow_quartet_info.is_composite) {
            DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_BASE_FLOW_ALREADY_MAPPED_BY_PREVIOUS_QUEUE_QUARTET_ERR, 90, exit);
         }
      }

      if ((is_interdigitated && info->is_composite) &&
          ((info->flow_quartet_index % 4) != 0)) {
         /* if interdigitated and composite, base flow quartet must be a multiply
            of 4 */
         DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_BASE_FLOW_QUARTET_NOT_MULTIPLY_OF_FOUR_ERR, 100, exit);
      }
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_to_flow_mapping_verify()", 0, 0);
}

/*********************************************************************
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
*     In direct voq to modport mapping mode, a mapping to modport is performed
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_to_flow_mapping_set_unsafe(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                 queue_quartet_ndx,
   DNX_SAND_IN  JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
   ) {
   uint32 res;
   uint32 fap_id = 0, fap_port_id = 0;
   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA ips_flow_id_lookup_table_tbl_data;
   JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA system_physical_port_tbl_data;
   JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA mod_port_tbl_data;
   uint32 indirect_mode = JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit) ? 1 : 0;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_SET_UNSAFE);
   DNX_SAND_CHECK_NULL_INPUT(info);

   /*
    * Set base flow {
    */
   res = jer2_arad_ips_flow_id_lookup_table_tbl_get_unsafe(unit, core, queue_quartet_ndx, &ips_flow_id_lookup_table_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   res = indirect_mode ?
      jer2_arad_indirect_base_queue_to_system_physical_port_tbl_get_unsafe(unit, core, queue_quartet_ndx, &system_physical_port_tbl_data) :
      jer2_arad_direct_base_queue_to_system_physical_port_tbl_get_unsafe(unit, core, queue_quartet_ndx, &mod_port_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 20 + indirect_mode, exit);

   /* multiply by 4 from quartet to base flow */
   ips_flow_id_lookup_table_tbl_data.base_flow = info->flow_quartet_index;

   ips_flow_id_lookup_table_tbl_data.sub_flow_mode = info->is_composite;

   res = jer2_arad_ips_flow_id_lookup_table_tbl_set_unsafe(unit, core, queue_quartet_ndx, &ips_flow_id_lookup_table_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 40 + indirect_mode, exit);

   /*
    * Set base flow }
    */

   if (indirect_mode) {
      system_physical_port_tbl_data.sys_phy_port = info->system_physical_port == DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID_JER2_ARAD ?
         DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID : info->system_physical_port;
      res = jer2_arad_indirect_base_queue_to_system_physical_port_tbl_set_unsafe(unit, core, queue_quartet_ndx, &system_physical_port_tbl_data);
   } else {
      /* In the direct mode we map to mod x port which we either get as input or map from input sysport */
      if (info->is_modport) { /* no mapping */
         mod_port_tbl_data.fap_id = info->fap_id;
         mod_port_tbl_data.fap_port_id = info->fap_port_id;
      } else { /* find mod_port from system port */
         if (info->system_physical_port == DNX_TMC_MAX_SYSTEM_PHYSICAL_PORT_ID_JER2_ARAD) {
            mod_port_tbl_data.fap_id = JER2_ARAD_MAX_FAP_ID;
            mod_port_tbl_data.fap_port_id = JER2_ARAD_MAX_FAP_PORT_ID;
         } else {

            if (JER2_ARAD_IS_HQOS_MAPPING_ENABLE(unit)) {
                res = jer2_arad_sw_db_sysport2modport_get(unit, info->system_physical_port, &fap_id, &fap_port_id);
                DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
                if (fap_id == JER2_ARAD_SW_DB_SYSPORT2MODPORT_INVALID_ID) {
                    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_INVALID_SYS_PORT_ERR, 55, exit);
                }
            } else {
                res = jer2_arad_sw_db_modport2sysport_reverse_get(unit, info->system_physical_port, &fap_id, &fap_port_id);
                DNX_SAND_CHECK_FUNC_RESULT(res, 50, exit);
                if (fap_id == JER2_ARAD_SW_DB_MODPORT2SYSPORT_REVERSE_GET_NOT_FOUND) {
                    DNX_SAND_SET_ERROR_CODE(JER2_ARAD_IPQ_INVALID_SYS_PORT_ERR, 55, exit);
                }
            }

            mod_port_tbl_data.fap_id = fap_id;
            mod_port_tbl_data.fap_port_id = fap_port_id;
         }
      }
      res = jer2_arad_direct_base_queue_to_system_physical_port_tbl_set_unsafe(unit, core, queue_quartet_ndx, &mod_port_tbl_data);
   }
   DNX_SAND_CHECK_FUNC_RESULT(res, 60 + indirect_mode, exit);

  
   DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
#endif 
    
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_to_flow_mapping_set_unsafe()", queue_quartet_ndx, 0);
}

static uint32
  soc_jer2_arad_ipq_queue_empty_check_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32                 queue_ndx,
    DNX_SAND_IN  uint32                 iterations,
    DNX_SAND_OUT uint32                 *is_empty
  )
{
  uint32
    res = DNX_SAND_OK,
    i;

  DNX_SAND_INIT_ERROR_DEFINITIONS(0);

  (*is_empty) = 1;

  for(i=0 ; i<iterations ; i++) {    
    res = jer2_arad_cosq_ips_non_empty_queues_info_get_unsafe(
        unit,
        core,
        queue_ndx,
        is_empty
        );
    DNX_SAND_CHECK_FUNC_RESULT(res, i, exit);

    /* keep on reading as long as the queue is empty*/
    if((*is_empty) == 0)
        break;
  }

exit:
  DNX_SAND_EXIT_AND_SEND_ERROR( "error in soc_jer2_arad_ipq_queue_flush_unsafe()",0,0);
}

/*********************************************************************
*     Unmap a queues-quartet, by mapping it to invalid
*     destination. Also, flush all the queues in the quartet.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_qrtt_unmap_unsafe(
   DNX_SAND_IN  int unit,
   DNX_SAND_IN  int core,
   DNX_SAND_IN  uint32  queue_quartet_ndx
   ) {
   uint32
      res = DNX_SAND_OK;
   uint32
      baseq_id,
      q_id,
      is_empty;
   soc_dnx_config_t
      *dnx = NULL;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_QRTT_UNMAP_UNSAFE);

   baseq_id = JER2_ARAD_IPQ_QRTT_TO_Q_ID(queue_quartet_ndx);
   DNX_SAND_ERR_IF_ABOVE_MAX(baseq_id, SOC_DNX_DEFS_GET(unit, nof_queues) - 1, JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);

   dnx = SOC_DNX_CONFIG(unit);
   if(dnx->jer2_arad->init.fabric.connect_mode == JER2_ARAD_FABRIC_CONNECT_MODE_MESH) {
       for (q_id = baseq_id; q_id <= baseq_id + 3; q_id++){
          res = soc_jer2_arad_ipq_queue_empty_check_unsafe(unit, core, q_id, JER2_ARAD_IPQ_QUEUE_EMPTY_ITERATIONS, &is_empty);
          DNX_SAND_CHECK_FUNC_RESULT(res, 15, exit);

          if(!is_empty) {
              LOG_ERROR(BSL_LS_SOC_INGRESS,
                        (BSL_META_U(unit,
                                    "Queue %d must be empty"),
                         q_id));
              DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 20, exit);
          }
       }
   }
   res = jer2_arad_ipq_quartet_reset_unsafe(
      unit,
      core,
      queue_quartet_ndx
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_qrtt_unmap_unsafe()", queue_quartet_ndx, 0);
}


/*********************************************************************
*     Map a queues-quartet to system port and flow-quartet(s).
*     This function should only be called after calling the
*     jer2_arad_ipq_queue_interdigitated_mode_set()
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_queue_to_flow_mapping_get_unsafe(
   DNX_SAND_IN  int     unit,
   DNX_SAND_IN  int     core,
   DNX_SAND_IN  uint32  queue_quartet_ndx,
   DNX_SAND_OUT JER2_ARAD_IPQ_QUARTET_MAP_INFO *info
   ) {
   uint32 res;
   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA ips_flow_id_lookup_table_tbl_data;
   JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA system_physical_port_tbl_data;
   JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA mod_port_tbl_data;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_TO_FLOW_MAPPING_GET_UNSAFE);

   DNX_SAND_CHECK_NULL_INPUT(info);

   res = jer2_arad_ips_flow_id_lookup_table_tbl_get_unsafe(unit, core, queue_quartet_ndx, &ips_flow_id_lookup_table_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

   /* divide by 4 from quartet to base flow */
   info->flow_quartet_index = ips_flow_id_lookup_table_tbl_data.base_flow;

   info->is_composite = (uint8)ips_flow_id_lookup_table_tbl_data.sub_flow_mode;

   if (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)) { /* indirect mapping mode */
      res = jer2_arad_indirect_base_queue_to_system_physical_port_tbl_get_unsafe(unit, core, queue_quartet_ndx, &system_physical_port_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
      info->system_physical_port = system_physical_port_tbl_data.sys_phy_port;
   } else { /* direct mapping mode */

      JER2_ARAD_SYSPORT sysport;
      res = jer2_arad_direct_base_queue_to_system_physical_port_tbl_get_unsafe(unit, core, queue_quartet_ndx, &mod_port_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
      info->fap_id = mod_port_tbl_data.fap_id;
      info->fap_port_id = mod_port_tbl_data.fap_port_id;

      if (JER2_ARAD_IS_HQOS_MAPPING_ENABLE(unit)) { /* different system ports are mapped to different queues but same modport */
          res = jer2_arad_sw_db_queuequartet2sysport_get(unit, core, queue_quartet_ndx, &sysport);
          DNX_SAND_CHECK_FUNC_RESULT(res, 25, exit);
      }
      else {
          res = jer2_arad_sw_db_modport2sysport_get(unit, mod_port_tbl_data.fap_id, mod_port_tbl_data.fap_port_id, &sysport);
          DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
      }
      info->system_physical_port = sysport == JER2_ARAD_NOF_SYS_PHYS_PORTS_GET(unit) ? -1 : sysport;
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_to_flow_mapping_get_unsafe()", 0, 0);
}

/*********************************************************************
*     Resets a quartet to default values.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_quartet_reset_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                  queue_quartet_ndx
   ) {
   uint32
      queue_index;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUARTET_RESET_VERIFY);

   queue_index = queue_quartet_ndx * 4;
   if (queue_index > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);
   }
   if ((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != SOC_CORE_ALL) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR, 20, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_quartet_reset_verify()", 0, 0);
}

/*********************************************************************
*     Resets a quartet to default values.
*********************************************************************/
uint32
jer2_arad_ipq_quartet_reset_unsafe(
   DNX_SAND_IN int unit,
   DNX_SAND_IN int core,
   DNX_SAND_IN uint32 queue_quartet_ndx
   ) {
   uint32 res;
   uint32 baseq_id;
   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA ips_flow_id_lookup_table_tbl_data;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUARTET_RESET_UNSAFE);

   baseq_id = JER2_ARAD_IPQ_QRTT_TO_Q_ID(queue_quartet_ndx);
   DNX_SAND_ERR_IF_ABOVE_MAX(baseq_id, SOC_DNX_DEFS_GET(unit, nof_queues) - 1, JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);

   /* set default mapping of base queue to system port / mod x port */
   if (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)) { /* indirect mapping mode */
      JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA system_physical_port_tbl_data;
      system_physical_port_tbl_data.sys_phy_port = JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_IN_IPS;
      res = jer2_arad_indirect_base_queue_to_system_physical_port_tbl_set_unsafe(unit, core, queue_quartet_ndx, &system_physical_port_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
   } else { /* direct mapping mode */
      JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA mod_port_tbl_data;
      mod_port_tbl_data.fap_id = JER2_ARAD_MAX_FAP_ID;
      mod_port_tbl_data.fap_port_id = JER2_ARAD_MAX_FAP_PORT_ID;
      res = jer2_arad_direct_base_queue_to_system_physical_port_tbl_set_unsafe(unit, core, queue_quartet_ndx, &mod_port_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
   }

   /*
    * Set base flow {
    */
   /* multiply by 4 from quartet to base flow */
   ips_flow_id_lookup_table_tbl_data.base_flow = JER2_ARAD_IPQ_INVALID_FLOW_QUARTET;
   ips_flow_id_lookup_table_tbl_data.sub_flow_mode = 0x0;

   res = jer2_arad_ips_flow_id_lookup_table_tbl_set_unsafe(unit, core, queue_quartet_ndx, &ips_flow_id_lookup_table_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 40, exit);
   /*
    * Set base flow }
    */

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_quartet_reset_unsafe()", 0, 0);
}

/*********************************************************************
*     Resets a quartet to default values.
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
jer2_arad_ipq_k_quartet_reset_verify(
   DNX_SAND_IN  int                 unit,
   DNX_SAND_IN  int                 core,
   DNX_SAND_IN  uint32                  queue_k_quartet_ndx,
   DNX_SAND_IN  uint32                  region_size
   ) {
   uint32
      queue_index;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_K_QUARTET_RESET_VERIFY);

   queue_index = queue_k_quartet_ndx * 4;
   if (((queue_index > (SOC_DNX_DEFS_GET(unit, nof_queues) - 1024))) && ((region_size * 4) == 1024)) {
      DNX_SAND_SET_ERROR_CODE(JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);
   }
   if ((core < 0 || core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) && core != SOC_CORE_ALL) {
       DNX_SAND_SET_ERROR_CODE(JER2_ARAD_CORE_INDEX_OUT_OF_RANGE_ERR, 20, exit);
   }
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_k_quartet_reset_verify()", 0, 0);
}

/*********************************************************************
*     Resets a kquartet to default values.
*********************************************************************/
uint32
jer2_arad_ipq_k_quartet_reset_unsafe(
   DNX_SAND_IN int unit,
   DNX_SAND_IN int core,
   DNX_SAND_IN uint32 queue_k_quartet_ndx,
   DNX_SAND_IN uint32 region_size
   ) {
   uint32 res;
   uint32 baseq_id;
   JER2_ARAD_IPS_FLOW_ID_LOOKUP_TABLE_TBL_DATA ips_flow_id_lookup_table_tbl_data;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_K_QUARTET_RESET_UNSAFE);

   baseq_id = JER2_ARAD_IPQ_QRTT_TO_Q_ID(queue_k_quartet_ndx);
   DNX_SAND_ERR_IF_ABOVE_MAX(baseq_id, SOC_DNX_DEFS_GET(unit, nof_queues) - region_size, JER2_ARAD_QUEUE_ID_OUT_OF_RANGE_ERR, 10, exit);

   if (JER2_ARAD_IS_VOQ_MAPPING_INDIRECT(unit)) { /* indirect mapping mode */
      JER2_ARAD_BASE_Q_TO_SYS_PORT_TABLE_TBL_DATA system_physical_port_tbl_data;
      system_physical_port_tbl_data.sys_phy_port = JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID_IN_IPS;
      res = jer2_arad_indirect_base_queue_to_system_physical_port_tbl_region_set_unsafe(unit, core, queue_k_quartet_ndx, region_size, &system_physical_port_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);
   } else { /* direct mapping mode */
      JER2_ARAD_BASE_Q_TO_MOD_PORT_TABLE_TBL_DATA mod_port_tbl_data;
      mod_port_tbl_data.fap_id = JER2_ARAD_MAX_FAP_ID;
      mod_port_tbl_data.fap_port_id = JER2_ARAD_MAX_FAP_PORT_ID;
      res = jer2_arad_direct_base_queue_to_system_physical_port_tbl_region_set_unsafe(unit, core, queue_k_quartet_ndx, region_size, &mod_port_tbl_data);
      DNX_SAND_CHECK_FUNC_RESULT(res, 20, exit);
   }
   /*
    * Set base flow {
    */
   /* multiply by 4 from quartet to base flow */
   ips_flow_id_lookup_table_tbl_data.base_flow = JER2_ARAD_IPQ_INVALID_FLOW_QUARTET;
   ips_flow_id_lookup_table_tbl_data.sub_flow_mode = 0x0;

   res = jer2_arad_ips_flow_id_lookup_table_tbl_region_set_unsafe(unit, core, queue_k_quartet_ndx, region_size, &ips_flow_id_lookup_table_tbl_data);
   DNX_SAND_CHECK_FUNC_RESULT(res, 30, exit);
   /*
    * Set base flow }
    */
exit:

   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_k_quartet_reset_unsafe()", 0, 0);
}


uint32
jer2_arad_ipq_attached_flow_port_get_unsafe(
   DNX_SAND_IN  int  unit,
   DNX_SAND_IN  int  core,
   DNX_SAND_IN  uint32  queue_ndx,
   DNX_SAND_OUT uint32  *flow_id,
   DNX_SAND_OUT uint32  *sys_port
   ) {
   uint32
      res = DNX_SAND_OK;
   uint8
      is_interdigitated;
   JER2_ARAD_IPQ_QUARTET_MAP_INFO
      flow_quartet_info;
   uint32
      flow_offset_in_quartet,
      quartet_offset,
      base_flow;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_ATTACHED_FLOW_PORT_GET_UNSAFE);

   res = jer2_arad_ipq_queue_interdigitated_mode_get_unsafe(
      unit,
      core,
      queue_ndx / 1024,
      &is_interdigitated
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);
   res = jer2_arad_ipq_queue_to_flow_mapping_get_unsafe(
      unit,
      core,
      queue_ndx / 4,
      &flow_quartet_info
      );
   DNX_SAND_CHECK_FUNC_RESULT(res, 2, exit);
   *sys_port = flow_quartet_info.system_physical_port;

   base_flow = (flow_quartet_info.flow_quartet_index * 4);

   if (flow_quartet_info.is_composite) {
      if (is_interdigitated) {
         quartet_offset = (queue_ndx % 4);
         flow_offset_in_quartet = 2;
      } else {
         quartet_offset = (queue_ndx % 4) / 2;
         flow_offset_in_quartet = (queue_ndx % 2) * 2;
      }
   } else {
      if (is_interdigitated) {
         quartet_offset = (queue_ndx % 4) / 2;
         flow_offset_in_quartet = 2 + (queue_ndx % 2);
      } else {
         quartet_offset = 0;
         flow_offset_in_quartet = (queue_ndx % 4);
      }
   }

   *flow_id = ((base_flow) + (quartet_offset * 4) + (flow_offset_in_quartet));

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_attached_flow_port_get()", 0, 0);
}

uint32
jer2_arad_ipq_queue_id_verify(
   DNX_SAND_IN  int    unit,
   DNX_SAND_IN  uint32    queue_id
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_QUEUE_ID_VERIFY);

   DNX_SAND_ERR_IF_ABOVE_MAX(queue_id, SOC_DNX_DEFS_GET(unit, nof_queues) - 1, JER2_ARAD_IPQ_INVALID_QUEUE_ID_ERR, 10, exit);

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_queue_id_verify()", 0, 0);
}

/*********************************************************************
* NAME:
*     jer2_arad_cosq_ips_non_empty_queues_info_get_unsafe
* FUNCTION:
*     This function checks if the cosq queue is empty or not by direct reading the queue
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of the device to access.
*   DNX_SAND_IN  int                   core,
*     core identifier - It's not for Jerichho
*   DNX_SAND_IN  uint32                queue_id,
*     The queue id to be tested
*   OUTPT:
*   DNX_SAND_OUT uint32*               is_empty
*    Flaag that indicates if the queue is empty (1) or full (0)
* RETURNS:
*   OK or ERROR indication.
* REMARKS:
*   Called as part of the cosq delete sequance
*********************************************************************/
static uint32
jer2_arad_cosq_ips_non_empty_queues_info_get_unsafe(
   DNX_SAND_IN  int                   unit,
   DNX_SAND_IN  int                   core,
   DNX_SAND_IN  uint32                queue_id,
   DNX_SAND_OUT uint32*               is_empty
   )
{
    uint32 res = DNX_SAND_OK;
    JER2_ARAD_IQM_DYNAMIC_TBL_DATA iqm_dynamic_tbl;

    DNX_SAND_INIT_ERROR_DEFINITIONS(0);


    DNX_SAND_SOC_IF_ERROR_RETURN(res, 1, exit, MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_iqm_dynamic_tbl_get_unsafe, (unit,core,queue_id,&iqm_dynamic_tbl))); 

    *is_empty = iqm_dynamic_tbl.pq_inst_que_size > 0 ? 0 : 1;

exit:
    DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_cosq_ips_non_empty_queues_info_get_unsafe()", 0, 0);
}

#if JER2_ARAD_DEBUG

/*Each entry size no more than 4 words*/

uint32
jer2_arad_ips_non_empty_queues_info_get_unsafe(
   DNX_SAND_IN  int                   unit,
   DNX_SAND_IN  int                   core,
   DNX_SAND_IN  uint32                first_queue,
   DNX_SAND_IN  uint32                max_array_size,
   DNX_SAND_OUT dnx_soc_ips_queue_info_t* queues,
   DNX_SAND_OUT uint32*               nof_queues_filled,
   DNX_SAND_OUT uint32*               next_queue,
   DNX_SAND_OUT uint32*               reached_end
   )
{
   uint32 res = DNX_SAND_OK;
   JER2_ARAD_IQM_DYNAMIC_TBL_DATA iqm_dynamic_tbl;
   uint32
      nof_queues,
      queue_byte_size,
      entry_words;
   uint32
      k=0,
      queue_id;
   uint8 got_flow_info = FALSE;
   uint32
      local_fap,
      target_fap_id = SAL_UINT32_MAX,
      target_data_port_id = SAL_UINT32_MAX,
      system_physical_port,
      target_flow_id = 0;
   uint32 size,index_min,index_max;
   uint32 *entry_array=NULL, *entry=NULL;
   int core_index;
   soc_mem_t mem;
   soc_field_t field;

   DNX_SAND_INIT_ERROR_DEFINITIONS(0);

   (*nof_queues_filled)=0;
   (*reached_end)=1;

   if (SOC_IS_QAX(unit)) {
       mem = CGM_VOQ_SIZEm;
       field = WORDS_SIZEf;
   } else {
       mem = IQM_PQDMDm;
       field = PQ_INST_QUE_SIZEf;
   }

   DNX_SAND_SOC_IF_ERROR_RETURN(res, 2, exit, MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_mgmt_system_fap_id_get, (unit, &local_fap)));

   nof_queues = SOC_DNX_DEFS_GET(unit, nof_queues_per_pipe);

   /* If DMA is enabled, try to alloc buffer for it, and if suceeding use DMA */
   if (soc_mem_dmaable(unit, mem, SOC_MEM_BLOCK_ANY(unit, mem))) {
      /* read the table using DMA */
      size = SOC_MEM_TABLE_BYTES(unit, mem);
      index_max = soc_mem_index_max(unit, mem);
      index_min = soc_mem_index_min(unit, mem);
      if (SOC_IS_QAX(unit)) {
          entry_array = soc_cm_salloc(unit, size, "CGM_VOQ_SIZEm");
      } else {
          entry_array = soc_cm_salloc(unit, size, "IQM_PQDMDm");
      }
      if (entry_array == NULL) {
          DNX_SAND_SET_ERROR_CODE(DNX_SAND_GEN_ERR, 5, exit); 
      }
      if (SOC_IS_QAX(unit)) {
          DNX_SAND_SOC_IF_ERROR_RETURN(res, 6, exit, soc_mem_array_read_range(unit, mem, 0, CGM_BLOCK(unit, core), index_min, index_max, entry_array)); 
      } else {
          DNX_SAND_SOC_IF_ERROR_RETURN(res, 6, exit, soc_mem_array_read_range(unit, mem, 0, IQM_BLOCK(unit, core), index_min, index_max, entry_array)); 
      }
   }

   /*iterate over queues*/
   SOC_DNX_CORES_ITER(core, core_index) {
       for (queue_id = first_queue; queue_id < nof_queues; ++queue_id) {
          if (entry_array != NULL) {
             entry_words = soc_mem_entry_words(unit, mem);
             entry = entry_array + (entry_words * queue_id);
             soc_mem_field_get(unit, mem, entry, field, &queue_byte_size);
          } else {
             DNX_SAND_SOC_IF_ERROR_RETURN(res, 6, exit, MBCM_DNX_DRIVER_CALL(unit, mbcm_dnx_iqm_dynamic_tbl_get_unsafe, (unit,core_index,queue_id,&iqm_dynamic_tbl))); 
             queue_byte_size=iqm_dynamic_tbl.pq_inst_que_size;
          }

          /*Calc Queue size*/
          queue_byte_size =  queue_byte_size * JER2_ARAD_INGR_QUEUE_TABLE_BYTE_RESOL;

          if (queue_byte_size) { /*if queue isn't empty*/
              /*get flow information*/
              res = jer2_arad_ipq_attached_flow_port_get_unsafe(
                  unit,
                  core_index,
                  queue_id,
                  &target_flow_id,
                  &system_physical_port
                  );
              DNX_SAND_CHECK_FUNC_RESULT(res, 8, exit);
              
              DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
              if (system_physical_port != -1) {
                  res = jer2_arad_sys_phys_to_local_port_map_get(
                      unit,
                      system_physical_port,
                      &target_fap_id,
                      &target_data_port_id
                      );
                   DNX_SAND_CHECK_FUNC_RESULT(res, 10, exit);

                   if (target_fap_id == local_fap) {
                      got_flow_info = TRUE;
                   }
                }
#endif 
    
             /* filling the Queue array*/
             if ((*nof_queues_filled)<max_array_size)
             {
                 queues[k].queue_id=queue_id;
                 queues[k].queue_byte_size=queue_byte_size;
                 queues[k].target_flow_id=target_flow_id;
                 queues[k].got_flow_info=got_flow_info;
                 queues[k].target_fap_id=target_fap_id;
                 queues[k].target_data_port_id=target_data_port_id;
                 k++;
                 (*nof_queues_filled)++;
             } else { /* max_array_size reached,save next queue id and break */
                 (*next_queue)=queue_id;
                 (*reached_end)=0;
                 break;
             }
          }
       }
   }

exit:
   if (entry_array){
      soc_cm_sfree(unit, entry_array);
   }

   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ips_non_empty_queues_print_unsafe()", 0, 0);
}
#endif

uint32
jer2_arad_ipq_tc_profile_set_unsafe(
   DNX_SAND_IN      int         unit,
   DNX_SAND_IN      int         core,
   DNX_SAND_IN      uint8       is_flow_ndx,
   DNX_SAND_IN      uint32      dest_ndx,
   DNX_SAND_IN      uint32      tc_profile
   ) {
   uint32
      line_ndx,
      tbl_data,
      fld_val;
   soc_field_t profile_id;
   uint32 res;
   int core_id;
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TC_PROFILE_SET_UNSAFE);

   fld_val = tc_profile;

   if (is_flow_ndx) {
       /* Destination is flow index */
       SOC_DNX_CORES_ITER(core, core_id) {
           line_ndx = DNX_SAND_DIV_ROUND_DOWN(dest_ndx, 16);
           profile_id = (DNX_SAND_DIV_ROUND_DOWN(dest_ndx, 4)) % 4;

           if (SOC_IS_QAX(unit)) {
               DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, READ_TAR_FLOW_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));
           } else {
               DNX_SAND_SOC_IF_ERROR_RETURN(res, 1000, exit, soc_mem_array_read(unit, IRR_FLOW_TABLEm, core_id, MEM_BLOCK_ANY, line_ndx, &tbl_data));
           }

           /* Convert profile id to profile field */
           switch (profile_id) {
               case 0:
                   profile_id = PROFILE_0f;
                   break;
               case 1:
                   profile_id = PROFILE_1f;
                   break;
               case 2:
                   profile_id = PROFILE_2f;
                   break;
               case 3:
                   profile_id = PROFILE_3f;
                   break;
           }
           if (SOC_IS_QAX(unit)) {
               soc_TAR_FLOW_TABLEm_field_set(unit, &tbl_data, profile_id, &fld_val);
               DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, WRITE_TAR_FLOW_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));
           } else {
               soc_IRR_FLOW_TABLEm_field_set(unit, &tbl_data, profile_id, &fld_val);
               DNX_SAND_SOC_IF_ERROR_RETURN(res, 1010, exit, soc_mem_array_write(unit, IRR_FLOW_TABLEm, core_id, MEM_BLOCK_ANY, line_ndx, &tbl_data));
           }
       }
   } else { /* Unicast mapping */
       line_ndx = dest_ndx;

       if (SOC_IS_QAX(unit)) {
           DNX_SAND_SOC_IF_ERROR_RETURN(res, 1020, exit, READ_TAR_DESTINATION_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));

           soc_IRR_DESTINATION_TABLEm_field_set(unit, &tbl_data, TC_PROFILEf, &fld_val);

           DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, WRITE_TAR_DESTINATION_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));
       } else {
           DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, READ_IRR_DESTINATION_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));
           soc_IRR_DESTINATION_TABLEm_field_set(unit, &tbl_data, TC_PROFILEf, &fld_val);

           DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, WRITE_IRR_DESTINATION_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));
       }
   }

   JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_tc_profile_set_unsafe()", 0, 0);
}

uint32
jer2_arad_ipq_tc_profile_get_unsafe(
   DNX_SAND_IN      int         unit,
   DNX_SAND_IN      int         core,
   DNX_SAND_IN      uint8       is_flow_ndx,
   DNX_SAND_IN      uint32      dest_ndx,
   DNX_SAND_OUT     uint32      *tc_profile
   ) {
   uint32
      line_ndx,
      profile_id,
      tbl_data;
   uint32 res;
   int
       core_id = (core == SOC_CORE_ALL) ? 0 : core;

   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TC_PROFILE_GET_UNSAFE);
   if (is_flow_ndx) {
      /* Destination is flow index */
      line_ndx = DNX_SAND_DIV_ROUND_DOWN(dest_ndx, 16);
      profile_id = (DNX_SAND_DIV_ROUND_DOWN(dest_ndx, 4)) % 4;

      if (SOC_IS_QAX(unit)) {
          DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, READ_TAR_FLOW_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));
      } else {
          DNX_SAND_SOC_IF_ERROR_RETURN(res, 1130, exit, soc_mem_array_read(unit, IRR_FLOW_TABLEm, core_id, MEM_BLOCK_ANY, line_ndx, &tbl_data));
      }

      /* Convert profile id to profile field */
      switch (profile_id) {
      case 0:
         profile_id = PROFILE_0f;
         break;
      case 1:
         profile_id = PROFILE_1f;
         break;
      case 2:
         profile_id = PROFILE_2f;
         break;
      case 3:
         profile_id = PROFILE_3f;
         break;
      }

      /* Read the TC Profile from the table data */
      if (SOC_IS_QAX(unit)) {
          soc_TAR_FLOW_TABLEm_field_get(unit, &tbl_data, profile_id, tc_profile);
      } else {
          soc_IRR_FLOW_TABLEm_field_get(unit, &tbl_data, profile_id, tc_profile);
      }
   } else {
      line_ndx = dest_ndx;

      if (SOC_IS_QAX(unit)) {
          /* Read the required line from the table data */
          DNX_SAND_SOC_IF_ERROR_RETURN(res, 1140, exit, READ_TAR_DESTINATION_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));

          /* Read the TC Profile from the table data */
          soc_TAR_DESTINATION_TABLEm_field_get(unit, &tbl_data, TC_PROFILEf, tc_profile);
      } else {
          /* Read the required line from the table data */
          DNX_SAND_SOC_IF_ERROR_RETURN(res, 1030, exit, READ_IRR_DESTINATION_TABLEm(unit, MEM_BLOCK_ANY, line_ndx, &tbl_data));

          /* Read the TC Profile from the table data */
          soc_IRR_DESTINATION_TABLEm_field_get(unit, &tbl_data, TC_PROFILEf, tc_profile);
      }
   }

   JER2_ARAD_DO_NOTHING_AND_EXIT;
exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_tc_profile_get_unsafe()", 0, 0);
}

uint32
jer2_arad_ipq_tc_profile_verify(
   DNX_SAND_IN      int         unit,
   DNX_SAND_IN      uint8       is_flow_ndx,
   DNX_SAND_IN      uint32      dest_ndx,
   DNX_SAND_IN      uint32      tc_profile
   ) {
   DNX_SAND_INIT_ERROR_DEFINITIONS(JER2_ARAD_IPQ_TC_PROFILE_VERIFY);

   if (is_flow_ndx) {
      DNX_SAND_ERR_IF_ABOVE_MAX(dest_ndx, SOC_DNX_DEFS_GET((unit), nof_flows) - 1, JER2_ARAD_IPQ_INVALID_FLOW_ID_ERR, 10, exit);
      DNX_SAND_ERR_IF_ABOVE_MAX(tc_profile, JER2_ARAD_NOF_INGRESS_FLOW_TC_MAPPING_PROFILES - 1, JER2_ARAD_IPQ_INVALID_TC_PROFILE_ERR, 20, exit);
   } else {
      DNX_SAND_ERR_IF_ABOVE_MAX(dest_ndx, JER2_ARAD_MAX_SYSTEM_PHYSICAL_PORT_ID, JER2_ARAD_IPQ_INVALID_SYS_PORT_ERR, 30, exit);
      DNX_SAND_ERR_IF_ABOVE_MAX(tc_profile, JER2_ARAD_NOF_INGRESS_UC_TC_MAPPING_PROFILES - 1, JER2_ARAD_IPQ_INVALID_TC_PROFILE_ERR, 40, exit);
   }

exit:
   DNX_SAND_EXIT_AND_SEND_ERROR("error in jer2_arad_ipq_tc_profile_verify()", dest_ndx, tc_profile);
}


/* size (in entries) if the tables for dma buffer calculations */
#define IRR_DESTINATION_TABLE_SIZE 32768
#define IPS_QPM_1_NO_SYS_RED_SIZE 24576
#define IPS_QPM_2_SIZE 6144
/* The entries of all three tables are not above 32 bits and fit in one uint32 */
#define DMA_ALLOC_WORDS (IRR_DESTINATION_TABLE_SIZE + IPS_QPM_1_NO_SYS_RED_SIZE + IPS_QPM_1_NO_SYS_RED_SIZE)


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

#endif /* of #if defined(BCM_88690_A0) */

