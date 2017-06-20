/*
 * $Id: rce.h,v 1.27.12.2 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    rce.h
 * Purpose: Caladan3 Rule Classifier drivers
 * Requires:
 */

#ifdef BCM_CALADAN3_SUPPORT

#include <sal/types.h>

/*
 *  The range of valid bits from the key.  The lower 448 of these are from the
 *  key provided by the LRP microcode; the next 64 are the various internal
 *  result registers; the final 16 are from the key_control register.
 */
#define SOC_C3_RCE_KEY_BIT_MINIMUM 0
#define SOC_C3_RCE_KEY_BIT_MAXIMUM 511

/*
 *  The number of programs allowed by the hardware
 */
#define SOC_C3_RCE_PROGRAM_COUNT 16

/*
 *  Indicates how many result registers are available.  It specifies how many
 *  result registers are available of each type (so if set to 4, that means
 *  there are 4 internal RCE result registers and 4 LRP result registers).
 */
#define SOC_C3_RCE_RESULT_REGISTER_COUNT 4

/*
 *  CONFIG:
 *
 *  When initialising, this assumes a number of groups based upon the fairly
 *  extravagant idea that there would be one filter set per group, and each
 *  group will average some number of instructions worth of qualifiers.  If the
 *  expected number of instructions is too high, it may allocate space for too
 *  few groups; if too low, it may allocate space for lots of groups that will
 *  go unused.  In any case, it assumes support for at least one group per
 *  active program as a baseline.  It is expressed in RCE clocks (the RCE runs
 *  eight instructions per clock).
 *
 *  This really has no effect on anything other than diagnostic estimates of
 *  capacity, and is currently only used during init for the initial estimate.
 */
#define SOC_C3_RCE_GROUP_EXPECTED_LENGTH 12

/*
 *  CONFIG:
 *
 *  The number of qualifiers supported per group.  This should be large enough
 *  for general use, but it is allowed to be changed here if there are special
 *  needs (or if optimisation is desired)...
 *
 *  If the customer does not need so many qualifiers, this can be reduced to a
 *  reasonable point (it must be greater than zero) to save memory.
 *
 *  If the customer finds more qualifiers are necessary, this can be increased
 *  to allow that, at cost of increased memory use.
 */
#define SOC_C3_RCE_GROUP_QUAL_MAX 32

/*
 *  CONFIG:
 *
 *  The number of actions supported per program.  This should be large enough
 *  for general use, but it is allowed to be changed here if there are special
 *  needs (or if optimisation is desired)...
 *
 *  If the customer does not need so many actions, this can be reduced to a
 *  reasonable point (it must be greater than zero) to save memory.
 *
 *  If the customer finds more actions are necessary, this can be increased to
 *  allow that, at cost of increased memory use.  NOTE: Taking advantage of
 *  such an increase may require modifications to the tools.
 */
#define SOC_C3_RCE_PROGRAM_ACTION_MAX 32

/*
 *  There is a set of hit counters that count the number of hits to particular
 *  ranges of results (not entries!).  This indicates the number of such
 *  counters.  There are APIs to manipulate these counters and their respective
 *  ranges by reference to entries, but those have certain restrictions.
 */
#define SOC_C3_RCE_RESULT_HIT_COUNTER_COUNT 16
#define SOC_C3_RCE_RESULT_HIT_ANY_COUNTER -1
#define SOC_C3_RCE_RESULT_MISS_COUNTER -2

/*
 *  CONFIG:
 *
 *  When describing the location of bits from a frame, this specifies the
 *  particular header from which the bits are taken.
 *
 *  Note in the case of split headers (such as Ethernet header with VLAN tags
 *  present) offsets are based upon the header being contiguous (as if the
 *  intervening headeer were removed), at least with the default LRP microcode.
 *  Customer versions may choose to behave differently.
 *
 *  The customer can add more header types here if desired.  It is easy to
 *  imagine new features or standards that would expand this list.  If this
 *  list is expanded, the strings below should also be updated.
 */
typedef enum soc_c3_rce_data_header_e {
    socC3RceDataDirectKey,          /* bits directly from the key */
    socC3RceDataMetadata,           /* metadata (offset indicates which) */
    socC3RceDataHeaderRaw,          /* frame/fabric/proprietary header */
    socC3RceDataHeaderEther,        /* Ethernet header */
    socC3RceDataHeaderVlan,         /* VLAN header */
    socC3RceDataHeaderMpls,         /* MPLS header */
    socC3RceDataHeaderIpv4,         /* IPv4 header */
    socC3RceDataHeaderIpv6,         /* IPv6 header */
    socC3RceDataHeaderTcpUdp,       /* TCP or UDP header */
    socC3RceDataOffsetCount         /* Always Last. Not a usable value. */
} soc_c3_rce_data_header_t;
#define SOC_C3_RCE_DATA_HEADER_TYPE_NAMES \
    "Key", \
    "Metadata", \
    "Raw", \
    "Ether", \
    "VLAN", \
    "MPLS", \
    "IPv4", \
    "IPv6", \
    "TCP/UDP"

/*
 *  CONFIG:
 *
 *  If the header for a particular set of bits is 'metadata', the offset will
 *  specify which particular metadatum.
 *
 *  The customer can add to this list if desired.  It is fairly easy to imagine
 *  there are other metadata (even variants of the existing metadata) that
 *  might be useful in a given application.  If changing this list, the strings
 *  below it should also be updated.
 */
typedef enum soc_c3_rce_metadata_type_e {
    socC3RceMetadataInPortNum = 0,      /* ingress port number */
    socC3RceMetadataOutPortNum = 1,     /* egress port number */
    socC3RceMetadataCount               /* Always last, not a usable value */
} soc_c3_rce_metadata_type_t;
#define SOC_C3_RCE_METADATA_TYPE_NAMES \
    "InPortNum", \
    "OutPortNum"

/*
 *  Bit numbering within frames is based upon the existing BCM tendency toward
 *  ethernet bit ordering, which includes 'big endian' byte numbering. This
 *  means that the least significant bit of a byte is bit zero, the least
 *  significant byte of a multiple-byte field is byte zero.  This leads to bits
 *  being numbered in a way that seems to be seldom drawn once you get out of
 *  the IEEE Ethernet spec sheets...
 *
 *  Bit      Value
 *  ---    ---------
 *   0     2^0 =   1
 *   1     2^1 =   2
 *   2     2^2 =   4
 *   3     2^3 =   8
 *   4     2^4 =  16
 *   5     2^5 =  32
 *   6     2^6 =  64
 *   7     2^7 = 128
 *
 *  Bytes are numbered from zero in increasing value.  The bitwise position is
 *  noted by (byte * 8) + bit.  This leads to bit 0 being the *least*
 *  significant bit of byte 0, bit 7 being the *most* significant bit of byte
 *  0, bit 8 being the least significant bit of byte 1, and so on.
 *
 *  Bytes are concatenated in network byte order to form larger items, with the
 *  bits aligned so bit 0 of one byte is adjacent to bit 7 of the next byte,
 *  and increasing the byte number as we go along.  In a sense, looking at the
 *  byte stream as going from left to right across the page, the byte numbers
 *  are increasing.
 *
 *  A field from the frame is considered to start at its most significant bit,
 *  and continue toward less significant bits for the specified number of bits.
 *
 *  While, most of the time, the way this is numbered makes a little intuitive
 *  sense, it starts to become a little odd when dealing with fields that span
 *  multiple bytes.  For example, the first two bytes would start with bit 7
 *  and run for sixteen bits (7,6,5,4,3,2,1,0,15,14,13,12,11,10,9,8).  However,
 *  where it spans non-integral bytes, things seem even little stranger: For
 *  example, IPv6 DSCP+ECN bits 7,6,5,4,3,2,1,0 covers bits 3,2,1,0,15,14,13,12
 *  of the IPv6 header when noted in this manner, and it is noted by start of 3
 *  and length of 8 here.
 *
 *    byte 0           byte 1                 byte 2
 *   7,6,5,4,3,2,1,0  15,14,13,12,11,10,9,8  23,22,21,20,19,18,17,16
 *
 *    byte 3                   byte 4                   byte 5
 *   31,30,29,28,27,26,25,24  39,38,37,36,35,34,33,32  47,46,45,44,43,42,41,40
 *
 *  For the purpose of bit numbering within the key, we stick to the same bit
 *  numbering within each byte, but arrange the bytes differently.  For this
 *  purpose, the bytes are laid out going right to left across the page, so
 *  byte number increases toward more significant bytes.
 *
 *  Key fields are described as starting with their *least* significant bit,
 *  and running toward more significant bits for a number of bytes.  This,
 *  combined with the bit numbering, makes a little more intuitive sense when
 *  considering fields that span bytes -- even fractional bytes.  For example,
 *  a field that covers the upper 4 bits of byte 0 and the lower 4 bits of byte
 *  1 starts with bit 4 and runs for 8 bits: 4,5,6,7,8,9,10,11.
 *
 *    byte 5                   byte 4                   byte 3
 *   47,46,45,44,43,42,41,40  39,38,37,36,35,34,33,32  31,30,29,28,27,26,25,24
 *
 *    byte 2                   byte 1                 byte 0
 *   23,22,21,20,19,18,17,16  15,14,13,12,11,10,9,8  7,6,5,4,3,2,1,0
 *
 *  Fields from the frame are not reordered when they are put into the key.
 *  Assume destination MAC address (Ethernet header bit 7 for 48 bits) covers
 *  the low six bytes of the key (bit 0 for 48 bits).  The bytes are in the
 *  same order within the key as they were within the frame; only the specific
 *  byte numbers change.  So if the MAC address was 12:34:56:78:9A:BC, network
 *  byte 0 and key byte 5 are 12, network byte 1 and key byte 4 are 34, network
 *  byte 2 and key byte 3 are 56, network byte 3 and key byte 2 are 78, network
 *  byte 4 and key byte 1 are 9A, and network byte 5 and key byte 0 are BC.
 *
 *  To avoid some of the oddities of the two numbering schemes, we use a start,
 *  length method of describing a given field.  This at least avoids some of
 *  the intuitive oddities between them, and simplifies conversion.
 *
 *  These macros (SOC_C3_RCE_FRAME_BIT, SOC_C3_RCE_KEY_BIT) are intended merely
 *  to provide an easy way to express (byte,bit) rather than using the combined
 *  bit value.  Note that they do not munge the value nor do they swap bit or
 *  byte numbering.
 */
#define SOC_C3_RCE_FRAME_BIT(_byte, _bit) (((_byte) * 8) + (_bit))
#define SOC_C3_RCE_KEY_BIT(_byte, _bit) (((_byte) * 8) + (_bit))
#define SOC_C3_RCE_NO_BITS 0, 0

/*
 *  Maximum number of segments per qualifier in microcode descriptions of
 *  qualifiers is specified by SOC_C3_RCE_MAX_SEG_PER_QUALIFIER.  Normally,
 *  this would be 1, but some qualifiers (IPv6 addresses in particular) are
 *  long and get split due to their length, so should probably be at least 2.
 *  Qualifiers which span multiple segments will be handled in 'sparse' mode.
 *
 *  Maximum number of segments per action in microcode descripitions of actions
 *  is specified by SOC_C3_RCE_MAX_SEG_PER_ACTION.  At this time, this must be
 *  1 since support for larger numbers is not yet in place (would require
 *  'sparse' action support).
 *
 *  Neither of these should ever be zero or less.
 */
#define SOC_C3_RCE_MAX_SEG_PER_QUALIFIER 2
#define SOC_C3_RCE_MAX_SEG_PER_ACTION    1

/*
 *  Describe a header field, specifically for use in building a qualifier or
 *  creating a range.
 *
 *  This indicates from where a set of key bits is taken, and how many bits are
 *  so taken from that place.  Note that the bit ordering is as if the bits for
 *  each byte are laid out most significant on the left (bit 7) and least
 *  significant on the right (bit 0) and bytes are placed in increasing offset
 *  toward the right (so bit 7 of byte 1 is immediately right of bit 0 of byte
 *  0).  Fields are designated as starting at MSb and run contiguous bits
 *  toward the right for the specified number of bits.
 *
 *  fieldName points to a constant string provided by the application when it
 *  sets up a header field.  It is optional; the RCE driver does not use it for
 *  its internal work, but it is used for certain dump function, and it will be
 *  propagated to the qualifier by soc_c3_rce_program_qualifier_build.  The
 *  application retains ownership of this string, while the RCE driver keeps a
 *  copy internally (so once a group is created, the string space that was
 *  pointed to by the field names in the qualifiers can be reclaimed, but not
 *  until after the group is created).
 */
