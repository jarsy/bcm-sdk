/*
 * $Id: stack.c,v 1.41 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * QE2000 Stack API
 */

#include <shared/bsl.h>

#include <soc/mem.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sirius.h>
#include <bcm_int/sbx/lock.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/sirius.h>
#include <bcm_int/sbx/state.h>

#include <bcm/error.h>
#include <bcm/stack.h>

int
bcm_sirius_stk_modid_set(int unit, int modid)
{
    bcm_sbx_subport_info_t *sp_info = NULL;
    int i = 0, j = 0;

    if (!BCM_STK_MOD_IS_NODE(modid)) {
	return BCM_E_PARAM;
    }

    SOC_SBX_CONTROL(unit)->node_id = BCM_STK_MOD_TO_NODE(modid);
    
    /* Fix modid values for previously created egress gports */
    if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
	for (i=0; i < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; i++) {
	    sp_info = &SOC_SBX_STATE(unit)->port_state->subport_info[i];
	    if (sp_info->valid == FALSE) continue;
	    BCM_GPORT_MODPORT_SET(sp_info->parent_gport, modid, 
					   BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport));
	    for (j=0; j < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; j++) {
		if (sp_info->egroup[j].egroup_gport == BCM_GPORT_INVALID) continue;
		BCM_GPORT_EGRESS_GROUP_SET(sp_info->egroup[j].egroup_gport, modid, 
					   BCM_GPORT_EGRESS_GROUP_GET(sp_info->egroup[j].egroup_gport));
	    }
	}
    }
    return soc_sirius_modid_set(unit, SOC_SBX_CONTROL(unit)->node_id);
}

int
bcm_sirius_stk_modid_get(int unit, int *modid)
{
  *modid = BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id); 
  return BCM_E_NONE;
}

int
bcm_sirius_stk_my_modid_set(int unit, int modid)
{
    bcm_sbx_subport_info_t *sp_info = NULL;
    int i = 0, j = 0;

     if (!BCM_STK_MOD_IS_NODE(modid)) {
	return BCM_E_PARAM;
    }

    SOC_SBX_CONTROL(unit)->node_id = BCM_STK_MOD_TO_NODE(modid);

    /* Fix modid values for previously created egress gports */
    if (SOC_SBX_STATE(unit)->port_state->subport_info != NULL) {
	for (i=0; i < SB_FAB_DEVICE_SIRIUS_MAX_FABRIC_PORTS; i++) {
	    sp_info = &SOC_SBX_STATE(unit)->port_state->subport_info[i];
	    if (sp_info->valid == FALSE) continue;
	    BCM_GPORT_MODPORT_SET(sp_info->parent_gport, modid, 
					   BCM_GPORT_MODPORT_PORT_GET(sp_info->parent_gport));
	    for (j=0; j < SB_FAB_DEVICE_SIRIUS_FIFO_GROUP_MAX; j++) {
		if (sp_info->egroup[j].egroup_gport == BCM_GPORT_INVALID) continue;
		BCM_GPORT_EGRESS_GROUP_SET(sp_info->egroup[j].egroup_gport, modid, 
					   BCM_GPORT_EGRESS_GROUP_GET(sp_info->egroup[j].egroup_gport));
	    }
	}
    }
    return soc_sirius_modid_set(unit, SOC_SBX_CONTROL(unit)->node_id);
}

int
bcm_sirius_stk_my_modid_get(int unit, int *modid)
{
    *modid = BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id);
    return BCM_E_NONE;
}

int
bcm_sirius_stk_module_enable(int unit,
			     int modid,
			     int nports, 
			     int enable)
{

    return BCM_E_NONE;
}

