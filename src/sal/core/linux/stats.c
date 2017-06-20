/*
 * $Id: stats.c, Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    stats.c
 * Purpose: cpu occupation stats.
 */

#include <sal/core/stats.h>
#include "lkm.h"
#include <linux/fs.h>
#include <linux/string.h>

#define MAX_BUF_LEN 128


int 
sal_cpu_stats_get(sal_cpu_stats_t* cpu_stats)
{
    struct file * f;
    mm_segment_t fs;
    char buf[MAX_BUF_LEN];
    uint64 nice = COMPILER_64_INIT(0,0);
    uint64 iowait = COMPILER_64_INIT(0,0);
       
    COMPILER_64_ZERO(cpu_stats->user);
    COMPILER_64_ZERO(cpu_stats->kernel);
    COMPILER_64_ZERO(cpu_stats->idle);
    COMPILER_64_ZERO(cpu_stats->total);
    
    if ((f = filp_open("/proc/stat", O_RDONLY, 0)) == NULL) {
        return FALSE;
    }
    
    fs = get_fs(); 
    set_fs(KERNEL_DS);  
    f->f_op->read(f, buf, MAX_BUF_LEN, &f->f_pos);
    filp_close(f, NULL);   
    set_fs(fs);
    
    
    /* The very first line should be cpu */
#if defined(LONGS_ARE_64BITS)
    (void)sscanf(buf, "cpu %lu %lu %lu %lu %lu", \
        &cpu_stats->user, \
        &nice, \
        &cpu_stats->kernel, \
        &cpu_stats->idle, \
        &iowait);  
#else 
#if defined(COMPILER_HAS_LONGLONG)
    (void)sscanf(buf, "cpu %llu %llu %llu %llu %llu", \
        &cpu_stats->user, \
        &nice, \
        &cpu_stats->kernel, \
        &cpu_stats->idle, \
        &iowait);
#else 
    (void)sscanf(buf, "cpu %u %u %u %u %u", \
        &u64_L(cpu_stats->user), \
        &u64_L(nice), \
        &u64_L(cpu_stats->kernel), \
        &u64_L(cpu_stats->idle), \
        &u64_L(iowait)); 
#endif
#endif

    COMPILER_64_ADD_64(cpu_stats->kernel, nice);
    COMPILER_64_ADD_64(cpu_stats->kernel, iowait);

    COMPILER_64_ADD_64(cpu_stats->total, cpu_stats->user);
    COMPILER_64_ADD_64(cpu_stats->total, cpu_stats->idle);
    COMPILER_64_ADD_64(cpu_stats->total, cpu_stats->kernel);
    
    return TRUE;
}

