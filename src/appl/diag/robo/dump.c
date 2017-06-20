/*
 * $Id: dump.c,v 1.36 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag dump command
 */

#include <shared/bsl.h>

#include <sal/appl/pci.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/phy.h>
#include <soc/phyctrl.h>

#include <bcm/l2.h>
#include <bcm/pkt.h>
#include <bcm_int/robo/rx.h>

#include <appl/diag/system.h>
#include <appl/diag/sysconf.h>
#include <appl/diag/bslcons.h>
#include <appl/diag/bslfile.h>

#include <ibde.h>

#define DUMP_PHY_COLS	4
#define DUMP_MW_COLS	4
#define DUMP_MH_COLS	8
#define DUMP_MB_COLS	16

/*
 * Dump all of the CMIC registers.
 */
/*
static void
do_dump_pcim(int unit, uint32 off_start, uint32 count)
{
    uint32 off, val;

    if ((off_start & 3) != 0) {
    	cli_out("dump_pcim ERROR: offset must be a multiple of 4\n");
    	return;
    }

    for (off = off_start; count--; off += 4) {
       	val = soc_pci_read(unit, off);
        cli_out("0x%04x %s: 0x%x\n", off, soc_pci_off2name(unit, off), val);
    }
}
*/


/*
 * Dump all of the SOC register addresses, and if do_values is true,
 * read and dump their values along with the addresses.
 */

#define DREG_ADR_SEL_MASK     0xf       /* Low order 4 bits */
#define DREG_ADDR               0       /* Address only */
#define DREG_RVAL               1       /* Address and reset default value */
#define DREG_REGS               2       /* Address and real value */
#define DREG_DIFF               3       /* Addr & real value if != default */

#define DREG_PORT_ALL -1
#define DREG_BLOCK_ALL -1

struct dreg_data {
    int unit;
    int dreg_select;
    int only_port;    /* Select which port/block.  -1 ==> all */
};

static int
dreg(int unit, soc_regaddrinfo_t *ainfo, void *data)
{
    struct dreg_data *dd = data;
    uint32 value = 0;
    uint64 val64, resetVal;
    char name[80];
    int is_default, rv;
    int nBytes;
    int no_match = FALSE;  /* If specific port/block requested, turns true */
    char rval_str[20];

    /* Filter (set no_match) on ports and blocks if selected. */
    if (dd->only_port != DREG_PORT_ALL) {
        /* Only print ports that match */
        if (ainfo->port != dd->only_port) {
            no_match = TRUE;
        }
    }

    if (no_match) {
        return 0;
    }

    soc_robo_reg_sprint_addr(unit, name, ainfo);

    if (dd->dreg_select == DREG_ADDR) {
        cli_out("0x%08x %s\n", ainfo->addr, name);
        return 0;
    }

    SOC_REG_RST_VAL_GET(unit, ainfo->reg, resetVal);
    format_uint64(rval_str, resetVal);

    if (dd->dreg_select == DREG_RVAL) {
        cli_out("0x%08x %s = 0x%s\n", ainfo->addr, name, rval_str);
        return 0;
    }

    if (SOC_REG_INFO(unit, ainfo->reg).flags & SOC_REG_FLAG_WO) {
        cli_out("0x%08x %s = Write Only\n", ainfo->addr, name);
        return 0;
    }

    if (SOC_REG_IS_SPECIAL(unit, ainfo->reg)) {
        cli_out("0x%08x %s = Requires special processing\n",
                ainfo->addr, name);
        return 0;
    }

    nBytes =  (DRV_SERVICES(unit)->reg_length_get)(unit, ainfo->reg);

    if (nBytes > 4) {
        rv = soc_robo_anyreg_read(unit, ainfo, &val64);
        is_default = COMPILER_64_EQ(val64, resetVal);
    } else {
        rv = soc_robo_anyreg_read(unit, ainfo, &val64);
        value = COMPILER_64_LO(val64);
        is_default = (value == COMPILER_64_LO(resetVal));
    }

    if (rv < 0) {
        cli_out("0x%08x %s = ERROR\n", ainfo->addr, name);
        return 0;
    }

    if (dd->dreg_select == DREG_DIFF && is_default) {
        return 0;
    }

    if (nBytes > 4) {
        cli_out("0x%08x %s = 0x%08x%08x\n",
                ainfo->addr, name,
                COMPILER_64_HI(val64), COMPILER_64_LO(val64));
    } else {
        cli_out("0x%08x %s = 0x%08x\n", ainfo->addr, name, value);
    }

    return 0;
}

static cmd_result_t
do_dump_soc(int unit, int dreg_select, int only_port)
{
    struct dreg_data dd;

    dd.unit = unit;
    dd.dreg_select = dreg_select;
    dd.only_port = only_port;

    (void) soc_robo_reg_iterate(unit, dreg, &dd);

    return CMD_OK;
}


#define ENABLE_PCI_PRINT_CONFIG 0

