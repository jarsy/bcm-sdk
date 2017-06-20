/*
 * Only edit this file under $(UKERNEL), the SDK version is a copy!
 *
 * $Id: bcm53084lib.c,v 1.55 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    bcm53084lib.c
 */

#ifdef BCM_PLATFORM_STRING
#define SDK_BUILD
#endif

#ifndef MGMT_DEBUG
#define MGMT_DEBUG              0
#endif

#ifdef SDK_BUILD

#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/core/libc.h>

#include <soc/error.h>
#include <soc/cmic.h>
#include <soc/drv.h>

#include <soc/arl.h>
#include <soc/counter.h>
#include <bcm/error.h>
#include <bcm/port.h>
#include <bcm_int/robo_dispatch.h>

#if (defined(LINUX) && !defined(__KERNEL__))
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#endif /* (defined(LINUX) && !defined(__KERNEL__)) */

#include <appl/diag/aps/bcm89500lib.h>
#include <appl/diag/aps/spiflash.h>

#else /* SDK_BUILD */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <endian.h>
#include <fcntl.h>

#include "bcm53084lib.h"

#ifdef MGMT_SPI_SUPPORT
#include <sys/ioctl.h>
#ifdef ANDROID
#include "/usr/include/linux/spi/spidev.h"
#else
#include <linux/spi/spidev.h>
#endif
#endif

#ifdef MGMT_ENCRYPTION_SUPPORT
#include <openssl/aes.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#include <openssl/engine.h>
#include <openssl/pem.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include <openssl/x509.h>
#endif

#ifdef MGMT_USB_SUPPORT
#include <libusb-1.0/libusb.h>
#endif

#include "spiflash.h"

#endif /* SDK_BUILD */

#include <sys/syslog.h>
#include "swmsyslog.h"
#endif

#ifndef SDK_BUILD

#define sal_memset(a, b, c)     memset(a, b, c)
#define sal_memcpy(a, b, c)     memcpy(a, b, c)
#define sal_alloc(len, tag)     malloc(len)
#define sal_free(ptr)           free(ptr)
#define sal_sleep(secs)         sleep(secs)
#define sal_usleep(usecs)       usleep(usecs)
#define sal_strcpy(dst, src)    strcpy(dst, src)
#define sal_strlen(str)         strlen(str)

#endif

#define DEFAULT_SPI_CLOCK_SPEED      2000000  /* 2MHz */

#define READ_POWER_STATE_FROM_APS    (0)
#define DEFAULT_DEVICE_ID            (0)

typedef struct mgmt_dmu_value_s {
    uint32      hz;
    uint32      hclk_freq;
    uint32      hclk_sel;
    uint32      pclk_freq;
    uint32      pclk_sel;
    uint32      p1div;
    uint32      p2div;
    uint32      ndiv;
    uint32      m1div;
    uint32      m2div;
    uint32      m3div;
    uint32      m4div;
    uint32      pll_num;
    uint32      frac;
    uint32      bclk_sel;
} mgmt_dmu_value_t;

mgmt_dmu_value_t mgmt_dmu_values[] = {
        { 25000000 },
        { 300000000, 150000000, 1, 75000000, 1, 1, 1, 60, 50, 5, 0, 0, 0, 0, 0 },
        { 250000000, 125000000, 1, 62500000, 1, 1, 1, 60, 50, 6, 0, 0, 0, 0, 0 },
        { 187000000, 937500000, 1, 46875000, 1, 1, 1, 60, 50, 8, 0, 0, 0, 0, 0 },
        { 166000000, 555000000, 1, 27700000, 1, 1, 1, 60, 50, 9, 0, 0, 0, 0, 0 },
};


/* Assign default power state when subsystem is out of reset */
#ifdef SDK_BUILD
static power_state_t power_state = POWER_FULL_SPEED;
#endif

/*
 * Generated with:
 *     od -v -t x1 BCM53084_A0-pll.image | sed -e 's/^.......//' -e 's/^ /0x/' -e 's/ /, 0x/g' -e 's/$/,/'
 */

static uint8_t mgmt_bcm53084_pll_image[] = {
0xe3, 0xcb, 0x65, 0x78, 0xe6, 0x9a, 0xb3, 0xb6, 0x27, 0x95, 0xe6, 0xd0, 0x10, 0x04, 0x00, 0x00,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0x17, 0x20, 0x31, 0xad, 0x47, 0x41, 0x07, 0x50, 0x0c, 0xa5, 0xd4, 0x0f, 0x12, 0x0d, 0xff, 0x74,
0xa9, 0x68, 0x1e, 0x85, 0x31, 0x17, 0x63, 0x58, 0x27, 0xb8, 0x92, 0x74, 0x88, 0x0b, 0xef, 0x3e,
0x07, 0xc2, 0xf1, 0x11, 0x10, 0x76, 0xf0, 0x80, 0x2a, 0x57, 0xc8, 0xfe, 0x2c, 0x0f, 0x91, 0x66,
0xba, 0x5b, 0x92, 0xdd, 0x7a, 0x8e, 0x7f, 0xaf, 0x07, 0xbc, 0xf5, 0xd9, 0xd7, 0xa0, 0x55, 0xfb,
0x53, 0x50, 0x26, 0x44, 0x3e, 0x00, 0xb7, 0x3d, 0xc8, 0x89, 0xe3, 0x79, 0xc1, 0xd2, 0x5e, 0x36,
0x0c, 0x7d, 0xde, 0x8f, 0x03, 0x41, 0xdf, 0xbf, 0x9c, 0x68, 0xeb, 0x30, 0xd9, 0x71, 0x7b, 0x7f,
0xde, 0x65, 0xcb, 0x23, 0xe4, 0xa4, 0xec, 0xba, 0x37, 0xe4, 0x2c, 0xc7, 0xf5, 0xe4, 0x9e, 0x96,
0xbb, 0x53, 0x4d, 0x9d, 0xdc, 0xc4, 0x3c, 0xa9, 0x20, 0x23, 0xb6, 0x17, 0x99, 0x25, 0xff, 0x0f,
0x41, 0x80, 0xe1, 0x0d, 0xfd, 0xe1, 0x71, 0x2a, 0x60, 0x88, 0x2d, 0xaf, 0x22, 0x89, 0x3a, 0x4d,
0xfa, 0x0b, 0x4b, 0xce, 0xb9, 0x01, 0x05, 0x78, 0x66, 0x59, 0xb3, 0x07, 0x02, 0x96, 0xcc, 0x6b,
0x75, 0x2b, 0x2f, 0x3f, 0xfc, 0x9b, 0x89, 0xc5, 0x42, 0x39, 0x4d, 0x77, 0x94, 0xc8, 0x38, 0x40,
0x5c, 0xb8, 0x95, 0x1d, 0xc6, 0x89, 0x8a, 0xc3, 0xa8, 0xd0, 0xea, 0xad, 0x49, 0xd9, 0x3c, 0x2d,
0xad, 0xd0, 0xd2, 0x29, 0xa5, 0x1d, 0x17, 0xc4, 0xae, 0x25, 0x83, 0xad, 0x1a, 0xa4, 0x43, 0x8b,
0x41, 0xdf, 0xa6, 0x48, 0x40, 0x83, 0x38, 0x14, 0x03, 0x75, 0x97, 0x7b, 0x30, 0x61, 0xae, 0x1c,
0x04, 0x74, 0xfb, 0xd7, 0x26, 0x9f, 0xec, 0xa1, 0x15, 0x0d, 0x3f, 0x27, 0xd6, 0x84, 0xc8, 0x3d,
0x43, 0xc9, 0xf5, 0xdc, 0x50, 0xa1, 0x10, 0x6a, 0x73, 0xde, 0xb5, 0xea, 0x41, 0x76, 0x11, 0xa7,
0xff, 0xff, 0xff, 0xfa, 0x70, 0xb5, 0xdf, 0xf8, 0x68, 0xc0, 0x0c, 0x46, 0x19, 0x49, 0x05, 0x46,
0x8c, 0x45, 0x38, 0xbf, 0x00, 0x23, 0x03, 0xd2, 0x4c, 0xf8, 0x04, 0x3b, 0x8c, 0x45, 0xfb, 0xd3,
0x15, 0x48, 0x01, 0x68, 0x41, 0xf0, 0x02, 0x01, 0x01, 0x60, 0x28, 0x46, 0x00, 0xf0, 0xf4, 0xf8,
0x4f, 0xf4, 0x48, 0x2c, 0xdc, 0xf8, 0x40, 0x00, 0xdc, 0xf8, 0x40, 0x10, 0x11, 0xf4, 0x80, 0x7f,
0x0c, 0xbf, 0x20, 0xf0, 0x01, 0x00, 0x40, 0xf0, 0x01, 0x00, 0xcc, 0xf8, 0x40, 0x00, 0x00, 0x21,
0x00, 0x2c, 0x04, 0xd9, 0xdc, 0xf8, 0x40, 0x00, 0x49, 0x1c, 0x8c, 0x42, 0xfa, 0xd8, 0x23, 0x46,
0x2a, 0x46, 0xbd, 0xe8, 0x70, 0x40, 0xdf, 0xf8, 0x14, 0xc0, 0x00, 0x21, 0x08, 0x46, 0x60, 0x47,
0x10, 0x04, 0x00, 0x00, 0x10, 0x04, 0x00, 0x00, 0x04, 0x08, 0x0a, 0x00, 0x14, 0x00, 0xff, 0xff,
0x2d, 0xe9, 0xff, 0x4f, 0x00, 0x23, 0x1d, 0x46, 0xdd, 0xe9, 0x10, 0xc9, 0xdd, 0xf8, 0x3c, 0xb0,
0xdd, 0xf8, 0x34, 0xa0, 0xbc, 0xf1, 0x00, 0x0f, 0x7e, 0xd1, 0x4f, 0xf4, 0x46, 0x24, 0x9c, 0x46,
0x66, 0x6c, 0x26, 0xf0, 0x03, 0x07, 0x67, 0x64, 0x42, 0xf2, 0x10, 0x76, 0xd4, 0xf8, 0x44, 0x80,
0x0c, 0xf1, 0x01, 0x0c, 0xb4, 0x45, 0x01, 0xd8, 0xb8, 0x45, 0xf7, 0xd1, 0xd4, 0xf8, 0x00, 0xc0,
0x4c, 0xf0, 0x80, 0x6c, 0xc4, 0xf8, 0x00, 0xc0, 0xd4, 0xf8, 0x00, 0xc0, 0x4c, 0xf0, 0x00, 0x6c,
0xc4, 0xf8, 0x00, 0xc0, 0x27, 0x68, 0x91, 0xfb, 0xf0, 0xfc, 0x0c, 0xfb, 0x02, 0xf8, 0x40, 0xea,
0x01, 0x1c, 0x4c, 0xea, 0x02, 0x2c, 0x08, 0xeb, 0xc8, 0x0e, 0x0e, 0xeb, 0x08, 0x1e, 0xbe, 0xf5,
0xc8, 0x6f, 0x4c, 0xf0, 0x40, 0x6c, 0x88, 0xbf, 0x4c, 0xf4, 0x80, 0x2c, 0xc4, 0xf8, 0x00, 0xc0,
0xdd, 0xf8, 0x0c, 0xc0, 0xc4, 0xf8, 0x08, 0xc0, 0x4a, 0xea, 0x0a, 0x4c, 0xc4, 0xf8, 0x0c, 0xc0,
0xd4, 0xf8, 0x04, 0xc0, 0xd4, 0xf8, 0x18, 0x80, 0xb9, 0xf1, 0x00, 0x0f, 0x18, 0xd0, 0x2c, 0xf0,
0x7f, 0x4a, 0xca, 0x45, 0x24, 0xd0, 0x2c, 0xf0, 0x60, 0x6c, 0x0c, 0xf0, 0x7f, 0x45, 0x45, 0xea,
0x09, 0x05, 0x25, 0xf0, 0x80, 0x75, 0x45, 0xf0, 0x80, 0x65, 0x65, 0x60, 0xbe, 0xf5, 0xc8, 0x6f,
0x8c, 0xbf, 0xdf, 0xf8, 0xc4, 0x90, 0xdf, 0xf8, 0xc4, 0x90, 0xc4, 0xf8, 0x18, 0x90, 0x0f, 0xe0,
0xbb, 0xf1, 0x01, 0x0f, 0x0c, 0xd0, 0x0c, 0xf0, 0x71, 0x45, 0x45, 0xf0, 0x80, 0x75, 0x65, 0x60,
0xbe, 0xf5, 0xc8, 0x6f, 0x8c, 0xbf, 0xdf, 0xf8, 0xa8, 0x90, 0xdf, 0xf8, 0xa8, 0x90, 0xec, 0xe7,
0xd4, 0xf8, 0x18, 0x90, 0xd4, 0xf8, 0x00, 0xa0, 0x07, 0xf0, 0x0f, 0x0e, 0x2a, 0xf0, 0x80, 0x6a,
0xc4, 0xf8, 0x00, 0xa0, 0x86, 0x45, 0x04, 0xbf, 0xc7, 0xf3, 0x03, 0x10, 0x88, 0x42, 0x08, 0xd1,
0xc7, 0xf3, 0x08, 0x20, 0x90, 0x42, 0x00, 0xe0, 0x32, 0xe0, 0x04, 0xbf, 0x65, 0x45, 0xc8, 0x45,
0x0a, 0xd0, 0x20, 0x6c, 0x10, 0xf0, 0x02, 0x0f, 0x05, 0xd1, 0xb3, 0xf5, 0xfa, 0x5f, 0x3c, 0xbf,
0x20, 0x6c, 0x5b, 0x1c, 0xf6, 0xd3, 0x00, 0x23, 0x20, 0x6c, 0x10, 0xf0, 0x01, 0x0f, 0x0b, 0xd1,
0x20, 0x6c, 0x5b, 0x1c, 0xb3, 0xf5, 0xfa, 0x5f, 0x03, 0xd9, 0x04, 0xb0, 0x01, 0x20, 0xbd, 0xe8,
0xf0, 0x8f, 0x10, 0xf0, 0x01, 0x0f, 0xf3, 0xd0, 0xbb, 0xf1, 0x01, 0x0f, 0x10, 0xd0, 0x20, 0x68,
0x20, 0xf0, 0x00, 0x60, 0x20, 0x60, 0x61, 0x6c, 0x00, 0x20, 0x21, 0xf0, 0x03, 0x01, 0x41, 0xf0,
0x01, 0x01, 0x61, 0x64, 0x62, 0x6c, 0x40, 0x1c, 0xb0, 0x42, 0x01, 0xd8, 0x8a, 0x42, 0xf9, 0xd1,
0x04, 0xb0, 0x00, 0x20, 0xbd, 0xe8, 0xf0, 0x8f, 0x60, 0x28, 0x2c, 0x38, 0x20, 0x28, 0x2c, 0x20,
0x00, 0x07, 0x00, 0x38, 0xc0, 0x05, 0x00, 0x20, 0x30, 0xb5, 0x71, 0x4c, 0x05, 0x00, 0x86, 0xb0,
0x42, 0xf2, 0x10, 0x72, 0x4f, 0xf4, 0x46, 0x21, 0x0f, 0xd0, 0x01, 0x2d, 0x3f, 0xd0, 0x02, 0x2d,
0x71, 0xd0, 0x03, 0x2d, 0x6e, 0xd0, 0x6b, 0x48, 0xa0, 0x60, 0x60, 0x60, 0x48, 0x6c, 0x20, 0xf0,
0xff, 0x00, 0x48, 0x64, 0x25, 0x70, 0x06, 0xb0, 0x30, 0xbd, 0x67, 0x48, 0xa0, 0x60, 0x40, 0x10,
0x60, 0x60, 0x4b, 0x6c, 0x00, 0x20, 0x23, 0xf0, 0x30, 0x03, 0x43, 0xf0, 0x10, 0x03, 0x4b, 0x64,
0xd1, 0xf8, 0x44, 0xc0, 0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x4b, 0x6c,
0x00, 0x20, 0x23, 0xf0, 0xc0, 0x03, 0x43, 0xf0, 0x40, 0x03, 0x4b, 0x64, 0xd1, 0xf8, 0x44, 0xc0,
0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x0d, 0xf1, 0x08, 0x0c, 0x00, 0x23,
0x1a, 0x46, 0x19, 0x46, 0x18, 0x46, 0xac, 0xe8, 0x0f, 0x00, 0x05, 0x22, 0xcd, 0xe9, 0x00, 0x23,
0x01, 0x21, 0x32, 0x23, 0x08, 0x46, 0x3c, 0x22, 0xff, 0xf7, 0xea, 0xfe, 0xca, 0xe7, 0x4f, 0x48,
0xa0, 0x60, 0x40, 0x10, 0x60, 0x60, 0x4b, 0x6c, 0x00, 0x20, 0x23, 0xf0, 0x30, 0x03, 0x43, 0xf0,
0x10, 0x03, 0x4b, 0x64, 0xd1, 0xf8, 0x44, 0xc0, 0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45,
0xf8, 0xd1, 0x4b, 0x6c, 0x00, 0x20, 0x23, 0xf0, 0xc0, 0x03, 0x43, 0xf0, 0x40, 0x03, 0x4b, 0x64,
0xd1, 0xf8, 0x44, 0xc0, 0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x0d, 0xf1,
0x08, 0x0c, 0x00, 0x23, 0x1a, 0x46, 0x19, 0x46, 0x18, 0x46, 0xac, 0xe8, 0x0f, 0x00, 0x06, 0x22,
0xcd, 0xe9, 0x00, 0x23, 0x01, 0x21, 0x32, 0x23, 0x08, 0x46, 0x3c, 0x22, 0xff, 0xf7, 0xb8, 0xfe,
0x98, 0xe7, 0x00, 0xe0, 0x31, 0xe0, 0x36, 0x48, 0xa0, 0x60, 0x40, 0x10, 0x60, 0x60, 0x4b, 0x6c,
0x00, 0x20, 0x23, 0xf0, 0x30, 0x03, 0x43, 0xf0, 0x10, 0x03, 0x4b, 0x64, 0xd1, 0xf8, 0x44, 0xc0,
0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x4b, 0x6c, 0x00, 0x20, 0x23, 0xf0,
0xc0, 0x03, 0x43, 0xf0, 0x40, 0x03, 0x4b, 0x64, 0xd1, 0xf8, 0x44, 0xc0, 0x40, 0x1c, 0x90, 0x42,
0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x0d, 0xf1, 0x08, 0x0c, 0x00, 0x23, 0x1a, 0x46, 0x19, 0x46,
0x18, 0x46, 0xac, 0xe8, 0x0f, 0x00, 0x08, 0x22, 0xcd, 0xe9, 0x00, 0x23, 0x01, 0x21, 0x32, 0x23,
0x08, 0x46, 0x3c, 0x22, 0xff, 0xf7, 0x84, 0xfe, 0x64, 0xe7, 0x1e, 0x48, 0xa0, 0x60, 0x1e, 0x48,
0x60, 0x60, 0x00, 0x20, 0x4b, 0x6c, 0x23, 0xf0, 0x30, 0x03, 0x43, 0xf0, 0x20, 0x03, 0x4b, 0x64,
0xd1, 0xf8, 0x44, 0xc0, 0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x4b, 0x6c,
0x00, 0x20, 0x23, 0xf0, 0xc0, 0x03, 0x43, 0xf0, 0x40, 0x03, 0x4b, 0x64, 0xd1, 0xf8, 0x44, 0xc0,
0x40, 0x1c, 0x90, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x0d, 0xf1, 0x08, 0x0c, 0x00, 0x23,
0x1a, 0x46, 0x19, 0x46, 0x18, 0x46, 0xac, 0xe8, 0x0f, 0x00, 0x09, 0x22, 0xcd, 0xe9, 0x00, 0x23,
0x01, 0x21, 0x32, 0x23, 0x08, 0x46, 0x3c, 0x22, 0xff, 0xf7, 0x52, 0xfe, 0x32, 0xe7, 0x00, 0x00,
0x00, 0x04, 0x00, 0x00, 0x40, 0x78, 0x7d, 0x01, 0x80, 0xd1, 0xf0, 0x08, 0x40, 0x59, 0x73, 0x07,
0xf0, 0x82, 0x96, 0x05, 0xe0, 0xdc, 0x4e, 0x03, 0x20, 0xab, 0xa6, 0x01, 0x00, 0x00, 0x00, 0x00,
0x04, 0x00, 0x00, 0x00, 0x40, 0x78, 0x7d, 0x01, 0x40, 0x78, 0x7d, 0x01, 0x00, 0x00, 0x00, 0x00,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint32_t mgmt_bcm53084_pll_image_size = sizeof(mgmt_bcm53084_pll_image);

static uint8_t mgmt_bcm89500_pll_image[] = {
0xe3, 0xcb, 0x65, 0x78, 0x6c, 0x23, 0x85, 0x1a, 0xa9, 0x0d, 0x15, 0x6d, 0x08, 0x03, 0x00, 0x00,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0xff, 0xff, 0xff, 0xfa, 0x70, 0xb5, 0xdf, 0xf8, 0x44, 0xc0, 0x0c, 0x46, 0x10, 0x49, 0x05, 0x46,
0x8c, 0x45, 0x38, 0xbf, 0x00, 0x23, 0x03, 0xd2, 0x4c, 0xf8, 0x04, 0x3b, 0x8c, 0x45, 0xfb, 0xd3,
0x00, 0xf0, 0xe6, 0xf8, 0x00, 0x21, 0x00, 0x2c, 0x88, 0xbf, 0x4f, 0xf4, 0x48, 0x23, 0x03, 0xd9,
0x18, 0x6c, 0x49, 0x1c, 0x8c, 0x42, 0xfb, 0xd8, 0x23, 0x46, 0x2a, 0x46, 0xbd, 0xe8, 0x70, 0x40,
0xdf, 0xf8, 0x10, 0xc0, 0x00, 0x21, 0x08, 0x46, 0x60, 0x47, 0x00, 0x00, 0x08, 0x03, 0x00, 0x00,
0x08, 0x03, 0x00, 0x00, 0x14, 0x00, 0xff, 0xff, 0x2d, 0xe9, 0xff, 0x4f, 0x00, 0x23, 0x1d, 0x46,
0xdd, 0xe9, 0x10, 0xc9, 0xdd, 0xf8, 0x3c, 0xb0, 0xdd, 0xf8, 0x34, 0xa0, 0xbc, 0xf1, 0x00, 0x0f,
0x7e, 0xd1, 0x4f, 0xf4, 0x46, 0x24, 0x9c, 0x46, 0x66, 0x6c, 0x26, 0xf0, 0x03, 0x07, 0x67, 0x64,
0x42, 0xf2, 0x10, 0x76, 0xd4, 0xf8, 0x44, 0x80, 0x0c, 0xf1, 0x01, 0x0c, 0xb4, 0x45, 0x01, 0xd8,
0xb8, 0x45, 0xf7, 0xd1, 0xd4, 0xf8, 0x00, 0xc0, 0x4c, 0xf0, 0x80, 0x6c, 0xc4, 0xf8, 0x00, 0xc0,
0xd4, 0xf8, 0x00, 0xc0, 0x4c, 0xf0, 0x00, 0x6c, 0xc4, 0xf8, 0x00, 0xc0, 0x27, 0x68, 0x91, 0xfb,
0xf0, 0xfc, 0x0c, 0xfb, 0x02, 0xf8, 0x40, 0xea, 0x01, 0x1c, 0x4c, 0xea, 0x02, 0x2c, 0x08, 0xeb,
0xc8, 0x0e, 0x0e, 0xeb, 0x08, 0x1e, 0xbe, 0xf5, 0xc8, 0x6f, 0x4c, 0xf0, 0x40, 0x6c, 0x88, 0xbf,
0x4c, 0xf4, 0x80, 0x2c, 0xc4, 0xf8, 0x00, 0xc0, 0xdd, 0xf8, 0x0c, 0xc0, 0xc4, 0xf8, 0x08, 0xc0,
0x4a, 0xea, 0x0a, 0x4c, 0xc4, 0xf8, 0x0c, 0xc0, 0xd4, 0xf8, 0x04, 0xc0, 0xd4, 0xf8, 0x18, 0x80,
0xb9, 0xf1, 0x00, 0x0f, 0x18, 0xd0, 0x2c, 0xf0, 0x7f, 0x4a, 0xca, 0x45, 0x24, 0xd0, 0x2c, 0xf0,
0x60, 0x6c, 0x0c, 0xf0, 0x7f, 0x45, 0x45, 0xea, 0x09, 0x05, 0x25, 0xf0, 0x80, 0x75, 0x45, 0xf0,
0x80, 0x65, 0x65, 0x60, 0xbe, 0xf5, 0xc8, 0x6f, 0x8c, 0xbf, 0xdf, 0xf8, 0xc4, 0x90, 0xdf, 0xf8,
0xc4, 0x90, 0xc4, 0xf8, 0x18, 0x90, 0x0f, 0xe0, 0xbb, 0xf1, 0x01, 0x0f, 0x0c, 0xd0, 0x0c, 0xf0,
0x71, 0x45, 0x45, 0xf0, 0x80, 0x75, 0x65, 0x60, 0xbe, 0xf5, 0xc8, 0x6f, 0x8c, 0xbf, 0xdf, 0xf8,
0xa8, 0x90, 0xdf, 0xf8, 0xa8, 0x90, 0xec, 0xe7, 0xd4, 0xf8, 0x18, 0x90, 0xd4, 0xf8, 0x00, 0xa0,
0x07, 0xf0, 0x0f, 0x0e, 0x2a, 0xf0, 0x80, 0x6a, 0xc4, 0xf8, 0x00, 0xa0, 0x86, 0x45, 0x04, 0xbf,
0xc7, 0xf3, 0x03, 0x10, 0x88, 0x42, 0x08, 0xd1, 0xc7, 0xf3, 0x08, 0x20, 0x90, 0x42, 0x00, 0xe0,
0x32, 0xe0, 0x04, 0xbf, 0x65, 0x45, 0xc8, 0x45, 0x0a, 0xd0, 0x20, 0x6c, 0x10, 0xf0, 0x02, 0x0f,
0x05, 0xd1, 0xb3, 0xf5, 0xfa, 0x5f, 0x3c, 0xbf, 0x20, 0x6c, 0x5b, 0x1c, 0xf6, 0xd3, 0x00, 0x23,
0x20, 0x6c, 0x10, 0xf0, 0x01, 0x0f, 0x0b, 0xd1, 0x20, 0x6c, 0x5b, 0x1c, 0xb3, 0xf5, 0xfa, 0x5f,
0x03, 0xd9, 0x04, 0xb0, 0x01, 0x20, 0xbd, 0xe8, 0xf0, 0x8f, 0x10, 0xf0, 0x01, 0x0f, 0xf3, 0xd0,
0xbb, 0xf1, 0x01, 0x0f, 0x10, 0xd0, 0x20, 0x68, 0x20, 0xf0, 0x00, 0x60, 0x20, 0x60, 0x61, 0x6c,
0x00, 0x20, 0x21, 0xf0, 0x03, 0x01, 0x41, 0xf0, 0x01, 0x01, 0x61, 0x64, 0x62, 0x6c, 0x40, 0x1c,
0xb0, 0x42, 0x01, 0xd8, 0x8a, 0x42, 0xf9, 0xd1, 0x04, 0xb0, 0x00, 0x20, 0xbd, 0xe8, 0xf0, 0x8f,
0x60, 0x28, 0x2c, 0x38, 0x20, 0x28, 0x2c, 0x20, 0x00, 0x07, 0x00, 0x38, 0xc0, 0x05, 0x00, 0x20,
0x30, 0xb5, 0x3c, 0x4c, 0x05, 0x46, 0x01, 0x2d, 0x42, 0xf2, 0x10, 0x71, 0x86, 0xb0, 0x4f, 0xf4,
0x46, 0x20, 0x0b, 0xd0, 0x02, 0x2d, 0x3b, 0xd0, 0x37, 0x49, 0xa1, 0x60, 0x61, 0x60, 0x41, 0x6c,
0x21, 0xf0, 0xff, 0x01, 0x41, 0x64, 0x25, 0x70, 0x06, 0xb0, 0x30, 0xbd, 0x33, 0x4a, 0xa2, 0x60,
0x52, 0x10, 0x62, 0x60, 0x43, 0x6c, 0x00, 0x22, 0x23, 0xf0, 0x30, 0x03, 0x43, 0xf0, 0x10, 0x03,
0x43, 0x64, 0xd0, 0xf8, 0x44, 0xc0, 0x52, 0x1c, 0x8a, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1,
0x43, 0x6c, 0x00, 0x22, 0x23, 0xf0, 0xc0, 0x03, 0x43, 0xf0, 0x40, 0x03, 0x43, 0x64, 0xd0, 0xf8,
0x44, 0xc0, 0x52, 0x1c, 0x8a, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1, 0x0d, 0xf1, 0x08, 0x0c,
0x00, 0x23, 0x1a, 0x46, 0x19, 0x46, 0x18, 0x46, 0xac, 0xe8, 0x0f, 0x00, 0x06, 0x22, 0xcd, 0xe9,
0x00, 0x23, 0x01, 0x21, 0x32, 0x23, 0x08, 0x46, 0x3c, 0x22, 0xff, 0xf7, 0xed, 0xfe, 0xca, 0xe7,
0x1b, 0x4a, 0xa2, 0x60, 0x52, 0x10, 0x62, 0x60, 0x43, 0x6c, 0x00, 0x22, 0x23, 0xf0, 0x30, 0x03,
0x43, 0xf0, 0x10, 0x03, 0x43, 0x64, 0xd0, 0xf8, 0x44, 0xc0, 0x52, 0x1c, 0x8a, 0x42, 0x01, 0xd8,
0x9c, 0x45, 0xf8, 0xd1, 0x43, 0x6c, 0x00, 0x22, 0x23, 0xf0, 0xc0, 0x03, 0x43, 0xf0, 0x40, 0x03,
0x43, 0x64, 0xd0, 0xf8, 0x44, 0xc0, 0x52, 0x1c, 0x8a, 0x42, 0x01, 0xd8, 0x9c, 0x45, 0xf8, 0xd1,
0x0d, 0xf1, 0x08, 0x0c, 0x00, 0x23, 0x1a, 0x46, 0x19, 0x46, 0x18, 0x46, 0xac, 0xe8, 0x0f, 0x00,
0x08, 0x22, 0xcd, 0xe9, 0x00, 0x23, 0x01, 0x21, 0x32, 0x23, 0x08, 0x46, 0x3c, 0x22, 0xff, 0xf7,
0xbb, 0xfe, 0x98, 0xe7, 0xf8, 0x02, 0x00, 0x00, 0x40, 0x78, 0x7d, 0x01, 0x40, 0x59, 0x73, 0x07,
0xf0, 0x82, 0x96, 0x05, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x40, 0x78, 0x7d, 0x01,
0x40, 0x78, 0x7d, 0x01, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static uint32_t mgmt_bcm89500_pll_image_size = sizeof(mgmt_bcm89500_pll_image);

void mgmt_print(mgmt_info_t *info, char *fmt, ...);
int mgmt_lock_init(mgmt_info_t *info);
int mgmt_lock_uninit(mgmt_info_t *info);
int mgmt_lock(mgmt_info_t *info);
int mgmt_unlock(mgmt_info_t *info);
int mgmt_cmd(mgmt_info_t *info, mgmt_command_t *cmd, int encrypt);
int mgmt_reply(mgmt_info_t *info, mgmt_reply_t *reply, int *len, int decrypt);
int mgmt_cmd_reply(mgmt_info_t *info, mgmt_command_t *cmd, mgmt_reply_t *reply, int encrypt);
int mgmt_erase_flash(mgmt_info_t *info, int spi, int start, int end);
int mgmt_calculate_flash_header(mgmt_info_t *info,
                                flash_info_t *flash_info,
                                flash_header_t *header,
                                uint32_t dmu_config,
                                uint32_t spi_speed);
int mgmt_pll_spread(mgmt_info_t *info, uint32_t spread);
int mgmt_install_arm_image(mgmt_info_t *info, int spi,
                           flash_header_t *header,
                           int slot,
                           uint8_t *armbuf,
                           uint32_t armlen);

int _mgmt_read_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t offset, uint32_t length, uint8_t *data);
int _mgmt_write_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t offset, uint32_t length, uint8_t *data);

