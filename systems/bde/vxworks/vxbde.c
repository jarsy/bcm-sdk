/*
 * $Id: vxbde.c,v 1.156 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * VxWorks BDE
 */

#include "vxbde.h"

#include "vxworks_shbde.h"

#ifdef BCM_ROBO_SUPPORT
#if defined(KEYSTONE)
#include <osl.h>
#include <sbconfig.h>
#include <bcmutils.h>
#include <siutils.h>
#include <sbchipc.h>
#include <hndpci.h>
#include "../../drv/et/etc_robo_spi.h"
#include <ethernet.h>
#include <et_dbg.h>
#include <etc53xx.h>

#include <shared/et/osl.h>
#include <shared/et/sbconfig.h>
#include <shared/et/bcmdevs.h>
#endif
#endif

#if defined(KEYSTONE)
#include <hndsoc.h>
#endif /* KEYSTONE */

#include <sal/types.h>
#include <sal/appl/pci.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/core/boot.h>
#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <soc/devids.h>

#include <soc/defs.h>
#include <soc/types.h>

#include <sal/appl/io.h>
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
#include <soc/dpp/drv.h>
#include <soc/dpp/SAND/Utils/sand_framework.h>
#endif

#include "intLib.h"
#include <shared/util.h>
/* Structure of private spi device */
typedef struct spi_dev_s {
    uint8          cid;         /* Chip ID */
    uint32          part;        /* Part number of the chip */
    uint8          rev;         /* Revision of the chip */
    void           *robo;       /* ptr to robo info required to access SPI */
    unsigned short phyid_high;  /* PHYID HIGH in MII regs of detected chip */
    unsigned short phyid_low;   /* PHYID LOW in MII regs of detected chip */
} spi_dev_t;

struct spi_device_id {
    unsigned short phyid_high;  /* PHYID HIGH in MII regs of detected chip */
    unsigned short phyid_low;   /* PHYID LOW in MII regs of detected chip */
    uint32  model_info;
    uint32  rev_info;
    uint32  spifreq;
};

typedef struct vxbde_dev_s {
    /* Specify the type of device, pci, spi, switch, ether ... */
    uint32 dev_type;

    ibde_dev_t bde_dev;
    union {
        pci_dev_t  _pci_dev;    /* PCI device type */
        spi_dev_t  _spi_dev;    /* SPI device type */
    } dev;
#define pci_dev     dev._pci_dev
#define spi_dev     dev._spi_dev
    int iLine;
    int iPin;

#define BDE_FLAG_BUS_RD_16BIT        0x1
#define BDE_FLAG_BUS_WR_16BIT        0x10
    uint32  flags;
    uint32  cpu_address;

    /* Hardware abstraction for shared BDE functions */
    shbde_hal_t shbde;

} vxbde_dev_t;

#define MAX_SWITCH_DEVICES 16
#define MAX_ETHER_DEVICES 2
#define MAX_DEVICES (MAX_SWITCH_DEVICES + MAX_ETHER_DEVICES)
#define BCM47XX_ENET_ID     0x4713      /* 4710 enet */
#define BCM5300X_GMAC_ID     0x4715    

#define PL0_OFFSET  0x00800000
#define PL0_SIZE    0x00040000

#ifdef METROCORE
#define FPGA_IRQ    37
#else
#define FPGA_IRQ    48
#endif /* METROCORE */

static vxbde_dev_t _devices[MAX_DEVICES];
static int _n_devices = 0;
static int _switch_n_devices = 0;
static int _ether_n_devices = 0;
 
#if (!defined(BCM_ICS) && !defined(INTERNAL_CPU))
static int _pri_bus_no = -1;
static int _n_pri_devices = 0;
static int _n_sec_devices = 0;

/* Assume there's only one PLX PCI-to-Local bus bridge if any */
static vxbde_dev_t plx_vxd;
static int num_plx = 0;

#ifndef METROCORE
static int _first_mac = 0;
#endif /* METROCORE */
#endif /* !BCM_ICS && !INTERNAL_CPU */

#define VALID_DEVICE(_n) ((_n >= 0) && (_n < _n_devices))
#define DEVICE_INDEX(_n) ((_n < _switch_n_devices) ? _n : \
                  (MAX_SWITCH_DEVICES+_n-_switch_n_devices))


#ifdef BCM_ROBO_SUPPORT
#if defined(KEYSTONE)
static int _robo_devices = 0;
static void *robo = NULL;
static si_t *sih = NULL;
#endif /* KEYSTONE */
#endif

#if defined(KEYSTONE)
static int pcie1_device_exist = 0;
static uint32  coreflags = 0;
#ifndef PCIE_PORT0_HB_BUS
#define PCIE_PORT0_HB_BUS (1)
#endif
#ifndef PCIE_PORT1_HB_BUS
#define PCIE_PORT1_HB_BUS (17)
#endif
#ifndef SI_PCI0_MEM
#define SI_PCI0_MEM 0x08000000
#endif
#ifndef SI_PCI1_MEM
#define SI_PCI1_MEM 0x40000000
#endif
#endif

static vxbde_bus_t _bus;

uint32 bcm_bde_soc_cm_memory_base = 0;

#ifdef INCLUDE_RCPU
/*****************EB devices***************/

static uint32  _read(int d, uint32 offset);
static int _write(int d, uint32 offset, uint32 data);


int vxbde_eb_bus_probe(vxbde_bus_t *bus, ibde_t **bde)
{
    char                prop[64], *s;
    int                 unit = 0;
    uint32_t            baddr;

    for (unit = _n_devices; unit < MAX_SWITCH_DEVICES; unit++) {
        sal_sprintf(prop, "eb_dev_addr.%d", unit);

        if ((s = sal_config_get(prop)) && 
            (baddr = sal_ctoi(s, 0))) {
            vxbde_eb_create(bus, bde, baddr);
        }
    }

    return 0;
}
#endif /* INCLUDE_RCPU */

static const char *
_name(void)
{
#ifdef IPROC_CMICD
    return "vxworks-iproc-bde";
#else
    return "vxworks-pci-bde";
#endif
}

static int
_num_devices(int type)
{
    switch (type) {
        case BDE_ALL_DEVICES:
            return _n_devices;
        case BDE_SWITCH_DEVICES:
            return _switch_n_devices; 
        case BDE_ETHER_DEVICES:
            return _ether_n_devices; 
    }

    return 0;
}

static const ibde_dev_t *
_get_dev(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) {
    return NULL;
    }
    _d = DEVICE_INDEX(d);

    return &_devices[_d].bde_dev;
}

static uint32
_get_dev_type(int d)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return  0;
    }
    _d = DEVICE_INDEX(d);

    return _devices[_d].dev_type;
}

static uint32 
_pci_read(int d, uint32 addr)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return (uint32)0xFFFFFFFF;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_PCI_DEV_TYPE)) {
        return (uint32)0xFFFFFFFF;
    }

    return pci_config_getw(&_devices[_d].pci_dev, addr);
}

static int
_pci_write(int d, uint32 addr, uint32 data)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_PCI_DEV_TYPE)) {
        return -1;
    }
    return pci_config_putw(&_devices[_d].pci_dev, addr, data);
}

static void
_pci_bus_features(int unit, int *be_pio, int *be_packet, int *be_other)
{
    *be_pio = _bus.be_pio;
    *be_packet = _bus.be_packet;
    *be_other = _bus.be_other;
}

#ifdef PCI_DECOUPLED
static uint32  
_read(int d, uint32 offset)
{
    int _d, s;
    uint16  msb, lsb;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (_devices[_d].dev_type & BDE_PCI_DEV_TYPE) {
        return sysPciRead(
            &(((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4]));
    }

    if (_devices[_d].dev_type & (BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE )) {
        if (_devices[_d].flags & BDE_FLAG_BUS_RD_16BIT) {
        s = intLock();
            offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
            lsb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
            msb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
        intUnlock(s);
            return (msb << 16) | lsb;
        } else {
            return ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4];
        }
    }
    return -1;
}

