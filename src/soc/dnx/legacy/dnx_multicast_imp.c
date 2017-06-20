/* $Id: dnx_multicast_imp.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

#include <soc/dnx/legacy/ARAD/arad_sw_db.h>
#include <soc/dnx/legacy/multicast_imp.h>
#include <soc/dnxc/legacy/error.h>
/* needed to init function pointers in MCDS */
#include <soc/dnx/legacy/ARAD/arad_multicast_imp.h>
#include <soc/dnx/legacy/JER/jer_multicast_imp.h>
#include <soc/dnx/legacy/JER/jer_init.h>

#include <soc/mcm/memregs.h>

#include <shared/bsl.h>


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnxc/legacy/dnxc_mem.h>

#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define JER2_ARAD_MC_INGRESS_LINK_PTR_END 0x1ffff /* marks the end of an ingress linked list, the last entry in the MCDB */
#define JER2_JER_MC_INGRESS_LINK_PTR_END 0x3ffff /* marks the end of an ingress linked list, the last entry in the MCDB */
#define JER2_JER_MCDB_SIZE (JER2_JER_MC_INGRESS_LINK_PTR_END + 1)

/* The value used for unoccupied entries, these entries mean to hardware an empty group if appearing as the first entry of an egress group, except for TDM */
/* The values are also used for empty egress linked lists */
#define JER2_ARAD_MC_UNOCCUPIED_ENTRY_LOW 0x7fffffff
#define JER2_ARAD_MC_UNOCCUPIED_ENTRY_HIGH 0
#define JER2_JER_MC_UNOCCUPIED_ENTRY_LOW 0xffffffff
#define JER2_JER_MC_UNOCCUPIED_ENTRY_HIGH 3

/* The value used for empty ingress MC groups */
#define JER2_ARAD_MC_ING_EMPTY_ENTRY_LOW  0xffffffff
#define JER2_ARAD_MC_ING_EMPTY_ENTRY_HIGH 0x7ffff
#define JER2_JER_MC_ING_EMPTY_ENTRY_LOW  0xffffffff
#define JER2_JER_MC_ING_EMPTY_ENTRY_HIGH 0x7fffff

#define IRDB_TABLE_GROUPS_PER_ENTRY 16
#define IRDB_TABLE_BITS_PER_GROUP 2

#define DNX_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS 2 /* replication slots reserved in buffers for snoop and mirror */

#define JER2_JER_MC_SPECIAL_MODE_ENTRIES_PER_ENTRY 4

uint32 dnx_init_mcds(int unit);
uint32 dnx_deinit_mcds(int unit);

/* } */


/********************************************************************************************
 * Configuration
 * {
 ********************************************************************************************/


/*
 * } Configuration
 */

/*************
 * FUNCTIONS *
 *************/
/* { */



/*
 * free blocks handling
 */


/*
 * Get a pointer to the mcdb entry with the givin index in the mcds
 */
dnx_mcdb_entry_t*
  dnx_mcds_get_mcdb_entry(
    DNX_SAND_IN  int unit,
    DNX_SAND_IN  uint32 mcdb_index
)
{
  return ((dnx_mcds_base_t*)dnx_get_mcds(unit))->mcdb + mcdb_index;
}
/*
 * Get a pointer to the mcdb entry with the given index in the mcds
 */
dnx_mcdb_entry_t* dnx_mcds_get_mcdb_entry_from_mcds(
    DNX_SAND_IN  dnx_mcds_t* mcds,
    DNX_SAND_IN  uint32 mcdb_index
)
{
  return ((dnx_mcds_base_t*)mcds)->mcdb + mcdb_index;
}


/* function to compare replications, for reverse sorting usage */
int dnx_rep_data_t_compare(void *a, void *b)
{ /* assumes sizeof(int) >= sizeof(uint32) which is already assumed in the driver */
  const dnx_rep_data_t *ca = b;
  const dnx_rep_data_t *cb = a;
  int32 res = ca->extra - cb->extra;
  return res ? res : (int32)(ca->base - cb->base);
}

/* functions to add a replication of a specific type to the group stored in the mcds */
void dnx_add_ingress_replication(
  dnx_mcds_base_t *mcds,
  const uint32     cud,
  const uint32     dest
)
{
  dnx_rep_data_t *rep = mcds->reps + mcds->nof_reps[0];
  DNX_MC_ASSERT(mcds->nof_ingr_reps == mcds->nof_reps[0] && mcds->nof_reps[0] < DNX_MULT_MAX_INGRESS_REPLICATIONS &&
    (mcds->nof_egr_port_outlif_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) == 0);
  rep->extra = rep->base = 0;
  DNX_MCDS_REP_DATA_SET_TYPE(rep, DNX_MCDS_REP_TYPE_INGRESS);
  DNX_MCDS_REP_DATA_SET_INGR_CUD(rep, cud);
  DNX_MCDS_REP_DATA_SET_INGR_DEST(rep, dest);
  mcds->nof_ingr_reps = ++mcds->nof_reps[0];
}

/* compare two replications, and return 0 if they are exactly the same, non zero otherwise */
static INLINE uint32 compare_dnx_rep_data_t(const dnx_rep_data_t *rep1, const dnx_rep_data_t *rep2)
{
  return (rep1->base - rep2->base) | (rep1->extra - rep2->extra);
}

void dnx_add_egress_replication_port_cud(
  dnx_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2,
  const uint32     tm_port
)
{
    dnx_rep_data_t *rep = mcds->reps + core * DNX_MULT_MAX_REPLICATIONS + mcds->nof_reps[core];
    DNX_MC_ASSERT(mcds->nof_ingr_reps == 0 && mcds->nof_reps[core] < DNX_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] == mcds->nof_reps[core]);
    rep->extra = rep->base = 0;
    DNX_MCDS_REP_DATA_SET_TYPE(rep, DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
    DNX_MCDS_REP_DATA_SET_EGR_CUD(rep, cud);
    DNX_MCDS_REP_DATA_SET_EXTRA_CUD(rep, cud2);
    DNX_MCDS_REP_DATA_SET_EGR_PORT(rep, tm_port);
    ++mcds->nof_reps[core];
    ++mcds->nof_egr_port_outlif_reps[core];
}

void dnx_add_egress_replication_cud(
  dnx_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2
)
{
    dnx_rep_data_t *rep = mcds->reps + core * DNX_MULT_MAX_REPLICATIONS + mcds->nof_reps[core];
    DNX_MC_ASSERT(mcds->nof_ingr_reps == 0 && mcds->nof_reps[core] < DNX_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] == mcds->nof_reps[core]);
    rep->extra = rep->base = 0;
    DNX_MCDS_REP_DATA_SET_TYPE(rep, DNX_MCDS_REP_TYPE_EGR_CUD);
    DNX_MCDS_REP_DATA_SET_EGR_CUD(rep, cud);
    DNX_MCDS_REP_DATA_SET_EXTRA_CUD(rep, cud2);
    ++mcds->nof_reps[core];
    ++mcds->nof_egr_outlif_reps[core];
}

void dnx_add_egress_replication_bitmap(
  dnx_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2,
  const uint32     bm_id
)
{
    dnx_rep_data_t *rep;
    rep = mcds->reps + core * DNX_MULT_MAX_REPLICATIONS + mcds->nof_reps[core];
    rep->extra = rep->base = 0;
    DNX_MCDS_REP_DATA_SET_TYPE(rep, DNX_MCDS_REP_TYPE_EGR_BM_CUD);
    DNX_MCDS_REP_DATA_SET_EGR_CUD(rep, cud);
    DNX_MCDS_REP_DATA_SET_EXTRA_CUD(rep, cud2);
    DNX_MCDS_REP_DATA_SET_EGR_BM_ID(rep, bm_id);
    ++mcds->nof_reps[core];
    ++mcds->nof_egr_bitmap_reps[core];
    DNX_MC_ASSERT(mcds->nof_ingr_reps == 0 && mcds->nof_reps[core] < DNX_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] == mcds->nof_reps[core]);
}



/*
 * clear the replications data in the mcds
 */
void dnx_mcds_clear_replications(dnx_mcds_base_t *mcds, const uint32 group_type)
{
    int i = 0;
    mcds->nof_ingr_reps = 0;
    mcds->info_2nd_cud = DNX_MC_GROUP_2ND_CUD_NONE;
    for (; i < SOC_DNX_DEFS_MAX(NOF_CORES); ++ i) {
        mcds->nof_egr_bitmap_reps[i] = mcds->nof_egr_outlif_reps[i] = mcds->nof_egr_port_outlif_reps[i] = mcds->nof_reps[i] = 0;
    }
}

/*
 * This functions copies the replication data from the mcds into the given gport and encap_id arrays.
 * It is an error if more entries are to be copied than available in the arrays.
 */
uint32 dnx_mcds_copy_replications_to_arrays(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  uint8        is_egress,           /* are the replications for an egress multicast group (opposed to ingress) */
    DNX_SAND_IN  uint32       arrays_size,         /* size of output arrays */
    DNX_SAND_OUT soc_gport_t  *port_array,         /* output array to contain logical ports/destinations, used if !reps */
    DNX_SAND_OUT soc_if_t     *encap_id_array,     /* output array to contain encapsulations/CUDs/outlifs, used if !reps */
    DNX_SAND_OUT soc_multicast_replication_t *reps /* output replication array (array of size mc_group_size*/
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint16 size;
    dnx_rep_data_t *rep;
    int is_normal_egress = mcds->nof_ingr_reps == 0;
    int swap_cuds = mcds->info_2nd_cud == DNX_MC_GROUP_2ND_CUD_OUTRIF;
    soc_gport_t out_gport;
    soc_if_t out_cud, out_cud2;
    DNXC_INIT_FUNC_DEFS;

    /* The temporary size here must be zero for ingress and for egress TDM.
       For regular egress mcds->nof_ingr_reps must be 0 */
    DNX_MC_ASSERT((is_normal_egress || (mcds->nof_egr_port_outlif_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) == 0) &&
      (SOC_IS_JERICHO(unit) || !swap_cuds));

    /* this code depends on the implementation of the mcds storage, but is faster */
    if (is_egress) {
        int core;
        soc_port_t local_logical_port;
        uint32 total_size = 0;
        for (core = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; core > 0;) {
          --core;
            size = mcds->nof_reps[core];
            total_size += size;
            if (total_size > arrays_size) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many egress replications to return")));
            }

            /* loop over the replications returning them in the appropriate arrays */
            for (rep = mcds->reps + core * DNX_MULT_MAX_REPLICATIONS; size; --size, ++rep) {
                out_cud = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep); /* set CUD/outlif (encapsulation) */
                switch (DNX_MCDS_REP_DATA_GET_TYPE(rep)) {
                  case DNX_MCDS_REP_TYPE_EGR_PORT_CUD:
                    if (SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_EGR_17B_CUDS_127_PORTS_MODE) {
                        _SHR_GPORT_LOCAL_SET(out_gport, DNX_MCDS_REP_DATA_GET_EGR_PORT(rep) & ~(1 << 7)); /* set the local logical port */
                        out_cud |= ((DNX_MCDS_REP_DATA_GET_EGR_PORT(rep) & (soc_if_t)(1 << 7)) << 9); /* set outlif (encapsulation) */
                    } else {
                        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, DNX_MCDS_REP_DATA_GET_EGR_PORT(rep), &local_logical_port));
                        _SHR_GPORT_LOCAL_SET(out_gport, local_logical_port); /* set the local logical port */
                    }
                    break;
                  case DNX_MCDS_REP_TYPE_EGR_CUD:
                    out_gport = _SHR_GPORT_INVALID ; /* set port to invalid to make outlif only */
                    break;
                  case DNX_MCDS_REP_TYPE_EGR_BM_CUD:
                    _SHR_GPORT_MCAST_SET(out_gport, DNX_MCDS_REP_DATA_GET_EGR_BM_ID(rep) & DNX_MC_EGR_MAX_BITMAP_ID); /* set the gport representation of the bitmap id */
                    out_cud |= ((DNX_MCDS_REP_DATA_GET_EGR_BM_ID(rep) & (1 << 14)) << 2); /* handle Arad+ special CUD encoding */
                    break;
                  default:
                    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("unexpected replication type")));
                }
                if (reps) {
                    if (swap_cuds) { /* If a 2nd CUD exists we switch it with the 1st CUD */
                        if ((out_cud2 = DNX_MCDS_REP_DATA_GET_EXTRA_CUD(rep)) == DNX_MC_NO_2ND_CUD) {
                            reps->flags = 0;
                            reps->encap1 = out_cud;
                            reps->encap2 = DNX_MC_NO_2ND_CUD;
                        } else { /* we have a 2nd CUD */
                            reps->flags = SOC_MUTICAST_REPLICATION_ENCAP2_VALID | SOC_MUTICAST_REPLICATION_ENCAP1_L3_INTF;
                            reps->encap1 = out_cud2;
                            reps->encap2 = out_cud;
                        }
                    } else { /* we do not need to switch CUDs */
                        reps->encap2 = DNX_MCDS_REP_DATA_GET_EXTRA_CUD(rep);
                        reps->encap1 = out_cud;
                        reps->flags = (reps->encap2 == DNX_MC_NO_2ND_CUD) ? 0 : SOC_MUTICAST_REPLICATION_ENCAP2_VALID;
                    }
                    reps++->port = out_gport;   /* destination */
                } else {
                    *(port_array++) = out_gport;
                    *(encap_id_array++) = out_cud;
                }
            }
        }

    } else { /* ingress */

        size = mcds->nof_reps[0];
        if (size > arrays_size) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many ingress replications to return")));
        }
        for (rep = mcds->reps; size; --size, ++rep) {
            DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_INGRESS);
            /* convert ingress hardware fields to API representation */
            DNXC_IF_ERR_EXIT(mcds->convert_ingress_replication_hw2api(unit, DNX_MCDS_REP_DATA_GET_INGR_CUD(rep),
              DNX_MCDS_REP_DATA_GET_INGR_DEST(rep), port_array, encap_id_array));
              ++port_array;
            ++encap_id_array;
        }

    } /* end of ingress handling */

