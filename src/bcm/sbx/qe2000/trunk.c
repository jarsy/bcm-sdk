/*
 * $Id: trunk.c,v 1.24 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    trunk.c
 * Purpose: BCM level APIs for Link Aggregation (a.k.a Trunking)
 */

#define SBX_HASH_DEFINED 0
#include <shared/bsl.h>

#include <soc/drv.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/sbStatus.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/trunk.h>
#include <bcm/vlan.h>
#include <bcm/stack.h>

#include <bcm_int/control.h>
#include <bcm_int/common/lock.h>
#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/trunk.h>
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/qe2000.h>
#include <bcm_int/sbx_dispatch.h>

typedef struct trunk_private_s {
    bcm_trunk_t     trunk_id;       /* trunk group ID */
    int             in_use;         /* whether particular trunk is in use */
    int             num_ports;      /* Number of ports in the trunk group */
    int             dlf_index;
    int             mc_index;
    bcm_port_t      tp[BCM_TRUNK_MAX_PORTCNT];    /* Ports in trunk */
    bcm_module_t    tm[BCM_TRUNK_MAX_PORTCNT];    /* Modules per port */
} trunk_private_t;

typedef struct trunk_cntl_s {
    int                 init;       /* TRUE if module has been initialized */
    sal_mutex_t         lock;       /* Database lock */
    int                 ngroups;    /* max number of trunk groups */
    int                 nports;     /* max port count per trunk group */
    int                 psc;        /* port spec criterion (a.k.a. hash algorithm) */
    trunk_private_t    *t_info;
} trunk_cntl_t;

/*
 * One trunk control entry for each SOC device containing trunk book keeping
 * info for that device.
 */
static trunk_cntl_t bcm_trunk_control[BCM_MAX_NUM_UNITS];

#define TRUNK_MIN_MEMBERS   0   /* Minimum number of ports in a trunk */

#define TRUNK_CNTL(unit)        bcm_trunk_control[unit]
#define TRUNK_INFO(unit, tid)   bcm_trunk_control[unit].t_info[tid]

#define	BCM_TRUNK_UNIT_VALID(_unit) (((_unit) >= 0) &&			\
				     ((_unit) < BCM_MAX_NUM_UNITS))

#define TRUNK_DB_LOCK(unit)                                                 \
        do {                                                                \
            if (NULL != TRUNK_CNTL(unit).lock)                              \
                sal_mutex_take(TRUNK_CNTL(unit).lock, sal_mutex_FOREVER);   \
        } while (0);

#define TRUNK_DB_UNLOCK(unit)                           \
        do {                                            \
            if (NULL != TRUNK_CNTL(unit).lock)          \
                sal_mutex_give(TRUNK_CNTL(unit).lock);  \
        } while (0);
    
/*
 * Cause a routine to return BCM_E_INIT if trunking subsystem is not
 * initialized.
 */
#define TRUNK_CHECK_INIT(unit)                                 \
    do {                                                       \
	if (!BCM_TRUNK_UNIT_VALID(unit)) return BCM_E_UNIT;    \
        if (TRUNK_CNTL(unit).init == FALSE) return BCM_E_INIT; \
        if (TRUNK_CNTL(unit).init != TRUE)                     \
            return TRUNK_CNTL(unit).init;                      \
    } while (0);

/*
 * Make sure TID is within valid range.
 */
#define TRUNK_CHECK_TID(unit, tid) \
    if (((tid) < 0) || ((tid) >= TRUNK_CNTL(unit).ngroups)) \
        return BCM_E_BADID;
/*
 * TID is in range, check to make sure it is actually in use.
 */
#define TRUNK_TID_VALID(unit, tid)                          \
    (TRUNK_INFO((unit), (tid)).trunk_id != BCM_TRUNK_INVALID)

#define TRUNK_PORTCNT_VALID(unit, port_cnt)                  \
    ((port_cnt >= TRUNK_MIN_MEMBERS) && (port_cnt <= TRUNK_CNTL(unit).nports))


#define QE_SPI_SUBPORT_GET(funit, fport) ((fport) + SOC_PORT_MIN((funit), spi_subport))

static int        _ngroups          = SBX_MAX_TRUNKS;
static int        _nports           = BCM_TRUNK_MAX_PORTCNT;
    
/*
 * Fixed offsets
 */ 
/* total system ports including CPU */
#define TRUNK_SBX_HASH_SIZE             3
#define TRUNK_SBX_FIXED_PORTCNT         (1<<TRUNK_SBX_HASH_SIZE)
#define TRUNK_SBX_OFF_NODE_PORT         0x3F
#define TRUNK_INGLAG_ENTRY_DISABLED     0x3FFFF

#define TRUNK_INDEX_SET(tid, offset)                 \
    (TRUNK_SBX_FIXED_PORTCNT > (offset)) ?           \
     ((((tid))<<(TRUNK_SBX_HASH_SIZE)) | (offset)) : \
     SBX_INVALID_TRUNK

extern int
_bcm_qe2000_vlan_port_remove(int unit,
                             bcm_vlan_t vid,
                             bcm_pbmp_t pbmp);

extern int
_bcm_qe2000_vlan_port_add(int unit,
                          bcm_vlan_t vid,
                          bcm_pbmp_t pbmp,
                          bcm_pbmp_t ubmp);

static int
_bcm_qe2000_trunk_get_fabric_modport(int unit, bcm_module_t mod, bcm_port_t port,
                                          bcm_module_t *fabric_mod, bcm_port_t *fabric_port)
{
    int  rc = BCM_E_NONE;
    bcm_gport_t switch_gport;
    bcm_gport_t fabric_gport;


    if (BCM_STK_MOD_IS_NODE(mod)) {
        (*fabric_mod) = mod;
        (*fabric_port) = port;
        return(rc);
    }

    BCM_GPORT_MODPORT_SET(switch_gport, mod, port);
    rc = bcm_sbx_stk_fabric_map_get(unit, switch_gport, &fabric_gport);
    if (rc != BCM_E_NONE) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Invalid Module 0x%x, Port: %d\n"),
                   mod, port));
        return(rc);
    }

    (*fabric_mod) = BCM_GPORT_MODPORT_MODID_GET(fabric_gport);
    (*fabric_port) = BCM_GPORT_MODPORT_PORT_GET(fabric_gport);

    return(rc);
}

#ifdef BROADCOM_DEBUG

typedef struct recursive_mutex_s {
    sal_sem_t       sem;
    sal_thread_t    owner;
    int             recurse_count;
    char           *desc;
} recursive_mutex_t;


/*
 * Function:
 *    _bcm_qe2000_trunk_debug
 * Purpose:
 *      Displays trunk information maintained by software.
 * Parameters:
 *      unit - Device unit number
 * Returns:
 *      None
 */
