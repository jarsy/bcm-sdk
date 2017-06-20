/* $Id: dnx_multicast_egress.c,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

#include <soc/dnx/legacy/multicast_imp.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>


#include <soc/mcm/memregs.h>


/*************
 * INCLUDES  *
 *************/
/* { */
#include <shared/bsl.h>
#include <shared/swstate/access/sw_state_access.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/SAND/Utils/sand_header.h>
#include <soc/dnx/legacy/TMC/tmc_api_multicast_egress.h>
#include <soc/dnx/legacy/port_sw_db.h>
#include <soc/dnx/legacy/ARAD/arad_tbl_access.h>


/* } */

/*************
 * DEFINES   *
 *************/
/* { */

#define DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_MAX          (8*1024)


#define DNX_EGQ_VLAN_TABLE_TBL_ENTRY_SIZE 9
#define DNX_FDA_MC_ENTRY_SIZE 3
#define FDT_IPT_MESH_MC_ENTRY_SIZE 5
#define EGR_PER_CORE_REP_MAX_ENTRY_SIZE FDT_IPT_MESH_MC_ENTRY_SIZE

#define DNX_FDA_BITS_PER_GROUP 2
#define DNX_FDA_GROUPS_PER_ENTRY 32
#define DNX_FDA_OFFSET_IN_GROUP_BITS 0
#define FDT_IPT_MESH_MC_BITS_PER_GROUP 4
#define FDT_IPT_MESH_MC_GROUPS_PER_ENTRY 32
#define FDT_IPT_MESH_MC_OFFSET_IN_GROUP_BITS 2
#define CORES_BITS_MASK 3


#define JER2_JER_CUD2PORT_MAPPING_PORTS_PER_ENTRY 4
#define JER2_JER_CUD2PORT_MAPPING_NOF_PORT_BITS 8
#define JER2_JER_CUD2PORT_MAPPING_PORT_MASK ((1 << JER2_JER_CUD2PORT_MAPPING_NOF_PORT_BITS) - 1)
/* } */

/*************
 *  MACROS   *
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


/* Get the cores that an egress multicast group is replicated to (Jericho hardware) */
static uint32 dnx_egress_group_get_core_replication(
    DNX_SAND_IN  int                   unit,       /* input: device */
    DNX_SAND_IN  dnx_mc_id_t           group_mcid, /* input: group mcid */
    DNX_SAND_OUT dnx_mc_core_bitmap_t  *out_cores) /* output: cores replicated to */
{
    int index, offset; /* index in the table being changed and bit offset inside the table entry */
    uint32 *entry_word; /* word to process in the table entry */
    uint32 data[EGR_PER_CORE_REP_MAX_ENTRY_SIZE];
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(group_mcid < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids && out_cores);
    if (SOC_DNX_CONFIG(unit)->tm.mc_mode & (DNX_MC_EGR_CORE_FDA_MODE | DNX_MC_EGR_CORE_MESH_MODE)) {
        /* If the hardware uses a table to control which (per core) egress group is active, then configure it */
        if (SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_EGR_CORE_FDA_MODE) {
            index = group_mcid / DNX_FDA_GROUPS_PER_ENTRY;
            offset = DNX_FDA_BITS_PER_GROUP * (group_mcid % DNX_FDA_GROUPS_PER_ENTRY);
            entry_word = data + offset / DNX_SAND_NOF_BITS_IN_UINT32;
            offset = offset % DNX_SAND_NOF_BITS_IN_UINT32;
            DNXC_IF_ERR_EXIT(READ_FDA_FDA_MCm(unit, MEM_BLOCK_ANY, index, data));
        } else {
            index = group_mcid / FDT_IPT_MESH_MC_GROUPS_PER_ENTRY;
            offset = FDT_IPT_MESH_MC_BITS_PER_GROUP * (group_mcid % FDT_IPT_MESH_MC_GROUPS_PER_ENTRY) + FDT_IPT_MESH_MC_OFFSET_IN_GROUP_BITS;
            entry_word = data + offset / DNX_SAND_NOF_BITS_IN_UINT32;
            offset = offset % DNX_SAND_NOF_BITS_IN_UINT32;
            DNXC_IF_ERR_EXIT(READ_FDT_IPT_MESH_MCm(unit, MEM_BLOCK_ANY, index, data));
        }
        *out_cores = (*entry_word >> offset) & CORES_BITS_MASK;
    } else { /* No hardware fore core replication */
        *out_cores = DNX_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit);
    }
exit:
    DNXC_FUNC_RETURN;
}

/* Set the cores that an egress multicast group (or all groups) is replicated to (Jericho hardware) */
static uint32 dnx_egress_group_set_core_replication(
    DNX_SAND_IN  int                   unit,       /* input: device */
    DNX_SAND_IN  dnx_mc_id_t           group_mcid, /* input: group mcid, SOC_DNX_CONFIG(unit)->tm.nof_mc_ids means set all groups */
    DNX_SAND_IN  dnx_mc_core_bitmap_t  cores)      /* input: cores to replicate to */
{
    int index, offset; /* index in the table being changed and bit offset inside the table entry */
    uint32 *word_to_change, orig_word; /* word to change in the table entry, and its original value */
    uint32 i;
    uint32 groups_per_entry, bits_per_group, offset_in_group_bits;
    uint32 nof_full_entries, groups_remainder, uint32s_per_entry;
    soc_mem_t table;
    int mode;
    const static uint32 mode_bit[2] = {DNX_MC_EGR_CORE_FDA_MODE, DNX_MC_EGR_CORE_MESH_MODE};
    uint32 data[EGR_PER_CORE_REP_MAX_ENTRY_SIZE];
    DNXC_INIT_FUNC_DEFS;

    DNX_MC_ASSERT(group_mcid <= SOC_DNX_CONFIG(unit)->tm.nof_mc_ids && cores <= DNX_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit));
    for (mode = 0; mode < 2; ++mode) { /* Set hardware fore ingress and for egress if needed */
        if ((SOC_DNX_CONFIG(unit)->tm.mc_mode & mode_bit[mode]) == 0) continue;

        /* select the table used to control which egress core is replicated to per group, and its properties */
        if (mode_bit[mode] == DNX_MC_EGR_CORE_FDA_MODE) {
            groups_per_entry = DNX_FDA_GROUPS_PER_ENTRY;
            bits_per_group = DNX_FDA_BITS_PER_GROUP;
            offset_in_group_bits = DNX_FDA_OFFSET_IN_GROUP_BITS;
            table = FDA_FDA_MCm;
        } else {
            groups_per_entry = FDT_IPT_MESH_MC_GROUPS_PER_ENTRY;
            bits_per_group = FDT_IPT_MESH_MC_BITS_PER_GROUP;
            offset_in_group_bits = FDT_IPT_MESH_MC_OFFSET_IN_GROUP_BITS;
            table = FDT_IPT_MESH_MCm;
        }

        if (group_mcid == SOC_DNX_CONFIG(unit)->tm.nof_mc_ids) { /* set core replication of all groups */
            if (SOC_DNX_CONFIG(unit)->tm.nof_mc_ids == 0) {
                DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("no egress MC groups")));
            }

            nof_full_entries = SOC_DNX_CONFIG(unit)->tm.nof_mc_ids / groups_per_entry;
            groups_remainder = SOC_DNX_CONFIG(unit)->tm.nof_mc_ids % groups_per_entry;
            uint32s_per_entry = groups_per_entry * bits_per_group / DNX_SAND_NOF_BITS_IN_UINT32; /* assumes no remainder */

            data[0] = ((uint32)cores) << offset_in_group_bits; /* set the bits of one group; */
            for (i = DNX_FDA_BITS_PER_GROUP; i < DNX_SAND_NOF_BITS_IN_UINT32; i *= 2) { /* fill a uint32 with the group */
                data[0] |= data[0] << i;
            }
            for (i = 1; i < uint32s_per_entry; ++i) { /* fill a table entry */
                data[i] = data[0];
            }
            data[i] = 0; /* ECC word */

            if (nof_full_entries > 0) {
                /* coverity[negative_returns:FALSE] */
                DNXC_SAND_IF_ERR_EXIT(jer2_arad_fill_partial_table_with_entry(unit, table, 0, 0, MEM_BLOCK_ANY, 0, nof_full_entries - 1, data));
            }
            if (groups_remainder > 0) {
                groups_remainder *= bits_per_group; /* offset of bits to clear in the additional table entry */
                i = groups_remainder / DNX_SAND_NOF_BITS_IN_UINT32; /* offset in uint32s */
                groups_remainder = groups_remainder % DNX_SAND_NOF_BITS_IN_UINT32; /* offset of bits to clear in uint32 */
    
                data[i] &= ((((uint32)1) << groups_remainder) - 1);
                for (++i; i < uint32s_per_entry; ++i) { /* clear the rest of the table entry */
                    data[i] = 0;
                }
                DNXC_IF_ERR_EXIT(soc_mem_write(unit, table, MEM_BLOCK_ANY, nof_full_entries, data));
            }

        } else { /* set core replication of a specific group */

            index = group_mcid / groups_per_entry;
            offset = bits_per_group * (group_mcid % groups_per_entry) + offset_in_group_bits;
            word_to_change = data + offset / DNX_SAND_NOF_BITS_IN_UINT32;
            offset = offset % DNX_SAND_NOF_BITS_IN_UINT32;

            DNXC_IF_ERR_EXIT(soc_mem_read(unit, table, MEM_BLOCK_ANY, index, data));
            orig_word = *word_to_change;
            *word_to_change &= ~(((uint32)CORES_BITS_MASK) << offset);
            *word_to_change |= ((uint32)cores) << offset;
            if (*word_to_change != orig_word) { /* If the value is changed, write the new value */
                DNXC_IF_ERR_EXIT(soc_mem_write(unit, table, MEM_BLOCK_ALL, index, data));
            }
        }
    }
exit:
    DNXC_FUNC_RETURN;
}


/* Mark the given egress group as open in the warm boot data */
uint32 dnx_egress_group_open_set(
    DNX_SAND_IN  int     unit, /* device */
    DNX_SAND_IN  uint32  group_id,  /* multicast ID */
    DNX_SAND_IN  uint8   is_open    /* non zero value will mark the group as open */
)
{
    uint8 bit_val;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS;

    DNX_MC_ASSERT(group_id < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(unit, group_id, &bit_val);
    DNXC_IF_ERR_EXIT(rv);
    bit_val = bit_val ? 1: 0;
    if (bit_val != is_open) {
        if(is_open) {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_set(unit, group_id);
        } else {
            rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_clear(unit, group_id);
        }
        DNXC_IF_ERR_EXIT(rv);
    }
exit:
    DNXC_FUNC_RETURN;
}

/* Check if the given egress group is created=open, will return 1 if the group is marked as open, or 0 */
int dnx_egress_group_open_get(
    DNX_SAND_IN  int    unit,
    DNX_SAND_IN  uint32 group_id, /* multicast ID */
    DNX_SAND_OUT uint8 *is_open
)
{
    uint8 bit_val;
    soc_error_t rv;

    DNXC_INIT_FUNC_DEFS

    DNX_MC_ASSERT(group_id < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
    rv = sw_state_access[unit].dnx.soc.jer2_arad.tm.jer2_arad_multicast.egress_groups_open_data.bit_get(unit, group_id, &bit_val);
    DNXC_IF_ERR_EXIT(rv);
    *is_open = bit_val ? 1: 0;
    
exit:
    DNXC_FUNC_RETURN
}


/*
 * Set the outlif (cud) to (local) port mapping from the given cud to the given port.
 */
uint32 dnx_mult_cud_to_port_map_set(
    DNX_SAND_IN int                 unit, /* input device */
    DNX_SAND_IN uint32              flags,/* flags - currently non zero value means allowing to switch cores */
    DNX_SAND_IN uint32              cud,  /* input cud/outlif */
    DNX_SAND_IN DNX_TMC_FAP_PORT_ID port  /* input local logical port mapped to */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint32 entry[2] = {0}, shift;
    uint32 tm_port, previous_core;
    int core;
    int index;

    DNXC_INIT_FUNC_DEFS;
    if (cud > mcds->max_egr_cud_field) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("CUD too big")));
    }

    if (SOC_IS_JERICHO(unit)) {
        index = cud / JER2_JER_CUD2PORT_MAPPING_PORTS_PER_ENTRY;
        shift = JER2_JER_CUD2PORT_MAPPING_NOF_PORT_BITS * (cud % JER2_JER_CUD2PORT_MAPPING_PORTS_PER_ENTRY);
        if (port == _SHR_GPORT_INVALID) {
            tm_port = DNX_MULT_EGRESS_PORT_INVALID;
            DNX_CUD2CORE_SET_CORE(unit, cud, DNX_CUD2CORE_UNDEF_VALUE);
        } else {
            DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
            DNX_CUD2CORE_GET_CORE(unit, cud, previous_core); /*get core from sw*/
            if (core != previous_core) {
                if (previous_core != DNX_CUD2CORE_UNDEF_VALUE && !flags) {
                    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Mapping to a port in a different core than the current mapped port is not allowed without BCM_PORT_ENCAP_MAP_ALLOW_CORE_CHANGE")));
                }
                DNX_CUD2CORE_SET_CORE(unit, cud, core);
            }
        }
        DNXC_IF_ERR_EXIT(READ_EDB_MAP_OUTLIF_TO_DSPm(unit, MEM_BLOCK_ANY, index, entry));
        entry[0] &= ~((uint32)JER2_JER_CUD2PORT_MAPPING_PORT_MASK << shift);
        entry[0] |= tm_port << shift;
        DNXC_IF_ERR_EXIT(WRITE_EDB_MAP_OUTLIF_TO_DSPm(unit, MEM_BLOCK_ANY, index, entry));
    } else {
        if (port >= DNX_MULT_EGRESS_PORT_INVALID) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("Invalid Port")));
        }
        soc_mem_field32_set(unit, EGQ_MAP_OUTLIF_TO_DSPm, entry, DSPf, port); 
        DNXC_IF_ERR_EXIT(WRITE_EGQ_MAP_OUTLIF_TO_DSPm(unit, MEM_BLOCK_ANY, cud, entry));
    }

