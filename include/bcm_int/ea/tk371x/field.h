/*
 * $Id: field.h,v 1.15 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * This file contains filed process module definitions internal to
 * the BCM library.
 */

#ifndef _BCM_INT_EA_FIELD_H
#define _BCM_INT_EA_FIELD_H
#include <soc/ea/tk371x/TkRuleApi.h>
#include <sal/core/sync.h>
#include <sal/core/time.h>
#include <sal/core/thread.h>
#include <sal/types.h>
#include <bcm/types.h>
#include <bcm/field.h>
#include <bcm_int/common/field.h>

#define _TK371X_FP_DEBUG	0

#define _FP_PORT_EPON	0x0001
#define _FP_PORT_GE		0x0002
#define _FP_PORT_FE		0x0004
#define _FP_PORT_LLID1	0x0008
#define _FP_PORT_LLID2	0x0010
#define _FP_PORT_LLID3	0x0020
#define _FP_PORT_LLID4	0x0040
#define _FP_PORT_LLID5	0x0080
#define _FP_PORT_LLID6	0x0100
#define _FP_PORT_LLID7	0x0200
#define _FP_PORT_LLID8	0x0400

/* Generic memory allocation routine. */
#define _FP_MEM_ALLOC(_ptr_,_size_,_descr_)                 \
            do {                                             \
                if (NULL == (_ptr_)) {                       \
                   (_ptr_) = sal_alloc((_size_), (_descr_)); \
                }                                            \
                if((_ptr_) != NULL) {                        \
                    sal_memset((_ptr_), 0, (_size_));        \
                }  else {                                    \
                    LOG_ERROR(BSL_LS_BCM_FP, \
                              (BSL_META("FP Error: Allocation failure %s\n"), (_descr_))); \
                }                                          \
            } while (0)

#define _FP_PARAM_INVALID				(-1)
#define _FP_INVALID_INDEX            	(-1)
#define _FP_ZERO_VAL					0
#define _FP_DEFAULT_VOLATILES			1		/*don't store to tk371x nvs*/
#define _TK371X_FP_SUPPORTED 			0

#define _BCM_TK371X_FIELD_MATCH_NEVER_MATCH 0
#define _BCM_TK371X_FIELD_MATCH_EQUAL		1
#define _BCM_TK371X_FIELD_MATCH_NOTEQUAL	2
#define _BCM_TK371X_FIELD_MATCH_LESSTHEN	3
#define _BCM_TK371X_FIELD_MATCH_MORETHEN	4
#define _BCM_TK371X_FIELD_MATCH_EXISTS		5
#define _BCM_TK371X_FIELD_MATCH_NOTEXISTS	6
#define _BCM_TK371X_FIELD_MATCH_ALWAYSMATCH	7
#define _BCM_TK371X_FIELD_MATCH_ERROR		0x1F

/* range reference value */
#define _BCM_TK371X_FP_PORT_VALUE_MIN	0
#define _BCM_TK371X_FP_PORT_VALUE_MAX	0xFFFF
#define _BCM_TK371X_FP_VLAN_COS_MIN		0
#define _BCM_TK371X_FP_VLAN_COS_MAX		7
#define _BCM_TK371X_FP_VLAN_VID_MIN		0
#define _BCM_TK371X_FP_VLAN_VID_MAX		0x0FFF

#define _TK371X_FP_RULE_COND_VALUE_MAX	8

typedef uint32 bcm_field_udf_t;

/*
 * Initial group IDs and entry IDs.
 */
#define _FP_GROUP_ID_BASE 	1
#define _FP_GROUP_ID_MAX	(1024)
#define _FP_ENTRY_ID_BASE 	1
#define _FP_ENTRY_ID_MAX 	(1024)
#define _FP_RANGE_ID_BASE	1
#define _FP_RANGE_ID_MAX	(1024)