static int
_write(int d, uint32 offset, uint32 data)
{
    int _d, s;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (_devices[_d].dev_type & BDE_PCI_DEV_TYPE) {
        sysPciWrite(
            &(((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4]),
            data);
        return 0;
    }

    if (_devices[_d].dev_type & (BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE )) {
        if (_devices[_d].flags & BDE_FLAG_BUS_WR_16BIT) {
            s = intLock();
            offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
            *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) =
                (data & 0xffff);
            *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) = 
                ((data>>16) & 0xffff);
            intUnlock(s);
        } else {
            ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4] = data;
        }
        return 0;
    }

    return -1;
}
#else
static uint32  
_read(int d, uint32 offset)
{
    int _d, s;
    volatile uint16  msb, lsb;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[_d].flags & BDE_FLAG_BUS_RD_16BIT) {
        s = intLock();
        offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
        lsb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
        msb = *((uint16 *)(_devices[_d].bde_dev.base_address + offset));
        intUnlock(s);
        return (msb << 16) | lsb;
    } else {
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
        if ((SOC_IS_SAND(d))) {
            return SOC_SAND_BYTE_SWAP(((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4]);
        } else
#endif		
        {
            return ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4];
        }
    
    }
}

static int
_write(int d, uint32 offset, uint32 data)
{
    int _d, s;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[_d].flags & BDE_FLAG_BUS_WR_16BIT) {
        s = intLock();
        offset = (offset & 0xffff0000) | ((offset & 0xffff) << 1);
        *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) =
            (data & 0xffff);
        *((uint16 *)(_devices[_d].bde_dev.base_address + offset)) = 
            ((data>>16) & 0xffff);
    intUnlock(s);
    } else {
#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)	
        if ((SOC_IS_SAND(d))) {
            ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4] = SOC_SAND_BYTE_SWAP(data);
        } else
#endif
        {
             ((uint32 *)_devices[_d].bde_dev.base_address)[offset / 4] = data;
        }
		
    }
    return 0;
}
#endif

static uint32 * 
_salloc(int d, int size, const char *name)
{
    COMPILER_REFERENCE(d);

    return sal_dma_alloc(size, (char *)name);
}

static void
_sfree(int d, void *ptr)
{
    COMPILER_REFERENCE(d);

    sal_dma_free(ptr);
}

static int 
_sflush(int d, void *addr, int length)
{
    COMPILER_REFERENCE(d);

    sal_dma_flush(addr, length);

    return 0;
}

static int
_sinval(int d, void *addr, int length)
{
    COMPILER_REFERENCE(d);

    sal_dma_inval(addr, length);

    return 0;
}

#ifdef METROCORE
extern STATUS sysFPGAIntConnect(VOIDFUNCPTR isr, int arg);
#endif

static int 
_interrupt_connect(int d, void (*isr)(void *), void *data)
{
    int _d;
#ifdef METROCORE
    STATUS status;
    static int FPGAIntConnected = 0;
#endif

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & 
#if (defined(IPROC_CMICD) && defined(INTERNAL_CPU))
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE 
          | BDE_AXI_DEV_TYPE))) {
#else
          (BDE_PCI_DEV_TYPE | BDE_ICS_DEV_TYPE | BDE_EB_DEV_TYPE))) {
#endif
        return -1;
    }

#ifdef METROCORE
    if ((_devices[_d].iLine == FPGA_IRQ) && !FPGAIntConnected) {
        status = sysFPGAIntConnect(isr, (int)data);
    FPGAIntConnected = 1;
        return status;
    }
    /*
     * FPGA ISR can only be connected once.
     * One ISR will have to handle all the FPGA interrupts.
     */
    if ((_devices[_d].iLine == FPGA_IRQ) && FPGAIntConnected) {
    return OK;
    }
#endif

    return pci_int_connect(_devices[_d].iLine, isr, data);
}

static int
_interrupt_disconnect(int d)
{
    COMPILER_REFERENCE(d);
    return 0;
}

#ifdef BCM_ICS

#define SB_K1_TO_PHYS(x) ((uint32)x & 0x1fffffff)
#define SB_PHYS_TO_K1(x) ((uint32)x | 0xa0000000)

static uint32 
_l2p(int d, void *laddr)
{
    COMPILER_REFERENCE(d);

    return (uint32) SB_K1_TO_PHYS(laddr);
}

static uint32 *
_p2l(int d, uint32 paddr)
{
    COMPILER_REFERENCE(d);

    return (uint32 *) SB_PHYS_TO_K1(paddr);
}

#else /* !BCM_ICS */
   
#if defined(NSX) || defined(METROCORE)
#define SB_K0_TO_PHYS(x) ((uint32)x & 0x7fffffff)
#define SB_PHYS_TO_K0(x) ((uint32)x | 0x80000000)
#endif

#define SB_K1_TO_PHYS(x) ((uint32)x & 0x1fffffff)
#define SB_PHYS_TO_K1(x) ((uint32)x | 0xa0000000)

static uint32 
_l2p(int d, void *laddr)
{

#if defined(NSX) || defined(METROCORE)
    COMPILER_REFERENCE(d);
    return (uint32) SB_K0_TO_PHYS(laddr);
#else
    int _d;

    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);

#if defined(KEYSTONE)
    return (uint32) SB_K1_TO_PHYS(laddr);
#else
    if ((_devices[_d].dev_type & BDE_ETHER_DEV_TYPE)) {
        return (uint32) SB_K1_TO_PHYS(laddr);
    }else {
    return (uint32)laddr;
    }
#endif /* KEYSTONE */

#endif /* defined(NSX) || defined(METROCORE) */
}

static uint32 *
_p2l(int d, uint32 paddr)
{

#if defined(NSX) || defined(METROCOORE)
    COMPILER_REFERENCE(d);
    return (uint32 *) SB_PHYS_TO_K0(paddr);
#else
    int _d;

    if (!VALID_DEVICE(d)) {
        return NULL;
    }
    _d = DEVICE_INDEX(d);

#if defined(KEYSTONE)
    return (uint32 *) SB_PHYS_TO_K1(paddr);
#else
    if ((_devices[_d].dev_type & BDE_ETHER_DEV_TYPE)) {
        return (uint32 *) SB_PHYS_TO_K1(paddr);
    }else {
    return (uint32 *) paddr;
    }
#endif /* KEYSTONE */

#endif /* defined(NSX) || defined(METROCORE) */
}

#endif /* BCM_ICS */

#define IPROC_CMIC_WINSZ        0x40000 /*CMICm/CMICd is 256K */

static uint32 _iproc_read(int d, uint32 addr);
static int _iproc_write(int d, uint32 addr, uint32 data);


static uint32  
_iproc_read(int d, uint32 addr)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return -1;
    }

    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[d].dev_type & BDE_AXI_DEV_TYPE) {
        return ((uint32 *)_devices[_d].bde_dev.base_address1)[addr / 4];
    }

    return shbde_iproc_pci_read(&_devices[d].shbde,
                                (void *)_devices[d].bde_dev.base_address1,
                                addr);
}

static int
_iproc_write(int d, uint32 addr, uint32 data)
{
    int _d;

    if (!VALID_DEVICE(d)) {
        return -1;
    }

    _d = DEVICE_INDEX(d);

    if (!(BDE_DEV_MEM_MAPPED(_devices[_d].dev_type))) {
        return -1;
    }

    if (_devices[d].dev_type & BDE_AXI_DEV_TYPE) {
        ((uint32 *)_devices[_d].bde_dev.base_address1)[addr / 4] = data;
    }

    shbde_iproc_pci_write(&_devices[d].shbde,
                          (void *)_devices[d].bde_dev.base_address1,
                          addr, data);

    return 0;
}

#define ROBO_ADDR_PAGE(_a) (((_a) >> 8) & 0xff)
#define ROBO_ADDR_OFFSET(_a) ((_a) & 0xff)

