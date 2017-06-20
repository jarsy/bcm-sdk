/*
 * $Id: field.c,v 1.22 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * Field Processor related CLI commands
 */

#include <shared/bsl.h>

#include <sal/core/libc.h>
#include <shared/alloc.h>
#include <soc/defs.h>

#include <appl/diag/system.h>
#include <sal/core/libc.h>
#include <sal/types.h>
#include <sal/appl/sal.h>
#include <sal/appl/io.h>

#include <appl/diag/shell.h>
#include <appl/diag/parse.h>
#include <appl/diag/diag.h>
#include <appl/diag/dport.h>

#include <bcm/init.h>
#include <bcm/field.h>
#include <bcm/error.h>

#include <soc/profile_mem.h>

#ifdef BCM_FIELD_SUPPORT

/* How many character per line */
#define FP_LINE_SZ 79

/* Some string size limit? */
#define FP_STAT_STR_SZ 256

/* Maximum policing levels? */
#define _FP_POLICER_LEVEL_COUNT       (2)

/* Maximum working stat array size (for these commands)? */
#define _BCM_CLI_STAT_ARR_SIZE     (5)

/*
 * Macro:
 *     FP_VRFY_GET_NUMB
 * Purpose:
 *     Get a numerical value from args, after making sure it's a number.
 */
#define FP_VRFY_GET_NUMB(numb, str, args) \
    if ((NULL == ((str) = ARG_GET(args))) || (!isint(str))) { \
        return CMD_USAGE; \
    } else { \
        (numb) = parse_integer(str); \
    }

/*
 * Macro:
 *     FP_GET_MAC
 * Purpose:
 *     Get a mac address from args.
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
 *     Get a port from args.
 */
#define FP_GET_PORT(_unit, _port, _str, _args)                   \
    if (((_str) = ARG_GET(_args)) == NULL) {                     \
        return CMD_USAGE;                                        \
    }                                                            \
    if (parse_bcm_port((_unit), (_str), &(_port)) < 0) {         \
       LOG_ERROR(BSL_LS_BCM_FP, \
                 (BSL_META_U(_unit, \
                             "FP(unit %d) Error: invalid port string: \"%s\"\n"), \
                  (_unit), (_str)));                                    \
       return CMD_FAIL;                                          \
    }

/*
 * Macro:
 *     FP_CHECK_RETURN
 * Purpose:
 *     Check the return value from an API call. Output either a failed
 *     message or okay along with the function name.
 */
#define FP_CHECK_RETURN(unit, retval, funct_name)                  \
    if (BCM_FAILURE(retval)) {                                     \
        cli_out(\
                "FP(unit %d) Error: %s() failed: %s\n", (unit),     \
                (funct_name), bcm_errmsg(retval));                 \
        return CMD_FAIL;                                           \
    } else {                                                       \
        LOG_VERBOSE(BSL_LS_BCM_FP,                                 \
                    (BSL_META_U(unit,                              \
                                "FP(unit %d) verb: %s() success \n"),   \
                     (unit), (funct_name)));                            \
    }

/*
 * Typedef:
 *     _bcm_field_qual_data_t
 * Purpose:
 *     Qualifier data/mask field buffer.
 */
#define _FP_QUAL_DATA_WORDS  (4)
typedef uint32 _bcm_field_qual_data_t[_FP_QUAL_DATA_WORDS];   /* Qualifier data/mask buffer. */

/* Internal DATA qualifiers. */
typedef enum _bcm_field_internal_qualify_e {
    _bcmFieldQualifyData0 = bcmFieldQualifyCount,/* [0x00] Data qualifier 0.  */
    _bcmFieldQualifyData1,                    /* [0x01] Data qualifier 1.     */
    _bcmFieldQualifyCount                     /* [0x02] Always last not used. */
} _bcm_field_internal_qualify_t;


const char if_sbx_field_proc_usage[] =
    "Manipulate the field processor\n"
    "\tfp action <subcommand>\n"
    "\tfp control <control> [<state>]\n"
    "\tfp counter <subcommand>\n"
    "\tfp data <subcommand>\n"
    "\tfp detach\n"
    "\tfp entry <subcommand>\n"
    "\tfp group <subcommnad>\n"
    "\tfp init\n"
    "\tfp install <entry>\n"
    "\tfp list {actions | qualifiers | quals | user | users}\n"
    "\tfp meter <subcommand>\n"
    "\tfp policer <subcommand>\n"
    "\tfp qset <subcommand>\n"
    "\tfp qual <subcommand>\n"
    "\tfp range <subcommand>\n"
    "\tfp resync\n"
    "\tfp show [<args>]\n"
    "\tfp stat <subcommand>\n"
    "\tfp status\n"
    "\tfp user <subcommand>\n";
static const char if_field_proc_action_usage[] =
    "Manipulate field processor actions\n"
    "\tfp action add <entry> <action> [<arg0> [<arg1>]]\n"
    "\tfp action get <entry> <action>\n"
    "\tfp action mac add <entry> <action> <MACaddr>\n"
    "\tfp action mac get <entry> <action>\n"
    "\tfp action ports add <entry> <action> <pbmp>\n"
    "\tfp action ports get <entry> <action>\n"
    "\tfp action remove <entry> [<action>]\n"
    "Some actions require both arg0 and arg1, some only arg0, some neither.\n";
static const char if_field_proc_counter_usage[] =
    "Manipulate field processor counters\n"
    "\tfp counter create <entry>\n"
    "\tfp counter destroy <entry>\n"
    "\tfp counter get <entry> [<counter>]\n"
    "\tfp counter set <entry> <counter> <value>\n"
    "\tfp counter share <src entry> <dst entry>\n";
static const char if_field_proc_data_usage[] =
    "Manipulate field processor data qualifiers\n"
#ifndef COMPILER_STRING_CONST_LIMIT
    "\tfp data create OffsetBase=<offset base> Offset=<offset>\n"
    "\t               Length=<length>\n"
    "\tfp data destroy all\n"
    "\tfp data destroy QualId=<qual ID>\n"
    "\tfp data ethertype add QualId=<qual ID> RelativeOffset=<rel offs>\n"
    "\t                      etype=<ethertype> VlanTag=<tag format> L2=<l2>\n"
    "\tfp data ethertype delete QualId=<qual ID> RelativeOffset=<rel offs>\n"
    "\t                         etype=<ethertype> VlanTag=<tag format>\n"
    "\t                         L2=<l2>\n"
    "\tfp data format add QualId=<qual ID> RelativeOffset=<rel offs>\n"
    "\t                   L2=<l2> VlanTag=<tag format> OuterIp=<outer IP>\n"
    "\t                   InnerIp=<inner IP> Tunnel=<tunnel> mpls=<mpls>\n"
    "\tfp data format delete QualId=<qual ID> RelativeOffset=<rel offs>\n"
    "\t                      L2=<l2> VlanTag=<tag format> OuterIp=<outer IP>\n"
    "\t                      InnerIp=<inner IP> Tunnel=<tunnel> mpls=<mpls>\n"
    "\tfp data ipproto add QualId=<qual ID> RelativeOffset=<rel offs>\n"
    "\t                    protocol=<protocol> VlanTag=<tag format> L2=<l2>\n"
    "\t                    IpVer=<IP version>\n"
    "\tfp data ipproto delete QualId=<qual ID> RelativeOffset=<rel offs>\n"
    "\t                       protocol=<protocol> VlanTag=<tag format>\n"
    "\t                       L2=<l2> IpVer=<IP version>\n"
#endif
;
static const char if_field_proc_entry_usage[] =
    "Manipulate field processor entries\n"
    "\tfp entry copy <src entry> [<dst entry>]\n"
    "\tfp entry create <group> [<entry>]\n"
    "\tfp entry destroy [<entry>]\n"
    "\tfp entry install <entry>\n"
    "\tfp entry prio <entry> [<priority>]\n"
    "\tfp entry reinstall <entry>\n"
    "\tfp entry remove <entry>\n"
#ifdef BROADCOM_DEBUG
    "\tfp entry show <entry>\n"
#endif /* def BROADCOM_DEBUG */
    "The value for <priority> is one of {highest | lowest | dontcare | default}\n"
    "or it is numeric (higher numbers indicate higher priority).\n";
static const char if_field_proc_group_usage[] =
    "Manipulate field processor groups\n"
    "\tfp group create <prio> [<group> [<mode> [<pbmp>]]]\n"
    "\tfp group destroy <group>\n"
    "\tfp group get <group>\n"
    "\tfp group install <group>\n"
    "\tfp group lookup <group> [{enable | disable}]\n"
    "\tfp group mode <group>\n"
    "\tfp group remove <group>\n"
    "\tfp group set <group>\n"
#ifdef BROADCOM_DEBUG
    "\tfp group show <group>\n"
#endif /* def BROADCOM_DEBUG */
    "\tfp group status <group>\n"
    "The value of <prio> is numeric and limited in some devices (higher numbers\n"
    "indicate higher priority).  Only one group can have a given priority.\n";
static const char if_field_proc_meter_usage[] =
    "Manipulate field processor meters\n"
    "\tfp meter create <entry>\n"
    "\tfp meter destroy <entry>\n"
    "\tfp meter getc <entry>\n"
    "\tfp meter getp <entry>\n"
    "\tfp meter setc <entry>\n"
    "\tfp meter setp <entry>\n"
    "\tfp meter share <src entry> <dst entry>\n";
static const char if_field_proc_policer_usage[] =
    "Manipulate field processor policers\n"
    "\tfp policer attach entry=<entry> level=<level> PolId=<policer ID>\n"
    "\tfp policer create PolId=<policer ID> mode=<mode> cbs=<cbs>\n"
    "\t                  cir=<cir> ebs=<ebx> eir=<eir>\n"
    "\t                  ColorBlind=<color blind>\n"
    "\t                  ColorMergeOr=<color merge or>\n"
    "\tfp policer destroy all\n"
    "\tfp policer destroy PolId=<policer ID>\n"
    "\tfp policer detach entry=<entry> level=<level>\n";
static const char if_field_proc_qset_usage[] =
    "Manipulate working qualifier set\n"
    "\tfp qset add <qualifier> [...]\n"
    "\tfp qset clear\n"
    "\tfp qset set {\"<qualifier> [...]\"|<qset as hex digits>} \n"
    "\tfp qset show\n";
static const char if_field_proc_qual_usage[] =
    "Manipulate qualifiers on an entry\n"
    "\tfp qual <entry> clear\n"
    "\tfp qual <entry> <qualifier> [<value> [<mask>]]\n"
    "\tfp qual <entry> Data <Data ID> <value> <mask>\n"
    "\tfp qual <entry> delete <qualifier>\n"
    "\tfp qual <entry> show|?\n"
    "\tfp qual <entry> UserDefined <UDF ID> <value> <mask>\n"
    "Some qualifiers require both value and mask, some only value, some neither.\n";
static const char if_field_proc_range_usage[] =
    "Manipulate field processor ranges\n"
    "\tfp range create [<range>] <flags> <min> <max>\n"
    "\tfp range destroy <range>\n"
    "\tfp range get <range>\n"
    "\tfp range group create [<range>] <flags> <min> <max> <group>\n";
#ifdef BROADCOM_DEBUG
static const char if_field_proc_show_usage[] =
    "Display information about the range processor\n"
    "\tfp show\n"
    "\tfp show entry <entry>\n"
    "\tfp show group <group>\n"
    "\tfp show qset\n";
#endif /* def BROADCOM_DEBUG */
static const char if_field_proc_stat_usage[] =
    "Manipulate field processor statistics\n"
    "\tfp stat attach entry=<entry> StatId=<stat ID>\n"
    "\tfp stat create group=<group> type0=<stat type 0> type1=<stat type 1>\n"
    "\t               ... type<n>=<stat type <n>>\n"
    "\tfp stat destroy all\n"
    "\tfp stat destroy StatId=<stat ID>\n"
    "\tfp stat detach entry=<entry> StatId=<stat ID>\n"
    "\tfp stat get StatId=<stat ID> type=<type>\n"
    "\tfp stat set StatId=<stat ID> type=<type> val=<value>\n";
static const char if_field_proc_user_usage[] =
    "Manipulate field processor UDFs\n"
    "\tfp user create [<UDF ID>]\n"
    "\tfp user init\n"
    "\tfp user get [<UDF ID>]\n"
    "\tfp user set <offset> L2=<l2> VLan=<tag format> IP=<IP type>\n"
    "\t            AdjustIpOptions={True | False}\n"
    "\t            AdjustGreOptions={True | False}\n"
    "\t            Higig={True | False} Higig2={True | False}\n";

static const char *packet_res_text[] = {"Unknown",
                                        "Control",
                                        "Bpdu",
                                        "L2BC",
                                        "L2Uc",
                                        "L2UnKnown",
                                        "L3McUnknown",
                                        "L3McKnown",
                                        "L2McKnown",
                                        "L2McUnknown",
                                        "L3UcKnown",
                                        "L3UcUnknown",
                                        "MplsKnown",
                                        "MplsL3Known",
                                        "MplsL2Known",
                                        "MplsUnknown",
                                        NULL};
static const char *ipfrag_text[] = {"IpFragNon",
                                    "IpFragFirst",
                                    "IpFragNonOrFirst",
                                    "IpFragNotFirst",
                                    "IpFragAny",
                                    NULL};
static const char *loopbacktype_text[] = {"Any",
                                          "Mirror",
                                          "Wlan",
                                          "Mim",
                                          "Redirect",
                                          NULL};
static const char *tunneltype_text[] = {"Any",
                                        "Ip",
                                        "Mpls",
                                        "Mim",
                                        "WtpToAc",
                                        "AcToAc",
                                        "AutoMulticast",
                                        NULL};
static const char *color_text[] = {"green",
                                   "yellow",
                                   "red",
                                   NULL};
static const char *policermode_text[] = {"SrTcm",
                                         "Committed",
                                         "Peak",
                                         "TrTcm",
                                         "TrTcmDs",
                                         "Green",
                                         "PassThrough",
                                         "SrTcmModified",
                                         "CoupledTrTcmDs",
                                         "Invalid",
                                         NULL};
static const char *stattype_text[] = {"Bytes",
                                      "Packets",
                                      "GreenBytes",
                                      "GreenPackets",
                                      "YellowBytes",
                                      "YellowPackets",
                                      "RedBytes",
                                      "RedPackets",
                                      "NotGreenBytes",
                                      "NotGreenPackets",
                                      "NotYellowBytes",
                                      "NotYellowPackets",
                                      "NotRedBytes",
                                      "NotRedPackets",
                                      "Invalid",
                                      NULL};
