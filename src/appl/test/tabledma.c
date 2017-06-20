/*
 * $Id: tabledma.c,v 1.101 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Table DMA Test
 */
#include <shared/bsl.h>
#include <sal/appl/sal.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/dma.h>
#include <soc/l2x.h>
#include "testlist.h"

#if defined (BCM_TOMAHAWK_SUPPORT)
#include <soc/tomahawk.h>
#endif

#if defined (BCM_TOMAHAWK2_SUPPORT)
#include <soc/tomahawk2.h>
#include <soc/pstats.h>
#endif

#ifdef BCM_PETRA_SUPPORT
#include <soc/dpp/drv.h>
#endif /* BCM_PETRA_SUPPORT */
#ifdef BCM_DFE_SUPPORT
#include <soc/dfe/cmn/dfe_drv.h>
#endif /* BCM_DFE_SUPPORT */

#if defined(BCM_DFE_SUPPORT)  || defined(BCM_PETRA_SUPPORT)
extern int bcm_common_linkscan_enable_set(int,int);
#endif

int compare_with_mask(const void *m1_void, const void *m2_void, size_t len);

#define TD_DEFAULT_ROM_VER_COUNT     10
#define TD_DEFAULT_DUMP_ENTRY_COUNT  10

#if defined(BCM_XGS_SWITCH_SUPPORT) || defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) || defined(BCM_DFE_SUPPORT) || defined (BCM_PETRA_SUPPORT)

typedef struct tdt_data_s {
    int             init_done;
    int             orig_enable;
    int             saved_tcam_protect_write;
    int             mem_scan;
    int             scan_rate;
    sal_usecs_t     scan_interval;
    int             sram_scan;
    int             sram_scan_rate;
    sal_usecs_t     sram_scan_interval;
    char           *mem_str;
    soc_mem_t       mem;
    soc_mem_t       mem_write;
    int             copyno;
    int             index_min;
    int             index_max;
    int             count;
    int             rom_ver_count;
    int             frag_seq_num_min[SOC_MAX_NUM_PIPES];
    int             frag_seq_num_max[SOC_MAX_NUM_PIPES];
    int             frag_index_min[SOC_MAX_NUM_PIPES];
    int             frag_count;

    uint32         *source;
    uint32          srcaddr;
    uint32         *dma_ed;
    uint32          dstaddr;

    soc_mem_cmp_t   comp_fn;
    int             bufwords;
    int             entwords;

    uint32          rnum;
    int             ecc_as_data; /* treat ecc field as regular field */

    uint32         datamask[SOC_MAX_MEM_WORDS];
    uint32         tcammask[SOC_MAX_MEM_WORDS];
    uint32         eccmask[SOC_MAX_MEM_WORDS];
    uint32         forcemask[SOC_MAX_MEM_WORDS];
    uint32         forcedata[SOC_MAX_MEM_WORDS];
    uint32         accum_tcammask;
    uint32         accum_forcemask;
    int            ccm;
    int            tslam_en;    /* Table slam Enable */
    int            tslam_dir;   /* Table slam direction */
    int            tdma_en;     /* Table dma enable */
    int            mor_en;
    int            pstats_ucq;
    int            td_check_data;
} td_test_data_t;


static td_test_data_t td_test_data;

static int td_test_mem_cmp_fn(int, void*, void*);

STATIC int
td_rand_common_clear(int unit, soc_mem_t mem, int copyno)
{
    int rv;

    SOC_MEM_TEST_SKIP_CACHE_SET(unit, 1);
    if ((rv = soc_mem_parity_control(unit, mem, copyno, FALSE)) < 0) {
        test_error(unit, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, mem));
        return -1;
    }
#ifdef INCLUDE_MEM_SCAN
    if ((td_test_data.mem_scan =
             soc_mem_scan_running(unit, &td_test_data.scan_rate,
                                  &td_test_data.scan_interval)) > 0) {
        if (soc_mem_scan_stop(unit)) {
            return -1;
        }
    }
#endif /* INCLUDE_MEM_SCAN */
#ifdef BCM_SRAM_SCAN_SUPPORT
    if ((td_test_data.sram_scan =
             soc_sram_scan_running(unit, &td_test_data.sram_scan_rate,
                                  &td_test_data.sram_scan_interval)) > 0) {
        if (soc_sram_scan_stop(unit)) {
            return -1;
        }
    }
#endif /* BCM_SRAM_SCAN_SUPPORT */
    return 0;
}

