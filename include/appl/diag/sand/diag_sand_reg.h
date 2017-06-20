/*
 * ! \file diag_sand_reg.h Purpose: shell registers commands for Dune Devices 
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef DIAG_SAND_REG_H_INCLUDED
#define DIAG_SAND_REG_H_INCLUDED

#include <appl/diag/parse.h>

extern char cmd_sand_reg_list_usage[];
cmd_result_t cmd_sand_reg_list(
    int unit,
    args_t * a);

extern char cmd_sand_reg_get_usage[];
cmd_result_t cmd_sand_reg_get(
    int unit,
    args_t * a);

extern char cmd_sand_reg_set_usage[];
cmd_result_t cmd_sand_reg_set(
    int unit,
    args_t * a);

extern char cmd_sand_reg_modify_usage[];
cmd_result_t cmd_sand_reg_modify(
    int unit,
    args_t * a);

int diag_sand_reg_get_all(
    int unit,
    int is_debug);

#endif /* DIAG_SAND_REG_H_INCLUDED */