static int
_spi_read(int d, uint32 addr, uint8 *buf, int len)
{
    int _d;
    
    if (!VALID_DEVICE(d)) {
        return -1;
    }

    _d = DEVICE_INDEX(d);
    
   
#ifdef BCM_ROBO_SUPPORT
#if defined(KEYSTONE)
    if (_devices[_d].dev_type & BDE_SPI_DEV_TYPE) {
        uint8 page, offset;

        page = ROBO_ADDR_PAGE(addr);
        offset = ROBO_ADDR_OFFSET(addr);
    
        robo_rreg(_devices[_d].spi_dev.robo, _devices[_d].spi_dev.cid,
                  page, offset, buf, (uint)len);
        return 0;
    }
#endif
#endif

    return -1;
}

static int
_spi_write(int d, uint32 addr, uint8 *buf, int len)
{
    int _d;
    
    if (!VALID_DEVICE(d)) {
        return -1;
    }
    _d = DEVICE_INDEX(d);
  
#ifdef BCM_ROBO_SUPPORT
#if defined(KEYSTONE)
    if (_devices[_d].dev_type & BDE_SPI_DEV_TYPE) {
        uint8 page, offset;

        page = ROBO_ADDR_PAGE(addr);
        offset = ROBO_ADDR_OFFSET(addr);

        robo_wreg(_devices[_d].spi_dev.robo, _devices[_d].spi_dev.cid,
                  page, offset, buf, (uint)len);
        return 0;
    }
#endif
#endif

    return -1;
}

#ifdef INCLUDE_APS_DIAG_LIBS

#if defined(KEYSTONE)
extern void robo_aps_spi_write(void *robo, uint8 * buf, int len);
extern void robo_aps_spi_read(void *robo, uint8 * buf, int len);
#endif

int
aps_spi_read(int d, uint8 *buf, int len)
{
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }
#if defined(KEYSTONE)
    robo_aps_spi_read(_devices[_d].spi_dev.robo, buf, (uint)len);
#endif
    return 0;
}

int
aps_spi_write(int d, uint8 *buf, int len)
{
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }
#if defined(KEYSTONE)
    robo_aps_spi_write(_devices[_d].spi_dev.robo, buf, (uint)len);
#endif
    return 0;
}

int
aps_spi_set_freq(int d, uint32 speed_hz)
{
    int _d;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    if (!(_devices[_d].dev_type & BDE_SPI_DEV_TYPE)) {
        return -1;
    }
#if defined(KEYSTONE)
    chipc_spi_set_freq(_devices[_d].spi_dev.robo, 0, speed_hz);
#endif
    return 0;
}
#endif /* INCLUDE_APS_DIAG_LIBS */


#ifdef BCM_PETRA_SUPPORT

int
_cpu_write(int d, uint32 addr, uint32 *buf)
{
    int     _d;
    uint32  *full_addr;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    full_addr = (uint32 *)(_devices[_d].cpu_address + addr);

    *full_addr = *buf;

    return 0;
}

int
_cpu_read(int d, uint32 addr, uint32 *buf)
{
    int     _d;
    uint32  *full_addr;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);

    full_addr   = (uint32 *)(_devices[_d].cpu_address + addr);

    *buf = *full_addr;
    return 0;
}

int
_cpu_pci_register(int d)
{
    int             _d;
    vxbde_dev_t     *vxd;
    uint32          val;

    if (!VALID_DEVICE(d)) return -1;
    _d = DEVICE_INDEX(d);
    vxd = &_devices[d];
    
    pciDevConfig(vxd->pci_dev.busNo, vxd->pci_dev.devNo, vxd->pci_dev.funcNo, 
                 0x0 /* IO address */, vxd->bde_dev.base_address, 
                 (PCI_CMD_MEM_ENABLE | PCI_CMD_MASTER_ENABLE));

    if (vxd->bde_dev.device == GEDI_DEVICE_ID) {
        /* Fix bar 0 address */
        val = pci_config_getw(&vxd->pci_dev, 0x10);
        val = (val & 0xffffff) | (0x60 << 24);
        pci_config_putw(&vxd->pci_dev, 0x10, val);

        /* Fix Max payload size */
        val = pci_config_getw(&vxd->pci_dev, 0x88);
        val = (val & 0xffff0000) | 0x102f;
        pci_config_putw(&vxd->pci_dev, 0x88, val);
    }
    return 0;
}
#endif /* BCM_PETRA_SUPPORT */

static ibde_t _ibde = {
    _name, 
    _num_devices, 
    _get_dev, 
    _get_dev_type, 
    _pci_read,
    _pci_write,
    _pci_bus_features,
    _read,
    _write,
    _salloc,
    _sfree,
    _sflush,
    _sinval,
    _interrupt_connect,
    _interrupt_disconnect,
    _l2p,
    _p2l,
    _spi_read,
    _spi_write,
    _iproc_read,
    _iproc_write,
};
#ifdef BCM_ROBO_SUPPORT
#if defined(KEYSTONE)
    
/*
* The model_info /rev_info for Robo devices is defined like this:
*
* 31 28 27     24 23  20 19 16 15      8 7      0
* +----+---------+------+-----+---------+--------+
* | op | reserved| mask |len  | page    |offset  |
* +----+---------+------+-----+---------+--------+
*
*
* op:          1:OR phyidl
* mlen:      mask len (in bytes) 1:means 0xf,2 means 0xff
* len:         Size of model/rev ID register (in bytes)
* page:      Page containing model ID and revision registers
* offset:     Model/rev ID register offset
*/
static struct spi_device_id _spi_id_table[] = {
    { BCM53242_PHYID_HIGH, BCM53242_PHYID_LOW , 0         , 0       , SPI_FREQ_DEFAULT},
    { BCM53262_PHYID_HIGH, BCM53262_PHYID_LOW , 0         , 0       , SPI_FREQ_DEFAULT},
    { BCM53115_PHYID_HIGH, BCM53115_PHYID_LOW , 0         , 0       , SPI_FREQ_DEFAULT},
    { BCM53118_PHYID_HIGH, BCM53118_PHYID_LOW , 0         , 0       , SPI_FREQ_DEFAULT},
    { BCM53280_PHYID_HIGH, BCM53280_PHYID_LOW , 0x101800e8, 0       , SPI_FREQ_20MHZ},
    { BCM53101_PHYID_HIGH, BCM53101_PHYID_LOW , 0         , 0x110240, SPI_FREQ_20MHZ},
    { BCM53125_PHYID_HIGH, BCM53125_PHYID_LOW , 0         , 0x110240, SPI_FREQ_20MHZ},
    { BCM53128_PHYID_HIGH, BCM53128_PHYID_LOW , 0         , 0x110240, SPI_FREQ_20MHZ},
    { BCM53600_PHYID_HIGH, BCM53600_PHYID_LOW , 0x101800e8, 0       , SPI_FREQ_20MHZ},
    { BCM89500_PHYID_HIGH, BCM89500_PHYID_LOW , 0x240230  , 0x110240, SPI_FREQ_20MHZ},
    { BCM53010_PHYID_HIGH, BCM53010_PHYID_LOW , 0x240230  , 0x110240, 0},
    { BCM53018_PHYID_HIGH, BCM53018_PHYID_LOW , 0x240230  , 0x110240, 0},
    { BCM5389_PHYID_HIGH , BCM5389_PHYID_LOW  , 0x110230  , 0x110240, SPI_FREQ_DEFAULT},
    { BCM5396_PHYID_HIGH , BCM5396_PHYID_LOW  , 0x110230  , 0x110240, SPI_FREQ_DEFAULT},
    { BCM53134_PHYID_HIGH, BCM53134_PHYID_LOW , 0         , 0x110240, SPI_FREQ_20MHZ},
    { 0, 0, 0, 0, 0 },
};
/* bustype 2: ROBO_MDCMDIO_BUS, 1: ROBO_SPI_BUS */
#define MAX_BUSTYPE 2

