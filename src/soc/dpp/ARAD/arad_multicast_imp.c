/* $Id: arad_multicast_imp.c,v $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#include <shared/bsl.h>

#include <soc/mcm/memregs.h>
#ifdef BCM_88650_A0

#ifdef _ERR_MSG_MODULE_NAME
  #error "_ERR_MSG_MODULE_NAME redefined"
#endif

#define _ERR_MSG_MODULE_NAME BSL_SOC_MULTICAST

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dpp/multicast_imp.h>
#include <soc/dpp/ARAD/arad_multicast_imp.h>
#include <soc/dcmn/error.h>
#include <shared/bsl.h>
#include <soc/dpp/SAND/Utils/sand_header.h>
#include <soc/dpp/mbcm.h>
#include <soc/dpp/ARAD/arad_ports.h>

/* } */

/*************
 * DEFINES   *
 *************/
/* { */

/* constants used for the interpretation of ingress multicast destinations */
#define ARAD_MC_ING_DESTINATION_FLD_TYPE_OFFSET 16
#define ARAD_MC_ING_DESTINATION_FLD_TYPE 0x30000 /* 3 << ARAD_MC_ING_DESTINATION_FLD_TYPE_OFFSET */
#define ARAD_MC_ING_DESTINATION_ID_MASK_TM_FLOW 0x1ffff
#define ARAD_MC_ING_DESTINATION_ID_MASK_OTHERS 0xffff
#define ARAD_MC_ING_DESTINATION_ID_MASK_IS_LAG 0x8000
#define ARAD_MC_ING_DESTINATION_ID_MASK_LAG 0x3fff
#define ARAD_MC_ING_DESTINATION_FLD_TYPE_TM_FLOW 0x20000
#define ARAD_MC_ING_DESTINATION_FLD_TYPE_PORT 0x00000
#define ARAD_MC_ING_DESTINATION_FLD_TYPE_MCID 0x10000


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


/*
 * Get the (pointer to the) next entry from the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 */
uint32 arad_mcdb_get_next_pointer(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint32  entry,      /* entry from which to get the next entry pointer */
    SOC_SAND_IN  uint32  entry_type, /* the type of the entry */
    SOC_SAND_OUT uint32  *next_entry /* the output next entry */
  )
{
  dpp_mcds_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, entry);
  soc_mem_t mem = 0;
  SOCDNX_INIT_FUNC_DEFS;
  DPP_MC_ASSERT(DPP_MCDS_TYPES_ARE_THE_SAME(entry_type, DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry)));

  if (DPP_MCDS_TYPE_IS_INGRESS(entry_type)) { /* set ingress entry pointer */
    mem = IRR_MCDBm;
  } else if (DPP_MCDS_TYPE_IS_EGRESS_NORMAL(entry_type)) { /* set egress non-TDM entry pointer */
    switch (soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, ENTRY_FORMATf))
    { /* select memory format based on the format type */
    case 0:
      mem = IRR_MCDB_EGRESS_FORMAT_0m;
      break;
    case 1:
      mem = IRR_MCDB_EGRESS_FORMAT_1m;
      break;
    case 2:
    case 3:
      mem = IRR_MCDB_EGRESS_FORMAT_2m;
      break;
    case 4:
    case 6:
      *next_entry = DPP_MC_EGRESS_LINK_PTR_END;
      ARAD_DO_NOTHING_AND_EXIT;
    default:
      *next_entry = entry + 1;
      DPP_MC_ASSERT(*next_entry < MCDS_INGRESS_LINK_END(mcds));
      ARAD_DO_NOTHING_AND_EXIT;
    }
  }
  if (!mem) {
    DPP_MC_ASSERT(0);
    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unhandled case")));
  }
  *next_entry = soc_mem_field32_get(unit, mem, mcdb_entry, LINK_PTRf);

exit:
  SOCDNX_FUNC_RETURN;
}


/*
 * Set the pointer to the next entry in the given entry.
 * The entry type (ingress/egress/egress) TDM is given as an argument.
 * changes both the mcds and hardware.
 */