exit:
  DNXC_FUNC_RETURN;
}


/* get the tm port and core from the gport  (local port) after it was processed by bcm - not in gport format */
static soc_error_t _get_tm_port_and_core_from_gport(int unit, const uint32 port, uint32 *tm_port, uint32 *port_core)
{
    int core;
    DNXC_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO(unit)) {
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, tm_port, &core));
        if (*tm_port >= DNX_MULT_EGRESS_PORT_INVALID) { /* disabled or invalid port */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid port")));
        }
        DNX_MC_ASSERT(core >= 0 && core < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores);
        *port_core = core;
    } else { /* special treatment is needed due to pre-Jericho (Arad+) special modes of port and CUD encoding to support bigger CUDs */
        *tm_port = port;
        *port_core = 0;
    }
exit:
    DNXC_FUNC_RETURN;
}


/*
 * This functions copies the replication data from the given port and encap_id arrays into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 * The function also converts each logical port to core + TM port in Jericho.
 */
uint32 dnx_mcds_copy_replications_from_arrays(
    DNX_SAND_IN  int       unit,
    DNX_SAND_IN  uint8     is_egress,       /* are the replications for an egress multicast group (opposed to ingress) */
    DNX_SAND_IN  uint8     do_clear,        /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32    arrays_size,     /* size of output arrays */
    DNX_SAND_IN  dnx_mc_replication_t *reps /* input array containing replications using logical ports */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint32 size_left = arrays_size;
    uint32 nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    DNXC_INIT_FUNC_DEFS;

    if (is_egress) {

        uint32 core = 0, port;
        uint32 cud, cud2;
        uint8 cud2_type;
        if (do_clear) {
            dnx_mcds_clear_replications(mcds, DNX_MCDS_TYPE_VALUE_EGRESS);
        }
        for (; size_left; --size_left) {
            port = reps->dest;
            cud = reps->cud;
            cud2 = (reps++)->additional_cud;
            cud2_type = DNX_MC_2ND_CUD_TYPE_REP2MCDS(cud2 & DNX_MC_2ND_CUD_IS_EEI);
            if (cud2 != DNX_MC_NO_2ND_CUD) { /* We have a 2nd CUD */
                if (mcds->info_2nd_cud == DNX_MC_GROUP_2ND_CUD_NONE) { /* This is the first replication with a 2nd CUD */
                    mcds->info_2nd_cud = cud2_type;
                } else if (cud2_type != mcds->info_2nd_cud) {
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Can not mix different types of a 2nd CUD")));
                }
                cud2 &= ~DNX_MC_2ND_CUD_IS_EEI;
            }

            if (port == _SHR_GPORT_INVALID) { /* CUD only replication */
                if (nof_active_cores > 1) { /* add to the appropriate core */
                    DNX_CUD2CORE_GET_CORE(unit, cud, core);
                    if (core == DNX_CUD2CORE_UNDEF_VALUE) {
                        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("CUD 0x%x is not mapped to a port/core"), cud));
                    }
                }
                dnx_add_egress_replication_cud(mcds, core, cud, cud2);
            } else if (port & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* CUD+bitmap replication, added to all cores */
                for (core = SOC_DNX_CONFIG(mcds->unit)->core_mode.nof_active_cores; core > 0;) {
                    --core;
                    dnx_add_egress_replication_bitmap(mcds, core, cud, cud2, port & ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT);
                }
            } else { /* CUD+port replication */
                DNXC_IF_ERR_EXIT(_get_tm_port_and_core_from_gport(unit, port, &port, &core));
                dnx_add_egress_replication_port_cud(mcds, core, cud, cud2, port);
            }
        }
        for (core = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; core > 0;) { /* check for too many replications */
            --core;
            DNX_MC_ASSERT(mcds->nof_reps[core] == (int32)mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] && mcds->nof_ingr_reps == 0);
            if (mcds->nof_reps[core] > DNX_MULT_MAX_EGRESS_REPLICATIONS) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
            }
        }

    } else { /* ingress - not currently in use */

        if (do_clear) {
            dnx_mcds_clear_replications(mcds, DNX_MCDS_TYPE_VALUE_INGRESS);
        }
        DNX_MC_ASSERT(mcds->nof_reps[0] == mcds->nof_ingr_reps && (mcds->nof_egr_port_outlif_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) == 0);
        if (mcds->nof_reps[0] + (int32)arrays_size > DNX_MULT_MAX_INGRESS_REPLICATIONS) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
        }

        for (; size_left; --size_left) {
            uint32 dest = reps->dest;
            soc_if_t cud = (reps++)->cud;
            if (dest >= DNX_MC_ING_DESTINATION_DISABLED) { /* wrong destination - disabled or out of range */
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid destination")));
            }
            dnx_add_ingress_replication(mcds, cud, dest);
        }

    } /* end of ingress handling */

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This functions copies the replication data from a consecutive egress entries block into the mcds (core 0).
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * All the block entries except for the last point implicitly to the next entry using formats 5,7.
 */
uint32
  dnx_mcds_copy_replications_from_egress_block(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  uint8                         do_clear,    /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    DNX_SAND_IN  uint32                        block_start, /* index of the block start */
    DNX_SAND_IN  dnx_free_entries_block_size_t block_size,  /* number of entries in the block */
    DNX_SAND_INOUT uint32                      *cud2,       /* the current 2nd CUD of replications */
    DNX_SAND_OUT uint32                        *next_entry  /* the next entry pointed to by the last block entry */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  uint32 cur_index = block_start;
  dnx_free_entries_block_size_t entries_remaining = block_size;
  uint16 nof_reps = 0, reps_left;
  DNXC_INIT_FUNC_DEFS;

  DNX_MC_ASSERT(block_start + block_size < DNX_MCDS_INGRESS_LINK_END(mcds));
  if (do_clear) {
    dnx_mcds_clear_replications(mcds, DNX_MCDS_TYPE_VALUE_EGRESS);
  } else {
    nof_reps = mcds->nof_reps[0];
  }
  DNX_MC_ASSERT(nof_reps == mcds->nof_egr_port_outlif_reps[0] + mcds->nof_egr_outlif_reps[0] +
    mcds->nof_egr_bitmap_reps[0] && mcds->nof_ingr_reps == 0 &&
    nof_reps < DNX_MULT_MAX_EGRESS_REPLICATIONS && mcds->nof_reps[0] == nof_reps);
  reps_left = DNX_MULT_MAX_EGRESS_REPLICATIONS - nof_reps;

  /* get replications from the rest of the entries */
  while (entries_remaining) {
    DNXC_IF_ERR_EXIT(mcds->get_replications_from_entry( /* add replications to mcds from cur_index entry */
      unit, 0, TRUE, cur_index, DNX_MCDS_TYPE_VALUE_EGRESS, cud2, &reps_left, &nof_reps, next_entry));
    ++cur_index;
    --entries_remaining;
    if (nof_reps > DNX_MULT_MAX_EGRESS_REPLICATIONS) {
      DNX_MC_ASSERT(0); /* group is somehow bigger than allowed - internal error */
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    } else if (entries_remaining && *next_entry == DNX_MC_EGRESS_LINK_PTR_END) {
      DNX_MC_ASSERT(0); /* entry does not point to next entry in the block - internal error */
      DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("block too small")));
    }
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * This functions removes the given (single) replication (dest and cud) from the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 */
uint32 dnx_mult_remove_replication(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  uint32               group_type, /* to what type of group does the replication belong */
    DNX_SAND_IN  uint32               dest,       /* local port/destination of the replication to remove */
    DNX_SAND_IN  soc_if_t             cud,        /* encapsulations/CUD/outlif of the replication to remove */
    DNX_SAND_IN  soc_if_t             cud2,       /* encapsulations/CUD/outlif of the second replication to remove */
    DNX_SAND_OUT DNX_TMC_ERROR        *out_err,   /* return possible errors that the caller may want to ignore: replication does not exist */
    DNX_SAND_OUT dnx_mc_core_bitmap_t *cores      /* return the linked list cores of the removed replication */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  dnx_rep_data_t *rep = mcds->reps, rep_to_find = {0};
  uint16 size_left = 0, *specific_nof_reps;
  uint32 core = 0; /* number of core to work on, which may be >0 when multiple linked lists exist for the group */
  DNXC_INIT_FUNC_DEFS;
  *out_err = _SHR_E_NONE;

  /* this code depends on the implementation of the mcds storage, but is faster */
  if (DNX_MCDS_TYPE_IS_EGRESS(group_type)) { /* egress */

    if (dest == _SHR_GPORT_INVALID) { /*outlif only replication */
      if(SOC_IS_JERICHO(unit)) {
          DNX_CUD2CORE_GET_CORE(unit, cud, core);
          if (core >= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores) {
              DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("CUD 0x%x to be removed is not mapped to a port/core"), cud));
          }
      }
      DNX_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DNX_MCDS_REP_TYPE_EGR_CUD);
      DNX_MCDS_REP_DATA_SET_EGR_CUD(&rep_to_find, cud);
      DNX_MCDS_REP_DATA_SET_EXTRA_CUD(&rep_to_find, cud2);
      specific_nof_reps = &mcds->nof_egr_outlif_reps[core];
    } else if (dest & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* bitmap + outlif replication */
      const uint32 bitmap = dest & ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT;
      dnx_rep_data_t *found_reps_per_core[SOC_DNX_DEFS_MAX(NOF_CORES)] = {0};
      DNX_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DNX_MCDS_REP_TYPE_EGR_BM_CUD);
      DNX_MCDS_REP_DATA_SET_EGR_CUD(&rep_to_find, cud);
      DNX_MCDS_REP_DATA_SET_EXTRA_CUD(&rep_to_find, cud2);
      DNX_MCDS_REP_DATA_SET_EGR_BM_ID(&rep_to_find, bitmap);
      for (core = 0; core < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; ++core) {
        /* search for an entry in each core */
        rep = mcds->reps + DNX_MCDS_GET_REP_INDEX(core, 0);
        for (size_left = mcds->nof_reps[core]; size_left; --size_left, ++rep) {
          DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep) != DNX_MCDS_REP_TYPE_INGRESS);
          if (!compare_dnx_rep_data_t(rep, &rep_to_find)) {
            found_reps_per_core[core] = rep;
            break;
          }
        }
        if (!size_left) {
          *out_err = _SHR_E_NOT_FOUND;
          break;
        }
      }
      if (*out_err != _SHR_E_NOT_FOUND) { /* remove the found entries */
        for (core = 0; core < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; ++core) {
          --mcds->nof_reps[core];
          --mcds->nof_egr_bitmap_reps[core];
          *found_reps_per_core[core] = mcds->reps[DNX_MCDS_GET_REP_INDEX(core, mcds->nof_reps[core])];
        }
      }
      if (cores != NULL) {
        *cores = DNX_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit);
      }
      SOC_EXIT;
    } else { /* port + outlif replication */
      uint32 tm_port;
      DNXC_IF_ERR_EXIT(_get_tm_port_and_core_from_gport(unit, dest, &tm_port, &core));
      DNX_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      DNX_MCDS_REP_DATA_SET_EGR_CUD(&rep_to_find, cud);
      DNX_MCDS_REP_DATA_SET_EXTRA_CUD(&rep_to_find, cud2);
      DNX_MCDS_REP_DATA_SET_EGR_PORT(&rep_to_find, tm_port);
      specific_nof_reps = &mcds->nof_egr_port_outlif_reps[core];
    }

    rep = mcds->reps + DNX_MCDS_GET_REP_INDEX(core, 0);
    for (size_left = mcds->nof_reps[core]; size_left; --size_left, ++rep) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep) != DNX_MCDS_REP_TYPE_INGRESS);
      if (!compare_dnx_rep_data_t(rep, &rep_to_find)) {
        break;
      }
    }

  } else if (DNX_MCDS_TYPE_IS_INGRESS(group_type)) { /* ingress */

    DNX_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DNX_MCDS_REP_TYPE_INGRESS);
    DNX_MCDS_REP_DATA_SET_INGR_CUD(&rep_to_find, cud);
    DNX_MCDS_REP_DATA_SET_INGR_DEST(&rep_to_find, dest);
    specific_nof_reps = &mcds->nof_ingr_reps;
    for (size_left = mcds->nof_reps[0]; size_left; --size_left, ++rep) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_INGRESS);
      if (!compare_dnx_rep_data_t(rep, &rep_to_find)) {
        break;
      }
    }

  } else { /* type is: not used */
    DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("unexpected entry type")));
  }

  if (!size_left) {
    *out_err = _SHR_E_NOT_FOUND;
  } else { /* remove the found entry */
    --mcds->nof_reps[core];
    --*specific_nof_reps;
    *rep = mcds->reps[DNX_MCDS_GET_REP_INDEX(core, mcds->nof_reps[core])];
  }
  if (cores != NULL) {
    *cores = DNX_MC_CORE_BITAMAP_CORE_0 << core;
  }
