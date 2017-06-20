/*
 * $Id: caladan3_cint_data.c,v 1.2 Broadcom SDK $
 *
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 *
 * caladan3_cint_data.c: Guadalupe2k V1.3 C-Interpreter functions
 *
 * This file contains the manually created portions of the 
 * g3p1 SoC CINT API.  Most of these will become auto generated.
 */

int caladan3_cint_data_not_empty;
#include <sdk_config.h>
#if defined(INCLUDE_LIB_CINT)
#if defined(BCM_CALADAN3_SUPPORT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <sal/core/libc.h>

#include <soc/sbx/caladan3.h>
#include <soc/sbx/caladan3/sws.h>
#include <soc/sbx/caladan3/list.h>

#define UTG_MALLOC(x) sal_alloc((x), "utg");

/* Static symbol name for initialization */
char soc_sbx_caladan3_sym[128];

/* Only for debugging */
char *soc_sbx_caladan3_errsym;

static void
__cint_fpointer__soc_sbx_caladan3_lrp_list_dequeue_cb_f(int unit,
                                                        void *dequeue_context, 
                                                        int prev_offset,
                                                        int nentries);
static void
__cint_fpointer__soc_sbx_caladan3_lrp_list_enqueue_cb_f(int unit,
                                                        void *enqueue_context, 
                                                        int prev_offset,
                                                        int nentries);


static cint_parameter_desc_t __cint_struct_members__soc_sbx_caladan3_pr_icc_lookup_data_t[] = {
    {
       "uint32",
       "shift",
       0,
       0
     },
    {
       "uint32",
       "select_de",
       0,
       0
     },
    {
       "uint32",
       "queue_action",
       0,
       0
     },
      {
       "uint32",
       "queue",
       0,
       0
     },
      {
       "uint32",
       "last",
       0,
       0
     },
      {
       "uint32",
       "drop",
       0,
       0
     },
      {
       "uint32",
       "dp",
       0,
       0
     },
      {
       "uint32",
       "default_de",
       0,
       0
     },
      {
       "uint32",
       "cos",
       0,
       0
     },
    { NULL }
};


static void*
__cint_maddr__soc_sbx_caladan3_pr_icc_lookup_data_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    soc_sbx_caladan3_pr_icc_lookup_data_t* s = (soc_sbx_caladan3_pr_icc_lookup_data_t*) p;

  switch (mnum) {
    case 0: rv = &(s->shift); break;
    case 1: rv = &(s->select_de); break;
    case 2: rv = &(s->queue_action); break;
    case 3: rv = &(s->queue); break;
    case 4: rv = &(s->last); break;
    case 5: rv = &(s->drop); break;
    case 6: rv = &(s->dp); break;
    case 7: rv = &(s->default_de); break;
    case 8: rv = &(s->cos); break;
    default: rv = NULL; break;
  }

    return rv;
}


CINT_FWRAPPER_CREATE_VP9(soc_sbx_caladan3_sws_pr_icc_tcam_program, 
                         int, int,      unit, 0, 0, 
                         int, int,      pr,   0, 0, 
                         int, int,      idx,  0, 0, 
                         int, int,      valid,  0, 0, 
                         uint8*, uint8, key,  1, 0, 
                         uint8*, uint8, mask, 1, 0, 
                         uint8*, uint8, state,  1, 0, 
                         uint8*, uint8, state_mask, 1, 0, 
                         soc_sbx_caladan3_pr_icc_lookup_data_t*,
                         soc_sbx_caladan3_pr_icc_lookup_data_t, data, 1, 0);

CINT_FWRAPPER_CREATE_RP10(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_init,
                         int, int, unit, 0, 0,
                         soc_sbx_caladan3_lrp_svp_t,
                             soc_sbx_caladan3_lrp_svp_t, svp, 0, 0,
                         soc_sbx_caladan3_lrp_list_mode_e_t,
                             soc_sbx_caladan3_lrp_list_mode_e_t, mode, 0, 0,
                         soc_sbx_caladan3_lrp_list_type_e_t,
                             soc_sbx_caladan3_lrp_list_type_e_t, type, 0, 0,
                         soc_sbx_caladan3_lrp_list_esize_e_t,
                             soc_sbx_caladan3_lrp_list_esize_e_t, entry_size, 0, 0,
                         uint32, uint32, num_entries, 0, 0,
                         int, int, enqueue_threshold, 0, 0,
                         int, int, dqueue_threshold, 0, 0,
                         int*, int, listmgr_id, 1, 0, 
                         int, int, reinit, 0, 0);

CINT_FWRAPPER_CREATE_RP6(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_memory_alloc,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         int, int, port, 0, 0,
                         int, int, segment, 0, 0,
                         int, int, num_entries, 0, 0,
                         int, int, entry_size, 0, 0);

CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_memory_free,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_enqueue_event_enable,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         int, int, enable, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_dequeue_event_enable,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         int, int, enable, 0, 0);

