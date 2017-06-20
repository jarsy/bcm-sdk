/*
 * $Id: mem.c,v 1.40 Broadcom SDK $
 * $Id: mem.c,v 1.39 2012/09/07 10:21:31 ako Exp $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * socdiag memory commands
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <soc/mem.h>
#include <soc/debug.h>

#include <soc/error.h>

#include <appl/diag/system.h>

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
#ifndef SOC_NO_NAMES
static int
_soc_mem_id_map(int unit, soc_mem_t mem, int *drv_mem){

    int table_name;
    
    switch(mem) {
        case INDEX(GEN_MEMORYm):
            table_name = DRV_MEM_GEN;
            break;
        case INDEX(L2_ARLm):
        case INDEX(L2_MARLm):
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
            break;
        case INDEX(VLAN2VLANm):
            table_name = DRV_MEM_VLANVLAN;
            break;
        case INDEX(MAC2VLANm):
            table_name = DRV_MEM_MACVLAN;
            break;
        case INDEX(PROTOCOL2VLANm):
            table_name = DRV_MEM_PROTOCOLVLAN;
            break;
        case INDEX(FLOW2VLANm):
            table_name = DRV_MEM_FLOWVLAN;
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
        case INDEX(EGRESS_VID_REMARKm):
            table_name = DRV_MEM_EGRVID_REMARK;
            break;
        default:
            cli_out("Unsupport memory table.\n");
            return -1;
    }
    *drv_mem = table_name;
    return SOC_E_NONE;
}
static int
_soc_mem_field_map(int unit, int table_name, int mem_field, int *drv_field)
{

    int field_name;
    if ((table_name == DRV_MEM_TCAM_DATA) ||
            (table_name == DRV_MEM_TCAM_MASK) ||
            (table_name == DRV_MEM_CFP_ACT) ||
            (table_name == DRV_MEM_CFP_METER) ||
            (table_name == DRV_MEM_CFP_STAT_IB) ||
            (table_name == DRV_MEM_CFP_STAT_OB)) {
            switch (mem_field) {
                case INDEX(VALID_Rf):
                    field_name = DRV_CFP_FIELD_VALID;
                    break;
                case INDEX(SLICEIDf):
                    field_name = DRV_CFP_FIELD_SLICE_ID;
                    break;
                case INDEX(VLANTAGGEDf):
                    field_name = DRV_CFP_FIELD_1QTAGGED;
                    break;
                case INDEX(SPTAGGEDf):
                    field_name = DRV_CFP_FIELD_SPTAGGED;
                    break;
                case INDEX(MAC_DAf):
                    field_name = DRV_CFP_FIELD_MAC_DA;
                    break;
                case INDEX(MAC_SAf):
                    field_name = DRV_CFP_FIELD_MAC_SA;
                    break;
                case INDEX(SP_VIDf):
                    field_name = DRV_CFP_FIELD_SP_VID;
                    break;
                case INDEX(SP_CFIf):
                    field_name = DRV_CFP_FIELD_SP_CFI;
                    break;
                case INDEX(SP_PRIf):
                    field_name = DRV_CFP_FIELD_SP_PRI;
                    break;
                case INDEX(USR_VIDf):
                    field_name = DRV_CFP_FIELD_USR_VID;
                    break;
                case INDEX(USR_CFIf):
                    field_name = DRV_CFP_FIELD_USR_CFI;
                    break;
                case INDEX(USR_PRIf):
                    field_name = DRV_CFP_FIELD_USR_PRI;
                    break;
                case INDEX(ETYPEf):
                    field_name = DRV_CFP_FIELD_ETYPE;
                    break;
                case INDEX(UDF0_VLDf):
                    field_name = DRV_CFP_FIELD_UDF0_VALID;
                    break;
                case INDEX(SAMEIPADDRf):
                    field_name = DRV_CFP_FIELD_SAME_IP;
                    break;
                case INDEX(L4DSTf):
                    field_name = DRV_CFP_FIELD_L4DST;
                    break;
                case INDEX(SAMEL4PORTf):
                    field_name = DRV_CFP_FIELD_SAME_L4PORT;
                    break;
                case INDEX(L4SRC_LESS_1024f):
                    field_name = DRV_CFP_FIELD_L4SRC_LESS1024;
                    break;
                case INDEX(TCP_SEQUENCE_ZEROf):
                    field_name = DRV_CFP_FIELD_TCP_SEQ_ZERO;
                    break;
                case INDEX(TCP_FLAGf):
                    field_name = DRV_CFP_FIELD_TCP_FLAG;
                    break;
                case INDEX(UDF2_VLDf):
                    field_name = DRV_CFP_FIELD_UDF2_VALID;
                    break;
                case INDEX(IP_TOSf):
                    field_name = DRV_CFP_FIELD_IP_TOS;
                    break;
                case INDEX(CURR_QUOTAf):
                    field_name = DRV_CFP_FIELD_CURR_QUOTA;
                    break;
                case INDEX(RATE_REFRESH_ENf):
                    field_name = DRV_CFP_FIELD_RATE_REFRESH_EN;
                    break;
                case INDEX(REF_CAPf):
                    field_name = DRV_CFP_FIELD_REF_CAP;
                    break;
                case INDEX(TOKEN_NUMf):
                    field_name = DRV_CFP_FIELD_RATE;
                    break;
                case INDEX(IN_BAND_CNTf):
                    field_name = DRV_CFP_FIELD_IB_CNT;
                    break;
                case INDEX(OUT_BAND_CNTf):
                    field_name = DRV_CFP_FIELD_OB_CNT;
                    break;
                /* Add for BCM5395 memory field */
                case INDEX(SRC_PMAPf):
                    field_name = DRV_CFP_FIELD_IN_PBMP;
                    break;
                case INDEX(IP_FRAGf):
                    field_name = DRV_CFP_FIELD_IP_FRAG;
                    break;
                case INDEX(NON_FIRST_FRAGf):
                    field_name = DRV_CFP_FIELD_IP_NON_FIRST_FRAG;
                    break;
                default :                    
                    return -1;
            }
    } else {
        switch(mem_field) {
            case INDEX(VID_Rf):
                field_name = DRV_MEM_FIELD_VLANID;
                break;
            case INDEX(PORTID_Rf):
                field_name = DRV_MEM_FIELD_SRC_PORT;
                break;
            case INDEX(MARL_PBMP_IDXf):
                field_name = DRV_MEM_FIELD_DEST_BITMAP;
                break;
            case INDEX(PRIORITY_Rf):
                field_name = DRV_MEM_FIELD_PRIORITY;
                break;
            case INDEX(AGEf):
                field_name = DRV_MEM_FIELD_AGE;
                break;
            case INDEX(STATICf):
                field_name = DRV_MEM_FIELD_STATIC;
                break;
            case INDEX(VALID_Rf):
                field_name = DRV_MEM_FIELD_VALID;
                break;
            case INDEX(MSPT_IDf):
                field_name = DRV_MEM_FIELD_SPT_GROUP_ID;
                break;
            case INDEX(UNTAG_MAPf):
            case INDEX(V_UNTAG_MAPf):
                field_name = DRV_MEM_FIELD_OUTPUT_UNTAG;
                break;
            case INDEX(FORWARD_MAPf):
                field_name = DRV_MEM_FIELD_PORT_BITMAP;
                break;
            case INDEX(MACADDRf):
                field_name = DRV_MEM_FIELD_MAC;
                break;
            case INDEX(DIS_LRNf):
                field_name = DRV_MEM_FIELD_FWD_MODE;
                break;
            case INDEX(DIR_FWDf):
                field_name = DRV_MEM_FIELD_DIR_FWD;
                break;
            case INDEX(ULF_DROPf):
                field_name = DRV_MEM_FIELD_UCAST_DROP;
                break;
            case INDEX(MLF_DROPf):
                field_name = DRV_MEM_FIELD_MCAST_DROP;
                break;
            case INDEX(ISO_MAPf):
                field_name = DRV_MEM_FIELD_ISO_MAP;
                break;
            case INDEX(OUTER_OPf):
                field_name = DRV_MEM_FIELD_OUTER_OP;
                break;
            case INDEX(OUTER_VIDf):
                field_name = DRV_MEM_FIELD_OUTER_VID;
                break;
            case INDEX(INNER_OPf):
                field_name = DRV_MEM_FIELD_INNER_OP;
                break;
            case INDEX(INNER_VIDf):
                field_name = DRV_MEM_FIELD_INNER_VID;
                break;
            default:
                if ((mem_field >= INDEX(MSP_TREE_PORT0f)) && \
                           (mem_field <= INDEX(MSP_TREE_PORT26f))) {
                    /* fld->field falls in port0 to port26 */
                    field_name = DRV_MEM_FIELD_MSTP_PORTST;
                } else {
                    return -1;
                }
            }
        }

    *drv_field = field_name;
    return SOC_E_NONE;

}
#endif /* !SOC_NO_NAME*/
/*
 * modify_mem_fields
 *
 *   Verify similar to modify_reg_fields (see reg.c) but works on
 *   memory table entries instead of register values.  Handles fields
 *   of any length.
 *
 *   "mod_value" is the raw data of the entry. "mod_str" is the string 
 *   containing field name and the desired field vlaue.
 *   Either mod_value or mod_str can be assigned in one function call.
 *
 *   Values may be specified with optional increment or decrement
 *   amounts; for example, a MAC address could be 0x1234+2 or 0x1234-1
 *   to specify an increment of +2 or -1, respectively.
 *
 *   If incr is FALSE, the increment is ignored and the plain value is
 *   stored in the field (e.g. 0x1234).
 *
 *   If incr is TRUE, the value portion is ignored.  Instead, the
 *   increment value is added to the existing value of the field.  The
 *   field value wraps around on overflow.
 *
 *   Returns -1 on failure, 0 on success.
 */