static int
_spi_device_valid_check(unsigned short phyidh,unsigned short phyidl, uint8 check_flag)
{
    struct spi_device_id *_ids;
    int idx, match_idx;

    match_idx = -1;
    idx = 0;

    if (check_flag == 0){
    /* check_flag == 0 check phyidh only*/
        for (_ids = _spi_id_table;
            _ids->phyid_high && _ids->phyid_low; _ids++){
            if (_ids->phyid_high == phyidh) {
                return 0;
            }
        }
        /* No valid SPI devices found */
        return 1;
    } else {        
        while(_spi_id_table[idx].phyid_high){
            if (phyidh == _spi_id_table[idx].phyid_high &&
                phyidl == _spi_id_table[idx].phyid_low) {
                /* Found a match */
                match_idx = idx;
                break;
            }
            idx++;
        }
        return match_idx;
    }
}


static int
_spi_attach(void)
{
    int dev;
    vxbde_dev_t *vxd;
    int max_devices, max_bustype;
    uint8   buf[8];
    unsigned short phyidh = 0, phyidl = 0;
    uint32 spi_freq = SPI_FREQ_20MHZ; /* The hightest supported SPI frequency, as of now 20MHz */
    uint32 spi_freq_override;         /* Used for user overrided SPI frequency */
    char *spi_freq_str = NULL;

    if (!(sih = si_kattach(SI_OSH))) {
        return -ENODEV;
    }

    /* Use ss=0 to operate on SPI interface */
    if (!(robo = (void *)robo_attach((void *)sih,0))){
        return -ENODEV;
    }
#ifdef ROBODVT
    else {
        /* For ROBO DVT, no need to probe the chips */
        return 0;
    }
#endif
    
    max_bustype = MAX_BUSTYPE + 1;

    while(_spi_device_valid_check(phyidh, 0, 0)) {
        max_bustype --;
        if(!max_bustype)
            return -ENODEV;
        robo_switch_bus(robo, max_bustype);
        buf[0] = buf[1] = 0;
        robo_rreg(robo, 0, 0x10, 0x04, buf, (uint)2);
        phyidh = buf[0] | (buf[1] << 8);
    }

    /* For psedo_phy access, only support one robo switch*/
    /* For Northstar, only one switch on SRAB interface */    
    max_devices = (max_bustype == MAX_BUSTYPE) ? 1 : MAX_SWITCH_DEVICES;

    for (dev = 0; dev < max_devices; dev++) {
        int match_idx, i;
        unsigned short phyidl_nr; /* phyidl with revision stripped */        
        uint16 model_id;
        uint8 rev_id;
        uint32 addr, len, mlen, op;

        if (_switch_n_devices >= MAX_SWITCH_DEVICES) {
            break;
        }
        buf[0] = buf[1] = 0;
        robo_rreg(robo, dev, 0x10, 0x04, buf, (uint)2);
        phyidh = buf[0] | (buf[1] << 8);

        buf[0] = buf[1] = 0;
        robo_rreg(robo, dev, 0x10, 0x06, buf, (uint)2);
        phyidl = buf[0] | (buf[1] << 8);

        /* Strip revision */
        phyidl_nr = phyidl & 0xfff0;

        match_idx = _spi_device_valid_check(phyidh, phyidl_nr, 1);
        if (match_idx == -1) {
            /*printf("found %d robo device(s).\n", _robo_devices); */
            break;
        }
        
        if(_spi_id_table[match_idx].model_info){
            addr = _spi_id_table[match_idx].model_info & 0xffff;
            len = (_spi_id_table[match_idx].model_info >> 16) & 0xf;
            robo_rreg(robo, dev, (addr >> 8), (addr & 0xff), buf, (uint)len);
            mlen = (_spi_id_table[match_idx].model_info >> 20) & 0xf;
            model_id = 0;
            for (i = 0; i < mlen; i++)
                model_id |= buf[i] << (i << 3);
            op = (_spi_id_table[match_idx].model_info >> 28) & 0xf;
            if(op == 1) {
                model_id |= phyidl_nr;
            }
        } else {
            model_id = phyidl_nr;
        }
        if(_spi_id_table[match_idx].rev_info){
            addr = _spi_id_table[match_idx].rev_info & 0xffff;
            len = (_spi_id_table[match_idx].rev_info >> 16) & 0xf;
            robo_rreg(robo, dev, (addr >> 8), (addr & 0xff), buf, (uint)len);
            mlen = (_spi_id_table[match_idx].rev_info >> 20) & 0xf;
            rev_id = 0;
            for (i = 0; i < mlen; i++)
                rev_id |= buf[i] << (i << 3);
        } else {
            rev_id = phyidl & 0xf;
        }
        /*
                printf("found robo device with %d:%04x:%04x:%04x:%02x\n",
                    dev, phyidh, phyidl, model_id, rev_id);
            */
        robo_select_device(robo, phyidh, phyidl);

        /* Match supported chips */
        vxd = _devices + _switch_n_devices++;
        
        vxd->dev_type = (BDE_SPI_DEV_TYPE | BDE_SWITCH_DEV_TYPE);
        vxd->spi_dev.cid = dev;
        vxd->spi_dev.part = model_id;
        vxd->spi_dev.rev = rev_id;
        vxd->spi_dev.robo = robo;
        vxd->spi_dev.phyid_high = phyidh;
        vxd->spi_dev.phyid_low = phyidl;
        vxd->bde_dev.device = model_id;
        vxd->bde_dev.rev = rev_id;
        vxd->bde_dev.base_address = (sal_vaddr_t)NULL;
        vxd->iLine = 0;
        vxd->iPin = 0;
        vxd->flags = 0;
        _robo_devices++;
        _n_devices++;

        spi_freq = _spi_id_table[match_idx].spifreq;

    }
    /* 
     * Override the SPI frequency from user configuration
     */ 
    if ((spi_freq_str = sal_config_get("spi_freq_override")) != NULL) {
        if ((spi_freq_override = sal_ctoi(spi_freq_str, 0)) != 0) {
            spi_freq = spi_freq_override;
        }
    }
    
    /* The underlying chip can support the SPI frequency higher than default (2MHz) */ 
    if (spi_freq != SPI_FREQ_DEFAULT) { 
        chipc_spi_set_freq(robo, 0, spi_freq); 
    } 

    return _robo_devices;
}
#endif
#endif

