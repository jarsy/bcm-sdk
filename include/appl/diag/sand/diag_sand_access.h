/*
 * ! \file diag_sand_access.h Purpose: shell registers commands for Dune Devices 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef DIAG_SAND_ACCESS_H_INCLUDED
#define DIAG_SAND_ACCESS_H_INCLUDED

#include <appl/diag/shell.h>
#include <appl/diag/sand/diag_sand_utils.h>
#include <appl/diag/sand/diag_sand_framework.h>

#define MAX_FIELDS_NUM  256

extern sh_sand_man_t sh_sand_access_man;
extern sh_sand_cmd_t sh_sand_access_cmds[];

extern const char cmd_sand_access_usage[];
extern const char cmd_sand_access_desc[];

cmd_result_t cmd_sand_access(
    int unit,
    args_t * a);

#endif /* DIAG_SAND_ACCESS_H_INCLUDED */