exit:
  DNXC_FUNC_RETURN;
}

/*
 * Get, set, increase and decrease the stored number of free entries
 */
uint32 dnx_mcds_unoccupied_get(
    DNX_SAND_IN dnx_mcds_base_t *mcds
)
{
  return mcds->nof_unoccupied;
}

void
  dnx_mcds_unoccupied_increase(
    DNX_SAND_INOUT dnx_mcds_base_t *mcds,
    DNX_SAND_IN    uint32               delta
)
{
  mcds->nof_unoccupied += delta;
  DNX_MC_ASSERT(mcds->nof_unoccupied < DNX_LAST_MCDB_ENTRY(mcds));
}

void
  dnx_mcds_unoccupied_decrease(
    DNX_SAND_INOUT dnx_mcds_base_t *mcds,
    DNX_SAND_IN    uint32               delta
)
{
  DNX_MC_ASSERT(mcds->nof_unoccupied >= delta);
  mcds->nof_unoccupied -= delta;
}

/*
 * Get the region corresponding to the given mcdb index
 */
dnx_free_entries_blocks_region_t* dnx_mcds_get_region(dnx_mcds_base_t *mcds, uint32 mcdb_index)
{
  DNX_MC_ASSERT(mcds && mcdb_index > 0 && mcdb_index < DNX_LAST_MCDB_ENTRY(mcds));

  if (mcdb_index >= mcds->ingress_alloc_free.range_start && mcdb_index <= mcds->ingress_alloc_free.range_end) {
    return &mcds->ingress_alloc_free;
  }
  if (mcdb_index >= mcds->egress_alloc_free.range_start && mcdb_index <= mcds->egress_alloc_free.range_end) {
    return &mcds->egress_alloc_free;
  }
  return &mcds->free_general;
}


/*
 * Return the region corresponding to the given mcdb index,
 * and get the max consecutive entries sub range from the range that includes mcdb_index.
 */
dnx_free_entries_blocks_region_t*
  dnx_mcds_get_region_and_consec_range(dnx_mcds_base_t *mcds, uint32 mcdb_index, uint32 *range_start, uint32 *range_end)
{
  dnx_free_entries_blocks_region_t *range = dnx_mcds_get_region(mcds, mcdb_index);
  *range_start = range->range_start;
  *range_end = range->range_end;
  /* This code depends on only the general range containing other ranges, so other ranges are consecutive */
  /* It also depends on the egress range being after the ingress range */
  if (range == &mcds->free_general) { 
    if (mcdb_index < mcds->ingress_alloc_free.range_start) {
      if (*range_end >= mcds->ingress_alloc_free.range_start) {
        *range_end = mcds->ingress_alloc_free.range_start - 1;
      }
    } else if (mcdb_index > mcds->egress_alloc_free.range_end) {
      if (*range_start <= mcds->egress_alloc_free.range_end) {
        *range_start = mcds->egress_alloc_free.range_end + 1;
      }
    } else if (mcdb_index > mcds->ingress_alloc_free.range_end && mcdb_index < mcds->egress_alloc_free.range_start) {
      if (*range_end >= mcds->egress_alloc_free.range_start) {
        *range_end = mcds->egress_alloc_free.range_start - 1;
      }
      if (*range_start <= mcds->ingress_alloc_free.range_end) {
        *range_start = mcds->ingress_alloc_free.range_end + 1;
      }
    }
  }
  DNX_MC_ASSERT(*range_start <= *range_end && mcdb_index >= *range_start && mcdb_index <= *range_end);
  return range;
}

/*
 * Copy the src_index entry to the dst_index entry, and write the dst_index entry to hardware.
 * Both the hardware and mcds is copied. So be bery careful if using this to copy a free entry.
 */
uint32 dnx_mcdb_copy_write(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 src_index, /* index of entry to be copied */
    DNX_SAND_IN uint32 dst_index  /* index of entry to be copied to and written to disk */
)
{
  uint32 data[DNX_MC_ENTRY_SIZE];
  dnx_mcds_base_t* mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *src_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, src_index);
  jer2_arad_mcdb_entry_t *dst_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, dst_index);
  DNXC_INIT_FUNC_DEFS;

  data[0] = dst_entry->word0 = src_entry->word0;
  dst_entry->word1 &= DNX_MCDS_WORD1_KEEP_BITS_MASK;
  dst_entry->word1 |= src_entry->word1 & ~DNX_MCDS_WORD1_KEEP_BITS_MASK;
  data[1] = src_entry->word1 & mcds->msb_word_mask;
  DNXC_IF_ERR_EXIT(WRITE_IRR_MCDBm(unit, MEM_BLOCK_ANY, dst_index, data));
  mcds->prev_entries[dst_index] = mcds->prev_entries[src_index];

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Init a free entries block list to be empty.
 * This does not look at its existing contents as it is assumed to be uninitialized.
 */
static INLINE void
  dnx_mcds_init_free_entries_block_list(dnx_free_entries_block_list_t *list)
{
  list->first = DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY;
}
/*
 * Remove the given free entries block from the given block list.
 * Does not modify the block itself.
 */
void _dnx_mcds_remove_free_entries_block_from_list(dnx_mcds_base_t *mcds, dnx_free_entries_block_list_t *list, uint32 block, const dnx_free_entries_block_size_t block_size)
{
  uint32 next, prev;

  DNX_MC_ASSERT(block < DNX_LAST_MCDB_ENTRY(mcds) && block > 0);
  DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
  DNX_MC_ASSERT(block_size > 0 && block_size == DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block));
  DNX_MC_ASSERT(list == dnx_mcds_get_region(mcds, block)->lists + (block_size-1));
  next = DNX_MCDS_GET_FREE_NEXT_ENTRY(mcds, block);
  prev = DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, block);
  if (next == block) { /* this was the only entry in the list */
    DNX_MC_ASSERT(prev == block && list->first == block);
    list->first = DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY; /* make list as empty */
  } else { /* the list has more entries */
    DNX_MC_ASSERT(prev != block && 
      DNX_MCDS_GET_TYPE(mcds, prev) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
      DNX_MCDS_GET_TYPE(mcds, next) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    DNX_MC_ASSERT(DNX_MCDS_GET_FREE_NEXT_ENTRY(mcds, prev) == block &&
                DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, next) == block);
    DNX_MCDS_SET_FREE_NEXT_ENTRY(mcds, prev, next);
    DNX_MCDS_SET_FREE_PREV_ENTRY(mcds, next, prev);
    if (list->first == block) {
      list->first = next; /* If this was the list start, advance list start to the next block */
    }
  }
  LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
              (BSL_META("removed(%u,%u) "), block, block_size));
  dnx_mcds_unoccupied_decrease(mcds, block_size); /* subtract the block size from the number of free entries */
}

/*
 * Remove the given free entries block from the given block list.
 * Does not modify the block itself.
 */
static INLINE void
  dnx_mcds_remove_free_entries_block_from_list(dnx_mcds_base_t *mcds, dnx_free_entries_block_list_t *list, uint32 block)
{
  _dnx_mcds_remove_free_entries_block_from_list(mcds, list, block, DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block));
}

/*
 * Remove the given free entries block from the given region's block list.
 * Does not modify the block itself.
 */
static INLINE void
  dnx_mcds_remove_free_entries_block_from_region(dnx_mcds_base_t *mcds, dnx_free_entries_blocks_region_t *region, uint32 block, dnx_free_entries_block_size_t block_size)
{
  dnx_free_entries_block_list_t *list = region->lists + (block_size-1);

  DNX_MC_ASSERT(block_size <= region->max_size);
  _dnx_mcds_remove_free_entries_block_from_list(mcds, list, block, block_size);
}

/*
 * Check if the given free entries block list is empty.
 * return non zero if empty.
 */
static INLINE int
  dnx_mcds_is_empty_free_entries_block_list(const dnx_mcds_base_t *mcds, const dnx_free_entries_block_list_t *list)
{
  if (list->first == DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY)
    return 1;
  DNX_MC_ASSERT(list->first < DNX_LAST_MCDB_ENTRY(mcds));
  return 0;
}

/*
 * Add the given free entries block to the given block list.
 * Does not modify the block itself.
 */
void
  dnx_mcds_add_free_entries_block_to_list(dnx_mcds_base_t *mcds, dnx_free_entries_block_list_t *list, uint32 block)
{
  uint32 next, prev;
  dnx_free_entries_block_size_t block_size = DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block);

  if (dnx_mcds_is_empty_free_entries_block_list(mcds, list)) {
    list->first = prev = next = block;
  } else {
    next = list->first;
    prev = DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, next);
    DNX_MC_ASSERT(DNX_MCDS_GET_FREE_NEXT_ENTRY(mcds, prev) == next &&
      DNX_MCDS_GET_TYPE(mcds, next) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    DNX_MCDS_SET_FREE_PREV_ENTRY(mcds, next, block);
    DNX_MCDS_SET_FREE_NEXT_ENTRY(mcds, prev, block);
  }
  DNX_MCDS_SET_FREE_PREV_ENTRY(mcds, block, prev);
  DNX_MCDS_SET_FREE_NEXT_ENTRY(mcds, block, next);
  dnx_mcds_unoccupied_increase(mcds, block_size);
} 

/*
 * Return the first entry of the given free entries block list, or 0 if it is empty.
 * If to_remove is non zero, the found block will be removed from the list (and not otherwise changed).
 */
uint32 dnx_mcds_get_free_entries_block_from_list(dnx_mcds_base_t *mcds, dnx_free_entries_block_list_t *list, int to_remove)
{
  uint32 block = list->first;
  if (block == DNX_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY)
    return 0;
  DNX_MC_ASSERT(block < DNX_LAST_MCDB_ENTRY(mcds));
  DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);

  if (to_remove) {
    dnx_mcds_remove_free_entries_block_from_list(mcds, list, block);
  }
  return block;
}

/*
 * Init a region with the given maximum block size. All lists will be marked as empty.
 */
void dnx_mcds_init_region(dnx_free_entries_blocks_region_t *region, dnx_free_entries_block_size_t max_size, uint32 range_start, uint32 range_end)
{
  unsigned i;
  dnx_free_entries_block_list_t *list_p = region->lists;
  region->max_size = max_size;
  region->range_start = range_start;
  region->range_end = range_end;
  for (i = 0; i < max_size; ++i) {
    dnx_mcds_init_free_entries_block_list(list_p);
    ++list_p;
  }
}


/*
 * Create a free block of a given size at the given entry.
 * add it to a free entries block list at the given region.
 * The block entries are assumed to be (marked as) free.
 * The created block or part of it may be merge with adjacent free blocks based on flags.
 */
