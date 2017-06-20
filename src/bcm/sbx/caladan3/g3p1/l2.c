/*
 * $Id: l2.c,v 1.28 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * File:        l2.c
 * Purpose:     BCM Layer-2 switch API
 */
#ifdef BCM_CALADAN3_G3P1_SUPPORT

#include <shared/bsl.h>

#include <sal/core/sync.h>

#include <soc/macipadr.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/sbx/g3p1/g3p1_int.h>
#include <soc/sbx/g3p1/g3p1_tmu.h>
#include <soc/sbx/g3p1/g3p1_defs.h>
#include <soc/sbx/g3p1/g3p1_ppe_tables.h>

#include <soc/sbx/g3p1/g3p1.h>
#include <soc/sbx/caladan3.h>

#include <shared/gport.h>

#include <bcm/types.h>
#include <bcm/error.h>
#include <bcm/l2.h>
#include <bcm/stack.h>
#include <bcm/mcast.h>
#include <bcm/vlan.h>
#include <bcm/tunnel.h>
#include <bcm/ipmc.h>
#include <bcm/mpls.h>
#include <bcm/vswitch.h>

#include <shared/idxres_fl.h>

#include <bcm_int/sbx/error.h>
#include <bcm_int/sbx/caladan3/allocator.h>
#include <bcm_int/sbx/l2.h>
#include <bcm_int/sbx/mcast.h>
#include <bcm_int/sbx/stat.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/state.h>
#include <bcm_int/sbx/caladan3/vlan.h>

#include <bcm_int/sbx/caladan3/l2.h>
#include <bcm_int/sbx/caladan3/vlan.h>
#include <bcm_int/sbx/caladan3/port.h>
#include <bcm_int/sbx/caladan3/mpls.h>

#ifdef DEBUG_MACLOG
typedef struct maclog_s {
  bcm_mac_t       mac;
  uint32          fn;
  int             flag;
  int             unit;
  int             port;
  int             modid;
  int             vid;
  int             fte;
} maclog_t;

extern int log_index;
extern maclog_t maclog[];
#endif /* DEBUG_MACLOG */
/* Array of mac info elements. Dynamically allocated based on microcode MACAGE table size */
_ageid_to_mac_info_t * p_age_id_mac_array[BCM_LOCAL_UNITS_MAX];


_age_id_stack_t           age_indexes_stack[BCM_LOCAL_UNITS_MAX];

#define DEBUG_AGING 0

extern sal_mutex_t      _l2_log_lock[BCM_LOCAL_UNITS_MAX];

            
#define L2_LOG_LOCK(unit) sal_mutex_take(_l2_log_lock[unit], sal_mutex_FOREVER)
#define L2_LOG_UNLOCK(unit)  sal_mutex_give(_l2_log_lock[unit])

static bcm_mac_t _mac_null = {0x0, 0x0,0x0, 0x0, 0x0, 0x0};

#ifdef BCM_FE2000_SUPPORT
extern int _bcm_fe2000_mim_fte_gport_get(int unit,
                                         bcm_gport_t gport,
                                         uint32 *ftidx);
extern bcm_gport_t _bcm_fe2000_mim_fte_to_gport_id(int unit,
                                            uint32 ftidx);
#endif

int
_bcm_caladan3_g3p1_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid);

/* Temporary externs until include files are up to date */

extern int _bcm_caladan3_l2_mcast_get(int unit, bcm_l2_addr_t *l2addr);

int _bcm_caladan3_l2_age_id_get(int unit , uint32 * id){

    int rv = BCM_E_NONE;

    /* Take the lock */
    sal_mutex_take(age_indexes_stack[unit].lock, sal_mutex_FOREVER);

    /* check the range */
    if (age_indexes_stack[unit].p == 0) {
        rv = BCM_E_EMPTY;
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Underflow of age indexes rv=%d(%s)\n"),
                   rv, bcm_errmsg(rv)));
        sal_mutex_give(age_indexes_stack[unit].lock);
        return rv;
    }

    /* Pull the value off the stack and bump back the pointer*/
    *id = age_indexes_stack[unit].age_indexes[--age_indexes_stack[unit].p];

    /* Free the lock */
    sal_mutex_give(age_indexes_stack[unit].lock);

    return rv;
}


int _bcm_caladan3_l2_age_id_free(int unit, int age_index)
{
    int rv = BCM_E_NONE;

    /* Take the lock */
    sal_mutex_take(age_indexes_stack[unit].lock, sal_mutex_FOREVER);

    /* check the range */
    if (age_indexes_stack[unit].p == age_indexes_stack[unit].size) {
        rv = BCM_E_FULL;
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Overflow of age indexes rv=%d(%s)\n"),
                   rv, bcm_errmsg(rv)));
        sal_mutex_give(age_indexes_stack[unit].lock);
        return rv;
    }

    /* Put the value on the stack and bump up the pointer*/
    age_indexes_stack[unit].age_indexes[age_indexes_stack[unit].p++] =
            age_index;

    /* Free the lock */
    sal_mutex_give(age_indexes_stack[unit].lock);

    return rv;
}





/* End of Temporary */

void _bcm_l2_caladan3_g3p1_dump_smac_entry(soc_sbx_g3p1_6_byte_t mac,
                                         int vid, 
					   soc_sbx_g3p1_mac_t *data, int age)
{
    L2_DUMP("  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x  %4d  "
            "0x%6.6x   %c    %c   %6d\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            vid, data->poe,
            data->sdrop ? CHAR_SET : CHAR_CLEAR,
            data->dontage ? CHAR_SET : CHAR_CLEAR, 
            age);
}

void _bcm_l2_caladan3_g3p1_dump_dmac_entry(soc_sbx_g3p1_6_byte_t mac,
                                         int vid, 
					   soc_sbx_g3p1_mac_t *data, int age)
{
    L2_DUMP("  %2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x  %4d  "
            "%8d   %c    %c    %c  %6d\n",
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
            vid, data->ftidx,
            data->dcopy ? CHAR_SET : CHAR_CLEAR,
            data->ddrop ? CHAR_SET : CHAR_CLEAR,
            data->dontage  ? CHAR_SET : CHAR_CLEAR, 
            age);
}

typedef void (*mac_dump_f) (soc_sbx_g3p1_6_byte_t mac,
                            int vid, 
                            soc_sbx_g3p1_mac_t *data, int age);

/* Age delta needs to be 1 step more to account for current age elapsed time */
#define L2_AGE_DELTA                    2

