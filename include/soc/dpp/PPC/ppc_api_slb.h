/* $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 */
#if !defined(__PPC_API_SLB_INCLUDED__) && !defined(PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY)

#define __PPC_API_SLB_INCLUDED__

/*************
 * INCLUDES  *
 *************/

#include <soc/dpp/SAND/Utils/sand_header.h>

#include <soc/dpp/SAND/Management/sand_general_macros.h>
#include <soc/dpp/SAND/Management/sand_error_code.h>

#include <soc/dpp/PPC/ppc_api_general.h>

/* Append args after evaluation. */
#define SOC_PPC_SLB_GLUE(a, b) a ## b
#define SOC_PPC_SLB_C_ASSERT_impl(exp, line, file) typedef char SOC_PPC_SLB_GLUE(assertion_failed_, line)[(exp) ? 1 : -1]

/* Compile time assert. */
#define SOC_PPC_SLB_C_ASSERT(exp) SOC_PPC_SLB_C_ASSERT_impl(exp, __LINE__, __FILE__)

#define SOC_PPC_SLB_MAX_ENTRIES_FOR_GET_BLOCK 130

/* This is c-inheritance. */
/* Abstract "classes" should not be instantiated. */
/* The point is that the inherited "classes" (structs) can be cast to ANY base class. */
/* (Because the struct is flattened and the base is always the first member). */
/* IMPORTANT: Only pointers to instances can be used, since an object might be bigger than its storage. */
/* Therefore, when passing an object a pointer must be passed, and for an array a pointer to pointer must be passed. */

/* Reuse and Extensibility: */
/* There are two main opportunities for extensibility: derivation from a concrete class or derivation from an abstract class. */

/* Class Diagram: 
 * ============== 
 * 
 * TRAVERSE_MATCH_RULE (*)
 * | 
 * |-ALL_ECMP 
 * |-ALL_LAG 
 * | 
 * |-LB_GROUP (*)
 * | |-LAG
 * | |-ECMP 
 * |  
 * |-LB_GROUP_MEMBER (*)
 *   |-LAG
 *   |-ECMP
 *  
 * TRAVERSE_ACTION (*)
 * | 
 * |-UPDATE 
 * |-REMOVE 
 * |-COUNT 
 *  
 * TRAVERSE_UPDATE_VALUE (*) 
 * | 
 * |-LAG_MEMBER 
 * |-ECMP_MEMBER
 *  
 * ENTRY_KEY 
 *  
 * ENTRY_VALUE 
 *  
 * CONFIGURATION_ITEM (*) 
 * | 
 * |-SLB_ENTRY_AGING_TIME_IN_SECONDS 
 * |-LAG_HASH_FUNCTION 
 * |-LAG_HASH_SEED
 * |-LAG_HASH_OFFSET
 * |-ECMP_HASH_SEED 
 * |-ECMP_HASH_KEY_OFFSET 
 * |-CRC_HASH_SEED 
 * |-CRC_HASH_MASK 
 *  
 * NOTES
 * 1) * - Abstract class.
 * 2) We drop the file prefix (SOC_PPC_SLB).
 * 3) At each level of the tree we drop the prefix of the super class.
 */

typedef enum {
    SOC_PPC_SLB_OBJECT_TYPE_BASE = 0,

    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_ALL_LAG,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_ALL_ECMP,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_LB_GROUP,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_LB_GROUP_LAG,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_LB_GROUP_ECMP,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_LAG,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_ECMP,

    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_ACTION,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_ACTION_COUNT,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_ACTION_UPDATE,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_ACTION_REMOVE,

    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_UPDATE_VALUE,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_UPDATE_VALUE_LAG_MEMBER,
    SOC_PPC_SLB_OBJECT_TYPE_TRAVERSE_UPDATE_VALUE_ECMP_MEMBER,

    SOC_PPC_SLB_OBJECT_TYPE_ENTRY_KEY,
    SOC_PPC_SLB_OBJECT_TYPE_ENTRY_VALUE,

    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_SLB_ENTRY_AGING_TIME_IN_SECONDS,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_LAG_HASH_FUNCTION,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_LAG_HASH_SEED,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_LAG_HASH_OFFSET,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_ECMP_HASH_SEED,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_ECMP_HASH_KEY_OFFSET,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_CRC_HASH_SEED,
    SOC_PPC_SLB_OBJECT_TYPE_CONFIGURATION_ITEM_CRC_HASH_MASK,
    SOC_PPC_SLB_NOF_OBJECT_TYPES
} SOC_PPC_SLB_OBJECT_TYPE;

/* Abstract. Should not be "instantiated". */
typedef struct {
  SOC_SAND_MAGIC_NUM_VAR
  SOC_PPC_SLB_OBJECT_TYPE type;
} SOC_PPC_SLB_OBJECT;

typedef struct {
  SOC_PPC_SLB_OBJECT base;
  uint32 lag_ndx;
} SOC_PPC_SLB_LAG_GROUP;

typedef struct {
  SOC_PPC_SLB_OBJECT base;
  SOC_PPC_FEC_ID ecmp_ndx;
} SOC_PPC_SLB_ECMP_GROUP;

