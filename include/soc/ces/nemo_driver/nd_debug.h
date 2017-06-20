/*
 * $Id$
 *
 * Copyright: (c) 2011 BATM
 */
#ifndef __NEMO_DEBUG_H__
#define __NEMO_DEBUG_H__
 
#include <stdarg.h>
#include "pub/nd_hw.h"
#include "pub/nd_api.h"

#ifdef __cplusplus
extern "C"
{
#endif


extern AG_U32 g_ndTraceMask;
extern AG_U16 g_ndTraceLevel;
extern AG_U32 g_ndTraceBusAddr[];
extern AG_U16 g_ndTraceBusAddrSize;
extern AG_BOOL g_force_kiss_out; /*BCMadd*/

#ifndef  BCM_CES_SDK
#pragma check_printf_formats
void ag_nd_trace(AgNdDevice *p_device, AG_U32 n_mask, AG_U16 n_level, const char *p_file, int n_line, const char *p_format, ...);
#pragma no_check_printf_formats
#endif

void ag_nd_vtrace(AgNdDevice *p_device, AG_U32 n_mask, AG_U16 n_level, const char *p_file, int n_line, const char *p_format, va_list argp);
void ag_nd_set_kiss_debug(AG_BOOL force); /*BCMadd*/
#define AG_ND_TRACE_API_CALL    "--- "

#ifdef  BCM_CES_SDK
        #define AG_ND_TRACE(device, mask, level, args...)
        #define AG_ND_ERR(device, code) code
        #define assert(ex)
#else
/* */
/* AG_ND_TRACE  */
/*  */
/* */
#ifdef WIN32

    void AG_ND_TRACE(AgNdDevice *p_device, AG_U32 n_mask, AG_U16 n_level, const char *p_format, ...);

#else /* WIN32 */

    #ifdef AG_ND_ENABLE_TRACE
        #define AG_ND_TRACE(device, mask, level, ...) ag_nd_trace(device, mask, level, __FILE__, __LINE__, __VA_ARGS__)
    #else
        #define AG_ND_TRACE(device, mask, level, ...) 0
    #endif

#endif /* WIN32 */


/* */
/* AG_ND_ERR */
/*  */
/*  */
#ifndef AG_RVDS
    #define AG_ND_ERR(device, code) ( AG_ND_TRACE(device, AG_ND_TRACE_API, AG_ND_TRACE_ERROR, __func__ " failed: " #code "\n"), code )
#else
    #define AG_ND_ERR(device, code) ( AG_ND_TRACE(device, AG_ND_TRACE_API, AG_ND_TRACE_ERROR, " failed: " #code "\n"), code )
#endif


/* */
/* assert */
/*  */
/*  */
#ifndef WIN32

    #ifdef AG_ND_ENABLE_ASSERT

        #ifndef AG_RVDS
            #define assert(exp) (void)((exp) || (AG_ND_TRACE(0, AG_ND_TRACE_ALL, AG_ND_TRACE_ERROR, __func__ " assertion: " #exp "\n"), 0))
        #else
            #define assert(exp) (void)((exp) || (AG_ND_TRACE(0, AG_ND_TRACE_ALL, AG_ND_TRACE_ERROR, " assertion: " #exp "\n"), 0))
        #endif

    #else /* AG_ND_ENABLE_ASSERT */

        #define assert(exp) 0

    #endif /* AG_ND_ENABLE_ASSERT */

#else /* WIN32 */

    #include <assert.h>

#endif /* WIN32 */
#endif /*BCM_CES_SDK*/


typedef enum
{
    AG_ND_BUS_MONITOR_ADDR_SPECIFIC = 0x1,
    AG_ND_BUS_MONITOR_ADDR_ALL      = 0x2

} AgNdBusMonitor;

AgResult ag_nd_debug_regs_print(
            AgNdDevice *p_device, 
            AG_BOOL b_writable, 
            AG_BOOL b_nondefault,
            AG_U32 n_max_entity,
            AG_U32 n_specific_entity,
            AgNdRegAddressFormat e_format);

AgResult ag_nd_opcode_read_debug(AgNdDevice *p_device, AgNdMsgDebug *p_msg);
AgResult ag_nd_opcode_write_debug(AgNdDevice *p_device, AgNdMsgDebug *p_msg);
AgResult ag_nd_debug_mm_print(AgNdDevice *device);
AgResult ag_nd_debug_device_print(AgNdDevice*);
AgResult ag_nd_debug_profile_data_print(void);
AgResult ag_nd_debug_ucode_print(AgNdDevice *p_device);
AgResult ag_nd_debug_policy_print(AgNdDevice *p_device);
AgResult ag_nd_debug_ucode_flags_find(AgNdDevice *p_device, AG_U32 n_delay);


AgResult ag_nd_all_regs_print(
    AgNdDevice *p_device, 
    AG_BOOL b_writableRegistersOnly,
    AG_BOOL b_nonDefaultValuesOnly,
    AG_U32 n_max_entity,
    AG_U32 n_specific_entity,
    AgNdRegAddressFormat e_format);


#ifdef __cplusplus
}
#endif


#endif /* __NEMO_DEBUG_H__ */