void
_bcm_qe2000_trunk_debug(int unit)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    recursive_mutex_t  *lock;
    int                 index, jindex;
    int                 invIdx = 0;
    bcm_trunk_t         invalidList[SBX_MAX_TRUNKS];

    if (!BCM_TRUNK_UNIT_VALID(unit)) {
        return;
    }

    tc = &TRUNK_CNTL(unit);
    LOG_CLI(("--- Debug ---\n"));
    LOG_CLI(("\nSW Information TRUNK - Unit %d\n", unit));
    LOG_CLI(("  Initialized         : %s\n", tc->init?"True":"False"));
    LOG_CLI(("  Lock                : 0x%08X\n", (uint32)tc->lock));
    if (TRUE == tc->init) {
        lock = (recursive_mutex_t *)((int)tc->lock + 12);
        LOG_CLI(("    Desc              : %s\n", lock->desc));
        LOG_CLI(("    Owner             : 0x%08X\n", (uint32)lock->owner));
        LOG_CLI(("    Count             : %d\n", lock->recurse_count));
    }
    LOG_CLI(("  Trunk groups        : %d\n", tc->ngroups));
    LOG_CLI(("  Trunk max ports     : %d\n", tc->nports));
    LOG_CLI(("  Port Select Criteria: 0x%x\n", tc->psc));

    for (index = 0; index < tc->ngroups; index++) {
        ti = &TRUNK_INFO(unit, index);
        if (ti->trunk_id == BCM_TRUNK_INVALID) {
            invalidList[invIdx++] = index;
        } else {
            LOG_CLI(("\n  Trunk %d\n", index));
            LOG_CLI(("      ID              : %d\n", ti->trunk_id));
            LOG_CLI(("      in use          : %d\n", ti->in_use));
            LOG_CLI(("      number of ports : %d\n", ti->num_ports));
            LOG_CLI(("      dlf index       : %d\n", ti->dlf_index));
            LOG_CLI(("      mc index        : %d\n", ti->mc_index));
            if (0 < ti->num_ports) {
                LOG_CLI(("         ports        : %d:%d", ti->tm[0], ti->tp[0]));
                for (jindex = 1; jindex < ti->num_ports; jindex++)
                    LOG_CLI((",  %d:%d", ti->tm[jindex], ti->tp[jindex]));
                LOG_CLI(("\n"));
            }
        }
    }
    LOG_CLI(("\nUnused Trunks: "));
    for (index = 0; index < invIdx; index++) {
        LOG_CLI(("%d ", invalidList[index]));
    }
    LOG_CLI(("\n\n"));

    return;
}
#endif /* BROADCOM_DEBUG */


/*
 * Function:
 *    _bcm_qe2000_trunk_set
 * Purpose:
 *      Add ports to a trunk group.
 * Parameters:
 *      unit       - Device unit number.
 *      tid        - The trunk ID to be affected.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_PARAM     - Invalid ports specified.
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      the following fields of the bcm_trunk_add_info_t structure are ignored
 *      on SBX:
 *          flags
 *          dlf_index
 *          mc_index
 *          ipmc_index
 */
