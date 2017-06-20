/*
 * $Id: field.c,v 1.11 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#include <sal/core/libc.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>
#include <sal/core/libc.h>

#include <soc/debug.h>

#include <appl/diag/shell.h>
#include <appl/diag/system.h>
#include <appl/diag/parse.h>
#include <appl/diag/diag.h>
#include <appl/diag/dport.h>

#include <bcm/init.h>
#include <bcm/field.h>
#include <bcm/error.h>
#if defined(BCM_TK371X_SUPPORT)
#include <bcm_int/ea/tk371x/field.h>
#endif

#if defined (BCM_FIELD_SUPPORT)
/*
 * Macro:
 *     FP_CHECK_RETURN
 * Purpose:
 *     Check the return value from an API call. Output either a failed
 *     message or okay along with the function name.
 */
#define FP_CHECK_RETURN(unit, retval, funct_name)                  \
    if (BCM_FAILURE(retval)) {                                     \
        LOG_ERROR(BSL_LS_BCM_FP, \
                  (BSL_META_U(unit,                                    \
                              "FP(unit %d) Error: %s() failed: %s\n"), \
                   unit,                                               \
                   funct_name, bcm_errmsg(retval)));                   \
        return CMD_FAIL; \
    } else {                                                       \
            LOG_VERBOSE(BSL_LS_BCM_FP,                             \
                        (BSL_META_U(unit,                               \
                                    "FP(unit %d) verb: %s() success \n"), \
                         unit, funct_name));                            \
    }

/*
 * Macro:
 *     FP_GET_NUMB
 * Purpose:
 *     Get a numerical value from stdin.
 */
#define FP_GET_NUMB(numb, str, args) \
    if (((str) = ARG_GET(args)) == NULL) { \
        return CMD_USAGE; \
    } \
    (numb) = parse_integer(str);

/*
 * Macro:
 *     FP_GET_MAC
 * Purpose:
 *     Get a mac address from stdin.
 */
#define FP_GET_MAC(_mac, _str, _args) \
    if (((_str) = ARG_GET(_args)) == NULL) { \
        return CMD_FAIL; \
    } \
    if(parse_macaddr((_str), (_mac))) { \
        return CMD_FAIL; \
    }

/*
 * Marker for last element in qualification table
 */
#define FP_TABLE_END_STR "tbl_end"

#define FP_STAT_STR_SZ 256
#define FP_LINE_SZ 72
/*
 * Macro:
 *     FP_GET_PORT
 * Purpose:
 *     Get a numerical value from stdin.
 */
#define FP_GET_PORT(_unit, _port, _str, _args)                   \
    if (((_str) = ARG_GET(_args)) == NULL) {                     \
        return CMD_USAGE;                                        \
    }                                                            \
    if (parse_bcm_port((_unit), (_str), &(_port)) < 0) {             \
        LOG_ERROR(BSL_LS_BCM_FP,                                        \
                  (BSL_META_U(_unit,                                    \
                              "FP(unit %d) Error: invalid port string: \"%s\"\n"), \
                   _unit, _str));                                       \
       return CMD_FAIL;                                          \
    }
#if defined(BCM_TK371X_SUPPORT)

#define FP_BCM_FIELD_QUALIFY_STR_LEN	19
/*
 * qset reference functions declare
 */