static int
modify_mem_fields(int unit, soc_mem_t mem, uint32 *entry,
          uint32 *mod_value, char *mod, int incr)
{
#ifndef SOC_NO_NAMES
    soc_field_info_t *fld;
    char *fmod, *fval, *s;
    char *modstr = NULL;
    uint32 fvalue[SOC_ROBO_MAX_MEM_FIELD_WORDS];
    uint32 fincr[SOC_ROBO_MAX_MEM_FIELD_WORDS];
    int i;
    int entry_dw;
    soc_mem_info_t *m = &SOC_MEM_INFO(unit, mem);
    int table_name;
    int field_name;
    uint32 entry_value[SOC_ROBO_MAX_MEM_WORDS];
    uint8 mem_mapping=FALSE;
    int rv = SOC_E_UNAVAIL;
    char *tokstr=NULL;

    table_name=-1;

    if (mod_value) {
        memcpy(entry_value, mod_value, sizeof(uint32)*SOC_ROBO_MAX_MEM_WORDS);
        rv = DRV_MEM_WRITE(unit, mem, *entry, 1, entry_value);       
        if (SOC_FAILURE(rv)){
            if (rv == SOC_E_PARAM){
                if(_soc_mem_id_map(unit, mem, &table_name) == SOC_E_NONE) {
                    rv = DRV_MEM_WRITE(unit, table_name, *entry, 1, entry_value);                    
                    SOC_IF_ERROR_RETURN(rv);
                    mem_mapping = TRUE;
                }
            }
        }        
        return 0;
    }
    
    entry_dw = BYTES2WORDS(m->bytes);

    if ((modstr = sal_alloc(ARGS_BUFFER, "modify_mem")) == NULL) {
        cli_out("modify_mem_fields: Out of memory\n");
        return CMD_FAIL;
    }

    strncpy(modstr, mod, ARGS_BUFFER);/* Don't destroy input string */
    modstr[ARGS_BUFFER - 1] = 0;
    mod = modstr;


    while ((fmod = sal_strtok_r(mod, ",", &tokstr)) != 0) {
        mod = NULL;         /* Pass strtok NULL next time */
        fval = strchr(fmod, '=');
        if (fval != NULL) {     /* Point fval to arg, NULL if none */
            *fval++ = 0;        /* Now fmod holds only field name. */
        }
        if (fmod[0] == 0) {
            cli_out("Null field name\n");
            goto error_return;
        }
        if (!sal_strcasecmp(fmod, "clear")) {
            memset(entry, 0, entry_dw * sizeof (*entry));
            continue;
        }
        for (fld = &m->fields[0]; fld < &m->fields[m->nFields]; fld++) {
            if (!sal_strcasecmp(fmod, soc_robo_fieldnames[fld->field])) {
                break;
            }
        }
    
        if ((fld == &m->fields[m->nFields])) {
            cli_out("No such field \"%s\" in memory \"%s\".\n",
                    fmod, SOC_ROBO_MEM_UFNAME(unit, mem));
            goto error_return;        }
        if (!fval) {
            cli_out("Missing %d-bit value to assign to \"%s\" field \"%s\".\n",
                    fld->len, SOC_ROBO_MEM_UFNAME(unit, mem), soc_robo_fieldnames[fld->field]);
            goto error_return;
        }
    
        rv = DRV_MEM_READ(unit, mem, *entry, 1, entry_value);
        if (SOC_FAILURE(rv)){
            if (rv == SOC_E_PARAM){
                if (_soc_mem_id_map(unit, mem, &table_name) == SOC_E_NONE) {
                    rv = DRV_MEM_READ(unit, table_name, *entry, 1, entry_value);;                    
                    if (SOC_FAILURE(rv)){
                        goto error_return;
                    }
                    mem_mapping = TRUE;                    
                }
            } else {
            goto error_return;
            }
        }        

        if (mem_mapping) {
            if (table_name == DRV_MEM_ARL) {
                if (!sal_strcasecmp(fmod, "macaddr") ||
                    !sal_strcasecmp(fmod, "macaddress") ||
                    !sal_strcasecmp(fmod,"vid")) {
                    cli_out("MAC or VID in ARL memory are not allowed to be modified\n");
                    goto error_return;
                }
            }            
            rv = _soc_mem_field_map(unit, table_name, fld->field, &field_name);
            if (SOC_FAILURE(rv)){
                goto error_return;
            }
        } else {
            if ((mem == INDEX(L2_ARLm)) || (mem == INDEX(L2_MARLm)) || 
                (mem == INDEX(L2_ARL_SWm)) || (mem == INDEX(L2_MARL_SWm))) {
                if ((fld->field ==  INDEX(VIDf)) ||(fld->field ==  INDEX(MACADDRf))) {
                    cli_out("MAC or VID in ARL memory are not allowed to be modified\n");
                    goto error_return;
                }
            }

            field_name = fld->field;
        }

        s = strchr(fval, '+');
        if (s == NULL) {
            s = strchr(fval, '-');
        }
        if (s == fval) {
            s = NULL;
        }
        if (incr) {
            if (s != NULL) {
                parse_long_integer(fincr, SOC_ROBO_MAX_MEM_FIELD_WORDS,
                           s[1] ? &s[1] : "1");
                if (*s == '-') {
                    neg_long_integer(fincr, SOC_ROBO_MAX_MEM_FIELD_WORDS);
                }
                if (fld->len & 31) {
                    /* Proper treatment of sign extension */
                    fincr[fld->len / 32] &= ~(0xffffffff << (fld->len & 31));
                }
                rv = DRV_MEM_FIELD_GET(unit, mem, field_name, entry,fvalue);
                if (SOC_FAILURE(rv)){
                    goto error_return;
                }

                add_long_integer(fvalue, fincr, SOC_ROBO_MAX_MEM_FIELD_WORDS);
                if (fld->len & 31) {
                    /* Proper treatment of sign extension */
                    fvalue[fld->len / 32] &= ~(0xffffffff << (fld->len & 31));
                }
                rv = DRV_MEM_READ(unit, mem, *entry, 1, entry_value);
                if (SOC_FAILURE(rv)){
                    goto error_return;
                }                
                rv = DRV_MEM_FIELD_SET(unit, mem, fld->field, entry_value, fvalue);
                if (SOC_FAILURE(rv)){
                    goto error_return;
                }
                rv = DRV_MEM_WRITE(unit, mem, *entry, 1, entry_value);
                if (SOC_FAILURE(rv)){
                    goto error_return;
                }

            }
        } else {
            if (s != NULL) {
                *s = 0;
            }

            parse_long_integer(fvalue, SOC_ROBO_MAX_MEM_FIELD_WORDS, fval);
            for (i = fld->len; i < SOC_ROBO_MAX_MEM_FIELD_BITS; i++) {
                if (fvalue[i / 32] & 1 << (i & 31)) {
                    cli_out("Value \"%s\" too large for "
                            "%d-bit field \"%s\".\n",
                            fval, fld->len, soc_robo_fieldnames[fld->field]);
                    goto error_return;
                }
            }
            rv = DRV_MEM_READ(unit, mem, *entry, 1, entry_value);
            if (SOC_FAILURE(rv)){
                goto error_return;
            }
            
            rv = DRV_MEM_FIELD_SET(unit, mem, fld->field, entry_value, fvalue);
            if (SOC_FAILURE(rv)){
                goto error_return;
            }

            rv = DRV_MEM_WRITE(unit, mem, *entry, 1, entry_value);
            if (SOC_FAILURE(rv)){
                goto error_return;
            }

        }
    }
    
    sal_free(modstr);
    return 0;
error_return:
    sal_free(modstr);
    return -1;
#else
    /* This function relies on the existance of soc_robo_fieldnames[] */
    return -1;
#endif /* !SOC_NO_NAME*/
}

