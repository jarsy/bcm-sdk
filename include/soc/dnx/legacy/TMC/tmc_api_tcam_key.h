/* $Id: soc_jer2_jer2_jer2_tmcapi_tcam_key.h,v 1.4 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
*/

#ifndef __DNX_TMC_API_TCAM_KEY_INCLUDED__
/* { */
#define __DNX_TMC_API_TCAM_KEY_INCLUDED__

/*************
 * INCLUDES  *
 *************/
/* { */

#include <soc/dnx/legacy/SAND/Utils/sand_header.h>

#include <soc/dnx/legacy/SAND/Management/sand_general_macros.h>
#include <soc/dnx/legacy/SAND/Management/sand_error_code.h>

#include <soc/dnx/legacy/TMC/tmc_api_tcam.h>
#include <soc/dnx/legacy/TMC/tmc_api_pmf_low_level_ce.h>

/* } */
/*************
 * DEFINES   *
 *************/
/* { */

/*     Maximum size in longs of a TCAM entry                   */
#define  DNX_TMC_TCAM_RULE_NOF_UINT32S_MAX (9)


/*     Maximum field value size in longs.                      */
#define  DNX_TMC_TCAM_KEY_FLD_NOF_UINT32S_MAX (4)

/* } */
/*************
 * MACROS    *
 *************/
/* { */

/* } */
/*************
 * TYPE DEFS *
 *************/
/* { */

typedef enum
{
  /*
   *  Ethernet L2 ACL Key
   */
  DNX_TMC_EGR_ACL_DB_TYPE_ETH = 0,
  /*
   *  L3 IPv4 ACL Key
   */
  DNX_TMC_EGR_ACL_DB_TYPE_IPV4 = 1,
  /*
   *  MPLS ACL Key
   */
  DNX_TMC_EGR_ACL_DB_TYPE_MPLS = 2,
  /*
   *  TM ACL Key
   */
  DNX_TMC_EGR_ACL_DB_TYPE_TM = 3,
  /*
   *  Number of types in DNX_TMC_EGR_ACL_DB_TYPE
   */
  DNX_TMC_EGR_NOF_ACL_DB_TYPES = 4
}DNX_TMC_EGR_ACL_DB_TYPE;

typedef enum
{
  /*
   *  L2 Key - 144 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_L2 = 0,
  /*
   *  L3 IPv4  Key- 144 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_L3_IPV4 = 1,
  /*
   *  L3 IPv6  Key - 288 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_L3_IPV6 = 2,
  /*
   *  (Key B[144:0], Key A[144:0]) - 288 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_B_A = 3,
  /*
   *  Key A [71:0] - 72 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_A_71_0 = 4,
  /*
   *  Key A [103:32] - 72 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_A_103_32 = 5,
  /*
   *  Key A [143:0] - 144 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_A_143_0 = 6,
  /*
   *  Key A [175:32] - 144 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_A_175_32 = 7,
  /*
   *  Key B [71:0] - 72 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_B_71_0 = 8,
  /*
   *  Key B [103:32] - 72 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_B_103_32 = 9,
  /*
   *  Key B [143:0] - 144 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_B_143_0 = 10,
  /*
   *  Key B [175:32] - 144 bits.
   */
  DNX_TMC_PMF_TCAM_KEY_SRC_B_175_32 = 11,
  /*
   *  Number of types in DNX_TMC_PMF_TCAM_KEY_SRC
   */
  DNX_TMC_NOF_PMF_TCAM_KEY_SRCS = 12
}DNX_TMC_PMF_TCAM_KEY_SRC;

typedef enum
{
  /*
   *  PMF.
   */
  DNX_TMC_TCAM_KEY_FORMAT_TYPE_PMF = 0,
  /*
   *  Egress ACL.
   */
  DNX_TMC_TCAM_KEY_FORMAT_TYPE_EGR_ACL = 1,
  /*
   *  Number of types in DNX_TMC_TCAM_KEY_FORMAT_TYPE
   */
  DNX_TMC_TCAM_NOF_KEY_FORMAT_TYPES = 2
}DNX_TMC_TCAM_KEY_FORMAT_TYPE;