STATIC cmd_result_t tk371x_fp_qset_add(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_qset_set(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_qset_show(bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_qset(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_qset(int unit, args_t *args, bcm_field_qset_t *qset);

/*
 * group reference functions declare
 */
STATIC cmd_result_t tk371x_fp_group(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_group_create(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_group_destroy(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_group_get(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_group_set(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC cmd_result_t tk371x_fp_group_status_get(int unit, args_t *args);


/*
 * action reference function declare
 */
STATIC cmd_result_t tk371x_fp_action(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_action_add(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_action_get(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_action_remove(int unit, args_t *args);

/*
 * Entry reference functions declare
 */
STATIC cmd_result_t tk371x_fp_entry_create(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_copy(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_destroy(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_install(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_reinstall(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_remove(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_prio(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_entry_inport(int unit, args_t *args);


/*
 * Range reference functions declare
 */
STATIC cmd_result_t tk371x_fp_range_group_create(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_range_create(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_range_get(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_range_destroy(int unit, args_t *args);
STATIC cmd_result_t tk371x_fp_range(int unit, args_t *args);

/*
 * qualify reference functions declare
 */
STATIC int tk371x_fp_qual_mac(int unit, bcm_field_entry_t eid, args_t *args,
            int func(int, bcm_field_entry_t, bcm_mac_t, bcm_mac_t),
            char *qual_str);
STATIC int tk371x_fp_qual_mac_help(const char *prefix,
		const char *qual_str, int width_col1);
STATIC int tk371x_fp_list_quals(int unit);
STATIC int tk371x_fp_qual_modport_help(const char *prefix,
		const char *qual_str, int width_col1);
STATIC int tk371x_fp_qual_8(int unit, bcm_field_entry_t eid, args_t *args,
          int func(int, bcm_field_entry_t, uint8, uint8),
          char *qual_str);
STATIC int tk371x_fp_qual_ip(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, bcm_ip_t, bcm_ip_t),
           char *qual_str);
STATIC int tk371x_fp_qual_ip_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip_t*, bcm_ip_t*),
            char *qual_str);
STATIC int tk371x_fp_qual_32(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint32, uint32),
           char *qual_str);
STATIC int tk371x_fp_qual_16(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint16, uint16),
           char *qual_str);
STATIC int tk371x_fp_qual_vlan(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_vlan_t, bcm_vlan_t),
             char *qual_str);
STATIC int tk371x_fp_qual_l4port(int unit, bcm_field_entry_t eid, args_t *args,
               int func(int, bcm_field_entry_t, bcm_l4_port_t, bcm_l4_port_t),
               char *qual_str);
STATIC int tk371x_fp_qual_ip6(int unit, bcm_field_entry_t eid, args_t *args,
            int func(int, bcm_field_entry_t, bcm_ip6_t, bcm_ip6_t),
            char *qual_str);
STATIC int tk371x_fp_qual_ip6_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip6_t*, bcm_ip6_t*),
            char *qual_str);
STATIC int tk371x_fp_qual_rangecheck(int unit, bcm_field_entry_t eid, args_t *args,
                   int func(int, bcm_field_entry_t, bcm_field_range_t, int),
                   char *qual_str);
STATIC cmd_result_t tk371x_fp_qual(int unit, args_t *args);


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC cmd_result_t
tk371x_fp_qset_add(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char                  *qual_str = NULL;
    char                  buf[BCM_FIELD_QUALIFY_WIDTH_MAX];
    bcm_field_qualify_t   qual;

    if ((qual_str = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    if (isint(qual_str)) {
        qual = parse_integer(qual_str);
    } else {
        qual = parse_field_qualifier(qual_str);

        if (qual == bcmFieldQualifyCount) {
            cli_out("FP(unit %d) Error: Unknown qualifier: %s\n",
                    unit, qual_str);
            return CMD_FAIL;
        }
    }

    BCM_FIELD_QSET_ADD(*qset, qual);
    LOG_CLI(("BCM_FIELD_QSET_ADD(%s) okay\n",
           format_field_qualifier(buf, qual, 1)));
    return CMD_OK;
}


STATIC cmd_result_t
tk371x_fp_qset_set(int unit, args_t *args, bcm_field_qset_t *qset){
    char                  *qual_str = NULL;
    char                  *buf;

    if ((qual_str = ARG_GET(args)) == NULL) {
        LOG_CLI(("tk371x_fp_qset_set %s\n", qual_str));
        return CMD_USAGE;
    }

    BCM_FIELD_QSET_INIT(*qset);
    buf = (char *) sal_alloc((BCM_FIELD_QSET_WIDTH_MAX * sizeof(char)),
                                   "qset string");
    if (NULL == buf) {
        return (BCM_E_MEMORY);
    }

    if (parse_field_qset(qual_str, qset) == 0) {
        sal_free(buf);
        return CMD_FAIL;
    }
    LOG_CLI(("fp_qset_set(%s) okay\n", format_field_qset(buf, *qset, " ")));
    sal_free(buf);
    return CMD_OK;
}

extern void _bcm_tk371x_field_qset_dump(
		char *prefix, bcm_field_qset_t qset, char *suffix);
STATIC cmd_result_t
tk371x_fp_qset_show(bcm_field_qset_t *qset){
    _bcm_tk371x_field_qset_dump("qset=", *qset, "\n");
	return CMD_OK;
}

/*
 * Function:
 *     tk371x_fp_qset
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC cmd_result_t
tk371x_fp_qset(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char*                 subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.1> fp qset clear */
    if(!sal_strcasecmp(subcmd, "clear")) {
        BCM_FIELD_QSET_INIT(*qset);
        LOG_CLI(("BCM_FIELD_QSET_INIT() okay\n"));
        return CMD_OK;
    }
    /* BCM.1> fp qset add ...*/
    if(!sal_strcasecmp(subcmd, "add")) {
        return tk371x_fp_qset_add(unit, args, qset);
    }
    /* BCM.1> fp qset set ...*/
    if(!sal_strcasecmp(subcmd, "set")) {
        return tk371x_fp_qset_set(unit, args, qset);
    }
    /* BCM.1> fp qset show */
    if(!sal_strcasecmp(subcmd, "show")) {
        return tk371x_fp_qset_show(qset);
    }
    return CMD_USAGE;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_mac(int unit, bcm_field_entry_t eid, args_t *args,
            int func(int, bcm_field_entry_t, bcm_mac_t, bcm_mac_t),
            char *qual_str)
{
    int                         retval = CMD_OK;
    bcm_mac_t                   data, mask;
    char                        stat_str[FP_STAT_STR_SZ];
    char                        *subcmd;
    parse_table_t               pt;

    /* Give data and mask default values. */
    sal_memset(data, 0xff, sizeof(bcm_mac_t));
    sal_memset(mask, 0xff, sizeof(bcm_mac_t));

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Data", PQ_DFL | PQ_MAC, 0,
                    (void *)&data, 0);
    parse_table_add(&pt, "Mask", PQ_DFL | PQ_MAC, 0,
                    (void *)&mask, 0);

    if (2 != (parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        /* Try to parse without key words. */
        FP_GET_MAC(data, subcmd, args);
        FP_GET_MAC(mask, subcmd, args);
    }

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(stat_str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(stat_str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}

STATIC int
tk371x_fp_qual_mac_help(const char *prefix, const char *qual_str, int width_col1)
{
    if (width_col1 < sal_strlen(qual_str)) {
        width_col1 = sal_strlen(qual_str) + 1;
    }

    /* "FieldProcessor qual <eid> XxxMac" */
    LOG_CLI(("%s%-*s%s\n", prefix, width_col1, qual_str,
           "Data=<mac> Mask=<mac>"));
    return CMD_OK;
}


STATIC int
tk371x_fp_list_quals(int unit){
    bcm_field_qualify_t qual;
    char                buf[BCM_FIELD_QUALIFY_WIDTH_MAX];
    char                *param_table[bcmFieldQualifyCount][2];
    int                 width_col1 = 20, width_col2 = 40, width_col3= 20;

    /* Fill the table with default values for param0 & param1. */
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        param_table[qual][0] = "n/a";
        param_table[qual][1] = "n/a";
    }
    param_table[bcmFieldQualifyIp6FlowLabel][0]    = "20-bit IPv6 Flow Label";
    param_table[bcmFieldQualifyIp6FlowLabel][1]    = "20-bit mask";
    /*
	param_table[bcmFieldQualifyInPort][0]          = "Single Input Port";
	param_table[bcmFieldQualifyInPort][1]          = "Port Mask";
    */
    param_table[bcmFieldQualifyEtherType][0]       = "Ethernet Type";
    param_table[bcmFieldQualifyEtherType][1]       = "16-bit mask";
    param_table[bcmFieldQualifyOuterVlanId][0]     = "Outer VLAN tag";
    param_table[bcmFieldQualifyOuterVlanId][1]     = "16-bit mask";
    param_table[bcmFieldQualifyDSCP][0]            = "UNI: IP TOS/DSCP(IPv4)";
    param_table[bcmFieldQualifyDSCP][1]            = "8-bit mask";
    param_table[bcmFieldQualifyIpProtocol][0]      = "EPON IPv4 protocol";
    param_table[bcmFieldQualifyIpProtocol][1]      = "8-bit mask";
    /*
     * bcmFieldQualifyLlidValue
     */
    param_table[bcmFieldQualifyIp6TrafficClass][0] = "(UNI):IPv6 Next Header, PON: LLID value";
    param_table[bcmFieldQualifyIp6TrafficClass][1] = "(UNI):8-bit mask, PON: LLID mask";
    param_table[bcmFieldQualifyL4SrcPort][0]       = "TCP/UDP Source port";
    param_table[bcmFieldQualifyL4SrcPort][1]       = "16-bit mask";
    param_table[bcmFieldQualifyL4DstPort][0]       = "TCP/UDP Destination port";
    param_table[bcmFieldQualifyL4DstPort][1]       = "16-bit mask";
    param_table[bcmFieldQualifyOuterVlanPri][0]    = "16-bit IEEE 802.1D User priority";
    param_table[bcmFieldQualifyOuterVlanPri][1]    = "16-bit mask";
    param_table[bcmFieldQualifyIp6NextHeader][0]   = "IPv4 Protocol,IPv6 Next Header";
    param_table[bcmFieldQualifyIp6NextHeader][1]   = "8-bit mask";
    param_table[bcmFieldQualifySrcIp][0]           = "Source IPv4 Address";
    param_table[bcmFieldQualifySrcIp][1]           = "IPv4 Address mask";
    param_table[bcmFieldQualifyDstIp][0]           = "Destination IPv4 Address";
    param_table[bcmFieldQualifyDstIp][1]           = "IPv4 Address";
    param_table[bcmFieldQualifySrcIp6Low][0]       = "Low 64-bits of Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6Low][1]       = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6Low][0]       = "Low 64-bits of Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6Low][1]       = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifySrcIp6High][0]      = "Top 64-bits of Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6High][1]      = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6High][0]      = "Top 64-bits of Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6High][1]      = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifySrcIp6][0]          = "Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6][1]          = "IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6][0]          = "Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6][1]          = "IPv6 Address mask";
    param_table[bcmFieldQualifyRangeCheck][0]      = "Range ID";
    param_table[bcmFieldQualifyRangeCheck][1]      = "Normal=0, Invert=1";
    param_table[bcmFieldQualifyLlidValue][0]       = "LLID value";
    param_table[bcmFieldQualifyLlidValue][1]       = "LLID value mask";

    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "QUALIFIER", width_col2, "DATA",
           width_col3, "MASK"));
    /* Print the normal 2 parameter qualifiers. */
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if (qual == bcmFieldQualifySrcPort ||
            qual == bcmFieldQualifyDstPort ||
            qual == bcmFieldQualifySrcMac ||
            qual == bcmFieldQualifyDstMac) {
            continue;
        }
        LOG_CLI(("%-*s%-*s%-*s\n",
               width_col1, format_field_qualifier(buf, qual, 1),
               width_col2, param_table[qual][0],
               width_col3, param_table[qual][1]));
    }

    /* Print the qualifiers that use parse tables. */
    tk371x_fp_qual_mac_help("", "SrcMac", width_col1);
    tk371x_fp_qual_mac_help("", "DstMac", width_col1);
    return CMD_OK;
}

STATIC int
tk371x_fp_qual_modport_help(const char *prefix, const char *qual_str, int width_col1)
{
    if (width_col1 < sal_strlen(qual_str)) {
        width_col1 = sal_strlen(qual_str) + 1;
    }

    /* "FieldProcessor qual <eid> DstPort" */
    LOG_CLI(("%s%-*s%s\n", prefix, width_col1, qual_str,
           "Port=<port_numb> PortMask=<mask> Modid=<mod>, ModidMask=<mask>"));

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_8(int unit, bcm_field_entry_t eid, args_t *args,
          int func(int, bcm_field_entry_t, uint8, uint8),
          char *qual_str)
{
    int   retval = CMD_OK;
    char  *subcmd = NULL;
    uint8 data, mask;
    char  stat_str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(stat_str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(stat_str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_ip(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, bcm_ip_t, bcm_ip_t),
           char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_ip_t                    data, mask;
    char                        stat_str[FP_STAT_STR_SZ];


    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    if (parse_ipaddr(subcmd, &data) < 0) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: invalid ip4 addr string: \"%s\"\n"),
                   unit, subcmd));
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    if (parse_ipaddr(subcmd, &mask) < 0) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: invalid ip4 addr string: \"%s\"\n"),
                   unit, subcmd));
        return CMD_FAIL;
    }

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(stat_str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(stat_str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}
STATIC int
tk371x_fp_qual_ip_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip_t*, bcm_ip_t*),
            char *qual_str)
{
    int                         retval = CMD_OK;
    bcm_ip_t                   data, mask;
    char                        str[FP_STAT_STR_SZ];

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, &data, &mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    LOG_CLI(("ip %x mask %x\n",data,mask));
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_32(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint32, uint32),
           char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint32                      data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_16(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint16, uint16),
           char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint16                      data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_vlan(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_vlan_t, bcm_vlan_t),
             char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_vlan_t                  data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_l4port(int unit, bcm_field_entry_t eid, args_t *args,
               int func(int, bcm_field_entry_t, bcm_l4_port_t, bcm_l4_port_t),
               char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_l4_port_t               data, mask;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);
    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_ip6(int unit, bcm_field_entry_t eid, args_t *args,
            int func(int, bcm_field_entry_t, bcm_ip6_t, bcm_ip6_t),
            char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_ip6_t                   data, mask;
    char                        str[FP_STAT_STR_SZ];

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    SOC_IF_ERROR_RETURN(parse_ip6addr(subcmd, data));

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    SOC_IF_ERROR_RETURN(parse_ip6addr(subcmd, mask));


    LOG_CLI(("\tip6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n\
        mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
        data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],
        data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],
        mask[0],mask[1],mask[2],mask[3],mask[4],mask[5],mask[6],mask[7],
        mask[8],mask[9],mask[10],mask[11],mask[12],mask[13],mask[14],mask[15]));

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

STATIC int
tk371x_fp_qual_ip6_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip6_t*, bcm_ip6_t*),
            char *qual_str)
{
    int                         retval = CMD_OK;
    bcm_ip6_t                   data, mask;
    char                        str[FP_STAT_STR_SZ];

    /* BCM.1> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, &data, &mask);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);


    LOG_CLI(("\tip6 %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n\
        mask %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x \n",
        data[0],data[1],data[2],data[3],data[4],data[5],data[6],data[7],
        data[8],data[9],data[10],data[11],data[12],data[13],data[14],data[15],
        mask[0],mask[1],mask[2],mask[3],mask[4],mask[5],mask[6],mask[7],
        mask[8],mask[9],mask[10],mask[11],mask[12],mask[13],mask[14],mask[15]));
    return CMD_OK;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_qual_rangecheck(int unit, bcm_field_entry_t eid, args_t *args,
                   int func(int, bcm_field_entry_t, bcm_field_range_t, int),
                   char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    int                         range, result;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);
    FP_GET_NUMB(range, subcmd, args);
    FP_GET_NUMB(result, subcmd, args);

    /* BCM.0> fp qual 'eid' RangeCheck 'range' 'result' */
    retval = func(unit, eid, range, result);
    /* coverity[secure_coding] */
    strncpy(str, "bcm_field_qualify_", FP_BCM_FIELD_QUALIFY_STR_LEN);
    /* coverity[overrun-buffer-arg] */
    strncat(str, qual_str, FP_STAT_STR_SZ - FP_BCM_FIELD_QUALIFY_STR_LEN);
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_qual(int unit, args_t *args)
{
    char                  *subcmd   = NULL;
    char                  *qual_str = NULL;
    bcm_field_entry_t     eid;
    int                   rv = CMD_OK;
    int qual_get = 0;

    FP_GET_NUMB(eid, subcmd, args);
    /* > fp qual 'eid' ...*/

    if ((qual_str = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /*
     * Argument exception: "clear" does not correspond directly to
     * any bcmFieldQualify* enum.
     */
    if(!sal_strcasecmp(qual_str, "clear")) {
        /* BCM.1> fp qual 'eid' clear  */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:fp_qual_clear 'eid=%d'\n"),
                     unit, eid));
        rv = bcm_field_qualify_clear(unit, eid);
        FP_CHECK_RETURN(unit, rv, "bcm_field_qualify_clear");
        return rv;
    }

    /*
     * Argument exception: "delete" does not correspond directly to
     * any bcmFieldQualify* enum.
     */
    if(!sal_strcasecmp(qual_str, "delete")) {
        if ((qual_str = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        /* BCM.1> fp qual 'eid' delete 'qual_name' */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:fp_qual_delete  'eid=%d qual=%s'\n"),
                     unit, eid, qual_str));
        rv = bcm_field_qualifier_delete(unit, eid,
                                        parse_field_qualifier(qual_str));
        FP_CHECK_RETURN(unit, rv, "bcm_field_qualifier_delete");

        return rv;
    }

    if(!sal_strcasecmp(qual_str, "get")) {
        if ((qual_str = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        qual_get = TRUE;
    }

    subcmd = ARG_GET(args);
    if (subcmd != NULL) {
        if ((!sal_strcasecmp(subcmd, "show")) ||
            (!sal_strcasecmp(subcmd, "?"))) {
            switch (parse_field_qualifier(qual_str)) {
                case bcmFieldQualifySrcMac:
                    return tk371x_fp_qual_mac_help("fp qual <eid> ", "SrcMac", 0);
                    break;
                case bcmFieldQualifyDstMac:
                    return tk371x_fp_qual_mac_help("fp qual <eid> ", "DstMac", 0);
                    break;
                case bcmFieldQualifySrcPort:
                    return tk371x_fp_qual_modport_help("fp qual <eid> ", "SrcPort", 0);
                    break;
                case bcmFieldQualifyDstPort:
                    return tk371x_fp_qual_modport_help("fp qual <eid> ", "DstPort", 0);
                    break;
                default:
                    return tk371x_fp_list_quals(unit);
            }
        } else {
            ARG_PREV(args);
        }
    }
    /* BCM.1> fp qual 'eid' bcmFieldQualifyXXX ...*/
	switch (parse_field_qualifier(qual_str)) {
		case bcmFieldQualifySrcMac:
			rv = tk371x_fp_qual_mac(unit, eid, args, bcm_field_qualify_SrcMac, "SrcMac");
			break;
		case bcmFieldQualifyDstMac:
			rv = tk371x_fp_qual_mac(unit, eid, args, bcm_field_qualify_DstMac, "DstMac");
			break;
		case bcmFieldQualifyIp6FlowLabel:
			rv = tk371x_fp_qual_32(unit, eid, args, bcm_field_qualify_Ip6FlowLabel,
					   "Ip6FlowLabel");
			break;
		case bcmFieldQualifyEtherType:
			rv = tk371x_fp_qual_16(unit, eid, args, bcm_field_qualify_EtherType,
					   "EtherType");
			break;
		case bcmFieldQualifyOuterVlanId:
			rv = tk371x_fp_qual_vlan(unit, eid, args, bcm_field_qualify_OuterVlanId,
						 "OuterVlanId");
			break;
		case bcmFieldQualifyDSCP:
			/*bcmFieldQualifyTos*/
			rv = tk371x_fp_qual_8(unit, eid, args, bcm_field_qualify_DSCP,
					  "DSCP");
			break;
		case bcmFieldQualifyLlidValue:
			rv = tk371x_fp_qual_16(unit, eid, args, bcm_field_qualify_LlidValue,
									   "LlidValue");
			break;
		case bcmFieldQualifyL4SrcPort:
			rv = tk371x_fp_qual_l4port(unit, eid, args, bcm_field_qualify_L4SrcPort,
						   "L4SrcPort");
			break;
		case bcmFieldQualifyL4DstPort:
			rv = tk371x_fp_qual_l4port(unit, eid, args, bcm_field_qualify_L4DstPort,
						   "L4DstPort");
			break;
		case bcmFieldQualifyOuterVlanPri:
			rv = tk371x_fp_qual_8(unit, eid, args, bcm_field_qualify_OuterVlanPri,
						 "OuterVlanPri");
			break;
		case bcmFieldQualifyIpProtocol:
			/*IPv6 next header*/
			if(!sal_strcasecmp(qual_str, "bcmFieldQualifyIpProtocol")) {
				rv = tk371x_fp_qual_8(unit, eid, args, bcm_field_qualify_IpProtocol,
					  "IpProtocol");
			}else if (!sal_strcasecmp(qual_str, "bcmFieldQualifyIp6NextHeader")){
				rv = tk371x_fp_qual_8(unit, eid, args, bcm_field_qualify_Ip6NextHeader,
										  "Ip6NextHeader");
			}else{
				rv = tk371x_fp_qual_8(unit, eid, args, bcm_field_qualify_IpProtocol,
										  "IpProtocol");
			}
			break;
		case bcmFieldQualifyDstIp:
			if(qual_get) {
				rv = tk371x_fp_qual_ip_get(unit, eid, bcm_field_qualify_DstIp_get,
					   "DstIp");
			} else {
				rv = tk371x_fp_qual_ip(unit, eid, args, bcm_field_qualify_DstIp,
					   "DstIp");
			}
			break;
		case bcmFieldQualifyDstIp6Low:
			if(qual_get) {
				rv = tk371x_fp_qual_ip6_get(unit, eid, bcm_field_qualify_DstIp6Low_get,"DstIp6Low");
			} else {
				rv = tk371x_fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6Low,
						"DstIp6Low");
			}
			break;
		case bcmFieldQualifyDstIp6High:
			if(qual_get) {
				rv = tk371x_fp_qual_ip6_get(unit, eid, bcm_field_qualify_DstIp6High_get,"DstIp6High");
			} else {
				rv = tk371x_fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6High,
						"DstIp6High");
			}
			break;
		case bcmFieldQualifySrcIp:
			if(qual_get) {
				rv = tk371x_fp_qual_ip_get(unit, eid, bcm_field_qualify_SrcIp_get,
					   "SrcIp");
			} else {
				rv = tk371x_fp_qual_ip(unit, eid, args, bcm_field_qualify_SrcIp,
					   "SrcIp");
			}
			break;
		case bcmFieldQualifySrcIp6Low:
			if(qual_get) {
				rv = tk371x_fp_qual_ip6_get(unit, eid, bcm_field_qualify_SrcIp6Low_get,"SrcIp6");
			} else {
				rv = tk371x_fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6Low,
						"SrcIp6Low");
			}
			break;
		case bcmFieldQualifySrcIp6High:
			if(qual_get) {
				rv = tk371x_fp_qual_ip6_get(unit, eid, bcm_field_qualify_SrcIp6High_get,"SrcIp6");
			} else {
				rv = tk371x_fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6High,
						"SrcIp6High");
			}
			break;
		case bcmFieldQualifyVlanFormat:
			rv = tk371x_fp_qual_8(unit, eid, args, bcm_field_qualify_VlanFormat,
						 "VlanFormat");
			break;
		case bcmFieldQualifyRangeCheck:
			rv = tk371x_fp_qual_rangecheck(unit, eid, args, bcm_field_qualify_RangeCheck,
						   "RangeCheck");
			break;
		case bcmFieldQualifyCount:
		default:
			tk371x_fp_list_quals(unit);
            cli_out("FP(unit %d) Error: Unknown qualifier: %s\n",
                    unit, qual_str);
			rv = CMD_FAIL;
	}

	if (CMD_OK != rv) {
        cli_out("FP(unit %d) Error: Qualifier installation error: %s\n",
                unit, qual_str);
	}

	return rv;
}

/*
 * Function:
 *     fp_list_actions
 * Purpose:
 *     Display a list of Field actions
 * Parmameters:
 * Returns:
 */
STATIC int
tk371x_fp_list_actions(int unit) {
    bcm_field_action_t action;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    char                *param_table[bcmFieldActionCount][2];
    int                 width_col1 = 20, width_col2 = 35, width_col3= 20;

    /* Fill the table with default values for param0 & param1. */
    for (action = 0; action < bcmFieldActionCount; action++) {
        param_table[action][0] = "n/a";
        param_table[action][1] = "n/a";
    }
	param_table[bcmFieldActionRedirectPort][0]        = "Dest. queue";
	param_table[bcmFieldActionRedirectPort][1]        = "Dest. queue offset";
	param_table[bcmFieldActionOuterVlanAdd][0]        = "New vlan id";
	param_table[bcmFieldActionOuterVlanPrioNew][0]    = "New priority";
	param_table[bcmFieldActionOuterVlanNew][0]        = "New vlan id";


    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "ACTION", width_col2, "PARAM0",
           width_col3, "PARAM1"));
    for (action = 0; action < bcmFieldActionCount; action++) {
        if (action == bcmFieldActionUpdateCounter ||
            action == bcmFieldActionMeterConfig) {
            continue;
        }
        LOG_CLI(("%-*s%-*s%-*s\n",
               width_col1, format_field_action(buf, action, 1),
               width_col2, param_table[action][0],
               width_col3, param_table[action][1]));
    }
    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_action_add(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    bcm_field_action_t  action;
    int                 retval;
    int                 p0 = 0, p1 = 0;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];

    FP_GET_NUMB(eid, subcmd, args);
    if ((subcmd  = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if (isint(subcmd)) {
        action = parse_integer(subcmd);
    } else {
        action = parse_field_action(subcmd);
        if (action == bcmFieldActionCount) {
        	tk371x_fp_list_actions(unit);
            cli_out("FP(unit %d) Error: Unknown action: %s\n",
                    unit, subcmd);
            return CMD_FAIL;
        }
    }

    /* Read the action parameters (p0 and p1).*/
    switch (action) {
		case bcmFieldActionOuterVlanAdd:
		case bcmFieldActionOuterVlanPrioNew:
		case bcmFieldActionOuterVlanNew:
		case bcmFieldActionRedirectPort:
			FP_GET_NUMB(p0, subcmd, args);
			FP_GET_NUMB(p1, subcmd, args);
			break;
		case bcmFieldActionPrioPktTos:
		case bcmFieldActionDrop:
		case bcmFieldActionDropCancel:
		case bcmFieldActionOuterVlanAddCancel:
		case bcmFieldActionOuterVlanDeleteCancel:
		case bcmFieldActionOuterVlanPrioCopyInner:
		case bcmFieldActionOuterVlanCopyInner:
		case bcmFieldActionOuterVlanDelete:
			p0 = 0;
			p1 = 0;
			break;
		default:
			LOG_CLI(("FP(unit %d) action: %s can't support\n", unit, subcmd));
			return CMD_USAGE;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: action add eid=%d, action=%s, "
                            "p0=0x%x, p1=0x%x\n"),
                 unit, eid, format_field_action(buf, action, 1), p0, p1));
    retval = bcm_field_action_add(unit, eid, action, p0, p1);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_add");

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_action_get(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;
    uint32              p0 = 0, p1 = 0;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];

    FP_GET_NUMB(eid, subcmd, args);
    if ((subcmd  = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if (isint(subcmd)) {
        action = parse_integer(subcmd);
    } else {
        action = parse_field_action(subcmd);
        if (action == bcmFieldActionCount) {
            cli_out("FP(unit %d) Error: Unknown action: %s\n",
                    unit, subcmd);
            return CMD_FAIL;
        }
    }

    retval = bcm_field_action_get(unit, eid, action, &p0, &p1);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_get");
    LOG_CLI(("FP action get: action=%s, p0=%d, p1=%d\n",
           format_field_action(buf, action, 1), p0, p1));

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_action_remove(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;

    FP_GET_NUMB(eid, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        retval = bcm_field_action_remove_all(unit, eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_action_remove_all");
    } else {
        if (isint(subcmd)) {
            action = parse_integer(subcmd);
        } else {
            action = parse_field_action(subcmd);
            if (action == bcmFieldActionCount) {
                cli_out("FP(unit %d) Error: Unrecognized action: %s\n",
                        unit, subcmd);
                return CMD_FAIL;
            }
        }
        retval = bcm_field_action_remove(unit, eid, action);
        FP_CHECK_RETURN(unit, retval, "bcm_field_action_remove");
    }

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_action(int unit, args_t *args){
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.1> fp action add ... */
    if(!sal_strcasecmp(subcmd, "add")) {
        return tk371x_fp_action_add(unit, args);
    }
    /* BCM.1> fp action get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return tk371x_fp_action_get(unit, args);
    }
    /* BCM.1> fp action remove... */
    if(!sal_strcasecmp(subcmd, "remove")) {
        return tk371x_fp_action_remove(unit, args);
    }

    return CMD_USAGE;
}


STATIC cmd_result_t
tk371x_fp_entry_create(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(gid, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.1> fp entry create 'gid'  */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: _entry_create gid=%d\n"),
                     unit, gid));
        retval = bcm_field_entry_create(unit, gid, &eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_create");
    } else {
        /* BCM.1> fp entry create 'gid' 'eid' */
        eid = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: _entry_create gid=%d, eid=%d\n"),
                     unit, gid, eid));
        retval = bcm_field_entry_create_id(unit, gid, eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_create_id");
    }
    LOG_CLI(("EID %d created!\n", eid));

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_entry_copy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           src_eid, dst_eid = -111;

    FP_GET_NUMB(src_eid, subcmd, args);
    subcmd = ARG_GET(args);

    if (subcmd ) {
        /* BCM.1> fp entry copy 'src_eid' 'dst_eid'  */
        dst_eid = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:  bcm_field_entry_copy_id(src_eid=%d, "
                                "dst_eid=%d)\n"),
                     unit, src_eid, dst_eid));
        retval = bcm_field_entry_copy_id(unit, src_eid, dst_eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_copy_id");
    } else {
        /* BCM.1> fp entry copy 'src_eid' */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_entry_copy(src_eid=%d)\n"),
                     unit, src_eid));
        retval = bcm_field_entry_copy(unit, src_eid, &dst_eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_copy");
    }
    LOG_CLI(("EID %d created!\n", dst_eid));
    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_entry_destroy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    if ((subcmd = ARG_GET(args)) == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_entry_destroy_all()\n"),
                     unit));
        retval = bcm_field_entry_destroy_all(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_destroy_all");
        return CMD_OK;
    } else {
        eid = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_entry_destroy(eid=%d)\n"),
                     unit, eid));
        retval = bcm_field_entry_destroy(unit, eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_destroy");
        return CMD_OK;
    }
    return CMD_USAGE;
}

