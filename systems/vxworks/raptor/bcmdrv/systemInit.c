/*
 * $Id: systemInit.c,v 1.12 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 */
#include "vxWorks.h"
#include "intLib.h"
#if (CPU_FAMILY == MIPS)
#include "mbz.h"
#endif
#include "config.h"
#include "stdio.h"
#include "assert.h"
#include "time.h"
#include "taskLib.h"
#include "sysLib.h"
#include "string.h"
#include "drv/pci/pciConfigLib.h"
#include "drv/pci/pciIntLib.h"
#include "systemInit.h"
#include "dmaOps.h"

extern rxq_ctrl_t g_rxq_ctrl;

int globalCount;

#include "memreg.c"

#if defined(BCM56218)

#define RAPTOR_GMII_MASK                (0x00000008)
#define RAPTOR_GMII_MASK_HI             (0x00000000)
#define RAPTOR_GE_CAP_PORT_MASK         (0xfffffff8)
#define RAPTOR_GE_CAP_PORT_MASK_HI      (0x003fffff)
#define RAPTOR_INT_PHY_MASK             (0xfffffff6)
#define RAPTOR_INT_PHY_MASK_HI          (0x003fffff)

#elif defined(BCM56218P48)

#define RAPTOR_GMII_MASK                (0x00000008)
#define RAPTOR_GMII_MASK_HI             (0x00000000)
#define RAPTOR_GE_CAP_PORT_MASK         (0xfffffffe)
#define RAPTOR_GE_CAP_PORT_MASK_HI      (0x003fffff)
#define RAPTOR_INT_PHY_MASK             (0xfffffff6)
#define RAPTOR_INT_PHY_MASK_HI          (0x003fffff)

#elif defined(BCM56214)

#define RAPTOR_GMII_MASK                (0x00000008)
#define RAPTOR_GMII_MASK_HI             (0x00000000)
#define RAPTOR_GE_CAP_PORT_MASK         (0x3ffffff8)
#define RAPTOR_GE_CAP_PORT_MASK_HI      (0x00000000)
#define RAPTOR_INT_PHY_MASK             (0x3ffffff6)
#define RAPTOR_INT_PHY_MASK_HI          (0x00000000)

#elif defined(BCM56018)

/* RAPTOR FE PART */
#define RAPTOR_GMII_MASK                (0x00000008)
#define RAPTOR_GMII_MASK_HI             (0x00000000)
#define RAPTOR_GE_CAP_PORT_MASK         (0x00000038)
#define RAPTOR_GE_CAP_PORT_MASK_HI      (0x00000000)
#define RAPTOR_INT_PHY_MASK             (0x00000036)
#define RAPTOR_INT_PHY_MASK_HI          (0x00000000)

#elif defined(BCM56014)

#define RAPTOR_GMII_MASK                (0x00000008)
#define RAPTOR_GMII_MASK_HI             (0x00000000)
#define RAPTOR_GE_CAP_PORT_MASK         (0x00000038)
#define RAPTOR_GE_CAP_PORT_MASK_HI      (0x00000000)
#define RAPTOR_INT_PHY_MASK             (0x00000036)
#define RAPTOR_INT_PHY_MASK_HI          (0x00000000)

#else
#error "NO Device type specified"
#endif

#define IS_GMII_PORT(p)     \
            (((p) < 32) ? ((1 << (p)) & RAPTOR_GMII_MASK) : \
                         ((1 << ((p) - 32)) & RAPTOR_GMII_MASK_HI))

#define IS_INT_PHY_PRESENT(p)     \
            (((p) < 32) ? ((1 << (p)) & RAPTOR_INT_PHY_MASK) : \
                         ((1 << ((p) - 32)) & RAPTOR_INT_PHY_MASK_HI))

#define IS_GE_CAP(p)     \
            (((p) < 32) ? ((1 << (p)) & RAPTOR_GE_CAP_PORT_MASK) : \
                         ((1 << ((p) - 32)) & RAPTOR_GE_CAP_PORT_MASK_HI))



/* Some defines */
/* Registers */

#define	MII_BMCR	0x00 	/* Basic Mode Control (rw) */
#define	MII_BMSR	0x01	/* Basic Mode Status (ro) */
#define MII_PHYIDR1	0x02
#define MII_PHYIDR2	0x03
#define MII_ANAR	0x04	/* Autonegotiation Advertisement */
#define	MII_ANLPAR	0x05	/* Autonegotiation Link Partner Ability (rw) */
#define MII_ANER	0x06	/* Autonegotiation Expansion */
#define MII_K1CTL	0x09	/* 1000baseT control */
#define MII_K1STSR	0x0A	/* 1K Status Register (ro) */
#define MII_AUXCTL	0x18	/* aux control register */


/* Basic Mode Control register (RW) */

#define BMCR_RESET		0x8000
#define BMCR_LOOPBACK		0x4000
#define BMCR_SPEED0		0x2000
#define BMCR_ANENABLE		0x1000
#define BMCR_POWERDOWN		0x0800
#define BMCR_ISOLATE		0x0400
#define BMCR_RESTARTAN		0x0200
#define BMCR_DUPLEX		0x0100
#define BMCR_COLTEST		0x0080
#define BMCR_SPEED1		0x0040
#define BMCR_SPEED1000		(BMCR_SPEED1)
#define BMCR_SPEED100		(BMCR_SPEED0)
#define BMCR_SPEED10		0


/* Basic Mode Status register (RO) */

#define BMSR_100BT4		0x8000
#define BMSR_100BT_FDX		0x4000
#define BMSR_100BT_HDX  	0x2000
#define BMSR_10BT_FDX   	0x1000
#define BMSR_10BT_HDX   	0x0800
#define BMSR_100BT2_FDX 	0x0400
#define BMSR_100BT2_HDX 	0x0200
#define BMSR_1000BT_XSR		0x0100
#define BMSR_PRESUP		0x0040
#define BMSR_ANCOMPLETE		0x0020
#define BMSR_REMFAULT		0x0010
#define BMSR_AUTONEG		0x0008
#define BMSR_LINKSTAT		0x0004
#define BMSR_JABDETECT		0x0002
#define BMSR_EXTCAPAB		0x0001


/* Autonegotiation Advertisement register (RW) */

#define ANAR_NP			0x8000
#define ANAR_RF			0x2000
#define ANAR_ASYPAUSE		0x0800
#define ANAR_PAUSE		0x0400
#define ANAR_T4			0x0200
#define ANAR_TXFD		0x0100
#define ANAR_TXHD		0x0080
#define ANAR_10FD		0x0040
#define ANAR_10HD		0x0020
#define ANAR_PSB		0x001F

#define PSB_802_3		0x0001	/* 802.3 */

/* Autonegotiation Link Partner Abilities register (RW) */

#define ANLPAR_NP		0x8000
#define ANLPAR_ACK		0x4000
#define ANLPAR_RF		0x2000
#define ANLPAR_ASYPAUSE		0x0800
#define ANLPAR_PAUSE		0x0400
#define ANLPAR_T4		0x0200
#define ANLPAR_TXFD		0x0100
#define ANLPAR_TXHD		0x0080
#define ANLPAR_10FD		0x0040
#define ANLPAR_10HD		0x0020
#define ANLPAR_PSB		0x001F


/* Autonegotiation Expansion register (RO) */

#define ANER_PDF		0x0010
#define ANER_LPNPABLE		0x0008
#define ANER_NPABLE		0x0004
#define ANER_PAGERX		0x0002
#define ANER_LPANABLE		0x0001


#define ANNPTR_NP		0x8000
#define ANNPTR_MP		0x2000
#define ANNPTR_ACK2		0x1000
#define ANNPTR_TOGTX		0x0800
#define ANNPTR_CODE		0x0008

#define ANNPRR_NP		0x8000
#define ANNPRR_MP		0x2000
#define ANNPRR_ACK3		0x1000
#define ANNPRR_TOGTX		0x0800
#define ANNPRR_CODE		0x0008


#define K1TCR_TESTMODE		0x0000
#define K1TCR_MSMCE		0x1000
#define K1TCR_MSCV		0x0800
#define K1TCR_RPTR		0x0400
#define K1TCR_1000BT_FDX 	0x200
#define K1TCR_1000BT_HDX 	0x100

