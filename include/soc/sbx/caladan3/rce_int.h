/*
 * $Id: rce_int.h,v 1.13 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * File:    rce.h
 * Purpose: Caladan3 Rule Classifier drivers
 * Requires:
 */

#ifdef BCM_CALADAN3_SUPPORT
#include <shared/shr_resmgr.h>
#include <shared/idxres_mdb.h>
#ifdef BCM_WARM_BOOT_SUPPORT
#include <soc/types.h>
#endif /* def BCM_WARM_BOOT_SUPPORT */

/*
 *  This file provides some structures that are necessary across files for
 *  the RCE code but that should not norbally be used by SOC callers.
 */

/*
 *  CONFIG:
 *
 *  If C3_RCE_ENABLE_ACTION_INDEX_COUNTERS is TRUE, this driver will also
 *  provide support for counters that use the result (action index) from the
 *  RCE lookup to select a counter.  This also requires the configuration
 *  tables include a function to initialise the counter space and to retrieve
 *  elements from the counter space (that supports both individual counters and
 *  blocks of up to one filter set worth (768)).  If enabled here and in the
 *  configuration tables, it imposes a significant penalty on updating filter
 *  sets, since each update also must copy and maintain counter state.  Even if
 *  this is TRUE, if the configuration table does not specify functions for
 *  clearing and retrieving counters on a given program, counters will not be
 *  supported for that action table.
 *
 *  If C3_RCE_ENABLE_ACTION_INDEX_COUNTERS is FALSE, the support for result
 *  (action index) counters is disabled, and this driver will always return
 *  SOC_E_UNAVAIL for the related functions.
 *
 *  Regardless of the setting of C3_RCE_ENABLE_ACTION_INDEX_COUNTERS, the
 *  application may still implement counters, for example, by including a
 *  counter index in the action table (so LRP microcode will use the counter
 *  index from the action table to select a counter), and managing the counters
 *  above this driver.  It is recommended that the application use only one of
 *  the two options, but it is possible to mix both methods.
 *
 *  WARNING: This feature assumes the CMU configuration is so that the CMU
 *  driver will provide a pair of 64b numbers as the counters -- the first one
 *  for bytes and the second one for frames.  Also, the index passed to the CMU
 *  driver will always refer to the pair as the unit of index.
 */
#define C3_RCE_ENABLE_ACTION_INDEX_COUNTERS TRUE

/*
 *  CONFIG:
 *
 *  Specify the maximum number of ranges supported.  The code will allow up to
 *  this many to be created.  Changing this only adjusts the size of the
 *  working memory (and backing store) assigned to keeping track of ranges.
 *
 *  Ranges only consume hardware resources on reference -- if a range has no
 *  entries referring to it, it will not be placed in hardware.
 *
 *  Note that due to a quirk in the BCM APIs, the actual range IDs supported
 *  will run from 1 through C3_RCE_RANGE_MAXIMUM.  Most other resources allow
 *  ID zero to be used.
 *
 *  This has no effect on the number of distinct range references that can be
 *  made from a single filter set.  That is specified when the group is
 *  created, and so can be varied accoring to the needs for each group.
 */
#define C3_RCE_RANGE_MAXIMUM 128

/*
 *  CONFIG:
 *
 *  Specify the maximum length of the name that will be kept with each range
 *  for diagnostics and in backing store for warm boot.  This is a number of
 *  bytes and includes the NUL character at the end.
 */
#define C3_RCE_RANGE_NAME_BYTES 32

/*
 *  CONFIG:
 *
 *  Specify the maximum length of the name that will be kept with qualifiers
 *  for diagnostics and in backing store for warm boot.  This is a number of
 *  bytes and includes the NUL character at the end.
 */
#define C3_RCE_QUAL_NAME_BYTES 32

/*
 *  The minimum program length allowed by hardware
 */
#define C3_RCE_PROGRAM_LENGTH_MINIMUM 128

/*
 *  Specify the minimum length for a filter set, based upon where it is within
 *  a program (first filter set or later filter set).  Hardware constraints, so
 *  these should not be adjusted.
 */
#define C3_RCE_FILTER_SET_LENGTH_FIRST 5
#define C3_RCE_FILTER_SET_LENGTH_LATER 4

/*
 *  CONFIG:
 *
 *  Since minimum program length is 128 clocks, we need to have padding. Rather
 *  than allocating padding on demand, the code sets up a set of padding filter
 *  sets and uses them according to what is needed per program.  These padding
 *  filter sets are shared between all active programs, and are divided into
 *  blocks based upon some manageable increment.  This must be at least
 *  C3_RCE_FILTER_SET_LENGTH_LATER, but can be larger.  Making it smaller
 *  provides better precision.
 *
 *  If this is == C3_RCE_FILTER_SET_LENGTH_FIRST, all padding blocks will be
 *  C3_RCE_FILTER_SET_LENGTH_PADDING long; if it is less, the first padding
 *  blcok will be C3_RCE_FILTER_SET_LENGTH_FIRST long and the others will be
 *  C3_RCE_FILTER_SET_LENGTH_PADDING long.
 *
 *  This must not be greater than C3_RCE_FILTER_SET_LENGTH_FIRST; if it is,
 *  there may be conditions where the padding wil clobber results when a
 *  program consists of a single very short filter set.
 */
#define C3_RCE_FILTER_SET_LENGTH_PADDING 4
/*
 *  But you need enough padding filter sets so they come out to 128 clocks, so
 *  this needs to be a number sufficiently large that multiplied by
 *  C3_RCE_FILTER_SET_LENGTH_PADDING will be at least 128.  Note the first of
 *  the padding filter sets will be C3_RCE_FILTER_SET_LENGTH_FIRST long, but
 *  the rest will be C3_RCE_FILTER_SET_LENGTH_PADDING long, so you can take
 *  that into account in setting this number.
 */
#define C3_RCE_FILTER_SET_COUNT_PADDING 32

/*
 *  Do not edit this; it is here for consistency and checking.
 */
#if (C3_RCE_FILTER_SET_LENGTH_PADDING < C3_RCE_FILTER_SET_LENGTH_LATER)
#error "Filter set length padding needs to be at least minimum filter set length"
#endif /* (C3_RCE_FILTER_SET_LENGTH_PADDING < C3_RCE_FILTER_SET_LENGTH_LATER) */
#if (C3_RCE_FILTER_SET_LENGTH_PADDING > C3_RCE_FILTER_SET_LENGTH_FIRST)
#error "Filter set padding must not be greater than first filter set length"
#endif /* (C3_RCE_FILTER_SET_LENGTH_PADDING > C3_RCE_FILTER_SET_LENGTH_FIRST) */
#if (C3_RCE_FILTER_SET_LENGTH_PADDING == C3_RCE_FILTER_SET_LENGTH_FIRST)
#define C3_RCE_FILTER_SET_FIRST_PADDING C3_RCE_FILTER_SET_LENGTH_PADDING
#define C3_RCE_FILTER_SET_PADDING (C3_RCE_FILTER_SET_LENGTH_PADDING * C3_RCE_FILTER_SET_COUNT_PADDING)
#else /* (C3_RCE_FILTER_SET_LENGTH_PADDING == C3_RCE_FILTER_SET_LENGTH_FIRST) */
#define C3_RCE_FILTER_SET_FIRST_PADDING C3_RCE_FILTER_SET_LENGTH_FIRST
#define C3_RCE_FILTER_SET_PADDING ((C3_RCE_FILTER_SET_LENGTH_PADDING * (C3_RCE_FILTER_SET_COUNT_PADDING - 1)) + C3_RCE_FILTER_SET_LENGTH_FIRST)
#endif /* (C3_RCE_FILTER_SET_LENGTH_PADDING == C3_RCE_FILTER_SET_LENGTH_FIRST) */
#if (C3_RCE_FILTER_SET_LENGTH_FIRST < C3_RCE_FILTER_SET_LENGTH_LATER)
#error "First filter set length must be >= later filter set length"
#endif /* (C3_RCE_FILTER_SET_LENGTH_FIRST < C3_RCE_FILTER_SET_LENGTH_LATER) */
#if (C3_RCE_FILTER_SET_PADDING < C3_RCE_PROGRAM_LENGTH_MINIMUM)
#error "Not enough space to pad shorter programs!"
#endif /* (C3_RCE_FILTER_SET_PADDING < 128) */
#if (C3_RCE_FILTER_SET_PADDING > (C3_RCE_PROGRAM_LENGTH_MINIMUM - 1 + C3_RCE_FILTER_SET_LENGTH_PADDING))
#error "Too much space to pad shorter programs!"
#endif /* (C3_RCE_FILTER_SET_PADDING > (127 + C3_RCE_FILTER_SET_LENGTH_PADDING)) */