STATIC cmd_result_t
tk371x_fp_entry_install(int unit, args_t *args)
{
    int                         retval;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    /* BCM.1> fp detach 'eid' */
    FP_GET_NUMB(eid, subcmd, args);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "_entry_install eid=%d\n"),
                 eid));
    retval = bcm_field_entry_install(unit, eid);

    /* Check return status. */
    LOG_CLI(("bcm_field_entry_install(unit=%d, entry=%d) ", unit, eid));
    if ((retval) != BCM_E_NONE) {
        LOG_CLI(("failed: %s\n", bcm_errmsg(retval)));
        return CMD_FAIL;
    }
    LOG_CLI(("okay\n"));
    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_entry_reinstall(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(eid, subcmd, args);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _entry_reinstall eid=%d\n"),
                 unit, eid));
    retval = bcm_field_entry_reinstall(unit, eid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_reinstall");
    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_entry_remove(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(eid, subcmd, args);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _entry_remove eid=%d\n"),
                 unit, eid));
    retval = bcm_field_entry_remove(unit, eid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_remove");
    return CMD_OK;
}

#define _TK371X_FIELD_ENTRY_PRIO_HIGHEST	0x0F
#define _TK371X_FIELD_ENTRY_PRIO_LOWEST		0x01
#define _TK371X_FIELD_ENTRY_PRIO_DEFAULT	0x0A