typedef struct soc_c3_rce_header_field_info_s {
    soc_c3_rce_data_header_t header;    /* header containing this field */
    int startBit;                       /* low bit of this field in the hdr */
    unsigned int numBits;               /* number of bits in this field */
    const char *fieldName;              /* pointer to name for field */
} soc_c3_rce_header_field_info_t;

/*
 *  This indicates a region of bits within the key.  Bits are arranged within
 *  bytes so the most significant bit (bit 7) is to the left, the least
 *  significant bit (bit 0) to the right.  Bytes are arranged in increasing
 *  order from right to left (so bit 0 of byte 1 is immediately left of bit 7
 *  of byte 0).  Fields are designated as starting at LSb and run contiguous
 *  bits toward the left (toward more significant bits) for the specified
 *  number of bits.
 */
typedef struct soc_c3_rce_start_len_s {
    uint16 startBit;                    /* low bit of this field in the key */
    uint16 numBits;                     /* number of bits this field */
} soc_c3_rce_start_len_t;

/*
 *  Indicates whence a group of bits in the key is extracted by the LRP
 *  microcode (which populates the key) and where those bits are placed within
 *  the key by the LRP microcode.  The origin should be used in building higher
 *  level constructs (bcmFieldQualify*, for example), while the placement is
 *  used in selecting fields for comparison.  The application chooses what
 *  comparison mechanism it wants for each field it chooses.
 *
 *  The RCE driver does not pay attention to this information; it is merely for
 *  the client's information.  The RCE driver takes the client's specification
 *  of interesting fields for the key from the configuration the client
 *  provides when creating groups.
 *
 *  The RCE driver does offer a helper function that will use this information
 *  to build qualifiers for groups, if the application chooses to use it.
 */
typedef struct soc_c3_rce_qual_uc_desc_s {
    soc_c3_rce_header_field_info_t hdr;/* header information for this field */
    soc_c3_rce_start_len_t loc[SOC_C3_RCE_MAX_SEG_PER_QUALIFIER]; /* key bits */
} soc_c3_rce_qual_uc_desc_t;

/*
 *  CONFIG:
 *
 *  Indicates what kind of action the LRP microcode will take with the
 *  particular value.  Various actions have different meanings.
 *
 *  Custom microcode is likely to offer different actions, so this list can be
 *  expanded to include more actions if desired.  If changing this list, the
 *  strings below it should also be updated.
 */
typedef enum soc_c3_rce_action_uc_type_e {
    socC3RceActionEnable,               /* enables another action */
    socC3RceActionNewVsi,               /* replace VSI for the frame */
    socC3RceActionNewFtIndex,           /* replace FT index for the frame */
    socC3RceActionMirror,               /* set mirror profile for the frame */
    socC3RceActionException,            /* set exception for the frame */
    socC3RceActionCopyToCpu,            /* copy the frame to the CPU */
    socC3RceActionDrop,                 /* discard the frame */
    socC3RceActionPolicer,              /* set the frame's policer */
    socC3RceActionNewPrio,              /* set priority values */
    socC3RceActionNewDp,                /* set DP value */
    socC3RceActionCounter,              /* update a counter */
    socC3RceActionCustom,               /* custom action */
    socC3RceActionCount                 /* not a valid ID; must be last */
} soc_c3_rce_action_uc_type_t;
#define SOC_C3_RCE_ACTION_UC_TYPE_NAMES \
    "Enable", \
    "NewVsi", \
    "NewFtIndex", \
    "Mirror", \
    "Exception", \
    "CopyToCpu", \
    "Drop", \
    "Policer", \
    "NewPrio", \
    "NewDp", \
    "Counter", \
    "Custom"

/*
 *  Provides information for a particular action; applies to all groups in a
 *  program.  The LRP microcode expects to take the RCE result and use it as an
 *  index into an action table, each of whose entries contains some number of
 *  fields each of which is described by one of these structs.  While the RCE
 *  driver manages the data, it does not particularly care about the actual
 *  underlying intent, but this exposes both to the application so it can make
 *  reasonable decisions about what an entry can do.
 *
 *  The RCE driver takes some of this information during init when it sets up
 *  programs, since each group in a program uses the same action table as all
 *  other groups in that program.  However, the RCE does not particularly care
 *  what kind of action it is -- this is only used to build the action set
 *  definition for each program.
 *
 *  Some actions can 'enable' others -- that is, control whether (or how!) they
 *  operate.  If an action has an enabler, that action's enableIndex is the
 *  index of that action's enabler in the array.  If there is no enabler, the
 *  enableIndex field should be set to (~0).
 *
 *  If an action does not have an 'enabler', then it probably has a value that
 *  effectively disables it (such as mirroring to mirror definition zero, which
 *  is normally defined as no mirror).  This is indicated by disableVal.
 *
 *  Sometimes more than one action overlays a set of bits.  Usually this is
 *  accomplished by using two separate enablers for that action -- each one
 *  indicting what specific intent is applied to the bits.  Alternatively, a
 *  multiple-bit enabler could specify a mode by which an action is applied.
 *
 *  The 'disable' value is also the 'default' value that will be applied to the
 *  particular action field when constructing the 'miss' action in an action
 *  table.  The intent is that whatever value goes here will not affect frames
 *  as they flow by (maybe also requiring an 'enable' field be set to its
 *  default value if there is one for the field).
 */
typedef struct soc_c3_rce_action_uc_desc_s {
    soc_c3_rce_action_uc_type_t action; /* what LRP does with this action */
    const char *actionName;             /* pointer to 'name' for action */
    uint16 enableIndex;                 /* action that controls this action */
    uint32 disableVal;                  /* value when disabled (no ena bit) */
    soc_c3_rce_start_len_t loc[SOC_C3_RCE_MAX_SEG_PER_ACTION]; /* which bits */
} soc_c3_rce_action_uc_desc_t;

/*
 *  Describe a 'qualifier'
 *
 *  For purposes here, a 'qualifier' is a collection of bits that will be
 *  programmed by the LRP microcode into the key registers before invoking an
 *  RCE program, plus the information about how those bits are to be examined
 *  by the RCE program in question.
 *
 *  While the key is common between all groups in an RCE program, the way the
 *  bits from the key are handled can vary between groups within a program,
 *  from just which bits are considered, to specifics about the way those bits
 *  are compared.  This capability allows better exposure of the RCE features
 *  to even customers who do not want to make many of their own changes.
 *
 *  Qualifiers are unique per group, so qualifier[0] for group 1 may not be the
 *  same as qualifier[0] for group 2.
 *
 *  Each qualifier type has various parameters.  The number of parameters may
 *  vary considerably as well.
 *
 *    type            parameters
 *    --------------  --------------------------------------------------
 *    prefix          low key bit, high key bit
 *    postfix         low key bit, high key bit
 *    masked          low key bit, high key bit
 *    exact           low key bit, high key bit
 *    prefix_sparse   each bit of the field in the key, LSb to MSb
 *    postfix_sparse  each bit of the field in the key, LSb to MSb
 *    masked_sparse   each bit of the field in the key, LSb to MSb
 *    exact_sparse    each bit of the field in the key, LSb to MSb
 *
 *  Note this structure does not include the param values, and so must be
 *  dynamically allocated, including the appropriate number of param values.
 *
 *  Notes on types:
 *
 *  'prefix' qualifiers are maskable, but the design is so that the mask is
 *  contiguous, and only the most significant bits are important (such as in
 *  specifying an IP subnet).
 *
 *  'postfix' qualifiers are maskable, but the design is so that the mask is
 *  contiguous, and only the least significant bits are important (such as
 *  considering the part of IPv6 host address that is often MAC address).
 *
 *  'masked' qualifiers are maskable, and the masked/selected bits can be
 *  arbitrarily chosen within the qualifier.
 *
 *  'exact' qualifiers are not maskable -- all bits of the qualifier must match
 *  the key exactly.  This is intended for use when some fields in the key can
 *  be different within the same program (such as if IPv4 addresses and IPv6
 *  addresses share bits, another bit can be used as an exact match qualifier
 *  to specify whether the addresses are IPv4 or IPv6.
 *
 *
 *  WARNING: While 'prefix', 'postfix', and 'masked' qualifiers can have bits
 *  that are not important to the comparison (thus match either one or zero),
 *  an 'exact' qualifier does not offer this feature -- all bits must match the
 *  pattern exactly.  Thus, the default state for 'prefix', 'postfix', and
 *  'masked' on a newly created entry will match any frame, but the default
 *  state for 'exact' will match only frames whose key bits for the 'exact'
 *  qualifier are all zeroes.  Entries MUST specify the value for 'exact'
 *  qualifiers if they do not want the assumption of zero to apply, and SHOULD
 *  always specify such fields even if they do want the default to apply.
 *
 *
 *  The C3 RCE is not like the C1/C2 RCE in that it lacks 'compare' opcodes and
 *  uses range predicates instead, so there is no 'compare' option here.  Also,
 *  it does not suport specifically 'all-or-nothing' fields; instead it
 *  provides two forms of masking (prefix/postfix and arbitrary).  Since the
 *  use of 'all-or-nothing' match fields on C1/C2 was for instruction
 *  efficiency, the nearest best thing here is to use prefix/postfix instead,
 *  except in the case of a single bit field, where 'masked' is better.
 */
typedef enum soc_c3_rce_qual_type_e {
    socC3RCEQualType_prefix,            /* prefix masked qualifier */
    socC3RCEQualType_postfix,           /* postfix masked qualifier */
    socC3RCEQualType_masked,            /* arbitrary masked qualifier */
    socC3RCEQualType_exact,             /* exact match qualifier */
    socC3RCEQualType_prefix_sparse,     /* sparse prefix masked qualifier */
    socC3RCEQualType_postfix_sparse,    /* sparse postfix masked qualifier */
    socC3RCEQualType_masked_sparse,     /* sparse arbitrary masked qualifier */
    socC3RCEQualType_exact_sparse,      /* sparse exact match qualifier */
    socC3RCEQualTypeCount               /* not valid; must be last */
} soc_c3_rce_qual_type_t;
#define SOC_C3_RCE_QUALIFIER_TYPE_NAMES \
    "prefix", \
    "postfix", \
    "masked", \
    "exact", \
    "sparse prefix", \
    "sparse postfix", \
    "sparse masked", \
    "sparse exact"
/*
 *  qualName points to a constant string provided by the application when it
 *  sets up a qualifier.  It is optional; the RCE driver does not use it for
 *  its internal work, but it is used for certain dump functions. The
 *  application retains ownership of this string, but must not free it or
 *  overwrite it until there are no pending references to it (such as ranges
 *  that use it or groups with qualifiers that use it but that have not yet
 *  been created).
 *
 *  qualName will be copied into the driver's memory when associated items are
 *  created (such as once a group is created, all of its qualifiers are copied
 *  internally to the driver state, or when creating a range, all of the
 *  information for that range will be copied to the driver state.  If it is
 *  NULL, the driver will treat it as a zero-length string.
 */
typedef struct soc_c3_rce_qual_desc_s {
    soc_c3_rce_qual_type_t qualType;    /* qualifier type (above) */
    const char *qualName;               /* pointer to name for qualifier */
    unsigned int paramCount;            /* number of parameters */
    int *param;                         /* pointer to individual parameters */
} soc_c3_rce_qual_desc_t;

/*
 *  Describe an action
 *
 *  For purposes here, an 'action' is nothing more than a set of bits in the
 *  action table for the program.  When running an RCE program, an index is
 *  returned according to the program settings and some data embedded in the
 *  RCE instructions, and that index looks up an entry in a table that will be
 *  used by the LRP microcode.  This describes an action.
 *
 *  Actions are common for all groups using a particular action table.
 *
 *  Since actions are basically just bitfields at this level, they are taken to
 *  higher-level constructs (like 'drop' or 'redirect') here, but those are
 *  simply defined so the code can provide basic manipulation for them as well,
 *  since it needs to keep up with them to properly manage tables.
 *
 *  Each qualifier type has various parameters.  The number of parameters may
 *  vary considerably as well.
 *
 *    type         parameters
 *    -----------  --------------------------------------------------
 *    bitfield     first bit, last bit
 *
 *  Note this structure does not include the param values, and so must be
 *  dynamically allocated, including the appropriate number of param values.
 *
 *  Notes on types:
 *
 *  'bitfield' actions specify a contiguous range of bits that comprise the
 *  data for the action.
 */
