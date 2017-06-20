/*
 **************************************************************************************
 Copyright 2012-2017 Broadcom Corporation

 This program is the proprietary software of Broadcom Corporation and/or its licensors,
 and may only be used, duplicated, modified or distributed pursuant to the terms and
 conditions of a separate, written license agreement executed between you and
 Broadcom (an "Authorized License").Except as set forth in an Authorized License,
 Broadcom grants no license (express or implied),right to use, or waiver of any kind
 with respect to the Software, and Broadcom expressly reserves all rights in and to
 the Software and all intellectual property rights therein.
 IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 WAY,AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.

 Except as expressly set forth in the Authorized License,

 1. This program, including its structure, sequence and organization, constitutes the
    valuable trade secrets of Broadcom, and you shall use all reasonable efforts to
    protect the confidentiality thereof,and to use this information only in connection
    with your use of Broadcom integrated circuit products.

 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS" AND WITH
    ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR WARRANTIES, EITHER
    EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO THE SOFTWARE.  BROADCOM
    SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY,
    NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR
    COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION.
    YOU ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.

 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS LICENSORS
    BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR EXEMPLARY DAMAGES
    WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR USE OF OR INABILITY TO USE
    THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES;
    OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF
    OR U.S. $1, WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING
    ANY FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 **************************************************************************************
 */

#ifndef __INIT_H
#define __INIT_H

/**
 * @file init.h
 *
 * This module provides the SerDes and lane-initialization code.
 */

#include <stdint.h>
#include <stdio.h>