typedef enum {
	_bcmTk371xFieldNoOp                     = 0x00,
	_bcmTk371xFieldReserved0				= 0x01,
	_bcmTk371xFieldSetPath					= 0x02,
	_bcmTk371xFieldSetAddVlanTag			= 0x03,
	_bcmTk371xFieldSetDelVlanTag			= 0x04,
	_bcmTk371xFieldSetVidAndAddVlanTag		= 0x05,
	_bcmTk371xFieldSetCos					= 0x06,
	_bcmTk371xFieldReplaceVlanTag			= 0x07,
	_bcmTk371xFieldReplaceVlanTagAndSetVid	= 0x08,
	_bcmTk371xFieldClearAddVlanTag			= 0x09,
	_bcmTk371xFieldClearDelVlanTag          = 0x0A,
	_bcmTk371xFieldClearDelVlanTagAndSetAddVlanTag = 0x0B,
	_bcmTk371xFieldCopyFieldToCos			= 0x0C,
	_bcmTk371xFieldCopyFieldToVid			= 0x0D,
	_bcmTk371xFieldDiscard					= 0x0E,
	_bcmTk371xFieldReserved1				= 0x0F,
	_bcmTk371xFieldForward					= 0x10,
	_bcmTk371xFieldReserved2				= 0x11,
	_bcmTk371xFieldSetPathAndForward		= 0x12,
	_bcmTk371xFieldSetAddVlanTagAndForward	= 0x13,
	_bcmTk371xFieldSetDelVlanTagAndForward  = 20,	/*0x14*/
	_bcmTk371xFieldSetVidAndSetAddVlanTagAndForward = 0x15,
	_bcmTk371xFieldSetCosAndForward			= 0x16,
	_bcmTk371xFieldReplaceTagAndForward		= 0x17,
	_bcmTk371xFieldReplaceTagAndSetVidAndForward	= 0x18,
	_bcmTk371xFieldClearAddVlanTagAndForward		= 0x19,
	_bcmTk371xFieldClearDelVlanTagAndForward		= 0x1A,
	_bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward	= 0x1B,
	_bcmTk371xFieldCopyFieldToCosAndForward	= 0x1C,
	_bcmTk371xFieldCopyFieldToVidAndForward	= 0x1D,
	_bcmTk371xFieldReserved3                = 30,
	_bcmTk371xFieldreserved4
} _bcmTk371xFieldActionType;

/*
 * Typedef:
 *     _field_action_t
 * Purpose:
 *     This is the real action storage structure that is hidden behind
 *     the opaque handle bcm_field_action_t.
 */
typedef struct _bcm_tk371x_field_action_s {
    bcm_field_action_t     action;       /* action type               */
    uint32                 param0;       /* Action specific parameter */
    uint32                 param1;       /* Action specific parameter */
    struct _bcm_tk371x_field_action_s *next;
} _bcm_tk371x_field_action_t;

typedef enum _bcm_tk371x_field_entry_cond_qset_s{
	_bcmTk371xFieldEntryCondQsetEthernetDa 	= 0,
	_bcmTk371xFieldEntryCondQsetEthernetSa 	= 1,
	_bcmTk371xFieldEntryCondQsetUser0		= 2,
	_bcmTk371xFieldEntryCondQsetLlidIndex	= 2,
	_bcmTk371xFieldEntryCondQsetL2LengthType= 3,
	_bcmTk371xFieldEntryCondQsetVlanId		= 4,
	_bcmTk371xFieldEntryCondQsetUser1		= 5,
	_bcmTk371xFieldEntryCondQsetUser2		= 6,
	_bcmTk371xFieldEntryCondQsetUser3		= 7,
	_bcmTk371xFieldEntryCondQsetUser4		= 8,
	_bcmTk371xFieldEntryCondQsetUser5		= 9,
	_bcmTk371xFieldEntryCondQsetUser6		= 10,
	_bcmTk371xFieldEntryCondQsetL3ProtocolType	= 11,
	_bcmTk371xFieldEntryCondQsetIpDa		= 12,
	_bcmTk371xFieldEntryCondQsetIpv6DaHi	= 13,
	_bcmTk371xFieldEntryCondQsetIpSa		= 14,
	_bcmTk371xFieldEntryCondQsetIpv6SaHi	= 15
}_bcm_tk371x_field_entry_cond_qset_t;

/*
 * Typedef
 *     _bcm_tk371x_field_rule_cond_t
 * Purpose:
 * 	  This is the real storage to store tk371x rules conditions.
 */
typedef struct _bcm_tk371x_field_entry_cond_s{
	bcm_field_qualify_t 	qual_id;
	uint8 					qset;			/*It is useful to store the field conditions*/
	union {
		bcm_mac_t 	dst_mac;
		bcm_mac_t 	src_mac;
		uint16 		ether_type;
		uint16		vlan;
		uint8		pri;
		uint8		protocol;
		uint8		value[8];
	}common;
	uint8					match;
	struct _bcm_tk371x_field_entry_cond_s *next;
}_bcm_tk371x_field_entry_cond_t;

typedef struct _bcm_tk371x_fp_operator_val_s{
	uint8 index;
	uint8 val[0];
}_bcm_tk371x_fp_operator_val_t;

typedef LogicalPortIndex _bcm_tk371x_rule_logical_port_index_t;
typedef TkPortRuleInfo 	_bcm_tk371x_rule_info_t;
/*structure for storing tk rule information*/
typedef struct _bcm_tk371x_rule_s{
	_bcm_tk371x_rule_logical_port_index_t index;
	_bcm_tk371x_rule_info_t info;
}_bcm_tk371x_rule_t;