static const char *offsetbase_text[] = {"PacketStart",
                                        "OuterL3Header",
                                        "InnerL3Header",
                                        "OuterL4Header",
                                        "InnerL4Header",
                                        "HigigHeader",
                                        "Higig2Header",
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

/*
 * Function:
 *     _fp_qual_IpType_name
 * Purpose:
 *     Translate IpType enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_IpType_e. (ex.bcmFieldIpTypeNonIp)
 * Returns:
 *     Text name of indicated IpType qualifier enum value.
 */
static char *
_fp_qual_IpType_name(bcm_field_IpType_t type)
{
    /* Text names of the enumerated qualifier IpType values. */
    /* All these are prefixed with "bcmFieldIpType" */
    char *IpType_text[bcmFieldIpTypeCount] = BCM_FIELD_IPTYPE_STRINGS;

    assert(COUNTOF(IpType_text) == bcmFieldIpTypeCount);

    return (type >= bcmFieldIpTypeCount ? "??" : IpType_text[type]);
}

/*
 * Function:
 *     _fp_qual_L2Format_name
 * Purpose:
 *     Translate L2Format enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_L2Format_e. (ex.bcmFieldL2FormatSnap)
 * Returns:
 *     Text name of indicated L2Format qualifier enum value.
 */
static char *
_fp_qual_L2Format_name(bcm_field_L2Format_t type)
{
    /* Text names of the enumerated qualifier L2Format values. */
    /* All these are prefixed with "bcmFieldL2Format" */
    char *L2Format_text[bcmFieldL2FormatCount] = BCM_FIELD_L2FORMAT_STRINGS;

    assert(COUNTOF(L2Format_text) == bcmFieldL2FormatCount);

    return (type >= bcmFieldL2FormatCount ? "??" : L2Format_text[type]);
}

/*
 * Function:
 *     _fp_qual_stage_name
 * Purpose:
 *     Translate a stage qualifier enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_stage_e. (ex.bcmFieldStageIngressEarly)
 * Returns:
 *     Text name of indicated stage qualifier enum value.
 */
static char *
_fp_qual_stage_name(bcm_field_stage_t stage)
{
    /* Text names of the enumerated qualifier stage values. */
    /* All these are prefixed with "bcmFieldStage" */
    char *stage_text[] = BCM_FIELD_STAGE_STRINGS;

    assert(COUNTOF(stage_text) == bcmFieldStageCount);

    return (stage >= bcmFieldStageCount ? "??" : stage_text[stage]);
}

/*
 * Function:
 *     _field_qual_name
 * Purpose:
 *     Translate a Qualifier enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_qualify_e. (ex.bcmFieldQualifyInPorts)
 * Returns:
 *     Text name of indicated qualifier enum value.
 */
static char *
_field_qual_name(bcm_field_qualify_t qid)
{
    /* Text names of the enumerated qualifier IDs. */
    static char *qual_text[bcmFieldQualifyCount] = BCM_FIELD_QUALIFY_STRINGS;

    if (qid < bcmFieldQualifyCount) {
        return qual_text[qid];
    } else if ((int) qid < _bcmFieldQualifyCount) {
        return "_bcmFieldQualifyData";
    }
    return "UnknownQualifier";
}

/*
 * Function:
 *     _fp_control_name
 * Purpose:
 *     Return text name of indicated control enum value.
 */
static char *
_fp_control_name(bcm_field_control_t control)
{
    /* Text names of Controls. These are used for debugging output and CLIs.
     * Note that the order needs to match the bcm_field_control_t enum order.
     */
    char *control_text[] = BCM_FIELD_CONTROL_STRINGS;
    assert(COUNTOF(control_text)     == bcmFieldControlCount);

    return (control >= bcmFieldControlCount ? "??" : control_text[control]);
}

/*
 *  Parse a string from hexadecimal into bytes
 */
static int
fp_parse_hex_string(char *str, int buf_sz, uint8 *buffer, int *length)
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
 *    fp_lookup_IpType
 * Purpose:
 *    Lookup a field IpType value from a user string.
 * Parmameters:
 *     type_str - search string
 * Returns:
 *     corresponding IpType value
 *
 */
static bcm_field_IpType_t
fp_lookup_IpType(const char *type_str)
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
        strncpy(tbl_str, _fp_qual_IpType_name(type), FP_STAT_STR_SZ - 1);
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
 *    fp_lookup_L2Format
 * Purpose:
 *    Lookup a field L2Format value from a user string.
 * Parmameters:
 *     type_str - search string
 * Returns:
 *     corresponding L2Format value
 *
 */
static bcm_field_L2Format_t
fp_lookup_L2Format(const char *type_str)
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
        strncpy(tbl_str, _fp_qual_L2Format_name(type), FP_STAT_STR_SZ - 1);
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
 *    fp_lookup_stage
 * Purpose:
 *    Lookup a field stage value from a user string.
 * Parmameters:
 *     stage_str - search string
 *     stage     - (OUT) corresponding stage value
 * Returns:
 *
 */
static bcm_field_stage_t
fp_lookup_stage(const char *stage_str)
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
        strncpy(tbl_str, _fp_qual_stage_name(stage), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, stage_str)) {
            break;
        }
        /* Test for whole name of the Stage */
        sal_strcpy(lng_str, "bcmFieldStage");
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
 *     fp_print_options
 * Purpose:
 *     Print an array of options bracketed by <>.
 * Parmameters:
 *     options - NULL terminated array of pointers to strings.
 * Returns:
 *     Nothing
 */
static void
fp_print_options(const char *options[], const int offset)
{
    int                 idx;
    int                 char_count = offset;
    int                 len;

    for (idx = 0; options[idx] != NULL; idx++) {
        len = (idx?3:1) + strlen(options[idx]);
        if ((char_count + len) >= FP_LINE_SZ) {
            cli_out("\n%-*s", offset, "");
            char_count = offset;
        }
        cli_out("%s%s", idx?" | ":"{", options[idx]);
        char_count += len;
    }
    cli_out("}");
}

static int
fp_qual_IpInfo_help(const char *prefix, int width_col1) {
    if (width_col1 < strlen("IpInfo")) {
        width_col1 = strlen("IpInfo") + 1;
    }

    cli_out("%s%-*s%s\n", prefix, width_col1, "IpInfo",
            "HeaderOffsetZero=<0/1> HeaderFlagsMF=<0/1> ChecksumOK=<0/1>");
    return CMD_OK;
}

static int
fp_qual_PacketRes_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("PacketRes")) {
        width_col1 = strlen("PacketRes") + 1;
    }

    offset = cli_out("%s%-*sRes=", prefix, width_col1, "PacketRes");
    fp_print_options(packet_res_text, offset);
    cli_out("\n");

    return CMD_OK;
}

static int
fp_qual_IpFrag_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("IpFrag")) {
        width_col1 = strlen("IpFrag") + 1;
    }

    offset = cli_out("%s%-*sfrag=", prefix, width_col1, "IpFrag");
    fp_print_options(ipfrag_text, offset);
    cli_out("\n");

    return CMD_OK;
}

static int
fp_qual_LoopbackType_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("LoopbackType")) {
        width_col1 = strlen("LoopbackType") + 1;
    }

    offset = cli_out("%s%-*slb_type=", prefix, width_col1, "LoopbackType");
    fp_print_options(loopbacktype_text, offset);
    cli_out("\n");

    return CMD_OK;
}

static int
fp_qual_TunnelType_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 < strlen("TunnelType")) {
        width_col1 = strlen("TunnelType") + 1;
    }

    offset = cli_out("%s%-*stnl_type=", prefix, width_col1, "TunnelType");
    fp_print_options(tunneltype_text, offset);
    cli_out("\n");

    return CMD_OK;
}

static int
fp_qual_modport_help(const char *prefix, const char *qual_str, int width_col1)
{
    if (width_col1 < sal_strlen(qual_str)) {
        width_col1 = sal_strlen(qual_str) + 1;
    }

    /* "FieldProcessor qual <eid> DstPort" */
    cli_out("%s%-*s%s\n", prefix, width_col1, qual_str,
            "Port=<port_numb> PortMask=<mask>");
    cli_out("%s%-*s%s\n", prefix, width_col1, "",
            "Modid=<mod> ModidMask=<mask>");

    return CMD_OK;
}

static int
fp_qual_mac_help(const char *prefix, const char *qual_str, int width_col1)
{
    if (width_col1 <= sal_strlen(qual_str)) {
        width_col1 = sal_strlen(qual_str) + 1;
    }

    /* "FieldProcessor qual <eid> XxxMac" */
    cli_out("%s%-*s%s\n", prefix, width_col1, qual_str,
            "Data=<mac> Mask=<mac>");

    return CMD_OK;
}

static int
fp_qual_Color_help(const char *prefix, int width_col1) {
    int                 offset;
    if (width_col1 <= strlen("Color")) {
        width_col1 = strlen("Color") + 1;
    }

    offset = cli_out("%s%-*scolor=", prefix, width_col1, "color");
    fp_print_options(color_text, offset);
    cli_out("\n");

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
static int
fp_list_quals(int unit)
{
    bcm_field_qualify_t qual;
    char                buf[BCM_FIELD_QUALIFY_WIDTH_MAX];
    int                 width_col1 = 20, width_col2 = 40, width_col3= 20;
    char ***param_table = sal_alloc(bcmFieldQualifyCount * sizeof(param_table),
                                    "Qualifier Space");

    /* Use heap memory than stack to reduce Stack_use */
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        param_table[qual] = sal_alloc(2 * sizeof(param_table), "");
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
    param_table[bcmFieldQualifySrcIp6Low][0]       = "Lower 64-bits of Source IPv6 Address";
    param_table[bcmFieldQualifySrcIp6Low][1]       = "64-bits of IPv6 Address mask";
    param_table[bcmFieldQualifyDstIp6Low][0]       = "Lower 64-bits of Destination IPv6 Address";
    param_table[bcmFieldQualifyDstIp6Low][1]       = "64-bits of IPv6 Address mask";
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
    param_table[bcmFieldQualifyOuterVlanPri][0]    = "Outer VLAN priority";
    param_table[bcmFieldQualifyOuterVlanPri][1]    = "3-bit mask";
    param_table[bcmFieldQualifyOuterVlanCfi][0]    = "Outer VLAN CFI";
    param_table[bcmFieldQualifyOuterVlanCfi][1]    = "1-bit mask";
    param_table[bcmFieldQualifyOuterVlanId][0]     = "Outer VLAN id";
    param_table[bcmFieldQualifyOuterVlanId][1]     = "12-bit mask";
    param_table[bcmFieldQualifyInnerVlan][0]       = "Inner VLAN tag";
    param_table[bcmFieldQualifyInnerVlan][1]       = "16-bit mask";
    param_table[bcmFieldQualifyInnerVlanPri][0]    = "Inner VLAN priority";
    param_table[bcmFieldQualifyInnerVlanPri][1]    = "3-bit mask";
    param_table[bcmFieldQualifyInnerVlanCfi][0]    = "Inner VLAN CFI";
    param_table[bcmFieldQualifyInnerVlanCfi][1]    = "1-bit mask";
    param_table[bcmFieldQualifyInnerVlanId][0]     = "Inner VLAN id";
    param_table[bcmFieldQualifyInnerVlanId][1]     = "12-bit mask";
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
    param_table[bcmFieldQualifySrcTrunk][0]        = "Source Trunk Group ID";
    param_table[bcmFieldQualifySrcTrunk][1]        = "8-bit mask";
    param_table[bcmFieldQualifyDstTrunk][0]        = "Destination Trunk Group ID";
    param_table[bcmFieldQualifyDstTrunk][1]        = "8-bit mask";
    param_table[bcmFieldQualifyTcpControl][0]      = "TCP control flags";
    param_table[bcmFieldQualifyTcpControl][1]      = "8-bit mask";
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

    cli_out("%-*s%-*s%-*s\n", width_col1, "QUALIFIER", width_col2, "DATA",
            width_col3, "MASK");
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
            qual == bcmFieldQualifyDstModid ||
            qual == bcmFieldQualifyDstPortTgid ||
            qual == bcmFieldQualifyPacketFormat ||
            qual == bcmFieldQualifySrcModid ||
            qual == bcmFieldQualifySrcPortTgid ||
            qual == bcmFieldQualifyIpInfo ||
            qual == bcmFieldQualifyLookupStatus) {
            continue;
        }
        cli_out("%-*s%-*s%-*s\n",
                width_col1, format_field_qualifier(buf, qual, 1),
                width_col2, param_table[qual][0],
                width_col3, param_table[qual][1]);
    }

    /* Print the qualifiers that use parse tables. */
    fp_qual_mac_help("", "SrcMac", width_col1);
    fp_qual_mac_help("", "DstMac", width_col1);
    fp_qual_PacketRes_help("", width_col1);
    fp_qual_IpFrag_help("", width_col1);
    fp_qual_LoopbackType_help("", width_col1);
    fp_qual_TunnelType_help("", width_col1);
    fp_qual_modport_help("", "SrcPort", width_col1);
    fp_qual_modport_help("", "DstPort", width_col1);
    fp_qual_IpInfo_help("", width_col1);
    fp_qual_Color_help("", width_col1);

    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        sal_free(param_table[qual]);
    }
    sal_free(param_table);
    return CMD_OK;
}

/*
 * Function:
 *     fp_list_actions
 * Purpose:
 *     Display a list of Field actions
 * Parmameters:
 * Returns:
 */
