/* 
 * $Id: reg.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag register commands
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <soc/counter.h>
#include <sal/appl/pci.h>
#include <soc/debug.h>
#include <soc/cmic.h>
#include <soc/drv_if.h>

#include <appl/diag/system.h>
#include <appl/diag/sysconf.h>
#include <ibde.h>

/* 
 * Utility routine to concatenate the first argument ("first"), with
 * the remaining arguments, with commas separating them.
 */

static void
collect_comma_args(args_t *a, char *valstr, char *first)
{
    char *s;

    strcpy(valstr, first);

    while ((s = ARG_GET(a)) != 0) {
        strcat(valstr, ",");
        strcat(valstr, s);
    }
}

/* 
 * modify_reg_fields
 *
 *   Takes a soc_reg_t 'regno', pointer to current value 'val',
 *   and a string 'mod' containing a field replacement spec of the form
 *   "FIELD=value".   The string may contain more than one spec separated
 *   by commas.  Updates the field in the register value accordingly,
 *   leaving all other unmodified fields intact.  If the string CLEAR is
 *   present in the list, the current register value is zeroed.
 *   If mask is non-NULL, it receives a mask of all fields modified.
 *
 *   Examples with modreg:
 *        modreg fe_mac1 lback=1        (modifies only lback field)
 *        modreg config ip_cfg=2        (modifies only ip_cfg field)
 *        modreg config clear,ip_cfg=2,cpu_pri=4  (zeroes all other fields)
 *
 *   Note that if "clear" appears in the middle of the list, the
 *   values in the list before "clear" are ignored.
 *
 *   Returns -1 on failure, 0 on success.
 */

static int
modify_reg_fields(int unit, soc_reg_t regno,
                  uint64 *val, uint64 *mask /* out param */, char *mod)
{
#ifndef SOC_NO_NAMES
    soc_field_info_t *fld;
    char *fmod, *fval;
    char modstr[256];
    soc_reg_info_t *reg = &SOC_REG_INFO(unit, regno);
    uint64 fvalue;
    uint64 fldmask;
    uint64 tmask;
    char *tokstr=NULL;

    strncpy(modstr, mod, sizeof(modstr));/* Don't destroy input string */
    modstr[sizeof(modstr) - 1] = 0;
    mod = modstr;

    if (mask) {
        COMPILER_64_ZERO(*mask);
    }

    while ((fmod = sal_strtok_r(mod, ",", &tokstr)) != 0) {
        mod = NULL;            /* Pass strtok NULL next time */
        fval = strchr(fmod, '=');
        if (fval) {            /* Point fval to arg, NULL if none */
            *fval++ = 0;       /* Now fmod holds only field name. */
        }
        if (fmod[0] == 0) {
            cli_out("Null field name\n");
            return -1;
        }
        if (!sal_strcasecmp(fmod, "clear")) {
            COMPILER_64_ZERO(*val);
            if (mask) {
                COMPILER_64_ALLONES(*mask);
            }
            continue;
        }
        for (fld = &reg->fields[0]; fld < &reg->fields[reg->nFields]; fld++) {
            if (!sal_strcasecmp(fmod, soc_robo_fieldnames[fld->field])) {
                break;
            }
        }
        if (fld == &reg->fields[reg->nFields]) {
            cli_out("No such field \"%s\" in register \"%s\".\n",
                    fmod, SOC_ROBO_REG_NAME(unit, regno));
            return -1;
        }
        if (!fval) {
            cli_out("Missing %d-bit value to assign to \"%s\" "
                    "field \"%s\".\n",
                    fld->len, SOC_ROBO_REG_NAME(unit, regno), soc_robo_fieldnames[fld->field]);
            return -1;
        }
        fvalue = parse_uint64(fval);
    
        /* Check that field value fits in field */
        COMPILER_64_MASK_CREATE(tmask, fld->len, 0);
        COMPILER_64_NOT(tmask);
        COMPILER_64_AND(tmask, fvalue);
    
        if (!COMPILER_64_IS_ZERO(tmask)) {
            cli_out("Value \"%s\" too large for %d-bit field \"%s\".\n",
                    fval, fld->len, soc_robo_fieldnames[fld->field]);
            return -1;
        }
    
        if (SOC_REG_IS_64(unit, regno)) {
#ifdef BE_HOST
            uint32 val32;
            if (fld->len <= 32){
                val32 = COMPILER_64_LO(fvalue);
                COMPILER_64_SET(fvalue,val32, 0);
            }
#endif
           (DRV_SERVICES(unit)->reg_field_set)
                (unit, regno, (void *)val, \
        	                  fld->field, (void *)&fvalue);
        } else {
            uint32 tmp;
            uint32 ftmp;
            COMPILER_64_TO_32_LO(tmp, *val);
            COMPILER_64_TO_32_LO(ftmp, fvalue);
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, regno, &tmp, fld->field, &ftmp);
            COMPILER_64_SET(*val, 0, tmp);
            COMPILER_64_SET(fvalue, 0, ftmp);
        }
    
        COMPILER_64_MASK_CREATE(fldmask, fld->len, fld->bp);
        if (mask) {
            COMPILER_64_OR(*mask, fldmask);
        }
    }
    
    return 0;
