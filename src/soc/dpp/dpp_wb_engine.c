/*
 * $Id: dpp_wb_engine.c,v 1.75 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */

#ifdef _ERR_MSG_MODULE_NAME 
    #error "_ERR_MSG_MODULE_NAME redefined" 
#endif
#define _ERR_MSG_MODULE_NAME BSL_SOC_INIT
#define _NO_JUMP_NEEDED_ 0xffffffff
#include <shared/bsl.h>
#include <soc/dcmn/error.h>

#include <soc/types.h>
#include <soc/error.h>
#include <soc/scache.h>
#include <soc/drv.h>

#ifdef VALGRIND_DEBUG
/* Used for VALGRIND_DEBUG. See _dpp_wb_engine_valgrind_check_array_is_initialized for details. */
#endif 

#include <bcm_int/dpp/alloc_mngr_lif.h>
#include <bcm_int/dpp/alloc_mngr_local_lif.h>
#include <soc/wb_engine.h>
#include <soc/dpp/drv.h>
#include <soc/dpp/PPC/ppc_api_mymac.h>
#include <soc/dpp/PPD/ppd_api_eg_encap.h>
#include <soc/dpp/PPD/ppd_api_trap_mgmt.h>
#include <soc/dpp/PPD/ppd_api_lif_ing_vlan_edit.h>
#include <soc/dpp/PPD/ppd_api_eg_vlan_edit.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#include <bcm_int/dpp/gport_mgmt.h>
#ifdef BCM_ARAD_SUPPORT
#include <soc/dpp/ARAD/arad_sim_em.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_llp_filter.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_sw_db.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_flp_init.h>
#include <soc/dpp/ARAD/ARAD_PP/arad_pp_eg_encap.h>
#include <soc/dpp/ARAD/arad_sw_db.h>
/* #include <soc/dpp/wb_db_hash_tbl.h> */
#endif /* BCM_ARAD_SUPPORT */




/* undefined at the end of the module */
#define NEVER_REMOVED 0xff
#define NO_FLAGS 0x0
#define DEFAULT 0xffffffff
#define VERSION(_ver) (_ver)


#ifdef BCM_ARAD_SUPPORT
/* the pp_Sw_db data */
ARAD_PP_SW_DB *Dpp_wb_engine_Arad_pp_sw_db;

/* OAM BCM */
uint8 _dpp_oam_is_init[SOC_MAX_NUM_DEVICES] = {0};
uint8 _dpp_bfd_is_init[SOC_MAX_NUM_DEVICES] = {0};

/* OAM End */

/*arad_sw_db*/
ARAD_SW_DB Arad_sw_db;

int
soc_dpp_wb_engine_Arad_pp_sw_db_get(ARAD_PP_SW_DB *sw_db)
{
    Dpp_wb_engine_Arad_pp_sw_db = sw_db;
    return SOC_E_NONE;
}

#define PP_SW_DB_DEVICE (Dpp_wb_engine_Arad_pp_sw_db->device[unit])

#endif /* BCM_ARAD_SUPPORT */

#ifdef VALGRIND_DEBUG

/* This is used to force the generation of code (machine instructions) for assignments to this variable. */
/* Specifically we want to avoid the situation where the compiler decides to optimize out the assignment. */
static int _dpp_wb_engine_valgrind_init_check_var = 0;

/* May be used for debugging VALGRIND errors. */
/* When enabled this code "uses" all the contents of var. */
/* "Uses" in this context means doing a branch depending on the contents of var. */
/* This is done to allow Valgrind to throw an error when uninitialized memory is written */
/* into a warmboot variable. */
/* If you have a Valgrind error here, then you are probably writing uninitialized memory to a warmboot array/variable. */
STATIC void _dpp_wb_engine_valgrind_check_array_is_initialized(const void *arr, uint32 outer_length, uint32 inner_length, uint32 outer_jump, uint32 inner_jump, uint32 data_size, int var)
{
  uint32 outer_arr_ndx, inner_arr_ndx;
  const uint8 *src;
  int i;

  if (arr == NULL) {
    return;
  }

  if (outer_jump == 0xffffffff) {
    outer_jump = inner_length * data_size;
  }

  if (inner_jump == 0xffffffff) {
    inner_jump = data_size;
  }

  for (outer_arr_ndx = 0; outer_arr_ndx < outer_length; outer_arr_ndx++) {
    for (inner_arr_ndx = 0; inner_arr_ndx < inner_length; inner_arr_ndx++) {

      src = (uint8*)arr + (outer_arr_ndx * outer_jump) + (inner_arr_ndx * inner_jump);

      for (i = 0; i < data_size; i++) {
        int byte = src[i];
        if (byte) {
          _dpp_wb_engine_valgrind_init_check_var = byte;
        } else {
          _dpp_wb_engine_valgrind_init_check_var = byte - 1;
       }
      }
    }
  }
}
#else /* VALGRIND_DEBUG */
#endif /* VALGRIND_DEBUG */

