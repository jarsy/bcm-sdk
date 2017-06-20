/*
 * $Id: cpu_i2c_cmds.c,v 1.4 Broadcom SDK $
 *
 * I2C specific commands for use in CPU driven mode.
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

int _cpu_i2c_cmds_not_empty;

#ifndef NO_SAL_APPL

#ifdef INCLUDE_I2C
#ifdef BCM_CALADAN3_SVK

#include <sal/core/libc.h>
#include <sal/core/boot.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/appl/config.h>
#include <shared/bsl.h>
#include <soc/cmext.h>
#include <soc/drv.h>
#include <soc/debug.h>
#include <soc/i2c.h>
#include <soc/mem.h>

#include <bcm/error.h>
#include <bcm/init.h>
#include <bcm/error.h>

#include <appl/diag/system.h>

extern char *
get_bits(uint8 byte);

/*
 * Function: cmd_cpu_i2c
 * Purpose: I2C probe/attach/show, configuration commands.
 *          Also sets up board index and configures I2C drivers
 *          based on the inferred system board type.
 */
char cmd_cpu_i2c_usage[] =
    "Usages:\n\t"
#ifdef COMPILER_STRING_CONST_LIMIT
    "cpui2c probe | reset | speeds | show\n"
#else
    "cpui2c probe [bus=<bus>] [saddr=<addr>] [pio|intr] [quiet]\n\t"
    "    - probe devices on I2C bus and build device tree.\n\t"
    "      If \"intr\" or \"pio\" is specified, change to that bus mode.\n\t"
    "      If a valid speed is specified, change the bus to that speed.\n\t"
    "      If \"quiet\" is specified, suppresses probe output.\n\t"
    "cpui2c scan [bus=<bus>] [saddr=<saddr>] [pio|intr] [quiet]\n\t"
    "    - Scan devices on I2C bus and display the device list.\n\t"
    "cpui2c show\n\t"
    "    - show devices found and their attributes.\n\t"
    "cpui2c read saddr=<saddr> comm=<addr> len=<nbytes> \n\t"
    "    - generic interface to read devices \n\t"
    "cpui2c readb saddr=<saddr> len=<nbytes> \n\t"
    "    - generic interface to read devices without register based access\n\t"
    "cpui2c write saddr=<saddr> comm=<addr> [data=<byte>] \n\t"
    "    - generic interface to write a byte to devices \n"
#endif
    ;


