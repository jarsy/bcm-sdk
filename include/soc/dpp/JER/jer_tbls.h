/*
 * $Id: drv.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_DPP_JER_TBLS_H
#define _SOC_DPP_JER_TBLS_H

#include <soc/dpp/ARAD/arad_tbl_access.h>
#include <soc/dpp/ARAD/arad_reg_access.h>
#include <shared/bsl.h>

uint32 soc_jer_sch_tbls_init (int unit);
uint32 soc_jer_irr_tbls_init (int unit);
uint32 soc_jer_ire_tbls_init (int unit);
uint32 soc_jer_ihp_tbls_init (int unit);
uint32 soc_jer_ihb_tbls_init (int unit);
uint32 soc_jer_iqm_tbls_init (int unit);
uint32 soc_jer_iqmt_tbls_init (int unit);
uint32 soc_jer_ips_tbls_init (int unit);
uint32 soc_jer_fcr_tbls_init (int unit);
soc_error_t soc_jer_ipt_tbls_init (int unit);
uint32 soc_jer_fdt_tbls_init (int unit);
int soc_jerplus_fdt_tbls_init(int unit); /*used also for QAX*/
uint32 soc_jer_egq_tbls_init (int unit);
uint32 soc_jer_epni_tbls_init (int unit);
uint32 soc_jer_oamp_tbls_init (int unit);
uint32 soc_jer_mrps_tbls_init (int unit);
uint32 soc_jer_mrpsEm_tbls_init (int unit);
uint32 soc_jer_idr_tbls_init (int unit);
uint32 soc_jer_ppdb_tbls_init(int unit);
uint32 soc_jer_xlp_tbls_init(int unit);
uint32 soc_jer_clp_tbls_init(int unit);
int soc_jer_dynamic_tbls_reset (int unit);

/*
 * Function:
 *      soc_jer_excluded_tbls_list_set
 * Purpose:
 *      sets the excluded memory list with the relevant memories
 * Parameters:
 *      unit    - Device Number 
 * Returns:
 *      SOC_E_XXX 
 * Note:
 *      to insert a memory to excluded list write the memory's name in the relevant exclude list above
 */
int soc_jer_excluded_tbls_list_set (int unit);

/*
 * Function:
 *      soc_jer_static_tbls_reset
 * Purpose:
 *      iterates over all memories and resets the static ones
 * Parameters:
 *      unit    - Device Number
 * Returns:
 *      SOC_E_XXX
 */
int soc_jer_static_tbls_reset (int unit);
/*
 * Read table SCH_DEVICE_RATE_MEMORY_DRM from block SCH,
 * doesn't take semaphore.
 */
uint32
  jer_sch_drm_tbl_get_unsafe(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32              entry_offset,
    SOC_SAND_OUT  ARAD_SCH_DRM_TBL_DATA* SCH_drm_tbl_data
  ) ;
/*
 * Write table SCH_DEVICE_RATE_MEMORY_DRM from block SCH,
 * doesn't take semaphore.
 */
uint32
  jer_sch_drm_tbl_set_unsafe(
    SOC_SAND_IN   int                   unit,
    SOC_SAND_IN   int                   core,
    SOC_SAND_IN   uint32                entry_offset,
    SOC_SAND_IN   ARAD_SCH_DRM_TBL_DATA *SCH_drm_tbl_data
  ) ;
/*
 * Read indirect table shds_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer_sch_shds_tbl_get_unsafe(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32              entry_offset,
    SOC_SAND_OUT  ARAD_SCH_SHDS_TBL_DATA* SCH_shds_tbl_data
  );
/*
 * Write indirect table shds_tbl from block SCH,
 * doesn't take semaphore.
 * Must only be called from a function taking the device semaphore
 */
uint32
  jer_sch_shds_tbl_set_unsafe(
    SOC_SAND_IN   int             unit,
    SOC_SAND_IN   int             core,
    SOC_SAND_IN   uint32              entry_offset,
    SOC_SAND_IN   ARAD_SCH_SHDS_TBL_DATA* SCH_shds_tbl_data
  );

/*
 * Fill the whole table with the given entry, uses fast DMA filling when run on real hardware.
 * Marks the table not to later be filled by soc_jerplus_tbls_init.
 */

int jer_fill_and_mark_memory(
    const int       unit,
    const soc_mem_t mem,        /* memory/table to fill */
    const void      *entry_data /* The contents of the entry to fill the table with. Does not have to be DMA memory */
  );

/* enable/disable dynamic memory writes using the givein dynamic memory write control registers */
int soc_jer_control_dynamic_mem_writes(
    SOC_SAND_IN int unit,
    SOC_SAND_IN soc_reg_t *regs, /* control registers to write to, terminated by INVALIDr */
    SOC_SAND_IN uint32 val);     /* value (0/1) to write to the registers */

/*
 * initialize memory zeroing exclusion to the given array.
 */
void soc_jer_tbls_zero_init(int unit, soc_mem_t *mem_exclude_list);

/*
 * initialize all tables to zero, except for exception array, and marked not to be zeroed.
 */
int soc_jer_tbls_zero(int unit);

/*
 * Function:
 *      soc_jerplus_tbls_init
 * Purpose:
 *      initialize all tables relevant for Jericho+.
 * Parameters:
 *      unit -  unit number
 *  
 * Returns:
 *      SOC_E_XXX
 */
int soc_jerplus_tbls_init(int unit);

#define JER_TBL_SOC_REF(unit, tbl_ptr, field, field_dst)       \
{                                                     \
    soc_mem_info_t *meminfo;                            \
    soc_field_info_t    *fieldinfo;                     \
                                                      \
        meminfo = &SOC_MEM_INFO(unit, tbl_ptr);           \
        SOC_FIND_FIELD(field,                                   \
                       meminfo->fields,                         \
                       meminfo->nFields,                        \
                       fieldinfo);                              \
        if (NULL != fieldinfo) {                          \
            SOCDNX_SAND_IF_ERR_EXIT(soc_sand_os_memcpy(field_dst, fieldinfo, sizeof(soc_field_info_t)));  \
                    }\
        else {\
            SOCDNX_EXIT_WITH_ERR_NO_MSG(SOC_E_INTERNAL);\
        }\
}

#endif /* _SOC_DPP_JER_TBLS_H */