STATIC cmd_result_t
tk371x_fp_entry_prio(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    int                         prio;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(eid, subcmd, args);

    /* BCM.0> fp entry prio <eid> */
    if ((subcmd = ARG_GET(args)) == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_entry_prio_get(eid=%d)\n"),
                     unit, eid));
        retval = bcm_field_entry_prio_get(unit, eid, &prio);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_prio_get");
        LOG_CLI(("FP entry=%d: prio=%d\n", eid, prio));
    } else {
        /* BCM.1> fp entry prio <eid> [prio] */
        if (isint(subcmd)) {
            prio = parse_integer(subcmd);
            if (prio > _TK371X_FIELD_ENTRY_PRIO_HIGHEST){
            	return CMD_USAGE;
            }
            if (prio < _TK371X_FIELD_ENTRY_PRIO_LOWEST){
            	return CMD_USAGE;
            }
        } else {
            if(!sal_strcasecmp(subcmd, "highest")) {
                prio = _TK371X_FIELD_ENTRY_PRIO_HIGHEST;
            } else if(!sal_strcasecmp(subcmd, "lowest")) {
                prio = _TK371X_FIELD_ENTRY_PRIO_LOWEST;
            } else if(!sal_strcasecmp(subcmd, "default")) {
                prio = _TK371X_FIELD_ENTRY_PRIO_DEFAULT;
            } else {
                return CMD_USAGE;
            }
        }


        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_entry_prio_set(eid=%d, "
                                "prio=%d)\n"),
                     unit, eid, prio));
        retval = bcm_field_entry_prio_set(unit, eid, prio);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_prio_set");
    }
    return CMD_OK;
}