typedef struct {
  SOC_PPC_SLB_OBJECT base;
  uint32 lag_member_ndx;
} SOC_PPC_SLB_LAG_MEMBER;

typedef struct {
  SOC_PPC_SLB_OBJECT base;
  uint32 ecmp_member_ndx;
} SOC_PPC_SLB_ECMP_MEMBER;

/* This is the base match rule. Abstract class. */
typedef struct {
  SOC_PPC_SLB_OBJECT base;
} SOC_PPC_SLB_TRAVERSE_MATCH_RULE;

  /* Match all LAG entries. */
  typedef struct {
    SOC_PPC_SLB_TRAVERSE_MATCH_RULE base;
  } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_ALL_LAG;

  /* Match all ECMP entries. */
  typedef struct {
    SOC_PPC_SLB_TRAVERSE_MATCH_RULE base;
  } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_ALL_ECMP;

  /* Match LB group. */
  typedef struct {
    SOC_PPC_SLB_TRAVERSE_MATCH_RULE base;
  } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP;

    /* Match LAG group. */
    typedef struct {
      SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP base;
      uint32 lag_ndx;
    } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_LAG;

    /* Match ECMP group. */
    typedef struct {
      SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP base;
      SOC_PPC_FEC_ID ecmp_ndx;
    } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_ECMP;

  /* Match LB group member. */
  typedef struct {
    SOC_PPC_SLB_TRAVERSE_MATCH_RULE base;
  } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER;

    /* Match LAG member. */
    typedef struct {
      SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER base;
      uint32 lag_member_ndx;
    } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_LAG;

    /* Match ECMP member. */
    typedef struct {
      SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER base;
      SOC_PPC_FEC_ID ecmp_member_ndx;
    } SOC_PPC_SLB_TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_ECMP;

typedef struct {
  SOC_PPC_SLB_OBJECT base;

} SOC_PPC_SLB_TRAVERSE_UPDATE_VALUE;

  typedef struct {
    SOC_PPC_SLB_TRAVERSE_UPDATE_VALUE base;

    /* The value of the new LAG member. */
    uint32 new_lag_member_ndx;
  } SOC_PPC_SLB_TRAVERSE_UPDATE_VALUE_LAG_MEMBER;

  typedef struct {
    SOC_PPC_SLB_TRAVERSE_UPDATE_VALUE base;

    /* The value of the new ECMP member (FEC). */
    uint32 new_ecmp_member_ndx;
  } SOC_PPC_SLB_TRAVERSE_UPDATE_VALUE_ECMP_MEMBER;

typedef struct {
  SOC_PPC_SLB_OBJECT base;

} SOC_PPC_SLB_TRAVERSE_ACTION;

  typedef struct {
    SOC_PPC_SLB_TRAVERSE_ACTION base;
  } SOC_PPC_SLB_TRAVERSE_ACTION_COUNT;

  typedef struct {
    SOC_PPC_SLB_TRAVERSE_ACTION base;
  } SOC_PPC_SLB_TRAVERSE_ACTION_REMOVE;

  typedef struct {
    SOC_PPC_SLB_TRAVERSE_ACTION base;

    /* Specify which value should be updated. */
    SOC_PPC_SLB_TRAVERSE_UPDATE_VALUE *traverse_update_value;
  } SOC_PPC_SLB_TRAVERSE_ACTION_UPDATE;

/*
 * Key and value for the table (get_block). 
 */
typedef struct {
  SOC_PPC_SLB_OBJECT base;

  /* The hash value (flow label) of the entry. */
  /* 47 bits. */
  uint64 flow_label_id;

  /* The destination LB group. */
  /* If (is_fec != 0) then ECMP, otherwise LAG. */
  union {
    uint32 lag_ndx;
    SOC_PPC_FEC_ID ecmp_ndx;
  } lb_group;

  /* See lb_group. */
  uint8 is_fec;

} SOC_PPC_SLB_ENTRY_KEY;

typedef struct {
  SOC_PPC_SLB_OBJECT base;

  /* The LAG group. */
  uint32 lag_ndx;

  /* The LAG member. */
  uint32 lag_member_ndx;
  
  /* If non-zero then the lag member info is valid. */
  uint8 lag_valid;

  /* FEC pointer (the selected member in the ECMP group). */
  SOC_PPC_FEC_ID ecmp_member_ndx;

  /* If non-zero then the ecmp member info is valid. */
  uint8 ecmp_valid;

} SOC_PPC_SLB_ENTRY_VALUE;

/* Abstract class. */
typedef struct {
  SOC_PPC_SLB_OBJECT base;
} SOC_PPC_SLB_CONFIGURATION_ITEM;

/* Set/get the aging time of SLB entries in seconds. */
/* When setting, the chosen time is the minimal time that is bigger or equal than the requested time, and available. */
/* When getting, the closest integer to the actual value of the aging time that was passed is returned. */
/* IMPORTANT: Get calculates the approximate hardware time based on the passed value, and not what is actually in the HW.*/
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  uint32 age_time_in_seconds;
} SOC_PPC_SLB_CONFIGURATION_ITEM_SLB_ENTRY_AGING_TIME_IN_SECONDS;