/*
 *  CONFIG:
 *
 *  The RCE must return its results before the LRP performs the corresponding
 *  switch in the next epoch.  This means that several conditions must be met:
 *
 *    1) total key transfer time must be less than one epoch
 *
 *    2) final program key transfer must finish before first program key
 *       transfer is expected to start in the next epoch
 *
 *    3) total program run time (plus one additional RCE clock per program)
 *       must be less than one epoch
 *
 *    4) final program run must complete before first program is expected to
 *       start in the next epoch
 *
 *    5) result transfer time must be less than one epoch (this is actually the
 *       easiest one, since it it shorter than either of the other two, and so
 *       as long as the others fit, this fits)
 *
 *    6) result transfer for any program must finish before the switch
 *       statement for that program in the next epoch (or second epoch out, if
 *       the LRP microcode is using switch-2)
 *
 *  If C3_RCE_TIMING_ENFORCE_PERSISTENT is TRUE, the driver will refuse to
 *  allow an operation (such as adding a filter set) that would result in the
 *  final state exceeding the timing requirements.  If it is FALSE, the driver
 *  will allow the configuration to fall outside of timing requirements, but it
 *  will still emit warnings for these conditions.
 *
 *  If C3_RCE_TIMING_ENFORCE_TRANSIENT is TRUE, the driver will refuse to allow
 *  an operation (such as adding a filter set) that temporarily pushes outside
 *  of the timing requirements.  If it is FALSE, the driver will allow such
 *  temporary states, but it will still emit warnings for these conditions.
 *
 *  It is possible to set C3_RCE_TIMING_ENFORCE_PERSISTENT to FALSE and
 *  C3_RCE_TIMING_ENFORCE_TRANSIENT to TRUE, but under many conditions this
 *  will have the effect of setting both to TRUE, since a persistent state
 *  outside of timing requirements often at least temporarily requires a
 *  transient state that is further outside of timing requirements.  Operation
 *  in this mode is not supported nor is the enforcement guaranteed.
 *
 *  If C3_RCE_TIMING_FULL_DIAGNOSTICS is TRUE, all errors that occur when not
 *  in probe mode will be displayed at error level.  If it is FALSE, only the
 *  first one will be displayed at error level and the rest at VVERB.  This can
 *  be helpful if there is more than one problem in fitting the program in the
 *  available time (or wrapping, or some other issues), but since the analysis
 *  runs for several epochs it can also be redundant (it normally repeats the
 *  same errors for each subsequent epoch after the first failure).
 *
 *  If C3_RCE_TIMING_DUMP_RESULTS_TABLE is TRUE, the results table for the
 *  timing check will be dumped.  This can be helpful in tweaking things to try
 *  to get more entries of a certain type, or at least figuring out where the
 *  limits are being hit.  If FALSE, this information is not dumped.
 *
 *  C3_RCE_TIMING_DUMP_RESULTS is one of LOG_ERROR, LOG_WARN, LOG_INFO,
 *  LOG_VERBOSE, LOG_DEBUG, or RCE_EVERB.  It controls the output 'level' of the
 *  dump results, and so should be set to something that comfortably allows the
 *  results to be seen when wanted and not seen when not wanted.  If one of the
 *  LOG_* forms is used, it must follow that form as illustrated below.  If an
 *  RCE_* form is used, it must follow that form (also illustrated below).
 *
 *  C3_RCE_TIMING_ANALYSIS_ITERATIONS indicates how many iterations of the
 *  timing analysis are allowed before things can settle.  This is intended to
 *  compensate for clock misalignment between the LRP and RCE, as well as the
 *  possibility of odd placement (such as one program very early in the epoch
 *  and another program very late, causing a delay to wrap around if the later
 *  program is sufficiently large, and interfere with the earlier program).
 *  Ideally, this should be the least common multiple of the LRP clock divide
 *  and the RCE clock divide.
 *
 *  C3_RCE_TIMING_SLACK sets how far over the epoch the timing analysis is
 *  allowed to go before it is considered a problem.  This is expressed in
 *  common clocks (not clocks divided to RCE or LRP domains).
 */
#define C3_RCE_TIMING_ENFORCE_PERSISTENT TRUE
#define C3_RCE_TIMING_ENFORCE_TRANSIENT TRUE
#define C3_RCE_TIMING_FULL_DIAGNOSTICS FALSE
#define C3_RCE_TIMING_DUMP_RESULTS_TABLE TRUE
#define C3_RCE_TIMING_DUMP_RESULTS(stuff) LOG_VERBOSE(BSL_LS_SOC_COMMON, stuff)
#if (0 == 1) /* demostrate forms for C3_RCE_TIMING_DUMP_RESULTS */
#define C3_RCE_TIMING_DUMP_RESULTS(stuff) LOG_VERBOSE(BSL_LS_SOC_COMMON, stuff)
#define C3_RCE_TIMING_DUMP_RESULTS RCE_EVERB
#endif /* (0 == 1) -- demonstrate forms for C3_RCE_TIMING_DUMP_RESULTS */
#define C3_RCE_TIMING_ANALYSIS_ITERATIONS 15
#define C3_RCE_TIMING_SLACK 0

/*
 *  CONFIG:
 *
 *  This is to avoid lots of roundabout like htonl or similar, and to handle
 *  the byte ordering of DMA -- on both ends of the DMA channel -- inline.
 *
 *  All of the internal work is done based upon little endian at the host end
 *  and at the chip end.  This assumption is probably not correct in all
 *  situations, so we have knobs here to adjust the byte order used on DMA.
 *  Basically, the assumption is that per device memory address, the host will
 *  place the bytes in its memory least significant byte first, most
 *  significant byte last.
 *
 *  C3_RCE_DMA_HOST_END_MASK should be set to a value that, when exclusive-ORed
 *  with the address lines, will order the host end addresses in a DMA buffer
 *  (assume the buffer is aligned) so the bytes are in the order expected by
 *  the DMA controller when transferring from the host memory.  The code also
 *  assumes this order will be used by the DMA controller when writing to host
 *  memory.  If the DMA controller expects little-endian, it should be zero
 *  (see above); if big endian 32 bit grains, it should be 0x03; if big endian
 *  64-bit grains, it should be 0x07; and so on.
 *
 *  C3_RCE_DMA_CHIP_END_MASK_IMEM should be set to a value that, when
 *  exclusive-ORed with the address lines, will order the bytes as the chip
 *  expects the bytes for RCE instruction memory.  The RCE instruction memory
 *  addresses encompass 32-byte words, so if the RCE expects these in
 *  little-endian order, use zero; if it expects the entire thing in big-endian
 *  order, use 0x1F; similar for some other grain size.
 *
 *  Note that the equivalent of this feature for action tables is generated by
 *  the tools, according to the layout for the action table in question.
 */