exit:
  DNXC_FUNC_RETURN;
}

/*
 * Get the outlif (cud) to (local) port mapping from the given cud.
 */
uint32 dnx_mult_cud_to_port_map_get(
    DNX_SAND_IN  int                 unit, /* input device */
    DNX_SAND_IN  uint32              cud,  /* input cud/outlif */
    DNX_SAND_OUT DNX_TMC_FAP_PORT_ID *port /* output local logical port */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint32 entry[2];
    uint32 tm_port;
    soc_port_t logical_port;
    int core;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(port);
    if (cud > mcds->max_egr_cud_field) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("CUD too big")));
    }

    if (SOC_IS_JERICHO(unit)) {
        DNXC_IF_ERR_EXIT(READ_EDB_MAP_OUTLIF_TO_DSPm(unit, MEM_BLOCK_ANY,
          cud / JER2_JER_CUD2PORT_MAPPING_PORTS_PER_ENTRY, entry));
        tm_port = (entry[0] >> (JER2_JER_CUD2PORT_MAPPING_NOF_PORT_BITS * (cud % JER2_JER_CUD2PORT_MAPPING_PORTS_PER_ENTRY))) &
          JER2_JER_CUD2PORT_MAPPING_PORT_MASK;
        DNX_CUD2CORE_GET_CORE(unit, cud, core); /*get core from sw*/
        if (core == DNX_CUD2CORE_UNDEF_VALUE) {
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("CUD isn't mapped to port")));
        }
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_tm_to_local_port_get(unit, core, tm_port, &logical_port)); /*set port to local logical port*/
        *port = logical_port;
    } else {
        DNXC_IF_ERR_EXIT(READ_EGQ_MAP_OUTLIF_TO_DSPm(unit, MEM_BLOCK_ANY, cud, entry));
        *port = soc_mem_field32_get(unit, EGQ_MAP_OUTLIF_TO_DSPm, entry, DSPf);
    }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Gets the egress multicast group with the specified multicast id.
 * will return up to mc_group_size replications, and the exact
 * Works with both TDM and non TDM groups.
 * The group's replication number is returned in exact_mc_group_size.
 * The number of replications returned in the output arrays is
 * min{mc_group_size, exact_mc_group_size}.
 * It is not an error if the group is not open.
 */
uint32 dnx_mult_eg_get_group(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t    group_mcid,           /* group id */
    DNX_SAND_IN  uint32         mc_group_size,        /* maximum replications to return */
    DNX_SAND_OUT soc_gport_t    *ports,               /* output logical ports (array of size mc_group_size) used if !reps */
    DNX_SAND_OUT soc_if_t       *cuds,                /* output CUDs (array of size mc_group_size) used if !reps */
    DNX_SAND_OUT soc_multicast_replication_t *reps,   /* output replication array (array of size mc_group_size*/
    DNX_SAND_OUT uint32         *exact_mc_group_size, /* the number of replications in the group will be returned here */
    DNX_SAND_OUT uint8          *is_open              /* will return if the group is open (false or true) */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(exact_mc_group_size);
    DNXC_NULL_CHECK(is_open);
    if (mc_group_size && !reps && (!ports || !cuds)) { /* we check that the output data pointers are not null if we need to return data */
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("NULL pointer")));
    }

    DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, group_mcid, TRUE, is_open)) ; /* check if the group is open */

    if (*is_open) {
        uint16 group_size;
        DNXC_IF_ERR_EXIT(dnx_mcds_get_group(
            unit, DNX_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit), TRUE, FALSE, group_mcid, DNX_MCDS_TYPE_VALUE_EGRESS, mc_group_size, &group_size));
        *exact_mc_group_size = group_size;
        DNXC_IF_ERR_EXIT(dnx_mcds_copy_replications_to_arrays(unit, 1, mc_group_size,  ports, cuds, reps));
    } else { /* group is not open */
        *exact_mc_group_size = 0;
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Read a bitmap from the egress MC bitmap table.
 */
uint32 dnx_egq_vlan_table_tbl_get(
    DNX_SAND_IN   int    unit,
    DNX_SAND_IN   uint32 entry_offset,
    DNX_SAND_OUT  uint32 *vlan_table_tbl_data
)
{
    int core, nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    uint32 *bitmap_ptr = vlan_table_tbl_data;
    DNXC_INIT_FUNC_DEFS;

    for (core = 0; core < nof_active_cores; ++core) {
        DNXC_IF_ERR_EXIT(READ_EGQ_VLAN_TABLEm(unit, EGQ_BLOCK(unit, core), entry_offset, bitmap_ptr));
        bitmap_ptr += DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32;
    }
exit:
    DNXC_FUNC_RETURN;
}

/*
 * Write a bitmap to the egress MC bitmap table.
 */
uint32 dnx_egq_vlan_table_tbl_set(
    DNX_SAND_IN   int     unit,
    DNX_SAND_IN   uint32  entry_offset,
    DNX_SAND_IN   uint32* vlan_table_tbl_data
)
{
    int core, i, nof_active_cores = SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dnx_mc_core_bitmap_t non_zero_cores = DNX_MC_CORE_BITAMAP_NO_CORES; /* The cores whose ports have replications */
    dnx_mc_core_bitmap_t prev_cores_with_reps, new_cores_with_reps; /* The cores whose ports previously had/now have replications */
    const uint32 *bitmap_ptr = vlan_table_tbl_data;
    DNXC_INIT_FUNC_DEFS;

    for (core = 0; core < nof_active_cores; ++core) { /* find which cores have replications */
        for (i = 0; i < DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32; ++i) {
            if (bitmap_ptr[i]) {
                non_zero_cores |= DNX_MC_CORE_BITAMAP_CORE_0 << core;
                break;
            }
        }
        bitmap_ptr += DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32;
    }

    DNXC_IF_ERR_EXIT(dnx_egress_group_get_core_replication(unit, entry_offset, &prev_cores_with_reps)); /* get egress cores replicated to */
    new_cores_with_reps = non_zero_cores & prev_cores_with_reps; /* zero cores who lost all their port replications */
    if (new_cores_with_reps != prev_cores_with_reps) { /* Do we have to disable replication to egress cores? */
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, entry_offset, new_cores_with_reps));
        prev_cores_with_reps = new_cores_with_reps;
    }

    /* Set the new bitmaps */
    bitmap_ptr = vlan_table_tbl_data;
    for (core = 0; core < nof_active_cores; ++core) {
        DNXC_IF_ERR_EXIT(WRITE_EGQ_VLAN_TABLEm(unit, EGQ_BLOCK(unit, core), entry_offset, (uint32*)bitmap_ptr));
        bitmap_ptr += DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32;
    }

    if (non_zero_cores != prev_cores_with_reps) { /* Do we have to enable replication to egress cores? */
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, entry_offset, non_zero_cores));
    }
exit:
    DNXC_FUNC_RETURN;
}



/*
 * Gets the egress replications (ports) of the given bitmap.
 * If the bitmap is a vlan egress group, it does not have to be open/created.
 */
uint32 dnx_mult_eg_bitmap_group_get(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  dnx_mc_id_t                           bitmap_id, /* ID of the bitmap */
    DNX_SAND_OUT DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *group     /* output TM port bitmap */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(group);
    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, bitmap_id, &group->bitmap[0]));
exit:
    DNXC_FUNC_RETURN;
}


/* entry format writing functions */

/*
 * This function writes egress format 0 (port+CUD replications with a link pointer) to a mcds mcdb entry and then to hardware.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
uint32 dnx_mult_eg_write_entry_port_cud(
    DNX_SAND_IN  int            unit,
    DNX_SAND_IN  dnx_mc_id_t    multicast_id_ndx, /* mcdb to write to */
    DNX_SAND_IN  dnx_rep_data_t *rep1,            /* replication 1 */
    DNX_SAND_IN  dnx_rep_data_t *rep2,            /* replication 2 (7 bit port for Arad) */
    DNX_SAND_IN  uint32         next_entry,       /* the next entry */
    DNX_SAND_IN  uint32         prev_entry        /* the previous entry written only to mcds */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;

  mcds->egr_mc_write_entry_port_cud(unit, mcdb_entry, rep1, rep2, next_entry); /* set the hardware fields */
  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_EGRESS_START : DNX_MCDS_TYPE_VALUE_EGRESS);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * This function writes egress format 4/5 (port_CUD replications with no link pointer) to a mcds mcdb entry and then to hardware.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
uint32 dnx_mult_eg_write_entry_port_cud_noptr(
    DNX_SAND_IN int            unit,
    DNX_SAND_IN dnx_mc_id_t    multicast_id_ndx, /* mcdb to write to */
    DNX_SAND_IN dnx_rep_data_t *rep1,            /* replication 1 */
    DNX_SAND_IN dnx_rep_data_t *rep2,            /* replication 2 */
    DNX_SAND_IN uint8          use_next,         /* If zero, select format indicating end of linked list, otherwise */
                                                 /* select format indicating that the following entry is next. */
    DNX_SAND_IN uint32         prev_entry        /* the previous entry written only to mcds */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = dnx_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;

  mcds->egr_mc_write_entry_port_cud_noptr(unit, mcdb_entry, rep1, rep2, use_next); /* set the hardware fields */
  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_EGRESS_START : DNX_MCDS_TYPE_VALUE_EGRESS);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * This function writes egress format 2 (CUD only with link pointer) to a mcds mcdb entry and then to hardware.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
uint32 dnx_mult_eg_write_entry_cud(
    DNX_SAND_IN int            unit,
    DNX_SAND_IN dnx_mc_id_t    multicast_id_ndx, /* mcdb index to write to */
    DNX_SAND_IN dnx_rep_data_t *rep1,            /* replication 1 */
    DNX_SAND_IN dnx_rep_data_t *rep2,            /* replication 2  */
    DNX_SAND_IN uint32         next_entry,       /* the next entry */
    DNX_SAND_IN uint32         prev_entry        /* the previous entry written only to mcds */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = dnx_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;

  mcds->egr_mc_write_entry_cud(unit, mcdb_entry, rep1, rep2, next_entry); /* set the hardware fields */
  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_EGRESS_START : DNX_MCDS_TYPE_VALUE_EGRESS);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * This function writes egress format 6/7 (CUD only with no link pointer) to a mcds mcdb entry and then to hardware.
 * The replications to write are specified by structure pointers, NULL pointers mean disabled replications.
 */
uint32 dnx_mult_eg_write_entry_cud_noptr(
    DNX_SAND_IN int            unit,
    DNX_SAND_IN dnx_mc_id_t    multicast_id_ndx, /* mcdb to write to */
    DNX_SAND_IN dnx_rep_data_t *rep1,            /* replication 1 */
    DNX_SAND_IN dnx_rep_data_t *rep2,            /* replication 2 */
    DNX_SAND_IN dnx_rep_data_t *rep3,            /* replication 3 */
    DNX_SAND_IN uint8          use_next,         /* If zero, select format indicating end of linked list, otherwise */
                                                 /* select format indicating that the following entry is next. */
    DNX_SAND_IN uint32         prev_entry        /* the previous entry written only to mcds */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = dnx_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;

  mcds->egr_mc_write_entry_cud_noptr(unit, mcdb_entry, rep1, rep2, rep3, use_next); /* set the hardware fields */
  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_EGRESS_START : DNX_MCDS_TYPE_VALUE_EGRESS);

exit:
  DNXC_FUNC_RETURN;
}

/*
 * This function writes egress format 1 (bitmap+CUD) to a mcds mcdb entry and then to hardware.
 * The replication to write is specified by structure pointers, NULL pointers mean disabled replication.
 */
uint32 dnx_mult_eg_write_entry_bm_cud(
    DNX_SAND_IN int            unit,
    DNX_SAND_IN dnx_mc_id_t    multicast_id_ndx, /* mcdb to write to */
    DNX_SAND_IN dnx_rep_data_t *rep,             /* the replication */
    DNX_SAND_IN uint32         next_entry,       /* the next entry */
    DNX_SAND_IN uint32         prev_entry        /* the previous entry written only to mcds */
)
{
  dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
  jer2_arad_mcdb_entry_t *mcdb_entry = dnx_mcds_get_mcdb_entry(unit, multicast_id_ndx);
  DNXC_INIT_FUNC_DEFS;

  mcds->egr_mc_write_entry_bm_cud(unit, mcdb_entry, rep, next_entry); /* set the hardware fields */
  DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, multicast_id_ndx)); /* write to hardware */

  DNX_MCDS_SET_PREV_ENTRY(mcds, multicast_id_ndx, prev_entry); /* set software link to previous entry */
  DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, prev_entry == multicast_id_ndx ? DNX_MCDS_TYPE_VALUE_EGRESS_START : DNX_MCDS_TYPE_VALUE_EGRESS);

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Creates a block of egress entries without pointers.
 * The next entry is always the next block entry implicitly by the format,
 * except for the last entry if last_entry_pointer==0.
 * Used to set the whole block if it is the last in the group,
 * or otherwise the whole block except for the last entry with the pointer.
 */