#define DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV  1 /* Will not merge with the previous consecutive free block */
#define DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT  2 /* Will not merge with the next consecutive free block */
#define DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE          3 /* Will not merge with consecutive free blocks */
static uint32 dnx_mcds_create_free_entries_block(
    DNX_SAND_INOUT dnx_mcds_base_t                 *mcds,
    DNX_SAND_IN    uint32                           flags,             /* DNX_MCDS_GET_FREE_BLOCKS_* flags that affect merging with adjacent free blocks */
    DNX_SAND_IN    uint32                           block_start_index, /* start index of the free block */
    DNX_SAND_IN    dnx_free_entries_block_size_t    block_size,        /* number of entries in the block */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region            /* region to contain the block in its lists */
)
{
  int unit = mcds->unit;
  uint32 i, current_block_start_index = block_start_index;
  uint32 block_end = block_start_index + block_size; /* the index of the entry immediately after the block */
  dnx_free_entries_block_size_t current_block_size = block_size, joint_block_size;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(region);
  if (block_start_index + block_size > DNX_MCDS_INGRESS_LINK_END(mcds) || !block_start_index) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("block out of range")));
  }
  if (block_size > region->max_size || block_size < 1) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid block size")));
  }

  /* check the block's entries */
  for (i = block_start_index; i < block_end; ++i) {
    if (DNX_MCDS_TYPE_IS_USED(DNX_MCDS_GET_TYPE(mcds, i))) {
      DNX_MC_ASSERT(0); /* the entries of the block must be free */
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("attempted to add a used entry number %u to a free block"), i));
    }
  }

  /* if block is not of max size, attempt to merge with adjacent blocks */
  if (block_size < region->max_size) {
    const uint32 next_block = block_start_index + block_size;
    uint32 prev_block = block_start_index - 1;
    dnx_free_entries_block_size_t prev_block_size = 0, next_block_size = 0;
    
    if (!(flags & DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV) && /* get information on the previous adjacent block */
        prev_block >= region->range_start && prev_block <= region->range_end &&
        region == dnx_mcds_get_region(mcds, prev_block) &&
        DNX_MCDS_TYPE_IS_FREE(i = DNX_MCDS_GET_TYPE(mcds, prev_block))) {
      prev_block_size = 1;
      if (i != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) { /* block size > 1 */
        prev_block = DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, prev_block);
        prev_block_size = block_start_index - prev_block;
        DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, prev_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
          prev_block < block_start_index - 1 && prev_block_size <= region->max_size);
      }
      DNX_MC_ASSERT(prev_block_size == DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, prev_block));
      if (prev_block_size == region->max_size) {
        prev_block_size = 0; /* do not merge with max size blocks */
      }
    }
 
    if (!(flags & DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT) && /* get information on the next adjacent block */
        next_block >= region->range_start && next_block <= region->range_end &&
        region == dnx_mcds_get_region(mcds, next_block) &&
        DNX_MCDS_GET_TYPE(mcds, next_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
      next_block_size = DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, next_block);
      if (next_block_size == region->max_size) {
        next_block_size = 0; /* do not merge with max size blocks */

      } else if (prev_block_size) { /* if we can merge in both directions, choose one direction */
        if (block_size + prev_block_size <= region->max_size) { /* If we can merge with the whole previous block, select it */
        } else if (block_size + next_block_size <= region->max_size) { /* If we can merge with the whole next block, select it */
          prev_block_size = 0;
        } else if (prev_block_size > next_block_size) { /* else merge with the smaller block ,or with the previous if they are of the same size */
          prev_block_size = 0;
        }
      }
    }

    if (prev_block_size) { /* merge with the previous block */

      joint_block_size = prev_block_size + block_size;
      if (joint_block_size > region->max_size) {
        current_block_size = joint_block_size - region->max_size;
        joint_block_size = region->max_size;
      } else {
        current_block_size = 0;
      }
      current_block_start_index = prev_block + joint_block_size;
      DNX_MC_ASSERT(joint_block_size + current_block_size == prev_block_size + block_size &&
        prev_block + joint_block_size == block_start_index + block_size - current_block_size);
      dnx_mcds_remove_free_entries_block_from_region(mcds, region, prev_block, prev_block_size); /* remove the previous block from free blocks list */

      LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                  (BSL_META_U(unit,
                              "merge with prev free block: prev:%u,%u  freed:%u,%u\n"), prev_block, prev_block_size, block_start_index, block_size));
      DNX_MCDS_SET_FREE_BLOCK_SIZE(mcds, prev_block, joint_block_size); /* mark the new previous block size */
      /* mark the type of the block entries added to the previous block */
      for (i = block_start_index; i < current_block_start_index; ++i) {
        DNX_MCDS_SET_TYPE(mcds, i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
        DNX_MCDS_SET_FREE_PREV_ENTRY(mcds, i, prev_block);
      }
      dnx_mcds_add_free_entries_block_to_list(mcds, region->lists + (joint_block_size-1), prev_block); /* add the previous block to different free blocks list */

    } else if (next_block_size) { /* merge with the next block */

      joint_block_size = next_block_size + block_size;
      if (joint_block_size > region->max_size) {
        current_block_size = joint_block_size - region->max_size;
        joint_block_size = region->max_size;
      } else {
        current_block_size = 0;
      }
      current_block_start_index += joint_block_size;
      DNX_MC_ASSERT(joint_block_size + current_block_size == next_block_size + block_size &&
        block_start_index + joint_block_size == next_block + next_block_size - current_block_size);

      LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                  (BSL_META_U(unit,
                              "merge with next free block: next:%u,%u  freed:%u,%u\n"), next_block, next_block_size, block_start_index, block_size));
      dnx_mcds_remove_free_entries_block_from_region(mcds, region, next_block, next_block_size); /* remove the next block from free blocks list */
      /* mark the type of the block entries */
      DNX_MCDS_SET_FREE_BLOCK_SIZE(mcds, block_start_index, joint_block_size); /* set the block size, to be called for the first block entry */ \
      DNX_MCDS_SET_TYPE(mcds, block_start_index, DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
      for (i = block_start_index + 1; i < current_block_start_index; ++i) {
        DNX_MCDS_SET_TYPE(mcds, i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
        DNX_MCDS_SET_FREE_PREV_ENTRY(mcds, i, block_start_index);
      }
      dnx_mcds_add_free_entries_block_to_list(mcds, region->lists + (joint_block_size-1), block_start_index); /* add the previous block to different free blocks list */
 
    }

  }

  if (current_block_size) {
    /* mark the block's size */
    DNX_MCDS_SET_FREE_BLOCK_SIZE(mcds, current_block_start_index, current_block_size); /* set the block size, to be called for the first block entry */ \
    LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                (BSL_META_U(unit,
                            "added free block: %u,%u\n"), current_block_start_index, current_block_size));
    /* mark the type of the block entries */
    DNX_MCDS_SET_TYPE(mcds, current_block_start_index, DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    block_end = current_block_start_index + current_block_size;
    for (i = current_block_start_index + 1; i < block_end; ++i) {
      DNX_MCDS_SET_TYPE(mcds, i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
      DNX_MCDS_SET_FREE_PREV_ENTRY(mcds, i, current_block_start_index);
    }

    /* add the block to the appropriate list of free blocks */
    dnx_mcds_add_free_entries_block_to_list(mcds, region->lists + (current_block_size-1), current_block_start_index);
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Add free entries in the given range as blocks to the lists of the given region.
 * If (check_free) add only blocks marked as free.
 * Otherwise add all entries in the range are expected to be marked used and they will be marked as free.
 */
uint32 dnx_mcds_build_free_blocks(
    DNX_SAND_IN    int                              unit,   /* used only if check_free is zero */
    DNX_SAND_INOUT dnx_mcds_base_t                  *mcds,
    DNX_SAND_IN    uint32                           start_index, /* start index of the range to work on */
    DNX_SAND_IN    uint32                           end_index,   /* end index of the range to work on, if smaller than start_index then do nothing */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region,     /* region to contain the block in its lists */
    DNX_SAND_IN    dnx_mcds_free_build_option_t         entry_option /* which option to use in selecting entries to add and verifying them */
)
{
  dnx_free_entries_block_size_t max_size, block_size;
  uint32 block_start, cur_entry;
  int check_free = entry_option == dnxMcdsFreeBuildBlocksAddOnlyFree;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(region);
  DNX_MC_ASSERT(check_free || mcds == dnx_get_mcds(unit));
  if (start_index > end_index) {
    SOC_EXIT;
  }
  if (end_index >= DNX_MCDS_INGRESS_LINK_END(mcds) || !start_index) { /* index out of allowed range */
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("block out of range")));
  }

  max_size = region->max_size;

  for (block_start = start_index; block_start <= end_index; block_start += block_size) { /* loop over the index range */
    if (check_free) {
      block_size = 0;
      for (; block_start <= end_index && DNX_MCDS_TYPE_IS_USED(DNX_MCDS_GET_TYPE(mcds, block_start));
           ++block_start) {} /* find the next free entry */
      if (block_start <= end_index) { /* found a block start */
        block_size = 1;
        for (cur_entry = block_start + 1; block_size < max_size && cur_entry <= end_index && /* get the current free entries block */
             DNX_MCDS_TYPE_IS_FREE(DNX_MCDS_GET_TYPE(mcds, cur_entry)); ++cur_entry ) {
          ++block_size;
        }
      }
    } else { /* add all entries */
      block_size = block_start + max_size <= end_index ? max_size : end_index - block_start + 1;
    }
    if (block_size) { /* found a free entries block (at least one entry) */
      DNX_MC_ASSERT(block_size <= max_size);
      if (!check_free) { /* mark the block entries as free if working in the mode in which they are used */
        dnx_free_entries_block_size_t i;
        for (i = 0; i < block_size; ++i) {
          DNX_MC_ASSERT(entry_option != dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed ||
            DNX_MCDS_TYPE_IS_USED(DNX_MCDS_GET_TYPE(mcds, block_start + i)));
          DNX_MCDS_SET_TYPE(mcds, block_start + i, DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
          /* write the entry as free in hardware, can be optimized by DMA */
          DNXC_IF_ERR_EXIT(WRITE_IRR_MCDBm(unit, MEM_BLOCK_ANY, block_start + i, ((dnx_mcds_base_t*)mcds)->free_value));
        }
      }
      DNXC_IF_ERR_EXIT( /* add the found block to the region */
        dnx_mcds_create_free_entries_block(mcds, 0, block_start, block_size, region));
    }
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Initialize the MultiCast Data Structures.
 * Do not fill the data from hardware yet.
 * dnx_mcds_multicast_init2() will be called to do so when we can access the MCDB using DMA.
 */
uint32
  dnx_mcds_multicast_init(
    DNX_SAND_IN int      unit
)
{
  uint32 start, end;
  dnx_mcds_base_t *mcds = NULL;
  int failed = 1;
  uint32 table_size;
  uint32 nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
  uint8 is_allocated;
  soc_error_t rv;

  DNXC_INIT_FUNC_DEFS;
  if (!SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCDS not initialized")));
  }
  DNXC_IF_ERR_EXIT(dnx_init_mcds(unit)); /* allocate and init mcds */
  mcds = dnx_get_mcds(unit);
  table_size = DNX_LAST_MCDB_ENTRY(mcds) + 1;

  /* init the members of dnx_mcds_base_t */
  mcds->nof_unoccupied = 0;
  mcds->mcdb = NULL;
  mcds->prev_entries = NULL;

  start = SOC_DNX_CONFIG(unit)->tm.ingress_mc_id_alloc_range_start;
  end   = SOC_DNX_CONFIG(unit)->tm.ingress_mc_id_alloc_range_end;
  if (start < 1) {
    start = 1;
  }
  if (end >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    end = SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids - 1;
  }
  if (end + 1 <= start)  { /* if the region should be empty */
    start = DNX_LAST_MCDB_ENTRY(mcds) + 1;
    end = DNX_LAST_MCDB_ENTRY(mcds);
  }
  dnx_mcds_init_region(&mcds->ingress_alloc_free, DNX_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED, start, end);
  dnx_mcds_init_region(&mcds->free_general, DNX_MCDS_MAX_FREE_BLOCK_SIZE_GENERAL, 1, DNX_LAST_MCDB_ENTRY(mcds) - 1); /* all of the MCDB entries except for the first and the last ones can be used as free entries */
  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) { /* init mcds values for Arad* */
    start = JER2_ARAD_MULT_NOF_MULTICAST_GROUPS + SOC_DNX_CONFIG(unit)->tm.egress_mc_id_alloc_range_start;
    end   = JER2_ARAD_MULT_NOF_MULTICAST_GROUPS + SOC_DNX_CONFIG(unit)->tm.egress_mc_id_alloc_range_end;
    if (end >= DNX_LAST_MCDB_ENTRY(mcds)) {
      end = DNX_LAST_MCDB_ENTRY(mcds) - 1;
    }
  } else {
    start = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1, 0);
    end = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids - 1, nof_active_cores - 1);
    DNX_MC_ASSERT(start == SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids &&
      end == SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids + nof_active_cores * mcds->nof_egr_ll_groups - 1 && end < DNX_LAST_MCDB_ENTRY(mcds));
  }
  dnx_mcds_init_region(&mcds->egress_alloc_free, DNX_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED, start, end);

  assert(sizeof(jer2_arad_mcdb_entry_t) == sizeof(uint32)*DNX_MC_ENTRY_SIZE &&
         sizeof(int) >= sizeof(int32)); /* The mcds will not work if for some reason this is not true */

  { /* allocate memory for the arrays */
    DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &mcds->mcdb, sizeof(jer2_arad_mcdb_entry_t) * table_size, "mcds-mc-mcdb"));
    DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &mcds->prev_entries, sizeof(uint16) * table_size, "mcds-mc-prev_entries"));
    if(!SOC_WARM_BOOT(unit)) {
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.is_allocated(unit, &is_allocated);
        DNXC_IF_ERR_EXIT(rv);
        if(!is_allocated) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.alloc(unit);
            DNXC_IF_ERR_EXIT(rv);
        }
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.alloc_bitmap(unit, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
        DNXC_IF_ERR_EXIT(rv);
        
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.cud2core.alloc_bitmap(unit, mcds->max_egr_cud_field *DNX_CUD2CORE_BITS_PER_CUD);
        DNXC_IF_ERR_EXIT(rv);
        /* init the allocated memory to no core */
        rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.cud2core.bit_range_set(unit,
            0,
            mcds->max_egr_cud_field *DNX_CUD2CORE_BITS_PER_CUD);
        DNXC_IF_ERR_EXIT(rv);
        
    }
  }
