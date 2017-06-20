/*
 * $Id: linux-user-bde.c,v 1.105 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Linux User BDE User Library
 */

#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sched.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <endian.h>

#include <sal/core/sync.h>
#include <sal/core/spl.h>
#include <sal/core/thread.h>
#include <sal/core/time.h>
#include <sal/core/alloc.h>
#include <soc/cmic.h>
#include <soc/devids.h>

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)

#ifdef __DUNE_LINUX_BCM_CPU_PCIE__
#include <soc/dpp/SAND/Utils/sand_framework.h>
#else /* !__DUNE_LINUX_BCM_CPU_PCIE__ */
#include <soc/i2c.h>
#if defined(BCM_DFE_SUPPORT)
#include <appl/diag/dfe/utils_fe1600_card.h>
#endif
#if (defined(__DUNE_GTO_BCM_CPU__) || defined(__DUNE_WRX_BCM_CPU__)) && defined(INCLUDE_I2C)
#define DUNE_GTO_I2C
#endif
#endif /* __DUNE_LINUX_BCM_CPU_PCIE__ */

#endif /* defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT) */

#include <mpool.h>
#include <linux-bde.h>

#include "kernel/linux-user-bde.h"

#include <linux/version.h>

#ifdef KEYSTONE
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#include <linux/types.h>
#include <linux/spi/spidev.h>

#include <sal/core/libc.h>
#include <sal/appl/config.h>

#define LINUX_SPIDEV_SUPPORT 1
#else
#define LINUX_SPIDEV_SUPPORT 0
#endif
#else /* !KEYSTONE */
#if defined(IPROC_CMICD) && defined(BCM_STARFIGHTER3_SUPPORT)
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sal/core/libc.h>
#include <sal/appl/config.h>
#define LINUX_SPIDEV_SUPPORT 1
#else
#define LINUX_SPIDEV_SUPPORT 0
#endif

#endif /* KEYSTONE */

#if defined(BCM_ESW_SUPPORT) && LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,27)
#define CMIC_SPI_SUPPORT 1
#endif
#ifndef CMIC_SPI_SUPPORT
#define CMIC_SPI_SUPPORT 0
#endif

#if CMIC_SPI_SUPPORT 
#include <linux/spi/spidev.h>

/* 
 * The path to the spidev driver device file 
 */
#define LINUX_USER_CMIC_SPIDEV_NAME "/dev/spidev0.0"

/* Data array to record Read/Wirte device through the Linux mdio device driver or not */
static int cmic_spidev_linux[LINUX_BDE_MAX_SWITCH_DEVICES];

/*
 * Actual spi devices in our system
 */
static struct {
    int unit;
    int spidev_fd; /* CPU MDIO device descriptor */
} _sys_cmic_spidevs[LINUX_BDE_MAX_SWITCH_DEVICES];

static int cmic_spidev_log_on = 0;

#define CMIC_SPI_NORMAL 0
#define CMIC_SPI_FAST 1
#define CMIC_SPI_READ 0
#define CMIC_SPI_WRITE 1
#define CMIC_SPI_NORMAL_BITS_7_5 0x3

static unsigned int 
_cmic_spidev_read(int dev, uint32 addr)
{    
    int i;
    int ret;    
    int fd = _sys_cmic_spidevs[dev].spidev_fd;
    unsigned int value;
    uint8 tx[5];
    uint8 rx[4];
    struct spi_ioc_transfer tr[2];

    memset(&rx, 0, sizeof(rx));

    tx[0] = (CMIC_SPI_NORMAL_BITS_7_5 << 5) | (CMIC_SPI_NORMAL << 4) | \
            ((dev & 0x7) << 1) | CMIC_SPI_READ;
    tx[1] = addr & 0x000000ff;
    tx[2] = (addr & 0x0000ff00) >> 8;
    tx[3] = (addr & 0x00ff0000) >> 16;
    tx[4] = (addr & 0xff000000) >> 24;

    memset(tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx;
    tr[0].len = sizeof(tx);

    tr[1].rx_buf = (unsigned long)rx;
    tr[1].len = sizeof(rx);

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
    if (ret < 1) {
        printf("Can't send spi message: read(ret=%d)\n", ret);
    }

    value = 0;
    for (i = 0; i < sizeof(rx); i++) {
        value |= (rx[i] << i*8);
    }

    return value;
}

static int
_cmic_spidev_write(int dev, uint32 addr, uint32 data)
{    
    int ret;    
    int fd = _sys_cmic_spidevs[dev].spidev_fd;
    uint8 tx1[9];
    struct spi_ioc_transfer tr[1];

    memset(tx1, 0, sizeof(tx1));

    tx1[0] = (CMIC_SPI_NORMAL_BITS_7_5 << 5) | (CMIC_SPI_NORMAL << 4) | \
            ((dev & 0x7) << 1) | CMIC_SPI_WRITE;
    tx1[1] = addr & 0x000000ff;
    tx1[2] = (addr & 0x0000ff00) >> 8;
    tx1[3] = (addr & 0x00ff0000) >> 16;
    tx1[4] = (addr & 0xff000000) >> 24;

    tx1[5] = data & 0x000000ff;
    tx1[6] = (data & 0x0000ff00) >> 8;
    tx1[7] = (data & 0x00ff0000) >> 16;
    tx1[8] = (data & 0xff000000) >> 24;
    
    memset(&tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx1;
    tr[0].len = sizeof(tx1);

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        printf("Can't send spi message: write(ret=%d)\n", ret);
    }

    return ret;
}

/* 
 * Function:
 *    _sys_cmic_spidev_deinit
 * Purpose:
 *    close linux spi dev driver device file
 * Parameters:
 *    dev - device number
 * Returns:
 *    void
 */
static void
linux_cmic_spidev_close(int dev)
{
    int fd;

    fd = _sys_cmic_spidevs[dev].spidev_fd;
    close(fd);
}

static int
linux_cmic_spidev_open(int dev)
{
    char *spidev_name = LINUX_USER_CMIC_SPIDEV_NAME;
    int fd = 0;


    /* Initialization */
    memset(cmic_spidev_linux, 0, sizeof(cmic_spidev_linux));

    fd = open(spidev_name, O_RDWR);
    if (fd >= 0) {
        _sys_cmic_spidevs[dev].unit = dev;
        _sys_cmic_spidevs[dev].spidev_fd = fd;
        cmic_spidev_linux[dev] = 1;
    }

    cmic_spidev_log_on = 1;

    return 0;

}
#endif 

#ifdef IPROC_CMICD
#include <soc/cmic.h>
#define LINUX_MDIODEV_SUPPORT 1

/* IOCTL commands */
#define MDIO_IOC_MAGIC       'm'
struct mdio_ioc_transfer {
    uint8   pa; /* phy address */
    uint8   ra; /* register address */
    uint16  tx_buf; /* buffer for write */
    uint16  rx_buf; /* buffer for read */
};

#define MDIO_MSGSIZE(N) \
    ((((N)*(sizeof (struct mdio_ioc_transfer))) < (1 << _IOC_SIZEBITS)) \
        ? ((N)*(sizeof (struct mdio_ioc_transfer))) : 0)

#define MDIO_IOC_MESSAGE(N) _IOW(MDIO_IOC_MAGIC, 0, char[MDIO_MSGSIZE(N)])

#define MDIO_IOC_EXTERNAL_R_REG _IOWR(MDIO_IOC_MAGIC, 0, char[MDIO_MSGSIZE(1)])
#define MDIO_IOC_EXTERNAL_W_REG _IOW(MDIO_IOC_MAGIC, 1, char[MDIO_MSGSIZE(1)])
#define MDIO_IOC_LOCAL_R_REG _IOWR(MDIO_IOC_MAGIC, 2, char[MDIO_MSGSIZE(1)])
#define MDIO_IOC_LOCAL_W_REG _IOW(MDIO_IOC_MAGIC, 3, char[MDIO_MSGSIZE(1)])

#else
#define LINUX_MDIODEV_SUPPORT 0
#endif /* IPROC_CMICD */

#ifdef SAL_BDE_32BIT_USER_64BIT_KERNEL

typedef struct sal_kernel64_ptr_s {
    uint64    ptr64;
} *sal_kernel64_ptr_t;

#define SAL_SEM_REDIRECT

#ifndef PHYS_ADDRS_ARE_64BITS
#define PHYS_ADDRS_ARE_64BITS
#endif
#include <sys/mman.h>
#define MMAP    mmap
typedef uint64 phys_addr_t;

#else /* SAL_BDE_32BIT_USER_64BIT_KERNEL */

#ifdef PHYS_ADDRS_ARE_64BITS
#include <sys/mman.h>
#ifdef SAL_BDE_USE_MMAP64
#define MMAP    mmap64
#else
#define MMAP    mmap
#endif
typedef uint64 phys_addr_t;
#else 
#define MMAP    mmap
typedef uint32 phys_addr_t;
#endif  /* PHYS_ADDRS_ARE_64BITS */

#endif /* SAL_BDE_32BIT_USER_64BIT_KERNEL */

#define _SWAP32(_x) \
    (((_x) << 24) | (((_x) & 0xff00) << 8) | \
     (((_x) & 0xff0000) >> 8) | ((_x) >> 24))

/* 
 * The path to the driver device file 
 */
#define LUBDE_DEVICE_NAME "/dev/" LINUX_USER_BDE_NAME

/* 
 * The path to the mem device file 
 */
#define MEM_DEVICE_NAME "/dev/mem"
#define LKBDE_DEVICE_NAME "/dev/"LINUX_KERNEL_BDE_NAME

/*
 * Device information structure 
 */
typedef struct bde_dev_s {
    int                 dev_id;         /* HW (probed in kernel BDE) device id*/
    uint32              dev_type;       /* Type of underlaying device */
    ibde_dev_t          bde_dev;        /* BDE device description */
    phys_addr_t         pbase;          /* Physical base address of the device */
    uint32             *vbase;          /* Virtual base address of the device */
    uint32             *vbase1;         /* Secondary virtual base address #1 */
    uint32             *vbase2;         /* Secondary virtual base address #2 */
#ifdef DUNE_GTO_I2C
    int                i2c_addr;       /* I2C base address */
    CPU_I2C_BUS_LEN    i2c_access_type;
#endif /*DUNE_GTO_I2C*/
} bde_dev_t;

static bde_dev_t* _devices[LINUX_BDE_MAX_DEVICES];
static int        _ndevices = 0;
static int        _switch_ndevices = 0;
static int        _ether_ndevices = 0;
static int        _cpu_ndevices = 0;
static int        _bde_version = -1;

/*
 * instance information
 */
static unsigned int     _inst_dev_mask = 0;
static unsigned int     _inst_dma_size = 0;

/*
 * Global DMA pool. 
 *
 * One DMA memory pool is shared by all devices. 
 */
static uint32* _dma_vbase = NULL;
static uint32 _dma_pbase = 0;
static uint32 _dma_size = 0;
static mpool_handle_t _dma_pool;

/* 
 * Device File Descriptors
 */
static int _devfd = -1;
static int _memfd = -1;
static int _kdevfd = -1;
static int _use_kernel_bde_mmap = 0;

#ifndef BCM_PLX9656_LOCAL_BUS
#ifdef SAL_BDE_CACHE_DMA_MEM
/*
 * Optionally use cached memory for DMA to improve performance.
 * Should be enabled on cache-coherent platforms only to avoid
 * data corruption and other fatal errors.
 */
static int _sync_flags = 0;
#else
static int _sync_flags = O_SYNC | O_DSYNC | O_RSYNC;
#endif /* SAL_BDE_CACHE_DMA_MEM */
#endif /* BCM_PLX9656_LOCAL_BUS */

#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT

#define USER_SPI_FREQ_DEFAULT 2000000  /* 2MHz */
#define USER_SPI_FREQ_20MHZ   20000000 /* 20MHz */

#define DEFAULT_SPI_TIMEOUT 50
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

/* 
 * The path to the spi driver device file 
 */
#define LINUX_USER_SPIDEV0_NAME "spidev0.0"
#define LUSPI_DEVICE0_NAME "/dev/" LINUX_USER_SPIDEV0_NAME

#define LINUX_USER_SPIDEV1_NAME "spidev0.1"
#define LUSPI_DEVICE1_NAME "/dev/" LINUX_USER_SPIDEV1_NAME

#define LINUX_USER_SPIDEV2_NAME "spidev0.2"
#define LUSPI_DEVICE2_NAME "/dev/" LINUX_USER_SPIDEV2_NAME

/* Data array to record Read/Wirte device through the Linux spidev driver or not */
static int spi_dev_linux[LINUX_BDE_MAX_SWITCH_DEVICES];

/*
 * Actual spi devices in our system
 */
static struct {
    int unit;

    /* For devices using Linux spidev driver */
    int spi_fd; /* SPI device descriptor */
    uint8 spi_mode;
    uint8 spi_bits;
    uint32 spi_speed;
    uint16 spi_delay;
} _sys_devs[LINUX_BDE_MAX_SWITCH_DEVICES];

static int spi_log_on = 0;

#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_SPIDEV_SUPPORT */

#if LINUX_MDIODEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT

/* 
 * The path to the mdio driver device file 
 */
#define LINUX_USER_MDIODEV_NAME "/dev/mdio"

/* Data array to record Read/Wirte device through the Linux mdio device driver or not */
static int mdio_dev_linux[LINUX_BDE_MAX_SWITCH_DEVICES];

/*
 * Actual spi devices in our system
 */
static struct {
    int unit;

    int mdio_fd; /* CPU MDIO device descriptor */
} _sys_mdiodevs[LINUX_BDE_MAX_SWITCH_DEVICES];

static int mdio_log_on = 0;

#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_MDIODEV_SUPPORT */

#define IPROC_SUBWIN_MAX        8
#define IPROC_DEFAULT_SUBWIN    7
#define IPROC_PAXB_PAGE         0x18012000
#define IPROC_PAXB_IMAP0_ADDR   (IPROC_PAXB_PAGE + 0xc00)
#define BAR0_PAXB_IMAP0_0       0x2c00


typedef struct _iproc_subwin_s {
    uint32 addr_min;
    uint32 addr_max;
} _iproc_subwin_t;

typedef struct _iproc_map_s {
    _iproc_subwin_t subwin[IPROC_SUBWIN_MAX];
} _iproc_map_t;

static _iproc_map_t iproc_map_default = {
    {
        { 0x18000000, 0x18000fff }, /* CCA */
        { 0x18030000, 0x18030fff }, /* CCB */
        { 0x18012000, 0x18012fff }, /* PAXB */
        { 0, 0 }
    }
};

static _iproc_map_t iproc_map[LINUX_BDE_MAX_DEVICES];

sal_mutex_t iproc_map_lock;

static int
_devio_remap(unsigned int command, lubde_ioctl_t *pdevio)
{
    int idx = 0;

    if ((_inst_dev_mask == 0)||(_devices[0] == NULL)) {
        return 0;
    }

    switch(command) {
    case LUBDE_GET_DEVICE_STATE:
    case LUBDE_GET_NUM_DEVICES :
    case LUBDE_GET_DMA_INFO:
    case LUBDE_ATTACH_INSTANCE:
    case LUBDE_USLEEP:
    case LUBDE_UDELAY:
    case LUBDE_SEM_OP:
        /* 
         * We don't need to do the remap for those devio.dev
         * which is not used to identify the device id 
         */
        return 0;
    }

    idx = pdevio->dev;
    pdevio->dev = _devices[idx]->dev_id;

    return 0;
}

/*
 * Function: _ioctl
 *
 * Purpose:
 *    Helper function for performing device ioctls
 * Parameters:  
 *    command - ioctl command code
 * Returns:
 *    Asserts if ioctl() system call fails. 
 *    Returns devio.rc value. 
 * Notes:
 *    You must program the devio structure with your parameters
 *    before calling this function. 
 */
static int
_ioctl(unsigned int command, lubde_ioctl_t *pdevio)
{
    pdevio->rc = -1;
    _devio_remap(command, pdevio);
    assert(ioctl(_devfd, command, pdevio) == 0);
    return pdevio->rc;
}

/*
 * Function: _mmap
 *
 * Purpose:
 *    Helper function for address mmapping. 
 * Parameters:
 *    p - physical address start
 *    size - size of region
 * Returns:
 *    Pointer to mapped region, or NULL on failure. 
 */
static void *
_mmap(phys_addr_t p, int size) 
{  
    void *map;
    phys_addr_t page_size, page_mask;
    unsigned int offset;
    phys_addr_t  paddr;
    sal_vaddr_t  vmap;
    int dev_fd = (_use_kernel_bde_mmap && /* device to use for mmap */
      (p < (phys_addr_t)_dma_pbase + _dma_size) && (p >= (phys_addr_t)_dma_pbase)) ?
      _kdevfd : _memfd;

    page_size = getpagesize();
    page_mask = ~(page_size - 1);

    if (p & ~page_mask) {
        /*
        * If address (p) not aligned to page_size, we could not get the virtual 
        * address. So we make the paddr aligned with the page size.
        * Get the _map by using the aligned paddr.
        * Add the offset back to return the virtual mapped region of p.
        */

        paddr = p & page_mask;
        offset = p - paddr;
        size += offset;

        map = MMAP(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dev_fd, paddr);
        if (map == MAP_FAILED) {
            perror("aligned mmap failed: ");
            map = NULL;
        }
        vmap = PTR_TO_UINTPTR(map) + offset;
        return (void *)(vmap);
    }
#ifdef SAL_BDE_USE_MMAP2
    size += (p & ~page_mask);
    map = (void *)syscall(4210, 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, (off_t)((p & page_mask) >> 12));
    if (map == MAP_FAILED) {
        perror("mmap2 failed: ");
        map = NULL;
    }
    else {
        map = (void *)(((unsigned char *)map) + (p & ~page_mask));
    }
#else
    map = MMAP(NULL, size, PROT_READ|PROT_WRITE, MAP_SHARED, dev_fd, p);
    if (map == MAP_FAILED) {
        perror("mmap failed: ");
        map = NULL;
    }
#endif
    return map;
}

/*
 * Function: _get_dma_info
 *
 * Purpose:
 *    Retrieve the size and base address of the DMA memory pool
 * Parameters:
 *    pbase - (out) physical address of the memory pool
 *    size  - (out) size of the memory pool
 * Returns:
 *    0
 */
static int
_get_dma_info(uint32* pbase, uint32* size)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.dev = _inst_dev_mask;
    devio.d2 = 0; /* if we work with older kernel modules, this will make us behave properly */
    _ioctl(LUBDE_GET_DMA_INFO, &devio);
    *pbase = devio.d0;
    *size = devio.d1;
    _use_kernel_bde_mmap = devio.d2;
    return 0;
}