#ifdef   BCM_ICS
int
vxbde_create(vxbde_bus_t *bus, 
         ibde_t **bde)
{
    if (_n_devices == 0) {
        vxbde_dev_t *vxd;
        uint32  dev_rev_id = 0x0;
        _bus = *bus;

        bcm_bde_soc_cm_memory_base = _bus.base_addr_start;
        vxd = _devices + _n_devices++;
        _switch_n_devices++;
      
        vxd->bde_dev.base_address = _bus.base_addr_start;
        vxd->iLine = _bus.int_line;
        vxd->iPin  = 0;
        vxd->flags  = 0;
        if (_bus.base_addr_start)
            _bus.base_addr_start += PCI_SOC_MEM_WINSZ;

        vxd->dev_type = BDE_ICS_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

        dev_rev_id = _read(0, 0x178);  /* CMIC_DEV_REV_ID */
        vxd->bde_dev.device = dev_rev_id & 0xFFFF;
        vxd->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;
    }
    *bde = &_ibde;

#ifdef INCLUDE_RCPU
    /*
     * Check if devices are connected to bus in EB mode.
     */
    if (sal_config_get("eb_probe") != NULL) {
        vxbde_eb_bus_probe(bus, bde);
    }
#endif /* INCLUDE_RCPU */

    return 0;  
}
#elif (defined(IPROC_CMICD) && defined(INTERNAL_CPU))
/* Refer to _iproc_device_create() in linux-kernel-bde.c */
int
vxbde_create(vxbde_bus_t *bus, 
         ibde_t **bde)
{
    if (_n_devices == 0) {

#define IPROC_CHIPCOMMONA_BASE  0x18000000
#define BCM53010_CHIP_ID      0xcf12      /* 53010 chipcommon chipid */
#define BCM53018_CHIP_ID      0xcf1a      /* 53018 chipcommon chipid */
#define BCM53020_CHIP_ID      0xcf1e      /* 53020 chipcommon chipid */

        sal_vaddr_t iproc_cca_base;
        uint32 cca_cid;
        uint32 has_cmic = 0;

        vxbde_dev_t *vxd;
        uint32  dev_rev_id = 0x0;
        _bus = *bus;

        iproc_cca_base = (sal_vaddr_t)IPROC_CHIPCOMMONA_BASE;
        /* cca_cid = readl((uint32 *)(iproc_cca_base + 0x0000)); */
		cca_cid = *((uint32_t*)(iproc_cca_base + 0x0000));
        cca_cid &= 0xffff;

        /* Only allowed accessing CMICD module if the SOC has it */
        switch (cca_cid) {
            case BCM53010_CHIP_ID:
            case BCM53018_CHIP_ID:
            case BCM53020_CHIP_ID:
                has_cmic = 0;
                break;
            default:
                has_cmic = 1;
                break;
        }

        if (!has_cmic) {
            return 0;
        }

        bcm_bde_soc_cm_memory_base = _bus.base_addr_start;
        vxd = _devices + _n_devices++;
        _switch_n_devices++;
      
        vxd->bde_dev.base_address = _bus.base_addr_start;
        vxd->iLine = _bus.int_line;
        vxd->iPin  = 0;
        vxd->flags  = 0;
        vxd->dev_type = BDE_AXI_DEV_TYPE | BDE_SWITCH_DEV_TYPE;
        dev_rev_id = *((uint32_t*)(vxd->bde_dev.base_address + 0x10224));  /* CMIC_DEV_REV_ID */
        vxd->bde_dev.device = dev_rev_id & 0xFFFF;
        vxd->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;
    }

    *bde = &_ibde;

#ifdef INCLUDE_RCPU
    /*
     * Check if devices are connected to bus in EB mode.
     */
    if (sal_config_get("eb_probe") != NULL) {
        vxbde_eb_bus_probe(bus, bde);
    }
#endif /* INCLUDE_RCPU */

    return 0;  
}

#else

#define PLX9656_LAS0_BA         0x00000004  /* LAS0 Local Base Address Remap   */
#define PLX9656_LAS1_BA         0x000000f4  /* LAS1 Local Base Address Remap   */
#define PLX9656_LAS_EN          0x01000000  /* Space Enable bit                */

#define PLX8608_DEVICE_ID       0x8608      /* PLX PEX8608 device id           */
#define PLX8617_DEVICE_ID       0x8617      /* PLX PEX8617 device id           */
#define PLX8608_DEV_CTRL_REG    0x70        /* PLX PEX8608 control register    */
#define PLX86XX_DEV_CTRL_REG    0x70        /* PLX PEX86XX control register    */

STATIC int 
_plx_las_baroff_get(pci_dev_t *dev)
{
    uint32      local_config_bar;
    int         baroff = -1;

    /* Get base of local configuration registers */
    local_config_bar = pci_config_getw(dev, PCI_CONF_BAR0);
    if (local_config_bar) {
        /* 
         * Make sure LAS0BA or LAS1BA is enabled before returning
         * BAR that will be used to access the Local Bus
         */
        if ((*(uint32 *)(local_config_bar + PLX9656_LAS0_BA)) & PLX9656_LAS_EN) {
            baroff = PCI_CONF_BAR2;
        } else if ((*(uint32 *)(local_config_bar + PLX9656_LAS1_BA)) & 
                   PLX9656_LAS_EN) {
            baroff = PCI_CONF_BAR3;
        } 
    }
    return baroff;
}

#ifdef KEYSTONE
extern int getintvar(char *vars, const char *name);
#endif

static void
_shbde_log_func(int level, const char *str, int param)
{
    if (level < SHBDE_DBG) {
        sal_printf("%s (%d)\n", str, param);
    }
}

