/* $Id: dpp_multicast_imp.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

#include <soc/dpp/ARAD/arad_sw_db.h>
#include <soc/dpp/multicast_imp.h>
#include <soc/dcmn/error.h>
/* needed to init function pointers in MCDS */
#include <soc/dpp/ARAD/arad_multicast_imp.h>
#include <soc/dpp/JER/jer_multicast_imp.h>


#include <soc/mcm/memregs.h>

#include <shared/bsl.h>


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dcmn/error.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dcmn/dcmn_mem.h>

#include <soc/dpp/SAND/Utils/sand_framework.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define ARAD_MC_INGRESS_LINK_PTR_END 0x1ffff /* marks the end of an ingress linked list, the last entry in the MCDB */
#define JER_MC_INGRESS_LINK_PTR_END 0x3ffff /* marks the end of an ingress linked list, the last entry in the MCDB */
#define JER_MCDB_SIZE (JER_MC_INGRESS_LINK_PTR_END + 1)

/* The value used for unoccupied entries, these entries mean to hardware an empty group if appearing as the first entry of an egress group, except for TDM */
/* The values are also used for empty egress linked lists */
#define ARAD_MC_UNOCCUPIED_ENTRY_LOW 0x7fffffff
#define ARAD_MC_UNOCCUPIED_ENTRY_HIGH 0
#define JER_MC_UNOCCUPIED_ENTRY_LOW 0xffffffff
#define JER_MC_UNOCCUPIED_ENTRY_HIGH 3

/* The value used for empty ingress MC groups */
#define ARAD_MC_ING_EMPTY_ENTRY_LOW  0xffffffff
#define ARAD_MC_ING_EMPTY_ENTRY_HIGH 0x7ffff
#define JER_MC_ING_EMPTY_ENTRY_LOW  0xffffffff
#define JER_MC_ING_EMPTY_ENTRY_HIGH 0x7fffff

#define IRDB_TABLE_GROUPS_PER_ENTRY 16
#define IRDB_TABLE_BITS_PER_GROUP 2

#define DPP_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS 2 /* replication slots reserved in buffers for snoop and mirror */

#define JER_MC_SPECIAL_MODE_ENTRIES_PER_ENTRY 4

uint32 dpp_init_mcds(int unit);
uint32 dpp_deinit_mcds(int unit);

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
dpp_mcdb_entry_t*
  dpp_mcds_get_mcdb_entry(
    SOC_SAND_IN  int unit,
    SOC_SAND_IN  uint32 mcdb_index
)
{
  return ((dpp_mcds_base_t*)dpp_get_mcds(unit))->mcdb + mcdb_index;
}
/*
 * Get a pointer to the mcdb entry with the given index in the mcds
 */
dpp_mcdb_entry_t* dpp_mcds_get_mcdb_entry_from_mcds(
    SOC_SAND_IN  dpp_mcds_t* mcds,
    SOC_SAND_IN  uint32 mcdb_index
)
{
  return ((dpp_mcds_base_t*)mcds)->mcdb + mcdb_index;
}


/* function to compare replications, for reverse sorting usage */
int dpp_rep_data_t_compare(void *a, void *b)
{ /* assumes sizeof(int) >= sizeof(uint32) which is already assumed in the driver */
  const dpp_rep_data_t *ca = b;
  const dpp_rep_data_t *cb = a;
  int32 res = ca->extra - cb->extra;
  return res ? res : (int32)(ca->base - cb->base);
}

/* functions to add a replication of a specific type to the group stored in the mcds */
void dpp_add_ingress_replication(
  dpp_mcds_base_t *mcds,
  const uint32     cud,
  const uint32     dest
)
{
  dpp_rep_data_t *rep = mcds->reps + mcds->nof_reps[0];
  DPP_MC_ASSERT(mcds->nof_ingr_reps == mcds->nof_reps[0] && mcds->nof_reps[0] < DPP_MULT_MAX_INGRESS_REPLICATIONS &&
    (mcds->nof_egr_port_outlif_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) == 0);
  rep->extra = rep->base = 0;
  DPP_MCDS_REP_DATA_SET_TYPE(rep, DPP_MCDS_REP_TYPE_INGRESS);
  DPP_MCDS_REP_DATA_SET_INGR_CUD(rep, cud);
  DPP_MCDS_REP_DATA_SET_INGR_DEST(rep, dest);
  mcds->nof_ingr_reps = ++mcds->nof_reps[0];
}

/* compare two replications, and return 0 if they are exactly the same, non zero otherwise */
STATIC INLINE uint32 compare_dpp_rep_data_t(const dpp_rep_data_t *rep1, const dpp_rep_data_t *rep2)
{
  return (rep1->base - rep2->base) | (rep1->extra - rep2->extra);
}

void dpp_add_egress_replication_port_cud(
  dpp_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2,
  const uint32     tm_port
)
{
    dpp_rep_data_t *rep = mcds->reps + core * DPP_MULT_MAX_REPLICATIONS + mcds->nof_reps[core];
    DPP_MC_ASSERT(mcds->nof_ingr_reps == 0 && mcds->nof_reps[core] < DPP_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] == mcds->nof_reps[core]);
    rep->extra = rep->base = 0;
    DPP_MCDS_REP_DATA_SET_TYPE(rep, DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
    DPP_MCDS_REP_DATA_SET_EGR_CUD(rep, cud);
    DPP_MCDS_REP_DATA_SET_EXTRA_CUD(rep, cud2);
    DPP_MCDS_REP_DATA_SET_EGR_PORT(rep, tm_port);
    ++mcds->nof_reps[core];
    ++mcds->nof_egr_port_outlif_reps[core];
}

void dpp_add_egress_replication_cud(
  dpp_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2
)
{
    dpp_rep_data_t *rep = mcds->reps + core * DPP_MULT_MAX_REPLICATIONS + mcds->nof_reps[core];
    DPP_MC_ASSERT(mcds->nof_ingr_reps == 0 && mcds->nof_reps[core] < DPP_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] == mcds->nof_reps[core]);
    rep->extra = rep->base = 0;
    DPP_MCDS_REP_DATA_SET_TYPE(rep, DPP_MCDS_REP_TYPE_EGR_CUD);
    DPP_MCDS_REP_DATA_SET_EGR_CUD(rep, cud);
    DPP_MCDS_REP_DATA_SET_EXTRA_CUD(rep, cud2);
    ++mcds->nof_reps[core];
    ++mcds->nof_egr_outlif_reps[core];
}

void dpp_add_egress_replication_bitmap(
  dpp_mcds_base_t *mcds,
  uint32           core,
  const uint32     cud,
  const uint32     cud2,
  const uint32     bm_id
)
{
    dpp_rep_data_t *rep;
    rep = mcds->reps + core * DPP_MULT_MAX_REPLICATIONS + mcds->nof_reps[core];
    rep->extra = rep->base = 0;
    DPP_MCDS_REP_DATA_SET_TYPE(rep, DPP_MCDS_REP_TYPE_EGR_BM_CUD);
    DPP_MCDS_REP_DATA_SET_EGR_CUD(rep, cud);
    DPP_MCDS_REP_DATA_SET_EXTRA_CUD(rep, cud2);
    DPP_MCDS_REP_DATA_SET_EGR_BM_ID(rep, bm_id);
    ++mcds->nof_reps[core];
    ++mcds->nof_egr_bitmap_reps[core];
    DPP_MC_ASSERT(mcds->nof_ingr_reps == 0 && mcds->nof_reps[core] < DPP_MULT_MAX_EGRESS_REPLICATIONS &&
      mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] == mcds->nof_reps[core]);
}



/*
 * clear the replications data in the mcds
 */
void dpp_mcds_clear_replications(dpp_mcds_base_t *mcds, const uint32 group_type)
{
    int i = 0;
    mcds->nof_ingr_reps = 0;
    mcds->info_2nd_cud = DPP_MC_GROUP_2ND_CUD_NONE;
    for (; i < SOC_DPP_DEFS_MAX(NOF_CORES); ++ i) {
        mcds->nof_egr_bitmap_reps[i] = mcds->nof_egr_outlif_reps[i] = mcds->nof_egr_port_outlif_reps[i] = mcds->nof_reps[i] = 0;
    }
}

/*
 * This functions copies the replication data from the mcds into the given gport and encap_id arrays.
 * It is an error if more entries are to be copied than available in the arrays.
 */
uint32 dpp_mcds_copy_replications_to_arrays(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint8        is_egress,           /* are the replications for an egress multicast group (opposed to ingress) */
    SOC_SAND_IN  uint32       arrays_size,         /* size of output arrays */
    SOC_SAND_OUT soc_gport_t  *port_array,         /* output array to contain logical ports/destinations, used if !reps */
    SOC_SAND_OUT soc_if_t     *encap_id_array,     /* output array to contain encapsulations/CUDs/outlifs, used if !reps */
    SOC_SAND_OUT soc_multicast_replication_t *reps /* output replication array (array of size mc_group_size*/
)
{
    dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
    uint16 size;
    dpp_rep_data_t *rep;
    int is_normal_egress = mcds->nof_ingr_reps == 0;
    int swap_cuds = mcds->info_2nd_cud == DPP_MC_GROUP_2ND_CUD_OUTRIF;
    soc_gport_t out_gport;
    soc_if_t out_cud, out_cud2;
    SOCDNX_INIT_FUNC_DEFS;

    /* The temporary size here must be zero for ingress and for egress TDM.
       For regular egress mcds->nof_ingr_reps must be 0 */
    DPP_MC_ASSERT((is_normal_egress || (mcds->nof_egr_port_outlif_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) == 0) &&
      (SOC_IS_JERICHO(unit) || !swap_cuds));

    /* this code depends on the implementation of the mcds storage, but is faster */
    if (is_egress) {
        int core;
        soc_port_t local_logical_port;
        uint32 total_size = 0;
        for (core = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores; core > 0;) {
          --core;
            size = mcds->nof_reps[core];
            total_size += size;
            if (total_size > arrays_size) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("too many egress replications to return")));
            }

            /* loop over the replications returning them in the appropriate arrays */
            for (rep = mcds->reps + core * DPP_MULT_MAX_REPLICATIONS; size; --size, ++rep) {
                out_cud = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep); /* set CUD/outlif (encapsulation) */
                switch (DPP_MCDS_REP_DATA_GET_TYPE(rep)) {
                  case DPP_MCDS_REP_TYPE_EGR_PORT_CUD:
                    if (SOC_DPP_CONFIG(unit)->tm.mc_mode & DPP_MC_EGR_17B_CUDS_127_PORTS_MODE) {
                        _SHR_GPORT_LOCAL_SET(out_gport, DPP_MCDS_REP_DATA_GET_EGR_PORT(rep) & ~(1 << 7)); /* set the local logical port */
                        out_cud |= ((DPP_MCDS_REP_DATA_GET_EGR_PORT(rep) & (soc_if_t)(1 << 7)) << 9); /* set outlif (encapsulation) */
                    } else {
                        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_tm_to_local_port_get(unit, core, DPP_MCDS_REP_DATA_GET_EGR_PORT(rep), &local_logical_port));
                        _SHR_GPORT_LOCAL_SET(out_gport, local_logical_port); /* set the local logical port */
                    }
                    break;
                  case DPP_MCDS_REP_TYPE_EGR_CUD:
                    out_gport = _SHR_GPORT_INVALID ; /* set port to invalid to make outlif only */
                    break;
                  case DPP_MCDS_REP_TYPE_EGR_BM_CUD:
                    _SHR_GPORT_MCAST_SET(out_gport, DPP_MCDS_REP_DATA_GET_EGR_BM_ID(rep) & DPP_MC_EGR_MAX_BITMAP_ID); /* set the gport representation of the bitmap id */
                    out_cud |= ((DPP_MCDS_REP_DATA_GET_EGR_BM_ID(rep) & (1 << 14)) << 2); /* handle Arad+ special CUD encoding */
                    break;
                  default:
                    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unexpected replication type")));
                }
                if (reps) {
                    if (swap_cuds) { /* If a 2nd CUD exists we switch it with the 1st CUD */
                        if ((out_cud2 = DPP_MCDS_REP_DATA_GET_EXTRA_CUD(rep)) == DPP_MC_NO_2ND_CUD) {
                            reps->flags = 0;
                            reps->encap1 = out_cud;
                            reps->encap2 = DPP_MC_NO_2ND_CUD;
                        } else { /* we have a 2nd CUD */
                            reps->flags = SOC_MUTICAST_REPLICATION_ENCAP2_VALID | SOC_MUTICAST_REPLICATION_ENCAP1_L3_INTF;
                            reps->encap1 = out_cud2;
                            reps->encap2 = out_cud;
                        }
                    } else { /* we do not need to switch CUDs */
                        reps->encap2 = DPP_MCDS_REP_DATA_GET_EXTRA_CUD(rep);
                        reps->encap1 = out_cud;
                        reps->flags = (reps->encap2 == DPP_MC_NO_2ND_CUD) ? 0 : SOC_MUTICAST_REPLICATION_ENCAP2_VALID;
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
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("too many ingress replications to return")));
        }
        for (rep = mcds->reps; size; --size, ++rep) {
            DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep) == DPP_MCDS_REP_TYPE_INGRESS);
            /* convert ingress hardware fields to API representation */
            SOCDNX_IF_ERR_EXIT(mcds->convert_ingress_replication_hw2api(unit, DPP_MCDS_REP_DATA_GET_INGR_CUD(rep),
              DPP_MCDS_REP_DATA_GET_INGR_DEST(rep), port_array, encap_id_array));
              ++port_array;
            ++encap_id_array;
        }

    } /* end of ingress handling */