typedef char *_fp_list_actions_param_table[2];
static int
fp_list_actions(int unit) {
    bcm_field_action_t action;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    _fp_list_actions_param_table *param_table;
    char                *drop_str = "{preserve | green | yellow | red}";
    int                 width_col1 = 20, width_col2 = 35, width_col3= 20;

    param_table = sal_alloc(sizeof(_fp_list_actions_param_table) * bcmFieldActionCount, "param table workspace");
    if (!param_table) {
        cli_out("unable to allocated required memory\n");
        return CMD_FAIL;
    }
    sal_memset(param_table,
               0x00,
               sizeof(_fp_list_actions_param_table) * bcmFieldActionCount);

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
    param_table[bcmFieldActionEgressMask][0]          = "Dest. port bitmap mask";
    param_table[bcmFieldActionEgressPortsAdd][0]      = "Dest. port bitmap";
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
    param_table[bcmFieldActionGpTosPrecedenceNew][0]  = "New tos precedence";
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

    cli_out("%-*s%-*s%-*s\n", width_col1, "ACTION", width_col2, "PARAM0",
            width_col3, "PARAM1");
    for (action = 0; action < bcmFieldActionCount; action++) {
        if (action == bcmFieldActionUpdateCounter ||
            action == bcmFieldActionMeterConfig) {
            continue;
        }
        cli_out("%-*s%-*s%-*s\n",
                width_col1, format_field_action(buf, action, 1),
                width_col2, param_table[action][0],
                width_col3, param_table[action][1]);
    }
    action = bcmFieldActionMeterConfig;
    cli_out("%-*s%-*s%-*s\n", width_col1, format_field_action(buf, action, 1),
            width_col2, "flow", width_col3, "{peak | committed}");
    cli_out("%-*s%-*s%-*s\n", width_col1, format_field_action(buf, action, 1),
            width_col2, "{trTCM_blind | trTCM_aware | ", width_col3, "n/a");
    cli_out("%-*s%-*s%-*s\n", width_col1, "",
            width_col2, " srTCM_blind | srTCM_aware |", width_col3, "n/a");
    cli_out("%-*s%-*s%-*s\n", width_col1, "",
            width_col2, " default}", width_col3, "n/a");

    action = bcmFieldActionUpdateCounter;
    cli_out("%-*s%-*s%-*s\n", width_col1, format_field_action(buf, action, 1),
            width_col2, "{lower | upper | ",
            width_col3, "[{bytes|packets}]");
    cli_out("%-*s%-*s%-*s\n", width_col1, "",
            width_col2, " red_notred | green_notgreen |",
            width_col3, "[{bytes|packets}]");
    cli_out("%-*s%-*s%-*s\n", width_col1, "",
            width_col2, " green_red | green_yellow |",
            width_col3, "[{bytes|packets}]");
    cli_out("%-*s%-*s%-*s\n", width_col1, "",
            width_col2, " red_yellow |",
            width_col3, "[{bytes|packets}]");
    cli_out("%-*s%-*s%-*s\n", width_col1, "",
            width_col2, " bytes_packets}",
            width_col3, "n/a");

    sal_free(param_table);
    return CMD_OK;
}

/*
 *  Execute 'data' related qualifier stuff
 */