#if BE_HOST
#define C3_RCE_DMA_HOST_END_MASK      0x00
#define C3_RCE_DMA_CHIP_END_MASK_IMEM 0x03
#else /* BE_HOST */
#define C3_RCE_DMA_HOST_END_MASK      0x00
#define C3_RCE_DMA_CHIP_END_MASK_IMEM 0x03
#endif /* BE_HOST */

/*
 *  The size of instruction memory on the C3's RCE (in blocks)
 */
#define C3_RCE_IMEM_SIZE 1536

/*
 *  Instructions per instruction block on the C3's RCE
 */
#define C3_RCE_IMEM_INSTR_PER_BLOCK 8

/*
 *  The number of entries in a single filter set
 */
#define C3_RCE_ENTRIES_PER_FILTER_SET 768

/*
 *  Debugging output formatting and control...
 */
#define _SOC_C3_RCE_EXCESS_VERBOSITY TRUE
#if _SOC_C3_RCE_EXCESS_VERBOSITY
#define RCE_EVERB(stuff)       LOG_DEBUG(BSL_LS_SOC_TCAM, stuff)
#else /* _SOC_C3_RCE_EXCESS_VERBOSITY */
#define RCE_EVERB(stuff)
#endif /* _SOC_C3_RCE_EXCESS_VERBOSITY */

#define RCE_MSG1(string) BSL_META(string)

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
 *  As long as fields are byte aligned, this still seems to make some kind of
 *  sense, with fields running across whole bytes.  However, there are
 *  sometimes fields that are split across bytes (IPv6 DSCP+ECN bits
 *  7,6,5,4,3,2,1,0 covers bits 3,2,1,0,15,14,13,12 of the IPv6 header when
 *  noted in this manner, and it is noted by start of 12 and length of 8 here).
 */
#define _C3_RCE_FRAME_BIT(_byte, _bit) (((_byte) * 8) + (_bit))
#define _C3_RCE_KEY_BIT(_byte, _bit) (((_byte) * 8) + (_bit))
#define _C3_RCE_NO_BITS 0, 0

/*
 *  Bit numbering within the key is the same concept as the bit numbering
 *  within frames, with the least significant bit being considered bits zero of
 *  a byte and the most significant byte being bit 7 of that byte, with the
 *  least significant byte of the key is numbered byte zero, however, the key
 *  is formed in the opposite direction (drawing the key out across the page
 *  would have the byte numbers increasing from right to left).
 *
 *  The upshot of this is that multibyte fields get mapped a little oddly --
 *  for example, the ethernet destination address occupies bytes 0,1,2,3,4,5
 *  of a frame, with byte 0 being the most significant byte and byte 5 being
 *  the least significant byte; the same bytes are copied into the key in the
 *  same order, but the key numbering places them as 5,4,3,2,1,0 with bye 5
 *  being the most significant byte and byte 0 being the least significant
 *  byte (particularly, frame byte 0 is key byte 5, frame byte 1 is key byte 4,
 *  and so on).
 *
 *  Fields start at one end and run contiguous bits until finished.
 *
 *  Just so things are even more confusing, the BCM layer prefers to number
 *  things in frames starting with the *most* significant bit, so this means
 *  the frame copy of the destination ethernet address actually starts with bit
 *  7 and runs for 48 bits total, through bit 40, thus:
 *
 *    byte 0          byte 1                byte 2
 *   7,6,5,4,3,2,1,0 15,14,13,12,11,10,9,8 23,22,21,20,19,18,17,16
 *
 *    byte 3                  byte 4                  byte 5
 *   31,30,29,28,27,26,25,24 39,38,37,36,35,34,33,32 47,46,45,44,43,42,41,40
 *
 *  But the key order starts with the least significant bit and runs in the
 *  other direction (note the bytes appear in the same order here though they
 *  are numbered differently) it starts with bit 0 and runs for 48 bits,
 *  through bit 47:
 *
 *    byte 5                  byte 4                  byte 3
 *   47,46,45,44,43,42,41,40 39,38,37,36,35,34,33,32 31,30,29,28,27,26,25,24
 *
 *    byte 2                  byte 1                byte 0
 *   23,22,21,20,19,18,17,16 15,14,13,12,11,10,9,8 7,6,5,4,3,2,1,0
 *
 *  So if the MAC address was 12:34:56:78:9A:BC, network byte 0 and key byte 5
 *  are 12, network byte 1 and key byte 4 are 34, network byte 2 and key byte 3
 *  are 56, network byte 3 and key byte 2 are 78, network byte 4 and key byte 1
 *  are 9A, and network byte 5 and key byte 0 are BC.
 */

/*
 *  Definition of a function that clears all counters in a segment.  This is
 *  used during init or reset to ensure all counters associated with a given
 *  action table are zero.
 */
typedef int (*_soc_c3_rce_counter_clear_f)(int unit);

/*
 *  Definition of a function to read a counter or set of counters.  This is
 *  used when accessing counters associated with an action table.
 */
typedef int (*_soc_c3_rce_counter_read_f)(int unit, uint32 ulIndex, uint32 ulEntries, int clear, uint64 *pCount);

/*
 *  Description of an action table
 *
 *  Each program can have up to four results, and each one can possibly refer
 *  to a different action table.
 *
 *  If a result refers to an action table, it will be managed according to the
 *  space in that action table (and associated counter table, if applicable to
 *  the action table in question).
 *
 *  A result can refer to no action tables.  In this case the driver considers
 *  it to be a debugging stub and its value will reflect only its position
 *  within the group (and so may overlap other entries).
 *
 *  If two or more results refer to the same action table, they will receive
 *  the same result value from the program (the particular action table
 *  resource consumption is not multiplied for such cases).
 *
 *  The ctrClearAll and ctrRead pointers are for use when an action table
 *  implies an action-indexed counter table is associated.  In this case, the
 *  ctrClearAll function is used to initialise the counters and the ctrRead
 *  function is used to collect them (sometimes individually and sometimes in
 *  lots of up to a full filter set).  If these pointers are NULL, it indicates
 *  there are no such counters for the action table (but this case does not
 *  preclude a counter action in the table -- only that the action-indexed
 *  counters feature does not apply to this action table).
 */
#define SOC_C3_RCE_MAX_OCM_SEG_PER_ACTION_TABLE 2
/*
 *  Describes a single OCM segment of an action table
 */
typedef struct _soc_c3_rce_act_uc_ocm_desc_s {
      const char *segmentName;            /* OCM segment name for this segment */
      const sbx_caladan3_ocm_port_e_t ocmPort; /* OCM port for action tbl seg */
      const int ocmSeg;                   /* OCM segment for action table seg */
      const unsigned int bitsPerAction;   /* bits/slot this action table seg */
      const unsigned int byteOrderMask;   /* byte order mask this table seg */
      const unsigned int actionsPerElem;  /* actions per OCM element this seg */
      const unsigned int elemsPerAction;  /* OCM elements per action this seg */
} _soc_c3_rce_actions_uc_ocm_desc_t;
/*
 *  Describes the action table including all OCM segments involved
 */