exit:
  SOCDNX_FUNC_RETURN;
}


/* get the tm port and core from the gport  (local port) after it was processed by bcm - not in gport format */
STATIC soc_error_t _get_tm_port_and_core_from_gport(int unit, const uint32 port, uint32 *tm_port, uint32 *port_core)
{
    int core;
    SOCDNX_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO(unit)) {
        SOCDNX_IF_ERR_EXIT(soc_port_sw_db_local_to_tm_port_get(unit, port, tm_port, &core));
        if (*tm_port >= DPP_MULT_EGRESS_PORT_INVALID) { /* disabled or invalid port */
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("invalid port")));
        }
        DPP_MC_ASSERT(core >= 0 && core < SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores);
        *port_core = core;
    } else { /* special treatment is needed due to pre-Jericho (Arad+) special modes of port and CUD encoding to support bigger CUDs */
        *tm_port = port;
        *port_core = 0;
    }
exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * This functions copies the replication data from the given port and encap_id arrays into the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 * The function also converts each logical port to core + TM port in Jericho.
 */
uint32 dpp_mcds_copy_replications_from_arrays(
    SOC_SAND_IN  int       unit,
    SOC_SAND_IN  uint8     is_egress,       /* are the replications for an egress multicast group (opposed to ingress) */
    SOC_SAND_IN  uint8     do_clear,        /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    SOC_SAND_IN  uint32    arrays_size,     /* size of output arrays */
    SOC_SAND_IN  dpp_mc_replication_t *reps /* input array containing replications using logical ports */
)
{
    dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
    uint32 size_left = arrays_size;
    uint32 nof_active_cores = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores;
    SOCDNX_INIT_FUNC_DEFS;

    if (is_egress) {

        uint32 core = 0, port;
        uint32 cud, cud2;
        uint8 cud2_type;
        if (do_clear) {
            dpp_mcds_clear_replications(mcds, DPP_MCDS_TYPE_VALUE_EGRESS);
        }
        for (; size_left; --size_left) {
            port = reps->dest;
            cud = reps->cud;
            cud2 = (reps++)->additional_cud;
            cud2_type = DPP_MC_2ND_CUD_TYPE_REP2MCDS(cud2 & DPP_MC_2ND_CUD_IS_EEI);
            if (cud2 != DPP_MC_NO_2ND_CUD) { /* We have a 2nd CUD */
                if (mcds->info_2nd_cud == DPP_MC_GROUP_2ND_CUD_NONE) { /* This is the first replication with a 2nd CUD */
                    mcds->info_2nd_cud = cud2_type;
                } else if (cud2_type != mcds->info_2nd_cud) {
                    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("Can not mix different types of a 2nd CUD")));
                }
                cud2 &= ~DPP_MC_2ND_CUD_IS_EEI;
            }

            if (port == _SHR_GPORT_INVALID) { /* CUD only replication */
                if (nof_active_cores > 1) { /* add to the appropriate core */
                    DPP_CUD2CORE_GET_CORE(unit, cud, core);
                    if (core == DPP_CUD2CORE_UNDEF_VALUE) {
                        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("CUD 0x%x is not mapped to a port/core"), cud));
                    }
                }
                dpp_add_egress_replication_cud(mcds, core, cud, cud2);
            } else if (port & ARAD_MC_EGR_IS_BITMAP_BIT) { /* CUD+bitmap replication, added to all cores */
                for (core = SOC_DPP_CONFIG(mcds->unit)->core_mode.nof_active_cores; core > 0;) {
                    --core;
                    dpp_add_egress_replication_bitmap(mcds, core, cud, cud2, port & ~ARAD_MC_EGR_IS_BITMAP_BIT);
                }
            } else { /* CUD+port replication */
                SOCDNX_IF_ERR_EXIT(_get_tm_port_and_core_from_gport(unit, port, &port, &core));
                dpp_add_egress_replication_port_cud(mcds, core, cud, cud2, port);
            }
        }
        for (core = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores; core > 0;) { /* check for too many replications */
            --core;
            DPP_MC_ASSERT(mcds->nof_reps[core] == (int32)mcds->nof_egr_port_outlif_reps[core] + mcds->nof_egr_outlif_reps[core] + mcds->nof_egr_bitmap_reps[core] && mcds->nof_ingr_reps == 0);
            if (mcds->nof_reps[core] > DPP_MULT_MAX_EGRESS_REPLICATIONS) {
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("too many replications")));
            }
        }

    } else { /* ingress - not currently in use */

        if (do_clear) {
            dpp_mcds_clear_replications(mcds, DPP_MCDS_TYPE_VALUE_INGRESS);
        }
        DPP_MC_ASSERT(mcds->nof_reps[0] == mcds->nof_ingr_reps && (mcds->nof_egr_port_outlif_reps[0] | mcds->nof_egr_outlif_reps[0] | mcds->nof_egr_bitmap_reps[0]) == 0);
        if (mcds->nof_reps[0] + (int32)arrays_size > DPP_MULT_MAX_INGRESS_REPLICATIONS) {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("too many replications")));
        }

        for (; size_left; --size_left) {
            uint32 dest = reps->dest;
            soc_if_t cud = (reps++)->cud;
            if (dest >= DPP_MC_ING_DESTINATION_DISABLED) { /* wrong destination - disabled or out of range */
                SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("invalid destination")));
            }
            dpp_add_ingress_replication(mcds, cud, dest);
        }

    } /* end of ingress handling */

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * This functions copies the replication data from a consecutive egress entries block into the mcds (core 0).
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * All the block entries except for the last point implicitly to the next entry using formats 5,7.
 */
uint32
  dpp_mcds_copy_replications_from_egress_block(
    SOC_SAND_IN  int                           unit,
    SOC_SAND_IN  uint8                         do_clear,    /* if zero, replications will be added in addition to existing ones, otherwise previous replications will be cleared */
    SOC_SAND_IN  uint32                        block_start, /* index of the block start */
    SOC_SAND_IN  dpp_free_entries_block_size_t block_size,  /* number of entries in the block */
    SOC_SAND_INOUT uint32                      *cud2,       /* the current 2nd CUD of replications */
    SOC_SAND_OUT uint32                        *next_entry  /* the next entry pointed to by the last block entry */
)
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  uint32 cur_index = block_start;
  dpp_free_entries_block_size_t entries_remaining = block_size;
  uint16 nof_reps = 0, reps_left;
  SOCDNX_INIT_FUNC_DEFS;

  DPP_MC_ASSERT(block_start + block_size < MCDS_INGRESS_LINK_END(mcds));
  if (do_clear) {
    dpp_mcds_clear_replications(mcds, DPP_MCDS_TYPE_VALUE_EGRESS);
  } else {
    nof_reps = mcds->nof_reps[0];
  }
  DPP_MC_ASSERT(nof_reps == mcds->nof_egr_port_outlif_reps[0] + mcds->nof_egr_outlif_reps[0] +
    mcds->nof_egr_bitmap_reps[0] && mcds->nof_ingr_reps == 0 &&
    nof_reps < DPP_MULT_MAX_EGRESS_REPLICATIONS && mcds->nof_reps[0] == nof_reps);
  reps_left = DPP_MULT_MAX_EGRESS_REPLICATIONS - nof_reps;

  /* get replications from the rest of the entries */
  while (entries_remaining) {
    SOCDNX_IF_ERR_EXIT(mcds->get_replications_from_entry( /* add replications to mcds from cur_index entry */
      unit, 0, TRUE, cur_index, DPP_MCDS_TYPE_VALUE_EGRESS, cud2, &reps_left, &nof_reps, next_entry));
    ++cur_index;
    --entries_remaining;
    if (nof_reps > DPP_MULT_MAX_EGRESS_REPLICATIONS) {
      DPP_MC_ASSERT(0); /* group is somehow bigger than allowed - internal error */
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("too many replications")));
    } else if (entries_remaining && *next_entry == DPP_MC_EGRESS_LINK_PTR_END) {
      DPP_MC_ASSERT(0); /* entry does not point to next entry in the block - internal error */
      SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("block too small")));
    }
  }

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * This functions removes the given (single) replication (dest and cud) from the mcds.
 * It is an error if the mcds is filled beyond the maximum size of a multicast group.
 * We currently assume that the destination/port translation is done by bcm code before calling this function.
 */
uint32 dpp_mult_remove_replication(
    SOC_SAND_IN  int                  unit,
    SOC_SAND_IN  uint32               group_type, /* to what type of group does the replication belong */
    SOC_SAND_IN  uint32               dest,       /* local port/destination of the replication to remove */
    SOC_SAND_IN  soc_if_t             cud,        /* encapsulations/CUD/outlif of the replication to remove */
    SOC_SAND_IN  soc_if_t             cud2,       /* encapsulations/CUD/outlif of the second replication to remove */
    SOC_SAND_OUT SOC_TMC_ERROR        *out_err,   /* return possible errors that the caller may want to ignore: replication does not exist */
    SOC_SAND_OUT dpp_mc_core_bitmap_t *cores      /* return the linked list cores of the removed replication */
)
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  dpp_rep_data_t *rep = mcds->reps, rep_to_find = {0};
  uint16 size_left = 0, *specific_nof_reps;
  uint32 core = 0; /* number of core to work on, which may be >0 when multiple linked lists exist for the group */
  SOCDNX_INIT_FUNC_DEFS;
  *out_err = _SHR_E_NONE;

  /* this code depends on the implementation of the mcds storage, but is faster */
  if (DPP_MCDS_TYPE_IS_EGRESS(group_type)) { /* egress */

    if (dest == _SHR_GPORT_INVALID) { /*outlif only replication */
      if(SOC_IS_JERICHO(unit)) {
          DPP_CUD2CORE_GET_CORE(unit, cud, core);
          if (core >= SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores) {
              SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("CUD 0x%x to be removed is not mapped to a port/core"), cud));
          }
      }
      DPP_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DPP_MCDS_REP_TYPE_EGR_CUD);
      DPP_MCDS_REP_DATA_SET_EGR_CUD(&rep_to_find, cud);
      DPP_MCDS_REP_DATA_SET_EXTRA_CUD(&rep_to_find, cud2);
      specific_nof_reps = &mcds->nof_egr_outlif_reps[core];
    } else if (dest & ARAD_MC_EGR_IS_BITMAP_BIT) { /* bitmap + outlif replication */
      const uint32 bitmap = dest & ~ARAD_MC_EGR_IS_BITMAP_BIT;
      dpp_rep_data_t *found_reps_per_core[SOC_DPP_DEFS_MAX(NOF_CORES)] = {0};
      DPP_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DPP_MCDS_REP_TYPE_EGR_BM_CUD);
      DPP_MCDS_REP_DATA_SET_EGR_CUD(&rep_to_find, cud);
      DPP_MCDS_REP_DATA_SET_EXTRA_CUD(&rep_to_find, cud2);
      DPP_MCDS_REP_DATA_SET_EGR_BM_ID(&rep_to_find, bitmap);
      for (core = 0; core < SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores; ++core) {
        /* search for an entry in each core */
        rep = mcds->reps + DPP_MCDS_GET_REP_INDEX(core, 0);
        for (size_left = mcds->nof_reps[core]; size_left; --size_left, ++rep) {
          DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep) != DPP_MCDS_REP_TYPE_INGRESS);
          if (!compare_dpp_rep_data_t(rep, &rep_to_find)) {
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
        for (core = 0; core < SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores; ++core) {
          --mcds->nof_reps[core];
          --mcds->nof_egr_bitmap_reps[core];
          *found_reps_per_core[core] = mcds->reps[DPP_MCDS_GET_REP_INDEX(core, mcds->nof_reps[core])];
        }
      }
      if (cores != NULL) {
        *cores = DPP_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit);
      }
      SOC_EXIT;
    } else { /* port + outlif replication */
      uint32 tm_port;
      SOCDNX_IF_ERR_EXIT(_get_tm_port_and_core_from_gport(unit, dest, &tm_port, &core));
      DPP_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      DPP_MCDS_REP_DATA_SET_EGR_CUD(&rep_to_find, cud);
      DPP_MCDS_REP_DATA_SET_EXTRA_CUD(&rep_to_find, cud2);
      DPP_MCDS_REP_DATA_SET_EGR_PORT(&rep_to_find, tm_port);
      specific_nof_reps = &mcds->nof_egr_port_outlif_reps[core];
    }

    rep = mcds->reps + DPP_MCDS_GET_REP_INDEX(core, 0);
    for (size_left = mcds->nof_reps[core]; size_left; --size_left, ++rep) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep) != DPP_MCDS_REP_TYPE_INGRESS);
      if (!compare_dpp_rep_data_t(rep, &rep_to_find)) {
        break;
      }
    }

  } else if (DPP_MCDS_TYPE_IS_INGRESS(group_type)) { /* ingress */

    DPP_MCDS_REP_DATA_SET_TYPE(&rep_to_find, DPP_MCDS_REP_TYPE_INGRESS);
    DPP_MCDS_REP_DATA_SET_INGR_CUD(&rep_to_find, cud);
    DPP_MCDS_REP_DATA_SET_INGR_DEST(&rep_to_find, dest);
    specific_nof_reps = &mcds->nof_ingr_reps;
    for (size_left = mcds->nof_reps[0]; size_left; --size_left, ++rep) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep) == DPP_MCDS_REP_TYPE_INGRESS);
      if (!compare_dpp_rep_data_t(rep, &rep_to_find)) {
        break;
      }
    }

  } else { /* type is: not used */
    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unexpected entry type")));
  }

  if (!size_left) {
    *out_err = _SHR_E_NOT_FOUND;
  } else { /* remove the found entry */
    --mcds->nof_reps[core];
    --*specific_nof_reps;
    *rep = mcds->reps[DPP_MCDS_GET_REP_INDEX(core, mcds->nof_reps[core])];
  }
  if (cores != NULL) {
    *cores = DPP_MC_CORE_BITAMAP_CORE_0 << core;
  }
exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Get, set, increase and decrease the stored number of free entries
 */
uint32 dpp_mcds_unoccupied_get(
    SOC_SAND_IN dpp_mcds_base_t *mcds
)
{
  return mcds->nof_unoccupied;
}

void
  dpp_mcds_unoccupied_increase(
    SOC_SAND_INOUT dpp_mcds_base_t *mcds,
    SOC_SAND_IN    uint32               delta
)
{
  mcds->nof_unoccupied += delta;
  DPP_MC_ASSERT(mcds->nof_unoccupied < DPP_LAST_MCDB_ENTRY(mcds));
}

void
  dpp_mcds_unoccupied_decrease(
    SOC_SAND_INOUT dpp_mcds_base_t *mcds,
    SOC_SAND_IN    uint32               delta
)
{
  DPP_MC_ASSERT(mcds->nof_unoccupied >= delta);
  mcds->nof_unoccupied -= delta;
}

/*
 * Get the region corresponding to the given mcdb index
 */
dpp_free_entries_blocks_region_t* dpp_mcds_get_region(dpp_mcds_base_t *mcds, uint32 mcdb_index)
{
  DPP_MC_ASSERT(mcds && mcdb_index > 0 && mcdb_index < DPP_LAST_MCDB_ENTRY(mcds));

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
dpp_free_entries_blocks_region_t*
  dpp_mcds_get_region_and_consec_range(dpp_mcds_base_t *mcds, uint32 mcdb_index, uint32 *range_start, uint32 *range_end)
{
  dpp_free_entries_blocks_region_t *range = dpp_mcds_get_region(mcds, mcdb_index);
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
  DPP_MC_ASSERT(*range_start <= *range_end && mcdb_index >= *range_start && mcdb_index <= *range_end);
  return range;
}

/*
 * Copy the src_index entry to the dst_index entry, and write the dst_index entry to hardware.
 * Both the hardware and mcds is copied. So be bery careful if using this to copy a free entry.
 */
uint32 dpp_mcdb_copy_write(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 src_index, /* index of entry to be copied */
    SOC_SAND_IN uint32 dst_index  /* index of entry to be copied to and written to disk */
)
{
  uint32 data[DPP_MC_ENTRY_SIZE];
  dpp_mcds_base_t* mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *src_entry = MCDS_GET_MCDB_ENTRY(mcds, src_index);
  arad_mcdb_entry_t *dst_entry = MCDS_GET_MCDB_ENTRY(mcds, dst_index);
  SOCDNX_INIT_FUNC_DEFS;

  data[0] = dst_entry->word0 = src_entry->word0;
  dst_entry->word1 &= DPP_MCDS_WORD1_KEEP_BITS_MASK;
  dst_entry->word1 |= src_entry->word1 & ~DPP_MCDS_WORD1_KEEP_BITS_MASK;
  data[1] = src_entry->word1 & mcds->msb_word_mask;
  SOCDNX_IF_ERR_EXIT(WRITE_IRR_MCDBm(unit, MEM_BLOCK_ANY, dst_index, data));
  mcds->prev_entries[dst_index] = mcds->prev_entries[src_index];

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * Init a free entries block list to be empty.
 * This does not look at its existing contents as it is assumed to be uninitialized.
 */
STATIC INLINE void
  dpp_mcds_init_free_entries_block_list(dpp_free_entries_block_list_t *list)
{
  list->first = DPP_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY;
}
/*
 * Remove the given free entries block from the given block list.
 * Does not modify the block itself.
 */
void _dpp_mcds_remove_free_entries_block_from_list(dpp_mcds_base_t *mcds, dpp_free_entries_block_list_t *list, uint32 block, const dpp_free_entries_block_size_t block_size)
{
  uint32 next, prev;

  DPP_MC_ASSERT(block < DPP_LAST_MCDB_ENTRY(mcds) && block > 0);
  DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, block) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);
  DPP_MC_ASSERT(block_size > 0 && block_size == DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, block));
  DPP_MC_ASSERT(list == dpp_mcds_get_region(mcds, block)->lists + (block_size-1));
  next = DPP_MCDS_GET_FREE_NEXT_ENTRY(mcds, block);
  prev = DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, block);
  if (next == block) { /* this was the only entry in the list */
    DPP_MC_ASSERT(prev == block && list->first == block);
    list->first = DPP_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY; /* make list as empty */
  } else { /* the list has more entries */
    DPP_MC_ASSERT(prev != block && 
      DPP_MCDS_GET_TYPE(mcds, prev) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
      DPP_MCDS_GET_TYPE(mcds, next) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    DPP_MC_ASSERT(DPP_MCDS_GET_FREE_NEXT_ENTRY(mcds, prev) == block &&
                DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, next) == block);
    DPP_MCDS_SET_FREE_NEXT_ENTRY(mcds, prev, next);
    DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, next, prev);
    if (list->first == block) {
      list->first = next; /* If this was the list start, advance list start to the next block */
    }
  }
  LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
              (BSL_META("removed(%u,%u) "), block, block_size));
  dpp_mcds_unoccupied_decrease(mcds, block_size); /* subtract the block size from the number of free entries */
}

/*
 * Remove the given free entries block from the given block list.
 * Does not modify the block itself.
 */
STATIC INLINE void
  dpp_mcds_remove_free_entries_block_from_list(dpp_mcds_base_t *mcds, dpp_free_entries_block_list_t *list, uint32 block)
{
  _dpp_mcds_remove_free_entries_block_from_list(mcds, list, block, DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, block));
}

/*
 * Remove the given free entries block from the given region's block list.
 * Does not modify the block itself.
 */
STATIC INLINE void
  dpp_mcds_remove_free_entries_block_from_region(dpp_mcds_base_t *mcds, dpp_free_entries_blocks_region_t *region, uint32 block, dpp_free_entries_block_size_t block_size)
{
  dpp_free_entries_block_list_t *list = region->lists + (block_size-1);

  DPP_MC_ASSERT(block_size <= region->max_size);
  _dpp_mcds_remove_free_entries_block_from_list(mcds, list, block, block_size);
}

/*
 * Check if the given free entries block list is empty.
 * return non zero if empty.
 */
STATIC INLINE int
  dpp_mcds_is_empty_free_entries_block_list(const dpp_mcds_base_t *mcds, const dpp_free_entries_block_list_t *list)
{
  if (list->first == DPP_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY)
    return 1;
  DPP_MC_ASSERT(list->first < DPP_LAST_MCDB_ENTRY(mcds));
  return 0;
}

/*
 * Add the given free entries block to the given block list.
 * Does not modify the block itself.
 */
void
  dpp_mcds_add_free_entries_block_to_list(dpp_mcds_base_t *mcds, dpp_free_entries_block_list_t *list, uint32 block)
{
  uint32 next, prev;
  dpp_free_entries_block_size_t block_size = DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, block);

  if (dpp_mcds_is_empty_free_entries_block_list(mcds, list)) {
    list->first = prev = next = block;
  } else {
    next = list->first;
    prev = DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, next);
    DPP_MC_ASSERT(DPP_MCDS_GET_FREE_NEXT_ENTRY(mcds, prev) == next &&
      DPP_MCDS_GET_TYPE(mcds, next) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, next, block);
    DPP_MCDS_SET_FREE_NEXT_ENTRY(mcds, prev, block);
  }
  DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, block, prev);
  DPP_MCDS_SET_FREE_NEXT_ENTRY(mcds, block, next);
  dpp_mcds_unoccupied_increase(mcds, block_size);
} 

/*
 * Return the first entry of the given free entries block list, or 0 if it is empty.
 * If to_remove is non zero, the found block will be removed from the list (and not otherwise changed).
 */
uint32 dpp_mcds_get_free_entries_block_from_list(dpp_mcds_base_t *mcds, dpp_free_entries_block_list_t *list, int to_remove)
{
  uint32 block = list->first;
  if (block == DPP_MC_FREE_ENTRIES_BLOCK_LIST_EMPTY)
    return 0;
  DPP_MC_ASSERT(block < DPP_LAST_MCDB_ENTRY(mcds));
  DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, block) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);

  if (to_remove) {
    dpp_mcds_remove_free_entries_block_from_list(mcds, list, block);
  }
  return block;
}

/*
 * Init a region with the given maximum block size. All lists will be marked as empty.
 */
void dpp_mcds_init_region(dpp_free_entries_blocks_region_t *region, dpp_free_entries_block_size_t max_size, uint32 range_start, uint32 range_end)
{
  unsigned i;
  dpp_free_entries_block_list_t *list_p = region->lists;
  region->max_size = max_size;
  region->range_start = range_start;
  region->range_end = range_end;
  for (i = 0; i < max_size; ++i) {
    dpp_mcds_init_free_entries_block_list(list_p);
    ++list_p;
  }
}


/*
 * Create a free block of a given size at the given entry.
 * add it to a free entries block list at the given region.
 * The block entries are assumed to be (marked as) free.
 * The created block or part of it may be merge with adjacent free blocks based on flags.
 */