static uint32 dnx_mult_eg_add_group_block(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  int                           core,               /* core of the linked list */
    DNX_SAND_IN  uint32                        block_start,        /* mcdb index of block start */
    DNX_SAND_IN  dnx_free_entries_block_size_t block_size,         /* number of entries to write in the block (excluding a possible last entry with an end of list format) */
    DNX_SAND_IN  dnx_free_entries_block_size_t last_entry_pointer, /* must be either: 0-last block entry ends group, 1-last block entry with pointer not handled by this function */
    DNX_SAND_IN  dnx_free_entries_block_size_t port_entries,       /* number of entries to create from remaining port+outlif replications */
    DNX_SAND_IN  dnx_free_entries_block_size_t outlif_entries,     /* number of entries to create from remaining outlif replications */
    DNX_SAND_IN  dnx_free_entries_block_size_t port_replications,  /* number of port+outlif replications (0-1) to add in one partially filled entry */
    DNX_SAND_IN  dnx_free_entries_block_size_t outlif_replications,/* number of port+outlif replications (0-2) to add in one partially filled entry */
    DNX_SAND_INOUT dnx_rep_data_t              **port_outlif_reps, /* A pointer to the non couple port+CUD replications to work on */
    DNX_SAND_INOUT dnx_rep_data_t              **cud_reps          /* A pointer to the CUD only replications to work on */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    dnx_rep_data_t *rep, *rep2;
    uint32 last_entry = block_start + block_size + last_entry_pointer - 1; /* last entry of the block */
    uint32 index = block_start, prev = 0; /* current entry, previous entry in the block */
    dnx_free_entries_block_size_t left;
    DNXC_INIT_FUNC_DEFS;

    DNX_MC_ASSERT(
      mcds->nof_egr_port_outlif_reps[core] >= port_entries * 2 &&
      mcds->nof_egr_outlif_reps[core] >= outlif_entries * 3 &&
      block_size > 0 && port_replications <= 1 &&
      outlif_replications <= 2 && last_entry_pointer <= 1 &&
      port_entries + outlif_entries + port_replications + (outlif_replications ? 1 : 0) == block_size);
    DNX_MC_ASSERT(port_replications == 0 && outlif_replications == 0); /* not yet supported */
    for (left = port_entries; left; --left, ++index) { /* add port+outlif entries */
        rep = (*port_outlif_reps)++;
        DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_port_cud_noptr(
          unit, index, (*port_outlif_reps)++, rep, last_entry - index, prev)); /* write to hardware */
        prev = index;
    }
    for (left = outlif_entries; left; --left, ++index) { /* add outlif entries */
        rep = (*cud_reps)++;
        rep2 = (*cud_reps)++;
        DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_cud_noptr( unit, index, rep, rep2, (*cud_reps)++, last_entry - index, prev)); /* write to hardware */
        prev = index;
    }
    DNX_MC_ASSERT(index == block_start + block_size); /* verify we filled the given size */

exit:
    DNXC_FUNC_RETURN;
}

/*
 * This helper function writes an MCDB entry specifying the 3nd CUD of the following MCDB entries.
 * It only writes to the given structure, and does not write to hardware.
 * The next entry field/pointer is not provided, and should be set later externally to this function.
 */
static INLINE uint32 dnx_mult_eg_write_cud2_entry_struct(
    DNX_SAND_IN    int               unit,
    DNX_SAND_IN    uint8             cud2_type,   /* 2nd CUD type of the group/linked list */
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *cud2_entry, /* entry to write to */
    DNX_SAND_IN    uint32            cud2,        /* 2nd CUD value to write */
    DNX_SAND_IN    uint32            next_entry   /* the next entry */
) {
    DNXC_INIT_FUNC_DEFS;
    cud2_entry->word0 = cud2_type == DNX_MC_GROUP_2ND_CUD_OUTRIF ? 0x80000 : 0x40000;
    cud2_entry->word1 = 0x100001 ;
    soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, cud2_entry, OUTLIF_1f, cud2); /* set the 2nd CUD */
    soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, cud2_entry, LINK_PTRf, next_entry);

    SOC_EXIT;
exit:
  DNXC_FUNC_RETURN;
}

/*
 * This helper function writes a mcdb entry with a pointer,
 * according to preferred types and availability.
 */
static uint32 dnx_mult_eg_write_pointer_entry(
    DNX_SAND_IN    int unit,
    DNX_SAND_IN    uint32 index,                    /* the mcdb index to write to */
    DNX_SAND_IN    uint32 prev_entry,               /* the mcdb index of the previous entry (to link to only in the mcds) */
    DNX_SAND_IN    uint32 next_entry,               /* the mcdb index of the next entry which will be the pointer in the entry */
    DNX_SAND_INOUT uint16 *nof_port_couples,        /* The number of replication couples with the same outlif and with the first port < 127 moved to */
    DNX_SAND_INOUT uint16 *nof_remaining_ports,     /* The number of remaining port+outlif replications now at the end of the port+outlif replications */
    DNX_SAND_INOUT uint16 *nof_outlif_replications, /* The number of outlif only replications in the mcds */
    DNX_SAND_INOUT uint16 *nof_bitmap_replications, /* The number of bitmap+outlif only replications in the mcds */
    DNX_SAND_INOUT dnx_rep_data_t **couples_reps,   /* The port+CUD replication couples to work on */
    DNX_SAND_INOUT dnx_rep_data_t **port_cud_reps,  /* The port+CUD replications not in couples to work on */
    DNX_SAND_INOUT dnx_rep_data_t **cud_only_reps,  /* The CUD only replications to work on */
    DNX_SAND_INOUT dnx_rep_data_t **bitmap_reps     /* The bitmap+CUD replications to work on */
)
{
    dnx_rep_data_t *rep;
    DNXC_INIT_FUNC_DEFS;

  /* select entry type by checking availability according to entry type precedence */
    if (*nof_port_couples) { /* use a port couple if available */
        --*nof_port_couples;
        rep = (*couples_reps)++;
        DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_port_cud(unit, index, (*couples_reps)++, rep, next_entry, prev_entry));

    } else if (*nof_bitmap_replications) { /* use a bitmap replication if available */

         --*nof_bitmap_replications;
        DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_bm_cud(unit, index, (*bitmap_reps)++, next_entry, prev_entry));

    } else if (*nof_outlif_replications) { /* use outlif only replications if available */

        rep = (*cud_only_reps)++;
        if (*nof_outlif_replications >= 2) {
            DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_cud(unit, index, rep,(*cud_only_reps)++, next_entry, prev_entry));
            (*nof_outlif_replications) -= 2;
        } else {
            DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_cud(unit, index, rep, 0, next_entry, prev_entry));
            --*nof_outlif_replications;
        }

    } else if (*nof_remaining_ports) { /* use a port+outlif replication if available */

        --*nof_remaining_ports;
        if (SOC_IS_ARADPLUS_AND_BELOW(unit) && DNX_MCDS_REP_DATA_GET_EGR_PORT(*port_cud_reps) < JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID) {
            /* store the replication in the 2nd port of the format */
            DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_port_cud(unit, index, 0, (*port_cud_reps)++, next_entry, prev_entry));
        } else { /* store the replication in the 1st port of the format */
            DNXC_IF_ERR_EXIT(dnx_mult_eg_write_entry_port_cud(unit, index, (*port_cud_reps)++, 0, next_entry, prev_entry));
        }

    } else { /* It is a bug if code reached here, since an entry must be left both for this usage and for the start of the group */

      DNX_MC_ASSERT(0);
      DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("wrong function input")));

    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This helper function writes a mcdb entry with a pointer to a given jer2_arad_mcdb_entry_t structure,
 * according to preferred types and availability.
 * This function is used to construct the first entry of an egress multicast group.
 * Since it does not write directly to the mcds, it does not (and can not) write the previous entry mcds link.
 */
static uint32 dnx_mult_eg_write_pointer_entry_struct(
    DNX_SAND_IN    int unit,
    DNX_SAND_INOUT jer2_arad_mcdb_entry_t *entry,        /* entry to write to */
    DNX_SAND_IN    uint32 next_entry,               /* the mcdb index of the next entry which will be the pointer in the entry */
    DNX_SAND_IN    uint16 nof_reps_in_core,         /* The number of replications in the current core */
    DNX_SAND_INOUT uint16 *nof_port_couples,        /* The number of replication couples with the same outlif and with the first port < 127 moved to */
    DNX_SAND_INOUT uint16 *nof_remaining_ports,     /* The number of remaining port+outlif replications now at the end of the port+outlif replications */
    DNX_SAND_INOUT uint16 *nof_outlif_replications, /* The number of outlif only replications in the mcds */
    DNX_SAND_INOUT uint16 *nof_bitmap_replications, /* The number of bitmap+outlif only replications in the mcds */
    DNX_SAND_INOUT dnx_rep_data_t **couples_reps,   /* The port+CUD replication couples to work on */
    DNX_SAND_INOUT dnx_rep_data_t **port_cud_reps,  /* The port+CUD replications not in couples to work on */
    DNX_SAND_INOUT dnx_rep_data_t **cud_only_reps,  /* The CUD only replications to work on */
    DNX_SAND_INOUT dnx_rep_data_t **egr_bitmap_reps /* The bitmap+CUD replications to work on */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    dnx_rep_data_t *rep, *rep2;
    DNXC_INIT_FUNC_DEFS;

    /* select entry type by checking availability according to entry type precedence */
    if (*nof_port_couples) { /* use a port couple if available */

        --*nof_port_couples;
        rep = (*couples_reps)++;
        mcds->egr_mc_write_entry_port_cud(unit, entry, (*couples_reps)++, rep, next_entry);

    } else if (*nof_bitmap_replications) { /* use a bitmap replication if available */

        mcds->egr_mc_write_entry_bm_cud(unit, entry, (*egr_bitmap_reps)++, next_entry);
        --*nof_bitmap_replications;

    } else if (*nof_remaining_ports == 2 && nof_reps_in_core == 2)  {

        /* If all the replications are two port+outlif, and this is the last 2nd CUD in the core, they fit in one entry without a pointer */
        rep = (*port_cud_reps)++;
        mcds->egr_mc_write_entry_port_cud_noptr(unit, entry, rep, (*port_cud_reps)++, 0);
        *nof_remaining_ports = 0;

    } else if (*nof_outlif_replications == 3 && nof_reps_in_core == 3)  {

        /* If all the replications are three outlif only, and this is the last 2nd CUD in the core, they fit in one entry without a pointer */
        rep = (*cud_only_reps)++;
        rep2 = (*cud_only_reps)++;
        mcds->egr_mc_write_entry_cud_noptr(unit, entry, rep, rep2, (*cud_only_reps)++, 0);
        (*nof_outlif_replications) = 0;

    } else if (*nof_outlif_replications) { /* use outlif only replications if available */

        rep = (*cud_only_reps)++;
        if (*nof_outlif_replications >= 2) {
            mcds->egr_mc_write_entry_cud(unit, entry, rep, (*cud_only_reps)++, next_entry);
            (*nof_outlif_replications) -= 2;
        } else {
            mcds->egr_mc_write_entry_cud(unit, entry, rep, 0, next_entry);
            --*nof_outlif_replications;
        }

    } else if (*nof_remaining_ports) { /* use a port+outlif replication if available */

        --*nof_remaining_ports;
        if (SOC_IS_ARADPLUS_AND_BELOW(unit) && DNX_MCDS_REP_DATA_GET_EGR_PORT(*port_cud_reps) < JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID) {
            /* store the replication in the 2nd port of the format */
            mcds->egr_mc_write_entry_port_cud(unit, entry, 0, (*port_cud_reps)++, next_entry);
        } else { /* store the replication in the 1st port of the format */
            mcds->egr_mc_write_entry_port_cud(unit, entry, (*port_cud_reps)++, 0, next_entry);
        }

    } else { /* It is a bug if code reached here, since an entry must be left both for this usage and for the start of the group */

        DNX_MC_ASSERT(0);
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("wrong function input")));
    }

exit:
    DNXC_FUNC_RETURN;
}


/* compare two replications, and return 0 if they are exactly the same, non zero otherwise */
static INLINE uint32 compare_dnx_rep_data_t(const dnx_rep_data_t *rep1, const dnx_rep_data_t *rep2)
{
    return (rep1->base - rep2->base) | (rep1->extra - rep2->extra);
}

/* Exchange the contents of two dnx_mcds_base_t structs */
static INLINE void dnx_exchange_dnx_rep_data_t(dnx_rep_data_t *a, dnx_rep_data_t *b)
{
    dnx_rep_data_t temp = *a;
    *a = *b;
    *b = temp;
}


/*
 * Set a linked list of the input egress entries, possibly using a provided free block as the first allocation.
 * The replications are taken form the mcds.
 * If is_group_start is non zero, then list_prev is the (free and reserved) group start entry and it is set with replications.
 * Otherwise we do not handle the start of the egress group so there is no need for special handling of the first entry.
 * If the function fails, it will free the given allocated block.
 * In the start_block_index entry, link to the previous entry according to the previous entry of input_block_index.
 * On failure allocated entries are freed, including alloced_block_start.
 * Linked lists replaced by new linked lists are not freed by this function.
 */