int
_bcm_qe2000_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info)
{
    trunk_private_t    *ti;
    bcm_port_t          port;
    bcm_module_t        module, my_module_id;
    bcm_trunk_t         test_tid;
    trunk_private_t     removed;
    int                 index;
    int                 tableIndex;
    int                 result = BCM_E_NONE;
    pbmp_t              aiPbmp;             /* addr_info port bitmap */
    pbmp_t              removedPbmp;            /* removed   port bitmap */
    pbmp_t              wPbmp;              /* working   port bitmap */
    bcm_port_t          desPort  = SBX_INVALID_PORT;
    int                 desIndex =  0;
    bcm_vlan_data_t    *listp;
    int                 count;
    char                pfmt[SOC_PBMP_FMT_LEN];
    trunk_private_t     *cur_config = NULL;
    trunk_private_t     *req_config = NULL;
    sbBool_t             isDesignateConfig = FALSE;
    pbmp_t               DesignatePbmp;


    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, [n=%d - "),
               FUNCTION_NAME(), unit, tid, add_info->num_ports));
    for (index = 0; index < add_info->num_ports; index++) {
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              " %d:%d"),
                   add_info->tm[index], add_info->tp[index]));
    }
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "]) - Enter\n")));

    /* initialization */
    cur_config = sal_alloc(sizeof(trunk_private_t), "Trunk-prev");
    if (cur_config == NULL) {
        result = BCM_E_MEMORY;
        goto err;
    }
    req_config = sal_alloc(sizeof(trunk_private_t), "Trunk-cur");
    if (req_config == NULL) {
        result = BCM_E_MEMORY;
        goto err;
    }
    sal_memset(&removed, 0, sizeof(trunk_private_t));
    sal_memset(cur_config, 0, sizeof(trunk_private_t));
    sal_memset(req_config, 0, sizeof(trunk_private_t));

    ti = &TRUNK_INFO(unit, tid);

    /* convert current state to fabric context */
    (*cur_config) = (*ti);
    for (index = 0; index < ti->num_ports; index++) {
        result = _bcm_qe2000_trunk_get_fabric_modport(unit, ti->tm[index], ti->tp[index],
                                          &cur_config->tm[index], &cur_config->tp[index]);
        if (result != BCM_E_NONE) {
            goto err;
        }
    }

    /* make a copy of the  requested mod/port configuration */
    req_config->num_ports = add_info->num_ports;
    sal_memcpy(req_config->tm, add_info->tm, sizeof(bcm_module_t) * BCM_TRUNK_MAX_PORTCNT);
    sal_memcpy(req_config->tp, add_info->tp, sizeof(bcm_port_t) * BCM_TRUNK_MAX_PORTCNT);

    /* start from a state were all current ports have been removed from the Trunk group. */
    removed.num_ports = cur_config->num_ports;
    sal_memcpy(removed.tm, cur_config->tm, sizeof(bcm_module_t) * BCM_TRUNK_MAX_PORTCNT);
    sal_memcpy(removed.tp, cur_config->tp, sizeof(bcm_port_t) * BCM_TRUNK_MAX_PORTCNT);

    /* retreive current module id */ 
    bcm_qe2000_stk_modid_get(unit, &my_module_id);
 
    /* find designate port's index. If not specified, use index 0. Designate  */
    /* port may or may not be on this QE. If it is not, the MVT entry will be */
    /* adjusted to remove ALL lag ports and that is OK.                       */
    desIndex = 0;
    if ((add_info->dlf_index >= 0) && (add_info->dlf_index < add_info->num_ports)) {
        desIndex = add_info->dlf_index;
    }
    else if ((add_info->mc_index >= 0 ) && (add_info->mc_index < add_info->num_ports)) {
        desIndex = add_info->mc_index;
    }

    /* - Consistency checks                                              */
    /*   - Make sure the ports/modules supplied are valid.               */
    /*   - Make sure the ports don't belong to a different trunk.        */
    /* - Build list of ports that have been removed from the trunk group */
    /* NOTE: Ideally these checks should be moved into common code       */
    if (req_config->num_ports > BCM_TRUNK_MAX_PORTCNT) {
        LOG_ERROR(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Invalid number of ports %d, Max Allowed: %d\n"),
                   req_config->num_ports, BCM_TRUNK_MAX_PORTCNT));
        result = BCM_E_PARAM;
        goto err;
    }
    for (index = 0; index < req_config->num_ports; index++) {
        result = _bcm_qe2000_trunk_get_fabric_modport(unit,
                                          req_config->tm[index], req_config->tp[index],
                                          &req_config->tm[index], &req_config->tp[index]);
        if (result != BCM_E_NONE) {
            goto err;
        }

        /* valid module */
        module = req_config->tm[index];
        if (!BCM_STK_MOD_IS_NODE(module)) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "Invalid Module 0x%x, Index: %d\n"),
                       module, index));
            result = BCM_E_PARAM;
            goto err;
        }

        /* valid port */
        port = req_config->tp[index];
        if (port < 0) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "Invalid Port: 0x%x on Module: 0x%x, Index: %d\n"),
                       port, module, index));
            result = BCM_E_PARAM;
            goto err;
        }
        if (module == my_module_id) {
            if (!(((port >= SOC_PORT_MIN(unit, spi_subport)) &&
                  (port <= SOC_PORT_MAX(unit, spi_subport))) || (port == CMIC_PORT(unit)))) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Invalid Port: 0x%x on Module: 0x%x, Index: %d\n"),
                           port, module, index));
                result = BCM_E_PARAM;
                goto err;
            }
        }

        /* mod:port not already present in some other trunk */
        result = bcm_trunk_find(unit, module, port, &test_tid);
        if (result == BCM_E_NONE) {     /* mod:port was found to exist in a trunk */
            if (tid == test_tid) {      /* mod:port is in this trunk */

                /* take this mod:port out of the list of ports that were removed */
                for (tableIndex = 0; tableIndex < removed.num_ports; tableIndex++) {
                    if ((port == removed.tp[tableIndex]) && (module == removed.tm[tableIndex])) {
                        removed.num_ports -= 1;
                        removed.tp[tableIndex] = removed.tp[removed.num_ports];
                        removed.tm[tableIndex] = removed.tm[removed.num_ports];
                        removed.tp[removed.num_ports] = SBX_INVALID_PORT;
                        removed.tm[removed.num_ports] = SBX_INVALID_MODID;
                        break;
                    }
                }
            }
            else {                      /* mod:port is in another trunk */
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "Port %d:%d exists in trunk %d\n"),
                           module, port, test_tid));
                result = BCM_E_PARAM;
                goto err;
            }
        }
        else if (result != BCM_E_NOT_FOUND) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "error bcm_trunk_find module: 0x%x, port: 0x%x, Err(0x%x, %s)\n"),
                       module, port, result, bcm_errmsg(result)));
            goto err;
        }
    }

    if(req_config->num_ports == 0) {
        sal_memset(&removed, 0, sizeof(trunk_private_t));
        for (tableIndex = 0; tableIndex < cur_config->num_ports; tableIndex++) {
            if (cur_config->tm[tableIndex] == my_module_id) {
                removed.tp[removed.num_ports] = cur_config->tp[tableIndex];
                removed.tm[removed.num_ports] = cur_config->tm[tableIndex];
                removed.num_ports += 1;
                removed.tp[removed.num_ports] = SBX_INVALID_PORT;
                removed.tm[removed.num_ports] = SBX_INVALID_MODID;
            }
        }
    }

    /* Update the the list of ports removed to pertain only to the current unit. */ 
    for (tableIndex = 0; tableIndex < removed.num_ports; tableIndex++) {
        if (removed.tm[tableIndex] != my_module_id) {
            removed.num_ports -= 1;
            removed.tp[tableIndex] = removed.tp[removed.num_ports];
            removed.tm[tableIndex] = removed.tm[removed.num_ports];
            removed.tp[removed.num_ports] = SBX_INVALID_PORT;
            removed.tm[removed.num_ports] = SBX_INVALID_MODID;
        }
    }

    if (result == BCM_E_NOT_FOUND) {
        result = BCM_E_NONE;
    }
    if (result != BCM_E_NONE) {
      /* coverity[dead_error_line] */
        goto err;
    }

    if (removed.num_ports != 0) {
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "Ports removed: [n=%d - "),
                   removed.num_ports));
        for (index = 0; index < removed.num_ports; index++) {
            LOG_DEBUG(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  " %d:%d"),
                       removed.tm[index], removed.tp[index]));
        }
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "])\n")));
    }

    /* first repair VLAN MVT entries for ports being removed from trunk */
    if (removed.num_ports != 0) {
        BCM_PBMP_CLEAR(removedPbmp);

        /* convert tm/tp to port bitmap for qe2000 */
        for (index = 0; index < removed.num_ports; index++) {
            BCM_PBMP_PORT_ADD(removedPbmp, removed.tp[index]);
        }

        result = bcm_vlan_list_by_pbmp(unit, removedPbmp, &listp, &count);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "error bcm_vlan_list_by_pbmp unit: 0x%x, Err(0x%x, %s)\n"),
                       unit, result, bcm_errmsg(result)));
            goto err;
        }

        for (index = 0; index < count; index++) {
            BCM_PBMP_AND(listp[index].port_bitmap, removedPbmp);
            BCM_PBMP_AND(listp[index].ut_port_bitmap, removedPbmp);
            result = _bcm_qe2000_vlan_port_add(unit, listp[index].vlan_tag,
                                                   listp[index].port_bitmap,
                                                   listp[index].ut_port_bitmap);
            if (result != BCM_E_NONE) {
                LOG_ERROR(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "error _bcm_qe2000_vlan_port_add unit: 0x%x, vlan: 0x%x, Err(0x%x, %s)\n"),
                           unit, listp[index].vlan_tag, result, bcm_errmsg(result)));
                break;
            }
            else {
                SOC_PBMP_FMT(listp[index].port_bitmap, pfmt);
                LOG_DEBUG(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "\tTo VLAN %d, Ports %s added back.\n"),
                           listp[index].vlan_tag, pfmt));
            }
        }
        (void)bcm_vlan_list_destroy(unit, listp, count);
    }

    if (result != BCM_E_NONE) {
        goto err;
    }


    /* At this point, MVT entries are repaired for removed ports.    */
    /* Now VLAN MVT entries should be fixed up for trunking. Namely  */
    /* all but a single trunk port must be removed from the entries. */
    desPort = SBX_INVALID_PORT;
    BCM_PBMP_CLEAR(aiPbmp);
    for (index = 0; index < req_config->num_ports; index++) {
        if (req_config->tm[index] != my_module_id) {
            continue;
        }

        BCM_PBMP_PORT_ADD(aiPbmp, req_config->tp[index]);
        if (index == desIndex) {
            desPort = req_config->tp[index];
        }
    }

    if (BCM_PBMP_NOT_NULL(aiPbmp)) {
        result = bcm_vlan_list_by_pbmp(unit, aiPbmp, &listp, &count);
        if (result != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "error bcm_vlan_list_by_pbmp unit: 0x%x, Err(0x%x, %s)\n"),
                       unit, result, bcm_errmsg(result)));
            goto err;
        }

        for (index = 0; index < count; index++) {
            BCM_PBMP_ASSIGN(wPbmp, listp[index].port_bitmap);
            BCM_PBMP_AND(wPbmp, aiPbmp);

            /* remove all common ports */
            if (BCM_PBMP_NOT_NULL(wPbmp)) {

                /* common ports found - this vlan needs to be tweaked.   */
                /* wPbmp contains the list of common ports to be         */
                /* removed. It may need to be reduced for the            */
                /* designate port, i.e. dlf_index or mc_index (desPort). */
                if (desPort != SBX_INVALID_PORT) {

                    /* This indicates that the designate port is on this */
                    /* unit. Make sure it is in this VLAN. If it is not, */
                    /* another designate must be selected for this VLAN. */
                    if (BCM_PBMP_MEMBER(wPbmp, desPort)) {
                        BCM_PBMP_PORT_REMOVE(wPbmp, desPort);
                        isDesignateConfig = TRUE;
                    }
                    else {
                        /* Commented it since the vlan and trunk APIs have to   */
                        /* be invoked There is no sequencing across trunk and   */
                        /* vlan APIs. The system  will be in a consistent state */
                        /* once trunk and vlan configuration is done.           */         
#if 0
                        /* just select the first on found ... */
                        BCM_PBMP_ITER(wPbmp, port) break;
                        BCM_PBMP_PORT_REMOVE(wPbmp, port);
#endif /* 0 */
                    }
                }
                result = _bcm_qe2000_vlan_port_remove(unit, listp[index].vlan_tag, wPbmp);
                if (result != BCM_E_NONE) {
                    LOG_ERROR(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "error _bcm_qe2000_vlan_port_remove unit: 0x%x, vlan: 0x%x, Err(0x%x, %s)\n"),
                               unit, listp[index].vlan_tag, result, bcm_errmsg(result)));
                    break;
                }
                else {
                    SOC_PBMP_FMT(wPbmp, pfmt);
                    LOG_DEBUG(BSL_LS_BCM_TRUNK,
                              (BSL_META_U(unit,
                                          "\tTo VLAN %d, Removed ports %s.\n"),
                               listp[index].vlan_tag, pfmt));
                }

                /* make sure the designated port is configured */
                if (isDesignateConfig == TRUE) {
                    isDesignateConfig = FALSE;
                    BCM_PBMP_CLEAR(DesignatePbmp);
                    BCM_PBMP_PORT_ADD(DesignatePbmp, desPort);
                    BCM_PBMP_AND(listp[index].port_bitmap, DesignatePbmp);
                    BCM_PBMP_AND(listp[index].ut_port_bitmap, DesignatePbmp);
                    result = _bcm_qe2000_vlan_port_add(unit, listp[index].vlan_tag,
                                                   listp[index].port_bitmap,
                                                   listp[index].ut_port_bitmap);
                    if (result != BCM_E_NONE) {
                        LOG_ERROR(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "error _bcm_qe2000_vlan_port_add unit: 0x%x, vlan: 0x%x, Err(0x%x, %s)\n"),
                                   unit, listp[index].vlan_tag, result, bcm_errmsg(result)));
                        break;
                    }
                    else {
                        SOC_PBMP_FMT(listp[index].port_bitmap, pfmt);
                        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                                  (BSL_META_U(unit,
                                              "\tTo VLAN %d, Ports %s added back.\n"),
                                   listp[index].vlan_tag, pfmt));
                    }
                }

            }
        }

        (void)bcm_vlan_list_destroy(unit, listp, count);
    }
    
    if (result != BCM_E_NONE) {
        goto err;
    }

    /* update trunk state */
    ti->num_ports   = add_info->num_ports;
    ti->dlf_index = SBX_INVALID_PORT;
    ti->mc_index = SBX_INVALID_PORT;
    if ((add_info->dlf_index >= 0) && (add_info->dlf_index < add_info->num_ports)) {
        ti->dlf_index = add_info->dlf_index;
    }
    if ((add_info->mc_index >= 0) && (add_info->mc_index < add_info->num_ports)) {
        ti->mc_index = add_info->mc_index;
    }
    else {
        ti->dlf_index = 0;
    }
    for (index = 0; index < add_info->num_ports; index++) {
        ti->tm[index] = add_info->tm[index];
        ti->tp[index] = add_info->tp[index];
    }

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, *) - Exit(%d: %s)\n"), 
               FUNCTION_NAME(), unit, tid, result, bcm_errmsg(result)));

    sal_free(cur_config);
    sal_free(req_config);

    return(result);