#define DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV  1 /* Will not merge with the previous consecutive free block */
#define DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT  2 /* Will not merge with the next consecutive free block */
#define DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE          3 /* Will not merge with consecutive free blocks */
STATIC uint32 dpp_mcds_create_free_entries_block(
    SOC_SAND_INOUT dpp_mcds_base_t                 *mcds,
    SOC_SAND_IN    uint32                           flags,             /* DPP_MCDS_GET_FREE_BLOCKS_* flags that affect merging with adjacent free blocks */
    SOC_SAND_IN    uint32                           block_start_index, /* start index of the free block */
    SOC_SAND_IN    dpp_free_entries_block_size_t    block_size,        /* number of entries in the block */
    SOC_SAND_INOUT dpp_free_entries_blocks_region_t *region            /* region to contain the block in its lists */
)
{
  int unit = mcds->unit;
  uint32 i, current_block_start_index = block_start_index;
  uint32 block_end = block_start_index + block_size; /* the index of the entry immediately after the block */
  dpp_free_entries_block_size_t current_block_size = block_size, joint_block_size;

  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(region);
  if (block_start_index + block_size > MCDS_INGRESS_LINK_END(mcds) || !block_start_index) {
    DPP_MC_ASSERT(0);
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("block out of range")));
  }
  if (block_size > region->max_size || block_size < 1) {
    DPP_MC_ASSERT(0);
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("invalid block size")));
  }

  /* check the block's entries */
  for (i = block_start_index; i < block_end; ++i) {
    if (DPP_MCDS_TYPE_IS_USED(DPP_MCDS_GET_TYPE(mcds, i))) {
      DPP_MC_ASSERT(0); /* the entries of the block must be free */
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("attempted to add a used entry number %u to a free block"), i));
    }
  }

  /* if block is not of max size, attempt to merge with adjacent blocks */
  if (block_size < region->max_size) {
    const uint32 next_block = block_start_index + block_size;
    uint32 prev_block = block_start_index - 1;
    dpp_free_entries_block_size_t prev_block_size = 0, next_block_size = 0;
    
    if (!(flags & DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV) && /* get information on the previous adjacent block */
        prev_block >= region->range_start && prev_block <= region->range_end &&
        region == dpp_mcds_get_region(mcds, prev_block) &&
        DPP_MCDS_TYPE_IS_FREE(i = DPP_MCDS_GET_TYPE(mcds, prev_block))) {
      prev_block_size = 1;
      if (i != DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START) { /* block size > 1 */
        prev_block = DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, prev_block);
        prev_block_size = block_start_index - prev_block;
        DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, prev_block) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
          prev_block < block_start_index - 1 && prev_block_size <= region->max_size);
      }
      DPP_MC_ASSERT(prev_block_size == DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, prev_block));
      if (prev_block_size == region->max_size) {
        prev_block_size = 0; /* do not merge with max size blocks */
      }
    }
 
    if (!(flags & DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT) && /* get information on the next adjacent block */
        next_block >= region->range_start && next_block <= region->range_end &&
        region == dpp_mcds_get_region(mcds, next_block) &&
        DPP_MCDS_GET_TYPE(mcds, next_block) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
      next_block_size = DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, next_block);
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
      DPP_MC_ASSERT(joint_block_size + current_block_size == prev_block_size + block_size &&
        prev_block + joint_block_size == block_start_index + block_size - current_block_size);
      dpp_mcds_remove_free_entries_block_from_region(mcds, region, prev_block, prev_block_size); /* remove the previous block from free blocks list */

      LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                  (BSL_META_U(unit,
                              "merge with prev free block: prev:%u,%u  freed:%u,%u\n"), prev_block, prev_block_size, block_start_index, block_size));
      DPP_MCDS_SET_FREE_BLOCK_SIZE(mcds, prev_block, joint_block_size); /* mark the new previous block size */
      /* mark the type of the block entries added to the previous block */
      for (i = block_start_index; i < current_block_start_index; ++i) {
        DPP_MCDS_SET_TYPE(mcds, i, DPP_MCDS_TYPE_VALUE_FREE_BLOCK);
        DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, i, prev_block);
      }
      dpp_mcds_add_free_entries_block_to_list(mcds, region->lists + (joint_block_size-1), prev_block); /* add the previous block to different free blocks list */

    } else if (next_block_size) { /* merge with the next block */

      joint_block_size = next_block_size + block_size;
      if (joint_block_size > region->max_size) {
        current_block_size = joint_block_size - region->max_size;
        joint_block_size = region->max_size;
      } else {
        current_block_size = 0;
      }
      current_block_start_index += joint_block_size;
      DPP_MC_ASSERT(joint_block_size + current_block_size == next_block_size + block_size &&
        block_start_index + joint_block_size == next_block + next_block_size - current_block_size);

      LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                  (BSL_META_U(unit,
                              "merge with next free block: next:%u,%u  freed:%u,%u\n"), next_block, next_block_size, block_start_index, block_size));
      dpp_mcds_remove_free_entries_block_from_region(mcds, region, next_block, next_block_size); /* remove the next block from free blocks list */
      /* mark the type of the block entries */
      DPP_MCDS_SET_FREE_BLOCK_SIZE(mcds, block_start_index, joint_block_size); /* set the block size, to be called for the first block entry */ \
      DPP_MCDS_SET_TYPE(mcds, block_start_index, DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);
      for (i = block_start_index + 1; i < current_block_start_index; ++i) {
        DPP_MCDS_SET_TYPE(mcds, i, DPP_MCDS_TYPE_VALUE_FREE_BLOCK);
        DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, i, block_start_index);
      }
      dpp_mcds_add_free_entries_block_to_list(mcds, region->lists + (joint_block_size-1), block_start_index); /* add the previous block to different free blocks list */
 
    }

  }

  if (current_block_size) {
    /* mark the block's size */
    DPP_MCDS_SET_FREE_BLOCK_SIZE(mcds, current_block_start_index, current_block_size); /* set the block size, to be called for the first block entry */ \
    LOG_VERBOSE(BSL_LS_SOC_MULTICAST,
                (BSL_META_U(unit,
                            "added free block: %u,%u\n"), current_block_start_index, current_block_size));
    /* mark the type of the block entries */
    DPP_MCDS_SET_TYPE(mcds, current_block_start_index, DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);
    block_end = current_block_start_index + current_block_size;
    for (i = current_block_start_index + 1; i < block_end; ++i) {
      DPP_MCDS_SET_TYPE(mcds, i, DPP_MCDS_TYPE_VALUE_FREE_BLOCK);
      DPP_MCDS_SET_FREE_PREV_ENTRY(mcds, i, current_block_start_index);
    }

    /* add the block to the appropriate list of free blocks */
    dpp_mcds_add_free_entries_block_to_list(mcds, region->lists + (current_block_size-1), current_block_start_index);
  }

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Add free entries in the given range as blocks to the lists of the given region.
 * If (check_free) add only blocks marked as free.
 * Otherwise add all entries in the range are expected to be marked used and they will be marked as free.
 */
uint32 dpp_mcds_build_free_blocks(
    SOC_SAND_IN    int                              unit,   /* used only if check_free is zero */
    SOC_SAND_INOUT dpp_mcds_base_t                  *mcds,
    SOC_SAND_IN    uint32                           start_index, /* start index of the range to work on */
    SOC_SAND_IN    uint32                           end_index,   /* end index of the range to work on, if smaller than start_index then do nothing */
    SOC_SAND_INOUT dpp_free_entries_blocks_region_t *region,     /* region to contain the block in its lists */
    SOC_SAND_IN    mcds_free_build_option_t         entry_option /* which option to use in selecting entries to add and verifying them */
)
{
  dpp_free_entries_block_size_t max_size, block_size;
  uint32 block_start, cur_entry;
  int check_free = entry_option == McdsFreeBuildBlocksAddOnlyFree;

  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(region);
  DPP_MC_ASSERT(check_free || mcds == dpp_get_mcds(unit));
  if (start_index > end_index) {
    SOC_EXIT;
  }
  if (end_index >= MCDS_INGRESS_LINK_END(mcds) || !start_index) { /* index out of allowed range */
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("block out of range")));
  }

  max_size = region->max_size;

  for (block_start = start_index; block_start <= end_index; block_start += block_size) { /* loop over the index range */
    if (check_free) {
      block_size = 0;
      for (; block_start <= end_index && DPP_MCDS_TYPE_IS_USED(DPP_MCDS_GET_TYPE(mcds, block_start));
           ++block_start) {} /* find the next free entry */
      if (block_start <= end_index) { /* found a block start */
        block_size = 1;
        for (cur_entry = block_start + 1; block_size < max_size && cur_entry <= end_index && /* get the current free entries block */
             DPP_MCDS_TYPE_IS_FREE(DPP_MCDS_GET_TYPE(mcds, cur_entry)); ++cur_entry ) {
          ++block_size;
        }
      }
    } else { /* add all entries */
      block_size = block_start + max_size <= end_index ? max_size : end_index - block_start + 1;
    }
    if (block_size) { /* found a free entries block (at least one entry) */
      DPP_MC_ASSERT(block_size <= max_size);
      if (!check_free) { /* mark the block entries as free if working in the mode in which they are used */
        dpp_free_entries_block_size_t i;
        for (i = 0; i < block_size; ++i) {
          DPP_MC_ASSERT(entry_option != McdsFreeBuildBlocksAdd_AllMustBeUsed ||
            DPP_MCDS_TYPE_IS_USED(DPP_MCDS_GET_TYPE(mcds, block_start + i)));
          DPP_MCDS_SET_TYPE(mcds, block_start + i, DPP_MCDS_TYPE_VALUE_FREE_BLOCK);
          /* write the entry as free in hardware, can be optimized by DMA */
          SOCDNX_IF_ERR_EXIT(WRITE_IRR_MCDBm(unit, MEM_BLOCK_ANY, block_start + i, ((dpp_mcds_base_t*)mcds)->free_value));
        }
      }
      SOCDNX_IF_ERR_EXIT( /* add the found block to the region */
        dpp_mcds_create_free_entries_block(mcds, 0, block_start, block_size, region));
    }
  }

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Initialize the MultiCast Data Structures.
 * Do not fill the data from hardware yet.
 * dpp_mcds_multicast_init2() will be called to do so when we can access the MCDB using DMA.
 */
uint32
  dpp_mcds_multicast_init(
    SOC_SAND_IN int      unit
)
{
  uint32 start, end;
  dpp_mcds_base_t *mcds = NULL;
  int failed = 1;
  uint32 table_size;
  uint32 nof_active_cores = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores;
  uint8 is_allocated;
  soc_error_t rv;

  SOCDNX_INIT_FUNC_DEFS;
  if (!SOC_DPP_CONFIG(unit)->tm.nof_mc_ids) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("MCDS not initialized")));
  }
  SOCDNX_IF_ERR_EXIT(dpp_init_mcds(unit)); /* allocate and init mcds */
  mcds = dpp_get_mcds(unit);
  table_size = DPP_LAST_MCDB_ENTRY(mcds) + 1;

  /* init the members of dpp_mcds_base_t */
  mcds->nof_unoccupied = 0;
  mcds->mcdb = NULL;
  mcds->prev_entries = NULL;

  start = SOC_DPP_CONFIG(unit)->tm.ingress_mc_id_alloc_range_start;
  end   = SOC_DPP_CONFIG(unit)->tm.ingress_mc_id_alloc_range_end;
  if (start < 1) {
    start = 1;
  }
  if (end >= SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids) {
    end = SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids - 1;
  }
  if (end + 1 <= start)  { /* if the region should be empty */
    start = DPP_LAST_MCDB_ENTRY(mcds) + 1;
    end = DPP_LAST_MCDB_ENTRY(mcds);
  }
  dpp_mcds_init_region(&mcds->ingress_alloc_free, DPP_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED, start, end);
  dpp_mcds_init_region(&mcds->free_general, DPP_MCDS_MAX_FREE_BLOCK_SIZE_GENERAL, 1, DPP_LAST_MCDB_ENTRY(mcds) - 1); /* all of the MCDB entries except for the first and the last ones can be used as free entries */
  if (SOC_IS_ARADPLUS_AND_BELOW(unit)) { /* init mcds values for Arad* */
    start = ARAD_MULT_NOF_MULTICAST_GROUPS + SOC_DPP_CONFIG(unit)->tm.egress_mc_id_alloc_range_start;
    end   = ARAD_MULT_NOF_MULTICAST_GROUPS + SOC_DPP_CONFIG(unit)->tm.egress_mc_id_alloc_range_end;
    if (end >= DPP_LAST_MCDB_ENTRY(mcds)) {
      end = DPP_LAST_MCDB_ENTRY(mcds) - 1;
    }
  } else {
    start = DPP_MCDS_GET_EGRESS_GROUP_START(mcds, SOC_DPP_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1, 0);
    end = DPP_MCDS_GET_EGRESS_GROUP_START(mcds, SOC_DPP_CONFIG(unit)->tm.nof_mc_ids - 1, nof_active_cores - 1);
    DPP_MC_ASSERT(start == SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids &&
      end == SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids + nof_active_cores * mcds->nof_egr_ll_groups - 1 && end < DPP_LAST_MCDB_ENTRY(mcds));
  }
  dpp_mcds_init_region(&mcds->egress_alloc_free, DPP_MCDS_MAX_FREE_BLOCK_SIZE_ALLOCED, start, end);

  assert(sizeof(arad_mcdb_entry_t) == sizeof(uint32)*DPP_MC_ENTRY_SIZE &&
         sizeof(int) >= sizeof(int32)); /* The mcds will not work if for some reason this is not true */

  { /* allocate memory for the arrays */
    SOCDNX_IF_ERR_EXIT(dcmn_alloc_mem(unit, &mcds->mcdb, sizeof(arad_mcdb_entry_t) * table_size, "mcds-mc-mcdb"));
    SOCDNX_IF_ERR_EXIT(dcmn_alloc_mem(unit, &mcds->prev_entries, sizeof(uint16) * table_size, "mcds-mc-prev_entries"));
    if(!SOC_WARM_BOOT(unit)) {
        rv = sw_state_access[unit].dpp.soc.arad.tm.arad_multicast.is_allocated(unit, &is_allocated);
        SOCDNX_IF_ERR_EXIT(rv);
        if(!is_allocated) {
            rv = sw_state_access[unit].dpp.soc.arad.tm.arad_multicast.alloc(unit);
            SOCDNX_IF_ERR_EXIT(rv);
        }
        rv = sw_state_access[unit].dpp.soc.arad.tm.arad_multicast.egress_groups_open_data.alloc_bitmap(unit, SOC_DPP_CONFIG(unit)->tm.nof_mc_ids);
        SOCDNX_IF_ERR_EXIT(rv);
        
        rv = sw_state_access[unit].dpp.soc.arad.tm.arad_multicast.cud2core.alloc_bitmap(unit, mcds->max_egr_cud_field *DPP_CUD2CORE_BITS_PER_CUD);
        SOCDNX_IF_ERR_EXIT(rv);
        /* init the allocated memory to no core */
        rv = sw_state_access[unit].dpp.soc.arad.tm.arad_multicast.cud2core.bit_range_set(unit,
            0,
            mcds->max_egr_cud_field *DPP_CUD2CORE_BITS_PER_CUD);
        SOCDNX_IF_ERR_EXIT(rv);
        
    }
  }
