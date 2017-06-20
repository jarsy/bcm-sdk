/*
 * $Id: trunk.c,v 1.80 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:    trunk.c
 * Purpose: BCM level APIs for trunking (a.k.a. Port Aggregation)
 */

#include <shared/bsl.h>

#include <sal/types.h>

#include <soc/mem.h>
#include <soc/debug.h>
#include <soc/drv.h>

#include <bcm/error.h>
#include <bcm/vlan.h>
#include <bcm/mcast.h>
#include <bcm/l2.h>
#include <bcm/port.h>
#include <bcm/trunk.h>
#include <bcm/switch.h>
#include <bcm/stack.h>

#include <bcm_int/robo_dispatch.h>
#include <bcm_int/robo/trunk.h>
#include <bcm_int/robo/port.h>
#include <bcm_int/common/lock.h>

#include <bcm/link.h>

int _bcm_robo_trunk_destroy(
    int unit, bcm_trunk_t tid, trunk_private_t *t_info);

#define FE_TRUNK_PATCH_LOCK(unit) \
        sal_mutex_take(fe_trunk_patch_lock_5324[unit]->lock, sal_mutex_FOREVER)

#define FE_TRUNK_PATCH_UNLOCK(unit) \
        sal_mutex_give(fe_trunk_patch_lock_5324[unit]->lock)

typedef struct trunk_cntl_s {
    int                ngroups;     /* number of trunk groups */
    int                nports;      /* port count per trunk group */
    trunk_private_t   *t_info;
} trunk_cntl_t;

/*
 * One trunk control entry for each SOC device containing trunk book keeping
 * info for that device.
 */

static trunk_cntl_t bcm_robo_trunk_control[BCM_MAX_NUM_UNITS];

#define TRUNK_CNTL(unit)       bcm_robo_trunk_control[unit]
#define TRUNK_INFO(unit, tid)  bcm_robo_trunk_control[unit].t_info[tid]

#define SDK_GNATS_897       /* supporting definition on SW GNATS #897 */

/*
 * Cause a routine to return BCM_E_INIT if trunking subsystem is not
 * initialized.
 */

#define TRUNK_INIT(unit)    \
      if (TRUNK_CNTL(unit).ngroups <= 0 ||TRUNK_CNTL(unit).t_info == NULL) {\
          return BCM_E_INIT; }

/*
 * Make sure TID is within valid range.
 */

#define TRUNK_CHECK(unit, tid) \
    if ((tid) < 0 || (tid) >= TRUNK_CNTL(unit).ngroups) { \
        return BCM_E_PARAM; }

/*
 * Make sure PSC is within valid range.
 */

/* BCM_53242_A0 || BCM_53280_A0 */
#if defined(BCM_53242_A0) || defined(BCM_53280_A0)
#define TRUNK_PSC_CHECK(unit, psc) \
    if ((psc & _BCM_TRUNK_PSC_VALID_VAL) < BCM_TRUNK_PSC_SRCMAC ) { \
        return BCM_E_BADID; } \
    else if ((psc & _BCM_TRUNK_PSC_VALID_VAL) > BCM_TRUNK_PSC_SRCDSTIP) { \
        return BCM_E_UNAVAIL; }
#else /* !(BCM_53242_A0 || BCM_53280_A0)*/
#define TRUNK_PSC_CHECK(unit, psc) \
    if ((psc & _BCM_TRUNK_PSC_VALID_VAL) < BCM_TRUNK_PSC_SRCMAC ) { \
        return BCM_E_BADID; } \
    else if ((psc & _BCM_TRUNK_PSC_VALID_VAL) > BCM_TRUNK_PSC_SRCDSTMAC) { \
        return BCM_E_UNAVAIL; }
#endif /* !(BCM_53242_A0 || BCM_53280_A0) */

/*********************************************************
 *  Internal Routines
 *********************************************************/

/*
 * Function:
 *      _bcm_robo_trunk_destroy
 * Purpose:
 *      Multiplexed trunking function for Robo
 */
int
_bcm_robo_trunk_destroy(
    int unit, bcm_trunk_t tid, trunk_private_t *t_info)
{
    bcm_port_t  port;
    bcm_pbmp_t  tr_pbmp;
    bcm_pbmp_t  old_trunk_pbmp, trunk_pbmp;
    uint32  flag, hash_op = 0;

    /* 
     * Get trunk pbmp :
     * - get the old trunk portmap but finally set its' port bitmap 
     *   been 0x0 still.
     */
    flag = DRV_TRUNK_FLAG_BITMAP;
    BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
        (unit, tid, &trunk_pbmp, flag, &hash_op));
    BCM_PBMP_CLEAR(old_trunk_pbmp);
    SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
    
    BCM_PBMP_CLEAR(tr_pbmp);
    BCM_PBMP_ITER(old_trunk_pbmp, port) {
        BCM_PBMP_PORT_ADD(tr_pbmp, port);
    }
    
    /* Clear the learned trunk ports from L2 table */ 
    BCM_IF_ERROR_RETURN(bcm_l2_addr_delete_by_trunk(unit, tid, 0));
    
    /* Disable this trunk and clear the port bitmap */
    flag = DRV_TRUNK_FLAG_DISABLE | DRV_TRUNK_FLAG_BITMAP;
    SOC_PBMP_CLEAR(trunk_pbmp);
    BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
        (unit, tid, trunk_pbmp, flag, hash_op));
    
    return (BCM_E_NONE);
    
}

/*
 * Function:
 *      _bcm_robo_trunk_mcast_join
 * Purpose:
 *      Multiplexed function for Robo
 */