err:
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, *) - Exit(%d: %s)\n"), 
               FUNCTION_NAME(), unit, tid, result, bcm_errmsg(result)));
    if (cur_config != NULL) {
        sal_free(cur_config);
    }
    if (req_config != NULL) {
        sal_free(req_config);
    }
    return(result);
}

/*
 *  Function
 *    bcm_qe2000_trunk_vlan_port_adjust
 * Purpose
 *    Adjust ports in a VLAN so that if there are any ports from an aggregate,
 *    only the designate for that aggregate will show up, and if there are no
 *    ports from an aggregate, nothing will show up from that aggregate.
 *  Arguments
 *    (in) int unit = the unit number on which to operate
 *    (in) bcm_vlan_t vid = the VLAN on which to operate
 *    (in) bcm_pbmp_t ports = the ports now in the VLAN (logical)
 *  Return
 *    bcm_error_t cast as int
 *      BCM_E_NONE if success
 *      BCM_E_INIT if unit not initialised
 *      BCM_E_* otherwise as appropriate
 *  Notes
 *    The 'ports' pbmp MUST be ALL ports that are LOGICALLY in this VLAN on
 *    this unit, and must include ports that may not appear in the hardware
 *    table, such as non-designate aggregate members.  This function will
 *    ensure non-designate aggregate members are removed from hardware, and
 *    will also ensure that designate aggregate members do appear in the
 *    hardware if there is at least one member of the aggregate logically
 *    participating in the VLAN.
 */