#ifdef _ARAD_MC_TEST_EXTRA_INITIALIZATION_0 /* already done by dcmn_alloc_mem */
  SOCDNX_IF_ERR_EXIT(soc_sand_os_memset(mcds->prev_entries, DPP_CUD2CORE_INITIAL_BTYE_VALUE, sizeof(uint16)*table_size));
#endif

 failed = 0;
exit:
  if (failed && mcds) {
    dpp_mcds_multicast_terminate(unit);
  }
  SOCDNX_FUNC_RETURN;
}

/*
 * Initialize the multicast part of the software database.
 * Must be run after dpp_mcds_multicast_init() was called successfully, and when DMA is up.
 * fills the multicast data from hardware.
 */
uint32
  dpp_mcds_multicast_init2(
    SOC_SAND_IN int      unit
)
{
  unsigned i;
  uint32 *alloced_mem = NULL; /* memory allocated for the duration of this function */
  uint32 *dest32;
  arad_mcdb_entry_t *dest;
  dpp_mcds_base_t* mcds = dpp_get_mcds(unit);
  uint32 table_size, irdb_table_nof_entries, r32;
  int do_not_read = !SOC_WARM_BOOT(unit);
  int use_dma = !do_not_read &&
#ifdef PLISIM
    !SAL_BOOT_PLISIM &&
#endif
      soc_mem_dmaable(unit, IRR_MCDBm, SOC_MEM_BLOCK_ANY(unit, IRR_MCDBm)); /* check if we can use DMA */
  int failed = 1;
  uint32 range_start, range_end, last_end;

  SOCDNX_INIT_FUNC_DEFS;
  if (!SOC_DPP_CONFIG(unit)->tm.nof_mc_ids) {
    SOC_EXIT;
  }
  if (!mcds || !mcds->mcdb || !mcds->prev_entries) { /* dpp_mcds_multicast_init() was not called successfully */
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("MCDS not initialized")));
  }
  table_size = DPP_LAST_MCDB_ENTRY(mcds) + 1;
  irdb_table_nof_entries = table_size / (2 * IRDB_TABLE_GROUPS_PER_ENTRY); /*number of entries in the IRDB table */

  if (!SOC_WARM_BOOT(unit)) {
    if (!SOC_DPP_CONFIG(unit)->arad->init.pp_enable) {
      SOCDNX_IF_ERR_EXIT(WRITE_EGQ_INVALID_OUTLIFr(unit, REG_PORT_ANY, DPP_MC_EGR_CUD_INVALID));
      SOCDNX_IF_ERR_EXIT(WRITE_EPNI_INVALID_OUTLIFr(unit, REG_PORT_ANY, DPP_MC_EGR_CUD_INVALID));
    }
    if (SOC_IS_JERICHO(unit)) { /* Jericho specific configuration */
      /* loop over active cores, setting egress MC MCDB offset for each core */
      for (i = 0; i < SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores; ++i) {
        uint32 offset = DPP_MCDS_GET_EGRESS_GROUP_START(mcds, 0, i);
        if (offset >= JER_MCDB_SIZE ) {
            offset += JER_MCDB_SIZE;
            DPP_MC_ASSERT(i == 0 && offset < JER_MCDB_SIZE);
        }
        SOCDNX_IF_ERR_EXIT(READ_EGQ_MULTICAST_OFFSET_ADDRESSr(unit, i, &r32));
        soc_reg_field_set(unit, EGQ_MULTICAST_OFFSET_ADDRESSr, &r32, MCDB_OFFSETf, offset);
        SOCDNX_IF_ERR_EXIT(WRITE_EGQ_MULTICAST_OFFSET_ADDRESSr(unit, i, r32));
      }
    }
  }

  dest = mcds->mcdb;

  /* fill mcdb from hardware, including if each entry is used or not, use (read) IRR_MCDBm and IDR_IRDBm */
  /* allocate buffers and read tables differently depending on if DMA is enabled */
  if (use_dma) { /* use DMA */
    arad_mcdb_entry_t *src;
    /* alloced_mem will first contain IRR_MCDBm and later contain IDR_IRDBm */
    alloced_mem = soc_cm_salloc(unit, 4 * (table_size * DPP_MC_ENTRY_SIZE), "dma-mc-buffer");
    if (alloced_mem == NULL) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_MEMORY, (_BSL_SOCDNX_MSG("Failed to allocate DMA buffer")));
    }
    SOCDNX_IF_ERR_EXIT(soc_mem_read_range(unit, IRR_MCDBm, MEM_BLOCK_ANY, 0, DPP_LAST_MCDB_ENTRY(mcds), alloced_mem));

    /* copy mcdb from dma buffer to to software database */
    src = (void*)alloced_mem;
    for (i = table_size ; i ; --i) {
      dest->word0 = src->word0;
      dest++->word1 = src++->word1 & mcds->msb_word_mask;
    }

  } else { /* do not use DMA, read tables entry by entry */
    SOCDNX_IF_ERR_EXIT(dcmn_alloc_mem(unit, &alloced_mem, sizeof(uint32) * irdb_table_nof_entries * IRDB_TABLE_ENTRY_WORDS, "mcds-irdb-tmp"));

    /* read mcdb entry by entry into software database. */

    if (do_not_read) { /* if not in warm boot, the MCDB and IRDB tables that we have just filled */
      for (i = DPP_LAST_MCDB_ENTRY(mcds) + 1 ; i ; --i) {
        dest->word0 = ((dpp_mcds_base_t*)mcds)->free_value[0];
        dest++->word1 = ((dpp_mcds_base_t*)mcds)->free_value[1];
      }
    } else {
      SOCDNX_IF_ERR_EXIT(soc_mem_read_range(unit, IRR_MCDBm, MEM_BLOCK_ANY, 0, DPP_LAST_MCDB_ENTRY(mcds), dest));
      for (i = table_size; i; --i) {
        dest++->word1 &= mcds->msb_word_mask;
      }
    }

  }
  if (!do_not_read) {
    SOCDNX_IF_ERR_EXIT(soc_mem_read_range(unit, IDR_IRDBm, MEM_BLOCK_ANY, 0, irdb_table_nof_entries - 1, alloced_mem));
  }

#ifdef BCM_WARM_BOOT_SUPPORT /* #if defined(MCAST_WARM_BOOT_UPDATE_ENABLED) && defined(BCM_WARM_BOOT_SUPPORT) ### */
  if (SOC_WARM_BOOT(unit)  /* if warm boot */
     ) {
    /* We now have the mcds initialized from IRR_MCDB, and all entries are marked as unused. We will now process ingress groups */
    uint32 mcid = 0, highest_bitmap_group = SOC_DPP_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high;
    unsigned j;
    uint16 group_entries;
    uint32 core_id, nof_active_cores = SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores;
    dest32 = alloced_mem;
    for (i = 0 ; i < irdb_table_nof_entries; ++i) {
      uint32 bits = *dest32;
      for (j = 0 ; j < IRDB_TABLE_GROUPS_PER_ENTRY; ++j) {
        if (bits & 1) { /* we found an open ingress multicast group and will traverse it */
          /* traverse group, needed for warm boot support, mark them with the correct type and update prev_entries */
          arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, mcid);
          uint32 cur_entry = soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, LINK_PTRf);
          uint32 prev_entry = mcid;

          if (mcid >= SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids || DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
            DPP_MC_ASSERT(0); /* MCID is out of range or the entry is already used */
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("found invalid hardware table values")));
          }
          DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, DPP_MCDS_TYPE_VALUE_INGRESS_START); /* mark the first group entry as the start of an ingress group */
          DPP_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, mcid, prev_entry);
          group_entries = 1;
          while (cur_entry != MCDS_INGRESS_LINK_END(mcds)) { /* mark the rest of the group as non first entries of an ingress group. */
            mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, cur_entry);
            if (DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START || ++group_entries > DPP_MULT_MAX_INGRESS_REPLICATIONS) {
              DPP_MC_ASSERT(0);
              SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("entry already used or too many group entries")));
            }
            DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, DPP_MCDS_TYPE_VALUE_INGRESS);
            DPP_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, cur_entry, prev_entry);
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
    for (mcid = 0; mcid < SOC_DPP_CONFIG(unit)->tm.nof_mc_ids; ++mcid) {
        uint8 bit_val;
        SOCDNX_IF_ERR_EXIT(
            sw_state_access[unit].dpp.soc.arad.tm.arad_multicast.egress_groups_open_data.bit_get(
                unit,
                mcid,
                &bit_val)
            );
        if (bit_val) { /* we found an open egress multicast group */
          if (mcid > highest_bitmap_group) {
            for (core_id = 0; core_id < nof_active_cores; ++core_id) {
              /* traverse linked list group, needed for warm boot support, mark them with the correct type and update prev_entries */
              uint32 prev_entry = DPP_MCDS_GET_EGRESS_GROUP_START(mcds, mcid, core_id);
              arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, prev_entry);
              uint32 cur_entry;
  
              if (DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
                DPP_MC_ASSERT(0);
                SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("entry already used")));
              }
              DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, DPP_MCDS_TYPE_VALUE_EGRESS_START); /* mark the first group entry as the start of an egress group */
              DPP_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, prev_entry, prev_entry);
              SOCDNX_IF_ERR_EXIT(MCDS_GET_NEXT_POINTER(mcds, unit, prev_entry, DPP_MCDS_TYPE_VALUE_EGRESS_START, &cur_entry)); /* Get the next entry */
              group_entries = 1;
              while (cur_entry != DPP_MC_EGRESS_LINK_PTR_END) { /* mark the rest of the group as non first entries of an egress group. */
                mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, cur_entry);
                if (DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry) != DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START || ++group_entries > DPP_MULT_MAX_INGRESS_REPLICATIONS) {
                  DPP_MC_ASSERT(0);
                  SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("entry already used or too many group entries")));
                }
                DPP_MCDS_ENTRY_SET_TYPE(mcdb_entry, DPP_MCDS_TYPE_VALUE_EGRESS);
                DPP_MCDS_ENTRY_SET_PREV_ENTRY(mcdb_entry, mcds, cur_entry, prev_entry);
                prev_entry = cur_entry;
                SOCDNX_IF_ERR_EXIT(MCDS_GET_NEXT_POINTER(mcds, unit, prev_entry, DPP_MCDS_TYPE_VALUE_EGRESS, &cur_entry)); /* Get the next entry */
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
              DPP_MC_ASSERT(0);
              SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("Illegal IRR_MCDB content")));
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
    if (range_start <= DPP_LAST_MCDB_ENTRY(mcds) && range_end >= range_start) { /* If we have a valid ingress range */
      DPP_MC_ASSERT(range_start >= 1);
      /* add free entries before the ingress allocation/linked list start range */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks(unit, mcds, 1, range_start - 1, &mcds->free_general, McdsFreeBuildBlocksAddOnlyFree));
      /* add free entries from the ingress allocation/linked list start range */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks(unit, mcds, range_start, range_end, &mcds->ingress_alloc_free, McdsFreeBuildBlocksAddOnlyFree));
      last_end = range_end;
    } else {
      last_end = 0; /* set the start of the next general free entries range */
    }
    /* add free entries from the first half of the table after the ingress allocation range, and from the second half before the egress allocation range */
    range_start = mcds->egress_alloc_free.range_start;
    range_end = mcds->egress_alloc_free.range_end;
    if (range_start <= DPP_LAST_MCDB_ENTRY(mcds) && range_end >= range_start) { /* If we have a valid egress range */
      DPP_MC_ASSERT(range_start > last_end);
      /* add free entries between the ingress and egress allocation/linked list start ranges */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks( unit, mcds, last_end + 1, range_start - 1, &mcds->free_general, McdsFreeBuildBlocksAddOnlyFree));
      /* add free entries from the egress allocation/linked list start range */
      if (range_end >= DPP_LAST_MCDB_ENTRY(mcds)) {
        range_end = DPP_LAST_MCDB_ENTRY(mcds) - 1;
      }
      /* add free entries from the egress allocation/linked list start range */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks(unit, mcds, range_start, range_end, &mcds->egress_alloc_free, McdsFreeBuildBlocksAddOnlyFree));
      last_end = range_end;
    }
    /* add the remaining entries as free */
    SOCDNX_IF_ERR_EXIT(dpp_mcds_build_free_blocks(unit, mcds, last_end + 1, DPP_LAST_MCDB_ENTRY(mcds) - 1, &mcds->free_general, McdsFreeBuildBlocksAddOnlyFree));
  }
  failed = 0;