int
_bcm_caladan3_g3p1_l2_hw_init(int unit)
{
    int                   rv = BCM_E_NONE;
    int                   macage_tb_elements = soc_sbx_g3p1_macage_table_size_get(unit);
    soc_sbx_g3p1_xt_t     sbx_xt;    /* exception table */
    uint32                exc_idx;
    soc_sbx_g3p1_state_t *pFe;
    int                      i;

    G3P1_FE_HANDLER_GET(unit, pFe);

    /*
     * Aging
     */

    /* Allocate the table that holds the mapping from age id to info needed to lookup the L2 entry */
     p_age_id_mac_array[unit] = sal_alloc(macage_tb_elements * sizeof(_ageid_to_mac_info_t), "age_id");

     if(p_age_id_mac_array[unit] == NULL) {
         rv =  BCM_E_MEMORY;
         LOG_ERROR(BSL_LS_BCM_L2,
                   (BSL_META_U(unit,
                               "failed to allocate memory for aging table rv=%d(%s)\n"),
                    rv, bcm_errmsg(rv)));
         return rv;
     }

     /* Initialize the entries to un-associated */
     for(i=0;  i < macage_tb_elements; i++)
         p_age_id_mac_array[unit][i].valid=0;

     /* Create the pool of aging ids. These will be used to link L2 entries back to the aging table and
      * must be freed back to the pool when a L2 entry is deleted.
      */
     age_indexes_stack[unit].age_indexes = sal_alloc(macage_tb_elements * sizeof(uint32), "age_indexes");
     age_indexes_stack[unit].p=0;
     age_indexes_stack[unit].size=macage_tb_elements;
     age_indexes_stack[unit].lock = sal_mutex_create("mac_age_index_lock");


#ifdef BCM_WARM_BOOT_SUPPORT
     if(SOC_WARM_BOOT(unit))
     {
         return rv;
     }
#endif /* def BCM_WARM_BOOT_SUPPORT */

     /* Do the initial load of indexes to the stack */
     for(i=0; i < macage_tb_elements; i++)
         _bcm_caladan3_l2_age_id_free(unit,i);

     /*
     * L2 learning
     */
    /* Set exception for L2 learning */
    BCM_IF_ERROR_RETURN
      (soc_sbx_g3p1_exc_smac_learn_idx_get(unit, &exc_idx));
    soc_sbx_g3p1_xt_t_init(&sbx_xt);

    rv = soc_sbx_g3p1_xt_get(unit, exc_idx, &sbx_xt);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to get L2 learn exception entry rv=%d(%s)\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    sbx_xt.dp     = 0;
    /*sbx_xt.qid    = SBX_EXC_QID_BASE;*/
    sbx_xt.learn  = 0;
    sbx_xt.forward = 1;
    sbx_xt.trunc  = 1;
    sbx_xt.ppspolice = 1;

    rv = soc_sbx_g3p1_xt_set(unit, exc_idx, &sbx_xt);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to set L2 learn exception entry rv=%d(%s)\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    return rv;
}

int
_bcm_caladan3_g3p1_l2_egress_hw_entry_delete(int unit, bcm_if_t encap_id)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx, ete_idx;
    soc_sbx_g3p1_oi2e_t         sbx_ohi;
    soc_sbx_g3p1_ete_t          ete;


    ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(encap_id);
    
    rv = soc_sbx_g3p1_oi2e_get(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    if (BCM_FAILURE(rv)) {
        return rv;
    }
    
    ete_idx =  sbx_ohi.eteptr;
    rv = soc_sbx_g3p1_ete_get(unit, ete_idx, &ete);
    if (BCM_FAILURE(rv)) {
        return rv;
    }

    soc_sbx_g3p1_ete_t_init(&ete);
    rv = soc_sbx_g3p1_ete_set(unit, ete_idx, &ete);
    if (BCM_FAILURE(rv)) {
        return rv;
    }


    soc_sbx_g3p1_oi2e_t_init(&sbx_ohi);        
    rv = soc_sbx_g3p1_oi2e_set(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);

    if (rv == BCM_E_NONE) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_ETE, 1, 
                               &ete_idx, 0);
    }

    return rv;
}


int
_bcm_caladan3_g3p1_l2addr_hw_add(int unit, bcm_l2_addr_t *l2addr)
{
    int                   rv;
    uint32                age;
    soc_sbx_g3p1_mac_t   sbx_mac_data;
    soc_sbx_g3p1_macage_t age_value;
 


    /* Delete the address if present to keep everything in sync */
    rv = soc_sbx_g3p1_mac_get(unit, l2addr->mac, l2addr->vid, &sbx_mac_data); 
    if (BCM_SUCCESS(rv)) {
        _bcm_caladan3_g3p1_l2_addr_delete(unit,
                                     l2addr->mac,
                                     l2addr->vid);
    }

    soc_sbx_g3p1_mac_t_init(&sbx_mac_data);


#ifdef DEBUG_MACLOG
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].mac[0]=l2addr->mac[0];
    maclog[my_log_id].mac[1]=l2addr->mac[1];
    maclog[my_log_id].mac[2]=l2addr->mac[2];
    maclog[my_log_id].mac[3]=l2addr->mac[3];
    maclog[my_log_id].mac[4]=l2addr->mac[4];
    maclog[my_log_id].mac[5]=l2addr->mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=l2addr->port;
    maclog[my_log_id].modid=l2addr->modid;
    maclog[my_log_id].vid=l2addr->vid;
