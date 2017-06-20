/*
 * $Id: g3p1_cint_data.c,v 1.44 Broadcom SDK $
 *
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * 
 *
 * g3p1_cint_data.c: Guadalupe2k V1.3 C-Interpreter functions
 *
 * This file contains the manually created portions of the 
 * g3p1 SoC CINT API.  Most of these will become auto generated.
 */

int g3p1_cint_data_not_empty;
#include <sdk_config.h>
#if defined(INCLUDE_LIB_CINT)
#if defined(BCM_CALADAN3_SUPPORT) && defined(BCM_CALADAN3_G3P1_SUPPORT)

#include <cint_config.h>
#include <cint_types.h>
#include <cint_porting.h>
#include <sal/core/libc.h>

#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/caladan3.h>

#define UTG_MALLOC(x) sal_alloc((x), "utg");

/* Static symbol name for initialization */
char soc_sbx_g3p1_sym[128];

/* Only for debugging */
char *soc_sbx_g3p1_errsym;


static cint_parameter_desc_t __cint_parameters__soc_sbx_g3p1_util_timer_event_callback_f[] =
{
    {
        "void",
        "r",
        0,
        0
    },
    {
        "int",
        "unit",
        0,
        0
    },
    {
        "soc_sbx_g3p1_util_timer_event_t",
        "event",
        1,
        0
    },
    {
        "void",
        "user_cookie",
        1,
        0
    },
    CINT_ENTRY_LAST
};


static void
__cint_fpointer__soc_sbx_g3p1_util_timer_event_callback_f(int unit,
                                                         soc_sbx_g3p1_util_timer_event_t *event,
                                                          void *user_cookie);

static cint_parameter_desc_t __cint_struct_members__soc_sbx_g3p1_util_timer_event_t[] = {
    {
       "uint32",
       "timer_segment",
       0,
       0
     },
    {
       "uint32",
       "id",
       0,
       0
     },
    {
       "uint8",
       "forced_timeout",
       0,
       0
     },
      {
       "uint8",
       "timer_active_when_forced",
       0,
       0
     },
    { NULL }
};




static void*
__cint_maddr__soc_sbx_g3p1_util_timer_event_t(void* p, int mnum, cint_struct_type_t* parent)
{
    void* rv;
    soc_sbx_g3p1_util_timer_event_t* s = (soc_sbx_g3p1_util_timer_event_t*) p;

  switch (mnum) {
    case 0: rv = &(s->timer_segment); break;
    case 1: rv = &(s->id); break;
    case 2: rv = &(s->forced_timeout); break;
    case 3: rv = &(s->timer_active_when_forced); break;
    default: rv = NULL; break;
  }

    return rv;
}


static cint_function_pointer_t __cint_g3p1_function_pointers[] =
{
    {
        "soc_sbx_g3p1_util_timer_event_callback_f",
        (cint_fpointer_t) __cint_fpointer__soc_sbx_g3p1_util_timer_event_callback_f,
        __cint_parameters__soc_sbx_g3p1_util_timer_event_callback_f
    },
    CINT_ENTRY_LAST
};

static void
__cint_fpointer__soc_sbx_g3p1_util_timer_event_callback_f(int unit,
							 soc_sbx_g3p1_util_timer_event_t *event,
							 void *user_cookie)
{

    cint_interpreter_callback(__cint_g3p1_function_pointers+0,
                              3,
                              0,
                              &unit,
                              &event,
                              &user_cookie);


}
CINT_FWRAPPER_CREATE2_RP1(int, int, 0, 0, soc_sbx_g3p1_bubble_table_init,
                          int, int, unit, 0, 0, CINT_PARAM_IN);

CINT_FWRAPPER_CREATE2_RP9(int, int, 0, 0, soc_sbx_g3p1_bubble_entry_set,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, bubble_idx, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, count, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, interval_index, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, task, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, stream, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, init, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, jitter_enable, 0, 0, CINT_PARAM_IN,
                          uint8, uint8, update_mode, 0, 0, CINT_PARAM_IN);
CINT_FWRAPPER_CREATE2_RP9(int, int, 0, 0, soc_sbx_g3p1_bubble_entry_get,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          uint32, uint32, bubble_idx, 0, 0, CINT_PARAM_IN,
                          uint8*, uint8, count, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, interval_index, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, task, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, stream, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, init, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, jitter_enable, 1, 0, CINT_PARAM_OUT,
                          uint8*, uint8, update_mode, 1, 0, CINT_PARAM_OUT);

CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0, soc_sbx_g3p1_init, 
                         int, int, unit, 0, 0,     
                         void*, void, ucode, 1, 0);

CINT_FWRAPPER_CREATE2_RP1(uint32, uint32, 0, 0, soc_sbx_g3p1_util_crc32_word,
                          uint32, uint32, x, 0, 0, CINT_PARAM_IN);

CINT_FWRAPPER_CREATE2_RP7(int,  int, 0, 0, soc_sbx_g3p1_timer_create,
                          int,  int, unit, 0, 0, CINT_PARAM_IN,
                          int,  int, timer_segment, 0, 0, CINT_PARAM_IN,
                          int,  int, timeout_ticks, 0, 0, CINT_PARAM_IN,
                          uint32*, uint32, timer_handle, 1, 0, CINT_PARAM_INOUT,
                          int,  int, id, 0, 0, CINT_PARAM_IN,
                          int,  int, doInterrupt, 0, 0, CINT_PARAM_IN,
                          int,  int, start, 0, 0, CINT_PARAM_IN);

CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0, soc_sbx_g3p1_ppe_entry_psc_set,
                         int, int, unit, 0, 0,
                         int, int, psc, 0, 0);

CINT_FWRAPPER_CREATE_RP2(int, int, 0, 0, soc_sbx_g3p1_ppe_entry_psc_get,
                         int, int, unit, 0, 0,
                         unsigned int*, unsigned int, psc, 1, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_g3p1_ppe_queue_psc_set,
                         int, int, unit, 0, 0,
                         int, int, queue, 0, 0,
                         int, int, psc, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_g3p1_ppe_queue_psc_get,
                         int, int, unit, 0, 0,
                         int, int, queue, 0, 0,
                         unsigned int*, unsigned int, psc, 1, 0);

CINT_FWRAPPER_CREATE2_RP3(int, int, 0, 0, soc_sbx_g3p1_util_register_timer_callback,
                          int, int, unit, 0, 0, CINT_PARAM_IN,
                          soc_sbx_g3p1_util_timer_event_callback_f, soc_sbx_g3p1_util_timer_event_callback_f, cb, 0, 0, CINT_PARAM_IN,
                          void*, void, user_cookie, 1, 0, CINT_PARAM_IN);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_g3p1_iqsm_stream_set,
                         int, int, unit, 0, 0,
                         int, int, queue, 0, 0,
                         int, int, str, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_g3p1_iqsm_stream_get,
                         int, int, unit, 0, 0,
                         int, int, queue, 0, 0,
                         unsigned int*, unsigned int, str, 1, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_g3p1_iqsm_checker_set,
                         int, int, unit, 0, 0,
                         int, int, queue, 0, 0,
                         int, int, checker, 0, 0);

CINT_FWRAPPER_CREATE_RP3(int, int, 0, 0, soc_sbx_g3p1_iqsm_checker_get,
                         int, int, unit, 0, 0,
                         int, int, queue, 0, 0,
                         unsigned int*, unsigned int, checker, 1, 0);

CINT_FWRAPPER_CREATE_RP5(int, int, 0, 0, soc_sbx_g3p1_mac_bulk_delete,
                         int, int, unit, 0, 0,
                         uint32*, uint32, filter_key, 1, 0,
                         uint32*, uint32, filter_key_mask, 1, 0,
                         uint32*, uint32, filter_value, 1, 0,
                         uint32*, uint32, filter_value_mask, 1, 0);

static cint_function_t __cint_g3p1_functions [] =
{
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_init),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_bubble_table_init),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_bubble_entry_set),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_bubble_entry_get),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_util_crc32_word),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_timer_create),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_util_register_timer_callback),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_ppe_entry_psc_get),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_ppe_entry_psc_set),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_ppe_queue_psc_get),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_ppe_queue_psc_set),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_iqsm_stream_set),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_iqsm_stream_get),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_iqsm_checker_set),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_iqsm_checker_get),
    CINT_FWRAPPER_ENTRY(soc_sbx_g3p1_mac_bulk_delete),
    { NULL }
};



static cint_constants_t __cint_g3p1_constants [] =
{
   { "SOC_SBX_G3P1_TABLE_BANKS_MAX", SOC_SBX_G3P1_TABLE_BANKS_MAX },
   { "SB_G3P1_PSC_MAC_DA",    SB_G3P1_PSC_MAC_DA    },
   { "SB_G3P1_PSC_MAC_SA",    SB_G3P1_PSC_MAC_SA    },
   { "SB_G3P1_PSC_IP_DA",     SB_G3P1_PSC_IP_DA     },
   { "SB_G3P1_PSC_IP_SA",     SB_G3P1_PSC_IP_SA     },
   { "SB_G3P1_PSC_L4SS",      SB_G3P1_PSC_L4SS      },
   { "SB_G3P1_PSC_L4DS",      SB_G3P1_PSC_L4DS      },
   { "SB_G3P1_PSC_VID",       SB_G3P1_PSC_VID       },
   { "SB_G3P1_PSC_VID_INNER", SB_G3P1_PSC_VID_INNER },
   { NULL }
};


static cint_struct_type_t __cint_g3p1_structures [] = 
{
  {
      "soc_sbx_g3p1_util_timer_event_t",
      sizeof(soc_sbx_g3p1_util_timer_event_t),
      __cint_struct_members__soc_sbx_g3p1_util_timer_event_t,
      __cint_maddr__soc_sbx_g3p1_util_timer_event_t
  },    
  { NULL }
};

/* 
 * CINT exported datastructure
 */ 
cint_data_t g3p1_cint_data =
{
    NULL,
    __cint_g3p1_functions,
    __cint_g3p1_structures,
    NULL,   /* __cint_g3p1_enums */
    NULL,   /* __cint_g3p1_typedefs */
    __cint_g3p1_constants,
    __cint_g3p1_function_pointers 
};

#endif /* BCM_CALADAN3_SUPPORT */
#endif /* INCLUDE_LIB_CINT */