int
_bcm_robo_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid, 
    sal_mac_addr_t mac, trunk_private_t *t_info)
{
    bcm_mcast_addr_t  mc_addr;
    bcm_pbmp_t  pbmp, m_pbmp, trunk_pbmp;
    uint32  flag, hash_op;

    flag = DRV_TRUNK_FLAG_BITMAP;
    BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
        (unit, tid, &trunk_pbmp, flag, &hash_op));

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "trunk_mcast_join: \n\t trunk_pbm=0x%x\n"), 
                 SOC_PBMP_WORD_GET(trunk_pbmp, 0)));
    BCM_PBMP_CLEAR(pbmp);
    SOC_PBMP_ASSIGN(pbmp, trunk_pbmp);

    BCM_IF_ERROR_RETURN(bcm_mcast_port_get(unit, mac, vid, &mc_addr));
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "\t mcast_pbm=0x%x\n"),SOC_PBMP_WORD_GET(mc_addr.pbmp, 0)));
    BCM_PBMP_ASSIGN(m_pbmp, mc_addr.pbmp);
    BCM_PBMP_XOR(m_pbmp, pbmp);
    BCM_PBMP_AND(m_pbmp, pbmp);
    if (SOC_PBMP_IS_NULL(m_pbmp)) {
        return (BCM_E_CONFIG);
    }

    BCM_IF_ERROR_RETURN(bcm_mcast_addr_remove(unit, mac, vid));
    SOC_PBMP_REMOVE(mc_addr.pbmp, pbmp);
    if (t_info->mc_port_used >= 0) {
        BCM_PBMP_PORT_ADD(mc_addr.pbmp, t_info->mc_port_used);
    }
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "\t last mc_addr.pbmp=0x%x\n"),SOC_PBMP_WORD_GET(mc_addr.pbmp,0)));
    BCM_IF_ERROR_RETURN(bcm_mcast_addr_add(unit, &mc_addr));

    return (BCM_E_NONE);
}

#ifndef SDK_GNATS_897
/*
 * Function:
 *      _robo_trunk_check_vlan
 * Purpose:
 *      Make sure all ports in t_pbmp_new are in the same VLANs
 * Parameters:
 *      t_pbmp_new - ports to be added to a trunk group
 *      t_pbmp_old - previous ports in this trunk group.
 */
STATIC int
_robo_trunk_check_vlan(
    int unit, bcm_pbmp_t t_pbmp_new, bcm_pbmp_t t_pbmp_old)
{
    bcm_vlan_data_t *list;
    bcm_pbmp_t      pbmp;
    bcm_vlan_t      vid0, vid;
    int             i, count, port;
    uint32          vlan_mode;

    BCM_IF_ERROR_RETURN(DRV_VLAN_MODE_GET
        (unit, (uint32 *)&vlan_mode));
    if (vlan_mode == DRV_VLAN_MODE_PORT_BASE) {    /* port based VLAN */
        /* check port based VLAN table to see if any conflicts */
        vid = 0;
        PBMP_ITER(t_pbmp_new, port) {
            /* call port API */
            BCM_IF_ERROR_RETURN(bcm_port_untagged_vlan_get(unit, port, &vid0));
            break;
        }
    
        PBMP_ITER(t_pbmp_new, port) {
            /* call port API */
            BCM_IF_ERROR_RETURN(bcm_port_untagged_vlan_get(unit, port, &vid));
            if (vid != vid0) {
                LOG_ERROR(BSL_LS_BCM_COMMON,
                          (BSL_META_U(unit,
                                      "Trunk ports in different VLANs\n")));
                return (BCM_E_CONFIG);
            }
        }
    } else {    /* tagged VLAN */
        /* check tagged based VLAN table  */
        /* call vlan API */
        BCM_IF_ERROR_RETURN(bcm_vlan_list_by_pbmp
            (unit, t_pbmp_new, &list, &count));

        for (i = 0; i < count; i++) {
            BCM_PBMP_ASSIGN(pbmp, list[i].port_bitmap);
            BCM_PBMP_OR(pbmp, t_pbmp_old);
    
            PBMP_ITER(t_pbmp_new, port) {
                if (!PBMP_MEMBER(pbmp, port)) {
                    LOG_ERROR(BSL_LS_BCM_COMMON,
                              (BSL_META_U(unit,
                                          "Trunk conflicts VLAN %d: port %d\n"), 
                               list[i].vlan_tag, port));
                    /* call vlan API */
                    BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy
                        (unit, list, count));
                    return (BCM_E_CONFIG);
                }
            }
        }
        /* call vlan API */
        BCM_IF_ERROR_RETURN(bcm_vlan_list_destroy(unit, list, count));
    
    }   
    return (BCM_E_NONE);

}
#endif

/*
 * Function:
 *      _bcm_robo_trunk_get
 * Purpose:
 *      Internal trunk_get function for Robo.
 * Note:
 *
 */
STATIC int
_bcm_robo_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info, 
    int member_max, bcm_trunk_member_t *member_array, 
    int *member_count, trunk_private_t *t_info)
{
    bcm_pbmp_t  trunk_pbmp;
    bcm_pbmp_t  old_trunk_pbmp;
    bcm_port_t  port;
    uint32  flag, hash_op;
    uint32  port_counter = 0;
    int  size;

    if (!t_info->in_use) {
        *member_count = 0;
    } else {
        /*
         * Read the old trunk member pbmp
         */
        flag = DRV_TRUNK_FLAG_BITMAP;
        BCM_PBMP_CLEAR(trunk_pbmp);
        BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
            (unit, tid, &trunk_pbmp, flag, &hash_op));
        BCM_PBMP_CLEAR(old_trunk_pbmp);
        SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
        BCM_PBMP_COUNT(old_trunk_pbmp, size);
        *member_count = size;

        if (member_max > 0) {
            /* Can't greater than maximum number of members in provided member_array */
            if (*member_count > member_max) {
                *member_count = member_max;
            }
    
            BCM_PBMP_ITER(old_trunk_pbmp, port) {
                if (port_counter < *member_count) {
                    bcm_trunk_member_t_init(&member_array[port_counter]);
                    member_array[port_counter].gport = port;
                    port_counter++;
                } else {
                    break;
                }
            }
        }
    }

    trunk_info->psc = t_info->psc;
    trunk_info->mc_index = t_info->mc_index_used;

    return BCM_E_NONE;
    
}

