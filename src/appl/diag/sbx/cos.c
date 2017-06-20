/*
 * $Id: cos.c,v 1.34 Broadcom SDK $
 * $Copyright: (c) 2016 Broadcom.
 * Broadcom Proprietary and Confidential. All rights reserved.$
 *
 * COS CLI commands
 */

#include <shared/bsl.h>

#include <sal/appl/io.h>
#include <bcm_int/sbx_dispatch.h>
#include "sbTypes.h"
#include <bcm_int/sbx/cosq.h>
#include <bcm_int/sbx/stack.h>
#include <bcm_int/sbx/state.h>
#include <bcm/error.h>
#include <bcm/cosq.h>
#include <bcm/debug.h>
#include <soc/sbx/sbx_drv.h>
#include <soc/defs.h>

#ifdef BCM_SIRIUS_SUPPORT
#include <bcm_int/sbx/sirius.h>
#include <soc/sbx/sirius.h>
#endif

int
_bcm_gport_delete(int unit,bcm_gport_t port,
		  int numq, uint32 flags,
		  bcm_gport_t gport, void *user_data)

{
  int rv = 0;
  int retval = 0;
  bcm_gport_t physical_port = 0;  
  uint32 gport_flag = 0;
  int num_cos_levels=0;
  int _modid;
  int _port;
  bcm_sbx_gport_cb_params_t *p_cb_params = (bcm_sbx_gport_cb_params_t *)user_data;

  if (p_cb_params == NULL) {
      LOG_ERROR(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "Callback params pointer is null\n")));
    return (-1);
  }

  if (p_cb_params->cmd == BCM_SBX_COSQ_GPORT_REMOVE_ALL) {
    rv = bcm_cosq_gport_delete(unit,gport);
    if (rv != BCM_E_NONE) {
      LOG_ERROR(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "delete gport(%d) failed\n"),
                 gport));
    }
  } else {
    /* remove only gports that match user entered modid/and or port */
    rv = bcm_cosq_gport_get(unit,gport,&physical_port,&num_cos_levels,&gport_flag);
    if (rv != BCM_E_NONE) {
      LOG_ERROR(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "gport_get(%d) failed\n"),
                 gport));
      return rv;
    }

    _modid = BCM_GPORT_MODPORT_MODID_GET(physical_port);  
    _port  = BCM_GPORT_MODPORT_PORT_GET(physical_port);    

    if (p_cb_params->modid == -1) {
      /* remove gports that match this port only */
      if (_port == p_cb_params->port) {
	rv = bcm_cosq_gport_delete(unit,gport);
	if (rv != BCM_E_NONE) {
          LOG_ERROR(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "remove gport(%d) with port=%d failed\n"),
                     gport,_port));
	  retval = -1;
	}

      }
    } else if (p_cb_params->port == -1) {
      /* remove gport that match this modid only */
      if (_modid == p_cb_params->modid) {
	rv = bcm_cosq_gport_delete(unit,gport);
	if (rv != BCM_E_NONE) {
          LOG_ERROR(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "remove gport(%d) with modid=%d failed\n"),
                     gport,_modid));
	  retval = -1;
	}

      }
    } else {
      /* remove gports that match both modid and port */
      if ((_modid == p_cb_params->modid && _port == p_cb_params->port)) {
	rv = bcm_cosq_gport_delete(unit,gport);
	if (rv != BCM_E_NONE) {
          LOG_ERROR(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "remove gport(%d) failed\n"),
                     gport));
	  retval = -1;
	}
      }
    }
  }
  return retval;
}