#if ENABLE_PCI_PRINT_CONFIG
static void   
_pci_print_config(int dev)   
{   
    uint32              data;   
    
    data = bde->pci_conf_read(dev, PCI_CONF_VENDOR_ID);   
    cli_out("%04x: %08x  DeviceID=%04x  VendorID=%04x\n",   
            PCI_CONF_VENDOR_ID, data,   
            (data & 0xffff0000) >> 16,   
            (data & 0x0000ffff) >>  0);   
    
    data = bde->pci_conf_read(dev, PCI_CONF_COMMAND);   
    cli_out("%04x: %08x  Status=%04x  Command=%04x\n",   
            PCI_CONF_COMMAND, data,   
            (data & 0xffff0000) >> 16,   
            (data & 0x0000ffff) >>  0);   
     
    data = bde->pci_conf_read(dev, PCI_CONF_REVISION_ID);   
    cli_out("%04x: %08x  ClassCode=%06x  RevisionID=%02x\n",    
            PCI_CONF_REVISION_ID, data,   
            (data & 0xffffff00) >> 8,   
            (data & 0x000000ff) >> 0);   
   
    data = bde->pci_conf_read(dev, PCI_CONF_CACHE_LINE_SIZE);   
    cli_out("%04x: %08x  BIST=%02x  HeaderType=%02x  "   
            "LatencyTimer=%02x  CacheLineSize=%02x\n",   
            PCI_CONF_CACHE_LINE_SIZE, data,   
            (data & 0xff000000) >> 24,   
            (data & 0x00ff0000) >> 16,   
            (data & 0x0000ff00) >>  8,   
            (data & 0x000000ff) >>  0);   
   
    data = bde->pci_conf_read(dev, PCI_CONF_BAR0);   
    cli_out("%04x: %08x  BaseAddress0=%08x\n",   
            PCI_CONF_BAR0, data, data);   
     
    data = bde->pci_conf_read(dev, PCI_CONF_BAR1);   
    cli_out("%04x: %08x  BaseAddress1=%08x\n",   
            PCI_CONF_BAR1, data, data);   
     
    data = bde->pci_conf_read(dev, PCI_CONF_BAR2);   
    cli_out("%04x: %08x  BaseAddress2=%08x\n",   
            PCI_CONF_BAR2, data, data);   
     
    data = bde->pci_conf_read(dev, PCI_CONF_BAR3);   
    cli_out("%04x: %08x  BaseAddress3=%08x\n",   
            PCI_CONF_BAR3, data, data);   
   
    data = bde->pci_conf_read(dev, PCI_CONF_BAR4);   
    cli_out("%04x: %08x  BaseAddress4=%08x\n",   
            PCI_CONF_BAR4, data, data);   
   
    data = bde->pci_conf_read(dev, PCI_CONF_BAR5);   
    cli_out("%04x: %08x  BaseAddress5=%08x\n",   
            PCI_CONF_BAR5, data, data);   
   
    data = bde->pci_conf_read(dev, PCI_CONF_CB_CIS_PTR);   
    cli_out("%04x: %08x  CardbusCISPointer=%08x\n",   
            PCI_CONF_CB_CIS_PTR, data, data);   
    
    data = bde->pci_conf_read(dev, PCI_CONF_SUBSYS_VENDOR_ID);   
    cli_out("%04x: %08x  SubsystemID=%02x  SubsystemVendorID=%02x\n",   
            PCI_CONF_SUBSYS_VENDOR_ID, data,   
            (data & 0xffff0000) >> 16,   
            (data & 0x0000ffff) >>  0);   
    
    data = bde->pci_conf_read(dev, PCI_CONF_EXP_ROM);   
    cli_out("%04x: %08x  ExpansionROMBaseAddress=%08x\n",   
            PCI_CONF_EXP_ROM, data, data);   
    
    data = bde->pci_conf_read(dev, 0x34);   
    cli_out("%04x: %08x  Reserved=%06x  CapabilitiesPointer=%02x\n",   
            0x34, data,   
            (data & 0xffffff00) >> 8,   
            (data & 0x000000ff) >> 0);   
    
    data = bde->pci_conf_read(dev, 0x38);   
    cli_out("%04x: %08x  Reserved=%08x\n",   
            0x38, data, data);   
      
    data = bde->pci_conf_read(dev, PCI_CONF_INTERRUPT_LINE);   
    cli_out("%04x: %08x  Max_Lat=%02x  Min_Gnt=%02x  "   
            "InterruptPin=%02x  InterruptLine=%02x\n",   
            PCI_CONF_INTERRUPT_LINE, data,   
            (data & 0xff000000) >> 24,   
            (data & 0x00ff0000) >> 16,   
            (data & 0x0000ff00) >>  8,   
            (data & 0x000000ff) >>  0);   
    
    data = bde->pci_conf_read(dev, 0x40);   
    cli_out("%04x: %08x  Reserved=%02x  "   
            "RetryTimeoutValue=%02x  TRDYTimeoutValue=%02x\n",   
            0x40, data,   
            (data & 0xffff0000) >> 16,   
            (data & 0x0000ff00) >>  8,   
            (data & 0x000000ff) >>  0);   
    
#ifdef VXWORKS   
       
    {   
#ifdef IDTRP334   
    extern void sysBusErrDisable(void);   
    extern void sysBusErrEnable(void);   
#endif   
    
    /* HINT (R) HB4 PCI-PCI Bridge (21150 clone) */   
#define HINT_HB4_VENDOR_ID    0x3388   
#define HINT_HB4_DEVICE_ID    0x0022   
    
    int BusNo, DevNo, FuncNo;   
    unsigned short tmp;    
    
#ifdef IDTRP334   
    sysBusErrDisable();   
#endif   
    
    /*    
     * HINTCORP HB4 PCI-PCI Bridge   
     */   
    if (pciFindDevice(HINT_HB4_VENDOR_ID,   
              HINT_HB4_DEVICE_ID,   
              0,   
              &BusNo, &DevNo, &FuncNo) != ERROR) {   
    
        cli_out("-------------------------------------\n");   
        cli_out("HB4 PCI-PCI Bridge Status Registers  \n");   
        cli_out("-------------------------------------\n");         
    
        /* Dump the status registers */   
        pciConfigInWord(BusNo,DevNo,FuncNo, 0x06, &tmp);       
        cli_out("Primary Status (%xh):   0x%x\n", 0x06, tmp);   
        pciConfigInWord(BusNo,DevNo,FuncNo, 0x1e, &tmp);       
        cli_out("Secondary Status (%xh): 0x%x\n", 0x1e, tmp);   
        pciConfigInWord(BusNo,DevNo,FuncNo, 0x3e, &tmp);   
        cli_out("Bridge Control (%xh):   0x%x\n", 0x3e, tmp);   
        pciConfigInWord(BusNo,DevNo,FuncNo, 0x6a, &tmp);   
        cli_out("P_SERR Status (%xh):    0x%x\n", 0x6a, tmp);   
    }   
    
#ifdef IDTRP334   
    sysBusErrEnable();   
#endif   
    }   
#endif   
}   
#endif 

