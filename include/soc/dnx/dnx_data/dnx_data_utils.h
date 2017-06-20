/*! \file dnx_data_utils.h
 * 
  * 
 * DEVICE DATA UTILS - Utilities function for DNX DATA
 * 
 * Device Data
 * SW component that maintains per device data
 * The data is static and won't be changed after device initialization.
 *     
 * Supported data types:
 *     - Define             - a 'uint32' number (a max value for all devices is maintained)
 *     - feature            - 1 bit per each feature (supported/not supported) - support soc properties 
 *     - table              - the data is accessed with keys and/or can maintain multiple values and/or set by soc property
 *     - numeric            - a 'uint32' number that support soc properties
 * 
 * User interface for DNX DATA component can be found in "dnx_data_if.h" and "dnx_data_if_#module#.h"
 * 
 * Adding the data is done via XMLs placed in "tools/autocoder/DeviceData/dnx/.." 
 * "How to" User Guide can be found in confluence.
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef _DNX_DATA_UTILS_H_
/*{*/
#define _DNX_DATA_UTILS_H_

/**
* \brief This file is only used by DNX (JR2 family). Including it by
* software that is not specific to DNX is an error.
*/
#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

/*
 * FUNCTIONS:
 * {
 */
/**
 * \brief Check weather to dump data according to input:
 * \par INDIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] state_flags - component state  \n
 *     see DNX_DATA_STATE_F_* for details
 *   \param [in] data_flags - specific data flags \n
 *     see DNX_DATA_F_* for details
 *   \param [in] dump_flags - flags to dump \n
 *     see DNX_DATA_F_* for details
 *  
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   \param [out] dump - whether to dump or not \n
 *     see DNX_DATA_F_* for details
 * \remark
 *   * None
 * \see
 *   * None
 */
shr_error_e dnx_data_utils_dump_verify(
    int unit,
    uint32 state_flags,
    uint32 data_flags,
    uint32 dump_flags,
    int *dump);

/*!
 * \brief Generic function to get dnx data 
 *        (will be mostly used as xml pointer read)
 *        The function will look by name the data and will return const pointer to the required data.
 * \par INDIRECT INPUT:
 *   \param [in] unit - Unit #
 *   \param [in] module - module name
 *   \param [in] submodule - submodule name
 *   \param [in] data - data namme
 *   \param [in] member - member name - required only to get data from tables (otherwiae should be NULL)
 * \par INDIRECT INPUT:
 *   module global data - _dnx_data[unit]
 * \par DIRECT OUTPUT:
 *   see shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
const uint32 *dnx_data_utils_generic_data_get(
    int unit,
    char *module,
    char *submodule,
    char *data,
    char *member);
/*
 * }
 */

/*}*/
#endif /*_DNX_DATA_UTILS_H_*/