#ifdef _JER2_ARAD_MC_TEST_EXTRA_INITIALIZATION_0 /* already done by dnxc_alloc_mem */
  DNXC_IF_ERR_EXIT(dnx_sand_os_memset(mcds->prev_entries, DNX_CUD2CORE_INITIAL_BTYE_VALUE, sizeof(uint16)*table_size));
#endif

 failed = 0;
exit:
  if (failed && mcds) {
    dnx_mcds_multicast_terminate(unit);
  }
  DNXC_FUNC_RETURN;
}

/*
 * Initialize the multicast part of the software database.
 * Must be run after dnx_mcds_multicast_init() was called successfully, and when DMA is up.
 * fills the multicast data from hardware.
 */
uint32
  dnx_mcds_multicast_init2(
    DNX_SAND_IN int      unit
)
{
  unsigned i;
  uint32 *alloced_mem = NULL; /* memory allocated for the duration of this function */
  uint32 *dest32;
  jer2_arad_mcdb_entry_t *dest;
  dnx_mcds_base_t* mcds = dnx_get_mcds(unit);
  uint32 table_size, irdb_table_nof_entries, r32;
  int do_not_read = !SOC_WARM_BOOT(unit);
  int use_dma = !do_not_read &&
#ifdef PLISIM
    !SAL_BOOT_PLISIM &&
#endif
      soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm)); /* check if we can use DMA */
  int failed = 1;
  uint32 range_start, range_end, last_end;

  DNXC_INIT_FUNC_DEFS;
  if (!SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) {
    SOC_EXIT;
  }
  if (!mcds || !mcds->mcdb || !mcds->prev_entries) { /* dnx_mcds_multicast_init() was not called successfully */
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCDS not initialized")));
  }
  table_size = DNX_LAST_MCDB_ENTRY(mcds) + 1;
  irdb_table_nof_entries = table_size / (2 * IRDB_TABLE_GROUPS_PER_ENTRY); /*number of entries in the IRDB table */

  if (!SOC_WARM_BOOT(unit)) {
    if (!SOC_DNX_CONFIG(unit)->jer2_arad->init.pp_enable) {
      DNXC_IF_ERR_EXIT(WRITE_EGQ_INVALID_OUTLIFr(unit, REG_PORT_ANY, DNX_MC_EGR_CUD_INVALID));
      DNXC_IF_ERR_EXIT(WRITE_EPNI_INVALID_OUTLIFr(unit, REG_PORT_ANY, DNX_MC_EGR_CUD_INVALID));
    }
    if (SOC_IS_JERICHO(unit)) { /* Jericho specific configuration */
      /* loop over active cores, setting egress MC MCDB offset for each core */
      for (i = 0; i < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; ++i) {
        uint32 offset = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, 0, i);
        if (offset >= JER2_JER_MCDB_SIZE ) {
            offset += JER2_JER_MCDB_SIZE;
            DNX_MC_ASSERT(i == 0 && offset < JER2_JER_MCDB_SIZE);
        }
        DNXC_IF_ERR_EXIT(READ_EGQ_MULTICAST_OFFSET_ADDRESSr(unit, i, &r32));
        soc_reg_field_set(unit, EGQ_MULTICAST_OFFSET_ADDRESSr, &r32, MCDB_OFFSETf, offset);
        DNXC_IF_ERR_EXIT(WRITE_EGQ_MULTICAST_OFFSET_ADDRESSr(unit, i, r32));
      }
    }
  }

  dest = mcds->mcdb;

  /* fill mcdb from hardware, including if each entry is used or not, use (read) IRR_MCDBm and IDR_IRDBm */
  /* allocate buffers and read tables differently depending on if DMA is enabled */
  if (use_dma) { /* use DMA */
    jer2_arad_mcdb_entry_t *src;
    /* alloced_mem will first contain IRR_MCDBm and later contain IDR_IRDBm */
    alloced_mem = soc_cm_salloc(unit, 4 * (table_size * DNX_MC_ENTRY_SIZE), "dma-mc-buffer");
    if (alloced_mem == NULL) {
      DNXC_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_DNXC_MSG("Failed to allocate DMA buffer")));
    }
    DNXC_IF_ERR_EXIT(soc_mem_read_range(unit, IRR_MCDBm, MEM_BLOCK_ANY, 0, DNX_LAST_MCDB_ENTRY(mcds), alloced_mem));

    /* copy mcdb from dma buffer to to software database */
    src = (void*)alloced_mem;
    for (i = table_size ; i ; --i) {
      dest->word0 = src->word0;
      dest++->word1 = src++->word1 & mcds->msb_word_mask;
    }

  } else { /* do not use DMA, read tables entry by entry */
    DNXC_IF_ERR_EXIT(dnxc_alloc_mem(unit, &alloced_mem, sizeof(uint32) * irdb_table_nof_entries * IRDB_TABLE_ENTRY_WORDS, "mcds-irdb-tmp"));

    /* read mcdb entry by entry into software database. */

    if (do_not_read) { /* if not in warm boot, the MCDB and IRDB tables that we have just filled */
      for (i = DNX_LAST_MCDB_ENTRY(mcds) + 1 ; i ; --i) {
        dest->word0 = ((dnx_mcds_base_t*)mcds)->free_value[0];
        dest++->word1 = ((dnx_mcds_base_t*)mcds)->free_value[1];
      }
    } else {
      DNXC_IF_ERR_EXIT(soc_mem_read_range(unit, IRR_MCDBm, MEM_BLOCK_ANY, 0, DNX_LAST_MCDB_ENTRY(mcds), dest));
      for (i = table_size; i; --i) {
        dest++->word1 &= mcds->msb_word_mask;
      }
    }

  }
  if (!do_not_read) {
    DNXC_IF_ERR_EXIT(soc_mem_read_range(unit, IDR_IRDBm, MEM_BLOCK_ANY, 0, irdb_table_nof_entries - 1, alloced_mem));
  }

#ifdef BCM_WARM_BOOT_SUPPORT /* #if defined(MCAST_WARM_BOOT_UPDATE_ENABLED) && defined(BCM_WARM_BOOT_SUPPORT) ### */
  if (SOC_WARM_BOOT(unit)  /* if warm boot */
     ) {
    /* We now have the mcds initialized from IRR_MCDB, and all entries are marked as unused. We will now process ingress groups */
    uint32 mcid = 0, highest_bitmap_group = SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high;
    unsigned j;
    uint16 group_entries;
    uint32 core_id, nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dest32 = alloced_mem;
    for (i = 0 ; i < irdb_table_nof_entries; ++i) {
      uint32 bits = *dest32;
      for (j = 0 ; j < IRDB_TABLE_GROUPS_PER_ENTRY; ++j) {
        if (bits & 1) { /* we found an open ingress multicast group and will traverse it */
          /* traverse group, needed for warm boot support, mark them with the correct type and update prev_entries */
          jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, mcid);
          uint32 cur_entry = soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf);
          uint32 prev_entry = mcid;

          if (mcid >= SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids || DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
            DNX_MC_ASSERT(0); /* MCID is out of range or the entry is already used */
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("found invalid hardware table values")));
          }
          DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS_START); /* mark the first group entry as the start of an ingress group */
          DNX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, mcid, prev_entry);
          group_entries = 1;
          while (cur_entry != DNX_MCDS_INGRESS_LINK_END(mcds)) { /* mark the rest of the group as non first entries of an ingress group. */
            mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, cur_entry);
            if (DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START || ++group_entries > DNX_MULT_MAX_INGRESS_REPLICATIONS) {
              DNX_MC_ASSERT(0);
              DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("entry already used or too many group entries")));
            }
            DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_INGRESS);
            DNX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, cur_entry, prev_entry);
            prev_entry = cur_entry;
            cur_entry = soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf);
          }

        }
        bits >>= IRDB_TABLE_BITS_PER_GROUP;
        ++mcid;
      }
      dest32 += IRDB_TABLE_ENTRY_WORDS;
    }

    /* We will now traverse the egress groups (bitmap and regular) and build their data. */
    for (mcid = 0; mcid < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids; ++mcid) {
        uint8 bit_val;
        DNXC_IF_ERR_EXIT(
            sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(
                unit,
                mcid,
                &bit_val)
            );
        if (bit_val) { /* we found an open egress multicast group */
          if (mcid > highest_bitmap_group) {
            for (core_id = 0; core_id < nof_active_cores; ++core_id) {
              /* traverse linked list group, needed for warm boot support, mark them with the correct type and update prev_entries */
              uint32 prev_entry = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid, core_id);
              jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, prev_entry);
              uint32 cur_entry;
  
              if (DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
                DNX_MC_ASSERT(0);
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("entry already used")));
              }
              DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_EGRESS_START); /* mark the first group entry as the start of an egress group */
              DNX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, prev_entry, prev_entry);
              DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, prev_entry, DNX_MCDS_TYPE_VALUE_EGRESS_START, &cur_entry)); /* Get the next entry */
              group_entries = 1;
              while (cur_entry != DNX_MC_EGRESS_LINK_PTR_END) { /* mark the rest of the group as non first entries of an egress group. */
                mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, cur_entry);
                if (DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START || ++group_entries > DNX_MULT_MAX_INGRESS_REPLICATIONS) {
                  DNX_MC_ASSERT(0);
                  DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("entry already used or too many group entries")));
                }
                DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_EGRESS);
                DNX_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, cur_entry, prev_entry);
                prev_entry = cur_entry;
                DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, prev_entry, DNX_MCDS_TYPE_VALUE_EGRESS, &cur_entry)); /* Get the next entry */
              }
            }
        }
      }
    }

  } else
#endif /* BCM_WARM_BOOT_SUPPORT */
  {
    /* We now have the mcds initialized from IRR_MCDB, and all entries are marked as unused. We will now process ingress groups */
    dest32 = alloced_mem;
    for (i = 0 ; i < irdb_table_nof_entries; ++i) {
      if (*dest32) {
              DNX_MC_ASSERT(0);
              DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("Illegal IRR_MCDB content")));
      }
      dest32 += IRDB_TABLE_ENTRY_WORDS;
    }

  }

  /* Now we finished marking all the used entries.
   * We will now process the rest of the entries (excluding the first and last entries)
   * and create free blocks from them. */

  {
    /* add free entries from the first half of the table after the ingress allocation range */
    range_start = mcds->ingress_alloc_free.range_start;
    range_end = mcds->ingress_alloc_free.range_end;
    if (range_start <= DNX_LAST_MCDB_ENTRY(mcds) && range_end >= range_start) { /* If we have a valid ingress range */
      DNX_MC_ASSERT(range_start >= 1);
      /* add free entries before the ingress allocation/linked list start range */
      DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, 1, range_start - 1, &mcds->free_general, dnxMcdsFreeBuildBlocksAddOnlyFree));
      /* add free entries from the ingress allocation/linked list start range */
      DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, range_start, range_end, &mcds->ingress_alloc_free, dnxMcdsFreeBuildBlocksAddOnlyFree));
      last_end = range_end;
    } else {
      last_end = 0; /* set the start of the next general free entries range */
    }
    /* add free entries from the first half of the table after the ingress allocation range, and from the second half before the egress allocation range */
    range_start = mcds->egress_alloc_free.range_start;
    range_end = mcds->egress_alloc_free.range_end;
    if (range_start <= DNX_LAST_MCDB_ENTRY(mcds) && range_end >= range_start) { /* If we have a valid egress range */
      DNX_MC_ASSERT(range_start > last_end);
      /* add free entries between the ingress and egress allocation/linked list start ranges */
      DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks( unit, mcds, last_end + 1, range_start - 1, &mcds->free_general, dnxMcdsFreeBuildBlocksAddOnlyFree));
      /* add free entries from the egress allocation/linked list start range */
      if (range_end >= DNX_LAST_MCDB_ENTRY(mcds)) {
        range_end = DNX_LAST_MCDB_ENTRY(mcds) - 1;
      }
      /* add free entries from the egress allocation/linked list start range */
      DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, range_start, range_end, &mcds->egress_alloc_free, dnxMcdsFreeBuildBlocksAddOnlyFree));
      last_end = range_end;
    }
    /* add the remaining entries as free */
    DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, last_end + 1, DNX_LAST_MCDB_ENTRY(mcds) - 1, &mcds->free_general, dnxMcdsFreeBuildBlocksAddOnlyFree));
  }
  failed = 0;