static int
fp_qual_data(int unit, bcm_field_entry_t eid, args_t *args)
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
    FP_VRFY_GET_NUMB(qualid,     str, args);

    /* Get match data. */
    if ((str= ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    rv = fp_parse_hex_string(str, sizeof(_bcm_field_qual_data_t),
                             (uint8 *)data, &data_length);
    if (rv < 0) {
        cli_out("Qualifier data parse error.\n");
        return CMD_USAGE;
    }

    /* Get match mask. */
    if ((str= ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    rv = fp_parse_hex_string(str, sizeof(_bcm_field_qual_data_t),
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
 *  Qualify based upon ingress (In) ports map
 */
static int
fp_qual_InPorts(int unit, bcm_field_entry_t eid, args_t *args)
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
 *  Qualify based upon egress (Out) ports map
 */
static int
fp_qual_OutPorts(int unit, bcm_field_entry_t eid, args_t *args)
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
 *  Qualify based upon port
 */
static int
fp_qual_port(int unit,
             bcm_field_entry_t eid,
             args_t *args,
             int func(int, bcm_field_entry_t, bcm_port_t, bcm_port_t),
             char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_port_t                  data, mask;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);

    FP_GET_PORT(unit, data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon ModPort
 */
static int
fp_qual_modport(int unit,
                bcm_field_entry_t eid,
                args_t *args,
                int func(int, bcm_field_entry_t, bcm_module_t, bcm_module_t, bcm_port_t, bcm_port_t),
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
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon aggregate ('Trunk') ID
 */
static int
fp_qual_trunk(int unit,
              bcm_field_entry_t eid,
              args_t *args,
              int func(int, bcm_field_entry_t, bcm_trunk_t, bcm_trunk_t),
              char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_port_t                  data, mask;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon L4 port
 */
static int
fp_qual_l4port(int unit,
               bcm_field_entry_t eid,
               args_t *args,
               int func(int, bcm_field_entry_t, bcm_l4_port_t, bcm_l4_port_t),
               char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_l4_port_t               data, mask;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);
    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon GPORT
 */
static int
fp_qual_Gport(int unit,
              bcm_field_entry_t eid,
              args_t *args,
              int func(int, bcm_field_entry_t, int),
              char *qual_str)
{
    int   retval = CMD_OK;
    char  *subcmd = NULL;
    uint32   data;
    char  str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data'  */
    retval = func(unit, eid, data);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon VID (VLAN?)
 */
static int
fp_qual_vlan(int unit,
             bcm_field_entry_t eid,
             args_t *args,
             int func(int, bcm_field_entry_t, bcm_vlan_t, bcm_vlan_t),
             char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    bcm_vlan_t                  data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon TPID value
 */
static int
fp_qual_tpid(int unit,
             bcm_field_entry_t eid,
             args_t *args,
             int func(int, bcm_field_entry_t, uint16),
             char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint16                      data;
    char                        str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' */
    retval = func(unit, eid, data);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon IPv4 address
 */
static int
fp_qual_ip(int unit,
           bcm_field_entry_t eid,
           args_t *args,
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
                   unit,
                   subcmd));
        return CMD_FAIL;
    }

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(stat_str, "bcm_field_qualify_");
    strncat(stat_str, qual_str, FP_STAT_STR_SZ - 1 - strlen(stat_str));
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}

/*
 *  Qualify based upon IPv6 address
 */
static int
fp_qual_ip6(int unit,
            bcm_field_entry_t eid,
            args_t *args,
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

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon MAC address
 */
static int
fp_qual_mac(int unit,
            bcm_field_entry_t eid,
            args_t *args,
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
    strcpy(stat_str, "bcm_field_qualify_");
    strncat(stat_str, qual_str, FP_STAT_STR_SZ - 1 - strlen(stat_str));
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}

/*
 *  Qualify based upon colour
 */
static int
fp_qual_Color(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    int                 data = -1;
    parse_table_t       pt;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "color", PQ_DFL | PQ_MULTI, 0,
                    (void *)&data, color_text);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* BCM.0> fp qual <eid> color [color=<>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: bcm_field_qualify_(entry=%d, data=%#x)\n"),
                 unit, eid, data));
    switch (data) {
      case 0:
          data = BCM_FIELD_COLOR_GREEN;
          break;
      case 1:
          data = BCM_FIELD_COLOR_YELLOW;
          break;
      case 2:
          data = BCM_FIELD_COLOR_RED;
          break;
      default:
          return CMD_FAIL;

    }
    retval = bcm_field_qualify_Color(unit, eid, data);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_Color");

    return CMD_OK;
}

/*
 *  Qualify based upon IpInfo information
 */
static int
fp_qual_IpInfo(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    uint32              data = 0, mask = 0;
    parse_table_t       pt;
    int                 hdr_offset_zero= 0, hdr_flags_mf= 0, checksum_ok = 0;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "HeaderOffsetZero", PQ_DFL | PQ_BOOL, 0,
                    (void *)&hdr_offset_zero, 0);
    parse_table_add(&pt, "HeaderFlagsMF", PQ_DFL | PQ_BOOL, 0,
                    (void *)&hdr_flags_mf, 0);
    parse_table_add(&pt, "ChecksumOK", PQ_DFL | PQ_BOOL, 0,
                    (void *)&checksum_ok, 0);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* Set the flags in the data and mask based on inputs. */
    if (pt.pt_entries[0].pq_type & PQ_PARSED) {
        if (hdr_offset_zero) {
            data |= BCM_FIELD_IP_HDR_OFFSET_ZERO;
        } else {
            data &= ~BCM_FIELD_IP_HDR_OFFSET_ZERO;
        }
        mask |= BCM_FIELD_IP_HDR_OFFSET_ZERO;
    }

    if (pt.pt_entries[1].pq_type & PQ_PARSED) {
        if (hdr_flags_mf) {
            data |= BCM_FIELD_IP_HDR_FLAGS_MF;
        } else {
            data &= ~BCM_FIELD_IP_HDR_FLAGS_MF;
        }
        mask |= BCM_FIELD_IP_HDR_FLAGS_MF;
    }

    if (pt.pt_entries[2].pq_type & PQ_PARSED) {
        if (checksum_ok) {
            data |= BCM_FIELD_IP_CHECKSUM_OK;
        } else {
            data &= ~BCM_FIELD_IP_CHECKSUM_OK;
        }
        mask |= BCM_FIELD_IP_CHECKSUM_OK;
    }

    /* BCM.0> fp qual <eid> IpInfo [HOZ=<0/1> HFMF=<0/1> COK=<0/1>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_qualify_IpInfo(entry=%d, data=%#x, mask=%#x)\n"),
                 unit, eid, data, mask));
    retval = bcm_field_qualify_IpInfo(unit, eid, data, mask);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_IpInfo");

    return CMD_OK;
}

/*
 *  Qualify based upon PacketRes information
 */
static int
fp_qual_PacketRes(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    uint32              data = 0, mask = 0xf;
    parse_table_t       pt;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "Res", PQ_DFL | PQ_MULTI, 0,
                    (void *)&data, packet_res_text);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* BCM.0> fp qual <eid> PacketRes [Res=<>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_qualify_PacketRes(entry=%d, data=%#x, mask=%#x)\n"),
                 unit, eid, data, mask));
    retval = bcm_field_qualify_PacketRes(unit, eid, data, mask);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_PacketRes");

    return CMD_OK;
}

/*
 *  Qualify based upon IpFrag information
 */
static int
fp_qual_IpFrag(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    int                 data = -1;
    parse_table_t       pt;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "frag", PQ_DFL | PQ_MULTI, 0,
                    (void *)&data, ipfrag_text);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* BCM.0> fp qual <eid> IpFrag [Frag=<>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_qualify_IpFrag(entry=%d, data=%#x)\n"),
                 unit, eid, data));
    retval = bcm_field_qualify_IpFrag(unit, eid, data);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_IpFrag");

    return CMD_OK;
}

/*
 *  Qualify based upon Source Lookup Class
 */
static int
fp_qual_SrcLookupClass(int unit,
                       bcm_field_entry_t eid,
                       args_t *args,
                       int func(int, bcm_field_entry_t, uint32, uint32),
                       char *qual_str)
{
    int   retval = CMD_OK;
    char  *subcmd = NULL;
    uint32   data, mask;
    char  str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon Destination Lookup Class
 */
static int
fp_qual_DstLookupClass(int unit,
                       bcm_field_entry_t eid,
                       args_t *args,
                       int func(int, bcm_field_entry_t, uint32, uint32),
                       char *qual_str)
{
    int   retval = CMD_OK;
    char  *subcmd = NULL;
    uint32   data, mask;
    char  str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/* Qualify based upon IP type */
static int
fp_qual_IpType(int unit, bcm_field_entry_t eid, args_t *args)
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
        type = fp_lookup_IpType(subcmd);
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
 *  Qualify based upon on L2Format
 */
static int
fp_qual_L2Format(int unit, bcm_field_entry_t eid, args_t *args)
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
        type = fp_lookup_L2Format(subcmd);
        if (type == bcmFieldL2FormatCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown L2Format value: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    /* BCM.0> fp qual 'eid' L2Format 'type' */
    retval = bcm_field_qualify_L2Format(unit, eid, type);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_L2Format");

    return CMD_OK;
}

/*
 *  Qualify based upon Decap information
 */
static int
fp_qual_Decap(int unit, bcm_field_entry_t eid, args_t *args)
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

/*
 *  Qualify based upon LoopbackType
 */
static int
fp_qual_LoopbackType(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    int                 data = -1;
    parse_table_t       pt;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "lb_type", PQ_DFL | PQ_MULTI, 0,
                    (void *)&data, loopbacktype_text);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* BCM.0> fp qual <eid> LoopbackType [lb_type =<>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: bcm_field_qualify_LoopbackType(entry=%d, data=%#x)\n"),
                 unit, eid, data));
    retval = bcm_field_qualify_LoopbackType(unit, eid, data);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_LoopbackType");

    return CMD_OK;
}

/*
 *  Qualify based upon TunnelType
 */
static int
fp_qual_TunnelType(int unit, bcm_field_entry_t eid, args_t *args)
{
    int                 retval = CMD_OK;
    int                 data = -1;
    parse_table_t       pt;

    parse_table_init(unit, &pt);
    parse_table_add(&pt, "tnl_type", PQ_DFL | PQ_MULTI, 0,
                    (void *)&data, tunneltype_text);

    if (BCM_FAILURE(parse_arg_eq(args, &pt))) {
        parse_arg_eq_done(&pt);
        return CMD_FAIL;
    }

    /* BCM.0> fp qual <eid> TunnelType [lb_type =<>] */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: bcm_field_qualify_TunnelType(entry=%d, data=%#x)\n"),
                 unit, eid, data));
    retval = bcm_field_qualify_TunnelType(unit, eid, data);
    FP_CHECK_RETURN(unit, retval, "bcm_field_qualify_TunnelType");

    return CMD_OK;
}

/*
 * Function:
 *     fp_qual_InterfaceClass
 * Purpose:
 *     Qualify on Interface class
 * Parmameters:
 * Returns:
 */
static int
fp_qual_InterfaceClass(int unit,
                       bcm_field_entry_t eid,
                       args_t *args,
                       int func(int, bcm_field_entry_t, uint32, uint32),
                       char *qual_str)
{
    int   retval = CMD_OK;
    char  *subcmd = NULL;
    uint32   data, mask;
    char  str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon a range
 */
static int
fp_qual_rangecheck(int unit, bcm_field_entry_t eid, args_t *args,
                   int func(int, bcm_field_entry_t, bcm_field_range_t, int),
                   char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    int                         range, result;
    char                        str[FP_STAT_STR_SZ];

    sal_memset(str, 0, sizeof(char) * FP_STAT_STR_SZ);
    FP_VRFY_GET_NUMB(range, subcmd, args);
    FP_VRFY_GET_NUMB(result, subcmd, args);

    /* BCM.0> fp qual 'eid' RangeCheck 'range' 'result' */
    retval = func(unit, eid, range, result);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon some 8b (or fits in 8b) value
 */
static int
fp_qual_8(int unit,
          bcm_field_entry_t eid,
          args_t *args,
          int func(int, bcm_field_entry_t, uint8, uint8),
          char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint8                       data, mask;
    char                        stat_str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(stat_str, "bcm_field_qualify_");
    strncat(stat_str, qual_str, FP_STAT_STR_SZ - 1 - strlen(stat_str));
    FP_CHECK_RETURN(unit, retval, stat_str);

    return CMD_OK;
}

/*
 *  Qualify based upon some 16b (or fits in 16b but not 8b) value
 */
static int
fp_qual_16(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint16, uint16),
           char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint16                      data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Qualify based upon some 32b (or fits in 32b but not 16b) value
 */
static int
fp_qual_32(int unit, bcm_field_entry_t eid, args_t *args,
           int func(int, bcm_field_entry_t, uint32, uint32),
           char *qual_str)
{
    int                         retval = CMD_OK;
    char                        *subcmd = NULL;
    uint32                      data, mask;
    char                        str[FP_STAT_STR_SZ];

    FP_VRFY_GET_NUMB(data, subcmd, args);
    FP_VRFY_GET_NUMB(mask, subcmd, args);

    /* BCM.0> fp qual 'eid' Qual 'data' 'mask' */
    retval = func(unit, eid, data, mask);
    strcpy(str, "bcm_field_qualify_");
    strncat(str, qual_str, FP_STAT_STR_SZ - 1 - strlen(str));
    FP_CHECK_RETURN(unit, retval, str);

    return CMD_OK;
}

/*
 *  Handle fp qualify command
 */
static int
fp_qual(int unit, args_t *args)
{
    char                  *subcmd   = NULL;
    char                  *qual_str = NULL;
    bcm_field_entry_t     eid;
    int                   rv = CMD_OK;

    FP_VRFY_GET_NUMB(eid, subcmd, args);
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
                     unit,
                     eid,
                     qual_str));
        rv = bcm_field_qualifier_delete(unit,
                                        eid,
                                        parse_field_qualifier(qual_str));
        FP_CHECK_RETURN(unit, rv, "bcm_field_qualifier_delete");

        return rv;
    }

    /*
     * Argument exception: "Data" does not correspond directly to
     * any bcmFieldQualify* enum.
     */
    if(!sal_strcasecmp(qual_str, "Data")) {
        return fp_qual_data(unit, eid, args);
    }

    subcmd = ARG_GET(args);
    if (subcmd != NULL) {
        if ((!sal_strcasecmp(subcmd, "show")) ||
            (!sal_strcasecmp(subcmd, "?"))) {
            switch (parse_field_qualifier(qual_str)) {
                case bcmFieldQualifyIpInfo:
                    return fp_qual_IpInfo_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyPacketRes:
                    return fp_qual_PacketRes_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyIpFrag:
                    return fp_qual_IpFrag_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyLoopbackType:
                    return fp_qual_LoopbackType_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifyTunnelType:
                    return fp_qual_TunnelType_help("fp qual <eid> ", 0);
                    break;
                case bcmFieldQualifySrcPort:
                    return fp_qual_modport_help("fp qual <eid> ", "SrcPort", 0);
                    break;
                case bcmFieldQualifyDstPort:
                    return fp_qual_modport_help("fp qual <eid> ", "DstPort", 0);
                    break;
                case bcmFieldQualifySrcMac:
                    return fp_qual_mac_help("fp qual <eid> ", "SrcMac", 0);
                    break;
                case bcmFieldQualifyDstMac:
                    return fp_qual_mac_help("fp qual <eid> ", "DstMac", 0);
                    break;
                default:
                    return fp_list_quals(unit);
            }
        } else {
            ARG_PREV(args);
        }
    }

    /* > fp qual 'eid' bcmFieldQualifyXXX ...*/
    switch (parse_field_qualifier(qual_str)) {
    case bcmFieldQualifyInPort:
        rv = fp_qual_port(unit, eid, args, bcm_field_qualify_InPort, "InPort");
        break;
    case bcmFieldQualifyOutPort:
        rv = fp_qual_port(unit, eid, args, bcm_field_qualify_OutPort, "OutPort");
        break;
    case bcmFieldQualifyInPorts:
        rv = fp_qual_InPorts(unit, eid, args);
        break;
    case bcmFieldQualifyOutPorts:
        rv = fp_qual_OutPorts(unit, eid, args);
        break;
    case bcmFieldQualifyDrop:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_Drop, "Drop");
        break;
    case bcmFieldQualifyLoopback:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_Loopback, "Loopback");
        break;
    case bcmFieldQualifySrcPort:
        rv = fp_qual_modport(unit, eid, args, bcm_field_qualify_SrcPort, "SrcPort");
        break;
    case bcmFieldQualifySrcTrunk:
        rv = fp_qual_trunk(unit, eid, args, bcm_field_qualify_SrcTrunk, "SrcTrunk");
        break;
    case bcmFieldQualifyDstPort:
        rv = fp_qual_modport(unit, eid, args, bcm_field_qualify_DstPort, "DstPort");
        break;
    case bcmFieldQualifyDstTrunk:
        rv = fp_qual_trunk(unit, eid, args, bcm_field_qualify_DstTrunk, "DstTrunk");
        break;
    case bcmFieldQualifyL4SrcPort:
        rv = fp_qual_l4port(unit, eid, args, bcm_field_qualify_L4SrcPort, "L4SrcPort");
        break;
    case bcmFieldQualifyL4DstPort:
        rv = fp_qual_l4port(unit, eid, args, bcm_field_qualify_L4DstPort, "L4DstPort");
        break;
    case bcmFieldQualifyOuterVlan:
        rv = fp_qual_vlan(unit, eid, args, bcm_field_qualify_OuterVlan, "OuterVlan");
        break;
    case bcmFieldQualifyOuterVlanId:
        rv = fp_qual_16(unit, eid, args, bcm_field_qualify_OuterVlanId, "OuterVlanId");
        break;
    case bcmFieldQualifyOuterVlanPri:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_OuterVlanPri, "OuterVlanPri");
        break;
    case bcmFieldQualifyOuterVlanCfi:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_OuterVlanCfi, "OuterVlanCfi");
        break;
    case bcmFieldQualifyInnerVlan:
        rv = fp_qual_vlan(unit, eid, args, bcm_field_qualify_InnerVlan, "InnerVlan");
        break;
    case bcmFieldQualifyInnerVlanId:
        rv = fp_qual_16(unit, eid, args, bcm_field_qualify_InnerVlanId, "InnerVlanId");
        break;
    case bcmFieldQualifyInnerVlanPri:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_InnerVlanPri, "InnerVlanPri");
        break;
    case bcmFieldQualifyInnerVlanCfi:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_InnerVlanCfi, "InnerVlanCfi");
        break;
    case bcmFieldQualifyEtherType:
        rv = fp_qual_16(unit, eid, args, bcm_field_qualify_EtherType, "EtherType");
       break;
    case bcmFieldQualifyOuterTpid:
        rv = fp_qual_tpid(unit, eid, args, bcm_field_qualify_OuterTpid, "OuterTpid");
       break;
    case bcmFieldQualifyInnerTpid:
        rv = fp_qual_tpid(unit, eid, args, bcm_field_qualify_InnerTpid, "InnerTpid");
       break;
     case bcmFieldQualifyVrf:
        rv = fp_qual_32(unit, eid, args, bcm_field_qualify_Vrf, "Vrf");
        break;
    case bcmFieldQualifyL4Ports:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L4Ports, "L4Ports");
        break;
    case bcmFieldQualifyMirrorCopy:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_MirrorCopy, "MirrorCopy");
        break;
    case bcmFieldQualifyTunnelTerminated:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_TunnelTerminated, "TunnelTerminated");
        break;
    case bcmFieldQualifyMplsTerminated:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_MplsTerminated, "TunnelTerminated");
        break;
    case bcmFieldQualifyIpProtocol:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_IpProtocol, "IpProtocol");
        break;
    case bcmFieldQualifyIpInfo:
        rv = fp_qual_IpInfo(unit, eid, args);
        break;
    case bcmFieldQualifyPacketRes:
        rv = fp_qual_PacketRes(unit, eid, args);
        break;
    case bcmFieldQualifyIpFrag:
        rv = fp_qual_IpFrag(unit, eid, args);
        break;
    case bcmFieldQualifySrcIp:
        rv = fp_qual_ip(unit, eid, args, bcm_field_qualify_SrcIp, "SrcIp");
        break;
    case bcmFieldQualifyDstIp:
        rv = fp_qual_ip(unit, eid, args, bcm_field_qualify_DstIp, "DstIp");
        break;
    case bcmFieldQualifyDSCP:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_DSCP, "DSCP");
        break;
    case bcmFieldQualifyVlanFormat:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_VlanFormat, "VlanFormat");
        break;
    case bcmFieldQualifyTranslatedVlanFormat:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_TranslatedVlanFormat, "TranslatedVlanFormat");
        break;
    case bcmFieldQualifyIntPriority:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_IntPriority, "IntPriority");
        break;
    case bcmFieldQualifyColor:
        rv = fp_qual_Color(unit, eid, args);
        break;
    case bcmFieldQualifyIpFlags:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_IpFlags, "IpFlags");
        break;
    case bcmFieldQualifyTcpHeaderSize:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_TcpHeaderSize, "TcpHeaderSize");
        break;
    case bcmFieldQualifyExtensionHeaderType:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_ExtensionHeaderType, "ExtensionHeaderType");
        break;
    case bcmFieldQualifyExtensionHeaderSubCode:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_ExtensionHeaderSubCode, "ExtensionHeaderSubCode");
        break;
    case bcmFieldQualifyL3Routable:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L3Routable, "L3Routable");
        break;
    case bcmFieldQualifyDosAttack:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_DosAttack, "DosAttack");
        break;
    case bcmFieldQualifyIpmcStarGroupHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_IpmcStarGroupHit, "IpmcStarGroupHit");
        break;
    case bcmFieldQualifyL3DestRouteHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L3DestRouteHit, "L3DestRouteHit");
        break;
    case bcmFieldQualifyL3DestHostHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L3DestHostHit, "L3DestHostHit");
        break;
    case bcmFieldQualifyL3SrcHostHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L3SrcHostHit, "L3SrcHostHit");
        break;
    case bcmFieldQualifyL2DestHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L2DestHit, "L2DestHit");
        break;
    case bcmFieldQualifyL2SrcHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L2SrcHit, "L2SrcHit");
        break;
    case bcmFieldQualifyL2StationMove:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L2StationMove, "L2StationMove");
        break;
    case bcmFieldQualifyL2CacheHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L2CacheHit, "L2CacheHit");
        break;
    case bcmFieldQualifyL2SrcStatic:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_L2SrcStatic, "L2SrcStatic");
        break;
    case bcmFieldQualifyVlanTranslationHit:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_VlanTranslationHit, "VlanTranslationHit");
        break;
    case bcmFieldQualifyIngressStpState:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_IngressStpState, "IngressStpState");
        break;
    case bcmFieldQualifyForwardingVlanValid:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_ForwardingVlanValid, "ForwardingVlanValid");
        break;
    case bcmFieldQualifyTcpControl:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_TcpControl, "TcpControl");
        break;
    case bcmFieldQualifyTtl:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_Ttl, "TTL");
        break;
    case bcmFieldQualifyRangeCheck:
        rv = fp_qual_rangecheck(unit, eid, args, bcm_field_qualify_RangeCheck, "RangeCheck");
        break;
    case bcmFieldQualifySrcIp6:
        rv = fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6, "SrcIp6");
        break;
    case bcmFieldQualifySrcIp6High:
        rv = fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6High, "SrcIp6High");
        break;
    case bcmFieldQualifySrcIp6Low:
        rv = fp_qual_ip6(unit, eid, args, bcm_field_qualify_SrcIp6Low, "SrcIp6Low");
        break;
    case bcmFieldQualifyDstIp6:
        rv = fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6, "DstIp6");
        break;
    case bcmFieldQualifyDstIp6High:
        rv = fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6High, "DstIp6High");
        break;
    case bcmFieldQualifyDstIp6Low:
        rv = fp_qual_ip6(unit, eid, args, bcm_field_qualify_DstIp6Low, "DstIp6Low");
        break;
    case bcmFieldQualifyIp6FlowLabel:
        rv = fp_qual_32(unit, eid, args, bcm_field_qualify_Ip6FlowLabel, "Ip6FlowLabel");
        break;
    case bcmFieldQualifySrcMac:
        rv = fp_qual_mac(unit, eid, args, bcm_field_qualify_SrcMac, "SrcMac");
        break;
    case bcmFieldQualifyDstMac:
        rv = fp_qual_mac(unit, eid, args, bcm_field_qualify_DstMac, "DstMac");
        break;
    case bcmFieldQualifySrcClassL2:
        rv = fp_qual_SrcLookupClass(unit, eid, args, bcm_field_qualify_SrcClassL2, "SrcClassL2");
        break;
    case bcmFieldQualifySrcClassL3:
        rv = fp_qual_SrcLookupClass(unit, eid, args, bcm_field_qualify_SrcClassL3, "SrcClassL3");
        break;
    case bcmFieldQualifySrcClassField:
        rv = fp_qual_SrcLookupClass(unit, eid, args, bcm_field_qualify_SrcClassField, "SrcClassField");
        break;
    case bcmFieldQualifyDstClassL2:
        rv = fp_qual_DstLookupClass(unit, eid, args, bcm_field_qualify_DstClassL2, "DstClassL2");
        break;
    case bcmFieldQualifyDstClassL3:
        rv = fp_qual_DstLookupClass(unit, eid, args, bcm_field_qualify_DstClassL3, "DstClassL3");
        break;
    case bcmFieldQualifyDstClassField:
        rv = fp_qual_DstLookupClass(unit, eid, args, bcm_field_qualify_DstClassField, "DstClassField");
        break;
    case bcmFieldQualifyIpType:
        rv = fp_qual_IpType(unit, eid, args);
        break;
    case bcmFieldQualifyL2Format:
        rv = fp_qual_L2Format(unit, eid, args);
        break;
    case bcmFieldQualifyMHOpcode:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_MHOpcode,
                  "MHOpcode");
        break;
    case bcmFieldQualifyDecap:
        rv = fp_qual_Decap(unit, eid, args);
        break;
    case bcmFieldQualifyHiGig:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_HiGig, "HiGig");
        break;
    case bcmFieldQualifyDstHiGig:
        rv = fp_qual_8(unit, eid, args, bcm_field_qualify_DstHiGig, "DstHiGig");
        break;
    case bcmFieldQualifySrcMplsGport:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_SrcMplsGport, "SrcMplsGport");
        break;
    case bcmFieldQualifyDstMplsGport:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_DstMplsGport, "DstMplsGport");
        break;
    case bcmFieldQualifySrcMimGport:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_SrcMimGport, "SrcMimGport");
        break;
    case bcmFieldQualifyDstMimGport:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_DstMimGport, "DstMimGport");
        break;
    case bcmFieldQualifySrcWlanGport:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_SrcWlanGport, "SrcWlanGport");
        break;
    case bcmFieldQualifyDstWlanGport:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_DstWlanGport, "DstWlanGport");
        break;
    case bcmFieldQualifyDstL3Egress:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_DstL3Egress, "DstL3Egress");
        break;
    case bcmFieldQualifyDstMulticastGroup:
        rv = fp_qual_Gport(unit, eid, args, bcm_field_qualify_DstMulticastGroup, "DstMulticastGroup");
        break;
    case bcmFieldQualifyLoopbackType:
        rv = fp_qual_LoopbackType(unit, eid, args);
        break;
    case bcmFieldQualifyTunnelType:
        rv = fp_qual_TunnelType(unit, eid, args);
        break;
    case bcmFieldQualifyInterfaceClassL2:
        rv = fp_qual_InterfaceClass(unit, eid, args, bcm_field_qualify_InterfaceClassL2, "InterfaceClassL2");
        break;
    case bcmFieldQualifyInterfaceClassL3:
        rv = fp_qual_InterfaceClass(unit, eid, args, bcm_field_qualify_InterfaceClassL3, "InterfaceClassL3");
        break;
    case bcmFieldQualifyInterfaceClassPort:
        rv = fp_qual_InterfaceClass(unit, eid, args, bcm_field_qualify_InterfaceClassPort, "InterfaceClassPort");
        break;
    case bcmFieldQualifyCount:
    default:
        fp_list_quals(unit);
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
 *  Add a qualifier to a QSET
 */
