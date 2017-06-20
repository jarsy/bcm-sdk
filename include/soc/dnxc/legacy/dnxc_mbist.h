/* $Id: dnxc_mbist.h,v  $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef _SOC_DNXC_MBIST_H
#define _SOC_DNXC_MBIST_H

#include <sal/types.h>
#include <soc/shared/sat.h>

#define DNXC_MBIST_COMMAND_WAIT    0
#define DNXC_MBIST_COMMAND_WRITE   0x40
#define DNXC_MBIST_COMMAND_READ    0x80
#define DNXC_MBIST_COMMAND_COMMENT 0xc0

#define DNXC_MBIST_COMMAND_MASK    0xc0
#define DNXC_MBIST_COMMAND_INVMASK 0x3f

#define DNXC_MBIST_TEST_LONG_WAIT_VALUE 0x3fffffff /* wait value representing to perform a long wait for testing purp_ses */
#define DNXC_MBIST_TEST_LONG_WAIT_DELAY_IS_SEC 0x80000000 /* If this bit is 1, then the delay is specified in seconds, otherwise in us */

#define DNXC_MBIST_UINT32(value) (value)>>24, ((value)>>16) & 0xff, ((value)>>8) & 0xff, (value) & 0xff

#define DNXC_MBIST_READ(mask, expected_value, error_line_offset) DNXC_MBIST_COMMAND_READ | (((error_line_offset)>>8) & \
  DNXC_MBIST_COMMAND_INVMASK), (error_line_offset) & 0xff, DNXC_MBIST_UINT32(mask), DNXC_MBIST_UINT32(expected_value)
#define DNXC_MBIST_WRITE(value) DNXC_MBIST_COMMAND_WRITE, DNXC_MBIST_UINT32(value)
#define DNXC_MBIST_WAIT(time) DNXC_MBIST_COMMAND_WAIT | (((time)>>24) & DNXC_MBIST_COMMAND_INVMASK), ((time)>>16) & 0xff, ((time)>>8) & 0xff, (time) & 0xff
#define DNXC_MBIST_COMMENT DNXC_MBIST_COMMAND_COMMENT
#ifdef COMPILER_STRING_CONST_LIMIT
#define DNXC_MBIST_COMMENT_TEXT(comment) ""
#else
#define DNXC_MBIST_COMMENT_TEXT(comment) comment
#endif


typedef struct dnxc_mbist_script_s {
    const uint8       *commands;
    const char        **comments;
    uint32      nof_commands;
    uint32      nof_comments;
    char        *script_name;
    uint32      sleep_after_write;
} dnxc_mbist_script_t;


typedef struct dnxc_mbist_dynamic_s {
    uint32      nof_errors;
    uint8       skip_errors;
    sal_usecs_t measured_time;
    uint32      ser_test_delay; /* The length of a delay in sec/us */
} dnxc_mbist_dynamic_t;


typedef struct dnxc_mbist_device_s {
    uint32      sleep_divisor;
    soc_reg_t   reg_tap_command;
    soc_reg_t   reg_tap_data_in;
    soc_reg_t   reg_tap_data_out;
} dnxc_mbist_device_t;



uint32 soc_dnxc_mbist_init(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN dnxc_mbist_device_t *mbist_device, 
    dnxc_mbist_dynamic_t *dynamic);
/* function to de-initialize the MBIST mechanism */
uint32 soc_dnxc_mbist_deinit(
                             SOC_SAND_IN int unit, 
                             SOC_SAND_IN dnxc_mbist_device_t *mbist_device, 
                             SOC_SAND_IN dnxc_mbist_dynamic_t *dynamic
                             );

uint32 soc_dnxc_run_mbist_script(
    SOC_SAND_IN int unit, 
    SOC_SAND_IN dnxc_mbist_script_t *script,  
    SOC_SAND_IN dnxc_mbist_device_t *mbist_device, 
    dnxc_mbist_dynamic_t *dynamic);


typedef uint16 dnxc_mbist_line_offset; /* 14 bit offset of source script line from previous read */



/* 
 * test using a given MBIST script
 */
#define DNXC_MBIST_TEST_SCRIPT(func, mbist_device) \
    DNXC_IF_ERR_EXIT(soc_dnxc_run_mbist_script(unit, & func ## _script, &mbist_device, &dynamic));


#endif /*_SOC_DNXC_MBIST_H*/

