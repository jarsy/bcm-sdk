/*
 * $Id: $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 * All Rights Reserved.$
 *
 * File:       field_em.c
 * Purpose:    BCM56960 Field Processor Exact Match functions.
 */

#include <soc/defs.h>
#if defined(BCM_TOMAHAWK_SUPPORT) && defined(BCM_FIELD_SUPPORT)
#include <shared/bsl.h>
#include <soc/mem.h>
#include <soc/drv.h>
#include <bcm_int/esw/tomahawk.h>

#include <bcm/field.h>
#include <bcm_int/esw/field.h>

/* Local functions prototypes. */
STATIC
int _field_th_action_profiles_init(int unit, _field_stage_t *stage_fc);

STATIC
int _field_th_qos_action_profiles_init(int unit, _field_stage_t *stage_fc);

static soc_mem_t em_entry_nar_mem[_FP_MAX_NUM_PIPES] = {
    EXACT_MATCH_2_PIPE0m,
    EXACT_MATCH_2_PIPE1m,
    EXACT_MATCH_2_PIPE2m,
    EXACT_MATCH_2_PIPE3m,
};
static soc_mem_t em_entry_wide_mem[_FP_MAX_NUM_PIPES] = {
    EXACT_MATCH_4_PIPE0m,
    EXACT_MATCH_4_PIPE1m,
    EXACT_MATCH_4_PIPE2m,
    EXACT_MATCH_4_PIPE3m,
};

/*Action sets array as per bit positions in Action profile */
uint8 em_action_profile_bitmap[17] = {
    _FieldActionMeterSet,
    _FieldActionGreenToPidSet,
    _FieldActionMirrorOverrideSet,
    _FieldActionNatOverrideSet,
    _FieldActionSflowSet,
    _FieldActionCutThrOverrideSet,
    _FieldActionUrpfOverrideSet,
    _FieldActionTtlOverrideSet,
    _FieldActionLbControlSet,
    _FieldActionDropSet,
    _FieldActionChangeCpuCosSet,
    _FieldActionMirrorSet,
    _FieldActionNatSet,
    _FieldActionCopyToCpuSet,
    _FieldActionL3SwChangeL2Set,
    _FieldActionRedirectSet,
    _FieldActionCounterSet
};
uint8 ifp_td3_action_profile_bitmap[44] = {
    _FieldActionProtectionSwitchingDropOverrideSet,
    _FieldActionIfpDlbAlternatePathControlSet,
    _FieldActionEcmpDlbActionSet,
    _FieldActionHgtLagDlbActionSet,
    _FieldActionEcmp2RhActionSet,
    _FieldActionEcmp1RhActionSet,
    _FieldActionLagRhActionSet,
    _FieldActionHgtRhActionSet,
    _FieldActionLoopbackProfileSet,
    _FieldActionExtractionCtrlId,
    _FieldActionOpaque4Set,
    _FieldActionOpaque3Set,
    _FieldActionOpaque2Set,
    _FieldActionOpaque1Set,
    _FieldActionTxTimestampInsertionSet,
    _FieldActionRxTimestampInsertionSet,
    _FieldActionIgnoreFCOEZoneCheckSet,
    _FieldActionGreenToPidSet,
    _FieldActionMirrorOverrideSet,
    _FieldActionNatOverrideSet,
    _FieldActionSflowSet,
    _FieldActionCutThrOverrideSet,
    _FieldActionUrpfOverrideSet,
    _FieldActionTtlOverrideSet,
    _FieldActionLbControlSet,
    _FieldActionDropSet,
    _FieldActionChangeCpuCosSet,
    _FieldActionMirrorSet,
    _FieldActionNatSet,
    _FieldActionCopyToCpuSet,
    _FieldActionL3SwChangeL2Set,
    _FieldActionRedirectSet,
    _FieldActionCounterSet,
    _FieldActionChangeEcnSet,
    _FieldActionChangePktPriSet,
    _FieldActionChangeDscpTosSet,
    _FieldActionChangeDropPrecendenceSet,
    _FieldActionChangeCosOrIntPriSet,
    _FieldActionChangeIntCNSet,
    _FieldActionChangeInputPrioritySet,
    _FieldActionInstrumentationSet,
    _FieldActionEditCtrlIdSet,
    _FieldActionFcoeVsanSet,
    _FieldActionMeterSet
};
uint8 em_td3_action_profile_bitmap[37] = {
    _FieldActionProtectionSwitchingDropOverrideSet,
    _FieldActionIfpDlbAlternatePathControlSet,
    _FieldActionEcmpDlbActionSet,
    _FieldActionHgtLagDlbActionSet,
    _FieldActionEcmp2RhActionSet,
    _FieldActionEcmp1RhActionSet,
    _FieldActionLagRhActionSet,
    _FieldActionHgtRhActionSet,
    _FieldActionLoopbackProfileSet,
    _FieldActionExtractionCtrlId,
    _FieldActionOpaque4Set,
    _FieldActionOpaque3Set,
    _FieldActionOpaque2Set,
    _FieldActionOpaque1Set,
    _FieldActionTxTimestampInsertionSet,
    _FieldActionRxTimestampInsertionSet,
    _FieldActionIgnoreFCOEZoneCheckSet,
    _FieldActionGreenToPidSet,
    _FieldActionMirrorOverrideSet,
    _FieldActionNatOverrideSet,
    _FieldActionSflowSet,
    _FieldActionCutThrOverrideSet,
    _FieldActionUrpfOverrideSet,
    _FieldActionTtlOverrideSet,
    _FieldActionLbControlSet,
    _FieldActionDropSet,
    _FieldActionChangeCpuCosSet,
    _FieldActionMirrorSet,
    _FieldActionNatSet,
    _FieldActionCopyToCpuSet,
    _FieldActionL3SwChangeL2Set,
    _FieldActionRedirectSet,
    _FieldActionCounterSet,
    _FieldActionInstrumentationSet,
    _FieldActionEditCtrlIdSet,
    _FieldActionFcoeVsanSet,
    _FieldActionMeterSet
};
uint8 em_th2_action_profile_bitmap[23] = {
    _FieldActionMeterSet,
    _FieldActionGreenToPidSet,
    _FieldActionMirrorOverrideSet,
    _FieldActionNatOverrideSet,
    _FieldActionSflowSet,
    _FieldActionCutThrOverrideSet,
    _FieldActionUrpfOverrideSet,
    _FieldActionTtlOverrideSet,
    _FieldActionLbControlSet,
    _FieldActionDropSet,
    _FieldActionChangeCpuCosSet,
    _FieldActionMirrorSet,
    _FieldActionNatSet,
    _FieldActionCopyToCpuSet,
    _FieldActionL3SwChangeL2Set,
    _FieldActionRedirectSet,
    _FieldActionCounterSet,
    _FieldActionIfpDlbAlternatePathControlSet,
    _FieldActionEcmpDlbActionSet,
    _FieldActionHgtLagDlbActionSet,
    _FieldActionTimestampInsertionSet,
    _FieldActionInstrumentationSet,
    _FieldActionProtectionSwitchingDropOverrideSet,
};
/*
 * Function:
 *  _field_th_emstage_init
 * Purpose:
 *  Initialize Exact Match Stage.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  fc       - (IN) Field control structure.
 *  stage_fc - (IN) Stage field control structure.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  Exact Match Stage in Tomahawk is almost same as IFP stage. Hence there
 *  is no need to make separate database for extractor and ibus. Instead IFP
 *  database is created first and then exact match stage pointers just use
 *  same database instead of creating it's own database.
 */
int
_field_th_emstage_init(int unit, _field_control_t *fc,
                                 _field_stage_t *stage_fc)
{
    int rv = BCM_E_NONE;         /* Operation Return Status */
    _field_stage_t *stage_ing;   /* Ingress Stage Control Struct */

    /* Input parameter Check */
    if ((fc == NULL) || (stage_fc == NULL)) {
        return (BCM_E_PARAM);
    }

    /* Applicable only for exact match stage */
    if (stage_fc->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_NONE);
    }

    /* Get Ingress Stage Control Structure */
    stage_ing = fc->stages;
    while (stage_ing) {
        if (stage_ing->stage_id == _BCM_FIELD_STAGE_INGRESS) {
            break;
        }
        stage_ing = stage_ing->next;
    }

    /* Ingress stage check */
    if (NULL == stage_ing) {
        return (BCM_E_PARAM);
    }

    /* Point to Extractor Configuration array as ingress stage */
    stage_fc->ext_cfg_arr = stage_ing->ext_cfg_arr;

    /* Point to Ibus qual sec info as ingress stage */
    stage_fc->input_bus.size = stage_ing->input_bus.size;
    stage_fc->input_bus.num_fields = stage_ing->input_bus.num_fields;
    stage_fc->input_bus.qual_list = stage_ing->input_bus.qual_list;

    /* Initialise Keygen Profile */
    rv = _field_th_keygen_profiles_init(unit, stage_fc);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                  "FP(unit %d) Error: _field_th_keygen_profiles_init=%d\n"),
                  unit, rv));
        return (rv);
    }

    /* Initialise Action Profile */
    rv = _field_th_action_profiles_init(unit, stage_fc);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                  "FP(unit %d) Error: _field_th_action_profiles_init=%d\n"),
                  unit, rv));
        return (rv);
    }

    /* Initialise Qos Action Profile */
    rv = _field_th_qos_action_profiles_init(unit, stage_fc);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                "FP(unit %d) Error: _field_th_qos_action_profiles_init=%d\n"),
                 unit, rv));
        return (rv);
    }
    /* Initializing supported Qset*/
    sal_memset(&stage_fc->_field_supported_qset, 0, sizeof(bcm_field_qset_t));

    /* Initialize stage Preselector information. */
    rv = _bcm_field_th_stage_preselector_init(unit, fc, stage_fc);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
           "FP(unit %d) Error: _bcm_field_th_stage_preselector_init=%d"
           "\n"), unit, rv));
        return (rv);
    }

    return (rv);
}

/*
 * Function:
 *    _field_th_action_profiles_init
 * Purpose:
 *   Initialize keygen program profiles based on group operational mode.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     stage_fc - (IN/OUT) Stage field control structure.
 *
 * Returns:
 *    BCM_E_PARAM - Null field control structure or stage control parameter.
 *    BCM_E_INTERNAL - Null stage slice control structure.
 *    BCM_E_MEMORY - Allocation failure.
 *    BCM_E_NONE - Success.
 */
STATIC int
_field_th_action_profiles_init(int unit, _field_stage_t *stage_fc)
{
    soc_mem_t mem;               /* SOC memory names.         */
    int entry_words;             /* Entry size in words.      */
    int inst;                    /* Instance iterator.        */
    int rv;                      /* Operation return status.  */
    static const soc_mem_t act_prof_mem[_FP_MAX_NUM_PIPES] =
    {
        /* Exact Match Action Profile. */
        EXACT_MATCH_ACTION_PROFILE_PIPE0m,
        EXACT_MATCH_ACTION_PROFILE_PIPE1m,
        EXACT_MATCH_ACTION_PROFILE_PIPE2m,
        EXACT_MATCH_ACTION_PROFILE_PIPE3m
    };

    /* Input parameter check. */
    if (NULL == stage_fc) {
        return (BCM_E_PARAM);
    }

    /* Applicable only for Exact Match Stage */
    if (stage_fc->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_NONE);
    }

    switch (stage_fc->oper_mode) {

        /* Field Groups operational in Global mode. */
        case bcmFieldGroupOperModeGlobal:

            /* Initialize action profile memory name. */
            mem = EXACT_MATCH_ACTION_PROFILEm;

            /* Determine action profile entry size in number of words. */
            entry_words = soc_mem_entry_words(unit, mem);

            /* Create action program profile table. */
            rv = soc_profile_mem_create(unit, &mem, &entry_words, 1,
                    &stage_fc->action_profile[_FP_DEF_INST]);

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                   "FP(unit %d) Error: action profile creation failed."
                   "=%d\n"), unit, rv));
                return (rv);
            }
            break;

        /* Field Groups operational in Per-Pipe mode. */
        case bcmFieldGroupOperModePipeLocal:

            /* Determine action profiles entry size in number of words. */
            entry_words = soc_mem_entry_words(unit,
                                 EXACT_MATCH_ACTION_PROFILE_PIPE0m);

            for (inst = 0; inst < stage_fc->num_instances; inst++) {

                /* Initialize action program profile memory name. */
                mem = act_prof_mem[inst];

                /* Create action program profile table. */
                rv = soc_profile_mem_create(unit, &mem, &entry_words, 1,
                        &stage_fc->action_profile[inst]);

                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                       "FP(unit %d) Error: action profile creation failed."
                       "=%d\n"), unit, rv));
                    return (rv);
                }
            }
            break;

        default:
            /*
             * Invalid Group Operational mode, should never hit this condition.
             */
            return (BCM_E_INTERNAL);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *    _field_th_qos_action_profiles_init
 * Purpose:
 *   Initialize qos action profiles based on group operational mode.
 * Parameters:
 *     unit - (IN) BCM device number.
 *     stage_fc - (IN/OUT) Stage field control structure.
 *
 * Returns:
 *    BCM_E_PARAM - Null field control structure or stage control parameter.
 *    BCM_E_INTERNAL - Null stage slice control structure.
 *    BCM_E_MEMORY - Allocation failure.
 *    BCM_E_NONE - Success.
 */