#endif /* DEBUG_MACLOG */

    if (l2addr->flags & BCM_L2_MCAST) {
        /* Mcast */
        /* Note: mcast address is handled separately */

    } else if (BCM_GPORT_IS_VLAN_PORT(l2addr->port)) {
        /* VLAN GPORT */
        uint32 portFte = 0;
        rv = _bcm_caladan3_vlan_fte_gport_get(unit, l2addr->port, &portFte);

        if (BCM_E_NONE == rv) {
            sbx_mac_data.poe = sbx_mac_data.ftidx = portFte;
#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0x10;
	    maclog[my_log_id].fte=portFte;
#endif /* DEBUG_MACLOG */
	    
        } else {
#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0xf010;
	    maclog[my_log_id].fte=portFte;
#endif /* DEBUG_MACLOG */
	    return rv;
        }
    } else if (BCM_GPORT_IS_MIM_PORT(l2addr->port)) {
        /* MiM GPORT */
#ifdef BCM_FE2000_SUPPORT
        uint32 portFte = 0;
        rv = _bcm_fe2000_mim_fte_gport_get(unit, l2addr->port, &portFte);
        if (BCM_E_NONE == rv) {
            sbx_mac_data.poe = sbx_mac_data.ftidx = portFte;
#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0x110;
	    maclog[my_log_id].fte=portFte;
        } else {
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0xF110;
	    maclog[my_log_id].fte=portFte;
#endif /* DEBUG_MACLOG */
            return rv;
        }
#endif
    } else if (BCM_GPORT_IS_MPLS_PORT(l2addr->port)) {
        /* MPLS GPORT */
        sbx_mac_data.poe = sbx_mac_data.ftidx = BCM_GPORT_MPLS_PORT_ID_GET(l2addr->port);
#ifdef DEBUG_MACLOG
	maclog[my_log_id].flag=1;
	maclog[my_log_id].fn=0x210;
	maclog[my_log_id].fte=sbx_mac_data.ftidx;
#endif /* DEBUG_MACLOG */

    } else if (l2addr->flags & BCM_L2_TRUNK_MEMBER) {    /* Trunk */
        if (!SBX_TRUNK_VALID(l2addr->tgid)) {
#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0xF610;
	    maclog[my_log_id].fte=sbx_mac_data.ftidx;
#endif /* DEBUG_MACLOG */
            return BCM_E_BADID;
        }
    
        sbx_mac_data.poe = sbx_mac_data.ftidx = SOC_SBX_TRUNK_FTE(unit, l2addr->tgid);
#ifdef DEBUG_MACLOG
	maclog[my_log_id].flag=1;
	maclog[my_log_id].fn=0x610;
	maclog[my_log_id].fte=sbx_mac_data.ftidx;
#endif /* DEBUG_MACLOG */
    } else {    /* Modid and port */
        int                   node, fab_unit, fab_port;

        if (!SOC_SBX_MODID_ADDRESSABLE(unit, l2addr->modid)) {

#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0xF310;
	    maclog[my_log_id].fte=-1;
#endif /* DEBUG_MACLOG */
            return BCM_E_BADID;
        }
        if (!SOC_SBX_PORT_ADDRESSABLE(unit, l2addr->port)) {
#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0xF410;
	    maclog[my_log_id].fte=-1;
#endif /* DEBUG_MACLOG */
            return BCM_E_PORT;
        }

        /* Get Node associate to module-id */
        node = SOC_SBX_REMOTE_NODE_GET(unit, l2addr->modid, l2addr->port);
        if (!SOC_SBX_NODE_ADDRESSABLE(unit, node)) {
	    /* coverity[dead_error_line] */
            return BCM_E_BADID;
        }

        if (soc_sbx_node_port_get(unit, l2addr->modid, l2addr->port,
                                   &fab_unit, &node, &fab_port) == 0) {

            sbx_mac_data.poe = sbx_mac_data.ftidx = SOC_SBX_PORT_FTE(unit, node, fab_port);

#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0x510;
	    maclog[my_log_id].fte=sbx_mac_data.ftidx;
#endif /* DEBUG_MACLOG */

        } else {
#ifdef DEBUG_MACLOG
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0xF510;
	    maclog[my_log_id].fte=-1;
#endif /* DEBUG_MACLOG */
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "soc_sbx_node_port_get for modid %d port %d "
                                   "returned node %d port %d\n"), 
                       l2addr->modid, l2addr->port, node, fab_port));
            return BCM_E_INTERNAL;
        }
    }

    /* Drop */
    if (l2addr->flags & BCM_L2_DISCARD_SRC) {
        sbx_mac_data.sdrop = 1;
    }
    if (l2addr->flags & BCM_L2_DISCARD_DST) {
        sbx_mac_data.ddrop = 1;
    }

    /* Static */
    if (l2addr->flags & BCM_L2_STATIC) {
        sbx_mac_data.dontage = 1;
    }
    /* Entry subject to aging */
    else {
        sbx_mac_data.dontage = 0;

        /* Get a new free age id to link the entry to the aging table */
        _bcm_caladan3_l2_age_id_get(unit, &sbx_mac_data.ageid);

        /*  Update the mac aging table mac address, vid and valid fields */
        memcpy(&p_age_id_mac_array[unit][sbx_mac_data.ageid].mac,
                &l2addr->mac,sizeof(l2addr->mac));
        p_age_id_mac_array[unit][sbx_mac_data.ageid].vid = l2addr->vid;
        p_age_id_mac_array[unit][sbx_mac_data.ageid].valid = 1;

        /* Set the age for this entry in the MACAGE table */
        soc_sbx_g3p1_age_get(unit, &age);
        age_value.age = age;
        soc_sbx_g3p1_macage_set(unit, sbx_mac_data.ageid, &age_value);

    } /* ageable entry */




    /* there is only one table for both SMAC an DMAC in g3p1 */
    /* this call adds/updates and commits to hw */
    if (SOC_SBX_STATE(unit)->cache_l2 == FALSE) {
        rv = soc_sbx_g3p1_mac_set(unit,
                                  l2addr->mac,
                                  l2addr->vid,
                                  &sbx_mac_data);
    } else {
        rv = soc_sbx_g3p1_mac_add(unit,
                                  l2addr->mac,
                                  l2addr->vid,
                                  &sbx_mac_data);
    }
#if 0
    maclog[my_log_id].fte=sbx_mac_data.ftidx;
#endif
    return rv;
}


STATIC int
_bcm_caladan3_g3p1_mac_dump(int unit, mac_dump_f dump_f,  bcm_mac_t mac,
                          bcm_vlan_t vid, int max_count)
{
    soc_sbx_g3p1_6_byte_t  sbx_mac;
    soc_sbx_g3p1_mac_t     sbx_mac_data;
    int                    sbx_vid;
    int                    count;
    int                    nullMac;
    int                    rv;
    soc_sbx_g3p1_macage_t  age;

    nullMac = (ENET_CMP_MACADDR(mac, _mac_null) == 0);
    /* Initialize the iterator if both supplied, else get the first.  */
    if ((vid != 0) && !nullMac) {
        sbx_vid = vid;
        sal_memcpy(sbx_mac, mac, sizeof(sbx_mac));
        rv = BCM_E_NONE;
    } else {
        rv = soc_sbx_g3p1_mac_first(unit, sbx_mac, &sbx_vid);
    }

    count = 0;

    while (rv == BCM_E_NONE && ((max_count <= 0) || (count < max_count))) {
        
        rv = soc_sbx_g3p1_mac_get(unit, sbx_mac, sbx_vid, &sbx_mac_data);
        if (BCM_FAILURE(rv)) {
            break;
        }

       
	rv = soc_sbx_g3p1_macage_get(unit, sbx_mac_data.ageid, &age);
        if (BCM_FAILURE(rv)) {
            break;
        }

        if (((vid == 0 || vid == sbx_vid) && nullMac) ||
            ((vid == 0 || vid == sbx_vid) && 
             (ENET_CMP_MACADDR(mac, sbx_mac) == 0))) 
        {
	  dump_f(sbx_mac, sbx_vid, &sbx_mac_data,age.age);
            count++;
        }          

        rv = soc_sbx_g3p1_mac_next(unit, sbx_mac, sbx_vid,
                                    sbx_mac, &sbx_vid); 
    }
    
    L2_DUMP("Total entries = %d\n", count);   
    return BCM_E_NONE;


}


int
_bcm_caladan3_g3p1_smac_dump(int unit, bcm_mac_t mac, 
                           bcm_vlan_t vid, int max_count)
{
    L2_DUMP("SMAC Table\n");
    L2_DUMP("  SMAC               VLAN       POE  DROP !AGE    AGE\n");
    L2_DUMP("  ---------------------------------------------------\n");

    _bcm_caladan3_g3p1_mac_dump(unit, 
                              _bcm_l2_caladan3_g3p1_dump_smac_entry,
                              mac, vid, max_count);
    return BCM_E_NONE;
}


int
_bcm_caladan3_g3p1_dmac_dump(int unit, bcm_mac_t mac, 
                           bcm_vlan_t vid, int max_count)
{

    L2_DUMP("DMAC Table\n");
    L2_DUMP("  DMAC               VLAN       FTE   CP  DROP !AGE   AGE\n");
    L2_DUMP("  --------------------------------------------------------\n");

    _bcm_caladan3_g3p1_mac_dump(unit, 
                              _bcm_l2_caladan3_g3p1_dump_dmac_entry,
                              mac, vid, max_count);

    return BCM_E_NONE;
}