static int
parse_dwords(int count, uint32 *dw, args_t *a)
{
    char *s;
    int i;

    for (i = 0; i < count; i++) {
        if ((s = ARG_GET(a)) == NULL) {
            cli_out("Not enough data values (have %d, need %d)\n",
                    i, count);
            return -1;
        }

        dw[i] = parse_integer(s);
    }

    if (ARG_CNT(a) > 0) {
        cli_out("Ignoring extra data on command line "
                "(only %d words needed)\n",
                count);
    }

    return 0;
}

cmd_result_t
cmd_robo_mem_write(int unit, args_t *a)
{
    int i, start, count, copyno;
    char *tab, *idx, *cnt, *s;
    soc_mem_t mem;
    uint32 entry[SOC_ROBO_MAX_MEM_WORDS];
    int entry_dw;
    char copyno_str[8];
    int update;
    int rv = CMD_FAIL;
    char valstr[1024];
    uint32 index;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        goto done;
    }

    tab = ARG_GET(a);

    if (!tab) {
        return CMD_USAGE;
    }

#if (!defined(__KERNEL__)) && (!defined(NO_SAL_APPL))
#if defined(BCM_53125) || defined(BCM_53128)
    if ( !sal_strcasecmp(tab, "int8051ram")) {
        /* write to 8051 image */
        uint32 reg_val, temp = 0;
        FILE	*fp; 
        char buf_8051[8];
        int in_char;
        int total_size = 0;

        if (!(SOC_IS_STARFIGHTER(unit) || SOC_IS_BLACKBIRD2(unit))) {
            goto done;
        }

        /* write the image to 8051 RAM */
        cli_out("Write the 8051 image file, 8051gm.bin, to 8051\n");
        if ((fp = sal_fopen("8051gm.bin", "r")) == NULL) {
#if defined(BROADCOM_DEBUG)
	    sal_printf("Write 8051 image: cannot read file: "
		   "variables not loaded\n");
#endif /* BROADCOM_DEBUG */
	    return CMD_FAIL;
        }


        /* Stop 8051 running */
        
        if (REG_READ_IN_CPU_CTRLr(unit, &reg_val) < 0) {
            sal_fclose(fp);
            return rv;
        }
        temp = 1;
        soc_IN_CPU_CTRLr_field_set(unit, &reg_val, IN_CPU_RSTf, &temp);
        soc_IN_CPU_CTRLr_field_set(unit, &reg_val, IN_CPU_CODE_SELf, &temp);
        temp = 15;
        soc_IN_CPU_CTRLr_field_set(unit, &reg_val, IN_RAM_CODE_SIZEf, &temp);
        if (REG_WRITE_IN_CPU_CTRLr(unit, &reg_val) < 0) {
            sal_fclose(fp);
            return rv;
        }

        /* Disable the MAC low power mode */        
        if (bcm_switch_control_set(unit, bcmSwitchMacLowPower, FALSE) < 0) {
            sal_fclose(fp);            
            return rv;        
        }

        /* write 8051 image */
        total_size = 0;
        do {
            in_char= fgetc(fp);
            if (in_char < 0) {
                DRV_MEM_WRITE(unit, DRV_MEM_8051_RAM, (total_size- 1)/8, 1, (uint32 *)&buf_8051[0]);
                break;
            }
#ifdef BE_HOST            
            buf_8051[7-(total_size%8)] = in_char;
#else /* !BE_HOST */
            buf_8051[(total_size%8)] = in_char;
#endif /* BE_HOST */
            total_size++;
            if ((total_size % 8) == 0) {
                DRV_MEM_WRITE(unit, DRV_MEM_8051_RAM, (total_size- 1)/8, 1, (uint32 *)&buf_8051[0]);
            }
        } while (in_char >= 0);       

       /* Start 8051 runing */
        temp = ((total_size-1)/1024) + 1;
        soc_IN_CPU_CTRLr_field_set(unit, &reg_val, IN_RAM_CODE_SIZEf, &temp);
        if (REG_WRITE_IN_CPU_CTRLr(unit, &reg_val) < 0) {            
            sal_fclose(fp);            
            return rv;        
        }
        
        temp = 0;
        soc_IN_CPU_CTRLr_field_set(unit, &reg_val, IN_CPU_RSTf, &temp);
        if (REG_WRITE_IN_CPU_CTRLr(unit, &reg_val) < 0) {
            sal_fclose(fp);
            return rv;
        }

        sal_fclose(fp);
        /* Start 8051 runing */
	goto done;
    }