STATIC int
_field_th_qos_action_profiles_init(int unit, _field_stage_t *stage_fc)
{
    soc_mem_t mem;               /* SOC memory names.                  */
    int entry_words;             /* Entry size in words.               */
    int inst;                    /* Instance iterator.                 */
    int rv;                      /* Operation return status.           */
    static const soc_mem_t qos_action_mem[_FP_MAX_NUM_PIPES] =
    {
        /* Exact Match Qos Action Profile. */
        EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE0m,
        EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE1m,
        EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE2m,
        EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE3m
    };

    /* Input parameter check. */
    if (NULL == stage_fc) {
        return (BCM_E_PARAM);
    }

    /* Applicable only for Exact Match Stage */
    if (stage_fc->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_NONE);
    }

    switch (stage_fc->oper_mode) {

        /* Field Groups operational in Global mode. */
        case bcmFieldGroupOperModeGlobal:

            /* Initialize profile1 memory name. */
            mem = EXACT_MATCH_QOS_ACTIONS_PROFILEm;

            /* Determine qos action profile entry size in number of words. */
            entry_words = soc_mem_entry_words(unit, mem);

            /* Create qos action program profile table. */
            rv = soc_profile_mem_create(unit, &mem, &entry_words, 1,
                    &stage_fc->qos_action_profile[_FP_DEF_INST]);

            if (BCM_FAILURE(rv)) {
                LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                   "FP(unit %d) Error: qos action "
                   "profile creation failed."
                   "=%d\n"), unit, rv));
                return (rv);
            }
            break;

         /* Field Groups operational in Per-Pipe mode. */
        case bcmFieldGroupOperModePipeLocal:

            /* Determine qos action profiles entry size in number of words. */
            entry_words = soc_mem_entry_words(unit,
                    EXACT_MATCH_QOS_ACTIONS_PROFILE_PIPE0m);

            for (inst = 0; inst < stage_fc->num_instances; inst++) {

                /* Initialize qos action program profile memory name. */
                mem = qos_action_mem[inst];

                /* Create qos action program profile table. */
                rv = soc_profile_mem_create(unit, &mem, &entry_words, 1,
                                         &stage_fc->qos_action_profile[inst]);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                       "FP(unit %d) Error: qos action "
                       "profile creation failed."
                       "=%d\n"), unit, rv));
                    return (rv);
                }
            }
            break;

        default:
            /*
             * Invalid Group Operational mode, should never hit this condition.
             */
            return (BCM_E_INTERNAL);
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *  _field_th_emstage_deinit
 * Purpose:
 *   De-Initialize Exact Match Stage.
 * Parameters:
 *   unit     - (IN) BCM device number.
 *   stage_fc - (IN) Stage field control structure.
 * Returns:
 *   BCM_E_XXX
 * Notes:
 */
int
_field_th_emstage_deinit(int unit, _field_stage_t *stage_fc)
{
    int rv = BCM_E_NONE;    /* Operation Return Status */

    /* Input Parameter Check. */
    if (stage_fc == NULL) {
        return (BCM_E_PARAM);
    }

    /* Do nothing for non EM stage. */
    if (stage_fc->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_NONE);
    }

    /* Since Exact Match Stage uses same extractor and input
     * bus database as ingress stage. hence deinit will happen
     * during ingress stage delete. Set to NULL here as we
     * are simply using ingress stage database previously.
     */

    stage_fc->ext_cfg_arr = NULL;
    stage_fc->input_bus.size = 0;
    stage_fc->input_bus.num_fields = 0;
    stage_fc->input_bus.qual_list = NULL;

    return (rv);
}

/*
 * Function:
 *  _field_th_em_entries_free_get
 * Purpose:
 *  Retreive Free Entries available in UFT table. 
 * Parameters:
 *  unit       - (IN)  BCM device number.
 *  fg         - (IN)  Field group structure.
 *  free_count - (OUT) Free Entries Count.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
int
_field_th_em_entries_free_get(int unit, _field_group_t *fg,
                                           int *free_count)
{
    _field_stage_t *stage_fc = NULL;    /* Field Stage Control.      */
    _field_slice_t *fs = NULL;          /* Slice info.               */
    int slice = 0;                      /* Slice Count Iterator.     */

    /* Input Paramter Check. */
    if ((fg == NULL) ||
        (free_count == NULL)) {
        return (BCM_E_PARAM);
    }

    /* Initialize free entry count. */
    *free_count = 0;

    /* Not applicable for non exact match stages. */
    if (fg->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_INTERNAL);
    }

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(
            _field_stage_control_get(unit, fg->stage_id, &stage_fc));

    /* Retreive Slice Instance. */
    fs = stage_fc->slices[fg->instance];

    /* Calculate free entry count. */
    for (slice = 0; slice < stage_fc->tcam_slices; slice++) {
        *free_count += fs[slice].free_count;
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *  _field_th_em_validate_view
 * Purpose:
 *   Validate Exact Match View.
 * Parameters:
 *   unit     - (IN) BCM device number.
 *   key_size - (IN) Qualifier Key Size.
 *   fg       - (IN/OUT) field group structure.
 * Returns:
 *   BCM_E_XXX
 * Notes:
 */
int
/*_field_th_em_validate_view(int unit, uint16 key_size, _field_group_t *fg)*/
_field_th_em_validate_view(int unit, uint16 qset_key_size,
                           _field_group_add_fsm_t *fsm_ptr)
{
    int rv = BCM_E_NONE;                /* Operational Status.   */
    uint16 aset_key_size;
    _field_group_t *fg;

    fg = fsm_ptr->fg;

    /* Input Parameter Check. */
    if (NULL == fg) {
        return (BCM_E_PARAM);
    }

    /* Retreive Action Set size from group*/
    aset_key_size = fsm_ptr->aset_size;
    /* Determine view of group
     * based on key and action size
     * View A(210b) = 160b key + 42b policy + 8b overhead
     *                policy (12b classid + 5b action profile id +
     *                        7b qos profile id + 18 bit action data
     * View B(210b) = 128b key + 74b policy + 8b overhead
     *                policy (12b classid + 5b action profile id +
     *                        7b qos profile id + 50 bit action data
     * View C(420b) = 320b key + 84b policy + 16b overhead
     *                policy (12b classid + 5b action profile id +
     *                        7b qos profile id + 60 bit action data
     */
    if (qset_key_size <= EM_MODE128_KEY_SIZE) {
        if (aset_key_size <= EM_MODE160_ACTION_DATA_SIZE) {
            fg->em_mode = _FieldExactMatchMode160;
        } else if ((EM_MODE160_ACTION_DATA_SIZE < aset_key_size)
                    && (aset_key_size <= EM_MODE128_ACTION_DATA_SIZE)) {
            fg->em_mode = _FieldExactMatchMode128;
        } else if ((EM_MODE128_ACTION_DATA_SIZE < aset_key_size) 
                    && (aset_key_size <= EM_MODE320_ACTION_DATA_SIZE)) {
            fg->em_mode = _FieldExactMatchMode320;
        } else {
            return (BCM_E_CONFIG);
        }
    } else if ((EM_MODE128_KEY_SIZE < qset_key_size) 
                && (qset_key_size <= EM_MODE160_KEY_SIZE)) {
        if (aset_key_size <= EM_MODE160_ACTION_DATA_SIZE) {
            fg->em_mode = _FieldExactMatchMode160;
        } else if ((EM_MODE160_ACTION_DATA_SIZE < aset_key_size)
                    && (aset_key_size <= EM_MODE320_ACTION_DATA_SIZE)) {
            fg->em_mode = _FieldExactMatchMode320;
        } else {
            return (BCM_E_CONFIG);
        }
    } else if ((EM_MODE160_KEY_SIZE < qset_key_size) 
                && (qset_key_size <= EM_MODE320_KEY_SIZE)) {
        if (aset_key_size <= EM_MODE320_ACTION_DATA_SIZE) {
            fg->em_mode = _FieldExactMatchMode320;
        } else {
            return (BCM_E_CONFIG);
        }
    } else {
        return (BCM_E_CONFIG);
    }

    /* Validate with group flag also */
    if (fg->flags & _FP_GROUP_SPAN_SINGLE_SLICE) {
        if (fg->em_mode == _FieldExactMatchMode320) {
            return (BCM_E_CONFIG);
        }
    }

    /* If Double Slice Request */
    if (fg->flags & _FP_GROUP_SPAN_DOUBLE_SLICE) {
        fg->em_mode = _FieldExactMatchMode320;
    }

    return (rv);
}

/*
 * Function:
 *  _field_th_em_qualifier_set
 * Purpose:
 *  Set Exact Match Qualifier Value.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  entry    - (IN) Entry ID.
 *  qual     - (IN) Qualifier Field.
 *  data     - (IN) Matching Data.
 *  mask     - (IN) Bit Mask For Data.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
int
_field_th_em_qualifier_set(int unit, bcm_field_entry_t entry,
                           int qual, uint32 *data, uint32 *mask)
{
    int rv = BCM_E_NONE;          /* Operational Status.    */
    _field_entry_t *f_ent;        /* Field Entry Control.   */
    _field_group_t *fg;           /* Field Group Struc.     */
    bcm_field_qualify_t qual_id;  /* Qualfier ID.           */
    int flag = 0;                 /* Flag.                  */

    if (NULL == data || NULL == mask) {
        return BCM_E_PARAM;
    }

    /* Retrieve the field entry. */
    rv = _field_entry_get(unit, entry, _FP_ENTRY_PRIMARY, &f_ent);
    BCM_IF_ERROR_RETURN(rv);

    /* Retreive Field Group. */
    fg = f_ent->group;

    if (qual == bcmFieldQualifyExactMatchHitStatus) {
        if (BCM_FIELD_QSET_TEST(fg->qset,
                     _bcmFieldQualifyExactMatchHitStatusLookup0)) {
            qual_id = _bcmFieldQualifyExactMatchHitStatusLookup0;
            rv = _bcm_field_th_qualify_set(unit, entry, qual_id,
                                                      data, mask,
                                                      _FP_QUALIFIER_DEL);
            BCM_IF_ERROR_RETURN(rv);
            flag = 1;
        }
        if (BCM_FIELD_QSET_TEST(fg->qset,
                     _bcmFieldQualifyExactMatchHitStatusLookup1)) {
            qual_id = _bcmFieldQualifyExactMatchHitStatusLookup1;
            rv = _bcm_field_th_qualify_set(unit, entry, qual_id,
                                                     data, mask,
                                                     _FP_QUALIFIER_DEL);
            BCM_IF_ERROR_RETURN(rv);
            flag = 1;
        }
    } else if (qual == bcmFieldQualifyExactMatchActionClassId) {
        if (BCM_FIELD_QSET_TEST(fg->qset,
                     _bcmFieldQualifyExactMatchActionClassIdLookup0)) {
            qual_id = _bcmFieldQualifyExactMatchActionClassIdLookup0;
            rv = _bcm_field_th_qualify_set(unit, entry, qual_id,
                                                     data, mask,
                                                     _FP_QUALIFIER_DEL);
            BCM_IF_ERROR_RETURN(rv);
            flag = 1;
        }
        if (BCM_FIELD_QSET_TEST(fg->qset,
                     _bcmFieldQualifyExactMatchActionClassIdLookup1)) {
            qual_id = _bcmFieldQualifyExactMatchActionClassIdLookup1;
            rv = _bcm_field_th_qualify_set(unit, entry, qual_id,
                                                     data, mask,
                                                     _FP_QUALIFIER_DEL);
            BCM_IF_ERROR_RETURN(rv);
            flag = 1;
        }
    } else if (qual == bcmFieldQualifyExactMatchGroupClassId) {
        if (BCM_FIELD_QSET_TEST(fg->qset,
                     _bcmFieldQualifyExactMatchGroupClassIdLookup0)) {
            qual_id = _bcmFieldQualifyExactMatchGroupClassIdLookup0;
            rv = _bcm_field_th_qualify_set(unit, entry, qual_id,
                                                     data, mask,
                                                     _FP_QUALIFIER_DEL);
            BCM_IF_ERROR_RETURN(rv);
            flag = 1;
        }
        if (BCM_FIELD_QSET_TEST(fg->qset,
                     _bcmFieldQualifyExactMatchGroupClassIdLookup1)) {
            qual_id = _bcmFieldQualifyExactMatchGroupClassIdLookup1;
            rv = _bcm_field_th_qualify_set(unit, entry, qual_id,
                                                     data, mask,
                                                     _FP_QUALIFIER_DEL);
            BCM_IF_ERROR_RETURN(rv);
            flag = 1;
        }
    } else {
        rv = BCM_E_PARAM;
    }

    /* Check if qual was present. */
    if (flag == 0) {
        LOG_ERROR(BSL_LS_BCM_FP,
                (BSL_META_U(unit,
                 "FP(unit %d) Error: qual=%s not in group=%d Qset\n"),
                 unit, _field_qual_name(qual), fg->gid));
        rv = BCM_E_PARAM;
    }

    return (rv);
}

/*
 * Function:
 *  _field_th_em_group_lookup_get
 * Purpose:
 *  Get Lookup information for exact match group
 *  with given priority.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  priority - (IN) Exact Match Group Priority.
 *  lookup   - (OUT) Exact Match Lookup Information.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  This function is applicable only for exact match
 *  group.
 */
int
_field_th_em_group_lookup_get(int unit,
                              int priority,
                              uint8 *lookup)
{
    int rv = BCM_E_NONE;        /* Operational Status.        */
    _field_control_t *fc;       /* Unit FP control structure. */
    _field_group_t *fg = NULL;  /* Group information.         */

    /* Input Parameter Check. */
    if (lookup == NULL) {
        return (BCM_E_PARAM);
    }

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_field_control_get(unit, &fc));

    /* Iterate over the groups linked-list looking for a matching
     * group Id. */
    fg = fc->groups;
    while (fg != NULL) {
        if ((fg->priority == priority) &&
            (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH)) {
            break;
        }
        fg = fg->next;
    }

    /* Exact Match Group Check. */
    if (fg == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
            "FP(unit %d) Error: Exact Match Group with priority %d "
            "does not exist.\n"),
            unit,priority));
        return (BCM_E_NOT_FOUND);
    }

    /* Slice Check. */
    if (fg->slices == NULL) {
        return (BCM_E_INTERNAL);
    }

    /* Return Lookup Information. */
    if (fg->slices->slice_number == 0) {
        *lookup = BCM_FIELD_EXACT_MATCH_LOOKUP_0;
    } else if (fg->slices->slice_number == 1) {
        *lookup = BCM_FIELD_EXACT_MATCH_LOOKUP_1;
    } else {
        rv = BCM_E_INTERNAL;
    }

    return (rv);
}

