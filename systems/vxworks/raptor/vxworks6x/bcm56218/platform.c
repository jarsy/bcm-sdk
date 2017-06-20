/* $Id: platform.c,v 1.7 2011/07/21 16:14:58 yshtil Exp $
    Copyright 2001, Broadcom Corporation
    All Rights Reserved.
    
    This is UNPUBLISHED PROPRIETARY SOURCE CODE of Broadcom Corporation;
    the contents of this file may not be disclosed to third parties, copied or
    duplicated in any form, in whole or in part, without the prior written
    permission of Broadcom Corporation.
*/

/*
modification history
--------------------
*/


#include "vxWorks.h"
#include "config.h"
#include "vxbsp.h"
#include "typedefs.h"
#include "vx_osl.h"

static uint32 sbclk=0;

void platform_init(void);

#define MIPS_CORE_CAPABILITY  0x18000004

/* routine to return the sb clock speed (discovered during init ) */
uint32 get_sb_clock(void) {
#ifdef QUICK_TURN
    sbclk = UART_REF_CLK_FREQ;
#else    
    uint32   val;
    if (!sbclk) {
        if (((sysGetSocDevId() & 0x0ff0) == 0x0310) ||
            ((sysGetSocDevId() & 0x0ff0) == 0x0320)) {
            val = *(volatile uint32*)KSEG1ADDR(MIPS_CORE_CAPABILITY);
            if (val & (1 << 22)) {
                sbclk = 133000000;
            } else {
                sbclk = 100000000;
            }
        } else {
            sbclk = 133000000;
        }
    }
#endif
    ASSERT(sbclk);
    return(sbclk);
}

uint32 get_sb_mips_clock(void) {
    uint32   core;
    volatile uint32  reg;

    core = get_sb_clock();

    reg = *(volatile uint32*)KSEG1ADDR(MIPS_CORE_CAPABILITY);
    if (reg & 0x00000008) {
        if ((sysGetSocDevId() & 0x0ff0) == 0x0310) {
            switch((reg >> 4) & 3) {
                case 0: core = core*2; break;
                case 1: core = core*3; break;
                case 2: core = (core*10)/4 ; break;
                case 3: break;
            }
        } else if ((sysGetSocDevId() & 0x00f0) == 0x0010) {
            switch ((reg >> 4) & 3) {
                case 0: core = core * 2; break;
                case 1: core = (core * 9)/4; break;
                case 2: core = (core * 6)/4; break;
                case 3: core = core; break;
            }
        } else {
            switch ((reg >> 4) & 3) {
                case 0: core = core * 2; break;
                case 1: core = (core * 9)/4; break;
                case 2: core = (core * 10)/4; break;
                case 3: core = core; break;
            }
        } 
    } else {
        core = core * 2;
    }
    
    return core;
}


#ifdef CACHE_MEM_POOL_RESERVED
LOCAL void
debug(char *fmt, ...)
{
}

#define PGSZ (16<<20)		/* bcm5836 implements 16MB page size */
#define TLBMASK 0x01ffe000	/* 16MB mask for CP0 register 5 */
#define TLB_FLAGS 0x1f		/* cachable-writeback, dirty, valid, global */
#define TLBWIRED_MAX 31		/* seems to be the max value for bcm5836 */
#define TLB_ENTRY_SHIFT (31-25)

/* defined in sysALib.s */
void setTlbEntry(uint32 EntryIdx, uint32 EntryHi,
	uint32 EntryLo0, uint32 EntryLo1);
void setPageSize(uint32);
int getTlbWired(void);
void setTlbWired(int);

LOCAL void
init_tlb(void)
{
    /* we're going to overwrite any previous mappings that others may have
     * programmed into the TLB.
     */
    setTlbWired(0);
}

LOCAL int
one_tlb(uint32 va, uint32 pa0, uint32 pa1)
{
    int idx;

    idx = getTlbWired() + 1;
    if(idx > TLBWIRED_MAX) return 0;
    debug("v=%#08x even=%#08x odd=%#08x idx=%d\n", va, pa0, pa1, idx);
    setTlbWired(idx);
    setTlbEntry(idx, va,
	    (pa0 >> TLB_ENTRY_SHIFT) | TLB_FLAGS, 
	    (pa1 >> TLB_ENTRY_SHIFT) | TLB_FLAGS);
    return 1;
}