CINT_FWRAPPER_CREATE_RP6(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_register_callback,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         soc_sbx_caladan3_lrp_list_enqueue_cb_f,
                             soc_sbx_caladan3_lrp_list_enqueue_cb_f, enq_func, 0, 0,
                         void *, void, enq_context, 1, 0,
                         soc_sbx_caladan3_lrp_list_dequeue_cb_f,
                             soc_sbx_caladan3_lrp_list_dequeue_cb_f, deq_func, 0, 0,
                         void *, void, deq_context, 1, 0);

CINT_FWRAPPER_CREATE_RP4(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_dequeue_item,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         uint8*, uint8, data, 1, 0,
                         int*, int, length, 1, 0);

CINT_FWRAPPER_CREATE_RP4(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_enqueue_item,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         uint8*, uint8, data, 1, 0,
                         int, int, length, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_set_enqueue_offset,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         int, int, offset, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0,
                         soc_sbx_caladan3_lr_list_manager_set_dequeue_offset,
                         int, int, unit, 0, 0,
                         int, int, listmgr_id, 0, 0,
                         int, int, offset, 0, 0);


static cint_function_t __cint_caladan3_functions [] =
{
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_sws_pr_icc_tcam_program),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_init),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_memory_alloc),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_memory_free),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_enqueue_event_enable),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_dequeue_event_enable),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_register_callback),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_enqueue_item),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_dequeue_item),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_set_dequeue_offset),
    CINT_FWRAPPER_ENTRY(soc_sbx_caladan3_lr_list_manager_set_enqueue_offset),
    { NULL }
};

static cint_parameter_desc_t __cint_parameters__soc_sbx_caladan3_lrp_list_dequeue_cb_f[] =
{
    { "int", "unit", 0, 0},
    { "void*", "dequeue_context", 1, 0},
    { "int", "prev_offset", 0, 0},
    { "int", "nentries", 0, 0},
    { NULL }
 };
static cint_parameter_desc_t __cint_parameters__soc_sbx_caladan3_lrp_list_enqueue_cb_f[] =
{
    { "int", "unit", 0, 0},
    { "void*", "enqueue_context", 1, 0},
    { "int", "prev_offset", 0, 0},
    { "int", "nentries", 0, 0},
    { NULL }
 };


static cint_function_pointer_t __cint_caladan3_function_pointers[] =
{
    {
        "soc_sbx_caladan3_lrp_list_enqueue_cb_f",
        (cint_fpointer_t) __cint_fpointer__soc_sbx_caladan3_lrp_list_enqueue_cb_f,
        __cint_parameters__soc_sbx_caladan3_lrp_list_enqueue_cb_f
    },
    {
        "soc_sbx_caladan3_lrp_list_dequeue_cb_f",
        (cint_fpointer_t) __cint_fpointer__soc_sbx_caladan3_lrp_list_dequeue_cb_f,
        __cint_parameters__soc_sbx_caladan3_lrp_list_dequeue_cb_f
    },
};

static void
__cint_fpointer__soc_sbx_caladan3_lrp_list_dequeue_cb_f(int unit,
                                                        void *dequeue_context, 
                                                        int prev_offset,
                                                        int nentries)
{
    cint_interpreter_callback(__cint_caladan3_function_pointers + 1, 
                              4, 0,
                              &unit, &dequeue_context, &prev_offset, &nentries);
}


static void
__cint_fpointer__soc_sbx_caladan3_lrp_list_enqueue_cb_f(int unit,
                                                        void *enqueue_context, 
                                                        int prev_offset,
                                                        int nentries)
{
    cint_interpreter_callback(__cint_caladan3_function_pointers + 0, 
                              4, 0,
                              &unit, &enqueue_context, &prev_offset, &nentries);
}



static cint_struct_type_t __cint_caladan3_structures [] = 
{
  {
      "soc_sbx_caladan3_pr_icc_lookup_data_t",
      sizeof(soc_sbx_caladan3_pr_icc_lookup_data_t),
      __cint_struct_members__soc_sbx_caladan3_pr_icc_lookup_data_t,
      __cint_maddr__soc_sbx_caladan3_pr_icc_lookup_data_t
  },    
  { NULL }
};

static cint_enum_map_t __cint_enum_map_soc_sbx_caladan3_lrp_list_operation_t[]=
{
    { "SOC_SBX_CALADAN3_LRP_LIST_GET_READ_PTR", SOC_SBX_CALADAN3_LRP_LIST_GET_READ_PTR },
    { "SOC_SBX_CALADAN3_LRP_LIST_DEQUEUE", SOC_SBX_CALADAN3_LRP_LIST_DEQUEUE },
    { "SOC_SBX_CALADAN3_LRP_LIST_GET_WRITE_PTR", SOC_SBX_CALADAN3_LRP_LIST_GET_WRITE_PTR },
    { "SOC_SBX_CALADAN3_LRP_LIST_ENQUEUE", SOC_SBX_CALADAN3_LRP_LIST_ENQUEUE }
};