static char *
sbx_get_bw_mode_string(int bw_mode) {
    switch ( bw_mode ) {
        case BCM_COSQ_SP:
            return "BCM_COSQ_SP";
        case BCM_COSQ_EF:
            return "BCM_COSQ_EF";
        case BCM_COSQ_SP_GLOBAL:
            return "BCM_COSQ_SP_GLOBAL";
        case BCM_COSQ_AF:
            return "BCM_COSQ_AF";
        case BCM_COSQ_WEIGHTED_FAIR_QUEUING:
            return "BCM_COSQ_WEIGHTED_FAIR_QUEUING";
        case BCM_COSQ_BE:
            return "BCM_COSQ_BE";
        case BCM_COSQ_AF0:
            return "BCM_COSQ_AF0";
        case BCM_COSQ_AF1:
            return "BCM_COSQ_AF1";
        case BCM_COSQ_AF2:
            return "BCM_COSQ_AF2";
        case BCM_COSQ_AF3:
            return "BCM_COSQ_AF3";
        case BCM_COSQ_SP0:
            return "BCM_COSQ_SP0";
        case BCM_COSQ_SP1:
            return "BCM_COSQ_SP1";
        case BCM_COSQ_SP2:
            return "BCM_COSQ_SP2";
        case BCM_COSQ_SP3:
            return "BCM_COSQ_SP3";
        case BCM_COSQ_SP4:
            return "BCM_COSQ_SP4";
        case BCM_COSQ_SP5:
            return "BCM_COSQ_SP5";
        case BCM_COSQ_SP6:
            return "BCM_COSQ_SP6";
        case BCM_COSQ_SP7:
            return "BCM_COSQ_SP7";
        case BCM_COSQ_NONE:
            return "BCM_COSQ_NONE";
    }
    return "bw mode invalid for SBX devices";
}

static char*
sbx_get_gport_type_string(bcm_gport_t gport) {

  if (BCM_GPORT_IS_MODPORT(gport)) {
      return "ModPort";
  }else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)){
      return "Unicast Queue Group";
  }else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
      return "Multicast Queue Group";
  }else if (BCM_GPORT_IS_UCAST_SUBSCRIBER_QUEUE_GROUP(gport)){
      return "Subscriber Unicast Queue Group";
  }else if (BCM_GPORT_IS_MCAST_SUBSCRIBER_QUEUE_GROUP(gport)){
      return "Subscriber Multicast Queue Group";
  }else if (BCM_GPORT_IS_MCAST(gport)) {
      return "Multicast";
  }else if (BCM_GPORT_IS_CHILD(gport)) {
      return "Child";
  }else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
      return "Egress child";
  }else if (BCM_GPORT_IS_SCHEDULER(gport)) {
      return "Scheduler";
  }else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
      return "Egress ModPort";
  }else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
      return "Egress Group";
  }
  /* gport is invalid */
  return "Invalid";
}


void
sbx_cosq_gport_get_node_port(int unit, bcm_gport_t gport, 
			   int *node, int *port) {


  int sysport = 0;
  int modid = 0;
  bcm_sbx_cosq_egress_group_state_t *p_eg = NULL;
  int eg_n = 0;

  if (BCM_GPORT_IS_MODPORT(gport)) {
    modid = BCM_GPORT_MODPORT_MODID_GET(gport);
    *port  = BCM_GPORT_MODPORT_PORT_GET(gport);
  } else if (BCM_GPORT_IS_UCAST_QUEUE_GROUP(gport)) {
    sysport = BCM_GPORT_UCAST_QUEUE_GROUP_SYSPORTID_GET(gport);
    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
      modid = -1; *port = -1;
    } else {
      modid = SOC_SBX_STATE(unit)->sysport_state[sysport].node;
      *port  = SOC_SBX_STATE(unit)->sysport_state[sysport].port;
    }
  } else if (BCM_GPORT_IS_MCAST_QUEUE_GROUP(gport)) {
    sysport = BCM_GPORT_MCAST_QUEUE_GROUP_SYSPORTID_GET(gport);
    if ( (sysport < 0) || (sysport >= SOC_SBX_CFG(unit)->num_sysports) ) {
      modid = -1; *port = -1;
    } else {
      modid = SOC_SBX_STATE(unit)->sysport_state[sysport].node;
      *port  = SOC_SBX_STATE(unit)->sysport_state[sysport].port;
    }
  } else if (BCM_GPORT_IS_CHILD(gport)) {
    modid = BCM_GPORT_CHILD_MODID_GET(gport);
    *port  = BCM_GPORT_CHILD_PORT_GET(gport);
  } else if (BCM_GPORT_IS_EGRESS_CHILD(gport)) {
    modid = BCM_GPORT_EGRESS_CHILD_MODID_GET(gport);
    *port  = BCM_GPORT_EGRESS_CHILD_PORT_GET(gport);
  } else if (BCM_GPORT_IS_EGRESS_MODPORT(gport)) {
    modid = BCM_GPORT_EGRESS_MODPORT_MODID_GET(gport);
    *port  = BCM_GPORT_EGRESS_MODPORT_PORT_GET(gport);
  } else if (BCM_GPORT_IS_EGRESS_GROUP(gport)) {
    modid = BCM_GPORT_EGRESS_GROUP_MODID_GET(gport);
    p_eg  = SOC_SBX_STATE(unit)->egress_group_state;
    eg_n  = BCM_GPORT_EGRESS_GROUP_GET(gport);
    if ((p_eg == NULL) || (eg_n < 0) || (eg_n >= SOC_SBX_CFG(unit)->num_egress_group)) {
      /* egress group is not valid */
      cli_out("Egress Group (0x%08x) is invalid\n",gport);
      *port = -1;
    } else {
      *port  = (p_eg + eg_n)->child_port;
    }
  }  else {
    /* gport type is qid based */
    *port = 0;
  }

  if (modid >= BCM_MODULE_FABRIC_BASE) {
    *node = BCM_STK_MOD_TO_NODE(modid);  
  } else {
    *node = modid;
  }

}