typedef struct _soc_c3_rce_actions_uc_desc_s {
      const char *tableName;              /* action table name */
      const soc_c3_rce_action_uc_desc_t *actions; /* action table entry layout */
      const unsigned int actionLimit;     /* number of action slots this table */
      const unsigned int ocmSegments;     /* number of OCM segments this table */
      const _soc_c3_rce_actions_uc_ocm_desc_t seg[SOC_C3_RCE_MAX_OCM_SEG_PER_ACTION_TABLE];
      const char *ctrName;                /* counter segment name */
      const _soc_c3_rce_counter_clear_f ctrClearAll; /* clear all counters */
      const _soc_c3_rce_counter_read_f ctrRead; /* read counters */
} _soc_c3_rce_actions_uc_desc_t;

/*
 *  Description of a program
 *
 *  There can be up to 16 programs, but it is unlikely that there will be more
 *  than one or two at high line rates due to timing constraints.
 *
 *  Each program must provide enough information that the driver knows how to
 *  deal with the way that program interfaces with the LRP microcode.
 *
 *  The qualifiers provided here are used by the RCE driver to provide a lookup
 *  feature (soc_c3_rce_program_qualifier_build) to ease the burden on the SOC
 *  client when constructing qualifiers.  The SOC client is free to directly
 *  specify key bits instead of using this feature, and must do so if the key
 *  format can change in a way that allows the same field to appear in the key
 *  in more than one location according to that format.
 *
 *  The resTable pointers point to a specific action table.  Each result
 *  register is represented by a corresponding pointer in resTable.  If that
 *  pointer is NULL, that result register is not used by the program.  If the
 *  pointer is not NULL, the result register is used by the program to refer to
 *  the specific action table.  It is possible to have up to four distinct
 *  action tables used by a single program.  It is also possible to have more
 *  than one program refer to the same action tables (in this case, the
 *  pointers must point to the same description -- the pointers themselves must
 *  be equal).  If a single program refers to the same action table more than
 *  once, each entry will have more than one action slot allocated in that
 *  table (one action slot per reference to that action table made by the
 *  program); while this can possibly allow overlapping or other complex action
 *  behaviour, it can increse resource consumption considerably.
 */
typedef struct _soc_c3_rce_program_uc_desc_s {
    const char *programName;            /* program name */
    const uint32 programFlags;          /* flags for this program */
    const uint32 tmuProg;               /* which TMU programs to map (bits) */
    const soc_c3_rce_qual_uc_desc_t *qualifiers; /* key layout for program */
    const _soc_c3_rce_actions_uc_desc_t *resTable[SOC_C3_RCE_RESULT_REGISTER_COUNT];
} _soc_c3_rce_program_uc_desc_t;
#define _SOC_C3_RCE_PROGRAM_ENABLE 0x00000001

/*
 *  Opcodes supported by the RC elements
 *
 *
 *  Nop is used for padding.  Not quite true to its name, it can have some
 *  effect: if the end flag is set, it indicates this instruction is part of
 *  the last block of instructions for this filter set.
 *
 *
 *  The data, match0, and match1 opcodes take a sixteen bit word from the key,
 *  operate on it using their predicates, and then act upon the result.
 *
 *  Each of these selects a 16-bit word (yes, always 16 bits long and always
 *  aligned to an integral multiple of 16 bits) from the key for comparison.
 *
 *  In data mode, Data compares the predicate result to the pattern bit,
 *  mismatching if they are not equal (so if the pattern bit is set, the
 *  predicate result must be TRUE; if the pattern bit is clear, the predicate
 *  result must be FALSE).
 *
 *  In prefix mode, Data ignores the predicate result until the instruction
 *  after the first zero bit in the pattern including the last prefix
 *  instruction; it then reverts to normal mode for subsequent instructions.
 *
 *  Match0 mismatches if the pattern bit is 1 and the predicate result is one
 *  (TRUE).  If the pattern bit is zero, this does not affect the match state.
 *
 *  Match1 mismatches if the pattern bit is 1 and the predicate result is zero
 *  (FALSE).  If the pattern bit is zero, this does not affect the match state.
 *
 *
 *  The prefix instruction places following data opcodes in 'prefix mode',
 *  where they will ignore any predicate results until the instruction after
 *  the next zero pattern bit (or, if the prefix instruction itself has its
 *  pattern bit zero, it remains in normal state). It does not directly affect
 *  the match state.
 *
 *  WARNING: prefix must not be encountered immediately after the first zero
 *  pattern bit from a prior prefix instruction; it will be ignored in this
 *  case and the matching will not correctly enter prefix mode.  The driver
 *  avoids this condition with a NOP following prefix-mode comparisons.
 *
 *
 *  StartFilter starts a filter set and indicates where to put the result from
 *  that filter set, what base value to use (result = base + rule index), and
 *  the address of the next filter set in this program (note the address is in
 *  8 instruction blocks).  If the next filter set address is 0xFFF, this is
 *  the last filter set of the program.
 *
 *  StartProgram (it's called start-rule in the docs, but this makes more sense
 *  to me) is used instead of StartFilter for the first filter set of a
 *  program.  It does everything StartFilter does plus it resets result
 *  registers, so they are ready for new matches.
 */
typedef enum _soc_c3_rce_opcode_e {
    _c3RCEOpcode_nop = 0,    /* do nothing */
    _c3RCEOpcode_data = 1,   /* predicate output must be equal to pattern bit */
    _c3RCEOpcode_prefix = 2, /* enter prefix mode (exits on FALSE predicate) */
    /* 3 is an invalid opcode so it is not included */
    _c3RCEOpcode_match0 = 4, /* predicate output must be 0 if pattern is TRUE */
    _c3RCEOpcode_match1 = 5, /* predicate output must be 1 if pattern is TRUE */
    _c3RCEOpcode_startFilter = 6, /* start new filter set */
    _c3RCEOpcode_startProgram = 7, /* start new program */
    _c3RCEOpcodeLimit        /* not a real value; must be last */
} _soc_c3_rce_opcode_t;
#define _SOC_C3_RCE_INSTRUCTION_NAMES \
    "NOP    ", \
    "Data   ", \
    "Prefix ", \
    "-------", \
    "Match0 ", \
    "Match1 ", \
    "StartFi", \
    "StartPr"

/*
 *  Some opcodes use 'predicates' to resolve the desired bit(s) from the key
 *
 *
 *  The range predicate takes the sixteen bit key word and compares it to a
 *  minimum value and a maximum value.
 *
 *  Iff rangeMin <= keyWord <= rangeMax, this predicate returns TRUE.
 *
 *
 *  The lookup predicate takes four bits from the sixteen bit key word and
 *  concatenates them to form an index, which is then used to look into the
 *  provided bitmap.  It returns TRUE iff the indexed bit in the bitmap is set.
 *
 *
 *  The individual bits of the key word, the data operand, and the mask
 *  operand, are run through a function to generate a set of output bits.
 *
 *  Key  Data Mask   OR   AND
 *  ---  ---- ----   ---  ---
 *   0    0    0      1    1
 *   0    0    1      0    1
 *   0    1    0      0    0
 *   0    1    1      0    1
 *   1    0    0      0    0
 *   1    0    1      0    1
 *   1    1    0      1    1
 *   1    1    1      0    1
 *
 *  The OR reduction then ORs the resulting bits together, so the result will
 *  be TRUE whenever there is at least one mask bit that is zero and the
 *  corresponding bit of data and key are equal; it will otherwise be FALSE.
 *
 *  The AND reduction then ANDs the resulting bits together, so the result will
 *  be FALSE whenever there is at least one mask bit that is zero and the
 *  corresponding bit of data and key are not equal; it will otherwise be
 *  TRUE.
 *
 *  For selecting a single bit, the AND and OR reduction predicates are
 *  equivalent: all bits of mask are set except the selected bit; the
 *  corresponding bit of data is equal to the desired value for the key bit,
 *  and the result will be TRUE if the key bit equals the data bit but will be
 *  FALSE when the key bit is not equal to the data bit.
 */