STATIC int
td_rand_common_restore(int unit, soc_mem_t mem, int copyno)
{
    SOC_MEM_TEST_SKIP_CACHE_SET(unit, 0);
    if (soc_mem_parity_restore(unit, mem, copyno) < 0) {
        test_error(unit, "Could not enable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, mem));
        return -1;
    }
#ifdef INCLUDE_MEM_SCAN
    if (td_test_data.mem_scan) {
        if (soc_mem_scan_start(unit, td_test_data.scan_rate,
            td_test_data.scan_interval)) {
            return -1;
        }
    }
#endif /* INCLUDE_MEM_SCAN */
#ifdef BCM_SRAM_SCAN_SUPPORT
    if (td_test_data.sram_scan) {
        if (soc_sram_scan_start(unit, td_test_data.sram_scan_rate,
            td_test_data.sram_scan_interval)) {
            return -1;
        }
    }
#endif /* BCM_SRAM_SCAN_SUPPORT */
    return 0;
}

static int
td_test_data_init(int unit, int alloc)
{
    int i;

    if (!td_test_data.init_done) {
#ifdef BCM_FIREBOLT_SUPPORT
         if (SOC_IS_FBX(unit)) {
#ifdef BCM_TRIUMPH3_SUPPORT
            if (SOC_IS_TRIUMPH3(unit)) {
                td_test_data.mem = L2_ENTRY_1m;
                td_test_data.mem_str = sal_strdup("l2_entry_1");
                td_test_data.count = soc_mem_index_count(unit, L2_ENTRY_1m);
            } else
#endif
            if (SOC_MEM_IS_VALID(unit, L2_ENTRY_ONLYm)) {
                td_test_data.mem = L2_ENTRY_ONLYm;
                td_test_data.mem_str = sal_strdup("l2_entry_only");
                td_test_data.count = soc_mem_index_count(unit, L2_ENTRY_ONLYm);
            }
        } else
#endif /* BCM_FIREBOLT_SUPPORT */
#if defined (BCM_DFE_SUPPORT) || defined (BCM_PETRA_SUPPORT)
         if (SOC_IS_DFE(unit) || SOC_IS_ARAD(unit)) {
       /* no default for dfe */
        } else
#endif /* BCM_DFE_SUPPORT */
#ifdef BCM_SIRIUS_SUPPORT
         if (SOC_IS_SIRIUS(unit)) {
       /* no default for sirius */
        } else
#endif /* BCM_SIRIUS_SUPPORT */
#ifdef BCM_CALADAN3_SUPPORT
         if (SOC_IS_CALADAN3(unit)) {
       /* no default for sirius */
        } else
#endif /* BCM_CALADAN3_SUPPORT */
        {
            test_error(unit, "Table DMA Error:  Invalid SOC type\n");
            return -1;
        }
        td_test_data.init_done = TRUE;
        td_test_data.tslam_en = FALSE;
        td_test_data.tslam_dir = TRUE; /* SLAM from low to high */
        td_test_data.tdma_en = TRUE;
        td_test_data.ccm = FALSE;
        td_test_data.rnum        = sal_rand();
        td_test_data.srcaddr = 0;
        td_test_data.dstaddr = 0;
        td_test_data.td_check_data = TRUE;
    }
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (!SOC_IS_XGS3_SWITCH(unit)) {
        td_test_data.tslam_en = FALSE;
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */

#ifdef BCM_SIRIUS_SUPPORT
    if (SOC_IS_SIRIUS(unit)) {
      td_test_data.tslam_en = TRUE;
    }
#endif /* BCM_SIRIUS_SUPPORT */

#ifdef BCM_CALADAN3_SUPPORT
    if (SOC_IS_CALADAN3(unit)) {
      td_test_data.tslam_en = TRUE;
    }
#endif /* BCM_CALADAN3_SUPPORT */

    /*
     *  Gets reset for every run.
     */
    td_test_data.rom_ver_count = TD_DEFAULT_ROM_VER_COUNT;

    if (!alloc) {
        return 0;
    }

    td_test_data.frag_seq_num_min[0] = 0;
    td_test_data.frag_seq_num_max[0] = td_test_data.index_max -
        td_test_data.index_min;
    td_test_data.frag_index_min[0] = td_test_data.index_min;
    td_test_data.frag_count = 1;
    td_test_data.entwords    = soc_mem_entry_words(unit, td_test_data.mem);
    td_test_data.bufwords    = td_test_data.entwords * td_test_data.count;
    if (0 == td_test_data.srcaddr) {
        td_test_data.source      = soc_cm_salloc(unit,
                                         WORDS2BYTES(td_test_data.bufwords),
                                         "td_test_data source");
    } else {
        td_test_data.source = INT_TO_PTR(td_test_data.srcaddr);
    }

    if (0 == td_test_data.dstaddr) {
        td_test_data.dma_ed      = soc_cm_salloc(unit,
                                         WORDS2BYTES(td_test_data.bufwords),
                                         "td_test_data dma_ed");
    } else {
        td_test_data.dma_ed = INT_TO_PTR(td_test_data.dstaddr);
    }
    if ((td_test_data.source == NULL) ||
        (td_test_data.dma_ed == NULL)) {
        if (td_test_data.source && (0 == td_test_data.srcaddr)) {
            soc_cm_sfree(unit, td_test_data.source);
            td_test_data.source = NULL;
        }
        if (td_test_data.dma_ed && (0 == td_test_data.dstaddr)) {
            soc_cm_sfree(unit, td_test_data.dma_ed);
            td_test_data.dma_ed = NULL;
        }
        test_error(unit, "Table DMA Error:  Failed to allocate DMA memory\n");
        return -1;
    }

    if (0 != td_test_data.rnum) {
        sal_memset(td_test_data.source, 0xff, WORDS2BYTES(td_test_data.bufwords));
        sal_memset(td_test_data.dma_ed, 0xff, WORDS2BYTES(td_test_data.bufwords));
    }
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT) ||\
    defined(BCM_GREYHOUND2_SUPPORT)
    if(SOC_IS_DPP(unit) || SOC_IS_DFE(unit) || SOC_IS_GREYHOUND2(unit)) {
        soc_mem_datamask_rw_get(unit, td_test_data.mem, td_test_data.datamask);
    } else
#endif /* BCM_PETRA_SUPPORT || BCM_DFE_SUPPORT */
    {
        soc_mem_datamask_get(unit, td_test_data.mem, td_test_data.datamask);
    }
    soc_mem_tcammask_get(unit, td_test_data.mem, td_test_data.tcammask);
    soc_mem_eccmask_get(unit, td_test_data.mem, td_test_data.eccmask);
    soc_mem_forcedata_get(unit, td_test_data.mem, td_test_data.forcemask,
                          td_test_data.forcedata);
    td_test_data.accum_tcammask = 0;
    for (i = 0; i < td_test_data.entwords; i++) {
        td_test_data.accum_tcammask |= td_test_data.tcammask[i];
    }
    td_test_data.accum_forcemask = 0;
    for (i = 0; i < td_test_data.entwords; i++) {
        td_test_data.accum_forcemask |= td_test_data.forcemask[i];
    }
    if (!td_test_data.ecc_as_data) {
        for (i = 0; i < td_test_data.entwords; i++) {
            td_test_data.datamask[i] &= ~td_test_data.eccmask[i];
        }
    }
    soc_mem_datamask_memtest(unit, td_test_data.mem, td_test_data.datamask);

    td_test_data.comp_fn = td_test_mem_cmp_fn;

    return 0;
}

static void
td_test_dump_data(int unit, int entry_cnt)
{
    int     i, base_index, idx;
    uint32 *s, *d;

    cli_out("Table DMA test for %s.%s: index_min %d index_max %d\n"
            "    bufwords %d entwords %d rnum %d count %d\n",
            SOC_MEM_UFNAME(unit, td_test_data.mem),
            SOC_BLOCK_NAME(unit, td_test_data.copyno),
            td_test_data.index_min,
            td_test_data.index_max, td_test_data.bufwords,
            td_test_data.entwords, td_test_data.rnum, td_test_data.count);

    cli_out("    Datamask: ");
    for (idx = 0; idx < td_test_data.entwords; idx++) {
        cli_out(" %08x", td_test_data.datamask[idx]);
    }
    cli_out("\n\n");

    s = td_test_data.source;
    d = td_test_data.dma_ed;

    assert(s && d);

    base_index = td_test_data.index_min;
    s += td_test_data.index_min;

    for (i = td_test_data.index_min;
         i <= td_test_data.index_max && i - base_index < entry_cnt;
         ++i) {
        cli_out("    Table index %d: "
                "source entry %d, dma_ed entry %d\n",
                i, i - base_index, i - td_test_data.index_min);
        cli_out("    Source:  ");
        for (idx = 0; idx < td_test_data.entwords; idx++) {
            cli_out(" %08x", s[idx]);
        }
        cli_out("\n");
        cli_out("    DMA_ed:  ");
        for (idx = 0; idx < td_test_data.entwords; idx++) {
            cli_out(" %08x", d[idx]);
        }
        cli_out("\n\n");
        d += td_test_data.entwords;
        s += td_test_data.entwords;
    }
}

static void
td_test_data_clear(int unit)
{
    if (td_test_data.source && (0 == td_test_data.srcaddr)) {
        soc_cm_sfree(unit, td_test_data.source);
        td_test_data.source = NULL;
    }
    if (td_test_data.dma_ed && (0 == td_test_data.dstaddr)) {
        soc_cm_sfree(unit, td_test_data.dma_ed);
        td_test_data.dma_ed = NULL;
    }
}

static uint32
td_test_get_curr_rnum(void)
{
    return td_test_data.rnum;
}

static uint32
td_test_rand32(uint32 prev)
{
    return td_test_data.rnum = 1664525L * prev + 1013904223L;
}

#define BSAND_BYTE_SWAP(x) ((((x) << 24)) | (((x) & 0x00ff00) << 8) | (((x) & 0xff0000) >> 8) | (((x) >> 24)))
#define BSAND_WORD_SWAP(x) ( (((x) & 0xFFff0000) >> 16) | (((x) & 0x0000FFff) << 16) )

static uint32 *
td_test_random_entry(int unit, soc_mem_t mem, uint32 *entbuf)
{
    int i;

    for (i = 0; i < td_test_data.entwords; ++i) {
        uint32 prev = td_test_get_curr_rnum();
#ifdef BCM_PETRA_SUPPORT
        if (SOC_IS_ARAD(unit)) {
            entbuf[i]   = td_test_rand32(prev) & BSAND_BYTE_SWAP(td_test_data.datamask[i]);
        } else
#endif /* BCM_PETRA_SUPPORT */
        {
            entbuf[i]   = td_test_rand32(prev) & td_test_data.datamask[i];
        }
    }

    return entbuf;
}

static int
td_test_write_entry(int unit, soc_mem_t mem, int copyno,
                 int index, uint32 *entbuf)
{
    int rv = 0;

    if (SOC_MEM_IS_VALID(unit, mem)) {
        rv = soc_mem_write(unit, mem, copyno, index, entbuf);
    }
    if (rv < 0) {
        test_error(unit,
                   "Write entry (copyno %d) failed: %s",
                   copyno, soc_errmsg(rv));
    }
    return rv;
}

static int
td_test_read_entry(int unit, soc_mem_t mem, int copyno,
                   int index, uint32 *entbuf)
{
    int rv = 0;

    if (SOC_MEM_IS_VALID(unit, mem)) {
        rv = soc_mem_read(unit, mem, copyno, index, entbuf);
    }
    if (rv < 0) {
        test_error(unit,
                   "Read entry (copyno %d) failed: %s",
                   copyno, soc_errmsg(rv));
    }
    return rv;
}

static int
td_test_verify(int unit)
{
    int rv         = 0;
    int rv_comp = 0;
    soc_mem_t mem  = td_test_data.mem;
    int base_index = soc_mem_index_min(unit, mem);
    int index_min  = td_test_data.index_min;
    int index_max  = td_test_data.index_max;
    int ent_words  = soc_mem_entry_words(unit, mem);
    int i, j;
    int idx;
    uint32 *d;
    uint32 *s;

    /*
     * For readonly mem, compare a few dma-ed entries with
     * entries in hardware from direct reading.
     */
    if (soc_mem_is_readonly(unit, mem)) {
        int error = 0;
        s = td_test_data.source;
        d = td_test_data.dma_ed;
        for (i = 0; i < td_test_data.rom_ver_count; ++i) {
            if (i >= td_test_data.count) {
                break;
            }
            if (soc_mem_read(unit, mem, td_test_data.copyno,
                             i+td_test_data.index_min,
                             s) < 0) {
                cli_out("Cannot read entry %d\n", i+td_test_data.index_min);
                return -1;
            }
            if (td_test_data.comp_fn(unit, d, s) != 0) {
                error = -1;
                cli_out("%s.%s diff entry index %d: \n",
                        SOC_MEM_UFNAME(unit, mem),
                        SOC_BLOCK_NAME(unit, td_test_data.copyno),
                        i+td_test_data.index_min);
                cli_out("Read directly:\n");
                soc_mem_entry_dump(unit, mem, s);
                cli_out("\n");
                cli_out("Read by table dma:\n");
                soc_mem_entry_dump(unit, mem, d);
                cli_out("\n");
            }
            d += ent_words;
            s += ent_words;
        }
        return error;
    }

    if (!td_test_data.ecc_as_data) {
        for (i = 0; i <= index_max - index_min; i++) {
            for (idx = 0; idx < td_test_data.entwords; idx++) {
                td_test_data.dma_ed[i * ent_words + idx] &=
                    ~td_test_data.eccmask[idx];
                td_test_data.source[i * ent_words + idx] &=
                    ~td_test_data.eccmask[idx];
            }
        }
    }

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
        if (SOC_IS_DPP(unit) || SOC_IS_DFE(unit) || SOC_IS_CALADAN3(unit)) {
            rv_comp = compare_with_mask(td_test_data.dma_ed, td_test_data.source, (index_max - index_min + 1) * ent_words * 4);
        } else
#endif /* BCM_PETRA_SUPPORT defined(BCM_DFE_SUPPORT */
    {
#if defined(BCM_XGS3_SWITCH_SUPPORT)
        if (soc_feature(unit, soc_feature_sbusdma)) {
            if ((td_test_data.tslam_dir == 0) && td_test_data.tslam_en &&
                soc_mem_slamable(unit, mem, td_test_data.copyno)) {
                /* swap the source buffer */
                uint32 temp[SOC_MAX_MEM_WORDS];
                s = td_test_data.source;
                d = s + ((index_max - index_min) * ent_words);
                for (i = 0; i < (index_max - index_min + 1)/2; ++i) {
                    sal_memcpy(temp, s + i * ent_words, ent_words * 4);
                    sal_memcpy(s + i * ent_words, d - i * ent_words, ent_words * 4);
                    sal_memcpy(d - i * ent_words, temp, ent_words * 4);
                }
            }
        }
#endif
        rv_comp = compare_with_mask(td_test_data.dma_ed, td_test_data.source,
                                    (index_max - index_min + 1) * ent_words * 4);
    }

    if (rv_comp != 0) { 

        cli_out("Mismatch found\n");
        s = td_test_data.source;
        d = td_test_data.dma_ed;
        for (i = index_min; i <= index_max; ++i) {
            if (td_test_data.comp_fn(unit, d, s) != 0) {
                cli_out("%s.%s diff entry index %d: "
                        "source entry %d, dma_ed entry %d\n",
                        SOC_MEM_UFNAME(unit, mem),
                        SOC_BLOCK_NAME(unit, td_test_data.copyno),
                        i, i - base_index, i - index_min);
                cli_out("Source:");
                for (j = 0; j < ent_words; j++) {
                    cli_out(" 0x%08x = 0x%08x", (PTR_TO_INT(s+j)), s[j]);
                }
                cli_out("\nDMA_ed:");
                for (j = 0; j < ent_words; j++) {
                    cli_out(" 0x%08x = 0x%08x", (PTR_TO_INT(d+j)), d[j]);
                }

                cli_out("\nMask");
                for (j = 0; j < ent_words; j++) {
                    cli_out("  = 0x%08x", (uint32)BSAND_BYTE_SWAP(td_test_data.datamask[j]));                    
                }
                cli_out("\n");

                return -1;
            }
            d += ent_words;
            s += ent_words;
        }
        /* If get here, something is wrong, dump for inspection */
        td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
    }

    return rv;
}


int
compare_with_mask(const void *m1_void, const void *m2_void, size_t len)
{
    const uint32 *m1 = m1_void, *m2 = m2_void;
    uint32  *mask_start;
    uint32  *mask;
    uint32 i;

     mask_start = (uint32 *)td_test_data.datamask;
/*    cli_out("Compare with mask  entwords= 0x%08x  len=0x%08x", td_test_data.entwords, (uint32) len);  */
    /* while (len != 0) */
	{
      /*  mask = td_test_data.datamask; */
        mask = mask_start;
        for (i = 0; i < td_test_data.entwords; i++) {
            len--;
/*            cli_out("Mask  i=%d maskvalue = 0x%08x  ", i, *mask);  */
            if ((*m1 & *mask) < (*m2 & *mask)) {
              /*  cli_out("*m1=0x%08x < *m2 = 0x%08x \n", *m1, *m2); */
                return -1;
            } else if ((*m1 & *mask) > (*m2 & *mask)) {
               /* cli_out("*m1=0x%08x > *m2 = 0x%08x \n", *m1, *m2); */
                return 1;
            }
            /* cli_out("*m1=0x%08x == *m2 = 0x%08x \n", *m1, *m2); */
            m1++;
            m2++;
            mask++;
        }
    }

    return 0;
}


static int
td_test_mem_cmp_fn(int unit, void *s, void*d)
{
    soc_mem_t mem  = td_test_data.mem;
    int ent_words  = soc_mem_entry_words(unit, mem);

    assert(s && d);

    return sal_memcmp(s, d, ent_words * 4);
}


static int
td_test_dma_verify_helper(int unit)
{
    int        i, rv = 0;
    uint32    *entbuf, *entbuf_source;
    int        frag;
    int        index_min, index_max;
    int        index_offset;

#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        if (TRUE == td_test_data.td_check_data) {
            if (sal_memcmp(td_test_data.dma_ed, td_test_data.source,
                           (td_test_data.count * 4)) != 0) {
                test_error(unit, "CCM DMA Compare failed\n");
                td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
                return -1;
            }
        }
    } else