static int
fp_qset_add(int unit, args_t *args, bcm_field_qset_t *qset)
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

    while (NULL != qual_str) {
        /*
         * Argument exception: "Data " does not correspond directly to
         * any bcmFieldQualify* enum.
         */
        if(!sal_strcasecmp(qual_str, "Data")) {
            FP_VRFY_GET_NUMB(dq_id, subcmd, args);
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

            if (qual >= bcmFieldQualifyCount) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: Unknown qualifier: %d\n"),
                           unit, qual));
                return CMD_FAIL;
            }
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
        cli_out("BCM_FIELD_QSET_ADD(%s) okay\n",
                format_field_qualifier(buf, qual, 1));
        qual_str = ARG_GET(args);
    }

    return CMD_OK;
}

/*
 *  Set a QSET explicitly
 */
static int
fp_qset_set(int unit, args_t *args, bcm_field_qset_t *qset)
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
    cli_out("fp_qset_set(%s) okay\n", format_field_qset(buf, *qset, " "));
    sal_free(buf);
    return CMD_OK;
}

/*
 * Function:
 *     _field_qset_dump
 * Purpose:
 *     Output qualiers set in 'qset'.
 */
static void
_field_qset_dump(char *prefix, bcm_field_qset_t qset, char *suffix)
{
    int                 qual;
    int                 idx;
    int first_qual = 1, first_udf_id = 1;

    if (prefix == NULL) {
        prefix = "";
    }
    if (suffix == NULL) {
        suffix = "";
    }

    cli_out("%s{", prefix);
    for (qual = 0; qual < _bcmFieldQualifyCount; qual++) {
        if (BCM_FIELD_QSET_TEST(qset, qual)) {
            cli_out("%s%s", (first_qual ? "" : ", "),
                     _field_qual_name(qual));
            first_qual = 0;
        }
    }

    for (idx = 0; idx < BCM_FIELD_USER_NUM_UDFS; idx++) {
        if (!SHR_BITGET(qset.udf_map, idx)) {
            continue;
        }
        cli_out("%s%d", (first_udf_id ? " : udf_id={" : ", "), idx);
        first_udf_id = 0;
    }
    if (first_udf_id == 0) {
        cli_out("}");
    }

    cli_out("}%s", suffix);
}

/*
 *  Dump a QSET as qualifiers by string
 */
static int
fp_qset_show(bcm_field_qset_t *qset)
{
    _field_qset_dump("qset=", *qset, "\n");
    return CMD_OK;
}

/*
 *  Handle fp qualifier set command
 */
static int
fp_qset(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char*                 subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "clear")) {
            /* BCM.0> fp qset clear */
            BCM_FIELD_QSET_INIT(*qset);
            result = CMD_OK;
        } else if(!sal_strcasecmp(subcmd, "add")) {
            /* BCM.0> fp qset add ...*/
            result = fp_qset_add(unit, args, qset);
        } else if(!sal_strcasecmp(subcmd, "set")) {
            /* BCM.0> fp qset set ...*/
            result = fp_qset_set(unit, args, qset);
        } else if(!sal_strcasecmp(subcmd, "show")) {
            /* BCM.0> fp qset show */
            result = fp_qset_show(qset);
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_qset_usage);
        result = CMD_NFND;
    }
    return result;
}

/*
 * Function:
 *     fp_action_ports_add
 * Purpose:
 *     FP action add which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_ports_add(int unit, args_t *args)
{
    char               *subcmd = NULL;
    bcm_field_entry_t   eid;
    bcm_field_action_t  action;
    int                 retval;
    bcm_pbmp_t          pbmp;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    char                buf_pbmp[SOC_PBMP_FMT_LEN];

    FP_VRFY_GET_NUMB(eid, subcmd, args);
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

    if ((action != bcmFieldActionRedirectPbmp) &&
        (action != bcmFieldActionEgressPortsAdd) &&
        (action != bcmFieldActionEgressMask)) {
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
                            "FP(unit %d) verb: action ports add eid=%d, action=%s, pbmp=%s\n"),
                 unit, eid, format_field_action(buf, action, 1),
                 format_pbmp(unit, buf_pbmp, sizeof(buf_pbmp), pbmp)));
    retval = bcm_field_action_ports_add(unit, eid, action, pbmp);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_ports_add");

    return CMD_OK;
}

/*
 * Function:
 *     fp_action_ports_get
 * Purpose:
 *     FP action get which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_ports_get(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;
    bcm_pbmp_t          pbmp;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    char                buf_pbmp[SOC_PBMP_FMT_LEN];

    FP_VRFY_GET_NUMB(eid, subcmd, args);
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

    if ((action != bcmFieldActionRedirectPbmp) &&
        (action != bcmFieldActionEgressPortsAdd) &&
        (action != bcmFieldActionEgressMask)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unrecognized action\n"),
                   unit));
        return CMD_FAIL;
    }

    retval = bcm_field_action_ports_get(unit, eid, action, &pbmp);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_ports_get");
    cli_out("FP action ports get: action=%s, pbmp=%s\n",
            format_field_action(buf, action, 1),
            SOC_PBMP_FMT(pbmp, buf_pbmp));

    return CMD_OK;
}

/*
 * Function:
 *     fp_action_ports
 * Purpose:
 *     FP action which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_ports(int unit, args_t *args)
{
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.0> fp action ports add ... */
    if(!sal_strcasecmp(subcmd, "add")) {
        return fp_action_ports_add(unit, args);
    }
    /* BCM.0> fp action ports get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return fp_action_ports_get(unit, args);
    }

    return CMD_USAGE;
}

/*
 * Function:
 *     fp_action_mac_add
 * Purpose:
 *     FP action add which takes bcm_mac_t as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_mac_add(int unit, args_t *args)
{
    char               *subcmd = NULL;
    bcm_field_entry_t   eid;
    bcm_field_action_t  action;
    int                 retval;
    bcm_mac_t           mac;

    FP_VRFY_GET_NUMB(eid, subcmd, args);
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

    if ((action != bcmFieldActionSrcMacNew) &&
        (action != bcmFieldActionDstMacNew)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unrecognized action\n"),
                   unit));
        return CMD_FAIL;
    }

    /* parse mac address. */
    FP_GET_MAC(mac, subcmd, args);

    retval = bcm_field_action_mac_add(unit, eid, action, mac);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_ports_add");

    return CMD_OK;
}

/*
 * Function:
 *     fp_action_mac_get
 * Purpose:
 *     FP action get which takes pbmp as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_mac_get(int unit, args_t *args)
{
    char*               subcmd = NULL;
    char                mac_str[SAL_MACADDR_STR_LEN];
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;
    bcm_mac_t           mac;

    FP_VRFY_GET_NUMB(eid, subcmd, args);
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

    if ((action != bcmFieldActionSrcMacNew) &&
        (action != bcmFieldActionDstMacNew)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP(unit %d) Error: Unrecognized action: %s\n"),
                   unit, subcmd));
        return CMD_FAIL;
    }

    retval = bcm_field_action_mac_get(unit, eid, action, &mac);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_mac_get");
    format_macaddr(mac_str, mac);
    cli_out("FP action mac get: action=%s, mac=%s\n",
            format_field_action(buf, action, 1), mac_str);
    return CMD_OK;
}

/*
 * Function:
 *     fp_action_mac
 * Purpose:
 *     FP action which takes bcm_mac_t as argument
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_mac(int unit, args_t *args)
{
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    /* BCM.0> fp action mac add ... */
    if(!sal_strcasecmp(subcmd, "add")) {
        return fp_action_mac_add(unit, args);
    }
    /* BCM.0> fp action ports get ... */
    if(!sal_strcasecmp(subcmd, "get")) {
        return fp_action_mac_get(unit, args);
    }
    return CMD_USAGE;
}

/*
 *  Decide on colour from string
 */