void *buffer_from_file(mgmt_info_t *info, const char *name, uint32_t *len);


#ifdef MGMT_ENCRYPTION_SUPPORT
static int mgmt_decrypt(mgmt_info_t *info, const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_len);
#endif

#ifdef MGMT_SPI_SUPPORT
int mgmt_spi_encode(uint8_t *in, uint32_t inlen, uint8_t *out);
int mgmt_spi_speed(mgmt_info_t *info, uint32_t spi_speed);
int mgmt_spi_send(mgmt_info_t *info, uint8_t *data, uint32_t datalen);
int mgmt_spi_recv(mgmt_info_t *info, uint8_t *data, uint32_t *datalen);
#endif

#if defined(VXWORKS)

extern int aps_spi_read(int d, uint8 *buf, int len);
extern int aps_spi_write(int d, uint8 *buf, int len);
extern int aps_spi_set_freq(int d, uint32 speed_hz);

#endif /* defined(VXWORKS) */

/*
 * @api
 * mgmt_print
 *
 * @brief
 * Library debug print support. Output may be routed as appropriate.
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=fmt - format string
 *
 * @returns void
 */
void mgmt_print(mgmt_info_t *info, char *fmt, ...)
{
    char buf[1024];
    va_list args;

    va_start(args, fmt);
    vsprintf(buf, fmt, args);
    syslog(LOG_ERR, "%s", buf);
#endif
#if defined(SDK_BUILD)
    sal_printf("%s", buf);
#elif MGMT_DEBUG > 0
    if (info->log) {
        fprintf(info->log, "%s", buf);
    }
    else {
        fprintf(stderr, "%s", buf);
    }
#else
    fprintf(stderr, "%s", buf);
#endif
}

/*
 * @api
 * mgmt_lock_init
 *
 * @brief
 * Stub API for initializing locking support.
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 on error, 1 otherwise
 */
int mgmt_lock_init(mgmt_info_t *info)
{
    return 1;
}

/*
 * @api
 * mgmt_lock_uninit
 *
 * @brief
 * Stub API for de-initializing locking support.
 * Shouldn't be called with locks held.
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 on error, 1 otherwise
 */
int mgmt_lock_uninit(mgmt_info_t *info)
{
    return 1;
}

/*
 * @api
 * mgmt_lock
 *
 * @brief
 * Lock the management interface. Nested calls by the same thread
 * may happen.
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 on error, 1 otherwise
 */
int mgmt_lock(mgmt_info_t *info)
{
    return 1;
}

/*
 * @api
 * mgmt_unlock
 *
 * @brief
 * Unlock the management interface. Nested calls by the same thread
 * may happen.
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 on error, 1 otherwise
 */
int mgmt_unlock(mgmt_info_t *info)
{
    return 1;
}

#if 0
#ifdef SDK_BUILD
static void
_mgmt_power_state_linkscan_cb(int unit, soc_port_t port, bcm_port_info_t *info)
{
    if (info->linkstatus) {
        if ((power_state == POWER_DEEP_SLEEP) || (power_state == POWER_SLEEP)) {
            /*
             * The ARM subsystem return to full-speed mode in case of link down
             * to link up transition of any port.
             */
            power_state = POWER_FULL_SPEED;
        }
    }
}
#endif
#endif

/*@api
 * mgmt_dump_buffer
 *
 * @brief
 * Print the contents of a buffer. For debugging.
 *
 * @param=file - FILE pointer to write the data to.
 * @param=name - name of the buffer
 * @param=data - pointer to the data
 * @param=len - length of the data
 * @returns void
 *
 * @desc
 */
void mgmt_dump_buffer(mgmt_info_t *info, char *name, uint8_t *data, int len)
{
    int i;
    char buffer[256] = { 0 };

    mgmt_print(info, "%s: %d bytes", name, len);
    for (i = 0; i < len; ++i) {
        sprintf(&buffer[(i % 16) * 3], "%c%02x", (i % 16) ? ' ' : '\n', data[i] & 0xff);
        if (((i % 16) == 15) || (i == (len - 1))) {
            /* End of line or end of buffer */
            mgmt_print(info, "%s", buffer);
        }
    }
    mgmt_print(info, "\n");
}

/*@api
 * buffer_from_file
 *
 * @brief
 * Create a buffer holding the contents of a file.
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=name - file to read
 * @param=len - pointer to length to return
 * @returns pointer to the data, or NULL
 *
 * @desc
 */
