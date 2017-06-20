/* $Id: multicast.h,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_MULTICAST_H__
#define __DNX_MULTICAST_H__

/*
 * This file contains joint multicast interfaces and mechanisms between mutiple dnx devices
 */

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/types.h>
#include <soc/dnx/legacy/TMC/tmc_api_multicast_ingress.h>
#include <soc/dnx/legacy/TMC/tmc_api_multicast_egress.h>
#include <shared/swstate/sw_state.h>
/*#include <soc/dnx/legacy/drv.h>*/

/*************
 * GLOBALS   *
 *************/


/*************
 *  MACROS   *
 *************/
/* { */

/* macros accessing device specific multicast functions */
#define DNX_MCDS_GET_COMMON_MEMBER(mcds, m) (((dnx_mcds_common_t*)(mcds))->m)

#define DNX_MCDS_INGRESS_LINK_END(mcds) DNX_MCDS_GET_COMMON_MEMBER(mcds, ingress_link_end)
#define DNX_MCDS_GET_MCDB_ENTRY(mcds, index) DNX_MCDS_GET_COMMON_MEMBER((mcds), get_mcdb_entry_from_mcds)((mcds), (index))

#define DNX_MCDS_GET_MCDB_ENTRY_TYPE(mcds, entry) DNX_MCDS_GET_COMMON_MEMBER(mcds, get_mcdb_entry_type)(entry)
#define DNX_MCDS_SET_MCDB_ENTRY_TYPE(mcds, entry, type_value) DNX_MCDS_GET_COMMON_MEMBER(mcds, set_mcdb_entry_type)((entry), (type_value))
#define DNX_MCDS_GET_MCDB_ENTRY_TYPE_FROM_MCDS(mcds, index) DNX_MCDS_GET_MCDB_ENTRY_TYPE(mcds, DNX_MCDS_GET_MCDB_ENTRY((mcds), (index)))
#define DNX_MCDS_SET_MCDB_ENTRY_TYPE_FROM_MCDS(mcds, index, type_value) DNX_MCDS_SET_MCDB_ENTRY_TYPE(mcds, DNX_MCDS_GET_MCDB_ENTRY((mcds), (index)), (type_value))

#define DNX_MCDS_GET_NEXT_POINTER(mcds, unit, entry, entry_type, next_entry) DNX_MCDS_GET_COMMON_MEMBER(mcds, get_next_pointer)((unit), (entry), (entry_type), (next_entry))
#define DNX_MCDS_SET_NEXT_POINTER(mcds, unit, entry, entry_type, next_entry) DNX_MCDS_GET_COMMON_MEMBER(mcds, set_next_pointer)((unit), (entry), (entry_type), (next_entry))


/* get and set the entry type */
#define JER2_JER_SWDB_ENTRY_GET_TYPE(entry) ((entry)->word1 >> JER2_JER_SWDB_MCDB_TYPE_SHIFT) /* assumes the usage of the msb bits */
#define JER2_JER_SWDB_ENTRY_SET_TYPE(entry, type_value) /* assumes the usage of the msb bits */ \
    do {(entry)->word1 = ((entry)->word1 & ~(JER2_JER_SWDB_MCDB_TYPE_MASK << JER2_JER_SWDB_MCDB_TYPE_SHIFT)) | \
      ((type_value) << JER2_JER_SWDB_MCDB_TYPE_SHIFT); } while (0)
#define JER2_JER_SWDB_MCDB_GET_TYPE(mcds, index) JER2_JER_SWDB_ENTRY_GET_TYPE((mcds)->mcdb + (index)) /* assumes the usage of the msb bits */
#define JER2_JER_SWDB_MCDB_SET_TYPE(mcds, index, type_value) JER2_JER_SWDB_ENTRY_SET_TYPE((mcds)->mcdb + (index), (type_value)) /* assumes the usage of the msb bits */
/* } */

/*************
 * TYPE DEFS *
 *************/
/* { */

/* types of replication information, that can be passed between functions inside the mcds and prevent the need for allocation */

/* definition of virtual types, where per device a real type is used instead */
typedef void dnx_mcds_t;       /* Points to the multicast data structure of a device.
                                  This must be a structure whose first member is dnx_mcds_common_t
                                  to implement the polymorphism / joint interfaces. */