#define K1STSR_MSMCFLT		0x8000
#define K1STSR_MSCFGRES		0x4000
#define K1STSR_LRSTAT		0x2000
#define K1STSR_RRSTAT		0x1000
#define K1STSR_LP1KFD		0x0800
#define K1STSR_LP1KHD   	0x0400
#define K1STSR_LPASMDIR		0x0200

#define K1SCR_1KX_FDX		0x8000
#define K1SCR_1KX_HDX		0x4000
#define K1SCR_1KT_FDX		0x2000
#define K1SCR_1KT_HDX		0x1000

#define ETHER_SPEED_AUTO	0	/* Auto detect */
#define ETHER_SPEED_UNKNOWN	0	/* Speed not known (on link status) */
#define ETHER_SPEED_10HDX	1	/* 10MB hdx and fdx */
#define ETHER_SPEED_10FDX	2
#define ETHER_SPEED_100HDX	3	/* 100MB hdx and fdx */
#define ETHER_SPEED_100FDX	4
#define ETHER_SPEED_1000HDX	5	/* 1000MB hdx and fdx */
#define ETHER_SPEED_1000FDX	6

extern int GetMacFromFlash(unsigned char *mac);
/*************************************************************************/
/************************* local functions *******************************/
/*************************************************************************/
static void _systemInit();
#ifndef POLLING_MODE
static void soc_intr(void *_unit);
#endif

#define SOC_IS_RAPTOR(id) \
    ((((id) & 0xB000) == 0xB000) || (((id) & 0xC000) == 0xC000))

/**********************************************************/
/*************************** local variables **************/
/**********************************************************/
static vxbde_dev_t _devices[MAX_DEVICES];
static uint32      _devicesIntrMasks[MAX_DEVICES];
static vxbde_bus_t _bus;
int _n_devices = 0;

#define SCHAN_LOCK(unit) \
    sal_mutex_take(GET_SOC_DMA_MANAGER(unit)->schanMutex, sal_mutex_FOREVER)
#define SCHAN_UNLOCK(unit) \
    sal_mutex_give(GET_SOC_DMA_MANAGER(unit)->schanMutex)

#define MIIM_LOCK(unit) \
    sal_mutex_take(GET_SOC_DMA_MANAGER(unit)->miimMutex, sal_mutex_FOREVER)
#define MIIM_UNLOCK(unit) \
    sal_mutex_give(GET_SOC_DMA_MANAGER(unit)->miimMutex)

static int
rx_queue_setup(int size)
{
    int total;

    sal_memset(&g_rxq_ctrl, 0, sizeof(struct rxq_ctrl_s));

    total = sizeof(netdrv_pkt_t) * size;
    if ((g_rxq_ctrl.rxq_data = sal_alloc(total, "queue")) == NULL) {
        return -1;
    }
    sal_memset(g_rxq_ctrl.rxq_data, 0, total);

    return 0;
}


int
bcmSystemInit(void)
{
    PRINTF_DEBUG0(("Initializing BCM Chips ... \n"));

    PRINTF_DEBUG1(("SYSTEM CLOCK : %d 250000 usec = %d ticks\n",
        sysClkRateGet(),
        (250000 + (1000000 / sysClkRateGet()) - 1) /
        (1000000 / sysClkRateGet())));

    /*
     * Create the packet queue here to make sure the queue is ready
     * before any RX interrupt could happen
     */
    rx_queue_setup(RXQ_MAX);

    _systemInit();

    sal_usleep(1000 * MILLISECOND_USEC);

    PRINTF_DEBUG0(("System Init Done ...\n"));

    return 0;
}

static int
_probe_device()
{
    vxbde_dev_t *vxd;
    int index = 0;
    uint32          dev;

    dev = ICS_DEV_REV_ID_REG;
    if (SOC_IS_RAPTOR(dev)) {
        index = soc_chip_type_to_index(SOC_CHIP_BCM56218_A0);
        PRINTF_DEBUG(("BCM56218_A0\n"));
    } 
    else {
        return 0;
    }
    SOC_DRIVER(_n_devices) = soc_base_driver_table[index];

    /* store devices into structure */
    vxd = _devices + _n_devices++;
    vxd->bde_dev.device       = BCM56218_DEVICE_ID;
    vxd->bde_dev.rev          = SOC_CHIP_BCM56218_A0;
    vxd->bde_dev.base_address = ICS_CMIC_BASE_ADDR;

    return 0;
}

#ifndef POLLING_MODE
/***********************************************************/
/************************ connect interrupt ****************/
/***********************************************************/
typedef void (*pci_isr_t)(void *isr_data);

/*
 * pci_int_connect
 *
 *   Adds an interrupt service routine to the ISR chain for a
 *   specified interrupt line.
 */
static int
pci_int_connect(int intLine, pci_isr_t isr, void *isr_data)
{
    extern int sysVectorIRQ0;

    PRINTF_DEBUG(("pci_int_connect: intLine=%d, isr=%p, isr_data=%p %08x\n",
        intLine, isr, isr_data,
        (int) INUM_TO_IVEC(sysVectorIRQ0 + intLine)));

    /*0x00000002 0x00363274*/
    if (intConnect ((VOIDFUNCPTR *)
        INUM_TO_IVEC(sysVectorIRQ0 + intLine),
        (VOIDFUNCPTR) isr,
    (uint32) (isr_data)) != OK) {
        return -1;
    }

#if (CPU_FAMILY == PPC)
    if (intEnable(intLine) != OK) {
        return -1;
    }
    #endif

    return 0;
}
#endif  /* POLLING_MODE */

int soc_dma_abort( int unit);
int soc_dma_attach(int unit, int reset);

static void
soc_endian_config_pci_burst_enable(int unit)
{
    uint32 reg;

    /* set endian mode */
    soc_pci_write(unit, CMIC_ENDIAN_SELECT,
        (_bus.be_pio ? ES_BIG_ENDIAN_PIO : 0) |
        (_bus.be_packet ? ES_BIG_ENDIAN_DMA_PACKET : 0) |
        (_bus.be_other  ? ES_BIG_ENDIAN_DMA_OTHER : 0));

    /* enable PCI burst */
    /* Make sure these reads/writes are not combined */
    sal_usleep(1000);

    /* Enable Read/Write bursting in the CMIC */
    reg = soc_pci_read(unit, CMIC_CONFIG);
    reg |= (CC_RD_BRST_EN | CC_WR_BRST_EN);
    soc_pci_write(unit, CMIC_CONFIG, reg);

    /* Make sure our previous write is not combined */
    sal_usleep(1000);
}


int soc_firebolt_misc_init(int unit);
int soc_firebolt_mmu_init(int unit);

void pci_print_all(void);
static int _initUnit(int unit);

static void
_systemInit()
{
    int dev;

    _bus.base_addr_start = 0;

    _bus.int_line = 0;
    _bus.be_pio = 0;
#ifdef IL_BIGENDIAN
    _bus.be_packet = 1;
#else
    _bus.be_packet = 0;
#endif
    _bus.be_other = 0;

    _probe_device();

    PRINTF_DEBUG(("ICS Probe done, # of XGS devices = %d.\n", _n_devices));

    for (dev = 0; dev < _n_devices; dev++) {
        PRINTF_DEBUG(("======== INIT UNIT %d========\n", dev));
        _initUnit(dev);
    }
}


#ifndef POLLING_MODE
/********************************************************************/
/*********************** interrupt handler **************************/
/********************************************************************/

/*
 * SOC Interrupt Service Routine
 *
 *   In PLI simulation, the intr thread can call this routine at any
 *   time.  The connection is protected at the level of pli_{set/get}reg.
 */

#define POLL_LIMIT 100000

void soc_dma_done_chain(int unit, uint32 chan);