int _bcm_caladan3_g3p1_l2_mac_cmp(int unit, int vid, soc_sbx_g3p1_6_byte_t mac,
                                soc_sbx_g3p1_mac_t *mac_data,
                                bcm_l2_addr_t *match, 
                                uint32 flags)
{
    int          port;
    int          modid;
    bcm_trunk_t  tgid;

    /* Skip non-static entries if DELETE flag is not set */
    if (mac_data->dontage && !(match->flags & BCM_L2_DELETE_STATIC)) {
        return -1;
    }
    
    if (flags == 0) {
        return -1;
    }

    SOC_SBX_PORT_TGID_GET(unit, mac_data->poe, modid, port, tgid);

    /* use short-circuit to compare any combination of values based on flags
     */
    if( ( !(flags & L2_CMP_MAC)   || !ENET_CMP_MACADDR(mac, match->mac)) &&
        ( !(flags & L2_CMP_VLAN)  || !CMP_VLAN(vid, match->vid))         &&
        ( !(flags & L2_CMP_PORT)  || ((modid == match->modid) && 
                                      (port == match->port)))            &&
        ( !(flags & L2_CMP_TRUNK) || tgid == match->tgid)
        )
    {
        return 0;
    }
    return -1;
}


int
_bcm_caladan3_g3p1_l2_addr_delete_by(int unit, bcm_l2_addr_t *match,
                                   uint32 cmp_flags)
{
    int                    rv = BCM_E_NONE;
    int                    count;
    int                    idx;
    int                    sbx_vid[2];
    soc_sbx_g3p1_6_byte_t  sbx_mac[2];
    soc_sbx_g3p1_mac_t     sbx_mac_data;

    count = 0;
    soc_sbx_g3p1_mac_t_init(&sbx_mac_data);

    idx = 0;
    rv = soc_sbx_g3p1_mac_first(unit, sbx_mac[idx], &sbx_vid[idx]);
    if (BCM_FAILURE(rv)){
        if ((cmp_flags & L2_CMP_VLAN) && rv == SOC_E_NOT_FOUND) {
            return BCM_E_NONE;
        }
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Failed mac_first_get: %d (%s)\n"), 
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    
    do {
        int compare;

        rv = soc_sbx_g3p1_mac_get(unit, sbx_mac[idx], sbx_vid[idx],
                                  &sbx_mac_data);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to get mac data. vid=%d mac="
                                   L2_6B_MAC_FMT "\n"),
                       sbx_vid[idx], L2_6B_MAC_PFMT(sbx_mac[idx])));

            break;
        }

        compare = _bcm_caladan3_g3p1_l2_mac_cmp(unit, sbx_vid[idx], 
                                              sbx_mac[idx], &sbx_mac_data,
                                              match, cmp_flags);

        /* get the next before deleting */
        rv = soc_sbx_g3p1_mac_next(unit, sbx_mac[idx], sbx_vid[idx],
                                    sbx_mac[!idx], &sbx_vid[!idx]);

        if (compare == 0) {
            _ageid_to_mac_info_t  *p_age_entry; /* Pointer to the L2 Address aging entry */

            LOG_VERBOSE(BSL_LS_BCM_L2,
                        (BSL_META_U(unit,
                                    "removed vid=%d mac=" L2_6B_MAC_FMT "\n"),
                         sbx_vid[idx], L2_6B_MAC_PFMT(sbx_mac[idx])));

            soc_sbx_g3p1_mac_remove(unit, sbx_mac[idx], sbx_vid[idx]);

            /* Clear out the entry for the age_id and free the id back to the pool if nonstatic*/
            if (!sbx_mac_data.dontage) {
                p_age_entry = &p_age_id_mac_array[unit][sbx_mac_data.ageid];
                
                p_age_entry->valid = 0;
                _bcm_caladan3_l2_age_id_free(unit, sbx_mac_data.ageid);

                
#if DEBUG_AGING
                LOG_CLI((BSL_META_U(unit,
                                    "%s AgeId 0x%x\n"),FUNCTION_NAME(),sbx_mac_data.ageid));
#endif
                
                
            }
            


            count++;
        }

        idx = !idx;

    } while (BCM_SUCCESS(rv));

    if (BCM_E_NOT_FOUND == rv) {
        /* exited the loop after scanning all of the addresses */
        rv = BCM_E_NONE;
    }

    LOG_VERBOSE(BSL_LS_BCM_L2,
                (BSL_META_U(unit,
                            "Total entries deleted %d, rv=%d\n"),
                 count, rv));
    
    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_l2_addr_delete
 * Purpose:
 *     Delete an L2 address (MAC+VLAN) from the device.
 * Parameters:
 *     unit - Device number
 *     mac  - MAC address to delete
 *     vid  - VLAN id 
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_XXX       - Failure
 * Notes:
 *     Assumes valid unit.
 *     Assumes lock is held.
 */
int
_bcm_caladan3_g3p1_l2_addr_delete(int unit, bcm_mac_t mac, bcm_vlan_t vid)
{
    int  rv = BCM_E_NONE;
    _ageid_to_mac_info_t  * p_age_entry; /* Pointer to the L2 Address aging entry */
    soc_sbx_g3p1_mac_t   sbx_mac_data;   /* Ucode entry info. Has the aging index */

#ifdef DEBUG_MACLOG 
    int my_log_id;
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x11;
    maclog[my_log_id].mac[0]=mac[0];
    maclog[my_log_id].mac[1]=mac[1];
    maclog[my_log_id].mac[2]=mac[2];
    maclog[my_log_id].mac[3]=mac[3];
    maclog[my_log_id].mac[4]=mac[4];
    maclog[my_log_id].mac[5]=mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=-1;     
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vid;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    /* Look up the entry so we can update the age table */
    rv = soc_sbx_g3p1_mac_get(unit, mac, vid, &sbx_mac_data);
    if (BCM_FAILURE(rv)) {
           LOG_ERROR(BSL_LS_BCM_L2,
                     (BSL_META_U(unit,
                                 "failed to retrieve L2 MAC entry for delete rv=%d(%s)\n"),
                      rv, bcm_errmsg(rv)));
           return rv;
    }


    rv = soc_sbx_g3p1_mac_remove(unit, mac, vid);

    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to remove L2 MAC entry rv=%d(%s)\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Clear out the entry for the age_id and free the id back to the pool if nonstatic*/
    if (!sbx_mac_data.dontage) {
        p_age_entry = &p_age_id_mac_array[unit][sbx_mac_data.ageid];

        p_age_entry->valid = 0;
        _bcm_caladan3_l2_age_id_free(unit, sbx_mac_data.ageid);

#if DEBUG_AGING
        LOG_CLI((BSL_META_U(unit,
                            "%s AgeId 0x%x\n"),FUNCTION_NAME(),sbx_mac_data.ageid));
#endif


    }

    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_l2_addr_get
 * Purpose:
 *     Given a MAC address and VLAN ID, return all associated information
 *     if entry is present in the L2 tables.
 * Parameters:
 *     unit   - Device number
 *     mac    - MAC address to search
 *     vid    - VLAN id to search
 *     l2addr - (OUT) Pointer to bcm_l2_addr_t structure to return L2 entry
 * Returns:
 *     BCM_E_NONE      - Success
 *     BCM_E_NOT_FOUND - L2 address entry (MAC+VLAN) not found
 *     BCM_E_PARAM     - Illegal parameter (NULL pointer)
 *     BCM_E_XXX       - Failure, other
 * Notes:
 *     Assumes valid unit and non-null params.
 *     Assumes lock is held.
 */
int
_bcm_caladan3_g3p1_l2_addr_get(int unit, sal_mac_addr_t mac, bcm_vlan_t vid,
                             bcm_l2_addr_t *l2addr)
{
    int                 rv;
    int                 node, port, tgid;
    soc_sbx_g3p1_mac_t  mac_data;
    bcm_gport_t         fabric_port, switch_port;
    bcm_vlan_t          vsi;

#ifdef DEBUG_MACLOG 
    int my_log_id;

    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x13;
    maclog[my_log_id].mac[0]=mac[0];
    maclog[my_log_id].mac[1]=mac[1];
    maclog[my_log_id].mac[2]=mac[2];
    maclog[my_log_id].mac[3]=mac[3];
    maclog[my_log_id].mac[4]=mac[4];
    maclog[my_log_id].mac[5]=mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=-1;     
    maclog[my_log_id].modid=-1;
    maclog[my_log_id].vid=vid;
    maclog[my_log_id].fte=0;

#endif /* DEBUG_MACLOG */

    rv = soc_sbx_g3p1_mac_get(unit, mac, vid, &mac_data);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to get L2 MAC entry rv=%d(%s)\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }


    /* Init struct */
    bcm_l2_addr_t_init(l2addr, mac, vid);
#ifdef DEBUG_MACLOG 
    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);
    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x14;
    maclog[my_log_id].mac[0]=l2addr->mac[0];
    maclog[my_log_id].mac[1]=l2addr->mac[1];
    maclog[my_log_id].mac[2]=l2addr->mac[2];
    maclog[my_log_id].mac[3]=l2addr->mac[3];
    maclog[my_log_id].mac[4]=l2addr->mac[4];
    maclog[my_log_id].mac[5]=l2addr->mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=l2addr->port;     
    maclog[my_log_id].modid=l2addr->modid;
    maclog[my_log_id].vid=l2addr->vid;
    maclog[my_log_id].fte=mac_data.ftidx;