typedef enum _soc_c3_rce_predicate_e {
    _c3RCEPredicate_Range = 0,      /* range comparison */
    _c3RCEPredicate_Lookup = 1,     /* look up table */
    _c3RCEPredicate_MaskOr = 2,     /* OR reduction */
    _c3RCEPredicate_MaskAnd = 3,    /* AND reduction */
    _c3RCEPredicateLimit            /* not a real value; must be last */
} _soc_c3_rce_predicate_t;
#define _SOC_C3_RCE_PREDICATE_NAMES \
    "Range  ", \
    "Lookup ", \
    "MaskOr ", \
    "MaskAnd"


/*
 *  Internal description of a complete single RCE instruction
 *
 *  The instructions for a group are built at group creation time, with only
 *  the start instructions getting modified at time of programming to the
 *  hardware.  Each group's instruction set begins with a start-filter
 *  instruction, which is changed to a start-program instruction for the first
 *  set.  Other opcodes are used normally, except that the filter set will be
 *  padded to an integral multiple of eight instructions by adding NOPs if
 *  necessary, and all opcodes in the final eight that have the EndF bit will
 *  have it set.
 *
 *  Note that within the driver, some of the upper bits (those above the 43 bit
 *  instruction length) are used internally for specific purposes, but these
 *  bits never appear in the hardware.
 */
typedef uint64 _soc_c3_rce_instruction_t;

/*
 *  DMA buffer format for RCE program block (8 instructions)
 *
 *  This may be 'only' 32 words to the RCE hardware, but since the RCE words
 *  are 32 bytes (256 bits) each, it amounts to 1KB.  This is the minimum block
 *  of RCE instruction+pattern that can be written with a DMA transfer.  A
 *  filter set consists of some number of these -- specifically, there must be
 *  at least 40 instructions in the first filter set of a program, and there
 *  must be at least 32 instructions in subsequent filter sets of a program.
 *  Also, since RCE filter sets must begin on an instruction that is an
 *  integral multiple of eight, and end on an instruction that is one short of
 *  an integral multiple of eight, this is the minimum granularity with which
 *  RCE program memory can be allocated.
 *
 *  The first two 32-byte words of this are instructions.  Each word holds four
 *  instructions, packed low (so 4 * 43 bits = 172 bits).  The first word holds
 *  instructions 0..3 (instruction 0 is lowest 43 bits); the second word holds
 *  instructions 4..7 (instruction 4 is lowest 43 bits).  The remaining bits in
 *  each word (bits 172..255) do not matter to the hardware.
 *
 *  The next six words do not matter to the hardware.
 *
 *  The remaining 24 words are the pattern data.  The bits corresponding to the
 *  instructions for each rule's pattern are gathered together, so byte 0 of
 *  the first of these pattern words is for rule 0, byte 1 is for rule 1,
 *  and so on, through rule 31, with word 1 being rules 32..63, and so on,
 *  through word 23 byte 31 being for pattern 767.
 */

typedef uint8 _soc_c3_rce_program_line_t[32];
typedef struct _soc_c3_rce_program_block_s {
    _soc_c3_rce_program_line_t instruction[2];
    _soc_c3_rce_program_line_t unused[6];
    _soc_c3_rce_program_line_t pattern[24];
} _soc_c3_rce_program_block_t;

/*
 *  Describe a header field, as stored internally.
 *
 *  This indicates from where a set of key bits is taken, and how many bits are
 *  so taken from that place.  Note that the bit ordering is as if the bits for
 *  each byte are laid out most significant on the left (bit 7) and least
 *  significant on the right (bit 0) and bytes are placed in increasing offset
 *  toward the right (so bit 7 of byte 1 is immediately right of bit 0 of byte
 *  0).  Fields are designated as starting at MSb and run contiguous bits
 *  toward the right (toward less significant bits) for the specified number of
 *  bits.
 *
 *  Since fieldName is now copied locally, we can't use the 'exposed' version
 *  of this internally (at least, while keeping the 'const' restriction on the
 *  field name in the exposed version).
 */
typedef struct _soc_c3_rce_header_field_desc_int_s {
    soc_c3_rce_data_header_t header;    /* header containing this field */
    int startBit;                       /* low bit of this field in the hdr */
    unsigned int numBits;               /* number of bits in this field */
    char *fieldName;                    /* pointer to name for field */
} _soc_c3_rce_header_field_desc_int_t;

/*
 *  Describe a range internally.
 *
 *  Group qualifiers are specified in terms of key bits.  This method works
 *  there, since the group will not be moved between stages (which can have
 *  different key formats).
 *
 *  However, ranges do not have such a limitation at the BCM layer -- a range
 *  can be created and could then be applied to any stage that supported them.
 *  Here, any stage can support them.
 *
 *  Basically, this gives us a problem: the bits underlying a range could, for
 *  'standard' qualifiers (such as UDP/TCP source port), move around in the key
 *  format between stages.  However, hardware qualifiers (such as the special
 *  data provided in words 27..31 of the hardware key) do not move between
 *  stages.  So, we're left with a need to specify it *either* way: literal bit
 *  range of the key, or mapped to a field provided by the LRP microcode.
 *
 *  In addition, we are restricted by hardware that a range can only be applied
 *  to something left-aligned to a 16-bit grain within the key.  We can, of
 *  course, make it *shorter* than 16 bits, if desired, but it must be left
 *  aligned within the 16-bit grain if we do this (the MSb of the field must be
 *  bit 15 of the grain).
 */
typedef struct _soc_c3_rce_range_desc_int_s {
    /* description of the range */
    uint32 rangeFlags;                          /* flags */
    _soc_c3_rce_header_field_desc_int_t headerField; /* header field descr */
    int lowerBound;                             /* lower boundary */
    int upperBound;                             /* upper boundary */
    /* accounting */
    uint32 validProgs;                          /* valid in which programs */
    unsigned int refCount;                      /* fsets including this range */
} _soc_c3_rce_range_desc_int_t;

/*
 *  Describe an entry internally
 *
 *  An entry consists of the various metadata values (entry ID, group ID,
 *  priority, flags, position, &c), plus the API state for the entry and the
 *  hardware state for the entry.  Note the pattern and action data are not
 *  actually part of the struct (only pointers to them) but are appended to the
 *  struct when it is allocated.
 *
 *  The API copy is the part that is being manipulated by the APIs; the
 *  hardware copy is the copy that has been committed to hardware.  This allows
 *  tracking of behaviour similar to the BCM layer, where an entry can be
 *  manipulated by bits and then gets committed to the hardware explicity (or
 *  along with an explicit commit of all of the other entries in its group).
 *
 *  The pattern is one bit per instruction, including the start instruction and
 *  any internal padding, padding with zeroes to a whole byte.  Note gaps in
 *  the key are not included here unless they correspond to NOPs in the
 *  instructions for the group (we keep one pattern bit per instruction per
 *  entry rather than keeping a 'key-matching' pattern).
 *
 *  The action is one bit per aciton bit, including any internal padding, also
 *  padding with zeroes to a whole byte.  Gaps in the action are included here,
 *  since this ends up as a raw image that will be used unchanged when writing
 *  to the action memory block (this is the value looked up by the LRP
 *  microcode based upon the result from the RCE, used to determine what is to
 *  be done to a frame based upon the classification).
 *
 *  The SOC_C3_RCE_ENTRY_INSERTION_FLAGS are working flags that do not affect
 *  the entry behaviour but are for diagnostics on certain mechanisms for
 *  dealing with entry insertion.  They are not meant to be of general use.
 *
 *  SOC_C3_RCE_ENTRY_FLAG_RANGE_UPDATE is used internally for bookkeeping
 *  during entry shifts between filter sets.  It should never be set on an
 *  entry except during the applicable functions, so it should never be set
 *  from any API perspective.
 *
 *  SOC_C3_RCE_ENTRY_FLAG_EXISTS is used by warm boot to track entries.
 *
 *  The other entry flags (SOC_C3_RCE_ENTRY_*) are defined in rce.h (and
 *  explained there), since they are intended to be used by API clients.
 *
 *  The basis counts are the current basis value for the entry's counters.
 *  These values are adjusted whenever the entry moves within a filter set and
 *  updated when reading the entry's counters on demand (since reading them
 *  from the counter driver will also clear those values).  The code mostly
 *  assumes there are two basis counts (bytes and frames), but the define
 *  SOC_C3_RCE_NUM_BASIS_COUNTERS allows the space to be adjusted in case there
 *  is reason to update the code to support more or fewer basis counters.
 *
 *  NOTE: the '2' in sizing the pattern and action data pointer arrays refers
 *  to the API state and the HW state.  It is not reasonable to expect these to
 *  change unless something particularly complex (like an unwind history) is
 *  added to the code, and even then it might be another array within the array
 *  of two that is already here.
 */