static void
soc_intr(void *_unit)
{
    uint32      irqStat, irqMask, i;
    int         unit = (uint32) (_unit);

    globalCount++;

    for( ; ;) {

        irqStat = soc_pci_read(unit, CMIC_IRQ_STAT);
        irqMask = soc_pci_read(unit, CMIC_IRQ_MASK);

        irqStat &= irqMask;

        if ( irqStat == 0)
            break;

        for( i = 0x80000000; i; i = i >> 1) {
            switch ( i & irqStat) {
                case 0x400: /* rx channel chain done */
                    soc_dma_done_chain(unit, 1);
                    break;

                case 0x100: /* tx channel chain done */
                    soc_dma_done_chain(unit, 0);
                    break;

                case 0x10: /* hw linkscan */
                    goto FATAL_ERROR;
                    break;

                case 0:
                    break;

                default:
                    goto FATAL_ERROR;
            }
        }
    }

    return;

FATAL_ERROR:
    soc_pci_write(unit, CMIC_IRQ_MASK, 0);

    /* soc_intr_disable(unit, i); */

    /* PRINTF_ERROR(("ERROR INTR, SYSTEM FAILURE 0x%08x NO HANDLER YET:\n", i)); */

    return;
}
#endif  /* POLLING_MODE */

/************************************************************/
/*************************** Utility Functions **************/
/************************************************************/
#ifdef  SOC_PCI_DEBUG
#define SOC_E_NONE 0
extern int PCI_DEBUG_ON;

/*
 * Get a CMIC register in PCI space using more "soc-like" semantics.
 * Input address is relative to the base of CMIC registers.
 */
int
soc_pci_getreg(int unit, uint32 addr, uint32 *datap)
{
    *datap = CMREAD(unit, addr);
    PRINTF_PCI2(("PCI%d memR(0x%x)=0x%x\n", unit, addr, *datap));
    return SOC_E_NONE;
}


/*
 * Get a CMIC register in PCI space.
 * Input address is relative to the base of CMIC registers.
 */
uint32
soc_pci_read(int unit, uint32 addr)
{
    uint32 data = CMREAD(unit, addr);
    PRINTF_PCI2(("PCI%d memR(0x%x)=0x%x\n", unit, addr, data));
    return data;
}


/*
 * Set a CMIC register in PCI space.
 * Input address is relative to the base of CMIC registers.
 */
int
soc_pci_write(int unit, uint32 addr, uint32 data)
{
    PRINTF_PCI2(("PCI%d memW(0x%x)=0x%x\n", unit, addr, data));
    CMWRITE(unit, addr, data);
    return 0;
}
#endif  /* SOC_PCI_DEBUG */

static void
soc_reset_bcm56218_a0(int unit)
{
    uint32              val;
    val = 0;

    /* Reset all blocks */
    WRITE_CMIC_SOFT_RESET_REGr(unit, val);

    /* bring the blocks out of reset */
    val = 0x23c;
    WRITE_CMIC_SOFT_RESET_REGr(unit, val);

    sal_usleep( 250 * MILLISECOND_USEC);

    val = 0x3ff;
    WRITE_CMIC_SOFT_RESET_REGr(unit, val);

    sal_usleep( 250 * MILLISECOND_USEC);

    val = 0x0ad42aaa;
    WRITE_CMIC_SBUS_RING_MAPr(unit, val);
}


/*
 * Reset the unit via the CMIC CPS reset function
 */
int
soc_reset(int unit)
{
    /* disable all interrupt */
    soc_intr_disable( unit, ~0);

    /*
     * Configure endian mode in case the system just came out of reset.
     * Configure bursting in case the system just came out of reset.
     */
    soc_endian_config_pci_burst_enable( unit);

    /*
     * After setting the reset bit, StrataSwitch PCI registers cannot be
     * accessed for 300 cycles or the CMIC will hang.  This is mostly of
     * concern on the Quickturn simulation but we leave plenty of time.
     */

    soc_pci_write(unit, CMIC_CONFIG, soc_pci_read(unit, CMIC_CONFIG) | CC_RESET_CPS);

    sal_usleep( 250 * MILLISECOND_USEC);

    soc_endian_config_pci_burst_enable( unit);

    soc_reset_bcm56218_a0(unit);
    /*
     * For H/W linkscan, disable all H/W linkscan
     */
    WRITE_CMIC_MIIM_PORT_TYPE_MAPr(unit, 0x0);

    return 0;
}


/*
 * soc_pci_test checks PCI memory range 0x00-0x4f
 */

int
soc_pci_test(int unit)
{
    uint32 i;
    uint32 tmp, reread;
    uint32 pat;

    for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
        pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
        soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), pat);
    }

    for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
        pat = 0x55555555 ^ (i << 24 | i << 16 | i << 8 | i);
        tmp = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
        if (tmp != pat) {
            goto error;
        }
    }

    /* Rotate walking zero/one pattern through each register */

    pat = 0xff7f0080;     /* Simultaneous walking 0 and 1 */

    for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
        int j;

        for (j = 0; j < 32; j++) {
            soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), pat);
            tmp = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
            if (tmp != pat) {
                goto error;
            }
            pat = (pat << 1) | ((pat >> 31) & 1); /* Rotate left */
        }
    }

    /* Clear to zeroes when done */

    for (i = 0; i < CMIC_SCHAN_WORDS(unit); i++) {
        soc_pci_write(unit, CMIC_SCHAN_MESSAGE(unit, i), 0);
    }

    PRINTF_DEBUG(("SCHAN_BUFFER write test succeed\n"));
    return 0;

    error:
    reread = soc_pci_read(unit, CMIC_SCHAN_MESSAGE(unit, i));
    PRINTF_ERROR(("FATAL PCI error testing PCIM[0x%x]:\n"
        "Wrote 0x%x, read 0x%x, re-read 0x%x\n",
        i, pat, tmp, reread));

    return -1;
}


#define SOC_IRQ_MASK(u)  (_devicesIntrMasks[(u)])

uint32
soc_intr_enable(int unit, uint32 mask)
{
    uint32 oldMask;

    oldMask = SOC_IRQ_MASK(unit);
#ifndef POLLING_MODE
    SOC_IRQ_MASK(unit) |= mask;
    soc_pci_write(unit, CMIC_IRQ_MASK, SOC_IRQ_MASK(unit));
#endif

    return oldMask;
}


uint32
soc_intr_disable(int unit, uint32 mask)
{
    uint32 oldMask;

    oldMask = SOC_IRQ_MASK(unit);
#ifndef POLLING_MODE
    SOC_IRQ_MASK(unit) &= ~mask;
    soc_pci_write(unit, CMIC_IRQ_MASK, SOC_IRQ_MASK(unit));
#endif

    return oldMask;
}