uint32 dnx_mcds_set_egress_linked_list(
    DNX_SAND_IN  int                           unit,
    DNX_SAND_IN  uint8                         is_group_start,      /* specifies if list_prev is a group start to be filled or not */
    DNX_SAND_IN  uint32                        group_id,            /* If is_group_start this is the group ID, otherwise this is the entry preceding the single linked list to be created */
    DNX_SAND_IN  uint32                        list_end,            /* The entry that end of the created linked list will point to, Same one for all given cores */
    DNX_SAND_IN  uint32                        alloced_block_start, /* start index of an allocated block to use for the free list */
    DNX_SAND_IN  dnx_free_entries_block_size_t alloced_block_size,  /* size of the allocated block to use, should be 0 if none */
    DNX_SAND_IN  dnx_mc_core_bitmap_t          cores_to_set,        /* cores of linked lists to set */
    DNX_SAND_OUT uint32                        *list_start,         /* The first entry of the created linked list */
    DNX_SAND_OUT DNX_TMC_ERROR                 *out_err             /* return possible errors that the caller may want to ignore: insufficient memory or duplicate replications */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    int failed = 1;
    uint32 linked_lists[SOC_DNX_DEFS_MAX(NOF_CORES)] = {0}, linked_list_ends[SOC_DNX_DEFS_MAX(NOF_CORES)] = {0}; /* the linked lists created for each core */
    uint32 block_start = alloced_block_start;
    uint32 list_prev; /* The entry preceding the linked list to be created */
    uint32 prev_entry; /* The previous MCDB entry in the linked list */
    dnx_free_entries_block_size_t block_size = alloced_block_size;
    uint16 pointer_entry; /* number of pointer entries needed at the end of the block, must be 1 or 0 */
    jer2_arad_mcdb_entry_t start_entries[SOC_DNX_DEFS_MAX(NOF_CORES)];
    dnx_mc_core_bitmap_t core_bm;
    dnx_mc_core_bitmap_t non_zero_cores = DNX_MC_CORE_BITAMAP_NO_CORES; /* The cores whose ports have replications */
    dnx_mc_core_bitmap_t prev_cores_with_reps, new_cores_with_reps = 0; /* The cores whose ports previously had/now have replications */
    dnx_rep_data_t *rep, *start_of_cud_reps, *start_of_cud2_reps, *couples_dst, *rep_ptr, *rep_ptr2;
    jer2_arad_mcdb_entry_t *cud2_entry; /* entry pointer used to write a 2nd CUD entry in MCDB */
    /* pointers to the start of or to the current location at the block of continuous replications of the same type in sorted replications */
    dnx_rep_data_t *egr_port_outlif_reps, *couples_reps, *egr_outlif_reps, *egr_bitmap_reps;

    uint16 nof_reps_left_in_core;   /* The number of replications left to be processed in the current core */
    uint16 i, nof_small_ports;      /* number of port+CUD replications whose port is small enough to fit in any MCDB entry field (In Arad this limits to port < 127), or index */
    uint16 nof_couples;             /* number of replication couples with the same outlif suitable for one MCDB entry with a link pointer, in the current 2nd CUD */
    uint16 nof_couples_in_cud;      /* number of replication couples with the same outlif suitable for one MCDB entry with a link pointer, in the current CUD */
    uint16 nof_remaining_ports;     /* The number of remaining port+CUD replications in the current 2nd CUD */
    uint16 nof_cud_remaining_ports; /* The number of remaining port+CUD replications in the current CUD */
    uint16 nof_port_cud_reps;       /* The number of port+CUD replications in the current 2nd CUD */
    uint16 nof_port_cud_reps_in_cud;/* The number of port+CUD replications in the current CUD */
    uint16 nof_outlif_replications; /* The number of outlif only replications in the current 2nd CUD */
    uint16 nof_bitmap_replications; /* The number of bitmap+outlif only replications in the current 2nd CUD */
    uint16 nof_cud_reps_in_core, nof_port_cud_reps_in_core, nof_bitmap_reps_in_core; /* number of each type of replication found in the core */
    uint32 core, cud = 0, prev_cud, cud2 = DNX_MC_NO_2ND_CUD;
    uint32 max_low_port_for_couple = SOC_IS_JERICHO(unit) ? DNX_MULT_EGRESS_PORT_INVALID : JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID;
    int end_of_cud2, is_port_cud_rep, created_linked_list_start;

    DNXC_INIT_FUNC_DEFS;
    DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) { /* init for cleanup on error */
        linked_lists[core] = list_end;
    }
    DNXC_NULL_CHECK(out_err);
    DNX_MC_ASSERT(core <= SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores);
    *out_err = _SHR_E_NONE;

    if (mcds->nof_ingr_reps != 0) {
        DNX_MC_ASSERT(0);
        DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("ingress replications exist")));
    }

    /* create the new linked lists, one per core, using this methodology: */
    /* per core: {sort core's replicastions, per 2nd CUD: {process CUD2 replications including loop per CUD} create the core's linked list} */
    DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) {
        /* init the linked list created so far to empty */
        linked_list_ends[core] = prev_entry = list_prev = is_group_start ? DNX_MCDS_GET_EGRESS_GROUP_START(mcds, group_id, core) : group_id;
        pointer_entry = list_end == DNX_MC_EGRESS_LINK_PTR_END ? 0 : 1; /* number of pointer entries needed at the end of the block, must be 1 or 0 */

        nof_reps_left_in_core = mcds->nof_reps[core];
        rep = mcds->reps + DNX_MCDS_GET_REP_INDEX(core, 0);
        if (nof_reps_left_in_core) {
            non_zero_cores |= DNX_MC_CORE_BITAMAP_CORE_0 << core; /* mark that the core has replications for core replication */
            /* sorting is always needed for separating replications by 2nd CUD and for finding port+outlif couples */
            dnx_sand_os_qsort(rep, nof_reps_left_in_core, sizeof(dnx_rep_data_t), dnx_rep_data_t_compare);
        }

        if (!(SOC_DNX_CONFIG(unit)->tm.mc_mode & DNX_MC_ALLOW_DUPLICATES_MODE)) { /* need to check duplicate replications */
            for (i = nof_reps_left_in_core; i > 1; --i) {
                rep_ptr = rep+1;
                if (!compare_dnx_rep_data_t(rep, rep_ptr)) {
                    *out_err = _SHR_E_PARAM;
                    SOC_EXIT; /* We found duplicate replications in the input */
                }
                rep = rep_ptr;
            }
            rep = mcds->reps + DNX_MCDS_GET_REP_INDEX(core, 0);
        }

        /* loop over the 2nd CUDs of the replications of this core, for each process the replications and add them to HW */
        rep = mcds->reps + DNX_MCDS_GET_REP_INDEX(core, 0);
        end_of_cud2 = nof_reps_left_in_core == 0;
        nof_cud_reps_in_core = nof_port_cud_reps_in_core = nof_bitmap_reps_in_core = 0;
        do { /* loop over the 2nd CUD */

            cud2 = end_of_cud2 ? DNX_MC_NO_2ND_CUD : DNX_MCDS_REP_DATA_GET_EXTRA_CUD(rep);
            couples_dst = start_of_cud2_reps = couples_reps = rep;
            nof_couples = nof_remaining_ports = nof_port_cud_reps = 0;

            /* Process the sorted port+CUD replications to find replication couples suitable for format 0 */
            if ((is_port_cud_rep = !end_of_cud2 &&  DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD)) {
                /* loop over each CUD of the port+CUD replications, this type of replications are at the start due to the sort */
                for (prev_cud = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep); is_port_cud_rep; prev_cud = cud) {
                    start_of_cud_reps = rep;
                    nof_small_ports = nof_cud_remaining_ports = 0;
                    do { /* loop over port+CUD replications with the same CUD */
                        if (DNX_MCDS_REP_DATA_GET_EGR_PORT(rep) < max_low_port_for_couple) {
                            ++nof_small_ports; /* count the small port replications which must be at the start of the replications in the loop */
                        } else {
                            ++nof_cud_remaining_ports; /* count the other replications which must be at the end of the replications in the loop */
                        }
                        ++rep;
                        if (--nof_reps_left_in_core == 0 || DNX_MCDS_REP_DATA_GET_EXTRA_CUD(rep) != cud2) {
                            end_of_cud2 = 1;
                            is_port_cud_rep = 0;
                            break;
                        }
                        if (!(is_port_cud_rep = DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_EGR_PORT_CUD)) {
                            break;
                        }
                    } while ((cud = DNX_MCDS_REP_DATA_GET_EGR_CUD(rep)) == prev_cud);

                    /* We have now advanced till after all the port+CUD replication with the same CUD, and will now process couples */
                    /* We will move replication couples suitable for the same MCDB entry to the start of the replications of the current 2nd CUD */
                    nof_couples_in_cud = (nof_port_cud_reps_in_cud = nof_small_ports + nof_cud_remaining_ports) / 2;
                    if (nof_couples_in_cud > nof_small_ports) {
                        nof_couples_in_cud = nof_small_ports; /* number of couples found = min{nof_small_ports, (nof_small_ports + nof_cud_remaining_ports) / 2} */
                    }
                    nof_cud_remaining_ports = nof_port_cud_reps_in_cud - 2 * nof_couples_in_cud;
                    /* reverse the order of the CUD's port+CUD replication, so they will be sorted by port */
                    DNX_MC_ASSERT(nof_port_cud_reps_in_cud == rep - start_of_cud_reps);
                    for (rep_ptr = start_of_cud_reps, rep_ptr2 = rep - 1; rep_ptr2 > rep_ptr;) {
                        dnx_exchange_dnx_rep_data_t(rep_ptr++, rep_ptr2--);
                    }
                        
                    /* move small port replications so that all couples will have a small port, as needed for the MCDB format */
                    i = (nof_small_ports + 1)/ 2; /* calculate the max possible number of couple that already have a small port */
                    if (nof_couples_in_cud > i) {
                        i = nof_couples_in_cud - i; /* From here i stores the number of couples currently without any small port */
                        rep_ptr = start_of_cud_reps + 1; /* CUD replications to swap small ports from: 1, 3, ... */
                        rep_ptr2 = start_of_cud_reps + (nof_couples_in_cud - 1) * 2; /* CUD replications to swap to: nof_couples_in_cud*2-2, nof_couples_in_cud*2-4, ... */
                        do { /* make the first replication of all couples be one with a small port, making the couples suitable for an MCDB entry */
                            dnx_exchange_dnx_rep_data_t(rep_ptr, rep_ptr2);
                            rep_ptr += 2;
                            rep_ptr2 -= 2;
                        } while (--i);
                        DNX_MC_ASSERT(rep_ptr < rep_ptr2 + 2);
                    }

                    if (couples_dst != start_of_cud_reps) {
                        for (i = nof_couples_in_cud * 2, rep_ptr = start_of_cud_reps; i; --i) { /* move the replication couples to the couples at the start of the 2nd CUD replications */
                            dnx_exchange_dnx_rep_data_t(couples_dst++, rep_ptr++);
                        }
                    } else {
                        couples_dst += nof_couples_in_cud * 2; /* The couples are in the correct location, only move the coupe pointer after their end */
                    }
                    nof_couples += nof_couples_in_cud;
                    nof_remaining_ports += nof_cud_remaining_ports;
                    nof_port_cud_reps += nof_port_cud_reps_in_cud;
                }

            }
            i = nof_couples * 2;
            egr_port_outlif_reps = nof_remaining_ports ? couples_reps + i : NULL;
            if (nof_couples == 0) {
                couples_reps = NULL;
            }
            DNX_MC_ASSERT(nof_port_cud_reps == rep - start_of_cud2_reps && nof_port_cud_reps == nof_remaining_ports + i && i == couples_dst - start_of_cud2_reps);

            /* process CUD only replications */
            rep_ptr = rep;
            while (!end_of_cud2 && DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_EGR_CUD) {
                ++rep;
                if (--nof_reps_left_in_core == 0 || DNX_MCDS_REP_DATA_GET_EXTRA_CUD(rep) != cud2) {
                    end_of_cud2 = 1;
                }
            }
            nof_outlif_replications = rep - rep_ptr;
            egr_outlif_reps = nof_outlif_replications ? rep_ptr : NULL;

            /* process bitmap+CUD replications */
            rep_ptr = rep;
            while (!end_of_cud2 && DNX_MCDS_REP_DATA_GET_TYPE(rep) == DNX_MCDS_REP_TYPE_EGR_BM_CUD) {
                ++rep;
                if (--nof_reps_left_in_core == 0 || DNX_MCDS_REP_DATA_GET_EXTRA_CUD(rep) != cud2) {
                    end_of_cud2 = 1;
                }
            }
            nof_bitmap_replications = rep - rep_ptr;
            egr_bitmap_reps = nof_bitmap_replications ? rep_ptr : NULL;

            DNX_MC_ASSERT(end_of_cud2 && nof_outlif_replications + nof_port_cud_reps + nof_bitmap_replications == rep - start_of_cud2_reps);
            nof_cud_reps_in_core += nof_outlif_replications;
            nof_port_cud_reps_in_core += nof_port_cud_reps;
            nof_bitmap_reps_in_core += nof_bitmap_replications;
            /* Finished processing the replications of one CUD2, and will now add them to the HW linked list */

            /* Do we need to create the first entry of the linked list */
            if ((created_linked_list_start = is_group_start && nof_reps_left_in_core == 0)) {
                /* build first group entry for writing after building all the linked list */
                if (cud2 != DNX_MC_NO_2ND_CUD) { /* If a 2nd CUD was specified, the first MCDB entry we create will be one specifying the 2nd CUD */
                    DNX_MC_ASSERT(SOC_IS_JERICHO(unit) && (mcds->info_2nd_cud == DNX_MC_GROUP_2ND_CUD_EEI || mcds->info_2nd_cud == DNX_MC_GROUP_2ND_CUD_OUTRIF));
                    /* write the needed hardware fields to the linked list start structure */
                    dnx_mult_eg_write_cud2_entry_struct(unit, mcds->info_2nd_cud, start_entries + core, cud2, DNX_MC_EGRESS_LINK_PTR_END);
                } else { /* we have yet to create a linked list start and this is the default or last 2nd CUD */
                    if (nof_couples + nof_remaining_ports + nof_outlif_replications + nof_bitmap_replications == 0) { /* handle empty group */
                        start_entries[core].word0 = mcds->free_value[0];
                        start_entries[core].word1 = mcds->free_value[1];
                        DNX_MC_ASSERT(mcds->nof_reps[core] == 0);
                    } else { /* set first group entry for a non empty group */
                        start_entries[core].word1 = start_entries[core].word0 = 0;
                        DNXC_IF_ERR_EXIT(dnx_mult_eg_write_pointer_entry_struct(
                          unit, start_entries + core, DNX_MC_EGRESS_LINK_PTR_END, mcds->nof_reps[core],
                          &nof_couples, &nof_remaining_ports, &nof_outlif_replications, &nof_bitmap_replications,
                          &couples_reps, &egr_port_outlif_reps, &egr_outlif_reps, &egr_bitmap_reps));
                    }
                }
            }

            /* build the linked list, loop while we have remaining replications not written */
            while (nof_couples + nof_remaining_ports + nof_outlif_replications + nof_bitmap_replications) {
                uint16 port_outlif_full_entries = nof_remaining_ports / 2; /* number of port+outlif entries that can be created with no pointers */
                uint16 outlif_full_entries = nof_outlif_replications / 3; /* number of outlif entries that can be created with no pointers */
                uint16 net_block_size = port_outlif_full_entries + outlif_full_entries ; /* the size we want for our block without a pointer entry */
                uint16 full_block_size; /* the block size including the terminating pointer entry */
                uint32 index = -1;

                if (!net_block_size) { /* if we have no entry suited for a block, we always need a pointer entry */
                    pointer_entry = 1;
                } else if (pointer_entry && /* we have replications suitable for a block terminated by a pointer entry (not at the group end) */
                    nof_couples + nof_bitmap_replications == 0 &&                /* If we have no available pointer entries to create, */
                    (nof_remaining_ports % 2) + (nof_outlif_replications % 3) == 0) { /* and no reminder replications from full non pointer entries */
                    /* Try to "convert" entries that can be non pointer to pointer */
                    --net_block_size;
                    if (outlif_full_entries) {
                        --outlif_full_entries;
                    } else {
                        DNX_MC_ASSERT(port_outlif_full_entries);
                        --port_outlif_full_entries;
                    }
                }

                full_block_size = net_block_size + pointer_entry; /* calculate size of entries block to write */
                if (full_block_size >= DNX_MCDS_MAX_FREE_BLOCK_SIZE) {
                    full_block_size = DNX_MCDS_MAX_FREE_BLOCK_SIZE;
                    net_block_size = DNX_MCDS_MAX_FREE_BLOCK_SIZE - pointer_entry;
                }
                DNX_MC_ASSERT(full_block_size && full_block_size <= DNX_MCDS_MAX_FREE_BLOCK_SIZE && (net_block_size || full_block_size == 1));

                if (!block_size) { /* no available MCDB entries, we need to allocate */
                    DNXC_IF_ERR_EXIT(dnx_mcds_get_free_entries_block( /* allocate free entries */
                      mcds, DNX_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS | DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL,
                      full_block_size, full_block_size, &block_start, &block_size));
                    if (!block_size) { /* could not get free entries */
                        *out_err = _SHR_E_FULL;
                        SOC_EXIT; /* will free the linked list allocated so far */
                    }
                    DNX_MC_ASSERT(block_size <= full_block_size);
                }

                net_block_size = block_size - pointer_entry; /* Adjust number of no pointer entries for a possibly smaller block size that was allocated */
                if (net_block_size) { /* create a block */
                    uint16 block_port_outlif_entries, block_outlif_entries; /* number of entries actually used in the block */
                    full_block_size = block_size;

                    if (net_block_size < port_outlif_full_entries) { /* calculate how many entries of each type are included in the block */
                        block_port_outlif_entries = net_block_size;
                        block_outlif_entries = 0;
                    } else {
                        block_port_outlif_entries = port_outlif_full_entries;
                        block_outlif_entries = net_block_size - port_outlif_full_entries;
                  
                    }
                    DNX_MC_ASSERT(block_port_outlif_entries +  block_outlif_entries + pointer_entry == block_size &&
                                  port_outlif_full_entries >= block_port_outlif_entries &&
                                  outlif_full_entries >= block_outlif_entries &&
                                  nof_remaining_ports >= block_port_outlif_entries * 2 &&
                                  nof_outlif_replications >= block_outlif_entries * 3);
                    nof_remaining_ports -= block_port_outlif_entries * 2; /* update remaining replication counts */
                    nof_outlif_replications -= block_outlif_entries * 3;
            
                    DNXC_IF_ERR_EXIT(dnx_mult_eg_add_group_block( /* write the block, excluding the last entry if it is a pointer */
                      unit, core, block_start, net_block_size, pointer_entry, block_port_outlif_entries, block_outlif_entries,
                      0, 0, &egr_port_outlif_reps, &egr_outlif_reps));

                    DNX_MC_ASSERT(port_outlif_full_entries >= block_port_outlif_entries && outlif_full_entries >= block_outlif_entries);
                    port_outlif_full_entries -= block_port_outlif_entries;
                    outlif_full_entries -= block_outlif_entries;

                    index = block_start + net_block_size;
                    prev_entry = index - 1;
                    /* finished creating the block non pointer entries */
                } else {
                    full_block_size = 1;
                    index = block_start;
                    DNX_MC_ASSERT(pointer_entry == 1);
                }

                if (pointer_entry) { /* This is not a block or a block ending in a pointer entry which we need to write */
                    DNXC_IF_ERR_EXIT(dnx_mult_eg_write_pointer_entry( /* write entry index with a pointer to linked_list */
                      unit, index, prev_entry, linked_lists[core],
                      &nof_couples, &nof_remaining_ports, &nof_outlif_replications, &nof_bitmap_replications,
                      &couples_reps, &egr_port_outlif_reps, &egr_outlif_reps, &egr_bitmap_reps));
                } else { /* We created a block not ending in a link pointer, but rather ending at the end of the linked list */
                    pointer_entry = 1;
                    DNX_MC_ASSERT(linked_lists[core] == list_end);
                }

                DNX_MC_ASSERT(block_size >= full_block_size);
                if (linked_lists[core] != list_end) { /* set software back link from the previously generated linked list (in previous iterations) to the last entry in this block */
                    DNX_MCDS_SET_PREV_ENTRY(mcds, linked_lists[core], block_start + full_block_size - 1);
                } else {
                    linked_list_ends[core] = block_start + full_block_size - 1;
                }

                linked_lists[core] = block_start; /* Mark the current block/entry as the start of the linked list */
                if (block_size -= full_block_size) { /* update the part of the current block that we used */
                    block_start += full_block_size;
                }

            } /* finished creating the linked list of the current 2nd CUD entries */
            DNX_MC_ASSERT(nof_couples + nof_remaining_ports + nof_outlif_replications + nof_bitmap_replications == 0 && !block_size);

            if (!created_linked_list_start && cud2 != DNX_MC_NO_2ND_CUD) {
                /* we need to add a 2nd CUD MCDB entry if it was not created at the start of the list and cud2!=0 */
                if (!block_size) { /* no available MCDB entries, we need to allocate */
                    DNXC_IF_ERR_EXIT(dnx_mcds_get_free_entries_block( /* allocate free entries */
                      mcds, DNX_MCDS_GET_FREE_BLOCKS_PREFER_INGRESS | DNX_MCDS_GET_FREE_BLOCKS_DONT_FAIL, 1, 1, &block_start, &block_size));
                    if (!block_size) { /* could not get free entries */
                        *out_err = _SHR_E_FULL;
                        SOC_EXIT; /* will free the linked list allocated so far */
                    }
                    DNX_MC_ASSERT(block_size == 1);
                }
                cud2_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, block_start);
                dnx_mult_eg_write_cud2_entry_struct(unit, mcds->info_2nd_cud, cud2_entry, cud2, linked_lists[core]); /* write the needed hardware fields to the entry structure */
                DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, block_start)); /* write to hardware */
                DNX_MCDS_ENTRY_SET_TYPE(cud2_entry, DNX_MCDS_TYPE_VALUE_EGRESS);
                if (linked_lists[core] != list_end) { /* set software back link from the previously generated linked list (of previous 2nd CUD) to the 2nd CUD entry */
                    DNX_MCDS_SET_PREV_ENTRY(mcds, linked_lists[core], block_start);
                } else {
                    linked_list_ends[core] = block_start;
                }
                prev_entry = linked_lists[core] = block_start; /* Mark the 2nd CUD entry as the current start of the linked list */
                --block_size;
                ++block_start;
            }

            end_of_cud2 = 0; /* mark that the iterations of the next 2nd CUD are not finished for the next iteration */
        } while (nof_reps_left_in_core != 0); /* finished looping over the 2nd CUDs (and the replications) of the core */

        DNX_MC_ASSERT( /* there must not be unprocessed replications belonging to this 2nd CUD */
          nof_cud_reps_in_core == mcds->nof_egr_outlif_reps[core] && nof_bitmap_reps_in_core == mcds->nof_egr_bitmap_reps[core] &&
          nof_port_cud_reps_in_core == mcds->nof_egr_port_outlif_reps[core] &&
          nof_port_cud_reps_in_core + nof_cud_reps_in_core + nof_bitmap_reps_in_core == mcds->nof_reps[core] &&
          mcds->nof_reps[core] + DNX_MCDS_GET_REP_INDEX(core, 0) == rep - mcds->reps &&
          mcds->nof_reps[core] <= DNX_MULT_MAX_EGRESS_REPLICATIONS);
    }

    /* If we are successful so far, and allocated everything, finalize the group by writing one entry per linked list/core */
    list_prev = group_id;
    if (is_group_start) { /* handle replication to egress cores */
        DNXC_IF_ERR_EXIT(dnx_egress_group_get_core_replication(unit, group_id, &prev_cores_with_reps)); /* get egress cores replicated to */
        new_cores_with_reps = (non_zero_cores | ~cores_to_set) & prev_cores_with_reps; /* stop replication to cores who lost all their port replications before changing the linked lists */
        if (new_cores_with_reps != prev_cores_with_reps) { /* Do we have to disable cores? */
            DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, group_id, new_cores_with_reps));
            prev_cores_with_reps = new_cores_with_reps;
        }
        new_cores_with_reps |= non_zero_cores; /* the final core replication value with cores that are newly replicated to */
    }
    DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) {
        /* now connect the written linked list to its preceding and succeeding (if any) entries */
        if (is_group_start) { /* build first group entry for writing after building all the linked list */
            jer2_arad_mcdb_entry_t *mcdb_entry;
            list_prev = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, group_id, core);
            mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, list_prev);
            mcdb_entry->word0 = start_entries[core].word0; /* copy previously filled hardware fields (next pinter not set) */
            mcdb_entry->word1 &= DNX_MCDS_WORD1_KEEP_BITS_MASK;
            /* coverity[uninit_use:FALSE] */
            mcdb_entry->word1 |= start_entries[core].word1;

            DNX_MCDS_SET_PREV_ENTRY(mcds, list_prev, list_prev); /* set software link to previous entry */
            DNX_MCDS_ENTRY_SET_TYPE(mcdb_entry, DNX_MCDS_TYPE_VALUE_EGRESS_START); /* set type */
        }
        if (linked_lists[core] != list_end) { /* set software back link from the start of the created linked list to the previous entry */
            DNX_MCDS_SET_PREV_ENTRY(mcds, linked_lists[core], list_prev);
        }
        DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the entry preceding the linked list */
          mcds, unit, list_prev, DNX_MCDS_TYPE_VALUE_EGRESS_START, linked_lists[core]));
        linked_lists[core] = list_end; /* prevent the freeing of the linked list on error */

        if (list_end != DNX_MC_EGRESS_LINK_PTR_END) { /* if group continues, link the continuation to the created linked list */
            DNX_MCDS_SET_PREV_ENTRY(mcds, list_end, linked_list_ends[core]);
        }
    }

    if (is_group_start && new_cores_with_reps != prev_cores_with_reps) { /* Do we have to enable replication to egress cores? */
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, group_id, new_cores_with_reps));
    }

    failed = 0;