#endif /* BCM_53125 BCM_53128 */    
#endif /* (!defined(__KERNEL__)) && (!defined(NO_SAL_APPL)) */


    idx = ARG_GET(a);
    cnt = ARG_GET(a);
    s = ARG_GET(a);

    /* you will need at least one value and all the args .. */
    if (!idx || !cnt || !s || !isint(cnt)) {
        return CMD_USAGE;
    }

    if (parse_memory_name(unit, &mem, tab, &copyno, 0) < 0) {
        cli_out("ERROR: unknown table \"%s\"\n",tab);
        goto done;
    }

    if (!SOC_IS_ROBO(unit) || !SOC_MEM_IS_VALID(unit, mem)) {
        cli_out("Error: Memory %d not valid for chip %s.\n",
                (int)mem, SOC_UNIT_NAME(unit));
        goto done;
    }

    if (soc_mem_is_readonly(unit, mem)) {
        cli_out("Error: Table %s is read-only\n",
                SOC_ROBO_MEM_UFNAME(unit, mem));
        goto done;
    }

    start = parse_memory_index(unit, mem, idx);
    count = parse_integer(cnt);

    if (copyno == COPYNO_ALL) {
        copyno_str[0] = 0;
    } else {
        sal_snprintf(copyno_str, sizeof(copyno_str), ".%d", copyno);
    }

    entry_dw = soc_mem_entry_words(unit, mem);

    /*
     * If a list of fields were specified, generate the entry from them.
     * Otherwise, generate it by reading raw dwords from command line.
     */

    if (!isint(s)) {
        /* List of fields */
        collect_comma_args(a, valstr, s);

        memset(entry, 0, sizeof (entry));

        update = TRUE;
    } else {
        /* List of numeric values */

        ARG_PREV(a);

        if (parse_dwords(entry_dw, entry, a) < 0) {
            goto done;
        }

        update = FALSE;
    }

    if (bsl_check(bslLayerAppl, bslSourceSocmem, bslSeverityNormal, unit)) {
        cli_out("WRITE[%s%s], DATA:", 
                SOC_ROBO_MEM_UFNAME(unit, mem),
                copyno_str);
        for (i = 0; i < entry_dw; i++) {
            cli_out(" 0x%x", entry[i]);
        }
        cli_out("\n");
    }

    /*
     * Take lock to ensure atomic modification of memory.
     */

    soc_mem_lock(unit, mem);

    /*
     * Created entry, now write it
     */
    for (index = start; index < start + count; index++) {
        if (update) {
            modify_mem_fields(unit, mem, \
                              (uint32 *)&index, NULL, valstr, FALSE);
        }
        else {
            modify_mem_fields(unit, mem, \
                              (uint32 *)&index, entry, NULL, FALSE);
        }
    }
    soc_mem_unlock(unit, mem);

    rv = CMD_OK;

 done:
    return rv;
}


