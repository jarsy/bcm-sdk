/* $Id: jer2_jer2_jer2_tmc_api_mgmt.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_MGMT_INCLUDED__
/* { */
#define __DNX_TMC_API_MGMT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/SAND_FM/sand_chip_defines.h>
#include <soc/dnx/legacy/TMC/tmc_api_general.h>
#include <soc/dnx/legacy/TMC/tmc_api_ports.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* $Id: jer2_jer2_jer2_tmc_api_mgmt.h,v 1.9 Broadcom SDK $
 *  Minimal credit size, in Byte resolution
 */

/*
 *  Maximal credit size, in Byte resolution
 */


/*     Maximal length of DRAM configuration sequence.          */

/*     Maximal number of DRAM interfaces.                      */

/*     Out of total of 15 SerDes quartets, two (one per
*     internal NIF Group consisting of 4 MAL-s) may be
*     assigned to either Network or Fabric interfaces.        */


/*
 *  This value disables limitation based on external (original)
 *  packet size
 */
#define DNX_TMC_MGMT_PCKT_SIZE_EXTERN_NO_LIMIT 0


/*
  * Indicator to the Software that the device was initialized, and traffic was enabled
  * Upon warm-start initialization, the software may read this register, and see
  * if the traffic was enabled.
  * If it was, the SW may utilize the SSR capabilities, to initialize the software,
  * without affecting the device
  * Used by soc_jer2_jer2_jer2_tmcmgmt_enable_traffic_set()
  */

#define DNX_TMC_MGMT_OCB_VOQ_NOF_THRESHOLDS  (2) 

#define DNX_TMC_MGMT_OCB_PRM_EN_TH_DEFAULT   (0x17f)   


#define DNX_TMC_FAP_CREDIT_VALUE_LOCAL 0
#define DNX_TMC_FAP_CREDIT_VALUE_REMOTE 1
#define DNX_TMC_FAP_CREDIT_VALUE_MAX 1


/*
 *  Minimal credit size, in Byte resolution
 */
#define DNX_TMC_CREDIT_SIZE_BYTES_MIN 1

/*
 *  Maximal credit size, in Byte resolution
 */
#define DNX_TMC_CREDIT_SIZE_BYTES_MAX ((8 * 1024) - 1)




/*************
 * MACROS    *
 *************/
/* { */




/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */


typedef enum
{
  /*
   * simple
   */
  DNX_TMC_INIT_PDM_MODE_SIMPLE = 0,
  /*
   * Reduced
   */
  DNX_TMC_INIT_PDM_MODE_REDUCED = 1,
  /*
   * numbet of modes
   */
  DNX_TMC_INIT_PDM_NOF_MODES = 2
} DNX_TMC_INIT_PDM_MODE;

typedef enum
{
  DNX_TMC_MGMT_CORE_CLK_RATE_250MHZ = 250,
  DNX_TMC_MGMT_CORE_CLK_RATE_300MHZ = 300
}DNX_TMC_MGMT_CORE_CLK_RATE;

typedef enum
{
  /*
   *  FTMH type - Soc_petra
   */
  DNX_TMC_MGMT_FABRIC_HDR_TYPE_PETRA = 0,
  /*
   *  FTMH type - fap20 compatible
   */
  DNX_TMC_MGMT_FABRIC_HDR_TYPE_FAP20 = 1,
  /*
   *  FTMH type - fap10m compatible
   */
  DNX_TMC_MGMT_FABRIC_HDR_TYPE_FAP10M = 2,
  /*
   *  Total number of supported FTMH types.
   */
  DNX_TMC_MGMT_NOF_FABRIC_HDR_TYPES = 3
}DNX_TMC_MGMT_FABRIC_HDR_TYPE;

typedef enum
{
  /*
   *  The traffic mode is High-Priority Packet Mode, which
   *  mixes CBR and data (non-CBR) traffic. In Soc_petra-B
   *  this mode must be used if data traffic is expected.
   *  In JER2_ARAD, this mode is used when no Bypass TDM ports
   *  are avaiable.
   */
  DNX_TMC_MGMT_TDM_MODE_PACKET = 0,
  /*
   *  TDM Cells traffic mode with an Optimized FTMH Header
   *  format. If set, all the devices this device can
   *  communicate with must be configured with the same mode.
   *  In JER2_ARAD, device can have mixed traffic, all bypass TDM ports
   *  are set with Optimizied FTMH Header.
   */
  DNX_TMC_MGMT_TDM_MODE_TDM_OPT = 1,
  /*
   *  TDM Cells traffic mode with a Standard FTMH Header
   *  format. In this mode, the device can communicate with
   *  devices configured in the same mode or in a 'PACKET'
   *  mode.
   *  In JER2_ARAD, device can have mixed traffic, all bypass TDM ports
   *  are set with Standard FTMH Header.
   */
  DNX_TMC_MGMT_TDM_MODE_TDM_STA = 2,
  /*
   *  Number of types in DNX_TMC_MGMT_TDM_MODE
   */
  DNX_TMC_MGMT_NOF_TDM_MODES = 3
}DNX_TMC_MGMT_TDM_MODE;

typedef enum
{
  /*
   *  Limit the Packet Size, sampled before ingress editing.
   */
  DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_EXTERN = 0,
  /*
   *  Limit the Packet Size, sampled after ingress editing.
   */
  DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_INTERN = 1,
  /*
   *  Number of types in DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE
   */
  DNX_TMC_MGMT_NOF_PCKT_SIZE_CONF_MODES = 2
}DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Minimal allowed packet size, bytes. Packets below this
   *  value will be dropped.
   */
  uint32 min;
  /*
   *  Maximal allowed packet size, bytes. Packets above this
   *  value will be dropped.
   */
  uint32 max;
} DNX_TMC_MGMT_PCKT_SIZE;

typedef struct{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   * is the queue eligible to use ocb
   */
  uint8 voq_eligible;
  /*If set, queue will use DRAM eligibility test*/
  uint8 dram_admission_exempt;
  /*
   * words thresholds
   */
  uint32 th_words[DNX_TMC_MGMT_OCB_VOQ_NOF_THRESHOLDS];
  /*
   * Buffer thresholds
   */
  uint32 th_buffers[DNX_TMC_MGMT_OCB_VOQ_NOF_THRESHOLDS];
}DNX_TMC_MGMT_OCB_VOQ_INFO;

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

void
  DNX_TMC_MGMT_PCKT_SIZE_clear(
    DNX_SAND_OUT DNX_TMC_MGMT_PCKT_SIZE *info
  );

void
  DNX_TMC_MGMT_OCB_VOQ_INFO_clear(
    DNX_SAND_OUT DNX_TMC_MGMT_OCB_VOQ_INFO *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_MGMT_FABRIC_HDR_TYPE_to_string(
    DNX_SAND_IN DNX_TMC_MGMT_FABRIC_HDR_TYPE enum_val
  );

const char*
  DNX_TMC_MGMT_TDM_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_MGMT_TDM_MODE enum_val
  );

const char*
  DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_MGMT_PCKT_SIZE_CONF_MODE enum_val
  );

void
  DNX_TMC_MGMT_PCKT_SIZE_print(
    DNX_SAND_IN  DNX_TMC_MGMT_PCKT_SIZE *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_MGMT_INCLUDED__*/
#endif