exit:
  if (alloced_mem) {
    if (use_dma) {
      soc_cm_sfree(unit, alloced_mem);
    } else {
      dnxc_free_mem_if_not_null(unit, (void*)&alloced_mem);
    }
  }
  if (failed) {
    dnx_mcds_multicast_terminate(unit);
  }
  DNXC_FUNC_RETURN;
}

/*
 * split a given free block to two blocks: a block of a given size, and the remaining entries.
 * The remaining entries will be added as a new block or merged to an existing block based on flags.
 * It is assumed that the block entries are marked appropriately as free.
 * The new details of the block are returned. Its position changes if the remaining entries are
 * placed at the start of the original block to enable their merge.
 * The input block must not belong to a block list.
 * If a merge is performed, the involved block lists are updated
 */
uint32
  dnx_mcds_split_free_entries_block(
    DNX_SAND_INOUT dnx_mcds_base_t             *mcds,    /* MC SWDB object */
    DNX_SAND_IN    uint32                          flags,       /* RAD_SWDB_MCDB_GET_FREE_BLOCKS_* flags that affect what the function does group */
    DNX_SAND_INOUT dnx_free_entries_blocks_region_t *region,     /* region containing the block */
    DNX_SAND_IN    dnx_free_entries_block_size_t    orig_size,   /* the size of block to be split (must be bigger than new_size) */
    DNX_SAND_IN    dnx_free_entries_block_size_t    new_size,    /* The new block size (of the sub block that will be returned) if one would have been returned, split it */
    DNX_SAND_INOUT uint32                          *block_start /* the start of the block, updated by the function */
)
{
  int unit = mcds->unit;
  uint32 i;
  const uint32 next_block = *block_start + orig_size;
  uint32 prev_block = *block_start - 1;
  const dnx_free_entries_block_size_t remaining_size = orig_size - new_size;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mcds);
  DNXC_NULL_CHECK(block_start);
  if (orig_size > region->max_size || new_size < 1 || new_size >= orig_size) {
    DNX_MC_ASSERT(0);
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("illegal size parameters")));
  }

  if (!(flags & DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT) &&  /* attempt to merge to next block */
      next_block >= region->range_start && next_block <= region->range_end &&
      region == dnx_mcds_get_region(mcds, next_block) &&
      DNX_MCDS_GET_TYPE(mcds, next_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
    dnx_free_entries_block_size_t merged_block_size = DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, next_block);
    dnx_free_entries_block_size_t joint_block_size = merged_block_size + remaining_size;
    if (joint_block_size <= region->max_size) { /* The merged block will not be too big, perform the merge with the next block */
      DNX_MC_ASSERT(next_block - remaining_size == new_size + *block_start);
      dnx_mcds_remove_free_entries_block_from_region(mcds, region, next_block, merged_block_size);
      DNXC_IF_ERR_EXIT(dnx_mcds_create_free_entries_block( /* add the merged block to the region */
        mcds, DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, next_block - remaining_size, joint_block_size, region));
      goto exit;
    }
  }
  
  if (!(flags & DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV) &&  /* attempt to merge to previous block */
             prev_block >= region->range_start && prev_block <= region->range_end &&
             region == dnx_mcds_get_region(mcds, prev_block) &&
             DNX_MCDS_TYPE_IS_FREE(i = DNX_MCDS_GET_TYPE(mcds, prev_block))) {
    dnx_free_entries_block_size_t merged_block_size = 1, joint_block_size;
    if (i != DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START) { /* block size > 1 */
      prev_block = DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, prev_block);
      merged_block_size = *block_start - prev_block;
      DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, prev_block) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
        prev_block < *block_start - 1 && merged_block_size <= region->max_size);
    }
    DNX_MC_ASSERT(merged_block_size == DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, prev_block));
    joint_block_size = merged_block_size + remaining_size;
    if (joint_block_size <= region->max_size) { /* The merged block will not be too big, perform the merge with the previous block */
      DNX_MC_ASSERT(prev_block + joint_block_size == remaining_size + *block_start);
      dnx_mcds_remove_free_entries_block_from_region(mcds, region, prev_block, merged_block_size);
      DNXC_IF_ERR_EXIT(dnx_mcds_create_free_entries_block( /* add the merged block to the region */
        mcds, DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT, prev_block, joint_block_size, region));
      *block_start += remaining_size;
      goto exit;
    }
  }
  
  /* did not merge, add the remaining entries as a new block */
  DNXC_IF_ERR_EXIT(dnx_mcds_create_free_entries_block( /* add the merged block to the region */
    mcds, DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, *block_start + new_size, remaining_size, region));

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Get a free block of size 1 at a given location.
 * Used for getting the first entry of a multicast group.
 * Does not mark mcdb_index as used.
 */
uint32 dnx_mcds_reserve_group_start(
    DNX_SAND_INOUT dnx_mcds_base_t *mcds,
    DNX_SAND_IN    uint32           mcdb_index /* the mcdb indx to reserve */
)
{
  int unit = mcds->unit;
  uint32 entry_type;
  DNXC_INIT_FUNC_DEFS;
  DNX_MC_ASSERT(mcdb_index <= DNX_LAST_MCDB_ENTRY(mcds));

  entry_type = DNX_MCDS_GET_TYPE(mcds, mcdb_index);
  if (DNX_MCDS_TYPE_IS_USED(entry_type)) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("entry must be free")));
  }
  if (mcdb_index > 0 && mcdb_index < DNX_LAST_MCDB_ENTRY(mcds)) { /* entry needs allocation */
    dnx_free_entries_blocks_region_t* region = dnx_mcds_get_region(mcds, mcdb_index);
    const uint32 block_start = entry_type == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START ?
      mcdb_index : DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, mcdb_index);
    const dnx_free_entries_block_size_t block_size = DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block_start);
    const uint32 block_end = block_start + block_size - 1;
    DNX_MC_ASSERT(block_start <= mcdb_index && block_start + region->max_size >= mcdb_index && block_size <= region->max_size);

    dnx_mcds_remove_free_entries_block_from_region(mcds, region, block_start, block_size); /* remove the existing free block from the free list */
    if (block_start < mcdb_index) { /* create free block for entries before mcdb_index */
      DNXC_IF_ERR_EXIT(dnx_mcds_create_free_entries_block(
        mcds, DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT, block_start, mcdb_index - block_start, region));
    }
    if (block_end > mcdb_index) { /* create free block for entries after mcdb_index */
      DNXC_IF_ERR_EXIT(dnx_mcds_create_free_entries_block(
        mcds, DNX_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, mcdb_index + 1, block_end - mcdb_index, region));
    }
  }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Get a free entries block of a given size, according to flags that needs to be used to start a multicast group.
 * Returns the start index and the number of entries in the block.
 */
uint32 dnx_mcds_get_free_entries_block(
    DNX_SAND_INOUT dnx_mcds_base_t              *mcds,
    DNX_SAND_IN    uint32                        flags,        /* DNX_MCDS_GET_FREE_BLOCKS_* flags that affect what the function does group */
    DNX_SAND_IN    dnx_free_entries_block_size_t wanted_size,  /* needed size of the free block group */
    DNX_SAND_IN    dnx_free_entries_block_size_t max_size,     /* do not return blocks above this size, if one would have been returned, split it */
    DNX_SAND_OUT   uint32                        *block_start, /* the start of the relocation block */
    DNX_SAND_OUT   dnx_free_entries_block_size_t *block_size   /* the size of the returned block */
)
{
  int unit = mcds->unit;
  dnx_free_entries_blocks_region_t *regions[JER2_ARAD_MCDS_NOF_REGIONS];
  int do_change = !(flags & DNX_MCDS_GET_FREE_BLOCKS_NO_UPDATES);
  uint32 block = 0;
  int r, s, loop_start, loop_end;
  int size_loop1_start, size_loop1_increase;
  int size_loop2_start, size_loop2_increase;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mcds);
  DNXC_NULL_CHECK(block_start);
  DNXC_NULL_CHECK(block_size);
  if (wanted_size > DNX_MCDS_MAX_FREE_BLOCK_SIZE || wanted_size > max_size || 1 > wanted_size) {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("illegal wanted size")));
  }

  regions[0] = &mcds->free_general;
  if (flags & DNX_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS) { /* select region order according to flags */
    regions[1] = &mcds->ingress_alloc_free;
    regions[2] = &mcds->egress_alloc_free;
  } else {
    regions[1] = &mcds->egress_alloc_free;
    regions[2] = &mcds->ingress_alloc_free;
  }

  if (flags & DNX_MCDS_GET_FREE_BLOCKS_PREFER_SMALL) { /* select (loop) free entries block size order according to flags */
    size_loop1_start = wanted_size; size_loop1_increase = -1;
    size_loop2_start = wanted_size + 1; size_loop2_increase = 1;
  } else {
    size_loop1_start = wanted_size; size_loop1_increase = 1;
    size_loop2_start = wanted_size - 1; size_loop2_increase = -1;
  }

  if (flags & DNX_MCDS_GET_FREE_BLOCKS_PREFER_SIZE) { /* Will prefer to return a block of a better size than in a better region */

    /* first loop over block sizes */
    loop_start = size_loop1_start;
    if (size_loop1_increase >= 0) { /* increasing loop */
      loop_end = DNX_MCDS_MAX_FREE_BLOCK_SIZE + 1;
    } else { /* decreasing loop */
      loop_end = 0;
    }
    for (s = loop_start; s != loop_end; s += size_loop1_increase) {
      for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
        dnx_free_entries_blocks_region_t *region = regions[r];
        if (region->max_size >= s) { /* if the current block size is supported by the region */
          if ((block = dnx_mcds_get_free_entries_block_from_list(mcds, region->lists + (s - 1), do_change))) {
            goto found;
          }
        }
      }
    }
    /* second loop over block sizes */
    loop_start = size_loop2_start;
    if (size_loop2_increase >= 0) { /* increasing loop */
      loop_end = DNX_MCDS_MAX_FREE_BLOCK_SIZE + 1;
    } else { /* decreasing loop */
      loop_end = 0;
    }
    for (s = loop_start; s != loop_end; s += size_loop2_increase) {
      for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
        dnx_free_entries_blocks_region_t *region = regions[r];
        if (region->max_size >= s) { /* if the current block size is supported by the region */
          if ((block = dnx_mcds_get_free_entries_block_from_list(mcds, region->lists + (s - 1), do_change))) {
            goto found;
          }
        }
      }
    }

  } else { /* Will prefer to return a block in a better region than a block of a better size. */

    for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
      dnx_free_entries_blocks_region_t *region = regions[r];
      /* coverity[pointer_outside_base_object:FALSE] */
      dnx_free_entries_block_list_t *lists = region->lists - 1;

      /* first loop over block sizes */
      loop_start = size_loop1_start;
      if (size_loop1_increase >= 0) { /* increasing loop */
        loop_end = region->max_size + 1;
        if (loop_start > loop_end) loop_start = loop_end;
      } else { /* decreasing loop */
        loop_end = 0;
        if (loop_start > region->max_size) loop_start = region->max_size;
      }
      for (s = loop_start; s != loop_end; s += size_loop1_increase) {
       if ((block = dnx_mcds_get_free_entries_block_from_list(mcds, lists + s, do_change))) {
         goto found;
       }
      }
      /* second loop over block sizes */
      loop_start = size_loop2_start;
      if (size_loop2_increase >= 0) { /* increasing loop */
        loop_end = region->max_size + 1;
        if (loop_start > loop_end) loop_start = loop_end;
      } else { /* decreasing loop */
        loop_end = 0;
        if (loop_start > region->max_size) loop_start = region->max_size;
      }
      for (s = loop_start; s != loop_end; s += size_loop2_increase) {
       if ((block = dnx_mcds_get_free_entries_block_from_list(mcds, lists + s, do_change))) {
         goto found;
       }
      }

    } /* end of regions loop */

  } /* end of preferred better region */

  DNX_MC_ASSERT(!mcds->nof_unoccupied);
  if (flags & DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL) {
    *block_start = 0;
    *block_size = 0;
    SOC_EXIT;
  }
  DNXC_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_DNXC_MSG("did not find any free block")));

found:
  if (do_change && s > max_size) {
    DNX_MC_ASSERT(s <= DNX_MCDS_MAX_FREE_BLOCK_SIZE);
    DNXC_IF_ERR_EXIT( /* get free entries */
      dnx_mcds_split_free_entries_block(mcds, flags, regions[r], s, max_size, &block));
    s = max_size;
  }

  *block_start = block;
  *block_size = s;
  DNX_MC_ASSERT(block && s);

exit:
  DNXC_FUNC_RETURN;
}
/* 
* Given a table index checks if that entry is of a consecutive format
*/
static uint32 
    dnx_mcds_is_egress_format_consecutive_next(
        int                      unit,
        uint8                    format,
        uint8                    *is_consecutive
)
{
    DNXC_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO(unit)) {
        if (format == 3 || (format >= 12 && format <= 15)) {
            *is_consecutive = TRUE;
        } else if (format == 0 || format == 1 || format == 2 || format == 3 || (format >= 4 && format <= 7) || (format >= 8 && format <= 11)) {
            *is_consecutive = FALSE;
        } else {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("wrong egress format")));
        }

    } else {
         *is_consecutive = JER2_ARAD_MCDS_IS_EGRESS_FORMAT_CONSECUTIVE_NEXT(format);
    }