extern int
bcm_qe2000_trunk_vlan_port_adjust(const int unit,
                                 const bcm_vlan_t vid,
                                 const bcm_pbmp_t pbmp)
{
    int result;
    int tempRes;
    trunk_private_t ti;
    bcm_port_t port;
    bcm_pbmp_t pbmpRemove;
    bcm_pbmp_t pbmpAdd;
    bcm_trunk_t tid;
    bcm_module_t myModId;
    int desIndex;
    unsigned int index;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s[%d] %s(%d, %d, *) - Enter\n"),
                 __FILE__,
                 __LINE__,
                 FUNCTION_NAME(),
                 unit,
                 vid));

    TRUNK_CHECK_INIT(unit);

    result = bcm_qe2000_stk_modid_get(unit, &myModId);
    if (BCM_E_NONE != result) {
        return result;
    }

    TRUNK_DB_LOCK(unit);

    /*
     *  Basically, we start out assuming that only logically present ports
     *  should be in the VLAN.  This means that we remove anything not
     *  logically included.  We'll adjust for physical exclusions and
     *  inclusions as we encounter aggregates in which the logically present
     *  ports are participating.
     */
    BCM_PBMP_ASSIGN(pbmpRemove, PBMP_SPI_SUBPORT_ALL(unit));
    BCM_PBMP_OR(pbmpRemove, PBMP_CMIC(unit));
    BCM_PBMP_REMOVE(pbmpRemove, pbmp);
    /*
     *  Now pbmpRemove is all ports that are not logically in the VLAN,
     *  basically all the ports that logically should not be in the hardware.
     *
     *  We need to adjust this so we don't include ports that are aggregate
     *  members, unless they're the designate port.  We don't care about
     *  aggregates that don't belong to this VLAN, so we only look at the
     *  logically present ports when searching for aggregates.
     */
    BCM_PBMP_CLEAR(pbmpAdd);
    BCM_PBMP_ITER(pbmp, port) {
        tempRes = bcm_qe2000_trunk_find(unit,
                                       BCM_STK_NODE_TO_MOD(SOC_SBX_CONTROL(unit)->node_id),
                                       port,
                                       &tid);
        if (BCM_E_NONE != tempRes) {
            /* no aggregate for this port; skip it */
            continue;
        }

        /* get current state and ensure it is using fabric context */
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              "%s[%d] %s: found aggregate %d in VLAN %03X\n"),
                   __FILE__,
                   __LINE__,
                   FUNCTION_NAME(),
                   tid,
                   vid));
        ti = TRUNK_INFO(unit, tid);
        for (index = 0;
             (index < ti.num_ports) && (BCM_E_NONE == tempRes);
             index++) {
            tempRes = _bcm_qe2000_trunk_get_fabric_modport(unit,
                                                           ti.tm[index],
                                                           ti.tp[index],
                                                           &(ti.tm[index]),
                                                           &(ti.tp[index]));
        }
        if (BCM_E_NONE != tempRes) {
            result = tempRes;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "%s[%d] %s: unable to get unit %d trunk %d member"
                                   " %d (mod %d:port %d) info: %d (%s)\n"),
                       __FILE__,
                       __LINE__,
                       FUNCTION_NAME(),
                       unit,
                       tid,
                       index,
                       ti.tm[index],
                       ti.tp[index],
                       result,
                       _SHR_ERRMSG(result)));
            /*
             *  We will try to continue even after this condition, on the basis
             *  that it is better to at least get some of the aggregates
             *  correct than none, but will return the last error.
             */
        } else { /* if (BCM_E_NONE != tempRes) */
            /* decide this aggregate's designate port */
            desIndex = 0;
            if ((ti.dlf_index >= 0) && (ti.dlf_index < ti.num_ports)) {
                desIndex = ti.dlf_index;
            } else if ((ti.mc_index >= 0 ) && (ti.mc_index < ti.num_ports)) {
                desIndex = ti.mc_index;
            }
            LOG_DEBUG(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "%s[%d] %s: using index %d as designate\n"),
                       __FILE__,
                       __LINE__,
                       FUNCTION_NAME(),
                       desIndex));
            /*
             *  Build pbmp of ports to remove (all ports in this aggregate on
             *  this module).
             */
            for (index = 0; index < ti.num_ports; index++) {
                if (ti.tm[index] == myModId) {
                    /*
                     *  It is on this module, so mark it for removal.  If it
                     *  was the designate port, we'll put it back later.
                     */
                    BCM_PBMP_PORT_ADD(pbmpRemove, ti.tp[index]);
                } /* if (this member is on this module) */
                LOG_DEBUG(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "%s[%d] %s: mod %d, port %d in aggregate "
                                       "(%s (%s%s\n"),
                           __FILE__,
                           __LINE__,
                           FUNCTION_NAME(),
                           ti.tm[index],
                           ti.tp[index],
                           (myModId == ti.tm[index])?"local) ":"remote)",
                           (myModId == ti.tm[index])?(BCM_PBMP_MEMBER(pbmp, ti.tp[index])?"present)":"absent) "):"unknown)",
                           (myModId == ti.tm[index])?"; removing":""));
            } /* for (all ports in this aggregate) */
            if (ti.tm[desIndex] == myModId) {
                /*
                 *  The designate port is on this module.  Ensure the designate
                 *  is included in the hardware ports, and remove it from the
                 *  non-hardware ports list (so we don't glitch it).
                 */
                BCM_PBMP_PORT_ADD(pbmpAdd, ti.tp[desIndex]);
                BCM_PBMP_PORT_REMOVE(pbmpRemove, ti.tp[desIndex]);
                LOG_DEBUG(BSL_LS_BCM_TRUNK,
                          (BSL_META_U(unit,
                                      "%s[%d] %s: local designate index %d,"
                                       " module %d, port %d; adding\n"),
                           __FILE__,
                           __LINE__,
                           FUNCTION_NAME(),
                           desIndex,
                           ti.tm[desIndex],
                           ti.tp[desIndex]));
            }
        } /* if (BCM_E_NONE != tempRes) */
    } /* BCM_PBMP_ITER(pbmp, port) */
    /* we need to change the hardware VLAN membership */
    if (BCM_PBMP_NOT_NULL(pbmpRemove)) {
        /* ensure non-designate ports are not included in hardware */
        tempRes = _bcm_qe2000_vlan_port_remove(unit, vid, pbmpRemove);
        if (BCM_E_NONE != tempRes) {
            result = tempRes;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "%s[%d] %s: unable to remove aggregate non-designate"
                                   " ports from unit %d VID %d: %d (%s)\n"),
                       __FILE__,
                       __LINE__,
                       FUNCTION_NAME(),
                       unit,
                       vid,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } /* if (BCM_PBMP_NOT_NULL(pbmpRemove)) */
    if (BCM_PBMP_NOT_NULL(pbmpAdd)) {
        /* ensure local designate ports are included in hardware */
        
        tempRes = _bcm_qe2000_vlan_port_add(unit, vid, pbmpAdd, pbmpAdd);
        if (BCM_E_NONE != tempRes) {
            result = tempRes;
            LOG_ERROR(BSL_LS_BCM_TRUNK,
                      (BSL_META_U(unit,
                                  "%s[%d] %s: unable to add aggregate designate"
                                   " ports to unit %d VID %d: %d (%s)\n"),
                       __FILE__,
                       __LINE__,
                       FUNCTION_NAME(),
                       unit,
                       vid,
                       result,
                       _SHR_ERRMSG(result)));
        }
    } /* if (BCM_PBMP_NOT_NULL(pbmpAdd)) */

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s[%d] %s(%d, %d, *) - Exit(%s)\n"),
                 __FILE__,
                 __LINE__,
                 FUNCTION_NAME(),
                 unit,
                 vid,
                 bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *    bcm_trunk_create
 * Purpose:
 *      Allocate an available Trunk ID from the pool
 *      bcm_trunk_create_id.
 * Parameters:
 *      unit - Device unit number.
 *      tid - (Out), The trunk ID.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_FULL      - Trunk table full, no more trunks available.
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_qe2000_trunk_create(int unit, bcm_trunk_t *tid)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 result = BCM_E_FULL;
    int                 index;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    TRUNK_CHECK_INIT(unit);
    TRUNK_DB_LOCK(unit);

    *tid = 0;
    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, *tid);

    for (index = 0; index < tc->ngroups; index++) {
        if (BCM_TRUNK_INVALID == ti->trunk_id) {
            result = bcm_trunk_create(unit, BCM_TRUNK_FLAG_WITH_ID, &index);
            if (BCM_E_NONE == result)
                *tid = index;
            break;
        }
        ti++;
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, *tid, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *      bcm_trunk_create_id
 * Purpose:
 *      Create the software data structure for this trunk ID and program the 
 *      hardware for this TID. User must call bcm_trunk_set() to finish setting
 *      up this trunk.
 * Parameters:
 *      unit - Device unit number.
 *      tid - The trunk ID.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_EXISTS    - TID already used
 *      BCM_E_BADID     - TID out of range
 */

int
bcm_qe2000_trunk_create_id(int unit, bcm_trunk_t tid)
{
    trunk_private_t        *ti;
    int                     result = BCM_E_EXISTS;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    ti = &TRUNK_INFO(unit, tid);            
    if (ti->trunk_id == BCM_TRUNK_INVALID) {
        result = BCM_E_NONE;
        ti->trunk_id  = tid;
        ti->in_use    = FALSE;
        ti->num_ports = 0;
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_psc_set
 * Purpose:
 *      Set the trunk selection criteria.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - The trunk ID to be affected.
 *      psc  - Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - TID out of range
 * Notes:
 *      On this platform, port selection criteria is global and cannot be
 *      configured per trunk group. The rule is, last psc_set wins and affects
 *      EVERY trunk group!
 */

int
bcm_qe2000_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_cntl_t       *tc;
    int                 result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, tid, psc));

    TRUNK_CHECK_INIT(unit);
    TRUNK_CHECK_TID(unit, tid);

    if TRUNK_TID_VALID(unit, tid) {
        TRUNK_DB_LOCK(unit);

        tc = &TRUNK_CNTL(unit);
        tc->psc = psc;

        TRUNK_DB_UNLOCK(unit);

        result = BCM_E_NONE;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, psc, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_psc_get
 * Purpose:
 *      Get the trunk selection criteria.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - The trunk ID to be used.
 *      psc  - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - TID out of range
 */

int
bcm_qe2000_trunk_psc_get(int unit, bcm_trunk_t tid, int *psc)
{
    int                 result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit);
    TRUNK_CHECK_TID(unit, tid);

    if TRUNK_TID_VALID(unit, tid) {
        *psc = TRUNK_CNTL(unit).psc;
        result = BCM_E_NONE;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, *psc, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_chip_info_get
 * Purpose:
 *      Get device specific trunking information.
 * Parameters:
 *      unit    - Device unit number.
 *      ta_info - (OUT) Chip specific Trunk information.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 */

int
bcm_qe2000_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    trunk_cntl_t   *tc;
    int             result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, *) - Enter\n"),
                 FUNCTION_NAME(), unit));

    TRUNK_CHECK_INIT(unit);

    tc = &TRUNK_CNTL(unit);

    ta_info->trunk_group_count = tc->ngroups;
    ta_info->trunk_id_min = 0;
    ta_info->trunk_id_max = tc->ngroups - 1;
    ta_info->trunk_ports_max        = BCM_TRUNK_MAX_PORTCNT;
    ta_info->trunk_fabric_id_min    = SBX_INVALID_MODID;
    ta_info->trunk_fabric_id_max    = SBX_INVALID_MODID;
    ta_info->trunk_fabric_ports_max = SBX_INVALID_PORT;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_set
 * Purpose:
 *      Add ports to a trunk group.
 * Parameters:
 *      unit       - Device unit number.
 *      tid        - The trunk ID the ports are added to.
 *      t_add_info - Information on the trunk group.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - the specified TID was not found
 *      BCM_E_PARAM     - too or invalid many ports specified
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      Any existing ports in the trunk group will be replaced with new ones.
 */
