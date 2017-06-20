/*
 * $Id: vlan.h,v 1.19 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _BCM_INT_SBX_FE2000_VLAN_H_
#define _BCM_INT_SBX_FE2000_VLAN_H_

#include <bcm/vlan.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbTypesGlue.h>
#ifdef BCM_FE2000_SUPPORT
#ifdef BCM_FE2000_P3_SUPPORT
#include <soc/sbx/g2p3/g2p3.h>
#endif /* def BCM_FE2000_P3_SUPPORT */


#define VLAN_INV_FTE_EXC          0x7F /* invalid FTE exception code */

/*
 *  Private things exported from SBX VLAN code to other SBX modules.
 *
 *  These should never be used except from the specific unit type.
 */

/*
 *  SBX_LAYER_*PV*_TABLES_ARE_PORT_VID must be nonzero if the SBX tables that
 *  are port,vid indexed are indexed by PORT then VID.  If must be zero if the
 *  tables are indexed by VID then port.
 *
 *  The parameter order keeps changing almost every time we get a new microcode
 *  release; this allows us to just flip a switch and adjust all of our calls
 *  to these generated functions.
 */
#define SBX_LAYER_PV2E_TABLES_ARE_PORT_VID          0
#define SBX_LAYER_PV2APPDATA_TABLES_ARE_PORT_VID    0
#define SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID         0

#if SBX_LAYER_PV2E_TABLES_ARE_PORT_VID
#define SOC_SBX_G2P3_PV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2e_get(_unit, _port, _vid, _data)
#define SOC_SBX_G2P3_PV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2e_set(_unit, _port, _vid, _data)
#else /* SBX_LAYER_PV2E_TABLES_ARE_PORT_VID */
#define SOC_SBX_G2P3_PV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2e_get(_unit, _vid, _port, _data)
#define SOC_SBX_G2P3_PV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2e_set(_unit, _vid, _port, _data)
#endif /* SBX_LAYER_PV2E_TABLES_ARE_PORT_VID */

#if SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID
#define SOC_SBX_G2P3_EPV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_epv2e_get(_unit, _port, _vid, _data)
#define SOC_SBX_G2P3_EPV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_epv2e_set(_unit, _port, _vid, _data)
#else /* SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID */
#define SOC_SBX_G2P3_EPV2E_GET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_epv2e_get(_unit, _vid, _port, _data)
#define SOC_SBX_G2P3_EPV2E_SET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_epv2e_set(_unit, _vid, _port, _data)
#endif /* SBX_LAYER_EPV2E_TABLES_ARE_PORT_VID */

#if SBX_LAYER_PV2APPDATA_TABLES_ARE_PORT_VID
#define SOC_SBX_G2P3_PV2APPDATA_GET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2appdata_get(_unit, _port, _vid, _data)
#define SOC_SBX_G2P3_PV2APPDATA_SET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2appdata_set(_unit, _port, _vid, _data)
#else /* SBX_LAYER_PV2APPDATA_TABLES_ARE_PORT_VID */
#define SOC_SBX_G2P3_PV2APPDATA_GET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2appdata_get(_unit, _vid, _port, _data)
#define SOC_SBX_G2P3_PV2APPDATA_SET(_unit, _port, _vid, _data) \
    soc_sbx_g2p3_pv2appdata_set(_unit, _vid, _port, _data)
#endif /* SBX_LAYER_PV2APPDATA_TABLES_ARE_PORT_VID */

/* Convert a vlan gport id to an FT index */
#define VLAN_VGPORT_ID_TO_FT_INDEX(unit__, gid__) \
          ((gid__) + SBX_LOCAL_GPORT_FTE_BASE(unit__))

/* Convert a local gport to a vlan gport id */
#define VLAN_FT_INDEX_TO_VGPORT_ID(unit__, fti__) \
          ((fti__) - SBX_LOCAL_GPORT_FTE_BASE(unit__))

