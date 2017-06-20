/*
 * ! \file diag_sand_mem.h Purpose: shell memory commands for Dune Devices 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef DIAG_SAND_MEM_H_INCLUDED
#define DIAG_SAND_MEM_H_INCLUDED

#include <appl/diag/parse.h>

/*
 * List the tables, or fields of a table entry
 */
extern char cmd_sand_mem_list_usage[];
cmd_result_t cmd_sand_mem_list(
    int unit,
    args_t * a);

extern char cmd_sand_mem_get_usage[];
cmd_result_t cmd_sand_mem_get(
    int unit,
    args_t * a);

extern char cmd_sand_mem_write_usage[];
cmd_result_t cmd_sand_mem_write(
    int unit,
    args_t * a);

extern char cmd_sand_mem_modify_usage[];
cmd_result_t cmd_sand_mem_modify(
    int unit,
    args_t * a);

#endif /* DIAG_SAND_MEM_H_INCLUDED */
