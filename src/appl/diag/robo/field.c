/*
 * $Id: field.c,v 1.49 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Field Processor related CLI commands
 */

#include <shared/bsl.h>

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

#include <bcm_int/robo/field.h>
#include <soc/cfp.h>


#ifdef BCM_FIELD_SUPPORT

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

/*
 * Typedef:
 *     _bcm_field_qual_data_t
 * Purpose:
 *     Qualifier data/mask field buffer.
 */
#define _FP_QUAL_DATA_WORDS  (4)
typedef uint32 _bcm_field_qual_data_t[_FP_QUAL_DATA_WORDS];   /* Qualifier data/mask buffer. */    

/*
 * Marker for last element in qualification table 
 */
#define FP_TABLE_END_STR "tbl_end"

#define FP_STAT_STR_SZ 256
#define FP_LINE_SZ 72
/*
 * local function prototypes
 */

STATIC char *_robo_fp_qual_stage_name(bcm_field_stage_t stage);
STATIC char *_robo_fp_qual_IpType_name(bcm_field_IpType_t type);
STATIC char *_robo_fp_control_name(bcm_field_control_t control);

STATIC int robo_fp_action(int unit, args_t *args);
STATIC int robo_fp_action_ports(int unit, args_t *args);
STATIC int robo_fp_action_add(int unit, args_t *args);
STATIC int robo_fp_action_ports_add(int unit, args_t *args);
STATIC int robo_fp_lookup_color(char *qual_str);
STATIC int robo_fp_lookup_counter_mode(char *qual_str);
STATIC int robo_fp_lookup_meter_mode(char *qual_str);
STATIC int robo_fp_lookup_meter(char *qual_str);
STATIC int robo_fp_action_get(int unit, args_t *args);
STATIC int robo_fp_action_ports_get(int unit, args_t *args);
STATIC int robo_fp_action_remove(int unit, args_t *args);

STATIC int robo_fp_control(int unit, args_t *args);

STATIC int robo_fp_entry(int unit, args_t *args);
STATIC int robo_fp_entry_create(int unit, args_t *args);
STATIC int robo_fp_entry_copy(int unit, args_t *args);
STATIC int robo_fp_entry_destroy(int unit, args_t *args);
STATIC int robo_fp_entry_install(int unit, args_t *args);
STATIC int robo_fp_entry_reinstall(int unit, args_t *args);
STATIC int robo_fp_entry_remove(int unit, args_t *args);
STATIC int robo_fp_entry_enable(int unit, args_t *args);
STATIC int robo_fp_entry_disable(int unit, args_t *args);
STATIC int robo_fp_entry_prio(int unit, args_t *args);
STATIC int robo_fp_entry_oper(int unit, args_t *args);

STATIC int robo_fp_group(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC int robo_fp_group_create(int unit, args_t *args,
                           bcm_field_qset_t *qset);

STATIC int robo_fp_group_get(int unit, args_t *args);
STATIC int robo_fp_group_destroy(int unit, args_t *args);
STATIC int robo_fp_group_set(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC int robo_fp_group_status_get(int unit, args_t *args);
STATIC int robo_fp_group_mode_get(int unit, args_t *args);

STATIC int robo_fp_list(int unit, args_t *args);
STATIC int robo_fp_list_actions(int unit);
STATIC int robo_fp_list_quals(int unit);
STATIC void robo_fp_print_options(const char *options[], int offset);

STATIC int robo_fp_range(int unit, args_t *args);
STATIC int robo_fp_range_create(int unit, args_t *args);
STATIC int robo_fp_range_group_create(int unit, args_t *args);
STATIC int robo_fp_range_get(int unit, args_t *args);
STATIC int robo_fp_range_destroy(int unit, args_t *args);
STATIC int robo_fp_group_lookup(int unit, args_t *args);
STATIC int robo_fp_group_enable_set(int unit, bcm_field_group_t gid, int enable);

STATIC int robo_fp_policer(int unit, args_t *args);
STATIC int robo_fp_policer_create(int unit, args_t *args);
STATIC int robo_fp_policer_destroy(int unit, args_t *args);
STATIC int robo_fp_policer_attach(int unit, args_t *args);
STATIC int robo_fp_policer_detach(int unit, args_t *args);

STATIC int robo_fp_stat(int unit, args_t *args);
STATIC int robo_fp_stat_create(int unit, args_t *args);
STATIC int robo_fp_stat_destroy(int unit, args_t *args);
STATIC int robo_fp_stat_attach(int unit, args_t *args);
STATIC int robo_fp_stat_detach(int unit, args_t *args);

STATIC int robo_fp_data(int unit, args_t *args);
STATIC int robo_fp_data_create(int unit, args_t *args);
STATIC int robo_fp_data_destroy(int unit, args_t *args);
STATIC int robo_fp_data_packet_format_add_delete(int unit, args_t *args, int op);
STATIC int robo_fp_data_ethertype_add_delete(int unit, args_t *args, int op);
STATIC int robo_fp_data_ipproto_add_delete(int unit, args_t *args, int op);
STATIC int robo_fp_qual_data(int unit, bcm_field_entry_t eid, args_t *args);


STATIC int robo_fp_qset(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC int robo_fp_qset_add(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC int robo_fp_qset_set(int unit, args_t *args, bcm_field_qset_t *qset);
STATIC int robo_fp_qset_show(bcm_field_qset_t *qset);
/* Lookup functions given string equivalent for type. */
STATIC void robo_fp_lookup_control(const char *control_str,
                              bcm_field_control_t *control);

STATIC bcm_field_IpType_t robo_fp_lookup_IpType(const char *type_str);
STATIC bcm_field_stage_t  robo_fp_lookup_stage(const char *stage_str);

/* Qualify related functions */
STATIC int robo_fp_qual(int unit, args_t *args);
STATIC int robo_fp_qual_InPorts(int unit, bcm_field_entry_t eid, args_t *args);
STATIC int robo_fp_qual_OutPorts(int unit, bcm_field_entry_t eid, args_t *args);
/* bcm_field_qualify_XXX exercise functions. */
STATIC int robo_fp_qual_port(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_port_t, bcm_port_t),
              char *qual_str);
STATIC int robo_fp_qual_modport(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_module_t, bcm_module_t,
                       bcm_port_t, bcm_port_t),
              char *qual_str);
STATIC int robo_fp_qual_modport_help(const char *prefix, const char *qual_str,
                                int width_col1);
STATIC int robo_fp_qual_trunk(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_trunk_t, bcm_trunk_t),
              char *qual_str);
STATIC int robo_fp_qual_l4port(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_l4_port_t, bcm_l4_port_t),
              char *qual_str);
STATIC int robo_fp_qual_rangecheck(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_field_range_t, int),
              char *qual_str);
STATIC int robo_fp_qual_vlan(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_vlan_t, bcm_vlan_t),
              char *qual_str);
STATIC int robo_fp_qual_ip(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_ip_t, bcm_ip_t),
              char *qual_str);
STATIC int robo_fp_qual_ip_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip_t*, bcm_ip_t*),
            char *qual_str);
STATIC int robo_fp_qual_ip6(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_ip6_t, bcm_ip6_t),
              char *qual_str);
STATIC int robo_fp_qual_ip6_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip6_t*, bcm_ip6_t*),
            char *qual_str);
STATIC int robo_fp_qual_mac(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, bcm_mac_t, bcm_mac_t),
              char *qual_str);
STATIC int robo_fp_qual_mac_help(const char *prefix, const char *qual_str,
                            int width_col1);
STATIC int robo_fp_qual_snap(int unit, bcm_field_entry_t eid, args_t *args,
                      int func(int, bcm_field_entry_t, bcm_field_snap_header_t, 
                      bcm_field_snap_header_t), char *qual_str);
STATIC int robo_fp_qual_8(int unit, bcm_field_entry_t eid, args_t *args,
              int func(int, bcm_field_entry_t, uint8, uint8),
              char *qual_str);
STATIC int robo_fp_qual_16(int unit, bcm_field_entry_t eid, args_t *args,
                      int func(int, bcm_field_entry_t, uint16, uint16),
                      char *qual_str);
STATIC int robo_fp_qual_32(int unit, bcm_field_entry_t eid, args_t *args,
                      int func(int, bcm_field_entry_t, uint32, uint32),
                      char *qual_str);
STATIC int robo_fp_qual_Decap(int unit, bcm_field_entry_t eid, args_t *args);
STATIC int robo_fp_qual_IpInfo_help(const char *prefix, int width_col1);
STATIC int robo_fp_qual_PacketRes_help(const char *prefix, int width_col1);
STATIC int robo_fp_qual_IpFrag(int unit, bcm_field_entry_t eid, args_t *args);
STATIC int robo_fp_qual_IpFrag_help(const char *prefix, int width_col1);
STATIC int robo_fp_qual_Color_help(const char *prefix, int width_col1);
STATIC int robo_fp_qual_IpType(int unit, bcm_field_entry_t eid, args_t *args);
STATIC int robo_fp_qual_L2Format(int unit, bcm_field_entry_t eid, args_t *args);
STATIC int robo_fp_qual_IpProtocolCommon(int unit, bcm_field_entry_t eid, args_t *args);
STATIC int robo_fp_qual_same(int unit, bcm_field_entry_t eid, args_t *args,
               int func(int, bcm_field_entry_t, uint32),
               char *qual_str);

STATIC int robo_fp_qual_InterfaceClass(int unit, bcm_field_entry_t eid, args_t *args,
            int func(int, bcm_field_entry_t, uint32, uint32), 
            char *qual_str);
STATIC int robo_fp_qual_LoopbackType_help(const char *prefix, int width_col1);
STATIC int robo_fp_qual_TunnelType_help(const char *prefix, int width_col1);

STATIC int robo_fp_thread(int unit, args_t *args);

const char *robo_l2_text[]   = {"Ethernet2", "Snap", "LLC", "Other", NULL};
const char *robo_vlan_text[] = {"NOtag", "ONEtag", "TWOtag", NULL};
const char *robo_ip_text[]   = {"IP4HdrOnly", "IP6HdrOnly", "IP6Fragment",
                             "IP4OverIP4", "IP6OverIP4", "IP4OverIP6", 
                             "IP6OverIP6", "GreIP4OverIP4", "GreIP6OverIP4",
                             "GreIP4OverIP6", "GreIP6OverIP6", "OneMplsLabel",
                             "TwoMplsLabels", "IP6FragmentOverIP4", "IPNotUsed", NULL};
const char *robo_ing_span_text[] = {"DISabled", "BLocK", "LeaRN", "ForWarD", NULL};
const char *robo_packet_res_text[] = {"Unknown", "Control", "Bpdu", "L2BC",
    "L2Uc", "L2UnKnown", "L3McUnknown", "L3McKnown", "L2McKnown", "L2McUnknown",
    "L3UcKnown", "L3UcUnknown", "MplsKnown", "MplsL3Known", "MplsL2Known",
    "MplsUnknown", NULL};
const char *robo_ipfrag_text[] = {"IpFragNon", "IpFragFirst", "IpFragNonOrFirst", 
    "IpFragNotFirst", "IpFragAny", NULL};
const char *robo_color_text[] = {"green", "yellow", "red", NULL};
const char *robo_loopbacktype_text[] = {
    "Any", "Mirror", "Wlan", "Mim", "Redirect",  NULL};
const char *robo_tunneltype_text[] = {
    "Any", "Ip", "Mpls", "Mim", "WtpToAc", "AcToAc", "AutoMulticast", NULL};
const char *robo_policermode_text[] = {"SrTcm", "Committed", "Peak", "TrTcm", 
    "TrTcmDs", "Green", "PassThrough", "SrTcmModified", "CoupledTrTcmDs", 
    "Invalid", NULL};
const char *robo_stattype_text[] = {"Bytes", "Packets", "GreenBytes", 
    "GreenPackets", "YellowBytes", "YellowPackets", "RedBytes", 
    "RedPackets", "NotGreenBytes", "NotGreenPackets", "NotYellowBytes", 
    "NotYellowPackets", "NotRedBytes", "NotRedPackets", "Invalid", NULL};
static const char *offsetbase_text[] = {"PacketStart",
                                        "OuterL3Header",
                                        "InnerL3Header",
                                        "OuterL4Header",
                                        "InnerL4Header",
                                        "HigigHeader",
                                        "Higig2Header",
                                        "FcoeHeader",
                                        "EndTag",
                                        NULL};
static const char *data_l2_text[] = {"Any",
                                     "Ethernet2",
                                     "Snap",
                                     "Llc",
                                     NULL};
static const char *data_vlan_text[] = {"Any",
                                       "NoTag",
                                       "SingleTagged",
                                       "DoubleTagged",
                                       NULL};
static const char *data_ip_text[] = {"Any",
                                     "None",
                                     "Ip4",
                                     "Ip6",
                                     NULL};
static const char *data_tunnel_text[] = {"Any",
                                         "None",
                                         "IpInIp",
                                         "Gre",
                                         "Mpls",
                                         NULL};
static const char *data_mpls_text[] = {"Any",
                                       "OneLabel",
                                       "TwoLabels",
                                       NULL};


static bcm_field_stat_t LastCreatedStatID = -1;
/*
 * Function:
 *      if_field_proc
 * Purpose:
 *      Manage Field Processor (FP)
 * Parameters:
 *      unit - SOC unit #
 *      args - pointer to command line arguments      
 * Returns:
 *    CMD_OK
 */