static int
_bde_instance_attach(unsigned int dev_mask, unsigned int dma_size)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.d0 = dev_mask;
    devio.d1 = dma_size;

    _ioctl(LUBDE_ATTACH_INSTANCE, &devio);

    return devio.rc;

}

/* 
 * Function:
 *     _get_dev_state
 * Purpose:
 *    BDE get_dev_state function. Returns device state.
 *     (BDE_DEV_STATE_REMOVED/CHANGED)
 * Parameters:
 *    d - device number
 * Returns:
 *    state of underlaying device.
 */
static int
_get_dev_state(int d)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

    assert(d >= 0 || d < _ndevices);

    if (_bde_version > 0) {
        /* LUBDE_GET_DEVICE_STATE support after BDE Version 1 */
        _devio.dev = d;
        if (_ioctl(LUBDE_GET_DEVICE_STATE, &_devio) != 0) {
            return -1;
        }
        return _devio.d0;
    } 

    return -1;
}

/*
 * Function: _open
 *
 * Purpose:
 *    Open device driver
 *    Initialize device structures
 *    Initialize DMA memory
 * Parameters:
 *    None
 * Returns:
 *    0 on successfully initialization
 *    -1 on error. 
 */
static int 
_open(void)
{ 
    lubde_ioctl_t devio;
    phys_addr_t pbase;
    int i, j, dev_no;
    int linux24;
    int procfd;
    char procbuf[4];
    uint32 rval;
    int num_devices;
    int _dev_mask = 0;
#ifdef BCM_JERICHO_SUPPORT
    int devknetfd = -1;
#endif

#ifndef NDEBUG    
    int _ioctl_LUBDE_GET_NUM_DEVICES;    
    int _ioctl_LUBDE_GET_DEVICE_TYPE;    
    int _ioctl_LUBDE_GET_DEVICE;
#endif /* !NDEBUG */   

    if (_devfd >= 0) {
        /* Already open */
        return 0;
    }

    /* Check Linux kernel version */
    linux24 = 0;
    if ((procfd = open("/proc/sys/kernel/osrelease", O_RDONLY)) >= 0) {
        if ((read(procfd, procbuf, sizeof(procbuf))) == 4) {
            if (strncmp(procbuf, "2.4", 3) == 0) {
                linux24 = 1;
            }
        }
        close(procfd);
    }

    /* Open the device driver */
    if ((_devfd = open(LUBDE_DEVICE_NAME,
                       O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0) {
        /* Try inserting modules from the current directory */
        if (linux24) {
            system("/sbin/insmod linux-kernel-bde.o");
            system("/sbin/insmod linux-user-bde.o");
        } else {
            system("/sbin/insmod linux-kernel-bde.ko");
            system("/sbin/insmod linux-user-bde.ko");
        }

        if ((_devfd = open(LUBDE_DEVICE_NAME,
                           O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0) {
            perror("open " LUBDE_DEVICE_NAME ": ");
            return -1;
        }
    }

/*  knet is only supported on jericho and qmx of DUNE */
#ifdef BCM_JERICHO_SUPPORT
#ifdef INCLUDE_KNET
    /* check if knet module exists */
    if ((devknetfd = open("/dev/linux-bcm-knet", O_RDONLY | O_NONBLOCK  )) < 0) {
        /* Try inserting knet module when KNET feature is enabled */
        if (linux24) {
            /* no linux-bcm-knet.o */
        } else {
            system("/sbin/insmod linux-bcm-knet.ko");
        }
    }
#else
    /* check if knet module exists */
    if ((devknetfd = open("/dev/linux-bcm-knet", O_RDONLY | O_NONBLOCK  )) >= 0) {
        close(devknetfd);
        /* Try removing knet module when KNET feature is not eanbled*/
        if (linux24) {
            /* no linux-bcm-knet.o */
        } else {
            system("/sbin/rmmod linux_bcm_knet");
        }   
    }
#endif
#endif

    if (_inst_dev_mask) {
        if (_bde_instance_attach(_inst_dev_mask, _inst_dma_size) < 0) {
            perror("fail to attach instance ");
            close(_devfd);
            return -1;
        }
    }

    _get_dma_info(&_dma_pbase, &_dma_size); /* get dma pool information and its handling mode */

#ifndef BCM_PLX9656_LOCAL_BUS
    if ((_memfd = open(MEM_DEVICE_NAME, O_RDWR | _sync_flags)) < 0) {
        perror("open " MEM_DEVICE_NAME ": ");
        close(_devfd);
        return -1;
    }
#else /* BCM_PLX9656_LOCAL_BUS */
    /*
     * On 440GX board with 2.6 kernel, /dev/mem and mmap don't work well
     * together.  So we use /dev/linux-user-bde for mapping instead
     * (see systems/linux/kernel/modules/shared/gmodule.c) -- hqian 8/15/07
     */
    _memfd = _devfd;
#endif
    if ( _use_kernel_bde_mmap &&
      (_kdevfd = open(LKBDE_DEVICE_NAME, O_RDWR | O_SYNC | O_DSYNC | O_RSYNC)) < 0) {
        perror("open " LKBDE_DEVICE_NAME ": ");
        close(_devfd);
#ifndef BCM_PLX9656_LOCAL_BUS
        close(_memfd);
#endif
        return -1;
    }

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    /* Get BDE module version */
    if (_bde_version == -1) {
#ifndef NDEBUG
        /* "assert" maps to NULL statement with NDEBUG */
        const int _ioctl_LUBDE_VERSION = 
#endif /* !NDEBUG */
        _ioctl(LUBDE_VERSION, &devio);
        assert(_ioctl_LUBDE_VERSION == 0);
        _bde_version = devio.d0;
    }

    /* Get the availabled devices from the driver */
    memset(_devices, 0, sizeof(_devices));  
 
    /* first, get all available devices */
    devio.dev = BDE_ALL_DEVICES;
    #ifndef NDEBUG
        _ioctl_LUBDE_GET_NUM_DEVICES =
    #endif /* !NDEBUG */
        _ioctl(LUBDE_GET_NUM_DEVICES, &devio);
    assert(_ioctl_LUBDE_GET_NUM_DEVICES == 0);

    num_devices = devio.d0;
    if (num_devices == 0) {
        printf("linux-user-bde: no devices\n");
    }
    _dev_mask = _inst_dev_mask;
    if(_dev_mask == 0) {
        /* Include all devices for non-instance mode */
        _dev_mask = ~0;
    }
    /* Initialize device structures for each device */
    for (dev_no = 0; dev_no < num_devices; dev_no++) {
        uint32 dev_type;
        uint32 dev_state;

        dev_state = _get_dev_state(dev_no);
        if (dev_state == BDE_DEV_STATE_REMOVED) {
            printf("linux-user-bde: device was removed.\n");
            continue;
        } else if (dev_state == -1){
            printf("linux-user-bde: hot-plug device state not supported "
                   "by kernel module.\n");
        }

        if ((_dev_mask & (1 << dev_no)) == 0) {
            continue;
        }
        /* i tracks devices belonging to this instance */
        i = _ndevices++;

        _devices[i] = (bde_dev_t*)malloc(sizeof(bde_dev_t));
        memset(_devices[i], 0, sizeof(bde_dev_t));
        iproc_map[i] = iproc_map_default;

        _devices[i]->dev_id = dev_no;
        /* Get the type of device */
        devio.dev = i;
#ifndef NDEBUG
    _ioctl_LUBDE_GET_DEVICE_TYPE=
#endif /* !NDEBUG */
    _ioctl(LUBDE_GET_DEVICE_TYPE, &devio);
    assert(_ioctl_LUBDE_GET_DEVICE_TYPE == 0);

        dev_type = devio.d0;

        if (dev_type & BDE_SWITCH_DEV_TYPE) {
            _switch_ndevices++;
        } else if (dev_type & BDE_ETHER_DEV_TYPE){
            _ether_ndevices++;
        } else if (dev_type & BDE_CPU_DEV_TYPE){
            _cpu_ndevices++;
        }

        /* Get the detail info of the device */
        devio.dev = i;
#ifndef NDEBUG
    _ioctl_LUBDE_GET_DEVICE =
#endif /* !NDEBUG */
    _ioctl(LUBDE_GET_DEVICE, &devio);
    assert(_ioctl_LUBDE_GET_DEVICE == 0);

        _devices[i]->bde_dev.device = devio.d0;
        _devices[i]->bde_dev.rev = devio.d1;

        if (BDE_DEV_MEM_MAPPED(dev_type)) {
            int size;

            /* Default is 64K memory window */
            size = 64 * 1024;
            if (dev_type & BDE_128K_REG_SPACE) {
                size = 128 * 1024;
            } else if (dev_type & BDE_256K_REG_SPACE) {
                size = 256 * 1024;
            } else if (dev_type & BDE_320K_REG_SPACE) {
                size = 320 * 1024;
            }

            /* Get physical devices address */
#ifdef PHYS_ADDRS_ARE_64BITS
            pbase = devio.d3;
            pbase <<= 32;
            pbase |= devio.d2;
#else
            pbase = devio.d2;
#endif /* PHYS_ADDRS_ARE_64BITS */
            _devices[i]->pbase = pbase;
            
            switch (_devices[i]->bde_dev.device) {
            case BCM53000_GMAC_DEVICE_ID:
                size = 0x300000;
                break;
            case GEDI_DEVICE_ID:
            case ACP_PCI_DEVICE_ID:
                size = 0x10000000;
                dev_type |= BDE_BYTE_SWAP;
                break;
            default:
                break;
            }
            {    
            _devices[i]->vbase = _mmap(pbase, size);
            _devices[i]->bde_dev.base_address = (sal_vaddr_t)_devices[i]->vbase;
            }
            /* Check for additional device resources */
            devio.dev = i;
            devio.d0 = 1; /* Resource number */
            if (_ioctl(LUBDE_DEV_RESOURCE, &devio) == 0) {
                /* Get physical devices address */
#ifdef PHYS_ADDRS_ARE_64BITS
                pbase = devio.d3;
                pbase <<= 32;
                pbase |= devio.d2;
#else
                pbase = devio.d2;
#endif /* PHYS_ADDRS_ARE_64BITS */
#ifdef __DUNE_LINUX_BCM_CPU_PCIE__ 
            if (dev_type & BDE_256K_REG_SPACE) {
                size = 0x8000;
            }
#endif
            iproc_map_lock = sal_mutex_create("iproc_map_mutex");
            if (iproc_map_lock == NULL) {
                return -1;
            }

                if (pbase) {
                    _devices[i]->vbase1 = _mmap(pbase, size);  
                    if (!(dev_type & BDE_NO_IPROC)) {
                        /* Cache IMAP0 registers from the current PAXB */
                        for (j = 0; j < IPROC_SUBWIN_MAX; j++) {
                            rval = _devices[i]->vbase1[(BAR0_PAXB_IMAP0_0/sizeof(uint32))+j];
                            if (rval & 1) { /* Valid Bit */
                                iproc_map[i].subwin[j].addr_min = rval & ~(0xfff);
                                iproc_map[i].subwin[j].addr_max = rval | 0xfff;
                            } else {
                                iproc_map[i].subwin[j].addr_min = 0;
                                iproc_map[i].subwin[j].addr_max = 0;
                            }
                        }
                        if (iproc_map[i].subwin[2].addr_min  & 0x1000) {
                            /* PAXB_1 */
                            dev_type |= BDE_DEV_BUS_ALT;
                        }
                    }
                }
            }
        }
        _devices[i]->dev_type = dev_type;

#ifdef DUNE_GTO_I2C
        _devices[i]->dev_type |= BDE_I2C_DEV_TYPE;
        _devices[i]->dev_type &= ~BDE_PCI_DEV_TYPE;
        switch (_devices[i]->bde_dev.device) {
        case GEDI_DEVICE_ID:
            _devices[i]->i2c_addr = GFA_BI_I2C_PCP_DEVICE_ADDR;
            _devices[i]->i2c_access_type = CPU_I2C_ALEN_LONG_DLEN_LONG;
            break;

        case BCM88750_DEVICE_ID:
        case BCM88753_DEVICE_ID:
        case BCM88754_DEVICE_ID:
        case BCM88755_DEVICE_ID:
        case BCM88752_DEVICE_ID:    
            _devices[i]->i2c_addr = FE1600_CARD_I2C_FE1600_DEVICE_ADDR;
            _devices[i]->i2c_access_type = CPU_I2C_ALEN_WORD_DLEN_LONG;
            break;
        case JERICHO_DEVICE_ID:
        case BCM88670_DEVICE_ID:
        case BCM88671_DEVICE_ID:
        case BCM88671M_DEVICE_ID:
        case BCM88672_DEVICE_ID:
        case BCM88673_DEVICE_ID:
        case BCM88674_DEVICE_ID:
        case BCM88675M_DEVICE_ID:
        case BCM88676_DEVICE_ID:
        case BCM88676M_DEVICE_ID:
        case BCM88677_DEVICE_ID:
        case BCM88678_DEVICE_ID:
        case BCM88679_DEVICE_ID:
        case QMX_DEVICE_ID:
        case BCM88370_DEVICE_ID:
        case BCM88371_DEVICE_ID:
        case BCM88371M_DEVICE_ID:
        case BCM88376_DEVICE_ID:
        case BCM88376M_DEVICE_ID:
        case BCM88377_DEVICE_ID:
        case BCM88378_DEVICE_ID:
        case BCM88379_DEVICE_ID:
        case BCM88680_DEVICE_ID:
        case BCM88681_DEVICE_ID:
        case BCM88682_DEVICE_ID:
        case BCM88683_DEVICE_ID:
        case BCM88380_DEVICE_ID:
        case BCM88381_DEVICE_ID:
        case BCM88680_DEVICE_ID:
        case BCM88690_DEVICE_ID:
        case BCM88770_DEVICE_ID:
        case BCM88773_DEVICE_ID:
        case BCM88774_DEVICE_ID:
        case BCM88775_DEVICE_ID:
        case BCM88776_DEVICE_ID:
        case BCM88777_DEVICE_ID:
        case QAX_DEVICE_ID:
        case BCM88471_DEVICE_ID:
        case BCM88473_DEVICE_ID:
        case BCM88474_DEVICE_ID:
        case BCM88474H_DEVICE_ID:
        case BCM88476_DEVICE_ID:
        case BCM88477_DEVICE_ID:

        case QUX_DEVICE_ID:
        case BCM88272_DEVICE_ID:
        case BCM88273_DEVICE_ID:
        case BCM88278_DEVICE_ID:
        case FLAIR_DEVICE_ID:
        case BCM88950_DEVICE_ID:
        case BCM88953_DEVICE_ID:
        case BCM88954_DEVICE_ID:
        case BCM88955_DEVICE_ID:
        case BCM88956_DEVICE_ID:
        case BCM88772_DEVICE_ID:
        case BCM88952_DEVICE_ID:
        case BCM88790_DEVICE_ID:
            _devices[i]->i2c_addr = 0x44;
            _devices[i]->i2c_access_type = CPU_I2C_ALEN_LONG_DLEN_LONG;
            break;
        default:
            break;
        }
#endif
    }
    /* Initialize DMA memory pool */
    mpool_init();
    assert(_dma_vbase = _mmap(_dma_pbase, _dma_size));
    printf("DMA pool size: %d\n", _dma_size);
    assert(_dma_pool = mpool_create(_dma_vbase, _dma_size));

    /* calibrate */
    sal_udelay(0);

    return 0;
}

/* 
 * Function: _close
 *
 * Purpose:
 *    close the driver
 * Parameters:
 *    None
 * Returns:
 *    0
 */
static int
_close(void)
{       
    if (_memfd >= 0 && _memfd != _devfd) {
        close(_memfd);
    }
    close(_devfd);
    if (_kdevfd >=0) {
        close(_kdevfd);
    }
    _kdevfd = _memfd = _devfd = -1;
    return 0;
}

/*
 * Function: _enable_interrupts
 *
 * Purpose:
 *    Enable interrupts on all devices in the driver
 * Parameters:
 *    d - 0      the switching devices
 *        others the ether device id of _devices.
 * Returns:
 *    0
 * Notes:
 *    When a real device interrupt occurs, the driver masks all 
 *    interrupts and wakes up the interrupt thread.
 *    It is assumed that the interrupt handler will unmask 
 *    interrupts upon exit.
 */
static int
_enable_interrupts(int d)
{
    lubde_ioctl_t devio;
#ifndef NDEBUG
    /* "assert" maps to NULL statement with NDEBUG */
    int _ioctl_irrupt_status;
#endif /* !NDEBUG */

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    if (_switch_ndevices > 0 || _devices[d]->dev_type & BDE_ETHER_DEV_TYPE) {
        devio.dev = d;
        #ifndef NDEBUG
        _ioctl_irrupt_status =
        #endif
        _ioctl(LUBDE_ENABLE_INTERRUPTS, &devio);
        assert(_ioctl_irrupt_status == 0);
    }
    return 0;
}

/* 
 * Function: _disable_interrupts
 *
 * Purpose:
 *    Disable interrupts on all devices in the driver
 * Parameters:
 *    d - 0      the switching devices
 *        others the ether device id of _devices.
 * Returns:
 *    0
 */
static int
_disable_interrupts(int d)
{
    lubde_ioctl_t devio;
#ifndef NDEBUG
    /* "assert" maps to NULL statement with NDEBUG */
    int _ioctl_irrupt_status;
#endif /* !NDEBUG */

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    if (_switch_ndevices > 0 || _devices[d]->dev_type & BDE_ETHER_DEV_TYPE) {
        devio.dev = d;
        #ifndef NDEBUG
            _ioctl_irrupt_status =
        #endif
            _ioctl(LUBDE_DISABLE_INTERRUPTS, &devio);
            assert(_ioctl_irrupt_status == 0);
    }
    return 0;
}

/* 
 * Function: _pci_config_put32
 *
 * Purpose:
 *    Write a PCI configuration register on the device
 * Parameters:
 *    d - device number
 *    offset - register offset
 *    data - register data
 * Returns:
 *    0
 */
int 
_pci_config_put32(int d, unsigned int offset, unsigned int data)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.dev = d;
    devio.d0 = offset;
    devio.d1 = data;
    _ioctl(LUBDE_PCI_CONFIG_PUT32, &devio);
    return 0;
}

/* 
 * Function: _pci_config_get32
 *
 * Purpose:
 *    Read a PCI configuration register on the device
 * Parameters:
 *    d - device number
 *    offset - register offset
 * Returns:
 *    register value
 */
unsigned int 
_pci_config_get32(int d, unsigned int offset)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.dev = d;
    devio.d0 = offset;
    _ioctl(LUBDE_PCI_CONFIG_GET32, &devio);
    return devio.d0;
}

/*
 * Function: _read
 *
 * Purpose:
 *    Read a register
 * Parameters:
 *    d - device number
 *    addr - register address
 * Returns:
 *    register value
 */
static unsigned int 
_read(int d, unsigned int addr)
{
    lubde_ioctl_t _devio;
    unsigned int rv = 0;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

#if CMIC_SPI_SUPPORT
    if (_devices[d]->dev_type & BDE_SPI_DEV_TYPE) {
        return _cmic_spidev_read(d, addr);
    }
#endif

    if (_devices[d]->dev_type & BDE_DEV_BUS_RD_16BIT) {
        _devio.dev = d;
        _devio.d0 = addr;
        if (_ioctl(LUBDE_READ_REG_16BIT_BUS, &_devio) != 0) {
            printf("linux-user-bde: warning: "
                   "eb_read from kernel failed unit=%d addr=%x",
                   d, addr);
            return -1;
        }
        return _devio.d1;
    }

    if (_devices[d]->dev_type & BDE_I2C_DEV_TYPE) {
#ifdef DUNE_GTO_I2C
        int data = 0;
        assert(_devices[d]->i2c_addr);
        if (cpu_i2c_read(_devices[d]->i2c_addr, addr,
                         _devices[d]->i2c_access_type, &data) != 0) {
            printf("linux-user-bde: warning: "
                   "cpu_i2c_read failed unit=%d addr=0x%x ,"
                   "i2c_addr=0x%x , i2c_access_type=0x%x .\n",
                   d, addr, _devices[d]->i2c_addr , _devices[d]->i2c_access_type);
            return -1;
        }
        return data;
#endif
    }

    assert(_devices[d]->vbase);
    rv = _devices[d]->vbase[addr/sizeof(uint32)];
    if (_devices[d]->dev_type & BDE_BYTE_SWAP) {
        rv = _SWAP32(rv);
    }

    return rv;
}

/*
 * Function: _write
 *
 * Purpose:
 *    Write a register
 * Parameters:
 *    d - device number
 *    addr - register address
 *    data - register data
 * Returns:
 *    0
 */
static int
_write(int d, uint32 addr, uint32 data)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

#if CMIC_SPI_SUPPORT
    if (_devices[d]->dev_type & BDE_SPI_DEV_TYPE) {
        return _cmic_spidev_write(d, addr, data);
    }
#endif

    if (_devices[d]->dev_type & BDE_DEV_BUS_WR_16BIT) {
        _devio.dev = d;
        _devio.d0 = addr;
        _devio.d1 = data;
        if (_ioctl(LUBDE_WRITE_REG_16BIT_BUS, &_devio) != 0) {
            printf("linux-user-bde: warnning: "
                   "eb_write failed unit=%d addr=%x",
                   d, addr);
            return -1;
        }
        return 0;
    }

    if (_devices[d]->dev_type & BDE_I2C_DEV_TYPE) {
#ifdef DUNE_GTO_I2C
        assert(_devices[d]->i2c_addr);
        if (cpu_i2c_write(_devices[d]->i2c_addr, addr,
                         _devices[d]->i2c_access_type, data) != 0) {
            printf("linux-user-bde: warning: "
                   "cpu_i2c_write failed unit=%d addr=0x%x ,"
                   "i2c_addr=0x%x , i2c_access_type=0x%x .\n",
                   d, addr , _devices[d]->i2c_addr , _devices[d]->i2c_access_type);
            return -1;
        }
        return 0;
#endif
    }

    assert(_devices[d]->vbase);
    if (_devices[d]->dev_type & BDE_BYTE_SWAP) {
        data = _SWAP32(data);
    }
    _devices[d]->vbase[addr/sizeof(uint32)] = data;

    return 0;
}

/*
 * Function: _salloc
 *
 * Purpose:
 *    Allocate DMA memory
 * Parameters:
 *    d - device number
 *    size - size of block
 *    name - name of block (debugging, unused)
 * Returns:
 *    0
 */
static uint32*
_salloc(int d, int size, const char *name)
{
    /* All devices use the same dma memory pool */
    uint32 *tmp;
    tmp =  mpool_alloc(_dma_pool, size);
#if AGGRESSIVE_ALLOC_DEBUG_TESTING
    printf("DMA Alloc: 0x%x. Descriptor: %s.\n", (unsigned int)tmp, name);
#endif
    return tmp;
}

/* 
 * Function: _sfree
 *
 * Purpose:
 *    Free DMA memory
 * Parameters:
 *    d - device number
 *    ptr - ptr to memory to free
 * Returns:
 *    0
 */
static void
_sfree(int d, void *ptr)
{
#if AGGRESSIVE_ALLOC_DEBUG_TESTING
     printf("DMA Free: 0x%x.\n", (unsigned int)ptr);
#endif
    /* All devices use the same dma memory pool */
    mpool_free(_dma_pool, ptr);
}       


/*
 * Thread ID of the signal handler/interrupt thread
 */
static volatile sal_thread_t _intr_thread = NULL;

/*
 * Thread ID of the signal handler/interrupt thread for ethernet devices
 */
static volatile sal_thread_t _ether_intr_thread = NULL;

/*
 * Function: intr_int_context
 *
 * Purpose:
 *    Used by the linux kernel SAL to implement sal_int_context(). 
 * Parameters:
 *    None
 * Returns:
 *    When the current thread is the interrupt thread. 
 */
int intr_int_context(void) 
{
    return ((_intr_thread == sal_thread_self()) ||
            (_ether_intr_thread == sal_thread_self()));
}


/* 
 * Client Interrrupt Management
 */

typedef struct intr_handler_s {
    void *data;
    void (*handler)(void*);
} intr_handler_t;

/* For switching devices */
static intr_handler_t _handlers[LINUX_BDE_MAX_SWITCH_DEVICES];
static int _handler_max = -1;
static int _intr_thread_running = 0;

/* for ethernet devices */ 
static intr_handler_t _ether_handler;
static int _ether_dev_handler = 0; /* device id for ethernet device */

/*
 * Function: _run_intr_handlers
 *
 * Purpose:
 *    Run application level interrupt handlers.
 * Parameters:
 *    None
 * Returns:
 *    Nothing
 */
static void
_run_intr_handlers(void)
{
    int i;
    int spl;

    /* 
     * Protect applications threads from interrupt thread.
     */
    spl = sal_splhi();

    /*
     * Run all of the client interrupt handlers
     */
    for (i = 0; i <= _handler_max; i++) {
        if (_handlers[i].handler) {
            _handlers[i].handler(_handlers[i].data);
        }
    }

    /* 
     * Restore spl
     */
    sal_spl(spl);
}
static void
_run_ether_intr_handlers(void)
{
    int spl;

    /* 
     * Protect applications threads from interrupt thread.
     */
    spl = sal_splhi();

    /*
     * Run all of the client interrupt handlers
     */
    
    _ether_handler.handler(_ether_handler.data);
    

    /* 
     * Restore spl
     */
    sal_spl(spl);
}

#ifdef SAL_BDE_THREAD_PRIO_DEFAULT
#else
/*
 * Function: _set_thread_priority
 *
 * Purpose:
 *    Raise the current thread's execution priority
 *    from regular to realtime. 
 * Parameters:
 *    prio - The realtime scheduling priority (0 - 99)
 * Returns:
 *    Nothing
 */
static void
_set_thread_priority(int prio)
{
    struct sched_param param;
    param.sched_priority = prio;
    if (sched_setscheduler(0, SCHED_RR, &param)) {       
        perror("\ninterrupt priority set: ");
    }
}       
#endif /* SAL_BDE_THREAD_PRIO_DEFAULT */


/*
 * Function: _interrupt_thread
 *
 * Purpose:
 *    Provides a thread context for interrupt handling. 
 * Parameters:
 *    context - unused
 * Returns:
 *    Nothing
 * Notes:
 */
static void
_interrupt_thread(void *d)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.dev = PTR_TO_INT(d);

    /* We are the interrupt thread for intr_int_context() */
    _intr_thread = sal_thread_self();

#ifdef SAL_BDE_THREAD_PRIO_DEFAULT
#else
    /* Increase our priority */
    _set_thread_priority(90);
#endif /* SAL_BDE_THREAD_PRIO_DEFAULT */

    while (_intr_thread_running) {
        devio.dev = PTR_TO_INT(d);
        _ioctl(LUBDE_WAIT_FOR_INTERRUPT, &devio);
        _run_intr_handlers();
    }
}

