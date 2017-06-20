/*
 * $Id: field.c,v 1.59 Broadcom SDK $
 *
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:     field.c
 * Purpose:
 *
 */
#include <shared/bsl.h>

#include <soc/cm.h>
#include <soc/drv.h>
#include <bcm/types.h>
#include <bcm/field.h>
#include <bcm/error.h>
#include <bcm_int/control.h>
#include <bcm_int/tk371x_dispatch.h>
#include <bcm_int/common/field.h>
#include <bcm_int/ea/init.h>
#include <bcm_int/ea/tk371x/field.h>
#include <bcm_int/ea/tk371x/port.h>
#include <bcm/cosq.h>
#ifdef BCM_FIELD_SUPPORT
/*
 *STATIC  Macro:
 *     FP_LOCK
 * Purpose:
 *     Lock take the Field control mutex
 */
#define FP_LOCK(control) \
    sal_mutex_take((control)->fc_lock, sal_mutex_FOREVER)
/*
 * Macro:
 *     FP_UNLOCK
 * Purpose:
 *     Lock take the Field control mutex
 */
#define FP_UNLOCK(control) \
    sal_mutex_give((control)->fc_lock)

static _bcm_tk371x_field_control_t     *_field_control = NULL;

/*
 * Macro:
 *     FIELD_IS_INIT (internal)
 * Purpose:
 *     Check that the unit is valid and confirm that the field functions
 *     are initialized.
 * Parameters:
 *     unit - BCM device number
 * Notes:
 *     Results in return(BCM_E_UNIT), return(BCM_E_UNAVAIL), or
 *     return(BCM_E_INIT) if fails.
 */
#define FIELD_IS_INIT(unit)                                      \
    if (!BCM_TK371X_UNIT_VALID(unit)) {                                 \
        return BCM_E_UNIT;                                       \
    }                                                            \
    if (!soc_feature(unit, soc_feature_field)) {                 \
        return BCM_E_UNAVAIL;                                    \
    }                                                            \
    if (_field_control == NULL) {                          \
        LOG_ERROR(BSL_LS_BCM_FP, \
                  (BSL_META_U(unit, \
                              "FP Error: unit=%d not initialized\n"),   \
                   unit));   \
        return BCM_E_INIT;                                       \
    }


/*
 * Function:
 *     _bcm_tk371x_field_qual_name
 * Purpose:
 *     Translate a Qualifier enum value to a text string.
 * Parameters:
 *     Enum value from bcm_field_qualify_e. (ex.bcmFieldQualifyInPorts)
 * Returns:
 *     Text name of indicated qualifier enum value.
 */
STATIC char *
_bcm_tk371x_field_qual_name(bcm_field_qualify_t qid)
{
    /* Text names of the enumerated qualifier IDs. */
    static char *qual_text[bcmFieldQualifyCount] = BCM_FIELD_QUALIFY_STRINGS;
    return (qid >= bcmFieldQualifyCount ? "??" : qual_text[qid]);
}

/*
 * Function:
 *     _bcm_tk371x_field_qset_dump
 * Purpose:
 *     Output qualiers set in 'qset'.
 */
void
_bcm_tk371x_field_qset_dump(char *prefix, bcm_field_qset_t qset, char *suffix)
{
    bcm_field_qualify_t qual;
    int first_qual = 1;

    if (prefix == NULL) {
        prefix = "";
    }
    if (suffix == NULL) {
        suffix = "";
    }

    LOG_CLI(("%s{", prefix));
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if (BCM_FIELD_QSET_TEST(qset, qual)) {
            LOG_CLI(("%s%s", (first_qual ? "" : ", "),
            		_bcm_tk371x_field_qual_name(qual)));
            first_qual = 0;
        }
    }
    LOG_CLI(("}%s", suffix));
}


/*
 * Function:
 *     _bcm_tk371x_field_control_get
 * Purpose:
 *     Lookup a FP control config from a bcm device id.
 * Parameters:
 *     unit -  (IN)BCM unit number.
 *     fc   -  (OUT) Field control structure.
 * Retruns:
 *     BCM_E_XXX
 */
int
_bcm_tk371x_field_control_get(int unit, _bcm_tk371x_field_control_t *fc)
{
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: _bcm_tk371x_field_control_get(%d)\n"),
               unit));
    /* Make sure system was initialized. */
    FIELD_IS_INIT(unit);

    if (fc == NULL){
    	return BCM_E_INIT;
    }
    /* Fill field control structure. */
    return (BCM_E_NONE);
}

/*
 * Function:
 *     _bcm_tk371x_field_action_name
 * Purpose:
 *     Return text name of indicated action enum value.
 */
#ifdef BROADCOM_DEBUG
STATIC char *
_bcm_tk371x_field_action_name(bcm_field_action_t action)
{
    /* Text names of Actions. These are used for debugging output and CLIs.
     * Note that the order needs to match the bcm_field_action_t enum order.
     */

    static char *action_text[] = BCM_FIELD_ACTION_STRINGS;
    assert(COUNTOF(action_text)     == bcmFieldActionCount);
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META("FP: _bcm_tk371x_field_action_name\n")));
    return (action >= bcmFieldActionCount ? "??" : action_text[action]);
}
#endif

/*
 * Function: _bcm_tk371x_field_control_free
 *
 * Purpose:
 *    Free field_control structure.
 *
 * Parameters:
 *     unit - (IN) BCM device number
 *     fc   - (IN) Field control structure.
 *
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_tk371x_field_control_free(int unit, _bcm_tk371x_field_control_t *fc)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_control_free(unit=%d)\n"),
	           unit));
    _field_control = NULL;
    if (NULL == fc) {
        return (BCM_E_NONE);
    }
    /* Free module control structure. */
    sal_free(fc);

    return (BCM_E_NONE);
}

/*
 * Function:
 *     _bcm_tk371x_field_range_get
 * Purpose:
 *     Lookup a _bcm_tk371x_field_range_t from a unit ID and slice choice.
 * Parmeters:
 *     unit   - BCM Unit
 *     rid    - Range ID
 *     entry_p - (OUT) Entry lookup result.
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_tk371x_field_range_get(int unit, bcm_field_range_t rid,
							_bcm_tk371x_field_range_t **range_p){
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_range_t		*fr = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_entry_get(unit=%d)\n"),
	           unit));
    fr = fc->ranges;
    while (fr != NULL){
    	if (fr->rid == rid){
    		*range_p = fr;
    		return BCM_E_NONE;
    	}
    	fr = fr->next;
    }
    return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_tk371x_vlan_cos_val_get
 * Purpose:
 *     compute the vlan's cos
 * Parmeters:
 *     vlan   - BCM vlan
 *     cos	  - (OUT) vlan's cos value
 * Returns:
 *    cos value
 */
STATIC uint8
_bcm_tk371x_vlan_cos_val_get(uint16 vlan, uint8 *cos){
	*cos = (vlan & 0xE000) >> 13;
	return *cos;
}

/*
 * Function:
 *     _bcm_tk371x_vlan_vid_val_get
 * Purpose:
 *     compute the vlan's vid
 * Parmeters:
 *     vlan   - BCM vlan
 *     cos	  - (OUT) vlan's vid value
 * Returns:
 *    vid value
 */
STATIC uint8
_bcm_tk371x_vlan_vid_val_get(uint16 vlan, uint16 *vid){
	*vid = (vlan & 0x0FFF);
	return *vid;
}

/*
 * Function:
 *     _bcm_tk371x_range_l4srcport_update
 * Purpose:
 *     Updated bcmFieldQualifyL4SrcPort value
 * Parmeters:
 *     fr --(IN) _bcm_field_range_t
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_tk371x_range_l4srcport_update(_bcm_tk371x_field_range_t *fr){
	_bcm_tk371x_field_tk_match_t *tk_match = NULL;
	bcm_field_qualify_t qid = bcmFieldQualifyL4SrcPort;

	if (fr->min == fr->max){
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser4;
		tk_match->qid = qid;
		tk_match->value = fr->max;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_EQUAL;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}else if (fr->min == _BCM_TK371X_FP_PORT_VALUE_MIN){
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser4;
		tk_match->value = fr->max;
		tk_match->qid = qid;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}else if (fr->max == _BCM_TK371X_FP_PORT_VALUE_MAX){
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser4;
		tk_match->value = fr->min;
		tk_match->qid = qid;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}else{
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser4;
		tk_match->value = fr->min;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	    tk_match = NULL;
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser4;
		tk_match->qid = qid;
		tk_match->value = fr->max;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}
	return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tk371x_range_l4dstport_update
 * Purpose:
 *     Updated bcmFieldQualifyL4DstPort value
 * Parmeters:
 *     fr --(IN) _bcm_field_range_t
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_tk371x_range_l4dstport_update(_bcm_tk371x_field_range_t *fr){
	_bcm_tk371x_field_tk_match_t *tk_match = NULL;
	bcm_field_qualify_t qid = bcmFieldQualifyL4DstPort;

	if (fr->min == fr->max){
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser5;
		tk_match->qid = qid;
		tk_match->value = fr->max;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_EQUAL;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}else if (fr->min == _BCM_TK371X_FP_PORT_VALUE_MIN){
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser5;
		tk_match->qid = qid;
		tk_match->value = fr->max;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}else if (fr->max == _BCM_TK371X_FP_PORT_VALUE_MAX){
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser5;
		tk_match->qid = qid;
		tk_match->value = fr->min;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}else{
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser5;
		tk_match->qid = qid;
		tk_match->value = fr->min;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	    tk_match = NULL;
		_FP_MEM_ALLOC(tk_match,
				sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
		tk_match->field = _bcmTk371xFieldEntryCondQsetUser5;
		tk_match->qid = qid;
		tk_match->value = fr->max;
		tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
	    tk_match->next = fr->tk_match;
	    fr->tk_match = tk_match;
	}
	return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tk371x_range_vlan_update
 * Purpose:
 *     Updated bcmFieldQualifyOuterVlanPri and  bcmFieldQualifyOuterVlanId value
 * Parmeters:
 *     fr --(IN) _bcm_field_range_t
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_tk371x_range_vlan_update(
		_bcm_tk371x_field_range_t *fr){
	_bcm_tk371x_field_tk_match_t *tk_match = NULL;
	bcm_field_qualify_t qid;
	uint8  min_cos, max_cos;
	uint16 min_vid, max_vid;

	_bcm_tk371x_vlan_cos_val_get((uint16)fr->min, &min_cos);
	_bcm_tk371x_vlan_cos_val_get((uint16)fr->max, &max_cos);
	_bcm_tk371x_vlan_vid_val_get((uint16)fr->min, &min_vid);
	_bcm_tk371x_vlan_vid_val_get((uint16)fr->max, &max_vid);

	if (min_vid == 0 && max_vid == 0){
		qid = bcmFieldQualifyOuterVlanPri;
		if (min_cos == max_cos){
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetUser6;
			tk_match->qid = qid;
			tk_match->value = min_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_EQUAL;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}else if (min_cos == _BCM_TK371X_FP_VLAN_COS_MIN){
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetUser6;
			tk_match->qid = qid;
			tk_match->value = max_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}else if (max_cos == _BCM_TK371X_FP_VLAN_COS_MAX){
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetUser6;
			tk_match->qid = qid;
			tk_match->value = min_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}else{
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetUser6;
			tk_match->qid = qid;
			tk_match->value = min_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
			tk_match = NULL;
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetUser6;
			tk_match->qid = qid;
			tk_match->value = max_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}
	}
	if (min_cos == 0 && max_cos == 0){
		qid = bcmFieldQualifyOuterVlanId;
		if (min_vid == max_vid){
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetVlanId;
			tk_match->qid = qid;
			tk_match->value = max_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_EQUAL;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}else if (min_vid == _BCM_TK371X_FP_VLAN_VID_MIN){
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetVlanId;
			tk_match->qid = qid;
			tk_match->value = max_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}else if (max_vid == _BCM_TK371X_FP_VLAN_VID_MAX){
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetVlanId;
			tk_match->qid = qid;
			tk_match->value = min_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}else{
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetVlanId;
			tk_match->qid = qid;
			tk_match->value = min_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_MORETHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
			tk_match = NULL;
			_FP_MEM_ALLOC(tk_match,
					sizeof(_bcm_tk371x_field_tk_match_t), "_bcm_tk371x_tk_match_t");
			tk_match->field = _bcmTk371xFieldEntryCondQsetVlanId;
			tk_match->qid = qid;
			tk_match->value = max_cos;
			tk_match->match = _BCM_TK371X_FIELD_MATCH_LESSTHEN;
			tk_match->next = fr->tk_match;
			fr->tk_match = tk_match;
		}
	}
	return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tk371x_range_vlan_update
 * Purpose:
 *     Updated the range content information
 * Parmeters:
 *     fr --(IN) _bcm_field_range_t
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_tk371x_range_update_content(
		_bcm_tk371x_field_range_t *fr){

    if (fr->flags & BCM_FIELD_RANGE_SRCPORT) {
    	return _bcm_tk371x_range_l4srcport_update(fr);
    }
    if (fr->flags & BCM_FIELD_RANGE_DSTPORT) {
    	return _bcm_tk371x_range_l4dstport_update(fr);
    }
    if (fr->flags & BCM_FIELD_RANGE_OUTER_VLAN) {
    	return _bcm_tk371x_range_vlan_update(fr);
    }
    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tk371x_range_vlan_update
 * Purpose:
 *    Update invert value
 * Parmeters:
 *     fr --(IN) _bcm_field_range_t
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_tk371x_range_invert_update(
		_bcm_tk371x_field_range_t *fr){
	int tmp;
	tmp = fr->min;
	fr->min = fr->max;
	fr->max = tmp;

	return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tk371x_range_vlan_update
 * Purpose:
 *     If invert is true, invert, then update
 * Parmeters:
 *     fr --(IN) _bcm_field_range_t
 * Returns:
 *    BCM_E_XXX
 */
STATIC int
_bcm_tk371x_range_tk_match_update(_bcm_tk371x_field_range_t *fr){
	return _bcm_tk371x_range_update_content(fr);
}

/*
 * Function: _bcm_tk371x_field_range_flags_check
 *
 * Purpose:
 *     Sanity check on range flags.
 *
 * Parameters:
 *     unit   - (IN) BCM device number.
 *     flags  - (IN) One or more of BCM_FIELD_RANGE_* flags.
 *     stage_id - (OUT) Pipeline stage id number.
 *
 * Returns:
 *     BCM_E_XXX  - Flags are valid & supported.
 */
STATIC int
_bcm_tk371x_field_range_flags_check(int unit, uint32 flags)
{
    int   cntr;                 /* Range types counter.  */

    if (flags & (BCM_FIELD_RANGE_INNER_VLAN |BCM_FIELD_RANGE_EXTERNAL|
        BCM_FIELD_RANGE_TCP |BCM_FIELD_RANGE_UDP|
        BCM_FIELD_RANGE_PACKET_LENGTH)) {
        return (BCM_E_UNAVAIL);
    }
    /* Make sure only one range was selected. */
    cntr = 0;
    if (flags & BCM_FIELD_RANGE_SRCPORT) {
        cntr++;
    }
    if (flags & BCM_FIELD_RANGE_DSTPORT) {
        cntr++;
    }
    if (flags & BCM_FIELD_RANGE_OUTER_VLAN) {
        cntr++;
    }
    if (flags & BCM_FIELD_RANGE_INVERT) {
        cntr++;
    }
    if (cntr > 1) {
        return (BCM_E_PARAM);
    }
    if (cntr == 0) {
        return (BCM_E_UNAVAIL);
    }
    return (BCM_E_NONE);
}

/*
 * Function:_bcm_tk371x_field_range_create
 *
 * Purpose:
 *     Create field range object.
 *
 * Parameters:
 *     unit   - (IN) BCM device number.
 *
 * Returns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_tk371x_field_range_create(int unit, bcm_field_range_t *range,
                    uint32 flags, bcm_l4_port_t min,
                    bcm_l4_port_t max)
{
    int rv;

    *range = _FP_RANGE_ID_BASE;
    for (;;) {
        rv = bcm_tk371x_field_range_create_id(unit,
        		*range, flags, min, max);
        if (rv != BCM_E_EXISTS) {
            break;
        }
        (*range)++;
    }
    return rv;
}
/*
 * Function:
 *     _bcm_tk371x_field_entry_get
 * Purpose:
 *     Lookup a _field_entry_t from a unit ID and slice choice.
 * Parmeters:
 *     unit   - BCM Unit
 *     eid    - Entry ID
 *     entry_p - (OUT) Entry lookup result.
 * Returns:
 *    BCM_E_XXX
 */
int
_bcm_tk371x_field_entry_get(int unit, bcm_field_entry_t eid,
							_bcm_tk371x_field_entry_t **entry_p)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t		*fe = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_entry_get(unit=%d)\n"),
	           unit));
    FIELD_IS_INIT(unit);
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    if (NULL == fg){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	fe = fg->entry;
    	while (fe != NULL){
    		if (fe->entry_id == eid){
    			*entry_p = fe;
    			return BCM_E_NONE;
    		}
    		fe = fe->next;
    	}
    	fg = fg->next;
    }
    return BCM_E_NOT_FOUND;
}


int
_bcm_tk371x_field_entry_qual_get(int unit, bcm_field_entry_t eid,
               bcm_field_qualify_t qual, _bcm_tk371x_field_entry_t **entry_p)
{
    uint8           found;
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t		*fe = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_entry_qual_get(unit=%d)\n"),
	           unit));
    FIELD_IS_INIT(unit);
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    found = FALSE;
    fg = fc->groups;
    if (NULL == fg){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	fe = fg->entry;
    	while (fe != NULL){
    		if (fe->entry_id == eid){
    		    if (BCM_FIELD_QSET_TEST(fg->qset, qual)) {
    		        found = TRUE;
    		    }
				/*except bcmFieldQualifyInnerVlanId for vlanformat using*/
				if (qual == bcmFieldQualifyInnerVlanId){
					found = TRUE;
				}
    		    if (FALSE == found) {
    		        return (BCM_E_PARAM);
    		    }
    		    LOG_DEBUG(BSL_LS_BCM_FP,
    		              (BSL_META_U(unit,
    		                          "FP: Has found(eid=%d, qset=%d)\n"),
    		               eid, qual));
    			*entry_p = fe;
    			return BCM_E_NONE;
    		}
    		fe = fe->next;
    	}
    	fg = fg->next;
    }
	return BCM_E_NOT_FOUND;
}

#define MAX_ENTRY_RULE_VAL_LEN	8
int
_bcm_tk371x_field_entry_group_id_get(int unit, bcm_field_entry_t eid, bcm_field_group_t *gid)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t		*fe = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_entry_group_id_get(unit=%d)\n"),
	           unit));
    FIELD_IS_INIT(unit);
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    while (fg != NULL){
    	fe = fg->entry;
    	while (fe != NULL){
    		if (fe->entry_id == eid){
    			(*gid) = fg->group_id;
    			return BCM_E_NONE;
    		}
    		fe = fe->next;
    	}
    	fg = fg->next;
    }
	return BCM_E_NOT_FOUND;
}

int
_bcm_tk371x_field_entry_qualify_update(
		int unit, bcm_field_entry_t entry, bcm_field_qualify_t qual_id,
		uint8 qset, uint8 match, uint8 *data, int len){
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*entry_P;
	_bcm_tk371x_field_entry_cond_t	*fe_cond;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_entry_qualify_update(unit=%d)\n"),
	           unit));
    if (fc == NULL){
    	return BCM_E_INIT;
    }
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_qual_get(unit, entry, qual_id, &entry_P));
	fe_cond = NULL;
	_FP_MEM_ALLOC(fe_cond,
			sizeof(_bcm_tk371x_field_entry_cond_t), "_bcm_tk371x_field_entry_cond_t");
	fe_cond->qual_id = qual_id;
	fe_cond->qset = qset;
	fe_cond->match = match;
	sal_memcpy((uint8*)
			(&fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), data, len);
	fe_cond->next = entry_P->conds;
	entry_P->conds = fe_cond;
	entry_P->flags |= _TK371X_ENTRY_FLAG_MODIFIED;
	entry_P->flags |= _TK371X_ENTRY_FLAG_EMPTY_QUAL;
	entry_P->flags |= _TK371X_ENTRY_FLAG_NOT_EMPTY;
	return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_destroy_all
 *
 * Purpose:
 *     Destroy all entries on a unit. It iterates over all slices in a unit.
 *     For each slice, If entries exist, it calls bcm_field_entry_destroy()
 *     using the Entry ID.
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_XXX       - Error from bcm_tk371x_field_entry_destroy()
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_destroy_all(int unit)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*group;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_destroy_all(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    /*destroy all entries*/
    group = fc->groups;
    while (group != NULL){
    	bcm_tk371x_field_group_destroy(unit, group->group_id);
    	group = group->next;
    }
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_tk371x_field_group_destroy
 *   Purpose
 *      Destroys a group.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      There must be no entries in this group when calling this function.
 */
int
bcm_tk371x_field_group_destroy(
	    int unit,
	    bcm_field_group_t group)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t      	*fg,                /* Group structure to free up   */
	                        		*fg_prev = NULL;    /* Previous node in groups list */
	_bcm_tk371x_field_entry_t		*entry;
	int ret_val;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_group_destroy(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	fg = NULL;
    /* Search the field control for the Group ID. */
    fg = fc->groups;
    while(fg != NULL) {
        if (fg->group_id == group) {
    		entry = fg->entry;
    		while (entry != NULL){
    			/* destory the group's entries */
    			ret_val = bcm_tk371x_field_entry_destroy(unit, entry->entry_id);
    			if (BCM_E_NONE != ret_val){
    				return ret_val;
    			}
    			entry = entry->next;
    		}
            break;
        }
        fg_prev = fg;
        fg      = fg->next;
    }
    if (fg == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: Group=%d not found in unit=%d.\n"),
                   group, unit));
        return BCM_E_NOT_FOUND;
    }
    /* Remove this group from device's linked-list of groups. */
    if (fg_prev == NULL) { /* Group is at head of list. */
        fc->groups = fg->next;
    } else {
        fg_prev->next = fg->next;
    }
    sal_free(fg);

    return BCM_E_NONE;
}