#include "errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup DEVICE_CONFIG_APIS
 * @{
 */

/**
 * Type of device.
 */

enum kbp_device_type {
    KBP_DEVICE_UNKNOWN,       /**< Invalid device */
    KBP_DEVICE_12K,           /**< Algorithmic 12K class of devices */
    KBP_DEVICE_OP,            /**< Algorithmic Optimus Prime class of devices */
    KBP_DEVICE_KAPS,          /**< KAPS class of devices */
    KBP_DEVICE_INVALID        /**< This should be the last entry */
};

/**
 * When initializing the device the following flags can be
 * OR'd together to specify the operating mode of the device.
 */

enum kbp_device_flags {
    KBP_DEVICE_DEFAULT = 0,           /**< Single port, non-SMT, nonnetwork byte order, no cascade mode. */
    KBP_DEVICE_SMT = 1,               /**< SMT mode in device enabled. Use kbp_device_thread_init() to obtain the thread's device handle. */
    KBP_DEVICE_DUAL_PORT = 2,         /**< Enable dual-port operation. */
    KBP_DEVICE_NBO = 4,               /**< Enable network byte order. */
    KBP_DEVICE_SUPPRESS_INDEX = 8,    /**< Suppress index generation in result when AD is present. */
    KBP_DEVICE_ISSU = 16,             /**< In-service software upgrade enabled. */
    KBP_DEVICE_CASCADE = 32,          /**< Cascading of KBP's is enabled */
    KBP_DEVICE_SKIP_INIT = 64,        /**< Skip the device init. To be used before ISSU restore. */
    KBP_DEVICE_SUPPRESS_PARITY = 128, /**< Disable Parity scan */
    KBP_DEVICE_QUAD_PORT = 256        /**< Upto four port support (Optimus Prime and beyond only) */
};

/**
 * SerDes lane speeds
 */

enum kbp_device_lane_speed {
    KBP_INIT_LANE_12_5 = 0,     /**< 12.5G SerDes. */
    KBP_INIT_LANE_3_125 = 1,    /**< 3.125G SerDes. */
    KBP_INIT_LANE_6_25 = 2,     /**< 6.25G SerDes. */
    KBP_INIT_LANE_10_3 = 3,     /**< 10.3G SerDes. */
    KBP_INIT_LANE_1_25 = 4,     /**< 1.25G SerDes. */

    KBP_INIT_LANE_15 = 5,       /**< 15G SerDes. */
    KBP_INIT_LANE_25_78125 = 6, /**< 25.78125G SerDes. */
    KBP_INIT_LANE_27_34375 = 7, /**< 27.34375G SerDes */
    KBP_INIT_LANE_28_125 = 8,   /**< 28.125G SerDes. */
    KBP_INIT_LANE_INVALID = 9
};

/**
 * ILA burst short length.
 */

enum kbp_device_ila_burst_short {
    KBP_INIT_BURST_SHORT_8_BYTES = 0, /**< 8-byte burst length. */
    KBP_INIT_BURST_SHORT_16_BYTES = 1,/**< 16-byte burst length. */
    KBP_INIT_BURST_SHORT_INVALID
};

/**
 * Device configuration structure used to initialize the device.
 */

struct kbp_device_config
{
    struct
    {
        uint32_t start_lane;   /**< Start lane number for the port. */
        uint32_t num_lanes;    /**< Number of lanes for the port, must be a multiple of four (Quad). */
    } port_map[4];             /**< The flags argument to kbp_device_init() or kbp_device_interface_init() must be OR'd with KBP_DEVICE_DUAL_PORT, to honor second port info. */

    enum kbp_device_lane_speed speed;      /**< SerDes speed. */
    enum kbp_device_ila_burst_short burst; /**< Short burst size */
    uint32_t meta_frame_len;               /**< Metaframe length. */
    uint32_t reset_only;                   /**< Bitmap to do not enable Rx lanes per port, One bit per port */

    void *handle;                          /**< Opaque handle passed to MDIO reset routines as first argument. */
    uint32_t reverse_lanes;                /**< Bitmap to enable reverse lane per port, One bit per port */

    /**
     * Total number of devices. Default is 1. When KBP_DEVICE_CASCADE
     * is specified, the requested number of devices are brought up.
     * The devices are addressed logically from 0 - 3. The user must
     * translate it to actual MDIO MPID
     */

    uint32_t num_devices;

    /**
     * Callback for asserting SerDes and core resets. A value of one
     * means asserted low (0V); a value of zero means deasserted. Function returns
     * zero on success or indicates an error otherwise. When KBP devices
     * are in cascade, the asserts are expected to be applied to
     * all devices
     *
     * @param handle Opaque user provided handle passed to the callback.
     * @param s_reset_low Assert S_RESET. A value of one means asserted low (0V); a value of zero means deasserted.
     * @param c_reset_low Assert C_RESET. A value of one means asserted low (0V); a value of zero means deasserted.
     */

    int32_t (*assert_kbp_resets)(void *handle, int32_t s_reset_low, int32_t c_reset_low);

    /**
     * MDIO read function. Implemented by the user. Returns a 16b read value as part of the last argument
     *
     * @param handle Opaque user provided handle passed to the callback.
     * @param chip_no Virtual chip number to address the device on cascade. The user is responsible to translate to the real MPID select.
     * @param dev The MDIO device ID.
     * @param reg The MDIO register to be read.
     * @param value Returned by the call. Valid if function returns zero
     *
     * @retval 0 On success, the contents of value are valid
     * @retval non-zero On failure. The contents of value are ignored.
     */

    int32_t (*mdio_read)(void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t *value);

    /**
     * MDIO write function. Implemented by the user. Writes a 16b value to the specified MDIO register
     *
     * @param handle Opaque user provided handle passed to the callback.
     * @param chip_no Virtual chip number to address the device on cascade. The user is responsible to translate to the real MPID select.
     * @param dev The MDIO device ID.
     * @param reg The MDIO register to write to.
     * @param value The value to write to the register.
     *
     * @retval 0 On success, the value has been written.
     * @retval non-zero On failure. Value could not be written to the register.
     */

    int32_t (*mdio_write)(void *handle, int32_t chip_no, uint8_t dev, uint16_t reg, uint16_t value);

    /**
     * Implementation of usleep provided by the user. The function is
     * expected to sleep for the requested micro seconds.
     *
     * @param handle Opaque user provided handle passed to the callback.
     * @param value The number of micro seconds to sleep
     *
     * @retval 0 on success
     * @retval non-zero on failure or not able to sleep for requested time
     */
    int32_t (*usleep)(void *handle, uint32_t value);

    /**
     * Option callback from the initialization code that is
     * called if provided before the TX/RX links are enabled.
     * Optionally NPU's may use this to perform specific
     * operations on their side. On successful return the links
     * will be enabled and active on the KBP
     *
     * @param handle Opaque user provided handle passed to the callback.
     *
     * @retval 0 on success
     * @retval non-zero on failure
     */
    int32_t (*pre_enable_link_callback)(void *handle);

    /**
     * Starting Optimus Prime, load serdes uCode through DMA if available
     * This callback can be NULL, in which case the uCode will be loaded
     * using MDIO callbacks.
     *
     * This callback is filled in by the KBP PCIE device driver
     *
     * @param handle Opaque user provided handle passed to the callback
     * @param ucode The uCode to be downloaded
     * @param len The uCode length in bytes
     *
     * @retval 0 on success
     * @retval non-zero on failure
     */
    int32_t (*ucode_dma_load)(void *handle, const uint8_t *ucode, uint32_t len);

    /**
     * Optional callback function, it should be used only when asymmetric lane usage is required,
     * Also the num lanes for each port in the above configuration should be set to the max lane count to start off with.
     * Each bit set to one enables the lane.
     *
     * @param rx_lanes_bmp Rx Lanes Bitmap
     * @param tx_lanes_bmp Tx Lanes Bitmap
     *
     * @retval 0 on success
     * @retval non-zero on failure
     */
    int32_t (*get_rx_tx_lanes_bmp)(uint64_t *rx_lanes_bmp, uint64_t *tx_lanes_bmp);

    /**
     * Optional callback function, it should be used only when polarity inversion is required for a lane.
     *
     * @param lanes_bmp lane bitmap to enable the polarity inversion, One bit per lane
     *
     * @retval 0 on success
     * @retval non-zero on failure
     */
    int32_t (*get_polarity_invert_lanes_bmp)(uint64_t *lanes_bmp);
};

/**
 * Default initializer for the configuration structure.
 */

#define KBP_DEVICE_CONFIG_DEFAULT                       \
    {                                                   \
        { { 0, 12 }, /* Port 0 lanes */                 \
          { 0, 0  }, /* Port 1 lanes */                 \
          { 0, 0  }, /* Port 2 lanes */                 \
          { 0, 0  }  /* Port 3 lanes */                 \
        },                                              \
        KBP_INIT_LANE_12_5, /* 12.5G SerDes */          \
        KBP_INIT_BURST_SHORT_8_BYTES, /* burst */       \
        0x80, /* meta frame */                          \
        0,    /* Reset only */                          \
        0,    /* handle */                              \
        0,    /* reverse lanes */                       \
        1,    /* Number of devices */                   \
        0,    /* reset function */                      \
        0,    /* MDIO read */                           \
        0,    /* MDIO write */                          \
        0,    /* uSleep */                              \
        0,    /* Pre link up callback */                \
        0,    /* uCode DMA load */                      \
        0,    /* get Rx Tx lanes bitmap */              \
        0,    /* get polarity invert lanes bitmap */    \
  }

/**
 * Initialize the SerDes, and configure the device.
 *
 * @param type The device type ::kbp_device_type.
 * @param flags ::kbp_device_flags OR'd together.
 * @param config Configuration structure initialized appropriately.
 *
 * @return KBP_OK on success or an error code.
 */

kbp_status kbp_device_interface_init(enum kbp_device_type type, uint32_t flags, struct kbp_device_config *config);

/**
 * Logs configuration and interface registers in verbose format
 *
 * Currently supports Optimus Prime only
 *
 * @param type the device type
 * @param flags the same flags used to initialize the device/interface
 * @param config Configuration structure initialized appropriately.
 * @param fp the file pointer to dump into
 *
 * @return KBP_OK on success or an error code.
 */

kbp_status kbp_device_interface_print_regs(enum kbp_device_type type, uint32_t flags, struct kbp_device_config *config, FILE *fp);

/**
 * PRBS polynomial types
 */

enum kbp_prbs_polynomial {
    KBP_PRBS_7  = 0,
    KBP_PRBS_9  = 1,
    KBP_PRBS_11 = 2,
    KBP_PRBS_15 = 3,
    KBP_PRBS_23 = 4,
    KBP_PRBS_31 = 5,
    KBP_PRBS_58 = 6
};
/**
 * Enable PRBS generation on RX and TX side
 *
 * @param type The device type, currently only Optimus Prime is supported
 * @param flags The device configuration flags
 * @param config Interface configuration structure initialized correctly
 * @param prbs_poly PRBS polynomial type
 * @param enable If one, PRBS is enabled, if zero disabled
 *
 * @retval KBP_OK on success or an error code
 */

kbp_status kbp_device_interface_prbs(enum kbp_device_type type, uint32_t flags, struct kbp_device_config *config,
                                     enum kbp_prbs_polynomial prbs_poly, uint32_t enable);

/**
 * Print verbose PRBS information
 *
 * @param type The device type, currently only Optimus Prime is supported
 * @param flags The device configuration flags
 * @param config Interface configuration structure initialized correctly
 *
 * @retval KBP_OK on success or an error code
 */

kbp_status kbp_device_interface_prbs_print(enum kbp_device_type type, uint32_t flags, struct kbp_device_config *config);

/**
 * Print eye scan information for specific lane
 *
 * @param type The device type, currently only Optimus Prime is supported
 * @param flags The device configuration flags
 * @param config Interface configuration structure initialized correctly
 *
 * @retval KBP_OK on success or an error code
 */

kbp_status kbp_device_interface_eyescan_print(enum kbp_device_type type, uint32_t flags, struct kbp_device_config *config);

/**
 * Writes Serdes TXFIR tap settings
 *
 * @param type The device type, currently only Optimus Prime is supported
 * @param config Interface configuration structure initialized correctly
 * @param lane_bmp bitmap to program TXFIR settings, One bit per lane
 * @param pre     TXFIR pre tap value (0..31)
 * @param main    TXFIR main tap value (0..127)
 * @param post1   TXFIR post tap value (-63..63)
 * @param post2   TXFIR post2 tap value (-15..15)
 * @param post3   TXFIR post3 tap value (-15..15)
 *
 * @retval KBP_OK on success or an error code
 */

kbp_status kbp_device_interface_tx_serdes_emphasize(enum kbp_device_type type, struct kbp_device_config *config, uint64_t lane_bmp,
                                                    int8_t pre,int8_t main,int8_t post1,int8_t post2,int8_t post3);

/**
 * Enable/Disable Tx link
 *
 * @param type The device type ::kbp_device_type
 * @param config Interface configuration structure initialized correctly
 * @param port_bmp port bitmap
 * @param enable 1. enable, 0. disable
 *
 * @retval KBP_OK on success or an error code
 */

kbp_status kbp_device_interface_enable_tx_link(enum kbp_device_type type, struct kbp_device_config *config,
                                                  uint8_t port_bmp, uint8_t enable);

/**
 * Enable/Disable Rx link
 *
 * @param type The device type ::kbp_device_type
 * @param config Interface configuration structure initialized correctly
 * @param port_bmp port bitmap
 * @param enable 1. enable, 0. disable
 *
 * @retval KBP_OK on success or an error code
 */

kbp_status kbp_device_interface_enable_rx_link(enum kbp_device_type type, struct kbp_device_config *config,
                                               uint8_t port_bmp, uint8_t enable);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /*__INIT_H */