#define _TK371X_FIELD_ENTRY_PORT_HIGHEST	10
#define _TK371X_FIELD_ENTRY_PORT_LOWEST		0
STATIC cmd_result_t
tk371x_fp_entry_inport(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    int                         inport, pmask = 0;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(eid, subcmd, args);

    /* BCM.1> fp entry inport <eid> */
    if ((subcmd = ARG_GET(args)) == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_qualify_InPort_get(eid=%d)\n"),
                     unit, eid));
        retval = bcm_field_qualify_InPort_get(unit, eid, &inport, &pmask);
        FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_InPort_get");
        LOG_CLI(("FP entry=%d: inport=%d\n", eid, inport));
    } else {
        /* BCM.1> fp entry inport <eid> [prio] */
        if (isint(subcmd)) {
            inport = parse_integer(subcmd);
            if (inport > _TK371X_FIELD_ENTRY_PORT_HIGHEST){
            	return CMD_USAGE;
            }
            if (inport < _TK371X_FIELD_ENTRY_PORT_LOWEST){
            	return CMD_USAGE;
            }
        } else {
           return CMD_USAGE;
        }


        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_qualify_InPort_set(eid=%d, "
                                "inport=%d)\n"),
                     unit, eid, inport));
        retval = bcm_field_qualify_InPort(unit, eid, inport, pmask);
        FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_InPort_set");
    }
    return CMD_OK;
}



