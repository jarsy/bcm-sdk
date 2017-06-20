/* $Id: jer2_jer_multicast_imp.h,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __JER2_JER_MULTICAST_IMP_INCLUDED__
#define __JER2_JER_MULTICAST_IMP_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */
#include <soc/types.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>


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

/*
 * Get the (pointer to the) next entry from the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 */
uint32 jer2_jer_mcdb_get_next_pointer(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  entry,      /* entry from which to get the next entry pointer */
    DNX_SAND_IN  uint32  entry_type, /* the type of the entry */
    DNX_SAND_OUT uint32  *next_entry /* the output next entry */
);

/*
 * Set the pointer to the next entry in the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 * changes both the mcds and hardware.
 */
uint32 jer2_jer_mcdb_set_next_pointer(
    DNX_SAND_IN  int     unit,
    DNX_SAND_IN  uint32  entry_to_set, /* index of the entry in which to set the pointer */
    DNX_SAND_IN  uint32  entry_type,   /* the type of entry_to_set */
    DNX_SAND_IN  uint32  next_entry    /* the entry that entry_to_set will point to */
);

/*
 * Adds the contents of a mcdb entry of the given type to the mcds buffer.
 * No more than *max_size replications are added, and the max_size value
 * is decreased by the number of added replications.
 * *group_size is increased by the number of found replications.
 * The next entry pointed to by this entry is returned in next_entry.
 */
uint32 jer2_jer_mcds_get_replications_from_entry(
    DNX_SAND_IN    int     unit,
    DNX_SAND_IN    int     core,        /* relevant when hardware has a per core linked list */
    DNX_SAND_IN    uint8   get_bm_reps, /* Should the function return bitmap replications (if non zero) or ignore them */
    DNX_SAND_IN    uint32  entry_index, /* table index of the entry */
    DNX_SAND_IN    uint32  entry_type,  /* the type of the entry */
    DNX_SAND_INOUT uint32  *cud2,       /* the current 2nd CUD of replications */
    DNX_SAND_INOUT uint16  *max_size,   /* the maximum number of replications to return from the group, decreased by the number of returned replications */
    DNX_SAND_INOUT uint16  *group_size, /* incremented by the number of found replications (even if they are not returned) */
    DNX_SAND_OUT   uint32  *next_entry  /* the next entry */
);

/*
 * sets an ingress replication in BCM API encoding from the given CUD and destination in hardware encoding.
 */
uint32 jer2_jer_convert_ingress_replication_hw2api(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint32       cud,            /* CUD to be converted */
    DNX_SAND_IN  uint32       dest,           /* destination to be converted */
    DNX_SAND_OUT soc_gport_t  *port_array,    /* output array to contain ports/destinations */
    DNX_SAND_OUT soc_if_t     *encap_id_array /* output array to contain encapsulations/CUDs/outlifs */
);

/*
 * This function encodes the destination and the CUD as will be written to the hardware fields of IRR_MCDB.
 * The input is the replication data (destination structure and outlif).
 * It is permitted to call the functions to change its input with these
 * arguments: unit, entry, &entry->destination.id, &entry->cud .
 * No locking is needed for this function.
 */
uint32 jer2_jer_mult_ing_encode_entry(
    DNX_SAND_IN    int                    unit,
    DNX_SAND_IN    DNX_TMC_MULT_ING_ENTRY *ing_entry,       /* replication data */
    DNX_SAND_OUT   uint32                 *out_destination, /* the destination field */
    DNX_SAND_OUT   uint32                 *out_cud          /* the CUD/outlif field */
  );


/* } */


#endif /* __JER2_JER_MULTICAST_IMP_INCLUDED__*/