exit:
  DNXC_FUNC_RETURN;
}


/*
 * Given a table index that needs to be used to start a multicast group,
 * returns the start index and the number of entries that need to be relocated.
 * If the number of entries returned is 0, a relocation is not needed.
 */
uint32
  dnx_mcds_get_relocation_block(
    DNX_SAND_IN  dnx_mcds_base_t             *mcds,
    DNX_SAND_IN  uint32                       mcdb_index,              /* table index needed for the start of a group */
    DNX_SAND_OUT uint32                       *relocation_block_start, /* the start of the relocation block */
    DNX_SAND_OUT dnx_free_entries_block_size_t *relocation_block_size   /* the size of the relocation block, 0 if relocation is not needed */
)
{
  int unit = mcds->unit;
  uint32 group_entry_type, start = mcdb_index;
  dnx_free_entries_block_size_t size = 1;
  uint8 is_consecutive;

  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mcds);
  DNXC_NULL_CHECK(relocation_block_size);

  group_entry_type = DNX_MCDS_GET_TYPE(mcds, mcdb_index);
  if (DNX_MCDS_TYPE_IS_USED(group_entry_type) &&  /* relocation is needed if the entry is used and is not the start of a group */
    !DNX_MCDS_TYPE_IS_START(group_entry_type)) {
    if (DNX_MCDS_TYPE_IS_EGRESS_NORMAL(group_entry_type)) { /* ingress & TDM groups do not have used consecutive blocks */

      uint32 entry, next_entry;
      DNX_MC_ASSERT(group_entry_type == DNX_MCDS_TYPE_VALUE_EGRESS);
      for (entry = mcdb_index; ; entry = next_entry) { /* look for consecutive entries before the given one */
        next_entry = DNX_MCDS_GET_PREV_ENTRY(mcds, entry);
        DNXC_IF_ERR_EXIT(dnx_mcds_is_egress_format_consecutive_next(unit,DNX_MCDS_GET_EGRESS_FORMAT(mcds, next_entry), &is_consecutive));
        if (next_entry + 1 != entry || /* previous entry is not consecutive, or it has a link pointer */
            !is_consecutive) {
          break;
        }
        DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, next_entry) == DNX_MCDS_TYPE_VALUE_EGRESS); /* must be a none TDM egress entry, and not the group start */
        ++size;
        DNX_MC_ASSERT(next_entry && size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE);
      }
      start = entry;
      DNXC_IF_ERR_EXIT(dnx_mcds_is_egress_format_consecutive_next(unit,DNX_MCDS_GET_EGRESS_FORMAT(mcds, mcdb_index), &is_consecutive));
      for (entry = mcdb_index; is_consecutive;)
      { /* look for consecutive entries after the given one */
        ++size;
        ++entry;
        DNX_MC_ASSERT(entry <= DNX_LAST_MCDB_ENTRY(mcds) && size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE);
        DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, entry) == DNX_MCDS_TYPE_VALUE_EGRESS); /* must be a none TDM egress entry, and not the group start */
        DNXC_IF_ERR_EXIT(dnx_mcds_is_egress_format_consecutive_next(unit,DNX_MCDS_GET_EGRESS_FORMAT(mcds, entry), &is_consecutive));
      }
      DNX_MC_ASSERT(entry - start + 1 == size);

    }
  } else { /* no relocation is needed */
    DNX_MC_ASSERT(DNX_MCDS_TYPE_IS_FREE(group_entry_type) || (mcdb_index < SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids ? /* is ingress group */
      group_entry_type == DNX_MCDS_TYPE_VALUE_INGRESS_START : (DNX_MCDS_TYPE_IS_EGRESS_START(group_entry_type) &&
      mcdb_index < DNX_MCDS_GET_EGRESS_GROUP_START(mcds, 0, SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores))));
  }

  *relocation_block_size = size;
  if (relocation_block_start) {
    *relocation_block_start = start;
  }

exit:
  DNXC_FUNC_RETURN;
}



/*
 * Write a MCDB entry to hardware from the mcds.
 * Using only this function for writes, and using it after mcds mcdb used
 * entries updates, ensures consistency between the mcds and the hardware.
 */

uint32
  dnx_mcds_write_entry(
    DNX_SAND_IN int unit,
    DNX_SAND_IN uint32 mcdb_index /* index of entry to write */
)
{
  uint32 data[DNX_MC_ENTRY_SIZE];
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, mcdb_index);

  DNXC_INIT_FUNC_DEFS;

  data[0] = entry->word0;
  data[1] = entry->word1 & mcds->msb_word_mask;
  DNXC_IF_ERR_EXIT(WRITE_IRR_MCDBm(unit, MEM_BLOCK_ANY, mcdb_index, data));

exit:
  DNXC_FUNC_RETURN;
}

int _dnx_mcds_test_free_entries(
    DNX_SAND_IN int unit
)
{
  uint32 nof_unoccupied = 0;
  dnx_mcds_base_t* mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *entry, *entry2;
  JER2_ARAD_MULT_ID mcid;


  /* init test bit to be 1 if the entry is free, and count free entries */
  for (mcid = 0; mcid <= DNX_LAST_MCDB_ENTRY(mcds); ++mcid) {
    entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, mcid);
    if (DNX_MCDS_TYPE_IS_FREE(DNX_MCDS_ENTRY_GET_TYPE(entry))) {
      DNX_MCDS_ENTRY_SET_TEST_BIT_ON(entry);
      ++nof_unoccupied;
    } else {
      DNX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    }
  }
  /* decrease from the free entry count the two entries which may not be allocated in case they are free */
  if (DNX_MCDS_TYPE_IS_FREE(DNX_MCDS_ENTRY_GET_TYPE(entry =
      DNX_MCDS_GET_MCDB_ENTRY(mcds, DNX_MCDS_INGRESS_LINK_END(mcds))))) {
    DNX_MC_ASSERT(nof_unoccupied);
    DNX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    --nof_unoccupied;
  }
  if (DNX_MCDS_TYPE_IS_FREE(DNX_MCDS_ENTRY_GET_TYPE(entry =
      DNX_MCDS_GET_MCDB_ENTRY(mcds, DNX_MC_EGRESS_LINK_PTR_END)))) {
    DNX_MC_ASSERT(nof_unoccupied);
    DNX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    --nof_unoccupied;
  }
  if (nof_unoccupied != mcds->nof_unoccupied) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST,
             (BSL_META_U(unit,
                         "The mcdb has %lu free allocatable entries and in the mcds the value is %lu\n"), (unsigned long)nof_unoccupied, (unsigned long)mcds->nof_unoccupied));
    DNX_MC_ASSERT(0);
    return 10;
  }

  /* process over the free block lists */
  nof_unoccupied = 0;
  {
      dnx_free_entries_blocks_region_t *regions[JER2_ARAD_MCDS_NOF_REGIONS];
    int r;
    dnx_free_entries_block_size_t size, size_i;
    uint32 block, first_block, prev_block;
    regions[0] = &mcds->free_general;
    regions[1] = &mcds->ingress_alloc_free;
    regions[2] = &mcds->egress_alloc_free;

    /* loop over regions, processing the entries of each block of each list; checking their validity and counting entries */
    for (r = 0; r < JER2_ARAD_MCDS_NOF_REGIONS; ++r) {
      dnx_free_entries_blocks_region_t *region = regions[r];
      dnx_free_entries_block_list_t *lists = region->lists;
      DNX_MC_ASSERT(region->max_size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE && region->max_size > 0);

      for (size = region->max_size; size; --size) { /* loop over the block sizes of the region */
        /* loop over the blocks in the list */
        if ((block = dnx_mcds_get_free_entries_block_from_list(mcds, lists + size - 1, 0))) { /* if the list is not empty */
          prev_block = DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, block);
          first_block = block;

          /* loop over the free block in the list */
          do {
            entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, block);
            DNX_MC_ASSERT(block >= region->range_start && block + size - 1 <= region->range_end);
            DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_TYPE(entry) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK_START);
            DNX_MC_ASSERT(DNX_MCDS_GET_FREE_BLOCK_SIZE(mcds, block) == size);
            DNX_MC_ASSERT(prev_block == DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, block));
            if (!DNX_MCDS_ENTRY_GET_TEST_BIT(entry)) {
              LOG_ERROR(BSL_LS_SOC_MULTICAST,
                       (BSL_META_U(unit,
                                   "Free block %lu of size %u appeared previously in a linked list\n"), (unsigned long)block, size));
              DNX_MC_ASSERT(0);
              return 20;
            }
            DNX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
            entry2 = entry;

            for (size_i = 1; size_i < size;  ++ size_i) { /* loop over remianing entries of the block */
              ++entry2;
              DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_TYPE(entry2) == DNX_MCDS_TYPE_VALUE_FREE_BLOCK);
              DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_FREE_PREV_ENTRY(entry2) == block);
              if (!DNX_MCDS_ENTRY_GET_TEST_BIT(entry2)) {
                LOG_ERROR(BSL_LS_SOC_MULTICAST,
                         (BSL_META_U(unit,
                                     "Free entry %lu of free block %lu of size %u appeared previously in a linked list\n"),
                                     (unsigned long)(block + size ), (unsigned long)block, size));
                DNX_MC_ASSERT(0);
                return 30;
              }
            DNX_MCDS_ENTRY_SET_TEST_BIT_OFF(entry2);
            }
            nof_unoccupied += size;
            prev_block = block;
            block = DNX_MCDS_GET_FREE_NEXT_ENTRY(mcds, block); /* move to new block */
          } while (block != first_block);
          DNX_MC_ASSERT(prev_block == DNX_MCDS_GET_FREE_PREV_ENTRY(mcds, block));
        }
      }
    }
  }
  if (nof_unoccupied != mcds->nof_unoccupied) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST,
             (BSL_META_U(unit,
                         "The mcdb free block lists contain %lu entries and in the mcds the value is %lu\n"), (unsigned long)nof_unoccupied, (unsigned long)mcds->nof_unoccupied));
    DNX_MC_ASSERT(0);
    return 40;
  }

  return 0;
}


uint32
    dnx_mcds_multicast_terminate(
        DNX_SAND_IN int unit
    )
{
    dnx_mcds_base_t* mcds = dnx_get_mcds(unit);
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnxc_free_mem(unit, (void**)&mcds->mcdb));
    DNXC_IF_ERR_EXIT(dnxc_free_mem(unit, (void**)&mcds->prev_entries));
    DNXC_IF_ERR_EXIT(dnx_deinit_mcds(unit));

    SOC_EXIT;
exit:
  DNXC_FUNC_RETURN;
}

/* Get the type of a MCDB entry */
uint32 dnx_get_mcdb_entry_type(
    DNX_SAND_IN  dnx_mcdb_entry_t* entry
)
{
    return DNX_MCDS_ENTRY_GET_TYPE((jer2_arad_mcdb_entry_t*)entry);
}
/* set the type of a MCDB entry */
void dnx_set_mcdb_entry_type(
    DNX_SAND_INOUT  dnx_mcdb_entry_t* entry,
    DNX_SAND_IN     uint32 type_value
)
{
    jer2_arad_mcdb_entry_t *e = (jer2_arad_mcdb_entry_t*)entry;
    DNX_MCDS_ENTRY_SET_TYPE(e, type_value);
}


/* Arad entry format writing functions */

/*
 * This function writes the hardware fields of egress format 0 (port+CUD replications with a link pointer) to a jer2_arad_mcdb_entry_t structure.
 */
void jer2_arad_mult_egr_write_entry_port_cud(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN    dnx_rep_data_t    *rep2,       /* replication 2 (7 bit port) */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dnx_mc_outlif_t cud1, cud2;
  dnx_mc_local_port_t port1, port2;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
      port1 = DNX_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep2);
      if (!rep1) {
          cud1 = cud2;
      } else {
          DNX_MC_ASSERT(cud1 == cud2);
      }
  } else {
      port2 = JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Af, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Bf, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, ENTRY_FORMATf, 0);
}

/*
 * This function writes the hardware fields of egress format 4/5 (port_CUD replications with no link pointer) to a jer2_arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void jer2_arad_mult_egr_write_entry_port_cud_noptr(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN    dnx_rep_data_t    *rep2,       /* replication 2 */
    DNX_SAND_IN    uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                  /* select format indicating that the following entry is next. */
)
{
  dnx_mc_outlif_t cud1, cud2;
  dnx_mc_local_port_t port1, port2;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
      port1 = DNX_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep2);
  } else {
      cud2 = DNX_MC_EGR_OUTLIF_DISABLED;
      port2 = DNX_MULT_EGRESS_PORT_INVALID;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, ENTRY_FORMATf, use_next ? 5 : 4);
}