exit:
    if (failed) { /* handle cleanup of allocated entries that were not used due to function failure */
        DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) {
            if (linked_lists[core] != list_end) { /* free the linked list allocated so far on error or when running out of mcdb entries */
                  dnx_mcdb_free_linked_list_till_my_end(unit, linked_lists[core], DNX_MCDS_TYPE_VALUE_EGRESS, list_end);
            }
        }
        if (block_size) { /* free the current block on error */
            DNX_MC_ASSERT(failed || (block_start >= alloced_block_start && block_start < alloced_block_start + alloced_block_size));
            dnx_mcds_build_free_blocks(unit, mcds,
              block_start, block_start + block_size - 1, dnx_mcds_get_region(mcds, block_start),
              block_start == alloced_block_start ? dnxMcdsFreeBuildBlocksAddAll : dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed);
        }
    }
    DNXC_FUNC_RETURN;
}


/*
 * Sets the egress group to the given replications configuring its linked list.
 * If the group does not exist, it will be created or an error will be returned based on allow_create.
 * Creation may involve relocating the MCDB entry which will be the start
 * of the group, and possibly other consecutive entries.
 *
 * We always want to create entries with pointers from port+outlif couples and from bitmaps.
 * We need to leave one entry with a pointer for the start of the group.
 * every block of entries with no pointers ends with an entry pointer, except for the end of the group.
 */
uint32 dnx_mult_eg_group_set(
    DNX_SAND_IN  int                  unit,
    DNX_SAND_IN  dnx_mc_id_t          mcid,         /* the group mcid */
    DNX_SAND_IN  uint8                allow_create, /* if non zero, will create the group if it does not exist */
    DNX_SAND_IN  uint32               group_size,   /* size of ports and cuds to read group replication data from */
    DNX_SAND_IN  dnx_mc_replication_t *reps,        /* input array containing replications (using logical ports) */
    DNX_SAND_OUT DNX_TMC_ERROR        *out_err      /* return possible errors that the caller may want to ignore */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint8 group_exists = 0;
    int failed = 1, core;
    uint32 entry_type, created_list, group_entry_id;
    uint32 old_group_entries[SOC_DNX_DEFS_MAX(NOF_CORES)] = {DNX_MC_EGRESS_LINK_PTR_END}; /* the linked lists (per core) of the previous group content */
    const dnx_mc_core_bitmap_t cores_to_set = DNX_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit);
    dnx_mc_core_bitmap_t core_bm, group_start_alloced_bm = DNX_MC_CORE_BITAMAP_NO_CORES;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(out_err);


    DNX_MC_ASSERT(mcid < SOC_DNX_CONFIG(unit)->tm.nof_mc_ids);
    DNXC_IF_ERR_EXIT(dnx_egress_group_open_get(unit, mcid, &group_exists));
    if (!group_exists && !allow_create) {
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }
    DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) { /* For each core handle its existing linked list or its first linked list entries */
        group_entry_id = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid, core);
        entry_type = DNX_MCDS_GET_TYPE(mcds, group_entry_id);
        DNX_MC_ASSERT(group_exists == DNX_MCDS_TYPE_IS_EGRESS_START(entry_type));

        /* If this is a new group, we need to reserve its first entry, and possibly relocate the entry there */
        if (!group_exists && group_entry_id != DNX_MCDS_INGRESS_LINK_END(mcds)) {
            if (DNX_MCDS_TYPE_IS_USED(entry_type)) { /* relocate conflicting entries if needed */
                DNXC_IF_ERR_EXIT(dnx_mcdb_relocate_entries(unit, group_entry_id, 0, 0, out_err));
                if (*out_err) { /* If failed due to lack of memories, do the same */
                    SOC_EXIT;
                }
            } else { /* just allocate the start entry of the group */
                DNXC_IF_ERR_EXIT(dnx_mcds_reserve_group_start(mcds, group_entry_id));
                DNX_MCDS_SET_TYPE(mcds, group_entry_id, DNX_MCDS_TYPE_VALUE_EGRESS_START); /* mark the first group entry as the start of an ingress group */
            }
            group_start_alloced_bm |= DNX_MC_CORE_BITAMAP_CORE_0 << core;
        }

        if (group_exists) { /* for an existing group store its linked list (to be freed) at old_group_entries */
            DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds, unit, group_entry_id, DNX_MCDS_TYPE_VALUE_EGRESS, old_group_entries + core));
        }
    }

    DNXC_IF_ERR_EXIT(dnx_mcds_copy_replications_from_arrays(unit, 1, 1, group_size, reps)); /* copy group replications to mcds */

    DNXC_IF_ERR_EXIT(mcds->set_egress_linked_list( /* build the group, including the first entry */
      unit, TRUE, mcid, DNX_MC_EGRESS_LINK_PTR_END, 0, 0, cores_to_set, &created_list, out_err));
    if (*out_err) { /* If failed due to lack of memories, do the same */
        SOC_EXIT;
    }

    if (group_exists) { /* for an existing group free its previous linked lists */
        DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) {
            if (old_group_entries[core] != DNX_MC_EGRESS_LINK_PTR_END) { /* free previous linked list of the group not used any more */
                DNXC_IF_ERR_EXIT(dnx_mcdb_free_linked_list(unit, old_group_entries[core], DNX_MCDS_TYPE_VALUE_EGRESS));
            }
        }
    } else {
        DNXC_IF_ERR_EXIT(dnx_egress_group_open_set(unit, mcid, 1));
    }

    failed = 0;
