/*
 * $Id: stat.h,v 1.9 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        stat.h
 * Purpose:     SBX internal stats definitions.
 */

#ifndef   _BCM_INT_SBX_STAT_H_
#define   _BCM_INT_SBX_STAT_H_

#include <bcm/types.h>
#include <shared/idxres_afl.h>
#include <bcm_int/control.h>

/* The REG_* macros require the following declaration in any function which
 * uses them:
 */

#define REG_MATH_DECL \
        uint64 reg_val

#define REG_ADD(unit, port, reg, val)					   \
    if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {	   \
	SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, reg,               \
                                            0, &reg_val));                 \
	COMPILER_64_ADD_64(val, reg_val);				   \
    }
#define REG_ADD_IDX(unit, port, reg, val, ar_idx)		     	   \
    if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {	   \
	SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, reg,               \
                                            ar_idx, &reg_val));            \
	COMPILER_64_ADD_64(val, reg_val);				   \
    }
#define REG_SUB(unit, port, reg, val)					   \
    if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {	   \
	SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, reg,               \
                                            0, &reg_val));                 \
        if (COMPILER_64_GT(val, reg_val)) {                                \
	    COMPILER_64_SUB_64(val, reg_val);				   \
        } else {                                                           \
            COMPILER_64_ZERO(val);                                         \
        }                                                                  \
    }
#define REG_SUB_IDX(unit, port, reg, val, ar_idx)			   \
    if (SOC_REG_IS_VALID(unit, reg) && SOC_REG_IS_COUNTER(unit, reg)) {	   \
	SOC_IF_ERROR_RETURN(soc_counter_get(unit, port, reg,               \
                                            ar_idx, &reg_val));            \
	COMPILER_64_SUB_64(val, reg_val);				   \
    }

/*
 * Control Structures.
 */
typedef struct bcm_stat_sbx_info_s {
    int                         segment;
    uint32                      ejectRate;
    shr_aidxres_list_handle_t   pList;
    shr_aidxres_element_t       first;
    shr_aidxres_element_t       last;
} bcm_stat_sbx_info_t;

typedef struct bcm_stat_info_s {
    int                     init;       /* TRUE if STAT module has been initialized */
    bcm_stat_sbx_info_t    *segInfo;
} bcm_stat_info_t;

#define     SBX_DASA_COUNTERS_PER_ENTRY     2

extern bcm_stat_info_t stat_info[BCM_MAX_NUM_UNITS];

#define STAT_CNTL(unit) stat_info[(unit)]

#define STAT_CHECK_INIT(unit)                                          \
    do {                                                               \
        if (!BCM_UNIT_VALID(unit)) return BCM_E_UNIT;                  \
        if (unit > (BCM_MAX_NUM_UNITS - 1)) return BCM_E_UNIT;         \
        if (STAT_CNTL(unit).init == FALSE) return BCM_E_INIT;          \
        if (STAT_CNTL(unit).init != TRUE) return STAT_CNTL(unit).init; \
    } while (0);

#define STAT_CHECK_PORT(unit, port)                                        \
    if (!SOC_PORT_VALID((unit), (port)) || !IS_E_PORT((unit), (port))) \
        return BCM_E_PORT;

#define STAT_CHECK_STAT(type)                                       \
    if ( ((type) < snmpIfInOctets) || ((type) >= snmpValCount) )    \
        return BCM_E_BADID;

#define STAT_CHECK_VLAN(unit, vlan)                                 \
    if ( ((BCM_VLAN_MAX <= vlan) || (0 == vlan)) )             \
        return BCM_E_PARAM;

/* Only allowed to set stats to zero */
#define STAT_CHECK_STAT_VALUE(val)          \
    if (0 < (val)) return BCM_E_PARAM;

#define STAT_CHECK_STAT_VALUE64(val)          \
    if (!COMPILER_64_IS_ZERO(val)) return BCM_E_PARAM;

#if 0
#define STAT_CHECK_VLAN_STAT(type) \
    if ( ((type) < bcmVlanStatUnicastPackets) || ((type) > bcmVlanStatCount) ) { \
        return BCM_E_BADID; \
    }
#endif

/*
 * Function:
 *      _bcm_fe2000_stat_block_init
 * Description:
 *      Initialize a statistics block.
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      pCtlBlock - pointer to a control structure defining params for the
 *                  block. If NULL, use default.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_XXXX      - Failure
 */
int _bcm_fe2000_g2p2_stat_block_init(int unit, int type,
                                     bcm_stat_sbx_info_t *pCtlBlock);

/*
 * Function:
 *      _bcm_fe2000_stat_block_alloc
 * Description:
 *      Allocate counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      count     - number of counters required
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_fe2000_stat_block_alloc(int unit,
                                 int type,
                                 shr_aidxres_element_t *start,
                                 shr_aidxres_element_t count);

/*
 * Function:
 *      _bcm_fe2000_stat_block_free
 * Description:
 *      Free counters from a statistics block
 * Parameters:
 *      unit      - device unit number.
 *      type      - one of the defined segment types
 *      start     - (OUT) where to put first of allocated counter block
 * Returns:
 *      BCM_E_NONE      - Success
 *      BCM_E_XXXX      - Failure
 */
int _bcm_fe2000_stat_block_free(int unit,
                                int type,
                                shr_aidxres_element_t start);

#endif /* _BCM_INT_SBX_STAT_H_ */