int
soc_firebolt_misc_init(int unit)
{
    uint32 rval;
    int32                 port;
    /*
    uint32              rval;
    uint32              misc_cfg;
    */

#if !defined(BCM_56218_A0)
    /* PARITY_EN & PARITY_IRQ_EN */
    if (SOC_DRIVER(unit)->type != SOC_CHIP_BCM56504_A0) {
        SOC_IF_ERROR_RETURN
            (WRITE_L2_ENTRY_PARITY_CONTROLr(unit, 0x0));
        SOC_IF_ERROR_RETURN
            (WRITE_L2_ENTRY_PARITY_CONTROLr(unit, 0x3));
        SOC_IF_ERROR_RETURN
            (WRITE_L3_ENTRY_PARITY_CONTROLr(unit, 0x0));
        SOC_IF_ERROR_RETURN
            (WRITE_L3_ENTRY_PARITY_CONTROLr(unit, 0x3));
    }
#endif

    /* clear IPIPE/EIPIE memories */
    SOC_IF_ERROR_RETURN
        (WRITE_ING_HW_RESET_CONTROL_1r(unit, 0x0));

    SOC_IF_ERROR_RETURN
        (WRITE_ING_HW_RESET_CONTROL_2r(unit, 0x00034000));

    SOC_IF_ERROR_RETURN
        (WRITE_EGR_HW_RESET_CONTROL_0r(unit, 0x0));

    SOC_IF_ERROR_RETURN
        (WRITE_EGR_HW_RESET_CONTROL_1r(unit, 0x00032000));

    {
        soc_timeout_t to;

        soc_timeout_init(&to, 50000, 0);
        do {
            if ( soc_timeout_check( &to)) {
                PRINTF_ERROR(("unit %d : ING_HW_RESET timeout\n", unit));
                break;
            }
            SOC_IF_ERROR_RETURN(READ_ING_HW_RESET_CONTROL_2r(unit, &rval));

            #define HW_ING_HW_RESET_CONTROL_2_DONE  0x40000
        } while ( (rval & HW_ING_HW_RESET_CONTROL_2_DONE) !=
            HW_ING_HW_RESET_CONTROL_2_DONE);

        soc_timeout_init(&to, 50000, 0);
        do {
            if ( soc_timeout_check( &to)) {
                PRINTF_ERROR(("unit %d : EGR_HW_RESET timeout\n", unit));
                break;
            }
            SOC_IF_ERROR_RETURN(READ_EGR_HW_RESET_CONTROL_1r(unit, &rval));
            #define HW_EGR_HW_RESET_CONTROL_1_DONE  0x40000
        } while ( (rval & HW_EGR_HW_RESET_CONTROL_1_DONE) !=
            HW_EGR_HW_RESET_CONTROL_1_DONE);
    }

    SOC_IF_ERROR_RETURN
        (WRITE_ING_HW_RESET_CONTROL_2r(unit, 0));
    SOC_IF_ERROR_RETURN
        (WRITE_EGR_HW_RESET_CONTROL_1r(unit, 0));

    /* Enable MMU parity error interrupt */
    SOC_IF_ERROR_RETURN( WRITE_MEMFAILINTSTATUSr( unit, 0x0));
    SOC_IF_ERROR_RETURN( WRITE_MEMFAILINTMASKr( unit, 0x1FF));
    SOC_IF_ERROR_RETURN( WRITE_MISCCONFIGr( unit, 0x605));

    SOC_IF_ERROR_RETURN(READ_MEMFAILINTSTATUSr(unit, &rval));
    soc_intr_enable(unit, IRQ_MEM_FAIL);

    /* enable egress */
    HE_PBMP_ALL_ALL_ITER(unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_EGR_ENABLEr(unit, port, 0x1));
        /* printf("==> %2d %08x\n", port, (((port)<<12) | (0xA900100)));*/
    }

    /* link status, CPU port is on */
    SOC_IF_ERROR_RETURN(WRITE_EPC_LINK_BMAPr(unit, 0x00000001));
    SOC_IF_ERROR_RETURN(WRITE_EPC_LINK_BMAP_HIr(unit, 0x00000000));

    HE_PBMP_GE_ITER( unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_GPORT_CONFIGr( unit, port, 0x3));
        /* PRINTF_DEBUG(("==> GPORT CONFIG %2d %08x\n",
                             port, ((((port)/12)<<20) | 0x80000)));*/
    }

    HE_PBMP_GE_ITER( unit, port) {
        SOC_IF_ERROR_RETURN(WRITE_GPORT_CONFIGr( unit, port, 0x1));
    }

    /* L3SRC_HIT_ENABLE, L2DST_HIT_ENABLE, we should disable L3, L2 is enough */
    SOC_IF_ERROR_RETURN(WRITE_ING_CONFIGr( unit, 0x0060000a));
    SOC_IF_ERROR_RETURN(WRITE_EGR_CONFIG_1r( unit, 0x1));

    /* Adjust the MDC output frequency to ~2.6MHz */
    soc_pci_write(unit, 0x1b8, ((1<<16) | 27));
    soc_pci_write(unit, 0x1bC, ((1<<16) | 27));

    return 0;
}


int
soc_firebolt_mmu_init(int unit)
{
    SOC_IF_ERROR_RETURN( WRITE_CFAPCONFIGr( unit, 0xFFF));
    SOC_IF_ERROR_RETURN( WRITE_CFAPFULLTHRESHOLDr( unit, 0x03e00fc0));

    SOC_IF_ERROR_RETURN( WRITE_PKTAGINGTIMERr( unit, 0x0));
    SOC_IF_ERROR_RETURN( WRITE_PKTAGINGLIMITr( unit, 0x0));

    /* enable CPU + GE ports */
    SOC_IF_ERROR_RETURN( WRITE_MMUPORTENABLEr( unit, 0xFFFFFFff));
    SOC_IF_ERROR_RETURN( WRITE_MMUPORTENABLE_HIr( unit, 0x003fffff));

    return 0;
}


/*
 * Timer functions
 */
void
soc_timeout_init(soc_timeout_t *to, sal_usecs_t usec, int min_polls)
{
    to->min_polls = min_polls;
    to->usec = usec;
    to->polls = 1;
    to->exp_delay = 1;    /* In case caller sets min_polls < 0 */
}


int
_sal_usec_to_ticks( uint32 usec)
{
    int     divisor;
    divisor = SECOND_USEC / sysClkRateGet();
    usec = (usec + divisor - 1) / divisor;
    return usec;
}


void
sal_usleep(uint32 usec)
{
    int     divisor;
    divisor = SECOND_USEC / sysClkRateGet();
    usec = (usec + divisor - 1) / divisor;
    taskDelay(usec);
}

sal_usecs_t
sal_time_usecs(void)
{
    struct timespec ltv;

    clock_gettime(CLOCK_REALTIME, &ltv);

    return (ltv.tv_sec * 1000000 + ltv.tv_nsec / 1000);
}


int
soc_timeout_check(soc_timeout_t *to)
{
    if (++to->polls >= to->min_polls) {
        if (to->min_polls >= 0) {
            /*
             * Just exceeded min_polls; calculate expiration time by
             * consulting O/S real time clock.
             */

            to->min_polls = -1;
            to->expire = SAL_USECS_ADD(sal_time_usecs(), to->usec);
            to->exp_delay = 1;
        } else {
            /*
             * Exceeded min_polls in a previous call.
             * Consult O/S real time clock to check for expiration.
             */

            if (SAL_USECS_SUB(sal_time_usecs(), to->expire) >= 0) {
                return 1;
            }

            sal_usleep(to->exp_delay);

            /* Exponential backoff with 10% maximum latency */

            if ((to->exp_delay *= 2) > to->usec / 10) {
                to->exp_delay = to->usec / 10;
            }
        }
    }

    return 0;
}


sal_usecs_t
soc_timeout_elapsed(soc_timeout_t *to)
{
    sal_usecs_t     start_time;

    start_time = SAL_USECS_SUB(to->expire, to->usec);

    return SAL_USECS_SUB(sal_time_usecs(), start_time);
}


/*********************************************************
 * Init functions
 *********************************************************/

int
bcm_fb_port_cfg_init(int unit, bcm_port_t port)
{
    port_tab_entry_t ptab;

    SOC_IF_ERROR_RETURN(WRITE_EGR_PORTr(unit, port, 0x4));

    SOC_IF_ERROR_RETURN
        (soc_mem_read(unit, PORT_TABm, MEM_BLOCK_ANY, port, &ptab));

    soc_mem_field32_set(unit, PORT_TABm, &ptab, PORT_VIDf, 1);
    soc_mem_field32_set(unit, PORT_TABm, &ptab, FILTER_ENABLEf, 0);
    soc_mem_field32_set(unit, PORT_TABm, &ptab, MY_MODIDf, unit);
    soc_mem_field32_set(unit, PORT_TABm, &ptab, OUTER_TPIDf, 0x8100);
    soc_mem_field32_set(unit, PORT_TABm, &ptab, MAC_BASED_VID_ENABLEf, 1);
    soc_mem_field32_set(unit, PORT_TABm, &ptab, SUBNET_BASED_VID_ENABLEf, 1);

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, PORT_TABm, MEM_BLOCK_ALL, port, &ptab));

    #define IS_CPU_PORT(u,p) ((p) == 0)

    if (IS_CPU_PORT(unit,port)) {
        SOC_IF_ERROR_RETURN(WRITE_IEGR_PORTr(unit, port, 0x1));
    }

    return BCM_E_NONE;
}


#define SOC_PORT_IF_GMII  0x3
int mac_ge_interface_set(int unit, soc_port_t port, soc_port_if_t pif);
int phy_5464_interface_set(int unit, soc_port_t port, soc_port_if_t pif);
int mac_ge_enable_set(int unit, soc_port_t port, int enable);
int phy_5464_enable_set(int unit, soc_port_t port, int enable);