/*
 * Function:
 *    _ether_interrupt_thread
 * Purpose:
 *    Provides a thread context for ethernet interrupt handling. 
 *     Interrupts are signaled, it provides the 
 *    unique context needed by the signal handler. 
 * Parameters:
 *    d - the device id in _devices
 * Returns:
 *    Nothing
 * Notes:
 */

static void
_ether_interrupt_thread(void* d)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.dev = PTR_TO_INT(d);

    /* We are the interrupt thread for intr_int_context() */
    _ether_intr_thread = sal_thread_self();
    
#ifdef SAL_BDE_THREAD_PRIO_DEFAULT
#else
    /* Increase our priority */
    _set_thread_priority(90);
#endif /* SAL_BDE_THREAD_PRIO_DEFAULT */

    for (;;) {
        devio.dev = PTR_TO_INT(d);
        _ioctl(LUBDE_WAIT_FOR_INTERRUPT, &devio);
        _run_ether_intr_handlers();
    }
}

/*
 * Function:
 *    _interrupt_connect
 * Purpose:
 *    BDE vector for connecting client interrupts to the device. 
 *    Initialized the SW interrupt controller and registers the handler. 
 * Parameters:
 *    d - device number
 *    handler - client interrupt handler
 *    data    - client interrupt handler data
 * Returns:
 *    0
 */