/* Indicates which fields of the lp needs to be updated. */
typedef enum {
    BCM_FE2K_LP_COPY_PID         = 0x0001,
    BCM_FE2K_LP_COPY_QOS         = 0x0002,
    BCM_FE2K_LP_COPY_MIRROR      = 0x0004,
    BCM_FE2K_LP_COPY_POLICER     = 0x0008,
    BCM_FE2K_LP_COPY_COUNTER     = 0x0010
} _bcm_fe2k_lp_copy_flags_t;

/*
 *  This describes what parts of the LP have diverged when applicable.
 */
typedef enum _bcm_fe2k_vlan_divergence_e {
    _FE2K_VLAN_DIVERGE_QOS     = 0x00000001, /* QoS settings have diverged */
    _FE2K_VLAN_DIVERGE_ALL     = 0x0000FFFF  /* everything has diverged */
} _bcm_fe2k_vlan_divergence_t;

/*
 *  This is used when setting native VID to allow other modules to override the
 *  default behaviours.  Normal (tranditional bridging) applications would
 *  leave none of this stuff set.
 *
 *  NVID_OVR_MASK is a mask indicating the bits that are used when replacing or
 *  inserting the tag for native VID.
 *
 *  OVERRIDE_PVV2E is a flag that indicates the caller wants a different PVV2E
 *  for [port,0,0xFFF] than that which would be used in the default rules. If
 *  this bit is clear, nothing else here matters.  All of the other bits are
 *  described based upon this being set (and they have no effect if this is
 *  clear, even if they are set).
 *
 *  USE_PVV2E is a flag that indicates a pvv2e entry is needed.  If clear, the
 *  pvv2e entry will not be created; if set, the pvv2e entry will be created.
 *
 *  SET_REPLACE indicates the value of the pvv2e entry replace flag: set is
 *  TRUE, clear is FALSE.
 *
 *  SET_KEEPORSTRIP indicates the value of the pvv2e entry keeporstrip flag:
 *  set is TRUE; clear is FALSE.
 *
 *  USE_NVID_OVR indicates that the native VID value is to replaced by the
 *  value under NVID_OVR_MASK when set; when clear, the value under
 *  NVID_OVR_MASK is ignored.
 *
 *  BCM_FE2K_NVID_SET_UNTAGGEDSTRIP defines the behavior for untagged pckets
 *  when defstrip=0 (_INNER_VLAN_PRESERVE=1).  If untagged_strip=0, the tag
 *  added by ucode is not stripped, to the user; untagged->native_vid_tagged.
 *  If untagged_strip=1, the tag added by ucode is removed; to the user,
 *  untagged->untagged.
 *
 *  All of these actions only apply to g2p3, as g2p2 manages native VID in a
 *  different manner.
 */
typedef enum _bcm_fe2k_nvid_pvv2e_control_flags_e {
    BCM_FE2K_NVID_NVID_OVR_MASK   = 0x00000FFF,
    BCM_FE2K_NVID_OVERRIDE_PVV2E  = 0x00001000,
    BCM_FE2K_NVID_USE_PVV2E       = 0x00002000,
    BCM_FE2K_NVID_SET_REPLACE     = 0x00004000,
    BCM_FE2K_NVID_SET_KEEPORSTRIP = 0x00008000,
    BCM_FE2K_NVID_USE_NVID_OVR    = 0x00010000,
    BCM_FE2K_NVID_SET_UNTAGGEDSTRIP=0x00020000
} _bcm_fe2k_nvid_pvv2e_control_flags_t;

typedef enum _bcm_fe2k_nvid_action_e {
    BCM_FE2K_NVID_ACTION_SET,
    BCM_FE2K_NVID_ACTION_TOUCH
} _bcm_fe2k_nvid_action_t;