#define SOC_C3_RCE_ENTRY_INSERTION_FLAGS   0xFFFF0000
#define SOC_C3_RCE_ENTRY_FLAG_RANGE_UPDATE 0x00008000
#define SOC_C3_RCE_ENTRY_FLAG_EXISTS       0x00004000
#define SOC_C3_RCE_NUM_BASIS_COUNTERS      2
typedef struct _soc_c3_rce_entry_desc_int_s {
    /* link entries within groups */
    struct _soc_c3_rce_entry_desc_int_s *entryPrev; /* prev entry this group */
    struct _soc_c3_rce_entry_desc_int_s *entryNext; /* next entry this group */
    /* metadata about the entry */
    uint32 entryFlags;                 /* entry flags */
    int entryId;                       /* entry ID for this entry */
    int entryGroup;                    /* member of which group */
    int entryPriority;                 /* relative priority within the group */
    unsigned int entryPosition;        /* position within the group */
    unsigned int entryPosBackout;      /* backout from temp/test moves */
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    unsigned int entryPosPrev;         /* previous position within the group */
    uint64 basisCounts[SOC_C3_RCE_NUM_BASIS_COUNTERS]; /* basis counters */
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    /* pointers to bytes in the appended byte array, API then HW for each */
    uint8 *pattData[2];                /* pointer to pattern */
    uint8 *actData[2][SOC_C3_RCE_RESULT_REGISTER_COUNT]; /* ptrs to actions */
} _soc_c3_rce_entry_desc_int_t;

/*
 *  Describe a group's qualifier(s) internally
 *
 *  This is like the qualifier data structure that is exposed, but internally
 *  we copy the string into the same alloc cell as the struct, so we can keep
 *  it around locally for various reasons.  This minor contrivance allows us to
 *  keep the 'const' restriction on qualifier name as exposed to the API.
 */
typedef struct _soc_c3_rce_qual_desc_int_s {
    soc_c3_rce_qual_type_t qualType;    /* qualifier type (above) */
    char *qualName;                     /* pointer to name for qualifier */
    unsigned int paramCount;            /* number of parameters */
    int *param;                         /* pointer to individual parameters */
} _soc_c3_rce_qual_desc_int_t;

/*
 *  Describe a group internally
 *
 *  A group will have a set of instructions that implement the comparisons for
 *  that group against the key provided to the RCE by the LRP microcode.  This
 *  set of instructions will be used to build the actual RCE program segment
 *  and pattern values for that group.
 *
 *  A group will also have a set of entries.  Each entry will have a set of
 *  pattern bits and a set of action bits; the pattern bits are used to fill in
 *  the pattern bits of the RCE program, while the action bits are treated
 *  opaquely and written to a table in the appropriate memory for the program.
 *
 *  Note that entries between groups are not folded together, and that the
 *  hardware operates in a way that implies an allocation granularity of 768
 *  entries, so if a group has but a single entry, it will have 767 free
 *  entries that will not be shared with other groups.  The application must
 *  take this into account when it creates groups, and should create as few
 *  groups as necessary according to the overall needs.
 *
 *  The _SOC_C3_RCE_*_INSERT_* flags are used for diagnostic purposes to keep
 *  track of how entries are managed.  They are not intended for use by the API
 *  client software (actually they appear in the upper 16b of the entry flags
 *  under certain conditions, though they are still only meant for diagnostics
 *  even under such conditions).
 *
 *  The _SOC_C3_RCE_GROUP_FLAG_COUNTER flag indicates that the group's result
 *  register points to an action table that uses action-indexed counters.  It
 *  is possible to have some groups with this flag set and others with it clear
 *  in a program that has multiple action tables (so uses multiple result
 *  registers to provide results to the LRP microcode).
 *
 *  _SOC_C3_RCE_GROUP_FLAG_EXISTS is used by warm boot to indicate that a group
 *  exists in the backing store.
 */
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_ENTRY_APPEND   0x00000001
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_ENTRY_INSERT   0x00000002
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_ENTRY_PREPEND  0x00000003
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_ENTRY_MASK     0x00000003
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_SPACE_APPEND   0x00000004
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_SPACE_PREPEND  0x00000008
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_SPACE_MASK     0x0000000C
#define _SOC_C3_RCE_GROUP_INSERT_FLAG_MASK           0x0000000F
#define _SOC_C3_RCE_ENTRY_INSERT_FLAG_HAS_PREV       0x00000010
#define _SOC_C3_RCE_ENTRY_INSERT_FLAG_HAS_NEXT       0x00000020
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
#define _SOC_C3_RCE_GROUP_FLAG_COUNTER               0x00010000
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
#define _SOC_C3_RCE_GROUP_FLAG_EXISTS                0x80000000
typedef struct _soc_c3_rce_group_desc_int_s {
    /* link groups within programs */
    struct _soc_c3_rce_group_desc_int_s *groupPrev; /* prev group this program */
    struct _soc_c3_rce_group_desc_int_s *groupNext; /* next group this program */
    /* where to put the result (bitmaps); at least one bit must be set */
    uint8 resultLrp;                    /* LRP result registers to use */
    uint8 resultRce;                    /* RCE result registers to use */
    uint8 resultLrpUniq;                /* unique LRP result bits */
    /* RCE supports numerous programs invoked by LRP */
    uint8 rceProgram;                   /* which program for this group */
    int groupId;                        /* group ID for this group */
    int groupPriority;                  /* group priority within the program */
    /* group flags */
#if (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC)
    uint32 groupFlags;                  /* last entry insertion flags */
#endif /* (C3_RCE_ENABLE_ACTION_INDEX_COUNTERS || C3_RCE_ENABLE_ENTRY_INSERT_HEURISTIC) */
    /* the qualifiers used by this group */
    unsigned int qualCount;             /* number of qualifiers used */
    _soc_c3_rce_qual_desc_int_t *qualData[SOC_C3_RCE_GROUP_QUAL_MAX]; /* qual data */
    unsigned int qualOffs[SOC_C3_RCE_GROUP_QUAL_MAX]; /* qual offset in ptrn */
    /* the information about this group's filter set(s) */
    unsigned int rangesPerFilterSet;    /* ranges per filter set */
    unsigned int maxFilterSets;         /* maximum number of filter sets */
    unsigned int instrCount;            /* number of RCE instructions */
    unsigned int instrBlocks;           /* number of RCE instr blocks for set */
    unsigned int dataSize;              /* size of pattern+action data */
    _soc_c3_rce_instruction_t *instr;   /* instructions in group's filter set */
    uint8 *defaultPattern;              /* default pattern for new entries */
    /* the information about this group's entries */
    unsigned int entryCount;            /* number of entries this group */
    int entryLastAdded;                 /* entry ID last added to group */
    unsigned int filterSetCount;        /* number of filter sets this group */
    _soc_c3_rce_entry_desc_int_t *entryHead; /* pointer to first entry descr */
    _soc_c3_rce_entry_desc_int_t *entryTail; /* pointer to last entry descr */
    
} _soc_c3_rce_group_desc_int_t;