/*
 * Function:
 *     _bcm_tk371x_field_group_get
 * Purpose:
 *     Lookup a group information on specified bcm device.
 * Parameters:
 *     unit - (IN)BCM device number.
 *     gid  - (IN)Group ID.
 * Returns:
 *     BCM_E_XXX
 */
int
_bcm_tk371x_field_group_get(int unit, bcm_field_group_t gid)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_group_get(unit=%d)\n"),
	           unit));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    while (fg != NULL){
    	if (fg->group_id == gid){
    		return BCM_E_EXISTS;
    	}
    	fg = fg->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *     _bcm_tk371x_field_group_id_generate
 *
 * Purpose:
 *     Find an unused Group ID.
 *
 * Parameters:
 *     unit  - BCM device number
 *     qset - qualify set
 *     group - (OUT) new Group ID
 *
 * Returns:
 *     BCM_E_NONE  - Success
 *     BCM_E_PARAM - null pointer to group
 */
STATIC int
_bcm_tk371x_field_group_id_generate(int unit, bcm_field_group_t *group)
{
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_group_id_generate(unit=%d)\n"),
	           unit));
    *group = _FP_GROUP_ID_BASE;
    while (1) {
    	rv = _bcm_tk371x_field_group_get(unit, *group);
    	if (BCM_E_EXISTS == rv){
    		*group += 1;
    	}else {
    		break;
    	}
    }
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: Generate ID is %d\n"),
               *group));
    return BCM_E_NONE;
}

STATIC
int _bcm_tk371x_rule_param_queue(
		int unit, bcm_gport_t gport,
		uint32 offset, uint8 *port_link, uint8 *queue){
	int rv;
	bcm_gport_t physical_port;
    int num_cos_levels;
    uint32 flags;

	rv = bcm_tk371x_cosq_gport_get(unit, gport,
			&physical_port, &num_cos_levels, &flags);
	if (rv != BCM_E_NONE){
		return rv;
	}
	if (offset > num_cos_levels){
		return BCM_E_PARAM;
	}
	*port_link = (BCM_GPORT_LOCAL_GET(physical_port));
	if (*port_link >= 3){
		*port_link -= 3;
	}
	*queue = (uint8)offset;
	return BCM_E_NONE;
}
#define _TK371X_MAX_ACTIONS		10
#define _FP_ACTION_INVALID_VAL	0x3FFFF

STATIC int
_bcm_tk371x_field_detrative_rule_update(
	_bcm_tk371x_field_entry_t *fe,
	uint8 rule1_soc_action,
	uint8 rule2_soc_action){
	_bcm_tk371x_field_entry_derivative_t *fe_derivat = NULL;
	_bcm_tk371x_rule_t *rule1, *rule2;
	_bcm_tk371x_field_entry_cond_t 	*conds_P = NULL;
	int i = 0;

	if (fe->derivative != NULL){
		if (fe->derivative->rule1 != NULL){
			sal_free(fe->derivative->rule1);
		}
		if (fe->derivative->rule2 != NULL){
			sal_free(fe->derivative->rule2);
		}
		sal_free(fe->derivative);
	}
	fe->derivative = NULL;
	fe_derivat = NULL;
	_FP_MEM_ALLOC(fe_derivat,
			sizeof(_bcm_tk371x_field_entry_derivative_t),
			"_bcm_tk371x_field_entry_derivative_t");
	rule1 = NULL;

	if (rule1_soc_action != 0xFF){
		_FP_MEM_ALLOC(rule1,
				sizeof(_bcm_tk371x_rule_t),
				"_bcm_tk371x_rule_t");
		rule1->index.index = fe->index;
		rule1->index.objType = fe->objType;
		rule1->info.param.vid_cos = fe->rule_para.vid_cos;
		rule1->info.priority =fe->pri;
		rule1->info.volatiles = fe->volatiles;
		conds_P = fe->conds;
		i = 0;
		while (conds_P != NULL){
			if (i == 8) break;
			rule1->info.ruleCondition.conditionList[i].field = conds_P->qset;
			rule1->info.ruleCondition.conditionList[i].operator = conds_P->match;
			sal_memcpy((uint8*)&(rule1->info.ruleCondition.conditionList[i].common.value[0]),
					(uint8*)&(conds_P->common.value[0]), 8);
			conds_P = conds_P->next;
			i++;
		}
		rule1->info.ruleCondition.conditionCount = i;
		rule1->info.action = rule1_soc_action;
		fe_derivat->rule1 = rule1;
	}
	rule2 = NULL;
	if (rule2_soc_action != 0xFF){
		_FP_MEM_ALLOC(rule2,
				sizeof(_bcm_tk371x_rule_t),
				"_bcm_tk371x_rule_t");
		rule2->index.index = fe->index;
		rule2->index.objType = fe->objType;
		rule2->info.param.vid_cos = fe->rule_para.vid_cos;
		rule2->info.priority = fe->pri;
		rule2->info.volatiles = fe->volatiles;
		conds_P = fe->conds;
		i = 0;
		while (conds_P != NULL){
			if (i == 8) break;
			rule2->info.ruleCondition.conditionList[i].field = conds_P->qset;
			rule2->info.ruleCondition.conditionList[i].operator = conds_P->match;
			sal_memcpy((uint8*)&(rule2->info.ruleCondition.conditionList[i].common.value[0]),
					(uint8*)&(conds_P->common.value[0]), 8);
			conds_P = conds_P->next;
			i++;
		}
		rule2->info.ruleCondition.conditionCount = i;
		rule2->info.action = rule2_soc_action;
		fe_derivat->rule2 = rule2;
	}
	fe->derivative = fe_derivat;
	return BCM_E_NONE;
}

STATIC int
_bcm_tk371x_field_bcmActionToRuleAction_update(
		int unit,
		bcm_field_entry_t	entry,
		_bcm_tk371x_field_action_t *action,
		uint8 *soc_act){
	_bcm_tk371x_field_entry_t		*fe = NULL;
	_bcm_tk371x_field_action_t		*fe_act = NULL;
	int 			act[_TK371X_MAX_ACTIONS];
	int i = 0;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_bcmActionToRuleAction_update(unit=%d)\n"),
	           unit));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	sal_memset(act, 0, sizeof(act));
	for (i = 0; i < _TK371X_MAX_ACTIONS; i++){
		act[i] = _FP_ACTION_INVALID_VAL;
	}
	i = 0;
	if (fe != NULL){
		fe_act = fe->bcm_action;
		while (fe_act != NULL){
			act[i++] = fe_act->action;
			fe_act = fe_act->next;
		}
	}else{
		return BCM_E_NOT_FOUND;
	}
#ifdef BROADCOM_DEBUG
	for (i = 0; i < _TK371X_MAX_ACTIONS; i++){
		if (act[i] == _FP_ACTION_INVALID_VAL){
			LOG_DEBUG(BSL_LS_BCM_FP,
			          (BSL_META_U(unit,
			                      "FP:act[%d] =  _FP_ACTION_INVALID_VAL\n"),
			           i));
			break;
		}else{
			LOG_DEBUG(BSL_LS_BCM_FP,
			          (BSL_META_U(unit,
			                      "FP:act[%d] =  %s\n"),
			           i, _bcm_tk371x_field_action_name(act[i])));
		}
	}
#endif

    if (bcmFieldActionRedirectPort == act[0] && _FP_ACTION_INVALID_VAL == act[1]){
    	/* the soc action value is 0x02
    	 * Set the destination queue for the frame to which the frame was forward
    	 **/
    	rv = _bcm_tk371x_rule_param_queue(unit,
    						fe->bcm_action->param0,
    						fe->bcm_action->param1,
    						&(fe->rule_para.RuleNameQueue.port_link),
    						&(fe->rule_para.RuleNameQueue.queue));
    	if (rv != BCM_E_NONE){
    		return rv;
    	}
    	fe->soc_action = _bcmTk371xFieldSetPath;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanDelete == act[0] &&
    					_FP_ACTION_INVALID_VAL == act[1]){
   		 /*the action value is 0x04
   		  *Set delete vlan tag action to true
   		  **/
   		 fe->soc_action = _bcmTk371xFieldSetDelVlanTag;
   	   	 *soc_act = fe->soc_action;
   	     *action = *(fe->bcm_action);
   		 return BCM_E_NONE;
    }else if (bcmFieldActionOuterVlanAdd == act[0] &&
    					_FP_ACTION_INVALID_VAL == act[1]){
    	/* the soc action value is 0x05
    	 * 1. Set add the Vlan Tag action to true
    	 * 2. Set the vid
    	 **/
    	fe->soc_action =_bcmTk371xFieldSetVidAndAddVlanTag;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanPrioNew == act[0] &&
    						_FP_ACTION_INVALID_VAL == act[1]){
    	/* Replace packet priority
    	 * Rule 1: Set CoS (06)
    	 * Rule 2: VID exist & Copy field to VID(0x0d)
    	 * Rule 3: Delete tag; Add tag; (Replace Tag 07);
    	 **/
    	fe->soc_action = _bcmTk371xFieldSetCos;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldCopyFieldToVid, _bcmTk371xFieldReplaceVlanTag);
    	*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanNew == act[0] &&
    					_FP_ACTION_INVALID_VAL == act[1]){
    	/* Replace packat VLAN ID
    	 * Rule 1: CoS Exist & Copy field to CoS (0C)
    	 * Rule 2: Delete Tag; Add Tag; Set VID (Replace Tag; Set VID (08))
    	 **/
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
    	fe->soc_action = _bcmTk371xFieldCopyFieldToCos;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldReplaceVlanTagAndSetVid, 0xFF);
       	*soc_act = fe->soc_action;
    	*action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanAddCancel == act[0] &&
    						_FP_ACTION_INVALID_VAL == act[1]){
    	/* the soc action value is 0x09
    	 * Set the Add Tag action to false
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearAddVlanTag;
       	*soc_act = fe->soc_action;
    	*action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanDeleteCancel == act[0] &&
    							_FP_ACTION_INVALID_VAL == act[1]){
    	/* the soc action value is 0x0a
    	 * Set Delete Vlan tag action to false
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearDelVlanTag;
       	*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if ((bcmFieldActionOuterVlanAddCancel == act[0] &&
    		   bcmFieldActionOuterVlanDeleteCancel == act[1] &&
    		   _FP_ACTION_INVALID_VAL == act[2])) {
    	/* the soc action value is 0x0B
    	 * 1. Set Delete VLAN tag action to true
    	 * 2. Set Add VLAN tag action to true
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTag;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanDeleteCancel == act[0] &&
    			bcmFieldActionOuterVlanAddCancel == act[1] &&
    			_FP_ACTION_INVALID_VAL == act[2]){
    	/* the soc action value is 0x0B
    	 * 1. Set Delete VLAN tag action to true
    	 * 2. Set Add VLAN tag action to true
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTag;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionPrioPktTos == act[0] &&
    				_FP_ACTION_INVALID_VAL == act[1]){
    	/* Packet priority from TOS field
    	 * 1. Rule One: ToS exist & Copy field to CoS (OC)
    	 * 2. Rule two: VID exist & copy field to VID (0D)
    	 * 3. Rule Three: Delete Tag; Add Tag; (Replace tag)(07)
    	 * */
    	fe->soc_action = _bcmTk371xFieldCopyFieldToCos;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldCopyFieldToVid, _bcmTk371xFieldReplaceVlanTag);
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
	}else if (bcmFieldActionPrioPktTos == act[0] && 
		bcmFieldActionDropCancel == act[1] && _FP_ACTION_INVALID_VAL == act[2]){
		/* Keep the COS in frame unchanged, and set the forward action
		 * Rule 1.ToS Exist & Copy field to CoS (0c)
		 * 	or Rule 1. VID Exist & Copy field to VID (0D)
		 * Rule 2.Forward (clear Discard) (10)
		 */		 
		fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
		fe->soc_action = _bcmTk371xFieldCopyFieldToCosAndForward;
		_bcm_tk371x_field_detrative_rule_update(fe,
				_bcmTk371xFieldCopyFieldToVidAndForward, 
				_bcmTk371xFieldReplaceTagAndForward);
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);		
	}if (bcmFieldActionDropCancel == act[0] &&
		bcmFieldActionPrioPktTos == act[1] && _FP_ACTION_INVALID_VAL == act[2]){
		/* Keep the COS in frame unchanged, and set the forward action
		 * Rule 1.ToS Exist & Copy field to CoS (0c)
		 * 	or Rule 1. VID Exist & Copy field to VID (0D)
		 * Rule 2.Forward (clear Discard) (10)
		 */		 
		fe->rule_para.vid_cos = (uint16)fe->bcm_action->next->param0;
		fe->soc_action = _bcmTk371xFieldCopyFieldToCosAndForward;
		_bcm_tk371x_field_detrative_rule_update(fe,
				_bcmTk371xFieldCopyFieldToVidAndForward, 
				_bcmTk371xFieldReplaceTagAndForward);
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);					
	}else if (bcmFieldActionDrop == act[0] &&
    			_FP_ACTION_INVALID_VAL == act[1]){
    	/* the soc action value is 0x0E
    	 * set the discard action
    	 * */
    	fe->soc_action = _bcmTk371xFieldDiscard;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionDropCancel == act[0] &&
    				_FP_ACTION_INVALID_VAL == act[1]){
    	/* the soc action value is 0x10
    	 * Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionDropCancel == act[0] &&
    		bcmFieldActionRedirectPort == act[1] &&
    		   	   _FP_ACTION_INVALID_VAL == act[2]){
    	/* the soc action value is 0x12
    	 * 1. Set the destination queue for the frame
    	 * 2. Set the forward action
    	 **/
    	rv = _bcm_tk371x_rule_param_queue(unit,
    						fe->bcm_action->param0,
    						fe->bcm_action->param1,
    						&(fe->rule_para.RuleNameQueue.port_link),
    						&(fe->rule_para.RuleNameQueue.queue));
    	if (rv != BCM_E_NONE){
    		return rv;
    	}
    	fe->soc_action = _bcmTk371xFieldSetPathAndForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if(bcmFieldActionRedirectPort == act[1] &&
    		    	bcmFieldActionDropCancel == act[0] &&
    		    	_FP_ACTION_INVALID_VAL == act[2]){
    	/* the soc action value is 0x12
    	 * 1. Set the destination queue for the frame
    	 * 2. Set the forward action
    	 **/
    	rv = _bcm_tk371x_rule_param_queue(unit,
    						fe->bcm_action->next->param0,
    						fe->bcm_action->next->param1,
    						&(fe->rule_para.RuleNameQueue.port_link),
    						&(fe->rule_para.RuleNameQueue.queue));

    	if (rv != BCM_E_NONE){
    		return rv;
    	}
    	fe->soc_action = _bcmTk371xFieldSetPathAndForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanDelete == act[0] &&
		   	   bcmFieldActionDropCancel == act[1] &&
		   	   _FP_ACTION_INVALID_VAL == act[2]) {
    	/* the soc action value is 0x14
    	 * 1. Set Delete VLAN tag action to true
    	 * 2. Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldSetDelVlanTagAndForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if(((bcmFieldActionDropCancel == act[0]) &&
		    	(bcmFieldActionOuterVlanDelete == act[1]) &&
		    	(_FP_ACTION_INVALID_VAL == act[2]))){
       	/* the soc action value is 0x14
		 * 1. Set Delete VLAN tag action to true
		 * 2. Set the forward action
		 * */
    	fe->soc_action = _bcmTk371xFieldSetDelVlanTagAndForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanAdd == act[0] &&
		   	   bcmFieldActionDropCancel == act[1] &&
		   	   _FP_ACTION_INVALID_VAL == act[2]){
    	/* the soc action value is 0x15
    	 * 1. Set VID
    	 * 2. Set Add VLAN tag action to true
    	 * 3. Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldSetVidAndAddVlanTag;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldSetVidAndSetAddVlanTagAndForward, 0xFF);
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if(bcmFieldActionDropCancel == act[0] &&
		    	bcmFieldActionOuterVlanAdd == act[1] &&
		    	_FP_ACTION_INVALID_VAL == act[2]){
    	/* the soc action value is 0x15
    	 * 1. Set VID
    	 * 2. Set Add VLAN tag action to true
    	 * 3. Set the forward action
    	 * */
		fe->soc_action = _bcmTk371xFieldSetVidAndAddVlanTag;
		fe->rule_para.vid_cos = (uint16)fe->bcm_action->next->param0;
		_bcm_tk371x_field_detrative_rule_update(fe,
				_bcmTk371xFieldSetVidAndSetAddVlanTagAndForward, 0xFF);
		*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanPrioNew == act[0] &&
		   	   bcmFieldActionDropCancel == act[1] &&
		   	   _FP_ACTION_INVALID_VAL == act[2]){
    	/* Replace packet priority and forwarding
    	 * Rule 1: Set CoS (16)
    	 * Rule 2: VID Exist & Copy field to VID (1D)
    	 * Rule 3: Delete Tag; Add Tag (Replace Tag 17)
    	 * */
#ifdef BROADCOM_DEBUG
		LOG_DEBUG(BSL_LS_BCM_FP,
		          (BSL_META_U(unit,
		                      "1:\n")));
		LOG_DEBUG(BSL_LS_BCM_FP,
		          (BSL_META_U(unit,
		                      "(uint16)fe->bcm_action->param0=%d\n"),
		           (uint16)fe->bcm_action->param0));
		LOG_DEBUG(BSL_LS_BCM_FP,
		          (BSL_META_U(unit,
		                      "(uint16)fe->bcm_action->next->param0=%d\n"),
		           (uint16)fe->bcm_action->next->param0));
#endif		
    	fe->soc_action = _bcmTk371xFieldSetCosAndForward;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldCopyFieldToVidAndForward, _bcmTk371xFieldReplaceTagAndForward);
    	*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if(bcmFieldActionDropCancel == act[0] &&
		    	bcmFieldActionOuterVlanPrioNew == act[1] &&
		    	_FP_ACTION_INVALID_VAL == act[2]){
    	/* Replace packet priority and forwarding
    	 * Rule 1: Set CoS (16)
    	 * Rule 2: VID Exist & Copy field to VID (1D)
    	 * Rule 3: Delete Tag; Add Tag (Replace Tag 17)
    	 * */
#ifdef BROADCOM_DEBUG
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "2:\n")));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "(uint16)fe->bcm_action->param0=%d\n"),
    	           (uint16)fe->bcm_action->param0));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "(uint16)fe->bcm_action->next->param0=%d\n"),
    	           (uint16)fe->bcm_action->next->param0));
#endif    	
    	fe->soc_action = _bcmTk371xFieldSetCosAndForward;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->next->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldCopyFieldToVidAndForward, _bcmTk371xFieldReplaceTagAndForward);
    	*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if ((bcmFieldActionOuterVlanNew == act[0]) &&
		   	   (bcmFieldActionDropCancel == act[1]) &&
		   	   (_FP_ACTION_INVALID_VAL == act[2])) {
    	/* the soc action value is 0x18
    	 * 1. CoS Exist & Copy field to CoS (1C)
    	 * 2. Delete Tag; Add Tag; Set VID (Replace Tag; Set VID)
    	 */		
    	fe->soc_action = _bcmTk371xFieldCopyFieldToCosAndForward;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldReplaceTagAndSetVidAndForward, 0xFF);
    	*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if ((bcmFieldActionDropCancel == act[0]) &&
		    	(bcmFieldActionOuterVlanNew == act[1]) &&
		    	(_FP_ACTION_INVALID_VAL == act[2])){
    	/* the soc action value is 0x18
    	 * 1. CoS Exist & Copy field to CoS (1C)
    	 * 2. Delete Tag; Add Tag; Set VID (Replace Tag; Set VID)
    	 */		
    	fe->soc_action = _bcmTk371xFieldCopyFieldToCosAndForward;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->next->param0;
    	_bcm_tk371x_field_detrative_rule_update(fe,
    			_bcmTk371xFieldReplaceTagAndSetVidAndForward, 0xFF);
    	*soc_act = fe->soc_action;
		*action = *(fe->bcm_action);
    }else if ((bcmFieldActionOuterVlanAddCancel == act[0]) &&
		   	   (bcmFieldActionDropCancel == act[1]) &&
		   	   (_FP_ACTION_INVALID_VAL == act[2])) {
    	/* the soc action value is 0x19
    	 * 1. Set the Add tag action to false
    	 * 2. Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearAddVlanTagAndForward;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->param0;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if ((bcmFieldActionDropCancel == act[0]) &&
		    	(bcmFieldActionOuterVlanAddCancel == act[1]) &&
		    	(_FP_ACTION_INVALID_VAL == act[2])){
    	/* the soc action value is 0x19
    	 * 1. Set the Add tag action to false
    	 * 2. Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearAddVlanTagAndForward;
    	fe->rule_para.vid_cos = (uint16)fe->bcm_action->next->param0;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if ((bcmFieldActionOuterVlanDeleteCancel == act[0]) &&
		   	   (bcmFieldActionDropCancel == act[1]) &&
		   	   (_FP_ACTION_INVALID_VAL == act[2])) {
    	/* the soc action value is 0x1A
    	 * 1. Set the delete tag action to false
    	 * 2. Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if((bcmFieldActionDropCancel == act[0]) &&
		    	(bcmFieldActionOuterVlanDeleteCancel == act[1]) &&
		    	(_FP_ACTION_INVALID_VAL == act[2])){
    	/* the soc action value is 0x1A
    	 * 1. Set the delete tag action to false
    	 * 2. Set the forward action
    	 * */
    	fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndForward;
       	*soc_act = fe->soc_action;
        *action = *(fe->bcm_action);
    }else if (bcmFieldActionOuterVlanAddCancel == act[0]){
    	if ((bcmFieldActionOuterVlanDeleteCancel == act[1]) &&
		   	  (bcmFieldActionDropCancel == act[2]) &&
		   	  (_FP_ACTION_INVALID_VAL == act[3])){
    		/* the soc action value is 0x1B
    		 * 1. Set the Delete tag action to false
    		 * 2. Set Add VLAN tag action to true
    		 * 3. Set the forward action
    		 * */
    		fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward;
			*soc_act = fe->soc_action;
			*action = *(fe->bcm_action);
    	}else if((bcmFieldActionDropCancel == act[1]) &&
		   	 (bcmFieldActionOuterVlanDeleteCancel == act[2]) &&
		     (_FP_ACTION_INVALID_VAL == act[3])){
    		/* the soc action value is 0x1B
    		 * 1. Set the Delete tag action to false
    		 * 2. Set Add VLAN tag action to true
    		 * 3. Set the forward action
    		 * */
			fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward;
			*soc_act = fe->soc_action;
			*action = *(fe->bcm_action);
    	}

    }else if (bcmFieldActionOuterVlanDeleteCancel == act[0]){
    	if ((bcmFieldActionOuterVlanAddCancel == act[1]) &&
		   	  (bcmFieldActionDropCancel == act[2]) &&
		   	  (_FP_ACTION_INVALID_VAL == act[3])) {
    		/* the soc action value is 0x1B
    		 * 1. Set the Delete tag action to false
    		 * 2. Set Add VLAN tag action to true
    		 * 3. Set the forward action
    		 * */
    		fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward;
           	*soc_act = fe->soc_action;
            *action = *(fe->bcm_action);
    	}else if((bcmFieldActionDropCancel == act[1]) &&
		   	 (bcmFieldActionOuterVlanAddCancel == act[2]) &&
		     (_FP_ACTION_INVALID_VAL == act[3])){
    		/* the soc action value is 0x1B
    		 * 1. Set the Delete tag action to false
    		 * 2. Set Add VLAN tag action to true
    		 * 3. Set the forward action
    		 * */
    		fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward;
           	*soc_act = fe->soc_action;
            *action = *(fe->bcm_action);
    	}

    }else if (bcmFieldActionDropCancel == act[0]){
    	if ((bcmFieldActionOuterVlanAddCancel == act[1]) &&
		   	  (bcmFieldActionOuterVlanDeleteCancel == act[2]) &&
		   	  (_FP_ACTION_INVALID_VAL == act[3])){
    		/* the soc action value is 0x1B
    		 * 1. Set the Delete tag action to false
    		 * 2. Set Add VLAN tag action to true
    		 * 3. Set the forward action
    		 * */
    		fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward;
           	*soc_act = fe->soc_action;
            *action = *(fe->bcm_action);
    	}else if((bcmFieldActionOuterVlanDeleteCancel == act[1]) &&
		   	 (bcmFieldActionOuterVlanAddCancel == act[2]) &&
		     (_FP_ACTION_INVALID_VAL == act[3])){
    		/* the soc action value is 0x1B
    		 * 1. Set the Delete tag action to false
    		 * 2. Set Add VLAN tag action to true
    		 * 3. Set the forward action
    		 * */
    		fe->soc_action = _bcmTk371xFieldClearDelVlanTagAndSetAddVlanTagAndForward;
           	*soc_act = fe->soc_action;
            *action = *(fe->bcm_action);
    	}
    }else{
    	return BCM_E_NONE;
    }
    if (fe->bcm_action != NULL){
    	fe->flags |= _TK371X_ENTRY_FLAG_NOT_EMPTY;
    	fe->flags |= _TK371X_ENTRY_FLAG_MODIFIED;
    	fe->flags |= _TK371X_ENTRY_FLAG_EMPTY_ACTION;
    }
    return BCM_E_NONE;
}