#ifndef VXWORKS
void *buffer_from_file(mgmt_info_t *info, const char *name, uint32_t *len)
{
    FILE *fp;
    int read_len;
    void *buffer;

    fp = fopen(name, "r");
    if (!fp) {
#if MGMT_DEBUG
        perror("fopen");
        mgmt_print(info, "could not open %s\n", name);
#endif
        return NULL;
    }
    if (fseek(fp, 0, SEEK_END) < 0) {
#if MGMT_DEBUG
        perror("fseek");
        mgmt_print(info, "error seeking to end of %s\n", name);
#endif
        fclose(fp);
        return NULL;
    }
    *len = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) < 0) {
#if MGMT_DEBUG
        perror("fseek");
        mgmt_print(info, "error seeking to beginning of %s\n", name);
#endif
        fclose(fp);
        return NULL;
    }
    buffer = sal_alloc(*len, "buffer_from_file");
    if (!buffer) {
#if MGMT_DEBUG
        mgmt_print(info, "could not allocate %d bytes\n", *len);
#endif
        fclose(fp);
        return NULL;
    }
    read_len = fread(buffer, 1, *len, fp);
    fclose(fp);
    if (read_len != (int)*len) {
#if MGMT_DEBUG
        mgmt_print(info, "error reading %s\n", name);
#endif
        sal_free(buffer);
        return NULL;
    }
    return buffer;
}
#endif /* STAND_ALONE */

#ifdef VXWORKS
void *buffer_from_file(mgmt_info_t *info, const char *name, uint32_t *len)
{
#ifdef NO_FILEIO
    mgmt_print(info, "NO FILE IO!\n");
    return NULL;
#else
    FILE *fp;
    int read_len;
    int alloc_len;
    void *buffer = NULL;
    int ch;
    uint8 *pch;

    fp = sal_fopen((char *)name, "r");
    if (!fp) {
        mgmt_print(info, "could not open %s\n", name);
        return NULL;
    }
    alloc_len = 8*1024*1024;
    buffer = sal_alloc(alloc_len, "buffer_from_file");
    pch = (uint8 *)buffer;
    if (!buffer) {
        mgmt_print(info, "could not allocate %d bytes\n", *len);
        sal_fclose(fp);
        return NULL;
    }
    read_len = 0;
    while ((ch = fgetc(fp)) != EOF) {
        read_len++;
        if (read_len > alloc_len) {
            mgmt_print(info, "file size limit is %d\n", alloc_len);
            sal_fclose(fp);
            sal_free(buffer);
            return NULL;
        }
        *pch = (uint8)ch;
        pch++;
    }
    *len = read_len;
    sal_fclose(fp);
    return buffer;
#endif
}
#endif

/*@api
 * is_navigator
 *
 * @brief
 * Identify the Broadcom Navigator device
 *
 * @param=device - libusb_device to examine
 * @returns 1 - Broadcom Navigator detected
 * @returns 0 - Broadcom Navigator not detected
 *
 * @desc
 * Used internally, support function for libusb.
 */
#ifdef MGMT_USB_SUPPORT
static int is_navigator(libusb_device *device)
{
    struct libusb_device_descriptor desc;

    if (libusb_get_device_descriptor(device, &desc)) {
#if MGMT_DEBUG
#ifdef SDK_BUILD
        mgmt_print(info, "error getting device descriptor\n");
#else
        fprintf(stderr, "error getting device descriptor\n");
#endif
#endif
        return 0;
    }
    if ((desc.idVendor == 0x0a5c) && (desc.idProduct == 0xcf5c)) {
        return 1;
    }
    return 0;
}
#endif

/*
 * @api
 * mgmt_read_private_key
 *
 * @brief
 * Read the private RSA key for communication with the BCM53084
 *
 * @param=info - pointer to the mgmt_into_t to use
 * @param=keyfile - name of the file to read
 *
 * @returns pointer to RSA or NULL
 */
#ifdef MGMT_ENCRYPTION_SUPPORT
static RSA *mgmt_read_private_key(mgmt_info_t *info, const char *keyfile)
{
    FILE *fp;
    RSA *rsa;
    EVP_PKEY *pkey;
	PKCS8_PRIV_KEY_INFO *p8inf = NULL;

    /* Read the private key */
    fp = fopen(keyfile, "r");
    if (!fp) {
        perror("fopen");
        mgmt_print(info, "could not open %s\n", keyfile);
        return NULL;
    }
    rsa = NULL;
    if (PEM_read_RSAPrivateKey(fp, &rsa, NULL, NULL)) {
        fclose(fp);
        mgmt_print(info, "private key info read from %s (PEM)\n", keyfile);
        return rsa;
    }
    fclose(fp);

    /* Try as a DER file */
    fp = fopen(keyfile, "r");
    if (!fp) {
        perror("fopen");
        mgmt_print(info, "could not open %s\n", keyfile);
        return NULL;
    }
    rsa = NULL;
    if (!d2i_PKCS8_PRIV_KEY_INFO_fp(fp, &p8inf)) {
        mgmt_print(info, "could not read private key info %s (DER)\n", keyfile);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    /* Extract the RSA key */
    pkey = EVP_PKCS82PKEY(p8inf);
    if (!pkey) {
        mgmt_print(info, "could not extract EVP key from PKCS8 key\n");
        PKCS8_PRIV_KEY_INFO_free(p8inf);
        return NULL;
    }

    rsa = EVP_PKEY_get1_RSA(pkey);
    PKCS8_PRIV_KEY_INFO_free(p8inf);

    if (!rsa) {
        mgmt_print(info, "could not extract RSA key from EVP key\n");
    }
    return rsa;
}
#endif


/*@api
 * mgmt_new
 *
 * @brief
 * Allocate a mgmt_info_t object
 *
 * @param=void
 * @returns pointer to the mgmt_info_t or NULL
 *
 * @desc
 */
mgmt_info_t *mgmt_new(const char *keyfile, const char *certfile, int (*decrypt)(mgmt_info_t *info, const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_len))
{
    mgmt_info_t *info;

    info = (mgmt_info_t *)sal_alloc(sizeof(mgmt_info_t), "Polar Host");
    if (info) {
        sal_memset(info, 0, sizeof(mgmt_info_t));
#ifdef MGMT_SPI_SUPPORT
        info->spifd = -1;
#endif
#ifndef SDK_BUILD
#if MGMT_DEBUG > 0
#ifdef ANDROID
        info->log = fopen("bcm53084log.txt", "w");
#endif
        if (!info->log) {
            info->log = stderr;
        }
#endif
#endif
        mgmt_lock_init(info);

        /* Store the key information */
#ifdef MGMT_ENCRYPTION_SUPPORT
        info->keyfile = keyfile;
        info->certfile = certfile;
        info->decrypt = decrypt ? decrypt : mgmt_decrypt;
#endif
    }

    return info;
}

/*
 * @api
 * mgmt_usb_init
 *
 * @brief
 * Initialize the libusb connection
 *
 * @param=info - pointer to the mgmt_info_t to use for the connection
 *
 * @returns 0 - success
 * @returns !0 - error
 */
#ifdef MGMT_USB_SUPPORT
static int mgmt_usb_init(mgmt_info_t *info)
{
    ssize_t cnt;
    ssize_t i = 0;
    int err;
    int tries = 10;
    libusb_device *found = NULL;

    /* We should probably discover this */
    info->interface = MGMT_INTERFACE;

    if (libusb_init(&info->context) != 0) {
#if MGMT_DEBUG
        perror("libusb_init() failed");
#endif
        mgmt_print(info, "libusb_init() failed\n");
        return -1;
    }

    while ((tries-- >= 0) && !found) {
        cnt = libusb_get_device_list(info->context, &info->list);
        if (cnt < 0) {
            mgmt_print(info, "could not get list of USB devices\n");
            libusb_exit(info->context);
            return -1;
        }

        for (i = 0; i < cnt; i++) {
            libusb_device *device = info->list[i];
            if (is_navigator(device)) {
                found = device;
                break;
            }
        }
        if (!found) {
            sal_sleep(1);
        }
    }
    if (!found) {
        mgmt_print(info, "could not find Broadcom Navigator\n");
        libusb_exit(info->context);
        return -1;
    }

    err = libusb_open(found, &info->handle);
    if (err) {
        perror("libusb_open");
        mgmt_print(info, "libusb_open failed\n");
        libusb_exit(info->context);
        return err;
    }

    err = libusb_kernel_driver_active(info->handle, info->interface);
    if (err < 0) {
        mgmt_print(info, "libusb_kernel_driver_active, err = %d\n", err);
        libusb_close(info->handle);
        libusb_exit(info->context);
        return err;
    }
    else if (err) {
        mgmt_print(info, "kernel driver active\n");
        libusb_close(info->handle);
        libusb_exit(info->context);
        return -1;
    }

    /* The following requires root permissions */
    err = libusb_claim_interface(info->handle, info->interface);
    if (err) {
        mgmt_print(info, "could not claim management interface, err = %d\n", err);
        libusb_close(info->handle);
        libusb_exit(info->context);
        return err;
    }

    return 0;
}
#endif

/*
 * @api
 * mgmt_initialize_connection
 *
 * @brief
 * Initialize the management connection to the BCM53084
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 - success
 * @returns !0 - error
 */

static int mgmt_initialize_connection(mgmt_info_t *info)
{
    int err;

    if (!info->version_valid) {
        err = mgmt_get_version(info, NULL);
        if (err) {
            mgmt_print(info, "could not get version info, err = %d\n", err);
            return err;
        }
    }

#if 0
    switch (info->version.model_id) {
    case 0x89500:   /* Polar */
    case 0x89501:
        break;

    case 0x53084:   /* Navigator */
    default:
        /* Flash init mode does not encrypt/decrypt messages, all other modes do */
        if (info->version.mode != MGMT_FLASH_INIT_MODE) {
#ifdef MGMT_ENCRYPTION_SUPPORT
            /* Request a session key, will fail if Navigator is executing from ROM */
            if (info->certfile) {
                err = mgmt_request_cert_session_key(info);
            }
            else {
                err = mgmt_request_session_key(info);
            }
            if (err) {
                mgmt_print(info, "%s: could not get session key\n", FUNCTION_NAME());
                return err;
            }
        }
        break;
    }
#endif
#endif
    return 0;
}


/*@api
 * mgmt_open_usb
 *
 * @brief
 * Initialize a connection to the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t to use for the connection
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_USB_SUPPORT
int mgmt_open_usb(mgmt_info_t *info)
{
    int err;

    /* Connect through USB */
    err = mgmt_usb_init(info);
    if (err) {
        mgmt_print(info, "mgmt_usb_init: failed\n");
        return err;
    }
    info->connection_type = MGMT_USB_CONNECTION;

    /* Initialize the connection */
    err = mgmt_initialize_connection(info);
    if (err) {
        mgmt_print(info, "mgmt_initialize_connection: failed\n");
        return err;
    }
    return 0;
}
#endif

/*
 * @api
 * _mgmt_spi_mux_aps_set
 *
 * @brief
 * Make SPI MUX point to ARM
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 - success
 * @returns !0 - error
 */
#ifdef SDK_BUILD
static int _mgmt_spi_mux_aps_set(mgmt_info_t *info)
{
    int err;
    soc_regaddrinfo_t ainfo;
    uint32 val32, regaddr = 0x320;
    uint64 val64;

    if (soc_attached(DEFAULT_DEVICE_ID)) {
        /* Stop possible active threads accessing switch registers/tables */
        soc_robo_arl_mode_get(DEFAULT_DEVICE_ID, &info->arl_mode);

        if (info->arl_mode != ARL_MODE_NONE) {
            soc_robo_arl_mode_set(DEFAULT_DEVICE_ID, ARL_MODE_NONE);
        }

        soc_robo_counter_status(DEFAULT_DEVICE_ID,
                                &info->counter_flags,
                                &info->counter,
                                &info->counter_pbmp);

        if (info->counter) {
            soc_robo_counter_stop(DEFAULT_DEVICE_ID);
        }

        err = bcm_robo_linkscan_enable_get(DEFAULT_DEVICE_ID, &info->linkscan);
        if (err == BCM_E_NONE) {
            if (info->linkscan) {
                bcm_robo_linkscan_enable_set(DEFAULT_DEVICE_ID, 0);
            }
        }

        if ((info->arl_mode != ARL_MODE_NONE) ||
            (info->counter != 0) ||
            (info->linkscan != 0)) {
            sal_sleep(1);
        }

        /* Trigger mailbox doorbell interrupt for internal CPU to switch SPI mux. */
        soc_robo_regaddrinfo_get(DEFAULT_DEVICE_ID, &ainfo, regaddr);
        err = soc_robo_anyreg_read(DEFAULT_DEVICE_ID, &ainfo, &val64);
        if (err < 0) {
            mgmt_print(info, "ERROR: soc_robo_anyreg_read failed: %s\n", soc_errmsg(err));
            return -1;
        }
        val32 = COMPILER_64_LO(val64);
        /* Bit 1: EXT_CPU_DOORBELL */
        val32 |= 0x2;
        COMPILER_64_SET(val64, 0, val32);
        err = soc_robo_anyreg_write(DEFAULT_DEVICE_ID, &ainfo, val64);
        if (err < 0) {
            mgmt_print(info, "ERROR: soc_robo_anyreg_write failed: %s\n", soc_errmsg(err));
            return -1;
        }

        if ((power_state == POWER_DEEP_SLEEP) || (power_state == POWER_SLEEP)) {
            /* Doorbell interrupt put ARM back to full speed mode */
            power_state = POWER_FULL_SPEED;
        }
    }

    return 0;
}
#endif

/*@api
 * mgmt_spi_mux_control
 *
 * @brief
 * Implements the SPI MUX selected to ARM
 *
 * @param=info - pointer to mgmt_info_t
 * @param=version - pointer to where to store the version info
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef SDK_BUILD
int mgmt_spi_mux_control(mgmt_info_t *info)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));

    cmd->cmd = uswap32(MGMT_SWITCH_SPI_CONTROL);
    cmd->len = uswap32(MGMT_COMMAND_LEN);
    cmd->magic = uswap32(MGMT_COMMAND_MAGIC);
    /* Do not wait for reply when switching SPI MUX. */
    return mgmt_cmd_reply(info, cmd, (mgmt_reply_t *)NULL, 0);

}
#endif

/*
 * @api
 * _mgmt_spi_mux_switch_set
 *
 * @brief
 * Make SPI MUX point to SWTICH
 *
 * @param=info - pointer to the mgmt_info_t
 *
 * @returns 0 - success
 * @returns !0 - error
 */
#ifdef SDK_BUILD
static int _mgmt_spi_mux_switch_set(mgmt_info_t *info)
{
    int err;

    if (soc_attached(DEFAULT_DEVICE_ID)) {
        err = mgmt_spi_mux_control(info);
        if (err < 0) {
            return err;
        }
        /* Resume previous active threads */
        if ((info->arl_mode != ARL_MODE_NONE) ||
            (info->counter != 0) ||
            (info->linkscan != 0)) {
            sal_sleep(1);
        }

        if (info->arl_mode != ARL_MODE_NONE) {
            soc_robo_arl_mode_set(DEFAULT_DEVICE_ID, info->arl_mode);
        }

        if (info->counter) {
            soc_robo_counter_start(DEFAULT_DEVICE_ID,
                                   info->counter_flags,
                                   info->counter,
                                   info->counter_pbmp);
        }

        if (info->linkscan) {
            bcm_robo_linkscan_enable_set(DEFAULT_DEVICE_ID, info->linkscan);
        }

    }
    return 0;
}
#endif