#else /* defined(SOC_NO_NAMES) */
    /* Functionality for this routine depends on soc_robo_fieldnames */
    return -1;
#endif /* !SOC_NO_NAME*/

}

#define PRINT_COUNT(str, len, wrap, prefix) \
    if ((wrap > 0) && (len > wrap)) { \
        cli_out(\
                "\n%s", prefix); \
        len = sal_strlen(prefix); \
    } \
    cli_out(\
            "%s", str); \
    len += strlen(str)

/* 
 * Print a SOC internal register with fields broken out.
 */
void
robo_reg_print(int unit, soc_regaddrinfo_t *ainfo, uint64 val, uint32 flags,
	  const char *fld_sep, int wrap)
{
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, ainfo->reg);
    int f;
    uint64 val64;
    char buf[80];
    char line_buf[256];
    int linelen = 0;

    uint64 dfl;
    uint32 v32, f32;
    uint32  *val_ptr, *fld_ptr;
    

    soc_robo_reg_sprint_addr(unit, buf, ainfo);

    if (flags & REG_PRINT_HEX) {
    	if (SOC_REG_IS_64(unit, ainfo->reg)) {
            cli_out("%08x%08x\n", COMPILER_64_HI(val), COMPILER_64_LO(val));
        } else {
            cli_out("%08x\n", COMPILER_64_LO(val));
        }
        return;
    }

    sal_sprintf(line_buf, "%s[0x%x]=", buf, ainfo->addr);
    PRINT_COUNT(line_buf, linelen, wrap, "   ");

    format_uint64(line_buf, val);
    PRINT_COUNT(line_buf, linelen, -1, "");

    if (flags & REG_PRINT_RAW) {
        cli_out("\n");
        return;
    }

    COMPILER_64_TO_32_LO(v32, val);
    if (SOC_REG_IS_64(unit, ainfo->reg)) {
        val_ptr = (uint32 *)(void *)&val;
    } else {
        val_ptr = &v32;
    }

    PRINT_COUNT(": <", linelen, wrap, "   ");

    for (f = reginfo->nFields - 1; f >= 0; f--) {
        soc_field_info_t *fld = &reginfo->fields[f];
        if ((fld->len) > 32) {
            fld_ptr = (uint32 *)(void *)&dfl;
        } else {
            fld_ptr = &f32;
        }
        (DRV_SERVICES(unit)->reg_field_get)(unit, ainfo->reg, val_ptr, fld->field, fld_ptr);
        if ((fld->len) > 32) {
#ifdef BE_HOST
            COMPILER_64_SET(val64, *fld_ptr, *(fld_ptr+1));
#else
            COMPILER_64_SET(val64, *(fld_ptr+1), *fld_ptr);
#endif
        } else {
            COMPILER_64_SET(val64, 0, *fld_ptr);
        }

#ifndef SOC_NO_NAMES
        sal_sprintf(line_buf, "%s=", soc_robo_fieldnames[fld->field]);
#else
        sal_sprintf(line_buf, "FIELD_%02d=", fld->field);
#endif /* !SOC_NO_NAME*/

        PRINT_COUNT(line_buf, linelen, wrap, "   ");
        format_uint64(line_buf, val64);
        PRINT_COUNT(line_buf, linelen, -1, "");
        if (f > 0) {
            sal_sprintf(line_buf, "%s", fld_sep);
            PRINT_COUNT(line_buf, linelen, -1, "");
        }
    }

    cli_out(">\n");
}

/* 
 * Reads and displays all SOC registers specified by alist structure.
 */
int
robo_reg_print_all(int unit, soc_regaddrlist_t *alist, uint32 flags)
{
    int j;
    uint64 value;
    int r, rv = 0;
    soc_regaddrinfo_t *ainfo;

    assert(alist);

    for (j = 0; j < alist->count; j++) {
        ainfo = &alist->ainfo[j];
        if ((r = soc_robo_anyreg_read(unit, ainfo, &value)) < 0) {
            char buf[80];
            soc_robo_reg_sprint_addr(unit, buf, ainfo);
            cli_out("ERROR: read from register %s failed: %s\n",
                    buf, soc_errmsg(r));
            rv = -1;
        } else {
            robo_reg_print(unit, ainfo, value, flags, ",", 62);
        }
    }
    return rv;
    return 0;
}


/*
 * Register Types - for getreg and dump commands
 */

static regtype_entry_t regtypes[] = {
 { "PCIC",	soc_pci_cfg_reg,"PCI Configuration space" },
 { "PCIM",	soc_cpureg,	"PCI Memory space (CMIC)" },
 { "SOC",	soc_schan_reg,	"SOC internal registers" },
 { "SCHAN",	soc_schan_reg,	"SOC internal registers" },
 { "PHY",	soc_phy_reg,	"PHY registers via MII (phyID<<8|phyADDR)" },
 { "MW",	soc_hostmem_w,	"Host Memory 32-bit" },
 { "MH",	soc_hostmem_h,	"Host Memory 16-bit" },
 { "MB",	soc_hostmem_b,	"Host Memory 8-bit" },
 { "MEM",	soc_hostmem_w,	"Host Memory 32-bit" },	/* Backward compat */
};

