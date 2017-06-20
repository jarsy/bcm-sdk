/*
 * $Id: techsupport_basic.c  Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    techsupport_basic.c
 * Purpose: This file contains the implementation to collect
 *          basic config/setup information from switch.
*/

#include <appl/diag/parse.h>
#include <appl/diag/techsupport.h>

/* The list commands exucuted as part of "techsupport basic" */
char * techsupport_basic_diag_cmdlist [] = {
    "a" ,
    "version",
    "config show",
    "show counters",
    "linkscan",
    "ps",
    "lls",
    "hsp",
    "soc",
    "fp show",
    "show pmap",
    "phy info",
    "show params",
    "show features",
    "dump soc diff",
    "dump pcic",
    NULL /* Must be the last element in this structure */
};
/* Function:    techsupport_basic
 * Purpose :    Executes the basic commands defined in cmdlist irrespect of chipset.
 * Parameters:  u - Unit number
 *              a - pointer to argument.
 * Returns    : CMD_OK :done
 *              CMD_FAIL : INVALID INPUT
 */

int 
techsupport_basic(int unit,args_t *a, techsupport_data_t *techsupport_feature_data)
{
    int i;
    for(i = 0; techsupport_basic_diag_cmdlist[i] != NULL; i++) {
        techsupport_command_execute(unit, techsupport_basic_diag_cmdlist[i]);
    }
    return 0;
}

