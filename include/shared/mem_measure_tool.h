/*
 * $Id: mem_measure_tool.h,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file defines memory measurement tool constants and functions.
 */

#ifndef _MEMORY_MEASUREMENT_TOOL
#define _MEMORY_MEASUREMENT_TOOL

#include <sal/core/thread.h>
#include <sal/types.h>
#include <sal/core/libc.h>
#include <shared/bsl.h>

#define MEMORY_MEASUREMENT_DIAGNOSTICS_PRINT 1

#define MEMORY_MEASUREMENT_ELEMENTS_MAX_NUM 100
#define MEMORY_MEASUREMENT_ID_MAX_LENGTH 256

#define MEMORY_MEASUREMENT_INITIALIZE do { \
    if(!memory_measurement_tool_initialized) { \
        memory_measurement_tool_init(); \
        memory_measurement_tool_initialized = 1; \
    } \
} while(0)

typedef struct memory_measurement_element_s {
    char            id[MEMORY_MEASUREMENT_ID_MAX_LENGTH];
    uint8           is_active;
    uint32          sal_size;
    uint32          sw_state_size;
    sal_thread_t    thread_id;
} memory_measurement_element_t;

typedef struct memory_measurement_tool_s {
    int                             count;
    memory_measurement_element_t    elements[MEMORY_MEASUREMENT_ELEMENTS_MAX_NUM];
} memory_measurement_tool_t;

extern int memory_measurement_tool_initialized;
extern memory_measurement_tool_t memory_measurement_tool;

extern int memory_consumption_start_measurement(char *str);
extern int memory_consumption_end_measurement(char *str);
extern int memory_measurement_print(char *str);
extern void memory_measurement_tool_init(void);

#endif	/* _MEMORY_MEASUREMENT_TOOL */
