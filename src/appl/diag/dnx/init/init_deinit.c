/** \file init_deinit.c
 * init and deinit functions to be used by the INIT_DNX command.
 */

/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifdef BSL_LOG_MODULE
#error "BSL_LOG_MODULE redefined"
#endif
#define BSL_LOG_MODULE BSL_LS_APPLDNX_INITSEQDNX

/*
* INCLUDE FILES:
* {
*/

#include <ibde.h>
#include <appl/diag/dnx/init_deinit.h>
#include <shared/shrextend/shrextend_debug.h>
#include <appl/diag/sysconf.h>
#include <soc/cmext.h>
#include <bcm/init.h>
#include <appl/diag/sand/diag_sand_utils.h>

/*
 * }
 */

char appl_dnx_init_usage[] =
#ifdef COMPILER_STRING_CONST_LIMIT
    "Application Deinit init short Usage:\n" "  NoInit=<value>, NoDeinit=<value>, NoBcm=<value>"
#else
    "Application Init/Deinit Full Usage:\n"
    "  NoInit=<value>         1: Init sequence will not be performed on the chip. (default=0)\n"
    "  NoDeinit=<value>       1: Deinit sequence will not be performed on the chip. (default=1).\n"
#endif
    ;

/**
 * \brief - parse parameters for DNX_INIT command.
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] args - arguments recieved to parse over.
 *   \param [out] init_param - returned initialization
 *          parameters
 *   
 * \par INDIRECT INPUT:
 *   * None
 * \par DIRECT OUTPUT:
 *   shr_error_e 
 * \par INDIRECT OUTPUT
 *   * None
 * \remark
 *   Addidtional options may be added, however the general
 *   guidline is to control different init/deinit options using
 *   SOC properties. each new option added here needs to be
 *   added as well to the usage section above.
 * \see
 *   * None
 */
static shr_error_e
appl_dnx_init_parse_params(
    int unit,
    args_t * args,
    appl_dnx_init_param_t * init_param)
{
    parse_table_t parse_table;

    SHR_FUNC_INIT_VARS(unit);

    parse_table_init(unit, &parse_table);
    /*
     * additional option might be added in the future, it is important to add
     * those options also to the usage string found in appl_dnx_init_usage. 
     */
    SHR_IF_ERR_EXIT(parse_table_add(&parse_table, "NoInit", PQ_INT, (void *) 0, &(init_param->no_init), NULL));
    SHR_IF_ERR_EXIT(parse_table_add
                    (&parse_table, "NoDeinit", PQ_INT | PQ_DFL, (void *) 1, &(init_param->no_deinit), NULL));

    SHR_IF_ERR_EXIT(parse_arg_eq(args, &parse_table));

    /*
     * make sure that no extra args were recieved as input
     */
    if (ARG_CNT(args) != 0)
    {
        SHR_ERR_EXIT(_SHR_E_PARAM, "%s: extra options starting with \"%s\"\n%s\n", ARG_CMD(args), ARG_CUR(args),
                     appl_dnx_init_usage);
    }

    /*
     * This is used to free memory allocated for parse_table_init
     */
    parse_arg_eq_done(&parse_table);

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - main init function
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] init_param - init parameters.
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
static shr_error_e
appl_dnx_init(
    int unit,
    appl_dnx_init_param_t * init_param)
{
    int rv;
    uint16 device_id;
    uint8 revision_id;
    const ibde_dev_t *device;

    SHR_FUNC_INIT_VARS(unit);

    /*
     * Override device ID and revision if needed
     */
    device = bde->get_dev(unit);
    device_id = device->device;
    revision_id = device->rev;
    sysconf_chip_override(unit, &device_id, &revision_id);

    /*
     * Step 1 (cmext.h)
     */
    rv = soc_cm_device_supported(device_id, revision_id);
    SHR_IF_ERR_EXIT_WITH_LOG(rv, "The Device is not supported, attemped to probe for Device:%u, Revision:%u\n%s",
                             device_id, revision_id, EMPTY);
    /*
     * Step 2 (cmext.h)
     */
    rv = soc_cm_device_create(device_id, revision_id, NULL);
    SHR_IF_ERR_EXIT_WITH_LOG(rv, "soc_cm_device_create() for device_id %u Failed:\n%s%s", device_id, EMPTY, EMPTY);

    /*
     * Step 3 (cmext.h) 
     * Attach unit: 
     * perform all needed actions to call bcm_init, 
     * essentially this preliminary step need to be 
     * as lean as possible, as many steps as possible 
     * should be called from inside bcm_init.
     */
    LOG_INFO_EX(BSL_LOG_MODULE, "Attach unit.\n%s%s%s%s", EMPTY, EMPTY, EMPTY, EMPTY);
    rv = sysconf_attach(unit);
    SHR_IF_ERR_EXIT_WITH_LOG(rv, "sysconf_attach() Failed:\n%s%s%s", EMPTY, EMPTY, EMPTY);

    
    rv = bcm_init(unit);
    SHR_IF_ERR_EXIT_WITH_LOG(rv, "bcm_init() Failed:\n%s%s%s", EMPTY, EMPTY, EMPTY);

exit:
    SHR_FUNC_EXIT;
}

/**
 * \brief - main deinit function
 * 
 * \par DIRECT_INPUT:
 *   \param [in] unit - unit #
 *   \param [in] init_param - (de)init parameters.
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
static shr_error_e
appl_dnx_deinit(
    int unit,
    appl_dnx_init_param_t * init_param)
{
    int rv;

    SHR_FUNC_INIT_VARS(unit);

    
    /*
     * rv = bcm_deinit(unit);
     * SHR_IF_ERR_CONT(rv);
     */

    rv = soc_cm_device_destroy(unit);
    SHR_IF_ERR_CONT(rv);

    SHR_FUNC_EXIT;
}

cmd_result_t
cmd_dnx_init_dnx(
    int unit,
    args_t * args)
{
    appl_dnx_init_param_t init_param;

    SHR_FUNC_INIT_VARS(unit);

    sal_memset(&init_param, 0x0, sizeof(init_param));

    /*
     * default init_param values
     */
    init_param.no_deinit = 1;

    /*
     * parse over incoming args
     */
    SHR_IF_ERR_EXIT(appl_dnx_init_parse_params(unit, args, &init_param));

    /*
     * Deinit if needed
     */
    if (init_param.no_deinit == 0)
    {
        SHR_IF_ERR_EXIT(appl_dnx_deinit(unit, &init_param));
    }

    /*
     * Init if needed
     */
    if (init_param.no_init == 0)
    {
        SHR_IF_ERR_EXIT(appl_dnx_init(unit, &init_param));
    }

exit:
    SHR_ERR_TO_SHELL;
    SHR_FUNC_EXIT;
}