typedef void dnx_mcdb_entry_t; /* points to contains one MCDB entry */

typedef uint32 dnx_mc_id_t;
 
/* virtual functions - functions that may be implemented differently (different function pointer in dnx_mcds_common_t) per device */

/* Get a pointer to the mcdb entry with the given index in the mcds */
typedef dnx_mcdb_entry_t* (*dnx_get_mcdb_entry_from_mcds_f)(
    DNX_SAND_IN  dnx_mcds_t* mcds,
    DNX_SAND_IN  uint32 mcdb_index
  );
/* Get the type of a MCDB entry */
typedef uint32 (*dnx_get_mcdb_entry_type_f)(
    DNX_SAND_IN  dnx_mcdb_entry_t* entry
  );
/* set the type of a MCDB entry */
typedef void (*dnx_set_mcdb_entry_type_f)(
    DNX_SAND_INOUT  dnx_mcdb_entry_t* entry,
    DNX_SAND_IN     uint32 type_value
  );


/*
 * Get the (pointer to the) next entry from the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 */
typedef uint32 (*dnx_get_next_pointer_f)(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry,      /* entry from which to get the next entry pointer */
    DNX_SAND_IN  uint32  entry_type, /* the type of the entry */
    DNX_SAND_OUT uint32  *next_entry /* the output next entry */
  );
  
/*
 * Set the pointer to the next entry in the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 * changes both the mcds and hardware.
 */
typedef uint32 (*dnx_set_next_pointer_f)(
    DNX_SAND_IN  int  unit,
    DNX_SAND_IN  uint32  entry_to_set, /* index of the entry in which to set the pointer */
    DNX_SAND_IN  uint32  entry_type,   /* the type of entry_to_set */
    DNX_SAND_IN  uint32  next_entry    /* the entry that entry_to_set will point to */
  );


typedef struct {
    /* functions to handle various multicast functionality */
    dnx_get_mcdb_entry_from_mcds_f get_mcdb_entry_from_mcds;
    dnx_get_mcdb_entry_type_f get_mcdb_entry_type;
    dnx_set_mcdb_entry_type_f set_mcdb_entry_type;
    dnx_get_next_pointer_f get_next_pointer;
    dnx_set_next_pointer_f set_next_pointer;

    uint32 ingress_link_end; /* link pointer marking the end of an ingress linked list */
    uint32 flags;
} dnx_mcds_common_t;

typedef struct
{
  PARSER_HINT_ARR SHR_BITDCL *egress_groups_open_data; /* replicate the information of is each egress group, in warm boot it is stored to the warm boot file */
  PARSER_HINT_ARR SHR_BITDCL *cud2core;           /* For Jericho dual core, store for each CUD to port mapping, what is the core of the port */
} JER2_ARAD_MULTICAST;
/* } */

/* return the mcds of the given unit */
dnx_mcds_t *dnx_get_mcds(
    DNX_SAND_IN  int unit
  );

/* Allocate the multicast data structure of a unit, using a given size */
uint32 dnx_alloc_mcds(
    DNX_SAND_IN  int        unit,
    DNX_SAND_IN  unsigned   size_of_mcds, /* size of mcds to allocate in bytes */
    DNX_SAND_OUT dnx_mcds_t **mcds_out    /* output: allocated mcds */
);

/* De-allocate the multicast data structure of a unit */
uint32 dnx_dealloc_mcds(
    DNX_SAND_IN  int        unit
);


/* Mutlicast asserts mecahnism */

#ifndef _DNX_NO_MC_ASSERTS
#define DNX_MC_ASSERT(cond) do {if (!(cond)) dnx_perform_mc_assert(__FILE__, __LINE__);} while (0)
void dnx_perform_mc_assert(const char *file_name, unsigned line_number);
EXTERN int jer2_arad_mcds_asserts_enabled;
#else
#define DNX_MC_ASSERT(cond)
#endif



/* get the number of mcds asserts that have occurred, return it as the return value */
uint32 dnx_mcds_get_nof_asserts(void);
uint8 dnx_mcds_get_mc_asserts_enabled(void);
void dnx_mcds_set_mc_asserts_enabled(uint8 enabled);
uint8 dnx_mcds_get_mc_asserts_real(void);
void dnx_mcds_set_mc_asserts_real(uint8 real);

#endif /* __DNX_MULTICAST_H__ */

