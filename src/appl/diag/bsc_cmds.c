/*
 * $Id: bsc_cmds.c,v 1.14 Broadcom SDK $
 *
 * BSC (Broadcom Serial Control) specific commands.
 * These commands are specific to the BSC driver and or BSC slave
 * devices which use that driver. See drv/i2c/ for more details.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#ifndef NO_SAL_APPL
#ifdef INCLUDE_I2C
#ifdef BCM_FE2000_SUPPORT

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/pci.h>
#include <sal/appl/config.h>
#include <shared/bsl.h>
#include <soc/cmext.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/i2c.h>
#include <soc/bsc.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/bcmi2c.h>
#include <bcm/init.h>
#include <bcm/error.h>

#include <appl/diag/system.h>

/*
 * Function: cmd_bsc
 * Purpose: BSC probe/attach/show, configuration commands.
 *	  Also sets up board index and configures BSC drivers
 *	  based on the inferred system board type.
 */
char cmd_bsc_usage[] =
	"\n\t"
	"bsc probe\n\t"
	"    - probe devices on BSC bus and build device tree\n\t"
	"bsc show\n\t"
	"    - show devices on BSC bus.\n\t"
	"bsc setmux <channel:0-7>\n\t"
	"    - set PCA9548 mux channel\n\t"
	"bsc epr <addr> <len>\n\t"
	"    - show eeprom contents at <addr> for <len> bytes\n\t"
	"bsc epw <addr> <string>\n\t"
	"    - write <string> to eeprom at <addr>\n\t"
	"bsc eptest\n\t"
	"    - eeprom destructive test\n\t"
	"bsc sfpread <sfpid> <addr> <len>\n\t"
	"    - show SFP contents at <addr> for <len> bytes\n\t"
	"bsc sfpwrite <sfpid> <addr> data\n\t"
	"    - write data to SFP at <addr>\n"
	;

