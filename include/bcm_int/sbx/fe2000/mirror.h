/*
 * $Id: mirror.h,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        mirror.h
 * Purpose:     Mirror internal definitions to the BCM library.
 */

#ifndef _BCM_INT_SBX_FE2000_MIRROR_H_
#define _BCM_INT_SBX_FE2000_MIRROR_H_

#include <bcm/types.h>

/*
 *  Some defines instead of constants...
 *
 *  Even if there are 'no' reserved mirrors on a side, the minimum must be 1,
 *  as entry zero is actually reserved to mean 'no mirroring'.
 */
#define G2P3_INGR_MIRROR_MIN     1 /* lowest valid nonreserved ingr mirror */
#define G2P3_EGR_MIRROR_MIN      4 /* lowest valid nonreserved egr mirror */

#define G2P3_MIRROR_PORT_MAX     2 /* dest port can have 1-ingress + 1-egress mirror */

/*
 *  This type describes a single mirror entity, either ingress or egress.
 *  
 *  Despite this, ingress and egress mirrors are separate resources.
 */
typedef enum _fe2k_mirror_flags_e {
    FE2K_MIRROR_VALID      = 0x0001,                /* entry is in use */
    FE2K_MIRROR_TUNNEL     = 0x0002                 /* entry is tunnel port */
} _fe2k_mirror_flags_t;

typedef struct _fe2k_mirror_s {
    _fe2k_mirror_flags_t flags;                     /* entry flags */
    unsigned int         refCount;                  /* references to entry */
    bcm_gport_t          dest_gport;
} _fe2k_mirror_t;

typedef enum _fe2k_mirror_direction_e { 
    _fe2k_ingress_mirror,
    _fe2k_egress_mirror,
    _fe2k_mirror_direction_max
} _fe2k_mirror_direction_t;

/*
 *  State per direction, ingress or egress
 */
typedef struct _fe2k_mirror_state_s {
    int32            mirror_min;
    int32            mirror_max;
    _fe2k_mirror_t  *mirrors;
} _fe2k_mirror_state_t;

/*  
 *  This contains the global variables for a single unit.
 */
typedef struct _fe2k_mirror_glob_s {
    sal_mutex_t           tableLock;                  /* locks tables */
    _fe2k_mirror_state_t  state[_fe2k_mirror_direction_max];
} _fe2k_mirror_glob_t;

/*  
 *  Global variables for this module
 */
extern _fe2k_mirror_glob_t *_mirror_glob[BCM_MAX_NUM_UNITS];
extern volatile sal_mutex_t _mirror_glob_lock;

/*
 *   Function
 *      _bcm_fe2000_ingr_mirror_alloc
 *   Purpose
 *      Get an ingress mirror that fits the <destModId,destPort> spec.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (out) unsigned int *mirrorId = where to put the mirror number
 *      (in) const bcm_module_t destModId = the destination module ID
 *      (in) const bcm_port_t destPort = the destination port ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      May return the same mirror to multiple callers if they specify the same
 *      <destModId, destPort>.  Will handle this internally.
 *      Will claim & release the unit lock.
 *      Never allocates mirror zero (assumes this means no mirroring).
 *      This is called directly by other modules that need to allocate mirrors.
 */
extern int
_bcm_fe2000_ingr_mirror_alloc(const int unit,
                              unsigned int *mirrorId,
                              const bcm_gport_t destGport);

/*
 *   Function
 *      _bcm_fe2000_ingr_mirror_free
 *   Purpose
 *      Release an ingress mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      This will correctly handle the case where multiple users have a mirror
 *      that is being freed.
 *      Will claim & release the unit lock.
 *      Will refuse to free mirror zero (assumes this means no mirroring).
 *      This is called directly by other modules that need to allocate mirrors.
 */
extern int
_bcm_fe2000_ingr_mirror_free(const int unit,
                             const unsigned int mirrorId);