typedef enum
{
  /*
   *  LLVP-Incoming-Tag-Structure field
   */
  DNX_TMC_PMF_TCAM_FLD_L2_LLVP = 0,
  /*
   *  S-Tag field equal to the bytes 15 and 16 from the
   *  beginning of the Ethernet header
   */
  DNX_TMC_PMF_TCAM_FLD_L2_STAG = 1,
  /*
   *  If the 'Use-AC' configuration is set, then equal to the
   *  Learn-ASD field, otherwise to the C-Tag field
   */
  DNX_TMC_PMF_TCAM_FLD_L2_CTAG_IN_AC = 2,
  /*
   *  Source MAC address field
   */
  DNX_TMC_PMF_TCAM_FLD_L2_SA = 3,
  /*
   *  Destination MAC address field
   */
  DNX_TMC_PMF_TCAM_FLD_L2_DA = 4,
  /*
   *  Ethernet-Type code field as encoded by the parser
   */
  DNX_TMC_PMF_TCAM_FLD_L2_ETHERTYPE = 5,
  /*
   *  In-PP-Port
   */
  DNX_TMC_PMF_TCAM_FLD_L2_IN_PP_PORT = 6,
  /*
   *  Number of types in DNX_TMC_PMF_TCAM_FLD_L2
   */
  DNX_TMC_NOF_PMF_TCAM_FLD_L2S = 7
}DNX_TMC_PMF_TCAM_FLD_L2;

typedef enum
{
  /*
   *  IPv4 mode
   */
  DNX_TMC_PMF_TCAM_FLD_L3_MODE_IPV4 = 0,
  /*
   *  IPv6 mode
   */
  DNX_TMC_PMF_TCAM_FLD_L3_MODE_IPV6 = 1,
  /*
   *  Number of types in DNX_TMC_PMF_TCAM_FLD_L3_MODE
   */
  DNX_TMC_NOF_PMF_TCAM_FLD_L3_MODES = 2
}DNX_TMC_PMF_TCAM_FLD_L3_MODE;

typedef enum
{
  /*
   *  Equal to L4Ops[22:16]
   */
  DNX_TMC_PMF_FLD_IPV4_L4OPS = 0,
  /*
   *  Next-Protocol as encoded by the parser
   */
  DNX_TMC_PMF_FLD_IPV4_NEXT_PRTCL = 1,
  /*
   *  Dont-Fragment field equal to the Dont-Fragment field in
   *  the IPv4 header
   */
  DNX_TMC_PMF_FLD_IPV4_DF = 2,
  /*
   *  More-Fragment field equal to the More-Fragment field in
   *  the IPv4 header
   */
  DNX_TMC_PMF_FLD_IPV4_MF = 3,
  /*
   *  Fragment-Offset-Non-Zero field set if the
   *  Fragment-Offset field in the IPv4 header is not zero
   */
  DNX_TMC_PMF_FLD_IPV4_FRAG_NON_0 = 4,
  /*
   *  L4Ops-flag field set if any L4-Ops[0..15] field in the
   *  IPv4 header is set. If set, then the key holds 16
   *  additional L4-Ops. Otherwise, the L4 Ports are
   *  explicitly available
   */
  DNX_TMC_PMF_FLD_IPV4_L4OPS_FLAG = 5,
  /*
   *  SIP field equal to the 4th 32-bit word in the IPv4
   *  header
   */
  DNX_TMC_PMF_FLD_IPV4_SIP = 6,
  /*
   *  DIP field equal to the 5th 32-bit word in the IPv4
   *  header
   */
  DNX_TMC_PMF_FLD_IPV4_DIP = 7,
  /*
   *  L4Ops-Optional field equal to L4Ops[15..0] field in the
   *  IPv4 header. Is valid only if L4Ops-Flag is set.
   */
  DNX_TMC_PMF_FLD_IPV4_L4OPS_OPT = 8,
  /*
   *  L4-Src-Port field equal to the first 16-bits after the
   *  IPv4 header. Is valid only if L4Ops-Flag is unset.
   */
  DNX_TMC_PMF_FLD_IPV4_SRC_PORT = 9,
  /*
   *  L4-Dest-Port field equal to the second 16-bits after the
   *  IPv4 header. Is valid only if L4Ops-Flag is unset.
   */
  DNX_TMC_PMF_FLD_IPV4_DEST_PORT = 10,
  /*
   *  ToS field equal to the TOS field in the IPv4 header
   */
  DNX_TMC_PMF_FLD_IPV4_TOS = 11,
  /*
   *  TCP-Control field equal to the bits 106-111 after the
   *  IPv4 header (Bits URG to FIN of the TCP header)
   */
  DNX_TMC_PMF_FLD_IPV4_TCP_CTL = 12,
  /*
   *  In-AC-Or-VRF field equal to VRF if not null, otherwise
   *  to the learnt ASD field (Learn-ASD). Valid only if the
   *  'Use-AC' configuration is set.
   */
  DNX_TMC_PMF_FLD_IPV4_IN_AC_VRF = 13,
  /*
   *  In-PP-Port field
   */
  DNX_TMC_PMF_FLD_IPV4_IN_PP_PORT = 14,
  /*
   *  Incoming-VID field equal to the Outermost VID (Bytes 15
   *  and 16 of the outermost Ethernet header) if the
   *  'VID-Valid' configuration is set, otherwise 0.
   */
  DNX_TMC_PMF_FLD_IPV4_IN_VID = 15,
  /*
   *  Number of types in DNX_TMC_PMF_FLD_IPV4
   */
  DNX_TMC_NOF_PMF_FLD_IPV4S = 16
}DNX_TMC_PMF_FLD_IPV4;

