/*
 * HND SiliconBackplane MIPS core software interface.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: hndmips.h,v 1.1 Broadcom SDK $
 */

#ifndef _hndmips_h_
#define _hndmips_h_

extern void si_mips_init(si_t *sih, uint shirq_map_base);
extern bool si_mips_setclock(si_t *sih, uint32 mipsclock, uint32 sbclock, uint32 pciclock);
extern void enable_pfc(uint32 mode);
extern uint32 si_memc_get_ncdl(si_t *sih);

#if defined(BCMINTERNAL) || defined (BCMPERFSTATS)
/* enable counting - exclusive version. Only one set of counters allowed at a time */
extern void hndmips_perf_cyclecount_enable(void);
extern void hndmips_perf_instrcount_enable(void);
extern void hndmips_perf_icachecount_enable(void);
extern void hndmips_perf_dcachecount_enable(void);
/* start and stop counting */
#define hndmips_perf_start01() \
	MTC0(C0_PERFORMANCE, 4, MFC0(C0_PERFORMANCE, 4) | 0x80008000)
#define hndmips_perf_stop01() \
	MTC0(C0_PERFORMANCE, 4, MFC0(C0_PERFORMANCE, 4) & ~0x80008000)
/* retrieve coutners - counters *decrement* */
#define hndmips_perf_read0() -(long)(MFC0(C0_PERFORMANCE, 0))
#define hndmips_perf_read1() -(long)(MFC0(C0_PERFORMANCE, 1))
#define hndmips_perf_read2() -(long)(MFC0(C0_PERFORMANCE, 2))
/* enable counting - modular version. Each counters can be enabled separately. */
extern void hndmips_perf_icache_hit_enable(void);
extern void hndmips_perf_icache_miss_enable(void);
extern uint32 hndmips_perf_read_instrcount(void);
extern uint32 hndmips_perf_read_cache_miss(void);
extern uint32 hndmips_perf_read_cache_hit(void);
#endif /*  defined(BCMINTERNAL) || defined (BCMPERFSTATS) */

#endif /* _hndmips_h_ */
