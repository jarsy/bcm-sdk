/** \file soc/dnx/drv.h
 * Slim SoC module to allow bcm actions.
 * 
 * This file contains structure and routine declarations for the
 * Switch-on-a-Chip Driver.
 *
 * This file also includes the more common include files so the
 * individual driver files don't have to include as much. 
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_DNX_DRV_H
/*
 * { 
 */
#define _SOC_DNX_DRV_H

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*
 * INCLUDE FILES:
 * {
 */
#include <sal/types.h>
#include <shared/cyclic_buffer.h>
#include <shared/utilex/utilex_framework.h>
#include <shared/utilex/utilex_sorted_list.h>
#include <shared/utilex/utilex_occupation_bitmap.h>
#include <shared/utilex/utilex_hashtable.h>
#include <shared/utilex/utilex_multi_set.h>
#include <soc/drv.h>
#ifdef BCM_CMICM_SUPPORT
#include <soc/cmicm.h>
#endif
#include <soc/scache.h>
#include <soc/mem.h>
#include <soc/dnxc/legacy/dnxc_defs.h>
#include <soc/dnxc/legacy/error.h>
#include <soc/dnx/legacy/drv.h>
#include <soc/dnx/legacy/dnx_config_imp_defs.h>
#include <soc/dnx/legacy/dnx_config_defs.h>
#include <bcm_int/dnx/init/init.h>
/*
 * }
 */

/*
 * Structs and Enums:
 * {
 */
/*
 * }
 */
/*
 * MACROs:
 
 */

#define SOC_IS_DNX_TYPE(dev_type)         ((dev_type) == BCM88690_DEVICE_ID)

/*
 * }
 */

int soc_dnx_info_config(
    int unit);
int soc_dnx_info_config_blocks(
    int unit);
int soc_dnx_init_reset(
    int unit,
    int reset_action);
int soc_dnx_init_reg_reset(
    int unit,
    int reset_action);
int soc_dnx_device_reset(
    int unit,
    int mode,
    int action);

/**
 * \brief - Setting chip type - used to implemented SOC_IS... MACROs
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] dev_id - the device ID enumeration value
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_chip_type_set(
    int unit,
    uint16 dev_id);

/**
 * \brief - matching the correct driver to the chip currently running.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] pci_dev_id - the device ID as identified by the PCI.
 *   \param [in] pci_rev_id - the revision ID as identified by the PCI.
 *   \param [out] found_driver - pointer to the found driver code
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_chip_driver_find(
    int unit,
    uint16 pci_dev_id,
    uint8 pci_rev_id,
    soc_driver_t ** found_driver);

/**
 * \brief - attach unit in the SOC level - should be minimal, 
 * most of the init should be done via the bcm_init or bcm_init_advanced
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
extern shr_error_e soc_dnx_attach(
    int unit);

/**
 * \brief - detach unit - basically undo the steps done in attach
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
extern shr_error_e soc_dnx_detach(
    int unit);

/**
 * \brief - Restores SOC control to its primal state. Should be 
 *        used either during init of soc control or during
 *        deinit to bring it back to initial state.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   SOC_CONTROL is initialized with the driver and attached
 *   flag
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_restore(
    int unit);


shr_error_e soc_dnx_sw_state_utils_init(
    int unit);


shr_error_e soc_dnx_sw_state_utils_deinit(
    int unit);

/**
 * \brief - allocates needed Mutexes for device
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_mutexes_init(
    int unit);

/**
 * \brief - free Devices Mutexes
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_mutexes_deinit(
    int unit);

/**
 * \brief - allocates dnx control and configuration structures
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_control_init(
    int unit);

/**
 * \brief - free dnx control and configuration structures
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e soc_dnx_control_deinit(
    int unit);

/*
 * } 
 */
#endif /* _SOC_DNX_DRV_H */