static int 
_interrupt_connect(int d, 
                   void (*handler)(void*),
                   void *data)
{
    if (d < 0 || d >= COUNTOF(_devices)) {
        return -1;
    }

    if (_devices[d]->dev_type & BDE_ETHER_DEV_TYPE) {
        _ether_dev_handler = d;
        _ether_handler.handler = handler;
        _ether_handler.data = data;

        /* Create the ethernet interrupt thread */
        sal_thread_create("bcmEthINTR",
                          8096, 0,
                          (void (*)(void*))_ether_interrupt_thread,
                          INT_TO_PTR(d));
        /* Enable interrupts on the device */
        _enable_interrupts(d);

        return 0;
    }

    if (d >= COUNTOF(_handlers)) {
        return -1;
    }

    /* Do not process more devices than necessary */
    if (d > _handler_max) {
        _handler_max = d;
    }

    _handlers[d].handler = handler;
    _handlers[d].data = data;
    
    /*
     * Start up interrupt processing if this is the first connect
     */
    if (_intr_thread_running == 0) {
        _intr_thread_running = 1;
        sal_thread_create("bcmINTR",
                          8096, 0,
                          (void (*)(void*))_interrupt_thread,
                          INT_TO_PTR(d)); 
    }

    /* Enable interrupts on the device */
    _enable_interrupts(d);

    return 0;
}