typedef enum soc_c3_rce_action_type_e {
    socC3RCEActionType_bitfield,        /* action is a bitfield */
    socC3RCEActionTypeCount             /* not valid; must be last */
} soc_c3_rce_action_type_t;
typedef struct soc_c3_rce_action_desc_s {
    soc_c3_rce_action_type_t actionType;/* action type (above) */
    unsigned int paramCount;            /* number of parameters */
    int *param;                         /* individual parameters */
} soc_c3_rce_action_desc_t;

/*
 *  Describe the entire unit.
 *
 *  This provides an overview of the entire unit.  Like the program
 *  description, below, this can only be read directly.
 *
 *  Program count does not vary across a run, nor do the maximum values.  Note
 *  that some max values duplicate max values found per program.  These are
 *  global; the max values per program still hold.  Also, it is possible for
 *  configurations to consume all available resources before reaching these
 *  values either for a given program or globally.
 *
 *  Values under 'accounting' are 'current' state and can change.
 */
typedef struct soc_c3_rce_unit_desc_s {
    /* limits */
    unsigned int programCount;          /* number of active programs */
    unsigned int actionTableCount;      /* number of action tables */
    unsigned int rangeMaxCount;         /* maximum number of ranges */
    unsigned int groupMaxCount;         /* maximum number of groups */
    unsigned int entryMaxCount;         /* maximum number of entries */
    /* accounting */
    unsigned int rangesInUse;           /* number of ranges in use */
    unsigned int groupsInUse;           /* number of groups in use */
    unsigned int entriesInUse;          /* number of entries in use */
} soc_c3_rce_unit_desc_t;

/*
 *  Describe a range
 *
 *  The hardware is capable of doing 'range compare' operations on fields of up
 *  to 16 bits (but only when left aligned within a 16 bit grain of the key).
 *  This has to be handled specially, and the BCM layer APIs treat it oddly as
 *  well, so we expose it here in something approaching that way...
 *
 *  Note that the refCount value is not entries but filter sets that use the
 *  range.  Basically, the intent of exposing this is to provide a flag to
 *  indicate whether the range can be destroyed (only when this value is zero).
 *
 *  validProgs is a bitmap indicating for which programs the range is valid.
 *  It is ignored on create (filled in internally) and returned by get.
 *
 *  The range is inclusive -- any value from lowerBound through upperBound is
 *  considered 'inside' the range, including lowerBound and upperBound.
 *
 *  Set SOC_C3_RCE_RANGE_FLAG_INVERT in the flags when creating the range to
 *  indicate its default behaviour is inverted (outside the range matches,
 *  inside misses).  Otherwise inside the range hits and outside misses.
 *
 *  SOC_C3_RCE_RANGE_FLAG_VALID is automatically set by the code and its state
 *  is only used for certain tracking purposes.
 */
#define SOC_C3_RCE_RANGE_FLAG_INVERT 0x00000001
#define SOC_C3_RCE_RANGE_FLAG_VALID  0x80000000
typedef struct soc_c3_rce_range_desc_s {
    uint32 rangeFlags;                          /* flags for this range */
    soc_c3_rce_header_field_info_t headerField; /* location of the range */
    int lowerBound;                             /* lower boundary */
    int upperBound;                             /* upper boundary */
    /* accounting */
    uint32 validProgs;                          /* valid in which programs */
    unsigned int refCount;                      /* references to the range */
} soc_c3_rce_range_desc_t;

/*
 *  Describe an 'action table'
 *
 *  Since the hardware supports more than one result per program, it is
 *  possible to have multiple action tables per program, with up to one per
 *  result register (four result registers).  A single group can use only one
 *  action table; multiple groups can share a single action table.
 *
 *  A program indicates which action table(s) would be applicable to each of
 *  its result registers, and these refer to action tables.  Each group selects
 *  which one of the program's result registers it will use, and this implies
 *  the action table that the group will use.
 *
 *  If an action table does not support action-indexed counters (or the driver
 *  is built without action-indexed counter support) the counterName field will
 *  be NULL.  If the driver is built with action-indexed counter support, and
 *  the action table supports action-indexed counters, the counter name will be
 *  the provided name for the counter segment.
 */
typedef struct soc_c3_rce_actiontable_desc_s {
    /* descriptions */
    char *tableName;                        /* action table name (string) */
    char *counterName;                      /* counter table name (string) */
    /* parameters */
    unsigned int entryBlocks;               /* capacity, in 768-entry blocks */
    unsigned int actionCount;               /* number of action fields */
    soc_c3_rce_action_uc_desc_t *actFields; /* action field descriptions */
    /* accounting */
    unsigned int entryBlocksUsed;           /* entry blocks in use */
} soc_c3_rce_actiontable_desc_t;

/*
 *  Describe a 'program'
 *
 *  In hardware, a program is a collection of filter sets, all chained
 *  together, and invoked by a single specific LRP 'key' opcode sequence, with
 *  results returned as an index to an action table.  Each filter set contains
 *  some number of rules that match data in the same specific way (which can
 *  vary between filter sets), but all of the filter sets share a single key
 *  format and action format.
 *
 *  In abstract here, a program is a collection of groups, each of which
 *  describes a set of qualifiers specifying interesting bits in the key and
 *  how those bits are to be matched; and groups contain some number of entries
 *  that each align to a unique offset in the program's action table.  In the
 *  context of some APIs, a program here might look like a 'stage'.
 *
 *  The RCE driver code correlates the entry/rule index to the space in the
 *  program's action memory, manages how qualifiers
 *
 *  The LRP microcode defines the key format for a program and the action
 *  format for a program.  In order to allow the caller to adapt to that, it is
 *  exposed through the API by this structure.  Some additional data about a
 *  program are also exposed here, but statistics are exposed elsewhere.
 *
 *  Note that while a group defines the qualifier index that will be used when
 *  setting up entries, the *program* defines the action index.  This is
 *  because the action format is common to an entire program, but each group
 *  can specify which key bits are interesting to it and how it wants them to
 *  be compared.
 *
 *  Items under accounting are 'current' values and can change.
 */
typedef struct soc_c3_rce_program_desc_s {
    uint32 tmuProg;                              /* LRP keys supported (bits) */
    unsigned int entryMaxCount;                  /* max entries this program */
    unsigned int groupMaxCount;                  /* max groups this program */
    unsigned int keyFieldCount;                  /* number of key fields */
    soc_c3_rce_qual_uc_desc_t *keyFields;        /* key field descriptions */
    uint8 actionTable[SOC_C3_RCE_RESULT_REGISTER_COUNT];/* acttbl ea res reg */
    /* accounting */
    unsigned int entriesInUse;                   /* entries in use this prog */
    unsigned int groupsInUse;                    /* groups in use this prog */
} soc_c3_rce_program_desc_t;

/*
 *  Describe a 'group'
 *
 *  A 'group' (concept borrowed from the BCM layer field APIs) is basically a
 *  number of filter sets within a single RCE program.  While within a program,
 *  the contents of the key will always be the same, within different groups,
 *  the way these bits are processed can vary.
 *
 *  There can be numerous groups within a program.  There can be numerous RCE
 *  filter sets within a group.  The maxFilterSets field allows a limit to be
 *  placed on the number of filter sets in the group, but it if it zero, the
 *  group is not thus limited (except by space and time).
 *
 *  Also, groups within a program are sorted into priority order according to
 *  the priority ordering rules defined at the BCM layer.  A group's filter
 *  sets will always be together within a program.
 *
 *  At this layer, the actions are opaque.  The per-entry action value 'blob'
 *  will be written to an entry in the appropriate memory for the program,
 *  according to settings established by the microcode.
 *
 *  When creating a group, the rangesPerFilterSet parameter specifies the
 *  *lower* limit of the number of ranges that will be available to entries in
 *  this group, per filter set.  The value of this indicates an additional
 *  number of NOPs that will be added to the group's filter set template, to be
 *  used for resolving range references.  Each range referenced (normal or
 *  inverted -- so a single range as both counts as two references even if to
 *  the same range) by at least one entry (any number of entries greater than
 *  zero in this context counts as a single reference) in a filter set will
 *  burn one of these reserved instructions.  Once all of these instructions
 *  are used, no new range references will be allowed in that filter set.  If
 *  all references to a range are removed, that instruction is reclaimed and
 *  can be used for another range or reference mode.
 *
 *  When getting the group data, the rangesPerFilterSet will reflect the actual
 *  number of supported ranges (the code will also use slack space for ranges).
 *
 *  Some fields in this struct are only used for retrieval of information about
 *  the group, and have no effect on other operations (for example, they are
 *  ignored when creating a group).  These fields are under accounting.
 */
typedef struct soc_c3_rce_group_desc_s {
    /* where to put the result (bitmaps); at least one bit must be set */
    uint8 resultLrp;                    /* LRP result registers to use */
    uint8 resultRce;                    /* RCE result registers to use */
    /* RCE supports numerous programs invoked by LRP */
    uint8 rceProgram;                   /* which program for this group */
    int groupPriority;                  /* group priority within the program */
    /* additional parameters for the group */
    unsigned int rangesPerFilterSet;    /* range space per filter set */
    unsigned int maxFilterSets;         /* limit on number of filter sets */
    /* the qualifiers used by this group */
    unsigned int qualCount;             /* number of qualifiers used */
    soc_c3_rce_qual_desc_t *qualData[SOC_C3_RCE_GROUP_QUAL_MAX]; /* qual data */
    /* accounting */
    unsigned int entriesInUse;          /* number of entries in use */
    unsigned int instrCount;            /* number of instruction in template */
    unsigned int filterSetCount;        /* number of filter sets in group */
} soc_c3_rce_group_desc_t;

/*
 *  Describe an 'entry'.
 *
 *  An 'entry' (name borrowed from the BCM layer field APIs -- many other
 *  matching APIs call the same concept a 'rule') is basically a set of values
 *  which are compared to a frame (according to the instructions in the entry's
 *  group), and an associated set of actions to be taken if the frame matches.
 *
 *  There can be many entries within a group.  In the C3 hardware, there can be
 *  many entries within a filter set (see the group description).  Each entry
 *  occurs within the group in order according to its relative priority.
 *
 *  The qualifier and action data are not included in this description.  Those
 *  can be retrieved through specific APIs; this structure is specifically for
 *  use in getting specific metadata about an entry.
 *
 *  SOC_C3_RCE_ENTRY_FLAG_MODIFIED set indicates that the entry has been
 *  modified since it was created or last installed (whichever is most recent).
 *  It is ignored when creating an entry.
 *
 *  SOC_C3_RCE_ENTRY_FLAG_INSTALLED set indicates that the entry has been
 *  installed to hardware.  Note that if SOC_C3_RCE_ENTRY_FLAG_MODIFIED is also
 *  set, the entry has changed since it was installed and the API state no
 *  longer reflects the state written to hardware.  It is ignored when creating
 *  an entry.
 */
typedef struct soc_c3_rce_entry_desc_s {
    uint32 entryFlags;                  /* entry flags */
    int groupId;                        /* group to which the entry belongs */
    int entryPriority;                  /* priority of the entry in the group */
} soc_c3_rce_entry_desc_t;
#define SOC_C3_RCE_ENTRY_FLAG_MODIFIED  0x00000001
#define SOC_C3_RCE_ENTRY_FLAG_INSTALLED 0x00000002