STATIC cmd_result_t
tk371x_fp_entry(int unit, args_t *args){
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.1> fp entry create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return tk371x_fp_entry_create(unit, args);
    }
    /* BCM.1> fp entry copy ... */
    if(!sal_strcasecmp(subcmd, "copy")) {
        return tk371x_fp_entry_copy(unit, args);
    }
    /* BCM.1> fp entry destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return tk371x_fp_entry_destroy(unit, args);
    }
    /* BCM.1> fp entry install ... */
    if(!sal_strcasecmp(subcmd, "install")) {
        return tk371x_fp_entry_install(unit, args);
    }
    /* BCM.1> fp entry reinstall ... */
    if(!sal_strcasecmp(subcmd, "reinstall")) {
        return tk371x_fp_entry_reinstall(unit, args);
    }
    /* BCM.1> fp entry remove ... */
    if(!sal_strcasecmp(subcmd, "remove")) {
        return tk371x_fp_entry_remove(unit, args);
    }
    /* BCM.1> fp entry prio ... */
    if(!sal_strcasecmp(subcmd, "prio")) {
        return tk371x_fp_entry_prio(unit, args);
    }
    /* BCM.1> fp entry inport ... */
    if(!sal_strcasecmp(subcmd, "inport")) {
        return tk371x_fp_entry_inport(unit, args);
    }

    return CMD_USAGE;
}