#endif
    {
        if (0 != td_test_data.rnum) {
            sal_memset(td_test_data.dma_ed, 0xff, td_test_data.bufwords * 4);
        }

        if (!SOC_IS_XGS_SWITCH(unit) && !SOC_IS_SIRIUS(unit) && !SOC_IS_CALADAN3(unit) && !SOC_IS_SAND(unit)) {
            test_error(unit, "Chip type not supported for table DMA test\n");
            return -1;
        }
        for (frag = 0; frag < td_test_data.frag_count; frag++) {
            index_min = td_test_data.frag_index_min[frag];
            index_max = index_min + td_test_data.frag_seq_num_max[frag] -
                td_test_data.frag_seq_num_min[frag];
            entbuf = td_test_data.dma_ed +
                td_test_data.frag_seq_num_min[frag] * td_test_data.entwords;

            entbuf_source = td_test_data.source +
                td_test_data.frag_seq_num_min[frag] * td_test_data.entwords;
            if (td_test_data.tdma_en) {
                rv = soc_mem_read_range(unit, td_test_data.mem,
                                        td_test_data.copyno, index_min,
                                        index_max, entbuf);
                if (rv < 0) {
                    test_error(unit, "Table DMA Error: Memory %s.%d %s\n",
                               td_test_data.mem_str, td_test_data.copyno,
                               soc_errmsg(rv));
                    return -1;
                }
                for (i = index_min; i <= index_max; i++) {
                    if (soc_mem_test_skip(unit, td_test_data.mem, i)) {
                        index_offset = (td_test_data.tslam_dir) ? (i - index_min) : (index_max - i);
                        sal_memcpy((void *)entbuf,
                                   (void *)(entbuf_source + index_offset * td_test_data.entwords),
                                   WORDS2BYTES(td_test_data.entwords));
                    }
                    entbuf += td_test_data.entwords;
                }
            } else {
                for (i = index_min; i <= index_max; i++) {
                    if (soc_mem_test_skip(unit, td_test_data.mem, i)) {
                        index_offset = (td_test_data.tslam_dir) ? (i - index_min) : (index_max - i); 
                        sal_memcpy((void *)entbuf,
                                   (void *)(entbuf_source + (i - index_min) * td_test_data.entwords),
                                   WORDS2BYTES(td_test_data.entwords));
                        entbuf += td_test_data.entwords;
                        continue;
                    }
                    if (td_test_read_entry(unit, td_test_data.mem,
                                           td_test_data.copyno, i, entbuf) <
                        0) {
                        return -1;
                    }
                    entbuf += td_test_data.entwords;
                }
            }
        }

        if (TRUE == td_test_data.td_check_data) {
            /*
             * Verifing received results
             */
            if ((rv = td_test_verify(unit)) < 0) {
                test_error(unit,
                           "Table DMA failed: inconsistency observed\n");
                td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
                return -1;
            }
        }
    }
    return 0;
}