/*
 * Function: _bcm_tk371x_field_action_insert
 *
 * Purpose:
 *     Add action performed when entry rule is matched for a packet
 *
 * Parameters:
 *     unit - BCM device number
 *     eid - entry ID
 *     act - Action to perform (bcmFieldActionXXX)
 *	   fa  - _bcm_tk371x_field_action_t
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_MEMORY    - Allocation failure
 *     BCM_E_RESOURCE  - Conflicting actions exist in entry.
 *     BCM_E_RESOURCE  - Counter not previously created for entry.
 *     BCM_E_PARAM     - param0 or param1 out of range for action
 *     BCM_E_NONE      - Success
 */
STATIC int
_bcm_tk371x_field_action_insert(int unit,
		bcm_field_entry_t eid,
		bcm_field_action_t act,
		_bcm_tk371x_field_action_t *fa)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_action_t	*fa_src = NULL;
	int rv;

	if (fc == NULL){
		return BCM_E_INIT;
	}
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: _bcm_tk371x_field_action_insert(unit=%d)\n"),
	           unit));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, eid, &fe));
	if (fe == NULL){
		return BCM_E_NOT_FOUND;
	}
	fa_src = fe->bcm_action;
	while (fa_src != NULL){
		if (fa_src->action == act){
			/* delete the old action */
			rv = bcm_tk371x_field_action_delete(unit, eid, act, fa_src->param0, fa_src->param1);
			if (rv != BCM_E_NONE){
				return rv;
			}
		}
		fa_src = fa_src->next;
	}
	/* insert the new action */
	fa->next = fe->bcm_action;
	fe->bcm_action = fa;
    return BCM_E_NONE;
}
/*
 * Function: bcm_tk371x_field_action_add
 *
 * Purpose:
 *     Add action performed when entry rule is matched for a packet
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - entry ID
 *     action - Action to perform (bcmFieldActionXXX)
 *     param0 - Action parameter (use 0 if not required)
 *     param1 - Action parameter (use 0 if not required)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_MEMORY    - Allocation failure
 *     BCM_E_RESOURCE  - Conflicting actions exist in entry.
 *     BCM_E_RESOURCE  - Counter not previously created for entry.
 *     BCM_E_PARAM     - param0 or param1 out of range for action
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_add(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action,
	    uint32 param0,
	    uint32 param1)
{
	_bcm_tk371x_field_action_t 		*fa_act;
	uint8 soc_action;
	int rv = BCM_E_NONE;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_action_add(unit=%d)\n"),
	           unit));
	switch (action){
		case bcmFieldActionRedirectPort:
		case bcmFieldActionOuterVlanAdd:
		case bcmFieldActionOuterVlanPrioNew:
		case bcmFieldActionOuterVlanNew:
		case bcmFieldActionPrioPktTos:
		case bcmFieldActionDrop:
		case bcmFieldActionDropCancel:
		case bcmFieldActionOuterVlanAddCancel:
		case bcmFieldActionOuterVlanDeleteCancel:
		case bcmFieldActionOuterVlanPrioCopyInner:
		case bcmFieldActionOuterVlanCopyInner:
		case bcmFieldActionOuterVlanDelete:
			break;
		default:
			return BCM_E_UNAVAIL;
	}
	fa_act = NULL;
	_FP_MEM_ALLOC(fa_act, sizeof(_bcm_tk371x_field_action_t), "_bcm_tk371x_field_action_t");
	fa_act->action = action;
	fa_act->param0 = param0;
	fa_act->param1 = param1;
	fa_act->next = NULL;

	rv = _bcm_tk371x_field_action_insert(unit, entry, action, fa_act);
	if (rv != BCM_E_NONE){
		return rv;
	}
    rv = _bcm_tk371x_field_bcmActionToRuleAction_update(unit, entry, fa_act, &soc_action);
    if (rv == BCM_E_NOT_FOUND){
    	return BCM_E_NOT_FOUND;
    }
    if (rv != BCM_E_NONE){
    	bcm_tk371x_field_action_delete(unit, entry, action, param0, param1);
    	return rv;
    }
	return BCM_E_NONE;
}

/*
 * Function: _bcm_tk371x_field_action_delete
 *
 * Purpose:
 *     Delete an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     action - Action to remove (bcmFieldActionXXX)
 *     fe     - _bcm_tk371x_field_entry_t
 *     fa     - _bcm_tk371x_field_action_t
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
STATIC
int _bcm_tk371x_field_action_delete(
		bcm_field_action_t act,
		_bcm_tk371x_field_entry_t *fe,
		_bcm_tk371x_field_action_t *fa)
{
	_bcm_tk371x_field_action_t	*fa_act, *act_prev = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META("FP: Enter _bcm_tk371x_field_action_delete\n")));
	if (fe == NULL){
		return BCM_E_NONE;
	}
	fa_act = NULL;
	fa_act = fe->bcm_action;
	while (fa_act != NULL){
		/* Find this action */
		if (fa_act->action == act){
			/* record this action */
			fa->action = fa_act->action;
			fa->param0 = fa_act->param0;
			fa->param1 = fa_act->param1;
			fa->next = NULL;
			/* delete this action */
			if (act_prev == NULL){
				fe->bcm_action = act_prev;
				sal_free(fa_act);
			}else{
				act_prev->next = fa_act->next;
				sal_free(fa_act);
			}
			return BCM_E_NONE;
		}
		act_prev = fa_act;
		fa_act = fa_act->next;
	}
    return BCM_E_NOT_FOUND;
}

/*
 * Function: bcm_tk371x_field_action_delete
 *
 * Purpose:
 *     Delete an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Entry ID
 *     action - Action to remove (bcmFieldActionXXX)
 *     param0 - Action parameter (use 0 if not required)
 *     param1 - Action parameter (use 0 if not required)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_delete(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action,
	    uint32 param0,
	    uint32 param1)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t		*fe = NULL;
	_bcm_tk371x_field_action_t 		fa_ret;
	uint8 soc_action;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_action_delete(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	switch (action){
		case bcmFieldActionRedirectPort:
		case bcmFieldActionOuterVlanAdd:
		case bcmFieldActionOuterVlanPrioNew:
		case bcmFieldActionOuterVlanNew:
		case bcmFieldActionPrioPktTos:
		case bcmFieldActionDrop:
		case bcmFieldActionDropCancel:
		case bcmFieldActionOuterVlanAddCancel:
		case bcmFieldActionOuterVlanDeleteCancel:
		case bcmFieldActionOuterVlanPrioCopyInner:
		case bcmFieldActionOuterVlanCopyInner:
		case bcmFieldActionOuterVlanDelete:
			break;
		default:
			return BCM_E_UNAVAIL;
	}

	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    rv = _bcm_tk371x_field_entry_get(unit, entry, &fe);
    if (rv != BCM_E_NONE){
    	return rv;
    }
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: _bcm_tk371x_field_entry_get find entry id = %d\n"),
               fe->entry_id));
    rv = _bcm_tk371x_field_action_delete(action, fe, &fa_ret);
    if (rv != BCM_E_NONE){
    	return rv;
    }
    rv = _bcm_tk371x_field_bcmActionToRuleAction_update(unit, entry,
    		&fa_ret, &soc_action);
    if (rv != BCM_E_NONE){
    	return rv;
    }
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_action_get
 *
 * Purpose:
 *     Get parameters associated with an entry action
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *     action - Action to perform (bcmFieldActionXXX)
 *     param0 - (OUT) Action parameter
 *     param1 - (OUT) Action parameter
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - paramX is NULL
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action,
	    uint32 *param0,
	    uint32 *param1)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t		*entry_P = NULL;
	_bcm_tk371x_field_action_t		*act_P = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_action_get(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	switch (action){
		case bcmFieldActionRedirectPort:
		case bcmFieldActionOuterVlanAdd:
		case bcmFieldActionOuterVlanPrioNew:
		case bcmFieldActionOuterVlanNew:
		case bcmFieldActionPrioPktTos:
		case bcmFieldActionDrop:
		case bcmFieldActionDropCancel:
		case bcmFieldActionOuterVlanAddCancel:
		case bcmFieldActionOuterVlanDeleteCancel:
		case bcmFieldActionOuterVlanPrioCopyInner:
		case bcmFieldActionOuterVlanCopyInner:
		case bcmFieldActionOuterVlanDelete:
			break;
		default:
			return BCM_E_UNAVAIL;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    rv = _bcm_tk371x_field_entry_get(unit, entry, &entry_P);
    if (rv != BCM_E_NONE){
    	return rv;
    }
    act_P = entry_P->bcm_action;
    while (act_P != NULL){
    	if (act_P->action == action){
    		*param0 = act_P->param0;
    		*param1 = act_P->param1;
    		return BCM_E_NONE;
    	}
    	act_P = act_P->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 * Function: bcm_tk371x_field_action_remove
 *
 * Purpose:
 *     Remove an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Entry ID
 *     action - Action to remove (bcmFieldActionXXX)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_remove(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_action_remove(unit=%d)\n"),
	           unit));
    return bcm_tk371x_field_action_delete(unit, entry, action,
                    (uint32)_FP_PARAM_INVALID, (uint32)_FP_PARAM_INVALID);
}

/*
 * Function: bcm_field_action_remove_all
 *
 * Purpose:
 *     Remove all actions from an entry rule.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_remove_all(
	    int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t		*entry_P = NULL;
	_bcm_tk371x_field_action_t		*act_P = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_action_remove_all(unit=%d)\n"),
	           unit));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    rv = _bcm_tk371x_field_entry_get(unit, entry, &entry_P);
    if (rv != BCM_E_NONE){
    	return rv;
    }
    act_P = entry_P->bcm_action;
    while (act_P != NULL){
    	rv = bcm_tk371x_field_action_remove(unit, entry, act_P->action);
    	if (rv != BCM_E_NONE){
    		return rv;
    	}
    	act_P = act_P->next;
    }

	return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_detach
 *
 * Purpose:
 *     Free resources associated with field module
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_INIT - BCM Unit not initialized.
 *     BCM_E_XXX  - Error code from bcm_field_group_destroy() or
 *                  bcm_field_entry_destroy_all().
 *     BCM_E_NONE - Success
 */
int
bcm_tk371x_field_detach(int unit)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t 		*fg = NULL;
	_bcm_tk371x_field_range_t		*fr = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_detach(unit=%d)\n"),
	           unit));

    /* Make sure the Unit can support this module. */
    if (!BCM_TK371X_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_field)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: No Field Processor available Unit=%d\n"),
                   unit));
        return BCM_E_UNAVAIL;
    }

	if (fc == NULL){
		return BCM_E_INIT;
	}

    /* Destroy all entries in unit. */
    rv = bcm_tk371x_field_entry_destroy_all(unit);
    if (BCM_FAILURE(rv)) {
         return (rv);
    }
    /* Destroy all groups in unit. */
    fg = fc->groups;
    while (fg != NULL) {
        rv = bcm_tk371x_field_group_destroy(unit, fg->group_id);
        if (BCM_FAILURE(rv)) {
            return (rv);
        }
        fg = fc->groups;
    }

    /* Destroy all ranges in unit*/
    fr = fc->ranges;
    while (fr != NULL){
    	rv = bcm_tk371x_field_range_destroy(unit, fr->rid);
    	if (BCM_FAILURE(rv)){
    		return (rv);
    	}
    	fr = fr->next;
    }

    _field_control = NULL;
    if (fc != NULL){
    	sal_free(fc);
    }
    return BCM_E_NONE;
}


/*
 * Function: bcm_tk371x_field_entry_copy
 *
 * Purpose:
 *     Create a copy of an existing entry. The new entry will be a member of
 *     the same group as the source entry.
 *
 * Parameters:
 *     unit      - BCM device number
 *     src_entry - Entry to copy
 *     dst_entry - (OUT) New entry
 *
 * Returns:
 *     BCM_E_INIT        BCM Unit not initialized
 *     BCM_E_NOT_FOUND   Source entry not found
 *     BCM_E_INTERNAL    No group exists for source entry ID.
 *     BCM_E_PARAM       dst_entry pointing to NULL
 *     BCM_E_RESOURCE    No destination entry available
 *     BCM_E_XXX         Error from bcm_field_entry_copy_id()
 */
int
bcm_tk371x_field_entry_copy(
	    int unit,
	    bcm_field_entry_t src_entry,
	    bcm_field_entry_t *dst_entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*fe_src = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_copy(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}

	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));

    rv = _bcm_tk371x_field_entry_get(unit, src_entry, &fe_src);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    if (dst_entry == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: dst_entry == NULL\n")));
        return BCM_E_PARAM;
    }

    *dst_entry = src_entry + 1;
    while (BCM_SUCCESS
        (_bcm_tk371x_field_entry_get(unit, *dst_entry, &fe_src))) {
        *dst_entry += 1;
        assert(*dst_entry < _FP_ENTRY_ID_BASE + _FP_ENTRY_ID_MAX);
    }
    rv = bcm_tk371x_field_entry_copy_id(unit, src_entry, *dst_entry);

    return rv;
}

