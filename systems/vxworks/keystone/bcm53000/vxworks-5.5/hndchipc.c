/*
 * BCM47XX support code for some chipcommon facilities (uart, jtagm)
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * $Id: hndchipc.c,v 1.2 Broadcom SDK $
 */

#include <typedefs.h>
#include <bcmdefs.h>
#include <osl.h>
#include <bcmutils.h>
#include <siutils.h>
#include <bcmnvram.h>
#include <hndsoc.h>
#include <sbchipc.h>
#include <hndchipc.h>
#include <hndcpu.h>

#if defined(vxworks)
#include <config.h>
#endif

/* debug/trace */
#ifdef BCMDBG_ERR
#define CC_ERROR(args)  printf args
#else
#define CC_ERROR(args)
#endif  /* BCMDBG_ERR */

#ifdef BCMDBG
#define CC_MSG(args)    printf args
#else
#define CC_MSG(args)
#endif  /* BCMDBG */

/* interested chipcommon interrupt source
 *  - GPIO
 *  - EXTIF
 *  - ECI
 *  - PMU
 *  - UART
 */
#define MAX_CC_INT_SOURCE 5

/* chipc secondary isr info */
typedef struct {
    uint intmask;       /* int mask */
    cc_isr_fn isr;      /* secondary isr handler */
    void *cbdata;       /* pointer to private data */
} cc_isr_info_t;

static cc_isr_info_t cc_isr_desc[MAX_CC_INT_SOURCE];

/* chip common intmask */
static uint32 cc_intmask = 0;

/*
 * Initializes UART access. The callback function will be called once
 * per found UART.
 */
void
BCMINITFN(si_serial_init)(si_t *sih, si_serial_init_fn add, uint32 baudrate)
{
    osl_t *osh;
    void *regs;
    chipcregs_t *cc;
    uint32 rev, cap, pll, baud_base, div;
    uint irq;
    int i, n;

    osh = si_osh(sih);

    cc = (chipcregs_t *)si_setcoreidx(sih, SI_CC_IDX);
    ASSERT(cc);

    /* Determine core revision and capabilities */
    rev = sih->ccrev;
    cap = sih->cccaps;
    pll = cap & CC_CAP_PLL_MASK;

    /* Determine IRQ */
    irq = si_irq(sih);

    if (CCPLL_ENAB(sih) && pll == PLL_TYPE1) {
        /* PLL clock */
        baud_base = si_clock_rate(pll,
                                  R_REG(osh, &cc->clockcontrol_n),
                                  R_REG(osh, &cc->clockcontrol_m2));
        div = 1;
    } else {
        /* Fixed ALP clock */
        /* XXX rev. 15 in 4785 breaks the rule! */
        if (rev >= 11 && rev != 15) {
            baud_base = si_alp_clock(sih);
            div = 1;
            
            /*
             * If baudrate parameter is given with a non-zero value,
             * UART clock will be divided for the given baud rate before 
             * feeding into UART module.
             * Note: UART driver could also adjust UART module's divider for 
             *       clock fine tunning.
             */
            if (baudrate) {
                /* Turn off UART clock before switching clock source */
                if (rev >= 21)
                    AND_REG(osh, &cc->corecontrol, ~CC_UARTCLKEN);

                /* Don't set the override bit so we divide it */
                AND_REG(osh, &cc->corecontrol, ~CC_UARTCLKO);

                div = baud_base / (baudrate * 16); 
                div = (div + 1) & ~0x1;
                AND_REG(osh, &cc->clkdiv, ~CLKD_UART);
                OR_REG(osh, &cc->clkdiv, div);

                /* Turn back UART clock on */
                if (rev >= 21)
                    OR_REG(osh, &cc->corecontrol, CC_UARTCLKEN);
            } else {

                /* Turn off UART clock before switching clock source */
                if (rev >= 21)
                    AND_REG(osh, &cc->corecontrol, ~CC_UARTCLKEN);

                OR_REG(osh, &cc->corecontrol, CC_UARTCLKO);

                /* Turn back UART clock on */
                if (rev >= 21)
                    OR_REG(osh, &cc->corecontrol, CC_UARTCLKEN);
            }

            if (CONSOLE_TTY == 1) {
                /* Use UART1, enable "UART1 use GPIO" bit */
                OR_REG(osh, &cc->corecontrol, 0x80);
            }
        } else if (rev >= 3) {
            /* Internal backplane clock */
            baud_base = si_clock(sih);
            div = 2;    /* Minimum divisor */
            W_REG(osh, &cc->clkdiv,
                  ((R_REG(osh, &cc->clkdiv) & ~CLKD_UART) | div));
        } else {
            /* Fixed internal backplane clock */
            baud_base = 88000000;
            div = 48;
        }

        /* Clock source depends on strapping if UartClkOverride is unset */
        if ((rev > 0) &&
            ((R_REG(osh, &cc->corecontrol) & CC_UARTCLKO) == 0)) {
            if ((cap & CC_CAP_UCLKSEL) == CC_CAP_UINTCLK) {
                /* Internal divided backplane clock */
                baud_base /= div;
            } else {
                /* Assume external clock of 1.8432 MHz */
                baud_base = 1843200;
            }
        }
    }

    /* Add internal UARTs */
    n = cap & CC_CAP_UARTS_MASK;
    for (i = 0; i < n; i++) {
        /* Register offset changed after revision 0 */
        if (rev)
            regs = (void *)((ulong) &cc->uart0data + (i * 256));
        else
            regs = (void *)((ulong) &cc->uart0data + (i * 8));

        if (add)
            add(regs, irq, baud_base, 0);
    }
}