STATIC int soc_dpp_wb_engine_init_tables(int unit, int buffer_id);



int soc_dpp_wb_engine_init_buffer(int unit, int buffer_id)
{
    int rc = SOC_E_NONE;
    SOCDNX_INIT_FUNC_DEFS;

    /* 
     * init the buffer info and buffer's variables info in wb engine tables, 
     * each buffer init it's own data. 
     * note that the location of original data is different in each boot.
     */ 
    rc = soc_dpp_wb_engine_init_tables(unit, buffer_id);
    if (rc != SOC_E_NONE) {
        SOCDNX_IF_ERR_EXIT(rc);
    }

    rc = soc_wb_engine_init_buffer(unit, SOC_DPP_WB_ENGINE, buffer_id, FALSE);
    if (rc != SOC_E_NONE) {
        SOCDNX_IF_ERR_EXIT(rc);
    }

exit:
    SOCDNX_FUNC_RETURN;
}

/* 
 * init of engine tables is done per buffer due to timing issues. 
 * init of a variable should be done only after variable location is determined 
 * (if vars are dynamically alocated then init must be called after relevant allocations occured)  
 */ 

/* general definitions to be included at the beginning of engine's init_tables functiom  */
/* these defs are later used by ADD_DYNAMIC_BUFF and ADD_DYNAMIC_VAR_WITH_FEATURES below */

STATIC int
soc_dpp_wb_engine_init_tables(int unit, int buffer_id)
{
    int rv = SOC_E_NONE;
    WB_ENGINE_INIT_TABLES_DEFS;
    SOCDNX_INIT_FUNC_DEFS;
    sal_memset(&tmp_var_info, 0, sizeof(tmp_var_info));

    /* all of dpp_wb_engine depracated code was packed into this macro */
    /* make sure not to use any of the logic inside it and avoid making any changes to it */
    /* unless it's a crucial bugfix */
    SOC_DPP_WB_ENGINE_DEPRECATED_CODE;

#ifdef BCM_ARAD_SUPPORT

        SOC_DPP_WB_ENGINE_DECLARATIONS_BEGIN

        /* !! dont add new declarations below this point !! */
        SOC_DPP_WB_ENGINE_DECLARATIONS_END

#endif /* BCM_ARAD_SUPPORT */

    /* validate that only one non-empty buffer is initialized per function call*/
    SOC_WB_ENGINE_INIT_TABLES_SANITY(rv);
    SOCDNX_IF_ERR_EXIT(rv);

exit:
    SOCDNX_FUNC_RETURN;
}




/* is called on device init to allocate the engine's tables */
int
soc_dpp_wb_engine_init(int unit)
{
    /* init pointers array - to be used to support addition of sw_state that was created prior to wb_engine*/
    return soc_wb_engine_init_tables(unit, SOC_DPP_WB_ENGINE, SOC_DPP_WB_ENGINE_NOF_BUFFERS, SOC_DPP_WB_ENGINE_VAR_NOF_VARS);
}

int
soc_dpp_wb_engine_deinit(int unit)
{
#ifdef BCM_ARAD_SUPPORT
    /* return static vars to default values*/
    sal_memset(&_dpp_oam_is_init[unit] ,0, sizeof(uint8));
    sal_memset(&_dpp_bfd_is_init[unit] ,0, sizeof(uint8));
#endif
    return soc_wb_engine_deinit_tables(unit, SOC_DPP_WB_ENGINE);
}

#ifdef BCM_WARM_BOOT_SUPPORT

int
soc_dpp_wb_engine_sync(int unit)
{
#if (0)
/* { */
#ifdef BCM_ARAD_SUPPORT
#ifdef BCM_WARM_BOOT_SUPPORT
    int i;
    int rv;
#endif
#endif

#ifdef BCM_ARAD_SUPPORT
#ifdef BCM_WARM_BOOT_SUPPORT   
    /* prepare bcm hash tables data to be saved to external storage*/
    for (i=0; i<SOC_DPP_WB_HASH_TBLS_NUM; i++) {
        rv = dpp_fill_wb_arrays_from_hash_tbl(unit,i);
        if(rv != SOC_E_NONE){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit:%d failed to prepare hash table %d for wb during soc_dpp_wb_engine_sync\n"),
                       unit, i));
            return rv;
        }
    }
#endif
#endif
/* } */
#endif

    return soc_wb_engine_sync(unit, SOC_DPP_WB_ENGINE);
}

#endif /*BCM_WARM_BOOT_SUPPORT*/

/* defined at the beginning of the module */
#undef _ERR_MSG_MODULE_NAME
#undef NEVER_REMOVED
#undef NO_FLAGS
#undef DEFAULT
#undef VERSION
#undef _NO_JUMP_NEEDED_