/*
 * Modify the fields of a table entry
 */

cmd_result_t
cmd_robo_mem_modify(int unit, args_t *a)
{
    int start, count, copyno;
    uint32 index;
    char *tab, *idx, *cnt, *s;
    soc_mem_t mem;
    char valstr[1024];
    int rv;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    tab = ARG_GET(a);
    idx = ARG_GET(a);
    cnt = ARG_GET(a);
    s = ARG_GET(a);

    /* you will need at least one dword and all the args .. */
    if (!tab || !idx || !cnt || !s || !isint(cnt)) {
        return CMD_USAGE;
    }

    collect_comma_args(a, valstr, s);

    if (parse_memory_name(unit, &mem, tab, &copyno, 0) < 0) {
        cli_out("ERROR: unknown table \"%s\"\n",tab);
        return CMD_FAIL;
    }

    if (!SOC_IS_ROBO(unit) || !SOC_MEM_IS_VALID(unit, mem)) {
        cli_out("Error: Memory %d not valid for chip %s.\n",
                (int)mem, SOC_UNIT_NAME(unit));
        return CMD_FAIL;
    }

    if (soc_mem_is_readonly(unit, mem)) {
        cli_out("ERROR: Table %s is read-only\n", SOC_ROBO_MEM_UFNAME(unit, mem));
        return CMD_FAIL;
    }
    start = parse_memory_index(unit, mem, idx);
    count = parse_integer(cnt);

    /*
     * Take lock to ensure atomic modification of memory.
     */

    soc_mem_lock(unit, mem);

    rv = CMD_OK;
    for (index = start; index < start + count; index++) {
        modify_mem_fields(unit, mem, (uint32 *)&index, NULL, valstr, FALSE);
    }
    soc_mem_unlock(unit, mem);

    return rv;
}