static int
_bcm_port_mode_setup(int unit, bcm_port_t port, int enable)
{
    soc_port_if_t       pif;

    pif = SOC_PORT_IF_GMII;

    SOC_IF_ERROR_RETURN
        (phy_5464_interface_set(unit, port, pif));
    SOC_IF_ERROR_RETURN
        (mac_ge_interface_set(unit, port, pif));

    SOC_IF_ERROR_RETURN
        (mac_ge_enable_set(unit,port,TRUE));

    return BCM_E_NONE;
}


int
bcm_port_enable_set(int unit, bcm_port_t port, int enable)
{
    if (enable) { /* mac / phy */
        SOC_IF_ERROR_RETURN(mac_ge_enable_set(unit,port,enable));
        SOC_IF_ERROR_RETURN(phy_5464_enable_set(unit,port,enable));
    } else { /* reverse */
        SOC_IF_ERROR_RETURN(phy_5464_enable_set(unit,port,enable));
        SOC_IF_ERROR_RETURN(mac_ge_enable_set(unit,port,enable));
    }

    PRINTF_DEBUG2(("bcm_port_enable_set: u=%d p=%d enable=%d\n",
        unit, port, enable));

    return BCM_E_NONE;
}


int phy_5464_init(int unit, soc_port_t port);
int mac_56304_ge_init(int unit, soc_port_t port);

extern int
soc_miim_write(int unit, uint8 phy_id, uint8 phy_reg_addr, uint16 phy_wr_data);
extern int
soc_miim_read(int unit, uint8 phy_id, uint8 phy_reg_addr, uint16 *phy_rd_data);

int mii_write(int unit, int phy_addr, int reg, uint16 val)
{
    soc_miim_write(unit, phy_addr, reg, val);
    return 0;
}

static uint32_t PORT_TO_PHY_ADDR(int u, int p)
{
#if defined(BCM56218) || defined(BCM56018)
    if (p == 4) /* MDIO address are reversed for port 4 and 5 */
        return 6;
    if (p == 5)
        return 5;
    return (p + ((p > 29) ? (0x40 - 29) : (1)));

#elif defined(BCM56218P48)

    switch (p) {
        case 3: return 6;
        case 4: return 4;
        case 5: return 5;
        default:
            return (p + ((p > 29) ? (0x40 - 29) : (1)));
    }

#elif defined(BCM56214)
    return (p + ((p > 29) ? (0x40 - 29) : (1)));
#else
    return (p + ((p > 29) ? (0x40 - 29) : (1)));
#endif
}


#define PORT_TO_PHY_ADDR_INT(u,p)  (p + ((p > 29) ? (0xc0 - 30) : (0x80)))

uint16 mii_read(int unit, int phy_addr, int reg)
{
    uint16 val;

    soc_miim_read(unit, phy_addr, reg, &val);
    return val;
}

int _init_mac(int unit, int port, int speed)
{
    int fd = 1;
    uint32  config = 0x20, rval;

    if (speed == ETHER_SPEED_UNKNOWN) return -1;

    if ((speed == ETHER_SPEED_1000FDX) ||
        (speed == ETHER_SPEED_100FDX) ||
        (speed == ETHER_SPEED_10FDX)) {
        fd = 1;
    } else { fd = 0; }

    /* reset MAC */
    READ_GMACC0r(unit, port, &rval);
    WRITE_GMACC0r(unit, port, (rval | 0x80000000));
    WRITE_GMACC0r(unit, port, (rval & 0x7fffffff));

    SOC_IF_ERROR_RETURN(WRITE_GPCSCr(unit, port, 0x4));
    if ((speed == ETHER_SPEED_1000FDX) ||
        (speed == ETHER_SPEED_1000HDX)) {
        PRINTF_DEBUG2(("GIG init ..port %d\n", port));
        /* disable FE mac */
        SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, port, 0));
        
        config |= (0x00 << 1);
        SOC_IF_ERROR_RETURN(WRITE_GE_PORT_CONFIGr(unit, port, config));
    
        if (fd == 0) {
            printf("Gig mode half duplex not supported\n");
        }
        SOC_IF_ERROR_RETURN(WRITE_GMACC2r(unit, port, 0xC));
        SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, 0x550a1681));
        SOC_IF_ERROR_RETURN(WRITE_MAXFRr(unit, port, 0x5ee));
        sal_usleep(10);
        SOC_IF_ERROR_RETURN(WRITE_GMACC0r(unit, port, 1));
        /* GE MAC init */
    } else 
    if ((speed == ETHER_SPEED_100FDX) ||
        (speed == ETHER_SPEED_100HDX)) {
        PRINTF_DEBUG2(("100MB init ..port %d\n", port));
        /* disable GE mac */
        SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, 0x000a1681));
        SOC_IF_ERROR_RETURN(WRITE_FE_SUPPr(unit, port, 0x1100));

        /* 100 MB MAC init */
        config |= (0x01 << 1);
        SOC_IF_ERROR_RETURN(WRITE_GE_PORT_CONFIGr(unit, port, config));
        SOC_IF_ERROR_RETURN(WRITE_FE_MAC2r(unit, port, 
                                           ((fd) ? 0x4001 : 0x4000)));
        SOC_IF_ERROR_RETURN(WRITE_FE_IPGTr(unit, port, 0x15));
        SOC_IF_ERROR_RETURN(WRITE_FE_IPGRr(unit, port, 0x60f));
        SOC_IF_ERROR_RETURN(WRITE_FE_MAXFr(unit, port, 0x5ef));
        SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, port, 0x800d));
    } else 
    if ((speed == ETHER_SPEED_10FDX) ||
        (speed == ETHER_SPEED_10HDX)) {
        PRINTF_DEBUG2(("10MB init ..port %d\n", port));
        /* disable GE mac */
        SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, port, 0x000a1681));
        SOC_IF_ERROR_RETURN(WRITE_FE_SUPPr(unit, port, 0x1100));
        /* 10 Mb init */
        config |= (0x02 << 1);
        SOC_IF_ERROR_RETURN(WRITE_GE_PORT_CONFIGr(unit, port, config));
        SOC_IF_ERROR_RETURN(WRITE_FE_MAC2r(unit, port, 
                                           ((fd) ? 0x4001 : 0x4000)));
        SOC_IF_ERROR_RETURN(WRITE_FE_IPGTr(unit, port, 0x15));
        SOC_IF_ERROR_RETURN(WRITE_FE_IPGRr(unit, port, 0x60f));
        SOC_IF_ERROR_RETURN(WRITE_FE_MAXFr(unit, port, 0x5ef));
        SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, port, 0x800d));
    }
    return 0;
}

int _reset_phy(int unit, int port)
{
    uint16    id2, id1, status, ctrl;
    int phy_addr = PORT_TO_PHY_ADDR(unit, port);
    int phy_addr_int = PORT_TO_PHY_ADDR_INT(unit, port);

    ctrl = mii_read(unit, phy_addr_int, 0x10);
    ctrl &= 0xfffc;
    mii_write(unit, phy_addr_int, 0x10, ctrl);

    id1 = mii_read(unit, phy_addr, MII_PHY_ID0_REG);
    id2 = mii_read(unit, phy_addr, MII_PHY_ID1_REG);
    if ((id1 != 0x0000 && id1 != 0xFFFF) ||
        (id2 != 0x0000 && id2 != 0xFFFF)) {
        if (id1 != id2) {
            PRINTF_DEBUG2(("(PORT %d phy_id 0x%02x ID1 0x%08x ID2 0x%08x)",
                        port, phy_addr, id1, id2));
            }
    } else {
        return -1;
    }

    /* reset internal phy */
    if (!IS_GMII_PORT(port)) {
        mii_write(unit, phy_addr_int, MII_BMCR, 0x8000);
    }

    /* reset phy */
    status = mii_read(unit, phy_addr, MII_BMSR);
    ctrl = mii_read(unit, phy_addr, MII_BMCR);
    mii_write(unit, phy_addr, MII_BMCR, 0x8000);

    if (!IS_GMII_PORT(port)) {
        status = mii_read(unit, phy_addr_int, MII_BMCR);
        mii_write(unit, phy_addr_int, MII_BMCR, 
                  (status & (~(1<<12))));
    }

    return 0;
}