int
bcm_qe2000_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *add_info)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 index;
    int                 result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, [n=%d - "),
                 FUNCTION_NAME(), unit, tid, add_info->num_ports));
    for (index = 0; index < add_info->num_ports; index++) {
        LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                    (BSL_META_U(unit,
                                " %d:%d"),
                     add_info->tm[index], add_info->tp[index]));
    }
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "]) - Enter\n")));

    TRUNK_CHECK_INIT(unit);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    /* make sure trunk is in use */
    if (TRUNK_TID_VALID(unit, tid)) {

        /* Check number of ports in trunk group */
        if (TRUNK_PORTCNT_VALID(unit, add_info->num_ports)) {
            result = _bcm_qe2000_trunk_set(unit, tid, add_info);
            if (BCM_E_NONE == result) {
                tc->psc     = add_info->psc;
                ti->in_use  = TRUE;
            }
        } else {
            result = BCM_E_PARAM;
        }
    } else {
        result = BCM_E_NOT_FOUND;
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_get
 * Purpose:
 *      Return information of a given trunk ID.
 * Parameters:
 *      unit   - Device unit number.
 *      tid    - Trunk ID.
 *      t_data - (Out), data about this trunk.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - the specified TID was not found
 */

int
bcm_qe2000_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_add_info_t *t_data)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    int                 index;
    int                 result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);

    sal_memset(t_data, 0, sizeof(bcm_trunk_add_info_t));
    if (ti->trunk_id != BCM_TRUNK_INVALID) {
        result = BCM_E_NONE;
        t_data->psc = tc->psc;
        t_data->ipmc_index = SBX_INVALID_PORT;
        t_data->dlf_index = ti->dlf_index;
        t_data->mc_index = ti->mc_index;
        t_data->num_ports = ti->num_ports;

        for (index = 0; index < ti->num_ports; index++) {
            t_data->tm[index] = ti->tm[index];
            t_data->tp[index] = ti->tp[index];
        }    
    }    

    TRUNK_DB_UNLOCK(unit);

    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "%s(%d, %d, [n=%d - "),
               FUNCTION_NAME(), unit, tid, t_data->num_ports));
    for (index = 0; index < t_data->num_ports; index++) {
        LOG_DEBUG(BSL_LS_BCM_TRUNK,
                  (BSL_META_U(unit,
                              " %d:%d"),
                   t_data->tm[index], t_data->tp[index]));
    }
    LOG_DEBUG(BSL_LS_BCM_TRUNK,
              (BSL_META_U(unit,
                          "]) - Exit(%d: %s)\n"),
               result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_destroy
 * Purpose:
 *      Removes a trunk group. Performs hardware steps neccessary to tear
 *      down a create trunk.
 * Parameters:
 *      unit - Device unit number.
 *      tid  - Trunk Id.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_BADID     - TID out of range
 *      BCM_E_NOT_FOUND - Trunk does not exist
 *      BCM_E_XXXXX     - As set by lower layers of software
 * Notes:
 *      The return code of the trunk_set call is purposely ignored. 
 */

int
bcm_qe2000_trunk_destroy(int unit, bcm_trunk_t tid)
{
    trunk_cntl_t           *tc;
    trunk_private_t        *ti;
    bcm_trunk_add_info_t    add_info;
    int                     result = BCM_E_NOT_FOUND;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, tid));

    TRUNK_CHECK_INIT(unit);
    TRUNK_CHECK_TID(unit, tid);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);
    ti = &TRUNK_INFO(unit, tid);            

    if (ti->trunk_id != BCM_TRUNK_INVALID) {
        result = BCM_E_NONE;
        if (0 < ti->num_ports) {
            sal_memset((void *)&add_info, 0, sizeof(bcm_trunk_add_info_t));
            add_info.psc = tc->psc;
            result = bcm_qe2000_trunk_set(unit, tid, &add_info);
        }

        if (BCM_E_NONE == result) {
            ti->trunk_id  = BCM_TRUNK_INVALID;
            ti->in_use    = FALSE;
            ti->num_ports = 0;
        }
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, result, bcm_errmsg(result)));

    return result;
}

