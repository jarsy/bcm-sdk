/*
 * $Id: trunk.h,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains TRUNK definitions internal to the BCM library.
 */

#ifndef _BCM_INT_SBX_TRUNK_H_
#define _BCM_INT_SBX_TRUNK_H_

#include <bcm/trunk.h>
#include <bcm_int/common/trunk.h>

/*
 *  This indicates how many aggregates we support on Sirius.  Basically, the
 *  normal value is the number of targets divided by two, under the assumption
 *  that the minimum useful number of links in an aggregate is two (this
 *  assumption ignores whether any of those links are actually down).
 */
#define _SIRIUS_LAG_COUNT (_SIRIUS_MC_MAX_TARGETS / 2)

/*
 *  'Fabric' aggregates refer in Sirius case to 'higig' aggregates, and are
 *  applicable only when a higig port is speaking HIGIG2 (XGS mode).
 *
 *  The _SIRIUS_FAB_LAG_MIN value must be power-of-two aligned.
 *
 *  WARNING: _SIRIUS_FAB_LAG_MAX changes may require adjustments to
 *  _MC_CACHE_EXTRA_HGA_MASK in the sirius/multicast.c module.
 */
#define _SIRIUS_FAB_LAG_MIN 0x00001000
#define _SIRIUS_FAB_LAG_COUNT (SB_FAB_DEVICE_SIRIUS_NUM_HG_LINE_PORTS + _SIRIUS_LAG_COUNT)
#define _SIRIUS_FAB_LAG_MAX_PORTS BCM_TRUNK_MAX_PORTCNT
#define _SIRIUS_FAB_LAG_MAX (_SIRIUS_FAB_LAG_MIN + _SIRIUS_FAB_LAG_COUNT - 1)


/*
 * Function:
 *    bcm_fe2000_trunk_vlan_remove_port
 * Purpose:
 *      When a port is removed from a VLAN, the vlan code calls this function
 *      to fix up the McPort2Etc table.
 * Parameters:
 *      unit - Device unit number.
 *      vid  - Vlan port is being removed from.
 *      port - The port being removed.
 * Returns:
 *      BCM_E_NONE      - Success or port not part of any lag.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      If the port is not a member of any lag, no action is taken
 */

int
bcm_fe2000_trunk_vlan_remove_port(int unit, bcm_vlan_t vid, bcm_port_t port);

/*
 *  Function
 *    bcm_qe2000_trunk_vlan_port_adjust
 * Purpose
 *    Adjust ports in a VLAN so that if there are any ports from an aggregate,
 *    only the designate for that aggregate will show up, and if there are no
 *    ports from an aggregate, nothing will show up from that aggregate.
 *  Arguments
 *    (in) int unit = the unit number on which to operate
 *    (in) bcm_vlan_t vid = the VLAN on which to operate
 *    (in) bcm_pbmp_t ports = the ports now in the VLAN
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
extern int
bcm_qe2000_trunk_vlan_port_adjust(const int unit,
                                 const bcm_vlan_t vid,
                                 const bcm_pbmp_t pbmp);

/*
 *  Function
 *    bcm_sirius_trunk_find_and_get
 *  Purpose
 *    Find the aggregate, given a target that is a member, and also get the
 *    'set' information for the aggregate.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_module_t modid = the module ID to check (with port ID)
 *    (in) bcm_port_t port = the port ID to check (with module ID)
 *    (out) bcm_trunk_id *tid = where to put the aggregate ID
 *    (out) bcm_trunk_info_t *t_data = where to put aggregate information
 *    (in) int member_max = max number of members to retrieve
 *    (out) bcm_trunk_member_t *member_array = pointer to member buffer
 *    (out) int *member_count = where to put retrieved member count
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_NOT_FOUND if the module,port's target is not in any aggregate
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of module or port is not permitted.
 *
 *    An aggregate will still be found for a *local* module,port tuple that
 *    points to a target for that aggregate, even if that module,port is not
 *    specifically the tuple used to add that target to the aggregate.  Remote
 *    targets must be specified exactly as they were added to the aggregate.
 *
 *    NULL arguments are allowed for tid and info; if these arguments are NULL,
 *    the appropriate data will not be returned.
 */
extern int
bcm_sirius_trunk_find_and_get(int unit,
                              bcm_module_t modid,
                              bcm_port_t port,
                              bcm_trunk_t *tid,
                              bcm_trunk_info_t *t_data,
                              int member_max,
                              bcm_trunk_member_t *member_array,
                              int *member_count);