static int _start_phy_autoneg(int unit, int port)
{
    uint16    ctrl;
    int phy_addr = PORT_TO_PHY_ADDR(unit, port);
    int phy_addr_int = PORT_TO_PHY_ADDR_INT(unit, port);

    if (!IS_GMII_PORT(port)) {
        ctrl = mii_read(unit, phy_addr_int, 0x10);
        ctrl &= 0xfffc;
        mii_write(unit, phy_addr_int, 0x10, ctrl);
    }

    mii_write(unit, phy_addr, MII_BMCR, 0x1140);
    sal_usleep(1);
    mii_write(unit, phy_addr, MII_BMCR, 0x1340);

    return 0;
}

int _do_init_phy(int unit, int port)
{
    uint16              status, linkspeed = -1, ctrl;
    int xremote, remote;
    int phy_addr = PORT_TO_PHY_ADDR(unit, port);
    int phy_addr_int = PORT_TO_PHY_ADDR_INT(unit, port);

    status = mii_read(unit, phy_addr, MII_BMSR);
    status = mii_read(unit, phy_addr, MII_BMSR);
    ctrl = mii_read(unit, phy_addr, MII_BMCR);

    PRINTF_DEBUG1(("Port-%d : ", port));
    if (((status & BMSR_ANCOMPLETE) == 0) || 
        ((status & BMSR_LINKSTAT) == 0)) {
        PRINTF_DEBUG1(("NC\n"));
        return ETHER_SPEED_UNKNOWN;
    }

    remote = mii_read(unit, phy_addr, MII_ANLPAR);


    if (status & BMSR_1000BT_XSR)
        xremote = mii_read(unit, phy_addr, MII_K1STSR);
    else
        xremote = 0;

    PRINTF_DEBUG0((" STATUS : 0x%04x remote %04x CTRL 0x%04x K1STATUS %04x ==> ", 
           status, remote, ctrl, xremote));

    if ((xremote & K1STSR_LP1KFD) != 0) {
        PRINTF_DEBUG1(("1000BaseT FDX\n"));
        linkspeed = ETHER_SPEED_1000FDX;
        if (IS_INT_PHY_PRESENT(port))
            mii_write(unit, phy_addr_int, MII_BMCR, 0x140);
    }
    else if ((xremote & K1STSR_LP1KHD) != 0) {
        PRINTF_DEBUG1(("1000BaseT HDX\n"));
        linkspeed = ETHER_SPEED_1000HDX;
        if (IS_INT_PHY_PRESENT(port))
            mii_write(unit, phy_addr_int, MII_BMCR, 0x040);
    }
    else if ((remote & ANLPAR_TXFD) != 0) {
        PRINTF_DEBUG1(("100BaseT FDX\n"));
        linkspeed = ETHER_SPEED_100FDX;	 

        if (IS_INT_PHY_PRESENT(port))
            mii_write(unit, phy_addr_int, MII_BMCR, 0x2100);
    }
    else if ((remote & ANLPAR_TXHD) != 0) {
        PRINTF_DEBUG1(("100BaseT HDX\n"));
        linkspeed = ETHER_SPEED_100HDX;	 
        if (IS_INT_PHY_PRESENT(port))
            mii_write(unit, phy_addr_int, MII_BMCR, 0x2000);
    }
    else if ((remote & ANLPAR_10FD) != 0) {
        PRINTF_DEBUG1(("10BaseT FDX\n"));
        linkspeed = ETHER_SPEED_10FDX;	 
        if (IS_INT_PHY_PRESENT(port))
            mii_write(unit, phy_addr_int, MII_BMCR, 0x100);
    }
    else if ((remote & ANLPAR_10HD) != 0) {
        PRINTF_DEBUG1(("10BaseT HDX\n"));
        linkspeed = ETHER_SPEED_10HDX;	 
        if (IS_INT_PHY_PRESENT(port))
            mii_write(unit, phy_addr_int, MII_BMCR, 0x000);
    }
    return linkspeed;
}

int
bcm_port_init(int unit)
{
    int             speed, phy_addr;
    bcm_port_t      p;
    uint16          rval;
    uint16          mode_ctrl, ctrl;

    PRINTF_DEBUG2(("bcm_port_init: unit %d\n", unit));

    assert(unit < BCM_MAX_NUM_UNITS);

    /*
     * Write port configuration tables to contain the Initial System
     * Configuration (see init.c).
     */
    HE_PBMP_ALL_ITER(unit, p) {
        BCM_IF_ERROR_RETURN(bcm_fb_port_cfg_init(unit, p));
    }

    /*
     * Clear egress port blocking table
     */
    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, MAC_BLOCKm, COPYNO_ALL, TRUE));
    HE_PBMP_GE_ITER( unit, p) {
        SOC_IF_ERROR_RETURN(WRITE_FE_MAC1r(unit, p, 0x8000));
        SOC_IF_ERROR_RETURN(WRITE_GMACC1r(unit, p, 0xa1681));
        _reset_phy(unit, p);
    }

    sal_usleep(SECOND_USEC);

    /*
     * Init Phy for GMII port.
     */
    HE_PBMP_GE_ITER( unit, p) {
        phy_addr = PORT_TO_PHY_ADDR(unit, p);
        if (IS_GMII_PORT(p)) {
            mii_write(unit, phy_addr, 0x1c, 0x7c00);
            rval = mii_read(unit, phy_addr, 0x1c);
            rval &= ~0x0007;
            rval |= 0x8000;
            mii_write(unit, phy_addr, 0x1c, rval);

            rval = mii_read(unit, phy_addr, MII_BMCR);
            mii_write(unit, phy_addr, MII_BMCR, (rval & ~BMCR_POWERDOWN));
        } else if (IS_GE_CAP(p)) {
            mii_write(unit, phy_addr, 0x1c, 0x7c00);
            mode_ctrl = mii_read(unit, phy_addr, 0x1c);
            mii_write(unit, phy_addr, 0x1c, mode_ctrl | 0x8001);
            ctrl = mii_read(unit, phy_addr, 0);
            mii_write(unit, phy_addr, 0x0, (ctrl & ~(0x1800)));
            mode_ctrl &= ~0xf;
            mode_ctrl |= 0xc;
            mii_write(unit, phy_addr, 0x1c, mode_ctrl | 0x8000);

#if defined(BCM56214)
            if ((p == 4)  || (p == 5)) {
                /* Disable auto-medium detect */
                mii_write(unit, phy_addr, 0x1c, 0x7800);
                mode_ctrl = mii_read(unit, phy_addr, 0x1c);
                mode_ctrl &= ~0x7;
                mii_write(unit, phy_addr, 0x1c, mode_ctrl | 0x8000);

                mii_write(unit, phy_addr, 0x4, 0x5e1);
            }
#endif
        }
    }
    sal_usleep(100);

    HE_PBMP_GE_ITER( unit, p) {
        _start_phy_autoneg(unit, p);
    }

    sal_usleep((SECOND_USEC*5));

    HE_PBMP_GE_ITER(unit, p) {
        speed = _do_init_phy(unit, p);
        _init_mac(unit, p, speed);

        SOC_IF_ERROR_RETURN(WRITE_UNKNOWN_UCAST_BLOCK_MASKr(unit, p, 0x00000001));
        SOC_IF_ERROR_RETURN(WRITE_UNKNOWN_MCAST_BLOCK_MASKr(unit, p, 0x00000001));
        SOC_IF_ERROR_RETURN(WRITE_BCAST_BLOCK_MASKr(unit, p, 0x00000001));

    }

    PRINTF_DEBUG(("DONE WITH INIT PORT DRIVERS\n"));

    WRITE_SYS_MAC_LIMITr(unit, 0x3fff);
    return BCM_E_NONE;
}


#define L2U_BPDU_COUNT                 6
#define bcm_l2_bpdu_count(u)           L2U_BPDU_COUNT