/*
 * Function: bcm_tk371x_field_entry_copy_id
 *
 * Purpose:
 *     Create a copy of an existing entry with a requested ID
 *
 * Parameters:
 *     unit      - BCM device number
 *     src_entry - Source entry to copy
 *     dst_entry - Destination entry for copy
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Source Entry ID not found
 *     BCM_E_XXX       - Error code from bcm_field_entry_create_id()
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_copy_id(
	    int unit,
	    bcm_field_entry_t src_entry,
	    bcm_field_entry_t dst_entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*f_ent_src, *f_ent_dst = NULL;
	_bcm_tk371x_field_action_t		*fa_src = NULL;
	_bcm_tk371x_field_entry_cond_t	*fc_src, *fe_cond = NULL;
	bcm_field_group_t gid;
	int rv;
	uint8 soc_action;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_copy_id(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	f_ent_src = NULL;
	fc_src = NULL;
	rv = _bcm_tk371x_field_entry_get(unit, src_entry, &f_ent_src);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: %s src:%d dst:%d\n"),
	           FUNCTION_NAME(),src_entry, dst_entry));
	rv = _bcm_tk371x_field_entry_group_id_get(unit, src_entry, &gid);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    rv = bcm_tk371x_field_entry_create_id(unit, gid, dst_entry);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    rv = _bcm_tk371x_field_entry_get(unit, dst_entry, &f_ent_dst);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    f_ent_dst->index = f_ent_src->index;
    f_ent_dst->linkid = f_ent_src->linkid;
    f_ent_dst->objType = f_ent_src->objType;
    f_ent_dst->pri = f_ent_src->pri;
    f_ent_dst->rule_para.vid_cos = f_ent_src->rule_para.vid_cos;
    /* Copy source entry's action linked list.  */
    for (fa_src = f_ent_src->bcm_action; fa_src != NULL; fa_src = fa_src->next) {
        rv = bcm_tk371x_field_action_add(unit, dst_entry, fa_src->action,
                                 fa_src->param0, fa_src->param1);
        if (BCM_FAILURE(rv)) {
            return rv;
        }
    }
    /* Copy source entry's condtions linked list */
    for (fc_src = f_ent_src->conds; fc_src != NULL; fc_src = fc_src->next){
    	_FP_MEM_ALLOC(fe_cond,
    			sizeof(_bcm_tk371x_field_entry_cond_t), "_bcm_tk371x_field_entry_cond_t");
    	fe_cond->qual_id = fc_src->qual_id;
    	fe_cond->qset = fc_src->qset;
    	fe_cond->match = fc_src->match;
    	sal_memcpy((uint8*)fe_cond->common.value, fc_src->common.value, 8);
    	fe_cond->next = NULL;
    	/*Insert the cond in dst entry*/
    	fe_cond->next = f_ent_dst->conds;
    	f_ent_dst->conds = fe_cond;
    	fe_cond = NULL;
    }
    rv = _bcm_tk371x_field_bcmActionToRuleAction_update(unit, dst_entry,
    		f_ent_dst->bcm_action, &soc_action);
    if (rv == BCM_E_NOT_FOUND){
    	return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_create
 *
 * Purpose:
 *     Create a blank entry based on a group. Automatically generate an entry
 *     ID.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group ID
 *     entry - (OUT) New entry
 *
 * Returns:
 *     BCM_E_INIT        BCM unit not initialized
 *     BCM_E_NOT_FOUND   group not found in unit
 *     BCM_E_PARAM       *entry was NULL
 *     BCM_E_RESOURCE    No unused entries available
 *     BCM_E_XXX         Error from bcm_field_entry_create_id
 *
 * See Also:
 * bcm_field_entry_create_id
 */
int
bcm_tk371x_field_entry_create(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_entry_t *entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t 		*f_ent_src, *f_ent_dst = NULL;
	int group_flag = 1;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_create(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	f_ent_src = NULL;
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    if (fg == NULL){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	if (fg->group_id == group){
    		group_flag = 0;
    		*entry = 1;
        	while (1){
        		/* wheter the   */
        		rv = _bcm_tk371x_field_entry_get(unit, *entry, &f_ent_src);
        		if (rv == BCM_E_NONE){
        			(*entry)++;
        		}else if (rv == BCM_E_NOT_FOUND){
        			break;
        		}else{
        			return rv;
        		}
        	}
        	_FP_MEM_ALLOC(f_ent_dst, sizeof(_bcm_tk371x_field_entry_t), "_bcm_tk371x_field_entry_t");
         	if (f_ent_dst == NULL){
         		return BCM_E_MEMORY;
         	}
    		/* default the entry value */
        	f_ent_dst->entry_id = *entry;
        	f_ent_dst->flags	= _FP_ZERO_VAL;
        	f_ent_dst->linkid 	= _FP_INVALID_INDEX;
        	f_ent_dst->objType 	= _FP_INVALID_INDEX;
        	f_ent_dst->index 	= _FP_INVALID_INDEX;
        	f_ent_dst->pri 		= _TK371X_ENTRY_PRIORITY_DEFAULT;
        	f_ent_dst->soc_action = _bcmTk371xFieldNoOp;
        	f_ent_dst->rule_para.vid_cos = _FP_ZERO_VAL;
        	f_ent_dst->volatiles = _FP_DEFAULT_VOLATILES;
        	f_ent_dst->bcm_action = NULL;
        	f_ent_dst->conds = NULL;
        	f_ent_dst->backup = NULL;
        	f_ent_dst->derivative = NULL;
    		/* Insert the entry node to the group entry link */
    		f_ent_dst->next = fg->entry;
    		fg->entry = f_ent_dst;
    	}
    	fg = fg->next;
    } /* while */
    if (group_flag){
    	return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_create_id
 *
 * Purpose:
 *     Create a blank entry group based on a group;
 *     allows selection of a specific slot in a slice
 *
 * Parameters:
 *     unit - BCM device number
 *     group - Field group ID
 *     entry - Requested entry ID; must be in the range prio_min through
 *             prio_max as returned by bcm_field_group_status_get().
 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_EXISTS    - Entry ID already in use
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_MEMORY    - allocation failure
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_create_id(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t 		*f_ent_src, *f_ent_dst = NULL;
	int group_flag = 1;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_create_id(unit=%d)\n"),
	           unit));
    f_ent_src = NULL;
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	f_ent_src = NULL;
	fg = fc->groups;
    if (fg == NULL){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	if (fg->group_id == group){
    		group_flag = 0;
       		rv = _bcm_tk371x_field_entry_get(unit, entry, &f_ent_src);
			if (rv == BCM_E_NONE){
				return BCM_E_EXISTS;
			}else if (rv == BCM_E_NOT_FOUND){
	            _FP_MEM_ALLOC(f_ent_dst, sizeof(_bcm_tk371x_field_entry_t), "_bcm_tk371x_field_entry_t");
				/* Insert the entry node to the group entry link */
				f_ent_dst->entry_id = entry;
				f_ent_dst->flags	= _FP_ZERO_VAL;
	        	f_ent_dst->linkid = _FP_INVALID_INDEX;
	        	f_ent_dst->objType = _FP_INVALID_INDEX;
	        	f_ent_dst->index = _FP_INVALID_INDEX;
	        	f_ent_dst->pri = _TK371X_ENTRY_PRIORITY_DEFAULT;
	        	f_ent_dst->soc_action = _bcmTk371xFieldNoOp;
	        	f_ent_dst->rule_para.vid_cos = _FP_ZERO_VAL;
	        	f_ent_dst->volatiles = _FP_DEFAULT_VOLATILES;
	        	f_ent_dst->bcm_action = NULL;
	        	f_ent_dst->conds = NULL;
	        	f_ent_dst->backup = NULL;
	        	f_ent_dst->derivative = NULL;
	    		f_ent_dst->next = fg->entry;
	    		fg->entry = f_ent_dst;
    		}else{
				return rv;
			}
    	}
    	fg = fg->next;
    } /* while */
    if (group_flag){
    	return BCM_E_NOT_FOUND;
    }
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_destroy
 *
 * Purpose:
 *     Destroy an entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_XXX       - From bcm_field_counter_destroy() or
 *                       bcm_field_meter_destroy()
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_destroy(
	    int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_cond_t  *cond, *cond_P = NULL;
	_bcm_tk371x_field_entry_t   	*fe,                /* entry structure to free up   */
	                        		*fe_prev = NULL;    /* Previous node in entry list */
	_bcm_tk371x_field_group_t		*group = NULL;
	_bcm_tk371x_field_action_t		*act, *act_P = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_destroy(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	fe = NULL;
	cond = NULL;
	act = NULL;
    /*destroy this entry*/
    group = fc->groups;
    while (group != NULL){
    	fe = group->entry;
    	while (fe != NULL){
    		if (fe->entry_id == entry){
    			/*
    			 * Remove the rules if there is installed
    			 */
    			bcm_tk371x_field_entry_remove(unit, entry);
    			/* destory all conditions */
    			cond_P = cond = fe->conds;
    			while (cond != NULL){
    				cond = cond->next;
    				sal_free(cond_P);
    				cond_P = cond;
    			}
        		/* free all actions */
        		act_P = act = fe->bcm_action;
        		while (act != NULL){
        			act = act->next;
        			sal_free(act_P);
        			act_P = act;
        		}

        		/* free back up information */
        		if (fe->backup != NULL){
        			sal_free(fe->backup);
        		}
    			/*free all derivative entry*/
    			if (fe->derivative != NULL){
    				if (fe->derivative->rule1 != NULL){
    					sal_free(fe->derivative->rule1);
    				}
    				if (fe->derivative->rule2 != NULL){
    					sal_free(fe->derivative->rule2);
    				}
					sal_free(fe->derivative);
    			}
        		/*destroy this entry*/
    			if (fe_prev == NULL){
    				group->entry = fe_prev;
    				sal_free(fe);
    			}else{
    				fe_prev->next = fe->next;
    				sal_free(fe);
    			}
    			return BCM_E_NONE;
    		}
            fe_prev = fe;
            fe      = fe->next;
    	}
    	/* find the entry*/
    	group = group->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 * Function: bcm_tk371x_field_entry_dump
 *
 * Purpose:
 *     Show contents of a field entry.
 *
 * Parameters:
 *     unit  - BCM device number
 *     entry - Field entry to dump
 *
 * Returns:
 *     Nothing.
 */
int
bcm_tk371x_field_entry_dump(
	    int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*entry_P = NULL;
	_bcm_tk371x_field_action_t		*act_P = NULL;
	_bcm_tk371x_field_entry_cond_t 	*conds_P = NULL;
	bcm_field_group_t gid;
	int i = 1, j = 0;
	int rv = 0;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_dump(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));

    rv = _bcm_tk371x_field_entry_group_id_get(unit, entry, &gid);
    if (rv != BCM_E_NONE){
    	return rv;
    }
    rv = _bcm_tk371x_field_entry_get(unit, entry, &entry_P);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    LOG_CLI(("Group ID :%d, Entry ID: %d\n", gid, entry));
    LOG_CLI(("TEK Rule Information:\n"));
    LOG_CLI(("linkid : %02X\n", entry_P->linkid));
    LOG_CLI(("ObjType: %02X\n", entry_P->objType));
    LOG_CLI(("index: %02X\n", entry_P->index));
    LOG_CLI(("volatiles : %02X\n", entry_P->volatiles));
    LOG_CLI(("pri : %02X\n", entry_P->pri));
    LOG_CLI(("soc_action: %02X\n", entry_P->soc_action));
    LOG_CLI(("Tk Rule Parameters:\n"));
    LOG_CLI(("vid_cos : %04X\n", entry_P->rule_para.vid_cos));
    LOG_CLI(("Teknovus conditions List:\n"));
    conds_P = entry_P->conds;
    while (conds_P != NULL){
    	LOG_CLI(("qaulify[%d]=%02X\n", i, conds_P->qual_id));
    	LOG_CLI(("qset[%d]=%02X\n", i, conds_P->qset));
    	LOG_CLI(("value[%d]=%02X", i, conds_P->common.value[0]));
    	for (j = 1; j < 8; j++){
    		LOG_CLI(("value[%d]=%02X", j, conds_P->common.value[j]));
    	}
    	LOG_CLI(("\n"));
    	LOG_CLI(("match[%d]=%02X\n", i, conds_P->match));
    	conds_P = conds_P->next;
       	i++;
    }
    LOG_CLI(("BCM Actions List:\n"));
    act_P = entry_P->bcm_action;
    while (act_P != NULL){
    	LOG_CLI(("action[%d]=%02X\n", i, act_P->action));
    	LOG_CLI(("param0[%d]=%08X\n", i, act_P->param0));
    	LOG_CLI(("param1[%d]=%08X\n", i, act_P->param1));
    	act_P = act_P->next;
    }
    if ((entry_P->flags & _TK371X_ENTRY_FLAG_INSTALLED)
    		&& (entry_P->flags & _TK371X_ENTRY_FLAG_NOT_EMPTY)){
		LOG_CLI(("BCM Rule Backup Information:\n"));
		LOG_CLI(("LogicalPortIndex.index=%02X\n", entry_P->backup->index.index));
		LOG_CLI(("LogicalPortIndex.objType=%02X\n", entry_P->backup->index.objType));
		LOG_CLI(("TkPortRuleInfo.actuion=%02X\n", entry_P->backup->info.action));
		LOG_CLI(("TkPortRuleInfo.param.vid_cos=%04X\n", entry_P->backup->info.param.vid_cos));
		LOG_CLI(("TkPortRuleInfo.priority=%02X\n", entry_P->backup->info.priority));
		LOG_CLI(("TkPortRuleInfo.volatiles=%02X\n", entry_P->backup->info.volatiles));
		LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionCount=%d\n",
				entry_P->backup->info.ruleCondition.conditionCount));
		for (i = 0; i < entry_P->backup->info.ruleCondition.conditionCount; i++){
			LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].field=%02X\n",
					i, entry_P->backup->info.ruleCondition.conditionList[i].field));
			LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].operator=%02X\n",
					i, entry_P->backup->info.ruleCondition.conditionList[i].operator));
			LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].comon.value", i));
			for (j = 0; j < 8; j++){
				LOG_CLI(("[%d]=%02X",
						j, entry_P->backup->info.ruleCondition.conditionList[i].common.value[j]));
			}
			LOG_CLI(("\n"));
		}
    }

    LOG_CLI(("Show derivative rules:\n"));
    if (entry_P->derivative != NULL){
    	if (entry_P->derivative->rule1 != NULL){
			LOG_CLI(("BCM derivative Rule One Information:\n"));
			LOG_CLI(("LogicalPortIndex.index=%02X\n", entry_P->derivative->rule1->index.index));
			LOG_CLI(("LogicalPortIndex.objType=%02X\n", entry_P->derivative->rule1->index.objType));
			LOG_CLI(("TkPortRuleInfo.actuion=%02X\n", entry_P->derivative->rule1->info.action));
			LOG_CLI(("TkPortRuleInfo.param.vid_cos=%04X\n", entry_P->derivative->rule1->info.param.vid_cos));
			LOG_CLI(("TkPortRuleInfo.priority=%02X\n", entry_P->derivative->rule1->info.priority));
			LOG_CLI(("TkPortRuleInfo.volatiles=%02X\n", entry_P->derivative->rule1->info.volatiles));
			LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionCount=%d\n",
					entry_P->derivative->rule1->info.ruleCondition.conditionCount));
			for (i = 0; i < entry_P->derivative->rule1->info.ruleCondition.conditionCount; i++){
				LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].field=%02X\n",
						i, entry_P->derivative->rule1->info.ruleCondition.conditionList[i].field));
				LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].operator=%02X\n",
						i, entry_P->derivative->rule1->info.ruleCondition.conditionList[i].operator));
				LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].comon.value", i));
				for (j = 0; j < 8; j++){
					LOG_CLI(("[%d]=%02X",
							j, entry_P->derivative->rule1->info.ruleCondition.conditionList[i].common.value[j]));
				}
				LOG_CLI(("\n"));
			}
    	}
		if (entry_P->derivative->rule2 != NULL){
			LOG_CLI(("BCM derivative Rule Two Information:\n"));
			LOG_CLI(("LogicalPortIndex.index=%02X\n", entry_P->derivative->rule2->index.index));
			LOG_CLI(("LogicalPortIndex.objType=%02X\n", entry_P->derivative->rule2->index.objType));
			LOG_CLI(("TkPortRuleInfo.actuion=%02X\n", entry_P->derivative->rule2->info.action));
			LOG_CLI(("TkPortRuleInfo.param.vid_cos=%04X\n", entry_P->derivative->rule2->info.param.vid_cos));
			LOG_CLI(("TkPortRuleInfo.priority=%02X\n", entry_P->derivative->rule2->info.priority));
			LOG_CLI(("TkPortRuleInfo.volatiles=%02X\n", entry_P->derivative->rule2->info.volatiles));
			LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionCount=%d\n",
					entry_P->derivative->rule2->info.ruleCondition.conditionCount));
			for (i = 0; i < entry_P->derivative->rule2->info.ruleCondition.conditionCount; i++){
				LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].field=%02X\n",
						i, entry_P->derivative->rule2->info.ruleCondition.conditionList[i].field));
				LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].operator=%02X\n",
						i, entry_P->derivative->rule2->info.ruleCondition.conditionList[i].operator));
				LOG_CLI(("TkPortRuleInfo.ruleCondition.conditionList[%d].comon.value", i));
				for (j = 0; j < 8; j++){
					LOG_CLI(("[%d]=%02X",
							j, entry_P->derivative->rule2->info.ruleCondition.conditionList[i].common.value[j]));
				}
				LOG_CLI(("\n"));
			}
		}
	}
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_install
 *
 * Purpose:
 *     Install a entry into the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to install
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit.
 *     BCM_E_XXX       - Error from _field_XX_tcam_policy_install()
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     Qualifications should be made and actions should be added
 *     prior to installing the entry.
 */
int
bcm_tk371x_field_entry_install(
	    int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*entry_P = NULL;
	_bcm_tk371x_field_entry_cond_t 	*conds_P = NULL;
	_bcm_tk371x_rule_t				*fec_bp = NULL;
	LogicalPortIndex 	index;
	TkPortRuleInfo		rule_info;
	int i = 0;
	int rv;
	uint8 soc_action;

#ifdef BROADCOM_DEBUG
	int j = 0;
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_install(unit=%d)\n"),
	           unit));
#endif
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));

    rv = _bcm_tk371x_field_entry_get(unit, entry, &entry_P);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    _bcm_tk371x_field_bcmActionToRuleAction_update(unit, entry,
    		entry_P->bcm_action, &soc_action);

    if ((entry_P->flags & _TK371X_ENTRY_FLAG_NOT_EMPTY) &&
    		(entry_P->flags & _TK371X_ENTRY_FLAG_INSTALLED) &&
    		!(entry_P->flags & _TK371X_ENTRY_FLAG_MODIFIED)){
    	return BCM_E_NONE;
    }
    if (!(entry_P->flags & _TK371X_ENTRY_FLAG_NOT_EMPTY) &&
    		!(entry_P->flags & _TK371X_ENTRY_FLAG_INSTALLED)){
    	entry_P->flags |= _TK371X_ENTRY_FLAG_INSTALLED;
    	return BCM_E_NONE;
    }

    index.index = entry_P->index;
    index.objType = entry_P->objType;
    rule_info.action = entry_P->soc_action;
    rule_info.param.vid_cos = entry_P->rule_para.vid_cos;
    rule_info.priority = entry_P->pri;
    rule_info.volatiles = entry_P->volatiles;

    /*fill the TkRuleConditionList*/
    conds_P = entry_P->conds;
    rule_info.ruleCondition.conditionCount = 0;
    i = 0;
    while (conds_P != NULL){
    	if (i == 8) break;
    	rule_info.ruleCondition.conditionCount++;
    	rule_info.ruleCondition.conditionList[i].field = conds_P->qset;
    	rule_info.ruleCondition.conditionList[i].operator = conds_P->match;
    	sal_memcpy((uint8*)&(rule_info.ruleCondition.conditionList[i].common.value[0]),
    			(uint8*)&(conds_P->common.value[0]), 8);
    	conds_P = conds_P->next;
    	i++;
    }
#ifdef BROADCOM_DEBUG
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "pathId=%d\n"),
               unit));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "LinkId=%d\n"),
               0));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "LogicalPortIndex.index=%02X\n"),
               index.index));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "LogicalPortIndex.objType=%02X\n"),
               index.objType));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "TkPortRuleInfo.actuion=%02X\n"),
               rule_info.action));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "TkPortRuleInfo.param.vid_cos=%04X\n"),
               rule_info.param.vid_cos));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "TkPortRuleInfo.priority=%02X\n"),
               rule_info.priority));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "TkPortRuleInfo.volatiles=%02X\n"),
               rule_info.volatiles));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "TkPortRuleInfo.ruleCondition.conditionCount=%d\n"),
               rule_info.ruleCondition.conditionCount));
    for (i = 0; i < rule_info.ruleCondition.conditionCount; i++){
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.ruleCondition.conditionList[%d].field=%02X\n"),
    	           i, rule_info.ruleCondition.conditionList[i].field));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.ruleCondition.conditionList[%d].operator=%02X\n"),
    	           i, rule_info.ruleCondition.conditionList[i].operator));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.ruleCondition.conditionList[%d].comon.value"),
    	           i));
    	for (j = 0; j < 8; j++){
    		LOG_DEBUG(BSL_LS_BCM_FP,
    		          (BSL_META_U(unit,
    		                      "[%d]=%02X"),
    		           j, rule_info.ruleCondition.conditionList[i].common.value[j]));
    	}
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "\n")));
    }
#endif
    rv = TkExtOamPortRuleAdd(unit, 0, index, &rule_info);
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "Call TkExtOamPortRuleAdd: rv=%d\n"),
               rv));
    if (OK == rv){
		entry_P->flags |= _TK371X_ENTRY_FLAG_INSTALLED;
		_FP_MEM_ALLOC(fec_bp, sizeof(_bcm_tk371x_rule_t), "_bcm_tk371x_rule_t");
		fec_bp->index.index = index.index;
		fec_bp->index.objType = index.objType;
		fec_bp->info.action = rule_info.action;
		fec_bp->info.param.vid_cos = rule_info.param.vid_cos;
		fec_bp->info.priority = rule_info.priority;
		fec_bp->info.volatiles = rule_info.volatiles;
		fec_bp->info.ruleCondition.conditionCount = rule_info.ruleCondition.conditionCount;
		for (i = 0; i < rule_info.ruleCondition.conditionCount; i++){
			fec_bp->info.ruleCondition.conditionList[i].field
					= rule_info.ruleCondition.conditionList[i].field;
			fec_bp->info.ruleCondition.conditionList[i].operator
					= rule_info.ruleCondition.conditionList[i].operator;
			sal_memcpy(&fec_bp->info.ruleCondition.conditionList[i].common.value[0],
					&rule_info.ruleCondition.conditionList[i].common.value[0],
					_TK371X_FP_RULE_COND_VALUE_MAX);
		}
		entry_P->backup = fec_bp;
		if (entry_P->derivative != NULL){
			if (entry_P->derivative->rule1 != NULL){
				sal_memcpy(&index, &(entry_P->derivative->rule1->index),
						sizeof(LogicalPortIndex));
				sal_memcpy(&rule_info, &(entry_P->derivative->rule1->info),
						sizeof(TkPortRuleInfo));

				rv = TkExtOamPortRuleAdd(unit, 0, index, &rule_info);
				if (OK != rv){
					return BCM_E_FAIL;
				}
			}
			if (entry_P->derivative->rule2 != NULL){
				sal_memcpy(&index, &(entry_P->derivative->rule2->index),
							sizeof(LogicalPortIndex));
				sal_memcpy(&rule_info, &(entry_P->derivative->rule2->info),
							sizeof(TkPortRuleInfo));
				rv = TkExtOamPortRuleAdd(unit, 0, index, &rule_info);
				if (OK != rv){
					return BCM_E_FAIL;
				}
    		}
		}
		return BCM_E_NONE;
    }else{
    	return BCM_E_FAIL;
    }
}

/*
 * Function: bcm_tk371x_field_entry_multi_get
 *
 * Purpose:
 *     Gets an array of a group's entry IDs
 *
 * Parameters:
 *     unit -  (IN) BCM device number.
 *     group - (IN) Field group ID.
 *     entry_size - (IN) Maximum number of entries to return.  Set to 0
 *                       to get the number of entries available
 *     entry_array - (OUT) Pointer to a buffer to fill with the array of
 *                         entry IDs.  Ignored if entry_size is 0
 *     entry_count - (OUT) Returns the number of entries returned in the
 *                         array, or if entry_size was 0, the number of
 *                         entries available

 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_multi_get(
	    int unit,
	    bcm_field_group_t group,
	    int entry_size,
	    bcm_field_entry_t *entry_array,
	    int *entry_count)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t 		*fe_t = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_multi_get(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));

	if (entry_size < 0){
		return BCM_E_PARAM;
	}
    fg = fc->groups;
    if (fg == NULL){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	*entry_count = 0;
    	if (fg->group_id == group){
    		fe_t = fg->entry;
    		while (fe_t != NULL){
    			/* if entry_size more than _FP_ENTRY_ID_MAX,
    			 * the entry_size give _FP_ENTRY_ID_MAX
    			 **/
    			if ((entry_size > _FP_ENTRY_ID_MAX) || (entry_size == 0)){
    				entry_size = _FP_ENTRY_ID_MAX;
    			}
    			if (*entry_count == entry_size){
    				return BCM_E_NONE;
    			}
    			entry_array[*entry_count] = fe_t->entry_id;
				(*entry_count)++;
    			fe_t = fe_t->next;
    		}
    		return BCM_E_NONE;
    	}
    	fg = fg->next;
    } /* end while */
	return BCM_E_NOT_FOUND;
}

/*
 * Function: bcm_tk371x_field_entry_prio_get
 *
 * Purpose:
 *     Gets the priority within the group of the entry.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Field entry to operate on
 *     prio   - (OUT) priority of entry
 *
 * Returns:
 *     BCM_E_NONE       - Success
 *     BCM_E_PARAM      - prio pointing to NULL
 *     BCM_E_NOT_FOUND  - Entry ID not found on unit
 */