uint32
  arad_mcdb_set_next_pointer(
    SOC_SAND_IN  int  unit,
    SOC_SAND_IN  uint32  entry_to_set, /* index of the entry in which to set the pointer */
    SOC_SAND_IN  uint32  entry_type,   /* the type of entry_to_set */
    SOC_SAND_IN  uint32  next_entry    /* the entry that entry_to_set will point to */
  )
{
  dpp_mcds_base_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, entry_to_set);
  soc_mem_t mem = 0;
  SOCDNX_INIT_FUNC_DEFS;
  DPP_MC_ASSERT(DPP_MCDS_TYPES_ARE_THE_SAME(entry_type, DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry)));

  if (DPP_MCDS_TYPE_IS_INGRESS(entry_type)) { /* set ingress entry pointer */
    mem = IRR_MCDBm;
  } else if (DPP_MCDS_TYPE_IS_EGRESS_NORMAL(entry_type)) { /* set egress non-TDM entry pointer */
    switch (soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, ENTRY_FORMATf))
    { /* select memory format based on the format type */
    case 0:
      mem = IRR_MCDB_EGRESS_FORMAT_0m;
      break;
    case 1:
      mem = IRR_MCDB_EGRESS_FORMAT_1m;
      break;
    case 2:
    case 3:
      mem = IRR_MCDB_EGRESS_FORMAT_2m;
      break;
    case 4:
    case 6:
      if (entry_type == DPP_MCDS_TYPE_VALUE_EGRESS_START && next_entry == DPP_MC_EGRESS_LINK_PTR_END) {
        break; /* We allow the first egress group entry to have no pointer if it marks the end of the group */
      }
    default:
      DPP_MC_ASSERT(0);
      SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("invalid entry type")));
    }
  } else {
    DPP_MC_ASSERT(0);
    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unhandled case")));
  }
  if (mem) {
    soc_mem_field32_set(unit, mem, mcdb_entry, LINK_PTRf, next_entry); /* store the pointer in the correct format */
  }
  SOCDNX_IF_ERR_EXIT(dpp_mcds_write_entry(unit, entry_to_set)); /* write to hardware */

exit:
  SOCDNX_FUNC_RETURN;
}




 /*
 * Adds the contents of a mcdb entry of the given type to the mcds buffer.
 * No more than *max_size replications are added, and the max_size value
 * is decreased by the number of added replications.
 * *group_size is increased by the number of found replications.
 * The next entry pointed to by this entry is returned in next_entry.
 */
