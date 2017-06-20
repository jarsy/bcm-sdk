/*
 * $Id: salIntf.h,v 1.3 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include <vxWorks.h>
#include <cacheLib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sysLib.h>
#include <string.h>


#ifndef SALINTF_H
#define SALINTF_H

#define	SAL_THREAD_STKSZ	16384	/* Default Stack Size */

#define sal_dma_alloc(s,c)  (cacheDmaMalloc((( (s) + 3) >> 2) << 2) )
#define sal_dma_free(s)     (cacheDmaFree(s))
#define sal_alloc(s,c)    (malloc((s)))
#define sal_free(s)       (free((s)))
#define sal_memset(d,v,l) (memset((d),(v),(l)))
#define sal_memcpy(d,s,l) (memcpy((d),(s),(l)))
#define soc_cm_salloc(u,s,c)  sal_dma_alloc(s,c)
#define soc_cm_sfree(u,s)   sal_dma_free(s)
#define soc_cm_sflush(d,a,l)  sal_dma_flush((a),(l))
#define soc_cm_sinval(d,a,l)  sal_dma_inval((a),(l))

typedef struct sal_thread_s{
    char thread_opaque_type;
} *sal_thread_t;

sal_thread_t sal_thread_create(char *name, int ss, int prio, void (f)(void *), void *arg);
int sal_thread_destroy(sal_thread_t thread);

typedef struct sal_mutex_s{
    char mutex_opaque_type;
} *sal_mutex_t;

#define sal_mutex_FOREVER	(-1)
#define sal_mutex_NOWAIT	0

sal_mutex_t 	sal_mutex_create(char *desc);
void		    sal_mutex_destroy(sal_mutex_t m);
int		        sal_mutex_take(sal_mutex_t m, int usec);
int		        sal_mutex_give(sal_mutex_t m);



int sal_spl(int level);
int sal_splhi(void);

typedef struct sal_sem_s{
    char sal_opaque_type;
} *sal_sem_t;

#define sal_sem_FOREVER		(-1)
#define sal_sem_BINARY		1
#define sal_sem_COUNTING	0

sal_sem_t	sal_sem_create(char *desc, int binary, int initial_count);
void		sal_sem_destroy(sal_sem_t b);
int		sal_sem_take(sal_sem_t b, int usec);
int		sal_sem_give(sal_sem_t b);

unsigned int sal_time_usecs(void);

void sal_thread_exit(int rc);


void sal_dma_flush(void *addr, int len);
void sal_dma_inval(void *addr, int len);

#endif