/*@api
 * mgmt_open_spi
 *
 * @brief
 * Initialize a connection to the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t to use for the connection
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_SPI_SUPPORT
int mgmt_open_spi(mgmt_info_t *info, const char *spidev, uint32_t speed)
{
    int err;
#if !defined(VXWORKS)
    int fd;
#endif

#ifdef SDK_BUILD
    err = _mgmt_spi_mux_aps_set(info);
    if (err < 0) {
        return -1;
    }
#endif

#ifdef VXWORKS
    aps_spi_set_freq(DEFAULT_DEVICE_ID, speed);
#else
    /* Connect through SPI */
    fd = open(spidev, O_RDWR);
    if (fd < 0) {
        perror("open");
        mgmt_print(info, "could not open SPI device %s\n", spidev);
        return MGMT_ERR_SPIDEV_OPEN_FAILED;
    }
    info->spifd = fd;
    info->connection_type = MGMT_SPI_CONNECTION;

	err = ioctl(info->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (err < 0) {
	    perror("ioctl");
	    mgmt_print(info, "SPI set-speed ioctl failed, err = %d\n", err);
	}
#if 1
	err = ioctl(info->spifd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (err < 0) {
	    perror("ioctl");
	    mgmt_print(info, "SPI get-speed ioctl failed, err = %d\n", err);
	}
	mgmt_print(info, "SPI speed %d\n", speed);
#endif
#endif

    /* Initialize the connection */
    err = mgmt_initialize_connection(info);
    if (err) {
        return err;
    }
    return 0;
}
#endif

/*@api
 * mgmt_close_internal
 *
 * @brief
 * Close the connection to the ROBO chip, prepare to reuse mgmt_info_t
 *
 * @param=info - pointer to mgmt_info_t
 * @returns void
 *
 * @desc
 */
static void mgmt_close_internal(mgmt_info_t *info)
{
    switch (info->connection_type) {
    case MGMT_NO_CONNECTION:
        break;

#ifdef MGMT_SPI_SUPPORT
    case MGMT_SPI_CONNECTION:
#ifdef SDK_BUILD
        (void)_mgmt_spi_mux_switch_set(info);
#endif
#if defined(VXWORKS)
        aps_spi_set_freq(DEFAULT_DEVICE_ID, DEFAULT_SPI_CLOCK_SPEED);
#else
        if (info->spifd >= 0) {
            close(info->spifd);
            info->spifd = -1;
        }
#endif
        break;
#endif

#ifdef MGMT_USB_SUPPORT
    case MGMT_USB_CONNECTION:
        if (info->handle) {
            int err;

            err = libusb_release_interface(info->handle, info->interface);
            if (err) {
#if MGMT_DEBUG
                perror("could not release management interface");
#endif
            }
            libusb_close(info->handle);
            libusb_free_device_list(info->list, 1);
            libusb_exit(info->context);
        }
        break;
#endif
    }
}

/*@api
 * mgmt_close
 *
 * @brief
 * Close the connection to the Broadcom Navigator, and free up resources.
 *
 * @param=info - pointer to mgmt_info_t
 * @returns void
 *
 * @desc
 */
void mgmt_close(mgmt_info_t *info)
{
    mgmt_close_internal(info);
    mgmt_lock_uninit(info);
    sal_free(info);
}

/*@api
 * mgmt_spi_encode
 *
 * @brief
 * Encode a buffer for SPI transmission
 *
 * @param=in - pointer to the data to encode
 * @param=len - length of the data to encode
 * @param=out - pointer to the buffer to encode into
 *
 * @returns the length of the encoded data
 *
 */
#ifdef MGMT_SPI_SUPPORT
int mgmt_spi_encode(uint8_t *in, uint32_t inlen, uint8_t *out)
{
    uint32_t i;
    uint32_t len = 0;
    uint32_t lastbyte = 256; /* invalid uint8_t */

    len = 0;
    for (i = 0; i < MGMT_SPI_PREAMBLE_LEN; ++i) {
        out[len++] = MGMT_SPI_PREAMBLE;
    }
    /* encode the packet */
    for (i = 0; i < inlen; ++i) {
        if (in[i] == MGMT_SPI_PREAMBLE) {
            out[len++] = MGMT_SPI_ESCAPE;
            out[len++] = MGMT_SPI_ESCAPE_PREAMBLE;
        }
        else if (in[i] == MGMT_SPI_ESCAPE) {
            out[len++] = MGMT_SPI_ESCAPE;
            out[len++] = MGMT_SPI_ESCAPE_ESCAPE;
        }
        else if (in[i] == MGMT_SPI_DUPLICATE_SEPARATOR) {
            out[len++] = MGMT_SPI_ESCAPE;
            out[len++] = MGMT_SPI_ESCAPE_DUPLICATE;
        }
        else if (in[i] == lastbyte) {
            out[len++] = MGMT_SPI_DUPLICATE_SEPARATOR;
            out[len++] = in[i];
        }
        else {
            out[len++] = in[i];
        }
        lastbyte = out[len-1];
    }
    /* and preamble bytes to end it */
    out[len++] = MGMT_SPI_PREAMBLE;
    out[len++] = MGMT_SPI_PREAMBLE;
    out[len++] = MGMT_SPI_PREAMBLE;
    out[len++] = MGMT_SPI_PREAMBLE;

    return len;
}
#endif

/*@api
 * mgmt_spi_speed
 *
 * @brief
 * Set SPI transmit speed
 *
 * @param=info - pointer to mgmt_info_t to use for the connection
 * @param=speed - speed to set
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_SPI_SUPPORT
int mgmt_spi_speed(mgmt_info_t *info, uint32_t spi_speed)
{
    int err = 0;

#if defined(VXWORKS)
    aps_spi_set_freq(DEFAULT_DEVICE_ID, spi_speed);
#else
    err = ioctl(info->spifd, SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed);
    if (err < 0) {
        perror("ioctl");
        mgmt_print(info, "SPI get-speed ioctl failed, err = %d\n", err);
    }
#endif

    return err;
}
#endif

/*@api
 * mgmt_spi_send
 *
 * @brief
 * Encode and send bytes out the SPI interface.
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=out - pointer to the data to send
 * @param=len - length of the data.
 *
 * @returns 0 - success
 * @returns !0 - error
 *
 */
#ifdef MGMT_SPI_SUPPORT
int mgmt_spi_send(mgmt_info_t *info, uint8_t *data, uint32_t datalen)
{
    int err;
    uint32_t i;
    uint32_t encoded_len;
	uint32_t pkt_len;
    uint8_t cksum;
    uint8_t out[4096];
#if !defined(VXWORKS)
    uint8_t in[4096];
#endif
    uint8_t tmp[4096];
#if defined(VXWORKS)
	int step = 1;
#else
	int step = 8;
#endif

	/* Encode the length, zero the checksum and copy the data */
	pkt_len = datalen + 3;
	tmp[0] = pkt_len & 0xff;
	tmp[1] = (pkt_len >> 8) & 0xff;
	tmp[2] = 0;
	memcpy(&tmp[3], data, datalen);

	/* Compute the checksum */
	cksum = 0;
	for (i = 0; i < pkt_len; ++i) {
	    cksum += tmp[i];
	}

	/* negate the checksum and install */
	tmp[2] = -cksum;

#if MGMT_DEBUG > 1
	mgmt_dump_buffer(info, "spi send plaintext", tmp, pkt_len);
#endif
	/* Encode the packet */
	encoded_len = mgmt_spi_encode(tmp, pkt_len, out);

#if MGMT_DEBUG > 1
	mgmt_dump_buffer(info, "spi send encoded", out, encoded_len);
#endif

	/* Send it out */
	for (i = 0; i < encoded_len; i += step) {
	    uint32_t remaining = encoded_len - i;
#if defined(VXWORKS)
        err = aps_spi_write(DEFAULT_DEVICE_ID, &out[i], (remaining > step) ? step : remaining);
#else
	    struct spi_ioc_transfer transfer = {
	        .tx_buf = (unsigned long)(out + i),
	        .rx_buf = (unsigned long)(in + i),
	        .len = (remaining > step) ? step : remaining
	    };

	    err = ioctl(info->spifd, SPI_IOC_MESSAGE(1), &transfer);
	    if (err < 1) {
	        /* These happen on large transfers, but don't appear fatal */
	        perror("ioctl");
	        mgmt_print(info, "SPI transfer ioctl failed, err = %d\n", err);
	    }
#endif
	}
	return 0;
}
#endif

/*@api
 * mgmt_spi_recv
 *
 * @brief
 * Receive bytes on the SPI interface
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=data - pointer to the buffer to place the data into
 * @param=datalen - pointer to where to store the length of the decoded data received
 *
 * @returns 0 - success
 * @returns !0 - error
 *
 */
#ifdef MGMT_SPI_SUPPORT
int mgmt_spi_recv(mgmt_info_t *info, uint8_t *data, uint32_t *datalen)
{
    int len = 0;
    int timeout = 1000;
    uint32_t last_in = 256;
    int err;
    enum {
        STATE_INITIAL,
        STATE_PREAMBLE,
        STATE_IN_PACKET,
        STATE_IN_ESCAPE,
        STATE_NORMAL
    } state = STATE_INITIAL;

   *datalen = 0;
	while (1) {
	    uint8_t in;
	    uint8_t out;

#if !defined(VXWORKS)
	    struct spi_ioc_transfer transfer = {
	        .tx_buf = (unsigned long)&out,
		    .rx_buf = (unsigned long)&in,
	        .len = 1
	    };
#endif

	    /* Get the next character */
	    out = MGMT_SPI_PREAMBLE;

#if defined(VXWORKS)
        err = aps_spi_read(DEFAULT_DEVICE_ID, &in, 1);
#else
        err = ioctl(info->spifd, SPI_IOC_MESSAGE(1), &transfer);
        if (err < 1) {
            perror("ioctl");
            mgmt_print(info, "SPI transfer ioctl failed, err = %d\n", err);
            return -1;
        }
#endif
#if MGMT_DEBUG > 3
        if (in) {
            mgmt_print(info, "SPI state %d out 0x%02x in 0x%02x\n", state, out, in);
        }
#endif

        /* timeout if needed */
        if (!timeout--) {
            *datalen = 0;
            return 0;
        }

        /* Toss anything that we get before the preamble */
        if (state == STATE_INITIAL) {
            if (in != MGMT_SPI_PREAMBLE) {
                continue;
            }
            /* Preamble byte detected */
            state = STATE_PREAMBLE;
        }

        /* Handle the preamble case */
        if (in == MGMT_SPI_PREAMBLE) {
	        if (state == STATE_PREAMBLE) {
	            continue;
	        }
	        /* must be done - packet already decoded, return */
	        *datalen = len;
	        return 0;
        }

        /* Non-preamble byte, now in packet processing */
        if (state == STATE_PREAMBLE) {
            state = STATE_IN_PACKET;
        }

        /* Discard duplicate bytes */
        if (in == last_in) {
            continue;
        }
        last_in = in;

        /* ignore duplicate separators */
        if (in == MGMT_SPI_DUPLICATE_SEPARATOR) {
            continue;
        }

        /* Escape processing */
        if (in == MGMT_SPI_ESCAPE) {
            state = STATE_IN_ESCAPE;
            continue;
        }
        if (state == STATE_IN_ESCAPE) {
            switch (in) {
            case MGMT_SPI_ESCAPE_PREAMBLE:
                data[len++] = MGMT_SPI_PREAMBLE;
                break;

            case MGMT_SPI_ESCAPE_ESCAPE:
                data[len++] = MGMT_SPI_ESCAPE;
                break;

            case MGMT_SPI_ESCAPE_DUPLICATE:
                data[len++] = MGMT_SPI_DUPLICATE_SEPARATOR;
                break;

            default:
                mgmt_print(info, "spi_recv: invalid escape 0x%02x\n", in);
            }
            state = STATE_IN_PACKET;
            continue;
        }
        /* Just a regular character */
        state = STATE_IN_PACKET;
        data[len++] = in;
	}
	return -1;
}
#endif

/*@api
 * mgmt_spi_cmd
 *
 * @brief
 * Issue a command to the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t
 * @param=cmd - pointer to mgmt_command_t to send
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_SPI_SUPPORT
static int mgmt_spi_cmd(mgmt_info_t *info, mgmt_command_t *cmd)
{
	int err;
	uint32_t len;
	uint8_t rx[sizeof(mgmt_reply_t) + 16];
	uint8_t tx[sizeof(mgmt_command_t) + 16];

	/* Wait for cmd ready */
	do {
	    uint8_t cmd_ready = MGMT_SPI_CMD_READY;

	    err = mgmt_spi_send(info, &cmd_ready, sizeof(cmd_ready));
	    if (err) {
	        mgmt_print(info, "mgmt_spi_cmd, mgmt_spi_send failed (CMD_READY)\n");
            return err;
        }
	    err = mgmt_spi_recv(info, rx, &len);
	    if (err) {
	        mgmt_print(info, "mgmt_spi_cmd, mgmt_spi_recv failed\n");
            return err;
        }
#if MGMT_DEBUG > 1
	    mgmt_dump_buffer(info, "SPI cmd-ready reply", rx, len);
#endif
	} while (rx[3] == 0x00);

	tx[0] = MGMT_SPI_CMD;
	sal_memcpy(&tx[1], cmd, uswap32(cmd->len));

	err = mgmt_spi_send(info, tx, uswap32(cmd->len) + 1);
	if (err) {
	    mgmt_print(info, "mgmt_spi_cmd, mgmt_spi_send failed (CMD)\n");
	    return err;
    }
    return 0;
}
#endif

/*@api
 * mgmt_spi_reply
 *
 * @brief
 * Retrieve a reply to a command.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=reply - pointer to mgmt_reply_t to fill in
 * @param=len - pointer to length to fill in
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_SPI_SUPPORT
static int mgmt_spi_reply(mgmt_info_t *info, mgmt_reply_t *reply, int *rlen)
{
    int retry = 100;

	/* Wait for reply ready */
	while (retry-- > 0) {
	    int err;
	    uint32_t i;
	    uint32_t len;
	    uint32_t spilen;
	    uint8_t cksum;
	    uint8_t reply_ready = MGMT_SPI_REPLY;
	    uint8_t rx[sizeof(mgmt_reply_t) + 16];

	    err = mgmt_spi_send(info, &reply_ready, sizeof(reply_ready));
	    if (err) {
            return err;
        }
	    sal_memset(rx, 0, sizeof(rx));
	    err = mgmt_spi_recv(info, rx, &spilen);
	    if (err) {
	        mgmt_print(info, "mgmt_spi_recv: err = %d\n", err);
            return err;
        }
#if MGMT_DEBUG > 1
	    mgmt_dump_buffer(info, "SPI reply", rx, spilen);
#endif
	    /* verify the checksum and length */
	    len = ((rx[1] << 8) | rx[0]);
	    if (len != spilen) {
	        mgmt_print(info, "mgmt_spi_reply: bad len, spilen = %d len = %d\n",
	                spilen, len);
#if MGMT_DEBUG > 1
	        mgmt_dump_buffer(info, "SPI reply, bad len", rx, 32);
#endif
	        continue;
	    }
	    cksum = 0;
	    for (i = 0; i < spilen; ++i) {
	        cksum += rx[i];
	    }
	    if (cksum) {
	        mgmt_print(info, "mgmt_spi_reply: bad cksum 0x%02x\n", cksum);
	        continue;
	    }
	    /* See if the reply is ready */
	    if (rx[3]) {
	        /* copy reply out and return success */
	        *rlen = spilen - 4;
	        sal_memcpy(reply, &rx[4], *rlen);
	        return 0;
	    }
	    /* Reply not ready, sleep 10ms */
	    sal_usleep(10000);
	}
    mgmt_print(info, "mgmt_spi_reply: retries exhausted\n");
	return -1;
}
#endif

/*@api
 * mgmt_cmd
 *
 * @brief
 * Issue a command to the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t
 * @param=cmd - pointer to mgmt_command_t to send
 * @param=encrypt - flag to request encryption
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_cmd(mgmt_info_t *info, mgmt_command_t *cmd, int encrypt)
{
    int err;
#ifdef MGMT_USB_SUPPORT
    int out_endpoint = MGMT_ENDPOINT | LIBUSB_ENDPOINT_OUT;
    int transferred;
#endif
#ifdef MGMT_ENCRYPTION_SUPPORT
    int encrypt_len;
#endif

    /*
     * The following code enables us to delay getting a session key
     * until we actually need one.
     */
    if (encrypt && !info->version_valid) {
        /* Refresh the version info (non-encrypted, so this won't recurse) */
        err = mgmt_get_version(info, NULL);
        if (err < 0) {
            mgmt_print(info, "%s: could not get version info\n", FUNCTION_NAME());
            return err;
        }
    }

    /* info->version now up-to-date */

#ifdef MGMT_ENCRYPTION_SUPPORT
    if (encrypt && !info->session_key_valid) {
        /* If Navigator in non-Flash Init mode, get session key */
        if ((info->version.model_id == 0x00053084) && (info->version.mode != MGMT_FLASH_INIT_MODE)) {
            /* Request a session key, will fail if Navigator is executing from ROM */
            if (info->certfile) {
                mgmt_print(info, "%s: requesting session key (certificate)\n", FUNCTION_NAME());
                err = mgmt_request_cert_session_key(info);
            }
            else if (info->keyfile) {
                mgmt_print(info, "%s: session key (private key)\n", FUNCTION_NAME());
                err = mgmt_request_session_key(info);
            }
            else {
                mgmt_print(info, "%s: session key (plaintext)\n", FUNCTION_NAME());
                err = mgmt_request_plaintext_session_key(info);
            }
            if (err) {
                mgmt_print(info, "%s: could not get session key\n", FUNCTION_NAME());
                return err;
            }
            /* Now have a valid session key */
            info->session_key_valid = 1;
        }
    }
#endif

    switch (info->connection_type) {
#ifdef MGMT_SPI_SUPPORT
    case MGMT_SPI_CONNECTION:
        break;
#endif

#ifdef MGMT_USB_SUPPORT
    case MGMT_USB_CONNECTION:
        /* If no libusb management handle, error out */
        if (!info->handle) {
            return -1;
        }
        break;
#endif

    default:
        mgmt_print(info, "%s: unknown connection type\n", FUNCTION_NAME());
        return -1;
    }

    /* lock the management interface */
    mgmt_lock(info);

    /* Set the magic number */
    cmd->magic = uswap32(MGMT_COMMAND_MAGIC);

    /* zero out unused bytes in the command structure */
    sal_memset(((uint8_t *)cmd) + uswap32(cmd->len), 0, sizeof(mgmt_command_t) - uswap32(cmd->len));

#if MGMT_DEBUG > 1
    mgmt_dump_buffer(info, "cmd(unencrypted)", (uint8_t *)cmd, uswap32(cmd->len));
#endif

#ifdef MGMT_ENCRYPTION_SUPPORT
    if (info->encrypt_enabled && encrypt) {
#if MGMT_DEBUG > 1
        mgmt_dump_buffer(info, "session key", info->key, sizeof(info->key));
        mgmt_dump_buffer(info, "nonce", info->nonce, sizeof(info->nonce));
        mgmt_dump_buffer(info, "ecount", info->ecount, sizeof(info->ecount));
#endif

        /* magic number and payload get encrypted */
        encrypt_len = uswap32(cmd->len) - MGMT_COMMAND_LEN + sizeof(cmd->magic);
        encrypt_len = ROUND_UP(encrypt_len, AES_BLOCK_SIZE);

        /* Encrypt the (padded) command with the AES session key */
        AES_ctr128_encrypt((unsigned char *)&cmd->magic,
                           (unsigned char *)&cmd->magic,
                           encrypt_len,
                           &info->aes_key,
                           (unsigned char *)info->nonce,
                           (unsigned char *)info->ecount,
                           &info->ctr128_state);
        cmd->len = uswap32(MGMT_COMMAND_LEN - sizeof(cmd->magic) + encrypt_len);
#if MGMT_DEBUG > 1
        mgmt_dump_buffer(info, "cmd(encrypted)", (uint8_t *)cmd, uswap32(cmd->len));
#endif
    }
#endif

    switch (info->connection_type) {
#ifdef MGMT_SPI_SUPPORT
    case MGMT_SPI_CONNECTION:
        err = mgmt_spi_cmd(info, cmd);
        if (err) {
            mgmt_print(info, "%s: could not send SPI data, err = %d\n", FUNCTION_NAME(), err);
            mgmt_unlock(info);
            return err;
        }
        break;
#endif

#ifdef MGMT_USB_SUPPORT
    case MGMT_USB_CONNECTION:
#if 0
        /* Now, read and discard any 'old' replies waiting on the device */
        do {
            int len;
            mgmt_reply_t reply;
            int in_endpoint = MGMT_ENDPOINT | LIBUSB_ENDPOINT_IN;

            err = libusb_bulk_transfer(info->handle, in_endpoint, (void *)&reply,
                                         sizeof(mgmt_reply_t), &len, 20);
            if (err >= 0) {
                mgmt_print(info, "%s: discarding old reply\n", FUNCTION_NAME());
            }
        } while (err >= 0);
#endif
        err = libusb_bulk_transfer(info->handle, out_endpoint,
                                     (void *)cmd, uswap32(cmd->len), &transferred, 0);
        if (err < 0) {
            mgmt_print(info, "%s: could not send USB data, err = %d\n", FUNCTION_NAME(), err);
            mgmt_unlock(info);
            return err;
        }
        break;
#endif

    default:
        mgmt_print(info, "%s: unknown connection type\n", FUNCTION_NAME());
        mgmt_unlock(info);
        return -1;
    }
    return 0;
}