int
bcm_sirius_stk_module_protocol_set(int unit,
				   int node,
				   bcm_module_protocol_t  protocol)
{
    int node_type;
    soc_reg_t reg;
    soc_field_t field;
    uint32 rval = 0;
    source_node_type_tab_entry_t type_entry;    
    burst_size_per_node_entry_t burst_size_entry;    
    int burst_size;

    if ( (node < 0) || (node >= SBX_MAXIMUM_NODES) ) {
	/* out of range */
	return BCM_E_PARAM;
    }

    node = SOC_SBX_L2P_NODE(unit, node);
    
    if ( (node < 0) || (node >= 256) ) {
	/* out of range */
	return BCM_E_PARAM;
    }

    switch (protocol) {
	case bcmModuleProtocol1:
	case bcmModuleProtocol2:
	    /* node type 1 is for qe2k */
	    node_type = 1;
	    break;
	case bcmModuleProtocol3:
	    /* node type 0 is for qe4k with A/B plane */
	    node_type = 0;
	    break;
	case bcmModuleProtocol5:
	    /* node type 0 is for qe4k with A plane single grant */
	    node_type = 0;
	    break;
	case bcmModuleProtocol4:
	    /* node type 2 is for qe4k with A/A plane or A/B loopback plane */
	    node_type = 2;
	    break;
	default:
	    return BCM_E_PARAM;
    }

    switch (node/16) {
	case 0:
	    reg = N2NT_00r;
	    break;
	case 1:
	    reg = N2NT_01r;
	    break;
	case 2:
	    reg = N2NT_02r;
	    break;
	case 3:
	    reg = N2NT_03r;
	    break;
	case 4:
	    reg = N2NT_04r;
	    break;
	case 5:
	    reg = N2NT_05r;
	    break;
	case 6:
	    reg = N2NT_06r;
	    break;
	case 7:
	    reg = N2NT_07r;
	    break;
	case 8:
	    reg = N2NT_08r;
	    break;
	case 9:
	    reg = N2NT_09r;
	    break;
	case 10:
	    reg = N2NT_10r;
	    break;
	case 11:
	    reg = N2NT_11r;
	    break;
	case 12:
	    reg = N2NT_12r;
	    break;
	case 13:
	    reg = N2NT_13r;
	    break;
	case 14:
	    reg = N2NT_14r;
	    break;
	default:
	    reg = N2NT_15r;
	    break;
    }

    switch (node % 16) {
	case 0:
	    field = SUB_N2NT0f;
	    break;
	case 1:
	    field = SUB_N2NT1f;
	    break;
	case 2:
	    field = SUB_N2NT2f;
	    break;
	case 3:
	    field = SUB_N2NT3f;
	    break;
	case 4:
	    field = SUB_N2NT4f;
	    break;
	case 5:
	    field = SUB_N2NT5f;
	    break;
	case 6:
	    field = SUB_N2NT6f;
	    break;
	case 7:
	    field = SUB_N2NT7f;
	    break;
	case 8:
	    field = SUB_N2NT8f;
	    break;
	case 9:
	    field = SUB_N2NT9f;
	    break;
	case 10:
	    field = SUB_N2NT10f;
	    break;
	case 11:
	    field = SUB_N2NT11f;
	    break;
	case 12:
	    field = SUB_N2NT12f;
	    break;
	case 13:
	    field = SUB_N2NT13f;
	    break;
	case 14:
	    field = SUB_N2NT14f;
	    break;
	default:
	    field = SUB_N2NT15f;
	    break;
    }

    if ((field == SUB_N2NT15f) && (reg == N2NT_15r)) {
      LOG_ERROR(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "invalid node 255 selected: unit %d - reserved for local grant\n"),
                 unit));
      return BCM_E_PARAM;
    }

    /* update node type in QSA */
    SOC_IF_ERROR_RETURN(soc_reg32_read(unit, soc_reg_addr(unit, reg, REG_PORT_ANY, 0), &rval));
    soc_reg_field_set(unit, reg, &rval, field, node_type);
    SOC_IF_ERROR_RETURN(soc_reg32_write(unit, soc_reg_addr(unit, reg, REG_PORT_ANY, 0), rval));

    /* update node type in FR */
    SOC_IF_ERROR_RETURN(READ_SOURCE_NODE_TYPE_TABm(unit, MEM_BLOCK_ANY, node, &type_entry));
    soc_mem_field32_set(unit, SOURCE_NODE_TYPE_TABm, &type_entry, SOURCE_NODE_TYPEf, node_type);
    SOC_IF_ERROR_RETURN(WRITE_SOURCE_NODE_TYPE_TABm(unit, MEM_BLOCK_ANY, node, &type_entry));
    
    /* NOTE: eset node type table should be updated by
     * the bcm_fabric_distribution_set API
     */

    /* update burst size per node, set it based on the max number of possible channels
     *   node type 0: sirius FIC
     *   node type 1: qe2k FIC
     *   node type 2: sirius Hybrid
     *   node type 3: unused, same as sirius FIC
     */
    switch (node_type) {
	case 0:
	    burst_size = SIRIUS_BURST_SIZE_FIC;
	    break;
	case 1:
	    burst_size = SIRIUS_BURST_SIZE_QE2K;
	    break;
	case 2:
	    burst_size = SIRIUS_BURST_SIZE_HYBRID;
	    break;
	    /* coverity[dead_error_begin] */
	default:
	    burst_size = SIRIUS_BURST_SIZE_FIC;
	    break;	    
    }

    
    /* This table really configured by hardware per grant and has the # of channels, no need to update */
    SOC_IF_ERROR_RETURN(READ_BURST_SIZE_PER_NODEm(unit, MEM_BLOCK_ANY, node, &burst_size_entry));
    soc_mem_field32_set(unit, BURST_SIZE_PER_NODEm, &burst_size_entry, BURST_SIZE_NODEf, burst_size);
    SOC_IF_ERROR_RETURN(WRITE_BURST_SIZE_PER_NODEm(unit, MEM_BLOCK_ANY, node, &burst_size_entry));

    return BCM_E_NONE;
}