exit:
    if (group_start_alloced_bm && failed) { /* free linked list start entries if needed */
        DNX_MC_ASSERT(!group_exists);
        DNX_MC_FOREACH_CORE(group_start_alloced_bm, core_bm, core) {
            group_entry_id = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid, core);
            dnx_mcds_build_free_blocks(unit, mcds, group_entry_id, group_entry_id,
              dnx_mcds_get_region(mcds, group_entry_id), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed);
        }
    }
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     This procedure configures the range of values of the
*     multicast ids entry points that their multicast groups
*     are to be found according to a bitmap (as opposed to a
*     Link List). Only the max bitmap ID is configurable.
*********************************************************************/
uint32 dnx_mult_eg_bitmap_group_range_set(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
)
{
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);
  if (info->mc_id_high >= DNX_MC_EGR_NOF_BITMAPS || info->mc_id_low)
  {
    DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid bitmap range")));
  }

  DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_modify(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, SOC_CORE_ALL, 0, EGRESS_REP_BITMAP_GROUP_VALUE_TOPf,  info->mc_id_high));

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     This procedure configures the range of values of the
*     multicast ids entry points that their multicast groups
*     are to be found according to a bitmap (as opposed to a
*     Link List).
*     Details: in the H file. (search for prototype)
*********************************************************************/
uint32
  dnx_mult_eg_bitmap_group_range_get(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_OUT DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE *info
)
{
  uint32 max_direct_bitmap_group;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(info);
  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP_RANGE_clear(info);

  /* Always zero (like Petra-B), no register to set */
  info->mc_id_low = 0;

  DNXC_IF_ERR_EXIT(soc_reg_above_64_field32_read(unit, EGQ_EGRESS_REPLICATION_GENERAL_CONFIGr, SOC_CORE_ALL, 0, EGRESS_REP_BITMAP_GROUP_VALUE_TOPf, &max_direct_bitmap_group));
  info->mc_id_high = max_direct_bitmap_group;

exit:
  DNXC_FUNC_RETURN;
}

/*********************************************************************
*     This API closes egress-multicast-replication group for
*     the specific multicast-id. The user only specifies the
*     multicast-id. All inner link-list/bitmap nodes are freed
*     and handled by the driver
*********************************************************************/
uint32 dnx_mult_eg_group_close(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t mcid
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    uint32 group_entry_id, core;
    const dnx_mc_core_bitmap_t cores_to_set = DNX_MC_CORE_BITAMAP_ALL_ACTIVE_CORES(unit);
    dnx_mc_core_bitmap_t core_bm;
    DNX_TMC_ERROR internal_err;
    uint8 does_exist;
    DNXC_INIT_FUNC_DEFS;

    DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, mcid, TRUE, &does_exist));
    if (does_exist) { /* do nothing if not open */
        if (mcid <= SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high) { /* this is a vlan bitmap group */

            DNXC_IF_ERR_EXIT(dnx_mult_eg_bitmap_group_close(unit, mcid));

        } else { /* not a bitmap group */

            DNXC_IF_ERR_EXIT(dnx_mult_eg_group_set( /* empty the group */
              unit, mcid, FALSE, 0, 0, &internal_err));
            if (internal_err) { /* should never require more entries, so if this happens it is an internal error */
                DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("too many entries")));
            }

            DNX_MC_FOREACH_CORE(cores_to_set, core_bm, core) { /* For each core handle its existing linked list or its first linked list entries */
                group_entry_id = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, mcid, core);
                if (group_entry_id != DNX_MCDS_INGRESS_LINK_END(mcds)) { /* free the group's starting entry, this also marks the group as closed */
                    DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks(unit, mcds, group_entry_id, group_entry_id,
                      dnx_mcds_get_region(mcds, group_entry_id), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
                } else { /* treat the non allocable last entry specifically */
                    jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, DNX_MCDS_INGRESS_LINK_END(mcds));
                    mcdb_entry->word0 = mcds->free_value[0];
                    mcdb_entry->word1 &= DNX_MCDS_WORD1_KEEP_BITS_MASK;
                    mcdb_entry->word1 |= mcds->free_value[1];
                    DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, DNX_MCDS_INGRESS_LINK_END(mcds))); /* write to hardware */
                }
            }

            DNXC_IF_ERR_EXIT(dnx_egress_group_open_set(unit, mcid, 0));
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * If the given egress linked list group contains the given replication,
 * then remove the replication from the linked list. If it is the only
 * replication in the linked list entry, the entry is removed from the group.
 * Does not support the TDM format.
 */