#define BCM_FE2K_NVID_FLAGS_MASK (BCM_FE2K_NVID_NVID_OVR_MASK | \
                                  BCM_FE2K_NVID_OVERRIDE_PVV2E | \
                                  BCM_FE2K_NVID_USE_PVV2E | \
                                  BCM_FE2K_NVID_SET_REPLACE | \
                                  BCM_FE2K_NVID_SET_KEEPORSTRIP | \
                                  BCM_FE2K_NVID_USE_NVID_OVR |\
                                  BCM_FE2K_NVID_SET_UNTAGGEDSTRIP)

#ifdef BCM_FE2000_P3_SUPPORT

/* Use this vid for untagged traffic when using logical bridging model */
#define _BCM_VLAN_G2P3_UNTAGGED_VID 0xfff

#endif /* BCM_FE2000_P3_SUPPORT */

/*
 *  This is the maximum VID that is valid on the platform.  This is not the
 *  'VLAN' (VSI), but the 802.1q tag VID.
 */
extern unsigned int _sbx_vlan_max_vid_value;


#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      _soc_sbx_g2p3_pv2e_set
 *   Purpose
 *      Writes a pv2e entry, mirroring to entry 0 if that port is in drop
 *      tagged mode and the VID is the native VID for that port.
 */

extern int
_soc_sbx_g2p3_pv2e_set(int unit,
                       int iport,
                       int ivid,
                       soc_sbx_g2p3_pv2e_t *e);
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_get_by_pv
 *   Purpose
 *      Gets a logical port, but does so by checking the pv2e.  Will fall back
 *      to the implied logical port for a port if the specified pv2e does not
 *      have an assigned logical port.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_INVALID as the VID.
 */
extern int
bcm_fe2000_lp_get_by_pv(int unit,
                        int iport,
                        int ivid,
                        soc_sbx_g2p3_lp_t *lpData);
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_set_by_pv
 *   Purpose
 *      Sets a logical port, but does so by checking the pv2e.  Will fall back
 *      to the implied logical port for a port if the specified pv2e does not
 *      have an assigned logical port.
 *
 *      Replicates non-diverged LP data from the native VID to the zero VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      native VID.
 *
 *      Replicates non-diverged LP data from the zero VID to the native VID if
 *      the native VID LP and zero VID LP have diverged and the write is to the
 *      zero VID.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_INVALID as the VID.
 */
extern int
bcm_fe2000_lp_set_by_pv(int unit,
                        int iport,
                        int ivid,
                        soc_sbx_g2p3_lp_t *lpData);
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_set_by_pv_and_diverge
 *   Purpose
 *      Adds logical port divergence between the native VID and the zero VID on
 *      a given port, if writing to the zero VID and some divergence is
 *      indicated. This divergence will allocate (if not already diverged) a
 *      new logicalPort ID and assign it to the zero VID, copying the
 *      nondiverged fields from the native VID's logicalPort.
 *
 *      This does not manage any other resources; caller must have already set
 *      up new resources for diverged logical port as appropriate.  All
 *      diverged resources for zero VID are considered to be owned by the zero
 *      VID and will be freed when reconverging.
 *
 *      The provided LP data are those for the zero VID; the native VID.  This
 *      will reflect any nondiverged fields back to the native VID, so the
 *      logical port data must have originally been read from the native VID.
 *
 *      Currently, the only way to reconverge is to set the native VID.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_INVALID as the VID.
 */
extern int
bcm_fe2000_lp_set_by_pv_and_diverge(int unit,
                                    int iport,
                                    int ivid,
                                    _bcm_fe2k_vlan_divergence_t divergence,
                                    soc_sbx_g2p3_lp_t *lpData);
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_lp_check_divergence
 *   Purpose
 *      Checks whether the logical port for the specified port,vid has diverged
 *      on the specified point.  This condition would only be true if the
 *      specified VID is zero and has indeed diverged on the specified point.
 *
 *      The resulting divergence will be the bitwise AND of the actual
 *      divergence and the specified divergence.
 *   Notes
 *      Access the port lp by specifying BCM_VLAN_INVALID as the VID.
 */