/*
 * Initialize jtag master and return handle for
 * jtag_rwreg. Returns NULL on failure.
 */
void *
hnd_jtagm_init(si_t *sih, uint clkd, bool exttap)
{
    void *regs;
        osl_t *osh;

        osh = si_osh(sih);
    if ((regs = si_setcoreidx(sih, SI_CC_IDX)) != NULL) {
        chipcregs_t *cc = (chipcregs_t *) regs;
        uint32 tmp;

        /*
         * Determine jtagm availability from
         * core revision and capabilities.
         */

        /*
         * Corerev 10 has jtagm, but the only chip
         * with it does not have a mips, and
         * the layout of the jtagcmd register is
         * different. We'll only accept >= 11.
         */
        if (sih->ccrev < 11)
            return (NULL);

        if ((sih->cccaps & CC_CAP_JTAGP) == 0)
            return (NULL);

        /* Set clock divider if requested */
        if (clkd != 0) {
            tmp = R_REG(osh, &cc->clkdiv);
            tmp = (tmp & ~CLKD_JTAG) |
                ((clkd << CLKD_JTAG_SHIFT) & CLKD_JTAG);
            W_REG(osh, &cc->clkdiv, tmp);
        }

        /* Enable jtagm */
        tmp = JCTRL_EN | (exttap ? JCTRL_EXT_EN : 0);
        W_REG(osh, &cc->jtagctrl, tmp);
    }

    return (regs);
}

void
hnd_jtagm_disable(osl_t *osh, void *h)
{
    chipcregs_t *cc = (chipcregs_t *)h;

    W_REG(osh, &cc->jtagctrl, R_REG(osh, &cc->jtagctrl) & ~JCTRL_EN);
}

/*
 * Read/write a jtag register. Assumes a target with
 * 8 bit IR and 32 bit DR.
 */
#define IRWIDTH     8   /* Default Instruction Register width */
#define DRWIDTH     32  /* Default Data Register width */

uint32
jtag_rwreg(osl_t *osh, void *h, uint32 ir, uint32 dr)
{
    chipcregs_t *cc = (chipcregs_t *) h;
    uint32 tmp;

    W_REG(osh, &cc->jtagir, ir);
    W_REG(osh, &cc->jtagdr, dr);
    tmp = JCMD_START | JCMD_ACC_IRDR |
        ((IRWIDTH - 1) << JCMD_IRW_SHIFT) |
        (DRWIDTH - 1);
    W_REG(osh, &cc->jtagcmd, tmp);
    while (((tmp = R_REG(osh, &cc->jtagcmd)) & JCMD_BUSY) == JCMD_BUSY) {
        /* OSL_DELAY(1); */
    }

    tmp = R_REG(osh, &cc->jtagdr);
    return (tmp);
}


/*
 * Interface to register chipc secondary isr
 */

bool
BCMINITFN(si_cc_register_isr)(si_t *sih, cc_isr_fn isr, uint32 ccintmask, void *cbdata)
{
    bool done = FALSE;
    chipcregs_t *regs;
    uint origidx;
    uint i;

    /* Save the current core index */
    origidx = si_coreidx(sih);
    regs = si_setcoreidx(sih, SI_CC_IDX);
    ASSERT(regs);

    for (i = 0; i < MAX_CC_INT_SOURCE; i++) {
        if (cc_isr_desc[i].isr == NULL) {
            cc_isr_desc[i].isr = isr;
            cc_isr_desc[i].cbdata = cbdata;
            cc_isr_desc[i].intmask = ccintmask;
            done = TRUE;
            break;
        }
    }

    if (done) {
        cc_intmask = R_REG(si_osh(sih), &regs->intmask);
        cc_intmask |= ccintmask;
        W_REG(si_osh(sih), &regs->intmask, cc_intmask);
    }

    /* restore original coreidx */
    si_setcoreidx(sih, origidx);
    return done;
}

/* 
 * chipc primary interrupt handler
 *
 */

void
si_cc_isr(si_t *sih, chipcregs_t *regs)
{
    uint32 ccintstatus;
    uint32 intstatus;
    uint32 i;

    /* prior to rev 21 chipc interrupt means uart and gpio */
    if (sih->ccrev >= 21)
        ccintstatus = R_REG(si_osh(sih), &regs->intstatus) & cc_intmask;
    else
        ccintstatus = (CI_UART | CI_GPIO);

    for (i = 0; i < MAX_CC_INT_SOURCE; i ++) {
        if ((cc_isr_desc[i].isr != NULL) &&
            (intstatus = (cc_isr_desc[i].intmask & ccintstatus))) {
            (cc_isr_desc[i].isr)(cc_isr_desc[i].cbdata, intstatus);
        }
    }
}