static int
_setup(pci_dev_t *dev, 
       uint16 pciVenID, 
       uint16 pciDevID,
       uint8 pciRevID)
{
    uint32 flags = 0;
    vxbde_dev_t *vxd = 0;
    char    pll_bypass[20];
    char    pci_soccfg0[20];
    char    pci_soccfg1[20];
    char    pci2eb_override[20];
    char    *val_str;
    int     cmic_bar;
    uint32  rval;
    int     baroff = PCI_CONF_BAR0;
    int     iproc = 0;
    shbde_hal_t shared_bde, *shbde = &shared_bde;

    /* Initialize Linux hardware abstraction for shared BDE functions */
    vxworks_shbde_hal_init(shbde, _shbde_log_func);

#ifdef METROCORE
    if (pciVenID != SANDBURST_VENDOR_ID) {
        return 0;
    }
    flags |= BDE_SWITCH_DEV_TYPE;
    vxd = _devices + _switch_n_devices++;
    _n_devices++;
#else 
    /* Common code for SBX and XGS boards */
    if ((pciVenID != SANDBURST_VENDOR_ID) &&
        (pciVenID != PLX_VENDOR_ID) &&
        (pciVenID != BROADCOM_VENDOR_ID) &&
        (pciVenID != PCP_PCI_VENDOR_ID)) {
        return 0;
    }

    /*
     * The presence of the PLX bridge indicates this is
     * a Metrocore GTO/XMC platform.
     * The PLX bridge itself won't be part of _devices[].
     * It's the devices behind PLX can be part of _devices[]
     */
    if (pciVenID == PLX_VENDOR_ID) {

        /* Fix up max payload on all PLX PEX8608 ports */        
        if ((pciDevID == PLX8608_DEVICE_ID) ||
            (pciDevID == PLX8617_DEVICE_ID)) {
            rval = pci_config_getw(dev, PLX8608_DEV_CTRL_REG);
            rval = (rval & ~(7<<5)) | (2 << 5);
            pci_config_putw(dev, PLX8608_DEV_CTRL_REG, rval);
            return 0;
        }

        if ((pciDevID == PLX9656_DEVICE_ID) ||
            (pciDevID == PLX9056_DEVICE_ID)) {
            baroff = _plx_las_baroff_get(dev);
            if (baroff == -1) {
                sal_printf("No Local Address Space enabled in PLX\n");
                return 0;
            }
            vxd = &plx_vxd;
            num_plx++;
            /* Adjust bus for METROCORE GTO */
            _bus.be_pio    = 0;
            _bus.be_packet = 0;
            _bus.be_other  = 0;
        }
    } else if (pciVenID == BROADCOM_VENDOR_ID &&
               (pciDevID == BCM47XX_ENET_ID ||
                pciDevID == BCM5300X_GMAC_ID)) {
        /*
         * BCM5836/4704 Use ENET MAC 1 
         */
        if (pciDevID == BCM47XX_ENET_ID) {
            if (_first_mac == 0) {
                _first_mac++;
                return 0;
            }
        }
#ifdef KEYSTONE
        if (pciDevID == BCM5300X_GMAC_ID) {
            _first_mac++ ;
            coreflags = (uint32)getintvar(NULL, "coreflags");
            if (coreflags == 0) {
                /* 
                 * The ROBO switch is connnected with GMAC core 0
                 * in the old ROBO boards.
                 * So set the bit of CORE_GMAC1 for Vxworks BSP.
                 */
                coreflags = CORE_GMAC1 | CORE_PCIE0 | CORE_PCIE1;
            }
            if ((coreflags & CORE_GMAC0) &&
                (_first_mac == 1)) {
                return 0;
            }
            if ((coreflags & CORE_GMAC1) &&
                (_first_mac == 2)) {
                return 0;
            }
        }   
#endif /* KEYSTONE */
        flags |= BDE_ETHER_DEV_TYPE;
        vxd = _devices + MAX_SWITCH_DEVICES + _ether_n_devices++;
        _n_devices++;
    } else if (pciVenID == PCP_PCI_VENDOR_ID) {
        if (pciDevID == PCP_PCI_DEVICE_ID) {
            /* over-ride the device and revision ids */
            pciDevID = GEDI_DEVICE_ID;
            pciRevID = GEDI_REV_ID;

            flags |= BDE_SWITCH_DEV_TYPE;
            vxd = _devices + _switch_n_devices++;
#if defined(GTO)
            vxd->cpu_address = 0xe0000000;
#endif /* GTO CPU */
            _n_devices++;
        }
    } else if (pciVenID == BROADCOM_VENDOR_ID &&
               ((pciDevID & 0xFF00) != 0x5600) &&
               ((pciDevID & 0xF000) != 0xc000) &&
               ((pciDevID & 0xFFF0) != 0x0230) &&
               ((pciDevID & 0xFFFF) != 0x0732) &&
               ((pciDevID & 0xFFF0) != 0x0030) &&
               ((pciDevID & 0xF000) != 0x8000) &&
               ((pciDevID & 0xF000) != 0xb000) &&
               ((pciDevID & 0xF000) != 0xa000)) {
        /* don't want to expose non 56XX/53XXX devices */
        return 0;
    } else {
        flags |= BDE_SWITCH_DEV_TYPE;
        vxd = _devices + _switch_n_devices++;
        _n_devices++;
    }
#endif /* METROCORE */

    /* Save shared BDE HAL in device structure */
    memcpy(&vxd->shbde, shbde, sizeof(vxd->shbde));

    sal_sprintf(pll_bypass, "pll_bypass.%d", _n_devices);
    sal_sprintf(pci_soccfg0, "pci_conf_soccfg0.%d", _n_devices);
    sal_sprintf(pci_soccfg1, "pci_conf_soccfg1.%d", _n_devices);
    sal_sprintf(pci2eb_override, "pci2eb_override.%d", _n_devices);
  
    flags |= BDE_PCI_DEV_TYPE;
    vxd->bde_dev.device = pciDevID;
    vxd->bde_dev.rev = pciRevID;
    vxd->bde_dev.base_address = 0; /* read back */
    vxd->pci_dev = *dev;
    vxd->dev_type = flags;

    /* Configure PCI */

    /* Write control word (turns on parity detect) */
    pci_config_putw(dev, PCI_CONF_COMMAND, (PCI_CONF_COMMAND_BM |
                     PCI_CONF_COMMAND_MS |
                     PCI_CONF_COMMAND_PERR));

    if (shbde_pci_is_pcie(shbde, dev)) {
        /* Set PCIe max payload */
        shbde_pci_max_payload_set(shbde, dev, 256);
    } else {
        /*
         * Set # retries to infinite.  Otherwise other devices using the bus
         * may monopolize it long enough for us to time out.
         */
        pci_config_putw(dev, PCI_CONF_TRDY_TO, 0x0080);

        /*
         * Optionally enable PLL bypass in reserved register.
         */
        if ((val_str = sal_config_get(pll_bypass)) != NULL) {
            if (_shr_ctoi(val_str)) {
                pci_config_putw(dev, PCI_CONF_PLL_BYPASS, 0x2);
                if (SAL_BOOT_QUICKTURN) {
                    sal_usleep(100000);
                }
            }
        }

        /*
         * Optionally configure clocks etc. in reserved registers.
         * These registers may contain multiple configuration bits.
         */
        if ((val_str = sal_config_get(pci_soccfg0)) != NULL) {
            pci_config_putw(dev, PCI_CONF_SOCCFG0, _shr_ctoi(val_str));
            if (SAL_BOOT_QUICKTURN) {
                sal_usleep(100000);
            }
        }
        if ((val_str = sal_config_get(pci_soccfg1)) != NULL) {

            pci_config_putw(dev, PCI_CONF_SOCCFG1, _shr_ctoi(val_str));
            if (SAL_BOOT_QUICKTURN) {
                sal_usleep(100000);
            }
        }
    }

    /* Check for iProc device */
    if (shbde_pci_is_iproc(shbde, dev, &cmic_bar)) {
        iproc = 1;
        if (cmic_bar == 2) {
            baroff = PCI_CONF_BAR2;
        }
    }

#if defined(KEYSTONE)
    if ((dev->busNo == PCIE_PORT0_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        _bus.base_addr_start = SI_PCI0_MEM;
        _bus.int_line = 3;
    }

    if ((dev->busNo == PCIE_PORT1_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        _bus.base_addr_start = SI_PCI1_MEM;
        _bus.int_line = 4;
        pcie1_device_exist = 1;
    }
    if (iproc) {
        /* CMIC would be on BAR2 */
        _bus.base_addr_start += IPROC_CMIC_WINSZ;
    }
#endif
    if (_bus.base_addr_start) {
        uint32 tmp;

        /*
         * The standard procedure to determine device window size is to
         * write 0xffffffff to BASE0, read it back, and see how many
         * LSBs are hardwired to zero.  In our case, we would get
         * 0xffff0000 indicating a window size of 64kB.
         *
         * While the window size could be assumed 64kB, we must still do
         * the standard write and read because some PCI bridges (most
         * notably the Quickturn Speed Bridge) observe these
         * transactions to record the window size internally.
         */
        
        pci_config_putw(dev, baroff, 0xffffffff);
        rval = pci_config_getw(dev, baroff);

        pci_config_putw(dev, baroff, _bus.base_addr_start);
        if ((rval & PCI_CONF_BAR_TYPE_MASK) == PCI_CONF_BAR_TYPE_64B) {
            pci_config_putw(dev, PCI_CONF_BAR1, 0);
        }
        
        if (!(flags & BDE_ETHER_DEV_TYPE)) {
            tmp = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE);
            tmp = (tmp & ~0xff) | _bus.int_line;
            
            pci_config_putw(dev, PCI_CONF_INTERRUPT_LINE, tmp);
        }

        /* should read window size from device */
        _bus.base_addr_start += PCI_SOC_MEM_WINSZ;
    }
#if defined(NSX) || defined(METROCORE)
    else {
        uint32 tmp;
        
        tmp = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE);
        tmp = (tmp & ~0xff) | _bus.int_line;
        
        pci_config_putw(dev, PCI_CONF_INTERRUPT_LINE, tmp);
    }
#endif /* NSX */

#if defined(KEYSTONE)
    if ((dev->busNo == PCIE_PORT0_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        vxd->bde_dev.base_address = 0xa8000000;
    }
    if ((dev->busNo == PCIE_PORT1_HB_BUS) && \
        (dev->devNo == 1) && (dev->funcNo == 0)) {
        vxd->bde_dev.base_address = 0xe0000000;
    }

    if ((pciDevID == BCM5300X_GMAC_ID) && (dev->busNo == 0)) {
        /* GMAC core in SI Bus */
        vxd->bde_dev.base_address = 0xb8000000;
    }
    if (iproc) {
        vxd->bde_dev.base_address1 = vxd->bde_dev.base_address;
        vxd->bde_dev.base_address += IPROC_CMIC_WINSZ;
    }

#else
    vxd->bde_dev.base_address = (sal_vaddr_t) 
        ((uint32*)(pci_config_getw(dev, baroff) & PCI_CONF_BAR_MASK));

    /* Map secondary address spaces */
    if (iproc) {
        vxd->bde_dev.base_address1 = (sal_vaddr_t) 
            ((uint32*)(pci_config_getw(dev, PCI_CONF_BAR0) & PCI_CONF_BAR_MASK));
    }

#endif
    if (iproc) {
        int paxb_core;
        void *iproc_regs;
        shbde_iproc_config_t iproc_config, *icfg = &iproc_config;

        /* Mapped iProc regs in PCI BAR 0 */
        iproc_regs = (void *)vxd->bde_dev.base_address1;

        /* iProc configuration parameters */
        memset(icfg, 0, sizeof(*icfg));
        shbde_iproc_config_init(icfg, vxd->bde_dev.device, vxd->bde_dev.rev);

        /* Call shared function */
        paxb_core = shbde_iproc_paxb_init(shbde, iproc_regs, icfg);

        /* Save PCI core information for CMIC */
        if (paxb_core == 1) {
            vxd->dev_type |= BDE_DEV_BUS_ALT;
        }

        /* iProc PCIe preemphasis */
        shbde_iproc_pcie_preemphasis_set(shbde, iproc_regs, icfg, dev);
    }
    vxd->flags = 0x0;
    if(flags & BDE_ETHER_DEV_TYPE){
#if defined(KEYSTONE)
        if (_first_mac == 1) {
            /* GMAC core 0 */
            vxd->iLine = 0x2;
        } else {
            /* GMAC core 1 */
            vxd->iLine = 0x1;
        }
#else /* !KEYSTONE */
        vxd->iLine = 0x2;
#endif /* KEYSTONE */
        vxd->iPin = 0x0;
        /*
         * Have at it.
         * ETHER_DEV_TYPE is for the internal emac in BCM5836/4704.
         * It is just a virtual/pseudo PCI device in PCI configuration space.
         */
        return 0;
    } else {
        vxd->iLine = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE) >> 0 & 0xff;
        vxd->iPin  = pci_config_getw(dev, PCI_CONF_INTERRUPT_LINE) >> 8 & 0xff;
#ifndef METROCORE
        if (pciVenID == SANDBURST_VENDOR_ID || pciVenID == PLX_VENDOR_ID) {
            vxd->iLine = FPGA_IRQ; /* all interrupts are from FPGA line */
            vxd->iPin  = 0;
        } 
#endif
    }

    /* Have at it */
    return 0;
}