#ifdef BCM_TOMAHAWK2_SUPPORT
int tomahawk2_tr_init(int unit) {
    soc_mem_t   mem = INVALIDm;
    int         found = 0, pipe, xpe;

    /* Tomahawk2 MMU_THDU_UCQ_STATS_TABLE memory need special process */
    for (pipe = 0; pipe < NUM_PIPE(unit); pipe++) {
        for (xpe = 0; xpe < NUM_XPE(unit); xpe++) {
            mem = SOC_MEM_UNIQUE_ACC_XPE_PIPE(unit, MMU_THDU_UCQ_STATS_TABLEm,
                                              xpe, pipe);
            if ((mem == INVALIDm) || (mem != td_test_data.mem)) {
                continue;
            } else {
                found = 1;
                td_test_data.pstats_ucq = 1;
                /* set memory for write */
                td_test_data.mem_write =
                    SOC_MEM_UNIQUE_ACC_XPE_PIPE(unit, MMU_THDU_UCQ_STATSm,
                                                xpe, pipe);
                break;
            }
        }
        if (found) {
            break;
        }
    }

    if (soc_tomahawk2_mmu_pstats_mode_set(unit, 0) < 0) {
        return -1;
    }

    return 0;
}

int tomahawk2_mor_init(int unit) {
    uint32      flag;

    if (td_test_data.tdma_en) {
        return -1;
    }

    /* invalid memory for mor */
    if (!td_test_data.pstats_ucq) {
        return -1;
    }

    soc_pstats_deinit(unit);
    if(soc_pstats_init(unit) < 0) {
        return -1;
    }

    /* call soc function */
    flag = _TH2_MMU_PSTATS_ENABLE |
           _TH2_MMU_PSTATS_PKT_MOD |
           _TH2_MMU_PSTATS_SYNC |
           _TH2_MMU_PSTATS_HWM_MOD |
           _TH2_MMU_PSTATS_MOR_EN;

    if (soc_tomahawk2_mmu_pstats_mode_set(unit, flag) < 0) {
        return -1;
    }

    return 0;
}