/*
 * Function:
 *  _field_th_em_ltid_based_groupid_get
 * Purpose:
 *  Get group id information for exact match group
 *  with given logical table id.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  ltid     - (IN) Exact Match Logical Table Id.
 *  group    - (OUT) Exact Match Group Id.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  This function is only applicable for exact match
 *  group.
 */
int
_field_th_em_ltid_based_groupid_get(int unit, int ltid,
                                              bcm_field_group_t *group)
{
    int rv = BCM_E_NONE;         /* Operational Status.        */
    _field_control_t *fc;        /* Unit FP control structure. */
    _field_group_t *fg = NULL;   /* Group information.         */

    /* Input Parameter Check */
    if (group == NULL) {
        return (BCM_E_PARAM);
    }

    /* Get unit FP control structure. */
    BCM_IF_ERROR_RETURN(_field_control_get(unit, &fc));

    /* Iterate over the groups linked-list looking for a matching Group ID */
    fg = fc->groups;
    while (fg != NULL) {
        if ((fg->lt_id == ltid) &&
            (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH)) {
            break;
        }
        fg = fg->next;
    }

    /* Exact Match Group Check. */
    if (fg == NULL) {
        LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
            "FP(unit %d) Error: Exact Match Group with ltid %d "
            "does not exist.\n"),
             unit,ltid));
        return (BCM_E_NOT_FOUND);
    }

    /* Return Group Information. */
    *group = fg->gid;

    return (rv);
}

/*
 * Function:
 *  _field_th_em_group_priority_hintbased_qset_update
 * Purpose:
 *  Update Field Group Qset based on Group Priority.
 * Parameters:
 *  unit       - (IN) BCM device number.
 *  hint_entry - (IN) - Field Hint Structure.
 *  fg         - (OUT) field group structure.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
int
_field_th_em_group_priority_hintbased_qset_update(int unit,
                                                  _field_group_t *fg,
                                                  bcm_field_hint_t *hint_entry)
{
    int rv = BCM_E_NONE;        /* Operational Status. */
    uint8 lkup = 0;             /* Exact Match Lookup. */

    /* Input Parameter Check. */
    if ((fg == NULL) || (hint_entry == NULL)) {
        return (BCM_E_PARAM);
    }

    /* Get Exact Match with provided priority. */
    rv = _field_th_em_group_lookup_get(unit, hint_entry->priority, &lkup);
    if (BCM_FAILURE(rv)) {
        return (rv);
    }

    /* Set Exact Match Hit Status. */
    if (hint_entry->qual == bcmFieldQualifyExactMatchHitStatus) {
        if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyExactMatchHitStatus)) {
            if (lkup == BCM_FIELD_EXACT_MATCH_LOOKUP_0) {
                BCM_FIELD_QSET_ADD(fg->qset,
                        _bcmFieldQualifyExactMatchHitStatusLookup0);
            } else if (lkup == BCM_FIELD_EXACT_MATCH_LOOKUP_1) {
                BCM_FIELD_QSET_ADD(fg->qset,
                        _bcmFieldQualifyExactMatchHitStatusLookup1);
            } else {
                rv = BCM_E_INTERNAL;
            }
        }
    } else if (hint_entry->qual == bcmFieldQualifyExactMatchGroupClassId) {
        /* Set Exact Match Group Class Id. */
        if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyExactMatchGroupClassId)) {
            if (lkup == BCM_FIELD_EXACT_MATCH_LOOKUP_0) {
                BCM_FIELD_QSET_ADD(fg->qset,
                        _bcmFieldQualifyExactMatchGroupClassIdLookup0);
            } else if (lkup == BCM_FIELD_EXACT_MATCH_LOOKUP_1) {
                BCM_FIELD_QSET_ADD(fg->qset,
                        _bcmFieldQualifyExactMatchGroupClassIdLookup1);
            } else {
                rv = BCM_E_INTERNAL;
            }
        }
    } else if (hint_entry->qual == bcmFieldQualifyExactMatchActionClassId) {
        /* Set Exact Match Action Class Id. */
        if (BCM_FIELD_QSET_TEST(fg->qset, bcmFieldQualifyExactMatchActionClassId)) {
            if (lkup == BCM_FIELD_EXACT_MATCH_LOOKUP_0) {
                BCM_FIELD_QSET_ADD(fg->qset,
                        _bcmFieldQualifyExactMatchActionClassIdLookup0);
            } else if (lkup == BCM_FIELD_EXACT_MATCH_LOOKUP_1) {
                BCM_FIELD_QSET_ADD(fg->qset,
                        _bcmFieldQualifyExactMatchActionClassIdLookup1);
            } else {
                rv = BCM_E_INTERNAL;
            }
        }
    } else {
        rv = BCM_E_PARAM;
    }

    return (rv);
}

/*
 * Function:
 *  _field_th_exactmatch_slice_validate
 * Purpose:
 *   Validate Exact Match Slice Allocation.
 * Parameters:
 *   unit     - (IN) BCM device number.
 *   stage_fc - (IN) Stage Control structure.
 *   fg       - (IN) field group structure.
 *   slice_id - (IN) Slice Identifier.
 * Returns:
 *   BCM_E_XXX
 * Notes:
 */
int
_field_th_exactmatch_slice_validate(int unit,
                                    _field_stage_t *stage_fc,
                                    _field_group_t *fg, int slice_id)
{
    int rv = BCM_E_NONE;                   /* Operational Status   */
    _field_group_t *fg_ptr = NULL;         /* Field Group Pointer. */
    _field_lt_slice_t *lt_slice = NULL;    /* Lt Slice Pointer.    */
    _field_slice_t *fs_part = NULL;        /* Field slice Pointer. */

    /* Input parameters check. */
    if (NULL == stage_fc || NULL == fg) {
        return (BCM_E_PARAM);
    }

    /* In Exact Match Single and Double Wide
     * Groups can share slices.
     */
    if ((fg->flags & _FP_GROUP_SPAN_DOUBLE_SLICE) && (1 == (slice_id % 3))) {
        LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META_U(unit,
                    "FP(unit %d) Verb: slices not available for DoubleWide"
                    " exact match group.\n"), unit));
        return (BCM_E_CONFIG);
    }

    if ((fg->flags & _FP_GROUP_SPAN_SINGLE_SLICE) && (1 == (slice_id % 3))) {
        fs_part = stage_fc->slices[fg->instance] + (slice_id - 1);
        if (fs_part->group_flags & _FP_GROUP_SPAN_DOUBLE_SLICE) {
            LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META_U(unit,
                            "FP(unit %d) Verb: slice=%d is secondary "
                            "slice of DoubleWide"
                            " group.\n"), unit, slice_id));
            return (BCM_E_CONFIG);
        }
    }

    if ((fg->flags & _FP_GROUP_SPAN_DOUBLE_SLICE)) {
        fs_part = stage_fc->slices[fg->instance] + (slice_id + 1);
        if (fs_part->group_flags & _FP_GROUP_SPAN_SINGLE_SLICE) {
            LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META_U(unit,
                            "FP(unit %d) Verb: slice=%d is already occupied "
                            "by SingleWide"
                            " group.\n"), unit, (slice_id + 1)));
            return (BCM_E_CONFIG);
        }
    }

    /* Retrieve the existing groups associated with the Slice */
    rv = _bcm_field_th_slice_group_get_next(unit, fg->instance,
                                            fg->stage_id, slice_id,
                                            &fg_ptr, &fg_ptr);
    if (BCM_SUCCESS(rv)) {
        /* Group with priority ANY, can't share
         * slice with any other group's. */
        if (BCM_FIELD_GROUP_PRIO_ANY == fg->priority) {
            LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META_U(unit, "Group with prio ANY"
                        "can't share slice with any other groups.\n\r")));
            return BCM_E_CONFIG;
        }

        /* Compare the group priority with the existing group in the slice. */
        if (fg->priority != fg_ptr->priority) {
            LOG_VERBOSE(BSL_LS_BCM_FP, (BSL_META_U(unit,
                "Slice[%d] contains other groups with different priority.\n\r"),
                 slice_id));
            return (BCM_E_CONFIG);
        }

        if ((fg_ptr->flags & _FP_GROUP_PRESELECTOR_SUPPORT) == 0) {
            LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                     "Group creation failed, Already default group exists"
                     " with the same priority[%d].\n\r"),
                     fg_ptr->priority));
            return (BCM_E_PARAM);
        }

        if ((fg->flags & _FP_GROUP_PRESELECTOR_SUPPORT) == 0) {
            LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                     "Group creation failed, can't create a group with the"
                     " priority"
                     " same as existing preselector group priority[%d]\n\r"),
                     fg_ptr->priority));
            return (BCM_E_PARAM);
        }

        lt_slice = fg_ptr->lt_slices;

    } else if (rv == BCM_E_NOT_FOUND) {
        /* Nothing to do. */
        rv = BCM_E_NONE;
    } else {
        return rv;
    }

    BCM_IF_ERROR_RETURN(_field_th_group_lt_slice_validate(unit, stage_fc, fg,
                                                          slice_id, lt_slice));

    return (rv);
}

/*
 * Function:
 *  _field_th_keygen_em_profile_entry_pack
 * Purpose:
 *  Build a LT keygen exact match program profile
 *  table entry.
 * Parameters:
 *  unit         - (IN)     BCM device number.
 *  stage_fc     - (IN)     Stage field control structure.
 *  fg           - (IN)     Field group structure.
 *  part         - (IN)     Entry part number.
 *  mem          - (IN)     Memory name identifier.
 *  prof_entry  - (IN/OUT) Entry buffer pointer.
 * Returns:
 *  BCM_E_XXX
 */
int
_field_th_keygen_em_profile_entry_pack(int unit, _field_stage_t *stage_fc,
                                   _field_group_t *fg, int part, soc_mem_t mem,
                        exact_match_key_gen_program_profile_entry_t *prof_entry)
{
    int idx; /* Iterator index. */
    static const soc_field_t l1_32_sel[] = /* 32 bit extractors. */
    {
        L1_E32_SEL_0f,
        L1_E32_SEL_1f,
        L1_E32_SEL_2f,
        L1_E32_SEL_3f
    };
    static const soc_field_t l1_16_sel[] = /* 16 bit extractors. */
    {
        L1_E16_SEL_0f,
        L1_E16_SEL_1f,
        L1_E16_SEL_2f,
        L1_E16_SEL_3f,
        L1_E16_SEL_4f,
        L1_E16_SEL_5f,
        L1_E16_SEL_6f
    };
    static const soc_field_t l1_8_sel[] = /* 8 bit extractors. */
    {
        L1_E8_SEL_0f,
        L1_E8_SEL_1f,
        L1_E8_SEL_2f,
        L1_E8_SEL_3f,
        L1_E8_SEL_4f,
        L1_E8_SEL_5f,
        L1_E8_SEL_6f
    };
    static const soc_field_t l1_4_sel[] = /* 4 bit extractors. */
    {
        L1_E4_SEL_0f,
        L1_E4_SEL_1f,
        L1_E4_SEL_2f,
        L1_E4_SEL_3f,
        L1_E4_SEL_4f,
        L1_E4_SEL_5f,
        L1_E4_SEL_6f,
        L1_E4_SEL_7f
    };
    static const soc_field_t l1_2_sel[] = /* 2 bit extractors. */
    {
        L1_E2_SEL_0f,
        L1_E2_SEL_1f,
        L1_E2_SEL_2f,
        L1_E2_SEL_3f,
        L1_E2_SEL_4f,
        L1_E2_SEL_5f,
        L1_E2_SEL_6f,
        L1_E2_SEL_7f
    };
    static const soc_field_t l2_16_sel[] = /* 16 bit extractors. */
    {
        L2_E16_SEL_0f,
        L2_E16_SEL_1f,
        L2_E16_SEL_2f,
        L2_E16_SEL_3f,
        L2_E16_SEL_4f,
        L2_E16_SEL_5f,
        L2_E16_SEL_6f,
        L2_E16_SEL_7f,
        L2_E16_SEL_8f,
        L2_E16_SEL_9f
    };
    static const soc_field_t l3_4_sel[] = /* 4 bit extractors. */
    {
        L3_E4_SEL_0f,
        L3_E4_SEL_1f,
        L3_E4_SEL_2f,
        L3_E4_SEL_3f,
        L3_E4_SEL_4f,
        L3_E4_SEL_5f,
        L3_E4_SEL_6f,
        L3_E4_SEL_7f,
        L3_E4_SEL_8f,
        L3_E4_SEL_9f,
        L3_E4_SEL_10f,
        L3_E4_SEL_11f,
        L3_E4_SEL_12f,
        L3_E4_SEL_13f,
        L3_E4_SEL_14f,
        L3_E4_SEL_15f,
        L3_E4_SEL_16f,
        L3_E4_SEL_17f,
        L3_E4_SEL_18f,
        L3_E4_SEL_19f,
        L3_E4_SEL_20f
    };
    static const soc_field_t l3_2_sel[] = /* 2 bit extractors. */
    {
        L3_E2_SEL_0f,
        L3_E2_SEL_1f,
        L3_E2_SEL_2f,
        L3_E2_SEL_3f,
        L3_E2_SEL_4f
    };
    static const soc_field_t l3_1_sel[] = /* 1 bit extractors. */
    {
        L3_E1_SEL_0f,
        L3_E1_SEL_1f
    };

    /* Input parameters check. */
    if ((NULL == stage_fc) || (NULL == fg) || (NULL == prof_entry)) {
        return (BCM_E_PARAM);
    }

    /* Set 32bit extractor selcode values. */
    for (idx = 0; idx < 4; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l1_e32_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l1_32_sel[idx],
                    fg->ext_codes[part].l1_e32_sel[idx]);
        }
    }

    /* Set 16bit extractor selcode values. */
    for (idx = 0; idx < 7; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l1_e16_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l1_16_sel[idx],
                    fg->ext_codes[part].l1_e16_sel[idx]);
        }
    }

    /* Set 8bit extractor selcode values. */
    for (idx = 0; idx < 7; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l1_e8_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l1_8_sel[idx],
                    fg->ext_codes[part].l1_e8_sel[idx]);
        }
    }

    /* Set 4bit extractor selcode values. */
    for (idx = 0; idx < 8; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l1_e4_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l1_4_sel[idx],
                    fg->ext_codes[part].l1_e4_sel[idx]);
        }
    }

    /* Set 2bit extractor selcode values. */
    for (idx = 0; idx < 8; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l1_e2_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l1_2_sel[idx],
                    fg->ext_codes[part].l1_e2_sel[idx]);
        }
    }

    /* Set 16bit extractor selcode values. */
    for (idx = 0; idx < 10; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l2_e16_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l2_16_sel[idx],
                    fg->ext_codes[part].l2_e16_sel[idx]);
        }
    }

    /* Set 4bit extractor selcode values. */
    for (idx = 0; idx < 21; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l3_e4_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l3_4_sel[idx],
                    fg->ext_codes[part].l3_e4_sel[idx]);
        }
    }

    /* Set 2bit extractor selcode values. */
    for (idx = 0; idx < 5; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l3_e2_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l3_2_sel[idx],
                    fg->ext_codes[part].l3_e2_sel[idx]);
        }
    }

    /* Set 1bit extractor selcode values. */
    for (idx = 0; idx < 2; idx++) {
        if (_FP_EXT_SELCODE_DONT_CARE != fg->ext_codes[part].l3_e1_sel[idx]) {
            soc_mem_field32_set(unit, mem, prof_entry, l3_1_sel[idx],
                    fg->ext_codes[part].l3_e1_sel[idx]);
        }
    }

    return (BCM_E_NONE);
}
/*
 * Function:
 *     _bcm_field_th_em_udf_keygen_mask_get
 * Purpose:
 *  Get the mask to be applied for data qualifiers.
 * Parameters:
 *  unit         - (IN)     BCM device number.
 *  fg           - (IN)     Field group structure.
 *  qid          - (IN)     Qualifier number.
 *  mask         - (OUT)    Mask to be applied.
 * Returns:
 *  BCM_E_XXX
 */
