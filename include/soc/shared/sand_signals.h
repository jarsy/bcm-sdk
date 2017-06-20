/*! \file sand_signals.h
 *
 * Contains definitions requested for fetching debug signals
 *
 */
/*
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */

#ifndef _SOC_DCMN_DSIG_H
#define _SOC_DCMN_DSIG_H

#include <shared/utilex/utilex_str.h>
#include <shared/utilex/utilex_rhlist.h>
#include <shared/dbx/dbx_xml.h>

#define DSIG_MAX_ADDRESS_RANGE_NUM  5
#define DSIG_MAX_SIZE_BITS          1152
#define DSIG_MAX_SIZE_BYTES         DSIG_MAX_SIZE_BITS / 8
#define DSIG_MAX_SIZE_UINT32        DSIG_MAX_SIZE_BITS / 32
#define DSIG_ADDRESS_MAX_SIZE       200
#define DSIG_OPTION_PARAM_MAX_NUM   5

#define DSIG_MAX_SIZE_STR           DSIG_MAX_SIZE_BYTES * 2 + 2 /* Maximal signal value transfered as a string */
#define DSIG_BITS_IN_RANGE          256

#define PRINT_LITTLE_ENDIAN     1
#define PRINT_BIG_ENDIAN        0

#define EXPANSION_STATIC    0
#define EXPANSION_DYNAMIC   1

#define SIGNALS_MATCH_EXACT     0x01
#define SIGNALS_MATCH_NOCOND    0x02
#define SIGNALS_MATCH_DOUBLE    0x04
#define SIGNALS_MATCH_HW        0x08
#define SIGNALS_MATCH_PERM      0x10
#define SIGNALS_MATCH_EXPAND    0x20
#define SIGNALS_MATCH_ONCE      0x40
#define SIGNALS_MATCH_NOVALUE   0x80

typedef struct {
    char *name;
    char *from;
    char *to;
    int  size;
    int  block_id;
    char *addr_str;
    char *expansion;
} static_signal_t;

typedef struct {
    char *name;
    static_signal_t *signals;
} static_block_t;

typedef struct {
    char *name;
    static_block_t *blocks;
} static_device_t;

typedef struct {
    char *name;
    char *bitstr;
} static_field_t;

typedef struct {
    char *name;
    int  size;
    static_field_t *fields;
} static_sigstruct_t;

typedef struct {
    int high;
    int low;
    int msb;
    int lsb;
} signal_address_t;

typedef struct debug_signal_s {
    int size;
    int block_id;
    signal_address_t address[DSIG_MAX_ADDRESS_RANGE_NUM];
    int range_num;
    int changeable;
    int double_flag;
    int perm;
    char expansion[RHNAME_MAX_SIZE];
    char resolution[RHNAME_MAX_SIZE];
    char from[RHNAME_MAX_SIZE];
    char to[RHNAME_MAX_SIZE];
    char block_n[RHNAME_MAX_SIZE];
    char attribute[RHNAME_MAX_SIZE];
    char hw[RHFILE_MAX_SIZE];
    char cond_attribute[RHNAME_MAX_SIZE];
    char addr_str[DSIG_ADDRESS_MAX_SIZE];
    uint32 value[DSIG_MAX_SIZE_UINT32];
    int  cond_value;
    struct debug_signal_s *cond_signal;
} debug_signal_t;

typedef struct
{
    char *block;
    char *stage;
    char *from;
    char *to;
    char *name;
    int flags;
    int output_order;
} match_t;

typedef struct
{
    int offset;
    int size;
    int buffer;
    char name[RHNAME_MAX_SIZE];
    char hw[RHFILE_MAX_SIZE];
} internal_signal_t;

typedef struct
{
    uint32 id;
    char name[RHNAME_MAX_SIZE];
    char programmable[RHKEYWORD_MAX_SIZE];
    internal_signal_t *signals;
    int number;
    int buffer0_size;
    int buffer1_size;
} pp_stage_t;

typedef struct
{
    char name[RHNAME_MAX_SIZE];
    int stage_num;
    pp_stage_t *stages;
    char debug_signals_n[RHNAME_MAX_SIZE];
    debug_signal_t *debug_signals;
    int signal_num;
} pp_block_t;

#define EXPANSION_STATIC    0
#define EXPANSION_DYNAMIC   1

/*
 * Sigstruct section
 */
typedef struct
{
    char name[RHNAME_MAX_SIZE];
    attribute_param_t param[DSIG_OPTION_PARAM_MAX_NUM];
} expansion_option_t;

typedef struct
{
    char name[RHNAME_MAX_SIZE];
    expansion_option_t *options;
    int option_num;
} expansion_t;

typedef struct
{
    char name[RHNAME_MAX_SIZE];
    int start_bit;
    int end_bit;
    int size;
    char resolution[RHNAME_MAX_SIZE];
    char cond_attribute[RHNAME_MAX_SIZE];
    int cond_value;
    expansion_t expansion_m;
} sigstruct_field_t;

typedef struct
{
    char name[RHNAME_MAX_SIZE];
    int size;
    sigstruct_field_t *fields;
    int field_num;
    expansion_t expansion_m;
} sigstruct_t;

typedef struct
{
    char name[RHNAME_MAX_SIZE];
    int value;
} sigparam_value_t;

typedef struct
{
    char name[RHNAME_MAX_SIZE];
    int size;
    char default_str[RHNAME_MAX_SIZE];
    sigparam_value_t *values;
    int value_num;
} sigparam_t;

typedef struct
{
    rhentry_t entry;
    rhlist_t *chip_list;
    pp_block_t *pp_blocks;
    int block_num;
    sigstruct_t *sigstructs;
    int sigstruct_num;
    sigparam_t *sigparams;
    int sigparam_num;
} device_t;

typedef struct
{
    rhentry_t entry;
    debug_signal_t *debug_signal;
    int size;
    int core;
    uint32 value[DSIG_MAX_SIZE_UINT32];
    char print_value[DSIG_MAX_SIZE_STR];
    char expansion[RHNAME_MAX_SIZE];
    device_t *device;
    rhlist_t *field_list;
} signal_output_t;

device_t *
sand_signal_device_get(
    int unit);

int
sand_signal_list_get(
    device_t * device,
    int unit,
    int core,
    match_t * match_p,
    rhlist_t * dsig_list);

void
sand_signal_list_free(
    rhlist_t * output_list);

/*
 * Function:
 *  dpp_dsig_read
 * Purpose:
 *  Get value for signal based on searching criteria
 * Parameters:
 *  unit, core - unit id and core id
 *  block - name of PP block, one of IRPP, ERPP, ETPP. If NULL all blocks will be searched for match
 *  from  - name of stage signal comes from, If null any from stages wil be searched for match
 *  to    - name of stage signal goes to, if null any to stage will be seacrched for match
 *  name  - full or partial name of signal requested
 *  value - pointer to buffer where signal value will be copied
 *  size  - size of buffer(bytes)
 * Returns:
 *  _SHR_E_NONE   - if signal was found and values successfully obtained
 *  _SHR_E_PARAM  - Bad parameters, like NULL pointer for value
 *  _SHR_E_MEMORY - Memory allocation problems
 *  _SHR_E_FOUND  - Signal was not found
 * Notes
 *  1. Use "diag pp sig" to verify existence and usage of signals
 *  2. Only Parameters with property "Perm" are available for diagnostics/testing
 */
int dpp_dsig_read(
    int unit,
    int core,
    char *block,
    char *from,
    char *to,
    char *name,
    uint32 * value,
    int size);

#endif /*_SOC_DCMN_DSIG_H*/
