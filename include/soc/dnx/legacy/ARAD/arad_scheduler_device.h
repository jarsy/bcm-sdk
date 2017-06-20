/* $Id: jer2_arad_scheduler_device.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/


#ifndef __JER2_ARAD_SCHEDULER_DEVICE_H_INCLUDED__
/* { */
#define __JER2_ARAD_SCHEDULER_DEVICE_H_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/ARAD/arad_api_end2end_scheduler.h>
/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/*
 * The device rate is determined by the interval between 2 consecutive credits
 * configured in 1/128th of a clock period units.
 */
#define JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_RESOLUTION (128)

/*
 * Minimum Device Rate Entry interval value (if enabled) is 128,
 * which results in 128 * (clock/128) = 1 clock (eq to ~250 Gbit/sec in 256 credit size)
 */
#define JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_MIN (JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_RESOLUTION)

/*
 * Maximum Device Rate Entry interval value (if enabled) is 1048575,
 * which results in 1048575 clock/128 = 8192 clock (eq to ~82 Mbit/sec in 256 credit size)
 */

#define JER2_ARAD_SCH_DEVICE_RATE_INTERVAL_MAX (1048575)

/*
 * Device interface weight - maximal value corresponds to lowest priority.
 * The weight  is in range 0 - 1023, where 0 means the weight is
 * disabled and 1 is the highest priority.
 */
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_MIN 0
#define JER2_ARAD_SCH_DEVICE_IF_WEIGHT_MAX 1023

/* } */

/*************
 *  MACROS   *
 *************/