uint32
  arad_mcds_get_replications_from_entry(
    SOC_SAND_IN    int     unit,
    SOC_SAND_IN    int     core,        /* not used in Arad */
    SOC_SAND_IN    uint8   get_bm_reps, /* Should the function return bitmap replications (if non zero) or ignore them, not used in Arad */
    SOC_SAND_IN    uint32  entry_index, /* table index of the entry */
    SOC_SAND_IN    uint32  entry_type,  /* the type of the entry */
    SOC_SAND_INOUT uint32  *cud2,       /* the current 2nd CUD of replications, not used in Arad */
    SOC_SAND_INOUT uint16  *max_size,   /* the maximum number of replications to return from the group, decreased by the number of returned replications */
    SOC_SAND_INOUT uint16  *group_size, /* incremented by the number of found replications (even if they are not returned) */
    SOC_SAND_OUT   uint32  *next_entry  /* the next entry */
  )
{
  dpp_mcds_t *mcds = dpp_get_mcds(unit);
  arad_mcdb_entry_t *mcdb_entry = MCDS_GET_MCDB_ENTRY(mcds, entry_index);
  uint32 format = 0;
  soc_mem_t mem = 0;
  uint16 found = 0, max = *max_size;
  SOCDNX_INIT_FUNC_DEFS;
  DPP_MC_ASSERT(entry_type == DPP_MCDS_ENTRY_GET_TYPE(mcdb_entry)); /* type should exactly match entry */

  if (DPP_MCDS_TYPE_IS_INGRESS(entry_type)) { /* set ingress entry pointer */

    uint32 dest = soc_mem_field32_get(unit, (mem = IRR_MCDBm), mcdb_entry, DESTINATIONf);
    if (dest != DPP_MC_ING_DESTINATION_DISABLED) { /* replication not disabled in the entry */
      if (++found <= max) {
        dpp_add_ingress_replication(mcds, soc_mem_field32_get(unit, IRR_MCDBm, mcdb_entry, OUT_LIFf), dest);
      }
    }

  } else if (DPP_MCDS_TYPE_IS_EGRESS_NORMAL(entry_type)) { /* set egress non-TDM entry pointer */

    switch (format = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, ENTRY_FORMATf))
    { /* select memory format based on the format type */
    case 0:
      {
        uint32 port = soc_mem_field32_get(unit, (mem = IRR_MCDB_EGRESS_FORMAT_0m), mcdb_entry, PP_DSP_1Af);
        uint32 cud = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, OUTLIF_1f);
        if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_port_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD, port);
          }
        }
        port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_0m, mcdb_entry, PP_DSP_1Bf);
        if (port != ARAD_MULT_EGRESS_SMALL_PORT_INVALID) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_port_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD, port);
          }
        }
      }
      break;

    case 1:
      {
        const uint32 bitmap = soc_mem_field32_get(unit, (mem = IRR_MCDB_EGRESS_FORMAT_1m), mcdb_entry, BMP_PTRf);
        if (bitmap != DPP_MC_EGR_BITMAP_DISABLED) { /* replication not disabled for this entry */
          if (++found <= max) {
            dpp_add_egress_replication_bitmap(mcds, 0, soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_1m, mcdb_entry, OUTLIF_1f), DPP_MC_NO_2ND_CUD, bitmap);
          }
        }
      }
      break;

    case 2:
    case 3:
      {
        uint32 cud = soc_mem_field32_get(unit, (mem = IRR_MCDB_EGRESS_FORMAT_2m), mcdb_entry, OUTLIF_1f);
        if (cud != DPP_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD);
          }
        }
        cud = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_2m, mcdb_entry, OUTLIF_2f);
        if (cud != DPP_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD);
          }
        }
      }
      break;

    case 4:
    case 5:
      {
        uint32 port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_1f);
        if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_port_cud(mcds, 0, soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_1f), DPP_MC_NO_2ND_CUD, port);
          }
        }
        port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, PP_DSP_2f);
        if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_port_cud(mcds, 0, soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_4m, mcdb_entry, OUTLIF_2f), DPP_MC_NO_2ND_CUD, port);
          }
        }
      }
      break;

    default: /* formats 6,7 */
      DPP_MC_ASSERT(format < 8);
      {
        uint32 cud = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_1f);
        if (cud != DPP_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD);
          }
        }
        cud = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_2f);
        if (cud != DPP_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD);
          }
        }
        cud = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_FORMAT_6m, mcdb_entry, OUTLIF_3f);
        if (cud != DPP_MC_EGR_OUTLIF_DISABLED) { /* replication not disabled for this entry part */
          if (++found <= max) {
            dpp_add_egress_replication_cud(mcds, 0, cud, DPP_MC_NO_2ND_CUD);
          }
        }
      }
    }

  } else if (DPP_MCDS_TYPE_IS_TDM(entry_type)) { /* set egress TDM entry pointer */
    uint32 port = soc_mem_field32_get(unit, (mem = IRR_MCDB_EGRESS_TDM_FORMATm), mcdb_entry, PP_DSP_1f);
    if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
          if (++found <= max) {
        dpp_add_egress_replication_cud(mcds, 0, port, DPP_MC_NO_2ND_CUD);
      }
    }
    port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_2f);
    if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
        if (++found <= max) {
        dpp_add_egress_replication_cud(mcds, 0, port, DPP_MC_NO_2ND_CUD);
      }
    }
    port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_3f);
    if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
        if (++found <= max) {
        dpp_add_egress_replication_cud(mcds, 0, port, DPP_MC_NO_2ND_CUD);
      }
    }
    port = soc_mem_field32_get(unit, IRR_MCDB_EGRESS_TDM_FORMATm, mcdb_entry, PP_DSP_4f);
    if (port != DPP_MULT_EGRESS_PORT_INVALID) { /* replication not disabled for this entry part */
        if (++found <= max) {
        dpp_add_egress_replication_cud(mcds, 0, port, DPP_MC_NO_2ND_CUD);
      }
    }

  }

  /* get the next entry */
  if (mem) {
    *next_entry = soc_mem_field32_get(unit, mem, mcdb_entry, LINK_PTRf);
  } else if (format >= 4) {
    if (format & 1) {
      *next_entry = entry_index + 1;
      DPP_MC_ASSERT(*next_entry < MCDS_INGRESS_LINK_END(mcds));
    } else {
      *next_entry = DPP_MC_EGRESS_LINK_PTR_END;
    }
  } else {
    DPP_MC_ASSERT(0);
    SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unhandled case")));
  }

  if (found < max) {
    *max_size -= found;
  } else {
    *max_size = 0;
  }
  *group_size += found;