/*
 *  Describe a filter set's reference to a single range.
 *
 *  Bit 31 of the flags is used by warm boot support to mark the default
 *  pattern bit for each instruction in the backing store filter set
 *  instruciton space (it is always zero for ranges, but other instructions in
 *  the template can require default pattern values of one).
 *
 *  Bit 30 of the flags is used by warm boot support to differentiate
 *  instructions that are part of the template from instructions that are
 *  generated per filter set (such as the start instruction and the range
 *  instructions).
 *
 *  Bits 11..15 of the flags value is the key doublebyte index.
 *
 *  Bit 10 of the flags value indicate whether the range really should be
 *  inverted (creation_invert ^ reference_invert).
 *
 *  Some other flags are as defined elsewhere (look for SOC_C3_RCE_RANGE_FLAG_*
 *  for these definitions).
 *
 *  The upper and lower limits are adjusted according to the LSb position in
 *  this structure, but not in the range descriptor itself.
 */
#define _SOC_C3_RCE_RANGE_FLAG_PATT_DEFAULT   (1 << 31)
#define _SOC_C3_RCE_RANGE_FLAG_IN_TEMPLATE    (1 << 30)
#define _SOC_C3_RCE_RANGE_FLAG_WORD_MASK      (0x1F << _SOC_C3_RCE_RANGE_FLAG_WORD_SHIFT)
#define _SOC_C3_RCE_RANGE_FLAG_WORD_SHIFT     11
#define _SOC_C3_RCE_RANGE_FLAG_MATCH_INVERTED (1 << 10)
typedef struct _soc_c3_rce_fset_range_info_s {
    int rangeId;                    /* range ID */
    uint32 rangeFlags;              /* flags for the range */
    uint16 refCount;                /* references to range in filter set */
    uint16 commitCount;             /* committed refs to range in fset */
    uint16 upperLimit;              /* upper limit (adjusted) */
    uint16 lowerLimit;              /* lower limit (adjusted) */
} _soc_c3_rce_fset_range_info_t;

/*
 *  Describe a filter set internally
 *
 *  Note instruction memory is global for the unit, but action memory is per
 *  action table.  Also, imemStart is in increments of 8 instructions, per the
 *  hardware limitations for where programs can start and end, and amemBlock is
 *  in increments of 768 entry positions, per the hardware limit for the number
 *  of entries in a single filter set.
 *
 *  When a filter set is created, the space between the START instruction and
 *  the first declared qualifier bit is padded with a number of NOPs that will
 *  be used for ranges as they are referenced.  The (minimum) value for this
 *  padding is specified during group creation.  The other instructions for the
 *  filter set are pushed out so the last active instruction is at offset 7, so
 *  there can be up to an additional 7 instructions available for range
 *  references.  This descriptor will have the appropriate number of slots in
 *  the array referenced by the range pointer.
 */
#define _SOC_C3_RCE_FSET_FLAGS_EXISTS  0x00000001
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
#define _SOC_C3_RCE_FSET_FLAGS_PHASE   0x00000002
#define _SOC_C3_RCE_FSET_FLAGS_COUNTER 0x00000004
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
typedef struct _soc_c3_rce_filterset_desc_int_s {
#if C3_RCE_ENABLE_ACTION_INDEX_COUNTERS
    uint32 fsetFlags;                       /* flags */
#endif /* C3_RCE_ENABLE_ACTION_INDEX_COUNTERS */
    int groupId;                            /* group ID */
    unsigned int filterIndex;               /* which filter set in the group */
    unsigned int imemStart;                 /* first instruction memory block */
    unsigned int imemSize;                  /* instruction memory blocks used */
    unsigned int amemBlock;                 /* action mem block */
    unsigned int entryCount;                /* entries in this filter set */
    _soc_c3_rce_entry_desc_int_t *entryHead;/* ptr to first entry in fset */
    _soc_c3_rce_fset_range_info_t *rangeInfo;/* ptr to range info array */
} _soc_c3_rce_filterset_desc_int_t;

/*
 *  Describe an action table internally
 *
 *  Since each program can return multiple results, and each result might point
 *  to a different action table, we need to keep track of information about
 *  action tables.  It is possible that more than one program would refer to a
 *  given action table, so this permits us to handle that in a manner that
 *  keeps resources for that table consistent.
 */
typedef struct _soc_c3_rce_actions_desc_int_s {
    /* microcode information */
    const _soc_c3_rce_actions_uc_desc_t *ucData; /* ptr to microcode data */
    /* action table information */
    unsigned int entryLimit;                 /* entry block limit */
    unsigned int entryActive;                /* active entry blocks */
    unsigned int firstEntryBias;             /* bias for first entry */
    /* action table entry layout */
    unsigned int actionBytes;                /* bytes in action 'pattern' */
    unsigned int actionCount;                /* number of actions */
    uint8 *defaultActions;                   /* default actions */
    soc_c3_rce_action_desc_t *actionData[SOC_C3_RCE_PROGRAM_ACTION_MAX]; /* acts */
} _soc_c3_rce_actions_desc_int_t;

/*
 *  Describe a program internally
 *
 *  A program will have a set of groups, each possibly with its own
 *  instructions, and a set of action tables which is common to all groups
 *  within that program (each group can use up to one action table, but more
 *  than one group can use a particular action table).
 *
 *  Each RCE program that can be invoked by the loaded LRP microcode will be
 *  considered separately.  Primarily because of timing, most of the standard
 *  LRP microcodes will probably support only two programs, ingress and egress,
 *  but some may support others, and custom LRP microcode images can support up
 *  to the hardware limit.
 *
 *  Unlike much else, pointers to this will be in an array that is kept
 *  per-unit, so it does not link to previous/next like the other structures,
 *  many of which are kept in doubly-linked lists.
 */
typedef struct _soc_c3_rce_program_desc_int_s {
    /* LRP microcode exported information */
    const _soc_c3_rce_program_uc_desc_t *ucData; /* microcode constants */
    /* action table entry layout */
    uint8 actIndex[SOC_C3_RCE_RESULT_REGISTER_COUNT]; /* action table indexes */
    /* filter sets in this program */
    unsigned int filterSetCount;             /* number of filter sets */
    unsigned int instrBlockCount;            /* instruction blocks count */
    unsigned int instrBlockAdded;            /* added blocks for padding */
    _soc_c3_rce_filterset_desc_int_t *fsetData; /* filter set descriptions */
    /* timing information */
    unsigned int keyTime;                    /* key begins, global clocks */
    unsigned int switchTime;                 /* result deadline, global clks */
    signed int keyToSwitch;                  /* key to swtich time */
    /* group information */
    unsigned int groupCount;                 /* number of groups in program */
    _soc_c3_rce_group_desc_int_t *groupHead; /* pointer to first group descr */
    _soc_c3_rce_group_desc_int_t *groupTail; /* pointer to last group descr */
} _soc_c3_rce_program_desc_int_t;