/*
 * This function writes the hardware fields of egress format 2 (CUD only with link pointer) to a jer2_arad_mcdb_entry_t structure.
 */
void jer2_arad_mult_egr_write_entry_cud(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN    dnx_rep_data_t    *rep2,       /* replication 2  */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dnx_mc_outlif_t cud1, cud2;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DNX_MC_EGR_OUTLIF_DISABLED;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, ENTRY_FORMATf, 1);
}

/*
 * This function writes the hardware fields of egress format 6/7 (CUD only with no link pointer) to a jer2_arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void jer2_arad_mult_egr_write_entry_cud_noptr(
    DNX_SAND_IN     int               unit,
    DNX_SAND_INOUT  jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN     dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN     dnx_rep_data_t    *rep2,       /* replication 2 */
    DNX_SAND_IN     dnx_rep_data_t    *rep3,       /* replication 3 */
    DNX_SAND_IN     uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                   /* select format indicating that the following entry is next. */
)
{
  dnx_mc_outlif_t cud1, cud2, cud3;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep3) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep3) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud3 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep3);
  } else {
      cud3 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f, cud3);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, ENTRY_FORMATf, use_next ? 7 : 6);
}

/*
 * This function writes the hardware fields of egress format 1 (bitmap+CUD) to a jer2_arad_mcdb_entry_t structure.
 */
void jer2_arad_mult_egr_write_entry_bm_cud(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep,        /* the replication */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dnx_mc_outlif_t cud;
  dnx_mc_bitmap_id_t bm_id;
  if (rep) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_EGR_BM_CUD);
      cud = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep);
      bm_id = DNX_MCDS_REP_DATA_GET_EGR_BM_ID(rep);
  } else {
      cud = DNX_MC_EGR_OUTLIF_DISABLED;
      bm_id = DNX_MC_EGR_BITMAP_DISABLED;
  }
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f, cud);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, BMP_PTRf, bm_id);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, ENTRY_FORMATf, 1);
}



/* Jericho entry format writing functions */

/*
 * This function writes the hardware fields of egress format 0 (port+CUD replications with a link pointer) to a jer2_arad_mcdb_entry_t structure.
 */
void jer2_jer_mult_egr_write_entry_port_cud(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN    dnx_rep_data_t    *rep2,       /* replication 2 */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dnx_mc_outlif_t cud1, cud2;
  dnx_mc_local_port_t port1, port2;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
      port1 = DNX_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep2);
      if (!rep1) {
          cud1 = cud2;
      } else {
          DNX_MC_ASSERT(cud1 == cud2);
      }
  } else {
      port2 = DNX_MULT_EGRESS_PORT_INVALID;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Af, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Bf, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, ENTRY_FORMATf, 0);
}

/*
 * This function writes the hardware fields of egress format 4/5 (port_CUD replications with no link pointer) to a jer2_arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void jer2_jer_mult_egr_write_entry_port_cud_noptr(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN    dnx_rep_data_t    *rep2,       /* replication 2 */
    DNX_SAND_IN    uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                  /* select format indicating that the following entry is next. */
)
{
  dnx_mc_outlif_t cud1, cud2;
  dnx_mc_local_port_t port1, port2;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
      port1 = DNX_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DNX_MCDS_REP_DATA_GET_EGR_PORT(rep2);
  } else {
      cud2 = DNX_MC_EGR_OUTLIF_DISABLED;
      port2 = DNX_MULT_EGRESS_PORT_INVALID;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, ENTRY_FORMATf, use_next ? 3 : 2);
}

/*
 * This function writes the hardware fields of egress format 2 (CUD only with link pointer) to a jer2_arad_mcdb_entry_t structure.
 */
void jer2_jer_mult_egr_write_entry_cud(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN    dnx_rep_data_t    *rep2,       /* replication 2  */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dnx_mc_outlif_t cud1, cud2;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DNX_MC_EGR_OUTLIF_DISABLED;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, ENTRY_FORMATf, 1);
}

/*
 * This function writes the hardware fields of egress format 6/7 (CUD only with no link pointer) to a jer2_arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void jer2_jer_mult_egr_write_entry_cud_noptr(
    DNX_SAND_IN     int               unit,
    DNX_SAND_INOUT  jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN     dnx_rep_data_t    *rep1,       /* replication 1 */
    DNX_SAND_IN     dnx_rep_data_t    *rep2,       /* replication 2 */
    DNX_SAND_IN     dnx_rep_data_t    *rep3,       /* replication 3 */
    DNX_SAND_IN     uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                   /* select format indicating that the following entry is next. */
)
{
  dnx_mc_outlif_t cud1, cud2, cud3;
  if (rep1) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep1) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep2) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep3) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep3) == DNX_MCDS_REP_TYPE_EGR_CUD);
      cud3 = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep3);
  } else {
      cud3 = DNX_MC_EGR_OUTLIF_DISABLED;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f, cud3);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, ENTRY_FORMATf, use_next ? 3 : 2);
}

/*
 * This function writes the hardware fields of egress format 1 (bitmap+CUD) to a jer2_arad_mcdb_entry_t structure.
 */
void jer2_jer_mult_egr_write_entry_bm_cud(
    DNX_SAND_IN    int               unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    DNX_SAND_IN    dnx_rep_data_t    *rep,        /* the replication */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dnx_mc_outlif_t cud;
  dnx_mc_bitmap_id_t bm_id;
  if (rep) {
      DNX_MC_ASSERT(DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_EGR_BM_CUD);
      cud = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep);
      bm_id = DNX_MCDS_REP_DATA_GET_EGR_BM_ID(rep);
  } else {
      cud = DNX_MC_EGR_OUTLIF_DISABLED;
      bm_id = DNX_MC_EGR_BITMAP_DISABLED;
  }
  mcdb_entry->word0 = 0; /* Zero bits not covered by these fields */
  mcdb_entry->word1 &= ~JER2_JER_MC_ENTRY_MASK_VAL;
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f, cud);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, BMP_PTRf, bm_id);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, ENTRY_FORMATf, 1);
}


/* Allocate and init the mcds structure, not allocating memories it points to */
uint32 dnx_init_mcds(
    DNX_SAND_IN    int         unit
)
{
    dnx_mcds_base_t *dnx_base;
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_alloc_mcds(unit, sizeof(*dnx_base), (void*)&dnx_base));

    dnx_base->unit = unit;
    dnx_base->common.flags = 0;
    dnx_base->nof_egr_ll_groups = SOC_DNX_CONFIG(unit)->tm.nof_mc_ids - (SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1); /* The number of egress linked list groups (per core) */
    dnx_base->common.get_mcdb_entry_type = dnx_get_mcdb_entry_type;
    dnx_base->common.set_mcdb_entry_type = dnx_set_mcdb_entry_type;
    dnx_base->set_egress_linked_list = dnx_mcds_set_egress_linked_list;
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) { /* init mcds values for Arad* */
        dnx_base->common.get_mcdb_entry_from_mcds = dnx_mcds_get_mcdb_entry_from_mcds;
        dnx_base->common.get_next_pointer = jer2_arad_mcdb_get_next_pointer;
        dnx_base->common.set_next_pointer = jer2_arad_mcdb_set_next_pointer;

        dnx_base->common.ingress_link_end = JER2_ARAD_MC_INGRESS_LINK_PTR_END;
        dnx_base->free_value[0] = JER2_ARAD_MC_UNOCCUPIED_ENTRY_LOW;
        dnx_base->free_value[1] = JER2_ARAD_MC_UNOCCUPIED_ENTRY_HIGH;
        dnx_base->empty_ingr_value[0] = JER2_ARAD_MC_ING_EMPTY_ENTRY_LOW;
        dnx_base->empty_ingr_value[1] = JER2_ARAD_MC_ING_EMPTY_ENTRY_HIGH;
        dnx_base->msb_word_mask = JER2_ARAD_MC_ENTRY_MASK_VAL;
        dnx_base->ingr_word1_replication_mask = 3;
        dnx_base->max_egr_cud_field = dnx_base->max_ingr_cud_field = ((1 << 16) - 1);
        /* The offset in the MCDB to which the MCID is added to get the first entry of the group of core 0 */
        dnx_base->egress_mcdb_offset = JER2_ARAD_MULT_NOF_MULTICAST_GROUPS;

        dnx_base->egr_mc_write_entry_port_cud = jer2_arad_mult_egr_write_entry_port_cud;
        dnx_base->egr_mc_write_entry_port_cud_noptr = jer2_arad_mult_egr_write_entry_port_cud_noptr;
        dnx_base->egr_mc_write_entry_cud = jer2_arad_mult_egr_write_entry_cud;
        dnx_base->egr_mc_write_entry_cud_noptr = jer2_arad_mult_egr_write_entry_cud_noptr;
        dnx_base->egr_mc_write_entry_bm_cud = jer2_arad_mult_egr_write_entry_bm_cud;
        dnx_base->get_replications_from_entry = jer2_arad_mcds_get_replications_from_entry;
        dnx_base->convert_ingress_replication_hw2api = jer2_arad_convert_ingress_replication_hw2api;

        
        switch (/*SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.fmc_dbuff_mode*/ -1) {
          case JER2_ARAD_INIT_FMC_4K_REP_64K_DBUFF_MODE:
            dnx_base->max_nof_ingr_replications = 4096 - DNX_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
            break;
          case JER2_ARAD_INIT_FMC_64_REP_128K_DBUFF_MODE:
            dnx_base->max_nof_ingr_replications = 64 - DNX_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
            break;
          default:
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("invalid buffer mode")));
        }
        dnx_base->max_nof_mmc_replications = 1; 

#ifdef BCM_88675_A0
    } else { /* init mcds values for Jericho */
        dnx_base->common.get_mcdb_entry_from_mcds = dnx_mcds_get_mcdb_entry_from_mcds;
        dnx_base->common.get_next_pointer = jer2_jer_mcdb_get_next_pointer;
        dnx_base->common.set_next_pointer = jer2_jer_mcdb_set_next_pointer;

        dnx_base->common.ingress_link_end = JER2_JER_MC_INGRESS_LINK_PTR_END;
        dnx_base->free_value[0] = JER2_JER_MC_UNOCCUPIED_ENTRY_LOW;
        dnx_base->free_value[1] = JER2_JER_MC_UNOCCUPIED_ENTRY_HIGH;
        dnx_base->empty_ingr_value[0] = JER2_JER_MC_ING_EMPTY_ENTRY_LOW;
        dnx_base->empty_ingr_value[1] = JER2_JER_MC_ING_EMPTY_ENTRY_HIGH;
        dnx_base->msb_word_mask = JER2_JER_MC_ENTRY_MASK_VAL;
        dnx_base->ingr_word1_replication_mask = 0x1f;
        dnx_base->max_ingr_cud_field = ((1 << 19) - 1);
        dnx_base->max_egr_cud_field = ((1 << 18) - 1);
        /* The offset in the MCDB to which the MCID is added to get the first entry of the group of core 0 */
        dnx_base->egress_mcdb_offset = SOC_DNX_CONFIG(unit)->tm.nof_ingr_mc_ids -
          (SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1);

        dnx_base->egr_mc_write_entry_port_cud = jer2_jer_mult_egr_write_entry_port_cud;
        dnx_base->egr_mc_write_entry_port_cud_noptr = jer2_jer_mult_egr_write_entry_port_cud_noptr;
        dnx_base->egr_mc_write_entry_cud = jer2_jer_mult_egr_write_entry_cud;
        dnx_base->egr_mc_write_entry_cud_noptr = jer2_jer_mult_egr_write_entry_cud_noptr;
        dnx_base->egr_mc_write_entry_bm_cud = jer2_jer_mult_egr_write_entry_bm_cud;
        dnx_base->get_replications_from_entry = jer2_jer_mcds_get_replications_from_entry;
        dnx_base->convert_ingress_replication_hw2api = jer2_jer_convert_ingress_replication_hw2api;

        
        DNXC_LEGACY_FIXME_ASSERT;
#ifdef FIXME_DNX_LEGACY
        switch (SOC_DNX_CONFIG(unit)->jer2_arad->init.dram.fmc_dbuff_mode) {
          case JER2_JERICHO_INIT_FMC_64_REP_512K_DBUFF_MODE:
            dnx_base->max_nof_ingr_replications = 64 - DNX_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
            break;
          default: /* in case DRAM buffers support 4K replications, or no DRAM buffers */
            dnx_base->max_nof_ingr_replications = 4096 - DNX_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
        }
        dnx_base->max_nof_mmc_replications = 8 - DNX_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
#endif 
#endif /* BCM_88675_A0 */
    }
exit:
    DNXC_FUNC_RETURN;
}


/* De-allocate the mcds structure, not de-allocating memories it points to */
uint32 dnx_deinit_mcds(
    DNX_SAND_IN    int         unit
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_dealloc_mcds(unit));

exit:
    DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

