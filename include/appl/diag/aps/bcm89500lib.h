/*
 * $Id: bcm53084lib.h,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    bcm53084lib.h
 * Purpose: BCM53084 remote management library
 */

#ifndef BCM53084LIB_H
#define BCM53084LIB_H

#if defined(STAND_ALONE)
#include <stdint.h>
#include <sys/types.h>
#include <byteswap.h>
#include <stdio.h>
#include <endian.h>
#endif

#if defined(BCM_PLATFORM_STRING) || defined(STAND_ALONE)
#include <sal/types.h>
#include <appl/diag/aps/mgmt.h>
#include <appl/diag/aps/spiflash_toc.h>
#include <appl/diag/aps/spiflash.h>
#if defined(BCM_PLATFORM_STRING)
#define uint32_t uint32
#define uint64_t uint64
#define uint8_t uint8
#endif
#else
/* must be the MDK */
#include <cdk_custom_config.h>
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint8_t uint8;
#include "mgmt.h"
#include "spiflash_toc.h"
#include "spiflash.h"
#endif

#ifdef MGMT_ENCRYPTION_SUPPORT
#include <openssl/aes.h>
#include <openssl/rsa.h>
#include <openssl/bn.h>
#endif

#ifdef MGMT_USB_SUPPORT
#include <libusb-1.0/libusb.h>
#endif

#if defined(BCM_PLATFORM_STRING)
#define MGMT_SPI_SUPPORT
#endif

#if !defined(MGMT_SPI_SUPPORT) && !defined(MGMT_USB_SUPPORT)
#error "one of SPI or USB support must be selected"
#endif

#define MGMT_INTERFACE  0
#define MGMT_ENDPOINT   1

#define ARM_UKERNEL_SLOT_BOOT    0
#define ARM_UKERNEL_SLOT_RUNTIME 1
#define ARM_UKERNEL_SLOT_MAX     2

/* Byte swap macros for communicating with the uKernel */
#if (defined(__BYTE_ORDER) && (__BYTE_ORDER == __BIG_ENDIAN)) || defined(MIPSEB)
#define uswap16(x)              \
	((\
		(((x) & 0x000000ffUL) << 8) | \
		(((x) & 0x0000ff00UL) >> 8)))
#define uswap32(x)              \
	((\
		(((x) & 0x000000ffUL) << 24) | \
		(((x) & 0x0000ff00UL) <<  8) | \
		(((x) & 0x00ff0000UL) >>  8) | \
		(((x) >> 24) & 0x00ff)))
#else
#define uswap16(x)              (x)
#define uswap32(x)              (x)
#endif

#ifndef ROUND_UP
#define ROUND_UP(x,y)           ((((uint32_t)(x)) + ((uint32_t)(y)) - 1) \
                                 & ~(uint32_t)((y) - 1))
#endif

typedef enum mgmt_error_e {
    MGMT_ERR_NONE                   =  0,
    MGMT_ERR                        = -1,
    MGMT_ERR_SPIDEV_OPEN_FAILED     = -2,
    MGMT_ERR_BUFFER_FROM_FILE_FAILED= -3
} mgmt_error_t;

typedef enum mgmt_connection_e {
#ifdef MGMT_USB_SUPPORT    
    MGMT_USB_CONNECTION,
#endif
#ifdef MGMT_SPI_SUPPORT    
    MGMT_SPI_CONNECTION,
#endif    
    MGMT_NO_CONNECTION
} mgmt_connection_t;

typedef struct mgmt_info_s mgmt_info_t;

struct mgmt_info_s {
    /* Indicator of connection type */
    mgmt_connection_t           connection_type;        /* SPI or USB */
    mgmt_reply_version_t        version;                /* Device version/info */
    int                         version_valid;          /* Valid flag for .version */

#if defined(MGMT_DEBUG) && (MGMT_DEBUG > 0)
    FILE                        *log;
#endif

#ifdef MGMT_USB_SUPPORT
    /* USB connection datastructures */
    libusb_context              *context;
    libusb_device_handle        *handle;
    libusb_device               **list;
    int                         interface;
#endif

#ifdef MGMT_SPI_SUPPORT
    /* SPI connection datastructures */
    int                         spifd;
#endif

#ifdef MGMT_ENCRYPTION_SUPPORT    
    int                         (*decrypt)(mgmt_info_t *info, const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_len);

    /* Shared state */
    const char                  *certfile;
    const char                  *keyfile;

    unsigned int                ctr128_state;
    AES_KEY                     aes_key;

    int                         encrypt_enabled;
    int                         session_key_valid;

    uint8_t                     key[MGMT_SESSION_KEY_LEN];
    uint8_t                     nonce[MGMT_SESSION_KEY_LEN];
    uint8_t                     ecount[MGMT_SESSION_KEY_LEN];
#endif    

#if defined(BCM_PLATFORM_STRING)
    int                         arl_mode;
    int                         linkscan;
    int                         counter;
    uint32                      counter_flags;
    pbmp_t                      counter_pbmp;
#endif
};