#ifdef BCM_SIRIUS_SUPPORT

  /* Recursion is more efficient for printing the tree,
   * but does not print in a top-down level for easy viewing.
   * Could only get this to work by having enough nested
   * for loops to get down to the queue level. Maximum
   * level for a scheduler is at level 6, so there are enough 
   * nested loops to get to the queue level for this scheduler. 
   */

void sbx_print_scheduler_tree(int unit, int level, int node) {
#define MAX_SCHEDULER_DEPTH 6
#define QUEUE_LEVEL 0
  bcm_gport_t child_gport = 0;
  bcm_gport_t parent = 0;
  int rv = BCM_E_NONE;
  bcm_sbx_sirius_ingress_scheduler_state_t *ps = NULL;

  /* place holders for schedulers at a given level
   * at each level a given scheduler will have n number of children
   * on the level below it, and a first child for that level.
   */
  bcm_gport_t sch [MAX_SCHEDULER_DEPTH] = {0};
  int children    [MAX_SCHEDULER_DEPTH] = {0};
  int first       [MAX_SCHEDULER_DEPTH] = {0};

  /* need to use nested for loops, the child array below
   * is used to index children within the current level
   */
  int child       [MAX_SCHEDULER_DEPTH] = {0};

  /* If we are at the queue level, just show the queue group
   * gport once. The parent(L1) will have ncos children, but each
   * will be the same queue group gport
   */

  ps = &is_state[unit][level][node];
  if (!ps || ps->in_use==FALSE) return;
  parent = ps->parent_gport;
  cli_out("L%d: gport=0x%08x ",level+1,parent);
  cli_out("\t\t\t\t %-10s\n",sbx_get_gport_type_string(parent));
  
  children[0]    = ps->num_child;
  first[0]       = ps->first_child;
  BCM_GPORT_SCHEDULER_SET(sch[0],ps->logical_scheduler);
  cli_out("   L%d: gport=0x%08x",level,sch[0]);
  cli_out("\t\t\t\t %-10s %-14s %07d \t %07d\n",sbx_get_gport_type_string(sch[0]),
          sbx_get_bw_mode_string(ps->scheduler_mode),
          ps->min_shaper_rate_kbps,
          ps->max_shaper_rate_kbps);
  if (children[0] > 0) {
    for (child[0]=0; child[0] < children[0]; child[0]++) {
      rv = bcm_sbx_cosq_gport_attach_get(unit,sch[0],&child_gport,&child[0]);
      if (rv == BCM_E_NOT_FOUND) continue;
      cli_out("      L%d: gport=0x%08x",level-1,child_gport);
      ps = &is_state[unit][level-1][first[0]+child[0]];
	cli_out("\t\t\t %-10s %-14s %07d \t %07d\n",sbx_get_gport_type_string(child_gport),
                sbx_get_bw_mode_string(ps->scheduler_mode),
                ps->min_shaper_rate_kbps,
                ps->max_shaper_rate_kbps);
      sch[1]      = child_gport;
      children[1] = ps->num_child;
      first[1]    = ps->first_child;
      if (children[1] > 0) {
	for (child[1] =0; child[1] < children[1]; child[1]++) {
	  rv = bcm_sbx_cosq_gport_attach_get(unit,sch[1],&child_gport,&child[1]);
	  if (rv == BCM_E_NOT_FOUND) continue;
	  cli_out("         L%d: gport=0x%08x",level-2,child_gport);	
	  ps = &is_state[unit][level-2][first[1]+child[1]];
	    cli_out("\t\t\t %-10s %-14s %07d \t %07d\n",sbx_get_gport_type_string(child_gport),
                    sbx_get_bw_mode_string(ps->scheduler_mode),
                    ps->min_shaper_rate_kbps,
                    ps->max_shaper_rate_kbps);
	  sch[2]      = child_gport;
	  children[2] = ps->num_child;
	  first[2]    = ps->first_child;
	  if (children[2] > 0) {
	    for(child[2]=0; child[2] < children[2]; child[2]++) {
	      rv = bcm_sbx_cosq_gport_attach_get(unit,sch[2],&child_gport,&child[2]);
	      if (rv == BCM_E_NOT_FOUND) continue;
	      cli_out("            L%d: gport=0x%08x",level-3,child_gport);	
	      ps = &is_state[unit][level-3][first[2]+child[2]];
		cli_out("\t\t %-10s %-14s %07d \t %07d\n",sbx_get_gport_type_string(child_gport),
                        sbx_get_bw_mode_string(ps->scheduler_mode),
                        ps->min_shaper_rate_kbps,
                        ps->max_shaper_rate_kbps);
	      sch[3]        = child_gport;
	      children[3]   = ps->num_child;
	      first[3]      = ps->first_child;
	      if (children[3] > 0) {
		for(child[3]=0;child[3] < ((level-4==QUEUE_LEVEL)?1:children[3]); child[3]++) {
		  if (!BCM_GPORT_IS_SCHEDULER(sch[3])) continue;
		  rv = bcm_sbx_cosq_gport_attach_get(unit,sch[3],&child_gport,&child[3]);
		  if (rv == BCM_E_NOT_FOUND) continue;
		  cli_out("               L%d: gport=0x%08x",level-4,child_gport);	
		  ps = &is_state[unit][level-4][first[3]+child[3]];
		  if (BCM_GPORT_IS_SCHEDULER(child_gport)) {
		    cli_out("\t\t %-10s %-14s %07d \t %07d\n",sbx_get_gport_type_string(child_gport),
                            sbx_get_bw_mode_string(ps->scheduler_mode),
                            ps->min_shaper_rate_kbps,
                            ps->max_shaper_rate_kbps);
		  } else {
		    cli_out("\t\t %-10s \n",sbx_get_gport_type_string(child_gport));
		  }
		}

		sch[4]         = child_gport;
		children[4]    = ps->num_child;
		first[4]       = ps->first_child;
		if (children[4] > 0) {
		  for(child[4]=0;child[4] < ((level-5==QUEUE_LEVEL)?1:children[4]) ; child[4]++) {
		    if (!BCM_GPORT_IS_SCHEDULER(sch[4])) continue;
		    rv = bcm_sbx_cosq_gport_attach_get(unit,sch[4],&child_gport,&child[4]);
		    if (rv == BCM_E_NOT_FOUND) continue;
		    cli_out("                  L%d: gport=0x%08x",level-5,child_gport);	
		    ps = &is_state[unit][level-5][first[4]+child[4]];
		    if (BCM_GPORT_IS_SCHEDULER(child_gport)) {
		      cli_out("\t\t %-10s %-14s %07d \t %07d\n",sbx_get_gport_type_string(child_gport),
                              sbx_get_bw_mode_string(ps->scheduler_mode),
                              ps->min_shaper_rate_kbps,
                              ps->max_shaper_rate_kbps);
		    } else {
		      cli_out("\t\t %-10s \n",sbx_get_gport_type_string(child_gport));
		    }
		  }
		  sch[5]      = child_gport;
		  children[5] = ps->num_child;
		  first[5]    = ps->first_child;
		  if (children[5] > 0) {
		    for (child[5]=0;child[5] < 1; child[5]++) {
		      if (!BCM_GPORT_IS_SCHEDULER(sch[5])) continue;
		      rv = bcm_sbx_cosq_gport_attach_get(unit,sch[5],&child_gport,&child[5]);
		      if (rv == BCM_E_NOT_FOUND) continue;
		      cli_out("                     L%d: gport=0x%08x",level-6,child_gport);
		      cli_out("\t %-10s \n",sbx_get_gport_type_string(child_gport));		    
		    }
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }
}

#endif


/* 
 *  Display information about gport
 */

void
sbx_gport_display(int unit, bcm_gport_t gport)
{
  int modid = 0;
  int port = 0;
#ifdef BCM_SIRIUS_SUPPORT
  int level = 0;
  int node = 0;
  int i = 0, rv = BCM_E_NONE;
  bcm_gport_t scheduler = 0;
  bcm_gport_t parent = 0;
  bcm_sbx_cosq_ingress_scheduler_state_t *p_i_scheduler = NULL;
  bcm_sbx_sirius_ingress_scheduler_state_t *ps = NULL;
  int cosq = -1;


  /* display information about gport, special case if scheduler type */
  if (BCM_GPORT_IS_SCHEDULER(gport)) {

    if (!SOC_IS_SIRIUS(unit)) return;

    p_i_scheduler = SOC_SBX_STATE(unit)->ingress_scheduler_state;
    if (p_i_scheduler == NULL ) {
      cli_out("No Ingress scheduler state found.\n");
      return;
    }

    /* find this scheduler in the ingress scheduler state */
    for (i=0;i<SOC_SBX_CFG(unit)->num_ingress_scheduler; i++) {
      if ( (p_i_scheduler + i) == NULL || (p_i_scheduler+i)->in_use == FALSE ) continue;
      level = (p_i_scheduler+i)->level;
      node  = (p_i_scheduler+i)->node;
      ps = &is_state[unit][level][node];
      /* coverity[dead_error_line] */
      if (ps == NULL) continue;
      BCM_GPORT_SCHEDULER_SET(scheduler,ps->logical_scheduler);
      if (scheduler == gport) {
	rv = bcm_sbx_cosq_gport_attach_get(unit,scheduler,&parent,&cosq);
        if (rv == BCM_E_NOT_FOUND) continue;
	cli_out(" gport \t\t type \t\tmode         node    level    min_rate(kbps)  max_rate(kbps)  parent \t type \n");
	cli_out(" 0x%08x \t %-10s %-14s   %04d    %04d\t%07d \t %07d      0x%08x %-10s \n",
                gport,
                sbx_get_gport_type_string(gport),
                sbx_get_bw_mode_string(ps->scheduler_mode),
                node,
                level,
                ps->min_shaper_rate_kbps,
                ps->max_shaper_rate_kbps,
                parent,
                sbx_get_gport_type_string(parent));
      }
    }
  } else {
    cli_out(" gport \t\t type \t\t     mod   port\n");
    sbx_cosq_gport_get_node_port(unit,gport,&modid,&port);
    cli_out(" 0x%08x \t %-12s        %04d  %04d\n",
            gport,
            sbx_get_gport_type_string(gport),
            modid,
            port);
  }
#else
  {
    cli_out(" gport \t\t type \t\t     mod   port\n");
    sbx_cosq_gport_get_node_port(unit,gport,&modid,&port);
    cli_out(" 0x%08x \t %-12s        %04d  %04d\n",
            gport,
            sbx_get_gport_type_string(gport),
            modid,
            port);
  }
#endif
}


#ifdef BCM_SIRIUS_SUPPORT
void 
sbx_gport_scheduler_show(int unit)
{

  int level = 0;
  int node = 0;
  int i = 0;
  bcm_gport_t parent = 0;
  bcm_sbx_cosq_ingress_scheduler_state_t *p_i_scheduler = NULL;
  bcm_sbx_sirius_ingress_scheduler_state_t *t_i_scheduler = NULL;
  int num_ingress_schedulers = 0;

  if (!SOC_IS_SIRIUS(unit)) {
    cli_out("Only supported on sirius\n");
    return;
  }

  p_i_scheduler = SOC_SBX_STATE(unit)->ingress_scheduler_state;
  if (p_i_scheduler == NULL ) {
    cli_out("No Ingress scheduler state found.\n");
    return;
  }

  num_ingress_schedulers = SOC_SBX_CFG(unit)->num_ingress_scheduler;
  if (num_ingress_schedulers == 0 ) {
    cli_out("No ingress schedulers found\n");
    return;
  }
  cli_out("\t\t\t\t\t\t Type \t    Mode         min_rate(kbps)  max_rate(kbps)\n");
  for (i=0;i<num_ingress_schedulers; i++) {
    if ( (p_i_scheduler + i) == NULL || (p_i_scheduler+i)->in_use == FALSE ) continue;

    level = (p_i_scheduler+i)->level;
    node  = (p_i_scheduler+i)->node;
    if ((0 > node) || (0 > level)) {
        
        cli_out("skip - level = %d, node = %d, invalid\n", level, node);
        continue;
    }
    t_i_scheduler = &is_state[unit][level][node];
    if (t_i_scheduler == NULL ) {
	/* coverity[dead_error_begin] */
        cli_out("skip scheduler is NULL\n");
        continue;
    }
    parent = t_i_scheduler->parent_gport;
    if (parent != BCM_GPORT_INVALID) {
      sbx_print_scheduler_tree(unit,level,node);
      cli_out("\n");
    }
  }
}
#endif

void 
sbx_gport_map_show(int unit) 
{

    uint32 entry;
    bcm_sbx_stack_portmap_block_t * map;
    int switch_node = 0;
    int switch_port = 0;
    int fabric_node = 0;
    int fabric_port = 0;
    cli_out(" Switch Gport     Type          Fabric Gport     Type  \t\t\t        Switch (Node|Port)  Fabric (Node|Port) \n");
    cli_out(" --------------------------------------------------------------------------------------------------------------------\n");
    /* Show mappings of switch port, fabric port */
    for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
      if (map->count > 0) {
	for(entry=0;entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE;entry+=2) {
	  if (map->portmap[entry] == BCM_GPORT_INVALID) continue; 
	  sbx_cosq_gport_get_node_port(unit,map->portmap[entry],&switch_node,&switch_port);
	  sbx_cosq_gport_get_node_port(unit,map->portmap[entry+1],&fabric_node,&fabric_port);
	  cli_out("  0x%08x \t %-s \t 0x%08x \t %-32s  (%05d|%04d)\t      (%05d|%04d)\n",
                  map->portmap[entry],sbx_get_gport_type_string(map->portmap[entry]),
                  map->portmap[entry+1],sbx_get_gport_type_string(map->portmap[entry+1]),
                  switch_node,switch_port,fabric_node,fabric_port);
	}
      }
    }
}

void 
sbx_gport_map_delete(int unit) 
{

    uint32 entry;
    bcm_sbx_stack_portmap_block_t * map;
    int rv = 0;
    for(map=SOC_SBX_STATE(unit)->stack_state->gport_map; map!=NULL; map=map->next) {
      if (map->count > 0) {
	for(entry=0;entry<2*_BCM_SBX_STACK_PORTMAP_CHUCK_SIZE;entry+=2) {
	  if (map->portmap[entry] == BCM_GPORT_INVALID) continue; 
	  /* make it invalid */
	  rv = bcm_sbx_stk_fabric_map_set(unit,map->portmap[entry],BCM_GPORT_INVALID);
	  if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "bcm_sbx_stk_fabric_map_set failed for "
                                  "switch_port(0x%08x): %s\n"),
                       map->portmap[entry], bcm_errmsg(rv)));
	  }
	}
      }
    }
}

int
sbx_cosq_gport_show(int unit,bcm_gport_t gport,
		    int verbose)
{

  int rv = BCM_E_NONE;
  int cos;
  int base_queue;
  int num_cos_levels;
  int mode;
  int weight;
  char *type = NULL;
  int modid = 0;
  int port = 0;
  bcm_gport_t physical_port = 0;
  char *mode_str = NULL;
  uint32 min_size, max_size, flags = 0;
  uint32 kbits_sec_min, kbits_sec_max;

  if (SOC_IS_SBX_FE(unit)) {
    return BCM_E_UNAVAIL;
  }

  type = sbx_get_gport_type_string(gport);
  if (strstr(type,"Invalid") != 0) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "ERROR: gport(0x%x) is invalid\n"),
               gport));
    return (BCM_E_PARAM);
  }

  /* get base_queue and num_cos_levels */
  rv = bcm_sbx_cosq_get_base_queue_from_gport(unit,gport,&base_queue,&num_cos_levels);
  if (rv != BCM_E_NONE) {
      return rv;
  }
    
  BCM_IF_ERROR_RETURN(bcm_cosq_gport_get(unit, gport, &physical_port, &num_cos_levels, &flags));

  modid = BCM_GPORT_MODPORT_MODID_GET(physical_port);
  if (SOC_IS_SBX_SIRIUS(unit) && modid == 0) {
      modid += BCM_MODULE_FABRIC_BASE;
  }
  port  = BCM_GPORT_MODPORT_PORT_GET(physical_port);
  cli_out("  0x%08x   %2d     %32s    %5d      %2d %12d\n",gport, num_cos_levels, type, modid, port, base_queue);
  /* get mode and weighted value for each cos level */
  if (verbose == 1) {
      cli_out("               cos       mode           weight    kbps_min   kbps_max  min_size  max_size  flags\n");
      for(cos=0;cos<num_cos_levels;cos++) {
	  BCM_IF_ERROR_RETURN(bcm_cosq_gport_sched_get(unit,gport,cos,&mode,&weight));
	  BCM_IF_ERROR_RETURN(bcm_cosq_gport_bandwidth_get(unit,gport,cos,&kbits_sec_min,&kbits_sec_max,&flags));
          BCM_IF_ERROR_RETURN(bcm_cosq_gport_size_get(unit, gport, cos, &min_size, &max_size));
	  mode_str = sbx_get_bw_mode_string(mode);
	  cli_out("              %3d    %-22s %d     %-6d      %-6d     %dKB    %dKB %3d  \n",cos,mode_str,weight,kbits_sec_min,kbits_sec_max, min_size/1024, max_size/1024, flags);
      }
  }

  return rv;
}