cmd_result_t
if_robo_field_proc(int unit, args_t *args)
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

    /* BCM.0> fp qual ... */
    if(!sal_strcasecmp(subcmd, "qual")) {
        return robo_fp_qual(unit, args);
    }

    /* BCM.0> fp qset ... */
    if(!sal_strcasecmp(subcmd, "qset")) {
        return robo_fp_qset(unit, args, &qset);
    }

    /* BCM.0> fp action ... */
    if(!sal_strcasecmp(subcmd, "action")) {
        return robo_fp_action(unit, args);
    }
 
    /* BCM.0> fp control ... */
    if(!sal_strcasecmp(subcmd, "control")) {
        return robo_fp_control(unit, args);
    }

    /* BCM.0> fp detach */
    if(!sal_strcasecmp(subcmd, "detach")) {
        retval = bcm_field_detach(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_detach");
        return CMD_OK;
    }

    /* BCM.0> fp entry ... */
    if(!sal_strcasecmp(subcmd, "entry")) {
        return robo_fp_entry(unit, args);
    }
 
    /* BCM.0> fp group ... */
    if(!sal_strcasecmp(subcmd, "group")) {
        return robo_fp_group(unit, args, &qset);
    }
 
    /* BCM.0> fp list ... */
    if(!sal_strcasecmp(subcmd, "list")) {
        return robo_fp_list(unit, args);
    }
    /* BCM.0> fp init */
    if(!sal_strcasecmp(subcmd, "init")) {
        retval = bcm_field_init(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_init");
        BCM_FIELD_QSET_INIT(qset);
        return CMD_OK;
    }

    /* BCM.0> fp install - deprecated, use fp entry install */
    if(!sal_strcasecmp(subcmd, "install")) {
        return robo_fp_entry_install(unit, args);
    }

    /* BCM.0> fp range ... */
    if(!sal_strcasecmp(subcmd, "range")) {
        return robo_fp_range(unit, args);
    }

    /* BCM.0> fp resync */
    if(!sal_strcasecmp(subcmd, "resync")) {
        retval = bcm_field_resync(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_resync");
        return CMD_OK;
    }

    /* BCM.0> fp policer ... */
    if(!sal_strcasecmp(subcmd, "policer")) {
        return robo_fp_policer(unit, args);
    }

    /* BCM.0> fp stat ... */
    if(!sal_strcasecmp(subcmd, "stat")) {
        return robo_fp_stat(unit, args);
    }

    /* BCM.0> fp data ... */
    if(!sal_strcasecmp(subcmd, "data")) {
        return robo_fp_data(unit, args);
    }

#ifdef BROADCOM_DEBUG
    /* BCM.0> fp show ...*/
    if(!sal_strcasecmp(subcmd, "show")) {
        if ((subcmd = ARG_GET(args)) != NULL) {
            /* BCM.0> fp show entry ...*/
            if(!sal_strcasecmp(subcmd, "entry")) {
                if ((subcmd = ARG_GET(args)) == NULL) {
                    return CMD_USAGE;
                } else {
                    /* BCM.0> fp show entry 'eid' */ 
                    eid = parse_integer(subcmd);
                    bcm_field_entry_dump(unit, eid);
                    return CMD_OK;
                }
            }
            /* BCM.0> fp show group ...*/
            if(!sal_strcasecmp(subcmd, "group")) {
                FP_GET_NUMB(gid, subcmd, args);
                bcm_field_group_dump(unit, gid);
                return CMD_OK;
            }
            /* BCM.0> fp show qset */
            if(!sal_strcasecmp(subcmd, "qset")) {
                robo_fp_qset_show(&qset);
                return CMD_OK;
            }
            return CMD_NFND;
        } else {
            /* BCM.0> fp show */
            bcm_field_show(unit, "FP");
            return CMD_OK;
        }
    }
#endif /* BROADCOM_DEBUG */

    if(!sal_strcasecmp(subcmd, "thread")) {
        return robo_fp_thread(unit, args);
    }

    return CMD_USAGE;
}


/*
 * Function:
 *     _robo_fp_qual_stage_name
 * Purpose:
 *     Translate a stage qualifier enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_stage_e. (ex.bcmFieldStageIngressEarly)
 * Returns:
 *     Text name of indicated stage qualifier enum value.
 */
STATIC char *
_robo_fp_qual_stage_name(bcm_field_stage_t stage)
{
    /* Text names of the enumerated qualifier stage values. */
    /* All these are prefixed with "bcmFieldStage" */
    char *stage_text[] = BCM_FIELD_STAGE_STRINGS;

    assert(COUNTOF(stage_text) == bcmFieldStageCount);

    return (stage >= bcmFieldStageCount ? "??" : stage_text[stage]);
}

/*
 * Function:
 *     _robo_fp_qual_IpType_name
 * Purpose:
 *     Translate IpType enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_IpType_e. (ex.bcmFieldIpTypeNonIp)
 * Returns:
 *     Text name of indicated IpType qualifier enum value.
 */
STATIC char *
_robo_fp_qual_IpType_name(bcm_field_IpType_t type)
{
    /* Text names of the enumerated qualifier IpType values. */
    /* All these are prefixed with "bcmFieldIpType" */
    char *IpType_text[bcmFieldIpTypeCount] = BCM_FIELD_IPTYPE_STRINGS;

    assert(COUNTOF(IpType_text) == bcmFieldIpTypeCount);

    return (type >= bcmFieldIpTypeCount ? "??" : IpType_text[type]);
}


/*
 * Function:
 *     _robo_fp_qual_L2Format_name
 * Purpose:
 *     Translate L2Format enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_L2Format_e. (ex.bcmFieldL2FormatSnap)
 * Returns:
 *     Text name of indicated L2Format qualifier enum value.
 */
STATIC char *
_robo_fp_qual_L2Format_name(bcm_field_L2Format_t type)
{
    /* Text names of the enumerated qualifier L2Format values. */
    /* All these are prefixed with "bcmFieldL2Format" */
    char *L2Format_text[bcmFieldL2FormatCount] = BCM_FIELD_L2FORMAT_STRINGS;

    assert(COUNTOF(L2Format_text) == bcmFieldL2FormatCount);

    return (type >= bcmFieldL2FormatCount ? "??" : L2Format_text[type]);
}

/*
 * Function:
 *     _robo_fp_qual_IpProtocolCommon_name
 * Purpose:
 *     Translate IpProtocolCommon enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_IpProtocolCommon_e. (ex.bcmFieldIpProtocolCommonTcp)
 * Returns:
 *     Text name of indicated IpProtocolCommon qualifier enum value.
 */
STATIC char *
_robo_fp_qual_IpProtocolCommon_name(bcm_field_IpProtocolCommon_t type)
{
    /* Text names of the enumerated qualifier IpProtocolCommon values. */
    /* All these are prefixed with "bcmFieldIpProtocolCommon" */
    char *IpProtocolCommon_text[bcmFieldIpProtocolCommonCount] = BCM_FIELD_IPPROTOCOLCOMMON_STRINGS;

    assert(COUNTOF(IpProtocolCommon_text) == bcmFieldIpProtocolCommonCount);

    return (type >= bcmFieldIpProtocolCommonCount ? "??" : IpProtocolCommon_text[type]);
}

/*
 * Function:
 *     _robo_fp_control_name
 * Purpose:
 *     Return text name of indicated control enum value.
 */
STATIC char *
_robo_fp_control_name(bcm_field_control_t control)
{
    /* Text names of Controls. These are used for debugging output and CLIs.
     * Note that the order needs to match the bcm_field_control_t enum order.
     */
    char *control_text[] = BCM_FIELD_CONTROL_STRINGS;
    assert(COUNTOF(control_text)     == bcmFieldControlCount);

    return (control >= bcmFieldControlCount ? "??" : control_text[control]);
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_action(int unit, args_t *args)
{
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.0> fp action ports ... */
    if(!sal_strcasecmp(subcmd, "ports")) {
        return robo_fp_action_ports(unit, args);
    }
    /* BCM.0> fp action add ... */
    if(!sal_strcasecmp(subcmd, "add")) {
        return robo_fp_action_add(unit, args);
    }
    /* BCM.0> fp action get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return robo_fp_action_get(unit, args);
    }
    /* BCM.0> fp action remove... */
    if(!sal_strcasecmp(subcmd, "remove")) {
        return robo_fp_action_remove(unit, args);
    }

    return CMD_USAGE;
}

/*
 * Function:
 *     robo_fp_action_ports
 * Purpose: 
 *     FP action which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
STATIC int
robo_fp_action_ports(int unit, args_t *args)
{
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.0> fp action ports add ... */
    if(!sal_strcasecmp(subcmd, "add")) {
        return robo_fp_action_ports_add(unit, args);
    }
    /* BCM.0> fp action ports get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return robo_fp_action_ports_get(unit, args);
    }

    return CMD_USAGE;
}

/*
 * Function:
 *     robo_fp_action_add
 * Purpose:
 *     Add an action to an entry. Can take action either in the form
 *     of a string or digit corresponding to action order in 
 *     bcm_field_action_t enum.
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
STATIC int
robo_fp_action_add(int unit, args_t *args)
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
            robo_fp_list_actions(unit);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown action: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    /* Read the action parameters (p0 and p1).*/ 
    switch (action) {
        case bcmFieldActionRedirect:
        case bcmFieldActionRpRedirectPort:
        case bcmFieldActionMirrorIngress:
        case bcmFieldActionRpMirrorIngress:
        case bcmFieldActionMirrorEgress:
        case bcmFieldActionGpMirrorIngress:
        case bcmFieldActionGpRedirectPort:
        case bcmFieldActionYpMirrorIngress:
        case bcmFieldActionYpRedirectPort:

            FP_GET_NUMB(p0, subcmd, args);
            if ((subcmd  = ARG_GET(args)) == NULL) { 
                return CMD_USAGE; 
            } 
            if (SOC_IS_ROBO53242(unit) || SOC_IS_ROBO53262(unit)) {
                if (isint(subcmd)) {
                    /* 
                     * Port id 0x3f is a magic number of BCM53242.
                     * It represents flood to all ports.
                     * The magic number need to be processed seperately
                     * and should not do port id validation.
                     */
                    p1 = parse_integer(subcmd);
                    if ((p1 != FP_ACT_CHANGE_FWD_ALL_PORTS) &&
                        (p1 != FP_ACT_CHANGE_FWD_MIRROT_TO_PORT)) {
                        if (parse_port(unit, subcmd, &p1) < 0) {
                            LOG_CLI(("ERROR: invalid port string: \"%s\"\n", subcmd));
                            return CMD_FAIL;
                        }
                    }
                } else if (parse_bcm_port(unit, subcmd, &p1) < 0) {
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META_U(unit,
                                          "FP(unit %d) Error: invalid port string: \"%s\"\n"), 
                               unit, subcmd));
                    return CMD_FAIL;
                }
            } else if (parse_bcm_port(unit, subcmd, &p1) < 0) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: invalid port string: \"%s\"\n"),
                           unit, subcmd));
                return CMD_FAIL;
            }
            break;
        case bcmFieldActionRedirectPbmp:
        case bcmFieldActionEgressMask:
            if ((subcmd  = ARG_GET(args)) == NULL) { 
                return CMD_USAGE; 
            } 
            if (BCM_FAILURE(parse_bcm_pbmp(unit, subcmd, (bcm_pbmp_t*)&p0))) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: Unrecognized port bitmap: \"%s\"\n"),
                           unit, subcmd));
                return CMD_FAIL;
            }
            break;
        case bcmFieldActionDropPrecedence:
        case bcmFieldActionYpDropPrecedence:
        case bcmFieldActionRpDropPrecedence:
        case bcmFieldActionGpDropPrecedence:
            if ((subcmd  = ARG_GET(args)) == NULL) { 
                return CMD_USAGE; 
            }
                p0 = robo_fp_lookup_color(subcmd);
            break;
        case bcmFieldActionUpdateCounter:
            if ((subcmd  = ARG_GET(args)) == NULL) { 
                return CMD_USAGE; 
            }

            p0 = robo_fp_lookup_counter_mode(subcmd);

            break;
        case bcmFieldActionMeterConfig:
            if ((subcmd  = ARG_GET(args)) == NULL) { 
                return CMD_USAGE; 
            }

            p0 = robo_fp_lookup_meter_mode(subcmd);

            if (p0 == BCM_FIELD_METER_MODE_FLOW) {
                if ((subcmd  = ARG_GET(args)) == NULL) { 
                    return CMD_USAGE; 
                }
                p1 = robo_fp_lookup_meter(subcmd);
            }
            break;
        default:
            FP_GET_NUMB(p0, subcmd, args);
            FP_GET_NUMB(p1, subcmd, args);
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

/*
 * Function:
 *     robo_fp_action_ports_add
 * Purpose: 
 *     FP action add which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
STATIC int
robo_fp_action_ports_add(int unit, args_t *args)
{
    char               *subcmd = NULL;
    bcm_field_entry_t   eid;
    bcm_field_action_t  action;
    int                 retval;
    bcm_pbmp_t          pbmp;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    char                buf_pbmp[SOC_PBMP_FMT_LEN];

    FP_GET_NUMB(eid, subcmd, args);
    if ((subcmd  = ARG_GET(args)) == NULL) { 
        return CMD_USAGE; 
    } 

    if (isint(subcmd)) {
        action = parse_integer(subcmd);
    } else {
        action = parse_field_action(subcmd);
        if (action == bcmFieldActionCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown action: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    if (action != bcmFieldActionRedirectPbmp) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unrecognized action\n"),
                   unit));
        return CMD_FAIL;
    }

    /* Read the action parameters pbmp.*/ 
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    } else if (parse_bcm_pbmp(unit, subcmd, &pbmp) < 0) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: %s Error: unrecognized port bitmap: %s\n"),
                   unit, ARG_CMD(args), subcmd));
        return CMD_FAIL;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: action ports add eid=%d, "
                            "action=%s, pbmp=%s\n"), 
                 unit, eid, format_field_action(buf, action, 1), 
                 format_pbmp(unit, buf_pbmp, sizeof(buf_pbmp), pbmp)));

    retval = bcm_field_action_ports_add(unit, eid, action, pbmp);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_ports_add");

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_lookup_color(char *qual_str) {
    assert(qual_str != NULL);
    
    if (isint(qual_str)) {
        return parse_integer(qual_str);
    } else if (!sal_strcasecmp(qual_str, "preserve")) {
        return BCM_FIELD_COLOR_PRESERVE;
    } else if (!sal_strcasecmp(qual_str, "green")) {
        return BCM_FIELD_COLOR_GREEN;
    } else if (!sal_strcasecmp(qual_str, "yellow")) {
        return BCM_FIELD_COLOR_YELLOW;
    } else if (!sal_strcasecmp(qual_str, "red")) {
        return BCM_FIELD_COLOR_RED;
    }

    return -1;
}

/*
 * Function:
 *     Convert symbolic counter mode name to integer value
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_lookup_counter_mode(char *qual_str) {
    assert(qual_str != NULL);

    if (isint(qual_str)) {
        return parse_integer(qual_str);
    } else if (!sal_strcasecmp(qual_str, "default")) {
        return BCM_FIELD_COUNTER_MODE_DEFAULT;
    } else if (!sal_strcasecmp(qual_str, "none")) {
        return BCM_FIELD_COUNTER_MODE_NO_NO;
    } else if (!sal_strcasecmp(qual_str, "lower")) {
        return BCM_FIELD_COUNTER_MODE_NO_YES;
    } else if (!sal_strcasecmp(qual_str, "upper")) {
        return BCM_FIELD_COUNTER_MODE_YES_NO;
    } else if (!sal_strcasecmp(qual_str, "red_notred")) {
        return BCM_FIELD_COUNTER_MODE_RED_NOTRED;
    } else if (!sal_strcasecmp(qual_str, "green_notgreen")) {
        return BCM_FIELD_COUNTER_MODE_GREEN_NOTGREEN;
    } else if (!sal_strcasecmp(qual_str, "green_red")) {
        return BCM_FIELD_COUNTER_MODE_GREEN_RED;
    } else if (!sal_strcasecmp(qual_str, "green_yellow")) {
        return BCM_FIELD_COUNTER_MODE_GREEN_YELLOW;
    } else if (!sal_strcasecmp(qual_str, "red_yellow")) {
        return BCM_FIELD_COUNTER_MODE_RED_YELLOW;
    }

    return -1;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_lookup_meter_mode(char *qual_str) {
    assert(qual_str != NULL);

    if (isint(qual_str)) {
        return parse_integer(qual_str);
    } else if (!sal_strcasecmp(qual_str, "default")) {
        return BCM_FIELD_METER_MODE_DEFAULT;
    } else if (!sal_strcasecmp(qual_str, "flow")) {
        return BCM_FIELD_METER_MODE_FLOW;
    } else if (!sal_strcasecmp(qual_str, "trTCM_blind")) {
        return BCM_FIELD_METER_MODE_trTCM_COLOR_BLIND;
    } else if (!sal_strcasecmp(qual_str, "trTCM_aware")) {
        return BCM_FIELD_METER_MODE_trTCM_COLOR_AWARE;
    } else if (!sal_strcasecmp(qual_str, "srTCM_blind")) {
        return BCM_FIELD_METER_MODE_srTCM_COLOR_BLIND;
    } else if (!sal_strcasecmp(qual_str, "srTCM_aware")) {
        return BCM_FIELD_METER_MODE_srTCM_COLOR_AWARE;
    }

    return -1;
}

/*
 * Function: robo_fp_lookup_meter
 * Purpose:
 *     Convert User string to the choice of either Peak or Committed meter
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_lookup_meter(char *qual_str) {
    assert(qual_str != NULL);

    if (isint(qual_str)) {
        return parse_integer(qual_str);
    } else if (!sal_strcasecmp(qual_str, "peak")) {
        return BCM_FIELD_METER_PEAK;
    } else if (!sal_strcasecmp(qual_str, "committed")) {
        return BCM_FIELD_METER_COMMITTED;
    }

    return -1;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_action_get(int unit, args_t *args)
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
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown action: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    retval = bcm_field_action_get(unit, eid, action, &p0, &p1);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_get");
    LOG_CLI(("FP action get: action=%s, p0=%d, p1=%d\n", 
           format_field_action(buf, action, 1), p0, p1));

    return CMD_OK;
}

/*
 * Function: 
 *     robo_fp_action_ports_get
 * Purpose: 
 *     FP action get which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
STATIC int
robo_fp_action_ports_get(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;
    bcm_pbmp_t          pbmp;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    char                buf_pbmp[SOC_PBMP_FMT_LEN];

    FP_GET_NUMB(eid, subcmd, args);
    if ((subcmd  = ARG_GET(args)) == NULL) { 
        return CMD_USAGE; 
    } 

    if (isint(subcmd)) {
        action = parse_integer(subcmd);
    } else {
        action = parse_field_action(subcmd);
        if (action == bcmFieldActionCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown action: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    if (action != bcmFieldActionRedirectPbmp) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unrecognized action\n"),
                   unit));
        return CMD_FAIL;
    }

    retval = bcm_field_action_ports_get(unit, eid, action, &pbmp);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_ports_get");
    LOG_CLI(("FP action ports get: action=%s, pbmp=%s\n", 
           format_field_action(buf, action, 1), 
           SOC_PBMP_FMT(pbmp, buf_pbmp)));

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_action_remove(int unit, args_t *args)
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
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: Unrecognized action: %s\n"),
                           unit, subcmd));
                return CMD_FAIL;
            }
        }
        retval = bcm_field_action_remove(unit, eid, action);
        FP_CHECK_RETURN(unit, retval, "bcm_field_action_remove");
    }

    return CMD_OK;
}

/*
 * Function:
 *    robo_fp_control
 * Purpose:
 *    Set/Get field control values.
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_control(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_control_t element;
    uint32              status = 0;
    int                 retval = 0;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if (isint(subcmd)) {
        element = parse_integer(subcmd);
    } else {
        robo_fp_lookup_control(subcmd, &element);
        if (element == bcmFieldControlCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown FP control: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    switch (element) {
        case bcmFieldControlStage:
        case bcmFieldControlColorIndependent:
		case bcmFieldControlRedirectIngressVlanCheck:
            break;
        default:
            LOG_CLI(("Control %d unknown\n", element));
            return CMD_OK;
    }

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp control <control_number>*/
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_control_get(element=%s)\n"),
                     unit, _robo_fp_control_name(element)));
        retval = bcm_field_control_get(unit, element, &status);
        FP_CHECK_RETURN(unit, retval, "bcm_field_control_get");
        LOG_CLI(("FP element=%s: status=%d\n", _robo_fp_control_name(element), status));
    } else {
        /* BCM.0> fp control <control_number> <status>*/
        if (element == bcmFieldControlStage && !isint(subcmd)) {
            status = robo_fp_lookup_stage(subcmd);
            if (status == bcmFieldStageCount) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: Unknown stage: %s\n"),
                           unit, subcmd));
                return CMD_FAIL;
            }
    } else {
            status = parse_integer(subcmd);
        }
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_control_set(element=%s, "
                                "status=%d)\n"), 
                     unit, _robo_fp_control_name(element), status));
        retval = bcm_field_control_set(unit, element, status);
        FP_CHECK_RETURN(unit, retval, "bcm_field_control_set");
    }
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_entry(int unit, args_t *args)
{
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp entry create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return robo_fp_entry_create(unit, args);
    }
    /* BCM.0> fp entry copy ... */
    if(!sal_strcasecmp(subcmd, "copy")) {
        return robo_fp_entry_copy(unit, args);
    }
    /* BCM.0> fp entry destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return robo_fp_entry_destroy(unit, args);
    }
    /* BCM.0> fp entry install ... */
    if(!sal_strcasecmp(subcmd, "install")) {
        return robo_fp_entry_install(unit, args);
    }
    /* BCM.0> fp entry reinstall ... */
    if(!sal_strcasecmp(subcmd, "reinstall")) {
        return robo_fp_entry_reinstall(unit, args);
    }
    /* BCM.0> fp entry remove ... */
    if(!sal_strcasecmp(subcmd, "remove")) {
        return robo_fp_entry_remove(unit, args);
    }
    /* BCM.0> fp entry enable ... */
    if(!sal_strcasecmp(subcmd, "enable")) {
        return robo_fp_entry_enable(unit, args);
    }
    /* BCM.0> fp entry disable ... */
    if(!sal_strcasecmp(subcmd, "disable")) {
        return robo_fp_entry_disable(unit, args);
    }
    /* BCM.0> fp entry prio ... */
    if(!sal_strcasecmp(subcmd, "prio")) {
        return robo_fp_entry_prio(unit, args);
    }
    /* BCM.0> fp entry oper ... */
    if(!sal_strcasecmp(subcmd, "oper")) {
        return robo_fp_entry_oper(unit, args);
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
robo_fp_entry_create(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(gid, subcmd, args);
 
    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp entry create 'gid'  */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: _entry_create gid=%d\n"),
                     unit, gid));
        retval = bcm_field_entry_create(unit, gid, &eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_create");
    } else {
        /* BCM.0> fp entry create 'gid' 'eid' */
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

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_entry_copy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           src_eid, dst_eid = -111;

    FP_GET_NUMB(src_eid, subcmd, args);
    subcmd = ARG_GET(args);

    if (subcmd ) {
        /* BCM.0> fp entry copy 'src_eid' 'dst_eid'  */
        dst_eid = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:  bcm_field_entry_copy_id(src_eid=%d, "
                                "dst_eid=%d)\n"),
                     unit, src_eid, dst_eid));
        retval = bcm_field_entry_copy_id(unit, src_eid, dst_eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_copy_id");
    } else {
        /* BCM.0> fp entry copy 'src_eid' */
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

/*
 * Function:
 *    fp_entry_oper
 * Purpose:
 *    Perform entry backup, restore and backup copy free operations
 * Parmameters:
 *    unit - (IN) BCM device number.
 *    args - (IN) Command arguments
 * Returns:
 *    CMD_XXX
 */
STATIC int
robo_fp_entry_oper(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_oper_t      entry_oper;

    bcm_field_entry_oper_t_init(&entry_oper);

    FP_GET_NUMB(entry_oper.entry_id, subcmd, args);
    subcmd = ARG_GET(args);

    if(!sal_strcasecmp(subcmd, "backup")) {
        entry_oper.flags = BCM_FIELD_ENTRY_OPER_BACKUP;
    } else if(!sal_strcasecmp(subcmd, "restore")) {
        entry_oper.flags = BCM_FIELD_ENTRY_OPER_RESTORE;
    } else if(!sal_strcasecmp(subcmd, "cleanup")) {
        entry_oper.flags = BCM_FIELD_ENTRY_OPER_CLEANUP;
    } else {
        return CMD_USAGE;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: bcm_field_entry_operation(eid=%d, "
                            "oper=0x%x)\n"), 
                 unit, entry_oper.entry_id,  entry_oper.flags));
    
    retval = bcm_field_entry_operation(unit, &entry_oper);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_operation");
    
    return CMD_OK;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_entry_destroy(int unit, args_t *args)
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
                                "FP(unit %d) verb: bcm_field_entry_destroy(eid=%d)\n")
                     , unit, eid));
        retval = bcm_field_entry_destroy(unit, eid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_destroy");
        return CMD_OK;
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
robo_fp_entry_install(int unit, args_t *args)
{
    int                         retval = BCM_E_NONE;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;
 
    /* BCM.0> fp detach 'eid' */
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

/*
 * Function:
 *     robo_fp_entry_reinstall
 * Purpose:
 * Parmameters:
 * Returns:
 *     CMD_OK 
 */
STATIC int
robo_fp_entry_reinstall(int unit, args_t *args)
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

/*
 * Function:
 *     robo_fp_entry_remove
 * Purpose:
 * Parmameters:
 * Returns:
 *     CMD_OK 
 */
STATIC int
robo_fp_entry_remove(int unit, args_t *args)
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

/*
 * Function:
 *     robo_fp_entry_enable
 * Purpose:
 *     To enable an entry in hardware.
 * Parmameters:
 * Returns:
 *     CMD_OK 
 */
STATIC int
robo_fp_entry_enable(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(eid, subcmd, args);
            
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _entry_enable_set eid=%d, en=T\n"),
                 unit, eid));
    retval = bcm_field_entry_enable_set(unit, eid, 1);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_enable_set");
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_entry_disable
 * Purpose:
 *     To disable an entry in hardware.
 * Parmameters:
 * Returns:
 *     CMD_OK 
 */
STATIC int
robo_fp_entry_disable(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_GET_NUMB(eid, subcmd, args);
            
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _entry_enable_set eid=%d, en=F\n"),
                 unit, eid));
    retval = bcm_field_entry_enable_set(unit, eid, 0);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_enable_set");
    return CMD_OK;
}