int
bcm_tk371x_field_entry_prio_get(
	    int unit,
	    bcm_field_entry_t entry,
	    int *prio)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*fe = NULL;
	int ret_val;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_prio_get(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	ret_val = _bcm_tk371x_field_entry_get(unit, entry, &fe);
	if (ret_val != BCM_E_NONE){
		return ret_val;
	}
	*prio = (int)fe->pri;
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_prio_set
 *
 * Purpose:
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *
 * Returns:
 *     BCM_E_NONE       Success
 */
int
bcm_tk371x_field_entry_prio_set(
	    int unit,
	    bcm_field_entry_t entry,
	    int prio)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*fe = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_prio_set(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	rv = _bcm_tk371x_field_entry_get(unit, entry, &fe);
	if (rv != BCM_E_NONE){
		return rv;
	}
	fe->pri = (uint8)prio;
	fe->flags |= _TK371X_ENTRY_FLAG_NOT_EMPTY;
	fe->flags |= _TK371X_ENTRY_FLAG_MODIFIED;
	if (fe->derivative != NULL){
		if (fe->derivative->rule1 != NULL){
			fe->derivative->rule1->info.priority = fe->pri;
		}
		if (fe->derivative->rule2 != NULL){
			fe->derivative->rule2->info.priority = fe->pri;
		}
	}
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_entry_reinstall
 *
 * Purpose:
 *     Re-install a entry into the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to install
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit
 *     BCM_E_UNAVAIL   - Feature not implemented.
 *     BCM_E_XXX       - From _bcm_XXX_install() calls
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     Reinstallation may only be used to change the actions for
 *     an installed entry without having to remove and re-add the
 *     entry.
 */
int
bcm_tk371x_field_entry_reinstall(
	    int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*fe = NULL;
	int rv = BCM_E_NOT_FOUND;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_reinstall(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	rv = _bcm_tk371x_field_entry_get(unit, entry, &fe);
	if (rv == BCM_E_NONE){
		if ((fe->flags & _TK371X_ENTRY_FLAG_INSTALLED) &&
				(fe->flags & _TK371X_ENTRY_FLAG_NOT_EMPTY)){
			/* remove the old rule*/
			rv = bcm_tk371x_field_entry_remove(unit, entry);
			if (rv != BCM_E_NONE){
				return rv;
			}
		}
		return bcm_tk371x_field_entry_install(unit, entry);
	}
	return rv;
}

/*
 * Function: bcm_tk371x_field_entry_remove
 *
 * Purpose:
 *     Remove an entry from the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to remove
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit
 *     BCM_E_XXX
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     This does not destroy the entry; it uninstalls it from
 *     any hardware tables.
 *     Destroy a entry using bcm_field_entry_destroy.
 */
int
bcm_tk371x_field_entry_remove(
	    int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t 		*entry_P = NULL;
	int rv;

#ifdef BROADCOM_DEBUG
    int i;
	int j = 0;
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_entry_remove(unit=%d)\n"),
	           unit));
#endif
	if (fc == NULL){
		return BCM_E_INIT;
	}
    /* remove the entry from the hardware table */
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    rv = _bcm_tk371x_field_entry_get(unit, entry, &entry_P);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }
    /* if the rule has installed and is not null,
     * remove the hardware rule */
    if ((entry_P->flags & _TK371X_ENTRY_FLAG_INSTALLED) &&
    		(entry_P->flags & _TK371X_ENTRY_FLAG_NOT_EMPTY)){
#ifdef BROADCOM_DEBUG
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "pathId=%d\n"),
    	           unit));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "LinkId=%d\n"),
    	           0));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "LogicalPortIndex.index=%02X\n"),
    	           entry_P->backup->index.index));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "LogicalPortIndex.objType=%02X\n"),
    	           entry_P->backup->index.objType));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.actuion=%02X\n"),
    	           entry_P->backup->info.action));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.param.vid_cos=%04X\n"),
    	           entry_P->backup->info.param.vid_cos));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.priority=%02X\n"),
    	           entry_P->backup->info.priority));
    	LOG_DEBUG(BSL_LS_BCM_FP,
    	          (BSL_META_U(unit,
    	                      "TkPortRuleInfo.volatiles=%02X\n"),
    	           entry_P->backup->info.volatiles));
        LOG_DEBUG(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "TkPortRuleInfo.ruleCondition.conditionCount=%d\n"),
                   entry_P->backup->info.ruleCondition.conditionCount));
        for (i = 0; i < entry_P->backup->info.ruleCondition.conditionCount; i++){
        	LOG_DEBUG(BSL_LS_BCM_FP,
        	          (BSL_META_U(unit,
        	                      "TkPortRuleInfo.ruleCondition.conditionList[%d].field=%02X\n"),
        	           i, entry_P->backup->info.ruleCondition.conditionList[i].field));
        	LOG_DEBUG(BSL_LS_BCM_FP,
        	          (BSL_META_U(unit,
        	                      "TkPortRuleInfo.ruleCondition.conditionList[%d].operator=%02X\n"),
        	           i, entry_P->backup->info.ruleCondition.conditionList[i].operator));
        	LOG_DEBUG(BSL_LS_BCM_FP,
        	          (BSL_META_U(unit,
        	                      "TkPortRuleInfo.ruleCondition.conditionList[%d].comon.value"),
        	           i));
        	for (j = 0; j < 8; j++){
        		LOG_DEBUG(BSL_LS_BCM_FP,
        		          (BSL_META_U(unit,
        		                      "[%d]=%02X"),
        		           j, entry_P->backup->info.ruleCondition.conditionList[i].common.value[j]));
        	}
        	LOG_DEBUG(BSL_LS_BCM_FP,
        	          (BSL_META_U(unit,
        	                      "\n")));
        }
#endif

        rv = TkExtOamPortRuleDel(unit, 0 ,
        		entry_P->backup->index,
        		(TkPortRuleInfo*)&entry_P->backup->info);
        if (OK != rv){
        	return BCM_E_FAIL;
        }
        if (entry_P->derivative != NULL){
        	if (entry_P->derivative->rule1 != NULL){
        		rv = TkExtOamPortRuleDel(unit, 0 ,
            		entry_P->derivative->rule1->index,
            		(TkPortRuleInfo*)&entry_P->derivative->rule1->info);
				if (rv != OK){
					return BCM_E_FAIL;
				}
        	}
        	if (entry_P->derivative->rule2 != NULL){
        		rv = TkExtOamPortRuleDel(unit, 0 ,
					entry_P->derivative->rule2->index,
					(TkPortRuleInfo*)&entry_P->derivative->rule2->info);
				if (rv != OK){
					return BCM_E_FAIL;
				}
        	}
        }
    }
    entry_P->flags &= ~_TK371X_ENTRY_FLAG_INSTALLED;
    return BCM_E_NONE;
}


/*
 * Function:
 *     _robo_field_group_status_init
 * Purpose:
 *     Fill in the initial fields of a group status struct.
 * Parameters:
 *     unit  -  (IN)BCM unit number.
 *     fg_stat -  (OUT)Initialized field group status structure.
 * Retruns:
 *     BCM_E_XXX
 */
STATIC int
_bcm_tk371x_field_group_status_init(int unit, bcm_field_group_status_t *fg_stat) {

    if (NULL == fg_stat) {
        return (BCM_E_PARAM);
    }
    sal_memset(fg_stat, 0,  sizeof(bcm_field_group_status_t));
    fg_stat->prio_min       = 0;
    fg_stat->prio_max       = BCM_FIELD_ENTRY_PRIO_HIGHEST;
    fg_stat->entries_total  = _FP_ZERO_VAL;
    fg_stat->entries_free   = _FP_ZERO_VAL;
    fg_stat->counters_total  = _FP_ZERO_VAL;
    fg_stat->counters_free   = _FP_ZERO_VAL;
    fg_stat->meters_total  = _FP_ZERO_VAL;
    fg_stat->meters_free   = _FP_ZERO_VAL;
    return (BCM_E_NONE);
}


/*
 * Function: bcm_tk371x_field_group_create
 *
 * Purpose:
 *     Create a field group based on the field group selector flags.
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *     group - (OUT) New field group ID
 *
 * Returns:
 *     BCM_E_INIT     - BCM unit not initialized
 *     BCM_E_XXX      - Error code from _field_group_prio_make()
 *     BCM_E_PARAM    - pri out of range (0-15 for FB & ER) or group == NULL
 *     BCM_E_RESOURCE - no select codes will satisfy qualifier set
 *     BCM_E_NONE     - Success
 *
 * Notes:
 *      Allocates a hardware slice at the requested priority or better.
 *      Higher numerical value for priority has better priority for
 *      conflict resolution when there is a search hit on multiple slices.
 */
int
bcm_tk371x_field_group_create(
	    int unit,
	    bcm_field_qset_t qset,
	    int pri,
	    bcm_field_group_t *group)
{
    int     rv;
    bcm_field_group_t gid;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "FP: bcm_tk371x_field_group_create(unit=%d)\n"),
               unit));
    FIELD_IS_INIT(unit);
    rv = _bcm_tk371x_field_group_id_generate(unit, &gid);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: new group won't create.\n")));
        return rv;
    }
	rv = bcm_tk371x_field_group_create_id(unit, qset, pri, gid);
	if (rv != BCM_E_NONE){
		return rv;
	}
    *group = gid;
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_group_create_id
 *
 * Purpose:
 *     Create a field group based on the field group selector flags
 *     with a requested ID
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    group - Requested field group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - No unused group/slices left
 *     BCM_E_PARAM     - priority out of range (0-15 for FB & ER)
 *     BCM_E_EXISTS    - group with that id already exists on this unit.
 *     BCM_E_MEMORY    - Group memory allocation failure
 *     BCM_E_XXX       - Error code from _field_group_prio_make().
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_group_create_id(
	    int unit,
	    bcm_field_qset_t qset,
	    int pri,
	    bcm_field_group_t group)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	int rv = 0;

	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp) && 
			BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6)){
		return BCM_E_PARAM;
	}
	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp) &&
		BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6High)){
		return BCM_E_PARAM;
	}
	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp) &&
		BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6Low)){
		return BCM_E_PARAM;
	}
	
	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp) && 
			BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6)){
		return BCM_E_PARAM;
	}
	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp) &&
		BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6High)){
		return BCM_E_PARAM;
	}
	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp) &&
		BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6Low)){
		return BCM_E_PARAM;
	}

	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6)){
		if (!BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6High)){
			BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp6High);
		}
		if (!BCM_FIELD_QSET_TEST(qset, bcmFieldQualifyDstIp6Low)){
			BCM_FIELD_QSET_ADD(qset, bcmFieldQualifyDstIp6Low);
		}
	}
	if (BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6)){
		if (!BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6High)){
			BCM_FIELD_QSET_ADD(qset, bcmFieldQualifySrcIp6High);
		}
		if (!BCM_FIELD_QSET_TEST(qset, bcmFieldQualifySrcIp6Low)){
			BCM_FIELD_QSET_ADD(qset, bcmFieldQualifySrcIp6Low);
		}
	}
	
	fg = NULL;
    FIELD_IS_INIT(unit);
	
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    if (pri < 0 || pri > 15){
    	return BCM_E_PARAM;
    }
    if (group > _FP_GROUP_ID_MAX){
    	return BCM_E_RESOURCE;
    }
    rv = _bcm_tk371x_field_group_get(unit, group);
    if (rv == BCM_E_EXISTS){
    	return BCM_E_EXISTS;
    }

    _FP_MEM_ALLOC(fg, sizeof(_bcm_tk371x_field_group_t), "_bcm_tk371x_field_group_t");
    if (fg == NULL){
    	return BCM_E_MEMORY;
    }
    fg->group_id = group;
    rv = _bcm_tk371x_field_group_status_init(unit, &fg->group_status);
    if (BCM_FAILURE(rv)) {
        sal_free(fg);
        return rv;
    }
    sal_memcpy((uint8*)&(fg->qset), (uint8*)&qset, sizeof(bcm_field_qset_t));

    /* Insert new field group into head of Unit's groups list */
    fg->next = fc->groups;
    fc->groups = fg;
 	return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_group_dump
 *
 * Purpose:
 *     Show contents of a field group.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group to dump
 *
 * Returns:
 *     Nothing.
 */
int
bcm_tk371x_field_group_dump(
	    int unit,
	    bcm_field_group_t group)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_entry_t		*fe = NULL;
	int e_count = 0;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_group_dump(unit=%d)\n"),
	           unit));
	if (fc == NULL){
		return BCM_E_INIT;
	}
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    while (fg != NULL){
    	if (fg->group_id == group){
    		LOG_CLI(("group id=%d\n", fg->group_id));
			fe = fg->entry;
			while (fe != NULL){
				bcm_tk371x_field_entry_dump(unit, fe->entry_id);
				fe = fe->next;
				e_count++;
			}
			LOG_CLI(("group id:%d, entry number are %d\n", fg->group_id, e_count));
			return BCM_E_NONE;
    	}
    	fg = fg->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 * Function: bcm_tk371x_field_group_get
 *
 * Purpose:
 *     Get the qualifier set for a previously created field group
 *
 * Parameters:
 *     unit  - BCM device number
 *     port  - Port number
 *     group - Field group ID
 *     qset  - (OUT) Field qualifier set
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized.
 *     BCM_E_NOT_FOUND - Group ID not found in this unit
 *     BCM_E_PARAM     - qset is NULL
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_group_get(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_qset_t *qset)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*group_p = NULL;
	bcm_field_qset_t	qsets;
	int rv = 0;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: %s(unit=%d,  gid=%d)\n"),
	           FUNCTION_NAME(),unit, group));
    FIELD_IS_INIT(unit);
	if (qset == NULL){
		return BCM_E_PARAM;
	}
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    if (group > _FP_GROUP_ID_MAX){
    	return BCM_E_RESOURCE;
    }
    rv = _bcm_tk371x_field_group_get(unit, group);
    if (rv == BCM_E_EXISTS){
    	group_p = fc->groups;
    	while (group_p != NULL){
    		if (group_p->group_id == group){
    			BCM_FIELD_QSET_INIT(qsets);
    			sal_memcpy(&qsets, &(group_p->qset), sizeof(bcm_field_qset_t));
    			if (qset != NULL){
        			sal_memcpy(qset, &qsets, sizeof(bcm_field_qset_t));
    			}
    			break;
    		}
    		group_p = group_p->next;
    	}
    	return BCM_E_NONE;
    }
    return rv;
}
/*
 *Function
 *	   bcm_tk371x_field_group_install
 * Purpose:
 *	  Install all of a group's entries into the hardware tables.
 *
 * Parameters:
 *	  unit - BCM device number
 *	  group - Group to install
 *
 * Returns:
 *    BCM_E_NONE	  - Success
 *	  BCM_E_*		  - Error
 *	
 */

int 
bcm_tk371x_field_group_install(
    int unit, 
    bcm_field_group_t group)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t 	*fg = NULL;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	int rv = BCM_E_NONE;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: %s(unit=%d,  gid=%d)\n"),
	           FUNCTION_NAME(),unit, group));
	if (fc == NULL){
		return BCM_E_INIT;
	}
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    if (fg == NULL){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	if (fg->group_id == group){
    		fe = fg->entry;
    		while (fe != NULL){
    			rv = bcm_tk371x_field_entry_install(unit, fe->entry_id);
    			if (rv != BCM_E_NONE){
    				return rv;
    			}
    			fe = fe->next;
    		}
    		return BCM_E_NONE;
    	}
    	fg = fg->next;
    }
    return BCM_E_NOT_FOUND;
}
/*
 *   Function
 *      bcm_tk371x_field_group_remove
 *   Purpose
 *      Remove all of a group's entries from the hardware, but do not remove
 *      the entries from the software table.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This removes the group's entries from the hardware, marking them so,
 *      and commits the changes to the hardware.
 */
int
bcm_tk371x_field_group_remove(
	    int unit,
	    bcm_field_group_t group)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t 	*fg = NULL;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: %s(unit=%d,  gid=%d)\n"),
	           FUNCTION_NAME(),unit, group));
	if (fc == NULL){
		return BCM_E_INIT;
	}
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    if (fg == NULL){
    	return BCM_E_NOT_FOUND;
    }
    while (fg != NULL){
    	if (fg->group_id == group){
    		fe = fg->entry;
    		/*remove all entries in this group*/
    		while (fe != NULL){
    			rv = bcm_tk371x_field_entry_remove(unit, fe->entry_id);
    			if (rv == BCM_E_NONE){
    	   			fe = fe->next;
    			}else{
    				return rv;
    			}
    		}
    		return BCM_E_NONE;
    	}
    	fg = fg->next;
    }
    return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_group_set
 *   Purpose
 *      This changes the group's qualifier set so it is the specified set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) bcm_field_qset_t qset = new qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If there are any entries, all of them must be representable using the
 *      new qualifier set (if not, this fails), plus the new qualifier set can
 *      not change the required pattern type or stage (it will also fail in
 *      these cases).
 *      Updates are always permitted if there are no entries present.
 */
int
bcm_tk371x_field_group_set(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_qset_t qset)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	int empty_qset, qual;
	int retval;

	fg = NULL;
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "BEGIN bcm_field_group_set(unit=%d, group=%d)\n"),
	           unit, group));
	if (fc == NULL){
		return BCM_E_INIT;
	}
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    /* Check NULL qualifier set */
    empty_qset = 1;
    for (qual = 0; qual < bcmFieldQualifyCount; qual++) {
        if(BCM_FIELD_QSET_TEST(qset, qual)) {
            empty_qset = 0;
            break;
        }
    }
    if (empty_qset) {

        return BCM_E_UNAVAIL;
    }
    /* Get unit FP group structure */
    retval = _bcm_tk371x_field_group_get(unit, group);
    if (retval == BCM_E_EXISTS){
    	fg = fc->groups;
    	while (fg != NULL){
    		if (fg->group_id == group){
    			sal_memcpy(&fg->qset,  &qset, sizeof(bcm_field_qset_t));
    			return BCM_E_NONE;
    		}
    		fg = fg->next;
    	}
    }
	return retval;
}

/*
 *   Function
 *      bcm_tk371x_field_group_status_get
 *   Purpose
 *      Gets the group's status.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_group_status_t *status = where to put the status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_group_status_get(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_group_status_t *status)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t	*fg = NULL;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	int retval = BCM_E_NONE;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "BEGIN bcm_tk371x_field_group_status_get(unit=%d, group=%d)\n"),
	           unit, group));
	if (fc == NULL){
		return BCM_E_INIT;
	}
    if (status == NULL){
    	return BCM_E_PARAM;
    }
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    /* Get unit FP group structure */
    retval = _bcm_tk371x_field_group_get(unit, group);
    if (retval != BCM_E_EXISTS) {
        return retval;
    }
    status->entries_free = 0;
    status->counters_free = 0;
    status->counters_total = 0;
    status->meters_total = 0;
    status->meters_free = 0;
    status->counter_count = 0;
    status->prio_max = 15;
    status->prio_min = 0;
    status->meter_count = 0;
	status->entries_total = 0;
	status->entry_count = 0;
	fg = fc->groups;
	while (fg != NULL){
		if (fg->group_id == group){
			fe = fg->entry;
			while (fe != NULL){
		    	status->entries_total++;
		    	status->entry_count++;
		    	fe = fe->next;
			}
			break;
		}
		fg = fg->next;
	}
	return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tk371x_field_group_status_t_init
 * Purpose:
 *      Initialize the Field Group Status structure.
 * Parameters:
 *      fgroup - Pointer to Field Group Status structure.
 * Returns:
 *      NONE
 */
void
bcm_tk371x_field_group_status_t_init(bcm_field_group_status_t *fgroup)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META("BEGIN bcm_tk371x_field_group_status_t_init\n")));
	if (fgroup != NULL){
		sal_memset(fgroup, 0, sizeof(bcm_field_group_status_t));
	}
}

/*
 * Function:
 *      bcm_tk371x_field_group_traverse
 * Purpose:
 *      Traverse all the fp groups in the system, calling a specified
 *      callback for each one
 * Parameters:
 *      unit - (IN) Unit number.
 *      callback - (IN) A pointer to the callback function to call for each fp group
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_tk371x_field_group_traverse(
	    int unit,
	    bcm_field_group_traverse_cb callback,
	    void *user_data)
{
    bcm_field_group_t *grp_arr;							/* Field group pointers.    */
    _bcm_tk371x_field_control_t *fc = _field_control;   /* Field control structure. */
    _bcm_tk371x_field_group_t *fg = NULL;        		/* Field group structure.   */
    int group_count;           /* Number of fp groups.     */
    int mem_sz;                /* Allocated memory size.   */
    int idx;                   /* Group array iterator.    */
    int rv = BCM_E_NONE;       /* Operation return status. */

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "BEGIN bcm_tk371x_field_group_status_get(unit=%d)\n"),
               unit));
    /* Input parameters check. */
    if (NULL == callback) {
        return (BCM_E_PARAM);
    }

    /* Field control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));

    /* Count fp groups. */
    fg = fc->groups;
    group_count = 0;
    while (fg != NULL) {
        group_count++;
        fg = fg->next;
    }

    if (0 == group_count) {
        return (rv);
    }

    /*
     * API can not use field contol groups linked list,
     * since group might be destroyed in callback.
     */
    mem_sz = group_count * sizeof(bcm_field_group_t);
    grp_arr = NULL;
    grp_arr = sal_alloc(mem_sz, "FP groups array");
    if (NULL == grp_arr) {
        return (BCM_E_MEMORY);
    }

    /* Programm fp group ids into allocated array. */
    fg = fc->groups;
    idx = 0;
    while (fg != NULL) {
        grp_arr[idx] = fg->group_id;
        idx++;
        fg = fg->next;
    }

    /* Call user callback. */
    for (idx = 0; idx < group_count; idx++) {
        rv = (*callback)(unit, grp_arr[idx], user_data);
        if (BCM_FAILURE(rv)) {
            break;
        }
    }
    sal_free(grp_arr);
    return (rv);
}

/*
 * Function: bcm_tk371x_field_init
 *
 * Purpose:
 *     Initialize field module.
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_UNIT    - Invalid BCM unit number.
 *     BCM_E_UNAVAIL - Field Processor not on device.
 *     BCM_E_MEMORY  - Allocation failure
 *     BCM_E_XXX     - Error code from bcm_XX_field_init()
 *     BCM_E_NONE    - Success
 */
int
bcm_tk371x_field_init(int unit)
{
	int retval;
	_bcm_tk371x_field_control_t 	*fc = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_init(unit=%d)\n"),
	           unit));
    /* Make sure the Unit can support this module. */
    if (!BCM_TK371X_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    if (!soc_feature(unit, soc_feature_field)) {
        LOG_ERROR(BSL_LS_BCM_FP,
                  (BSL_META_U(unit,
                              "FP Error: No Field Processor available Unit=%d\n"),
                   unit));
        return BCM_E_UNAVAIL;
    }
    assert(BCM_FIELD_QUALIFY_MAX        >= bcmFieldQualifyCount);
    /* Detatch first if it's been previously initialized. */
    if (_field_control != NULL) {
        retval = bcm_tk371x_field_detach(unit);
        if (BCM_FAILURE(retval)) {
            LOG_ERROR(BSL_LS_BCM_FP,
                      (BSL_META_U(unit,
                                  "FP(unit %d) Error: Module deinit failed.\n"),
                       unit));
            return (retval);
        }
    }
	/* Allocate a bcm_field_control */
	fc = sal_alloc(sizeof (_bcm_tk371x_field_control_t), "field_control");
	if (fc == NULL) {
		LOG_ERROR(BSL_LS_BCM_FP,
		          (BSL_META_U(unit,
		                      "FP Error: Allocation failure for Field Control\n")));
		return BCM_E_MEMORY;
	}
	sal_memset(fc, 0, sizeof (_bcm_tk371x_field_control_t));
	fc->groups = NULL;
	fc->ranges = NULL;
	_field_control = fc;
    return BCM_E_NONE;
}

/*
 * Function: bcm_tk371x_field_qset_add_udf
 *
 * Purpose:
 *     Add a UDF definition to a qset
 *
 * Parameters:
 *     unit     - BCM device number
 *     qset     - Pointer to a qset structure
 *     udf_id   - UDF ID to add to the qset
 *
 * Returns:
 *     BCM_E_INIT     - BCM unit not initialized
 *     BCM_E_RESOURCE - Cannot add UDF (all UDFs used)
 *     BCM_E_NONE     - Success
 *     BCM_E_XXX
 */
