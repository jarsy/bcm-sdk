/*
 * $Id: bcm988230.c,v 1.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        bcm988230.c
 * Purpose:     bcm988230 (Sirius) board driver
 *
 * This is an example of how existing Sirius board driver code could
 * be bootstrapped into the board driver framework. This is not
 * intended for production use.
 *
 */

#include <soc/devids.h>
#include <sal/core/libc.h>
#include <sal/core/alloc.h>
#include <sal/appl/io.h>
#include <bcm/error.h>
#include <bcm/stack.h>
#include <board/board.h>
#if defined(INCLUDE_LIB_STKTASK)
#include <appl/stktask/topo_brd.h>
#endif
#if defined(INCLUDE_LIB_DISCOVER)
#include <appl/discover/disc.h>
#endif

/* Needed for config and rc loader */
#include <sal/appl/config.h>
#include <appl/diag/shell.h>

#include <board/manager.h>
#include <board/support.h>
#include <board/driver.h>

#if defined (INCLUDE_BOARD_BCM988230)

#define MAX_DESC 80
#define MAX_UNIT 1
#define UNIT 0

#define PRIVATE(field) \
    (((bcm988230_private_data_t *)(driver->user_data))->field)

typedef struct bcm988230_private_data_s {
    char description[MAX_DESC];
    board_connection_t *conn;
    int conn_count;
    bcm_info_t info;
    int modid;
    bcm_gport_t cpu;
} bcm988230_private_data_t;

STATIC bcm988230_private_data_t _bcm988230_private_data;

/* Supported devices */
STATIC int _bcm8823x_dev[] = {
    BCM88230_DEVICE_ID,
    BCM88235_DEVICE_ID
};

/*
 * Function:
 *     _bcm988230_modid_get
 * Purpose:
 *     Get board modid
 * Parameters:
 *     driver  - (IN)  current driver (ignored)
 *     name    - (IN)  attribute name (ignored)
 *     modid   - (OUT) modid
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_PARAM - modid param NULL
 */