typedef enum
{
  /*
   *  Equal to L4Ops[7:0]
   */
  DNX_TMC_PMF_FLD_IPV6_L4OPS = 0,
  /*
   *  SIP field equal to the 4th 32-bit word in the IPv6
   *  header
   */
  DNX_TMC_PMF_FLD_IPV6_SIP_HIGH = 1,
  DNX_TMC_PMF_FLD_IPV6_SIP_LOW = 2,
  /*
   *  DIP field equal to the 5th 32-bit word in the IPv6
   *  header
   */
   DNX_TMC_PMF_FLD_IPV6_DIP_HIGH = 3,
   DNX_TMC_PMF_FLD_IPV6_DIP_LOW = 4,
  /*
   *  Next-Protocol as encoded by the parser
   */
  DNX_TMC_PMF_FLD_IPV6_NEXT_PRTCL = 5,
  /*
   *  TCP-Control field equal to the bits 106-111 after the
   *  IPv6 header (Bits URG to FIN of the TCP header) if the
   *  'TCP-Control' configuration is set, otherwise it equals
   *  the In-PP-Port field.
   */
  DNX_TMC_PMF_FLD_IPV6_PP_PORT_TCP_CTL = 6,
  /*
   *  In-AC-Or-VRF field equal to VRF if not null, otherwise
   *  to the learnt ASD field (Learn-ASD).
   */
  DNX_TMC_PMF_FLD_IPV6_IN_AC_VRF = 7,
  /*
   *  Number of types in DNX_TMC_PMF_FLD_IPV6
   */
  DNX_TMC_NOF_PMF_FLD_IPV6S = 8
}DNX_TMC_PMF_FLD_IPV6;

typedef enum
{
  /*
   *  The field is located in the IRPP information.
   */
  DNX_TMC_PMF_TCAM_FLD_A_B_LOC_IRPP = 0,
  /*
   *  The field is located in a packet sub-header.
   */
  DNX_TMC_PMF_TCAM_FLD_A_B_LOC_HDR = 1,
  /*
   *  Number of types in DNX_TMC_PMF_TCAM_FLD_A_B_LOC
   */
  DNX_TMC_NOF_PMF_TCAM_FLD_A_B_LOCS = 2
}DNX_TMC_PMF_TCAM_FLD_A_B_LOC;

typedef enum
{
  /*
   *  Ethernet-Type-Code (4b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_ETH_TYPE_CODE = 0,
  /*
   *  Ethernet-Tag-Format (5b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_ETH_TAG_FORMAT = 1,
  /*
   *  Outer TAG (Equal to Bytes 2 and 3) (16b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_OUTER_TAG = 2,
  /*
   *  Inner TAG (Equal to Bytes 6 and 7) (16b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_INNER_TAG = 3,
  /*
   *  Source MAC address field (48b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_SA = 4,
  /*
   *  Destination MAC address field (48b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_DA = 5,
  /*
   *  Out-PP-Port. ACL-Data (6b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_OUT_PP_PORT_ACL_DATA = 6,
  /*
   *  Number of types in DNX_TMC_EGR_ACL_TCAM_FLD_L2
   */
  DNX_TMC_EGR_NOF_ACL_TCAM_FLD_L2S = 7
}DNX_TMC_EGR_ACL_TCAM_FLD_L2;