static int
fp_lookup_color(char *qual_str) {
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
static int
fp_lookup_counter_mode(args_t *args)
{
    char *subcmd = NULL;
    int mode = -1;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if (isint(subcmd)) {
        mode =  parse_integer(subcmd);
    } else if (!sal_strcasecmp(subcmd, "default")) {
        mode = BCM_FIELD_COUNTER_MODE_DEFAULT;
    } else if (!sal_strcasecmp(subcmd, "none")) {
        mode = BCM_FIELD_COUNTER_MODE_NO_NO;
    } else if (!sal_strcasecmp(subcmd, "lower")) {
        mode = BCM_FIELD_COUNTER_MODE_NO_YES;
    } else if (!sal_strcasecmp(subcmd, "upper")) {
        mode = BCM_FIELD_COUNTER_MODE_YES_NO;
    } else if (!sal_strcasecmp(subcmd, "red_notred")) {
        mode = BCM_FIELD_COUNTER_MODE_RED_NOTRED;
    } else if (!sal_strcasecmp(subcmd, "green_notgreen")) {
        mode = BCM_FIELD_COUNTER_MODE_GREEN_NOTGREEN;
    } else if (!sal_strcasecmp(subcmd, "green_red")) {
        mode = BCM_FIELD_COUNTER_MODE_GREEN_RED;
    } else if (!sal_strcasecmp(subcmd, "green_yellow")) {
        mode = BCM_FIELD_COUNTER_MODE_GREEN_YELLOW;
    } else if (!sal_strcasecmp(subcmd, "red_yellow")) {
        mode = BCM_FIELD_COUNTER_MODE_RED_YELLOW;
    } else if (!sal_strcasecmp(subcmd, "bytes_packets")) {
        mode = BCM_FIELD_COUNTER_MODE_BYTES_PACKETS;
    }

    if (mode >= 0) {
        if ((subcmd = ARG_GET(args)) != NULL) {
            if (!sal_strcasecmp(subcmd, "bytes")) {
                mode |= BCM_FIELD_COUNTER_MODE_BYTES;
            } else if (sal_strcasecmp(subcmd, "packets")) {
                ARG_PREV(args);
            }
        }
    }
    return mode;
}

/*
 *  Figure out meter mode from a string
 */
static int
fp_lookup_meter_mode(char *qual_str) {
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
 * Function: fp_lookup_meter
 * Purpose:
 *     Convert User string to the choice of either Peak or Committed meter
 * Parmameters:
 * Returns:
 */
static int
fp_lookup_meter(char *qual_str) {
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
 *     fp_action_add
 * Purpose:
 *     Add an action to an entry. Can take action either in the form
 *     of a string or digit corresponding to action order in
 *     bcm_field_action_t enum.
 * Parmameters:
 *     unit - BCM unit number
 *     args - command line arguments
 * Returns:
 */
static int
fp_action_add(int unit, args_t *args)
{
    char               *subcmd = NULL;
    bcm_field_entry_t   eid;
    bcm_field_action_t  action;
    int                 retval;
    int                 p0 = 0, p1 = 0;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];

    FP_VRFY_GET_NUMB(eid, subcmd, args);
    if ((subcmd  = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    if (isint(subcmd)) {
        action = parse_integer(subcmd);
    } else {
        action = parse_field_action(subcmd);
        if (action == bcmFieldActionCount) {
            fp_list_actions(unit);
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown action: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    /* Read the action parameters (p0 and p1).*/
    switch (action) {
        case bcmFieldActionOffloadRedirect:
        case bcmFieldActionRedirect:
        case bcmFieldActionMirrorIngress:
        case bcmFieldActionMirrorEgress:
            FP_VRFY_GET_NUMB(p0, subcmd, args);
            if ((subcmd  = ARG_GET(args)) == NULL) {
                return CMD_USAGE;
            }
            if (parse_bcm_port(unit, subcmd, &p1) < 0) {
                LOG_ERROR(BSL_LS_BCM_FP,
                          (BSL_META_U(unit,
                                      "FP(unit %d) Error: invalid port string: \"%s\"\n"),
                           unit, subcmd));
                return CMD_FAIL;
            }
            break;
        case bcmFieldActionRedirectPbmp:
        case bcmFieldActionEgressMask:
        case bcmFieldActionEgressPortsAdd:
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
                p0 = fp_lookup_color(subcmd);
            break;
        case bcmFieldActionTimeStampToCpu:
        case bcmFieldActionRpTimeStampToCpu:
        case bcmFieldActionYpTimeStampToCpu:
        case bcmFieldActionGpTimeStampToCpu:
        case bcmFieldActionCopyToCpu:
        case bcmFieldActionRpCopyToCpu:
        case bcmFieldActionYpCopyToCpu:
        case bcmFieldActionGpCopyToCpu:
            FP_VRFY_GET_NUMB(p0, subcmd, args);
            FP_VRFY_GET_NUMB(p1, subcmd, args);
            break;
        case bcmFieldActionTosNew:
        case bcmFieldActionDscpNew:
        case bcmFieldActionRpDscpNew:
        case bcmFieldActionYpDscpNew:
        case bcmFieldActionGpDscpNew:
        case bcmFieldActionGpTosPrecedenceNew:
        case bcmFieldActionEcnNew:
        case bcmFieldActionRpEcnNew:
        case bcmFieldActionYpEcnNew:
        case bcmFieldActionGpEcnNew:
        case bcmFieldActionRedirectTrunk:
        case bcmFieldActionL3ChangeVlan:
        case bcmFieldActionL3ChangeMacDa:
        case bcmFieldActionL3Switch:
        case bcmFieldActionMultipathHash:
        case bcmFieldActionAddClassTag:
        case bcmFieldActionCosQNew:
        case bcmFieldActionRpCosQNew:
        case bcmFieldActionYpCosQNew:
        case bcmFieldActionGpCosQNew:
        case bcmFieldActionCosQCpuNew:
        case bcmFieldActionVlanCosQNew:
        case bcmFieldActionRpVlanCosQNew:
        case bcmFieldActionYpVlanCosQNew:
        case bcmFieldActionGpVlanCosQNew:
        case bcmFieldActionPrioPktNew:
        case bcmFieldActionRpPrioPktNew:
        case bcmFieldActionYpPrioPktNew:
        case bcmFieldActionGpPrioPktNew:
        case bcmFieldActionPrioIntNew:
        case bcmFieldActionRpPrioIntNew:
        case bcmFieldActionYpPrioIntNew:
        case bcmFieldActionGpPrioIntNew:
        case bcmFieldActionPrioPktAndIntNew:
        case bcmFieldActionRpPrioPktAndIntNew:
        case bcmFieldActionYpPrioPktAndIntNew:
        case bcmFieldActionGpPrioPktAndIntNew:
        case bcmFieldActionRpOuterVlanCfiNew:
        case bcmFieldActionYpOuterVlanCfiNew:
        case bcmFieldActionGpOuterVlanCfiNew:
        case bcmFieldActionOuterVlanCfiNew:
        case bcmFieldActionRpInnerVlanCfiNew:
        case bcmFieldActionYpInnerVlanCfiNew:
        case bcmFieldActionGpInnerVlanCfiNew:
        case bcmFieldActionInnerVlanCfiNew:
        case bcmFieldActionRpOuterVlanPrioNew:
        case bcmFieldActionYpOuterVlanPrioNew:
        case bcmFieldActionGpOuterVlanPrioNew:
        case bcmFieldActionOuterVlanPrioNew:
        case bcmFieldActionRpInnerVlanPrioNew:
        case bcmFieldActionYpInnerVlanPrioNew:
        case bcmFieldActionGpInnerVlanPrioNew:
        case bcmFieldActionInnerVlanPrioNew:
        case bcmFieldActionOuterTpidNew:
        case bcmFieldActionOuterVlanNew:
        case bcmFieldActionInnerVlanNew:
        case bcmFieldActionOuterVlanLookup:
        case bcmFieldActionOuterVlanAdd:
        case bcmFieldActionInnerVlanAdd:
        case bcmFieldActionClassDestSet:
        case bcmFieldActionClassSourceSet:
        case bcmFieldActionVrfSet:
        case bcmFieldActionColorIndependent:
        case bcmFieldActionRedirectIpmc:
        case bcmFieldActionRedirectMcast:
        case bcmFieldActionIncomingMplsPortSet:
        case bcmFieldActionOamUpMep:
        case bcmFieldActionOamTx:
        case bcmFieldActionOamLmepMdl:
        case bcmFieldActionOamServicePriMappingPtr:
        case bcmFieldActionOamLmBasePtr:
        case bcmFieldActionOamDmEnable:
        case bcmFieldActionOamLmEnable:
        case bcmFieldActionOamLmepEnable:
        case bcmFieldActionOamPbbteLookupEnable:
            FP_VRFY_GET_NUMB(p0, subcmd, args);
            break;
        case bcmFieldActionUpdateCounter:
            p0 = fp_lookup_counter_mode(args);

            break;
        case bcmFieldActionMeterConfig:
            if ((subcmd  = ARG_GET(args)) == NULL) {
                return CMD_USAGE;
            }

            p0 = fp_lookup_meter_mode(subcmd);

            if (p0 == BCM_FIELD_METER_MODE_FLOW) {
                if ((subcmd  = ARG_GET(args)) == NULL) {
                    return CMD_USAGE;
                }
                p1 = fp_lookup_meter(subcmd);
            }
            break;
        case bcmFieldActionSrcMacNew:
        case bcmFieldActionDstMacNew:
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Please use \"fp action mac add\" "
                                  "command"), unit));
            return CMD_FAIL;

        default:
            /* Use defaults of p0 and p1 for other actions */
            break;
    }

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: action add eid=%d, action=%s, p0=0x%x, p1=0x%x\n"),
                 unit,
                 eid,
                 format_field_action(buf, action, 1),
                 p0,
                 p1));
    retval = bcm_field_action_add(unit, eid, action, p0, p1);
    FP_CHECK_RETURN(unit, retval, "bcm_field_action_add");

    return CMD_OK;
}

/*
 *  Get an action
 */
static int
fp_action_get(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;
    uint32              p0 = 0, p1 = 0;
    char                buf[BCM_FIELD_ACTION_WIDTH_MAX];

    FP_VRFY_GET_NUMB(eid, subcmd, args);
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
    cli_out("FP action get: action=%s, p0=%d, p1=%d\n",
            format_field_action(buf, action, 1), p0, p1);

    return CMD_OK;
}

/*
 *  Remove an action
 */
static int
fp_action_remove(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    bcm_field_action_t  action;


    FP_VRFY_GET_NUMB(eid, subcmd, args);

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
 *  Handle fp action command
 */
static int
fp_action(int unit, args_t *args)
{
    char*               subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "ports")) {
            /* BCM.0> fp action ports ... */
            result = fp_action_ports(unit, args);
        } else if(!sal_strcasecmp(subcmd, "mac")) {
            /* BCM.0> fp action mac ... */
            result = fp_action_mac(unit, args);
        } else if(!sal_strcasecmp(subcmd, "add")) {
            /* BCM.0> fp action add ... */
            result = fp_action_add(unit, args);
        } else if(!sal_strcasecmp(subcmd, "get")) {
            /* BCM.0> fp action get ... */
            result = fp_action_get(unit, args);
        } else if(!sal_strcasecmp(subcmd, "remove")) {
            /* BCM.0> fp action remove... */
            result = fp_action_remove(unit, args);
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_action_usage);
        result = CMD_NFND;
    }
    return result;
}

/*
 * Function:
 *    fp_lookup_control
 * Purpose:
 *    Lookup a control from a user string.
 * Parmameters:
 * Returns:
 */
static void
fp_lookup_control(const char *control_str, bcm_field_control_t *control)
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
        strncpy(tbl_str, _fp_control_name(*control), FP_STAT_STR_SZ - 1);
        if (!sal_strcasecmp(tbl_str, control_str)) {
            break;
        }
        /* Test for whole name of the control */
        sal_strcpy(lng_str, "bcmFieldControl");
        strncat(lng_str, tbl_str,
                FP_STAT_STR_SZ - 1 - sal_strlen("bcmFieldControl"));
        if (!sal_strcasecmp(lng_str, control_str)) {
            break;
        }
    }

    /* If not found, result is bcmFieldControlCount */
}

/*
 *  Handle fp control command
 */
static int
fp_control(int unit, args_t *args)
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
        fp_lookup_control(subcmd, &element);
        if (element == bcmFieldControlCount) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Unknown FP control: %s\n"),
                       unit, subcmd));
            return CMD_FAIL;
        }
    }

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp control <control_number>*/
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_control_get(element=%s)\n"),
                     unit, _fp_control_name(element)));
        retval = bcm_field_control_get(unit, element, &status);
        FP_CHECK_RETURN(unit, retval, "bcm_field_control_get");
        cli_out("FP element=%s: status=%d\n", _fp_control_name(element), status);
    } else {
        /* BCM.0> fp control <control_number> <status>*/
        if (element == bcmFieldControlStage && !isint(subcmd)) {
            status = fp_lookup_stage(subcmd);
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
                                "FP(unit %d) verb: bcm_field_control_set(element=%s, status=%d)\n"),
                     unit, _fp_control_name(element), status));
        retval = bcm_field_control_set(unit, element, status);
        FP_CHECK_RETURN(unit, retval, "bcm_field_control_set");
    }
    return CMD_OK;
}

/*
 *  Create a counter
 */
static int
fp_counter_create(int unit, args_t *args)
{
    FP_CHECK_RETURN(unit, BCM_E_UNAVAIL, "bcm_field_counter_create");

    return CMD_OK;
}

/*
 *  Set a counter
 */
static int
fp_counter_set(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 counter_num;
    int                 retval;
    uint64              val;

    FP_VRFY_GET_NUMB(eid, subcmd, args);
    FP_VRFY_GET_NUMB(counter_num, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }
    val = parse_uint64(subcmd);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _counter_set(eid=%d, counter_num=%d, val=%d)\n"),
                 unit, eid, counter_num, COMPILER_64_LO(val)));
    retval = BCM_E_UNAVAIL;
    FP_CHECK_RETURN(unit, retval, "bcm_field_counter_set");
    return CMD_OK;
}

/*
 *  Get a counter
 */
static int
fp_counter_get(int unit, args_t *args)
{
    char*               subcmd = NULL;
    bcm_field_entry_t   eid;
    int                 retval;
    int                 counter_num;
    uint64              val64;

    COMPILER_64_ZERO(val64);
    FP_VRFY_GET_NUMB(eid, subcmd, args);

    if ((subcmd = ARG_GET(args))) {
        counter_num = parse_integer(subcmd);
        retval = BCM_E_UNAVAIL;
        FP_CHECK_RETURN(unit, retval, "bcm_field_counter_get");
        cli_out("FP counter get: eid=%d, counter_num=%d val=0x%x%08x\n", eid,
                counter_num, COMPILER_64_HI(val64), COMPILER_64_LO(val64));
    } else {
        /* Get both countes in pair. */
        counter_num = 0;
        retval = BCM_E_UNAVAIL;
        FP_CHECK_RETURN(unit, retval, "bcm_field_counter_get");
        cli_out("FP counter get: eid=%d, counter_num=%d val=0x%x%08x\n", eid,
                counter_num, COMPILER_64_HI(val64), COMPILER_64_LO(val64));
        counter_num = 1;
        retval = BCM_E_UNAVAIL;
        FP_CHECK_RETURN(unit, retval, "bcm_field_counter_get");
        cli_out("FP counter get: eid=%d, counter_num=%d val=0x%x%08x\n", eid,
                counter_num, COMPILER_64_HI(val64), COMPILER_64_LO(val64));
    }

    return CMD_OK;
}

/*
 *  Share another entry's counter
 */
static int
fp_counter_share(int unit, args_t *args)
{

    FP_CHECK_RETURN(unit, BCM_E_UNAVAIL, "bcm_field_counter_share");

    return CMD_OK;
}

/*
 *  Destroy a counter
 */
static int
fp_counter_destroy(int unit, args_t *args)
{

    FP_CHECK_RETURN(unit, BCM_E_UNAVAIL, "bcm_field_counter_destroy");

    return CMD_OK;
}

/*
 *  Handle fp counter command
 */
static int
fp_counter(int unit, args_t *args)
{
    char*               subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp counter create ... */
            result = fp_counter_create(unit, args);
        } else if(!sal_strcasecmp(subcmd, "set")) {
            /* BCM.0> fp counter set ... */
            result = fp_counter_set(unit, args);
        } else if(!sal_strcasecmp(subcmd, "get")) {
            /* BCM.0> fp counter get ... */
            result = fp_counter_get(unit, args);
        } else if(!sal_strcasecmp(subcmd, "share")) {
            /* BCM.0> fp counter share ... */
            result = fp_counter_share(unit, args);
        } else  if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp counter destroy ... */
            result = fp_counter_destroy(unit, args);
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_counter_usage);
        result = CMD_NFND;
    }
    return result;
}

/*
 *  Handle fp install command
 */
static int
fp_entry_install(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    /* BCM.0> fp install 'eid' */
    FP_VRFY_GET_NUMB(eid, subcmd, args);

    retval = bcm_field_entry_install(unit, eid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_install");

    return CMD_OK;
}

/*
 *  Create an entry
 */
static int
fp_entry_create(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_entry_t           eid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

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
        if (isint(subcmd)) {
            eid = parse_integer(subcmd);
            LOG_VERBOSE(BSL_LS_BCM_FP,
                        (BSL_META_U(unit,
                                    "FP(unit %d) verb: _entry_create gid=%d, eid=%d\n"),
                         unit, gid, eid));
            retval = bcm_field_entry_create_id(unit, gid, eid);
            FP_CHECK_RETURN(unit, retval, "bcm_field_entry_create_id");
        } else {
            return CMD_USAGE;
        }
    }
    return CMD_OK;
}

/*
 *  Copy an entry
 */
static int
fp_entry_copy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           src_eid, dst_eid = -111;

    FP_VRFY_GET_NUMB(src_eid, subcmd, args);
    subcmd = ARG_GET(args);

    if (subcmd ) {
        /* BCM.0> fp entry copy 'src_eid' 'dst_eid'  */
        dst_eid = parse_integer(subcmd);
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:  bcm_field_entry_copy_id(src_eid=%d, dst_eid=%d)\n"),
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
    return CMD_OK;
}

/*
 *  Destroy an entry
 */
static int
fp_entry_destroy(int unit, args_t *args)
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

/*
 *  Reinstall an entry
 */
static int
fp_entry_reinstall(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_VRFY_GET_NUMB(eid, subcmd, args);

    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _entry_reinstall eid=%d\n"),
                 unit, eid));
    retval = bcm_field_entry_reinstall(unit, eid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_reinstall");
    return CMD_OK;
}

/*
 *  Remove an entry
 */