#endif /* DEBUG_MACLOG */

    /* Mod/port, trunk or mcast index */
    SOC_SBX_PORT_TGID_GET(unit, mac_data.poe, node, port, tgid);
    if ((node >= 0) && (port >= 0)) {
        /* Modid and port */
        /* Convert qid node port to switch port for local settings */
        SOC_SBX_MODID_FROM_NODE(node, node);
        BCM_GPORT_MODPORT_SET(fabric_port, node, port);
        rv = bcm_sbx_stk_fabric_map_get_switch_port(unit, 
                                                    fabric_port, &switch_port);
        if (BCM_FAILURE(rv)) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Failed to convert fabric port to switch"
                                   " port: %s\n"), bcm_errmsg(rv)));
            return rv;
        }

        l2addr->modid = BCM_GPORT_MODPORT_MODID_GET(switch_port);
        l2addr->port  = BCM_GPORT_MODPORT_PORT_GET(switch_port);  
#ifdef DEBUG_MACLOG 
	maclog[my_log_id].flag=1;
	maclog[my_log_id].fn=0x15;
	maclog[my_log_id].mac[0]=l2addr->mac[0];
	maclog[my_log_id].mac[1]=l2addr->mac[1];
	maclog[my_log_id].mac[2]=l2addr->mac[2];
	maclog[my_log_id].mac[3]=l2addr->mac[3];
	maclog[my_log_id].mac[4]=l2addr->mac[4];
	maclog[my_log_id].mac[5]=l2addr->mac[5];
	maclog[my_log_id].unit=unit;
	maclog[my_log_id].port=l2addr->port;     
	maclog[my_log_id].modid=l2addr->modid;
	maclog[my_log_id].vid=l2addr->vid;
	maclog[my_log_id].fte=mac_data.ftidx;
#endif /* DEBUG_MACLOG */


    } else if (tgid >= 0) {
        /* Trunk */
        l2addr->tgid   = tgid;
        l2addr->flags |= BCM_L2_TRUNK_MEMBER;
    } else {

        /* If FTIDX is a global GPORT FT, verify if its a mim port */
        if (mac_data.ftidx >= SBX_GLOBAL_GPORT_FTE_BASE(unit) &&
            mac_data.poe <= SBX_GLOBAL_GPORT_FTE_END(unit)) {
            uint8 type = 0;
            
            rv = _sbx_caladan3_get_global_fte_type(unit, mac_data.ftidx, &type);

            if (BCM_SUCCESS(rv)) {
                /* if its a MiM GPORT FTE, return GPORT ID on port */
#ifdef BCM_FE2000_SUPPORT
                if (BCM_GPORT_MIM_PORT == type) {
                    l2addr->port = _bcm_fe2000_mim_fte_to_gport_id(unit, mac_data.ftidx);
                } else {
                    LOG_WARN(BSL_LS_BCM_L2,
                             (BSL_META_U(unit,
                                         "FtIdx 0x%x in gport range, but unexpected type %d\n"),
                              mac_data.ftidx, type));
                }
#endif
            } else {
                LOG_ERROR(BSL_LS_BCM_L2,
                          (BSL_META_U(unit,
                                      "Failed to get type for gport 0x%x: %s\n"),
                           mac_data.ftidx, bcm_errmsg(rv)));
            }

#ifdef DEBUG_MACLOG 
	    maclog[my_log_id].flag=1;
	    maclog[my_log_id].fn=0x16;
	    maclog[my_log_id].mac[0]=l2addr->mac[0];
	    maclog[my_log_id].mac[1]=l2addr->mac[1];
	    maclog[my_log_id].mac[2]=l2addr->mac[2];
	    maclog[my_log_id].mac[3]=l2addr->mac[3];
	    maclog[my_log_id].mac[4]=l2addr->mac[4];
	    maclog[my_log_id].mac[5]=l2addr->mac[5];
	    maclog[my_log_id].unit=unit;
	    maclog[my_log_id].port=l2addr->port;     
	    maclog[my_log_id].modid=l2addr->modid;
	    maclog[my_log_id].vid=l2addr->vid;
	    maclog[my_log_id].fte=mac_data.ftidx;
#endif /* DEBUG_MACLOG */

        } else if (mac_data.ftidx >= SBX_LOCAL_GPORT_FTE_BASE(unit) &&
                   mac_data.ftidx <= SBX_LOCAL_GPORT_FTE_END(unit)) {

            l2addr->port = VLAN_FT_INDEX_TO_VGPORT_ID(unit, mac_data.ftidx);
            BCM_GPORT_VLAN_PORT_ID_SET(l2addr->port, l2addr->port);

            rv = bcm_vswitch_port_get(unit, l2addr->port, &vsi);
            if (BCM_FAILURE(rv)) {
                int status = BCM_E_NONE;
                /* Try to see if this is a MPLS VPLS gport */
                bcm_mpls_vpn_config_t vpncb;

                status = bcm_mpls_vpn_id_get(unit, vid, &vpncb);
                if (BCM_FAILURE(status)) {
                    LOG_ERROR(BSL_LS_BCM_L2,
                              (BSL_META_U(unit,
                                          "Failed to get vsi for gport 0x%x: %s or %s\n"),
                               l2addr->port, bcm_errmsg(rv), bcm_errmsg(status)));
                    return rv;
                } else {
                    if (vpncb.flags & BCM_MPLS_VPN_VPLS) {
                        BCM_GPORT_MPLS_PORT_ID_SET(l2addr->port, mac_data.poe);
                        rv = BCM_E_NONE;
                    } else {
                        LOG_ERROR(BSL_LS_BCM_L2,
                                  (BSL_META_U(unit,
                                              "VSI %d unsupported L2 Service\n"),
                                   vid));
                        return BCM_E_INTERNAL;
                    }
                }
            } else {
                if (vsi != vid) {
                    /* no match */
                    l2addr->port = 0;
                }
            }
        }

        if(!l2addr->port) {
            /* Mcast */
            BCM_IF_ERROR_RETURN(_bcm_caladan3_l2_mcast_get(unit, l2addr));
        }
    }

    /* Drop */
    if (mac_data.sdrop) {
        l2addr->flags |= BCM_L2_DISCARD_SRC;
    }
    if (mac_data.ddrop) {
        l2addr->flags |= BCM_L2_DISCARD_DST;
    }

    /* Static */
    if(mac_data.dontage) {
        l2addr->flags |= BCM_L2_STATIC;
    }

    return rv;
}


