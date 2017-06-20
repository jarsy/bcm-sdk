/*
 * $Id: soc.c,v 1.6 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>

#include <bcm/error.h>

#include <soc/mem.h>

/*
 * PBMP
 */

cmd_result_t
cmd_robo_pbmp(int unit, args_t *a)
{
    pbmp_t pbmp;
    char *c;
    soc_port_t port;
    char pbmp_str[80];
    char pfmt[SOC_PBMP_FMT_LEN];

    COMPILER_REFERENCE(unit);

    c = ARG_GET(a);

    if (!c) {
        cli_out("Current bitmaps:\n");
        cli_out("     FE   ==> %s\n",
                SOC_PBMP_FMT(PBMP_FE_ALL(unit), pfmt));
        cli_out("     GE   ==> %s\n",
                SOC_PBMP_FMT(PBMP_GE_ALL(unit), pfmt));
        cli_out("     E    ==> %s\n",
                SOC_PBMP_FMT(PBMP_E_ALL(unit), pfmt));
        cli_out("     PORT ==> %s\n",
                SOC_PBMP_FMT(PBMP_PORT_ALL(unit), pfmt));
        cli_out("     MII  ==> %s\n",
                SOC_PBMP_FMT(PBMP_CMIC(unit), pfmt));
        cli_out("     ALL  ==> %s\n",
                SOC_PBMP_FMT(PBMP_ALL(unit), pfmt));
        return CMD_OK;
    }

    if (sal_strcasecmp(c, "port") == 0) {
        if ((c = ARG_GET(a)) == NULL) {
            cli_out("ERROR: missing port string\n");
            return CMD_FAIL;
        }
        if (parse_port(unit, c, &port) < 0) {
            cli_out("%s: Invalid port string: %s\n", ARG_CMD(a), c);
            return CMD_FAIL;
        }
        cli_out("    port %s ==> %s (%d)\n",
                c, SOC_PORT_NAME(unit, port), port);
        return CMD_OK;
    }

    if (parse_pbmp(unit, c, &pbmp) < 0) {
        cli_out("%s: Invalid pbmp string (%s); use 'pbmp ?' for more info.\n",
                ARG_CMD(a), c);
        return CMD_FAIL;
    }

    format_pbmp(unit, pbmp_str, sizeof (pbmp_str), pbmp);

    cli_out("    %s ==> %s\n", SOC_PBMP_FMT(pbmp, pfmt), pbmp_str);

    return CMD_OK;
}


/*
 * Function:    cmd_robo_soc
 * Purpose: Print soc control information if compiled in debug mode.
 * Parameters:  u - unit #
 *              a - pointer to args, expects <unit> ...., if none passed,
 *                  default unit # used.
 * Returns: CMD_OK/CMD_FAIL
 */
cmd_result_t
cmd_robo_soc(int u, args_t *a)
{
#if defined(BROADCOM_DEBUG)
    char *c;
    cmd_result_t rv = CMD_OK;

    if (!sh_check_attached("soc", u)) {
        return(CMD_FAIL);
    }

    if (!ARG_CNT(a)) {
        rv = soc_robo_dump(u, "Driver Control: ") ? CMD_FAIL : CMD_OK;
    } else {
        while ((CMD_OK == rv) && (c = ARG_GET(a))) {
            if (!isint(c)) {
                cli_out("%s: Invalid unit identifier: %s\n", ARG_CMD(a), c);
                rv = CMD_FAIL;
            } else {
                rv = soc_robo_dump(parse_integer(c), "Driver Control: ") ?
                    CMD_FAIL : CMD_OK;
            }
        }
    }
    return(rv);
#else /* !defined(BROADCOM_DEBUG) */
    cli_out("%s: Warning: Not compiled with BROADCOM_DEBUG enabled\n", ARG_CMD(a));
    return(CMD_OK);
#endif /* defined(BROADCOM_DEBUG) */
}