/*
 * Print out help for all the memory types.
 *
 * If substr_match is non-NULL, only prints out info for memories with
 * this substring in their name or user-friendly name.
 */

static void
do_help_socmem(int unit, char *substr_match)
{
    soc_mem_t mem;
    char buf[80];
    int i, copies;
    int found = 0;

    for (mem = 0; mem < NUM_SOC_MEM; mem++) {
        if (!SOC_IS_ROBO(unit) || !SOC_MEM_IS_VALID(unit, mem)) {
            continue;
        }

        if (substr_match != NULL &&
            strcaseindex(SOC_ROBO_MEM_NAME(unit, mem), substr_match) == NULL &&
            strcaseindex(SOC_ROBO_MEM_UFNAME(unit, mem), substr_match) == NULL) {
            continue;
        }

        strncpy(buf, SOC_ROBO_MEM_UFNAME(unit, mem), sizeof(buf)-1);

        copies = 0;
        SOC_MEM_BLOCK_ITER(unit, mem, i) {
            copies += 1;
        }
        if (copies > 1) {
            sal_snprintf(buf + strlen(buf), (sizeof(buf) - strlen(buf)), "/%d", copies);
        }
    
        if (!found) {
            cli_out("   Flags    %-16s  MIN -  MAX   Words  %s\n",
                    "Name/Copies", "Description");
            found = 1;
        }

        cli_out("  %c%c%c%c%c  %-16s %4d - %5d  %2d     %s\n",
                soc_mem_is_readonly(unit, mem) ? 'r' : '-',
                soc_mem_is_debug(unit, mem) ? 'd' : '-',
                soc_mem_is_sorted(unit, mem) ? 's' :
                soc_mem_is_hashed(unit, mem) ? 'h' : '-',
                soc_mem_is_cam(unit, mem) ? 'A' : '-',
                soc_mem_is_cachable(unit, mem) ? 'C' : '-',
                buf,
                soc_robo_mem_index_min(unit, mem),
                soc_robo_mem_index_max(unit, mem),
                soc_mem_entry_words(unit, mem),
                SOC_ROBO_MEM_DESC(unit, mem));
    }

    if (found) {
        cli_out("Flags: (r)eadonly, (d)ebug, (s)orted, (h)ashed\n"
                "       C(A)M, (c)bp, (b)ist-able, (C)achable\n");
    } else if (substr_match != NULL) {
        cli_out("No memory found with the substring '%s' in its name.\n",
                substr_match);
    }
}