int _bcm_field_th_em_udf_keygen_mask_get(int unit, 
                                         _field_group_t *fg,
                                         uint16 qid, uint32 *mask)
{
    int rv = BCM_E_NONE; /* Operational status. */
    uint8 idx = 0;       /* Index Variable. */
    uint8 num_bytes = 0; /* number of bytes used in each UDF chunk. */
    _field_stage_t *stage_fc = NULL; /* Stage operational Structure. */

    /* Parameters Check. */
    if (NULL == fg) {
        return BCM_E_PARAM;
    }

    /* Exact Match and IFP shares UDF's */
    rv = _field_stage_control_get(unit, _BCM_FIELD_STAGE_INGRESS, &stage_fc);
    BCM_IF_ERROR_RETURN(rv);

    /* Derive the offset of byte bit map in QSET's udf_map 
     * for a data qualifier. */

    *mask = 0;
    switch (qid) {
         case _bcmFieldQualifyData0:
              num_bytes = 2;
              idx = stage_fc->data_ctrl->num_elems * 2;
              break; 
         case _bcmFieldQualifyData1:
              num_bytes = 2;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 2;
              break;
         case _bcmFieldQualifyData2:
              num_bytes = 4;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 4;
              break;
         case _bcmFieldQualifyData3:
              num_bytes = 4;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 8;
              break;
         case _bcmFieldQualifyData4:
              num_bytes = 4;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 12;
              break;
         case _bcmFieldQualifyData5:
              num_bytes = 2;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 16;
              break;
         case _bcmFieldQualifyData6:
              num_bytes = 2;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 18;
              break;
         case _bcmFieldQualifyData7:
              num_bytes = 4;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 20;
              break;
         case _bcmFieldQualifyData8:
              num_bytes = 4;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 24;
              break;
         case _bcmFieldQualifyData9:
              num_bytes = 4;
              idx = (stage_fc->data_ctrl->num_elems * 2) + 28;
              break;
         default:
              *mask = 0xFFFFFFFF;
              break;
    }

    if (idx) {

        if (SHR_BITGET(fg->qset.udf_map, idx)) { 
            *mask |= 0xFF;            
        }

        if (SHR_BITGET(fg->qset.udf_map, idx + 1)) {
            *mask |= 0xFF00;
        }

        if (4 == num_bytes) {
            if (SHR_BITGET(fg->qset.udf_map, idx + 2)) {
                *mask |= 0xFF0000;
            }

            if (SHR_BITGET(fg->qset.udf_map, idx + 3)) {
                *mask |= 0xFF000000;
            }
        }
    }

    return BCM_E_NONE;
}
/*
 * Function:
 *  _field_th_keygen_em_profile_mask_entry_pack
 * Purpose:
 *  Build a LT keygen exact match program profile
 *  mask table entry.
 * Parameters:
 *  unit         - (IN)     BCM device number.
 *  stage_fc     - (IN)     Stage field control structure.
 *  fg           - (IN)     Field group structure.
 *  part         - (IN)     Entry part number.
 *  mem          - (IN)     Memory name identifier.
 *  prof_entry  - (IN/OUT) Entry buffer pointer.
 * Returns:
 *  BCM_E_XXX
 */
int
_field_th_keygen_em_profile_mask_entry_pack(int unit,
                                _field_stage_t *stage_fc,
                                _field_group_t *fg,
                                int part, soc_mem_t mem,
                                exact_match_key_gen_mask_entry_t *prof_entry)
{
    _bcm_field_group_qual_t *q_arr;     /* Group Qualifier Array.    */
    _bcm_field_qual_offset_t *offset;   /* Qualifier Offset Pointer. */
    uint32 *fn_data;                    /* Data Buffer Pointer.      */
    uint32 value;                       /* Mask Value.               */
    uint32 mask; 
    int i,j;                            /* Iterators.                */
    int rv = BCM_E_NONE;                /* Operational Status        */

    /* Input Parameter Check. */
    if ((NULL == stage_fc) ||
        (NULL == fg) ||
        (NULL == prof_entry)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Buffer data pointer and qualifier array. */
    fn_data = (uint32 *)prof_entry;
    q_arr = &(fg->qual_arr[_FP_ENTRY_TYPE_DEFAULT][part]);

    /* Set Mask entry. */
    if (q_arr != NULL) {
        if (q_arr->size > 0) {
            for(i=0; i < q_arr->size; i++) {

                offset = q_arr->offset_arr + i;

                if ((_bcmFieldQualifyData0 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData1 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData2 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData3 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData4 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData5 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData6 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData7 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData8 == q_arr->qid_arr[i]) ||
                    (_bcmFieldQualifyData9 == q_arr->qid_arr[i])) {

                    rv = _bcm_field_th_em_udf_keygen_mask_get(unit, fg,
                                                             q_arr->qid_arr[i],
                                                             &mask);
                    BCM_IF_ERROR_RETURN(rv);
                    for (j=0; j < offset->num_offsets; j++) {
                        if (offset->width[j] > 0) {
                            value = (1 << offset->width[j]) - 1;
                            value = value & mask;
                            mask = (mask >> offset->width[j]);
                            if (!value) {
                                continue;
                            }
                            rv = _bcm_field_th_val_set(fn_data, &value, 
                                                       offset->offset[j],
                                                       offset->width[j]);
                            BCM_IF_ERROR_RETURN(rv);
                        }
                    }
                } else {
                    for (j=0; j < offset->num_offsets; j++) {
                        if (offset->width[j] > 0) {
                            value = (1 << offset->width[j]) - 1;
                            rv = _bcm_field_th_val_set(fn_data, &value, 
                                                       offset->offset[j],
                                                       offset->width[j]);
                            BCM_IF_ERROR_RETURN(rv);

                        }
                    }
                }
            }
        }
    }

    return (BCM_E_NONE);
}

/*
 * Function:
 *  _field_th_em_flex_stat_action_set
 * Purpose:
 *  Attach Flex Stat Action data for Exact Match Entry.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry.
 *  buf      - (OUT) Exact Match Action Stat Data Buffer.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
STATIC int
_field_th_em_flex_stat_action_set(int unit, _field_entry_t *f_ent,
                                             soc_mem_t policy_mem,
                                                      uint32 *buf)
{
    int rv = BCM_E_NONE;             /* BCM Operational Status.      */
    _field_stage_t *stage_fc = NULL; /* Field Stage Control.         */
    _field_group_t *fg = NULL;       /* Field Group Control.         */
    _field_stat_t  *f_st = NULL;     /* Field statistics descriptor. */
    int idx = 0;                     /* Stat array index.            */
    uint32 g_off = 0, y_off = 0, r_off = 0;
                                     /* Color Offsets.               */
    uint32 hw_real_mode = 0;         /* Real Hw Mode.                */
    uint32 hw_pool_num = 0;          /* Stat pool Number.            */
    uint32 hw_ctr_base_idx = 0;      /* Stat Ctr base index.         */
    soc_mem_t mem;                   /* Memory Identifier.           */

    /* Input Parameters Check. */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == f_ent->fs) ||
        (NULL == buf)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Group */
    fg = f_ent->group;

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(
            _field_stage_control_get(unit, fg->stage_id, &stage_fc));

    if (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
        /* Get Hardware Buffer Pointer and memory identifier. */
        if ((fg->em_mode == _FieldExactMatchMode128) ||
            (fg->em_mode == _FieldExactMatchMode160)) {
            if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
                mem = EXACT_MATCH_2m;
            } else {
                mem = em_entry_nar_mem[fg->instance];
            }
        } else {
            if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
                mem = EXACT_MATCH_4m;
            } else {
                mem = em_entry_wide_mem[fg->instance];
            }
        }
    } else {
        mem = policy_mem;
    }

    /* Increment statistics hw reference count. */
    if ((f_ent->statistic.flags & _FP_ENTRY_STAT_VALID)
         && !(f_ent->statistic.flags & _FP_ENTRY_STAT_INSTALLED)) {

        /* Get statistics entity description structure. */
        BCM_IF_ERROR_RETURN
            (_bcm_field_stat_get(unit, f_ent->statistic.sid, &f_st));
        if (_FP_INVALID_INDEX != f_st->hw_index) {
            /*
             * For STATs that are shared by entries, hardware counters
             * are not allocated again. But reference count is incremented
             * for these counters.
             */
            f_st->hw_ref_count++;

            BCM_IF_ERROR_RETURN
                (_bcm_esw_stat_flex_attach_ingress_table_counters1
                 (unit, mem, 0, f_st->hw_mode, f_st->hw_index,
                  f_st->pool_index, buf));

            /* Mark entry as installed. */
            f_ent->statistic.flags |=  _FP_ENTRY_STAT_INSTALLED;

            /* Set Stat Action Buffer. */
            rv = _bcm_field_th_em_color_offset_get(unit, f_st, 
                                                   &g_off, &y_off, &r_off);
            BCM_IF_ERROR_RETURN(rv);

            rv = _bcm_field_th_val_set(buf, &g_off, 0, 2);
            BCM_IF_ERROR_RETURN(rv);

            rv = _bcm_field_th_val_set(buf, &y_off, 2, 2);
            BCM_IF_ERROR_RETURN(rv);

            rv = _bcm_field_th_val_set(buf, &r_off, 4, 2);
            BCM_IF_ERROR_RETURN(rv);

            hw_pool_num = f_st->pool_index;
            rv = _bcm_field_th_val_set(buf, &hw_pool_num, 6, 5);
            BCM_IF_ERROR_RETURN(rv);

            rv = _bcm_esw_stat_flex_get_hw_mode(unit,
                                                f_st->hw_mode, &hw_real_mode);
            BCM_IF_ERROR_RETURN(rv);

            rv = _bcm_field_th_val_set(buf, &hw_real_mode, 11, 2);
            BCM_IF_ERROR_RETURN(rv);

            hw_ctr_base_idx = f_st->hw_index;
            rv = _bcm_field_th_val_set(buf, &hw_ctr_base_idx, 13,
                 (soc_feature(unit, soc_feature_td3_style_fp) ? 13 : 12));
            BCM_IF_ERROR_RETURN(rv);

            /*
             * Write individual statistics previous value, first time
             * entry is installed in hardware.
             */
            if ((1 == f_st->hw_ref_count)
                 && !(f_ent->flags & _FP_ENTRY_INSTALLED)) {
                for (idx = 0; idx < f_st->nstat; idx++) {
                    BCM_IF_ERROR_RETURN
                        (_field_stat_value_set(unit, f_st, f_st->stat_arr[idx],
                                               f_st->stat_values[idx]));
                }
            }
        }
    }

    return (rv);
}

/*
 * Function:
 *  _field_th_em_qos_profile_actions_alloc
 * Purpose:
 *  Allocate Qos Actions Profile for Exact Match Entry.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry.
 *  qp_idx   - (OUT) Qos Actions Profile Index.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  This api will allocate actions in exact match
 *  qos profile table. Also will return qos profile
 *  index allocated.
 */