#define regtypes_count	COUNTOF(regtypes)

regtype_entry_t *robo_regtype_lookup_name(char* str)
{
    int i;

    for (i = 0; i < regtypes_count; i++) {
	if (!sal_strcasecmp(str,regtypes[i].name)) {
	    return &regtypes[i];
        }
    }

    return 0;
}

static int
_robo_internal_reg_get(int unit, uint32 regaddr, uint64 *data, int len)
{
    int rv;
    uint64 val64;
    
    rv = DRV_REG_READ
        (unit, regaddr, (uint32 *)data, len);

    if (len <= 4) {
        COMPILER_64_SET(val64, 0, *(uint32 *)data);
        COMPILER_64_SET(*data, 0, 0);
        COMPILER_64_OR(*data, val64);
     }

    if (len < 8) {
        COMPILER_64_SET(val64, 0, 1);
        COMPILER_64_SHL(val64, 8 * len);
        COMPILER_64_SUB_32(val64, 1);
        COMPILER_64_AND(*data, val64);
    }

    return rv;
}

static int
_robo_internal_reg_set(int unit, uint32 regaddr, uint64 data, int len)
{
    int rv = SOC_E_NONE;
#ifdef BE_HOST
    uint32 val32=0;
#endif

#ifdef BE_HOST
    if (len <= 4) {
        COMPILER_64_TO_32_LO(val32, data);
        rv = DRV_REG_WRITE
            (unit, regaddr, &val32, len);
    } else {
        rv = DRV_REG_WRITE
            (unit, regaddr, &data, len);
    }
#else
    rv = DRV_REG_WRITE
        (unit, regaddr, &data, len);
#endif

    return rv;

}

/* 
 * Get a register by type.
 *
 * doprint:  Boolean.  If set, display data.
 */
STATIC int
_robo_reg_get_by_type(int unit, uint32 regaddr, soc_regtype_t regtype,
                 uint64 *outval, uint32 flags)
{
    int rv = CMD_OK;
    int r;
    uint16 phy_rd_data;
    soc_regaddrinfo_t ainfo;
    int is64 = FALSE;
    

    switch (regtype) {
        case soc_pci_cfg_reg:
            if (regaddr & 3) {
                cli_out("ERROR: PCI config addr must be multiple of 4\n");
                rv = CMD_FAIL;
            } else {
                COMPILER_64_SET(*outval, 0, bde->pci_conf_read(unit, regaddr));
            }
            break;
    
        case soc_cpureg:
            if (regaddr & 3) {
                cli_out("ERROR: PCI memory addr must be multiple of 4\n");
                rv = CMD_FAIL;
            } else {
                COMPILER_64_SET(*outval, 0, soc_pci_read(unit, regaddr));
            }
            break;
    
        case soc_schan_reg:
        case soc_spi_reg:
        case soc_genreg:
        case soc_portreg:
        case soc_cosreg:
            soc_robo_regaddrinfo_get(unit, &ainfo, regaddr);
            if (ainfo.reg >= 0) {
                is64 = SOC_REG_IS_64(unit, ainfo.reg);
            }

            r = soc_robo_anyreg_read(unit, &ainfo, outval);
            if (r < 0) {
                if (!LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)) {
                    cli_out("ERROR: soc_reg_read failed: %s\n", soc_errmsg(r));
                }
                rv = CMD_FAIL;
            }
    
            break;
    
        case soc_hostmem_w:
            COMPILER_64_SET(*outval, 0, *((uint32 *)INT_TO_PTR(regaddr)));
            break;
        case soc_hostmem_h:
            COMPILER_64_SET(*outval, 0, *((uint16 *)INT_TO_PTR(regaddr)));
            break;
        case soc_hostmem_b:
            COMPILER_64_SET(*outval, 0, *((uint8 *)INT_TO_PTR(regaddr)));
            break;
    
        case soc_phy_reg:
            /* Leave for MII debug reads */
            if ((r = soc_robo_miim_read(unit,
                           (uint8) (regaddr >> 8 & 0xff),	/* Phy ID */
                           (uint8) (regaddr & 0xff),	/* Phy addr */
                           &phy_rd_data)) < 0) {
                cli_out("ERROR: soc_miim_read failed: %s\n", soc_errmsg(r));
                rv = CMD_FAIL;
            } else {
                COMPILER_64_SET(*outval, 0, (uint32) phy_rd_data);
            }
            break;
        case soc_invalidreg:
            if (LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)) {
                r = _robo_internal_reg_get(unit, regaddr, outval, flags);
                if (r < 0) {
                    cli_out("ERROR: soc_reg_read failed: %s\n", soc_errmsg(r));
                    rv = CMD_FAIL;
                }
                flags = REG_PRINT_DO_PRINT;
                break;
            }
        default:
            assert(0);
            rv = CMD_FAIL;
            break;
    }

    if ((rv == CMD_OK) && (flags & REG_PRINT_DO_PRINT)) {
        if (flags & REG_PRINT_HEX) {
            if (is64) {
                cli_out("%08x%08x\n",
                        COMPILER_64_HI(*outval),
                        COMPILER_64_LO(*outval));
            } else {
                cli_out("%08x\n",
                        COMPILER_64_LO(*outval));
            }
        } else {
            char buf[80];

            format_uint64(buf, *outval);

            cli_out("%s[0x%x] = %s\n",
                    soc_regtypenames[regtype], regaddr, buf);
        }
    }

    return rv;
}