/*
 * Function:
 *     robo_fp_entry_prio
 * Purpose:
 *     CLI interface to bcm_field_entry_prio_get/set()
 * Parmameters:
 * Returns:
 *     CMD_OK 
 */
STATIC int
robo_fp_entry_prio(int unit, args_t *args)
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
        /* BCM.0> fp entry prio <eid> [prio] */
        if (isint(subcmd)) {
            prio = parse_integer(subcmd);
        } else {
            if(!sal_strcasecmp(subcmd, "highest")) {
                prio = BCM_FIELD_ENTRY_PRIO_HIGHEST;
            } else if(!sal_strcasecmp(subcmd, "lowest")) {
                prio = BCM_FIELD_ENTRY_PRIO_LOWEST;
            } else if(!sal_strcasecmp(subcmd, "dontcare")) {
                prio = BCM_FIELD_ENTRY_PRIO_DONT_CARE;
            } else if(!sal_strcasecmp(subcmd, "default")) {
                prio = BCM_FIELD_ENTRY_PRIO_DEFAULT;
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

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char*               subcmd = NULL;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.0> fp group create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return robo_fp_group_create(unit, args, qset);
    }
    /* BCM.0> fp group destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return robo_fp_group_destroy(unit, args);
    }

    /* BCM.0> fp group get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return robo_fp_group_get(unit, args);
    }

    /* BCM.0> fp group set ... */
    if(!sal_strcasecmp(subcmd, "set")) {
        return robo_fp_group_set(unit, args, qset);
    }

    /* BCM.0> fp group status ... */
    if(!sal_strcasecmp(subcmd, "status")) {
        return robo_fp_group_status_get(unit, args);
    }

    /* BCM.0> fp group mode ... */
    if(!sal_strcasecmp(subcmd, "mode")) {
        return robo_fp_group_mode_get(unit, args);
    }

    /* BCM.0> fp group lookup ... */
    if(!sal_strcasecmp(subcmd, "lookup")) {
        return robo_fp_group_lookup(unit, args);
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
robo_fp_group_create(int unit, args_t *args, bcm_field_qset_t *qset)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    char                        buf[FORMAT_PBMP_MAX];
    int                         pri;
    bcm_field_group_t           gid;
    bcm_field_group_mode_t      mode;
    bcm_pbmp_t                  pbmp;
    bcm_port_t                  port, dport;
    int                         count;
 
    FP_GET_NUMB(pri, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp group create 'prio'  */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: _group_create pri=%d\n"),
                     unit, pri));
        retval = bcm_field_group_create(unit, *qset, pri, &gid);
        FP_CHECK_RETURN(unit, retval, "bcm_field_group_create");
    } else {
        gid = parse_integer(subcmd);
        if ((subcmd = ARG_GET(args)) == NULL) {
            /* BCM.0> fp group create 'prio' 'gid' */
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "FP(unit %d) verb: _group_create_id pri=%d gid=%d\n"),
                         unit, pri, gid));
            retval = bcm_field_group_create_id(unit, *qset, pri, gid);
            FP_CHECK_RETURN(unit, retval, "bcm_field_group_create_id");
        } else {
            /* BCM.0> fp group create 'prio' 'gid' 'mode' */
            if (isint(subcmd)) {
                mode = parse_integer(subcmd);
            } else {
                mode = parse_field_group_mode(subcmd);
                if (mode == bcmFieldGroupModeCount) {
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META_U(unit,
                                          "FP(unit %d) Error: Unknown mode: %s\n"),
                               unit, subcmd));
                    return CMD_FAIL;
                }
            }
            /* BCM.0> fp group create 'prio' 'gid' 'mode' 'pbmp' */
            if ((subcmd = ARG_GET(args)) == NULL) {
                LOG_VERBOSE(BSL_LS_BCM_FP,
                            (BSL_META_U(unit,
                                        "FP(unit %d) verb: _group_create_id pri=%d "
                                        "gid=%d, mode=%d\n"), 
                             unit, pri, gid, mode));
            retval = bcm_field_group_create_mode_id(unit, *qset, pri, mode,
                                                    gid);
                FP_CHECK_RETURN(unit, retval, "bcm_field_group_create_mode_id");
            } else {
                if (BCM_FAILURE(parse_bcm_pbmp(unit, subcmd, &pbmp))) {
                    LOG_ERROR(BSL_LS_BCM_FP,
                              (BSL_META_U(unit,
                                          "FP(unit %d) Error: Unrecognized port bitmap: %s\n"),
                               unit, subcmd));
                    return CMD_FAIL;
            }
                BCM_PBMP_COUNT(pbmp, count);

                if (count == 1) {
                    DPORT_BCM_PBMP_ITER(unit, pbmp, dport, port) {
                        LOG_VERBOSE(BSL_LS_BCM_FP,
                                    (BSL_META_U(unit,
                                                "FP(unit %d) verb: _group_port_create_id "
                                                "pri=%d gid=%d, mode=%d, port=%s\n"),
                                     unit, pri, gid, mode, 
                                 format_pbmp(unit, buf, sizeof(buf), pbmp)));
                    
                        retval = bcm_field_group_port_create_mode_id(unit, port,
                                                         *qset, pri, mode, gid);
                        FP_CHECK_RETURN(unit, retval,
                                        "bcm_field_group_ports_create_mode_id");
                }
        } else {
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "FP(unit %d) verb: _group_ports_create_id "
                                            "pri=%d gid=%d, mode=%d"
                                            "pbmp=%s\n"),
                                 unit, pri, gid, mode,
                             format_pbmp(unit, buf,sizeof(buf), pbmp)));
                    retval = bcm_field_group_ports_create_mode_id(unit, pbmp,
                                                     *qset, pri, mode, gid);
                    FP_CHECK_RETURN(unit, retval,
                                    "bcm_field_group_ports_create_mode_id");
            }
        }
        }
    }
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group_destroy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
 
    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group destroy 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:_group_destroy gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_destroy(unit, gid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_destroy");

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_qset_t            qset;
 
    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group create 'prio'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _group_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_get(unit, gid, &qset);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_get");
    robo_fp_qset_show(&qset);
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_range(int unit, args_t *args)
{
    char*               subcmd = NULL;
    
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.0> fp range group create ... */
    if(!sal_strcasecmp(subcmd, "group")) {
        if ((subcmd = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        if(!sal_strcasecmp(subcmd, "create")) {
            return robo_fp_range_group_create(unit, args);
        }
    }
    /* BCM.0> fp range create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return robo_fp_range_create(unit, args);
    }
    /* BCM.0> fp range get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return robo_fp_range_get(unit, args);
    }
    /* BCM.0> fp range destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return robo_fp_range_destroy(unit, args);
    }
    return CMD_USAGE;
}

/*
 * Function:
 *    FP CLI function to create an FP range
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_range_create(int unit, args_t *args)
{
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
        /* BCM.0> fp range create 'flags' 'min' 'max' */
        flags = param[0];
        min   = param[1];
        max   = param[2];
        LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:_range_create flags=0x%x, min=%d, max=%d \n"),
                 unit, flags, min, max));
        retval = bcm_field_range_create(unit, &rid, flags, min, max);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_create");
    } else {
        /* BCM.0> fp range create 'rid' 'flags' 'min' 'max' */
        rid   = param[0];
        flags = param[1];
        min   = param[2];
        max   = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:_range_create_id rid=%d, flags=0x%x, "
                                "min=%d, max=%d \n"),
                     unit, rid, flags, min, max));
        retval = bcm_field_range_create_id(unit, rid, flags, min, max);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_create_id");
    }
    LOG_CLI(("RID %d created!\n", rid));

    return CMD_OK;
}

/*
 * Function:
 *    FP CLI function to create an FP range group
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_range_group_create(int unit, args_t *args)
{
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
        /* BCM.0> fp range group create 'flags' 'min' 'max' */
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
        /* BCM.0> fp range group create 'rid' 'flags' 'min' 'max' */
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

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_range_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;
 
    FP_GET_NUMB(rid, subcmd, args);

    /* BCM.0> fp range get 'rid'  */
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

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_range_destroy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         rid;
 
    FP_GET_NUMB(rid, subcmd, args);

    /* BCM.0> fp range destroy 'rid'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:fp_range_destroy 'rid=%d'\n"),
                 unit, rid));
    retval = bcm_field_range_destroy(unit, rid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_range_destroy");

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group_set(int unit, args_t *args, bcm_field_qset_t *qset)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
 
    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group set 'gid' */
    retval = bcm_field_group_set(unit, gid, *qset);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_set");
 
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group_status_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_group_status_t    gstat;
 
    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group status 'gid' */
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
        LOG_CLI(("entries_free=%d,\n\t\t", gstat.entries_free));
        LOG_CLI(("counters_total=%d,\t",   gstat.counters_total));
        LOG_CLI(("counters_free=%d,\t",    gstat.counters_free));
        LOG_CLI(("meters_total=%d,\t",     gstat.meters_total));
        LOG_CLI(("meters_free=%d",         gstat.meters_free));
        LOG_CLI(("}\n"));
    
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_status_get");
 
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group_mode_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_group_mode_t      mode;
    char                        buf[BCM_FIELD_GROUP_MODE_WIDTH_MAX];

    FP_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group mode 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_mode_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_mode_get(unit, gid, &mode);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_mode_get");
    LOG_CLI(("group mode=%s\n", format_field_group_mode(buf, mode, 1)));
 
    return CMD_OK;
} 

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_list(int unit, args_t *args) {
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp list actions */
    if(!sal_strcasecmp(subcmd, "actions")) {
        return robo_fp_list_actions(unit);
    }
    /* BCM.0> fp list qualifiers */
    if(!sal_strcasecmp(subcmd, "qualifiers") ||
       !sal_strcasecmp(subcmd, "quals")) {
        return robo_fp_list_quals(unit);
    }

    return CMD_USAGE;
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
robo_fp_list_actions(int unit) {
    bcm_field_action_t action;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    char                *param_table[bcmFieldActionCount][2];
    char                *drop_str = "{preserve | green | yellow | red}";
    int                 width_col1 = 20, width_col2 = 35, width_col3= 20;
   
    /* Fill the table with default values for param0 & param1. */
    for (action = 0; action < bcmFieldActionCount; action++) {
        param_table[action][0] = "n/a";
        param_table[action][1] = "n/a";
    }

    param_table[bcmFieldActionCosQNew][0]             = "New CosQ Value";
    param_table[bcmFieldActionRpCosQNew][0]           = "New CosQ Value";
    param_table[bcmFieldActionYpCosQNew][0]           = "New CosQ Value";
    param_table[bcmFieldActionGpCosQNew][0]           = "New CosQ Value";
    param_table[bcmFieldActionCosQCpuNew][0]          = "New CPU CosQ Value";
    param_table[bcmFieldActionVlanCosQNew][0]         = "New VLAN CosQ Value";
    param_table[bcmFieldActionRpVlanCosQNew][0]       = "New VLAN CosQ Value";
    param_table[bcmFieldActionYpVlanCosQNew][0]       = "New VLAN CosQ Value";
    param_table[bcmFieldActionGpVlanCosQNew][0]       = "New VLAN CosQ Value";
    param_table[bcmFieldActionPrioPktAndIntNew][0]    = "New Priority";
    param_table[bcmFieldActionRpPrioPktAndIntNew][0]  = "New Priority";
    param_table[bcmFieldActionYpPrioPktAndIntNew][0]  = "New Priority";
    param_table[bcmFieldActionGpPrioPktAndIntNew][0]  = "New Priority";
    param_table[bcmFieldActionPrioPktNew][0]          = "New Priority";
    param_table[bcmFieldActionRpPrioPktNew][0]        = "New Priority";
    param_table[bcmFieldActionYpPrioPktNew][0]        = "New Priority";
    param_table[bcmFieldActionGpPrioPktNew][0]        = "New Priority";
    param_table[bcmFieldActionPrioIntNew][0]          = "New Priority";
    param_table[bcmFieldActionRpPrioIntNew][0]        = "New Priority";
    param_table[bcmFieldActionYpPrioIntNew][0]        = "New Priority";
    param_table[bcmFieldActionGpPrioIntNew][0]        = "New Priority";
    param_table[bcmFieldActionTosNew][0]              = "New TOS value";
    param_table[bcmFieldActionCopyToCpu][0]           = "non-zero -> matched rule";
    param_table[bcmFieldActionCopyToCpu][1]           = "matched rule ID (0-127)";
    param_table[bcmFieldActionRpCopyToCpu][0]         = "non-zero -> matched rule";
    param_table[bcmFieldActionRpCopyToCpu][1]         = "matched rule ID (0-127)";
    param_table[bcmFieldActionYpCopyToCpu][0]         = "non-zero -> matched rule";
    param_table[bcmFieldActionYpCopyToCpu][1]         = "matched rule ID (0-127)";
    param_table[bcmFieldActionGpCopyToCpu][0]         = "non-zero -> matched rule";
    param_table[bcmFieldActionGpCopyToCpu][1]         = "matched rule ID (0-127)";
    param_table[bcmFieldActionTimeStampToCpu][0]      = "non-zero -> matched rule";
    param_table[bcmFieldActionTimeStampToCpu][1]      = "matched rule ID (0-127)";
    param_table[bcmFieldActionRpTimeStampToCpu][0]    = "non-zero -> matched rule";
    param_table[bcmFieldActionRpTimeStampToCpu][1]    = "matched rule ID (0-127)";
    param_table[bcmFieldActionYpTimeStampToCpu][0]    = "non-zero -> matched rule";
    param_table[bcmFieldActionYpTimeStampToCpu][1]    = "matched rule ID (0-127)";
    param_table[bcmFieldActionGpTimeStampToCpu][0]    = "non-zero -> matched rule";
    param_table[bcmFieldActionGpTimeStampToCpu][1]    = "matched rule ID (0-127)";
    param_table[bcmFieldActionRedirect][0]            = "Dest. Modid";
    param_table[bcmFieldActionRedirect][1]            = "Dest. port";
    param_table[bcmFieldActionOffloadRedirect][0]     = "Dest. Modid";
    param_table[bcmFieldActionOffloadRedirect][1]     = "Dest. port";
    param_table[bcmFieldActionRedirectTrunk][0]       = "Dest. Trunk ID";
    param_table[bcmFieldActionRedirectIpmc][0]        = "Dest. multicast index";
    param_table[bcmFieldActionRedirectMcast][0]       = "Dest. multicast index";
    param_table[bcmFieldActionRedirectPbmp][0]        = "Dest. port bitmap";
    param_table[bcmFieldActionEgressMask][0]          = "Dest. port bitmap";
    param_table[bcmFieldActionIncomingMplsPortSet][0] = "Incoming MPLS port";
    param_table[bcmFieldActionMirrorIngress][0]       = "Dest. Modid";
    param_table[bcmFieldActionMirrorIngress][1]       = "Dest. port/TGID";
    param_table[bcmFieldActionMirrorEgress][0]        = "Dest. Modid";
    param_table[bcmFieldActionMirrorEgress][1]        = "Dest. port/TGID";
    param_table[bcmFieldActionL3ChangeVlan][0]        = "Egress Object Id";
    param_table[bcmFieldActionL3ChangeMacDa][0]       = "Egress Object Id";
    param_table[bcmFieldActionL3Switch][0]            = "Egress Object Id";
    param_table[bcmFieldActionMultipathHash][0]       = "BCM_FIELD_MULTIPATH_HASH_XXX";
    param_table[bcmFieldActionAddClassTag][0]         = "New HG header Classification Tag Value";
    param_table[bcmFieldActionDropPrecedence][0]      = drop_str;
    param_table[bcmFieldActionYpDropPrecedence][0]    = drop_str;
    param_table[bcmFieldActionRpDropPrecedence][0]    = drop_str;
    param_table[bcmFieldActionGpDropPrecedence][0]    = drop_str;
    param_table[bcmFieldActionDscpNew][0]             = "New DSCP value";
    param_table[bcmFieldActionRpDscpNew][0]           = "New DSCP value";
    param_table[bcmFieldActionYpDscpNew][0]           = "New DSCP value";
    param_table[bcmFieldActionGpDscpNew][0]           = "New DSCP value";
    param_table[bcmFieldActionEcnNew][0]              = "New ECN value";
    param_table[bcmFieldActionRpEcnNew][0]            = "New ECN value";
    param_table[bcmFieldActionYpEcnNew][0]            = "New ECN value";
    param_table[bcmFieldActionGpEcnNew][0]            = "New ECN value";
    param_table[bcmFieldActionRpOuterVlanPrioNew][0]  = "New dot1P priority";
    param_table[bcmFieldActionYpOuterVlanPrioNew][0]  = "New dot1P priority";
    param_table[bcmFieldActionGpOuterVlanPrioNew][0]  = "New dot1P priority";
    param_table[bcmFieldActionOuterVlanPrioNew][0]    = "New dot1P priority";
    param_table[bcmFieldActionRpInnerVlanPrioNew][0]  = "New dot1P priority";
    param_table[bcmFieldActionYpInnerVlanPrioNew][0]  = "New dot1P priority";
    param_table[bcmFieldActionGpInnerVlanPrioNew][0]  = "New dot1P priority";
    param_table[bcmFieldActionInnerVlanPrioNew][0]    = "New dot1P priority";
    param_table[bcmFieldActionRpOuterVlanCfiNew][0]   = "New dot1P cfi";
    param_table[bcmFieldActionYpOuterVlanCfiNew][0]   = "New dot1P cfi";
    param_table[bcmFieldActionGpOuterVlanCfiNew][0]   = "New dot1P cfi";
    param_table[bcmFieldActionOuterVlanCfiNew][0]     = "New dot1P cfi";
    param_table[bcmFieldActionRpInnerVlanCfiNew][0]   = "New dot1P cfi";
    param_table[bcmFieldActionYpInnerVlanCfiNew][0]   = "New dot1P cfi";
    param_table[bcmFieldActionGpInnerVlanCfiNew][0]   = "New dot1P cfi";
    param_table[bcmFieldActionInnerVlanCfiNew][0]     = "New dot1P cfi";
    param_table[bcmFieldActionOuterTpidNew][0]        = "New tpid value";
    param_table[bcmFieldActionVlanAdd][0]             = "New vlan id";
    param_table[bcmFieldActionOuterVlanNew][0]        = "New vlan id";
    param_table[bcmFieldActionInnerVlanNew][0]        = "New vlan id";
    param_table[bcmFieldActionOuterVlanLookup][0]     = "Lookup vlan id";
    param_table[bcmFieldActionOuterVlanAdd][0]        = "New vlan id";
    param_table[bcmFieldActionInnerVlanAdd][0]        = "New vlan id";
    param_table[bcmFieldActionVrfSet][0]              = "New vrf id";
    param_table[bcmFieldActionClassDestSet][0]        = "New class id";
    param_table[bcmFieldActionClassSourceSet][0]      = "New class id";
    param_table[bcmFieldActionColorIndependent][0]    = "0(Green)/1(Any Color)";
    

    width_col2 = sal_strlen(drop_str) + 2;

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
    action = bcmFieldActionMeterConfig;
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, format_field_action(buf, action, 1),
           width_col2, "flow", width_col3, "{peak | committed}"));
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, format_field_action(buf, action, 1),
           width_col2, "{trTCM_blind | trTCM_aware | ", width_col3, "n/a"));
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "",
           width_col2, " srTCM_blind | srTCM_aware |", width_col3, "n/a"));
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "",
           width_col2, " default}", width_col3, "n/a"));

    action = bcmFieldActionUpdateCounter;
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, format_field_action(buf, action, 1),
           width_col2, "{lower | upper | ",
           width_col3, "n/a"));
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "",
           width_col2, "red_notred | green_notgreen |",
           width_col3, "n/a"));
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "",
           width_col2, "green_red | green_yellow |",
           width_col3, "n/a"));
    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "",
           width_col2, "red_yellow}",
           width_col3, "n/a"));


    return CMD_OK;
}