static int
_bcm_l2_bpdu_init(int unit)
{
#if 0
    l2_user_entry_entry_t lue;
    sal_mac_addr_t mac_addr = {0x01, 0x80, 0xc2, 0x00, 0x00, 0x00};
    sal_mac_addr_t mask = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint32 *ent;
    int i;

    sal_memset(&lue, 0, sizeof(lue));
    soc_mem_mac_addr_set(unit, L2_USER_ENTRYm, &lue, MAC_ADDRf, mac_addr);
    soc_mem_mac_addr_set(unit, L2_USER_ENTRYm, &lue, MASKf, mask);
    soc_mem_field32_set(unit, L2_USER_ENTRYm, &lue, VALIDf, 1);
    soc_mem_field32_set(unit, L2_USER_ENTRYm, &lue, VLAN_IDf, 1);
    soc_mem_field32_set(unit, L2_USER_ENTRYm, &lue, CPUf, 1);
    soc_mem_field32_set(unit, L2_USER_ENTRYm, &lue, BPDUf, 1);
    soc_mem_field32_set(unit, L2_USER_ENTRYm, &lue, MODULE_IDf, unit);

    ent = (uint32*)&lue;
    PRINTF_DEBUG(("ev[0]=%x, ev[1]=%x, ev[2]=%x, ev[3]=%x, ev[4]=%x\n",
        ent[0], ent[1], ent[2], ent[3], ent[4]));

    for (i = 0; i < bcm_l2_bpdu_count(unit); i++) {
        SOC_IF_ERROR_RETURN(
            soc_mem_write(unit, L2_USER_ENTRYm, MEM_BLOCK_ALL, i, &lue));
    }
#endif

    return BCM_E_NONE;
}


/* L2 Init */
int
bcm_l2_init(int unit)
{
    sal_mac_addr_t mac_addr;

    PRINTF_DEBUG(("Initializing L2 ...................\n"));

    BCM_IF_ERROR_RETURN(_bcm_l2_bpdu_init(unit));

    WRITE_MAC_LIMIT_ENABLEr(unit, 0);
    WRITE_MAC_LIMIT_CONFIGr(unit, 0x1fff);

    if (GetMacFromFlash(mac_addr) == ERROR) {
        printf("Error: unable to enter L2 entry (Invalid MAC Address)\n");
        return BCM_E_PARAM;
    }
    soc_l2_insert(unit, MEM_BLOCK_ALL, mac_addr, 7, 1, 1);

    return BCM_E_NONE;
}


#define STP_GROUP               1
#define STG_BITS_PER_PORT       2
#define STG_PORT_MASK           ((1 << STG_BITS_PER_PORT)-1)
#define STG_PORTS_PER_WORD      (16)
#define STG_WORD(port)          ((port) / STG_PORTS_PER_WORD)
#define STG_BITS_SHIFT(port) \
    (STG_BITS_PER_PORT * ((port) % STG_PORTS_PER_WORD))
#define STG_BITS_MASK(port)     (STG_PORT_MASK << (STG_BITS_SHIFT(port)))

int
bcm_stg_init(int unit)
{
    uint32      entry[SOC_MAX_MEM_WORDS];
    int port;

    PRINTF_DEBUG(("Initializing STG ...................\n"));
    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, STG_TABm, COPYNO_ALL, TRUE));
    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, EGR_VLAN_STGm, COPYNO_ALL, TRUE));
#if 1
    for (port = 0; port < 54; port++) {
        entry[STG_WORD(port)] &= ~(STG_BITS_MASK(port));
        entry[STG_WORD(port)] |= (0x3 << STG_BITS_SHIFT(port));
    }
#else
    entry[0] = 0xffffffff;
    entry[1] = 0xffffffff;
    entry[2] = 0xffffffff;
    entry[3] = 0xfff;
#endif

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, STG_TABm, MEM_BLOCK_ANY, STP_GROUP, entry));

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, EGR_VLAN_STGm, MEM_BLOCK_ANY, STP_GROUP, entry));

    return BCM_E_NONE;
}


/*
 * Initializa the VLAN
 */
int
bcm_vlan_init(int unit)
{
    vlan_tab_entry_t  vt;
    egr_vlan_entry_t  ev;
    uint32 *ent;

    PRINTF_DEBUG(("Initializing VLAN ...................\n"));

    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, VLAN_TABm, COPYNO_ALL, TRUE));
    SOC_IF_ERROR_RETURN(soc_mem_clear(unit, EGR_VLANm, COPYNO_ALL, TRUE));

    sal_memset(&vt, 0, sizeof(vlan_tab_entry_t));
    soc_mem_field32_set(unit, VLAN_TABm, &vt, PORT_BITMAP_LOf, 0xffffffff);
    soc_mem_field32_set(unit, VLAN_TABm, &vt, PORT_BITMAP_HIf, 0x3fffff);
    soc_mem_field32_set(unit, VLAN_TABm, &vt, STGf, 1);
    soc_mem_field32_set(unit, VLAN_TABm, &vt, PFMf, 1);
    soc_mem_field32_set(unit, VLAN_TABm, &vt, VALIDf, 1);

    ent = (uint32*)&vt;
    PRINTF_DEBUG(("VLAN 1 : vt[0] = %8x, vt[1] = %8x\n", ent[0], ent[1]));

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, VLAN_TABm, MEM_BLOCK_ANY, 1, &vt));

    sal_memset(&ev, 0, sizeof(egr_vlan_entry_t));
    soc_mem_field32_set(unit, EGR_VLANm, &ev, UT_BITMAP_LOf, 0xfffffff8);
    soc_mem_field32_set(unit, EGR_VLANm, &ev, UT_BITMAP_HIf, 0x3fffff);
    soc_mem_field32_set(unit, EGR_VLANm, &ev, VALIDf, 1);
    soc_mem_field32_set(unit, EGR_VLANm, &ev, STGf, 1);

    PRINTF_DEBUG(("EGR VLAN : ev[0] = 0x%x, ev[1] = 0x%x, ev[2] = 0x%x\n",
        ent[0], ent[1], ent[2]));

    SOC_IF_ERROR_RETURN
        (soc_mem_write(unit, EGR_VLANm, MEM_BLOCK_ALL, 1, &ev));

    return BCM_E_NONE;
}

int
bcm_cosq_init( int unit)
{
    SOC_IF_ERROR_RETURN(WRITE_MISCCONFIGr(unit, 0x605));
    return 0;
}


int phy_fe_ge_link_get(int unit, soc_port_t port, int *link);
int myLinkScan(int unit);
int bcm_rx_init(int unit);
int bcm_rx_start(int unit, void *cfg);

/* disable both units */
int
bcm_stop_activities(void)
{
    int i;

    for( i = 0; i < _n_devices; i++) {
        soc_intr_disable(i, 0xFFFFFFFF);
    }

    return 0;
}