exit:
  if (alloced_mem) {
    if (use_dma) {
      soc_cm_sfree(unit, alloced_mem);
    } else {
      dcmn_free_mem_if_not_null(unit, (void*)&alloced_mem);
    }
  }
  if (failed) {
    dpp_mcds_multicast_terminate(unit);
  }
  SOCDNX_FUNC_RETURN;
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
  dpp_mcds_split_free_entries_block(
    SOC_SAND_INOUT dpp_mcds_base_t             *mcds,    /* MC SWDB object */
    SOC_SAND_IN    uint32                          flags,       /* RAD_SWDB_MCDB_GET_FREE_BLOCKS_* flags that affect what the function does group */
    SOC_SAND_INOUT dpp_free_entries_blocks_region_t *region,     /* region containing the block */
    SOC_SAND_IN    dpp_free_entries_block_size_t    orig_size,   /* the size of block to be split (must be bigger than new_size) */
    SOC_SAND_IN    dpp_free_entries_block_size_t    new_size,    /* The new block size (of the sub block that will be returned) if one would have been returned, split it */
    SOC_SAND_INOUT uint32                          *block_start /* the start of the block, updated by the function */
)
{
  int unit = mcds->unit;
  uint32 i;
  const uint32 next_block = *block_start + orig_size;
  uint32 prev_block = *block_start - 1;
  const dpp_free_entries_block_size_t remaining_size = orig_size - new_size;

  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(mcds);
  SOCDNX_NULL_CHECK(block_start);
  if (orig_size > region->max_size || new_size < 1 || new_size >= orig_size) {
    DPP_MC_ASSERT(0);
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("illegal size parameters")));
  }

  if (!(flags & DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT) &&  /* attempt to merge to next block */
      next_block >= region->range_start && next_block <= region->range_end &&
      region == dpp_mcds_get_region(mcds, next_block) &&
      DPP_MCDS_GET_TYPE(mcds, next_block) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START) {
    dpp_free_entries_block_size_t merged_block_size = DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, next_block);
    dpp_free_entries_block_size_t joint_block_size = merged_block_size + remaining_size;
    if (joint_block_size <= region->max_size) { /* The merged block will not be too big, perform the merge with the next block */
      DPP_MC_ASSERT(next_block - remaining_size == new_size + *block_start);
      dpp_mcds_remove_free_entries_block_from_region(mcds, region, next_block, merged_block_size);
      SOCDNX_IF_ERR_EXIT(dpp_mcds_create_free_entries_block( /* add the merged block to the region */
        mcds, DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, next_block - remaining_size, joint_block_size, region));
      goto exit;
    }
  }
  
  if (!(flags & DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV) &&  /* attempt to merge to previous block */
             prev_block >= region->range_start && prev_block <= region->range_end &&
             region == dpp_mcds_get_region(mcds, prev_block) &&
             DPP_MCDS_TYPE_IS_FREE(i = DPP_MCDS_GET_TYPE(mcds, prev_block))) {
    dpp_free_entries_block_size_t merged_block_size = 1, joint_block_size;
    if (i != DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START) { /* block size > 1 */
      prev_block = DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, prev_block);
      merged_block_size = *block_start - prev_block;
      DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, prev_block) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START &&
        prev_block < *block_start - 1 && merged_block_size <= region->max_size);
    }
    DPP_MC_ASSERT(merged_block_size == DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, prev_block));
    joint_block_size = merged_block_size + remaining_size;
    if (joint_block_size <= region->max_size) { /* The merged block will not be too big, perform the merge with the previous block */
      DPP_MC_ASSERT(prev_block + joint_block_size == remaining_size + *block_start);
      dpp_mcds_remove_free_entries_block_from_region(mcds, region, prev_block, merged_block_size);
      SOCDNX_IF_ERR_EXIT(dpp_mcds_create_free_entries_block( /* add the merged block to the region */
        mcds, DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT, prev_block, joint_block_size, region));
      *block_start += remaining_size;
      goto exit;
    }
  }
  
  /* did not merge, add the remaining entries as a new block */
  SOCDNX_IF_ERR_EXIT(dpp_mcds_create_free_entries_block( /* add the merged block to the region */
    mcds, DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, *block_start + new_size, remaining_size, region));

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Get a free block of size 1 at a given location.
 * Used for getting the first entry of a multicast group.
 * Does not mark mcdb_index as used.
 */
uint32 dpp_mcds_reserve_group_start(
    SOC_SAND_INOUT dpp_mcds_base_t *mcds,
    SOC_SAND_IN    uint32           mcdb_index /* the mcdb indx to reserve */
)
{
  int unit = mcds->unit;
  uint32 entry_type;
  SOCDNX_INIT_FUNC_DEFS;
  DPP_MC_ASSERT(mcdb_index <= DPP_LAST_MCDB_ENTRY(mcds));

  entry_type = DPP_MCDS_GET_TYPE(mcds, mcdb_index);
  if (DPP_MCDS_TYPE_IS_USED(entry_type)) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("entry must be free")));
  }
  if (mcdb_index > 0 && mcdb_index < DPP_LAST_MCDB_ENTRY(mcds)) { /* entry needs allocation */
    dpp_free_entries_blocks_region_t* region = dpp_mcds_get_region(mcds, mcdb_index);
    const uint32 block_start = entry_type == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START ?
      mcdb_index : DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, mcdb_index);
    const dpp_free_entries_block_size_t block_size = DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, block_start);
    const uint32 block_end = block_start + block_size - 1;
    DPP_MC_ASSERT(block_start <= mcdb_index && block_start + region->max_size >= mcdb_index && block_size <= region->max_size);

    dpp_mcds_remove_free_entries_block_from_region(mcds, region, block_start, block_size); /* remove the existing free block from the free list */
    if (block_start < mcdb_index) { /* create free block for entries before mcdb_index */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_create_free_entries_block(
        mcds, DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_NEXT, block_start, mcdb_index - block_start, region));
    }
    if (block_end > mcdb_index) { /* create free block for entries after mcdb_index */
      SOCDNX_IF_ERR_EXIT(dpp_mcds_create_free_entries_block(
        mcds, DPP_MCDS_SPLIT_FREE_BLOCKS_DONT_MERGE_TO_PREV, mcdb_index + 1, block_end - mcdb_index, region));
    }
  }

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * Get a free entries block of a given size, according to flags that needs to be used to start a multicast group.
 * Returns the start index and the number of entries in the block.
 */
uint32 dpp_mcds_get_free_entries_block(
    SOC_SAND_INOUT dpp_mcds_base_t              *mcds,
    SOC_SAND_IN    uint32                        flags,        /* DPP_MCDS_GET_FREE_BLOCKS_* flags that affect what the function does group */
    SOC_SAND_IN    dpp_free_entries_block_size_t wanted_size,  /* needed size of the free block group */
    SOC_SAND_IN    dpp_free_entries_block_size_t max_size,     /* do not return blocks above this size, if one would have been returned, split it */
    SOC_SAND_OUT   uint32                        *block_start, /* the start of the relocation block */
    SOC_SAND_OUT   dpp_free_entries_block_size_t *block_size   /* the size of the returned block */
)
{
  int unit = mcds->unit;
  dpp_free_entries_blocks_region_t *regions[ARAD_MCDS_NOF_REGIONS];
  int do_change = !(flags & DPP_MCDS_GET_FREE_BLOCKS_NO_UPDATES);
  uint32 block = 0;
  int r, s, loop_start, loop_end;
  int size_loop1_start, size_loop1_increase;
  int size_loop2_start, size_loop2_increase;
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(mcds);
  SOCDNX_NULL_CHECK(block_start);
  SOCDNX_NULL_CHECK(block_size);
  if (wanted_size > DPP_MCDS_MAX_FREE_BLOCK_SIZE || wanted_size > max_size || 1 > wanted_size) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("illegal wanted size")));
  }

  regions[0] = &mcds->free_general;
  if (flags & DPP_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS) { /* select region order according to flags */
    regions[1] = &mcds->ingress_alloc_free;
    regions[2] = &mcds->egress_alloc_free;
  } else {
    regions[1] = &mcds->egress_alloc_free;
    regions[2] = &mcds->ingress_alloc_free;
  }

  if (flags & DPP_MCDS_GET_FREE_BLOCKS_PREFER_SMALL) { /* select (loop) free entries block size order according to flags */
    size_loop1_start = wanted_size; size_loop1_increase = -1;
    size_loop2_start = wanted_size + 1; size_loop2_increase = 1;
  } else {
    size_loop1_start = wanted_size; size_loop1_increase = 1;
    size_loop2_start = wanted_size - 1; size_loop2_increase = -1;
  }

  if (flags & DPP_MCDS_GET_FREE_BLOCKS_PREFER_SIZE) { /* Will prefer to return a block of a better size than in a better region */

    /* first loop over block sizes */
    loop_start = size_loop1_start;
    if (size_loop1_increase >= 0) { /* increasing loop */
      loop_end = DPP_MCDS_MAX_FREE_BLOCK_SIZE + 1;
    } else { /* decreasing loop */
      loop_end = 0;
    }
    for (s = loop_start; s != loop_end; s += size_loop1_increase) {
      for (r = 0; r < ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
        dpp_free_entries_blocks_region_t *region = regions[r];
        if (region->max_size >= s) { /* if the current block size is supported by the region */
          if ((block = dpp_mcds_get_free_entries_block_from_list(mcds, region->lists + (s - 1), do_change))) {
            goto found;
          }
        }
      }
    }
    /* second loop over block sizes */
    loop_start = size_loop2_start;
    if (size_loop2_increase >= 0) { /* increasing loop */
      loop_end = DPP_MCDS_MAX_FREE_BLOCK_SIZE + 1;
    } else { /* decreasing loop */
      loop_end = 0;
    }
    for (s = loop_start; s != loop_end; s += size_loop2_increase) {
      for (r = 0; r < ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
        dpp_free_entries_blocks_region_t *region = regions[r];
        if (region->max_size >= s) { /* if the current block size is supported by the region */
          if ((block = dpp_mcds_get_free_entries_block_from_list(mcds, region->lists + (s - 1), do_change))) {
            goto found;
          }
        }
      }
    }

  } else { /* Will prefer to return a block in a better region than a block of a better size. */

    for (r = 0; r < ARAD_MCDS_NOF_REGIONS; ++r) { /* loop over regions */
      dpp_free_entries_blocks_region_t *region = regions[r];
      /* coverity[pointer_outside_base_object:FALSE] */
      dpp_free_entries_block_list_t *lists = region->lists - 1;

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
       if ((block = dpp_mcds_get_free_entries_block_from_list(mcds, lists + s, do_change))) {
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
       if ((block = dpp_mcds_get_free_entries_block_from_list(mcds, lists + s, do_change))) {
         goto found;
       }
      }

    } /* end of regions loop */

  } /* end of preferred better region */

  DPP_MC_ASSERT(!mcds->nof_unoccupied);
  if (flags & DPP_MCDS_GET_FREE_BLOCKS_DONT_FAIL) {
    *block_start = 0;
    *block_size = 0;
    SOC_EXIT;
  }
  SOCDNX_EXIT_WITH_ERR(SOC_E_RESOURCE, (_BSL_SOCDNX_MSG("did not find any free block")));

found:
  if (do_change && s > max_size) {
    DPP_MC_ASSERT(s <= DPP_MCDS_MAX_FREE_BLOCK_SIZE);
    SOCDNX_IF_ERR_EXIT( /* get free entries */
      dpp_mcds_split_free_entries_block(mcds, flags, regions[r], s, max_size, &block));
    s = max_size;
  }

  *block_start = block;
  *block_size = s;
  DPP_MC_ASSERT(block && s);

exit:
  SOCDNX_FUNC_RETURN;
}
/* 
* Given a table index checks if that entry is of a consecutive format
*/
STATIC uint32 
    dpp_mcds_is_egress_format_consecutive_next(
        int                      unit,
        uint8                    format,
        uint8                    *is_consecutive
)
{
    SOCDNX_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO(unit)) {
        if (format == 3 || (format >= 12 && format <= 15)) {
            *is_consecutive = TRUE;
        } else if (format == 0 || format == 1 || format == 2 || format == 3 || (format >= 4 && format <= 7) || (format >= 8 && format <= 11)) {
            *is_consecutive = FALSE;
        } else {
            SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("wrong egress format")));
        }

    } else {
         *is_consecutive = ARAD_MCDS_IS_EGRESS_FORMAT_CONSECUTIVE_NEXT(format);
    }
exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * Given a table index that needs to be used to start a multicast group,
 * returns the start index and the number of entries that need to be relocated.
 * If the number of entries returned is 0, a relocation is not needed.
 */