static uint32 dnx_mcds_remove_replications_from_egress_group(
    DNX_SAND_IN int         unit,        /* device */
    DNX_SAND_IN int         core,        /* core of the linked list */
    DNX_SAND_IN dnx_mc_id_t group_mcid,  /* group mcid */
    DNX_SAND_IN uint32      dest,        /* The destination of the replication to be removed */
    DNX_SAND_IN uint32      cud,         /* The CUD of the replication to be removed */
    DNX_SAND_IN uint32      cud2         /* The 2nd CUD of the replication to be removed */
)
{
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    const uint32 entry_index = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, group_mcid, core); /* table index of the first entry in the group */
    uint32 freed_index = entry_index; /* entry to be freed, the entry_index value means: can not free the entry */
    jer2_arad_mcdb_entry_t *mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, entry_index);
    uint32 format = 0, next_index = entry_index, cur_index = 0, prev_index = entry_index, bitmap = -1, cud2_field;
    int is_cud_only_replication = 0, is_port_cud_replication = 0, is_bitmap_cud_replication = 0;
    int remove_entry = 0, found = 0, correct_cud2;
    uint16 entries_left = DNX_MULT_MAX_EGRESS_REPLICATIONS; /* If we loop over more iterations this this it is certainly an error */
    DNXC_INIT_FUNC_DEFS;
    DNX_MC_ASSERT(DNX_MCDS_TYPE_VALUE_EGRESS_START == DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */

    /* process the destination to search for and delete */
    if (dest == _SHR_GPORT_INVALID) { /*outlif only, or port+outlif replication */
        is_cud_only_replication = 1;
        if (cud == DNX_MC_EGR_OUTLIF_DISABLED) { /* Invalid value, used to mark no replication in hardware */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid replication to delete")));
        }
    } else if (dest & JER2_ARAD_MC_EGR_IS_BITMAP_BIT) { /* bitmap + outlif replication */
        bitmap = dest & ~JER2_ARAD_MC_EGR_IS_BITMAP_BIT;
        is_bitmap_cud_replication = 1;
        if (bitmap == DNX_MC_EGR_OUTLIF_DISABLED) { /* Invalid value, used to mark no replication in hardware */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid replication to delete")));
        }
    } else {
        is_port_cud_replication = 1;
        if (dest == DNX_MULT_EGRESS_PORT_INVALID) { /* Invalid value, used to mark no replication in hardware */
            DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("invalid replication to delete")));
        }
    }

    if (SOC_IS_JERICHO(unit)) { /* handle Jericho */

        correct_cud2 = (cud2_field = cud2 & ~DNX_MC_2ND_CUD_IS_EEI) == DNX_MC_NO_2ND_CUD;
        for (; entries_left; --entries_left) { /* loop over the linked list */
            cur_index = next_index;
            next_index = DNX_MC_EGRESS_LINK_PTR_END;
            format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, ENTRY_FORMATf);
            switch (format) {
              /* select memory format based on the format type */
              case 0: /* Entry containing two port+CUD replications sharing the same CUD */
                next_index = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, LINK_PTRf);
                if (correct_cud2 && is_port_cud_replication && soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f) == cud) {
                    uint32 port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Af);
                    if (dest == port) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Af, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Bf) == DNX_MULT_EGRESS_PORT_INVALID) {
                            remove_entry = 1;
                        }
                    } else if (dest == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Bf)) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, PP_DSP_1Bf, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (port == DNX_MULT_EGRESS_PORT_INVALID) {
                            remove_entry = 1;
                        }
                    }
                }
                break;

              case 1: /* bitmap or 2nd CUD entry */
                next_index = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, LINK_PTRf);
                if (soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, NEW_FORMATf)) { /* This is a 2nd CUD specification entry */
                    correct_cud2 = cud2_field == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f);
                } else if (correct_cud2 && is_bitmap_cud_replication && bitmap == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, BMP_PTRf) &&
                    /* Entry containing a bitmap pointer+CUD replications */
                    cud == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f)) {
                    soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, BMP_PTRf, DNX_MC_EGR_BITMAP_DISABLED); /* remove replication */
                    found = remove_entry = 1;
                }
                break;

              case 3: /* An entry with two port+CUD replications and no link pointer */
                next_index = cur_index + 1;
              case 2:
                if (correct_cud2 && is_port_cud_replication) {
                    uint32 port1 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f);
                    uint32 port2 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f);
                    if (port1 == dest && soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f) == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (port2 == DNX_MULT_EGRESS_PORT_INVALID) { /* replication disabled for this entry part */
                            remove_entry = 1;
                        }
                    } else if (port2 == dest && soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f) == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (port1 == DNX_MULT_EGRESS_PORT_INVALID) { /* replication disabled for this entry part */
                            remove_entry = 1;
                        }
                    }
                }
                break;

              default:
                if (format < 8) { /* two CUD only replications */
                    next_index = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, LINK_PTRf);
                    if (correct_cud2 && is_cud_only_replication) {
                        uint32 outlif1 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_1f);
                        uint32 outlif2 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_2f);
                        if (outlif1 == cud) {
                            found = 1;
                            soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_1f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                            if (outlif2 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                                remove_entry = 1;
                            }
                        } else if (outlif2 == cud) {
                            found = 1;
                            soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_3m, mcdb_entry, OUTLIF_2f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                            if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                                remove_entry = 1;
                            }
                        }
                    }
                    break;

                } else { /* three CUD only replication, no link pointer */

                    if (format & 4) {
                        next_index = cur_index + 1;
                    }
                    if (correct_cud2 && is_cud_only_replication) {
                        uint32 outlif1 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f);
                        uint32 outlif2 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f);
                        uint32 outlif3 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f);
                        if (outlif1 == cud) {
                            found = 1;
                            soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                            if (outlif2 == DNX_MC_EGR_OUTLIF_DISABLED && outlif3 == DNX_MC_EGR_OUTLIF_DISABLED) { /* This is the only replication in the entry */
                                remove_entry = 1;
                            }
                        } else if (outlif2 == cud) {
                            found = 1;
                            soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                            if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED && outlif3 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                                remove_entry = 1;
                            }
                        } else if (outlif3 == cud) {
                            found = 1;
                            soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                            if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED && outlif2 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                                remove_entry = 1;
                            }
                        }
                    }
                }

            } /* finished handling the format */

            if (found) {
                break;
            }

            if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* The replication was not found till the end of the group */
              DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("replication not found in group")));
            }
            prev_index = cur_index;
            mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index);
            DNX_MC_ASSERT(DNX_MCDS_TYPE_VALUE_EGRESS == DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */
        }

        if (!found) {
            DNX_MC_ASSERT(0);
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("The replication was not found till the legal size of a group")));
        }

        if (remove_entry) { /* If we want to remove the entry since no replication was left in it */
            int try_to_copy_the_next_entry = 0;
            if (cur_index == entry_index) { /* the first entry in the group needs to be removed */
                DNX_MC_ASSERT(prev_index == entry_index);
                if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* this is the only entry in the group, just update it */
                    mcdb_entry->word0 = mcds->free_value[0];
                    mcdb_entry->word1 &= ~mcds->msb_word_mask;
                    mcdb_entry->word1 |= mcds->free_value[1];
                    DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, group_mcid, DNX_MC_CORE_BITAMAP_CORE_0 << core));
                } else  { /* The removed entry points to a second entry */
                    try_to_copy_the_next_entry = 1;
                }
            } else { /* the replication entry is not the start of the group, it will be removed */
                jer2_arad_mcdb_entry_t *prev_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, prev_index);
                uint32 prev_format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, prev_entry, ENTRY_FORMATf), prev_format_next_bit;
                DNX_MC_ASSERT(prev_index != cur_index);
                if (prev_format < 8 && (prev_format & 14) != 2) { /* Does the previous entry have a link pointer to this entry? */
                    /* We will remove the entry by having the previous entry point to the next entry */
                    DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
                      mcds, unit, prev_index, DNX_MCDS_TYPE_VALUE_EGRESS, next_index));
                    freed_index = cur_index;
                } else if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* We can remove the last entry even if there is no pointer to it */
                    prev_format_next_bit = prev_format >= 8 ? 4 : 1;
                    DNX_MC_ASSERT(prev_format & prev_format_next_bit);
                    soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, prev_entry, ENTRY_FORMATf, prev_format & ~prev_format_next_bit); /* mark previous entry as end of group */
                    DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, prev_index));
                    freed_index = cur_index;
                } else {
                    try_to_copy_the_next_entry = 1;
                }
            }

            if (try_to_copy_the_next_entry) { /* If the next entry is the end of the group or has a pointer, we can copy it on top of this entry */
                /* We will not copy the whole block, as this will produce interim wrong content (duplicate replications) in the MC group */
                jer2_arad_mcdb_entry_t *next_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index);
                uint32 next_format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, next_entry, ENTRY_FORMATf);
                DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_TYPE(next_entry) == DNX_MCDS_TYPE_VALUE_EGRESS);
                if (next_format < 12 && next_format != 3) { /* If the next entry has a pointer or is the last entry in the group */
                    mcdb_entry->word0 = next_entry->word0;
                    mcdb_entry->word1 &= ~mcds->msb_word_mask;
                    mcdb_entry->word1 |= (next_entry->word1 & mcds->msb_word_mask);
                    freed_index = next_index;
                    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds,unit, next_index, DNX_MCDS_TYPE_VALUE_EGRESS, &next_index)); /* get entry to update the back_pointer in */
                }
            }
        }
    } else { /* Arad/+ */

        for (; entries_left; --entries_left) { /* loop over the linked list */
            cur_index = next_index;

            switch (format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, ENTRY_FORMATf)) {
              /* select memory format based on the format type */
              case 0: /* Entry containing two port+CUD replications sharing the same CUD */
                next_index = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, LINK_PTRf);
                if (is_port_cud_replication && soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, OUTLIF_1f) == cud) {
                    uint32 port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Af);
                    if (dest == port) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Af, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Bf) == JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID) {
                            remove_entry = 1;
                        }
                    } else if (dest < JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID && dest == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Bf)) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Bf, JER2_ARAD_MULT_EGRESS_SMALL_PORT_INVALID); /* remove replication */
                        if (port == DNX_MULT_EGRESS_PORT_INVALID) {
                            remove_entry = 1;
                        }
                    }
                }
                break;

              case 1: /* Entry containing a bitmap pointer+CUD replications */
                next_index = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, LINK_PTRf);
                if (is_bitmap_cud_replication && bitmap == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, BMP_PTRf) &&
                    cud == soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f)) {
                    soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, BMP_PTRf, DNX_MC_EGR_BITMAP_DISABLED); /* remove replication */
                    found = remove_entry = 1;
                }
                break;

              case 2: /* two CUD only replications */
              case 3:
                next_index = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, LINK_PTRf);
                if (is_cud_only_replication) {
                    uint32 outlif1 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f);
                    uint32 outlif2 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_2f);
                    if (outlif1 == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_1f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                        if (outlif2 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                            remove_entry = 1;
                        }
                    } else if (outlif2 == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_2f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                        if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                            remove_entry = 1;
                        }
                    }
                }
                break;

              case 4: /* An entry with two port+CUD replications and no link pointer */
              case 5:
                if (is_port_cud_replication) {
                    uint32 port1 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f);
                    uint32 port2 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f);
                    if (port1 == dest && soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f) == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (port2 == DNX_MULT_EGRESS_PORT_INVALID) { /* replication disabled for this entry part */
                            remove_entry = 1;
                        }
                    } else if (port2 == dest && soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f) == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f, DNX_MULT_EGRESS_PORT_INVALID); /* remove replication */
                        if (port1 == DNX_MULT_EGRESS_PORT_INVALID) { /* replication disabled for this entry part */
                            remove_entry = 1;
                        }
                    }
                }
                break;

              default: /* formats 6,7: three CUD only replication, no link pointer */
                if (is_cud_only_replication) {
                    uint32 outlif1 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f);
                    uint32 outlif2 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f);
                    uint32 outlif3 = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f);
                    if (outlif1 == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                        if (outlif2 == DNX_MC_EGR_OUTLIF_DISABLED && outlif3 == DNX_MC_EGR_OUTLIF_DISABLED) { /* This is the only replication in the entry */
                            remove_entry = 1;
                        }
                    } else if (outlif2 == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                        if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED && outlif3 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                            remove_entry = 1;
                        }
                    } else if (outlif3 == cud) {
                        found = 1;
                        soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f, DNX_MC_EGR_OUTLIF_DISABLED); /* remove replication */
                        if (outlif1 == DNX_MC_EGR_OUTLIF_DISABLED && outlif2 == DNX_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
                            remove_entry = 1;
                        }
                    }
                }
            }

            /* get the next entry */
            if (format >= 4) {
                if (format & 1) {
                    next_index = cur_index + 1;
                    DNX_MC_ASSERT(next_index < DNX_MCDS_INGRESS_LINK_END(mcds));
                } else {
                    next_index = DNX_MC_EGRESS_LINK_PTR_END;
                }
            }
            if (found) {
                break;
            }

            if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* The replication was not found till the end of the group */
              DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("replication not found in group")));
            }
            prev_index = cur_index;
            mcdb_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index);
            DNX_MC_ASSERT(DNX_MCDS_TYPE_VALUE_EGRESS == DNX_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */
        }

        if (!found) {
            DNX_MC_ASSERT(0);
            DNXC_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_DNXC_MSG("The replication was not found till the legal size of a group")));
        }

        if (remove_entry) { /* If we want to remove the entry since no replication was left in it */
            int try_to_copy_the_next_entry = 0;
            if (cur_index == entry_index) { /* the first entry in the group needs to be removed */
                DNX_MC_ASSERT(prev_index == entry_index);
                if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* this is the only entry in the group, just update it */
                    mcdb_entry->word0 = mcds->free_value[0];
                    mcdb_entry->word1 &= ~mcds->msb_word_mask;
                    mcdb_entry->word1 |= mcds->free_value[1];
                    DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, group_mcid, DNX_MC_CORE_BITAMAP_CORE_0 << core));
                } else  { /* The removed entry points to a second entry */
                    try_to_copy_the_next_entry = 1;
                }
            } else { /* the replication entry is not the start of the group, it will be removed */
                jer2_arad_mcdb_entry_t *prev_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, prev_index);
                uint32 prev_format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, prev_entry, ENTRY_FORMATf);
                DNX_MC_ASSERT(prev_index != cur_index);
                if (prev_format < 4) { /* Does the previous entry have a link pointer to this entry? */
                    /* We will remove the entry by having the previous entry point to the next entry */
                    DNXC_IF_ERR_EXIT(DNX_MCDS_SET_NEXT_POINTER( /* set next pointer and write to hardware the first group entry */
                      mcds, unit, prev_index, DNX_MCDS_TYPE_VALUE_EGRESS, next_index));
                    freed_index = cur_index;
                } else if (next_index == DNX_MC_EGRESS_LINK_PTR_END) { /* We can remove the last entry even if there is no pointer to it */
                    DNX_MC_ASSERT(prev_format & 1);
                    soc_mem_field32_set(unit, IRR_MCDB_EGRESS_FORMAT_0m, prev_entry, ENTRY_FORMATf, prev_format & ~1); /* mark previous entry as end of group */
                    DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, prev_index));
                    freed_index = cur_index;
                } else {
                    try_to_copy_the_next_entry = 1;
                }
            }

            if (try_to_copy_the_next_entry) { /* If the next entry is the end of the group or has a pointer, we can copy it on top of this entry */
                jer2_arad_mcdb_entry_t *next_entry = DNX_MCDS_GET_MCDB_ENTRY(mcds, next_index);
                uint32 next_format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, next_entry, ENTRY_FORMATf);
                DNX_MC_ASSERT(DNX_MCDS_ENTRY_GET_TYPE(next_entry) == DNX_MCDS_TYPE_VALUE_EGRESS);
                if (next_format < 4 || !(next_format & 1)) { /* If the next entry has a pointer or is the last entry in the group */
                    mcdb_entry->word0 = next_entry->word0;
                    mcdb_entry->word1 &= ~mcds->msb_word_mask;
                    mcdb_entry->word1 |= (next_entry->word1 & mcds->msb_word_mask);
                    freed_index = next_index;
                    DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER(mcds,unit, next_index, DNX_MCDS_TYPE_VALUE_EGRESS, &next_index)); /* get entry to update the back_pointer in */
                }
            }
        }
    }

    if (freed_index != cur_index || !remove_entry) { /* if not freeing the current entry, it needs to be updated in hardware */
        DNXC_IF_ERR_EXIT(dnx_mcds_write_entry(unit, cur_index)); /* write group start entry to hardware */
    }

    if (freed_index != entry_index) { /* if an entry was removed, free it */
        if (next_index != DNX_MC_EGRESS_LINK_PTR_END) {
            DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, next_index) == DNX_MCDS_TYPE_VALUE_EGRESS);
            DNX_MCDS_SET_PREV_ENTRY(mcds, next_index, prev_index); /* make entry next_index point back to prev_index */
        }
        DNXC_IF_ERR_EXIT(dnx_mcds_build_free_blocks( /* free the entry freed_index */
          unit, mcds, freed_index, freed_index, dnx_mcds_get_region(mcds, freed_index), dnxMcdsFreeBuildBlocksAdd_AllMustBeUsed));
    }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * Adds the given replications to the non bitmap egress multicast group.
 * It is an error if the group is not open.
 */
uint32 dnx_mult_eg_reps_add(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  dnx_mc_id_t           group_mcid, /* group mcid */
    DNX_SAND_IN  uint32                nof_reps,   /* number of replications to add */
    DNX_SAND_IN  dnx_mc_replication_t  *reps,      /* input array containing replications to add*/
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err    /* return possible errors that the caller may want to ignore */
)
{
    uint16 group_size;
    uint32 linked_list_start, created_list;
    uint32 old_group_entries[SOC_DNX_DEFS_MAX(NOF_CORES)] = {DNX_MC_EGRESS_LINK_PTR_END}; /* the linked lists (per core) of the previous group content */
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    int core;
    dnx_mc_core_bitmap_t changed_cores = DNX_MC_CORE_BITAMAP_NO_CORES, core_bm;
    uint8 is_open;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reps);
    DNXC_NULL_CHECK(out_err);
    DNXC_IF_ERR_EXIT(dnx_egress_group_open_get(unit, group_mcid, &is_open));
    if (!is_open) { /* group is not open */
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    }


    /* clear the replications in mcds and add the new replication */
    DNXC_IF_ERR_EXIT(dnx_mcds_copy_replications_from_arrays( unit, TRUE, TRUE, nof_reps, reps));

    /* set changed_cores to contain cores for which a replication is added, and store current linked lists of these cores */
    for (core = 0; core < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; ++core) {
        if (mcds->nof_reps[core] > 0) {
            changed_cores |= DNX_MC_CORE_BITAMAP_CORE_0 << core;
        }

        linked_list_start = DNX_MCDS_GET_EGRESS_GROUP_START(mcds, group_mcid, core);
        DNX_MC_ASSERT(DNX_MCDS_GET_TYPE(mcds, linked_list_start) == DNX_MCDS_TYPE_VALUE_EGRESS_START);
        DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER( /* store previous linked lists to be freed */
          mcds, unit, linked_list_start, DNX_MCDS_TYPE_VALUE_EGRESS, old_group_entries + core));
    }

    /* get the replications of the current group into the mcds */
    DNXC_IF_ERR_EXIT(dnx_mcds_get_group( unit, changed_cores, FALSE, TRUE, group_mcid,
      DNX_MCDS_TYPE_VALUE_EGRESS_START, DNX_MULT_MAX_EGRESS_REPLICATIONS - nof_reps, &group_size));
    if (group_size > DNX_MULT_MAX_EGRESS_REPLICATIONS - nof_reps) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("too many replications")));
    }

    /* recreate the group with the extra replication by building the linked lists, including the first entry */
    DNXC_IF_ERR_EXIT(mcds->set_egress_linked_list(unit, TRUE, group_mcid,
      DNX_MC_EGRESS_LINK_PTR_END, 0, 0, changed_cores, &created_list, out_err));
    if (*out_err) { /* If failed, do some error code fixing */
        if (*out_err == _SHR_E_PARAM) {
            *out_err = _SHR_E_EXISTS;
        }
    } else { /* If succeeded, free previous linked lists that are not used any more */
        DNX_MC_FOREACH_CORE(changed_cores, core_bm, core) {
            if (old_group_entries[core] != DNX_MC_EGRESS_LINK_PTR_END) {
                DNXC_IF_ERR_EXIT(dnx_mcdb_free_linked_list( unit, old_group_entries[core], DNX_MCDS_TYPE_VALUE_EGRESS));
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}

/*
 * Removes the given replication from the non bitmap egress multicast group.
 * It is an error if the group is not open or does not contain the replication.
 */
uint32 dnx_mult_eg_reps_remove(
    DNX_SAND_IN  int                   unit,
    DNX_SAND_IN  dnx_mc_id_t           group_mcid,   /* group mcid */
    DNX_SAND_IN  uint32                nof_reps,     /* number of replications to remove */
    DNX_SAND_IN  dnx_mc_replication_t  *reps,        /* input array containing replications to remove */
    DNX_SAND_OUT DNX_TMC_ERROR         *out_err      /* return possible errors that the caller may want to ignore */
)
{
    uint16 group_size;
    dnx_mcds_base_t *mcds = dnx_get_mcds(unit);
    int core;
    dnx_mc_core_bitmap_t changed_cores = DNX_MC_CORE_BITAMAP_NO_CORES;
    uint8 is_open;

    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(reps);


    DNXC_IF_ERR_EXIT(dnx_egress_group_open_get(unit, group_mcid, &is_open));
    if (!is_open) { /* group is not open */
        DNXC_EXIT_WITH_ERR(SOC_E_NOT_FOUND, (_BSL_DNXC_MSG("MC group is not created")));
    } else if (nof_reps != 1) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("currently supporting the removal of only one replication")));
    }

    /* Set changed_cores to contain the cores of the replication to be removed */
    DNXC_IF_ERR_EXIT(dnx_mcds_copy_replications_from_arrays(unit, TRUE, TRUE, nof_reps, reps)); /* Clear the replications in mcds and add the replication */
    for (core = 0; core < SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores; ++core) {
        if (mcds->nof_reps[core] > 0) {
            changed_cores |= DNX_MC_CORE_BITAMAP_CORE_0 << core;
        }
    }

    /* get the replications of the relevant cores of the group into the mcds */
    DNXC_IF_ERR_EXIT(dnx_mcds_get_group(unit, changed_cores, TRUE, TRUE, group_mcid,
        DNX_MCDS_TYPE_VALUE_EGRESS_START, DNX_MULT_MAX_EGRESS_REPLICATIONS, &group_size));

    /* remove the given replication from the mcds */
    DNXC_IF_ERR_EXIT(dnx_mult_remove_replication(unit, DNX_MCDS_TYPE_VALUE_EGRESS_START, reps->dest, reps->cud, reps->additional_cud, out_err, &changed_cores));
    if (*out_err) { /* If the replication was not found, exit */
        SOC_EXIT;
    }

    /* recreate the group with the extra replication */
    {
        uint32 old_group_entries[SOC_DNX_DEFS_MAX(NOF_CORES)] = {0}, created_list;
        dnx_mc_core_bitmap_t core_bm;
        DNX_MC_FOREACH_CORE(changed_cores, core_bm, core) {
            DNXC_IF_ERR_EXIT(DNX_MCDS_GET_NEXT_POINTER( /* store previous linked list to be freed */
                mcds, unit, DNX_MCDS_GET_EGRESS_GROUP_START(mcds, group_mcid, core), DNX_MCDS_TYPE_VALUE_EGRESS, old_group_entries + core));
        }

        DNXC_IF_ERR_EXIT(mcds->set_egress_linked_list( /* build the group, including the first entry */
            unit, TRUE, group_mcid, DNX_MC_EGRESS_LINK_PTR_END, 0, 0, changed_cores, &created_list, out_err));
        if (*out_err) { /* If failed due to lack of free entries */
#ifndef JER2_ARAD_EGRESS_MC_DELETE_FAILS_ON_FULL_MCDB
            soc_error_t ret1, ret2 = SOC_E_NONE;
            /* If we can not reconstruct the group due to MCDB being full, just remove the replication/entry */
            DNX_MC_FOREACH_CORE(changed_cores, core_bm, core) {
                ret1 = dnx_mcds_remove_replications_from_egress_group(unit, core, group_mcid, reps->dest, reps->cud, reps->additional_cud);
                if (ret2 == SOC_E_NONE) {
                    ret2 = ret1;
                }
            }
            if (ret2 != SOC_E_NONE) {
                DNXC_EXIT_WITH_ERR(ret2, (_BSL_DNXC_MSG("failed to delete egress replication")));
            }
            *out_err = _SHR_E_NONE;
#endif /* JER2_ARAD_EGRESS_MC_DELETE_FAILS_ON_FULL_MCDB */
        } else { /* free previous linked list of the group not used any more */
            DNX_MC_FOREACH_CORE(changed_cores, core_bm, core) {
                if (old_group_entries[core] != DNX_MC_EGRESS_LINK_PTR_END) {
                    DNXC_IF_ERR_EXIT(dnx_mcdb_free_linked_list(unit, old_group_entries[core], DNX_MCDS_TYPE_VALUE_EGRESS));
                }
            }
        }
    }