/*
 * Function: _interrupt_disconnect
 *
 * Purpose:
 *    BDE interrupt disconnect function
 * Parameters:
 *    d - device number
 * Returns:
 *    0
 */
static int 
_interrupt_disconnect(int d)
{
    int spl;

    if (d < 0 || d >= COUNTOF(_devices)) {
        return -1;
    }

    /* Ethernet devices */
    if (d >= _switch_ndevices && d < _ndevices) {
        _disable_interrupts(d);
        return 0;
    }

    if (d >= COUNTOF(_handlers)) {
        return -1;
    }

    /* Switch devices */
    if (d >= 0 && d < _switch_ndevices) {
        _disable_interrupts(d);
        spl = sal_splhi();
        _handlers[d].handler = NULL;
        _handlers[d].data = NULL;
        sal_spl(spl);
    }
    return 0;
}

/*
 * Function: _l2p
 *
 * Purpose:
 *    BDE l2p function. Converts CPU virtual DMA addresses
 *    to Device Physical DMA addresses. 
 * Parameters:
 *    d - device number
 *    laddr - logical address to convert
 * Returns:
 *    Physical address
 */
static uint32
_l2p(int d, void *laddr)
{
    /* DMA memory is one contiguous block */
    uint32 pbase = (uint32)_dma_pbase;
    sal_vaddr_t vbase = PTR_TO_UINTPTR(_dma_vbase);
    sal_vaddr_t addr = PTR_TO_UINTPTR(laddr);

    uint32 rc;

    if (!laddr) return 0;

    rc = pbase + (uint32)(addr-vbase);

    return rc;
}       

/* 
 * Function: _p2l
 *
 * Purpose:
 *    BDE p2l function. Converts Device Physical DMA addresses
 *    to CPU virtual DMA addresses. 
 * Parameters:
 *    d - device number
 *    paddr - physical address to convert
 * Returns:
 *    Virtual address
 */
static uint32* 
_p2l(int d, uint32 paddr)
{
    /* DMA memory is one contiguous block */
    uint32 pbase = (uint32)_dma_pbase;
    sal_vaddr_t vbase = PTR_TO_UINTPTR(_dma_vbase);
    sal_vaddr_t rc;

    if (!paddr) return NULL;

    rc = vbase + (paddr-pbase);

    return (uint32*)UINTPTR_TO_PTR(rc);
}

static uint32
_iproc_offset(int d, uint32 addr)
{
    _iproc_map_t *map;
    _iproc_subwin_t *subwin;
    volatile uint32 *bar0, *pv;
    int idx;

    if (d >= LINUX_BDE_MAX_DEVICES) {
        return 0;
    }

    map = &iproc_map[d];
    for (idx = 0; idx < IPROC_SUBWIN_MAX; idx++) {
        subwin = &map->subwin[idx];
        if (addr >= subwin->addr_min && addr <= subwin->addr_max) {
            return subwin->addr_min - (idx * 0x1000);
        }
    }

    /* Not found. (Re)use the default Subwindow */
    subwin = &map->subwin[IPROC_DEFAULT_SUBWIN];
    if (addr < subwin->addr_min || addr > subwin->addr_max) {
        subwin->addr_min = addr & ~(0xfff);
        subwin->addr_max = addr | 0xfff;

        bar0 = (volatile uint32 *)_devices[d]->vbase1;
        pv = &bar0[BAR0_PAXB_IMAP0_0 / sizeof(uint32)];
        pv[IPROC_DEFAULT_SUBWIN] = (subwin->addr_min | 0x1);

        /* Read back IMAP register to ensure the write completes before proceeding */
        if (pv[IPROC_DEFAULT_SUBWIN] != (subwin->addr_min | 0x1)) {
            return 0;
        }
    }
    return subwin->addr_min - (IPROC_DEFAULT_SUBWIN * 0x1000);
}


static uint32
_iproc_ihost_read(int d, uint32 addr)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

    _devio.dev = d;
    _devio.d0 = addr;
    if (_ioctl(LUBDE_IPROC_READ_REG, &_devio) != 0) {
        printf("BDE iproc_read failed at 0x%08x\n", addr);
        return -1;
    }

    return _devio.d1;
}

static int
_iproc_ihost_write(int d, uint32 addr, uint32 data)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = data;

    if (_ioctl(LUBDE_IPROC_WRITE_REG, &_devio) != 0) {
        printf("BDE iproc_write failed at 0x%08x\n", addr);
        return -1;
    }

    return 0;
}

/*
 * Function: _iproc_read
 *
 * Purpose:
 *    Read an iProc register
 * Parameters:
 *    d - device number
 *    addr - register address
 * Returns:
 *    register value
 */
static unsigned int 
_iproc_read(int d, unsigned int addr)
{
    uint32 offset;
    uint32 rval;

    if (_devices[d]->dev_type & BDE_AXI_DEV_TYPE) {
        return _iproc_ihost_read(d, addr);
    }
    sal_mutex_take(iproc_map_lock, sal_mutex_FOREVER);

    if (!(_devices[d]->dev_type & BDE_NO_IPROC)) {
        if ((offset = _iproc_offset(d, addr)) == 0) {
            sal_mutex_give(iproc_map_lock);
            return -1;
        }
        addr -= offset;
    }

    assert(_devices[d]->vbase1);
    rval = _devices[d]->vbase1[addr/sizeof(uint32)];
    sal_mutex_give(iproc_map_lock);
    return rval;
}

/*
 * Function: _iproc_write
 *
 * Purpose:
 *    Write an iProc register
 * Parameters:
 *    d - device number
 *    addr - register address
 *    data - register data
 * Returns:
 *    0
 */
static int
_iproc_write(int d, uint32 addr, uint32 data)
{
    uint32 offset;

    if (_devices[d]->dev_type & BDE_AXI_DEV_TYPE) {
        return _iproc_ihost_write(d, addr, data);
    }
    sal_mutex_take(iproc_map_lock, sal_mutex_FOREVER);

    if (!(_devices[d]->dev_type & BDE_NO_IPROC)) {
        if ((offset = _iproc_offset(d, addr)) == 0) {
            sal_mutex_give(iproc_map_lock);
            return -1;
        }
        addr -= offset;
    }

    assert(_devices[d]->vbase1);
    _devices[d]->vbase1[addr/sizeof(uint32)] = data;

    sal_mutex_give(iproc_map_lock);

    return 0;
}

#if LINUX_SPIDEV_SUPPORT /* Linux spidev driver for switch register access */

#ifdef BCM_ROBO_SUPPORT
/* 
 * Function:
 *    _spi_dev_read
 * Purpose:
 *    Issue to send spi read message for Robo devices.
 * Parameters:
 *    dev - device number
 *    reg - register addr to read
 *    buf - (out) buffer addr to store the reading result
 *    len - number bytes to read
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_spi_dev_read(int dev, int reg, uint8 *buf, int len)
{    
    int i;
    int ret;    
    int fd = _sys_devs[dev].spi_fd;

    uint8 tx[2];
    uint8 rx[8];
    struct spi_ioc_transfer tr[2];

    memset(&rx, 0, ARRAY_SIZE(rx) * sizeof(uint8));

    tx[0] = 0x60;
    tx[1] = (uint8)reg;

    memset(tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx;
    tr[0].len = ARRAY_SIZE(tx);

    tr[1].rx_buf = (unsigned long)rx;
    tr[1].len = len;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
    if (ret == 1) {
        printf("Can't send spi message: read(ret=%d)\n", ret);
    }
    for (i = 0; i < len; i++) {
        *(buf + i) = rx[i];
    }
    return ret;
}

/* 
 * Function:
 *    _spi_dev_write
 * Purpose:
 *    Issue to send spi write message for Robo devices.
 * Parameters:
 *    dev - device number
 *    reg - register addr to write
 *    buf - buffer to write to spi
 *    len - number bytes to write
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_spi_dev_write(int dev, int reg, const uint8 *buf, int len)
{    
    int i;
    int ret;    
    int fd = _sys_devs[dev].spi_fd;
    uint8 tx1[2];
    uint8 tx2[8];
    
    struct spi_ioc_transfer tr[2];

    memset(tx2, 0, ARRAY_SIZE(tx2) * sizeof(uint8));
    for (i = 0; i < len; i++) {
        tx2[i] = *(buf+i);
    }

    tx1[0] = 0x61;
    tx1[1] = (uint8)reg;
    
    memset(&tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx1;
    tr[0].len = ARRAY_SIZE(tx1);

    tr[1].tx_buf = (unsigned long)tx2;
    tr[1].len = len;

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), &tr);
    if (ret == 1) {
        printf("Can't send spi message: write(ret=%d)\n", ret);
    }

    return ret;
}

/* 
 * Function:
 *    _spi_dev_poll_for_spif
 * Purpose:
 *    Polling the SPI status of device for SPIF (bit 7).
 * Parameters:
 *    dev - device number
 * Returns:
 *    0 - Success
 *   -1 - Timeout
 */