/*
 * Dump registers, tables, or an address space.
 */

static cmd_result_t
do_dump_registers(int unit, regtype_entry_t *rt, args_t *a)
{
    int i;
    sal_vaddr_t vaddr;
    uint32 t2;
    pbmp_t pbmp;
    soc_port_t port;
    int rv = CMD_OK;
    uint32 flags = DREG_REGS;
    int dump_port = DREG_PORT_ALL;
    char *an_arg;
    char *count;

    an_arg = ARG_GET(a);
    count = ARG_GET(a);

    /* PCI config space does not take an offset */
    switch (rt->type) {
        
        /*
        case soc_pci_cfg_reg:
            _pci_print_config(unit);
            break;
        case soc_cpureg:
            if (an_arg) {
                if (parse_cmic_regname(unit, an_arg, &t1) < 0) {
                    cli_out(\
                            "ERROR: unknown CMIC register name: %s\n", \
                            an_arg);
                    rv = CMD_FAIL;
                    goto done;
                }
                t2 = count ? parse_integer(count) : 1;
            } else {
                t1 = CMIC_OFFSET_MIN;
                t2 = (CMIC_OFFSET_MAX - CMIC_OFFSET_MIN) / 4 + 1;
            }
            do_dump_pcim(unit, t1, t2);
            break;
        */
        case soc_schan_reg:
        case soc_spi_reg:
        case soc_genreg:
        case soc_portreg:
        case soc_cosreg:
            while (an_arg) {
                if (sal_strcasecmp(an_arg, "addr") == 0) {
                    flags = DREG_ADDR;
                } else if (sal_strcasecmp(an_arg, "rval") == 0) {
                    flags = DREG_RVAL;
                } else if (sal_strcasecmp(an_arg, "diff") == 0) {
                    flags = DREG_DIFF;
                } else if (sal_strcasecmp(an_arg, "port") == 0) {
                    if (count != NULL) {
                    	dump_port = parse_integer(count);
                    	cli_out("SOC_ROBO_MAX_NUM_PORTS %d\n",SOC_ROBO_MAX_NUM_PORTS);
                        if (dump_port >= SOC_ROBO_MAX_NUM_PORTS) {
                        	cli_out("ERROR: Exceed max port number.\n");
                        	return CMD_FAIL;
                        }
                  	} else {
                  	    cli_out("ERROR: port number needed.\n");
                  	    return CMD_FAIL;
                  	}
                } else {
                    cli_out("ERROR: unrecognized argument to DUMP SOC: %s\n",
                            an_arg);
                    return CMD_FAIL;
                }
                an_arg = ARG_GET(a);
            }
            rv = do_dump_soc(unit, flags, dump_port);
            break;
        case soc_phy_reg:
            if (an_arg) {
                if (parse_pbmp(unit, an_arg, &pbmp)) {
                    cli_out("Error: Invalid port identifier: %s\n", an_arg);
                    rv = CMD_FAIL;
                    break;
                }
            } else {
                pbmp = PBMP_ALL(unit);
            }
            SOC_PBMP_AND(pbmp, PBMP_ALL(unit));
            PBMP_ITER(pbmp, port) {
                uint8	phy_id = PORT_TO_PHY_ADDR(unit, port);
                uint16	phy_data, phy_reg;
                cli_out("\nPort %d (Phy ID %d)", port, phy_id);
                for (phy_reg = PHY_MIN_REG; 
                     phy_reg <= PHY_MAX_REG; phy_reg++) {
                    rv = soc_robo_miim_read
                                      (unit, phy_id, phy_reg, &phy_data);
                    if (rv < 0) {
                        cli_out("Error: Port %d: cmic_read_miim failed: %s\n",
                                port, soc_errmsg(rv));
                        rv = CMD_FAIL;
                        goto done;
                    }
                    cli_out("%s\t0x%02x: 0x%04x", 
                            ((phy_reg % DUMP_PHY_COLS) == 0) ? "\n" : "", 
                            phy_reg, phy_data);
                }
                cli_out("\n");
            }
            break;
    
        case soc_hostmem_w:
            if (!an_arg) {
                cli_out("Dumping memory requires address and optional count\n");
                rv = CMD_FAIL;
                goto done;
            }

            vaddr = parse_address(an_arg) & ~3;
            t2 = count ? parse_integer(count) : 1;
            for (i = 0; i < (int)t2; i++, vaddr += 4) {
                uint32 *memptr = INT_TO_PTR(vaddr);
                if ((i % DUMP_MW_COLS) == 0) {
                    cli_out("%p: ", (void *)memptr);
                }
                cli_out("%08x%c", *memptr,
                        ((i + 1) % DUMP_MW_COLS) == 0 ? '\n' : ' ');
            }
            if (i % DUMP_MW_COLS) {
                cli_out("\n");
            }
            break;
        case soc_hostmem_h:
            if (!an_arg) {
                cli_out("Dumping memory requires address and optional count\n");
                rv = CMD_FAIL;
                goto done;
            }
            vaddr = parse_address(an_arg) & ~1;
            t2 = count ? parse_integer(count) : 1;
            for (i = 0; i < (int)t2; i++, vaddr += 2) {
                uint16 *memptr = INT_TO_PTR(vaddr);
                if ((i % DUMP_MH_COLS) == 0) {
                    cli_out("%p: ", (void *)memptr);
                }
                cli_out("%04x%c", *memptr,
                        ((i + 1) % DUMP_MH_COLS) == 0 ? '\n' : ' ');
            }
            if (i % DUMP_MH_COLS) {
                cli_out("\n");
            }
            break;
        case soc_hostmem_b:
            if (!an_arg) {
                cli_out("Dumping memory requires address and optional count\n");
                rv = CMD_FAIL;
                goto done;
            }
            vaddr = parse_address(an_arg);
            t2 = count ? parse_integer(count) : 1;
            for (i = 0; i < (int)t2; i++, vaddr += 1) {
                uint8 *memptr = INT_TO_PTR(vaddr);
                if ((i % DUMP_MB_COLS) == 0) {
                    cli_out("%p: ", (void *)memptr);
                }
                cli_out("%02x%c", *memptr,
                        ((i + 1) % DUMP_MB_COLS) == 0 ? '\n' : ' ');
            }
            if (i % DUMP_MB_COLS) {
                cli_out("\n");
            }
            break;
        default:
            cli_out("Dumping register type %s is not yet implemented.\n",
                    rt->name);
            rv = CMD_FAIL;
            break;
    }

 done:
    return rv;
}

