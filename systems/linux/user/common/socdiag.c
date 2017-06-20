/*
 * $Id: socdiag.c,v 1.25 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag: low-level diagnostics shell for Orion (SOC) driver.
 */

#include <unistd.h>
#include <stdlib.h>

#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/config.h>
#include <sal/appl/pci.h>
#include <soc/debug.h>
#include <shared/shr_bprof.h>

#include <appl/diag/system.h>

#include <linux-bde.h>

#if defined(MEMLOG_SUPPORT) && defined(__GNUC__)
#include <sal/core/memlog.h>
#include <sys/stat.h>
#include <fcntl.h>
int memlog_fd = 0;
char memlog_buf[MEM_LOG_BUF_SIZE];
sal_mutex_t memlog_lock = NULL;
#endif

ibde_t *bde;

/* The bus properties are (currently) the only system specific
 * settings required. 
 * These must be defined beforehand 
 */

#ifndef SYS_BE_PIO
#error "SYS_BE_PIO must be defined for the target platform"
#endif
#ifndef SYS_BE_PACKET
#error "SYS_BE_PACKET must be defined for the target platform"
#endif
#ifndef SYS_BE_OTHER
#error "SYS_BE_OTHER must be defined for the target platform"
#endif

#if !defined(SYS_BE_PIO) || !defined(SYS_BE_PACKET) || !defined(SYS_BE_OTHER)
#error "platform bus properties not defined."
#endif

#ifdef INCLUDE_KNET

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <soc/knet.h>
#include <uk-proxy-kcom.h>
#include <bcm-knet-kcom.h>

/* Function defined in linux-user-bde.c */
extern int
bde_irq_mask_set(int unit, uint32 addr, uint32 mask);
extern int
bde_hw_unit_get(int unit, int inverse);

static soc_knet_vectors_t knet_vect_uk_proxy = {
    {
        uk_proxy_kcom_open,
        uk_proxy_kcom_close,
        uk_proxy_kcom_msg_send,
        uk_proxy_kcom_msg_recv
    },
    bde_irq_mask_set,
    bde_hw_unit_get
};

static soc_knet_vectors_t knet_vect_bcm_knet = {
    {
        bcm_knet_kcom_open,
        bcm_knet_kcom_close,
        bcm_knet_kcom_msg_send,
        bcm_knet_kcom_msg_recv
    },
    bde_irq_mask_set,
    bde_hw_unit_get
};

static void
knet_kcom_config(void)
{
    soc_knet_vectors_t *knet_vect;
    char *kcom_name;
    int procfd;
    char procbuf[128];

    /* Direct IOCTL by default */
    knet_vect = &knet_vect_bcm_knet;
    kcom_name = "bcm-knet";

    if ((procfd = open("/proc/linux-uk-proxy", O_RDONLY)) >= 0) {
        if ((read(procfd, procbuf, sizeof(procbuf))) > 0 &&
            strstr(procbuf, "KCOM_KNET : ACTIVE") != NULL) {
            /* Proxy loaded and active */
            knet_vect = &knet_vect_uk_proxy;
            kcom_name = "uk-proxy";
        }
        close(procfd);
    }

    soc_knet_config(knet_vect);
    var_set("kcom", kcom_name, 0, 0);
}

#endif /* INCLUDE_KNET */


#ifdef BCM_INSTANCE_SUPPORT
static int
_instance_config(const char *inst)
{
    const char *ptr;
#ifndef NO_SAL_APPL
    char *estr;
#endif
    unsigned int dev_mask, dma_size;

    if (inst == NULL) {
#ifndef NO_SAL_APPL
        estr = "./bcm.user -i <dev_mask>[:dma_size_in_mb] \n";
        sal_console_write(estr, sal_strlen(estr) + 1);
#endif
        return -1;
    }
    dev_mask = strtol(inst, NULL, 0);
    if ((ptr = strchr(inst,':')) == NULL) {
        dma_size = 4;
    } else {
        ptr++;
        dma_size = strtol(ptr, NULL, 0);
    }

    if (dma_size < 4) {
#ifndef NO_SAL_APPL
        estr = "dmasize must be > 4M and a power of 2 (4M, 8M etc.)\n";
        sal_console_write(estr, sal_strlen(estr) + 1);
#endif
        return -1;
    } else {
        if ( (dma_size >> 2) & ((dma_size >> 2 )-1)) {
#ifndef NO_SAL_APPL
            estr = "dmasize must be a power of 2 (4M, 8M etc.)\n";
            sal_console_write(estr, sal_strlen(estr) + 1);
#endif
            return -1;
        }
    }
    
    return linux_bde_instance_attach(dev_mask, dma_size);
}
#endif

int
bde_create(void)
{
    linux_bde_bus_t bus;
    bus.be_pio = SYS_BE_PIO;
    bus.be_packet = SYS_BE_PACKET;
    bus.be_other = SYS_BE_OTHER;

    return linux_bde_create(&bus, &bde);

}