extern int
bcm_fe2000_lp_check_diverged(int unit,
                             int iport,
                             int ivid,
                             _bcm_fe2k_vlan_divergence_t *divergence);
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *  Function
 *    bcm_fe2000_g2p3_vlan_port_nativevid_touch
 *  Purpose
 *    Touch a port's nativeVid's pv2e and pvv2e whenever things that should
 *    affect it have changed.
 *  Parameters
 *    (in) int unit = unit number on which to operate
 *    (in) bcm_port_t port = physical port on which to operate
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE for success
 *      BCM_E_* as appropriate otherwise
 *  Notes
 *    Basically just refreshes the pv2e entry using _i_soc_sbx_g2p3_pv2e_set;
 *    that function automatically takes care of the pv2e entry replication and
 *    addition or update or removal of the pvv2e entry.
 */
extern int
bcm_fe2000_g2p3_vlan_port_nativevid_touch(int unit,
                                          bcm_port_t port);
#endif /* def BCM_FE2000_P3_SUPPORT */

#ifdef BCM_FE2000_P3_SUPPORT
/*
 *   Function
 *      bcm_fe2000_vlan_port_lp_replicate
 *   Purpose
 *      Replicate changes from a port's logicalport to the associated pv2es'
 *      logicalports.
 *   Parameters
 *      (in) int unit = unit number on which to operate
 *      (in) _bcm_fe2k_lp_copy_flags flags = what field(s) to update
 *      (in) soc_sbx_g2p3_lp_t *oldValues = ptr to the values before the update
 *      (in) soc_sbx_g2p3_lp_t *newValues = ptr to the values after the update
 *   Returns
 *      bcm_error_t cast as int
 *         BCM_E_NONE for success
 *         BCM_E_* otherwise as appropriate
 *   Notes
 *      Only updates pv2e->lp field(s) as selected in the flags, for pv2e with
 *      lp with the field value(s) equal to the oldValues, setting only these
 *      fields to the newValues.  This way, it does not affect any pv2e lp that
 *      has its own values (so port changes apply except where overridden).
 */
extern int
bcm_fe2000_vlan_port_lp_replicate(const int unit,
                                  const bcm_port_t port,
                                  const _bcm_fe2k_lp_copy_flags_t flags,
                                  const soc_sbx_g2p3_lp_t *oldValues,
                                  const soc_sbx_g2p3_lp_t *newValues);
#endif /* def BCM_FE2000_P3_SUPPORT */

/*
 *   Function
 *      bcm_fe2000_vlan_default_get
 *   Purpose
 *      Get the current default VID
 *   Parameters
 *      (in) int unit = unit number whose default VID is to be retrieved
 *      (out) bcm_vlan_t *vid_ptr = where to put default VID number
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This just reads the local state for the particular unit.  No hardware
 *      work is committed.
 */
extern int
bcm_fe2000_vlan_default_get(int unit,
                            bcm_vlan_t *vid_ptr);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_change
 *   Purpose
 *      Change whether a port is a member or not of a VLAN
 *   Parameters
 *      (in) sbG2Fe_t *pFe = forwarding engine handle
 *      (in) int unit = unit number which is to be affected
 *      (in) bcm_vlan_t vid = vid which is to be affected
 *      (in) bcm_port_t port = port number to add
 *      (in) unsigned int untagged = TRUE if untagged
 *      (in) unsigned int remove = TRUE to remove port from VLAN
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Assumes all parameters are already verified.
 *      Does not update explicit default VID membership bitmap.
 *      Loses translation on port,VID if removing port; otherwise preserves
 *      (but this means that it can't changed tagged state if the port has
 *      egress translation).
 */