#if defined(KEYSTONE)
/* PCI-E  host Bridge */
#define BCM53000PCIE0_REG_BASE 0xb8005400
#define BCM53000PCIE1_REG_BASE 0xb800e400
#define BCM53000PCIE_DEV_CAP_REG  0xd4
#define BCM53000PCIE_DEV_CTRL_REG 0xd8
#define BCM53000PCIE_MAX_PAYLOAD_MASK  0x7

#define BCM53000PCIE0_SPROM_SHADOW_OFF_14 0xb800581c
#define BCM53000PCIE1_SPROM_SHADOW_OFF_14 0xb800e81c
#define BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_MASK 0xe000
#define BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_OFFSET 13

#define MAX_PAYLOAD_256B       (1 << 5)
#define MAX_READ_REQ_256B      (1 << 12)

static void
pcie_adjust(int port)
{
    uint32 tmp;
    uint16 tmp16, tmp161;
    uint32 base;
    uint32 rom_data;
    uint32 mask;
    uint32 offset;
    /* configure the PCIE cap: Max payload size: 256, Max Read
     * Request size: 256, disabling relax ordering.
     * Writes to the PCIE capability device control register
     */

    if ((port == 0) && (coreflags & CORE_PCIE0)) {
        base = BCM53000PCIE0_REG_BASE;
        rom_data = BCM53000PCIE0_SPROM_SHADOW_OFF_14;
    } else if ((port == 1) && (coreflags & CORE_PCIE1)) {
        base = BCM53000PCIE1_REG_BASE;
        rom_data = BCM53000PCIE1_SPROM_SHADOW_OFF_14;
    } else {
        return;
    }

    tmp = *((uint32 *)(base+BCM53000PCIE_DEV_CAP_REG));
    if ((tmp & BCM53000PCIE_MAX_PAYLOAD_MASK) != 1) {
        tmp16 = *((uint16 *)rom_data);
        mask = BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_MASK;
        offset = BCM53000PCIE1_SPROM_SHADOW_MAX_PAYLOAD_OFFSET;
        if (((tmp16 & mask) >> offset) != 0x1) {
            tmp161 = (tmp16 & ~mask) | (0x1 << offset);
            *((uint16 *)rom_data) = tmp161;
        }
    }

    tmp16 = *((uint16 *)(base+BCM53000PCIE_DEV_CTRL_REG));
    if (!(tmp16 & MAX_PAYLOAD_256B) || !(tmp16 & MAX_READ_REQ_256B)) {
        tmp161 = tmp16 | MAX_PAYLOAD_256B | MAX_READ_REQ_256B;
        *((uint16 *)(base+BCM53000PCIE_DEV_CTRL_REG)) = tmp161;
    }

    return;
}
#endif 

static int
_soc_dev_probe(pci_dev_t *dev, 
       uint16 pciVenID, 
       uint16 pciDevID,
       uint8 pciRevID)
{
#ifdef METROCORE
    if (pciVenID != SANDBURST_VENDOR_ID) {
        return 0;
    }
#else
    if (pciVenID != BROADCOM_VENDOR_ID) {
        return 0;
    }

    /* Expose 56XX/53XXX and some non 56XX/53XXX devices */
    if(((pciDevID & 0xFF00) != 0x5600) &&
       ((pciDevID & 0xF000) != 0xc000) &&
       ((pciDevID & 0xFFF0) != 0x0230) &&
       ((pciDevID & 0xFFF0) != 0x0030) &&
       ((pciDevID & 0xF000) != 0xb000) &&
       ((pciDevID & 0xF000) != 0x8000)) {
        return 0;
    }
#endif

    if (_pri_bus_no ==  -1) {
        _pri_bus_no = dev->busNo;
    }

    if (dev->busNo == _pri_bus_no) {
        _n_pri_devices++;
    } else {
        _n_sec_devices++;
    }
  
    return 0;
}

static void
_adjust_bus_base_addr_start()
{
    if (!_bus.base_addr_start) {
        return;
    }
    pci_device_iter(_soc_dev_probe);
    if (_n_sec_devices) {
        if (_bus.base_addr_start)
            _bus.base_addr_start -= PCI_SOC_MEM_WINSZ * _n_pri_devices;
    }
    bcm_bde_soc_cm_memory_base = _bus.base_addr_start;

    /* printk("_n_pri_devices = %d _n_sec_devices = %d _bus.base_addr_start = %08x\n", _n_pri_devices, _n_sec_devices, _bus.base_addr_start);*/
}
    
/*
 * SBX platform has both PCI- and local bus-attached devices
 * The local bus devices have fixed address ranges (and don't
 * support or require DMA), but are otherwise the same as PCI devices
 */
#define FPGA_PHYS               0x100E0000
#define BME_PHYS                0x100C0000
#define SE_PHYS                 0x100D0000

/*
 * Please refer to "Supervisor Fabric Module (SFM) Specification"
 * page 23 for the following registers.
 */
#define FPGA_LC_POWER_DISABLE_OFFSET             0x4
#define FPGA_LC_POWER_DISABLE_ENABLE_ALL_MASK    0x1e

#define FPGA_LC_POWER_RESET_OFFSET               0x5
#define FPGA_LC_POWER_RESET_ENABLE_ALL_MASK      0x1e

#define FPGA_SW_SFM_MASTER_MODE_OFFSET           0x14
#define FPGA_SW_SFM_MASTER_MODE_ENABLE_MASK      0x10