/*
 *  Several diagnostic features are supported, including dumping state of
 *  various objects (and optionally children).  These flags control how the
 *  dump functions operate.  They act as a bitmap so can be combined.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_IMEM_ALLOC set indicates that a unit dump is
 *  to include information about the imem allocator.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_ALLOC set indicates that a unit dump will
 *  include information about the allocators for each action table.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DFLT set indicates that a unit dump will
 *  include the default value for each action table.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DATA set indicates that a unit dump will
 *  include details about the action tables.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACTIONS set indicates that a unit dump will
 *  include the action tables supported by the unit.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_RANGES set indicates that a unit dump is to
 *  include the active ranges.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_PADDING set indicates that a unit dump is to
 *  include the list of padding filter sets.
 *
 *  SOC_C3_RCE_DUMP_UNIT_INCLUDE_PROGRAMS set indicates that a unit dump is
 *  also to include dumps of all programs in that unit.
 *
 *  SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_DFLTACT set indicates that a program dump
 *  is to include a list of the action tables used by that program.
 *
 *  SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_KEY set indicates that a program dump is to
 *  include a list of the fields in the key for that program.
 *
 *  SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_FSETS set indicates that a program dump is
 *  to include a list of active filter sets for that program.
 *
 *  SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_DISASM set indicates that a program dump is
 *  to include a disassembly listing of the program.
 *
 *  SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_DATA set indicates that a program dump
 *  is also to include dumps of all groups in that program.
 *
 *  SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_ID set indicates that a program dump is
 *  also to include a list of all groups in the program.
 *
 *  SOC_C3_RCE_DUMP_FSET_INCLUDE_RANGES set indicates a dump including filter
 *  sets is to include the ranges referenced by each filter set.
 *
 *  SOC_C3_RCE_DUMP_FSET_INCLUDE_ENTRIES set indicates a dump including filter
 *  sets is to include a list of the entries and their positions within each
 *  filter set.
 *
 *  SOC_C3_RCE_DUMP_GROUP_INCLUDE_DFLTPATT set indicates that a group dump is
 *  to include the 'default' pattern used when creating entries in the group.
 *
 *  SOC_C3_RCE_DUMP_GROUP_INCLUDE_QUALS set indicates that a group dump is to
 *  include a listing of the group's qualifiers.
 *
 *  SOC_C3_RCE_DUMP_GROUP_INCLUDE_DISASM set indicates that a group dump is to
 *  include a disassembly of the group's filter set template.
 *
 *  SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_ID set indicates that a group dump is
 *  also to include a list of the entry IDs in the group.
 *
 *  SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_DATA set indicates that a group dump is
 *  also to include dumps of all entries in that group.
 *
 *  SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTION set indicates that an entry dump is
 *  to include the hardware copy packed action values.
 *
 *  SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTION set indicates that an entry dump is
 *  to include the API copy packed action values.
 *
 *  SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_PATT set indicates that an entry dump is to
 *  include the hardware copy pattern values.
 *
 *  SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_PATT set indicates that an entry dump is to
 *  include the API copy pattern values.
 */
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_IMEM_ALLOC  0x80000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_ALLOC   0x40000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DFLT    0x20000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACT_DATA    0x10000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_ACTIONS     0x08000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_RANGES      0x04000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_PADDING     0x02000000
#define SOC_C3_RCE_DUMP_UNIT_INCLUDE_PROGRAMS    0x01000000
#define SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_KEY      0x00100000
#define SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_FSETS    0x00080000
#define SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_DISASM   0x00040000
#define SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_DATA 0x00020000
#define SOC_C3_RCE_DUMP_PROGRAM_INCLUDE_GRP_ID   0x00010000
#define SOC_C3_RCE_DUMP_FSET_INCLUDE_RANGES      0x00008000
#define SOC_C3_RCE_DUMP_FSET_INCLUDE_ENTRIES     0X00004000
#define SOC_C3_RCE_DUMP_GROUP_INCLUDE_DFLTPATT   0x00002000
#define SOC_C3_RCE_DUMP_GROUP_INCLUDE_QUALS      0x00001000
#define SOC_C3_RCE_DUMP_GROUP_INCLUDE_DISASM     0x00000800
#define SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_DATA   0x00000400
#define SOC_C3_RCE_DUMP_GROUP_INCLUDE_ENT_ID     0x00000200
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_RANGES     0x00000100
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTS     0x00000080
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTS     0x00000040
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_QUALS    0x00000020
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_QUALS    0x00000010
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_ACTION   0x00000008
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_ACTION   0x00000004
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_H_PATT     0x00000002
#define SOC_C3_RCE_DUMP_ENTRY_INCLUDE_A_PATT     0x00000001

/*
 *  These flags control entry copy behaviour.
 *
 *  If SOC_C3_RCE_ENTRY_COPY_SOURCE_QUALS is set, the copy will have the same
 *  qualifier settings as the source.  If not, the copy will have default
 *  values for its qualifiers (taken from the group defaults as if creating a
 *  new entry instead of copying an existing one).
 *
 *  If SOC_C3_RCE_ENTRY_COPY_SOURCE_ACTS is set, the copy will have the same
 *  action settings as the source.  If not, the copy will have default values
 *  for its actions (taken from the action table defaults as if creating a new
 *  entry instead of copying an existing one).
 *
 *  If SOC_C3_RCE_ENTRY_COPY_SOURCE_INST is set, and the source entry is
 *  installed, and SOC_C3_RCE_ENTRY_COPY_SOURCE_QUALS is set, the copy will be
 *  automatically installed once it is created.  If any of these is not true,
 *  the copy will not be installed once created (the caller will have to do
 *  this explicitly using soc_c3_rce_{entry|group}_install).
 *
 *  If SOC_C3_RCE_ENTRY_COPY_SOURCE_CTRS is set, the counter basis from the old
 *  entry is copied to the new entry.  If not, the copy's counter basis values
 *  will be zeroed.  This has no effect if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
 *  (compile-time switch in the RCE driver module itself) is FALSE.
 *
 *  NOTE: Copying an entry does not link action-indexed counters between the
 *  two copies -- it only makes a copy of the state; afterward, the entries
 *  count independently.
 *
 *  NOTE: SOC_C3_RCE_ENTRY_COPY_SOURCE_CTRS only affects action-indexed
 *  counters, and even in that case it merely copies the basis values for
 *  action-indexed counters (and does not link the counters between the
 *  entries); if a counter index is specified by a field in the actions,
 *  copying the actions will copy that counter index, and in that case, the
 *  entries will be counting together (since they share a counter index).
 *
 *  SOC_C3_RCE_ENTRY_COPY_SOURCE_ALL is a shorthand way to specify all of the
 *  flags at once instead of having to OR them together.  This is the typical
 *  intent to copy, so it most likely should be used unless there is specific
 *  reason to want to control the individual features above.
 */
#define SOC_C3_RCE_ENTRY_COPY_SOURCE_QUALS 0x00000001
#define SOC_C3_RCE_ENTRY_COPY_SOURCE_ACTS  0x00000002
#define SOC_C3_RCE_ENTRY_COPY_SOURCE_INST  0x00000004
#define SOC_C3_RCE_ENTRY_COPY_SOURCE_CTRS  0x00000008
#define SOC_C3_RCE_ENTRY_COPY_SOURCE_ALL   0x0000000F

/*
 *  Another diagnostic feature is access to several trace registers.  These
 *  registers can capture key and result values.
 *
 *  SOC_C3_RCE_RESULT_TRACE_ENABLE indicates the capture mechanism for results
 *  being sent to the LRP.  Must be cleared and then reset once results have
 *  been captured before the capture can occur again.
 *
 *  SOC_C3_RCE_RESULT_TRACE_FROM_KEY indicates the capture is meant to be
 *  triggered by a hit on the specified key.
 *
 *  SOC_C3_RCE_RESULT_TRACE_CAPTURED indicates that a result has been captured.
 *  It must be written 1 to clear before another capture can occur.
 *
 *  SOC_C3_RCE_KEY_TRACE_ENABLE indicates a key capture should be made.  It
 *  must be cleared before the key capture can occur again.
 *
 *  SOC_C3_RCE_KEY_TRACE_INTERRUPT indicates an interrupt should occur when a
 *  key capture happens.
 *
 *  SOC_C3_RCE_KEY_TRACE_HALT indicates the RCE should halt immediately once a
 *  key capture happens.  The RCE will need to be reset and reinitilaised at
 *  this point, so it should not generally be used.
 *
 *  SOC_C3_RCE_KEY_TRACE_CAPTURED indicates that a key has been captured.  It
 *  must be written 1 in order to clear the capture state.
 */
#define SOC_C3_RCE_RESULT_TRACE_ENABLE          0x00000001
#define SOC_C3_RCE_RESULT_TRACE_FROM_KEY        0x00000002
#define SOC_C3_RCE_RESULT_TRACE_CAPTURED        0x00000008
#define SOC_C3_RCE_KEY_TRACE_ENABLE             0x00000100
#define SOC_C3_RCE_KEY_TRACE_INTERRUPT          0x00000200
#define SOC_C3_RCE_KEY_TRACE_HALT               0x00000400
#define SOC_C3_RCE_KEY_TRACE_CAPTURED           0x00000800

/*
 *  Flags used by soc_c3_rce_state_check
 *
 *  If SOC_C3_RCE_STATE_CHECK_ABORT_ERRORS is set, the scan will be aborted on
 *  the first error encountered while accessing the hardware; if it is clear,
 *  the function will try to continue even after encountering errors.  Note
 *  that leaving SOC_c3_RCE_STATE_CHECK_ABORT_ERRORS clear does not prevent a
 *  fatal error (such as not enough memory) from aborting the scan.  Despite
 *  the setting, all errors will be logged as they are encountered.
 *
 *  If SOC_C3_RCE_STATE_CHECK_ABORT_MISMATCH is set, the scan will be aborted
 *  after the first state mismatch; if it is clear, the function will try to
 *  continue the scan even after encountering mismatches.  Despite this
 *  setting, all mismatches are logged as if they were errors.
 *
 *  If SOC_C3_RCE_STATE_CHECK_ABORT_INVALID is set, the scan will be aborted
 *  after the first invalid condition is detected; if it is clear, the function
 *  will try to continue the scan even after encountering invalid conditions.
 *  This specifically only considers conditions invalid for hardware; a driver
 *  level inconsistency is not 'invalid' for this flag, even if it is an
 *  invalid condition within the drier.  Despite this setting, all invalid
 *  conditions are logged as if they were errors.
 *
 *  If SOC_C3_RCE_STATE_CHECK_ABORT_CORRUPT is set, the scan will be aborted
 *  ater the first apparent corruption is detected.  Corruption, for this flag,
 *  includes invalid states within the driver data, but does not consider mere
 *  data mismatches as corruption.
 */
#define SOC_C3_RCE_STATE_CHECK_ABORT_ERRORS     0x00000001
#define SOC_C3_RCE_STATE_CHECK_ABORT_MISMATCH   0x00000002
#define SOC_C3_RCE_STATE_CHECK_ABORT_INVALID    0x00000004
#define SOC_C3_RCE_STATE_CHECK_ABORT_CORRUPT    0x00000008

/*
 *  Function
 *    soc_c3_rce_init
 *  Purpose
 *    Allocates resources used for managing RCE and sets up some parameters
 *    based upon information from the hardware descriptions.
 *  Arguments
 *    (IN) unit = unit number
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Resets hardware state, freeing any existing software state first.
 *
 *    If warm boot support is enabled, and SOC_WARM_BOOT(unit) is FALSE, this
 *    will also allocate and initialise the warm boot space.
 *
 *    If warm boot support is enabled, and SOC_WARM_BOOT(unit) is TRUE, this
 *    will act as a reload function and attempt to recreate the current state
 *    from the backing store copy after it has reloaded the information about
 *    the LRP microcode and its configuration.
 */
extern int
soc_c3_rce_init(int unit);

/*
 *  Function
 *    soc_c3_rce_detach
 *  Purpose
 *    Frees all resources used in managing the RCE
 *  Arguments
 *    (IN) unit = unit number
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Does not affect hardware state
 */
extern int
soc_c3_rce_detach(int unit);

/*
 *  Function
 *    soc_c3_rce_wb_immed_sync
 *  Purpose
 *    Force an immediate 'full sync' for the RCE driver state into its warm
 *    boot backing store.
 *  Arguments
 *    (in) unit = unit number
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If the warm boot feature is enabled, this will obliterate the current
 *    backing store state for the RCE driver on the specified unit, and then
 *    rebuild it from scratch.  If autosync is enabled, it will then commit the
 *    results to backing store; if autosync is disabled, it will then mark the
 *    backing store as dirty.
 *
 *    If the warm boot feature is disabled, this returns SOC_E_UNAVAIL.
 */
extern int
soc_c3_rce_wb_immed_sync(int unit);