cmd_result_t
cmd_cpu_i2c(int unit, args_t *a)
{
    uint32 flags = 0; 
    int	rv, quiet=0, speed = -1, pio=0, intr=0, saddr=0;
    int bus = 1;
    int show=0, reset=0, scan=0, probe=0, freq=0;
    int length = 0, data = -1, command = -1;
    int i, readb = 0, read=0, write = 0;
    uint8 buffer[1024];
    uint8 len = 0;

    if (ARG_CNT(a)) {
        int ret_code;
        parse_table_t pt;
        parse_table_init(0, &pt);

        parse_table_add(&pt, "show", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &show, NULL);
        parse_table_add(&pt, "reset", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &reset, NULL);
        parse_table_add(&pt, "scan", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &scan, NULL);
        parse_table_add(&pt, "bus", PQ_DFL | PQ_INT ,
                        0, &bus, NULL);
        parse_table_add(&pt, "quiet", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &quiet, NULL);
        parse_table_add(&pt, "probe", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &probe, NULL);
        parse_table_add(&pt, "speeds", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &freq, NULL);
        parse_table_add(&pt, "write", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &write, NULL);
        parse_table_add(&pt, "read", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &read, NULL);
        parse_table_add(&pt, "readb", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &readb, NULL);
        parse_table_add(&pt, "saddr", PQ_DFL | PQ_INT ,
                        0, &saddr, NULL);
        parse_table_add(&pt, "comm", PQ_DFL | PQ_INT ,
                        0, &command, NULL);
        parse_table_add(&pt, "data", PQ_DFL | PQ_INT ,
                        0, &data, NULL);
        parse_table_add(&pt, "len", PQ_DFL | PQ_INT ,
                        0, &length, NULL);
        parse_table_add(&pt, "pio", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &pio, NULL);
        parse_table_add(&pt, "intr", PQ_DFL | PQ_BOOL | PQ_NO_EQ_OPT,
                        0, &intr, NULL);

        if (!parseEndOk(a, &pt, &ret_code)) {
            return ret_code;
        }
    }
    
    if (length > 256) {
        cli_out("Invalid len %d specified\n", len);
        return CMD_FAIL;
    } else if (length == 0) {
        len = 1;
    } else {
        len = length; /* parsing expects int */
    }
    if (read) {
        if (saddr == 0) { 
            cli_out("\nNeed Slave address ");
            return CMD_FAIL; 
        } 
        if (command == -1) {
            cli_out("\nNeed base register <comm>");
            return CMD_FAIL; 
        }
        if (len == 1) {
            rv = cpu_i2c_read(saddr, command, 
                               CPU_I2C_ALEN_BYTE_DLEN_BYTE, &data);
            if (rv==0) {
                cli_out("%X \n", data);
            }
        } else {
            memset(buffer, 0, sizeof(buffer));
            rv = cpu_i2c_block_read(bus, saddr, command, 
                       buffer, &len);
            if (rv==0) {
                for (i=0; i<len; i++) {
                    if (i%8==0) {
                        cli_out("\n%02X: ", i);
                    }
                    cli_out("%02x ", buffer[i]);
                }
                cli_out("\n");
            }
        }
    } else if (readb) {
        if (saddr == 0) { 
            cli_out("\nNeed Slave address ");
            return CMD_FAIL; 
        } 
        if (len == 1) {
            rv = cpu_i2c_read(saddr, 0, CPU_I2C_ALEN_NONE_DLEN_BYTE, &data);
            if (rv==0) {
                cli_out("%X ", data);
            }
        } else {
            memset(buffer, 0, sizeof(buffer));
            rv = cpu_i2c_block_read(bus, saddr, 0, 
                        buffer, &len);
            for (i=0; i<len; i++) {
                if (i%8==0) {
                    cli_out("\n%02X: ", i);
                }
                cli_out("%02x ", buffer[i]);
            }
            cli_out("\n");
        }
        
    } else if (write) {
        if (saddr == 0) { 
            cli_out("\nNeed Slave address ");
            return CMD_FAIL; 
        }
        if (data == -1) {
            rv = cpu_i2c_write(saddr, 0, 
                            CPU_I2C_ALEN_NONE_DLEN_BYTE, command);
        } else { 
            /* write command + byte */
            rv = cpu_i2c_write(saddr, command, 
                            CPU_I2C_ALEN_BYTE_DLEN_BYTE, data);
        }
    } else if (show) {
    	cli_out("cpui2c -> show!!!\n\r");
	cpu_i2c_show(bus);
    } else if (reset) {
    	cli_out("cpui2c -> reset!!!\n\r");
	/*cpu_i2c_reset(bus);*/
    } else if (freq) {
    	cli_out("cpui2c -> freq!!!\n\r");
    	rv = CMD_OK;
	/*cpu_i2c_show_speeds(bus);*/
    } else if ( probe || scan) {

        if (pio && intr) {
           cli_out("Error: cannot set both pio and intr\n");
           return CMD_FAIL;		 
         
        }
        if (pio) {
           flags = SOC_I2C_MODE_PIO;
	} else if (intr) {
           flags = SOC_I2C_MODE_INTR;
        }

        if (probe) {
            /*
             * (Re-)attach to the I2C bus if the INTR/PIO mode or
             * bus speed has been specified (presumably new setting).
             */
            if ( ((flags != 0) || (speed > 0)) &&
                 ((rv=cpu_i2c_attach(bus, flags, speed)) < 0) ) {
                cli_out("%s: Error: attach failed: %s\n",
                        ARG_CMD(a), bcm_errmsg(rv));
            }

            rv = cpu_i2c_probe(bus); /* Will attach if not yet attached */
            if (rv < 0) {
                cli_out("%s: Error: probe failed: %s\n",
                        ARG_CMD(a), bcm_errmsg(rv));
            } else if (!quiet) {
                cli_out("%s: detected %d device%s\n",
                        ARG_CMD(a), rv, rv==1 ? "" : "s");
            }
        } else {
            i2c_saddr_t saddr = 0x00;
            int saddr_start = I2C_7BIT_START;
            int saddr_end = I2C_7BIT_END;

            /*
             * (Re-)attach to the I2C bus if the INTR/PIO mode or
             * bus speed has been specified (presumably new setting).
             */
            if ( (flags != 0) &&
                 ((rv=cpu_i2c_attach(bus, flags, -1)) < 0) ) {
                cli_out("%s: ERROR: attach failed: %s\n",
                        ARG_CMD(a), bcm_errmsg(rv));
            }

            if  (speed > 0) {
                saddr_start = saddr_end = speed & 0x7F;
            }
            for(;saddr_start <= saddr_end; saddr_start++) {
                saddr = saddr_start & 0x7F;
                if (cpu_i2c_device_present(bus, saddr) >= 0) {
                    cli_out("I2C device found at slave address 0X%02X (%s)\n",
                            saddr_start, cpu_i2c_saddr_to_string(bus, saddr));
                }
            }
        }
    }
    else {
	return CMD_USAGE;
    }
    return CMD_OK;
}