int
bcm_tk371x_field_qset_add_udf(
	    int unit,
	    bcm_field_qset_t *qset,
	    bcm_field_udf_t udf_id)
{
	_bcm_tk371x_field_control_t *fc = _field_control;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_qset_add_udf(unit=%d)\n"),
	           unit));
    FIELD_IS_INIT(unit);

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    if ( (qset == NULL) ||
         (!fc->udf[udf_id].valid) ) {
        return BCM_E_NOT_FOUND;
    }
    SHR_BITSET(qset->udf_map, udf_id); /* in use */
    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qset_t_init
 * Purpose:
 *      Initialize the bcm_field_qset_t structure.
 * Parameters:
 *      qset - Pointer to field qset structure.
 * Returns:
 *      NONE
 */
void
bcm_tk371x_field_qset_t_init(bcm_field_qset_t *qset)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META("FP: bcm_tk371x_field_qset_t_init\n")));
	if (qset != NULL){
		sal_memset(qset, 0, sizeof(bcm_field_qset_t));
	}
}

/*
 * Function:
 *      bcm_tk371x_field_qset_udf_get
 * Purpose:
 *      Get array of virtual UDF resources included in group qset.
 * Parameters:
 *      unit         - (IN) bcm device.
 *      qset         - (IN) Group qualifier set.
 *      udf_max      - (IN) Maximum qualifiers to fill.
 *      udf_arr      - (OUT) Udf ids array.
 *      udf_count    - (OUT) Number of udf ids filled in udf_arr.
 * Returns:
 *      BCM_E_XXX
 */
int
bcm_tk371x_field_qset_udf_get(
	    int unit,
	    bcm_field_qset_t *qset,
	    int udf_max,
	    int *udf_arr,
	    int *udf_count)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_qset_udf_get(unit=%d)\n"),
	           unit));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualifier_delete
 * Purpose:
 *      Remove match criteria from a field processor entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      qual_id - (IN) BCM field qualifier id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualifier_delete(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_qualify_t qual_id)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t *fe_cond, *fe_cond_prev = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "BEGIN bcm_tk371x_field_qualifier_delete(unit=%d, entry=%d qual_id=%d)\n"),
	           unit, entry, qual_id));
    FIELD_IS_INIT(unit);
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fe_cond = NULL;
	rv = _bcm_tk371x_field_entry_get(unit, entry, &fe);
	if (rv != BCM_E_NONE){
		return BCM_E_NONE;
	}
	fe_cond = fe->conds;
	/* find the qualid and delete the cond*/
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
       		/*destroy this condition*/
			if (fe_cond_prev == NULL){
				fe->conds = fe_cond_prev;
				sal_free(fe_cond);
			}else{
				fe_cond_prev->next = fe_cond->next;
				sal_free(fe_cond);
			}
			return BCM_E_NONE;
		}
		fe_cond_prev = fe_cond;
		fe_cond = fe_cond->next;
	}
	return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DSCP
 *   Purpose
 *      Set expected IPv4 DSCP for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which DSCP
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_qualify_DSCP(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	int qual_id = bcmFieldQualifyDSCP;
	int qset = _bcmTk371xFieldEntryCondQsetUser1;
	int len = sizeof(uint8);
	uint8 match;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "(unit=%d,entry=%08X,data=%02X,mask=%02X)\n"),
	           unit,
	           entry,
	           data,
	           mask));

	match = _bcm_ea_field_operator_cond(&data, &mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DSCP_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyDSCP;
	int ret_val;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp(unit=%d,entry=%08X)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy(data,&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								data, mask, sizeof(uint8));
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstIp
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_DstIp(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip_t data,
	    bcm_ip_t mask)
{
	uint8 match;
	int qual_id = bcmFieldQualifyDstIp;
	int qset = _bcmTk371xFieldEntryCondQsetIpDa;
	int rv;
	int len = sizeof(bcm_ip_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp (unit=%d,entry=%08X)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	rv = _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
	return rv;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstIp6
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv6 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_DstIp6(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	int rv;

	rv = bcm_tk371x_field_qualify_DstIp6High(unit, entry, data, mask);
	if (rv != BCM_E_NONE){
		return rv;
	}
	rv = bcm_tk371x_field_qualify_DstIp6Low(unit, entry, data, mask);
	return rv;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstIp6High
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv6 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_DstIp6High(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp6High (unit=%d,entry=%08X)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data[len], (uint8*)&mask[len], len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	qual_id = bcmFieldQualifyDstIp6High;
	qset = _bcmTk371xFieldEntryCondQsetIpv6DaHi;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data[len], len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6High_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyDstIp6High;
	int ret_val;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp6High_get (unit=%d,entry=%08X)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)&(*data)[len],
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)&(*data)[len], (uint8*)&(*mask)[len], len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_ip6_t));
				return ret_val;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6Low(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	int qual_id = bcmFieldQualifyDstIp6Low;
	int qset = _bcmTk371xFieldEntryCondQsetIpDa;
	uint8 match;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp6Low (unit=%d,entry=%08X)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data[0], (uint8*)&mask[0], len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data[0], len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6Low_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyDstIp6Low;
	int ret_val;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp6Low_get (unit=%d,entry=%08X)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)(*data),
				&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)(*data), (uint8*)(*mask), len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_ip6_t));
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	int ret_val;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp6_get (unit=%d,entry=%08X)\n"),
	           unit, entry));
	ret_val = bcm_tk371x_field_qualify_DstIp6Low_get(unit, entry, data, mask);
	if (ret_val != BCM_E_NONE){
		return ret_val;
	}
	ret_val = bcm_tk371x_field_qualify_DstIp6High_get(unit, entry, data, mask);

	return ret_val;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip_t *data,
	    bcm_ip_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyDstIp;
	int ret_val;
	int len = sizeof(bcm_ip_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp_get (unit=%d,entry=%08X)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_ip_t));
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstMac
 *   Purpose
 *      Set expected destination MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which destination MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the destination MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_DstMac(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t data,
	    bcm_mac_t mask)
{
	int qual_id = bcmFieldQualifyDstMac;
	int qset = _bcmTk371xFieldEntryCondQsetEthernetDa;
	uint8 match;
	int len = sizeof(bcm_mac_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstMac (unit=%d,entry=%08X)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data[0], (uint8*)&mask[0], len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data[0], len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstMac_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t *data,
	    bcm_mac_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyDstMac;
	int ret_val;
	int len = sizeof(bcm_mac_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstMac_get (unit=%d,entry=%08X)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_mac_t));
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_EtherType
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_EtherType(
	    int unit,
	    bcm_field_entry_t entry,
	    uint16 data,
	    uint16 mask)
{
	int qual_id = bcmFieldQualifyEtherType;
	int qset = _bcmTk371xFieldEntryCondQsetL2LengthType;
	uint8 match;
	int len = sizeof(uint16);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_EtherType (unit=%d,entry=%08X)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, (uint8)match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_EtherType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_EtherType_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint16 *data,
	    uint16 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyEtherType;
	int ret_val;
	int len = sizeof(uint16);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_EtherType_get (unit=%d,entry=%08X)\n"),
	           unit, entry));
    FIELD_IS_INIT(unit);
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

static int
_bcm_tk371x_field_port_info_update(
		_bcm_tk371x_field_entry_t *fe, bcm_port_t port){
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META("_bcm_tk371x_field_port_info_update\n")));
	if (port >= _BCM_TK371X_LLID_PORT_BASE){
		fe->linkid = port - _BCM_TK371X_LLID_PORT_BASE;
		fe->objType = TkObjTypeLink;
		fe->index = (uint8)port - _BCM_TK371X_LLID_PORT_BASE;
	}else{
		fe->linkid = 0;
		fe->objType = TkObjTypePort;
		fe->index = (uint8)port;
	}
	return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_InPort
 *   Purpose
 *      Set allowed ingress port for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = allowed port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Supports GPORTs of various types and will map back to phys port.
 */

int
bcm_tk371x_field_qualify_InPort(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_port_t data,
	    bcm_port_t mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_InPort(%d,%d,%d%d)\n"),
	           unit, entry, data, mask));
    /* coverity[result_independent_of_operands] */
    if (0 == SOC_PORT_VALID(unit, data)){
		LOG_ERROR(BSL_LS_BCM_FP,
		          (BSL_META_U(unit,
		                      "FP Error: the port index is more then the max port number.\n")));
		return BCM_E_PARAM;
	}
    if (1 == TK371X_PORT_VALID(data)){
    	if ((_FP_PORT_EPON & mask) || (_FP_PORT_GE & mask) ||
    			(_FP_PORT_FE & mask) || (_FP_PORT_LLID1 & mask) ||
    			(_FP_PORT_LLID2 & mask) || (_FP_PORT_LLID3 & mask) ||
    			(_FP_PORT_LLID4 & mask) || (_FP_PORT_LLID5 & mask) ||
    			(_FP_PORT_LLID6 & mask) || (_FP_PORT_LLID7 & mask) ||
    			(_FP_PORT_LLID8 & mask)	){
    		BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    		rv = _bcm_tk371x_field_entry_get(unit, entry, &fe);
    		if (rv != BCM_E_NONE){
    			return rv;
    		}
    		_bcm_tk371x_field_port_info_update(fe, data);
    		if (fe->derivative != NULL){
    			if (fe->derivative->rule1 != NULL){
    				fe->derivative->rule1->index.index = fe->index;
    				fe->derivative->rule1->index.objType = fe->objType;
    			}
    			if (fe->derivative->rule2 != NULL){
    				fe->derivative->rule2->index.index = fe->index;
    				fe->derivative->rule2->index.objType = fe->objType;
    			}
    		}
    		fe->flags |= _TK371X_ENTRY_FLAG_NOT_EMPTY;
    		fe->flags |= _TK371X_ENTRY_FLAG_MODIFIED;

    		return BCM_E_NONE;
    	}else{
    		return BCM_E_PARAM;
    	}
    }else{
    	return BCM_E_PARAM;
    }
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_InPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_InPort_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_port_t *data,
	    bcm_port_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_InPort_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	rv = _bcm_tk371x_field_entry_get(unit, entry, &fe);
	if (rv != BCM_E_NONE){
		return rv;
	}
	if (fe->objType == TkObjTypeLink){
		*data = fe->index + _BCM_TK371X_LLID_PORT_BASE;
		switch (*data){
			case 3:
				*mask |= _FP_PORT_LLID1;
				break;
			case 4:
				*mask |= _FP_PORT_LLID2;
				break;
			case 5:
				*mask |= _FP_PORT_LLID3;
				break;
			case 6:
				*mask |= _FP_PORT_LLID4;
				break;
			case 7:
				*mask |= _FP_PORT_LLID5;
				break;
			case 8:
				*mask |= _FP_PORT_LLID6;
				break;
			case 9:
				*mask |= _FP_PORT_LLID7;
				break;
			case 10:
				*mask |= _FP_PORT_LLID8;
				break;
			default:
				return BCM_E_FAIL;
		}
	}else if(fe->objType == TkObjTypePort){
		*data = fe->index;
		switch (*data){
			case 0:
				*mask |= _FP_PORT_EPON;
				break;
			case 1:
				*mask |= _FP_PORT_GE;
				break;
			case 2:
				*mask |= _FP_PORT_FE;
				break;
			default:
				return BCM_E_FAIL;
		}
	}else{
		return BCM_E_FAIL;
	}
	return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_InPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t data = allowed ports
 *      (in) bcm_pbmp_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_qualify_InPorts(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_pbmp_t data,
	    bcm_pbmp_t mask)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_InPorts(%d,%d)\n"),
	           unit, entry));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_InPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_InPorts_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_pbmp_t *data,
	    bcm_pbmp_t *mask)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_InPorts_get(%d,%d)\n"),
	           unit, entry));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6FlowLabel
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6FlowLabel(
	    int unit,
	    bcm_field_entry_t entry,
	    uint32 data,
	    uint32 mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(uint32);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_Ip6FlowLabel(%d,%d)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	qual_id = bcmFieldQualifyIp6FlowLabel;
	qset = _bcmTk371xFieldEntryCondQsetUser0;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6FlowLabel_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint32 *data,
	    uint32 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyIp6FlowLabel;
	int ret_val;
	int len = sizeof(uint32);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_Ip6FlowLabel_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6NextHeader
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6NextHeader
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6NextHeader(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_Ip6NextHeader(%d,%d)\n"),
	           unit, entry));
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "(%d,%08X,%016X,%016X)\n"),
	           unit,
	           entry,
	           data,
	           mask));
	match = _bcm_ea_field_operator_cond(&data, &mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	qual_id = bcmFieldQualifyIp6NextHeader;
	qset = _bcmTk371xFieldEntryCondQsetL3ProtocolType;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6NextHeader_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6NextHeader
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6NextHeader_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyIp6NextHeader;
	int ret_val;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_Ip6NextHeader_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								data, mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}
int
bcm_tk371x_field_qualify_Ip6TrafficClass(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_Ip6TrafficClass(%d,%d)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond(&data, &mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	qual_id = bcmFieldQualifyIp6TrafficClass;
	qset = _bcmTk371xFieldEntryCondQsetUser3; /* the value is 7 */
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6TrafficClass_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6TrafficClass
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6TrafficClass_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyIp6TrafficClass;
	int ret_val;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_Ip6TrafficClass_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								data, mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}

	return BCM_E_NOT_FOUND;
}
/*
 *   Function
 *      bcm_tk371x_field_qualify_IpProtocol
 *   Purpose
 *      Set expected IPv4 protocol type type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_IpProtocol(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_IpProtocol(%d,%d)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond(&data, &mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	qual_id = bcmFieldQualifyIpProtocol;
	qset = _bcmTk371xFieldEntryCondQsetL3ProtocolType; /* Ip protocol */
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

int
bcm_tk371x_field_qualify_IpProtocolCommon(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpProtocolCommon_t protocol)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_IpProtocolCommon(%d,%d)\n"),
	           unit, entry));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier protocol encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpProtocolCommon_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpProtocolCommon_t *protocol)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_IpProtocolCommon_get(%d,%d)\n"),
	           unit, entry));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpProtocol_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyIpProtocol;
	int ret_val;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_IpProtocol_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								data, mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}

	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpType
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpType(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpType_t type)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_IpType(%d,%d)\n"),
	           unit, entry));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpType_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpType_t *type)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_IpType_get(%d,%d)\n"),
	           unit, entry));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_L4DstPort
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_L4DstPort(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t data,
	    bcm_l4_port_t mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(bcm_l4_port_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_L4DstPort(%d,%d)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	qual_id = bcmFieldQualifyL4DstPort;
	qset = _bcmTk371xFieldEntryCondQsetUser5; /* the value is 7 */
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_L4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_L4DstPort_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t *data,
	    bcm_l4_port_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyL4DstPort;
	int ret_val;
	int len = sizeof(bcm_l4_port_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_L4DstPort_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}

	return BCM_E_NOT_FOUND;
}

int
bcm_tk371x_field_qualify_L4SrcPort(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t data,
	    bcm_l4_port_t mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(bcm_l4_port_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_L4SrcPort(%d,%08X,%08X,%08X)\n"),
	           unit,
	           entry,
	           data,
	           mask));

	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	qual_id = bcmFieldQualifyL4SrcPort;
	qset = _bcmTk371xFieldEntryCondQsetUser4;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_L4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_L4SrcPort_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t *data,
	    bcm_l4_port_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyL4SrcPort;
	int ret_val;
	int len = sizeof(bcm_l4_port_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_L4SrcPort_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}

	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_OuterVlanId
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VID (12 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_OuterVlanId(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_vlan_t data,
	    bcm_vlan_t mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(bcm_vlan_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_OuterVlanId(%d,%08X,%08X,%08X)\n"),
	           unit,
	           entry,
	           data,
	           mask));
	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	qual_id = bcmFieldQualifyOuterVlanId;
	qset = _bcmTk371xFieldEntryCondQsetVlanId;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_OuterVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_OuterVlanId_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_vlan_t *data,
	    bcm_vlan_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyOuterVlanId;
	int ret_val;
	int len = sizeof(bcm_vlan_t);


	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_OuterVlanId_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_OuterVlanPri
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which Pri (3 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_OuterVlanPri(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_OuterVlanPri(%d,%08X,%016X,%016X)\n"),
	           unit,
	           entry,
	           data,
	           mask));
	match = _bcm_ea_field_operator_cond(&data, &mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	qual_id = bcmFieldQualifyOuterVlanPri;
	qset = _bcmTk371xFieldEntryCondQsetUser6;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_OuterVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_OuterVlanPri_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyOuterVlanPri;
	int ret_val;
	int len = sizeof(uint8);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_OuterVlanPri_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								data, mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_RangeCheck
 *   Purpose
 *      Set expected TCP/UDP port range for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_range_t range = which ethertype
 *      (in) int invert = whether the range match is to be inverted
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The range that is specified is read only during this call; it will not
 *      be read later if that range changes; another call to this function will
 *      be required should the range change and the update need to apply.
 *      The invert flag is not supported.
 *      This can't use the helper functions because it is setting a more
 *      complex set of fields under a more complex set of conditions.
 */
int
bcm_tk371x_field_qualify_RangeCheck(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_range_t range,
	    int invert)
{
	_bcm_tk371x_field_control_t *fc = _field_control;
	_bcm_tk371x_field_range_t	*fr = NULL;
	_bcm_tk371x_field_tk_match_t	*tk_match = NULL;
	int i = 0;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_RangeCheck(%d,%d)\n"),
	           unit, entry));
    if (fc == NULL){
    	return BCM_E_INIT;
    }
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_range_get(unit, range, &fr));
	fr->eid = entry;
	if (invert == TRUE){
		fr->flags &= BCM_FIELD_RANGE_INVERT;
	}
    rv =  _bcm_tk371x_range_tk_match_update(fr);
    if (rv != BCM_E_NONE){
    	return rv;
    }

	tk_match = fr->tk_match;
	while (tk_match != NULL){
		if (i > 8){
			break;
		}
		rv = _bcm_tk371x_field_entry_qualify_update(unit,
				entry, tk_match->qid, tk_match->field,
				tk_match->match, (uint8*)&tk_match->value, sizeof(tk_match->value));
		if (rv == BCM_E_NOT_FOUND){
			return rv;
		}
		tk_match = tk_match->next;
		i++;
	}
	return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_RangeCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyRangeCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      max_count - (IN) Max entries to fill.
 *      range - (OUT) Range checkers array.
 *      invert - (OUT) Range checkers invert array.
 *      count - (OUT) Number of filled range checkers.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_RangeCheck_get(
	    int unit,
	    bcm_field_entry_t entry,
	    int max_count,
	    bcm_field_range_t *range,
	    int *invert,
	    int *count)
{
	_bcm_tk371x_field_control_t *fc = _field_control;
	_bcm_tk371x_field_range_t	*fr = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_RangeCheck_get(%d,%d)\n"),
	           unit, entry));
    if (fc == NULL){
    	return BCM_E_INIT;
    }
    if (max_count != 1){
    	return BCM_E_UNAVAIL;
    }
    fr = fc->ranges;
    while (fr != NULL){
    	if (fr->eid == entry){
    		*range = fr->rid;
    		if (fr->flags & BCM_FIELD_RANGE_INVERT){
    			*invert = 1;	/*It means true */
    		}
    		*invert = 0;		/*It means false*/
    		return BCM_E_NONE;
    	}
    	fr = fr->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_SrcIp(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip_t data,
	    bcm_ip_t mask)
{
	uint8 match;
	int qual_id = bcmFieldQualifySrcIp;
	int qset = _bcmTk371xFieldEntryCondQsetIpSa;
	int rv;
	int len = sizeof(bcm_ip_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcIp(%d,%08X,%08X,%08X)\n"),
	           unit,
	           entry,
	           data,
	           mask));
	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	rv = _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
	return rv;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcIp6
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_SrcIp6(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	int rv;

	rv = bcm_tk371x_field_qualify_SrcIp6High(unit, entry, data, mask);
	if (rv != BCM_E_NONE){
		return rv;
	}
	rv = bcm_tk371x_field_qualify_SrcIp6Low(unit, entry, data, mask);
	return rv;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcIp6High
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_SrcIp6High(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	int qual_id;
	int qset;
	uint8 match;
	int len = sizeof(bcm_ip6_t) / 2;

	match = _bcm_ea_field_operator_cond((uint8*)&data[len], (uint8*)&mask[len], len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}

	qual_id = bcmFieldQualifySrcIp6High;
	qset = _bcmTk371xFieldEntryCondQsetIpv6SaHi;
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data[len], len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6High_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifySrcIp6High;
	int ret_val;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcIp6High_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)&(*data)[len],
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)&(*data)[len], (uint8*)&(*mask)[len], len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_ip6_t));
				return ret_val;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6Low(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	int qual_id = bcmFieldQualifySrcIp6Low;
	int qset = _bcmTk371xFieldEntryCondQsetIpSa;
	uint8 match;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcIp6Low(%d,%d)\n"),
	           unit, entry));
	match = _bcm_ea_field_operator_cond((uint8*)data, (uint8*)mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6Low_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifySrcIp6Low;
	int ret_val;
	int len = sizeof(bcm_ip6_t) / 2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcIp6Low_get(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)data,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_ip6_t));
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	int ret_val;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcIp6_get(%d,%d)\n"),
	           unit, entry));
	ret_val = bcm_tk371x_field_qualify_SrcIp6Low_get(unit, entry, data, mask);
	if (ret_val != BCM_E_NONE){
		return ret_val;
	}
	ret_val = bcm_tk371x_field_qualify_SrcIp6High_get(unit, entry, data, mask);
	return ret_val;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_SrcMac(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t data,
	    bcm_mac_t mask)
{
	int qual_id = bcmFieldQualifySrcMac;
	int qset = _bcmTk371xFieldEntryCondQsetEthernetSa;
	uint8 match;
	int len = sizeof(bcm_mac_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcMac(%d,%d)\n"),
	           unit, entry));
    FIELD_IS_INIT(unit);
	match = _bcm_ea_field_operator_cond((uint8*)&data[0], (uint8*)&mask[0], len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data[0], len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcMac_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t *data,
	    bcm_mac_t *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifySrcMac;
	int ret_val;
	int len = sizeof(bcm_mac_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_SrcMac_get(%d,%d)\n"),
	           unit, entry));
    FIELD_IS_INIT(unit);
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)&data[0],
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				sal_memset(mask, 0, sizeof(bcm_mac_t));
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}