int
bcm_sirius_stk_module_protocol_get(int unit,
				   int node,
				   bcm_module_protocol_t *protocol)
{
    return BCM_E_NONE;
}

static int
_bcm_sirius_stk_steering_set(int unit,
                             int steer_id,
                             int multicast,
                             uint16 low,
                             uint16 high,
                             int num_queue_groups,
                             bcm_gport_t *queue_groups)
{
    soc_sbx_sirius_config_t *sir = SOC_SBX_CFG_SIRIUS(unit);
    bcm_fabric_predicate_t predId;
    bcm_fabric_predicate_action_t predActId;
    bcm_fabric_action_t actId;
    bcm_fabric_qsel_t queueSeg;
    unsigned int lbidSize;
    unsigned int index;
    unsigned int offset;
    unsigned int queue;
    bcm_gport_t *qs = NULL;
    bcm_fabric_qsel_offset_t *qso = NULL;
    int xgsMode = TRUE;
    int restoreQueueSeg = FALSE;
    int restorePredicate = FALSE;
    int restoreAction = FALSE;
    int result = BCM_E_NONE;
    int auxRes = BCM_E_NONE;
    bcm_fabric_predicate_info_t predInfo;
    bcm_fabric_predicate_action_info_t predActInfo;
    bcm_fabric_action_info_t actInfo;

    if ((1 > steer_id) || (SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES <= steer_id)) {
        /* for some reason, invalid ID is usually 'not found' */
        result = BCM_E_NOT_FOUND;
        goto exit;
    }
    if (!multicast) {
        if ((255 < low) || (255 < high)) {
            /* bogus module IDs */
            result = BCM_E_PARAM;
            goto exit;
        }
    }
    qs = sal_alloc(sizeof(bcm_gport_t) * 256, "GPORT workspace");
    if (!qs) {
        result = BCM_E_MEMORY;
        goto exit;
    }
    qso = sal_alloc(sizeof(bcm_fabric_qsel_offset_t) * 256,
                    "qsel_offset workspace");
    if (!qso) {
        result = BCM_E_MEMORY;
        goto exit;
    }
    /* must determine XGS or SBX mode first */
    xgsMode = (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_XGS);
    lbidSize = xgsMode?256:8;
    /* make sure the number of queue groups provide will fit */
    if ((lbidSize < num_queue_groups) || (1 > num_queue_groups)) {
        /* must have at least one queue group, but no more than lbid covers */
        result = BCM_E_CONFIG;
        goto exit;
    }
    /* prepare workspace */
    bcm_fabric_predicate_info_t_init(unit, &predInfo);
    bcm_fabric_predicate_action_info_t_init(unit, &predActInfo);
    bcm_fabric_action_info_t_init(unit, &actInfo);
    /* find assocaited predicate->action rule or allocate a new one */
    if (sir->userPredAction[steer_id - 1]) {
        predActId = sir->userPredAction[steer_id - 1] - 1;
        result = bcm_fabric_predicate_action_get(unit,
                                                 predActId,
                                                 &predActInfo);
        if (BCM_E_NONE != result) {
            goto exit;
        }
        /* will want to replace it later */
        predActInfo.flags |= (BCM_FABRIC_PREDICATE_ACTION_INFO_WITH_ID |
                              BCM_FABRIC_PREDICATE_ACTION_INFO_REPLACE);
    } else {
        /* need to create a new one later */
        predActInfo.flags = BCM_FABRIC_PREDICATE_ACTION_INFO_INGRESS;
    }
    /* find assigned action or allocate a new one */
    if (sir->userAction[steer_id - 1]) {
        actId = sir->userAction[steer_id - 1] - 1;
        result = bcm_fabric_action_get(unit, actId, &actInfo);
        if (BCM_E_NONE != result) {
            goto exit;
        }
        /* will want to replace this one later */
        actInfo.flags |= (BCM_FABRIC_ACTION_INFO_WITH_ID |
                          BCM_FABRIC_ACTION_INFO_REPLACE);
        /* load existing qsel */
        queueSeg = actInfo.qsel;
    } else {
        /* need to create a new action later */
        actInfo.flags = BCM_FABRIC_ACTION_INFO_INGRESS;
        /* need a new qsel */
        queueSeg = ~0;
    }
    /* find assigned predicate or allocate a free predicate */
    predInfo.flags = (BCM_FABRIC_PREDICATE_INFO_INGRESS |
                      BCM_FABRIC_PREDICATE_INFO_RANGE);
    if (sir->userPredicate[steer_id - 1]) {
        /* there is already an assigned predicate */
        predId = sir->userPredicate[steer_id - 1] - 1;
        predInfo.flags |= (BCM_FABRIC_PREDICATE_INFO_WITH_ID |
                           BCM_FABRIC_PREDICATE_INFO_REPLACE);
    } else {
        predId = ~0;
    }
    /* configure the predicate */
    predInfo.source = bcmFabricPredicateTypePacket;
    predInfo.field.offset = xgsMode ? 0x18 : 0x28;
    predInfo.field.length = 16;
    predInfo.range_low = multicast ? low : (low << 8);
    predInfo.range_high = multicast ? high : ((high << 8) | 0xFF);
    /* determine current qsel */
    if ((~0) == queueSeg) {
        /* need a queue segment */
        result = bcm_sirius_fabric_qsel_create(unit,
                                               BCM_FABRIC_QSEL_INGRESS,
                                               0,
                                               lbidSize,
                                               &queueSeg);
        if (BCM_E_NONE != result) {
            goto exit;
        }
        restoreQueueSeg = TRUE;
        actInfo.qsel = queueSeg;
    }
    /* set up the action configuration */
    actInfo.qsel = queueSeg;
    if (xgsMode) {
        actInfo.queue_field_low.offset = 48;
        actInfo.queue_field_low.length = 8;
        actInfo.qsel_offset_index_field.offset = 8;
        actInfo.qsel_offset_index_field.length = 4;
    } else {
        actInfo.queue_field_low.offset = 64;
        actInfo.queue_field_low.length = 3;
        actInfo.qsel_offset_index_field.offset = 67;
        actInfo.qsel_offset_index_field.length = 4;
    }
    /* program the queue map table block */
    for (index = 0, offset = 0;
         (index < lbidSize) && (BCM_E_NONE == result);
         offset++, index++) {
        if (offset > (num_queue_groups - 1)) {
            offset = 0;
        }
        queue = queue_groups[offset];
        if (multicast) {
            if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(queue)) {
                queue = BCM_GPORT_MCAST_QUEUE_GROUP_QID_GET(queue);
            } else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(queue)) {
                queue = BCM_GPORT_MCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(queue);
            } else {
                result = BCM_E_PARAM;
            }
        } else { /* if (multicast) */
            if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(queue)) {
                queue = BCM_GPORT_UCAST_QUEUE_GROUP_QID_GET(queue);
            } else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(queue)) {
                queue = BCM_GPORT_UCAST_SUBSCRIBER_QUEUE_GROUP_QID_GET(queue);
            } else {
                result = BCM_E_PARAM;
            }
        } /* if (multicast) */
        qs[index] = queue;
        qso[index] = (_SIRIUS_I_COS_PROFILE_GENERAL +
                      SIRIUS_COS_MAP_BLOCK_OFFSET_INGRESS);
    } /* for (all entries in new queue segment) */
    result = bcm_sirius_fabric_qsel_entry_multi_set(unit,
                                                    queueSeg,
                                                    0,
                                                    lbidSize,
                                                    &(qs[0]),
                                                    &(qso[0]));
    /* program the parser */
    if (BCM_E_NONE == result) {
        result = bcm_fabric_action_create(unit,
                                          &actInfo,
                                          &actId);
        if (BCM_E_NONE == result) {
            restoreAction = (0 == (actInfo.flags &
                                   BCM_FABRIC_ACTION_INFO_REPLACE));
        }
    }
    /* program the predicate */
    if (BCM_E_NONE == result) {
        result = bcm_sirius_fabric_predicate_create(unit, &predInfo, &predId);
        if (BCM_E_NONE == result) {
            restorePredicate = (0 == (predInfo.flags &
                                      BCM_FABRIC_PREDICATE_INFO_REPLACE));
        }
    }
    /* program the rule to select the action */
    if (BCM_E_NONE == result) {
        predActInfo.action_id = actId;
        predActInfo.priority = 16 + steer_id;
        for (index = 0;
             index < BCM_FABRIC_PREDICATE_COUNT;
             index++) {
            if (index == predId) {
                BCM_FABRIC_PREDICATE_VECTOR_SET(predActInfo.mask, index);
                BCM_FABRIC_PREDICATE_VECTOR_SET(predActInfo.data, index);
            } else if ((_SIRIUS_E_PRED_ALL_MC +
                        SIRIUS_PREDICATE_OFFSET_INGRESS) == index) {
                BCM_FABRIC_PREDICATE_VECTOR_SET(predActInfo.mask, index);
                if (multicast) {
                    BCM_FABRIC_PREDICATE_VECTOR_SET(predActInfo.data, index);
                } else {
                    BCM_FABRIC_PREDICATE_VECTOR_CLR(predActInfo.data, index);
                }
            } else {
                BCM_FABRIC_PREDICATE_VECTOR_CLR(predActInfo.mask, index);
                BCM_FABRIC_PREDICATE_VECTOR_CLR(predActInfo.data, index);
            }
       } /* for (all supported predicate IDs) */
        result = bcm_sirius_fabric_predicate_action_create(unit,
                                                           &predActInfo,
                                                           &predActId);
    }
    if (BCM_E_NONE == result) {
        sir->userPredicate[steer_id - 1] = predId + 1;
        sir->userAction[steer_id - 1] = actId + 1;
        sir->userPredAction[steer_id - 1] = predActId + 1;
    } else { /* if (SOC_E_NONE == result) */
        if (restoreQueueSeg) {
            auxRes = bcm_sirius_fabric_qsel_destroy(unit, queueSeg);
        }
        if (BCM_E_NONE == auxRes) {
            if (restorePredicate) {
                auxRes = bcm_sirius_fabric_predicate_destroy(unit, predId);
            }
            if (BCM_E_NONE == auxRes) {
                if (restoreAction) {
                    auxRes = bcm_sirius_fabric_action_destroy(unit, actId);
                    COMPILER_REFERENCE(auxRes);
                }
            }
        }
    } /* if (SOC_E_NONE == result) */