cmd_result_t cpu_i2c_muxsel(int unit, args_t *a)
{
    int saddr, rv = SOC_E_NONE;    
    CPU_I2C_BUS_LEN len;
    int lptVal;
    int mux COMPILER_ATTRIBUTE((unused)), type;
    char * dev_name = ARG_GET(a);
    char *channel = ARG_GET(a);

    len = CPU_I2C_ALEN_BYTE_DLEN_BYTE;
    if ((dev_name == NULL)  ||
        (strncmp(dev_name, "mux", 3) != 0)) {
        cli_out("ERROR: Valid devices are mux0 - mux6 \n");
        return CMD_FAIL;
    }
    if (strncmp(dev_name, I2C_MUX_0, 4) == 0) {
        saddr = I2C_MUX_SADDR0;
        mux = 0;
        type = PCA9548_DEVICE_TYPE;
    } else if (strncmp(dev_name, I2C_MUX_1, 4) == 0) {
        saddr = I2C_MUX_SADDR1;
        mux = 1;
        type = PCA9548_DEVICE_TYPE;
    } else if (strncmp(dev_name, I2C_MUX_2, 4) == 0) {
        saddr = I2C_MUX_SADDR2;
        mux = 2;
        type = PCA9548_DEVICE_TYPE;
    } else if (strncmp(dev_name, I2C_MUX_3, 4) == 0) {
        saddr = I2C_MUX_SADDR3;
        mux = 3;
        type = PCA9548_DEVICE_TYPE;
    } else if (strncmp(dev_name, I2C_MUX_4, 4) == 0) {
        saddr = I2C_MUX_SADDR4;
        mux = 4;
        type = PCA9548_DEVICE_TYPE;
    } else if (strncmp(dev_name, I2C_MUX_5, 4) == 0) {
        saddr = I2C_MUX_SADDR5;
        mux = 5;
        type = PCA9548_DEVICE_TYPE;
    } else if (strncmp(dev_name, I2C_MUX_6, 4) == 0) {
        saddr = I2C_MUX_SADDR6;
        mux = 6;
        type = PCA9548_DEVICE_TYPE;
    } else {
        cli_out("ERROR: Valid devices are I2C_MUX_0 - I2C_MUX_6 \n");
        return CMD_FAIL;
    }
    if (! channel ){
        lptVal = 0;
	if( (rv = cpu_i2c_read(saddr, 0, len, &lptVal)) < 0){
	    cli_out("ERROR: read byte failed: %s\n",
                    bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	cli_out("Read I2C MuxSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    } else {
	/* Write LPT bits */
	lptVal = (uint8)parse_integer(channel);
        if (lptVal < 0 || lptVal > 8) {
	    cli_out("ERROR: Valid Channels are 1-8 (0 to turn off) \n");
            return CMD_FAIL;
        }
        if (lptVal > 0) {
            if (type == PCA9548_DEVICE_TYPE) {
                /* Handle mux and switch, zero based */
                lptVal =  1 << (lptVal -1);
            } else {
                lptVal =  (lptVal-1);
            }
        }
	if( (rv = cpu_i2c_write(saddr, 0, len, lptVal)) < 0) {
	    cli_out("ERROR: write byte failed: %s\n",
                    bcm_errmsg(rv));
	    return CMD_FAIL;
	}
	if (lptVal)  {
	    cli_out("Selecting I2C Channel %s\n", channel);
        } else {
	    cli_out("Disabling all channels on %s\n", dev_name);
        }
	cli_out("Write I2C MuxSel = 0x%x (%s)\n", lptVal, get_bits(lptVal));
    }
    return CMD_OK;
}

#endif /* BCM_CALADAN3_SVK */

#endif /* INCLUDE_I2C */

#endif /* NO_SAL_APPL */