/* 
 * Set a register by type.  For SOC registers, is64 is used to
 * indicate if this is a 64 bit register.  Otherwise, is64 is
 * ignored.
 *
 */
STATIC int
_robo_reg_set_by_type(int unit, uint32 regaddr, soc_regtype_t regtype,
                 uint64 regval)
{
    int rv = CMD_OK, r;
    uint32 val32;
    soc_regaddrinfo_t ainfo;

    COMPILER_64_TO_32_LO(val32, regval);

    switch (regtype) {
        case soc_pci_cfg_reg:
            bde->pci_conf_write(unit, regaddr, val32);
            break;
    
        case soc_cpureg:
            soc_pci_write(unit, regaddr, val32);
            break;
    
        case soc_schan_reg:
        case soc_spi_reg:
        case soc_genreg:
        case soc_portreg:
        case soc_cosreg:
            soc_robo_regaddrinfo_get(unit, &ainfo, regaddr);
    
            r = soc_robo_anyreg_write(unit, &ainfo, regval);
            if (r < 0) {
                if (!LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)) {
                    cli_out("ERROR: write reg failed: %s\n", soc_errmsg(r));
                }
                rv = CMD_FAIL;
            }
    
            break;
    
        case soc_hostmem_w:
            *((uint32 *)INT_TO_PTR(regaddr)) = val32;
            break;
    
        case soc_hostmem_h:
            *((uint16 *)INT_TO_PTR(regaddr)) = val32;
            break;
    
        case soc_hostmem_b:
            *((uint8 *)INT_TO_PTR(regaddr)) = val32;
            break;
    
        case soc_phy_reg:
            /* Leave for MII debug writes */
            if ((r = soc_robo_miim_write(unit,
                        (uint8) (regaddr >> 8 & 0xff),	/* Phy ID */
                        (uint8) (regaddr & 0xff),	/* Phy addr */
                        (uint16) val32)) < 0) {
                cli_out("ERROR: write miim failed: %s\n", soc_errmsg(r));
                rv = CMD_FAIL;
            }
            break;
        default:
            assert(0);
            rv = CMD_FAIL;
            break;
    }

    return rv;
}

/* 
 * Gets a memory value or register from the SOC.
 * Syntax: getreg [<regtype>] <offset|symbol>
 */

