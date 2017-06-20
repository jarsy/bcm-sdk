/* $Id: multicast.h,v 1.8 Broadcom SDK $
 * $Id:
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: multicast header file
 */
#ifndef _BCM_INT_SIRIUS_SBX_MULTICAST_H_
#define _BCM_INT_SIRIUS_SBX_MULTICAST_H_

/*
 *  These constants should be known to some other modules (such as 'trunk'),
 *  but probably not generally known.
 *
 *  _SIRIUS_MC_MAX_TARGETS is the maximum number of multicast targets (it is
 *  also the maximum number of aggregation targets).
 */
#define _SIRIUS_MC_MAX_TARGETS SB_FAB_DEVICE_SIRIUS_MAX_PHYSICAL_PORTS

/*
 *  Function
 *    bcm_sirius_multicast_aggregate_create_id
 *  Purpose
 *    Create a specific aggregate group.  This sets up the multicast support
 *    for an aggregate; the caller is still responsible for the squelch table.
 *    Note that aggregates should be configured with override operations below
 *    and have their OI translation disabled.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) uint32 flags = the flags for this group
 *    (in) bcm_multicast_t group = group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    We allocate a zero-target MVR to store the logical/physical and source
 *    knockout modes.  It'll be replaced later as needed.
 *
 *    This will balk at creating something in the multicast groups space, and
 *    does not itself manage the system port space, in which it expects to
 *    place aggregates.
 */
extern int
bcm_sirius_multicast_aggregate_create_id(int unit,
                                         uint32 flags,
                                         bcm_multicast_t group);

/*
 *  Function
 *    bcm_sirius_multicast_egress_add_override_oi
 *  Purpose
 *    Add an egress element to an existing multicast group, using the specified
 *    OI translation mode instead of the current OI translation mode.
 *  Arguments
 *    (in) _mc_unit_t *unitData = pointer to the unit information
 *    (in) bcm_multicast_t group = group ID
 *    (in) bcm_gport_t port = gport to add
 *    (in) bcm_if_t encap_id = OI for the gport
 *    (in) int use_oi = TRUE if using OI translation, FALSE if not
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    Will not allow multiple references to the same target in physical mode.
 *
 *    Does not care about multiple references to the same target in logical
 *    mode, even if the OIs are identical (allows all).
 *
 *    OI translation in hardware is sticky per target -- if a target already
 *    exists, then its OI translation setting overrides the provided one.  If
 *    not, the provided translation setting is used.
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 */
extern int
bcm_sirius_multicast_egress_add_override_oi(int unit,
                                            bcm_multicast_t group,
                                            bcm_gport_t port,
                                            bcm_if_t encap_id,
                                            int use_oi);
extern int
bcm_sirius_multicast_egress_subscriber_add_override_oi(int unit,
                                                       bcm_multicast_t group,
                                                       bcm_gport_t port,
                                                       bcm_if_t encap_id,
                                                       bcm_gport_t queue_id,
                                                       int use_oi);

/*
 *  Function
 *    bcm_sirius_multicast_egress_set_override_oi
 *  Purpose
 *    Set a group to contain exactly the specified elements.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_multicast_t group = group ID
 *    (in) int port_count = number of ports (why can this be negative?)
 *    (in) bcm_gport_t *port_array = pointer to array of gport IDs
 *    (in) bcm_if_t *encap_id_array = pointer to array of encap_ids
 *    (in) int use_oi = TRUE if using OI translation, FALSE if not
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    port_array and encap_id_array elements are paired 1:1.
 *
 *    Replaces entire existing group contents with new contents, effectively
 *    deleting anything not in the new set.  The API doc doesn't actually *say*
 *    this, but it should.
 *
 *    Somebody should explain why this doesn't take an *unsigned* count.  I
 *    don't see any information about what to do with negative count, so this
 *    implementation will call it BCM_E_PARAM.  Zero count is valid (it just
 *    means there are no replicants in the group).  Yes, you can provide NULL
 *    pointers to the arrays in the case of zero elements.  However, it's
 *    probably more efficient to just call the delete_all function instead.
 *
 *    Wants a BIG pile of stack space (_SHR_PBMP_PORT_MAX * (sizeof(bcm_port_t)
 *    + sizeof(_mc_oitt_elem_internal_t)) so it does not have to do heap
 *    thrashing for resizing data.
 *
 *    Will not allow multiple replicants on same target for physical mode.
 *
 *    The OI translation override can be useful in setting up aggregates in
 *    systems that do use OI translation for multicast but do not need it for
 *    unicast.  OI translation should always be disabled for the entire unit
 *    (improves performance and removes a particular resource limitation) on
 *    systems that do not use OIs for multicast or unicast.  Not sure if
 *    there's any good reason to override in the TRUE direction...
 *
 *    Does not sort the added GPORT,OI tuples in the cache.
 *
 *    Automatically disables OITT for XGS mode higig targets in logical groups;
 *    maintains user statement for OITT mode in all other cases.
 */
extern int
bcm_sirius_multicast_egress_set_override_oi(int unit,
                                            bcm_multicast_t group,
                                            int port_count,
                                            bcm_gport_t *port_array,
                                            bcm_if_t *encap_id_array,
                                            int use_oi);
extern int
bcm_sirius_multicast_egress_subscriber_set_override_oi(int unit,
                                                       bcm_multicast_t group,
                                                       int port_count,
                                                       bcm_gport_t *port_array,
                                                       bcm_if_t *encap_id_array,
                                                       bcm_gport_t *queue_id_array,
                                                       int use_oi);

/*
 *  Function
 *    bcm_sirius_aggregate_to_multicast_id
 *  Purpose
 *    Translate the BCM layer aggregate ('trunk') ID to something used within
 *    multicast to specify the GMT entry that applies to the aggregate.
 *  Arguments
 *    (in) int unit = the unit
 *    (in) bcm_trunk_t aggregate = aggregate ID to use
 *    (out) bcm_multicast_t *group = where to put the aggregate's group ID
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* otherwise as appropriate
 *  Notes
 */
extern int
bcm_sirius_aggregate_to_multicast_id(int unit,
                                     bcm_trunk_t aggregate,
                                     bcm_multicast_t *group);

#endif /* ndef _BCM_INT_SIRIUS_SBX_MULTICAST_H_ */