extern int
_bcm_fe2000_vlan_port_change(const int unit,
                             const bcm_vlan_t vid,
                             const bcm_port_t port,
                             const unsigned int untagged,
                             const unsigned int remove);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_fetch
 *   Purpose
 *      Get the list of member ports for the specified VID.  The pbmp is the
 *      union of tagged and untagged ports in the VID; the ubmp is the
 *      untagged ports only.
 *   Parameters
 *      (in) sbG2Fe_t *pFe = FE handle
 *      (in) int unit = unit number whose VID is to have the ports list fetched
 *      (in) bcm_vlan_t = VID to which the ports are to ba added
 *      (out) pbmp_t pbmp = ptr to bitmap of ports
 *      (out) pbmp_t *ubmp = ptr to bitmap of untagged ports
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      ubmp will be a subset of pbmp.
 *      ubmp and pbmp are indeterminate on most errors.
 *      Does not check parameter validity
 */
extern int
_bcm_fe2000_vlan_port_fetch(int unit,
                            bcm_vlan_t vid,
                            pbmp_t* pbmp,
                            pbmp_t* ubmp);

/*
 *   Function
 *      _bcm_fe2000_vlan_list_fetch
 *   Purpose
 *      Get a list of the VIDs that exist on this unit, that maybe include any
 *      of the specified ports (or perhaps that merely exist).
 *   Parameters
 *      (in) int unit = unit number whose VID list is to be fetched
 *      (in) pbmp_t pbmp = ports to consider
 *      (out) bcm_vlan_data_t **listp = pointer to variable for address of list
 *      (out) int *countp = pointer to variable for number of list elements
 *      (in) int chkport = nonzero to check ports list; zero to skip ports list
 *   untagged ports Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Only locks the table while processing a VID (allows others access).
 */
extern int
_bcm_fe2000_vlan_list_fetch(int unit,
                            pbmp_t pbmp,
                            bcm_vlan_data_t **listp,
                            int *countp,
                            int chkPort);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_filter_get
 *   Purpose
 *      Get the filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be checked
 *      (out) int *ingress = where to put ingress filtering state (TRUE/FALSE)
 *      (out) int *egress = where to put egress filtering state (TRUE/FALSE)
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      *ingress and *egress will be set TRUE (filtering enabled) or FALSE
 *      (filtering disabled) on success.
 */
extern int
_bcm_fe2000_vlan_port_filter_get(const int unit,
                                 const bcm_port_t port,
                                 int *ingress,
                                 int *egress);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_ingress_filter_set
 *   Purpose
 *      Set the ingress filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be checked
 *      (in) const int filter = new filter state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
extern int
_bcm_fe2000_vlan_port_ingress_filter_set(const int unit,
                                         const bcm_port_t port,
                                         const int filter);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_egress_filter_set
 *   Purpose
 *      Set the egress filter setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const int filter = new filter state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
extern int
_bcm_fe2000_vlan_port_egress_filter_set(const int unit,
                                        const bcm_port_t port,
                                        const int filter);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_native_vlan_set
 *   Purpose
 *      Set the native VID for a port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t port = port whose filter state is to be set
 *      (in) const bcm_vlan_t vlan = new native VID for the port
 *      (in) const _bcm_fe2k_nvid_pvv2e_control_flags_t flags = reflect flags
 *      (in) const _bcm_fe2k_nvid_action_t action = detailed action
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This mirrors the pvid2etc entry for the native VID to the zero VID, and
 *      sets up so that any further writes will also be mirrored.
 *      The reflect flags control pvv2e behaviour when reflecting changes to
 *      the native VID properties (including this call).
 */
extern int
_bcm_fe2000_vlan_port_native_vlan_set(const int unit,
                                      const bcm_port_t port,
                                      const bcm_vlan_t vlan,
                                      const _bcm_fe2k_nvid_pvv2e_control_flags_t flags,
                                      const _bcm_fe2k_nvid_action_t action);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_native_vlan_get
 *   Purpose
 *      Get the native VID for the specified port
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (out) bcm_vlan_t *vlan = where to put native VLAN if dropping tagged
 *      (out) int *drop = where to put the port's drop tagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Only sets vlan if dropping of tagged frames is enabled, else it is left
 *      untouched.
 */
