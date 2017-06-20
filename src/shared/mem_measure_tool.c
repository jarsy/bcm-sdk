/*
 * $Id: mem_measure_tool.c,v 1.0 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains the memory measurement tool implementation.
 */

#include <shared/mem_measure_tool.h>

#include <shared/error.h>

int memory_measurement_tool_initialized = 0;
memory_measurement_tool_t memory_measurement_tool;

int memory_consumption_start_measurement(char *str)
{
    int fnd = 0;
    int idx;
    for(idx = 0;idx < memory_measurement_tool.count;idx++) { 
        if(sal_strcmp(memory_measurement_tool.elements[idx].id, str) == 0) {
            if(memory_measurement_tool.elements[idx].is_active) {
                return _SHR_E_EXISTS;
            }
            memory_measurement_tool.elements[idx].is_active = 1;
            fnd = 1; 
            break; 
        } 
    } 
    if(!fnd) {
        if(memory_measurement_tool.count + 1 > MEMORY_MEASUREMENT_ELEMENTS_MAX_NUM) {
            return _SHR_E_MEMORY;
        }
        memory_measurement_tool.elements[memory_measurement_tool.count].is_active = 1; 
        memory_measurement_tool.elements[memory_measurement_tool.count].sal_size = 0; 
        memory_measurement_tool.elements[memory_measurement_tool.count].sw_state_size = 0;
        sal_strncpy(memory_measurement_tool.elements[memory_measurement_tool.count].id, str, MEMORY_MEASUREMENT_ID_MAX_LENGTH - 1); 
        memory_measurement_tool.elements[memory_measurement_tool.count].thread_id = sal_thread_self(); 
        memory_measurement_tool.count++; 
    }
    return _SHR_E_NONE;
}

int memory_consumption_end_measurement(char *str)
{
    int idx;
    int fnd = 0;
    for(idx = 0;idx < memory_measurement_tool.count;idx++) {
        if((sal_strcmp(memory_measurement_tool.elements[idx].id, str) == 0) && (memory_measurement_tool.elements[idx].thread_id == sal_thread_self())) {
            fnd = 1;
            memory_measurement_tool.elements[idx].is_active = 0;
            if(idx != (memory_measurement_tool.count - 1)) {
                memory_measurement_tool.elements[idx] = memory_measurement_tool.elements[memory_measurement_tool.count - 1];
                memory_measurement_tool.count--;
            }
            break; 
        } 
    } 
    if (fnd != 1) {
        return _SHR_E_NOT_FOUND;
    }
    return _SHR_E_NONE;
}

void memory_measurement_tool_init(void)
{
    memory_measurement_tool.count = 0;
}

int memory_measurement_print(char *str)
{
#ifdef MEMORY_MEASUREMENT_DIAGNOSTICS_PRINT
    int fnd = 0;
    int idx;
    for(idx = 0;idx < memory_measurement_tool.count;idx++) {
        if(sal_strcmp(memory_measurement_tool.elements[idx].id, str) == 0) {
            fnd = 1;
            cli_out(" Str = %s\n is_active = %d\n sal_size = %u sw_state_size = %u\n\n", 
                    memory_measurement_tool.elements[idx].id, memory_measurement_tool.elements[idx].is_active, 
                    memory_measurement_tool.elements[idx].sal_size, memory_measurement_tool.elements[idx].sw_state_size);           
        }
    }
    if (fnd != 1) {
        return _SHR_E_NOT_FOUND;
    }
    return _SHR_E_NONE;
#else
    return _SHR_E_DISABLED;
#endif
}