LOCAL void *
tlb_map(unsigned long len, unsigned long pa, unsigned long va)
{
    int i, n;

    /* We require that va and len be two-page aligned to simplify the code
     * dealing with the (even, odd) TLB layout
     */
    if((len & ((PGSZ*2)-1)) || (pa & (PGSZ-1)) || (va & ((PGSZ*2)-1))) {
	debug("tlb_map(%#lx, %#lx, %#lx): ignoring non-rounded request\n",
		len, pa, va);
	return 0;
    }
    setPageSize(TLBMASK);
    n = len / (PGSZ*2);
    for(i=0; i<n; i++) {
	uint32 even, odd;
	int off = i * PGSZ*2;

	even = off;
	odd = even + PGSZ;
	if(one_tlb(va+off, pa+even, pa+odd) == 0) break;
    }
    if(i < n) return 0;
    return (void *)va;
}

/* size_mem looks at 1M increments for aliases of memory page 0, which is,
 * empirically, a successful method of determining memory size.  This
 * method is sub-optimal (as is, apparently, every single DRAM sizing
 * function in the world).  If page 0 changes (for example, due to a DMA or
 * interrupt handler or whatever) then the memcmp is likely to fail.  To
 * minimize the likelihood, it is mapped thorough uncached kseg1, which
 * impacts performance, but 128*512 uncached word reads is fast enough.
 *
 * There are a variety of pathological cases (for example, page 0 being
 * all-bytes-zero, or odd copies of page 0 hanging around) but they're
 * unlikely enough to be of no concern.
 */
LOCAL unsigned long
size_mem(void *p, unsigned long len)
{
    char *a = p, *membase = (char *)KSEG1ADDR(0);
    /* int j;*/
    int i, step = 1<<20;
    char buf[5];
    unsigned char pattern[256];

    for(i=0; i<sizeof(pattern); i++)
	pattern[i] = i;

    for(i=0; i<len; i += step) {
	sprintf(buf, " %3d", i>>20); sysLedDsply(buf);
	/* debug("size_mem i=%#08x a=%p membase=%p\n", i, a, membase); */
	if(!memcmp(a+i, membase, 128))
	    break;
	sprintf(buf, "*%3d", i>>20); sysLedDsply(buf);
    }
    sprintf(buf, "m%3d", i>>20); sysLedDsply(buf);
    return i;
}

/* The 4704/5836 has a 512MB maximum physical memory address space on the
 * OCP bus.  128MB is "special", because it can be directly addressed at
 * OCP address 0 (and thus from MIPS in kseg0/kseg1), and also has a
 * "byteswapped window" at OCP address 0x1000_0000.  Any memory above 128M
 * in the system lacks byteswapped addressibility, and furthermore can only
 * be accessed from MIPS via TLB entries at OCP address 0x8000_0000.
 *
 * This routine maps the "highmem" address region (the part above 128M)
 * into locked TLB entries in kseg2, then calls size_mem to figure out how
 * much physical memory is actually present.
 */

void *Bcm47xxHiMemAddr;
int Bcm47xxHiMemSize;

LOCAL void
map_56218_highmem(void)
{
    unsigned long pa = BCM56218_SDRAM_HIGH + BCM56218_SDRAM_SZ;
    unsigned long va = KSEG2BASE + BCM56218_SDRAM_SZ;
    unsigned long len = BCM56218_SDRAM_HIGH_SZ - BCM56218_SDRAM_SZ;
    unsigned long sz;
    void *p;

    sysLedDsply("HIGH");
    debug("Mapping %dMB, PA=%#08x, VA=%#08x\n", len>>20, pa, va);
    init_tlb();
    p = tlb_map(len, pa, va);
    if (!p) {
	Bcm47xxHiMemSize = 0;
	return;
    }
    sysLedDsply("SIZE");
    debug("Sizing %dMB...", len>>20);
    sz = size_mem(p, len);

    debug(" Found %dMB\n", sz>>20);
    Bcm47xxHiMemAddr = p;
    Bcm47xxHiMemSize = sz;
}
#endif /* CACHE_MEM_POOL_RESERVED */

IMPORT void sysExtInt (void);

void
platform_init()
{
#ifdef CACHE_MEM_POOL_RESERVED
    map_56218_highmem();
#endif

#define SBIPSFLAG_ADDR   0xb8000100
#define SBINTVEC_ADDR    0xb8000104
    *((volatile unsigned long*)(SBIPSFLAG_ADDR)) = DEF_SBIPSFLAG;
    *((volatile unsigned long*)(SBINTVEC_ADDR)) = DEF_SBINTVEC;
}