cmd_result_t
cmd_robo_reg_get(int unit, args_t *a)
{
    uint64 regval;
    uint32 regaddr = 0, len = 0;
    cmd_result_t rv = CMD_OK;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist;
    char *name;
    uint32 flags = REG_PRINT_DO_PRINT;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    /* 
     * If first arg is a register type, take it and use the next argument
     * as the name or address, otherwise default to register type "soc."
     */
    name = ARG_GET(a);

    for (;;) {
        if (name != NULL && !sal_strcasecmp(name, "raw")) {
            flags |= REG_PRINT_RAW;
            name = ARG_GET(a);
        } else if (name != NULL && !sal_strcasecmp(name, "hex")) {
            flags |= REG_PRINT_HEX;
            name = ARG_GET(a);
        } else {
            break;
        }
    }

    if (name == NULL) {
        return CMD_USAGE;
    }

    if ((rt = robo_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = robo_regtype_lookup_name("soc");
    }

    assert(rt);

    if (soc_robo_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        rv = CMD_FAIL;
        goto done;
    }

    if (isint(name)) {             /* Numerical address given */
        regaddr = parse_integer(name);  /* register page value */
        if (((name = ARG_GET(a)) == 0) || !isint(name)) {
            return CMD_USAGE;
        }
        /* full address is ((page << 8) + offset) */
        regaddr = (regaddr << SOC_ROBO_PAGE_BP) | parse_integer(name);
        rv = _robo_reg_get_by_type(unit, regaddr, rt->type, &regval, flags);
        if (SOC_FAILURE(rv)) {
            if (LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)) {
                if((name = ARG_GET(a)) == 0){                
                    cli_out("length should be be specified!");
                    return CMD_USAGE;
                }
                len = parse_integer(name);
                cli_out("%d len %d\n",__LINE__,len);
                rv = _robo_reg_get_by_type(unit, regaddr, soc_invalidreg, &regval, len);
            }
        }
    } else {
        if (*name == '$') {
            name++;
        }
    
        /* Symbolic name given, print all or some values ... */
        if (parse_symbolic_reference(unit, &alist, name) < 0) {
            cli_out("Syntax error parsing \"%s\"\n", name);
            rv = CMD_FAIL;
        } else if (robo_reg_print_all(unit, &alist, flags) < 0) {
            rv = CMD_FAIL;
        }
    }

done:
    soc_robo_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Auxilliary routine to handle setreg and modreg.
 *      mod should be 0 for setreg, which takes either a value or a
 *              list of <field>=<value>, and in the latter case, sets all
 *              non-specified fields to zero.
 *      mod should be 1 for modreg, which does a read-modify-write of
 *              the register and permits value to be specified by a list
 *              of <field>=<value>[,...] only.
 */

STATIC cmd_result_t
do_robo_reg_set(int unit, args_t *a, int mod)
{
    uint64 regval;
    uint32 regaddr = 0, len = 0;
    int rv = CMD_OK, i;
    regtype_entry_t *rt;
    soc_regaddrlist_t alist;
    soc_regaddrinfo_t *ainfo, tmp_ainfo;
    char *name;
    char *s, valstr[256];

    COMPILER_64_ALLONES(regval);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((name = ARG_GET(a)) == 0) {
        return CMD_USAGE;
    }

    /* 
     * If first arg is an access type, take it and use the next argument
     * as the name, otherwise use default access type.
     * modreg command does not allow this and assumes soc.
     */

    if (!mod && (rt = robo_regtype_lookup_name(name)) != 0) {
        if ((name = ARG_GET(a)) == 0) {
            return CMD_USAGE;
        }
    } else {
        rt = robo_regtype_lookup_name("soc");
        assert(rt);
    }

    if (soc_robo_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (!mod && isint(name)) {
        /* Numerical address given */
        regaddr = parse_integer(name);  /* register page value */

        if (((name = ARG_GET(a)) == 0) || !isint(name)) {
            return CMD_USAGE;
        }
        /* full address is ((page << 8) + offset) */
        regaddr = (regaddr << SOC_ROBO_PAGE_BP) | parse_integer(name);

        if (LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)) {
            soc_robo_regaddrinfo_get(unit, &tmp_ainfo, regaddr);
            if (tmp_ainfo.reg == INDEX(INVALID_Rr)) {
                if (((name = ARG_GET(a)) == 0) || !isint(name)) {
                    return CMD_USAGE;
                }
                len = parse_integer(name);
            }
        }
        /* 
         * Parse the value field.  If there are more than one, string them
         * together with commas separating them (to make field-based setreg
         * inputs more convenient).
         */

        if ((s = ARG_GET(a)) == 0) {
            cli_out("Syntax error: missing value\n");
            return CMD_USAGE;
        }

        collect_comma_args(a, valstr, s);

        regval = parse_uint64(valstr);
        rv = _robo_reg_set_by_type(unit, regaddr, rt->type, regval);

        if (SOC_FAILURE(rv)) {
            if (LOG_CHECK(BSL_LS_SOC_REG | BSL_INFO)) {
                rv = _robo_internal_reg_set(unit, regaddr, regval, len);
                if (rv < 0) {
                    cli_out("ERROR: write reg failed: %s\n", soc_errmsg(rv));
                    rv = CMD_FAIL;
                }
            }
        }
    } else {    /* Symbolic name given, set all or some * values ... */
        /* 
         * Parse the value field.  If there are more than one, string them
         * together with commas separating them (to make field-based setreg
         * inputs more convenient).
         */

        if ((s = ARG_GET(a)) == 0) {
            cli_out("Syntax error: missing value\n");
            return CMD_USAGE;
        }

        collect_comma_args(a, valstr, s);

        if (mod && isint(valstr)) {
            return CMD_USAGE;
        }

        if (*name == '$') {
            name++;
        }
        if (parse_symbolic_reference(unit, &alist, name) < 0) {
            cli_out("Syntax error parsing \"%s\"\n", name);
            rv = CMD_FAIL;
        } else {
            if (isint(valstr)) {       /* valstr is numeric */
                regval = parse_uint64(valstr);
            }
        
            for (i = 0; i < alist.count; i++) {
                ainfo = &alist.ainfo[i];
                
                /* alist now holds list of registers to change */
                if (!isint(valstr)) {  /* Must modify registers */
                    /* 
                     * valstr must be a field replacement spec.
                     * In modreg mode, read the current register value,
                     * and modify it.  In setreg mode,
                     * assume a starting value of zero and modify it.
                     */
                    if (mod) {
                        rv = _robo_reg_get_by_type(unit, ainfo->addr,
                                        SOC_REG_INFO(unit, ainfo->reg).regtype,
                                        &regval, 0);
                    } else {
                        COMPILER_64_ZERO(regval);
                    }
                
                    if (rv == CMD_OK) {
                        if ((rv = modify_reg_fields(unit, ainfo->reg, &regval,
                                        (uint64 *) 0, valstr)) < 0) {
                            cli_out("Syntax error, aborted\n");
                        }
                    }
                }
                
                if (rv == CMD_OK) {
                    rv = _robo_reg_set_by_type(unit, ainfo->addr,
                            SOC_REG_INFO(unit, ainfo->reg).regtype, regval);
                }
                if (rv != CMD_OK) {
                    break;
                }
            }
        }
    }

    soc_robo_regaddrlist_free(&alist);
    return rv;
}

/* 
 * Sets a memory value or register on the SOC.
 * Syntax 1: setreg [<regtype>] <offset|symbol> <value>
 * Syntax 2: setreg [<regtype>] <offset|symbol> <field>=<value>[,...]
 */
cmd_result_t
cmd_robo_reg_set(int unit, args_t *a)
{
    return do_robo_reg_set(unit, a, 0);
}

/* 
 * Read/modify/write a memory value or register on the SOC.
 * Syntax: modreg [<regtype>] <offset|symbol> <field>=<value>[,...]
 */
cmd_result_t
cmd_robo_reg_mod(int unit, args_t * a)
{
    return do_robo_reg_set(unit, a, 1);
}

/* 
 * Lists registers containing a specific pattern
 *
 * If use_reset is true, ignores val and uses reset default value.
 */

static void
do_reg_list(int unit, soc_regaddrinfo_t *ainfo, int use_reset, uint64 regval)
{
    soc_reg_t reg = ainfo->reg;
    soc_reg_info_t *reginfo = &SOC_REG_INFO(unit, reg);
    soc_field_info_t *fld;
    int f;
    uint32 flags;
    uint64 mask, rval, rmsk;
    uint64 fldval;
    char buf[80];
    char rval_str[20], rmsk_str[20], dval_str[20];
    int i;

    if (!SOC_IS_ROBO(unit) || !SOC_REG_IS_VALID(unit, reg)) {
        cli_out("Register %d is not valid for chip %s\n",
                (int)reg, SOC_UNIT_NAME(unit));
        return;
    }

    flags = reginfo->flags;

    COMPILER_64_ZERO(fldval);
    COMPILER_64_ALLONES(mask);

    SOC_REG_RST_VAL_GET(unit, reg, rval);
    SOC_REG_RST_MSK_GET(unit, reg, rmsk);
    format_uint64(rval_str, rval);
    format_uint64(rmsk_str, rmsk);
    if (use_reset) {
        regval = rval;
        mask = rmsk;
    } else {
        format_uint64(dval_str, regval);
    }

    soc_robo_reg_sprint_addr(unit, buf, ainfo);

    cli_out("Register: %s", buf);
    if (SOC_ROBO_REG_ALIAS(unit, reg) && *SOC_ROBO_REG_ALIAS(unit, reg)) {
        cli_out(" alias %s", SOC_ROBO_REG_ALIAS(unit, reg));
    }
    cli_out(" %s register", soc_regtypenames[reginfo->regtype]);
    cli_out(" address 0x%04x", ainfo->addr);

    cli_out("Flags:");
    /* Need to Check SOC_REG_FLAG_64_BITS flag */
    cli_out(" %d-bits",  (DRV_SERVICES(unit)->reg_length_get)(unit, reg)*8);

    if (flags & SOC_REG_FLAG_COUNTER) {
        cli_out(" counter");
    }
    if (flags & SOC_REG_FLAG_ARRAY) {
        cli_out(" array[%d-%d]", 0, reginfo->numels-1);
    }
    if (flags & SOC_REG_FLAG_NO_DGNL) {
        cli_out(" no-diagonals");
    }
    if (flags & SOC_REG_FLAG_RO) {
        cli_out(" read-only");
    }
    if (flags & SOC_REG_FLAG_WO) {
        cli_out(" write-only");
    }
    if (flags & SOC_REG_FLAG_ED_CNTR) {
        cli_out(" error/discard-counter");
    }
    cli_out("\n");

    if (SOC_ROBO_REG_DESC(unit, reg) && *SOC_ROBO_REG_DESC(unit, reg)) {
        cli_out("Description: %s\n", SOC_ROBO_REG_DESC(unit, reg));
    }
    cli_out("Displaying:");
    if (use_reset) {
        cli_out(" reset defaults");
    } else {
        cli_out(" value %s", dval_str);
    }
    cli_out(", reset value %s mask %s\n", rval_str, rmsk_str);

    for (f = reginfo->nFields - 1; f >= 0; f--) {
        fld = &reginfo->fields[f];
#ifndef SOC_NO_NAMES
        cli_out("  %s<%d", soc_robo_fieldnames[fld->field],
            fld->bp + fld->len - 1);
#else
        cli_out("  FIELD_%02d<%d", fld->field, fld->bp + fld->len - 1);
#endif /* !SOC_NO_NAME*/
        if (fld->len > 1) {
            cli_out(":%d", fld->bp);
        }

        if (SOC_REG_IS_64(unit, reg)) {
            (DRV_SERVICES(unit)->reg_field_get)(unit, reg, (void *)&mask, \
                              fld->field, (void *)&fldval);
        } else {
            uint32 tmp;
            uint32 ftmp;
            COMPILER_64_TO_32_LO(tmp, mask);
            COMPILER_64_TO_32_LO(ftmp, fldval);
            (DRV_SERVICES(unit)->reg_field_get)(unit, reg, &tmp, fld->field, &ftmp);
            COMPILER_64_SET(mask, 0, tmp);
            COMPILER_64_SET(fldval, 0, ftmp);
        }

        if (use_reset && COMPILER_64_IS_ZERO(fldval)) {
            cli_out("> = x");
        } else {
            if (SOC_REG_IS_64(unit, reg)) {
    	        (DRV_SERVICES(unit)->reg_field_get)(unit, reg, (void *)&regval, \
	                              fld->field, (void *)&fldval);
            } else {
                uint32 tmp;
                uint32 ftmp;
                COMPILER_64_TO_32_LO(tmp, regval);
                COMPILER_64_TO_32_LO(ftmp, fldval);
                (DRV_SERVICES(unit)->reg_field_get)(unit, reg, &tmp, fld->field, &ftmp);
                COMPILER_64_SET(regval, 0, tmp);
                COMPILER_64_SET(fldval, 0, ftmp);
            }
            format_uint64(buf, fldval);
            cli_out("> = %s", buf);
        }
        if (fld->flags & (SOCF_RO|SOCF_WO)) {
            cli_out(" [");
            i = 0;
            if (fld->flags & SOCF_RO) {
                cli_out("%sRO", i++ ? "," : "");
            }
            if (fld->flags & SOCF_WO) {
                cli_out("%sWO", i++ ? "," : "");
            }
            cli_out("]");
        }
        cli_out("\n");
    }
}

static void
_print_regname(int unit, soc_reg_t reg, int *col, int alias)
{
    int             len;

    len = strlen(SOC_ROBO_REG_NAME(unit, reg)) + 1;
    if (*col + len > (alias ? 65 : 72)) {
    	cli_out("\n  ");
    	*col = 2;
    }
    cli_out(\
            "%s%s ", SOC_ROBO_REG_NAME(unit, reg), \
            SOC_REG_ARRAY(unit, reg) ? "[]" : "");
    if (alias && SOC_ROBO_REG_ALIAS(unit, reg)) {
        len += strlen(SOC_ROBO_REG_ALIAS(unit, reg)) + 8;
        cli_out("(aka %s) ", SOC_ROBO_REG_ALIAS(unit, reg));
    }
    *col += len;
}

cmd_result_t
cmd_robo_reg_list(int unit, args_t *a)
{
    char *str;
    char *val;
    uint64 value;
    soc_regaddrinfo_t ainfo;
    int found;
    cmd_result_t rv = CMD_OK;
    int all_regs = 0;
    soc_reg_t reg;
    int col = 2;
    int alias = FALSE;

    ainfo.reg = INDEX(INVALID_Rr);

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if ((str = ARG_GET(a)) == NULL) {
        return CMD_USAGE;
    }

    if (isint(str)) {
        /* 
         * Address given, look up SOC register.
         */
        char            buf[80];
        uint32          addr;
        addr = parse_integer(str);  /* register page value */
        if (((str = ARG_GET(a)) == 0) || !isint(str)) {
            return CMD_USAGE;
        }
        /* full address is ((page << 8) + offset) */
        addr = (addr << SOC_ROBO_PAGE_BP) | parse_integer(str);
        soc_robo_regaddrinfo_get(unit, &ainfo, addr);
        if (!ainfo.valid || (int)ainfo.reg < 0) {
            cli_out("Unknown register address: 0x%x\n", addr);
            rv = CMD_FAIL;
        } else {
            soc_robo_reg_sprint_addr(unit, buf, &ainfo);
            cli_out("Address %s\n", buf);
        }
    } else {
        soc_regaddrlist_t alist;

        if (soc_robo_regaddrlist_alloc(&alist) < 0) {
            cli_out("Could not allocate address list.  Memory error.\n");
            return CMD_FAIL;
        }

        /* 
         * Symbolic name.
         * First check if the register is there as exact match.
         * If not, list all substring matches.
         */

        if (*str == '$') {
            str++;
        } else if (*str == '*') {
            str++;
            all_regs = 1;
        }

        if (parse_symbolic_reference(unit, &alist, str) < 0) {
            found = 0;
            for (reg = 0; reg < NUM_SOC_ROBO_REG; reg++) {
                 if (&SOC_REG_INFO(unit,reg) == NULL) {            
                    continue;
                }
                if (!SOC_REG_IS_VALID(unit, reg)) {
                    continue;
                }

                if (strcaseindex(SOC_ROBO_REG_NAME(unit, reg), str) != 0) {
                    if (!found) {
                        cli_out("%s  ",
                                all_regs ? "" :
                                "Unknown register; "
                                "possible matches are:\n");
                    }
                    _print_regname(unit, reg, &col, alias);
                    found = 1;
                }
            }
            if (!found) {
                cli_out("No matching register found");
            }
            cli_out("\n");
            rv = CMD_FAIL;
        } else {
            ainfo = alist.ainfo[0];
        }

        soc_robo_regaddrlist_free(&alist);
    }

    if ((val = ARG_GET(a)) != NULL) {
        value = parse_uint64(val);
    } else {
        COMPILER_64_ZERO(value);
    }

    /* 
     * Now have ainfo -- if reg is no longer INVALID_Rr
     */

    if (ainfo.reg != INDEX(INVALID_Rr)) {
        if (val) {
            do_reg_list(unit, &ainfo, 0, value);
        } else {
            COMPILER_64_ZERO(value);
            do_reg_list(unit, &ainfo, 1, value);
        }
    }

    return rv;
}

/* 
 * Editreg allows modifying register fields.
 * Works on fully qualified SOC registers only.
 */

cmd_result_t
cmd_robo_reg_edit(int unit, args_t *a)
{
    soc_reg_info_t *reginfo;
    soc_field_info_t *fld;
    soc_regaddrlist_t alist;
    soc_reg_t reg;
    uint64 v64, dfl;
    uint32 v32, f32;
    char ans[64], dfl_str[64];
    char *name = ARG_GET(a);
    int r, rv = CMD_FAIL;
    int i, f;
    uint32  *val_ptr, *fld_ptr;
    uint32  fv[2];
    uint32 mask[2];

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return rv;
    }

    if (!name) {
        return CMD_USAGE;
    }

    if (*name == '$') {
        name++;
    }

    if (soc_robo_regaddrlist_alloc(&alist) < 0) {
        cli_out("Could not allocate address list.  Memory error.\n");
        return CMD_FAIL;
    }

    if (parse_symbolic_reference(unit, &alist, name) < 0) {
        cli_out("Syntax error parsing \"%s\"\n", name);
        goto done;
    }

    reg = alist.ainfo[0].reg;
    reginfo = &SOC_REG_INFO(unit, reg);

    /* 
     * If more than one register was specified, read the first one
     * and write the edited value to all of them.
     */

    if (soc_robo_anyreg_read(unit, &alist.ainfo[0], &v64) < 0) {
        cli_out("ERROR: read reg failed\n");
        goto done;
    }

    COMPILER_64_TO_32_LO(v32, v64);
    if (SOC_REG_IS_64(unit, reg)) {
        val_ptr = (uint32 *)(void *)&v64;
    } else {
        val_ptr = &v32;
    }

    if (SOC_REG_IS_64(unit, reg)) {
#ifdef BE_HOST
        cli_out("Current value (bit31~0): 0x%x\n", *(val_ptr+1));
        cli_out("Current value (bit63~32): 0x%x\n", *val_ptr);
#else
        cli_out("Current value (bit31~0): 0x%x\n", *val_ptr);
        cli_out("Current value (bit63~32): 0x%x\n", *(val_ptr+1));
#endif
    } else {
        cli_out("Current value: 0x%x\n", v32);
    }

    for (f = 0; f < (int)reginfo->nFields; f++) {
        fld = &reginfo->fields[f];
        if ((fld->len) > 32) {
            fld_ptr = (uint32 *)(void *)&dfl;
        } else {
            fld_ptr = &f32;
        }
         (DRV_SERVICES(unit)->reg_field_get)(unit, reg, val_ptr, fld->field, fld_ptr);
        if ((fld->len) > 32) {
#ifdef BE_HOST
            sal_snprintf(dfl_str, sizeof(dfl_str), "0x%x%8x", *fld_ptr, *(fld_ptr+1));
#else
            sal_snprintf(dfl_str, sizeof(dfl_str), "0x%x%8x", *(fld_ptr+1), *fld_ptr);
#endif
        } else {
            sal_snprintf(dfl_str, sizeof(dfl_str), "0x%x", *fld_ptr);
        }
        
#ifndef SOC_NO_NAMES
        sal_snprintf(ans, sizeof(ans),          /* Also use ans[] for prompt */
        "  %s<%d", soc_robo_fieldnames[fld->field], fld->bp + fld->len - 1);
#else
        sal_snprintf(ans, sizeof(ans),     /* Also use ans[] for prompt */
        "  FIELD_%02d<%d", fld->field, fld->bp + fld->len - 1);
#endif /* !SOC_NO_NAME*/

        if (fld->len > 1) {
            sal_snprintf(ans + strlen(ans), (sizeof(ans) - strlen(ans)), ":%d", fld->bp);
        }
        strncat(ans, ">? ", sizeof(ans) - strlen(ans));
        if (sal_readline(ans, ans, sizeof(ans), dfl_str) == 0 || ans[0] == 0) {
            cli_out("Aborted\n");
            goto done;
        }
        fv[0] = 0;
        fv[1] = 1;
        parse_long_integer(fv, 2, ans);
        if (fld->len > 32) {
            mask[0] = -1;
            mask[1] = (1 << (fld->len - 32))-1;
        } else {
            mask[0] = (1 << fld->len) - 1;
            mask[1] = 0;
        }
        if ((fv[0] & ~(mask[0])) || (fv[1] & ~(mask[1]))) {
            cli_out("Value too big for %d-bit field, try again.\n",
                    fld->len);
            f--;
        } else {
#ifdef BE_HOST
            if (fld->len > 32) {
                f32 = fv[0];
                fv[0] = fv[1];
                fv[1] = f32;
            }
#endif
            (DRV_SERVICES(unit)->reg_field_set)
                (unit, reg, val_ptr, fld->field, &fv[0]);
        }
    }

    if (SOC_REG_IS_64(unit, reg)) {
#ifdef BE_HOST
        cli_out("Writing new value: 0x%x 0x%x\n", *val_ptr, *(val_ptr+1));
#else
        cli_out("Writing new value: 0x%x 0x%x\n", *(val_ptr+1), *val_ptr);
#endif
    } else {
        cli_out("Writing new value: 0x%x\n", *val_ptr);
    }
    if (!SOC_REG_IS_64(unit, reg)) {
        COMPILER_64_SET(v64, 0, v32);
    }
    for (i = 0; i < alist.count; i++) {
        if ((r = soc_robo_anyreg_write(unit, &alist.ainfo[i], v64)) < 0) {
            cli_out("ERROR: write reg 0x%x failed: %s\n",
                    alist.ainfo[i].addr, soc_errmsg(r));
            goto done;
        }
    }

    rv = CMD_OK;

  done:
    soc_robo_regaddrlist_free(&alist);
    return rv;
}