/*
 * Function:
 *     fp_list_quals
 * Purpose:
 *     Display a list of Field qualifiers
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_list_quals(int unit)
{
    bcm_field_qualify_t qual;
    char                buf[BCM_FIELD_QUALIFY_WIDTH_MAX];
    int                 width_col1 = 20, width_col2 = 40, width_col3= 20;
    char ***param_table = sal_alloc(bcmFieldQualifyCount * sizeof(char **),
                                    "Qualifier Space");   


    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        param_table[qual] = sal_alloc(2 * sizeof(char *), "Qual Desc");
    }
    /* Fill the table with default values for param0 & param1. */
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        param_table[qual][0] = "n/a";
        param_table[qual][1] = "n/a";
    }

    param_table[bcmFieldQualifySrcIp6][0]          = "Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6][1]          = "IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6][0]          = "Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6][1]          = "IPv6 Address mask";
    param_table[bcmFieldQualifySrcIp6High][0]      = "Top 64-bits of Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6High][1]      = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6High][0]      = "Top 64-bits of Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6High][1]      = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifySrcIp6Low][0]      = "Low 64-bits of Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6Low][1]      = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6Low][0]      = "Low 64-bits of Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6Low][1]      = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifySrcIp][0]           = "Source IPv4 Address";
    param_table[bcmFieldQualifySrcIp][1]           = "IPv4 Address mask";
    param_table[bcmFieldQualifyDstIp][0]           = "Destination IPv4 Address";
    param_table[bcmFieldQualifyDstIp][1]           = "IPv4 Address";
    param_table[bcmFieldQualifyInPort][0]          = "Single Input Port";
    param_table[bcmFieldQualifyInPort][1]          = "Port Mask";
    param_table[bcmFieldQualifyOutPort][0]         = "Single Output Port";
    param_table[bcmFieldQualifyOutPort][1]         = "Port Mask";
    param_table[bcmFieldQualifyInPorts][0]         = "Input Port Bitmap";
    param_table[bcmFieldQualifyInPorts][1]         = "Port Bitmap Mask";
    param_table[bcmFieldQualifyOutPorts][0]        = "Output Port Bitmap";
    param_table[bcmFieldQualifyOutPorts][1]        = "Port Bitmap Mask";
    param_table[bcmFieldQualifyDrop][0]            = "0 or 1";
    param_table[bcmFieldQualifyDrop][1]            = "0 or 1";
    param_table[bcmFieldQualifyLoopback][0]        = "0 or 1";
    param_table[bcmFieldQualifyLoopback][1]        = "0 or 1";
    param_table[bcmFieldQualifyIp6FlowLabel][0]    = "20-bit IPv6 Flow Label";
    param_table[bcmFieldQualifyIp6FlowLabel][1]    = "20-bit mask";
    param_table[bcmFieldQualifyIp6HopLimit][0]     = "8-bit IPv6 Hop Limit";
    param_table[bcmFieldQualifyIp6HopLimit][1]     = "8-bit mask";
    param_table[bcmFieldQualifyOuterVlan][0]       = "Outer VLAN tag";
    param_table[bcmFieldQualifyOuterVlan][1]       = "16-bit mask";
    param_table[bcmFieldQualifyInnerVlan][0]       = "Inner VLAN tag";
    param_table[bcmFieldQualifyInnerVlan][1]       = "16-bit mask";
    param_table[bcmFieldQualifyRangeCheck][0]      = "Range ID";
    param_table[bcmFieldQualifyRangeCheck][1]      = "Normal=0, Invert=1";
    param_table[bcmFieldQualifyL4SrcPort][0]       = "TCP/UDP Source port";
    param_table[bcmFieldQualifyL4SrcPort][1]       = "16-bit mask";
    param_table[bcmFieldQualifyL4DstPort][0]       = "TCP/UDP Destination port";
    param_table[bcmFieldQualifyL4DstPort][1]       = "16-bit mask";
    param_table[bcmFieldQualifyEtherType][0]       = "Ethernet Type";
    param_table[bcmFieldQualifyEtherType][1]       = "16-bit mask";
    param_table[bcmFieldQualifyL4Ports][0]         = "L4 ports valid bit";
    param_table[bcmFieldQualifyL4Ports][1]         = "L4 ports valid bit mask";
    param_table[bcmFieldQualifyMirrorCopy][0]      = "Mirrored only";
    param_table[bcmFieldQualifyMirrorCopy][1]      = "Mirrored only mask";
    param_table[bcmFieldQualifyTunnelTerminated][0]= "Tunnel terminated";
    param_table[bcmFieldQualifyTunnelTerminated][1]= "Tunnel terminated mask";
    param_table[bcmFieldQualifyMplsTerminated][0]  = "Mpls terminated";
    param_table[bcmFieldQualifyMplsTerminated][1]  = "Mpls terminated mask";
    param_table[bcmFieldQualifyIpProtocol][0]      = "IP protocol field";
    param_table[bcmFieldQualifyIpProtocol][1]      = "8-bit mask";
    param_table[bcmFieldQualifyDSCP][0]            = "Differential Code Point";
    param_table[bcmFieldQualifyDSCP][1]            = "8-bit mask";
    param_table[bcmFieldQualifyVlanFormat][0]      = "Vlan tag format";
    param_table[bcmFieldQualifyVlanFormat][1]      = "8-bit mask";
    param_table[bcmFieldQualifyTranslatedVlanFormat][0]      = "Vlan tag format";
    param_table[bcmFieldQualifyTranslatedVlanFormat][1]      = "8-bit mask";
    param_table[bcmFieldQualifyIntPriority][0]     = "Internal priority";
    param_table[bcmFieldQualifyIntPriority][1]     = "8-bit mask";
    param_table[bcmFieldQualifyTtl][0]             = "Time to live";
    param_table[bcmFieldQualifyTtl][1]             = "8-bit mask";
    param_table[bcmFieldQualifyIp6NextHeader][0]   = "IPv6 Next Header";
    param_table[bcmFieldQualifyIp6NextHeader][1]   = "8-bit mask";
    param_table[bcmFieldQualifyIp6TrafficClass][0] = "IPv6 Next Header";
    param_table[bcmFieldQualifyIp6TrafficClass][1] = "8-bit mask";
    param_table[bcmFieldQualifyIp6HopLimit][0]     = "IPv6 Hop Limit";
    param_table[bcmFieldQualifyIp6HopLimit][1]     = "8-bit mask";
    param_table[bcmFieldQualifySrcModid][0]        = "Source Module ID";
    param_table[bcmFieldQualifySrcModid][1]        = "6-bit mask";
    param_table[bcmFieldQualifySrcPortTgid][0]     = "Source Port or Trunk Group ID";
    param_table[bcmFieldQualifySrcPortTgid][1]     = "6-bit mask";
    param_table[bcmFieldQualifySrcTrunk][0]        = "Source Trunk Group ID";
    param_table[bcmFieldQualifySrcTrunk][1]        = "8-bit mask";
    param_table[bcmFieldQualifyDstModid][0]        = "Destination Module ID";
    param_table[bcmFieldQualifyDstModid][1]        = "6-bit mask";
    param_table[bcmFieldQualifyDstPortTgid][0]     = "Destination Port or Trunk Group ID";
    param_table[bcmFieldQualifyDstPortTgid][1]     = "6-bit mask";
    param_table[bcmFieldQualifyDstTrunk][0]        = "Destination Trunk Group ID";
    param_table[bcmFieldQualifyDstTrunk][1]        = "8-bit mask";
    param_table[bcmFieldQualifyTcpControl][0]      = "TCP control flags";
    param_table[bcmFieldQualifyTcpControl][1]      = "8-bit mask";
    param_table[bcmFieldQualifyPacketFormat][0]    = "Packet Format";
    param_table[bcmFieldQualifyPacketFormat][1]    = "6-bit mask";
    param_table[bcmFieldQualifySrcClassL2][0]      = "Source L2 class";
    param_table[bcmFieldQualifySrcClassL2][1]      = "Source L2 class mask";
    param_table[bcmFieldQualifySrcClassL3][0]      = "Source L3 class";
    param_table[bcmFieldQualifySrcClassL3][1]      = "Source L3 class mask";
    param_table[bcmFieldQualifySrcClassField][0]   = "Source Field class";
    param_table[bcmFieldQualifySrcClassField][1]   = "Source Field class mask";
    param_table[bcmFieldQualifyDstClassL2][0]      = "Destination L2 class";
    param_table[bcmFieldQualifyDstClassL2][1]      = "Destination L2 class mask";
    param_table[bcmFieldQualifyDstClassL3][0]      = "Destination L3 class";
    param_table[bcmFieldQualifyDstClassL3][1]      = "Destination L3 class mask";
    param_table[bcmFieldQualifyDstClassField][0]   = "Destination Field class";
    param_table[bcmFieldQualifyDstClassField][1]   = "Destination Field class mask";
    param_table[bcmFieldQualifyMHOpcode][0]        = "Module Header opcodes";
    param_table[bcmFieldQualifyMHOpcode][1]        = "3-bit mask";
    param_table[bcmFieldQualifyIpFlags][0]         = "IPv4 Flags";
    param_table[bcmFieldQualifyIpFlags][1]         = "3-bit mask";
    param_table[bcmFieldQualifyIpType][0]          = "bcm_field_IpType_t";
    param_table[bcmFieldQualifyL2Format][0]        = "bcm_field_L2Format_t";
    param_table[bcmFieldQualifyDecap][0]           = "bcm_field_decap_t";
    param_table[bcmFieldQualifyHiGig][0]           = "HiGig=1, non-HiGig=0";
    param_table[bcmFieldQualifyHiGig][1]           = "1/0";
    param_table[bcmFieldQualifyDstHiGig][0]        = "HiGig=1, non-HiGig=0";
    param_table[bcmFieldQualifyDstHiGig][1]        = "1/0";
    param_table[bcmFieldQualifyInnerTpid][0]       = "Inner vlan tag tpid";
    param_table[bcmFieldQualifyOuterTpid][0]       = "Outer vlan tag tpid";
    param_table[bcmFieldQualifyStage][0]           = "bcm_field_stage_t";
    param_table[bcmFieldQualifyStageIngress][0]    = "Ingress FP pipeline stage";
    param_table[bcmFieldQualifyStageLookup][0]     = "Lookup FP pipeline stage";
    param_table[bcmFieldQualifyStageEgress][0]     = "Egress FP pipeline stage";
    param_table[bcmFieldQualifySrcIpEqualDstIp][0] = "1 if SrcIp==DstIp, 0 otherwise";
    param_table[bcmFieldQualifyEqualL4Port][0]     = "1 if L4 Src.==Dst., 0 otherwise"; 
    param_table[bcmFieldQualifyTcpSequenceZero][0] = "1 if TCP Sequence#==0, 0 if !=";
    param_table[bcmFieldQualifyTcpSequenceZero][0] = "1 if TCP Sequence#==0, 0 if !=";
    param_table[bcmFieldQualifyDstL3Egress][0]    = "Destination L3 Egress id.";
    param_table[bcmFieldQualifyDstMplsGport][0]    = "Destination mpls gport.";
    param_table[bcmFieldQualifyDstMimGport][0]    = "Destination mim gport.";
    param_table[bcmFieldQualifyDstWlanGport][0]    = "Destination wlan gport.";
    param_table[bcmFieldQualifyDstMulticastGroup][0]  = 
                                          "Destination multicast group.";
    param_table[bcmFieldQualifySrcMplsGport][0]    = "Source mpls gport.";
    param_table[bcmFieldQualifySrcMimGport][0]     = "Source mim gport.";
    param_table[bcmFieldQualifySrcWlanGport][0]    = "Source wlan gport.";
    param_table[bcmFieldQualifySrcModPortGport][0]    = "Source mod port gport.";
    param_table[bcmFieldQualifyInterfaceClassL2][0]    = "Interface Class L2.";
    param_table[bcmFieldQualifyInterfaceClassL2][1]    = "Interface Class L2.";
    param_table[bcmFieldQualifyInterfaceClassL3][0]    = "Interface Class L3.";
    param_table[bcmFieldQualifyInterfaceClassL3][1]    = "Interface Class L3.";
    param_table[bcmFieldQualifyInterfaceClassPort][0]  = "Interface Class Port.";
    param_table[bcmFieldQualifyInterfaceClassPort][1]  = "Interface Class Port.";
    param_table[bcmFieldQualifyTcpHeaderSize][0]   = "TCP Size";
    param_table[bcmFieldQualifyTcpHeaderSize][1]   = "8-bit mask";
    param_table[bcmFieldQualifyVrf][0]             = "VRF id";
    param_table[bcmFieldQualifyVrf][1]             = "VRF id mask";
    param_table[bcmFieldQualifyExtensionHeaderType][0] = "Next Header In Ext Hdr";
    param_table[bcmFieldQualifyExtensionHeaderType][1] = "Next Header byte mask";
    param_table[bcmFieldQualifyExtensionHeaderSubCode][0] = "Next Header Sub Code";
    param_table[bcmFieldQualifyExtensionHeaderSubCode][1] = "Next Header Sub Code mask";
    param_table[bcmFieldQualifyL3Routable][0]      = "1 should be L3 routed 0 otherwise";
    param_table[bcmFieldQualifyL3Routable][1]      = "Routed mask";
    param_table[bcmFieldQualifyDosAttack][0]       = "1 DOS attac 0 otherwise";
    param_table[bcmFieldQualifyDosAttack][1]       = "Dos attack mask";
    param_table[bcmFieldQualifyIpmcStarGroupHit][0] = "1 Star, G hit 0 otherwise";
    param_table[bcmFieldQualifyIpmcStarGroupHit][1] = "Star, G entry hit mask";
    param_table[bcmFieldQualifyL3DestRouteHit][0]  = "1 L3 dest route table hit, 0 otherwise";
    param_table[bcmFieldQualifyL3DestRouteHit][1]  = "L3 dest route table hit mask";
    param_table[bcmFieldQualifyL3DestHostHit][0]   = "1 L3 dest host table hit, 0 otherwise";
    param_table[bcmFieldQualifyL3DestHostHit][1]   = "L3 dest host table hit mask";
    param_table[bcmFieldQualifyL3SrcHostHit][0]    = "1 L3 source host table hit , 0 otherwise";
    param_table[bcmFieldQualifyL3SrcHostHit][1]    = "L3 source host table hit mask";
    param_table[bcmFieldQualifyL2CacheHit][0]      = "1 L2 dest cache hit, 0 otherwise";
    param_table[bcmFieldQualifyL2CacheHit][1]      = "L2 dest cache hit mask";
    param_table[bcmFieldQualifyL2StationMove][0]   = "1 L2 src station move, 0 otherwise";
    param_table[bcmFieldQualifyL2StationMove][1]   = "L2 src station move mask";
    param_table[bcmFieldQualifyL2SrcHit][0]        = "1 L2 src lookup success, 0 otherwise";
    param_table[bcmFieldQualifyL2SrcHit][1]        = "L2 src lookup mask";
    param_table[bcmFieldQualifyL2DestHit][0]       = "1 L2 dest lookup success, 0 otherwise";
    param_table[bcmFieldQualifyL2DestHit][1]       = "L2 dest lookup mask";
    param_table[bcmFieldQualifyL2SrcStatic][0]     = "1 L2 src static, 0 otherwise";
    param_table[bcmFieldQualifyL2SrcStatic][1]     = "L2 src static mask";
    param_table[bcmFieldQualifyIngressStpState][0] = "BCM_STG_STP_XXX";
    param_table[bcmFieldQualifyIngressStpState][1] = "STG Stp state mask";
    param_table[bcmFieldQualifyForwardingVlanValid][0] = "Forwarding vlan id valid.";
    param_table[bcmFieldQualifyForwardingVlanValid][1] = "Forwarding vlan id valid mask";
    param_table[bcmFieldQualifyVlanTranslationHit][0] = "Vlan Translation table hit.";
    param_table[bcmFieldQualifyVlanTranslationHit][1] = "Vlan Translation table hit mask";
    param_table[bcmFieldQualifyFlowId][0]   = "Flow id";
    param_table[bcmFieldQualifyFlowId][1]   = "12-bit mask";
    param_table[bcmFieldQualifyInVPort][0]   = "Ingress virtual port id";
    param_table[bcmFieldQualifyInVPort][1]   = "8-bit mask";
    param_table[bcmFieldQualifyOutVPort][0]   = "Egress virtual port id";
    param_table[bcmFieldQualifyOutVPort][1]   = "8-bit mask";

    LOG_CLI(("%-*s%-*s%-*s\n", width_col1, "QUALIFIER", width_col2, "DATA",
           width_col3, "MASK"));
    /* Print the normal 2 parameter qualifiers. */
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if (qual == bcmFieldQualifySrcPort ||
            qual == bcmFieldQualifyDstPort ||
            qual == bcmFieldQualifySrcMac ||
            qual == bcmFieldQualifyDstMac ||
            qual == bcmFieldQualifyPacketRes || 
            qual == bcmFieldQualifyIpFrag || 
            qual == bcmFieldQualifyLoopbackType || 
            qual == bcmFieldQualifyTunnelType || 
            qual == bcmFieldQualifyIpInfo || 
            qual == bcmFieldQualifyLookupStatus) {
            continue;
        }
        LOG_CLI(("%-*s%-*s%-*s\n",
               width_col1, format_field_qualifier(buf, qual, 1),
               width_col2, param_table[qual][0],
               width_col3, param_table[qual][1]));
    }

    /* Print the qualifiers that use parse tables. */
    robo_fp_qual_mac_help("", "SrcMac", width_col1);
    robo_fp_qual_mac_help("", "DstMac", width_col1);
    robo_fp_qual_PacketRes_help("", width_col1);
    robo_fp_qual_IpFrag_help("", width_col1);
    robo_fp_qual_LoopbackType_help("", width_col1);
    robo_fp_qual_TunnelType_help("", width_col1);
    robo_fp_qual_modport_help("", "SrcPort", width_col1);
    robo_fp_qual_modport_help("", "DstPort", width_col1);
    robo_fp_qual_IpInfo_help("", width_col1);
    robo_fp_qual_Color_help("", width_col1);

    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        sal_free(param_table[qual]);
    }    
    sal_free(param_table);
    return CMD_OK;
}