static cint_enum_map_t __cint_enum_map_soc_sbx_caladan3_lrp_list_type_e_t[] = 
{
    { "SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL", SOC_SBX_CALADAN3_LRP_LIST_TYPE_WHEEL },
    { "SOC_SBX_CALADAN3_LRP_LIST_TYPE_LIST", SOC_SBX_CALADAN3_LRP_LIST_TYPE_LIST },
    { "SOC_SBX_CALADAN3_LRP_LIST_TYPE_BIDIR", SOC_SBX_CALADAN3_LRP_LIST_TYPE_BIDIR },
    { NULL }
};

static cint_enum_map_t __cint_enum_map_soc_sbx_caladan3_lrp_list_mode_e_t[] =
{
    { "SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL", SOC_SBX_CALADAN3_LRP_LIST_MODE_TRADITIONAL },
    { "SOC_SBX_CALADAN3_LRP_LIST_MODE_HOST_TO_UCODE", SOC_SBX_CALADAN3_LRP_LIST_MODE_HOST_TO_UCODE },
    { "SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_HOST", SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_HOST },
    { "SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE", SOC_SBX_CALADAN3_LRP_LIST_MODE_UCODE_TO_UCODE },
    { NULL }
};
static cint_enum_map_t __cint_enum_map_soc_sbx_caladan3_lrp_list_esize_e_t[] =
{
    { "SOC_SBX_CALADAN3_1B", SOC_SBX_CALADAN3_1B },
    { "SOC_SBX_CALADAN3_2B", SOC_SBX_CALADAN3_2B },
    { "SOC_SBX_CALADAN3_4B", SOC_SBX_CALADAN3_4B },
    { "SOC_SBX_CALADAN3_8B", SOC_SBX_CALADAN3_8B },
    { "SOC_SBX_CALADAN3_16B", SOC_SBX_CALADAN3_16B },
    { "SOC_SBX_CALADAN3_32B", SOC_SBX_CALADAN3_32B },
    { "SOC_SBX_CALADAN3_64B", SOC_SBX_CALADAN3_64B },
    { "SOC_SBX_CALADAN3_128B", SOC_SBX_CALADAN3_128B },
    { "SOC_SBX_CALADAN3_256B", SOC_SBX_CALADAN3_256B },
    { "SOC_SBX_CALADAN3_512B", SOC_SBX_CALADAN3_512B },
    { "SOC_SBX_CALADAN3_1024B", SOC_SBX_CALADAN3_1024B },
    { "SOC_SBX_CALADAN3_2048B", SOC_SBX_CALADAN3_2048B },
    { "SOC_SBX_CALADAN3_4096B", SOC_SBX_CALADAN3_4096B },
    { "SOC_SBX_CALADAN3_8192B", SOC_SBX_CALADAN3_8192B },
    { "SOC_SBX_CALADAN3_16384B", SOC_SBX_CALADAN3_16384B },
    { "SOC_SBX_CALADAN3_32768B", SOC_SBX_CALADAN3_32768B },
    { NULL }
};

static cint_enum_map_t __cint_enum_map_soc_sbx_caladan3_lrp_svp_t[] =
{
    { "SOC_SBX_CALADAN3_SVP0", SOC_SBX_CALADAN3_SVP0 },
    { "SOC_SBX_CALADAN3_SVP1", SOC_SBX_CALADAN3_SVP1 },
    { "SOC_SBX_CALADAN3_SVP2", SOC_SBX_CALADAN3_SVP2 },
    { "SOC_SBX_CALADAN3_SVP3", SOC_SBX_CALADAN3_SVP3 },
    { "SOC_SBX_CALADAN3_SVP4", SOC_SBX_CALADAN3_SVP4 },
    { NULL }
};

static cint_enum_type_t __cint_caladan3_enums[] =
{
    { "soc_sbx_caladan3_lrp_list_type_e_t", __cint_enum_map_soc_sbx_caladan3_lrp_list_type_e_t},
    { "soc_sbx_caladan3_lrp_list_mode_e_t", __cint_enum_map_soc_sbx_caladan3_lrp_list_mode_e_t},
    { "soc_sbx_caladan3_lrp_svp_t", __cint_enum_map_soc_sbx_caladan3_lrp_svp_t},
    { "soc_sbx_caladan3_lrp_list_operation_t", __cint_enum_map_soc_sbx_caladan3_lrp_list_operation_t},
    { "soc_sbx_caladan3_lrp_list_esize_e_t", __cint_enum_map_soc_sbx_caladan3_lrp_list_esize_e_t},
    { NULL }
};



/* 
 * CINT exported datastructure
 */ 
cint_data_t caladan3_cint_data =
{
    NULL,
    __cint_caladan3_functions,
    __cint_caladan3_structures,
    __cint_caladan3_enums,       /* __cint_caladan3_enums */
    NULL,   /* __cint_caladan3_typedefs */
    NULL,   /*__cint_caladan3_constants */
    __cint_caladan3_function_pointers    /*__cint_caladan3_function_pointers */
};

#endif /* BCM_CALADAN3_SUPPORT */
#endif /* INCLUDE_LIB_CINT */