/*@api
 * mgmt_reply
 *
 * @brief
 * Retrieve a reply to a command.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=reply - pointer to mgmt_reply_t to fill in
 * @param=len - pointer to length to fill in
 * @param=decrypt - flag to request decryption
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_reply(mgmt_info_t *info, mgmt_reply_t *reply, int *len, int decrypt)
{
    int err;
#ifdef MGMT_USB_SUPPORT
    int in_endpoint = MGMT_ENDPOINT | LIBUSB_ENDPOINT_IN;
#endif
#ifdef MGMT_ENCRYPTION_SUPPORT
    int decrypt_len;
#endif

    switch (info->connection_type) {
#ifdef MGMT_SPI_SUPPORT
    case MGMT_SPI_CONNECTION:
        err = mgmt_spi_reply(info, reply, len);
        if (err) {
            mgmt_print(info, "%s: could not receive SPI data, err = %d\n", FUNCTION_NAME(), err);
            mgmt_unlock(info);
            return err;
        }
        break;
#endif

#ifdef MGMT_USB_SUPPORT
    case MGMT_USB_CONNECTION:
        err = libusb_bulk_transfer(info->handle, in_endpoint, (void *)reply,
                                     sizeof(mgmt_reply_t), len, 0);
        if (err < 0) {
            mgmt_print(info, "%s: could not receive USB data, err = %d\n", FUNCTION_NAME(), err);
            mgmt_unlock(info);
            return err;
        }
        break;
#endif

    default:
        mgmt_unlock(info);
        return -1;
    }
#if MGMT_DEBUG > 1
    mgmt_print(info, "%s: status %d len %d magic 0x%08x\n",
           FUNCTION_NAME(), uswap32(reply->status), uswap32(reply->len), uswap32(reply->magic));
    mgmt_dump_buffer(info, "reply", reply, reply->len);
#endif
    if (uswap32(reply->status)) {
        mgmt_print(info, "%s: error status %d/%x\n", FUNCTION_NAME(), uswap32(reply->status), uswap32(reply->status));
        mgmt_unlock(info);
        return -1;
    }

    if (uswap32(reply->len) < MGMT_STATUS_LEN) {
        mgmt_print(info, "%s: truncated reply\n", FUNCTION_NAME());
        mgmt_unlock(info);
        return -1;
    }
#ifdef MGMT_ENCRYPTION_SUPPORT
    /* Detect unexpected plaintext replies (usually errors) */
    if (uswap32(reply->magic) == MGMT_REPLY_MAGIC) {
        if (info->encrypt_enabled && decrypt) {
            mgmt_print(info, "%s: unexpected plaintext reply\n", FUNCTION_NAME());
        }
        decrypt = 0;
    }
    decrypt_len = uswap32(reply->len) - MGMT_STATUS_LEN + sizeof(reply->magic);
    decrypt_len = ROUND_UP(decrypt_len, AES_BLOCK_SIZE);
    if (info->encrypt_enabled && decrypt && decrypt_len) {
        /* Decrypt the reply with the AES session key */
        AES_ctr128_encrypt((unsigned char *)&reply->magic,
                           (unsigned char *)&reply->magic,
                           decrypt_len,
                           &info->aes_key,
                           (unsigned char *)info->nonce,
                           (unsigned char *)info->ecount,
                           &info->ctr128_state);
    }
#endif
    if (uswap32(reply->magic) != MGMT_REPLY_MAGIC) {
        mgmt_print(info, "%s: bad magic number in reply, 0x%08x\n", FUNCTION_NAME(), uswap32(reply->magic));
        mgmt_unlock(info);
        return -1;
    }

    mgmt_unlock(info);
    return 0;
}

/*@api
 * mgmt_cmd
 *
 * @brief
 * Issue a command to the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t
 * @param=cmd - pointer to mgmt_command_t to send
 * @param=reply - pointer to mgmt_reply_t to fill in
 * @param=encrypt - flag to request encryption/decryption
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_cmd_reply(mgmt_info_t *info, mgmt_command_t *cmd, mgmt_reply_t *reply, int encrypt)
{
    int len;
    int err;

    err = mgmt_cmd(info, cmd, encrypt);
    if (err) {
        mgmt_print(info, "%s: could not send command\n", FUNCTION_NAME());
        return err;
    }
    if (reply) {
        err = mgmt_reply(info, reply, &len, encrypt);
        if (err) {
            mgmt_print(info, "%s: could not get reply\n", FUNCTION_NAME());
            return err;
        }
        if (reply->status != MGMT_SUCCESS) {
            mgmt_print(info, "%s: error in reply status=%d\n", FUNCTION_NAME(), uswap32(reply->status));
            return -1;
        }
    }
    return 0;
}

/*@api
 * mgmt_power_up
 *
 * @brief
 * Powers up the PLL to full speed
 *
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_power_up(mgmt_info_t *info)
{
#ifdef MGMT_SPI_SUPPORT
    int err;
    int dmu_mode = 0;
    int delay = 0;
    mgmt_reply_version_t version;
    uint8_t *pll_image;
    uint32_t pll_image_size;

    /* Must start in SPI mode */
    if (info->connection_type != MGMT_SPI_CONNECTION) {
        return -1;
    }

    /* Download and run the PLL image via SPI */

    /* Get the version info */
    err = mgmt_get_version(info, &version);
    if (err) {
        mgmt_print(info, "%s: mgmt_get_version() failed, err = %d\n", FUNCTION_NAME(), err);
        return err;
    }

    /* This only works if we're in ROM mode */
    if (version.mode != MGMT_FLASH_INIT_MODE) {
        mgmt_print(info, "%s: part not in FLASH-INIT mode\n", FUNCTION_NAME());
        return -1;
    }

    /* Determine the DMU mode to use */
    if (version.model_id == 0x00053084) {
        /* Navigator */
        dmu_mode = 0;
        delay = 500000;
        pll_image = mgmt_bcm53084_pll_image;
        pll_image_size = mgmt_bcm53084_pll_image_size;
    }
    else {
        /* Polar */
        dmu_mode = 1;
        pll_image = mgmt_bcm89500_pll_image;
        pll_image_size = mgmt_bcm89500_pll_image_size;
    }

    err = mgmt_download_image(info, pll_image, pll_image_size);
    if (err) {
        mgmt_print(info, "download of PLL image failed, err = %d\n", err);
        return err;
    }

    /* Now try to execute the PLL code */
    err = mgmt_execute_image(info, pll_image, dmu_mode, delay, 0, 0);
    if (err) {
        mgmt_print(info, "error executing PLL image, err = %d\n", err);
        return err;
    }

#if 0
#ifdef MGMT_USB_SUPPORT
    /* On Navigator, replace the SPI connection with a USB one */
    if (version.model_id == 0x00053084) {

        /* Close the SPI connection */
        mgmt_close_internal(info);

        /* Open the USB connection (for firmware download) */
        err = mgmt_open_usb(info);
        if (err) {
            mgmt_print(info, "could not open USB mgmt connection, err = %d\n", err);
            return err;
        }
    }
#endif
#endif
    return 0;
#else
    return -1;
#endif
}

/*@api
 * mgmt_get_version
 *
 * @brief
 * Implements the MGMT_VERSION_INFO command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=version - pointer to where to store the version info
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_get_version(mgmt_info_t *info, mgmt_reply_version_t *version)
{
    int err;
    uint32_t i;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_VERSION_INFO);
    cmd->len = uswap32(MGMT_COMMAND_LEN);
    cmd->magic = uswap32(MGMT_COMMAND_MAGIC);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        return err;
    }

    /* Update info->version */

    info->version.mode = uswap32(reply->u.version.mode);
    info->version.mos_version = uswap32(reply->u.version.mos_version);
    for (i = 0; i < sizeof(reply->u.version.args)/sizeof(reply->u.version.args[0]); ++i) {
        info->version.args[i] = uswap32(reply->u.version.args[i]);
    }
    info->version.model_id = uswap32(reply->u.version.model_id);
    info->version.chip_id = uswap32(reply->u.version.chip_id);
    info->version.otp_bits = uswap32(reply->u.version.otp_bits);
    info->version_valid = 1;
#if MGMT_DEBUG > 0
    mgmt_print(info, "%s: model = 0x%08x mode = %d version = 0x%08x\n",
            FUNCTION_NAME(), info->version.model_id, info->version.mode, info->version.mos_version);
#endif

    /* Copy out info->version */
    if (version) {
        sal_memcpy(version, &info->version, sizeof(mgmt_reply_version_t));
    }
    return 0;
}

/*@api
 * mgmt_decrypt
 *
 * @brief
 * Default decryption routine
 *
 * @param=info - pointer to the mgmt_into_t to use
 * @param=in - input buffer
 * @param=in_len - input buffer len
 * @param=out - output buffer
 * @param=out_len - output buffer len
 *
 * @returns 0 - success
 * @returns !0 - error
 */
#ifdef MGMT_ENCRYPTION_SUPPORT
static int mgmt_decrypt(mgmt_info_t *info, const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_len)
{
    int err;
    RSA *rsa;

    if (!info->keyfile) {
        mgmt_print(info, "%s: no keyfile\n", FUNCTION_NAME());
        return -1;
    }

    rsa = mgmt_read_private_key(info, info->keyfile);
    if (!rsa) {
        mgmt_print(info, "%s: could not read private key\n", FUNCTION_NAME());
        return -1;
    }

    /* decrypt the session key using the RSA private key read earlier */
    err = RSA_private_decrypt(in_len, in, out, rsa, RSA_PKCS1_PADDING);
    if (err < 0) {
#if MGMT_DEBUG
        char buf[256];
        unsigned long e;

        mgmt_print(info, "RSA_private_decrypt returns %d\n", err);
        ERR_load_crypto_strings();
        e = ERR_get_error();
        ERR_error_string(e, buf);
        mgmt_print(info, "ERR: %lu: %s\n", e, buf);
#endif
        return err;
    }
    /* Success */
    return 0;
}
#endif

/*@api
 * mgmt_request_session_key
 *
 * @brief
 * Requests a new session key from the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_ENCRYPTION_SUPPORT
int mgmt_request_session_key(mgmt_info_t *info)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SESSION_KEY);
    cmd->len = uswap32(MGMT_COMMAND_LEN);
    cmd->magic = uswap32(MGMT_COMMAND_MAGIC);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        return err;
    }

    /* extract the NONCE */
    sal_memcpy(info->nonce, reply->u.session.nonce, sizeof(info->nonce));
#if MGMT_DEBUG > 1
    mgmt_dump_buffer(info, "nonce", info->nonce, sizeof(info->nonce));
#endif

    /* initialize the ecount */
    info->ctr128_state = 0;
    sal_memset(info->ecount, 0, sizeof(info->ecount));

#if MGMT_DEBUG > 1
    mgmt_dump_buffer(info, "pkcs encrypted session key",
                reply->u.session.message, sizeof(reply->u.session.message));
#endif

    err = (*info->decrypt)(info, reply->u.session.message, sizeof(reply->u.session.message),
                           info->key, sizeof(info->key));

    /* err holds length of recovered plaintext */
#if MGMT_DEBUG > 1
    mgmt_print(info, "plaintext length %d\n", err);
    mgmt_dump_buffer(info, "session key", info->key, MGMT_SESSION_KEY_LEN);
#endif
    /* Initialize the encryption/decryption contexts */

    /* If the session key isn't 128-bit, the call below needs to change */
#if 0
    assert(MGMT_SESSION_KEY_LEN == (128/8));
#endif
    AES_set_encrypt_key((unsigned char *)info->key, 128, &info->aes_key);

    info->encrypt_enabled = 1;

    return 0;
}
#endif

/*@api
 * mgmt_request_plaintext_session_key
 *
 * @brief
 * Requests a new session key from the Broadcom Navigator
 *
 * @param=info - pointer to mgmt_info_t
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_ENCRYPTION_SUPPORT
int mgmt_request_plaintext_session_key(mgmt_info_t *info)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_PLAINTEXT_SESSION_KEY);
    cmd->len = uswap32(MGMT_COMMAND_LEN);
    cmd->magic = uswap32(MGMT_COMMAND_MAGIC);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        return err;
    }

    /* extract the NONCE */
    sal_memcpy(info->nonce, reply->u.session.nonce, sizeof(info->nonce));
#if MGMT_DEBUG > 1
    mgmt_dump_buffer(info, "nonce", info->nonce, sizeof(info->nonce));
#endif

    /* initialize the ecount */
    info->ctr128_state = 0;
    sal_memset(info->ecount, 0, sizeof(info->ecount));

    /* Extract the key */
    sal_memcpy(info->key, reply->u.session.message, MGMT_SESSION_KEY_LEN);

    /* Initialize the encryption/decryption contexts */

    /* If the session key isn't 128-bit, the call below needs to change */
#if 0
    assert(MGMT_SESSION_KEY_LEN == (128/8));
#endif
    AES_set_encrypt_key((unsigned char *)info->key, 128, &info->aes_key);

    info->encrypt_enabled = 1;

    return 0;
}
#endif

/*@api
 * mgmt_request_cert_session_key
 *
 * @brief
 * Requests a new session key, using a certificate
 *
 * @param=info - pointer to mgmt_info_t
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
#ifdef MGMT_ENCRYPTION_SUPPORT
int mgmt_request_cert_session_key(mgmt_info_t *info)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    int buffer = 0;         /* use buffer 0 */
    uint32_t len;
    uint32_t offset;
    uint32_t data_length;
    uint8_t *data;

    if (!info->certfile) {
        mgmt_print(info, "no certificate file provided\n");
        return -1;
    }
    data = buffer_from_file(info, info->certfile, &data_length);
    if (!data) {
        mgmt_print(info, "error reading certificate file %s\n", info->certfile);
        return -1;
    }

    /* Write to the buffer */
    err = mgmt_write_buffer(info, 0, buffer, data_length, data);
    if (err) {
        mgmt_print(info, "%s: error writing to buffer\n", FUNCTION_NAME());
        sal_free(data);
        return -1;
    }

    /* Free the data */
    sal_free(data);

    /* Request a session key based on the cert in the buffer */

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_CERT_SESSION_KEY);
    cmd->len = uswap32(MGMT_COMMAND_LEN) + sizeof(cmd->u.cert_session_key);
    cmd->magic = uswap32(MGMT_COMMAND_MAGIC);
    cmd->u.cert_session_key.buffer = uswap32(buffer);
    cmd->u.cert_session_key.length = uswap32(data_length);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        mgmt_print(info, "MGMT_CERT_SESSION_KEY failed\n");
        return err;
    }

    /* extract the NONCE */
    sal_memcpy(info->nonce, reply->u.session.nonce, sizeof(info->nonce));
#if MGMT_DEBUG > 1
    mgmt_dump_buffer(info, "nonce", info->nonce, sizeof(info->nonce));
#endif

    /* initialize the ecount */
    info->ctr128_state = 0;
    sal_memset(info->ecount, 0, sizeof(info->ecount));

    /* decrypt the session key using the RSA private key read earlier */
#if MGMT_DEBUG > 1
    mgmt_dump_buffer(info, "pkcs encrypted session key",
                reply->u.session.message, sizeof(reply->u.session.message));
#endif

    err = (*info->decrypt)(info, reply->u.session.message, sizeof(reply->u.session.message),
                           info->key, sizeof(info->key));
    if (err < 0) {
        mgmt_print(info, "%s: info->decrypt returned error\n", FUNCTION_NAME());
        return err;
    }

    /* err holds length of recovered plaintext */
#if MGMT_DEBUG > 1
    mgmt_print(info, "plaintext length %d\n", err);
    mgmt_dump_buffer(info, "session key", info->key, MGMT_SESSION_KEY_LEN);
#endif
    /* Initialize the encryption/decryption contexts */

    /* If the session key isn't 128-bit, the call below needs to change */
#if 0
    assert(MGMT_SESSION_KEY_LEN == (128/8));
#endif
    AES_set_encrypt_key((unsigned char *)info->key, 128, &info->aes_key);

    info->encrypt_enabled = 1;

    return 0;
}
#endif

/*@api
 * mgmt_set_macaddr
 *
 * @brief
 * Implements the MGMT_SET_MACADDR command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=addr - pointer to the mac address
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_set_macaddr(mgmt_info_t *info, void *addr)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SET_MACADDR);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.macaddr));
    sal_memcpy(cmd->u.macaddr.addr, addr, sizeof(cmd->u.macaddr));

    return mgmt_cmd_reply(info, cmd, reply, 1);
}

/*@api
 * mgmt_get_flash_info
 *
 * @brief
 * Implements the MGMT_FLASH_INFO command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=flash_info - pointer to the flash_info_t to fill in
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_get_flash_info(mgmt_info_t *info, int spi, flash_info_t *finfo)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;
    flash_info_t *flash_info;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_FLASH_INFO);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.flash_info));
    cmd->u.flash_info.spi = uswap32(spi);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    flash_info = spiflash_info_by_rdid(uswap32(reply->u.flash_info.rdid));
    if (!flash_info) {
        mgmt_print(info, "could not find flash-info for RDID 0x%08x\n", uswap32(reply->u.flash_info.rdid));
        return -1;
    }
    sal_memcpy(finfo, flash_info, sizeof(flash_info_t));

    return 0;
}

/*@api
 * mgmt_read_switch_register
 *
 * @brief
 * Implements the MGMT_SWITCH_READ_REG command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=addr - switch register to read
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_read_switch_register(mgmt_info_t *info, uint32_t addr, uint32_t size, uint64_t *regval)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SWITCH_READ_REG);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.switch_read));
    cmd->u.switch_read.size = uswap32(size);
    cmd->u.switch_read.address = uswap32(addr);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }

    *regval = ((uint64_t)uswap32(reply->u.switch_read.upper_regval) << 32)
               | uswap32(reply->u.switch_read.lower_regval);
    return 0;
}

/*@api
 * mgmt_write_switch_register
 *
 * @brief
 * Implements the MGMT_SWITCH_WRITE_REG command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=addr - switch register to write
 * @param=regval - value to write
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_write_switch_register(mgmt_info_t *info, uint32_t addr, uint32_t size, uint64_t regval)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SWITCH_WRITE_REG);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.switch_write));
    cmd->u.switch_write.size = uswap32(size);
    cmd->u.switch_write.address = uswap32(addr);
    cmd->u.switch_write.upper_regval = uswap32((uint32_t)(regval >> 32));
    cmd->u.switch_write.lower_regval = uswap32((uint32_t)regval);

    return mgmt_cmd_reply(info, cmd, reply, 1);
}

/*@api
 * mgmt_erase_flash_sector
 *
 * @brief
 * Implements the MGMT_FLASH_ERASE_SECTOR command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=page - page # of the sector to erase
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_erase_flash_sector(mgmt_info_t *info, int spi, uint32_t page)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_FLASH_ERASE_SECTOR);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.flash_erase));
    cmd->u.flash_erase.spi = uswap32(spi);
    cmd->u.flash_erase.address = uswap32(page * MGMT_FLASH_PAGE_SIZE);

    return mgmt_cmd_reply(info, cmd, reply, 1);
}

/*@api
 * mgmt_erase_flash_page
 *
 * @brief
 * Implements the MGMT_FLASH_ERASE_PAGE command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=page - page # of the page to erase
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_erase_flash_page(mgmt_info_t *info, int spi, uint32_t page)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_FLASH_ERASE_PAGE);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.flash_erase));
    cmd->u.flash_erase.spi = uswap32(spi);
    cmd->u.flash_erase.address = uswap32(page * MGMT_FLASH_PAGE_SIZE);

    return mgmt_cmd_reply(info, cmd, reply, 1);
}

/*@api
 * mgmt_erase_flash
 *
 * @brief
 * Erase a range of flash
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=start - first page to erase
 * @param=end - one past last page to erase
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_erase_flash(mgmt_info_t *info, int spi, int start, int end)
{
    int page;
    int err;
    flash_info_t flash_info;

    mgmt_print(info, "mgmt_erase_flash: spi=%d start=%d end=%d\n", spi, start, end);

    err = mgmt_get_flash_info(info, spi, &flash_info);
    if (err) {
        mgmt_print(info, "mgmt_erase_flash: could not get flash info, err=%d \n", err);
        return err;
    }

    /* Erase the pages */
    page = start;
    while (page < end) {
        err = mgmt_erase_flash_page(info, spi, page);
        if (err) {
            mgmt_print(info, "mgmt_erase_flash: error erasing page %d\n", page);
            return err;
        }
        page += flash_info.erase_page_size / MGMT_FLASH_PAGE_SIZE;
    }
    return 0;
}