mgmt_info_t *mgmt_new(const char *keyfile, const char *certfile, int (*decrypt)(mgmt_info_t *info, const uint8_t *in, uint32_t in_len, uint8_t *out, uint32_t out_len));
#ifdef MGMT_USB_SUPPORT
int mgmt_open_usb(mgmt_info_t *info);
#endif
#ifdef MGMT_SPI_SUPPORT
int mgmt_open_spi(mgmt_info_t *info, const char *spidev, uint32_t spi_speed);
#endif
void mgmt_close(mgmt_info_t *info);

#ifdef MGMT_ENCRYPTION_SUPPORT    
int mgmt_request_session_key(mgmt_info_t *info);
int mgmt_request_cert_session_key(mgmt_info_t *info);
#endif

int mgmt_power_up(mgmt_info_t *info);
int mgmt_initialize_flash(mgmt_info_t *info, int spi, uint32_t dmu_config, uint32_t spi_speed);
int mgmt_get_version(mgmt_info_t *info, mgmt_reply_version_t *version);
int mgmt_set_macaddr(mgmt_info_t *info, void *addr);
int mgmt_get_flash_info(mgmt_info_t *info, int spi, flash_info_t *flash_info);
int mgmt_read_switch_register(mgmt_info_t *info, uint32_t addr, uint32_t size, uint64_t *regval);
int mgmt_write_switch_register(mgmt_info_t *info, uint32_t addr, uint32_t size, uint64_t regval);
int mgmt_erase_flash(mgmt_info_t *info, int spi, int start, int end);
int mgmt_erase_flash_sector(mgmt_info_t *info, int spi, uint32_t addr);
int mgmt_erase_flash_page(mgmt_info_t *info, int spi, uint32_t addr);
int mgmt_set_macsec_key(mgmt_info_t *info, char *keyinfo);
int mgmt_read_flash_page(mgmt_info_t *info, int spi, uint32_t page, uint8_t *data);
int mgmt_write_flash_page(mgmt_info_t *info, int spi, uint32_t page, uint8_t *data);

int mgmt_read_flash(mgmt_info_t *info, int spi, int startpage, int endpage, FILE *fp);
int mgmt_write_flash(mgmt_info_t *info, int spi, int startpage, uint8_t *buffer, uint32_t len);
int mgmt_read_flash_header(mgmt_info_t *info, int spi, int slot, flash_header_t *header);
int mgmt_write_flash_header(mgmt_info_t *info, int spi, int slot, flash_header_t *header);

int mgmt_install_flash_header(mgmt_info_t *info, int spi, flash_header_t *header);

int mgmt_download_image(mgmt_info_t *info, uint8_t *armbuf, uint32_t armlen);
int mgmt_install_images(mgmt_info_t *info, int spi,
                        char *arm_image, char *switch_image);
int mgmt_install_arm_images(mgmt_info_t *info, int spi, char *arm_image, int slot);
int mgmt_install_switch_images(mgmt_info_t *info, int spi,
                               uint8_t *switch_config, uint32_t len);
int mgmt_force_coredump(mgmt_info_t *info, int spi);
int mgmt_read_coredump(mgmt_info_t *info, int spi, int slot, char *filename);
int mgmt_erase_coredump(mgmt_info_t *info, int spi, int slot);

int mgmt_read_memory(mgmt_info_t *info, uint32_t address, uint8_t *data);
int mgmt_write_memory(mgmt_info_t *info, uint32_t address, uint8_t *data);
int mgmt_execute_image(mgmt_info_t *info, uint8_t *image, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);
int mgmt_reboot(mgmt_info_t *info, uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

int mgmt_set_processor_clock(mgmt_info_t *info, uint32_t hz);
int mgmt_set_power_state(mgmt_info_t *info, uint32_t state);
int mgmt_get_power_state(mgmt_info_t *info, uint32_t *state);
int mgmt_shell_cmd(mgmt_info_t *info, char *cmd);
int mgmt_pll_spread(mgmt_info_t *info, uint32_t spread);

int mgmt_set_imp_speed(mgmt_info_t *info, uint32_t speed);

int mgmt_acd(mgmt_info_t *info, uint32_t port, uint32_t command, mgmt_reply_acd_results_t *results);
int mgmt_vpd_show(mgmt_info_t *info, int spi);
int mgmt_vpd_set(mgmt_info_t *info, int spi, uint8_t *mac_addr, uint8_t *serial_number);
int mgmt_install_avb_configs(mgmt_info_t *info, int32_t spi,
                               uint8_t *avb_config, uint32_t len);


/* locking support */
int mgmt_lock(mgmt_info_t *info);
int mgmt_unlock(mgmt_info_t *info);

/* buffer management */
int mgmt_get_buffer_info(mgmt_info_t *info, mgmt_reply_buffer_info_t *buffer_info);
int mgmt_read_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t length, uint8_t *data);
int mgmt_write_buffer(mgmt_info_t *info, int protected, uint32_t buffer, uint32_t length, uint8_t *data);
int mgmt_transmit_buffer(mgmt_info_t *info, uint32_t buffer, uint32_t length);

/* For debugging */
void mgmt_dump_buffer(mgmt_info_t *info, char *name, uint8_t *data, int len);
int mgmt_ping(mgmt_info_t *info, uint32_t number, uint32_t *rnumber, uint32_t skip_reply);

#endif