static int
_spi_dev_poll_for_spif(int dev)
{
    int timeout;
    uint8 byte;

    for (timeout = DEFAULT_SPI_TIMEOUT; timeout;) {
        _spi_dev_read(dev, 0xfe, &byte, 1);
        /* SPIF is bit 7 of SPI_STS */
        /* check SPIF = 0 ? */
        if (!(byte & 0x80)) {
            break;
        } else {
            timeout--;
        }
    }

    if (timeout == 0) {
        return -1;
    }
    return 0;
}

/* 
 * Function:
 *    _spi_dev_poll_for_rack
 * Purpose:
 *    Polling the SPI status of device for rack (bit 5).
 * Parameters:
 *    dev - device number
 * Returns:
 *    0 - Success
 *   -1 - Timeout
 */
static int
_spi_dev_poll_for_rack(int dev)
{
    int timeout;
    uint8 byte;

    for (timeout = DEFAULT_SPI_TIMEOUT; timeout;) {
        _spi_dev_read(dev, 0xfe, &byte, 1);
         /*  In normal read mode */
         /*  check bit 5 are high */
        if (byte & 0x20) {
            break;
        } else {
            timeout--;
        }
    }

    if (timeout == 0) {
        return -1;
    }
    return 0;
}

/* 
 * Function:
 *    _spi_dev_select
 * Purpose:
  *    Issue to send spi write message for Robo register's page selection.
 * Parameters:
 *    dev - device number
 *    page - page address
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_spi_dev_select(int dev, int page)
{
    int ret;
    int fd = _sys_devs[dev].spi_fd;

    uint8 tx[3];
    struct spi_ioc_transfer tr[1];

    tx[0] = 0x61;
    tx[1] = 0xff;
    tx[2] = (uint8)page;

    memset(&tr, 0, sizeof(tr));
    tr[0].tx_buf = (unsigned long)tx;
    tr[0].len = ARRAY_SIZE(tx);

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret == 1) {
        printf("Can't send spi message: select(ret=%d)\n", ret);
    }
    return ret;
}

/* 
 * Function:
 *    _sys_spi_read
 * Purpose:
 *    Linux spidev read function. Issue spi read via Linux spidev driver.
 * Parameters:
 *    dev - device number
 *    addr - register addr to read
 *    data - (out) data addr to store the reading result
 *    len - number bytes to read
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_sys_spi_read(int dev, uint32 addr, uint8 *data, int len)
{
    uint8 page;
    uint8 offset;
    uint8 byte;

    page = (addr >> 8) & 0xff;
    offset = addr & 0xff;

    /* Poll for SPIF */
    if (_spi_dev_poll_for_spif(dev) < 0) {
        /* timeout */
        if (spi_log_on) {
            printf("SPI poll for SPIF: timeout\n");
        }
        return -1;
    }

    /* Select chip and page */
    if (_spi_dev_select(dev, page) < 0) {
        if (spi_log_on) {
            printf("SPI select chip and page failed\n");
        }
        return -1;
    }

    /* Normal read. Discard first read */
    if (_spi_dev_read(dev, offset, &byte, 1) < 0) {
        if (spi_log_on) {
            printf("SPI device read failed\n");
        }
        return -1;
    }

    /* Poll for RACK */
    if (_spi_dev_poll_for_rack(dev) < 0) {
        /* timeout */
        if (spi_log_on) {
            printf("SPI poll for RACK: timeout\n");
        }
        return -1;
    }

    /* Read register from dataport */
    if (_spi_dev_read(dev, 0xf0, data, len) < 0) {
        if (spi_log_on) {
            printf("SPI device read failed\n");
        }
        return -1;
    }

    return 0;
}

/* 
 * Function:
 *    _sys_spi_write
 * Purpose:
 *    Linux spidev write function. Issue spi write via Linux spidev driver.
 * Parameters:
 *    dev - device number
 *    addr - register addr to write
 *    data - data buffer to write to spi
 *    len - number bytes to write
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_sys_spi_write(int dev, uint32 addr, const uint8 *data, int len)
{
    uint8 page;
    uint8 offset;

    page = (addr >> 8) & 0xff;
    offset = addr & 0xff;

    /* Poll for SPIF */
    if (_spi_dev_poll_for_spif(dev) < 0) {
        /* timeout */
        if (spi_log_on) {
            printf("SPI poll for SPIF: timeout\n");
        }
        return -1;
    }

    /* Select chip and page */
    if (_spi_dev_select(dev, page) < 0) {
        if (spi_log_on) {
            printf("SPI select chip and page failed\n");
        }
        return -1;
    }

    /* Write register */
    if (_spi_dev_write(dev, offset, data, len) < 0) {
        if (spi_log_on) {
            printf("SPI device write failed\n");
        }
        return -1;
    }

    return 0;
}

/* 
 * Function:
 *    _sys_spi_dev_init
 * Purpose:
 *    Initialize to setup linux spidev driver
 * Parameters:
 *    dev - device number
 *    fd - SPI device descriptor
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static int
_sys_spi_dev_init(int dev, int fd)
{
    uint8 mode;
    uint32 speed;

    _sys_devs[dev].unit = fd;
    _sys_devs[dev].spi_fd = fd;

#if defined(IPROC_CMICD) && defined(BCM_STARFIGHTER3_SUPPORT)
    mode = SPI_MODE_0;
#else
    mode = SPI_MODE_3; /* SPI_CPOL|SPI_CPHA */
#endif

    if (ioctl(fd, SPI_IOC_WR_MODE, &mode) < 0) {
        printf("Can not set mode for SPI device %d.\n", dev);
        close(fd);
        return -1;
    }
    _sys_devs[dev].spi_mode = mode;

    if (ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed) < 0) {
        printf("Can not get max speed for SPI device %d.\n", dev);
        close(fd);
        return -1;
    }

    if (speed != USER_SPI_FREQ_DEFAULT) {
        speed = USER_SPI_FREQ_DEFAULT;
        if (ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) < 0) {
            printf("Can not set max speed %dHz for SPI device %d.\n", speed, dev);
            close(fd);
            return -1;
        }
    }
    _sys_devs[dev].spi_speed = speed;

    _sys_devs[dev].spi_bits = 0;
    _sys_devs[dev].spi_delay = 0;

    return 0;
}

/* 
 * Function:
 *    _sys_spi_dev_deinit
 * Purpose:
 *    close linux spidev driver device file
 * Parameters:
 *    dev - device number
 * Returns:
 *    void
 */
static void
_sys_spi_dev_deinit(int dev)
{
    int fd;

    fd = _sys_devs[dev].spi_fd;
    close(fd);
}

/* 
 * Function:
 *    _sys_spi_dev_check
 * Purpose:
 *    Check the device ID (phyid_low) is valid or not (compare with the result of Kernel BDE)
 * Parameters:
 *    phyidl_nr - phy id low without revision
 * Returns:
 *    0 - Found valid SPI devices
 *    1 - No valid SPI devices found
 */
static int
_sys_spi_dev_check(int dev, unsigned short phyidl_nr)
{
    int ndevs;
    uint8 part_8 = 0;
    uint16 part_16 = 0;
    uint32 part_32 = 0;
    uint64 part_64 = 0;

    if (phyidl_nr == BCM5348_PHYID_LOW) {
        _sys_spi_read(dev, 0x0388, (uint8 *)&part_8, 1);
        part_16 = part_8;
    } else if ((phyidl_nr == BCM53242_PHYID_LOW) ||
               (phyidl_nr == BCM53262_PHYID_LOW)) {
        part_16 = 0;
    } else if ((phyidl_nr == BCM53115_PHYID_LOW) ||
               (phyidl_nr == BCM53118_PHYID_LOW) ||
               (phyidl_nr == BCM53101_PHYID_LOW) ||
               (phyidl_nr == BCM53125_PHYID_LOW) ||
               (phyidl_nr == BCM53128_PHYID_LOW) ||
               (phyidl_nr == BCM53134_PHYID_LOW) ||
               (phyidl_nr == BCM89500_PHYID_LOW)) {
        /* Register Model ID len = 4 for BCM53115, BCM53118 */
        _sys_spi_read(dev, 0x0230, (uint8 *)&part_32, 4);
#ifdef BE_HOST
        part_32 = (((part_32 >> 24) & 0x000000FF) |
                   ((part_32 >> 8)  & 0x0000FF00) |
                   ((part_32 << 8)  & 0x00FF0000) |
                   ((part_32 << 24) & 0xFF000000));
#endif
        part_16 = (part_32 & 0x0000FFFF);
    } else if ((phyidl_nr == BCM53280_PHYID_LOW) ||
               (phyidl_nr == BCM53600_PHYID_LOW)) {
        _sys_spi_read(dev, 0x00e8, (uint8 *)&part_64, 8);
        part_8 =  *(uint8 *)&part_64;
        part_16 = part_8;
    } else {
        _sys_spi_read(dev, 0x0230, (uint8 *)&part_8, 1);
        part_16 = part_8;
    }

    for (ndevs = 0; ndevs < _ndevices; ndevs++) {
        if ((_devices[ndevs]->dev_type & BDE_SPI_DEV_TYPE) &&
            (_devices[ndevs]->dev_type & BDE_SWITCH_DEV_TYPE)) {
            if ((phyidl_nr == (_devices[ndevs]->bde_dev.device & 0xfff0)) ||
                (part_16 == _devices[ndevs]->bde_dev.device)) {
                return 0;
            }
        }
    }

    /* No valid SPI devices found */
    return 1;
}