/*@api
 * mgmt_set_macsec_key
 *
 * @brief
 * Implements the MGMT_SET_MACSEC_KEY command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=keyinfo - address of the key info to send
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_set_macsec_key(mgmt_info_t *info, char *keyinfo)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SET_MACSEC_KEY);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.macsec_key));
    sal_strcpy((char *)cmd->u.macsec_key.data, keyinfo);

    return mgmt_cmd_reply(info, cmd, reply, 1);
}

/*@api
 * mgmt_read_flash_page
 *
 * @brief
 * Implements the MGMT_FLASH_PAGE_READ command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=page - page number to read
 * @param=data - pointer to buffer to return the data in
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_read_flash_page(mgmt_info_t *info, int spi, uint32_t page, uint8_t *data)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_FLASH_PAGE_READ);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.flash_read));
    cmd->u.flash_read.spi = uswap32(spi);
    cmd->u.flash_read.address = uswap32(page * MGMT_FLASH_PAGE_SIZE);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }

    /* copy out the data */
    sal_memcpy(data, reply->u.flash_read.data, MGMT_FLASH_PAGE_SIZE);
    return 0;
}

/*@api
 * mgmt_write_flash_page
 *
 * @brief
 * Implements the MGMT_FLASH_PAGE_WRITE command/reply exchange
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=page - page number to write
 * @param=data - pointer to data to write
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_write_flash_page(mgmt_info_t *info, int spi, uint32_t page, uint8_t *data)
{
    int i;
    int err;
    uint8_t verify[MGMT_FLASH_PAGE_SIZE];

    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    /* check the page is erased */
    err = mgmt_read_flash_page(info, spi, page, verify);
    if (err) {
        return err;
    }
    err = 0;
    for (i = 0; i < MGMT_FLASH_PAGE_SIZE; ++i) {
        if (verify[i] != 0xff) {
#if MGMT_DEBUG
            mgmt_print(info, "flash page %d failed erase, offset %d got 0x%02x\n",
                    page, i, verify[i]);
            mgmt_dump_buffer(info, "verify", verify, MGMT_FLASH_PAGE_SIZE);
#endif
            return -1;
        }
    }

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_FLASH_PAGE_WRITE);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.flash_write));
    cmd->u.flash_write.spi = uswap32(spi);
    cmd->u.flash_write.address = uswap32(page * MGMT_FLASH_PAGE_SIZE);

    /* copy in the data */
    sal_memcpy(cmd->u.flash_write.data, data, MGMT_FLASH_PAGE_SIZE);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }

    /* Write successful, read back data to verify */
    err = mgmt_read_flash_page(info, spi, page, verify);
    if (err) {
        return err;
    }
    for (i = 0; i < MGMT_FLASH_PAGE_SIZE; ++i) {
        if (data[i] != verify[i]) {
#if MGMT_DEBUG
            mgmt_print(info, "flash page %d failed verify, offset %d got 0x%02x expected 0x%02x\n",
                    page, i, verify[i], data[i]);
            mgmt_dump_buffer(info, "data", data, MGMT_FLASH_PAGE_SIZE);
            mgmt_dump_buffer(info, "verify", verify, MGMT_FLASH_PAGE_SIZE);
#endif
            return -1;
        }
    }
    return 0;
}

/*@api
 * mgmt_read_flash
 *
 * @brief
 * Reads a range of pages, writing the data to a file
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=startpage - first page to read
 * @param=endpage - last page to read + 1
 * @param=fp - FILE pointer to write the data to.
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_read_flash(mgmt_info_t *info, int spi, int startpage, int endpage, FILE *fp)
{
    int page;
    int err;
    uint8_t data[MGMT_FLASH_PAGE_SIZE];

    for (page = startpage; page < endpage; ++page) {
        err = mgmt_read_flash_page(info, spi, page, data);
        if (err) {
            mgmt_print(info, "error reading flash page %d\n", page);
            return err;
        }
        fwrite(data, 1, MGMT_FLASH_PAGE_SIZE, fp);
    }
    return 0;
}

/*@api
 * mgmt_write_flash
 *
 * @brief
 * Reads a file, writing a range of pages to flash
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=startpage - first page to write
 * @param=buffer - buffer to write
 * @param=len - length of the data
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_write_flash(mgmt_info_t *info, int spi, int startpage, uint8_t *buffer, uint32_t len)
{
    int page;
    int err;

    page = startpage;
    mgmt_print(info, "mgmt_write_flash: spi=%d startpage=%d buffer=%p len=%d\n",
           spi, startpage, buffer, len);
    for ( ; len > 0; page++, buffer += MGMT_FLASH_PAGE_SIZE) {
        int i;
        int write_page = 0;
        uint8_t data[MGMT_FLASH_PAGE_SIZE];

        if (len > MGMT_FLASH_PAGE_SIZE) {
            sal_memcpy(data, buffer, MGMT_FLASH_PAGE_SIZE);
            len -= MGMT_FLASH_PAGE_SIZE;
        }
        else {
            sal_memset(data, 0xff, MGMT_FLASH_PAGE_SIZE);
            sal_memcpy(data, buffer, len);
            len = 0;
        }

        /* only write pages with non-erased content */
        for (i = 0; i < SPIFLASH_PAGE_SIZE; ++i) {
            if (buffer[i] != 0xff) {
                write_page = 1;
                break;
            }
        }
        if (write_page) {
            err = mgmt_write_flash_page(info, spi, page, buffer);
            if (err) {
                mgmt_print(info, "error writing flash page %d\n", page);
                return err;
            }
        }
    }
    return 0;
}

/*@api
 * mgmt_read_flash_header
 *
 * @brief
 * Reads the flash header/toc block from the flash and validates it
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=page - page to read the header from
 * @param=header - flash_header_t to fill in
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_read_flash_header(mgmt_info_t *info, int spi, int page, flash_header_t *header)
{
    int err;
    uint32_t i;
    uint32_t cksum;
    uint32_t data[MGMT_FLASH_PAGE_SIZE/sizeof(uint32_t)];

    err = mgmt_read_flash_page(info, spi, page, (uint8_t *)data);
    if (err) {
        mgmt_print(info, "error reading flash header from page %d\n", page);
        return err;
    }
    sal_memcpy(header, data, sizeof(flash_header_t));

    header->magic = uswap32(header->magic);
    header->cksum = uswap32(header->cksum);
    header->spi_speed = uswap32(header->spi_speed);
    header->dmu_config = uswap32(header->dmu_config);
    for (i = 0; i < FLASH_NUM_BOOT_IMAGES; ++i) {
        header->boot_image[i] = uswap32(header->boot_image[i]);
    }
    for (i = 0; i < FLASH_NUM_IMAGES; ++i) {
        header->image_offset[i] = uswap32(header->image_offset[i]);
    }
    for (i = 0; i < FLASH_NUM_CONFIGS; ++i) {
        header->config_offset[i] = uswap32(header->config_offset[i]);
    }
    for (i = 0; i < FLASH_NUM_CORE_DUMPS; ++i) {
        header->core_dump_offset[i] = uswap32(header->core_dump_offset[i]);
    }
    for (i = 0; i < FLASH_NUM_AVB_CONFIGS; ++i) {
        header->expansion[AVB_CFG_EXP_OFFSET + i].offset = uswap32(header->expansion[AVB_CFG_EXP_OFFSET + i].offset);
    }
    header->expansion[VPD_EXP_OFFSET].offset = uswap32(header->expansion[VPD_EXP_OFFSET].offset);
    /* Validate the header */
    if (header->magic != FLASH_HEADER_MAGIC) {
        mgmt_print(info, "flash header page %d: bad magic number\n", page);
        mgmt_dump_buffer(info, "flash header", (uint8_t *)data, sizeof(flash_header_t));
        return -1;
    }
    cksum = 0;
    for (i = 0; i < sizeof(flash_header_t)/sizeof(uint32_t); ++i) {
        cksum += uswap32(data[i]);
    }
    if (cksum) {
        mgmt_print(info, "flash header page %d: bad checksum 0x%08x\n", page, cksum);
        mgmt_dump_buffer(info, "flash header", (uint8_t *)data, sizeof(flash_header_t));
        return -1;
    }
    return 0;
}

/*@api
 * mgmt_write_flash_header
 *
 * @brief
 * Writes the flash header/toc block to the flash
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=page - page to write the header to
 * @param=header - flash_header_t to write
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_write_flash_header(mgmt_info_t *info, int spi, int page, flash_header_t *header)
{
    int err;
    uint32_t i;
    flash_header_t sheader;                 /* swapped header */
    uint8_t data[MGMT_FLASH_PAGE_SIZE];

    /* fill the page with 0xff before copying in the flash_header_t */
    sal_memset(data, 0xff, MGMT_FLASH_PAGE_SIZE);

    sal_memset(&sheader, 0, sizeof(flash_header_t));
    sheader.magic = uswap32(header->magic);
    sheader.cksum = uswap32(header->cksum);
    sheader.spi_speed = uswap32(header->spi_speed);
    sheader.dmu_config = uswap32(header->dmu_config);
    for (i = 0; i < FLASH_NUM_BOOT_IMAGES; ++i) {
        sheader.boot_image[i] = uswap32(header->boot_image[i]);
    }
    for (i = 0; i < FLASH_NUM_IMAGES; ++i) {
        sheader.image_offset[i] = uswap32(header->image_offset[i]);
    }
    for (i = 0; i < FLASH_NUM_CONFIGS; ++i) {
        sheader.config_offset[i] = uswap32(header->config_offset[i]);
    }
    for (i = 0; i < FLASH_NUM_CORE_DUMPS; ++i) {
        sheader.core_dump_offset[i] = uswap32(header->core_dump_offset[i]);
    }
    for (i = 0; i < FLASH_NUM_AVB_CONFIGS; ++i) {
        sheader.expansion[AVB_CFG_EXP_OFFSET + i].offset = uswap32(header->expansion[AVB_CFG_EXP_OFFSET + i].offset);
    }
    sheader.expansion[VPD_EXP_OFFSET].offset = uswap32(header->expansion[VPD_EXP_OFFSET].offset);

    sal_memcpy(data, &sheader, sizeof(flash_header_t));

    err = mgmt_write_flash_page(info, spi, page, data);
    if (err) {
        mgmt_print(info, "error writing flash header\n");
        return err;
    }
    return 0;
}


/*@api
 * mgmt_calculate_flash_header
 *
 * @brief
 * Calculates the flash header based on the input parameters
 *
 * @param=info - pointer to mgmt_info_t
 * @param=flash_info - pointer to the flash_info_t for the flash
 * @param=header - pointer to the flash_header_t to fill in
  * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_calculate_flash_header(mgmt_info_t *info,
                                flash_info_t *flash_info,
                                flash_header_t *header,
                                uint32_t dmu_config,
                                uint32_t spi_speed)
{
    return spiflash_calculate_header(flash_info, header, dmu_config, spi_speed);
}

/*@api
 * mgmt_download_image
 *
 * @brief
 * Downloads the specified ARM firmware image
 *
 * @param=info - pointer to mgmt_info_t
 * @param=armbuf - pointer to the ARM firmware image
 * @param=armlen - length of the ARM firmware image
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_download_image(mgmt_info_t *info, uint8_t *armbuf, uint32_t armlen)
{
    int err;
    uint32_t i;
    uint32_t download_len = armlen - 2*MGMT_MEMORY_PAGE_SIZE;
    uint8_t *download_start = &armbuf[2*MGMT_MEMORY_PAGE_SIZE];

    for (i = 0; i < download_len; i += MGMT_MEMORY_PAGE_SIZE) {
        err = mgmt_write_memory(info, i, &download_start[i]);
        if (err) {
            mgmt_print(info, "error downloading ARM image, err = %d\n", err);
            return err;
        }
    }

    return 0;
}

/*@api
 * mgmt_install_flash_header
 *
 * @brief
 * Write the flash TOC to the SPI flash
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI flash to refer to
 * @param=header - pointer to the flash_header_t to fill in
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 * Install the specified flash header into the SPI flash.
 */
int mgmt_install_flash_header(mgmt_info_t *info, int spi, flash_header_t *header)
{
    int err;
    uint32_t page;
    uint32_t i;
    uint32_t cksum;
    flash_info_t flash_info;

    err = mgmt_get_flash_info(info, spi, &flash_info);
    if (err) {
        mgmt_print(info, "mgmt_install_flash_header: could not get flash info, err=%d\n", err);
        return err;
    }

    /* Calculate and install the checksum */
    cksum = 0;
    header->cksum = 0;
    for (i = 0; i < sizeof(flash_header_t)/sizeof(uint32_t); ++i) {
        cksum += ((uint32_t *)header)[i];
    }
    /* Install the negation of the checksum, so on the target it results in 0 */
    header->cksum = -cksum;

    /* Erase the first two sectors in turn, installing the flash headers */
    page = 0;
    while (page < (flash_info.erase_page_size * FLASH_NUM_HEADER_BLOCKS) / MGMT_FLASH_PAGE_SIZE) {
        err = mgmt_erase_flash_page(info, spi, page);
        if (err) {
            mgmt_print(info, "mgmt_install_flash_header: error erasing page %d\n", page);
            return err;
        }
        /* Now install the headers into the erased pages */
        for (i = 0; i < flash_info.erase_page_size / MGMT_FLASH_PAGE_SIZE; ++i) {
            err = mgmt_write_flash_header(info, spi, page + i, header);
            if (err) {
                mgmt_print(info, "mgmt_install_flash_header: error writing flash header to page %d\n", page + i);
                return err;
            }
        }
        page += flash_info.erase_page_size / MGMT_FLASH_PAGE_SIZE;
    }
    return 0;
}

/*@api
 * mgmt_install_arm_image
 *
 * @brief
 * Installs the specified ARM image into the flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=header - pointer to the flash_header_t
 * @param=slot - slot to install the image in
 * @param=arm_image - path to the ARM image to install
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_install_arm_image(mgmt_info_t *info, int spi,
                           flash_header_t *header,
                           int slot,
                           uint8_t *armbuf,
                           uint32_t armlen)
{
    int err;
    int startpage;
    int endpage;

    /* Write the arm image to the specified bank */
    startpage = header->image_offset[slot];
    endpage = startpage + ROUND_UP(armlen, MGMT_FLASH_PAGE_SIZE) / MGMT_FLASH_PAGE_SIZE;

    mgmt_print(info, "installing arm image in slot %d [pages %d - %d]\n", slot, startpage, endpage);
    err = mgmt_erase_flash(info, spi, startpage, endpage);
    if (err) {
        mgmt_print(info, "error erasing arm image slot %d\n", slot);
        return err;
    }
    err = mgmt_write_flash(info, spi, startpage, armbuf, armlen);
    if (err) {
        mgmt_print(info, "error writing arm image to slot %d\n", slot);
        return err;
    }
    return 0;
}