cmd_result_t
cmd_bsc(int unit, args_t *a)
{
	char *function, *tmp;
	int retv, chan, devno, len, i;
	uint8 *buf;
	uint32 addr, data, rlen;

	if (!sh_check_attached(ARG_CMD(a), unit))
		return CMD_FAIL;
    
	function = ARG_GET(a);

	if (!function) {
		return CMD_USAGE;
	} else if (!sal_strncasecmp(function, "setmux", sal_strlen(function))) {
		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		chan = parse_integer(tmp);
		if ((chan < 0) || (chan > 7)) {
			return CMD_USAGE;
		}
		soc_bsc_setmux(unit, chan);
	} else if (!sal_strncasecmp(function, "readmax", sal_strlen(function))) {
		soc_bsc_readmax(unit);
	} else if (!sal_strncasecmp(function, "readdpm", sal_strlen(function))) {
		soc_bsc_readdpm(unit);
	} else if (!sal_strncasecmp(function,"probe", sal_strlen(function))) {
		retv = soc_bsc_probe(unit); /* Will attach if not yet attached */
		if (retv < 0) {
			cli_out("%s: Error: probe failed: %s\n",
                    ARG_CMD(a), bcm_errmsg(retv));
		} else {
			cli_out("%s: detected %d device%s\n", ARG_CMD(a), retv, retv==1 ? "" : "s");
		}
	} else if (!sal_strncasecmp(function,"show", sal_strlen(function))) {
		soc_bsc_show(unit); /* Will attach if not yet attached */
	} else if (!sal_strncasecmp(function,"epr", sal_strlen(function))) {
                char buf_tmp[17];
                buf_tmp[16] = '\0';
                buf_tmp[0] = '\0';

		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		addr = parse_integer(tmp);
		tmp = ARG_GET(a);
		if (tmp) {
			len = parse_integer(tmp);
		} else {
			len = 0x20;
		}

		if (addr + len > AT24C64_DEVICE_RW_SIZE) {
			cli_out("at24c64_read error: addr + len > %d\n",
                                AT24C64_DEVICE_RW_SIZE);
			return CMD_OK;
		}
		buf = (uint8*)sal_alloc(len, "bsc");
		if (buf == NULL) {
			cli_out("No memory\n");
			return CMD_OK;
		}

		devno = soc_bsc_devopen(unit, BSC_AT24C32_NAME);

                if (devno  < 0) {
                    cli_out("%s: cannot open I2C device %s: %s\n",
                            ARG_CMD(a), BSC_AT24C32_NAME, bcm_errmsg(devno));
                    sal_free(buf);
                    return CMD_FAIL;
                }
		cli_out("0x%04x:", addr);
		rlen = len;
		retv = soc_eep24c64_read(unit, devno, addr, buf, &rlen);
		if ((rlen != len) || (retv != SOC_E_NONE)) {
			sal_free(buf);
			return CMD_OK;
		}

		for (i = 0; i < len; i++) {
			data = buf[i];
			if (i && ((i % 8) == 0)) {
				cli_out(" ");
			}
			if (i && ((i % 16) == 0)) {
                                cli_out("%s", buf_tmp);
				cli_out("\n0x%04x:", addr + i);
                                buf_tmp[0] = '\0';
			}
                        buf_tmp[i % 16] = (isprint(data)) ? buf[i] : '.';
			cli_out(" %02x", data);
		}

                if (i && (i % 16)) {
                    buf_tmp[i % 16] = '\0';
                }
                cli_out(" %s", buf_tmp);
		sal_free(buf);
		cli_out("\n");
	} else if (!sal_strncasecmp(function,"epw", sal_strlen(function))) {
		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		addr = parse_integer(tmp);
		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		devno = soc_bsc_devopen(unit, BSC_AT24C32_NAME);
                if (devno  < 0) {
                    cli_out("%s: cannot open I2C device %s: %s\n",
                            ARG_CMD(a), BSC_AT24C32_NAME, bcm_errmsg(devno));
                    return CMD_FAIL;
                }
		retv = soc_eep24c64_write(unit, devno,
					addr, (uint8 *)tmp, sal_strlen(tmp));
	} else if (!sal_strncasecmp(function,"eptest", sal_strlen(function))) {
		devno = soc_bsc_devopen(unit, BSC_AT24C32_NAME);
                if (devno  < 0) {
                    cli_out("%s: cannot open I2C device %s: %s\n",
                            ARG_CMD(a), BSC_AT24C32_NAME, bcm_errmsg(devno));
                    return CMD_FAIL;
                }
		soc_eep24c64_test(unit, devno);
	} else if (!sal_strncasecmp(function,"sfpread", sal_strlen(function))) {
                char sfp_dev_name[16];
		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		addr = parse_integer(tmp);
                sal_sprintf(sfp_dev_name, "%s%d", BSC_SFP_NAME, addr);

		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		addr = parse_integer(tmp);

		tmp = ARG_GET(a);
		if (tmp) {
			len = parse_integer(tmp);
		} else {
			len = 0x20;
		}

		if (addr + len > SFP_DEVICE_SIZE) {
			cli_out("sfp_read error: addr + len > %d\n",
                                SFP_DEVICE_SIZE);
			return CMD_OK;
		}
		buf = (uint8*)sal_alloc(len, "bsc");
		if (buf == NULL) {
			cli_out("No memory\n");
			return CMD_OK;
		}

		devno = soc_bsc_devopen(unit, sfp_dev_name);

                if (devno  < 0) {
                    cli_out("%s: cannot open I2C device %s: %s\n",
                            ARG_CMD(a), sfp_dev_name, bcm_errmsg(devno));
                    sal_free(buf);
                    return CMD_FAIL;
                }
		cli_out("0x%04x:", addr);
		rlen = len;
		retv = soc_sfp_read(unit, devno, addr, buf, &rlen);
		if ((rlen != len) || (retv != SOC_E_NONE)) {
			sal_free(buf);
			return CMD_OK;
		}

		for (i = 0; i < len; i++) {
			data = buf[i];
			if (i && ((i % 8) == 0)) {
				cli_out(" ");
			}
			if (i && ((i % 16) == 0)) {
				cli_out("\n0x%04x:", addr + i);
			}
			cli_out(" %02x", data);
		}
		sal_free(buf);
		cli_out("\n");
	} else if (!sal_strncasecmp(function,"sfpwrite", sal_strlen(function))) {
                char sfp_dev_name[16];
                uint8 db;
		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		addr = parse_integer(tmp);
                sal_sprintf(sfp_dev_name, "%s%d", BSC_SFP_NAME, addr);

		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
		addr = parse_integer(tmp);

		tmp = ARG_GET(a);
		if (!tmp) {
			return CMD_USAGE;
		}
                db = (uint8)parse_integer(tmp);
		devno = soc_bsc_devopen(unit, sfp_dev_name);
                if (devno  < 0) {
                    cli_out("%s: cannot open I2C device %s: %s\n",
                            ARG_CMD(a), sfp_dev_name, bcm_errmsg(devno));
                    return CMD_FAIL;
                }
		retv = soc_sfp_write(unit, devno,
					addr, (uint8 *)&db, 1);
	} else {
		return CMD_USAGE;
	}
	return CMD_OK;
}

/*
 * Function: cmd_sensor
 * Purpose: Sensor and Sensor monitoring commands.
 */