typedef enum
{
  /*
   *  Type of Service (8b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_TOS = 0,
  /*
   *  L4 Protocol Code (resolved value) (4b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_L4_PRO_CODE = 1,
  /*
   *  Source-IP (32b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_SIP = 2,
  /*
   *  Destination IP (32b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_DIP = 3,
  /*
   *  Outer VID (12b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_OUTER_VID = 4,
  /*
   *  Out-PP-Port. ACL-Data (6b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_OUT_PP_PORT_ACL_DATA = 5,
  /*
   *  Number of types in DNX_TMC_EGR_ACL_TCAM_FLD_IPV4
   */
  DNX_TMC_EGR_NOF_ACL_TCAM_FLD_IPV4S = 6
}DNX_TMC_EGR_ACL_TCAM_FLD_IPV4;

typedef enum
{
  /*
   *  The entire FTMH-Base Header (48b) and
   *  The CUD from the FTMH or as assigned by the Default TM
   *  Actions block (16b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_FTMH = 0,
  /*
   *  Copy of header data following the FTMH-Base and CUD (if
   *  exist) (38b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_HDR_DATA = 1,
  /*
   *  The MPLS shim used for the forwarding (32b)
   */
   DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_LABEL = 2,
   DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_EXP = 3,
   DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_TTL = 4,
  /*
   *  Out-PP-Port. ACL-Data (6b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_OUT_PP_PORT_ACL_DATA = 5,
  /*
   *  Number of types in DNX_TMC_EGR_ACL_TCAM_FLD_MPLS
   */
  DNX_TMC_EGR_NOF_ACL_TCAM_FLD_MPLSS = 6
}DNX_TMC_EGR_ACL_TCAM_FLD_MPLS;

typedef enum
{
  /*
   *  The entire FTMH-Base Header (48b) and
   *  The CUD from the FTMH or as assigned by the Default TM
   *  Actions block (16b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_TM_FTMH = 0,
  /*
   *  Copy of header data following the FTMH-Base and CUD (if
   *  exist) - PPH (70b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_TM_HDR_DATA = 1,
  /*
   *  Out-PP-Port. ACL-Data (6b)
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_TM_OUT_PP_PORT_ACL_DATA = 2,
  /*
   *  Number of types in DNX_TMC_EGR_ACL_TCAM_FLD_TM
   */
  DNX_TMC_EGR_NOF_ACL_TCAM_FLD_TMS = 3
}DNX_TMC_EGR_ACL_TCAM_FLD_TM;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Value of the rule. Must be set according to the bank
   *  size (For Soc_petra-B, 72b / 144b / 288b).
   */
  uint32 val[DNX_TMC_TCAM_RULE_NOF_UINT32S_MAX];
  /*
   *  Mask of the rule. Must be set according to the bank size
   *  (For Soc_petra-B, 72b / 144b / 288b). Set '1' for the
   *  corresponding bit to be meaningful
   */
  uint32 mask[DNX_TMC_TCAM_RULE_NOF_UINT32S_MAX];

} DNX_TMC_TCAM_RULE_INFO;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  TCAM Key format type: PMF or Egress ACL.
   */
  DNX_TMC_TCAM_KEY_FORMAT_TYPE type;
  /*
   *  PMF Key format source. Valid only if type is 'PMF'
   */
  DNX_TMC_PMF_TCAM_KEY_SRC pmf;
  /*
   *  Egress ACL Key format source. Valid only if type is
   *  'EGR_ACL'
   */
  DNX_TMC_EGR_ACL_DB_TYPE egr_acl;

} DNX_TMC_TCAM_KEY_FORMAT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Size of the key data. Must be set according to the bank
   *  size (For Soc_petra-B, 72b / 144b / 288b).
   */
  DNX_TMC_TCAM_BANK_ENTRY_SIZE size;
  /*
   *  Key data.
   */
  DNX_TMC_TCAM_RULE_INFO data;
  /*
   *  Format (i.e., key source) of the TCAM key.
   */
  DNX_TMC_TCAM_KEY_FORMAT format;
  /*
   *  PMF Program-ID. Range: 0 - 31.
   */
  uint32 pmf_pgm_id;

} DNX_TMC_TCAM_KEY;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Value of the output. Is encoded in 32 bits for Soc_petra-B.
   */
  uint32 val;

} DNX_TMC_TCAM_OUTPUT;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Field Layer 3 mode: IPv4 or IPv6.
   */
  DNX_TMC_PMF_TCAM_FLD_L3_MODE mode;
  /*
   *  Field type of the IPv4 Key. Must be set only if the
   *  location 'mode' is 'IPV4'.
   */
  DNX_TMC_PMF_FLD_IPV4 ipv4_fld;
  /*
   *  Field type of the IPv6 Key. Must be set only if the
   *  location 'mode' is 'IPV6'.
   */
  DNX_TMC_PMF_FLD_IPV6 ipv6_fld;

} DNX_TMC_PMF_TCAM_FLD_L3;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Location of the field: IRPP information or Sub-header of
   *  the Packet.
   */
  DNX_TMC_PMF_TCAM_FLD_A_B_LOC loc;
  /*
   *  Field type of the IRPP information. Must be set only if
   *  the location 'loc' is 'IRPP' and must fit one of the
   *  Copy Engine configurations.
   */
  DNX_TMC_PMF_IRPP_INFO_FIELD irpp_fld;
  /*
   *  Sub-header type of the Packet. Must be set only if the
   *  location 'loc' is 'HDR'.
   */
  DNX_TMC_PMF_CE_SUB_HEADER sub_header;
  /*
   *  Internal offset in the packet sub-header. Must be set
   *  only if the location 'loc' is 'HDR' and must fit one of
   *  the Copy Engine configurations.
   */
  int32 sub_header_offset;
  /*
   *  PMF Program Index. Indicate which Copy Engine
   *  instructions to look for in order to find the field
   *  location. Range: 0 - 31. (Soc_petra-B)
   */
  uint32 pmf_pgm_id;

} DNX_TMC_PMF_TCAM_FLD_A_B;