exit:
  SOCDNX_FUNC_RETURN;
}

/*
 * sets an ingress replication in BCM API encoding from the given CUD and destination in hardware encoding.
 */
uint32 arad_convert_ingress_replication_hw2api(
    SOC_SAND_IN  int          unit,
    SOC_SAND_IN  uint32       cud,            /* CUD to be converted */
    SOC_SAND_IN  uint32       dest,           /* destination to be converted */
    SOC_SAND_OUT soc_gport_t  *port_array,    /* output array to contain ports/destinations */
    SOC_SAND_OUT soc_if_t     *encap_id_array /* output array to contain encapsulations/CUDs/outlifs */
)
{
    uint32 destination = dest, type_bits;
    SOCDNX_INIT_FUNC_DEFS;

    *encap_id_array = cud; /* set CUD/outlif (encapsulation) */
    /* perform reverse field encoding */
    switch (SOC_DPP_CONFIG(unit)->tm.mc_ing_encoding_mode) { /* reverse ingress decoding according to mode */
      case ARAD_MC_DEST_ENCODING_0:
        break;
      case ARAD_MC_DEST_ENCODING_1:
        *encap_id_array |= dest & (1 << 16);
        destination &= ~(1 << 16);
        break;
      case ARAD_MC_DEST_ENCODING_2:
        if (dest & (1 << 17)) {
            *encap_id_array |= (dest & (3 << 15)) << 1;
            destination &= ~(3 << 15);
        } else {
            *encap_id_array |= ((dest & (1 << 14)) << 2) | ((dest & (1 << 16)) << 1);
            destination &= ~((1 << 14) | (1 << 16));
        }
        break;
      case ARAD_MC_DEST_ENCODING_3:
        *encap_id_array |= (dest & (1 << 17)) >> 1;
        destination |= (1 << 17);
        break;
      default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unknown mode")));
    }

    /* interpret destination type and set gport accordingly */
    type_bits = (destination & ARAD_MC_ING_DESTINATION_FLD_TYPE) >> ARAD_MC_ING_DESTINATION_FLD_TYPE_OFFSET;
    if (type_bits > 1) { /* the destination is a queue */
        _SHR_GPORT_UNICAST_QUEUE_GROUP_SET(*port_array, destination & ARAD_MC_ING_DESTINATION_ID_MASK_TM_FLOW);
    } else if (type_bits) { /* the destination is a multicast */
        _SHR_GPORT_MCAST_SET(*port_array, destination & ARAD_MC_ING_DESTINATION_ID_MASK_OTHERS);
    } else { /* the destination is a system physical port, to be mapped to a mod-port */
        if (destination & ARAD_MC_ING_DESTINATION_ID_MASK_IS_LAG) { /* the destination is a LAG */
            _SHR_GPORT_TRUNK_SET(*port_array, destination & ARAD_MC_ING_DESTINATION_ID_MASK_LAG);
        } else {
            uint32 fap_id, fap_port_id;
            SOCDNX_SAND_IF_ERR_EXIT(arad_sys_phys_to_local_port_map_get(unit, destination, &fap_id, &fap_port_id));
            _SHR_GPORT_MODPORT_SET(*port_array, fap_id, fap_port_id);
        }
    }

exit:
    SOCDNX_FUNC_RETURN;
}


/*
 * This function encodes the destination and the CUD as will be written to the hardware fields of IRR_MCDB.
 * The input is the replication data (destination structure and outlif).
 * It is permitted to call the functions to change its input with these
 * arguments: unit, entry, &entry->destination.id, &entry->cud .
 * No locking is needed for this function.
 */