#define DUMP_TABLE_RAW          0x01
#define DUMP_TABLE_HEX          0x02
#define DUMP_TABLE_ALL          0x04

static cmd_result_t
_do_dump_cfp_table(int unit, soc_mem_t mem,
                        int index, int count, int flags)
{
    int idx, j = 0;
    drv_cfp_entry_t cfp_entry;
    int entry_len;
    int retval = CMD_FAIL;
    uint32 tbl_name, stat_name = 0, stat_counter = 0;
    uint32  *data_ptr, *mask_ptr = NULL;

    entry_len = soc_mem_entry_words(unit, mem);

    sal_memset(&cfp_entry, 0, sizeof(drv_cfp_entry_t));
    switch(mem) {
        case INDEX(CFP_ACT_POLm):
    		tbl_name = DRV_CFP_RAM_ACT;
    		data_ptr= cfp_entry.act_data;
    		break;
        case INDEX(CFP_METERm):
    		tbl_name = DRV_CFP_RAM_METER;
    		data_ptr= cfp_entry.meter_data;
    		break;
        case INDEX(CFP_STAT_IBm):
        case INDEX(CFP_GREEN_STATm):
    		tbl_name = DRV_CFP_RAM_STAT_IB;
    		stat_name = DRV_CFP_STAT_INBAND;
    		data_ptr= &stat_counter;
    		break;
        case INDEX(CFP_STAT_OBm):
        case INDEX(CFP_RED_STATm):
    		tbl_name = DRV_CFP_RAM_STAT_OB;
    		stat_name = DRV_CFP_STAT_OUTBAND;
    		data_ptr= &stat_counter;
    		break;
        case INDEX(CFP_YELLOW_STATm):
            tbl_name = DRV_CFP_RAM_STAT_YELLOW;
    		stat_name = DRV_CFP_STAT_YELLOW;
    		data_ptr= &stat_counter;
            break;
        case INDEX(CFP_STATm):
            tbl_name = DRV_CFP_RAM_STAT_IB;
            stat_name = DRV_CFP_STAT_ALL;
            data_ptr= &stat_counter;
            break;
        case INDEX(CFP_DATA_MASKm):            
        case INDEX(CFP_TCAM_IPV4_S0m):            
        case INDEX(CFP_TCAM_IPV4_S1m):            
        case INDEX(CFP_TCAM_IPV4_S2m):                        
        case INDEX(CFP_TCAM_IPV6_S0m):            
        case INDEX(CFP_TCAM_IPV6_S1m):            
        case INDEX(CFP_TCAM_IPV6_S2m):                        
            tbl_name = DRV_CFP_RAM_TCAM;
            data_ptr= cfp_entry.tcam_data;
            mask_ptr= cfp_entry.tcam_mask;
            break;
    	case INDEX(CFP_TCAM_S0m):
    	case INDEX(CFP_TCAM_S1m):
    	case INDEX(CFP_TCAM_S2m):
        case INDEX(CFP_TCAM_SCm):
    		tbl_name = DRV_CFP_RAM_TCAM;
    		data_ptr= cfp_entry.tcam_data;
    		mask_ptr= cfp_entry.tcam_mask;
    		break;
        case INDEX(CFP_TCAM_CHAIN_MASKm):
        case INDEX(CFP_TCAM_CHAIN_SCm): 
        case INDEX(CFP_TCAM_IPV4_MASKm):
        case INDEX(CFP_TCAM_IPV4_SCm): 
        case INDEX(CFP_TCAM_IPV6_MASKm):
        case INDEX(CFP_TCAM_IPV6_SCm): 
        case INDEX(CFP_TCAM_NONIP_MASKm):
        case INDEX(CFP_TCAM_NONIP_SCm): 
              tbl_name = DRV_CFP_RAM_TCAM;
    		data_ptr= cfp_entry.tcam_data;
    		mask_ptr= cfp_entry.tcam_mask;
              break;
		default:
		    cli_out(\
                            "Unsupport CFP memory table.\n");\
		    return CMD_FAIL;
    }


    for (idx = index; idx < index + count; idx++) {
        if ((tbl_name == DRV_CFP_RAM_STAT_IB) ||
            (tbl_name == DRV_CFP_RAM_STAT_OB) ||
            (tbl_name == DRV_CFP_RAM_STAT_YELLOW)){
            if ((retval = DRV_CFP_STAT_GET
            	                   (unit, stat_name, idx, &stat_counter)) < 0) {
                cli_out("Read ERROR: table %s[%d]: %s\n",
                        SOC_ROBO_MEM_UFNAME(unit, mem),
                        idx, soc_errmsg(retval));
                goto done;
            }
        }else {
            if ((retval = DRV_CFP_ENTRY_READ
            	                   (unit, idx, tbl_name, &cfp_entry)) < 0) {
                cli_out("Read ERROR: table %s[%d]: %s\n",
                        SOC_ROBO_MEM_UFNAME(unit, mem),
                        idx, soc_errmsg(retval));
                goto done;
            }
        }

        if (flags & DUMP_TABLE_HEX) {
            for (j = 0; j < entry_len; j++) {
                cli_out("%08x\n", data_ptr[j]);
            }
            if (tbl_name == DRV_CFP_RAM_TCAM) {
                if (mask_ptr != NULL ) {
                    cli_out("\n");            
                    for (j = 0; j < entry_len; j++) {
                        cli_out("%08x\n", mask_ptr[j]);
                    }
                }
            }
        } else {
    		cli_out("%s[%d]: ", SOC_ROBO_MEM_UFNAME(unit, mem), idx);
            if (flags & DUMP_TABLE_RAW) {
                for (j = 0; j < entry_len; j++) {
                    cli_out("0x%08x ", data_ptr[j]);
                }
                if (tbl_name == DRV_CFP_RAM_TCAM) {
                    if (mask_ptr != NULL ) {
                        cli_out("\n");
                        cli_out("TCAM MASK[%d]: ", idx);
                        for (j = 0; j < entry_len; j++) {
                            cli_out("0x%08x ", mask_ptr[j]);
                        }
                    }
                }
            } else {
                cli_out("ERROR: all not support yet.\n");
            }
        
            cli_out("\n");
        }
    }

done:
    return retval;
}
cmd_result_t
robo_do_dump_table(int unit, soc_mem_t mem,
          int copyno, int index, int count, int flags)
{
    int k, i = 0;
    uint32 entry[SOC_ROBO_MAX_MEM_WORDS];
    int entry_dw;
    int rv = CMD_FAIL;
    uint32 table_name;
    int dt_mode = 0;
    uint8 mem_mapping = FALSE;

    if (soc_feature(unit, soc_feature_field)) {
        if ((mem == INDEX(CFP_ACT_POLm)) || 
            (mem == INDEX(CFP_METERm)) || (mem == INDEX(CFP_STAT_IBm)) ||
            (mem == INDEX(CFP_STAT_OBm)) || (mem == INDEX(CFP_TCAM_S0m)) ||
            (mem == INDEX(CFP_TCAM_S1m)) || (mem == INDEX(CFP_TCAM_S2m)) ||
            (mem == INDEX(CFP_TCAM_IPV4_SCm)) ||
            (mem == INDEX(CFP_TCAM_IPV4_MASKm)) ||
            (mem == INDEX(CFP_TCAM_IPV6_SCm)) ||
            (mem == INDEX(CFP_TCAM_IPV6_MASKm)) ||
            (mem == INDEX(CFP_TCAM_NONIP_SCm)) ||
            (mem == INDEX(CFP_TCAM_NONIP_MASKm)) ||
            (mem == INDEX(CFP_TCAM_CHAIN_SCm)) ||
            (mem == INDEX(CFP_TCAM_CHAIN_MASKm)) ||
            (mem == INDEX(CFP_TCAM_SCm)) || (mem == INDEX(CFP_GREEN_STATm)) || 
            (mem == INDEX(CFP_RED_STATm)) || (mem == INDEX(CFP_YELLOW_STATm))
            ) {
            return _do_dump_cfp_table(unit, mem, index, count, flags);
        }
    }
    entry_dw = soc_mem_entry_words(unit, mem);

    rv = DRV_MEM_READ(unit, mem, index, 1, entry);
    if (SOC_FAILURE(rv)){
        if (rv == SOC_E_PARAM){
                mem_mapping = TRUE;                                
        } else {
            return CMD_FAIL;
        }
    }     

    if(mem_mapping) {
        switch(mem) {
        	case INDEX(GEN_MEMORYm):
        		table_name = DRV_MEM_GEN;
        		break;
        	case INDEX(L2_ARLm):
        	case INDEX(L2_ARL_SWm):
        	case INDEX(L2_MARL_SWm):
        		table_name = DRV_MEM_ARL_HW;
        		break;
        	case INDEX(MARL_PBMPm):
        		table_name = DRV_MEM_MCAST;
        		break;
        	case INDEX(MSPT_TABm):
        		table_name = DRV_MEM_MSTP;
        		break;
            case INDEX(VLAN_1Qm):
                table_name = DRV_MEM_VLAN;

                bcm_port_dtag_mode_get(unit, 0, &dt_mode);
                if (dt_mode){
                    cli_out("\n Double tagging mode! ") ;
                    cli_out("Check 'VLAN show' for the real untagbitmap.\n\n");
                }
        		break;
            case INDEX(MAC2VLANm):
                table_name = DRV_MEM_MACVLAN;
                break;
            case INDEX(VLAN2VLANm):
                table_name = DRV_MEM_VLANVLAN;
                break;
            case INDEX(PROTOCOL2VLANm):
                table_name = DRV_MEM_PROTOCOLVLAN;
                break;
            case INDEX(FLOW2VLANm):
                table_name = DRV_MEM_FLOWVLAN;
                break;
            case INDEX(EGRESS_VID_REMARKm):
                table_name = DRV_MEM_EGRVID_REMARK;
        		break;
            case INDEX(CFP_TCAM_S0m):
        	case INDEX(CFP_TCAM_S1m):
        	case INDEX(CFP_TCAM_S2m):
            case INDEX(CFP_TCAM_IPV4_SCm):
            case INDEX(CFP_TCAM_IPV6_SCm):
            case INDEX(CFP_TCAM_NONIP_SCm):
            case INDEX(CFP_TCAM_CHAIN_SCm):
        	    table_name = DRV_MEM_TCAM_DATA;
        	    break;
        	case INDEX(CFP_TCAM_MASKm):
            case INDEX(CFP_TCAM_IPV4_MASKm):
            case INDEX(CFP_TCAM_IPV6_MASKm):
            case INDEX(CFP_TCAM_NONIP_MASKm):
            case INDEX(CFP_TCAM_CHAIN_MASKm):
        	    table_name = DRV_MEM_TCAM_MASK;
        	    break;
        	case INDEX(CFP_ACT_POLm):
		case INDEX(CFP_ACTm):
        	    table_name = DRV_MEM_CFP_ACT;
        	    break;
        	case INDEX(CFP_METERm):
        	    table_name = DRV_MEM_CFP_METER;
        	    break;
        	case INDEX(CFP_STAT_IBm):
        	    table_name = DRV_MEM_CFP_STAT_IB;
        	    break;
        	case INDEX(CFP_STAT_OBm):
        	    table_name = DRV_MEM_CFP_STAT_OB;
        	    break;
        	case INDEX(PORT_MASKm):
        	    table_name = DRV_MEM_PORTMASK;
        	    break;
        	case INDEX(SA_LRN_CNTm):
        	    table_name = DRV_MEM_SALRN_CNT_CTRL;
        	    break;
        	case INDEX(MCAST_VPORT_MAPm):
        	    table_name = DRV_MEM_MCAST_VPORT_MAP;
        	    break;
        	case INDEX(VPORT_VID_MAPm):
        	    table_name = DRV_MEM_VPORT_VID_MAP;
        	    break;
        	case INDEX(DPTC2PCPm):
        	    table_name = DRV_MEM_TCDP_TO_1P;
        	    break;
        	case INDEX(PCP2DPTCm):
        	    table_name = DRV_MEM_1P_TO_TCDP;
        	    break;
        	case INDEX(ERC_PORTm):
        	    table_name = DRV_MEM_ERC_PORT;
        	    break;
        	case INDEX(IRC_PORTm):
        	    table_name = DRV_MEM_IRC_PORT;
        	    break;
    	default:
               cli_out("Unsupport memory table.\n");
    	    return CMD_FAIL;
        }
    } else {
        table_name = mem;
    }

    for (k = index; k < index + count; k++) {
        rv = DRV_MEM_READ(unit, table_name, k, 1, entry);        
        if (rv  < 0) {
            cli_out("Read ERROR: table %s[%d]: %s\n",
                    SOC_ROBO_MEM_UFNAME(unit, mem),
                    k, soc_errmsg(rv));
            goto done;
        }

        if (flags & DUMP_TABLE_HEX) {
            for (i = 0; i < entry_dw; i++) {
                cli_out("%08x\n", entry[i]);
            }
        } else {
    		cli_out("%s[%d]: ", SOC_ROBO_MEM_UFNAME(unit, mem), k);
            if (flags & DUMP_TABLE_RAW) {
                for (i = 0; i < entry_dw; i++) {
                    cli_out("0x%08x ", entry[i]);
                }
            } else {
                cli_out("ERROR: all not support yet.\n");
            }
        
            cli_out("\n");
        }
    }

done:
    return rv;
}