/*
 * List the tables, or fields of a table entry
 */

cmd_result_t
cmd_robo_mem_list(int unit, args_t *a)
{
    soc_mem_info_t *m;
    soc_field_info_t *fld;
    char *tab, *s;
    soc_mem_t mem;
    uint32 entry[SOC_ROBO_MAX_MEM_WORDS];
    uint32 mask[SOC_ROBO_MAX_MEM_WORDS];
    int have_entry, i, dw, copyno;
    uint32 flags;
    int minidx, maxidx;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    tab = ARG_GET(a);

    if (!tab) {
        do_help_socmem(unit, NULL);
        return CMD_OK;
    }

    if (parse_memory_name(unit, &mem, tab, &copyno, 0) < 0) {
        if ((s = strchr(tab, '.')) != NULL) {
            *s = 0;
        }
        do_help_socmem(unit, tab);
        return CMD_OK;
    }

    if ((!SOC_MEM_IS_VALID(unit, mem)) || (!SOC_IS_ROBO(unit))) {
        cli_out("ERROR: Memory \"%s\" not valid for this unit\n", tab);
        return CMD_FAIL;
    }

    m = &SOC_MEM_INFO(unit, mem);
    flags = m->flags;

    dw = BYTES2WORDS(m->bytes);

    if ((s = ARG_GET(a)) == 0) {
        have_entry = 0;
    } else {
        for (i = 0; i < dw; i++) {
            if (s == 0) {
                cli_out("Not enough data specified (%d words needed)\n", dw);
                return CMD_FAIL;
            }
            entry[i] = parse_integer(s);
            s = ARG_GET(a);
        }
        if (s) {
            cli_out("Extra data specified (ignored)\n");
        }
        have_entry = 1;
    }

    cli_out("Memory: %s", SOC_ROBO_MEM_UFNAME(unit, mem));
    s = SOC_ROBO_MEM_UFALIAS(unit, mem);
    if (s && *s && strcmp(SOC_ROBO_MEM_UFNAME(unit, mem), s) != 0) {
        cli_out(" alias %s", s);
    }

    cli_out("Flags:");
    if (flags & SOC_MEM_FLAG_READONLY) {
        cli_out(" read-only");
    }
    if (flags & SOC_MEM_FLAG_VALID) {
        cli_out(" valid");
    }
    if (flags & SOC_MEM_FLAG_DEBUG) {
        cli_out(" debug");
    }
    if (flags & SOC_MEM_FLAG_SORTED) {
        cli_out(" sorted");
    }
    if (flags & SOC_MEM_FLAG_CACHABLE) {
        cli_out(" cachable");
    }
    if (flags & SOC_MEM_FLAG_UNIFIED) {
        cli_out(" unified");
    }
    if (flags & SOC_MEM_FLAG_HASHED) {
        cli_out(" hashed");
    }
    if (flags & SOC_MEM_FLAG_WORDADR) {
        cli_out(" word-addressed");
    }
    if (flags & SOC_MEM_FLAG_BE) {
        cli_out(" big-endian");
    }
    cli_out("\n");

    minidx = soc_robo_mem_index_min(unit, mem);
    maxidx = soc_robo_mem_index_max(unit, mem);
    cli_out("Entries: %d with indices %d-%d (0x%x-0x%x)",
            maxidx - minidx + 1,
            minidx,
            maxidx,
            minidx,
            maxidx);
    cli_out(", each %d bytes %d words\n", m->bytes, dw);

    cli_out("Entry mask:");
    soc_mem_datamask_get(unit, mem, mask);
    for (i = 0; i < dw; i++) {
        if (mask[i] == 0xffffffff) {
            cli_out(" -1");
        } else if (mask[i] == 0) {
            cli_out(" 0");
        } else {
            cli_out(" 0x%08x", mask[i]);
        }
    }
    cli_out("\n");

    s = SOC_ROBO_MEM_DESC(unit, mem);
    
    if (s && *s) {
        cli_out("Description: %s\n", s);
    }

    for (fld = &m->fields[m->nFields - 1]; fld >= &m->fields[0]; fld--) {
#ifndef SOC_NO_NAMES
        cli_out("  %s<%d", soc_robo_fieldnames[fld->field], fld->bp + fld->len - 1);
#else
        /* Print FIELD_XX when there are no available field names */
        cli_out("  FIELD_%02d<%d", fld->field, fld->bp + fld->len - 1);

#endif /* !SOC_NO_NAME*/
        if (fld->len > 1) {
            cli_out(":%d", fld->bp);
        }
        if (have_entry) {
            uint32 fval[SOC_ROBO_MAX_MEM_FIELD_WORDS];
            char tmp[132];
            int rv;
        
            memset(fval, 0, sizeof (fval));
            rv = DRV_MEM_FIELD_GET(unit, mem, fld->field, entry, fval);
            if (rv < 0){
             return CMD_FAIL;
            }                                                        
            format_long_integer(tmp, fval, SOC_ROBO_MAX_MEM_FIELD_WORDS);
            cli_out("> = %s\n", tmp);
        } else {
            cli_out(">\n");
        }
    }

    return CMD_OK;
}

