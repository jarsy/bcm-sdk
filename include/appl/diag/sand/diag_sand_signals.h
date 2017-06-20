/*
 * ! \file diag_sand_access.h Purpose: shell registers commands for Dune Devices 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef DIAG_DNX_DSIG_H_INCLUDED
#define DIAG_DNX_DSIG_H_INCLUDED

extern sh_sand_cmd_t sh_sand_signal_cmds[];
extern sh_sand_man_t sh_sand_signal_man;

extern const char cmd_sand_signal_usage[];
extern const char cmd_sand_signal_desc[];

/* Serves Legacy invocation */
cmd_result_t cmd_sand_signal(int unit, args_t * args);

#endif /* DIAG_SAND_ACCESS_H_INCLUDED */