extern int
_bcm_fe2000_vlan_port_native_vlan_get(const int unit,
                                      const bcm_port_t port,
                                      bcm_vlan_t *vlan);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_tagged_ingress_drop_set
 *   Purpose
 *      Set the 'drop tagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) const bcm_vlan_t = port's native VLAN ID
 *      (in) const int drop = new drop tagged state for the port
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      This will destroy ingress translations on the port.
 */
extern int
_bcm_fe2000_vlan_port_tagged_ingress_drop_set(const int unit,
                                              const bcm_port_t port,
                                              const bcm_vlan_t vlan,
                                              const int drop);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_tagged_ingress_drop_get
 *   Purpose
 *      Get the 'drop tagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (out) bcm_vlan_t *vlan = where to put native VLAN if dropping tagged
 *      (out) int *drop = where to put the port's drop tagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Only sets vlan if dropping of tagged frames is enabled, else it is left
 *      untouched.
 */
extern int
_bcm_fe2000_vlan_port_tagged_ingress_drop_get(const int unit,
                                              const bcm_port_t port,
                                              bcm_vlan_t *vlan,
                                              int *drop);

/*
 *   Function
 *      _bcm_fe2000_vlan_port_untagged_ingress_drop_set
 *   Purpose
 *      Set the 'drop untagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (in) int drop = the port's drop untagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 *      Destroys translation that applies to untagged/pritagged frames.
 */
extern int
_bcm_fe2000_vlan_port_untagged_ingress_drop_set(const int unit,
                                                const bcm_port_t port,
                                                int drop);
/*
 *   Function
 *      _bcm_fe2000_vlan_port_untagged_ingress_drop_get
 *   Purpose
 *      Get the 'drop untagged frames' setting for the specified port.
 *   Parameters
 *      (in) const int unit = unit whose port is to be checked
 *      (in) const bcm_port_t = port whose filter state is to be set
 *      (out) int *drop = where to put the port's drop untagged state
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Assumes VLAN:VID mapping is 1:1.
 */
extern int
_bcm_fe2000_vlan_port_untagged_ingress_drop_get(const int unit,
                                                const bcm_port_t port,
                                                int *drop);
/*
 *  Function:
 *    bcm_fe2000_vlan_stat_enable_set
 *  Description:
 *    Enable/Disable statistics on the indicated vlan
 *  Parameters:
 *    in int unit - the unit to manipulate
 *    in bcm_vlan_t vlan - the VLAN whose stats are to be controlled
 *    in int enable - zero to disable stats, nonzero to enable stats
 *  to set Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 */

extern int
_bcm_fe2000_vlan_stat_enable_set(int unit,
                                 bcm_vlan_t vlan,
                                 int enable);

/*
 *  Function:
 *    _bcm_fe2000_vlan_stat_rw
 *  Purpose:
 *    Access statistics for a particular VLAN
 *  Parameters:
 *    in int unit - unit to manipulate
 *    in int clear - zero to read, nonzero to clear
 *    in bcm_vlan_t vlan - VLAN whose stats are to be accessed
 *    in bcm_cos_t cos - COS level for the stats to be accessed
 *    in bcm_vlan_stat_t type - which statistic is to be accesssed
 *    out uint64 *val - where to put the value (if reading)
 *  Returns:
 *    BCM_E_NONE for success
 *    BCM_E_* as appropriate
 *  Notes:
 *    Limited parameter checking is performed here.
 *    There is no write support on SBX; only read and clear.
 *    Some stats are aggregates of other stats.  Clearing such stats will clear
 *    all of the contributors, just as reading them will sum the contributors.
 *    BCM_COS_INVALID as COS level indicates to collect stats for all COS
 *    levels.  In some cases, this implies aggregation, but aggregation of COS
 *    levels can be disabled at compile time; in other cases, the statistic
 *    already has all COS levels aggregated, so BCM_COS_INVALID is the only way
 *    to access the statistic, even when COS aware counters are in use.
 */