/*
 * Function:
 *     fp_print_options
 * Purpose:
 *     Print an array of options bracketed by <>.
 * Parmameters:
 *     options - NULL terminated array of pointers to strings.
 * Returns:
 *     Nothing
 */
STATIC void
robo_fp_print_options(const char *options[], const int offset) 
{
    int                 idx;
    int                 char_count = offset;

    for (idx = 0; options[idx] != NULL; idx++) {
        char_count += cli_out("%s%s", idx == 0 ? "<" : " | ", options[idx]);
        if (char_count >= FP_LINE_SZ) {
            cli_out("\n%-*s", offset, "");
            char_count -= FP_LINE_SZ;
        }
    }

    cli_out(">");

    return;
}

/*
 * Function:
 *     robo_fp_group_lookup
 * Purpose:
 *     Test getting/setting the FP group packet lookup enable/disable APIs.
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_group_lookup(int unit, args_t *args)
{
    char*                       subcmd = NULL;
    int                         retval = CMD_OK;
    bcm_field_group_t           gid;
    int                         enable;
    
    FP_GET_NUMB(gid, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp group lookup 'gid' */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:bcm_field_group_enable_get(gid=%d)\n"),
                     unit, gid));
        retval = bcm_field_group_enable_get(unit, gid, &enable);
        FP_CHECK_RETURN(unit, retval, "bcm_field_group_enable_get");
        if (enable) {
            LOG_CLI(("GID %d: lookup=Enabled\n", gid));
        } else {
            LOG_CLI(("GID %d: lookup=Disabled\n", gid));
        }
 
        return CMD_OK;
    }

    /* BCM.0> fp group lookup 'gid' enable */
    if(!sal_strcasecmp(subcmd, "enable")) {
        return robo_fp_group_enable_set(unit, gid, 1);
    }

    /* BCM.0> fp group lookup 'gid' disable */
    if(!sal_strcasecmp(subcmd, "disable")) {
        return robo_fp_group_enable_set(unit, gid, 0);
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
robo_fp_group_enable_set(int unit, bcm_field_group_t gid, int enable)
{
    int                         retval = CMD_OK;

    /* BCM.0> fp group enable/disable 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_enable_set(gid=%d, "
                            "enable=%d)\n"),
                 unit, gid, enable));
    retval = bcm_field_group_enable_set(unit, gid, enable);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_enable_set");
 
    return CMD_OK;
} 

/*
 * Function:
 *     robo_fp_qset
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qset(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char*                 subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp qset clear */
    if(!sal_strcasecmp(subcmd, "clear")) {
        BCM_FIELD_QSET_INIT(*qset);
        LOG_CLI(("BCM_FIELD_QSET_INIT() okay\n"));
        return CMD_OK;
    }
    /* BCM.0> fp qset add ...*/
    if(!sal_strcasecmp(subcmd, "add")) {
        return robo_fp_qset_add(unit, args, qset);
    }
    /* BCM.0> fp qset set ...*/
    if(!sal_strcasecmp(subcmd, "set")) {
        return robo_fp_qset_set(unit, args, qset);
    }
    /* BCM.0> fp qset show */
    if(!sal_strcasecmp(subcmd, "show")) {
        return robo_fp_qset_show(qset);
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
robo_fp_qset_set(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char                  *qual_str = NULL;
    char                  *buf;

    if ((qual_str = ARG_GET(args)) == NULL) {
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
/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qset_add(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char                  *qual_str = NULL;
    char                  buf[BCM_FIELD_QUALIFY_WIDTH_MAX];
    bcm_field_qualify_t   qual;
    int                   dq_id;
    char                  *subcmd   = NULL;
    int                   retval; 

    if ((qual_str = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* 
     * Argument exception: "Data " does not correspond directly to 
     * any bcmFieldQualify* enum.
     */
    if(!sal_strcasecmp(qual_str, "Data")) {
        FP_GET_NUMB(dq_id, subcmd, args);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:fp_qset_add: data qualifier=%d\n"),
                     unit, dq_id));
        retval = bcm_field_qset_data_qualifier_add(unit, qset, dq_id);
        FP_CHECK_RETURN(unit, retval, "bcm_field_qset_data_qualifier_add");
        return CMD_OK;
    }

    if (isint(qual_str)) {
        qual = parse_integer(qual_str);
    } else {
        qual = parse_field_qualifier(qual_str);

        if (qual == bcmFieldQualifyCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown qualifier: %s\n"),
                       unit, qual_str));
            return CMD_FAIL;
        }
    }

    BCM_FIELD_QSET_ADD(*qset, qual);
    LOG_CLI(("BCM_FIELD_QSET_ADD(%s) okay\n",
           format_field_qualifier(buf, qual, 1)));
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qset_show(bcm_field_qset_t *qset)
{
#ifdef BROADCOM_DEBUG
    _robo_field_qset_dump("qset=", *qset, "\n");
#endif /* BROADCOM_DEBUG */

    return CMD_OK;
}

/*
 * Function:
 *    robo_fp_lookup_control
 * Purpose:
 *    Lookup a control from a user string.
 * Parmameters:
 * Returns:
 */
STATIC void
robo_fp_lookup_control(const char *control_str, bcm_field_control_t *control)
{
    char                   tbl_str[FP_STAT_STR_SZ];
    char                   lng_str[FP_STAT_STR_SZ];

    assert(control_str != NULL);
    assert(sal_strlen(control_str) < FP_STAT_STR_SZ - 1);
    assert(control != NULL);

    for (*control = 0; *control < bcmFieldControlCount; (*control)++) {
        sal_memset(tbl_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        sal_memset(lng_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        /* Test for the suffix only */
        strncpy(tbl_str, _robo_fp_control_name(*control), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, control_str)) {
            break;
        }
        /* Test for whole name of the control */
        sal_strncpy(lng_str, "bcmFieldControl", sal_strlen("bcmFieldControl"));
        lng_str[sal_strlen("bcmFieldControl")] = '\0';
        strncat(lng_str, tbl_str,
                FP_STAT_STR_SZ - 1 - sal_strlen("bcmFieldControl"));
        if (!sal_strcasecmp(lng_str, control_str)) {
            break;
        }
    }

    /* If not found, result is bcmFieldActionCount */
}

/*
 * Function:
 *    robo_fp_lookup_L2Format
 * Purpose:
 *    Lookup a field L2Format value from a user string.
 * Parmameters:
 *     type_str - search string
 * Returns:
 *     corresponding L2Format value
 *    
 */
STATIC bcm_field_L2Format_t
robo_fp_lookup_L2Format(const char *type_str)
{
    char                   tbl_str[FP_STAT_STR_SZ];
    char                   lng_str[FP_STAT_STR_SZ];
    const char             *prefix = "bcmFieldL2Format";
    bcm_field_L2Format_t    type;

    assert(type_str != NULL);
    assert(sal_strlen(type_str) < FP_STAT_STR_SZ - 1);

    for (type = 0; type < bcmFieldL2FormatCount; (type)++) {
        /* Test for the suffix only */
        sal_memset(tbl_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        strncpy(tbl_str, _robo_fp_qual_L2Format_name(type), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, type_str)) {
            break;
        }
        /* Test for whole name of the L2Format*/
        sal_memset(lng_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        strncpy(lng_str, prefix, sal_strlen(prefix));
        strncat(lng_str, tbl_str, FP_STAT_STR_SZ - 1 - sal_strlen(prefix));
        if (!sal_strcasecmp(lng_str, type_str)) {
            break;
        }
    }

    /* If not found, result is bcmFieldL2FormatCount */
    return type;
}


/*
 * Function:
 *    robo_fp_lookup_IpType
 * Purpose:
 *    Lookup a field IpType value from a user string.
 * Parmameters:
 *     type_str - search string
 * Returns:
 *     corresponding IpType value
 *    
 */
STATIC bcm_field_IpType_t
robo_fp_lookup_IpType(const char *type_str)
{
    char                   tbl_str[FP_STAT_STR_SZ];
    char                   lng_str[FP_STAT_STR_SZ];
    const char             *prefix = "bcmFieldIpType";
    bcm_field_IpType_t     type;

    assert(type_str != NULL);
    assert(sal_strlen(type_str) < FP_STAT_STR_SZ - 1);

    for (type = 0; type < bcmFieldIpTypeCount; (type)++) {
        /* Test for the suffix only */
        sal_memset(tbl_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        strncpy(tbl_str, _robo_fp_qual_IpType_name(type), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, type_str)) {
            break;
        }
        /* Test for whole name of the IpType*/
        sal_memset(lng_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        strncpy(lng_str, prefix, sal_strlen(prefix));
        strncat(lng_str, tbl_str, FP_STAT_STR_SZ - 1 - sal_strlen(prefix));
        if (!sal_strcasecmp(lng_str, type_str)) {
            break;
        }
    }

    /* If not found, result is bcmFieldIpTypeCount */
    return type;
}

/*
 * Function:
 *    robo_fp_lookup_IpProtocolCommon
 * Purpose:
 *    Lookup a field IpProtocolCommon value from a user string.
 * Parmameters:
 *     type_str - search string
 * Returns:
 *     corresponding IpProtocolCommon value
 *    
 */
STATIC bcm_field_IpProtocolCommon_t
robo_fp_lookup_IpProtocolCommon(const char *type_str)
{
    char                   tbl_str[FP_STAT_STR_SZ];
    char                   lng_str[FP_STAT_STR_SZ];
    const char             *prefix = "bcmFieldIpProtocolCommon";
    bcm_field_IpProtocolCommon_t     type;

    assert(type_str != NULL);
    assert(strlen(type_str) < FP_STAT_STR_SZ - 1);

    for (type = 0; type < bcmFieldIpProtocolCommonCount; (type)++) {
        /* Test for the suffix only */
        sal_memset(tbl_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        strncpy(tbl_str, _robo_fp_qual_IpProtocolCommon_name(type), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, type_str)) {
            break;
        }
        /* Test for whole name of the IpProtocolCommon*/
        sal_memset(lng_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        strncpy(lng_str, prefix, strlen(prefix));
        strncat(lng_str, tbl_str, FP_STAT_STR_SZ - 1 - strlen(prefix));
        if (!sal_strcasecmp(lng_str, type_str)) {
            break;
        }
    }

    /* If not found, result is bcmFieldIpProtocolCommonCount */
    return type;
}


/*
 * Function:
 *    robo_fp_lookup_stage
 * Purpose:
 *    Lookup a field stage value from a user string.
 * Parmameters:
 *     stage_str - search string
 *     stage     - (OUT) corresponding stage value
 * Returns:
 *    
 */
STATIC bcm_field_stage_t
robo_fp_lookup_stage(const char *stage_str) 
{
    char                   tbl_str[FP_STAT_STR_SZ];
    char                   lng_str[FP_STAT_STR_SZ];
    bcm_field_stage_t      stage;

    assert(stage_str != NULL);
    assert(sal_strlen(stage_str) < FP_STAT_STR_SZ - 1);

    for (stage = 0; stage < bcmFieldStageCount; stage++) {
        sal_memset(tbl_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        sal_memset(lng_str, 0, sizeof(char) * FP_STAT_STR_SZ);
        /* Test for the suffix only */
        strncpy(tbl_str, _robo_fp_qual_stage_name(stage), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, stage_str)) {
            break;
        }
        /* Test for whole name of the Stage */
        sal_strncpy(lng_str, "bcmFieldStage", sal_strlen("bcmFieldStage"));
        lng_str[sal_strlen("bcmFieldStage")] = '\0';
        strncat(lng_str, tbl_str,
                FP_STAT_STR_SZ - 1 - sal_strlen("bcmFieldStage"));
        if (!sal_strcasecmp(lng_str, stage_str)) {
            break;
        }
    }

    /* If not found, result is bcmFieldStageCount */
    return stage;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual(int unit, args_t *args)
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
        /* BCM.0> fp qual 'eid' clear  */
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
        /* BCM.0> fp qual 'eid' delete 'qual_name' */
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

    /* 
     * Argument exception: "Data" does not correspond directly to 
     * any bcmFieldQualify* enum.
     */
    if(!sal_strcasecmp(qual_str, "Data")) {
        return robo_fp_qual_data(unit, eid, args);
    }

    subcmd = ARG_GET(args);
    if (subcmd != NULL) {
        if ((!sal_strcasecmp(subcmd, "show")) || 
            (!sal_strcasecmp(subcmd, "?"))) {
            switch (parse_field_qualifier(qual_str)) {
                case bcmFieldQualifyIpInfo:
                    return robo_fp_qual_IpInfo_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyPacketRes:
                    return robo_fp_qual_PacketRes_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyIpFrag:
                    return robo_fp_qual_IpFrag_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyLoopbackType:
                    return robo_fp_qual_LoopbackType_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyTunnelType:
                    return robo_fp_qual_TunnelType_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifySrcPort:
                    return robo_fp_qual_modport_help("fp qual <eid> ", "SrcPort", 0);
                    break;
                case bcmFieldQualifyDstPort:
                    return robo_fp_qual_modport_help("fp qual <eid> ", "DstPort", 0);
                    break;
                case bcmFieldQualifySrcMac:
                    return robo_fp_qual_mac_help("fp qual <eid> ", "SrcMac", 0);
                    break;
                case bcmFieldQualifyDstMac:
                    return robo_fp_qual_mac_help("fp qual <eid> ", "DstMac", 0);
                    break;
                default:
                    return robo_fp_list_quals(unit);
            }
        } else {
            ARG_PREV(args);
        }
    }

    /* > fp qual 'eid' bcmFieldQualifyXXX ...*/
    switch (parse_field_qualifier(qual_str)) {
    case bcmFieldQualifyInPort:
        rv = robo_fp_qual_port(unit, eid, args, bcm_field_qualify_InPort,
                     "InPort");
        break;
    case bcmFieldQualifyOutPort:
        rv = robo_fp_qual_port(unit, eid, args, bcm_field_qualify_OutPort,
                     "OutPort");
        break;
    case bcmFieldQualifyInPorts:
        rv = robo_fp_qual_InPorts(unit, eid, args);
        break;
    case bcmFieldQualifyOutPorts:
        rv = robo_fp_qual_OutPorts(unit, eid, args);
        break;
    case bcmFieldQualifyDrop:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_Drop,
                  "Drop");
        break;
    case bcmFieldQualifyLoopback:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_Loopback,
                  "Loopback");
        break;
    case bcmFieldQualifySrcPort:
        rv = robo_fp_qual_modport(unit, eid, args, bcm_field_qualify_SrcPort,
                     "SrcPort");
        break;
    case bcmFieldQualifySrcTrunk:
        rv = robo_fp_qual_trunk(unit, eid, args, bcm_field_qualify_SrcTrunk,
                     "SrcTrunk");
        break;
    case bcmFieldQualifyDstPort:
        rv = robo_fp_qual_modport(unit, eid, args, bcm_field_qualify_DstPort,
                     "DstPort");
        break;
    case bcmFieldQualifyDstTrunk:
        rv = robo_fp_qual_trunk(unit, eid, args, bcm_field_qualify_DstTrunk,
                     "DstTrunk");
        break;
    case bcmFieldQualifyL4SrcPort:
        rv = robo_fp_qual_l4port(unit, eid, args, bcm_field_qualify_L4SrcPort,
                       "L4SrcPort");
        break;
    case bcmFieldQualifyL4DstPort:
        rv = robo_fp_qual_l4port(unit, eid, args, bcm_field_qualify_L4DstPort,
                       "L4DstPort");
        break;
    case bcmFieldQualifyOuterVlan:
        rv = robo_fp_qual_vlan(unit, eid, args, bcm_field_qualify_OuterVlan,
                     "OuterVlan");
        break;
    case bcmFieldQualifyOuterVlanId:
        rv = robo_fp_qual_vlan(unit, eid, args, bcm_field_qualify_OuterVlanId,
                     "OuterVlanId");
        break;
    case bcmFieldQualifyOuterVlanPri:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_OuterVlanPri,
                     "OuterVlanPri");
        break;
    case bcmFieldQualifyOuterVlanCfi:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_OuterVlanCfi,
                     "OuterVlanCfi");
        break;
    case bcmFieldQualifyInnerVlan:
        rv = robo_fp_qual_vlan(unit, eid, args, bcm_field_qualify_InnerVlan,
                     "InnerVlan");
        break;
    case bcmFieldQualifyInnerVlanId:
        rv = robo_fp_qual_vlan(unit, eid, args, bcm_field_qualify_InnerVlanId,
                     "InnerVlanId");
        break;
    case bcmFieldQualifyInnerVlanPri:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_InnerVlanPri,
                     "InnerVlanPri");
        break;
    case bcmFieldQualifyInnerVlanCfi:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_InnerVlanCfi,
                     "InnerVlanCfi");
        break;
    case bcmFieldQualifyEtherType:
        rv = robo_fp_qual_16(unit, eid, args, bcm_field_qualify_EtherType,
                   "EtherType");
        break;
    case bcmFieldQualifyIpProtocol:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_IpProtocol,
                  "IpProtocol");
        break;
    case bcmFieldQualifyIpInfo:
        rv = robo_fp_qual_32(unit, eid, args, bcm_field_qualify_IpInfo,
                   "IpInfo");
        break;
    case bcmFieldQualifyPacketRes:
        rv = robo_fp_qual_32(unit, eid, args, bcm_field_qualify_PacketRes,
                   "PacketRes");
        break;
    case bcmFieldQualifySrcIp:
        if(qual_get) {
            rv = robo_fp_qual_ip_get(unit, eid, bcm_field_qualify_SrcIp_get,
                   "SrcIp");
        } else {
            rv = robo_fp_qual_ip(unit, eid, args, bcm_field_qualify_SrcIp,
                   "SrcIp");
        }
        break;
    case bcmFieldQualifyDstIp:
        if(qual_get) {
            rv = robo_fp_qual_ip_get(unit, eid, bcm_field_qualify_DstIp_get,
                   "DstIp");
        } else {
            rv = robo_fp_qual_ip(unit, eid, args, bcm_field_qualify_DstIp,
                   "DstIp");
        }
        break;
    case bcmFieldQualifyDSCP:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_DSCP,
                  "DSCP");
        break;
    case bcmFieldQualifyIpFlags:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_IpFlags,
                  "IpFlags");
        break;
    case bcmFieldQualifyTcpControl:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_TcpControl,
                  "TcpControl");
        break;
    case bcmFieldQualifyTtl:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_Ttl,
                  "TTL");
        break;
    case bcmFieldQualifyRangeCheck:
        rv = robo_fp_qual_rangecheck(unit, eid, args, bcm_field_qualify_RangeCheck,
                       "RangeCheck");
        break;
    case bcmFieldQualifySrcIp6:
        if(qual_get) {
            rv = robo_fp_qual_ip6_get(unit, eid, bcm_field_qualify_SrcIp6_get,"SrcIp6");
        } else {
            rv = robo_fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6,
                    "SrcIp6");
        }
        break;
    case bcmFieldQualifySrcIp6High:
        if(qual_get) {
            rv = robo_fp_qual_ip6_get(unit, eid, bcm_field_qualify_SrcIp6High_get,"SrcIp6");
        } else {
            rv = robo_fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6High,
                    "SrcIp6High");
        }
        break;
    case bcmFieldQualifySrcIp6Low:
        if(qual_get) {
            rv = robo_fp_qual_ip6_get(unit, eid, bcm_field_qualify_SrcIp6Low_get,"SrcIp6");
        } else {
            rv = robo_fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6Low,
                    "SrcIp6Low");
        }
        break;
    case bcmFieldQualifyDstIp6:
        if(qual_get) {
            rv = robo_fp_qual_ip6_get(unit, eid, bcm_field_qualify_DstIp6_get,"DstIp6");
        } else {
            rv = robo_fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6, "DstIp6");
        }
        break;
    case bcmFieldQualifyDstIp6High:
        if(qual_get) {
            rv = robo_fp_qual_ip6_get(unit, eid, bcm_field_qualify_DstIp6High_get,"DstIp6High");
        } else {
            rv = robo_fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6High,
                    "DstIp6High");
        }
        break;
    case bcmFieldQualifyDstIp6Low:
        if(qual_get) {
            rv = robo_fp_qual_ip6_get(unit, eid, bcm_field_qualify_DstIp6Low_get,"DstIp6Low");
        } else {
            rv = robo_fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6Low,
                    "DstIp6Low");
        }
        break;
    case bcmFieldQualifyIp6FlowLabel:
        rv = robo_fp_qual_32(unit, eid, args, bcm_field_qualify_Ip6FlowLabel,
                   "Ip6FlowLabel");
        break;
    case bcmFieldQualifySrcMac:
        rv = robo_fp_qual_mac(unit, eid, args, bcm_field_qualify_SrcMac, "SrcMac");
        break;
    case bcmFieldQualifyDstMac:
        rv = robo_fp_qual_mac(unit, eid, args, bcm_field_qualify_DstMac, "DstMac");
        break;
    case bcmFieldQualifyIpType:
        rv = robo_fp_qual_IpType(unit, eid, args);
        break;
    case bcmFieldQualifyL2Format:
        rv = robo_fp_qual_L2Format(unit, eid, args);
        break;
    case bcmFieldQualifyMHOpcode:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_MHOpcode,
                  "MHOpcode");
        break;
    case bcmFieldQualifyDecap:
        rv = robo_fp_qual_Decap(unit, eid, args);
        break;
    case bcmFieldQualifyHiGig:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_HiGig, "HiGig");
        break;
    case bcmFieldQualifySrcIpEqualDstIp:
        rv = robo_fp_qual_same(unit, eid, args, bcm_field_qualify_SrcIpEqualDstIp, 
                       "SrcIpEqualDstIp");
        break;
    case bcmFieldQualifyEqualL4Port:
        rv = robo_fp_qual_same(unit, eid, args, bcm_field_qualify_EqualL4Port, 
                       "EqualL4Port");
        break;
    case bcmFieldQualifyTcpSequenceZero:
        rv = robo_fp_qual_same(unit, eid, args, bcm_field_qualify_TcpSequenceZero, 
                       "TcpSequenceZero");
        break;
    case bcmFieldQualifyTcpHeaderSize:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_TcpHeaderSize, 
                       "TcpHeaderSize");
        break;
    case bcmFieldQualifyVlanFormat:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_VlanFormat,
                  "VlanFormat");
        break;
    case bcmFieldQualifyIpFrag:
        rv = robo_fp_qual_IpFrag(unit, eid, args);
        break;
    case bcmFieldQualifySnap:
        rv = robo_fp_qual_snap(unit, eid, args, bcm_field_qualify_Snap,
                   "Snap");
       break;
    case bcmFieldQualifyIpAuth:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_IpAuth,
                   "IpAuth");
       break;
    case bcmFieldQualifyIpProtocolCommon:
        rv = robo_fp_qual_IpProtocolCommon(unit, eid, args);
        break;
    case bcmFieldQualifyBigIcmpCheck:
        rv = robo_fp_qual_32(unit, eid, args, bcm_field_qualify_BigIcmpCheck,
                   "BigIcmpCheck");
        break;
    case bcmFieldQualifyIcmpTypeCode:
        rv = robo_fp_qual_16(unit, eid, args, bcm_field_qualify_IcmpTypeCode,
                   "IcmpTypeCode");
        break;
    case bcmFieldQualifyIgmpTypeMaxRespTime:
        rv = robo_fp_qual_16(unit, eid, args, bcm_field_qualify_IgmpTypeMaxRespTime,
                   "IgmpTypeMaxRespTime");
        break;
    case bcmFieldQualifyInterfaceClassPort:
        rv = robo_fp_qual_InterfaceClass(unit, eid, args, 
                                    bcm_field_qualify_InterfaceClassPort,
                    "InterfaceClassPort");
        break;
    case bcmFieldQualifyFlowId:
        rv = robo_fp_qual_16(unit, eid, args, bcm_field_qualify_FlowId,
                   "FlowId");
        break;
    case bcmFieldQualifyInVPort:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_InVPort,
                  "InVPort");
        break;
    case bcmFieldQualifyOutVPort:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_OutVPort,
                  "OutVPort");
        break;
    case bcmFieldQualifyL4Ports:
        rv = robo_fp_qual_8(unit, eid, args, bcm_field_qualify_L4Ports,
                  "L4Ports");
        break;        
    case bcmFieldQualifyCount:
    default:
        robo_fp_list_quals(unit);
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unknown qualifier: %s\n"),
                   unit, qual_str));
        rv = CMD_FAIL;
    }

    if (CMD_OK != rv) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Qualifier installation error: %s\n"),
                   unit, qual_str));
    }

    return rv;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_same(int unit, bcm_field_entry_t eid, args_t *args,
               int func(int, bcm_field_entry_t, uint32),
               char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint32                      data;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);
    FP_GET_NUMB(data, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'flag' */
    retval = func(unit, eid, data);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen("bcm_field_qualify_"));
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
robo_fp_qual_InPorts(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_pbmp_t                  data, mask;
    bcm_port_config_t           pcfg;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    } else if (parse_bcm_pbmp(unit, subcmd, &data) < 0) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: %s: Error: unrecognized port bitmap: %s\n"),
                unit, ARG_CMD(args), subcmd));
        return CMD_FAIL;
    }

    if (bcm_port_config_get(unit, &pcfg) != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: %s: Error: bcm ports not initialized\n"),
                   unit,
                ARG_CMD(args)));
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(args)) == NULL) {
        BCM_PBMP_ASSIGN(mask, pcfg.all);
    } else if (parse_bcm_pbmp(unit, subcmd, &mask) < 0) {
        return CMD_FAIL;
    }

    /* BCM.0> fp qual 'eid' InPorts 'data' 'mask' */
    retval = bcm_field_qualify_InPorts(unit, eid, data, mask);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_InPorts");

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_OutPorts(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_pbmp_t                  data, mask;
    bcm_port_config_t           pcfg;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    } else if (parse_bcm_pbmp(unit, subcmd, &data) < 0) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: %s: Error: unrecognized port bitmap: %s\n"),
                       unit, ARG_CMD(args), subcmd));
        return CMD_FAIL;
    }

    if (bcm_port_config_get(unit, &pcfg) != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: %s: Error: bcm ports not initialized\n"),
                   unit, ARG_CMD(args)));
        return CMD_FAIL;
    }

    if ((subcmd = ARG_GET(args)) == NULL) {
        BCM_PBMP_ASSIGN(mask, pcfg.port);
    } else if (parse_bcm_pbmp(unit, subcmd, &mask) < 0) {
        return CMD_FAIL;
    }

    /* BCM.0> fp qual 'eid' OutPorts 'data' 'mask' */
    retval = bcm_field_qualify_OutPorts(unit, eid, data, mask);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_OutPorts");

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_port(int unit, bcm_field_entry_t eid, args_t *args,
             int func(int, bcm_field_entry_t, bcm_port_t, bcm_port_t),
             char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_port_t                  data, mask;
    char                        str[FP_STAT_STR_SZ];
   
    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);

    FP_GET_PORT(unit, data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_modport(int unit, bcm_field_entry_t eid, args_t *args,
             int func(int, bcm_field_entry_t, bcm_module_t, bcm_module_t,
                      bcm_port_t, bcm_port_t),
             char *qual_str)
{
    int                         retval = CMD_OK;
    bcm_module_t                data_modid, mask_modid = (1 << 6) - 1;
    int                         data_port, mask_port = (1 << 6) - 1;
    char                        str[FP_STAT_STR_SZ];
    parse_table_t               pt;
   
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Modid", PQ_DFL | PQ_INT, 0,
                    (void *)&data_modid, 0);
    parse_table_add(&pt, "ModidMask", PQ_DFL | PQ_INT, 0,
                    (void *)&mask_modid, 0);
    parse_table_add(&pt, "Port", PQ_DFL | PQ_PORT, 0,
                    (void *)&data_port, 0);
    parse_table_add(&pt, "PortMask", PQ_DFL | PQ_INT, 0,
                    (void *)&mask_port, 0);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data_modid, mask_modid, data_port, mask_port);
    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

