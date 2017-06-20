/*
 * $Id: dnx_wb_engine.c,v 1.75 Broadcom SDK $
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
#include <soc/dnxc/legacy/error.h>

#include <soc/types.h>
#include <soc/error.h>
#include <soc/scache.h>
#include <soc/drv.h>

#ifdef VALGRIND_DEBUG
/* Used for VALGRIND_DEBUG. See _dnx_wb_engine_valgrind_check_array_is_initialized for details. */
#endif 

#include <soc/wb_engine.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/dnx_wb_engine.h>
#include <soc/dnx/legacy/dnx_wb_engine_defs.h>
#include <soc/dnx/legacy/SAND/Utils/sand_framework.h>
#include <soc/dnx/legacy/ARAD/arad_sw_db.h>




/* undefined at the end of the module */
#define NEVER_REMOVED 0xff
#define NO_FLAGS 0x0
#define DEFAULT 0xffffffff
#define VERSION(_ver) (_ver)


/*jer2_arad_sw_db*/
JER2_ARAD_SW_DB Jer2_arad_sw_db;


#ifdef VALGRIND_DEBUG

/* This is used to force the generation of code (machine instructions) for assignments to this variable. */
/* Specifically we want to avoid the situation where the compiler decides to optimize out the assignment. */
static int _dnx_wb_engine_valgrind_init_check_var = 0;

/* May be used for debugging VALGRIND errors. */
/* When enabled this code "uses" all the contents of var. */
/* "Uses" in this context means doing a branch depending on the contents of var. */
/* This is done to allow Valgrind to throw an error when uninitialized memory is written */
/* into a warmboot variable. */
/* If you have a Valgrind error here, then you are probably writing uninitialized memory to a warmboot array/variable. */
static void _dnx_wb_engine_valgrind_check_array_is_initialized(const void *arr, uint32 outer_length, uint32 inner_length, uint32 outer_jump, uint32 inner_jump, uint32 data_size, int var)
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
          _dnx_wb_engine_valgrind_init_check_var = byte;
        } else {
          _dnx_wb_engine_valgrind_init_check_var = byte - 1;
       }
      }
    }
  }
}
#else /* VALGRIND_DEBUG */
#endif /* VALGRIND_DEBUG */

static int soc_dnx_wb_engine_init_tables(int unit, int buffer_id);

int soc_dnx_wb_engine_init_buffer(int unit, int buffer_id)
{
    int rc = SOC_E_NONE;
    DNXC_INIT_FUNC_DEFS;

    /* 
     * init the buffer info and buffer's variables info in wb engine tables, 
     * each buffer init it's own data. 
     * note that the location of original data is different in each boot.
     */ 
    rc = soc_dnx_wb_engine_init_tables(unit, buffer_id);
    if (rc != SOC_E_NONE) {
        DNXC_IF_ERR_EXIT(rc);
    }

    rc = soc_wb_engine_init_buffer(unit, SOC_DNX_WB_ENGINE, buffer_id, FALSE);
    if (rc != SOC_E_NONE) {
        DNXC_IF_ERR_EXIT(rc);
    }

exit:
    DNXC_FUNC_RETURN;
}

/* 
 * init of engine tables is done per buffer due to timing issues. 
 * init of a variable should be done only after variable location is determined 
 * (if vars are dynamically alocated then init must be called after relevant allocations occured)  
 */ 

/* general definitions to be included at the beginning of engine's init_tables functiom  */
/* these defs are later used by ADD_DYNAMIC_BUFF and ADD_DYNAMIC_VAR_WITH_FEATURES below */

static int
soc_dnx_wb_engine_init_tables(int unit, int buffer_id)
{
    
#ifdef FIXME_DNX_LEGACY
    int rv = SOC_E_NONE;
    WB_ENGINE_INIT_TABLES_DEFS;
    DNXC_INIT_FUNC_DEFS;
    sal_memset(&tmp_var_info, 0, sizeof(tmp_var_info));

    /* all of dnx_wb_engine depracated code was packed into this macro */
    /* make sure not to use any of the logic inside it and avoid making any changes to it */
    /* unless it's a crucial bugfix */
    SOC_DNX_WB_ENGINE_DEPRECATED_CODE;

#ifdef BCM_JER2_ARAD_SUPPORT

        SOC_DNX_WB_ENGINE_DECLARATIONS_BEGIN

        /* !! dont add new declarations below this point !! */
        SOC_DNX_WB_ENGINE_DECLARATIONS_END

#endif /* BCM_JER2_ARAD_SUPPORT */

    /* validate that only one non-empty buffer is initialized per function call*/
    SOC_WB_ENGINE_INIT_TABLES_SANITY(rv);
    DNXC_IF_ERR_EXIT(rv);

exit:
    DNXC_FUNC_RETURN;
#else 
    return -1;
#endif 
}




/* is called on device init to allocate the engine's tables */
int
soc_dnx_wb_engine_init(int unit)
{
    /* init pointers array - to be used to support addition of sw_state that was created prior to wb_engine*/
    return soc_wb_engine_init_tables(unit, SOC_DNX_WB_ENGINE, SOC_DNX_WB_ENGINE_NOF_BUFFERS, SOC_DNX_WB_ENGINE_VAR_NOF_VARS);
}

int
soc_dnx_wb_engine_deinit(int unit)
{
#ifdef BCM_JER2_ARAD_SUPPORT
    /* return static vars to default values*/
    sal_memset(&_dnx_oam_is_init[unit] ,0, sizeof(uint8));
    sal_memset(&_dnx_bfd_is_init[unit] ,0, sizeof(uint8));
#endif
    return soc_wb_engine_deinit_tables(unit, SOC_DNX_WB_ENGINE);
}

#ifdef BCM_WARM_BOOT_SUPPORT

int
soc_dnx_wb_engine_sync(int unit)
{
#if (0)
/* { */
#ifdef BCM_JER2_ARAD_SUPPORT
#ifdef BCM_WARM_BOOT_SUPPORT
    int i;
    int rv;
#endif
#endif

#ifdef BCM_JER2_ARAD_SUPPORT
#ifdef BCM_WARM_BOOT_SUPPORT   
    /* prepare bcm hash tables data to be saved to external storage*/
    for (i=0; i<SOC_DNX_WB_HASH_TBLS_NUM; i++) {
        rv = dnx_fill_wb_arrays_from_hash_tbl(unit,i);
        if(rv != SOC_E_NONE){
            LOG_ERROR(BSL_LS_SOC_COMMON,
                      (BSL_META_U(unit,
                                  "Unit:%d failed to prepare hash table %d for wb during soc_dnx_wb_engine_sync\n"),
                       unit, i));
            return rv;
        }
    }
#endif
#endif
/* } */
#endif

    return soc_wb_engine_sync(unit, SOC_DNX_WB_ENGINE);
}

#endif /*BCM_WARM_BOOT_SUPPORT*/

/* defined at the beginning of the module */
#undef _ERR_MSG_MODULE_NAME
#undef NEVER_REMOVED
#undef NO_FLAGS
#undef DEFAULT
#undef VERSION
#undef _NO_JUMP_NEEDED_