/*
 *  Function
 *    soc_c3_rce_state_check
 *  Purpose
 *    Verify 'hardware' side of state against actual hardware.  This reads the
 *    current state back from hardware and compares it against the internal
 *    state.  This can not verify the 'API' side of the state.
 *  Arguments
 *    (in) unit = unit number
 *    (in) flags = flags controlling operation
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_FAIL if hardware mismatches expected state
 *      SOC_E_CONFIG if an invalid hardware condition is encountered
 *      SOC_E_INTERNAL if internal corruption in driver state is detected
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The 'API' side of the state is ignored, but this does verify state that
 *    affects hardware state (such as allocator state).
 *
 *    This can take a long time, because it reads from hardware -- settings,
 *    instructions + pattern, action tables.  For each filter set, it reads
 *    these data from hardware and then verifies the state against what it
 *    expects to be in place.
 *
 *    If some error occurs that prevents proper access to the hardware, that
 *    error will be reported, even if the function encounters reasons why it
 *    would return some other erorr to indicate mismatch, invalid state, or
 *    corrupt state.
 *
 *    Conditions that will trigger 'invalid' state:
 *      invalid initial filter set address for a program
 *      filter sets cross-linked within IMEM/PMEM
 *      initial instruction in a fitler set is not a start instruction
 *      any later instruction in a filter set is a start instruction
 *      initial filter set in program starts with 'startFilter' instruction
 *      later filter set in program starts with 'startProgram' instruction
 *      invalid next filter set address in hardware
 *      last active filter set in program is not end nor points to padding
 *      invalid opcode in program
 *      a filter set is too short (initial <5, later <4, in RCE clocks)
 *
 *    Conditions that will trigger 'corrupt' state:
 *      initial filter set of a group is not numbered zero
 *      subsequent filter sets in a group are not incrementing by one each
 *      entry 'install' state is not consistent between API and HW state
 *      two or more entries share the same position in a group
 *      entry position is not within correct range for its filter set
 *      hardware filter set length differs from software expected length
 *      hardware filter set length differs from allocated length
 *      group entry count disagrees with entries counted within group
 *      filter set entry count disagrees with entries counted within filter set
 *      stored range committed reference count not equal to counted value
 *      stored range reference count not equal to counted value
 *      action table segment not allocated as expected
 *      action table segment base differs between hardware and software
 *      filter set IMEM/PMEM allocation is not as expected
 *      program length in hardware disagrees with software expected length
 *      program padding in hardware disagrees with software expected padding
 *      filter set count in hardware disagrees with software expected count
 *      scanned group count in hardware disagrees with software expected count
 *      program group link chain not correctly terminated
 *      active group count disagrees with expected group count
 *      group entry link chain not correctly terminated scanned
 *      group entry count disagrees with expected group entry count
 *      total filter set entry count disagrees with expected total entry count
 *      total length of all programs disagrees with expected total length
 *      total filter set count disagrees with expected filter set count
 *      total group count disagrees with expected group count
 *      padding filter set returns a result to RCE or LRP result registers
 *      padding filter set base result not as expected
 *      padding filter set instruction not as expected
 *      padding filter set pattern byte not zero
 *      padding filter set length not as expected
 *
 *    Conditions that will trigger 'mismatch' state:
 *      program is enabled in hardware but not software
 *      initial filter set is padding in a program that contains group(s)
 *      filter set start address disagrees between hardware and software
 *      'next' filter set start address disagrees between HW and SW
 *      LRP result register for filter set disagrees between HW and SW
 *      RCE result register for filter set disagrees between HW and SW
 *      range instruction in HW not as expected from SW state
 *      template instruction in HW does not match SW template
 *      an entry is enabled in software but not in hardware
 *      an entry is disabled in software but not in hardware
 *      an entry pattern byte mismatches between HW and expected HW value
 *      an entry 'extra' pattern byte in hardware is not zero
 *      an entry action byte mismatches between HW and expected HW value
 */
extern int
soc_c3_rce_state_check(int unit, uint32 flags);

/*
 *  Function
 *    soc_c3_rce_info_get
 *  Purpose
 *    Provide information about the entire unit
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) unitInfo = where to put the pointer to the info
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Caller owns returned memory cell but must free it using the
 *    soc_c3_rce_info_free function below.
 */
extern int
soc_c3_rce_info_get(int unit,
                    soc_c3_rce_unit_desc_t **unitInfo);

/*
 *  Function
 *    soc_c3_rce_info_free
 *  Purpose
 *    Free information about the entire unit
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) unitInfo = pointer to the info to free
 *  Returns
 *    (nothing)
 *  Notes
 *    Must be used to free the unit information provided by the
 *    soc_c3_rce_info_get function, above.
 */
extern void
soc_c3_rce_info_free(int unit,
                     soc_c3_rce_unit_desc_t *unitInfo);

/*
 *  Function
 *    soc_c3_rce_dump
 *  Purpose
 *    Dump information about the entire unit
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) flags = flags for the dump
 *    (IN) prefix = prefix string
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Uses bsl_printf to display the information being dumped.
 */
extern int
soc_c3_rce_dump(int unit,
                uint32 flags,
                const char *prefix);

/*
 *  Function
 *    soc_c3_rce_debug_capture_parse
 *  Purpose
 *    Get the captured key (if any) and use the debug configuration to figure
 *    out which entries (if any) are referred to by the result capture
 *    registers (when there are captured results).
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) flags = flags indicating capture state
 *    (OUT) keyData = where to put key data
 *    (OUT) programId = where to put ID of program whose data were captured
 *    (OUT) entryIds = where to put array of results
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The key is SOC_C3_RCE_KEY_BIT_MAXIMUM+33 (544) bits, returned like the
 *    qualifiers from least significant byte to most significant byte.
 *
 *    entryIds array is SOC_C3_RCE_RESULT_REGISTER_COUNT long.
 *
 *    Of the key, the first 432 bits are from the captured key.  64 bits are
 *    then skipped (the key capture does not also capture the RCE internal
 *    result fields and there seems to be no way to capture them on current
 *    hardware).  The next 16 bits are taken from the current value of the
 *    KEY_GLOBAL field in RC_KEYMEM_CONFIG, with the low 4 of those bits set
 *    according to how the hardware 'should' always set them.  To those 512
 *    bits we also throw in an extra 32 bits, the value of the trace point
 *    capture register.  Excluding the addidional 32 bits for the trace point
 *    capture register, this should mirror the key format exactly, except that
 *    the place where the real key would have the RCE internal results is
 *    filled with zeroes.
 *
 *    The additional 32b for the trace point capture information contains:
 *       0..10  key tag
 *        11    key first
 *        12    run
 *      13..16  program ID
 *      17..31  reserved
 *
 *    Most of the additional data in the trace point capture information is
 *    used internally to the hardware, but the program ID is probably important
 *    in order to understand the key format and is definitely important in
 *    parsing the results capture (if the results capture was triggered by the
 *    key capture mechanism).
 *
 *    This returns -1 in the entry ID array for a result that does not map to a
 *    valid entry ID, otherwise it returns the entry ID.  Under typical use,
 *    all of these being -1 indicates that a miss was captured; at least one of
 *    them should be a valid entry ID if a hit was captured.  This function
 *    deals with getting the program ID from either the trace point capture
 *    data or from the results capture configuration as appropriate.
 *
 *    Output data will be clobbered even on errors.
 *
 *    This resets the capture status, if possible.  If a capture was made in a
 *    mode where halt will be asserted after the capture, it is not recoverable
 *    and the RCE must be reset.
 */
extern int
soc_c3_rce_debug_capture_parse(int unit,
                               uint32 *flags,
                               uint8 *keyData,
                               int *programId,
                               int *entryIds);

/*
 *  Function
 *    soc_c3_rce_debug_capture_set
 *  Purpose
 *    Set the debug capture configuration.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) flags = flags indicating desired capture state
 *    (IN) threshold = key capture threshold value
 *    (IN) keyData = data values for key capture
 *    (IN) keyMask = mask values for key capture
 *    (IN) programId = program whose results are to be captured
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Sets up the hardware assists for capturing results and keys according to
 *    the provided flags and arguments.
 *
 *    If SOC_C3_RCE_RESULT_TRACE_ENABLE but not
 *    SOC_C3_RCE_RESULT_TRACE_FROM_KEY, then programId must be a valid program
 *    ID and indicates which program's results to capture.
 *
 *    If SOC_C3_RCE_RESULT_TRACE_ENABLE and SOC_C3_RCE_RESULT_TRACE_FROM_KEY,
 *    the result capture will be triggered by the key capture, so programId
 *    specified here will be ignored.
 *
 *    If SOC_C3_RCE_KEY_TRACE_ENABLE, then keyData and keyMask must be
 *    provided, so the key capture knows what to capture.  Masking is allowed.
 *
 *    If not SOC_C3_RCE_KEY_TRACE_ENABLE, the keyData and keyMask will be
 *    ignored, so they may be NULL.  If either (or both) is NULL under any
 *    other condition, it will be interpreted as the trace is to match any
 *    possible values.
 *
 *    If SOC_C3_RCE_KEY_TRACE_HALT, the RCE will enter a halted state
 *    immediately after the capture, and will require reset before it will
 *    function again.  Normally, this flag will NOT be used.
 *
 *    The mask behaviour here corresponds to the BCM interpretration, where a 1
 *    indicates data bit of intereset and a 0 indicates a data bit that does
 *    not matter.  The hardware flips this meaning; this is handled here.
 *
 *    The soc_c3_rce_debug_capture_parse function, above, describes the key
 *    format.  The key data and mask here use that format.  This function will
 *    return an error if you try to make fields not available in the hardware
 *    important (such as the RCE internal result registers).
 *
 *    This will reset the capture status after it is done setting values.
 */
extern int
soc_c3_rce_debug_capture_set(int unit,
                             uint32 flags,
                             uint32 threshold,
                             const uint8 *keyData,
                             const uint8 *keyMask,
                             int programId);

/*
 *  Function
 *    soc_c3_rce_debug_capture_get
 *  Purpose
 *    Get the debug capture configuration.
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) flags = where to put capture state flags
 *    (OUT) threshold = where to put key capture threshold value
 *    (OUT) threshCount = where to put current count toward threshold
 *    (OUT) keyData = where to put data values for key capture
 *    (OUT) keyMask = where to put mask values for key capture
 *    (OUT) programId = where to put program ID for result capture
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Gets the settings of the hardware assists for capturing results and keys.
 *
 *    See soc_c3_rce_debug_capture_set for more notes and descriptions.
 *
 *    This does not reset the capture state, nor does it read it outside of the
 *    state that is reported by the flags.  Use soc_c3_rce_debug_capture_parse
 *    to get the current capture information.
 */
extern int
soc_c3_rce_debug_capture_get(int unit,
                             uint32 *flags,
                             uint32 *threshold,
                             uint32 *threshCount,
                             uint8 *keyData,
                             uint8 *keyMask,
                             int *programId);

/*
 *  Function
 *    soc_c3_rce_result_hit_counter_read
 *  Purpose
 *    Read the value of one of the result hit counters
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) hitCounter = which hit counter to read
 *    (OUT) value = where to put the counter value
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Reads and resets the requested hit counter.
 *
 *    Hit counters 0..SOC_C3_RCE_RESULT_HIT_COUNTER_COUNT are the result range
 *    based hit counters.  Hit counter SOC_C3_RCE_RESULT_HIT_ANY_COUNTER is the
 *    'any hit' counter, and hit counter SOC_C3_RCE_RESULT_MISS_COUNTER is the
 *    'any miss' counter.
 *
 *    Note that the 'any hit' counter is incremented for each result that is
 *    not the default value, and the 'any miss' counter is incremented for each
 *    result that is the default value, and that there are *four* results
 *    possible for each search (so if a search normally returns one result on
 *    hit, the 'any hit' counter would go up one and the 'any miss' counter
 *    would go up three for such a hit (or would go up four for a miss).
 */
extern int
soc_c3_rce_result_hit_counter_read(int unit,
                                   int hitCounter,
                                   uint32 *value);

