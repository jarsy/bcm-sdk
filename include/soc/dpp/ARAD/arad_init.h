/* $Id: arad_init.h,v 1.10 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __ARAD_INIT_INCLUDED__
/* { */
#define __ARAD_INIT_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/ARAD/arad_api_mgmt.h>
/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/* } */

/*************
 * MACROS    *
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

uint32 arad_mgmt_init_pll_reset(
    SOC_SAND_IN  int            unit,
    SOC_SAND_IN  ARAD_MGMT_INIT *init);

uint32 arad_mgmt_pon_init(
    SOC_SAND_IN  int                                unit);

uint32
  arad_mgmt_ihb_tbls_init(
    SOC_SAND_IN  int                 unit
  );

uint32 
arad_init_pdm_nof_entries_calc(
    SOC_SAND_IN  int                     unit,
    SOC_SAND_IN  ARAD_INIT_PDM_MODE         pdm_mode,
    SOC_SAND_OUT uint32                     *pdm_nof_entries
   );

/*********************************************************************
* NAME:
*     arad_mgmt_init_sequence_phase1_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Initialize the device, including:1. Prevent all the
*     control cells. 2. Initialize the device tables and
*     registers to default values. 3. Initialize
*     board-specific hardware interfaces according to
*     configurable information, as passed in 'hw_adjust'. 4.
*     Perform basic device initialization. The configuration
*     can be enabled/disabled as passed in 'enable_info'.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_HW_ADJUSTMENTS      *hw_adjust -
*     Contains user-defined initialization information for
*     hardware interfaces.
*  SOC_SAND_IN  ARAD_INIT_BASIC_CONF     *basic_conf -
*     Basic configuration that must be configured for all
*     systems - credit worth, dram buffers configuration etc.
*  SOC_SAND_IN  ARAD_INIT_PORTS          *fap_ports -
*     local FAP ports configuration - header parsing type,
*     mapping to NIF etc.
*  SOC_SAND_INOUT ARAD_INIT_DROP_AND_FC    *drp_and_fc -
*     local packet drop and flow control configurations -
*     ingress, egress, nif, scheduler etc.
*  SOC_SAND_IN  uint8                 silent -
*     If TRUE, progress printing will be suppressed.
* REMARKS:
*     1. For all configurations that can be done per-direction
*     (e.g. NIF - rx/tx, FAP - incoming/outgoing) - the
*     configuration is performed for both directions if
*     enabled. It may be overridden before phase2 if needed.
*     2. For all input structures, NULL pointer may be passed.
*     If input structure is passed as NULL, the appropriate
*     configuration will not be performed.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  arad_mgmt_init_sequence_phase1_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN     ARAD_MGMT_INIT           *init,
    SOC_SAND_IN  uint8                 silent
  );

uint32
  arad_mgmt_init_sequence_phase1_verify(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN     ARAD_MGMT_INIT           *init
  );

/*********************************************************************
* NAME:
*     arad_mgmt_init_sequence_phase2_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     Out-of-reset sequence. Enable/Disable the device from
*     receiving and transmitting control cells.
* INPUT:
*  SOC_SAND_IN  int                 unit -
*     Identifier of the device to access.
*  SOC_SAND_IN  ARAD_INIT_OOR            *oor_info -
*     Out Of Reset configuration. Some blocks need to be set
*     out of reset before traffic can be enabled.
*  SOC_SAND_IN  uint8                 silent -
*     TRUE - Print progress messages. FALSE - Do not print
*     progress messages.
* REMARKS:
*     1. After phase 2 initialization, traffic can be enabled.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
arad_ser_init(int unit);

uint32 arad_mgmt_ipt_init(
    SOC_SAND_IN  int                 unit
);

uint32
  arad_mgmt_init_sequence_phase2_unsafe(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 silent
  );

int _arad_mgmt_irr_tbls_init_dma_callback(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN int copyno, 
    SOC_SAND_IN int array_index, 
    SOC_SAND_IN int index, 
    SOC_SAND_OUT uint32 *value, 
    SOC_SAND_IN int entry_sz, 
    SOC_SAND_IN void *opaque);

uint32
  arad_mgmt_tbls_init(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 silent
  );

/*
 * Init ALL Arad tables that are not initialized elsewhere
 */

uint32
  arad_mgmt_all_tbls_init(
    SOC_SAND_IN  int                 unit,
    SOC_SAND_IN  uint8                 silent
  );

uint32
  arad_mgmt_ips_tbls_init(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  uint8                 silent
    );

uint32
  arad_mgmt_ipt_tbls_init(
      SOC_SAND_IN  int                 unit,
      SOC_SAND_IN  uint8                 silent
    );

uint32
  arad_init_dram_fbc_buffs_get(
    SOC_SAND_IN  uint32  buffs_without_fbc,
    SOC_SAND_IN  uint32  buff_size_bytes,
    SOC_SAND_OUT uint32 *fbc_nof_bufs
  );

/*********************************************************************
*     Initialize MESH_TOPOLOGY block with default values
*********************************************************************/
uint32
  arad_init_mesh_topology(
    SOC_SAND_IN int             unit
  );

/*********************************************************************
*     Initialize Stacking application
*********************************************************************/
uint32
  arad_mgmt_stk_init(
    SOC_SAND_IN  int                          unit,
    SOC_SAND_IN  ARAD_MGMT_INIT               *init
  );

/*********************************************************************
*     Perform an mbist check on Arad.
*     Stop on errors or not depending on skip_errors.
*********************************************************************/
int soc_bist_all_arad(const int unit, const int skip_errors);
int soc_bist_irdp_arad(const int unit, const int skip_errors);
int soc_bist_arad_ser_test(const int unit, const int skip_errors, uint32 nof_repeats, uint32 time_to_wait, uint32 ser_test_num);
int soc_bist_all_ardon(const int unit, const int skip_errors);
int soc_bist_irfc_ardon(const int unit, const int skip_errors);

/* returns the mask for ecc masking according to active ILKN lanes */
uint32
arad_mgmt_nbi_ecc_mask_get(
      SOC_SAND_IN int                unit,
      SOC_SAND_OUT uint64            *mask
      );

uint32      arad_iqm_workaround(SOC_SAND_IN  int    unit);
int         arad_activate_power_savings(int unit);
uint32      arad_mgmt_init_finalize(SOC_SAND_IN int unit);

/* Arad DMA init */
int soc_dpp_arad_dma_init(int unit);

/* } */

#include <soc/dpp/SAND/Utils/sand_footer.h>


/* } __ARAD_INIT_INCLUDED__*/
#endif


