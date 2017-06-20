/* 
 * $Id: srcell.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        srcell.c
 * Purpose:     Source Route Cell command
 *
 */

#include <appl/diag/dcmn/cmdlist.h>

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/drv.h>
#endif

char cmd_dpp_source_routed_cell_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "source_routed_cell [options]\n"
#else
    "source_routed_cell [Func name] "
    "[in parameters ordered as in the dpp/fabric.h file]\n\t"
#endif
;