STATIC int
robo_fp_qual_modport_help(const char *prefix, const char *qual_str, int width_col1)
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
robo_fp_qual_trunk(int unit, bcm_field_entry_t eid, args_t *args,
             int func(int, bcm_field_entry_t, bcm_trunk_t, bcm_trunk_t),
             char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_port_t                  data, mask;
    char                        str[FP_STAT_STR_SZ];
   
    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_l4port(int unit, bcm_field_entry_t eid, args_t *args,
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

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_rangecheck(int unit, bcm_field_entry_t eid, args_t *args,
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
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_vlan(int unit, bcm_field_entry_t eid, args_t *args,
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
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_ip(int unit, bcm_field_entry_t eid, args_t *args,
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

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(stat_str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    stat_str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(stat_str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}
STATIC int
robo_fp_qual_ip_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip_t*, bcm_ip_t*),
            char *qual_str)
{
    int                         retval = CMD_OK;
    bcm_ip_t                   data, mask;
    char                        str[FP_STAT_STR_SZ];

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, &data, &mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_ip6(int unit, bcm_field_entry_t eid, args_t *args,
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

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

STATIC int
robo_fp_qual_ip6_get(int unit, bcm_field_entry_t eid,
            int func(int, bcm_field_entry_t, bcm_ip6_t*, bcm_ip6_t*),
            char *qual_str)
{
    int                         retval = CMD_OK;
    bcm_ip6_t                   data, mask;
    char                        str[FP_STAT_STR_SZ];

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, &data, &mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_mac(int unit, bcm_field_entry_t eid, args_t *args,
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
 
    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(stat_str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    stat_str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(stat_str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
    }

STATIC int
robo_fp_qual_mac_help(const char *prefix, const char *qual_str, int width_col1)
{
    if (width_col1 < sal_strlen(qual_str)) {
        width_col1 = sal_strlen(qual_str) + 1;
    }

    /* "FieldProcessor qual <eid> XxxMac" */
    LOG_CLI(("%s%-*s%s\n", prefix, width_col1, qual_str,
           "Data=<mac> Mask=<mac>"));
    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_snap(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, bcm_field_snap_header_t, 
           bcm_field_snap_header_t), char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_field_snap_header_t snap_data, snap_mask;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(&snap_data, 0, sizeof(bcm_field_snap_header_t));
    sal_memset(&snap_mask, 0, sizeof(bcm_field_snap_header_t));
    FP_GET_NUMB(snap_data.org_code, subcmd, args);
    FP_GET_NUMB(snap_mask.org_code, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, snap_data, snap_mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen("bcm_field_qualify_"));
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
robo_fp_parse_hex_string(char *str, int buf_sz, uint8 *buffer, int *length)
{
    int      str_len;
    char     *ptr;
    uint8    *data_ptr;

    /* Input parameters check. */
    if ((NULL == str) || (NULL == buffer) || (NULL == length)) {
        cli_out("Invalid parameters fp_parse_hex_string\n"); 
        return -1;
    }

    str_len = strlen(str); 

    if (str[0] == '0' && tolower((int)str[1]) == 'x') {
        ptr = str + 2;
    } else {
        ptr = str;
    }

    data_ptr = (uint8 *)buffer;
    while ((ptr < (str + str_len)) && (buf_sz > 0)) {
        if (!isxdigit((unsigned) *ptr)) {  /* bad character */
            cli_out("Invalid data character. (%c) \n", *ptr);
            return -1;
        }
        *data_ptr = 16 * xdigit2i((unsigned) *(ptr++)); 
        if (!isxdigit((unsigned) *ptr)) {  /* bad character */
            cli_out("Invalid data character. (%c) \n", *ptr);
            return CMD_USAGE;
        }
        *data_ptr |=  xdigit2i((unsigned) *(ptr++)); 
        data_ptr++;
        buf_sz--;
    }
    *length = data_ptr - buffer;
    return CMD_OK;
}
    


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_data(int unit, bcm_field_entry_t eid, args_t *args)
{
    _bcm_field_qual_data_t data;
    _bcm_field_qual_data_t mask;
    char                   *str= NULL;
    int                    retval = CMD_OK;
    int                    qualid;
    int                    data_length;
    int                    mask_length;
    int                    rv;

    sal_memset(data, 0, sizeof(_bcm_field_qual_data_t));
    sal_memset(mask, 0, sizeof(_bcm_field_qual_data_t));

    /* Parse command option arguments */
    FP_GET_NUMB(qualid,     str, args);

    /* Get match data. */
    if ((str= ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    rv = robo_fp_parse_hex_string(str, sizeof(_bcm_field_qual_data_t), 
                             (uint8 *)data, &data_length);
    if (rv < 0) {
        cli_out("Qualifier data parse error.\n");
        return CMD_USAGE;
    }

    /* Get match mask. */
    if ((str= ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    rv = robo_fp_parse_hex_string(str, sizeof(_bcm_field_qual_data_t), 
                             (uint8 *)mask, &mask_length);
    if (rv < 0) {
        cli_out("Qualifier mask parse error.\n");
        return CMD_USAGE;
    }

    /* BCM.0> fp qual 'eid' data qualid length 'data' 'mask' */
    retval = bcm_field_qualify_data(unit, eid, qualid,
            (uint8 *)data, (uint8 *)mask, mask_length);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_data");
    return CMD_OK;
}


/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_8(int unit, bcm_field_entry_t eid, args_t *args,
          int func(int, bcm_field_entry_t, uint8, uint8),
          char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint8                       data, mask;
    char                        stat_str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(stat_str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    stat_str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(stat_str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_16(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint16, uint16),
           char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint16                      data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_32(int unit, bcm_field_entry_t eid, args_t *args,
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
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
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
robo_fp_qual_Decap(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_field_decap_t           decap;

    if ((subcmd  = ARG_GET(args)) == NULL) { 
        return CMD_USAGE; 
    } 

    if (isint(subcmd)) {
        decap = parse_integer(subcmd);
    } else {
        decap = parse_field_decap(subcmd);
        if (decap == bcmFieldDecapCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown decap value: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    /* BCM.0> fp qual 'eid' Decap 'decap' */
    retval = bcm_field_qualify_Decap(unit, eid, decap);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_Decap");

    return CMD_OK;
}


STATIC int 
robo_fp_qual_IpInfo_help(const char *prefix, int width_col1) {
    if (width_col1 < strlen("IpInfo")) {
        width_col1 = strlen("IpInfo") + 1;
    }

    LOG_CLI(("%s%-*s%s\n", prefix, width_col1, "IpInfo",
           "HeaderOffsetZero=<0/1> HeaderFlagsMF=<0/1> ChecksumOK=<0/1>"));
    return CMD_OK;
}

STATIC int 
robo_fp_qual_PacketRes_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("PacketRes")) {
        width_col1 = strlen("PacketRes") + 1;
    }

    offset = cli_out("%s%-*sRes=", prefix, width_col1, "PacketRes");
    robo_fp_print_options(robo_packet_res_text, offset);
    cli_out("\n");

    return CMD_OK;
}

STATIC int 
robo_fp_qual_Color_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("Color")) {
        width_col1 = strlen("Color") + 1;
    }

    offset = cli_out("%s%-*scolor=", prefix, width_col1, "color");
    robo_fp_print_options(robo_color_text, offset);
    cli_out("\n");

    return CMD_OK;
}

/*
 * Function:
 * Purpose:
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_IpFrag(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    int                 data = -1;
    parse_table_t       pt;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "frag", PQ_DFL | PQ_MULTI, 0,
                    (void *)&data, robo_ipfrag_text);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }
    
    /* BCM.0> fp qual <eid> IpFrag [Frag=<>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_qualify_IpFrag(entry=%d, "
                            "data=%#x)\n"),
                 unit, eid, data));
    retval = bcm_field_qualify_IpFrag(unit, eid, data);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_IpFrag");

    return CMD_OK;
}


STATIC int 
robo_fp_qual_IpFrag_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("IpFrag")) {
        width_col1 = strlen("IpFrag") + 1;
    }

    offset = cli_out("%s%-*sfrag=", prefix, width_col1, "IpFrag");
    robo_fp_print_options(robo_ipfrag_text, offset);
    cli_out("\n");

    return CMD_OK;
}

STATIC int 
robo_fp_qual_LoopbackType_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("LoopbackType")) {
        width_col1 = strlen("LoopbackType") + 1;
    }

    offset = cli_out("%s%-*slb_type=", prefix, width_col1, "LoopbackType");
    robo_fp_print_options(robo_loopbacktype_text, offset);
    cli_out("\n");

    return CMD_OK;
}


STATIC int 
robo_fp_qual_TunnelType_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("TunnelType")) {
        width_col1 = strlen("TunnelType") + 1;
    }

    offset = cli_out("%s%-*slb_type=", prefix, width_col1, "TunnelType");
    robo_fp_print_options(robo_tunneltype_text, offset);
    cli_out("\n");

    return CMD_OK;
}

/*
 * Function: 
 *     robo_fp_qual_InterfaceClass
 * Purpose:
 *     Qualify on Interface class
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_InterfaceClass(int unit, bcm_field_entry_t eid, args_t *args,
            int func(int, bcm_field_entry_t, uint32, uint32),
            char *qual_str)
{
    int   retval = CMD_OK;
    char  *subcmd = NULL;
    uint32   data, mask;
    char  str[FP_STAT_STR_SZ];

    FP_GET_NUMB(data, subcmd, args);
    FP_GET_NUMB(mask, subcmd, args);
    
    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    sal_strncpy(str, "bcm_field_qualify_", sal_strlen("bcm_field_qualify_"));
    str[sal_strlen("bcm_field_qualify_")] = '\0';
    strncat(str, qual_str, 
        FP_STAT_STR_SZ - 1 - sal_strlen("bcm_field_qualify_"));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_qual_L2Format
 * Purpose:
 *     Qualify on L2Format.
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_L2Format(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_field_L2Format_t          type;

    if ((subcmd  = ARG_GET(args)) == NULL) { 
        return CMD_USAGE; 
    } 

    if (isint(subcmd)) {
        type = parse_integer(subcmd);
    } else {
        type = robo_fp_lookup_L2Format(subcmd);
        if (type == bcmFieldL2FormatCount) {
            LOG_CLI(("Unknown L2Format value: %s\n", subcmd));
            return CMD_FAIL;
        }
    }

    /* BCM.0> fp qual 'eid' L2Format 'type' */
    retval = bcm_field_qualify_L2Format(unit, eid, type);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_L2Format");

    return CMD_OK;
}


/*
 * Function:
 *     robo_fp_qual_IpType
 * Purpose:
 *     Qualify on IpType.
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_IpType(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_field_IpType_t          type;

    if ((subcmd  = ARG_GET(args)) == NULL) { 
        return CMD_USAGE; 
    } 

    if (isint(subcmd)) {
        type = parse_integer(subcmd);
    } else {
        type = robo_fp_lookup_IpType(subcmd);
        if (type == bcmFieldIpTypeCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown IpType value: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    /* BCM.0> fp qual 'eid' IpType 'type' */
    retval = bcm_field_qualify_IpType(unit, eid, type);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_IpType");

    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_qual_IpProtocolCommon
 * Purpose:
 *     Qualify on IpProtocolCommon.
 * Parmameters:
 * Returns:
 */
STATIC int
robo_fp_qual_IpProtocolCommon(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_field_IpProtocolCommon_t protocol;

    if ((subcmd  = ARG_GET(args)) == NULL) { 
        return CMD_USAGE; 
    } 

    if (isint(subcmd)) {
        protocol = parse_integer(subcmd);
    } else {
        protocol = robo_fp_lookup_IpProtocolCommon(subcmd);
        if (protocol == bcmFieldIpProtocolCommonCount) {
            LOG_CLI(("Unknown IpProtocolCommon value: %s\n", subcmd));
            return CMD_FAIL;
        }
    }

    /* BCM.0> fp qual 'eid' IpProtocolcommon 'type' */
    retval = bcm_field_qualify_IpProtocolCommon(unit, eid, protocol);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_IpProtocolCommon");

    return CMD_OK;
}


STATIC int robo_fp_thread(int unit, args_t *args) 
{
    char*   subcmd = NULL;
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    if(!sal_strcasecmp(subcmd, "off")) {
        _robo_field_thread_stop(unit);
        return CMD_OK;
    }

    return CMD_USAGE;
}

/*
 * Function:
 *     fp_policer_create
 * Purpose:
 *     Add fp meter entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_policer_create(int unit, args_t *args)
{
    parse_table_t           pt;
    bcm_policer_config_t    pol_cfg;
    bcm_policer_t           polid = -1;
    cmd_result_t            retCode;
    bcm_policer_mode_t      mode;
    int                     color_blind;
    int                     rv;

    mode = bcmPolicerModeCount;
    color_blind = 0;
    bcm_policer_config_t_init(&pol_cfg);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
    parse_table_add(&pt, "ColorBlind", PQ_DFL|PQ_INT, 0, &color_blind, NULL);
    parse_table_add(&pt, "Mode", PQ_DFL | PQ_MULTI, 0,
                    (void *)&mode, robo_policermode_text);
    parse_table_add(&pt, "cbs", PQ_DFL|PQ_INT, 0, 
                    &pol_cfg.ckbits_burst, NULL);
    parse_table_add(&pt, "cir", PQ_DFL|PQ_INT, 0, 
                    &pol_cfg.ckbits_sec, NULL);
    parse_table_add(&pt, "ebs", PQ_DFL|PQ_INT, 0, 
                    &pol_cfg.pkbits_burst, NULL);
    parse_table_add(&pt, "eir", PQ_DFL|PQ_INT, 0, &pol_cfg.pkbits_sec, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (mode == bcmPolicerModeCount) {
        cli_out("Invalid policer mode\n");
        robo_fp_print_options(robo_policermode_text, sal_strlen("\tMode="));
        cli_out("\n");
    } else {
        pol_cfg.mode = mode;
    }

    if (polid >= 0) {
        pol_cfg.flags |= (BCM_POLICER_WITH_ID | BCM_POLICER_REPLACE);
    }

    if (color_blind) {
        pol_cfg.flags |= (BCM_POLICER_COLOR_BLIND);
    }


    if ((rv = bcm_policer_create(unit, &pol_cfg, &polid)) != BCM_E_NONE) {
        cli_out("Policer add failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (!(pol_cfg.flags & BCM_POLICER_WITH_ID)) {
        cli_out("Policer created with id: %d \n", polid);
    }

    return CMD_OK;
}

/*
 * Function:
 *     fp_policer_destroy
 * Purpose:
 *     Remove fp meter entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_policer_destroy(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    bcm_policer_t           polid = -1;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    if (0 == sal_strncasecmp(param, "all", 3)) {
        param = ARG_GET(args);
        if ((rv = bcm_policer_destroy_all(unit)) != BCM_E_NONE) {
            cli_out("ERROR: bcm_policer_destroy_all(unit=%d) failed.(%s) \n",
                    unit, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        if (polid < 0) {
            cli_out("Invalid policer id specified\n");
            return CMD_FAIL;
        }
        if ((rv = bcm_policer_destroy(unit, polid)) != BCM_E_NONE) {
            cli_out("ERROR: bcm_policer_destroy(unit=%d, id=%d) failed (%s) \n",
                    unit, polid, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

/*
 * Function:
 *     fp_policer_attach
 * Purpose:
 *     Attach fp meter to a field entry. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_policer_attach(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    bcm_policer_t           polid = -1;
    bcm_field_entry_t       eid = -1; 
    int                     level = 0;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
    parse_table_add(&pt, "entry", PQ_DFL|PQ_INT, 0, &eid, NULL);
    parse_table_add(&pt, "level", PQ_DFL|PQ_INT, 0, &level, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }
    if (polid < 0) {
        cli_out("Invalid policer id specified\n");
        return CMD_FAIL;
    }
    if (eid < 0) {
        cli_out("Invalid entry id specified\n");
        return CMD_FAIL;
    }
    if ((level >= _FP_POLICER_LEVEL_COUNT) || (level < 0)) {
        cli_out("Invalid level specified\n");
        return CMD_FAIL;
    }

    if ((rv = bcm_field_entry_policer_attach(unit, eid, level, polid)) != BCM_E_NONE) {
        cli_out(\
                "ERROR: bcm_policer_attach(unit=%d, eid=%d, level=%d,\
                polid=%d) failed (%s) \n", unit, eid, level, polid, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *     fp_policer_detach
 * Purpose:
 *     Detach fp meter from a field entry. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_policer_detach(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    bcm_field_entry_t       eid = -1; 
    int                     level = 0;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "entry", PQ_DFL|PQ_INT, 0, &eid, NULL);
    parse_table_add(&pt, "level", PQ_DFL|PQ_INT, 0, &level, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }
    if (eid < 0) {
        cli_out("Invalid entry id specified\n");
        return CMD_FAIL;
    }
    if ((level >= _FP_POLICER_LEVEL_COUNT) || (level < 0)) {
        cli_out("Invalid level specified\n");
        return CMD_FAIL;
    }

    if ((rv = bcm_field_entry_policer_detach(unit, eid, level)) != BCM_E_NONE) {
        cli_out(\
                "ERROR: bcm_policer_attach(unit=%d, eid=%d, level=%d)\
                failed (%s) \n", unit, eid, level, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}
/*
 * Function:
 *     fp_policer_set
 * Purpose:
 *     Set fp meter entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_policer_set(int unit, args_t *args)
{
    parse_table_t           pt;
    bcm_policer_config_t    pol_cfg, org_pol_cfg;
    bcm_policer_t           polid = -1;
    cmd_result_t            retCode;
    bcm_policer_mode_t      mode;
    int                     color_blind;
    int                     rv;

    mode = bcmPolicerModeCount;
    color_blind = 0;
    bcm_policer_config_t_init(&pol_cfg);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
    parse_table_add(&pt, "ColorBlind", PQ_DFL|PQ_INT, 0, &color_blind, NULL);
    parse_table_add(&pt, "Mode", PQ_DFL | PQ_MULTI, 0,
                    (void *)&mode, robo_policermode_text);
    parse_table_add(&pt, "cbs", PQ_DFL|PQ_INT, 0, 
                    &pol_cfg.ckbits_burst, NULL);
    parse_table_add(&pt, "cir", PQ_DFL|PQ_INT, 0, 
                    &pol_cfg.ckbits_sec, NULL);
    parse_table_add(&pt, "ebs", PQ_DFL|PQ_INT, 0, 
                    &pol_cfg.pkbits_burst, NULL);
    parse_table_add(&pt, "eir", PQ_DFL|PQ_INT, 0, &pol_cfg.pkbits_sec, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (mode == bcmPolicerModeCount) {
        cli_out("Invalid policer mode\n");
        robo_fp_print_options(robo_policermode_text, sal_strlen("\tMode="));
        cli_out("\n");
    } else {
        pol_cfg.mode = mode;
    }

    if ((rv = bcm_policer_get(unit, polid, &org_pol_cfg)) != BCM_E_NONE) {
        cli_out("Get Policer id %d  failed. (%s) \n", polid, bcm_errmsg(rv));
        return CMD_FAIL;
    }

    if (color_blind) {
        pol_cfg.flags |= (BCM_POLICER_COLOR_BLIND);
    }

    if ((rv = bcm_policer_set(unit, polid, &pol_cfg)) != BCM_E_NONE) {
        cli_out("Policer set failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    return CMD_OK;
}


STATIC int
robo_fp_policer(int unit, args_t *args)
{
    char* subcmd = NULL;
 
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp policer create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return robo_fp_policer_create(unit, args);
    }

    /* BCM.0> fp policer destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return robo_fp_policer_destroy(unit, args);
    }

    /* BCM.0> fp policer attach ... */
    if(!sal_strcasecmp(subcmd, "attach")) {
        return robo_fp_policer_attach(unit, args);
    }

    /* BCM.0> fp policer detach ... */
    if(!sal_strcasecmp(subcmd, "detach")) {
        return robo_fp_policer_detach(unit, args);
    }

    /* BCM.0> fp policer attach ... */
    if(!sal_strcasecmp(subcmd, "set")) {
        return robo_fp_policer_set(unit, args);
    }
    return CMD_USAGE;
}


#define _BCM_CLI_STAT_ARR_SIZE     (5)
/*
 * Function:
 *     robo_fp_stat_create
 * Purpose:
 *     Add fp counter entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat_create(int unit, args_t *args)
{
    parse_table_t           pt;
    int                     statid = -1;
    int                     group = -1;
    cmd_result_t            retCode;
    bcm_field_stat_t        stat_arr[_BCM_CLI_STAT_ARR_SIZE];
    int                     stat_arr_sz; 
    char                    buffer[_BCM_CLI_STAT_ARR_SIZE][10];
    int                     idx;
    int                     rv;

    /* Initialization. */
    for (idx = 0; idx < _BCM_CLI_STAT_ARR_SIZE; idx++) {
        stat_arr[idx] = bcmFieldStatCount;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "group", PQ_DFL|PQ_INT, 0, &group, NULL);

    for (idx = 0; idx < _BCM_CLI_STAT_ARR_SIZE; idx++) {
        sal_sprintf(buffer[idx], "type%d", idx);
        parse_table_add(&pt, buffer[idx], PQ_DFL | PQ_MULTI, 0,
                        (void *)&stat_arr[idx], robo_stattype_text);
    }
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    stat_arr_sz = 0;
    for (idx = 0; idx < _BCM_CLI_STAT_ARR_SIZE; idx++) {
        if ((stat_arr[idx] >= 0) && (stat_arr[idx] < bcmFieldStatCount)) {
            stat_arr_sz++; 
        }
    }
    if (0 == stat_arr_sz) {
        cli_out("Stat type parse failed\n");
        robo_fp_print_options(robo_stattype_text, sal_strlen("\tstatXX="));
        cli_out("\n");
        return CMD_FAIL;
    }

    if (group < 0) {
        cli_out("Invalid group id (%d) \n", group);
    }

    rv = bcm_field_stat_create(unit, group, stat_arr_sz, stat_arr, &statid);
    if (BCM_FAILURE(rv)) {
        cli_out("Stat add failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    LastCreatedStatID = statid;

    cli_out("Stat created with id: %d \n", statid);
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_stat_destroy
 * Purpose:
 *     Remove fp stat entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat_destroy(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    int                     statid = -1;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    if (0 == sal_strncasecmp(param, "all", 3)) {
        /* Delete all stat entities. 
           param = ARG_GET(args);
           if ((rv = bcm_field_stat_destroy_all(unit)) != BCM_E_NONE) {
           cli_out("ERROR: bcm_field_stat_destroy_all(unit=%d) failed.(%s) \n",
                   unit, bcm_errmsg(rv));
           }
         */
        cli_out("ERROR: bcm_field_stat_destroy_all: Unimplemented\n");
        return CMD_FAIL;
    } else {
        /* Parse command option arguments */
        parse_table_init(unit, &pt);
        parse_table_add(&pt, "StatId", PQ_DFL|PQ_INT, 0, &statid, NULL);
        if (!parseEndOk(args, &pt, &retCode)) {
            return retCode;
        }
        if (statid < 0) {
            cli_out("Invalid stat id specified\n");
            return CMD_FAIL;
        }
        if ((rv = bcm_field_stat_destroy(unit, statid)) != BCM_E_NONE) {
            cli_out("ERROR: bcm_field_stat_destroy(unit=%d, id=%d) failed (%s) \n",
                    unit, statid, bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }

    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_stat_attach
 * Purpose:
 *     Attach fp counter to a field entry. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat_attach(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    int                     statid = -1;
    bcm_field_entry_t       eid = -1; 

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);

    parse_table_add(&pt, "StatId", PQ_INT, (void *) LastCreatedStatID,
        &statid, NULL);

    parse_table_add(&pt, "entry", PQ_DFL|PQ_INT, 0, &eid, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }
    if (statid < 0) {
        cli_out("Stat id parse failed\n");
        return CMD_FAIL;
    }
    if (eid < 0) {
        cli_out("Entry id partse failed\n");
        return CMD_FAIL;
    }

    if ((rv = bcm_field_entry_stat_attach(unit, eid, statid)) != BCM_E_NONE) {
        cli_out(\
                "ERROR: bcm_field_entry_stat_attach(unit=%d, eid=%d, \
                statid=%d) failed (%s) \n", unit, eid, statid, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_stat_detach
 * Purpose:
 *     Detach fp counter from a field entry. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat_detach(int unit, args_t *args)
{
    char                    *param;
    int                     rv;
    parse_table_t           pt;
    cmd_result_t            retCode;
    bcm_field_entry_t       eid = -1; 
    int                     statid = -1;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "entry", PQ_DFL|PQ_INT, 0, &eid, NULL);
    parse_table_add(&pt, "StatId", PQ_DFL|PQ_INT, 0, &statid, NULL);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }
    if (eid < 0) {
        cli_out("Invalid entry id specified\n");
        return CMD_FAIL;
    }

    if (statid < 0) {
        cli_out("Stat id parse failed\n");
        return CMD_FAIL;
    }

    if ((rv = bcm_field_entry_stat_detach(unit, eid, statid)) != BCM_E_NONE) {
        cli_out(\
                "ERROR: bcm_stat_attach(unit=%d, eid=%d, statid=%d)\
                failed (%s) \n", unit, eid, statid, bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_stat_set
 * Purpose:
 *     Set stat value.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat_set(int unit, args_t *args)
{
    int                 statid = -1;
    bcm_field_stat_t    type = bcmFieldStatCount;
    int                 retval;
    uint64              val64;
    parse_table_t       pt;

    COMPILER_64_ZERO(val64);
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "StatId", PQ_DFL|PQ_INT, 0, &statid, NULL);
    parse_table_add(&pt, "type", PQ_DFL | PQ_MULTI, 0, (void *)&type, robo_stattype_text);
    parse_table_add(&pt, "val", PQ_DFL|PQ_INT64, 0, &val64, NULL);
    if (!parseEndOk(args, &pt, &retval)) {
        return retval;
    }

    if (statid < 0) {
        cli_out("Stat id parse failed\n");
        return CMD_FAIL;
    }

    if (type == bcmFieldStatCount) {
        cli_out("Stat type parse failed\n");
        robo_fp_print_options(robo_stattype_text, sal_strlen("\ttype="));
        cli_out("\n");
    }

    retval = bcm_field_stat_set(unit, statid, type, val64);
    FP_CHECK_RETURN(unit, retval, "bcm_field_stat_set");
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_stat_get
 * Purpose:
 *     Get stat value.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat_get(int unit, args_t *args)
{
    int                 statid = -1;
    bcm_field_stat_t    stat_arr[_BCM_CLI_STAT_ARR_SIZE];
    int                 retval;
    uint64              val64;
    int                 idx;
    parse_table_t       pt;

    COMPILER_64_ZERO(val64);
    for (idx = 0; idx < _BCM_CLI_STAT_ARR_SIZE; idx++) {
        stat_arr[idx] = bcmFieldStatCount;
    }

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "StatId", PQ_DFL|PQ_INT, 0, &statid, NULL);
    parse_table_add(&pt, "type", PQ_DFL | PQ_MULTI, 0, (void *)&stat_arr[0], robo_stattype_text);
    if (!parseEndOk(args, &pt, &retval)) {
        return retval;
    }

    if (statid < 0) {
        cli_out("Stat id parse failed\n");
        return CMD_FAIL;
    }

    if (stat_arr[0] == bcmFieldStatCount) {
        retval = bcm_field_stat_config_get(unit, statid, 
                                           _BCM_CLI_STAT_ARR_SIZE, 
                                           stat_arr);
        FP_CHECK_RETURN(unit, retval, "bcm_field_stat_config_get");

        for (idx = 0; idx < _BCM_CLI_STAT_ARR_SIZE; idx++) {
            if (bcmFieldStatCount == stat_arr[idx]) {
                break;
            }
            retval = bcm_field_stat_get(unit, statid, stat_arr[idx], &val64);
            FP_CHECK_RETURN(unit, retval, "bcm_field_stat_get");
            cli_out("%s, value is: 0x%x%x\n", robo_stattype_text[stat_arr[idx]],
                    COMPILER_64_HI(val64), COMPILER_64_LO(val64));
        }
        return CMD_OK;
    }

    retval = bcm_field_stat_get(unit, statid, stat_arr[0], &val64);
    FP_CHECK_RETURN(unit, retval, "bcm_field_stat_get");
    cli_out("The value is: 0x%x%x\n", COMPILER_64_HI(val64), 
            COMPILER_64_LO(val64));
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_stat
 * Purpose:
 *     bcm_field_stat_xxx CLI implementation. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_stat(int unit, args_t *args)
{
    char* subcmd = NULL;
 
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp stat create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return robo_fp_stat_create(unit, args);
    }

    /* BCM.0> fp stat destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return robo_fp_stat_destroy(unit, args);
    }

    /* BCM.0> fp stat attach ... */
    if(!sal_strcasecmp(subcmd, "attach")) {
        return robo_fp_stat_attach(unit, args);
    }

    /* BCM.0> fp stat detach ... */
    if(!sal_strcasecmp(subcmd, "detach")) {
        return robo_fp_stat_detach(unit, args);
    }

    /* BCM.0> fp stat set ... */
    if(!sal_strcasecmp(subcmd, "set")) {
        return robo_fp_stat_set(unit, args);
    }

    /* BCM.0> fp stat get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return robo_fp_stat_get(unit, args);
    }
    return CMD_USAGE;
}


/*
 * Function:
 *     robo_fp_data_create
 * Purpose:
 *     Add fp data qualifier entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_data_create(int unit, args_t *args)
{
    parse_table_t              pt;
    int                        length = -1;
    int                        offset = -1;
    int                        offset_base = 0; /* Packet Start. */
    cmd_result_t               retCode;
    bcm_field_data_qualifier_t data_qual;
    int                        rv;
    int                        qualid = -1;

    /* Initialization. */
    bcm_field_data_qualifier_t_init(&data_qual);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "OffsetBase", PQ_DFL | PQ_MULTI, 0,
                    (void *)&offset_base, offsetbase_text);
    parse_table_add(&pt, "offset", PQ_DFL|PQ_INT, 0, &offset, NULL);
    parse_table_add(&pt, "length", PQ_DFL|PQ_INT, 0, &length, NULL);
    parse_table_add(&pt, "QualId", PQ_DFL|PQ_INT, 0, &qualid, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (offset < 0) {
        cli_out("Offset parse failed.\n");
        return CMD_FAIL;
    } 

    if (length < 0) {
        cli_out("Offset parse failed.\n");
        return CMD_FAIL;
    }

    if (qualid >= 0) {
        data_qual.flags |= BCM_FIELD_DATA_QUALIFIER_WITH_ID;
        data_qual.qual_id = qualid;
    }

    data_qual.offset_base = offset_base;
    data_qual.offset = offset;
    data_qual.length = length;
    rv = bcm_field_data_qualifier_create(unit, &data_qual);
    if (BCM_FAILURE(rv)) {
        cli_out("Data qualifier add failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }
    cli_out("Data qualifier created with id: %d \n", data_qual.qual_id);
    return CMD_OK;
}


/*
 * Function:
 *     robo_fp_data_destroy
 * Purpose:
 *     Destroy fp data qualifier entity. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_data_destroy(int unit, args_t *args)
{
    char                       *param;
    cmd_result_t               retCode;
    parse_table_t              pt;
    int                        qualid = -1;
    int                        rv;

    param = ARG_CUR(args);
    if (!param) {
        return CMD_USAGE;
    }

    if (0 == sal_strncasecmp(param, "all", 3)) {
        /* Delete all data qualifiers. */
        param = ARG_GET(args);
        if ((rv = bcm_field_data_qualifier_destroy_all(unit)) != BCM_E_NONE) {
            cli_out("ERROR: data qualifier destroy all unit=%d) failed.(%s) \n",
                    unit, bcm_errmsg(rv));
            return (CMD_FAIL);
        }
        return (CMD_OK);
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "QualId", PQ_DFL|PQ_INT, 0, &qualid, NULL);

    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (qualid < 0) {
        cli_out("Qualifier id parse failed.\n");
        return CMD_FAIL;
    }

    rv = bcm_field_data_qualifier_destroy(unit, qualid);
    if (BCM_FAILURE(rv)) {
        cli_out("Data qualifier destroy failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }
    return CMD_OK;
}


/*
 * Function:
 *     robo_fp_data_packet_format_add_delete
 * Purpose:
 *     Add/delete packet format to the fp data qualifier. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 *     op - (0 delete, else add)
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_data_packet_format_add_delete(int unit, args_t *args, int op)
{
    bcm_field_data_packet_format_t  pkt_fmt;
    cmd_result_t                    retCode;
    int                             relative_offset = 0;
    int                             vlan_tag = 0;  /* Vlan Tag ANY.*/
    int                             outer_ip = 0;  /* Outer Ip ANY.*/ 
    int                             inner_ip = 0;  /* Inner Ip ANY.*/
    int                             tunnel = 1;    /* Tunnel None. */
    int                             qualid = -1;
    int                             mpls = 0;      /* MPLS labels ANY.*/
    parse_table_t                   pt;
    int                             rv;
    int                             l2 = 0;        /* L2 format ANY. */  

    /* Initialization. */
    bcm_field_data_packet_format_t_init(&pkt_fmt);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "QualId", PQ_DFL|PQ_INT, 0, &qualid, NULL);
    parse_table_add(&pt, "RelativeOffset", PQ_DFL|PQ_INT, 0,
                    &relative_offset, NULL);
    parse_table_add(&pt, "L2", PQ_DFL|PQ_MULTI, 0,
                    (void *)&l2, data_l2_text);
    parse_table_add(&pt, "VlanTag", PQ_DFL|PQ_MULTI, 0,
                    (void *)&vlan_tag, data_vlan_text);
    parse_table_add(&pt, "OuterIp", PQ_DFL|PQ_MULTI, 0,
                    (void *)&outer_ip, data_ip_text);
    parse_table_add(&pt, "InnerIp", PQ_DFL|PQ_MULTI, 0,
                    (void *)&inner_ip, data_ip_text);
    parse_table_add(&pt, "Tunnel", PQ_DFL|PQ_MULTI, 0,
                    (void *)&tunnel, data_tunnel_text);
    parse_table_add(&pt, "Mpls", PQ_DFL|PQ_MULTI, 0,
                    (void *)&mpls, data_mpls_text);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (qualid < 0) {
        cli_out("Qualifier id parse failed.\n");
        return CMD_FAIL;
    }

    pkt_fmt.relative_offset = relative_offset;
    pkt_fmt.l2 = (l2 == 0) ? BCM_FIELD_DATA_FORMAT_L2_ANY : (1 << (l2 - 1));
    pkt_fmt.vlan_tag = (vlan_tag == 0) ? BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY: \
                       (1 << (vlan_tag - 1));
    pkt_fmt.outer_ip = (outer_ip == 0) ? BCM_FIELD_DATA_FORMAT_IP_ANY : \
                       (1 << (outer_ip - 1));
    pkt_fmt.inner_ip = (inner_ip == 0) ? BCM_FIELD_DATA_FORMAT_IP_ANY : \
                       (1 << (inner_ip - 1));
    pkt_fmt.tunnel = (tunnel == 0) ? BCM_FIELD_DATA_FORMAT_TUNNEL_ANY : \
                     (1 << (tunnel - 1));
    pkt_fmt.mpls = (mpls == 0) ? BCM_FIELD_DATA_FORMAT_MPLS_ANY : \
                   (1 << (mpls - 1));

    if (op) {
        rv = bcm_field_data_qualifier_packet_format_add(unit, qualid,
                                                        &pkt_fmt);
        if (BCM_FAILURE(rv)) {
            cli_out("Data qualifier packet format add. (%s) \n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else {
        rv = bcm_field_data_qualifier_packet_format_delete(unit, qualid,
                                                           &pkt_fmt);
        if (BCM_FAILURE(rv)) {
            cli_out("Data qualifier packet format delete. (%s) \n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_data_ethertype_add_delete
 * Purpose:
 *     Add/Delete ethertype to the fp data qualifier. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 *     op - (0 delete, else add)
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_data_ethertype_add_delete(int unit, args_t *args, int op)
{
    bcm_field_data_ethertype_t      eth_type;
    cmd_result_t                    retCode;
    int                             relative_offset = 0;
    int                             vlan_tag = 0;  /* Vlan Tag ANY.   */
    int                             l2 = 0;        /* L2 format ANY.  */  
    int                             ethertype = 0; /* EtherType value.*/
    int                             qualid = -1;
    parse_table_t                   pt;
    int                             rv;

    /* Initialization. */
    bcm_field_data_ethertype_t_init(&eth_type);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "QualId", PQ_DFL|PQ_INT, 0, &qualid, NULL);
    parse_table_add(&pt, "RelativeOffset", PQ_DFL|PQ_INT, 0,
                    &relative_offset, NULL);
    parse_table_add(&pt, "etype", PQ_DFL|PQ_INT, 0, &ethertype, NULL);
    parse_table_add(&pt, "L2", PQ_DFL|PQ_MULTI, 0,
                    (void *)&l2, data_l2_text);
    parse_table_add(&pt, "VlanTag", PQ_DFL|PQ_MULTI, 0,
                    (void *)&vlan_tag, data_vlan_text);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (qualid < 0) {
        cli_out("Qualifier id parse failed.\n");
        return CMD_FAIL;
    }

    eth_type.relative_offset = relative_offset;
    eth_type.l2 = (l2 == 0) ? BCM_FIELD_DATA_FORMAT_L2_ANY : (1 << (l2 - 1));
    eth_type.vlan_tag = (vlan_tag == 0) ? BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY: \
                       (1 << (vlan_tag - 1));
    eth_type.ethertype = ethertype;

    if (op) {
        rv = bcm_field_data_qualifier_ethertype_add(unit, qualid, &eth_type);
        if (BCM_FAILURE(rv)) {
            cli_out("Data qualifier ethertype add. (%s) \n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else {
        rv = bcm_field_data_qualifier_ethertype_delete(unit, qualid, &eth_type);
        if (BCM_FAILURE(rv)) {
            cli_out("Data qualifier ethertype delete. (%s) \n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }
    return CMD_OK;
}

/*
 * Function:
 *     robo_fp_data_ipproto_add_delete
 * Purpose:
 *     Add/Delete ip protocol to the fp data qualifier. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 *     op - (0 delete, else add)
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_data_ipproto_add_delete(int unit, args_t *args, int op)
{
    bcm_field_data_ip_protocol_t    ipproto;
    cmd_result_t                    retCode;
    int                             relative_offset = 0;
    int                             vlan_tag = 0;   /* Vlan Tag ANY.   */
    int                             l2 = 0;         /* L2 format ANY.  */  
    int                             ipver = 0;      /* Ip type ANY.    */ 
    int                             protocol = -1;  /* Protocol value. */
    int                             qualid = -1;
    parse_table_t                   pt;
    int                             rv;

    /* Initialization. */
    bcm_field_data_ip_protocol_t_init(&ipproto);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "QualId", PQ_DFL|PQ_INT, 0, &qualid, NULL);
    parse_table_add(&pt, "RelativeOffset", PQ_DFL|PQ_INT, 0,
                    &relative_offset, NULL);
    parse_table_add(&pt, "protocol", PQ_DFL|PQ_INT, 0, &protocol, NULL);
    parse_table_add(&pt, "L2", PQ_DFL|PQ_MULTI, 0,
                    (void *)&l2, data_l2_text);
    parse_table_add(&pt, "VlanTag", PQ_DFL|PQ_MULTI, 0,
                    (void *)&vlan_tag, data_vlan_text);
    parse_table_add(&pt, "IpVer", PQ_DFL|PQ_MULTI, 0,
                    (void *)&ipver, data_ip_text);
    if (!parseEndOk(args, &pt, &retCode)) {
        return retCode;
    }

    if (qualid < 0) {
        cli_out("Qualifier id parse failed.\n");
        return CMD_FAIL;
    }
    if (protocol < 0) {
        cli_out("Ip protocol parse failed.\n");
        return CMD_FAIL;
    }

    ipproto.relative_offset = relative_offset;
    ipproto.l2 = (l2 == 0) ? BCM_FIELD_DATA_FORMAT_L2_ANY : (1 << (l2 - 1));
    ipproto.vlan_tag = (vlan_tag == 0) ? BCM_FIELD_DATA_FORMAT_VLAN_TAG_ANY: \
                       (1 << (vlan_tag - 1));
    switch (ipver) {
      case 0: /* Any */
          ipproto.flags = (BCM_FIELD_DATA_FORMAT_IP4 |
                           BCM_FIELD_DATA_FORMAT_IP6);
          break;
      case 1: /* None */
          ipproto.flags = 0;
          break;
      case 2: /* Ip4 */
          ipproto.flags = BCM_FIELD_DATA_FORMAT_IP4;
          break;
      case 3: /* Ip6 */
          ipproto.flags = BCM_FIELD_DATA_FORMAT_IP6;
          break;
      default:
          cli_out("Invalid IP version.\n");
          return CMD_FAIL;
    }
    ipproto.ip = protocol;

    if (op) {
        rv = bcm_field_data_qualifier_ip_protocol_add(unit, qualid, &ipproto);
        if (BCM_FAILURE(rv)) {
            cli_out("Data qualifier ip protocol add. (%s) \n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    } else {
        rv = bcm_field_data_qualifier_ip_protocol_delete(unit, qualid, &ipproto);
        if (BCM_FAILURE(rv)) {
            cli_out("Data qualifier protocol delete. (%s) \n",
                    bcm_errmsg(rv));
            return CMD_FAIL;
        }
    }
    return CMD_OK;
}



/*
 * Function:
 *     robo_fp_data
 * Purpose:
 *     bcm_field_data_xxx CLI implementation. 
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
STATIC int
robo_fp_data(int unit, args_t *args)
{
    char* subcmd = NULL;
 
    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp data create ... */
    if(!sal_strcasecmp(subcmd, "create")) {
        return robo_fp_data_create(unit, args);
    }

    /* BCM.0> fp data destroy ... */
    if(!sal_strcasecmp(subcmd, "destroy")) {
        return robo_fp_data_destroy(unit, args);
    }

    /* BCM.0> fp data format ... */
    if(!sal_strcasecmp(subcmd, "format")) {
        if ((subcmd = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        if(!sal_strcasecmp(subcmd, "add")) {
            return robo_fp_data_packet_format_add_delete(unit, args, TRUE);
        }else if(!sal_strcasecmp(subcmd, "delete")) {
            return robo_fp_data_packet_format_add_delete(unit, args, FALSE);
        } else {
            return CMD_USAGE;
        }
    }

    /* BCM.0> fp data ethertype ... */
    if(!sal_strcasecmp(subcmd, "ethertype")) {
        if ((subcmd = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        if(!sal_strcasecmp(subcmd, "add")) {
            return robo_fp_data_ethertype_add_delete(unit, args, TRUE);
        }else if(!sal_strcasecmp(subcmd, "delete")) {
            return robo_fp_data_ethertype_add_delete(unit, args, FALSE);
        } else {
            return CMD_USAGE;
        }
    }

    /* BCM.0> fp data ipprotocol ... */
    if(!sal_strcasecmp(subcmd, "ipproto")) {
        if ((subcmd = ARG_GET(args)) == NULL) {
            return CMD_USAGE;
        }
        if(!sal_strcasecmp(subcmd, "add")) {
            return robo_fp_data_ipproto_add_delete(unit, args, TRUE);
        }else if(!sal_strcasecmp(subcmd, "delete")) {
            return robo_fp_data_ipproto_add_delete(unit, args, FALSE);
        } else {
            return CMD_USAGE;
        }
    }
    return CMD_USAGE;
}


#endif  /* BCM_FIELD_SUPPORT */