typedef union
{
  /*
   *  Field type of the Layer 2 Ethernet ACL key. Must be set
   *  only if the Key format is 'L2'.
   */
  DNX_TMC_PMF_TCAM_FLD_L2 l2;
  /*
   *  Field type of the Layer 3 IPv4 or IPv6 key. Must be set
   *  only if the Key format is 'L3'.
   */
  DNX_TMC_PMF_TCAM_FLD_L3 l3;
  /*
   *  The field is part of the IRPP information or of the
   *  packet header and is used in a Copy Engine. Must be set
   *  only if the Key format is part of Key-A or Key-B.
   */
  DNX_TMC_PMF_TCAM_FLD_A_B a_b;
  /*
   *  The field is part of the predefined Egress ACL L2
   *  Ethernet Key. Must be set only if the Key format is
   *  Egress ACL L2.
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_L2 egr_l2;
  /*
   *  The field is part of the predefined Egress ACL IPv4 Key.
   *  Must be set only if the Key format is Egress ACL IPv4.
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4 egr_ipv4;
  /*
   *  The field is part of the predefined Egress ACL MPLS Key.
   *  Must be set only if the Key format is Egress ACL MPLS.
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_MPLS egr_mpls;
  /*
   *  The field is part of the predefined Egress ACL TM Key.
   *  Must be set only if the Key format is Egress ACL TM.
   */
  DNX_TMC_EGR_ACL_TCAM_FLD_TM egr_tm;

} DNX_TMC_TCAM_KEY_FLD_TYPE;

typedef struct
{
  DNX_SAND_MAGIC_NUM_VAR
  /*
   *  Field value. The considered value size is determined
   *  according to field type.
   */
  uint32 val[DNX_TMC_TCAM_KEY_FLD_NOF_UINT32S_MAX];

} DNX_TMC_TCAM_KEY_FLD_VAL;

/* } */
/*************
 * GLOBALS   *
 *************/
/* { */

/* } */
/*************
 * FUNCTIONS *
 *************/
/* { */

void
  DNX_TMC_TCAM_RULE_INFO_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_RULE_INFO *info
  );

void
  DNX_TMC_TCAM_KEY_FORMAT_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_KEY_FORMAT *info
  );

void
  DNX_TMC_TCAM_KEY_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_KEY *info
  );

void
  DNX_TMC_TCAM_OUTPUT_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_OUTPUT *info
  );


void
  DNX_TMC_PMF_TCAM_FLD_L3_clear(
    DNX_SAND_OUT DNX_TMC_PMF_TCAM_FLD_L3 *info
  );