static int
_initUnit(int unit)
{
    uint32 reg;

    soc_dma_manager_t      *soc =    GET_SOC_DMA_MANAGER(unit);

    SOC_IF_ERROR_RETURN( !(soc->schanMutex = sal_mutex_create("SCHAN")));
    SOC_IF_ERROR_RETURN( !(soc->miimMutex = sal_mutex_create("MIIM")));

    /*****************************/
    /** insert interrupt handler */
    /*****************************/

    /* disable interrupt */
    PRINTF_DEBUG(("Disable CMIC Interrupt : \n"));
    {
        int soc_pci_test(int unit);
        int soc_reset(int unit);

        soc_reset( unit);
        sal_usleep(50000);
        soc_pci_test( unit);
    }

    /*
     * Enable enhanced DMA modes:
     * Scatter/gather, reload, and unaligned transfers
     *
     * Enable read and write bursts.
     * Note: very fast CPUs (above ~500 MHz) may combine multiple
     * memory operations into bursts.  The CMIC will hang if burst
     *  operations are not enabled.
     */

    reg = soc_pci_read(unit, CMIC_CONFIG);

    reg |= (CC_SG_OPN_EN | CC_RLD_OPN_EN | CC_ALN_OPN_EN |
        CC_RD_BRST_EN | CC_WR_BRST_EN);
    reg |= CC_EXT_MDIO_MSTR_DIS;

    soc_pci_write( unit, CMIC_CONFIG, reg);

    /* initialize DMA */
    PRINTF_DEBUG(("ATTACH DMA .........\n"));
    if (soc_dma_attach(unit, 1) < 0) {
        PRINTF_DEBUG(("ATTACH DMA failed %d\n", unit));
    }

#ifdef POLLING_MODE
    /* disable all interrupts */
    soc_pci_write(unit, CMIC_IRQ_MASK, 0);
#else   /* Intr mode */
    /* connect interrupt */
    PRINTF_DEBUG(("pci_int_connect .........\n"));
    pci_int_connect(_devices[unit].iLine, soc_intr, (void*) unit);

    /* ENABLE INTERRUPT */
    soc_intr_enable( unit, IRQ_PCI_PARITY_ERR | IRQ_PCI_FATAL_ERR | IRQ_SCHAN_ERR);
    soc_pci_write( unit, CMIC_SCHAN_CTRL, SC_MSG_DONE_CLR);
    soc_pci_write( unit, CMIC_SCHAN_CTRL, SC_MIIM_OP_DONE_CLR);

    /* no HW interrupt, no need to enable this interrupt */
    soc_intr_enable( unit, IRQ_LINK_STAT_MOD);
#endif /* ifdef POLLING_MODE */

    PRINTF_DEBUG(("MISC_INIT ...........\n"));
    if ( soc_firebolt_misc_init( unit) != 0) {
        PRINTF_ERROR(("MISC_INIT FAIL\n"));
        sal_usleep(50000);
    }

    PRINTF_DEBUG(("MMU_INIT ...........\n"));
    if ( soc_firebolt_mmu_init( unit) != 0) {
        PRINTF_ERROR(("MMU_INIT FAIL\n"));
        sal_usleep(50000);
    }

    PRINTF_DEBUG(("PORT/L2/STG/VLAN/COSQ INIT ..................\n"));
    BCM_IF_ERROR_RETURN(bcm_port_init( unit));

    BCM_IF_ERROR_RETURN(bcm_vlan_init( unit));

    BCM_IF_ERROR_RETURN(bcm_stg_init( unit));

    BCM_IF_ERROR_RETURN(bcm_cosq_init( unit));

    BCM_IF_ERROR_RETURN(bcm_l2_init( unit));

    SOC_IF_ERROR_RETURN(WRITE_EPC_LINK_BMAPr(unit, 0xffffffff));
    SOC_IF_ERROR_RETURN(WRITE_EPC_LINK_BMAP_HIr(unit, 0x3fffff));

    BCM_IF_ERROR_RETURN(bcm_rx_init( unit));

    bcm_rx_start( unit, NULL);

    PRINTF_DEBUG(("Link Scan starts ..................\n"));
    myLinkScan( unit);

    {
        int initStatistic(int unit, int port);
        initStatistic(unit, 0);
    }

    return SOC_E_NONE;
}


int
initStatistic(int unit, int port)
{
    /*
        unsigned int l2_entry[3]; 
    unsigned int msg_hdr;
        unsigned char mac[6];
    */

    int i;
    int RDBGC_SELECT[] = {
        RDBGC0_SELECTr, RDBGC1_SELECTr, RDBGC2_SELECTr,
        RDBGC3_SELECTr, RDBGC4_SELECTr, RDBGC5_SELECTr,
        RDBGC6_SELECTr, RDBGC7_SELECTr, RDBGC8_SELECTr,
    };

    int TDBGC_SELECT[] = {
        TDBGC0_SELECTr, TDBGC1_SELECTr, TDBGC2_SELECTr,
        TDBGC3_SELECTr, TDBGC4_SELECTr, TDBGC5_SELECTr,
        TDBGC6_SELECTr, TDBGC7_SELECTr, TDBGC8_SELECTr,
    };

    for( i = 0; i < 8; i++) {
        SOC_IF_ERROR_RETURN
            (soc_reg32_write(unit,
                    soc_reg_addr(unit, RDBGC_SELECT[i], REG_PORT_ANY, 0),
                    0xF << (i*4)));
        SOC_IF_ERROR_RETURN
            (soc_reg32_write(unit,
                    soc_reg_addr(unit, TDBGC_SELECT[i], REG_PORT_ANY, 0),
                    0xF << (i*4)));
    }
    return 0;
}


int
dumpStatistic(int unit, int port)
{
    uint32 v;
    int i, s;
    int CTR[] = {
        GTPKTr, GTBYTr, GTBCAr, GRPKTr, GRBYTr, ING_EVENT_DEBUGr, EGR_EVENT_DEBUGr
                };
    char * CTR_name[] = {
        "GTPKT", "GTBYT", "GTBCA", "GRPKT", "GRBYT", "ING_EVENT_DEBUGr", "EGR_EVENT_DEBUGr"
                        };
    int mac[] = { GE_PORT_CONFIGr, BCAST_BLOCK_MASKr, UNKNOWN_UCAST_BLOCK_MASKr,
                  GMACC0r, GMACC1r, GMACC2r, GPCSCr, MAXFRr, FE_MAC1r, FE_MAC2r,
                  FE_IPGTr, FE_IPGRr, FE_MAXFr
                };
#if 0
    int RDBGC_SELECT[] = {
        RDBGC0r, RDBGC1r, RDBGC2r,
        RDBGC3r, RDBGC4r, RDBGC5r,
        RDBGC6r, RDBGC7r, RDBGC8r,
        RDISCr, RUCr, RPORTDr,
    };

    int TDBGC_SELECT[] = {
        TDBGC0r, TDBGC1r, TDBGC2r,
        TDBGC3r, TDBGC4r, TDBGC5r,
        TDBGC6r, TDBGC7r, TDBGC8r,
    };
#endif
    if ((port > 5) || (port == 3)) {
        for( i = 0; i < 13; i++) {
            SOC_IF_ERROR_RETURN
                (soc_reg32_read(unit, 
                                soc_reg_addr(unit, mac[i], port, 0), &v));
            printf("%d:reg-%d.port%d :: %08x\n", unit, mac[i], port, v);
        }
        s = 0;
    } else {
        s = 5;
    }
            
    for( i = s; i < 7; i++) {
        SOC_IF_ERROR_RETURN
            (soc_reg32_read(unit, 
                            soc_reg_addr(unit, CTR[i], port, 0), &v));
        printf("%d:%s.port%d :: %08x\n", unit, CTR_name[i], port, v);
    }
#if 0
    printf("PHY :\n");
    if (port != 0)
    for (i=0; i<16; i++) {
        v = PORT_TO_PHY_ADDR_INT(unit, port);
        piv = mii_read(unit, v, i);
        v = PORT_TO_PHY_ADDR(unit, port);
        pv = mii_read(unit, v, i);
        printf("PHY-reg%d (Int:ext): 0x%04x 0x%04x\n", i, piv, pv);
    }
    READ_EPC_LINK_BMAPr(unit, &v);
    printf("EPC_LINK_BMAPr : 0x%08x\n", v);
    printf("MMU registers\n");
    for (i=0; i<8; i++) {
        READ_COSLCCOUNTr(unit, port, i, &v);
        printf("%d.COSLCCOUNT.%d : 0x%08x\n", port, i, v);
    }

    for( i = 0; i < 9; i++) {
        SOC_IF_ERROR_RETURN
            (soc_reg32_read(unit, 
                    soc_reg_addr(unit, TDBGC_SELECT[i], REG_PORT_ANY, 0), &v));
        printf("%d:TDBGC%d :: %08x\n", unit, i, v);
    }
    for( i = 0; i < 12; i++) {
        SOC_IF_ERROR_RETURN
            (soc_reg32_read(unit, 
                    soc_reg_addr(unit, RDBGC_SELECT[i], REG_PORT_ANY, 0), &v));
        if (i < 9)
            printf("%d:RDBGC%d :: %08x\n", unit, i, v);
        else 
            printf("%d:Rctr[%d] :: %08x\n", unit, i, v);
    }
#endif
    return 0;
}