/*
 *  Function
 *    soc_c3_rce_result_hit_counter_set
 *  Purpose
 *    Set the entries included by a particular hit counter
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) hitCounter = which hit counter to configure
 *    (IN) entryId0 = entry at one end of the set to include
 *    (IN) entryId1 = entry at the other end of the set to include
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Sets the range of entries that will be considered by a particular hit
 *    counter.
 *
 *    The hardware actually uses the result value to trigger the hit counters.
 *    In order to have the behaviour make some reasonable kind of sense,
 *    therefore, the range of entries to be considered must be within the same
 *    filter set.  This is a little more strict than simply being in the same
 *    group, since (in theory) a group can span filter sets, but is required
 *    filter sets within a given group are not guaranteed to have contiguous
 *    blocks of result values.
 *
 *    Note the entries are considered sorted in priority order (the entry
 *    traverse API walks entries in this order) for the purposes of defining
 *    the range, so hits to all entries in the traversal from entryId0 through
 *    entryId1 (inclusive of both entryId0 and entryId1) will be counted.
 *
 *    The order for entryId0 and entryId1 does not matter here.  This function
 *    automatically sorts them so the range spans from the entry with the lower
 *    result value (higher priority) to the entry with the higher result value
 *    (lower priority).
 *
 *    WARNING: It is possible that any operation that adds/updates any entry to
 *    the hardware will invalidate the mapping between result and entry, since
 *    filter sets need to be moved around for certain updates.  If such an
 *    update occurs and the results are still important, this must be called
 *    again to update the internal state for the counter so it once again
 *    reflects the desired entries.
 *
 *    WARNING: Since the hardware matches based upon result (which is merely an
 *    index into an action table that is specific per program), it is entirely
 *    possible that entries in another program will also match.
 *
 *    This does not allow setting of the 'all hit' or 'all miss' counters.
 *
 *    An error from this function may result in the requested counter's state
 *    being inconsistent.
 */
extern int
soc_c3_rce_result_hit_counter_set(int unit,
                                  int hitCounter,
                                  int entryId0,
                                  int entryId1);

/*
 *  Function
 *    soc_c3_rce_program_next_existing
 *  Purpose
 *    Given a program ID, find the next one that exists.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) currProgramId = starting point in the program ID space
 *    (OUT) nextProgramId = where to put next existing program ID if found
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The current program ID does not need to be an existing program; the
 *    search will start with the program ID immediately subequent to
 *    currProgramId.
 *
 *    If the current program ID is not valid (outside of valid program ID space
 *    for the unit), the search will begin with program ID zero.
 *
 *    If no existing program is encountered before exhausing the remainder of
 *    the program ID space, will return SOC_E_NOT_FOUND.
 */
extern int
soc_c3_rce_program_next_existing(int unit,
                                 int currProgramId,
                                 int *nextProgramId);

/*
 *  Function
 *    soc_c3_rce_program_info_get
 *  Purpose
 *    Expose information about a given program to the caller, so as to provide
 *    the information necessary to (hopefully) generically accomplish most of
 *    the required objectives of the API.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) program = program number
 *    (OUT) programInfo = where to put a pointer to the program data
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This does not provide a pointer to the actual internal tables, which are
 *    considered read-only by the code.  Instead, it copies the data into a new
 *    buffer and returns the pointer to that buffer.  At return, the caller is
 *    considered to own the buffer, and must call soc_c3_rce_program_info_free
 *    to dispose of the buffer (which may actually be comprised of more than
 *    one heap cell or similar).
 *
 *    The caller is expected to keep the program information as long as it will
 *    be needed (either long enough to build its own internal tables or for the
 *    duration of the run if the program data will be consulted directly),
 *    rather than to request and destroy it multiple times during a single run.
 */
extern int
soc_c3_rce_program_info_get(int unit,
                            int programId,
                            soc_c3_rce_program_desc_t **programInfo);

/*
 *  Function
 *    soc_c3_rce_program_info_free
 *  Purpose
 *    Dispose of a program description that is no longer needed, for example,
 *    when shutting down.
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) programInfo = a pointer to the program data
 *  Returns
 *    (nothing)
 *  Notes
 *    The caller is expected to keep the program information as long as it will
 *    be needed (either long enough to build its own internal tables or for the
 *    duration of the run if the program data will be consulted directly),
 *    rather than to request and destroy it multiple times during a single run.
 *
 *    This function must only be used to destroy program information that was
 *    created by the soc_c3_rce_program_info_get function.
 */
extern void
soc_c3_rce_program_info_free(int unit,
                             soc_c3_rce_program_desc_t *programInfo);

/*
 *  Function
 *    soc_c3_rce_program_traverse
 *  Purpose
 *    Walk through all of the programs on a unit, calling a caller-provided
 *    function for each program.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) callback = pointer to function to call
 *    (IN) extra = pointer to caller's additional data
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If the callback function returns any value other than SOC_E_NONE, it will
 *    abort the traverse.
 */
typedef int (*soc_c3_rce_program_traverse_cb_t)(int unit,
                                                int programId,
                                                void *extras);
extern int
soc_c3_rce_program_traverse(int unit,
                            soc_c3_rce_program_traverse_cb_t callback,
                            void *extras);

/*
 *  Function
 *    soc_c3_rce_program_dump
 *  Purpose
 *    Dump information about an entire program
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) flags = flags for the dump
 *    (IN) prefix = prefix string
 *    (IN) programId = which program to dump
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Uses bsl_printf to display the information being dumped.
 */
extern int
soc_c3_rce_program_dump(int unit,
                        uint32 flags,
                        const char *prefix,
                        int programId);

/*
 *  Function
 *    soc_c3_rce_program_qualifier_build
 *  Purpose
 *    Build a qualifier by scanning a program's key for a specified field
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) programId = which program to dump
 *    (IN/OUT) headerField = pointer to header field information
 *    (IN) qualType = what qualifier type is desired
 *    (OUT) qualDesc = where to put pointer to the constructed qualifier
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This builds a qualifier description of the requested type, if possible,
 *    that represents the requested frame field or metadatum, if available in
 *    the program's key.
 *
 *    The resulting qualifier description is owned by the caller.  It can be
 *    used to build (a) group(s) as needed, and then disposed of by the call
 *    below, soc_c3_rce_program_qualifier_free.
 *
 *    If the requested type implies a contiguous field in the key, and this is
 *    not possible (the key field is not contiguous), but it is possible to use
 *    a non-contiguous field instead, this will automatically use the
 *    non-contiguous form.  Similar for non-contiguous forms -- if it would be
 *    valid to express the field contiguously, that form will be used.
 *
 *    If a metadata field is requested but the requested bit count is zero, the
 *    resulting qualifier will be for the entire metadata field, and the
 *    headerField descriptor will be updated to indicate the correct length.
 *
 *    In any other case, a zero bit count is not valid.
 *
 *    This function does not support offsetting metadata fields; if the
 *    requested length is nonzero but is shorter than the entire field, the
 *    resulting qualifier will cover the LSbs of the field.
 *
 *    Because SOC_E_UNAVAIL would be traditional for return in case of an
 *    unsupported field (see the BCM field APIs), this returns SOC_E_UNAVAIL if
 *    it is unable to find any of the requested bits in the program's key.
 *
 *    At this time, this function will not combine fields.  This means that if
 *    the key has IPv4 source and IPv4 destination as separate fields in the
 *    key, they can not be combined into a single qualifier.
 */
extern int
soc_c3_rce_program_qualifier_build(int unit,
                                   int programId,
                                   soc_c3_rce_header_field_info_t *headerField,
                                   soc_c3_rce_qual_type_t qualType,
                                   soc_c3_rce_qual_desc_t **qualDesc);

/*
 *  Function
 *    soc_c3_rce_program_qualifier_free
 *  Purpose
 *    Free a qualifier description built by soc_c3_rce_program_qualifier_build
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) qualDesc = pointer to the constructed qualifier to free
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This must be used to free qualifier descriptions constructed by the
 *    soc_c3_rce_program_qualifier_build call.  It must not be used to free
 *    qualifier descriptions built in other ways.
 */
extern void
soc_c3_rce_program_qualifier_free(int unit,
                                  soc_c3_rce_qual_desc_t *qualDesc);

/*
 *  Function
 *    soc_c3_rce_program_scan
 *  Purpose
 *    Scan a specific program, given a key, for the entry/entries that would be
 *    hit if that key were provided by the LRP to the RCE.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) programId = program ID
 *    (IN) keyData = pointer to key data (see notes)
 *    (OUT) hitEntries = pointer to hit entry array (see notes)
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The keyData argument is expected to point to a full 432-bit key value, in
 *    the arrangement as it would be provided by the LRP, except that the key
 *    is stored as bytes in little-endian order (byte 0 = bits 0..7 of the key,
 *    byte 1 = bits 8..16 of the key, through byte 53 = bits 424..431).
 *
 *    The hitEntries argument points to an array of four integers, one for each
 *    result register.  If a result register is not filled by the program,
 *    whether due to the program not providing a result in that register, or
 *    due to no entry that would be using that result register matching, the
 *    corresponding entry in the array will be negative.  If an entry does
 *    match the key, the corresponding hitEntries element will contain that
 *    entry's ID.
 *
 *    A miss on all results is not considered an error.
 */
extern int
soc_c3_rce_program_scan(int unit,
                        int programId,
                        const uint8 *keyData,
                        int *hitEntries);

/*
 *  Function
 *    soc_c3_rce_actiontable_info_get
 *  Purpose
 *    Get information about a particular action table (program information only
 *    points to action tables, since each program can have up to four distinct
 *    action tables associated (one per result register) and it is possible
 *    that more than one program could use a single action table.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) actionTable = action table number
 *    (OUT) actionTableInfo = where to put pointer to action table information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The soc_c3_rce_actiontable_info_free function must be used to dispose of
 *    the data this returns, once the caller no longer has use for it.
 */
extern int
soc_c3_rce_actiontable_info_get(int unit,
                                unsigned int actionTable,
                                soc_c3_rce_actiontable_desc_t **actionTableInfo);

/*
 *  Function
 *    soc_c3_rce_actiontable_info_free
 *  Purpose
 *    Dispose of the action table information that was allocated and filled in
 *    by a call to soc_c3_rce_actiontable_info_get.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) actionTableInfo = pointer to the action table information
 *  Returns
 *    (nothing)
 *  Notes
 *    The caller is expected to keep the action table information around as
 *    long as it will be needed, but must dispose of it with this function once
 *    it is no longer needed.
 */
extern void
soc_c3_rce_actiontable_info_free(int unit,
                                 soc_c3_rce_actiontable_desc_t *actionTableInfo);

/*
 *  Function
 *    soc_c3_rce_group_first_avail
 *  Purpose
 *    Return the first available group ID (in case the application does not
 *    want to manage group IDs itself)
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) firstAvailGroupId = where to put the first available group ID
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Provides the first available group ID at the moment (subject to change as
 *    groups are created / destroyed).  Will provide a group ID that is valid
 *    for the device.  If there are no available valid group IDs, will return
 *    SOC_E_RESOURCE.
 */
extern int
soc_c3_rce_group_first_avail(int unit,
                             int *firstAvailGroupId);

/*
 *  Function
 *    soc_c3_rce_group_next_existing
 *  Purpose
 *    Given a group ID, find the next one that exists.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) currGroupId = starting point in the group ID space
 *    (OUT) nextGroupId = where to put next existing group ID if found
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The current group ID does not need to be an existing group; the search
 *    will start with the group ID immediately subequent to currGroupId.
 *
 *    If the current group ID is not valid (outside of valid group ID space for
 *    the unit), the search will begin with group ID zero.
 *
 *    If no existing group is encountered before exhausing the remainder of the
 *    group ID space, will return SOC_E_NOT_FOUND.
 */
extern int
soc_c3_rce_group_next_existing(int unit,
                               int currGroupId,
                               int *nextGroupId);

/*
 *  Function
 *    soc_c3_rce_group_info_get
 *  Purpose
 *    Get information about a group.  This retrieves the information about the
 *    group that was specified by the caller when creating the group.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = group ID
 *    (OUT) groupInfo = where to put the pointer to the group information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This does not provide a pointer to the actual internal tables, which are
 *    considered read-only by the code.  Instead, it copies the data into a new
 *    buffer and returns the pointer to that buffer.  At return, the caller is
 *    considered to own the buffer, and must call soc_c3_rce_group_info_free to
 *    dispose of the buffer (which may actually be comprised of more than one
 *    heap cell or similar).
 *
 *    This is generally intended as a way to recover state under certain
 *    conditions.  The application is expected to keep track of the group
 *    information it needs rather than querying this function regularly.
 */
extern int
soc_c3_rce_group_info_get(int unit,
                          int groupId,
                          soc_c3_rce_group_desc_t **groupInfo);

/*
 *  Function
 *    soc_c3_rce_group_info_free
 *  Purpose
 *    Free information about a group that is no longer needed.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = group ID
 *    (OUT) groupInfo = a pointer to the group information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function must be used to release group information that was created
 *    by the soc_c3_rce_group_info_get, and only to release such information.
 */