/*
 *   Function
 *      _bcm_fe2000_ingr_mirror_get
 *   Purpose
 *      Get the data for an ingress mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *      (out) bcm_module_t *destModId = where to put destination module ID
 *      (out) bcm_port_t *destPort = where to put destination port ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Minimal error checking is done here; input parameters must be valid.
 *      This is called directly by other modules that need to allocate mirrors.
 */
extern int
_bcm_fe2000_ingr_mirror_get(const int unit,
                            const unsigned int mirrorId,
                            bcm_gport_t *destGport);

/*
 *   Function
 *      _bcm_fe2000_egr_mirror_alloc
 *   Purpose
 *      Get an egress mirror that fits the <destModId,destPort> spec.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (out) unsigned int *mirrorId = where to put the mirror number
 *      (in) const bcm_module_t destModId = the destination module ID
 *      (in) const bcm_port_t destPort = the destination port ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      May return the same mirror to multiple callers if they specify the same
 *      <destModId,destPort>.  Will handle this internally.
 *      Will claim & release the unit lock.
 *      Never allocates mirror zero (assumes this means no mirroring).
 *      This is called directly by other modules that need to allocate mirrors.
 */
extern int
_bcm_fe2000_egr_mirror_alloc(const int unit,
                             unsigned int *mirrorId,
                             const bcm_gport_t destGport);

/*
 *   Function
 *      _bcm_fe2000_egr_mirror_free
 *   Purpose
 *      Release an egress mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      This will correctly handle the case where multiple users have a mirror
 *      that is being freed.
 *      Will claim & release the unit lock.
 *      Will refuse to free mirror zero (assumes this means no mirroring).
 *      This is called directly by other modules that need to allocate mirrors.
 */
extern int
_bcm_fe2000_egr_mirror_free(const int unit,
                            const unsigned int mirrorId);

/*
 *   Function
 *      _bcm_fe2000_egr_mirror_addref
 *   Purpose
 *      Add a reference to an egress mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Will claim & release the unit lock.
 *      Will not allow mirror zero (assumes this means no mirroring).
 */
extern int
_bcm_fe2000_egr_mirror_addref(const int unit,
                              const unsigned int mirrorId);

/*
 *   Function
 *      _bcm_fe2000_egr_mirror_get
 *   Purpose
 *      Get the data for an egress mirror.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const unsigned int mirrorId = the mirror number
 *      (out) bcm_module_t *destModId = where to put destination module ID
 *      (out) bcm_port_t *destPort = where to put destination port ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Minimal error checking is done here; input parameters must be valid.
 *      This is called directly by other modules that need to allocate mirrors.
 */
extern int
_bcm_fe2000_egr_mirror_get(const int unit,
                           const unsigned int mirrorId,
                           bcm_gport_t *destGport);

/*
 *   Function
 *      _bcm_fe2000_modPort_to_ftEntry
 *   Purpose
 *      Look up the correct FT entry from a destModule,destPort pair.
 *   Parameters
 *      (in) const int unit = the unit number
 *      (in) const bcm_module_t destModId = the destination module ID
 *      (in) const bcm_port_t destPort = the destination port ID
 *      (out) uint32 *destFte = where to put the destination FT entry ID
 *      (out) uint32 *destQueue = where to put the destination queue ID
 *   Returns
 *      BCM_E_NONE if success, else some BCM_E_* as appropriate
 *   Notes
 *      Minimal error checking is done here; input parameters must be valid.
 */
extern int
_bcm_fe2000_modPort_to_ftEntry(const int unit,
                               const bcm_module_t destModId,
                               const bcm_port_t destPort,
                               uint32 *destFte,
                               uint32 *destQueue);

int
_bcm_fe2000_mirror_qid_translate(int unit, uint32 qid,
                                 int *switch_gport);

#endif /* _BCM_INT_SBX_FE2000_MIRROR_H_ */