/*
 * Turn on/off software caching of hardware tables
 */


cmd_result_t
mem_robo_cache(int unit, args_t *a)
{
    soc_mem_t       mem;
    int         copyno;
    char        *c;
    uint32  enable;
    uint32      table_name;
    int         rv = CMD_OK;

    if (!sh_check_attached(ARG_CMD(a), unit)) {
        return CMD_FAIL;
    }

    if (ARG_CNT(a) == 0) {
        cli_out("Memory table cache status:\n");
        (DRV_SERVICES(unit)->mem_cache_get)(unit, DRV_MEM_VLAN, &enable);
        cli_out("VLAN    : cache %s.\n", enable ? "Enabled" : "Disabled");
        (DRV_SERVICES(unit)->mem_cache_get)(unit, DRV_MEM_SECMAC, &enable);
        cli_out("SEC_MAC : cache %s.\n", enable ? "Enabled" : "Disabled");
    
        return CMD_OK;
    }

    while ((c = ARG_GET(a)) != 0) {
        switch (*c) {
            case '+':
                enable = 1;
                c++;
                break;
            case '-':
                enable = 0;
                c++;
                break;
            default:
                enable = 1;
                break;
        }

        if (parse_memory_name(unit, &mem, c, &copyno, 0) < 0) {
            cli_out("%s: Unknown table \"%s\"\n", ARG_CMD(a), c);
            return CMD_FAIL;
        }
    
        if (!SOC_IS_ROBO(unit) || !SOC_MEM_IS_VALID(unit, mem)) {
            cli_out("Error: Memory %d not valid for chip %s.\n",
                    (int)mem, SOC_UNIT_NAME(unit));
            continue;
        }

        switch(mem) {
            case INDEX(VLAN_1Qm):
                table_name = DRV_MEM_VLAN;
                break;
            default:
                cli_out("%s: Memory %s is not cachable\n",
                        ARG_CMD(a), SOC_ROBO_MEM_UFNAME(unit, mem));
                return CMD_FAIL;
        }
        rv = (DRV_SERVICES(unit)->mem_cache_set)(unit, table_name, enable);
        if (rv < 0) {
            return rv;
        }
    }

    return CMD_OK;
}