exit:
    DNXC_FUNC_RETURN;
}


/*
 * Returns the size of the multicast group with the specified multicast id.
 * Not needed for bcm APIs, so not tested.
 * returns 0 for non open groups.
 */
uint32 dnx_mult_eg_group_size_get(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  dnx_mc_id_t  multicast_id_ndx,
    DNX_SAND_OUT uint32       *mc_group_size
)
{
  uint8 does_exit;
  DNXC_INIT_FUNC_DEFS;
  DNXC_NULL_CHECK(mc_group_size);


  if (multicast_id_ndx <= SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high) { /* this is a vlan bitmap group */

    *mc_group_size = 0;
    DNXC_IF_ERR_EXIT(dnx_mult_does_group_exist(unit, multicast_id_ndx, TRUE, &does_exit));
    if (does_exit) { /* group open */
      dnx_mc_egr_bitmap_t bitmap;
      uint32 bits, *bits_ptr = bitmap;
      unsigned bits_left, words_left = (DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;

      DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));
      for (; words_left; --words_left) {
        bits = *(bits_ptr++);
        for (bits_left = DNX_SAND_NOF_BITS_IN_UINT32; bits_left; --bits_left) {
          *mc_group_size += (bits & 1);
          bits >>= 1;
        }
      }
    }

  } else { /* not a bitmap */

    uint8 is_open;
    DNXC_IF_ERR_EXIT(dnx_mult_eg_get_group(
      unit, multicast_id_ndx, 0, 0, 0, 0, mc_group_size, &is_open));

  }

exit:
  DNXC_FUNC_RETURN;
}


/*
 * This function creates or destroys a bitmap group, Setting the bitmap to not replicate (0).
 */
uint32 dnx_mult_eg_bitmap_group_zero(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx, /* group mcid */
    DNX_SAND_IN  uint8       create            /* should be 1 for create, and 0 for destroy */
)
{
    uint32 data[DNX_EGQ_VLAN_TABLE_TBL_ENTRY_SIZE] = {0};
    dnx_mc_core_bitmap_t old_cores, new_cores = DNX_MC_CORE_BITAMAP_NO_CORES;

    DNXC_INIT_FUNC_DEFS;
    if (multicast_id_ndx > SOC_DNX_CONFIG(unit)->tm.multicast_egress_bitmap_group_range.mc_id_high) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("MCID is not a bitmap group")));
    }

    DNXC_IF_ERR_EXIT(dnx_egress_group_get_core_replication(unit, multicast_id_ndx, &old_cores)); /* get egress cores replicated to */
    if (old_cores != new_cores) {
        DNXC_IF_ERR_EXIT(dnx_egress_group_set_core_replication(unit, multicast_id_ndx, new_cores)); /* mark no replications to cores */
    }
    DNXC_IF_ERR_EXIT(WRITE_EGQ_VLAN_TABLEm(unit, EGQ_BLOCK(unit, SOC_CORE_ALL), multicast_id_ndx, data)); /* mark bitmap as containing no ports */
    DNXC_IF_ERR_EXIT(dnx_egress_group_open_set(unit, multicast_id_ndx, create)); /* mark the group as open */

exit:
    DNXC_FUNC_RETURN;
}


/*
 * This function opens a bitmap group, and sets it to replicate to the given ports.
 */
uint32 dnx_mult_eg_bitmap_group_create(
    DNX_SAND_IN  int         unit,
    DNX_SAND_IN  dnx_mc_id_t multicast_id_ndx /* group mcid */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_mult_eg_bitmap_group_zero(unit, multicast_id_ndx, 1));
exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*     Set the bitmap of the given egress bitmap group to the given bitmap.
*     The bitmap is of TM ports (and not of local ports).
*********************************************************************/
uint32 dnx_mult_eg_bitmap_group_update(
    DNX_SAND_IN  int                                   unit,
    DNX_SAND_IN  dnx_mc_id_t                           multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *group /* TM port bitmap to set */
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_NULL_CHECK(group);
    if (multicast_id_ndx >= DNX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, &group->bitmap[0]));
exit:
    DNXC_FUNC_RETURN;
}

/*
 * This function closed a bitmap group, clearing its hardware replications.
 */
uint32 dnx_mult_eg_bitmap_group_close(
    DNX_SAND_IN  int          unit,
    DNX_SAND_IN  dnx_mc_id_t  multicast_id_ndx
)
{
    DNXC_INIT_FUNC_DEFS;
    DNXC_IF_ERR_EXIT(dnx_mult_eg_bitmap_group_zero(unit, multicast_id_ndx, 0));
exit:
    DNXC_FUNC_RETURN;
}


/*********************************************************************
*   Get the index inside a bitmap from a local port.
*   The bitmap index is the index in the bitmap depending on the core ID and TM port.
*********************************************************************/
uint32 dnx_mult_eg_bitmap_local_port_2_bitmap_bit_index(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  DNX_TMC_FAP_PORT_ID port,          /* local port */
    DNX_SAND_OUT uint32              *out_bit_index /* returned bit index */
)
{
    DNXC_INIT_FUNC_DEFS;
    if (SOC_IS_JERICHO(unit)) {
        uint32 tm_port;
        int core;
        DNXC_IF_ERR_EXIT(dnx_port_sw_db_local_to_tm_port_get(unit, port, &tm_port, &core));
        *out_bit_index = tm_port + DNX_TMC_NOF_FAP_PORTS_PER_CORE * core;
    } else {
        *out_bit_index = port;
    }
exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Add port replications to an Egress-Multicast bitmap group. - replaced by dnx_mult_eg_bitmap_group_bm_add
*********************************************************************/
uint32 dnx_mult_eg_bitmap_group_port_add(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_FAP_PORT_ID port,         /* local port to add */
    DNX_SAND_OUT DNX_TMC_ERROR       *out_err      /* return possible errors that the caller may want to ignore */
)
{
    uint32 bit_index, word_index, bit_mask;
    dnx_mc_egr_bitmap_t bitmap = {0};

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= DNX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }
    DNXC_IF_ERR_EXIT(dnx_mult_eg_bitmap_local_port_2_bitmap_bit_index(unit, port, &bit_index));
    word_index = bit_index / DNX_SAND_NOF_BITS_IN_UINT32; 
    bit_mask = (uint32)1 << (bit_index & (DNX_SAND_NOF_BITS_IN_UINT32 - 1));

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    if (bitmap[word_index] & bit_mask) {
        *out_err = _SHR_E_EXISTS; /* port is already replicated to */
    } else {
        bitmap[word_index] |= bit_mask;
        DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
        *out_err = _SHR_E_NONE;
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Add port replications from a bitmap to an Egress-Multicast bitmap group.
*********************************************************************/
uint32 dnx_mult_eg_bitmap_group_bm_add(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *add_bm, /* TM ports to add */
    DNX_SAND_OUT DNX_TMC_ERROR                         *out_err /* return possible errors that the caller may want to ignore */
)
{
    unsigned nof_tm_port_words = (DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dnx_mc_egr_bitmap_t bitmap = {0};
    const uint32 *changes = &add_bm->bitmap[0];
    uint32 *to_write = bitmap;

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= DNX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    for (; nof_tm_port_words; --nof_tm_port_words) { /* add new ports to bitmap */
        if (*to_write & *changes) {
            *out_err = _SHR_E_EXISTS; /* some ports are already replicated to */
            SOC_EXIT;
        }
        *(to_write++) |= *(changes++);
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
    *out_err = _SHR_E_NONE;

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Removes a port replication of an egress multicast bitmap group. - replaced by dnx_mult_eg_bitmap_group_bm_remove
*********************************************************************/
uint32 dnx_mult_eg_bitmap_group_port_remove(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_FAP_PORT_ID port,         /* local port to remove */
    DNX_SAND_OUT DNX_TMC_ERROR       *out_err      /* return possible errors that the caller may want to ignore */
)
{
    uint32 bit_index, word_index, bit_mask;
    dnx_mc_egr_bitmap_t bitmap = {0};

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= DNX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }
    DNXC_IF_ERR_EXIT(dnx_mult_eg_bitmap_local_port_2_bitmap_bit_index(unit, port, &bit_index));
    word_index = bit_index / DNX_SAND_NOF_BITS_IN_UINT32;
    bit_mask = (uint32)1 << (bit_index & (DNX_SAND_NOF_BITS_IN_UINT32 - 1));

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    if (bitmap[word_index] & bit_mask) {
        bitmap[word_index] &= ~bit_mask;
        DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
        *out_err = _SHR_E_NONE;
    } else {
        *out_err = _SHR_E_NOT_FOUND; /* port is not replicated to */
    }

exit:
    DNXC_FUNC_RETURN;
}

/*********************************************************************
*   Add port replications from a bitmap to an Egress-Multicast bitmap group.
*********************************************************************/
uint32 dnx_mult_eg_bitmap_group_bm_remove(
    DNX_SAND_IN  int                 unit,
    DNX_SAND_IN  dnx_mc_id_t         multicast_id_ndx,
    DNX_SAND_IN  DNX_TMC_MULT_EG_VLAN_MEMBERSHIP_GROUP *rem_bm, /* TM ports to remove */
    DNX_SAND_OUT DNX_TMC_ERROR                         *out_err /* return possible errors that the caller may want to ignore */
)
{
    unsigned nof_tm_port_words = (DNX_TMC_NOF_FAP_PORTS_PER_CORE / DNX_SAND_NOF_BITS_IN_UINT32) * SOC_DNX_CONFIG(unit)->core_mode.nof_active_cores;
    dnx_mc_egr_bitmap_t bitmap = {0};
    const uint32 *changes = &rem_bm->bitmap[0];
    uint32 *to_write = bitmap;

    DNXC_INIT_FUNC_DEFS;

    if (multicast_id_ndx >= DNX_MC_EGR_NOF_BITMAPS) {
        DNXC_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_DNXC_MSG("ID is too high for a multicast bitmap")));
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_get(unit, multicast_id_ndx, bitmap));

    for (; nof_tm_port_words; --nof_tm_port_words) { /* add new ports to bitmap */
        if (~*to_write & *changes) {
            *out_err = _SHR_E_NOT_FOUND; /* some ports are not replicated to */
            SOC_EXIT;
        }
        *(to_write++) &= ~*(changes++);
    }

    DNXC_IF_ERR_EXIT(dnx_egq_vlan_table_tbl_set(unit, multicast_id_ndx, bitmap));
    *out_err = _SHR_E_NONE;

exit:
    DNXC_FUNC_RETURN;
}

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