/*
 * Function: bcm_tk371x_field_qualify_clear
 *
 * Purpose:
 *     Remove all field qualifications from a filter entry
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *
 * Returns:
 *     BCM_E_INIT       BCM Unit not initialized.
 *     BCM_E_NOT_FOUND  Entry ID not found in unit.
 *     BCM_E_NONE       Success
 */
int
bcm_tk371x_field_qualify_clear(
		int unit,
	    bcm_field_entry_t entry)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t		*fe = NULL;
	_bcm_tk371x_field_entry_cond_t  *cond = NULL;
	int rv;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_clear(%d,%d)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	/* destory all conditions */
	cond = fe->conds;
	while (cond != NULL){
		rv = bcm_tk371x_field_qualifier_delete(unit, entry, cond->qual_id);
		if (rv != BCM_E_NONE){
			return rv;
		}
		cond = cond->next;
	}
    return BCM_E_NONE;
}



/*
 *   Function
 *      bcm_tk371x_field_range_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_create(
	    int unit,
	    bcm_field_range_t *range,
	    uint32 flags,
	    bcm_l4_port_t min,
	    bcm_l4_port_t max)
{
    _bcm_tk371x_field_control_t    *fc = _field_control;
    int                 			rv;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "BEGIN bcm_tk371x_field_range_create(unit=%d, range->0x%x,"),
               unit, *range));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "flags=0x%08x, min=0x%x, max=0x%x)\n"),
               flags, min, max));
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    /* Range type sanity check. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_range_flags_check(unit, flags));
    rv = _bcm_tk371x_field_range_create(unit, range, flags, min, max);
    return rv;
}


STATIC int
_bcm_tk371x_field_range_item_insert(
		int unit,
		bcm_field_range_t range,
		uint32 flags,
		bcm_l4_port_t min,
		bcm_l4_port_t max){
    _bcm_tk371x_field_control_t    *fc = _field_control;
    _bcm_tk371x_field_range_t      *fr = NULL;
    uint32            ranger_num = 0;
    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    /* Search existing ranges */
    for (fr = fc->ranges;  fr != NULL; fr = fr->next) {
        if (fr->rid == range) {
            return (BCM_E_EXISTS);
        }
        ranger_num++;
    }
    if (ranger_num > _FP_RANGE_ID_MAX){
    	return BCM_E_RESOURCE;
    }
	_FP_MEM_ALLOC(fr, sizeof(_bcm_tk371x_field_range_t), "_bcm_tk371x_field_range_t");
    fr->flags    = flags;
    fr->rid      = range;
    fr->eid		 = _FP_INVALID_INDEX;
    fr->min      = min;
    fr->max      = max;
    fr->tk_match = NULL;
    fr->next = fc->ranges;
    fc->ranges = fr;
    return BCM_E_NONE;
}

/*
 *   Function
 *      bcm_tk371x_field_range_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_create_id(
	    int unit,
	    bcm_field_range_t range,
	    uint32 flags,
	    bcm_l4_port_t min,
	    bcm_l4_port_t max)
{
    uint8 			min_cos, max_cos;
    uint16 			min_vid, max_vid;

    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "BEGIN bcm_tk371x_field_range_create_id(unit=%d, range=%d, "),
               unit,
               range));
    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "flags=0x%08x, min=0x%x, max=0x%x)\n"),
               flags, min, max));
    /* Range type sanity check. */
    BCM_IF_ERROR_RETURN(_bcm_tk371x_field_range_flags_check(unit, flags));

    if (flags & BCM_FIELD_RANGE_SRCPORT) {
        if (min < 0 || min > max){
        	return BCM_E_PARAM;
        }
    	if (max > _BCM_TK371X_FP_PORT_VALUE_MAX){
    		return BCM_E_PARAM;
    	}
    	return _bcm_tk371x_field_range_item_insert(unit, range, flags, min, max);
    }else if (flags & BCM_FIELD_RANGE_DSTPORT) {
        if (min < 0 || min > max){
        	return BCM_E_PARAM;
        }
    	if (max > _BCM_TK371X_FP_PORT_VALUE_MAX){
    		return BCM_E_PARAM;
    	}
    	return _bcm_tk371x_field_range_item_insert(unit, range, flags, min, max);
    }else if (flags & BCM_FIELD_RANGE_OUTER_VLAN) {
    	if (0 == (uint16)min && 0 == (uint16)max){
    		return BCM_E_PARAM;
    	}
    	_bcm_tk371x_vlan_cos_val_get((uint16)min, &min_cos);
    	_bcm_tk371x_vlan_cos_val_get((uint16)max, &max_cos);
    	if (min_cos > max_cos){
    		return BCM_E_PARAM;
    	}
    	_bcm_tk371x_vlan_vid_val_get((uint16)min, &min_vid);
    	_bcm_tk371x_vlan_vid_val_get((uint16)max, &max_vid);
    	if (min_vid > max_vid){
    		return BCM_E_PARAM;
    	}
    	if (min_cos == 0 && max_cos == 0){
        	return _bcm_tk371x_field_range_item_insert(unit, range, flags, min, max);
    	}else if (min_vid == 0 && max_vid ==0){
        	return _bcm_tk371x_field_range_item_insert(unit, range, flags, min, max);
    	}else{
    		return BCM_E_PARAM;
    	}
    }else{
    	return BCM_E_PARAM;
    }
}

/*
 *   Function
 *      bcm_tk371x_field_range_destroy
 *   Purpose
 *      Destroy a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_destroy(
	    int unit,
	    bcm_field_range_t range)
{
    _bcm_tk371x_field_control_t    *fc = _field_control;
    _bcm_tk371x_field_range_t      *fr,
    							   *fr_prev = NULL; /* the prior ranges*/
    _bcm_tk371x_field_tk_match_t   *tk_match, *tk_match_prev = NULL;


    LOG_DEBUG(BSL_LS_BCM_FP,
              (BSL_META_U(unit,
                          "bcm_tk371x_field_range_destroy(%d,%d)\n"),
               unit, range));
    if (fc == NULL){
    	return BCM_E_INIT;
    }
    fr = NULL;
    tk_match = NULL;
    fr = fc->ranges;
    while (fr != NULL){
    	if (fr->rid == range){
			/* destory all matchs */
    		tk_match_prev = tk_match = fr->tk_match;
    		while (tk_match != NULL){
				tk_match = tk_match->next;
				sal_free(tk_match_prev);
				tk_match_prev = tk_match;
			}

    		/*destroy this range*/
    	    if (fr_prev == NULL) { /* Group is at head of list. */
    	        fc->ranges = fr->next;
    	    } else {
    	        fr_prev->next = fr->next;
    	    }
    	    sal_free(fr);
			return BCM_E_NONE;
    	}
    	fr_prev = fr;
    	fr = fr->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_range_get
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 *flags = where to put the flags for the range
 *      (in) bcm_l4_port_t *min = where to put range's low port number
 *      (in) bcm_l4_port_t *max = where to put range's high port number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_get(
	    int unit,
	    bcm_field_range_t range,
	    uint32 *flags,
	    bcm_l4_port_t *min,
	    bcm_l4_port_t *max)
{
	_bcm_tk371x_field_control_t    *fc = _field_control;
	_bcm_tk371x_field_range_t      *fr = NULL;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_range_get(%d,%d)\n"),
	           unit, range));
    if (fc == NULL){
    	return BCM_E_INIT;
    }
    fr = fc->ranges;
    while (fr != NULL){
    	if (fr->rid == range){
    		*flags = fr->flags;
    		*min = fr->min;
    		*max = fr->max;
    		return BCM_E_NONE;
    	}
    	fr = fr->next;
    }
	return BCM_E_NOT_FOUND;
}

/*
 *   Function
 *      bcm_tk371x_field_show
 *   Purpose
 *      Dump all field information for the unit
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_show(
	    int unit,
	    const char *pfx)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_group_t		*fg = NULL;
	_bcm_tk371x_field_range_t		*fr = NULL;
	_bcm_tk371x_field_entry_t		*fe = NULL;
	_bcm_tk371x_field_tk_match_t	*tk_match = NULL;
	int e_count = 0;
	int g_count = 0;
	int r_count = 0;
	int m_count = 0;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_show(unit=%d)\n"),
	           unit));
    if (fc == NULL){
    	return BCM_E_INIT;
    }
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
    fg = fc->groups;
    while (fg != NULL){
		LOG_CLI(("pfx=%s\n", pfx));
		LOG_CLI(("group id=%d\n", fg->group_id));
    	fe = fg->entry;
    	e_count = 0;
    	while (fe != NULL){
    		bcm_tk371x_field_entry_dump(unit, fe->entry_id);
    		fe = fe->next;
    		e_count++;
    	}
    	LOG_CLI(("group %d entries are %d\n", fg->group_id, e_count));
    	fg = fg->next;
    	g_count++;
    }
    LOG_CLI(("group numbers are  %d\n", g_count));
    LOG_CLI(("Show Ranges:\n"));
    fr = fc->ranges;
    while (fr != NULL){
    	LOG_CLI(("Range ID=%d\n", fr->rid));
    	LOG_CLI(("Flags=%08x\n", fr->flags));
    	LOG_CLI(("min=%d\n", fr->min));
    	LOG_CLI(("max=%d\n", fr->max));
    	LOG_CLI(("TK Matches:\n"));
    	tk_match = fr->tk_match;
    	m_count = 0;
    	while (tk_match != NULL){
    		LOG_CLI(("match field=%d\n", tk_match->field));
    		LOG_CLI(("match value=%d\n", tk_match->value));
    		LOG_CLI(("match = %d\n", tk_match->match));
    		tk_match = tk_match->next;
    		m_count++;
    	}
    	LOG_CLI(("match number are %d\n", m_count));
    	fr = fr->next;
    	r_count++;
    }
    LOG_CLI(("Range numbers are %d\n", r_count));

    return BCM_E_NONE;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIpEqualDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIpEqualDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIpEqualDstIp_get(
		int unit,
		bcm_field_entry_t field_entry,
		uint32 *flags)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_qualify_SrcIpEqualDstIp_get(unit=%d)\n"),
	           unit));
	return BCM_E_UNAVAIL;
}

int
bcm_tk371x_field_qualify_SrcIpEqualDstIp(
		int unit,
		bcm_field_entry_t field_entry,
		uint32 flags)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_qualify_SrcIpEqualDstIp(unit=%d)\n"),
	           unit));
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_create_mode_id
 *   Purpose
 *      Create a new group using the specified ID, spanning the specified
 *      width, that has the specified qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_tk371x_field_group_create_mode_id(
		int unit,
		bcm_field_qset_t field_qset,
		int flags,
		bcm_field_group_mode_t mode,
		bcm_field_group_t field_group)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_group_create_mode_id(unit=%d)\n"),
	           unit));
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_create_mode
 *   Purpose
 *      Create a new group spanning the specified width, that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_tk371x_field_group_create_mode(
		int unit,
		bcm_field_qset_t field_qset,
		int flags,
		bcm_field_group_mode_t mode,
		bcm_field_group_t *group)
{
	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_group_create_mode(unit=%d)\n"),
	           unit));
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp_get(
		int unit,
		bcm_field_entry_t field_entry,
		bcm_ip_t *srcip,
		bcm_ip_t *dstip)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t	*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifySrcIp;
	int ret_val;
	int len = sizeof(bcm_ip_t);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_qualify_SrcIp_get(unit=%d)\n"),
	           unit));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, field_entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy((uint8*)srcip,
					&(fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len]), len);
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
					(uint8*)srcip, (uint8*)dstip, len);
			if (ret_val != BCM_E_NONE){
				sal_memset(dstip, 0, sizeof(bcm_ip_t));
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}



int
bcm_tk371x_field_qualify_VlanFormat(int unit,
		bcm_field_entry_t eid, uint8 data,uint8 mask)
{
	int qual_id = bcmFieldQualifyVlanFormat;
	int qset = _bcmTk371xFieldEntryCondQsetVlanId;
	int len = sizeof(uint8);
	uint8 match;
	uint8 real_data;
	uint8 cond;
	uint16 vlanid;
	int rv1, rv2;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_VlanFormat(unit=%d,entry=%08X,data=%02X,mask=%02X)\n"),
	           unit,
	           eid,
	           data,
	           mask));
	if (mask != 0xff){
		return BCM_E_PARAM;
	}
	cond = data & mask;
	switch (cond){
		case BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED:
			len = sizeof(uint8);
			qual_id = bcmFieldQualifyVlanFormat;
			qset = _bcmTk371xFieldEntryCondQsetVlanId;
			match = _BCM_TK371X_FIELD_MATCH_EXISTS;
			real_data = 1;
			rv1 = _bcm_tk371x_field_entry_qualify_update(unit,
					eid, qual_id, qset, match, (uint8*)&real_data, len);
			qual_id = bcmFieldQualifyInnerVlanId;
			match = _BCM_TK371X_FIELD_MATCH_NOTEQUAL;
			vlanid = 0;
			len = sizeof(uint16);
			rv2 = _bcm_tk371x_field_entry_qualify_update(unit, eid, qual_id,
					qset, match, (uint8*)&vlanid, len);			
			if (rv1 != BCM_E_NONE){
				return rv1;
			}
			if (rv2 != BCM_E_NONE){
				return rv2;
			}
			return BCM_E_NONE;
		case BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO:				
			len = sizeof(uint8);
			qual_id = bcmFieldQualifyVlanFormat;
			qset = _bcmTk371xFieldEntryCondQsetVlanId;
			match = _BCM_TK371X_FIELD_MATCH_EXISTS;
			real_data = 1;
			rv1 = _bcm_tk371x_field_entry_qualify_update(unit,
					eid, qual_id, qset, match, (uint8*)&real_data, len);
			match = _BCM_TK371X_FIELD_MATCH_EQUAL;
				
			qual_id = bcmFieldQualifyInnerVlanId;
			vlanid = 0;
			len = sizeof(uint16);
			rv2 = _bcm_tk371x_field_entry_qualify_update(unit, eid, qual_id,
					qset, match, (uint8*)&vlanid, len);			
			if (rv1 != BCM_E_NONE){
				return rv1;
			}
			if (rv2 != BCM_E_NONE){
				return rv2;
			}
			return BCM_E_NONE;
		case (BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
				BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO):
			len = sizeof(uint8);
			qual_id = bcmFieldQualifyVlanFormat;
			qset = _bcmTk371xFieldEntryCondQsetVlanId;
			real_data = 1;
			match = _BCM_TK371X_FIELD_MATCH_EXISTS;	
			return _bcm_tk371x_field_entry_qualify_update(unit,
					eid, qual_id, qset, match, (uint8*)&real_data, len);
		default:			
			len = sizeof(uint8);
			qual_id = bcmFieldQualifyVlanFormat;
			qset = _bcmTk371xFieldEntryCondQsetVlanId;
			real_data = 0;
			match = _BCM_TK371X_FIELD_MATCH_NOTEXISTS;
			return _bcm_tk371x_field_entry_qualify_update(unit,
					eid, qual_id, qset, match, (uint8*)&real_data, len);
	}

}

/*
 * Function:
 *      bcm_tk371x_field_qualify_VlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_VlanFormat_get(int unit,
		bcm_field_entry_t eid, uint8 *data,uint8 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t		*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qset = 0;
	int dflag, vflag;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "FP: bcm_tk371x_field_qualify_VlanFormat_get(unit=%d)\n"),
	           unit));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, eid, &fe));
	fe_cond = fe->conds;
	dflag = vflag = 0;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == bcmFieldQualifyVlanFormat){
			vflag = 1;
		}
		if (fe_cond->qual_id == bcmFieldQualifyInnerVlanId){
			qset = fe_cond->match;
			dflag = 1;
		}		
		fe_cond = fe_cond->next;
	}
	if (vflag){
		*mask = 0xff;
		if (dflag){
			if (qset == _BCM_TK371X_FIELD_MATCH_EQUAL){
				*data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;
			}else if (qset == _BCM_TK371X_FIELD_MATCH_NOTEQUAL){
				*data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED;
			}else{
				return BCM_E_FAIL;
			}			
		}else{
			*data = BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED |
				BCM_FIELD_VLAN_FORMAT_OUTER_TAGGED_VID_ZERO;	
		}
		return BCM_E_NONE;
	}else{	
		return BCM_E_NOT_FOUND;
	}
}

/*
* Function:
*      bcm_tk371x_field_qualify_LlidValue
* Purpose:
*      Get match criteria for bcmFieldQualifyLlidValue
*                     qualifier from the field entry.
* Parameters:
*      unit - (IN) Unit number.
*      entry - (IN) BCM field entry id.
*      data - (OUT) Qualifier match data.
*      mask - (OUT) Qualifier match mask.
* Returns:
*      BCM_E_XXX
* Notes:
*/
int
bcm_tk371x_field_qualify_LlidValue(int unit,
		bcm_field_entry_t entry, uint16 data, uint16 mask)
{
	int qual_id = bcmFieldQualifyLlidValue;
	int qset = _bcmTk371xFieldEntryCondQsetUser3;
	int len = sizeof(uint16);
	uint8 match;

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_LlidValue"
	                       "(unit=%d,entry=%08X,data=%04X,mask=%04X)\n"),
	           unit,
	           entry,
	           data,
	           mask));

	match = _bcm_ea_field_operator_cond((uint8*)&data, (uint8*)&mask, len);
	if (match == (uint8)_BCM_TK371X_FIELD_MATCH_ERROR) {
		return BCM_E_PARAM;
	}
	return _bcm_tk371x_field_entry_qualify_update(unit,
			entry, qual_id, qset, match, (uint8*)&data, len);
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_LlidValue_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyLlidValue
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_LlidValue_get(int unit,
		bcm_field_entry_t entry, uint16 *data, uint16 *mask)
{
	_bcm_tk371x_field_control_t 	*fc = _field_control;
	_bcm_tk371x_field_entry_t		*fe = NULL;
	_bcm_tk371x_field_entry_cond_t	*fe_cond = NULL;
	int qual_id = bcmFieldQualifyLlidValue;
	int ret_val;
	int len = sizeof(uint16);

	LOG_DEBUG(BSL_LS_BCM_FP,
	          (BSL_META_U(unit,
	                      "bcm_tk371x_field_qualify_DstIp(unit=%d,entry=%08X)\n"),
	           unit, entry));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_control_get(unit, fc));
	BCM_IF_ERROR_RETURN(_bcm_tk371x_field_entry_get(unit, entry, &fe));
	fe_cond = fe->conds;
	while (fe_cond != NULL){
		if (fe_cond->qual_id == qual_id){
			sal_memcpy(data,
					&fe_cond->common.value[MAX_ENTRY_RULE_VAL_LEN - len],
					sizeof(uint16));
			ret_val = _bcm_ea_field_operator_cond_mask_get(fe_cond->match,
								(uint8*)data, (uint8*)mask, len);
			if (ret_val != BCM_E_NONE){
				*mask = 0;
			}
			return BCM_E_NONE;
		}
		fe_cond = fe_cond->next;
	}
	return BCM_E_NOT_FOUND;
}
#else
/*
 * Function: bcm_tk371x_field_action_add
 *
 * Purpose:
 *     Add action performed when entry rule is matched for a packet
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - entry ID
 *     action - Action to perform (bcmFieldActionXXX)
 *     param0 - Action parameter (use 0 if not required)
 *     param1 - Action parameter (use 0 if not required)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_MEMORY    - Allocation failure
 *     BCM_E_RESOURCE  - Conflicting actions exist in entry.
 *     BCM_E_RESOURCE  - Counter not previously created for entry.
 *     BCM_E_PARAM     - param0 or param1 out of range for action
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_add(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action,
	    uint32 param0,
	    uint32 param1)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_action_delete
 *
 * Purpose:
 *     Delete an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Entry ID
 *     action - Action to remove (bcmFieldActionXXX)
 *     param0 - Action parameter (use 0 if not required)
 *     param1 - Action parameter (use 0 if not required)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_delete(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action,
	    uint32 param0,
	    uint32 param1)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_action_get
 *
 * Purpose:
 *     Get parameters associated with an entry action
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *     action - Action to perform (bcmFieldActionXXX)
 *     param0 - (OUT) Action parameter
 *     param1 - (OUT) Action parameter
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - paramX is NULL
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action,
	    uint32 *param0,
	    uint32 *param1)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_action_remove
 *
 * Purpose:
 *     Remove an action performed when entry rule is matched for a packet.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Entry ID
 *     action - Action to remove (bcmFieldActionXXX)
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NOT_FOUND - No matching Action for entry
 *     BCM_E_PARAM     - Action out of valid range.
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_remove(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_action_t action)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_field_action_remove_all
 *
 * Purpose:
 *     Remove all actions from an entry rule.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_action_remove_all(
	    int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_detach
 *
 * Purpose:
 *     Free resources associated with field module
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_INIT - BCM Unit not initialized.
 *     BCM_E_XXX  - Error code from bcm_field_group_destroy() or
 *                  bcm_field_entry_destroy_all().
 *     BCM_E_NONE - Success
 */
int
bcm_tk371x_field_detach(int unit)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_copy
 *
 * Purpose:
 *     Create a copy of an existing entry. The new entry will be a member of
 *     the same group as the source entry.
 *
 * Parameters:
 *     unit      - BCM device number
 *     src_entry - Entry to copy
 *     dst_entry - (OUT) New entry
 *
 * Returns:
 *     BCM_E_INIT        BCM Unit not initialized
 *     BCM_E_NOT_FOUND   Source entry not found
 *     BCM_E_INTERNAL    No group exists for source entry ID.
 *     BCM_E_PARAM       dst_entry pointing to NULL
 *     BCM_E_RESOURCE    No destination entry available
 *     BCM_E_XXX         Error from bcm_field_entry_copy_id()
 */