static int
fp_entry_remove(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_VRFY_GET_NUMB(eid, subcmd, args);

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
 *     fp_entry_prio
 * Purpose:
 *     CLI interface to bcm_field_entry_prio_get/set()
 * Parmameters:
 * Returns:
 *     CMD_OK
 */
static int
fp_entry_prio(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    int                         prio;
    char*                       subcmd = NULL;
    bcm_field_entry_t           eid;

    FP_VRFY_GET_NUMB(eid, subcmd, args);

    /* BCM.0> fp entry prio <eid> */
    if ((subcmd = ARG_GET(args)) == NULL) {
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb: bcm_field_entry_prio_get(eid=%d)\n"),
                     unit, eid));
        retval = bcm_field_entry_prio_get(unit, eid, &prio);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_prio_get");
        cli_out("FP entry=%d: prio=%d\n", eid, prio);
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
                                "FP(unit %d) verb: bcm_field_entry_prio_set(eid=%d, prio=%d)\n"),
                     unit, eid, prio));
        retval = bcm_field_entry_prio_set(unit, eid, prio);
        FP_CHECK_RETURN(unit, retval, "bcm_field_entry_prio_set");
    }

    return CMD_OK;
}

#ifdef BROADCOM_DEBUG
/*
 *  Show information about an entry
 */
static int
fp_entry_show(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp entry show 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_entry_dump gid=%d\n"),
                 unit, gid));
    retval = bcm_field_entry_dump(unit, gid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_entry_dump");

    return CMD_OK;
}
#endif /* def BROADCOM_DEBUG */

/*
 *  Handle fp entry command
 */
static int
fp_entry(int unit, args_t *args)
{
    char*               subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp entry create ... */
            result = fp_entry_create(unit, args);
        } else if(!sal_strcasecmp(subcmd, "copy")) {
            /* BCM.0> fp entry copy ... */
            result = fp_entry_copy(unit, args);
        } else if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp entry destroy ... */
            result = fp_entry_destroy(unit, args);
        } else if(!sal_strcasecmp(subcmd, "install")) {
            /* BCM.0> fp entry install ... */
            result = fp_entry_install(unit, args);
        } else if(!sal_strcasecmp(subcmd, "reinstall")) {
            /* BCM.0> fp entry reinstall ... */
            result = fp_entry_reinstall(unit, args);
        } else if(!sal_strcasecmp(subcmd, "remove")) {
            /* BCM.0> fp entry remove ... */
            result = fp_entry_remove(unit, args);
        } else if(!sal_strcasecmp(subcmd, "prio")) {
            /* BCM.0> fp entry prio ... */
            result = fp_entry_prio(unit, args);
#ifdef BROADCOM_DEBUG
        } else if (!sal_strcasecmp(subcmd, "show")) {
            /* BCM.0> fp entry show ... */
            result = fp_entry_show(unit, args);
#endif /* def BROADCOM_DEBUG */
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_entry_usage);
        result = CMD_NFND;
    }
    return result;
}

/*
 *  Create a group
 */
static int
fp_group_create(int unit, args_t *args, bcm_field_qset_t *qset)
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

    FP_VRFY_GET_NUMB(pri, subcmd, args);

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
                                        "FP(unit %d) verb: _group_create_id pri=%d gid=%d, mode=%d\n"),
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
                                                "FP(unit %d) verb: _group_port_create_id pri=%d gid=%d, mode=%d"
                                                 "port=%s\n"), unit, pri, gid, mode,
                                     format_pbmp(unit, buf, sizeof(buf), pbmp)));

                        retval = bcm_field_group_port_create_mode_id(unit, port,
                                                         *qset, pri, mode, gid);
                        FP_CHECK_RETURN(unit, retval,
                                        "bcm_field_group_ports_create_mode_id");
		    }
		} else {
                    LOG_VERBOSE(BSL_LS_BCM_FP,
                                (BSL_META_U(unit,
                                            "FP(unit %d) verb: _group_ports_create_id pri=%d gid=%d, mode=%d"
                                             "pbmp=%s\n"), unit, pri, gid, mode,
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
 *  Destroy a group
 */
static int
fp_group_destroy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

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
 *  Get a group's QSET
 */
static int
fp_group_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_qset_t            qset;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group create 'prio'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb: _group_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_get(unit, gid, &qset);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_get");
    fp_qset_show(&qset);
    return CMD_OK;
}

/*
 *  Set a group's QSET
 */
static int
fp_group_set(int unit, args_t *args, bcm_field_qset_t *qset)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group set 'gid' */
    retval = bcm_field_group_set(unit, gid, *qset);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_set");

    return CMD_OK;
}

/*
 *  Get a group's status
 */
static int
fp_group_status_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_group_status_t    gstat;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group status 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:_group_status_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_status_get(unit, gid, &gstat);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_status_get");

    cli_out("group_status={\n");
    cli_out("\tprio_min=%d,\n", gstat.prio_min);
    cli_out("\tprio_max=%d,\n", gstat.prio_max);
    cli_out("\tentries_total=%d,\n", gstat.entries_total);
    cli_out("\tentries_free=%d,\n", gstat.entries_free);
    cli_out("\tcounters_total=%d,\n", gstat.counters_total);
    cli_out("\tcounters_free=%d,\n", gstat.counters_free);
    cli_out("\tmeters_total=%d,\n", gstat.meters_total);
    cli_out("\tmeters_free=%d\n", gstat.meters_free);
    cli_out("}\n");

    return CMD_OK;
}

/*
 *  Get a group's mode
 */
static int
fp_group_mode_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;
    bcm_field_group_mode_t      mode;
    char                        buf[BCM_FIELD_GROUP_MODE_WIDTH_MAX];

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group mode 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_mode_get gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_mode_get(unit, gid, &mode);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_mode_get");
    cli_out("group mode=%s\n", format_field_group_mode(buf, mode, 1));

    return CMD_OK;
}

/*
 *  Set a group's enable state
 */
static int
fp_group_enable_set(int unit, bcm_field_group_t gid, int enable)
{
    int                         retval = CMD_OK;

    /* BCM.0> fp group enable/disable 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_enable_set(gid=%d, enable=%d)\n"),
                 unit, gid, enable));
    retval = bcm_field_group_enable_set(unit, gid, enable);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_enable_set");

    return CMD_OK;
}

/*
 * Function:
 *     fp_group_lookup
 * Purpose:
 *     Test getting/setting the FP group packet lookup enable/disable APIs.
 * Parmameters:
 * Returns:
 */
static int
fp_group_lookup(int unit, args_t *args)
{
    char*                       subcmd = NULL;
    int                         retval = CMD_OK;
    bcm_field_group_t           gid;
    int                         enable;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp group lookup 'gid' */
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:bcm_field_group_enable_get(gid=%d)\n"),
                     unit, gid));
        retval = bcm_field_group_enable_get(unit, gid, &enable);
        FP_CHECK_RETURN(unit, retval, "bcm_field_group_enable_get");
        if (enable) {
            cli_out("GID %d: lookup=Enabled\n", gid);
        } else {
            cli_out("GID %d: lookup=Disabled\n", gid);
        }

        return CMD_OK;
    }

    /* BCM.0> fp group lookup 'gid' enable */
    if(!sal_strcasecmp(subcmd, "enable")) {
        return fp_group_enable_set(unit, gid, 1);
    }

    /* BCM.0> fp group lookup 'gid' disable */
    if(!sal_strcasecmp(subcmd, "disable")) {
        return fp_group_enable_set(unit, gid, 0);
    }
    return CMD_USAGE;
}

/*
 *  Install (or reinstall) all entries in a group
 */
static int
fp_group_install(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group install 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_install gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_install(unit, gid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_install");

    return CMD_OK;
}

/*
 *  Remove (not destroy!) all entries in a group
 */
static int
fp_group_remove(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group remove 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_remove gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_remove(unit, gid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_remove");

    return CMD_OK;
}

#ifdef BROADCOM_DEBUG
/*
 *  Show information about a group
 */
static int
fp_group_show(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_group_t           gid;

    FP_VRFY_GET_NUMB(gid, subcmd, args);

    /* BCM.0> fp group show 'gid' */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:bcm_field_group_dump gid=%d\n"),
                 unit, gid));
    retval = bcm_field_group_dump(unit, gid);
    FP_CHECK_RETURN(unit, retval, "bcm_field_group_dump");

    return CMD_OK;
}
#endif /* def BROADCOM_DEBUG */

/*
 *  Handle fp group command
 */
static int
fp_group(int unit, args_t *args, bcm_field_qset_t *qset)
{
    char*               subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp group create ... */
            result = fp_group_create(unit, args, qset);
        } else if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp group destroy ... */
            result = fp_group_destroy(unit, args);
        } else if(!sal_strcasecmp(subcmd, "get")) {
            /* BCM.0> fp group get ... */
            result = fp_group_get(unit, args);
        } else if(!sal_strcasecmp(subcmd, "set")) {
            /* BCM.0> fp group set ... */
            result = fp_group_set(unit, args, qset);
        } else if(!sal_strcasecmp(subcmd, "status")) {
            /* BCM.0> fp group status ... */
            result = fp_group_status_get(unit, args);
        } else if(!sal_strcasecmp(subcmd, "mode")) {
            /* BCM.0> fp group mode ... */
            result = fp_group_mode_get(unit, args);
        } else if(!sal_strcasecmp(subcmd, "lookup")) {
            /* BCM.0> fp group lookup ... */
            result = fp_group_lookup(unit, args);
        } else if (!sal_strcasecmp(subcmd, "install")) {
            /* BCM.0> fp group install ... */
            result = fp_group_install(unit, args);
        } else if (!sal_strcasecmp(subcmd, "remove")) {
            /* BCM.0> fp group remove ... */
            result = fp_group_remove(unit, args);
#ifdef BROADCOM_DEBUG
        } else if (!sal_strcasecmp(subcmd, "show")) {
            /* BCM.0> fp group show ... */
            result = fp_group_show(unit, args);
#endif /* def BROADCOM_DEBUG */
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_group_usage);
        result = CMD_NFND;
    }
    return result;
}

/*
 *  Handle fp list command
 */
static int
fp_list(int unit, args_t *args) {
    char*               subcmd = NULL;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp list actions */
    if(!sal_strcasecmp(subcmd, "actions")) {
        return fp_list_actions(unit);
    }
    /* BCM.0> fp list qualifiers */
    if(!sal_strcasecmp(subcmd, "qualifiers") ||
       !sal_strcasecmp(subcmd, "quals")) {
        return fp_list_quals(unit);
    }
    return CMD_USAGE;
}

/*
 * Function:
 *    FP CLI function to create an FP range group
 * Purpose:
 * Parmameters:
 * Returns:
 */
static int
fp_range_group_create(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_range_t           rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;
    bcm_if_group_t              group;
    uint32                      param[5];

    FP_VRFY_GET_NUMB(param[0], subcmd, args);
    FP_VRFY_GET_NUMB(param[1], subcmd, args);
    FP_VRFY_GET_NUMB(param[2], subcmd, args);
    FP_VRFY_GET_NUMB(param[3], subcmd, args);

    if ((subcmd = ARG_GET(args)) == NULL) {
        /* BCM.0> fp range group create 'flags' 'min' 'max' */
        flags = param[0];
        min   = param[1];
        max   = param[2];
        group = param[3];
        LOG_VERBOSE(BSL_LS_BCM_FP,
                    (BSL_META_U(unit,
                                "FP(unit %d) verb:_range_group_create flags=0x%x, min=%d, max=%d group=%d\n"),
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
                                "FP(unit %d) verb:_range_group_create_id  rid=%d, flags=0x%x, min=%d, max=%d group=%d\n"),
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
 *    FP CLI function to create an FP range
 * Purpose:
 * Parmameters:
 * Returns:
 */
static int
fp_range_create(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    bcm_field_range_t           rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;
    uint32                      param[4];

    FP_VRFY_GET_NUMB(param[0], subcmd, args);
    FP_VRFY_GET_NUMB(param[1], subcmd, args);
    FP_VRFY_GET_NUMB(param[2], subcmd, args);

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
                                "FP(unit %d) verb:_range_create_id rid=%d, flags=0x%x, min=%d, max=%d \n"),
                     unit, rid, flags, min, max));
        retval = bcm_field_range_create_id(unit, rid, flags, min, max);
        FP_CHECK_RETURN(unit, retval, "bcm_field_range_create_id");
    }

    return CMD_OK;
}

/*
 *  Get information about a range
 */
static int
fp_range_get(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         rid;
    bcm_l4_port_t               min, max;
    uint32                      flags;

    FP_VRFY_GET_NUMB(rid, subcmd, args);

    /* BCM.0> fp range get 'rid'  */
    LOG_VERBOSE(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                            "FP(unit %d) verb:fp_range_get 'rid=%d'\n"),
                 unit, rid));
    retval = bcm_field_range_get(unit, rid, &flags, &min, &max);
    FP_CHECK_RETURN(unit, retval, "bcm_field_range_get");
    cli_out("FP range get: rid=%d, min=%d max=%d ", rid, min, max);
    cli_out("flags=0x%x%s%s%s%s\n",
            flags,
            flags & BCM_FIELD_RANGE_SRCPORT ? " SRCPORT" : "",
            flags & BCM_FIELD_RANGE_DSTPORT ? " DSTPORT" : "",
            flags & BCM_FIELD_RANGE_OUTER_VLAN? " OUTERVLAN" : "",
            flags & BCM_FIELD_RANGE_PACKET_LENGTH? " PACKET LEN" : "");

    return CMD_OK;
}

/*
 *  Destroy a range
 */