STATIC int
_field_th_em_qos_profile_actions_alloc(int unit,
                                       _field_entry_t *f_ent,
                                       uint32 *qp_idx)
{
    int rv = BCM_E_NONE;                /* Operational Status.               */
    _field_group_t *fg = NULL;          /* Field Group Control.              */
    _field_action_t *fa = NULL;         /* Field Action Structure.           */
    _field_stage_t *stage_fc = NULL;    /* Field Stage Control.              */
    exact_match_qos_actions_profile_entry_t qos_prof_entry;
                                        /* Qos Actions Profile Entry Buffer. */
    void *entries[1];                   /* Profile Entry.                    */
    uint32 *bufp = NULL;                /* Hardware Buffer Ptr.              */
    soc_mem_t mem;                      /* Memory Identifier.                */

    /* Input Parameter Check */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == qp_idx)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Entry Group */
    fg = f_ent->group;

    /* Retreive Stage Control */
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                                                 fg->stage_id,
                                                 &stage_fc));

    /* Assign Qos Actions Profile Memory */
    mem = EXACT_MATCH_QOS_ACTIONS_PROFILEm;

    /* Initialize Qos Profile Actions Entry */
    bufp = (uint32 *)&qos_prof_entry;
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
            soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Set QoS Profile Action Buffer. */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (_FP_ACTION_VALID & fa->flags) {
            switch (fa->action) {
                case bcmFieldActionCosQNew:
                case bcmFieldActionGpCosQNew:
                case bcmFieldActionYpCosQNew:
                case bcmFieldActionRpCosQNew:
                case bcmFieldActionCosMapNew:
                case bcmFieldActionGpCosMapNew:
                case bcmFieldActionYpCosMapNew:
                case bcmFieldActionRpCosMapNew:
                case bcmFieldActionUcastCosQNew:
                case bcmFieldActionGpUcastCosQNew:
                case bcmFieldActionYpUcastCosQNew:
                case bcmFieldActionRpUcastCosQNew:
                case bcmFieldActionMcastCosQNew:
                case bcmFieldActionGpMcastCosQNew:
                case bcmFieldActionYpMcastCosQNew:
                case bcmFieldActionRpMcastCosQNew:
                case bcmFieldActionPrioIntNew:
                case bcmFieldActionGpPrioIntNew:
                case bcmFieldActionYpPrioIntNew:
                case bcmFieldActionRpPrioIntNew:
                case bcmFieldActionDropPrecedence:
                case bcmFieldActionGpDropPrecedence:
                case bcmFieldActionYpDropPrecedence:
                case bcmFieldActionRpDropPrecedence:
                case bcmFieldActionPrioIntCopy:
                case bcmFieldActionPrioIntTos:
                case bcmFieldActionPrioIntCancel:
                case bcmFieldActionGpPrioIntCopy:
                case bcmFieldActionGpPrioIntTos:
                case bcmFieldActionGpPrioIntCancel:
                case bcmFieldActionYpPrioIntCopy:
                case bcmFieldActionYpPrioIntTos:
                case bcmFieldActionYpPrioIntCancel:
                case bcmFieldActionRpPrioIntCopy:
                case bcmFieldActionRpPrioIntTos:
                case bcmFieldActionRpPrioIntCancel:
                case bcmFieldActionGpIntCongestionNotificationNew:
                case bcmFieldActionYpIntCongestionNotificationNew:
                case bcmFieldActionRpIntCongestionNotificationNew:
                case bcmFieldActionPfcClassNew:
                case bcmFieldActionUntaggedPacketPriorityNew:
                    rv = _bcm_field_th_profile1_action_set(unit,
                                                           f_ent, fa,
                                                           bufp);
                    BCM_IF_ERROR_RETURN(rv);
                    fa->flags &= ~_FP_ACTION_DIRTY;
                    break;
                case bcmFieldActionEcnNew:
                case bcmFieldActionGpEcnNew:
                case bcmFieldActionYpEcnNew:
                case bcmFieldActionRpEcnNew:
                case bcmFieldActionPrioPktNew:
                case bcmFieldActionGpPrioPktNew:
                case bcmFieldActionYpPrioPktNew:
                case bcmFieldActionRpPrioPktNew:
                case bcmFieldActionGpTosPrecedenceNew:
                case bcmFieldActionDscpNew:
                case bcmFieldActionGpDscpNew:
                case bcmFieldActionYpDscpNew:
                case bcmFieldActionRpDscpNew:
                case bcmFieldActionPrioPktCopy:
                case bcmFieldActionGpPrioPktCopy:
                case bcmFieldActionYpPrioPktCopy:
                case bcmFieldActionRpPrioPktCopy:
                case bcmFieldActionPrioPktTos:
                case bcmFieldActionGpPrioPktTos:
                case bcmFieldActionYpPrioPktTos:
                case bcmFieldActionRpPrioPktTos:
                case bcmFieldActionPrioPktCancel:
                case bcmFieldActionGpPrioPktCancel:
                case bcmFieldActionYpPrioPktCancel:
                case bcmFieldActionRpPrioPktCancel:
                case bcmFieldActionDot1pPreserve:
                case bcmFieldActionGpDot1pPreserve:
                case bcmFieldActionYpDot1pPreserve:
                case bcmFieldActionRpDot1pPreserve:
                case bcmFieldActionDscpCancel:
                case bcmFieldActionGpDscpCancel:
                case bcmFieldActionYpDscpCancel:
                case bcmFieldActionRpDscpCancel:
                case bcmFieldActionGpTosPrecedenceCopy:
                case bcmFieldActionDscpPreserve:
                case bcmFieldActionGpDscpPreserve:
                case bcmFieldActionYpDscpPreserve:
                case bcmFieldActionRpDscpPreserve:
                    rv = _bcm_field_th_profile2_action_set(unit,
                                                           f_ent, fa,
                                                           bufp);
                    BCM_IF_ERROR_RETURN(rv);
                    fa->flags &= ~_FP_ACTION_DIRTY;
                    break;
                default :
                    /* Do nothing */
                    break;
            }
        }
    }

    /* Set Profile Entry. */
    entries[0] = bufp;

    /* Add entry to Qos Actions Profile Tables In Hardware */
    rv = soc_profile_mem_add(unit,
                             &stage_fc->qos_action_profile[fg->instance],
                             entries, 1, qp_idx);
    return rv;
}

/*
 * Function:
 *  _field_th_entry_action_profile_data_set 
 * Purpose:
 *  generate the action profiel data buffer from
 *  Actions Profile index 
 *    1) for Exact Match Entry of TH and TD3.
 *    2) for Ingress Entry of TD3.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry.
 *  abuf     - (OUT) Action Data Buffer.
 *  ap_idx   - (IN) Actions Profile Index.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  This api will return the filled action data
 *  buffer using action profile entry index
 */
int
_field_th_entry_action_profile_data_set(int unit,
                                   _field_entry_t *f_ent,
                                   uint32 *ap_idx,
                                   soc_mem_t policy_mem,
                                   uint32 *abuf)
{
    int rv = BCM_E_NONE;              /* Operational Status.          */
    _field_stage_t *stage_fc = NULL;  /* Field Stage Control.         */
    _field_group_t *fg = NULL;        /* Field Group Control.         */
    _field_action_t *fa = NULL;       /* Field Action Structure.      */
    exact_match_action_profile_entry_t em_act_prof_entry;
                                      /* Action Profile Table Buffer. */
    void *act_prof_entry;               /* Profile table buffer pointer */
    _field_em_action_set_t acts_buf[_FieldActionSetCount];
                                      /* Action Set Buuffer.          */
    _field_em_action_set_t *acts = NULL;
                                      /* Action Set Data.             */
    int idx;                          /* Exact Match Action Set
                                         Iterator.                    */
    uint32 act_offset = 0;            /* Exact Match Action Set
                                        Offset.                      */
    uint32 meterbuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                      /* Action Meter Data Buffer.    */
    uint8 field_action_set;           /* to hold Action set value     */
    _bcm_field_action_conf_t *action_conf_ptr;
                                      /* Action configuration pointer */
    int counter_present = 0;          /* Counter Present flag.        */
    void *entries[1];                 /* Profile Entry.               */
    soc_mem_t mem;                    /* Memory Identifier.           */
    uint32 *action_set_bitmap;       /* Action bitmap as per hardware*/
    uint32 act_valid =0;               /* Action valid flag */
    _bcm_field_action_set_t            *action_set_ptr;
                                        /* action set pointer */
    uint8 *hw_bitmap;                    /* hw bitmap */
    uint8 hw_bitmap_entries = 0;

    /* Input Parameter Check */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == abuf) ||
        (NULL == ap_idx)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Group */
    fg = f_ent->group;

    /* Retreive Stage Control */
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                                                 fg->stage_id,
                                                 &stage_fc));

    /* Check whether it is a valid stage */
    if (!(((SOC_IS_TOMAHAWK(unit) || SOC_IS_TOMAHAWK2(unit))
        && (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH))
        || soc_feature(unit, soc_feature_ifp_action_profiling)))
    {
        return BCM_E_INTERNAL;
    }
    action_set_ptr = stage_fc->action_set_ptr;
    /* Assign correct Action Profile Memory
     * and Action set's depending on stage */
    if (fg->stage_id == _BCM_FIELD_STAGE_EXACTMATCH) {
        /* Assign Exact Match Action Profile Memory */
        mem = EXACT_MATCH_ACTION_PROFILEm;
        act_prof_entry = &em_act_prof_entry;
        if (SOC_IS_TOMAHAWK(unit)) {
            hw_bitmap = em_action_profile_bitmap;
            hw_bitmap_entries =
                sizeof(em_action_profile_bitmap)/sizeof(uint8);
        } else if (SOC_IS_TOMAHAWK2(unit)){
            hw_bitmap = em_th2_action_profile_bitmap;
            hw_bitmap_entries =
                sizeof(em_th2_action_profile_bitmap)/sizeof(uint8);
        } else {
            hw_bitmap = em_td3_action_profile_bitmap;
            hw_bitmap_entries =
                sizeof(em_td3_action_profile_bitmap)/sizeof(uint8);
        }
    } else {
        return BCM_E_INTERNAL;
    }

    /* Initialise variables */
    sal_memset(acts_buf, 0, sizeof(acts_buf));
    sal_memcpy(&act_prof_entry, soc_mem_entry_null(unit, mem),
                soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Set Action Set Buffer. */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (_FP_ACTION_VALID & fa->flags) {
            /* get the action conf ptr from stage structure */
            action_conf_ptr = stage_fc->f_action_arr[fa->action];
            /* Test whether the current action is
             * supported in this stage */
            if (NULL != action_conf_ptr) {
                /* get the aset enum from action_conf_pointer */
                field_action_set = action_conf_ptr->offset->aset;
            }
            else if (fa->action == bcmFieldActionPolicerGroup ||
                     fa->action == bcmFieldActionStatGroup) {
                /* Policer and Stat are dealt later */
                continue;
            } else {

                /* action is not supported in this stage */
                LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                  "FP(unit %d) Error: Action is not supported \n"),
                  unit ));
                return (BCM_E_UNAVAIL);
            }

            /* if the aset for current action is 0, or
             * action_set size is 0 for this stage/chip
             * skip updating the action set data */
            if ((0 == field_action_set) ||
                (0 == action_set_ptr[field_action_set].size))
            {
                continue;
            } else  {
                /* Set Action Set Buffer. */
                acts = &acts_buf[field_action_set];
                rv = _bcm_field_th_action_set(unit, 0, f_ent,
                                          0, fa,  acts->data);
                if (BCM_FAILURE(rv)) {
                    LOG_ERROR(BSL_LS_BCM_FP, (BSL_META("TH Action set failed.\n")));
                    return rv;
                }
                acts->valid = 1;
            }
        }
    }

    /* Set Counter Action Data if present.
     * Stat can be attached to entry only if it is
     * present in entry group qset.*/
    if (f_ent->statistic.flags & _FP_ENTRY_STAT_VALID) {
        counter_present = 1;
    }

    if (counter_present) {
        acts = &acts_buf[_FieldActionCounterSet];
        rv = _field_th_em_flex_stat_action_set(unit, f_ent, policy_mem,
                                               acts->data);
        BCM_IF_ERROR_RETURN(rv);
        acts->valid = 1;
    }
    /* Set Meter related information */
    acts = &acts_buf[_FieldActionMeterSet];
#if defined(BCM_TRIDENT3_SUPPORT)
    if(soc_feature(unit, soc_feature_ifp_action_profiling)) {
        rv = _field_td3_ingress_policer_action_set(unit, f_ent, meterbuf);
    } else
#endif
    {
        rv = _field_th_ingress_policer_action_set(unit, f_ent, meterbuf);
    }
    BCM_IF_ERROR_RETURN(rv);
    rv = _bcm_field_th_val_get(meterbuf,
                      acts->data, 0,
                      action_set_ptr[_FieldActionMeterSet].size);
    BCM_IF_ERROR_RETURN(rv);
    acts->valid = 1;

    /* Handle Green to Pid Action. */
    if ((acts_buf[_FieldActionL3SwChangeL2Set].valid) ||
        (acts_buf[_FieldActionRedirectSet].valid)) {
        acts = &acts_buf[_FieldActionGreenToPidSet];
        acts->valid = 1;
        if (f_ent->flags & _FP_ENTRY_COLOR_INDEPENDENT) {
            acts->data[0] = 1;
        } else {
            acts->data[0] = 0;
        }
    }

    entries[0] = &act_prof_entry;

    /* Now Populate Action Data Buffer.
     * Sequence here for action set is hardware dependant
     */
    /* get the action profile pointer from fg structure */
    BCM_IF_ERROR_RETURN(soc_profile_mem_get(unit,
                             &stage_fc->action_profile[fg->instance],
                             *ap_idx, 1, entries));
    act_offset = 0;
    action_set_bitmap = entries[0];
    /* Iterate through all the valid actions
     * for the group and update action data buffer */
    {
        for (idx=0; idx < hw_bitmap_entries;idx++)
        {
            rv = _bcm_field_th_val_get(action_set_bitmap, &act_valid, idx, 1);
            BCM_IF_ERROR_RETURN(rv);
            if (act_valid == 1) {
                /*idx Bit is set in the action profile */
                if (acts_buf[hw_bitmap[idx]].valid)
                {
                     rv = _bcm_field_th_val_set(abuf,
                            &acts_buf[hw_bitmap[idx]].data[0],
                            act_offset,
                            action_set_ptr[hw_bitmap[idx]].size);
                     BCM_IF_ERROR_RETURN(rv);
                }
                act_offset +=  action_set_ptr[hw_bitmap[idx]].size;
            }
        }
    }
    return (rv);
}

/*
 * Function:
 *  _field_th_em_entry_data_set
 * Purpose:
 *   Set Exact Match Policy Data.
 * Parameters:
 *   unit     - (IN) BCM device number.
 *   f_ent    - (IN) Field Entry.
 *   entbuf   - (OUT) Entry Policy Data Buffer.
 *   qp_idx   - (OUT) Qos Actions Profile Index.
 *   ap_idx   - (OUT) Action Profile Index.
 * Returns:
 *   BCM_E_XXX
 * Notes:
 */