STATIC cmd_result_t
tk371x_fp_group_create(int unit, args_t *args, bcm_field_qset_t *qset)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         pri;
    bcm_field_group_t           gid;

    FP_GET_NUMB(pri, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.1> fp group create 'prio'  */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: _group_create pri=%d\n"),
                     unit, pri));
        retval = bcm_field_group_create(unit, *qset, pri, &gid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_group_create");
    } else {
        gid = parse_integer(subcmd);
        if ((subcmd = ARG_GET(args)) == NULL) {
            /* BCM.1> fp group create 'prio' 'gid' */
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "FP(unit %d) verb: _group_create_id pri=%d gid=%d\n"),
                         unit, pri, gid));
            retval = bcm_field_group_create_id(unit, *qset, pri, gid);
            FP_CHECK_RETURN(unit, retval, "bcm_field_group_create_id");
        }else{
        	return CMD_USAGE;
        }
    }
    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_group_destroy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_GET_NUMB(gid, subcmd, args);
    LOG_CLI(("gid=%d\n", gid));
    /* BCM.1> fp group destroy 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:_group_destroy gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_destroy(unit, gid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_destroy");

    return CMD_OK;
}


STATIC cmd_result_t
tk371x_fp_group_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_qset_t            qset;

    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.1> fp group create 'prio'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _group_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_get(unit, gid, &qset);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_get");
    tk371x_fp_qset_show(&qset);
    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_group_set(int unit, args_t *args, bcm_field_qset_t *qset)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.1> fp group set 'gid' */
    retval = bcm_field_group_set(unit, gid, *qset);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_set");

    return CMD_OK;
}


STATIC cmd_result_t
tk371x_fp_group_status_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_group_status_t    gstat;

    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.1> fp group status 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:_group_status_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_status_get(unit, gid, &gstat);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_status_get");

        LOG_CLI(("group_status={\t"));
        LOG_CLI(("prio_min=%d,       \t",  gstat.prio_min));
        LOG_CLI(("prio_max=%d,       \t",  gstat.prio_max));
        LOG_CLI(("entries_total=%d,\t",    gstat.entries_total));
        LOG_CLI(("entries_count=%d \t\t", gstat.entry_count));
        LOG_CLI(("}\n"));

    FP_CHECK_RETURN(unit, retval, "bcm_field_group_status_get");

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_group(int unit, args_t *args, bcm_field_qset_t *qset){
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.1> fp group create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return tk371x_fp_group_create(unit, args, qset);
    }
    /* BCM.1> fp group destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return tk371x_fp_group_destroy(unit, args);
    }

    /* BCM.1> fp group get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return tk371x_fp_group_get(unit, args);
    }

    /* BCM.1> fp group set ... */
    if(!sal_strcasecmp(subcmd, "set")) {
        return tk371x_fp_group_set(unit, args, qset);
    }

    /* BCM.1> fp group status ... */
    if(!sal_strcasecmp(subcmd, "status")) {
        return tk371x_fp_group_status_get(unit, args);
    }

	return CMD_USAGE;
}