extern int
_bcm_fe2000_vlan_stat_rw(int unit,
                         int clear,
                         bcm_vlan_t vlan,
                         bcm_cos_t cos,
                         bcm_vlan_stat_t type,
                         uint64 *val);


extern int
_bcm_fe2000_vlan_vsi_gport_add(int unit, bcm_vlan_t vsi, bcm_gport_t gport);

extern int
_bcm_fe2000_vlan_vsi_gport_delete(int unit, bcm_vlan_t vsi, bcm_gport_t gport);

/*
 *   Function
 *      _bcm_fe2000_vlan_vsi_port_get
 *   Purpose
 *      Get VSI for the given gport
 *   Parameters
 *      (in) unit    - BCM device number
 *      (in) gport   - GPORT to check
 *      (out) vsi    - where to put the VSI to which the GPORT belongs
 *   Returns
 *      BCM_E_NONE if successfull
 *      BCM_E_* on error
 */
extern int
_bcm_fe2000_vlan_vsi_gport_get(int unit,
                               bcm_gport_t gport,
                               bcm_vlan_t *vsi);

/*
 *   Function
 *      _bcm_fe2000_vlan_fte_gport_get
 *   Purpose
 *      Get FT entry index for the given gport
 *   Parameters
 *      (in) unit    - BCM device number
 *      (in) gport   - GPORT to check
 *      (in) ftei    - where to put the VSI to which the GPORT belongs
 *   Returns
 *      BCM_E_NONE if successfull
 *      BCM_E_* on error
 */
extern int
_bcm_fe2000_vlan_fte_gport_get(int unit,
                               bcm_gport_t gport,
                               uint32 *ftei);

#endif /* BCM_FE2000_SUPPORT */

/*
 *  Function
 *    _bcm_fe2000_map_vlan_gport_target
 *  Purpose
 *    Get local module ID and target module ID / port ID from a gport that is
 *    supported by VLAN for inclusion in bcm_vlan_port_t.
 *  Parameters
 *    (in) int unit = unit number
 *    (in) bcm_gport_t gport = target port object
 *    (out) bcm_module_t *locMod = where to put local module ID
 *    (out) bcm_module_t *tgtMod = where to put target module ID
 *    (out) bcm_port_t *tgtPort = where to put target port ID
 *    (out) uint32 *portFte = FT entry index for the port
 *    (out) uint32 *portQueue = where to put the port queue ID
 *  Returns
 *    bcm_error_t cast as int
 *      BCM_E_NONE if successful
 *      BCM_E_* as appropriate if not successful
 *  Notes
 *    No parameter checking
 *    Must call this one from outside VLAN context
 */
extern int
_bcm_fe2000_map_vlan_gport_target(int unit,
                                  bcm_gport_t gport,
                                  bcm_module_t *locMod,
                                  bcm_module_t *tgtMod,
                                  bcm_port_t *tgtPort,
                                  uint32 *portFte,
                                  uint32 *portQueue);

/*
 *  Function
 *    _bcm_fe2000_vlan_check_exists
 *  Purpose
 *    Check whether a VLAN has been configured
 *  Parameters
 *    (in) const int unit = target unit
 *    (in) const bcm_vlan_t vid = target VLAN ID
 *    (out) int *valid = pointer to where to put valid flag
 *  Returns
 *    bcm_error_t = BCM_E_NONE if successful
 *                  BCM_E_* as appropriate otherwise
 *  Notes
 *    Practically no argument checking is performed
 */
extern int
_bcm_fe2000_vlan_check_exists(const int unit,
                              const bcm_vlan_t vid,
                              int *valid);