/*
 * Function:
 *      _bcm_robo_trunk_set
 * Purpose:
 *      Internal trunk_set function for Robo.
 * Note:
 *      1.Assume this tid has been created and the trunk_pbmp
 *        is a valid value. (bcm_trunk_set() has verified on 
 *        tid & trunk ports)
 */
STATIC int
_bcm_robo_trunk_set(int unit, bcm_trunk_t tid, 
    bcm_trunk_info_t *trunk_info, int member_count, 
    bcm_trunk_member_t *member_array, trunk_private_t *t_info)
{
    bcm_port_t  port;
    bcm_pbmp_t  old_trunk_pbmp;
    bcm_pbmp_t  new_trunk_pbmp;
    bcm_pbmp_t  diff_pbmp;
    bcm_pbmp_t  trunk_pbmp;
    int  i, mc_index;
    uint32  flag, hash_op = 0, hash_op_new = 0;

    if (trunk_info->psc <= 0) {
        trunk_info->psc = BCM_TRUNK_PSC_DEFAULT;
    }

#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
        /* Northstar MUST plus VLAN_ID for hash algorithm
         * - (VLAN_ID + MAC_DA) ^ (VLAN_ID + MAC_SA)
         * - (VLAN_ID + MAC_DA)
         * - (VLAN_ID + MAC_SA)
         */
    if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
        hash_op |= DRV_TRUNK_HASH_FIELD_VLANID;
    }
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */

    /* Set the Trunk criteria */
    switch (trunk_info->psc & _BCM_TRUNK_PSC_VALID_VAL) {
        case BCM_TRUNK_PSC_SRCMAC:
            hash_op |= DRV_TRUNK_HASH_FIELD_MACSA;
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
            /* Remove the undefined psc :
             * - below psc may not be suitable for all Robo chips but 
             *   implemented for general purpose on supporting every 
             *   Robo Chip.
             */
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                (unit, 
                DRV_TRUNK_HASH_FIELD_MACDA | 
                DRV_TRUNK_HASH_FIELD_ETHERTYPE | 
                DRV_TRUNK_HASH_FIELD_VLANID |
                DRV_TRUNK_HASH_FIELD_L3));
            break;
        case BCM_TRUNK_PSC_DSTMAC:
            hash_op |= DRV_TRUNK_HASH_FIELD_MACDA;
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
            /* Remove the undefined psc :
             * - below psc may not be suitable for all Robo chips but 
             *   implemented for general purpose on supporting every 
             *   Robo Chip.
             */
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                (unit, 
                DRV_TRUNK_HASH_FIELD_MACSA | 
                DRV_TRUNK_HASH_FIELD_ETHERTYPE | 
                DRV_TRUNK_HASH_FIELD_VLANID |
                DRV_TRUNK_HASH_FIELD_L3));
            break;
        case BCM_TRUNK_PSC_SRCDSTMAC:
            hash_op |= DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA;
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
            /* Remove the undefined psc :
             * - below psc may not be suitable for all Robo chips but 
             *   implemented for general purpose on supporting every 
             *   Robo Chip.
             */
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                (unit, 
                DRV_TRUNK_HASH_FIELD_ETHERTYPE | 
                DRV_TRUNK_HASH_FIELD_VLANID |
                DRV_TRUNK_HASH_FIELD_L3));
            break;
            /* This psc may not be suitable for all Robo chips but 
             * implemented for general purpose on supporting every 
             * Robo Chip.
             * Below psc type will be implemented in case further RoboChip 
             * provides those psc type.
             */
        case BCM_TRUNK_PSC_SRCIP:
            hash_op = DRV_TRUNK_HASH_FIELD_IP_MACSA;
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
            /* Remove the undefined psc :
             * - below psc may not be suitable for all Robo chips but 
             *   implemented for general purpose on supporting every 
             *   Robo Chip.
             */
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                (unit, 
                DRV_TRUNK_HASH_FIELD_IP_MACDA | 
                DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE | 
                DRV_TRUNK_HASH_FIELD_IP_VLANID |
                DRV_TRUNK_HASH_FIELD_L3));
            break;
        case BCM_TRUNK_PSC_DSTIP:
            hash_op = DRV_TRUNK_HASH_FIELD_IP_MACDA;
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
            /* Remove the undefined psc :
             * - below psc may not be suitable for all Robo chips but 
             *   implemented for general purpose on supporting every 
             *   Robo Chip.
             */
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                (unit, 
                DRV_TRUNK_HASH_FIELD_IP_MACSA | 
                DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE | 
                DRV_TRUNK_HASH_FIELD_IP_VLANID |
                DRV_TRUNK_HASH_FIELD_L3));
            break;
        case BCM_TRUNK_PSC_SRCDSTIP:
            hash_op = (DRV_TRUNK_HASH_FIELD_IP_MACDA | 
                       DRV_TRUNK_HASH_FIELD_IP_MACSA);
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op));
            /* Remove the undefined psc :
             * - below psc may not be suitable for all Robo chips but 
             *   implemented for general purpose on supporting every 
             *   Robo Chip.
             */
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_REMOVE
                (unit, 
                DRV_TRUNK_HASH_FIELD_IP_ETHERTYPE | 
                DRV_TRUNK_HASH_FIELD_IP_VLANID |
                DRV_TRUNK_HASH_FIELD_L3));
            break;
        default:
            return BCM_E_PARAM;
    }
    
    /* OR-able hash key selection : additional fields for hashing in trunk mode */
    if (trunk_info->psc & ~_BCM_TRUNK_PSC_VALID_VAL) {
        if (trunk_info->psc & 
            ~(_BCM_TRUNK_PSC_VALID_VAL | _BCM_TRUNK_PSC_ORABLE_VALID_VAL)) {
            return BCM_E_UNAVAIL;
        }

        hash_op_new = hash_op;
        if (trunk_info->psc & BCM_TRUNK_PSC_IPMACSA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACSA | 
                           DRV_TRUNK_HASH_FIELD_MACSA;
        }
        if (trunk_info->psc & BCM_TRUNK_PSC_IPMACDA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACDA | 
                           DRV_TRUNK_HASH_FIELD_MACDA;
        }
        if (trunk_info->psc & BCM_TRUNK_PSC_IPSA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACSA;
        }
        if (trunk_info->psc & BCM_TRUNK_PSC_IPDA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_IP_MACDA;
        }
        if (trunk_info->psc & BCM_TRUNK_PSC_MACSA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_MACSA;
        }
        if (trunk_info->psc & BCM_TRUNK_PSC_MACDA) {
            hash_op_new |= DRV_TRUNK_HASH_FIELD_MACDA;
        }

        if (hash_op_new != hash_op) {
            BCM_IF_ERROR_RETURN(DRV_TRUNK_HASH_FIELD_ADD(unit, hash_op_new));
        }
    }

    if (member_count < 1) {
        if (t_info->in_use) {
            /* Delete existing trunk member ports */
            BCM_IF_ERROR_RETURN(bcm_trunk_destroy(unit, tid));
        }
    } else {
        /*
         * Make sure the ports supplied are valid and DLF port is
         * part of the trunk group ports
         */
        for (i = 0; i < member_count; i++) {
            port = member_array[i].gport;
            if (!IS_E_PORT(unit, port) && !IS_CPU_PORT(unit, port)) {
                return BCM_E_BADID;
            }
        }
    
        /*
         * Read the old trunk member pbmp
         */
        flag = DRV_TRUNK_FLAG_BITMAP;
        BCM_PBMP_CLEAR(trunk_pbmp);
        BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
            (unit, tid, &trunk_pbmp, flag, &hash_op));
        BCM_PBMP_CLEAR(old_trunk_pbmp);
        SOC_PBMP_ASSIGN(old_trunk_pbmp, trunk_pbmp);
        
        /*
         * Write new turnk member setting process
         */
        BCM_PBMP_CLEAR(new_trunk_pbmp);
        for (i = 0; i < member_count; i++) {
            port = member_array[i].gport;
            BCM_PBMP_PORT_ADD(new_trunk_pbmp, port);
        }
    
        BCM_PBMP_ASSIGN(diff_pbmp, old_trunk_pbmp);
        BCM_PBMP_XOR(diff_pbmp, new_trunk_pbmp);
        
        /* VLAN check is now the responsibility of the caller. */
