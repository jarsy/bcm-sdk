/* 
 * $Id: robo.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        robo.c
 * Purpose:     generic ROBO board driver
 *
 */

#include <shared/bsl.h>

#include <soc/drv.h> /* SOC_IS_ROBO */
#include <sal/core/alloc.h>
#include <sal/core/libc.h>
#include <bcm/error.h>
#include <bcm/stack.h>

#include <board/board.h>
#include <board/manager.h>
#include <board/support.h>
#include <board/driver.h>

#if defined (INCLUDE_BOARD_ROBO)

#define MAX_DESC 80
#define MAX_UNIT 1
#define UNIT 0

#define PRIVATE(field) (((robo_private_data_t *)(driver->user_data))->field)

typedef struct robo_private_data_s {
    char description[MAX_DESC];
    bcm_info_t info;
} robo_private_data_t;

STATIC robo_private_data_t _robo_private_data;

/*
 * Function:
 *     _robo_num_modid_get
 * Purpose:
 *     Get the number of modids required by this board.
 *     ROBO devices have no modids.
 * Parameters:
 *     driver  - (IN)  current driver (ignored)
 *     name    - (IN)  attribute name (ignored)
 *     modid   - (OUT) modid
 * Returns:
 *     BCM_E_NONE - success
 */
STATIC int
_robo_num_modid_get(board_driver_t *driver, char *name, int *num_modid)
{
    *num_modid = 0;
    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_description
 * Purpose:
 *     Return the board description string
 * Parameters:
 *     driver - (IN)  current driver
 * Returns:
 *     description string
 */
STATIC char *
_robo_description(board_driver_t *driver)
{
    return PRIVATE(description);
}

/*
 * Function:
 *     _robo_probe
 * Purpose:
 *     Test if board devices are appropriate for this board.
 *     All ROBO devices are appropriate.
 * Parameters:
 *      driver - (IN) board driver
 *      num    - (IN) number of devices; length of info array
 *      info   - (IN) array of bcm_info_t
 * Returns:
 *     BCM_E_NONE - devices match what is expected for this board
 *     BCM_E_XXX - failed
 */
STATIC int
_robo_probe(board_driver_t *driver, int num, bcm_info_t *info)
{
    int rv = BCM_E_FAIL;

    
    if (num == MAX_UNIT && SOC_IS_ROBO(0)) {
        LOG_VERBOSE(BSL_LS_BOARD_COMMON,
                    (BSL_META(__FILE__": accept\n")));
        rv = BCM_E_NONE;
    } else {
        LOG_VERBOSE(BSL_LS_BOARD_COMMON,
                    (BSL_META(__FILE__": reject - num_units(%d) != %d\n"),
                     num,
                     MAX_UNIT));
    }

    return rv;
}

/*
 * Function:
 *     _robo_start
 * Purpose:
 *     Start board according to flags
 * Parameters:
 *      driver - (IN) current driver
 *      flags  - (IN) start flags
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
STATIC int
_robo_start(board_driver_t *driver, uint32 flags)
{

    /* init the unit */
    if (flags & BOARD_START_F_WARM_BOOT) {
        return BCM_E_UNAVAIL;
    } else {
        BCM_IF_ERROR_RETURN(board_device_init(UNIT));
        BCM_IF_ERROR_RETURN(board_port_init(UNIT));
    }
    
    if (!(flags & (BOARD_START_F_CLEAR|BOARD_START_F_WARM_BOOT))) {

        /* Cold start */

        /* Reset user data */
        sal_memset(driver->user_data, 0, sizeof(robo_private_data_t));

        BCM_IF_ERROR_RETURN(bcm_info_get(UNIT, &PRIVATE(info)));

        sal_sprintf(PRIVATE(description),
                    "Robo %s board driver",
                    board_device_name(UNIT));

    } else if (flags & BOARD_START_F_CLEAR) {
        /* nothing to clear */
    }

    LOG_VERBOSE(BSL_LS_BOARD_COMMON,
                (BSL_META(__FILE__": started %s - %s\n"),
                 robo_board.name,
                 PRIVATE(description)));
    
    return BCM_E_NONE;
}

/*
 * Function:
 *     _robo_stop
 * Purpose:
 *     Stop the board
 * Parameters:
 *      driver - (IN) current driver
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
STATIC int
_robo_stop(board_driver_t *driver)
{
    BCM_IF_ERROR_RETURN(board_port_deinit(UNIT));
    BCM_IF_ERROR_RETURN(board_device_deinit(UNIT));

    /* Reset user data */
    sal_memset(driver->user_data, 0, sizeof(robo_private_data_t));

    return BCM_E_NONE;
}

/* Supported attributes */
STATIC board_attribute_t _robo_attrib[] = {
    {
        BOARD_ATTRIBUTE_NUM_MODID,
        _robo_num_modid_get,
        NULL
    }
};

/* Board driver */
board_driver_t robo_board = {
    "robo",
    BOARD_GENERIC_PRIORITY+1,
    (void *)&_robo_private_data,
    0,
    NULL,
    COUNTOF(_robo_attrib),
    _robo_attrib,
    _robo_description,
    _robo_probe,
    _robo_start,
    _robo_stop
};

#endif /* INCLUDE_BOARD_ROBO */
