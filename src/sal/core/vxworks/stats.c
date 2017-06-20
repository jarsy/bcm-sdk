/*
 * $Id: stats.c, Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    stats.c
 * Purpose: cpu occupation stats.
 */

#include <sys/types.h>
#include <sal/core/stats.h>

int 
sal_cpu_stats_get(sal_cpu_stats_t* cpu_stats)
{
    COMPILER_64_ZERO(cpu_stats->user);
    COMPILER_64_ZERO(cpu_stats->kernel);
    COMPILER_64_ZERO(cpu_stats->idle);
    COMPILER_64_ZERO(cpu_stats->total);
   
   return FALSE;
}