/* Set/get the LAG hash function. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  /* Valid: SOC_PPC_LAG_LB_XXX. */
  uint32 hash_function;
} SOC_PPC_SLB_CONFIGURATION_ITEM_LAG_HASH_FUNCTION;

/* Set/get the LAG hash seed. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  uint16 seed;
} SOC_PPC_SLB_CONFIGURATION_ITEM_LAG_HASH_SEED;

/* Set/get the LAG shift. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  /* 4 bits. */
  uint8 offset;
} SOC_PPC_SLB_CONFIGURATION_ITEM_LAG_HASH_OFFSET;

/* Set/get the ECMP hash seed. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  uint16 seed;
} SOC_PPC_SLB_CONFIGURATION_ITEM_ECMP_HASH_SEED;

/* Set/get the ECMP Key shift. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  /* 6 bits. */
  uint8 offset;

  /* 2 keys*/
  uint8 index;

} SOC_PPC_SLB_CONFIGURATION_ITEM_ECMP_HASH_KEY_OFFSET;

/* Set/get the CRC functions hash seed. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  uint16 seed;
} SOC_PPC_SLB_CONFIGURATION_ITEM_CRC_HASH_SEED;

/* Set/get the 2byte resolution mask for CRC functions. */
typedef struct {
  SOC_PPC_SLB_CONFIGURATION_ITEM base;

  uint16 mask;

  /*4 masks*/
  uint8 index;
} SOC_PPC_SLB_CONFIGURATION_ITEM_CRC_HASH_MASK;

/*************
 * FUNCTIONS *
 *************/

void SOC_PPC_SLB_clear(SOC_SAND_OUT SOC_PPC_SLB_OBJECT *object, SOC_SAND_IN SOC_PPC_SLB_OBJECT_TYPE type);
#define SOC_PPC_SLB_CLEAR(object, type) SOC_PPC_SLB_clear((SOC_PPC_SLB_OBJECT*)object, type)
#define SOC_PPC_SLB_DOWNCAST(object, type) ((type *)object)

#if SOC_PPC_DEBUG_IS_LVL1

void SOC_PPC_SLB_OBJECT_print(SOC_SAND_IN  SOC_PPC_SLB_OBJECT *object);

#endif /* SOC_PPC_DEBUG */

/* SOC API. */

/* If error is returned then traverse stops. */
typedef uint32 (*soc_ppc_slb_traverse_type_tree_cb)(SOC_SAND_IN SOC_PPC_SLB_OBJECT *object, SOC_SAND_IN SOC_PPC_SLB_OBJECT_TYPE type, void *opaque);

/* Object is type. E.g. if D derives from B then an instance of B is B, but not is D, and an instance of D is B and is D. */
uint32 soc_ppc_slb_object_is(SOC_SAND_IN SOC_PPC_SLB_OBJECT *object, SOC_SAND_IN SOC_PPC_SLB_OBJECT_TYPE type, SOC_SAND_OUT uint8 *is_type);

/* Traverse the type tree of the object. */
uint32 soc_ppc_slb_object_traverse_type_tree(SOC_SAND_IN SOC_PPC_SLB_OBJECT *object, SOC_SAND_IN soc_ppc_slb_traverse_type_tree_cb callback, SOC_SAND_IN void *opaque);
uint32 soc_ppc_slb_object_type(SOC_SAND_IN SOC_PPC_SLB_OBJECT *object, SOC_SAND_OUT SOC_PPC_SLB_OBJECT_TYPE *type);

/* Initialize the object system. Should be called from the device init. */
uint32 soc_ppc_slb_init(void);

#include <soc/dpp/SAND/Utils/sand_footer.h>

#elif defined(PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY)
/* Paste the list of objects. */

#ifndef PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION /* (object_suffix) */
# error "PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION must be defined"
#endif

/* WARNING - OBJECT is missing from this list. */

PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_ALL_LAG)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_ALL_ECMP)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_LB_GROUP)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_LB_GROUP_LAG)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_LB_GROUP_ECMP)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_LAG)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_MATCH_RULE_LB_GROUP_MEMBER_ECMP)

PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_ACTION)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_ACTION_COUNT)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_ACTION_REMOVE)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_ACTION_UPDATE)

PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_UPDATE_VALUE)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_UPDATE_VALUE_LAG_MEMBER)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(TRAVERSE_UPDATE_VALUE_ECMP_MEMBER)

PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(ENTRY_KEY)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(ENTRY_VALUE)

PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_SLB_ENTRY_AGING_TIME_IN_SECONDS)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_LAG_HASH_FUNCTION)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_LAG_HASH_SEED)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_LAG_HASH_OFFSET)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_ECMP_HASH_SEED)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_ECMP_HASH_KEY_OFFSET)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_CRC_HASH_SEED)
PPC_API_SLB_INTERNAL_OBJECT_LIST_ONLY_OBJECT_ACTION(CONFIGURATION_ITEM_CRC_HASH_MASK)

#endif /* __PPC_API_SLB_INCLUDED__ */