/*@api
 * mgmt_install_arm_images
 *
 * @brief
 * Installs the specified ARM image into the flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=arm_image - path to the ARM image to install
 * @param=slot - 0=stage1 boot image, 1=stage2 runtime image
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_install_arm_images(mgmt_info_t *info, int spi, char *arm_image, int slot)
{
    int err;
    int boot_slot;
    uint32_t i;
    uint8_t *armbuf;
    uint32_t armlen;
    int dmu_mode = 1;
    int delay = 0;
    flash_header_t header;
    uint32_t free_mask;
    mgmt_reply_version_t version;

    /* Open the ARM image file to get the actual size */
    armbuf = buffer_from_file(info, arm_image, &armlen);
    if (!armbuf) {
        mgmt_print(info, "could not read ARM image file %s\n", arm_image);
        return -1;
    }

    /* See if we're running in ROM (implies no flash write routines) */
    err = mgmt_get_version(info, &version);
    if (err) {
        mgmt_print(info, "could not get version info, err = %d\n", err);
        return err;
    }

    /* If in flash-init mode, we need to download and execute the ARM image first */
    if (version.mode == MGMT_FLASH_INIT_MODE) {
        err = mgmt_download_image(info, armbuf, armlen);
        if (err) {
            mgmt_print(info, "error downloading ARM image, err = %d\n", err);
            return err;
        }

      /* Now try to execute the image */
      err = mgmt_execute_image(info, armbuf, dmu_mode, delay, 0, 0);
      if (err) {
          mgmt_print(info, "error executing ARM image, err = %d\n", err);
          return err;
      }
    }

    /* Find a valid TOC */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "error reading flash header\n");
        sal_free(armbuf);
        return err;
    }

    free_mask = (1 << FLASH_NUM_IMAGES) - 1;
    /* Find the occupied slots */
    for (i = 0; i < FLASH_NUM_BOOT_IMAGES; ++i) {
        if (header.boot_image[i] != 0xffffffff) {
            free_mask &= ~(1 << header.boot_image[i]);
        }
    }

    /* If a slot was specified we assume a two stage uKernel for B0.
       slot 0/1 are for stage1 boot uKernel
       slot 2/3 are for stage2 runtime uKernel
       will always mark slot 0/1 as active in this case 
    */
    if (slot < ARM_UKERNEL_SLOT_MAX)
    {
      for (i = slot*FLASH_NUM_BOOT_IMAGES; i < (slot+1)*FLASH_NUM_BOOT_IMAGES; ++i)
      {
        err = mgmt_install_arm_image(info, spi, &header, i, armbuf, armlen);
        if (err) {
            mgmt_print(info, "error writing flash image to slot %d\n", i);
            sal_free(armbuf);
            return err;
        }
      }
      /* Update the local copy of the header to refer to the new image - force to slot 0/1 */
      boot_slot = ARM_UKERNEL_SLOT_BOOT;
      for (i = 0; i < FLASH_NUM_BOOT_IMAGES; ++i) {
          header.boot_image[boot_slot++] = i;
      }
    }
    else
    {
      /* Install the ARM image to each (unused) slot in turn */
      for (i = 0; i < FLASH_NUM_IMAGES; ++i) {
        if (free_mask & (1 << i)) {
            err = mgmt_install_arm_image(info, spi, &header, i,
                                           armbuf, armlen);
            if (err) {
                mgmt_print(info, "error writing flash image to slot %d\n", i);
                sal_free(armbuf);
                return err;
            }
            /* reboot the ARM with the new candidate image */
        }
      }
      /* Update the local copy of the header to refer to the new image */
      boot_slot = 0;
      for (i = 0; i < FLASH_NUM_IMAGES; ++i) {
          if (free_mask & (1 << i)) {
              header.boot_image[boot_slot++] = i;
              /* If we've filled the boot_image[] array, stop */
              if (boot_slot == FLASH_NUM_BOOT_IMAGES) {
                  break;
              }
          }
      }
    }
    sal_free(armbuf);

    /* Write the updated header */
    return mgmt_install_flash_header(info, spi, &header);
}

 /*@api
 * mgmt_install_switch_images
 *
 * @brief
 * Installs the specified ARM image into the flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=switch_config - pointer to data to write
 * @param=len - data length
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_install_switch_images(mgmt_info_t *info, int spi,
                               uint8_t *switch_config, uint32_t len)
{
    int err;
    uint32_t i;
    uint32_t switch_pages;
    flash_header_t header;

    /* Read and validate the TOC, create a new one if necssary */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "error reading flash header\n");
        return err;
    }

    switch_pages = ROUND_UP(len, MGMT_FLASH_PAGE_SIZE) / MGMT_FLASH_PAGE_SIZE;

    for (i = 0; i < FLASH_NUM_CONFIGS; ++i) {
        int startpage = header.config_offset[i];
        int endpage = startpage + switch_pages;

        mgmt_print(info, "installing switch configuration in slot %d\n", i);
        err = mgmt_erase_flash(info, spi, startpage, endpage);
        if (err) {
            mgmt_print(info, "error erasing switch image slot %d\n", i);
            return err;
        }
        err = mgmt_write_flash(info, spi, startpage, switch_config, len);
        if (err) {
            mgmt_print(info, "error writing switch image to slot %d\n", i);
            return err;
        }
    }
    return 0;
}

/*@api
 * mgmt_install_images
 *
 * @brief
 * Installs the specified ARM and switch images into the flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=arm_image - path to the ARM image to install
 * @param=switch_image - path to the switch image to install
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_install_images(mgmt_info_t *info, int spi,
                        char *arm_image, char *switch_image)
{
    int err;
    uint32_t len;
    uint8_t *switch_config;

    /* Open the ARM image file to get the actual size */
    switch_config = buffer_from_file(info, switch_image, &len);
    if (!switch_config) {
        mgmt_print(info, "could not read switch config image file %s\n", switch_image);
        return -1;
    }

    err = mgmt_install_arm_images(info, spi, arm_image, ARM_UKERNEL_SLOT_RUNTIME);
    if (err) {
        mgmt_print(info, "failed to install ARM image\n");
        return err;
    }

    err = mgmt_install_switch_images(info, spi, switch_config, len);
    if (err) {
        mgmt_print(info, "failed to install switch image\n");
        return err;
    }
    return 0;
}

/*@api
 * mgmt_initialize_flash
 *
 * @brief
 * Initializes the SPI flash on the specified interface
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=dmu_config - DMU configuration option to choose
 * @param=spi_speed - SPI speed to install (0 = no speed change)
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_initialize_flash(mgmt_info_t *info, int spi, uint32_t dmu_config, uint32_t spi_speed)
{
    int err;
    uint32_t page;
    flash_info_t flash_info;
    flash_header_t header;

    /* Find the flash parameters */
    err = mgmt_get_flash_info(info, spi, &flash_info);
    if (err) {
        mgmt_print(info, "error reading flash info\n");
        return err;
    }

    /* Erase using the larger of the erase sizes */
    page = 0;
    while (page < (flash_info.flash_size / MGMT_FLASH_PAGE_SIZE)) {
        err = mgmt_erase_flash_sector(info, spi, page);
        if (err) {
            mgmt_print(info, "error erasing sector at page %d\n", page);
            return err;
        }
        page += flash_info.erase_sector_size / MGMT_FLASH_PAGE_SIZE;
    }
    /* Calculate a new flash header */
    sal_memset(&header, 0, sizeof(flash_header_t));
    err = mgmt_calculate_flash_header(info, &flash_info, &header, dmu_config, spi_speed);
    if (err) {
        mgmt_print(info, "error calculating flash header\n");
        return err;
    }
    /* Install it */
    return mgmt_install_flash_header(info, spi, &header);
}

/*@api
 * mgmt_force_coredump
 *
 * @brief
 * Ask the ARM to dump a corefile to flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */

int mgmt_force_coredump(mgmt_info_t *info, int spi)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_FLASH_DUMP_CORE);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.flash_coredump));
    cmd->u.flash_coredump.spi = uswap32(spi);

    return mgmt_cmd_reply(info, cmd, reply, 1);
}

/*@api
 * mgmt_read_coredump
 *
 * @brief
 * Read a core dump file from SPI flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=slot - core dump slot to read
 * @param=filename - file to write the coredump to
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */

int mgmt_read_coredump(mgmt_info_t *info, int spi, int slot, char *filename)
{
    int err;
    flash_header_t header;
    uint8_t data[MGMT_FLASH_PAGE_SIZE];
    mos_coredump_header_t coredump;
    FILE *fp;
    int startpage;

    /* Check the slot number */
    if ((slot < 0) || (slot >= FLASH_NUM_CORE_DUMPS)) {
        mgmt_print(info, "mgmt_read_coredump: bad slot\n");
        return -1;
    }

    /* Read the TOC */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "mgmt_read_coredump: reading flash header\n");
        return err;
    }
    if (header.magic != FLASH_HEADER_MAGIC) {
        mgmt_print(info, "mgmt_read_coredump: invalid TOC magic number\n");
        return -1;
    }

    startpage = header.core_dump_offset[slot];
    mgmt_print(info, "mgmt_read_coredump: slot %d coredump at %d\n", slot, startpage);

    /* If no core dump area defined, skip */
    if (!startpage) {
        mgmt_print(info, "mgmt_read_coredump: no startpage\n");
        return -1;
    }
    /* read the core dump header */
    err = mgmt_read_flash_page(info, spi, startpage, data);
    if (err) {
        mgmt_print(info, "mgmt_read_coredump: error reading flash page %d\n", startpage);
        return err;
    }
    /* Extract the header */
    sal_memcpy(&coredump, data, sizeof(mos_coredump_header_t));

    coredump.magic = uswap32(coredump.magic);
    coredump.start = uswap32(coredump.start);
    coredump.end = uswap32(coredump.end);

    /* Check the coredump magic number */
    if (coredump.magic != MOS_COREDUMP_HEADER_MAGIC) {
        mgmt_print(info, "mgmt_read_coredump: bad coredump header 0x%08x\n", coredump.magic);
        return -1;
    }
    /* Read the entire coredump out to the file */
    fp = fopen(filename, "w");
    if (!fp) {
        mgmt_print(info, "mgmt_read_coredump: could not open %s for writing\n", filename);
        return -1;
    }
    err = mgmt_read_flash(info, spi, startpage, coredump.end, fp);
    fclose(fp);
    if (err) {
        mgmt_print(info, "mgmt_read_coredump: could not write coredump to %s\n", filename);
        return -1;
    }
    /* success */
    return 0;
}

/*@api
 * mgmt_erase_coredump
 *
 * @brief
 * Erase a core dump from the flash, freeing it up for the next dump.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=slot - slot to erase
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */

int mgmt_erase_coredump(mgmt_info_t *info, int spi, int slot)
{
    int err;
    flash_header_t header;

    /* Check the slot number */
    if ((slot < 0) || (slot >= FLASH_NUM_CORE_DUMPS)) {
        return -1;
    }

    /* Read the TOC */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "mgmt_erase_coredump: reading flash header\n");
        return err;
    }
    if (header.magic != FLASH_HEADER_MAGIC) {
        mgmt_print(info, "mgmt_erase_coredump: invalid TOC magic number\n");
        return -1;
    }
    /* Check that there's a core dump slot allocated */
    if (!header.core_dump_offset[slot]) {
        mgmt_print(info, "mgmt_erase_coredump: no allocation for slot %d\n", slot);
       return -1;
    }
#if 0
    /* Erase the first page of the core image to clear the header */
    return mgmt_erase_flash_page(info, spi, header.core_dump_offset[slot]);
#else
    return 0;
#endif
}

/*
 * @api
 * mgmt_reboot
 *
 * @brief
 * Send a command to the BCM53084, telling it to reboot.
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=slot - slot to reboot
 * @returns 0 on success, !0 on error
 */
int mgmt_reboot(mgmt_info_t *info, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_REBOOT);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.reboot));

    cmd->u.reboot.args[0] = uswap32(arg0);
    cmd->u.reboot.args[1] = uswap32(arg1);
    cmd->u.reboot.args[2] = uswap32(arg2);
    cmd->u.reboot.args[3] = uswap32(arg3);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        return err;
    }

    /* Let it boot */
    sal_sleep(5);

    /* version info no longer valid */
    info->version_valid = 0;

    /* re-initialize the connection */
#ifdef MGMT_USB_SUPPORT
    if (info->connection_type == MGMT_USB_CONNECTION) {
        if (info->handle) {
            err = libusb_release_interface(info->handle, info->interface);
            if (err) {
#if MGMT_DEBUG
                perror("could not release management interface");
#endif
            }
            libusb_close(info->handle);
            libusb_free_device_list(info->list, 1);
            libusb_exit(info->context);
        }

        /* Start over from scratch */

        /* Connect through USB */
        err = mgmt_usb_init(info);
        if (err) {
            return err;
        }
        info->connection_type = MGMT_USB_CONNECTION;

        /* Initialize the connection */
        err = mgmt_initialize_connection(info);
        if (err) {
            return err;
        }
    }
#endif
    return 0;
}

/*
 * @api
 * mgmt_execute_image
 *
 * @brief
 * Execute the ARM image that's been downloaded to the target
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=armbuf - pointer to the ARM image
 * @param=arg0 - argument 0
 * @param=arg1 - argument 1
 * @param=arg2 - argument 2
 * @param=arg3 - argument 3
 * @returns 0 on success, !0 on error
 */
int mgmt_execute_image(mgmt_info_t *info, uint8_t *armbuf, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    uint32_t i;
    uint32_t cksum;
    uint32_t len;
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_EXECUTE);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.execute));

    /* Set the length */
    len = uswap32(((mos_arm_image_header_t *)armbuf)->length);
    cmd->u.execute.length = uswap32(len);

    /* Calculate the checksum */
    cksum = 0;
    for (i = 0; i < len/sizeof(uint32_t); i++) {
        cksum += uswap32(((uint32_t *)armbuf)[i + 2*MGMT_FLASH_PAGE_SIZE/sizeof(uint32_t)]);
    }
    cmd->u.execute.cksum = uswap32(cksum);

    /* copy in the signature */
    sal_memcpy(cmd->u.execute.signature, &armbuf[MGMT_FLASH_PAGE_SIZE], MGMT_FLASH_PAGE_SIZE);

    /* Set the arguments */
    cmd->u.execute.args[0] = uswap32(arg0);
    cmd->u.execute.args[1] = uswap32(arg1);
    cmd->u.execute.args[2] = uswap32(arg2);
    cmd->u.execute.args[3] = uswap32(arg3);

    /* Execute */
    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }

    /* version info no longer valid */
    info->version_valid = 0;

#if 0
    /* re-initialize the connection */
#ifdef MGMT_USB_SUPPORT
    if (info->connection_type == MGMT_USB_CONNECTION) {
        if (info->handle) {
#if MGMT_DEBUG
            mgmt_print(info, "%s: releasing interface\n", FUNCTION_NAME());
#endif
            err = libusb_release_interface(info->handle, info->interface);
            if (err) {
#if MGMT_DEBUG
                perror("could not release management interface");
#endif
            }
#if MGMT_DEBUG
            mgmt_print(info, "%s: closing USB\n", FUNCTION_NAME());
#endif
            libusb_close(info->handle);
            libusb_free_device_list(info->list, 1);
            libusb_exit(info->context);
        }
    }
#endif

    /* Let it boot */
    sal_sleep(5);

#ifdef MGMT_USB_SUPPORT
    if (info->connection_type == MGMT_USB_CONNECTION) {
        /* Start over from scratch */

        /* Connect through USB */
#if MGMT_DEBUG
        mgmt_print(info, "%s: calling mgmt_usb_init()\n", FUNCTION_NAME());
#endif
        err = mgmt_usb_init(info);
        if (err) {
            return err;
        }
        info->connection_type = MGMT_USB_CONNECTION;
    
        /* Initialize the connection */
#if MGMT_DEBUG
        mgmt_print(info, "%s: calling mgmt_initialize_connection()\n", FUNCTION_NAME());
#endif
        err = mgmt_initialize_connection(info);
        if (err) {
            return err;
        }
    }
#endif
#endif
#if MGMT_DEBUG
    mgmt_print(info, "%s: finished\n", FUNCTION_NAME());
#endif
    return 0;
}

/*
 * @api
 * mgmt_read_memory
 *
 * @brief
 * Read a page of memory from the BCM53084
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=address - address to read
 * @param=data - buffer to return the data in
 * @returns 0 on success, !0 on error
 */
int mgmt_read_memory(mgmt_info_t *info, uint32_t address, uint8_t *data)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_READ_MEMORY);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.read_memory));
    cmd->u.read_memory.address = uswap32(address);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }

    /* copy out the data */
    sal_memcpy(data, reply->u.read_memory.data, MGMT_MEMORY_PAGE_SIZE);
    return 0;
}

/*
 * @api
 * mgmt_write_memory
 *
 * @brief
 * Write a page of memory to the target
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=address - address to write
 * @param=data - buffer containing the data
 * @returns 0 on success, !0 on error
 */
int mgmt_write_memory(mgmt_info_t *info, uint32_t address, uint8_t *data)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_WRITE_MEMORY);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.write_memory));
    cmd->u.write_memory.address = uswap32(address);

    /* copy in the data */
    sal_memcpy(cmd->u.write_memory.data, data, MGMT_MEMORY_PAGE_SIZE);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_shell_cmd
 *
 * @brief
 * Run a shell command
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=cmd - command string to run
 * @returns 0 on success, !0 on error
 */
int mgmt_shell_cmd(mgmt_info_t *info, char *cmdstring)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SHELL_CMD);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.shell_cmd));
    sal_strcpy(cmd->u.shell_cmd.cmd, cmdstring);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_set_power_state
 *
 * @brief
 * Set the power management state
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=state - state to set
 * @returns 0 on success, !0 on error
 */
int mgmt_set_power_state(mgmt_info_t *info, uint32_t state)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SET_POWER_STATE);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.power));
    cmd->u.power.state = uswap32(state);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_get_power_state
 *
 * @brief
 * Set the power management state
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=state - pointer to where to return the state
 * @returns 0 on success, !0 on error
 */
int mgmt_get_power_state(mgmt_info_t *info, uint32_t *state)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_GET_POWER_STATE);
    cmd->len = uswap32(MGMT_COMMAND_LEN);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    *state = uswap32(reply->u.power.state);
    return 0;
}

/*
 * @api
 * mgmt_ping
 *
 * @brief
 * Ping the device
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=number - number to embed in command
 * @returns 0 on success, !0 on error
 */
int mgmt_ping(mgmt_info_t *info, uint32_t number, uint32_t *rnumber, uint32_t skip_reply)
{
    int err;
    int len;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_PING);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.ping));
    cmd->u.ping.number = uswap32(number);

    err = mgmt_cmd(info, cmd, 0);
    if (err) {
        mgmt_print(info, "could not send command\n");
        return err;
    }
    if (skip_reply) {
        mgmt_unlock(info);
        return 0;
    }
    err = mgmt_reply(info, reply, &len, 0);
    if (err) {
        mgmt_print(info, "could not get reply\n");
        return err;
    }
    if (reply->status != MGMT_SUCCESS) {
        mgmt_print(info, "error in reply status=%d\n", uswap32(reply->status));
        return -1;
    }
    *rnumber = uswap32(reply->u.ping.number);
    return 0;
}

/*
 * @api
 * mgmt_pll_spread
 *
 * @brief
 * Enable/Disable PLL spread spectrum testing
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=spread - spread param or zero
 * @returns 0 on success, !0 on error
 */
int mgmt_pll_spread(mgmt_info_t *info, uint32_t spread)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_PLL_SPREAD);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.spread));
    cmd->u.spread.spread = uswap32(spread);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_set_imp_speed
 *
 * @brief
 * Set the speed of the IMP port
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=speed - speed param (10, 100, 1000)
 * @returns 0 on success, !0 on error
 */
int mgmt_set_imp_speed(mgmt_info_t *info, uint32_t speed)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_SET_IMP_SPEED);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.set_imp_speed));
    cmd->u.set_imp_speed.speed = uswap32(speed);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_get_buffer_info
 *
 * @brief
 * Get information about system buffers
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=buffer_info - structure to fill in
 * @returns 0 on success, !0 on error
 */