/*
 * Function:
 *    bcm_trunk_detach
 * Purpose:
 *      Shuts down the trunk module. Any configured trunks are destroyed. All 
 *      steps taken to initialize the trunk module are undone.
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_NONE              Success.
 *      BCM_E_XXX
 */

int
bcm_qe2000_trunk_detach(int unit)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         tid;
    int                 rv;
    int                 result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    /* Don't use TRUNK_CHECK_INIT macro here - It module is not initialized
     * just return OK. If TRUNK_CNTL(unit).init != TRUE or FALSE, return its
     * value.
     */

    if (!BCM_TRUNK_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    } else if (TRUNK_CNTL(unit).init == FALSE) {
        return BCM_E_NONE;
    } else if (TRUNK_CNTL(unit).init != TRUE) {
        return TRUNK_CNTL(unit).init;
    }

    tc = &TRUNK_CNTL(unit);

    TRUNK_DB_LOCK(unit);
    /* Destroy existing trunks */
    for (tid = 0; tid < tc->ngroups; tid++) {
        ti = &TRUNK_INFO(unit, tid);
        if (BCM_TRUNK_INVALID != ti->trunk_id) {
            /* Destroy known trunks ignoring return code */
            rv = bcm_trunk_destroy(unit, tid);
            if (BCM_E_NONE != rv) {
                /* return last failure */
                result = rv;
            }
        }
    }

    tc->init = FALSE;

    /* Free trunk_private_t (t_info) structures */
    if (NULL != tc->t_info) {
        sal_free(tc->t_info);
        tc->t_info = NULL;
    }

    /* Set number of ports and groups to zero */
    tc->ngroups = 0;
    tc->nports  = 0;

    TRUNK_DB_UNLOCK(unit);

    /* Destroy LOCK (no more data to protect */
    if (NULL != tc->lock) {
        sal_mutex_destroy(tc->lock);
        tc->lock = NULL;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, result, bcm_errmsg(result)));
    return result;
}

/*
 * Function:
 *    bcm_trunk_bitmap_expand
 * Purpose:
 *      Given a port bitmap, if any of the ports are in a trunk,
 *      add all the trunk member ports to the bitmap.
 * Parameters:
 *      unit     - Device unit number.
 *      pbmp_ptr - Input/output port bitmap
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 * Notes:
 *      This will succeed only if the application has adopted a uniform model
 *      of specifing the trunk module/port as either switch mod/port or
 *      fabric mod/port.
 *
 *      The alternate is that this API be not supported for SBX fabric.
 */

int
bcm_qe2000_trunk_bitmap_expand(int unit, bcm_pbmp_t *pbmp_ptr)
{
#ifdef _SBX_FABRIC_TRUNK_EXPAND_SUPPORTED
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         tid;
    bcm_port_t          port;
    int                 index;
    bcm_pbmp_t          pbmp, t_pbmp;
    int                 result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Enter\n"),
                 FUNCTION_NAME(), unit, *pbmp_ptr));

    TRUNK_CHECK_INIT(unit);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    for (tid = 0; tid < tc->ngroups; tid++) {
        ti = &TRUNK_INFO(unit, tid);
        if ((TRUE == ti->in_use) && (0 < ti->num_ports)) {
            BCM_PBMP_CLEAR(t_pbmp);
            BCM_PBMP_CLEAR(pbmp);
            for (index=0; index<ti->num_ports; index++) {
                /* create the port bitmap of this trunk */
                port = ti->tp[index];
                BCM_PBMP_PORT_ADD(t_pbmp, port);    /* construct temp bitmap */
            }
            BCM_PBMP_ASSIGN(pbmp, t_pbmp);      /* save a copy */
            BCM_PBMP_AND(t_pbmp, *pbmp_ptr);    /* find common ports */

            /* if lists have common member */
            if (TRUE == BCM_PBMP_NOT_NULL(t_pbmp)) { 
                BCM_PBMP_OR(*pbmp_ptr, pbmp);   /* add saved member set */
            }
        }
    }
    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, *pbmp_ptr, result, bcm_errmsg(result)));

    return result;
#else /* _SBX_FABRIC_TRUNK_EXPAND_SUPPORTED */
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, ...) - Exit(%d: %s)\n"),
                 FUNCTION_NAME(), unit, BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));
    return BCM_E_UNAVAIL;
#endif /* !_SBX_FABRIC_TRUNK_EXPAND_SUPPORTED */
}

/*
 * Function:
 *    bcm_trunk_mcast_join
 * Purpose:
 *    Add the trunk group to existing MAC multicast entry.
 * Parameters:
 *      unit - Device unit number.
 *      tid - Trunk Id.
 *      vid - Vlan ID.
 *      mac - MAC address.
 * Returns:
 *    BCM_E_XXX
 * Notes:
 *      Applications have to remove the MAC multicast entry and re-add in with
 *      new port bitmap to remove the trunk group from MAC multicast entry.
 */

int
bcm_qe2000_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid, sal_mac_addr_t mac)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %02X:%02X:%02X:%02X:%02X:%02X) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, vid, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcm_trunk_find
 * Description:
 *      Get trunk id that contains the given system port
 * Parameters:
 *      unit    - Device unit number
 *      modid   - Module ID
 *      port    - Port number
 *      tid     - (OUT) Trunk id
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_INIT      - Trunking software not initialized
 *      BCM_E_NOT_FOUND - The module:port combo was not found in a trunk.
 */
int
bcm_qe2000_trunk_find(int unit, bcm_module_t modid, bcm_port_t port,
                      bcm_trunk_t *tid)
{
    trunk_cntl_t       *tc;
    trunk_private_t    *ti;
    bcm_trunk_t         t;
    int                 index;
    int                 result = BCM_E_NOT_FOUND;
    bcm_module_t        fabric_mod;
    bcm_port_t          fabric_port;


    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, *) - Enter\n"),
                 FUNCTION_NAME(), unit, modid, port));

    TRUNK_CHECK_INIT(unit);

    TRUNK_DB_LOCK(unit);

    tc = &TRUNK_CNTL(unit);

    *tid = BCM_TRUNK_INVALID;
    for (t = 0; t < tc->ngroups; t++) {
        ti = &TRUNK_INFO(unit, t);
        if ((TRUE == ti->in_use) && (0 < ti->num_ports)) {
            for (index = 0; index < ti->num_ports; index++) {
                if (_bcm_qe2000_trunk_get_fabric_modport(unit, ti->tm[index], ti->tp[index],
                                       &fabric_mod, &fabric_port) != BCM_E_NONE) {
                    continue;
                }
                if ((fabric_mod == modid) && (fabric_port == port)) {
                    *tid = ti->trunk_id;
                    result = BCM_E_NONE;
                    break;
                }
            }
        }
        if (BCM_E_NONE == result) {
            break;
        }
    }

    TRUNK_DB_UNLOCK(unit);

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, modid, port, *tid, result, bcm_errmsg(result)));
    return result;

}