/*
 *  Function
 *    bcm_sirius_aggregate_gport_translate
 *  Purpose
 *    Given a module,gport pair, translate it into a target ID if it is local,
 *    or a bogus value (>= _SIRIUS_MC_MAX_TARGETS) otherwise.
 *  Arguments
 *    (in) int unit = the unit on which the operation occurs
 *    (in) uint32 flags = operational flags
 *    (in) bcm_module_t myModId = the intended local module
 *    (in) bcm_module_t module = the module for the port
 *    (in) bcm_port_t port = the gport on the given module
 *    (out) unsigned int *target = where to put the first target
 *    (out) unsigned int *count = where to put the target count
 *    (out) int *isHigig = indicates target is higig MC target (not child)
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Assumes arguments are correct and valid.
 *
 *    Does not validate ports on remote modules (actually, doesn't do anything
 *    with them -- it sets the target to a bogus value to indicate that it is a
 *    remote target if it is not local).
 */
extern int
bcm_sirius_aggregate_gport_translate(const int unit,
                                     const uint32 flags,
                                     const bcm_module_t myModId,
                                     const bcm_module_t module,
                                     bcm_port_t port,
                                     unsigned int *target,
                                     unsigned int *count,
                                     int *isHigig);
/* don't do remove port lookup via modportmap */
#define BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_INHIBIT_MODPORTMAP 1
/* call an internal GPORT (higig or unicast oversub) as higig */
#define BCM_SIRIUS_AGGREGATE_GPORT_TRANSLATE_CALL_INTERNAL_HIGIG 2

/*
 *  Function
 *    bcm_sirius_trunk_port_down
 *  Purpose
 *    Mark all of the targets going to a particular port as down, and remove
 *    them from all active aggregates, recalculating the aggregates so that the
 *    traffic will be put on any remaining subports.  It is possible that an
 *    aggregate will lose all local ports if all of the associated higigs go
 *    down; hopefully the control software can react quickly enough to prevent
 *    multiple-module aggregates from losing all traffic that would have gone
 *    to a particular module in such a case.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_port_t = the higig port number being declared 'down'
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of port is not permitted.  Declaring multiple ports as down
 *    will require multiple calls.
 *
 *    All targets associated with the specified higig will be declared 'down'
 *    by this function.  To get them back up, the caller needs only to update
 *    the aggregates that contain them (yes, the same configuration is
 *    acceptable, but doing so will cause problems such as frames getting stuck
 *    in buffers or queues if the higig link has not been fixed before the call
 *    is made).
 */
int
bcm_sirius_trunk_port_down(int unit, bcm_port_t port);

/*
 *  Function
 *    bcm_sirius_trunk_port_up
 *  Purpose
 *    Mark all of the targets going to a particular port as up, and return them
 *    to active aggregates, recalculating the aggregates so that the traffic
 *    will be put on all available subports.  This has the opposite effect of
 *    the complementary bcm_sirius_trunk_port_down call.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *    (in) bcm_port_t = the higig port number being declared 'down'
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Wildcarding of port is not permitted.  Declaring multiple ports as down
 *    will require multiple calls.
 *
 *    All targets associated with the specified higig will be declared not
 *    'down' by this function.
 */
int
bcm_sirius_trunk_port_up(int unit, bcm_port_t port);

/*
 *  Function
 *    bcm_sirius_trunk_debug
 *  Purpose
 *    Print a diagnostic dump of this module's status.
 *  Arguments
 *    (in) int unit = the unit number on which to create the aggregate
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success (port is in an aggregate)
 *      BCM_E_INIT if support on unit not initialised
 *      BCM_E_* otherwise as appropriate
 */
int
bcm_sirius_trunk_debug(int unit);

extern int
_bcm_sirius_aggregate_data_map_internal_port(const int unit, int port, int si_index);

#ifdef BCM_WARM_BOOT_SUPPORT
extern int
bcm_sbx_wb_trunk_state_sync(int unit, int sync);
#endif /* BCM_WARM_BOOT_SUPPORT */
#ifdef BCM_WARM_BOOT_SUPPORT_SW_DUMP
void 
bcm_sbx_wb_trunk_sw_dump(int unit);
#endif /* BCM_WARM_BOOT_SUPPORT_SW_DUMP */
#endif  /* _BCM_INT_SBX_TRUNK_H_ */