/*
 *  Describe an entire unit internally
 *
 *  We use shr_mres for dealing with most resources, but in order to deal with
 *  the need to defragment instruction memory, we use mdb to manage that (since
 *  mdb has switches to adjust alloc preferences).
 *
 *  The 'padding' filter sets are required to ensure that any program is at
 *  least 1024 instructions long.  This is a requirement due to pipeline
 *  timings of the hardware, to prevent the results from a program trampling
 *  the results from a previously run program during transfer.
 *
 *  SOC_C3_RCE_UNIT_RESTORE_FAIL indicates that the unit tried to restore from
 *  backing store but some critical problem caused this restore to fail.
 *  Backing store writes are disabled at this point until either reinit plus
 *  restore succeeds or until reinit without restore.
 *
 *  SOC_C3_RCE_UNIT_FLAGS_SWITCH2 indicates the unit is using switch-2 for at
 *  least the switch instructions that affect RCE programs.  If it is not set,
 *  the unit is using switch-1 for at least one RCE related switch.
 *
 *  SOC_C3_RCE_UNIT_FLAGS_OVERTIME is set when warm booting and the recovered
 *  configuration is over-time (whether timing enforcement is enabled or not).
 */
#define SOC_C3_RCE_UNIT_RESTORE_FAIL   0x80000000
#define SOC_C3_RCE_UNIT_FLAGS_OVERTIME 0x00000002
#define SOC_C3_RCE_UNIT_FLAGS_SWITCH2  0x00000001
typedef struct _soc_c3_rce_unit_desc_int_s {
    int unit;                                 /* unit number */
    /* hardware configuraiton information */
    uint32 unitFlags;                         /* flags for this unit */
    unsigned int epochTime;                   /* epoch length, global clocks */
    unsigned int instrBlockCount;             /* in use instruction blocks */
    unsigned int imemBlockCount;              /* active filter sets in imem */
    unsigned int lrpClockDivide;              /* LRP clock divider */
    unsigned int rceClockDivide;              /* RCE clock divider */
    /* data for all programs */
    _soc_c3_rce_program_desc_int_t *progData[SOC_C3_RCE_PROGRAM_COUNT];
    uint8 progOrder[SOC_C3_RCE_PROGRAM_COUNT]; /* execution order */
    /* data for all action tables */
    unsigned int actTableCount;               /* number of action tables */
    _soc_c3_rce_actions_desc_int_t *actData[(SOC_C3_RCE_PROGRAM_COUNT *
                                             SOC_C3_RCE_RESULT_REGISTER_COUNT)];
    /* data for groups */
    unsigned int groupLimit;                  /* maximum number of groups */
    _soc_c3_rce_group_desc_int_t **groupData; /* pointer to pointer table */
    /* data for entries */
    unsigned int entryLimit;                  /* maximum number of entries */
    unsigned int entryAvailLast;              /* last 'first avail' */
    _soc_c3_rce_entry_desc_int_t **entryData; /* pointer to pointer table */
    /* data for ranges */
    unsigned int rangeLimit;                  /* maximum number of ranges */
    _soc_c3_rce_range_desc_int_t **rangeData; /* pointer to pointer table */
    /* resource management */
    shr_mdb_list_handle_t imemRes;            /* instruction mem resource */
    shr_mres_handle_t amemRes;                /* action memory resources */
    unsigned int imemExtra[C3_RCE_FILTER_SET_COUNT_PADDING]; /* padding fsets */
#ifdef BCM_WARM_BOOT_SUPPORT
    /* warm boot support */
    soc_scache_handle_t scHandle;             /* scache handle for this unit */
    unsigned int scSize;                      /* size of scache for this unit */
    uint8 *scPtr;                             /* scache pointer for this unit */
#endif /* def BCM_WARM_BOOT_SUPPORT */
    /* actual tables for groupData and entryData will be appended on alloc */
} _soc_c3_rce_unit_desc_int_t;

#ifdef BCM_WARM_BOOT_SUPPORT
/*
 *  Function
 *    _soc_c3_rce_wb_purge
 *  Purpose
 *    Purge/free backing store state for a unit
 *  Arguments
 *    (IN/OUT) unitData = pointer to unit information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This is only called when about to destroy the unitData for a unit, which
 *    should only occur if detaching the unit or when init has failed.
 */
extern int
_soc_c3_rce_wb_purge(_soc_c3_rce_unit_desc_int_t *unitData);

/*
 *  Function
 *    _soc_c3_rce_wb_init
 *  Purpose
 *    Init or reset backing store state for a unit, or recover state from
 *    backing store for a unit
 *  Arguments
 *    (IN/OUT) unitData = pointer to unit information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    If SOC_WARM_BOOT(unit) is FALSE, this sets up the backing store state,
 *    allocating the space if needed.
 *
 *    If SOC_WARM_BOOT(unit) is not FALSE, this tries to recover the unit state
 *    from the backing store and hardware state.
 */
extern int
_soc_c3_rce_wb_init(_soc_c3_rce_unit_desc_int_t *unitData);

/*
 *  Function
 *    _soc_c3_rce_wb_size_calc
 *  Purpose
 *    Calculate the size of the warm boot backing store needed by RCE
 *  Arguments
 *    (IN/OUT) unitData = pointer to unit information
 *    (OUT) expectedSize = where to put the required backing store size
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    Assumes current version is desired.  It is possible for older versions to
 *    be smaller than the current version, in which case the backing store
 *    space will need to be resized to meet the new needs if moving between
 *    versions; this allows that to be checked.
 */
extern int
_soc_c3_rce_wb_size_calc(_soc_c3_rce_unit_desc_int_t *unitData,
                         unsigned int *expectedSize);

/*
 *  Function
 *    _soc_c3_rce_wb_full_sync
 *  Purpose
 *    Initialise the warm boot space (or upgrade it to the current version)
 *  Arguments
 *    (IN/OUT) unitData = pointer to unit information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This always clears the warm boot space and then writes everything out.
 *
 *    It does not bother filling in individual stuff that does not exist.
 *
 *    It assumes there is enough space and that the space is already allocated.
 *
 *    If autosync is disabled, this will not commit the changes but will mark
 *    the backing store as 'dirty'.  If autosync is enabled, this will commit
 *    the entire RCE space.
 */
extern int
_soc_c3_rce_wb_full_sync(_soc_c3_rce_unit_desc_int_t *unitData);

/*
 *  Function
 *    _soc_c3_rce_wb_immed_sync
 *  Purpose
 *    Initialise the warm boot space (or upgrade it to the current version)
 *  Arguments
 *    (IN/OUT) unitData = pointer to unit information
 *  Returns
 *    soc_result_t cast as int
 *      SOC_E_NONE if successful
 *      SOC_E_* appropriately otherwise
 *  Notes
 *    This always clears the warm boot space and then writes everything out.
 *
 *    It does not bother filling in individual stuff that does not exist.
 *
 *    It assumes there is enough space and that the space is already allocated.
 *
 *    If autosync is disabled, this will not commit the changes but will mark
 *    the backing store as 'dirty'.  If autosync is enabled, this will commit
 *    the entire RCE space.
 *
 *    This does not force realloc of the backing store space for RCE; that
 *    would already have been done during init if the version has changed.
 */
extern int
_soc_c3_rce_wb_immed_sync(_soc_c3_rce_unit_desc_int_t *unitData);

#endif /* def BCM_WARM_BOOT_SUPPORT */


#endif /* def BCM_CALADAN3_SUPPORT */