extern void
soc_c3_rce_group_info_free(int unit,
                           soc_c3_rce_group_desc_t *groupInfo);

/*
 *  Function
 *    soc_c3_rce_group_traverse
 *  Purpose
 *    Walk through all of the groups in a program on a unit, calling a
 *    caller-provided function for each group.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) programId = which program to scan
 *    (IN) callback = pointer to function to call
 *    (IN) extra = pointer to caller's additional data
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If the callback function returns any value other than SOC_E_NONE, it will
 *    abort the traverse.
 */
typedef int (*soc_c3_rce_group_traverse_cb_t)(int unit,
                                              int programId,
                                              int groupId,
                                              void *extras);
extern int
soc_c3_rce_group_traverse(int unit,
                          int programId,
                          soc_c3_rce_group_traverse_cb_t callback,
                          void *extras);

/*
 *  Function
 *    soc_c3_rce_group_dump
 *  Purpose
 *    Dump information about a group
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) flags = flags for the dump
 *    (IN) prefix = prefix string
 *    (IN) groupId = which group to dump
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Uses bsl_printf to display the information being dumped.
 */
extern int
soc_c3_rce_group_dump(int unit,
                      uint32 flags,
                      const char *prefix,
                      int groupId);

/*
 *  Function
 *    soc_c3_rce_group_create
 *  Purpose
 *    Creates a group based upon the requested group specification
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = proposed ID for the new group
 *    (IN) groupInfo = pointer to group specification
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Adds a filter set for the group (but no entries).
 *
 *    If there is already a group with the proposed ID, this will return
 *    SOC_E_EXISTS.  If the group ID is outside of the supported range, it will
 *    return SOC_E_BADID.  The application is encouraged to either manage group
 *    IDs itself or to use the soc_c3_rce_group_next_avail call to request the
 *    next available group ID when creating groups.
 */
extern int
soc_c3_rce_group_create(int unit,
                        int groupId,
                        const soc_c3_rce_group_desc_t *groupInfo);


/*
 *  Function
 *    soc_c3_rce_group_destroy
 *  Purpose
 *    Destroys an exiting group in an existing program
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = group ID to be destroyed
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If the group has any entries, this will also destroy them.
 */
extern int
soc_c3_rce_group_destroy(int unit,
                         int groupId);

/*
 *  Function
 *    soc_c3_rce_group_install
 *  Purpose
 *    Installs the group's entries to hardware
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = group to install
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Marks all entries in the group as active and then copies API state
 *    qualifier/action data to the hardware qualifer/action space, then updates
 *    all filter sets of the group to hardware.
 */
extern int
soc_c3_rce_group_install(int unit,
                         int groupId);

/*
 *  Function
 *    soc_c3_rce_group_remove
 *  Purpose
 *    Removes the group's entries from hardware
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = group to remove
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Marks the group's entries as inactive in API state, hardware state, and
 *    pattern memory.  No other changes are made, since with the entries
 *    disabled, the rest of its pattern/action state is not important.
 */
extern int
soc_c3_rce_group_remove(int unit,
                        int groupId);

/*
 *  Function
 *    soc_c3_rce_group_compress
 *  Purpose
 *    Attempt to reduce the number of filter sets used by a group by removing
 *    gaps between entries in filter sets.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = group to compress
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This will attempt to pack the entries into fewer filter sets by moving
 *    all of the entries in a filter set to the lowest positions in that filter
 *    set, then moving any entries from the next filter set that will fit into
 *    the filter set, and repeating the process for later filter sets.  If a
 *    filter set is completely emptied by the process, it will be removed.
 */
extern int
soc_c3_rce_group_compress(int unit,
                          int groupId);

/*
 *  Function
 *    soc_c3_rce_entry_first_avail
 *  Purpose
 *    Return the first available entry ID (in case the application does not
 *    want to manage entry IDs itself)
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) firstAvailEntryId = where to put the first available entry ID
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Provides the first available entry ID at the moment (subject to change as
 *    groups are created / destroyed).  Will provide an entry ID that is valid
 *    for the device.  If there are no available valid entry IDs, will return
 *    SOC_E_RESOURCE.
 */
extern int
soc_c3_rce_entry_first_avail(int unit,
                             int *firstAvailEntryId);

/*
 *  Function
 *    soc_c3_rce_entry_next_existing
 *  Purpose
 *    Given a entry ID, find the next one that exists.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) currEntryId = starting point in the entry ID space
 *    (OUT) nextEntryId = where to put next existing entry ID if found
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The current entry ID does not need to be an existing entry; the search
 *    will start with the entry ID immediately subequent to currEntryId.
 *
 *    If the current entry ID is not valid (outside of valid entry ID space for
 *    the unit), the search will begin with entry ID zero.
 *
 *    If no existing entry is encountered before exhausing the remainder of the
 *    entry ID space, will return SOC_E_NOT_FOUND.
 */
extern int
soc_c3_rce_entry_next_existing(int unit,
                               int currEntryId,
                               int *nextEntryId);

/*
 *  Function
 *    soc_c3_rce_entry_info_get
 *  Purpose
 *    Get information about a entry.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID
 *    (OUT) entryInfo = where to put the pointer to the entry information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This does not provide a pointer to the actual internal tables, which are
 *    considered read-only by the code.  Instead, it copies the data into a new
 *    buffer and returns the pointer to that buffer.  At return, the caller is
 *    considered to own the buffer, and must call soc_c3_rce_group_info_free to
 *    dispose of the buffer (which may actually be comprised of more than one
 *    heap cell or similar).
 *
 *    This is generally intended as a way to recover state under certain
 *    conditions.  The application is expected to keep track of the entry
 *    information it needs rather than querying this function regularly.
 */
extern int
soc_c3_rce_entry_info_get(int unit,
                          int entryId,
                          soc_c3_rce_entry_desc_t **entryInfo);

/*
 *  Function
 *    soc_c3_rce_entry_info_free
 *  Purpose
 *    Free information about a entry that is no longer needed.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID
 *    (OUT) entryInfo = a pointer to the entry information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function must be used to release entry information that was created
 *    by the soc_c3_rce_entry_info_get, and only to release such information.
 */
extern void
soc_c3_rce_entry_info_free(int unit,
                           soc_c3_rce_entry_desc_t *entryInfo);

/*
 *  Function
 *    soc_c3_rce_entry_traverse
 *  Purpose
 *    Walk through all of the entries in a group on a unit, calling a
 *    caller-provided function for each entry.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) groupId = which group to scan
 *    (IN) callback = pointer to function to call
 *    (IN) extra = pointer to caller's additional data
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If the callback function returns any value other than SOC_E_NONE, it will
 *    abort the traverse.
 */
typedef int (*soc_c3_rce_entry_traverse_cb_t)(int unit,
                                              int programId,
                                              int groupId,
                                              int entryId,
                                              void *extras);
extern int
soc_c3_rce_entry_traverse(int unit,
                          int groupId,
                          soc_c3_rce_entry_traverse_cb_t callback,
                          void *extras);

/*
 *  Function
 *    soc_c3_rce_entry_dump
 *  Purpose
 *    Dump information about an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) flags = flags for the dump
 *    (IN) prefix = prefix string
 *    (IN) entryId = which entry to dump
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Uses bsl_printf to display the information being dumped.
 */
extern int
soc_c3_rce_entry_dump(int unit,
                      uint32 flags,
                      const char *prefix,
                      int entryId);

/*
 *  Function
 *    soc_c3_rce_entry_create
 *  Purpose
 *    Creates an entry based upon the requested entry specification
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID
 *    (IN) entryInfo = a pointer to the entry information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function creates an entry according to the provided entry
 *    information (including group and priority).  It does not 'install' the
 *    entry, nor does it fill in any matching parameters or actions.
 *
 *    This might allocate an additional filter set for the group, or rearrange
 *    entries in some of the group's filter sets, if the filter set into which
 *    the entry is being placed does not any additional space.
 */
extern int
soc_c3_rce_entry_create(int unit,
                        int entryId,
                        const soc_c3_rce_entry_desc_t *entryInfo);

/*
 *  Function
 *    soc_c3_rce_entry_copy
 *  Purpose
 *    Creates an entry based upon another entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID
 *    (IN) entryInfo = a pointer to the entry information
 *    (IN) sourceEntryId = source entry ID
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function creates an entry based upon another entry, with the
 *    priority specified by the entryInfo (but not group or other fields, since
 *    these are taken from the source entry).
 *
 *    This can only create the copy in the same group as the source.
 *
 *    This might allocate an additional filter set for the group, or rearrange
 *    entries in some of the group's filter sets, if the filter set into which
 *    the entry is being placed does not any additional space.
 *
 *    See SOC_C3_RCE_ENTRY_COPY_... definitions above for the copyFlags value.
 */
extern int
soc_c3_rce_entry_copy(int unit,
                      uint32 copyFlags,
                      int entryId,
                      const soc_c3_rce_entry_desc_t *entryInfo,
                      int sourceEntryId);

/*
 *  Function
 *    soc_c3_rce_entry_priority_set
 *  Purpose
 *    Changes the priority on an existing entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID
 *    (IN) entryPriority = new priority for the entry
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function actually creates a copy of the entry using any available
 *    entry ID, then destroys the original and updates the tables so the
 *    original's entry ID refers to the copy.
 *
 *    In this way, the change of priority appears atomic to traffic and
 *    conditions which might cause an issue will not result in the loss of the
 *    original entry.
 *
 *    Due to the way it works, though, it requires space for at least one more
 *    entry in the group in question (so it will not work if that entry can not
 *    be created for any reason).
 */
extern int
soc_c3_rce_entry_priority_set(int unit,
                              int entryId,
                              int entryPriority);

/*
 *  Function
 *    soc_c3_rce_entry_destroy
 *  Purpose
 *    Destroys an exiting entry in an existing group
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID to be destroyed
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Does not commit to hardware.  If an entry is not removed before it is
 *    destroyed, the entry may remain active in hardware at least until the
 *    next time the filter set containing the entry is updated in hardware, at
 *    which time it will be removed from hardware.
 */
extern int
soc_c3_rce_entry_destroy(int unit,
                         int entryId);

/*
 *  Function
 *    soc_c3_rce_entry_qualify_set
 *  Purpose
 *    Sets the specified qualifier on an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry to manipulate
 *    (IN) qualIdx = index (within group) of qualifier to update
 *    (IN) data = pointer to the data bits
 *    (IN) mask = pointer to the mask bits
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Does not commit to hardware.
 *
 *    This sets the part of the 'API' copy of the entry's pattern that
 *    corresponds to the selected qualifier (by index).  The specifics of the
 *    qualifiers were set when creating the group.
 *
 *    This quietly disregards any provided bits beyond those that would apply
 *    to the specified qualifier.
 */
extern int
soc_c3_rce_entry_qualify_set(int unit,
                             int entryId,
                             unsigned int qualIdx,
                             const uint8 *data,
                             const uint8 *mask);

/*
 *  Function
 *    soc_c3_rce_entry_qualify_get
 *  Purpose
 *    Gets the specified qualifier from an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry to manipulate
 *    (IN) qualIdx = index (within group) of qualifier to update
 *    (OUT) data = pointer to the data bits
 *    (OUT) mask = pointer to the mask bits
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This gets the part of the 'API' copy of the entry's pattern that
 *    corresponds to the selected qualifier (by index).  The specifics of the
 *    qualifiers were set when creating the group.
 *
 *    This will only fill in bits that apply to the specified qualifier; any
 *    leftover bits in the provided buffers are not guaranteed.
 */
extern int
soc_c3_rce_entry_qualify_get(int unit,
                             int entryId,
                             unsigned int qualIdx,
                             uint8 *data,
                             uint8 *mask);

/*
 *  Function
 *    soc_c3_rce_entry_qualify_range_traverse
 *  Purpose
 *    Traverse the ranges used for qualification of a particular entry.  This
 *    will invoke the provided callback only for the ranges the specific entry
 *    is using.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = the entry ID
 *    (IN) callback = the function to call for each range this entry
 *    (IN) extra = pointer to caller's additional data
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if range was found or added successfully
 *      SOC_E_* appropriately otherwise
 *  Notes
 */
typedef int (*soc_c3_rce_entry_qualify_range_traverse_cb_t)(int unit,
                                                            int programId,
                                                            int groupId,
                                                            int entryId,
                                                            int rangeId,
                                                            int qualify,
                                                            void *extras);