int
_bcm_caladan3_g3p1_l2_addr_update_dest(int unit, bcm_l2_addr_t *l2addr, 
                                     int qidunion)
{
    int                   rv = BCM_E_NONE;
    uint32              fteIdx;
    soc_sbx_g3p1_mac_t    macData;
    soc_sbx_g3p1_ft_t     sbxFte;


#ifdef DEBUG_MACLOG 
    int my_log_id;

    L2_LOG_LOCK(unit);
    my_log_id=log_index&0x3fff;
    log_index++;
    L2_LOG_UNLOCK(unit);

    maclog[my_log_id].flag=1;
    maclog[my_log_id].fn=0x12;
    maclog[my_log_id].mac[0]=l2addr->mac[0];
    maclog[my_log_id].mac[1]=l2addr->mac[1];
    maclog[my_log_id].mac[2]=l2addr->mac[2];
    maclog[my_log_id].mac[3]=l2addr->mac[3];
    maclog[my_log_id].mac[4]=l2addr->mac[4];
    maclog[my_log_id].mac[5]=l2addr->mac[5];
    maclog[my_log_id].unit=unit;
    maclog[my_log_id].port=l2addr->port;
    maclog[my_log_id].modid=l2addr->modid;
    maclog[my_log_id].vid=l2addr->vid;
    maclog[my_log_id].fte=0;
#endif /* DEBUG_MACLOG */

    /* Special handling on a mcast entry */
    if (l2addr->flags & BCM_L2_MCAST) {
        return BCM_E_PARAM;        
    }
        
    /* If LOCAL_CPU, get local CPU port and modid */
    if (l2addr->flags & BCM_L2_LOCAL_CPU) {
        return BCM_E_PARAM;
    }

    rv = soc_sbx_g3p1_mac_get(unit, l2addr->mac, l2addr->vid, &macData);
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to read MAC Payload: %d %s\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    /* Read the FTE */
    fteIdx = macData.ftidx;
    rv = soc_sbx_g3p1_ft_get(unit, fteIdx, &sbxFte);
#ifdef DEBUG_MACLOG 
    maclog[my_log_id].fte=fteIdx;    
#endif /* DEBUG_MACLOG */
    if (BCM_FAILURE(rv)) {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Error getting FTE: fte_idx=0x%x rv=%d %s\n"),
                   fteIdx, rv, bcm_errmsg(rv)));
        return rv;
    }
    
    /* 
     * will allocate FTE. It will be up to the l2 management code
     *  to de-allocate FTEs which are in the dynamic range 
     */
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT, 
                                 1, &fteIdx, 0);
    if (BCM_FAILURE(rv)) { 
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to allocate an FTE: %d %s\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }

    /* change only the qidunion */
    sbxFte.qid = qidunion;
    
    rv = soc_sbx_g3p1_ft_set(unit, fteIdx, &sbxFte);
    if (BCM_FAILURE(rv)) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT, 1,
                               &fteIdx, 0);
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Error setting FTE: fte_idx=0x%x rv=%d\n"),
                   fteIdx, rv));
        return rv;
    }
    
    /* update FTE on dmacyload */
    macData.ftidx = fteIdx;

    /* only update the DMAC, and only do so if it exists already */
    /* Add Dmac entry */
    rv = soc_sbx_g3p1_mac_update(unit, l2addr->mac, l2addr->vid, &macData);
    if (BCM_FAILURE(rv)) {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_FTE_LOCAL_GPORT, 1,
                               &fteIdx, 0);
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "failed to update L2 DMAC entry rv=%d (%s)\n"),
                   rv, bcm_errmsg(rv)));
        return rv;
    }
    
    return rv;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_l2_mac_size_get
 * Purpose:
 *     Get the L2 MAC table size.
 * Parameters:
 *     unit       - Device number
 *     table_size - Returns the L2 MAC table size.
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid params.
 */
int
_bcm_caladan3_g3p1_l2_mac_size_get(int unit, int *table_size)
{
    soc_sbx_g3p1_table_bank_params_t tbparams;

    SOC_IF_ERROR_RETURN
        (soc_sbx_g3p1_macage_bank_params_get(unit, 1, &tbparams));

    *table_size = tbparams.size;
    return BCM_E_NONE;
}


/*
 * Function:
 *     _bcm_caladan3_g3p1_l2_age_ager_set
 * Purpose:
 *     Set the L2 ager timestamp (age).
 * Parameters:
 *     unit - Device number
 *     ager - Age ager to set, 0..15
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit/ager values.
 */
int
_bcm_caladan3_g3p1_l2_age_ager_set(int unit, uint32 ager)
{
    return soc_sbx_g3p1_age_set(unit, ager);
}

/*
 * Function:
 *     _bcm_caladan3_g3p1_l2_age_ager_get
 * Purpose:
 *     Get the L2 ager timestamp (age).
 * Parameters:
 *     unit - Device number
 *     age - Pointer to age
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_XXX  - Failure
 * Notes:
 *     Assumes valid unit/ager values.
 */

int
_bcm_caladan3_g3p1_l2_age_ager_get(int unit, uint32 * age)
{
/* Set the age for this entry in the MACAGE table */
    return  soc_sbx_g3p1_age_get(unit, age);
  }

/*
 * Function:
 *     _bcm_caladan3_g3p1_l2_age_range_get
 * Purpose:
 *     Gets the range of the global GLOBAL_AGE
 * Parameters:
 *     unit - Device number
 *      - Age ager to set, 0..15
 * Returns:
 *     BCM_E_NONE - Success
 *     BCM_E_NOT_FOUND  - Failure
 * Notes:
 */
int
_bcm_caladan3_g3p1_l2_age_range_get(int unit, uint32 * p_age_range)
{

	if(!soc_sbx_g3p1_global_get(unit, "AGE_RANGE", p_age_range))
	{
		return BCM_E_NOT_FOUND;
	}
	else
	{
		return BCM_E_NONE;
	}
}