int tomahawk2_pstats_ucq_test(int unit, int index, uint32 *entbuf) {
    soc_field_t     ucq_field[4] = {UCQ_COUNT0f, UCQ_COUNT1f,
                                    UCQ_COUNT2f, UCQ_COUNT3f};
    uint32          fval;
    int             field_len;
    int             i, idx;
    uint32          entry_w[SOC_MAX_MEM_WORDS];

    field_len = soc_mem_field_length(unit, td_test_data.mem_write, UCQ_COUNTf);
    for (i = 0; i < 4; i++) {
        idx = index * 4 + i;

        /* field length need to be adjusted. */
        fval = soc_mem_field32_get(unit, td_test_data.mem, (void *)entbuf,
                                   ucq_field[i]);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit, "fval %d is %x\n"), i, fval));
        fval &= (1 << field_len) - 1;
        soc_mem_field32_set(unit, td_test_data.mem, entbuf, ucq_field[i], fval);
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit, "adjusted fval %d is %x\n"), i, fval));

        if (td_test_read_entry(unit, td_test_data.mem_write,
                               td_test_data.copyno, idx, entry_w) < 0) {
            return -1;
        }
        soc_mem_field32_set(unit, td_test_data.mem_write, entry_w, UCQ_COUNTf, fval);
        if (td_test_write_entry(unit, td_test_data.mem_write,
                                td_test_data.copyno, idx, entry_w) < 0) {
            return -1;
        }
    }

    return 0;
}

int tomahawk2_mor_test_verify(int unit) {
    soc_mem_t   mem = td_test_data.mem;
    uint8       *buf = NULL;
    int         alloc_size, num_entries, frag, i;
    int         index_min, index_max;
    void        *pentry;
    uint32      *entbuf;

    if (soc_pstats_sync(unit) < 0){
        return -1;
    }

    num_entries = soc_mem_index_max(unit, mem) + 1;
    alloc_size = sizeof(uint32) * soc_mem_entry_words(unit, mem);
    alloc_size *= num_entries;

    buf = sal_alloc(alloc_size, "pstats buffer");
    if (buf == NULL) {
        return -1;
    }

    if (soc_pstats_mem_get(unit, mem, buf, 0) < 0) {
        sal_free(buf);
        return -1;
    }

    for (frag = 0; frag < td_test_data.frag_count; frag++) {
        index_min = td_test_data.frag_index_min[frag];
        index_max = index_min + td_test_data.frag_seq_num_max[frag] -
            td_test_data.frag_seq_num_min[frag];
        entbuf = td_test_data.dma_ed +
            td_test_data.frag_seq_num_min[frag] * td_test_data.entwords;
        for (i = index_min; i <= index_max; i++) {
            pentry = soc_mem_table_idx_to_pointer(unit, mem, void*, buf, i);
            sal_memcpy(entbuf, (uint32*)pentry,
                       sizeof(uint32) * td_test_data.entwords);
            entbuf += td_test_data.entwords;
        }
    }

    if (TRUE == td_test_data.td_check_data) {
        /*
         * Verifing received results
         */
        if (td_test_verify(unit) < 0) {
            test_error(unit,
                       "Table DMA failed: inconsistency observed\n");
            td_test_dump_data(unit, TD_DEFAULT_DUMP_ENTRY_COUNT);
            sal_free(buf);
            return -1;
        }
    }

    sal_free(buf);
    return 0;
}
#endif/*BCM_TOMAHAWK2_SUPPORT*/

int
td_test_test(int unit, args_t *a, void *pa)
{
    int        i, j;
    uint32    *entbuf;
    int        frag;
    int        index_min, index_max;

#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "Note:  Cross Coupled Memory \n")));
    } else 
#endif
    {
        LOG_INFO(BSL_LS_APPL_TESTS,
                 (BSL_META_U(unit,
                             "Table DMA %s: copy %d, from entry %d for %d entries SEED = %d\n"),
                  td_test_data.mem_str,
                  td_test_data.copyno,
                  td_test_data.index_min,
                  td_test_data.count,
                  td_test_data.rnum));
        if (!soc_mem_dmaable(unit, td_test_data.mem, td_test_data.copyno)) {
            cli_out("WARNING: DMA will not be used for memory %s.%d.\n",
                    td_test_data.mem_str, td_test_data.copyno);
        }
#if 0
        /* NOTE: The following is not valid anymore as we always dma from h/w and
                 we bypass cache for mem tests */
        if (soc_mem_cache_get(unit, td_test_data.mem, td_test_data.copyno)) {
            cli_out("WARNING: All reads from memory %s.%d will come from cache.\n",
                    td_test_data.mem_str, td_test_data.copyno);
        }
#endif
    }

    td_test_data_clear(unit);
    if (td_test_data_init(unit, 1) < 0) {
        return -1;
    }
#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        if (soc_host_ccm_copy(unit, td_test_data.source, td_test_data.dma_ed,
                            td_test_data.count, 0) < 0) {
            test_error(unit, "CCM DMA Error\n");
            return -1;
        }
    } else
#endif
    /* If not a RO memory, write random data into table and hardware */
    if (!soc_mem_is_readonly(unit, td_test_data.mem)) {
        entbuf = td_test_data.source;
        if (0 != td_test_data.rnum) {
            sal_memset(entbuf, 0xff, WORDS2BYTES(td_test_data.bufwords));
        }

#if defined(BCM_XGS3_SWITCH_SUPPORT) || defined(BCM_DFE_SUPPORT) || defined(BCM_PETRA_SUPPORT)
        if ((SOC_IS_XGS3_SWITCH(unit) &&
             td_test_data.tslam_en && !td_test_data.pstats_ucq &&
             (soc_mem_slamable(unit, td_test_data.mem, td_test_data.copyno))) ||
            (SOC_IS_DFE(unit) || SOC_IS_DPP(unit))) {
            for (i = 0; i < td_test_data.count; i++) {
                td_test_random_entry(unit, td_test_data.mem, entbuf);
                if (td_test_data.accum_tcammask) {
                    /* data read back has dependency on mask */
                    if ((SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                          SOC_BLK_ESM) ||
                        (SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                               SOC_BLK_ETU)) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] &= ~td_test_data.tcammask[j];
                        }
                    } else if (soc_feature(unit, soc_feature_xy_tcam)) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] |= td_test_data.tcammask[j];
                        }
                    }
                }
                if (td_test_data.accum_forcemask) {
                    for (j = 0; j < td_test_data.entwords; j++) {
                        entbuf[j] &= ~td_test_data.forcemask[j];
                        entbuf[j] |= td_test_data.forcedata[j];
                    }
                }
                entbuf += td_test_data.entwords;
            }
            soc_cm_sflush(unit, (void *)td_test_data.source,
                          WORDS2BYTES(td_test_data.bufwords));
            for (frag = 0; frag < td_test_data.frag_count; frag++) {
                index_min = td_test_data.frag_index_min[frag];
                index_max = index_min + td_test_data.frag_seq_num_max[frag] -
                    td_test_data.frag_seq_num_min[frag];
                entbuf = td_test_data.source +
                    td_test_data.frag_seq_num_min[frag] * td_test_data.entwords;
                if (soc_mem_write_range(unit, td_test_data.mem,
                                        td_test_data.copyno,
                                        (td_test_data.tslam_dir ?
                                         index_min : index_max),
                                        (td_test_data.tslam_dir ?
                                         index_max : index_min),
                                        entbuf) < 0) {
                    test_error(unit,
                               "Table SLAM DMA Error:  Memory %s.%d\n",
                               td_test_data.mem_str, td_test_data.copyno);
                    return -1;
                }
            }
        } else