extern int
soc_c3_rce_entry_qualify_range_traverse(int unit,
                                        int entryId,
                                        soc_c3_rce_entry_qualify_range_traverse_cb_t callback,
                                        void *extras);

/*
 *  Function
 *    soc_c3_rce_entry_qualify_range_set
 *  Purpose
 *    Set whether an entry qualifiers on a specific range+inversion.  This will
 *    add the range+inversion to the filter set if necessary and possible when
 *    setting the entry to qualify on a range, and will remove the
 *    range+inversion if it is present and no longer needed.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = the entry ID
 *    (IN) rangeId = the range ID
 *    (IN) qualify = negative if inverted, zero don't care, positive if normal
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if range was found or added successfully
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The qualify value indicates three states.
 *
 *    If qualify is negative, it indicates the intent is to set the entry so it
 *    matches on the inverse of the range.
 *
 *    If qualify is zero, it indicates the intent is to set the entry so the
 *    range does not matter for qualification.
 *
 *    If qualify is positive, it indicates the intent is to set the entry so it
 *    matches on the range as created.
 *
 *    It is possible, when switching an entry from matching on normal to
 *    inverse or inverse to normal, that there will not be enough resources to
 *    accomplish the switch.  This is particularly true when more than one
 *    entry might be using the same range+inversion.  If this happens, the
 *    entry will not be qualified on either normal or inverse of the range.
 */
extern int
soc_c3_rce_entry_qualify_range_set(int unit,
                                   int entryId,
                                   int rangeId,
                                   int qualify);

/*
 *  Function
 *    _soc_c3_rce_entry_qualify_range_get
 *  Purpose
 *    Get whether an entry qualifiers on a specific range and the inversion.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = the entry ID
 *    (IN) rangeId = the range ID
 *    (IN) qualify = where to put qualify value (see notes)
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if range was found or added successfully
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The qualify value indicates three states.
 *
 *    If qualify is negative, it indicates the entry matches on the inverse of
 *    the range.
 *
 *    If qualify is zero, it indicates the entry ignores the range (the range
 *    does not apply to the entry).
 *
 *    If qualify is positive, it indicates the entry matches on the range as
 *    created.
 */
extern int
soc_c3_rce_entry_qualify_range_get(int unit,
                                   int entryId,
                                   int rangeId,
                                   int *qualify);

/*
 *  Function
 *    soc_c3_rce_entry_qualify_clear
 *  Purpose
 *    Resets entry pattern to defaults (clears most qualifier forms)
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry whose qualifiers are to be reset
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This reverts the entry so its pattern is the same as the default pattern
 *    for the group.  If the entry was using range references, these references
 *    are cleaned up as appropriate.
 *
 *    Does not affect actions (only resets the entry's pattern).
 *
 *    Does not commit to hardware.
 */
extern int
soc_c3_rce_entry_qualify_clear(int unit,
                               int entryId);

/*
 *  Function
 *    soc_c3_rce_entry_action_set
 *  Purpose
 *    Sets the specified action on an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry to manipulate
 *    (IN) actIdx = index (within program) of action to update
 *    (IN) value = pointer to the action value bits
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Does not commit to hardware.
 *
 *    This quietly disregards any provided bits beyond those that would apply
 *    to the specified action.
 */
extern int
soc_c3_rce_entry_action_set(int unit,
                            int entryId,
                            unsigned int actIdx,
                            const uint8 *value);

/*
 *  Function
 *    soc_c3_rce_entry_action_get
 *  Purpose
 *    Sets the specified action on an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry to manipulate
 *    (IN) actIdx = index (within program) of action to update
 *    (IN) value = pointer to the action value bits
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Does not commit to hardware.
 *
 *    This will only fill in bits that apply to the specified action; any
 *    leftover bits in the provided buffers are not guaranteed.
 */
extern int
soc_c3_rce_entry_action_get(int unit,
                            int entryId,
                            unsigned int actIdx,
                            uint8 *value);

/*
 *  Function
 *    soc_c3_rce_entry_action_clear
 *  Purpose
 *    Resets entry actions to defaults
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry whose actions are to be reset
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This reverts the entry so its action data is the same as the default
 *    action data for the program.
 *
 *    Does not affect qualifiers (only resets the entry's actions).
 *
 *    Does not commit to hardware.
 */
extern int
soc_c3_rce_entry_action_clear(int unit,
                              int entryId);

/*
 *  Function
 *    soc_c3_rce_entry_install
 *  Purpose
 *    Installs the entry to hardware
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry to install
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Marks the entry as active and then copies API state qualifier/action data
 *    to the hardware qualifer/action space, then updates the appropriate
 *    filter set of the entry's group to hardware.
 */
extern int
soc_c3_rce_entry_install(int unit,
                         int entryId);

/*
 *  Function
 *    soc_c3_rce_entry_remove
 *  Purpose
 *    Removes the entry from hardware
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry to remove
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Marks the entry as inactive in API state, hardware state, and pattern
 *    memory.  No other changes are made, since with the entry disabled, the
 *    rest of its pattern/action state is not important.
 */
extern int
soc_c3_rce_entry_remove(int unit,
                        int entryId);

/*
 *  Function
 *    soc_c3_rce_entry_counter_set
 *  Purpose
 *    Sets the value for the byte+frame counters on an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry Id whose counters are to be written
 *    (IN) counters = where to get counters for the entry
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_UNAVAIL if counter support not available
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Supports action-indexed counters, which is where the RCE result is used
 *    by the LRP microcode to select a counter to increment.  This takes care
 *    of the dynamic nature of the mapping of entry to result index.
 *
 *    This does not support other counter modes, such as having a 'counter'
 *    action specified in the actions for an entry.  Those do not require the
 *    overhead of action-indexed mapping, and so can be managed by the client.
 *
 *    Will return SOC_E_UNAVAIL if the action-indexed counter support is not
 *    available, either because it is disabled by compile time switch or
 *    because the entry's program does not support that mode.
 *
 *    Counters must point to a buffer holding TWO uint64 values.  The first
 *    value is bytes, the second value is frames.
 */
extern int
soc_c3_rce_entry_counter_set(int unit,
                             int entryId,
                             const uint64 *counters);

/*
 *  Function
 *    soc_c3_rce_entry_counter_get
 *  Purpose
 *    Gets the value for the byte+frame counters on an entry
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry Id whose counters are to be read
 *    (IN) clear = TRUE to clear after read; FALSE to not clear after read
 *    (OUT) counters = where to put counters for the entry
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_UNAVAIL if counter support not available
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Supports action-indexed counters, which is where the RCE result is used
 *    by the LRP microcode to select a counter to increment.  This takes care
 *    of the dynamic nature of the mapping of entry to result index.
 *
 *    This does not support other counter modes, such as having a 'counter'
 *    action specified in the actions for an entry.  Those do not require the
 *    overhead of action-indexed mapping, and so can be managed by the client.
 *
 *    Will return SOC_E_UNAVAIL if the action-indexed counter support is not
 *    available, either because it is disabled by compile time switch or
 *    because the entry's program does not support that mode.
 *
 *    Counters must point to a buffer to hold TWO uint64 values.  The first
 *    value is bytes, the second value is frames.
 *
 *    Technically, the clear is done in a sort of rippled manner, but it is
 *    done in such a way as to prevent count loss (frames that come after the
 *    clear will be counted, those that came before the clear were already
 *    considered in the returned values).  The effect is a lossless
 *    read-and-clear, with the effective time of the clear being between the
 *    entry and return.
 */
extern int
soc_c3_rce_entry_counter_get(int unit,
                             int entryId,
                             int clear,
                             uint64 *counters);

/*
 *  Function
 *    soc_c3_rce_range_first_avail
 *  Purpose
 *    Return the first available range ID (in case the application does not
 *    want to manage range IDs itself)
 *  Arguments
 *    (IN) unit = unit number
 *    (OUT) firstAvailRangeId = where to put the first available range ID
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Provides the first available range ID at the moment (subject to change as
 *    groups are created / destroyed).  Will provide an range ID that is valid
 *    for the device.  If there are no available valid range IDs, will return
 *    SOC_E_RESOURCE.
 *
 *    Note range IDs start at one (not zero) due to a quirk in the BCM layer.
 */
extern int
soc_c3_rce_range_first_avail(int unit,
                             int *firstAvailRangeId);

/*
 *  Function
 *    soc_c3_rce_range_next_existing
 *  Purpose
 *    Given a range ID, find the next one that exists.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) currRangeId = starting point in the range ID space
 *    (OUT) nextRangeId = where to put next existing range ID if found
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    The current range ID does not need to be an existing range; the search
 *    will start with the range ID immediately subequent to currRangeId.
 *
 *    If the current range ID is not valid (outside of valid range ID space for
 *    the unit), the search will begin with range ID one.
 *
 *    If no existing range is encountered before exhausing the remainder of the
 *    range ID space, will return SOC_E_NOT_FOUND.
 *
 *    Note range IDs start at one (not zero) due to a quirk in the BCM layer.
 */
extern int
soc_c3_rce_range_next_existing(int unit,
                               int currRangeId,
                               int *nextRangeId);

/*
 *  Function
 *    soc_c3_rce_range_info_get
 *  Purpose
 *    Get information about a range.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) entryId = entry ID
 *    (OUT) entryInfo = where to put the pointer to the range information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This does not provide a pointer to the actual internal tables, which are
 *    considered read-only by the code.  Instead, it copies the data into a new
 *    buffer and returns the pointer to that buffer.  At return, the caller is
 *    considered to own the buffer, and must call soc_c3_rce_range_info_free to
 *    dispose of the buffer (which may actually be comprised of more than one
 *    heap cell or similar).
 *
 *    This is generally intended as a way to recover state under certain
 *    conditions.  The application is expected to keep track of the entry
 *    information it needs rather than querying this function regularly.
 */
extern int
soc_c3_rce_range_info_get(int unit,
                          int rangeId,
                          soc_c3_rce_range_desc_t **rangeInfo);

/*
 *  Function
 *    soc_c3_rce_range_info_free
 *  Purpose
 *    Free information about a range that is no longer needed.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) rangeId = range ID
 *    (OUT) rangeInfo = a pointer to the range information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function must be used to release range information that was created
 *    by the soc_c3_rce_range_info_get, and only to release such information.
 */
extern void
soc_c3_rce_range_info_free(int unit,
                           soc_c3_rce_range_desc_t *rangeInfo);

/*
 *  Function
 *    soc_c3_rce_range_traverse
 *  Purpose
 *    Walk through all of the ranges in a unit, calling a caller-provided
 *    function for each range.
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) callback = pointer to function to call
 *    (IN) extra = pointer to caller's additional data
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If the callback function returns any value other than SOC_E_NONE, it will
 *    abort the traverse.
 */
typedef int (*soc_c3_rce_range_traverse_cb_t)(int unit,
                                              int rangeId,
                                              void *extras);
extern int
soc_c3_rce_range_traverse(int unit,
                          soc_c3_rce_range_traverse_cb_t callback,
                          void *extras);

/*
 *  Function
 *    soc_c3_rce_range_dump
 *  Purpose
 *    Dump information about a range
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) flags = flags for the dump
 *    (IN) prefix = prefix string
 *    (IN) rangeId = which range to dump
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Uses bsl_printf to display the information being dumped.
 */
extern int
soc_c3_rce_range_dump(int unit,
                      uint32 flags,
                      const char *prefix,
                      int rangeId);

/*
 *  Function
 *    soc_c3_rce_range_create
 *  Purpose
 *    Creates a range based upon the requested range specification
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) rangeId = range ID
 *    (IN) rangeInfo = a pointer to the range information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This function creates a range according to the provided range
 *    information.  It does not install the range in any group or otherwise
 *    modify hardware.
 */
extern int
soc_c3_rce_range_create(int unit,
                        int rangeId,
                        const soc_c3_rce_range_desc_t *rangeInfo);

/*
 *  Function
 *    soc_c3_rce_range_destroy
 *  Purpose
 *    Destroys an exiting range
 *  Arguments
 *    (IN) unit = unit number
 *    (IN) rangeId = range ID to be destroyed
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    A range can not be destroyed if any entry refers to it.
 */
extern int
soc_c3_rce_range_destroy(int unit,
                         int rangeId);


#endif /* def BCM_CALADAN3_SUPPORT */