int
bcm_tk371x_field_entry_copy(
	    int unit,
	    bcm_field_entry_t src_entry,
	    bcm_field_entry_t *dst_entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_copy_id
 *
 * Purpose:
 *     Create a copy of an existing entry with a requested ID
 *
 * Parameters:
 *     unit      - BCM device number
 *     src_entry - Source entry to copy
 *     dst_entry - Destination entry for copy
 *
 * Returns:
 *     BCM_E_INIT      - BCM Unit not initialized
 *     BCM_E_NOT_FOUND - Source Entry ID not found
 *     BCM_E_XXX       - Error code from bcm_field_entry_create_id()
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_copy_id(
	    int unit,
	    bcm_field_entry_t src_entry,
	    bcm_field_entry_t dst_entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_create
 *
 * Purpose:
 *     Create a blank entry based on a group. Automatically generate an entry
 *     ID.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group ID
 *     entry - (OUT) New entry
 *
 * Returns:
 *     BCM_E_INIT        BCM unit not initialized
 *     BCM_E_NOT_FOUND   group not found in unit
 *     BCM_E_PARAM       *entry was NULL
 *     BCM_E_RESOURCE    No unused entries available
 *     BCM_E_XXX         Error from bcm_field_entry_create_id
 *
 * See Also:
 * bcm_field_entry_create_id
 */
int
bcm_tk371x_field_entry_create(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_entry_t *entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_create_id
 *
 * Purpose:
 *     Create a blank entry group based on a group;
 *     allows selection of a specific slot in a slice
 *
 * Parameters:
 *     unit - BCM device number
 *     group - Field group ID
 *     entry - Requested entry ID; must be in the range prio_min through
 *             prio_max as returned by bcm_field_group_status_get().
 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_EXISTS    - Entry ID already in use
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_MEMORY    - allocation failure
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_create_id(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_destroy
 *
 * Purpose:
 *     Destroy an entry.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found
 *     BCM_E_XXX       - From bcm_field_counter_destroy() or
 *                       bcm_field_meter_destroy()
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_destroy(
	    int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_destroy_all
 *
 * Purpose:
 *     Destroy all entries on a unit. It iterates over all slices in a unit.
 *     For each slice, If entries exist, it calls bcm_field_entry_destroy()
 *     using the Entry ID.
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_XXX       - Error from bcm_tk371x_field_entry_destroy()
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_destroy_all(int unit)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_dump
 *
 * Purpose:
 *     Show contents of a field entry.
 *
 * Parameters:
 *     unit  - BCM device number
 *     entry - Field entry to dump
 *
 * Returns:
 *     Nothing.
 */
int
bcm_tk371x_field_entry_dump(
	    int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_install
 *
 * Purpose:
 *     Install a entry into the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to install
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit.
 *     BCM_E_XXX       - Error from _field_XX_tcam_policy_install()
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     Qualifications should be made and actions should be added
 *     prior to installing the entry.
 */
int
bcm_tk371x_field_entry_install(
	    int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_multi_get
 *
 * Purpose:
 *     Gets an array of a group's entry IDs
 *
 * Parameters:
 *     unit -  (IN) BCM device number.
 *     group - (IN) Field group ID.
 *     entry_size - (IN) Maximum number of entries to return.  Set to 0
 *                       to get the number of entries available
 *     entry_array - (OUT) Pointer to a buffer to fill with the array of
 *                         entry IDs.  Ignored if entry_size is 0
 *     entry_count - (OUT) Returns the number of entries returned in the
 *                         array, or if entry_size was 0, the number of
 *                         entries available

 * Returns:
 *     BCM_E_INIT      - unit not initialized
 *     BCM_E_NOT_FOUND - Group ID not found in unit
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_entry_multi_get(
	    int unit,
	    bcm_field_group_t group,
	    int entry_size,
	    bcm_field_entry_t *entry_array,
	    int *entry_count)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_prio_get
 *
 * Purpose:
 *     Gets the priority within the group of the entry.
 *
 * Parameters:
 *     unit   - BCM device number
 *     entry  - Field entry to operate on
 *     prio   - (OUT) priority of entry
 *
 * Returns:
 *     BCM_E_NONE       - Success
 *     BCM_E_PARAM      - prio pointing to NULL
 *     BCM_E_NOT_FOUND  - Entry ID not found on unit
 */
int
bcm_tk371x_field_entry_prio_get(
	    int unit,
	    bcm_field_entry_t entry,
	    int *prio)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_prio_set
 *
 * Purpose:
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *
 * Returns:
 *     BCM_E_NONE       Success
 */
int
bcm_tk371x_field_entry_prio_set(
	    int unit,
	    bcm_field_entry_t entry,
	    int prio)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_reinstall
 *
 * Purpose:
 *     Re-install a entry into the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to install
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit
 *     BCM_E_UNAVAIL   - Feature not implemented.
 *     BCM_E_XXX       - From _bcm_XXX_install() calls
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     Reinstallation may only be used to change the actions for
 *     an installed entry without having to remove and re-add the
 *     entry.
 */
int
bcm_tk371x_field_entry_reinstall(
	    int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_entry_remove
 *
 * Purpose:
 *     Remove an entry from the hardware tables.
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Entry to remove
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_NOT_FOUND - Entry ID not found on unit
 *     BCM_E_XXX
 *     BCM_E_NONE      - Success
 *
 * Notes:
 *     This does not destroy the entry; it uninstalls it from
 *     any hardware tables.
 *     Destroy a entry using bcm_field_entry_destroy.
 */
int
bcm_tk371x_field_entry_remove(
	    int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_group_create
 *
 * Purpose:
 *     Create a field group based on the field group selector flags.
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *     group - (OUT) New field group ID
 *
 * Returns:
 *     BCM_E_INIT     - BCM unit not initialized
 *     BCM_E_XXX      - Error code from _field_group_prio_make()
 *     BCM_E_PARAM    - pri out of range (0-15 for FB & ER) or group == NULL
 *     BCM_E_RESOURCE - no select codes will satisfy qualifier set
 *     BCM_E_NONE     - Success
 *
 * Notes:
 *      Allocates a hardware slice at the requested priority or better.
 *      Higher numerical value for priority has better priority for
 *      conflict resolution when there is a search hit on multiple slices.
 */
int
bcm_tk371x_field_group_create(
	    int unit,
	    bcm_field_qset_t qset,
	    int pri,
	    bcm_field_group_t *group)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_group_create_id
 *
 * Purpose:
 *     Create a field group based on the field group selector flags
 *     with a requested ID
 *
 * Parameters:
 *     unit - BCM device number.
 *     port - Port number
 *     qset - Field qualifier set
 *     pri  - Priority within allowable range (see bcm_field_status_get),
 *            or BCM_FIELD_GROUP_PRIO_ANY to automatically assign a
 *            priority; each priority value may be used only once
 *    group - Requested field group ID
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized
 *     BCM_E_RESOURCE  - No unused group/slices left
 *     BCM_E_PARAM     - priority out of range (0-15 for FB & ER)
 *     BCM_E_EXISTS    - group with that id already exists on this unit.
 *     BCM_E_MEMORY    - Group memory allocation failure
 *     BCM_E_XXX       - Error code from _field_group_prio_make().
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_group_create_id(
	    int unit,
	    bcm_field_qset_t qset,
	    int pri,
	    bcm_field_group_t group)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_create_mode
 *   Purpose
 *      Create a new group spanning the specified width, that has the specified
 *      qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (out) bcm_field_group_t *group = where to put the group ID
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_tk371x_field_group_create_mode(
		int unit,
		bcm_field_qset_t field_qset,
		int flags,
		bcm_field_group_mode_t mode,
		bcm_field_group_t *group)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_create_mode_id
 *   Purpose
 *      Create a new group using the specified ID, spanning the specified
 *      width, that has the specified qualifier set and priority.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_qset_t qset = the qualifier set
 *      (in) int pri = the priority
 *      (in) bcm_mode_t mode = the mode (width) of the group
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Allocates first available group ID.
 *      Can not specify a priority already taken by an existing group.
 *      Can not specify a qualifier that another group in the same stage has.
 *      If no stage qualifier, it is assumed to be bcmFieldQualifyStageIngress.
 *      Not supported on SBX?
 */
int
bcm_tk371x_field_group_create_mode_id(
		int unit,
		bcm_field_qset_t field_qset,
		int flags,
		bcm_field_group_mode_t mode,
		bcm_field_group_t field_group)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_destroy
 *   Purpose
 *      Destroys a group.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      There must be no entries in this group when calling this function.
 */
int
bcm_tk371x_field_group_destroy(
	    int unit,
	    bcm_field_group_t group)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_group_dump
 *
 * Purpose:
 *     Show contents of a field group.
 *
 * Parameters:
 *     unit  - BCM device number
 *     group - Field group to dump
 *
 * Returns:
 *     Nothing.
 */
int
bcm_tk371x_field_group_dump(
	    int unit,
	    bcm_field_group_t group)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_group_get
 *
 * Purpose:
 *     Get the qualifier set for a previously created field group
 *
 * Parameters:
 *     unit  - BCM device number
 *     port  - Port number
 *     group - Field group ID
 *     qset  - (OUT) Field qualifier set
 *
 * Returns:
 *     BCM_E_INIT      - BCM unit not initialized.
 *     BCM_E_NOT_FOUND - Group ID not found in this unit
 *     BCM_E_PARAM     - qset is NULL
 *     BCM_E_NONE      - Success
 */
int
bcm_tk371x_field_group_get(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_qset_t *qset)
{
	return BCM_E_UNAVAIL;
}

/*
 *Function
 *	   bcm_tk371x_field_group_install
 * Purpose:
 *	  Install all of a group's entries into the hardware tables.
 *
 * Parameters:
 *	  unit - BCM device number
 *	  group - Group to install
 *
 * Returns:
 *    BCM_E_NONE	  - Success
 *	  BCM_E_*		  - Error
 *	
 */

int 
bcm_tk371x_field_group_install(
    int unit, 
    bcm_field_group_t group)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_remove
 *   Purpose
 *      Remove all of a group's entries from the hardware, but do not remove
 *      the entries from the software table.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      This removes the group's entries from the hardware, marking them so,
 *      and commits the changes to the hardware.
 */
int
bcm_tk371x_field_group_remove(
	    int unit,
	    bcm_field_group_t group)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_set
 *   Purpose
 *      This changes the group's qualifier set so it is the specified set.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (in) bcm_field_qset_t qset = new qset
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      If there are any entries, all of them must be representable using the
 *      new qualifier set (if not, this fails), plus the new qualifier set can
 *      not change the required pattern type or stage (it will also fail in
 *      these cases).
 *      Updates are always permitted if there are no entries present.
 */
int
bcm_tk371x_field_group_set(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_qset_t qset)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_group_status_get
 *   Purpose
 *      Gets the group's status.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_group_t group = which group ID to use
 *      (out) bcm_field_group_status_t *status = where to put the status
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_group_status_get(
	    int unit,
	    bcm_field_group_t group,
	    bcm_field_group_status_t *status)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_group_traverse
 * Purpose:
 *      Traverse all the fp groups in the system, calling a specified
 *      callback for each one
 * Parameters:
 *      unit - (IN) Unit number.
 *      callback - (IN) A pointer to the callback function to call for each fp group
 *      user_data - (IN) Pointer to user data to supply in the callback
 * Returns:
 *      BCM_E_xxx
 * Notes:
 */
int
bcm_tk371x_field_group_traverse(
	    int unit,
	    bcm_field_group_traverse_cb callback,
	    void *user_data)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_init
 *
 * Purpose:
 *     Initialize field module.
 *
 * Parameters:
 *     unit - BCM device number
 *
 * Returns:
 *     BCM_E_UNIT    - Invalid BCM unit number.
 *     BCM_E_UNAVAIL - Field Processor not on device.
 *     BCM_E_MEMORY  - Allocation failure
 *     BCM_E_XXX     - Error code from bcm_XX_field_init()
 *     BCM_E_NONE    - Success
 */
int
bcm_tk371x_field_init(int unit)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualifier_delete
 * Purpose:
 *      Remove match criteria from a field processor entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      qual_id - (IN) BCM field qualifier id.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualifier_delete(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_qualify_t qual_id)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DSCP
 *   Purpose
 *      Set expected IPv4 DSCP for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint8 data = which DSCP
 *      (in) uint8 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_qualify_DSCP(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DSCP_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDSCP
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DSCP_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstIp
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_DstIp(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip_t data,
	    bcm_ip_t mask)

{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstIp6
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv6 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_DstIp6(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstIp6High
 *   Purpose
 *      Set expected destination IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which destination IPv6 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_DstIp6High(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6High_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6Low(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6Low_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp6_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstIp_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip_t *data,
	    bcm_ip_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_DstMac
 *   Purpose
 *      Set expected destination MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which destination MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the destination MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_DstMac(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t data,
	    bcm_mac_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_DstMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyDstMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_DstMac_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t *data,
	    bcm_mac_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_EtherType
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_EtherType(
	    int unit,
	    bcm_field_entry_t entry,
	    uint16 data,
	    uint16 mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_EtherType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyEtherType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_EtherType_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint16 *data,
	    uint16 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_InPort
 *   Purpose
 *      Set allowed ingress port for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_port_t data = allowed port
 *      (in) bcm_port_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Supports GPORTs of various types and will map back to phys port.
 */

int
bcm_tk371x_field_qualify_InPort(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_port_t data,
	    bcm_port_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_InPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_InPort_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_port_t *data,
	    bcm_port_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_InPorts
 *   Purpose
 *      Set allowed ingress ports for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_pbmp_t data = allowed ports
 *      (in) bcm_pbmp_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_qualify_InPorts(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_pbmp_t data,
	    bcm_pbmp_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_InPorts_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyInPorts
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_InPorts_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_pbmp_t *data,
	    bcm_pbmp_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6FlowLabel
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6FlowLabel(
	    int unit,
	    bcm_field_entry_t entry,
	    uint32 data,
	    uint32 mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_Ip6FlowLabel_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIp6FlowLabel
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_Ip6FlowLabel_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint32 *data,
	    uint32 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_IpProtocol
 *   Purpose
 *      Set expected IPv4 protocol type type for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) uint16 data = which ethertype
 *      (in) uint16 mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the ethernet type to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_IpProtocol(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	return BCM_E_UNAVAIL;
}

int
bcm_tk371x_field_qualify_IpProtocolCommon(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpProtocolCommon_t protocol)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpProtocolCommon_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocolCommon
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      protocol - (OUT) Qualifier protocol encoding.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpProtocolCommon_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpProtocolCommon_t *protocol)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpProtocol_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpProtocol
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpProtocol_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpType
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpType(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpType_t type)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_IpType_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyIpType
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      type - (OUT) Qualifier match ip type.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_IpType_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_IpType_t *type)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_L4DstPort
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_L4DstPort(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t data,
	    bcm_l4_port_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_L4DstPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4DstPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_L4DstPort_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t *data,
	    bcm_l4_port_t *mask)
{
	return BCM_E_UNAVAIL;
}

int
bcm_tk371x_field_qualify_L4SrcPort(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t data,
	    bcm_l4_port_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_L4SrcPort_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyL4SrcPort
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_L4SrcPort_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_l4_port_t *data,
	    bcm_l4_port_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
* Function:
*      bcm_tk371x_field_qualify_LlidValue
* Purpose:
*      Get match criteria for bcmFieldQualifyLlidValue
*                     qualifier from the field entry.
* Parameters:
*      unit - (IN) Unit number.
*      entry - (IN) BCM field entry id.
*      data - (OUT) Qualifier match data.
*      mask - (OUT) Qualifier match mask.
* Returns:
*      BCM_E_XXX
* Notes:
*/
int
bcm_tk371x_field_qualify_LlidValue(int unit,
		bcm_field_entry_t entry, 
		uint16 data, uint16 mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_LlidValue_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyLlidValue
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_LlidValue_get(int unit,
		bcm_field_entry_t entry, uint16 *data, uint16 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_OuterVlanId
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which VID (12 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_OuterVlanId(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_vlan_t data,
	    bcm_vlan_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_OuterVlanId_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanId
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match data.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_OuterVlanId_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_vlan_t *data,
	    bcm_vlan_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_OuterVlanPri
 *   Purpose
 *      Set expected outer VLAN for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_vlan_t data = which Pri (3 bits)
 *      (in) bcm_vlan_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the outer VLAN ID to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_OuterVlanPri(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 data,
	    uint8 mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_OuterVlanPri_get
 * Purpose:
 *       Get match criteria for bcmFieildQualifyOuterVlanPri
 *                       qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_OuterVlanPri_get(
	    int unit,
	    bcm_field_entry_t entry,
	    uint8 *data,
	    uint8 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_RangeCheck
 *   Purpose
 *      Set expected TCP/UDP port range for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_field_range_t range = which ethertype
 *      (in) int invert = whether the range match is to be inverted
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      The range that is specified is read only during this call; it will not
 *      be read later if that range changes; another call to this function will
 *      be required should the range change and the update need to apply.
 *      The invert flag is not supported.
 *      This can't use the helper functions because it is setting a more
 *      complex set of fields under a more complex set of conditions.
 */
int
bcm_tk371x_field_qualify_RangeCheck(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_field_range_t range,
	    int invert)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_RangeCheck_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifyRangeCheck
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      max_count - (IN) Max entries to fill.
 *      range - (OUT) Range checkers array.
 *      invert - (OUT) Range checkers invert array.
 *      count - (OUT) Number of filled range checkers.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_RangeCheck_get(
	    int unit,
	    bcm_field_entry_t entry,
	    int max_count,
	    bcm_field_range_t *range,
	    int *invert,
	    int *count)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcIp
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip_t data = which source IPv4 address
 *      (in) bcm_ip_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_SrcIp(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip_t data,
	    bcm_ip_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcIp6
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_SrcIp6(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcIp6High
 *   Purpose
 *      Set expected source IPv4 address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_ip6_t data = which source IPv6 address
 *      (in) bcm_ip6_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be prefix style mask.
 */
int
bcm_tk371x_field_qualify_SrcIp6High(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6High_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6High
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6High_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6Low
 * Purpose:
 *      Set match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier in the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6Low(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t data,
	    bcm_ip6_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6Low_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6Low
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6Low_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp6_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp6
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp6_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_ip6_t *data,
	    bcm_ip6_t *mask)
{
	return BCM_E_UNAVAIL;
}

int
bcm_tk371x_field_qualify_SrcIpEqualDstIp(
		int unit,
		bcm_field_entry_t field_entry,
		uint32 flags)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIpEqualDstIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIpEqualDstIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      flag - (OUT) Qualifier match flags.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIpEqualDstIp_get(
		int unit,
		bcm_field_entry_t field_entry,
		uint32 *flags)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcIp_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcIp
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcIp_get(
		int unit,
		bcm_field_entry_t field_entry,
		bcm_ip_t *srcip,
		bcm_ip_t *dstip)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_qualify_SrcMac
 *   Purpose
 *      Set expected source MAC address for this entry
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_entry_t entry = the entry ID
 *      (in) bcm_mac_t data = which source MAC address
 *      (in) bcm_mac_t mask = which bits of data are significant
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Mask must be either zero or all ones, else BCM_E_PARAM, since SBX
 *      doesn't allow the source MAC address to be masked (all are always
 *      significant if any are significant).
 */
int
bcm_tk371x_field_qualify_SrcMac(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t data,
	    bcm_mac_t mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_SrcMac_get
 * Purpose:
 *      Get match criteria for bcmFieildQualifySrcMac
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_SrcMac_get(
	    int unit,
	    bcm_field_entry_t entry,
	    bcm_mac_t *data,
	    bcm_mac_t *mask)
{
	return BCM_E_UNAVAIL;
}

int
bcm_tk371x_field_qualify_VlanFormat(int unit,
		bcm_field_entry_t eid, uint8 data,uint8 mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_tk371x_field_qualify_VlanFormat_get
 * Purpose:
 *      Get match criteria for bcmFieldQualifyVlanFormat
 *                     qualifier from the field entry.
 * Parameters:
 *      unit - (IN) Unit number.
 *      entry - (IN) BCM field entry id.
 *      data - (OUT) Qualifier match data.
 *      mask - (OUT) Qualifier match mask.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 */
int
bcm_tk371x_field_qualify_VlanFormat_get(int unit,
		bcm_field_entry_t eid, uint8 *data,uint8 *mask)
{
	return BCM_E_UNAVAIL;
}

/*
 * Function: bcm_tk371x_field_qualify_clear
 *
 * Purpose:
 *     Remove all field qualifications from a filter entry
 *
 * Parameters:
 *     unit - BCM device number
 *     entry - Field entry to operate on
 *
 * Returns:
 *     BCM_E_INIT       BCM Unit not initialized.
 *     BCM_E_NOT_FOUND  Entry ID not found in unit.
 *     BCM_E_NONE       Success
 */
int
bcm_tk371x_field_qualify_clear(
		int unit,
	    bcm_field_entry_t entry)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_range_create
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (out) bcm_field_range_t *range = where to put the assigned range ID
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_create(
	    int unit,
	    bcm_field_range_t *range,
	    uint32 flags,
	    bcm_l4_port_t min,
	    bcm_l4_port_t max)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_range_create_id
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 flags = flags for the range
 *      (in) bcm_l4_port_t min = low port number for the range
 *      (in) bcm_l4_port_t max = high port number for the range
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_create_id(
	    int unit,
	    bcm_field_range_t range,
	    uint32 flags,
	    bcm_l4_port_t min,
	    bcm_l4_port_t max)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_range_destroy
 *   Purpose
 *      Destroy a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to destroy
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_destroy(
	    int unit,
	    bcm_field_range_t range)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_range_get
 *   Purpose
 *      Create a descriptor for a range of L4 (specifically, TCP or UDP over
 *      IP) ports.
 *   Parameters
 *      (in) int unit = the unit number
 *      (in) bcm_field_range_t range = the range ID to use
 *      (in) uint32 *flags = where to put the flags for the range
 *      (in) bcm_l4_port_t *min = where to put range's low port number
 *      (in) bcm_l4_port_t *max = where to put range's high port number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 *      Range ID is always nonzero.
 */
int
bcm_tk371x_field_range_get(
	    int unit,
	    bcm_field_range_t range,
	    uint32 *flags,
	    bcm_l4_port_t *min,
	    bcm_l4_port_t *max)
{
	return BCM_E_UNAVAIL;
}

/*
 *   Function
 *      bcm_tk371x_field_show
 *   Purpose
 *      Dump all field information for the unit
 *   Parameters
 *      (in) int unit = the unit number
 *   Returns
 *      int (implied cast from bcm_error_t)
 *                    BCM_E_NONE if successful
 *                    BCM_E_* appropriately if not
 *   Notes
 */
int
bcm_tk371x_field_show(
	    int unit,
	    const char *pfx)
{
	return BCM_E_UNAVAIL;
}


#endif


