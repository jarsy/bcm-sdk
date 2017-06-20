/** \file access.c
 * access related SOC functions and Access related CBs for init mechanism.
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_DNX_ACCESS_H
/*
 * { 
 */
#define _SOC_DNX_ACCESS_H

#include <bcm_int/dnx/init/init.h>

/**
 * \brief - allocates memory mutexes
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
shr_error_e dnx_access_mem_mutex_init(
    int unit);

/**
 * \brief - free memory mutexes
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
shr_error_e dnx_access_mem_mutex_deinit(
    int unit);

/*
 * } 
 */
#endif /* _SOC_DNX_ACCESS_H */