uint32
  dpp_mcds_get_relocation_block(
    SOC_SAND_IN  dpp_mcds_base_t             *mcds,
    SOC_SAND_IN  uint32                       mcdb_index,              /* table index needed for the start of a group */
    SOC_SAND_OUT uint32                       *relocation_block_start, /* the start of the relocation block */
    SOC_SAND_OUT dpp_free_entries_block_size_t *relocation_block_size   /* the size of the relocation block, 0 if relocation is not needed */
)
{
  int unit = mcds->unit;
  uint32 group_entry_type, start = mcdb_index;
  dpp_free_entries_block_size_t size = 1;
  uint8 is_consecutive;

  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(mcds);
  SOCDNX_NULL_CHECK(relocation_block_size);

  group_entry_type = DPP_MCDS_GET_TYPE(mcds, mcdb_index);
  if (DPP_MCDS_TYPE_IS_USED(group_entry_type) &&  /* relocation is needed if the entry is used and is not the start of a group */
    !DPP_MCDS_TYPE_IS_START(group_entry_type)) {
    if (DPP_MCDS_TYPE_IS_EGRESS_NORMAL(group_entry_type)) { /* ingress & TDM groups do not have used consecutive blocks */

      uint32 entry, next_entry;
      DPP_MC_ASSERT(group_entry_type == DPP_MCDS_TYPE_VALUE_EGRESS);
      for (entry = mcdb_index; ; entry = next_entry) { /* look for consecutive entries before the given one */
        next_entry = DPP_MCDS_GET_PREV_ENTRY(mcds, entry);
        SOCDNX_IF_ERR_EXIT(dpp_mcds_is_egress_format_consecutive_next(unit,DPP_MCDS_GET_EGRESS_FORMAT(mcds, next_entry), &is_consecutive));
        if (next_entry + 1 != entry || /* previous entry is not consecutive, or it has a link pointer */
            !is_consecutive) {
          break;
        }
        DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, next_entry) == DPP_MCDS_TYPE_VALUE_EGRESS); /* must be a none TDM egress entry, and not the group start */
        ++size;
        DPP_MC_ASSERT(next_entry && size <= DPP_MCDS_MAX_FREE_BLOCK_SIZE);
      }
      start = entry;
      SOCDNX_IF_ERR_EXIT(dpp_mcds_is_egress_format_consecutive_next(unit,DPP_MCDS_GET_EGRESS_FORMAT(mcds, mcdb_index), &is_consecutive));
      for (entry = mcdb_index; is_consecutive;)
      { /* look for consecutive entries after the given one */
        ++size;
        ++entry;
        DPP_MC_ASSERT(entry <= DPP_LAST_MCDB_ENTRY(mcds) && size <= DPP_MCDS_MAX_FREE_BLOCK_SIZE);
        DPP_MC_ASSERT(DPP_MCDS_GET_TYPE(mcds, entry) == DPP_MCDS_TYPE_VALUE_EGRESS); /* must be a none TDM egress entry, and not the group start */
        SOCDNX_IF_ERR_EXIT(dpp_mcds_is_egress_format_consecutive_next(unit,DPP_MCDS_GET_EGRESS_FORMAT(mcds, entry), &is_consecutive));
      }
      DPP_MC_ASSERT(entry - start + 1 == size);

    }
  } else { /* no relocation is needed */
    DPP_MC_ASSERT(DPP_MCDS_TYPE_IS_FREE(group_entry_type) || (mcdb_index < SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids ? /* is ingress group */
      group_entry_type == DPP_MCDS_TYPE_VALUE_INGRESS_START : (DPP_MCDS_TYPE_IS_EGRESS_START(group_entry_type) &&
      mcdb_index < DPP_MCDS_GET_EGRESS_GROUP_START(mcds, 0, SOC_DPP_CONFIG(unit)->core_mode.nof_active_cores))));
  }

  *relocation_block_size = size;
  if (relocation_block_start) {
    *relocation_block_start = start;
  }

exit:
  SOCDNX_FUNC_RETURN;
}



/*
 * Write a MCDB entry to hardware from the mcds.
 * Using only this function for writes, and using it after mcds mcdb used
 * entries updates, ensures consistency between the mcds and the hardware.
 */

uint32
  dpp_mcds_write_entry(
    SOC_SAND_IN int unit,
    SOC_SAND_IN uint32 mcdb_index /* index of entry to write */
)
{
  uint32 data[DPP_MC_ENTRY_SIZE];
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *entry = MCDS_GET_MCDB_ENTRY(mcds, mcdb_index);

  SOCDNX_INIT_FUNC_DEFS;

  data[0] = entry->word0;
  data[1] = entry->word1 & mcds->msb_word_mask;
  SOCDNX_IF_ERR_EXIT(WRITE_IRR_MCDBm(unit, MEM_BLOCK_ANY, mcdb_index, data));

exit:
  SOCDNX_FUNC_RETURN;
}

int _dpp_mcds_test_free_entries(
    SOC_SAND_IN int unit
)
{
  uint32 nof_unoccupied = 0;
  dpp_mcds_base_t* mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *entry, *entry2;
  ARAD_MULT_ID mcid;


  /* init test bit to be 1 if the entry is free, and count free entries */
  for (mcid = 0; mcid <= DPP_LAST_MCDB_ENTRY(mcds); ++mcid) {
    entry = MCDS_GET_MCDB_ENTRY(mcds, mcid);
    if (DPP_MCDS_TYPE_IS_FREE(DPP_MCDS_ENTRY_GET_TYPE(entry))) {
      DPP_MCDS_ENTRY_SET_TEST_BIT_ON(entry);
      ++nof_unoccupied;
    } else {
      DPP_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    }
  }
  /* decrease from the free entry count the two entries which may not be allocated in case they are free */
  if (DPP_MCDS_TYPE_IS_FREE(DPP_MCDS_ENTRY_GET_TYPE(entry =
      MCDS_GET_MCDB_ENTRY(mcds, MCDS_INGRESS_LINK_END(mcds))))) {
    DPP_MC_ASSERT(nof_unoccupied);
    DPP_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    --nof_unoccupied;
  }
  if (DPP_MCDS_TYPE_IS_FREE(DPP_MCDS_ENTRY_GET_TYPE(entry =
      MCDS_GET_MCDB_ENTRY(mcds, DPP_MC_EGRESS_LINK_PTR_END)))) {
    DPP_MC_ASSERT(nof_unoccupied);
    DPP_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
    --nof_unoccupied;
  }
  if (nof_unoccupied != mcds->nof_unoccupied) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST,
             (BSL_META_U(unit,
                         "The mcdb has %lu free allocatable entries and in the mcds the value is %lu\n"), (unsigned long)nof_unoccupied, (unsigned long)mcds->nof_unoccupied));
    DPP_MC_ASSERT(0);
    return 10;
  }

  /* process over the free block lists */
  nof_unoccupied = 0;
  {
      dpp_free_entries_blocks_region_t *regions[ARAD_MCDS_NOF_REGIONS];
    int r;
    dpp_free_entries_block_size_t size, size_i;
    uint32 block, first_block, prev_block;
    regions[0] = &mcds->free_general;
    regions[1] = &mcds->ingress_alloc_free;
    regions[2] = &mcds->egress_alloc_free;

    /* loop over regions, processing the entries of each block of each list; checking their validity and counting entries */
    for (r = 0; r < ARAD_MCDS_NOF_REGIONS; ++r) {
      dpp_free_entries_blocks_region_t *region = regions[r];
      dpp_free_entries_block_list_t *lists = region->lists;
      DPP_MC_ASSERT(region->max_size <= DPP_MCDS_MAX_FREE_BLOCK_SIZE && region->max_size > 0);

      for (size = region->max_size; size; --size) { /* loop over the block sizes of the region */
        /* loop over the blocks in the list */
        if ((block = dpp_mcds_get_free_entries_block_from_list(mcds, lists + size - 1, 0))) { /* if the list is not empty */
          prev_block = DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, block);
          first_block = block;

          /* loop over the free block in the list */
          do {
            entry = MCDS_GET_MCDB_ENTRY(mcds, block);
            DPP_MC_ASSERT(block >= region->range_start && block + size - 1 <= region->range_end);
            DPP_MC_ASSERT(DPP_MCDS_ENTRY_GET_TYPE(entry) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK_START);
            DPP_MC_ASSERT(DPP_MCDS_GET_FREE_BLOCK_SIZE(mcds, block) == size);
            DPP_MC_ASSERT(prev_block == DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, block));
            if (!DPP_MCDS_ENTRY_GET_TEST_BIT(entry)) {
              LOG_ERROR(BSL_LS_SOC_MULTICAST,
                       (BSL_META_U(unit,
                                   "Free block %lu of size %u appeared previously in a linked list\n"), (unsigned long)block, size));
              DPP_MC_ASSERT(0);
              return 20;
            }
            DPP_MCDS_ENTRY_SET_TEST_BIT_OFF(entry);
            entry2 = entry;

            for (size_i = 1; size_i < size;  ++ size_i) { /* loop over remianing entries of the block */
              ++entry2;
              DPP_MC_ASSERT(DPP_MCDS_ENTRY_GET_TYPE(entry2) == DPP_MCDS_TYPE_VALUE_FREE_BLOCK);
              DPP_MC_ASSERT(DPP_MCDS_ENTRY_GET_FREE_PREV_ENTRY(entry2) == block);
              if (!DPP_MCDS_ENTRY_GET_TEST_BIT(entry2)) {
                LOG_ERROR(BSL_LS_SOC_MULTICAST,
                         (BSL_META_U(unit,
                                     "Free entry %lu of free block %lu of size %u appeared previously in a linked list\n"),
                                     (unsigned long)(block + size ), (unsigned long)block, size));
                DPP_MC_ASSERT(0);
                return 30;
              }
            DPP_MCDS_ENTRY_SET_TEST_BIT_OFF(entry2);
            }
            nof_unoccupied += size;
            prev_block = block;
            block = DPP_MCDS_GET_FREE_NEXT_ENTRY(mcds, block); /* move to new block */
          } while (block != first_block);
          DPP_MC_ASSERT(prev_block == DPP_MCDS_GET_FREE_PREV_ENTRY(mcds, block));
        }
      }
    }
  }
  if (nof_unoccupied != mcds->nof_unoccupied) {
    LOG_ERROR(BSL_LS_SOC_MULTICAST,
             (BSL_META_U(unit,
                         "The mcdb free block lists contain %lu entries and in the mcds the value is %lu\n"), (unsigned long)nof_unoccupied, (unsigned long)mcds->nof_unoccupied));
    DPP_MC_ASSERT(0);
    return 40;
  }

  return 0;
}


uint32
    dpp_mcds_multicast_terminate(
        SOC_SAND_IN int unit
    )
{
    dpp_mcds_base_t* mcds = dpp_get_mcds(unit);
    SOCDNX_INIT_FUNC_DEFS;
 
    if (mcds != NULL) {
        SOCDNX_IF_ERR_EXIT(dcmn_free_mem(unit, (void**)&mcds->mcdb));
        SOCDNX_IF_ERR_EXIT(dcmn_free_mem(unit, (void**)&mcds->prev_entries));
    }

    SOCDNX_IF_ERR_EXIT(dpp_deinit_mcds(unit));

    SOC_EXIT;
exit:
  SOCDNX_FUNC_RETURN;
}

/* Get the type of a MCDB entry */
uint32 dpp_get_mcdb_entry_type(
    SOC_SAND_IN  dpp_mcdb_entry_t* entry
)
{
    return DPP_MCDS_ENTRY_GET_TYPE((arad_mcdb_entry_t*)entry);
}
/* set the type of a MCDB entry */
void dpp_set_mcdb_entry_type(
    SOC_SAND_INOUT  dpp_mcdb_entry_t* entry,
    SOC_SAND_IN     uint32 type_value
)
{
    arad_mcdb_entry_t *e = (arad_mcdb_entry_t*)entry;
    DPP_MCDS_ENTRY_SET_TYPE(e, type_value);
}


/* Arad entry format writing functions */

/*
 * This function writes the hardware fields of egress format 0 (port+CUD replications with a link pointer) to a arad_mcdb_entry_t structure.
 */
void arad_mult_egr_write_entry_port_cud(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2 (7 bit port) */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dpp_mc_outlif_t cud1, cud2;
  dpp_mc_local_port_t port1, port2;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
      port1 = DPP_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep2);
      if (!rep1) {
          cud1 = cud2;
      } else {
          DPP_MC_ASSERT(cud1 == cud2);
      }
  } else {
      port2 = ARAD_MULT_EGRESS_SMALL_PORT_INVALID;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Af, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Bf, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, ENTRY_FORMATf, 0);
}

/*
 * This function writes the hardware fields of egress format 4/5 (port_CUD replications with no link pointer) to a arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void arad_mult_egr_write_entry_port_cud_noptr(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN    uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                  /* select format indicating that the following entry is next. */
)
{
  dpp_mc_outlif_t cud1, cud2;
  dpp_mc_local_port_t port1, port2;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
      port1 = DPP_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep2);
  } else {
      cud2 = DPP_MC_EGR_OUTLIF_DISABLED;
      port2 = DPP_MULT_EGRESS_PORT_INVALID;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, ENTRY_FORMATf, use_next ? 5 : 4);
}

/*
 * This function writes the hardware fields of egress format 2 (CUD only with link pointer) to a arad_mcdb_entry_t structure.
 */
void arad_mult_egr_write_entry_cud(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2  */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dpp_mc_outlif_t cud1, cud2;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DPP_MC_EGR_OUTLIF_DISABLED;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, ENTRY_FORMATf, 1);
}