exit:
    if (qso) {
        sal_free(qso);
    }
    if (qs) {
        sal_free(qs);
    }
    return result;
}


int
bcm_sirius_stk_steering_unicast_set(int unit,
                                    int steer_id,
                                    bcm_module_t destmod_lo,
                                    bcm_module_t destmod_hi,
                                    int num_queue_groups,
                                    bcm_gport_t *queue_groups)
{
    int result;

    result = _bcm_sirius_stk_steering_set(unit,
                                          steer_id,
                                          FALSE,
                                          destmod_lo,
                                          destmod_hi,
                                          num_queue_groups,
                                          queue_groups);
    return result;
}

int
bcm_sirius_stk_steering_multicast_set(int unit,
                                      int steer_id,
                                      bcm_multicast_t mgid_lo,
                                      bcm_multicast_t mgid_hi,
                                      int num_queue_groups,
                                      bcm_gport_t *queue_groups)
{
    int result;

    result = _bcm_sirius_stk_steering_set(unit,
                                          steer_id,
                                          TRUE,
                                          mgid_lo,
                                          mgid_hi,
                                          num_queue_groups,
                                          queue_groups);
    return result;
}

int
bcm_sirius_stk_steering_clear(int unit,
                              int steer_id)
{
    bcm_fabric_predicate_t predId;
    bcm_fabric_predicate_action_t predActId;
    bcm_fabric_action_t actId;
    bcm_fabric_action_info_t actInfo;
    soc_sbx_sirius_config_t *sir = SOC_SBX_CFG_SIRIUS(unit);

    if ((1 > steer_id) || (SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES <= steer_id)) {
        /* for some reason, invalid ID is usually 'not found' */
        return BCM_E_NOT_FOUND;
    }
    if (!(sir->userAction[steer_id - 1])) {
        /* this one is not in use, so it's already clear */
        return BCM_E_NONE;
    }
    actId = sir->userAction[steer_id - 1] - 1;
    /* need to get the qsel ID from the action */
    BCM_IF_ERROR_RETURN(bcm_sirius_fabric_action_get(unit,
                                                     actId,
                                                     &actInfo));
    /* unlink the action from the predicates */
    if (sir->userPredAction[steer_id - 1]) {
        /* there is a predicate->action mapping rule */
        predActId = sir->userPredAction[steer_id - 1] - 1;
        BCM_IF_ERROR_RETURN(bcm_sirius_fabric_predicate_action_destroy(unit,
                                                                       predActId));
        sir->userPredAction[steer_id - 1] = 0;
    }
    /* get rid of the predicate */
    if (sir->userPredicate[steer_id - 1]) {
        /* there is a predicate */
        predId = sir->userPredicate[steer_id - 1] - 1;
        BCM_IF_ERROR_RETURN(bcm_sirius_fabric_predicate_destroy(unit,
                                                                predId));
        sir->userPredicate[steer_id - 1] = 0;
    }
    /* get rid of the qsel */
    if (actInfo.qsel) {
        /* since qsel 0 means bypass and we don't do that, we have one */
        BCM_IF_ERROR_RETURN(bcm_sirius_fabric_qsel_destroy(unit,
                                                           actInfo.qsel));
    }
    /*
     *  If we get to here, and this next part fails, the state may be
     *  inconsistent.  Caller needs to retry the clear if that happens.
     */
    BCM_IF_ERROR_RETURN(bcm_sirius_fabric_action_destroy(unit, actId));
    sir->userAction[steer_id - 1] = 0;
    return BCM_E_NONE;;
}