#ifndef SDK_GNATS_897
        /*
         * Make sure all ports in the trunk group are in the same VLAN.
         */
        BCM_IF_ERROR_RETURN
            (_robo_trunk_check_vlan(unit, new_trunk_pbmp, old_trunk_pbmp));
#endif

        /*
         * Update Trunk setting based on new trunk bitmap, if necessary
         */
        if (BCM_PBMP_NOT_NULL(diff_pbmp)) {
            /* write the pbmp into this trunk */
            SOC_PBMP_ASSIGN(trunk_pbmp, new_trunk_pbmp);
            flag = DRV_TRUNK_FLAG_BITMAP | DRV_TRUNK_FLAG_ENABLE;
            BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
                (unit, tid, trunk_pbmp, flag, 0));
        }
        
        /* Set trunk SW information */
        if (BCM_PBMP_NOT_NULL(new_trunk_pbmp)) {
            t_info->mc_index_spec = trunk_info->mc_index;
    
            /* Determine the multicast port */
            mc_index = t_info->mc_index_spec;
            if (mc_index < 0) {
                mc_index = 0;
            }
            t_info->mc_index_used = mc_index;
            t_info->mc_port_used = member_array[mc_index].gport; 
        }

    }

    return BCM_E_NONE;
}
/************** End of Internal Routine ***********************/