int
_bcm_caladan3_g3p1_l2_egress_dest_port_process(int unit, bcm_gport_t dest_port,
                                             int pri_remark, 
                                             soc_sbx_g3p1_ete_t *ete)
{
    int                 rv = BCM_E_NONE;
    bcm_module_t        my_modid, modid; 
    int                 port, use_pid;
    bcm_vlan_t          vlan, vsi;
    uint32            lpi, pid, egr_remark_idx;
    bcm_mpls_port_t     mpls_port;
    soc_sbx_g3p1_lp_t   lp;
    bcm_trunk_t         trunkId;
    uint32              trunkFte = ~0;
    int                 trunk_rv = BCM_E_NONE;
    uint8             vpwsuni=0;
    uint16            num_ports = 1;

    if (!ete) {
        return BCM_E_PARAM;
    }

    BCM_IF_ERROR_RETURN(bcm_stk_my_modid_get(unit, &my_modid));

    modid = port = -1;
    pid = ~0;
    use_pid = 0;
    if (BCM_GPORT_IS_MODPORT(dest_port)) {
        modid = BCM_GPORT_MODPORT_MODID_GET(dest_port);
        port = BCM_GPORT_MODPORT_PORT_GET(dest_port);
    } else if (BCM_GPORT_IS_MPLS_PORT(dest_port)) {
        bcm_mpls_port_t_init(&mpls_port);
        rv = _bcm_caladan3_mpls_port_vlan_vector_internal(unit, dest_port, 
                                                        &port, &vlan, &vsi, 
                                                        &lpi, &mpls_port, 
                                                        &vpwsuni, &num_ports);
        if (rv == BCM_E_NONE) {
            modid = my_modid;

            if (vpwsuni) {
                lpi += _BCM_CALADAN3_VPWS_UNI_OFFSET;
            }

            soc_sbx_g3p1_lp_t_init(&lp);
            rv = soc_sbx_g3p1_lp_get(unit, lpi, &lp);
        }
        if (rv == BCM_E_NONE) {
            use_pid = 1;
            pid = lp.pid;
        }
    } else if (BCM_GPORT_IS_VLAN_PORT(dest_port)) {
        rv = _bcm_caladan3_map_vlan_gport_target(unit, dest_port, &my_modid, 
                                               &modid, &port, &pid, NULL);
        if (rv == BCM_E_NONE) {
            use_pid = 1;
        }
        /* check for trunk membership.  If this vlan port exists on a trunk
         * a different PID/color is used to avoid reflection on the trunk.
         */
        trunk_rv = bcm_trunk_find(unit, my_modid, port, &trunkId);
        if (trunk_rv == BCM_E_NONE) {
           trunkFte = SOC_SBX_TRUNK_FTE(unit, trunkId);
        }
        if (trunkFte != ~0) {
            pid = trunkFte;
        }
    } else {
        LOG_ERROR(BSL_LS_BCM_L2,
                  (BSL_META_U(unit,
                              "Un-supported gport (0x%x) specified. MODPORT, "
                               " MPLS & VLAN gports are supported.\n"), 
                   dest_port));
        rv = BCM_E_PARAM;
    }

    if (rv == BCM_E_NONE) {
        if (my_modid != modid) {
            LOG_ERROR(BSL_LS_BCM_L2,
                      (BSL_META_U(unit,
                                  "Destination port (0x%x) is not on local unit.\n"), 
                       dest_port));
            return BCM_E_PARAM;
        }
        /* program the ete. NOTE: not written to HW at this stage. */
        if (pri_remark) {
            rv = bcm_caladan3_port_egr_remark_idx_get(unit, port, 
                                                     &egr_remark_idx);
            if (rv == BCM_E_NONE) {
                ete->remark = egr_remark_idx;
            }
        }
        if (use_pid) {
            ete->etepid = 1;
            ete->pid = pid;
        }
    }

    return rv;
}


int
_bcm_caladan3_g3p1_l2_egress_hw_entry_add(int unit, bcm_if_t encap_id, 
                                        bcm_l2_egress_t *egr)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx, ete_idx;
    soc_sbx_g3p1_oi2e_t         sbx_ohi;
    soc_sbx_g3p1_ete_t          ete;
    soc_sbx_g3p1_tpid_t         tpidstr;

    ete_idx = ~0;
    ete_idx = ~0;
    ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(encap_id);

    /* get an ETE */
    rv = _sbx_caladan3_resource_alloc(unit, SBX_CALADAN3_USR_RES_ETE, 1, 
                                 &ete_idx, 0);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    /* initialize the OHI */
    soc_sbx_g3p1_oi2e_t_init(&sbx_ohi);
    sbx_ohi.eteptr  = ete_idx;

    /* initialize the encap ete */
    soc_sbx_g3p1_ete_t_init(&ete);
    ete.dmacset = 0;
    ete.dmacsetlsb = 0;
    ete.nosplitcheck = 0;
    ete.ipttldec = 0;
    ete.ttlcheck = 0;
    ete.smacset = 0;
    ete.etype = 0;

    if (egr->flags & BCM_L2_EGRESS_DEST_MAC_PREFIX5_REPLACE) {
        ete.dmacset = 1;
    }
    if (egr->flags & BCM_L2_EGRESS_DEST_MAC_REPLACE) {
        ete.dmacset = 1;
        ete.dmacsetlsb = 1;
    }

    if (ete.dmacset) {
        ete.dmac0 = egr->dest_mac[0];
        ete.dmac1 = egr->dest_mac[1];
        ete.dmac2 = egr->dest_mac[2];
        ete.dmac3 = egr->dest_mac[3];
        ete.dmac4 = egr->dest_mac[4];
        if (ete.dmacsetlsb) {
            ete.dmac5 = egr->dest_mac[5];
        } else {
            /* dmac5 is not really used */
            ete.dmac5 = 0xff;
        }
    }
    
    if (egr->flags & BCM_L2_EGRESS_SRC_MAC_REPLACE) {
        
        rv = BCM_E_UNAVAIL;
    }

    if (egr->flags & BCM_L2_EGRESS_ETHERTYPE_REPLACE) {
        ete.etype = egr->ethertype;
        ete.encaplen = 2;
    }

    ete.mtu = SBX_DEFAULT_MTU_SIZE;
    
    /* default to untagged */
    ete.usevid       = 1;
    ete.vid          = _BCM_VLAN_G3P1_UNTAGGED_VID;

    if (egr->flags & BCM_L2_EGRESS_INNER_VLAN_REPLACE) {
        rv = soc_sbx_g3p1_tpid_get(unit, SB_G3P1_FE_CTPID_INDEX, &tpidstr);
        ete.encaplen = 4;
        ete.tpid = tpidstr.tpid;
        ete.encap_vid = egr->inner_vlan & 0xfff;
        if (egr->flags & BCM_L2_EGRESS_INNER_PRIO_REPLACE) {
            /* pricfi comes directly from the ETE */
            ete.pricfi = ((egr->inner_vlan & 0xf000) >> 12);
        }
    }

    if ((rv == BCM_E_NONE) && (egr->flags & BCM_L2_EGRESS_OUTER_VLAN_REPLACE)){
        ete.usevid       = 1;
        ete.vid          = egr->outer_vlan & 0xfff;
        ete.remark    = 0; /* default remarking table */
        /* - pricfi comes from ete.remark...
           - tpid comes from ep2e table...based on egress port
        */
    }

    if ((rv == BCM_E_NONE) && (egr->flags & BCM_L2_EGRESS_DEST_PORT)) {
        /* set the remark index and pid */
        rv = _bcm_caladan3_g3p1_l2_egress_dest_port_process(unit, egr->dest_port,
                                                          ete.usevid, 
                                                          &ete);
    }

    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g3p1_ete_set(unit, ete_idx, &ete);
    }
    if (rv == BCM_E_NONE) {
        rv = soc_sbx_g3p1_oi2e_set(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    }

    if (rv == BCM_E_NONE) {
        egr->encap_id = encap_id;
    } else {
        _sbx_caladan3_resource_free(unit, SBX_CALADAN3_USR_RES_ETE, 1, &ete_idx, 0);
    }

    return rv;
}