/* { */
#define JER2_ARAD_SCH_IS_CHNIF_ID(unit, offset) (offset < SOC_DNX_IMP_DEFS_GET(unit, nof_channelized_interfaces))  
#define JER2_ARAD_SCH_OFFSET_TO_NON_CHANNELIZED_OFFSET(unit, sch_offset) (sch_offset - SOC_DNX_IMP_DEFS_GET(unit, nof_channelized_interfaces)) 
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
*     Converts from interface index as represented by
*     JER2_ARAD_INTERFACE_ID (when used in Arad-A mode) or internal id
*     (JER2_ARAD_INTERFACE_ID in jer2_arad-B mode converted to internal using
*     soc_pb_nif2intern_id), to the offset for the relevant
*     scheduler register (channelized or 1-port).
*********************************************************************/
uint32
  jer2_arad_sch_if2sched_offset(
    uint32 nif_id
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_device_rate_entry_set_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This function sets an entry in the device rate table.
*     Each entry sets a credit generation rate, for a given
*     pair of fabric congestion (presented by rci_level) and
*     the number of active fabric links. The driver writes to
*     the following tables: Device Rate Memory (DRM)
* INPUT:
*  DNX_SAND_IN  uint32                 rci_level_ndx -
*     RCI bucket level. Range: 0 - 7
*  DNX_SAND_IN  uint32                 nof_active_links_ndx -
*     Number of current active links range: 0 - 36
*  DNX_SAND_IN  uint32                  rate -
*     The credit generation rate, in Mega-Bit-Sec. If 0 - no
*     credits are generated.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_device_rate_entry_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rci_level_ndx,
    DNX_SAND_IN  uint32                 nof_active_links_ndx,
    DNX_SAND_IN  uint32                  rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_device_rate_entry_verify
* TYPE:
*   PROC
* FUNCTION:
*     This function sets an entry in the device rate table.
*     Each entry sets a credit generation rate, for a given
*     pair of fabric congestion (presented by rci_level) and
*     the number of active fabric links. The driver writes to
*     the following tables: Device Rate Memory (DRM)
* INPUT:
*  DNX_SAND_IN  uint32                 rci_level_ndx -
*     RCI bucket level. Range: 0 - 7
*  DNX_SAND_IN  uint32                 nof_active_links_ndx -
*     Number of current active links range: 0 - 36
*  DNX_SAND_IN  uint32                  rate -
*     The credit generation rate, in Mega-Bit-Sec. If 0 - no
*     credits are generated.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_device_rate_entry_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rci_level_ndx,
    DNX_SAND_IN  uint32                 nof_active_links_ndx,
    DNX_SAND_IN  uint32                  rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_device_rate_entry_get_unsafe
* TYPE:
*   PROC
* FUNCTION:
*     This function sets an entry in the device rate table.
*     Each entry sets a credit generation rate, for a given
*     pair of fabric congestion (presented by rci_level) and
*     the number of active fabric links. The driver writes to
*     the following tables: Device Rate Memory (DRM)
* INPUT:
*  DNX_SAND_IN  uint32                 rci_level_ndx -
*     RCI bucket level. Range: 0 - 7
*  DNX_SAND_IN  uint32                 nof_active_links_ndx -
*     Number of current active links range: 0 - 36
*  DNX_SAND_OUT uint32                  *rate -
*     The credit generation rate, in Mega-Bit-Sec. If 0 - no
*     credits are generated.
* REMARKS:
*     None.
* RETURNS:
*     OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_device_rate_entry_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32                 rci_level_ndx,
    DNX_SAND_IN  uint32                 nof_active_links_ndx,
    DNX_SAND_OUT uint32                  *rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_if_shaper_rate_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling & CPU) its maximal credit rate. This API is
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS
*     section below.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index, consists of interface type, and an
*     interface index for network interfaces.
*  DNX_SAND_IN  uint32                  if_rate -
*     Maximum credit rate in Kilo-Bit-Sec.
*  DNX_SAND_OUT uint32                  *exact_if_rate -
*     Loaded with the actual written values. These can differ
*     from the given values due to rounding.
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*   This function must only be called for Channelized interfaces.
*   For one-port-interfaces, including OLP and ERP, rate configuration
*   is done according to port_rate, as part of
*   egress port configuration.
*********************************************************************/
int
  jer2_arad_sch_if_shaper_rate_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  int                 core,
    DNX_SAND_IN  uint32              tm_port,
    DNX_SAND_IN  uint32              if_rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_if_shaper_rate_verify
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling & CPU) its maximal credit rate. This API is
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS
*     section below.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index, consists of interface type, and an
*     interface index for network interfaces.
*  DNX_SAND_IN  uint32                  if_rate -
*     Maximum credit rate in Kilo-Bit-Sec.
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*   This function must only be called for Channelized interfaces.
*   For one-port-interfaces, including OLP and ERP, rate configuration
*   is done according to port_rate, as part of
*   egress port configuration.
*********************************************************************/
uint32
  jer2_arad_sch_if_shaper_rate_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx,
    DNX_SAND_IN  uint32              if_rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_if_shaper_rate_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling & CPU) its maximal credit rate. This API is
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS
*     section below.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index, consists of interface type, and an
*     interface index for network interfaces.
*  DNX_SAND_OUT uint32                  *if_rate -
*     Maximum credit rate in Kilo-Bit-Sec.
* RETURNS:
*     OK or ERROR indication.
* REMARKS:
*   This function must only be called for Channelized interfaces.
*   For one-port-interfaces, including OLP and ERP, rate configuration
*   is done according to port_rate, as part of
*   egress port configuration.
*********************************************************************/
int
  jer2_arad_sch_if_shaper_rate_get(
    DNX_SAND_IN  int           unit,
    DNX_SAND_IN  int           core,
    DNX_SAND_IN  uint32        tm_port,
    DNX_SAND_OUT uint32        *if_rate
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_device_if_weight_idx_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling, OLP, ERP) its weight index. Range: 0-7. The
*     actual weight value (one of 8, configurable) is in range
*     1-1023, 0 meaning inactive interface. This API is only
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS section
*     below.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index, consists of interface type, and an
*     interface index for network interfaces.
*  DNX_SAND_IN  uint32                  weight_index -
*     Interface weight index. Range: 0-7. Selects one of 8
*     configurable weights for interfaces WFQ.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_device_if_weight_idx_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_port_t          port,
    DNX_SAND_IN  uint32              weight_index
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_device_if_weight_idx_verify
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling, OLP, ERP) its weight index. Range: 0-7. The
*     actual weight value (one of 8, configurable) is in range
*     1-1023, 0 meaning inactive interface. This API is only
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS section
*     below.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index, consists of interface type, and an
*     interface index for network interfaces.
*  DNX_SAND_IN  uint32                  weight_index -
*     Interface weight index. Range: 0-7. Selects one of 8
*     configurable weights for interfaces WFQ.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_device_if_weight_idx_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  uint32              weight_index
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_device_if_weight_idx_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     Sets, for a specified device interface, (NIF-Ports,
*     recycling, OLP, ERP) its weight index. Range: 0-7. The
*     actual weight value (one of 8, configurable) is in range
*     1-1023, 0 meaning inactive interface. This API is only
*     only valid for Channelized interface id-s (0, 4, 8... for NIF) - see REMARKS section
*     below.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_INTERFACE_ID    if_ndx -
*     Interface index, consists of interface type, and an
*     interface index for network interfaces.
*  DNX_SAND_OUT uint32                  *weight_index -
*     Interface weight index. Range: 0-7. Selects one of 8
*     configurable weights for interfaces WFQ.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_device_if_weight_idx_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  soc_port_t          port,
    DNX_SAND_OUT uint32              *weight_index
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_if_weight_conf_set_unsafe
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     This function sets the device interfaces scheduler
*     weight configuration. Up to 8 weight configuration can
*     be pre-configured. Each scheduler interface will be
*     configured to use one of these pre-configured weights.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHTS      *if_weights -
*     The weighs configuration for the device interfaces
*     scheduler.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_if_weight_conf_set_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHTS      *if_weights
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_if_weight_conf_verify
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     This function sets the device interfaces scheduler
*     weight configuration. Up to 8 weight configuration can
*     be pre-configured. Each scheduler interface will be
*     configured to use one of these pre-configured weights.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHTS      *if_weights -
*     The weighs configuration for the device interfaces
*     scheduler.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_if_weight_conf_verify(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  JER2_ARAD_SCH_IF_WEIGHTS      *if_weights
  );