/*
 * Function:
 *      bcm_robo_trunk_init
 * Purpose:
 *      Initializes the trunk tables to empty (no trunks configured)
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_init(int unit)
{
    int  alloc_size;
    trunk_private_t  *t_info;
    bcm_trunk_t  tid;
    uint32  trunk_num = 0;
    uint32  trunk_max_port = 0;
    uint32  flag, hash_op=0;
    bcm_pbmp_t  trunk_pbmp;

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_init()..\n")));

    TRUNK_CNTL(unit).ngroups = 0;
    /* Get the number of allowed turnk groups in this unit */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_NUM, &trunk_num));
    TRUNK_CNTL(unit).ngroups = trunk_num;
    
    /* Set the number of ports that allowed in a trunk in this unit */
    flag = DRV_TRUNK_FLAG_HASH_DEFAULT;
    if (SOC_IS_NORTHSTAR(unit) || SOC_IS_NORTHSTARPLUS(unit)) {
#if defined(BCM_NORTHSTAR_SUPPORT) || defined(BCM_NORTHSTARPLUS_SUPPORT)
        hash_op = (DRV_TRUNK_HASH_FIELD_VLANID |
            DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA);
#endif /* BCM_NORTHSTAR_SUPPORT || NS+ */
    } else {
        hash_op = DRV_TRUNK_HASH_FIELD_MACDA | DRV_TRUNK_HASH_FIELD_MACSA;
    }
    SOC_PBMP_CLEAR(trunk_pbmp);
    BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
        (unit, 0, trunk_pbmp, flag, hash_op));

    /* Get the max port number allowed in a given trunk. */
    BCM_IF_ERROR_RETURN(DRV_DEV_PROP_GET
        (unit, DRV_DEV_PROP_TRUNK_MAX_PORT_NUM, &trunk_max_port));
    TRUNK_CNTL(unit).nports = trunk_max_port;
   
    for (tid = 0; tid < TRUNK_CNTL(unit).ngroups; tid++) {
        /* Disable all trunk group and clear all trunk bitmap*/
        flag = DRV_TRUNK_FLAG_DISABLE | DRV_TRUNK_FLAG_BITMAP;
        SOC_PBMP_CLEAR(trunk_pbmp);
        BCM_IF_ERROR_RETURN(DRV_TRUNK_SET
            (unit, tid, trunk_pbmp, flag, 0));
    }

    if (TRUNK_CNTL(unit).t_info != NULL) {
        sal_free(TRUNK_CNTL(unit).t_info);
        TRUNK_CNTL(unit).t_info = NULL;
    }

    if (TRUNK_CNTL(unit).ngroups > 0) {
        alloc_size = TRUNK_CNTL(unit).ngroups * sizeof(trunk_private_t);
        t_info = (trunk_private_t *) sal_alloc(alloc_size, "trunk_priv");
        if (t_info == NULL) {
            return (BCM_E_MEMORY);
        }
        TRUNK_CNTL(unit).t_info = t_info;
        
        for (tid = 0; tid < TRUNK_CNTL(unit).ngroups; tid++) {
            t_info->tid = BCM_TRUNK_INVALID;
            t_info->in_use = FALSE;
            t_info->mc_index_spec = -1;
            t_info->mc_index_used = -1;
            t_info++;
        }
    }

    return BCM_E_NONE;
}               

/*
 * Function:
 *      bcm_robo_trunk_detach
 * Purpose:
 *      Cleans up the trunk tables.
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_detach(int unit)
{
    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_detach()..\n")));

    if (TRUNK_CNTL(unit).t_info != NULL) {
        sal_free(TRUNK_CNTL(unit).t_info);
        TRUNK_CNTL(unit).t_info = NULL;
    }
    TRUNK_CNTL(unit).ngroups = 0;

    return BCM_E_NONE;
}               
    
/*
 * Function:
 *      _bcm_robo_trunk_create_id
 * Purpose:
 *      Create the software data structure for the specified trunk ID.
 *      This function does not update any hardware tables,
 *      must call bcm_trunk_set() to finish trunk setting.
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 *      tid  - The trunk ID.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_INIT    - trunking software not initialized
 *      BCM_E_EXISTS  - TID already used
 *      BCM_E_BADID   - TID out of range
 */
STATIC int 
_bcm_robo_trunk_create_id(int unit, bcm_trunk_t tid)
{
    trunk_private_t  *t_info;
    int  rv;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : _bcm_robo_trunk_create_id()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        t_info->tid = tid;
        t_info->psc = BCM_TRUNK_PSC_DEFAULT;
        t_info->in_use = FALSE;
        rv = BCM_E_NONE;
    } else {
        rv = BCM_E_EXISTS;
    }

    return rv;
}   

/*
 * Function:
 *      bcm_robo_trunk_create
 * Purpose:
 *      Create the software data structure for a trunk ID.
 *      This function does not update any hardware tables,
 *      must call bcm_trunk_set() to finish trunk setting.
 * Parameters:
 *      unit  - RoboSwitch device unit number (driver internal).
 *      flags - Flags.
 *      tid   - (IN/Out) The trunk ID, IN if BCM_TRUNK_FLAG_WITH_ID flag is set.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_FULL    - run out of TIDs
 */
int 
bcm_robo_trunk_create(int unit, uint32 flags, bcm_trunk_t *tid)
{
    trunk_private_t  *t_info;
    int  rv, i;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_create()..\n")));

    if (tid == NULL) {
        return BCM_E_PARAM;
    }

    if (flags & BCM_TRUNK_FLAG_WITH_ID) {
        return _bcm_robo_trunk_create_id(unit, *tid);
    }

    rv = BCM_E_FULL;

    t_info = TRUNK_CNTL(unit).t_info;

    for (i = 0; i < TRUNK_CNTL(unit).ngroups; i++) {
        if (t_info->tid == BCM_TRUNK_INVALID) {
            t_info->tid = i;
            t_info->in_use = FALSE;
            t_info->psc = BCM_TRUNK_PSC_DEFAULT;
            *tid = i;
            rv = BCM_E_NONE;
            break;
        }
        t_info++;
    }

    return rv;
}               
    
/*
 * Function:
 *       _bcm_robo_trunk_id_validate
 * Purpose:
 *      Service routine to validate validity of trunk id.
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 *      tid  - The trunk ID.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_INIT    - trunking software not initialized
 *      BCM_E_BADID   - TID out of range
 */