int
bcm_sirius_stk_steering_clear_all(int unit)
{
    int result = BCM_E_NONE;
    int index;

    for (index = 1;
         (index < SB_FAB_DEVICE_SIRIUS_CONFIG_PREDICATES) &&
         (BCM_E_NONE == result);
         index++) {
        result = bcm_sirius_stk_steering_clear(unit, index);
    }
    return result;
}

int
bcm_sirius_stk_fabric_map_set(int unit,
                              bcm_gport_t switch_port,
                              bcm_gport_t fabric_port)
{
    bcm_port_t               fe_port, qe_port;
    int                      rv = BCM_E_NONE;
    int                      my_modid = -1, qe_mod = -1, fe_mod, idx = 0;
    int                      subport = 0, eg_n = 0, num_fifos = 4, node;
    ep_dest_port_map_entry_t ep_entry;
    bcm_sbx_subport_info_t *sp_info = NULL;
    tx_pfc_src_port_lkup_entry_t TxPFCSrcPortLutEntry;
    pfc_enq_src_port_lkup_entry_t QmPFCSrcPortLutEntry;
    int                      key, enable=FALSE;
    uint32 uRegValue;

    if ((switch_port >= 0) && (switch_port <= 15)) {
	if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    /* set mod/port as key */
	    if (BCM_GPORT_IS_MODPORT(fabric_port)) {
		fe_mod = BCM_GPORT_MODPORT_MODID_GET(fabric_port);
		fe_port = BCM_GPORT_MODPORT_PORT_GET(fabric_port);       
		key = (((fe_mod & 0xFF) << 8) | (fe_port & 0xFF));
		enable = TRUE;
	    } else if (fabric_port == BCM_GPORT_INVALID) {
		key = 0;
		enable = FALSE;
	    } else {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, 0x%x gport type not supported , unit %d\n"),
		           FUNCTION_NAME(), fabric_port, unit));
		return BCM_E_PARAM;
	    }

	    /* source port ID generation, care about all 16 bits */
	    SOC_IF_ERROR_RETURN(READ_TX_PFC_SRC_PORT_LKUP_CFGr(unit, &uRegValue));    
	    soc_reg_field_set(unit, TX_PFC_SRC_PORT_LKUP_CFGr, &uRegValue,
			      SOURCE_ID_MASKf, 0x0);	
	    SOC_IF_ERROR_RETURN(WRITE_TX_PFC_SRC_PORT_LKUP_CFGr(unit, uRegValue));

	    SOC_IF_ERROR_RETURN(READ_QM_PFC_ENQ_SRC_PORT_LKUP_CFGr(unit, &uRegValue));    
	    soc_reg_field_set(unit, QM_PFC_ENQ_SRC_PORT_LKUP_CFGr, &uRegValue,
			      SOURCE_ID_MASKf, 0x0);
	    SOC_IF_ERROR_RETURN(WRITE_QM_PFC_ENQ_SRC_PORT_LKUP_CFGr(unit, uRegValue));

	    SOC_IF_ERROR_RETURN(READ_TX_PFC_SRC_PORT_LKUPm(unit, MEM_BLOCK_ANY, switch_port, &TxPFCSrcPortLutEntry));
	    soc_mem_field32_set(unit, TX_PFC_SRC_PORT_LKUPm, &TxPFCSrcPortLutEntry, ENABLEf, enable?1:0);
	    soc_mem_field32_set(unit, TX_PFC_SRC_PORT_LKUPm, &TxPFCSrcPortLutEntry, KEYf, key);
	    SOC_IF_ERROR_RETURN(WRITE_TX_PFC_SRC_PORT_LKUPm(unit, MEM_BLOCK_ANY, switch_port, &TxPFCSrcPortLutEntry));

	    SOC_IF_ERROR_RETURN(READ_PFC_ENQ_SRC_PORT_LKUPm(unit, MEM_BLOCK_ANY, switch_port, &QmPFCSrcPortLutEntry));
	    soc_mem_field32_set(unit, PFC_ENQ_SRC_PORT_LKUPm, &QmPFCSrcPortLutEntry, ENABLEf, enable?1:0);
	    soc_mem_field32_set(unit, PFC_ENQ_SRC_PORT_LKUPm, &QmPFCSrcPortLutEntry, KEYf, key);
	    SOC_IF_ERROR_RETURN(WRITE_PFC_ENQ_SRC_PORT_LKUPm(unit, MEM_BLOCK_ANY, switch_port, &QmPFCSrcPortLutEntry));
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, (Source Port, PG) based PFC is not supported on unit %d\n"),
	               FUNCTION_NAME(), unit));	    
	    return BCM_E_UNAVAIL;
	}
    }

    if (BCM_GPORT_IS_MODPORT(fabric_port)) {
        qe_port = BCM_GPORT_MODPORT_PORT_GET(fabric_port);
        if (IS_CPU_PORT(unit, qe_port)) {
            fe_mod = BCM_GPORT_MODPORT_MODID_GET(switch_port);
            fe_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);

            rv = soc_sirius_rb_higig2_remote_cpu_config(unit, fe_mod, fe_port,
                                                        SOC_SIRIUS_API_PARAM_NO_CHANGE);
        }
    } else if (BCM_GPORT_IS_CHILD(fabric_port)  || 
	       BCM_GPORT_IS_EGRESS_CHILD(fabric_port) ||
	       BCM_GPORT_IS_EGRESS_GROUP(fabric_port)) {
	if (BCM_GPORT_IS_CHILD(fabric_port)) {
	    qe_mod = BCM_GPORT_CHILD_MODID_GET(fabric_port);
	    subport = BCM_GPORT_CHILD_PORT_GET(fabric_port); 
	} else if (BCM_GPORT_IS_EGRESS_CHILD(fabric_port)) {
	    qe_mod = BCM_GPORT_EGRESS_CHILD_MODID_GET(fabric_port);
	    subport = BCM_GPORT_EGRESS_CHILD_PORT_GET(fabric_port);
	} else {
	    qe_mod = BCM_GPORT_EGRESS_GROUP_MODID_GET(fabric_port);
	    subport = -1;
	    rv = bcm_sbx_cosq_egress_group_info_get(unit, fabric_port, &subport, &eg_n, &num_fifos);
	    if (rv != BCM_E_NONE) {
		LOG_ERROR(BSL_LS_BCM_COMMON,
		          (BSL_META_U(unit,
		                      "ERROR: %s, Egress Group 0x%x does not contain fabric_port, unit %d\n"),
		           FUNCTION_NAME(), fabric_port, unit));
		return BCM_E_PARAM;
	    }
	}

	rv = bcm_sirius_stk_my_modid_get(unit, &my_modid);
	if (rv != BCM_E_NONE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, unable to retrieve modid\n"),
	               FUNCTION_NAME()));
	    return BCM_E_INTERNAL;
	}

	if (my_modid != qe_mod) {
	    return BCM_E_NONE;
	}

	sp_info = &(SOC_SBX_STATE(unit)->port_state->subport_info[subport]);
	if (sp_info->valid == FALSE) {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, invalid fabric_port %d, unit %d\n"),
	               FUNCTION_NAME(), subport, unit));
	    return BCM_E_PARAM;
	}

	idx = sp_info->egroup[eg_n].es_scheduler_level0_node;
	for (qe_port = idx; qe_port < (idx + num_fifos); qe_port++) {
	    /* skip if idx is invalid */
	    if ( qe_port > SOC_MEM_INFO(unit, FIFO_MAP_TABLEm).index_max) {
		return BCM_E_NONE;
	    }
	    
            fe_mod = BCM_GPORT_MODPORT_MODID_GET(switch_port);
            fe_port = BCM_GPORT_MODPORT_PORT_GET(switch_port);
		
            /*
             * Configure EP port mapping info
             */
            sal_memset(&ep_entry, 0, sizeof(ep_entry));
            if (SOC_SBX_CFG(unit)->uInterfaceProtocol == SOC_SBX_IF_PROTOCOL_SBX) {
                soc_mem_field32_set(unit, EP_DEST_PORT_MAPm, &ep_entry, QUEUEf, (fe_port) << 8);
            } else {
                soc_mem_field32_set(unit, EP_DEST_PORT_MAPm, &ep_entry, QUEUEf, (fe_mod << 8) | fe_port);
            }
	    SOC_IF_ERROR_RETURN(WRITE_EP_DEST_PORT_MAPm(unit, MEM_BLOCK_ANY, qe_port, &ep_entry));
	}
    }

    if (sp_info == NULL) {
	return rv;
    }

    if (BCM_GPORT_IS_MODPORT(sp_info->parent_gport)) {
	node = BCM_GPORT_MODPORT_MODID_GET(sp_info->parent_gport) - BCM_MODULE_FABRIC_BASE;
    } else {
	node = BCM_GPORT_EGRESS_MODPORT_MODID_GET(sp_info->parent_gport) - BCM_MODULE_FABRIC_BASE;
    }

    bcm_sirius_cosq_module_congestion_set(unit, sp_info->parent_gport, node);

    return rv;
}