STATIC int
_field_th_em_entry_data_set(int unit,
                            _field_entry_t *f_ent,
                            uint32 *entbuf,
                            uint32 *qp_idx,
                            uint32 *ap_idx)
{
    int rv = BCM_E_NONE;              /* Operation Return Status.    */
    _field_stage_t *stage_fc = NULL;  /* Field Stage Control.        */
    _field_group_t *fg = NULL;        /* Field Group Control.        */
    _field_action_t *fa = NULL;       /* Field action descriptor.    */
    uint32 abuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                      /* Action Profile Data Buffer. */
    uint32 class_id = 0;              /* Action Param 0.             */

    /* Input Parameters Check. */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == entbuf) ||
        (NULL == qp_idx) ||
        (NULL == ap_idx)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Group. */
    fg = f_ent->group;

    /* Return for non EM Stage. */
    if (fg->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_INTERNAL);
    }

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                                                 fg->stage_id, &stage_fc));

    /* Get Action Exact Match class id. */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (_FP_ACTION_VALID & fa->flags) {
            if (fa->action == bcmFieldActionExactMatchClassId) {
                break;
            }
        }
    }
    
    /* If found set class id. */
    if (fa != NULL) {
        class_id = fa->param[0];
        fa->flags &= ~_FP_ACTION_DIRTY;
    }

    /* Set Qos Profile Index. */
    rv = _field_th_em_qos_profile_actions_alloc(unit,
                                                f_ent,
                                                qp_idx);
    BCM_IF_ERROR_RETURN(rv);
    /* Set Action Profile data. */
    rv = _field_th_entry_action_profile_data_set(unit,
                                            f_ent,
                                            ap_idx,
                                            0, abuf);
    if (BCM_FAILURE(rv)) {
        BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
                               &stage_fc->qos_action_profile[fg->instance],
                               *qp_idx));
        return (rv);
    }

    /* Set Action Data in entry Buffer. */
    if (fg->em_mode == _FieldExactMatchMode128) {
        soc_mem_field32_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE128__QOS_PROFILE_IDf, *qp_idx);
        soc_mem_field32_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE128__ACTION_PROFILE_IDf, *ap_idx);
        soc_mem_field_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE128__ACTION_DATAf, abuf);
        soc_mem_field32_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE128__EXACT_MATCH_CLASS_IDf,
                            class_id);
    } else if (fg->em_mode == _FieldExactMatchMode160) {
        soc_mem_field32_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE160__QOS_PROFILE_IDf, *qp_idx);
        soc_mem_field32_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE160__ACTION_PROFILE_IDf, *ap_idx);
        soc_mem_field_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE160__ACTION_DATAf, abuf);
        soc_mem_field32_set(unit, EXACT_MATCH_2m,
                            entbuf,
                            MODE160__EXACT_MATCH_CLASS_IDf,
                            class_id);
    } else if (fg->em_mode == _FieldExactMatchMode320) {
        soc_mem_field32_set(unit, EXACT_MATCH_4m,
                            entbuf,
                            MODE320__QOS_PROFILE_IDf, *qp_idx);
        soc_mem_field32_set(unit, EXACT_MATCH_4m,
                            entbuf,
                            MODE320__ACTION_PROFILE_IDf, *ap_idx);
        soc_mem_field_set(unit, EXACT_MATCH_4m,
                            entbuf,
                            MODE320__ACTION_DATAf, abuf);
        soc_mem_field32_set(unit, EXACT_MATCH_4m,
                            entbuf,
                            MODE320__EXACT_MATCH_CLASS_IDf, class_id);
    } else {
        /* Invalid Exact Match Mode. */
        return (BCM_E_PARAM);
    }

    return rv;
}

/*
 * Function:
 *  _field_th_em_entry_action_size_get
 * Purpose:
 *   Get exact match entry action size.
 *   required to add an action
 * Parameters:
 *   unit          - (IN) BCM device number.
 *   f_ent         - (IN) Field Entry.
 *   action_size   - (OUT) Qos Actions Profile Index.
 * Returns:
 *   BCM_E_XXX
 * Notes:
 *  This api returns size of action data based on
 *  actions added to entry, stat and policer
 *  attached to it.
 */
 int
_field_th_em_entry_action_size_get(int unit, _field_entry_t *f_ent,
                                   int *action_size, uint32 *action_prof_idx)
{
    _field_stage_t *stage_fc = NULL;    /* Field Stage Control.         */
    _field_group_t *fg;                 /* Field Group structure pointer. */
    _field_action_t *fa = NULL;         /* Field action descriptor.  */
    uint16 aset_key_size = 0;           /* Action Set Size.      */
    uint8 aset_cnt[_FieldActionSetCount] = {0};
                                        /* Action Per Set Count. */
    _bcm_field_action_conf_t           *action_conf_ptr;
                                        /* action configuration pointer */
    uint8 field_action_set;             /* to hold Aset enum */
    exact_match_action_profile_entry_t em_act_prof_entry;
                                        /* Action Profile Table Buffer. */
    void *entries[1];                   /* Profile Entry.               */
    void *act_prof_entry;               /* Profile table buffer pointer */
    soc_mem_t mem;                      /* Memory Identifier.           */
    _bcm_field_action_set_t            *action_set_ptr;
                                        /* action set pointer */
    uint32 action_profile_idx;          /* Action profile index         */

    /* Input Parameter Check. */
    if ((f_ent == NULL) ||
        (f_ent->group == NULL) ||
        (action_size == NULL)) {
        return (BCM_E_PARAM);
    }
    fg = f_ent->group;
    /* Allowed only for exact match stage currently. */
    if (f_ent->group->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_PARAM);
    }
    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(
            _field_stage_control_get(unit, f_ent->group->stage_id, &stage_fc));

    mem = EXACT_MATCH_ACTION_PROFILEm;
    act_prof_entry = &em_act_prof_entry;
    action_set_ptr = stage_fc->action_set_ptr;

    /* Initialise variables */
    sal_memcpy(act_prof_entry, soc_mem_entry_null(unit, mem),
                soc_mem_entry_words(unit, mem) * sizeof(uint32));

    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (_FP_ACTION_VALID & fa->flags) {
            /* get the action conf ptr from stage structure */
            action_conf_ptr = stage_fc->f_action_arr[fa->action];
            /* Test whether the current action is
             * supported in this stage */
            if (NULL != action_conf_ptr) {
                /* get the aset enum from action_conf_pointer */
                field_action_set = action_conf_ptr->offset->aset;
            } else if (fa->action == bcmFieldActionPolicerGroup) {
                /* Flag in ASETs to indicate group
                 * policer usage with the group */
                field_action_set = _FieldActionMeterSet;
            } else if (fa->action == bcmFieldActionStatGroup) {
                /* Flag in ASETs to indicate stat group
                 * usage with the group */
                 field_action_set = _FieldActionCounterSet;
            } else {
                 /* action is not supported in this stage */
                 /* add log_error with stage, action, chip*/
                 LOG_ERROR(BSL_LS_BCM_FP, (BSL_META_U(unit,
                  "FP(unit %d) Error: Action is not supported \n"),
                  unit ));
                 return BCM_E_INTERNAL;
            }
            /* if the aset for current action is 0, or
               action_set size is 0 for this stage/chip
               skip the hw field updation step */
            if ((0 == field_action_set) ||
                (0 == action_set_ptr[field_action_set].size)) {
                continue;
            } else {
               if (aset_cnt[field_action_set] ==0) {
                   aset_key_size +=
                              action_set_ptr[field_action_set].size;
                   aset_cnt[field_action_set]++;
                   if (NULL != action_prof_idx) {
                       soc_mem_field32_set(unit, mem, act_prof_entry,
                          action_set_ptr[field_action_set].hw_field, 1);
                   }
               }
            }
        }
    }
    /* Check if L3SW/Redirect action is set.
     * If yes then reserve 1 bit for Green To Pid. */
    if (aset_cnt[_FieldActionL3SwChangeL2Set] ||
        aset_cnt[_FieldActionRedirectSet]) {
        aset_key_size +=
                   action_set_ptr[_FieldActionGreenToPidSet].size;
        if (NULL != action_prof_idx) {
            soc_mem_field32_set(unit, mem, act_prof_entry,
                   action_set_ptr[_FieldActionGreenToPidSet].hw_field , 1);
        }
    }

    *action_size = aset_key_size;
    LOG_VERBOSE(BSL_LS_BCM_FP,(BSL_META_U(unit,
            "VERB: Default entry EM  aset size %d \n\r"),
             aset_key_size));

    if (NULL != action_prof_idx) {
        entries[0] = act_prof_entry;
        /* Add Entries To Action Profile Table In Hardware. */
        BCM_IF_ERROR_RETURN(soc_profile_mem_add(unit,
                             &stage_fc->action_profile[fg->instance],
                             entries, 1, &action_profile_idx));
        *action_prof_idx = action_profile_idx;
    }
    return (BCM_E_NONE);
}

/*
 * Function:
 *  _field_th_em_default_entry_install
 * Purpose:
 *  Install Default Exact Match Entry.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
STATIC int
_field_th_em_default_entry_install(int unit, _field_entry_t *f_ent)
{
    int rv = BCM_E_NONE;                /* Operational Status.       */
    _field_group_t *fg = NULL;          /* Field Group Control.      */
    _field_stage_t *stage_fc = NULL;    /* Field Stage Control.      */
    exact_match_default_policy_entry_t ebuf;
                                        /* Default Entry Buffer.     */
    uint32 *bufp = NULL;                /* Hardware Buffer Ptr.      */
    uint32 abuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                        /* Action Data Buffer.       */
    uint32 lt_entry[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                        /* LT Entry buffer.          */
    _field_presel_entry_t *f_presel = NULL;
                                        /* Presel entry structure.   */
    _field_lt_entry_t *f_lt_ent = NULL;
                                        /* Logical entry strucutre.  */
    _field_entry_t *f_ent_part = NULL;  /* Field Entry Part.         */
    _field_action_t *fa = NULL;         /* Field action descriptor.  */
    int part_idx = 0;                   /* Entry Part Index.         */
    int parts_count = 0;                /* Field Entry Parts Count.  */
    uint32 qp_idx = 0;                  /* Qos Actions Profile Index.*/
    uint32 ap_idx = 0;                  /* Actions Profile Index.    */
    uint32 class_id = 0;                /* Exact Match Classs Id.    */
    int aset_size = 0;                  /* Action Data Size.         */
    int idx = 0, tcam_idx = 0;          /* Iter variables.           */
    soc_mem_t mem;                      /* Memory Identifier.        */
    soc_mem_t lt_tcam_mem;              /* LT memory identifier.     */
    static soc_mem_t em_default_entry_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_DEFAULT_POLICY_PIPE0m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE1m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE2m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE3m,
    };                                  /* Default Policy Per Pipe
                                           Memory Identifier.        */

    /* Input Parameter Check. */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == f_ent->fs)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Group */
    fg = f_ent->group;

    /* Group must have a valid KeyGen index, else return error. */
    if (_FP_EXT_SELCODE_DONT_CARE == fg->ext_codes[0].keygen_index) {
        return (BCM_E_INTERNAL);
    }

    /* Should be default entry. */
    if (!(f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT)) {
        return (BCM_E_CONFIG);
    }

    /* Do nothing for non EM Stage. */
    if (fg->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_INTERNAL);
    }

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(
            _field_stage_control_get(unit, fg->stage_id, &stage_fc));

    /* Get Hardware Buffer Pointer and memory identifier. */
    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        mem = EXACT_MATCH_DEFAULT_POLICYm;
    } else {
        mem = em_default_entry_mem[fg->instance];
    }

    /* Initialize exact match default policy entry buffer. */
    bufp = (uint32 *)&ebuf;
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
            soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Get number of entry parts. */
    rv = _bcm_field_entry_tcam_parts_count(unit, fg->stage_id,
                                      fg->flags, &parts_count);
    BCM_IF_ERROR_RETURN(rv);

    /* Get Size of action attached to entry. */
    rv = _field_th_em_entry_action_size_get(unit, f_ent, &aset_size, &ap_idx);
    BCM_IF_ERROR_RETURN(rv);

    /* Validate action Data Size. */
    if (aset_size > EM_DEFAULT_POLICY_ACTION_DATA_SIZE) {
        /* Delete Action Profile. */
        BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
            &stage_fc->action_profile[fg->instance],
            ap_idx));
        return (BCM_E_RESOURCE);
    }

    /* Get Action Exact Match class id. */
    for (fa = f_ent->actions; fa != NULL; fa = fa->next) {
        if (_FP_ACTION_VALID & fa->flags) {
            if (fa->action == bcmFieldActionExactMatchClassId) {
                break;
            }
        }
    }

    /* If found set class id. */
    if (fa != NULL) {
        class_id = fa->param[0];
        fa->flags &= ~_FP_ACTION_DIRTY;
    }

    /* Set Qos Profile Index. */
    rv = _field_th_em_qos_profile_actions_alloc(unit,
                                                f_ent,
                                                &qp_idx);
    BCM_IF_ERROR_RETURN(rv);
    /* Set Action Profile data. */
    rv = _field_th_entry_action_profile_data_set(unit,
                                            f_ent,
                                            &ap_idx,
                                            0, abuf);
    if (BCM_FAILURE(rv)) {
        BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
                               &stage_fc->qos_action_profile[fg->instance],
                               qp_idx));
        /* Delete Action Profile. */
        BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
            &stage_fc->action_profile[fg->instance],
            ap_idx));
    }

    /* Set Default Policy Data Buffer. */
    soc_mem_field32_set(unit, mem,
                        bufp,
                        QOS_PROFILE_IDf, qp_idx);
    soc_mem_field32_set(unit, mem,
                        bufp,
                        ACTION_PROFILE_IDf, ap_idx);
    soc_mem_field_set(unit, mem,
                      bufp,
                      ACTION_DATAf, abuf);
    soc_mem_field32_set(unit, mem, bufp,
                        EXACT_MATCH_CLASS_IDf, class_id);

    /* Install entry in hardware. */
    rv = soc_mem_write(unit, mem, MEM_BLOCK_ALL,
                       fg->ext_codes[0].keygen_index, bufp);
    if (BCM_FAILURE(rv)) {
        goto bcm_error;
    }

    /* Enable Default Policy in Logical table entries. */
    if (fg->flags & _FP_GROUP_PRESELECTOR_SUPPORT) {
        for (idx = 0; idx < _FP_PRESEL_ENTRIES_MAX_PER_GROUP; idx++) {
            if (fg->presel_ent_arr[idx] != NULL) {
                f_presel = fg->presel_ent_arr[idx];

                tcam_idx = f_presel->hw_index + f_presel->lt_fs->start_tcam_idx;

                rv = _bcm_field_th_lt_tcam_policy_mem_get(unit, 
                                        stage_fc, fg->instance,
                                        _BCM_FIELD_MEM_TYPE_EM_LT,
                                        _BCM_FIELD_MEM_VIEW_TYPE_TCAM_DATA_COMB,
                                        &lt_tcam_mem, NULL);
                if (BCM_FAILURE(rv)) {
                    goto bcm_error;
                }

                /* Read entry into SW buffer. */
                rv = soc_mem_read(unit, lt_tcam_mem, MEM_BLOCK_ANY, tcam_idx,
                                                                    lt_entry);
                if (BCM_FAILURE(rv)) {
                    goto bcm_error;
                }

                /* Set entry valid bit status. */
                soc_mem_field32_set(unit, lt_tcam_mem, lt_entry, 
                                    DEFAULT_POLICY_ENABLEf, 1);

                /* Install entry in hardware. */
                rv = soc_mem_write(unit, lt_tcam_mem, MEM_BLOCK_ALL,
                                                 tcam_idx, lt_entry);
                if (BCM_FAILURE(rv)) {
                    goto bcm_error;
                }
            }
        }
    } else {
        for (idx = 0; idx < (fg->lt_grp_status.entry_count); idx++) {
            if (fg->lt_entry_arr[idx] == NULL) {
                continue;
            }
            f_lt_ent = fg->lt_entry_arr[idx];

            tcam_idx = f_lt_ent->index + f_lt_ent->lt_fs->start_tcam_idx;
            rv = _bcm_field_th_lt_tcam_policy_mem_get(unit, stage_fc, fg->instance,
                                                      _BCM_FIELD_MEM_TYPE_EM_LT,
                                                      _BCM_FIELD_MEM_VIEW_TYPE_TCAM_DATA_COMB,
                                                      &lt_tcam_mem, NULL);
            if (BCM_FAILURE(rv)) {
                goto bcm_error;
            }

            /* Read entry into SW buffer. */
            rv = soc_mem_read(unit, lt_tcam_mem, MEM_BLOCK_ANY, tcam_idx,
                                                                lt_entry);
            if (BCM_FAILURE(rv)) {
                goto bcm_error;
            }

            /* Set entry valid bit status. */
            soc_mem_field32_set(unit, lt_tcam_mem, lt_entry, DEFAULT_POLICY_ENABLEf, 1);

            /* Install entry in hardware. */
            rv = soc_mem_write(unit, lt_tcam_mem, MEM_BLOCK_ALL,
                                               tcam_idx, lt_entry);
            if (BCM_FAILURE(rv)) {
                goto bcm_error;
            }
        }
    }

    /* Set Flags. */
    for (part_idx = 0; part_idx < parts_count; part_idx++) {
        f_ent_part = f_ent + part_idx;

        /* Set Entry Flags. */
        f_ent_part->flags &= ~_FP_ENTRY_DIRTY;
        f_ent_part->flags |= _FP_ENTRY_INSTALLED;
        f_ent_part->flags |= _FP_ENTRY_ENABLED;
    }

    return (rv);