/*********************************************************************
* NAME:
*     jer2_arad_sch_if_weight_conf_get_unsafe
* TYPE:
*   PROC
* DATE:
*   Dec  2 2007
* FUNCTION:
*     This function sets the device interfaces scheduler
*     weight configuration. Up to 8 weight configuration can
*     be pre-configured. Each scheduler interface will be
*     configured to use one of these pre-configured weights.
* INPUT:
*  DNX_SAND_IN  int                 unit -
*     Identifier of device to access.
*  DNX_SAND_OUT JER2_ARAD_SCH_IF_WEIGHTS      *if_weights -
*     The weighs configuration for the device interfaces
*     scheduler.
* RETURNS:
*   OK or ERROR indication.
*********************************************************************/
uint32
  jer2_arad_sch_if_weight_conf_get_unsafe(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT JER2_ARAD_SCH_IF_WEIGHTS      *if_weights
  );


/*********************************************************************
*     Sets, for a specified device MAC LANE or equivalent, (NIF-Ports,
*     recycling & CPU) its actual credit rate (sum of ports). This API is
*     only valid for Channelized interfaces - see REMARKS
*     section below.
*     Note: for SGMII - configures only the first interface in MAL
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  jer2_arad_sch_ch_if_rate_set_unsafe(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            sch_offset,
    DNX_SAND_IN     uint32            rate
  );

uint32
  jer2_arad_sch_ch_if_rate_get_unsafe(
    DNX_SAND_IN     int               unit,
    DNX_SAND_IN     int               core,
    DNX_SAND_IN     uint32            sch_offset,
    DNX_SAND_OUT    uint32            *rate
  );
/* } */


#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>


/* } __JER2_ARAD_SCHEDULER_DEVICE_H_INCLUDED_*/
#endif

