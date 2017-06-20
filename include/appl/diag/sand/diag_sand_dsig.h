/*
 * ! \file diag_sand_dsig.h
 *
 * Purpose: Interface to debug_signal database
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef __DIAG_SAND_DSIG_H
#define __DIAG_SAND_DSIG_H

#include <appl/diag/shell.h>

#define SIGNALS_PRINT_DETAIL    0x01
#define SIGNALS_PRINT_VALUE     0x02
#define SIGNALS_PRINT_DEVICE    0x08
#define SIGNALS_PRINT_CORE      0x10
#define SIGNALS_PRINT_HW        0x20

typedef enum
{
    SIG_COMMAND_DEBUG,
    SIG_COMMAND_INTERNAL,
    SIG_COMMAND_STAGE,
    SIG_COMMAND_PARAM,
    SIG_COMMAND_STRUCT,
    SIG_COMMAND_DEBUG_SINGLE
} SIG_COMMAND;

cmd_result_t cmd_sand_dsig_show(
    int unit,
    args_t * a);

#endif /* __DIAG_SAND_DSIG_H */
