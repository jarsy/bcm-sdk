/** \file init_deinit.h
 *
 * init and deinit functions to be used by the INIT_DNX command.
 *
 * need to make minimal amount of steps before we are able to use call bcm_init
 *
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef FILE_INIT_DEINIT_H_INCLUDED
/* { */
#define FILE_INIT_DEINIT_H_INCLUDED

#ifndef BCM_DNX_SUPPORT
#error "This file is for use by DNX (JR2) family only!"
#endif

#include <appl/diag/diag.h>

/*
* MACROs:
* {
*/

/*
 * }
 */

/*
 * Structs and Enums:
 * {
 */

/**
 * \brief dnx init parameters
 */
typedef struct
{
    uint32 unit;
    int no_init;
    int no_deinit;
} appl_dnx_init_param_t;

/*
 * }
 */

/*
* Function Declarations:
* {
*/

/**
 * \brief - BCM Shell command to init/deinit DNX
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] args - arguments recieved to parse over.
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   cmd_result_t 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   * None
 * \see
 *   * None
 */
cmd_result_t cmd_dnx_init_dnx(
    int unit,
    args_t * args);

/*
 * }
 */

/* } */
#endif