int
_bcm_gport_show(int unit,bcm_gport_t port,
		int numq, uint32 flags,
		bcm_gport_t gport, void *user_data)

{
  int rv = 0;
  int retval = 0;
  bcm_gport_t physical_port = 0;  
  uint32 gport_flag = 0;
  int num_cos_levels=0;
  int _modid;
  int _port;
  bcm_sbx_gport_cb_params_t *p_cb_params = (bcm_sbx_gport_cb_params_t *)user_data;

  if (p_cb_params == NULL) {
    LOG_ERROR(BSL_LS_BCM_COSQ,
              (BSL_META_U(unit,
                          "callback params pointer is null\n")));
    return (-1);
  }

  if (p_cb_params->cmd == BCM_SBX_COSQ_GPORT_SHOW_ALL) {
    rv = sbx_cosq_gport_show(unit,gport,p_cb_params->verbose);
    if (rv != BCM_E_NONE) {
      LOG_ERROR(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "show gport(%d) failed\n"),
                 gport));
      retval = -1;
    }
  } else {
    /* show only gports that match this modid/and or port */
    rv = bcm_cosq_gport_get(unit,gport,&physical_port,&num_cos_levels,&gport_flag);
    if (rv != BCM_E_NONE) {
      LOG_ERROR(BSL_LS_BCM_COSQ,
                (BSL_META_U(unit,
                            "gport_get(%d) failed\n"),
                 gport));
      return rv;
    }

    _modid = BCM_GPORT_MODPORT_MODID_GET(physical_port);  
    _port  = BCM_GPORT_MODPORT_PORT_GET(physical_port);    

    if (p_cb_params->modid == -1) {
      /* show gports that match this port only */
      if (_port == p_cb_params->port) {
	rv = sbx_cosq_gport_show(unit,gport,p_cb_params->verbose);
	if (rv != BCM_E_NONE) {
          LOG_ERROR(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "show gport(%d) with port=%d failed\n"),
                     gport,_port));
	  retval = -1;
	}

      }
    } else if (p_cb_params->port == -1) {
      /* show gport that match this modid only */
      if (_modid == p_cb_params->modid) {
	rv = sbx_cosq_gport_show(unit,gport,p_cb_params->verbose);
	if (rv != BCM_E_NONE) {
          LOG_ERROR(BSL_LS_BCM_COSQ,
                    (BSL_META_U(unit,
                                "show gport(%d) with modid=%d failed\n"),
                     gport,_modid));
	  retval = -1;
	}

      }
    } else {
      /* show gports that match both modid and port */
      if ((_modid == p_cb_params->modid && _port == p_cb_params->port)) {
	rv = sbx_cosq_gport_show(unit,gport,p_cb_params->verbose);
	if (rv != BCM_E_NONE) {
            LOG_ERROR(BSL_LS_BCM_COSQ,
                      (BSL_META_U(unit,
                                  "show gport(%d) failed\n"),
                       gport));
	  retval = -1;
	}
      }
    }
  }
  return retval;
}