static int
fp_range_destroy(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    int                         rid;

    FP_VRFY_GET_NUMB(rid, subcmd, args);

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
 *  Handle fp range command
 */
static int
fp_range(int unit, args_t *args)
{
    char*               subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "group")) {
            /* BCM.0> fp range group ... */
            if ((subcmd = ARG_GET(args)) != NULL) {
                if(!sal_strcasecmp(subcmd, "create")) {
                    /* BCM.0> fp range group create ... */
                    result = fp_range_group_create(unit, args);
                }
            }
        } else if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp range create ... */
            result = fp_range_create(unit, args);
        } else if(!sal_strcasecmp(subcmd, "get")) {
            /* BCM.0> fp range get ... */
            result = fp_range_get(unit, args);
        } else if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp range destroy ... */
            result = fp_range_destroy(unit, args);
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_range_usage);
        result = CMD_NFND;
    }
    return result;
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
static int
fp_policer_create(int unit, args_t *args)
{
    parse_table_t           pt;
    bcm_policer_config_t    pol_cfg;
    bcm_policer_t           polid = -1;
    cmd_result_t            retCode;
    bcm_policer_mode_t      mode;
    int                     color_blind;
    int                     color_merge_or;
    int                     rv;

    mode = bcmPolicerModeCount;
    color_blind = 0;
    bcm_policer_config_t_init(&pol_cfg);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "PolId", PQ_DFL|PQ_INT, 0, &polid, NULL);
    parse_table_add(&pt, "ColorBlind", PQ_DFL|PQ_INT, 0, &color_blind, NULL);
    parse_table_add(&pt, "ColorMergeOr", PQ_DFL|PQ_INT, 0,
                    &color_merge_or, NULL);
    parse_table_add(&pt, "Mode", PQ_DFL | PQ_MULTI, 0,
                    (void *)&mode, policermode_text);
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
        fp_print_options(policermode_text, sal_strlen("\tMode="));
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

    if (color_merge_or) {
        pol_cfg.flags |= (BCM_POLICER_COLOR_MERGE_OR);
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
static int
fp_policer_destroy(int unit, args_t *args)
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
static int
fp_policer_attach(int unit, args_t *args)
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
static int
fp_policer_detach(int unit, args_t *args)
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
 *  Handle fp policer command
 */
static int
fp_policer(int unit, args_t *args)
{
    char* subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp policer create ... */
            result = fp_policer_create(unit, args);
        } else if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp policer destroy ... */
            result = fp_policer_destroy(unit, args);
        } else if(!sal_strcasecmp(subcmd, "attach")) {
            /* BCM.0> fp policer attach ... */
            result = fp_policer_attach(unit, args);
        } else if(!sal_strcasecmp(subcmd, "detach")) {
            /* BCM.0> fp policer detach ... */
            result = fp_policer_detach(unit, args);
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_policer_usage);
        result = CMD_FAIL;
    }
    return result;
}

/*
 * Function:
 *     fp_stat_create
 * Purpose:
 *     Add fp counter entity.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_stat_create(int unit, args_t *args)
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
        stat_arr[idx] = 14;
    }

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "group", PQ_DFL|PQ_INT, 0, &group, NULL);

    for (idx = 0; idx < _BCM_CLI_STAT_ARR_SIZE; idx++) {
        sal_sprintf(buffer[idx], "type%d", idx);
        parse_table_add(&pt, buffer[idx], PQ_DFL | PQ_MULTI, 0,
                        (void *)&stat_arr[idx], stattype_text);
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
        fp_print_options(stattype_text, sal_strlen("\tstatXX="));
        cli_out("\n");
    }

    if (group < 0) {
        cli_out("Invalid group id (%d) \n", group);
    }

    rv = bcm_field_stat_create(unit, group, stat_arr_sz, stat_arr, &statid);
    if (BCM_FAILURE(rv)) {
        cli_out("Stat add failed. (%s) \n", bcm_errmsg(rv));
        return CMD_FAIL;
    }

    cli_out("Stat created with id: %d \n", statid);
    return CMD_OK;
}

/*
 * Function:
 *     fp_stat_destroy
 * Purpose:
 *     Remove fp stat entity.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_stat_destroy(int unit, args_t *args)
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
 *     fp_stat_attach
 * Purpose:
 *     Attach fp counter to a field entry.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_stat_attach(int unit, args_t *args)
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
    parse_table_add(&pt, "StatId", PQ_DFL|PQ_INT, 0, &statid, NULL);
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
 *     fp_stat_detach
 * Purpose:
 *     Detach fp counter from a field entry.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_stat_detach(int unit, args_t *args)
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
 *     fp_stat_set
 * Purpose:
 *     Set stat value.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_stat_set(int unit, args_t *args)
{
    int                 statid = -1;
    bcm_field_stat_t    type = bcmFieldStatCount;
    int                 retval;
    uint64              val64;
    parse_table_t       pt;

    COMPILER_64_ZERO(val64);
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "StatId", PQ_DFL|PQ_INT, 0, &statid, NULL);
    parse_table_add(&pt, "type", PQ_DFL | PQ_MULTI, 0, (void *)&type, stattype_text);
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
        fp_print_options(stattype_text, sal_strlen("\ttype="));
        cli_out("\n");
    }

    retval = bcm_field_stat_set(unit, statid, type, val64);
    FP_CHECK_RETURN(unit, retval, "bcm_field_stat_set");
    return CMD_OK;
}

/*
 * Function:
 *     fp_stat_get
 * Purpose:
 *     Get stat value.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_stat_get(int unit, args_t *args)
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
    parse_table_add(&pt, "type", PQ_DFL | PQ_MULTI, 0, (void *)&stat_arr[0], stattype_text);
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
            cli_out("%s, value is: 0x%x%x\n", stattype_text[stat_arr[idx]],
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
 *  Handle fp stat command
 */
static int
fp_stat(int unit, args_t *args)
{
    char* subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp stat create ... */
            result = fp_stat_create(unit, args);
        } else if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp stat destroy ... */
            result = fp_stat_destroy(unit, args);
        } else if(!sal_strcasecmp(subcmd, "attach")) {
            /* BCM.0> fp stat attach ... */
            result = fp_stat_attach(unit, args);
        } else if(!sal_strcasecmp(subcmd, "detach")) {
            /* BCM.0> fp stat detach ... */
            result = fp_stat_detach(unit, args);
        } else if(!sal_strcasecmp(subcmd, "set")) {
            /* BCM.0> fp stat set ... */
            result = fp_stat_set(unit, args);
        } else if(!sal_strcasecmp(subcmd, "get")) {
            /* BCM.0> fp stat get ... */
            result = fp_stat_get(unit, args);
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_stat_usage);
        result = CMD_NFND;
    }
    return result;
}

/*
 * Function:
 *     fp_data_create
 * Purpose:
 *     Add fp data qualifier entity.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_data_create(int unit, args_t *args)
{
    parse_table_t              pt;
    int                        length = -1;
    int                        offset = -1;
    int                        offset_base = 0; /* Packet Start. */
    cmd_result_t               retCode;
    bcm_field_data_qualifier_t data_qual;
    int                        rv;

    /* Initialization. */
    bcm_field_data_qualifier_t_init(&data_qual);

    /* Parse command option arguments */
    parse_table_init(unit, &pt);
    parse_table_add(&pt, "OffsetBase", PQ_DFL | PQ_MULTI, 0,
                    (void *)&offset_base, offsetbase_text);
    parse_table_add(&pt, "offset", PQ_DFL|PQ_INT, 0, &offset, NULL);
    parse_table_add(&pt, "length", PQ_DFL|PQ_INT, 0, &length, NULL);

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
 *     fp_data_destroy
 * Purpose:
 *     Destroy fp data qualifier entity.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 * Returns:
 *     Command result.
 */
static int
fp_data_destroy(int unit, args_t *args)
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
 *     fp_data_packet_format_add_delete
 * Purpose:
 *     Add/delete packet format to the fp data qualifier.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 *     op - (0 delete, else add)
 * Returns:
 *     Command result.
 */
static int
fp_data_packet_format_add_delete(int unit, args_t *args, int op)
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
 *     fp_data_ethertype_add_delete
 * Purpose:
 *     Add/Delete ethertype to the fp data qualifier.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 *     op - (0 delete, else add)
 * Returns:
 *     Command result.
 */
static int
fp_data_ethertype_add_delete(int unit, args_t *args, int op)
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
 *     fp_data_ipproto_add_delete
 * Purpose:
 *     Add/Delete ip protocol to the fp data qualifier.
 * Parmameters:
 *     unit - (IN) Bcm device number.
 *     args - (IN) Command arguments.
 *     op - (0 delete, else add)
 * Returns:
 *     Command result.
 */
static int
fp_data_ipproto_add_delete(int unit, args_t *args, int op)
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
      case 2: /* IPv4 */
          ipproto.flags = BCM_FIELD_DATA_FORMAT_IP4;
          break;
      case 3: /* IPv6 */
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
 *  Handle fp data command
 */
static int
fp_data(int unit, args_t *args)
{
    char* subcmd = NULL;
    int result = CMD_USAGE;

    if ((subcmd = ARG_GET(args)) != NULL) {
        if(!sal_strcasecmp(subcmd, "create")) {
            /* BCM.0> fp data create ... */
            result = fp_data_create(unit, args);
        } else if(!sal_strcasecmp(subcmd, "destroy")) {
            /* BCM.0> fp data destroy ... */
            result = fp_data_destroy(unit, args);
        } else if(!sal_strcasecmp(subcmd, "format")) {
            /* BCM.0> fp data format ... */
            if ((subcmd = ARG_GET(args)) != NULL) {
                if(!sal_strcasecmp(subcmd, "add")) {
                    /* BCM.0> fp data format add ... */
                    result = fp_data_packet_format_add_delete(unit, args, TRUE);
                } else if(!sal_strcasecmp(subcmd, "delete")) {
                    /* BCM.0> fp data format delete ... */
                    result = fp_data_packet_format_add_delete(unit, args, FALSE);
                }
            }
        } else if(!sal_strcasecmp(subcmd, "ethertype")) {
            /* BCM.0> fp data ethertype ... */
            if ((subcmd = ARG_GET(args)) != NULL) {
                if(!sal_strcasecmp(subcmd, "add")) {
                    /* BCM.0> fp data ethertype add ... */
                    result = fp_data_ethertype_add_delete(unit, args, TRUE);
                } else if(!sal_strcasecmp(subcmd, "delete")) {
                    /* BCM.0> fp data ethertype delete ... */
                    result = fp_data_ethertype_add_delete(unit, args, FALSE);
                }
            }
        } else if(!sal_strcasecmp(subcmd, "ipproto")) {
            /* BCM.0> fp data ipprotocol ... */
            if ((subcmd = ARG_GET(args)) != NULL) {
                if(!sal_strcasecmp(subcmd, "add")) {
                    /* BCM.0> fp data ipprotocol add ... */
                    result = fp_data_ipproto_add_delete(unit, args, TRUE);
                } else if(!sal_strcasecmp(subcmd, "delete")) {
                    /* BCM.0> fp data ipprotocol delete ... */
                    result = fp_data_ipproto_add_delete(unit, args, FALSE);
                }
            }
        }
    }
    if (CMD_USAGE == result) {
        cli_out(if_field_proc_data_usage);
        result = CMD_NFND;
    }
    return result;
}

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
if_sbx_field_proc(int unit, args_t *args)
{
    int                         retval = CMD_OK;
    char*                       subcmd = NULL;
    static bcm_field_qset_t     qset;

    if ((subcmd = ARG_GET(args)) == NULL) {
        return CMD_USAGE;
    }

    /* BCM.0> fp qual ... */
    if((!sal_strcasecmp(subcmd, "qual")) ||
       (!sal_strcasecmp(subcmd, "qualify"))) {
        retval = fp_qual(unit, args);
        if (CMD_USAGE == retval) {
            cli_out(if_field_proc_qual_usage);
            retval = CMD_NFND;
        }
        return retval;
    }

    /* BCM.0> fp qset ... */
    if((!sal_strcasecmp(subcmd, "qset")) ||
       (!sal_strcasecmp(subcmd, "qualifierset"))) {
        return fp_qset(unit, args, &qset);
    }

    /* BCM.0> fp action ... */
    if(!sal_strcasecmp(subcmd, "action")) {
        return fp_action(unit, args);
    }

    /* BCM.0> fp control ... */
    if(!sal_strcasecmp(subcmd, "control")) {
        return fp_control(unit, args);
    }

    /* BCM.0> fp counter ... */
    if(!sal_strcasecmp(subcmd, "counter")) {
        return fp_counter(unit, args);
    }

    /* BCM.0> fp detach */
    if(!sal_strcasecmp(subcmd, "detach")) {
        retval = bcm_field_detach(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_detach");
        return CMD_OK;
    }

    /* BCM.0> fp entry ... */
    if(!sal_strcasecmp(subcmd, "entry")) {
        return fp_entry(unit, args);
    }

    /* BCM.0> fp group ... */
    if(!sal_strcasecmp(subcmd, "group")) {
        return fp_group(unit, args, &qset);
    }

    /* BCM.0> fp list ... */
    if(!sal_strcasecmp(subcmd, "list")) {
        return fp_list(unit, args);
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
        return fp_entry_install(unit, args);
    }

    /* BCM.0> fp range ... */
    if(!sal_strcasecmp(subcmd, "range")) {
        return fp_range(unit, args);
    }

    /* BCM.0> fp resync */
    if(!sal_strcasecmp(subcmd, "resync")) {
        retval = bcm_field_resync(unit);
        FP_CHECK_RETURN(unit, retval, "bcm_field_resync");
        return CMD_OK;
    }

    /* BCM.0> fp policer ... */
    if(!sal_strcasecmp(subcmd, "policer")) {
        return fp_policer(unit, args);
    }

    /* BCM.0> fp stat ... */
    if(!sal_strcasecmp(subcmd, "stat")) {
        return fp_stat(unit, args);
    }

    /* BCM.0> fp data ... */
    if(!sal_strcasecmp(subcmd, "data")) {
        return fp_data(unit, args);
    }

#ifdef BROADCOM_DEBUG
    /* BCM.0> fp show ...*/
    if(!sal_strcasecmp(subcmd, "show")) {
        retval = CMD_USAGE;
        if ((subcmd = ARG_GET(args)) != NULL) {
            if(!sal_strcasecmp(subcmd, "entry")) {
                /* BCM.0> fp show entry ...*/
                retval = fp_entry_show(unit, args);
            } else if(!sal_strcasecmp(subcmd, "group")) {
                /* BCM.0> fp show group ...*/
                retval = fp_group_show(unit, args);
            } else if(!sal_strcasecmp(subcmd, "qset")) {
                /* BCM.0> fp show qset */
                fp_qset_show(&qset);
                retval = CMD_OK;
            }
        } else {
            /* BCM.0> fp show */
            bcm_field_show(unit, "FP ");
            retval = CMD_OK;
        }
        if (CMD_USAGE == retval) {
            cli_out(if_field_proc_show_usage);
            retval = CMD_NFND;
        }
        return retval;
    }
#endif /* def BROADCOM_DEBUG */
    return CMD_USAGE;
}


#endif /* def BCM_FIELD_SUPPORT */