static int
linux_spidev_setup(void)
{
    int rv = 0;
    char *spi_device_name = LUSPI_DEVICE0_NAME;
    int i = 0, fd = 0;
    int ndevs = 0, spi_freq = 0;
    unsigned short phyidl = 0, phyidl_nr = 0;
    int spi_freq_override;         /* Used for user overrided SPI frequency */
    char *spi_freq_str = NULL;


    /* Initialization */
    memset(spi_dev_linux, 0, sizeof(spi_dev_linux));

    /* 
     * if (spi_freq_str != NULL): try to use linux spidev bus driver
     * else (spi_freq_str == NULL): use kernel BDE bus decision
     */
#ifndef NO_SAL_APPL
    spi_freq_str = sal_config_get("spi_freq_override");
#endif

    if (spi_freq_str != NULL) {
        /* For Keystone, maximum three spi devices */
        for (i = 0; i < 3; i++) {
            /* Loop to open the spi driver device file: /dev/spidev0.x */
            switch (i) {
                case 0:
                    spi_device_name = LUSPI_DEVICE0_NAME;
                    break;
                case 1:
                    spi_device_name = LUSPI_DEVICE1_NAME;
                    break;
                case 2:
                    spi_device_name = LUSPI_DEVICE2_NAME;
                    break;
            }

            /* Open the device file */
            fd = open(spi_device_name, O_RDWR);
            if (fd >= 0) {
                /* Open successful and then initialize to setup the SPI device */
                if (_sys_spi_dev_init(ndevs, fd) == 0) {
                    /* Setup successful */

                    /* Try to read device phyid low by Linux spidev driver */
                    rv = _sys_spi_read(ndevs, 0x1006, (uint8 *)&phyidl, 2);
                    if (rv < 0) {
                         /* Read failed for Linux spidev driver */
                        _sys_spi_dev_deinit(ndevs);
                    } else {
                        /* Read successful and check if it is a valid SPI device */
#ifdef BE_HOST
                        phyidl = (phyidl >> 8) | (phyidl << 8);
#endif
                        /* Strip revision */
                        phyidl_nr = phyidl & 0xfff0;

                        rv = _sys_spi_dev_check(ndevs, phyidl_nr);
                        if (rv == 0) {
                            /* Valid SPI device */
                            /* Thunderbolt and Lotus can support SPI Frequency up to 20MHz */
                            if ((phyidl_nr == BCM53280_PHYID_LOW) ||
                                (phyidl_nr == BCM53101_PHYID_LOW) ||
                                (phyidl_nr == BCM53125_PHYID_LOW) || 
                                (phyidl_nr == BCM53128_PHYID_LOW) ||
                                (phyidl_nr == BCM53134_PHYID_LOW) || 
                                (phyidl_nr == BCM53600_PHYID_LOW) ||
                                (phyidl_nr == BCM89500_PHYID_LOW)) {
                                spi_freq = USER_SPI_FREQ_20MHZ;
                            } else {
                                spi_freq = USER_SPI_FREQ_DEFAULT;
                            }

                            /* 
                             * if (spi_freq_override > 0) Override the SPI frequency from user
                             * else (spi_freq_override <= 0) Use device probed SPI frequency
                             */
                            if ((spi_freq_override = sal_ctoi(spi_freq_str, 0)) > 0) {
                                spi_freq = spi_freq_override;
                            }

                            rv = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_freq);
                            if (rv < 0) {
                                _sys_spi_dev_deinit(ndevs);
                            } else {
                                _sys_devs[ndevs].spi_speed = (uint32)spi_freq;
                                /* Valid SPI device for linux spidev driver */
                                spi_dev_linux[ndevs] = 1;
                                ndevs++;
                            }
                        } else {
                             /* Not a valid SPI device */
                            _sys_spi_dev_deinit(ndevs);
                        }
                    }
                }
            }
            if (ndevs >= _switch_ndevices) {
                break;
            }
        }
    }
    spi_log_on = 1;

    return 0;

}
#endif /* BCM_ROBO_SUPPORT */

#endif /* LINUX_SPIDEV_SUPPORT */


#if LINUX_MDIODEV_SUPPORT 
/* Linux mdio device driver for external PHY access */

#ifdef BCM_ROBO_SUPPORT
static int 
_mdio_cdev_read(int dev, uint32 addr, uint8 *data, int len)
{
    int ret;
    struct mdio_ioc_transfer ioc[1];
    int fd = _sys_mdiodevs[dev].mdio_fd;

    if (fd < 0) {
        return 1;
    }
    memset(ioc, 0, sizeof(ioc));
    ioc[0].pa = (addr >> 8) & 0xff;
    ioc[0].ra = addr & 0xff;

    ret = ioctl(fd, MDIO_IOC_EXTERNAL_R_REG, &ioc);
    if ((ret == 1) && (mdio_log_on)) {
        printf("Failed to read via CPU MDIO interface.\n");
        return ret;
    }
    memcpy(data, &ioc[0].rx_buf, sizeof(ioc[0].rx_buf));
    return ret;
}

static int
_mdio_cdev_write(int dev, uint32 addr, const uint8 *data, int len)
{
    int ret;    
    struct mdio_ioc_transfer ioc[1];
    int fd = _sys_mdiodevs[dev].mdio_fd;

    if (fd < 0) {
        return 1;
    }

    ioc[0].pa = (addr >> 8) & 0xff;
    ioc[0].ra = addr & 0xff;
    memcpy(&ioc[0].tx_buf, data, sizeof(ioc[0].tx_buf));

    ret = ioctl(fd, MDIO_IOC_EXTERNAL_W_REG, &ioc);
    if ((ret == 1) && (mdio_log_on)) {
        printf("Failed to write via CPU MDIO interface.\n");
    }
    return ret;    
}

/* 
 * Function:
 *    _sys_mdio_dev_init
 * Purpose:
 *    Initialize setup
 * Parameters:
 *    dev - device number
 *    fd - MDIO device descriptor
 * Returns:
 *    0 - Success
 *   -1 - Failed
 */
static void
_sys_mdio_dev_init(int dev, int fd)
{

    _sys_mdiodevs[dev].unit = dev;
    _sys_mdiodevs[dev].mdio_fd = fd;

    return;
}

/* 
 * Function:
 *    _sys_mdio_dev_deinit
 * Purpose:
 *    close linux mdio dev driver device file
 * Parameters:
 *    dev - device number
 * Returns:
 *    void
 */
static void
_sys_mdio_dev_deinit(int dev)
{
    int fd;

    fd = _sys_mdiodevs[dev].mdio_fd;
    close(fd);
}

static int
linux_mdiodev_setup(void)
{
    char *mdio_device_name = LINUX_USER_MDIODEV_NAME;
    int fd = 0;
    int ndevs = 0;


    /* Initialization */
    memset(mdio_dev_linux, 0, sizeof(mdio_dev_linux));

    fd = open(mdio_device_name, O_RDWR);
    if (fd >= 0) {
        _sys_mdio_dev_init(ndevs, fd);
        mdio_dev_linux[ndevs] = 1;
    }
    
    mdio_log_on = 1;

    return 0;

}
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_MDIODEV_SUPPORT */

/* 
 * Function:
 *    _spi_read
 * Purpose:
 *    BDE spi_read function. Issue spi read via SMP
 * Parameters:
 *    d - device number
 *    addr - register addr to read
 *    buf - buffer addr to store the reading result
 *    int - number bytes to read
 * Returns:
 *    0 - Success
 *     -1 - Failed
 */

static int
_spi_read(int d, uint32 addr, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    if (spi_dev_linux[d]) {
        return _sys_spi_read(d, addr, buf, len);
    }
#endif /* BCM_ROBO_SUPPORT */    
#endif /* LINUX_SPIDEV_SUPPORT */

#if LINUX_MDIODEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    if (addr & SOC_EXTERNAL_PHY_BUS_CPUMDIO) {
        if (mdio_dev_linux[d]) {
            return _mdio_cdev_read(d, addr, buf, len);
        }
    }
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_MDIODEV_SUPPORT */

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = len;
    memset(_devio.dx.buf, 0, sizeof(_devio.dx.buf));

    if (_ioctl(LUBDE_SPI_READ_REG, &_devio) != 0) {
    printf("linux-user-bde: warnning: spi_read failed \
        unit=%d addr=%x, len=%d", d, addr, len);
        return -1;
    }

    memcpy(buf, _devio.dx.buf, len);

    return 0;
}

/* 
 * Function:
 *    _spi_write
 * Purpose:
 *    BDE spi_write function. Issue spi read via SMP
 * Parameters:
 *    d - device number
 *    addr - register addr to write
 *    buf - buffer to write to spi
 *    int - number bytes for write
 * Returns:
 *    0 - Success
 *     -1 - Failed
 */

static int
_spi_write(int d, uint32 addr, uint8 *buf, int len)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    if (spi_dev_linux[d]) {
        return _sys_spi_write(d, addr, buf, len);
    }
#endif /* BCM_ROBO_SUPPORT */    
#endif /* LINUX_SPIDEV_SUPPORT */

#if LINUX_MDIODEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT

    if (addr & SOC_EXTERNAL_PHY_BUS_CPUMDIO) {
        if (mdio_dev_linux[d]) {
            return _mdio_cdev_write(d, addr, buf, len);
        }
    }
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_MDIODEV_SUPPORT */

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = len;
    memcpy(_devio.dx.buf, buf, len);

    if (_ioctl(LUBDE_SPI_WRITE_REG, &_devio) != 0) {
    printf("linux-user-bde: warnning: spi_write failed \
        unit=%d addr=%x, len=%d", d, addr, len);
        return -1;
    }

    return 0;
}

#if defined(BCM_PETRA_SUPPORT) || defined(BCM_DFE_SUPPORT)
int
_cpu_write(int d, uint32 addr, uint32 *buf)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = sizeof(uint32);
    memcpy(_devio.dx.buf, buf, sizeof(uint32));

    if (_ioctl(LUBDE_CPU_WRITE_REG, &_devio) != 0) {
        printf("linux-user-bde: warnning: _cpu_write failed unit=%d addr=0x%x, buf=%p, *buf=0x%x", d, addr, (void*)buf, *buf);
        return -1;
    }

    return 0;
}

int
_cpu_read(int d, uint32 addr, uint32 *buf)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

    _devio.dev = d;
    _devio.d0 = addr;
    _devio.d1 = sizeof(uint32);
    memset(_devio.dx.buf, 0, sizeof(_devio.dx.buf));

    if (_ioctl(LUBDE_CPU_READ_REG, &_devio) != 0) {
        printf("linux-user-bde: warnning: _cpu_read failed unit=%d addr=%x", d, addr);
        return -1;
    }

    memcpy(buf, _devio.dx.buf, sizeof(uint32));

    return 0;
}

int
_cpu_pci_register(int d)
{
    lubde_ioctl_t _devio;

    /* Initialize the variable */
    memset(&_devio, 0, sizeof(lubde_ioctl_t));

    _devio.dev = d;

    if (_ioctl(LUBDE_CPU_PCI_REGISTER, &_devio) != 0) {
        printf("linux-user-bde: warnning: _cpu_pci_register failed unit=%d\n", d);
        return -1;
    }

    return 0;
}

#endif

/* 
 * Function:
 *    _num_devices
 * Purpose:
 *    BDE num_devices function. Returns the number of devices. 
 * Parameters:
 *    None
 * Returns:
 *    number of devices in this bde
 */
static int
_num_devices(int type)
{
    switch (type) {
    case BDE_ALL_DEVICES:
        return _ndevices;
    case BDE_SWITCH_DEVICES:
        return _switch_ndevices;
    case BDE_ETHER_DEVICES:
        return _ether_ndevices;
    case BDE_CPU_DEVICES:
        return _cpu_ndevices;
    }
    return 0;
}

/* 
 * Function: _get_dev
 *
 * Purpose:
 *    BDE get_dev function. Returns device information structure. 
 * Parameters:
 *    d - device number
 * Returns:
 *    const pointer to BDE device information structure.
 */
static const ibde_dev_t*
_get_dev(int d)
{
    assert(d >= 0 || d < _ndevices);
    return &_devices[d]->bde_dev;
}

/* 
 * Function:
 *     _get_dev_type
 * Purpose:
 *    BDE get_dev_type function. Returns device type of BUS(PCI,SPI)/
 *    FUNCTIONALITY(SWITCH/ETHERNET). 
 * Parameters:
 *    d - device number
 * Returns:
 *    unsigned dword ORed with capablities of underlaying device.
 */