bcm_error:
    /* Delete QoS Action Profile. */
    BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
            &stage_fc->qos_action_profile[fg->instance],
            qp_idx));
    return (rv);
}

/*
 * Function:
 *  _field_th_em_default_entry_remove
 * Purpose:
 *  Remove Default Exact Match Entry.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 */
STATIC int
_field_th_em_default_entry_remove(int unit, _field_entry_t *f_ent)
{
    int rv = BCM_E_NONE;                /* Operational Status.       */
    _field_group_t *fg = NULL;          /* Field Group Control.      */
    _field_stage_t *stage_fc = NULL;    /* Field Stage Control.      */
    exact_match_default_policy_entry_t ebuf;
                                        /* Default Entry Buffer.     */
    uint32 *bufp = NULL;                /* Hardware Buffer Ptr.      */
    uint32 lt_entry[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                        /* LT Entry buffer.          */
    _field_presel_entry_t *f_presel = NULL;
                                        /* Presel entry structure.   */
    _field_lt_entry_t *f_lt_ent = NULL;
                                        /* Logical entry strucutre.  */
    _field_entry_t *f_ent_part = NULL;  /* Field Entry Part.         */
    int part_idx = 0;                   /* Entry Part Index.         */
    int parts_count = 0;                /* Field Entry Parts Count.  */
    uint32 qp_idx = 0;                  /* Qos Actions Profile Index.*/
    uint32 ap_idx = 0;                  /* Actions Profile Index.    */
    int idx = 0, tcam_idx = 0;          /* Iter variables.           */
    soc_mem_t mem;                      /* Memory Identifier.        */
    soc_mem_t lt_tcam_mem;              /* LT memory identifier.     */
    static soc_mem_t em_default_entry_mem[_FP_MAX_NUM_PIPES] = {
        EXACT_MATCH_DEFAULT_POLICY_PIPE0m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE1m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE2m,
        EXACT_MATCH_DEFAULT_POLICY_PIPE3m,
    };                                  /* Default Policy Per Pipe
                                           Memory Identifier.        */

    /* Input Parameter Check. */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == f_ent->fs)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Group */
    fg = f_ent->group;

    /* Group must have a valid KeyGen index, else return error. */
    if (_FP_EXT_SELCODE_DONT_CARE == fg->ext_codes[0].keygen_index) {
        return (BCM_E_INTERNAL);
    }

    /* Should be default entry. */
    if (!(f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT)) {
        return (BCM_E_CONFIG);
    }

    /* Do nothing for non EM Stage. */
    if (fg->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_INTERNAL);
    }

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(
            _field_stage_control_get(unit, fg->stage_id, &stage_fc));

    /* Get Hardware Buffer Pointer and memory identifier. */
    if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
        mem = EXACT_MATCH_DEFAULT_POLICYm;
    } else {
        mem = em_default_entry_mem[fg->instance];
    }

    /* Initialize exact match entries. */
    bufp = (uint32 *)&ebuf;
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
            soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Get number of entry parts. */
    rv = _bcm_field_entry_tcam_parts_count(unit, fg->stage_id,
                                           fg->flags, &parts_count);
    BCM_IF_ERROR_RETURN(rv);

    /* Read Exact Match Default Policy Entry. */
    rv = soc_mem_read(unit, mem, MEM_BLOCK_ANY,
                      fg->ext_codes[0].keygen_index, bufp);
    BCM_IF_ERROR_RETURN(rv);

    /* Retreive Qos and Action Profile Index from entry. */
    qp_idx = soc_mem_field32_get(unit, mem,
                                 bufp, QOS_PROFILE_IDf);
    ap_idx = soc_mem_field32_get(unit, mem,
                                 bufp, ACTION_PROFILE_IDf);

    /* Delete Entry Profile Memories for actions. */
    BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
                           &stage_fc->qos_action_profile[fg->instance],
                           qp_idx));
    BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
                           &stage_fc->action_profile[fg->instance],
                           ap_idx));

    /* Clear entry buffer. */
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
                soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Install clear entry in hardware. */
    BCM_IF_ERROR_RETURN(soc_mem_write(unit, mem, MEM_BLOCK_ALL,
                fg->ext_codes[0].keygen_index, bufp));

    /* Enable Default Policy in Logical table entries. */
    if (fg->flags & _FP_GROUP_PRESELECTOR_SUPPORT) {
        for (idx = 0; idx < _FP_PRESEL_ENTRIES_MAX_PER_GROUP; idx++) {
            if (fg->presel_ent_arr[idx] != NULL) {
                f_presel = fg->presel_ent_arr[idx];

                tcam_idx = f_presel->hw_index + f_presel->lt_fs->start_tcam_idx;

                BCM_IF_ERROR_RETURN
                    (_bcm_field_th_lt_tcam_policy_mem_get(unit, stage_fc, fg->instance,
                                                          _BCM_FIELD_MEM_TYPE_EM_LT,
                                                          _BCM_FIELD_MEM_VIEW_TYPE_TCAM_DATA_COMB,
                                                          &lt_tcam_mem, NULL));
                /* Read entry into SW buffer. */
                BCM_IF_ERROR_RETURN(soc_mem_read(unit,
                                                 lt_tcam_mem, MEM_BLOCK_ANY,
                                                 tcam_idx, lt_entry));

                /* Set entry valid bit status. */
                soc_mem_field32_set(unit, lt_tcam_mem,
                                    lt_entry, DEFAULT_POLICY_ENABLEf, 0);

                /* Install entry in hardware. */
                BCM_IF_ERROR_RETURN(soc_mem_write(unit, lt_tcam_mem, MEM_BLOCK_ALL,
                                                              tcam_idx, lt_entry));
            }
        }
    } else {
        for (idx = 0; idx < (fg->lt_grp_status.entry_count); idx++) {
            if (fg->lt_entry_arr[idx] == NULL) {
                continue;
            }
            f_lt_ent = fg->lt_entry_arr[idx];

            tcam_idx = f_lt_ent->index + f_lt_ent->lt_fs->start_tcam_idx;

            BCM_IF_ERROR_RETURN
                (_bcm_field_th_lt_tcam_policy_mem_get(unit, stage_fc, fg->instance,
                                                      _BCM_FIELD_MEM_TYPE_EM_LT,
                                                      _BCM_FIELD_MEM_VIEW_TYPE_TCAM_DATA_COMB,
                                                      &lt_tcam_mem, NULL));

            /* Read entry into SW buffer. */
            BCM_IF_ERROR_RETURN(soc_mem_read(unit,
                                             lt_tcam_mem, MEM_BLOCK_ANY,
                                             tcam_idx, lt_entry));

            /* Set entry valid bit status. */
            soc_mem_field32_set(unit, lt_tcam_mem,
                                lt_entry, DEFAULT_POLICY_ENABLEf, 0);

            /* Install entry in hardware. */
            BCM_IF_ERROR_RETURN(soc_mem_write(unit, lt_tcam_mem, MEM_BLOCK_ALL,
                                                           tcam_idx, lt_entry));
        }
    }

    /* Clear Flags. */
    for (part_idx = 0; part_idx < parts_count; part_idx++) {
        f_ent_part = f_ent + part_idx;
        /* CLear  Entry Flags. */
        f_ent_part->flags |= _FP_ENTRY_DIRTY;
        f_ent_part->flags &= ~_FP_ENTRY_INSTALLED;
        f_ent_part->flags &= ~_FP_ENTRY_ENABLED;
    }

    return (rv);
}

/*
 * Function:
 *  _field_th_em_entry_install
 * Purpose:
 *  Install Exact Match Entry in Hardware.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry Pointer.
 * Returns:
 *  BCM_E_XXX
 * Notes:
 *  f_ent is primary entry part.
 */