#endif /* BCM_XGS3_SWITCH_SUPPORT || BCM_DFE_SUPPORT || BCM_PETRA_SUPPORT */
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
      if ((SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) &&
            td_test_data.tslam_en &&
            (soc_mem_slamable(unit, td_test_data.mem, td_test_data.copyno))) {
            for (i = td_test_data.index_min; i <= td_test_data.index_max; ++i) {
                td_test_random_entry(unit, td_test_data.mem, entbuf);
                entbuf += td_test_data.entwords;
            }
            soc_cm_sflush(unit, (void *)td_test_data.source,
                          WORDS2BYTES(td_test_data.bufwords));
            if (soc_mem_write_range(unit, td_test_data.mem, td_test_data.copyno,
                                    (td_test_data.tslam_dir ?
                                    td_test_data.index_min : td_test_data.index_max),
                                    (td_test_data.tslam_dir ?
                                    td_test_data.index_max : td_test_data.index_min),
                                    td_test_data.source) < 0) {
                test_error(unit, "Table SLAM DMA Error: Memory %s.%d\n",
                           td_test_data.mem_str, td_test_data.copyno);
                return -1;
            }
        } else
#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */
        {
            for (frag = 0; frag < td_test_data.frag_count; frag++) {
                index_min = td_test_data.frag_index_min[frag];
                index_max = index_min + td_test_data.frag_seq_num_max[frag] -
                    td_test_data.frag_seq_num_min[frag];
                entbuf = td_test_data.source +
                    td_test_data.frag_seq_num_min[frag] * td_test_data.entwords;
                for (i = index_min; i <= index_max; i++) {
                    td_test_random_entry(unit, td_test_data.mem, entbuf);
                    if (td_test_data.accum_tcammask) {
                        /* data read back has dependency on mask */
                        if ((SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                             SOC_BLK_ESM) ||
                            (SOC_BLOCK_TYPE(unit, td_test_data.copyno) ==
                             SOC_BLK_ETU)) {
                            for (j = 0; j < td_test_data.entwords; j++) {
                                entbuf[j] &= ~td_test_data.tcammask[j];
                            }
                        } else if (soc_feature(unit, soc_feature_xy_tcam)) {
                            for (j = 0; j < td_test_data.entwords; j++) {
                                entbuf[j] |= td_test_data.tcammask[j];
                            }
                        }
                    }
                    if (td_test_data.accum_forcemask) {
                        for (j = 0; j < td_test_data.entwords; j++) {
                            entbuf[j] &= ~td_test_data.forcemask[j];
                            entbuf[j] |= td_test_data.forcedata[j];
                        }
                    }

                    if (td_test_data.pstats_ucq) {
#ifdef BCM_TOMAHAWK2_SUPPORT
                        if (SOC_IS_TOMAHAWK2(unit)) {
                            if (tomahawk2_pstats_ucq_test(unit, i, entbuf) < 0) {
                                return -1;
                            }
                        }
#endif
                    } else if (soc_mem_test_skip(unit, td_test_data.mem, i)) {
                        entbuf += td_test_data.entwords;
                        continue;
                    }

                    if (td_test_write_entry(unit, td_test_data.mem,
                                            td_test_data.copyno, i,
                                            entbuf) < 0) {
                        return -1;
                    }
                    entbuf += td_test_data.entwords;
                }
            }
        }
        if (td_test_data.mor_en) {
#ifdef BCM_TOMAHAWK2_SUPPORT
            if (SOC_IS_TOMAHAWK2(unit)) {
                if (tomahawk2_mor_test_verify(unit) < 0) {
                    return -1;
                }
            }
#endif
        } else
        if (td_test_dma_verify_helper(unit) < 0) {
            return -1;
        }
    } else {
        cli_out("Note:  memory %s is read only, so verifying a few\n",
                td_test_data.mem_str);
        cli_out("entries directly.\n");
    }

    return 0;
}


char tr71_test_usage[] = 
"TR71 (single memory write dma test) usage:\n"
" \n"
 #ifdef COMPILER_STRING_CONST_LIMIT
    "\nFull documentation cannot be displayed with -pedantic compiler\n";
 #else

  "Mem=<value> Memory - The memory to be checked\n"
  "Start=<value>  - Starting index\n"
  "Count=<value> how many rows\n"
  "readonlymemVerifyCount=<value> \n"
  "SEED=<value> the seed to use in the random function\n"
  "EccAsData=<value> use ecc as part of the data\n"
  "CheckData=<value> \n"
  "Help=<1/0>                      -  Specifies if tr 71 help is on and exit or off (off by default)\n"
  "To enable printing values (write/read) of each memory proceed: debug +test +verb\n"
  "\n";
  #endif



int
td_test_init(int unit, args_t *a, void **pa)
{
    parse_table_t  pt;
    int            index_min, index_max;
    int count_arg;
    uint32 help_status_default = 0;
    uint32 help_status=0;

    if (td_test_data_init(unit, 0) < 0) {
        return -1;
    }

    parse_table_init(unit, &pt);

    parse_table_add(&pt, "Mem", PQ_DFL|PQ_STRING|PQ_STATIC,
                    0, &td_test_data.mem_str, NULL);
    parse_table_add(&pt, "Start", PQ_DFL|PQ_INT,
                    0, &td_test_data.index_min, NULL);
    parse_table_add(&pt, "Count", PQ_DFL|PQ_INT,
                    (void *)(-1), &count_arg, NULL);
    parse_table_add(&pt, "readonlymemVerifyCount", PQ_DFL|PQ_INT,
                    0, &td_test_data.rom_ver_count, NULL);
    parse_table_add(&pt, "SEED",        PQ_INT|PQ_DFL,  0,
                    &td_test_data.rnum,       NULL);
    parse_table_add(&pt, "EccAsData", PQ_BOOL|PQ_DFL,  0,
                    &td_test_data.ecc_as_data, NULL);
#ifdef BCM_XGS3_SWITCH_SUPPORT
    if (SOC_IS_XGS3_SWITCH(unit)) {
        parse_table_add(&pt, "TSlamEnable", PQ_BOOL|PQ_DFL,
                        0, &td_test_data.tslam_en, NULL);
        parse_table_add(&pt, "SlamLowToHigh", PQ_BOOL|PQ_DFL,
                        0, &td_test_data.tslam_dir, NULL);
        parse_table_add(&pt, "TDmaEnable", PQ_BOOL|PQ_DFL,
                        0, &td_test_data.tdma_en, NULL);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit)) {
      parse_table_add(&pt, "SlamLowToHigh", PQ_BOOL|PQ_DFL,
              0, &td_test_data.tslam_dir,NULL);
    }