static uint32
_get_dev_type(int d)
{
    assert(d >= 0 || d < _ndevices);
    return _devices[d]->dev_type;
}

/* 
 * Function:
 *    _name
 * Purpose:
 *    BDE name function. Returns the name of the BDE. 
 * Parameters:
 *    None
 * Returns:
 *    Name of this BDE
 */
static const char*
_name(void)
{
    return LINUX_USER_BDE_NAME;
}


/* 
 * Function: _bus_features
 *
 * Purpose:
 *    BDE bus features function. Returns the endian features of the system bus. 
 * Parameters:
 *    be_pio - (out) returns the big endian pio bit. 
 *    be_packet - (out) returns the big endian packet bit. 
 *    be_other - (out) returns the big endian other bit. 
 * Returns:
 *    nothing
 * Notes:
 *    This just uses the values passed in by the BDE creator. 
 *    See linux_bde_create(). 
 */ 
static linux_bde_bus_t _bus;

static void
_bus_features(int unit, int *be_pio, int *be_packet, int *be_other)
{
    /*
     * XGS BCM 56xxx/53xxx devices get the endianness from compile flags
     * whereas SBX devices (BCM88020 and QE-2000) get it by querying
     * the kernel BDE (ioctl)
     */
    if ((_devices[unit]->bde_dev.device & 0xFF00) != 0x5600 &&
        (_devices[unit]->bde_dev.device & 0xF000) != 0xc000 &&
        (_devices[unit]->bde_dev.device & 0xF000) != 0xb000 &&
        (_devices[unit]->bde_dev.device & 0xF000) != 0x8000 &&
        (_devices[unit]->bde_dev.device & 0xFFFF) != 0x0732 &&
        (_devices[unit]->bde_dev.device & 0xFFF0) != 0x0230 &&
        (_devices[unit]->bde_dev.device & 0xFFF0) != 0x0030 &&
        (_devices[unit]->bde_dev.device & 0xFFF0) != 0xa440) {
        lubde_ioctl_t devio;

        /* Initialize the variable */
        memset(&devio, 0, sizeof(lubde_ioctl_t));

        devio.dev = unit;
        _ioctl(LUBDE_GET_BUS_FEATURES, &devio);
        *be_pio = devio.d0;
    *be_packet = devio.d1;
    *be_other = devio.d2;
    } else {
        *be_pio = _bus.be_pio;
        *be_packet = _bus.be_packet;
        *be_other = _bus.be_other;
    }
}

/* 
* Our BDE interface structure
*/
static ibde_t _ibde = {
    _name, 
    _num_devices, 
    _get_dev, 
    _get_dev_type, 
    _pci_config_get32, 
    _pci_config_put32,
    _bus_features,
    _read, 
    _write, 
    _salloc, 
    _sfree, 
    NULL, 
    NULL, 
    _interrupt_connect, 
    _interrupt_disconnect, 
    _l2p, 
    _p2l, 
    _spi_read,
    _spi_write,
    _iproc_read,
    _iproc_write,
};

#ifdef BCM_INSTANCE_SUPPORT
int
linux_bde_instance_attach(unsigned int dev_mask,unsigned int dma_size)
{
    _inst_dev_mask = dev_mask;
    _inst_dma_size = dma_size;
    return 0;
}
#endif

/* 
 * Function: linux_bde_create
 *
 * Purpose:
 *    Creator function for this BDE interface. 
 * Parameters:
 *    bus - pointer to the bus features structure you want this 
 *          bde to export. Depends on the system. 
 *    ibde - pointer to a location to recieve the bde interface pointer. 
 * Returns:
 *    0 on success
 *    -1 on failure. 
 * Notes:
 *    This is the main BDE create function for this interface. 
 *    Used by the external system initialization code.
 */
int 
linux_bde_create(linux_bde_bus_t* bus, ibde_t** ibde)
{
    static int _init = 0;
    
    if (!_init) {
        if (_open() == -1) {       
            *ibde = NULL;
            return -1;
        }       
        _init = 1;
    }
    memset(&_bus, 0, sizeof(_bus));
    if (bus) {
        _bus = *bus;
    }
    *ibde = &_ibde;

#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    linux_spidev_setup();
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_SPIDEV_SUPPORT */

#if LINUX_MDIODEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    linux_mdiodev_setup();
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_MDIODEV_SUPPORT */

#if CMIC_SPI_SUPPORT 
    linux_cmic_spidev_open(0);
#endif	
    return 0;
}
    
/*
 * Function: linux_bde_destroy
 *
 * Purpose:
 *    destroy this bde
 * Parameters:
 *    BDE interface pointer
 * Returns:
 *    0 on success, < 0 on error. 
 */
int
linux_bde_destroy(ibde_t* ibde)
{
#if LINUX_SPIDEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    int ndevs;

    for (ndevs = 0; ndevs < _switch_ndevices; ndevs++) {
        if (spi_dev_linux[ndevs]) {
            _sys_spi_dev_deinit(ndevs);
            spi_dev_linux[ndevs] = 0;
        }
    }
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_SPIDEV_SUPPORT */

#if LINUX_MDIODEV_SUPPORT
#ifdef BCM_ROBO_SUPPORT
    _sys_mdio_dev_deinit(0);
#endif /* BCM_ROBO_SUPPORT */
#endif /* LINUX_MDIODEV_SUPPORT */

#if CMIC_SPI_SUPPORT 
    linux_cmic_spidev_close(0);
#endif
    return _close();
}

/*
 * Function: bde_irq_mask_set
 *
 * Purpose:
 *    Set interrupt mask from user space interrupt handler
 * Parameters:
 *    unit - unit number
 *    addr - PCI address of interrupt mask register
 *    mask - interrupt mask
 * Returns:
 *    0 on success, < 0 on error. 
 */
int
bde_irq_mask_set(int unit, uint32 addr, uint32 mask)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.dev = unit;
    devio.d0 = addr;
    devio.d1 = mask;
    _ioctl(LUBDE_WRITE_IRQ_MASK, &devio);
    return devio.rc;
}
/*
 * Function: bde_hw_unit_get
 *
 * Purpose:
 *  Get the hw or user unit map
 * Parameter:
 *  unit (IN)   : inverse == 0, unit = user unit
 *                inverse != 0, unit = hw unit
 *  inverse (IN): specify to get hw or user unip map
 * Returns:
 *  hw unit (probed in kernel BDE) : when inverse == 0
 *  user unit : when invers != 0
 */
int
bde_hw_unit_get(int unit, int inverse)
{
    int u = 0;
    if (inverse) {
        for (u = 0; u < _ndevices; u ++) {
            if (_devices[u]->dev_id == unit) {
                return u;
            }
        }
    } else {
        if ((unit >= 0) && (unit < _ndevices)) {
            u = _devices[unit]->dev_id;
        } else {
            u = _devices[0]->dev_id;
        }
    }
    return u;
}

#ifdef LINUX_SAL_USLEEP_OVERRIDE

/*
 * Function: sal_usleep
 *
 * Notes:
 *    See src/sal/core/unix/thread.c for details.
 */
void
sal_usleep(uint32 usec)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    if (_devfd < 0 && _open() < 0) {
        /* Should not get here unless we are broken */
        sal_sleep(1);
        return;
    }
    devio.d0 = usec;
    _ioctl(LUBDE_USLEEP, &devio);
}

#endif /* LINUX_SAL_USLEEP_OVERRIDE */

#ifdef LINUX_SAL_UDELAY_OVERRIDE

/*
 * Function: sal_udelay
 *
 * Notes:
 *    See src/sal/core/unix/thread.c for details.
 */
void
sal_udelay(uint32 usec)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    if (_devfd < 0) {
        return;
    }
    devio.d0 = usec;
    _ioctl(LUBDE_UDELAY, &devio);
}

#endif /* LINUX_SAL_UDELAY_OVERRIDE */

#ifdef LINUX_SAL_DMA_ALLOC_OVERRIDE

/*
 * Function: sal_dma_alloc
 *
 * Notes:
 *    See src/sal/core/unix/alloc.c for details.
 */
void *
sal_dma_alloc(unsigned int sz, char *name)
{
    return _ibde.salloc(0, sz, name); 
}

/*
 * Function: sal_dma_free
 *
 * Notes:
 *    See src/sal/core/unix/alloc.c for details.
 */
void
sal_dma_free(void *ptr)
{
    _ibde.sfree(0, ptr); 
}

#endif /* LINUX_SAL_DMA_ALLOC_OVERRIDE */

#ifdef LINUX_SAL_SEM_OVERRIDE

sal_sem_t
sal_sem_create(char *desc, int binary, int initial_count)
{
    lubde_ioctl_t devio;
#ifdef SAL_SEM_REDIRECT
    sal_kernel64_ptr_t sem;
#endif

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    if (_devfd < 0 && _open() < 0) {
        /* Should not get here unless we are broken */
        return NULL;
    }
    devio.d0 = LUBDE_SEM_OP_CREATE;
    devio.d1 = binary;
    devio.d2 = initial_count;
    _ioctl(LUBDE_SEM_OP, &devio);
#ifdef SAL_SEM_REDIRECT
    sem = sal_alloc(sizeof(struct sal_kernel64_ptr_s), desc);
    if (sem) {
        sem->ptr64 = devio.p0;
        return (sal_sem_t)sem;
    }
    else {
        devio.d0 = LUBDE_SEM_OP_DESTROY;
        _ioctl(LUBDE_SEM_OP, &devio);
        return NULL;
    }
#else
    return (sal_sem_t)devio.p0;
#endif
}

void
sal_sem_destroy(sal_sem_t b)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.d0 = LUBDE_SEM_OP_DESTROY;
#ifdef SAL_SEM_REDIRECT
    devio.p0 = b ? ((sal_kernel64_ptr_t)b)->ptr64 : 0;
    _ioctl(LUBDE_SEM_OP, &devio);
    if (b) {
    sal_free(b);
    }
#else
    devio.p0 = (bde_kernel_addr_t)b;
    _ioctl(LUBDE_SEM_OP, &devio);
#endif
}

int
sal_sem_take(sal_sem_t b, int usec)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.d0 = LUBDE_SEM_OP_TAKE;
#ifdef SAL_SEM_REDIRECT
    devio.p0 = b ? ((sal_kernel64_ptr_t)b)->ptr64 : 0;
#else
    devio.p0 = (bde_kernel_addr_t)b;
#endif
    devio.d2 = usec;
    _ioctl(LUBDE_SEM_OP, &devio);

    /* Handle SIGSTOP gracefully in user mode */
    while (devio.rc == -2) {
        _ioctl(LUBDE_SEM_OP, &devio);
    }

    return devio.rc;
}

int
sal_sem_give(sal_sem_t b)
{
    lubde_ioctl_t devio;

    /* Initialize the variable */
    memset(&devio, 0, sizeof(lubde_ioctl_t));

    devio.d0 = LUBDE_SEM_OP_GIVE;
#ifdef SAL_SEM_REDIRECT
    devio.p0 = b ? ((sal_kernel64_ptr_t)b)->ptr64 : 0;
#else
    devio.p0 = (bde_kernel_addr_t)b;
#endif
    _ioctl(LUBDE_SEM_OP, &devio);
    return devio.rc;
}

#endif /* LINUX_SAL_SEM_OVERRIDE */

int
_dma_get_usage(void)
{
    return mpool_usage(_dma_pool);
}