void
  DNX_TMC_PMF_TCAM_FLD_A_B_clear(
    DNX_SAND_OUT DNX_TMC_PMF_TCAM_FLD_A_B *info
  );
  
void
  DNX_TMC_TCAM_KEY_FLD_TYPE_clear(
    DNX_SAND_IN  DNX_TMC_TCAM_KEY_FORMAT    *key_format,
    DNX_SAND_OUT DNX_TMC_TCAM_KEY_FLD_TYPE *info
  );

void
  DNX_TMC_TCAM_KEY_FLD_VAL_clear(
    DNX_SAND_OUT DNX_TMC_TCAM_KEY_FLD_VAL *info
  );

#if DNX_TMC_DEBUG_IS_LVL1

const char*
  DNX_TMC_EGR_ACL_DB_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_EGR_ACL_DB_TYPE enum_val
  );

const char*
  DNX_TMC_PMF_TCAM_KEY_SRC_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_TCAM_KEY_SRC enum_val
  );

const char*
  DNX_TMC_TCAM_KEY_FORMAT_TYPE_to_string(
    DNX_SAND_IN  DNX_TMC_TCAM_KEY_FORMAT_TYPE enum_val
  );

const char*
  DNX_TMC_PMF_TCAM_FLD_L2_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_TCAM_FLD_L2 enum_val
  );

const char*
  DNX_TMC_PMF_TCAM_FLD_L3_MODE_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_TCAM_FLD_L3_MODE enum_val
  );

const char*
  DNX_TMC_PMF_FLD_IPV4_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_FLD_IPV4 enum_val
  );

const char*
  DNX_TMC_PMF_FLD_IPV6_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_FLD_IPV6 enum_val
  );

const char*
  DNX_TMC_PMF_TCAM_FLD_A_B_LOC_to_string(
    DNX_SAND_IN  DNX_TMC_PMF_TCAM_FLD_A_B_LOC enum_val
  );

const char*
  DNX_TMC_EGR_ACL_TCAM_FLD_L2_to_string(
    DNX_SAND_IN  DNX_TMC_EGR_ACL_TCAM_FLD_L2 enum_val
  );

const char*
  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4_to_string(
    DNX_SAND_IN  DNX_TMC_EGR_ACL_TCAM_FLD_IPV4 enum_val
  );

const char*
  DNX_TMC_EGR_ACL_TCAM_FLD_MPLS_to_string(
    DNX_SAND_IN  DNX_TMC_EGR_ACL_TCAM_FLD_MPLS enum_val
  );

const char*
  DNX_TMC_EGR_ACL_TCAM_FLD_TM_to_string(
    DNX_SAND_IN  DNX_TMC_EGR_ACL_TCAM_FLD_TM enum_val
  );

void
  DNX_TMC_TCAM_RULE_INFO_print(
    DNX_SAND_IN  DNX_TMC_TCAM_RULE_INFO *info
  );

void
  DNX_TMC_TCAM_KEY_FORMAT_print(
    DNX_SAND_IN  DNX_TMC_TCAM_KEY_FORMAT *info
  );

void
  DNX_TMC_TCAM_KEY_print(
    DNX_SAND_IN  DNX_TMC_TCAM_KEY *info
  );

void
  DNX_TMC_TCAM_OUTPUT_print(
    DNX_SAND_IN  DNX_TMC_TCAM_OUTPUT *info
  );

void
  DNX_TMC_PMF_TCAM_FLD_L3_print(
    DNX_SAND_IN  DNX_TMC_PMF_TCAM_FLD_L3 *info
  );

void
  DNX_TMC_PMF_TCAM_FLD_A_B_print(
    DNX_SAND_IN  DNX_TMC_PMF_TCAM_FLD_A_B *info
  );


void
  DNX_TMC_TCAM_KEY_FLD_TYPE_print(
    DNX_SAND_IN  DNX_TMC_TCAM_KEY_FLD_TYPE *info
  );

void
  DNX_TMC_TCAM_KEY_FLD_VAL_print(
    DNX_SAND_IN  DNX_TMC_TCAM_KEY_FLD_VAL *info
  );

#endif /* DNX_TMC_DEBUG_IS_LVL1 */

/* } */

#include <soc/dnx/legacy/SAND/Utils/sand_footer.h>

/* } __DNX_TMC_API_TCAM_KEY_INCLUDED__*/
#endif