#ifdef METROCORE
static int
probe_metrocore_local_bus(void)
{
    vxbde_dev_t         *vxd;
    uint32              dev_rev_id;
    volatile uint8_t    *fpga;

    /*
     * Write the FPGA on the fabric card, to let metrocore
     * line cards out of reset.  We actually don't bother to determine whether
     * the card is a line card or a fabric card because when we do
     * this on the line cards, it has no effect.
     */
    fpga = (uint8_t *)FPGA_PHYS;

    fpga[FPGA_SW_SFM_MASTER_MODE_OFFSET]
        |= FPGA_SW_SFM_MASTER_MODE_ENABLE_MASK;
    fpga[FPGA_LC_POWER_DISABLE_OFFSET]
        |= FPGA_LC_POWER_DISABLE_ENABLE_ALL_MASK;
    fpga[FPGA_LC_POWER_RESET_OFFSET]
        |= FPGA_LC_POWER_RESET_ENABLE_ALL_MASK;

    /*
     * Metrocore Local Bus device: BME 3200
     */
    vxd = _devices + _n_devices;
    _n_devices++;
    _switch_n_devices++;

    vxd->bde_dev.base_address = (sal_vaddr_t) BME_PHYS;
    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

    dev_rev_id = *(uint32 *)vxd->bde_dev.base_address;

    vxd->bde_dev.device = (dev_rev_id >> 16) & 0xFFFF;
    vxd->bde_dev.rev = dev_rev_id & 0xFFFF;
    vxd->iLine = FPGA_IRQ;
    vxd->iPin  = 0;

    if (vxd->bde_dev.device != BME3200_DEVICE_ID) {
        /* printk("probe_metrocore_local_bus: wrong BME type: "
                "0x%x (vs 0x%x)\n", vxd->bde_dev.device, BME3200_DEVICE_ID); */
        return -1;
    }

    /*
     * Metrocore Local Bus device: SE 3200
     */
    vxd = _devices + _n_devices;
    _n_devices++;
    _switch_n_devices++;

    vxd->bde_dev.base_address = (sal_vaddr_t) SE_PHYS;
    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

    dev_rev_id = *(uint32 *)vxd->bde_dev.base_address;

    vxd->bde_dev.device = (dev_rev_id >> 16) & 0xFFFF;
    vxd->bde_dev.rev = dev_rev_id & 0xFFFF;
    vxd->iLine = FPGA_IRQ;
    vxd->iPin  = 0;

    if (vxd->bde_dev.device != BME3200_DEVICE_ID) {
        /* printk("probe_metrocore_local_bus: wrong SE type: "
                "0x%x (vs 0x%x)\n", vxd->bde_dev.device, BME3200_DEVICE_ID); */
        return -1;
    }
    return 0;
}
#endif /* METROCORE */

/*
 * The address space behind the PLX9656 PCI-to-LOCAL bridge has already
 * been allocated by the system (vxworks) during the bus enumeration.
 *
 * Devices which have registers mapped in the local bus space:
 * BME9600 IO space start at PL0_OFFSET, PL0_SIZE(64KB).
 * FPGA IO space is right next, PL0_OFFSET + PL0_SIZE
 
 *
 */
#if 1
#define DEV_REG_BASE_OFFSET     PL0_OFFSET /* Polaris register base */
#define DEV_REG_DEVID           0          /* Device ID is first register */
#endif

static int
probe_plx_local_bus(void)
{
    vxbde_dev_t *vxd;
    uint32 dev_rev_id;

    if (num_plx == 0) {
            sal_printf("Not found PLX 9656/9056 chip\n");
        return -1;
    }
    if (num_plx > 1) {
            sal_printf("There's more than one PLX 9656/9056 chip\n");
        return -1;
    }

    /*
     * PLX Local Bus device: BME 9600
     */
    vxd = _devices + _n_devices;
    _n_devices++;
    _switch_n_devices++;

#if 1
    vxd->bde_dev.base_address =
        (int)plx_vxd.bde_dev.base_address + PL0_OFFSET;
    dev_rev_id = *(uint32 *)(vxd->bde_dev.base_address);
    dev_rev_id = _shr_swap32(dev_rev_id);
    vxd->bde_dev.device = (dev_rev_id >> 16) & 0xFFFF;
    vxd->bde_dev.rev = dev_rev_id & 0xFFFF;
#endif

    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;
    vxd->iLine = FPGA_IRQ;
    vxd->iPin  = 0;

    switch (vxd->bde_dev.device) {
    case BM9600_DEVICE_ID:
        break;
    default:
        sal_printf("probe_plx_local_bus: unexpected device type type: "
               "0x%x \n", vxd->bde_dev.device);
        return -1;
    }
    return 0;
}

#ifdef BCM_EA_SUPPORT
#define MAX_EA_DEVICES 8

#ifdef BCM_TK371X_SUPPORT
static int
_ea_tk371x_attach(void)
{
    vxbde_dev_t *vxd;
    char prop[64],*s;
    int dev = 0;
    int port = 0;
    
    for (dev = -1; dev < SOC_MAX_NUM_DEVICES; dev++) {
        for (port = 0; port < SOC_PBMP_PORT_MAX; port++) {
            sal_sprintf(prop, "ea_attach.port%d.%d", port, dev);
            if ((s = sal_config_get(prop))) {
                vxd = _devices + _switch_n_devices++;
                vxd->dev_type = (BDE_ET_DEV_TYPE | BDE_SWITCH_DEV_TYPE);
                vxd->bde_dev.device = TK371X_DEVICE_ID;
                vxd->bde_dev.rev = 0x0;
                vxd->bde_dev.base_address = (sal_vaddr_t)NULL;
                vxd->iLine = 0;
                vxd->iPin = 0;
                vxd->flags = 0;
                _n_devices++;
            }
        }
    }
    return 0;
}
#endif /* BCM_TK371X_SUPPORT*/
#endif /* BCM_EA_SUPPORT */

int
vxbde_create(vxbde_bus_t *bus, 
         ibde_t **bde)
{
    if (_n_devices == 0) {
    _bus = *bus;
        sal_sleep(1);
        _adjust_bus_base_addr_start();
    pci_device_iter(_setup);
    }

#if defined(KEYSTONE)
    pcie_adjust(0);
    if (pcie1_device_exist) {
        pcie_adjust(1);
    }
#endif

#ifdef BCM_ROBO_SUPPORT
#if defined(KEYSTONE)
    _spi_attach();
#endif    
#endif    

#ifdef BCM_EA_SUPPORT
#ifdef BCM_TK371X_SUPPORT
    _ea_tk371x_attach();
#endif

#endif /*BCM_EA_SUPPORT */

    *bde = &_ibde;
#ifdef PCI_DECOUPLED 
    sysPciRWInit();
#endif

#ifdef INCLUDE_RCPU
    /*
     * Check if devices are connected to bus in EB mode.
     */
    if (sal_config_get("eb_probe") != NULL) {
        vxbde_eb_bus_probe(bus, bde);
    }
#endif /* INCLUDE_RCPU */

#ifdef METROCORE
    if (probe_metrocore_local_bus()) {
        return -1;
    }
#endif

    if (num_plx) {
        if (probe_plx_local_bus()) {
            return -1;
        }
    }

    return 0;  
}
#endif

/*
 * Create EB device at the specified base address.
 */
int vxbde_eb_create(vxbde_bus_t *bus, 
                    ibde_t **bde, sal_vaddr_t base_address)
{
    int                 devId;
    vxbde_dev_t         *vxd;
    char                prop[64], *s;
    uint32              dev_rev_id = 0x0;

    vxd = _devices + _n_devices;

    if (_n_devices == 0) {
        bcm_bde_soc_cm_memory_base = _bus.base_addr_start = base_address;
    }

    vxd->bde_dev.base_address = (sal_vaddr_t) base_address;
    vxd->dev_type = BDE_EB_DEV_TYPE | BDE_SWITCH_DEV_TYPE;

    devId = _n_devices;

    _n_devices++;
    _switch_n_devices++;

    sal_sprintf(prop, "eb_bus_read_16bit.%d", devId);
    if ((s = sal_config_get(prop))) {
        vxd->flags |= BDE_FLAG_BUS_RD_16BIT;
    }
    sal_sprintf(prop, "eb_bus_write_16bit.%d", devId);
    if ((s = sal_config_get(prop))) {
        vxd->flags |= BDE_FLAG_BUS_WR_16BIT;
    }

    dev_rev_id = _read(devId, 0x178);  /* CMIC_DEV_REV_ID */

    vxd->bde_dev.device = dev_rev_id & 0xFFFF;
    vxd->bde_dev.rev = (dev_rev_id >> 16) & 0xFF;
#ifdef   BCM_ICS
    vxd->iLine = 1;
#else
    vxd->iLine = 0;
#endif /* BCM_ICS */
    vxd->iPin  = 0;

    if (bde) {
        *bde = &_ibde;
    }
    return 0;
}