int
_bcm_robo_trunk_id_validate(int unit, bcm_trunk_t tid)
{
    int  rv;
    bcm_trunk_info_t  trunk_info;
    bcm_trunk_member_t  member_array[BCM_TRUNK_MAX_PORTCNT];
    int  member_count = 0;

    TRUNK_CHECK(unit, tid);

    rv = bcm_trunk_get(unit, tid, &trunk_info, 
             BCM_TRUNK_MAX_PORTCNT, &member_array[0], &member_count);
    
    if (rv < 0) {
        return BCM_E_PARAM;
    }
    
    /* 
     * If no any port assigned to the current trunk,
     * there will be no further processes.
     * Since no tgid information recorded in the arl entry of robo chip.
     */
    if (!member_count) {
        return BCM_E_PARAM;
    }
    return (BCM_E_NONE);
}
    
/*
 * Function:
 *       bcm_robo_trunk_psc_set
 * Purpose:
 *      Set the trunk selection criteria.
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 *      tid  - The trunk ID to be affected.
 *      psc  - Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_psc_set(int unit, bcm_trunk_t tid, int psc)
{
    trunk_private_t  *t_info;
    bcm_trunk_info_t  trunk_info;
    bcm_trunk_member_t *member_array = NULL;
    int  member_count, unused_count;
    int  rv = BCM_E_NONE;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_psc_set()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    if (psc <= 0) {
        psc = BCM_TRUNK_PSC_DEFAULT;
    }

    if (t_info->psc == psc) {
        return BCM_E_NONE;
    }

    if (!t_info->in_use) {
        t_info->psc = psc;
        return BCM_E_NONE;
    }

    BCM_IF_ERROR_RETURN(bcm_trunk_get(unit, tid, &trunk_info, 
        0, NULL, &member_count));

    if (member_count > 0) {
        member_array = sal_alloc(sizeof(bcm_trunk_member_t) * member_count,
            "trunk member array");
        if (member_array == NULL) {
            return BCM_E_MEMORY;
        }
        rv = bcm_trunk_get(unit, tid, &trunk_info, 
                 member_count, member_array, &unused_count);
        if (BCM_FAILURE(rv)) {
            sal_free(member_array);
            return rv;
        }
    } 

    trunk_info.psc = psc;

    rv = bcm_trunk_set(unit, tid, &trunk_info, member_count, member_array);

    if (member_array != NULL) {
        sal_free(member_array);
    }

    return rv;
}               
    
/*
 * Function:
 *       bcm_robo_trunk_psc_get
 * Purpose:
 *      Get the trunk selection criteria.
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 *      tid  - The trunk ID to be used.
 *      psc  - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_psc_get(int unit, bcm_trunk_t tid, int *psc)
{
    trunk_private_t  *t_info;
    int  rv;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_psc_get()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        *psc = 0;
        rv = BCM_E_NOT_FOUND;
    } else {
        *psc = t_info->psc;
        rv = BCM_E_NONE;
    }

    return rv;
}               
    
/*
 * Function:
 *       bcm_robo_trunk_chip_info_get
 * Purpose:
 *      Get the trunk information.
 * Parameters:
 *      unit   - RoboSwitch device unit number (driver internal).
 *      t_info - (OUT) Identify the trunk selection criteria.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 * Notes:
 *      None.
 */
int 
bcm_robo_trunk_chip_info_get(int unit, bcm_trunk_chip_info_t *ta_info)
{
    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_chip_info_get()..\n")));

    ta_info->trunk_id_min = 0;
    ta_info->trunk_id_max = TRUNK_CNTL(unit).ngroups - 1;
    ta_info->trunk_group_count = TRUNK_CNTL(unit).ngroups;
    ta_info->trunk_ports_max = TRUNK_CNTL(unit).nports;

    /* There is no supported fabric trunk info. for ROBO, set -1 as initialized value */
    ta_info->trunk_fabric_id_min = -1;
    ta_info->trunk_fabric_id_max = -1;
    ta_info->trunk_fabric_ports_max = -1;

    return BCM_E_NONE;
}               

/*
 * Function:
 *	_bcm_robo_trunk_gport_construct
 * Purpose:
 *	Converts ports and modules given in member_array structure into gports
 * Parameters:
 *      unit         - RoboSwitch device unit number (driver internal).
 *      member_count - number of failover gports in list
 *      member_list  - (IN) list of port numbers
 *      member_array - (OUT) list of gports to return
 *
 * Note:
 *      member_list and member_array may be the same list.  This updates
 *      the list in place.
 */
STATIC int
_bcm_robo_trunk_gport_construct(int unit, int member_count, 
    bcm_trunk_member_t *member_list, bcm_trunk_member_t *member_array)
{
    bcm_gport_t  gport;
    int  i, mod_is_local, my_modid;
    _bcm_gport_dest_t  dest;

    sal_memset(&dest, 0, sizeof(_bcm_gport_dest_t));

    BCM_IF_ERROR_RETURN(
        bcm_stk_my_modid_get(unit, &my_modid));

    /* Stacking ports should be encoded as devport */
    BCM_IF_ERROR_RETURN(
        _bcm_robo_modid_is_local(unit, my_modid, &mod_is_local));
	
    for (i = 0; i < member_count; i++) {
        gport = 0;
        if (mod_is_local && IS_ST_PORT(unit, member_list[i].gport)) {
            dest.gport_type = _SHR_GPORT_TYPE_DEVPORT;
        } else {
            dest.gport_type = _SHR_GPORT_TYPE_MODPORT;
        }

        dest.port = member_list[i].gport;
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_construct(unit, &dest, &gport));
        member_array[i].gport = gport;
    }

    return BCM_E_NONE;
}