#endif /* defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) */
#ifdef BCM_CMICM_SUPPORT
    parse_table_add(&pt, "CrossCoupledMemory", PQ_BOOL|PQ_DFL,
                    0, &td_test_data.ccm, NULL);
#endif
#ifdef BCM_XGS3_SWITCH_SUPPORT /* For now implement only for katana */
    if (SOC_IS_KATANA(unit)) {
        parse_table_add(&pt, "SourceAddressOverride", PQ_INT|PQ_DFL,
                        0, &td_test_data.srcaddr, NULL);
        parse_table_add(&pt, "DestAddressOverride", PQ_INT|PQ_DFL,
                        0, &td_test_data.dstaddr, NULL);
    }
    if (SOC_IS_TOMAHAWK2(unit)) {
        td_test_data.pstats_ucq = 0;
        td_test_data.mor_en = 0;
        parse_table_add(&pt, "DmaMOR", PQ_BOOL|PQ_DFL,
                        0, &td_test_data.mor_en, NULL);
    }
#endif /* BCM_XGS3_SWITCH_SUPPORT */
    parse_table_add(&pt, "CheckData",   PQ_BOOL|PQ_DFL, 0,
                    &td_test_data.td_check_data, NULL);

    parse_table_add(&pt, "Help",    PQ_DFL|PQ_INT, &help_status_default, &help_status,           NULL);

    if (parse_arg_eq(a, &pt) < 0 || ARG_CNT(a) != 0) {
        test_error(unit,
                   "%s: Invalid option: %s\n", ARG_CMD(a),
                   ARG_CUR(a) ? ARG_CUR(a) : "*");
        parse_arg_eq_done(&pt);
        return -1;
    }
    if (help_status == 1) {
        cli_out("%s\n",tr71_test_usage);
        return -1;
    }

    if (parse_memory_name(unit, &td_test_data.mem, td_test_data.mem_str,
                          &td_test_data.copyno, 0) < 0) {
        test_error(unit, "Memory \"%s\" is invalid\n",
                   td_test_data.mem_str);
        parse_arg_eq_done(&pt);
        return -1;
    }