STATIC cmd_result_t
tk371x_fp_range_group_create(int unit, args_t *args){
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_range_t           rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;
    bcm_if_group_t              group;
    uint32                      param[5];

    FP_GET_NUMB(param[0], subcmd, args);
    FP_GET_NUMB(param[1], subcmd, args);
    FP_GET_NUMB(param[2], subcmd, args);
    FP_GET_NUMB(param[3], subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.1> fp range group create 'flags' 'min' 'max' */
        flags = param[0];
        min   = param[1];
        max   = param[2];
        group = param[3];
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:_range_group_create flags=0x%x, "
                                "min=%d, max=%d group=%d\n"),
                     unit, flags, min, max, group));
        retval = bcm_field_range_group_create(unit, &rid, flags, min, max, group);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_group_create");
    } else {
        /* BCM.1> fp range group create 'rid' 'flags' 'min' 'max' */
        rid   = param[0];
        flags = param[1];
        min   = param[2];
        max   = param[3];
        group = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:_range_group_create_id  rid=%d, "
                                "flags=0x%x, min=%d, max=%d group=%d\n"),
                     unit, rid, flags, min, max, group));
        retval = bcm_field_range_group_create_id(unit, rid, flags, min, max, group);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_group_create_id");
    }
    LOG_INFO(BSL_LS_BCM_FP,
             (BSL_META_U(unit,
                         "RID %d created!\n"),
              rid));

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_range_create(int unit, args_t *args){
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_range_t           rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;
    uint32                      param[4];

    FP_GET_NUMB(param[0], subcmd, args);
    FP_GET_NUMB(param[1], subcmd, args);
    FP_GET_NUMB(param[2], subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.1> fp range create 'flags' 'min' 'max' */
        flags = param[0];
        min   = param[1];
        max   = param[2];
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:_range_create flags=0x%x, "
                                "min=%d, max=%d \n"),
                 unit, flags, min, max));
        retval = bcm_field_range_create(unit, &rid, flags, min, max);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_create");
    } else {
        /* BCM.1> fp range create 'rid' 'flags' 'min' 'max' */
        rid   = param[0];
        flags = param[1];
        min   = param[2];
        max   = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:_range_create_id rid=%d, "
                                "flags=0x%x, min=%d, max=%d \n"),
                     unit, rid, flags, min, max));
        retval = bcm_field_range_create_id(unit, rid, flags, min, max);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_create_id");
    }
    LOG_CLI(("RID %d created!\n", rid));

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_range_get(int unit, args_t *args){
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;

    FP_GET_NUMB(rid, subcmd, args);

    /* BCM.1> fp range get 'rid'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:fp_range_get 'rid=%d'\n"),
                 unit, rid));
    retval = bcm_field_range_get(unit, rid, &flags, &min, &max);
    FP_CHECK_RETURN(unit, retval, "bcm_field_range_get");
    LOG_CLI(("FP range get: rid=%d, min=%d max=%d ", rid, min, max));
    LOG_CLI(("flags=0x%x%s%s%s%s\n",
             flags,
             flags & BCM_FIELD_RANGE_SRCPORT ? " SRCPORT" : "",
             flags & BCM_FIELD_RANGE_DSTPORT ? " DSTPORT" : "",
             flags & BCM_FIELD_RANGE_OUTER_VLAN? " OUTERVLAN" : "",
             flags & BCM_FIELD_RANGE_PACKET_LENGTH? " PACKET LEN" : ""));

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_range_destroy(int unit, args_t *args){
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         rid;

    FP_GET_NUMB(rid, subcmd, args);

    /* BCM.1> fp range destroy 'rid'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:fp_range_destroy 'rid=%d'\n"),
                 unit, rid));
    retval = bcm_field_range_destroy(unit, rid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_range_destroy");

    return CMD_OK;
}

STATIC cmd_result_t
tk371x_fp_range(int unit, args_t *args){
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.1> fp range group create ... */
    if(!sal_strcasecmp(subcmd, "group")) {
        if ((subcmd = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        if(!sal_strcasecmp(subcmd, "create")) {
            return tk371x_fp_range_group_create(unit, args);
        }
    }
    /* BCM.1> fp range create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return tk371x_fp_range_create(unit, args);
    }
    /* BCM.1> fp range get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return tk371x_fp_range_get(unit, args);
    }
    /* BCM.1> fp range destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return tk371x_fp_range_destroy(unit, args);
    }
    return CMD_USAGE;
}


cmd_result_t
if_ea_field_proc(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    static bcm_field_qset_t     qset;
#ifdef BROADCOM_DEBUG
    bcm_field_group_t           gid;
    bcm_field_entry_t           eid = 0;
#endif /* BROADCOM_DEBUG */

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.1> fp qual ... */
    if(!sal_strcasecmp(subcmd, "qual")) {
    /*
     *COVERITY
     *
     *It's reported on a project which is never used any more.
     */
    /* coverity[stack_use_overflow] */
        return tk371x_fp_qual(unit, args);
    }

    /* BCM.1> fp qset ... */
    if(!sal_strcasecmp(subcmd, "qset")) {
        return tk371x_fp_qset(unit, args, &qset);
    }

    /* BCM.1> fp action ... */
    if(!sal_strcasecmp(subcmd, "action")) {
        return tk371x_fp_action(unit, args);
    }

    /* BCM.1> fp detach */
    if(!sal_strcasecmp(subcmd, "detach")) {
        retval = bcm_field_detach(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_detach");
        return CMD_OK;
    }

    /* BCM.1> fp entry ... */
    if(!sal_strcasecmp(subcmd, "entry")) {
        return tk371x_fp_entry(unit, args);
    }

    /* BCM.1> fp group ... */
    if(!sal_strcasecmp(subcmd, "group")) {
        return tk371x_fp_group(unit, args, &qset);
    }

    /* BCM.1> fp init */
    if(!sal_strcasecmp(subcmd, "init")) {
        retval = bcm_field_init(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_init");
        BCM_FIELD_QSET_INIT(qset);
        return CMD_OK;
    }

    /* BCM.1> fp install - deprecated, use fp entry install */
    if(!sal_strcasecmp(subcmd, "install")) {
        return tk371x_fp_entry_install(unit, args);
    }

    /* BCM.1> fp range ... */
    if(!sal_strcasecmp(subcmd, "range")) {
        return tk371x_fp_range(unit, args);
    }
#ifdef BROADCOM_DEBUG
    /* BCM.1> fp show ...*/
    if(!sal_strcasecmp(subcmd, "show")) {
        if ((subcmd = ARG_GET(args)) != NULL) {
            /* BCM.1> fp show entry ...*/
            if(!sal_strcasecmp(subcmd, "entry")) {
                if ((subcmd = ARG_GET(args)) == NULL) {
                    return CMD_USAGE;
                } else {
                    /* BCM.1> fp show entry 'eid' */
                    eid = parse_integer(subcmd);
                    bcm_field_entry_dump(unit, eid);
                    return CMD_OK;
                }
            }
            /* BCM.1> fp show group ...*/
            if(!sal_strcasecmp(subcmd, "group")) {
                FP_GET_NUMB(gid, subcmd, args);
                bcm_field_group_dump(unit, gid);
                return CMD_OK;
            }
            /* BCM.1> fp show qset */
            if(!sal_strcasecmp(subcmd, "qset")) {
                tk371x_fp_qset_show(&qset);
                return CMD_OK;
            }
            return CMD_NFND;
        } else {
            /* BCM.1> fp show */
            bcm_field_show(unit, "FP");
            return CMD_OK;
        }
    }
#endif /* BROADCOM_DEBUG */
    return CMD_USAGE;
}
#endif /* BCM_TK371X_SUPPORT */
#endif /* BCM_FIELD_SUPPORT */