/*
 * Function:
 *    bcm_trunk_egress_set
 * Description:
 *    Set switching only to indicated ports from given trunk.
 * Parameters:
 *    unit - Device unit number.
 *      tid - Trunk Id.  Negative trunk id means set all trunks.
 *    pbmp - bitmap of ports to allow egress.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_qe2000_trunk_egress_set(int unit, bcm_trunk_t tid, bcm_pbmp_t pbmp)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid,
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *    bcm_trunk_egress_get
 * Description:
 *    Retrieve bitmap of ports for which switching is enabled for trunk.
 * Parameters:
 *    unit - Device unit number.
 *      tid - Trunk Id.  Negative trunk id means choose any trunk.
 *    pbmp - (OUT) bitmap of ports where egress allowed.
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_qe2000_trunk_egress_get(int unit, bcm_trunk_t tid, bcm_pbmp_t *pbmp)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid,
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ucast_set
 * Description:
 *      Set the trunk override over UC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_ucast_set(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int modid, int enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, port, tid, modid, enable, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ucast_get
 * Description:
 *      Get the trunk override over UC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      modid  - Module id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_ucast_get(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int modid, int *enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, port, tid, modid, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_mcast_set
 * Description:
 *      Set the trunk override over MC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_mcast_set(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int idx, int enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, port, tid, idx, enable, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_mcast_get
 * Description:
 *      Get the trunk override over MC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - MC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_mcast_get(int unit, bcm_port_t port,
                             bcm_trunk_t tid, int idx, int *enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, port, tid, idx, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ipmc_set
 * Description:
 *      Set the trunk override over IPMC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_ipmc_set(int unit, bcm_port_t port,
                            bcm_trunk_t tid, int idx, int enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, port, tid, idx, enable,
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_ipmc_get
 * Description:
 *      Get the trunk override over IPMC.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      idx    - IPMC index carried in HiGig header.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_ipmc_get(int unit, bcm_port_t port,
                            bcm_trunk_t tid, int idx, int *enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, port, tid, idx, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_vlan_set
 * Description:
 *      Set the trunk override over VLAN.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number, -1 to all ports.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_vlan_set(int unit, bcm_port_t port,
                            bcm_trunk_t tid, bcm_vlan_t vid, int enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, %d) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, vid, enable, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_override_vlan_get
 * Description:
 *      Get the trunk override over VLAN.
 * Parameters:
 *      unit   - Device unit number.
 *      port   - Port number.
 *      tid    - Trunk id.
 *      vid    - VLAN id.
 *      enable - (OUT) TRUE if enabled, FALSE if disabled.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_override_vlan_get(int unit, bcm_port_t port,
                            bcm_trunk_t tid, bcm_vlan_t vid, int *enable)
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, vid, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_pool_set
 * Description:
 *      Set trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - Device unit number.
 *      port    - Port number, -1 to all ports.
 *      tid     - Trunk id.
 *      size    - Trunk pool size.
 *      weights - Weights for each port, all 0 means weighted fair.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_pool_set(int unit, bcm_port_t port, bcm_trunk_t tid,
                   int size, const int weights[BCM_TRUNK_MAX_PORTCNT])
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, size, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_pool_get
 * Description:
 *      Get trunk pool table that contains the egress port number
 *      indexed by the hash value.
 * Parameters:
 *      unit    - Device unit number.
 *      port    - Port number.
 *      tid     - Trunk id.
 *      size    - (OUT) Trunk pool size.
 *      weights - (OUT) Weights (total count) for each port.
 * Returns:
 *      BCM_E_xxxx
 */

int
bcm_qe2000_trunk_pool_get(int unit, bcm_port_t port, bcm_trunk_t tid,
                   int *size, int weights[BCM_TRUNK_MAX_PORTCNT])
{
    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d, %d, %d, *) - Exit(%d: %s)\n"), 
                 FUNCTION_NAME(), unit, tid, *size, 
                 BCM_E_UNAVAIL, bcm_errmsg(BCM_E_UNAVAIL)));

    return BCM_E_UNAVAIL;
}

/*
 * Function:
 *      bcm_trunk_init
 * Purpose:
 *      Initializes the trunk module. The hardware and the software data 
 *      structures are both set to their initial states with no trunks 
 *      configured.
 * Parameters:
 *      unit - Device unit number.
 * Returns:
 *      BCM_E_NONE      - Success.
 *      BCM_E_MEMORY    - Out of memory
 *      BCM_E_XXXXX     - As set by lower layers of software
 */

int
bcm_qe2000_trunk_init(int unit)
{
    trunk_cntl_t               *tc;
    trunk_private_t            *ti;
    int                         alloc_size;
    bcm_trunk_t                 tid;
    int                         result = BCM_E_NONE;

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Enter\n"),
                 FUNCTION_NAME(), unit));

    if (!BCM_TRUNK_UNIT_VALID(unit)) {
        return BCM_E_UNIT;
    }

    tc = &TRUNK_CNTL(unit);

    if (NULL == tc->lock) {
        if (NULL == (tc->lock = sal_mutex_create("qe2000_trunk_lock"))) {
            return BCM_E_MEMORY;
        }
    }
    tc->ngroups = _ngroups;
    tc->nports  = _nports;
    tc->psc     = BCM_TRUNK_PSC_DEFAULT;

    if (tc->t_info != NULL) {
        sal_free(tc->t_info);
        tc->t_info = NULL;
    }

    /* alloc memory and clear */
    if (tc->ngroups > 0) {
        alloc_size = tc->ngroups * sizeof(trunk_private_t);
        ti = sal_alloc(alloc_size, "Trunk-private");
        if (NULL == ti) {
            return BCM_E_MEMORY;
        }
        tc->t_info = ti;
        sal_memset(ti, 0, alloc_size);

        /* internal structures */
        for (tid = 0; tid < tc->ngroups; tid++) {
            /* disable all trunk group and clear all trunk bitmap */
            ti->trunk_id        = BCM_TRUNK_INVALID;
            ti->in_use          = FALSE;
            ti->dlf_index = SBX_INVALID_PORT;
            ti->mc_index = SBX_INVALID_PORT;
            ti++;
        }
    }

    if (BCM_E_NONE == result) {
        tc->init = TRUE;
    }

    LOG_VERBOSE(BSL_LS_BCM_TRUNK,
                (BSL_META_U(unit,
                            "%s(%d) - Exit(%d: %s)\n"),
                 FUNCTION_NAME(), unit, result, bcm_errmsg(result)));
    return result;
}