extern int
_bcm_fe2000_vlan_port_vlan_vector_set(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

extern int
_bcm_fe2000_vlan_port_vlan_vector_get(int unit,
                                      bcm_gport_t gport,
                                      bcm_vlan_vector_t vlan_vec);

/*
 *   Function
 *      bcm_fe2000_vlan_port_qosmap_set
 *   Purpose
 *      Set QoS mapping behaviour on a VLAN GPORT
 *   Parameters
 *      (in) int unit          = BCM device number
 *      (in) bcm_gport_t gport = VLAN GPORT
 *      (in) int ingrMap       = ingress map
 *      (in) int egrMap        = egress map
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
extern int
bcm_fe2000_vlan_port_qosmap_set(int unit,
                                bcm_gport_t gport,
                                int ing_idx,
                                int egr_idx,
                                uint32 ingFlags, 
                                uint32 egrFlags);

/*
 *   Function
 *      bcm_fe2000_vlan_port_qosmap_get
 *   Purpose
 *      Set QoS mapping behaviour on a VLAN GPORT
 *   Parameters
 *      (in)  int unit          = BCM device number
 *      (in)  bcm_gport_t gport = VLAN GPORT
 *      (out) int ing_map       = ingress map
 *      (out) int egr_map       = egress map
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
extern int
bcm_fe2000_vlan_port_qosmap_get(int unit, bcm_gport_t gport,
                                int *ing_idx, int *egr_idx,
                                uint32 *ing_flags, uint32 *egr_flags);

/*
 *   Function
 *      bcm_fe2000_vlan_port_get_lpid
 *   Purpose
 *      Find a logical port based upon the provided VLAN GPORT ID
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = VLAN GPORT to be found
 *      (out) lpid     = LP ID for the GPORT
 *      (out) pport    = physical port for the GPORT
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
extern int
bcm_fe2000_vlan_port_get_lpid(int unit,
                              bcm_gport_t gport,
                              uint32 *lpid,
                              bcm_port_t *pport);

/*
 *   Function
 *      bcm_fe2000_vgp_policer_set
 *   Purpose
 *      set a policer on a vlan gport
 *   Parameters
 *      (in) unit      = BCM device number
 *      (in) gport     = VLAN GPORT to modify
 *      (in) pol_id    = policer id to set
 *   Returns
 *      bcm_error_t = BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
extern int
bcm_fe2000_vgp_policer_set(int unit, bcm_gport_t port, 
                           bcm_policer_t pol_id);


extern int
bcm_fe2k_vlan_vgp_frame_max_access(int unit, bcm_gport_t port, 
                                   int *size, int set);

/*
 *  Function
 *      _bcm_fe2000_g2p3_vlan_nvid_pvv2e_flags_set
 *  Purpose
 *      Sets the vlan nvflags for pvv2e 
 *  Parameters
 *      (in) int unit  - BCM device number
 *      (in) int iport - Port number 
 *      (in) _bcm_fe2k_nvid_pvv2e_control_flags_t flag
 */
extern int
_bcm_fe2000_g2p3_vlan_nvid_pvv2e_flags_set(int unit,
                                   int iport,
                                   _bcm_fe2k_nvid_pvv2e_control_flags_t flags);


/*
 * Function:
 *      _bcm_fe2000_g2p3_vlan_port_diverage(
 * Description:
 *      Diverage the Logical port associated with this port,vlan
 * Parameters:
 *      unit   - device unit number.
 *      vlan   - Vlan ID
 *      port   - port to set policer
 *      diverage - 1 diverage, 0 clear
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_*         - Appropriately
 */
extern int
_bcm_fe2000_g2p3_vlan_port_diverage(int unit, bcm_vlan_t vlan,
                                   bcm_port_t port, int diverage);


#endif  /* _BCM_INT_SBX_FE2000_VLAN_H_ */