/*
 * Main loop.
 */
int main(int argc, char *argv[])
{
    int i, len;
    char *envstr;
    char *config_file, *config_temp;
#ifdef BCM_INSTANCE_SUPPORT
    const char *inst = NULL;
#endif

#ifdef BCM_BPROF_STATS
    shr_bprof_stats_time_init();
#endif

    if ((envstr = getenv("BCM_CONFIG_FILE")) != NULL) {
        config_file = envstr;
        len = sal_strlen(config_file);
        if ((config_temp = sal_alloc(len+5, NULL)) != NULL) {
            sal_strcpy(config_temp, config_file);
            sal_strcpy(&config_temp[len], ".tmp");
#ifndef NO_SAL_APPL
            sal_config_file_set(config_file, config_temp);
#endif
            sal_free(config_temp);
        }
    }

#ifdef BCM_INSTANCE_SUPPORT
    for (i = 1; i < argc; i++) {
         if (!strcmp(argv[i], "-i") || !strcmp(argv[i], "--instance")) {
            inst = argv[i+1];            
            /*
             * specify the dev_mask and its dma_size (optional,default:4MB)
             * bcm.user -i 0x1[:8]
             */
            if (_instance_config(inst) < 0){
#ifndef NO_SAL_APPL
                char *estr = "config error!\n";
                sal_console_write(estr, sal_strlen(estr) + 1);
#endif
                exit(1);
            }
        }
    }
#endif

    if (sal_core_init() < 0
#ifndef NO_SAL_APPL
        || sal_appl_init() < 0
#endif
        ) {
        /*
         * If SAL initialization fails then printf will most
         * likely assert or fail. Try direct console access
         * instead to get the error message out.
         */
#ifndef NO_SAL_APPL
        char *estr = "SAL Initialization failed\r\n";
        sal_console_write(estr, sal_strlen(estr) + 1);
#endif
        exit(1);
    }

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--reload")) {
            sal_boot_flags_set(sal_boot_flags_get() | BOOT_F_RELOAD);
        }
    }

#ifdef MEMLOG_SUPPORT
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-memlog") && argv[i+1] != NULL) {
            memlog_fd = creat(argv[i+1], S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        }
    }
    if (memlog_fd > 0) {
        memlog_lock = sal_mutex_create("memlog lock");
    }
#endif
#ifdef INCLUDE_KNET
    knet_kcom_config();
#endif

    diag_shell();

    linux_bde_destroy(bde);
#ifdef MEMLOG_SUPPORT
    if (memlog_lock) {
        sal_mutex_destroy(memlog_lock);
    }
#endif
    return 0;
}



/*
 * These stubs are here for legacy compatability reasons. 
 * They are used only by the diag/test code, not the driver, 
 * so they are really not that important. 
 */

void pci_print_all(void)
{
    int device;

    if (NULL == bde) {
        sal_printf("Devices not probed yet.\n");
        return;
    }

    sal_printf("Scanning function 0 of devices 0-%d\n", bde->num_devices(BDE_SWITCH_DEVICES) - 1);
    sal_printf("device fn venID devID class  rev MBAR0    MBAR1    IPIN ILINE\n");

    for (device = 0; device < bde->num_devices(BDE_SWITCH_DEVICES); device++) {
	uint32		vendorID, deviceID, class, revID;
	uint32		MBAR0, MBAR1, ipin, iline;
	
	vendorID = (bde->pci_conf_read(device, PCI_CONF_VENDOR_ID) & 0x0000ffff);
	
	if (vendorID == 0)
	    continue;
	

#define CONFIG(offset)	bde->pci_conf_read(device, (offset))

	deviceID = (CONFIG(PCI_CONF_VENDOR_ID) & 0xffff0000) >> 16;
	class    = (CONFIG(PCI_CONF_REVISION_ID) & 0xffffff00) >>  8;
	revID    = (CONFIG(PCI_CONF_REVISION_ID) & 0x000000ff) >>  0;
	MBAR0    = (CONFIG(PCI_CONF_BAR0) & 0xffffffff) >>  0;
	MBAR1    = (CONFIG(PCI_CONF_BAR1) & 0xffffffff) >>  0;
	iline    = (CONFIG(PCI_CONF_INTERRUPT_LINE) & 0x000000ff) >>  0;
	ipin     = (CONFIG(PCI_CONF_INTERRUPT_LINE) & 0x0000ff00) >>  8;
	
#undef CONFIG

	sal_printf("%02x  %02x %04x  %04x  "
		   "%06x %02x  %08x %08x %02x   %02x\n",
		   device, 0, vendorID, deviceID, class, revID,
		   MBAR0, MBAR1, ipin, iline);
    }
}