STATIC int
_bcm988230_modid_get(board_driver_t *driver, char *name, int *modid)
{
    if (!modid) {
        return BCM_E_PARAM;
    }

    *modid = PRIVATE(modid);

    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm988230_modid_set
 * Purpose:
 *     Set the board modid
 * Parameters:
 *     driver  - (IN)  current driver (ignored)
 *     name    - (IN)  attribute name (ignored)
 *     modid   - (IN) modid
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
STATIC int
_bcm988230_modid_set(board_driver_t *driver, char *name, int modid)
{
    /* iterate across all units, setting modid. Handle even modid case. */
    PRIVATE(modid) = modid;
    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_set(UNIT, PRIVATE(modid)));

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm988230_num_modid_get
 * Purpose:
 *     Get the number of modids required by this board
 * Parameters:
 *     driver  - (IN)  current driver (ignored)
 *     name    - (IN)  attribute name (ignored)
 *     modid   - (OUT) modid
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
STATIC int
_bcm988230_num_modid_get(board_driver_t *driver, char *name, int *num_modid)
{
    return bcm_stk_modid_count(UNIT, num_modid);
}

/*
 * Function:
 *     _bcm988230_description
 * Purpose:
 *     Return the board description string
 * Parameters:
 *     driver - (IN)  current driver
 * Returns:
 *     description string
 */
STATIC char *
_bcm988230_description(board_driver_t *driver)
{
    return PRIVATE(description);
}

/*
 * Function:
 *     _bcm988230_probe
 * Purpose:
 *     Test if board devices are appropriate for this board
 * Parameters:
 *      driver - (IN) board driver
 *      num    - (IN) number of devices; length of info array
 *      info   - (IN) array of bcm_info_t
 * Returns:
 *     BCM_E_NONE - devices match what is expected for this board
 *     BCM_E_XXX - failed
 */
STATIC int
_bcm988230_probe(board_driver_t *driver, int num, bcm_info_t *info)
{
    int rv = BCM_E_FAIL;
    int i;

    if (num == MAX_UNIT) {
        for (i=0; i<COUNTOF(_bcm8823x_dev); i++) {
            if (_bcm8823x_dev[i] == info[0].device) {
                rv = BCM_E_NONE;
                break;
            }
        }
    }

    return rv;
}

/*
 * Function:
 *     _bcm988230_start
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
_bcm988230_start(board_driver_t *driver, uint32 flags)
{

    /* init the unit */
    if (flags & BOARD_START_F_WARM_BOOT) {
        return BCM_E_UNAVAIL;
    } else {
        /* Reset user data */
        sal_memset(driver->user_data, 0, sizeof(bcm988230_private_data_t));

#if 0
        /* This should be used eventually */
        BCM_IF_ERROR_RETURN(board_device_init(UNIT));
        BCM_IF_ERROR_RETURN(board_port_init(UNIT));
#else
        /* Just use the existing config and soc files and see
           if it works. */
        if (sal_config_file_set("config-sbx-sirius.bcm",
                                "config-sbx-sirius.tmp") < 0) {
            return BCM_E_FAIL;
        }
        /* Merge station_mac_address if it exists */
        {
            char *name = NULL;
            char *value = NULL;

            while (!sal_config_get_next(&name, &value)) {
                sal_printf("*** config %s=%s\n", name, value);
                if (!sal_strcmp(name, "station_mac_address")) {
                    sal_printf("*** found station_mac_address\n");
                    name = sal_strdup(name);
                    value = sal_strdup(value);
                    break;
                }
            }
            /* load new config vars */
            sal_config_refresh();
            if (name) {
                /* restore */
                sal_config_set(name, value);
                sal_printf("*** restore %s=%s\n", name, value);
            }
            sal_config_show();
        }
        sal_printf("reading sbx.soc\n");
        sh_rcload_file(0, NULL, "sbx.soc", 0);
        sal_printf("done with sbx.soc\n");
#endif
    }

        BCM_IF_ERROR_RETURN(bcm_info_get(UNIT, &PRIVATE(info)));

        sal_sprintf(PRIVATE(description),
                    "%s board driver",
                    board_device_name(UNIT));

    /* Set up board parameters */
    if (!(flags & (BOARD_START_F_CLEAR|BOARD_START_F_WARM_BOOT))) {

        BCM_IF_ERROR_RETURN
            (board_connections_discover
             (bcm988230_board.num_connection,
              bcm988230_board.connection,
              &bcm988230_board.num_connection));


        /* Set initial CPU port and modid */
        BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(UNIT, &PRIVATE(modid)));
        BCM_IF_ERROR_RETURN(board_device_cpu_port_get(UNIT, &PRIVATE(cpu)));

    } else if (flags & BOARD_START_F_CLEAR) {
        /* Reset */
        BCM_IF_ERROR_RETURN(bcm_stk_my_modid_set(UNIT, PRIVATE(modid)));
    }


    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm988230_stop
 * Purpose:
 *     Stop the board
 * Parameters:
 *      driver - (IN) current driver
 * Returns:
 *     BCM_E_NONE - success
 *     BCM_E_XXX - failed
 */
STATIC int
_bcm988230_stop(board_driver_t *driver)
{
#if 0
    int unit;

    /* deinit all the units */
    for (unit=0; unit<MAX_UNIT; unit++) {
        BCM_IF_ERROR_RETURN(board_port_deinit(unit));
        BCM_IF_ERROR_RETURN(board_device_deinit(unit));
    }
#endif
    /* Free connection data */
    sal_free(PRIVATE(conn));

    /* Reset user data */
    sal_memset(driver->user_data, 0, sizeof(bcm988230_private_data_t));

    return BCM_E_NONE;
}

/* Supported attributes */
STATIC board_attribute_t _bcm988230_attrib[] = {
    {
        BOARD_ATTRIBUTE_MODID,
        _bcm988230_modid_get,
        _bcm988230_modid_set,
    },
    {
        BOARD_ATTRIBUTE_NUM_MODID,
        _bcm988230_num_modid_get,
        NULL
    }
};

STATIC board_connection_t _bcm988230_conn[BCM_PBMP_PORT_MAX];

/* Board driver */
board_driver_t bcm988230_board = {
    "bcm988230",
    BOARD_DEFAULT_PRIORITY,
    (void *)&_bcm988230_private_data,
    COUNTOF(_bcm988230_conn),
    _bcm988230_conn,
    COUNTOF(_bcm988230_attrib),
    _bcm988230_attrib,
    _bcm988230_description,
    _bcm988230_probe,
    _bcm988230_start,
    _bcm988230_stop
};

#endif