#ifdef BCM_PETRA_SUPPORT
if (SOC_IS_ARAD(unit) ) {
    if (!soc_mem_is_valid(unit, td_test_data.mem) ||
        soc_mem_is_readonly(unit, td_test_data.mem)

        || (SOC_IS_ARAD(unit) && (soc_mem_is_writeonly(unit, td_test_data.mem) || soc_mem_is_signal(unit, td_test_data.mem)))

        ) {
        test_error(unit, "Memory %s is invalid/readonly/writeonly/signal \n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
        return -1;
    }

    if(SOC_IS_ARAD(unit)) {
        switch(td_test_data.mem) {
            case BRDC_FSRD_FSRD_WL_EXT_MEMm:
            case IQM_MEM_8000000m:
            case NBI_TBINS_MEMm:
            case NBI_RBINS_MEMm:
            case PORT_WC_UCMEM_DATAm:
                test_error(unit, "Memory %s is invalid/readonly/writeonly/signal \n",
                           SOC_MEM_UFNAME(unit, td_test_data.mem));
                return -1;
            default:
                break;
        }
    }
}
#endif

#ifdef BCM_DFE_SUPPORT
    if (SOC_IS_DFE(unit) )
    {
        int rv;
        int is_filtered = 0;

        if (!soc_mem_is_valid(unit, td_test_data.mem) || soc_mem_is_readonly(unit, td_test_data.mem) ||
             (soc_mem_is_writeonly(unit, td_test_data.mem) || soc_mem_is_signal(unit, td_test_data.mem)))
        {
            test_error(unit, "Memory %s is invalid/readonly/writeonly/signal \n",
                       SOC_MEM_UFNAME(unit, td_test_data.mem));
            return -1;
        }

        rv = MBCM_DFE_DRIVER_CALL(unit, mbcm_dfe_drv_test_mem_filter, (unit,  td_test_data.mem, &is_filtered));
        if (rv != SOC_E_NONE)
        {
            return rv;
        }
        if (is_filtered)
        {
               test_error(unit, "Memory %s is invalid/readonly/writeonly/signal \n",
                       SOC_MEM_UFNAME(unit,  td_test_data.mem));
               return -1;
        }
    }
#endif /*BCM_DFE_SUPPORT*/

    if (count_arg != -1) {
        td_test_data.count = count_arg;
    }

#if defined(BCM_SIRIUS_SUPPORT) || defined(BCM_CALADAN3_SUPPORT) || \
    defined(BCM_TOMAHAWK_SUPPORT)
    if (SOC_IS_SIRIUS(unit) || SOC_IS_CALADAN3(unit) || SOC_IS_TOMAHAWKX(unit)) {
      /* if Count is entered use that, otherwise set to zero
       * to allow full range of memory to be tested */
        if (count_arg == -1) {
            td_test_data.count = 0;
        }
    }
#endif /* defined(BCM_SIRIUS_SUPPORT)  || defined(BCM_CALADAN3_SUPPORT) \
          defined(BCM_TOMAHAWK_SUPPORT) */

    if (td_test_data.srcaddr || td_test_data.dstaddr) {
        test_msg("Overriding buffers can potentially corrupt the system\n");
        test_msg("Don't do it unless you know what you're doing\n");
    }

    if (td_test_data.copyno == COPYNO_ALL) {
        td_test_data.copyno = SOC_MEM_BLOCK_ANY(unit, td_test_data.mem);
    }
    if (!SOC_MEM_BLOCK_VALID(unit, td_test_data.mem, td_test_data.copyno)) {
        test_error(unit,
                   "Invalid copyno %d specified in %s\n",
                   td_test_data.copyno, td_test_data.mem_str);
        parse_arg_eq_done(&pt);
        return -1;
    }

    index_min = soc_mem_index_min(unit, td_test_data.mem);
    index_max = soc_mem_index_max(unit, td_test_data.mem);

    if (td_test_data.index_min <= 0) {
        td_test_data.index_min = index_min;
    }
    if (td_test_data.index_min > index_max) {
        cli_out("Min index out of range: %d\n",  td_test_data.index_min);
        td_test_data.index_min = index_min;
        cli_out("Changed to %d\n", td_test_data.index_min);
    }

    if (td_test_data.count <= 0) {
    /*
     * do not use soc_mem_index_count,index_max may have been,
     * index_max may have been modified above
     */
        td_test_data.count = index_max - index_min + 1;
    }

    if (td_test_data.index_min + td_test_data.count - 1 > index_max) {
        td_test_data.count = index_max - td_test_data.index_min + 1;
        cli_out("Reduced the count to %d\n", td_test_data.count);
    }

    if (td_test_data.count == 0) {
        if (soc_feature(unit,soc_feature_esm_support)) {
              if ((td_test_data.mem == L3_DEFIPm && SOC_MEM_IS_ENABLED(unit, L3_DEFIPm)) ||
                  (td_test_data.mem == L3_DEFIP_ONLYm && SOC_MEM_IS_ENABLED(unit, L3_DEFIP_ONLYm)) ||
                  (td_test_data.mem == L3_DEFIP_DATA_ONLYm && SOC_MEM_IS_ENABLED(unit, L3_DEFIP_DATA_ONLYm)) ||
                  (td_test_data.mem == L3_DEFIP_HIT_ONLYm && SOC_MEM_IS_ENABLED(unit, L3_DEFIP_DATA_ONLYm)) ||
                  (td_test_data.mem == L3_DEFIP_PAIR_128_DATA_ONLYm && SOC_MEM_IS_ENABLED(unit, L3_DEFIP_PAIR_128_DATA_ONLYm)) ||
                  (td_test_data.mem == L3_DEFIP_PAIR_128_HIT_ONLYm && SOC_MEM_IS_ENABLED(unit, L3_DEFIP_PAIR_128_HIT_ONLYm))) {
                  /* The above internal memories will not be supported if External TCAM is present.
                  These internal memories are set to 0 and hence can not run the test on these memories.*/
                  return BCM_E_UNAVAIL;
              } else {
                  test_error(unit,
                     "Cannot test memory %s:  No entries.\n",
                        SOC_MEM_UFNAME(unit, td_test_data.mem));
                  parse_arg_eq_done(&pt);
                  return -1;
	      }
       } else {
           test_error(unit,
           "Cannot test memory %s:  No entries.\n",
           SOC_MEM_UFNAME(unit, td_test_data.mem));
           parse_arg_eq_done(&pt);
           return -1;
        }
    }

    /* for read only memory, number of entries to verify */
    if (td_test_data.rom_ver_count < 0) {
        td_test_data.rom_ver_count = TD_DEFAULT_ROM_VER_COUNT;
    }
    if (td_test_data.rom_ver_count > td_test_data.count) {
        td_test_data.rom_ver_count = td_test_data.count;
    }

    /* Disable non-atomic TCAM write handling */
    td_test_data.saved_tcam_protect_write =
        SOC_CONTROL(unit)->tcam_protect_write;
    SOC_CONTROL(unit)->tcam_protect_write = FALSE;

    if(SOC_IS_DFE(unit)){
        SOC_MEM_TEST_SKIP_CACHE_SET(unit, 1);
        if (soc_mem_parity_control(unit, td_test_data.mem, td_test_data.copyno,
                               FALSE) < 0) {
            test_error(unit, "Could not disable parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
            parse_arg_eq_done(&pt);
            return -1;
        }
    } else {
        if (td_rand_common_clear(unit, td_test_data.mem, td_test_data.copyno)) {
            parse_arg_eq_done(&pt);
            return -1;
        }
    }

    if (soc_mem_cpu_write_control(unit, td_test_data.mem, td_test_data.copyno,
                                  TRUE, &td_test_data.orig_enable) < 0) {
        test_error(unit, "Could not enable exclusive cpu write on memory %s\n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
        parse_arg_eq_done(&pt);
        return -1;
    }

    /*
     * Turn off L2 task to keep it from going crazy if L2 memory is
     * being tested.
     */

#ifdef BCM_XGS_SWITCH_SUPPORT
    if (soc_feature(unit, soc_feature_arl_hashed)) {
        (void)soc_l2x_stop(unit);
    }
#endif

#ifdef BCM_TRIUMPH_SUPPORT
    if (soc_feature(unit, soc_feature_esm_support)) {
        /* Some tables have dependency on the content of other table */
        switch (td_test_data.mem) {
        case EXT_ACL360_TCAM_DATAm:
        case EXT_ACL360_TCAM_DATA_IPV6_SHORTm:
            if (SOC_FAILURE(soc_mem_clear(unit, EXT_ACL360_TCAM_MASKm,
                MEM_BLOCK_ALL, TRUE)))
            {
                test_error(unit, "Failed to clear EXT_ACL360_TCAM_MASKm\n");
                parse_arg_eq_done(&pt);
                return -1;
            }
            break;
        case EXT_ACL432_TCAM_DATAm:
        case EXT_ACL432_TCAM_DATA_IPV6_LONGm:
        case EXT_ACL432_TCAM_DATA_L2_IPV4m:
        case EXT_ACL432_TCAM_DATA_L2_IPV6m:
            if (SOC_FAILURE(soc_mem_clear(unit, EXT_ACL432_TCAM_MASKm,
                MEM_BLOCK_ALL, TRUE)))
            {
                test_error(unit, "Failed to clear EXT_ACL432_TCAM_MASKm\n");
                parse_arg_eq_done(&pt);
                return -1;
            }
            break;
        default:
            break;
        }
    }
#endif /* BCM_TRIUMPH_SUPPORT */

    td_test_data.index_max = td_test_data.index_min +
        td_test_data.count - 1;

#ifdef BCM_TOMAHAWK2_SUPPORT
        if (SOC_IS_TOMAHAWK2(unit)) {
            if (tomahawk2_tr_init(unit) < 0) {
                parse_arg_eq_done(&pt);
                return -1;
            }
        }
#endif /* BCM_TOMAHAWK2_SUPPORT */

    if (td_test_data.mor_en) {
#ifdef BCM_TOMAHAWK2_SUPPORT
        if (SOC_IS_TOMAHAWK2(unit)) {
            if (tomahawk2_mor_init(unit) < 0) {
                test_error(unit, "Table DMA mor init error\n");
                parse_arg_eq_done(&pt);
                return -1;
            }
        }
#endif /* BCM_TOMAHAWK2_SUPPORT */
    }

    parse_arg_eq_done(&pt);
#ifdef BCM_PETRA_SUPPORT
    if (SOC_IS_ARAD(unit))
    {
      bcm_common_linkscan_enable_set(unit,0);
      soc_counter_stop(unit);
    }
#endif
    return 0;
}

int
td_test_done(int unit, void *pa)
{
    td_test_data_clear(unit);
#ifdef BCM_CMICM_SUPPORT
    if(td_test_data.ccm) {
        return 0;
    }
#endif

    /* Restore non-atomic TCAM write handling status */
    SOC_CONTROL(unit)->tcam_protect_write =
        td_test_data.saved_tcam_protect_write;

    if (soc_mem_cpu_write_control(unit, td_test_data.mem, td_test_data.copyno,
                                  td_test_data.orig_enable,
                                  &td_test_data.orig_enable) < 0) {
        test_error(unit, "Could not disable exclusive cpu write on memory "
                   "%s\n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
        return -1;
    }

    if(SOC_IS_DFE(unit)) {
        SOC_MEM_TEST_SKIP_CACHE_SET(unit, 0);
        if (soc_mem_parity_restore(unit, td_test_data.mem,
                               td_test_data.copyno) < 0) {
            test_error(unit, "Could not restore parity warnings on memory %s\n",
                   SOC_MEM_UFNAME(unit, td_test_data.mem));
            return -1;
        }
    } else {
        if (td_rand_common_restore(unit, td_test_data.mem, td_test_data.copyno)) {
            return -1;
        }
    }

    return 0;
}

#endif /* BCM_XGS_SWITCH_SUPPORT || BCM_SIRIUS_SUPPORT  || BCM_CALADAN3_SUPPORT */