/*
 * Function:
 *      _bcm_robo_trunk_gport_resolve
 * Purpose:
 *      Converts gports given in t_data structure into ports and modules
 * Parameters:
 *      unit         - RoboSwitch device unit number (driver internal).
 *      member_count - Number of trunk members.
 *      member_array - Array of trunk members.
 */
int
_bcm_robo_trunk_gport_resolve(
    int unit, int member_count, bcm_trunk_member_t *member_array)
{
    bcm_port_t  port;
    bcm_gport_t  gport;
    bcm_module_t  modid;
    bcm_trunk_t  tgid;
    int  i, id;

    if (member_array == NULL) {
        return BCM_E_PARAM;
    }

    for (i = 0; i < member_count; i++) {
        gport = member_array[i].gport;
        if (BCM_GPORT_IS_SET(gport)) {
            BCM_IF_ERROR_RETURN(_bcm_robo_gport_resolve
                (unit, gport, &modid, &port, &tgid, &id));

            if ((-1 != tgid) || (-1 != id)) {
                return BCM_E_PARAM;
            }
            member_array[i].gport = port;
        }
    }

    return BCM_E_NONE;
}               

/*
 * Function:
 *      bcm_robo_trunk_set
 * Purpose:
 *      Adds ports to a trunk group.
 * Parameters:
 *      unit         - RoboSwitch device unit number (driver internal).
 *      tid          - The trunk ID to be affected.
 *      trunk_info   - Information on the trunk group.
 *      member_count - Number of trunk members.
 *      member_array - Array of trunk members.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 * Notes:
 *
 */
int 
bcm_robo_trunk_set(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
    int member_count, bcm_trunk_member_t *member_array)
{
    int  rv;
    trunk_private_t  *t_info;
    bcm_trunk_info_t  local_trunk_info;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_set()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return BCM_E_NOT_FOUND;
    }

    if (trunk_info == NULL) {
        return BCM_E_PARAM;
    }

    if (member_count > 0) {
        if (member_array == NULL) {
            return BCM_E_PARAM;
        }
    } else {
        if (member_array != NULL) {
            return BCM_E_PARAM;
        }
    }

    /* Check number of ports in trunk group */
    if (member_count > TRUNK_CNTL(unit).nports) {
        return BCM_E_PARAM;
    }

    if ((member_count > 0) && (member_array != NULL)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_trunk_gport_resolve(unit, member_count, member_array));
    }

    bcm_trunk_info_t_init(&local_trunk_info);
    sal_memcpy(&local_trunk_info, trunk_info, sizeof(bcm_trunk_info_t));

    rv = _bcm_robo_trunk_set(unit, tid, &local_trunk_info, 
             member_count, member_array, t_info);
    if (BCM_SUCCESS(rv)) {
        t_info->psc = trunk_info->psc;
        if (member_count > 0) {
            t_info->in_use = TRUE;
        }
    }

    return rv;
}               

/* 
 * Function:
 *      bcm_robo_trunk_destroy
 * Purpose:
 *      Removes a trunk group.
 * Parameters:
 *      unit - Device unit number (driver internal).
 *      tid  - Trunk Id.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 * Notes:
 *
 */
int 
bcm_robo_trunk_destroy(int unit, bcm_trunk_t tid)
{
    trunk_private_t  *t_info;
    int  rv;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_destroy()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (!t_info->in_use) {
        t_info->tid = BCM_TRUNK_INVALID;
        return (BCM_E_NONE);
    }

    rv = _bcm_robo_trunk_destroy(unit, tid, t_info);
    
    if (rv >= 0) {
        t_info->tid = BCM_TRUNK_INVALID;
        t_info->in_use = FALSE;
    }

    return (rv);
}                   
         
/*
 * Function:
 *      bcm_robo_trunk_get
 * Purpose:
 *      Return a port information of given trunk ID.
 * Parameters:
 *      unit         - RoboSwitch device unit number (driver internal).
 *      tid          - Trunk ID.
 *      trunk_info   - (OUT) Place to store returned trunk info.
 *      member_max   - (IN) Size of member_array.
 *      member_array - (OUT) Place to store returned trunk members.
 *      member_count - (OUT) Place to store returned number of trunk members.
 * Returns:
 *      BCM_E_NONE    - Success.
 *      BCM_E_XXX
 */
int 
bcm_robo_trunk_get(int unit, bcm_trunk_t tid, bcm_trunk_info_t *trunk_info,
    int member_max, bcm_trunk_member_t *member_array, int *member_count)
{
    trunk_private_t  *t_info;
    bcm_trunk_info_t  local_trunk_info;
    int  local_member_count;
    int  isGport = 0;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_get()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);

    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if ((member_max > 0) && (member_array == NULL)) {
        return BCM_E_PARAM;
    }

    if ((member_max > 0) && (member_count == NULL)) {
        return BCM_E_PARAM;
    }

    bcm_trunk_info_t_init(&local_trunk_info);

    BCM_IF_ERROR_RETURN(_bcm_robo_trunk_get(unit, tid, &local_trunk_info, 
        member_max, member_array, &local_member_count, t_info));

    if (trunk_info != NULL) {
        *trunk_info = local_trunk_info;
    }
    if (member_count != NULL) {
        *member_count = local_member_count;
    }

    if (member_max > 0) {
        BCM_IF_ERROR_RETURN(
            bcm_switch_control_get(unit, bcmSwitchUseGport, &isGport));
        if (isGport != 0) {
            BCM_IF_ERROR_RETURN(_bcm_robo_trunk_gport_construct
                (unit, *member_count, member_array, member_array));
        }
    }

    return BCM_E_NONE;
}               
             