/*
 * This function writes the hardware fields of egress format 6/7 (CUD only with no link pointer) to a arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void arad_mult_egr_write_entry_cud_noptr(
    SOC_SAND_IN     int               unit,
    SOC_SAND_INOUT  arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN     dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN     dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN     dpp_rep_data_t    *rep3,       /* replication 3 */
    SOC_SAND_IN     uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                   /* select format indicating that the following entry is next. */
)
{
  dpp_mc_outlif_t cud1, cud2, cud3;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep3) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep3) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud3 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep3);
  } else {
      cud3 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f, cud3);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, ENTRY_FORMATf, use_next ? 7 : 6);
}

/*
 * This function writes the hardware fields of egress format 1 (bitmap+CUD) to a arad_mcdb_entry_t structure.
 */
void arad_mult_egr_write_entry_bm_cud(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep,        /* the replication */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dpp_mc_outlif_t cud;
  dpp_mc_bitmap_id_t bm_id;
  if (rep) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep) == DPP_MCDS_REP_TYPE_EGR_BM_CUD);
      cud = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep);
      bm_id = DPP_MCDS_REP_DATA_GET_EGR_BM_ID(rep);
  } else {
      cud = DPP_MC_EGR_OUTLIF_DISABLED;
      bm_id = DPP_MC_EGR_BITMAP_DISABLED;
  }
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f, cud);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, BMP_PTRf, bm_id);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, ENTRY_FORMATf, 1);
}



/* Jericho entry format writing functions */

/*
 * This function writes the hardware fields of egress format 0 (port+CUD replications with a link pointer) to a arad_mcdb_entry_t structure.
 */
void jer_mult_egr_write_entry_port_cud(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dpp_mc_outlif_t cud1, cud2;
  dpp_mc_local_port_t port1, port2;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
      port1 = DPP_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep2);
      if (!rep1) {
          cud1 = cud2;
      } else {
          DPP_MC_ASSERT(cud1 == cud2);
      }
  } else {
      port2 = DPP_MULT_EGRESS_PORT_INVALID;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Af, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Bf, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, ENTRY_FORMATf, 0);
}

/*
 * This function writes the hardware fields of egress format 4/5 (port_CUD replications with no link pointer) to a arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void jer_mult_egr_write_entry_port_cud_noptr(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN    uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                  /* select format indicating that the following entry is next. */
)
{
  dpp_mc_outlif_t cud1, cud2;
  dpp_mc_local_port_t port1, port2;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
      port1 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
      port1 = DPP_MULT_EGRESS_PORT_INVALID;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_PORT_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
      port2 = DPP_MCDS_REP_DATA_GET_EGR_PORT(rep2);
  } else {
      cud2 = DPP_MC_EGR_OUTLIF_DISABLED;
      port2 = DPP_MULT_EGRESS_PORT_INVALID;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f, port1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f, port2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, ENTRY_FORMATf, use_next ? 3 : 2);
}

/*
 * This function writes the hardware fields of egress format 2 (CUD only with link pointer) to a arad_mcdb_entry_t structure.
 */
void jer_mult_egr_write_entry_cud(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN    dpp_rep_data_t    *rep2,       /* replication 2  */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dpp_mc_outlif_t cud1, cud2;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DPP_MC_EGR_OUTLIF_DISABLED;
  }

  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, ENTRY_FORMATf, 1);
}

/*
 * This function writes the hardware fields of egress format 6/7 (CUD only with no link pointer) to a arad_mcdb_entry_t structure.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
void jer_mult_egr_write_entry_cud_noptr(
    SOC_SAND_IN     int               unit,
    SOC_SAND_INOUT  arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN     dpp_rep_data_t    *rep1,       /* replication 1 */
    SOC_SAND_IN     dpp_rep_data_t    *rep2,       /* replication 2 */
    SOC_SAND_IN     dpp_rep_data_t    *rep3,       /* replication 3 */
    SOC_SAND_IN     uint8             use_next     /* If zero, select format indicating end of linked list, otherwise */
                                                   /* select format indicating that the following entry is next. */
)
{
  dpp_mc_outlif_t cud1, cud2, cud3;
  if (rep1) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep1) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud1 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep1);
  } else {
      cud1 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep2) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep2) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud2 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep2);
  } else {
      cud2 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  if (rep3) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep3) == DPP_MCDS_REP_TYPE_EGR_CUD);
      cud3 = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep3);
  } else {
      cud3 = DPP_MC_EGR_OUTLIF_DISABLED;
  }
  /* set the hardware fields */
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f, cud1);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f, cud2);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f, cud3);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, ENTRY_FORMATf, use_next ? 3 : 2);
}

/*
 * This function writes the hardware fields of egress format 1 (bitmap+CUD) to a arad_mcdb_entry_t structure.
 */
void jer_mult_egr_write_entry_bm_cud(
    SOC_SAND_IN    int               unit,
    SOC_SAND_INOUT arad_mcdb_entry_t *mcdb_entry, /* structure to write to */
    SOC_SAND_IN    dpp_rep_data_t    *rep,        /* the replication */
    SOC_SAND_IN    uint32            next_entry   /* the next entry */
)
{
  dpp_mc_outlif_t cud;
  dpp_mc_bitmap_id_t bm_id;
  if (rep) {
      DPP_MC_ASSERT(DPP_MCDS_REP_DATA_GET_TYPE(rep) == DPP_MCDS_REP_TYPE_EGR_BM_CUD);
      cud = DPP_MCDS_REP_DATA_GET_EGR_CUD(rep);
      bm_id = DPP_MCDS_REP_DATA_GET_EGR_BM_ID(rep);
  } else {
      cud = DPP_MC_EGR_OUTLIF_DISABLED;
      bm_id = DPP_MC_EGR_BITMAP_DISABLED;
  }
  mcdb_entry->word0 = 0; /* Zero bits not covered by these fields */
  mcdb_entry->word1 &= ~JER_MC_ENTRY_MASK_VAL;
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f, cud);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, BMP_PTRf, bm_id);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, LINK_PTRf, next_entry);
  soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, ENTRY_FORMATf, 1);
}


/* Allocate and init the mcds structure, not allocating memories it points to */
uint32 dpp_init_mcds(
    SOC_SAND_IN    int         unit
)
{
    dpp_mcds_base_t *dpp_base;
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(dpp_alloc_mcds(unit, sizeof(*dpp_base), (void*)&dpp_base));

    dpp_base->unit = unit;
    dpp_base->common.flags = 0;
    dpp_base->nof_egr_ll_groups = SOC_DPP_CONFIG(unit)->tm.nof_mc_ids - (SOC_DPP_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1); /* The number of egress linked list groups (per core) */
    dpp_base->common.get_mcdb_entry_type = dpp_get_mcdb_entry_type;
    dpp_base->common.set_mcdb_entry_type = dpp_set_mcdb_entry_type;
    dpp_base->set_egress_linked_list = dpp_mcds_set_egress_linked_list;
    if (SOC_IS_ARADPLUS_AND_BELOW(unit)) { /* init mcds values for Arad* */
        dpp_base->common.get_mcdb_entry_from_mcds = dpp_mcds_get_mcdb_entry_from_mcds;
        dpp_base->common.get_next_pointer = arad_mcdb_get_next_pointer;
        dpp_base->common.set_next_pointer = arad_mcdb_set_next_pointer;

        dpp_base->common.ingress_link_end = ARAD_MC_INGRESS_LINK_PTR_END;
        dpp_base->free_value[0] = ARAD_MC_UNOCCUPIED_ENTRY_LOW;
        dpp_base->free_value[1] = ARAD_MC_UNOCCUPIED_ENTRY_HIGH;
        dpp_base->empty_ingr_value[0] = ARAD_MC_ING_EMPTY_ENTRY_LOW;
        dpp_base->empty_ingr_value[1] = ARAD_MC_ING_EMPTY_ENTRY_HIGH;
        dpp_base->msb_word_mask = ARAD_MC_ENTRY_MASK_VAL;
        dpp_base->ingr_word1_replication_mask = 3;
        dpp_base->max_egr_cud_field = dpp_base->max_ingr_cud_field = ((1 << 16) - 1);
        /* The offset in the MCDB to which the MCID is added to get the first entry of the group of core 0 */
        dpp_base->egress_mcdb_offset = ARAD_MULT_NOF_MULTICAST_GROUPS;

        dpp_base->egr_mc_write_entry_port_cud = arad_mult_egr_write_entry_port_cud;
        dpp_base->egr_mc_write_entry_port_cud_noptr = arad_mult_egr_write_entry_port_cud_noptr;
        dpp_base->egr_mc_write_entry_cud = arad_mult_egr_write_entry_cud;
        dpp_base->egr_mc_write_entry_cud_noptr = arad_mult_egr_write_entry_cud_noptr;
        dpp_base->egr_mc_write_entry_bm_cud = arad_mult_egr_write_entry_bm_cud;
        dpp_base->get_replications_from_entry = arad_mcds_get_replications_from_entry;
        dpp_base->convert_ingress_replication_hw2api = arad_convert_ingress_replication_hw2api;

        switch (SOC_DPP_CONFIG(unit)->arad->init.dram.fmc_dbuff_mode) {
          case ARAD_INIT_FMC_4K_REP_64K_DBUFF_MODE:
            dpp_base->max_nof_ingr_replications = 4096 - DPP_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
            break;
          case ARAD_INIT_FMC_64_REP_128K_DBUFF_MODE:
            dpp_base->max_nof_ingr_replications = 64 - DPP_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
            break;
          default:
            SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("invalid buffer mode")));
        }
        dpp_base->max_nof_mmc_replications = 1; 

#ifdef BCM_88675_A0
    } else { /* init mcds values for Jericho */
        dpp_base->common.get_mcdb_entry_from_mcds = dpp_mcds_get_mcdb_entry_from_mcds;
        dpp_base->common.get_next_pointer = jer_mcdb_get_next_pointer;
        dpp_base->common.set_next_pointer = jer_mcdb_set_next_pointer;

        dpp_base->common.ingress_link_end = JER_MC_INGRESS_LINK_PTR_END;
        dpp_base->free_value[0] = JER_MC_UNOCCUPIED_ENTRY_LOW;
        dpp_base->free_value[1] = JER_MC_UNOCCUPIED_ENTRY_HIGH;
        dpp_base->empty_ingr_value[0] = JER_MC_ING_EMPTY_ENTRY_LOW;
        dpp_base->empty_ingr_value[1] = JER_MC_ING_EMPTY_ENTRY_HIGH;
        dpp_base->msb_word_mask = JER_MC_ENTRY_MASK_VAL;
        dpp_base->ingr_word1_replication_mask = 0x1f;
        dpp_base->max_ingr_cud_field = ((1 << 19) - 1);
        dpp_base->max_egr_cud_field = ((1 << 18) - 1);
        /* The offset in the MCDB to which the MCID is added to get the first entry of the group of core 0 */
        dpp_base->egress_mcdb_offset = SOC_DPP_CONFIG(unit)->tm.nof_ingr_mc_ids -
          (SOC_DPP_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high + 1);

        dpp_base->egr_mc_write_entry_port_cud = jer_mult_egr_write_entry_port_cud;
        dpp_base->egr_mc_write_entry_port_cud_noptr = jer_mult_egr_write_entry_port_cud_noptr;
        dpp_base->egr_mc_write_entry_cud = jer_mult_egr_write_entry_cud;
        dpp_base->egr_mc_write_entry_cud_noptr = jer_mult_egr_write_entry_cud_noptr;
        dpp_base->egr_mc_write_entry_bm_cud = jer_mult_egr_write_entry_bm_cud;
        dpp_base->get_replications_from_entry = jer_mcds_get_replications_from_entry;
        dpp_base->convert_ingress_replication_hw2api = jer_convert_ingress_replication_hw2api;

        switch (SOC_DPP_CONFIG(unit)->arad->init.dram.fmc_dbuff_mode) {
          case JERICHO_INIT_FMC_64_REP_512K_DBUFF_MODE:
            dpp_base->max_nof_ingr_replications = 64 - DPP_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
            break;
          default: /* in case DRAM buffers support 4K replications, or no DRAM buffers */
            dpp_base->max_nof_ingr_replications = 4096 - DPP_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
        }
        dpp_base->max_nof_mmc_replications = 8 - DPP_INGR_MC_NOF_RESERVED_BUFFER_REPLICATIONS;
#endif /* BCM_88675_A0 */
    }
exit:
    SOCDNX_FUNC_RETURN;
}


/* De-allocate the mcds structure, not de-allocating memories it points to */
uint32 dpp_deinit_mcds(
    SOC_SAND_IN    int         unit
)
{
    SOCDNX_INIT_FUNC_DEFS;
    SOCDNX_IF_ERR_EXIT(dpp_dealloc_mcds(unit));

exit:
    SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

