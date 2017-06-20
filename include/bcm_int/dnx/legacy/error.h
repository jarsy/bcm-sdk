/*
 * $Id: error.h,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Module: Error translation
 */

#ifndef _BCM_INT_DNX_ERROR_H_
#define _BCM_INT_DNX_ERROR_H_

#include <bcm/debug.h>
#include <bcm/error.h>
#include <soc/dnx/legacy/error.h>

#define DNX_BCM_SAND_IF_ERR_EXIT(_sand_ret) \
    BCMDNX_IF_ERR_EXIT(dnx_handle_sand_result(_sand_ret))

#define DNX_BCM_SAND_IF_ERR_EXIT_NO_UNIT(_sand_ret) \
    BCMDNX_IF_ERR_EXIT_NO_UNIT(dnx_handle_sand_result(_sand_ret))


#endif /* _BCM_INT_DNX_ERROR_H_ */