cmd_result_t
cmd_robo_dump(int unit, args_t *a)
{
    regtype_entry_t *rt;
    soc_mem_t mem;
    char *arg1, *arg2, *arg3;
    volatile int flags = DUMP_TABLE_RAW;
    int copyno;
    volatile int rv = CMD_FAIL;
    parse_table_t pt;
    volatile char *fname = "";
    int append = FALSE;
    volatile int console_was_on = 0, console_disabled = 0, pushed_ctrl_c = 0;
    jmp_buf	ctrl_c;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "File", PQ_STRING, 0, &fname, 0);
    parse_table_add(&pt, "Append", PQ_BOOL, 0, &append, FALSE);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        goto done;
    }

    if (parse_arg_eq(a, &pt) < 0) {
        rv = CMD_USAGE;
        goto done;
    }

    console_was_on = bslcons_is_enabled();

    if (fname[0] != 0) {
    /*
     * Catch control-C in case if using file output option.
     */

#ifndef NO_CTRL_C
        if (setjmp(ctrl_c)) {
            rv = CMD_INTR;
            goto done;
        }
#endif

        sh_push_ctrl_c(&ctrl_c);

        pushed_ctrl_c = TRUE;

        if (bslfile_is_enabled()) {
            cli_out("%s: Can't dump to file while logging is enabled\n",
                    ARG_CMD(a));
            rv = CMD_FAIL;
            goto done;
        }

        if (bslfile_open((char *)fname, append) < 0) {
            cli_out("%s: Could not start log file\n", ARG_CMD(a));
            rv = CMD_FAIL;
            goto done;
        }

        bslcons_enable(FALSE);

        console_disabled = 1;
    }

    arg1 = ARG_GET(a);

    for (;;) {
        if (arg1 != NULL && !sal_strcasecmp(arg1, "raw")) {
            flags |= DUMP_TABLE_RAW;
            arg1 = ARG_GET(a);
    	} else if (arg1 != NULL && !sal_strcasecmp(arg1, "hex")) {
	        flags |= DUMP_TABLE_HEX;
	        arg1 = ARG_GET(a);
        } else {
            break;
        }
    }

    if (arg1 == NULL) {
        rv = CMD_USAGE;
        goto done;
    }

    /* Dump CPU Ethernet registers */
    if (!sal_strcasecmp(arg1, "emac")) {
	bcm_rx_debug(unit);
	rv = CMD_OK;
#if defined(__ECOS) && defined(GUI_SUPPORT) && defined(BCM_ROBO_SUPPORT)
	{
		extern uint32 get_cp0_status(void);
		extern void set_cp0_status(uint32 val);
		uint32 val;

		val = get_cp0_status();
		cli_out("CP0 Status %x\n", val);
	}
#endif
	goto done;
    }

    /* Dump CPU Ethernet registers */
    if (!sal_strcasecmp(arg1, "reg")) {
	int tloop = 0, loop = 0, wrong, r;
	uint32 regaddr, page, addr;
        soc_regaddrinfo_t ainfo;
	uint32 expect;
	uint64 regdata;
        char *arg4, *arg5;

	arg2 = ARG_GET(a);
	arg3 = ARG_GET(a);
	if (arg2 && isint(arg2) && arg3 && isint(arg3)) {
	    page = (uint32)parse_integer(arg2);
	    addr = (uint32)parse_integer(arg3);
	} else {
	    rv = CMD_USAGE;
  	    goto done;
	}

        regaddr = (parse_integer(arg2) << SOC_ROBO_PAGE_BP) | parse_integer(arg3);
        soc_robo_regaddrinfo_get(unit, &ainfo, regaddr);

	arg4 = ARG_GET(a);
	arg5 = ARG_GET(a);
	if ((arg4 && !isint(arg4)) || (arg5 && !isint(arg5))) {
	    rv = CMD_USAGE;
  	    goto done;
	}
	if (arg4) {
	    tloop = parse_integer(arg4);
	}
	if (arg5) {
	    expect = parse_integer(arg5);
	} else {
            r = soc_robo_anyreg_read(unit, &ainfo, &regdata);
	    if (r < 0) {
	        rv = CMD_FAIL;
	        goto done;
	    }
            expect = COMPILER_64_LO(regdata);
	}
	cli_out("Expect %x for register reg [0x%x 0x%x]\n", expect, page, addr);

	wrong = 0;
	do {
            r = soc_robo_anyreg_read(unit, &ainfo, &regdata);
	    if ((r < 0) || (expect != COMPILER_64_LO(regdata))) {
		wrong = 1;
	        cli_out("Found wrong register access: expect 0x%x,"
                        "got 0x%x after %d times\n",
                        expect, COMPILER_64_LO(regdata), loop);
	        break;
	    }
	    LOG_INFO(BSL_LS_APPL_PCI,
                     (BSL_META_U(unit,
                                 "dump reg loops [0x%x 0x%x] got 0x%x for %d times\n"),
                      page, addr, expect, ++loop));
	} while ((loop < tloop) || !tloop);

	if (!wrong)
	    cli_out("phydbg is successful for %d times\n", loop);

	rv = CMD_OK;
	goto done;
    }

    /* See if dumping a memory table */

    if (parse_memory_name(unit, &mem, arg1, &copyno, 0) >= 0) {
        int index, count = 0;
        arg2 = ARG_GET(a);
        arg3 = ARG_GET(a);
        if (mem > NUM_SOC_ROBO_MEM){
            cli_out("Error: Memory %d is out of range for chip %s.\n",
                    (int)mem, SOC_UNIT_NAME(unit));
            goto done;
        }
        if (!SOC_MEM_IS_VALID(unit, mem)) {
            cli_out("Error: Memory %d not valid for chip %s.\n",
                    (int)mem, SOC_UNIT_NAME(unit));
            goto done;
        }
        if (copyno == COPYNO_ALL) {
            copyno = SOC_MEM_BLOCK_ANY(unit, mem);
        }
        if (arg2) {
            index = parse_memory_index(unit, mem, arg2);
            count = (arg3 ? parse_integer(arg3) : 1);
        } else {
            index = soc_robo_mem_index_min(unit, mem);
            if (soc_mem_is_sorted(unit, mem) &&
                !(flags & DUMP_TABLE_ALL)) {
            } else {
                count = soc_robo_mem_index_max(unit, mem) - index + 1;
            }
        }
        rv = robo_do_dump_table(unit, mem, copyno, index, count, flags);
        goto done;
    }

    /*
     * See if dumping a register type
     */

    if ((rt = robo_regtype_lookup_name(arg1)) != NULL) {
        rv = do_dump_registers(unit, rt, a);
        goto done;
    }

    cli_out("Unknown option or memory to dump "
            "(use 'help dump' for more info)\n");

    rv = CMD_FAIL;

 done:
    if (fname[0] != 0) {
        bslfile_close();
    }

    if (console_disabled && console_was_on) {
        bslcons_enable(TRUE);
    }

    if (pushed_ctrl_c) {
        sh_pop_ctrl_c();
    }

    parse_arg_eq_done(&pt);
    return rv;
}