/*Definition tk371x flags*/
#define _TK371X_ENTRY_FLAG_INSTALLED	0x00000001
#define _TK371X_ENTRY_FLAG_NOT_EMPTY	0x00000002
#define _TK371X_ENTRY_FLAG_MODIFIED		0x00000004
#define _TK371X_ENTRY_FLAG_EMPTY_ACTION	0x00000008
#define _TK371X_ENTRY_FLAG_EMPTY_QUAL	0x00000010

/*Definition entry default priority */
#define _TK371X_ENTRY_PRIORITY_LOW		0
#define _TK371X_ENTRY_PRIORITY_MAX		15
#define _TK371X_ENTRY_PRIORITY_DEFAULT	15

typedef struct _bcm_tk371x_field_entry_derivative_s{
	_bcm_tk371x_rule_t	*rule1;
	_bcm_tk371x_rule_t	*rule2;
}_bcm_tk371x_field_entry_derivative_t;
/*
 * Typedef
 *     _bcm_tk371x_field_rule_t
 * Purpose:
 *     This is the real storage structure to store tk371x rules.
 */
typedef struct _bcm_tk371x_field_entry_s{
	int   entry_id;
	uint8 linkid;
	uint8 objType;
	uint8 index;
	uint8 volatiles;
	uint8 pri;
	uint32	flags;
	_bcm_tk371x_field_entry_cond_t *conds;
	uint8 soc_action;
	_bcm_tk371x_field_action_t *bcm_action;
	union {
		struct {
		    uint8       port_link;
		    uint8       queue;
		} RuleNameQueue;
		uint16 vid_cos;
	}rule_para;
	_bcm_tk371x_rule_t *backup;
	struct _bcm_tk371x_field_entry_s *next;
	struct _bcm_tk371x_field_entry_derivative_s *derivative;
}_bcm_tk371x_field_entry_t;

/*
 * Typedef
 *     _bcm_tk371x_field_group_t
 * Purpose:
 *     Management field group
 * */
typedef struct _bcm_tk371x_field_group_s{
	bcm_field_group_t 			group_id;
	_bcm_tk371x_field_entry_t	*entry;
	bcm_field_qset_t 			qset;
    /*
     * Public data for each group: The number of used and available entries,
     * counters, and meters for a field group.
     */
    bcm_field_group_status_t group_status;
	struct _bcm_tk371x_field_group_s *next;
}_bcm_tk371x_field_group_t;


/*
 * Typedef:
 *     _field_udf_t
 * Purpose:
 *     Holds user-defined field (UDF) hardware metadata.
 */
typedef struct _bcm_tk371x_field_udf_s {
    uint8                  valid;     /* Indicates valid UDF             */
    int                    use_count; /* Number of groups using this UDF */
    bcm_field_qualify_t    udf_num;   /* UDFn (UDF0 or UDF1)             */
} _bcm_tk371x_field_udf_t;


typedef struct _bcm_tk371x_field_tk_match_s{
	bcm_field_qualify_t	qid;
	uint8				field;
	int 				value;		/*qualify value*/
	uint8 				match;		/*tk qualify*/
	struct _bcm_tk371x_field_tk_match_s *next;
}_bcm_tk371x_field_tk_match_t;
/*
 * Typedef:
 *     _bcm_tk371x_field_range_t
 * Purpose:
 *     Internal management of Range Checkers. There are two styles or range
 *     checkers, the Firebolt style that only chooses source or destination
 *     port, without any sense of TCP vs. UDP or inverting. This style writes
 *     into the FP_RANGE_CHECK table. The Easyrider style is able to specify
 *     TCP vs. UDP.
 *     The choice of styles is based on the user supplied flags.
 *     If a Fire bolt style checker is sufficient, that will be used. If an
 *     Easy rider style checker is required then that will be used.
 *
 */
typedef struct _bcm_tk371x_field_range_s {
    uint32                 	flags;
    bcm_field_range_t      	rid;
    bcm_field_entry_t		eid;		/*valid to which entry*/
    bcm_l4_port_t          	min, max;
    _bcm_tk371x_field_tk_match_t 	*tk_match;
    struct _bcm_tk371x_field_range_s *next;
} _bcm_tk371x_field_range_t;


typedef struct _bcm_tk371x_field_control_s{
	_bcm_tk371x_field_udf_t		*udf;
	_bcm_tk371x_field_range_t	*ranges;	/* ranges */
    _bcm_tk371x_field_group_t 	*groups;		/* Group arrays */
}_bcm_tk371x_field_control_t;

extern int
_bcm_ea_field_operator_cond(uint8 *data, uint8 *mask, int len);
extern int
_bcm_ea_field_operator_cond_mask_get(uint8 match, uint8 *data, uint8 *mask, int len);

extern int _tk371x_field_thread_stop(int unit);

#endif /* _BCM_INT_EA_FIELD_H */