int mgmt_get_buffer_info(mgmt_info_t *info, mgmt_reply_buffer_info_t *buffer_info)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_BUFFER_INFO);
    cmd->len = uswap32(MGMT_COMMAND_LEN);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        return err;
    }
    sal_memcpy(buffer_info, &reply->u.buffer_info, sizeof(mgmt_reply_buffer_info_t));
    return 0;
}

/*
 * @api
 * _mgmt_read_buffer
 *
 * @brief
 * Read from a system buffer
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=protected - true if buffer is protected
 * @param=buffer - buffer ID to read from
 * @param=offset - offset in buffer to start at
 * @param=length - length of data
 * @param=data - buffer to return the data in
 * @returns 0 on success, !0 on error
 */
int _mgmt_read_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t offset, uint32_t length, uint8_t *data)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    if (length > MGMT_BUFFER_PAGE_SIZE) {
        return -1;
    }

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(protected ? MGMT_READ_BUFFER_PROT : MGMT_READ_BUFFER);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.read_buffer));
    cmd->u.read_buffer.buffer = uswap32(buffer);
    cmd->u.read_buffer.offset = uswap32(offset);
    cmd->u.read_buffer.length = uswap32(length);

    err = mgmt_cmd_reply(info, cmd, reply, protected);
    if (err) {
        return err;
    }

    /* copy out the data */
    sal_memcpy(data, reply->u.read_buffer.data, length);
    return 0;
}

/*
 * @api
 * mgmt_read_buffer
 *
 * @brief
 * Read from a system buffer
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=protected - true if buffer is protected
 * @param=buffer - buffer ID to read from
 * @param=length - length of data
 * @param=data - buffer to return the data in
 * @returns 0 on success, !0 on error
 */
int mgmt_read_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t length, uint8_t *data)
{
    int err;
    uint32_t len;
    uint32_t offset;

    for (len = length, offset = 0; len > 0; ) {
        uint32_t rdlen = (len > MGMT_BUFFER_PAGE_SIZE) ? MGMT_BUFFER_PAGE_SIZE : len;

        err = _mgmt_read_buffer(info, protected, buffer, offset, rdlen, &data[offset]);
        if (err) {
            mgmt_print(info, "error reading buffer\n");
            return err;
        }
        offset += rdlen;
        len -= rdlen;
    }
    return 0;
}

/*
 * @api
 * _mgmt_write_buffer
 *
 * @brief
 * Write to a system buffer
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=protected - true if buffer is protected
 * @param=buffer - buffer ID to write to
 * @param=offset - offset in buffer to start at
 * @param=length - length of data
 * @param=data - buffer to return the data in
 * @returns 0 on success, !0 on error
 */
int _mgmt_write_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t offset, uint32_t length, uint8_t *data)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    if (length > MGMT_BUFFER_PAGE_SIZE) {
        return -1;
    }

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(protected ? MGMT_WRITE_BUFFER_PROT : MGMT_WRITE_BUFFER);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.write_buffer));
    cmd->u.write_buffer.buffer = uswap32(buffer);
    cmd->u.write_buffer.offset = uswap32(offset);
    cmd->u.write_buffer.length = uswap32(length);

    /* copy in the data */
    sal_memcpy(cmd->u.write_buffer.data, data, length);

    err = mgmt_cmd_reply(info, cmd, reply, protected);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_write_buffer
 *
 * @brief
 * Write to a system buffer
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=protected - true if buffer is protected
 * @param=buffer - buffer ID to write to
 * @param=length - length of data
 * @param=data - buffer to write
 * @returns 0 on success, !0 on error
 */
int mgmt_write_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t length, uint8_t *data)
{
    int err;
    uint32_t len;
    uint32_t offset;

#if MGMT_DEBUG > 0
    mgmt_print(info, "%s: protected = %d buffer = %d length = %d\n",
            FUNCTION_NAME(), protected, buffer, length);
#endif
    for (len = length, offset = 0; len > 0; ) {
        uint32_t wrlen = (len > MGMT_BUFFER_PAGE_SIZE) ? MGMT_BUFFER_PAGE_SIZE : len;

        err = _mgmt_write_buffer(info, protected, buffer, offset, wrlen, &data[offset]);
        if (err) {
            mgmt_print(info, "%s: error writing buffer\n", FUNCTION_NAME());
            return err;
        }
        offset += wrlen;
        len -= wrlen;
    }
    return 0;
}

/*
 * @api
 * mgmt_transmit_buffer
 *
 * @brief
 * Write to a system buffer
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=buffer - buffer ID to transmit
 * @param=length - length of data to transmit
 * @returns 0 on success, !0 on error
 */
int mgmt_transmit_buffer(mgmt_info_t *info, uint32_t buffer, uint32_t length)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_TRANSMIT_BUFFER);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.transmit_buffer));
    cmd->u.transmit_buffer.buffer = uswap32(buffer);
    cmd->u.transmit_buffer.length = uswap32(length);

    err = mgmt_cmd_reply(info, cmd, reply, 0);
    if (err) {
        return err;
    }
    return 0;
}

/*
 * @api
 * mgmt_acd
 *
 * @brief
 * Run the automotive cable diagnostics (Polar only)
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=port - port to run the diagnostics on
 * @param=command - command bits to use
 * @param=results - pointer to the mgmt_reply_acd_results_t to fill in
 *
 * @returns 0 on success, !0 on error
 */
int mgmt_acd(mgmt_info_t *info, uint32_t port, uint32_t command, mgmt_reply_acd_results_t *results)
{
    int err;
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_ACD);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.acd));
    cmd->u.acd.port = uswap32(port);
    cmd->u.acd.command = uswap32(command);

    err = mgmt_cmd_reply(info, cmd, reply, 1);
    if (err) {
        return err;
    }

    /* copy out the results */
    results->fault = uswap32(reply->u.acd.fault);
    results->index = uswap32(reply->u.acd.index);
    results->peak_amplitude = uswap32(reply->u.acd.peak_amplitude);
    return 0;
}

/*
 * @api
 * mgmt_set_dmu
 *
 * @brief
 * Send the DMU settings to the device.
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=hz - frequency of processor clock
 *
 * @returns 0 on success, !0 on error
 */
int mgmt_set_dmu(mgmt_info_t *info, uint32_t hz,
                    uint32_t hclk_freq, uint32_t hclk_sel,
                    uint32_t pclk_freq, uint32_t pclk_sel,
                    uint32_t p1div, uint32_t p2div, uint32_t ndiv,
                    uint32_t m1div, uint32_t m2div, uint32_t m3div, uint32_t m4div,
                    uint32_t pll_num, uint32_t frac, uint32_t bclk_sel)
{
    mgmt_command_t cmd_packet;
    mgmt_command_t *cmd = &cmd_packet;
    mgmt_reply_t reply_packet;
    mgmt_reply_t *reply = &reply_packet;

    sal_memset(cmd, 0, sizeof(mgmt_command_t));
    sal_memset(reply, 0, sizeof(mgmt_reply_t));

    cmd->cmd = uswap32(MGMT_DMU);
    cmd->len = uswap32(MGMT_COMMAND_LEN + sizeof(cmd->u.dmu));
    cmd->u.dmu.hz = uswap32(hz);
    cmd->u.dmu.hclk_freq = uswap32(hclk_freq);
    cmd->u.dmu.hclk_sel = uswap32(hclk_sel);
    cmd->u.dmu.pclk_freq = uswap32(pclk_freq);
    cmd->u.dmu.pclk_sel = uswap32(pclk_sel);
    cmd->u.dmu.p1div = uswap32(p1div);
    cmd->u.dmu.p2div = uswap32(p2div);
    cmd->u.dmu.ndiv = uswap32(ndiv);
    cmd->u.dmu.m1div = uswap32(m1div);
    cmd->u.dmu.m2div = uswap32(m2div);
    cmd->u.dmu.m3div = uswap32(m3div);
    cmd->u.dmu.m4div = uswap32(m4div);
    cmd->u.dmu.pll_num = uswap32(pll_num);
    cmd->u.dmu.frac = uswap32(frac);
    cmd->u.dmu.bclk_sel = uswap32(bclk_sel);

    return mgmt_cmd_reply(info, cmd, reply, 0);
}

/*
 * @api
 * mgmt_set_processor_clock
 *
 * @brief
 * Send the DMU settings to the device.
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=hz - frequency of processor clock
 *
 * @returns 0 on success, !0 on error
 */
int mgmt_set_processor_clock(mgmt_info_t *info, uint32_t hz)
{
    int i;

    for (i = 0; i < sizeof(mgmt_dmu_values)/sizeof(mgmt_dmu_values[0]); ++i) {
        mgmt_dmu_value_t *entry;

        entry = &mgmt_dmu_values[i];
        if (entry->hz == hz) {
            return mgmt_set_dmu(info, hz,
                            entry->hclk_freq,
                            entry->hclk_sel,
                            entry->pclk_freq,
                            entry->pclk_sel,
                            entry->p1div,
                            entry->p2div,
                            entry->ndiv,
                            entry->m1div,
                            entry->m2div,
                            entry->m3div,
                            entry->m4div,
                            entry->pll_num,
                            entry->frac,
                            entry->bclk_sel);
        }
    }
    return -1;
}

 /*@api
 * mgmt_install_avb_configs
 *
 * @brief
 * Installs the specified AVB config into the flash.
 *
 * @param=info - pointer to mgmt_info_t
 * @param=spi - SPI interface number the flash is connected to
 * @param=avb_config - pointer to data to write
 * @param=len - data length
 *
 * @returns 0 - success
 * @returns !0 - error
 *
 * @desc
 */
int mgmt_install_avb_configs(mgmt_info_t *info, int32_t spi,
                               uint8_t *avb_config, uint32_t len)
{
    int32_t i;
    int32_t err;
    uint32_t avb_config_pages;
    flash_header_t header;

    /* Read and validate the TOC, create a new one if necssary */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "Error reading flash header\n");
        return err;
    }

    avb_config_pages = ROUND_UP(len, MGMT_FLASH_PAGE_SIZE) / MGMT_FLASH_PAGE_SIZE;

    for (i = 0; i < FLASH_NUM_AVB_CONFIGS; ++i) {
        int startpage = header.expansion[AVB_CFG_EXP_OFFSET + i].offset;
        int endpage = startpage + avb_config_pages;

#if DEBUG
        mgmt_print(info, "installing AVB configuration in slot %d\n", i);
#endif
        err = mgmt_erase_flash(info, spi, startpage, endpage);
        if (err) {
            mgmt_print(info, "Error erasing AVB config slot %d\n", i);
            return err;
        }
        err = mgmt_write_flash(info, spi, startpage, avb_config, len);
        if (err) {
            mgmt_print(info, "error writing AVB config to slot %d\n", i);
            return err;
        }
    }
    return 0;
}


/*
 * @api
 * mgmt_vpd_show
 *
 * @brief
 * show the vpd data
 *
 * @param=info - pointer to the mgmt_info_t
 * @param=spi - spi number
 *
 * @returns 0 on success, !0 on error
 */
int mgmt_vpd_show(mgmt_info_t *info, int spi)
{
    uint32_t page;
    int32_t err;
    int32_t i;
    uint8_t length;
    uint8_t vector;
    uint32_t temp;
    uint32_t checksum;
    uint8_t buffer[256];
    flash_header_t header;
    mos_flash_expansion_header_t vpd_header;
    mos_flash_expansion_header_t *vpd_header_ptr;

      /* Read the TOC */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "Error reading flash header\n");
        return err;
    }
    /* verify header */
    page = header.expansion[VPD_EXP_OFFSET].offset;
    err = mgmt_read_flash_page(info, spi, page, buffer);
    if (err) {
        mgmt_print(info, "Error reading flash page\n");
        return err;
    }

    vpd_header_ptr = (mos_flash_expansion_header_t *)buffer;
    vpd_header.magic = uswap32(vpd_header_ptr->magic);
    vpd_header.length = uswap32(vpd_header_ptr->length);
    vpd_header.cksum =  uswap32(vpd_header_ptr->cksum);
    vpd_header.image_cksum = uswap32(vpd_header_ptr->image_cksum);

    checksum = 0;
    for (i = 0; i < sizeof(mos_flash_expansion_header_t)/sizeof(uint32_t); i++) {
        checksum += ((uint32_t *)&vpd_header)[i];
    }
    if (checksum) {
        mgmt_print(info, " VPD header bad checksum \n");
        return -1;
    }

    page = page + 1;
    err = mgmt_read_flash_page(info, spi, page, buffer);
    if (err) {
        mgmt_print(info, "Error reading flash page\n");
        return err;
    }
    checksum = 0;
    for (i = 0; i < sizeof(buffer)/sizeof(uint32); ++i) {
        temp = ((uint32 *)buffer)[i];
        checksum +=  uswap32(temp);
    }
    checksum = -checksum;
    if (vpd_header.image_cksum != checksum){
        mgmt_print(info, "Invalid VPD data checksum\n");
        return -1;
    }

    i = 0;
    while (i < 256){
        length = buffer[i];
        vector = buffer[i+1];
        switch(vector){
        case 0:
            return 0;
            break;
        case 1:
            mgmt_print(info, "Mac address: %x:%x:%x:%x:%x:%x\n",buffer[i+2], buffer[i+3],
                           buffer[i+4], buffer[i+5], buffer[i+6], buffer[i+7]);
            break;
        case 2:
            mgmt_print(info, "Serial number: %s\n",&buffer[i+2]);
            break;
        default:
            return -1;
            break;
        }
        i = i + length;
    }
    return -1;
}
/*
 * @api
 * mgmt_vpd_set
 *
 * @brief
 * Set the vpd parameters
 *
 * @param=mac_addr - pointer to the mac address
 * @param=serial_number - serial number
 *
 * @returns 0 on success, !0 on error
 */
int mgmt_vpd_set(mgmt_info_t *info, int spi, uint8_t *mac_addr, uint8_t *serial_number)
{
    uint8_t mac_length;
    uint8_t buffer[256];
    uint8_t mac_hex[6];
    uint32_t cksum;
    uint32_t temp;
    int32_t start_page;
    uint8_t ch;
    int32_t i;
    int err;
    mos_flash_expansion_header_t vpd_header;
    flash_header_t header;

      /* Read the TOC */
    err = mgmt_read_flash_header(info, spi, 0, &header);

    mac_length = sal_strlen((char *)mac_addr);
    if (mac_length != 12){
        mgmt_print(info, "\nInvalid MAC address \n");
        return -1;
    }
    temp = sal_strlen((char *)serial_number);
    if (mac_length > 30){
        mgmt_print(info, "\nInvalid Serial number \n");
        return -1;
    }

    for (i = 0; i < mac_length; i++){
        ch = mac_addr[i];
        if ((ch >= '0') && (ch <= '9')){
            buffer[i] = ch - '0';
        }
        else if ((ch >= 'a') && (ch <= 'f')){
            buffer[i] = ch - 'a' + 10;
        }
        else if ((ch >= 'A') && (ch <= 'F')){
            buffer[i] = ch - 'A' + 10;
        }
        else{
            mgmt_print(info, "\nInvalid MAC address \n");
            return -1;   /* Invalid character */
        }
    }
    sal_memset(mac_hex, 0, sizeof(mac_hex));
    for ( i = 0; i < 12; i+= 2 ){
        mac_hex[i/2] = (buffer[i] * 16) + buffer[i+1];
    }
    if (((mac_hex[0] == 0) && (mac_hex[1] == 0) && (mac_hex[2] == 0) &&
         (mac_hex[3] == 0) && (mac_hex[4] == 0) && (mac_hex[5] == 0)) ||
        ((mac_hex[0] == 0xff) && (mac_hex[1] == 0xff) && (mac_hex[2] == 0xff) &&
         (mac_hex[3] == 0xff) && (mac_hex[4] == 0xff) && (mac_hex[5] == 0xff)) ||
        ((mac_hex[0] & 0x01) == 0x01)){
        mgmt_print(info, "\nInvalid MAC address \n");
        return -1;
    }
    /* Read the TOC */
    err = mgmt_read_flash_header(info, spi, 0, &header);
    if (err) {
        mgmt_print(info, "\nError reading flash header\n");
        return err;
    }

    start_page = header.expansion[VPD_EXP_OFFSET].offset;
    err = mgmt_erase_flash(info, spi, start_page, (start_page + 1));
    if (err) {
        mgmt_print(info, "\nError erasing flash sector\n");
        return err;
    }

    i = 0;
    buffer[i++] = 8;
    buffer[i++] = 1;
    buffer[i++] = mac_hex[0];
    buffer[i++] = mac_hex[1];
    buffer[i++] = mac_hex[2];
    buffer[i++] = mac_hex[3];
    buffer[i++] = mac_hex[4];
    buffer[i++] = mac_hex[5];
    buffer[i++] = (sal_strlen((char *)serial_number) + 3);
    buffer[i++] = 2;
    sal_strcpy((char *)&buffer[i], (char *)serial_number);
    i += sal_strlen((char *)serial_number);
    buffer[i++] = 0;

    cksum = 0;
    for (i = 0; i < sizeof(buffer)/sizeof(uint32); ++i) {
        temp = ((uint32 *)buffer)[i];
        cksum +=  uswap32(temp);
    }
    sal_memset((uint8 *)&vpd_header, 0, sizeof(vpd_header));

    /* Install the negation of the checksum, so on the target it results in 0 */
    vpd_header.image_cksum = -cksum;
    /* write vpd data to page */
    err = mgmt_write_flash_page(info, spi, (start_page + 1), buffer);
    if (err) {
        mgmt_print(info, "error writing flash page %d\n", (start_page + 1) );
        return err;
    }

    vpd_header.length = 256;
    sal_memset(buffer, 0, sizeof(buffer));
    cksum = 0;
    for (i = 0; i < sizeof(mos_switch_config_header_t)/sizeof(uint32); ++i) {
        cksum += ((uint32 *)&vpd_header)[i];
    }
    vpd_header.cksum = -cksum;
     /* write vpd header page */
    vpd_header.magic = uswap32(vpd_header.magic);
    vpd_header.length = uswap32(vpd_header.length);
    vpd_header.cksum =  uswap32(vpd_header.cksum);
    vpd_header.image_cksum = uswap32(vpd_header.image_cksum);
    sal_memcpy(buffer, (uint8*)&vpd_header, sizeof(vpd_header));
    err = mgmt_write_flash_page(info, spi, start_page, buffer);
    if (err) {
        mgmt_print(info,  "error writing flash page %d\n", start_page);
        return err;
    }
    return err;
}