#ifdef BCM_SIRIUS_SUPPORT
int
_sbx_sirius_ingress_tree_level(int unit, int levels, int level, int node)
{
    int current;
    int parent;
    int first;
    int children;
    int result = SOC_E_NONE;
    uint32 queue;
    int leaf;
    int mc;
    static const char *l7 = "+-------";
    static const char *l6 = " +------";
    static const char *l5 = "  +-----";
    static const char *l4 = "   +----";
    static const char *l3 = "    +---";
    static const char *l2 = "     +--";
    static const char *l1 = "      +-";
    static const char *l0 = "       +";
    static const int limit[] = { 65536, 8192, 4096, 1024, 264, 132, 56, 7 };
    const char *prefix;

    if (level > (7 - levels)) {
        result = soc_sirius_ts_node_hierachy_config_get(unit,
                                                        level,
                                                        node,
                                                        &parent,
                                                        &first,
                                                        &children);
        switch (level) {
        case 7:
            prefix = l7;
            break;
        case 6:
            prefix = l6;
            break;
        case 5:
            prefix = l5;
            break;
        case 4:
            prefix = l4;
            break;
        case 3:
            prefix = l3;
            break;
        case 2:
            prefix = l2;
            break;
        case 1:
            prefix = l1;
            break;
        default:
            prefix = l0;
        }
        if (children) {
            if (7 == level) {
                cli_out("%1X.%04X %s        %1X.%04X+%04X\n",
                        level,
                        node,
                        prefix,
                        level - 1,
                        first,
                        children);
            } else if (1 == level) {
                result = soc_sirius_qs_leaf_node_to_queue_get(unit,
                                                              first,
                                                              &queue);
                if (SOC_E_NONE == result) {
                    result = soc_sirius_qs_queue_to_leaf_node_get(unit,
                                                                  queue,
                                                                  &mc,
                                                                  &leaf);
                }
                if (SOC_E_NONE == result) {
                    cli_out("%1X.%04X %s %1X.%04X %1X.%04X+%04X %04X %s %04X\n",
                            level,
                            node,
                            prefix,
                            level + 1,
                            parent,
                            level - 1,
                            first,
                            children,
                            queue,
                            mc?"M":"-",
                            leaf);
                    first = queue;
                } else {
                    return result;
                }
            } else {
                cli_out("%1X.%04X %s %1X.%04X %1X.%04X+%04X\n",
                        level,
                        node,
                        prefix,
                        level + 1,
                        parent,
                        level - 1,
                        first,
                        children);
            }
            for (current = first;
                  (SOC_E_NONE == result) &&
                  (current < first + children);
                  current++) {
                if ((1 < level) && (current < limit[level - 1])) {
                    result = _sbx_sirius_ingress_tree_level(unit,
                                                            levels,
                                                            level - 1,
                                                            current);
                } else if ((1 == level) &&
                           ((level - 1) > (7 - levels)) &&
                           (current < limit[level - 1])) {
                    cli_out("%1X.%04X %s %1X.%04X\n",
                            level,
                            node,
                            l0,
                            level - 1,
                            current);
                }
            }
        } else {
            if (7 > level) {
                cli_out("%1X.%04X %s\n",
                        level,
                        node,
                        prefix);
            } else {
                cli_out("%1X.%04X %s %1X.%04X\n",
                        level,
                        node,
                        prefix,
                        level + 1,
                        parent);
            }
        }
    }
    return result;
}

/*
 *  Dump ingress heirarchy to the specified depth.
 */
int
sbx_sirius_ingress_tree_show(int unit, int levels)
{
    int current;
    int result = SOC_E_NONE;

    if (6 < levels) {
        cli_out("Node             Parent Child  Cnt  QID   M Leaf\n");
        cli_out("------ --------- ------ ------ ---- ----- - ----\n");
    } else {
        cli_out("Node             Parent Child  Cnt\n");
        cli_out("------ --------- ------ ------ ----\n");
    }
    for (current = 0;
         (SOC_E_NONE == result) &&
         (current < 7);
         current++) {
        
        result = _sbx_sirius_ingress_tree_level(unit, levels, 7, current);
    }
    return result;
}
#endif /* def BCM_SIRIUS_SUPPORT */