char cmd_sensor_usage[] =
	"\n\t"
	"sensor [show]\n\t"
	"    - show current sensor on all devices.\n\t"
	"sensor [watch|nowatch] [delay]\n\t"
	"    - monitor sensor in background, reporting changes.\n\t"
	"\n";

cmd_result_t
cmd_sensor(int unit, args_t *a)
{
	char *function = ARG_GET(a);
	char *interval = ARG_GET(a);
	uint32 delay = 5; /* Report stats every 5 seconds */

	if (!sh_check_attached(ARG_CMD(a), unit))
		return CMD_FAIL;
	if (!function || !sal_strncasecmp(function, "show", sal_strlen(function)) ){
		soc_bsc_max6653_show(unit);
	} else if (!sal_strncasecmp(function,"watch",sal_strlen(function)) ){
		if (interval) {
			delay = parse_integer(interval);
		}
		soc_bsc_max6653_watch(unit, TRUE, delay);
	} else if (!sal_strncasecmp(function,"nowatch",sal_strlen(function)) ){
		soc_bsc_max6653_watch(unit, FALSE, delay);
	} else {
		return CMD_USAGE;
	}
	return CMD_OK;
}

/*
 * Function: cmd_setfanspeed
 * Purpose: Sensor and Sensor monitoring commands.
 */
char cmd_setfanspeed_usage[] =
	"\n\t"
	"setfanspeed <devno:0-7> <speed:0-15>\n\t"
	"    - Set <devno> fan controller to <speed>\n\t"
	"\n";

cmd_result_t
cmd_setfanspeed(int unit, args_t *a)
{
	char *d = ARG_GET(a);
	char *s = ARG_GET(a);
	int devno = -1, speed = -1;
	int retv;

	if (!sh_check_attached(ARG_CMD(a), unit))
		return CMD_FAIL;

	if (d) {
		devno = parse_integer(d);
	}
	if (s) {
		speed = parse_integer(s);
	}
	if ((devno < 0) || (devno > 7) || (speed < 0) || (speed > 15)) {
		return CMD_USAGE;
	}

	retv = soc_bsc_max6653_set(unit, devno, speed);
	if (retv) {
		sal_printf("%s failed\n", FUNCTION_NAME());
		return CMD_FAIL;
	}

	return CMD_OK;
}

/*
 * Digital Power Manager
 *
 * Function: cmd_dpm
 * Purpose: show and set POL (point of load) voltages
 */
char cmd_dpm_usage[] =
	"\n\t"
	"dpm show\n\t"
	"    - Show voltage on all availabe POLs\n\t"
	"dpm [watch|nowatch] [delay]\n\t"
	"    - monitor dpm in background, reporting changes.\n\t"
	"dpm setpol <pol:0-31> <volt:0.5-5.5>\n\t"
	"    - Set <pol> voltage to <volt>\n\t"
	"\n";

cmd_result_t
cmd_dpm(int unit, args_t *a)
{
	char *function = ARG_GET(a);
	char *interval, *p, *v;
	uint32 delay = 5;	/* Report stats every 5 seconds */
	int pol = -1;
	float volt = -1;

	if (!sh_check_attached(ARG_CMD(a), unit))
		return CMD_FAIL;

	if (!function) {
		return CMD_USAGE;
	} else if (!sal_strncasecmp(function, "show", sal_strlen(function))) {
		soc_bsc_zm73xx_show(unit);
	} else if (!sal_strncasecmp(function,"watch",sal_strlen(function)) ){
		interval = ARG_GET(a);
		if (interval) {
			delay = parse_integer(interval);
		}
		soc_bsc_zm73xx_watch(unit, TRUE, delay);
	} else if (!sal_strncasecmp(function,"nowatch",sal_strlen(function)) ){
		soc_bsc_zm73xx_watch(unit, FALSE, delay);
	} else if (!sal_strncasecmp(function,"setpol",sal_strlen(function))) {
		p = ARG_GET(a);
		v = ARG_GET(a);
		if (p) {
			pol = parse_integer(p);
		}
		if (v) {
            /* coverity[secure_coding] */
			sscanf(v, "%f", &volt);
		}

		if ((pol < 0) || (pol > 31) || (volt < 0.5) || (volt > 5.5)) {
			return CMD_USAGE;
		}
		soc_bsc_zm73xx_set(unit, pol, volt);
	} else {
		return CMD_USAGE;
	}

	return CMD_OK;
}

#else
int _bsc_cmds_bcm_fe2000_support_not_empty;
#endif /* BCM_FE2000_SUPPORT */
#else
int _bsc_cmds_include_i2c_not_empty;
#endif /* INCLUDE_I2C */
#else
int _bsc_cmds_no_sal_appl_not_empty;
#endif /* NO_SAL_APPL */