int
_bcm_caladan3_g3p1_l2_egress_hw_entry_get(int unit, bcm_if_t encap_id,
                                        bcm_l2_egress_t *egr)
{
    int                         rv = BCM_E_NONE;
    uint32                      ohi_idx, ete_idx;
    soc_sbx_g3p1_oi2e_t         sbx_ohi;
    soc_sbx_g3p1_ete_t          ete;
    
    PARAM_NULL_CHECK(egr);
    ohi_idx = SOC_SBX_OHI_FROM_L2_ENCAP_ID(encap_id);

    soc_sbx_g3p1_oi2e_t_init(&sbx_ohi);
    rv = soc_sbx_g3p1_oi2e_get(unit, ohi_idx - SBX_RAW_OHI_BASE, &sbx_ohi);
    if (rv != BCM_E_NONE) {
        return rv;
    }

    ete_idx = sbx_ohi.eteptr;
    soc_sbx_g3p1_ete_t_init(&ete);
    rv = soc_sbx_g3p1_ete_get(unit, ete_idx, &ete);
    if (rv != BCM_E_NONE) {
        return rv;
    }
    
    sal_memset(egr, 0, sizeof(bcm_l2_egress_t));

    if ((ete.usevid) && (ete.vid != _BCM_VLAN_G3P1_UNTAGGED_VID)) {
        egr->outer_vlan = ete.vid;
        egr->flags |= BCM_L2_EGRESS_OUTER_VLAN_REPLACE;
        
        egr->flags |= BCM_L2_EGRESS_OUTER_PRIO_REPLACE;
    }
    if (ete.encaplen == 4) {
        egr->inner_vlan = (ete.encap_vid & 0xfff);
        egr->flags |= BCM_L2_EGRESS_INNER_VLAN_REPLACE;
        if (ete.pricfi) {
            
            egr->flags |= BCM_L2_EGRESS_INNER_PRIO_REPLACE;
            egr->inner_vlan |= ((ete.pricfi & 0xf) << 12);
        }
    }

    if (ete.encaplen == 2) {
        egr->ethertype = ete.etype;
        egr->flags |= BCM_L2_EGRESS_ETHERTYPE_REPLACE;
    }

    if (ete.dmacset) {
        egr->dest_mac[0] = ete.dmac0;
        egr->dest_mac[1] = ete.dmac1;
        egr->dest_mac[2] = ete.dmac2;
        egr->dest_mac[3] = ete.dmac3;
        egr->dest_mac[4] = ete.dmac4;
        if (ete.dmacsetlsb) {
           egr->flags |= BCM_L2_EGRESS_DEST_MAC_REPLACE;
           egr->dest_mac[5] = ete.dmac5;
        } else {
           egr->flags |= BCM_L2_EGRESS_DEST_MAC_PREFIX5_REPLACE;
           egr->dest_mac[5] = 0xff;
        }
    }

    egr->encap_id = encap_id;

    return rv;
}

int
_bcm_caladan3_g3p1_run_ager(int unit,age_cb aged_mac_hndr, int delete, int current, int entries, int age_range)
{
    static int             count[BCM_LOCAL_UNITS_MAX]={0};
    int                    i;
    int                    macindex = 0;
    int                    rv = BCM_E_NONE;
    soc_sbx_g3p1_macage_t  age;
    _ageid_to_mac_info_t  * p_age_entry;

    /* Sanity Checks */
    if(entries == 0 || aged_mac_hndr == NULL)
    	return BCM_E_PARAM;

    /* Walk through the requested number of entries */
    for(i=0; i< entries; i++ )
    {
        /* Get the index for this unit for this entry */
        macindex = count[unit] + i;

        /* Look up the mac entry based on our internal mapping */
        p_age_entry =  &p_age_id_mac_array[unit][macindex];

        /* Check if we need to evaluate it */
        if (!p_age_entry->dontage && p_age_entry->valid) {

            /* Load the age from the table updated by microcode */
            rv = soc_sbx_g3p1_macage_get(unit, macindex, &age);

            /* Check if we are ok, if not continue */
            if (BCM_FAILURE(rv)) {
                continue;
            }

#if DEBUG_AGING
            LOG_CLI((BSL_META_U(unit,
                                "%s madindex 0x%x Age_range %d current %d age.age %d\n"),FUNCTION_NAME(),macindex, age_range,current,age.age));
#endif
            /* Check the age of the entry against the current time */
            if (L2_AGE_DELTA <= ((age_range + (current - age.age)) % age_range)) {
                /* Entry is stale - Call the call back function */
                aged_mac_hndr(unit, p_age_entry->mac, p_age_entry->vid, delete);
            }

        }

        /* check for end of array */
        if(macindex == age_indexes_stack[unit].size -1) {
            count[unit]=0;
            return BCM_E_EMPTY;
        }
    }

    /* Next pass will pick up here */
    count[unit] = macindex+1;

    return BCM_E_NONE;
}

void _bcm_caladan3_dump_ager_info(int unit, int start, int end,int stack)
{
uint32 i;

/* Take the lock */
  sal_mutex_take(age_indexes_stack[unit].lock, sal_mutex_FOREVER);

  if(stack)
  {
      LOG_CLI((BSL_META_U(unit,
                          "--------------Mac Aging Index Table------------------\n")));

      LOG_CLI((BSL_META_U(unit,
                          "Size 0x%x Current Pointer 0x%x\n"),  age_indexes_stack[unit].size,
               age_indexes_stack[unit].p));

    for(i=start; i <= end; i++) {
        LOG_CLI((BSL_META_U(unit,
                            "index[0x%x]=0x%x\n"),i,age_indexes_stack[unit].age_indexes[i]));
    }
  }
  else
  {
      LOG_CLI((BSL_META_U(unit,
                          "--------------Mac Aging Entry Table------------------\n")));

      for(i=start; i <=  end; i++) {
          LOG_CLI((BSL_META_U(unit,
                              "i:0x%x  mac 0x%02x:%02x:%02x:%02x:%02x:%02x vid %d valid %d dontage %d\n"),i,
                   p_age_id_mac_array[unit][i].mac[0], p_age_id_mac_array[unit][i].mac[1],
                   p_age_id_mac_array[unit][i].mac[2], p_age_id_mac_array[unit][i].mac[3],
                   p_age_id_mac_array[unit][i].mac[4], p_age_id_mac_array[unit][i].mac[5],
                   p_age_id_mac_array[unit][i].vid,  p_age_id_mac_array[unit][i].valid,
                   p_age_id_mac_array[unit][i].dontage));
      }

  }
    /* free the lock */
    sal_mutex_give(age_indexes_stack[unit].lock);

}



#endif /* BCM_CALADAN3_G3P1_SUPPORT */