uint32
  arad_mult_ing_encode_entry(
    SOC_SAND_IN    int                    unit,
    SOC_SAND_IN    SOC_TMC_MULT_ING_ENTRY *ing_entry,       /* replication data */
    SOC_SAND_OUT   uint32                 *out_destination, /* the destination field */
    SOC_SAND_OUT   uint32                 *out_cud          /* the CUD/outlif field */
  )
{
  uint32 destination, cud;
  uint8 encoding_mode = SOC_DPP_CONFIG(unit)->tm.mc_ing_encoding_mode;
  SOCDNX_INIT_FUNC_DEFS;
  SOCDNX_NULL_CHECK(ing_entry);
  SOCDNX_NULL_CHECK(out_destination);
  SOCDNX_NULL_CHECK(out_cud);

  if ((cud = ing_entry->cud) > SOC_DPP_CONFIG(unit)->tm.ingress_mc_max_cud) {
    SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("CUD out of range")));
  }
  if (ing_entry->destination.type == SOC_TMC_DEST_TYPE_QUEUE) { /* direct Queue_id */
    if ((destination = ing_entry->destination.id) > SOC_DPP_CONFIG(unit)->tm.ingress_mc_max_queue) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("queue out of range")));
    }
    switch (encoding_mode) {
      case ARAD_MC_DEST_ENCODING_0:
        destination |= ARAD_MC_ING_DESTINATION_FLD_TYPE_TM_FLOW;
        break;
      case ARAD_MC_DEST_ENCODING_1:
        destination |= (cud & (1 << 16)) | ARAD_MC_ING_DESTINATION_FLD_TYPE_TM_FLOW;
        cud &= ~(1 << 16);
        break;
      case ARAD_MC_DEST_ENCODING_2:
        destination |= ((cud & (3 << 16)) >> 1) | ARAD_MC_ING_DESTINATION_FLD_TYPE_TM_FLOW;
        cud &= ~(3 << 16);
        break;
      case ARAD_MC_DEST_ENCODING_3:
        destination |= (cud & (1 << 16)) << 1;
        cud &= ~(1 << 16);
        break;
      default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unknown mode")));
    }

  } else if (ing_entry->destination.type == SOC_TMC_DEST_TYPE_MULTICAST) { /* multicast_id */
    if (encoding_mode != ARAD_MC_DEST_ENCODING_0) {
      SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("MC destination works only in mode 0")));
    }
    destination = ing_entry->destination.id | ARAD_MC_ING_DESTINATION_FLD_TYPE_MCID;

  } else { /* sys_port_id, type files is either SOC_TMC_DEST_TYPE_LAG or sys_phy_port */
    if (ing_entry->destination.type == SOC_TMC_DEST_TYPE_LAG)
    {
      if (SOC_DPP_CONFIG(unit)->tm.mc_ing_encoding_mode == ARAD_MC_DEST_ENCODING_3) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("LAG destinations not supported in this mode")));
      }
      SOCDNX_SAND_IF_ERR_EXIT(arad_ports_logical_sys_id_build(
        unit, TRUE, ing_entry->destination.id, 0, 0, &destination));
    }
    else  /* destination-type == sys_phy_port*/
    {
      SOCDNX_SAND_IF_ERR_EXIT(arad_ports_logical_sys_id_build(
        unit, FALSE, 0, 0, ing_entry->destination.id, &destination));
      if (destination >= SOC_DPP_CONFIG(unit)->tm.ingress_mc_nof_sysports) {
        SOCDNX_EXIT_WITH_ERR(SOC_E_PARAM, (_BSL_SOCDNX_MSG("system port out of range")));
      }
    }
    switch (encoding_mode) {
      case ARAD_MC_DEST_ENCODING_0:
        break;
      case ARAD_MC_DEST_ENCODING_1:
        destination |= cud & (1 << 16);
        cud &= ~(1 << 16);
        break;
      case ARAD_MC_DEST_ENCODING_2:
        destination |= ((cud & (1 << 16)) >> 2) | ((cud & (1 << 17)) >> 1);
        cud &= ~(3 << 16);
        break;
      default:
        SOCDNX_EXIT_WITH_ERR(SOC_E_INTERNAL, (_BSL_SOCDNX_MSG("unknown mode")));
    }
  }
  *out_destination = destination;
  *out_cud = cud;

exit:
  SOCDNX_FUNC_RETURN;
}

#include <soc/dpp/SAND/Utils/sand_footer.h>

#endif /* BCM_88650_A0 */