/*
 * Function:
 *      bcm_robo_trunk_bitmap_expand
 * Purpose:
 *      Given a port bitmap, if any of the ports are in a trunk,
 *      add all of the ports of that trunk to the bitmap.
 * Parameters:
 *      unit     - RoboSwitch device unit number (driver internal).
 *      pbmp_ptr - Input/output port bitmap
 * Returns:
 *      BCM_E_NONE     - Success.
 *      BCM_E_INTERNAL - Chip access failure.
 * Notes:
 *      RoboSwitch family not suitable for this API.
 */
int 
bcm_robo_trunk_bitmap_expand(int unit, bcm_pbmp_t *pbmp_ptr)
{
    int  i, rv = BCM_E_NONE;
    bcm_pbmp_t  t_pbmp, trunk_pbmp;
    uint32  hash_op;
    bcm_trunk_chip_info_t  ti;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_bitmap_expand().. \n")));
    
    BCM_IF_ERROR_RETURN(bcm_trunk_chip_info_get(unit, &ti));
    for (i = ti.trunk_id_min; i <= ti.trunk_id_max; i++) {
        SOC_PBMP_CLEAR(trunk_pbmp);
        BCM_IF_ERROR_RETURN(DRV_TRUNK_GET
            (unit, i, &trunk_pbmp, DRV_TRUNK_FLAG_BITMAP, &hash_op));
        
        SOC_PBMP_ASSIGN(t_pbmp, *pbmp_ptr);
        SOC_PBMP_AND(t_pbmp, trunk_pbmp);
        if (SOC_PBMP_NOT_NULL(t_pbmp)) {
            SOC_PBMP_OR(*pbmp_ptr, trunk_pbmp);
        }
    }

    return rv;
}               
    
/*
 * Function:
 *      bcm_robo_trunk_mcast_join
 * Purpose:
 *      Add the trunk group to existing MAC multicast entry.
 * Parameters:
 *      unit - RoboSwitch device unit number (driver internal).
 *      tid  - Trunk Id.
 *      vid  - Vlan ID.
 *      mac  - MAC address.
 * Returns:
 *      BCM_E_XXX
 * Notes:
 *      Applications have to remove the MAC multicast entry and re-add in with
 *      new port bitmap to remove the trunk group from MAC multicast entry.
 */
int 
bcm_robo_trunk_mcast_join(int unit, bcm_trunk_t tid, bcm_vlan_t vid, 
    bcm_mac_t mac)
{
    trunk_private_t  *t_info;
    int  rv;

    TRUNK_INIT(unit);

    LOG_VERBOSE(BSL_LS_BCM_COMMON,
                (BSL_META_U(unit,
                            "BCM API : bcm_robo_trunk_mcast_join()..\n")));

    TRUNK_CHECK(unit, tid);

    t_info = &TRUNK_INFO(unit, tid);
    if (t_info->tid == BCM_TRUNK_INVALID) {
        return (BCM_E_NOT_FOUND);
    }

    if (!t_info->in_use) {
        return (BCM_E_NONE);
    }

    rv = _bcm_robo_trunk_mcast_join(unit, tid, vid, mac, t_info);

    return rv;
}               

/*
 * Function:
 *      bcm_robo_trunk_find
 * Description:
 *      Get trunk id that contains the given system port
 * Parameters:
 *      unit    - RoboSwitch device unit number (driver internal)
 *      modid   - Module ID
 *      port    - Port number
 *      tid     - (OUT) Trunk id
 * Returns:
 *      BCM_E_xxxx
 */
int
bcm_robo_trunk_find(int unit, bcm_module_t modid, 
    bcm_port_t port, bcm_trunk_t *tid)
{
    bcm_trunk_info_t  trunk_info;
    bcm_trunk_t  tgid;
    int  i = 0, idx = 0;
	int  id;
    int  rv;
    bcm_module_t  hw_mod;
    bcm_port_t  hw_port;
    bcm_trunk_member_t  member_array[BCM_TRUNK_MAX_PORTCNT]; 
    int  member_count;

    TRUNK_INIT(unit);

    if (tid == NULL) {
        return BCM_E_PARAM;
    }

    if (BCM_GPORT_IS_SET(port)) {
        BCM_IF_ERROR_RETURN(
            _bcm_robo_gport_resolve(unit, port, &modid, &port, &tgid, &id));

        if ((-1 != tgid) || (-1 != id)) {
            return BCM_E_PORT;
        }
    } else {
        BCM_IF_ERROR_RETURN(bcm_stk_modmap_map
            (unit, BCM_STK_MODMAP_SET, modid, port, &hw_mod, &hw_port));

        if (!SOC_MODID_ADDRESSABLE(unit, hw_mod)) {
            return BCM_E_BADID;
        }

        if (!SOC_PORT_VALID(unit, hw_port)) { 
            return BCM_E_PORT; 
        }
    }

    for (idx = 0; idx < TRUNK_CNTL(unit).ngroups; idx++) {
        if (TRUNK_INFO(unit, idx).tid == BCM_TRUNK_INVALID) {
            continue;
        }
             
        sal_memset(&trunk_info, 0, sizeof(bcm_trunk_info_t));
        rv = bcm_trunk_get(unit, idx, &trunk_info, 
                 BCM_TRUNK_MAX_PORTCNT, &member_array[0], &member_count);

        if (member_count <= 0) {
            continue;
        }

        if (BCM_SUCCESS(rv)) {
            for (i = 0; i < member_count; i++) {
                if (member_array[i].gport == port) {
                    *tid = idx;
                    return BCM_E_NONE;
                }
            }
        }
    }

    return BCM_E_NOT_FOUND;
}