int
_field_th_em_entry_install(int unit, _field_entry_t *f_ent)
{
    int rv = BCM_E_NONE;                /* Operational Status.       */
    _field_stage_t *stage_fc = NULL;    /* Field Stage Control.      */
    _field_group_t *fg = NULL;          /* Field Group Control.      */
    _field_entry_t *f_ent_part = NULL;  /* Field Entry Part.         */
    exact_match_2_entry_t ebuf_nar;     /* Narrow Entry Buffer.      */
    exact_match_4_entry_t ebuf_wide;    /* Wide Entry Buffer.        */
    uint32 *bufp = NULL;                /* Hardware Buffer Ptr.      */
    uint32 tbuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                        /* Temp Buffer One.          */
    uint32 tbuf_temp[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                        /* Temp Buffer Two.          */
    uint32 qp_idx = 0;                  /* Qos Actions Profile Index.*/
    uint32 ap_idx = 0;                  /* Actions Profile Index.    */
    int part_idx = 0;                   /* Entry Part Index.         */
    int parts_count = 0;                /* Field Entry Parts Count.  */
    soc_mem_t mem;                      /* Memory Identifier.        */

    /* Input Parameters Check. */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == f_ent->fs)) {
        return (BCM_E_PARAM);
    }

    /* Retreive Field Group */
    fg = f_ent->group;

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(
            _field_stage_control_get(unit, fg->stage_id, &stage_fc));

    /* Not applicable for non EM Stage. */
    if (fg->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_INTERNAL);
    }
    /* set Action profile index */
    ap_idx = fg->action_profile_idx[0];
    /* Default Entry Install. */
    if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
        return _field_th_em_default_entry_install(unit, f_ent);
    }

    /* Get Hardware Buffer Pointer and memory identifier. */
    if ((fg->em_mode == _FieldExactMatchMode128) ||
        (fg->em_mode == _FieldExactMatchMode160)) {
        bufp = (uint32 *)&ebuf_nar;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_2m;
        } else {
            mem = em_entry_nar_mem[fg->instance];
        }
    } else {
        bufp = (uint32 *)&ebuf_wide;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_4m;
        } else {
            mem = em_entry_wide_mem[fg->instance];
        }
    }

    /* Get number of entry parts. */
    rv = _bcm_field_entry_tcam_parts_count(unit, fg->stage_id,
                                           fg->flags, &parts_count);
    BCM_IF_ERROR_RETURN(rv);

    /* Get Key If Not Installed, for all entry parts. */
    if (!(f_ent->flags & _FP_ENTRY_INSTALLED)) {
        for (part_idx = 0; part_idx < parts_count; part_idx++) {
            f_ent_part = f_ent + part_idx;
            BCM_IF_ERROR_RETURN(
                    _bcm_field_qual_tcam_key_mask_get(unit, f_ent_part));
        }
    }

    /* Initialize exact match entries. */
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
                soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Validate Parts and EM View. */
    if ((((fg->em_mode == _FieldExactMatchMode128) ||
          (fg->em_mode == _FieldExactMatchMode160)) &&
          (parts_count != 1)) ||
         ((fg->em_mode == _FieldExactMatchMode320) &&
          (parts_count != 2))) {
        return (BCM_E_INTERNAL);
    }

    /* Set Action Buffer. */
    rv = _field_th_em_entry_data_set(unit, f_ent, bufp, &qp_idx, &ap_idx);
    BCM_IF_ERROR_RETURN(rv);

    /* Set Exact Match Entry */
    if (fg->em_mode == _FieldExactMatchMode128) {

        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
        rv = _bcm_field_th_val_get(f_ent->tcam.key, tbuf, 0, 
                                   EM_MODE128_KEY_PART0_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE128__KEY_0_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent->tcam.key, tbuf,
                                   EM_MODE128_KEY_PART0_SIZE,
                                   EM_MODE128_KEY_PART1_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE128__KEY_1_ONLYf, tbuf);

    } else if (fg->em_mode == _FieldExactMatchMode160) {

        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
        rv = _bcm_field_th_val_get(f_ent->tcam.key, tbuf, 0, 
                                   EM_MODE160_KEY_PART0_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE160__KEY_0_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent->tcam.key, tbuf,
                                   EM_MODE160_KEY_PART0_SIZE,
                                   EM_MODE160_KEY_PART1_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE160__KEY_1_ONLYf, tbuf);

    } else if (fg->em_mode == _FieldExactMatchMode320) {

        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_2f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_3f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_2f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_3f, 1);
        rv = _bcm_field_th_val_get(f_ent->tcam.key, tbuf, 0,
                                   EM_MODE320_KEY_PART0_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_0_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent->tcam.key, tbuf,
                                   EM_MODE320_KEY_PART0_SIZE,
                                   (EM_ENTRY_PART0_KEYSIZE -
                                    EM_MODE320_KEY_PART0_SIZE));

        /* Key From Second Entry Part. */
        f_ent_part = f_ent + 1;
        rv = _bcm_field_th_val_get(f_ent_part->tcam.key, tbuf_temp, 0,
                                   (EM_MODE320_KEY_PART1_SIZE -
                                   (EM_ENTRY_PART0_KEYSIZE -
                                    EM_MODE320_KEY_PART0_SIZE)));
        rv = _bcm_field_th_val_set(tbuf, tbuf_temp,
                                   (EM_ENTRY_PART0_KEYSIZE -
                                    EM_MODE320_KEY_PART0_SIZE),
                                   (EM_MODE320_KEY_PART1_SIZE -
                                   (EM_ENTRY_PART0_KEYSIZE -
                                    EM_MODE320_KEY_PART0_SIZE)));
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_1_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent_part->tcam.key, tbuf,
                                   (EM_MODE320_KEY_PART1_SIZE -
                                   (EM_ENTRY_PART0_KEYSIZE -
                                    EM_MODE320_KEY_PART0_SIZE)), 
                                    EM_MODE320_KEY_PART2_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_2_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent_part->tcam.key, tbuf, 
                                   ((EM_MODE320_KEY_PART1_SIZE -
                                    (EM_ENTRY_PART0_KEYSIZE -
                                     EM_MODE320_KEY_PART0_SIZE)) +
                                     EM_MODE320_KEY_PART2_SIZE), 
                                     EM_MODE320_KEY_PART3_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_3_ONLYf, tbuf);

    } else {
        /* Invalid Exact Match Mode. */
        rv = BCM_E_PARAM;
        goto bcm_error;
    }

    /* Write Entry In Hardware. */
    rv = soc_mem_insert(unit, mem, MEM_BLOCK_ALL, bufp);

    if (BCM_FAILURE(rv)) {
        goto bcm_error;
    }

    /* Copy into Hw Key. */
    for (part_idx = 0; part_idx < parts_count; part_idx++) {
        f_ent_part = f_ent + part_idx;
        if(f_ent_part->tcam.key_hw == NULL) {
            _FP_XGS3_ALLOC(f_ent_part->tcam.key_hw,
                f_ent_part->tcam.key_size, "EM_TCAM Key Alloc.");
        }

        sal_memcpy(f_ent_part->tcam.key_hw,
                f_ent_part->tcam.key, f_ent_part->tcam.key_size);

        /* Set Entry Flags. */
        f_ent_part->flags &= ~_FP_ENTRY_DIRTY;
        f_ent_part->flags |= _FP_ENTRY_INSTALLED;
        f_ent_part->flags |= _FP_ENTRY_ENABLED;
    }

    /* Increment Hardware Count to count of entry parts. */
    for (part_idx = 0; part_idx < parts_count; part_idx++) {
        f_ent->fs->hw_ent_count++;
    }

    return rv;

bcm_error:
    /* Delete QoS Action Profile. */
    BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
            &stage_fc->qos_action_profile[fg->instance],
            qp_idx));
    return (rv);
}

/*
 * Function:
 *  _field_th_em_entry_remove
 * Purpose:
 *  Remove Exact Match Entry from hardware.
 * Parameters:
 *  unit     - (IN) BCM device number.
 *  f_ent    - (IN) Field Entry.
 * Returns:
 *   BCM_E_XXX
 * Notes:
 *   Install and Enabled Flags will
 *   be cleared by calling function.
 */
int
_field_th_em_entry_remove(int unit, _field_entry_t *f_ent)
{
    int rv = BCM_E_NONE;             /* Operational Status.         */
    _field_control_t *fc = NULL;     /* Field Control Structure.    */
    _field_stage_t *stage_fc = NULL; /* Field Stage Control.        */
    _field_group_t *fg = NULL;       /* Field Group Control.        */
    _field_entry_t *f_ent_part = NULL;
                                     /* Field Entry Part.           */
    exact_match_2_entry_t ebuf_nar;  /* Narrow Entry Buffer.        */
    exact_match_4_entry_t ebuf_wide; /* Wide Entry Buffer.          */
    uint32 *bufp = NULL;             /* Hardware Buffer Ptr.        */
    uint32 tbuf[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                     /* Temp Buffer One.            */
    uint32 tbuf_temp[SOC_MAX_MEM_FIELD_WORDS] = {0};
                                     /* Temp Buffer Two.            */
    int eindex = 0;                  /* Entry Index inn UFT memory. */
    uint32 qp_idx = 0;               /* Qos Actions Profile Index.  */
    int parts_count = 0;             /* Field entry parts count.    */
    int part_idx = 0;                /* Entry Part Iterator.        */
    soc_mem_t mem;                   /* Memory Identifier.          */

    /* Input Parameters Check. */
    if ((NULL == f_ent) ||
        (NULL == f_ent->group) ||
        (NULL == f_ent->fs)) {
        return (BCM_E_PARAM);
    }

    /* If secondary part return none as uninstall
     * has happened in primary part already.
     */
    if (f_ent->flags & _FP_ENTRY_SECONDARY) {
        return (BCM_E_NONE);
    }

    BCM_IF_ERROR_RETURN(_field_control_get(unit, &fc));

    /* Retreive Field Group. */
    fg = f_ent->group;

    /* Retreive Stage Control. */
    BCM_IF_ERROR_RETURN(_field_stage_control_get(unit,
                                                 fg->stage_id, &stage_fc));

    /* Do nothing for non EM Stage. */
    if (fg->stage_id != _BCM_FIELD_STAGE_EXACTMATCH) {
        return (BCM_E_INTERNAL);
    }

    /* Default Entry Remove. */
    if (f_ent->flags & _FP_ENTRY_EXACT_MATCH_GROUP_DEFAULT) {
        return _field_th_em_default_entry_remove(unit, f_ent);
    }

    /* Check if HW key is present to remove entry. */
    if (NULL == f_ent->tcam.key_hw) {
        return (BCM_E_PARAM);
    }

    /* Get Hardware Buffer Pointer and memory identifier. */
    if ((fg->em_mode == _FieldExactMatchMode128) ||
        (fg->em_mode == _FieldExactMatchMode160)) {
        bufp = (uint32 *)&ebuf_nar;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_2m;
        } else {
            mem = em_entry_nar_mem[fg->instance];
        }
    } else {
        bufp = (uint32 *)&ebuf_wide;
        if (stage_fc->oper_mode == bcmFieldGroupOperModeGlobal) {
            mem = EXACT_MATCH_4m;
        } else {
            mem = em_entry_wide_mem[fg->instance];
        }
    }

    /* Initialize exact match entries. */
    sal_memcpy(bufp, soc_mem_entry_null(unit, mem),
            soc_mem_entry_words(unit, mem) * sizeof(uint32));

    /* Get number of entry parts. */
    rv = _bcm_field_entry_tcam_parts_count(unit, fg->stage_id,
                                           fg->flags, &parts_count);
    BCM_IF_ERROR_RETURN(rv);

    /* Validate Parts and EM View. */
    if ((((fg->em_mode == _FieldExactMatchMode128) ||
          (fg->em_mode == _FieldExactMatchMode160)) &&
          (parts_count != 1)) ||
         ((fg->em_mode == _FieldExactMatchMode320) &&
          (parts_count != 2))) {
        return (BCM_E_INTERNAL);
    }

    /* Set Exact Match Entry Search Key. */
    if (fg->em_mode == _FieldExactMatchMode128) {

        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
        rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 0,
                                   EM_MODE128_KEY_PART0_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE128__KEY_0_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 
                                   EM_MODE128_KEY_PART0_SIZE,
                                   EM_MODE128_KEY_PART1_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE128__KEY_1_ONLYf, tbuf);

    } else if (fg->em_mode == _FieldExactMatchMode160) {

        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
        rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 0, 
                                   EM_MODE160_KEY_PART0_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE160__KEY_0_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 
                                   EM_MODE160_KEY_PART0_SIZE, 
                                   EM_MODE160_KEY_PART1_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE160__KEY_1_ONLYf, tbuf);

    } else if (fg->em_mode == _FieldExactMatchMode320) {

        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_0f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_1f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_2f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, KEY_TYPE_3f, fg->em_mode);
        soc_mem_field32_set(unit, mem, bufp, VALID_0f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_1f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_2f, 1);
        soc_mem_field32_set(unit, mem, bufp, VALID_3f, 1);
        rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 0, 
                                   EM_MODE320_KEY_PART0_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_0_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent->tcam.key_hw, tbuf, 
                                   EM_MODE320_KEY_PART0_SIZE, 59);

        /* Key From Second Entry Part. */
        f_ent_part = f_ent + 1;
        if (f_ent_part->tcam.key_hw == NULL) {
            return (BCM_E_PARAM);
        }
        rv = _bcm_field_th_val_get(f_ent_part->tcam.key_hw, tbuf_temp, 0, 42);
        rv = _bcm_field_th_val_set(tbuf, tbuf_temp, 59, 42);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_1_ONLYf, tbuf);;
        rv = _bcm_field_th_val_get(f_ent_part->tcam.key_hw, tbuf, 42, 
                                   EM_MODE320_KEY_PART2_SIZE);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_2_ONLYf, tbuf);
        rv = _bcm_field_th_val_get(f_ent_part->tcam.key_hw, tbuf, 143, 17);
        soc_mem_field_set(unit, mem, bufp, MODE320__KEY_3_ONLYf, tbuf);

    } else {
        /* Invalid Exact Match Mode. */
        return (BCM_E_PARAM);
    }

    /* Search Entry First Before Delete. */
    rv = soc_mem_search(unit, mem, MEM_BLOCK_ANY, &eindex, bufp, bufp, 0);
    if (BCM_FAILURE(rv)) {
        if (fc->init == TRUE) {
            return (rv);
        } else {
            /* If init is false it means field detach is happening.
             * which might be due to rc or fp init.
             * since soc init will clear all memories hence
             * exact match memory will be cleared and search will
             * not be a success. Since memory is cleared before itself
             * so delete call should return success in such case.
             */
            rv = BCM_E_NONE;
            return rv;
        }
    }

    /* Get Qos and Action Profile Index Allocated From Entry. */
    if (fg->em_mode == _FieldExactMatchMode128) {
        qp_idx = soc_mem_field32_get(unit, mem,
                                     bufp, MODE128__QOS_PROFILE_IDf);
    } else if (fg->em_mode == _FieldExactMatchMode160) {
        qp_idx = soc_mem_field32_get(unit, mem,
                                     bufp, MODE160__QOS_PROFILE_IDf);
    } else if (fg->em_mode == _FieldExactMatchMode320) {
        qp_idx = soc_mem_field32_get(unit, mem,
                                     bufp, MODE320__QOS_PROFILE_IDf);
    } else {
        /* Invalid Exact Mode Mode. */
        return (BCM_E_PARAM);
    }

    /* Delete Entry From HW if found. */
    BCM_IF_ERROR_RETURN(
            soc_mem_delete(unit, mem, MEM_BLOCK_ALL, bufp));

    /* Delete Entry Profile Memories for actions. */
    BCM_IF_ERROR_RETURN(soc_profile_mem_delete(unit,
                           &stage_fc->qos_action_profile[fg->instance],
                           qp_idx));

    /* Free Hw Entry Copy. */
    for (part_idx = 0; part_idx < parts_count; part_idx++) {
        f_ent_part = f_ent + part_idx;
        sal_free(f_ent_part->tcam.key_hw);
        f_ent_part->tcam.key_hw = NULL;

        /* Set Entry Flags. */
        f_ent_part->flags |= _FP_ENTRY_DIRTY;
        f_ent_part->flags &= ~_FP_ENTRY_INSTALLED;
        f_ent_part->flags &= ~_FP_ENTRY_ENABLED;
    }

    /* Decrement Hw Slice Entry Count to count of entry parts. */
    for (part_idx = 0; part_idx < parts_count; part_idx++) {
        f_ent->fs->hw_ent_count--;
    }

    return rv;
}

#else /* BCM_TOMAHAWK_SUPPORT && BCM_FIELD_SUPPORT */
int _th_field_not_empty;
#endif /* !BCM_TOMAHAWK_SUPPORT && !BCM_FIELD_SUPPORT */