int
bcm_sirius_stk_fabric_map_get(int unit,
                              bcm_gport_t switch_port,
                              bcm_gport_t *fabric_port)
{
    tx_pfc_src_port_lkup_entry_t TxPFCSrcPortLutEntry;
    int                      key, enable=FALSE;
    int                      rv = BCM_E_NONE;

    if ((switch_port >= 0) && (switch_port <= 15)) {
	if (soc_feature(unit, soc_feature_source_port_priority_flow_control)) {
	    /* source port ID generation, care about all 16 bits */
	    SOC_IF_ERROR_RETURN(READ_TX_PFC_SRC_PORT_LKUPm(unit, MEM_BLOCK_ANY, switch_port, &TxPFCSrcPortLutEntry));
	    enable = soc_mem_field32_get(unit, TX_PFC_SRC_PORT_LKUPm, &TxPFCSrcPortLutEntry, ENABLEf);
	    key = soc_mem_field32_get(unit, TX_PFC_SRC_PORT_LKUPm, &TxPFCSrcPortLutEntry, KEYf);

	    if (enable == 0) {
		*fabric_port = BCM_GPORT_INVALID;
	    } else {
		BCM_GPORT_MODPORT_SET(*fabric_port, (key >> 8) & 0xFF, key & 0xFF);
	    }
	} else {
	    LOG_ERROR(BSL_LS_BCM_COMMON,
	              (BSL_META_U(unit,
	                          "ERROR: %s, (Source Port, PG) based PFC is not supported on unit %d\n"),
	               FUNCTION_NAME(), unit));	    
	    return BCM_E_UNAVAIL;
	}
    }

    return rv;
}
