/*
 * $Id: mirror.c,v 1.5 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Mirror CLI commands
 */
#include <shared/bsl.h>

#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/dport.h>


#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <bcm/error.h>
#include <bcm/mirror.h>
#include <bcm/stack.h>
#include <bcm/debug.h>

char cmd_sbx_mirror_usage[] = 
"Usage:\n"
" MIRror <port> [mode=off|none|ingress|egress|both] [[dest=mod.port]|port=port]\n\n"
"Installs an ingress and/or egress mirror on a <port>.  If a mirror exists on\n"
"the <port>, then the existing mirror will be removed and replaced by the\n"
"one specified.\nIf only <port> is supplied, the configured mirror is diplayed.\n"
"\n";

cmd_result_t
cmd_sbx_mirror(int unit, args_t *args)
{

    enum { 
        mirrorModeOff, mirrorModeNone,
        mirrorModeIngress, mirrorModeEgress, mirrorModeBoth,
        mirrorModeCount
    };

    char          *str;
    int            mode_arg, rv;
    uint32       flags;
    bcm_port_t     mirror_port;
    bcm_mod_port_t modport_dest;
    parse_table_t  pt;
    cmd_result_t   ret_code;
    char          *mode_list[mirrorModeCount+1];

    mode_list[mirrorModeOff]     = "Off";
    mode_list[mirrorModeNone]    = "None";
    mode_list[mirrorModeIngress] = "Ingress";
    mode_list[mirrorModeEgress]  = "Egress";
    mode_list[mirrorModeBoth]    = "Both";
    mode_list[mirrorModeCount]   = NULL;
    
    flags             = 0;
    mode_arg          = -1;
    modport_dest.mod  = BCM_MODID_INVALID;
    modport_dest.port = -1;

    if (!SOC_IS_SBX_CALADAN3(unit)) {
        cli_out("not supported on this device.\n");
        return CMD_USAGE;
    }

    if (ARG_CNT(args) == 0) {
#ifdef BCM_CALADAN3_SUPPORT
        if (SOC_IS_SBX_CALADAN3(unit)) {
            void _bcm_caladan3_mirror_sw_dump(int unit);
            
            _bcm_caladan3_mirror_sw_dump(unit);
        }
#endif
        return CMD_OK;
    }

    /* Get the first argument, it must be the port to mirror */
    str = ARG_GET(args);
    if (!isint(str)) {
        cli_out("'%s' is not a valid port\n", str);
        return CMD_USAGE;
    }

    mirror_port = parse_integer(str);

    if (SOC_PORT_VALID(unit, mirror_port) == FALSE) {
        cli_out("Invalid mirror port specified: %d\n", mirror_port);
        return CMD_USAGE;
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "dest", PQ_DFL | PQ_MOD_PORT, 0, &modport_dest, NULL);
    parse_table_add(&pt, "port", PQ_DFL | PQ_INT, 0, &modport_dest.port, NULL);
    parse_table_add(&pt, "mode", PQ_DFL | PQ_MULTI, 0, &mode_arg, mode_list);

    /* No more args?  Show the state of the mirror port, else configure it */
    if (ARG_CNT(args) == 0) {
        bcm_port_t     dest_port;
        bcm_module_t   dest_mod;

        rv = bcm_mirror_port_get(unit, mirror_port, 
                                 &dest_mod, &dest_port, &flags);
        if (BCM_FAILURE(rv)) {
            cli_out("Failed to get mirror port info: %s\n", bcm_errmsg(rv));
            return CMD_FAIL;
        }

        if (flags & BCM_MIRROR_PORT_ENABLE) {
            char *dir = NULL;

            if (flags & BCM_MIRROR_PORT_INGRESS) {
                dir = "Ingress";
            }

            if (flags & BCM_MIRROR_PORT_EGRESS) {
                if (dir == NULL) {
                    dir = "Egress";
                } else {
                    dir = "Ingress/Egress";
                }
            }

            cli_out("%s Mirror enabled on port %d to modport %d.%d\n", 
                    dir, mirror_port, dest_mod, dest_port);
            
        } else {
            cli_out("Port %d has no mirrors enabled\n", mirror_port);
        }

        return CMD_OK;
    }

    if (!parseEndOk(args, &pt, &ret_code)) {
        return ret_code;
    }

    /* Configure the mirror(s) */

    flags = BCM_MIRROR_PORT_ENABLE;
    switch (mode_arg) 
    {
    case mirrorModeOff:      /* fall thru intentional */
    case mirrorModeNone:     flags &= ~BCM_MIRROR_PORT_ENABLE;  break;
    case mirrorModeIngress:  flags |=  BCM_MIRROR_PORT_INGRESS; break;
    case mirrorModeEgress:   flags |=  BCM_MIRROR_PORT_EGRESS;  break;
    case mirrorModeBoth:
        flags |= BCM_MIRROR_PORT_INGRESS | BCM_MIRROR_PORT_EGRESS;
        break;
    default:
        return CMD_FAIL;
    }

    /* Validate the mod/port only if the mirror is enabled  */
    if (flags & BCM_MIRROR_PORT_ENABLE) {
        if (SOC_PORT_VALID(unit, modport_dest.port) == FALSE) {
            cli_out("Invalid destination port specified: %d\n", 
                    modport_dest.port);
            return CMD_USAGE;
        }
        
        if (modport_dest.mod == BCM_MODID_INVALID) {
            
            rv = bcm_stk_modid_get(unit, &modport_dest.mod);
            if (BCM_FAILURE(rv)) {
                cli_out("Failed to get modid: %s\n", bcm_errmsg(rv));
                return CMD_FAIL;
            }
        }
    }

    rv = bcm_mirror_port_set(unit, mirror_port, 
                             modport_dest.mod, modport_dest.port, flags);
    if (BCM_FAILURE(rv)) {
        cli_out("Failed to set mirror port: %s\n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (flags & BCM_MIRROR_PORT_ENABLE) {
        cli_out("Installed mirror on port %d to mod/port %d/%d\n",
                mirror_port, modport_dest.mod, modport_dest.port);
    } else {
        cli_out("Removed mirror(s) from port %d\n", mirror_port);
    }

    return CMD_OK;
}

